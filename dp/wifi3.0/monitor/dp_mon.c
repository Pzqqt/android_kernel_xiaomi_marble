/*
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.
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
#include "htt_ppdu_stats.h"
#include "dp_cal_client_api.h"
#if defined(DP_CON_MON)
#ifndef REMOVE_PKT_LOG
#include <pktlog_ac_api.h>
#include <pktlog_ac.h>
#endif
#endif
#ifdef FEATURE_PERPKT_INFO
#include "dp_ratetable.h"
#endif

#define DP_INTR_POLL_TIMER_MS	5
#define INVALID_FREE_BUFF 0xffffffff

#ifdef WLAN_RX_PKT_CAPTURE_ENH
#include "dp_rx_mon_feature.h"
#endif /* WLAN_RX_PKT_CAPTURE_ENH */

#ifdef QCA_MCOPY_SUPPORT
static inline void
dp_pdev_disable_mcopy_code(struct dp_pdev *pdev)
{
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	mon_pdev->mcopy_mode = M_COPY_DISABLED;
	mon_pdev->monitor_configured = false;
	mon_pdev->mvdev = NULL;
}

static inline void
dp_reset_mcopy_mode(struct dp_pdev *pdev)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;
	struct cdp_mon_ops *cdp_ops;

	if (mon_pdev->mcopy_mode) {
		cdp_ops = dp_mon_cdp_ops_get(pdev->soc);
		if (cdp_ops  && cdp_ops->config_full_mon_mode)
			cdp_ops->soc_config_full_mon_mode((struct cdp_pdev *)pdev,
							  DP_FULL_MON_ENABLE);
		dp_pdev_disable_mcopy_code(pdev);
		dp_mon_filter_reset_mcopy_mode(pdev);
		status = dp_mon_filter_update(pdev);
		if (status != QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  FL("Failed to reset AM copy mode filters"));
		}
		mon_pdev->monitor_configured = false;
	}
}

static QDF_STATUS
dp_config_mcopy_mode(struct dp_pdev *pdev, int val)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;
	struct dp_mon_ops *mon_ops;
	struct cdp_mon_ops *cdp_ops;

	if (mon_pdev->mvdev)
		return QDF_STATUS_E_RESOURCES;

	mon_pdev->mcopy_mode = val;
	mon_pdev->tx_sniffer_enable = 0;
	mon_pdev->monitor_configured = true;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (!wlan_cfg_is_delay_mon_replenish(pdev->soc->wlan_cfg_ctx)) {
		if (mon_ops && mon_ops->mon_vdev_set_monitor_mode_rings)
			mon_ops->mon_vdev_set_monitor_mode_rings(pdev, true);
	}

	/*
	 * Setup the M copy mode filter.
	 */
	cdp_ops = dp_mon_cdp_ops_get(pdev->soc);
	if (cdp_ops  && cdp_ops->config_full_mon_mode)
		cdp_ops->soc_config_full_mon_mode((struct cdp_pdev *)pdev,
						  DP_FULL_MON_ENABLE);
	dp_mon_filter_setup_mcopy_mode(pdev);
	status = dp_mon_filter_update(pdev);
	if (status != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  FL("Failed to set M_copy mode filters"));
		dp_mon_filter_reset_mcopy_mode(pdev);
		dp_pdev_disable_mcopy_code(pdev);
		return status;
	}

	if (!mon_pdev->pktlog_ppdu_stats)
		dp_h2t_cfg_stats_msg_send(pdev,
					  DP_PPDU_STATS_CFG_SNIFFER,
					  pdev->pdev_id);

	return status;
}
#else
static inline void
dp_reset_mcopy_mode(struct dp_pdev *pdev)
{
}

static inline QDF_STATUS
dp_config_mcopy_mode(struct dp_pdev *pdev, int val)
{
	return QDF_STATUS_E_INVAL;
}
#endif /* QCA_MCOPY_SUPPORT */

QDF_STATUS dp_reset_monitor_mode(struct cdp_soc_t *soc_hdl,
				 uint8_t pdev_id,
				 uint8_t special_monitor)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_mon_pdev *mon_pdev;
	struct cdp_mon_ops *cdp_ops;

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	mon_pdev = pdev->monitor_pdev;

	qdf_spin_lock_bh(&mon_pdev->mon_lock);

	cdp_ops = dp_mon_cdp_ops_get(soc);
	if (cdp_ops  && cdp_ops->soc_config_full_mon_mode)
		cdp_ops->soc_config_full_mon_mode((struct cdp_pdev *)pdev,
						  DP_FULL_MON_DISABLE);
	mon_pdev->mvdev = NULL;
	mon_pdev->monitor_configured = false;

	/*
	 * Lite monitor mode, smart monitor mode and monitor
	 * mode uses this APIs to filter reset and mode disable
	 */
	if (mon_pdev->mcopy_mode) {
#if defined(QCA_MCOPY_SUPPORT)
		dp_pdev_disable_mcopy_code(pdev);
		dp_mon_filter_reset_mcopy_mode(pdev);
#endif /* QCA_MCOPY_SUPPORT */
	} else if (special_monitor) {
#if defined(ATH_SUPPORT_NAC)
		dp_mon_filter_reset_smart_monitor(pdev);
#endif /* ATH_SUPPORT_NAC */
	} else {
		dp_mon_filter_reset_mon_mode(pdev);
	}

	status = dp_mon_filter_update(pdev);
	if (status != QDF_STATUS_SUCCESS) {
		dp_rx_mon_dest_err("%pK: Failed to reset monitor filters",
				   soc);
	}

	qdf_spin_unlock_bh(&mon_pdev->mon_lock);
	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_ADVANCE_MON_FILTER_SUPPORT
QDF_STATUS
dp_pdev_set_advance_monitor_filter(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
				   struct cdp_monitor_filter *filter_val)
{
	/* Many monitor VAPs can exists in a system but only one can be up at
	 * anytime
	 */
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_vdev *vdev;
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_mon_pdev *mon_pdev;

	if (!pdev || !pdev->monitor_pdev)
		return QDF_STATUS_E_FAILURE;

	mon_pdev = pdev->monitor_pdev;
	vdev = mon_pdev->mvdev;

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
		  "pdev=%pK, pdev_id=%d, soc=%pK vdev=%pK",
		  pdev, pdev_id, soc, vdev);

	/*Check if current pdev's monitor_vdev exists */
	if (!mon_pdev->mvdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "vdev=%pK", vdev);
		qdf_assert(vdev);
	}

	/* update filter mode, type in pdev structure */
	mon_pdev->mon_filter_mode = filter_val->mode;
	mon_pdev->fp_mgmt_filter = filter_val->fp_mgmt;
	mon_pdev->fp_ctrl_filter = filter_val->fp_ctrl;
	mon_pdev->fp_data_filter = filter_val->fp_data;
	mon_pdev->mo_mgmt_filter = filter_val->mo_mgmt;
	mon_pdev->mo_ctrl_filter = filter_val->mo_ctrl;
	mon_pdev->mo_data_filter = filter_val->mo_data;

	dp_mon_filter_setup_mon_mode(pdev);
	status = dp_mon_filter_update(pdev);
	if (status != QDF_STATUS_SUCCESS) {
		dp_rx_mon_dest_err("%pK: Failed to set filter for adv mon mode",
				   soc);
		dp_mon_filter_reset_mon_mode(pdev);
	}

	return status;
}
#endif

QDF_STATUS
dp_deliver_tx_mgmt(struct cdp_soc_t *cdp_soc, uint8_t pdev_id, qdf_nbuf_t nbuf)
{
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)cdp_soc,
						   pdev_id);

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	dp_deliver_mgmt_frm(pdev, nbuf);

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_SUPPORT_SCAN_SPCL_VAP_STATS
/**
 * dp_scan_spcl_vap_stats_attach() - alloc spcl vap stats struct
 * @mon_vdev: Datapath mon VDEV handle
 *
 * Return: 0 on success, not 0 on failure
 */
static inline QDF_STATUS
dp_scan_spcl_vap_stats_attach(struct dp_mon_vdev *mon_vdev)
{
	mon_vdev->scan_spcl_vap_stats =
		qdf_mem_malloc(sizeof(struct cdp_scan_spcl_vap_stats));

	if (!mon_vdev->scan_spcl_vap_stats) {
		dp_mon_err("scan spcl vap stats attach fail");
		return QDF_STATUS_E_NOMEM;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_scan_spcl_vap_stats_detach() - free spcl vap stats struct
 * @mon_vdev: Datapath mon VDEV handle
 *
 * Return: void
 */
static inline void
dp_scan_spcl_vap_stats_detach(struct dp_mon_vdev *mon_vdev)
{
	if (mon_vdev->scan_spcl_vap_stats) {
		qdf_mem_free(mon_vdev->scan_spcl_vap_stats);
		mon_vdev->scan_spcl_vap_stats = NULL;
	}
}

/**
 * dp_reset_scan_spcl_vap_stats() - reset spcl vap rx stats
 * @vdev: Datapath VDEV handle
 *
 * Return: void
 */
static inline void
dp_reset_scan_spcl_vap_stats(struct dp_vdev *vdev)
{
	struct dp_mon_vdev *mon_vdev;
	struct dp_mon_pdev *mon_pdev;

	mon_pdev = vdev->pdev->monitor_pdev;
	if (!mon_pdev || !mon_pdev->reset_scan_spcl_vap_stats_enable)
		return;

	mon_vdev = vdev->monitor_vdev;
	if (!mon_vdev || !mon_vdev->scan_spcl_vap_stats)
		return;

	qdf_mem_zero(mon_vdev->scan_spcl_vap_stats,
		     sizeof(struct cdp_scan_spcl_vap_stats));
}

/**
 * dp_get_scan_spcl_vap_stats() - get spcl vap rx stats
 * @soc_hdl: Datapath soc handle
 * @vdev_id: vdev id
 * @stats: structure to hold spcl vap stats
 *
 * Return: 0 on success, not 0 on failure
 */
static QDF_STATUS
dp_get_scan_spcl_vap_stats(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
			   struct cdp_scan_spcl_vap_stats *stats)
{
	struct dp_mon_vdev *mon_vdev = NULL;
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);

	if (!vdev || !stats) {
		if (vdev)
			dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_E_INVAL;
	}

	mon_vdev = vdev->monitor_vdev;
	if (!mon_vdev || !mon_vdev->scan_spcl_vap_stats) {
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_copy(stats, mon_vdev->scan_spcl_vap_stats,
		     sizeof(struct cdp_scan_spcl_vap_stats));

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}
#else
static inline void
dp_reset_scan_spcl_vap_stats(struct dp_vdev *vdev)
{
}

static inline QDF_STATUS
dp_scan_spcl_vap_stats_attach(struct dp_mon_vdev *mon_vdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline void
dp_scan_spcl_vap_stats_detach(struct dp_mon_vdev *mon_vdev)
{
}
#endif

/**
 * dp_vdev_set_monitor_mode() - Set DP VDEV to monitor mode
 * @vdev_handle: Datapath VDEV handle
 * @smart_monitor: Flag to denote if its smart monitor mode
 *
 * Return: 0 on success, not 0 on failure
 */
static QDF_STATUS dp_vdev_set_monitor_mode(struct cdp_soc_t *dp_soc,
					   uint8_t vdev_id,
					   uint8_t special_monitor)
{
	struct dp_soc *soc = (struct dp_soc *)dp_soc;
	struct dp_pdev *pdev;
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_mon_pdev *mon_pdev;
	struct cdp_mon_ops *cdp_ops;

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	pdev = vdev->pdev;

	if (!pdev || !pdev->monitor_pdev)
		return QDF_STATUS_E_FAILURE;

	mon_pdev = pdev->monitor_pdev;

	mon_pdev->mvdev = vdev;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
		  "pdev=%pK, pdev_id=%d, soc=%pK vdev=%pK\n",
		  pdev, pdev->pdev_id, pdev->soc, vdev);

	/*
	 * do not configure monitor buf ring and filter for smart and
	 * lite monitor
	 * for smart monitor filters are added along with first NAC
	 * for lite monitor required configuration done through
	 * dp_set_pdev_param
	 */

	if (special_monitor) {
		status = QDF_STATUS_SUCCESS;
		goto fail;
	}

	if (mon_pdev->scan_spcl_vap_configured)
		dp_reset_scan_spcl_vap_stats(vdev);

	/*Check if current pdev's monitor_vdev exists */
	if (mon_pdev->monitor_configured) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "monitor vap already created vdev=%pK\n", vdev);
		status = QDF_STATUS_E_RESOURCES;
		goto fail;
	}

	mon_pdev->monitor_configured = true;

	cdp_ops = dp_mon_cdp_ops_get(soc);
	if (cdp_ops  && cdp_ops->soc_config_full_mon_mode)
		cdp_ops->soc_config_full_mon_mode((struct cdp_pdev *)pdev,
						  DP_FULL_MON_ENABLE);
	dp_mon_filter_setup_mon_mode(pdev);
	status = dp_mon_filter_update(pdev);
	if (status != QDF_STATUS_SUCCESS) {
		dp_cdp_err("%pK: Failed to reset monitor filters", soc);
		dp_mon_filter_reset_mon_mode(pdev);
		mon_pdev->monitor_configured = false;
		mon_pdev->mvdev = NULL;
	}

fail:
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return status;
}

#ifdef QCA_TX_CAPTURE_SUPPORT
static QDF_STATUS
dp_config_tx_capture_mode(struct dp_pdev *pdev)
{
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	mon_pdev->tx_sniffer_enable = 1;
	mon_pdev->monitor_configured = false;

	if (!mon_pdev->pktlog_ppdu_stats)
		dp_h2t_cfg_stats_msg_send(pdev,
					  DP_PPDU_STATS_CFG_SNIFFER,
					  pdev->pdev_id);

	return QDF_STATUS_SUCCESS;
}
#else
#ifdef QCA_MCOPY_SUPPORT
static QDF_STATUS
dp_config_tx_capture_mode(struct dp_pdev *pdev)
{
	return QDF_STATUS_E_INVAL;
}
#endif
#endif

#if defined(QCA_MCOPY_SUPPORT) || defined(QCA_TX_CAPTURE_SUPPORT)
QDF_STATUS
dp_config_debug_sniffer(struct dp_pdev *pdev, int val)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	/*
	 * Note: The mirror copy mode cannot co-exist with any other
	 * monitor modes. Hence disabling the filter for this mode will
	 * reset the monitor destination ring filters.
	 */
	dp_reset_mcopy_mode(pdev);
	switch (val) {
	case 0:
		mon_pdev->tx_sniffer_enable = 0;
		mon_pdev->monitor_configured = false;

		/*
		 * We don't need to reset the Rx monitor status ring  or call
		 * the API dp_ppdu_ring_reset() if all debug sniffer mode is
		 * disabled. The Rx monitor status ring will be disabled when
		 * the last mode using the monitor status ring get disabled.
		 */
		if (!mon_pdev->pktlog_ppdu_stats &&
		    !mon_pdev->enhanced_stats_en &&
		    !mon_pdev->bpr_enable) {
			dp_h2t_cfg_stats_msg_send(pdev, 0, pdev->pdev_id);
		} else if (mon_pdev->enhanced_stats_en &&
			   !mon_pdev->bpr_enable) {
			dp_h2t_cfg_stats_msg_send(pdev,
						  DP_PPDU_STATS_CFG_ENH_STATS,
						  pdev->pdev_id);
		} else if (!mon_pdev->enhanced_stats_en &&
			   mon_pdev->bpr_enable) {
			dp_h2t_cfg_stats_msg_send(pdev,
						  DP_PPDU_STATS_CFG_BPR_ENH,
						  pdev->pdev_id);
		} else {
			dp_h2t_cfg_stats_msg_send(pdev,
						  DP_PPDU_STATS_CFG_BPR,
						  pdev->pdev_id);
		}
		break;

	case 1:
		status = dp_config_tx_capture_mode(pdev);
		break;
	case 2:
	case 4:
		status = dp_config_mcopy_mode(pdev, val);
		break;

	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "Invalid value, mode not supported");
		status = QDF_STATUS_E_INVAL;
		break;
	}
	return status;
}
#endif

/**
 * dp_monitor_mode_ring_config() - Send the tlv config to fw for monitor buffer
 *                                 ring based on target
 * @soc: soc handle
 * @mac_for_pdev: WIN- pdev_id, MCL- mac id
 * @pdev: physical device handle
 * @ring_num: mac id
 * @htt_tlv_filter: tlv filter
 *
 * Return: zero on success, non-zero on failure
 */
static inline QDF_STATUS
dp_monitor_mode_ring_config(struct dp_soc *soc, uint8_t mac_for_pdev,
			    struct dp_pdev *pdev, uint8_t ring_num,
			    struct htt_rx_ring_tlv_filter htt_tlv_filter)
{
	QDF_STATUS status;

	if (soc->wlan_cfg_ctx->rxdma1_enable)
		status = htt_h2t_rx_ring_cfg(soc->htt_handle, mac_for_pdev,
					     soc->rxdma_mon_buf_ring[ring_num]
					     .hal_srng,
					     RXDMA_MONITOR_BUF,
					     RX_MONITOR_BUFFER_SIZE,
					     &htt_tlv_filter);
	else
		status = htt_h2t_rx_ring_cfg(soc->htt_handle, mac_for_pdev,
					     pdev->rx_mac_buf_ring[ring_num]
					     .hal_srng,
					     RXDMA_BUF, RX_DATA_BUFFER_SIZE,
					     &htt_tlv_filter);

	return status;
}

/**
 * dp_get_mon_vdev_from_pdev_wifi3() - Get vdev id of monitor mode
 * @soc_hdl: datapath soc handle
 * @pdev_id: physical device instance id
 *
 * Return: virtual interface id
 */
static uint8_t dp_get_mon_vdev_from_pdev_wifi3(struct cdp_soc_t *soc_hdl,
		uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);

	if (qdf_unlikely(!pdev || !pdev->monitor_pdev ||
				!pdev->monitor_pdev->mvdev))
		return -EINVAL;

	return pdev->monitor_pdev->mvdev->vdev_id;
}

#if defined(QCA_TX_CAPTURE_SUPPORT) || defined(QCA_ENHANCED_STATS_SUPPORT)
#ifndef WLAN_TX_PKT_CAPTURE_ENH
void dp_deliver_mgmt_frm(struct dp_pdev *pdev, qdf_nbuf_t nbuf)
{
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	if (mon_pdev->tx_sniffer_enable || mon_pdev->mcopy_mode) {
		dp_wdi_event_handler(WDI_EVENT_TX_MGMT_CTRL, pdev->soc,
				     nbuf, HTT_INVALID_PEER,
				     WDI_NO_VAL, pdev->pdev_id);
	} else {
		if (!mon_pdev->bpr_enable)
			qdf_nbuf_free(nbuf);
	}
}
#endif
#endif

QDF_STATUS dp_htt_ppdu_stats_attach(struct dp_pdev *pdev)
{
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	mon_pdev->ppdu_tlv_buf = qdf_mem_malloc(HTT_T2H_MAX_MSG_SIZE);

	if (!mon_pdev->ppdu_tlv_buf) {
		QDF_TRACE_ERROR(QDF_MODULE_ID_DP, "ppdu_tlv_buf alloc fail");
		return QDF_STATUS_E_NOMEM;
	}

	return QDF_STATUS_SUCCESS;
}

void dp_htt_ppdu_stats_detach(struct dp_pdev *pdev)
{
	struct ppdu_info *ppdu_info, *ppdu_info_next;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;


	TAILQ_FOREACH_SAFE(ppdu_info, &mon_pdev->ppdu_info_list,
			   ppdu_info_list_elem, ppdu_info_next) {
		if (!ppdu_info)
			break;
		TAILQ_REMOVE(&mon_pdev->ppdu_info_list,
			     ppdu_info, ppdu_info_list_elem);
		mon_pdev->list_depth--;
		qdf_assert_always(ppdu_info->nbuf);
		qdf_nbuf_free(ppdu_info->nbuf);
		qdf_mem_free(ppdu_info);
	}

	TAILQ_FOREACH_SAFE(ppdu_info, &mon_pdev->sched_comp_ppdu_list,
			   ppdu_info_list_elem, ppdu_info_next) {
		if (!ppdu_info)
			break;
		TAILQ_REMOVE(&mon_pdev->sched_comp_ppdu_list,
			     ppdu_info, ppdu_info_list_elem);
		mon_pdev->sched_comp_list_depth--;
		qdf_assert_always(ppdu_info->nbuf);
		qdf_nbuf_free(ppdu_info->nbuf);
		qdf_mem_free(ppdu_info);
	}

	if (mon_pdev->ppdu_tlv_buf)
		qdf_mem_free(mon_pdev->ppdu_tlv_buf);
}

void
dp_print_pdev_rx_mon_stats(struct dp_pdev *pdev)
{
	struct cdp_pdev_mon_stats *rx_mon_stats;
	uint32_t *stat_ring_ppdu_ids;
	uint32_t *dest_ring_ppdu_ids;
	int i, idx;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	rx_mon_stats = &mon_pdev->rx_mon_stats;

	DP_PRINT_STATS("PDEV Rx Monitor Stats:\n");

	DP_PRINT_STATS("status_ppdu_compl_cnt = %d",
		       rx_mon_stats->status_ppdu_compl);
	DP_PRINT_STATS("status_ppdu_start_cnt = %d",
		       rx_mon_stats->status_ppdu_start);
	DP_PRINT_STATS("status_ppdu_end_cnt = %d",
		       rx_mon_stats->status_ppdu_end);
	DP_PRINT_STATS("status_ppdu_start_mis_cnt = %d",
		       rx_mon_stats->status_ppdu_start_mis);
	DP_PRINT_STATS("status_ppdu_end_mis_cnt = %d",
		       rx_mon_stats->status_ppdu_end_mis);
	DP_PRINT_STATS("status_ppdu_done_cnt = %d",
		       rx_mon_stats->status_ppdu_done);
	DP_PRINT_STATS("dest_ppdu_done_cnt = %d",
		       rx_mon_stats->dest_ppdu_done);
	DP_PRINT_STATS("dest_mpdu_done_cnt = %d",
		       rx_mon_stats->dest_mpdu_done);
	DP_PRINT_STATS("tlv_tag_status_err_cnt = %u",
		       rx_mon_stats->tlv_tag_status_err);
	DP_PRINT_STATS("mon status DMA not done WAR count= %u",
		       rx_mon_stats->status_buf_done_war);
	DP_PRINT_STATS("dest_mpdu_drop_cnt = %d",
		       rx_mon_stats->dest_mpdu_drop);
	DP_PRINT_STATS("dup_mon_linkdesc_cnt = %d",
		       rx_mon_stats->dup_mon_linkdesc_cnt);
	DP_PRINT_STATS("dup_mon_buf_cnt = %d",
		       rx_mon_stats->dup_mon_buf_cnt);
	DP_PRINT_STATS("mon_rx_buf_reaped = %u",
		       rx_mon_stats->mon_rx_bufs_reaped_dest);
	DP_PRINT_STATS("mon_rx_buf_replenished = %u",
		       rx_mon_stats->mon_rx_bufs_replenished_dest);
	DP_PRINT_STATS("ppdu_id_mismatch = %u",
		       rx_mon_stats->ppdu_id_mismatch);
	DP_PRINT_STATS("mpdu_ppdu_id_match_cnt = %d",
		       rx_mon_stats->ppdu_id_match);
	DP_PRINT_STATS("ppdus dropped frm status ring = %d",
		       rx_mon_stats->status_ppdu_drop);
	DP_PRINT_STATS("ppdus dropped frm dest ring = %d",
		       rx_mon_stats->dest_ppdu_drop);
	stat_ring_ppdu_ids =
		(uint32_t *)qdf_mem_malloc(sizeof(uint32_t) * MAX_PPDU_ID_HIST);
	dest_ring_ppdu_ids =
		(uint32_t *)qdf_mem_malloc(sizeof(uint32_t) * MAX_PPDU_ID_HIST);

	if (!stat_ring_ppdu_ids || !dest_ring_ppdu_ids)
		DP_PRINT_STATS("Unable to allocate ppdu id hist mem\n");

	qdf_spin_lock_bh(&mon_pdev->mon_lock);
	idx = rx_mon_stats->ppdu_id_hist_idx;
	qdf_mem_copy(stat_ring_ppdu_ids,
		     rx_mon_stats->stat_ring_ppdu_id_hist,
		     sizeof(uint32_t) * MAX_PPDU_ID_HIST);
	qdf_mem_copy(dest_ring_ppdu_ids,
		     rx_mon_stats->dest_ring_ppdu_id_hist,
		     sizeof(uint32_t) * MAX_PPDU_ID_HIST);
	qdf_spin_unlock_bh(&mon_pdev->mon_lock);

	DP_PRINT_STATS("PPDU Id history:");
	DP_PRINT_STATS("stat_ring_ppdu_ids\t dest_ring_ppdu_ids");
	for (i = 0; i < MAX_PPDU_ID_HIST; i++) {
		idx = (idx + 1) & (MAX_PPDU_ID_HIST - 1);
		DP_PRINT_STATS("%*u\t%*u", 16,
			       rx_mon_stats->stat_ring_ppdu_id_hist[idx], 16,
			       rx_mon_stats->dest_ring_ppdu_id_hist[idx]);
	}
	qdf_mem_free(stat_ring_ppdu_ids);
	qdf_mem_free(dest_ring_ppdu_ids);
	DP_PRINT_STATS("mon_rx_dest_stuck = %d",
		       rx_mon_stats->mon_rx_dest_stuck);
}

#ifdef QCA_SUPPORT_BPR
QDF_STATUS
dp_set_bpr_enable(struct dp_pdev *pdev, int val)
{
	struct dp_mon_ops *mon_ops;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_set_bpr_enable)
		return mon_ops->mon_set_bpr_enable(pdev, val);

	return QDF_STATUS_E_FAILURE;
}
#endif

#ifdef WDI_EVENT_ENABLE
int dp_set_pktlog_wifi3(struct dp_pdev *pdev, uint32_t event,
		        bool enable)
{
	struct dp_soc *soc = NULL;
	int max_mac_rings = wlan_cfg_get_num_mac_rings
					(pdev->wlan_cfg_ctx);
	uint8_t mac_id = 0;
	struct dp_mon_soc *mon_soc;
	struct dp_mon_ops *mon_ops;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	soc = pdev->soc;
	mon_soc = soc->monitor_soc;
	mon_ops = dp_mon_ops_get(soc);

	if (!mon_ops)
		return 0;

	dp_update_num_mac_rings_for_dbs(soc, &max_mac_rings);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  FL("Max_mac_rings %d "),
		  max_mac_rings);

	if (enable) {
		switch (event) {
		case WDI_EVENT_RX_DESC:
			if (mon_pdev->mvdev) {
				/* Nothing needs to be done if monitor mode is
				 * enabled
				 */
				mon_pdev->rx_pktlog_mode = DP_RX_PKTLOG_FULL;
				return 0;
			}

			if (mon_pdev->rx_pktlog_mode != DP_RX_PKTLOG_FULL) {
				mon_pdev->rx_pktlog_mode = DP_RX_PKTLOG_FULL;
				dp_mon_filter_setup_rx_pkt_log_full(pdev);
				if (dp_mon_filter_update(pdev) !=
						QDF_STATUS_SUCCESS) {
					dp_cdp_err("%pK: Pktlog full filters set failed", soc);
					dp_mon_filter_reset_rx_pkt_log_full(pdev);
					mon_pdev->rx_pktlog_mode =
							DP_RX_PKTLOG_DISABLED;
					return 0;
				}

				if (mon_soc->reap_timer_init &&
				    (!dp_mon_is_enable_reap_timer_non_pkt(pdev)))
					qdf_timer_mod(&mon_soc->mon_reap_timer,
						      DP_INTR_POLL_TIMER_MS);
			}
			break;

		case WDI_EVENT_LITE_RX:
			if (mon_pdev->mvdev) {
				/* Nothing needs to be done if monitor mode is
				 * enabled
				 */
				mon_pdev->rx_pktlog_mode = DP_RX_PKTLOG_LITE;
				return 0;
			}
			if (mon_pdev->rx_pktlog_mode != DP_RX_PKTLOG_LITE) {
				mon_pdev->rx_pktlog_mode = DP_RX_PKTLOG_LITE;

				/*
				 * Set the packet log lite mode filter.
				 */
				dp_mon_filter_setup_rx_pkt_log_lite(pdev);
				if (dp_mon_filter_update(pdev) !=
				    QDF_STATUS_SUCCESS) {
					dp_cdp_err("%pK: Pktlog lite filters set failed", soc);
					dp_mon_filter_reset_rx_pkt_log_lite(pdev);
					mon_pdev->rx_pktlog_mode =
						DP_RX_PKTLOG_DISABLED;
					return 0;
				}

				if (mon_soc->reap_timer_init &&
				    (!dp_mon_is_enable_reap_timer_non_pkt(pdev)))
					qdf_timer_mod(&mon_soc->mon_reap_timer,
						      DP_INTR_POLL_TIMER_MS);
			}
			break;

		case WDI_EVENT_LITE_T2H:
			for (mac_id = 0; mac_id < max_mac_rings; mac_id++) {
				int mac_for_pdev = dp_get_mac_id_for_pdev(
							mac_id,	pdev->pdev_id);

				mon_pdev->pktlog_ppdu_stats = true;
				dp_h2t_cfg_stats_msg_send(pdev,
					DP_PPDU_TXLITE_STATS_BITMASK_CFG,
					mac_for_pdev);
			}
			break;

		case WDI_EVENT_RX_CBF:
			if (mon_pdev->mvdev) {
				/* Nothing needs to be done if monitor mode is
				 * enabled
				 */
				dp_mon_info("Mon mode, CBF setting filters");
				mon_pdev->rx_pktlog_cbf = true;
				return 0;
			}
			if (!mon_pdev->rx_pktlog_cbf) {
				mon_pdev->rx_pktlog_cbf = true;
				mon_pdev->monitor_configured = true;
				if (mon_ops->mon_vdev_set_monitor_mode_buf_rings)
					mon_ops->mon_vdev_set_monitor_mode_buf_rings(pdev);
				/*
				 * Set the packet log lite mode filter.
				 */
				qdf_info("Non mon mode: Enable destination ring");

				dp_mon_filter_setup_rx_pkt_log_cbf(pdev);
				if (dp_mon_filter_update(pdev) !=
				    QDF_STATUS_SUCCESS) {
					dp_mon_err("Pktlog set CBF filters failed");
					dp_mon_filter_reset_rx_pktlog_cbf(pdev);
					mon_pdev->rx_pktlog_mode =
						DP_RX_PKTLOG_DISABLED;
					mon_pdev->monitor_configured = false;
					return 0;
				}

				if (mon_soc->reap_timer_init &&
				    !dp_mon_is_enable_reap_timer_non_pkt(pdev))
					qdf_timer_mod(&mon_soc->mon_reap_timer,
						      DP_INTR_POLL_TIMER_MS);
			}
			break;

#ifdef QCA_WIFI_QCN9224
		case WDI_EVENT_HYBRID_TX:
			if (mon_pdev->mvdev) {
				/* Nothing needs to be done if monitor mode is
				 * enabled
				 */
				mon_pdev->pktlog_hybrid_mode = true;
				return 0;
			}

			if (!mon_pdev->pktlog_hybrid_mode) {
				mon_pdev->pktlog_hybrid_mode = true;
				dp_mon_filter_setup_pktlog_hybrid(pdev);
				if (dp_mon_filter_update(pdev) !=
				    QDF_STATUS_SUCCESS) {
					dp_cdp_err("Set hybrid filters failed");
					dp_mon_filter_reset_pktlog_hybrid(pdev);
					mon_pdev->rx_pktlog_mode =
						DP_RX_PKTLOG_DISABLED;
					return 0;
				}

				if (mon_soc->reap_timer_init &&
				    !dp_mon_is_enable_reap_timer_non_pkt(pdev))
					qdf_timer_mod(&mon_soc->mon_reap_timer,
						      DP_INTR_POLL_TIMER_MS);
			}
			break;
#endif

		default:
			/* Nothing needs to be done for other pktlog types */
			break;
		}
	} else {
		switch (event) {
		case WDI_EVENT_RX_DESC:
		case WDI_EVENT_LITE_RX:
			if (mon_pdev->mvdev) {
				/* Nothing needs to be done if monitor mode is
				 * enabled
				 */
				mon_pdev->rx_pktlog_mode =
						DP_RX_PKTLOG_DISABLED;
				return 0;
			}
			if (mon_pdev->rx_pktlog_mode != DP_RX_PKTLOG_DISABLED) {
				mon_pdev->rx_pktlog_mode =
						DP_RX_PKTLOG_DISABLED;
				dp_mon_filter_reset_rx_pkt_log_full(pdev);
				if (dp_mon_filter_update(pdev) !=
						QDF_STATUS_SUCCESS) {
					dp_cdp_err("%pK: Pktlog filters reset failed", soc);
					return 0;
				}

				dp_mon_filter_reset_rx_pkt_log_lite(pdev);
				if (dp_mon_filter_update(pdev) !=
						QDF_STATUS_SUCCESS) {
					dp_cdp_err("%pK: Pktlog filters reset failed", soc);
					return 0;
				}

				if (mon_soc->reap_timer_init &&
				    (!dp_mon_is_enable_reap_timer_non_pkt(pdev)))
					qdf_timer_stop(&mon_soc->mon_reap_timer);
			}
			break;
		case WDI_EVENT_LITE_T2H:
			/*
			 * To disable HTT_H2T_MSG_TYPE_PPDU_STATS_CFG in FW
			 * passing value 0. Once these macros will define in htt
			 * header file will use proper macros
			 */
			for (mac_id = 0; mac_id < max_mac_rings; mac_id++) {
				int mac_for_pdev =
						dp_get_mac_id_for_pdev(mac_id,
								pdev->pdev_id);

				mon_pdev->pktlog_ppdu_stats = false;
				if (!mon_pdev->enhanced_stats_en &&
				    !mon_pdev->tx_sniffer_enable &&
				    !mon_pdev->mcopy_mode) {
					dp_h2t_cfg_stats_msg_send(pdev, 0,
								  mac_for_pdev);
				} else if (mon_pdev->tx_sniffer_enable ||
					   mon_pdev->mcopy_mode) {
					dp_h2t_cfg_stats_msg_send(pdev,
						DP_PPDU_STATS_CFG_SNIFFER,
						mac_for_pdev);
				} else if (mon_pdev->enhanced_stats_en) {
					dp_h2t_cfg_stats_msg_send(pdev,
						DP_PPDU_STATS_CFG_ENH_STATS,
						mac_for_pdev);
				}
			}

			break;
		case WDI_EVENT_RX_CBF:
			mon_pdev->rx_pktlog_cbf = false;
			break;

#ifdef QCA_WIFI_QCN9224
		case WDI_EVENT_HYBRID_TX:
			mon_pdev->pktlog_hybrid_mode = false;
			break;
#endif

		default:
			/* Nothing needs to be done for other pktlog types */
			break;
		}
	}
	return 0;
}
#endif

/* MCL specific functions */
#if defined(DP_CON_MON) && !defined(REMOVE_PKT_LOG)
void dp_pktlogmod_exit(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	struct hif_opaque_softc *scn = soc->hif_handle;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	if (!scn) {
		dp_mon_err("Invalid hif(scn) handle");
		return;
	}

	/* stop mon_reap_timer if it has been started */
	if (mon_pdev->rx_pktlog_mode != DP_RX_PKTLOG_DISABLED &&
	    mon_soc->reap_timer_init &&
	    (!dp_mon_is_enable_reap_timer_non_pkt(pdev)))
		qdf_timer_sync_cancel(&mon_soc->mon_reap_timer);

	pktlogmod_exit(scn);
	mon_pdev->pkt_log_init = false;
}
#endif /*DP_CON_MON*/

#ifdef WDI_EVENT_ENABLE
QDF_STATUS dp_peer_stats_notify(struct dp_pdev *dp_pdev, struct dp_peer *peer)
{
	struct cdp_interface_peer_stats peer_stats_intf;
	struct cdp_peer_stats *peer_stats = &peer->stats;

	if (!peer->vdev)
		return QDF_STATUS_E_FAULT;

	qdf_mem_zero(&peer_stats_intf, sizeof(peer_stats_intf));
	if (peer_stats->rx.last_snr != peer_stats->rx.snr)
		peer_stats_intf.rssi_changed = true;

	if ((peer_stats->rx.snr && peer_stats_intf.rssi_changed) ||
	    (peer_stats->tx.tx_rate &&
	     peer_stats->tx.tx_rate != peer_stats->tx.last_tx_rate)) {
		qdf_mem_copy(peer_stats_intf.peer_mac, peer->mac_addr.raw,
			     QDF_MAC_ADDR_SIZE);
		peer_stats_intf.vdev_id = peer->vdev->vdev_id;
		peer_stats_intf.last_peer_tx_rate = peer_stats->tx.last_tx_rate;
		peer_stats_intf.peer_tx_rate = peer_stats->tx.tx_rate;
		peer_stats_intf.peer_rssi = peer_stats->rx.snr;
		peer_stats_intf.tx_packet_count = peer_stats->tx.ucast.num;
		peer_stats_intf.rx_packet_count = peer_stats->rx.to_stack.num;
		peer_stats_intf.tx_byte_count = peer_stats->tx.tx_success.bytes;
		peer_stats_intf.rx_byte_count = peer_stats->rx.to_stack.bytes;
		peer_stats_intf.per = peer_stats->tx.last_per;
		peer_stats_intf.ack_rssi = peer_stats->tx.last_ack_rssi;
		peer_stats_intf.free_buff = INVALID_FREE_BUFF;
		dp_wdi_event_handler(WDI_EVENT_PEER_STATS, dp_pdev->soc,
				     (void *)&peer_stats_intf, 0,
				     WDI_NO_VAL, dp_pdev->pdev_id);
	}

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef FEATURE_NAC_RSSI
/**
 * dp_rx_nac_filter(): Function to perform filtering of non-associated
 * clients
 * @pdev: DP pdev handle
 * @rx_pkt_hdr: Rx packet Header
 *
 * return: dp_vdev*
 */
static
struct dp_vdev *dp_rx_nac_filter(struct dp_pdev *pdev,
				 uint8_t *rx_pkt_hdr)
{
	struct ieee80211_frame *wh;
	struct dp_neighbour_peer *peer = NULL;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	wh = (struct ieee80211_frame *)rx_pkt_hdr;

	if ((wh->i_fc[1] & IEEE80211_FC1_DIR_MASK) != IEEE80211_FC1_DIR_TODS)
		return NULL;

	qdf_spin_lock_bh(&mon_pdev->neighbour_peer_mutex);
	TAILQ_FOREACH(peer, &mon_pdev->neighbour_peers_list,
		      neighbour_peer_list_elem) {
		if (qdf_mem_cmp(&peer->neighbour_peers_macaddr.raw[0],
				wh->i_addr2, QDF_MAC_ADDR_SIZE) == 0) {
			dp_rx_debug("%pK: NAC configuration matched for mac-%2x:%2x:%2x:%2x:%2x:%2x",
				    pdev->soc,
				    peer->neighbour_peers_macaddr.raw[0],
				    peer->neighbour_peers_macaddr.raw[1],
				    peer->neighbour_peers_macaddr.raw[2],
				    peer->neighbour_peers_macaddr.raw[3],
				    peer->neighbour_peers_macaddr.raw[4],
				    peer->neighbour_peers_macaddr.raw[5]);

				qdf_spin_unlock_bh(&mon_pdev->neighbour_peer_mutex);

			return mon_pdev->mvdev;
		}
	}
	qdf_spin_unlock_bh(&mon_pdev->neighbour_peer_mutex);

	return NULL;
}

QDF_STATUS dp_filter_neighbour_peer(struct dp_pdev *pdev,
				    uint8_t *rx_pkt_hdr)
{
	struct dp_vdev *vdev = NULL;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	if (mon_pdev->filter_neighbour_peers) {
		/* Next Hop scenario not yet handle */
		vdev = dp_rx_nac_filter(pdev, rx_pkt_hdr);
		if (vdev) {
			dp_rx_mon_deliver(pdev->soc, pdev->pdev_id,
					  pdev->invalid_peer_head_msdu,
					  pdev->invalid_peer_tail_msdu);

			pdev->invalid_peer_head_msdu = NULL;
			pdev->invalid_peer_tail_msdu = NULL;
			return QDF_STATUS_SUCCESS;
		}
	}

	return QDF_STATUS_E_FAILURE;
}
#endif

#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
/*
 * dp_update_filter_neighbour_peers() - set neighbour peers(nac clients)
 * address for smart mesh filtering
 * @txrx_soc: cdp soc handle
 * @vdev_id: id of virtual device object
 * @cmd: Add/Del command
 * @macaddr: nac client mac address
 *
 * Return: success/failure
 */
static int dp_update_filter_neighbour_peers(struct cdp_soc_t *soc_hdl,
					    uint8_t vdev_id,
					    uint32_t cmd, uint8_t *macaddr)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_pdev *pdev;
	struct dp_neighbour_peer *peer = NULL;
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);
	struct dp_mon_pdev *mon_pdev;

	if (!vdev || !macaddr)
		goto fail0;

	pdev = vdev->pdev;

	if (!pdev)
		goto fail0;

	mon_pdev = pdev->monitor_pdev;

	/* Store address of NAC (neighbour peer) which will be checked
	 * against TA of received packets.
	 */
	if (cmd == DP_NAC_PARAM_ADD) {
		peer = (struct dp_neighbour_peer *)qdf_mem_malloc(
				sizeof(*peer));

		if (!peer) {
			dp_cdp_err("%pK: DP neighbour peer node memory allocation failed"
				   , soc);
			goto fail0;
		}

		qdf_mem_copy(&peer->neighbour_peers_macaddr.raw[0],
			     macaddr, QDF_MAC_ADDR_SIZE);
		peer->vdev = vdev;

		qdf_spin_lock_bh(&mon_pdev->neighbour_peer_mutex);

		/* add this neighbour peer into the list */
		TAILQ_INSERT_TAIL(&mon_pdev->neighbour_peers_list, peer,
				  neighbour_peer_list_elem);
		qdf_spin_unlock_bh(&mon_pdev->neighbour_peer_mutex);

		/* first neighbour */
		if (!mon_pdev->neighbour_peers_added) {
			QDF_STATUS status = QDF_STATUS_SUCCESS;

			mon_pdev->neighbour_peers_added = true;
			dp_mon_filter_setup_smart_monitor(pdev);
			status = dp_mon_filter_update(pdev);
			if (status != QDF_STATUS_SUCCESS) {
				dp_cdp_err("%pK: smart mon filter setup failed",
					   soc);
				dp_mon_filter_reset_smart_monitor(pdev);
				mon_pdev->neighbour_peers_added = false;
			}
		}

	} else if (cmd == DP_NAC_PARAM_DEL) {
		qdf_spin_lock_bh(&mon_pdev->neighbour_peer_mutex);
		TAILQ_FOREACH(peer, &mon_pdev->neighbour_peers_list,
			      neighbour_peer_list_elem) {
			if (!qdf_mem_cmp(&peer->neighbour_peers_macaddr.raw[0],
					 macaddr, QDF_MAC_ADDR_SIZE)) {
				/* delete this peer from the list */
				TAILQ_REMOVE(&mon_pdev->neighbour_peers_list,
					     peer, neighbour_peer_list_elem);
				qdf_mem_free(peer);
				break;
			}
		}
		/* last neighbour deleted */
		if (TAILQ_EMPTY(&mon_pdev->neighbour_peers_list)) {
			QDF_STATUS status = QDF_STATUS_SUCCESS;

			dp_mon_filter_reset_smart_monitor(pdev);
			status = dp_mon_filter_update(pdev);
			if (status != QDF_STATUS_SUCCESS) {
				dp_cdp_err("%pK: smart mon filter clear failed",
					   soc);
			}
			mon_pdev->neighbour_peers_added = false;
		}
		qdf_spin_unlock_bh(&mon_pdev->neighbour_peer_mutex);
	}
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return 1;

fail0:
	if (vdev)
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return 0;
}
#endif /* ATH_SUPPORT_NAC_RSSI || ATH_SUPPORT_NAC */

#ifdef ATH_SUPPORT_NAC_RSSI
/**
 * dp_vdev_get_neighbour_rssi(): Store RSSI for configured NAC
 * @soc_hdl: DP soc handle
 * @vdev_id: id of DP vdev handle
 * @mac_addr: neighbour mac
 * @rssi: rssi value
 *
 * Return: 0 for success. nonzero for failure.
 */
static QDF_STATUS  dp_vdev_get_neighbour_rssi(struct cdp_soc_t *soc_hdl,
					      uint8_t vdev_id,
					      char *mac_addr,
					      uint8_t *rssi)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);
	struct dp_pdev *pdev;
	struct dp_neighbour_peer *peer = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct dp_mon_pdev *mon_pdev;

	if (!vdev)
		return status;

	pdev = vdev->pdev;
	mon_pdev = pdev->monitor_pdev;

	*rssi = 0;
	qdf_spin_lock_bh(&mon_pdev->neighbour_peer_mutex);
	TAILQ_FOREACH(peer, &mon_pdev->neighbour_peers_list,
		      neighbour_peer_list_elem) {
		if (qdf_mem_cmp(&peer->neighbour_peers_macaddr.raw[0],
				mac_addr, QDF_MAC_ADDR_SIZE) == 0) {
			*rssi = peer->rssi;
			status = QDF_STATUS_SUCCESS;
			break;
		}
	}
	qdf_spin_unlock_bh(&mon_pdev->neighbour_peer_mutex);
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return status;
}

static QDF_STATUS
dp_config_for_nac_rssi(struct cdp_soc_t *cdp_soc,
		       uint8_t vdev_id,
		       enum cdp_nac_param_cmd cmd, char *bssid,
		       char *client_macaddr,
		       uint8_t chan_num)
{
	struct dp_soc *soc = (struct dp_soc *)cdp_soc;
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_CDP);
	struct dp_pdev *pdev;
	struct dp_mon_pdev *mon_pdev;

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	pdev = (struct dp_pdev *)vdev->pdev;

	mon_pdev = pdev->monitor_pdev;
	mon_pdev->nac_rssi_filtering = 1;
	/* Store address of NAC (neighbour peer) which will be checked
	 * against TA of received packets.
	 */

	if (cmd == CDP_NAC_PARAM_ADD) {
		dp_update_filter_neighbour_peers(cdp_soc, vdev->vdev_id,
						 DP_NAC_PARAM_ADD,
						 (uint8_t *)client_macaddr);
	} else if (cmd == CDP_NAC_PARAM_DEL) {
		dp_update_filter_neighbour_peers(cdp_soc, vdev->vdev_id,
						 DP_NAC_PARAM_DEL,
						 (uint8_t *)client_macaddr);
	}

	if (soc->cdp_soc.ol_ops->config_bssid_in_fw_for_nac_rssi)
		soc->cdp_soc.ol_ops->config_bssid_in_fw_for_nac_rssi
			(soc->ctrl_psoc, pdev->pdev_id,
			 vdev->vdev_id, cmd, bssid, client_macaddr);

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
/*
 * dp_cfr_filter() -  Configure HOST RX monitor status ring for CFR
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 * @enable: Enable/Disable CFR
 * @filter_val: Flag to select Filter for monitor mode
 */
static void dp_cfr_filter(struct cdp_soc_t *soc_hdl,
			  uint8_t pdev_id,
			  bool enable,
			  struct cdp_monitor_filter *filter_val)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev = NULL;
	struct htt_rx_ring_tlv_filter htt_tlv_filter = {0};
	int max_mac_rings;
	uint8_t mac_id = 0;
	struct dp_mon_pdev *mon_pdev;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	if (!pdev) {
		dp_mon_err("pdev is NULL");
		return;
	}

	mon_pdev = pdev->monitor_pdev;

	if (mon_pdev->mvdev) {
		dp_mon_info("No action is needed since mon mode is enabled\n");
		return;
	}
	soc = pdev->soc;
	pdev->cfr_rcc_mode = false;
	max_mac_rings = wlan_cfg_get_num_mac_rings(pdev->wlan_cfg_ctx);
	dp_update_num_mac_rings_for_dbs(soc, &max_mac_rings);

	dp_mon_debug("Max_mac_rings %d", max_mac_rings);
	dp_mon_info("enable : %d, mode: 0x%x", enable, filter_val->mode);

	if (enable) {
		pdev->cfr_rcc_mode = true;

		htt_tlv_filter.ppdu_start = 1;
		htt_tlv_filter.ppdu_end = 1;
		htt_tlv_filter.ppdu_end_user_stats = 1;
		htt_tlv_filter.ppdu_end_user_stats_ext = 1;
		htt_tlv_filter.ppdu_end_status_done = 1;
		htt_tlv_filter.mpdu_start = 1;
		htt_tlv_filter.offset_valid = false;

		htt_tlv_filter.enable_fp =
			(filter_val->mode & MON_FILTER_PASS) ? 1 : 0;
		htt_tlv_filter.enable_md = 0;
		htt_tlv_filter.enable_mo =
			(filter_val->mode & MON_FILTER_OTHER) ? 1 : 0;
		htt_tlv_filter.fp_mgmt_filter = filter_val->fp_mgmt;
		htt_tlv_filter.fp_ctrl_filter = filter_val->fp_ctrl;
		htt_tlv_filter.fp_data_filter = filter_val->fp_data;
		htt_tlv_filter.mo_mgmt_filter = filter_val->mo_mgmt;
		htt_tlv_filter.mo_ctrl_filter = filter_val->mo_ctrl;
		htt_tlv_filter.mo_data_filter = filter_val->mo_data;
	}

	for (mac_id = 0;
	     mac_id  < soc->wlan_cfg_ctx->num_rxdma_status_rings_per_pdev;
	     mac_id++) {
		int mac_for_pdev =
			dp_get_mac_id_for_pdev(mac_id,
					       pdev->pdev_id);

		htt_h2t_rx_ring_cfg(soc->htt_handle,
				    mac_for_pdev,
				    soc->rxdma_mon_status_ring[mac_id]
				    .hal_srng,
				    RXDMA_MONITOR_STATUS,
				    RX_MON_STATUS_BUF_SIZE,
				    &htt_tlv_filter);
	}
}

/*
 * dp_enable_mon_reap_timer() - enable/disable reap timer
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of objmgr pdev
 * @enable: Enable/Disable reap timer of monitor status ring
 *
 * Return: none
 */
static void
dp_enable_mon_reap_timer(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			 bool enable)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev = NULL;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
	struct dp_mon_pdev *mon_pdev;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	if (!pdev) {
		dp_mon_err("pdev is NULL");
		return;
	}

	mon_pdev = pdev->monitor_pdev;
	mon_pdev->enable_reap_timer_non_pkt = enable;
	if (mon_pdev->rx_pktlog_mode != DP_RX_PKTLOG_DISABLED) {
		dp_mon_debug("pktlog enabled %d", mon_pdev->rx_pktlog_mode);
		return;
	}

	if (!mon_soc->reap_timer_init) {
		dp_mon_err("reap timer not init");
		return;
	}

	if (enable)
		qdf_timer_mod(&mon_soc->mon_reap_timer,
			      DP_INTR_POLL_TIMER_MS);
	else
		qdf_timer_sync_cancel(&mon_soc->mon_reap_timer);
}
#endif

#if defined(DP_CON_MON)
#ifndef REMOVE_PKT_LOG
/**
 * dp_pkt_log_init() - API to initialize packet log
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 * @scn: HIF context
 *
 * Return: none
 */
void dp_pkt_log_init(struct cdp_soc_t *soc_hdl, uint8_t pdev_id, void *scn)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *handle =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	struct dp_mon_pdev *mon_pdev;

	if (!handle) {
		dp_mon_err("pdev handle is NULL");
		return;
	}

	mon_pdev = handle->monitor_pdev;

	if (mon_pdev->pkt_log_init) {
		dp_mon_err("%pK: Packet log not initialized", soc);
		return;
	}

	pktlog_sethandle(&mon_pdev->pl_dev, scn);
	pktlog_set_pdev_id(mon_pdev->pl_dev, pdev_id);
	pktlog_set_callback_regtype(PKTLOG_DEFAULT_CALLBACK_REGISTRATION);

	if (pktlogmod_init(scn)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s: pktlogmod_init failed", __func__);
		mon_pdev->pkt_log_init = false;
	} else {
		mon_pdev->pkt_log_init = true;
	}
}

/**
 * dp_pkt_log_con_service() - connect packet log service
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 * @scn: device context
 *
 * Return: none
 */
static void dp_pkt_log_con_service(struct cdp_soc_t *soc_hdl,
				   uint8_t pdev_id, void *scn)
{
	dp_pkt_log_init(soc_hdl, pdev_id, scn);
	pktlog_htc_attach();
}

/**
 * dp_pkt_log_exit() - Wrapper API to cleanup pktlog info
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of data path pdev handle
 *
 * Return: none
 */
static void dp_pkt_log_exit(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);

	if (!pdev) {
		dp_err("pdev handle is NULL");
		return;
	}

	dp_pktlogmod_exit(pdev);
}

#else
static void dp_pkt_log_con_service(struct cdp_soc_t *soc_hdl,
				   uint8_t pdev_id, void *scn)
{
}

static void dp_pkt_log_exit(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
}
#endif
#endif

void dp_neighbour_peers_detach(struct dp_pdev *pdev)
{
	struct dp_neighbour_peer *peer = NULL;
	struct dp_neighbour_peer *temp_peer = NULL;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	TAILQ_FOREACH_SAFE(peer, &mon_pdev->neighbour_peers_list,
			   neighbour_peer_list_elem, temp_peer) {
		/* delete this peer from the list */
		TAILQ_REMOVE(&mon_pdev->neighbour_peers_list,
			     peer, neighbour_peer_list_elem);
		qdf_mem_free(peer);
	}

	qdf_spinlock_destroy(&mon_pdev->neighbour_peer_mutex);
}

/*
 * is_ppdu_txrx_capture_enabled() - API to check both pktlog and debug_sniffer
 *                              modes are enabled or not.
 * @dp_pdev: dp pdev handle.
 *
 * Return: bool
 */
static inline bool is_ppdu_txrx_capture_enabled(struct dp_pdev *pdev)
{
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	if (!mon_pdev->pktlog_ppdu_stats && !mon_pdev->tx_sniffer_enable &&
	    !mon_pdev->mcopy_mode)
		return true;
	else
		return false;
}

#ifdef QCA_ENHANCED_STATS_SUPPORT
/*
 * dp_enable_enhanced_stats()- API to enable enhanced statistcs
 * @soc_handle: DP_SOC handle
 * @pdev_id: id of DP_PDEV handle
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dp_enable_enhanced_stats(struct cdp_soc_t *soc, uint8_t pdev_id)
{
	struct dp_pdev *pdev = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_mon_pdev *mon_pdev;

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						  pdev_id);

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	mon_pdev = pdev->monitor_pdev;

	if (!mon_pdev)
		return QDF_STATUS_E_FAILURE;

	if (mon_pdev->enhanced_stats_en == 0)
		dp_cal_client_timer_start(mon_pdev->cal_client_ctx);

	mon_pdev->enhanced_stats_en = 1;

	dp_mon_filter_setup_enhanced_stats(pdev);
	status = dp_mon_filter_update(pdev);
	if (status != QDF_STATUS_SUCCESS) {
		dp_cdp_err("%pK: Failed to set enhanced mode filters", soc);
		dp_mon_filter_reset_enhanced_stats(pdev);
		dp_cal_client_timer_stop(mon_pdev->cal_client_ctx);
		mon_pdev->enhanced_stats_en = 0;
		return QDF_STATUS_E_FAILURE;
	}

	pdev->enhanced_stats_en = true;
	if (is_ppdu_txrx_capture_enabled(pdev) && !mon_pdev->bpr_enable) {
		dp_h2t_cfg_stats_msg_send(pdev, DP_PPDU_STATS_CFG_ENH_STATS,
					  pdev->pdev_id);
	} else if (is_ppdu_txrx_capture_enabled(pdev) &&
		   mon_pdev->bpr_enable) {
		dp_h2t_cfg_stats_msg_send(pdev,
					  DP_PPDU_STATS_CFG_BPR_ENH,
					  pdev->pdev_id);
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_disable_enhanced_stats()- API to disable enhanced statistcs
 *
 * @param soc - the soc handle
 * @param pdev_id - pdev_id of pdev
 * @return - QDF_STATUS
 */
static QDF_STATUS
dp_disable_enhanced_stats(struct cdp_soc_t *soc, uint8_t pdev_id)
{
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	struct dp_mon_pdev *mon_pdev;


	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	mon_pdev = pdev->monitor_pdev;

	if (mon_pdev->enhanced_stats_en == 1)
		dp_cal_client_timer_stop(mon_pdev->cal_client_ctx);

	mon_pdev->enhanced_stats_en = 0;
	pdev->enhanced_stats_en = false;

	if (is_ppdu_txrx_capture_enabled(pdev) && !mon_pdev->bpr_enable) {
		dp_h2t_cfg_stats_msg_send(pdev, 0, pdev->pdev_id);
	} else if (is_ppdu_txrx_capture_enabled(pdev) && mon_pdev->bpr_enable) {
		dp_h2t_cfg_stats_msg_send(pdev,
					  DP_PPDU_STATS_CFG_BPR,
					  pdev->pdev_id);
	}

	dp_mon_filter_reset_enhanced_stats(pdev);
	if (dp_mon_filter_update(pdev) != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  FL("Failed to reset enhanced mode filters"));
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WDI_EVENT_ENABLE
QDF_STATUS dp_peer_qos_stats_notify(struct dp_pdev *dp_pdev,
				    struct cdp_rx_stats_ppdu_user *ppdu_user)
{
	struct cdp_interface_peer_qos_stats qos_stats_intf;

	if (ppdu_user->peer_id == HTT_INVALID_PEER) {
		dp_mon_warn("Invalid peer id");
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_zero(&qos_stats_intf, sizeof(qos_stats_intf));

	qdf_mem_copy(qos_stats_intf.peer_mac, ppdu_user->mac_addr,
		     QDF_MAC_ADDR_SIZE);
	qos_stats_intf.frame_control = ppdu_user->frame_control;
	qos_stats_intf.frame_control_info_valid =
			ppdu_user->frame_control_info_valid;
	qos_stats_intf.qos_control = ppdu_user->qos_control;
	qos_stats_intf.qos_control_info_valid =
			ppdu_user->qos_control_info_valid;
	qos_stats_intf.vdev_id = ppdu_user->vdev_id;
	dp_wdi_event_handler(WDI_EVENT_PEER_QOS_STATS, dp_pdev->soc,
			     (void *)&qos_stats_intf, 0,
			     WDI_NO_VAL, dp_pdev->pdev_id);

	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
dp_peer_qos_stats_notify(struct dp_pdev *dp_pdev,
			 struct cdp_rx_stats_ppdu_user *ppdu_user)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif /* QCA_ENHANCED_STATS_SUPPORT */

/**
 * dp_enable_peer_based_pktlog() - Set Flag for peer based filtering
 * for pktlog
 * @soc: cdp_soc handle
 * @pdev_id: id of dp pdev handle
 * @mac_addr: Peer mac address
 * @enb_dsb: Enable or disable peer based filtering
 *
 * Return: QDF_STATUS
 */
static int
dp_enable_peer_based_pktlog(struct cdp_soc_t *soc, uint8_t pdev_id,
			    uint8_t *mac_addr, uint8_t enb_dsb)
{
	struct dp_peer *peer;
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	struct dp_mon_pdev *mon_pdev;

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	mon_pdev = pdev->monitor_pdev;

	peer = dp_peer_find_hash_find((struct dp_soc *)soc, mac_addr,
				      0, DP_VDEV_ALL, DP_MOD_ID_CDP);

	if (!peer) {
		dp_mon_err("Invalid Peer");
		return QDF_STATUS_E_FAILURE;
	}

	peer->peer_based_pktlog_filter = enb_dsb;
	mon_pdev->dp_peer_based_pktlog = enb_dsb;

	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_peer_update_pkt_capture_params: Set Rx & Tx Capture flags for a peer
 * @soc: DP_SOC handle
 * @pdev_id: id of DP_PDEV handle
 * @is_rx_pkt_cap_enable: enable/disable Rx packet capture in monitor mode
 * @is_tx_pkt_cap_enable: enable/disable/delete/print
 * Tx packet capture in monitor mode
 * @peer_mac: MAC address for which the above need to be enabled/disabled
 *
 * Return: Success if Rx & Tx capture is enabled for peer, false otherwise
 */
#if defined(WLAN_TX_PKT_CAPTURE_ENH) || defined(WLAN_RX_PKT_CAPTURE_ENH)
static QDF_STATUS
dp_peer_update_pkt_capture_params(ol_txrx_soc_handle soc,
				  uint8_t pdev_id,
				  bool is_rx_pkt_cap_enable,
				  uint8_t is_tx_pkt_cap_enable,
				  uint8_t *peer_mac)
{
	struct dp_peer *peer;
	QDF_STATUS status;
	struct dp_pdev *pdev =
			dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
							   pdev_id);
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	peer = dp_peer_find_hash_find((struct dp_soc *)soc,
				      peer_mac, 0, DP_VDEV_ALL,
				      DP_MOD_ID_CDP);
	if (!peer)
		return QDF_STATUS_E_FAILURE;

	/* we need to set tx pkt capture for non associated peer */
	status = dp_peer_set_tx_capture_enabled(pdev, peer,
						is_tx_pkt_cap_enable,
						peer_mac);

	status = dp_peer_set_rx_capture_enabled(pdev, peer,
						is_rx_pkt_cap_enable,
						peer_mac);
	dp_peer_unref_delete(peer, DP_MOD_ID_CDP);

	return status;
}
#endif

#ifdef QCA_MCOPY_SUPPORT
QDF_STATUS dp_mcopy_check_deliver(struct dp_pdev *pdev,
				  uint16_t peer_id,
				  uint32_t ppdu_id,
				  uint8_t first_msdu)
{
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	if (mon_pdev->mcopy_mode) {
		if (mon_pdev->mcopy_mode == M_COPY) {
			if ((mon_pdev->m_copy_id.tx_ppdu_id == ppdu_id) &&
			    (mon_pdev->m_copy_id.tx_peer_id == peer_id)) {
				return QDF_STATUS_E_INVAL;
			}
		}

		if (!first_msdu)
			return QDF_STATUS_E_INVAL;

		mon_pdev->m_copy_id.tx_ppdu_id = ppdu_id;
		mon_pdev->m_copy_id.tx_peer_id = peer_id;
	}

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WDI_EVENT_ENABLE
#ifndef REMOVE_PKT_LOG
static void *dp_get_pldev(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);

	if (!pdev || !pdev->monitor_pdev)
		return NULL;

	return pdev->monitor_pdev->pl_dev;
}
#else
static void *dp_get_pldev(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	return NULL;
}
#endif
#endif

QDF_STATUS dp_rx_populate_cbf_hdr(struct dp_soc *soc,
				  uint32_t mac_id,
				  uint32_t event,
				  qdf_nbuf_t mpdu,
				  uint32_t msdu_timestamp)
{
	uint32_t data_size, hdr_size, ppdu_id, align4byte;
	struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	uint32_t *msg_word;

	if (!pdev)
		return QDF_STATUS_E_INVAL;

	ppdu_id = pdev->monitor_pdev->ppdu_info.com_info.ppdu_id;

	hdr_size = HTT_T2H_PPDU_STATS_IND_HDR_SIZE
		+ qdf_offsetof(htt_ppdu_stats_rx_mgmtctrl_payload_tlv, payload);

	data_size = qdf_nbuf_len(mpdu);

	qdf_nbuf_push_head(mpdu, hdr_size);

	msg_word = (uint32_t *)qdf_nbuf_data(mpdu);
	/*
	 * Populate the PPDU Stats Indication header
	 */
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_T2H_MSG_TYPE_PPDU_STATS_IND);
	HTT_T2H_PPDU_STATS_MAC_ID_SET(*msg_word, mac_id);
	HTT_T2H_PPDU_STATS_PDEV_ID_SET(*msg_word, pdev->pdev_id);
	align4byte = ((data_size +
		qdf_offsetof(htt_ppdu_stats_rx_mgmtctrl_payload_tlv, payload)
		+ 3) >> 2) << 2;
	HTT_T2H_PPDU_STATS_PAYLOAD_SIZE_SET(*msg_word, align4byte);
	msg_word++;
	HTT_T2H_PPDU_STATS_PPDU_ID_SET(*msg_word, ppdu_id);
	msg_word++;

	*msg_word = msdu_timestamp;
	msg_word++;
	/* Skip reserved field */
	msg_word++;
	/*
	 * Populate MGMT_CTRL Payload TLV first
	 */
	HTT_STATS_TLV_TAG_SET(*msg_word,
			      HTT_PPDU_STATS_RX_MGMTCTRL_PAYLOAD_TLV);

	align4byte = ((data_size - sizeof(htt_tlv_hdr_t) +
		qdf_offsetof(htt_ppdu_stats_rx_mgmtctrl_payload_tlv, payload)
		+ 3) >> 2) << 2;
	HTT_STATS_TLV_LENGTH_SET(*msg_word, align4byte);
	msg_word++;

	HTT_PPDU_STATS_RX_MGMTCTRL_TLV_FRAME_LENGTH_SET(
		*msg_word, data_size);
	msg_word++;

	dp_wdi_event_handler(event, soc, (void *)mpdu,
			     HTT_INVALID_PEER, WDI_NO_VAL, pdev->pdev_id);

	qdf_nbuf_pull_head(mpdu, hdr_size);

	return QDF_STATUS_SUCCESS;
}

#ifdef ATH_SUPPORT_EXT_STAT
/*dp_peer_cal_clients_stats_update - update peer stats on cal client timer
 * @soc : Datapath SOC
 * @peer : Datapath peer
 * @arg : argument to iter function
 */
static void
dp_peer_cal_clients_stats_update(struct dp_soc *soc,
				 struct dp_peer *peer,
				 void *arg)
{
	dp_cal_client_update_peer_stats(&peer->stats);
}

/*dp_iterate_update_peer_list - update peer stats on cal client timer
 * @pdev_hdl: pdev handle
 */
static void dp_iterate_update_peer_list(struct cdp_pdev *pdev_hdl)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_hdl;

	dp_pdev_iterate_peer(pdev, dp_peer_cal_clients_stats_update, NULL,
			     DP_MOD_ID_CDP);
}
#else
static void  dp_iterate_update_peer_list(struct cdp_pdev *pdev_hdl)
{
}
#endif

#ifdef ATH_SUPPORT_NAC
int dp_set_filter_neigh_peers(struct dp_pdev *pdev,
			      bool val)
{
	/* Enable/Disable smart mesh filtering. This flag will be checked
	 * during rx processing to check if packets are from NAC clients.
	 */
	pdev->monitor_pdev->filter_neighbour_peers = val;
	return 0;
}
#endif /* ATH_SUPPORT_NAC */

#ifdef WLAN_ATF_ENABLE
void dp_set_atf_stats_enable(struct dp_pdev *pdev, bool value)
{
	if (!pdev) {
		dp_cdp_err("Invalid pdev");
		return;
	}

	pdev->monitor_pdev->dp_atf_stats_enable = value;
}
#endif

void
dp_mon_set_bsscolor(struct dp_pdev *pdev, uint8_t bsscolor)
{
	pdev->monitor_pdev->rx_mon_recv_status.bsscolor = bsscolor;
}

bool dp_pdev_get_filter_ucast_data(struct cdp_pdev *pdev_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	if ((mon_pdev->fp_data_filter & FILTER_DATA_UCAST) ||
	    (mon_pdev->mo_data_filter & FILTER_DATA_UCAST))
		return true;

	return false;
}

bool dp_pdev_get_filter_mcast_data(struct cdp_pdev *pdev_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	if ((mon_pdev->fp_data_filter & FILTER_DATA_MCAST) ||
	    (mon_pdev->mo_data_filter & FILTER_DATA_MCAST))
		return true;

	return false;
}

bool dp_pdev_get_filter_non_data(struct cdp_pdev *pdev_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	if ((mon_pdev->fp_mgmt_filter & FILTER_MGMT_ALL) ||
	    (mon_pdev->mo_mgmt_filter & FILTER_MGMT_ALL)) {
		if ((mon_pdev->fp_ctrl_filter & FILTER_CTRL_ALL) ||
		    (mon_pdev->mo_ctrl_filter & FILTER_CTRL_ALL)) {
			return true;
		}
	}

	return false;
}

QDF_STATUS dp_mon_soc_cfg_init(struct dp_soc *soc)
{
	int target_type;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
	struct cdp_mon_ops *cdp_ops;

	cdp_ops = dp_mon_cdp_ops_get(soc);
	target_type = hal_get_target_type(soc->hal_soc);
	switch (target_type) {
	case TARGET_TYPE_QCA6290:
	case TARGET_TYPE_QCA6390:
	case TARGET_TYPE_QCA6490:
	case TARGET_TYPE_QCA6750:
	case TARGET_TYPE_KIWI:
		/* do nothing */
		break;
	case TARGET_TYPE_QCA8074:
		wlan_cfg_set_mon_delayed_replenish_entries(soc->wlan_cfg_ctx,
							   MON_BUF_MIN_ENTRIES);
		break;
	case TARGET_TYPE_QCA8074V2:
	case TARGET_TYPE_QCA6018:
	case TARGET_TYPE_QCA9574:
		wlan_cfg_set_mon_delayed_replenish_entries(soc->wlan_cfg_ctx,
							   MON_BUF_MIN_ENTRIES);
		mon_soc->hw_nac_monitor_support = 1;
		break;
	case TARGET_TYPE_QCN9000:
		wlan_cfg_set_mon_delayed_replenish_entries(soc->wlan_cfg_ctx,
							   MON_BUF_MIN_ENTRIES);
		mon_soc->hw_nac_monitor_support = 1;
		if (cfg_get(soc->ctrl_psoc, CFG_DP_FULL_MON_MODE)) {
			if (cdp_ops  && cdp_ops->config_full_mon_mode)
				cdp_ops->config_full_mon_mode((struct cdp_soc_t *)soc, 1);
		}
		break;
	case TARGET_TYPE_QCA5018:
	case TARGET_TYPE_QCN6122:
		wlan_cfg_set_mon_delayed_replenish_entries(soc->wlan_cfg_ctx,
							   MON_BUF_MIN_ENTRIES);
		mon_soc->hw_nac_monitor_support = 1;
		break;
	case TARGET_TYPE_QCN9224:
		wlan_cfg_set_mon_delayed_replenish_entries(soc->wlan_cfg_ctx,
							   MON_BUF_MIN_ENTRIES);
		mon_soc->hw_nac_monitor_support = 1;
		mon_soc->monitor_mode_v2 = 1;
		break;
	default:
		dp_mon_info("%s: Unknown tgt type %d\n", __func__, target_type);
		qdf_assert_always(0);
		break;
	}

	dp_mon_info("hw_nac_monitor_support = %d",
		    mon_soc->hw_nac_monitor_support);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_mon_pdev_per_target_config() - Target specific monitor pdev configuration
 * @pdev: PDEV handle [Should be valid]
 *
 * Return: None
 */
static void dp_mon_pdev_per_target_config(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;
	int target_type;

	target_type = hal_get_target_type(soc->hal_soc);
	switch (target_type) {
	case TARGET_TYPE_KIWI:
		mon_pdev->is_tlv_hdr_64_bit = true;
		break;
	default:
		mon_pdev->is_tlv_hdr_64_bit = false;
		break;
	}
}

QDF_STATUS dp_mon_pdev_attach(struct dp_pdev *pdev)
{
	struct dp_soc *soc;
	struct dp_mon_pdev *mon_pdev;
	struct dp_mon_ops *mon_ops;

	if (!pdev) {
		dp_mon_err("pdev is NULL");
		goto fail0;
	}

	soc = pdev->soc;

	mon_pdev = (struct dp_mon_pdev *)qdf_mem_malloc(sizeof(*mon_pdev));
	if (!mon_pdev) {
		dp_mon_err("%pK: MONITOR pdev allocation failed", pdev);
		goto fail0;
	}

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (!mon_ops) {
		dp_mon_err("%pK: Invalid monitor ops", pdev);
		goto fail1;
	}

	if (mon_ops->mon_pdev_alloc) {
		if (mon_ops->mon_pdev_alloc(pdev)) {
			dp_mon_err("%pK: MONITOR pdev alloc failed", pdev);
			goto fail1;
		}
	}

	if (mon_ops->mon_rings_alloc) {
		if (mon_ops->mon_rings_alloc(pdev)) {
			dp_mon_err("%pK: MONITOR rings setup failed", pdev);
			goto fail2;
		}
	}

	/* Rx monitor mode specific init */
	if (mon_ops->rx_mon_desc_pool_alloc) {
		if (mon_ops->rx_mon_desc_pool_alloc(pdev)) {
			dp_mon_err("%pK: dp_rx_pdev_mon_attach failed", pdev);
			goto fail3;
		}
	}

	pdev->monitor_pdev = mon_pdev;
	dp_mon_pdev_per_target_config(pdev);

	return QDF_STATUS_SUCCESS;
fail3:
	if (mon_ops->mon_rings_free)
		mon_ops->mon_rings_free(pdev);
fail2:
	if (mon_ops->mon_pdev_free)
		mon_ops->mon_pdev_free(pdev);
fail1:
	pdev->monitor_pdev = NULL;
	qdf_mem_free(mon_pdev);
fail0:
	return QDF_STATUS_E_NOMEM;
}

QDF_STATUS dp_mon_pdev_detach(struct dp_pdev *pdev)
{
	struct dp_mon_pdev *mon_pdev;
	struct dp_mon_ops *mon_ops = NULL;

	if (!pdev) {
		dp_mon_err("pdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	mon_pdev = pdev->monitor_pdev;
	mon_ops = dp_mon_ops_get(pdev->soc);
	if (!mon_ops) {
		dp_mon_err("Monitor ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (mon_ops->rx_mon_desc_pool_free)
		mon_ops->rx_mon_desc_pool_free(pdev);
	if (mon_ops->mon_rings_free)
		mon_ops->mon_rings_free(pdev);
	if (mon_ops->mon_pdev_free)
		mon_ops->mon_pdev_free(pdev);

	pdev->monitor_pdev = NULL;
	qdf_mem_free(mon_pdev);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_mon_pdev_init(struct dp_pdev *pdev)
{
	struct dp_soc *soc;
	struct dp_mon_pdev *mon_pdev;
	struct dp_mon_ops *mon_ops = NULL;

	if (!pdev) {
		dp_mon_err("pdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	soc = pdev->soc;
	mon_pdev = pdev->monitor_pdev;

	mon_pdev->filter = dp_mon_filter_alloc(mon_pdev);
	if (!mon_pdev->filter) {
		dp_mon_err("%pK: Memory allocation failed for monitor filter",
			   pdev);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_spinlock_create(&mon_pdev->ppdu_stats_lock);
	qdf_spinlock_create(&mon_pdev->neighbour_peer_mutex);
	mon_pdev->monitor_configured = false;
	mon_pdev->mon_chan_band = REG_BAND_UNKNOWN;

	TAILQ_INIT(&mon_pdev->neighbour_peers_list);
	mon_pdev->neighbour_peers_added = false;
	mon_pdev->monitor_configured = false;
	/* Monitor filter init */
	mon_pdev->mon_filter_mode = MON_FILTER_ALL;
	mon_pdev->fp_mgmt_filter = FILTER_MGMT_ALL;
	mon_pdev->fp_ctrl_filter = FILTER_CTRL_ALL;
	mon_pdev->fp_data_filter = FILTER_DATA_ALL;
	mon_pdev->mo_mgmt_filter = FILTER_MGMT_ALL;
	mon_pdev->mo_ctrl_filter = FILTER_CTRL_ALL;
	mon_pdev->mo_data_filter = FILTER_DATA_ALL;

	/*
	 * initialize ppdu tlv list
	 */
	TAILQ_INIT(&mon_pdev->ppdu_info_list);
	TAILQ_INIT(&mon_pdev->sched_comp_ppdu_list);

	mon_pdev->list_depth = 0;
	mon_pdev->tlv_count = 0;
	/* initlialize cal client timer */
	dp_cal_client_attach(&mon_pdev->cal_client_ctx,
			     dp_pdev_to_cdp_pdev(pdev),
			     pdev->soc->osdev,
			     &dp_iterate_update_peer_list);
	if (dp_htt_ppdu_stats_attach(pdev) != QDF_STATUS_SUCCESS)
		goto fail0;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (!mon_ops) {
		dp_mon_err("Monitor ops is NULL");
		goto fail1;
	}

	if (mon_ops->mon_rings_init) {
		if (mon_ops->mon_rings_init(pdev)) {
			dp_mon_err("%pK: MONITOR rings setup failed", pdev);
			goto fail1;
		}
	}

	/* initialize sw monitor rx descriptors */
	if (mon_ops->rx_mon_desc_pool_init)
		mon_ops->rx_mon_desc_pool_init(pdev);

	/* allocate buffers and replenish the monitor RxDMA ring */
	if (mon_ops->rx_mon_buffers_alloc) {
		if (mon_ops->rx_mon_buffers_alloc(pdev)) {
			dp_mon_err("%pK: rx mon buffers alloc failed", pdev);
			goto fail2;
		}
	}

	dp_tx_ppdu_stats_attach(pdev);
	mon_pdev->is_dp_mon_pdev_initialized = true;

	return QDF_STATUS_SUCCESS;

fail2:
	if (mon_ops->rx_mon_desc_pool_deinit)
		mon_ops->rx_mon_desc_pool_deinit(pdev);

	if (mon_ops->mon_rings_deinit)
		mon_ops->mon_rings_deinit(pdev);

fail1:
	dp_htt_ppdu_stats_detach(pdev);
fail0:
	qdf_spinlock_destroy(&mon_pdev->neighbour_peer_mutex);
	qdf_spinlock_destroy(&mon_pdev->ppdu_stats_lock);
	dp_mon_filter_dealloc(mon_pdev);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS dp_mon_pdev_deinit(struct dp_pdev *pdev)
{
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (!mon_ops) {
		dp_mon_err("Monitor ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!mon_pdev->is_dp_mon_pdev_initialized)
		return QDF_STATUS_SUCCESS;

	dp_tx_ppdu_stats_detach(pdev);

	if (mon_ops->rx_mon_buffers_free)
		mon_ops->rx_mon_buffers_free(pdev);
	if (mon_ops->rx_mon_desc_pool_deinit)
		mon_ops->rx_mon_desc_pool_deinit(pdev);
	if (mon_ops->mon_rings_deinit)
		mon_ops->mon_rings_deinit(pdev);
	dp_cal_client_detach(&mon_pdev->cal_client_ctx);
	dp_htt_ppdu_stats_detach(pdev);
	qdf_spinlock_destroy(&mon_pdev->ppdu_stats_lock);
	dp_neighbour_peers_detach(pdev);
	dp_pktlogmod_exit(pdev);
	if (mon_pdev->filter)
		dp_mon_filter_dealloc(mon_pdev);
	if (mon_ops->mon_rings_deinit)
		mon_ops->mon_rings_deinit(pdev);
	mon_pdev->is_dp_mon_pdev_initialized = false;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_mon_vdev_attach(struct dp_vdev *vdev)
{
	struct dp_mon_vdev *mon_vdev;
	struct dp_pdev *pdev = vdev->pdev;

	mon_vdev = (struct dp_mon_vdev *)qdf_mem_malloc(sizeof(*mon_vdev));
	if (!mon_vdev) {
		dp_mon_err("%pK: Monitor vdev allocation failed", vdev);
		return QDF_STATUS_E_NOMEM;
	}

	if (pdev->monitor_pdev->scan_spcl_vap_configured)
		dp_scan_spcl_vap_stats_attach(mon_vdev);

	vdev->monitor_vdev = mon_vdev;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_mon_vdev_detach(struct dp_vdev *vdev)
{
	struct dp_mon_vdev *mon_vdev = vdev->monitor_vdev;
	struct dp_pdev *pdev = vdev->pdev;

	if (!mon_vdev)
		return QDF_STATUS_E_FAILURE;

	if (pdev->monitor_pdev->scan_spcl_vap_configured)
		dp_scan_spcl_vap_stats_detach(mon_vdev);

	qdf_mem_free(mon_vdev);
	vdev->monitor_vdev = NULL;
	/* set mvdev to NULL only if detach is called for monitor/special vap
	 */
	if (pdev->monitor_pdev->mvdev == vdev)
		pdev->monitor_pdev->mvdev = NULL;

	return QDF_STATUS_SUCCESS;
}

#if defined(WLAN_TX_PKT_CAPTURE_ENH) || defined(FEATURE_PERPKT_INFO)
QDF_STATUS dp_mon_peer_attach(struct dp_peer *peer)
{
	struct dp_mon_peer *mon_peer;
	struct dp_pdev *pdev;

	mon_peer = (struct dp_mon_peer *)qdf_mem_malloc(sizeof(*mon_peer));
	if (!mon_peer) {
		dp_mon_err("%pK: MONITOR peer allocation failed", peer);
		return QDF_STATUS_E_NOMEM;
	}

	peer->monitor_peer = mon_peer;
	pdev = peer->vdev->pdev;
	/*
	 * In tx_monitor mode, filter may be set for unassociated peer
	 * when unassociated peer get associated peer need to
	 * update tx_cap_enabled flag to support peer filter.
	 */
	dp_peer_tx_capture_filter_check(pdev, peer);

	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS dp_mon_peer_detach(struct dp_peer *peer)
{
	struct dp_mon_peer *mon_peer = peer->monitor_peer;

	qdf_mem_free(mon_peer);
	peer->monitor_peer = NULL;

	return QDF_STATUS_SUCCESS;
}

#ifndef DISABLE_MON_CONFIG
void dp_mon_register_intr_ops(struct dp_soc *soc)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(soc);
	if (!mon_ops) {
		dp_mon_err("Monitor ops is NULL");
		return;
	}
	if (mon_ops->mon_register_intr_ops)
		mon_ops->mon_register_intr_ops(soc);
}
#endif

void dp_mon_ops_register(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
	uint32_t target_type;

	target_type = hal_get_target_type(soc->hal_soc);
	switch (target_type) {
	case TARGET_TYPE_QCA6290:
	case TARGET_TYPE_QCA6390:
	case TARGET_TYPE_QCA6490:
	case TARGET_TYPE_QCA6750:
	case TARGET_TYPE_KIWI:
	case TARGET_TYPE_QCA8074:
	case TARGET_TYPE_QCA8074V2:
	case TARGET_TYPE_QCA6018:
	case TARGET_TYPE_QCA9574:
	case TARGET_TYPE_QCN9000:
	case TARGET_TYPE_QCA5018:
	case TARGET_TYPE_QCN6122:
		mon_soc->mon_ops = dp_mon_ops_get_1_0();
		break;
	case TARGET_TYPE_QCN9224:
#ifdef QCA_MONITOR_2_0_SUPPORT
		mon_soc->mon_ops = dp_mon_ops_get_2_0();
#endif
		break;
	default:
		dp_mon_err("%s: Unknown tgt type %d", __func__, target_type);
		qdf_assert_always(0);
		break;
	}
}

void dp_mon_cdp_ops_register(struct dp_soc *soc)
{
	struct cdp_ops *ops = soc->cdp_soc.ops;
	uint32_t target_type;

	if (!ops) {
		dp_mon_err("cdp_ops is NULL");
		return;
	}

	target_type = hal_get_target_type(soc->hal_soc);
	switch (target_type) {
	case TARGET_TYPE_QCA6290:
	case TARGET_TYPE_QCA6390:
	case TARGET_TYPE_QCA6490:
	case TARGET_TYPE_QCA6750:
	case TARGET_TYPE_KIWI:
	case TARGET_TYPE_QCA8074:
	case TARGET_TYPE_QCA8074V2:
	case TARGET_TYPE_QCA6018:
	case TARGET_TYPE_QCA9574:
	case TARGET_TYPE_QCN9000:
	case TARGET_TYPE_QCA5018:
	case TARGET_TYPE_QCN6122:
		ops->mon_ops = dp_mon_cdp_ops_get_1_0();
		break;
	case TARGET_TYPE_QCN9224:
#ifdef QCA_MONITOR_2_0_SUPPORT
		ops->mon_ops = dp_mon_cdp_ops_get_2_0();
#endif
		break;
	default:
		dp_mon_err("%s: Unknown tgt type %d", __func__, target_type);
		qdf_assert_always(0);
		break;
	}

#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
	ops->cfr_ops->txrx_cfr_filter = dp_cfr_filter;
	ops->cfr_ops->txrx_enable_mon_reap_timer = dp_enable_mon_reap_timer;
#endif
	ops->cmn_drv_ops->txrx_set_monitor_mode = dp_vdev_set_monitor_mode;
	ops->cmn_drv_ops->txrx_get_mon_vdev_from_pdev =
				dp_get_mon_vdev_from_pdev_wifi3;
#ifdef DP_PEER_EXTENDED_API
	ops->misc_ops->pkt_log_init = dp_pkt_log_init;
	ops->misc_ops->pkt_log_con_service = dp_pkt_log_con_service;
	ops->misc_ops->pkt_log_exit = dp_pkt_log_exit;
#endif
#ifdef ATH_SUPPORT_NAC_RSSI
	ops->ctrl_ops->txrx_vdev_config_for_nac_rssi = dp_config_for_nac_rssi;
	ops->ctrl_ops->txrx_vdev_get_neighbour_rssi =
					dp_vdev_get_neighbour_rssi;
#endif
#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
	ops->ctrl_ops->txrx_update_filter_neighbour_peers =
		dp_update_filter_neighbour_peers;
#endif /* ATH_SUPPORT_NAC_RSSI || ATH_SUPPORT_NAC */
	ops->ctrl_ops->enable_peer_based_pktlog =
				dp_enable_peer_based_pktlog;
#if defined(WLAN_TX_PKT_CAPTURE_ENH) || defined(WLAN_RX_PKT_CAPTURE_ENH)
	ops->ctrl_ops->txrx_update_peer_pkt_capture_params =
				 dp_peer_update_pkt_capture_params;
#endif /* WLAN_TX_PKT_CAPTURE_ENH || WLAN_RX_PKT_CAPTURE_ENH */
#ifdef QCA_ENHANCED_STATS_SUPPORT
	ops->host_stats_ops->txrx_enable_enhanced_stats =
					dp_enable_enhanced_stats;
	ops->host_stats_ops->txrx_disable_enhanced_stats =
					dp_disable_enhanced_stats;
#endif /* QCA_ENHANCED_STATS_SUPPORT */
#ifdef WDI_EVENT_ENABLE
	ops->ctrl_ops->txrx_get_pldev = dp_get_pldev;
#endif
#ifdef QCA_SUPPORT_SCAN_SPCL_VAP_STATS
	ops->host_stats_ops->txrx_get_scan_spcl_vap_stats =
					dp_get_scan_spcl_vap_stats;
#endif
	return;
}

void dp_mon_cdp_ops_deregister(struct dp_soc *soc)
{
	struct cdp_ops *ops = soc->cdp_soc.ops;

	if (!ops) {
		dp_mon_err("cdp_ops is NULL");
		return;
	}

	ops->mon_ops = NULL;
#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
	ops->cfr_ops->txrx_cfr_filter = NULL;
	ops->cfr_ops->txrx_enable_mon_reap_timer = NULL;
#endif
	ops->cmn_drv_ops->txrx_set_monitor_mode = NULL;
	ops->cmn_drv_ops->txrx_get_mon_vdev_from_pdev = NULL;
#ifdef DP_PEER_EXTENDED_API
	ops->misc_ops->pkt_log_init = NULL;
	ops->misc_ops->pkt_log_con_service = NULL;
	ops->misc_ops->pkt_log_exit = NULL;
#endif
#ifdef ATH_SUPPORT_NAC_RSSI
	ops->ctrl_ops->txrx_vdev_config_for_nac_rssi = NULL;
	ops->ctrl_ops->txrx_vdev_get_neighbour_rssi = NULL;
#endif
#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
	ops->ctrl_ops->txrx_update_filter_neighbour_peers = NULL;
#endif /* ATH_SUPPORT_NAC_RSSI || ATH_SUPPORT_NAC */
	ops->ctrl_ops->enable_peer_based_pktlog = NULL;
#if defined(WLAN_TX_PKT_CAPTURE_ENH) || defined(WLAN_RX_PKT_CAPTURE_ENH)
	ops->ctrl_ops->txrx_update_peer_pkt_capture_params = NULL;
#endif /* WLAN_TX_PKT_CAPTURE_ENH || WLAN_RX_PKT_CAPTURE_ENH */
#ifdef FEATURE_PERPKT_INFO
	ops->host_stats_ops->txrx_enable_enhanced_stats = NULL;
	ops->host_stats_ops->txrx_disable_enhanced_stats = NULL;
#endif /* FEATURE_PERPKT_INFO */
#ifdef WDI_EVENT_ENABLE
	ops->ctrl_ops->txrx_get_pldev = NULL;
#endif
	return;
}

QDF_STATUS dp_mon_soc_attach(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc;

	if (!soc) {
		dp_mon_err("dp_soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mon_soc = (struct dp_mon_soc *)qdf_mem_malloc(sizeof(*mon_soc));
	if (!mon_soc) {
		dp_mon_err("%pK: mem allocation failed", soc);
		return QDF_STATUS_E_NOMEM;
	}
	/* register monitor ops */
	soc->monitor_soc = mon_soc;
	dp_mon_ops_register(soc);
	dp_mon_register_intr_ops(soc);

	dp_mon_cdp_ops_register(soc);
	dp_mon_register_feature_ops(soc);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_mon_soc_detach(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc;

	if (!soc) {
		dp_mon_err("dp_soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mon_soc = soc->monitor_soc;
	dp_monitor_vdev_timer_deinit(soc);
	dp_mon_cdp_ops_deregister(soc);
	soc->monitor_soc = NULL;
	qdf_mem_free(mon_soc);
	return QDF_STATUS_SUCCESS;
}
