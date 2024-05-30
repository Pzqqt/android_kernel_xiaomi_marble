/*
 * Copyright (c) 2012-2015, 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "wni_api.h"
#include "connection_mgr/core/src/wlan_cm_roam.h"
#include <wlan_mlo_mgr_sta.h>

static void cm_abort_connect_request_timers(struct wlan_objmgr_vdev *vdev)
{
	struct scheduler_msg msg;
	QDF_STATUS status;

	qdf_mem_zero(&msg, sizeof(msg));
	msg.bodyval = wlan_vdev_get_id(vdev);
	msg.type = CM_ABORT_CONN_TIMER;
	status = scheduler_post_message(QDF_MODULE_ID_MLME,
					QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_debug("msg CM_ABORT_CONN_TIMER post fail");
}

QDF_STATUS cm_disconnect_start_ind(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_disconnect_req *req)
{
	struct wlan_objmgr_psoc *psoc;
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
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err("vdev_id: %d psoc not found", req->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	if (cm_csr_is_ss_wait_for_key(req->vdev_id)) {
		mlme_debug("Stop Wait for key timer");
		cm_stop_wait_for_key_timer(psoc, req->vdev_id);
		cm_csr_set_ss_none(req->vdev_id);
	}

	user_disconnect = req->source == CM_OSIF_DISCONNECT ? true : false;
	if (user_disconnect) {
		wlan_p2p_cleanup_roc_by_vdev(vdev);
		wlan_tdls_notify_sta_disconnect(req->vdev_id, false,
						user_disconnect, vdev);
	}
	cm_abort_connect_request_timers(vdev);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cm_handle_disconnect_req(struct wlan_objmgr_vdev *vdev,
			 struct wlan_cm_vdev_discon_req *req)
{
	struct wlan_cm_vdev_discon_req *discon_req;
	struct scheduler_msg msg;
	QDF_STATUS status;
	enum QDF_OPMODE opmode;
	struct wlan_objmgr_pdev *pdev;
	uint8_t vdev_id;
	struct rso_config *rso_cfg;
	struct wlan_objmgr_psoc *psoc;

	if (!vdev || !req)
		return QDF_STATUS_E_FAILURE;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "pdev not found",
			 CM_PREFIX_REF(req->req.vdev_id, req->cm_id));
		return QDF_STATUS_E_INVAL;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "psoc not found",
			 CM_PREFIX_REF(req->req.vdev_id, req->cm_id));
		return QDF_STATUS_E_INVAL;
	}
	rso_cfg = wlan_cm_get_rso_config(vdev);
	if (!rso_cfg)
		return QDF_STATUS_E_INVAL;
	vdev_id = wlan_vdev_get_id(vdev);

	qdf_mem_zero(&msg, sizeof(msg));

	discon_req = qdf_mem_malloc(sizeof(*discon_req));
	if (!discon_req)
		return QDF_STATUS_E_NOMEM;

	cm_csr_handle_diconnect_req(vdev, req);
	wlan_roam_reset_roam_params(psoc);
	cm_roam_restore_default_config(pdev, vdev_id);
	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode == QDF_STA_MODE)
		wlan_cm_roam_state_change(pdev, vdev_id,
					  WLAN_ROAM_DEINIT,
					  REASON_DISCONNECTED);
	if (rso_cfg->roam_scan_freq_lst.freq_list)
		qdf_mem_free(rso_cfg->roam_scan_freq_lst.freq_list);
	rso_cfg->roam_scan_freq_lst.freq_list = NULL;
	rso_cfg->roam_scan_freq_lst.num_chan = 0;

	qdf_mem_copy(discon_req, req, sizeof(*req));
	msg.bodyptr = discon_req;
	msg.type = CM_DISCONNECT_REQ;

	status = scheduler_post_message(QDF_MODULE_ID_MLME,
					QDF_MODULE_ID_PE,
					QDF_MODULE_ID_PE, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "msg post fail",
			 CM_PREFIX_REF(req->req.vdev_id, req->cm_id));
		qdf_mem_free(discon_req);
	}
	return status;
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
static void cm_disconnect_diag_event(struct wlan_objmgr_vdev *vdev,
				     struct wlan_cm_discon_rsp *rsp)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_mlme_psoc_ext_obj *mlme_obj;
	WLAN_HOST_DIAG_EVENT_DEF(connect_status,
				 host_event_wlan_status_payload_type);

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return;

	mlme_obj = mlme_get_psoc_ext_obj(psoc);
	if (!mlme_obj)
		return;
	qdf_mem_zero(&connect_status,
		     sizeof(host_event_wlan_status_payload_type));
	qdf_mem_copy(&connect_status,
		     &mlme_obj->cfg.sta.event_payload,
		     sizeof(host_event_wlan_status_payload_type));

	connect_status.rssi = mlme_obj->cfg.sta.current_rssi;
	connect_status.eventId = DIAG_WLAN_STATUS_DISCONNECT;
	if (rsp->req.req.reason_code == REASON_MIC_FAILURE) {
		connect_status.reason = DIAG_REASON_MIC_ERROR;
	} else if (rsp->req.req.source == CM_PEER_DISCONNECT) {
		switch (rsp->req.req.reason_code) {
		case REASON_PREV_AUTH_NOT_VALID:
		case REASON_CLASS2_FRAME_FROM_NON_AUTH_STA:
		case REASON_DEAUTH_NETWORK_LEAVING:
			connect_status.reason = DIAG_REASON_DEAUTH;
			break;
		default:
			connect_status.reason = DIAG_REASON_DISASSOC;
			break;
		}
		connect_status.reasonDisconnect = rsp->req.req.reason_code;
	} else if (rsp->req.req.source == CM_SB_DISCONNECT) {
		connect_status.reason = DIAG_REASON_DEAUTH;
		connect_status.reasonDisconnect = rsp->req.req.reason_code;
	} else {
		connect_status.reason = DIAG_REASON_USER_REQUESTED;
	}

	WLAN_HOST_DIAG_EVENT_REPORT(&connect_status,
				    EVENT_WLAN_STATUS_V2);
}
#else
static inline void cm_disconnect_diag_event(struct wlan_objmgr_vdev *vdev,
					    struct wlan_cm_discon_rsp *rsp)
{}
#endif

QDF_STATUS
cm_disconnect_complete_ind(struct wlan_objmgr_vdev *vdev,
			   struct wlan_cm_discon_rsp *rsp)
{
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	enum QDF_OPMODE op_mode;

	if (!vdev || !rsp) {
		mlme_err("vdev or rsp is NULL");
		return QDF_STATUS_E_INVAL;
	}
	cm_csr_diconnect_done_ind(vdev, rsp);

	vdev_id = wlan_vdev_get_id(vdev);
	op_mode = wlan_vdev_mlme_get_opmode(vdev);
	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "pdev not found",
			 CM_PREFIX_REF(vdev_id, rsp->req.cm_id));
		return QDF_STATUS_E_INVAL;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "psoc not found",
			 CM_PREFIX_REF(vdev_id, rsp->req.cm_id));
		return QDF_STATUS_E_INVAL;
	}
	cm_disconnect_diag_event(vdev, rsp);
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
	struct wlan_objmgr_vdev *vdev;

	if (!msg || !msg->bodyptr)
		return QDF_STATUS_E_FAILURE;

	ind = msg->bodyptr;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(ind->psoc,
						ind->disconnect_param.vdev_id,
						WLAN_MLME_CM_ID);

	if (!vdev) {
		mlme_err("vdev_id: %d: vdev not found",
			 ind->disconnect_param.vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = mlo_disconnect(vdev, ind->disconnect_param.source,
				ind->disconnect_param.reason_code,
				&ind->disconnect_param.bssid);
	qdf_mem_free(ind);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

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

#ifdef WLAN_FEATURE_HOST_ROAM
static inline
QDF_STATUS cm_fill_disconnect_resp(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_discon_rsp *resp)
{
	struct wlan_cm_vdev_reassoc_req req;

	/* Fill from reassoc req for Handsoff disconnect */
	if (cm_get_active_reassoc_req(vdev, &req)) {
		resp->req.cm_id = req.cm_id;
		resp->req.req.vdev_id = req.vdev_id;
		qdf_copy_macaddr(&resp->req.req.bssid, &req.prev_bssid);
		resp->req.req.source = CM_ROAM_DISCONNECT;
	} else if (!cm_get_active_disconnect_req(vdev, &resp->req)) {
		/* If not reassoc then fill from disconnect active */
		return QDF_STATUS_E_FAILURE;
	}
	mlme_debug(CM_PREFIX_FMT "disconnect found source %d",
		   CM_PREFIX_REF(resp->req.req.vdev_id, resp->req.cm_id),
		   resp->req.req.source);

	return QDF_STATUS_SUCCESS;
}
#else
static inline
QDF_STATUS cm_fill_disconnect_resp(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_discon_rsp *resp)
{
	if (!cm_get_active_disconnect_req(vdev, &resp->req))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS cm_handle_disconnect_resp(struct scheduler_msg *msg)
{
	QDF_STATUS status;
	struct cm_vdev_disconnect_rsp *ind;
	struct wlan_cm_discon_rsp resp;
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

	qdf_mem_zero(&resp, sizeof(resp));
	status = cm_fill_disconnect_resp(vdev, &resp);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Active disconnect not found for vdev %d",
			 ind->vdev_id);
		qdf_mem_free(ind);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		return QDF_STATUS_E_FAILURE;
	}

	if (resp.req.req.source == CM_PEER_DISCONNECT)
		cm_copy_peer_disconnect_ies(vdev, &resp.ap_discon_ie);

	status = wlan_cm_disconnect_rsp(vdev, &resp);
	mlme_free_peer_disconnect_ies(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);

	qdf_mem_free(ind);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_11BE_MLO
static QDF_STATUS
wlan_cm_mlo_update_disconnecting_vdev_id(struct wlan_objmgr_psoc *psoc,
					 uint8_t *vdev_id)
{
	struct wlan_objmgr_vdev *vdev = NULL;
	struct wlan_objmgr_vdev *vdev_list[WLAN_UMAC_MLO_MAX_VDEVS] = {NULL};
	uint16_t num_links = 0, i = 0;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, *vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		mlme_err("vdev_id: %d vdev not found", *vdev_id);
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!wlan_vdev_mlme_is_mlo_vdev(vdev)) {
		status = QDF_STATUS_SUCCESS;
		goto done;
	}

	mlo_get_ml_vdev_list(vdev, &num_links, vdev_list);
	if (!num_links) {
		mlme_err("No VDEVs under vdev id: %d", *vdev_id);
		status = QDF_STATUS_E_NULL_VALUE;
		goto done;
	}

	if (num_links > QDF_ARRAY_SIZE(vdev_list)) {
		mlme_err("Invalid number of VDEVs num_links:%u", num_links);
		status = QDF_STATUS_E_INVAL;
		goto release_mlo_ref;
	}

	for (i = 0; i < num_links; i++) {
		if (wlan_vdev_mlme_is_mlo_vdev(vdev_list[i]) &&
		    wlan_cm_is_vdev_disconnecting(vdev_list[i])) {
			/*
			 * This is expected to match only once as per current
			 * design.
			 */
			*vdev_id = wlan_vdev_get_id(vdev_list[i]);
			break;
		}
	}

release_mlo_ref:
	for (i = 0; i < QDF_ARRAY_SIZE(vdev_list); i++)
		mlo_release_vdev_ref(vdev_list[i]);

done:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);

	return status;
}
#else
static QDF_STATUS
wlan_cm_mlo_update_disconnecting_vdev_id(struct wlan_objmgr_psoc *psoc,
					 uint8_t *vdev_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif /* WLAN_FEATURE_11BE_MLO */

QDF_STATUS
wlan_cm_rso_stop_continue_disconnect(struct wlan_objmgr_psoc *psoc,
				     uint8_t rso_stop_vdev_id, bool is_ho_fail)
{
	struct wlan_objmgr_vdev *vdev = NULL;
	struct wlan_cm_vdev_discon_req *req;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t vdev_id = rso_stop_vdev_id;

	wlan_cm_mlo_update_disconnecting_vdev_id(psoc, &vdev_id);
	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		mlme_err("vdev_id: %d vdev not found", vdev_id);
		return QDF_STATUS_E_NULL_VALUE;
	}

	req = qdf_mem_malloc(sizeof(*req));
	if (!req) {
		status = QDF_STATUS_E_NOMEM;
		goto done;
	}

	if (!cm_get_active_disconnect_req(vdev, req)) {
		mlme_err("vdev: %d: Active disconnect not found", vdev_id);
		status = QDF_STATUS_E_EXISTS;
		goto done;
	}
	wlan_cm_disc_cont_after_rso_stop(vdev, is_ho_fail, req);

done:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);
	qdf_mem_free(req);

	return status;
}
