
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
#include <osdep.h>
#include <wmi.h>
#include <wmi_unified_priv.h>
#include "wmi_unified_bcn_api.h"

/**
 * send_bcn_buf_ll_cmd_tlv() - prepare and send beacon buffer to fw for LL
 * @wmi_handle: wmi handle
 * @param: bcn ll cmd parameter
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */
static QDF_STATUS send_bcn_buf_ll_cmd_tlv(
		wmi_unified_t wmi_handle,
		wmi_bcn_send_from_host_cmd_fixed_param *param)
{
	wmi_bcn_send_from_host_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	QDF_STATUS ret;

	wmi_buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!wmi_buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_bcn_send_from_host_cmd_fixed_param *)wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_send_from_host_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_bcn_send_from_host_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->data_len = param->data_len;
	cmd->frame_ctrl = param->frame_ctrl;
	cmd->frag_ptr = param->frag_ptr;
	cmd->dtim_flag = param->dtim_flag;

	wmi_mtrace(WMI_PDEV_SEND_BCN_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, wmi_buf, sizeof(*cmd),
				   WMI_PDEV_SEND_BCN_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_PDEV_SEND_BCN_CMDID command");
		wmi_buf_free(wmi_buf);
	}

	return ret;
}

void wmi_bcn_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_bcn_buf_ll_cmd = send_bcn_buf_ll_cmd_tlv;
}
