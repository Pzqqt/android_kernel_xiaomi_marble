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
 * DOC: Implements init/deinit specific apis of connection manager
 */

#include "wlan_cm_main.h"
#include "wlan_cm_sm.h"

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
	QDF_STATUS status;

	if (op_mode != QDF_STA_MODE && op_mode != QDF_P2P_CLIENT_MODE)
		return QDF_STATUS_SUCCESS;

	vdev_mlme->cnx_mgr_ctx = qdf_mem_malloc(sizeof(struct cnx_mgr));
	if (!vdev_mlme->cnx_mgr_ctx)
		return QDF_STATUS_E_NOMEM;

	vdev_mlme->cnx_mgr_ctx->vdev = vdev_mlme->vdev;
	status = cm_sm_create(vdev_mlme->cnx_mgr_ctx);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_mem_free(vdev_mlme->cnx_mgr_ctx);
		vdev_mlme->cnx_mgr_ctx = NULL;
		return QDF_STATUS_E_NOMEM;
	}
	vdev_mlme->cnx_mgr_ctx->max_connect_attempts =
					CM_MAX_CONNECT_ATTEMPTS;
	qdf_list_create(&vdev_mlme->cnx_mgr_ctx->req_list, CM_MAX_REQ);
	cm_req_lock_create(vdev_mlme->cnx_mgr_ctx);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cm_deinit(struct vdev_mlme_obj *vdev_mlme)
{
	struct wlan_objmgr_vdev *vdev = vdev_mlme->vdev;
	enum QDF_OPMODE op_mode = wlan_vdev_mlme_get_opmode(vdev);

	if (op_mode != QDF_STA_MODE && op_mode != QDF_P2P_CLIENT_MODE)
		return QDF_STATUS_SUCCESS;

	cm_req_lock_destroy(vdev_mlme->cnx_mgr_ctx);
	qdf_list_destroy(&vdev_mlme->cnx_mgr_ctx->req_list);
	cm_sm_destroy(vdev_mlme->cnx_mgr_ctx);
	qdf_mem_free(vdev_mlme->cnx_mgr_ctx);
	vdev_mlme->cnx_mgr_ctx = NULL;

	return QDF_STATUS_SUCCESS;
}
