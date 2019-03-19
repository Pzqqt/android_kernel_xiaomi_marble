/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

/**
 * wlan_util_vdev_get_cdp_txrx_opmode - get cdp txrx opmode from qdf mode
 * @vdev: pointer to vdev object
 *
 * Return: wlan_opmode
 */
enum wlan_op_mode
wlan_util_vdev_get_cdp_txrx_opmode(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_util_vdev_mlme_set_ratemask_config) – common MLME API to set
 * ratemask configuration and send it to FW
 * @vdev_mlme: pointer to vdev_mlme object
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS
wlan_util_vdev_mlme_set_ratemask_config(struct vdev_mlme_obj *vdev_mlme);

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

#endif /* __WLAN_VDEV_MGR_UTILS_API_H__ */
