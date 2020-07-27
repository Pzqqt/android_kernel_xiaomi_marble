/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

/**
 * DOC: This file contains definitions for target_if roaming offload.
 */

#include "qdf_types.h"
#include "target_if_cm_roam_offload.h"
#include "target_if.h"
#include "wmi_unified_sta_api.h"
#include "wlan_mlme_dbg.h"
#include "wlan_mlme_api.h"
#include "wlan_crypto_global_api.h"

#if defined(WLAN_FEATURE_ROAM_OFFLOAD) || defined(ROAM_OFFLOAD_V1)
static struct wmi_unified
*target_if_cm_roam_get_wmi_handle_from_vdev(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev *pdev;
	struct wmi_unified *wmi_handle;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		target_if_err("PDEV is NULL");
		return NULL;
	}

	wmi_handle = get_wmi_unified_hdl_from_pdev(pdev);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return NULL;
	}

	return wmi_handle;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * target_if_cm_roam_send_vdev_set_pcl_cmd  - Send set vdev pcl
 * command to wmi.
 * @vdev: VDEV object pointer
 * @req:  Pointer to the pcl request msg
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_cm_roam_send_vdev_set_pcl_cmd(struct wlan_objmgr_vdev *vdev,
					struct set_pcl_req *req)
{
	wmi_unified_t wmi_handle;
	struct set_pcl_cmd_params params;

	wmi_handle = target_if_cm_roam_get_wmi_handle_from_vdev(vdev);
	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	params.weights = &req->chan_weights;
	params.vdev_id = req->vdev_id;

	return wmi_unified_vdev_set_pcl_cmd(wmi_handle, &params);
}

static void
target_if_cm_roam_register_lfr3_ops(struct wlan_cm_roam_tx_ops *tx_ops)
{
	tx_ops->send_vdev_set_pcl_cmd = target_if_cm_roam_send_vdev_set_pcl_cmd;
}
#else
static inline void
target_if_cm_roam_register_lfr3_ops(struct wlan_cm_roam_tx_ops *tx_ops)
{}
#endif

#ifdef ROAM_OFFLOAD_V1
/**
 * target_if_is_vdev_valid - vdev id is valid or not
 * @vdev_id: vdev id
 *
 * Return: true or false
 */
static bool target_if_is_vdev_valid(uint8_t vdev_id)
{
	return (vdev_id < WLAN_MAX_VDEVS ? true : false);
}

/**
 * target_if_vdev_set_param() - set per vdev params in fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @param_id: parameter id
 * @param_value: parameter value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
target_if_vdev_set_param(wmi_unified_t wmi_handle, uint32_t vdev_id,
			 uint32_t param_id, uint32_t param_value)
{
	struct vdev_set_params param = {0};

	if (!target_if_is_vdev_valid(vdev_id)) {
		target_if_err("vdev_id: %d is invalid, reject the req: param id %d val %d",
			      vdev_id, param_id, param_value);
		return QDF_STATUS_E_INVAL;
	}

	param.vdev_id = vdev_id;
	param.param_id = param_id;
	param.param_value = param_value;

	return wmi_unified_vdev_set_param_send(wmi_handle, &param);
}

/**
 * target_if_cm_roam_scan_bmiss_cnt() - set bmiss count to fw
 * @wmi_handle: wmi handle
 * @req: bmiss count parameters
 *
 * Set first & final bmiss count to fw.
 *
 * Return: QDF status
 */
static QDF_STATUS
target_if_cm_roam_scan_bmiss_cnt(wmi_unified_t wmi_handle,
				 struct wlan_roam_beacon_miss_cnt *req)
{
	QDF_STATUS status;
	uint32_t vdev_id;
	uint8_t first_bcnt;
	uint8_t final_bcnt;

	vdev_id = req->vdev_id;
	first_bcnt = req->roam_bmiss_first_bcnt;
	final_bcnt = req->roam_bmiss_final_bcnt;

	target_if_debug("first_bcnt: %d, final_bcnt: %d",
			first_bcnt, final_bcnt);

	status = target_if_vdev_set_param(wmi_handle, vdev_id,
					  WMI_VDEV_PARAM_BMISS_FIRST_BCNT,
					  first_bcnt);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("vdev set WMI_VDEV_PARAM_BMISS_FIRST_BCNT params returned error %d",
			      status);
		return status;
	}

	status = target_if_vdev_set_param(wmi_handle, vdev_id,
					  WMI_VDEV_PARAM_BMISS_FINAL_BCNT,
					  final_bcnt);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("vdev set WMI_VDEV_PARAM_BMISS_FINAL_BCNT params returned error %d",
			      status);
		return status;
	}

	return status;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/* target_if_cm_roam_reason_vsie(): set vdev param
 * WMI_VDEV_PARAM_ENABLE_DISABLE_ROAM_REASON_VSIE
 * @wmi_handle: handle to WMI
 * @req: roam reason vsie enable parameters
 *
 * Return: void
 */
static void
target_if_cm_roam_reason_vsie(wmi_unified_t wmi_handle,
			      struct wlan_roam_reason_vsie_enable *req)
{
	QDF_STATUS status;

	status = target_if_vdev_set_param(
				wmi_handle,
				req->vdev_id,
				WMI_VDEV_PARAM_ENABLE_DISABLE_ROAM_REASON_VSIE,
				req->enable_roam_reason_vsie);

	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("Failed to set vdev param %d",
			      WMI_VDEV_PARAM_ENABLE_DISABLE_ROAM_REASON_VSIE);
}

/* target_if_cm_roam_triggers(): send roam triggers to WMI
 * @wmi_handle: handle to WMI
 * @req: roam triggers parameters
 *
 * Return: QDF status
 */
static QDF_STATUS
target_if_cm_roam_triggers(wmi_unified_t wmi_handle,
			   struct wlan_roam_triggers *req)
{
	if (!target_if_is_vdev_valid(req->vdev_id))
		return QDF_STATUS_E_INVAL;

	return wmi_unified_set_roam_triggers(wmi_handle, req);
}

/**
 * target_if_cm_roam_scan_get_cckm_mode() - Get the CCKM auth mode
 * @vdev: vdev object
 * @auth_mode: Auth mode to be converted
 *
 * Based on LFR2.0 or LFR3.0, return the proper auth type
 *
 * Return: if LFR2.0, then return WMI_AUTH_CCKM for backward compatibility
 *         if LFR3.0 then return the appropriate auth type
 */
static uint32_t
target_if_cm_roam_scan_get_cckm_mode(struct wlan_objmgr_vdev *vdev,
				     uint32_t auth_mode)
{
	struct wlan_objmgr_psoc *psoc;
	bool roam_offload_enable;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		target_if_err("psoc handle is NULL");
		return WMI_AUTH_CCKM;
	}

	wlan_mlme_get_roaming_offload(psoc, &roam_offload_enable);
	if (roam_offload_enable)
		return auth_mode;
	else
		return WMI_AUTH_CCKM;
}
#else
static void
target_if_cm_roam_reason_vsie(wmi_unified_t wmi_handle,
			      struct wlan_roam_reason_vsie_enable *req)
{
}

static QDF_STATUS
target_if_cm_roam_triggers(wmi_unified_t wmi_handle,
			   struct wlan_roam_triggers *req)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static uint32_t
target_if_cm_roam_scan_get_cckm_mode(struct wlan_objmgr_vdev *vdev,
				     uint32_t auth_mode)
{
	return WMI_AUTH_CCKM;
}
#endif
/**
 * target_if_cm_roam_scan_offload_rssi_thresh() - Send roam scan rssi threshold
 * commands to wmi
 * @wmi_handle: wmi handle
 * @req: roam scan rssi threshold related parameters
 *
 * This function fills some parameters @req and send down roam scan rssi
 * threshold command to wmi
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_cm_roam_scan_offload_rssi_thresh(
				wmi_unified_t wmi_handle,
				struct wlan_roam_offload_scan_rssi_params *req)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool db2dbm_enabled;

	db2dbm_enabled = wmi_service_enabled(wmi_handle,
					     wmi_service_hw_db2dbm_support);
	if (!db2dbm_enabled) {
		req->rssi_thresh -= NOISE_FLOOR_DBM_DEFAULT;
		req->rssi_thresh &= 0x000000ff;
		req->hi_rssi_scan_rssi_ub -= NOISE_FLOOR_DBM_DEFAULT;
		req->bg_scan_bad_rssi_thresh -= NOISE_FLOOR_DBM_DEFAULT;
		req->good_rssi_threshold -= NOISE_FLOOR_DBM_DEFAULT;
		req->good_rssi_threshold &= 0x000000ff;
	}

	req->hi_rssi_scan_rssi_ub &= 0x000000ff;
	/*
	 * The current Noise floor in firmware is -96dBm. Penalty/Boost
	 * threshold is applied on a weaker signal to make it even more weaker.
	 * So, there is a chance that the user may configure a very low
	 * Penalty/Boost threshold beyond the noise floor. If that is the case,
	 * then suppress the penalty/boost threshold to the noise floor.
	 */
	if (req->raise_rssi_thresh_5g < NOISE_FLOOR_DBM_DEFAULT) {
		if (db2dbm_enabled) {
			req->penalty_threshold_5g = RSSI_MIN_VALUE;
			req->boost_threshold_5g = RSSI_MAX_VALUE;
		} else {
			req->penalty_threshold_5g = 0;
		}
	} else {
		if (db2dbm_enabled) {
			req->boost_threshold_5g = req->raise_rssi_thresh_5g;
		} else {
			req->boost_threshold_5g =
				(req->raise_rssi_thresh_5g -
					NOISE_FLOOR_DBM_DEFAULT) & 0x000000ff;
		}
	}

	if (req->drop_rssi_thresh_5g < NOISE_FLOOR_DBM_DEFAULT) {
		if (db2dbm_enabled)
			req->penalty_threshold_5g = RSSI_MIN_VALUE;
		else
			req->penalty_threshold_5g = 0;
	} else {
		if (db2dbm_enabled) {
			req->penalty_threshold_5g = req->drop_rssi_thresh_5g;
		} else {
			req->penalty_threshold_5g =
				(req->drop_rssi_thresh_5g -
					NOISE_FLOOR_DBM_DEFAULT) & 0x000000ff;
		}
	}

	if (req->early_stop_scan_enable) {
		if (!db2dbm_enabled) {
			req->roam_earlystop_thres_min -=
						NOISE_FLOOR_DBM_DEFAULT;
			req->roam_earlystop_thres_max -=
						NOISE_FLOOR_DBM_DEFAULT;
		}
	} else {
		if (db2dbm_enabled) {
			req->roam_earlystop_thres_min = RSSI_MIN_VALUE;
			req->roam_earlystop_thres_max = RSSI_MIN_VALUE;
		} else {
			req->roam_earlystop_thres_min = 0;
			req->roam_earlystop_thres_max = 0;
		}
	}

	target_if_debug("good_rssi_threshold %d, early_stop_thresholds en=%d, min=%d, max=%d roam_scan_rssi_thresh=%d, roam_rssi_thresh_diff=%d",
			req->good_rssi_threshold, req->early_stop_scan_enable,
			req->roam_earlystop_thres_min,
			req->roam_earlystop_thres_max, req->rssi_thresh,
			req->rssi_thresh_diff);
	target_if_debug("hirssi max cnt %d, delta %d, hirssi upper bound %d dense rssi thresh offset %d, dense min aps cnt %d, traffic_threshold %d dense_status=%d",
			req->hi_rssi_scan_max_count,
			req->hi_rssi_scan_rssi_delta,
			req->hi_rssi_scan_rssi_ub,
			req->dense_rssi_thresh_offset,
			req->dense_min_aps_cnt,
			req->traffic_threshold,
			req->initial_dense_status);
	target_if_debug("BG Scan Bad RSSI:%d, bitmap:0x%x Offset for 2G to 5G Roam:%d",
			req->bg_scan_bad_rssi_thresh,
			req->bg_scan_client_bitmap,
			req->roam_bad_rssi_thresh_offset_2g);

	status = wmi_unified_roam_scan_offload_rssi_thresh_cmd(wmi_handle, req);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("roam_scan_offload_rssi_thresh_cmd failed %d",
			      status);
		return status;
	}

	return status;
}

/**
 * target_if_roam_scan_offload_scan_period() - set roam offload scan period
 * @wmi_handle: wmi handle
 * @req:  roam scan period parameters
 *
 * Send WMI_ROAM_SCAN_PERIOD parameters to fw.
 *
 * Return: QDF status
 */
static QDF_STATUS
target_if_cm_roam_scan_offload_scan_period(
				wmi_unified_t wmi_handle,
				struct wlan_roam_scan_period_params *req)
{
	if (!target_if_is_vdev_valid(req->vdev_id)) {
		target_if_err("Invalid vdev id:%d", req->vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_roam_scan_offload_scan_period(wmi_handle, req);
}

#ifdef WLAN_FEATURE_11W
/**
 * target_if_roam_fill_11w_params() - Fill the 11w related parameters
 * for ap profile
 * @vdev: vdev object
 * @req: roam ap profile parameters
 *
 * Return: None
 */
static void
target_if_cm_roam_fill_11w_params(struct wlan_objmgr_vdev *vdev,
				  struct ap_profile_params *req)
{
	uint32_t group_mgmt_cipher;
	uint16_t rsn_caps;
	bool peer_rmf_capable = false;
	uint32_t keymgmt;

	if (!vdev) {
		target_if_err("Invalid vdev");
		return;
	}

	rsn_caps = (uint16_t)wlan_crypto_get_param(vdev,
						   WLAN_CRYPTO_PARAM_RSN_CAP);
	if (wlan_crypto_vdev_has_mgmtcipher(
					vdev,
					(1 << WLAN_CRYPTO_CIPHER_AES_GMAC) |
					(1 << WLAN_CRYPTO_CIPHER_AES_GMAC_256) |
					(1 << WLAN_CRYPTO_CIPHER_AES_CMAC)) &&
					(rsn_caps &
					 WLAN_CRYPTO_RSN_CAP_MFP_ENABLED))
		peer_rmf_capable = true;

	keymgmt = wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_MGMT_CIPHER);

	if (keymgmt & (1 << WLAN_CRYPTO_CIPHER_AES_CMAC))
		group_mgmt_cipher = WMI_CIPHER_AES_CMAC;
	else if (keymgmt & (1 << WLAN_CRYPTO_CIPHER_AES_GMAC))
		group_mgmt_cipher = WMI_CIPHER_AES_GMAC;
	else if (keymgmt & (1 << WLAN_CRYPTO_CIPHER_AES_GMAC_256))
		group_mgmt_cipher = WMI_CIPHER_BIP_GMAC_256;
	 else
		group_mgmt_cipher = WMI_CIPHER_NONE;

	if (peer_rmf_capable) {
		req->profile.rsn_mcastmgmtcipherset = group_mgmt_cipher;
		req->profile.flags |= WMI_AP_PROFILE_FLAG_PMF;
	} else {
		req->profile.rsn_mcastmgmtcipherset = WMI_CIPHER_NONE;
	}
}
#else
static inline
void target_if_cm_roam_fill_11w_params(struct wlan_objmgr_vdev *vdev,
				       struct ap_profile_params *req)
{}
#endif

/**
 * target_if_cm_roam_scan_offload_ap_profile() - send roam ap profile to
 * firmware
 * @vdev: vdev object
 * @wmi_handle: wmi handle
 * @req: roam ap profile parameters
 *
 * Send WMI_ROAM_AP_PROFILE parameters to firmware
 *
 * Return: QDF status
 */
static QDF_STATUS
target_if_cm_roam_scan_offload_ap_profile(
				struct wlan_objmgr_vdev *vdev,
				wmi_unified_t wmi_handle,
				struct ap_profile_params *req)
{
	uint32_t rsn_authmode;
	bool db2dbm_enabled;

	if (!target_if_is_vdev_valid(req->vdev_id)) {
		target_if_err("Invalid vdev id:%d", req->vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	rsn_authmode = req->profile.rsn_authmode;
	if (rsn_authmode == WMI_AUTH_CCKM_WPA ||
	    rsn_authmode == WMI_AUTH_CCKM_RSNA)
		req->profile.rsn_authmode =
		target_if_cm_roam_scan_get_cckm_mode(vdev, rsn_authmode);

	target_if_cm_roam_fill_11w_params(vdev, req);

	db2dbm_enabled = wmi_service_enabled(wmi_handle,
					     wmi_service_hw_db2dbm_support);
	if (!req->profile.rssi_abs_thresh) {
		if (db2dbm_enabled)
			req->profile.rssi_abs_thresh = RSSI_MIN_VALUE;
	} else {
		if (!db2dbm_enabled)
			req->profile.rssi_abs_thresh -=
						NOISE_FLOOR_DBM_DEFAULT;
	}

	if (!db2dbm_enabled) {
		req->min_rssi_params[DEAUTH_MIN_RSSI].min_rssi -=
						NOISE_FLOOR_DBM_DEFAULT;
		req->min_rssi_params[DEAUTH_MIN_RSSI].min_rssi &= 0x000000ff;

		req->min_rssi_params[BMISS_MIN_RSSI].min_rssi -=
						NOISE_FLOOR_DBM_DEFAULT;
		req->min_rssi_params[BMISS_MIN_RSSI].min_rssi &= 0x000000ff;
	}

	return wmi_unified_send_roam_scan_offload_ap_cmd(wmi_handle, req);
}

/**
 * target_if_cm_roam_scan_filter() - send roam scan filter to firmware
 * @wmi_handle: wmi handle
 * @command: rso command
 * @req: roam scan filter parameters
 *
 * Send WMI_ROAM_FILTER_CMDID parameters to firmware
 *
 * Return: QDF status
 */
static QDF_STATUS
target_if_cm_roam_scan_filter(wmi_unified_t wmi_handle, uint8_t command,
			      struct wlan_roam_scan_filter_params *req)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!target_if_is_vdev_valid(req->filter_params.vdev_id)) {
		target_if_err("Invalid vdev id:%d",
			      req->filter_params.vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	if (command != ROAM_SCAN_OFFLOAD_STOP) {
		switch (req->reason) {
		case REASON_ROAM_SET_BLACKLIST_BSSID:
		case REASON_ROAM_SET_SSID_ALLOWED:
		case REASON_ROAM_SET_FAVORED_BSSID:
			break;
		case REASON_CTX_INIT:
			if (command == ROAM_SCAN_OFFLOAD_START) {
				req->filter_params.op_bitmap |=
				ROAM_FILTER_OP_BITMAP_LCA_DISALLOW |
				ROAM_FILTER_OP_BITMAP_RSSI_REJECTION_OCE;
			} else {
				target_if_debug("Roam Filter need not be sent");
				return QDF_STATUS_SUCCESS;
			}
			break;
		default:
			target_if_debug("Roam Filter need not be sent");
			return QDF_STATUS_SUCCESS;
		}
	}

	status = wmi_unified_roam_scan_filter_cmd(wmi_handle,
						  &req->filter_params);
	return status;
}

/**
 * target_if_cm_roam_send_roam_start() - Send roam start related commands
 * to wmi
 * @vdev: vdev object
 * @req: roam start config parameters
 *
 * This function is used to Send roam start related commands to wmi
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
target_if_cm_roam_send_roam_start(struct wlan_objmgr_vdev *vdev,
				  struct wlan_roam_start_config *req)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	wmi_unified_t wmi_handle;

	wmi_handle = target_if_cm_roam_get_wmi_handle_from_vdev(vdev);
	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	status = target_if_cm_roam_scan_offload_rssi_thresh(
							wmi_handle,
							&req->rssi_params);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Sending roam scan offload rssi thresh failed");
		goto end;
	}

	status = target_if_cm_roam_scan_bmiss_cnt(wmi_handle,
						  &req->beacon_miss_cnt);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("vdev set bmiss bcnt param failed");
		goto end;
	}

	target_if_cm_roam_reason_vsie(wmi_handle, &req->reason_vsie_enable);

	target_if_cm_roam_triggers(wmi_handle, &req->roam_triggers);

	/* Opportunistic scan runs on a timer, value set by
	 * empty_scan_refresh_period. Age out the entries after 3 such
	 * cycles.
	 */
	if (req->scan_period_params.empty_scan_refresh_period > 0) {
		status = target_if_cm_roam_scan_offload_scan_period(
						wmi_handle,
						&req->scan_period_params);
		if (QDF_IS_STATUS_ERROR(status))
			goto end;
	}

	status = target_if_cm_roam_scan_offload_ap_profile(
							vdev, wmi_handle,
							&req->profile_params);
	if (QDF_IS_STATUS_ERROR(status))
		goto end;

	status = target_if_cm_roam_scan_filter(wmi_handle,
					       ROAM_SCAN_OFFLOAD_START,
					       &req->scan_filter_params);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Sending start for roam scan filter failed");
		goto end;
	}

	/* add other wmi commands */
end:
	return status;
}

/**
 * target_if_cm_roam_register_rso_req_ops() - Register rso req tx ops fucntions
 * @tx_ops: tx ops
 *
 * This function is used to register rso req tx ops fucntions
 *
 * Return: none
 */
static void
target_if_cm_roam_register_rso_req_ops(struct wlan_cm_roam_tx_ops *tx_ops)
{
	tx_ops->send_roam_start_req = target_if_cm_roam_send_roam_start;
}
#else
static void
target_if_cm_roam_register_rso_req_ops(struct wlan_cm_roam_tx_ops *tx_ops)
{
}
#endif

QDF_STATUS target_if_cm_roam_register_tx_ops(struct wlan_cm_roam_tx_ops *tx_ops)
{
	if (!tx_ops) {
		target_if_err("target if tx ops is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	target_if_cm_roam_register_lfr3_ops(tx_ops);
	target_if_cm_roam_register_rso_req_ops(tx_ops);

	return QDF_STATUS_SUCCESS;
}
