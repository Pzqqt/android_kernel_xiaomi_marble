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
 * DOC: contains nan target if functions
 */

#include "../../../umac/nan/core/src/nan_main_i.h"
#include "nan_public_structs.h"
#include "nan_ucfg_api.h"
#include "target_if_nan.h"
#include "wmi_unified_api.h"
#include "scheduler_api.h"

static QDF_STATUS target_if_nan_event_flush_cb(struct scheduler_msg *msg)
{
	void *ptr = msg->bodyptr;
	struct wlan_objmgr_vdev *vdev = NULL;

	switch (msg->type) {
	case NDP_INITIATOR_RSP:
		vdev = ((struct nan_datapath_initiator_rsp *)ptr)->vdev;
		break;
	case NDP_INDICATION:
		vdev = ((struct nan_datapath_indication_event *)ptr)->vdev;
		break;
	case NDP_CONFIRM:
		vdev = ((struct nan_datapath_confirm_event *)ptr)->vdev;
		break;
	case NDP_RESPONDER_RSP:
		vdev = ((struct nan_datapath_responder_rsp *)ptr)->vdev;
		break;
	case NDP_END_RSP:
		vdev = ((struct nan_datapath_end_rsp_event *)ptr)->vdev;
		break;
	case NDP_END_IND:
		vdev = ((struct nan_datapath_end_indication_event *)ptr)->vdev;
		break;
	case NDP_SCHEDULE_UPDATE:
		vdev = ((struct nan_datapath_sch_update_event *)ptr)->vdev;
		break;
	default:
		break;
	}

	if (vdev)
		wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
	qdf_mem_free(msg->bodyptr);
	msg->bodyptr = NULL;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_nan_event_dispatcher(struct scheduler_msg *msg)
{
	QDF_STATUS status;
	void *ptr = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev = NULL;
	struct wlan_lmac_if_nan_rx_ops *nan_rx_ops;

	switch (msg->type) {
	case NDP_INITIATOR_RSP:
		vdev = ((struct nan_datapath_initiator_rsp *)ptr)->vdev;
		break;
	case NDP_INDICATION:
		vdev = ((struct nan_datapath_indication_event *)ptr)->vdev;
		break;
	case NDP_CONFIRM:
		vdev = ((struct nan_datapath_confirm_event *)ptr)->vdev;
		break;
	case NDP_RESPONDER_RSP:
		vdev = ((struct nan_datapath_responder_rsp *)ptr)->vdev;
		break;
	case NDP_END_RSP:
		vdev = ((struct nan_datapath_end_rsp_event *)ptr)->vdev;
		break;
	case NDP_END_IND:
		vdev = ((struct nan_datapath_end_indication_event *)ptr)->vdev;
		break;
	case NDP_SCHEDULE_UPDATE:
		vdev = ((struct nan_datapath_sch_update_event *)ptr)->vdev;
		break;
	default:
		target_if_err("invalid msg type %d", msg->type);
		status = QDF_STATUS_E_INVAL;
		goto free_res;
	}

	if (!vdev) {
		target_if_err("vdev is null");
		status = QDF_STATUS_E_NULL_VALUE;
		goto free_res;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		target_if_err("psoc is null");
		status = QDF_STATUS_E_NULL_VALUE;
		goto free_res;
	}

	nan_rx_ops = target_if_nan_get_rx_ops(psoc);
	if (!nan_rx_ops) {
		target_if_err("nan_rx_ops is null");
		status = QDF_STATUS_E_NULL_VALUE;
		goto free_res;
	}

	status = nan_rx_ops->nan_event_rx(msg);
free_res:
	if (vdev)
		wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
	qdf_mem_free(msg->bodyptr);
	msg->bodyptr = NULL;
	return status;
}

static QDF_STATUS target_if_nan_ndp_initiator_req(
			struct nan_datapath_initiator_req *ndp_req)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct scheduler_msg pe_msg = {0};
	struct wlan_lmac_if_nan_rx_ops *nan_rx_ops;
	struct nan_datapath_initiator_rsp ndp_rsp = {0};

	if (!ndp_req) {
		target_if_err("ndp_req is null.");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(ndp_req->vdev);
	if (!psoc) {
		target_if_err("psoc is null.");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null.");
		return QDF_STATUS_E_INVAL;
	}

	nan_rx_ops = target_if_nan_get_rx_ops(psoc);
	if (!nan_rx_ops) {
		target_if_err("nan_rx_ops is null.");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_ndp_initiator_req_cmd_send(wmi_handle, ndp_req);
	if (QDF_IS_STATUS_SUCCESS(status))
		return status;

	ndp_rsp.vdev = ndp_req->vdev;
	ndp_rsp.transaction_id = ndp_req->transaction_id;
	ndp_rsp.ndp_instance_id = ndp_req->service_instance_id;
	ndp_rsp.status = NAN_DATAPATH_DATA_INITIATOR_REQ_FAILED;
	pe_msg.type = NDP_INITIATOR_RSP;
	pe_msg.bodyptr = &ndp_rsp;
	if (nan_rx_ops->nan_event_rx)
		nan_rx_ops->nan_event_rx(&pe_msg);

	return status;
}

static int target_if_ndp_initiator_rsp_handler(ol_scn_t scn, uint8_t *data,
						uint32_t len)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct scheduler_msg msg = {0};
	struct nan_datapath_initiator_rsp *rsp;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	rsp = qdf_mem_malloc(sizeof(*rsp));
	if (!rsp) {
		target_if_err("malloc failed");
		return -ENOMEM;
	}

	status = wmi_extract_ndp_initiator_rsp(wmi_handle, data, rsp);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("parsing of event failed, %d", status);
		qdf_mem_free(rsp);
		return -EINVAL;
	}

	msg.bodyptr = rsp;
	msg.type = NDP_INITIATOR_RSP;
	msg.callback = target_if_nan_event_dispatcher;
	msg.flush_callback = target_if_nan_event_flush_cb;
	target_if_debug("NDP_INITIATOR_RSP sent: %d", msg.type);
	status = scheduler_post_message(QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		target_if_nan_event_flush_cb(&msg);
		return -EINVAL;
	}

	return 0;
}

static int target_if_ndp_ind_handler(ol_scn_t scn, uint8_t *data,
				     uint32_t data_len)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct scheduler_msg msg = {0};
	struct nan_datapath_indication_event *rsp;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	rsp = qdf_mem_malloc(sizeof(*rsp));
	if (!rsp) {
		target_if_err("malloc failed");
		return -ENOMEM;
	}

	status = wmi_extract_ndp_ind(wmi_handle, data, rsp);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("parsing of event failed, %d", status);
		qdf_mem_free(rsp);
		return -EINVAL;
	}

	msg.bodyptr = rsp;
	msg.type = NDP_INDICATION;
	msg.callback = target_if_nan_event_dispatcher;
	msg.flush_callback = target_if_nan_event_flush_cb;
	target_if_debug("NDP_INDICATION sent: %d", msg.type);
	status = scheduler_post_message(QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		target_if_nan_event_flush_cb(&msg);
		return -EINVAL;
	}

	return 0;
}

static int target_if_ndp_confirm_handler(ol_scn_t scn, uint8_t *data,
					uint32_t data_len)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct scheduler_msg msg = {0};
	struct nan_datapath_confirm_event *rsp;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	rsp = qdf_mem_malloc(sizeof(*rsp));
	if (!rsp) {
		target_if_err("malloc failed");
		return -ENOMEM;
	}

	status = wmi_extract_ndp_confirm(wmi_handle, data, rsp);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("parsing of event failed, %d", status);
		qdf_mem_free(rsp);
		return -EINVAL;
	}

	msg.bodyptr = rsp;
	msg.type = NDP_CONFIRM;
	msg.callback = target_if_nan_event_dispatcher;
	msg.flush_callback = target_if_nan_event_flush_cb;
	target_if_debug("NDP_CONFIRM sent: %d", msg.type);
	status = scheduler_post_message(QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		target_if_nan_event_flush_cb(&msg);
		return -EINVAL;
	}

	return 0;
}

static QDF_STATUS target_if_nan_ndp_responder_req(
				struct nan_datapath_responder_req *req)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct scheduler_msg pe_msg = {0};
	struct wlan_lmac_if_nan_rx_ops *nan_rx_ops;
	struct nan_datapath_responder_rsp rsp = {0};

	if (!req) {
		target_if_err("Invalid req.");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(req->vdev);
	if (!psoc) {
		target_if_err("psoc is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	nan_rx_ops = target_if_nan_get_rx_ops(psoc);
	if (!nan_rx_ops) {
		target_if_err("nan_rx_ops is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = wmi_unified_ndp_responder_req_cmd_send(wmi_handle, req);
	if (QDF_IS_STATUS_SUCCESS(status))
		return status;

	rsp.vdev = req->vdev;
	rsp.transaction_id = req->transaction_id;
	rsp.status = NAN_DATAPATH_RSP_STATUS_ERROR;
	rsp.reason = NAN_DATAPATH_DATA_RESPONDER_REQ_FAILED;
	pe_msg.bodyptr = &rsp;
	pe_msg.type = NDP_RESPONDER_RSP;
	if (nan_rx_ops->nan_event_rx)
		nan_rx_ops->nan_event_rx(&pe_msg);

	return status;
}

static int target_if_ndp_responder_rsp_handler(ol_scn_t scn, uint8_t *data,
						uint32_t len)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct scheduler_msg msg = {0};
	struct nan_datapath_responder_rsp *rsp;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null.");
		return -EINVAL;
	}

	rsp = qdf_mem_malloc(sizeof(*rsp));
	if (!rsp) {
		target_if_err("malloc failed");
		return -ENOMEM;
	}

	status = wmi_extract_ndp_responder_rsp(wmi_handle, data, rsp);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("parsing of event failed, %d", status);
		qdf_mem_free(rsp);
		return -EINVAL;
	}

	msg.bodyptr = rsp;
	msg.type = NDP_RESPONDER_RSP;
	msg.callback = target_if_nan_event_dispatcher;
	msg.flush_callback = target_if_nan_event_flush_cb;
	target_if_debug("NDP_INITIATOR_RSP sent: %d", msg.type);
	status = scheduler_post_message(QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		target_if_nan_event_flush_cb(&msg);
		return -EINVAL;
	}

	return 0;
}

static QDF_STATUS target_if_nan_ndp_end_req(struct nan_datapath_end_req *req)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct scheduler_msg msg = {0};
	struct wlan_lmac_if_nan_rx_ops *nan_rx_ops;
	struct nan_datapath_end_rsp_event end_rsp = {0};

	if (!req) {
		target_if_err("req is null");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(req->vdev);
	if (!psoc) {
		target_if_err("psoc is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	nan_rx_ops = target_if_nan_get_rx_ops(psoc);
	if (!nan_rx_ops) {
		target_if_err("nan_rx_ops is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = wmi_unified_ndp_end_req_cmd_send(wmi_handle, req);
	if (QDF_IS_STATUS_SUCCESS(status))
		return status;

	end_rsp.vdev = req->vdev;
	msg.type = NDP_END_RSP;
	end_rsp.status = NAN_DATAPATH_RSP_STATUS_ERROR;
	end_rsp.reason = NAN_DATAPATH_END_FAILED;
	end_rsp.transaction_id = req->transaction_id;
	msg.bodyptr = &end_rsp;

	if (nan_rx_ops->nan_event_rx)
		nan_rx_ops->nan_event_rx(&msg);

	return status;
}

static int target_if_ndp_end_rsp_handler(ol_scn_t scn, uint8_t *data,
					 uint32_t data_len)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct scheduler_msg msg = {0};
	struct nan_datapath_end_rsp_event *end_rsp;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null.");
		return -EINVAL;
	}

	end_rsp = qdf_mem_malloc(sizeof(*end_rsp));
	if (!end_rsp) {
		target_if_err("malloc failed");
		return -ENOMEM;
	}

	status = wmi_extract_ndp_end_rsp(wmi_handle, data, end_rsp);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("parsing of event failed, %d", status);
		qdf_mem_free(end_rsp);
		return -EINVAL;
	}

	msg.bodyptr = end_rsp;
	msg.type = NDP_END_RSP;
	msg.callback = target_if_nan_event_dispatcher;
	msg.flush_callback = target_if_nan_event_flush_cb;
	target_if_debug("NDP_END_RSP sent: %d", msg.type);
	status = scheduler_post_message(QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		target_if_nan_event_flush_cb(&msg);
		return -EINVAL;
	}

	return 0;
}

static int target_if_ndp_end_ind_handler(ol_scn_t scn, uint8_t *data,
					 uint32_t data_len)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct scheduler_msg msg = {0};
	struct nan_datapath_end_indication_event *rsp = NULL;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null.");
		return -EINVAL;
	}

	status = wmi_extract_ndp_end_ind(wmi_handle, data, &rsp);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("parsing of event failed, %d", status);
		return -EINVAL;
	}

	rsp->vdev = wlan_objmgr_get_vdev_by_opmode_from_psoc(
			  wmi_handle->soc->wmi_psoc, QDF_NDI_MODE, WLAN_NAN_ID);
	if (!rsp->vdev) {
		target_if_err("vdev is null");
		qdf_mem_free(rsp);
		return -EINVAL;
	}

	msg.bodyptr = rsp;
	msg.type = NDP_END_IND;
	msg.callback = target_if_nan_event_dispatcher;
	msg.flush_callback = target_if_nan_event_flush_cb;
	target_if_debug("NDP_END_IND sent: %d", msg.type);
	status = scheduler_post_message(QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		target_if_nan_event_flush_cb(&msg);
		return -EINVAL;
	}

	return 0;
}

static int target_if_ndp_sch_update_handler(ol_scn_t scn, uint8_t *data,
					    uint32_t data_len)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct scheduler_msg msg = {0};
	struct nan_datapath_sch_update_event *rsp;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null.");
		return -EINVAL;
	}

	rsp = qdf_mem_malloc(sizeof(*rsp));
	if (!rsp) {
		target_if_err("malloc failed");
		return -ENOMEM;
	}

	status = wmi_extract_ndp_sch_update(wmi_handle, data, rsp);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("parsing of event failed, %d", status);
		qdf_mem_free(rsp);
		return -EINVAL;
	}

	msg.bodyptr = rsp;
	msg.type = NDP_SCHEDULE_UPDATE;
	msg.callback = target_if_nan_event_dispatcher;
	msg.flush_callback = target_if_nan_event_flush_cb;
	target_if_debug("NDP_SCHEDULE_UPDATE sent: %d", msg.type);
	status = scheduler_post_message(QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		target_if_nan_event_flush_cb(&msg);
		return -EINVAL;
	}

	return 0;
}

static QDF_STATUS target_if_nan_req(void *req, uint32_t req_type)
{
	/* send cmd to fw */
	switch (req_type) {
	case NDP_INITIATOR_REQ:
		target_if_nan_ndp_initiator_req(req);
		break;
	case NDP_RESPONDER_REQ:
		target_if_nan_ndp_responder_req(req);
		break;
	case NDP_END_REQ:
		target_if_nan_ndp_end_req(req);
		break;
	default:
		target_if_err("invalid req type");
		break;
	}
	return QDF_STATUS_SUCCESS;
}

void target_if_nan_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->nan_tx_ops.nan_req_tx = target_if_nan_req;
}

void target_if_nan_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	rx_ops->nan_rx_ops.nan_event_rx = nan_event_handler;
}

inline struct wlan_lmac_if_nan_tx_ops *target_if_nan_get_tx_ops(
				struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		target_if_err("psoc is null");
		return NULL;
	}

	return &psoc->soc_cb.tx_ops.nan_tx_ops;
}

inline struct wlan_lmac_if_nan_rx_ops *target_if_nan_get_rx_ops(
				struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		target_if_err("psoc is null");
		return NULL;
	}

	return &psoc->soc_cb.rx_ops.nan_rx_ops;
}

QDF_STATUS target_if_nan_register_events(struct wlan_objmgr_psoc *psoc)
{
	int ret;
	wmi_unified_t handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!handle) {
		target_if_err("handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	ret = wmi_unified_register_event_handler(handle,
		wmi_ndp_initiator_rsp_event_id,
		target_if_ndp_initiator_rsp_handler,
		WMI_RX_UMAC_CTX);
	if (ret) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
		wmi_ndp_indication_event_id,
		target_if_ndp_ind_handler,
		WMI_RX_UMAC_CTX);
	if (ret) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		target_if_nan_deregister_events(psoc);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
		wmi_ndp_confirm_event_id,
		target_if_ndp_confirm_handler,
		WMI_RX_UMAC_CTX);
	if (ret) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		target_if_nan_deregister_events(psoc);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
		wmi_ndp_responder_rsp_event_id,
		target_if_ndp_responder_rsp_handler,
		WMI_RX_UMAC_CTX);
	if (ret) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		target_if_nan_deregister_events(psoc);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
		wmi_ndp_end_indication_event_id,
		target_if_ndp_end_ind_handler,
		WMI_RX_UMAC_CTX);
	if (ret) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		target_if_nan_deregister_events(psoc);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
		wmi_ndp_end_rsp_event_id,
		target_if_ndp_end_rsp_handler,
		WMI_RX_UMAC_CTX);
	if (ret) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		target_if_nan_deregister_events(psoc);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
		wmi_ndl_schedule_update_event_id,
		target_if_ndp_sch_update_handler,
		WMI_RX_UMAC_CTX);
	if (ret) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		target_if_nan_deregister_events(psoc);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_nan_deregister_events(struct wlan_objmgr_psoc *psoc)
{
	int ret, status = 0;
	wmi_unified_t handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!handle) {
		target_if_err("handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	ret = wmi_unified_unregister_event_handler(handle,
				wmi_ndl_schedule_update_event_id);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	ret = wmi_unified_unregister_event_handler(handle,
				wmi_ndp_end_rsp_event_id);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	ret = wmi_unified_unregister_event_handler(handle,
				wmi_ndp_end_indication_event_id);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	ret = wmi_unified_unregister_event_handler(handle,
				wmi_ndp_responder_rsp_event_id);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	ret = wmi_unified_unregister_event_handler(handle,
				wmi_ndp_confirm_event_id);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	ret = wmi_unified_unregister_event_handler(handle,
				wmi_ndp_indication_event_id);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	ret = wmi_unified_unregister_event_handler(handle,
				wmi_ndp_initiator_rsp_event_id);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	if (status)
		return QDF_STATUS_E_FAILURE;
	else
		return QDF_STATUS_SUCCESS;
}
