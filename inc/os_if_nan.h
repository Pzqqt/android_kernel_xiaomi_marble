/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * DOC: declares nan component os interface APIs
 */

#ifndef _OS_IF_NAN_H_
#define _OS_IF_NAN_H_

#include "qdf_types.h"
#include "nan_public_structs.h"
#include "nan_ucfg_api.h"

struct wlan_objmgr_psoc;
struct wlan_objmgr_vdev;

#ifdef WLAN_FEATURE_NAN_CONVERGENCE

/**
 * os_if_nan_process_ndp_cmd: os_if api to handle nan request message
 * @psoc: pointer to psoc object
 * @data: request data. contains vendor cmd tlvs
 * @data_len: length of data
 *
 * Return: status of operation
 */
int os_if_nan_process_ndp_cmd(struct wlan_objmgr_psoc *psoc,
				const void *data, int data_len);

/**
 * os_if_nan_event_handler: os_if handler api for nan response messages
 * @psoc: pointer to psoc object
 * @vdev: pointer to vdev object
 * @type: message type
 * @msg: msg buffer
 *
 * Return: None
 */
void os_if_nan_event_handler(struct wlan_objmgr_psoc *psoc,
			     struct wlan_objmgr_vdev *vdev,
			     uint32_t type, void *msg);

/**
 * os_if_nan_register_hdd_callbacks: os_if api to register hdd callbacks
 * @psoc: pointer to psoc object
 * @cb_obj: struct pointer containing callbacks
 *
 * Return: status of operation
 */
int os_if_nan_register_hdd_callbacks(struct wlan_objmgr_psoc *psoc,
				     struct nan_callbacks *cb_obj);

/**
 * os_if_nan_register_lim_callbacks: os_if api to register lim callbacks
 * @psoc: pointer to psoc object
 * @cb_obj: struct pointer containing callbacks
 *
 * Return: status of operation
 */
int os_if_nan_register_lim_callbacks(struct wlan_objmgr_psoc *psoc,
				     struct nan_callbacks *cb_obj);

/**
 * os_if_nan_post_ndi_create_rsp: os_if api to pos ndi create rsp to umac nan
 * component
 * @psoc: pointer to psoc object
 * @vdev_id: vdev id of ndi
 * @success: if create was success or failure
 *
 * Return: None
 */
void os_if_nan_post_ndi_create_rsp(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id, bool success);

/**
 * os_if_nan_post_ndi_delete_rsp: os_if api to pos ndi delete rsp to umac nan
 * component
 * @psoc: pointer to psoc object
 * @vdev_id: vdev id of ndi
 * @success: if delete was success or failure
 *
 * Return: None
 */
void os_if_nan_post_ndi_delete_rsp(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id, bool success);

/**
 * os_if_nan_ndi_session_end: os_if api to process ndi session end
 * component
 * @vdev: pointer to vdev deleted
 *
 * Return: None
 */
void os_if_nan_ndi_session_end(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_nan_set_ndi_state: os_if api set NDI state
 * @vdev: pointer to vdev deleted
 * @state: value to set
 *
 * Return: status of operation
 */
static inline QDF_STATUS os_if_nan_set_ndi_state(struct wlan_objmgr_vdev *vdev,
						 uint32_t state)
{
	return ucfg_nan_set_ndi_state(vdev, state);
}

/**
 * os_if_nan_set_ndp_create_transaction_id: set ndp create transaction id
 * @vdev: pointer to vdev object
 * @val: value to set
 *
 * Return: status of operation
 */
static inline QDF_STATUS os_if_nan_set_ndp_create_transaction_id(
						struct wlan_objmgr_vdev *vdev,
						uint16_t val)
{
	return ucfg_nan_set_ndp_create_transaction_id(vdev, val);
}

/**
 * os_if_nan_set_ndp_delete_transaction_id: set ndp delete transaction id
 * @vdev: pointer to vdev object
 * @val: value to set
 *
 * Return: status of operation
 */
static inline QDF_STATUS os_if_nan_set_ndp_delete_transaction_id(
						struct wlan_objmgr_vdev *vdev,
						uint16_t val)
{
	return ucfg_nan_set_ndp_delete_transaction_id(vdev, val);
}

#else

static inline void os_if_nan_post_ndi_create_rsp(struct wlan_objmgr_psoc *psoc,
						 uint8_t vdev_id, bool success)
{
}

static inline void os_if_nan_post_ndi_delete_rsp(struct wlan_objmgr_psoc *psoc,
						 uint8_t vdev_id, bool success)
{
}

#endif /* WLAN_FEATURE_NAN_CONVERGENCE */

#endif
