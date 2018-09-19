/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

/**
 * struct mlme_legacy_priv - VDEV MLME legacy priv object
 * @chan_switch_in_progress: flag to indicate that channel switch is in progress
 * @hidden_ssid_restart_in_progress: flag to indicate hidden ssid restart is
 *                                   in progress
 * @vdev_start_failed: flag to indicate that vdev start failed.
 */
struct mlme_legacy_priv {
	bool chan_switch_in_progress;
	bool hidden_ssid_restart_in_progress;
	bool vdev_start_failed;
};

/**
 * mlme_register_vdev_mgr_ops() - Register vdev mgr ops
 * @vdev_mlme: vdev mlme object
 *
 * This function is called to register vdev manager operations
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_register_vdev_mgr_ops(void *mlme);
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
 * mlme_get_vdev_start_failed() - get mlme priv vdev restart fail flag
 * @vdev: vdev pointer
 *
 * Return: value of mlme priv vdev restart fail flag
 */
bool mlme_get_vdev_start_failed(struct wlan_objmgr_vdev *vdev);

#endif
#endif
