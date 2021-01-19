/*
 * Copyright (c) 2011-2021 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cm_roam.h
 *
 * This header file maintain APIs required for connection mgr roam functions
 */

#ifndef __WLAN_CM_ROAM_H__
#define __WLAN_CM_ROAM_H__

#ifdef WLAN_FEATURE_HOST_ROAM
/**
 * cm_roam_bss_peer_create_rsp() - handle bss peer create response for roam
 * @vdev: vdev
 * @status: bss peer create status
 * @peer_mac: peer mac
 *
 * Return: QDF status
 */
QDF_STATUS cm_roam_bss_peer_create_rsp(struct wlan_objmgr_vdev *vdev,
				       QDF_STATUS status,
				       struct qdf_mac_addr *peer_mac);

/**
 * cm_reassoc_rsp() - Connection manager reassoc response
 * @vdev: vdev pointer
 * @resp: Reassoc response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_reassoc_rsp(struct wlan_objmgr_vdev *vdev,
			  struct wlan_cm_roam_resp *resp);

/**
 * cm_roam_disconnect_rsp() - Connection manager api to post connect event
 * @vdev: VDEV object
 * @cm_discon_rsp: Disconnect response
 *
 * Context: Any context.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_roam_disconnect_rsp(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_discon_rsp *resp);

/**
 * cm_reassoc_complete() - This API would be called after reassoc complete
 * request from the serialization.
 * @cm_ctx: connection manager context
 * @resp: Roam complete resp.
 *
 * This API would be called after roam completion resp from VDEV mgr
 *
 * Return: QDF status
 */
QDF_STATUS cm_reassoc_complete(struct cnx_mgr *cm_ctx,
			       struct wlan_cm_roam_resp *resp);

/**
 * cm_get_active_reassoc_req() - Get copy of active reassoc request
 * @vdev: vdev pointer
 * @req: pointer to the copy of the active reassoc request
 * *
 * Context: Should be called only in the conext of the
 * cm request activation
 *
 * Return: true and reassoc req if any request is active
 */
bool cm_get_active_reassoc_req(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_vdev_reassoc_req *req);

/**
 * cm_host_roam_start_req() - Start host roam request
 * @cm_ctx: Connection manager context
 * @cm_req: Struct containing the roam request
 *
 * Return: QDF_STATUS_SUCCESS on delivering the event
 * to connection state machine else error value.
 */
QDF_STATUS cm_host_roam_start_req(struct cnx_mgr *cm_ctx,
				  struct cm_req *cm_req);

/**
 * cm_reassoc_start() - This API will be called to initiate the reassoc
 * process
 * @cm_ctx: connection manager context
 * @req: roam request.
 *
 * Return: QDF status
 */
QDF_STATUS cm_reassoc_start(struct cnx_mgr *cm_ctx, struct cm_roam_req *req);

/**
 * cm_reassoc_active() - This API would be called after the reassoc
 * request gets activated in serialization.
 * @cm_ctx: connection manager context
 * @cm_id: Connection mgr ID assigned to this reassoc request.
 *
 * Return: QDF status
 */
QDF_STATUS cm_reassoc_active(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id);

/**
 * cm_reassoc_disconnect_complete() - This API would be called after
 * disconnect complete due to reassoc request.
 * @cm_ctx: connection manager context
 * @resp: disconnection complete resp.
 *
 * This API would be called after disconnection completion resp from VDEV mgr
 *
 * Return: QDF status
 */

QDF_STATUS cm_reassoc_disconnect_complete(struct cnx_mgr *cm_ctx,
					  struct wlan_cm_discon_rsp *resp);

/**
 * cm_resume_reassoc_after_peer_create() - Called after bss create rsp
 * @cm_ctx: connection manager context
 * @cm_id: Connection mgr ID assigned to this reassoc request.
 *
 * Return: QDF status
 */
QDF_STATUS
cm_resume_reassoc_after_peer_create(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id);

/**
 * cm_roam_resp_cmid_match_list_head() - Check if resp cmid is same as list
 * head
 * @cm_ctx: connection manager context
 * @resp: roam resp
 *
 * Return: bool
 */
bool cm_roam_resp_cmid_match_list_head(struct cnx_mgr *cm_ctx,
				       struct wlan_cm_roam_resp *resp);

/**
 * cm_send_reassoc_start_fail() - initiate reassoc failure
 * @cm_ctx: connection manager context
 * @cm_id: active command id
 * @reason: failure reason
 * @sync: Indicate if the event has to be dispatched in sync or async mode
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_send_reassoc_start_fail(struct cnx_mgr *cm_ctx,
			   wlan_cm_id cm_id,
			   enum wlan_cm_connect_fail_reason reason,
			   bool sync);

#else
static inline QDF_STATUS cm_reassoc_complete(struct cnx_mgr *cm_ctx,
					     struct wlan_cm_roam_resp *resp)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS cm_roam_disconnect_rsp(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_discon_rsp *resp)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS cm_roam_bss_peer_create_rsp(struct wlan_objmgr_vdev *vdev,
				       QDF_STATUS status,
				       struct qdf_mac_addr *peer_mac)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * cm_check_and_prepare_roam_req() - Initiate roam request
 * @cm_ctx: connection manager context
 * @connect_req: connection manager request
 * @roam_req: Roam request
 *
 * Context: Can be called only while handling connection manager event
 *          ie holding state machine lock
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_check_and_prepare_roam_req(struct cnx_mgr *cm_ctx,
			      struct cm_connect_req *connect_req,
			      struct cm_req **roam_req);
/**
 * cm_free_roam_req_mem() - free croam req internal memory, to be called
 * before cm_req is freed
 * @roam_req: roam req
 *
 * Return: void
 */
void cm_free_roam_req_mem(struct cm_roam_req *roam_req);

/**
 * cm_add_roam_req_to_list() - add connect req to the connection manager
 * req list
 * @vdev: vdev on which connect is received
 * @cm_req: Roam req provided
 *
 * Return: QDF status
 */
QDF_STATUS cm_add_roam_req_to_list(struct cnx_mgr *cm_ctx,
				   struct cm_req *cm_req);

/**
 * cm_fill_bss_info_in_roam_rsp_by_cm_id() - fill bss info for the cm id
 * @cm_ctx: connection manager context
 * @cm_id: cm id of connect/disconnect req
 * @resp: resp to copy bss info like ssid/bssid and freq
 *
 * Return: Success if entry was found else failure
 */
QDF_STATUS
cm_fill_bss_info_in_roam_rsp_by_cm_id(struct cnx_mgr *cm_ctx,
				      wlan_cm_id cm_id,
				      struct wlan_cm_roam_resp *resp);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * cm_roam_offload_enabled() - check if roam offload(LFR3) is enabled
 * @psoc: psoc pointer to get the INI
 *
 * Return: bool
 */
static inline bool cm_roam_offload_enabled(struct wlan_objmgr_psoc *psoc)
{
	/* use INI CFG_LFR3_ROAMING_OFFLOAD,return true for now */
	return true;
}
#else
static inline bool cm_roam_offload_enabled(struct wlan_objmgr_psoc *psoc)
{
	return false;
}
#endif

#endif /* __WLAN_CM_ROAM_H__ */
