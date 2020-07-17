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
	wmi_unified_t wmi_handle;

	wmi_handle = target_if_cm_roam_get_wmi_handle_from_vdev(vdev);
	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	target_if_cm_roam_scan_offload_rssi_thresh(wmi_handle,
						   &req->rssi_params);
	/* add other wmi commands */

	return QDF_STATUS_SUCCESS;
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
