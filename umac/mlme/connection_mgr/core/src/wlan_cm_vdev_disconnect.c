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
 * DOC: Implements leagcy disconnect connect specific APIs of
 * connection mgr to initiate vdev manager operations
 */

#include "wlan_cm_vdev_api.h"
#include "wlan_mlme_main.h"
#include "wlan_cm_api.h"
#include "wlan_p2p_api.h"
#include "wlan_tdls_api.h"
#include <wlan_policy_mgr_api.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_cm_roam_api.h>

QDF_STATUS cm_disconnect_start_ind(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_disconnect_req *req)
{
	struct wlan_objmgr_pdev *pdev;
	bool user_disconnect;

	if (!vdev || !req) {
		mlme_err("vdev or req is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("vdev_id: %d pdev not found", req->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	user_disconnect = req->source == CM_OSIF_CONNECT ? true : false;
	wlan_p2p_cleanup_roc_by_vdev(vdev);
	wlan_tdls_notify_sta_disconnect(req->vdev_id, false, user_disconnect,
					vdev);
	wlan_cm_abort_rso(pdev, req->vdev_id);
	/* stop wait for key timer */

	return QDF_STATUS_SUCCESS;
}

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

	cm_csr_handle_diconnect_req(vdev, req);

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
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	enum QDF_OPMODE op_mode;

	if (!vdev || !rsp) {
		mlme_err("vdev or rsp is NULL");
		return QDF_STATUS_E_INVAL;
	}
	cm_csr_diconnect_done_ind(vdev, rsp);

	vdev_id = wlan_vdev_get_id(vdev);
	op_mode = wlan_vdev_mlme_get_opmode(vdev);
	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_err("vdev_id: %d psoc not found", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	wlan_tdls_notify_sta_disconnect(vdev_id, false, false, vdev);
	policy_mgr_decr_session_set_pcl(psoc, op_mode, vdev_id);

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

QDF_STATUS cm_disconnect(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			 enum wlan_cm_source source,
			 enum wlan_reason_code reason_code,
			 struct qdf_mac_addr *bssid)
{
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev_id: %d: vdev not found", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = wlan_cm_disconnect(vdev, source, reason_code, bssid);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	return status;
}

QDF_STATUS cm_send_sb_disconnect_req(struct scheduler_msg *msg)
{
	struct cm_vdev_discon_ind *ind;
	QDF_STATUS status;

	if (!msg || !msg->bodyptr)
		return QDF_STATUS_E_FAILURE;

	ind = msg->bodyptr;

	status = cm_disconnect(ind->psoc, ind->disconnect_param.vdev_id,
			       ind->disconnect_param.source,
			       ind->disconnect_param.reason_code,
			       &ind->disconnect_param.bssid);
	qdf_mem_free(ind);

	return status;
}

static void cm_copy_peer_disconnect_ies(struct wlan_objmgr_vdev *vdev,
					struct element_info *ap_ie)
{
	struct element_info *discon_ie;

	discon_ie = mlme_get_peer_disconnect_ies(vdev);
	if (!discon_ie)
		return;

	ap_ie->len = discon_ie->len;
	ap_ie->ptr = discon_ie->ptr;
}

QDF_STATUS cm_handle_disconnect_resp(struct scheduler_msg *msg)
{
	QDF_STATUS status;
	struct cm_vdev_disconnect_rsp *ind;
	struct wlan_cm_discon_rsp resp = {0};
	struct wlan_objmgr_vdev *vdev;

	if (!msg || !msg->bodyptr)
		return QDF_STATUS_E_FAILURE;

	ind = msg->bodyptr;
	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(ind->psoc, ind->vdev_id,
						    WLAN_MLME_CM_ID);
	if (!vdev) {
		mlme_err("vdev_id: %d : vdev not found", ind->vdev_id);
		qdf_mem_free(ind);
		return QDF_STATUS_E_INVAL;
	}

	if (!cm_get_active_disconnect_req(vdev, &resp.req)) {
		qdf_mem_free(ind);
		return QDF_STATUS_E_FAILURE;
	}

	if (resp.req.req.source == CM_PEER_DISCONNECT)
		cm_copy_peer_disconnect_ies(vdev, &resp.ap_discon_ie);

	status = wlan_cm_disconnect_rsp(vdev, &resp);
	mlme_free_peer_disconnect_ies(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	qdf_mem_free(ind);

	return QDF_STATUS_E_FAILURE;
}
