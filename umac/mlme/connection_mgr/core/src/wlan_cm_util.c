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

#define CONNECT_REQ_PREFIX          0x00C00000
#define DISCONNECT_REQ_PREFIX       0x00D00000
#define CM_ID_MASK                  0x0000FFFF

#define CM_ID_GET_PREFIX(cm_id)     cm_id & 0xFFFF0000

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

struct cnx_mgr *cm_get_cm_ctx(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme)
		return NULL;

	return vdev_mlme->cnx_mgr_ctx;
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
		mlme_info("head_cm_id %d didn't match the given cm_id %d",
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
		mlme_info("head_scan_id %d didn't match the given scan_id %d prefix %x",
			  head_scan_id, *scan_id, prefix);

	return match;
}

struct cm_req *cm_get_req_by_cm_id(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
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

	return NULL;
}

QDF_STATUS cm_add_req_to_list_and_indicate_osif(struct cnx_mgr *cm_ctx,
						struct cm_req *cm_req,
						enum wlan_cm_source source)
{
	uint32_t prefix = CM_ID_GET_PREFIX(cm_req->cm_id);

	cm_req_lock_acquire(cm_ctx);
	if (qdf_list_size(&cm_ctx->req_list) >= CM_MAX_REQ) {
		cm_req_lock_release(cm_ctx);
		mlme_err("List full size %d", qdf_list_size(&cm_ctx->req_list));
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
		mlme_err("cm req id %d not found", cm_id);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_list_remove_node(&cm_ctx->req_list, &cm_req->node);
	if (prefix == CONNECT_REQ_PREFIX) {
		cm_ctx->connect_count--;
		wlan_scan_purge_results(cm_req->connect_req.candidate_list);
	} else {
		cm_ctx->disconnect_count--;
	}

	qdf_mem_free(cm_req);
	cm_req_lock_release(cm_ctx);

	return QDF_STATUS_SUCCESS;
}
