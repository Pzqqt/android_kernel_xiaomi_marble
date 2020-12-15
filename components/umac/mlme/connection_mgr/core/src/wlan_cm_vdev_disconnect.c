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
 * DOC: Implements leagcy disconnect connect specific APIs of
 * connection mgr to initiate vdev manager operations
 */

#include "wlan_cm_vdev_api.h"
#include "wlan_mlme_main.h"
#include "wlan_cm_api.h"

QDF_STATUS
cm_handle_disconnect_req(struct wlan_objmgr_vdev *vdev,
			 struct wlan_cm_vdev_discon_req *req)
{
	struct wlan_cm_vdev_discon_req *discon_req;
	struct scheduler_msg msg;
	QDF_STATUS status;

	if (!vdev || !req)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_zero(&msg, sizeof(msg));

	discon_req = qdf_mem_malloc(sizeof(*discon_req));

	if (!discon_req)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_copy(discon_req, req, sizeof(*req));
	msg.bodyptr = discon_req;
	msg.callback = cm_process_disconnect_req;

	status = scheduler_post_message(QDF_MODULE_ID_MLME,
					QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (QDF_IS_STATUS_ERROR(status))
		qdf_mem_free(discon_req);

	return status;
}

QDF_STATUS
cm_disconnect_complete_ind(struct wlan_objmgr_vdev *vdev,
			   struct wlan_cm_discon_rsp *rsp)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_send_vdev_down_req(struct wlan_objmgr_vdev *vdev)
{
	struct del_bss_resp *resp;

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return QDF_STATUS_E_NOMEM;

	resp->status = QDF_STATUS_SUCCESS;
	resp->vdev_id = wlan_vdev_get_id(vdev);
	return wlan_vdev_mlme_sm_deliver_evt(vdev,
					     WLAN_VDEV_SM_EV_MLME_DOWN_REQ,
					     sizeof(*resp), resp);
}

QDF_STATUS cm_disconnect_indication(struct scheduler_msg *msg)
{
	struct cm_vdev_discon_ind *ind;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status;

	if (!msg || !msg->bodyptr)
		return QDF_STATUS_E_FAILURE;

	ind = msg->bodyptr;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(
			ind->psoc, ind->disconnect_param.vdev_id,
			WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev_id: %d : vdev not found",
			 ind->disconnect_param.vdev_id);
		qdf_mem_free(ind);
		return QDF_STATUS_E_INVAL;
	}

	status = wlan_cm_disconnect(vdev, ind->disconnect_param.source,
				    ind->disconnect_param.reason_code,
				    &ind->disconnect_param.bssid);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
	qdf_mem_free(ind);

	return status;
}
