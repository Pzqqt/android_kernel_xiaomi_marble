/*
 * Copyright (c) 2012-2015, 2020-2021, The Linux Foundation. All rights reserved.
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
 * DOC: Implements init/deinit specific apis of connection manager
 */

#include "wlan_cm_main.h"
#include "wlan_cm_roam.h"
#include "wlan_cm_main_api.h"
#include "wlan_scan_api.h"

#ifdef WLAN_CM_USE_SPINLOCK
/**
 * cm_req_lock_create - Create CM req SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * Creates CM SM mutex/spinlock
 *
 * Return: void
 */
static inline void
cm_req_lock_create(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_create(&cm_ctx->cm_req_lock);
}

/**
 * cm_req_lock_destroy - Destroy CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * Destroy CM SM mutex/spinlock
 *
 * Return: void
 */
static inline void
cm_req_lock_destroy(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_destroy(&cm_ctx->cm_req_lock);
}
#else
static inline void
cm_req_lock_create(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_create(&cm_ctx->cm_req_lock);
}

static inline void
cm_req_lock_destroy(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_destroy(&cm_ctx->cm_req_lock);
}
#endif /* WLAN_CM_USE_SPINLOCK */

QDF_STATUS wlan_cm_init(struct vdev_mlme_obj *vdev_mlme)
{
	struct wlan_objmgr_vdev *vdev = vdev_mlme->vdev;
	enum QDF_OPMODE op_mode = wlan_vdev_mlme_get_opmode(vdev);
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(vdev);
	QDF_STATUS status;

	if (op_mode != QDF_STA_MODE && op_mode != QDF_P2P_CLIENT_MODE)
		return QDF_STATUS_SUCCESS;

	vdev_mlme->cnx_mgr_ctx = qdf_mem_malloc(sizeof(struct cnx_mgr));
	if (!vdev_mlme->cnx_mgr_ctx)
		return QDF_STATUS_E_NOMEM;

	vdev_mlme->cnx_mgr_ctx->vdev = vdev;
	status = mlme_cm_ext_hdl_create(vdev,
					&vdev_mlme->cnx_mgr_ctx->ext_cm_ptr);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_mem_free(vdev_mlme->cnx_mgr_ctx);
		vdev_mlme->cnx_mgr_ctx = NULL;
		return status;
	}

	status = cm_sm_create(vdev_mlme->cnx_mgr_ctx);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_cm_ext_hdl_destroy(vdev,
					vdev_mlme->cnx_mgr_ctx->ext_cm_ptr);
		vdev_mlme->cnx_mgr_ctx->ext_cm_ptr = NULL;
		qdf_mem_free(vdev_mlme->cnx_mgr_ctx);
		vdev_mlme->cnx_mgr_ctx = NULL;
		return status;
	}
	vdev_mlme->cnx_mgr_ctx->max_connect_attempts =
					CM_MAX_CONNECT_ATTEMPTS;
	vdev_mlme->cnx_mgr_ctx->connect_timeout =
					CM_MAX_PER_CANDIDATE_CONNECT_TIMEOUT;
	qdf_list_create(&vdev_mlme->cnx_mgr_ctx->req_list, CM_MAX_REQ);
	cm_req_lock_create(vdev_mlme->cnx_mgr_ctx);

	vdev_mlme->cnx_mgr_ctx->scan_requester_id =
		wlan_scan_register_requester(psoc,
					     "CM",
					     wlan_cm_scan_cb,
					     vdev_mlme->cnx_mgr_ctx);
	qdf_event_create(&vdev_mlme->cnx_mgr_ctx->disconnect_complete);
	cm_req_history_init(vdev_mlme->cnx_mgr_ctx);

	return QDF_STATUS_SUCCESS;
}

static void cm_deinit_req_list(struct cnx_mgr *cm_ctx)
{
	uint32_t prefix;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req = NULL;

	/*
	 * flush unhandled req from the list, this should not happen if SM is
	 * handled properly, but in cases of active command timeout
	 * (which needs be debugged and avoided anyway) if VDEV/PEER SM
	 * is not able to handle the req it may send out of sync command and
	 * thus resulting in a unhandled request. Thus to avoid memleak flush
	 * all unhandled req before destroying the list.
	 */
	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);

		cm_req = qdf_container_of(cur_node, struct cm_req, node);
		prefix = CM_ID_GET_PREFIX(cm_req->cm_id);
		qdf_list_remove_node(&cm_ctx->req_list, &cm_req->node);
		mlme_info(CM_PREFIX_FMT "flush prefix %x",
			  CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
					cm_req->cm_id), prefix);
		if (prefix == CONNECT_REQ_PREFIX) {
			cm_ctx->connect_count--;
			cm_free_connect_req_mem(&cm_req->connect_req);
		} else if (prefix == ROAM_REQ_PREFIX) {
			cm_free_roam_req_mem(&cm_req->roam_req);
		} else if (prefix == DISCONNECT_REQ_PREFIX) {
			cm_ctx->disconnect_count--;
		}
		qdf_mem_free(cm_req);

		cur_node = next_node;
		next_node = NULL;
		cm_req = NULL;
	}
	cm_req_lock_release(cm_ctx);

	cm_req_lock_destroy(cm_ctx);
	qdf_list_destroy(&cm_ctx->req_list);
}

QDF_STATUS wlan_cm_deinit(struct vdev_mlme_obj *vdev_mlme)
{
	struct wlan_objmgr_vdev *vdev = vdev_mlme->vdev;
	enum QDF_OPMODE op_mode = wlan_vdev_mlme_get_opmode(vdev);
	struct wlan_objmgr_psoc *psoc = wlan_vdev_get_psoc(vdev);
	wlan_scan_requester scan_requester_id;
	struct cnx_mgr *cm_ctx;

	if (op_mode != QDF_STA_MODE && op_mode != QDF_P2P_CLIENT_MODE)
		return QDF_STATUS_SUCCESS;

	cm_ctx = vdev_mlme->cnx_mgr_ctx;
	cm_req_history_deinit(cm_ctx);
	qdf_event_destroy(&cm_ctx->disconnect_complete);
	scan_requester_id = cm_ctx->scan_requester_id;
	wlan_scan_unregister_requester(psoc, scan_requester_id);

	cm_deinit_req_list(cm_ctx);
	cm_sm_destroy(cm_ctx);
	mlme_cm_ext_hdl_destroy(vdev, cm_ctx->ext_cm_ptr);
	cm_ctx->ext_cm_ptr = NULL;
	qdf_mem_free(cm_ctx);
	vdev_mlme->cnx_mgr_ctx = NULL;

	return QDF_STATUS_SUCCESS;
}
