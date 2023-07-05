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

#include "wlan_cm_main.h"
#include "wlan_cm_roam_sm.h"
#include "wlan_cm_sm.h"
#include <include/wlan_mlme_cmn.h>
#include "wlan_cm_main_api.h"
#include <wlan_scan_api.h>
#include <wlan_serialization_api.h>
#include <wlan_utility.h>
#include <wlan_cm_api.h>
#ifdef WLAN_POLICY_MGR_ENABLE
#include "wlan_policy_mgr_api.h"
#endif

static void
cm_fill_roam_fail_resp_from_cm_id(struct cnx_mgr *cm_ctx,
				  struct wlan_cm_connect_resp *resp,
				  wlan_cm_id cm_id,
				  enum wlan_cm_connect_fail_reason reason)
{
	resp->connect_status = QDF_STATUS_E_FAILURE;
	resp->cm_id = cm_id;
	resp->vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	resp->reason = reason;
	cm_fill_bss_info_in_roam_rsp_by_cm_id(cm_ctx, cm_id, resp);
}

static QDF_STATUS
cm_reassoc_fail_disconnect(struct wlan_objmgr_vdev *vdev,
			   enum wlan_cm_source source,
			   enum wlan_reason_code reason_code,
			   struct qdf_mac_addr *bssid)
{
	struct cnx_mgr *cm_ctx;
	struct cm_req *cm_req;
	struct cm_disconnect_req *disconnect_req;
	struct wlan_cm_disconnect_req req = {0};
	QDF_STATUS status;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_INVAL;

	/*
	 * This would be freed as part of removal from cm req list if adding
	 * to list is success after posting WLAN_CM_SM_EV_DISCONNECT_REQ.
	 */
	cm_req = qdf_mem_malloc(sizeof(*cm_req));
	if (!cm_req)
		return QDF_STATUS_E_NOMEM;

	req.vdev_id = wlan_vdev_get_id(vdev);
	req.source = source;
	req.reason_code = reason_code;
	if (bssid)
		qdf_copy_macaddr(&req.bssid, bssid);

	disconnect_req = &cm_req->discon_req;
	disconnect_req->req = req;

	status = cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_DISCONNECT_REQ,
					  sizeof(*disconnect_req),
					  disconnect_req);
	if (QDF_IS_STATUS_ERROR(status))
		qdf_mem_free(cm_req);

	return status;
}

QDF_STATUS
cm_send_reassoc_start_fail(struct cnx_mgr *cm_ctx,
			   wlan_cm_id cm_id,
			   enum wlan_cm_connect_fail_reason reason,
			   bool sync)
{
	struct wlan_cm_connect_resp *resp;
	QDF_STATUS status;

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return QDF_STATUS_E_NOMEM;

	cm_fill_roam_fail_resp_from_cm_id(cm_ctx, resp, cm_id, reason);
	if (sync)
		status = cm_sm_deliver_event_sync(
				cm_ctx, WLAN_CM_SM_EV_REASSOC_FAILURE,
				sizeof(*resp), resp);
	else
		status = cm_sm_deliver_event(cm_ctx->vdev,
					     WLAN_CM_SM_EV_REASSOC_FAILURE,
					     sizeof(*resp), resp);

	if (QDF_IS_STATUS_ERROR(status))
		cm_reassoc_complete(cm_ctx, resp);

	qdf_mem_free(resp);

	return status;
}

#ifdef CONN_MGR_ADV_FEATURE
static QDF_STATUS
cm_update_roam_scan_filter(
		struct wlan_objmgr_vdev *vdev, struct cm_roam_req *cm_req,
		struct scan_filter *filter, bool security_valid_for_6ghz)
{
	return cm_update_advance_roam_scan_filter(vdev, filter);
}
#else
static QDF_STATUS
cm_update_roam_scan_filter(
		struct wlan_objmgr_vdev *vdev, struct cm_roam_req *cm_req,
		struct scan_filter *filter, bool security_valid_for_6ghz)
{
	uint16_t rsn_caps;

	filter->num_of_ssid = 1;
	wlan_vdev_mlme_get_ssid(vdev, filter->ssid_list[0].ssid,
				&filter->ssid_list[0].length);

	if (cm_req->req.chan_freq) {
		filter->num_of_channels = 1;
		filter->chan_freq_list[0] = cm_req->req.chan_freq;
	}

	/* Security is not valid for 6Ghz so ignore 6Ghz APs */
	if (!security_valid_for_6ghz)
		filter->ignore_6ghz_channel = true;

	if (!QDF_HAS_PARAM(filter->authmodeset, WLAN_CRYPTO_AUTH_WAPI) &&
	    !QDF_HAS_PARAM(filter->authmodeset, WLAN_CRYPTO_AUTH_RSNA) &&
	    !QDF_HAS_PARAM(filter->authmodeset, WLAN_CRYPTO_AUTH_WPA)) {
		filter->ignore_auth_enc_type = 1;
	}

	rsn_caps =
		wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_RSN_CAP);

	if (rsn_caps & WLAN_CRYPTO_RSN_CAP_MFP_REQUIRED)
		filter->pmf_cap = WLAN_PMF_REQUIRED;
	else if (rsn_caps & WLAN_CRYPTO_RSN_CAP_MFP_ENABLED)
		filter->pmf_cap = WLAN_PMF_CAPABLE;
	else
		filter->pmf_cap = WLAN_PMF_DISABLED;
	return QDF_STATUS_SUCCESS;
}
#endif

static QDF_STATUS cm_connect_prepare_scan_filter_for_roam(
		struct cnx_mgr *cm_ctx, struct cm_roam_req *cm_req,
		struct scan_filter *filter, bool security_valid_for_6ghz)
{
	struct wlan_objmgr_vdev *vdev = cm_ctx->vdev;

	if (!qdf_is_macaddr_zero(&cm_req->req.bssid)) {
		filter->num_of_bssid = 1;
		qdf_copy_macaddr(&filter->bssid_list[0], &cm_req->req.bssid);
	}

	filter->authmodeset =
		wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_AUTH_MODE);

	filter->ucastcipherset =
		wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_UCAST_CIPHER);

	filter->mcastcipherset =
		wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_MCAST_CIPHER);

	filter->key_mgmt =
		wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_KEY_MGMT);

	filter->mgmtcipherset =
		wlan_crypto_get_param(vdev, WLAN_CRYPTO_PARAM_MGMT_CIPHER);

	return cm_update_roam_scan_filter(vdev, cm_req, filter,
					  security_valid_for_6ghz);
}

static QDF_STATUS cm_roam_get_candidates(struct wlan_objmgr_pdev *pdev,
					 struct cnx_mgr *cm_ctx,
					 struct cm_roam_req *cm_req)
{
	struct scan_filter *filter;
	uint32_t num_bss = 0;
	enum QDF_OPMODE op_mode;
	qdf_list_t *candidate_list;
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	qdf_list_node_t *cur_node = NULL;
	bool security_valid_for_6ghz = true;

	filter = qdf_mem_malloc(sizeof(*filter));
	if (!filter)
		return QDF_STATUS_E_NOMEM;

	cm_connect_prepare_scan_filter_for_roam(cm_ctx, cm_req, filter,
						security_valid_for_6ghz);

	candidate_list = wlan_scan_get_result(pdev, filter);
	if (candidate_list) {
		num_bss = qdf_list_size(candidate_list);
		mlme_debug(CM_PREFIX_FMT "num_entries found %d",
			   CM_PREFIX_REF(vdev_id, cm_req->cm_id), num_bss);
	}

	op_mode = wlan_vdev_mlme_get_opmode(cm_ctx->vdev);
	if (num_bss && op_mode == QDF_STA_MODE)
		cm_calculate_scores(cm_ctx, pdev, filter, candidate_list);

	qdf_mem_free(filter);

	if (!candidate_list || !qdf_list_size(candidate_list)) {
		if (candidate_list)
			wlan_scan_purge_results(candidate_list);

		mlme_info(CM_PREFIX_FMT "no valid candidate found, num_bss %d",
			  CM_PREFIX_REF(vdev_id, cm_req->cm_id), num_bss);
		cm_req->candidate_list = NULL;
		return QDF_STATUS_E_EMPTY;
	}

	qdf_list_peek_front(candidate_list, &cur_node);
	cm_req->candidate_list = candidate_list;
	cm_req->cur_candidate = qdf_container_of(cur_node,
						 struct scan_cache_node,
						 node);
	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_PREAUTH_ENABLE
QDF_STATUS cm_handle_reassoc_timer(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	struct cm_req *cm_req;

	if (!cm_id)
		return QDF_STATUS_E_INVAL;

	cm_req = cm_get_req_by_cm_id(cm_ctx, *cm_id);
	if (!cm_req)
		return QDF_STATUS_E_INVAL;

	return cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_START_REASSOC,
					sizeof(cm_req->roam_req),
					&cm_req->roam_req);
}

static QDF_STATUS cm_host_roam_start(struct cnx_mgr *cm_ctx,
				     struct cm_req *cm_req)
{
	struct wlan_cm_roam_req *req;
	struct qdf_mac_addr connected_bssid;

	req = &cm_req->roam_req.req;

	wlan_vdev_get_bss_peer_mac(cm_ctx->vdev, &connected_bssid);
	if (qdf_is_macaddr_equal(&req->bssid, &connected_bssid)) {
		mlme_info(CM_PREFIX_FMT "Self reassoc with" QDF_MAC_ADDR_FMT,
			  CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
					cm_req->cm_id),
			  QDF_MAC_ADDR_REF(req->bssid.bytes));
		req->self_reassoc = true;
	}

	/* if self reassoc continue with reassoc and skip preauth */
	if (req->self_reassoc)
		return cm_sm_deliver_event_sync(cm_ctx,
						WLAN_CM_SM_EV_START_REASSOC,
						sizeof(cm_req->roam_req),
						&cm_req->roam_req);
	/*
	 * if not self reassoc reset cur candidate to perform preauth with
	 * all candidate.
	 */
	cm_req->roam_req.cur_candidate = NULL;
	return cm_host_roam_preauth_start(cm_ctx, cm_req);
}

static
QDF_STATUS cm_host_roam_start_fail(struct cnx_mgr *cm_ctx,
				   struct cm_req *cm_req,
				   enum wlan_cm_connect_fail_reason reason)
{
	cm_send_preauth_start_fail(cm_ctx, cm_req->cm_id, reason);

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS cm_host_roam_start(struct cnx_mgr *cm_ctx,
				     struct cm_req *cm_req)
{
	return cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_START_REASSOC,
					sizeof(cm_req->roam_req),
					&cm_req->roam_req);
}

static
QDF_STATUS cm_host_roam_start_fail(struct cnx_mgr *cm_ctx,
				   struct cm_req *cm_req,
				   enum wlan_cm_connect_fail_reason reason)
{
	return cm_send_reassoc_start_fail(cm_ctx, cm_req->cm_id, reason, true);
}
#endif

QDF_STATUS cm_host_roam_start_req(struct cnx_mgr *cm_ctx,
				  struct cm_req *cm_req)
{
	QDF_STATUS status;
	struct wlan_objmgr_pdev *pdev;
	enum wlan_cm_connect_fail_reason reason = CM_GENERIC_FAILURE;

	mlme_cm_roam_start_ind(cm_ctx->vdev, &cm_req->roam_req.req);

	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);
	if (!pdev) {
		reason = CM_GENERIC_FAILURE;
		goto roam_err;
	}

	status = cm_roam_get_candidates(pdev, cm_ctx, &cm_req->roam_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		reason = CM_NO_CANDIDATE_FOUND;
		goto roam_err;
	}

	status = cm_host_roam_start(cm_ctx, cm_req);
	if (QDF_IS_STATUS_SUCCESS(status))
		return status;

roam_err:
	return cm_host_roam_start_fail(cm_ctx, cm_req, reason);
}

bool cm_roam_resp_cmid_match_list_head(struct cnx_mgr *cm_ctx,
				       struct wlan_cm_connect_resp *resp)
{
	return cm_check_cmid_match_list_head(cm_ctx, &resp->cm_id);
}

QDF_STATUS cm_reassoc_active(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	struct cm_req *cm_req;
	struct wlan_cm_vdev_discon_req req;
	struct cm_disconnect_req *discon_req;

	cm_req = cm_get_req_by_cm_id(cm_ctx, *cm_id);
	if (!cm_req)
		return QDF_STATUS_E_INVAL;

	cm_ctx->active_cm_id = *cm_id;

	/* For self reassoc no need to disconnect or create peer */
	if (cm_req->roam_req.req.self_reassoc)
		return cm_resume_reassoc_after_peer_create(cm_ctx, cm_id);

	qdf_mem_zero(&req, sizeof(req));
	req.cm_id = *cm_id;
	req.req.vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	req.req.source = CM_ROAM_DISCONNECT;
	wlan_vdev_get_bss_peer_mac(cm_ctx->vdev, &req.req.bssid);

	discon_req = qdf_mem_malloc(sizeof(*discon_req));
	if (!discon_req)
		return QDF_STATUS_E_NOMEM;

	discon_req->cm_id = *cm_id;
	discon_req->req.vdev_id = req.req.vdev_id;
	qdf_copy_macaddr(&discon_req->req.bssid,
			 &req.req.bssid);
	cm_update_scan_mlme_on_disconnect(cm_ctx->vdev, discon_req);
	qdf_mem_free(discon_req);

	return mlme_cm_disconnect_req(cm_ctx->vdev, &req);
}

QDF_STATUS cm_reassoc_disconnect_complete(struct cnx_mgr *cm_ctx,
					  struct wlan_cm_discon_rsp *resp)
{
	QDF_STATUS status;
	struct cm_req *cm_req;
	struct qdf_mac_addr *bssid;
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	wlan_cm_id cm_id = cm_ctx->active_cm_id;

	cm_req = cm_get_req_by_cm_id(cm_ctx, cm_id);
	if (!cm_req)
		return QDF_STATUS_E_INVAL;

	mlme_cm_disconnect_complete_ind(cm_ctx->vdev, resp);
	bssid = &cm_req->roam_req.cur_candidate->entry->bssid;

	status = mlme_cm_bss_peer_create_req(cm_ctx->vdev, bssid, NULL, false);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "Peer create request failed",
			 CM_PREFIX_REF(vdev_id, cm_id));
		status = cm_send_reassoc_start_fail(cm_ctx, cm_id,
						    CM_PEER_CREATE_FAILED,
						    true);
	}

	return status;
}

QDF_STATUS
cm_resume_reassoc_after_peer_create(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	struct wlan_cm_vdev_reassoc_req *req;
	struct cm_req *cm_req;
	QDF_STATUS status;

	cm_req = cm_get_req_by_cm_id(cm_ctx, *cm_id);
	if (!cm_req)
		return QDF_STATUS_E_FAILURE;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return QDF_STATUS_E_NOMEM;

	req->vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	req->cm_id = *cm_id;
	req->self_reassoc = cm_req->roam_req.req.self_reassoc;
	req->bss = cm_req->roam_req.cur_candidate;

	mlme_nofl_info(CM_PREFIX_FMT "Reassoc to %.*s " QDF_MAC_ADDR_FMT " rssi: %d freq: %d source %d",
		       CM_PREFIX_REF(req->vdev_id, req->cm_id),
		       req->bss->entry->ssid.length,
		       req->bss->entry->ssid.ssid,
		       QDF_MAC_ADDR_REF(req->bss->entry->bssid.bytes),
		       req->bss->entry->rssi_raw,
		       req->bss->entry->channel.chan_freq,
		       cm_req->roam_req.req.source);

	status = mlme_cm_reassoc_req(cm_ctx->vdev, req);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "Reassoc request failed",
			 CM_PREFIX_REF(req->vdev_id, req->cm_id));
		/* Delete peer only if not self reassoc */
		if (!cm_req->roam_req.req.self_reassoc)
			mlme_cm_bss_peer_delete_req(cm_ctx->vdev);
		status = cm_send_reassoc_start_fail(cm_ctx, *cm_id,
						    CM_JOIN_FAILED, true);
	}

	qdf_mem_free(req);
	return status;
}

QDF_STATUS cm_reassoc_complete(struct cnx_mgr *cm_ctx,
			       struct wlan_cm_connect_resp *resp)
{
	/*
	 * If the entry is not present in the list, it must have been cleared
	 * already.
	 */
	if (!cm_get_req_by_cm_id(cm_ctx, resp->cm_id))
		return QDF_STATUS_SUCCESS;

	resp->is_reassoc = true;
	cm_connect_complete(cm_ctx, resp);
	/*
	 * If roaming fails and conn_sm is in ROAMING state, then
	 * initiate disconnect to cleanup and move conn_sm to INIT state
	 */
	if (QDF_IS_STATUS_ERROR(resp->connect_status) &&
	    cm_get_state(cm_ctx) == WLAN_CM_S_ROAMING) {
		cm_reassoc_fail_disconnect(cm_ctx->vdev, CM_ROAM_DISCONNECT,
					   REASON_UNSPEC_FAILURE,
					   &resp->bssid);
	}

	return QDF_STATUS_SUCCESS;
}

static void
cm_reassoc_handle_event_post_fail(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
{
	struct wlan_cm_connect_resp *resp;

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return;

	cm_fill_roam_fail_resp_from_cm_id(cm_ctx, resp, cm_id,
					  CM_GENERIC_FAILURE);
	cm_reassoc_complete(cm_ctx, resp);
	qdf_mem_free(resp);
}

static QDF_STATUS cm_reassoc_cmd_timeout(struct cnx_mgr *cm_ctx,
					 wlan_cm_id cm_id)
{
	struct wlan_cm_connect_resp *resp;
	QDF_STATUS status;

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return QDF_STATUS_E_NOMEM;

	cm_fill_roam_fail_resp_from_cm_id(cm_ctx, resp, cm_id, CM_SER_TIMEOUT);
	status = cm_sm_deliver_event(cm_ctx->vdev,
				     WLAN_CM_SM_EV_REASSOC_FAILURE,
				     sizeof(*resp), resp);
	if (QDF_IS_STATUS_ERROR(status))
		cm_reassoc_complete(cm_ctx, resp);

	qdf_mem_free(resp);

	return status;
}

#ifdef WLAN_CM_USE_SPINLOCK
static QDF_STATUS cm_activate_reassoc_req_sched_cb(struct scheduler_msg *msg)
{
	struct wlan_serialization_command *cmd = msg->bodyptr;
	struct wlan_objmgr_vdev *vdev;
	struct cnx_mgr *cm_ctx;
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;

	if (!cmd || !cmd->vdev) {
		mlme_err("Invalid Input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = cmd->vdev;
	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_INVAL;

	ret = cm_sm_deliver_event(vdev,
				  WLAN_CM_SM_EV_REASSOC_ACTIVE,
				  sizeof(wlan_cm_id),
				  &cmd->cmd_id);
	/*
	 * Called from scheduler context hence posting failure
	 */
	if (QDF_IS_STATUS_ERROR(ret))
		cm_reassoc_handle_event_post_fail(cm_ctx, cmd->cmd_id);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
	return ret;
}

static QDF_STATUS
cm_activate_reassoc_req(struct wlan_serialization_command *cmd)
{
	struct wlan_objmgr_vdev *vdev = cmd->vdev;
	struct scheduler_msg msg = {0};
	QDF_STATUS ret;

	msg.bodyptr = cmd;
	msg.callback = cm_activate_reassoc_req_sched_cb;
	msg.flush_callback = cm_activate_cmd_req_flush_cb;

	ret = wlan_objmgr_vdev_try_get_ref(vdev, WLAN_MLME_CM_ID);
	if (QDF_IS_STATUS_ERROR(ret))
		return ret;

	ret = scheduler_post_message(QDF_MODULE_ID_MLME,
				     QDF_MODULE_ID_MLME,
				     QDF_MODULE_ID_MLME, &msg);

	if (QDF_IS_STATUS_ERROR(ret)) {
		mlme_err(CM_PREFIX_FMT "Failed to post scheduler_msg",
			 CM_PREFIX_REF(wlan_vdev_get_id(vdev), cmd->cmd_id));
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return ret;
	}

	return ret;
}
#else
static QDF_STATUS
cm_activate_reassoc_req(struct wlan_serialization_command *cmd)
{
	return cm_sm_deliver_event(cmd->vdev,
				   WLAN_CM_SM_EV_REASSOC_ACTIVE,
				   sizeof(wlan_cm_id),
				   &cmd->cmd_id);
}
#endif

static QDF_STATUS
cm_ser_reassoc_cb(struct wlan_serialization_command *cmd,
		  enum wlan_serialization_cb_reason reason)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_vdev *vdev;
	struct cnx_mgr *cm_ctx;

	if (!cmd) {
		mlme_err("cmd is NULL, reason: %d", reason);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	vdev = cmd->vdev;
	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_NULL_VALUE;

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		/*
		 * For pending to active reason, use async api to take lock.
		 * For direct activation use sync api to avoid taking lock
		 * as lock is already acquired by the requester.
		 */
		if (cmd->activation_reason == SER_PENDING_TO_ACTIVE)
			status = cm_activate_reassoc_req(cmd);
		else
			status = cm_sm_deliver_event_sync(
					cm_ctx, WLAN_CM_SM_EV_REASSOC_ACTIVE,
					sizeof(wlan_cm_id), &cmd->cmd_id);

		if (QDF_IS_STATUS_SUCCESS(status))
			break;
		/*
		 * Handle failure if posting fails, i.e. the SM state has
		 * changed or head cm_id doesn't match the active cm_id.
		 * connect active should be handled only in JOIN_PENDING. If
		 * new command has been received connect activation should be
		 * aborted from here with connect req cleanup.
		 */
		cm_reassoc_handle_event_post_fail(cm_ctx, cmd->cmd_id);
		break;
	case WLAN_SER_CB_CANCEL_CMD:
		/* command removed from pending list. */
		break;
	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		mlme_err(CM_PREFIX_FMT "Active command timeout",
			 CM_PREFIX_REF(wlan_vdev_get_id(vdev), cmd->cmd_id));
		cm_trigger_panic_on_cmd_timeout(cm_ctx->vdev);
		cm_reassoc_cmd_timeout(cm_ctx, cmd->cmd_id);
		break;
	case WLAN_SER_CB_RELEASE_MEM_CMD:
		cm_reset_active_cm_id(vdev, cmd->cmd_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		break;
	default:
		QDF_ASSERT(0);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
}

#define REASSOC_TIMEOUT	10000
static QDF_STATUS cm_ser_reassoc_req(struct cnx_mgr *cm_ctx,
				     struct cm_roam_req *cm_req)
{
	struct wlan_serialization_command cmd = {0, };
	enum wlan_serialization_status ser_cmd_status;
	QDF_STATUS status;
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);

	mlme_cm_osif_roam_sync_ind(cm_ctx->vdev);

	status = wlan_objmgr_vdev_try_get_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "unable to get reference",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id));
		return status;
	}

	cmd.cmd_type = WLAN_SER_CMD_VDEV_ROAM;
	cmd.cmd_id = cm_req->cm_id;
	cmd.cmd_cb = cm_ser_reassoc_cb;
	cmd.source = WLAN_UMAC_COMP_MLME;
	cmd.is_high_priority = false;
	cmd.cmd_timeout_duration = REASSOC_TIMEOUT;
	cmd.vdev = cm_ctx->vdev;
	cmd.is_blocking = cm_ser_get_blocking_cmd();

	ser_cmd_status = wlan_serialization_request(&cmd);
	switch (ser_cmd_status) {
	case WLAN_SER_CMD_PENDING:
		/* command moved to pending list.Do nothing */
		break;
	case WLAN_SER_CMD_ACTIVE:
		/* command moved to active list. Do nothing */
		break;
	default:
		mlme_err(CM_PREFIX_FMT "ser cmd status %d",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id), ser_cmd_status);
		wlan_objmgr_vdev_release_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);

		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_POLICY_MGR_ENABLE
QDF_STATUS
cm_handle_reassoc_hw_mode_change(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id,
				 enum wlan_cm_sm_evt event)
{
	struct cm_req *cm_req;
	enum wlan_cm_connect_fail_reason reason = CM_GENERIC_FAILURE;
	struct wlan_objmgr_pdev *pdev;
	QDF_STATUS status;

	if (!cm_id)
		return QDF_STATUS_E_FAILURE;

	cm_req = cm_get_req_by_cm_id(cm_ctx, *cm_id);
	if (!cm_req)
		return QDF_STATUS_E_INVAL;

	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "Failed to find pdev",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
				       cm_req->cm_id));
		goto send_failure;
	}

	if (event == WLAN_CM_SM_EV_HW_MODE_SUCCESS) {
		status = cm_ser_reassoc_req(cm_ctx, &cm_req->roam_req);
		if (QDF_IS_STATUS_ERROR(status)) {
			reason = CM_SER_FAILURE;
			goto send_failure;
		}
		return status;
	}

	/* Set reason HW mode fail for event WLAN_CM_SM_EV_HW_MODE_FAILURE */
	reason = CM_HW_MODE_FAILURE;

send_failure:
	return cm_send_reassoc_start_fail(cm_ctx, cm_req->cm_id, reason, true);
}

void cm_reassoc_hw_mode_change_resp(struct wlan_objmgr_pdev *pdev,
				    uint8_t vdev_id, wlan_cm_id cm_id,
				    QDF_STATUS status)
{
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS qdf_status;
	enum wlan_cm_sm_evt event = WLAN_CM_SM_EV_HW_MODE_SUCCESS;
	struct cnx_mgr *cm_ctx;

	mlme_debug(CM_PREFIX_FMT "Continue Reassoc after HW mode change, status %d",
		   CM_PREFIX_REF(vdev_id, cm_id), status);

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev)
		return;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return;
	}

	if (QDF_IS_STATUS_ERROR(status))
		event = WLAN_CM_SM_EV_HW_MODE_FAILURE;
	qdf_status = cm_sm_deliver_event(vdev, event, sizeof(wlan_cm_id),
					 &cm_id);

	/*
	 * Handle failure if posting fails, i.e. the SM state has
	 * changed or head cm_id doesn't match the active cm_id.
	 * hw mode change resp should be handled in REASSOC state. If
	 * new command has been received reassoc should be
	 * aborted from here with reassoc req cleanup.
	 */
	if (QDF_IS_STATUS_ERROR(status))
		cm_reassoc_handle_event_post_fail(cm_ctx, cm_id);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

static QDF_STATUS
cm_check_for_reassoc_hw_mode_change(struct cnx_mgr *cm_ctx,
				    struct cm_roam_req *cm_req)
{
	qdf_freq_t candidate_freq;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	psoc = wlan_vdev_get_psoc(cm_ctx->vdev);
	if (!psoc)
		return QDF_STATUS_E_INVAL;

	if (!cm_req->cur_candidate)
		return QDF_STATUS_E_EMPTY;

	/* HW mode change not required for self reassoc */
	if (cm_req->req.self_reassoc)
		return QDF_STATUS_E_ALREADY;

	candidate_freq = cm_req->cur_candidate->entry->channel.chan_freq;
	status = policy_mgr_handle_conc_multiport(
			psoc, cm_req->req.vdev_id,
			candidate_freq, POLICY_MGR_UPDATE_REASON_LFR2_ROAM,
			cm_req->cm_id);
	if (status == QDF_STATUS_E_NOSUPPORT)
		status = QDF_STATUS_E_ALREADY;

	return status;
}
#else
static inline QDF_STATUS
cm_check_for_reassoc_hw_mode_change(struct cnx_mgr *cm_ctx,
				    struct cm_roam_req *cm_req)
{
	return QDF_STATUS_E_ALREADY;
}
#endif

QDF_STATUS cm_reassoc_start(struct cnx_mgr *cm_ctx,
			    struct cm_roam_req *cm_req)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	enum wlan_cm_connect_fail_reason reason = CM_GENERIC_FAILURE;

	status = cm_check_for_reassoc_hw_mode_change(cm_ctx, cm_req);
	if (QDF_IS_STATUS_ERROR(status) && status != QDF_STATUS_E_ALREADY) {
		reason = CM_HW_MODE_FAILURE;
		mlme_err(CM_PREFIX_FMT "Failed to set HW mode change status %d",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id), status);
		goto err;
	} else if (QDF_IS_STATUS_SUCCESS(status)) {
		mlme_debug(CM_PREFIX_FMT "Reassoc will continue after HW mode change",
			   CM_PREFIX_REF(vdev_id, cm_req->cm_id));
		return QDF_STATUS_SUCCESS;
	}

	status = cm_ser_reassoc_req(cm_ctx, cm_req);
	if (QDF_IS_STATUS_SUCCESS(status))
		return status;

	reason = CM_SER_FAILURE;
	mlme_err(CM_PREFIX_FMT "Serialization of reassoc failed",
		 CM_PREFIX_REF(vdev_id, cm_req->cm_id));
err:
	return cm_send_reassoc_start_fail(cm_ctx, cm_req->cm_id, reason, true);
}

QDF_STATUS cm_reassoc_rsp(struct wlan_objmgr_vdev *vdev,
			  struct wlan_cm_connect_resp *resp)
{
	struct cnx_mgr *cm_ctx;
	QDF_STATUS qdf_status;
	wlan_cm_id cm_id;
	uint32_t prefix;
	enum wlan_cm_sm_evt event;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_INVAL;

	cm_id = cm_ctx->active_cm_id;
	prefix = CM_ID_GET_PREFIX(cm_id);

	if (prefix != ROAM_REQ_PREFIX ||
	    cm_id != resp->cm_id) {
		mlme_err(CM_PREFIX_FMT " Active cm_id 0x%x is different",
			 CM_PREFIX_REF(wlan_vdev_get_id(vdev), resp->cm_id),
			 cm_id);
		qdf_status = QDF_STATUS_E_FAILURE;
		goto post_err;
	}

	if (QDF_IS_STATUS_SUCCESS(resp->connect_status)) {
		/*
		 * On successful connection to sae single pmk AP,
		 * clear all the single pmk AP.
		 */
		if (cm_is_cm_id_current_candidate_single_pmk(cm_ctx, cm_id))
			wlan_crypto_selective_clear_sae_single_pmk_entries(
					vdev, &resp->bssid);
		event = WLAN_CM_SM_EV_REASSOC_DONE;
	} else {
		event = WLAN_CM_SM_EV_REASSOC_FAILURE;
	}

	qdf_status = cm_sm_deliver_event(cm_ctx->vdev, event, sizeof(*resp),
					 resp);
	if (QDF_IS_STATUS_SUCCESS(qdf_status))
		return qdf_status;
post_err:
	cm_reassoc_complete(cm_ctx, resp);

	return qdf_status;
}

QDF_STATUS cm_roam_bss_peer_create_rsp(struct wlan_objmgr_vdev *vdev,
				       QDF_STATUS status,
				       struct qdf_mac_addr *peer_mac)
{
	struct cnx_mgr *cm_ctx;
	QDF_STATUS qdf_status;
	wlan_cm_id cm_id;
	uint32_t prefix;
	struct cm_req *cm_req;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_INVAL;

	cm_id = cm_ctx->active_cm_id;
	prefix = CM_ID_GET_PREFIX(cm_id);

	if (prefix != ROAM_REQ_PREFIX) {
		mlme_err(CM_PREFIX_FMT "Active req is not roam req",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id));
		mlme_cm_bss_peer_delete_req(vdev);
		return QDF_STATUS_E_INVAL;
	}

	if (QDF_IS_STATUS_ERROR(status)) {
		cm_req = cm_get_req_by_cm_id(cm_ctx, cm_id);
		if (!cm_req)
			return QDF_STATUS_E_INVAL;

		return cm_send_reassoc_start_fail(
				cm_ctx, cm_id,
				CM_PEER_CREATE_FAILED, false);
	}

	qdf_status = cm_sm_deliver_event(
			vdev, WLAN_CM_SM_EV_BSS_CREATE_PEER_SUCCESS,
			sizeof(wlan_cm_id), &cm_id);
	if (QDF_IS_STATUS_SUCCESS(qdf_status))
		return qdf_status;

	mlme_cm_bss_peer_delete_req(vdev);
	cm_reassoc_handle_event_post_fail(cm_ctx, cm_id);

	return qdf_status;
}

QDF_STATUS cm_roam_disconnect_rsp(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_discon_rsp *resp)
{
	struct cnx_mgr *cm_ctx;
	QDF_STATUS qdf_status;
	wlan_cm_id cm_id;
	uint32_t prefix;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_INVAL;

	cm_id = cm_ctx->active_cm_id;
	prefix = CM_ID_GET_PREFIX(cm_id);

	if (prefix != ROAM_REQ_PREFIX || cm_id != resp->req.cm_id) {
		mlme_err(CM_PREFIX_FMT "Active cm_id 0x%x is different",
			 CM_PREFIX_REF(wlan_vdev_get_id(vdev), resp->req.cm_id),
			 cm_id);
		qdf_status = QDF_STATUS_E_FAILURE;
		goto disconnect_complete;
	}
	qdf_status =
		cm_sm_deliver_event(vdev,
				    WLAN_CM_SM_EV_HO_ROAM_DISCONNECT_DONE,
				    sizeof(*resp), resp);
	if (QDF_IS_STATUS_SUCCESS(qdf_status))
		return qdf_status;

disconnect_complete:
	cm_reassoc_handle_event_post_fail(cm_ctx, cm_id);
	return qdf_status;
}
