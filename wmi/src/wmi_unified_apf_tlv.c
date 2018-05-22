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

#include "wmi_unified_apf_tlv.h"

QDF_STATUS send_set_active_apf_mode_cmd_tlv(wmi_unified_t wmi_handle,
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

	WMI_LOGD("Sending WMI_APF_SET_VDEV_ACTIVE_MODE_CMDID(%u, %d, %d)",
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
		WMI_LOGE("Failed to send WMI_APF_SET_VDEV_ACTIVE_MODE_CMDID:%d",
			 status);
		wmi_buf_free(buf);
		return status;
	}

	WMI_LOGD("Sent WMI_APF_SET_VDEV_ACTIVE_MODE_CMDID successfully");

	return QDF_STATUS_SUCCESS;
}

