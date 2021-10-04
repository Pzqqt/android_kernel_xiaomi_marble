/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
#include <dp_rx_mon_2.0.h>
#include <dp_mon_2.0.h>
#include <dp_mon_filter_2.0.h>
#include <dp_tx_mon_2.0.h>
#include <hal_be_api_mon.h>
#include <dp_be.h>

extern QDF_STATUS dp_srng_alloc(struct dp_soc *soc, struct dp_srng *srng,
				int ring_type, uint32_t num_entries,
				bool cached);
extern void dp_srng_free(struct dp_soc *soc, struct dp_srng *srng);
extern QDF_STATUS dp_srng_init(struct dp_soc *soc, struct dp_srng *srng,
			       int ring_type, int ring_num, int mac_id);
extern void dp_srng_deinit(struct dp_soc *soc, struct dp_srng *srng,
			   int ring_type, int ring_num);

#if !defined(DISABLE_MON_CONFIG)
static
void dp_vdev_set_monitor_mode_buf_rings_2_0(struct dp_pdev *pdev)
{
}

static
QDF_STATUS dp_vdev_set_monitor_mode_rings_2_0(struct dp_pdev *pdev,
					      uint8_t delayed_replenish)
{
	return QDF_STATUS_SUCCESS;
}
#else
static inline
void dp_vdev_set_monitor_mode_buf_rings_2_0(struct dp_pdev *pdev)
{
}

static inline
QDF_STATUS dp_vdev_set_monitor_mode_rings_2_0(struct dp_pdev *pdev,
					      uint8_t delayed_replenish)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef QCA_SUPPORT_BPR
static QDF_STATUS
dp_set_bpr_enable_2_0(struct dp_pdev *pdev, int val)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifndef DISABLE_MON_CONFIG
static
QDF_STATUS dp_mon_htt_srng_setup_2_0(struct dp_soc *soc,
				     struct dp_pdev *pdev,
				     int mac_id,
				     int mac_for_pdev)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;
	QDF_STATUS status;

	status = htt_srng_setup(soc->htt_handle, mac_for_pdev,
				soc->rxdma_mon_dst_ring[mac_id].hal_srng,
				RXDMA_MONITOR_DST);

	if (status != QDF_STATUS_SUCCESS) {
		dp_mon_err("Failed to send htt srng setup message for Rxdma dst ring");
		return status;
	}

	status = htt_srng_setup(soc->htt_handle, mac_for_pdev,
				mon_soc->tx_mon_dst_ring[mac_id].hal_srng,
				TX_MONITOR_DST);

	if (status != QDF_STATUS_SUCCESS) {
		dp_mon_err("Failed to send htt srng message for Tx mon dst ring");
		return status;
	}
	return status;
}

static uint32_t
dp_rx_mon_process_2_0(struct dp_soc *soc, struct dp_intr *int_ctx,
		      uint32_t mac_id, uint32_t quota)
{
	return 0;
}

static uint32_t
dp_tx_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
		  uint32_t mac_id, uint32_t quota)
{
	return 0;
}

static
QDF_STATUS dp_mon_soc_detach_2_0(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	dp_tx_mon_buffers_free(soc);
	dp_rx_mon_buffers_free(soc);
	dp_tx_mon_buf_desc_pool_free(soc);
	dp_rx_mon_buf_desc_pool_free(soc);

	if (mon_soc) {
		dp_srng_free(soc, &soc->rxdma_mon_buf_ring[0]);
		dp_srng_free(soc, &mon_soc->tx_mon_buf_ring);
		qdf_mem_free(be_soc->monitor_soc_be);
	}

	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS dp_mon_soc_attach_2_0(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = NULL;
	int entries = 8192;

	mon_soc = (struct dp_mon_soc_be *)qdf_mem_malloc(sizeof(*mon_soc));
	if (!mon_soc) {
		dp_mon_err("%pK: mem allocation failed", soc);
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_zero(mon_soc, sizeof(*mon_soc));
	be_soc->monitor_soc_be = mon_soc;

	if (dp_srng_alloc(soc, &soc->rxdma_mon_buf_ring[0],
			  RXDMA_MONITOR_BUF, entries, 0)) {
		dp_mon_err("%pK: " RNG_ERR "rx_mon_buf_ring", soc);
		goto fail;
	}

	if (dp_srng_alloc(soc, &mon_soc->tx_mon_buf_ring,
			  TX_MONITOR_BUF, entries, 0)) {
		dp_mon_err("%pK: " RNG_ERR "tx_mon_buf_ring", soc);
		goto fail;
	}

	/* sw desc pool for src ring */
	if (dp_rx_mon_buf_desc_pool_alloc(soc)) {
		dp_mon_err("%pK: Rx mon desc pool allocation failed", soc);
		goto fail;
	}

	if (dp_tx_mon_buf_desc_pool_alloc(soc)) {
		dp_mon_err("%pK: Tx mon desc pool allocation failed", soc);
		goto fail;
	}

	/* monitor buffers for src */
	if (dp_rx_mon_buffers_alloc(soc)) {
		dp_mon_err("%pK: Rx mon buffers allocation failed", soc);
		goto fail;
	}

	if (dp_tx_mon_buffers_alloc(soc)) {
		dp_mon_err("%pK: Tx mon buffers allocation failed", soc);
		goto fail;
	}

	return QDF_STATUS_SUCCESS;
fail:
	dp_mon_soc_detach_2_0(soc);
	return QDF_STATUS_E_NOMEM;
}

static
QDF_STATUS dp_mon_soc_deinit_2_0(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	dp_tx_mon_buf_desc_pool_deinit(soc);
	dp_rx_mon_buf_desc_pool_deinit(soc);

	dp_srng_deinit(soc, &soc->rxdma_mon_buf_ring[0],
		       RXDMA_MONITOR_BUF, 0);

	dp_srng_deinit(soc, &mon_soc->tx_mon_buf_ring,
		       TX_MONITOR_BUF, 0);

	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS dp_mon_soc_init_2_0(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	if (dp_srng_init(soc, &soc->rxdma_mon_buf_ring[0],
			 RXDMA_MONITOR_BUF, 0, 0)) {
		dp_mon_err("%pK: " RNG_ERR "rx_mon_buf_ring", soc);
		goto fail;
	}

	if (dp_srng_init(soc, &mon_soc->tx_mon_buf_ring,
			 TX_MONITOR_BUF, 0, 0)) {
		dp_mon_err("%pK: " RNG_ERR "tx_mon_buf_ring", soc);
		goto fail;
	}

	if (dp_tx_mon_buf_desc_pool_init(soc)) {
		dp_mon_err("%pK: " RNG_ERR "tx mon desc pool init", soc);
		goto fail;
	}

	if (dp_rx_mon_buf_desc_pool_init(soc)) {
		dp_mon_err("%pK: " RNG_ERR "rx mon desc pool init", soc);
		goto fail;
	}

	htt_srng_setup(soc->htt_handle, 0,
		       soc->rxdma_mon_buf_ring[0].hal_srng,
		       RXDMA_MONITOR_BUF);

	htt_srng_setup(soc->htt_handle, 0,
		       mon_soc->tx_mon_buf_ring.hal_srng,
		       TX_MONITOR_BUF);

	return QDF_STATUS_SUCCESS;
fail:
	dp_mon_soc_deinit_2_0(soc);
	return QDF_STATUS_E_FAILURE;
}

static
void dp_pdev_mon_rings_deinit(struct dp_pdev *pdev)
{
}

static
QDF_STATUS dp_pdev_mon_rings_init(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static
void dp_pdev_mon_rings_free(struct dp_pdev *pdev)
{
}

static
QDF_STATUS dp_pdev_mon_rings_alloc(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static
void dp_mon_pdev_free_2_0(struct dp_pdev *pdev)
{
	struct dp_pdev_be *be_pdev = dp_get_be_pdev_from_dp_pdev(pdev);

	qdf_mem_free(be_pdev->monitor_pdev_be);
	be_pdev->monitor_pdev_be = NULL;
}

static
QDF_STATUS dp_mon_pdev_alloc_2_0(struct dp_pdev *pdev)
{
	struct dp_mon_pdev_be *mon_pdev = NULL;
	struct dp_pdev_be *be_pdev = dp_get_be_pdev_from_dp_pdev(pdev);

	mon_pdev = (struct dp_mon_pdev_be *)qdf_mem_malloc(sizeof(*mon_pdev));
	if (!mon_pdev) {
		dp_mon_err("%pK: mem allocation failed", pdev);
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_zero(mon_pdev, sizeof(*mon_pdev));
	be_pdev->monitor_pdev_be = mon_pdev;

	return QDF_STATUS_SUCCESS;
}
#else
static inline
QDF_STATUS dp_mon_htt_srng_setup_2_0(struct dp_soc *soc,
				     struct dp_pdev *pdev,
				     int mac_id,
				     int mac_for_pdev)
{
	return QDF_STATUS_SUCCESS;
}

static uint32_t
dp_rx_mon_process_2_0(struct dp_soc *soc, struct dp_intr *int_ctx,
		      uint32_t mac_id, uint32_t quota)
{
	return 0;
}

static uint32_t
dp_tx_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
		  uint32_t mac_id, uint32_t quota)
{
	return 0;
}

static inline
QDF_STATUS dp_mon_soc_attach_2_0(struct dp_soc *soc)
{
	return status;
}

static inline
QDF_STATUS dp_mon_soc_detach_2_0(struct dp_soc *soc)
{
	return status;
}

static inline
QDF_STATUS dp_mon_soc_init_2_0(struct dp_soc *soc)
{
	return status;
}

static inline
QDF_STATUS dp_mon_soc_deinit_2_0(struct dp_soc *soc)
{
	return status;
}

static inline
void dp_pdev_mon_rings_deinit(struct dp_pdev *pdev)
{
}

static inline
QDF_STATUS dp_pdev_mon_rings_init(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void dp_pdev_mon_rings_free(struct dp_pdev *pdev)
{
}

static inline
QDF_STATUS dp_pdev_mon_rings_alloc(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void dp_mon_pdev_free_2_0(struct dp_pdev *pdev)
{
}

static inline
QDF_STATUS dp_mon_pdev_alloc_2_0(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

struct dp_mon_ops monitor_ops_2_0 = {
	.mon_soc_cfg_init = dp_mon_soc_cfg_init,
	.mon_soc_attach = dp_mon_soc_attach_2_0,
	.mon_soc_detach = dp_mon_soc_detach_2_0,
	.mon_soc_init = dp_mon_soc_init_2_0,
	.mon_soc_deinit = dp_mon_soc_deinit_2_0,
	.mon_pdev_alloc = dp_mon_pdev_alloc_2_0,
	.mon_pdev_free = dp_mon_pdev_free_2_0,
	.mon_pdev_attach = dp_mon_pdev_attach,
	.mon_pdev_detach = dp_mon_pdev_detach,
	.mon_pdev_init = dp_mon_pdev_init,
	.mon_pdev_deinit = dp_mon_pdev_deinit,
	.mon_vdev_attach = dp_mon_vdev_attach,
	.mon_vdev_detach = dp_mon_vdev_detach,
	.mon_peer_attach = dp_mon_peer_attach,
	.mon_peer_detach = dp_mon_peer_detach,
	.mon_config_debug_sniffer = dp_config_debug_sniffer,
	.mon_flush_rings = NULL,
#if !defined(DISABLE_MON_CONFIG)
	.mon_htt_srng_setup = dp_mon_htt_srng_setup_2_0,
#endif
#if defined(DP_CON_MON)
	.mon_service_rings = NULL,
#endif
#ifndef DISABLE_MON_CONFIG
	.mon_rx_process = dp_rx_mon_process_2_0,
	.mon_tx_process = dp_tx_mon_process,
#endif
#if !defined(DISABLE_MON_CONFIG) && defined(MON_ENABLE_DROP_FOR_MAC)
	.mon_drop_packets_for_mac = NULL,
#endif
	.mon_peer_tx_init = NULL,
	.mon_peer_tx_cleanup = NULL,
#ifdef WLAN_TX_PKT_CAPTURE_ENH
	.mon_peer_tid_peer_id_update = NULL,
	.mon_tx_ppdu_stats_attach = dp_tx_ppdu_stats_attach,
	.mon_tx_ppdu_stats_detach = dp_tx_ppdu_stats_detach,
	.mon_tx_capture_debugfs_init = NULL,
	.mon_tx_add_to_comp_queue = NULL,
	.mon_peer_tx_capture_filter_check = NULL,
#endif
#if defined(WDI_EVENT_ENABLE) &&\
	(defined(QCA_ENHANCED_STATS_SUPPORT) || !defined(REMOVE_PKT_LOG))
	.mon_ppdu_stats_ind_handler = NULL,
#endif
	.mon_htt_ppdu_stats_attach = dp_htt_ppdu_stats_attach,
	.mon_htt_ppdu_stats_detach = dp_htt_ppdu_stats_detach,
	.mon_print_pdev_rx_mon_stats = dp_print_pdev_rx_mon_stats,
#ifdef WLAN_TX_PKT_CAPTURE_ENH
	.mon_print_pdev_tx_capture_stats = NULL,
	.mon_config_enh_tx_capture = dp_config_enh_tx_capture,
#endif
#ifdef WLAN_RX_PKT_CAPTURE_ENH
	.mon_config_enh_rx_capture = NULL,
#endif
#ifdef QCA_SUPPORT_BPR
	.mon_set_bpr_enable = dp_set_bpr_enable_2_0,
#endif

#ifdef ATH_SUPPORT_NAC
	.mon_set_filter_neigh_peers = dp_set_filter_neigh_peers,
#endif
#ifdef WLAN_ATF_ENABLE
	.mon_set_atf_stats_enable = dp_set_atf_stats_enable,
#endif
	.mon_set_bsscolor = dp_mon_set_bsscolor,
	.mon_pdev_get_filter_ucast_data = dp_pdev_get_filter_ucast_data,
	.mon_pdev_get_filter_mcast_data = dp_pdev_get_filter_mcast_data,
	.mon_pdev_get_filter_non_data = dp_pdev_get_filter_non_data,
#ifdef WDI_EVENT_ENABLE
	.mon_set_pktlog_wifi3 = dp_set_pktlog_wifi3,
#endif
#if defined(DP_CON_MON) && !defined(REMOVE_PKT_LOG)
	.mon_pktlogmod_exit = dp_pktlogmod_exit,
#endif
	.mon_vdev_set_monitor_mode_buf_rings =
			dp_vdev_set_monitor_mode_buf_rings_2_0,
	.mon_vdev_set_monitor_mode_rings =
			dp_vdev_set_monitor_mode_rings_2_0,
	.mon_neighbour_peers_detach = dp_neighbour_peers_detach,
#ifdef FEATURE_NAC_RSSI
	.mon_filter_neighbour_peer = dp_filter_neighbour_peer,
#endif
	.mon_vdev_timer_init = NULL,
	.mon_vdev_timer_start = NULL,
	.mon_vdev_timer_stop = NULL,
	.mon_vdev_timer_deinit = NULL,
	.mon_reap_timer_init = NULL,
	.mon_reap_timer_start = NULL,
	.mon_reap_timer_stop = NULL,
	.mon_reap_timer_deinit = NULL,
#ifdef QCA_MCOPY_SUPPORT
	.mon_mcopy_check_deliver = NULL,
#endif
	.mon_neighbour_peer_add_ast = NULL,
#ifdef QCA_ENHANCED_STATS_SUPPORT
	.mon_filter_setup_enhanced_stats =
		dp_mon_filter_setup_enhanced_stats_2_0,
	.mon_filter_reset_enhanced_stats =
		dp_mon_filter_reset_enhanced_stats_2_0,
#endif
#ifdef QCA_MCOPY_SUPPORT
	.mon_filter_setup_mcopy_mode = dp_mon_filter_setup_mcopy_mode_2_0,
	.mon_filter_reset_mcopy_mode = dp_mon_filter_reset_mcopy_mode_2_0,
#endif
#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
	.mon_filter_setup_smart_monitor = dp_mon_filter_setup_smart_monitor_2_0,
#endif
#ifdef WLAN_RX_PKT_CAPTURE_ENH
	.mon_filter_setup_rx_enh_capture =
		dp_mon_filter_setup_rx_enh_capture_2_0,
#endif
	.mon_filter_setup_mon_mode = dp_mon_filter_setup_mon_mode_2_0,
	.mon_filter_reset_mon_mode = dp_mon_filter_reset_mon_mode_2_0,
#ifdef WDI_EVENT_ENABLE
	.mon_filter_setup_rx_pkt_log_full =
		dp_mon_filter_setup_rx_pkt_log_full_2_0,
	.mon_filter_reset_rx_pkt_log_full =
		dp_mon_filter_reset_rx_pkt_log_full_2_0,
	.mon_filter_setup_rx_pkt_log_lite =
		dp_mon_filter_setup_rx_pkt_log_lite_2_0,
	.mon_filter_reset_rx_pkt_log_lite =
		dp_mon_filter_reset_rx_pkt_log_lite_2_0,
	.mon_filter_setup_rx_pkt_log_cbf =
		dp_mon_filter_setup_rx_pkt_log_cbf_2_0,
	.mon_filter_reset_rx_pkt_log_cbf =
		dp_mon_filter_reset_rx_pktlog_cbf_2_0,
#endif
	.mon_filter_update = dp_mon_filter_update_2_0,
	.mon_rings_alloc = dp_pdev_mon_rings_alloc,
	.mon_rings_free = dp_pdev_mon_rings_free,
	.mon_rings_init = dp_pdev_mon_rings_init,
	.mon_rings_deinit = dp_pdev_mon_rings_deinit,
	.rx_mon_desc_pool_init = NULL,
	.rx_mon_desc_pool_deinit = NULL,
	.rx_mon_desc_pool_alloc = NULL,
	.rx_mon_desc_pool_free = NULL,
	.rx_mon_buffers_alloc = NULL,
	.rx_mon_buffers_free = NULL,
	.tx_mon_desc_pool_init = NULL,
	.tx_mon_desc_pool_deinit = NULL,
	.tx_mon_desc_pool_alloc = NULL,
	.tx_mon_desc_pool_free = NULL,
};

struct cdp_mon_ops dp_ops_mon_2_0 = {
	.txrx_reset_monitor_mode = dp_reset_monitor_mode,
	/* Added support for HK advance filter */
	.txrx_set_advance_monitor_filter = dp_pdev_set_advance_monitor_filter,
	.txrx_deliver_tx_mgmt = dp_deliver_tx_mgmt,
	.config_full_mon_mode = NULL,
	.soc_config_full_mon_mode = NULL,
};

struct dp_mon_ops *dp_mon_ops_get_2_0(void)
{
	return &monitor_ops_2_0;
}

struct cdp_mon_ops *dp_mon_cdp_ops_get_2_0(void)
{
	return &dp_ops_mon_2_0;
}
