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

#ifdef WLAN_POLICY_MGR_ENABLE
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

	/* TODO: Fill band (STA+STA) */
	/* TODO: RSN OVERRIDE */

	filter->authmodeset = cm_req->req.crypto.auth_type;
	filter->ucastcipherset = cm_req->req.crypto.ciphers_pairwise;
	filter->key_mgmt = cm_req->req.crypto.akm_suites;
	filter->mcastcipherset = cm_req->req.crypto.group_cipher;
	filter->mgmtcipherset = cm_req->req.crypto.mgmt_ciphers;

	cm_set_pmf_caps(cm_req, filter);

	/* TODO: FOR WPS/OSEN set ignore auth */
	/* TODO: SET mobility domain */
	/* TODO: fill fils info */
}

static QDF_STATUS
cm_send_connect_start_fail(struct cnx_mgr *cm_ctx,
			   struct cm_connect_req *cm_req,
			   enum wlan_cm_connect_fail_reason reason)
{
	struct wlan_cm_connect_rsp *resp;

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return QDF_STATUS_E_NOMEM;

	return cm_sm_deliver_event(cm_ctx, WLAN_CM_SM_EV_CONNECT_FAILURE,
				   sizeof(*resp), resp);
	qdf_mem_free(resp);
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

	if (!candidate_list ||
	    (candidate_list && !qdf_list_size(candidate_list))) {
		if (candidate_list)
			wlan_scan_purge_results(candidate_list);
		mlme_info("No valid candidate found num_bss %d", num_bss);
		/*
		 * Do connect scan only of no candidates were found
		 * if candidates were found and were removed due to invalid
		 * return failure
		 */
		if (!num_bss)
			return cm_sm_deliver_event(cm_ctx, WLAN_CM_SM_EV_SCAN,
						   sizeof(*cm_req), cm_req);

		return QDF_STATUS_E_EMPTY;
	}
	cm_req->candidate_list = candidate_list;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_connect_start(struct cnx_mgr *cm_ctx,
			    struct cm_connect_req *cm_req)
{
	struct wlan_objmgr_pdev *pdev;
	enum wlan_cm_connect_fail_reason reason = CM_GENERIC_FAILURE;
	QDF_STATUS status;

	/* TODO: Interface event */
	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);
	if (!pdev)
		return QDF_STATUS_E_INVAL;
	status = cm_connect_get_candidates(pdev, cm_ctx, cm_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		reason = CM_NO_CANDIDATE_FOUND;
		goto connect_err;
	}

	/* TODO: Do HW mode change */
	/* TODO: serialize */
	return QDF_STATUS_SUCCESS;
connect_err:
	return cm_send_connect_start_fail(cm_ctx, cm_req, reason);
}

QDF_STATUS cm_connect_active(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	/*
	 * TODO: get first valid candidate, create bss peer.
	 * fill vdev crypto for the peer.
	 * call vdev sm to start connect for the candidate.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_try_next_candidate(struct cnx_mgr *cm_ctx,
				 struct wlan_cm_connect_rsp *resp)
{
	/*
	 * TODO: get next valid candidate, if no candidate left, post
	 * WLAN_CM_SM_EV_CONNECT_FAILURE to SM, inform osif about failure for
	 * the candidate if its not last one. and initiate the connect for
	 * next candidate.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_connect_cmd_timeout(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
{
	/*
	 * TODO: get the connect req from connect list and post
	 * WLAN_CM_SM_EV_CONNECT_FAILURE.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_connect_complete(struct cnx_mgr *cm_ctx,
			       struct wlan_cm_connect_rsp *resp)
{
	/*
	 * TODO: inform osif about success/failure, inform interface manager
	 * update fils/wep key and inform legacy, update bcn filter and scan
	 * entry mlme info, blm action and remove from serialization at the end.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_connect_start_req(struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_connect_req *req)
{
	struct cnx_mgr *cm_ctx = NULL;
	struct cm_connect_req *cm_req = NULL;

	/*
	 * TODO: Get WAPI/WPA/RSN IE and refill crypto params of req.
	 * Prepare cm_connect_req cm_req, get cm id and inform it to OSIF.
	 * store connect req to the cm ctx req_list
	 */

	return cm_sm_deliver_event(cm_ctx, WLAN_CM_SM_EV_CONNECT_REQ,
				   sizeof(*cm_req), cm_req);
}
