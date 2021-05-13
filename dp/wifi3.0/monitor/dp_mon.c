/*
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

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
#include <dp_rx_mon.h>
#include <dp_mon_filter.h>
#include <dp_mon.h>
#include "htt_ppdu_stats.h"

#define RNG_ERR		"SRNG setup failed for"
#define mon_init_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_INIT, params)

QDF_STATUS dp_srng_alloc(struct dp_soc *soc, struct dp_srng *srng,
			 int ring_type, uint32_t num_entries,
			 bool cached);
void dp_srng_free(struct dp_soc *soc, struct dp_srng *srng);
QDF_STATUS dp_srng_init(struct dp_soc *soc, struct dp_srng *srng,
			int ring_type, int ring_num, int mac_id);
void dp_srng_deinit(struct dp_soc *soc, struct dp_srng *srng,
		    int ring_type, int ring_num);

QDF_STATUS dp_htt_ppdu_stats_attach(struct dp_pdev *pdev);
void dp_htt_ppdu_stats_detach(struct dp_pdev *pdev);
void dp_neighbour_peers_detach(struct dp_pdev *pdev);
void dp_pktlogmod_exit(struct dp_pdev *handle);
QDF_STATUS dp_vdev_set_monitor_mode_rings(struct dp_pdev *pdev,
					  uint8_t delayed_replenish);

#if !defined(DISABLE_MON_CONFIG)
/**
 * dp_mon_rings_deinit() - Deinitialize monitor rings
 * @pdev: DP pdev handle
 *
 * Return: None
 *
 */
static void dp_mon_rings_deinit(struct dp_pdev *pdev)
{
	int mac_id = 0;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;
	struct dp_soc *soc = pdev->soc;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id,
							 pdev->pdev_id);

		dp_srng_deinit(soc, &soc->rxdma_mon_status_ring[lmac_id],
			       RXDMA_MONITOR_STATUS, 0);

		if (!soc->wlan_cfg_ctx->rxdma1_enable)
			continue;

		dp_srng_deinit(soc, &soc->rxdma_mon_buf_ring[lmac_id],
			       RXDMA_MONITOR_BUF, 0);
		dp_srng_deinit(soc, &soc->rxdma_mon_dst_ring[lmac_id],
			       RXDMA_MONITOR_DST, 0);
		dp_srng_deinit(soc, &soc->rxdma_mon_desc_ring[lmac_id],
			       RXDMA_MONITOR_DESC, 0);
	}
}

/**
 * dp_mon_rings_free() - free monitor rings
 * @pdev: Datapath pdev handle
 *
 * Return: None
 *
 */
static void dp_mon_rings_free(struct dp_pdev *pdev)
{
	int mac_id = 0;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;
	struct dp_soc *soc = pdev->soc;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id,
							 pdev->pdev_id);

		dp_srng_free(soc, &soc->rxdma_mon_status_ring[lmac_id]);

		if (!soc->wlan_cfg_ctx->rxdma1_enable)
			continue;

		dp_srng_free(soc, &soc->rxdma_mon_buf_ring[lmac_id]);
		dp_srng_free(soc, &soc->rxdma_mon_dst_ring[lmac_id]);
		dp_srng_free(soc, &soc->rxdma_mon_desc_ring[lmac_id]);
	}
}

/**
 * dp_mon_rings_init() - Initialize monitor srng rings
 * @pdev: Datapath pdev handle
 *
 * return: QDF_STATUS_SUCCESS on success
 *	   QDF_STATUS_E_NOMEM on failure
 */
static
QDF_STATUS dp_mon_rings_init(struct dp_soc *soc, struct dp_pdev *pdev)
{
	int mac_id = 0;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id,
							 pdev->pdev_id);

		if (dp_srng_init(soc, &soc->rxdma_mon_status_ring[lmac_id],
				 RXDMA_MONITOR_STATUS, 0, lmac_id)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_status_ring",
				     soc);
			goto fail1;
		}

		if (!soc->wlan_cfg_ctx->rxdma1_enable)
			continue;

		if (dp_srng_init(soc, &soc->rxdma_mon_buf_ring[lmac_id],
				 RXDMA_MONITOR_BUF, 0, lmac_id)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_buf_ring ",
				     soc);
			goto fail1;
		}

		if (dp_srng_init(soc, &soc->rxdma_mon_dst_ring[lmac_id],
				 RXDMA_MONITOR_DST, 0, lmac_id)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_dst_ring", soc);
			goto fail1;
		}

		if (dp_srng_init(soc, &soc->rxdma_mon_desc_ring[lmac_id],
				 RXDMA_MONITOR_DESC, 0, lmac_id)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_desc_ring",
				     soc);
			goto fail1;
		}
	}
	return QDF_STATUS_SUCCESS;

fail1:
	dp_mon_rings_deinit(pdev);
	return QDF_STATUS_E_NOMEM;
}

/**
 * dp_mon_rings_alloc() - Allocate memory for monitor srng rings
 * @soc: Datapath soc handle
 * @pdev: Datapath pdev handle
 *
 * return: QDF_STATUS_SUCCESS on success
 *	   QDF_STATUS_E_NOMEM on failure
 */
static
QDF_STATUS dp_mon_rings_alloc(struct dp_soc *soc, struct dp_pdev *pdev)
{
	int mac_id = 0;
	int entries;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0; mac_id < NUM_RXDMA_RINGS_PER_PDEV; mac_id++) {
		int lmac_id =
		dp_get_lmac_id_for_pdev_id(soc, mac_id, pdev->pdev_id);
		entries = wlan_cfg_get_dma_mon_stat_ring_size(pdev_cfg_ctx);
		if (dp_srng_alloc(soc, &soc->rxdma_mon_status_ring[lmac_id],
				  RXDMA_MONITOR_STATUS, entries, 0)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_status_ring",
				     soc);
			goto fail1;
		}

		if (!soc->wlan_cfg_ctx->rxdma1_enable)
			continue;

		entries = wlan_cfg_get_dma_mon_buf_ring_size(pdev_cfg_ctx);
		if (dp_srng_alloc(soc, &soc->rxdma_mon_buf_ring[lmac_id],
				  RXDMA_MONITOR_BUF, entries, 0)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_buf_ring ",
				     soc);
			goto fail1;
		}

		entries = wlan_cfg_get_dma_mon_dest_ring_size(pdev_cfg_ctx);
		if (dp_srng_alloc(soc, &soc->rxdma_mon_dst_ring[lmac_id],
				  RXDMA_MONITOR_DST, entries, 0)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_dst_ring", soc);
			goto fail1;
		}

		entries = wlan_cfg_get_dma_mon_desc_ring_size(pdev_cfg_ctx);
		if (dp_srng_alloc(soc, &soc->rxdma_mon_desc_ring[lmac_id],
				  RXDMA_MONITOR_DESC, entries, 0)) {
			mon_init_err("%pK: " RNG_ERR "rxdma_mon_desc_ring",
				     soc);
			goto fail1;
		}
	}
	return QDF_STATUS_SUCCESS;

fail1:
	dp_mon_rings_free(pdev);
	return QDF_STATUS_E_NOMEM;
}
#else
static void dp_mon_rings_free(struct dp_pdev *pdev)
{
}

static void dp_mon_rings_deinit(struct dp_pdev *pdev)
{
}

static
QDF_STATUS dp_mon_rings_init(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS dp_mon_rings_alloc(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#ifdef QCA_SUPPORT_FULL_MON
static inline QDF_STATUS
dp_config_full_mon_mode(struct cdp_soc_t *soc_handle,
			uint8_t val)
{
	struct dp_soc *soc = (struct dp_soc *)soc_handle;

	soc->full_mon_mode = val;
	dp_cdp_err("Configure full monitor mode val: %d ", val);

	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
dp_config_full_mon_mode(struct cdp_soc_t *soc_handle,
			uint8_t val)
{
	return 0;
}
#endif

static inline void
dp_pdev_disable_mcopy_code(struct dp_pdev *pdev)
{
	pdev->mcopy_mode = M_COPY_DISABLED;
	pdev->monitor_configured = false;
	pdev->monitor_vdev = NULL;
}

#ifdef QCA_SUPPORT_FULL_MON
static inline QDF_STATUS
dp_soc_config_full_mon_mode(struct dp_pdev *pdev, enum dp_full_mon_config val)
{
	struct dp_soc *soc = pdev->soc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!soc->full_mon_mode)
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
dp_soc_config_full_mon_mode(struct dp_pdev *pdev, enum dp_full_mon_config val)
{
	return 0;
}
#endif

/**
 * dp_reset_monitor_mode() - Disable monitor mode
 * @soc_hdl: Datapath soc handle
 * @pdev_id: id of datapath PDEV handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_reset_monitor_mode(struct cdp_soc_t *soc_hdl,
				 uint8_t pdev_id,
				 uint8_t special_monitor)
{
	struct dp_soc *soc = (struct dp_soc *)soc_hdl;
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3((struct dp_soc *)soc,
						   pdev_id);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	qdf_spin_lock_bh(&pdev->mon_lock);

	dp_soc_config_full_mon_mode(pdev, DP_FULL_MON_DISABLE);
	pdev->monitor_vdev = NULL;
	pdev->monitor_configured = false;

	/*
	 * Lite monitor mode, smart monitor mode and monitor
	 * mode uses this APIs to filter reset and mode disable
	 */
	if (pdev->mcopy_mode) {
#if defined(FEATURE_PERPKT_INFO)
		dp_pdev_disable_mcopy_code(pdev);
		dp_mon_filter_reset_mcopy_mode(pdev);
#endif /* FEATURE_PERPKT_INFO */
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

	qdf_spin_unlock_bh(&pdev->mon_lock);
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_pdev_set_advance_monitor_filter() - Set DP PDEV monitor filter
 * @soc: soc handle
 * @pdev_id: id of Datapath PDEV handle
 * @filter_val: Flag to select Filter for monitor mode
 * Return: 0 on success, not 0 on failure
 */
static QDF_STATUS
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

	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	vdev = pdev->monitor_vdev;

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
		  "pdev=%pK, pdev_id=%d, soc=%pK vdev=%pK",
		  pdev, pdev_id, soc, vdev);

	/*Check if current pdev's monitor_vdev exists */
	if (!pdev->monitor_vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "vdev=%pK", vdev);
		qdf_assert(vdev);
	}

	/* update filter mode, type in pdev structure */
	pdev->mon_filter_mode = filter_val->mode;
	pdev->fp_mgmt_filter = filter_val->fp_mgmt;
	pdev->fp_ctrl_filter = filter_val->fp_ctrl;
	pdev->fp_data_filter = filter_val->fp_data;
	pdev->mo_mgmt_filter = filter_val->mo_mgmt;
	pdev->mo_ctrl_filter = filter_val->mo_ctrl;
	pdev->mo_data_filter = filter_val->mo_data;

	dp_mon_filter_setup_mon_mode(pdev);
	status = dp_mon_filter_update(pdev);
	if (status != QDF_STATUS_SUCCESS) {
		dp_rx_mon_dest_err("%pK: Failed to set filter for adv mon mode",
				   soc);
		dp_mon_filter_reset_mon_mode(pdev);
	}

	return status;
}

/**
 * dp_deliver_tx_mgmt() - Deliver mgmt frame for tx capture
 * @cdp_soc : data path soc handle
 * @pdev_id : pdev_id
 * @nbuf: Management frame buffer
 */
static QDF_STATUS
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

	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	pdev = vdev->pdev;
	pdev->monitor_vdev = vdev;
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

	/*Check if current pdev's monitor_vdev exists */
	if (pdev->monitor_configured) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "monitor vap already created vdev=%pK\n", vdev);
		status = QDF_STATUS_E_RESOURCES;
		goto fail;
	}

	pdev->monitor_configured = true;

	dp_soc_config_full_mon_mode(pdev, DP_FULL_MON_ENABLE);
	dp_mon_filter_setup_mon_mode(pdev);
	status = dp_mon_filter_update(pdev);
	if (status != QDF_STATUS_SUCCESS) {
		dp_cdp_err("%pK: Failed to reset monitor filters", soc);
		dp_mon_filter_reset_mon_mode(pdev);
		pdev->monitor_configured = false;
		pdev->monitor_vdev = NULL;
	}

fail:
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
	return status;
}

/*
 * dp_config_debug_sniffer()- API to enable/disable debug sniffer
 * @pdev: DP_PDEV handle
 * @val: user provided value
 *
 * Return: 0 for success. nonzero for failure.
 */
static QDF_STATUS
dp_config_debug_sniffer(struct dp_pdev *pdev, int val)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/*
	 * Note: The mirror copy mode cannot co-exist with any other
	 * monitor modes. Hence disabling the filter for this mode will
	 * reset the monitor destination ring filters.
	 */
	if (pdev->mcopy_mode) {
#ifdef FEATURE_PERPKT_INFO
		dp_soc_config_full_mon_mode(pdev, DP_FULL_MON_DISABLE);
		dp_pdev_disable_mcopy_code(pdev);
		dp_mon_filter_reset_mcopy_mode(pdev);
		status = dp_mon_filter_update(pdev);
		if (status != QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  FL("Failed to reset AM copy mode filters"));
		}
#endif /* FEATURE_PERPKT_INFO */
	}
	switch (val) {
	case 0:
		pdev->tx_sniffer_enable = 0;
		pdev->monitor_configured = false;

		/*
		 * We don't need to reset the Rx monitor status ring  or call
		 * the API dp_ppdu_ring_reset() if all debug sniffer mode is
		 * disabled. The Rx monitor status ring will be disabled when
		 * the last mode using the monitor status ring get disabled.
		 */
		if (!pdev->pktlog_ppdu_stats && !pdev->enhanced_stats_en &&
		    !pdev->bpr_enable) {
			dp_h2t_cfg_stats_msg_send(pdev, 0, pdev->pdev_id);
		} else if (pdev->enhanced_stats_en && !pdev->bpr_enable) {
			dp_h2t_cfg_stats_msg_send(pdev,
						  DP_PPDU_STATS_CFG_ENH_STATS,
						  pdev->pdev_id);
		} else if (!pdev->enhanced_stats_en && pdev->bpr_enable) {
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
		pdev->tx_sniffer_enable = 1;
		pdev->monitor_configured = false;

		if (!pdev->pktlog_ppdu_stats)
			dp_h2t_cfg_stats_msg_send(pdev,
						  DP_PPDU_STATS_CFG_SNIFFER,
						  pdev->pdev_id);
		break;
	case 2:
	case 4:
		if (pdev->monitor_vdev) {
			status = QDF_STATUS_E_RESOURCES;
			break;
		}

#ifdef FEATURE_PERPKT_INFO
		pdev->mcopy_mode = val;
		pdev->tx_sniffer_enable = 0;
		pdev->monitor_configured = true;

		if (!wlan_cfg_is_delay_mon_replenish(pdev->soc->wlan_cfg_ctx))
			dp_vdev_set_monitor_mode_rings(pdev, true);

		/*
		 * Setup the M copy mode filter.
		 */
		dp_soc_config_full_mon_mode(pdev, DP_FULL_MON_ENABLE);
		dp_mon_filter_setup_mcopy_mode(pdev);
		status = dp_mon_filter_update(pdev);
		if (status != QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  FL("Failed to set M_copy mode filters"));
			dp_mon_filter_reset_mcopy_mode(pdev);
			dp_pdev_disable_mcopy_code(pdev);
			return status;
		}

		if (!pdev->pktlog_ppdu_stats)
			dp_h2t_cfg_stats_msg_send(pdev,
						  DP_PPDU_STATS_CFG_SNIFFER,
						  pdev->pdev_id);
#endif /* FEATURE_PERPKT_INFO */
		break;

	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "Invalid value");
		break;
	}
	return status;
}

QDF_STATUS dp_mon_soc_cfg_init(struct dp_soc *soc)
{
	int target_type;

	target_type = hal_get_target_type(soc->hal_soc);
	switch (target_type) {
	case TARGET_TYPE_QCA6290:
	case TARGET_TYPE_QCA6390:
	case TARGET_TYPE_QCA6490:
	case TARGET_TYPE_QCA6750:
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
		soc->hw_nac_monitor_support = 1;
		break;
	case TARGET_TYPE_QCN9000:
		wlan_cfg_set_mon_delayed_replenish_entries(soc->wlan_cfg_ctx,
							   MON_BUF_MIN_ENTRIES);
		soc->hw_nac_monitor_support = 1;
		if (cfg_get(soc->ctrl_psoc, CFG_DP_FULL_MON_MODE))
			dp_config_full_mon_mode((struct cdp_soc_t *)soc, 1);
		break;
	case TARGET_TYPE_QCA5018:
	case TARGET_TYPE_QCN6122:
		wlan_cfg_set_mon_delayed_replenish_entries(soc->wlan_cfg_ctx,
							   MON_BUF_MIN_ENTRIES);
		soc->hw_nac_monitor_support = 1;
		break;
	case TARGET_TYPE_QCN9224:
		wlan_cfg_set_mon_delayed_replenish_entries(soc->wlan_cfg_ctx,
							   MON_BUF_MIN_ENTRIES);
		soc->hw_nac_monitor_support = 1;
		break;
	default:
		qdf_print("%s: Unknown tgt type %d\n", __func__, target_type);
		qdf_assert_always(0);
		break;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_mon_pdev_attach(struct dp_pdev *pdev)
{
	struct dp_soc *soc;
	struct dp_mon_pdev *mon_pdev;

	if (!pdev) {
		mon_init_err("pdev is NULL");
		goto fail0;
	}

	soc = pdev->soc;

	mon_pdev = (struct dp_mon_pdev *)qdf_mem_malloc(sizeof(*mon_pdev));
	if (!mon_pdev) {
		mon_init_err("%pK: MONITOR pdev allocation failed", pdev);
		goto fail0;
	}

	if (dp_mon_rings_alloc(soc, pdev)) {
		mon_init_err("%pK: MONITOR rings setup failed", pdev);
		goto fail1;
	}

	/* Rx monitor mode specific init */
	if (dp_rx_pdev_mon_desc_pool_alloc(pdev)) {
		mon_init_err("%pK: dp_rx_pdev_mon_attach failed", pdev);
		goto fail2;
	}

	pdev->monitor_pdev = mon_pdev;

	return QDF_STATUS_SUCCESS;
fail2:
	dp_mon_rings_free(pdev);
fail1:
	pdev->monitor_pdev = NULL;
	qdf_mem_free(mon_pdev);
fail0:
	return QDF_STATUS_E_NOMEM;
}

QDF_STATUS dp_mon_pdev_detach(struct dp_pdev *pdev)
{
	struct dp_mon_pdev *mon_pdev;

	if (!pdev) {
		mon_init_err("pdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mon_pdev = pdev->monitor_pdev;
	dp_rx_pdev_mon_desc_pool_free(pdev);
	dp_mon_rings_free(pdev);
	pdev->monitor_pdev = NULL;
	qdf_mem_free(mon_pdev);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_mon_pdev_init(struct dp_pdev *pdev)
{
	struct dp_soc *soc;

	if (!pdev) {
		mon_init_err("pdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	soc = pdev->soc;

	pdev->filter = dp_mon_filter_alloc(pdev);
	if (!pdev->filter) {
		mon_init_err("%pK: Memory allocation failed for monitor filter",
			     pdev);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_spinlock_create(&pdev->ppdu_stats_lock);
	qdf_spinlock_create(&pdev->neighbour_peer_mutex);
	pdev->monitor_configured = false;
	pdev->mon_chan_band = REG_BAND_UNKNOWN;
	/* Monitor filter init */
	pdev->mon_filter_mode = MON_FILTER_ALL;

	TAILQ_INIT(&pdev->neighbour_peers_list);
	pdev->neighbour_peers_added = false;
	pdev->monitor_configured = false;
	/* Monitor filter init */
	pdev->mon_filter_mode = MON_FILTER_ALL;
	pdev->fp_mgmt_filter = FILTER_MGMT_ALL;
	pdev->fp_ctrl_filter = FILTER_CTRL_ALL;
	pdev->fp_data_filter = FILTER_DATA_ALL;
	pdev->mo_mgmt_filter = FILTER_MGMT_ALL;
	pdev->mo_ctrl_filter = FILTER_CTRL_ALL;
	pdev->mo_data_filter = FILTER_DATA_ALL;

	if (dp_htt_ppdu_stats_attach(pdev) != QDF_STATUS_SUCCESS)
		goto fail0;

	if (dp_mon_rings_init(soc, pdev)) {
		mon_init_err("%pK: MONITOR rings setup failed", pdev);
		goto fail1;
	}
	/* initialize sw monitor rx descriptors */
	dp_rx_pdev_mon_desc_pool_init(pdev);
	/* allocate buffers and replenish the monitor RxDMA ring */
	dp_rx_pdev_mon_buffers_alloc(pdev);
	dp_tx_ppdu_stats_attach(pdev);

	return QDF_STATUS_SUCCESS;
fail1:
	dp_htt_ppdu_stats_detach(pdev);
fail0:
	qdf_spinlock_destroy(&pdev->neighbour_peer_mutex);
	qdf_spinlock_destroy(&pdev->ppdu_stats_lock);
	dp_mon_filter_dealloc(pdev);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS dp_mon_pdev_deinit(struct dp_pdev *pdev)
{
	dp_tx_ppdu_stats_detach(pdev);
	dp_rx_pdev_mon_buffers_free(pdev);
	dp_rx_pdev_mon_desc_pool_deinit(pdev);
	dp_mon_rings_deinit(pdev);
	dp_htt_ppdu_stats_detach(pdev);
	qdf_spinlock_destroy(&pdev->ppdu_stats_lock);
	dp_neighbour_peers_detach(pdev);
	dp_pktlogmod_exit(pdev);

	if (pdev->filter)
		dp_mon_filter_dealloc(pdev);

	return QDF_STATUS_SUCCESS;
}

static struct dp_mon_ops monitor_ops = {
	.mon_soc_cfg_init = dp_mon_soc_cfg_init,
	.mon_pdev_attach = dp_mon_pdev_attach,
	.mon_pdev_detach = dp_mon_pdev_detach,
	.mon_pdev_init = dp_mon_pdev_init,
	.mon_pdev_deinit = dp_mon_pdev_deinit,
	.mon_config_debug_sniffer = dp_config_debug_sniffer,
};

static struct cdp_mon_ops dp_ops_mon = {
	.txrx_reset_monitor_mode = dp_reset_monitor_mode,
	/* Added support for HK advance filter */
	.txrx_set_advance_monitor_filter = dp_pdev_set_advance_monitor_filter,
	.txrx_deliver_tx_mgmt = dp_deliver_tx_mgmt,
	.config_full_mon_mode = dp_config_full_mon_mode,
};

static inline void dp_mon_ops_register(struct dp_mon_soc *mon_soc)
{
	mon_soc->mon_ops = &monitor_ops;
}

static inline void dp_mon_cdp_ops_register(struct dp_soc *soc)
{
	struct cdp_ops *ops = soc->cdp_soc.ops;

	if (!ops) {
		mon_init_err("cdp_ops is NULL");
		return;
	}

	ops->mon_ops = &dp_ops_mon;
	ops->cmn_drv_ops->txrx_set_monitor_mode = dp_vdev_set_monitor_mode;
}

QDF_STATUS dp_mon_soc_attach(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc;

	if (!soc) {
		mon_init_err("dp_soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mon_soc = (struct dp_mon_soc *)qdf_mem_malloc(sizeof(*mon_soc));
	if (!mon_soc) {
		mon_init_err("%pK: mem allocation failed", soc);
		return QDF_STATUS_E_NOMEM;
	}
	/* register monitor ops */
	dp_mon_ops_register(mon_soc);
	soc->monitor_soc = mon_soc;

	dp_mon_cdp_ops_register(soc);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_mon_soc_detach(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc;

	if (!soc) {
		mon_init_err("dp_soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mon_soc = soc->monitor_soc;
	soc->monitor_soc = NULL;
	qdf_mem_free(mon_soc);
	return QDF_STATUS_SUCCESS;
}
