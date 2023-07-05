/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "wlan_cm_public_struct.h"
#include "wlan_ext_mlme_obj_types.h"

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
 * wlan_cm_disconnect() - disconnect start request
 * @vdev: vdev pointer
 * @source: disconnect source
 * @reason_code: disconnect reason
 * @bssid: bssid of AP to disconnect, can be null if not known
 *
 * Context: can be called from any context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_disconnect(struct wlan_objmgr_vdev *vdev,
			      enum wlan_cm_source source,
			      enum wlan_reason_code reason_code,
			      struct qdf_mac_addr *bssid);

/**
 * wlan_cm_disconnect_sync() - disconnect request with wait till
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
QDF_STATUS wlan_cm_disconnect_sync(struct wlan_objmgr_vdev *vdev,
				   enum wlan_cm_source source,
				   enum wlan_reason_code reason_code);

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
			       struct wlan_cm_connect_resp *resp);

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
 * @resp: disconnect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_disconnect_rsp(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_discon_rsp *resp);

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
 * wlan_cm_set_max_connect_timeout() - Set max connect timeout
 * @vdev: vdev pointer
 * @max_connect_timeout: max connect timeout to be set.
 *
 * Set max connect timeout.
 *
 * Return: void
 */
void wlan_cm_set_max_connect_timeout(struct wlan_objmgr_vdev *vdev,
				     uint32_t max_connect_timeout);

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
 * wlan_cm_is_vdev_active() - check if vdev is in active state ie conneted or
 * roaming state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool wlan_cm_is_vdev_active(struct wlan_objmgr_vdev *vdev);

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

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * wlan_cm_is_vdev_roam_started() - check if vdev is in roaming state and
 * roam started sub stated
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool wlan_cm_is_vdev_roam_started(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cm_is_vdev_roam_sync_inprogress() - check if vdev is in roaming state
 * and roam sync substate
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool wlan_cm_is_vdev_roam_sync_inprogress(struct wlan_objmgr_vdev *vdev);
#else
static inline bool wlan_cm_is_vdev_roam_started(struct wlan_objmgr_vdev *vdev)
{
	return false;
}

static inline
bool wlan_cm_is_vdev_roam_sync_inprogress(struct wlan_objmgr_vdev *vdev)
{
	return false;
}
#endif

#ifdef WLAN_FEATURE_HOST_ROAM
/**
 * wlan_cm_is_vdev_roam_preauth_state() - check if vdev is in roaming state and
 * preauth is in progress
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool wlan_cm_is_vdev_roam_preauth_state(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cm_is_vdev_roam_reassoc_state() - check if vdev is in roaming state
 * and reassoc is in progress
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool wlan_cm_is_vdev_roam_reassoc_state(struct wlan_objmgr_vdev *vdev);
#else
static inline
bool wlan_cm_is_vdev_roam_preauth_state(struct wlan_objmgr_vdev *vdev)
{
	return false;
}

static inline
bool wlan_cm_is_vdev_roam_reassoc_state(struct wlan_objmgr_vdev *vdev)
{
	return false;
}
#endif

/**
 * wlan_cm_get_active_connect_req() - Get copy of active connect request
 * @vdev: vdev pointer
 * @req: pointer to the copy of the active connect request
 * *
 * Context: Should be called only in the conext of the
 * cm request activation
 *
 * Return: true and connect req if any request is active
 */
bool wlan_cm_get_active_connect_req(struct wlan_objmgr_vdev *vdev,
				    struct wlan_cm_vdev_connect_req *req);

#ifdef WLAN_FEATURE_HOST_ROAM
/**
 * wlan_cm_get_active_reassoc_req() - Get copy of active reassoc request
 * @vdev: vdev pointer
 * @req: pointer to the copy of the active reassoc request
 * *
 * Context: Should be called only in the conext of the
 * cm request activation
 *
 * Return: true and reassoc req if any request is active
 */
bool wlan_cm_get_active_reassoc_req(struct wlan_objmgr_vdev *vdev,
				    struct wlan_cm_vdev_reassoc_req *req);
#else
static inline
bool wlan_cm_get_active_reassoc_req(struct wlan_objmgr_vdev *vdev,
				    struct wlan_cm_vdev_reassoc_req *req)
{
	return false;
}
#endif

/**
 * wlan_cm_get_active_disconnect_req() - Get copy of active disconnect request
 * @vdev: vdev pointer
 * @req: pointer to the copy of the active disconnect request
 * *
 * Context: Should be called only in the conext of the
 * cm request activation
 *
 * Return: true and disconnect req if any request is active
 */
bool wlan_cm_get_active_disconnect_req(struct wlan_objmgr_vdev *vdev,
				       struct wlan_cm_vdev_discon_req *req);

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
 * wlan_cm_get_active_req_type() - return cm  active request type
 * @vdev: vdev pointer
 *
 * This function returns the cm active request type
 *
 * Return: active request type if any, otherwise return 0
 */
enum wlan_cm_active_request_type
wlan_cm_get_active_req_type(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cm_get_ext_hdl() - Get connection manager ext context from vdev
 * @vdev: vdev pointer
 *
 * Return: pointer to connection manager ext context
 */
cm_ext_t *wlan_cm_get_ext_hdl(struct wlan_objmgr_vdev *vdev);

#ifdef WLAN_FEATURE_HOST_ROAM
/**
 * wlan_cm_reassoc_rsp() - Connection manager reassoc response
 * @vdev: vdev pointer
 * @resp: Connect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_reassoc_rsp(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_connect_resp *resp);
#else
static inline
QDF_STATUS wlan_cm_reassoc_rsp(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_connect_resp *resp)
{
	return QDF_STATUS_SUCCESS;
}
#endif

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

#ifdef SM_ENG_HIST_ENABLE
/**
 * wlan_cm_sm_history_print() - Prints SM history
 * @vdev: Objmgr vdev
 *
 * API to print CM SM history
 *
 * Return: void
 */
void wlan_cm_sm_history_print(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cm_req_history_print() - Prints CM request history
 * @vdev: Objmgr vdev
 *
 * API to print CM request history
 *
 * Return: void
 */
void wlan_cm_req_history_print(struct wlan_objmgr_vdev *vdev);
#else
static inline void wlan_cm_sm_history_print(struct wlan_objmgr_vdev *vdev)
{
}

static inline void wlan_cm_req_history_print(struct wlan_objmgr_vdev *vdev)
{}
#endif

#ifdef CONN_MGR_ADV_FEATURE
/**
 * wlan_cm_set_candidate_advance_filter_cb() - Set CM candidate advance
 * filter cb
 * @vdev: Objmgr vdev
 * @filter_fun: CM candidate advance filter cb
 *
 * Return: void
 */
static inline
void wlan_cm_set_candidate_advance_filter_cb(
		struct wlan_objmgr_vdev *vdev,
		void (*filter_fun)(struct wlan_objmgr_vdev *vdev,
				   struct scan_filter *filter))
{
}

/**
 * wlan_cm_set_candidate_custom_sort_cb() - Set CM candidate custom sort cb
 * @vdev: Objmgr vdev
 * @sort_fun: CM candidate custom sort cb
 *
 * Return: void
 */
static inline
void wlan_cm_set_candidate_custom_sort_cb(
		struct wlan_objmgr_vdev *vdev,
		void (*sort_fun)(struct wlan_objmgr_vdev *vdev,
				 qdf_list_t *list))
{
}
#else
void wlan_cm_set_candidate_advance_filter_cb(
		struct wlan_objmgr_vdev *vdev,
		void (*filter_fun)(struct wlan_objmgr_vdev *vdev,
				   struct scan_filter *filter));

void wlan_cm_set_candidate_custom_sort_cb(
		struct wlan_objmgr_vdev *vdev,
		void (*sort_fun)(struct wlan_objmgr_vdev *vdev,
				 qdf_list_t *list));
#endif

/**
 * wlan_cm_get_rnr() - get rnr
 * @vdev:vdev
 * @cm_id: connect mgr id
 *
 * Return: rnr pointer
 */
struct reduced_neighbor_report *wlan_cm_get_rnr(struct wlan_objmgr_vdev *vdev,
						wlan_cm_id cm_id);

/**
 * wlan_cm_disc_cont_after_rso_stop() - Continue disconnect after RSO stop
 * @vdev: Objmgr vdev
 * @is_ho_fail: True if ho_fail happened
 * @req: pointer to cm vdev disconnect req

 * This is a wrapper to call core API cm_disconnect_continue_after_rso_stop
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_disc_cont_after_rso_stop(struct wlan_objmgr_vdev *vdev,
				 bool is_ho_fail,
				 struct wlan_cm_vdev_discon_req *req);
#endif /* __WLAN_CM_UCFG_API_H */
