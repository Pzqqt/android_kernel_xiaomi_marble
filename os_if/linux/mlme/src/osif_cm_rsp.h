/*
 * Copyright (c) 2012-2015,2020-2021 The Linux Foundation. All rights reserved.
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
 * DOC: osif_cm_rsp.h
 *
 * This header file maintains declarations of connect, disconnect, roam
 * response apis.
 */

#ifndef __OSIF_CM_RSP_H
#define __OSIF_CM_RSP_H

#include "wlan_objmgr_vdev_obj.h"
#include "wlan_cm_public_struct.h"

/**
 * osif_disconnect_handler() - Indicate disconnnect to userspace
 * @vdev: vdev pointer
 * @rsp: Disconnect response from connection manager
 *
 * This function indicates disconnect to the kernel which thus indicates
 * to the userspace.
 *
 * Context: Any context
 * Return: QDF_STATUS_SUCCESS on successful indication to kernel,
 * else QDF_STATUS with failure reason
 */
QDF_STATUS osif_disconnect_handler(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_discon_rsp *rsp);

/**
 * osif_cm_get_assoc_req_ie_data() - Get the assoc req IE offset and length
 * if valid assoc req is present
 * @assoc_req: assoc req info
 * @ie_data_len: IE date length to be calculated
 * @ie_data_ptr: IE data pointer to be calculated
 *
 * Return: void
 */
void osif_cm_get_assoc_req_ie_data(struct element_info *assoc_req,
				   size_t *ie_data_len,
				   const uint8_t **ie_data_ptr);

/**
 * osif_cm_get_assoc_rsp_ie_data() - Get the assoc resp IE offset and length
 * if valid assoc req is present
 * @assoc_req: assoc req info
 * @ie_data_len: IE date length to be calculated
 * @ie_data_ptr: IE data pointer to be calculated
 *
 * Return: void
 */
void osif_cm_get_assoc_rsp_ie_data(struct element_info *assoc_rsp,
				   size_t *ie_data_len,
				   const uint8_t **ie_data_ptr);

/**
 * osif_connect_handler() - API to send connect response to kernel
 * @vdev: vdev pointer
 * @rsp: Connection manager connect response
 *
 * The API is used to send connection response to kernel
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
QDF_STATUS osif_connect_handler(struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_connect_resp *rsp);

#ifdef CONN_MGR_ADV_FEATURE
/**
 * osif_indicate_reassoc_results() - API to send reassoc response to kernel
 * @vdev: vdev pointer
 * @osif_priv: OS private structure of vdev
 * @rsp: Connection manager response
 *
 * The API is used to send reassoc response to kernel
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
void osif_indicate_reassoc_results(struct wlan_objmgr_vdev *vdev,
				   struct vdev_osif_priv *osif_priv,
				   struct wlan_cm_connect_resp *rsp);

/**
 * @osif_pmksa_candidate_notify_cb: Roam pmksa candidate notify callback
 * @vdev: vdev pointer
 * @bssid: bssid
 * @index: index
 * @preauth: preauth flag
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
osif_pmksa_candidate_notify(struct wlan_objmgr_vdev *vdev,
			    struct qdf_mac_addr *bssid,
			    int index, bool preauth);
#else
static inline void
osif_indicate_reassoc_results(struct wlan_objmgr_vdev *vdev,
			      struct vdev_osif_priv *osif_priv,
			      struct wlan_cm_connect_resp *rsp)
{}
#endif

/**
 * osif_failed_candidate_handler() - API to indicate individual candidate
 * connect failure resp
 * @vdev: vdev pointer
 * @rsp: Connection manager connect response for the candidate
 *
 * The API is is used indicate individual candidate connect failure resp for
 * for tried all but last tried candidate. The last candidate will be sent in
 * osif_connect_handler.
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
QDF_STATUS osif_failed_candidate_handler(struct wlan_objmgr_vdev *vdev,
					 struct wlan_cm_connect_resp *rsp);

#endif /* __OSIF_CM_RSP_H */
