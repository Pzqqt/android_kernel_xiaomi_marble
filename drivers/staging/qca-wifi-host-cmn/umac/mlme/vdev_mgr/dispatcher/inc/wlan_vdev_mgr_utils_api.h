/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_vdev_mgr_utils_api.h
 *
 * This file provides declaration for APIs used for psoc enable/disable
 */

#ifndef __WLAN_VDEV_MGR_UTILS_API_H__
#define __WLAN_VDEV_MGR_UTILS_API_H__

#include <wlan_objmgr_psoc_obj.h>
#include <include/wlan_vdev_mlme.h>
#include <wlan_vdev_mgr_ucfg_api.h>
#include <cdp_txrx_cmn_struct.h>

/* The total time required to receive CSA event handler from FW with CSA count
 * 0, plus, time required to process the CSA event, plus, time required to
 * send multi-vdev restart request on the new channel and send updated beacon
 * template is approximately 1 second (considered 16 AP vaps).
 */
#define VDEV_RESTART_TIME 1

/* Convert seconds to milliseconds */
#define SECONDS_TO_MS(seconds) ((seconds) * 1000)

/**
 * wlan_util_vdev_get_cdp_txrx_opmode - get cdp txrx opmode from qdf mode
 * @vdev: pointer to vdev object
 *
 * Return: wlan_opmode
 */
enum wlan_op_mode
wlan_util_vdev_get_cdp_txrx_opmode(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_util_vdev_get_cdp_txrx_subtype - get cdp txrx subtype from qdf mode
 * @vdev: pointer to vdev object
 *
 * Return: wlan_opmode
 */
enum wlan_op_subtype
wlan_util_vdev_get_cdp_txrx_subtype(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_util_vdev_mlme_set_ratemask_config) – common MLME API to set
 * ratemask configuration and send it to FW
 * @vdev_mlme: pointer to vdev_mlme object
 * @index: ratemask array index
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS
wlan_util_vdev_mlme_set_ratemask_config(struct vdev_mlme_obj *vdev_mlme,
					uint8_t index);

/**
 * wlan_util_vdev_mlme_set_param() – common MLME API to fill common
 * parameters of vdev_mlme object
 * @vdev_mlme: pointer to vdev_mlme object
 * @param_id: param id for which the value should be set
 * @param_value: value that should bem set to the parameter
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_util_vdev_mlme_set_param(struct vdev_mlme_obj *vdev_mlme,
					 enum wlan_mlme_cfg_id param_id,
					 struct wlan_vdev_mgr_cfg mlme_cfg);

/**
 * wlan_util_vdev_mlme_get_param() – common MLME API to get common
 * parameters of vdev_mlme object
 * @vdev_mlme: pointer to vdev_mlme object
 * @param_id: param id for which the value should be set
 * @param_value: value that should bem set to the parameter
 *
 * Return: QDF_STATUS - Success or Failure
 */
void wlan_util_vdev_mlme_get_param(struct vdev_mlme_obj *vdev_mlme,
				   enum wlan_mlme_cfg_id param_id,
				   uint32_t *param_value);

/**
 * wlan_util_vdev_get_param() – common MLME API to get common
 * parameters of vdev_mlme object
 * @vdev: pointer to vdev object
 * @param_id: param id for which the value should be set
 * @param_value: value that should bem set to the parameter
 *
 * Return: QDF_STATUS - Success or Failure
 */
void wlan_util_vdev_get_param(struct wlan_objmgr_vdev *vdev,
			      enum wlan_mlme_cfg_id param_id,
			      uint32_t *param_value);

/**
 * wlan_util_vdev_mgr_get_csa_channel_switch_time() - Returns the time required
 * to switch the channel after completing the CSA announcement. This does not
 * include the CAC duration.
 * @vdev: Pointer to vdev object
 * @chan_switch_time: Pointer to save the CSA channel switch time. This does not
 *                    include the DFS CAC duration
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_util_vdev_mgr_get_csa_channel_switch_time(
		struct wlan_objmgr_vdev *vdev,
		uint32_t *chan_switch_time);

/**
 * wlan_util_vdev_mgr_compute_max_channel_switch_time() - Compute the max
 * channel switch time for the given vdev
 * @vdev: pointer to vdev object
 * @max_chan_switch_time: Pointer to save the computed max channel switch time
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_util_vdev_mgr_compute_max_channel_switch_time(
		struct wlan_objmgr_vdev *vdev, uint32_t *max_chan_switch_time);

/**
 * wlan_utils_get_vdev_remaining_channel_switch_time() - Get the remaining
 *                                                       channel switch time.
 * @vdev: Pointer to vdev object
 *
 * Remaining channel switch time is equal to the time when last beacon sent on
 * the CSA triggered vap plus max channel switch time minus current
 * time.
 *
 * Return: Remaining cac time
 */
uint32_t wlan_utils_get_vdev_remaining_channel_switch_time(
		struct wlan_objmgr_vdev *vdev);

/**
 * wlan_util_vdev_mgr_get_cac_timeout_for_vdev() - Get the CAC timeout value for
 * a given vdev.
 * @vdev: Pointer to vdev object.
 *
 * Return: CAC timeout value
 */
int wlan_util_vdev_mgr_get_cac_timeout_for_vdev(struct wlan_objmgr_vdev *vdev);

#ifdef MOBILE_DFS_SUPPORT
/**
 * wlan_util_vdev_mgr_set_cac_timeout_for_vdev() - set the CAC timeout value for
 * a given vdev.
 * @vdev: Pointer to vdev object.
 * @new_chan_cac_ms: cac duration of new channel
 *
 * Return: void
 */
void wlan_util_vdev_mgr_set_cac_timeout_for_vdev(struct wlan_objmgr_vdev *vdev,
						 uint32_t new_chan_cac_ms);
#endif /* MOBILE_DFS_SUPPORT */

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * wlan_util_vdev_mgr_quiet_offload() - set quiet status for given link
 * @psoc: pointer to psoc
 * @quiet_event: pointer to struct vdev_sta_quiet_event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_util_vdev_mgr_quiet_offload(
				struct wlan_objmgr_psoc *psoc,
				struct vdev_sta_quiet_event *quiet_event);
#endif /* WLAN_FEATURE_11BE_MLO */
#endif /* __WLAN_VDEV_MGR_UTILS_API_H__ */
