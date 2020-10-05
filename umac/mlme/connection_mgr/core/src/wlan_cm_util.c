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
 * DOC: Implements general util apis of connection manager
 */

#include "wlan_cm_main_api.h"
#include "wlan_scan_api.h"
#include "wlan_cm_public_struct.h"
#include "wlan_serialization_api.h"

static uint32_t cm_get_prefix_for_cm_id(enum wlan_cm_source source) {
	switch (source) {
	case CM_OSIF_CONNECT:
	case CM_ROAMING:
		return CONNECT_REQ_PREFIX;
	default:
		return DISCONNECT_REQ_PREFIX;
	}
}

wlan_cm_id cm_get_cm_id(struct cnx_mgr *cm_ctx, enum wlan_cm_source source)
{
	wlan_cm_id cmd_id;
	uint32_t prefix;

	prefix = cm_get_prefix_for_cm_id(source);

	cmd_id = qdf_atomic_inc_return(&cm_ctx->global_cmd_id);
	cmd_id = (cmd_id & CM_ID_MASK);
	cmd_id = (cmd_id | prefix);

	return cmd_id;
}

struct cnx_mgr *cm_get_cm_ctx_fl(struct wlan_objmgr_vdev *vdev,
				 const char *func, uint32_t line)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct cnx_mgr *cm_ctx = NULL;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (vdev_mlme)
		cm_ctx = vdev_mlme->cnx_mgr_ctx;

	if (!cm_ctx)
		mlme_nofl_err("%s:%u: vdev %d cm_ctx is NULL", func, line,
			      wlan_vdev_get_id(vdev));

	return cm_ctx;
}

void cm_reset_active_cm_id(struct wlan_objmgr_vdev *vdev, wlan_cm_id cm_id)
{
	struct cnx_mgr *cm_ctx;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return;

	/* Reset active cm id if cm id match */
	if (cm_ctx->active_cm_id == cm_id)
		cm_ctx->active_cm_id = CM_ID_INVALID;
}


#ifdef WLAN_CM_USE_SPINLOCK
/**
 * cm_req_lock_acquire - acquire CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * acquire CM SM mutex/spinlock
 *
 * return: void
 */
static inline void cm_req_lock_acquire(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_acquire(&cm_ctx->cm_req_lock);
}

/**
 * cm_req_lock_release - release CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * release CM SM mutex/spinlock
 *
 * return: void
 */
static inline void cm_req_lock_release(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_release(&cm_ctx->cm_req_lock);
}
#else
static inline void cm_req_lock_acquire(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_acquire(&cm_ctx->cm_req_lock);
}

static inline void cm_req_lock_release(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_release(&cm_ctx->cm_req_lock);
}
#endif /* WLAN_CM_USE_SPINLOCK */

bool cm_check_cmid_match_list_head(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	qdf_list_node_t *cur_node = NULL;
	struct cm_req *cm_req;
	bool match = false;
	wlan_cm_id head_cm_id = 0;

	if (!cm_id)
		return false;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	if (!cur_node)
		goto exit;

	cm_req = qdf_container_of(cur_node, struct cm_req, node);
	head_cm_id = cm_req->cm_id;
	if (head_cm_id == *cm_id)
		match = true;

exit:
	cm_req_lock_release(cm_ctx);
	if (!match)
		mlme_info("head_cm_id 0x%x didn't match the given cm_id 0x%x",
			  head_cm_id, *cm_id);

	return match;
}

bool cm_check_scanid_match_list_head(struct cnx_mgr *cm_ctx,
				     wlan_scan_id *scan_id)
{
	qdf_list_node_t *cur_node = NULL;
	struct cm_req *cm_req;
	bool match = false;
	wlan_cm_id head_scan_id = 0;
	uint32_t prefix = 0;

	if (!scan_id)
		return false;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	if (!cur_node)
		goto exit;

	cm_req = qdf_container_of(cur_node, struct cm_req, node);
	prefix = CM_ID_GET_PREFIX(cm_req->cm_id);
	/* Check only if head is connect req */
	if (prefix != CONNECT_REQ_PREFIX)
		goto exit;
	head_scan_id = cm_req->connect_req.scan_id;
	if (head_scan_id == *scan_id)
		match = true;

exit:
	cm_req_lock_release(cm_ctx);
	if (!match)
		mlme_info("head_scan_id 0x%x didn't match the given scan_id 0x%x prefix 0x%x",
			  head_scan_id, *scan_id, prefix);

	return match;
}

struct cm_req *cm_get_req_by_cm_id_fl(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id,
				      const char *func, uint32_t line)
{
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req * cm_req;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);
		cm_req = qdf_container_of(cur_node, struct cm_req, node);

		if (cm_req->cm_id == cm_id) {
			cm_req_lock_release(cm_ctx);
			return cm_req;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	mlme_nofl_info("%s:%u: cm req not found for cm id 0x%x", func,
		       line, cm_id);

	return NULL;
}

QDF_STATUS
cm_fill_bss_info_in_connect_rsp_by_cm_id(struct cnx_mgr *cm_ctx,
					 wlan_cm_id cm_id,
					 struct wlan_cm_connect_rsp *resp)
{
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req;
	uint32_t prefix = CM_ID_GET_PREFIX(cm_id);
	struct scan_cache_node *candidate;
	struct wlan_cm_connect_req *req;

	if (prefix != CONNECT_REQ_PREFIX)
		return QDF_STATUS_E_INVAL;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);
		cm_req = qdf_container_of(cur_node, struct cm_req, node);

		if (cm_req->cm_id == cm_id) {
			req = &cm_req->connect_req.req;
			candidate = cm_req->connect_req.cur_candidate;
			if (candidate)
				qdf_copy_macaddr(&resp->bssid,
						 &candidate->entry->bssid);
			else if (!qdf_is_macaddr_zero(&req->bssid))
				qdf_copy_macaddr(&resp->bssid,
						 &req->bssid);
			else
				qdf_copy_macaddr(&resp->bssid,
						 &req->bssid_hint);
			if (candidate)
				resp->freq =
					candidate->entry->channel.chan_freq;
			else
				resp->freq = req->chan_freq;
			qdf_mem_copy(&resp->ssid, &req->ssid,
				     sizeof(resp->bssid));
			cm_req_lock_release(cm_ctx);
			return QDF_STATUS_SUCCESS;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS cm_add_req_to_list_and_indicate_osif(struct cnx_mgr *cm_ctx,
						struct cm_req *cm_req,
						enum wlan_cm_source source)
{
	uint32_t prefix = CM_ID_GET_PREFIX(cm_req->cm_id);

	cm_req_lock_acquire(cm_ctx);
	if (qdf_list_size(&cm_ctx->req_list) >= CM_MAX_REQ) {
		cm_req_lock_release(cm_ctx);
		mlme_err(CM_PREFIX_FMT "List full size %d",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
				       cm_req->cm_id),
			 qdf_list_size(&cm_ctx->req_list));
		return QDF_STATUS_E_FAILURE;
	}

	qdf_list_insert_front(&cm_ctx->req_list, &cm_req->node);
	if (prefix == CONNECT_REQ_PREFIX)
		cm_ctx->connect_count++;
	else
		cm_ctx->disconnect_count++;
	cm_req_lock_release(cm_ctx);

	mlme_cm_osif_update_id_and_src(cm_ctx->vdev, source, cm_req->cm_id);

	return QDF_STATUS_SUCCESS;
}

void cm_free_connect_req_mem(struct cm_connect_req *connect_req)
{
	if (connect_req->candidate_list)
		wlan_scan_purge_results(connect_req->candidate_list);

	if (connect_req->req.assoc_ie.ptr) {
		qdf_mem_zero(connect_req->req.assoc_ie.ptr,
			     connect_req->req.assoc_ie.len);
		qdf_mem_free(connect_req->req.assoc_ie.ptr);
		connect_req->req.assoc_ie.ptr = NULL;
	}

	if (connect_req->req.crypto.wep_keys.key) {
		qdf_mem_zero(connect_req->req.crypto.wep_keys.key,
			     connect_req->req.crypto.wep_keys.key_len);
		qdf_mem_free(connect_req->req.crypto.wep_keys.key);
		connect_req->req.crypto.wep_keys.key = NULL;
	}

	if (connect_req->req.crypto.wep_keys.seq) {
		qdf_mem_zero(connect_req->req.crypto.wep_keys.seq,
			     connect_req->req.crypto.wep_keys.seq_len);
		qdf_mem_free(connect_req->req.crypto.wep_keys.seq);
		connect_req->req.crypto.wep_keys.seq = NULL;
	}
}

QDF_STATUS
cm_delete_req_from_list(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
{
	uint32_t prefix = CM_ID_GET_PREFIX(cm_id);
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req = NULL;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);

		cm_req = qdf_container_of(cur_node, struct cm_req, node);
		if (cm_req->cm_id == cm_id)
			break;

		cur_node = next_node;
		next_node = NULL;
		cm_req = NULL;
	}

	if (!cm_req) {
		cm_req_lock_release(cm_ctx);
		mlme_err(CM_PREFIX_FMT " req not found",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id));
		return QDF_STATUS_E_FAILURE;
	}

	qdf_list_remove_node(&cm_ctx->req_list, &cm_req->node);
	if (prefix == CONNECT_REQ_PREFIX) {
		cm_ctx->connect_count--;
		cm_free_connect_req_mem(&cm_req->connect_req);
	} else {
		cm_ctx->disconnect_count--;
	}

	qdf_mem_free(cm_req);
	cm_req_lock_release(cm_ctx);

	return QDF_STATUS_SUCCESS;
}

void cm_remove_cmd(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_serialization_queued_cmd_info cmd_info;
	uint32_t prefix = CM_ID_GET_PREFIX(cm_id);
	QDF_STATUS status;

	psoc = wlan_vdev_get_psoc(cm_ctx->vdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "Failed to find psoc",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id));
		return;
	}

	status = cm_delete_req_from_list(cm_ctx, cm_id);
	if (QDF_IS_STATUS_ERROR(status))
		return;

	qdf_mem_zero(&cmd_info, sizeof(cmd_info));
	cmd_info.cmd_id = cm_id;
	cmd_info.req_type = WLAN_SER_CANCEL_NON_SCAN_CMD;

	if (prefix == CONNECT_REQ_PREFIX)
		cmd_info.cmd_type = WLAN_SER_CMD_VDEV_CONNECT;
	else
		cmd_info.cmd_type = WLAN_SER_CMD_VDEV_DISCONNECT;

	cmd_info.vdev = cm_ctx->vdev;

	if (cm_id == cm_ctx->active_cm_id) {
		cmd_info.queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE;
		wlan_serialization_remove_cmd(&cmd_info);
	} else {
		cmd_info.queue_type = WLAN_SERIALIZATION_PENDING_QUEUE;
		wlan_serialization_cancel_request(&cmd_info);
	}
}

void cm_vdev_scan_cancel(struct wlan_objmgr_pdev *pdev,
			 struct wlan_objmgr_vdev *vdev)
{
	struct scan_cancel_request *req;
	QDF_STATUS status;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return;

	req->vdev = vdev;
	req->cancel_req.scan_id = INVAL_SCAN_ID;
	req->cancel_req.vdev_id = wlan_vdev_get_id(vdev);
	req->cancel_req.pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	req->cancel_req.req_type = WLAN_SCAN_CANCEL_VDEV_ALL;

	status = wlan_scan_cancel(req);
	/* In success/failure case wlan_scan_cancel free the req memory */
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("vdev %d cancel scan request failed",
			 wlan_vdev_get_id(vdev));
}

void cm_set_max_connect_attempts(struct wlan_objmgr_vdev *vdev,
				 uint8_t max_connect_attempts)
{
	struct cnx_mgr *cm_ctx;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return;

	cm_ctx->max_connect_attempts =
		QDF_MIN(max_connect_attempts, CM_MAX_CONNECT_ATTEMPTS);
	mlme_debug("vdev %d max connect attempts set to %d, requested %d",
		   wlan_vdev_get_id(vdev),
		   cm_ctx->max_connect_attempts, max_connect_attempts);
}

QDF_STATUS
cm_fill_disconnect_resp_from_cm_id(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id,
				   struct wlan_cm_discon_rsp *resp)
{
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req;
	uint32_t prefix = CM_ID_GET_PREFIX(cm_id);

	if (prefix != DISCONNECT_REQ_PREFIX)
		return QDF_STATUS_E_INVAL;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);
		cm_req = qdf_container_of(cur_node, struct cm_req, node);

		if (cm_req->cm_id == cm_id) {
			resp->req.cm_id = cm_id;
			resp->req.req = cm_req->discon_req.req;
			cm_req_lock_release(cm_ctx);
			return QDF_STATUS_SUCCESS;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	return QDF_STATUS_E_FAILURE;
}

bool cm_is_vdev_connecting(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);

	if (state == WLAN_CM_S_CONNECTING)
		return true;

	return false;
}

bool cm_is_vdev_connected(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);

	if (state == WLAN_CM_S_CONNECTED)
		return true;

	return false;
}

bool cm_is_vdev_disconnecting(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);

	if (state == WLAN_CM_S_DISCONNECTING)
		return true;

	return false;
}

bool cm_is_vdev_disconnected(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);

	if (state == WLAN_CM_S_INIT)
		return true;

	return false;
}

bool cm_is_vdev_roaming(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);

	if (state == WLAN_CM_S_ROAMING)
		return true;

	return false;
}
