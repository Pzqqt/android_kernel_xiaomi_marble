/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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

/**
 * DOC: target_if_cp_stats.c
 *
 * This file provide definition for APIs registered through lmac Tx Ops
 */

#include <qdf_mem.h>
#include <qdf_status.h>
#include <target_if_cp_stats.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_param.h>
#include <target_if.h>
#include <wlan_tgt_def_config.h>
#include <wmi_unified_api.h>
#include <wlan_osif_priv.h>
#include <wlan_cp_stats_utils_api.h>
#include <wlan_cp_stats_mc_tgt_api.h>
#include "../../../umac/cmn_services/utils/inc/wlan_utility.h"

#ifdef WLAN_FEATURE_MIB_STATS
static void target_if_cp_stats_free_mib_stats(struct stats_event *ev)
{
	qdf_mem_free(ev->mib_stats);
	ev->mib_stats = NULL;
}
#else
static void target_if_cp_stats_free_mib_stats(struct stats_event *ev)
{
}
#endif
static void target_if_cp_stats_free_stats_event(struct stats_event *ev)
{
	qdf_mem_free(ev->pdev_stats);
	ev->pdev_stats = NULL;
	qdf_mem_free(ev->peer_stats);
	ev->peer_stats = NULL;
	qdf_mem_free(ev->peer_adv_stats);
	ev->peer_adv_stats = NULL;
	qdf_mem_free(ev->peer_extended_stats);
	ev->peer_extended_stats = NULL;
	qdf_mem_free(ev->cca_stats);
	ev->cca_stats = NULL;
	qdf_mem_free(ev->vdev_summary_stats);
	ev->vdev_summary_stats = NULL;
	qdf_mem_free(ev->vdev_chain_rssi);
	ev->vdev_chain_rssi = NULL;
	target_if_cp_stats_free_mib_stats(ev);
}

static QDF_STATUS target_if_cp_stats_extract_pdev_stats(
					struct wmi_unified *wmi_hdl,
					wmi_host_stats_event *stats_param,
					struct stats_event *ev,
					uint8_t *data)
{
	uint32_t i;
	QDF_STATUS status;
	wmi_host_pdev_stats pdev_stats;

	ev->num_pdev_stats = stats_param->num_pdev_stats;
	if (!ev->num_pdev_stats)
		return QDF_STATUS_SUCCESS;

	/*
	 * num_pdev_stats is validated within function wmi_extract_stats_param
	 * which is called to populated wmi_host_stats_event stats_param
	 */
	ev->pdev_stats = qdf_mem_malloc(sizeof(*ev->pdev_stats) *
						ev->num_pdev_stats);
	if (!ev->pdev_stats)
		return QDF_STATUS_E_NOMEM;

	for (i = 0; i < ev->num_pdev_stats; i++) {
		status = wmi_extract_pdev_stats(wmi_hdl, data, i, &pdev_stats);
		if (QDF_IS_STATUS_ERROR(status)) {
			cp_stats_err("wmi_extract_pdev_stats failed");
			return status;
		}
		ev->pdev_stats[i].max_pwr = pdev_stats.chan_tx_pwr;
	}

	return QDF_STATUS_SUCCESS;
}

static void target_if_cp_stats_extract_peer_extd_stats(
	struct wmi_unified *wmi_hdl,
	wmi_host_stats_event *stats_param,
	struct stats_event *ev,
	uint8_t *data)

{
	QDF_STATUS status;
	uint32_t i;
	wmi_host_peer_extd_stats peer_extd_stats;

	if (!stats_param->num_peer_extd_stats)
		return;

	ev->peer_extended_stats =
			qdf_mem_malloc(sizeof(*ev->peer_extended_stats) *
				       stats_param->num_peer_extd_stats);
	if (!ev->peer_extended_stats)
		return;

	ev->num_peer_extd_stats = stats_param->num_peer_extd_stats;

	for (i = 0; i < ev->num_peer_extd_stats; i++) {
		status = wmi_extract_peer_extd_stats(wmi_hdl, data, i,
						     &peer_extd_stats);
		if (QDF_IS_STATUS_ERROR(status)) {
			cp_stats_err("wmi_extract_peer_extd_stats failed");
			continue;
		}
		WMI_MAC_ADDR_TO_CHAR_ARRAY(
			     &peer_extd_stats.peer_macaddr,
			ev->peer_extended_stats[i].peer_macaddr);
		ev->peer_extended_stats[i].rx_mc_bc_cnt =
						peer_extd_stats.rx_mc_bc_cnt;
	}
}

static QDF_STATUS target_if_cp_stats_extract_peer_stats(
					struct wmi_unified *wmi_hdl,
					wmi_host_stats_event *stats_param,
					struct stats_event *ev,
					uint8_t *data)
{
	uint32_t i;
	QDF_STATUS status;
	wmi_host_peer_stats peer_stats;
	bool db2dbm_enabled;
	struct wmi_host_peer_adv_stats *peer_adv_stats;

	/* Extract peer_stats */
	if (!stats_param->num_peer_stats)
		return QDF_STATUS_SUCCESS;

	ev->peer_stats = qdf_mem_malloc(sizeof(*ev->peer_stats) *
						stats_param->num_peer_stats);
	if (!ev->peer_stats)
		return QDF_STATUS_E_NOMEM;
	ev->num_peer_stats = stats_param->num_peer_stats;

	db2dbm_enabled = wmi_service_enabled(wmi_hdl,
					     wmi_service_hw_db2dbm_support);
	for (i = 0; i < ev->num_peer_stats; i++) {
		status = wmi_extract_peer_stats(wmi_hdl, data, i, &peer_stats);
		if (QDF_IS_STATUS_ERROR(status)) {
			cp_stats_err("wmi_extract_peer_stats failed");
			continue;
		}
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&peer_stats.peer_macaddr,
					   ev->peer_stats[i].peer_macaddr);
		ev->peer_stats[i].tx_rate = peer_stats.peer_tx_rate;
		ev->peer_stats[i].rx_rate = peer_stats.peer_rx_rate;
		if (db2dbm_enabled)
			ev->peer_stats[i].peer_rssi = peer_stats.peer_rssi;
		else
			ev->peer_stats[i].peer_rssi = peer_stats.peer_rssi +
							TGT_NOISE_FLOOR_DBM;
	}

	target_if_cp_stats_extract_peer_extd_stats(wmi_hdl, stats_param, ev,
						   data);

	/* Extract peer_adv_stats */
	ev->num_peer_adv_stats = stats_param->num_peer_adv_stats;
	if (!ev->num_peer_adv_stats)
		return QDF_STATUS_SUCCESS;

	ev->peer_adv_stats = qdf_mem_malloc(sizeof(*ev->peer_adv_stats) *
					    ev->num_peer_adv_stats);
	if (!ev->peer_adv_stats)
		return QDF_STATUS_E_NOMEM;

	peer_adv_stats = qdf_mem_malloc(sizeof(*peer_adv_stats) *
					ev->num_peer_adv_stats);
	if (!peer_adv_stats) {
		qdf_mem_free(ev->peer_adv_stats);
		return QDF_STATUS_E_NOMEM;
	}

	status = wmi_extract_peer_adv_stats(wmi_hdl, data, peer_adv_stats);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("wmi_extract_peer_stats failed");
		qdf_mem_free(peer_adv_stats);
		qdf_mem_free(ev->peer_adv_stats);
		ev->peer_adv_stats = NULL;
		return QDF_STATUS_SUCCESS;
	}

	for (i = 0; i < ev->num_peer_adv_stats; i++) {
		qdf_mem_copy(&ev->peer_adv_stats[i].peer_macaddr,
			     &peer_adv_stats[i].peer_macaddr,
			     QDF_MAC_ADDR_SIZE);
		ev->peer_adv_stats[i].fcs_count = peer_adv_stats[i].fcs_count;
		ev->peer_adv_stats[i].rx_bytes = peer_adv_stats[i].rx_bytes;
		ev->peer_adv_stats[i].rx_count = peer_adv_stats[i].rx_count;
	}
	qdf_mem_free(peer_adv_stats);
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_cp_stats_extract_cca_stats(
					struct wmi_unified *wmi_hdl,
					wmi_host_stats_event *stats_param,
					struct stats_event *ev, uint8_t *data)
{
	QDF_STATUS status;
	struct wmi_host_congestion_stats stats = {0};

	status = wmi_extract_cca_stats(wmi_hdl, data, &stats);
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_SUCCESS;

	ev->cca_stats = qdf_mem_malloc(sizeof(*ev->cca_stats));
	if (!ev->cca_stats)
		return QDF_STATUS_E_NOMEM;

	ev->cca_stats->vdev_id = stats.vdev_id;
	ev->cca_stats->congestion = stats.congestion;

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_MIB_STATS
static QDF_STATUS target_if_cp_stats_extract_mib_stats(
					struct wmi_unified *wmi_hdl,
					wmi_host_stats_event *stats_param,
					struct stats_event *ev, uint8_t *data)
{
	QDF_STATUS status;

	if (!stats_param->num_mib_stats)
		return QDF_STATUS_SUCCESS;

	if (stats_param->num_mib_stats != MAX_MIB_STATS ||
	    (stats_param->num_mib_extd_stats &&
	    stats_param->num_mib_extd_stats != MAX_MIB_STATS)) {
		cp_stats_err("number of mib stats wrong, num_mib_stats %d, num_mib_extd_stats %d",
			     stats_param->num_mib_stats,
			     stats_param->num_mib_extd_stats);
		return QDF_STATUS_E_INVAL;
	}

	ev->num_mib_stats = stats_param->num_mib_stats;

	ev->mib_stats = qdf_mem_malloc(sizeof(*ev->mib_stats));
	if (!ev->mib_stats)
		return QDF_STATUS_E_NOMEM;

	status = wmi_extract_mib_stats(wmi_hdl, data, ev->mib_stats);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("wmi_extract_mib_stats failed");
		return status;
	}

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS target_if_cp_stats_extract_mib_stats(
					struct wmi_unified *wmi_hdl,
					wmi_host_stats_event *stats_param,
					struct stats_event *ev, uint8_t *data)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static QDF_STATUS target_if_cp_stats_extract_vdev_summary_stats(
					struct wmi_unified *wmi_hdl,
					wmi_host_stats_event *stats_param,
					struct stats_event *ev, uint8_t *data)
{
	uint32_t i, j;
	QDF_STATUS status;
	int32_t bcn_snr, dat_snr;
	wmi_host_vdev_stats vdev_stats;
	bool db2dbm_enabled;

	ev->num_summary_stats = stats_param->num_vdev_stats;
	if (!ev->num_summary_stats)
		return QDF_STATUS_SUCCESS;

	ev->vdev_summary_stats = qdf_mem_malloc(sizeof(*ev->vdev_summary_stats)
					* ev->num_summary_stats);

	if (!ev->vdev_summary_stats)
		return QDF_STATUS_E_NOMEM;

	db2dbm_enabled = wmi_service_enabled(wmi_hdl,
					     wmi_service_hw_db2dbm_support);
	for (i = 0; i < ev->num_summary_stats; i++) {
		status = wmi_extract_vdev_stats(wmi_hdl, data, i, &vdev_stats);
		if (QDF_IS_STATUS_ERROR(status))
			continue;

		bcn_snr = vdev_stats.vdev_snr.bcn_snr;
		dat_snr = vdev_stats.vdev_snr.dat_snr;
		ev->vdev_summary_stats[i].vdev_id = vdev_stats.vdev_id;

		cp_stats_debug("vdev %d SNR bcn: %d data: %d",
			       ev->vdev_summary_stats[i].vdev_id, bcn_snr,
			       dat_snr);

		for (j = 0; j < 4; j++) {
			ev->vdev_summary_stats[i].stats.tx_frm_cnt[j]
					= vdev_stats.tx_frm_cnt[j];
			ev->vdev_summary_stats[i].stats.fail_cnt[j]
					= vdev_stats.fail_cnt[j];
			ev->vdev_summary_stats[i].stats.multiple_retry_cnt[j]
					= vdev_stats.multiple_retry_cnt[j];
		}

		ev->vdev_summary_stats[i].stats.rx_frm_cnt =
						vdev_stats.rx_frm_cnt;
		ev->vdev_summary_stats[i].stats.rx_error_cnt =
						vdev_stats.rx_err_cnt;
		ev->vdev_summary_stats[i].stats.rx_discard_cnt =
						vdev_stats.rx_discard_cnt;
		ev->vdev_summary_stats[i].stats.ack_fail_cnt =
						vdev_stats.ack_fail_cnt;
		ev->vdev_summary_stats[i].stats.rts_succ_cnt =
						vdev_stats.rts_succ_cnt;
		ev->vdev_summary_stats[i].stats.rts_fail_cnt =
						vdev_stats.rts_fail_cnt;
		/* Update SNR and RSSI in SummaryStats */
		wlan_util_stats_get_rssi(db2dbm_enabled, bcn_snr, dat_snr,
					 &ev->vdev_summary_stats[i].stats.rssi);
		ev->vdev_summary_stats[i].stats.snr =
				ev->vdev_summary_stats[i].stats.rssi -
				TGT_NOISE_FLOOR_DBM;
	}

	return QDF_STATUS_SUCCESS;
}


static QDF_STATUS target_if_cp_stats_extract_vdev_chain_rssi_stats(
					struct wmi_unified *wmi_hdl,
					wmi_host_stats_event *stats_param,
					struct stats_event *ev, uint8_t *data)
{
	uint32_t i, j;
	QDF_STATUS status;
	int32_t bcn_snr, dat_snr;
	struct wmi_host_per_chain_rssi_stats rssi_stats;
	bool db2dbm_enabled;

	ev->num_chain_rssi_stats = stats_param->num_rssi_stats;
	if (!ev->num_chain_rssi_stats)
		return QDF_STATUS_SUCCESS;

	ev->vdev_chain_rssi = qdf_mem_malloc(sizeof(*ev->vdev_chain_rssi) *
						ev->num_chain_rssi_stats);
	if (!ev->vdev_chain_rssi)
		return QDF_STATUS_E_NOMEM;

	db2dbm_enabled = wmi_service_enabled(wmi_hdl,
					     wmi_service_hw_db2dbm_support);
	for (i = 0; i < ev->num_chain_rssi_stats; i++) {
		status = wmi_extract_per_chain_rssi_stats(wmi_hdl, data, i,
							  &rssi_stats);
		if (QDF_IS_STATUS_ERROR(status))
			continue;
		ev->vdev_chain_rssi[i].vdev_id = rssi_stats.vdev_id;

		for (j = 0; j < MAX_NUM_CHAINS; j++) {
			dat_snr = rssi_stats.rssi_avg_data[j];
			bcn_snr = rssi_stats.rssi_avg_beacon[j];
			cp_stats_nofl_debug("Chain %d SNR bcn: %d data: %d", j,
					    bcn_snr, dat_snr);
			/*
			 * Get the absolute rssi value from the current rssi
			 * value the snr value is hardcoded into 0 in the
			 * qcacld-new/CORE stack
			 */
			wlan_util_stats_get_rssi(db2dbm_enabled, bcn_snr,
						 dat_snr,
						 &ev->vdev_chain_rssi[i].
						 chain_rssi[j]);
		}
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_cp_stats_extract_event(struct wmi_unified *wmi_hdl,
						   struct stats_event *ev,
						   uint8_t *data)
{
	QDF_STATUS status;
	wmi_host_stats_event stats_param = {0};

	status = wmi_extract_stats_param(wmi_hdl, data, &stats_param);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("stats param extract failed: %d", status);
		return status;
	}
	cp_stats_nofl_debug("num: pdev: %d, pdev_extd: %d, vdev: %d, peer: %d,"
			    "peer_extd: %d rssi: %d, mib %d, mib_extd %d, "
			    "bcnflt: %d, channel: %d, bcn: %d, peer_extd2: %d,"
			    "last_event: %x",
			    stats_param.num_pdev_stats,
			    stats_param.num_pdev_ext_stats,
			    stats_param.num_vdev_stats,
			    stats_param.num_peer_stats,
			    stats_param.num_peer_extd_stats,
			    stats_param.num_rssi_stats,
			    stats_param.num_mib_stats,
			    stats_param.num_mib_extd_stats,
			    stats_param.num_bcnflt_stats,
			    stats_param.num_chan_stats,
			    stats_param.num_bcn_stats,
			    stats_param.num_peer_adv_stats, stats_param.last_event);

	ev->last_event = stats_param.last_event;
	status = target_if_cp_stats_extract_pdev_stats(wmi_hdl, &stats_param,
						       ev, data);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = target_if_cp_stats_extract_peer_stats(wmi_hdl, &stats_param,
						       ev, data);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = target_if_cp_stats_extract_cca_stats(wmi_hdl, &stats_param,
						      ev, data);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = target_if_cp_stats_extract_vdev_summary_stats(wmi_hdl,
							       &stats_param,
							       ev, data);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = target_if_cp_stats_extract_vdev_chain_rssi_stats(wmi_hdl,
								  &stats_param,
								  ev, data);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = target_if_cp_stats_extract_mib_stats(wmi_hdl,
						      &stats_param,
						      ev, data);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_mc_cp_stats_stats_event_handler() - function to handle stats event
 * from firmware.
 * @scn: scn handle
 * @data: data buffer for event
 * @datalen: data length
 *
 * Return: status of operation.
 */
static int target_if_mc_cp_stats_stats_event_handler(ol_scn_t scn,
						     uint8_t *data,
						     uint32_t datalen)
{
	QDF_STATUS status;
	struct stats_event ev = {0};
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_lmac_if_cp_stats_rx_ops *rx_ops;

	if (!scn || !data) {
		cp_stats_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}
	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		cp_stats_err("null psoc");
		return -EINVAL;
	}

	rx_ops = target_if_cp_stats_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->process_stats_event) {
		cp_stats_err("callback not registered");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		cp_stats_err("wmi_handle is null");
		return -EINVAL;
	}

	status = target_if_cp_stats_extract_event(wmi_handle, &ev, data);
	if (QDF_IS_STATUS_ERROR(status)) {
		cp_stats_err("extract event failed");
		goto end;
	}

	status = rx_ops->process_stats_event(psoc, &ev);

end:
	target_if_cp_stats_free_stats_event(&ev);

	return qdf_status_to_os_return(status);
}

static void target_if_cp_stats_inc_wake_lock_stats(uint32_t reason,
					struct wake_lock_stats *stats,
					uint32_t *unspecified_wake_count)
{
	switch (reason) {
	case WOW_REASON_UNSPECIFIED:
		(*unspecified_wake_count)++;
		break;

	case WOW_REASON_ASSOC_REQ_RECV:
		stats->mgmt_assoc++;
		break;

	case WOW_REASON_DISASSOC_RECVD:
		stats->mgmt_disassoc++;
		break;

	case WOW_REASON_ASSOC_RES_RECV:
		stats->mgmt_assoc_resp++;
		break;

	case WOW_REASON_REASSOC_REQ_RECV:
		stats->mgmt_reassoc++;
		break;

	case WOW_REASON_REASSOC_RES_RECV:
		stats->mgmt_reassoc_resp++;
		break;

	case WOW_REASON_AUTH_REQ_RECV:
		stats->mgmt_auth++;
		break;

	case WOW_REASON_DEAUTH_RECVD:
		stats->mgmt_deauth++;
		break;

	case WOW_REASON_ACTION_FRAME_RECV:
		stats->mgmt_action++;
		break;

	case WOW_REASON_NLOD:
		stats->pno_match_wake_up_count++;
		break;

	case WOW_REASON_NLO_SCAN_COMPLETE:
		stats->pno_complete_wake_up_count++;
		break;

	case WOW_REASON_LOW_RSSI:
		stats->low_rssi_wake_up_count++;
		break;

	case WOW_REASON_EXTSCAN:
		stats->gscan_wake_up_count++;
		break;

	case WOW_REASON_RSSI_BREACH_EVENT:
		stats->rssi_breach_wake_up_count++;
		break;

	case WOW_REASON_OEM_RESPONSE_EVENT:
		stats->oem_response_wake_up_count++;
		break;

	case WOW_REASON_11D_SCAN:
		stats->scan_11d++;
		break;

	case WOW_REASON_CHIP_POWER_FAILURE_DETECT:
		stats->pwr_save_fail_detected++;
		break;

	default:
		break;
	}
}

static QDF_STATUS
target_if_cp_stats_register_event_handler(struct wlan_objmgr_psoc *psoc)
{
	int ret_val;
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		cp_stats_err("PSOC is NULL!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		cp_stats_err("wmi_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	ret_val = wmi_unified_register_event_handler(
			wmi_handle,
			wmi_update_stats_event_id,
			target_if_mc_cp_stats_stats_event_handler,
			WMI_RX_WORK_CTX);
	if (ret_val)
		cp_stats_err("Failed to register stats event cb");

	return qdf_status_from_os_return(ret_val);
}

static QDF_STATUS
target_if_cp_stats_unregister_event_handler(struct wlan_objmgr_psoc *psoc)
{
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		cp_stats_err("PSOC is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		cp_stats_err("wmi_handle is null");
		return QDF_STATUS_E_INVAL;
	}
	wmi_unified_unregister_event_handler(wmi_handle,
					     wmi_update_stats_event_id);

	return QDF_STATUS_SUCCESS;
}

static uint32_t get_stats_id(enum stats_req_type type)
{
	switch (type) {
	default:
		break;
	case TYPE_CONNECTION_TX_POWER:
		return WMI_REQUEST_PDEV_STAT;
	case TYPE_PEER_STATS:
		return WMI_REQUEST_PEER_STAT | WMI_REQUEST_PEER_EXTD_STAT;
	case TYPE_STATION_STATS:
		return (WMI_REQUEST_AP_STAT   |
			WMI_REQUEST_PEER_STAT |
			WMI_REQUEST_VDEV_STAT |
			WMI_REQUEST_PDEV_STAT |
			WMI_REQUEST_PEER_EXTD2_STAT |
			WMI_REQUEST_RSSI_PER_CHAIN_STAT);
	case TYPE_MIB_STATS:
		return (WMI_REQUEST_MIB_STAT | WMI_REQUEST_MIB_EXTD_STAT);
	}

	return 0;
}

/**
 * target_if_cp_stats_send_stats_req() - API to send stats request to wmi
 * @psoc: pointer to psoc object
 * @req: pointer to object containing stats request parameters
 *
 * Return: status of operation.
 */
static QDF_STATUS target_if_cp_stats_send_stats_req(
					struct wlan_objmgr_psoc *psoc,
					enum stats_req_type type,
					struct request_info *req)

{
	struct wmi_unified *wmi_handle;
	struct stats_request_params param = {0};

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		cp_stats_err("wmi_handle is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}
	/* refer  (WMI_REQUEST_STATS_CMDID) */
	param.stats_id = get_stats_id(type);
	param.vdev_id = req->vdev_id;
	param.pdev_id = req->pdev_id;

	return wmi_unified_stats_request_send(wmi_handle, req->peer_mac_addr,
					      &param);
}

QDF_STATUS
target_if_cp_stats_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_cp_stats_tx_ops *cp_stats_tx_ops;

	if (!tx_ops) {
		cp_stats_err("lmac tx ops is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	cp_stats_tx_ops = &tx_ops->cp_stats_tx_ops;
	if (!cp_stats_tx_ops) {
		cp_stats_err("lmac tx ops is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	cp_stats_tx_ops->cp_stats_attach =
		target_if_cp_stats_register_event_handler;
	cp_stats_tx_ops->cp_stats_detach =
		target_if_cp_stats_unregister_event_handler;
	cp_stats_tx_ops->inc_wake_lock_stats =
		target_if_cp_stats_inc_wake_lock_stats;
	cp_stats_tx_ops->send_req_stats = target_if_cp_stats_send_stats_req;

	return QDF_STATUS_SUCCESS;
}

