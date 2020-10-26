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
 * DOC: Implements legacy connect specific APIs of connection manager to
 * initiate vdev manager operations
 */

#include "wlan_cm_vdev_api.h"
#include "wlan_scan_utils_api.h"
#include "wlan_mlme_dbg.h"

void cm_free_join_req(struct cm_vdev_join_req *join_req)
{
	if (!join_req)
		return;

	util_scan_free_cache_entry(join_req->entry);
	join_req->entry = NULL;
	qdf_mem_free(join_req->assoc_ie.ptr);
	qdf_mem_free(join_req->scan_ie.ptr);
	join_req->assoc_ie.ptr = NULL;
	join_req->scan_ie.ptr = NULL;
	qdf_mem_free(join_req);
}

static QDF_STATUS cm_flush_join_req(struct scheduler_msg *msg)
{
	struct cm_vdev_join_req *join_req;

	if (!msg || !msg->bodyptr) {
		mlme_err("msg or msg->bodyptr is NULL");
		return QDF_STATUS_E_INVAL;
	}

	join_req = msg->bodyptr;
	cm_free_join_req(join_req);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
cm_copy_join_params(struct cm_vdev_join_req *join_req,
		    struct wlan_cm_vdev_connect_req *req)
{
	join_req->assoc_ie.ptr = qdf_mem_malloc(req->assoc_ie.len);

	if (!join_req->assoc_ie.ptr)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_copy(join_req->assoc_ie.ptr, req->assoc_ie.ptr,
		     req->assoc_ie.len);

	join_req->scan_ie.ptr = qdf_mem_malloc(req->scan_ie.len);

	if (!join_req->scan_ie.ptr)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_copy(join_req->scan_ie.ptr, req->scan_ie.ptr,
		     req->scan_ie.len);

	join_req->entry = util_scan_copy_cache_entry(req->bss->entry);

	if (!join_req->entry)
		return QDF_STATUS_E_NOMEM;

	join_req->vdev_id = req->vdev_id;
	join_req->cm_id = req->cm_id;
	join_req->force_rsne_override = req->force_rsne_override;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cm_handle_connect_req(struct wlan_objmgr_vdev *vdev,
		      struct wlan_cm_vdev_connect_req *req)
{
	struct cm_vdev_join_req *join_req;
	struct scheduler_msg msg;
	QDF_STATUS status;

	if (!vdev || !req)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_zero(&msg, sizeof(msg));
	join_req = qdf_mem_malloc(sizeof(*join_req));

	if (!join_req)
		return QDF_STATUS_E_NOMEM;

	status = cm_copy_join_params(join_req, req);

	if (QDF_IS_STATUS_ERROR(status)) {
		cm_free_join_req(join_req);
		return QDF_STATUS_E_FAILURE;
	}

	msg.bodyptr = join_req;
	msg.callback = cm_process_join_req;
	msg.flush_callback = cm_flush_join_req;

	status = scheduler_post_message(QDF_MODULE_ID_MLME,
					QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (QDF_IS_STATUS_ERROR(status))
		cm_free_join_req(join_req);

	return status;
}

QDF_STATUS
cm_send_bss_peer_create_req(struct wlan_objmgr_vdev *vdev,
			    struct qdf_mac_addr *peer_mac)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cm_handle_connect_complete(struct wlan_objmgr_vdev *vdev,
			   struct wlan_cm_connect_rsp *rsp)
{
	return QDF_STATUS_SUCCESS;
}
