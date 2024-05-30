/*
 * Copyright (c) 2011-2021 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cm_roam.h
 *
 * This header file maintain APIs required for connection mgr roam functions
 */

#ifndef __WLAN_CM_ROAM_H__
#define __WLAN_CM_ROAM_H__

#include "wlan_cm_main.h"
#include "wlan_cm_sm.h"

#ifdef WLAN_FEATURE_HOST_ROAM
/**
 * cm_is_host_roam_enabled() - Check if WLAN_FEATURE_HOST_ROAM is enabled
 *
 * Return: Return true if WLAN_FEATURE_HOST_ROAM is enabled
 */
static inline bool cm_is_host_roam_enabled(void)
{
	return true;
}

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
 * @resp: Connect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_reassoc_rsp(struct wlan_objmgr_vdev *vdev,
			  struct wlan_cm_connect_resp *resp);

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
 * @resp: connect resp.
 *
 * This API would be called after roam completion resp from VDEV mgr
 *
 * Return: QDF status
 */
QDF_STATUS cm_reassoc_complete(struct cnx_mgr *cm_ctx,
			       struct wlan_cm_connect_resp *resp);

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

#ifdef WLAN_POLICY_MGR_ENABLE
/**
 * cm_reassoc_hw_mode_change_resp() - HW mode change response
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @cm_id: reassoc ID which gave the hw mode change request
 * @status: status of the HW mode change.
 *
 * Return: void
 */
void cm_reassoc_hw_mode_change_resp(struct wlan_objmgr_pdev *pdev,
				    uint8_t vdev_id,
				    wlan_cm_id cm_id, QDF_STATUS status);

/**
 * cm_handle_reassoc_hw_mode_change() - SM handling of reassoc hw mode change
 * resp
 * @cm_ctx: connection manager context
 * @cm_id: Connection mgr ID assigned to this connect request.
 * @event: HW mode success or failure event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_handle_reassoc_hw_mode_change(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id,
				 enum wlan_cm_sm_evt event);
#else
static inline QDF_STATUS
cm_handle_reassoc_hw_mode_change(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id,
				 enum wlan_cm_sm_evt event)
{
	return QDF_STATUS_SUCCESS;
}
#endif

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
 * @resp: connect resp
 *
 * Return: bool
 */
bool cm_roam_resp_cmid_match_list_head(struct cnx_mgr *cm_ctx,
				       struct wlan_cm_connect_resp *resp);

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

#ifdef CONN_MGR_ADV_FEATURE
/*
 * cm_update_advance_roam_scan_filter() - fill scan filter for roam
 * @vdev: vdev
 * @filter: scan filter
 *
 * Return QDF_STATUS
 */
QDF_STATUS cm_update_advance_roam_scan_filter(
		struct wlan_objmgr_vdev *vdev, struct scan_filter *filter);
#endif

#ifdef WLAN_FEATURE_PREAUTH_ENABLE
/*
 * cm_host_roam_preauth_start() - start preauth process
 * @cm_ctx: Connection manager context
 * @cm_req: Struct containing the roam request
 *
 * Return QDF_STATUS
 */
QDF_STATUS cm_host_roam_preauth_start(struct cnx_mgr *cm_ctx,
				      struct cm_req *cm_req);

/**
 * cm_preauth_active() - This API would be called after the preauth
 * request gets activated in serialization.
 * @cm_ctx: connection manager context
 * @cm_id: Connection mgr ID assigned to this preauth request.
 *
 * Return: QDF status
 */
QDF_STATUS cm_preauth_active(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id);

/**
 * cm_preauth_done_resp() - This API would be called when preauth
 * response msg handling
 * @cm_ctx: connection manager context
 * @cm_id: Connection mgr ID assigned to this preauth request.
 *
 * Return: void
 */
void cm_preauth_done_resp(struct cnx_mgr *cm_ctx, struct wlan_preauth_rsp *rsp);

/**
 * cm_preauth_success() - Preauth is successfully completed
 * @cm_ctx: connection manager context
 * @rsp: Preauth resp
 *
 * Return: void
 */
void cm_preauth_success(struct cnx_mgr *cm_ctx, struct wlan_preauth_rsp *rsp);

/**
 * cm_preauth_fail() - This API would be called after the preauth
 * request gets failed.
 * @cm_ctx: connection manager context
 * @preauth_fail_rsp: preauth fail response
 *
 * Return: none
 */
void cm_preauth_fail(struct cnx_mgr *cm_ctx,
		     struct wlan_cm_preauth_fail *preauth_fail_rsp);

/**
 * cm_send_preauth_start_fail() - This API would be called after send preauth
 * request failed.
 * @cm_ctx: connection manager context
 * @cm_id: connection mgr ID assigned to this preauth request.
 * @reason: connect fail reason
 *
 * Return: QDF status
 */
QDF_STATUS cm_send_preauth_start_fail(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id,
				      enum wlan_cm_connect_fail_reason reason);

/**
 * cm_handle_reassoc_timer() - handle ressoc timer expiry
 * @cm_ctx: connection manager context
 * @cm_id: connection mgr ID assigned to this preauth request.
 *
 * Return: QDF status
 */
QDF_STATUS cm_handle_reassoc_timer(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id);
#endif /* WLAN_FEATURE_PREAUTH_ENABLE */
#else /* WLAN_FEATURE_HOST_ROAM */

static inline bool cm_is_host_roam_enabled(void)
{
	return false;
}

#ifdef WLAN_POLICY_MGR_ENABLE
static inline
void cm_reassoc_hw_mode_change_resp(struct wlan_objmgr_pdev *pdev,
				    uint8_t vdev_id,
				    wlan_cm_id cm_id, QDF_STATUS status) {}
#endif

static inline QDF_STATUS cm_reassoc_complete(struct cnx_mgr *cm_ctx,
					     struct wlan_cm_connect_resp *resp)
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
#endif /* WLAN_FEATURE_HOST_ROAM */

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
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
				      struct wlan_cm_connect_resp *resp);

/**
 * cm_is_roam_enabled() - Check if host roam or roam offload is enabled.
 * @psoc: psoc context
 *
 * Return: true if any of the roaming mode is enabled
 */
bool cm_is_roam_enabled(struct wlan_objmgr_psoc *psoc);
#else
static inline QDF_STATUS
cm_check_and_prepare_roam_req(struct cnx_mgr *cm_ctx,
			      struct cm_connect_req *connect_req,
			      struct cm_req **roam_req)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline void cm_free_roam_req_mem(struct cm_roam_req *roam_req) {}

static inline QDF_STATUS
cm_add_roam_req_to_list(struct cnx_mgr *cm_ctx, struct cm_req *cm_req)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
cm_fill_bss_info_in_roam_rsp_by_cm_id(struct cnx_mgr *cm_ctx,
				      wlan_cm_id cm_id,
				      struct wlan_cm_connect_resp *resp)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline bool cm_is_roam_enabled(struct wlan_objmgr_psoc *psoc)
{
	mlme_rl_debug("Roaming is disabled");
	return false;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * cm_fw_roam_start() - Handle roam start event
 * @cm_ctx: connection mgr context
 *
 * This function handles the roam start event received from FW.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_fw_roam_start(struct cnx_mgr *cm_ctx);

/**
 * cm_send_roam_invoke_req() - Send Roam invoke req to FW
 * @cm_ctx: connection manager context
 * @req: connection manager request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_send_roam_invoke_req(struct cnx_mgr *cm_ctx, struct cm_req *req);

/**
 * cm_roam_offload_enabled() - check if roam offload(LFR3) is enabled
 * @psoc: psoc pointer to get the INI
 *
 * Return: bool
 */
bool cm_roam_offload_enabled(struct wlan_objmgr_psoc *psoc);

/**
 * cm_get_first_roam_command() - Get first roam request from list
 * @vdev: vdev pointer
 *
 * Context: Can be called from any context and to be used only after posting a
 * msg to SM (ie holding the SM lock) to avoid use after free. also returned req
 * should only be used till SM lock is hold.
 *
 * Return: cm roam req from the req list
 */
struct cm_roam_req *cm_get_first_roam_command(struct wlan_objmgr_vdev *vdev);

/**
 * cm_prepare_roam_cmd() - Prepare roam req
 * @cm_ctx: connection mgr context
 * @cm_req: connection mgr req
 * @source: connection mgr req source
 *
 * This function prepares roam request when roam start ind is received
 * when CM SM is in connected state.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_prepare_roam_cmd(struct cnx_mgr *cm_ctx,
			       struct cm_req **roam_req,
			       enum wlan_cm_source source);

/**
 * cm_add_fw_roam_cmd_to_list_n_ser() - Add roam req to list and serialize req
 * @cm_ctx: connection mgr context
 * @cm_req: connection mgr req
 *
 * This function adds roam request to list and the serialization queue.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_add_fw_roam_cmd_to_list_n_ser(struct cnx_mgr *cm_ctx,
					    struct cm_req *cm_req);

/**
 * cm_fw_send_vdev_roam_event() - CM send VDEV ROAM Event
 * @cm_ctx: connection mgr context
 * @data_len: data size
 * @data: event data
 *
 * This function sends ROAM Event to vdev manager
 * state machine
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_fw_send_vdev_roam_event(struct cnx_mgr *cm_ctx, uint16_t data_len,
			   void *data);

/**
 * cm_fw_roam_complete() - CM handle roam complete
 * @cm_ctx: connection mgr context
 * @data: join rsp data
 *
 * This function CM handle roam complete
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_fw_roam_complete(struct cnx_mgr *cm_ctx, void *data);

#else
static inline bool cm_roam_offload_enabled(struct wlan_objmgr_psoc *psoc)
{
	return false;
}
#endif

#endif /* __WLAN_CM_ROAM_H__ */
