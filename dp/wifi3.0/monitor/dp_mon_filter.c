/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#include <dp_types.h>
#include <dp_htt.h>
#include <dp_internal.h>
#include <dp_rx_mon.h>
#include <dp_mon_filter.h>
#include <dp_mon.h>

QDF_STATUS
dp_mon_ht2_rx_ring_cfg(struct dp_soc *soc,
		       struct dp_pdev *pdev,
		       enum dp_mon_filter_srng_type srng_type,
		       struct htt_rx_ring_tlv_filter *tlv_filter)
{
	int mac_id;
	int max_mac_rings = wlan_cfg_get_num_mac_rings(pdev->wlan_cfg_ctx);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/*
	 * Overwrite the max_mac_rings for the status rings.
	 */
	if (srng_type == DP_MON_FILTER_SRNG_TYPE_RXDMA_MONITOR_STATUS)
		dp_is_hw_dbs_enable(soc, &max_mac_rings);

	dp_mon_filter_info("%pK: srng type %d Max_mac_rings %d ",
			   soc, srng_type, max_mac_rings);

	/*
	 * Loop through all MACs per radio and set the filter to the individual
	 * macs. For MCL
	 */
	for (mac_id = 0; mac_id < max_mac_rings; mac_id++) {
		int mac_for_pdev =
			dp_get_mac_id_for_pdev(mac_id, pdev->pdev_id);
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id, pdev->pdev_id);
		int hal_ring_type, ring_buf_size;
		hal_ring_handle_t hal_ring_hdl;

		switch (srng_type) {
		case DP_MON_FILTER_SRNG_TYPE_RXDMA_BUF:
			hal_ring_hdl = pdev->rx_mac_buf_ring[lmac_id].hal_srng;
			hal_ring_type = RXDMA_BUF;
			ring_buf_size = RX_DATA_BUFFER_SIZE;
			break;

		case DP_MON_FILTER_SRNG_TYPE_RXDMA_MONITOR_STATUS:
			/*
			 * If two back to back HTT msg sending happened in
			 * short time, the second HTT msg source SRNG HP
			 * writing has chance to fail, this has been confirmed
			 * by HST HW.
			 * for monitor mode, here is the last HTT msg for sending.
			 * if the 2nd HTT msg for monitor status ring sending failed,
			 * HW won't provide anything into 2nd monitor status ring.
			 * as a WAR, add some delay before 2nd HTT msg start sending,
			 * > 2us is required per HST HW, delay 100 us for safe.
			 */
			if (mac_id)
				qdf_udelay(100);

			hal_ring_hdl =
				soc->rxdma_mon_status_ring[lmac_id].hal_srng;
			hal_ring_type = RXDMA_MONITOR_STATUS;
			ring_buf_size = RX_MON_STATUS_BUF_SIZE;
			break;

		case DP_MON_FILTER_SRNG_TYPE_RXDMA_MON_BUF:
			hal_ring_hdl =
				soc->rxdma_mon_buf_ring[lmac_id].hal_srng;
			hal_ring_type = RXDMA_MONITOR_BUF;
			ring_buf_size = RX_DATA_BUFFER_SIZE;
			break;

		default:
			return QDF_STATUS_E_FAILURE;
		}

		status = htt_h2t_rx_ring_cfg(soc->htt_handle, mac_for_pdev,
					     hal_ring_hdl, hal_ring_type,
					     ring_buf_size,
					     tlv_filter);
		if (status != QDF_STATUS_SUCCESS)
			return status;
	}

	return status;
}

#ifdef QCA_ENHANCED_STATS_SUPPORT
void dp_mon_filter_setup_enhanced_stats(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_setup_enhanced_stats)
		mon_ops->mon_filter_setup_enhanced_stats(pdev);
}

void dp_mon_filter_reset_enhanced_stats(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_reset_enhanced_stats)
		mon_ops->mon_filter_reset_enhanced_stats(pdev);
}
#endif /* QCA_ENHANCED_STATS_SUPPORT */

#ifdef QCA_MCOPY_SUPPORT
void dp_mon_filter_setup_mcopy_mode(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_setup_mcopy_mode)
		mon_ops->mon_filter_setup_mcopy_mode(pdev);
}

void dp_mon_filter_reset_mcopy_mode(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_reset_mcopy_mode)
		mon_ops->mon_filter_reset_mcopy_mode(pdev);
}
#endif /* QCA_MCOPY_SUPPORT */

#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
void dp_mon_filter_setup_smart_monitor(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_setup_smart_monitor)
		mon_ops->mon_filter_setup_smart_monitor(pdev);
}

void dp_mon_filter_reset_smart_monitor(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_reset_smart_monitor)
		mon_ops->mon_filter_reset_smart_monitor(pdev);
}
#endif /* ATH_SUPPORT_NAC_RSSI || ATH_SUPPORT_NAC */

#ifdef WLAN_RX_PKT_CAPTURE_ENH
void dp_mon_filter_setup_rx_enh_capture(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_setup_rx_enh_capture)
		mon_ops->mon_filter_setup_rx_enh_capture(pdev);
}

void dp_mon_filter_reset_rx_enh_capture(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_reset_rx_enh_capture)
		mon_ops->mon_filter_reset_rx_enh_capture(pdev);
}
#endif /* WLAN_RX_PKT_CAPTURE_ENH */

void dp_mon_filter_setup_mon_mode(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_setup_mon_mode)
		mon_ops->mon_filter_setup_mon_mode(pdev);
}

void dp_mon_filter_reset_mon_mode(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_reset_mon_mode)
		mon_ops->mon_filter_reset_mon_mode(pdev);
}

#ifdef WDI_EVENT_ENABLE
void dp_mon_filter_setup_rx_pkt_log_full(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_setup_rx_pkt_log_full)
		mon_ops->mon_filter_setup_rx_pkt_log_full(pdev);
}

void dp_mon_filter_reset_rx_pkt_log_full(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_reset_rx_pkt_log_full)
		mon_ops->mon_filter_reset_rx_pkt_log_full(pdev);
}

void dp_mon_filter_setup_rx_pkt_log_lite(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_setup_rx_pkt_log_lite)
		mon_ops->mon_filter_setup_rx_pkt_log_lite(pdev);
}

void dp_mon_filter_reset_rx_pkt_log_lite(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_reset_rx_pkt_log_lite)
		mon_ops->mon_filter_reset_rx_pkt_log_lite(pdev);
}

void dp_mon_filter_setup_rx_pkt_log_cbf(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_setup_rx_pkt_log_cbf)
		mon_ops->mon_filter_setup_rx_pkt_log_cbf(pdev);
}

void dp_mon_filter_reset_rx_pktlog_cbf(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_reset_rx_pkt_log_cbf)
		mon_ops->mon_filter_reset_rx_pkt_log_cbf(pdev);
}
#endif /* WDI_EVENT_ENABLE */

QDF_STATUS dp_mon_filter_update(struct dp_pdev *pdev)
{
	struct dp_mon_ops *mon_ops = NULL;

	mon_ops = dp_mon_ops_get(pdev->soc);
	if (mon_ops && mon_ops->mon_filter_update)
		return mon_ops->mon_filter_update(pdev);

	return QDF_STATUS_E_FAILURE;
}

void
dp_mon_filter_reset_mon_srng(struct dp_soc *soc, struct dp_pdev *pdev,
			     enum dp_mon_filter_srng_type mon_srng_type)
{
	struct htt_rx_ring_tlv_filter tlv_filter = {0};

	if (dp_mon_ht2_rx_ring_cfg(soc, pdev, mon_srng_type,
				   &tlv_filter) != QDF_STATUS_SUCCESS) {
		dp_mon_filter_err("%pK: Monitor destinatin ring filter setting failed",
				  soc);
	}
}

void dp_mon_filter_set_mon_cmn(struct dp_mon_pdev *mon_pdev,
			       struct dp_mon_filter *filter)
{
	filter->tlv_filter.mpdu_start = 1;
	filter->tlv_filter.msdu_start = 1;
	filter->tlv_filter.packet = 1;
	filter->tlv_filter.msdu_end = 1;
	filter->tlv_filter.mpdu_end = 1;
	filter->tlv_filter.packet_header = 1;
	filter->tlv_filter.attention = 1;
	filter->tlv_filter.ppdu_start = 0;
	filter->tlv_filter.ppdu_end = 0;
	filter->tlv_filter.ppdu_end_user_stats = 0;
	filter->tlv_filter.ppdu_end_user_stats_ext = 0;
	filter->tlv_filter.ppdu_end_status_done = 0;
	filter->tlv_filter.header_per_msdu = 1;
	filter->tlv_filter.enable_fp =
		(mon_pdev->mon_filter_mode & MON_FILTER_PASS) ? 1 : 0;
	filter->tlv_filter.enable_mo =
		(mon_pdev->mon_filter_mode & MON_FILTER_OTHER) ? 1 : 0;

	filter->tlv_filter.fp_mgmt_filter = mon_pdev->fp_mgmt_filter;
	filter->tlv_filter.fp_ctrl_filter = mon_pdev->fp_ctrl_filter;
	filter->tlv_filter.fp_data_filter = mon_pdev->fp_data_filter;
	filter->tlv_filter.mo_mgmt_filter = mon_pdev->mo_mgmt_filter;
	filter->tlv_filter.mo_ctrl_filter = mon_pdev->mo_ctrl_filter;
	filter->tlv_filter.mo_data_filter = mon_pdev->mo_data_filter;
	filter->tlv_filter.offset_valid = false;
}

void dp_mon_filter_set_status_cmn(struct dp_mon_pdev *mon_pdev,
				  struct dp_mon_filter *filter)
{
	filter->tlv_filter.mpdu_start = 1;
	filter->tlv_filter.msdu_start = 0;
	filter->tlv_filter.packet = 0;
	filter->tlv_filter.msdu_end = 0;
	filter->tlv_filter.mpdu_end = 0;
	filter->tlv_filter.attention = 0;
	filter->tlv_filter.ppdu_start = 1;
	filter->tlv_filter.ppdu_end = 1;
	filter->tlv_filter.ppdu_end_user_stats = 1;
	filter->tlv_filter.ppdu_end_user_stats_ext = 1;
	filter->tlv_filter.ppdu_end_status_done = 1;
	filter->tlv_filter.enable_fp = 1;
	filter->tlv_filter.enable_md = 0;
	filter->tlv_filter.fp_mgmt_filter = FILTER_MGMT_ALL;
	filter->tlv_filter.fp_ctrl_filter = FILTER_CTRL_ALL;
	filter->tlv_filter.fp_data_filter = FILTER_DATA_ALL;
	filter->tlv_filter.offset_valid = false;

	if (mon_pdev->mon_filter_mode & MON_FILTER_OTHER) {
		filter->tlv_filter.enable_mo = 1;
		filter->tlv_filter.mo_mgmt_filter = FILTER_MGMT_ALL;
		filter->tlv_filter.mo_ctrl_filter = FILTER_CTRL_ALL;
		filter->tlv_filter.mo_data_filter = FILTER_DATA_ALL;
	} else {
		filter->tlv_filter.enable_mo = 0;
	}
}

void dp_mon_filter_set_status_cbf(struct dp_pdev *pdev,
				  struct dp_mon_filter *filter)
{
	filter->tlv_filter.mpdu_start = 1;
	filter->tlv_filter.msdu_start = 0;
	filter->tlv_filter.packet = 0;
	filter->tlv_filter.msdu_end = 0;
	filter->tlv_filter.mpdu_end = 0;
	filter->tlv_filter.attention = 0;
	filter->tlv_filter.ppdu_start = 1;
	filter->tlv_filter.ppdu_end = 1;
	filter->tlv_filter.ppdu_end_user_stats = 1;
	filter->tlv_filter.ppdu_end_user_stats_ext = 1;
	filter->tlv_filter.ppdu_end_status_done = 1;
	filter->tlv_filter.enable_fp = 1;
	filter->tlv_filter.enable_md = 0;
	filter->tlv_filter.fp_mgmt_filter = FILTER_MGMT_ACT_NO_ACK;
	filter->tlv_filter.fp_ctrl_filter = 0;
	filter->tlv_filter.fp_data_filter = 0;
	filter->tlv_filter.offset_valid = false;
	filter->tlv_filter.enable_mo = 0;
}

void dp_mon_filter_set_cbf_cmn(struct dp_pdev *pdev,
			       struct dp_mon_filter *filter)
{
	filter->tlv_filter.mpdu_start = 1;
	filter->tlv_filter.msdu_start = 1;
	filter->tlv_filter.packet = 1;
	filter->tlv_filter.msdu_end = 1;
	filter->tlv_filter.mpdu_end = 1;
	filter->tlv_filter.attention = 1;
	filter->tlv_filter.ppdu_start = 0;
	filter->tlv_filter.ppdu_end =  0;
	filter->tlv_filter.ppdu_end_user_stats = 0;
	filter->tlv_filter.ppdu_end_user_stats_ext = 0;
	filter->tlv_filter.ppdu_end_status_done = 0;
	filter->tlv_filter.enable_fp = 1;
	filter->tlv_filter.enable_md = 0;
	filter->tlv_filter.fp_mgmt_filter = FILTER_MGMT_ACT_NO_ACK;
	filter->tlv_filter.offset_valid = false;
	filter->tlv_filter.enable_mo = 0;
}

/**
 * dp_mon_filter_dealloc() - Deallocate the filter objects to be stored in
 * the radio object.
 * @pdev: DP pdev handle
 */
void dp_mon_filter_dealloc(struct dp_mon_pdev *mon_pdev)
{
	enum dp_mon_filter_mode mode;
	struct dp_mon_filter **mon_filter = NULL;

	if (!mon_pdev) {
		dp_mon_filter_err("Monitor pdev Context is null");
		return;
	}

	mon_filter = mon_pdev->filter;

	/*
	 * Check if the monitor filters are already allocated to the mon_pdev.
	 */
	if (!mon_filter) {
		dp_mon_filter_err("Found NULL memmory for the Monitor filter");
		return;
	}

	/*
	 * Iterate through the every mode and free the filter object.
	 */
	for (mode = 0; mode < DP_MON_FILTER_MAX_MODE; mode++) {
		if (!mon_filter[mode]) {
			continue;
		}

		qdf_mem_free(mon_filter[mode]);
		mon_filter[mode] = NULL;
	}

	qdf_mem_free(mon_filter);
	mon_pdev->filter = NULL;
}

/**
 * dp_mon_filter_alloc() - Allocate the filter objects to be stored in
 * the radio object.
 * @mon_pdev: Monitor pdev handle
 */
struct dp_mon_filter **dp_mon_filter_alloc(struct dp_mon_pdev *mon_pdev)
{
	struct dp_mon_filter **mon_filter = NULL;
	enum dp_mon_filter_mode mode;

	if (!mon_pdev) {
		dp_mon_filter_err("pdev Context is null");
		return NULL;
	}

	mon_filter = (struct dp_mon_filter **)qdf_mem_malloc(
			(sizeof(struct dp_mon_filter *) *
			 DP_MON_FILTER_MAX_MODE));
	if (!mon_filter) {
		dp_mon_filter_err("Monitor filter mem allocation failed");
		return NULL;
	}

	qdf_mem_zero(mon_filter,
		     sizeof(struct dp_mon_filter *) * DP_MON_FILTER_MAX_MODE);

	/*
	 * Allocate the memory for filters for different srngs for each modes.
	 */
	for (mode = 0; mode < DP_MON_FILTER_MAX_MODE; mode++) {
		mon_filter[mode] = qdf_mem_malloc(sizeof(struct dp_mon_filter) *
						  DP_MON_FILTER_SRNG_TYPE_MAX);
		/* Assign the mon_filter to the pdev->filter such
		 * that the dp_mon_filter_dealloc() can free up the filters. */
		if (!mon_filter[mode]) {
			mon_pdev->filter = mon_filter;
			goto fail;
		}
	}

	return mon_filter;
fail:
	dp_mon_filter_dealloc(mon_pdev);
	return NULL;
}
