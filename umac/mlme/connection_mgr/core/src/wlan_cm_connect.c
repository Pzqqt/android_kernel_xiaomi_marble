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
 * DOC: Implements connect specific APIs of connection manager
 */

#include "wlan_cm_main_api.h"
#include "wlan_cm_bss_score_param.h"
#include "wlan_scan_api.h"
#include "wlan_cm_sm.h"
#ifdef WLAN_POLICY_MGR_ENABLE
#include "wlan_policy_mgr_api.h"
#endif
#include <wlan_serialization_api.h>
#ifdef CONN_MGR_ADV_FEATURE
#include "wlan_blm_api.h"
#endif

/**
 * cm_connect_handle_event_post_fail() - initiate connect failure if msg posting
 * to SM fails
 * @cm_ctx: connection manager context
 * @cm_id: cm_id for connect req for which post fails
 *
 * Context: Can be called from any context and to be used only after posting a
 * msg to SM fails from external event e.g. peer create resp,
 * HW mode change resp  serialization cb.
 *
 * Return: QDF_STATUS
 */
static void
cm_connect_handle_event_post_fail(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
{
	struct wlan_cm_connect_rsp *resp;

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return;

	resp->connect_status = QDF_STATUS_E_FAILURE;
	resp->cm_id = cm_id;
	resp->vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	resp->reason = CM_ABORT_DUE_TO_NEW_REQ_RECVD;

	cm_connect_complete(cm_ctx, resp);
	qdf_mem_free(resp);
}

static QDF_STATUS cm_connect_cmd_timeout(struct cnx_mgr *cm_ctx,
					 wlan_cm_id cm_id)
{
	struct wlan_cm_connect_rsp *resp;
	QDF_STATUS status;

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return QDF_STATUS_E_NOMEM;

	resp->connect_status = QDF_STATUS_E_FAILURE;
	resp->cm_id = cm_id;
	resp->vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	resp->reason = CM_SER_TIMEOUT;
	wlan_vdev_mlme_get_ssid(cm_ctx->vdev, resp->ssid.ssid,
				&resp->ssid.length);

	status = cm_sm_deliver_event(cm_ctx->vdev,
				     WLAN_CM_SM_EV_CONNECT_FAILURE,
				     sizeof(*resp), resp);
	qdf_mem_free(resp);

	if (QDF_IS_STATUS_ERROR(status))
		cm_connect_handle_event_post_fail(cm_ctx, cm_id);

	return status;
}

static QDF_STATUS
cm_ser_connect_cb(struct wlan_serialization_command *cmd,
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

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		status = cm_sm_deliver_event(vdev, WLAN_CM_SM_EV_CONNECT_ACTIVE,
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
		cm_ctx = cm_get_cm_ctx(vdev);
		if (cm_ctx)
			cm_connect_handle_event_post_fail(cm_ctx, cmd->cmd_id);
		break;
	case WLAN_SER_CB_CANCEL_CMD:
		/* command removed from pending list. */
		break;
	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		mlme_err("Active command timeout cm_id %d", cmd->cmd_id);
		QDF_ASSERT(0);
		cm_ctx = cm_get_cm_ctx(vdev);
		if (cm_ctx)
			cm_connect_cmd_timeout(cm_ctx, cmd->cmd_id);
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

#define CONNECT_TIMEOUT       30000

static QDF_STATUS cm_ser_connect_req(struct wlan_objmgr_pdev *pdev,
				     struct cnx_mgr *cm_ctx,
				     struct cm_connect_req *cm_req)
{
	struct wlan_serialization_command cmd = {0, };
	enum wlan_serialization_status ser_cmd_status;
	QDF_STATUS status;

	status = wlan_objmgr_vdev_try_get_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("unable to get reference");
		return status;
	}

	cmd.cmd_type = WLAN_SER_CMD_VDEV_CONNECT;
	cmd.cmd_id = cm_req->cm_id;
	cmd.cmd_cb = cm_ser_connect_cb;
	cmd.source = WLAN_UMAC_COMP_MLME;
	cmd.is_high_priority = false;
	cmd.cmd_timeout_duration = CONNECT_TIMEOUT;
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
		mlme_err("ser cmd status %d", ser_cmd_status);
		wlan_objmgr_vdev_release_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);

		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_send_connect_start_fail() - initiate conenct failure
 * @cm_ctx: connection manager context
 * @req: connect req for which connect failed
 * @reason: failure reason
 *
 * Context: Can be called from any context and to be used only after posting a
 * msg to SM (ie holding the SM lock) to avoid use after free for req.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_send_connect_start_fail(struct cnx_mgr *cm_ctx,
			   struct cm_connect_req *req,
			   enum wlan_cm_connect_fail_reason reason)
{
	struct wlan_cm_connect_rsp *resp;
	QDF_STATUS status;

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return QDF_STATUS_E_NOMEM;

	resp->connect_status = QDF_STATUS_E_FAILURE;
	resp->cm_id = req->cm_id;
	resp->vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	resp->reason = reason;
	resp->ssid.length = req->req.ssid.length;
	qdf_mem_copy(resp->ssid.ssid, req->req.ssid.ssid, resp->ssid.length);
	if (req->cur_candidate) {
		qdf_copy_macaddr(&resp->bssid,
				 &req->cur_candidate->entry->bssid);
		resp->freq = req->cur_candidate->entry->channel.chan_freq;
	}

	status = cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_CONNECT_FAILURE,
					  sizeof(*resp), resp);
	qdf_mem_free(resp);

	return status;
}

#ifdef WLAN_POLICY_MGR_ENABLE

QDF_STATUS cm_handle_hw_mode_change(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id,
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
		mlme_err("Failed to find pdev from vdev");
		goto send_failure;
	}

	if (event == WLAN_CM_SM_EV_HW_MODE_SUCCESS) {
		status = cm_ser_connect_req(pdev, cm_ctx, &cm_req->connect_req);
		if (QDF_IS_STATUS_ERROR(status)) {
			reason = CM_SER_FAILURE;
			goto send_failure;
		}
		return status;
	}

	/* Set reason HW mode fail for event WLAN_CM_SM_EV_HW_MODE_FAILURE */
	reason = CM_HW_MODE_FAILURE;

send_failure:
	return cm_send_connect_start_fail(cm_ctx, &cm_req->connect_req, reason);
}

void cm_hw_mode_change_resp(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			    wlan_cm_id cm_id, QDF_STATUS status)
{
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS qdf_status;
	enum wlan_cm_sm_evt event = WLAN_CM_SM_EV_HW_MODE_SUCCESS;
	struct cnx_mgr *cm_ctx;

	mlme_debug("vdev %d cm id %d Continue connect after HW mode change, status %d",
		   vdev_id, cm_id, status);

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev)
		return;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx) {
		mlme_err("cm ctx NULL");
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
	 * hw mode change resp should be handled only in JOIN_PENDING. If
	 * new command has been received connect should be
	 * aborted from here with connect req cleanup.
	 */
	if (QDF_IS_STATUS_ERROR(status))
		cm_connect_handle_event_post_fail(cm_ctx, cm_id);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
}

static QDF_STATUS cm_check_for_hw_mode_change(struct wlan_objmgr_psoc *psoc,
					      qdf_list_t *scan_list,
					      uint8_t vdev_id,
					      wlan_cm_id connect_id)
{
	return policy_mgr_change_hw_mode_sta_connect(psoc, scan_list, vdev_id,
						     connect_id);
}

static void
cm_get_pcl_chan_weigtage_for_sta(struct wlan_objmgr_pdev *pdev,
				 struct pcl_freq_weight_list *pcl_lst)
{
	enum QDF_OPMODE opmode = QDF_STA_MODE;
	enum policy_mgr_con_mode pm_mode;
	uint32_t num_entries = 0;
	QDF_STATUS status;

	if (!pcl_lst)
		return;

	if (policy_mgr_map_concurrency_mode(&opmode, &pm_mode)) {
		status = policy_mgr_get_pcl(wlan_pdev_get_psoc(pdev), pm_mode,
					    pcl_lst->pcl_freq_list,
					    &num_entries,
					    pcl_lst->pcl_weight_list,
					    NUM_CHANNELS);
		if (QDF_IS_STATUS_ERROR(status))
			return;
		pcl_lst->num_of_pcl_channels = num_entries;
	}
}

static void cm_calculate_scores(struct wlan_objmgr_pdev *pdev,
				struct scan_filter *filter, qdf_list_t *list)
{
	struct pcl_freq_weight_list *pcl_lst = NULL;

	if (!filter->num_of_bssid) {
		pcl_lst = qdf_mem_malloc(sizeof(*pcl_lst));
		cm_get_pcl_chan_weigtage_for_sta(pdev, pcl_lst);
		if (pcl_lst && !pcl_lst->num_of_pcl_channels) {
			qdf_mem_free(pcl_lst);
			pcl_lst = NULL;
		}
	}
	wlan_cm_calculate_bss_score(pdev, pcl_lst, list, &filter->bssid_hint);
	if (pcl_lst)
		qdf_mem_free(pcl_lst);
}
#else

static QDF_STATUS cm_check_for_hw_mode_change(struct wlan_objmgr_psoc *psoc,
					      qdf_list_t *scan_list,
					      uint8_t vdev_id,
					      uint8_t connect_id)
{
	return QDF_STATUS_E_ALREADY;
}

static inline
void cm_calculate_scores(struct wlan_objmgr_pdev *pdev,
			 struct scan_filter *filter, qdf_list_t *list)
{
	wlan_cm_calculate_bss_score(pdev, NULL, list, &filter->bssid_hint);
}
#endif

#ifdef WLAN_FEATURE_11W
static inline void
cm_set_pmf_caps(struct cm_connect_req *cm_req, struct scan_filter *filter)
{
	filter->pmf_cap = cm_req->req.crypto.pmf_cap;
}
#else
static inline void
cm_set_pmf_caps(struct cm_connect_req *cm_req, struct scan_filter *filter)
{}
#endif

static void cm_connect_prepare_scan_fliter(struct cnx_mgr *cm_ctx,
					   struct cm_connect_req *cm_req,
					   struct scan_filter *filter)
{
	if (!qdf_is_macaddr_zero(&cm_req->req.bssid)) {
		filter->num_of_bssid = 1;
		qdf_copy_macaddr(&filter->bssid_list[0], &cm_req->req.bssid);
	}

	qdf_copy_macaddr(&filter->bssid_hint, &cm_req->req.bssid_hint);
	filter->num_of_ssid = 1;
	qdf_mem_copy(&filter->ssid_list[0], &cm_req->req.ssid,
		     sizeof(struct wlan_ssid));

	if (cm_req->req.chan_freq) {
		filter->num_of_channels = 1;
		filter->chan_freq_list[0] = cm_req->req.chan_freq;
	}

	/* Fill band (STA+STA) */
	/* RSN OVERRIDE */

	filter->authmodeset = cm_req->req.crypto.auth_type;
	filter->ucastcipherset = cm_req->req.crypto.ciphers_pairwise;
	filter->key_mgmt = cm_req->req.crypto.akm_suites;
	filter->mcastcipherset = cm_req->req.crypto.group_cipher;
	filter->mgmtcipherset = cm_req->req.crypto.mgmt_ciphers;

	cm_set_pmf_caps(cm_req, filter);

	/* FOR WPS/OSEN set ignore auth */
	/* SET mobility domain */
	/* Fill fils info */
}

static QDF_STATUS cm_connect_get_candidates(struct wlan_objmgr_pdev *pdev,
					    struct cnx_mgr *cm_ctx,
					    struct cm_connect_req *cm_req)
{
	struct scan_filter *filter;
	uint32_t num_bss = 0;
	enum QDF_OPMODE op_mode;
	qdf_list_t *candidate_list;

	filter = qdf_mem_malloc(sizeof(*filter));
	if (!filter)
		return QDF_STATUS_E_NOMEM;

	cm_connect_prepare_scan_fliter(cm_ctx, cm_req, filter);

	candidate_list = wlan_scan_get_result(pdev, filter);
	if (candidate_list) {
		num_bss = qdf_list_size(candidate_list);
		mlme_debug("num_entries found %d", num_bss);
	}

	op_mode = wlan_vdev_mlme_get_opmode(cm_ctx->vdev);
	if (num_bss && op_mode == QDF_STA_MODE)
		cm_calculate_scores(pdev, filter, candidate_list);
	qdf_mem_free(filter);

	if (!candidate_list || !qdf_list_size(candidate_list)) {
		if (candidate_list)
			wlan_scan_purge_results(candidate_list);
		mlme_info("No valid candidate found num_bss %d", num_bss);
		/*
		 * Do connect scan only of no candidates were found
		 * if candidates were found and were removed due to invalid
		 * return failure
		 */
		if (!num_bss)
			return cm_sm_deliver_event_sync(cm_ctx,
							WLAN_CM_SM_EV_SCAN,
							sizeof(*cm_req),
							cm_req);

		return QDF_STATUS_E_EMPTY;
	}
	cm_req->candidate_list = candidate_list;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_connect_start(struct cnx_mgr *cm_ctx,
			    struct cm_connect_req *cm_req)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	enum wlan_cm_connect_fail_reason reason = CM_GENERIC_FAILURE;
	QDF_STATUS status;
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);

	/* Interface event */
	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);
	if (!pdev) {
		mlme_err("Failed to find pdev from vdev %d", vdev_id);
		goto connect_err;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err("Failed to find psoc from vdev %d", vdev_id);
		goto connect_err;
	}

	status = cm_connect_get_candidates(pdev, cm_ctx, cm_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		reason = CM_NO_CANDIDATE_FOUND;
		goto connect_err;
	}

	status = cm_check_for_hw_mode_change(psoc, cm_req->candidate_list,
					     vdev_id, cm_req->cm_id);
	if (QDF_IS_STATUS_ERROR(status) && status != QDF_STATUS_E_ALREADY) {
		reason = CM_HW_MODE_FAILURE;
		mlme_err("Vdev %d Failed to set HW mode change", vdev_id);
		goto connect_err;
	} else if (QDF_IS_STATUS_SUCCESS(status)) {
		mlme_debug("Vdev %d Connect will continue after HW mode change",
			   vdev_id);
		return QDF_STATUS_SUCCESS;
	}

	status = cm_ser_connect_req(pdev, cm_ctx, cm_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		reason = CM_SER_FAILURE;
		goto connect_err;
	}

	return QDF_STATUS_SUCCESS;

connect_err:
	return cm_send_connect_start_fail(cm_ctx, cm_req, reason);
}

#ifdef WLAN_FEATURE_INTERFACE_MGR
static QDF_STATUS cm_validate_candidate(struct cnx_mgr *cm_ctx,
					struct scan_cache_entry *scan_entry)
{
	struct if_mgr_event_data event_data = {0};

	event_data.validate_bss_info.chan_freq = scan_entry->channel.chan_freq;
	qdf_copy_macaddr(&event_data.validate_bss_info.peer_addr,
			 &scan_entry->bssid);

	return if_mgr_deliver_event(cm_ctx->vdev,
				    WLAN_IF_MGR_EV_VALIDATE_CANDIDATE,
				    &event_data);
}

static QDF_STATUS
cm_inform_if_mgr_connect_complete(struct wlan_objmgr_vdev *vdev,
				  QDF_STATUS connect_status)
{
	struct if_mgr_event_data *connect_complete;

	connect_complete = qdf_mem_malloc(sizeof(*connect_complete));
	if (!connect_complete)
		return QDF_STATUS_E_NOMEM;

	connect_complete->status = connect_status;
	if_mgr_deliver_event(vdev, WLAN_IF_MGR_EV_CONNECT_COMPLETE,
			     connect_complete);
	qdf_mem_free(connect_complete);

	return QDF_STATUS_SUCCESS;
}
#else
static inline
QDF_STATUS cm_validate_candidate(struct cnx_mgr *cm_ctx,
				 struct scan_cache_entry *scan_entry)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
cm_inform_if_mgr_connect_complete(struct wlan_objmgr_vdev *vdev,
				  QDF_STATUS connect_status)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * cm_get_valid_candidate() - This API will be called to get the next valid
 * candidate
 * @cm_ctx: connection manager context
 * @cm_req: Connect request.
 *
 * This function return a valid candidate to try connection. It return failure
 * if no valid candidate is present or all valid candidate are tried.
 *
 * Return: QDF status
 */
static QDF_STATUS cm_get_valid_candidate(struct cnx_mgr *cm_ctx,
					 struct cm_req *cm_req)
{
	struct scan_cache_node *scan_node = NULL;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct scan_cache_node *cur_candidate;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (cm_req->connect_req.connect_attempts >=
	    CM_MAX_CANDIDATE_TO_BE_TRIED) {
		mlme_info("Max %d candidates tried for the conenction",
			  cm_req->connect_req.connect_attempts);
		return QDF_STATUS_E_FAILURE;
	}

	cur_candidate = cm_req->connect_req.cur_candidate;
	/*
	 * Get next candidate if cur_candidate is not NULL, else get
	 * the first candidate
	 */
	if (cur_candidate)
		qdf_list_peek_next(cm_req->connect_req.candidate_list,
				   &cur_candidate->node, &cur_node);
	else
		qdf_list_peek_front(cm_req->connect_req.candidate_list,
				    &cur_node);

	while (cur_node) {
		qdf_list_peek_next(cm_req->connect_req.candidate_list,
				   cur_node, &next_node);
		scan_node = qdf_container_of(cur_node, struct scan_cache_node,
					     node);
		status = cm_validate_candidate(cm_ctx, scan_node->entry);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			cur_candidate = scan_node;
			break;
		}

		cur_node = next_node;
		next_node = NULL;
	}

	/*
	 * If cur_node is NULL cur candidate was last to be tried so no more
	 * candidates left for connect now.
	 */
	if (!cur_node) {
		mlme_debug("No more candidates left");
		cm_req->connect_req.cur_candidate = NULL;
		return QDF_STATUS_E_FAILURE;
	}

	cm_req->connect_req.connect_attempts++;
	cm_req->connect_req.cur_candidate = cur_candidate;

	return QDF_STATUS_SUCCESS;
}

static void
cm_fill_resp_for_bss_create_fail(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_connect_rsp *resp,
				 wlan_cm_id cm_id,
				 struct qdf_mac_addr *peer_mac)
{
	resp->connect_status = QDF_STATUS_E_FAILURE;
	resp->cm_id = cm_id;
	resp->vdev_id = wlan_vdev_get_id(vdev);
	resp->reason = CM_PEER_CREATE_FAILED;
	qdf_copy_macaddr(&resp->bssid, peer_mac);
	wlan_vdev_mlme_get_ssid(vdev, resp->ssid.ssid, &resp->ssid.length);
}

static void cm_create_bss_peer(struct cnx_mgr *cm_ctx,
			       struct cm_connect_req *req)
{
	QDF_STATUS status;
	struct qdf_mac_addr *bssid;

	bssid = &req->cur_candidate->entry->bssid;
	status = mlme_cm_bss_peer_create_req(cm_ctx->vdev, bssid);
	if (QDF_IS_STATUS_ERROR(status)) {
		struct wlan_cm_connect_rsp *resp;

		/* In case of failure try with next candidate */
		mlme_err("peer create request failed %d", status);

		resp = qdf_mem_malloc(sizeof(*resp));
		if (!resp)
			return;

		cm_fill_resp_for_bss_create_fail(cm_ctx->vdev, resp, req->cm_id,
						 bssid);
		cm_sm_deliver_event_sync(cm_ctx,
				WLAN_CM_SM_EV_CONNECT_GET_NEXT_CANDIDATE,
				sizeof(*resp), resp);
		qdf_mem_free(resp);
	}
}

QDF_STATUS cm_try_next_candidate(struct cnx_mgr *cm_ctx,
				 struct wlan_cm_connect_rsp *resp)
{
	QDF_STATUS status;
	struct cm_req *cm_req;

	cm_req = cm_get_req_by_cm_id(cm_ctx, resp->cm_id);
	if (!cm_req) {
		mlme_err("cm req NULL connect fail");
		return QDF_STATUS_E_FAILURE;
	}

	status = cm_get_valid_candidate(cm_ctx, cm_req);
	if (QDF_IS_STATUS_ERROR(status))
		goto connect_err;

	mlme_cm_osif_failed_candidate_ind(cm_ctx->vdev, resp);

	cm_create_bss_peer(cm_ctx, &cm_req->connect_req);

	return QDF_STATUS_SUCCESS;

connect_err:
	return cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_CONNECT_FAILURE,
					sizeof(*resp), resp);

}

bool cm_connect_resp_cmid_match_list_head(struct cnx_mgr *cm_ctx,
					  struct wlan_cm_connect_rsp *resp)
{
	return cm_check_cmid_match_list_head(cm_ctx, &resp->cm_id);
}

QDF_STATUS cm_connect_active(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	struct cm_req *cm_req;
	QDF_STATUS status;
	struct wlan_cm_connect_req *req;

	cm_req = cm_get_req_by_cm_id(cm_ctx, *cm_id);
	if (!cm_req) {
		mlme_err("cm req NULL for *cm_id %x", *cm_id);
		return QDF_STATUS_E_INVAL;
	}

	cm_ctx->active_cm_id = *cm_id;
	req = &cm_req->connect_req.req;
	wlan_vdev_mlme_set_ssid(cm_ctx->vdev, req->ssid.ssid, req->ssid.length);

	status = cm_get_valid_candidate(cm_ctx, cm_req);
	if (QDF_IS_STATUS_ERROR(status))
		goto connect_err;

	cm_create_bss_peer(cm_ctx, &cm_req->connect_req);

	return QDF_STATUS_SUCCESS;

connect_err:
	return cm_send_connect_start_fail(cm_ctx,
					  &cm_req->connect_req, CM_JOIN_FAILED);
}

QDF_STATUS
cm_resume_connect_after_peer_create(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	struct wlan_cm_vdev_connect_req req;
	struct cm_req *cm_req;
	QDF_STATUS status;

	cm_req = cm_get_req_by_cm_id(cm_ctx, *cm_id);
	if (!cm_req) {
		mlme_err("cm req NULL for *cm_id %x", *cm_id);
		return QDF_STATUS_E_FAILURE;
	}
	/*
	 * fill vdev crypto for the peer.
	 */

	req.vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	req.cm_id = *cm_id;
	req.bss = cm_req->connect_req.cur_candidate;

	status = mlme_cm_connect_req(cm_ctx->vdev, &req);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("connect request failed for *cm_id %x", *cm_id);
		status = cm_send_connect_start_fail(cm_ctx,
						    &cm_req->connect_req,
						    CM_JOIN_FAILED);
	}

	return status;
}

#ifdef CONN_MGR_ADV_FEATURE
static QDF_STATUS
cm_inform_blm_connect_complete(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_connect_rsp *resp)
{
	struct wlan_objmgr_pdev *pdev;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("Failed to find pdev from vdev");
		return QDF_STATUS_E_FAILURE;
	}

	if (QDF_IS_STATUS_SUCCESS(resp->connect_status))
		wlan_blm_update_bssid_connect_params(pdev, resp->bssid,
						     BLM_AP_CONNECTED);

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS
cm_inform_blm_connect_complete(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_connect_rsp *resp)
{
	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS cm_connect_complete(struct cnx_mgr *cm_ctx,
			       struct wlan_cm_connect_rsp *resp)
{
	/*
	 * If the entry is not present in the list, it must have been cleared
	 * already.
	 */
	if (!cm_get_req_by_cm_id(cm_ctx, resp->cm_id))
		return QDF_STATUS_SUCCESS;

	mlme_cm_osif_connect_complete(cm_ctx->vdev, resp);

	if (cm_get_state(cm_ctx) == WLAN_CM_S_INIT ||
	    cm_get_state(cm_ctx) == WLAN_CM_S_CONNECTED) {
		cm_inform_if_mgr_connect_complete(cm_ctx->vdev,
						  resp->connect_status);

		cm_inform_blm_connect_complete(cm_ctx->vdev, resp);

		mlme_cm_connect_complete_ind(cm_ctx->vdev, resp);
	}

	/*
	 * update fils/wep key and inform legacy, update bcn filter and scan
	 * entry mlme info,, start OBSS scan for open mode.
	 */

	cm_remove_cmd(cm_ctx, resp->cm_id);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_add_connect_req_to_list(struct cnx_mgr *cm_ctx,
				      struct cm_connect_req *req)
{
	QDF_STATUS status;
	struct cm_req *cm_req;

	cm_req = qdf_container_of(req, struct cm_req, connect_req);
	req->cm_id = cm_get_cm_id(cm_ctx, req->req.source);
	cm_req->cm_id = req->cm_id;
	status = cm_add_req_to_list_and_indicate_osif(cm_ctx, cm_req,
						      req->req.source);

	return status;
}

QDF_STATUS cm_connect_rsp(struct wlan_objmgr_vdev *vdev,
			  struct wlan_cm_connect_rsp *resp)
{
	struct cnx_mgr *cm_ctx;
	QDF_STATUS qdf_status;
	wlan_cm_id cm_id;
	uint32_t prefix;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx) {
		mlme_err("cm ctx NULL");
		return QDF_STATUS_E_INVAL;
	}

	cm_id = cm_ctx->active_cm_id;
	prefix = CM_ID_GET_PREFIX(cm_id);

	if (prefix != CONNECT_REQ_PREFIX || cm_id != resp->cm_id) {
		mlme_err("Active req %x is not connect req connect req cm_id %x",
			 cm_id, resp->cm_id);
		qdf_status = QDF_STATUS_E_FAILURE;
		goto post_err;
	}

	if (QDF_IS_STATUS_SUCCESS(resp->connect_status)) {
		qdf_status =
			cm_sm_deliver_event(vdev,
					    WLAN_CM_SM_EV_CONNECT_SUCCESS,
					    sizeof(*resp), resp);
		if (QDF_IS_STATUS_SUCCESS(qdf_status))
			return qdf_status;
		/*
		 * failure mean that the new connect/disconnect is received so
		 * cleanup.
		 */
		goto post_err;
	}

	/* In case of failure try with next candidate */
	qdf_status =
		cm_sm_deliver_event(vdev,
				    WLAN_CM_SM_EV_CONNECT_GET_NEXT_CANDIDATE,
				    sizeof(*resp), resp);

	if (QDF_IS_STATUS_SUCCESS(qdf_status))
		return qdf_status;

post_err:
	/*
	 * If there is a event posting error it means the SM state is not in
	 * JOIN ACTIVE (some new cmd has changed the state of SM), so just
	 * complete the connect command.
	 */
	cm_connect_complete(cm_ctx, resp);

	return qdf_status;
}

QDF_STATUS cm_bss_peer_create_rsp(struct wlan_objmgr_vdev *vdev,
				  QDF_STATUS status,
				  struct qdf_mac_addr *peer_mac)
{
	struct cnx_mgr *cm_ctx;
	QDF_STATUS qdf_status;
	wlan_cm_id cm_id;
	uint32_t prefix;
	struct wlan_cm_connect_rsp *resp;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx) {
		mlme_err("cm ctx NULL");
		return QDF_STATUS_E_INVAL;
	}

	cm_id = cm_ctx->active_cm_id;
	prefix = CM_ID_GET_PREFIX(cm_id);

	if (prefix != CONNECT_REQ_PREFIX) {
		mlme_err("Active req %x is not connect req", cm_id);
		mlme_cm_bss_peer_delete_req(vdev);
		return QDF_STATUS_E_INVAL;
	}

	if (QDF_IS_STATUS_SUCCESS(status)) {
		qdf_status =
			cm_sm_deliver_event(vdev,
					    WLAN_CM_EV_BSS_CREATE_PEER_SUCCESS,
					    sizeof(wlan_cm_id), &cm_id);
		if (QDF_IS_STATUS_SUCCESS(qdf_status))
			return qdf_status;

		mlme_cm_bss_peer_delete_req(vdev);
		goto post_err;
	}

	/* In case of failure try with next candidate */
	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp) {
		qdf_status = QDF_STATUS_E_NOMEM;
		goto post_err;
	}

	cm_fill_resp_for_bss_create_fail(cm_ctx->vdev, resp, cm_id, peer_mac);

	qdf_status =
		cm_sm_deliver_event(vdev,
				    WLAN_CM_SM_EV_CONNECT_GET_NEXT_CANDIDATE,
				    sizeof(*resp), resp);
	qdf_mem_free(resp);
	if (QDF_IS_STATUS_SUCCESS(qdf_status))
		return qdf_status;

post_err:
	/*
	 * If there is a event posting error it means the SM state is not in
	 * JOIN ACTIVE (some new cmd has changed the state of SM), so just
	 * complete the connect command.
	 */
	cm_connect_handle_event_post_fail(cm_ctx, cm_id);
	return qdf_status;
}

QDF_STATUS cm_connect_start_req(struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_connect_req *req)
{
	struct cnx_mgr *cm_ctx;
	struct cm_req *cm_req;
	struct cm_connect_req *connect_req;
	QDF_STATUS status;

	cm_ctx = cm_get_cm_ctx(vdev);

	if (!cm_ctx) {
		mlme_err("cm ctx NULL");
		return QDF_STATUS_E_INVAL;
	}

	/*
	 * This would be freed as part of removal from cm req list if adding
	 * to list is success after posting WLAN_CM_SM_EV_CONNECT_REQ.
	 */
	cm_req = qdf_mem_malloc(sizeof(*cm_req));
	if (!cm_req)
		return QDF_STATUS_E_NOMEM;

	/*
	 * Get WAPI/WPA/RSN IE and refill crypto params of req.
	 */

	connect_req = &cm_req->connect_req;
	connect_req->req = *req;

	status = cm_sm_deliver_event(vdev, WLAN_CM_SM_EV_CONNECT_REQ,
				     sizeof(*connect_req), connect_req);

	/* free the req if connect is not handled */
	if (QDF_IS_STATUS_ERROR(status))
		qdf_mem_free(cm_req);

	return status;
}
