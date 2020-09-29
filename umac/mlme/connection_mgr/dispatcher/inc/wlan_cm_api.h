/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cm_api.h
 *
 * This file maintains declarations of public apis
 */

#ifndef __WLAN_CM_API_H
#define __WLAN_CM_API_H

#ifdef FEATURE_CM_ENABLE
#include "wlan_cm_public_struct.h"

/**
 * wlan_cm_start_connect() - connect start request
 * @vdev: vdev pointer
 * @req: connect req
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_start_connect(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_connect_req *req);

/**
 * wlan_cm_start_disconnect() - disconnect start request
 * @vdev: vdev pointer
 * @req: disconnect req
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_start_disconnect(struct wlan_objmgr_vdev *vdev,
				    struct wlan_cm_disconnect_req *req);

/**
 * wlan_cm_bss_select_ind_rsp() - Connection manager resp for bss
 * select indication
 * @vdev: vdev pointer
 * @status: Status
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_bss_select_ind_rsp(struct wlan_objmgr_vdev *vdev,
				      QDF_STATUS status);

/**
 * wlan_cm_bss_peer_create_rsp() - Connection manager bss peer create response
 * @vdev: vdev pointer
 * @status: Status
 * @peer_mac: Peer mac address
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_bss_peer_create_rsp(struct wlan_objmgr_vdev *vdev,
				       QDF_STATUS status,
				       struct qdf_mac_addr *peer_mac);

/**
 * wlan_cm_connect_rsp() - Connection manager connect response
 * @vdev: vdev pointer
 * @resp: Connect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_connect_rsp(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_connect_rsp *resp);

/**
 * wlan_cm_bss_peer_delete_ind() - Connection manager peer delete indication
 * @vdev: vdev pointer
 * @peer_mac: Peer mac address
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_bss_peer_delete_ind(struct wlan_objmgr_vdev *vdev,
				       struct qdf_mac_addr *peer_mac);

/**
 * wlan_cm_bss_peer_delete_rsp() - Connection manager peer delete response
 * @vdev: vdev pointer
 * @status: status
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_bss_peer_delete_rsp(struct wlan_objmgr_vdev *vdev,
				       uint32_t status);

/**
 * wlan_cm_disconnect_rsp() - Connection manager disconnect response
 * @vdev: vdev pointer
 * @cm_discon_rsp: disconnect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_disconnect_rsp(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_discon_rsp cm_discon_rsp);

/**
 * wlan_cm_set_max_connect_attempts() - Set max connect attempts
 * @vdev: vdev pointer
 * @max_connect_attempts: max connect attempts to be set.
 *
 * Set max connect attempts. Max value is limited to CM_MAX_CONNECT_ATTEMPTS.
 *
 * Return: void
 */
void wlan_cm_set_max_connect_attempts(struct wlan_objmgr_vdev *vdev,
				      uint8_t max_connect_attempts);

/**
 * wlan_cm_is_vdev_connecting() - check if vdev is in conneting state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool wlan_cm_is_vdev_connecting(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cm_is_vdev_connected() - check if vdev is in conneted state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool wlan_cm_is_vdev_connected(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cm_is_vdev_disconnecting() - check if vdev is in disconneting state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool wlan_cm_is_vdev_disconnecting(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cm_is_vdev_disconnected() - check if vdev is disconnected/init state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool wlan_cm_is_vdev_disconnected(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cm_is_vdev_roaming() - check if vdev is in roaming state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool wlan_cm_is_vdev_roaming(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cm_reason_code_to_str() - return string conversion of reason code
 * @reason: reason code.
 *
 * This utility function helps log string conversion of reason code.
 *
 * Return: string conversion of reason code, if match found;
 *         "Unknown" otherwise.
 */
const char *wlan_cm_reason_code_to_str(enum wlan_reason_code reason);

/**
 * wlan_cm_hw_mode_change_resp() - HW mode change response
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @cm_id: connection ID which gave the hw mode change request
 * @status: status of the HW mode change.
 *
 * Return: void
 */
#ifdef WLAN_POLICY_MGR_ENABLE
void wlan_cm_hw_mode_change_resp(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
				 wlan_cm_id cm_id, QDF_STATUS status);
#endif /* ifdef POLICY_MGR_ENABLE */

#else

#ifdef WLAN_POLICY_MGR_ENABLE
static inline void
wlan_cm_hw_mode_change_resp(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			    uint32_t cm_id, QDF_STATUS status)
{
}
#endif /* ifdef POLICY_MGR_ENABLE */

#endif
#endif /* __WLAN_CM_UCFG_API_H */
