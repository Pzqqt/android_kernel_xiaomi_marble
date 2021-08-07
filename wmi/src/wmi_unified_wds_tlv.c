/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
#include <wmi_unified_wds_api.h>

/**
 * send_peer_add_wds_entry_cmd_tlv() - send peer add command to fw
 * @wmi_handle: wmi handle
 * @param: pointer holding peer details
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
send_peer_add_wds_entry_cmd_tlv(wmi_unified_t wmi_handle,
				struct peer_add_wds_entry_params *param)
{
	wmi_peer_add_wds_entry_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int len = sizeof(*cmd);
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_peer_add_wds_entry_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_add_wds_entry_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_peer_add_wds_entry_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->dest_addr, &cmd->wds_macaddr);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_addr, &cmd->peer_macaddr);

	cmd->flags = (param->flags & WMI_HOST_WDS_FLAG_STATIC) ?
					WMI_WDS_FLAG_STATIC : 0;
	cmd->vdev_id = param->vdev_id;

	wmi_mtrace(WMI_PEER_ADD_WDS_ENTRY_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PEER_ADD_WDS_ENTRY_CMDID);
	if (QDF_IS_STATUS_ERROR(ret))
		wmi_buf_free(buf);

	return ret;
}

/**
 * send_peer_del_wds_entry_cmd_tlv() - send peer delete command to fw
 * @wmi_handle: wmi handle
 * @param: pointer holding peer details
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
send_peer_del_wds_entry_cmd_tlv(wmi_unified_t wmi_handle,
				struct peer_del_wds_entry_params *param)
{
	wmi_peer_remove_wds_entry_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int len = sizeof(*cmd);
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_peer_remove_wds_entry_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_remove_wds_entry_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_peer_remove_wds_entry_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->dest_addr, &cmd->wds_macaddr);

	cmd->vdev_id = param->vdev_id;

	wmi_mtrace(WMI_PEER_REMOVE_WDS_ENTRY_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PEER_REMOVE_WDS_ENTRY_CMDID);
	if (QDF_IS_STATUS_ERROR(ret))
		wmi_buf_free(buf);

	return ret;
}

/**
 * send_peer_update_wds_entry_cmd_tlv() - send peer update command to fw
 * @wmi_handle: wmi handle
 * @param: pointer holding peer details
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
send_peer_update_wds_entry_cmd_tlv(wmi_unified_t wmi_handle,
				   struct peer_update_wds_entry_params *param)
{
	wmi_peer_update_wds_entry_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int len = sizeof(*cmd);
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_peer_update_wds_entry_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_update_wds_entry_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_peer_update_wds_entry_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->dest_addr, &cmd->wds_macaddr);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_addr, &cmd->peer_macaddr);

	cmd->flags = (param->flags & WMI_HOST_WDS_FLAG_STATIC) ?
						WMI_WDS_FLAG_STATIC : 0;
	cmd->vdev_id = param->vdev_id;

	wmi_mtrace(WMI_PEER_UPDATE_WDS_ENTRY_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PEER_UPDATE_WDS_ENTRY_CMDID);
	if (QDF_IS_STATUS_ERROR(ret))
		wmi_buf_free(buf);

	return ret;
}

void wmi_wds_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_peer_add_wds_entry_cmd = send_peer_add_wds_entry_cmd_tlv;
	ops->send_peer_del_wds_entry_cmd = send_peer_del_wds_entry_cmd_tlv;
	ops->send_peer_update_wds_entry_cmd =
		send_peer_update_wds_entry_cmd_tlv;
}
