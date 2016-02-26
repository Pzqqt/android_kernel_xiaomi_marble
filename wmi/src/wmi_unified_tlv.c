/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#include "wmi_unified_tlv.h"
#include "wmi_unified_api.h"
#include "wmi.h"
#include "wmi_unified_priv.h"
#include "wma_api.h"
#include "wma.h"

/**
 * send_vdev_create_cmd_tlv() - send VDEV create command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vdev create parameter
 * @macaddr: vdev mac address
 *
 * Return: 0 for success or error code
 */
int32_t send_vdev_create_cmd_tlv(wmi_unified_t wmi_handle,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param)
{
	wmi_vdev_create_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);
	int32_t ret;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGP("%s:wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_create_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_create_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_create_cmd_fixed_param));
	cmd->vdev_id = param->if_id;
	cmd->vdev_type = param->type;
	cmd->vdev_subtype = param->subtype;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->vdev_macaddr);
	WMA_LOGD("%s: ID = %d VAP Addr = %02x:%02x:%02x:%02x:%02x:%02x",
		 __func__, param->if_id,
		 macaddr[0], macaddr[1], macaddr[2],
		 macaddr[3], macaddr[4], macaddr[5]);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_VDEV_CREATE_CMDID);
	if (ret != EOK) {
		WMA_LOGE("Failed to send WMI_VDEV_CREATE_CMDID");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_vdev_delete_cmd_tlv() - send VDEV delete command to fw
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 *
 * Return: 0 for success or error code
 */
int32_t send_vdev_delete_cmd_tlv(wmi_unified_t wmi_handle,
					  uint8_t if_id)
{
	wmi_vdev_delete_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t ret;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMA_LOGP("%s:wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}

	cmd = (wmi_vdev_delete_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_delete_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_delete_cmd_fixed_param));
	cmd->vdev_id = if_id;
	ret = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(wmi_vdev_delete_cmd_fixed_param),
				   WMI_VDEV_DELETE_CMDID);
	if (ret != EOK) {
		WMA_LOGE("Failed to send WMI_VDEV_DELETE_CMDID");
		wmi_buf_free(buf);
	}
	WMA_LOGD("%s:vdev id = %d", __func__, if_id);

	return ret;
}

/**
 * send_vdev_stop_cmd_tlv() - send vdev stop command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or erro code
 */
int32_t send_vdev_stop_cmd_tlv(wmi_unified_t wmi,
					uint8_t vdev_id)
{
	wmi_vdev_stop_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s : wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_stop_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_stop_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_stop_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_STOP_CMDID)) {
		WMA_LOGP("%s: Failed to send vdev stop command", __func__);
		cdf_nbuf_free(buf);
		return -EIO;
	}

	return 0;
}

/**
 * send_vdev_down_cmd_tlv() - send vdev down command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
int32_t send_vdev_down_cmd_tlv(wmi_unified_t wmi, uint8_t vdev_id)
{
	wmi_vdev_down_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s : wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_down_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_down_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_down_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_DOWN_CMDID)) {
		WMA_LOGP("%s: Failed to send vdev down", __func__);
		cdf_nbuf_free(buf);
		return -EIO;
	}
	WMA_LOGD("%s: vdev_id %d", __func__, vdev_id);

	return 0;
}

/**
 * send_peer_flush_tids_cmd_tlv() - flush peer tids packets in fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to hold peer flush tid parameter
 *
 * Return: 0 for sucess or error code
 */
int32_t send_peer_flush_tids_cmd_tlv(wmi_unified_t wmi,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 struct peer_flush_params *param)
{
	wmi_peer_flush_tids_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_peer_flush_tids_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_flush_tids_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_flush_tids_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->peer_tid_bitmap = param->peer_tid_bitmap;
	cmd->vdev_id = param->vdev_id;
	WMA_LOGD("%s: peer_addr %pM vdev_id %d and peer bitmap %d", __func__,
				peer_addr, param->vdev_id,
				param->peer_tid_bitmap);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_FLUSH_TIDS_CMDID)) {
		WMA_LOGP("%s: Failed to send flush tid command", __func__);
		cdf_nbuf_free(buf);
		return -EIO;
	}

	return 0;
}

/**
 * send_peer_delete_cmd_tlv() - send PEER delete command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac addr
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
int32_t send_peer_delete_cmd_tlv(wmi_unified_t wmi,
				 uint8_t peer_addr[IEEE80211_ADDR_LEN],
				 uint8_t vdev_id)
{
	wmi_peer_delete_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_peer_delete_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_delete_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_delete_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->vdev_id = vdev_id;

	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_DELETE_CMDID)) {
		WMA_LOGP("%s: Failed to send peer delete command", __func__);
		cdf_nbuf_free(buf);
		return -EIO;
	}
	WMA_LOGD("%s: peer_addr %pM vdev_id %d", __func__, peer_addr, vdev_id);

	return 0;
}

/**
 * send_peer_param_cmd_tlv() - set peer parameter in fw
 * @wma_ctx: wmi handle
 * @peer_addr: peer mac address
 * @param    : pointer to hold peer set parameter
 *
 * Return: 0 for success or error code
 */
int32_t send_peer_param_cmd_tlv(wmi_unified_t wmi,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param)
{
	wmi_peer_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t err;

	buf = wmi_buf_alloc(wmi, sizeof(*cmd));
	if (!buf) {
		WMA_LOGE("Failed to allocate buffer to send set_param cmd");
		return -ENOMEM;
	}
	cmd = (wmi_peer_set_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
				(wmi_peer_set_param_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->param_id = param->param_id;
	cmd->param_value = param->param_value;
	err = wmi_unified_cmd_send(wmi, buf,
				   sizeof(wmi_peer_set_param_cmd_fixed_param),
				   WMI_PEER_SET_PARAM_CMDID);
	if (err) {
		WMA_LOGE("Failed to send set_param cmd");
		cdf_mem_free(buf);
		return -EIO;
	}

	return 0;
}

/**
 * send_vdev_up_cmd_tlv() - send vdev up command in fw
 * @wmi: wmi handle
 * @bssid: bssid
 * @vdev_up_params: pointer to hold vdev up parameter
 *
 * Return: 0 for success or error code
 */
int32_t send_vdev_up_cmd_tlv(wmi_unified_t wmi,
			     uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *params)
{
	wmi_vdev_up_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	WMA_LOGD("%s: VDEV_UP", __func__);
	WMA_LOGD("%s: vdev_id %d aid %d bssid %pM", __func__,
		 params->vdev_id, params->assoc_id, bssid);
	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_up_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_up_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_up_cmd_fixed_param));
	cmd->vdev_id = params->vdev_id;
	cmd->vdev_assoc_id = params->assoc_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(bssid, &cmd->vdev_bssid);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_UP_CMDID)) {
		WMA_LOGP("%s: Failed to send vdev up command", __func__);
		cdf_nbuf_free(buf);
		return -EIO;
	}

	return 0;
}

/**
 * send_peer_create_cmd_tlv() - send peer create command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @peer_type: peer type
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
int32_t send_peer_create_cmd_tlv(wmi_unified_t wmi,
					struct peer_create_params *param)
{
	wmi_peer_create_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_peer_create_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_create_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_create_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_addr, &cmd->peer_macaddr);
	cmd->peer_type = param->peer_type;
	cmd->vdev_id = param->vdev_id;

	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_CREATE_CMDID)) {
		WMA_LOGP("%s: failed to send WMI_PEER_CREATE_CMDID", __func__);
		cdf_nbuf_free(buf);
		return -EIO;
	}
	WMA_LOGD("%s: peer_addr %pM vdev_id %d", __func__, param->peer_addr,
			param->vdev_id);

	return 0;
}

/**
 * send_green_ap_ps_cmd_tlv() - enable green ap powersave command
 * @wmi_handle: wmi handle
 * @value: value
 * @mac_id: mac id to have radio context
 *
 * Return: 0 for success or error code
 */
int32_t send_green_ap_ps_cmd_tlv(wmi_unified_t wmi_handle,
						uint32_t value, uint8_t mac_id)
{
	wmi_pdev_green_ap_ps_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	WMA_LOGD("Set Green AP PS val %d", value);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGP("%s: Green AP PS Mem Alloc Failed", __func__);
		return -ENOMEM;
	}

	cmd = (wmi_pdev_green_ap_ps_enable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		   WMITLV_TAG_STRUC_wmi_pdev_green_ap_ps_enable_cmd_fixed_param,
		   WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_green_ap_ps_enable_cmd_fixed_param));
	cmd->reserved0 = 0;
	cmd->enable = value;

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_GREEN_AP_PS_ENABLE_CMDID)) {
		WMA_LOGE("Set Green AP PS param Failed val %d", value);
		cdf_nbuf_free(buf);
		return -EIO;
	}

	return 0;
}

/**
 * send_pdev_utf_cmd_tlv() - send utf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev_utf_params
 * @mac_id: mac id to have radio context
 *
 * Return: 0 for success or error code
 */
int32_t
send_pdev_utf_cmd_tlv(wmi_unified_t wmi_handle,
				struct pdev_utf_params *param,
				uint8_t mac_id)
{
	wmi_buf_t buf;
	uint8_t *cmd;
	int32_t ret = 0;
	static uint8_t msgref = 1;
	uint8_t segNumber = 0, segInfo, numSegments;
	uint16_t chunk_len, total_bytes;
	uint8_t *bufpos;
	struct seg_hdr_info segHdrInfo;

	bufpos = param->utf_payload;
	total_bytes = param->len;
	ASSERT(total_bytes / MAX_WMI_UTF_LEN ==
	       (uint8_t) (total_bytes / MAX_WMI_UTF_LEN));
	numSegments = (uint8_t) (total_bytes / MAX_WMI_UTF_LEN);

	if (param->len - (numSegments * MAX_WMI_UTF_LEN))
		numSegments++;

	while (param->len) {
		if (param->len > MAX_WMI_UTF_LEN)
			chunk_len = MAX_WMI_UTF_LEN;    /* MAX messsage */
		else
			chunk_len = param->len;

		buf = wmi_buf_alloc(wmi_handle,
				    (chunk_len + sizeof(segHdrInfo) +
				     WMI_TLV_HDR_SIZE));
		if (!buf) {
			WMA_LOGE("%s:wmi_buf_alloc failed", __func__);
			return -ENOMEM;
		}

		cmd = (uint8_t *) wmi_buf_data(buf);

		segHdrInfo.len = total_bytes;
		segHdrInfo.msgref = msgref;
		segInfo = ((numSegments << 4) & 0xF0) | (segNumber & 0xF);
		segHdrInfo.segmentInfo = segInfo;
		segHdrInfo.pad = 0;

		WMA_LOGD("%s:segHdrInfo.len = %d, segHdrInfo.msgref = %d,"
			 " segHdrInfo.segmentInfo = %d",
			 __func__, segHdrInfo.len, segHdrInfo.msgref,
			 segHdrInfo.segmentInfo);

		WMA_LOGD("%s:total_bytes %d segNumber %d totalSegments %d"
			 "chunk len %d", __func__, total_bytes, segNumber,
			 numSegments, chunk_len);

		segNumber++;

		WMITLV_SET_HDR(cmd, WMITLV_TAG_ARRAY_BYTE,
			       (chunk_len + sizeof(segHdrInfo)));
		cmd += WMI_TLV_HDR_SIZE;
		memcpy(cmd, &segHdrInfo, sizeof(segHdrInfo));   /* 4 bytes */
		memcpy(&cmd[sizeof(segHdrInfo)], bufpos, chunk_len);

		ret = wmi_unified_cmd_send(wmi_handle, buf,
					   (chunk_len + sizeof(segHdrInfo) +
					    WMI_TLV_HDR_SIZE),
					   WMI_PDEV_UTF_CMDID);

		if (ret != EOK) {
			WMA_LOGE("Failed to send WMI_PDEV_UTF_CMDID command");
			wmi_buf_free(buf);
			break;
		}

		param->len -= chunk_len;
		bufpos += chunk_len;
	}

	msgref++;

	return ret;
}

/**
 * send_pdev_param_cmd_tlv() - set pdev parameters
 * @wmi_handle: wmi handle
 * @param: pointer to pdev parameter
 * @mac_id: radio context
 *
 * Return: 0 on success, errno on failure
 */
int32_t
send_pdev_param_cmd_tlv(wmi_unified_t wmi_handle,
			   struct pdev_params *param,
				uint8_t mac_id)
{
	int32_t ret;
	wmi_pdev_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s:wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_pdev_set_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_set_param_cmd_fixed_param));
	cmd->reserved0 = 0;
	cmd->param_id = param->param_id;
	cmd->param_value = param->param_value;
	WMA_LOGD("Setting pdev param = %x, value = %u", param->param_id,
				param->param_value);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PDEV_SET_PARAM_CMDID);
	if (ret != EOK) {
		WMA_LOGE("Failed to send set param command ret = %d", ret);
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 * send_suspend_cmd_tlv() - WMI suspend function
 * @param wmi_handle      : handle to WMI.
 * @param param    : pointer to hold suspend parameter
 * @mac_id: radio context
 *
 * Return 0  on success and -ve on failure.
 */
int32_t send_suspend_cmd_tlv(wmi_unified_t wmi_handle,
				struct suspend_params *param,
				uint8_t mac_id)
{
	wmi_pdev_suspend_cmd_fixed_param *cmd;
	wmi_buf_t wmibuf;
	uint32_t len = sizeof(*cmd);
	int32_t ret;

	/*
	 * send the comand to Target to ignore the
	 * PCIE reset so as to ensure that Host and target
	 * states are in sync
	 */
	wmibuf = wmi_buf_alloc(wmi_handle, len);
	if (wmibuf == NULL)
		return -ENOMEM;

	cmd = (wmi_pdev_suspend_cmd_fixed_param *) wmi_buf_data(wmibuf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_suspend_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_suspend_cmd_fixed_param));
	if (param->disable_target_intr)
		cmd->suspend_opt = WMI_PDEV_SUSPEND_AND_DISABLE_INTR;
	else
		cmd->suspend_opt = WMI_PDEV_SUSPEND;
	ret = wmi_unified_cmd_send(wmi_handle, wmibuf, len,
				 WMI_PDEV_SUSPEND_CMDID);
	if (ret) {
		cdf_nbuf_free(wmibuf);
		WMA_LOGE("Failed to send WMI_PDEV_SUSPEND_CMDID command");
	}

	return ret;
}

/**
 * send_resume_cmd_tlv() - WMI resume function
 * @param wmi_handle      : handle to WMI.
 * @mac_id: radio context
 *
 * Return: 0  on success and -ve on failure.
 */
int32_t send_resume_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t mac_id)
{
	wmi_buf_t wmibuf;
	wmi_pdev_resume_cmd_fixed_param *cmd;
	int32_t ret;

	wmibuf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (wmibuf == NULL)
		return -ENOMEM;
	cmd = (wmi_pdev_resume_cmd_fixed_param *) wmi_buf_data(wmibuf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_resume_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_resume_cmd_fixed_param));
	cmd->reserved0 = 0;
	ret = wmi_unified_cmd_send(wmi_handle, wmibuf, sizeof(*cmd),
				   WMI_PDEV_RESUME_CMDID);
	if (ret != EOK) {
		WMA_LOGE("Failed to send WMI_PDEV_RESUME_CMDID command");
		wmi_buf_free(wmibuf);
	}

	return ret;
}

/**
 *  send_wow_enable_cmd_tlv() - WMI wow enable function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold wow enable parameter
 *  @mac_id: radio context
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_wow_enable_cmd_tlv(wmi_unified_t wmi_handle,
				struct wow_cmd_params *param,
				uint8_t mac_id)
{
	wmi_wow_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	int32_t ret;

	len = sizeof(wmi_wow_enable_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s: Failed allocate wmi buffer", __func__);
		return CDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_wow_enable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_wow_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_wow_enable_cmd_fixed_param));
	cmd->enable = param->enable;
	if (param->can_suspend_link)
		cmd->pause_iface_config = WOW_IFACE_PAUSE_ENABLED;
	else
		cmd->pause_iface_config = WOW_IFACE_PAUSE_DISABLED;

	WMA_LOGI("suspend type: %s",
		cmd->pause_iface_config == WOW_IFACE_PAUSE_ENABLED ?
		"WOW_IFACE_PAUSE_ENABLED" : "WOW_IFACE_PAUSE_DISABLED");

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WOW_ENABLE_CMDID);
	if (ret)
		wmi_buf_free(buf);

	return ret;
}

/**
 * send_set_ap_ps_param_cmd_tlv() - set ap powersave parameters
 * @wma_ctx: wma context
 * @peer_addr: peer mac address
 * @param: pointer to ap_ps parameter structure
 *
 * Return: 0 for success or error code
 */
int32_t send_set_ap_ps_param_cmd_tlv(wmi_unified_t wmi_handle,
					   uint8_t *peer_addr,
					   struct ap_ps_params *param)
{
	wmi_ap_ps_peer_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t err;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMA_LOGE("Failed to allocate buffer to send set_ap_ps_param cmd");
		return -ENOMEM;
	}
	cmd = (wmi_ap_ps_peer_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_ap_ps_peer_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_ap_ps_peer_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->param = param->param;
	cmd->value = param->value;
	err = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(*cmd), WMI_AP_PS_PEER_PARAM_CMDID);
	if (err) {
		WMA_LOGE("Failed to send set_ap_ps_param cmd");
		cdf_mem_free(buf);
		return -EIO;
	}

	return 0;
}

/**
 * send_set_sta_ps_param_cmd_tlv() - set sta powersave parameters
 * @wma_ctx: wma context
 * @peer_addr: peer mac address
 * @param: pointer to sta_ps parameter structure
 *
 * Return: 0 for success or error code
 */
int32_t send_set_sta_ps_param_cmd_tlv(wmi_unified_t wmi_handle,
					   struct sta_ps_params *param)
{
	wmi_sta_powersave_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGP("%s: Set Sta Ps param Mem Alloc Failed", __func__);
		return -ENOMEM;
	}

	cmd = (wmi_sta_powersave_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sta_powersave_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_sta_powersave_param_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->param = param->param;
	cmd->value = param->value;

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_STA_POWERSAVE_PARAM_CMDID)) {
		WMA_LOGE("Set Sta Ps param Failed vdevId %d Param %d val %d",
			 param->vdev_id, param->param, param->value);
		cdf_nbuf_free(buf);
		return -EIO;
	}

	return 0;
}

/**
 * send_crash_inject_cmd_tlv() - inject fw crash
 * @wma_handle: wma handle
 * @param: ponirt to crash inject paramter structure
 *
 * Return: 0 for success or return error
 */
int32_t send_crash_inject_cmd_tlv(wmi_unified_t wmi_handle,
			 struct crash_inject *param)
{
	int32_t ret = 0;
	WMI_FORCE_FW_HANG_CMD_fixed_param *cmd;
	uint16_t len = sizeof(*cmd);
	wmi_buf_t buf;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed!", __func__);
		return -ENOMEM;
	}

	cmd = (WMI_FORCE_FW_HANG_CMD_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_FORCE_FW_HANG_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_FORCE_FW_HANG_CMD_fixed_param));
	cmd->type = param->type;
	cmd->delay_time_ms = param->delay_time_ms;

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_FORCE_FW_HANG_CMDID);
	if (ret) {
		WMA_LOGE("%s: Failed to send set param command, ret = %d",
			 __func__, ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_dbglog_cmd_tlv() - set debug log level
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold dbglog level parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t
send_dbglog_cmd_tlv(wmi_unified_t wmi_handle,
				struct dbglog_params *dbglog_param)
{
	wmi_buf_t buf;
	wmi_debug_log_config_cmd_fixed_param *configmsg;
	A_STATUS status = A_OK;
	int32_t i;
	int32_t len;
	int8_t *buf_ptr;
	int32_t *module_id_bitmap_array;     /* Used to fomr the second tlv */

	ASSERT(bitmap_len < MAX_MODULE_ID_BITMAP_WORDS);

	/* Allocate size for 2 tlvs - including tlv hdr space for second tlv */
	len = sizeof(wmi_debug_log_config_cmd_fixed_param) + WMI_TLV_HDR_SIZE +
	      (sizeof(int32_t) * MAX_MODULE_ID_BITMAP_WORDS);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (buf == NULL)
		return A_NO_MEMORY;

	configmsg =
		(wmi_debug_log_config_cmd_fixed_param *) (wmi_buf_data(buf));
	buf_ptr = (int8_t *) configmsg;
	WMITLV_SET_HDR(&configmsg->tlv_header,
		       WMITLV_TAG_STRUC_wmi_debug_log_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_debug_log_config_cmd_fixed_param));
	configmsg->dbg_log_param = dbglog_param->param;
	configmsg->value = dbglog_param->val;
	/* Filling in the data part of second tlv -- should
	 * follow first tlv _ WMI_TLV_HDR_SIZE */
	module_id_bitmap_array = (A_UINT32 *) (buf_ptr +
				       sizeof
				       (wmi_debug_log_config_cmd_fixed_param)
				       + WMI_TLV_HDR_SIZE);
	WMITLV_SET_HDR(buf_ptr + sizeof(wmi_debug_log_config_cmd_fixed_param),
		       WMITLV_TAG_ARRAY_UINT32,
		       sizeof(A_UINT32) * MAX_MODULE_ID_BITMAP_WORDS);
	if (dbglog_param->module_id_bitmap) {
		for (i = 0; i < dbglog_param->bitmap_len; ++i) {
			module_id_bitmap_array[i] =
					dbglog_param->module_id_bitmap[i];
		}
	}

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_DBGLOG_CFG_CMDID);

	if (status != A_OK)
		cdf_nbuf_free(buf);

	return status;
}

/**
 *  send_vdev_set_param_cmd_tlv() - WMI vdev set parameter function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold vdev set parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_vdev_set_param_cmd_tlv(wmi_unified_t wmi_handle,
				struct vdev_set_params *param)
{
	int32_t ret;
	wmi_vdev_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s:wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_set_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_set_param_cmd_fixed_param));
	cmd->vdev_id = param->if_id;
	cmd->param_id = param->param_id;
	cmd->param_value = param->param_value;
	WMA_LOGD("Setting vdev %d param = %x, value = %u",
		 param->if_id, param->param_id, param->param_value);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SET_PARAM_CMDID);
	if (ret < 0) {
		WMA_LOGE("Failed to send set param command ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_stats_request_cmd_tlv() - WMI request stats function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold stats request parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_stats_request_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param)
{
	int32_t ret;
	wmi_request_stats_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed", __func__);
		return -CDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_request_stats_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));
	cmd->stats_id = param->stats_id;
	cmd->vdev_id = param->vdev_id;
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_REQUEST_STATS_CMDID);
	if (ret) {
		WMA_LOGE("Failed to send status request to fw =%d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_packet_log_enable_cmd_tlv() - WMI request stats function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold stats request parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_packet_log_enable_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct packet_enable_params *param)
{
	return 0;
}

/**
 *  send_beacon_send_cmd_tlv() - WMI beacon send function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold beacon send cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_beacon_send_cmd_tlv(wmi_unified_t wmi_handle,
				struct beacon_params *param)
{
	int32_t ret;
	wmi_bcn_tmpl_cmd_fixed_param *cmd;
	wmi_bcn_prb_info *bcn_prb_info;
	wmi_buf_t wmi_buf;
	uint8_t *buf_ptr;
	uint32_t wmi_buf_len;

	wmi_buf_len = sizeof(wmi_bcn_tmpl_cmd_fixed_param) +
		      sizeof(wmi_bcn_prb_info) + WMI_TLV_HDR_SIZE +
		      param->tmpl_len_aligned;
	wmi_buf = wmi_buf_alloc(wmi_handle, wmi_buf_len);
	if (!wmi_buf) {
		WMA_LOGE("%s : wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_bcn_tmpl_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_tmpl_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_bcn_tmpl_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->tim_ie_offset = param->tim_ie_offset;
	cmd->buf_len = param->tmpl_len;
	buf_ptr += sizeof(wmi_bcn_tmpl_cmd_fixed_param);

	bcn_prb_info = (wmi_bcn_prb_info *) buf_ptr;
	WMITLV_SET_HDR(&bcn_prb_info->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_prb_info,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_bcn_prb_info));
	bcn_prb_info->caps = 0;
	bcn_prb_info->erp = 0;
	buf_ptr += sizeof(wmi_bcn_prb_info);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, param->tmpl_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	cdf_mem_copy(buf_ptr, param->frm, param->tmpl_len);

	ret = wmi_unified_cmd_send(wmi_handle,
				   wmi_buf, wmi_buf_len, WMI_BCN_TMPL_CMDID);
	if (ret) {
		WMA_LOGE("%s: Failed to send bcn tmpl: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}
	return 0;
}

/**
 *  send_peer_assoc_cmd_tlv() - WMI peer assoc function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to peer assoc parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_peer_assoc_cmd_tlv(wmi_unified_t wmi_handle,
				struct peer_assoc_params *param)
{
	wmi_peer_assoc_complete_cmd_fixed_param *cmd;
	wmi_vht_rate_set *mcs;
	wmi_buf_t buf;
	int32_t len;
	uint8_t *buf_ptr;
	int ret;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE +
	      (param->num_peer_legacy_rates * sizeof(uint8_t)) +
	      WMI_TLV_HDR_SIZE +
	      (param->num_peer_ht_rates * sizeof(uint8_t)) +
	      sizeof(wmi_vht_rate_set);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_peer_assoc_complete_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_assoc_complete_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_assoc_complete_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->peer_new_assoc = param->peer_new_assoc;
	cmd->peer_associd = param->peer_associd;
	cmd->peer_flags = param->peer_flags;
	cmd->peer_rate_caps = param->peer_rate_caps;
	cmd->peer_caps = param->peer_caps;
	cmd->peer_listen_intval = param->peer_listen_intval;
	cmd->peer_ht_caps = param->peer_ht_caps;
	cmd->peer_max_mpdu = param->peer_max_mpdu;
	cmd->peer_mpdu_density = param->peer_mpdu_density;
	cmd->num_peer_legacy_rates = param->num_peer_legacy_rates;
	cmd->num_peer_ht_rates = param->num_peer_ht_rates;
	cmd->peer_vht_caps = param->peer_vht_caps;
	cmd->peer_phymode = param->peer_phymode;

	/* Update peer legacy rate information */
	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
				   param->num_peer_legacy_rates);
	buf_ptr += WMI_TLV_HDR_SIZE;
	cmd->num_peer_legacy_rates = param->num_peer_legacy_rates;
	cdf_mem_copy(buf_ptr, param->peer_legacy_rates.rates,
		     param->peer_legacy_rates.num_rates);

	/* Update peer HT rate information */
	buf_ptr += param->num_peer_legacy_rates;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
			  param->num_peer_ht_rates);
	buf_ptr += WMI_TLV_HDR_SIZE;
	cmd->num_peer_ht_rates = param->num_peer_ht_rates;
	cdf_mem_copy(buf_ptr, param->peer_ht_rates.rates,
				 param->peer_ht_rates.num_rates);

	/* VHT Rates */
	buf_ptr += param->num_peer_ht_rates;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_STRUC_wmi_vht_rate_set,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vht_rate_set));

	cmd->peer_nss = param->peer_nss;
	mcs = (wmi_vht_rate_set *) buf_ptr;
	if (param->vht_capable) {
		mcs->rx_max_rate = param->rx_max_rate;
		mcs->rx_mcs_set = param->rx_mcs_set;
		mcs->tx_max_rate = param->tx_max_rate;
		mcs->tx_mcs_set = param->tx_mcs_set;
	}

	WMA_LOGD("%s: vdev_id %d associd %d peer_flags %x rate_caps %x "
		 "peer_caps %x listen_intval %d ht_caps %x max_mpdu %d "
		 "nss %d phymode %d peer_mpdu_density %d "
		 "cmd->peer_vht_caps %x", __func__,
		 cmd->vdev_id, cmd->peer_associd, cmd->peer_flags,
		 cmd->peer_rate_caps, cmd->peer_caps,
		 cmd->peer_listen_intval, cmd->peer_ht_caps,
		 cmd->peer_max_mpdu, cmd->peer_nss, cmd->peer_phymode,
		 cmd->peer_mpdu_density,
		 cmd->peer_vht_caps);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PEER_ASSOC_CMDID);
	if (ret != EOK) {
		WMA_LOGP("%s: Failed to send peer assoc command ret = %d",
			 __func__, ret);
		cdf_nbuf_free(buf);
	}

	return ret;
}

/**
 *  send_scan_start_cmd_tlv() - WMI scan start function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold scan start cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_scan_start_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_start_params *params)
{
	int32_t ret = 0;
	int32_t i;
	wmi_buf_t wmi_buf;
	wmi_start_scan_cmd_fixed_param *cmd;
	uint8_t *buf_ptr;
	uint32_t *tmp_ptr;
	wmi_ssid *ssid = NULL;
	wmi_mac_addr *bssid;
	int len = sizeof(*cmd);

	/* Length TLV placeholder for array of uint32_t */
	len += WMI_TLV_HDR_SIZE;
	/* calculate the length of buffer required */
	if (params->num_chan)
		len += params->num_chan * sizeof(uint32_t);

	/* Length TLV placeholder for array of wmi_ssid structures */
	len += WMI_TLV_HDR_SIZE;
	if (params->num_ssids)
		len += params->num_ssids * sizeof(wmi_ssid);

	/* Length TLV placeholder for array of wmi_mac_addr structures */
	len += WMI_TLV_HDR_SIZE;
	len += sizeof(wmi_mac_addr);

	/* Length TLV placeholder for array of bytes */
	len += WMI_TLV_HDR_SIZE;
	if (params->ie_len)
		len += roundup(params->ie_len, sizeof(uint32_t));

	/* Allocate the memory */
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMA_LOGP("%s: failed to allocate memory for start scan cmd",
			 __func__);
		return CDF_STATUS_E_FAILURE;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_start_scan_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_start_scan_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_start_scan_cmd_fixed_param));

	cmd->scan_id = params->scan_id;
	cmd->scan_req_id = params->scan_req_id;
	cmd->vdev_id = params->vdev_id;
	cmd->scan_priority = params->scan_priority;
	cmd->notify_scan_events = params->notify_scan_events;
	cmd->dwell_time_active = params->dwell_time_active;
	cmd->dwell_time_passive = params->dwell_time_passive;
	cmd->min_rest_time = params->min_rest_time;
	cmd->max_rest_time = params->max_rest_time;
	cmd->repeat_probe_time = params->repeat_probe_time;
	cmd->probe_spacing_time = params->probe_spacing_time;
	cmd->idle_time = params->idle_time;
	cmd->max_scan_time = params->max_scan_time;
	cmd->probe_delay = params->probe_delay;
	cmd->scan_ctrl_flags = params->scan_ctrl_flags;
	cmd->burst_duration = params->burst_duration;
	cmd->num_chan = params->num_chan;
	cmd->num_bssid = params->num_bssid;
	cmd->num_ssids = params->num_ssids;
	cmd->ie_len = params->ie_len;
	cmd->n_probes = params->n_probes;
	buf_ptr += sizeof(*cmd);
	tmp_ptr = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);
	for (i = 0; i < params->num_chan; ++i)
		tmp_ptr[i] = params->chan_list[i];

	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_UINT32,
		       (params->num_chan * sizeof(uint32_t)));
	buf_ptr += WMI_TLV_HDR_SIZE + (params->num_chan * sizeof(uint32_t));
	if (params->num_ssids > SIR_SCAN_MAX_NUM_SSID) {
		WMA_LOGE("Invalid value for numSsid");
		goto error;
	}

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
	       (params->num_ssids * sizeof(wmi_ssid)));

	if (params->num_ssids) {
		ssid = (wmi_ssid *) (buf_ptr + WMI_TLV_HDR_SIZE);
		for (i = 0; i < params->num_ssids; ++i) {
			ssid->ssid_len = params->ssid[i].length;
			cdf_mem_copy(ssid->ssid, params->ssid[i].mac_ssid,
				     params->ssid[i].length);
			ssid++;
		}
	}
	buf_ptr += WMI_TLV_HDR_SIZE + (params->num_ssids * sizeof(wmi_ssid));

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
		       (params->num_bssid * sizeof(wmi_mac_addr)));
	bssid = (wmi_mac_addr *) (buf_ptr + WMI_TLV_HDR_SIZE);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(params->mac_add_bytes, bssid);
	buf_ptr += WMI_TLV_HDR_SIZE + (params->num_bssid * sizeof(wmi_mac_addr));

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, params->ie_len_with_pad);
	if (params->ie_len) {
		cdf_mem_copy(buf_ptr + WMI_TLV_HDR_SIZE,
			     (uint8_t *) params->ie_base +
			     (params->uie_fieldOffset), params->ie_len);
	}
	buf_ptr += WMI_TLV_HDR_SIZE + params->ie_len_with_pad;

	ret = wmi_unified_cmd_send(wmi_handle, wmi_buf,
				      len, WMI_START_SCAN_CMDID);
	if (ret) {
		WMA_LOGE("%s: Failed to start scan: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}
	return ret;
error:
	cdf_nbuf_free(wmi_buf);
	return CDF_STATUS_E_FAILURE;
}

/**
 *  send_scan_stop_cmd_tlv() - WMI scan start function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold scan start cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_scan_stop_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_stop_params *param)
{
	wmi_stop_scan_cmd_fixed_param *cmd;
	int ret;
	int len = sizeof(*cmd);
	wmi_buf_t wmi_buf;

	/* Allocate the memory */
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMA_LOGP("%s: failed to allocate memory for stop scan cmd",
			 __func__);
		ret = -ENOMEM;
		goto error;
	}

	cmd = (wmi_stop_scan_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_stop_scan_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_stop_scan_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->requestor = param->requestor;
	cmd->scan_id = param->scan_id;
	/* stop the scan with the corresponding scan_id */
	cmd->req_type = param->req_type;
	ret = wmi_unified_cmd_send(wmi_handle, wmi_buf,
				      len, WMI_STOP_SCAN_CMDID);
	if (ret) {
		WMA_LOGE("%s: Failed to send stop scan: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}

error:
	return ret;
}

/**
 *  send_scan_chan_list_cmd_tlv() - WMI scan channel list function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold scan channel list parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_scan_chan_list_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_chan_list_params *chan_list)
{
	wmi_buf_t buf;
	CDF_STATUS cdf_status = CDF_STATUS_SUCCESS;
	wmi_scan_chan_list_cmd_fixed_param *cmd;
	int status, i;
	uint8_t *buf_ptr;
	wmi_channel *chan_info, *tchan_info;
	uint16_t len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;

	len += sizeof(wmi_channel) * chan_list->num_scan_chans;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGE("Failed to allocate memory");
		cdf_status = CDF_STATUS_E_NOMEM;
		goto end;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_scan_chan_list_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_scan_chan_list_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_scan_chan_list_cmd_fixed_param));

	WMA_LOGD("no of channels = %d, len = %d", chan_list->num_scan_chans, len);

	cmd->num_scan_chans = chan_list->num_scan_chans;
	WMITLV_SET_HDR((buf_ptr + sizeof(wmi_scan_chan_list_cmd_fixed_param)),
		       WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_channel) * chan_list->num_scan_chans);
	chan_info = (wmi_channel *) (buf_ptr + sizeof(*cmd) + WMI_TLV_HDR_SIZE);
	tchan_info = chan_list->chan_info;

	for (i = 0; i < chan_list->num_scan_chans; ++i) {
		WMITLV_SET_HDR(&chan_info->tlv_header,
			       WMITLV_TAG_STRUC_wmi_channel,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
		chan_info->mhz = tchan_info->mhz;
		chan_info->band_center_freq1 =
				 tchan_info->band_center_freq1;
		chan_info->band_center_freq2 =
				tchan_info->band_center_freq2;
		chan_info->info = tchan_info->info;
		chan_info->reg_info_1 = tchan_info->reg_info_1;
		chan_info->reg_info_2 = tchan_info->reg_info_2;
		WMA_LOGD("chan[%d] = %u", i, chan_info->mhz);

		/*TODO: Set WMI_SET_CHANNEL_MIN_POWER */
		/*TODO: Set WMI_SET_CHANNEL_ANTENNA_MAX */
		/*TODO: WMI_SET_CHANNEL_REG_CLASSID */
		tchan_info++;
		chan_info++;
	}

	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_SCAN_CHAN_LIST_CMDID);

	if (status != EOK) {
		cdf_status = CDF_STATUS_E_FAILURE;
		WMA_LOGE("Failed to send WMI_SCAN_CHAN_LIST_CMDID");
		wmi_buf_free(buf);
	}
end:
	return cdf_status;
}

/**
 *  send_mgmt_cmd_tlv() - WMI scan start function
 *  @wmi_handle      : handle to WMI.
 *  @param    : pointer to hold mgmt cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_mgmt_cmd_tlv(wmi_unified_t wmi_handle,
				struct wmi_mgmt_params *param)
{
	return 0;
}

struct wmi_ops tlv_ops =  {
	.send_vdev_create_cmd = send_vdev_create_cmd_tlv,
	.send_vdev_delete_cmd = send_vdev_delete_cmd_tlv,
	.send_vdev_down_cmd = send_vdev_down_cmd_tlv,
	.send_peer_flush_tids_cmd = send_peer_flush_tids_cmd_tlv,
	.send_peer_param_cmd = send_peer_param_cmd_tlv,
	.send_vdev_up_cmd = send_vdev_up_cmd_tlv,
	.send_peer_create_cmd = send_peer_create_cmd_tlv,
	.send_green_ap_ps_cmd = send_green_ap_ps_cmd_tlv,
	.send_pdev_utf_cmd = send_pdev_utf_cmd_tlv,
	.send_pdev_param_cmd = send_pdev_param_cmd_tlv,
	.send_suspend_cmd = send_suspend_cmd_tlv,
	.send_resume_cmd = send_resume_cmd_tlv,
	.send_wow_enable_cmd = send_wow_enable_cmd_tlv,
	.send_set_ap_ps_param_cmd = send_set_ap_ps_param_cmd_tlv,
	.send_set_sta_ps_param_cmd = send_set_sta_ps_param_cmd_tlv,
	.send_crash_inject_cmd = send_crash_inject_cmd_tlv,
	.send_dbglog_cmd = send_dbglog_cmd_tlv,
	.send_vdev_set_param_cmd = send_vdev_set_param_cmd_tlv,
	.send_stats_request_cmd = send_stats_request_cmd_tlv,
	.send_packet_log_enable_cmd = send_packet_log_enable_cmd_tlv,
	.send_beacon_send_cmd = send_beacon_send_cmd_tlv,
	.send_peer_assoc_cmd = send_peer_assoc_cmd_tlv,
	.send_scan_start_cmd = send_scan_start_cmd_tlv,
	.send_scan_stop_cmd = send_scan_stop_cmd_tlv,
	.send_scan_chan_list_cmd = send_scan_chan_list_cmd_tlv,

	/* TODO - Add other tlv apis here */
};

/**
 * wmi_get_tlv_ops() - gives pointer to wmi tlv ops
 *
 * Return: pointer to wmi tlv ops
 */
struct wmi_ops *wmi_get_tlv_ops(void)
{
	return &tlv_ops;
}

