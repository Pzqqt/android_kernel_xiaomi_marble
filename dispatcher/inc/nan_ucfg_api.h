/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: contains interface prototypes for OS_IF layer
 */

#ifndef _NAN_UCFG_API_H_
#define _NAN_UCFG_API_H_

#include "qdf_types.h"
#include "qdf_status.h"

struct nan_callbacks;
struct wlan_objmgr_vdev;
struct wlan_objmgr_psoc;

/**
 * ucfg_nan_set_ndi_state: set ndi state
 * @vdev: pointer to vdev object
 * @state: value to set
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_nan_set_ndi_state(struct wlan_objmgr_vdev *vdev,
				  uint32_t state);

/**
 * ucfg_nan_get_ndi_state: get ndi state from vdev obj
 * @vdev: pointer to vdev object
 *
 * Return: ndi state
 */
enum nan_datapath_state ucfg_nan_get_ndi_state(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_nan_set_active_peers: set active ndi peer
 * @vdev: pointer to vdev object
 * @val: value to set
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_nan_set_active_peers(struct wlan_objmgr_vdev *vdev,
				     uint32_t val);

/**
 * ucfg_nan_get_active_peers: get active ndi peer from vdev obj
 * @vdev: pointer to vdev object
 *
 * Return: active ndi peer
 */
uint32_t ucfg_nan_get_active_peers(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_nan_set_active_ndp_sessions: set active ndp sessions
 * @vdev: pointer to vdev object
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_nan_set_active_ndp_sessions(struct wlan_objmgr_vdev *vdev,
					    uint32_t val, uint8_t idx);

/**
 * ucfg_nan_get_active_ndp_sessions: get active ndp sessions from vdev obj
 * @vdev: pointer to vdev object
 *
 * Return: pointer to NAN psoc private object
 */
uint32_t ucfg_nan_get_active_ndp_sessions(struct wlan_objmgr_vdev *vdev,
					  uint8_t idx);

/**
 * ucfg_nan_set_ndp_create_transaction_id: set ndp create transaction id
 * @vdev: pointer to vdev object
 * @val: value to set
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_nan_set_ndp_create_transaction_id(struct wlan_objmgr_vdev *vdev,
						  uint16_t val);

/**
 * ucfg_nan_get_ndp_create_transaction_id: get ndp create transaction id
 * vdev obj
 * @vdev: pointer to vdev object
 *
 * Return: ndp create transaction_id
 */
uint16_t ucfg_nan_get_ndp_create_transaction_id(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_nan_set_ndp_delete_transaction_id: set ndp delete transaction id
 * @vdev: pointer to vdev object
 * @val: value to set
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_nan_set_ndp_delete_transaction_id(struct wlan_objmgr_vdev *vdev,
						  uint16_t val);

/**
 * ucfg_nan_get_ndp_delete_transaction_id: get ndp delete transaction id from
 * vdev obj
 * @vdev: pointer to vdev object
 *
 * Return: ndp delete transaction_id
 */
uint16_t ucfg_nan_get_ndp_delete_transaction_id(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_nan_set_ndi_delete_rsp_reason: set ndi delete response reason
 * @vdev: pointer to vdev object
 * @val: value to set
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_nan_set_ndi_delete_rsp_reason(struct wlan_objmgr_vdev *vdev,
					      uint32_t val);

/**
 * ucfg_nan_get_ndi_delete_rsp_reason: get ndi delete response reason from vdev
 * obj
 * @vdev: pointer to vdev object
 *
 * Return: ndi delete rsp reason
 */
uint32_t ucfg_nan_get_ndi_delete_rsp_reason(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_nan_set_ndi_delete_rsp_status: set ndi delete response reason
 * @vdev: pointer to vdev object
 * @val: value to set
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_nan_set_ndi_delete_rsp_status(struct wlan_objmgr_vdev *vdev,
					      uint32_t val);

/**
 * ucfg_nan_get_ndi_delete_rsp_status: get ndi delete response status from vdev
 * obj
 * @vdev: pointer to vdev object
 *
 * Return: ndi delete rsp status
 */
uint32_t ucfg_nan_get_ndi_delete_rsp_status(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_nan_get_callbacks: ucfg API to return callbacks
 * @psoc: pointer to psoc object
 * @cb_obj: callback struct to populate
 *
 * Return: callback struct on sucess, NULL otherwise
 */
QDF_STATUS ucfg_nan_get_callbacks(struct wlan_objmgr_psoc *psoc,
				  struct nan_callbacks *cb_obj);

#endif /* _NAN_UCFG_API_H_ */
