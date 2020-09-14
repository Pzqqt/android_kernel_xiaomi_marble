/*
 * Copyright (c) 2016-2020, The Linux Foundation. All rights reserved.
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

#include "osdep.h"
#include "wmi.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_param.h"

/**
 * send_stats_request_cmd_tlv() - WMI request stats function
 * @param wmi_handle: handle to WMI.
 * @param macaddr: MAC address
 * @param param: pointer to hold stats request parameter
 *
 * Return: 0  on success and -ve on failure.
 */
static QDF_STATUS
send_stats_request_cmd_tlv(wmi_unified_t wmi_handle,
			   uint8_t macaddr[QDF_MAC_ADDR_SIZE],
			   struct stats_request_params *param)
{
	int32_t ret;
	wmi_request_stats_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_request_stats_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));
	cmd->stats_id = param->stats_id;
	cmd->vdev_id = param->vdev_id;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							wmi_handle,
							param->pdev_id);

	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);

	wmi_debug("STATS REQ STATS_ID:%d VDEV_ID:%d PDEV_ID:%d-->",
		 cmd->stats_id, cmd->vdev_id, cmd->pdev_id);

	wmi_mtrace(WMI_REQUEST_STATS_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send_pm_chk(wmi_handle, buf, len,
					  WMI_REQUEST_STATS_CMDID);

	if (ret) {
		wmi_err("Failed to send status request to fw =%d", ret);
		wmi_buf_free(buf);
	}

	return qdf_status_from_os_return(ret);
}

/**
 * extract_all_stats_counts_tlv() - extract all stats count from event
 * @param wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param stats_param: Pointer to hold stats count
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_all_stats_counts_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			     wmi_host_stats_event *stats_param)
{
	wmi_stats_event_fixed_param *ev;
	wmi_per_chain_rssi_stats *rssi_event;
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	uint64_t min_data_len;
	uint32_t i;

	qdf_mem_zero(stats_param, sizeof(*stats_param));
	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	rssi_event = param_buf->chain_stats;
	if (!ev) {
		wmi_err("event fixed param NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (param_buf->num_data > WMI_SVC_MSG_MAX_SIZE - sizeof(*ev)) {
		wmi_err("num_data : %u is invalid", param_buf->num_data);
		return QDF_STATUS_E_FAULT;
	}

	for (i = 1; i <= WMI_REQUEST_VDEV_EXTD_STAT; i = i << 1) {
		switch (ev->stats_id & i) {
		case WMI_REQUEST_PEER_STAT:
			stats_param->stats_id |= WMI_HOST_REQUEST_PEER_STAT;
			break;

		case WMI_REQUEST_AP_STAT:
			stats_param->stats_id |= WMI_HOST_REQUEST_AP_STAT;
			break;

		case WMI_REQUEST_PDEV_STAT:
			stats_param->stats_id |= WMI_HOST_REQUEST_PDEV_STAT;
			break;

		case WMI_REQUEST_VDEV_STAT:
			stats_param->stats_id |= WMI_HOST_REQUEST_VDEV_STAT;
			break;

		case WMI_REQUEST_BCNFLT_STAT:
			stats_param->stats_id |= WMI_HOST_REQUEST_BCNFLT_STAT;
			break;

		case WMI_REQUEST_VDEV_RATE_STAT:
			stats_param->stats_id |=
				WMI_HOST_REQUEST_VDEV_RATE_STAT;
			break;

		case WMI_REQUEST_BCN_STAT:
			stats_param->stats_id |= WMI_HOST_REQUEST_BCN_STAT;
			break;
		case WMI_REQUEST_PEER_EXTD_STAT:
			stats_param->stats_id |= WMI_REQUEST_PEER_EXTD_STAT;
			break;

		case WMI_REQUEST_PEER_EXTD2_STAT:
			stats_param->stats_id |=
				WMI_HOST_REQUEST_PEER_ADV_STATS;
			break;

		case WMI_REQUEST_PMF_BCN_PROTECT_STAT:
			stats_param->stats_id |=
				WMI_HOST_REQUEST_PMF_BCN_PROTECT_STAT;
			break;

		case WMI_REQUEST_VDEV_EXTD_STAT:
			stats_param->stats_id |=
				WMI_HOST_REQUEST_VDEV_PRB_FILS_STAT;
			break;
		}
	}

	/* ev->num_*_stats may cause uint32_t overflow, so use uint64_t
	 * to save total length calculated
	 */
	min_data_len =
		(((uint64_t)ev->num_pdev_stats) * sizeof(wmi_pdev_stats)) +
		(((uint64_t)ev->num_vdev_stats) * sizeof(wmi_vdev_stats)) +
		(((uint64_t)ev->num_peer_stats) * sizeof(wmi_peer_stats)) +
		(((uint64_t)ev->num_bcnflt_stats) *
		 sizeof(wmi_bcnfilter_stats_t)) +
		(((uint64_t)ev->num_chan_stats) * sizeof(wmi_chan_stats)) +
		(((uint64_t)ev->num_mib_stats) * sizeof(wmi_mib_stats)) +
		(((uint64_t)ev->num_bcn_stats) * sizeof(wmi_bcn_stats)) +
		(((uint64_t)ev->num_peer_extd_stats) *
		 sizeof(wmi_peer_extd_stats)) +
		(((uint64_t)ev->num_mib_extd_stats) *
		 sizeof(wmi_mib_extd_stats));
	if (param_buf->num_data != min_data_len) {
		wmi_err("data len: %u isn't same as calculated: %llu",
			 param_buf->num_data, min_data_len);
		return QDF_STATUS_E_FAULT;
	}

	stats_param->last_event = ev->last_event;
	stats_param->num_pdev_stats = ev->num_pdev_stats;
	stats_param->num_pdev_ext_stats = 0;
	stats_param->num_vdev_stats = ev->num_vdev_stats;
	stats_param->num_peer_stats = ev->num_peer_stats;
	stats_param->num_peer_extd_stats = ev->num_peer_extd_stats;
	stats_param->num_bcnflt_stats = ev->num_bcnflt_stats;
	stats_param->num_chan_stats = ev->num_chan_stats;
	stats_param->num_mib_stats = ev->num_mib_stats;
	stats_param->num_mib_extd_stats = ev->num_mib_extd_stats;
	stats_param->num_bcn_stats = ev->num_bcn_stats;
	stats_param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							wmi_handle,
							ev->pdev_id);

	/* if chain_stats is not populated */
	if (!param_buf->chain_stats || !param_buf->num_chain_stats)
		return QDF_STATUS_SUCCESS;

	if (WMITLV_TAG_STRUC_wmi_per_chain_rssi_stats !=
	    WMITLV_GET_TLVTAG(rssi_event->tlv_header))
		return QDF_STATUS_SUCCESS;

	if (WMITLV_GET_STRUCT_TLVLEN(wmi_per_chain_rssi_stats) !=
	    WMITLV_GET_TLVLEN(rssi_event->tlv_header))
		return QDF_STATUS_SUCCESS;

	if (rssi_event->num_per_chain_rssi_stats >=
	    WMITLV_GET_TLVLEN(rssi_event->tlv_header)) {
		wmi_err("num_per_chain_rssi_stats:%u is out of bounds",
			 rssi_event->num_per_chain_rssi_stats);
		return QDF_STATUS_E_INVAL;
	}
	stats_param->num_rssi_stats = rssi_event->num_per_chain_rssi_stats;

	if (param_buf->vdev_extd_stats)
		stats_param->num_vdev_extd_stats =
			param_buf->num_vdev_extd_stats;

	/* if peer_adv_stats is not populated */
	if (param_buf->num_peer_extd2_stats)
		stats_param->num_peer_adv_stats =
			param_buf->num_peer_extd2_stats;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_tx_stats() - extract pdev tx stats from event
 */
static void extract_pdev_tx_stats(wmi_host_dbg_tx_stats *tx,
				  struct wlan_dbg_tx_stats *tx_stats)
{
	/* Tx Stats */
	tx->comp_queued = tx_stats->comp_queued;
	tx->comp_delivered = tx_stats->comp_delivered;
	tx->msdu_enqued = tx_stats->msdu_enqued;
	tx->mpdu_enqued = tx_stats->mpdu_enqued;
	tx->wmm_drop = tx_stats->wmm_drop;
	tx->local_enqued = tx_stats->local_enqued;
	tx->local_freed = tx_stats->local_freed;
	tx->hw_queued = tx_stats->hw_queued;
	tx->hw_reaped = tx_stats->hw_reaped;
	tx->underrun = tx_stats->underrun;
	tx->tx_abort = tx_stats->tx_abort;
	tx->mpdus_requed = tx_stats->mpdus_requed;
	tx->data_rc = tx_stats->data_rc;
	tx->self_triggers = tx_stats->self_triggers;
	tx->sw_retry_failure = tx_stats->sw_retry_failure;
	tx->illgl_rate_phy_err = tx_stats->illgl_rate_phy_err;
	tx->pdev_cont_xretry = tx_stats->pdev_cont_xretry;
	tx->pdev_tx_timeout = tx_stats->pdev_tx_timeout;
	tx->pdev_resets = tx_stats->pdev_resets;
	tx->stateless_tid_alloc_failure = tx_stats->stateless_tid_alloc_failure;
	tx->phy_underrun = tx_stats->phy_underrun;
	tx->txop_ovf = tx_stats->txop_ovf;

	return;
}


/**
 * extract_pdev_rx_stats() - extract pdev rx stats from event
 */
static void extract_pdev_rx_stats(wmi_host_dbg_rx_stats *rx,
				  struct wlan_dbg_rx_stats *rx_stats)
{
	/* Rx Stats */
	rx->mid_ppdu_route_change = rx_stats->mid_ppdu_route_change;
	rx->status_rcvd = rx_stats->status_rcvd;
	rx->r0_frags = rx_stats->r0_frags;
	rx->r1_frags = rx_stats->r1_frags;
	rx->r2_frags = rx_stats->r2_frags;
	/* Only TLV */
	rx->r3_frags = 0;
	rx->htt_msdus = rx_stats->htt_msdus;
	rx->htt_mpdus = rx_stats->htt_mpdus;
	rx->loc_msdus = rx_stats->loc_msdus;
	rx->loc_mpdus = rx_stats->loc_mpdus;
	rx->oversize_amsdu = rx_stats->oversize_amsdu;
	rx->phy_errs = rx_stats->phy_errs;
	rx->phy_err_drop = rx_stats->phy_err_drop;
	rx->mpdu_errs = rx_stats->mpdu_errs;

	return;
}

/**
 * extract_pdev_stats_tlv() - extract pdev stats from event
 * @param wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into pdev stats
 * @param pdev_stats: Pointer to hold pdev stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_pdev_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf, uint32_t index,
		       wmi_host_pdev_stats *pdev_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;

	data = param_buf->data;

	if (index < ev_param->num_pdev_stats) {
		wmi_pdev_stats *ev = (wmi_pdev_stats *) ((data) +
				(index * sizeof(wmi_pdev_stats)));

		pdev_stats->chan_nf = ev->chan_nf;
		pdev_stats->tx_frame_count = ev->tx_frame_count;
		pdev_stats->rx_frame_count = ev->rx_frame_count;
		pdev_stats->rx_clear_count = ev->rx_clear_count;
		pdev_stats->cycle_count = ev->cycle_count;
		pdev_stats->phy_err_count = ev->phy_err_count;
		pdev_stats->chan_tx_pwr = ev->chan_tx_pwr;

		extract_pdev_tx_stats(&(pdev_stats->pdev_stats.tx),
			&(ev->pdev_stats.tx));
		extract_pdev_rx_stats(&(pdev_stats->pdev_stats.rx),
			&(ev->pdev_stats.rx));
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_stats_tlv() - extract vdev stats from event
 * @param wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into vdev stats
 * @param vdev_stats: Pointer to hold vdev stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_vdev_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_vdev_stats *vdev_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	data = (uint8_t *) param_buf->data;

	if (index < ev_param->num_vdev_stats) {
		wmi_vdev_stats *ev = (wmi_vdev_stats *) ((data) +
				((ev_param->num_pdev_stats) *
				sizeof(wmi_pdev_stats)) +
				(index * sizeof(wmi_vdev_stats)));

		vdev_stats->vdev_id = ev->vdev_id;
		vdev_stats->vdev_snr.bcn_snr = ev->vdev_snr.bcn_snr;
		vdev_stats->vdev_snr.dat_snr = ev->vdev_snr.dat_snr;

		OS_MEMCPY(vdev_stats->tx_frm_cnt, ev->tx_frm_cnt,
			sizeof(ev->tx_frm_cnt));
		vdev_stats->rx_frm_cnt = ev->rx_frm_cnt;
		OS_MEMCPY(vdev_stats->multiple_retry_cnt,
				ev->multiple_retry_cnt,
				sizeof(ev->multiple_retry_cnt));
		OS_MEMCPY(vdev_stats->fail_cnt, ev->fail_cnt,
				sizeof(ev->fail_cnt));
		vdev_stats->rts_fail_cnt = ev->rts_fail_cnt;
		vdev_stats->rts_succ_cnt = ev->rts_succ_cnt;
		vdev_stats->rx_err_cnt = ev->rx_err_cnt;
		vdev_stats->rx_discard_cnt = ev->rx_discard_cnt;
		vdev_stats->ack_fail_cnt = ev->ack_fail_cnt;
		OS_MEMCPY(vdev_stats->tx_rate_history, ev->tx_rate_history,
			sizeof(ev->tx_rate_history));
		OS_MEMCPY(vdev_stats->bcn_rssi_history, ev->bcn_rssi_history,
			sizeof(ev->bcn_rssi_history));

	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_stats_tlv() - extract peer stats from event
 * @param wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into peer stats
 * @param peer_stats: Pointer to hold peer stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_peer_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf, uint32_t index,
		       wmi_host_peer_stats *peer_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	data = (uint8_t *) param_buf->data;

	if (index < ev_param->num_peer_stats) {
		wmi_peer_stats *ev = (wmi_peer_stats *) ((data) +
			((ev_param->num_pdev_stats) * sizeof(wmi_pdev_stats)) +
			((ev_param->num_vdev_stats) * sizeof(wmi_vdev_stats)) +
			(index * sizeof(wmi_peer_stats)));

		OS_MEMSET(peer_stats, 0, sizeof(wmi_host_peer_stats));

		OS_MEMCPY(&(peer_stats->peer_macaddr),
			&(ev->peer_macaddr), sizeof(wmi_mac_addr));

		peer_stats->peer_rssi = ev->peer_rssi;
		peer_stats->peer_tx_rate = ev->peer_tx_rate;
		peer_stats->peer_rx_rate = ev->peer_rx_rate;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_extd_stats_tlv() - extract extended peer stats from event
 * @param wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended peer stats
 * @param peer_extd_stats: Pointer to hold extended peer stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
extract_peer_extd_stats_tlv(wmi_unified_t wmi_handle,
			    void *evt_buf, uint32_t index,
			    wmi_host_peer_extd_stats *peer_extd_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *)evt_buf;
	ev_param = (wmi_stats_event_fixed_param *)param_buf->fixed_param;
	data = (uint8_t *)param_buf->data;
	if (!data)
		return QDF_STATUS_E_FAILURE;

	if (index < ev_param->num_peer_extd_stats) {
		wmi_peer_extd_stats *ev = (wmi_peer_extd_stats *) (data +
			(ev_param->num_pdev_stats * sizeof(wmi_pdev_stats)) +
			(ev_param->num_vdev_stats * sizeof(wmi_vdev_stats)) +
			(ev_param->num_peer_stats * sizeof(wmi_peer_stats)) +
			(ev_param->num_bcnflt_stats *
			sizeof(wmi_bcnfilter_stats_t)) +
			(ev_param->num_chan_stats * sizeof(wmi_chan_stats)) +
			(ev_param->num_mib_stats * sizeof(wmi_mib_stats)) +
			(ev_param->num_bcn_stats * sizeof(wmi_bcn_stats)) +
			(index * sizeof(wmi_peer_extd_stats)));

		qdf_mem_zero(peer_extd_stats, sizeof(wmi_host_peer_extd_stats));
		qdf_mem_copy(&peer_extd_stats->peer_macaddr, &ev->peer_macaddr,
			     sizeof(wmi_mac_addr));

		peer_extd_stats->rx_mc_bc_cnt = ev->rx_mc_bc_cnt;
	}

	return QDF_STATUS_SUCCESS;

}

/**
 * extract_pmf_bcn_protect_stats_tlv() - extract pmf bcn stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @pmf_bcn_stats: Pointer to hold pmf bcn protect stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */

static QDF_STATUS
extract_pmf_bcn_protect_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
				  wmi_host_pmf_bcn_protect_stats *pmf_bcn_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf)
		return QDF_STATUS_E_FAILURE;

	ev_param = (wmi_stats_event_fixed_param *)param_buf->fixed_param;

	if ((ev_param->stats_id & WMI_REQUEST_PMF_BCN_PROTECT_STAT) &&
	    param_buf->pmf_bcn_protect_stats) {
		pmf_bcn_stats->igtk_mic_fail_cnt =
			param_buf->pmf_bcn_protect_stats->igtk_mic_fail_cnt;
		pmf_bcn_stats->igtk_replay_cnt =
			param_buf->pmf_bcn_protect_stats->igtk_replay_cnt;
		pmf_bcn_stats->bcn_mic_fail_cnt =
			param_buf->pmf_bcn_protect_stats->bcn_mic_fail_cnt;
		pmf_bcn_stats->bcn_replay_cnt =
			param_buf->pmf_bcn_protect_stats->bcn_replay_cnt;
	}

	return QDF_STATUS_SUCCESS;
}

void wmi_cp_stats_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_stats_request_cmd = send_stats_request_cmd_tlv;
	ops->extract_all_stats_count = extract_all_stats_counts_tlv;
	ops->extract_pdev_stats = extract_pdev_stats_tlv;
	ops->extract_vdev_stats = extract_vdev_stats_tlv;
	ops->extract_peer_stats = extract_peer_stats_tlv;
	ops->extract_peer_extd_stats = extract_peer_extd_stats_tlv;
	ops->extract_pmf_bcn_protect_stats = extract_pmf_bcn_protect_stats_tlv,

	wmi_mc_cp_stats_attach_tlv(wmi_handle);
}
