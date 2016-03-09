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
	if (ret < 0) {
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
	if (ret < 0) {
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
	return 0;
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
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold beacon send cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_beacon_send_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct beacon_params *param)
{
	return 0;
}

/**
 *  send_peer_assoc_cmd_tlv() - WMI peer assoc function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to peer assoc parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_peer_assoc_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct peer_assoc_params *param)
{
	return 0;
}

/**
 *  send_scan_start_cmd_tlv() - WMI scan start function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold scan start cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_scan_start_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct scan_start_params *param)
{
	return 0;
}

/**
 *  send_scan_stop_cmd_tlv() - WMI scan start function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold scan start cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_scan_stop_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct scan_stop_params *param)
{
	return 0;
}

/**
 *  send_scan_chan_list_cmd_tlv() - WMI scan channel list function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold scan channel list parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
int32_t send_scan_chan_list_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct scan_chan_list_params *param)
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

