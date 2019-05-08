/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#include "wmi_unified_apf_tlv.h"

QDF_STATUS wmi_send_set_active_apf_mode_cmd_tlv(wmi_unified_t wmi_handle,
					    uint8_t vdev_id,
					    enum wmi_host_active_apf_mode
								     ucast_mode,
					    enum wmi_host_active_apf_mode
							       mcast_bcast_mode)
{
	const WMITLV_TAG_ID tag_id =
		WMITLV_TAG_STRUC_wmi_bpf_set_vdev_active_mode_cmd_fixed_param;
	const uint32_t tlv_len = WMITLV_GET_STRUCT_TLVLEN(
				wmi_bpf_set_vdev_active_mode_cmd_fixed_param);
	QDF_STATUS status;
	wmi_bpf_set_vdev_active_mode_cmd_fixed_param *cmd;
	wmi_buf_t buf;

	WMI_LOGD("Sending WMI_BPF_SET_VDEV_ACTIVE_MODE_CMDID(%u, %d, %d)",
		 vdev_id, ucast_mode, mcast_bcast_mode);

	/* allocate command buffer */
	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	/* set TLV header */
	cmd = (wmi_bpf_set_vdev_active_mode_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header, tag_id, tlv_len);

	/* populate data */
	cmd->vdev_id = vdev_id;
	cmd->uc_mode = ucast_mode;
	cmd->mcbc_mode = mcast_bcast_mode;

	/* send to FW */
	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_BPF_SET_VDEV_ACTIVE_MODE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("Failed to send WMI_BPF_SET_VDEV_ACTIVE_MODE_CMDID:%d",
			 status);
		wmi_buf_free(buf);
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wmi_send_apf_enable_cmd_tlv(wmi_unified_t wmi_handle,
				       uint32_t vdev_id,
				       bool enable)
{
	wmi_bpf_set_vdev_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_bpf_set_vdev_enable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_bpf_set_vdev_enable_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_bpf_set_vdev_enable_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->is_enabled = enable;

	if (wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				 WMI_BPF_SET_VDEV_ENABLE_CMDID)) {
		WMI_LOGE("%s: Failed to enable/disable APF interpreter",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wmi_send_apf_write_work_memory_cmd_tlv(wmi_unified_t wmi_handle,
				       struct wmi_apf_write_memory_params
							      *apf_write_params)
{
	wmi_bpf_set_vdev_work_memory_cmd_fixed_param *cmd;
	uint32_t wmi_buf_len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t aligned_len = 0;

	wmi_buf_len = sizeof(*cmd);
	if (apf_write_params->length) {
		aligned_len = roundup(apf_write_params->length,
				      sizeof(A_UINT32));

		wmi_buf_len += WMI_TLV_HDR_SIZE + aligned_len;

	}

	buf = wmi_buf_alloc(wmi_handle, wmi_buf_len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	cmd = (wmi_bpf_set_vdev_work_memory_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_bpf_set_vdev_work_memory_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_bpf_set_vdev_work_memory_cmd_fixed_param));
	cmd->vdev_id = apf_write_params->vdev_id;
	cmd->bpf_version = apf_write_params->apf_version;
	cmd->program_len = apf_write_params->program_len;
	cmd->addr_offset = apf_write_params->addr_offset;
	cmd->length = apf_write_params->length;

	if (apf_write_params->length) {
		buf_ptr += sizeof(*cmd);
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
			       aligned_len);
		buf_ptr += WMI_TLV_HDR_SIZE;
		qdf_mem_copy(buf_ptr, apf_write_params->buf,
			     apf_write_params->length);
	}

	if (wmi_unified_cmd_send(wmi_handle, buf, wmi_buf_len,
				 WMI_BPF_SET_VDEV_WORK_MEMORY_CMDID)) {
		WMI_LOGE("%s: Failed to write APF work memory", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wmi_send_apf_read_work_memory_cmd_tlv(wmi_unified_t wmi_handle,
				      struct wmi_apf_read_memory_params
							       *apf_read_params)
{
	wmi_bpf_get_vdev_work_memory_cmd_fixed_param *cmd;
	wmi_buf_t buf;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_bpf_get_vdev_work_memory_cmd_fixed_param *)
							wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_bpf_get_vdev_work_memory_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_bpf_get_vdev_work_memory_cmd_fixed_param));
	cmd->vdev_id = apf_read_params->vdev_id;
	cmd->addr_offset = apf_read_params->addr_offset;
	cmd->length = apf_read_params->length;

	if (wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				 WMI_BPF_GET_VDEV_WORK_MEMORY_CMDID)) {
		WMI_LOGE("%s: Failed to get APF work memory", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wmi_extract_apf_read_memory_resp_event_tlv(wmi_unified_t wmi_handle,
				void *evt_buf,
				struct wmi_apf_read_memory_resp_event_params
									  *resp)
{
	WMI_BPF_GET_VDEV_WORK_MEMORY_RESP_EVENTID_param_tlvs *param_buf;
	wmi_bpf_get_vdev_work_memory_resp_evt_fixed_param *data_event;

	param_buf = evt_buf;
	if (!param_buf) {
		WMI_LOGE("encrypt decrypt resp evt_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	data_event = param_buf->fixed_param;

	resp->vdev_id = data_event->vdev_id;
	resp->offset = data_event->offset;
	resp->more_data = data_event->fragment;

	if (data_event->length > param_buf->num_data) {
		WMI_LOGE("FW msg data_len %d more than TLV hdr %d",
			 data_event->length,
			 param_buf->num_data);
		return QDF_STATUS_E_INVAL;
	}

	if (data_event->length && param_buf->data) {
		resp->length = data_event->length;
		resp->data = (uint8_t *)param_buf->data;
	}

	return QDF_STATUS_SUCCESS;
}
