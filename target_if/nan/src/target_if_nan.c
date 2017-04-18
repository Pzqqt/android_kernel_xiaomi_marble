/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

static QDF_STATUS target_if_nan_event_dispatcher(struct scheduler_msg *msg)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev = NULL;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_nan_rx_ops *nan_rx_ops;

	switch (msg->type) {
	case NDP_INITIATOR_RSP: {
		struct nan_datapath_initiator_rsp *initiator_rsp = msg->bodyptr;
		vdev = initiator_rsp->vdev;
		break;
	}
	case NDP_INDICATION: {
		struct nan_datapath_indication_event *ind = msg->bodyptr;
		vdev = ind->vdev;
		break;
	}
	case NDP_CONFIRM: {
		struct nan_datapath_confirm_event *confirm = msg->bodyptr;
		vdev = confirm->vdev;
		break;
	}
	case NDP_RESPONDER_RSP: {
		struct nan_datapath_responder_rsp *rsp = msg->bodyptr;
		vdev = rsp->vdev;
		break;
	}
	case NDP_END_RSP: {
		struct nan_datapath_end_rsp_event *rsp = msg->bodyptr;
		vdev = rsp->vdev;
		break;
	}
	case NDP_END_IND: {
		struct nan_datapath_end_indication_event *rsp = msg->bodyptr;
		vdev = rsp->vdev;
		break;
	}
	default:
		target_if_err("invalid msg type %d", msg->type);
		qdf_mem_free(msg->bodyptr);
		return QDF_STATUS_E_INVAL;
	}

	if (!vdev) {
		target_if_err("vdev is null");
		qdf_mem_free(msg->bodyptr);
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* try get ref now, if failure, then vdev may have been deleted */
	status = wlan_objmgr_vdev_try_get_ref(vdev, WLAN_NAN_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		nan_alert("couldn't get ref. vdev maybe deleted");
		qdf_mem_free(msg->bodyptr);
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		target_if_err("psoc is null");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
		qdf_mem_free(msg->bodyptr);
		return QDF_STATUS_E_NULL_VALUE;
	}

	nan_rx_ops = target_if_nan_get_rx_ops(psoc);
	if (!nan_rx_ops) {
		target_if_err("nan_rx_ops is null");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
		qdf_mem_free(msg->bodyptr);
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = nan_rx_ops->nan_event_rx(msg);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
	qdf_mem_free(msg->bodyptr);

	return status;
}

static QDF_STATUS target_if_nan_ndp_intiaitor_req(void *req)
{
	int ret;
	uint16_t len;
	wmi_buf_t buf;
	uint8_t *tlv_ptr;
	QDF_STATUS status;
	wmi_channel *ch_tlv;
	wmi_unified_t wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct scheduler_msg pe_msg = {0};
	wmi_ndp_initiator_req_fixed_param *cmd;
	struct wlan_lmac_if_nan_rx_ops *nan_rx_ops;
	uint32_t ndp_cfg_len, ndp_app_info_len, pmk_len;
	struct nan_datapath_initiator_rsp ndp_rsp = {0};
	struct nan_datapath_initiator_req *ndp_req = req;

	if (!ndp_req) {
		target_if_err("ndp_req is null.");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(ndp_req->vdev);
	if (!psoc) {
		target_if_err("psoc is null.");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null.");
		return QDF_STATUS_E_INVAL;
	}

	nan_rx_ops = target_if_nan_get_rx_ops(psoc);
	if (!nan_rx_ops) {
		target_if_err("nan_rx_ops is null.");
		return QDF_STATUS_E_INVAL;
	}

	/*
	 * WMI command expects 4 byte alligned len:
	 * round up ndp_cfg_len and ndp_app_info_len to 4 bytes
	 */
	ndp_cfg_len = qdf_roundup(ndp_req->ndp_config.ndp_cfg_len, 4);
	ndp_app_info_len = qdf_roundup(ndp_req->ndp_info.ndp_app_info_len, 4);
	pmk_len = qdf_roundup(ndp_req->pmk.pmk_len, 4);
	/* allocated memory for fixed params as well as variable size data */
	len = sizeof(*cmd) + sizeof(*ch_tlv) + (3 * WMI_TLV_HDR_SIZE)
		+ ndp_cfg_len + ndp_app_info_len + pmk_len;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		target_if_err("wmi_buf_alloc failed");
		status = QDF_STATUS_E_NOMEM;
		goto send_ndp_initiator_fail;
	}
	cmd = (wmi_ndp_initiator_req_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_ndp_initiator_req_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_ndp_initiator_req_fixed_param));
	cmd->vdev_id = wlan_vdev_get_id(ndp_req->vdev);
	cmd->transaction_id = ndp_req->transaction_id;
	cmd->service_instance_id = ndp_req->service_instance_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(ndp_req->peer_discovery_mac_addr.bytes,
				   &cmd->peer_discovery_mac_addr);

	cmd->ndp_cfg_len = ndp_req->ndp_config.ndp_cfg_len;
	cmd->ndp_app_info_len = ndp_req->ndp_info.ndp_app_info_len;
	cmd->ndp_channel_cfg = ndp_req->channel_cfg;
	cmd->nan_pmk_len = ndp_req->pmk.pmk_len;
	cmd->nan_csid = ndp_req->ncs_sk_type;

	ch_tlv = (wmi_channel *)&cmd[1];
	WMITLV_SET_HDR(ch_tlv, WMITLV_TAG_STRUC_wmi_channel,
			WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
	ch_tlv->mhz = ndp_req->channel;
	/*
	ch_tlv->band_center_freq1 =
		cds_chan_to_freq(cds_freq_to_chan(ndp_req->channel));
	*/
	tlv_ptr = (uint8_t *)&ch_tlv[1];

	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, ndp_cfg_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE],
		     ndp_req->ndp_config.ndp_cfg, cmd->ndp_cfg_len);
	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + ndp_cfg_len;

	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, ndp_app_info_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE],
		     ndp_req->ndp_info.ndp_app_info, cmd->ndp_app_info_len);
	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + ndp_app_info_len;

	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, pmk_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE], ndp_req->pmk.pmk,
		     cmd->nan_pmk_len);
	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + pmk_len;

	target_if_debug("vdev_id = %d, transaction_id: %d, service_instance_id: %d, ch: %d, ch_cfg: %d, csid: %d",
		cmd->vdev_id, cmd->transaction_id, cmd->service_instance_id,
		ch_tlv->mhz, cmd->ndp_channel_cfg, cmd->nan_csid);
	target_if_debug("peer mac addr: mac_addr31to0: 0x%x, mac_addr47to32: 0x%x",
		cmd->peer_discovery_mac_addr.mac_addr31to0,
		cmd->peer_discovery_mac_addr.mac_addr47to32);

	target_if_debug("ndp_config len: %d", cmd->ndp_cfg_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
			   ndp_req->ndp_config.ndp_cfg,
			   ndp_req->ndp_config.ndp_cfg_len);

	target_if_debug("ndp_app_info len: %d", cmd->ndp_app_info_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
			   ndp_req->ndp_info.ndp_app_info,
			   ndp_req->ndp_info.ndp_app_info_len);

	target_if_debug("pmk len: %d", cmd->nan_pmk_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
			   ndp_req->pmk.pmk, cmd->nan_pmk_len);
	target_if_debug("sending WMI_NDP_INITIATOR_REQ_CMDID(0x%X)",
		WMI_NDP_INITIATOR_REQ_CMDID);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_NDP_INITIATOR_REQ_CMDID);
	if (ret < 0) {
		target_if_err("WMI_NDP_INITIATOR_REQ_CMDID failed, ret: %d",
			      ret);
		wmi_buf_free(buf);
		goto send_ndp_initiator_fail;
	}

	return QDF_STATUS_SUCCESS;
send_ndp_initiator_fail:
	ndp_rsp.vdev = ndp_req->vdev;
	ndp_rsp.transaction_id = ndp_req->transaction_id;
	ndp_rsp.ndp_instance_id = ndp_req->service_instance_id;
	ndp_rsp.status = NAN_DATAPATH_DATA_INITIATOR_REQ_FAILED;
	pe_msg.type = NDP_INITIATOR_RSP;
	pe_msg.bodyptr = &ndp_rsp;
	/* process even here and call callback */
	if (nan_rx_ops && nan_rx_ops->nan_event_rx)
		nan_rx_ops->nan_event_rx(&pe_msg);

	return status;
}

static int target_if_ndp_initiator_rsp_handler(ol_scn_t scn, uint8_t *data,
						uint32_t len)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;
	struct scheduler_msg msg = {0};
	struct nan_datapath_initiator_rsp *rsp;
	WMI_NDP_INITIATOR_RSP_EVENTID_param_tlvs *event;
	wmi_ndp_initiator_rsp_event_fixed_param  *fixed_params;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	event = (WMI_NDP_INITIATOR_RSP_EVENTID_param_tlvs *)data;
	fixed_params = event->fixed_param;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
				fixed_params->vdev_id, WLAN_NAN_ID);
	if (!vdev) {
		target_if_err("vdev is null");
		return -EINVAL;
	}

	rsp = qdf_mem_malloc(sizeof(*rsp));
	if (!rsp) {
		target_if_err("malloc failed");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
		return -ENOMEM;
	}

	rsp->vdev = vdev;
	rsp->transaction_id = fixed_params->transaction_id;
	rsp->ndp_instance_id = fixed_params->ndp_instance_id;
	rsp->status = fixed_params->rsp_status;
	rsp->reason = fixed_params->reason_code;

	msg.type = NDP_INITIATOR_RSP;
	msg.bodyptr = rsp;
	msg.callback = target_if_nan_event_dispatcher;

	target_if_err("NDP_INITIATOR_RSP sent: %d", msg.type);
	status = scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		qdf_mem_free(rsp);
		return -EINVAL;
	}

	return 0;
}

static int target_if_ndp_ind_handler(ol_scn_t scn, uint8_t *data,
					uint32_t data_len)
{
	int ret;
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;
	struct scheduler_msg msg = {0};
	struct nan_datapath_indication_event *rsp;
	WMI_NDP_INDICATION_EVENTID_param_tlvs *event;
	wmi_ndp_indication_event_fixed_param *fixed_params;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	event = (WMI_NDP_INDICATION_EVENTID_param_tlvs *)data;
	fixed_params =
		(wmi_ndp_indication_event_fixed_param *)event->fixed_param;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
					fixed_params->vdev_id, WLAN_NAN_ID);
	if (!vdev) {
		target_if_err("vdev is null");
		return -EINVAL;
	}

	rsp = qdf_mem_malloc(sizeof(*rsp));
	if (!rsp) {
		target_if_err("malloc failed");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
		return -ENOMEM;
	}

	rsp->vdev = vdev;
	rsp->service_instance_id = fixed_params->service_instance_id;
	rsp->ndp_instance_id = fixed_params->ndp_instance_id;
	rsp->role = fixed_params->self_ndp_role;
	rsp->policy = fixed_params->accept_policy;

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&fixed_params->peer_ndi_mac_addr,
				rsp->peer_mac_addr.bytes);
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&fixed_params->peer_discovery_mac_addr,
				rsp->peer_discovery_mac_addr.bytes);

	target_if_debug("WMI_NDP_INDICATION_EVENTID(0x%X) received. vdev %d,\n"
		"service_instance %d, ndp_instance %d, role %d, policy %d,\n"
		"csid: %d, scid_len: %d, peer_addr: %pM, peer_disc_addr: %pM",
		 WMI_NDP_INDICATION_EVENTID, fixed_params->vdev_id,
		 fixed_params->service_instance_id,
		 fixed_params->ndp_instance_id, fixed_params->self_ndp_role,
		 fixed_params->accept_policy,
		 fixed_params->nan_csid, fixed_params->nan_scid_len,
		 rsp->peer_mac_addr.bytes,
		 rsp->peer_discovery_mac_addr.bytes);

	target_if_debug("ndp_cfg - %d bytes", fixed_params->ndp_cfg_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
			   &event->ndp_cfg, fixed_params->ndp_cfg_len);

	target_if_debug("ndp_app_info - %d bytes",
			fixed_params->ndp_app_info_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
			&event->ndp_app_info, fixed_params->ndp_app_info_len);

	rsp->ndp_config.ndp_cfg_len = fixed_params->ndp_cfg_len;
	rsp->ndp_info.ndp_app_info_len = fixed_params->ndp_app_info_len;
	rsp->ncs_sk_type = fixed_params->nan_csid;
	rsp->scid.scid_len = fixed_params->nan_scid_len;

	if (rsp->ndp_config.ndp_cfg_len) {
		rsp->ndp_config.ndp_cfg =
			qdf_mem_malloc(fixed_params->ndp_cfg_len);
		if (!rsp->ndp_config.ndp_cfg) {
			target_if_err("malloc failed");
			ret = -ENOMEM;
			goto free_ind_resources;
		}
		qdf_mem_copy(rsp->ndp_config.ndp_cfg, event->ndp_cfg,
			     rsp->ndp_config.ndp_cfg_len);
	}

	if (rsp->ndp_info.ndp_app_info_len) {
		rsp->ndp_info.ndp_app_info =
			qdf_mem_malloc(rsp->ndp_info.ndp_app_info_len);
		if (!rsp->ndp_info.ndp_app_info) {
			target_if_err("malloc failed");
			ret = -ENOMEM;
			goto free_ind_resources;
		}
		qdf_mem_copy(rsp->ndp_info.ndp_app_info,
			     event->ndp_app_info,
			     rsp->ndp_info.ndp_app_info_len);
	}

	if (rsp->scid.scid_len) {
		rsp->scid.scid =
			qdf_mem_malloc(rsp->scid.scid_len);
		if (!rsp->scid.scid) {
			target_if_err("malloc failed");
			ret = -ENOMEM;
			goto free_ind_resources;
		}
		qdf_mem_copy(rsp->scid.scid,
			     event->ndp_scid, rsp->scid.scid_len);
		target_if_debug("scid hex dump:");
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
			rsp->scid.scid, rsp->scid.scid_len);
	}

	msg.type = NDP_INDICATION;
	msg.bodyptr = rsp;
	msg.callback = target_if_nan_event_dispatcher;
	target_if_debug("NDP_INDICATION sent: %d", msg.type);
	status = scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		ret = -EINVAL;
		goto free_ind_resources;
	}
	wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
	return 0;

free_ind_resources:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
	qdf_mem_free(rsp->ndp_config.ndp_cfg);
	qdf_mem_free(rsp->ndp_info.ndp_app_info);
	qdf_mem_free(rsp->scid.scid);
	qdf_mem_free(rsp);

	return ret;
}

static int target_if_ndp_confirm_handler(ol_scn_t scn, uint8_t *data,
					uint32_t data_len)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;
	struct scheduler_msg msg = {0};
	WMI_NDP_CONFIRM_EVENTID_param_tlvs *event;
	wmi_ndp_confirm_event_fixed_param *fixed_params;
	struct nan_datapath_confirm_event *rsp;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	rsp = qdf_mem_malloc(sizeof(*rsp));
	if (!rsp) {
		target_if_err("malloc failed");
		return -ENOMEM;
	}

	event = (WMI_NDP_CONFIRM_EVENTID_param_tlvs *) data;
	fixed_params = (wmi_ndp_confirm_event_fixed_param *)event->fixed_param;
	target_if_debug("WMI_NDP_CONFIRM_EVENTID(0x%X) recieved. vdev %d, ndp_instance %d, rsp_code %d, reason_code: %d, num_active_ndps_on_peer: %d",
		 WMI_NDP_CONFIRM_EVENTID, fixed_params->vdev_id,
		 fixed_params->ndp_instance_id, fixed_params->rsp_code,
		 fixed_params->reason_code,
		 fixed_params->num_active_ndps_on_peer);
	target_if_debug("ndp_cfg - %d bytes", fixed_params->ndp_cfg_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
		&event->ndp_cfg, fixed_params->ndp_cfg_len);

	target_if_debug("ndp_app_info - %d bytes",
			fixed_params->ndp_app_info_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
		&event->ndp_app_info, fixed_params->ndp_app_info_len);

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
					fixed_params->vdev_id, WLAN_NAN_ID);
	if (!vdev) {
		target_if_err("vdev is null");
		return -EINVAL;
	}

	rsp->vdev = vdev;
	rsp->ndp_instance_id = fixed_params->ndp_instance_id;
	rsp->rsp_code = fixed_params->rsp_code;
	rsp->reason_code = fixed_params->reason_code;
	rsp->num_active_ndps_on_peer =
				fixed_params->num_active_ndps_on_peer;

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&fixed_params->peer_ndi_mac_addr,
				   rsp->peer_ndi_mac_addr.bytes);

	rsp->ndp_info.ndp_app_info_len = fixed_params->ndp_app_info_len;

	if (rsp->ndp_info.ndp_app_info_len) {
		rsp->ndp_info.ndp_app_info =
				qdf_mem_malloc(fixed_params->ndp_app_info_len);
		if (!rsp->ndp_info.ndp_app_info) {
			target_if_err("malloc failed");
			qdf_mem_free(rsp);
			wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
			return -ENOMEM;
		}
		qdf_mem_copy(&rsp->ndp_info.ndp_app_info,
			     event->ndp_app_info,
			     rsp->ndp_info.ndp_app_info_len);
	}
	msg.type = NDP_CONFIRM;
	msg.bodyptr = rsp;
	msg.callback = target_if_nan_event_dispatcher;
	target_if_err("NDP_CONFIRM sent: %d", msg.type);
	status = scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		qdf_mem_free(rsp->ndp_info.ndp_app_info);
		qdf_mem_free(rsp);
		return -EINVAL;
	}

	return 0;
}

static QDF_STATUS target_if_nan_ndp_responder_req(
				struct nan_datapath_responder_req *req)
{
	int ret;
	uint16_t len;
	wmi_buf_t buf;
	uint8_t *tlv_ptr;
	QDF_STATUS status;
	wmi_unified_t wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct scheduler_msg pe_msg = {0};
	wmi_ndp_responder_req_fixed_param *cmd;
	struct wlan_lmac_if_nan_rx_ops *nan_rx_ops;
	struct nan_datapath_responder_rsp rsp = {0};
	uint32_t vdev_id = 0, ndp_cfg_len, ndp_app_info_len, pmk_len;

	if (!req) {
		target_if_err("Invalid req.");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(req->vdev);
	if (!psoc) {
		target_if_err("psoc is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	nan_rx_ops = target_if_nan_get_rx_ops(psoc);
	if (!nan_rx_ops) {
		target_if_err("nan_rx_ops is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	vdev_id = wlan_vdev_get_id(req->vdev);
	target_if_debug("vdev_id: %d, transaction_id: %d, ndp_rsp %d, ndp_instance_id: %d, ndp_app_info_len: %d",
			vdev_id, req->transaction_id,
			req->ndp_rsp,
			req->ndp_instance_id,
			req->ndp_info.ndp_app_info_len);

	/*
	 * WMI command expects 4 byte alligned len:
	 * round up ndp_cfg_len and ndp_app_info_len to 4 bytes
	 */
	ndp_cfg_len = qdf_roundup(req->ndp_config.ndp_cfg_len, 4);
	ndp_app_info_len = qdf_roundup(req->ndp_info.ndp_app_info_len, 4);
	pmk_len = qdf_roundup(req->pmk.pmk_len, 4);
	/* allocated memory for fixed params as well as variable size data */
	len = sizeof(*cmd) + 3*WMI_TLV_HDR_SIZE + ndp_cfg_len + ndp_app_info_len
		+ pmk_len;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		target_if_err("wmi_buf_alloc failed");
		status = QDF_STATUS_E_NOMEM;
		goto send_ndp_responder_fail;
	}
	cmd = (wmi_ndp_responder_req_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_ndp_responder_req_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
				wmi_ndp_responder_req_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->transaction_id = req->transaction_id;
	cmd->ndp_instance_id = req->ndp_instance_id;
	cmd->rsp_code = req->ndp_rsp;
	cmd->ndp_cfg_len = req->ndp_config.ndp_cfg_len;
	cmd->ndp_app_info_len = req->ndp_info.ndp_app_info_len;
	cmd->nan_pmk_len = req->pmk.pmk_len;
	cmd->nan_csid = req->ncs_sk_type;

	tlv_ptr = (uint8_t *)&cmd[1];
	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, ndp_cfg_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE],
		req->ndp_config.ndp_cfg, cmd->ndp_cfg_len);

	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + ndp_cfg_len;
	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, ndp_app_info_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE],
		     req->ndp_info.ndp_app_info,
		     req->ndp_info.ndp_app_info_len);

	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + ndp_app_info_len;
	WMITLV_SET_HDR(tlv_ptr, WMITLV_TAG_ARRAY_BYTE, pmk_len);
	qdf_mem_copy(&tlv_ptr[WMI_TLV_HDR_SIZE], req->pmk.pmk,
		     cmd->nan_pmk_len);

	tlv_ptr = tlv_ptr + WMI_TLV_HDR_SIZE + pmk_len;
	target_if_debug("vdev_id = %d, transaction_id: %d, csid: %d",
		cmd->vdev_id, cmd->transaction_id, cmd->nan_csid);

	target_if_debug("ndp_config len: %d",
		req->ndp_config.ndp_cfg_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
			req->ndp_config.ndp_cfg,
			req->ndp_config.ndp_cfg_len);

	target_if_debug("ndp_app_info len: %d",
		req->ndp_info.ndp_app_info_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
			req->ndp_info.ndp_app_info,
			req->ndp_info.ndp_app_info_len);

	target_if_debug("pmk len: %d", cmd->nan_pmk_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
			   req->pmk.pmk, cmd->nan_pmk_len);

	target_if_debug("sending WMI_NDP_RESPONDER_REQ_CMDID(0x%X)",
		WMI_NDP_RESPONDER_REQ_CMDID);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_NDP_RESPONDER_REQ_CMDID);
	if (ret < 0) {
		target_if_err("WMI_NDP_RESPONDER_REQ_CMDID failed, ret: %d",
				ret);
		wmi_buf_free(buf);
		status = QDF_STATUS_E_FAILURE;
		goto send_ndp_responder_fail;
	}
	return QDF_STATUS_SUCCESS;

send_ndp_responder_fail:
	rsp.vdev = req->vdev;
	rsp.transaction_id = req->transaction_id;
	rsp.status = NAN_DATAPATH_RSP_STATUS_ERROR;
	rsp.reason = NAN_DATAPATH_DATA_RESPONDER_REQ_FAILED;
	pe_msg.bodyptr = &rsp;
	pe_msg.type = NDP_RESPONDER_RSP;
	if (nan_rx_ops && nan_rx_ops->nan_event_rx)
		nan_rx_ops->nan_event_rx(&pe_msg);

	return status;
}

static int target_if_ndp_responder_rsp_handler(ol_scn_t scn, uint8_t *data,
						uint32_t len)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;
	struct scheduler_msg msg = {0};
	struct wlan_lmac_if_nan_rx_ops *nan_rx_ops;
	struct nan_datapath_responder_rsp *rsp;
	WMI_NDP_RESPONDER_RSP_EVENTID_param_tlvs *event;
	wmi_ndp_responder_rsp_event_fixed_param  *fixed_params;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	nan_rx_ops = target_if_nan_get_rx_ops(psoc);
	/* process even here and call callback */
	if (!nan_rx_ops || !nan_rx_ops->nan_event_rx) {
		target_if_err("lmac callbacks not registered");
		return -EINVAL;
	}

	event = (WMI_NDP_RESPONDER_RSP_EVENTID_param_tlvs *)data;
	fixed_params = event->fixed_param;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
					fixed_params->vdev_id, WLAN_NAN_ID);
	if (!vdev) {
		target_if_err("vdev is null");
		return -EINVAL;
	}

	rsp = qdf_mem_malloc(sizeof(*rsp));
	if (!rsp) {
		target_if_err("malloc failed");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
		return -ENOMEM;
	}

	rsp->vdev = vdev;
	rsp->transaction_id = fixed_params->transaction_id;
	rsp->reason = fixed_params->reason_code;
	rsp->status = fixed_params->rsp_status;
	rsp->create_peer = fixed_params->create_peer;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&fixed_params->peer_ndi_mac_addr,
				rsp->peer_mac_addr.bytes);

	target_if_debug("WMI_NDP_RESPONDER_RSP_EVENTID(0x%X) received. vdev_id: %d, peer_mac_addr: %pM,transaction_id: %d, status_code %d, reason_code: %d, create_peer: %d",
			WMI_NDP_RESPONDER_RSP_EVENTID, fixed_params->vdev_id,
			rsp->peer_mac_addr.bytes, rsp->transaction_id,
			rsp->status, rsp->reason, rsp->create_peer);
	msg.bodyptr = rsp;
	msg.type = NDP_RESPONDER_RSP;
	msg.callback = target_if_nan_event_dispatcher;

	target_if_debug("NDP_INITIATOR_RSP sent: %d", msg.type);
	status = scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		qdf_mem_free(rsp);
		return -EINVAL;
	}

	return 0;
}

static QDF_STATUS target_if_nan_ndp_end_req(struct nan_datapath_end_req *req)
{
	int ret;
	uint16_t len;
	wmi_buf_t buf;
	QDF_STATUS status;
	wmi_unified_t wmi_handle;
	uint32_t ndp_end_req_len, i;
	struct wlan_objmgr_psoc *psoc;
	struct scheduler_msg msg = {0};
	wmi_ndp_end_req *ndp_end_req_lst;
	wmi_ndp_end_req_fixed_param *cmd;
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

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	nan_rx_ops = target_if_nan_get_rx_ops(psoc);
	if (!nan_rx_ops) {
		target_if_err("nan_rx_ops is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* len of tlv following fixed param  */
	ndp_end_req_len = sizeof(wmi_ndp_end_req) * req->num_ndp_instances;
	/* above comes out to 4 byte alligned already, no need of padding */
	len = sizeof(*cmd) + ndp_end_req_len + WMI_TLV_HDR_SIZE;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		target_if_err("Malloc failed");
		status = QDF_STATUS_E_NOMEM;
		goto send_ndp_end_fail;
	}
	cmd = (wmi_ndp_end_req_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_ndp_end_req_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_ndp_end_req_fixed_param));

	cmd->transaction_id = req->transaction_id;

	/* set tlv pointer to end of fixed param */
	WMITLV_SET_HDR((uint8_t *)&cmd[1], WMITLV_TAG_ARRAY_STRUC,
			ndp_end_req_len);

	ndp_end_req_lst = (wmi_ndp_end_req *)((uint8_t *)&cmd[1] +
						WMI_TLV_HDR_SIZE);
	for (i = 0; i < req->num_ndp_instances; i++) {
		WMITLV_SET_HDR(&ndp_end_req_lst[i],
				WMITLV_TAG_ARRAY_FIXED_STRUC,
				(sizeof(*ndp_end_req_lst) - WMI_TLV_HDR_SIZE));

		ndp_end_req_lst[i].ndp_instance_id = req->ndp_ids[i];
	}

	target_if_debug("Sending WMI_NDP_END_REQ_CMDID to FW");
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_NDP_END_REQ_CMDID);
	if (ret < 0) {
		target_if_err("WMI_NDP_END_REQ_CMDID failed, ret: %d", ret);
		wmi_buf_free(buf);
		status = QDF_STATUS_E_FAILURE;
		goto send_ndp_end_fail;
	}
	return QDF_STATUS_SUCCESS;

send_ndp_end_fail:
	end_rsp.vdev = req->vdev;
	msg.type = NDP_END_RSP;
	end_rsp.status = NAN_DATAPATH_RSP_STATUS_ERROR;
	end_rsp.reason = NAN_DATAPATH_END_FAILED;
	end_rsp.transaction_id = req->transaction_id;
	msg.bodyptr = &end_rsp;

	if (nan_rx_ops && nan_rx_ops->nan_event_rx)
		nan_rx_ops->nan_event_rx(&msg);

	return status;
}

static int target_if_ndp_end_rsp_handler(ol_scn_t scn, uint8_t *data,
					 uint32_t data_len)
{
	int ret = 0;
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;
	struct scheduler_msg msg = {0};
	WMI_NDP_END_RSP_EVENTID_param_tlvs *event;
	struct nan_datapath_end_rsp_event *end_rsp;
	struct wlan_lmac_if_nan_rx_ops *nan_rx_ops;
	wmi_ndp_end_rsp_event_fixed_param *fixed_params = NULL;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	nan_rx_ops = target_if_nan_get_rx_ops(psoc);
	/* process even here and call callback */
	if (!nan_rx_ops || !nan_rx_ops->nan_event_rx) {
		target_if_err("lmac callbacks not registered");
		return -EINVAL;
	}

	event = (WMI_NDP_END_RSP_EVENTID_param_tlvs *) data;
	fixed_params = (wmi_ndp_end_rsp_event_fixed_param *)event->fixed_param;
	target_if_debug("WMI_NDP_END_RSP_EVENTID(0x%X) recieved. transaction_id: %d, rsp_status: %d, reason_code: %d",
		 WMI_NDP_END_RSP_EVENTID, fixed_params->transaction_id,
		 fixed_params->rsp_status, fixed_params->reason_code);

	vdev = ucfg_nan_get_ndi_vdev(psoc, WLAN_NAN_ID);
	if (!vdev) {
		target_if_err("vdev is null");
		return -EINVAL;
	}

	end_rsp = qdf_mem_malloc(sizeof(*end_rsp));
	if (!end_rsp) {
		target_if_err("malloc failed");
		ret = -ENOMEM;
		wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
		goto send_ndp_end_rsp;
	}

	end_rsp->vdev = vdev;
	end_rsp->transaction_id = fixed_params->transaction_id;
	end_rsp->reason = fixed_params->reason_code;
	end_rsp->status = fixed_params->rsp_status;

send_ndp_end_rsp:
	msg.bodyptr = end_rsp;
	msg.type = NDP_END_RSP;
	msg.callback = target_if_nan_event_dispatcher;
	target_if_err("NDP_END_RSP sent: %d", msg.type);
	status = scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		qdf_mem_free(end_rsp);
		return -EINVAL;
	}
	return ret;
}

static int target_if_ndp_end_ind_handler(ol_scn_t scn, uint8_t *data,
					 uint32_t data_len)
{
	int i, buf_size;
	QDF_STATUS status;
	struct scheduler_msg msg = {0};
	wmi_ndp_end_indication *ind;
	struct qdf_mac_addr peer_addr;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_lmac_if_nan_rx_ops *nan_rx_ops;
	WMI_NDP_END_INDICATION_EVENTID_param_tlvs *event;
	struct nan_datapath_end_indication_event *rsp;

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	nan_rx_ops = target_if_nan_get_rx_ops(psoc);
	if (!nan_rx_ops || !nan_rx_ops->nan_event_rx) {
		target_if_err("lmac callbacks not registered");
		return -EINVAL;
	}

	event = (WMI_NDP_END_INDICATION_EVENTID_param_tlvs *) data;
	if (event->num_ndp_end_indication_list == 0) {
		target_if_err("Error: Event ignored, 0 ndp instances");
		return -EINVAL;
	}

	vdev = ucfg_nan_get_ndi_vdev(psoc, WLAN_NAN_ID);
	if (!vdev) {
		target_if_err("vdev is null");
		return -EINVAL;
	}

	target_if_debug("number of ndp instances = %d",
			event->num_ndp_end_indication_list);
	buf_size = sizeof(*rsp) + event->num_ndp_end_indication_list *
			sizeof(rsp->ndp_map[0]);
	rsp = qdf_mem_malloc(buf_size);
	if (!rsp) {
		target_if_err("Failed to allocate memory");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
		return -ENOMEM;
	}

	rsp->vdev = vdev;
	rsp->num_ndp_ids = event->num_ndp_end_indication_list;

	ind = event->ndp_end_indication_list;
	for (i = 0; i < rsp->num_ndp_ids; i++) {
		WMI_MAC_ADDR_TO_CHAR_ARRAY(
			&ind[i].peer_ndi_mac_addr,
			peer_addr.bytes);
		/* add mac address print - TBD */
		target_if_debug(
			"ind[%d]: type %d, reason_code %d, instance_id %d num_active %d ",
			i, ind[i].type, ind[i].reason_code,
			ind[i].ndp_instance_id, ind[i].num_active_ndps_on_peer);

		/* Add each instance entry to the list */
		rsp->ndp_map[i].ndp_instance_id =
			ind[i].ndp_instance_id;
		rsp->ndp_map[i].vdev_id = ind[i].vdev_id;
		WMI_MAC_ADDR_TO_CHAR_ARRAY(&ind[i].peer_ndi_mac_addr,
			rsp->ndp_map[i].peer_ndi_mac_addr.bytes);
		rsp->ndp_map[i].num_active_ndp_sessions =
			ind[i].num_active_ndps_on_peer;
		rsp->ndp_map[i].type = ind[i].type;
		rsp->ndp_map[i].reason_code =
			ind[i].reason_code;
	}

	msg.type = NDP_END_IND;
	msg.bodyptr = rsp;
	msg.callback = target_if_nan_event_dispatcher;

	target_if_err("NDP_END_IND sent: %d", msg.type);
	status = scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to post msg, status: %d", status);
		qdf_mem_free(rsp);
		return -EINVAL;
	}
	return 0;
}

static QDF_STATUS target_if_nan_req(void *req, uint32_t req_type)
{
	/* send cmd to fw */
	switch (req_type) {
	case NDP_INITIATOR_REQ:
		target_if_nan_ndp_intiaitor_req(req);
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
	wmi_unified_t handle = GET_WMI_HDL_FROM_PSOC(psoc);

	ret = wmi_unified_register_event_handler(handle,
		WMI_NDP_INITIATOR_RSP_EVENTID,
		target_if_ndp_initiator_rsp_handler,
		WMI_RX_UMAC_CTX);
	if (ret) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
		WMI_NDP_INDICATION_EVENTID,
		target_if_ndp_ind_handler,
		WMI_RX_UMAC_CTX);
	if (ret) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		target_if_nan_deregister_events(psoc);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
		WMI_NDP_CONFIRM_EVENTID,
		target_if_ndp_confirm_handler,
		WMI_RX_UMAC_CTX);
	if (ret) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		target_if_nan_deregister_events(psoc);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
		WMI_NDP_RESPONDER_RSP_EVENTID,
		target_if_ndp_responder_rsp_handler,
		WMI_RX_UMAC_CTX);
	if (ret) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		target_if_nan_deregister_events(psoc);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
		WMI_NDP_END_INDICATION_EVENTID,
		target_if_ndp_end_ind_handler,
		WMI_RX_UMAC_CTX);
	if (ret) {
		target_if_err("wmi event registration failed, ret: %d", ret);
		target_if_nan_deregister_events(psoc);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wmi_unified_register_event_handler(handle,
		WMI_NDP_END_RSP_EVENTID,
		target_if_ndp_end_rsp_handler,
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
	wmi_unified_t handle = GET_WMI_HDL_FROM_PSOC(psoc);

	ret = wmi_unified_unregister_event_handler(handle,
				WMI_NDP_INITIATOR_RSP_EVENTID);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	ret = wmi_unified_unregister_event_handler(handle,
				WMI_NDP_INDICATION_EVENTID);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	ret = wmi_unified_unregister_event_handler(handle,
				WMI_NDP_CONFIRM_EVENTID);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	ret = wmi_unified_unregister_event_handler(handle,
				WMI_NDP_RESPONDER_RSP_EVENTID);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	ret = wmi_unified_unregister_event_handler(handle,
				WMI_NDP_END_INDICATION_EVENTID);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	ret = wmi_unified_unregister_event_handler(handle,
				WMI_NDP_END_RSP_EVENTID);
	if (ret) {
		target_if_err("wmi event deregistration failed, ret: %d", ret);
		status = ret;
	}

	if (status)
		return QDF_STATUS_E_FAILURE;
	else
		return QDF_STATUS_SUCCESS;
}
