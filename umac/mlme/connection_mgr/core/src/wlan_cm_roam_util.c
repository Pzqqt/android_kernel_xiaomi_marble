/*
 * Copyright (c) 2011-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: Implements general Roam utils for connection manager
 */

#include "wlan_cm_main.h"
#include "wlan_cm_roam_sm.h"
#include "wlan_cm_sm.h"
#include "wlan_cm_main_api.h"
#include "wlan_cm_roam.h"
#include <wlan_scan_api.h>

void cm_free_roam_req_mem(struct cm_roam_req *roam_req)
{
	if (roam_req->candidate_list)
		wlan_scan_purge_results(roam_req->candidate_list);
}

#ifndef CONN_MGR_ADV_FEATURE
static void cm_fill_roam_vdev_crypto_params(struct cnx_mgr *cm_ctx,
					    struct wlan_cm_connect_req *req)
{
	cm_fill_vdev_crypto_params(cm_ctx, req);
}
#else
static void cm_fill_roam_vdev_crypto_params(struct cnx_mgr *cm_ctx,
					    struct wlan_cm_connect_req *req)
{
}
#endif /* CONN_MGR_ADV_FEATURE */

QDF_STATUS cm_check_and_prepare_roam_req(struct cnx_mgr *cm_ctx,
					 struct cm_connect_req *connect_req,
					 struct cm_req **roam_req)
{
	QDF_STATUS status;
	struct wlan_cm_connect_req *req;
	struct qdf_mac_addr bssid;
	struct wlan_ssid ssid;
	struct cm_req *cm_req, *req_ptr;
	qdf_freq_t freq = 0;

	cm_req = qdf_container_of(connect_req, struct cm_req, connect_req);
	req = &connect_req->req;

	if (req->chan_freq)
		freq = req->chan_freq;
	else if (req->chan_freq_hint)
		freq = req->chan_freq_hint;
	/*
	 * Reject re-assoc unless freq along with prev bssid and one
	 * of bssid or bssid hint is present.
	 */
	if (!freq || qdf_is_macaddr_zero(&req->prev_bssid) ||
	    (qdf_is_macaddr_zero(&req->bssid) &&
	     qdf_is_macaddr_zero(&req->bssid_hint)))
		return QDF_STATUS_E_FAILURE;

	wlan_vdev_get_bss_peer_mac(cm_ctx->vdev, &bssid);

	/* Reject re-assoc unless prev_bssid matches the current BSSID. */
	if (!qdf_is_macaddr_equal(&req->prev_bssid, &bssid)) {
		mlme_debug("BSSID didn't matched: bssid: "QDF_MAC_ADDR_FMT " prev bssid: " QDF_MAC_ADDR_FMT,
			   QDF_MAC_ADDR_REF(bssid.bytes),
			   QDF_MAC_ADDR_REF(req->prev_bssid.bytes));
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_vdev_mlme_get_ssid(cm_ctx->vdev, ssid.ssid, &ssid.length);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("failed to get ssid");
		return QDF_STATUS_E_FAILURE;
	}

	/* Reject re-assoc unless ssid matches. */
	if (ssid.length != req->ssid.length ||
	    qdf_mem_cmp(ssid.ssid, req->ssid.ssid, ssid.length)) {
		mlme_debug("SSID didn't matched: self ssid: \"%.*s\", ssid in req: \"%.*s\"",
			   ssid.length, ssid.ssid, req->ssid.length,
			   req->ssid.ssid);
		return QDF_STATUS_E_FAILURE;
	}

	/* fill roam_req for roaming and free cm_req */
	*roam_req = qdf_mem_malloc(sizeof(**roam_req));
	if (!*roam_req)
		return QDF_STATUS_E_NOMEM;

	req_ptr = *roam_req;
	if (!qdf_is_macaddr_zero(&req->bssid))
		qdf_copy_macaddr(&req_ptr->roam_req.req.bssid, &req->bssid);
	else
		qdf_copy_macaddr(&req_ptr->roam_req.req.bssid,
				 &req->bssid_hint);

	qdf_copy_macaddr(&req_ptr->roam_req.req.prev_bssid, &req->prev_bssid);
	cm_fill_roam_vdev_crypto_params(cm_ctx, &connect_req->req);
	req_ptr->roam_req.req.chan_freq = freq;
	req_ptr->roam_req.req.source = CM_ROAMING_HOST;

	/* Free the connect req, as reassoc is tried */
	cm_free_connect_req_mem(connect_req);
	qdf_mem_free(cm_req);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_add_roam_req_to_list(struct cnx_mgr *cm_ctx,
				   struct cm_req *cm_req)
{
	QDF_STATUS status;

	cm_req->roam_req.cm_id =
			cm_get_cm_id(cm_ctx, cm_req->roam_req.req.source);
	cm_req->cm_id = cm_req->roam_req.cm_id;
	cm_req->roam_req.req.vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	status =
	    cm_add_req_to_list_and_indicate_osif(cm_ctx, cm_req,
						 cm_req->roam_req.req.source);

	return status;
}

#ifdef WLAN_FEATURE_HOST_ROAM
bool cm_get_active_reassoc_req(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_vdev_reassoc_req *req)
{
	struct cnx_mgr *cm_ctx;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req = NULL;
	bool status = false;
	uint32_t cm_id_prefix;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return status;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);

		cm_req = qdf_container_of(cur_node, struct cm_req, node);
		cm_id_prefix = CM_ID_GET_PREFIX((cm_req->cm_id));

		if (cm_req->cm_id == cm_ctx->active_cm_id &&
		    cm_id_prefix == ROAM_REQ_PREFIX) {
			req->vdev_id = wlan_vdev_get_id(vdev);
			req->cm_id = cm_req->roam_req.cm_id;
			qdf_copy_macaddr(&req->prev_bssid,
					 &cm_req->roam_req.req.prev_bssid);
			req->bss = cm_req->roam_req.cur_candidate;
			status = true;
			cm_req_lock_release(cm_ctx);
			return status;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	return status;
}
#endif
QDF_STATUS
cm_fill_bss_info_in_roam_rsp_by_cm_id(struct cnx_mgr *cm_ctx,
				      wlan_cm_id cm_id,
				      struct wlan_cm_connect_resp *resp)
{
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req;
	uint32_t prefix = CM_ID_GET_PREFIX(cm_id);
	struct wlan_cm_roam_req *req;

	if (prefix != ROAM_REQ_PREFIX)
		return QDF_STATUS_E_INVAL;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);
		cm_req = qdf_container_of(cur_node, struct cm_req, node);

		if (cm_req->cm_id == cm_id) {
			req = &cm_req->roam_req.req;
			resp->freq = req->chan_freq;
			wlan_vdev_mlme_get_ssid(cm_ctx->vdev, resp->ssid.ssid,
						&resp->ssid.length);

			if (!qdf_is_macaddr_zero(&req->bssid))
				qdf_copy_macaddr(&resp->bssid, &req->bssid);

			cm_req_lock_release(cm_ctx);
			return QDF_STATUS_SUCCESS;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
struct cm_roam_req *cm_get_first_roam_command(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req = NULL;
	uint32_t cm_id_prefix;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return NULL;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);

		cm_req = qdf_container_of(cur_node, struct cm_req, node);
		cm_id_prefix = CM_ID_GET_PREFIX((cm_req->cm_id));

		if (cm_id_prefix == ROAM_REQ_PREFIX) {
			cm_req_lock_release(cm_ctx);
			return &cm_req->roam_req;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	return NULL;
}
#endif
