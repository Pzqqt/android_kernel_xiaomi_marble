/*
 * Copyright (c) 2013-2018, 2020 The Linux Foundation. All rights reserved.
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

#include <osdep.h>
#include <wmi.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_p2p_api.h>

/**
 * send_set_p2pgo_noa_req_cmd_tlv() - send p2p go noa request to fw
 * @wmi_handle: wmi handle
 * @noa: p2p power save parameters
 *
 * Return: CDF status
 */
static QDF_STATUS send_set_p2pgo_noa_req_cmd_tlv(wmi_unified_t wmi_handle,
						 struct p2p_ps_params *noa)
{
	wmi_p2p_set_noa_cmd_fixed_param *cmd;
	wmi_p2p_noa_descriptor *noa_discriptor;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint16_t len;
	QDF_STATUS status;
	uint32_t duration;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE + sizeof(*noa_discriptor);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_p2p_set_noa_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_p2p_set_noa_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_p2p_set_noa_cmd_fixed_param));
	duration = (noa->count == 1) ? noa->single_noa_duration : noa->duration;
	cmd->vdev_id = noa->session_id;
	cmd->enable = (duration) ? true : false;
	cmd->num_noa = 1;

	WMITLV_SET_HDR((buf_ptr + sizeof(wmi_p2p_set_noa_cmd_fixed_param)),
		       WMITLV_TAG_ARRAY_STRUC, sizeof(wmi_p2p_noa_descriptor));
	noa_discriptor = (wmi_p2p_noa_descriptor *)(buf_ptr +
						    sizeof
						    (wmi_p2p_set_noa_cmd_fixed_param)
						     + WMI_TLV_HDR_SIZE);
	WMITLV_SET_HDR(&noa_discriptor->tlv_header,
		       WMITLV_TAG_STRUC_wmi_p2p_noa_descriptor,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_p2p_noa_descriptor));
	noa_discriptor->type_count = noa->count;
	noa_discriptor->duration = duration;
	noa_discriptor->interval = noa->interval;
	noa_discriptor->start_time = 0;

	wmi_debug("SET P2P GO NOA:vdev_id:%d count:%d duration:%d interval:%d",
		 cmd->vdev_id, noa->count, noa_discriptor->duration,
		 noa->interval);
	wmi_mtrace(WMI_FWTEST_P2P_SET_NOA_PARAM_CMDID, cmd->vdev_id, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_FWTEST_P2P_SET_NOA_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_FWTEST_P2P_SET_NOA_PARAM_CMDID");
		wmi_buf_free(buf);
	}

end:
	return status;
}

/**
 * send_set_p2pgo_oppps_req_cmd_tlv() - send p2p go opp power save request to fw
 * @wmi_handle: wmi handle
 * @noa: p2p opp power save parameters
 *
 * Return: CDF status
 */
static QDF_STATUS send_set_p2pgo_oppps_req_cmd_tlv(wmi_unified_t wmi_handle,
						   struct p2p_ps_params *oppps)
{
	wmi_p2p_set_oppps_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	cmd = (wmi_p2p_set_oppps_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_p2p_set_oppps_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
					wmi_p2p_set_oppps_cmd_fixed_param));
	cmd->vdev_id = oppps->session_id;
	if (oppps->ctwindow)
		WMI_UNIFIED_OPPPS_ATTR_ENABLED_SET(cmd);

	WMI_UNIFIED_OPPPS_ATTR_CTWIN_SET(cmd, oppps->ctwindow);
	wmi_debug("SET P2P GO OPPPS:vdev_id:%d ctwindow:%d",
		 cmd->vdev_id, oppps->ctwindow);
	wmi_mtrace(WMI_P2P_SET_OPPPS_PARAM_CMDID, cmd->vdev_id, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_P2P_SET_OPPPS_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to send WMI_P2P_SET_OPPPS_PARAM_CMDID");
		wmi_buf_free(buf);
	}

end:
	return status;
}

/**
 * extract_p2p_noa_ev_param_tlv() - extract p2p noa information from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold p2p noa info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_p2p_noa_ev_param_tlv(
	wmi_unified_t wmi_handle, void *evt_buf,
	struct p2p_noa_info *param)
{
	WMI_P2P_NOA_EVENTID_param_tlvs *param_tlvs;
	wmi_p2p_noa_event_fixed_param *fixed_param;
	uint8_t i;
	wmi_p2p_noa_info *wmi_noa_info;
	uint8_t *buf_ptr;
	uint32_t descriptors;

	param_tlvs = (WMI_P2P_NOA_EVENTID_param_tlvs *)evt_buf;
	if (!param_tlvs) {
		wmi_err("Invalid P2P NoA event buffer");
		return QDF_STATUS_E_INVAL;
	}

	if (!param) {
		wmi_err("noa information param is null");
		return QDF_STATUS_E_INVAL;
	}

	fixed_param = param_tlvs->fixed_param;
	buf_ptr = (uint8_t *) fixed_param;
	buf_ptr += sizeof(wmi_p2p_noa_event_fixed_param);
	wmi_noa_info = (wmi_p2p_noa_info *) (buf_ptr);

	if (!WMI_UNIFIED_NOA_ATTR_IS_MODIFIED(wmi_noa_info)) {
		wmi_err("noa attr is not modified");
		return QDF_STATUS_E_INVAL;
	}

	param->vdev_id = fixed_param->vdev_id;
	param->index =
		(uint8_t)WMI_UNIFIED_NOA_ATTR_INDEX_GET(wmi_noa_info);
	param->opps_ps =
		(uint8_t)WMI_UNIFIED_NOA_ATTR_OPP_PS_GET(wmi_noa_info);
	param->ct_window =
		(uint8_t)WMI_UNIFIED_NOA_ATTR_CTWIN_GET(wmi_noa_info);
	descriptors = WMI_UNIFIED_NOA_ATTR_NUM_DESC_GET(wmi_noa_info);
	param->num_desc = (uint8_t)descriptors;
	if (param->num_desc > WMI_P2P_MAX_NOA_DESCRIPTORS) {
		wmi_err("Invalid num desc: %d", param->num_desc);
		return QDF_STATUS_E_INVAL;
	}

	wmi_debug("index %u, opps_ps %u, ct_window %u, num_descriptors = %u",
		 param->index, param->opps_ps, param->ct_window,
		 param->num_desc);
	for (i = 0; i < param->num_desc; i++) {
		param->noa_desc[i].type_count =
			(uint8_t)wmi_noa_info->noa_descriptors[i].
			type_count;
		param->noa_desc[i].duration =
			wmi_noa_info->noa_descriptors[i].duration;
		param->noa_desc[i].interval =
			wmi_noa_info->noa_descriptors[i].interval;
		param->noa_desc[i].start_time =
			wmi_noa_info->noa_descriptors[i].start_time;
		wmi_debug("NoA descriptor[%d] type_count %u, duration %u, interval %u, start_time = %u",
			 i, param->noa_desc[i].type_count,
			param->noa_desc[i].duration,
			param->noa_desc[i].interval,
			param->noa_desc[i].start_time);
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
send_set_mac_addr_rx_filter_cmd_tlv(wmi_unified_t wmi_handle,
				    struct p2p_set_mac_filter *param)
{
	wmi_vdev_add_mac_addr_to_rx_filter_cmd_fixed_param *cmd;
	uint32_t len;
	wmi_buf_t buf;
	int ret;

	if (!wmi_handle) {
		wmi_err("WMA context is invald!");
		return QDF_STATUS_E_INVAL;
	}

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("Failed allocate wmi buffer");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_vdev_add_mac_addr_to_rx_filter_cmd_fixed_param *)
		wmi_buf_data(buf);

	WMITLV_SET_HDR(
	   &cmd->tlv_header,
	   WMITLV_TAG_STRUC_wmi_vdev_add_mac_addr_to_rx_filter_cmd_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN(
			wmi_vdev_add_mac_addr_to_rx_filter_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	cmd->freq = param->freq;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->mac, &cmd->mac_addr);
	if (param->set)
		cmd->enable = 1;
	else
		cmd->enable = 0;
	wmi_debug("set random mac rx vdev %d freq %d set %d "QDF_MAC_ADDR_FMT,
		 param->vdev_id, param->freq, param->set,
		 QDF_MAC_ADDR_REF(param->mac));
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_ADD_MAC_ADDR_TO_RX_FILTER_CMDID);
	if (ret) {
		wmi_err("Failed to send action frame random mac cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS extract_mac_addr_rx_filter_evt_param_tlv(
	wmi_unified_t wmi_handle, void *evt_buf,
	struct p2p_set_mac_filter_evt *param)
{
	WMI_VDEV_ADD_MAC_ADDR_TO_RX_FILTER_STATUS_EVENTID_param_tlvs *param_buf;
	wmi_vdev_add_mac_addr_to_rx_filter_status_event_fixed_param *event;

	param_buf =
		(WMI_VDEV_ADD_MAC_ADDR_TO_RX_FILTER_STATUS_EVENTID_param_tlvs *)
		evt_buf;
	if (!param_buf) {
		wmi_err("Invalid action frame filter mac event");
		return QDF_STATUS_E_INVAL;
	}
	event = param_buf->fixed_param;
	if (!event) {
		wmi_err("Invalid fixed param");
		return QDF_STATUS_E_INVAL;
	}
	param->vdev_id = event->vdev_id;
	param->status = event->status;

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_P2P_LISTEN_OFFLOAD
/**
 * send_p2p_lo_start_cmd_tlv() - send p2p lo start request to fw
 * @wmi_handle: wmi handle
 * @param: p2p listen offload start parameters
 *
 * Return: QDF status
 */
static QDF_STATUS send_p2p_lo_start_cmd_tlv(wmi_unified_t wmi_handle,
					    struct p2p_lo_start *param)
{
	wmi_buf_t buf;
	wmi_p2p_lo_start_cmd_fixed_param *cmd;
	int32_t len = sizeof(*cmd);
	uint8_t *buf_ptr;
	QDF_STATUS status;
	int device_types_len_aligned;
	int probe_resp_len_aligned;

	if (!param) {
		wmi_err("lo start param is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_debug("vdev_id: %d", param->vdev_id);

	device_types_len_aligned =
		qdf_roundup(param->dev_types_len,
			    sizeof(uint32_t));
	probe_resp_len_aligned =
		qdf_roundup(param->probe_resp_len,
			    sizeof(uint32_t));

	len += 2 * WMI_TLV_HDR_SIZE + device_types_len_aligned +
			probe_resp_len_aligned;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_p2p_lo_start_cmd_fixed_param *)wmi_buf_data(buf);
	buf_ptr = (uint8_t *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		 WMITLV_TAG_STRUC_wmi_p2p_lo_start_cmd_fixed_param,
		 WMITLV_GET_STRUCT_TLVLEN(wmi_p2p_lo_start_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	cmd->ctl_flags = param->ctl_flags;
	cmd->channel = param->freq;
	cmd->period = param->period;
	cmd->interval = param->interval;
	cmd->count = param->count;
	cmd->device_types_len = param->dev_types_len;
	cmd->prob_resp_len = param->probe_resp_len;

	buf_ptr += sizeof(wmi_p2p_lo_start_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
		       device_types_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, param->device_types,
		     param->dev_types_len);

	buf_ptr += device_types_len_aligned;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
		       probe_resp_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, param->probe_resp_tmplt,
		     param->probe_resp_len);

	wmi_debug("Sending WMI_P2P_LO_START command, channel=%d, period=%d, interval=%d, count=%d",
		 cmd->channel, cmd->period, cmd->interval, cmd->count);

	wmi_mtrace(WMI_P2P_LISTEN_OFFLOAD_START_CMDID, cmd->vdev_id, 0);
	status = wmi_unified_cmd_send(wmi_handle,
				      buf, len,
				      WMI_P2P_LISTEN_OFFLOAD_START_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		wmi_err("Failed to send p2p lo start: %d", status);
		wmi_buf_free(buf);
		return status;
	}

	wmi_debug("Successfully sent WMI_P2P_LO_START");

	return QDF_STATUS_SUCCESS;
}

/**
 * send_p2p_lo_stop_cmd_tlv() - send p2p lo stop request to fw
 * @wmi_handle: wmi handle
 * @param: p2p listen offload stop parameters
 *
 * Return: QDF status
 */
static QDF_STATUS send_p2p_lo_stop_cmd_tlv(wmi_unified_t wmi_handle,
					   uint8_t vdev_id)
{
	wmi_buf_t buf;
	wmi_p2p_lo_stop_cmd_fixed_param *cmd;
	int32_t len;
	QDF_STATUS status;

	wmi_debug("vdev_id: %d", vdev_id);

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_p2p_lo_stop_cmd_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_p2p_lo_stop_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_p2p_lo_stop_cmd_fixed_param));

	cmd->vdev_id = vdev_id;

	wmi_debug("Sending WMI_P2P_LO_STOP command");

	wmi_mtrace(WMI_P2P_LISTEN_OFFLOAD_STOP_CMDID, cmd->vdev_id, 0);
	status = wmi_unified_cmd_send(wmi_handle,
				      buf, len,
				      WMI_P2P_LISTEN_OFFLOAD_STOP_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		wmi_err("Failed to send p2p lo stop: %d", status);
		wmi_buf_free(buf);
		return status;
	}

	wmi_debug("Successfully sent WMI_P2P_LO_STOP");

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_p2p_lo_stop_ev_param_tlv() - extract p2p lo stop
 * information from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold p2p lo stop event information
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_p2p_lo_stop_ev_param_tlv(
	wmi_unified_t wmi_handle, void *evt_buf,
	struct p2p_lo_event *param)
{
	WMI_P2P_LISTEN_OFFLOAD_STOPPED_EVENTID_param_tlvs *param_tlvs;
	wmi_p2p_lo_stopped_event_fixed_param *lo_param;

	param_tlvs = (WMI_P2P_LISTEN_OFFLOAD_STOPPED_EVENTID_param_tlvs *)
					evt_buf;
	if (!param_tlvs) {
		wmi_err("Invalid P2P lo stop event buffer");
		return QDF_STATUS_E_INVAL;
	}

	if (!param) {
		wmi_err("lo stop event param is null");
		return QDF_STATUS_E_INVAL;
	}

	lo_param = param_tlvs->fixed_param;
	param->vdev_id = lo_param->vdev_id;
	param->reason_code = lo_param->reason;
	wmi_debug("vdev_id:%d, reason:%d",
		 param->vdev_id, param->reason_code);

	return QDF_STATUS_SUCCESS;
}

void wmi_p2p_listen_offload_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_p2p_lo_start_cmd = send_p2p_lo_start_cmd_tlv;
	ops->send_p2p_lo_stop_cmd = send_p2p_lo_stop_cmd_tlv;
	ops->extract_p2p_lo_stop_ev_param =
			extract_p2p_lo_stop_ev_param_tlv;
}
#endif /* FEATURE_P2P_LISTEN_OFFLOAD */

void wmi_p2p_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_set_p2pgo_oppps_req_cmd = send_set_p2pgo_oppps_req_cmd_tlv;
	ops->send_set_p2pgo_noa_req_cmd = send_set_p2pgo_noa_req_cmd_tlv;
	ops->extract_p2p_noa_ev_param = extract_p2p_noa_ev_param_tlv;
	ops->set_mac_addr_rx_filter = send_set_mac_addr_rx_filter_cmd_tlv,
	ops->extract_mac_addr_rx_filter_evt_param =
				extract_mac_addr_rx_filter_evt_param_tlv,
	wmi_p2p_listen_offload_attach_tlv(wmi_handle);
}

