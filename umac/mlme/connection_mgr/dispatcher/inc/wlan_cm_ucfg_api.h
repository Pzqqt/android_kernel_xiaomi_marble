/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cm_ucfg_api.h
 *
 * This file maintains declarations of public ucfg apis
 */

#ifndef __WLAN_CM_UCFG_API_H
#define __WLAN_CM_UCFG_API_H

#include <wlan_cm_api.h>

/**
 * ucfg_cm_start_connect() - connect start request
 * @vdev: vdev pointer
 * @req: connect req
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_cm_start_connect(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_connect_req *req);

/**
 * ucfg_cm_start_disconnect() - disconnect start request
 * @vdev: vdev pointer
 * @req: disconnect req
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_cm_start_disconnect(struct wlan_objmgr_vdev *vdev,
				    struct wlan_cm_disconnect_req *req);

/**
 * ucfg_cm_disconnect_sync() - disconnect request with wait till
 * completed
 * @vdev: vdev pointer
 * @source: disconnect source
 * @reason_code: disconnect reason
 *
 * Context: Only call for north bound disconnect req, if wait till complete
 * is required, e.g. during vdev delete. Do not call from scheduler context.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_cm_disconnect_sync(struct wlan_objmgr_vdev *vdev,
				   enum wlan_cm_source source,
				   enum wlan_reason_code reason_code);

/**
 * ucfg_cm_is_vdev_connecting() - check if vdev is in conneting state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool ucfg_cm_is_vdev_connecting(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cm_is_vdev_connected() - check if vdev is in conneted state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool ucfg_cm_is_vdev_connected(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cm_is_vdev_active() - check if vdev is in active state ie conneted or
 * roaming state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool ucfg_cm_is_vdev_active(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cm_is_vdev_disconnecting() - check if vdev is in disconneting state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool ucfg_cm_is_vdev_disconnecting(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cm_is_vdev_disconnected() - check if vdev is disconnected/init state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool ucfg_cm_is_vdev_disconnected(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cm_is_vdev_roaming() - check if vdev is in roaming state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool ucfg_cm_is_vdev_roaming(struct wlan_objmgr_vdev *vdev);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * ucfg_cm_is_vdev_roam_started() - check if vdev is in roaming state and
 * roam started sub stated
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool ucfg_cm_is_vdev_roam_started(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cm_is_vdev_roam_sync_inprogress() - check if vdev is in roaming state
 * and roam sync substate
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool ucfg_cm_is_vdev_roam_sync_inprogress(struct wlan_objmgr_vdev *vdev);
#else
static inline bool ucfg_cm_is_vdev_roam_started(struct wlan_objmgr_vdev *vdev)
{
	return false;
}

static inline
bool ucfg_cm_is_vdev_roam_sync_inprogress(struct wlan_objmgr_vdev *vdev)
{
	return false;
}
#endif

#ifdef WLAN_FEATURE_HOST_ROAM
/**
 * ucfg_cm_is_vdev_roam_preauth_state() - check if vdev is in roaming state and
 * preauth is in progress
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool ucfg_cm_is_vdev_roam_preauth_state(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cm_is_vdev_roam_reassoc_state() - check if vdev is in roaming state
 * and reassoc is in progress
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool ucfg_cm_is_vdev_roam_reassoc_state(struct wlan_objmgr_vdev *vdev);
#else
static inline
bool ucfg_cm_is_vdev_roam_preauth_state(struct wlan_objmgr_vdev *vdev)
{
	return false;
}

static inline
bool ucfg_cm_is_vdev_roam_reassoc_state(struct wlan_objmgr_vdev *vdev)
{
	return false;
}
#endif

/**
 * ucfg_cm_reason_code_to_str() - return string conversion of reason code
 * @reason: reason code.
 *
 * This utility function helps log string conversion of reason code.
 *
 * Return: string conversion of reason code, if match found;
 *         "Unknown" otherwise.
 */
static inline
const char *ucfg_cm_reason_code_to_str(enum wlan_reason_code reason)
{
	return wlan_cm_reason_code_to_str(reason);
}
#endif /* __WLAN_CM_UCFG_API_H */
