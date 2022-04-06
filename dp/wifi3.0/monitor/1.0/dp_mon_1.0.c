/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <dp_types.h>
#include "dp_rx.h"
#include "dp_peer.h"
#include <dp_htt.h>
#include <dp_mon_filter.h>
#include <dp_mon.h>
#include <dp_rx_mon.h>
#include <dp_rx_mon_1.0.h>
#include <dp_mon_1.0.h>
#include <dp_mon_filter_1.0.h>

#include "htt_ppdu_stats.h"
#if defined(DP_CON_MON)
#ifndef REMOVE_PKT_LOG
#include <pktlog_ac_api.h>
#include <pktlog_ac.h>
#endif
#endif
#ifdef FEATURE_PERPKT_INFO
#include "dp_ratetable.h"
#endif

#ifdef WLAN_TX_PKT_CAPTURE_ENH
#include "dp_tx_capture.h"
#endif

#define HTT_MGMT_CTRL_TLV_HDR_RESERVERD_LEN 16
#define HTT_TLV_HDR_LEN HTT_T2H_EXT_STATS_CONF_TLV_HDR_SIZE
#define HTT_SHIFT_UPPER_TIMESTAMP 32
#define HTT_MASK_UPPER_TIMESTAMP 0xFFFFFFFF00000000

#ifndef WLAN_TX_PKT_CAPTURE_ENH
static inline void
dp_process_ppdu_stats_update_failed_bitmap(struct dp_pdev *pdev,
					   void *data,
					   uint32_t ppdu_id,
					   uint32_t size)
{
}
#endif

extern QDF_STATUS dp_srng_alloc(struct dp_soc *soc, struct dp_srng *srng,
				int ring_type, uint32_t num_entries,
				bool cached);
extern void dp_srng_free(struct dp_soc *soc, struct dp_srng *srng);
extern QDF_STATUS dp_srng_init(struct dp_soc *soc, struct dp_srng *srng,
			       int ring_type, int ring_num, int mac_id);
extern void dp_srng_deinit(struct dp_soc *soc, struct dp_srng *srng,
			   int ring_type, int ring_num);

extern enum timer_yield_status
dp_should_timer_irq_yield(struct dp_soc *soc, uint32_t work_done,
			  uint64_t start_time);

#ifdef QCA_SUPPORT_FULL_MON
static QDF_STATUS
dp_config_full_mon_mode(struct cdp_soc_t *soc_handle,
			uint8_t val)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	mon_soc->full_mon_mode = val;
	dp_cdp_err("Configure full monitor mode val: %d ", val);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
dp_soc_config_full_mon_mode(struct cdp_pdev *cdp_pdev, uint8_t val)
{
	struct dp_pdev *pdev = (struct dp_pdev *)cdp_pdev;
	struct dp_soc *soc = pdev->soc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (!mon_soc->full_mon_mode)
		return QDF_STATUS_SUCCESS;

	if ((htt_h2t_full_mon_cfg(soc->htt_handle,
				  pdev->pdev_id,
				  val)) != QDF_STATUS_SUCCESS) {
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}
#else
static inline QDF_STATUS
dp_config_full_mon_mode(struct cdp_soc_t *soc_handle,
			uint8_t val)
{
	return 0;
}

static inline QDF_STATUS
dp_soc_config_full_mon_mode(struct cdp_pdev *cdp_pdev,
			    uint8_t val)
{
	return 0;
}
#endif

#if !defined(DISABLE_MON_CONFIG)
void dp_flush_monitor_rings(struct dp_soc *soc)
{
	struct dp_pdev *pdev = soc->pdev_list[0];
	hal_soc_handle_t hal_soc = soc->hal_soc;
	uint32_t lmac_id;
	uint32_t hp, tp;
	int dp_intr_id;
	int budget;
	void *mon_dst_srng;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	/* Reset monitor filters before reaping the ring*/
	qdf_spin_lock_bh(&mon_pdev->mon_lock);
	dp_mon_filter_reset_mon_mode(pdev);
	if (dp_mon_filter_update(pdev) != QDF_STATUS_SUCCESS)
		dp_info("failed to reset monitor filters");
	qdf_spin_unlock_bh(&mon_pdev->mon_lock);

	if (mon_pdev->mon_chan_band == REG_BAND_UNKNOWN)
		return;

	lmac_id = pdev->ch_band_lmac_id_mapping[mon_pdev->mon_chan_band];
	if (qdf_unlikely(lmac_id == DP_MON_INVALID_LMAC_ID))
		return;

	dp_intr_id = soc->mon_intr_id_lmac_map[lmac_id];
	if (qdf_unlikely(dp_intr_id == DP_MON_INVALID_LMAC_ID))
		return;

	mon_dst_srng = dp_rxdma_get_mon_dst_ring(pdev, lmac_id);

	/* reap full ring */
	budget = wlan_cfg_get_dma_mon_stat_ring_size(pdev->wlan_cfg_ctx);

	hal_get_sw_hptp(hal_soc, mon_dst_srng, &tp, &hp);
	dp_info("Before reap: Monitor DST ring HP %u TP %u", hp, tp);

	dp_mon_process(soc, &soc->intr_ctx[dp_intr_id], lmac_id, budget);

	hal_get_sw_hptp(hal_soc, mon_dst_srng, &tp, &hp);
	dp_info("After reap: Monitor DST ring HP %u TP %u", hp, tp);
}

static
void dp_mon_rings_deinit_1_0(struct dp_pdev *pdev)
{
	int mac_id = 0;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;
	struct dp_soc *soc = pdev->soc;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0;
	     mac_id  < soc->wlan_cfg_ctx->num_rxdma_status_rings_per_pdev;
	     mac_id++) {
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id,
							 pdev->pdev_id);

		dp_srng_deinit(soc, &soc->rxdma_mon_status_ring[lmac_id],
			       RXDMA_MONITOR_STATUS, 0);

		dp_mon_dest_rings_deinit(pdev, lmac_id);
	}
}

static
void dp_mon_rings_free_1_0(struct dp_pdev *pdev)
{
	int mac_id = 0;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;
	struct dp_soc *soc = pdev->soc;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0;
	     mac_id  < soc->wlan_cfg_ctx->num_rxdma_status_rings_per_pdev;
	     mac_id++) {
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id,
							 pdev->pdev_id);

		dp_srng_free(soc, &soc->rxdma_mon_status_ring[lmac_id]);

		dp_mon_dest_rings_free(pdev, lmac_id);
	}
}

static
QDF_STATUS dp_mon_rings_init_1_0(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	int mac_id = 0;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0;
	     mac_id  < soc->wlan_cfg_ctx->num_rxdma_status_rings_per_pdev;
	     mac_id++) {
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id,
							 pdev->pdev_id);

		if (dp_srng_init(soc, &soc->rxdma_mon_status_ring[lmac_id],
				 RXDMA_MONITOR_STATUS, 0, lmac_id)) {
			dp_mon_err("%pK: " RNG_ERR "rxdma_mon_status_ring",
				   soc);
			goto fail1;
		}

		if (dp_mon_dest_rings_init(pdev, lmac_id))
			goto fail1;
	}
	return QDF_STATUS_SUCCESS;

fail1:
	dp_mon_rings_deinit_1_0(pdev);
	return QDF_STATUS_E_NOMEM;
}

static
QDF_STATUS dp_mon_rings_alloc_1_0(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	int mac_id = 0;
	int entries;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0;
	     mac_id  < soc->wlan_cfg_ctx->num_rxdma_status_rings_per_pdev;
	     mac_id++) {
		int lmac_id =
		dp_get_lmac_id_for_pdev_id(soc, mac_id, pdev->pdev_id);
		entries = wlan_cfg_get_dma_mon_stat_ring_size(pdev_cfg_ctx);
		if (dp_srng_alloc(soc, &soc->rxdma_mon_status_ring[lmac_id],
				  RXDMA_MONITOR_STATUS, entries, 0)) {
			dp_mon_err("%pK: " RNG_ERR "rxdma_mon_status_ring",
				   soc);
			goto fail1;
		}

		if (dp_mon_dest_rings_alloc(pdev, lmac_id))
			goto fail1;
	}
	return QDF_STATUS_SUCCESS;

fail1:
	dp_mon_rings_free_1_0(pdev);
	return QDF_STATUS_E_NOMEM;
}
#else
inline
void dp_flush_monitor_rings(struct dp_soc *soc)
{
}

static inline
void dp_mon_rings_deinit_1_0(struct dp_pdev *pdev)
{
}

static inline
void dp_mon_rings_free_1_0(struct dp_pdev *pdev)
{
}

static inline
QDF_STATUS dp_mon_rings_init_1_0(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS dp_mon_rings_alloc_1_0(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

#endif

#ifdef QCA_MONITOR_PKT_SUPPORT
void dp_vdev_set_monitor_mode_buf_rings(struct dp_pdev *pdev)
{
	uint32_t mac_id;
	uint32_t mac_for_pdev;
	struct dp_srng *mon_buf_ring;
	uint32_t num_entries;
	struct dp_soc *soc = pdev->soc;

	/* If delay monitor replenish is disabled, allocate link descriptor
	 * monitor ring buffers of ring size.
	 */
	if (!wlan_cfg_is_delay_mon_replenish(soc->wlan_cfg_ctx)) {
		dp_vdev_set_monitor_mode_rings(pdev, false);
	} else {
		for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
			mac_for_pdev =
				dp_get_lmac_id_for_pdev_id(pdev->soc,
							   mac_id,
							   pdev->pdev_id);

			dp_rx_pdev_mon_buf_buffers_alloc(pdev, mac_for_pdev,
							 FALSE);
			mon_buf_ring =
				&pdev->soc->rxdma_mon_buf_ring[mac_for_pdev];
			/*
			 * Configure low interrupt threshld when monitor mode is
			 * configured.
			 */
			if (mon_buf_ring->hal_srng) {
				num_entries = mon_buf_ring->num_entries;
				hal_set_low_threshold(mon_buf_ring->hal_srng,
						      num_entries >> 3);
				htt_srng_setup(pdev->soc->htt_handle,
					       pdev->pdev_id,
					       mon_buf_ring->hal_srng,
					       RXDMA_MONITOR_BUF);
			}
		}
	}
}
#endif

#ifdef QCA_MONITOR_PKT_SUPPORT
QDF_STATUS dp_vdev_set_monitor_mode_rings(struct dp_pdev *pdev,
					  uint8_t delayed_replenish)
{
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;
	uint32_t mac_id;
	uint32_t mac_for_pdev;
	struct dp_soc *soc = pdev->soc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_srng *mon_buf_ring;
	uint32_t num_entries;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	/* If monitor rings are aleady initilized, return from here */
	if (mon_pdev->pdev_mon_init)
		return QDF_STATUS_SUCCESS;

	for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
		mac_for_pdev = dp_get_lmac_id_for_pdev_id(pdev->soc, mac_id,
							  pdev->pdev_id);

		/* Allocate sw rx descriptor pool for mon RxDMA buffer ring */
		status = dp_rx_pdev_mon_buf_desc_pool_alloc(pdev, mac_for_pdev);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			dp_err("%s: dp_rx_pdev_mon_buf_desc_pool_alloc() failed\n",
			       __func__);
			goto fail0;
		}

		dp_rx_pdev_mon_buf_desc_pool_init(pdev, mac_for_pdev);

		/* If monitor buffers are already allocated,
		 * do not allocate.
		 */
		status = dp_rx_pdev_mon_buf_buffers_alloc(pdev, mac_for_pdev,
							  delayed_replenish);

		mon_buf_ring = &pdev->soc->rxdma_mon_buf_ring[mac_for_pdev];
		/*
		 * Configure low interrupt threshld when monitor mode is
		 * configured.
		 */
		if (mon_buf_ring->hal_srng) {
			num_entries = mon_buf_ring->num_entries;
			hal_set_low_threshold(mon_buf_ring->hal_srng,
					      num_entries >> 3);
			htt_srng_setup(pdev->soc->htt_handle,
				       pdev->pdev_id,
				       mon_buf_ring->hal_srng,
				       RXDMA_MONITOR_BUF);
		}

		/* Allocate link descriptors for the mon link descriptor ring */
		status = dp_hw_link_desc_pool_banks_alloc(soc, mac_for_pdev);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			dp_err("%s: dp_hw_link_desc_pool_banks_alloc() failed",
			       __func__);
			goto fail0;
		}
		dp_link_desc_ring_replenish(soc, mac_for_pdev);

		htt_srng_setup(soc->htt_handle, pdev->pdev_id,
			       soc->rxdma_mon_desc_ring[mac_for_pdev].hal_srng,
			       RXDMA_MONITOR_DESC);
		htt_srng_setup(soc->htt_handle, pdev->pdev_id,
			       soc->rxdma_mon_dst_ring[mac_for_pdev].hal_srng,
			       RXDMA_MONITOR_DST);
	}
	mon_pdev->pdev_mon_init = 1;

	return QDF_STATUS_SUCCESS;

fail0:
	return QDF_STATUS_E_FAILURE;
}
#endif

/* dp_mon_vdev_timer()- timer poll for interrupts
 *
 * @arg: SoC Handle
 *
 * Return:
 *
 */
static void dp_mon_vdev_timer(void *arg)
{
	struct dp_soc *soc = (struct dp_soc *)arg;
	struct dp_pdev *pdev = soc->pdev_list[0];
	enum timer_yield_status yield = DP_TIMER_NO_YIELD;
	uint32_t work_done  = 0, total_work_done = 0;
	int budget = 0xffff;
	uint32_t remaining_quota = budget;
	uint64_t start_time;
	uint32_t lmac_id = DP_MON_INVALID_LMAC_ID;
	uint32_t lmac_iter;
	int max_mac_rings = wlan_cfg_get_num_mac_rings(pdev->wlan_cfg_ctx);
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	if (!qdf_atomic_read(&soc->cmn_init_done))
		return;

	if (mon_pdev->mon_chan_band != REG_BAND_UNKNOWN)
		lmac_id = pdev->ch_band_lmac_id_mapping[mon_pdev->mon_chan_band];

	start_time = qdf_get_log_timestamp();
	dp_update_num_mac_rings_for_dbs(soc, &max_mac_rings);

	while (yield == DP_TIMER_NO_YIELD) {
		for (lmac_iter = 0; lmac_iter < max_mac_rings; lmac_iter++) {
			if (lmac_iter == lmac_id)
				work_done = dp_monitor_process(
						    soc, NULL,
						    lmac_iter, remaining_quota);
			else
				work_done =
					dp_monitor_drop_packets_for_mac(pdev,
								     lmac_iter,
								     remaining_quota);
			if (work_done) {
				budget -=  work_done;
				if (budget <= 0) {
					yield = DP_TIMER_WORK_EXHAUST;
					goto budget_done;
				}
				remaining_quota = budget;
				total_work_done += work_done;
			}
		}

		yield = dp_should_timer_irq_yield(soc, total_work_done,
						  start_time);
		total_work_done = 0;
	}

budget_done:
	if (yield == DP_TIMER_WORK_EXHAUST ||
	    yield == DP_TIMER_TIME_EXHAUST)
		qdf_timer_mod(&mon_soc->mon_vdev_timer, 1);
	else
		qdf_timer_mod(&mon_soc->mon_vdev_timer, DP_INTR_POLL_TIMER_MS);
}

/* MCL specific functions */
#if defined(DP_CON_MON)
/*
 * dp_mon_reap_timer_handler()- timer to reap monitor rings
 * reqd as we are not getting ppdu end interrupts
 * @arg: SoC Handle
 *
 * Return:
 *
 */
static void dp_mon_reap_timer_handler(void *arg)
{
	struct dp_soc *soc = (struct dp_soc *)arg;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	dp_service_mon_rings(soc, QCA_NAPI_BUDGET);

	qdf_timer_mod(&mon_soc->mon_reap_timer, DP_INTR_POLL_TIMER_MS);
}

static void dp_mon_reap_timer_init(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

        qdf_timer_init(soc->osdev, &mon_soc->mon_reap_timer,
                       dp_mon_reap_timer_handler, (void *)soc,
                       QDF_TIMER_TYPE_WAKE_APPS);
        mon_soc->reap_timer_init = 1;
}
#else
static void dp_mon_reap_timer_init(struct dp_soc *soc)
{
}
#endif

static void dp_mon_reap_timer_deinit(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
        if (mon_soc->reap_timer_init) {
                qdf_timer_free(&mon_soc->mon_reap_timer);
                mon_soc->reap_timer_init = 0;
        }
}

static void dp_mon_reap_timer_start(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
        if (mon_soc->reap_timer_init) {
                qdf_timer_mod(&mon_soc->mon_reap_timer, DP_INTR_POLL_TIMER_MS);
        }

}

static bool dp_mon_reap_timer_stop(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
        if (mon_soc->reap_timer_init) {
                qdf_timer_sync_cancel(&mon_soc->mon_reap_timer);
                return true;
        }

        return false;
}

static void dp_mon_vdev_timer_init(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

        qdf_timer_init(soc->osdev, &mon_soc->mon_vdev_timer,
                       dp_mon_vdev_timer, (void *)soc,
                       QDF_TIMER_TYPE_WAKE_APPS);
        mon_soc->mon_vdev_timer_state |= MON_VDEV_TIMER_INIT;
}

static void dp_mon_vdev_timer_deinit(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
        if (mon_soc->mon_vdev_timer_state & MON_VDEV_TIMER_INIT) {
                qdf_timer_free(&mon_soc->mon_vdev_timer);
                mon_soc->mon_vdev_timer_state = 0;
        }
}

static void dp_mon_vdev_timer_start(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
        if (mon_soc->mon_vdev_timer_state & MON_VDEV_TIMER_INIT) {
                qdf_timer_mod(&mon_soc->mon_vdev_timer, DP_INTR_POLL_TIMER_MS);
                mon_soc->mon_vdev_timer_state |= MON_VDEV_TIMER_RUNNING;
        }
}

static bool dp_mon_vdev_timer_stop(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
        if (mon_soc->mon_vdev_timer_state & MON_VDEV_TIMER_RUNNING) {
                qdf_timer_sync_cancel(&mon_soc->mon_vdev_timer);
                mon_soc->mon_vdev_timer_state &= ~MON_VDEV_TIMER_RUNNING;
		return true;
        }

	return false;
}

static void dp_mon_neighbour_peer_add_ast(struct dp_pdev *pdev,
					  struct dp_peer *ta_peer,
					  uint8_t *mac_addr,
					  qdf_nbuf_t nbuf,
					  uint32_t flags)
{
	struct dp_neighbour_peer *neighbour_peer = NULL;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;
	struct dp_soc *soc = pdev->soc;
	uint32_t ret = 0;

	if (mon_pdev->neighbour_peers_added) {
		qdf_mem_copy(mac_addr,
			     (qdf_nbuf_data(nbuf) +
			      QDF_MAC_ADDR_SIZE),
			      QDF_MAC_ADDR_SIZE);

		qdf_spin_lock_bh(&mon_pdev->neighbour_peer_mutex);
		TAILQ_FOREACH(neighbour_peer,
			      &mon_pdev->neighbour_peers_list,
			      neighbour_peer_list_elem) {
			if (!qdf_mem_cmp(&neighbour_peer->neighbour_peers_macaddr,
					 mac_addr,
					 QDF_MAC_ADDR_SIZE)) {
				ret = dp_peer_add_ast(soc,
						      ta_peer,
						      mac_addr,
						      CDP_TXRX_AST_TYPE_WDS,
						      flags);
				QDF_TRACE(QDF_MODULE_ID_DP,
					  QDF_TRACE_LEVEL_INFO,
					  "sa valid and nac roamed to wds");
				break;
			}
		}
		qdf_spin_unlock_bh(&mon_pdev->neighbour_peer_mutex);
	}
}

#if !defined(DISABLE_MON_CONFIG)

/**
 * dp_mon_htt_srng_setup_1_0() - Prepare HTT messages for Monitor rings
 * @soc: soc handle
 * @pdev: physical device handle
 * @mac_id: ring number
 * @mac_for_pdev: mac_id
 *
 * Return: non-zero for failure, zero for success
 */
#if defined(DP_CON_MON)
static
QDF_STATUS dp_mon_htt_srng_setup_1_0(struct dp_soc *soc,
				     struct dp_pdev *pdev,
				     int mac_id,
				     int mac_for_pdev)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = dp_mon_htt_dest_srng_setup(soc, pdev, mac_id, mac_for_pdev);
	if (status != QDF_STATUS_SUCCESS)
		return status;

	if (!soc->rxdma_mon_status_ring[mac_id].hal_srng)
		return QDF_STATUS_SUCCESS;

	status = htt_srng_setup(soc->htt_handle, mac_for_pdev,
				soc->rxdma_mon_status_ring[mac_id]
				.hal_srng,
				RXDMA_MONITOR_STATUS);

	if (status != QDF_STATUS_SUCCESS) {
		dp_mon_err("Failed to send htt srng setup message for Rxdma mon status ring");
		return status;
	}

	return status;
}
#else
/* This is only for WIN */
static
QDF_STATUS dp_mon_htt_srng_setup_1_0(struct dp_soc *soc,
				     struct dp_pdev *pdev,
				     int mac_id,
				     int mac_for_pdev)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_mon_soc *mon_soc;

	mon_soc = soc->monitor_soc;
	if(!mon_soc) {
		dp_mon_err("%pK: monitor SOC not initialized", soc);
		return status;
	}

	if (mon_soc->monitor_mode_v2)
		return status;

	if (wlan_cfg_is_delay_mon_replenish(soc->wlan_cfg_ctx)) {
		status = dp_mon_htt_dest_srng_setup(soc, pdev,
						    mac_id, mac_for_pdev);
		if (status != QDF_STATUS_SUCCESS)
			return status;
	}

	status = htt_srng_setup(soc->htt_handle, mac_for_pdev,
				soc->rxdma_mon_status_ring[mac_id]
				.hal_srng,
				RXDMA_MONITOR_STATUS);

	if (status != QDF_STATUS_SUCCESS) {
		dp_mon_err("Failed to send htt srng setup msg for Rxdma mon status ring");
		return status;
	}

	return status;
}
#endif
#endif

/* MCL specific functions */
#if defined(DP_CON_MON)

/*
 * dp_service_mon_rings()- service monitor rings
 * @soc: soc dp handle
 * @quota: number of ring entry that can be serviced
 *
 * Return: None
 *
 */
void dp_service_mon_rings(struct  dp_soc *soc, uint32_t quota)
{
	int ring = 0, work_done;
	struct dp_pdev *pdev = NULL;

	for (ring = 0 ; ring < MAX_NUM_LMAC_HW; ring++) {
		pdev = dp_get_pdev_for_lmac_id(soc, ring);
		if (!pdev)
			continue;
		work_done = dp_mon_process(soc, NULL, ring, quota);

		dp_rx_mon_dest_debug("Reaped %d descs from Monitor rings",
				     work_done);
	}
}
#endif

/*
 * dp_peer_tx_init() – Initialize receive TID state
 * @pdev: Datapath pdev
 * @peer: Datapath peer
 *
 */
static void
dp_peer_tx_init(struct dp_pdev *pdev, struct dp_peer *peer)
{
	dp_peer_tid_queue_init(peer);
	dp_peer_update_80211_hdr(peer->vdev, peer);
}

/*
 * dp_peer_tx_cleanup() – Deinitialize receive TID state
 * @vdev: Datapath vdev
 * @peer: Datapath peer
 *
 */
static void
dp_peer_tx_cleanup(struct dp_vdev *vdev, struct dp_peer *peer)
{
	dp_peer_tid_queue_cleanup(peer);
}

#ifdef QCA_SUPPORT_BPR
static QDF_STATUS
dp_set_bpr_enable_1_0(struct dp_pdev *pdev, int val)
{
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	switch (val) {
	case CDP_BPR_DISABLE:
		mon_pdev->bpr_enable = CDP_BPR_DISABLE;
		if (!mon_pdev->pktlog_ppdu_stats &&
		    !mon_pdev->enhanced_stats_en &&
		    !mon_pdev->tx_sniffer_enable && !mon_pdev->mcopy_mode) {
			dp_h2t_cfg_stats_msg_send(pdev, 0, pdev->pdev_id);
		} else if (mon_pdev->enhanced_stats_en &&
			   !mon_pdev->tx_sniffer_enable &&
			   !mon_pdev->mcopy_mode &&
			   !mon_pdev->pktlog_ppdu_stats) {
			dp_h2t_cfg_stats_msg_send(pdev,
						  DP_PPDU_STATS_CFG_ENH_STATS,
						  pdev->pdev_id);
		}
		break;
	case CDP_BPR_ENABLE:
		mon_pdev->bpr_enable = CDP_BPR_ENABLE;
		if (!mon_pdev->enhanced_stats_en &&
		    !mon_pdev->tx_sniffer_enable &&
		    !mon_pdev->mcopy_mode && !mon_pdev->pktlog_ppdu_stats) {
			dp_h2t_cfg_stats_msg_send(pdev,
						  DP_PPDU_STATS_CFG_BPR,
						  pdev->pdev_id);
		} else if (mon_pdev->enhanced_stats_en &&
			   !mon_pdev->tx_sniffer_enable &&
			   !mon_pdev->mcopy_mode &&
			   !mon_pdev->pktlog_ppdu_stats) {
			dp_h2t_cfg_stats_msg_send(pdev,
						  DP_PPDU_STATS_CFG_BPR_ENH,
						  pdev->pdev_id);
		} else if (mon_pdev->pktlog_ppdu_stats) {
			dp_h2t_cfg_stats_msg_send(pdev,
						  DP_PPDU_STATS_CFG_BPR_PKTLOG,
						  pdev->pdev_id);
		}
		break;
	default:
		break;
	}

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef QCA_ENHANCED_STATS_SUPPORT
/*
 * dp_process_ppdu_stats_tx_mgmtctrl_payload_tlv: Process
 * htt_ppdu_stats_tx_mgmtctrl_payload_tlv
 * @pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_tx_mgmtctrl_payload_tlv
 * @length: tlv_length
 *
 * return:QDF_STATUS_SUCCESS if nbuf as to be freed in caller
 */
QDF_STATUS
dp_process_ppdu_stats_tx_mgmtctrl_payload_tlv(struct dp_pdev *pdev,
					      qdf_nbuf_t tag_buf,
					      uint32_t ppdu_id)
{
	uint32_t *nbuf_ptr;
	uint8_t trim_size;
	size_t head_size;
	struct cdp_tx_mgmt_comp_info *ptr_mgmt_comp_info;
	uint32_t *msg_word;
	uint32_t tsf_hdr;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	if ((!mon_pdev->tx_sniffer_enable) && (!mon_pdev->mcopy_mode) &&
	    (!mon_pdev->bpr_enable) && (!mon_pdev->tx_capture_enabled))
		return QDF_STATUS_SUCCESS;

	/*
	 * get timestamp from htt_t2h_ppdu_stats_ind_hdr_t
	 */
	msg_word = (uint32_t *)qdf_nbuf_data(tag_buf);
	msg_word = msg_word + 2;
	tsf_hdr = *msg_word;

	trim_size = ((mon_pdev->mgmtctrl_frm_info.mgmt_buf +
		      HTT_MGMT_CTRL_TLV_HDR_RESERVERD_LEN) -
		      qdf_nbuf_data(tag_buf));

	if (!qdf_nbuf_pull_head(tag_buf, trim_size))
		return QDF_STATUS_SUCCESS;

	qdf_nbuf_trim_tail(tag_buf, qdf_nbuf_len(tag_buf) -
			    mon_pdev->mgmtctrl_frm_info.mgmt_buf_len);

	if (mon_pdev->tx_capture_enabled) {
		head_size = sizeof(struct cdp_tx_mgmt_comp_info);
		if (qdf_unlikely(qdf_nbuf_headroom(tag_buf) < head_size)) {
			qdf_err("Fail to get headroom h_sz %zu h_avail %d\n",
				head_size, qdf_nbuf_headroom(tag_buf));
			qdf_assert_always(0);
			return QDF_STATUS_E_NOMEM;
		}
		ptr_mgmt_comp_info = (struct cdp_tx_mgmt_comp_info *)
					qdf_nbuf_push_head(tag_buf, head_size);
		qdf_assert_always(ptr_mgmt_comp_info);
		ptr_mgmt_comp_info->ppdu_id = ppdu_id;
		ptr_mgmt_comp_info->is_sgen_pkt = true;
		ptr_mgmt_comp_info->tx_tsf = tsf_hdr;
	} else {
		head_size = sizeof(ppdu_id);
		nbuf_ptr = (uint32_t *)qdf_nbuf_push_head(tag_buf, head_size);
		*nbuf_ptr = ppdu_id;
	}
	if (mon_pdev->bpr_enable) {
		dp_wdi_event_handler(WDI_EVENT_TX_BEACON, pdev->soc,
				     tag_buf, HTT_INVALID_PEER,
				     WDI_NO_VAL, pdev->pdev_id);
	}

	dp_deliver_mgmt_frm(pdev, tag_buf);

	return QDF_STATUS_E_ALREADY;
}

/*
 * dp_htt_get_ppdu_sniffer_ampdu_tlv_bitmap() - Get ppdu stats tlv
 * bitmap for sniffer mode
 * @bitmap: received bitmap
 *
 * Return: expected bitmap value, returns zero if doesn't match with
 * either 64-bit Tx window or 256-bit window tlv bitmap
 */
int
dp_htt_get_ppdu_sniffer_ampdu_tlv_bitmap(uint32_t bitmap)
{
	if (bitmap == (HTT_PPDU_SNIFFER_AMPDU_TLV_BITMAP_64))
		return HTT_PPDU_SNIFFER_AMPDU_TLV_BITMAP_64;
	else if (bitmap == (HTT_PPDU_SNIFFER_AMPDU_TLV_BITMAP_256))
		return HTT_PPDU_SNIFFER_AMPDU_TLV_BITMAP_256;

	return 0;
}

/*
 * dp_peer_find_by_id_valid - check if peer exists for given id
 * @soc: core DP soc context
 * @peer_id: peer id from peer object can be retrieved
 *
 * Return: true if peer exists of false otherwise
 */

static
bool dp_peer_find_by_id_valid(struct dp_soc *soc, uint16_t peer_id)
{
	struct dp_peer *peer = dp_peer_get_ref_by_id(soc, peer_id,
						     DP_MOD_ID_HTT);

	if (peer) {
		/*
		 * Decrement the peer ref which is taken as part of
		 * dp_peer_get_ref_by_id if PEER_LOCK_REF_PROTECT is enabled
		 */
		dp_peer_unref_delete(peer, DP_MOD_ID_HTT);

		return true;
	}

	return false;
}

/*
 * dp_peer_copy_delay_stats() - copy ppdu stats to peer delayed stats.
 * @peer: Datapath peer handle
 * @ppdu: User PPDU Descriptor
 * @cur_ppdu_id: PPDU_ID
 *
 * Return: None
 *
 * on Tx data frame, we may get delayed ba set
 * in htt_ppdu_stats_user_common_tlv. which mean we get Block Ack(BA) after we
 * request Block Ack Request(BAR). Successful msdu is received only after Block
 * Ack. To populate peer stats we need successful msdu(data frame).
 * So we hold the Tx data stats on delayed_ba for stats update.
 */
static void
dp_peer_copy_delay_stats(struct dp_peer *peer,
			 struct cdp_tx_completion_ppdu_user *ppdu,
			 uint32_t cur_ppdu_id)
{
	struct dp_pdev *pdev;
	struct dp_vdev *vdev;
	struct dp_mon_peer *mon_peer = peer->monitor_peer;

	if (mon_peer->last_delayed_ba) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "BA not yet recv for prev delayed ppdu[%d] - cur ppdu[%d]",
			  mon_peer->last_delayed_ba_ppduid, cur_ppdu_id);
		vdev = peer->vdev;
		if (vdev) {
			pdev = vdev->pdev;
			pdev->stats.cdp_delayed_ba_not_recev++;
		}
	}

	mon_peer->delayed_ba_ppdu_stats.ltf_size = ppdu->ltf_size;
	mon_peer->delayed_ba_ppdu_stats.stbc = ppdu->stbc;
	mon_peer->delayed_ba_ppdu_stats.he_re = ppdu->he_re;
	mon_peer->delayed_ba_ppdu_stats.txbf = ppdu->txbf;
	mon_peer->delayed_ba_ppdu_stats.bw = ppdu->bw;
	mon_peer->delayed_ba_ppdu_stats.nss = ppdu->nss;
	mon_peer->delayed_ba_ppdu_stats.gi = ppdu->gi;
	mon_peer->delayed_ba_ppdu_stats.dcm = ppdu->dcm;
	mon_peer->delayed_ba_ppdu_stats.ldpc = ppdu->ldpc;
	mon_peer->delayed_ba_ppdu_stats.dcm = ppdu->dcm;
	mon_peer->delayed_ba_ppdu_stats.mpdu_tried_ucast =
					ppdu->mpdu_tried_ucast;
	mon_peer->delayed_ba_ppdu_stats.mpdu_tried_mcast =
					ppdu->mpdu_tried_mcast;
	mon_peer->delayed_ba_ppdu_stats.frame_ctrl = ppdu->frame_ctrl;
	mon_peer->delayed_ba_ppdu_stats.qos_ctrl = ppdu->qos_ctrl;
	mon_peer->delayed_ba_ppdu_stats.dcm = ppdu->dcm;

	mon_peer->delayed_ba_ppdu_stats.ru_start = ppdu->ru_start;
	mon_peer->delayed_ba_ppdu_stats.ru_tones = ppdu->ru_tones;
	mon_peer->delayed_ba_ppdu_stats.is_mcast = ppdu->is_mcast;

	mon_peer->delayed_ba_ppdu_stats.user_pos = ppdu->user_pos;
	mon_peer->delayed_ba_ppdu_stats.mu_group_id = ppdu->mu_group_id;

	mon_peer->last_delayed_ba = true;

	ppdu->debug_copied = true;
}

/*
 * dp_peer_copy_stats_to_bar() - copy delayed stats to ppdu stats.
 * @peer: Datapath peer handle
 * @ppdu: PPDU Descriptor
 *
 * Return: None
 *
 * For Tx BAR, PPDU stats TLV include Block Ack info. PPDU info
 * from Tx BAR frame not required to populate peer stats.
 * But we need successful MPDU and MSDU to update previous
 * transmitted Tx data frame. Overwrite ppdu stats with the previous
 * stored ppdu stats.
 */
static void
dp_peer_copy_stats_to_bar(struct dp_peer *peer,
			  struct cdp_tx_completion_ppdu_user *ppdu)
{
	struct dp_mon_peer *mon_peer = peer->monitor_peer;

	ppdu->ltf_size = mon_peer->delayed_ba_ppdu_stats.ltf_size;
	ppdu->stbc = mon_peer->delayed_ba_ppdu_stats.stbc;
	ppdu->he_re = mon_peer->delayed_ba_ppdu_stats.he_re;
	ppdu->txbf = mon_peer->delayed_ba_ppdu_stats.txbf;
	ppdu->bw = mon_peer->delayed_ba_ppdu_stats.bw;
	ppdu->nss = mon_peer->delayed_ba_ppdu_stats.nss;
	ppdu->gi = mon_peer->delayed_ba_ppdu_stats.gi;
	ppdu->dcm = mon_peer->delayed_ba_ppdu_stats.dcm;
	ppdu->ldpc = mon_peer->delayed_ba_ppdu_stats.ldpc;
	ppdu->dcm = mon_peer->delayed_ba_ppdu_stats.dcm;
	ppdu->mpdu_tried_ucast =
			mon_peer->delayed_ba_ppdu_stats.mpdu_tried_ucast;
	ppdu->mpdu_tried_mcast =
			mon_peer->delayed_ba_ppdu_stats.mpdu_tried_mcast;
	ppdu->frame_ctrl = mon_peer->delayed_ba_ppdu_stats.frame_ctrl;
	ppdu->qos_ctrl = mon_peer->delayed_ba_ppdu_stats.qos_ctrl;
	ppdu->dcm = mon_peer->delayed_ba_ppdu_stats.dcm;

	ppdu->ru_start = mon_peer->delayed_ba_ppdu_stats.ru_start;
	ppdu->ru_tones = mon_peer->delayed_ba_ppdu_stats.ru_tones;
	ppdu->is_mcast = mon_peer->delayed_ba_ppdu_stats.is_mcast;

	ppdu->user_pos = mon_peer->delayed_ba_ppdu_stats.user_pos;
	ppdu->mu_group_id = mon_peer->delayed_ba_ppdu_stats.mu_group_id;

	mon_peer->last_delayed_ba = false;

	ppdu->debug_copied = true;
}

/*
 * dp_tx_rate_stats_update() - Update rate per-peer statistics
 * @peer: Datapath peer handle
 * @ppdu: PPDU Descriptor
 *
 * Return: None
 */
static void
dp_tx_rate_stats_update(struct dp_peer *peer,
			struct cdp_tx_completion_ppdu_user *ppdu)
{
	uint32_t ratekbps = 0;
	uint64_t ppdu_tx_rate = 0;
	uint32_t rix;
	uint16_t ratecode = 0;

	if (!peer || !ppdu)
		return;

	if (ppdu->completion_status != HTT_PPDU_STATS_USER_STATUS_OK)
		return;

	ratekbps = dp_getrateindex(ppdu->gi,
				   ppdu->mcs,
				   ppdu->nss,
				   ppdu->preamble,
				   ppdu->bw,
				   &rix,
				   &ratecode);

	DP_STATS_UPD(peer, tx.last_tx_rate, ratekbps);

	if (!ratekbps)
		return;

	/* Calculate goodput in non-training period
	 * In training period, don't do anything as
	 * pending pkt is send as goodput.
	 */
	if ((!peer->bss_peer) && (!ppdu->sa_is_training)) {
		ppdu->sa_goodput = ((ratekbps / CDP_NUM_KB_IN_MB) *
				(CDP_PERCENT_MACRO - ppdu->current_rate_per));
	}
	ppdu->rix = rix;
	ppdu->tx_ratekbps = ratekbps;
	ppdu->tx_ratecode = ratecode;
	peer->stats.tx.avg_tx_rate =
		dp_ath_rate_lpf(peer->stats.tx.avg_tx_rate, ratekbps);
	ppdu_tx_rate = dp_ath_rate_out(peer->stats.tx.avg_tx_rate);
	DP_STATS_UPD(peer, tx.rnd_avg_tx_rate, ppdu_tx_rate);

	peer->stats.tx.bw_info = ppdu->bw;
	peer->stats.tx.gi_info = ppdu->gi;
	peer->stats.tx.nss_info = ppdu->nss;
	peer->stats.tx.mcs_info = ppdu->mcs;
	peer->stats.tx.preamble_info = ppdu->preamble;
	if (peer->vdev) {
		/*
		 * In STA mode:
		 *	We get ucast stats as BSS peer stats.
		 *
		 * In AP mode:
		 *	We get mcast stats as BSS peer stats.
		 *	We get ucast stats as assoc peer stats.
		 */
		if (peer->vdev->opmode == wlan_op_mode_ap && peer->bss_peer) {
			peer->vdev->stats.tx.mcast_last_tx_rate = ratekbps;
			peer->vdev->stats.tx.mcast_last_tx_rate_mcs = ppdu->mcs;
		} else {
			peer->vdev->stats.tx.last_tx_rate = ratekbps;
			peer->vdev->stats.tx.last_tx_rate_mcs = ppdu->mcs;
		}
	}
}

/*
 * dp_tx_stats_update() - Update per-peer statistics
 * @pdev: Datapath pdev handle
 * @peer: Datapath peer handle
 * @ppdu: PPDU Descriptor
 * @ack_rssi: RSSI of last ack received
 *
 * Return: None
 */
static void
dp_tx_stats_update(struct dp_pdev *pdev, struct dp_peer *peer,
		   struct cdp_tx_completion_ppdu_user *ppdu,
		   uint32_t ack_rssi)
{
	uint8_t preamble, mcs;
	uint16_t num_msdu;
	uint16_t num_mpdu;
	uint16_t mpdu_tried;
	uint16_t mpdu_failed;

	preamble = ppdu->preamble;
	mcs = ppdu->mcs;
	num_msdu = ppdu->num_msdu;
	num_mpdu = ppdu->mpdu_success;
	mpdu_tried = ppdu->mpdu_tried_ucast + ppdu->mpdu_tried_mcast;
	mpdu_failed = mpdu_tried - num_mpdu;

	/* If the peer statistics are already processed as part of
	 * per-MSDU completion handler, do not process these again in per-PPDU
	 * indications
	 */
	if (pdev->soc->process_tx_status)
		return;

	if (ppdu->completion_status != HTT_PPDU_STATS_USER_STATUS_OK) {
		/*
		 * All failed mpdu will be retried, so incrementing
		 * retries mpdu based on mpdu failed. Even for
		 * ack failure i.e for long retries we get
		 * mpdu failed equal mpdu tried.
		 */
		DP_STATS_INC(peer, tx.retries, mpdu_failed);
		DP_STATS_INC(peer, tx.tx_failed, ppdu->failed_msdus);
		return;
	}

	if (ppdu->is_ppdu_cookie_valid)
		DP_STATS_INC(peer, tx.num_ppdu_cookie_valid, 1);

	if (ppdu->mu_group_id <= MAX_MU_GROUP_ID &&
	    ppdu->ppdu_type != HTT_PPDU_STATS_PPDU_TYPE_SU) {
		if (unlikely(!(ppdu->mu_group_id & (MAX_MU_GROUP_ID - 1))))
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				  "mu_group_id out of bound!!\n");
		else
			DP_STATS_UPD(peer, tx.mu_group_id[ppdu->mu_group_id],
				     (ppdu->user_pos + 1));
	}

	if (ppdu->ppdu_type == HTT_PPDU_STATS_PPDU_TYPE_MU_OFDMA ||
	    ppdu->ppdu_type == HTT_PPDU_STATS_PPDU_TYPE_MU_MIMO_OFDMA) {
		DP_STATS_UPD(peer, tx.ru_tones, ppdu->ru_tones);
		DP_STATS_UPD(peer, tx.ru_start, ppdu->ru_start);
		switch (ppdu->ru_tones) {
		case RU_26:
			DP_STATS_INC(peer, tx.ru_loc[RU_26_INDEX].num_msdu,
				     num_msdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_26_INDEX].num_mpdu,
				     num_mpdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_26_INDEX].mpdu_tried,
				     mpdu_tried);
		break;
		case RU_52:
			DP_STATS_INC(peer, tx.ru_loc[RU_52_INDEX].num_msdu,
				     num_msdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_52_INDEX].num_mpdu,
				     num_mpdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_52_INDEX].mpdu_tried,
				     mpdu_tried);
		break;
		case RU_106:
			DP_STATS_INC(peer, tx.ru_loc[RU_106_INDEX].num_msdu,
				     num_msdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_106_INDEX].num_mpdu,
				     num_mpdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_106_INDEX].mpdu_tried,
				     mpdu_tried);
		break;
		case RU_242:
			DP_STATS_INC(peer, tx.ru_loc[RU_242_INDEX].num_msdu,
				     num_msdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_242_INDEX].num_mpdu,
				     num_mpdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_242_INDEX].mpdu_tried,
				     mpdu_tried);
		break;
		case RU_484:
			DP_STATS_INC(peer, tx.ru_loc[RU_484_INDEX].num_msdu,
				     num_msdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_484_INDEX].num_mpdu,
				     num_mpdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_484_INDEX].mpdu_tried,
				     mpdu_tried);
		break;
		case RU_996:
			DP_STATS_INC(peer, tx.ru_loc[RU_996_INDEX].num_msdu,
				     num_msdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_996_INDEX].num_mpdu,
				     num_mpdu);
			DP_STATS_INC(peer, tx.ru_loc[RU_996_INDEX].mpdu_tried,
				     mpdu_tried);
		break;
		}
	}

	/*
	 * All failed mpdu will be retried, so incrementing
	 * retries mpdu based on mpdu failed. Even for
	 * ack failure i.e for long retries we get
	 * mpdu failed equal mpdu tried.
	 */
	DP_STATS_INC(peer, tx.retries, mpdu_failed);
	DP_STATS_INC(peer, tx.tx_failed, ppdu->failed_msdus);

	DP_STATS_INC(peer, tx.transmit_type[ppdu->ppdu_type].num_msdu,
		     num_msdu);
	DP_STATS_INC(peer, tx.transmit_type[ppdu->ppdu_type].num_mpdu,
		     num_mpdu);
	DP_STATS_INC(peer, tx.transmit_type[ppdu->ppdu_type].mpdu_tried,
		     mpdu_tried);

	DP_STATS_UPD(peer, tx.tx_rate, ppdu->tx_rate);
	DP_STATS_INC(peer, tx.sgi_count[ppdu->gi], num_msdu);
	DP_STATS_INC(peer, tx.bw[ppdu->bw], num_msdu);
	DP_STATS_INC(peer, tx.nss[ppdu->nss], num_msdu);
	if (ppdu->tid < CDP_DATA_TID_MAX)
		DP_STATS_INC(peer, tx.wme_ac_type[TID_TO_WME_AC(ppdu->tid)],
			     num_msdu);
	DP_STATS_INCC(peer, tx.stbc, num_msdu, ppdu->stbc);
	DP_STATS_INCC(peer, tx.ldpc, num_msdu, ppdu->ldpc);
	if (!(ppdu->is_mcast) && ppdu->ack_rssi_valid)
		DP_STATS_UPD(peer, tx.last_ack_rssi, ack_rssi);

	DP_STATS_INCC(peer,
		      tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
		      ((mcs >= MAX_MCS_11A) && (preamble == DOT11_A)));
	DP_STATS_INCC(peer,
		      tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
		      ((mcs < MAX_MCS_11A) && (preamble == DOT11_A)));
	DP_STATS_INCC(peer,
		      tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
		      ((mcs >= MAX_MCS_11B) && (preamble == DOT11_B)));
	DP_STATS_INCC(peer,
		      tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
		      ((mcs < (MAX_MCS_11B)) && (preamble == DOT11_B)));
	DP_STATS_INCC(peer,
		      tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
		      ((mcs >= MAX_MCS_11A) && (preamble == DOT11_N)));
	DP_STATS_INCC(peer,
		      tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
		      ((mcs < MAX_MCS_11A) && (preamble == DOT11_N)));
	DP_STATS_INCC(peer,
		      tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
		      ((mcs >= MAX_MCS_11AC) && (preamble == DOT11_AC)));
	DP_STATS_INCC(peer,
		      tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
		      ((mcs < MAX_MCS_11AC) && (preamble == DOT11_AC)));
	DP_STATS_INCC(peer,
		      tx.pkt_type[preamble].mcs_count[MAX_MCS-1], num_msdu,
		      ((mcs >= (MAX_MCS - 1)) && (preamble == DOT11_AX)));
	DP_STATS_INCC(peer,
		      tx.pkt_type[preamble].mcs_count[mcs], num_msdu,
		      ((mcs < (MAX_MCS - 1)) && (preamble == DOT11_AX)));
	DP_STATS_INCC(peer, tx.ampdu_cnt, num_mpdu, ppdu->is_ampdu);
	DP_STATS_INCC(peer, tx.non_ampdu_cnt, num_mpdu, !(ppdu->is_ampdu));
	DP_STATS_INCC(peer, tx.pream_punct_cnt, 1, ppdu->pream_punct);

	dp_peer_stats_notify(pdev, peer);

#if defined(FEATURE_PERPKT_INFO) && WDI_EVENT_ENABLE
	dp_wdi_event_handler(WDI_EVENT_UPDATE_DP_STATS, pdev->soc,
			     &peer->stats, ppdu->peer_id,
			     UPDATE_PEER_STATS, pdev->pdev_id);
#endif
}

/*
 * dp_get_ppdu_info_user_index: Find and allocate a per-user descriptor for a PPDU,
 * if a new peer id arrives in a PPDU
 * pdev: DP pdev handle
 * @peer_id : peer unique identifier
 * @ppdu_info: per ppdu tlv structure
 *
 * return:user index to be populated
 */
static uint8_t dp_get_ppdu_info_user_index(struct dp_pdev *pdev,
					   uint16_t peer_id,
					   struct ppdu_info *ppdu_info)
{
	uint8_t user_index = 0;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;

	ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	while ((user_index + 1) <= ppdu_info->last_user) {
		ppdu_user_desc = &ppdu_desc->user[user_index];
		if (ppdu_user_desc->peer_id != peer_id) {
			user_index++;
			continue;
		} else {
			/* Max users possible is 8 so user array index should
			 * not exceed 7
			 */
			qdf_assert_always(user_index <= (ppdu_desc->max_users - 1));
			return user_index;
		}
	}

	ppdu_info->last_user++;
	/* Max users possible is 8 so last user should not exceed 8 */
	qdf_assert_always(ppdu_info->last_user <= ppdu_desc->max_users);
	return ppdu_info->last_user - 1;
}

/*
 * dp_process_ppdu_stats_common_tlv: Process htt_ppdu_stats_common_tlv
 * pdev: DP pdev handle
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_common_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void
dp_process_ppdu_stats_common_tlv(struct dp_pdev *pdev,
				 uint32_t *tag_buf,
				 struct ppdu_info *ppdu_info)
{
	uint16_t frame_type;
	uint16_t frame_ctrl;
	uint16_t freq;
	struct dp_soc *soc = NULL;
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	uint64_t ppdu_start_timestamp;
	uint32_t *start_tag_buf;

	start_tag_buf = tag_buf;
	ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	ppdu_desc->ppdu_id = ppdu_info->ppdu_id;

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(RING_ID_SCH_CMD_ID);
	ppdu_info->sched_cmdid =
		HTT_PPDU_STATS_COMMON_TLV_SCH_CMDID_GET(*tag_buf);
	ppdu_desc->num_users =
		HTT_PPDU_STATS_COMMON_TLV_NUM_USERS_GET(*tag_buf);

	qdf_assert_always(ppdu_desc->num_users <= ppdu_desc->max_users);

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(QTYPE_FRM_TYPE);
	frame_type = HTT_PPDU_STATS_COMMON_TLV_FRM_TYPE_GET(*tag_buf);
	ppdu_desc->htt_frame_type = frame_type;

	frame_ctrl = ppdu_desc->frame_ctrl;

	ppdu_desc->bar_ppdu_id = ppdu_info->ppdu_id;

	switch (frame_type) {
	case HTT_STATS_FTYPE_TIDQ_DATA_SU:
	case HTT_STATS_FTYPE_TIDQ_DATA_MU:
	case HTT_STATS_FTYPE_SGEN_QOS_NULL:
		/*
		 * for management packet, frame type come as DATA_SU
		 * need to check frame_ctrl before setting frame_type
		 */
		if (HTT_GET_FRAME_CTRL_TYPE(frame_ctrl) <= FRAME_CTRL_TYPE_CTRL)
			ppdu_desc->frame_type = CDP_PPDU_FTYPE_CTRL;
		else
			ppdu_desc->frame_type = CDP_PPDU_FTYPE_DATA;
	break;
	case HTT_STATS_FTYPE_SGEN_MU_BAR:
	case HTT_STATS_FTYPE_SGEN_BAR:
		ppdu_desc->frame_type = CDP_PPDU_FTYPE_BAR;
	break;
	default:
		ppdu_desc->frame_type = CDP_PPDU_FTYPE_CTRL;
	break;
	}

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(FES_DUR_US);
	ppdu_desc->tx_duration = *tag_buf;

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(START_TSTMP_L32_US);
	ppdu_desc->ppdu_start_timestamp = *tag_buf;

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(CHAN_MHZ_PHY_MODE);
	freq = HTT_PPDU_STATS_COMMON_TLV_CHAN_MHZ_GET(*tag_buf);
	if (freq != ppdu_desc->channel) {
		soc = pdev->soc;
		ppdu_desc->channel = freq;
		pdev->operating_channel.freq = freq;
		if (soc && soc->cdp_soc.ol_ops->freq_to_channel)
			pdev->operating_channel.num =
			    soc->cdp_soc.ol_ops->freq_to_channel(soc->ctrl_psoc,
								 pdev->pdev_id,
								 freq);

		if (soc && soc->cdp_soc.ol_ops->freq_to_band)
			pdev->operating_channel.band =
			       soc->cdp_soc.ol_ops->freq_to_band(soc->ctrl_psoc,
								 pdev->pdev_id,
								 freq);
	}

	ppdu_desc->phy_mode = HTT_PPDU_STATS_COMMON_TLV_PHY_MODE_GET(*tag_buf);

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(RESV_NUM_UL_BEAM);
	ppdu_desc->phy_ppdu_tx_time_us =
		HTT_PPDU_STATS_COMMON_TLV_PHY_PPDU_TX_TIME_US_GET(*tag_buf);
	ppdu_desc->beam_change =
		HTT_PPDU_STATS_COMMON_TLV_BEAM_CHANGE_GET(*tag_buf);
	ppdu_desc->doppler =
		HTT_PPDU_STATS_COMMON_TLV_DOPPLER_INDICATION_GET(*tag_buf);
	ppdu_desc->spatial_reuse =
		HTT_PPDU_STATS_COMMON_TLV_SPATIAL_REUSE_GET(*tag_buf);

	dp_tx_capture_htt_frame_counter(pdev, frame_type);

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(START_TSTMP_U32_US);
	ppdu_start_timestamp = *tag_buf;
	ppdu_desc->ppdu_start_timestamp |= ((ppdu_start_timestamp <<
					     HTT_SHIFT_UPPER_TIMESTAMP) &
					    HTT_MASK_UPPER_TIMESTAMP);

	ppdu_desc->ppdu_end_timestamp = ppdu_desc->ppdu_start_timestamp +
					ppdu_desc->tx_duration;
	/* Ack time stamp is same as end time stamp*/
	ppdu_desc->ack_timestamp = ppdu_desc->ppdu_end_timestamp;

	ppdu_desc->ppdu_end_timestamp = ppdu_desc->ppdu_start_timestamp +
					ppdu_desc->tx_duration;

	ppdu_desc->bar_ppdu_start_timestamp = ppdu_desc->ppdu_start_timestamp;
	ppdu_desc->bar_ppdu_end_timestamp = ppdu_desc->ppdu_end_timestamp;
	ppdu_desc->bar_tx_duration = ppdu_desc->tx_duration;

	/* Ack time stamp is same as end time stamp*/
	ppdu_desc->ack_timestamp = ppdu_desc->ppdu_end_timestamp;

	tag_buf = start_tag_buf + HTT_GET_STATS_CMN_INDEX(BSSCOLOR_OBSS_PSR);
	ppdu_desc->bss_color =
		HTT_PPDU_STATS_COMMON_TLV_BSS_COLOR_ID_GET(*tag_buf);
}

/*
 * dp_process_ppdu_stats_user_common_tlv: Process ppdu_stats_user_common
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_user_common_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_common_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint16_t peer_id;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	struct dp_peer *peer;
	struct dp_vdev *vdev;
	uint32_t tlv_type = HTT_STATS_TLV_TAG_GET(*tag_buf);

	ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;
	peer_id = HTT_PPDU_STATS_USER_RATE_TLV_SW_PEER_ID_GET(*tag_buf);

	curr_user_index =
		dp_get_ppdu_info_user_index(pdev,
					    peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->tlv_bitmap |= (1 << tlv_type);

	ppdu_desc->vdev_id =
		HTT_PPDU_STATS_USER_COMMON_TLV_VAP_ID_GET(*tag_buf);

	ppdu_user_desc->peer_id = peer_id;

	tag_buf++;

	if (HTT_PPDU_STATS_USER_COMMON_TLV_DELAYED_BA_GET(*tag_buf)) {
		ppdu_user_desc->delayed_ba = 1;
		ppdu_desc->delayed_ba = 1;
	}

	if (HTT_PPDU_STATS_USER_COMMON_TLV_MCAST_GET(*tag_buf)) {
		ppdu_user_desc->is_mcast = true;
		ppdu_user_desc->mpdu_tried_mcast =
		HTT_PPDU_STATS_USER_COMMON_TLV_MPDUS_TRIED_GET(*tag_buf);
		ppdu_user_desc->num_mpdu = ppdu_user_desc->mpdu_tried_mcast;
	} else {
		ppdu_user_desc->mpdu_tried_ucast =
		HTT_PPDU_STATS_USER_COMMON_TLV_MPDUS_TRIED_GET(*tag_buf);
	}

	ppdu_user_desc->is_seq_num_valid =
	HTT_PPDU_STATS_USER_COMMON_TLV_IS_SQNUM_VALID_IN_BUFFER_GET(*tag_buf);
	tag_buf++;

	ppdu_user_desc->qos_ctrl =
		HTT_PPDU_STATS_USER_COMMON_TLV_QOS_CTRL_GET(*tag_buf);
	ppdu_user_desc->frame_ctrl =
		HTT_PPDU_STATS_USER_COMMON_TLV_FRAME_CTRL_GET(*tag_buf);
	ppdu_desc->frame_ctrl = ppdu_user_desc->frame_ctrl;

	if (ppdu_user_desc->delayed_ba)
		ppdu_user_desc->mpdu_success = 0;

	tag_buf += 3;

	if (HTT_PPDU_STATS_IS_OPAQUE_VALID_GET(*tag_buf)) {
		ppdu_user_desc->ppdu_cookie =
			HTT_PPDU_STATS_HOST_OPAQUE_COOKIE_GET(*tag_buf);
		ppdu_user_desc->is_ppdu_cookie_valid = 1;
	}

	/* returning earlier causes other feilds unpopulated */
	if (peer_id == DP_SCAN_PEER_ID) {
		vdev = dp_vdev_get_ref_by_id(pdev->soc, ppdu_desc->vdev_id,
					     DP_MOD_ID_TX_PPDU_STATS);
		if (!vdev)
			return;
		qdf_mem_copy(ppdu_user_desc->mac_addr, vdev->mac_addr.raw,
			     QDF_MAC_ADDR_SIZE);
		dp_vdev_unref_delete(pdev->soc, vdev, DP_MOD_ID_TX_PPDU_STATS);
	} else {
		peer = dp_peer_get_ref_by_id(pdev->soc, peer_id,
					     DP_MOD_ID_TX_PPDU_STATS);
		if (!peer) {
			/*
			 * fw sends peer_id which is about to removed but
			 * it was already removed in host.
			 * eg: for disassoc, fw send ppdu stats
			 * with peer id equal to previously associated
			 * peer's peer_id but it was removed
			 */
			vdev = dp_vdev_get_ref_by_id(pdev->soc,
						     ppdu_desc->vdev_id,
						     DP_MOD_ID_TX_PPDU_STATS);
			if (!vdev)
				return;
			qdf_mem_copy(ppdu_user_desc->mac_addr,
				     vdev->mac_addr.raw, QDF_MAC_ADDR_SIZE);
			dp_vdev_unref_delete(pdev->soc, vdev,
					     DP_MOD_ID_TX_PPDU_STATS);
			return;
		}
		qdf_mem_copy(ppdu_user_desc->mac_addr,
			     peer->mac_addr.raw, QDF_MAC_ADDR_SIZE);
		dp_peer_unref_delete(peer, DP_MOD_ID_TX_PPDU_STATS);
	}
}

/**
 * dp_process_ppdu_stats_user_rate_tlv() - Process htt_ppdu_stats_user_rate_tlv
 * @pdev: DP pdev handle
 * @tag_buf: T2H message buffer carrying the user rate TLV
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void
dp_process_ppdu_stats_user_rate_tlv(struct dp_pdev *pdev,
				    uint32_t *tag_buf,
				    struct ppdu_info *ppdu_info)
{
	uint16_t peer_id;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	struct dp_vdev *vdev;
	uint32_t tlv_type = HTT_STATS_TLV_TAG_GET(*tag_buf);

	ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;
	peer_id = HTT_PPDU_STATS_USER_RATE_TLV_SW_PEER_ID_GET(*tag_buf);

	curr_user_index =
		dp_get_ppdu_info_user_index(pdev,
					    peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->tlv_bitmap |= (1 << tlv_type);
	if (peer_id == DP_SCAN_PEER_ID) {
		vdev = dp_vdev_get_ref_by_id(pdev->soc, ppdu_desc->vdev_id,
					     DP_MOD_ID_TX_PPDU_STATS);
		if (!vdev)
			return;
		dp_vdev_unref_delete(pdev->soc, vdev,
				     DP_MOD_ID_TX_PPDU_STATS);
	}
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->tid =
		HTT_PPDU_STATS_USER_RATE_TLV_TID_NUM_GET(*tag_buf);

	tag_buf += 1;

	ppdu_user_desc->user_pos =
		HTT_PPDU_STATS_USER_RATE_TLV_USER_POS_GET(*tag_buf);
	ppdu_user_desc->mu_group_id =
		HTT_PPDU_STATS_USER_RATE_TLV_MU_GROUPID_GET(*tag_buf);

	tag_buf += 1;

	ppdu_user_desc->ru_start =
		HTT_PPDU_STATS_USER_RATE_TLV_RU_START_GET(*tag_buf);
	ppdu_user_desc->ru_tones =
		(HTT_PPDU_STATS_USER_RATE_TLV_RU_END_GET(*tag_buf) -
		HTT_PPDU_STATS_USER_RATE_TLV_RU_START_GET(*tag_buf)) + 1;
	ppdu_desc->usr_ru_tones_sum += ppdu_user_desc->ru_tones;

	tag_buf += 2;

	ppdu_user_desc->ppdu_type =
		HTT_PPDU_STATS_USER_RATE_TLV_PPDU_TYPE_GET(*tag_buf);

	tag_buf++;
	ppdu_user_desc->tx_rate = *tag_buf;

	ppdu_user_desc->ltf_size =
		HTT_PPDU_STATS_USER_RATE_TLV_LTF_SIZE_GET(*tag_buf);
	ppdu_user_desc->stbc =
		HTT_PPDU_STATS_USER_RATE_TLV_STBC_GET(*tag_buf);
	ppdu_user_desc->he_re =
		HTT_PPDU_STATS_USER_RATE_TLV_HE_RE_GET(*tag_buf);
	ppdu_user_desc->txbf =
		HTT_PPDU_STATS_USER_RATE_TLV_TXBF_GET(*tag_buf);
	ppdu_user_desc->bw =
		HTT_PPDU_STATS_USER_RATE_TLV_BW_GET(*tag_buf) - 2;
	ppdu_user_desc->nss = HTT_PPDU_STATS_USER_RATE_TLV_NSS_GET(*tag_buf);
	ppdu_desc->usr_nss_sum += ppdu_user_desc->nss;
	ppdu_user_desc->mcs = HTT_PPDU_STATS_USER_RATE_TLV_MCS_GET(*tag_buf);
	ppdu_user_desc->preamble =
		HTT_PPDU_STATS_USER_RATE_TLV_PREAMBLE_GET(*tag_buf);
	ppdu_user_desc->gi = HTT_PPDU_STATS_USER_RATE_TLV_GI_GET(*tag_buf);
	ppdu_user_desc->dcm = HTT_PPDU_STATS_USER_RATE_TLV_DCM_GET(*tag_buf);
	ppdu_user_desc->ldpc = HTT_PPDU_STATS_USER_RATE_TLV_LDPC_GET(*tag_buf);
}

/*
 * dp_process_ppdu_stats_enq_mpdu_bitmap_64_tlv: Process
 * htt_ppdu_stats_enq_mpdu_bitmap_64_tlv
 * pdev: DP PDEV handle
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_enq_mpdu_bitmap_64_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_enq_mpdu_bitmap_64_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	htt_ppdu_stats_enq_mpdu_bitmap_64_tlv *dp_stats_buf =
		(htt_ppdu_stats_enq_mpdu_bitmap_64_tlv *)tag_buf;

	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	uint16_t peer_id;
	uint32_t size = CDP_BA_64_BIT_MAP_SIZE_DWORDS;
	uint32_t tlv_type = HTT_STATS_TLV_TAG_GET(*tag_buf);

	ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;

	peer_id =
	HTT_PPDU_STATS_ENQ_MPDU_BITMAP_TLV_SW_PEER_ID_GET(*tag_buf);

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->tlv_bitmap |= (1 << tlv_type);
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->start_seq = dp_stats_buf->start_seq;
	qdf_mem_copy(&ppdu_user_desc->enq_bitmap, &dp_stats_buf->enq_bitmap,
		     sizeof(uint32_t) * CDP_BA_64_BIT_MAP_SIZE_DWORDS);

	dp_process_ppdu_stats_update_failed_bitmap(pdev,
						   (void *)ppdu_user_desc,
						   ppdu_info->ppdu_id,
						   size);
}

/*
 * dp_process_ppdu_stats_enq_mpdu_bitmap_256_tlv: Process
 * htt_ppdu_stats_enq_mpdu_bitmap_256_tlv
 * soc: DP SOC handle
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_enq_mpdu_bitmap_256_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_enq_mpdu_bitmap_256_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	htt_ppdu_stats_enq_mpdu_bitmap_256_tlv *dp_stats_buf =
		(htt_ppdu_stats_enq_mpdu_bitmap_256_tlv *)tag_buf;

	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	uint16_t peer_id;
	uint32_t size = CDP_BA_256_BIT_MAP_SIZE_DWORDS;
	uint32_t tlv_type = HTT_STATS_TLV_TAG_GET(*tag_buf);

	ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;

	peer_id =
	HTT_PPDU_STATS_ENQ_MPDU_BITMAP_TLV_SW_PEER_ID_GET(*tag_buf);

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->tlv_bitmap |= (1 << tlv_type);
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->start_seq = dp_stats_buf->start_seq;
	qdf_mem_copy(&ppdu_user_desc->enq_bitmap, &dp_stats_buf->enq_bitmap,
		     sizeof(uint32_t) * CDP_BA_256_BIT_MAP_SIZE_DWORDS);

	dp_process_ppdu_stats_update_failed_bitmap(pdev,
						   (void *)ppdu_user_desc,
						   ppdu_info->ppdu_id,
						   size);
}

/*
 * dp_process_ppdu_stats_user_cmpltn_common_tlv: Process
 * htt_ppdu_stats_user_cmpltn_common_tlv
 * soc: DP SOC handle
 * @tag_buf: buffer containing the tlv htt_ppdu_stats_user_cmpltn_common_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_cmpltn_common_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint16_t peer_id;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	uint8_t bw_iter;
	htt_ppdu_stats_user_cmpltn_common_tlv *dp_stats_buf =
		(htt_ppdu_stats_user_cmpltn_common_tlv *)tag_buf;
	uint32_t tlv_type = HTT_STATS_TLV_TAG_GET(*tag_buf);

	ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;
	peer_id =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_SW_PEER_ID_GET(*tag_buf);

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->tlv_bitmap |= (1 << tlv_type);
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->completion_status =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_COMPLETION_STATUS_GET(
				*tag_buf);

	ppdu_user_desc->tid =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_TID_NUM_GET(*tag_buf);

	tag_buf++;
	if (qdf_likely(ppdu_user_desc->completion_status ==
			HTT_PPDU_STATS_USER_STATUS_OK)) {
		ppdu_desc->ack_rssi = dp_stats_buf->ack_rssi;
		ppdu_user_desc->usr_ack_rssi = dp_stats_buf->ack_rssi;
		ppdu_user_desc->ack_rssi_valid = 1;
	} else {
		ppdu_user_desc->ack_rssi_valid = 0;
	}

	tag_buf++;

	ppdu_user_desc->mpdu_success =
	HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_MPDU_SUCCESS_GET(*tag_buf);

	ppdu_user_desc->mpdu_failed =
	HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_MPDU_TRIED_GET(*tag_buf) -
						ppdu_user_desc->mpdu_success;

	tag_buf++;

	ppdu_user_desc->long_retries =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_LONG_RETRY_GET(*tag_buf);

	ppdu_user_desc->short_retries =
	HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_SHORT_RETRY_GET(*tag_buf);
	ppdu_user_desc->retry_msdus =
		ppdu_user_desc->long_retries + ppdu_user_desc->short_retries;

	ppdu_user_desc->is_ampdu =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_IS_AMPDU_GET(*tag_buf);
	ppdu_info->is_ampdu = ppdu_user_desc->is_ampdu;

	ppdu_desc->resp_type =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_RESP_TYPE_GET(*tag_buf);
	ppdu_desc->mprot_type =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_MPROT_TYPE_GET(*tag_buf);
	ppdu_desc->rts_success =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_RTS_SUCCESS_GET(*tag_buf);
	ppdu_desc->rts_failure =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_RTS_FAILURE_GET(*tag_buf);
	ppdu_user_desc->pream_punct =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_PREAM_PUNC_TX_GET(*tag_buf);

	ppdu_info->compltn_common_tlv++;

	/*
	 * MU BAR may send request to n users but we may received ack only from
	 * m users. To have count of number of users respond back, we have a
	 * separate counter bar_num_users per PPDU that get increment for every
	 * htt_ppdu_stats_user_cmpltn_common_tlv
	 */
	ppdu_desc->bar_num_users++;

	tag_buf++;
	for (bw_iter = 0; bw_iter < CDP_RSSI_CHAIN_LEN; bw_iter++) {
		ppdu_user_desc->rssi_chain[bw_iter] =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_CHAIN_RSSI_GET(*tag_buf);
		tag_buf++;
	}

	ppdu_user_desc->sa_tx_antenna =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_TX_ANTENNA_MASK_GET(*tag_buf);

	tag_buf++;
	ppdu_user_desc->sa_is_training =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_IS_TRAINING_GET(*tag_buf);
	if (ppdu_user_desc->sa_is_training) {
		ppdu_user_desc->sa_goodput =
			HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_PENDING_TRAINING_PKTS_GET(*tag_buf);
	}

	tag_buf++;
	for (bw_iter = 0; bw_iter < CDP_NUM_SA_BW; bw_iter++) {
		ppdu_user_desc->sa_max_rates[bw_iter] =
			HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_MAX_RATES_GET(tag_buf[bw_iter]);
	}

	tag_buf += CDP_NUM_SA_BW;
	ppdu_user_desc->current_rate_per =
		HTT_PPDU_STATS_USER_CMPLTN_COMMON_TLV_CURRENT_RATE_PER_GET(*tag_buf);
}

/*
 * dp_process_ppdu_stats_user_compltn_ba_bitmap_64_tlv: Process
 * htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv
 * pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_compltn_ba_bitmap_64_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv *dp_stats_buf =
		(htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv *)tag_buf;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	uint8_t curr_user_index = 0;
	uint16_t peer_id;
	uint32_t tlv_type = HTT_STATS_TLV_TAG_GET(*tag_buf);

	ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;

	peer_id =
	HTT_PPDU_STATS_USER_CMPLTN_BA_BITMAP_TLV_SW_PEER_ID_GET(*tag_buf);

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->tlv_bitmap |= (1 << tlv_type);
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->ba_seq_no = dp_stats_buf->ba_seq_no;
	qdf_mem_copy(&ppdu_user_desc->ba_bitmap, &dp_stats_buf->ba_bitmap,
		     sizeof(uint32_t) * CDP_BA_64_BIT_MAP_SIZE_DWORDS);
	ppdu_user_desc->ba_size = CDP_BA_64_BIT_MAP_SIZE_DWORDS * 32;
}

/*
 * dp_process_ppdu_stats_user_compltn_ba_bitmap_256_tlv: Process
 * htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv
 * pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_compltn_ba_bitmap_256_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv *dp_stats_buf =
		(htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv *)tag_buf;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	uint8_t curr_user_index = 0;
	uint16_t peer_id;
	uint32_t tlv_type = HTT_STATS_TLV_TAG_GET(*tag_buf);

	ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;

	peer_id =
	HTT_PPDU_STATS_USER_CMPLTN_BA_BITMAP_TLV_SW_PEER_ID_GET(*tag_buf);

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->tlv_bitmap |= (1 << tlv_type);
	ppdu_user_desc->peer_id = peer_id;

	ppdu_user_desc->ba_seq_no = dp_stats_buf->ba_seq_no;
	qdf_mem_copy(&ppdu_user_desc->ba_bitmap, &dp_stats_buf->ba_bitmap,
		     sizeof(uint32_t) * CDP_BA_256_BIT_MAP_SIZE_DWORDS);
	ppdu_user_desc->ba_size = CDP_BA_256_BIT_MAP_SIZE_DWORDS * 32;
}

/*
 * dp_process_ppdu_stats_user_compltn_ack_ba_status_tlv: Process
 * htt_ppdu_stats_user_compltn_ack_ba_status_tlv
 * pdev: DP PDE handle
 * @tag_buf: buffer containing the htt_ppdu_stats_user_compltn_ack_ba_status_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_compltn_ack_ba_status_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint16_t peer_id;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	uint32_t tlv_type = HTT_STATS_TLV_TAG_GET(*tag_buf);

	ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf += 2;
	peer_id =
	HTT_PPDU_STATS_USER_CMPLTN_ACK_BA_STATUS_TLV_SW_PEER_ID_GET(*tag_buf);

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);
	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->tlv_bitmap |= (1 << tlv_type);
	if (!ppdu_user_desc->ack_ba_tlv) {
		ppdu_user_desc->ack_ba_tlv = 1;
	} else {
		pdev->stats.ack_ba_comes_twice++;
		return;
	}

	ppdu_user_desc->peer_id = peer_id;

	tag_buf++;
	/* not to update ppdu_desc->tid from this TLV */
	ppdu_user_desc->num_mpdu =
		HTT_PPDU_STATS_USER_CMPLTN_ACK_BA_STATUS_TLV_NUM_MPDU_GET(*tag_buf);

	ppdu_user_desc->num_msdu =
		HTT_PPDU_STATS_USER_CMPLTN_ACK_BA_STATUS_TLV_NUM_MSDU_GET(*tag_buf);

	ppdu_user_desc->success_msdus = ppdu_user_desc->num_msdu;

	tag_buf++;
	ppdu_user_desc->start_seq =
		HTT_PPDU_STATS_USER_CMPLTN_ACK_BA_STATUS_TLV_START_SEQ_GET(
			*tag_buf);

	tag_buf++;
	ppdu_user_desc->success_bytes = *tag_buf;

	/* increase ack ba tlv counter on successful mpdu */
	if (ppdu_user_desc->num_mpdu)
		ppdu_info->ack_ba_tlv++;

	if (ppdu_user_desc->ba_size == 0) {
		ppdu_user_desc->ba_seq_no = ppdu_user_desc->start_seq;
		ppdu_user_desc->ba_bitmap[0] = 1;
		ppdu_user_desc->ba_size = 1;
	}
}

/*
 * dp_process_ppdu_stats_user_common_array_tlv: Process
 * htt_ppdu_stats_user_common_array_tlv
 * pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_user_compltn_ack_ba_status_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void dp_process_ppdu_stats_user_common_array_tlv(
		struct dp_pdev *pdev, uint32_t *tag_buf,
		struct ppdu_info *ppdu_info)
{
	uint32_t peer_id;
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct cdp_tx_completion_ppdu_user *ppdu_user_desc;
	uint8_t curr_user_index = 0;
	struct htt_tx_ppdu_stats_info *dp_stats_buf;
	uint32_t tlv_type = HTT_STATS_TLV_TAG_GET(*tag_buf);

	ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);

	tag_buf++;
	dp_stats_buf = (struct htt_tx_ppdu_stats_info *)tag_buf;
	tag_buf += 3;
	peer_id =
		HTT_PPDU_STATS_ARRAY_ITEM_TLV_PEERID_GET(*tag_buf);

	if (!dp_peer_find_by_id_valid(pdev->soc, peer_id)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Invalid peer");
		return;
	}

	curr_user_index = dp_get_ppdu_info_user_index(pdev, peer_id, ppdu_info);

	ppdu_user_desc = &ppdu_desc->user[curr_user_index];
	ppdu_user_desc->tlv_bitmap |= (1 << tlv_type);

	ppdu_user_desc->retry_bytes = dp_stats_buf->tx_retry_bytes;
	ppdu_user_desc->failed_bytes = dp_stats_buf->tx_failed_bytes;

	tag_buf++;

	ppdu_user_desc->success_msdus =
		HTT_PPDU_STATS_ARRAY_ITEM_TLV_TX_SUCC_MSDUS_GET(*tag_buf);
	ppdu_user_desc->retry_bytes =
		HTT_PPDU_STATS_ARRAY_ITEM_TLV_TX_RETRY_MSDUS_GET(*tag_buf);
	tag_buf++;
	ppdu_user_desc->failed_msdus =
		HTT_PPDU_STATS_ARRAY_ITEM_TLV_TX_FAILED_MSDUS_GET(*tag_buf);
}

/*
 * dp_process_ppdu_stats_flush_tlv: Process
 * htt_ppdu_stats_flush_tlv
 * @pdev: DP PDEV handle
 * @tag_buf: buffer containing the htt_ppdu_stats_flush_tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void
dp_process_ppdu_stats_user_compltn_flush_tlv(struct dp_pdev *pdev,
					     uint32_t *tag_buf,
					     struct ppdu_info *ppdu_info)
{
	struct cdp_tx_completion_ppdu *ppdu_desc;
	uint32_t peer_id;
	uint8_t tid;
	struct dp_peer *peer;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)
				qdf_nbuf_data(ppdu_info->nbuf);
	ppdu_desc->is_flush = 1;

	tag_buf++;
	ppdu_desc->drop_reason = *tag_buf;

	tag_buf++;
	ppdu_desc->num_msdu = HTT_PPDU_STATS_FLUSH_TLV_NUM_MSDU_GET(*tag_buf);
	ppdu_desc->num_mpdu = HTT_PPDU_STATS_FLUSH_TLV_NUM_MPDU_GET(*tag_buf);
	ppdu_desc->flow_type = HTT_PPDU_STATS_FLUSH_TLV_FLOW_TYPE_GET(*tag_buf);

	tag_buf++;
	peer_id = HTT_PPDU_STATS_FLUSH_TLV_SW_PEER_ID_GET(*tag_buf);
	tid = HTT_PPDU_STATS_FLUSH_TLV_TID_NUM_GET(*tag_buf);

	ppdu_desc->num_users = 1;
	ppdu_desc->user[0].peer_id = peer_id;
	ppdu_desc->user[0].tid = tid;

	ppdu_desc->queue_type =
			HTT_PPDU_STATS_FLUSH_TLV_QUEUE_TYPE_GET(*tag_buf);

	peer = dp_peer_get_ref_by_id(pdev->soc, peer_id,
				     DP_MOD_ID_TX_PPDU_STATS);
	if (!peer)
		goto add_ppdu_to_sched_list;

	if (ppdu_desc->drop_reason == HTT_FLUSH_EXCESS_RETRIES) {
		DP_STATS_INC(peer,
			     tx.excess_retries_per_ac[TID_TO_WME_AC(tid)],
			     ppdu_desc->num_msdu);
	}

	dp_peer_unref_delete(peer, DP_MOD_ID_TX_PPDU_STATS);

add_ppdu_to_sched_list:
	ppdu_info->done = 1;
	TAILQ_REMOVE(&mon_pdev->ppdu_info_list, ppdu_info, ppdu_info_list_elem);
	mon_pdev->list_depth--;
	TAILQ_INSERT_TAIL(&mon_pdev->sched_comp_ppdu_list, ppdu_info,
			  ppdu_info_list_elem);
	mon_pdev->sched_comp_list_depth++;
}

/**
 * dp_process_ppdu_stats_sch_cmd_status_tlv: Process schedule command status tlv
 * Here we are not going to process the buffer.
 * @pdev: DP PDEV handle
 * @ppdu_info: per ppdu tlv structure
 *
 * return:void
 */
static void
dp_process_ppdu_stats_sch_cmd_status_tlv(struct dp_pdev *pdev,
					 struct ppdu_info *ppdu_info)
{
	struct cdp_tx_completion_ppdu *ppdu_desc;
	struct dp_peer *peer;
	uint8_t num_users;
	uint8_t i;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)
				qdf_nbuf_data(ppdu_info->nbuf);

	num_users = ppdu_desc->bar_num_users;

	for (i = 0; i < num_users; i++) {
		if (ppdu_desc->user[i].user_pos == 0) {
			if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_BAR) {
				/* update phy mode for bar frame */
				ppdu_desc->phy_mode =
					ppdu_desc->user[i].preamble;
				ppdu_desc->user[0].mcs = ppdu_desc->user[i].mcs;
				break;
			}
			if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_CTRL) {
				ppdu_desc->frame_ctrl =
					ppdu_desc->user[i].frame_ctrl;
				break;
			}
		}
	}

	if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_DATA &&
	    ppdu_desc->delayed_ba) {
		qdf_assert_always(ppdu_desc->num_users <= ppdu_desc->max_users);

		for (i = 0; i < ppdu_desc->num_users; i++) {
			struct cdp_delayed_tx_completion_ppdu_user *delay_ppdu;
			uint64_t start_tsf;
			uint64_t end_tsf;
			uint32_t ppdu_id;
			struct dp_mon_peer *mon_peer;

			ppdu_id = ppdu_desc->ppdu_id;
			peer = dp_peer_get_ref_by_id
				(pdev->soc, ppdu_desc->user[i].peer_id,
				 DP_MOD_ID_TX_PPDU_STATS);
			/**
			 * This check is to make sure peer is not deleted
			 * after processing the TLVs.
			 */
			if (!peer)
				continue;

			mon_peer = peer->monitor_peer;
			delay_ppdu = &mon_peer->delayed_ba_ppdu_stats;
			start_tsf = ppdu_desc->ppdu_start_timestamp;
			end_tsf = ppdu_desc->ppdu_end_timestamp;
			/**
			 * save delayed ba user info
			 */
			if (ppdu_desc->user[i].delayed_ba) {
				dp_peer_copy_delay_stats(peer,
							 &ppdu_desc->user[i],
							 ppdu_id);
				mon_peer->last_delayed_ba_ppduid = ppdu_id;
				delay_ppdu->ppdu_start_timestamp = start_tsf;
				delay_ppdu->ppdu_end_timestamp = end_tsf;
			}
			ppdu_desc->user[i].peer_last_delayed_ba =
				mon_peer->last_delayed_ba;

			dp_peer_unref_delete(peer, DP_MOD_ID_TX_PPDU_STATS);

			if (ppdu_desc->user[i].delayed_ba &&
			    !ppdu_desc->user[i].debug_copied) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					  QDF_TRACE_LEVEL_INFO_MED,
					  "%s: %d ppdu_id[%d] bar_ppdu_id[%d] num_users[%d] usr[%d] htt_frame_type[%d]\n",
					  __func__, __LINE__,
					  ppdu_desc->ppdu_id,
					  ppdu_desc->bar_ppdu_id,
					  ppdu_desc->num_users,
					  i,
					  ppdu_desc->htt_frame_type);
			}
		}
	}

	/*
	 * when frame type is BAR and STATS_COMMON_TLV is set
	 * copy the store peer delayed info to BAR status
	 */
	if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_BAR) {
		for (i = 0; i < ppdu_desc->bar_num_users; i++) {
			struct cdp_delayed_tx_completion_ppdu_user *delay_ppdu;
			uint64_t start_tsf;
			uint64_t end_tsf;
			struct dp_mon_peer *mon_peer;

			peer = dp_peer_get_ref_by_id
				(pdev->soc,
				 ppdu_desc->user[i].peer_id,
				 DP_MOD_ID_TX_PPDU_STATS);
			/**
			 * This check is to make sure peer is not deleted
			 * after processing the TLVs.
			 */
			if (!peer)
				continue;

			mon_peer = peer->monitor_peer;
			if (ppdu_desc->user[i].completion_status !=
			    HTT_PPDU_STATS_USER_STATUS_OK) {
				dp_peer_unref_delete(peer,
						     DP_MOD_ID_TX_PPDU_STATS);
				continue;
			}

			delay_ppdu = &mon_peer->delayed_ba_ppdu_stats;
			start_tsf = delay_ppdu->ppdu_start_timestamp;
			end_tsf = delay_ppdu->ppdu_end_timestamp;

			if (mon_peer->last_delayed_ba) {
				dp_peer_copy_stats_to_bar(peer,
							  &ppdu_desc->user[i]);
				ppdu_desc->ppdu_id =
					mon_peer->last_delayed_ba_ppduid;
				ppdu_desc->ppdu_start_timestamp = start_tsf;
				ppdu_desc->ppdu_end_timestamp = end_tsf;
			}
			ppdu_desc->user[i].peer_last_delayed_ba =
						mon_peer->last_delayed_ba;
			dp_peer_unref_delete(peer, DP_MOD_ID_TX_PPDU_STATS);
		}
	}

	TAILQ_REMOVE(&mon_pdev->ppdu_info_list, ppdu_info, ppdu_info_list_elem);
	mon_pdev->list_depth--;
	TAILQ_INSERT_TAIL(&mon_pdev->sched_comp_ppdu_list, ppdu_info,
			  ppdu_info_list_elem);
	mon_pdev->sched_comp_list_depth++;
}

/**
 * dp_validate_fix_ppdu_tlv(): Function to validate the length of PPDU
 *
 * If the TLV length sent as part of PPDU TLV is less that expected size i.e
 * size of corresponding data structure, pad the remaining bytes with zeros
 * and continue processing the TLVs
 *
 * @pdev: DP pdev handle
 * @tag_buf: TLV buffer
 * @tlv_expected_size: Expected size of Tag
 * @tlv_len: TLV length received from FW
 *
 * Return: Pointer to updated TLV
 */
static inline uint32_t *dp_validate_fix_ppdu_tlv(struct dp_pdev *pdev,
						 uint32_t *tag_buf,
						 uint16_t tlv_expected_size,
						 uint16_t tlv_len)
{
	uint32_t *tlv_desc = tag_buf;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	qdf_assert_always(tlv_len != 0);

	if (tlv_len < tlv_expected_size) {
		qdf_mem_zero(mon_pdev->ppdu_tlv_buf, tlv_expected_size);
		qdf_mem_copy(mon_pdev->ppdu_tlv_buf, tag_buf, tlv_len);
		tlv_desc = mon_pdev->ppdu_tlv_buf;
	}

	return tlv_desc;
}

/**
 * dp_process_ppdu_tag(): Function to process the PPDU TLVs
 * @pdev: DP pdev handle
 * @tag_buf: TLV buffer
 * @tlv_len: length of tlv
 * @ppdu_info: per ppdu tlv structure
 *
 * return: void
 */
static void dp_process_ppdu_tag(struct dp_pdev *pdev,
				uint32_t *tag_buf,
				uint32_t tlv_len,
				struct ppdu_info *ppdu_info)
{
	uint32_t tlv_type = HTT_STATS_TLV_TAG_GET(*tag_buf);
	uint16_t tlv_expected_size;
	uint32_t *tlv_desc;

	switch (tlv_type) {
	case HTT_PPDU_STATS_COMMON_TLV:
		tlv_expected_size = sizeof(htt_ppdu_stats_common_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_common_tlv(pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMMON_TLV:
		tlv_expected_size = sizeof(htt_ppdu_stats_user_common_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_common_tlv(pdev, tlv_desc,
						      ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_RATE_TLV:
		tlv_expected_size = sizeof(htt_ppdu_stats_user_rate_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_rate_tlv(pdev, tlv_desc,
						    ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_MPDU_ENQ_BITMAP_64_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_enq_mpdu_bitmap_64_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_enq_mpdu_bitmap_64_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_MPDU_ENQ_BITMAP_256_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_enq_mpdu_bitmap_256_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_enq_mpdu_bitmap_256_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_COMMON_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_user_cmpltn_common_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_cmpltn_common_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_64_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_user_compltn_ba_bitmap_64_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_compltn_ba_bitmap_64_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_256_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_user_compltn_ba_bitmap_256_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_compltn_ba_bitmap_256_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_user_compltn_ack_ba_status_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_compltn_ack_ba_status_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMMON_ARRAY_TLV:
		tlv_expected_size =
			sizeof(htt_ppdu_stats_usr_common_array_tlv_v);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_common_array_tlv(
				pdev, tlv_desc, ppdu_info);
		break;
	case HTT_PPDU_STATS_USR_COMPLTN_FLUSH_TLV:
		tlv_expected_size = sizeof(htt_ppdu_stats_flush_tlv);
		tlv_desc = dp_validate_fix_ppdu_tlv(pdev, tag_buf,
						    tlv_expected_size, tlv_len);
		dp_process_ppdu_stats_user_compltn_flush_tlv(pdev, tlv_desc,
							     ppdu_info);
		break;
	case HTT_PPDU_STATS_SCH_CMD_STATUS_TLV:
		dp_process_ppdu_stats_sch_cmd_status_tlv(pdev, ppdu_info);
		break;
	default:
		break;
	}
}

#ifdef WLAN_ATF_ENABLE
static void
dp_ppdu_desc_user_phy_tx_time_update(struct dp_pdev *pdev,
				     struct cdp_tx_completion_ppdu *ppdu_desc,
				     struct cdp_tx_completion_ppdu_user *user)
{
	uint32_t nss_ru_width_sum = 0;
	struct dp_mon_pdev *mon_pdev = NULL;

	if (!pdev || !ppdu_desc || !user)
		return;

	mon_pdev = pdev->monitor_pdev;

	if (!mon_pdev || !mon_pdev->dp_atf_stats_enable)
		return;

	if (ppdu_desc->frame_type != CDP_PPDU_FTYPE_DATA)
		return;

	nss_ru_width_sum = ppdu_desc->usr_nss_sum * ppdu_desc->usr_ru_tones_sum;
	if (!nss_ru_width_sum)
		nss_ru_width_sum = 1;

	/**
	 * For SU-MIMO PPDU phy Tx time is same for the single user.
	 * For MU-MIMO phy Tx time is calculated per user as below
	 *     user phy tx time =
	 *           Entire PPDU duration * MU Ratio * OFDMA Ratio
	 *     MU Ratio = usr_nss / Sum_of_nss_of_all_users
	 *     OFDMA_ratio = usr_ru_width / Sum_of_ru_width_of_all_users
	 *     usr_ru_widt = ru_end – ru_start + 1
	 */
	if (ppdu_desc->htt_frame_type == HTT_STATS_FTYPE_TIDQ_DATA_SU) {
		user->phy_tx_time_us = ppdu_desc->phy_ppdu_tx_time_us;
	} else {
		user->phy_tx_time_us = (ppdu_desc->phy_ppdu_tx_time_us *
				user->nss * user->ru_tones) / nss_ru_width_sum;
	}
}
#else
static void
dp_ppdu_desc_user_phy_tx_time_update(struct dp_pdev *pdev,
				     struct cdp_tx_completion_ppdu *ppdu_desc,
				     struct cdp_tx_completion_ppdu_user *user)
{
}
#endif

/**
 * dp_ppdu_desc_user_stats_update(): Function to update TX user stats
 * @pdev: DP pdev handle
 * @ppdu_info: per PPDU TLV descriptor
 *
 * return: void
 */
void
dp_ppdu_desc_user_stats_update(struct dp_pdev *pdev,
			       struct ppdu_info *ppdu_info)
{
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	struct dp_peer *peer = NULL;
	uint32_t tlv_bitmap_expected;
	uint32_t tlv_bitmap_default;
	uint16_t i;
	uint32_t num_users;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	ppdu_desc = (struct cdp_tx_completion_ppdu *)
		qdf_nbuf_data(ppdu_info->nbuf);

	if (ppdu_desc->frame_type != CDP_PPDU_FTYPE_BAR)
		ppdu_desc->ppdu_id = ppdu_info->ppdu_id;

	tlv_bitmap_expected = HTT_PPDU_DEFAULT_TLV_BITMAP;
	if (mon_pdev->tx_sniffer_enable || mon_pdev->mcopy_mode ||
	    mon_pdev->tx_capture_enabled) {
		if (ppdu_info->is_ampdu)
			tlv_bitmap_expected =
				dp_htt_get_ppdu_sniffer_ampdu_tlv_bitmap(
					ppdu_info->tlv_bitmap);
	}

	tlv_bitmap_default = tlv_bitmap_expected;

	if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_BAR) {
		num_users = ppdu_desc->bar_num_users;
		ppdu_desc->num_users = ppdu_desc->bar_num_users;
	} else {
		num_users = ppdu_desc->num_users;
	}
	qdf_assert_always(ppdu_desc->num_users <= ppdu_desc->max_users);

	for (i = 0; i < num_users; i++) {
		ppdu_desc->num_mpdu += ppdu_desc->user[i].num_mpdu;
		ppdu_desc->num_msdu += ppdu_desc->user[i].num_msdu;

		peer = dp_peer_get_ref_by_id(pdev->soc,
					     ppdu_desc->user[i].peer_id,
					     DP_MOD_ID_TX_PPDU_STATS);
		/**
		 * This check is to make sure peer is not deleted
		 * after processing the TLVs.
		 */
		if (!peer)
			continue;

		ppdu_desc->user[i].is_bss_peer = peer->bss_peer;
		/*
		 * different frame like DATA, BAR or CTRL has different
		 * tlv bitmap expected. Apart from ACK_BA_STATUS TLV, we
		 * receive other tlv in-order/sequential from fw.
		 * Since ACK_BA_STATUS TLV come from Hardware it is
		 * asynchronous So we need to depend on some tlv to confirm
		 * all tlv is received for a ppdu.
		 * So we depend on both SCHED_CMD_STATUS_TLV and
		 * ACK_BA_STATUS_TLV. for failure packet we won't get
		 * ACK_BA_STATUS_TLV.
		 */
		if (!(ppdu_info->tlv_bitmap &
		      (1 << HTT_PPDU_STATS_SCH_CMD_STATUS_TLV)) ||
		    (!(ppdu_info->tlv_bitmap &
		       (1 << HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV)) &&
		     (ppdu_desc->user[i].completion_status ==
		      HTT_PPDU_STATS_USER_STATUS_OK))) {
			dp_peer_unref_delete(peer, DP_MOD_ID_TX_PPDU_STATS);
			continue;
		}

		/**
		 * Update tx stats for data frames having Qos as well as
		 * non-Qos data tid
		 */

		if ((ppdu_desc->user[i].tid < CDP_DATA_TID_MAX ||
		     (ppdu_desc->user[i].tid == CDP_DATA_NON_QOS_TID) ||
		     (ppdu_desc->htt_frame_type ==
		      HTT_STATS_FTYPE_SGEN_QOS_NULL) ||
		     ((ppdu_desc->frame_type == CDP_PPDU_FTYPE_BAR) &&
		      (ppdu_desc->num_mpdu > 1))) &&
		      (ppdu_desc->frame_type != CDP_PPDU_FTYPE_CTRL)) {
			dp_tx_stats_update(pdev, peer,
					   &ppdu_desc->user[i],
					   ppdu_desc->ack_rssi);
			dp_tx_rate_stats_update(peer, &ppdu_desc->user[i]);
		}

		dp_ppdu_desc_user_phy_tx_time_update(pdev, ppdu_desc,
						     &ppdu_desc->user[i]);

		dp_peer_unref_delete(peer, DP_MOD_ID_TX_PPDU_STATS);
		tlv_bitmap_expected = tlv_bitmap_default;
	}
}

#ifndef WLAN_TX_PKT_CAPTURE_ENH

/**
 * dp_ppdu_desc_deliver(): Function to deliver Tx PPDU status descriptor
 * to upper layer
 * @pdev: DP pdev handle
 * @ppdu_info: per PPDU TLV descriptor
 *
 * return: void
 */
static
void dp_ppdu_desc_deliver(struct dp_pdev *pdev,
			  struct ppdu_info *ppdu_info)
{
	struct ppdu_info *s_ppdu_info = NULL;
	struct ppdu_info *ppdu_info_next = NULL;
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	qdf_nbuf_t nbuf;
	uint32_t time_delta = 0;
	bool starved = 0;
	bool matched = 0;
	bool recv_ack_ba_done = 0;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	if (ppdu_info->tlv_bitmap &
	    (1 << HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV) &&
	    ppdu_info->done)
		recv_ack_ba_done = 1;

	mon_pdev->last_sched_cmdid = ppdu_info->sched_cmdid;

	s_ppdu_info = TAILQ_FIRST(&mon_pdev->sched_comp_ppdu_list);

	TAILQ_FOREACH_SAFE(s_ppdu_info, &mon_pdev->sched_comp_ppdu_list,
			   ppdu_info_list_elem, ppdu_info_next) {
		if (s_ppdu_info->tsf_l32 > ppdu_info->tsf_l32)
			time_delta = (MAX_TSF_32 - s_ppdu_info->tsf_l32) +
					ppdu_info->tsf_l32;
		else
			time_delta = ppdu_info->tsf_l32 - s_ppdu_info->tsf_l32;

		if (!s_ppdu_info->done && !recv_ack_ba_done) {
			if (time_delta < MAX_SCHED_STARVE) {
				dp_mon_info("pdev[%d] ppdu_id[%d] sched_cmdid[%d] TLV_B[0x%x] TSF[%u] D[%d]",
					    pdev->pdev_id,
					    s_ppdu_info->ppdu_id,
					    s_ppdu_info->sched_cmdid,
					    s_ppdu_info->tlv_bitmap,
					    s_ppdu_info->tsf_l32,
					    s_ppdu_info->done);
				break;
			}
			starved = 1;
		}

		mon_pdev->delivered_sched_cmdid = s_ppdu_info->sched_cmdid;
		TAILQ_REMOVE(&mon_pdev->sched_comp_ppdu_list, s_ppdu_info,
			     ppdu_info_list_elem);
		mon_pdev->sched_comp_list_depth--;

		nbuf = s_ppdu_info->nbuf;
		qdf_assert_always(nbuf);
		ppdu_desc = (struct cdp_tx_completion_ppdu *)
				qdf_nbuf_data(nbuf);
		ppdu_desc->tlv_bitmap = s_ppdu_info->tlv_bitmap;

		if (starved) {
			dp_mon_err("ppdu starved fc[0x%x] h_ftype[%d] tlv_bitmap[0x%x] cs[%d]\n",
				   ppdu_desc->frame_ctrl,
				   ppdu_desc->htt_frame_type,
				   ppdu_desc->tlv_bitmap,
				   ppdu_desc->user[0].completion_status);
			starved = 0;
		}

		if (ppdu_info->ppdu_id == s_ppdu_info->ppdu_id &&
		    ppdu_info->sched_cmdid == s_ppdu_info->sched_cmdid)
			matched = 1;

		dp_ppdu_desc_user_stats_update(pdev, s_ppdu_info);

		qdf_mem_free(s_ppdu_info);

		/**
		 * Deliver PPDU stats only for valid (acked) data
		 * frames if sniffer mode is not enabled.
		 * If sniffer mode is enabled, PPDU stats
		 * for all frames including mgmt/control
		 * frames should be delivered to upper layer
		 */
		if (mon_pdev->tx_sniffer_enable || mon_pdev->mcopy_mode) {
			dp_wdi_event_handler(WDI_EVENT_TX_PPDU_DESC,
					     pdev->soc,
					     nbuf, HTT_INVALID_PEER,
					     WDI_NO_VAL,
					     pdev->pdev_id);
		} else {
			if (ppdu_desc->num_mpdu != 0 &&
			    ppdu_desc->num_users != 0 &&
			    ppdu_desc->frame_ctrl &
			    HTT_FRAMECTRL_DATATYPE) {
				dp_wdi_event_handler(WDI_EVENT_TX_PPDU_DESC,
						     pdev->soc,
						     nbuf, HTT_INVALID_PEER,
						     WDI_NO_VAL,
						     pdev->pdev_id);
			} else {
				qdf_nbuf_free(nbuf);
			}
		}

		if (matched)
			break;
	}
}

#endif

/**
 * dp_get_ppdu_desc(): Function to allocate new PPDU status
 * desc for new ppdu id
 * @pdev: DP pdev handle
 * @ppdu_id: PPDU unique identifier
 * @tlv_type: TLV type received
 * @tsf_l32: timestamp received along with ppdu stats indication header
 * @max_users: Maximum user for that particular ppdu
 *
 * return: ppdu_info per ppdu tlv structure
 */
static
struct ppdu_info *dp_get_ppdu_desc(struct dp_pdev *pdev, uint32_t ppdu_id,
				   uint8_t tlv_type, uint32_t tsf_l32,
				   uint8_t max_users)
{
	struct ppdu_info *ppdu_info = NULL;
	struct ppdu_info *s_ppdu_info = NULL;
	struct ppdu_info *ppdu_info_next = NULL;
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	uint32_t size = 0;
	struct cdp_tx_completion_ppdu *tmp_ppdu_desc = NULL;
	struct cdp_tx_completion_ppdu_user *tmp_user;
	uint32_t time_delta;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	/*
	 * Find ppdu_id node exists or not
	 */
	TAILQ_FOREACH_SAFE(ppdu_info, &mon_pdev->ppdu_info_list,
			   ppdu_info_list_elem, ppdu_info_next) {
		if (ppdu_info && (ppdu_info->ppdu_id == ppdu_id)) {
			if (ppdu_info->tsf_l32 > tsf_l32)
				time_delta  = (MAX_TSF_32 -
					       ppdu_info->tsf_l32) + tsf_l32;
			else
				time_delta  = tsf_l32 - ppdu_info->tsf_l32;

			if (time_delta > WRAP_DROP_TSF_DELTA) {
				TAILQ_REMOVE(&mon_pdev->ppdu_info_list,
					     ppdu_info, ppdu_info_list_elem);
				mon_pdev->list_depth--;
				pdev->stats.ppdu_wrap_drop++;
				tmp_ppdu_desc =
					(struct cdp_tx_completion_ppdu *)
					qdf_nbuf_data(ppdu_info->nbuf);
				tmp_user = &tmp_ppdu_desc->user[0];
				dp_htt_tx_stats_info("S_PID [%d] S_TSF[%u] TLV_BITMAP[0x%x] [CMPLTN - %d ACK_BA - %d] CS[%d] - R_PID[%d] R_TSF[%u] R_TLV_TAG[0x%x]\n",
						     ppdu_info->ppdu_id,
						     ppdu_info->tsf_l32,
						     ppdu_info->tlv_bitmap,
						     tmp_user->completion_status,
						     ppdu_info->compltn_common_tlv,
						     ppdu_info->ack_ba_tlv,
						     ppdu_id, tsf_l32,
						     tlv_type);
				qdf_nbuf_free(ppdu_info->nbuf);
				ppdu_info->nbuf = NULL;
				qdf_mem_free(ppdu_info);
			} else {
				break;
			}
		}
	}

	/*
	 * check if it is ack ba tlv and if it is not there in ppdu info
	 * list then check it in sched completion ppdu list
	 */
	if (!ppdu_info &&
	    tlv_type == HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV) {
		TAILQ_FOREACH(s_ppdu_info,
			      &mon_pdev->sched_comp_ppdu_list,
			      ppdu_info_list_elem) {
			if (s_ppdu_info && (s_ppdu_info->ppdu_id == ppdu_id)) {
				if (s_ppdu_info->tsf_l32 > tsf_l32)
					time_delta  = (MAX_TSF_32 -
						       s_ppdu_info->tsf_l32) +
							tsf_l32;
				else
					time_delta  = tsf_l32 -
						s_ppdu_info->tsf_l32;
				if (time_delta < WRAP_DROP_TSF_DELTA) {
					ppdu_info = s_ppdu_info;
					break;
				}
			} else {
				/*
				 * ACK BA STATUS TLV comes sequential order
				 * if we received ack ba status tlv for second
				 * ppdu and first ppdu is still waiting for
				 * ACK BA STATUS TLV. Based on fw comment
				 * we won't receive it tlv later. So we can
				 * set ppdu info done.
				 */
				if (s_ppdu_info)
					s_ppdu_info->done = 1;
			}
		}
	}

	if (ppdu_info) {
		if (ppdu_info->tlv_bitmap & (1 << tlv_type)) {
			/**
			 * if we get tlv_type that is already been processed
			 * for ppdu, that means we got a new ppdu with same
			 * ppdu id. Hence Flush the older ppdu
			 * for MUMIMO and OFDMA, In a PPDU we have
			 * multiple user with same tlv types. tlv bitmap is
			 * used to check whether SU or MU_MIMO/OFDMA
			 */
			if (!(ppdu_info->tlv_bitmap &
			    (1 << HTT_PPDU_STATS_SCH_CMD_STATUS_TLV)))
				return ppdu_info;

			ppdu_desc = (struct cdp_tx_completion_ppdu *)
				qdf_nbuf_data(ppdu_info->nbuf);

			/**
			 * apart from ACK BA STATUS TLV rest all comes in order
			 * so if tlv type not ACK BA STATUS TLV we can deliver
			 * ppdu_info
			 */
			if ((tlv_type ==
			     HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV) &&
			    (ppdu_desc->htt_frame_type ==
			     HTT_STATS_FTYPE_SGEN_MU_BAR))
				return ppdu_info;

			dp_ppdu_desc_deliver(pdev, ppdu_info);
		} else {
			return ppdu_info;
		}
	}

	/**
	 * Flush the head ppdu descriptor if ppdu desc list reaches max
	 * threshold
	 */
	if (mon_pdev->list_depth > HTT_PPDU_DESC_MAX_DEPTH) {
		ppdu_info = TAILQ_FIRST(&mon_pdev->ppdu_info_list);
		TAILQ_REMOVE(&mon_pdev->ppdu_info_list,
			     ppdu_info, ppdu_info_list_elem);
		mon_pdev->list_depth--;
		pdev->stats.ppdu_drop++;
		qdf_nbuf_free(ppdu_info->nbuf);
		ppdu_info->nbuf = NULL;
		qdf_mem_free(ppdu_info);
	}

	size = sizeof(struct cdp_tx_completion_ppdu) +
		(max_users * sizeof(struct cdp_tx_completion_ppdu_user));

	/*
	 * Allocate new ppdu_info node
	 */
	ppdu_info = qdf_mem_malloc(sizeof(struct ppdu_info));
	if (!ppdu_info)
		return NULL;

	ppdu_info->nbuf = qdf_nbuf_alloc(pdev->soc->osdev, size,
					 0, 4, TRUE);
	if (!ppdu_info->nbuf) {
		qdf_mem_free(ppdu_info);
		return NULL;
	}

	ppdu_info->ppdu_desc =
		(struct cdp_tx_completion_ppdu *)qdf_nbuf_data(ppdu_info->nbuf);
	qdf_mem_zero(qdf_nbuf_data(ppdu_info->nbuf), size);

	if (!qdf_nbuf_put_tail(ppdu_info->nbuf, size)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "No tailroom for HTT PPDU");
		qdf_nbuf_free(ppdu_info->nbuf);
		ppdu_info->nbuf = NULL;
		ppdu_info->last_user = 0;
		qdf_mem_free(ppdu_info);
		return NULL;
	}

	ppdu_info->ppdu_desc->max_users = max_users;
	ppdu_info->tsf_l32 = tsf_l32;
	/**
	 * No lock is needed because all PPDU TLVs are processed in
	 * same context and this list is updated in same context
	 */
	TAILQ_INSERT_TAIL(&mon_pdev->ppdu_info_list, ppdu_info,
			  ppdu_info_list_elem);
	mon_pdev->list_depth++;
	return ppdu_info;
}

/**
 * dp_htt_process_tlv(): Function to process each PPDU TLVs
 * @pdev: DP pdev handle
 * @htt_t2h_msg: HTT target to host message
 *
 * return: ppdu_info per ppdu tlv structure
 */

static struct ppdu_info *dp_htt_process_tlv(struct dp_pdev *pdev,
					    qdf_nbuf_t htt_t2h_msg)
{
	uint32_t length;
	uint32_t ppdu_id;
	uint8_t tlv_type;
	uint32_t tlv_length, tlv_bitmap_expected;
	uint8_t *tlv_buf;
	struct ppdu_info *ppdu_info = NULL;
	struct cdp_tx_completion_ppdu *ppdu_desc = NULL;
	uint8_t max_users = CDP_MU_MAX_USERS;
	uint32_t tsf_l32;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	uint32_t *msg_word = (uint32_t *)qdf_nbuf_data(htt_t2h_msg);

	length = HTT_T2H_PPDU_STATS_PAYLOAD_SIZE_GET(*msg_word);

	msg_word = msg_word + 1;
	ppdu_id = HTT_T2H_PPDU_STATS_PPDU_ID_GET(*msg_word);

	msg_word = msg_word + 1;
	tsf_l32 = (uint32_t)(*msg_word);

	msg_word = msg_word + 2;
	while (length > 0) {
		tlv_buf = (uint8_t *)msg_word;
		tlv_type = HTT_STATS_TLV_TAG_GET(*msg_word);
		tlv_length = HTT_STATS_TLV_LENGTH_GET(*msg_word);
		if (qdf_likely(tlv_type < CDP_PPDU_STATS_MAX_TAG))
			pdev->stats.ppdu_stats_counter[tlv_type]++;

		if (tlv_length == 0)
			break;

		tlv_length += HTT_TLV_HDR_LEN;

		/**
		 * Not allocating separate ppdu descriptor for MGMT Payload
		 * TLV as this is sent as separate WDI indication and it
		 * doesn't contain any ppdu information
		 */
		if (tlv_type == HTT_PPDU_STATS_TX_MGMTCTRL_PAYLOAD_TLV) {
			mon_pdev->mgmtctrl_frm_info.mgmt_buf = tlv_buf;
			mon_pdev->mgmtctrl_frm_info.ppdu_id = ppdu_id;
			mon_pdev->mgmtctrl_frm_info.mgmt_buf_len =
				HTT_PPDU_STATS_TX_MGMTCTRL_TLV_FRAME_LENGTH_GET
						(*(msg_word + 1));
			msg_word =
				(uint32_t *)((uint8_t *)tlv_buf + tlv_length);
			length -= (tlv_length);
			continue;
		}

		/*
		 * retrieve max_users if it's USERS_INFO,
		 * else, it's 1 for COMPLTN_FLUSH,
		 * else, use CDP_MU_MAX_USERS
		 */
		if (tlv_type == HTT_PPDU_STATS_USERS_INFO_TLV) {
			max_users =
				HTT_PPDU_STATS_USERS_INFO_TLV_MAX_USERS_GET(*(msg_word + 1));
		} else if (tlv_type == HTT_PPDU_STATS_USR_COMPLTN_FLUSH_TLV) {
			max_users = 1;
		}

		ppdu_info = dp_get_ppdu_desc(pdev, ppdu_id, tlv_type,
					     tsf_l32, max_users);
		if (!ppdu_info)
			return NULL;

		ppdu_info->ppdu_id = ppdu_id;
		ppdu_info->tlv_bitmap |= (1 << tlv_type);

		dp_process_ppdu_tag(pdev, msg_word, tlv_length, ppdu_info);

		/**
		 * Increment pdev level tlv count to monitor
		 * missing TLVs
		 */
		mon_pdev->tlv_count++;
		ppdu_info->last_tlv_cnt = mon_pdev->tlv_count;
		msg_word = (uint32_t *)((uint8_t *)tlv_buf + tlv_length);
		length -= (tlv_length);
	}

	if (!ppdu_info)
		return NULL;

	mon_pdev->last_ppdu_id = ppdu_id;

	tlv_bitmap_expected = HTT_PPDU_DEFAULT_TLV_BITMAP;

	if (mon_pdev->tx_sniffer_enable || mon_pdev->mcopy_mode ||
	    mon_pdev->tx_capture_enabled) {
		if (ppdu_info->is_ampdu)
			tlv_bitmap_expected =
				dp_htt_get_ppdu_sniffer_ampdu_tlv_bitmap(
					ppdu_info->tlv_bitmap);
	}

	ppdu_desc = ppdu_info->ppdu_desc;

	if (!ppdu_desc)
		return NULL;

	if (ppdu_desc->user[ppdu_desc->last_usr_index].completion_status !=
	    HTT_PPDU_STATS_USER_STATUS_OK) {
		tlv_bitmap_expected = tlv_bitmap_expected & 0xFF;
	}

	/*
	 * for frame type DATA and BAR, we update stats based on MSDU,
	 * successful msdu and mpdu are populate from ACK BA STATUS TLV
	 * which comes out of order. successful mpdu also populated from
	 * COMPLTN COMMON TLV which comes in order. for every ppdu_info
	 * we store successful mpdu from both tlv and compare before delivering
	 * to make sure we received ACK BA STATUS TLV. For some self generated
	 * frame we won't get ack ba status tlv so no need to wait for
	 * ack ba status tlv.
	 */
	if (ppdu_desc->frame_type != CDP_PPDU_FTYPE_CTRL &&
	    ppdu_desc->htt_frame_type != HTT_STATS_FTYPE_SGEN_QOS_NULL) {
		/*
		 * most of the time bar frame will have duplicate ack ba
		 * status tlv
		 */
		if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_BAR &&
		    (ppdu_info->compltn_common_tlv != ppdu_info->ack_ba_tlv))
			return NULL;
		/*
		 * For data frame, compltn common tlv should match ack ba status
		 * tlv and completion status. Reason we are checking first user
		 * for ofdma, completion seen at next MU BAR frm, for mimo
		 * only for first user completion will be immediate.
		 */
		if (ppdu_desc->frame_type == CDP_PPDU_FTYPE_DATA &&
		    (ppdu_desc->user[0].completion_status == 0 &&
		     (ppdu_info->compltn_common_tlv != ppdu_info->ack_ba_tlv)))
			return NULL;
	}

	/**
	 * Once all the TLVs for a given PPDU has been processed,
	 * return PPDU status to be delivered to higher layer.
	 * tlv_bitmap_expected can't be available for different frame type.
	 * But SCHED CMD STATS TLV is the last TLV from the FW for a ppdu.
	 * apart from ACK BA TLV, FW sends other TLV in sequential order.
	 * flush tlv comes separate.
	 */
	if ((ppdu_info->tlv_bitmap != 0 &&
	     (ppdu_info->tlv_bitmap &
	      (1 << HTT_PPDU_STATS_SCH_CMD_STATUS_TLV))) ||
	    (ppdu_info->tlv_bitmap &
	     (1 << HTT_PPDU_STATS_USR_COMPLTN_FLUSH_TLV))) {
		ppdu_info->done = 1;
		return ppdu_info;
	}

	return NULL;
}
#endif /* QCA_ENHANCED_STATS_SUPPORT */

/**
 * dp_txrx_ppdu_stats_handler() - Function to process HTT PPDU stats from FW
 * @soc: DP SOC handle
 * @pdev_id: pdev id
 * @htt_t2h_msg: HTT message nbuf
 *
 * return:void
 */
#if defined(WDI_EVENT_ENABLE)
#ifdef QCA_ENHANCED_STATS_SUPPORT
static bool dp_txrx_ppdu_stats_handler(struct dp_soc *soc,
				       uint8_t pdev_id, qdf_nbuf_t htt_t2h_msg)
{
	struct dp_pdev *pdev = soc->pdev_list[pdev_id];
	struct ppdu_info *ppdu_info = NULL;
	bool free_buf = true;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	if (pdev_id >= MAX_PDEV_CNT)
		return true;

	pdev = soc->pdev_list[pdev_id];
	if (!pdev)
		return true;

	if (!mon_pdev->enhanced_stats_en && !mon_pdev->tx_sniffer_enable &&
	    !mon_pdev->mcopy_mode && !mon_pdev->bpr_enable)
		return free_buf;

	qdf_spin_lock_bh(&mon_pdev->ppdu_stats_lock);
	ppdu_info = dp_htt_process_tlv(pdev, htt_t2h_msg);

	if (mon_pdev->mgmtctrl_frm_info.mgmt_buf) {
		if (dp_process_ppdu_stats_tx_mgmtctrl_payload_tlv
		    (pdev, htt_t2h_msg, mon_pdev->mgmtctrl_frm_info.ppdu_id) !=
		    QDF_STATUS_SUCCESS)
			free_buf = false;
	}

	if (ppdu_info)
		dp_ppdu_desc_deliver(pdev, ppdu_info);

	mon_pdev->mgmtctrl_frm_info.mgmt_buf = NULL;
	mon_pdev->mgmtctrl_frm_info.mgmt_buf_len = 0;
	mon_pdev->mgmtctrl_frm_info.ppdu_id = 0;

	qdf_spin_unlock_bh(&mon_pdev->ppdu_stats_lock);

	return free_buf;
}
#else
static bool dp_txrx_ppdu_stats_handler(struct dp_soc *soc,
				       uint8_t pdev_id, qdf_nbuf_t htt_t2h_msg)
{
	return true;
}
#endif/* QCA_ENHANCED_STATS_SUPPORT */
#endif

#if defined(WDI_EVENT_ENABLE) &&\
	(defined(QCA_ENHANCED_STATS_SUPPORT) || !defined(REMOVE_PKT_LOG))
/*
 * dp_ppdu_stats_ind_handler() - PPDU stats msg handler
 * @htt_soc:	 HTT SOC handle
 * @msg_word:    Pointer to payload
 * @htt_t2h_msg: HTT msg nbuf
 *
 * Return: True if buffer should be freed by caller.
 */
static inline bool
dp_ppdu_stats_ind_handler(struct htt_soc *soc,
			  uint32_t *msg_word,
			  qdf_nbuf_t htt_t2h_msg)
{
	u_int8_t pdev_id;
	u_int8_t target_pdev_id;
	bool free_buf;

	target_pdev_id = HTT_T2H_PPDU_STATS_PDEV_ID_GET(*msg_word);
	pdev_id = dp_get_host_pdev_id_for_target_pdev_id(soc->dp_soc,
							 target_pdev_id);
	dp_wdi_event_handler(WDI_EVENT_LITE_T2H, soc->dp_soc,
			     htt_t2h_msg, HTT_INVALID_PEER, WDI_NO_VAL,
			     pdev_id);

	free_buf = dp_txrx_ppdu_stats_handler(soc->dp_soc, pdev_id,
					      htt_t2h_msg);

	return free_buf;
}
#endif

#ifndef QCA_SUPPORT_FULL_MON
/**
 * dp_rx_mon_process () - Core brain processing for monitor mode
 *
 * This API processes monitor destination ring followed by monitor status ring
 * Called from bottom half (tasklet/NET_RX_SOFTIRQ)
 *
 * @soc: datapath soc context
 * @int_ctx: interrupt context
 * @mac_id: mac_id on which interrupt is received
 * @quota: Number of status ring entry that can be serviced in one shot.
 *
 * @Return: Number of reaped status ring entries
 */
static inline uint32_t
dp_rx_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
		  uint32_t mac_id, uint32_t quota)
{
	return quota;
}
#endif

#ifndef DISABLE_MON_CONFIG
static uint32_t
dp_rx_mon_process_1_0(struct dp_soc *soc, struct dp_intr *int_ctx,
	              uint32_t mac_id, uint32_t quota)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	if (qdf_unlikely(mon_soc->full_mon_mode))
		return dp_rx_mon_process(soc, int_ctx, mac_id, quota);

	return dp_rx_mon_status_process(soc, int_ctx, mac_id, quota);
}

static void dp_mon_register_intr_ops_1_0(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	mon_soc->mon_rx_process = dp_rx_mon_process_1_0;
}
#endif

/**
 * dp_mon_register_feature_ops_1_0() - register feature ops
 *
 * @soc: dp soc context
 *
 * @return: void
 */
static void
dp_mon_register_feature_ops_1_0(struct dp_soc *soc)
{
	struct dp_mon_ops *mon_ops = dp_mon_ops_get(soc);

	if (!mon_ops) {
		dp_err("mon_ops is NULL, feature ops registration failed");
		return;
	}

	mon_ops->mon_config_debug_sniffer = dp_config_debug_sniffer;
	mon_ops->mon_peer_tx_init = dp_peer_tx_init;
	mon_ops->mon_peer_tx_cleanup = dp_peer_tx_cleanup;
	mon_ops->mon_htt_ppdu_stats_attach = dp_htt_ppdu_stats_attach;
	mon_ops->mon_htt_ppdu_stats_detach = dp_htt_ppdu_stats_detach;
	mon_ops->mon_print_pdev_rx_mon_stats = dp_print_pdev_rx_mon_stats;
	mon_ops->mon_set_bsscolor = dp_mon_set_bsscolor;
	mon_ops->mon_pdev_get_filter_ucast_data =
				dp_pdev_get_filter_ucast_data;
	mon_ops->mon_pdev_get_filter_mcast_data =
				dp_pdev_get_filter_mcast_data;
	mon_ops->mon_pdev_get_filter_non_data = dp_pdev_get_filter_non_data;
	mon_ops->mon_neighbour_peer_add_ast = dp_mon_neighbour_peer_add_ast;
#ifdef WLAN_TX_PKT_CAPTURE_ENH
	mon_ops->mon_peer_tid_peer_id_update = dp_peer_tid_peer_id_update;
	mon_ops->mon_tx_ppdu_stats_attach = dp_tx_ppdu_stats_attach;
	mon_ops->mon_tx_ppdu_stats_detach = dp_tx_ppdu_stats_detach;
	mon_ops->mon_tx_capture_debugfs_init = dp_tx_capture_debugfs_init;
	mon_ops->mon_tx_add_to_comp_queue = dp_tx_add_to_comp_queue;
	mon_ops->mon_peer_tx_capture_filter_check =
				dp_peer_tx_capture_filter_check;
	mon_ops->mon_print_pdev_tx_capture_stats =
				dp_print_pdev_tx_capture_stats;
	mon_ops->mon_config_enh_tx_capture = dp_config_enh_tx_capture;
#endif
#if defined(WDI_EVENT_ENABLE) &&\
	(defined(QCA_ENHANCED_STATS_SUPPORT) || !defined(REMOVE_PKT_LOG))
	mon_ops->mon_ppdu_stats_ind_handler = dp_ppdu_stats_ind_handler;
#endif
#ifdef WLAN_RX_PKT_CAPTURE_ENH
	mon_ops->mon_config_enh_rx_capture = dp_config_enh_rx_capture;
#endif
#ifdef QCA_SUPPORT_BPR
	mon_ops->mon_set_bpr_enable = dp_set_bpr_enable_1_0;
#endif
#ifdef ATH_SUPPORT_NAC
	mon_ops->mon_set_filter_neigh_peers = dp_set_filter_neigh_peers;
#endif
#ifdef WLAN_ATF_ENABLE
	mon_ops->mon_set_atf_stats_enable = dp_set_atf_stats_enable;
#endif
#ifdef FEATURE_NAC_RSSI
	mon_ops->mon_filter_neighbour_peer = dp_filter_neighbour_peer;
#endif
#ifdef QCA_MCOPY_SUPPORT
	mon_ops->mon_filter_setup_mcopy_mode =
				dp_mon_filter_setup_mcopy_mode_1_0;
	mon_ops->mon_filter_reset_mcopy_mode =
				dp_mon_filter_reset_mcopy_mode_1_0;
	mon_ops->mon_mcopy_check_deliver = dp_mcopy_check_deliver;
#endif
#ifdef QCA_ENHANCED_STATS_SUPPORT
	mon_ops->mon_filter_setup_enhanced_stats =
				dp_mon_filter_setup_enhanced_stats_1_0;
	mon_ops->mon_filter_reset_enhanced_stats =
				dp_mon_filter_reset_enhanced_stats_1_0;
#endif
#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
	mon_ops->mon_filter_setup_smart_monitor =
				dp_mon_filter_setup_smart_monitor_1_0;
#endif
#ifdef WLAN_RX_PKT_CAPTURE_ENH
	mon_ops->mon_filter_setup_rx_enh_capture =
				dp_mon_filter_setup_rx_enh_capture_1_0;
#endif
#ifdef WDI_EVENT_ENABLE
	mon_ops->mon_set_pktlog_wifi3 = dp_set_pktlog_wifi3;
	mon_ops->mon_filter_setup_rx_pkt_log_full =
				dp_mon_filter_setup_rx_pkt_log_full_1_0;
	mon_ops->mon_filter_reset_rx_pkt_log_full =
				dp_mon_filter_reset_rx_pkt_log_full_1_0;
	mon_ops->mon_filter_setup_rx_pkt_log_lite =
				dp_mon_filter_setup_rx_pkt_log_lite_1_0;
	mon_ops->mon_filter_reset_rx_pkt_log_lite =
				dp_mon_filter_reset_rx_pkt_log_lite_1_0;
	mon_ops->mon_filter_setup_rx_pkt_log_cbf =
				dp_mon_filter_setup_rx_pkt_log_cbf_1_0;
	mon_ops->mon_filter_reset_rx_pkt_log_cbf =
				dp_mon_filter_reset_rx_pktlog_cbf_1_0;
#ifdef QCA_WIFI_QCN9224
	mon_ops->mon_filter_setup_pktlog_hybrid =
				dp_mon_filter_setup_pktlog_hybrid_1_0;
	mon_ops->mon_filter_reset_pktlog_hybrid =
				dp_mon_filter_reset_pktlog_hybrid_1_0;
#endif
#endif
#if defined(DP_CON_MON) && !defined(REMOVE_PKT_LOG)
	mon_ops->mon_pktlogmod_exit = dp_pktlogmod_exit;
#endif
	mon_ops->rx_packet_length_set = NULL;
	mon_ops->rx_wmask_subscribe = NULL;
	mon_ops->rx_enable_mpdu_logging = NULL;
	mon_ops->mon_neighbour_peers_detach = dp_neighbour_peers_detach;
	mon_ops->mon_vdev_set_monitor_mode_buf_rings =
				dp_vdev_set_monitor_mode_buf_rings;
	mon_ops->mon_vdev_set_monitor_mode_rings =
				dp_vdev_set_monitor_mode_rings;
}

struct dp_mon_ops monitor_ops_1_0 = {
	.mon_soc_cfg_init = dp_mon_soc_cfg_init,
	.mon_pdev_alloc = NULL,
	.mon_pdev_free = NULL,
	.mon_pdev_attach = dp_mon_pdev_attach,
	.mon_pdev_detach = dp_mon_pdev_detach,
	.mon_pdev_init = dp_mon_pdev_init,
	.mon_pdev_deinit = dp_mon_pdev_deinit,
	.mon_vdev_attach = dp_mon_vdev_attach,
	.mon_vdev_detach = dp_mon_vdev_detach,
	.mon_peer_attach = dp_mon_peer_attach,
	.mon_peer_detach = dp_mon_peer_detach,
	.mon_flush_rings = dp_flush_monitor_rings,
#if !defined(DISABLE_MON_CONFIG)
	.mon_pdev_htt_srng_setup = dp_mon_htt_srng_setup_1_0,
#endif
#if defined(DP_CON_MON)
	.mon_service_rings = dp_service_mon_rings,
#endif
#ifndef DISABLE_MON_CONFIG
	.mon_rx_process = NULL,
#endif
#if !defined(DISABLE_MON_CONFIG) && defined(MON_ENABLE_DROP_FOR_MAC)
	.mon_drop_packets_for_mac = dp_mon_drop_packets_for_mac,
#endif
	.mon_vdev_timer_init = dp_mon_vdev_timer_init,
	.mon_vdev_timer_start = dp_mon_vdev_timer_start,
	.mon_vdev_timer_stop = dp_mon_vdev_timer_stop,
	.mon_vdev_timer_deinit = dp_mon_vdev_timer_deinit,
	.mon_reap_timer_init = dp_mon_reap_timer_init,
	.mon_reap_timer_start = dp_mon_reap_timer_start,
	.mon_reap_timer_stop = dp_mon_reap_timer_stop,
	.mon_reap_timer_deinit = dp_mon_reap_timer_deinit,
	.mon_filter_setup_mon_mode = dp_mon_filter_setup_mon_mode_1_0,
	.mon_filter_reset_mon_mode = dp_mon_filter_reset_mon_mode_1_0,
	.mon_filter_update = dp_mon_filter_update_1_0,
	.rx_mon_desc_pool_init = dp_rx_pdev_mon_desc_pool_init,
	.rx_mon_desc_pool_deinit = dp_rx_pdev_mon_desc_pool_deinit,
	.rx_mon_desc_pool_alloc = dp_rx_pdev_mon_desc_pool_alloc,
	.rx_mon_desc_pool_free = dp_rx_pdev_mon_desc_pool_free,
	.rx_mon_buffers_alloc = dp_rx_pdev_mon_buffers_alloc,
	.rx_mon_buffers_free = dp_rx_pdev_mon_buffers_free,
	.tx_mon_desc_pool_init = NULL,
	.tx_mon_desc_pool_deinit = NULL,
	.tx_mon_desc_pool_alloc = NULL,
	.tx_mon_desc_pool_free = NULL,
	.mon_rings_alloc = dp_mon_rings_alloc_1_0,
	.mon_rings_free = dp_mon_rings_free_1_0,
	.mon_rings_init = dp_mon_rings_init_1_0,
	.mon_rings_deinit = dp_mon_rings_deinit_1_0,
#if !defined(DISABLE_MON_CONFIG)
	.mon_register_intr_ops = dp_mon_register_intr_ops_1_0,
#endif
	.mon_register_feature_ops = dp_mon_register_feature_ops_1_0,
};

struct cdp_mon_ops dp_ops_mon_1_0 = {
	.txrx_reset_monitor_mode = dp_reset_monitor_mode,
	/* Added support for HK advance filter */
	.txrx_set_advance_monitor_filter = dp_pdev_set_advance_monitor_filter,
	.txrx_deliver_tx_mgmt = dp_deliver_tx_mgmt,
	.config_full_mon_mode = dp_config_full_mon_mode,
	.soc_config_full_mon_mode = dp_soc_config_full_mon_mode,
};

struct dp_mon_ops *dp_mon_ops_get_1_0(void)
{
	return &monitor_ops_1_0;
}

struct cdp_mon_ops *dp_mon_cdp_ops_get_1_0(void)
{
	return &dp_ops_mon_1_0;
}
