/*
 * Copyright (c) 2012-2015, 2020, The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cm_main_api.h
 *
 * This header file maintain connect, disconnect APIs of connection manager
 */

#ifndef __WLAN_CM_MAIN_API_H__
#define __WLAN_CM_MAIN_API_H__

#include "wlan_cm_main.h"
#include "wlan_cm_sm.h"
#include <include/wlan_mlme_cmn.h>
#ifdef WLAN_FEATURE_INTERFACE_MGR
#include <wlan_if_mgr_api.h>
#endif

#define CONNECT_REQ_PREFIX          0x00C00000
#define DISCONNECT_REQ_PREFIX       0x00D00000
#define CM_ID_MASK                  0x0000FFFF

#define CM_ID_GET_PREFIX(cm_id)     cm_id & 0xFFFF0000

#define CM_PREFIX_LOG "vdev %d cm_id 0x%x: "

/*************** CONNECT APIs ****************/

/**
 * cm_connect_start() - This API will be called to initiate the connect
 * process
 * @cm_ctx: connection manager context
 * @req: Connect request.
 *
 * Return: QDF status
 */
QDF_STATUS cm_connect_start(struct cnx_mgr *cm_ctx, struct cm_connect_req *req);

/**
 * cm_connect_scan_start() - This API will be called to initiate the connect
 * scan if no candidate are found in scan db.
 * @cm_ctx: connection manager context
 * @req: Connect request.
 *
 * Return: QDF status
 */
QDF_STATUS cm_connect_scan_start(struct cnx_mgr *cm_ctx,
				 struct cm_connect_req *req);

/**
 * cm_connect_scan_resp() - Handle the connect scan resp and next action
 * scan if no candidate are found in scan db.
 * @scan_id: scan id of the req
 * @status: Connect scan status
 *
 * Return: QDF status
 */
QDF_STATUS cm_connect_scan_resp(struct cnx_mgr *cm_ctx, wlan_scan_id *scan_id,
				QDF_STATUS status);

/**
 * cm_connect_resp_cmid_match_list_head() - Check if resp cmid is same as list
 * head
 * @cm_ctx: connection manager context
 * @resp: connect resp
 *
 * Return: bool
 */
bool cm_connect_resp_cmid_match_list_head(struct cnx_mgr *cm_ctx,
					  struct wlan_cm_connect_rsp *resp);

/**
 * cm_connect_active() - This API would be called after the connect
 * request gets activated in serialization.
 * @cm_ctx: connection manager context
 * @cm_id: Connection mgr ID assigned to this connect request.
 *
 * Return: QDF status
 */
QDF_STATUS cm_connect_active(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id);

/**
 * cm_try_next_candidate() - This API would try to connect to next valid
 * candidate and fail if no candidate left
 * request gets activated in serialization.
 * @cm_ctx: connection manager context
 * @connect_resp: connect resp.
 *
 * Return: QDF status
 */
QDF_STATUS cm_try_next_candidate(struct cnx_mgr *cm_ctx,
				 struct wlan_cm_connect_rsp *connect_resp);

/**
 * cm_peer_create_on_bss_select_ind_resp() - Called to create peer
 * if bss select inidication's resp was success
 * @cm_ctx: connection manager context
 * @cm_id: Connection mgr ID assigned to this connect request.
 *
 * Return: QDF status
 */
QDF_STATUS
cm_peer_create_on_bss_select_ind_resp(struct cnx_mgr *cm_ctx,
				      wlan_cm_id *cm_id);

/**
 * cm_resume_connect_after_peer_create() - Called after bss create rsp
 * @cm_ctx: connection manager context
 * @cm_id: Connection mgr ID assigned to this connect request.
 *
 * Return: QDF status
 */
QDF_STATUS
cm_resume_connect_after_peer_create(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id);

/**
 * cm_bss_select_ind_rsp() - Connection manager resp for bss
 * select indication
 * @vdev: vdev pointer
 * @status: Status
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_bss_select_ind_rsp(struct wlan_objmgr_vdev *vdev,
				 QDF_STATUS status);

/**
 * cm_bss_peer_create_rsp() - handle bss peer create response
 * @vdev: vdev
 * @status: bss peer create status
 * @peer_mac: peer mac
 *
 * Return: QDF status
 */
QDF_STATUS cm_bss_peer_create_rsp(struct wlan_objmgr_vdev *vdev,
				  QDF_STATUS status,
				  struct qdf_mac_addr *peer_mac);

/**
 * cm_connect_rsp() - Connection manager connect response
 * @vdev: vdev pointer
 * @resp: Connect response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_connect_rsp(struct wlan_objmgr_vdev *vdev,
			  struct wlan_cm_connect_rsp *resp);

/**
 * cm_connect_complete() - This API would be called after connect complete
 * request from the serialization.
 * @cm_ctx: connection manager context
 * @resp: Connection complete resp.
 *
 * This API would be called after connection completion resp from VDEV mgr
 *
 * Return: QDF status
 */
QDF_STATUS cm_connect_complete(struct cnx_mgr *cm_ctx,
			       struct wlan_cm_connect_rsp *resp);

/**
 * cm_add_connect_req_to_list() - add connect req to the connection manager
 * req list
 * @vdev: vdev on which connect is received
 * @req: Connection req provided
 *
 * Return: QDF status
 */
QDF_STATUS cm_add_connect_req_to_list(struct cnx_mgr *cm_ctx,
				      struct cm_connect_req *req);

/**
 * cm_connect_start_req() - Connect start req from the requester
 * @vdev: vdev on which connect is received
 * @req: Connection req provided
 *
 * Return: QDF status
 */
QDF_STATUS cm_connect_start_req(struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_connect_req *req);

#ifdef WLAN_POLICY_MGR_ENABLE
/**
 * cm_hw_mode_change_resp() - HW mode change response
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @cm_id: connection ID which gave the hw mode change request
 * @status: status of the HW mode change.
 *
 * Return: void
 */
void cm_hw_mode_change_resp(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			    wlan_cm_id cm_id, QDF_STATUS status);

/**
 * cm_handle_hw_mode_change() - SM handling of hw mode change resp
 * @cm_ctx: connection manager context
 * @cm_id: Connection mgr ID assigned to this connect request.
 * @event: HW mode success or failure event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_handle_hw_mode_change(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id,
				    enum wlan_cm_sm_evt event);
#else
QDF_STATUS cm_handle_hw_mode_change(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id,
				    enum wlan_cm_sm_evt event)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/*************** DISCONNECT APIs ****************/

/**
 * cm_disconnect_start() - Initiate the disconnect process
 * @cm_ctx: connection manager context
 * @req: Disconnect request.
 *
 * Return: QDF status
 */
QDF_STATUS cm_disconnect_start(struct cnx_mgr *cm_ctx,
			       struct cm_disconnect_req *req);

/**
 * cm_disconnect_active() - This API would be called after the disconnect
 * request gets activated in serialization.
 * @cm_ctx: connection manager context
 * @cm_id: Connection mgr ID assigned to this connect request.
 *
 * Return: QDF status
 */
QDF_STATUS cm_disconnect_active(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id);

/**
 * cm_disconnect_complete() - This API would be called after disconnect complete
 * request from the serialization.
 * @cm_ctx: connection manager context
 * @resp: disconnection complete resp.
 *
 * This API would be called after connection completion resp from VDEV mgr
 *
 * Return: QDF status
 */
QDF_STATUS cm_disconnect_complete(struct cnx_mgr *cm_ctx,
				  struct wlan_cm_discon_rsp *resp);

/**
 * cm_add_disconnect_req_to_list() - add disconnect req to the connection
 * manager req list
 * @vdev: vdev on which connect is received
 * @req: Disconnection req provided
 *
 * Return: QDF status
 */
QDF_STATUS cm_add_disconnect_req_to_list(struct cnx_mgr *cm_ctx,
					 struct cm_disconnect_req *req);

/**
 * cm_disconnect_start_req() - Disconnect start req from the requester
 * @vdev: vdev on which connect is received
 * @req: disconnection req provided
 *
 * Return: QDF status
 */
QDF_STATUS cm_disconnect_start_req(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_disconnect_req *req);

/*************** UTIL APIs ****************/

/**
 * cm_ser_get_blocking_cmd() - check if serialization command needs to be
 * blocking
 *
 * Return: bool
 */
#ifdef CONN_MGR_ADV_FEATURE
static inline bool cm_ser_get_blocking_cmd(void)
{
	return true;
}
#else
static inline bool cm_ser_get_blocking_cmd(void)
{
	return false;
}
#endif

/**
 * cm_get_cm_id() - Get unique cm id for connect/disconnect request
 * @cm_ctx: connection manager context
 * @source: source of the request (can be connect or disconnect request)
 *
 * Return: cm id
 */
wlan_cm_id cm_get_cm_id(struct cnx_mgr *cm_ctx, enum wlan_cm_source source);

struct cnx_mgr *cm_get_cm_ctx_fl(struct wlan_objmgr_vdev *vdev,
				 const char *func, uint32_t line);

/**
 * cm_get_cm_ctx() - Get connection manager context from vdev
 * @vdev: vdev object pointer
 *
 * Return: pointer to connection manager context
 */
#define cm_get_cm_ctx(vdev) \
	cm_get_cm_ctx_fl(vdev, __func__, __LINE__)

/**
 * cm_reset_active_cm_id() - Reset active cm_id from cm context, if its same as
 * passed cm_id
 * @vdev: vdev object pointer
 * @cm_id: cmid to match
 *
 * Return: void
 */
void cm_reset_active_cm_id(struct wlan_objmgr_vdev *vdev, wlan_cm_id cm_id);

/**
 * cm_check_cmid_match_list_head() - check if list head command matches the
 * given cm_id
 * @cm_ctx: connection manager context
 * @cm_id: cm id of connect/disconnect req
 *
 * Check if front req command matches the given
 * cm_id, this can be used to check if the latest (head) is same we are
 * trying to processing
 *
 * Return: true if match else false
 */
bool cm_check_cmid_match_list_head(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id);

/**
 * cm_check_scanid_match_list_head() - check if list head command matches the
 * given scan_id
 * @cm_ctx: connection manager context
 * @scan_id: scan_id of connect req
 *
 * Check if front req command is connect command and matches the given
 * scan_id, this can be used to check if the latest (head) is same we are
 * trying to processing
 *
 * Return: true if match else false
 */
bool cm_check_scanid_match_list_head(struct cnx_mgr *cm_ctx,
				     wlan_scan_id *scan_id);

/**
 * cm_free_connect_req_mem() - free connect req internal memory, to be called
 * before cm_req is freed
 * @connect_req: connect req
 *
 * Return: void
 */
void cm_free_connect_req_mem(struct cm_connect_req *connect_req);

/**
 * cm_delete_req_from_list() - Delete the request matching cm id
 * @cm_ctx: connection manager context
 * @cm_id: cm id of connect/disconnect req
 *
 * Context: Can be called from any context.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_delete_req_from_list(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id);

/**
 * cm_fill_bss_info_in_connect_rsp_by_cm_id() - fill bss info for the cm id
 * @cm_ctx: connection manager context
 * @cm_id: cm id of connect/disconnect req
 * @resp: resp to copy bss info like ssid/bssid and freq
 *
 * Fill the SSID form the connect req.
 * Fill freq and bssid from current candidate if available (i.e the connection
 * has tried to connect to a candidate), else get the bssid from req bssid or
 * bssid hint which ever is present.
 *
 * Return: Success if entry was found else failure
 */
QDF_STATUS
cm_fill_bss_info_in_connect_rsp_by_cm_id(struct cnx_mgr *cm_ctx,
					 wlan_cm_id cm_id,
					 struct wlan_cm_connect_rsp *resp);

/**
 * cm_remove_cmd() - Remove cmd from req list and serialization
 * @cm_ctx: connection manager context
 * @cm_id: cm id of connect/disconnect req
 *
 * Return: void
 */
void cm_remove_cmd(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id);

/**
 * cm_add_req_to_list_and_indicate_osif() - Add the request to request list in
 * cm ctx and indicate same to osif
 * @cm_ctx: connection manager context
 * @cm_req: cm request
 * @source: source of request
 *
 * Context: Can be called from any context.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_add_req_to_list_and_indicate_osif(struct cnx_mgr *cm_ctx,
						struct cm_req *cm_req,
						enum wlan_cm_source source);

struct cm_req *cm_get_req_by_cm_id_fl(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id,
				      const char *func, uint32_t line);

/**
 * cm_get_req_by_cm_id() - Get cm req matching the cm id
 * @cm_ctx: connection manager context
 * @cm_id: cm id of connect/disconnect req
 *
 * Context: Can be called from any context and to be used only after posting a
 * msg to SM (ie holding the SM lock) to avoid use after free. also returned req
 * should only be used till SM lock is hold.
 *
 * Return: cm req from the req list whose cm id matches the argument
 */
#define cm_get_req_by_cm_id(cm_ctx, cm_id) \
	cm_get_req_by_cm_id_fl(cm_ctx, cm_id, __func__, __LINE__)

/**
 * cm_vdev_scan_cancel() - cancel all scans for vdev
 * @pdev: pdev pointer
 * @vdev: vdev for which scan to be canceled
 *
 * Return: void
 */
void cm_vdev_scan_cancel(struct wlan_objmgr_pdev *pdev,
			 struct wlan_objmgr_vdev *vdev);

/**
 * cm_set_max_connect_attempts() - Set max connect attempts
 * @vdev: vdev pointer
 * @max_connect_attempts: max connect attempts to be set.
 *
 * Set max connect attempts. Max value is limited to CM_MAX_CONNECT_ATTEMPTS.
 *
 * Return: void
 */
void cm_set_max_connect_attempts(struct wlan_objmgr_vdev *vdev,
				 uint8_t max_connect_attempts);

/**
 * cm_is_vdev_connecting() - check if vdev is in conneting state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool cm_is_vdev_connecting(struct wlan_objmgr_vdev *vdev);

/**
 * cm_is_vdev_connected() - check if vdev is in conneted state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool cm_is_vdev_connected(struct wlan_objmgr_vdev *vdev);

/**
 * cm_is_vdev_disconnecting() - check if vdev is in disconneting state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool cm_is_vdev_disconnecting(struct wlan_objmgr_vdev *vdev);

/**
 * cm_is_vdev_disconnected() - check if vdev is disconnected/init state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool cm_is_vdev_disconnected(struct wlan_objmgr_vdev *vdev);

/**
 * cm_is_vdev_roaming() - check if vdev is in roaming state
 * @vdev: vdev pointer
 *
 * Return: bool
 */
bool cm_is_vdev_roaming(struct wlan_objmgr_vdev *vdev);

#endif /* __WLAN_CM_MAIN_API_H__ */
