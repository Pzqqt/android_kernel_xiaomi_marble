/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: declare VDEV Manager interface APIs exposed by the mlme component
 */

#ifndef _WLAN_MLME_VDEV_MGR_INT_API_H_
#define _WLAN_MLME_VDEV_MGR_INT_API_H_

#ifdef CONFIG_VDEV_SM
#include <wlan_objmgr_vdev_obj.h>
#include "include/wlan_vdev_mlme.h"
#include "wlan_mlme_main.h"

/**
 * mlme_register_mlme_ext_ops() - Register mlme ext ops
 *
 * This function is called to register mlme ext operations
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_register_mlme_ext_ops(void);
/**
 * mlme_register_vdev_mgr_ops() - Register vdev mgr ops
 * @vdev_mlme: vdev mlme object
 *
 * This function is called to register vdev manager operations
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_register_vdev_mgr_ops(struct vdev_mlme_obj *vdev_mlme);
/**
 * mlme_unregister_vdev_mgr_ops() - Unregister vdev mgr ops
 * @vdev_mlme: vdev mlme object
 *
 * This function is called to unregister vdev manager operations
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_unregister_vdev_mgr_ops(struct vdev_mlme_obj *vdev_mlme);

/**
 * mlme_set_chan_switch_in_progress() - set mlme priv restart in progress
 * @vdev: vdev pointer
 * @val: value to be set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_set_chan_switch_in_progress(struct wlan_objmgr_vdev *vdev,
					       bool val);

/**
 * mlme_is_chan_switch_in_progress() - get mlme priv restart in progress
 * @vdev: vdev pointer
 *
 * Return: value of mlme priv restart in progress
 */
bool mlme_is_chan_switch_in_progress(struct wlan_objmgr_vdev *vdev);

/**
 * ap_mlme_set_hidden_ssid_restart_in_progress() - set mlme priv hidden ssid
 * restart in progress
 * @vdev: vdev pointer
 * @val: value to be set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ap_mlme_set_hidden_ssid_restart_in_progress(struct wlan_objmgr_vdev *vdev,
					    bool val);

/**
 * ap_mlme_is_hidden_ssid_restart_in_progress() - get mlme priv hidden ssid
 * restart in progress
 * @vdev: vdev pointer
 *
 * Return: value of mlme priv hidden ssid restart in progress
 */
bool ap_mlme_is_hidden_ssid_restart_in_progress(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_set_vdev_start_failed() - set mlme priv vdev restart fail flag
 * @vdev: vdev pointer
 * @val: value to be set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_set_vdev_start_failed(struct wlan_objmgr_vdev *vdev, bool val);

/**
 * mlme_is_connection_fail() - get connection fail flag
 * @vdev: vdev pointer
 *
 * Return: value of vdev connection failure flag
 */
bool mlme_is_connection_fail(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_set_connection_fail() - set connection failure flag
 * @vdev: vdev pointer
 * @val: value to be set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_set_connection_fail(struct wlan_objmgr_vdev *vdev, bool val);

/**
 * mlme_get_vdev_start_failed() - get mlme priv vdev restart fail flag
 * @vdev: vdev pointer
 *
 * Return: value of mlme priv vdev restart fail flag
 */
bool mlme_get_vdev_start_failed(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_get_cac_required() - get if cac is required for new channel
 * @vdev: vdev pointer
 *
 * Return: if cac is required
 */
bool mlme_get_cac_required(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_set_cac_required() - set if cac is required for new channel
 * @vdev: vdev pointer
 * @val: value to be set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_set_cac_required(struct wlan_objmgr_vdev *vdev, bool val);

/**
 * mlme_set_mbssid_info() - save mbssid info
 * @vdev: vdev pointer
 * @mbssid_info: mbssid info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mlme_set_mbssid_info(struct wlan_objmgr_vdev *vdev,
		     struct scan_mbssid_info *mbssid_info);

/**
 * mlme_get_mbssid_info() - get mbssid info
 * @vdev: vdev pointer
 * @mbss_11ax: mbss 11ax info
 *
 * Return: None
 */
void mlme_get_mbssid_info(struct wlan_objmgr_vdev *vdev,
			  struct vdev_mlme_mbss_11ax *mbss_11ax);

/**
 * mlme_is_vdev_in_beaconning_mode() - check if vdev is beaconing mode
 * @vdev_opmode: vdev opmode
 *
 * To check if vdev is operating in beaconing mode or not.
 *
 * Return: true or false
 */
bool mlme_is_vdev_in_beaconning_mode(enum QDF_OPMODE vdev_opmode);

/**
 * mlme_set_assoc_type() - set associate type
 * @vdev: vdev pointer
 * @assoc_type: type to be set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_set_assoc_type(struct wlan_objmgr_vdev *vdev,
			       enum vdev_assoc_type assoc_type);

/**
 * mlme_get_assoc_type() - get associate type
 * @vdev: vdev pointer
 *
 * Return: associate type
 */
enum vdev_assoc_type  mlme_get_assoc_type(struct wlan_objmgr_vdev *vdev);
#endif
#endif
