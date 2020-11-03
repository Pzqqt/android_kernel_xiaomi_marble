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
#include "wlan_cm_roam_api.h"
#endif

static void
cm_fill_failure_resp_from_cm_id(struct cnx_mgr *cm_ctx,
				struct wlan_cm_connect_resp *resp,
				wlan_cm_id cm_id,
				enum wlan_cm_connect_fail_reason reason)
{
	resp->connect_status = QDF_STATUS_E_FAILURE;
	resp->cm_id = cm_id;
	resp->vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	resp->reason = reason;
	/* Get bssid and ssid and freq for the cm id from the req list */
	cm_fill_bss_info_in_connect_rsp_by_cm_id(cm_ctx, cm_id, resp);
}

static QDF_STATUS cm_connect_cmd_timeout(struct cnx_mgr *cm_ctx,
					 wlan_cm_id cm_id)
{
	struct wlan_cm_connect_resp *resp;
	QDF_STATUS status;

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return QDF_STATUS_E_NOMEM;

	cm_fill_failure_resp_from_cm_id(cm_ctx, resp, cm_id, CM_SER_TIMEOUT);
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
			status = cm_sm_deliver_event(vdev,
						   WLAN_CM_SM_EV_CONNECT_ACTIVE,
						   sizeof(wlan_cm_id),
						   &cmd->cmd_id);
		else
			status = cm_sm_deliver_event_sync(cm_ctx,
						   WLAN_CM_SM_EV_CONNECT_ACTIVE,
						   sizeof(wlan_cm_id),
						   &cmd->cmd_id);
		if (QDF_IS_STATUS_SUCCESS(status))
			break;
		/*
		 * Handle failure if posting fails, i.e. the SM state has
		 * changed or head cm_id doesn't match the active cm_id.
		 * connect active should be handled only in JOIN_PENDING. If
		 * new command has been received connect activation should be
		 * aborted from here with connect req cleanup.
		 */
		cm_connect_handle_event_post_fail(cm_ctx, cmd->cmd_id);
		break;
	case WLAN_SER_CB_CANCEL_CMD:
		/* command removed from pending list. */
		break;
	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		mlme_err(CM_PREFIX_FMT "Active command timeout",
			 CM_PREFIX_REF(wlan_vdev_get_id(vdev), cmd->cmd_id));
		QDF_ASSERT(0);

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
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);

	status = wlan_objmgr_vdev_try_get_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "unable to get reference",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id));
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
		mlme_err(CM_PREFIX_FMT "ser cmd status %d",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id), ser_cmd_status);
		wlan_objmgr_vdev_release_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);

		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

void
cm_connect_handle_event_post_fail(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
{
	struct wlan_cm_connect_resp *resp;

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return;

	cm_fill_failure_resp_from_cm_id(cm_ctx, resp, cm_id,
					CM_ABORT_DUE_TO_NEW_REQ_RECVD);
	cm_connect_complete(cm_ctx, resp);
	qdf_mem_free(resp);
}

QDF_STATUS
cm_send_connect_start_fail(struct cnx_mgr *cm_ctx,
			   struct cm_connect_req *req,
			   enum wlan_cm_connect_fail_reason reason)
{
	struct wlan_cm_connect_resp *resp;
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
		mlme_err(CM_PREFIX_FMT "Failed to find pdev",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
				       cm_req->cm_id));
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

	mlme_debug(CM_PREFIX_FMT "Continue connect after HW mode change, status %d",
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

static inline
QDF_STATUS cm_check_for_hw_mode_change(struct wlan_objmgr_psoc *psoc,
				       qdf_list_t *scan_list, uint8_t vdev_id,
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
#endif /* WLAN_POLICY_MGR_ENABLE */

static inline void cm_delete_pmksa_for_bssid(struct cnx_mgr *cm_ctx,
					     struct qdf_mac_addr *bssid)
{
	struct wlan_crypto_pmksa pmksa;

	qdf_mem_zero(&pmksa, sizeof(pmksa));
	qdf_copy_macaddr(&pmksa.bssid, bssid);
	wlan_crypto_set_del_pmksa(cm_ctx->vdev, &pmksa, false);
}

#if defined(WLAN_SAE_SINGLE_PMK) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
static inline
void cm_delete_pmksa_for_single_pmk_bssid(struct cnx_mgr *cm_ctx,
					  struct qdf_mac_addr *bssid)
{
	cm_delete_pmksa_for_bssid(cm_ctx, bssid);
}
#else
static inline
void cm_delete_pmksa_for_single_pmk_bssid(struct cnx_mgr *cm_ctx,
					  struct qdf_mac_addr *bssid)
{
}
#endif /* WLAN_SAE_SINGLE_PMK && WLAN_FEATURE_ROAM_OFFLOAD */

static inline void
cm_set_pmf_caps(struct wlan_cm_connect_req *req, struct scan_filter *filter)
{
	if (req->crypto.rsn_caps & WLAN_CRYPTO_RSN_CAP_MFP_REQUIRED)
		filter->pmf_cap = WLAN_PMF_REQUIRED;
	else if (req->crypto.rsn_caps & WLAN_CRYPTO_RSN_CAP_MFP_ENABLED)
		filter->pmf_cap = WLAN_PMF_CAPABLE;
	else
		filter->pmf_cap = WLAN_PMF_DISABLED;
}

#ifdef CONN_MGR_ADV_FEATURE
#ifdef WLAN_FEATURE_FILS_SK
/*
 * cm_create_fils_realm_hash: API to create hash using realm
 * @fils_info: fils connection info obtained from supplicant
 * @tmp_hash: pointer to new hash
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
cm_create_fils_realm_hash(struct wlan_fils_con_info *fils_info,
			  uint8_t *tmp_hash)
{
	uint8_t *hash;
	uint8_t *data;

	if (!fils_info->realm_len)
		return QDF_STATUS_E_NOSUPPORT;

	hash = qdf_mem_malloc(SHA256_DIGEST_SIZE);
	if (!hash)
		return QDF_STATUS_E_NOMEM;

	data = fils_info->realm;
	qdf_get_hash(SHA256_CRYPTO_TYPE, 1, &data, &fils_info->realm_len, hash);
	qdf_mem_copy(tmp_hash, hash, REALM_HASH_LEN);
	qdf_mem_free(hash);

	return QDF_STATUS_SUCCESS;
}

static void cm_update_fils_scan_filter(struct scan_filter *filter,
				       struct cm_connect_req *cm_req)

{
	uint8_t realm_hash[REALM_HASH_LEN];
	QDF_STATUS status;

	if (!cm_req->req.fils_info.is_fils_connection)
		return;

	status = cm_create_fils_realm_hash(&cm_req->req.fils_info, realm_hash);
	if (QDF_IS_STATUS_ERROR(status))
		return;

	filter->fils_scan_filter.realm_check = true;
	mlme_debug(CM_PREFIX_FMT "creating realm based on fils info",
		   CM_PREFIX_REF(cm_req->req.vdev_id, cm_req->cm_id));
	qdf_mem_copy(filter->fils_scan_filter.fils_realm, realm_hash,
		     REALM_HASH_LEN);
}

static inline bool cm_is_fils_connection(struct cnx_mgr *cm_ctx,
					 struct wlan_cm_connect_resp *resp)
{
	int32_t key_mgmt;

	key_mgmt = wlan_crypto_get_param(cm_ctx->vdev,
					 WLAN_CRYPTO_PARAM_KEY_MGMT);

	if (!(key_mgmt & (1 << WLAN_CRYPTO_KEY_MGMT_FILS_SHA256 |
			  1 << WLAN_CRYPTO_KEY_MGMT_FILS_SHA384 |
			  1 << WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA256 |
			  1 << WLAN_CRYPTO_KEY_MGMT_FT_FILS_SHA384)))
		return false;

	resp->is_fils_connection = true;

	return true;
}

static QDF_STATUS cm_set_fils_key(struct cnx_mgr *cm_ctx,
				  struct wlan_cm_connect_resp *resp)
{
	struct fils_connect_rsp_params *fils_ie;

	fils_ie = resp->connect_ies.fils_ie;

	if (!fils_ie)
		return QDF_STATUS_E_INVAL;

	cm_store_fils_key(cm_ctx, true, 0, fils_ie->tk_len, fils_ie->tk,
			  &resp->bssid, resp->cm_id);
	cm_store_fils_key(cm_ctx, false, 2, fils_ie->gtk_len, fils_ie->gtk,
			  &resp->bssid, resp->cm_id);
	cm_set_key(cm_ctx, true, 0, &resp->bssid);
	cm_set_key(cm_ctx, false, 2, &resp->bssid);

	return QDF_STATUS_SUCCESS;
}

#else
static inline void cm_update_fils_scan_filter(struct scan_filter *filter,
					      struct cm_connect_req *cm_req)
{ }

static inline bool cm_is_fils_connection(struct cnx_mgr *cm_ctx,
					 struct wlan_cm_connect_resp *resp)
{
	return false;
}

static inline QDF_STATUS cm_set_fils_key(struct cnx_mgr *cm_ctx,
					 struct wlan_cm_connect_resp *resp)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_FILS_SK */

static QDF_STATUS
cm_inform_blm_connect_complete(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_connect_resp *resp)
{
	struct wlan_objmgr_pdev *pdev;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "Failed to find pdev",
			 CM_PREFIX_REF(wlan_vdev_get_id(vdev), resp->cm_id));
		return QDF_STATUS_E_FAILURE;
	}

	if (QDF_IS_STATUS_SUCCESS(resp->connect_status))
		wlan_blm_update_bssid_connect_params(pdev, resp->bssid,
						     BLM_AP_CONNECTED);

	return QDF_STATUS_SUCCESS;
}

/**
 * cm_is_retry_with_same_candidate() - This API check if reconnect attempt is
 * required with the same candidate again
 * @cm_ctx: connection manager context
 * @req: Connect request.
 * @resp: connect resp from previous connection attempt
 *
 * This function return true if same candidate needs to be tried again
 *
 * Return: bool
 */
static bool cm_is_retry_with_same_candidate(struct cnx_mgr *cm_ctx,
					    struct cm_connect_req *req,
					    struct wlan_cm_connect_resp *resp)
{
	uint8_t max_retry_count = CM_MAX_CANDIDATE_RETRIES;
	uint32_t key_mgmt;
	struct wlan_objmgr_psoc *psoc;
	bool sae_connection;

	psoc = wlan_pdev_get_psoc(wlan_vdev_get_pdev(cm_ctx->vdev));
	key_mgmt = req->cur_candidate->entry->neg_sec_info.key_mgmt;

	/* Try once again for the invalid PMKID case without PMKID */
	if (resp->status_code == STATUS_INVALID_PMKID)
		goto use_same_candidate;

	/* Try again for the JOIN timeout if only one candidate */
	if (resp->reason == CM_JOIN_TIMEOUT &&
	    qdf_list_size(req->candidate_list) == 1) {
		/* Get assoc retry count */
		wlan_mlme_get_sae_assoc_retry_count(psoc, &max_retry_count);
		goto use_same_candidate;
	}

	/*
	 * Try again for the ASSOC timeout in SAE connection or
	 * AP has reconnect on assoc timeout OUI.
	 */
	sae_connection = key_mgmt & (1 << WLAN_CRYPTO_KEY_MGMT_SAE |
				     1 << WLAN_CRYPTO_KEY_MGMT_FT_SAE);
	if (resp->reason == CM_ASSOC_TIMEOUT && (sae_connection ||
	    (mlme_get_reconn_after_assoc_timeout_flag(psoc, resp->vdev_id)))) {
		/* For SAE use max retry count from INI */
		if (sae_connection)
			wlan_mlme_get_sae_assoc_retry_count(psoc,
							    &max_retry_count);
		goto use_same_candidate;
	}

	return false;

use_same_candidate:
	if (req->cur_candidate_retries >= max_retry_count)
		return false;

	mlme_info(CM_PREFIX_FMT "Retry again with " QDF_MAC_ADDR_FMT ", status code %d reason %d key_mgmt 0x%x retry count %d max retry %d",
		  CM_PREFIX_REF(resp->vdev_id, resp->cm_id),
		  QDF_MAC_ADDR_REF(resp->bssid.bytes), resp->status_code,
		  resp->reason, key_mgmt, req->cur_candidate_retries,
		  max_retry_count);

	req->cur_candidate_retries++;

	return true;
}

static inline void cm_update_advance_filter(struct wlan_objmgr_pdev *pdev,
					    struct cnx_mgr *cm_ctx,
					    struct scan_filter *filter,
					    struct cm_connect_req *cm_req)
{
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	filter->enable_adaptive_11r =
		wlan_mlme_adaptive_11r_enabled(psoc);
	if (wlan_vdev_mlme_get_opmode(cm_ctx->vdev) != QDF_STA_MODE)
		return;

	wlan_cm_dual_sta_roam_update_connect_channels(psoc, filter);
	filter->dot11mode = cm_req->req.dot11mode_filter;
	cm_update_fils_scan_filter(filter, cm_req);
}

static void cm_update_security_filter(struct scan_filter *filter,
				      struct wlan_cm_connect_req *req)
{
	uint8_t wsc_oui[OUI_LENGTH];
	uint8_t osen_oui[OUI_LENGTH];
	uint32_t oui_cpu;

	oui_cpu = qdf_be32_to_cpu(WSC_OUI);
	qdf_mem_copy(wsc_oui, &oui_cpu, OUI_LENGTH);
	oui_cpu = qdf_be32_to_cpu(OSEN_OUI);
	qdf_mem_copy(osen_oui, &oui_cpu, OUI_LENGTH);

	/* Ignore security match for rsn override, OSEN and WPS connection */
	if (req->force_rsne_override ||
	    wlan_get_vendor_ie_ptr_from_oui(wsc_oui, OUI_LENGTH,
					    req->assoc_ie.ptr,
					    req->assoc_ie.len) ||
	    wlan_get_vendor_ie_ptr_from_oui(osen_oui, OUI_LENGTH,
					    req->assoc_ie.ptr,
					    req->assoc_ie.len)) {
		filter->ignore_auth_enc_type = 1;
		return;
	}

	filter->authmodeset = req->crypto.auth_type;
	filter->ucastcipherset = req->crypto.ciphers_pairwise;
	filter->key_mgmt = req->crypto.akm_suites;
	filter->mcastcipherset = req->crypto.group_cipher;
	filter->mgmtcipherset = req->crypto.mgmt_ciphers;
	cm_set_pmf_caps(req, filter);
}

/**
 * cm_set_fils_wep_key() - check and set wep or fils keys if required
 * @cm_ctx: connection manager context
 * @resp: connect resp
 *
 * Context: Can be called from any context and to be used only after posting a
 * msg to SM (ie holding the SM lock) i.e. on successful connection.
 */
static void cm_set_fils_wep_key(struct cnx_mgr *cm_ctx,
				struct wlan_cm_connect_resp *resp)
{
	int32_t cipher;
	struct qdf_mac_addr broadcast_mac = QDF_MAC_ADDR_BCAST_INIT;

	/* Check and set FILS keys */
	if (cm_is_fils_connection(cm_ctx, resp)) {
		cm_set_fils_key(cm_ctx, resp);
		return;
	}
	/* Check and set WEP keys */
	cipher = wlan_crypto_get_param(cm_ctx->vdev,
				       WLAN_CRYPTO_PARAM_UCAST_CIPHER);
	if (cipher < 0)
		return;

	if (!(cipher & (1 << WLAN_CRYPTO_CIPHER_WEP_40 |
			1 << WLAN_CRYPTO_CIPHER_WEP_104)))
		return;

	cm_set_key(cm_ctx, true, 0, &resp->bssid);
	cm_set_key(cm_ctx, false, 0, &broadcast_mac);
}
#else
static inline QDF_STATUS
cm_inform_blm_connect_complete(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_connect_resp *resp)
{
	return QDF_STATUS_SUCCESS;
}

static inline
bool cm_is_retry_with_same_candidate(struct cnx_mgr *cm_ctx,
				     struct cm_connect_req *req,
				     struct wlan_cm_connect_resp *resp)
{
	return false;
}

static inline void cm_update_advance_filter(struct wlan_objmgr_pdev *pdev,
					    struct cnx_mgr *cm_ctx,
					    struct scan_filter *filter,
					    struct cm_connect_req *cm_req)
{ }

static void cm_update_security_filter(struct scan_filter *filter,
				      struct wlan_cm_connect_req *req)
{
	if (!QDF_HAS_PARAM(req->crypto.auth_type, WLAN_CRYPTO_AUTH_WAPI) &&
	    !QDF_HAS_PARAM(req->crypto.auth_type, WLAN_CRYPTO_AUTH_RSNA) &&
	    !QDF_HAS_PARAM(req->crypto.auth_type, WLAN_CRYPTO_AUTH_WPA)) {
		filter->ignore_auth_enc_type = 1;
		return;
	}

	filter->authmodeset = req->crypto.auth_type;
	filter->ucastcipherset = req->crypto.ciphers_pairwise;
	filter->key_mgmt = req->crypto.akm_suites;
	filter->mcastcipherset = req->crypto.group_cipher;
	filter->mgmtcipherset = req->crypto.mgmt_ciphers;
	cm_set_pmf_caps(req, filter);
}

static inline void cm_set_fils_wep_key(struct cnx_mgr *cm_ctx,
				       struct wlan_cm_connect_resp *resp)
{}

#endif /* CONN_MGR_ADV_FEATURE */

static void cm_connect_prepare_scan_filter(struct wlan_objmgr_pdev *pdev,
					   struct cnx_mgr *cm_ctx,
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

	cm_update_security_filter(filter, &cm_req->req);
	cm_update_advance_filter(pdev, cm_ctx, filter, cm_req);
}

static QDF_STATUS cm_connect_get_candidates(struct wlan_objmgr_pdev *pdev,
					    struct cnx_mgr *cm_ctx,
					    struct cm_connect_req *cm_req)
{
	struct scan_filter *filter;
	uint32_t num_bss = 0;
	enum QDF_OPMODE op_mode;
	qdf_list_t *candidate_list;
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);

	filter = qdf_mem_malloc(sizeof(*filter));
	if (!filter)
		return QDF_STATUS_E_NOMEM;

	cm_connect_prepare_scan_filter(pdev, cm_ctx, cm_req, filter);

	candidate_list = wlan_scan_get_result(pdev, filter);
	if (candidate_list) {
		num_bss = qdf_list_size(candidate_list);
		mlme_debug(CM_PREFIX_FMT "num_entries found %d",
			   CM_PREFIX_REF(vdev_id, cm_req->cm_id), num_bss);
	}

	op_mode = wlan_vdev_mlme_get_opmode(cm_ctx->vdev);
	if (num_bss && op_mode == QDF_STA_MODE)
		cm_calculate_scores(pdev, filter, candidate_list);
	qdf_mem_free(filter);

	if (!candidate_list || !qdf_list_size(candidate_list)) {
		QDF_STATUS status;

		if (candidate_list)
			wlan_scan_purge_results(candidate_list);
		mlme_info(CM_PREFIX_FMT "no valid candidate found, num_bss %d scan_id %d",
			  CM_PREFIX_REF(vdev_id, cm_req->cm_id), num_bss,
			  cm_req->scan_id);

		/*
		 * If connect scan was already done OR candidate were found
		 * but none of them were valid return QDF_STATUS_E_EMPTY.
		 */
		if (cm_req->scan_id || num_bss)
			return QDF_STATUS_E_EMPTY;

		/* Try connect scan to search for any valid candidate */
		status = cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_SCAN,
						  sizeof(*cm_req), cm_req);
		/*
		 * If connect scan is initiated, return pending, so that
		 * connect start after scan complete
		 */
		if (QDF_IS_STATUS_SUCCESS(status))
			status = QDF_STATUS_E_PENDING;

		return status;
	}
	cm_req->candidate_list = candidate_list;

	return QDF_STATUS_SUCCESS;
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

static QDF_STATUS
cm_inform_if_mgr_connect_start(struct wlan_objmgr_vdev *vdev)
{
	return if_mgr_deliver_event(vdev, WLAN_IF_MGR_EV_CONNECT_START, NULL);
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

static inline QDF_STATUS
cm_inform_if_mgr_connect_start(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

#endif

QDF_STATUS
cm_handle_connect_req_in_non_init_state(struct cnx_mgr *cm_ctx,
					struct cm_connect_req *cm_req,
					enum wlan_cm_sm_state cm_state_substate)
{
	switch (cm_state_substate) {
	case WLAN_CM_S_CONNECTED:
	case WLAN_CM_SS_JOIN_ACTIVE:
		/*
		 * In connected state, there would be no pending command, so
		 * for new connect request, queue internal disconnect
		 *
		 * In join active state there would be only one active connect
		 * request in the cm req list, so to abort at certain stages and
		 * to cleanup after its completion, queue internal disconnect.
		 */
		cm_initiate_internal_disconnect(cm_ctx);
		break;
	case WLAN_CM_SS_SCAN:
		/* In the scan state abort the ongoing scan */
		cm_vdev_scan_cancel(wlan_vdev_get_pdev(cm_ctx->vdev),
				    cm_ctx->vdev);
		/* fallthrough */
	case WLAN_CM_SS_JOIN_PENDING:
		/*
		 * In case of scan or join pending there could be 2 scenarios:-
		 *
		 * 1. There is a connect request pending, so just remove
		 *    the pending connect req. As we will queue a new connect
		 *    req, all resp for pending connect req will be dropped.
		 * 2. There is a connect request in active and
		 *    and a internal disconnect followed by a connect req in
		 *    pending. In this case the disconnect will take care of
		 *    cleaning up the active connect request and thus only
		 *    remove the pending connect.
		 */
		cm_flush_pending_request(cm_ctx, CONNECT_REQ_PREFIX, false);
		break;
	case WLAN_CM_S_DISCONNECTING:
		/*
		 * Flush failed pending connect req as new req is received
		 * and its no longer the latest one.
		 */
		if (cm_ctx->connect_count)
			cm_flush_pending_request(cm_ctx, CONNECT_REQ_PREFIX,
						 true);
		/*
		 * In case of disconnecting state, there could be 2 scenarios:-
		 * In both case no state specific action is required.
		 * 1. There is disconnect request in the cm_req list, no action
		 *    required to cleanup.
		 *    so just add the connect request to the list.
		 * 2. There is a connect request activated, followed by
		 *    disconnect in pending queue. So keep the disconenct
		 *    to cleanup the active connect and no action required to
		 *    cleanup.
		 */
		break;
	default:
		mlme_err("Vdev %d Connect req in invalid state %d",
			 wlan_vdev_get_id(cm_ctx->vdev),
			 cm_state_substate);
		return QDF_STATUS_E_FAILURE;
	};

	/* Queue the new connect request after state specific actions */
	return cm_add_connect_req_to_list(cm_ctx, cm_req);
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
		mlme_err(CM_PREFIX_FMT "Failed to find pdev",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id));
		goto connect_err;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "Failed to find psoc",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id));
		goto connect_err;
	}

	cm_inform_if_mgr_connect_start(cm_ctx->vdev);
	mlme_cm_connect_start_ind(cm_ctx->vdev, &cm_req->req);

	status = cm_connect_get_candidates(pdev, cm_ctx, cm_req);
	/* In case of status pending connect will continue after scan */
	if (status == QDF_STATUS_E_PENDING)
		return QDF_STATUS_SUCCESS;
	if (QDF_IS_STATUS_ERROR(status)) {
		reason = CM_NO_CANDIDATE_FOUND;
		goto connect_err;
	}

	status = cm_check_for_hw_mode_change(psoc, cm_req->candidate_list,
					     vdev_id, cm_req->cm_id);
	if (QDF_IS_STATUS_ERROR(status) && status != QDF_STATUS_E_ALREADY) {
		reason = CM_HW_MODE_FAILURE;
		mlme_err(CM_PREFIX_FMT "Failed to set HW mode change",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id));
		goto connect_err;
	} else if (QDF_IS_STATUS_SUCCESS(status)) {
		mlme_debug(CM_PREFIX_FMT "Connect will continue after HW mode change",
			   CM_PREFIX_REF(vdev_id, cm_req->cm_id));
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

/**
 * cm_get_valid_candidate() - This API will be called to get the next valid
 * candidate
 * @cm_ctx: connection manager context
 * @cm_req: Connect request.
 * @resp: connect resp from previous connection attempt
 * @same_candidate_used: this will be set if same candidate used
 *
 * This function return a valid candidate to try connection. It return failure
 * if no valid candidate is present or all valid candidate are tried.
 *
 * Return: QDF status
 */
static QDF_STATUS cm_get_valid_candidate(struct cnx_mgr *cm_ctx,
					 struct cm_req *cm_req,
					 struct wlan_cm_connect_resp *resp,
					 bool *same_candidate_used)
{
	struct scan_cache_node *scan_node = NULL;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct scan_cache_node *new_candidate = NULL, *prev_candidate;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	bool use_same_candidate = false;

	prev_candidate = cm_req->connect_req.cur_candidate;
	if (cm_req->connect_req.connect_attempts >=
	    cm_ctx->max_connect_attempts) {
		mlme_info(CM_PREFIX_FMT "%d attempts tried, max %d",
			  CM_PREFIX_REF(vdev_id, cm_req->cm_id),
			  cm_req->connect_req.connect_attempts,
			  cm_ctx->max_connect_attempts);
		status = QDF_STATUS_E_FAILURE;
		goto flush_single_pmk;
	}

	if (prev_candidate && resp &&
	    cm_is_retry_with_same_candidate(cm_ctx, &cm_req->connect_req,
					    resp)) {
		new_candidate = prev_candidate;
		use_same_candidate = true;
		goto try_same_candidate;
	}

	/*
	 * Get next candidate if prev_candidate is not NULL, else get
	 * the first candidate
	 */
	if (prev_candidate)
		qdf_list_peek_next(cm_req->connect_req.candidate_list,
				   &prev_candidate->node, &cur_node);
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
			new_candidate = scan_node;
			break;
		}

		cur_node = next_node;
		next_node = NULL;
	}

	/*
	 * If cur_node is NULL prev candidate was last to be tried so no more
	 * candidates left for connect now.
	 */
	if (!cur_node) {
		mlme_debug(CM_PREFIX_FMT "No more candidates left",
			   CM_PREFIX_REF(vdev_id, cm_req->cm_id));
		cm_req->connect_req.cur_candidate = NULL;
		status = QDF_STATUS_E_FAILURE;
		goto flush_single_pmk;
	}

	/* Reset current candidate retries when a new candidate is tried */
	cm_req->connect_req.cur_candidate_retries = 0;

try_same_candidate:
	cm_req->connect_req.connect_attempts++;
	cm_req->connect_req.cur_candidate = new_candidate;

flush_single_pmk:
	/*
	 * If connection fails with Single PMK bssid (prev candidate),
	 * clear the pmk entry. Flush only in case if we are not trying again
	 * with same candidate again.
	 */
	if (prev_candidate && !use_same_candidate &&
	    util_scan_entry_single_pmk(prev_candidate->entry))
		cm_delete_pmksa_for_single_pmk_bssid(cm_ctx,
						&prev_candidate->entry->bssid);

	if (same_candidate_used)
		*same_candidate_used = use_same_candidate;

	return status;
}

static void cm_create_bss_peer(struct cnx_mgr *cm_ctx,
			       struct cm_connect_req *req)
{
	QDF_STATUS status;
	struct qdf_mac_addr *bssid;

	bssid = &req->cur_candidate->entry->bssid;
	status = mlme_cm_bss_peer_create_req(cm_ctx->vdev, bssid);
	if (QDF_IS_STATUS_ERROR(status)) {
		struct wlan_cm_connect_resp *resp;
		uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);

		/* In case of failure try with next candidate */
		mlme_err(CM_PREFIX_FMT "peer create request failed %d",
			 CM_PREFIX_REF(vdev_id, req->cm_id), status);

		resp = qdf_mem_malloc(sizeof(*resp));
		if (!resp)
			return;

		cm_fill_failure_resp_from_cm_id(cm_ctx, resp, req->cm_id,
						CM_PEER_CREATE_FAILED);
		cm_sm_deliver_event_sync(cm_ctx,
				WLAN_CM_SM_EV_CONNECT_GET_NEXT_CANDIDATE,
				sizeof(*resp), resp);
		qdf_mem_free(resp);
	}
}

static QDF_STATUS
cm_send_bss_select_ind(struct cnx_mgr *cm_ctx, struct cm_connect_req *req)
{
	QDF_STATUS status;
	struct wlan_cm_vdev_connect_req vdev_req;
	struct wlan_cm_connect_resp *resp;

	vdev_req.vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	vdev_req.cm_id = req->cm_id;
	vdev_req.bss = req->cur_candidate;

	status = mlme_cm_bss_select_ind(cm_ctx->vdev, &vdev_req);
	if (QDF_IS_STATUS_SUCCESS(status) ||
	    status == QDF_STATUS_E_NOSUPPORT)
		return status;

	/* In supported and failure try with next candidate */
	mlme_err(CM_PREFIX_FMT "mlme candidate select indication failed %d",
		 CM_PREFIX_REF(vdev_req.vdev_id, req->cm_id), status);
	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return QDF_STATUS_E_FAILURE;

	cm_fill_failure_resp_from_cm_id(cm_ctx, resp, req->cm_id,
					CM_BSS_SELECT_IND_FAILED);
	cm_sm_deliver_event_sync(cm_ctx,
				 WLAN_CM_SM_EV_CONNECT_GET_NEXT_CANDIDATE,
				 sizeof(*resp), resp);
	qdf_mem_free(resp);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_try_next_candidate(struct cnx_mgr *cm_ctx,
				 struct wlan_cm_connect_resp *resp)
{
	QDF_STATUS status;
	struct cm_req *cm_req;
	bool same_candidate_used = false;

	cm_req = cm_get_req_by_cm_id(cm_ctx, resp->cm_id);
	if (!cm_req)
		return QDF_STATUS_E_FAILURE;

	status = cm_get_valid_candidate(cm_ctx, cm_req, resp,
					&same_candidate_used);
	if (QDF_IS_STATUS_ERROR(status))
		goto connect_err;

	/*
	 * Do not indicate to OSIF if same candidate is used again as we are not
	 * done with this candidate. So inform once we move to next candidate.
	 * This will also avoid flush for the scan entry.
	 */
	if (!same_candidate_used)
		mlme_cm_osif_failed_candidate_ind(cm_ctx->vdev, resp);

	status = cm_send_bss_select_ind(cm_ctx, &cm_req->connect_req);
	/*
	 * If candidate select indication is not supported continue with bss
	 * peer create, else peer will be created after resp.
	 */
	if (status == QDF_STATUS_E_NOSUPPORT)
		cm_create_bss_peer(cm_ctx, &cm_req->connect_req);
	else if (QDF_IS_STATUS_ERROR(status))
		goto connect_err;

	return QDF_STATUS_SUCCESS;

connect_err:
	return cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_CONNECT_FAILURE,
					sizeof(*resp), resp);

}

bool cm_connect_resp_cmid_match_list_head(struct cnx_mgr *cm_ctx,
					  struct wlan_cm_connect_resp *resp)
{
	return cm_check_cmid_match_list_head(cm_ctx, &resp->cm_id);
}

static void cm_fill_vdev_crypto_params(struct cnx_mgr *cm_ctx,
				       struct wlan_cm_connect_req *req)
{
	/* fill vdev crypto from the connect req */
	wlan_crypto_set_vdev_param(cm_ctx->vdev, WLAN_CRYPTO_PARAM_AUTH_MODE,
				   req->crypto.auth_type);
	wlan_crypto_set_vdev_param(cm_ctx->vdev, WLAN_CRYPTO_PARAM_KEY_MGMT,
				   req->crypto.akm_suites);
	wlan_crypto_set_vdev_param(cm_ctx->vdev, WLAN_CRYPTO_PARAM_UCAST_CIPHER,
				   req->crypto.ciphers_pairwise);
	wlan_crypto_set_vdev_param(cm_ctx->vdev, WLAN_CRYPTO_PARAM_MCAST_CIPHER,
				   req->crypto.group_cipher);
	wlan_crypto_set_vdev_param(cm_ctx->vdev, WLAN_CRYPTO_PARAM_MGMT_CIPHER,
				   req->crypto.mgmt_ciphers);
	wlan_crypto_set_vdev_param(cm_ctx->vdev, WLAN_CRYPTO_PARAM_RSN_CAP,
				   req->crypto.rsn_caps);
}

QDF_STATUS cm_connect_active(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	struct cm_req *cm_req;
	QDF_STATUS status;
	struct wlan_cm_connect_req *req;

	cm_req = cm_get_req_by_cm_id(cm_ctx, *cm_id);
	if (!cm_req)
		return QDF_STATUS_E_INVAL;

	cm_ctx->active_cm_id = *cm_id;
	req = &cm_req->connect_req.req;
	wlan_vdev_mlme_set_ssid(cm_ctx->vdev, req->ssid.ssid, req->ssid.length);
	cm_fill_vdev_crypto_params(cm_ctx, req);
	cm_store_wep_key(cm_ctx, &req->crypto, *cm_id);

	status = cm_get_valid_candidate(cm_ctx, cm_req, NULL, NULL);
	if (QDF_IS_STATUS_ERROR(status))
		goto connect_err;

	status = cm_send_bss_select_ind(cm_ctx, &cm_req->connect_req);
	/*
	 * If candidate select indication is not supported continue with bss
	 * peer create, else peer will be created after resp.
	 */
	if (status == QDF_STATUS_E_NOSUPPORT)
		cm_create_bss_peer(cm_ctx, &cm_req->connect_req);
	else if (QDF_IS_STATUS_ERROR(status))
		goto connect_err;

	return QDF_STATUS_SUCCESS;

connect_err:
	return cm_send_connect_start_fail(cm_ctx,
					  &cm_req->connect_req, CM_JOIN_FAILED);
}

QDF_STATUS
cm_peer_create_on_bss_select_ind_resp(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	struct cm_req *cm_req;

	cm_req = cm_get_req_by_cm_id(cm_ctx, *cm_id);
	if (!cm_req)
		return QDF_STATUS_E_FAILURE;

	cm_create_bss_peer(cm_ctx, &cm_req->connect_req);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_FILS_SK
static void cm_copy_fils_info(struct wlan_cm_vdev_connect_req *req,
			      struct cm_req *cm_req)
{
	req->fils_info = &cm_req->connect_req.req.fils_info;
}
#else
static inline void cm_copy_fils_info(struct wlan_cm_vdev_connect_req *req,
				     struct cm_req *cm_req)
{
}
#endif

QDF_STATUS
cm_resume_connect_after_peer_create(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	struct wlan_cm_vdev_connect_req req;
	struct cm_req *cm_req;
	QDF_STATUS status;
	struct security_info *neg_sec_info;
	uint16_t rsn_caps;
	uint8_t country_code[REG_ALPHA2_LEN + 1];
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(wlan_vdev_get_pdev(cm_ctx->vdev));

	cm_req = cm_get_req_by_cm_id(cm_ctx, *cm_id);
	if (!cm_req)
		return QDF_STATUS_E_FAILURE;

	/*
	 * Some non PMF AP misbehave if in assoc req RSN IE contain PMF capable
	 * bit set. Thus only if AP and self are capable, try PMF connection
	 * else set PMF as 0. The PMF filtering is already taken care in
	 * get scan results.
	 */
	neg_sec_info = &cm_req->connect_req.cur_candidate->entry->neg_sec_info;
	rsn_caps = cm_req->connect_req.req.crypto.rsn_caps;
	if (!(neg_sec_info->rsn_caps & WLAN_CRYPTO_RSN_CAP_MFP_ENABLED &&
	      rsn_caps & WLAN_CRYPTO_RSN_CAP_MFP_ENABLED)) {
		rsn_caps &= ~WLAN_CRYPTO_RSN_CAP_MFP_ENABLED;
		rsn_caps &= ~WLAN_CRYPTO_RSN_CAP_MFP_REQUIRED;
		rsn_caps &= ~WLAN_CRYPTO_RSN_CAP_OCV_SUPPORTED;
		wlan_crypto_set_vdev_param(cm_ctx->vdev,
					   WLAN_CRYPTO_PARAM_RSN_CAP,
					   rsn_caps);
	}

	req.vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	req.cm_id = *cm_id;
	req.force_rsne_override = cm_req->connect_req.req.force_rsne_override;
	req.assoc_ie = cm_req->connect_req.req.assoc_ie;
	req.scan_ie = cm_req->connect_req.req.scan_ie;
	req.bss = cm_req->connect_req.cur_candidate;
	cm_copy_fils_info(&req, cm_req);
	req.ht_caps = cm_req->connect_req.req.ht_caps;
	req.ht_caps_mask = cm_req->connect_req.req.ht_caps_mask;
	req.vht_caps = cm_req->connect_req.req.vht_caps;
	req.vht_caps_mask = cm_req->connect_req.req.vht_caps_mask;

	wlan_reg_get_cc_and_src(psoc, country_code);
	mlme_nofl_info(CM_PREFIX_FMT "Connecting to %.*s " QDF_MAC_ADDR_FMT " rssi: %d freq: %d akm 0x%x cipher: uc 0x%x mc 0x%x, CC: %c%c",
		       CM_PREFIX_REF(req.vdev_id, req.cm_id),
		       cm_req->connect_req.req.ssid.length,
		       cm_req->connect_req.req.ssid.ssid,
		       QDF_MAC_ADDR_REF(req.bss->entry->bssid.bytes),
		       req.bss->entry->rssi_raw,
		       req.bss->entry->channel.chan_freq,
		       neg_sec_info->key_mgmt, neg_sec_info->ucastcipherset,
		       neg_sec_info->mcastcipherset, country_code[0],
		       country_code[1]);

	status = mlme_cm_connect_req(cm_ctx->vdev, &req);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "connect request failed",
			 CM_PREFIX_REF(req.vdev_id, req.cm_id));
		status = cm_send_connect_start_fail(cm_ctx,
						    &cm_req->connect_req,
						    CM_JOIN_FAILED);
	}

	return status;
}

/**
 * cm_update_scan_db_on_connect_success() - update scan db with beacon or
 * probe resp
 * @cm_ctx: connection manager context
 * @resp: connect resp
 *
 * update scan db, so that kernel and driver do not age out
 * the connected AP entry.
 *
 * Context: Can be called from any context and to be used only if connect
 * is successful and SM is in conencted state. i.e. SM lock is hold.
 *
 * Return: void
 */
static void
cm_update_scan_db_on_connect_success(struct cnx_mgr *cm_ctx,
				     struct wlan_cm_connect_resp *resp)
{
	struct element_info *bcn_probe_rsp;
	struct cm_req *cm_req;
	int32_t rssi;

	if (!cm_is_vdev_connected(cm_ctx->vdev))
		return;

	cm_req = cm_get_req_by_cm_id(cm_ctx, resp->cm_id);
	if (!cm_req)
		return;
	if (!cm_req->connect_req.cur_candidate)
		return;

	/*
	 * Get beacon or probe resp from connect response, and if not present
	 * use cur candidate to get beacon or probe resp
	 */
	if (resp->connect_ies.bcn_probe_rsp.ptr)
		bcn_probe_rsp = &resp->connect_ies.bcn_probe_rsp;
	else
		bcn_probe_rsp =
			&cm_req->connect_req.cur_candidate->entry->raw_frame;

	rssi = cm_req->connect_req.cur_candidate->entry->rssi_raw;

	cm_inform_bcn_probe(cm_ctx, bcn_probe_rsp->ptr, bcn_probe_rsp->len,
			    resp->freq, rssi, resp->cm_id);
}

QDF_STATUS cm_connect_complete(struct cnx_mgr *cm_ctx,
			       struct wlan_cm_connect_resp *resp)
{
	enum wlan_cm_sm_state sm_state;
	struct bss_info bss_info;
	struct mlme_info mlme_info;

	/*
	 * If the entry is not present in the list, it must have been cleared
	 * already.
	 */
	if (!cm_get_req_by_cm_id(cm_ctx, resp->cm_id))
		return QDF_STATUS_SUCCESS;

	sm_state = cm_get_state(cm_ctx);
	if (QDF_IS_STATUS_SUCCESS(resp->connect_status) &&
	    sm_state == WLAN_CM_S_CONNECTED) {
		cm_update_scan_db_on_connect_success(cm_ctx, resp);
		/* set WEP and FILS key on success */
		cm_set_fils_wep_key(cm_ctx, resp);
	}

	mlme_cm_connect_complete_ind(cm_ctx->vdev, resp);
	mlme_cm_osif_connect_complete(cm_ctx->vdev, resp);
	cm_inform_if_mgr_connect_complete(cm_ctx->vdev, resp->connect_status);
	cm_inform_blm_connect_complete(cm_ctx->vdev, resp);

	/* Update scan entry in case connect is success or fails with bssid */
	if (!qdf_is_macaddr_zero(&resp->bssid)) {
		if (QDF_IS_STATUS_SUCCESS(resp->connect_status))
			mlme_info.assoc_state  = SCAN_ENTRY_CON_STATE_ASSOC;
		else
			mlme_info.assoc_state = SCAN_ENTRY_CON_STATE_NONE;
		qdf_copy_macaddr(&bss_info.bssid, &resp->bssid);
		bss_info.freq = resp->freq;
		bss_info.ssid.length = resp->ssid.length;
		qdf_mem_copy(&bss_info.ssid.ssid, resp->ssid.ssid,
			     bss_info.ssid.length);
		wlan_scan_update_mlme_by_bssinfo(
					wlan_vdev_get_pdev(cm_ctx->vdev),
					&bss_info, &mlme_info);
	}
	/*
	 * update fils/wep key and inform legacy, update bcn filter,
	 * start OBSS scan for open mode.
	 */

	mlme_debug(CM_PREFIX_FMT,
		   CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
				 resp->cm_id));
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
			  struct wlan_cm_connect_resp *resp)
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

	if (prefix != CONNECT_REQ_PREFIX || cm_id != resp->cm_id) {
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
			wlan_crypto_selective_clear_sae_single_pmk_entries(vdev,
								&resp->bssid);
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

	/*
	 * Delete the PMKID of the BSSID for which the assoc reject is
	 * received from the AP due to invalid PMKID reason.
	 * This will avoid the driver trying to connect to same AP with
	 * the same stale PMKID. when connection is tried again with this AP.
	 */
	if (resp->status_code == STATUS_INVALID_PMKID)
		cm_delete_pmksa_for_bssid(cm_ctx, &resp->bssid);

	/* In case of failure try with next candidate */
	qdf_status =
		cm_sm_deliver_event(vdev,
				    WLAN_CM_SM_EV_CONNECT_GET_NEXT_CANDIDATE,
				    sizeof(*resp), resp);

	if (QDF_IS_STATUS_SUCCESS(qdf_status))
		return qdf_status;
	/*
	 * If connection fails with Single PMK bssid, clear this pmk
	 * entry in case of post failure.
	 */
	if (cm_is_cm_id_current_candidate_single_pmk(cm_ctx, cm_id))
		cm_delete_pmksa_for_single_pmk_bssid(cm_ctx, &resp->bssid);
post_err:
	/*
	 * If there is a event posting error it means the SM state is not in
	 * JOIN ACTIVE (some new cmd has changed the state of SM), so just
	 * complete the connect command.
	 */
	cm_connect_complete(cm_ctx, resp);

	return qdf_status;
}

QDF_STATUS cm_bss_select_ind_rsp(struct wlan_objmgr_vdev *vdev,
				 QDF_STATUS status)
{
	struct cnx_mgr *cm_ctx;
	QDF_STATUS qdf_status;
	wlan_cm_id cm_id;
	uint32_t prefix;
	struct wlan_cm_connect_resp *resp;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_INVAL;

	cm_id = cm_ctx->active_cm_id;
	prefix = CM_ID_GET_PREFIX(cm_id);

	if (prefix != CONNECT_REQ_PREFIX) {
		mlme_err(CM_PREFIX_FMT "active req is not connect req",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id));
		return QDF_STATUS_E_INVAL;
	}

	if (QDF_IS_STATUS_SUCCESS(status)) {
		qdf_status =
			cm_sm_deliver_event(vdev,
				WLAN_CM_SM_EV_BSS_SELECT_IND_SUCCESS,
				sizeof(wlan_cm_id), &cm_id);
		if (QDF_IS_STATUS_SUCCESS(qdf_status))
			return qdf_status;

		goto post_err;
	}

	/* In case of failure try with next candidate */
	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp) {
		qdf_status = QDF_STATUS_E_NOMEM;
		goto post_err;
	}

	cm_fill_failure_resp_from_cm_id(cm_ctx, resp, cm_id,
					CM_BSS_SELECT_IND_FAILED);
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

QDF_STATUS cm_bss_peer_create_rsp(struct wlan_objmgr_vdev *vdev,
				  QDF_STATUS status,
				  struct qdf_mac_addr *peer_mac)
{
	struct cnx_mgr *cm_ctx;
	QDF_STATUS qdf_status;
	wlan_cm_id cm_id;
	uint32_t prefix;
	struct wlan_cm_connect_resp *resp;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_INVAL;

	cm_id = cm_ctx->active_cm_id;
	prefix = CM_ID_GET_PREFIX(cm_id);

	if (prefix != CONNECT_REQ_PREFIX) {
		mlme_err(CM_PREFIX_FMT "active req is not connect req",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id));
		mlme_cm_bss_peer_delete_req(vdev);
		return QDF_STATUS_E_INVAL;
	}

	if (QDF_IS_STATUS_SUCCESS(status)) {
		qdf_status =
			cm_sm_deliver_event(vdev,
					  WLAN_CM_SM_EV_BSS_CREATE_PEER_SUCCESS,
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

	cm_fill_failure_resp_from_cm_id(cm_ctx, resp, cm_id,
					CM_PEER_CREATE_FAILED);
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

static void
cm_copy_crypto_prarams(struct wlan_cm_connect_crypto_info *dst_params,
		       struct wlan_crypto_params  *src_params)
{
	dst_params->akm_suites = src_params->key_mgmt;
	dst_params->auth_type = src_params->authmodeset;
	dst_params->ciphers_pairwise = src_params->ucastcipherset;
	dst_params->group_cipher = src_params->mcastcipherset;
	dst_params->mgmt_ciphers = src_params->mgmtcipherset;
	dst_params->rsn_caps = src_params->rsn_caps;
}

static void
cm_set_crypto_params_from_ie(struct wlan_cm_connect_req *req)
{
	struct wlan_crypto_params crypto_params;
	QDF_STATUS status;

	if (!req->assoc_ie.ptr)
		return;

	status = wlan_get_crypto_params_from_rsn_ie(&crypto_params,
						    req->assoc_ie.ptr,
						    req->assoc_ie.len);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		cm_copy_crypto_prarams(&req->crypto, &crypto_params);
		return;
	}

	status = wlan_get_crypto_params_from_wpa_ie(&crypto_params,
						    req->assoc_ie.ptr,
						    req->assoc_ie.len);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		cm_copy_crypto_prarams(&req->crypto, &crypto_params);
		return;
	}

	status = wlan_get_crypto_params_from_wapi_ie(&crypto_params,
						     req->assoc_ie.ptr,
						     req->assoc_ie.len);
	if (QDF_IS_STATUS_SUCCESS(status))
		cm_copy_crypto_prarams(&req->crypto, &crypto_params);
}

static QDF_STATUS
cm_allocate_and_copy_ies_and_keys(struct wlan_cm_connect_req *target,
				  struct wlan_cm_connect_req *source)
{
	/* Reset the copied pointers of target */
	source->assoc_ie.ptr = NULL;
	source->crypto.wep_keys.key = NULL;
	source->crypto.wep_keys.seq = NULL;
	source->scan_ie.ptr = NULL;

	if (source->scan_ie.ptr) {
		target->scan_ie.ptr = qdf_mem_malloc(source->scan_ie.len);
		if (!target->scan_ie.ptr)
			target->scan_ie.len = 0;
		else
			qdf_mem_copy(target->scan_ie.ptr,
				     source->scan_ie.ptr, source->scan_ie.len);
	}

	if (source->assoc_ie.ptr) {
		target->assoc_ie.ptr = qdf_mem_malloc(source->assoc_ie.len);
		if (!target->assoc_ie.ptr)
			return QDF_STATUS_E_NOMEM;

		qdf_mem_copy(target->assoc_ie.ptr, source->assoc_ie.ptr,
			     source->assoc_ie.len);
	}

	if (source->crypto.wep_keys.key) {
		target->crypto.wep_keys.key =
			qdf_mem_malloc(source->crypto.wep_keys.key_len);
		if (!target->crypto.wep_keys.key)
			return QDF_STATUS_E_NOMEM;

		qdf_mem_copy(target->crypto.wep_keys.key,
			     source->crypto.wep_keys.key,
			     source->crypto.wep_keys.key_len);
	}

	if (source->crypto.wep_keys.seq) {
		target->crypto.wep_keys.seq =
			qdf_mem_malloc(source->crypto.wep_keys.seq_len);
		if (!target->crypto.wep_keys.seq)
			return QDF_STATUS_E_NOMEM;

		qdf_mem_copy(target->crypto.wep_keys.seq,
			     source->crypto.wep_keys.seq,
			     source->crypto.wep_keys.seq_len);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_connect_start_req(struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_connect_req *req)
{
	struct cnx_mgr *cm_ctx;
	struct cm_req *cm_req;
	struct cm_connect_req *connect_req;
	QDF_STATUS status;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_INVAL;

	cm_vdev_scan_cancel(wlan_vdev_get_pdev(cm_ctx->vdev), cm_ctx->vdev);

	/*
	 * This would be freed as part of removal from cm req list if adding
	 * to list is success after posting WLAN_CM_SM_EV_CONNECT_REQ.
	 */
	cm_req = qdf_mem_malloc(sizeof(*cm_req));
	if (!cm_req)
		return QDF_STATUS_E_NOMEM;

	connect_req = &cm_req->connect_req;
	connect_req->req = *req;

	status = cm_allocate_and_copy_ies_and_keys(&connect_req->req, req);
	if (QDF_IS_STATUS_ERROR(status))
		goto err;

	cm_set_crypto_params_from_ie(&connect_req->req);

	status = cm_sm_deliver_event(vdev, WLAN_CM_SM_EV_CONNECT_REQ,
				     sizeof(*connect_req), connect_req);

err:
	/* free the req if connect is not handled */
	if (QDF_IS_STATUS_ERROR(status)) {
		cm_free_connect_req_mem(connect_req);
		qdf_mem_free(cm_req);
	}

	return status;
}
