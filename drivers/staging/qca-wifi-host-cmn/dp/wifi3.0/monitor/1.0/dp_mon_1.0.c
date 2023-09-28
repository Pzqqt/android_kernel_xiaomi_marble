/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
