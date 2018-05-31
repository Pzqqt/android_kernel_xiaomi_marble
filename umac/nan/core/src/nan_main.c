/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: contains core nan function definitions
 */

#include "nan_main_i.h"
#include "nan_ucfg_api.h"
#include "wlan_nan_api.h"
#include "target_if_nan.h"
#include "scheduler_api.h"
#include "wlan_serialization_api.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"

struct nan_vdev_priv_obj *nan_get_vdev_priv_obj(
				struct wlan_objmgr_vdev *vdev)
{
	struct nan_vdev_priv_obj *obj;

	if (!vdev) {
		nan_err("vdev is null");
		return NULL;
	}
	obj = wlan_objmgr_vdev_get_comp_private_obj(vdev, WLAN_UMAC_COMP_NAN);

	return obj;
}

struct nan_psoc_priv_obj *nan_get_psoc_priv_obj(
				struct wlan_objmgr_psoc *psoc)
{
	struct nan_psoc_priv_obj *obj;

	if (!psoc) {
		nan_err("psoc is null");
		return NULL;
	}
	obj = wlan_objmgr_psoc_get_comp_private_obj(psoc, WLAN_UMAC_COMP_NAN);

	return obj;
}

void nan_release_cmd(void *in_req, uint32_t cmdtype)
{
	struct wlan_objmgr_vdev *vdev = NULL;

	if (!in_req)
		return;

	switch (cmdtype) {
	case WLAN_SER_CMD_NDP_INIT_REQ: {
		struct nan_datapath_initiator_req *req = in_req;
		vdev = req->vdev;
		break;
	}
	case WLAN_SER_CMD_NDP_RESP_REQ: {
		struct nan_datapath_responder_req *req = in_req;
		vdev = req->vdev;
		break;
	}
	case WLAN_SER_CMD_NDP_DATA_END_INIT_REQ: {
		struct nan_datapath_end_req *req = in_req;
		vdev = req->vdev;
		break;
	}
	default:
		nan_err("invalid req type: %d", cmdtype);
		break;
	}

	if (vdev)
		wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
	else
		nan_err("vdev is null");

	qdf_mem_free(in_req);
}

static void nan_req_incomplete(void *req, uint32_t cmdtype)
{
	/* send msg to userspace if needed that cmd got incomplete */
}

static void nan_req_activated(void *in_req, uint32_t cmdtype)
{
	uint32_t req_type;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_lmac_if_nan_tx_ops *tx_ops;

	switch (cmdtype) {
	case WLAN_SER_CMD_NDP_INIT_REQ: {
		struct nan_datapath_initiator_req *req = in_req;
		vdev = req->vdev;
		req_type = NDP_INITIATOR_REQ;
		break;
	}
	case WLAN_SER_CMD_NDP_RESP_REQ: {
		struct nan_datapath_responder_req *req = in_req;
		vdev = req->vdev;
		req_type = NDP_RESPONDER_REQ;
		break;
	}
	case WLAN_SER_CMD_NDP_DATA_END_INIT_REQ: {
		struct nan_datapath_end_req *req = in_req;
		vdev = req->vdev;
		req_type = NDP_END_REQ;
		break;
	}
	default:
		nan_alert("in correct cmdtype: %d", cmdtype);
		return;
	}

	if (!vdev) {
		nan_alert("vdev is null");
		return;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		nan_alert("psoc is null");
		return;
	}

	tx_ops = target_if_nan_get_tx_ops(psoc);
	if (!tx_ops) {
		nan_alert("tx_ops is null");
		return;
	}

	/* send ndp_intiator_req/responder_req/end_req to FW */
	tx_ops->nan_req_tx(in_req, req_type);
}

static QDF_STATUS nan_serialized_cb(void *cmd,
				enum wlan_serialization_cb_reason reason)
{
	void *req;
	struct wlan_serialization_command *ser_cmd = cmd;

	if (!ser_cmd || !ser_cmd->umac_cmd) {
		nan_alert("cmd or umac_cmd is null");
		return QDF_STATUS_E_NULL_VALUE;
	}
	req = ser_cmd->umac_cmd;

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		nan_req_activated(req, ser_cmd->cmd_type);
		break;
	case WLAN_SER_CB_CANCEL_CMD:
	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		nan_req_incomplete(req, ser_cmd->cmd_type);
		break;
	case WLAN_SER_CB_RELEASE_MEM_CMD:
		nan_release_cmd(req, ser_cmd->cmd_type);
		break;
	default:
		/* Do nothing but logging */
		nan_alert("invalid serialized cb reason: %d", reason);
		break;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS nan_scheduled_msg_handler(struct scheduler_msg *msg)
{
	enum wlan_serialization_status status = 0;
	struct wlan_serialization_command cmd = {0};

	if (!msg || !msg->bodyptr) {
		nan_alert("msg or bodyptr is null");
		return QDF_STATUS_E_NULL_VALUE;
	}
	switch (msg->type) {
	case NDP_INITIATOR_REQ: {
		struct nan_datapath_initiator_req *req = msg->bodyptr;
		cmd.cmd_type = WLAN_SER_CMD_NDP_INIT_REQ;
		cmd.vdev = req->vdev;
		break;
	}
	case NDP_RESPONDER_REQ: {
		struct nan_datapath_responder_req *req = msg->bodyptr;
		cmd.cmd_type = WLAN_SER_CMD_NDP_RESP_REQ;
		cmd.vdev = req->vdev;
		break;
	}
	case NDP_END_REQ: {
		struct nan_datapath_end_req *req = msg->bodyptr;
		cmd.cmd_type = WLAN_SER_CMD_NDP_DATA_END_INIT_REQ;
		cmd.vdev = req->vdev;
		break;
	}
	default:
		nan_err("wrong request type: %d", msg->type);
		return QDF_STATUS_E_INVAL;
	}

	/* TBD - support more than one req of same type or avoid */
	cmd.cmd_id = 0;
	cmd.cmd_cb = nan_serialized_cb;
	cmd.umac_cmd = msg->bodyptr;
	cmd.source = WLAN_UMAC_COMP_NAN;
	cmd.is_high_priority = false;
	cmd.cmd_timeout_duration = 30000 /* 30 sec for now. TBD */;
	nan_debug("cmd_type: %d", cmd.cmd_type);

	status = wlan_serialization_request(&cmd);
	/* following is TBD */
	if (status != WLAN_SER_CMD_ACTIVE && status != WLAN_SER_CMD_PENDING) {
		nan_err("unable to serialize command");
		wlan_objmgr_vdev_release_ref(cmd.vdev, WLAN_NAN_ID);
		return QDF_STATUS_E_INVAL;
	}
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS nan_handle_confirm(
				struct nan_datapath_confirm_event *confirm)
{
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	struct nan_psoc_priv_obj *psoc_nan_obj;

	vdev_id = wlan_vdev_get_id(confirm->vdev);
	psoc = wlan_vdev_get_psoc(confirm->vdev);
	if (!psoc) {
		nan_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc_nan_obj = nan_get_psoc_priv_obj(psoc);
	if (!psoc_nan_obj) {
		nan_err("psoc_nan_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (confirm->rsp_code != NAN_DATAPATH_RESPONSE_ACCEPT &&
		confirm->num_active_ndps_on_peer == 0) {
		/*
		 * This peer was created at ndp_indication but
		 * confirm failed, so it needs to be deleted
		 */
		nan_err("NDP confirm with reject and no active ndp sessions. deleting peer: "QDF_MAC_ADDR_STR" on vdev_id: %d",
			QDF_MAC_ADDR_ARRAY(confirm->peer_ndi_mac_addr.bytes),
			vdev_id);
		psoc_nan_obj->cb_obj.delete_peers_by_addr(vdev_id,
						confirm->peer_ndi_mac_addr);
	}
	psoc_nan_obj->cb_obj.os_if_event_handler(psoc, confirm->vdev,
						 NDP_CONFIRM, confirm);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS nan_handle_initiator_rsp(
				struct nan_datapath_initiator_rsp *rsp,
				struct wlan_objmgr_vdev **vdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct nan_psoc_priv_obj *psoc_nan_obj;

	*vdev = rsp->vdev;
	psoc = wlan_vdev_get_psoc(rsp->vdev);
	if (!psoc) {
		nan_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc_nan_obj = nan_get_psoc_priv_obj(psoc);
	if (!psoc_nan_obj) {
		nan_err("psoc_nan_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc_nan_obj->cb_obj.os_if_event_handler(psoc, rsp->vdev,
					NDP_INITIATOR_RSP, rsp);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS nan_handle_ndp_ind(
				struct nan_datapath_indication_event *ndp_ind)
{
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct nan_psoc_priv_obj *psoc_nan_obj;

	vdev_id = wlan_vdev_get_id(ndp_ind->vdev);
	psoc = wlan_vdev_get_psoc(ndp_ind->vdev);
	if (!psoc) {
		nan_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc_nan_obj = nan_get_psoc_priv_obj(psoc);
	if (!psoc_nan_obj) {
		nan_err("psoc_nan_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	nan_debug("role: %d, vdev: %d, csid: %d, peer_mac_addr "
		QDF_MAC_ADDR_STR,
		ndp_ind->role, vdev_id, ndp_ind->ncs_sk_type,
		QDF_MAC_ADDR_ARRAY(ndp_ind->peer_mac_addr.bytes));

	if ((ndp_ind->role == NAN_DATAPATH_ROLE_INITIATOR) ||
	   ((NAN_DATAPATH_ROLE_RESPONDER == ndp_ind->role) &&
	   (NAN_DATAPATH_ACCEPT_POLICY_ALL == ndp_ind->policy))) {
		status = psoc_nan_obj->cb_obj.add_ndi_peer(vdev_id,
						ndp_ind->peer_mac_addr);
		if (QDF_IS_STATUS_ERROR(status)) {
			nan_err("Couldn't add ndi peer, ndp_role: %d",
				ndp_ind->role);
			return status;
		}
	}
	if (NAN_DATAPATH_ROLE_RESPONDER == ndp_ind->role)
		psoc_nan_obj->cb_obj.os_if_event_handler(psoc, ndp_ind->vdev,
						NDP_INDICATION, ndp_ind);

	return status;
}

static QDF_STATUS nan_handle_responder_rsp(
				struct nan_datapath_responder_rsp *rsp,
				struct wlan_objmgr_vdev **vdev)
{
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct nan_psoc_priv_obj *psoc_nan_obj;

	*vdev = rsp->vdev;
	psoc = wlan_vdev_get_psoc(rsp->vdev);
	if (!psoc) {
		nan_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc_nan_obj = nan_get_psoc_priv_obj(psoc);
	if (!psoc_nan_obj) {
		nan_err("psoc_nan_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (QDF_IS_STATUS_SUCCESS(rsp->status) && rsp->create_peer == true) {
		status = psoc_nan_obj->cb_obj.add_ndi_peer(
						wlan_vdev_get_id(rsp->vdev),
						rsp->peer_mac_addr);
		if (QDF_IS_STATUS_ERROR(status)) {
			nan_err("Couldn't add ndi peer");
			rsp->status = QDF_STATUS_E_FAILURE;
		}
	}
	psoc_nan_obj->cb_obj.os_if_event_handler(psoc, rsp->vdev,
						 NDP_RESPONDER_RSP, rsp);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS nan_handle_ndp_end_rsp(
			struct nan_datapath_end_rsp_event *rsp,
			struct wlan_objmgr_vdev **vdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct nan_psoc_priv_obj *psoc_nan_obj;

	*vdev = rsp->vdev;
	psoc = wlan_vdev_get_psoc(rsp->vdev);
	if (!psoc) {
		nan_err("psoc is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc_nan_obj = nan_get_psoc_priv_obj(psoc);
	if (!psoc_nan_obj) {
		nan_err("psoc_nan_obj is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc_nan_obj->cb_obj.os_if_event_handler(psoc, rsp->vdev,
						NDP_END_RSP, rsp);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS nan_handle_end_ind(
				struct nan_datapath_end_indication_event *ind)
{
	struct wlan_objmgr_psoc *psoc;
	struct nan_psoc_priv_obj *psoc_nan_obj;

	psoc = wlan_vdev_get_psoc(ind->vdev);
	if (!psoc) {
		nan_err("psoc is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc_nan_obj = nan_get_psoc_priv_obj(psoc);
	if (!psoc_nan_obj) {
		nan_err("psoc_nan_obj is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc_nan_obj->cb_obj.ndp_delete_peers(ind->ndp_map, ind->num_ndp_ids);
	psoc_nan_obj->cb_obj.os_if_event_handler(psoc, ind->vdev,
						 NDP_END_IND, ind);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS nan_handle_schedule_update(
				struct nan_datapath_sch_update_event *ind)
{
	struct wlan_objmgr_psoc *psoc;
	struct nan_psoc_priv_obj *psoc_nan_obj;

	psoc = wlan_vdev_get_psoc(ind->vdev);
	if (!psoc) {
		nan_err("psoc is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc_nan_obj = nan_get_psoc_priv_obj(psoc);
	if (!psoc_nan_obj) {
		nan_err("psoc_nan_obj is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc_nan_obj->cb_obj.os_if_event_handler(psoc, ind->vdev,
						 NDP_SCHEDULE_UPDATE, ind);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS nan_event_handler(struct scheduler_msg *pe_msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_serialization_queued_cmd_info cmd;

	cmd.requestor = WLAN_UMAC_COMP_NAN;
	cmd.cmd_id = 0;
	cmd.req_type = WLAN_SER_CANCEL_NON_SCAN_CMD;
	cmd.queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE;

	if (!pe_msg->bodyptr) {
		nan_err("msg body is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	switch (pe_msg->type) {
	case NDP_CONFIRM: {
		nan_handle_confirm(pe_msg->bodyptr);
		break;
	}
	case NDP_INITIATOR_RSP: {
		nan_handle_initiator_rsp(pe_msg->bodyptr, &cmd.vdev);
		cmd.cmd_type = WLAN_SER_CMD_NDP_INIT_REQ;
		wlan_serialization_remove_cmd(&cmd);
		break;
	}
	case NDP_INDICATION: {
		nan_handle_ndp_ind(pe_msg->bodyptr);
		break;
	}
	case NDP_RESPONDER_RSP:
		nan_handle_responder_rsp(pe_msg->bodyptr, &cmd.vdev);
		cmd.cmd_type = WLAN_SER_CMD_NDP_RESP_REQ;
		wlan_serialization_remove_cmd(&cmd);
		break;
	case NDP_END_RSP:
		nan_handle_ndp_end_rsp(pe_msg->bodyptr, &cmd.vdev);
		cmd.cmd_type = WLAN_SER_CMD_NDP_DATA_END_INIT_REQ;
		wlan_serialization_remove_cmd(&cmd);
		break;
	case NDP_END_IND:
		nan_handle_end_ind(pe_msg->bodyptr);
		break;
	case NDP_SCHEDULE_UPDATE:
		nan_handle_schedule_update(pe_msg->bodyptr);
		break;
	default:
		nan_alert("Unhandled NDP event: %d", pe_msg->type);
		status = QDF_STATUS_E_NOSUPPORT;
		break;
	}
	return status;
}
