/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cm_host_roam_preauth.c
 *
 * Implements general roam pre-auth functionality for connection manager
 */

#include "wlan_cm_vdev_api.h"
#include "wni_api.h"
#include <wlan_cm_api.h>
#include "wlan_cm_roam_api.h"
#include "wlan_cm_roam_public_struct.h"
#include "wlan_cm_public_struct.h"
#include "wlan_mlme_vdev_mgr_interface.h"
#include "connection_mgr/core/src/wlan_cm_roam.h"
#include "connection_mgr/core/src/wlan_cm_sm.h"
#include "connection_mgr/core/src/wlan_cm_main_api.h"

#define MAX_NUM_PREAUTH_RETRIES 3
#define CM_PREAUTH_TIMEOUT 10000
#define REASSOC_TIMER_DURATION 60

static QDF_STATUS cm_get_valid_preauth_candidate(struct cm_roam_req *cm_req)
{
	qdf_list_t *candidate_list;
	qdf_list_node_t *cur_node = NULL;
	struct scan_cache_node *prev_candidate;
	bool new_candidate = false;
	uint8_t vdev_id = cm_req->req.vdev_id;

	candidate_list = cm_req->candidate_list;
	if (!candidate_list || !qdf_list_size(candidate_list)) {
		mlme_info(CM_PREFIX_FMT "no valid candidate found",
			  CM_PREFIX_REF(vdev_id, cm_req->cm_id));
		return QDF_STATUS_E_EMPTY;
	}

	prev_candidate = cm_req->cur_candidate;
	if (!prev_candidate) {
		qdf_list_peek_front(candidate_list, &cur_node);
		new_candidate = true;
	} else if (cm_req->num_preauth_retry >= MAX_NUM_PREAUTH_RETRIES) {
		qdf_list_peek_next(candidate_list, &prev_candidate->node,
				   &cur_node);
		new_candidate = true;
	}

	if (new_candidate) {
		if (!cur_node) {
			mlme_debug(CM_PREFIX_FMT "All canidate tried",
				   CM_PREFIX_REF(vdev_id, cm_req->cm_id));
			return QDF_STATUS_E_FAILURE;
		}
		cm_req->num_preauth_retry = 0;
		cm_req->cur_candidate = qdf_container_of(cur_node,
							 struct scan_cache_node,
							 node);
	}

	cm_req->num_preauth_retry++;

	mlme_debug(CM_PREFIX_FMT "Try preauth attempt no. %d for " QDF_MAC_ADDR_FMT,
		   CM_PREFIX_REF(vdev_id, cm_req->cm_id),
		   cm_req->num_preauth_retry,
		   QDF_MAC_ADDR_REF(cm_req->cur_candidate->entry->bssid.bytes));

	return QDF_STATUS_SUCCESS;
}

void cm_preauth_fail(struct cnx_mgr *cm_ctx,
		     struct wlan_cm_preauth_fail *preauth_fail_rsp)
{
	struct cm_req *cm_req;
	wlan_cm_id cm_id;

	cm_id = preauth_fail_rsp->cm_id;
	cm_req = cm_get_req_by_cm_id(cm_ctx, cm_id);
	/*
	 * If the entry is not present in the list, it must have been cleared
	 * already.
	 */
	if (!cm_req)
		return;

	if (cm_get_state(cm_ctx) == WLAN_CM_S_CONNECTED)
		cm_mlme_roam_preauth_fail(cm_ctx->vdev, &cm_req->roam_req.req,
					  preauth_fail_rsp->reason);

	mlme_debug(CM_PREFIX_FMT,
		   CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id));

	if (cm_req->roam_req.req.source == CM_ROAMING_HOST &&
	    cm_get_state(cm_ctx) == WLAN_CM_S_CONNECTED)
		wlan_cm_disconnect(cm_ctx->vdev, CM_ROAM_DISCONNECT,
				   REASON_USER_TRIGGERED_ROAM_FAILURE, NULL);

	cm_remove_cmd(cm_ctx, &cm_id);
}

static void
cm_preauth_handle_event_post_fail(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
{
	struct wlan_cm_preauth_fail preauth_fail_rsp;

	preauth_fail_rsp.cm_id = cm_id;
	preauth_fail_rsp.reason = CM_ABORT_DUE_TO_NEW_REQ_RECVD;

	cm_preauth_fail(cm_ctx, &preauth_fail_rsp);
}

static QDF_STATUS
cm_preauth_cmd_timeout(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
{
	QDF_STATUS status;
	struct wlan_cm_preauth_fail preauth_fail_rsp;

	preauth_fail_rsp.cm_id = cm_id;
	preauth_fail_rsp.reason = CM_SER_TIMEOUT;

	status = cm_sm_deliver_event(
			cm_ctx->vdev, WLAN_CM_SM_EV_PREAUTH_FAIL,
			sizeof(struct wlan_cm_preauth_fail), &preauth_fail_rsp);
	if (QDF_IS_STATUS_ERROR(status))
		cm_preauth_handle_event_post_fail(cm_ctx, cm_id);

	return status;
}

static QDF_STATUS
cm_ser_preauth_cb(struct wlan_serialization_command *cmd,
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
		if (cmd->activation_reason == SER_PENDING_TO_ACTIVE)
			status = cm_sm_deliver_event(
					vdev, WLAN_CM_SM_EV_PREAUTH_ACTIVE,
					sizeof(wlan_cm_id), &cmd->cmd_id);
		else
			status = cm_sm_deliver_event_sync(
					cm_ctx, WLAN_CM_SM_EV_PREAUTH_ACTIVE,
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
		cm_preauth_handle_event_post_fail(cm_ctx, cmd->cmd_id);
		break;
	case WLAN_SER_CB_CANCEL_CMD:
		/* command removed from pending list. */
		break;
	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		mlme_err(CM_PREFIX_FMT "Active command timeout",
			 CM_PREFIX_REF(wlan_vdev_get_id(vdev), cmd->cmd_id));
		QDF_ASSERT(0);

		cm_preauth_cmd_timeout(cm_ctx, cmd->cmd_id);
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

static QDF_STATUS cm_ser_preauth_req(struct cnx_mgr *cm_ctx,
				     struct cm_req *cm_req)
{
	struct wlan_serialization_command cmd = {0, };
	enum wlan_serialization_status ser_cmd_status;
	QDF_STATUS status;
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);

	status = wlan_objmgr_vdev_try_get_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "unable to get reference",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id));
		return status;
	}

	cmd.cmd_type = WLAN_SER_CMD_PERFORM_PRE_AUTH;
	cmd.cmd_id = cm_req->cm_id;
	cmd.cmd_cb = cm_ser_preauth_cb;
	cmd.source = WLAN_UMAC_COMP_MLME;
	cmd.is_high_priority = false;
	cmd.cmd_timeout_duration = CM_PREAUTH_TIMEOUT;
	cmd.vdev = cm_ctx->vdev;
	cmd.is_blocking = true;

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

QDF_STATUS
cm_send_preauth_start_fail(struct cnx_mgr *cm_ctx,
			   wlan_cm_id cm_id,
			   enum wlan_cm_connect_fail_reason reason)
{
	QDF_STATUS status;
	struct wlan_cm_preauth_fail preauth_fail_rsp;

	preauth_fail_rsp.cm_id = cm_id;
	preauth_fail_rsp.reason = reason;

	status = cm_sm_deliver_event_sync(
			cm_ctx, WLAN_CM_SM_EV_PREAUTH_FAIL,
			sizeof(struct wlan_cm_preauth_fail), &preauth_fail_rsp);
	if (QDF_IS_STATUS_ERROR(status))
		cm_preauth_handle_event_post_fail(cm_ctx, cm_id);

	return status;
}

static void cm_flush_invalid_preauth_ap(struct cnx_mgr *cm_ctx,
					struct cm_roam_req *roam_req)
{
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	qdf_list_t *candidate_list;
	struct scan_cache_node *scan_node = NULL;
	struct qdf_mac_addr connected_bssid;
	bool is_valid;
	uint8_t vdev_id = roam_req->req.vdev_id;
	struct wlan_objmgr_psoc *psoc;
	uint8_t enable_mcc_mode = false;
	qdf_freq_t conc_freq, bss_freq;

	/*
	 * Only When entering first time (ie cur_candidate is NULL),
	 * flush invalid APs from the list and if list is not NULL.
	 */
	if (roam_req->cur_candidate || !roam_req->candidate_list)
		return;

	psoc = wlan_vdev_get_psoc(cm_ctx->vdev);
	if (!psoc)
		return;

	wlan_mlme_get_mcc_feature(psoc, &enable_mcc_mode);

	wlan_vdev_get_bss_peer_mac(cm_ctx->vdev, &connected_bssid);

	candidate_list = roam_req->candidate_list;

	qdf_list_peek_front(candidate_list, &cur_node);

	while (cur_node) {
		is_valid = true;
		qdf_list_peek_next(candidate_list, cur_node, &next_node);
		scan_node = qdf_container_of(cur_node, struct scan_cache_node,
					     node);
		bss_freq = scan_node->entry->channel.chan_freq;
		if (qdf_is_macaddr_equal(&connected_bssid,
					 &scan_node->entry->bssid)) {
			mlme_debug(CM_PREFIX_FMT "Remove connected AP " QDF_MAC_ADDR_FMT " from list",
				   CM_PREFIX_REF(vdev_id, roam_req->cm_id),
				   QDF_MAC_ADDR_REF(connected_bssid.bytes));
			is_valid = false;
		}

		/*
		 * Continue if MCC is disabled in INI and if AP
		 * will create MCC
		 */
		if (policy_mgr_concurrent_open_sessions_running(psoc) &&
		    !enable_mcc_mode) {
			conc_freq = wlan_get_conc_freq();
			if (conc_freq && (conc_freq != bss_freq)) {
				mlme_info(CM_PREFIX_FMT "Remove AP " QDF_MAC_ADDR_FMT ", MCC not supported. freq %d conc_freq %d",
					  CM_PREFIX_REF(vdev_id, roam_req->cm_id),
					  QDF_MAC_ADDR_REF(connected_bssid.bytes),
					  bss_freq, conc_freq);
				is_valid = false;
			}
		}

		if (!is_valid) {
			qdf_list_remove_node(candidate_list, cur_node);
			util_scan_free_cache_entry(scan_node->entry);
			qdf_mem_free(scan_node);
		}

		cur_node = next_node;
		next_node = NULL;
	}
}

QDF_STATUS cm_host_roam_preauth_start(struct cnx_mgr *cm_ctx,
				      struct cm_req *cm_req)
{
	QDF_STATUS status;

	cm_flush_invalid_preauth_ap(cm_ctx, &cm_req->roam_req);

	status = cm_get_valid_preauth_candidate(&cm_req->roam_req);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	return cm_ser_preauth_req(cm_ctx, cm_req);
}

void cm_free_preauth_req(struct wlan_preauth_req *preauth_req)
{
	if (!preauth_req)
		return;

	util_scan_free_cache_entry(preauth_req->entry);
	preauth_req->entry = NULL;
	qdf_mem_free(preauth_req);
}

static QDF_STATUS cm_flush_preauth_req(struct scheduler_msg *msg)
{
	struct wlan_preauth_req *preauth_req;

	if (!msg || !msg->bodyptr) {
		mlme_err("msg or msg->bodyptr is NULL");
		return QDF_STATUS_E_INVAL;
	}

	preauth_req = msg->bodyptr;
	cm_free_preauth_req(preauth_req);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
cm_issue_preauth_req(struct cnx_mgr *cm_ctx, struct cm_roam_req *roam_req)
{
	struct wlan_preauth_req *preauth_req;
	struct scheduler_msg msg;
	QDF_STATUS status;

	qdf_mem_zero(&msg, sizeof(msg));
	preauth_req = qdf_mem_malloc(sizeof(*preauth_req));
	if (!preauth_req)
		return QDF_STATUS_E_NOMEM;

	preauth_req->vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	preauth_req->entry =
		util_scan_copy_cache_entry(roam_req->cur_candidate->entry);

	if (!preauth_req->entry) {
		qdf_mem_free(preauth_req);
		return QDF_STATUS_E_NOMEM;
	}

	msg.bodyptr = preauth_req;
	msg.type = CM_PREAUTH_REQ;
	msg.flush_callback = cm_flush_preauth_req;

	status = scheduler_post_message(QDF_MODULE_ID_MLME,
					QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "msg post fail",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
				       roam_req->cm_id));
		cm_free_preauth_req(preauth_req);
	}

	return status;
}

QDF_STATUS cm_preauth_active(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	struct cm_req *cm_req;
	struct cm_roam_req *roam_req;
	QDF_STATUS status;

	cm_req = cm_get_req_by_cm_id(cm_ctx, *cm_id);
	if (!cm_req)
		return QDF_STATUS_E_INVAL;

	cm_ctx->active_cm_id = *cm_id;
	roam_req = &cm_req->roam_req;

	status = cm_issue_preauth_req(cm_ctx, roam_req);
	if (QDF_IS_STATUS_ERROR(status))
		cm_send_preauth_start_fail(cm_ctx, *cm_id, CM_GENERIC_FAILURE);

	return status;
}

#ifdef FEATURE_WLAN_ESE
static void cm_update_cckmtsf(uint32_t *timestamp0, uint32_t *timestamp1,
			      uint64_t *incr)
{
	uint64_t timestamp64 = ((uint64_t)*timestamp1 << 32) | (*timestamp0);

	timestamp64 = (uint64_t)(timestamp64 + (*incr));
	*timestamp0 = (uint32_t)(timestamp64 & 0xffffffff);
	*timestamp1 = (uint32_t)((timestamp64 >> 32) & 0xffffffff);
}

/**
 * cm_roam_read_tsf() - read TSF
 * @cm_ctx: connection manager context
 * @rsp: preauth response
 *
 * This function reads the TSF and also add the time elapsed since last
 * beacon or probe response reception from the hand off AP to arrive at
 * the latest TSF value.
 *
 * Return: none
 */
static void cm_roam_read_tsf(struct cnx_mgr *cm_ctx,
			     struct wlan_preauth_rsp *rsp)
{
	struct cm_req *cm_req;
	struct scan_cache_entry *scan_entry;
	uint64_t timer_diff = 0;

	cm_req = cm_get_req_by_cm_id(cm_ctx, rsp->cm_id);
	if (!cm_req)
		return;
	if (!cm_req->roam_req.cur_candidate ||
	    !cm_req->roam_req.cur_candidate->entry)
		return;

	scan_entry = cm_req->roam_req.cur_candidate->entry;
	qdf_mem_copy(rsp->timestamp, scan_entry->tsf_info.data, 8);

	/* Get the time diff in nano seconds */
	timer_diff = qdf_get_monotonic_boottime_ns() - scan_entry->boottime_ns;
	/* Convert msec to micro sec timer */
	timer_diff = do_div(timer_diff, SYSTEM_TIME_NSEC_TO_USEC);
	/*  Update the TSF with the difference in system time */
	cm_update_cckmtsf(&rsp->timestamp[0], &rsp->timestamp[1],
			  &timer_diff);
}
#else
static inline void cm_roam_read_tsf(struct cnx_mgr *cm_ctx,
				    struct wlan_preauth_rsp *rsp)
{}
#endif

#define MD_IE_ID 54

void cm_reassoc_timer_callback(void *context)
{
	QDF_STATUS status;
	struct reassoc_timer_ctx *ctx = context;
	struct cnx_mgr *cm_ctx;
	struct wlan_objmgr_vdev *vdev;
	wlan_cm_id cm_id = ctx->cm_id;
	uint8_t vdev_id = ctx->vdev_id;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(ctx->pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err(CM_PREFIX_FMT "vdev object is NULL",
			 CM_PREFIX_REF(vdev_id, cm_id));
		return;
	}

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		goto rel_ref;

	status = cm_sm_deliver_event(vdev, WLAN_CM_SM_EV_REASSOC_TIMER,
				     sizeof(wlan_cm_id), &cm_id);

	if (QDF_IS_STATUS_ERROR(status))
		cm_preauth_handle_event_post_fail(cm_ctx, cm_id);

rel_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

void cm_preauth_success(struct cnx_mgr *cm_ctx, struct wlan_preauth_rsp *rsp)
{
	QDF_STATUS status;
	struct mlme_legacy_priv *mlme_priv;
	struct rso_config *rso_cfg;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id = rsp->vdev_id;
	wlan_cm_id cm_id = rsp->cm_id;
	struct cm_roam_values_copy config;
	bool is_11r;

	vdev = cm_ctx->vdev;
	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev || !rsp->psoc) {
		mlme_err(CM_PREFIX_FMT "pdev or psoc is NULL",
			 CM_PREFIX_REF(vdev_id, cm_id));
		goto err;
	}

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv) {
		mlme_err(CM_PREFIX_FMT "vdev ext priv is NULL",
			 CM_PREFIX_REF(vdev_id, cm_id));
		goto err;
	}

	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg) {
		mlme_err(CM_PREFIX_FMT "rso_cfg is NULL",
			 CM_PREFIX_REF(vdev_id, cm_id));
		goto err;
	}

	mlme_err(CM_PREFIX_FMT "Preauth success with " QDF_MAC_ADDR_FMT,
		 CM_PREFIX_REF(vdev_id, rsp->cm_id),
		 QDF_MAC_ADDR_REF(rsp->pre_auth_bssid.bytes));

	cm_csr_preauth_done(vdev);

	qdf_mem_copy(rsp->ric_ies,
		     mlme_priv->connect_info.ft_info.ric_ies,
		     mlme_priv->connect_info.ft_info.ric_ies_length);
	rsp->ric_ies_length = mlme_priv->connect_info.ft_info.ric_ies_length;

	mlme_priv->connect_info.ft_info.ft_state = FT_REASSOC_REQ_WAIT;

	/* start reassoc timer */
	rso_cfg->ctx.pdev = pdev;
	rso_cfg->ctx.vdev_id = vdev_id;
	rso_cfg->ctx.cm_id = cm_id;

	status = qdf_mc_timer_start(&rso_cfg->reassoc_timer,
				    REASSOC_TIMER_DURATION);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "start reassoc timer failed, status %d",
			 CM_PREFIX_REF(vdev_id, cm_id), status);
		goto err;
	}

	wlan_cm_roam_cfg_get_value(rsp->psoc, vdev_id, IS_11R_CONNECTION,
				   &config);
	is_11r = config.bool_value;
	if (is_11r)
		mlme_cm_osif_ft_preauth_complete(vdev, rsp);

	if (wlan_cm_get_ese_assoc(pdev, vdev_id)) {
		cm_roam_read_tsf(cm_ctx, rsp);
		mlme_cm_osif_cckm_preauth_complete(vdev, rsp);
	}

	if (cm_is_fast_roam_enabled(rsp->psoc) &&
	    cm_is_rsn_or_8021x_sha256_auth_type(vdev))
		mlme_cm_osif_pmksa_candidate_notify(vdev, &rsp->pre_auth_bssid,
						    1, false);

	mlme_priv->connect_info.ft_info.add_mdie = false;
	if (!(is_11r && cm_is_open_mode(vdev)))
		return;

	qdf_mem_zero(mlme_priv->connect_info.ft_info.reassoc_ft_ie,
		     MAX_FTIE_SIZE);
	mlme_priv->connect_info.ft_info.reassoc_ie_len = 0;

	if (wlan_get_ie_ptr_from_eid(MD_IE_ID, rsp->ft_ie, rsp->ft_ie_length))
		mlme_priv->connect_info.ft_info.add_mdie = true;

	if (!mlme_priv->connect_info.ft_info.ric_ies_length)
		return;

	/* Copy the RIC IEs to reassoc IEs */
	qdf_mem_copy(mlme_priv->connect_info.ft_info.reassoc_ft_ie,
		     mlme_priv->connect_info.ft_info.ric_ies,
		     mlme_priv->connect_info.ft_info.ric_ies_length);
	mlme_priv->connect_info.ft_info.reassoc_ie_len =
			mlme_priv->connect_info.ft_info.ric_ies_length;
	mlme_priv->connect_info.ft_info.add_mdie = true;
	return;

err:
	rsp->status = QDF_STATUS_E_ABORTED;
	status = cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_PREAUTH_RESP,
					  sizeof(*rsp), rsp);
	if (QDF_IS_STATUS_ERROR(status))
		cm_preauth_handle_event_post_fail(cm_ctx, cm_id);
}

static QDF_STATUS
cm_hanlde_preauth_failure(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
{
	QDF_STATUS status;
	struct wlan_cm_preauth_fail preauth_fail_rsp;

	preauth_fail_rsp.cm_id = cm_id;
	preauth_fail_rsp.reason = CM_GENERIC_FAILURE;

	status = cm_sm_deliver_event_sync(
			cm_ctx, WLAN_CM_SM_EV_PREAUTH_FAIL,
			sizeof(struct wlan_cm_preauth_fail), &preauth_fail_rsp);
	if (QDF_IS_STATUS_ERROR(status))
		cm_preauth_handle_event_post_fail(cm_ctx, cm_id);

	return status;
}

void cm_preauth_done_resp(struct cnx_mgr *cm_ctx, struct wlan_preauth_rsp *rsp)
{
	QDF_STATUS status;
	struct cm_req *cm_req;
	wlan_cm_id cm_id = rsp->cm_id;

	if (QDF_IS_STATUS_ERROR(rsp->status)) {
		cm_req = cm_get_req_by_cm_id(cm_ctx, cm_id);
		if (!cm_req)
			return;
		mlme_info(CM_PREFIX_FMT "Preauth attempt no. %d failed for " QDF_MAC_ADDR_FMT,
			  CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id),
			  cm_req->roam_req.num_preauth_retry,
			  QDF_MAC_ADDR_REF(rsp->pre_auth_bssid.bytes));

		/* retry again with same or new candidate */
		status = cm_host_roam_preauth_start(cm_ctx, cm_req);
		if (QDF_IS_STATUS_ERROR(status))
			cm_hanlde_preauth_failure(cm_ctx, cm_id);
	} else {
		status = cm_sm_deliver_event_sync(cm_ctx,
						  WLAN_CM_SM_EV_PREAUTH_DONE,
						  sizeof(*rsp), rsp);
		if (QDF_IS_STATUS_ERROR(status))
			cm_preauth_handle_event_post_fail(cm_ctx, cm_id);
	}
}

/**
 * cm_remove_preauth_cmd_from_serialization() - Remove preauth cmd
 * from serialization
 * @cm_ctx: connection manager context
 * @cm_id: cm id of roam req
 *
 * Return: void
 */
static void cm_remove_preauth_cmd_from_serialization(struct cnx_mgr *cm_ctx,
						     wlan_cm_id cm_id)
{
	struct wlan_serialization_queued_cmd_info cmd_info;

	qdf_mem_zero(&cmd_info, sizeof(cmd_info));
	cmd_info.vdev = cm_ctx->vdev;
	cmd_info.cmd_id = cm_id;
	cmd_info.req_type = WLAN_SER_CANCEL_NON_SCAN_CMD;
	cmd_info.cmd_type = WLAN_SER_CMD_PERFORM_PRE_AUTH;

	mlme_debug(CM_PREFIX_FMT "Remove cmd type %d from active",
		   CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id),
		   cmd_info.cmd_type);
	cmd_info.queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE;
	wlan_serialization_remove_cmd(&cmd_info);
}

static QDF_STATUS cm_preauth_rsp(struct wlan_objmgr_vdev *vdev,
				 struct wlan_preauth_rsp *rsp)
{
	QDF_STATUS status;
	struct cnx_mgr *cm_ctx;
	wlan_cm_id cm_id;
	uint32_t prefix;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_NULL_VALUE;

	cm_id = cm_ctx->active_cm_id;
	prefix = CM_ID_GET_PREFIX(cm_id);

	if (prefix != ROAM_REQ_PREFIX) {
		mlme_err(CM_PREFIX_FMT "active req is not roam req",
			 CM_PREFIX_REF(wlan_vdev_get_id(vdev), cm_id));
		return QDF_STATUS_E_INVAL;
	}
	rsp->cm_id = cm_id;

	mlme_debug(CM_PREFIX_FMT "preauth resp status %d for " QDF_MAC_ADDR_FMT,
		   CM_PREFIX_REF(wlan_vdev_get_id(vdev), cm_id),
		   rsp->status, QDF_MAC_ADDR_REF(rsp->pre_auth_bssid.bytes));

	cm_remove_preauth_cmd_from_serialization(cm_ctx, cm_id);

	status = cm_sm_deliver_event(vdev, WLAN_CM_SM_EV_PREAUTH_RESP,
				     sizeof(*rsp), rsp);
	if (QDF_IS_STATUS_ERROR(status))
		cm_preauth_handle_event_post_fail(cm_ctx, cm_id);

	return status;
}

QDF_STATUS cm_handle_preauth_rsp(struct scheduler_msg *msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_preauth_rsp *rsp = NULL;
	struct wlan_objmgr_vdev *vdev;

	if (!msg || !msg->bodyptr) {
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	rsp = (struct wlan_preauth_rsp *)msg->bodyptr;
	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(rsp->psoc, rsp->vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev_id: %d : vdev not found, status %d",
			 rsp->vdev_id, rsp->status);
		status = QDF_STATUS_E_INVAL;
		goto end;
	}

	status = cm_preauth_rsp(vdev, rsp);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
end:
	if (rsp)
		qdf_mem_free(rsp);

	return status;
}
