/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#include "wmi_unified_api.h"
#include "wmi.h"
#include "wmi_version.h"
#include "wmi_unified_priv.h"
#include "wmi_version_whitelist.h"
#ifdef CONVERGED_P2P_ENABLE
#include "wlan_p2p_public_struct.h"
#endif
#ifdef WLAN_PMO_ENABLE
#include "wlan_pmo_hw_filter_public_struct.h"
#endif
#include <wlan_utility.h>

/* copy_vdev_create_pdev_id() - copy pdev from host params to target command
 *                              buffer.
 * @wmi_handle: pointer to wmi_handle
 * @cmd: pointer target vdev create command buffer
 * @param: pointer host params for vdev create
 *
 * Return: None
 */
#ifdef CONFIG_MCL
static inline void copy_vdev_create_pdev_id(
		struct wmi_unified *wmi_handle,
		wmi_vdev_create_cmd_fixed_param * cmd,
		struct vdev_create_params *param)
{
	cmd->pdev_id = WMI_PDEV_ID_SOC;
}
#else
static inline void copy_vdev_create_pdev_id(
		struct wmi_unified *wmi_handle,
		wmi_vdev_create_cmd_fixed_param * cmd,
		struct vdev_create_params *param)
{
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							param->pdev_id);
}
#endif

/**
 * send_vdev_create_cmd_tlv() - send VDEV create command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vdev create parameter
 * @macaddr: vdev mac address
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_vdev_create_cmd_tlv(wmi_unified_t wmi_handle,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param)
{
	wmi_vdev_create_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);
	QDF_STATUS ret;
	int num_bands = 2;
	uint8_t *buf_ptr;
	wmi_vdev_txrx_streams *txrx_streams;

	len += (num_bands * sizeof(*txrx_streams) + WMI_TLV_HDR_SIZE);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_create_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_create_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_create_cmd_fixed_param));
	cmd->vdev_id = param->if_id;
	cmd->vdev_type = param->type;
	cmd->vdev_subtype = param->subtype;
	cmd->num_cfg_txrx_streams = num_bands;
	copy_vdev_create_pdev_id(wmi_handle, cmd, param);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->vdev_macaddr);
	WMI_LOGD("%s: ID = %d VAP Addr = %02x:%02x:%02x:%02x:%02x:%02x",
		 __func__, param->if_id,
		 macaddr[0], macaddr[1], macaddr[2],
		 macaddr[3], macaddr[4], macaddr[5]);
	buf_ptr = (uint8_t *)cmd + sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			(num_bands * sizeof(wmi_vdev_txrx_streams)));
	buf_ptr += WMI_TLV_HDR_SIZE;

	WMI_LOGD("%s: type %d, subtype %d, nss_2g %d, nss_5g %d", __func__,
			param->type, param->subtype,
			param->nss_2g, param->nss_5g);
	txrx_streams = (wmi_vdev_txrx_streams *)buf_ptr;
	txrx_streams->band = WMI_TPC_CHAINMASK_CONFIG_BAND_2G;
	txrx_streams->supported_tx_streams = param->nss_2g;
	txrx_streams->supported_rx_streams = param->nss_2g;
	WMITLV_SET_HDR(&txrx_streams->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_txrx_streams,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_txrx_streams));

	txrx_streams++;
	txrx_streams->band = WMI_TPC_CHAINMASK_CONFIG_BAND_5G;
	txrx_streams->supported_tx_streams = param->nss_5g;
	txrx_streams->supported_rx_streams = param->nss_5g;
	WMITLV_SET_HDR(&txrx_streams->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_txrx_streams,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_txrx_streams));
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_VDEV_CREATE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_VDEV_CREATE_CMDID");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_vdev_delete_cmd_tlv() - send VDEV delete command to fw
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_vdev_delete_cmd_tlv(wmi_unified_t wmi_handle,
					  uint8_t if_id)
{
	wmi_vdev_delete_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGP("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
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
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_VDEV_DELETE_CMDID");
		wmi_buf_free(buf);
	}
	WMI_LOGD("%s:vdev id = %d", __func__, if_id);

	return ret;
}

/**
 * send_vdev_stop_cmd_tlv() - send vdev stop command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or erro code
 */
static QDF_STATUS send_vdev_stop_cmd_tlv(wmi_unified_t wmi,
					uint8_t vdev_id)
{
	wmi_vdev_stop_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMI_LOGP("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_stop_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_stop_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_stop_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_STOP_CMDID)) {
		WMI_LOGP("%s: Failed to send vdev stop command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("%s:vdev id = %d", __func__, vdev_id);

	return 0;
}

/**
 * send_vdev_down_cmd_tlv() - send vdev down command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_vdev_down_cmd_tlv(wmi_unified_t wmi, uint8_t vdev_id)
{
	wmi_vdev_down_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMI_LOGP("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_down_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_down_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_down_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_DOWN_CMDID)) {
		WMI_LOGP("%s: Failed to send vdev down", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("%s: vdev_id %d", __func__, vdev_id);

	return 0;
}

#ifdef CONFIG_MCL
static inline void copy_channel_info(
		wmi_vdev_start_request_cmd_fixed_param * cmd,
		wmi_channel *chan,
		struct vdev_start_params *req)
{
	chan->mhz = req->chan_freq;

	WMI_SET_CHANNEL_MODE(chan, req->chan_mode);

	chan->band_center_freq1 = req->band_center_freq1;
	chan->band_center_freq2 = req->band_center_freq2;

	if (req->is_half_rate)
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_HALF_RATE);
	else if (req->is_quarter_rate)
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_QUARTER_RATE);

	if (req->is_dfs && req->flag_dfs) {
		WMI_SET_CHANNEL_FLAG(chan, req->flag_dfs);
		cmd->disable_hw_ack = req->dis_hw_ack;
	}

	WMI_SET_CHANNEL_REG_POWER(chan, req->max_txpow);
	WMI_SET_CHANNEL_MAX_TX_POWER(chan, req->max_txpow);

}
#else
static inline void copy_channel_info(
		wmi_vdev_start_request_cmd_fixed_param * cmd,
		wmi_channel *chan,
		struct vdev_start_params *req)
{
	chan->mhz = req->channel.mhz;

	WMI_SET_CHANNEL_MODE(chan, req->channel.phy_mode);

	chan->band_center_freq1 = req->channel.cfreq1;
	chan->band_center_freq2 = req->channel.cfreq2;

	if (req->channel.half_rate)
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_HALF_RATE);
	else if (req->channel.quarter_rate)
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_QUARTER_RATE);

	if (req->channel.dfs_set) {
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_DFS);
		cmd->disable_hw_ack = req->disable_hw_ack;
	}

	if (req->channel.dfs_set_cfreq2)
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_DFS_CFREQ2);

	/* FIXME: Find out min, max and regulatory power levels */
	WMI_SET_CHANNEL_REG_POWER(chan, req->channel.maxregpower);
	WMI_SET_CHANNEL_MAX_TX_POWER(chan, req->channel.maxpower);

}
#endif
/**
 * send_vdev_start_cmd_tlv() - send vdev start request to fw
 * @wmi_handle: wmi handle
 * @req: vdev start params
 *
 * Return: QDF status
 */
static QDF_STATUS send_vdev_start_cmd_tlv(wmi_unified_t wmi_handle,
			  struct vdev_start_params *req)
{
	wmi_vdev_start_request_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	wmi_channel *chan;
	int32_t len, ret;
	uint8_t *buf_ptr;

	len = sizeof(*cmd) + sizeof(wmi_channel) + WMI_TLV_HDR_SIZE;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_start_request_cmd_fixed_param *) buf_ptr;
	chan = (wmi_channel *) (buf_ptr + sizeof(*cmd));
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_start_request_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_start_request_cmd_fixed_param));
	WMITLV_SET_HDR(&chan->tlv_header, WMITLV_TAG_STRUC_wmi_channel,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
	cmd->vdev_id = req->vdev_id;

	/* Fill channel info */
	copy_channel_info(cmd, chan, req);

	cmd->beacon_interval = req->beacon_intval;
	cmd->dtim_period = req->dtim_period;

	if (!req->is_restart) {
		cmd->beacon_interval = req->beacon_intval;
		cmd->dtim_period = req->dtim_period;

		/* Copy the SSID */
		if (req->ssid.length) {
			if (req->ssid.length < sizeof(cmd->ssid.ssid))
				cmd->ssid.ssid_len = req->ssid.length;
			else
				cmd->ssid.ssid_len = sizeof(cmd->ssid.ssid);
			qdf_mem_copy(cmd->ssid.ssid, req->ssid.mac_ssid,
				     cmd->ssid.ssid_len);
		}

		if (req->hidden_ssid)
			cmd->flags |= WMI_UNIFIED_VDEV_START_HIDDEN_SSID;

		if (req->pmf_enabled)
			cmd->flags |= WMI_UNIFIED_VDEV_START_PMF_ENABLED;
	}

	cmd->num_noa_descriptors = req->num_noa_descriptors;
	cmd->preferred_rx_streams = req->preferred_rx_streams;
	cmd->preferred_tx_streams = req->preferred_tx_streams;
	cmd->cac_duration_ms = req->cac_duration_ms;
	cmd->regdomain = req->regdomain;
	cmd->he_ops = req->he_ops;

	buf_ptr = (uint8_t *) (((uintptr_t) cmd) + sizeof(*cmd) +
			       sizeof(wmi_channel));
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       cmd->num_noa_descriptors *
		       sizeof(wmi_p2p_noa_descriptor));
	WMI_LOGA("%s: vdev_id %d freq %d chanmode %d ch_info: 0x%x is_dfs %d "
		"beacon interval %d dtim %d center_chan %d center_freq2 %d "
		"reg_info_1: 0x%x reg_info_2: 0x%x, req->max_txpow: 0x%x "
		"Tx SS %d, Rx SS %d, cac %d, regd %d, HE ops: %d",
		__func__, req->vdev_id, chan->mhz, req->chan_mode, chan->info,
		req->is_dfs, req->beacon_intval, cmd->dtim_period,
		chan->band_center_freq1, chan->band_center_freq2,
		chan->reg_info_1, chan->reg_info_2, req->max_txpow,
		req->preferred_tx_streams, req->preferred_rx_streams,
		req->cac_duration_ms, req->regdomain, req->he_ops);

	if (req->is_restart)
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					   WMI_VDEV_RESTART_REQUEST_CMDID);
	else
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					   WMI_VDEV_START_REQUEST_CMDID);
	 if (ret) {
		WMI_LOGP("%s: Failed to send vdev start command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	 }

	return QDF_STATUS_SUCCESS;
}

/**
 * send_hidden_ssid_vdev_restart_cmd_tlv() - restart vdev to set hidden ssid
 * @wmi_handle: wmi handle
 * @restart_params: vdev restart params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_hidden_ssid_vdev_restart_cmd_tlv(wmi_unified_t wmi_handle,
			struct hidden_ssid_vdev_restart_params *restart_params)
{
	wmi_vdev_start_request_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	wmi_channel *chan;
	int32_t len;
	uint8_t *buf_ptr;
	QDF_STATUS ret = 0;

	len = sizeof(*cmd) + sizeof(wmi_channel) + WMI_TLV_HDR_SIZE;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_start_request_cmd_fixed_param *) buf_ptr;
	chan = (wmi_channel *) (buf_ptr + sizeof(*cmd));

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_start_request_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_start_request_cmd_fixed_param));

	WMITLV_SET_HDR(&chan->tlv_header,
		       WMITLV_TAG_STRUC_wmi_channel,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));

	cmd->vdev_id = restart_params->session_id;
	cmd->ssid.ssid_len = restart_params->ssid_len;
	qdf_mem_copy(cmd->ssid.ssid,
		     restart_params->ssid,
		     cmd->ssid.ssid_len);
	cmd->flags = restart_params->flags;
	cmd->requestor_id = restart_params->requestor_id;
	cmd->disable_hw_ack = restart_params->disable_hw_ack;

	chan->mhz = restart_params->mhz;
	chan->band_center_freq1 =
			restart_params->band_center_freq1;
	chan->band_center_freq2 =
			restart_params->band_center_freq2;
	chan->info = restart_params->info;
	chan->reg_info_1 = restart_params->reg_info_1;
	chan->reg_info_2 = restart_params->reg_info_2;

	cmd->num_noa_descriptors = 0;
	buf_ptr = (uint8_t *) (((uint8_t *) cmd) + sizeof(*cmd) +
			       sizeof(wmi_channel));
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       cmd->num_noa_descriptors *
		       sizeof(wmi_p2p_noa_descriptor));

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_RESTART_REQUEST_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}


/**
 * send_peer_flush_tids_cmd_tlv() - flush peer tids packets in fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to hold peer flush tid parameter
 *
 * Return: 0 for sucess or error code
 */
static QDF_STATUS send_peer_flush_tids_cmd_tlv(wmi_unified_t wmi,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 struct peer_flush_params *param)
{
	wmi_peer_flush_tids_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_peer_flush_tids_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_flush_tids_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_flush_tids_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->peer_tid_bitmap = param->peer_tid_bitmap;
	cmd->vdev_id = param->vdev_id;
	WMI_LOGD("%s: peer_addr %pM vdev_id %d and peer bitmap %d", __func__,
				peer_addr, param->vdev_id,
				param->peer_tid_bitmap);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_FLUSH_TIDS_CMDID)) {
		WMI_LOGP("%s: Failed to send flush tid command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * send_peer_delete_cmd_tlv() - send PEER delete command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac addr
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_peer_delete_cmd_tlv(wmi_unified_t wmi,
				 uint8_t peer_addr[IEEE80211_ADDR_LEN],
				 uint8_t vdev_id)
{
	wmi_peer_delete_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_peer_delete_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_delete_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_delete_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->vdev_id = vdev_id;

	WMI_LOGD("%s: peer_addr %pM vdev_id %d", __func__, peer_addr, vdev_id);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_DELETE_CMDID)) {
		WMI_LOGP("%s: Failed to send peer delete command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * convert_host_peer_id_to_target_id_tlv - convert host peer param_id
 * to target id.
 * @targ_paramid: Target parameter id to hold the result.
 * @peer_param_id: host param id.
 *
 * Return: QDF_STATUS_SUCCESS for success
 *         QDF_STATUS_E_NOSUPPORT when the param_id in not supported in tareget
 */
#ifdef CONFIG_MCL
static QDF_STATUS convert_host_peer_id_to_target_id_tlv(
		uint32_t *targ_paramid,
		uint32_t peer_param_id)
{
	*targ_paramid = peer_param_id;
	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS convert_host_peer_id_to_target_id_tlv(
		uint32_t *targ_paramid,
		uint32_t peer_param_id)
{
	switch (peer_param_id) {
	case WMI_HOST_PEER_MIMO_PS_STATE:
		*targ_paramid = WMI_PEER_MIMO_PS_STATE;
		break;
	case WMI_HOST_PEER_AMPDU:
		*targ_paramid = WMI_PEER_AMPDU;
		break;
	case WMI_HOST_PEER_AUTHORIZE:
		*targ_paramid = WMI_PEER_AUTHORIZE;
		break;
	case WMI_HOST_PEER_CHWIDTH:
		*targ_paramid = WMI_PEER_CHWIDTH;
		break;
	case WMI_HOST_PEER_NSS:
		*targ_paramid = WMI_PEER_NSS;
		break;
	case WMI_HOST_PEER_USE_4ADDR:
		*targ_paramid = WMI_PEER_USE_4ADDR;
		break;
	case WMI_HOST_PEER_MEMBERSHIP:
		*targ_paramid = WMI_PEER_MEMBERSHIP;
		break;
	case WMI_HOST_PEER_USERPOS:
		*targ_paramid = WMI_PEER_USERPOS;
		break;
	case WMI_HOST_PEER_CRIT_PROTO_HINT_ENABLED:
		*targ_paramid = WMI_PEER_CRIT_PROTO_HINT_ENABLED;
		break;
	case WMI_HOST_PEER_TX_FAIL_CNT_THR:
		*targ_paramid = WMI_PEER_TX_FAIL_CNT_THR;
		break;
	case WMI_HOST_PEER_SET_HW_RETRY_CTS2S:
		*targ_paramid = WMI_PEER_SET_HW_RETRY_CTS2S;
		break;
	case WMI_HOST_PEER_IBSS_ATIM_WINDOW_LENGTH:
		*targ_paramid = WMI_PEER_IBSS_ATIM_WINDOW_LENGTH;
		break;
	case WMI_HOST_PEER_PHYMODE:
		*targ_paramid = WMI_PEER_PHYMODE;
		break;
	case WMI_HOST_PEER_USE_FIXED_PWR:
		*targ_paramid = WMI_PEER_USE_FIXED_PWR;
		break;
	case WMI_HOST_PEER_PARAM_FIXED_RATE:
		*targ_paramid = WMI_PEER_PARAM_FIXED_RATE;
		break;
	case WMI_HOST_PEER_SET_MU_WHITELIST:
		*targ_paramid = WMI_PEER_SET_MU_WHITELIST;
		break;
	case WMI_HOST_PEER_SET_MAC_TX_RATE:
		*targ_paramid = WMI_PEER_SET_MAX_TX_RATE;
		break;
	case WMI_HOST_PEER_SET_MIN_TX_RATE:
		*targ_paramid = WMI_PEER_SET_MIN_TX_RATE;
		break;
	case WMI_HOST_PEER_SET_DEFAULT_ROUTING:
		*targ_paramid = WMI_PEER_SET_DEFAULT_ROUTING;
		break;
	default:
		return QDF_STATUS_E_NOSUPPORT;
	}

	return QDF_STATUS_SUCCESS;
}
#endif
/**
 * send_peer_param_cmd_tlv() - set peer parameter in fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @param    : pointer to hold peer set parameter
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_peer_param_cmd_tlv(wmi_unified_t wmi,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param)
{
	wmi_peer_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t err;
	uint32_t param_id;

	if (convert_host_peer_id_to_target_id_tlv(&param_id,
				param->param_id) != QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_NOSUPPORT;

	buf = wmi_buf_alloc(wmi, sizeof(*cmd));
	if (!buf) {
		WMI_LOGE("Failed to allocate buffer to send set_param cmd");
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_peer_set_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
				(wmi_peer_set_param_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->param_id = param_id;
	cmd->param_value = param->param_value;
	err = wmi_unified_cmd_send(wmi, buf,
				   sizeof(wmi_peer_set_param_cmd_fixed_param),
				   WMI_PEER_SET_PARAM_CMDID);
	if (err) {
		WMI_LOGE("Failed to send set_param cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * send_vdev_up_cmd_tlv() - send vdev up command in fw
 * @wmi: wmi handle
 * @bssid: bssid
 * @vdev_up_params: pointer to hold vdev up parameter
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_vdev_up_cmd_tlv(wmi_unified_t wmi,
			     uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *params)
{
	wmi_vdev_up_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	WMI_LOGD("%s: VDEV_UP", __func__);
	WMI_LOGD("%s: vdev_id %d aid %d bssid %pM", __func__,
		 params->vdev_id, params->assoc_id, bssid);
	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_up_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_up_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_up_cmd_fixed_param));
	cmd->vdev_id = params->vdev_id;
	cmd->vdev_assoc_id = params->assoc_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(bssid, &cmd->vdev_bssid);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_UP_CMDID)) {
		WMI_LOGP("%s: Failed to send vdev up command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
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
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_peer_create_cmd_tlv(wmi_unified_t wmi,
					struct peer_create_params *param)
{
	wmi_peer_create_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
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
		WMI_LOGP("%s: failed to send WMI_PEER_CREATE_CMDID", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("%s: peer_addr %pM vdev_id %d", __func__, param->peer_addr,
			param->vdev_id);

	return 0;
}

/**
 * send_peer_rx_reorder_queue_setup_cmd_tlv() - send rx reorder setup
 * 	command to fw
 * @wmi: wmi handle
 * @rx_reorder_queue_setup_params: Rx reorder queue setup parameters
 *
 * Return: 0 for success or error code
 */
static
QDF_STATUS send_peer_rx_reorder_queue_setup_cmd_tlv(wmi_unified_t wmi,
		struct rx_reorder_queue_setup_params *param)
{
	wmi_peer_reorder_queue_setup_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_peer_reorder_queue_setup_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_peer_reorder_queue_setup_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			(wmi_peer_reorder_queue_setup_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_macaddr, &cmd->peer_macaddr);
	cmd->vdev_id = param->vdev_id;
	cmd->tid = param->tid;
	cmd->queue_ptr_lo = param->hw_qdesc_paddr_lo;
	cmd->queue_ptr_hi = param->hw_qdesc_paddr_hi;
	cmd->queue_no = param->queue_no;

	if (wmi_unified_cmd_send(wmi, buf, len,
			WMI_PEER_REORDER_QUEUE_SETUP_CMDID)) {
		WMI_LOGP("%s: fail to send WMI_PEER_REORDER_QUEUE_SETUP_CMDID",
			__func__);
		qdf_nbuf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("%s: peer_macaddr %pM vdev_id %d, tid %d\n", __func__,
		param->peer_macaddr, param->vdev_id, param->tid);

	return QDF_STATUS_SUCCESS;
}

/**
 * send_peer_rx_reorder_queue_remove_cmd_tlv() - send rx reorder remove
 * 	command to fw
 * @wmi: wmi handle
 * @rx_reorder_queue_remove_params: Rx reorder queue remove parameters
 *
 * Return: 0 for success or error code
 */
static
QDF_STATUS send_peer_rx_reorder_queue_remove_cmd_tlv(wmi_unified_t wmi,
		struct rx_reorder_queue_remove_params *param)
{
	wmi_peer_reorder_queue_remove_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_peer_reorder_queue_remove_cmd_fixed_param *)
			wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_peer_reorder_queue_remove_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			(wmi_peer_reorder_queue_remove_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_macaddr, &cmd->peer_macaddr);
	cmd->vdev_id = param->vdev_id;
	cmd->tid_mask = param->peer_tid_bitmap;

	if (wmi_unified_cmd_send(wmi, buf, len,
			WMI_PEER_REORDER_QUEUE_REMOVE_CMDID)) {
		WMI_LOGP("%s: fail to send WMI_PEER_REORDER_QUEUE_REMOVE_CMDID",
			__func__);
		qdf_nbuf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("%s: peer_macaddr %pM vdev_id %d, tid_map %d", __func__,
		param->peer_macaddr, param->vdev_id, param->peer_tid_bitmap);

	return QDF_STATUS_SUCCESS;
}

/**
 * send_peer_add_wds_entry_cmd_tlv() - send peer add command to fw
 * @wmi_handle: wmi handle
 * @param: pointer holding peer details
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_peer_add_wds_entry_cmd_tlv(wmi_unified_t wmi_handle,
					struct peer_add_wds_entry_params *param)
{
	wmi_peer_add_wds_entry_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_peer_add_wds_entry_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_peer_add_wds_entry_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN
				(wmi_peer_add_wds_entry_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->dest_addr, &cmd->wds_macaddr);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_addr, &cmd->peer_macaddr);
	cmd->flags = param->flags;

	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_ADD_WDS_ENTRY_CMDID);
}

/**
 * send_peer_del_wds_entry_cmd_tlv() - send peer delete command to fw
 * @wmi_handle: wmi handle
 * @param: pointer holding peer details
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_peer_del_wds_entry_cmd_tlv(wmi_unified_t wmi_handle,
					struct peer_del_wds_entry_params *param)
{
	wmi_peer_remove_wds_entry_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_peer_remove_wds_entry_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_peer_remove_wds_entry_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN
				(wmi_peer_remove_wds_entry_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->dest_addr, &cmd->wds_macaddr);
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_REMOVE_WDS_ENTRY_CMDID);
}

/**
 * send_peer_update_wds_entry_cmd_non_tlv() - send peer update command to fw
 * @wmi_handle: wmi handle
 * @param: pointer holding peer details
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_peer_update_wds_entry_cmd_tlv(wmi_unified_t wmi_handle,
				struct peer_update_wds_entry_params *param)
{
	wmi_peer_update_wds_entry_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	/* wmi_buf_alloc returns zeroed command buffer */
	cmd = (wmi_peer_update_wds_entry_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_peer_update_wds_entry_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN
				(wmi_peer_update_wds_entry_cmd_fixed_param));
	cmd->flags = (param->flags) ? WMI_WDS_FLAG_STATIC : 0;
	if (param->wds_macaddr)
		WMI_CHAR_ARRAY_TO_MAC_ADDR(param->wds_macaddr,
				&cmd->wds_macaddr);
	if (param->peer_macaddr)
		WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_macaddr,
				&cmd->peer_macaddr);
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_UPDATE_WDS_ENTRY_CMDID);
}



/**
 * send_green_ap_ps_cmd_tlv() - enable green ap powersave command
 * @wmi_handle: wmi handle
 * @value: value
 * @mac_id: mac id to have radio context
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_green_ap_ps_cmd_tlv(wmi_unified_t wmi_handle,
						uint32_t value, uint8_t mac_id)
{
	wmi_pdev_green_ap_ps_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	WMI_LOGD("Set Green AP PS val %d", value);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: Green AP PS Mem Alloc Failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_green_ap_ps_enable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		   WMITLV_TAG_STRUC_wmi_pdev_green_ap_ps_enable_cmd_fixed_param,
		   WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_green_ap_ps_enable_cmd_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(mac_id);
	cmd->enable = value;

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_GREEN_AP_PS_ENABLE_CMDID)) {
		WMI_LOGE("Set Green AP PS param Failed val %d", value);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * send_pdev_utf_cmd_tlv() - send utf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev_utf_params
 * @mac_id: mac id to have radio context
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
send_pdev_utf_cmd_tlv(wmi_unified_t wmi_handle,
				struct pdev_utf_params *param,
				uint8_t mac_id)
{
	wmi_buf_t buf;
	uint8_t *cmd;
	/* if param->len is 0 no data is sent, return error */
	QDF_STATUS ret = QDF_STATUS_E_INVAL;
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
			WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
			return QDF_STATUS_E_NOMEM;
		}

		cmd = (uint8_t *) wmi_buf_data(buf);

		segHdrInfo.len = total_bytes;
		segHdrInfo.msgref = msgref;
		segInfo = ((numSegments << 4) & 0xF0) | (segNumber & 0xF);
		segHdrInfo.segmentInfo = segInfo;
		segHdrInfo.pad = 0;

		WMI_LOGD("%s:segHdrInfo.len = %d, segHdrInfo.msgref = %d,"
			 " segHdrInfo.segmentInfo = %d",
			 __func__, segHdrInfo.len, segHdrInfo.msgref,
			 segHdrInfo.segmentInfo);

		WMI_LOGD("%s:total_bytes %d segNumber %d totalSegments %d"
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

		if (QDF_IS_STATUS_ERROR(ret)) {
			WMI_LOGE("Failed to send WMI_PDEV_UTF_CMDID command");
			wmi_buf_free(buf);
			break;
		}

		param->len -= chunk_len;
		bufpos += chunk_len;
	}

	msgref++;

	return ret;
}
#ifdef CONFIG_MCL
static inline uint32_t convert_host_pdev_param_tlv(wmi_unified_t wmi_handle,
				uint32_t host_param)
{
	return host_param;
}
#else
static inline uint32_t convert_host_pdev_param_tlv(wmi_unified_t wmi_handle,
				uint32_t host_param)
{
	if (host_param < wmi_pdev_param_max)
		return wmi_handle->pdev_param[host_param];

	return WMI_UNAVAILABLE_PARAM;
}
#endif
/**
 * send_pdev_param_cmd_tlv() - set pdev parameters
 * @wmi_handle: wmi handle
 * @param: pointer to pdev parameter
 * @mac_id: radio context
 *
 * Return: 0 on success, errno on failure
 */
static QDF_STATUS
send_pdev_param_cmd_tlv(wmi_unified_t wmi_handle,
			   struct pdev_params *param,
				uint8_t mac_id)
{
	QDF_STATUS ret;
	wmi_pdev_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);
	uint32_t pdev_param;

	pdev_param = convert_host_pdev_param_tlv(wmi_handle, param->param_id);
	if (pdev_param == WMI_UNAVAILABLE_PARAM) {
		WMI_LOGW("%s: Unavailable param %d\n",
				__func__, param->param_id);
		return QDF_STATUS_E_INVAL;
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_pdev_set_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_set_param_cmd_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(mac_id);
	cmd->param_id = pdev_param;
	cmd->param_value = param->param_value;
	WMI_LOGD("Setting pdev param = %x, value = %u", param->param_id,
				param->param_value);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PDEV_SET_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send set param command ret = %d", ret);
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
static QDF_STATUS send_suspend_cmd_tlv(wmi_unified_t wmi_handle,
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
		return QDF_STATUS_E_NOMEM;

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
		wmi_buf_free(wmibuf);
		WMI_LOGE("Failed to send WMI_PDEV_SUSPEND_CMDID command");
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
static QDF_STATUS send_resume_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t mac_id)
{
	wmi_buf_t wmibuf;
	wmi_pdev_resume_cmd_fixed_param *cmd;
	QDF_STATUS ret;

	wmibuf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (wmibuf == NULL)
		return QDF_STATUS_E_NOMEM;
	cmd = (wmi_pdev_resume_cmd_fixed_param *) wmi_buf_data(wmibuf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_resume_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_resume_cmd_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(mac_id);
	ret = wmi_unified_cmd_send(wmi_handle, wmibuf, sizeof(*cmd),
				   WMI_PDEV_RESUME_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_PDEV_RESUME_CMDID command");
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
static QDF_STATUS send_wow_enable_cmd_tlv(wmi_unified_t wmi_handle,
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
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
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
	cmd->flags = param->flags;

	WMI_LOGI("suspend type: %s",
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
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to ap_ps parameter structure
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_set_ap_ps_param_cmd_tlv(wmi_unified_t wmi_handle,
					   uint8_t *peer_addr,
					   struct ap_ps_params *param)
{
	wmi_ap_ps_peer_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t err;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGE("Failed to allocate buffer to send set_ap_ps_param cmd");
		return QDF_STATUS_E_NOMEM;
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
		WMI_LOGE("Failed to send set_ap_ps_param cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * send_set_sta_ps_param_cmd_tlv() - set sta powersave parameters
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to sta_ps parameter structure
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_set_sta_ps_param_cmd_tlv(wmi_unified_t wmi_handle,
					   struct sta_ps_params *param)
{
	wmi_sta_powersave_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: Set Sta Ps param Mem Alloc Failed", __func__);
		return QDF_STATUS_E_NOMEM;
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
		WMI_LOGE("Set Sta Ps param Failed vdevId %d Param %d val %d",
			 param->vdev_id, param->param, param->value);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * send_crash_inject_cmd_tlv() - inject fw crash
 * @wmi_handle: wmi handle
 * @param: ponirt to crash inject paramter structure
 *
 * Return: QDF_STATUS_SUCCESS for success or return error
 */
static QDF_STATUS send_crash_inject_cmd_tlv(wmi_unified_t wmi_handle,
			 struct crash_inject *param)
{
	int32_t ret = 0;
	WMI_FORCE_FW_HANG_CMD_fixed_param *cmd;
	uint16_t len = sizeof(*cmd);
	wmi_buf_t buf;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed!", __func__);
		return QDF_STATUS_E_NOMEM;
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
		WMI_LOGE("%s: Failed to send set param command, ret = %d",
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
 static QDF_STATUS
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
		wmi_buf_free(buf);

	return status;
}

#ifdef CONFIG_MCL
static inline uint32_t convert_host_vdev_param_tlv(wmi_unified_t wmi_handle,
				uint32_t host_param)
{
	return host_param;
}
#else
static inline uint32_t convert_host_vdev_param_tlv(wmi_unified_t wmi_handle,
				uint32_t host_param)
{
	if (host_param < wmi_vdev_param_max)
		return wmi_handle->vdev_param[host_param];

	return WMI_UNAVAILABLE_PARAM;
}
#endif
/**
 *  send_vdev_set_param_cmd_tlv() - WMI vdev set parameter function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold vdev set parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_vdev_set_param_cmd_tlv(wmi_unified_t wmi_handle,
				struct vdev_set_params *param)
{
	QDF_STATUS ret;
	wmi_vdev_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);
	uint32_t vdev_param;

	vdev_param = convert_host_vdev_param_tlv(wmi_handle, param->param_id);
	if (vdev_param == WMI_UNAVAILABLE_PARAM) {
		WMI_LOGW("%s:Vdev param %d not available", __func__,
				param->param_id);
		return QDF_STATUS_E_INVAL;

	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_set_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_set_param_cmd_fixed_param));
	cmd->vdev_id = param->if_id;
	cmd->param_id = vdev_param;
	cmd->param_value = param->param_value;
	WMI_LOGD("Setting vdev %d param = %x, value = %u",
		 param->if_id, param->param_id, param->param_value);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SET_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send set param command ret = %d", ret);
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
static QDF_STATUS send_stats_request_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param)
{
	int32_t ret;
	wmi_request_stats_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return -QDF_STATUS_E_NOMEM;
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
		WMI_LOGE("Failed to send status request to fw =%d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

#ifdef CONFIG_WIN
/**
 *  send_packet_log_enable_cmd_tlv() - WMI request stats function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold stats request parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_packet_log_enable_cmd_tlv(wmi_unified_t wmi_handle,
				WMI_HOST_PKTLOG_EVENT PKTLOG_EVENT)
{
	return 0;
}
#else
/**
 *  send_packet_log_enable_cmd_tlv() - WMI request stats function
 *  @param wmi_handle      : handle to WMI.
 *  @param macaddr        : MAC address
 *  @param param    : pointer to hold stats request parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_packet_log_enable_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct packet_enable_params *param)
{
	return 0;
}
#endif

#ifdef CONFIG_MCL
/**
 *  send_beacon_send_cmd_tlv() - WMI beacon send function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold beacon send cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_beacon_send_cmd_tlv(wmi_unified_t wmi_handle,
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
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
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
	qdf_mem_copy(buf_ptr, param->frm, param->tmpl_len);

	ret = wmi_unified_cmd_send(wmi_handle,
				   wmi_buf, wmi_buf_len, WMI_BCN_TMPL_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send bcn tmpl: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}
	return 0;
}
#else
static QDF_STATUS send_beacon_send_cmd_tlv(wmi_unified_t wmi_handle,
				struct beacon_params *param)
{
	QDF_STATUS ret;
	wmi_bcn_send_from_host_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	qdf_dma_addr_t dma_addr;
	uint32_t dtim_flag = 0;

	wmi_buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!wmi_buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	if (param->is_dtim_count_zero) {
		dtim_flag |= WMI_BCN_SEND_DTIM_ZERO;
		if (param->is_bitctl_reqd) {
			/* deliver CAB traffic in next DTIM beacon */
			dtim_flag |= WMI_BCN_SEND_DTIM_BITCTL_SET;
		}
	}
	cmd = (wmi_bcn_send_from_host_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_bcn_send_from_host_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
				(wmi_bcn_send_from_host_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->data_len = qdf_nbuf_len(param->wbuf);
	cmd->frame_ctrl = param->frame_ctrl;
	cmd->dtim_flag = dtim_flag;
	dma_addr = qdf_nbuf_get_frag_paddr(param->wbuf, 0);
	cmd->frag_ptr_lo = qdf_get_lower_32_bits(dma_addr);
#if defined(HTT_PADDR64)
	cmd->frag_ptr_hi = qdf_get_upper_32_bits(dma_addr) & 0x1F;
#endif
	cmd->bcn_antenna = param->bcn_txant;

	ret = wmi_unified_cmd_send(wmi_handle,
			wmi_buf, sizeof(*cmd), WMI_PDEV_SEND_BCN_CMDID);
	if (ret != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s: Failed to send bcn: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}

	return ret;
}

/**
 *  send_beacon_send_tmpl_cmd_tlv() - WMI beacon send function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold beacon send cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_beacon_tmpl_send_cmd_tlv(wmi_unified_t wmi_handle,
				struct beacon_tmpl_params *param)
{
	int32_t ret;
	wmi_bcn_tmpl_cmd_fixed_param *cmd;
	wmi_bcn_prb_info *bcn_prb_info;
	wmi_buf_t wmi_buf;
	uint8_t *buf_ptr;
	uint32_t wmi_buf_len;

	WMI_LOGI("%s\n", __func__);
	wmi_buf_len = sizeof(wmi_bcn_tmpl_cmd_fixed_param) +
		      sizeof(wmi_bcn_prb_info) + WMI_TLV_HDR_SIZE +
		      param->tmpl_len_aligned;
	wmi_buf = wmi_buf_alloc(wmi_handle, wmi_buf_len);
	if (!wmi_buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_bcn_tmpl_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_tmpl_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_bcn_tmpl_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->tim_ie_offset = param->tim_ie_offset;
	cmd->csa_switch_count_offset = param->csa_switch_count_offset;
	cmd->ext_csa_switch_count_offset = param->ext_csa_switch_count_offset;
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
	qdf_mem_copy(buf_ptr, param->frm, param->tmpl_len);

	ret = wmi_unified_cmd_send(wmi_handle,
				   wmi_buf, wmi_buf_len, WMI_BCN_TMPL_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send bcn tmpl: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}

	return 0;
}
#endif

#ifdef CONFIG_MCL
static inline void copy_peer_flags_tlv(
			wmi_peer_assoc_complete_cmd_fixed_param * cmd,
			struct peer_assoc_params *param)
{
	cmd->peer_flags = param->peer_flags;
}
#else
static inline void copy_peer_flags_tlv(
			wmi_peer_assoc_complete_cmd_fixed_param * cmd,
			struct peer_assoc_params *param)
{
	/*
	 * The target only needs a subset of the flags maintained in the host.
	 * Just populate those flags and send it down
	 */
	cmd->peer_flags = 0;

	/*
	 * Do not enable HT/VHT if WMM/wme is disabled for vap.
	 */
	if (param->is_wme_set) {

		if (param->qos_flag)
			cmd->peer_flags |= WMI_PEER_QOS;
		if (param->apsd_flag)
			cmd->peer_flags |= WMI_PEER_APSD;
		if (param->ht_flag)
			cmd->peer_flags |= WMI_PEER_HT;
		if (param->bw_40)
			cmd->peer_flags |= WMI_PEER_40MHZ;
		if (param->bw_80)
			cmd->peer_flags |= WMI_PEER_80MHZ;
		if (param->bw_160)
			cmd->peer_flags |= WMI_PEER_160MHZ;

		/* Typically if STBC is enabled for VHT it should be enabled
		 * for HT as well
		 **/
		if (param->stbc_flag)
			cmd->peer_flags |= WMI_PEER_STBC;

		/* Typically if LDPC is enabled for VHT it should be enabled
		 * for HT as well
		 **/
		if (param->ldpc_flag)
			cmd->peer_flags |= WMI_PEER_LDPC;

		if (param->static_mimops_flag)
			cmd->peer_flags |= WMI_PEER_STATIC_MIMOPS;
		if (param->dynamic_mimops_flag)
			cmd->peer_flags |= WMI_PEER_DYN_MIMOPS;
		if (param->spatial_mux_flag)
			cmd->peer_flags |= WMI_PEER_SPATIAL_MUX;
		if (param->vht_flag)
			cmd->peer_flags |= WMI_PEER_VHT;
		if (param->he_flag)
			cmd->peer_flags |= WMI_PEER_HE;
	}

	/*
	 * Suppress authorization for all AUTH modes that need 4-way handshake
	 * (during re-association).
	 * Authorization will be done for these modes on key installation.
	 */
	if (param->auth_flag)
		cmd->peer_flags |= WMI_PEER_AUTH;
	if (param->need_ptk_4_way)
		cmd->peer_flags |= WMI_PEER_NEED_PTK_4_WAY;
	else
		cmd->peer_flags &= ~WMI_PEER_NEED_PTK_4_WAY;
	if (param->need_gtk_2_way)
		cmd->peer_flags |= WMI_PEER_NEED_GTK_2_WAY;
	/* safe mode bypass the 4-way handshake */
	if (param->safe_mode_enabled)
		cmd->peer_flags &=
		    ~(WMI_PEER_NEED_PTK_4_WAY | WMI_PEER_NEED_GTK_2_WAY);
	/* Disable AMSDU for station transmit, if user configures it */
	/* Disable AMSDU for AP transmit to 11n Stations, if user configures
	 * it
	 * if (param->amsdu_disable) Add after FW support
	 **/

	/* Target asserts if node is marked HT and all MCS is set to 0.
	 * Mark the node as non-HT if all the mcs rates are disabled through
	 * iwpriv
	 **/
	if (param->peer_ht_rates.num_rates == 0)
		cmd->peer_flags &= ~WMI_PEER_HT;
}
#endif

#ifdef CONFIG_MCL
static inline void copy_peer_mac_addr_tlv(
		wmi_peer_assoc_complete_cmd_fixed_param * cmd,
		struct peer_assoc_params *param)
{
	qdf_mem_copy(&cmd->peer_macaddr, &param->peer_macaddr,
			sizeof(param->peer_macaddr));
}
#else
static inline void copy_peer_mac_addr_tlv(
		wmi_peer_assoc_complete_cmd_fixed_param * cmd,
		struct peer_assoc_params *param)
{
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_mac, &cmd->peer_macaddr);
}
#endif

/**
 *  send_peer_assoc_cmd_tlv() - WMI peer assoc function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to peer assoc parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_peer_assoc_cmd_tlv(wmi_unified_t wmi_handle,
				struct peer_assoc_params *param)
{
	wmi_peer_assoc_complete_cmd_fixed_param *cmd;
	wmi_vht_rate_set *mcs;
	wmi_he_rate_set *he_mcs;
	wmi_buf_t buf;
	int32_t len;
	uint8_t *buf_ptr;
	QDF_STATUS ret;
	uint32_t peer_legacy_rates_align;
	uint32_t peer_ht_rates_align;
	int32_t i;


	peer_legacy_rates_align = wmi_align(param->peer_legacy_rates.num_rates);
	peer_ht_rates_align = wmi_align(param->peer_ht_rates.num_rates);

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE +
		(peer_legacy_rates_align * sizeof(uint8_t)) +
		WMI_TLV_HDR_SIZE +
		(peer_ht_rates_align * sizeof(uint8_t)) +
		sizeof(wmi_vht_rate_set) +
		(sizeof(wmi_he_rate_set) * param->peer_he_mcs_count
		+ WMI_TLV_HDR_SIZE);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
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

	copy_peer_flags_tlv(cmd, param);
	copy_peer_mac_addr_tlv(cmd, param);

	cmd->peer_rate_caps = param->peer_rate_caps;
	cmd->peer_caps = param->peer_caps;
	cmd->peer_listen_intval = param->peer_listen_intval;
	cmd->peer_ht_caps = param->peer_ht_caps;
	cmd->peer_max_mpdu = param->peer_max_mpdu;
	cmd->peer_mpdu_density = param->peer_mpdu_density;
	cmd->peer_vht_caps = param->peer_vht_caps;
	cmd->peer_phymode = param->peer_phymode;

	/* Update 11ax capabilities */
	cmd->peer_he_cap_info = param->peer_he_cap_macinfo;
	cmd->peer_he_ops = param->peer_he_ops;
	qdf_mem_copy(&cmd->peer_he_cap_phy, &param->peer_he_cap_phyinfo,
				sizeof(param->peer_he_cap_phyinfo));
	qdf_mem_copy(&cmd->peer_ppet, &param->peer_ppet,
				sizeof(param->peer_ppet));

	/* Update peer legacy rate information */
	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
				peer_legacy_rates_align);
	buf_ptr += WMI_TLV_HDR_SIZE;
	cmd->num_peer_legacy_rates = param->peer_legacy_rates.num_rates;
	qdf_mem_copy(buf_ptr, param->peer_legacy_rates.rates,
		     param->peer_legacy_rates.num_rates);

	/* Update peer HT rate information */
	buf_ptr += peer_legacy_rates_align;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
			  peer_ht_rates_align);
	buf_ptr += WMI_TLV_HDR_SIZE;
	cmd->num_peer_ht_rates = param->peer_ht_rates.num_rates;
	qdf_mem_copy(buf_ptr, param->peer_ht_rates.rates,
				 param->peer_ht_rates.num_rates);

	/* VHT Rates */
	buf_ptr += peer_ht_rates_align;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_STRUC_wmi_vht_rate_set,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vht_rate_set));

	cmd->peer_nss = param->peer_nss;

	/* Update bandwidth-NSS mapping */
	cmd->peer_bw_rxnss_override = 0;
	cmd->peer_bw_rxnss_override |= param->peer_bw_rxnss_override;

	mcs = (wmi_vht_rate_set *) buf_ptr;
	if (param->vht_capable) {
		mcs->rx_max_rate = param->rx_max_rate;
		mcs->rx_mcs_set = param->rx_mcs_set;
		mcs->tx_max_rate = param->tx_max_rate;
		mcs->tx_mcs_set = param->tx_mcs_set;
	}

	/* HE Rates */
	cmd->peer_he_mcs = param->peer_he_mcs_count;
	buf_ptr += sizeof(wmi_vht_rate_set);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		(param->peer_he_mcs_count * sizeof(wmi_he_rate_set)));
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Loop through the HE rate set */
	for (i = 0; i < param->peer_he_mcs_count; i++) {
		he_mcs = (wmi_he_rate_set *) buf_ptr;
		WMITLV_SET_HDR(he_mcs, WMITLV_TAG_STRUC_wmi_he_rate_set,
			WMITLV_GET_STRUCT_TLVLEN(wmi_he_rate_set));

		he_mcs->rx_mcs_set = param->peer_he_rx_mcs_set[i];
		he_mcs->tx_mcs_set = param->peer_he_tx_mcs_set[i];
		WMI_LOGD("%s:HE idx %d RxMCSmap %x TxMCSmap %x ", __func__,
			i, he_mcs->rx_mcs_set, he_mcs->tx_mcs_set);
		buf_ptr += sizeof(wmi_he_rate_set);
	}


	WMI_LOGD("%s: vdev_id %d associd %d peer_flags %x rate_caps %x "
		 "peer_caps %x listen_intval %d ht_caps %x max_mpdu %d "
		 "nss %d phymode %d peer_mpdu_density %d "
		 "cmd->peer_vht_caps %x "
		 "HE cap_info %x ops %x "
		 "HE phy %x  %x  %x  "
		 "peer_bw_rxnss_override %x", __func__,
		 cmd->vdev_id, cmd->peer_associd, cmd->peer_flags,
		 cmd->peer_rate_caps, cmd->peer_caps,
		 cmd->peer_listen_intval, cmd->peer_ht_caps,
		 cmd->peer_max_mpdu, cmd->peer_nss, cmd->peer_phymode,
		 cmd->peer_mpdu_density,
		 cmd->peer_vht_caps, cmd->peer_he_cap_info,
		 cmd->peer_he_ops, cmd->peer_he_cap_phy[0],
		 cmd->peer_he_cap_phy[1], cmd->peer_he_cap_phy[2],
		 cmd->peer_bw_rxnss_override);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PEER_ASSOC_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGP("%s: Failed to send peer assoc command ret = %d",
			 __func__, ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/* copy_scan_notify_events() - Helper routine to copy scan notify events
 */
static inline void copy_scan_event_cntrl_flags(
		wmi_start_scan_cmd_fixed_param * cmd,
		struct scan_req_params *param)
{

	/* Scan events subscription */
	if (param->scan_ev_started)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_STARTED;
	if (param->scan_ev_completed)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_COMPLETED;
	if (param->scan_ev_bss_chan)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_BSS_CHANNEL;
	if (param->scan_ev_foreign_chan)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_FOREIGN_CHANNEL;
	if (param->scan_ev_dequeued)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_DEQUEUED;
	if (param->scan_ev_preempted)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_PREEMPTED;
	if (param->scan_ev_start_failed)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_START_FAILED;
	if (param->scan_ev_restarted)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_RESTARTED;
	if (param->scan_ev_foreign_chn_exit)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_FOREIGN_CHANNEL_EXIT;
	if (param->scan_ev_suspended)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_SUSPENDED;
	if (param->scan_ev_resumed)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_RESUMED;

	/** Set scan control flags */
	cmd->scan_ctrl_flags = 0;
	if (param->scan_f_passive)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_PASSIVE;
	if (param->scan_f_strict_passive_pch)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_STRICT_PASSIVE_ON_PCHN;
	if (param->scan_f_promisc_mode)
		cmd->scan_ctrl_flags |= WMI_SCAN_FILTER_PROMISCOUS;
	if (param->scan_f_capture_phy_err)
		cmd->scan_ctrl_flags |= WMI_SCAN_CAPTURE_PHY_ERROR;
	if (param->scan_f_half_rate)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_HALF_RATE_SUPPORT;
	if (param->scan_f_quarter_rate)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_QUARTER_RATE_SUPPORT;
	if (param->scan_f_cck_rates)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_CCK_RATES;
	if (param->scan_f_ofdm_rates)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_OFDM_RATES;
	if (param->scan_f_chan_stat_evnt)
		cmd->scan_ctrl_flags |= WMI_SCAN_CHAN_STAT_EVENT;
	if (param->scan_f_filter_prb_req)
		cmd->scan_ctrl_flags |= WMI_SCAN_FILTER_PROBE_REQ;
	if (param->scan_f_bcast_probe)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_BCAST_PROBE_REQ;
	if (param->scan_f_offchan_mgmt_tx)
		cmd->scan_ctrl_flags |= WMI_SCAN_OFFCHAN_MGMT_TX;
	if (param->scan_f_offchan_data_tx)
		cmd->scan_ctrl_flags |= WMI_SCAN_OFFCHAN_DATA_TX;
	if (param->scan_f_force_active_dfs_chn)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_FORCE_ACTIVE_ON_DFS;
	if (param->scan_f_add_tpc_ie_in_probe)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_TPC_IE_IN_PROBE_REQ;
	if (param->scan_f_add_ds_ie_in_probe)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_DS_IE_IN_PROBE_REQ;
	if (param->scan_f_add_spoofed_mac_in_probe)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_SPOOFED_MAC_IN_PROBE_REQ;
	if (param->scan_f_add_rand_seq_in_probe)
		cmd->scan_ctrl_flags |= WMI_SCAN_RANDOM_SEQ_NO_IN_PROBE_REQ;
	if (param->scan_f_en_ie_whitelist_in_probe)
		cmd->scan_ctrl_flags |=
			WMI_SCAN_ENABLE_IE_WHTELIST_IN_PROBE_REQ;

	/* for adaptive scan mode using 3 bits (21 - 23 bits) */
	WMI_SCAN_SET_DWELL_MODE(cmd->scan_ctrl_flags,
		param->adaptive_dwell_time_mode);
}

/* scan_copy_ie_buffer() - Copy scan ie_data */
static inline void scan_copy_ie_buffer(uint8_t *buf_ptr,
				struct scan_req_params *params)
{
	qdf_mem_copy(buf_ptr, params->extraie.ptr, params->extraie.len);
}

/*
 * get_pdev_wmi_handle() - Helper func to derive pdev wmi handle from vdev_id
 * @param wmi_handle : Handle to WMI
 * @param vdev_id : vdev identifier
 *
 * Return : void *
 */
static inline void *get_pdev_wmi_handle(wmi_unified_t wmi_handle, uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev = NULL;
	struct wlan_objmgr_pdev *pdev = NULL;
	uint8_t pdev_id = 0;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(
			(struct wlan_objmgr_psoc *)wmi_handle->soc->wmi_psoc,
			vdev_id, WLAN_SCAN_ID);
	if (vdev) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_SCAN_ID);
		pdev = wlan_vdev_get_pdev(vdev);
		if (pdev)
			pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
		else {
			qdf_print("%s : Invalid PDEV, forcing pdev_id to 0\n", __func__);
		}
	} else {
		qdf_print("%s : Invalid VDEV, forcing pdev_id to 0\n", __func__);
	}

	return wmi_unified_get_pdev_handle(wmi_handle->soc, pdev_id);
}

/**
 *  send_scan_start_cmd_tlv() - WMI scan start function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold scan start cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_scan_start_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_req_params *params)
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
	uint8_t extraie_len_with_pad = 0;

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
	if (params->num_bssid)
		len += sizeof(wmi_mac_addr) * params->num_bssid;

	/* Length TLV placeholder for array of bytes */
	len += WMI_TLV_HDR_SIZE;
	if (params->extraie.len)
		extraie_len_with_pad =
		roundup(params->extraie.len, sizeof(uint32_t));
		len += extraie_len_with_pad;

	/* Allocate the memory */
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGP("%s: failed to allocate memory for start scan cmd",
			 __func__);
		return QDF_STATUS_E_FAILURE;
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

	copy_scan_event_cntrl_flags(cmd, params);

	cmd->dwell_time_active = params->dwell_time_active;
	cmd->dwell_time_passive = params->dwell_time_passive;
	cmd->min_rest_time = params->min_rest_time;
	cmd->max_rest_time = params->max_rest_time;
	cmd->repeat_probe_time = params->repeat_probe_time;
	cmd->probe_spacing_time = params->probe_spacing_time;
	cmd->idle_time = params->idle_time;
	cmd->max_scan_time = params->max_scan_time;
	cmd->probe_delay = params->probe_delay;
	cmd->burst_duration = params->burst_duration;
	cmd->num_chan = params->num_chan;
	cmd->num_bssid = params->num_bssid;
	cmd->num_ssids = params->num_ssids;
	cmd->ie_len = params->extraie.len;
	cmd->n_probes = params->n_probes;
	buf_ptr += sizeof(*cmd);
	tmp_ptr = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);
	for (i = 0; i < params->num_chan; ++i)
		tmp_ptr[i] = params->chan_list[i];

	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_UINT32,
		       (params->num_chan * sizeof(uint32_t)));
	buf_ptr += WMI_TLV_HDR_SIZE + (params->num_chan * sizeof(uint32_t));
	if (params->num_ssids > WMI_SCAN_MAX_NUM_SSID) {
		WMI_LOGE("Invalid value for numSsid");
		goto error;
	}

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
	       (params->num_ssids * sizeof(wmi_ssid)));

	if (params->num_ssids) {
		ssid = (wmi_ssid *) (buf_ptr + WMI_TLV_HDR_SIZE);
		for (i = 0; i < params->num_ssids; ++i) {
			ssid->ssid_len = params->ssid[i].length;
			qdf_mem_copy(ssid->ssid, params->ssid[i].ssid,
				     params->ssid[i].length);
			ssid++;
		}
	}
	buf_ptr += WMI_TLV_HDR_SIZE + (params->num_ssids * sizeof(wmi_ssid));

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
		       (params->num_bssid * sizeof(wmi_mac_addr)));
	bssid = (wmi_mac_addr *) (buf_ptr + WMI_TLV_HDR_SIZE);

	if (params->num_bssid) {
		for (i = 0; i < params->num_bssid; ++i) {
			WMI_CHAR_ARRAY_TO_MAC_ADDR(
				&params->bssid_list[i].bytes[0], bssid);
			bssid++;
		}
	}

	buf_ptr += WMI_TLV_HDR_SIZE +
		(params->num_bssid * sizeof(wmi_mac_addr));

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, extraie_len_with_pad);
	if (params->extraie.len)
		scan_copy_ie_buffer(buf_ptr + WMI_TLV_HDR_SIZE,
			     params);

	buf_ptr += WMI_TLV_HDR_SIZE + extraie_len_with_pad;

	ret = wmi_unified_cmd_send(
			get_pdev_wmi_handle(wmi_handle, cmd->vdev_id), wmi_buf,
				      len, WMI_START_SCAN_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to start scan: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}
	return ret;
error:
	wmi_buf_free(wmi_buf);
	return QDF_STATUS_E_FAILURE;
}

/**
 *  send_scan_stop_cmd_tlv() - WMI scan start function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold scan cancel cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_scan_stop_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_cancel_param *param)
{
	wmi_stop_scan_cmd_fixed_param *cmd;
	int ret;
	int len = sizeof(*cmd);
	wmi_buf_t wmi_buf;

	/* Allocate the memory */
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGP("%s: failed to allocate memory for stop scan cmd",
			 __func__);
		ret = QDF_STATUS_E_NOMEM;
		goto error;
	}

	cmd = (wmi_stop_scan_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_stop_scan_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_stop_scan_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->requestor = param->requester;
	cmd->scan_id = param->scan_id;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	/* stop the scan with the corresponding scan_id */
	if (param->req_type == WLAN_SCAN_CANCEL_PDEV_ALL) {
		/* Cancelling all scans */
		cmd->req_type = WMI_SCAN_STOP_ALL;
	} else if (param->req_type == WLAN_SCAN_CANCEL_VDEV_ALL) {
		/* Cancelling VAP scans */
		cmd->req_type = WMI_SCN_STOP_VAP_ALL;
	} else if (param->req_type == WLAN_SCAN_CANCEL_SINGLE) {
		/* Cancelling specific scan */
		cmd->req_type = WMI_SCAN_STOP_ONE;
	} else {
		WMI_LOGE("%s: Invalid Command : ", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_INVAL;
	}

	ret = wmi_unified_cmd_send(get_pdev_wmi_handle(wmi_handle, cmd->vdev_id), wmi_buf,
				      len, WMI_STOP_SCAN_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send stop scan: %d", __func__, ret);
		wmi_buf_free(wmi_buf);
	}

error:
	return ret;
}

#ifdef CONFIG_MCL
/**
 *  send_scan_chan_list_cmd_tlv() - WMI scan channel list function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold scan channel list parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_scan_chan_list_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_chan_list_params *chan_list)
{
	wmi_buf_t buf;
	QDF_STATUS qdf_status;
	wmi_scan_chan_list_cmd_fixed_param *cmd;
	int i;
	uint8_t *buf_ptr;
	wmi_channel_param *chan_info, *tchan_info;
	uint16_t len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;

	len += sizeof(wmi_channel) * chan_list->num_scan_chans;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("Failed to allocate memory");
		qdf_status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_scan_chan_list_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_scan_chan_list_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_scan_chan_list_cmd_fixed_param));

	WMI_LOGD("no of channels = %d, len = %d", chan_list->num_scan_chans, len);

	cmd->num_scan_chans = chan_list->num_scan_chans;
	WMITLV_SET_HDR((buf_ptr + sizeof(wmi_scan_chan_list_cmd_fixed_param)),
		       WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_channel) * chan_list->num_scan_chans);
	chan_info = (wmi_channel_param *)
			(buf_ptr + sizeof(*cmd) + WMI_TLV_HDR_SIZE);
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
		WMI_LOGD("chan[%d] = %u", i, chan_info->mhz);

		/*TODO: Set WMI_SET_CHANNEL_MIN_POWER */
		/*TODO: Set WMI_SET_CHANNEL_ANTENNA_MAX */
		/*TODO: WMI_SET_CHANNEL_REG_CLASSID */
		tchan_info++;
		chan_info++;
	}
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							chan_list->pdev_id);

	qdf_status = wmi_unified_cmd_send(wmi_handle,
			buf, len, WMI_SCAN_CHAN_LIST_CMDID);

	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		WMI_LOGE("Failed to send WMI_SCAN_CHAN_LIST_CMDID");
		wmi_buf_free(buf);
	}

end:
	return qdf_status;
}
#else
static QDF_STATUS send_scan_chan_list_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_chan_list_params *chan_list)
{
	wmi_buf_t buf;
	QDF_STATUS qdf_status;
	wmi_scan_chan_list_cmd_fixed_param *cmd;
	int i;
	uint8_t *buf_ptr;
	wmi_channel *chan_info;
	struct channel_param *tchan_info;
	uint16_t len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;

	len += sizeof(wmi_channel) * chan_list->nallchans;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("Failed to allocate memory");
		qdf_status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_scan_chan_list_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_scan_chan_list_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_scan_chan_list_cmd_fixed_param));

	WMI_LOGD("no of channels = %d, len = %d", chan_list->nallchans, len);

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							chan_list->pdev_id);
	cmd->num_scan_chans = chan_list->nallchans;
	WMITLV_SET_HDR((buf_ptr + sizeof(wmi_scan_chan_list_cmd_fixed_param)),
		       WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_channel) * chan_list->nallchans);
	chan_info = (wmi_channel *) (buf_ptr + sizeof(*cmd) + WMI_TLV_HDR_SIZE);
	tchan_info = &(chan_list->ch_param[0]);

	for (i = 0; i < chan_list->nallchans; ++i) {
		WMITLV_SET_HDR(&chan_info->tlv_header,
			       WMITLV_TAG_STRUC_wmi_channel,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
		chan_info->mhz = tchan_info->mhz;
		chan_info->band_center_freq1 =
				 tchan_info->cfreq1;
		chan_info->band_center_freq2 =
				tchan_info->cfreq2;

		if (tchan_info->is_chan_passive)
			WMI_SET_CHANNEL_FLAG(chan_info,
					WMI_CHAN_FLAG_PASSIVE);

		if (tchan_info->allow_vht)
			WMI_SET_CHANNEL_FLAG(chan_info,
					WMI_CHAN_FLAG_ALLOW_VHT);
		else  if (tchan_info->allow_ht)
			WMI_SET_CHANNEL_FLAG(chan_info,
					WMI_CHAN_FLAG_ALLOW_HT);
		WMI_SET_CHANNEL_MODE(chan_info,
				tchan_info->phy_mode);

		/* Add tchan_info->half_rate and tchan_info->quarter_rate later
		 * after FW support
		 */

		/* also fill in power information */
		WMI_SET_CHANNEL_MIN_POWER(chan_info,
				tchan_info->minpower);
		WMI_SET_CHANNEL_MAX_POWER(chan_info,
				tchan_info->maxpower);
		WMI_SET_CHANNEL_REG_POWER(chan_info,
				tchan_info->maxregpower);
		WMI_SET_CHANNEL_ANTENNA_MAX(chan_info,
				tchan_info->antennamax);
		WMI_SET_CHANNEL_REG_CLASSID(chan_info,
				tchan_info->reg_class_id);

		WMI_LOGD("chan[%d] = %u", i, chan_info->mhz);

		tchan_info++;
		chan_info++;
	}
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							chan_list->pdev_id);

	qdf_status = wmi_unified_cmd_send(
			wmi_handle,
			buf, len, WMI_SCAN_CHAN_LIST_CMDID);

	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		WMI_LOGE("Failed to send WMI_SCAN_CHAN_LIST_CMDID");
		wmi_buf_free(buf);
	}

end:
	return qdf_status;
}
#endif
/**
 *  send_mgmt_cmd_tlv() - WMI scan start function
 *  @wmi_handle      : handle to WMI.
 *  @param    : pointer to hold mgmt cmd parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_mgmt_cmd_tlv(wmi_unified_t wmi_handle,
				struct wmi_mgmt_params *param)
{
	wmi_buf_t buf;
	wmi_mgmt_tx_send_cmd_fixed_param *cmd;
	int32_t cmd_len;
	uint64_t dma_addr;
	void *qdf_ctx = param->qdf_ctx;
	uint8_t *bufp;
	QDF_STATUS status;
	int32_t bufp_len = (param->frm_len < mgmt_tx_dl_frm_len) ? param->frm_len :
		mgmt_tx_dl_frm_len;

	cmd_len = sizeof(wmi_mgmt_tx_send_cmd_fixed_param) +
		WMI_TLV_HDR_SIZE + roundup(bufp_len, sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, cmd_len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_mgmt_tx_send_cmd_fixed_param *)wmi_buf_data(buf);
	bufp = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_mgmt_tx_send_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_mgmt_tx_send_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;

	cmd->desc_id = param->desc_id;
	cmd->chanfreq = param->chanfreq;
	bufp += sizeof(wmi_mgmt_tx_send_cmd_fixed_param);
	WMITLV_SET_HDR(bufp, WMITLV_TAG_ARRAY_BYTE, roundup(bufp_len,
							    sizeof(uint32_t)));
	bufp += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(bufp, param->pdata, bufp_len);

	status = qdf_nbuf_map_single(qdf_ctx, param->tx_frame,
				     QDF_DMA_TO_DEVICE);
	if (status != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s: wmi buf map failed", __func__);
		goto err1;
	}

	dma_addr = qdf_nbuf_get_frag_paddr(param->tx_frame, 0);
	cmd->paddr_lo = (uint32_t)(dma_addr & 0xffffffff);
#if defined(HTT_PADDR64)
	cmd->paddr_hi = (uint32_t)((dma_addr >> 32) & 0x1F);
#endif
	cmd->frame_len = param->frm_len;
	cmd->buf_len = bufp_len;

	wmi_mgmt_cmd_record(wmi_handle, WMI_MGMT_TX_SEND_CMDID,
			bufp, cmd->vdev_id, cmd->chanfreq);

	if (wmi_unified_cmd_send(wmi_handle, buf, cmd_len,
				      WMI_MGMT_TX_SEND_CMDID)) {
		WMI_LOGE("%s: Failed to send mgmt Tx", __func__);
		goto err1;
	}
	return QDF_STATUS_SUCCESS;

err1:
	wmi_buf_free(buf);
	return QDF_STATUS_E_FAILURE;
}

/**
 *  send_offchan_data_tx_send_cmd_tlv() - Send off-chan tx data
 *  @wmi_handle      : handle to WMI.
 *  @param    : pointer to offchan data tx cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS  on success and error on failure.
 */
static QDF_STATUS send_offchan_data_tx_cmd_tlv(wmi_unified_t wmi_handle,
				struct wmi_offchan_data_tx_params *param)
{
	wmi_buf_t buf;
	wmi_offchan_data_tx_send_cmd_fixed_param *cmd;
	int32_t cmd_len;
	uint64_t dma_addr;
	void *qdf_ctx = param->qdf_ctx;
	uint8_t *bufp;
	int32_t bufp_len = (param->frm_len < mgmt_tx_dl_frm_len) ?
					param->frm_len : mgmt_tx_dl_frm_len;

	cmd_len = sizeof(wmi_offchan_data_tx_send_cmd_fixed_param) +
		WMI_TLV_HDR_SIZE + roundup(bufp_len, sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, cmd_len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_offchan_data_tx_send_cmd_fixed_param *) wmi_buf_data(buf);
	bufp = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_offchan_data_tx_send_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_offchan_data_tx_send_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;

	cmd->desc_id = param->desc_id;
	cmd->chanfreq = param->chanfreq;
	bufp += sizeof(wmi_offchan_data_tx_send_cmd_fixed_param);
	WMITLV_SET_HDR(bufp, WMITLV_TAG_ARRAY_BYTE, roundup(bufp_len,
							    sizeof(uint32_t)));
	bufp += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(bufp, param->pdata, bufp_len);
	qdf_nbuf_map_single(qdf_ctx, param->tx_frame, QDF_DMA_TO_DEVICE);
	dma_addr = qdf_nbuf_get_frag_paddr(param->tx_frame, 0);
	cmd->paddr_lo = (uint32_t)(dma_addr & 0xffffffff);
#if defined(HTT_PADDR64)
	cmd->paddr_hi = (uint32_t)((dma_addr >> 32) & 0x1F);
#endif
	cmd->frame_len = param->frm_len;
	cmd->buf_len = bufp_len;

	wmi_mgmt_cmd_record(wmi_handle, WMI_OFFCHAN_DATA_TX_SEND_CMDID,
			bufp, cmd->vdev_id, cmd->chanfreq);

	if (wmi_unified_cmd_send(wmi_handle, buf, cmd_len,
				WMI_OFFCHAN_DATA_TX_SEND_CMDID)) {
		WMI_LOGE("%s: Failed to offchan data Tx", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_modem_power_state_cmd_tlv() - set modem power state to fw
 * @wmi_handle: wmi handle
 * @param_value: parameter value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_modem_power_state_cmd_tlv(wmi_unified_t wmi_handle,
		uint32_t param_value)
{
	QDF_STATUS ret;
	wmi_modem_power_state_cmd_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_modem_power_state_cmd_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_modem_power_state_cmd_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_modem_power_state_cmd_param));
	cmd->modem_power_state = param_value;
	WMI_LOGD("%s: Setting cmd->modem_power_state = %u", __func__,
		 param_value);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				     WMI_MODEM_POWER_STATE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send notify cmd ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_sta_ps_mode_cmd_tlv() - set sta powersave mode in fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @val: value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
static QDF_STATUS send_set_sta_ps_mode_cmd_tlv(wmi_unified_t wmi_handle,
			       uint32_t vdev_id, uint8_t val)
{
	wmi_sta_powersave_mode_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	WMI_LOGD("Set Sta Mode Ps vdevId %d val %d", vdev_id, val);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: Set Sta Mode Ps Mem Alloc Failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_sta_powersave_mode_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sta_powersave_mode_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_sta_powersave_mode_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	if (val)
		cmd->sta_ps_mode = WMI_STA_PS_MODE_ENABLED;
	else
		cmd->sta_ps_mode = WMI_STA_PS_MODE_DISABLED;

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_STA_POWERSAVE_MODE_CMDID)) {
		WMI_LOGE("Set Sta Mode Ps Failed vdevId %d val %d",
			 vdev_id, val);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return 0;
}

/**
 * send_set_mimops_cmd_tlv() - set MIMO powersave
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
static QDF_STATUS send_set_mimops_cmd_tlv(wmi_unified_t wmi_handle,
			uint8_t vdev_id, int value)
{
	QDF_STATUS ret;
	wmi_sta_smps_force_mode_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_sta_smps_force_mode_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sta_smps_force_mode_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_sta_smps_force_mode_cmd_fixed_param));

	cmd->vdev_id = vdev_id;

	/* WMI_SMPS_FORCED_MODE values do not directly map
	 * to SM power save values defined in the specification.
	 * Make sure to send the right mapping.
	 */
	switch (value) {
	case 0:
		cmd->forced_mode = WMI_SMPS_FORCED_MODE_NONE;
		break;
	case 1:
		cmd->forced_mode = WMI_SMPS_FORCED_MODE_DISABLED;
		break;
	case 2:
		cmd->forced_mode = WMI_SMPS_FORCED_MODE_STATIC;
		break;
	case 3:
		cmd->forced_mode = WMI_SMPS_FORCED_MODE_DYNAMIC;
		break;
	default:
		WMI_LOGE("%s:INVALID Mimo PS CONFIG", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	WMI_LOGD("Setting vdev %d value = %u", vdev_id, value);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_STA_SMPS_FORCE_MODE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send set Mimo PS ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_smps_params_cmd_tlv() - set smps params
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @value: value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
static QDF_STATUS send_set_smps_params_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id,
			       int value)
{
	QDF_STATUS ret;
	wmi_sta_smps_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_sta_smps_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sta_smps_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_sta_smps_param_cmd_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->value = value & WMI_SMPS_MASK_LOWER_16BITS;
	cmd->param =
		(value >> WMI_SMPS_PARAM_VALUE_S) & WMI_SMPS_MASK_UPPER_3BITS;

	WMI_LOGD("Setting vdev %d value = %x param %x", vdev_id, cmd->value,
		 cmd->param);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_STA_SMPS_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send set Mimo PS ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

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

	WMI_LOGD("%s: Enter", __func__);
	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE + sizeof(*noa_discriptor);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("Failed to allocate memory");
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_p2p_set_noa_cmd_fixed_param *) buf_ptr;
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
	noa_discriptor = (wmi_p2p_noa_descriptor *) (buf_ptr +
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

	WMI_LOGI("SET P2P GO NOA:vdev_id:%d count:%d duration:%d interval:%d",
		 cmd->vdev_id, noa->count, noa_discriptor->duration,
		 noa->interval);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_FWTEST_P2P_SET_NOA_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("Failed to send WMI_FWTEST_P2P_SET_NOA_PARAM_CMDID");
		wmi_buf_free(buf);
	}

end:
	WMI_LOGD("%s: Exit", __func__);
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

	WMI_LOGD("%s: Enter", __func__);
	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGE("Failed to allocate memory");
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	cmd = (wmi_p2p_set_oppps_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_p2p_set_oppps_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_p2p_set_oppps_cmd_fixed_param));
	cmd->vdev_id = oppps->session_id;
	if (oppps->ctwindow)
		WMI_UNIFIED_OPPPS_ATTR_ENABLED_SET(cmd);

	WMI_UNIFIED_OPPPS_ATTR_CTWIN_SET(cmd, oppps->ctwindow);
	WMI_LOGI("SET P2P GO OPPPS:vdev_id:%d ctwindow:%d",
		 cmd->vdev_id, oppps->ctwindow);
	status = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				      WMI_P2P_SET_OPPPS_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("Failed to send WMI_P2P_SET_OPPPS_PARAM_CMDID");
		wmi_buf_free(buf);
	}

end:
	WMI_LOGD("%s: Exit", __func__);
	return status;
}

#ifdef CONVERGED_P2P_ENABLE
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
		WMI_LOGE("lo start param is null");
		return QDF_STATUS_E_INVAL;
	}

	WMI_LOGD("%s: vdev_id:%d", __func__, param->vdev_id);

	device_types_len_aligned =
		qdf_roundup(param->dev_types_len,
			sizeof(A_UINT32));
	probe_resp_len_aligned =
		qdf_roundup(param->probe_resp_len,
			sizeof(A_UINT32));

	len += 2 * WMI_TLV_HDR_SIZE + device_types_len_aligned +
			probe_resp_len_aligned;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed to allocate memory for p2p lo start",
			__func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_p2p_lo_start_cmd_fixed_param *)wmi_buf_data(buf);
	buf_ptr = (uint8_t *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		 WMITLV_TAG_STRUC_wmi_p2p_lo_start_cmd_fixed_param,
		 WMITLV_GET_STRUCT_TLVLEN(
			wmi_p2p_lo_start_cmd_fixed_param));

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

	WMI_LOGD("%s: Sending WMI_P2P_LO_START command, channel=%d, period=%d, interval=%d, count=%d", __func__,
	cmd->channel, cmd->period, cmd->interval, cmd->count);

	status = wmi_unified_cmd_send(wmi_handle,
				buf, len,
				WMI_P2P_LISTEN_OFFLOAD_START_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s: Failed to send p2p lo start: %d",
			__func__, status);
		wmi_buf_free(buf);
		return status;
	}

	WMI_LOGD("%s: Successfully sent WMI_P2P_LO_START", __func__);

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

	WMI_LOGD("%s: vdev_id:%d", __func__, vdev_id);

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: Failed to allocate memory for p2p lo stop",
			__func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_p2p_lo_stop_cmd_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_p2p_lo_stop_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_p2p_lo_stop_cmd_fixed_param));

	cmd->vdev_id = vdev_id;

	WMI_LOGD("%s: Sending WMI_P2P_LO_STOP command", __func__);

	status = wmi_unified_cmd_send(wmi_handle,
				buf, len,
				WMI_P2P_LISTEN_OFFLOAD_STOP_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s: Failed to send p2p lo stop: %d",
			__func__, status);
		wmi_buf_free(buf);
		return status;
	}

	WMI_LOGD("%s: Successfully sent WMI_P2P_LO_STOP", __func__);

	return QDF_STATUS_SUCCESS;
}
#endif /* End of CONVERGED_P2P_ENABLE */

/**
 * send_get_temperature_cmd_tlv() - get pdev temperature req
 * @wmi_handle: wmi handle
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
static QDF_STATUS send_get_temperature_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_pdev_get_temperature_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len = sizeof(wmi_pdev_get_temperature_cmd_fixed_param);
	uint8_t *buf_ptr;

	if (!wmi_handle) {
		WMI_LOGE(FL("WMI is closed, can not issue cmd"));
		return QDF_STATUS_E_INVAL;
	}

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_pdev_get_temperature_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_get_temperature_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_get_temperature_cmd_fixed_param));

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_PDEV_GET_TEMPERATURE_CMDID)) {
		WMI_LOGE(FL("failed to send get temperature command"));
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_sta_uapsd_auto_trig_cmd_tlv() - set uapsd auto trigger command
 * @wmi_handle: wmi handle
 * @vdevid: vdev id
 * @peer_addr: peer mac address
 * @auto_triggerparam: auto trigger parameters
 * @num_ac: number of access category
 *
 * This function sets the trigger
 * uapsd params such as service interval, delay interval
 * and suspend interval which will be used by the firmware
 * to send trigger frames periodically when there is no
 * traffic on the transmit side.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code.
 */
static QDF_STATUS send_set_sta_uapsd_auto_trig_cmd_tlv(wmi_unified_t wmi_handle,
				struct sta_uapsd_trig_params *param)
{
	wmi_sta_uapsd_auto_trig_cmd_fixed_param *cmd;
	QDF_STATUS ret;
	uint32_t param_len = param->num_ac * sizeof(wmi_sta_uapsd_auto_trig_param);
	uint32_t cmd_len = sizeof(*cmd) + param_len + WMI_TLV_HDR_SIZE;
	uint32_t i;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	struct sta_uapsd_params *uapsd_param;
	wmi_sta_uapsd_auto_trig_param *trig_param;

	buf = wmi_buf_alloc(wmi_handle, cmd_len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_sta_uapsd_auto_trig_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sta_uapsd_auto_trig_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_sta_uapsd_auto_trig_cmd_fixed_param));
	cmd->vdev_id = param->vdevid;
	cmd->num_ac = param->num_ac;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_addr, &cmd->peer_macaddr);

	/* TLV indicating array of structures to follow */
	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, param_len);

	buf_ptr += WMI_TLV_HDR_SIZE;

	/*
	 * Update tag and length for uapsd auto trigger params (this will take
	 * care of updating tag and length if it is not pre-filled by caller).
	 */
	uapsd_param = (struct sta_uapsd_params *)param->auto_triggerparam;
	trig_param = (wmi_sta_uapsd_auto_trig_param *)buf_ptr;
	for (i = 0; i < param->num_ac; i++) {
		WMITLV_SET_HDR((buf_ptr +
				(i * sizeof(wmi_sta_uapsd_auto_trig_param))),
			       WMITLV_TAG_STRUC_wmi_sta_uapsd_auto_trig_param,
			       WMITLV_GET_STRUCT_TLVLEN
				       (wmi_sta_uapsd_auto_trig_param));
		trig_param->wmm_ac = uapsd_param->wmm_ac;
		trig_param->user_priority = uapsd_param->user_priority;
		trig_param->service_interval = uapsd_param->service_interval;
		trig_param->suspend_interval = uapsd_param->suspend_interval;
		trig_param->delay_interval = uapsd_param->delay_interval;
		trig_param++;
		uapsd_param++;
	}

	ret = wmi_unified_cmd_send(wmi_handle, buf, cmd_len,
				   WMI_STA_UAPSD_AUTO_TRIG_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send set uapsd param ret = %d", ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_ocb_set_utc_time_cmd() - send the UTC time to the firmware
 * @wmi_handle: pointer to the wmi handle
 * @utc: pointer to the UTC time struct
 *
 * Return: 0 on succes
 */
static QDF_STATUS send_ocb_set_utc_time_cmd_tlv(wmi_unified_t wmi_handle,
				struct ocb_utc_param *utc)
{
	QDF_STATUS ret;
	wmi_ocb_set_utc_time_cmd_fixed_param *cmd;
	uint8_t *buf_ptr;
	uint32_t len, i;
	wmi_buf_t buf;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_ocb_set_utc_time_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_ocb_set_utc_time_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_ocb_set_utc_time_cmd_fixed_param));
	cmd->vdev_id = utc->vdev_id;

	for (i = 0; i < SIZE_UTC_TIME; i++)
		WMI_UTC_TIME_SET(cmd, i, utc->utc_time[i]);

	for (i = 0; i < SIZE_UTC_TIME_ERROR; i++)
		WMI_TIME_ERROR_SET(cmd, i, utc->time_error[i]);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_OCB_SET_UTC_TIME_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL("Failed to set OCB UTC time"));
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_ocb_start_timing_advert_cmd_tlv() - start sending the timing advertisement
 *				   frames on a channel
 * @wmi_handle: pointer to the wmi handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: 0 on succes
 */
static QDF_STATUS send_ocb_start_timing_advert_cmd_tlv(wmi_unified_t wmi_handle,
	struct ocb_timing_advert_param *timing_advert)
{
	QDF_STATUS ret;
	wmi_ocb_start_timing_advert_cmd_fixed_param *cmd;
	uint8_t *buf_ptr;
	uint32_t len, len_template;
	wmi_buf_t buf;

	len = sizeof(*cmd) +
		     WMI_TLV_HDR_SIZE;

	len_template = timing_advert->template_length;
	/* Add padding to the template if needed */
	if (len_template % 4 != 0)
		len_template += 4 - (len_template % 4);
	len += len_template;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_ocb_start_timing_advert_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_ocb_start_timing_advert_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_ocb_start_timing_advert_cmd_fixed_param));
	cmd->vdev_id = timing_advert->vdev_id;
	cmd->repeat_rate = timing_advert->repeat_rate;
	cmd->channel_freq = timing_advert->chan_freq;
	cmd->timestamp_offset = timing_advert->timestamp_offset;
	cmd->time_value_offset = timing_advert->time_value_offset;
	cmd->timing_advert_template_length = timing_advert->template_length;
	buf_ptr += sizeof(*cmd);

	/* Add the timing advert template */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
		       len_template);
	qdf_mem_copy(buf_ptr + WMI_TLV_HDR_SIZE,
		     (uint8_t *)timing_advert->template_value,
		     timing_advert->template_length);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_OCB_START_TIMING_ADVERT_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL("Failed to start OCB timing advert"));
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_ocb_stop_timing_advert_cmd_tlv() - stop sending the timing advertisement frames
 *				  on a channel
 * @wmi_handle: pointer to the wmi handle
 * @timing_advert: pointer to the timing advertisement struct
 *
 * Return: 0 on succes
 */
static QDF_STATUS send_ocb_stop_timing_advert_cmd_tlv(wmi_unified_t wmi_handle,
	struct ocb_timing_advert_param *timing_advert)
{
	QDF_STATUS ret;
	wmi_ocb_stop_timing_advert_cmd_fixed_param *cmd;
	uint8_t *buf_ptr;
	uint32_t len;
	wmi_buf_t buf;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_ocb_stop_timing_advert_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_ocb_stop_timing_advert_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_ocb_stop_timing_advert_cmd_fixed_param));
	cmd->vdev_id = timing_advert->vdev_id;
	cmd->channel_freq = timing_advert->chan_freq;

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_OCB_STOP_TIMING_ADVERT_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL("Failed to stop OCB timing advert"));
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_ocb_get_tsf_timer_cmd_tlv() - get ocb tsf timer val
 * @wmi_handle: pointer to the wmi handle
 * @request: pointer to the request
 *
 * Return: 0 on succes
 */
static QDF_STATUS send_ocb_get_tsf_timer_cmd_tlv(wmi_unified_t wmi_handle,
			  uint8_t vdev_id)
{
	QDF_STATUS ret;
	wmi_ocb_get_tsf_timer_cmd_fixed_param *cmd;
	uint8_t *buf_ptr;
	wmi_buf_t buf;
	int32_t len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *)wmi_buf_data(buf);

	cmd = (wmi_ocb_get_tsf_timer_cmd_fixed_param *)buf_ptr;
	qdf_mem_zero(cmd, len);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_ocb_get_tsf_timer_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_ocb_get_tsf_timer_cmd_fixed_param));
	cmd->vdev_id = vdev_id;

	/* Send the WMI command */
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_OCB_GET_TSF_TIMER_CMDID);
	/* If there is an error, set the completion event */
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL("Failed to send WMI message: %d"), ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_dcc_get_stats_cmd_tlv() - get the DCC channel stats
 * @wmi_handle: pointer to the wmi handle
 * @get_stats_param: pointer to the dcc stats
 *
 * Return: 0 on succes
 */
static QDF_STATUS send_dcc_get_stats_cmd_tlv(wmi_unified_t wmi_handle,
		     struct dcc_get_stats_param *get_stats_param)
{
	QDF_STATUS ret;
	wmi_dcc_get_stats_cmd_fixed_param *cmd;
	wmi_dcc_channel_stats_request *channel_stats_array;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len;
	uint32_t i;

	/* Validate the input */
	if (get_stats_param->request_array_len !=
	    get_stats_param->channel_count * sizeof(*channel_stats_array)) {
		WMI_LOGE(FL("Invalid parameter"));
		return QDF_STATUS_E_INVAL;
	}

	/* Allocate memory for the WMI command */
	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE +
		get_stats_param->request_array_len;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);

	/* Populate the WMI command */
	cmd = (wmi_dcc_get_stats_cmd_fixed_param *)buf_ptr;
	buf_ptr += sizeof(*cmd);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_dcc_get_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			   wmi_dcc_get_stats_cmd_fixed_param));
	cmd->vdev_id = get_stats_param->vdev_id;
	cmd->num_channels = get_stats_param->channel_count;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       get_stats_param->request_array_len);
	buf_ptr += WMI_TLV_HDR_SIZE;

	channel_stats_array = (wmi_dcc_channel_stats_request *)buf_ptr;
	qdf_mem_copy(channel_stats_array, get_stats_param->request_array,
		     get_stats_param->request_array_len);
	for (i = 0; i < cmd->num_channels; i++)
		WMITLV_SET_HDR(&channel_stats_array[i].tlv_header,
			WMITLV_TAG_STRUC_wmi_dcc_channel_stats_request,
			WMITLV_GET_STRUCT_TLVLEN(
			    wmi_dcc_channel_stats_request));

	/* Send the WMI command */
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_DCC_GET_STATS_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL("Failed to send WMI message: %d"), ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_dcc_clear_stats_cmd_tlv() - command to clear the DCC stats
 * @wmi_handle: pointer to the wmi handle
 * @vdev_id: vdev id
 * @dcc_stats_bitmap: dcc status bitmap
 *
 * Return: 0 on succes
 */
static QDF_STATUS send_dcc_clear_stats_cmd_tlv(wmi_unified_t wmi_handle,
				uint32_t vdev_id, uint32_t dcc_stats_bitmap)
{
	QDF_STATUS ret;
	wmi_dcc_clear_stats_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len;

	/* Allocate memory for the WMI command */
	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);

	/* Populate the WMI command */
	cmd = (wmi_dcc_clear_stats_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_dcc_clear_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			   wmi_dcc_clear_stats_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->dcc_stats_bitmap = dcc_stats_bitmap;

	/* Send the WMI command */
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_DCC_CLEAR_STATS_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL("Failed to send the WMI command"));
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_dcc_update_ndl_cmd_tlv() - command to update the NDL data
 * @wmi_handle: pointer to the wmi handle
 * @update_ndl_param: pointer to the request parameters
 *
 * Return: 0 on success
 */
static QDF_STATUS send_dcc_update_ndl_cmd_tlv(wmi_unified_t wmi_handle,
		       struct dcc_update_ndl_param *update_ndl_param)
{
	QDF_STATUS qdf_status;
	wmi_dcc_update_ndl_cmd_fixed_param *cmd;
	wmi_dcc_ndl_chan *ndl_chan_array;
	wmi_dcc_ndl_active_state_config *ndl_active_state_array;
	uint32_t active_state_count;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len;
	uint32_t i;

	/* validate the input */
	if (update_ndl_param->dcc_ndl_chan_list_len !=
	    update_ndl_param->channel_count * sizeof(*ndl_chan_array)) {
		WMI_LOGE(FL("Invalid parameter"));
		return QDF_STATUS_E_INVAL;
	}
	active_state_count = 0;
	ndl_chan_array = update_ndl_param->dcc_ndl_chan_list;
	for (i = 0; i < update_ndl_param->channel_count; i++)
		active_state_count +=
			WMI_NDL_NUM_ACTIVE_STATE_GET(&ndl_chan_array[i]);
	if (update_ndl_param->dcc_ndl_active_state_list_len !=
	    active_state_count * sizeof(*ndl_active_state_array)) {
		WMI_LOGE(FL("Invalid parameter"));
		return QDF_STATUS_E_INVAL;
	}

	/* Allocate memory for the WMI command */
	len = sizeof(*cmd) +
		WMI_TLV_HDR_SIZE + update_ndl_param->dcc_ndl_chan_list_len +
		WMI_TLV_HDR_SIZE +
		update_ndl_param->dcc_ndl_active_state_list_len;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);

	/* Populate the WMI command */
	cmd = (wmi_dcc_update_ndl_cmd_fixed_param *)buf_ptr;
	buf_ptr += sizeof(*cmd);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_dcc_update_ndl_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			   wmi_dcc_update_ndl_cmd_fixed_param));
	cmd->vdev_id = update_ndl_param->vdev_id;
	cmd->num_channel = update_ndl_param->channel_count;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       update_ndl_param->dcc_ndl_chan_list_len);
	buf_ptr += WMI_TLV_HDR_SIZE;

	ndl_chan_array = (wmi_dcc_ndl_chan *)buf_ptr;
	qdf_mem_copy(ndl_chan_array, update_ndl_param->dcc_ndl_chan_list,
		     update_ndl_param->dcc_ndl_chan_list_len);
	for (i = 0; i < cmd->num_channel; i++)
		WMITLV_SET_HDR(&ndl_chan_array[i].tlv_header,
			WMITLV_TAG_STRUC_wmi_dcc_ndl_chan,
			WMITLV_GET_STRUCT_TLVLEN(
			    wmi_dcc_ndl_chan));
	buf_ptr += update_ndl_param->dcc_ndl_chan_list_len;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       update_ndl_param->dcc_ndl_active_state_list_len);
	buf_ptr += WMI_TLV_HDR_SIZE;

	ndl_active_state_array = (wmi_dcc_ndl_active_state_config *) buf_ptr;
	qdf_mem_copy(ndl_active_state_array,
		     update_ndl_param->dcc_ndl_active_state_list,
		     update_ndl_param->dcc_ndl_active_state_list_len);
	for (i = 0; i < active_state_count; i++) {
		WMITLV_SET_HDR(&ndl_active_state_array[i].tlv_header,
			WMITLV_TAG_STRUC_wmi_dcc_ndl_active_state_config,
			WMITLV_GET_STRUCT_TLVLEN(
			    wmi_dcc_ndl_active_state_config));
	}
	buf_ptr += update_ndl_param->dcc_ndl_active_state_list_len;

	/* Send the WMI command */
	qdf_status = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_DCC_UPDATE_NDL_CMDID);
	/* If there is an error, set the completion event */
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		WMI_LOGE(FL("Failed to send WMI message: %d"), qdf_status);
		wmi_buf_free(buf);
	}

	return qdf_status;
}

/**
 * send_ocb_set_config_cmd_tlv() - send the OCB config to the FW
 * @wmi_handle: pointer to the wmi handle
 * @config: the OCB configuration
 *
 * Return: 0 on success
 */
static QDF_STATUS send_ocb_set_config_cmd_tlv(wmi_unified_t wmi_handle,
				struct ocb_config_param *config, uint32_t *ch_mhz)
{
	QDF_STATUS ret;
	wmi_ocb_set_config_cmd_fixed_param *cmd;
	wmi_channel *chan;
	wmi_ocb_channel *ocb_chan;
	wmi_qos_parameter *qos_param;
	wmi_dcc_ndl_chan *ndl_chan;
	wmi_dcc_ndl_active_state_config *ndl_active_config;
	wmi_ocb_schedule_element *sched_elem;
	uint8_t *buf_ptr;
	wmi_buf_t buf;
	int32_t len;
	int32_t i, j, active_state_count;

	/*
	 * Validate the dcc_ndl_chan_list_len and count the number of active
	 * states. Validate dcc_ndl_active_state_list_len.
	 */
	active_state_count = 0;
	if (config->dcc_ndl_chan_list_len) {
		if (!config->dcc_ndl_chan_list ||
			config->dcc_ndl_chan_list_len !=
			config->channel_count * sizeof(wmi_dcc_ndl_chan)) {
			WMI_LOGE(FL("NDL channel is invalid. List len: %d"),
				 config->dcc_ndl_chan_list_len);
			return QDF_STATUS_E_INVAL;
		}

		for (i = 0, ndl_chan = config->dcc_ndl_chan_list;
				i < config->channel_count; ++i, ++ndl_chan)
			active_state_count +=
				WMI_NDL_NUM_ACTIVE_STATE_GET(ndl_chan);

		if (active_state_count) {
			if (!config->dcc_ndl_active_state_list ||
				config->dcc_ndl_active_state_list_len !=
				active_state_count *
				sizeof(wmi_dcc_ndl_active_state_config)) {
				WMI_LOGE(FL("NDL active state is invalid."));
				return QDF_STATUS_E_INVAL;
			}
		}
	}

	len = sizeof(*cmd) +
		WMI_TLV_HDR_SIZE + config->channel_count *
			sizeof(wmi_channel) +
		WMI_TLV_HDR_SIZE + config->channel_count *
			sizeof(wmi_ocb_channel) +
		WMI_TLV_HDR_SIZE + config->channel_count *
			sizeof(wmi_qos_parameter) * WMI_MAX_NUM_AC +
		WMI_TLV_HDR_SIZE + config->dcc_ndl_chan_list_len +
		WMI_TLV_HDR_SIZE + active_state_count *
			sizeof(wmi_dcc_ndl_active_state_config) +
		WMI_TLV_HDR_SIZE + config->schedule_size *
			sizeof(wmi_ocb_schedule_element);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_ocb_set_config_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_ocb_set_config_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_ocb_set_config_cmd_fixed_param));
	cmd->vdev_id = config->session_id;
	cmd->channel_count = config->channel_count;
	cmd->schedule_size = config->schedule_size;
	cmd->flags = config->flags;
	buf_ptr += sizeof(*cmd);

	/* Add the wmi_channel info */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       config->channel_count*sizeof(wmi_channel));
	buf_ptr += WMI_TLV_HDR_SIZE;
	for (i = 0; i < config->channel_count; i++) {
		chan = (wmi_channel *)buf_ptr;
		WMITLV_SET_HDR(&chan->tlv_header,
				WMITLV_TAG_STRUC_wmi_channel,
				WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
		chan->mhz = config->channels[i].chan_freq;
		chan->band_center_freq1 = config->channels[i].chan_freq;
		chan->band_center_freq2 = 0;
		chan->info = 0;

		WMI_SET_CHANNEL_MODE(chan, ch_mhz[i]);
		WMI_SET_CHANNEL_MAX_POWER(chan, config->channels[i].max_pwr);
		WMI_SET_CHANNEL_MIN_POWER(chan, config->channels[i].min_pwr);
		WMI_SET_CHANNEL_MAX_TX_POWER(chan, config->channels[i].max_pwr);
		WMI_SET_CHANNEL_REG_POWER(chan, config->channels[i].reg_pwr);
		WMI_SET_CHANNEL_ANTENNA_MAX(chan,
					    config->channels[i].antenna_max);

		if (config->channels[i].bandwidth < 10)
			WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_QUARTER_RATE);
		else if (config->channels[i].bandwidth < 20)
			WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_HALF_RATE);
		buf_ptr += sizeof(*chan);
	}

	/* Add the wmi_ocb_channel info */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       config->channel_count*sizeof(wmi_ocb_channel));
	buf_ptr += WMI_TLV_HDR_SIZE;
	for (i = 0; i < config->channel_count; i++) {
		ocb_chan = (wmi_ocb_channel *)buf_ptr;
		WMITLV_SET_HDR(&ocb_chan->tlv_header,
			       WMITLV_TAG_STRUC_wmi_ocb_channel,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_ocb_channel));
		ocb_chan->bandwidth = config->channels[i].bandwidth;
		WMI_CHAR_ARRAY_TO_MAC_ADDR(
					config->channels[i].mac_address.bytes,
					&ocb_chan->mac_address);
		buf_ptr += sizeof(*ocb_chan);
	}

	/* Add the wmi_qos_parameter info */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		config->channel_count * sizeof(wmi_qos_parameter)*WMI_MAX_NUM_AC);
	buf_ptr += WMI_TLV_HDR_SIZE;
	/* WMI_MAX_NUM_AC parameters for each channel */
	for (i = 0; i < config->channel_count; i++) {
		for (j = 0; j < WMI_MAX_NUM_AC; j++) {
			qos_param = (wmi_qos_parameter *)buf_ptr;
			WMITLV_SET_HDR(&qos_param->tlv_header,
				WMITLV_TAG_STRUC_wmi_qos_parameter,
				WMITLV_GET_STRUCT_TLVLEN(wmi_qos_parameter));
			qos_param->aifsn =
				config->channels[i].qos_params[j].aifsn;
			qos_param->cwmin =
				config->channels[i].qos_params[j].cwmin;
			qos_param->cwmax =
				config->channels[i].qos_params[j].cwmax;
			buf_ptr += sizeof(*qos_param);
		}
	}

	/* Add the wmi_dcc_ndl_chan (per channel) */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       config->dcc_ndl_chan_list_len);
	buf_ptr += WMI_TLV_HDR_SIZE;
	if (config->dcc_ndl_chan_list_len) {
		ndl_chan = (wmi_dcc_ndl_chan *)buf_ptr;
		qdf_mem_copy(ndl_chan, config->dcc_ndl_chan_list,
			     config->dcc_ndl_chan_list_len);
		for (i = 0; i < config->channel_count; i++)
			WMITLV_SET_HDR(&(ndl_chan[i].tlv_header),
				WMITLV_TAG_STRUC_wmi_dcc_ndl_chan,
				WMITLV_GET_STRUCT_TLVLEN(wmi_dcc_ndl_chan));
		buf_ptr += config->dcc_ndl_chan_list_len;
	}

	/* Add the wmi_dcc_ndl_active_state_config */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, active_state_count *
		       sizeof(wmi_dcc_ndl_active_state_config));
	buf_ptr += WMI_TLV_HDR_SIZE;
	if (active_state_count) {
		ndl_active_config = (wmi_dcc_ndl_active_state_config *)buf_ptr;
		qdf_mem_copy(ndl_active_config,
			config->dcc_ndl_active_state_list,
			active_state_count * sizeof(*ndl_active_config));
		for (i = 0; i < active_state_count; ++i)
			WMITLV_SET_HDR(&(ndl_active_config[i].tlv_header),
			  WMITLV_TAG_STRUC_wmi_dcc_ndl_active_state_config,
			  WMITLV_GET_STRUCT_TLVLEN(
				wmi_dcc_ndl_active_state_config));
		buf_ptr += active_state_count *
			sizeof(*ndl_active_config);
	}

	/* Add the wmi_ocb_schedule_element info */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		config->schedule_size * sizeof(wmi_ocb_schedule_element));
	buf_ptr += WMI_TLV_HDR_SIZE;
	for (i = 0; i < config->schedule_size; i++) {
		sched_elem = (wmi_ocb_schedule_element *)buf_ptr;
		WMITLV_SET_HDR(&sched_elem->tlv_header,
			WMITLV_TAG_STRUC_wmi_ocb_schedule_element,
			WMITLV_GET_STRUCT_TLVLEN(wmi_ocb_schedule_element));
		sched_elem->channel_freq = config->schedule[i].chan_freq;
		sched_elem->total_duration = config->schedule[i].total_duration;
		sched_elem->guard_interval = config->schedule[i].guard_interval;
		buf_ptr += sizeof(*sched_elem);
	}


	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_OCB_SET_CONFIG_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to set OCB config");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_enable_disable_mcc_adaptive_scheduler_cmd_tlv() -enable/disable mcc scheduler
 * @wmi_handle: wmi handle
 * @mcc_adaptive_scheduler: enable/disable
 *
 * This function enable/disable mcc adaptive scheduler in fw.
 *
 * Return: QDF_STATUS_SUCCESS for sucess or error code
 */
static QDF_STATUS send_set_enable_disable_mcc_adaptive_scheduler_cmd_tlv(
		wmi_unified_t wmi_handle, uint32_t mcc_adaptive_scheduler,
		uint32_t pdev_id)
{
	QDF_STATUS ret;
	wmi_buf_t buf = 0;
	wmi_resmgr_adaptive_ocs_enable_disable_cmd_fixed_param *cmd = NULL;
	uint16_t len =
		sizeof(wmi_resmgr_adaptive_ocs_enable_disable_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_resmgr_adaptive_ocs_enable_disable_cmd_fixed_param *)
		wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_resmgr_adaptive_ocs_enable_disable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_resmgr_adaptive_ocs_enable_disable_cmd_fixed_param));
	cmd->enable = mcc_adaptive_scheduler;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(pdev_id);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_RESMGR_ADAPTIVE_OCS_ENABLE_DISABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGP("%s: Failed to send enable/disable MCC"
			 " adaptive scheduler command", __func__);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_mcc_channel_time_latency_cmd_tlv() -set MCC channel time latency
 * @wmi: wmi handle
 * @mcc_channel: mcc channel
 * @mcc_channel_time_latency: MCC channel time latency.
 *
 * Currently used to set time latency for an MCC vdev/adapter using operating
 * channel of it and channel number. The info is provided run time using
 * iwpriv command: iwpriv <wlan0 | p2p0> setMccLatency <latency in ms>.
 *
 * Return: CDF status
 */
static QDF_STATUS send_set_mcc_channel_time_latency_cmd_tlv(wmi_unified_t wmi_handle,
	uint32_t mcc_channel_freq, uint32_t mcc_channel_time_latency)
{
	QDF_STATUS ret;
	wmi_buf_t buf = 0;
	wmi_resmgr_set_chan_latency_cmd_fixed_param *cmdTL = NULL;
	uint16_t len = 0;
	uint8_t *buf_ptr = NULL;
	wmi_resmgr_chan_latency chan_latency;
	/* Note: we only support MCC time latency for a single channel */
	uint32_t num_channels = 1;
	uint32_t chan1_freq = mcc_channel_freq;
	uint32_t latency_chan1 = mcc_channel_time_latency;


	/* If 0ms latency is provided, then FW will set to a default.
	 * Otherwise, latency must be at least 30ms.
	 */
	if ((latency_chan1 > 0) &&
	    (latency_chan1 < WMI_MCC_MIN_NON_ZERO_CHANNEL_LATENCY)) {
		WMI_LOGE("%s: Invalid time latency for Channel #1 = %dms "
			 "Minimum is 30ms (or 0 to use default value by "
			 "firmware)", __func__, latency_chan1);
		return QDF_STATUS_E_INVAL;
	}

	/*   Set WMI CMD for channel time latency here */
	len = sizeof(wmi_resmgr_set_chan_latency_cmd_fixed_param) +
	      WMI_TLV_HDR_SIZE +  /*Place holder for chan_time_latency array */
	      num_channels * sizeof(wmi_resmgr_chan_latency);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmdTL = (wmi_resmgr_set_chan_latency_cmd_fixed_param *)
		wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmdTL->tlv_header,
		WMITLV_TAG_STRUC_wmi_resmgr_set_chan_latency_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_resmgr_set_chan_latency_cmd_fixed_param));
	cmdTL->num_chans = num_channels;
	/* Update channel time latency information for home channel(s) */
	buf_ptr += sizeof(*cmdTL);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
		       num_channels * sizeof(wmi_resmgr_chan_latency));
	buf_ptr += WMI_TLV_HDR_SIZE;
	chan_latency.chan_mhz = chan1_freq;
	chan_latency.latency = latency_chan1;
	qdf_mem_copy(buf_ptr, &chan_latency, sizeof(chan_latency));
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_RESMGR_SET_CHAN_LATENCY_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send MCC Channel Time Latency command",
			 __func__);
		wmi_buf_free(buf);
		QDF_ASSERT(0);
	}

	return ret;
}

/**
 * send_set_mcc_channel_time_quota_cmd_tlv() -set MCC channel time quota
 * @wmi: wmi handle
 * @adapter_1_chan_number: adapter 1 channel number
 * @adapter_1_quota: adapter 1 quota
 * @adapter_2_chan_number: adapter 2 channel number
 *
 * Return: CDF status
 */
static QDF_STATUS send_set_mcc_channel_time_quota_cmd_tlv(wmi_unified_t wmi_handle,
	uint32_t adapter_1_chan_freq,
	uint32_t adapter_1_quota, uint32_t adapter_2_chan_freq)
{
	QDF_STATUS ret;
	wmi_buf_t buf = 0;
	uint16_t len = 0;
	uint8_t *buf_ptr = NULL;
	wmi_resmgr_set_chan_time_quota_cmd_fixed_param *cmdTQ = NULL;
	wmi_resmgr_chan_time_quota chan_quota;
	uint32_t quota_chan1 = adapter_1_quota;
	/* Knowing quota of 1st chan., derive quota for 2nd chan. */
	uint32_t quota_chan2 = 100 - quota_chan1;
	/* Note: setting time quota for MCC requires info for 2 channels */
	uint32_t num_channels = 2;
	uint32_t chan1_freq = adapter_1_chan_freq;
	uint32_t chan2_freq = adapter_2_chan_freq;

	WMI_LOGD("%s: freq1:%dMHz, Quota1:%dms, "
		 "freq2:%dMHz, Quota2:%dms", __func__,
		 chan1_freq, quota_chan1, chan2_freq,
		 quota_chan2);

	/*
	 * Perform sanity check on time quota values provided.
	 */
	if (quota_chan1 < WMI_MCC_MIN_CHANNEL_QUOTA ||
	    quota_chan1 > WMI_MCC_MAX_CHANNEL_QUOTA) {
		WMI_LOGE("%s: Invalid time quota for Channel #1=%dms. Minimum "
			 "is 20ms & maximum is 80ms", __func__, quota_chan1);
		return QDF_STATUS_E_INVAL;
	}
	/* Set WMI CMD for channel time quota here */
	len = sizeof(wmi_resmgr_set_chan_time_quota_cmd_fixed_param) +
	      WMI_TLV_HDR_SIZE +       /* Place holder for chan_time_quota array */
	      num_channels * sizeof(wmi_resmgr_chan_time_quota);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmdTQ = (wmi_resmgr_set_chan_time_quota_cmd_fixed_param *)
		wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmdTQ->tlv_header,
		       WMITLV_TAG_STRUC_wmi_resmgr_set_chan_time_quota_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_resmgr_set_chan_time_quota_cmd_fixed_param));
	cmdTQ->num_chans = num_channels;

	/* Update channel time quota information for home channel(s) */
	buf_ptr += sizeof(*cmdTQ);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
		       num_channels * sizeof(wmi_resmgr_chan_time_quota));
	buf_ptr += WMI_TLV_HDR_SIZE;
	chan_quota.chan_mhz = chan1_freq;
	chan_quota.channel_time_quota = quota_chan1;
	qdf_mem_copy(buf_ptr, &chan_quota, sizeof(chan_quota));
	/* Construct channel and quota record for the 2nd MCC mode. */
	buf_ptr += sizeof(chan_quota);
	chan_quota.chan_mhz = chan2_freq;
	chan_quota.channel_time_quota = quota_chan2;
	qdf_mem_copy(buf_ptr, &chan_quota, sizeof(chan_quota));

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_RESMGR_SET_CHAN_TIME_QUOTA_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send MCC Channel Time Quota command");
		wmi_buf_free(buf);
		QDF_ASSERT(0);
	}

	return ret;
}

/**
 * send_set_thermal_mgmt_cmd_tlv() - set thermal mgmt command to fw
 * @wmi_handle: Pointer to wmi handle
 * @thermal_info: Thermal command information
 *
 * This function sends the thermal management command
 * to the firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */
static QDF_STATUS send_set_thermal_mgmt_cmd_tlv(wmi_unified_t wmi_handle,
				struct thermal_cmd_params *thermal_info)
{
	wmi_thermal_mgmt_cmd_fixed_param *cmd = NULL;
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	uint32_t len = 0;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("Failed to allocate buffer to send set key cmd");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_thermal_mgmt_cmd_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_thermal_mgmt_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_thermal_mgmt_cmd_fixed_param));

	cmd->lower_thresh_degreeC = thermal_info->min_temp;
	cmd->upper_thresh_degreeC = thermal_info->max_temp;
	cmd->enable = thermal_info->thermal_enable;

	WMI_LOGE("TM Sending thermal mgmt cmd: low temp %d, upper temp %d, enabled %d",
		cmd->lower_thresh_degreeC, cmd->upper_thresh_degreeC, cmd->enable);

	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_THERMAL_MGMT_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		WMI_LOGE("%s:Failed to send thermal mgmt command", __func__);
	}

	return status;
}


/**
 * send_lro_config_cmd_tlv() - process the LRO config command
 * @wmi_handle: Pointer to WMI handle
 * @wmi_lro_cmd: Pointer to LRO configuration parameters
 *
 * This function sends down the LRO configuration parameters to
 * the firmware to enable LRO, sets the TCP flags and sets the
 * seed values for the toeplitz hash generation
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */
static QDF_STATUS send_lro_config_cmd_tlv(wmi_unified_t wmi_handle,
	 struct wmi_lro_config_cmd_t *wmi_lro_cmd)
{
	wmi_lro_info_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;


	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGE("Failed to allocate buffer to send set key cmd");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_lro_info_cmd_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		 WMITLV_TAG_STRUC_wmi_lro_info_cmd_fixed_param,
		 WMITLV_GET_STRUCT_TLVLEN(wmi_lro_info_cmd_fixed_param));

	cmd->lro_enable = wmi_lro_cmd->lro_enable;
	WMI_LRO_INFO_TCP_FLAG_VALS_SET(cmd->tcp_flag_u32,
		 wmi_lro_cmd->tcp_flag);
	WMI_LRO_INFO_TCP_FLAGS_MASK_SET(cmd->tcp_flag_u32,
		 wmi_lro_cmd->tcp_flag_mask);
	cmd->toeplitz_hash_ipv4_0_3 =
		 wmi_lro_cmd->toeplitz_hash_ipv4[0];
	cmd->toeplitz_hash_ipv4_4_7 =
		 wmi_lro_cmd->toeplitz_hash_ipv4[1];
	cmd->toeplitz_hash_ipv4_8_11 =
		 wmi_lro_cmd->toeplitz_hash_ipv4[2];
	cmd->toeplitz_hash_ipv4_12_15 =
		 wmi_lro_cmd->toeplitz_hash_ipv4[3];
	cmd->toeplitz_hash_ipv4_16 =
		 wmi_lro_cmd->toeplitz_hash_ipv4[4];

	cmd->toeplitz_hash_ipv6_0_3 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[0];
	cmd->toeplitz_hash_ipv6_4_7 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[1];
	cmd->toeplitz_hash_ipv6_8_11 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[2];
	cmd->toeplitz_hash_ipv6_12_15 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[3];
	cmd->toeplitz_hash_ipv6_16_19 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[4];
	cmd->toeplitz_hash_ipv6_20_23 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[5];
	cmd->toeplitz_hash_ipv6_24_27 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[6];
	cmd->toeplitz_hash_ipv6_28_31 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[7];
	cmd->toeplitz_hash_ipv6_32_35 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[8];
	cmd->toeplitz_hash_ipv6_36_39 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[9];
	cmd->toeplitz_hash_ipv6_40 =
		 wmi_lro_cmd->toeplitz_hash_ipv6[10];

	WMI_LOGD("WMI_LRO_CONFIG: lro_enable %d, tcp_flag 0x%x",
		cmd->lro_enable, cmd->tcp_flag_u32);

	status = wmi_unified_cmd_send(wmi_handle, buf,
		 sizeof(*cmd), WMI_LRO_CONFIG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		WMI_LOGE("%s:Failed to send WMI_LRO_CONFIG_CMDID", __func__);
	}

	return status;
}

/**
 * send_peer_rate_report_cmd_tlv() - process the peer rate report command
 * @wmi_handle: Pointer to wmi handle
 * @rate_report_params: Pointer to peer rate report parameters
 *
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */
static QDF_STATUS send_peer_rate_report_cmd_tlv(wmi_unified_t wmi_handle,
	 struct wmi_peer_rate_report_params *rate_report_params)
{
	wmi_peer_set_rate_report_condition_fixed_param *cmd = NULL;
	wmi_buf_t buf = NULL;
	QDF_STATUS status = 0;
	uint32_t len = 0;
	uint32_t i, j;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("Failed to alloc buf to peer_set_condition cmd\n");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_peer_set_rate_report_condition_fixed_param *)
		wmi_buf_data(buf);

	WMITLV_SET_HDR(
	&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_peer_set_rate_report_condition_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN(
		wmi_peer_set_rate_report_condition_fixed_param));

	cmd->enable_rate_report  = rate_report_params->rate_report_enable;
	cmd->report_backoff_time = rate_report_params->backoff_time;
	cmd->report_timer_period = rate_report_params->timer_period;
	for (i = 0; i < PEER_RATE_REPORT_COND_MAX_NUM; i++) {
		cmd->cond_per_phy[i].val_cond_flags        =
			rate_report_params->report_per_phy[i].cond_flags;
		cmd->cond_per_phy[i].rate_delta.min_delta  =
			rate_report_params->report_per_phy[i].delta.delta_min;
		cmd->cond_per_phy[i].rate_delta.percentage =
			rate_report_params->report_per_phy[i].delta.percent;
		for (j = 0; j < MAX_NUM_OF_RATE_THRESH; j++) {
			cmd->cond_per_phy[i].rate_threshold[j] =
			rate_report_params->report_per_phy[i].
						report_rate_threshold[j];
		}
	}

	WMI_LOGE("%s enable %d backoff_time %d period %d\n", __func__,
		 cmd->enable_rate_report,
		 cmd->report_backoff_time, cmd->report_timer_period);

	status = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_SET_RATE_REPORT_CONDITION_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_buf_free(buf);
		WMI_LOGE("%s:Failed to send peer_set_report_cond command",
			 __func__);
	}
	return status;
}

/**
 * send_bcn_buf_ll_cmd_tlv() - prepare and send beacon buffer to fw for LL
 * @wmi_handle: wmi handle
 * @param: bcn ll cmd parameter
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */
static QDF_STATUS send_bcn_buf_ll_cmd_tlv(wmi_unified_t wmi_handle,
			wmi_bcn_send_from_host_cmd_fixed_param *param)
{
	wmi_bcn_send_from_host_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	QDF_STATUS ret;

	wmi_buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_bcn_send_from_host_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_send_from_host_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_bcn_send_from_host_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->data_len = param->data_len;
	cmd->frame_ctrl = param->frame_ctrl;
	cmd->frag_ptr = param->frag_ptr;
	cmd->dtim_flag = param->dtim_flag;

	ret = wmi_unified_cmd_send(wmi_handle, wmi_buf, sizeof(*cmd),
				      WMI_PDEV_SEND_BCN_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_PDEV_SEND_BCN_CMDID command");
		wmi_buf_free(wmi_buf);
	}

	return ret;
}

/**
 * send_set_sta_sa_query_param_cmd_tlv() - set sta sa query parameters
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @max_retries: max retries
 * @retry_interval: retry interval
 * This function sets sta query related parameters in fw.
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 */

static QDF_STATUS send_set_sta_sa_query_param_cmd_tlv(wmi_unified_t wmi_handle,
				       uint8_t vdev_id, uint32_t max_retries,
					   uint32_t retry_interval)
{
	wmi_buf_t buf;
	WMI_PMF_OFFLOAD_SET_SA_QUERY_CMD_fixed_param *cmd;
	int len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (WMI_PMF_OFFLOAD_SET_SA_QUERY_CMD_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_WMI_PMF_OFFLOAD_SET_SA_QUERY_CMD_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(WMI_PMF_OFFLOAD_SET_SA_QUERY_CMD_fixed_param));


	cmd->vdev_id = vdev_id;
	cmd->sa_query_max_retry_count = max_retries;
	cmd->sa_query_retry_interval = retry_interval;

	WMI_LOGD(FL("STA sa query: vdev_id:%d interval:%u retry count:%d"),
		 vdev_id, retry_interval, max_retries);

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PMF_OFFLOAD_SET_SA_QUERY_CMDID)) {
		WMI_LOGE(FL("Failed to offload STA SA Query"));
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	WMI_LOGD(FL("Exit :"));
	return 0;
}

/**
 * send_set_sta_keep_alive_cmd_tlv() - set sta keep alive parameters
 * @wmi_handle: wmi handle
 * @params: sta keep alive parameter
 *
 * This function sets keep alive related parameters in fw.
 *
 * Return: CDF status
 */
static QDF_STATUS send_set_sta_keep_alive_cmd_tlv(wmi_unified_t wmi_handle,
				struct sta_params *params)
{
	wmi_buf_t buf;
	WMI_STA_KEEPALIVE_CMD_fixed_param *cmd;
	WMI_STA_KEEPALVE_ARP_RESPONSE *arp_rsp;
	uint8_t *buf_ptr;
	int len;
	QDF_STATUS ret;

	WMI_LOGD("%s: Enter", __func__);

	len = sizeof(*cmd) + sizeof(*arp_rsp);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("wmi_buf_alloc failed");
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (WMI_STA_KEEPALIVE_CMD_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_STA_KEEPALIVE_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_STA_KEEPALIVE_CMD_fixed_param));
	cmd->interval = params->timeperiod;
	cmd->enable = (params->timeperiod) ? 1 : 0;
	cmd->vdev_id = params->vdev_id;
	WMI_LOGD("Keep Alive: vdev_id:%d interval:%u method:%d", params->vdev_id,
		 params->timeperiod, params->method);
	arp_rsp = (WMI_STA_KEEPALVE_ARP_RESPONSE *) (buf_ptr + sizeof(*cmd));
	WMITLV_SET_HDR(&arp_rsp->tlv_header,
		       WMITLV_TAG_STRUC_WMI_STA_KEEPALVE_ARP_RESPONSE,
		       WMITLV_GET_STRUCT_TLVLEN(WMI_STA_KEEPALVE_ARP_RESPONSE));

	if (params->method == WMI_KEEP_ALIVE_UNSOLICIT_ARP_RSP) {
		if ((NULL == params->hostv4addr) ||
			(NULL == params->destv4addr) ||
			(NULL == params->destmac)) {
			WMI_LOGE("%s: received null pointer, hostv4addr:%p "
			   "destv4addr:%p destmac:%p ", __func__,
			   params->hostv4addr, params->destv4addr, params->destmac);
			wmi_buf_free(buf);
			return QDF_STATUS_E_FAILURE;
		}
		cmd->method = WMI_STA_KEEPALIVE_METHOD_UNSOLICITED_ARP_RESPONSE;
		qdf_mem_copy(&arp_rsp->sender_prot_addr, params->hostv4addr,
			     WMI_IPV4_ADDR_LEN);
		qdf_mem_copy(&arp_rsp->target_prot_addr, params->destv4addr,
			     WMI_IPV4_ADDR_LEN);
		WMI_CHAR_ARRAY_TO_MAC_ADDR(params->destmac, &arp_rsp->dest_mac_addr);
	} else {
		cmd->method = WMI_STA_KEEPALIVE_METHOD_NULL_FRAME;
	}

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_STA_KEEPALIVE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to set KeepAlive");
		wmi_buf_free(buf);
	}

	WMI_LOGD("%s: Exit", __func__);
	return ret;
}

/**
 * send_vdev_set_gtx_cfg_cmd_tlv() - set GTX params
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 * @gtx_info: GTX config params
 *
 * This function set GTX related params in firmware.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_vdev_set_gtx_cfg_cmd_tlv(wmi_unified_t wmi_handle, uint32_t if_id,
				  struct wmi_gtx_config *gtx_info)
{
	wmi_vdev_set_gtx_params_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int len = sizeof(wmi_vdev_set_gtx_params_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_set_gtx_params_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_gtx_params_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_set_gtx_params_cmd_fixed_param));
	cmd->vdev_id = if_id;

	cmd->gtxRTMask[0] = gtx_info->gtx_rt_mask[0];
	cmd->gtxRTMask[1] = gtx_info->gtx_rt_mask[1];
	cmd->userGtxMask = gtx_info->gtx_usrcfg;
	cmd->gtxPERThreshold = gtx_info->gtx_threshold;
	cmd->gtxPERMargin = gtx_info->gtx_margin;
	cmd->gtxTPCstep = gtx_info->gtx_tpcstep;
	cmd->gtxTPCMin = gtx_info->gtx_tpcmin;
	cmd->gtxBWMask = gtx_info->gtx_bwmask;

	WMI_LOGD("Setting vdev%d GTX values:htmcs 0x%x, vhtmcs 0x%x, usermask 0x%x, \
		gtxPERThreshold %d, gtxPERMargin %d, gtxTPCstep %d, gtxTPCMin %d, \
		gtxBWMask 0x%x.", if_id, cmd->gtxRTMask[0], cmd->gtxRTMask[1],
		 cmd->userGtxMask, cmd->gtxPERThreshold, cmd->gtxPERMargin,
		 cmd->gtxTPCstep, cmd->gtxTPCMin, cmd->gtxBWMask);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				    WMI_VDEV_SET_GTX_PARAMS_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to set GTX PARAMS");
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 * send_process_update_edca_param_cmd_tlv() - update EDCA params
 * @wmi_handle: wmi handle
 * @edca_params: edca parameters
 *
 * This function updates EDCA parameters to the target
 *
 * Return: CDF Status
 */
static QDF_STATUS send_process_update_edca_param_cmd_tlv(wmi_unified_t wmi_handle,
				    uint8_t vdev_id,
				    wmi_wmm_vparams gwmm_param[WMI_MAX_NUM_AC])
{
	uint8_t *buf_ptr;
	wmi_buf_t buf;
	wmi_vdev_set_wmm_params_cmd_fixed_param *cmd;
	wmi_wmm_vparams *wmm_param, *twmm_param;
	int len = sizeof(*cmd);
	int ac;

	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_set_wmm_params_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_wmm_params_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_set_wmm_params_cmd_fixed_param));
	cmd->vdev_id = vdev_id;

	for (ac = 0; ac < WMI_MAX_NUM_AC; ac++) {
		wmm_param = (wmi_wmm_vparams *) (&cmd->wmm_params[ac]);
		twmm_param = (wmi_wmm_vparams *) (&gwmm_param[ac]);
		WMITLV_SET_HDR(&wmm_param->tlv_header,
			       WMITLV_TAG_STRUC_wmi_vdev_set_wmm_params_cmd_fixed_param,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_wmm_vparams));
		wmm_param->cwmin = twmm_param->cwmin;
		wmm_param->cwmax = twmm_param->cwmax;
		wmm_param->aifs = twmm_param->aifs;
		wmm_param->txoplimit = twmm_param->txoplimit;
		wmm_param->acm = twmm_param->acm;
		wmm_param->no_ack = twmm_param->no_ack;
	}

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_SET_WMM_PARAMS_CMDID))
		goto fail;

	return QDF_STATUS_SUCCESS;

fail:
	wmi_buf_free(buf);
	WMI_LOGE("%s: Failed to set WMM Paremeters", __func__);
	return QDF_STATUS_E_FAILURE;
}

/**
 * send_probe_rsp_tmpl_send_cmd_tlv() - send probe response template to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @probe_rsp_info: probe response info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_probe_rsp_tmpl_send_cmd_tlv(wmi_unified_t wmi_handle,
				   uint8_t vdev_id,
				   struct wmi_probe_resp_params *probe_rsp_info,
				   uint8_t *frm)
{
	wmi_prb_tmpl_cmd_fixed_param *cmd;
	wmi_bcn_prb_info *bcn_prb_info;
	wmi_buf_t wmi_buf;
	uint32_t tmpl_len, tmpl_len_aligned, wmi_buf_len;
	uint8_t *buf_ptr;
	QDF_STATUS ret;

	WMI_LOGD(FL("Send probe response template for vdev %d"), vdev_id);

	tmpl_len = probe_rsp_info->probeRespTemplateLen;
	tmpl_len_aligned = roundup(tmpl_len, sizeof(A_UINT32));

	wmi_buf_len = sizeof(wmi_prb_tmpl_cmd_fixed_param) +
			sizeof(wmi_bcn_prb_info) + WMI_TLV_HDR_SIZE +
			tmpl_len_aligned;

	if (wmi_buf_len > WMI_BEACON_TX_BUFFER_SIZE) {
		WMI_LOGE(FL("wmi_buf_len: %d > %d. Can't send wmi cmd"),
		wmi_buf_len, WMI_BEACON_TX_BUFFER_SIZE);
		return QDF_STATUS_E_INVAL;
	}

	wmi_buf = wmi_buf_alloc(wmi_handle, wmi_buf_len);
	if (!wmi_buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_prb_tmpl_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_prb_tmpl_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_prb_tmpl_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->buf_len = tmpl_len;
	buf_ptr += sizeof(wmi_prb_tmpl_cmd_fixed_param);

	bcn_prb_info = (wmi_bcn_prb_info *) buf_ptr;
	WMITLV_SET_HDR(&bcn_prb_info->tlv_header,
		       WMITLV_TAG_STRUC_wmi_bcn_prb_info,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_bcn_prb_info));
	bcn_prb_info->caps = 0;
	bcn_prb_info->erp = 0;
	buf_ptr += sizeof(wmi_bcn_prb_info);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, tmpl_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, frm, tmpl_len);

	ret = wmi_unified_cmd_send(wmi_handle,
				   wmi_buf, wmi_buf_len, WMI_PRB_TMPL_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL("Failed to send PRB RSP tmpl: %d"), ret);
		wmi_buf_free(wmi_buf);
	}

	return ret;
}

#if defined(ATH_SUPPORT_WAPI) || defined(FEATURE_WLAN_WAPI)
#define WPI_IV_LEN 16

/**
 * wmi_update_wpi_key_counter() - update WAPI tsc and rsc key counters
 *
 * @dest_tx: destination address of tsc key counter
 * @src_tx: source address of tsc key counter
 * @dest_rx: destination address of rsc key counter
 * @src_rx: source address of rsc key counter
 *
 * This function copies WAPI tsc and rsc key counters in the wmi buffer.
 *
 * Return: None
 *
 */
static void wmi_update_wpi_key_counter(uint8_t *dest_tx, uint8_t *src_tx,
					uint8_t *dest_rx, uint8_t *src_rx)
{
	qdf_mem_copy(dest_tx, src_tx, WPI_IV_LEN);
	qdf_mem_copy(dest_rx, src_rx, WPI_IV_LEN);
}
#else
static void wmi_update_wpi_key_counter(uint8_t *dest_tx, uint8_t *src_tx,
					uint8_t *dest_rx, uint8_t *src_rx)
{
	return;
}
#endif

/**
 * send_setup_install_key_cmd_tlv() - set key parameters
 * @wmi_handle: wmi handle
 * @key_params: key parameters
 *
 * This function fills structure from information
 * passed in key_params.
 *
 * Return: QDF_STATUS_SUCCESS - success
 *         QDF_STATUS_E_FAILURE - failure
 *         QDF_STATUS_E_NOMEM - not able to allocate buffer
 */
static QDF_STATUS send_setup_install_key_cmd_tlv(wmi_unified_t wmi_handle,
					   struct set_key_params *key_params)
{
	wmi_vdev_install_key_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len;
	uint8_t *key_data;
	QDF_STATUS status;

	len = sizeof(*cmd) + roundup(key_params->key_len, sizeof(uint32_t)) +
	       WMI_TLV_HDR_SIZE;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("Failed to allocate buffer to send set key cmd");
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_install_key_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_install_key_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_install_key_cmd_fixed_param));
	cmd->vdev_id = key_params->vdev_id;
	cmd->key_ix = key_params->key_idx;


	WMI_CHAR_ARRAY_TO_MAC_ADDR(key_params->peer_mac, &cmd->peer_macaddr);
	cmd->key_flags |= key_params->key_flags;
	cmd->key_cipher = key_params->key_cipher;
	if ((key_params->key_txmic_len) &&
			(key_params->key_rxmic_len)) {
		cmd->key_txmic_len = key_params->key_txmic_len;
		cmd->key_rxmic_len = key_params->key_rxmic_len;
	}
#if defined(ATH_SUPPORT_WAPI) || defined(FEATURE_WLAN_WAPI)
	wmi_update_wpi_key_counter(cmd->wpi_key_tsc_counter,
				   key_params->tx_iv,
				   cmd->wpi_key_rsc_counter,
				   key_params->rx_iv);
#endif
	buf_ptr += sizeof(wmi_vdev_install_key_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
		       roundup(key_params->key_len, sizeof(uint32_t)));
	key_data = (A_UINT8 *) (buf_ptr + WMI_TLV_HDR_SIZE);
	qdf_mem_copy((void *)key_data,
		     (const void *)key_params->key_data, key_params->key_len);
	cmd->key_len = key_params->key_len;

	status = wmi_unified_cmd_send(wmi_handle, buf, len,
					      WMI_VDEV_INSTALL_KEY_CMDID);
	if (QDF_IS_STATUS_ERROR(status))
		wmi_buf_free(buf);

	return status;
}

/**
 * send_sar_limit_cmd_tlv() - send sar limit cmd to fw
 * @wmi_handle: wmi handle
 * @params: sar limit params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_sar_limit_cmd_tlv(wmi_unified_t wmi_handle,
		struct sar_limit_cmd_params *sar_limit_params)
{
	wmi_buf_t buf;
	QDF_STATUS qdf_status;
	wmi_sar_limits_cmd_fixed_param *cmd;
	int i;
	uint8_t *buf_ptr;
	wmi_sar_limit_cmd_row *wmi_sar_rows_list;
	struct sar_limit_cmd_row *sar_rows_list;
	uint32_t len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;

	len += sizeof(wmi_sar_limit_cmd_row) * sar_limit_params->num_limit_rows;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("Failed to allocate memory");
		qdf_status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_sar_limits_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_sar_limits_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_sar_limits_cmd_fixed_param));
	cmd->sar_enable = sar_limit_params->sar_enable;
	cmd->commit_limits = sar_limit_params->commit_limits;
	cmd->num_limit_rows = sar_limit_params->num_limit_rows;

	WMI_LOGD("no of sar rows = %d, len = %d",
		 sar_limit_params->num_limit_rows, len);
	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_sar_limit_cmd_row) *
			       sar_limit_params->num_limit_rows);
	if (cmd->num_limit_rows == 0)
		goto send_sar_limits;

	wmi_sar_rows_list = (wmi_sar_limit_cmd_row *)
			(buf_ptr + WMI_TLV_HDR_SIZE);
	sar_rows_list = sar_limit_params->sar_limit_row_list;

	for (i = 0; i < sar_limit_params->num_limit_rows; i++) {
		WMITLV_SET_HDR(&wmi_sar_rows_list->tlv_header,
			       WMITLV_TAG_STRUC_wmi_sar_limit_cmd_row,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_sar_limit_cmd_row));
		wmi_sar_rows_list->band_id = sar_rows_list->band_id;
		wmi_sar_rows_list->chain_id = sar_rows_list->chain_id;
		wmi_sar_rows_list->mod_id = sar_rows_list->mod_id;
		wmi_sar_rows_list->limit_value = sar_rows_list->limit_value;
		wmi_sar_rows_list->validity_bitmap =
						sar_rows_list->validity_bitmap;
		WMI_LOGD("row %d, band_id = %d, chain_id = %d, mod_id = %d, limit_value = %d, validity_bitmap = %d",
			 i, wmi_sar_rows_list->band_id,
			 wmi_sar_rows_list->chain_id,
			 wmi_sar_rows_list->mod_id,
			 wmi_sar_rows_list->limit_value,
			 wmi_sar_rows_list->validity_bitmap);
		sar_rows_list++;
		wmi_sar_rows_list++;
	}
send_sar_limits:
	qdf_status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_SAR_LIMITS_CMDID);

	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		WMI_LOGE("Failed to send WMI_SAR_LIMITS_CMDID");
		wmi_buf_free(buf);
	}

end:
	return qdf_status;
}

/**
 * send_encrypt_decrypt_send_cmd() - send encrypt/decrypt cmd to fw
 * @wmi_handle: wmi handle
 * @params: encrypt/decrypt params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static
QDF_STATUS send_encrypt_decrypt_send_cmd_tlv(wmi_unified_t wmi_handle,
		struct encrypt_decrypt_req_params *encrypt_decrypt_params)
{
	wmi_vdev_encrypt_decrypt_data_req_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint8_t *buf_ptr;
	QDF_STATUS ret;
	uint32_t len;

	WMI_LOGD(FL("Send encrypt decrypt cmd"));

	len = sizeof(*cmd) +
		roundup(encrypt_decrypt_params->data_len, sizeof(A_UINT32)) +
		WMI_TLV_HDR_SIZE;
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGP("%s: failed to allocate memory for encrypt/decrypt msg",
			 __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(wmi_buf);
	cmd = (wmi_vdev_encrypt_decrypt_data_req_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_encrypt_decrypt_data_req_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_vdev_encrypt_decrypt_data_req_cmd_fixed_param));

	cmd->vdev_id = encrypt_decrypt_params->vdev_id;
	cmd->key_flag = encrypt_decrypt_params->key_flag;
	cmd->key_idx = encrypt_decrypt_params->key_idx;
	cmd->key_cipher = encrypt_decrypt_params->key_cipher;
	cmd->key_len = encrypt_decrypt_params->key_len;
	cmd->key_txmic_len = encrypt_decrypt_params->key_txmic_len;
	cmd->key_rxmic_len = encrypt_decrypt_params->key_rxmic_len;

	qdf_mem_copy(cmd->key_data, encrypt_decrypt_params->key_data,
				encrypt_decrypt_params->key_len);

	qdf_mem_copy(cmd->mac_hdr, encrypt_decrypt_params->mac_header,
				MAX_MAC_HEADER_LEN);

	cmd->data_len = encrypt_decrypt_params->data_len;

	if (cmd->data_len) {
		buf_ptr += sizeof(*cmd);
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
				roundup(encrypt_decrypt_params->data_len,
					sizeof(A_UINT32)));
		buf_ptr += WMI_TLV_HDR_SIZE;
		qdf_mem_copy(buf_ptr, encrypt_decrypt_params->data,
					encrypt_decrypt_params->data_len);
	}

	/* This conversion is to facilitate data to FW in little endian */
	cmd->pn[5] = encrypt_decrypt_params->pn[0];
	cmd->pn[4] = encrypt_decrypt_params->pn[1];
	cmd->pn[3] = encrypt_decrypt_params->pn[2];
	cmd->pn[2] = encrypt_decrypt_params->pn[3];
	cmd->pn[1] = encrypt_decrypt_params->pn[4];
	cmd->pn[0] = encrypt_decrypt_params->pn[5];

	ret = wmi_unified_cmd_send(wmi_handle,
				   wmi_buf, len,
				   WMI_VDEV_ENCRYPT_DECRYPT_DATA_REQ_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send ENCRYPT DECRYPT cmd: %d", ret);
		wmi_buf_free(wmi_buf);
	}

	return ret;
}



/**
 * send_p2p_go_set_beacon_ie_cmd_tlv() - set beacon IE for p2p go
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @p2p_ie: p2p IE
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_p2p_go_set_beacon_ie_cmd_tlv(wmi_unified_t wmi_handle,
				    A_UINT32 vdev_id, uint8_t *p2p_ie)
{
	QDF_STATUS ret;
	wmi_p2p_go_set_beacon_ie_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t ie_len, ie_len_aligned, wmi_buf_len;
	uint8_t *buf_ptr;

	ie_len = (uint32_t) (p2p_ie[1] + 2);

	/* More than one P2P IE may be included in a single frame.
	   If multiple P2P IEs are present, the complete P2P attribute
	   data consists of the concatenation of the P2P Attribute
	   fields of the P2P IEs. The P2P Attributes field of each
	   P2P IE may be any length up to the maximum (251 octets).
	   In this case host sends one P2P IE to firmware so the length
	   should not exceed more than 251 bytes
	 */
	if (ie_len > 251) {
		WMI_LOGE("%s : invalid p2p ie length %u", __func__, ie_len);
		return QDF_STATUS_E_INVAL;
	}

	ie_len_aligned = roundup(ie_len, sizeof(A_UINT32));

	wmi_buf_len =
		sizeof(wmi_p2p_go_set_beacon_ie_fixed_param) + ie_len_aligned +
		WMI_TLV_HDR_SIZE;

	wmi_buf = wmi_buf_alloc(wmi_handle, wmi_buf_len);
	if (!wmi_buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_p2p_go_set_beacon_ie_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_p2p_go_set_beacon_ie_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_p2p_go_set_beacon_ie_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->ie_buf_len = ie_len;

	buf_ptr += sizeof(wmi_p2p_go_set_beacon_ie_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, ie_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, p2p_ie, ie_len);

	WMI_LOGI("%s: Sending WMI_P2P_GO_SET_BEACON_IE", __func__);

	ret = wmi_unified_cmd_send(wmi_handle,
				   wmi_buf, wmi_buf_len,
				   WMI_P2P_GO_SET_BEACON_IE);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send bcn tmpl: %d", ret);
		wmi_buf_free(wmi_buf);
	}

	WMI_LOGI("%s: Successfully sent WMI_P2P_GO_SET_BEACON_IE", __func__);
	return ret;
}

/**
 * send_set_gateway_params_cmd_tlv() - set gateway parameters
 * @wmi_handle: wmi handle
 * @req: gateway parameter update request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and sends down the gateway configs down to the firmware
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS send_set_gateway_params_cmd_tlv(wmi_unified_t wmi_handle,
				struct gateway_update_req_param *req)
{
	wmi_roam_subnet_change_config_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_roam_subnet_change_config_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_roam_subnet_change_config_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_roam_subnet_change_config_fixed_param));

	cmd->vdev_id = req->session_id;
	qdf_mem_copy(&cmd->inet_gw_ip_v4_addr, req->ipv4_addr,
		QDF_IPV4_ADDR_SIZE);
	qdf_mem_copy(&cmd->inet_gw_ip_v6_addr, req->ipv6_addr,
		QDF_IPV6_ADDR_SIZE);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(req->gw_mac_addr.bytes,
		&cmd->inet_gw_mac_addr);
	cmd->max_retries = req->max_retries;
	cmd->timeout = req->timeout;
	cmd->num_skip_subnet_change_detection_bssid_list = 0;
	cmd->flag = 0;
	if (req->ipv4_addr_type)
		WMI_SET_ROAM_SUBNET_CHANGE_FLAG_IP4_ENABLED(cmd->flag);

	if (req->ipv6_addr_type)
		WMI_SET_ROAM_SUBNET_CHANGE_FLAG_IP6_ENABLED(cmd->flag);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_ROAM_SUBNET_CHANGE_CONFIG_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send gw config parameter to fw, ret: %d",
			ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_rssi_monitoring_cmd_tlv() - set rssi monitoring
 * @wmi_handle: wmi handle
 * @req: rssi monitoring request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and send down the rssi monitoring configs down to the firmware
 *
 * Return: 0 on success; error number otherwise
 */
static QDF_STATUS send_set_rssi_monitoring_cmd_tlv(wmi_unified_t wmi_handle,
					struct rssi_monitor_param *req)
{
	wmi_rssi_breach_monitor_config_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	uint32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_rssi_breach_monitor_config_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_rssi_breach_monitor_config_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_rssi_breach_monitor_config_fixed_param));

	cmd->vdev_id = req->session_id;
	cmd->request_id = req->request_id;
	cmd->lo_rssi_reenable_hysteresis = 0;
	cmd->hi_rssi_reenable_histeresis = 0;
	cmd->min_report_interval = 0;
	cmd->max_num_report = 1;
	if (req->control) {
		/* enable one threshold for each min/max */
		cmd->enabled_bitmap = 0x09;
		cmd->low_rssi_breach_threshold[0] = req->min_rssi;
		cmd->hi_rssi_breach_threshold[0] = req->max_rssi;
	} else {
		cmd->enabled_bitmap = 0;
		cmd->low_rssi_breach_threshold[0] = 0;
		cmd->hi_rssi_breach_threshold[0] = 0;
	}

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_RSSI_BREACH_MONITOR_CONFIG_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_RSSI_BREACH_MONITOR_CONFIG_CMDID");
		wmi_buf_free(buf);
	}

	WMI_LOGI("Sent WMI_RSSI_BREACH_MONITOR_CONFIG_CMDID to FW");
	return ret;
}

/**
 * send_scan_probe_setoui_cmd_tlv() - set scan probe OUI
 * @wmi_handle: wmi handle
 * @psetoui: OUI parameters
 *
 * set scan probe OUI parameters in firmware
 *
 * Return: CDF status
 */
static QDF_STATUS send_scan_probe_setoui_cmd_tlv(wmi_unified_t wmi_handle,
			  struct scan_mac_oui *psetoui)
{
	wmi_scan_prob_req_oui_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;
	uint32_t *oui_buf;

	len = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_scan_prob_req_oui_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_scan_prob_req_oui_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_scan_prob_req_oui_cmd_fixed_param));

	oui_buf = &cmd->prob_req_oui;
	qdf_mem_zero(oui_buf, sizeof(cmd->prob_req_oui));
	*oui_buf = psetoui->oui[0] << 16 | psetoui->oui[1] << 8
		   | psetoui->oui[2];
	WMI_LOGD("%s: wmi:oui received from hdd %08x", __func__,
		 cmd->prob_req_oui);

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_SCAN_PROB_REQ_OUI_CMDID)) {
		WMI_LOGE("%s: failed to send command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_reset_passpoint_network_list_cmd_tlv() - reset passpoint network list
 * @wmi_handle: wmi handle
 * @req: passpoint network request structure
 *
 * This function sends down WMI command with network id set to wildcard id.
 * firmware shall clear all the config entries
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS send_reset_passpoint_network_list_cmd_tlv(wmi_unified_t wmi_handle,
					struct wifi_passpoint_req_param *req)
{
	wmi_passpoint_config_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint32_t len;
	int ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_passpoint_config_cmd_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_passpoint_config_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
			wmi_passpoint_config_cmd_fixed_param));
	cmd->id = WMI_PASSPOINT_NETWORK_ID_WILDCARD;

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PASSPOINT_LIST_CONFIG_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send reset passpoint network list wmi cmd",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_passpoint_network_list_cmd_tlv() - set passpoint network list
 * @wmi_handle: wmi handle
 * @req: passpoint network request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and send down the passpoint configs down to the firmware
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS send_set_passpoint_network_list_cmd_tlv(wmi_unified_t wmi_handle,
					struct wifi_passpoint_req_param *req)
{
	wmi_passpoint_config_cmd_fixed_param *cmd;
	u_int8_t i, j, *bytes;
	wmi_buf_t buf;
	uint32_t len;
	int ret;

	len = sizeof(*cmd);
	for (i = 0; i < req->num_networks; i++) {
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
			return QDF_STATUS_E_NOMEM;
		}

		cmd = (wmi_passpoint_config_cmd_fixed_param *)
				wmi_buf_data(buf);

		WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_passpoint_config_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
			wmi_passpoint_config_cmd_fixed_param));
		cmd->id = req->networks[i].id;
		WMI_LOGD("%s: network id: %u", __func__, cmd->id);
		qdf_mem_copy(cmd->realm, req->networks[i].realm,
			strlen(req->networks[i].realm) + 1);
		WMI_LOGD("%s: realm: %s", __func__, cmd->realm);
		for (j = 0; j < PASSPOINT_ROAMING_CONSORTIUM_ID_NUM; j++) {
			bytes = (uint8_t *) &req->networks[i].roaming_consortium_ids[j];
			WMI_LOGD("index: %d rcids: %02x %02x %02x %02x %02x %02x %02x %02x",
				j, bytes[0], bytes[1], bytes[2], bytes[3],
				bytes[4], bytes[5], bytes[6], bytes[7]);

			qdf_mem_copy(&cmd->roaming_consortium_ids[j],
				&req->networks[i].roaming_consortium_ids[j],
				PASSPOINT_ROAMING_CONSORTIUM_ID_LEN);
		}
		qdf_mem_copy(cmd->plmn, req->networks[i].plmn,
				PASSPOINT_PLMN_ID_LEN);
		WMI_LOGD("%s: plmn: %02x:%02x:%02x", __func__,
			cmd->plmn[0], cmd->plmn[1], cmd->plmn[2]);

		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					   WMI_PASSPOINT_LIST_CONFIG_CMDID);
		if (ret) {
			WMI_LOGE("%s: Failed to send set passpoint network list wmi cmd",
				 __func__);
			wmi_buf_free(buf);
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_roam_scan_offload_mode_cmd_tlv() - send roam scan mode request to fw
 * @wmi_handle: wmi handle
 * @scan_cmd_fp: start scan command ptr
 * @roam_req: roam request param
 *
 * send WMI_ROAM_SCAN_MODE TLV to firmware. It has a piggyback
 * of WMI_ROAM_SCAN_MODE.
 *
 * Return: QDF status
 */
static QDF_STATUS send_roam_scan_offload_mode_cmd_tlv(wmi_unified_t wmi_handle,
				      wmi_start_scan_cmd_fixed_param *
				      scan_cmd_fp,
				      struct roam_offload_scan_params *roam_req)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	int len;
	uint8_t *buf_ptr;
	wmi_roam_scan_mode_fixed_param *roam_scan_mode_fp;

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	int auth_mode = roam_req->auth_mode;
	wmi_roam_offload_tlv_param *roam_offload_params;
	wmi_roam_11i_offload_tlv_param *roam_offload_11i;
	wmi_roam_11r_offload_tlv_param *roam_offload_11r;
	wmi_roam_ese_offload_tlv_param *roam_offload_ese;
	wmi_tlv_buf_len_param *assoc_ies;
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
	/* Need to create a buf with roam_scan command at
	 * front and piggyback with scan command */
	len = sizeof(wmi_roam_scan_mode_fixed_param) +
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	      (2 * WMI_TLV_HDR_SIZE) +
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
	      sizeof(wmi_start_scan_cmd_fixed_param);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		if (roam_req->is_roam_req_valid &&
				roam_req->roam_offload_enabled) {
			len += sizeof(wmi_roam_offload_tlv_param);
			len += WMI_TLV_HDR_SIZE;
			if ((auth_mode != WMI_AUTH_NONE) &&
				((auth_mode != WMI_AUTH_OPEN) ||
				 (auth_mode == WMI_AUTH_OPEN &&
				  roam_req->mdid.mdie_present) ||
				  roam_req->is_ese_assoc)) {
				len += WMI_TLV_HDR_SIZE;
				if (roam_req->is_ese_assoc)
					len +=
					sizeof(wmi_roam_ese_offload_tlv_param);
				else if (auth_mode == WMI_AUTH_FT_RSNA ||
					 auth_mode == WMI_AUTH_FT_RSNA_PSK ||
					 (auth_mode == WMI_AUTH_OPEN &&
					  roam_req->mdid.mdie_present))
					len +=
					sizeof(wmi_roam_11r_offload_tlv_param);
				else
					len +=
					sizeof(wmi_roam_11i_offload_tlv_param);
			} else {
				len += WMI_TLV_HDR_SIZE;
			}

			len += (sizeof(*assoc_ies) + (2*WMI_TLV_HDR_SIZE)
					+ roundup(roam_req->assoc_ie_length,
					sizeof(uint32_t)));
		} else {
			if (roam_req->is_roam_req_valid)
				WMI_LOGD("%s : roam offload = %d",
				     __func__, roam_req->roam_offload_enabled);
			else
				WMI_LOGD("%s : roam_req is NULL", __func__);
			len += (4 * WMI_TLV_HDR_SIZE);
		}
		if (roam_req->is_roam_req_valid &&
				roam_req->roam_offload_enabled) {
			roam_req->mode = roam_req->mode |
				WMI_ROAM_SCAN_MODE_ROAMOFFLOAD;
		}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

	if (roam_req->mode == (WMI_ROAM_SCAN_MODE_NONE
				|WMI_ROAM_SCAN_MODE_ROAMOFFLOAD))
		len = sizeof(wmi_roam_scan_mode_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);

	roam_scan_mode_fp = (wmi_roam_scan_mode_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&roam_scan_mode_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_roam_scan_mode_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_roam_scan_mode_fixed_param));

	roam_scan_mode_fp->roam_scan_mode = roam_req->mode;
	roam_scan_mode_fp->vdev_id = roam_req->vdev_id;
	if (roam_req->mode == (WMI_ROAM_SCAN_MODE_NONE |
			WMI_ROAM_SCAN_MODE_ROAMOFFLOAD)) {
		roam_scan_mode_fp->flags |=
			WMI_ROAM_SCAN_MODE_FLAG_REPORT_STATUS;
		goto send_roam_scan_mode_cmd;
	}

	/* Fill in scan parameters suitable for roaming scan */
	buf_ptr += sizeof(wmi_roam_scan_mode_fixed_param);

	qdf_mem_copy(buf_ptr, scan_cmd_fp,
		     sizeof(wmi_start_scan_cmd_fixed_param));
	/* Ensure there is no additional IEs */
	scan_cmd_fp->ie_len = 0;
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_STRUC_wmi_start_scan_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_start_scan_cmd_fixed_param));
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	buf_ptr += sizeof(wmi_start_scan_cmd_fixed_param);
	if (roam_req->is_roam_req_valid && roam_req->roam_offload_enabled) {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			       sizeof(wmi_roam_offload_tlv_param));
		buf_ptr += WMI_TLV_HDR_SIZE;
		roam_offload_params = (wmi_roam_offload_tlv_param *) buf_ptr;
		WMITLV_SET_HDR(buf_ptr,
			       WMITLV_TAG_STRUC_wmi_roam_offload_tlv_param,
			       WMITLV_GET_STRUCT_TLVLEN
				       (wmi_roam_offload_tlv_param));
		roam_offload_params->prefer_5g = roam_req->prefer_5ghz;
		roam_offload_params->rssi_cat_gap = roam_req->roam_rssi_cat_gap;
		roam_offload_params->select_5g_margin =
			roam_req->select_5ghz_margin;
		roam_offload_params->reassoc_failure_timeout =
			roam_req->reassoc_failure_timeout;

		/* Fill the capabilities */
		roam_offload_params->capability =
				roam_req->roam_offload_params.capability;
		roam_offload_params->ht_caps_info =
				roam_req->roam_offload_params.ht_caps_info;
		roam_offload_params->ampdu_param =
				roam_req->roam_offload_params.ampdu_param;
		roam_offload_params->ht_ext_cap =
				roam_req->roam_offload_params.ht_ext_cap;
		roam_offload_params->ht_txbf =
				roam_req->roam_offload_params.ht_txbf;
		roam_offload_params->asel_cap =
				roam_req->roam_offload_params.asel_cap;
		roam_offload_params->qos_caps =
				roam_req->roam_offload_params.qos_caps;
		roam_offload_params->qos_enabled =
				roam_req->roam_offload_params.qos_enabled;
		roam_offload_params->wmm_caps =
				roam_req->roam_offload_params.wmm_caps;
		qdf_mem_copy((uint8_t *)roam_offload_params->mcsset,
				(uint8_t *)roam_req->roam_offload_params.mcsset,
				ROAM_OFFLOAD_NUM_MCS_SET);

		buf_ptr += sizeof(wmi_roam_offload_tlv_param);
		/* The TLV's are in the order of 11i, 11R, ESE. Hence,
		 * they are filled in the same order.Depending on the
		 * authentication type, the other mode TLV's are nullified
		 * and only headers are filled.*/
		if ((auth_mode != WMI_AUTH_NONE) &&
		    ((auth_mode != WMI_AUTH_OPEN) ||
		     (auth_mode == WMI_AUTH_OPEN
		      && roam_req->mdid.mdie_present) ||
			roam_req->is_ese_assoc)) {
			if (roam_req->is_ese_assoc) {
				WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
					       WMITLV_GET_STRUCT_TLVLEN(0));
				buf_ptr += WMI_TLV_HDR_SIZE;
				WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
					       WMITLV_GET_STRUCT_TLVLEN(0));
				buf_ptr += WMI_TLV_HDR_SIZE;
				WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
					sizeof(wmi_roam_ese_offload_tlv_param));
				buf_ptr += WMI_TLV_HDR_SIZE;
				roam_offload_ese =
				    (wmi_roam_ese_offload_tlv_param *) buf_ptr;
				qdf_mem_copy(roam_offload_ese->krk,
					     roam_req->krk,
					     sizeof(roam_req->krk));
				qdf_mem_copy(roam_offload_ese->btk,
					     roam_req->btk,
					     sizeof(roam_req->btk));
				WMITLV_SET_HDR(&roam_offload_ese->tlv_header,
				WMITLV_TAG_STRUC_wmi_roam_ese_offload_tlv_param,
				WMITLV_GET_STRUCT_TLVLEN
				(wmi_roam_ese_offload_tlv_param));
				buf_ptr +=
					sizeof(wmi_roam_ese_offload_tlv_param);
			} else if (auth_mode == WMI_AUTH_FT_RSNA
				   || auth_mode == WMI_AUTH_FT_RSNA_PSK
				   || (auth_mode == WMI_AUTH_OPEN
				       && roam_req->mdid.mdie_present)) {
				WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
					       0);
				buf_ptr += WMI_TLV_HDR_SIZE;
				WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
					sizeof(wmi_roam_11r_offload_tlv_param));
				buf_ptr += WMI_TLV_HDR_SIZE;
				roam_offload_11r =
				    (wmi_roam_11r_offload_tlv_param *) buf_ptr;
				roam_offload_11r->r0kh_id_len =
					roam_req->rokh_id_length;
				qdf_mem_copy(roam_offload_11r->r0kh_id,
					     roam_req->rokh_id,
					     roam_offload_11r->r0kh_id_len);
				qdf_mem_copy(roam_offload_11r->psk_msk,
					     roam_req->psk_pmk,
					     sizeof(roam_req->psk_pmk));
				roam_offload_11r->psk_msk_len =
					roam_req->pmk_len;
				roam_offload_11r->mdie_present =
					roam_req->mdid.mdie_present;
				roam_offload_11r->mdid =
					roam_req->mdid.mobility_domain;
				if (auth_mode == WMI_AUTH_OPEN) {
					/* If FT-Open ensure pmk length
					   and r0khid len are zero */
					roam_offload_11r->r0kh_id_len = 0;
					roam_offload_11r->psk_msk_len = 0;
				}
				WMITLV_SET_HDR(&roam_offload_11r->tlv_header,
				WMITLV_TAG_STRUC_wmi_roam_11r_offload_tlv_param,
				WMITLV_GET_STRUCT_TLVLEN
				(wmi_roam_11r_offload_tlv_param));
				buf_ptr +=
					sizeof(wmi_roam_11r_offload_tlv_param);
				WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
					       WMITLV_GET_STRUCT_TLVLEN(0));
				buf_ptr += WMI_TLV_HDR_SIZE;
			} else {
				WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
					sizeof(wmi_roam_11i_offload_tlv_param));
				buf_ptr += WMI_TLV_HDR_SIZE;
				roam_offload_11i =
				     (wmi_roam_11i_offload_tlv_param *) buf_ptr;
				if (roam_req->roam_key_mgmt_offload_enabled &&
				    roam_req->okc_enabled) {
					WMI_SET_ROAM_OFFLOAD_OKC_ENABLED
						(roam_offload_11i->flags);
					WMI_LOGE("LFR3:OKC Enabled");
				} else {
					WMI_SET_ROAM_OFFLOAD_OKC_DISABLED
						(roam_offload_11i->flags);
					WMI_LOGE("LFR3:OKC Disabled");
				}

				qdf_mem_copy(roam_offload_11i->pmk,
					     roam_req->psk_pmk,
					     sizeof(roam_req->psk_pmk));
				roam_offload_11i->pmk_len = roam_req->pmk_len;
				WMITLV_SET_HDR(&roam_offload_11i->tlv_header,
				WMITLV_TAG_STRUC_wmi_roam_11i_offload_tlv_param,
				WMITLV_GET_STRUCT_TLVLEN
				(wmi_roam_11i_offload_tlv_param));
				buf_ptr +=
					sizeof(wmi_roam_11i_offload_tlv_param);
				WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
					       0);
				buf_ptr += WMI_TLV_HDR_SIZE;
				WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
					       0);
				buf_ptr += WMI_TLV_HDR_SIZE;
			}
		} else {
			WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
				       WMITLV_GET_STRUCT_TLVLEN(0));
			buf_ptr += WMI_TLV_HDR_SIZE;
			WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
				       WMITLV_GET_STRUCT_TLVLEN(0));
			buf_ptr += WMI_TLV_HDR_SIZE;
			WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
				       WMITLV_GET_STRUCT_TLVLEN(0));
			buf_ptr += WMI_TLV_HDR_SIZE;
		}

		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
					sizeof(*assoc_ies));
		buf_ptr += WMI_TLV_HDR_SIZE;

		assoc_ies = (wmi_tlv_buf_len_param *) buf_ptr;
		WMITLV_SET_HDR(&assoc_ies->tlv_header,
			WMITLV_TAG_STRUC_wmi_tlv_buf_len_param,
			WMITLV_GET_STRUCT_TLVLEN(wmi_tlv_buf_len_param));
		assoc_ies->buf_len = roam_req->assoc_ie_length;

		buf_ptr += sizeof(*assoc_ies);

		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
				roundup(assoc_ies->buf_len, sizeof(uint32_t)));
		buf_ptr += WMI_TLV_HDR_SIZE;

		if (assoc_ies->buf_len != 0) {
			qdf_mem_copy(buf_ptr, roam_req->assoc_ie,
					assoc_ies->buf_len);
		}

	} else {
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			       WMITLV_GET_STRUCT_TLVLEN(0));
		buf_ptr += WMI_TLV_HDR_SIZE;
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			       WMITLV_GET_STRUCT_TLVLEN(0));
		buf_ptr += WMI_TLV_HDR_SIZE;
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			       WMITLV_GET_STRUCT_TLVLEN(0));
		buf_ptr += WMI_TLV_HDR_SIZE;
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			       WMITLV_GET_STRUCT_TLVLEN(0));
		buf_ptr += WMI_TLV_HDR_SIZE;
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
				WMITLV_GET_STRUCT_TLVLEN(0));
		buf_ptr += WMI_TLV_HDR_SIZE;
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
				WMITLV_GET_STRUCT_TLVLEN(0));
	}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

send_roam_scan_mode_cmd:
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_ROAM_SCAN_MODE);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE(
		    "wmi_unified_cmd_send WMI_ROAM_SCAN_MODE returned Error %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}


/**
 * send_roam_scan_offload_rssi_thresh_cmd_tlv() - set scan offload
 *                                                rssi threashold
 * @wmi_handle: wmi handle
 * @roam_req:   Roaming request buffer
 *
 * Send WMI_ROAM_SCAN_RSSI_THRESHOLD TLV to firmware
 *
 * Return: QDF status
 */
static QDF_STATUS send_roam_scan_offload_rssi_thresh_cmd_tlv(wmi_unified_t wmi_handle,
				struct roam_offload_scan_rssi_params *roam_req)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	int len;
	uint8_t *buf_ptr;
	wmi_roam_scan_rssi_threshold_fixed_param *rssi_threshold_fp;
	wmi_roam_scan_extended_threshold_param *ext_thresholds = NULL;
	wmi_roam_earlystop_rssi_thres_param *early_stop_thresholds = NULL;
	wmi_roam_dense_thres_param *dense_thresholds = NULL;

	len = sizeof(wmi_roam_scan_rssi_threshold_fixed_param);
	len += WMI_TLV_HDR_SIZE; /* TLV for ext_thresholds*/
	len += sizeof(wmi_roam_scan_extended_threshold_param);
	len += WMI_TLV_HDR_SIZE;
	len += sizeof(wmi_roam_earlystop_rssi_thres_param);
	len += WMI_TLV_HDR_SIZE; /* TLV for dense thresholds*/
	len += sizeof(wmi_roam_dense_thres_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	rssi_threshold_fp =
		(wmi_roam_scan_rssi_threshold_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&rssi_threshold_fp->tlv_header,
		      WMITLV_TAG_STRUC_wmi_roam_scan_rssi_threshold_fixed_param,
		      WMITLV_GET_STRUCT_TLVLEN
			       (wmi_roam_scan_rssi_threshold_fixed_param));
	/* fill in threshold values */
	rssi_threshold_fp->vdev_id = roam_req->session_id;
	rssi_threshold_fp->roam_scan_rssi_thresh = roam_req->rssi_thresh;
	rssi_threshold_fp->roam_rssi_thresh_diff = roam_req->rssi_thresh_diff;
	rssi_threshold_fp->hirssi_scan_max_count =
			roam_req->hi_rssi_scan_max_count;
	rssi_threshold_fp->hirssi_scan_delta =
			roam_req->hi_rssi_scan_rssi_delta;
	rssi_threshold_fp->hirssi_upper_bound = roam_req->hi_rssi_scan_rssi_ub;

	buf_ptr += sizeof(wmi_roam_scan_rssi_threshold_fixed_param);
	WMITLV_SET_HDR(buf_ptr,
			WMITLV_TAG_ARRAY_STRUC,
			sizeof(wmi_roam_scan_extended_threshold_param));
	buf_ptr += WMI_TLV_HDR_SIZE;
	ext_thresholds = (wmi_roam_scan_extended_threshold_param *) buf_ptr;

	ext_thresholds->penalty_threshold_5g = roam_req->penalty_threshold_5g;
	if (roam_req->raise_rssi_thresh_5g >= WMI_NOISE_FLOOR_DBM_DEFAULT)
		ext_thresholds->boost_threshold_5g =
					roam_req->boost_threshold_5g;

	ext_thresholds->boost_algorithm_5g =
		WMI_ROAM_5G_BOOST_PENALIZE_ALGO_LINEAR;
	ext_thresholds->boost_factor_5g = roam_req->raise_factor_5g;
	ext_thresholds->penalty_algorithm_5g =
		WMI_ROAM_5G_BOOST_PENALIZE_ALGO_LINEAR;
	ext_thresholds->penalty_factor_5g = roam_req->drop_factor_5g;
	ext_thresholds->max_boost_5g = roam_req->max_raise_rssi_5g;
	ext_thresholds->max_penalty_5g = roam_req->max_drop_rssi_5g;
	ext_thresholds->good_rssi_threshold = roam_req->good_rssi_threshold;

	WMITLV_SET_HDR(&ext_thresholds->tlv_header,
		WMITLV_TAG_STRUC_wmi_roam_scan_extended_threshold_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_roam_scan_extended_threshold_param));
	buf_ptr += sizeof(wmi_roam_scan_extended_threshold_param);
	WMITLV_SET_HDR(buf_ptr,
			WMITLV_TAG_ARRAY_STRUC,
			sizeof(wmi_roam_earlystop_rssi_thres_param));
	buf_ptr += WMI_TLV_HDR_SIZE;
	early_stop_thresholds = (wmi_roam_earlystop_rssi_thres_param *) buf_ptr;
	early_stop_thresholds->roam_earlystop_thres_min =
		roam_req->roam_earlystop_thres_min;
	early_stop_thresholds->roam_earlystop_thres_max =
		roam_req->roam_earlystop_thres_max;
	WMITLV_SET_HDR(&early_stop_thresholds->tlv_header,
		WMITLV_TAG_STRUC_wmi_roam_earlystop_rssi_thres_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_roam_earlystop_rssi_thres_param));

	buf_ptr += sizeof(wmi_roam_earlystop_rssi_thres_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			 sizeof(wmi_roam_dense_thres_param));
	buf_ptr += WMI_TLV_HDR_SIZE;
	dense_thresholds = (wmi_roam_dense_thres_param *) buf_ptr;
	dense_thresholds->roam_dense_rssi_thres_offset =
			roam_req->dense_rssi_thresh_offset;
	dense_thresholds->roam_dense_min_aps = roam_req->dense_min_aps_cnt;
	dense_thresholds->roam_dense_traffic_thres =
			roam_req->traffic_threshold;
	dense_thresholds->roam_dense_status = roam_req->initial_dense_status;
	WMITLV_SET_HDR(&dense_thresholds->tlv_header,
			WMITLV_TAG_STRUC_wmi_roam_dense_thres_param,
			WMITLV_GET_STRUCT_TLVLEN
			(wmi_roam_dense_thres_param));

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_ROAM_SCAN_RSSI_THRESHOLD);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("cmd WMI_ROAM_SCAN_RSSI_THRESHOLD returned Error %d",
					status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_adapt_dwelltime_params_cmd_tlv() - send wmi cmd of adaptive dwelltime
 * configuration params
 * @wma_handle:  wma handler
 * @dwelltime_params: pointer to dwelltime_params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF failure reason code for failure
 */
static
QDF_STATUS send_adapt_dwelltime_params_cmd_tlv(wmi_unified_t wmi_handle,
		struct wmi_adaptive_dwelltime_params *dwelltime_params)
{
	wmi_scan_adaptive_dwell_config_fixed_param *dwell_param;
	wmi_scan_adaptive_dwell_parameters_tlv *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t err;
	int len;

	len = sizeof(wmi_scan_adaptive_dwell_config_fixed_param);
	len += WMI_TLV_HDR_SIZE; /* TLV for ext_thresholds*/
	len += sizeof(wmi_scan_adaptive_dwell_parameters_tlv);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s :Failed to allocate buffer to send cmd",
				__func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	dwell_param = (wmi_scan_adaptive_dwell_config_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&dwell_param->tlv_header,
		WMITLV_TAG_STRUC_wmi_scan_adaptive_dwell_config_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_scan_adaptive_dwell_config_fixed_param));

	dwell_param->enable = dwelltime_params->is_enabled;
	buf_ptr += sizeof(wmi_scan_adaptive_dwell_config_fixed_param);
	WMITLV_SET_HDR(buf_ptr,
			WMITLV_TAG_ARRAY_STRUC,
			sizeof(wmi_scan_adaptive_dwell_parameters_tlv));
	buf_ptr += WMI_TLV_HDR_SIZE;

	cmd = (wmi_scan_adaptive_dwell_parameters_tlv *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_scan_adaptive_dwell_parameters_tlv,
			WMITLV_GET_STRUCT_TLVLEN(
				wmi_scan_adaptive_dwell_parameters_tlv));

	cmd->default_adaptive_dwell_mode = dwelltime_params->dwelltime_mode;
	cmd->adapative_lpf_weight = dwelltime_params->lpf_weight;
	cmd->passive_monitor_interval_ms = dwelltime_params->passive_mon_intval;
	cmd->wifi_activity_threshold_pct = dwelltime_params->wifi_act_threshold;
	err = wmi_unified_cmd_send(wmi_handle, buf,
			len, WMI_SCAN_ADAPTIVE_DWELL_CONFIG_CMDID);
	if (err) {
		WMI_LOGE("Failed to send adapt dwelltime cmd err=%d", err);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}


/**
 * send_roam_scan_filter_cmd_tlv() - Filter to be applied while roaming
 * @wmi_handle:     wmi handle
 * @roam_req:       Request which contains the filters
 *
 * There are filters such as whitelist, blacklist and preferred
 * list that need to be applied to the scan results to form the
 * probable candidates for roaming.
 *
 * Return: Return success upon succesfully passing the
 *         parameters to the firmware, otherwise failure.
 */
static QDF_STATUS send_roam_scan_filter_cmd_tlv(wmi_unified_t wmi_handle,
				struct roam_scan_filter_params *roam_req)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	uint32_t i;
	uint32_t len;
	uint8_t *buf_ptr;
	wmi_roam_filter_fixed_param *roam_filter;
	uint8_t *bssid_src_ptr = NULL;
	wmi_mac_addr *bssid_dst_ptr = NULL;
	wmi_ssid *ssid_ptr = NULL;
	uint32_t *bssid_preferred_factor_ptr = NULL;

	len = sizeof(wmi_roam_filter_fixed_param);
	len += WMI_TLV_HDR_SIZE;
	len += roam_req->len;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (u_int8_t *) wmi_buf_data(buf);
	roam_filter = (wmi_roam_filter_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&roam_filter->tlv_header,
		WMITLV_TAG_STRUC_wmi_roam_filter_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_roam_filter_fixed_param));
	/* fill in fixed values */
	roam_filter->vdev_id = roam_req->session_id;
	roam_filter->flags = 0;
	roam_filter->op_bitmap = roam_req->op_bitmap;
	roam_filter->num_bssid_black_list = roam_req->num_bssid_black_list;
	roam_filter->num_ssid_white_list = roam_req->num_ssid_white_list;
	roam_filter->num_bssid_preferred_list =
			roam_req->num_bssid_preferred_list;
	buf_ptr += sizeof(wmi_roam_filter_fixed_param);

	WMITLV_SET_HDR((buf_ptr),
		WMITLV_TAG_ARRAY_FIXED_STRUC,
		(roam_req->num_bssid_black_list * sizeof(wmi_mac_addr)));
	bssid_src_ptr = (uint8_t *)&roam_req->bssid_avoid_list;
	bssid_dst_ptr = (wmi_mac_addr *)(buf_ptr + WMI_TLV_HDR_SIZE);
	for (i = 0; i < roam_req->num_bssid_black_list; i++) {
		WMI_CHAR_ARRAY_TO_MAC_ADDR(bssid_src_ptr, bssid_dst_ptr);
		bssid_src_ptr += ATH_MAC_LEN;
		bssid_dst_ptr++;
	}
	buf_ptr += WMI_TLV_HDR_SIZE +
		(roam_req->num_bssid_black_list * sizeof(wmi_mac_addr));
	WMITLV_SET_HDR((buf_ptr),
		WMITLV_TAG_ARRAY_FIXED_STRUC,
		(roam_req->num_ssid_white_list * sizeof(wmi_ssid)));
	ssid_ptr = (wmi_ssid *)(buf_ptr + WMI_TLV_HDR_SIZE);
	for (i = 0; i < roam_req->num_ssid_white_list; i++) {
		qdf_mem_copy(&ssid_ptr->ssid,
			&roam_req->ssid_allowed_list[i].mac_ssid,
			roam_req->ssid_allowed_list[i].length);
		ssid_ptr->ssid_len = roam_req->ssid_allowed_list[i].length;
		ssid_ptr++;
	}
	buf_ptr += WMI_TLV_HDR_SIZE + (roam_req->num_ssid_white_list *
							sizeof(wmi_ssid));
	WMITLV_SET_HDR((buf_ptr),
		WMITLV_TAG_ARRAY_FIXED_STRUC,
		(roam_req->num_bssid_preferred_list * sizeof(wmi_mac_addr)));
	bssid_src_ptr = (uint8_t *)&roam_req->bssid_favored;
	bssid_dst_ptr = (wmi_mac_addr *)(buf_ptr + WMI_TLV_HDR_SIZE);
	for (i = 0; i < roam_req->num_bssid_preferred_list; i++) {
		WMI_CHAR_ARRAY_TO_MAC_ADDR(bssid_src_ptr,
				(wmi_mac_addr *)bssid_dst_ptr);
		bssid_src_ptr += ATH_MAC_LEN;
		bssid_dst_ptr++;
	}
	buf_ptr += WMI_TLV_HDR_SIZE +
		(roam_req->num_bssid_preferred_list * sizeof(wmi_mac_addr));
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		(roam_req->num_bssid_preferred_list * sizeof(uint32_t)));
	bssid_preferred_factor_ptr = (uint32_t *)(buf_ptr + WMI_TLV_HDR_SIZE);
	for (i = 0; i < roam_req->num_bssid_preferred_list; i++) {
		*bssid_preferred_factor_ptr =
			roam_req->bssid_favored_factor[i];
		bssid_preferred_factor_ptr++;
	}
	buf_ptr += WMI_TLV_HDR_SIZE +
		(roam_req->num_bssid_preferred_list * sizeof(uint32_t));

	status = wmi_unified_cmd_send(wmi_handle, buf,
		len, WMI_ROAM_FILTER_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("cmd WMI_ROAM_FILTER_CMDID returned Error %d",
				status);
		wmi_buf_free(buf);
	}

	return status;
}

/** send_set_epno_network_list_cmd_tlv() - set epno network list
 * @wmi_handle: wmi handle
 * @req: epno config params request structure
 *
 * This function reads the incoming epno config request structure
 * and constructs the WMI message to the firmware.
 *
 * Returns: 0 on success, error number otherwise
 */
static QDF_STATUS send_set_epno_network_list_cmd_tlv(wmi_unified_t wmi_handle,
		struct wifi_enhanched_pno_params *req)
{
	wmi_nlo_config_cmd_fixed_param *cmd;
	nlo_configured_parameters *nlo_list;
	enlo_candidate_score_params *cand_score_params;
	u_int8_t i, *buf_ptr;
	wmi_buf_t buf;
	uint32_t len;
	QDF_STATUS ret;

	/* Fixed Params */
	len = sizeof(*cmd);
	if (req->num_networks) {
		/* TLV place holder for array of structures
		 * then each nlo_configured_parameters(nlo_list) TLV.
		 */
		len += WMI_TLV_HDR_SIZE;
		len += (sizeof(nlo_configured_parameters)
			    * QDF_MIN(req->num_networks, WMI_NLO_MAX_SSIDS));
		/* TLV for array of uint32 channel_list */
		len += WMI_TLV_HDR_SIZE;
		/* TLV for nlo_channel_prediction_cfg */
		len += WMI_TLV_HDR_SIZE;
		/* TLV for candidate score params */
		len += sizeof(enlo_candidate_score_params);
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_nlo_config_cmd_fixed_param *) wmi_buf_data(buf);

	buf_ptr = (u_int8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_nlo_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			       wmi_nlo_config_cmd_fixed_param));
	cmd->vdev_id = req->session_id;

	/* set flag to reset if num of networks are 0 */
	cmd->flags = (req->num_networks == 0 ?
		WMI_NLO_CONFIG_ENLO_RESET : WMI_NLO_CONFIG_ENLO);

	buf_ptr += sizeof(wmi_nlo_config_cmd_fixed_param);

	cmd->no_of_ssids = QDF_MIN(req->num_networks, WMI_NLO_MAX_SSIDS);
	WMI_LOGD("SSID count: %d flags: %d",
		cmd->no_of_ssids, cmd->flags);

	/* Fill nlo_config only when num_networks are non zero */
	if (cmd->no_of_ssids) {
		/* Fill networks */
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			cmd->no_of_ssids * sizeof(nlo_configured_parameters));
		buf_ptr += WMI_TLV_HDR_SIZE;

		nlo_list = (nlo_configured_parameters *) buf_ptr;
		for (i = 0; i < cmd->no_of_ssids; i++) {
			WMITLV_SET_HDR(&nlo_list[i].tlv_header,
				WMITLV_TAG_ARRAY_BYTE,
				WMITLV_GET_STRUCT_TLVLEN(
				nlo_configured_parameters));
			/* Copy ssid and it's length */
			nlo_list[i].ssid.valid = true;
			nlo_list[i].ssid.ssid.ssid_len =
				req->networks[i].ssid.length;
			qdf_mem_copy(nlo_list[i].ssid.ssid.ssid,
				     req->networks[i].ssid.mac_ssid,
				     nlo_list[i].ssid.ssid.ssid_len);
			WMI_LOGD("index: %d ssid: %.*s len: %d", i,
				 nlo_list[i].ssid.ssid.ssid_len,
				 (char *) nlo_list[i].ssid.ssid.ssid,
				 nlo_list[i].ssid.ssid.ssid_len);

			/* Copy pno flags */
			nlo_list[i].bcast_nw_type.valid = true;
			nlo_list[i].bcast_nw_type.bcast_nw_type =
					req->networks[i].flags;
			WMI_LOGD("PNO flags (%u)",
				nlo_list[i].bcast_nw_type.bcast_nw_type);

			/* Copy auth bit field */
			nlo_list[i].auth_type.valid = true;
			nlo_list[i].auth_type.auth_type =
					req->networks[i].auth_bit_field;
			WMI_LOGD("Auth bit field (%u)",
					nlo_list[i].auth_type.auth_type);
		}

		buf_ptr += cmd->no_of_ssids * sizeof(nlo_configured_parameters);
		/* Fill the channel list */
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, 0);
		buf_ptr += WMI_TLV_HDR_SIZE;

		/* Fill prediction_param */
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
		buf_ptr += WMI_TLV_HDR_SIZE;

		/* Fill epno candidate score params */
		cand_score_params = (enlo_candidate_score_params *) buf_ptr;
		WMITLV_SET_HDR(buf_ptr,
			WMITLV_TAG_STRUC_enlo_candidate_score_param,
			WMITLV_GET_STRUCT_TLVLEN(enlo_candidate_score_params));
		cand_score_params->min5GHz_rssi =
			req->min_5ghz_rssi;
		cand_score_params->min24GHz_rssi =
			req->min_24ghz_rssi;
		cand_score_params->initial_score_max =
			req->initial_score_max;
		cand_score_params->current_connection_bonus =
			req->current_connection_bonus;
		cand_score_params->same_network_bonus =
			req->same_network_bonus;
		cand_score_params->secure_bonus =
			req->secure_bonus;
		cand_score_params->band5GHz_bonus =
			req->band_5ghz_bonus;
		buf_ptr += sizeof(enlo_candidate_score_params);
	}

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_NETWORK_LIST_OFFLOAD_CONFIG_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send nlo wmi cmd", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_INVAL;
	}

	WMI_LOGD("set ePNO list request sent successfully for vdev %d",
		 req->session_id);

	return ret;
}


/** send_ipa_offload_control_cmd_tlv() - ipa offload control parameter
 * @wmi_handle: wmi handle
 * @ipa_offload: ipa offload control parameter
 *
 * Returns: 0 on success, error number otherwise
 */
static QDF_STATUS send_ipa_offload_control_cmd_tlv(wmi_unified_t wmi_handle,
		struct ipa_offload_control_params *ipa_offload)
{
	wmi_ipa_offload_enable_disable_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	u_int8_t *buf_ptr;

	len  = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed (len=%d)", __func__, len);
		return QDF_STATUS_E_NOMEM;
	}

	WMI_LOGE("%s: offload_type=%d, enable=%d", __func__,
		ipa_offload->offload_type, ipa_offload->enable);

	buf_ptr = (u_int8_t *)wmi_buf_data(wmi_buf);

	cmd = (wmi_ipa_offload_enable_disable_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUCT_wmi_ipa_offload_enable_disable_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		wmi_ipa_offload_enable_disable_cmd_fixed_param));

	cmd->offload_type = ipa_offload->offload_type;
	cmd->vdev_id = ipa_offload->vdev_id;
	cmd->enable = ipa_offload->enable;

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
		WMI_IPA_OFFLOAD_ENABLE_DISABLE_CMDID)) {
		WMI_LOGE("%s: failed to command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_extscan_get_capabilities_cmd_tlv() - extscan get capabilities
 * @wmi_handle: wmi handle
 * @pgetcapab: get capabilities params
 *
 * This function send request to fw to get extscan capabilities.
 *
 * Return: CDF status
 */
static QDF_STATUS send_extscan_get_capabilities_cmd_tlv(wmi_unified_t wmi_handle,
		    struct extscan_capabilities_params *pgetcapab)
{
	wmi_extscan_get_capabilities_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;

	len = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_extscan_get_capabilities_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_extscan_get_capabilities_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (wmi_extscan_get_capabilities_cmd_fixed_param));

	cmd->request_id = pgetcapab->request_id;

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_EXTSCAN_GET_CAPABILITIES_CMDID)) {
		WMI_LOGE("%s: failed to  command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_extscan_get_cached_results_cmd_tlv() - extscan get cached results
 * @wmi_handle: wmi handle
 * @pcached_results: cached results parameters
 *
 * This function send request to fw to get cached results.
 *
 * Return: CDF status
 */
static QDF_STATUS send_extscan_get_cached_results_cmd_tlv(wmi_unified_t wmi_handle,
		  struct extscan_cached_result_params *pcached_results)
{
	wmi_extscan_get_cached_results_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;

	len = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_extscan_get_cached_results_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_extscan_get_cached_results_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_extscan_get_cached_results_cmd_fixed_param));

	cmd->request_id = pcached_results->request_id;
	cmd->vdev_id = pcached_results->session_id;
	cmd->control_flags = pcached_results->flush;

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_EXTSCAN_GET_CACHED_RESULTS_CMDID)) {
		WMI_LOGE("%s: failed to  command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_extscan_stop_change_monitor_cmd_tlv() - send stop change monitor cmd
 * @wmi_handle: wmi handle
 * @reset_req: Reset change request params
 *
 * This function sends stop change monitor request to fw.
 *
 * Return: CDF status
 */
static QDF_STATUS send_extscan_stop_change_monitor_cmd_tlv(wmi_unified_t wmi_handle,
			struct extscan_capabilities_reset_params *reset_req)
{
	wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;
	int change_list = 0;

	len = sizeof(*cmd);

	/* reset significant change tlv is set to 0 */
	len += WMI_TLV_HDR_SIZE;
	len += change_list * sizeof(wmi_extscan_wlan_change_bssid_param);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param *)
		buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param));

	cmd->request_id = reset_req->request_id;
	cmd->vdev_id = reset_req->session_id;
	cmd->mode = 0;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       change_list *
		       sizeof(wmi_extscan_wlan_change_bssid_param));
	buf_ptr += WMI_TLV_HDR_SIZE + (change_list *
				       sizeof
				       (wmi_extscan_wlan_change_bssid_param));

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
			 WMI_EXTSCAN_CONFIGURE_WLAN_CHANGE_MONITOR_CMDID)) {
		WMI_LOGE("%s: failed to  command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_get_buf_extscan_change_monitor_cmd() - fill change monitor request
 * @wmi_handle: wmi handle
 * @psigchange: change monitor request params
 * @buf: wmi buffer
 * @buf_len: buffer length
 *
 * This function fills elements of change monitor request buffer.
 *
 * Return: CDF status
 */
static QDF_STATUS wmi_get_buf_extscan_change_monitor_cmd(wmi_unified_t wmi_handle,
			struct extscan_set_sig_changereq_params
			*psigchange, wmi_buf_t *buf, int *buf_len)
{
	wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param *cmd;
	wmi_extscan_wlan_change_bssid_param *dest_chglist;
	uint8_t *buf_ptr;
	int j;
	int len = sizeof(*cmd);
	uint32_t numap = psigchange->num_ap;
	struct ap_threshold_params *src_ap = psigchange->ap;

	if (!numap || (numap > WMI_WLAN_EXTSCAN_MAX_SIGNIFICANT_CHANGE_APS)) {
		WMI_LOGE("%s: Invalid number of bssid's", __func__);
		return QDF_STATUS_E_INVAL;
	}
	len += WMI_TLV_HDR_SIZE;
	len += numap * sizeof(wmi_extscan_wlan_change_bssid_param);

	*buf = wmi_buf_alloc(wmi_handle, len);
	if (!*buf) {
		WMI_LOGP("%s: failed to allocate memory for change monitor cmd",
			 __func__);
		return QDF_STATUS_E_FAILURE;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(*buf);
	cmd =
		(wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param *)
		buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param));

	cmd->request_id = psigchange->request_id;
	cmd->vdev_id = psigchange->session_id;
	cmd->total_entries = numap;
	cmd->mode = 1;
	cmd->num_entries_in_page = numap;
	cmd->lost_ap_scan_count = psigchange->lostap_sample_size;
	cmd->max_rssi_samples = psigchange->rssi_sample_size;
	cmd->rssi_averaging_samples = psigchange->rssi_sample_size;
	cmd->max_out_of_range_count = psigchange->min_breaching;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       numap * sizeof(wmi_extscan_wlan_change_bssid_param));
	dest_chglist = (wmi_extscan_wlan_change_bssid_param *)
		       (buf_ptr + WMI_TLV_HDR_SIZE);

	for (j = 0; j < numap; j++) {
		WMITLV_SET_HDR(dest_chglist,
		       WMITLV_TAG_STRUC_wmi_extscan_bucket_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_extscan_wlan_change_bssid_param));

		dest_chglist->lower_rssi_limit = src_ap->low;
		dest_chglist->upper_rssi_limit = src_ap->high;
		WMI_CHAR_ARRAY_TO_MAC_ADDR(src_ap->bssid.bytes,
					   &dest_chglist->bssid);

		WMI_LOGD("%s: min_rssi %d", __func__,
			 dest_chglist->lower_rssi_limit);
		dest_chglist++;
		src_ap++;
	}
	buf_ptr += WMI_TLV_HDR_SIZE +
		   (numap * sizeof(wmi_extscan_wlan_change_bssid_param));
	*buf_len = len;
	return QDF_STATUS_SUCCESS;
}

/**
 * send_extscan_start_change_monitor_cmd_tlv() - send start change monitor cmd
 * @wmi_handle: wmi handle
 * @psigchange: change monitor request params
 *
 * This function sends start change monitor request to fw.
 *
 * Return: CDF status
 */
static QDF_STATUS send_extscan_start_change_monitor_cmd_tlv(wmi_unified_t wmi_handle,
			   struct extscan_set_sig_changereq_params *
			   psigchange)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	wmi_buf_t buf;
	int len;


	qdf_status = wmi_get_buf_extscan_change_monitor_cmd(wmi_handle,
			     psigchange, &buf,
			     &len);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s: Failed to get buffer for change monitor cmd",
			 __func__);
		return QDF_STATUS_E_FAILURE;
	}
	if (!buf) {
		WMI_LOGE("%s: Failed to get buffer", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
		 WMI_EXTSCAN_CONFIGURE_WLAN_CHANGE_MONITOR_CMDID)) {
		WMI_LOGE("%s: failed to send command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_extscan_stop_hotlist_monitor_cmd_tlv() - stop hotlist monitor
 * @wmi_handle: wmi handle
 * @photlist_reset: hotlist reset params
 *
 * This function configures hotlist monitor to stop in fw.
 *
 * Return: CDF status
 */
static QDF_STATUS send_extscan_stop_hotlist_monitor_cmd_tlv(wmi_unified_t wmi_handle,
		  struct extscan_bssid_hotlist_reset_params *photlist_reset)
{
	wmi_extscan_configure_hotlist_monitor_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;
	int hotlist_entries = 0;

	len = sizeof(*cmd);

	/* reset bssid hotlist with tlv set to 0 */
	len += WMI_TLV_HDR_SIZE;
	len += hotlist_entries * sizeof(wmi_extscan_hotlist_entry);

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_extscan_configure_hotlist_monitor_cmd_fixed_param *)
	      buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_extscan_configure_hotlist_monitor_cmd_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN
	(wmi_extscan_configure_hotlist_monitor_cmd_fixed_param));

	cmd->request_id = photlist_reset->request_id;
	cmd->vdev_id = photlist_reset->session_id;
	cmd->mode = 0;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       hotlist_entries * sizeof(wmi_extscan_hotlist_entry));
	buf_ptr += WMI_TLV_HDR_SIZE +
		   (hotlist_entries * sizeof(wmi_extscan_hotlist_entry));

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_EXTSCAN_CONFIGURE_HOTLIST_MONITOR_CMDID)) {
		WMI_LOGE("%s: failed to  command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_stop_extscan_cmd_tlv() - stop extscan command to fw.
 * @wmi_handle: wmi handle
 * @pstopcmd: stop scan command request params
 *
 * This function sends stop extscan request to fw.
 *
 * Return: CDF Status.
 */
static QDF_STATUS send_stop_extscan_cmd_tlv(wmi_unified_t wmi_handle,
			  struct extscan_stop_req_params *pstopcmd)
{
	wmi_extscan_stop_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;

	len = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_extscan_stop_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_extscan_stop_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_extscan_stop_cmd_fixed_param));

	cmd->request_id = pstopcmd->request_id;
	cmd->vdev_id = pstopcmd->session_id;

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_EXTSCAN_STOP_CMDID)) {
		WMI_LOGE("%s: failed to  command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_get_buf_extscan_start_cmd() - Fill extscan start request
 * @wmi_handle: wmi handle
 * @pstart: scan command request params
 * @buf: event buffer
 * @buf_len: length of buffer
 *
 * This function fills individual elements of extscan request and
 * TLV for buckets, channel list.
 *
 * Return: CDF Status.
 */
static
QDF_STATUS wmi_get_buf_extscan_start_cmd(wmi_unified_t wmi_handle,
			 struct wifi_scan_cmd_req_params *pstart,
			 wmi_buf_t *buf, int *buf_len)
{
	wmi_extscan_start_cmd_fixed_param *cmd;
	wmi_extscan_bucket *dest_blist;
	wmi_extscan_bucket_channel *dest_clist;
	struct wifi_scan_bucket_params *src_bucket = pstart->buckets;
	struct wifi_scan_channelspec_params *src_channel = src_bucket->channels;
	struct wifi_scan_channelspec_params save_channel[WMI_WLAN_EXTSCAN_MAX_CHANNELS];

	uint8_t *buf_ptr;
	int i, k, count = 0;
	int len = sizeof(*cmd);
	int nbuckets = pstart->numBuckets;
	int nchannels = 0;

	/* These TLV's are are NULL by default */
	uint32_t ie_len_with_pad = 0;
	int num_ssid = 0;
	int num_bssid = 0;
	int ie_len = 0;

	uint32_t base_period = pstart->basePeriod;

	/* TLV placeholder for ssid_list (NULL) */
	len += WMI_TLV_HDR_SIZE;
	len += num_ssid * sizeof(wmi_ssid);

	/* TLV placeholder for bssid_list (NULL) */
	len += WMI_TLV_HDR_SIZE;
	len += num_bssid * sizeof(wmi_mac_addr);

	/* TLV placeholder for ie_data (NULL) */
	len += WMI_TLV_HDR_SIZE;
	len += ie_len * sizeof(uint32_t);

	/* TLV placeholder for bucket */
	len += WMI_TLV_HDR_SIZE;
	len += nbuckets * sizeof(wmi_extscan_bucket);

	/* TLV channel placeholder */
	len += WMI_TLV_HDR_SIZE;
	for (i = 0; i < nbuckets; i++) {
		nchannels += src_bucket->numChannels;
		src_bucket++;
	}

	WMI_LOGD("%s: Total buckets: %d total #of channels is %d",
		__func__, nbuckets, nchannels);
	len += nchannels * sizeof(wmi_extscan_bucket_channel);
	/* Allocate the memory */
	*buf = wmi_buf_alloc(wmi_handle, len);
	if (!*buf) {
		WMI_LOGP("%s: failed to allocate memory"
			 " for start extscan cmd", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(*buf);
	cmd = (wmi_extscan_start_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_extscan_start_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_extscan_start_cmd_fixed_param));

	cmd->request_id = pstart->requestId;
	cmd->vdev_id = pstart->sessionId;
	cmd->base_period = pstart->basePeriod;
	cmd->num_buckets = nbuckets;
	cmd->configuration_flags = 0;
	if (pstart->configuration_flags & WMI_EXTSCAN_LP_EXTENDED_BATCHING)
		cmd->configuration_flags |= WMI_EXTSCAN_EXTENDED_BATCHING_EN;
	WMI_LOGI("%s: configuration_flags: 0x%x", __func__,
			cmd->configuration_flags);
#ifdef FEATURE_WLAN_EXTSCAN
	cmd->min_rest_time = WMI_EXTSCAN_REST_TIME;
	cmd->max_rest_time = WMI_EXTSCAN_REST_TIME;
	cmd->max_scan_time = WMI_EXTSCAN_MAX_SCAN_TIME;
	cmd->burst_duration = WMI_EXTSCAN_BURST_DURATION;
#endif
	cmd->max_bssids_per_scan_cycle = pstart->maxAPperScan;

	/* The max dwell time is retrieved from the first channel
	 * of the first bucket and kept common for all channels.
	 */
	cmd->min_dwell_time_active = pstart->min_dwell_time_active;
	cmd->max_dwell_time_active = pstart->max_dwell_time_active;
	cmd->min_dwell_time_passive = pstart->min_dwell_time_passive;
	cmd->max_dwell_time_passive = pstart->max_dwell_time_passive;
	cmd->max_bssids_per_scan_cycle = pstart->maxAPperScan;
	cmd->max_table_usage = pstart->report_threshold_percent;
	cmd->report_threshold_num_scans = pstart->report_threshold_num_scans;

	cmd->repeat_probe_time = cmd->max_dwell_time_active /
					WMI_SCAN_NPROBES_DEFAULT;
	cmd->probe_delay = 0;
	cmd->probe_spacing_time = 0;
	cmd->idle_time = 0;
	cmd->scan_ctrl_flags = WMI_SCAN_ADD_BCAST_PROBE_REQ |
			       WMI_SCAN_ADD_CCK_RATES |
			       WMI_SCAN_ADD_OFDM_RATES |
			       WMI_SCAN_ADD_SPOOFED_MAC_IN_PROBE_REQ |
			       WMI_SCAN_ADD_DS_IE_IN_PROBE_REQ;
	WMI_SCAN_SET_DWELL_MODE(cmd->scan_ctrl_flags,
			pstart->extscan_adaptive_dwell_mode);
	cmd->scan_priority = WMI_SCAN_PRIORITY_VERY_LOW;
	cmd->num_ssids = 0;
	cmd->num_bssid = 0;
	cmd->ie_len = 0;
	cmd->n_probes = (cmd->repeat_probe_time > 0) ?
			cmd->max_dwell_time_active / cmd->repeat_probe_time : 0;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_FIXED_STRUC,
		       num_ssid * sizeof(wmi_ssid));
	buf_ptr += WMI_TLV_HDR_SIZE + (num_ssid * sizeof(wmi_ssid));

	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_FIXED_STRUC,
		       num_bssid * sizeof(wmi_mac_addr));
	buf_ptr += WMI_TLV_HDR_SIZE + (num_bssid * sizeof(wmi_mac_addr));

	ie_len_with_pad = 0;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
			  ie_len_with_pad);
	buf_ptr += WMI_TLV_HDR_SIZE + ie_len_with_pad;

	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       nbuckets * sizeof(wmi_extscan_bucket));
	dest_blist = (wmi_extscan_bucket *)
		     (buf_ptr + WMI_TLV_HDR_SIZE);
	src_bucket = pstart->buckets;

	/* Retrieve scanning information from each bucket and
	 * channels and send it to the target
	 */
	for (i = 0; i < nbuckets; i++) {
		WMITLV_SET_HDR(dest_blist,
		      WMITLV_TAG_STRUC_wmi_extscan_bucket_cmd_fixed_param,
		      WMITLV_GET_STRUCT_TLVLEN(wmi_extscan_bucket));

		dest_blist->bucket_id = src_bucket->bucket;
		dest_blist->base_period_multiplier =
			src_bucket->period / base_period;
		dest_blist->min_period = src_bucket->period;
		dest_blist->max_period = src_bucket->max_period;
		dest_blist->exp_backoff = src_bucket->exponent;
		dest_blist->exp_max_step_count = src_bucket->step_count;
		dest_blist->channel_band = src_bucket->band;
		dest_blist->num_channels = src_bucket->numChannels;
		dest_blist->notify_extscan_events = 0;

		if (src_bucket->reportEvents & WMI_EXTSCAN_REPORT_EVENTS_EACH_SCAN)
			dest_blist->notify_extscan_events =
					WMI_EXTSCAN_CYCLE_COMPLETED_EVENT |
					WMI_EXTSCAN_CYCLE_STARTED_EVENT;

		if (src_bucket->reportEvents &
				WMI_EXTSCAN_REPORT_EVENTS_FULL_RESULTS) {
			dest_blist->forwarding_flags =
				WMI_EXTSCAN_FORWARD_FRAME_TO_HOST;
			dest_blist->notify_extscan_events |=
				WMI_EXTSCAN_BUCKET_COMPLETED_EVENT |
				WMI_EXTSCAN_CYCLE_STARTED_EVENT |
				WMI_EXTSCAN_CYCLE_COMPLETED_EVENT;
		} else {
			dest_blist->forwarding_flags =
				WMI_EXTSCAN_NO_FORWARDING;
		}

		if (src_bucket->reportEvents & WMI_EXTSCAN_REPORT_EVENTS_NO_BATCH)
			dest_blist->configuration_flags = 0;
		else
			dest_blist->configuration_flags =
				WMI_EXTSCAN_BUCKET_CACHE_RESULTS;

		WMI_LOGI("%s: ntfy_extscan_events:%u cfg_flags:%u fwd_flags:%u",
			__func__, dest_blist->notify_extscan_events,
			dest_blist->configuration_flags,
			dest_blist->forwarding_flags);

		dest_blist->min_dwell_time_active =
				   src_bucket->min_dwell_time_active;
		dest_blist->max_dwell_time_active =
				   src_bucket->max_dwell_time_active;
		dest_blist->min_dwell_time_passive =
				   src_bucket->min_dwell_time_passive;
		dest_blist->max_dwell_time_passive =
				   src_bucket->max_dwell_time_passive;
		src_channel = src_bucket->channels;

		/* save the channel info to later populate
		 * the  channel TLV
		 */
		for (k = 0; k < src_bucket->numChannels; k++) {
			save_channel[count++].channel = src_channel->channel;
			src_channel++;
		}
		dest_blist++;
		src_bucket++;
	}
	buf_ptr += WMI_TLV_HDR_SIZE + (nbuckets * sizeof(wmi_extscan_bucket));
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       nchannels * sizeof(wmi_extscan_bucket_channel));
	dest_clist = (wmi_extscan_bucket_channel *)
		     (buf_ptr + WMI_TLV_HDR_SIZE);

	/* Active or passive scan is based on the bucket dwell time
	 * and channel specific active,passive scans are not
	 * supported yet
	 */
	for (i = 0; i < nchannels; i++) {
		WMITLV_SET_HDR(dest_clist,
		WMITLV_TAG_STRUC_wmi_extscan_bucket_channel_event_fixed_param,
			   WMITLV_GET_STRUCT_TLVLEN
			   (wmi_extscan_bucket_channel));
		dest_clist->channel = save_channel[i].channel;
		dest_clist++;
	}
	buf_ptr += WMI_TLV_HDR_SIZE +
		   (nchannels * sizeof(wmi_extscan_bucket_channel));
	*buf_len = len;
	return QDF_STATUS_SUCCESS;
}

/**
 * send_start_extscan_cmd_tlv() - start extscan command to fw.
 * @wmi_handle: wmi handle
 * @pstart: scan command request params
 *
 * This function sends start extscan request to fw.
 *
 * Return: CDF Status.
 */
static QDF_STATUS send_start_extscan_cmd_tlv(wmi_unified_t wmi_handle,
			  struct wifi_scan_cmd_req_params *pstart)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	wmi_buf_t buf;
	int len;

	/* Fill individual elements of extscan request and
	 * TLV for buckets, channel list.
	 */
	qdf_status = wmi_get_buf_extscan_start_cmd(wmi_handle,
			     pstart, &buf, &len);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s: Failed to get buffer for ext scan cmd", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	if (!buf) {
		WMI_LOGE("%s:Failed to get buffer"
			 "for current extscan info", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	if (wmi_unified_cmd_send(wmi_handle, buf,
				 len, WMI_EXTSCAN_START_CMDID)) {
		WMI_LOGE("%s: failed to send command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_plm_stop_cmd_tlv() - plm stop request
 * @wmi_handle: wmi handle
 * @plm: plm request parameters
 *
 * This function request FW to stop PLM.
 *
 * Return: CDF status
 */
static QDF_STATUS send_plm_stop_cmd_tlv(wmi_unified_t wmi_handle,
			  const struct plm_req_params *plm)
{
	wmi_vdev_plmreq_stop_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_vdev_plmreq_stop_cmd_fixed_param *) wmi_buf_data(buf);

	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_plmreq_stop_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_vdev_plmreq_stop_cmd_fixed_param));

	cmd->vdev_id = plm->session_id;

	cmd->meas_token = plm->meas_token;
	WMI_LOGD("vdev %d meas token %d", cmd->vdev_id, cmd->meas_token);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_PLMREQ_STOP_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send plm stop wmi cmd", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_plm_start_cmd_tlv() - plm start request
 * @wmi_handle: wmi handle
 * @plm: plm request parameters
 *
 * This function request FW to start PLM.
 *
 * Return: CDF status
 */
static QDF_STATUS send_plm_start_cmd_tlv(wmi_unified_t wmi_handle,
			  const struct plm_req_params *plm,
			  uint32_t *gchannel_list)
{
	wmi_vdev_plmreq_start_cmd_fixed_param *cmd;
	uint32_t *channel_list;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint8_t count;
	int ret;

	/* TLV place holder for channel_list */
	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += sizeof(uint32_t) * plm->plm_num_ch;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_plmreq_start_cmd_fixed_param *) wmi_buf_data(buf);

	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_plmreq_start_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_plmreq_start_cmd_fixed_param));

	cmd->vdev_id = plm->session_id;

	cmd->meas_token = plm->meas_token;
	cmd->dialog_token = plm->diag_token;
	cmd->number_bursts = plm->num_bursts;
	cmd->burst_interval = WMI_SEC_TO_MSEC(plm->burst_int);
	cmd->off_duration = plm->meas_duration;
	cmd->burst_cycle = plm->burst_len;
	cmd->tx_power = plm->desired_tx_pwr;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(plm->mac_addr.bytes, &cmd->dest_mac);
	cmd->num_chans = plm->plm_num_ch;

	buf_ptr += sizeof(wmi_vdev_plmreq_start_cmd_fixed_param);

	WMI_LOGD("vdev : %d measu token : %d", cmd->vdev_id, cmd->meas_token);
	WMI_LOGD("dialog_token: %d", cmd->dialog_token);
	WMI_LOGD("number_bursts: %d", cmd->number_bursts);
	WMI_LOGD("burst_interval: %d", cmd->burst_interval);
	WMI_LOGD("off_duration: %d", cmd->off_duration);
	WMI_LOGD("burst_cycle: %d", cmd->burst_cycle);
	WMI_LOGD("tx_power: %d", cmd->tx_power);
	WMI_LOGD("Number of channels : %d", cmd->num_chans);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (cmd->num_chans * sizeof(uint32_t)));

	buf_ptr += WMI_TLV_HDR_SIZE;
	if (cmd->num_chans) {
		channel_list = (uint32_t *) buf_ptr;
		for (count = 0; count < cmd->num_chans; count++) {
			channel_list[count] = plm->plm_ch_list[count];
			if (channel_list[count] < WMI_NLO_FREQ_THRESH)
				channel_list[count] =
					gchannel_list[count];
			WMI_LOGD("Ch[%d]: %d MHz", count, channel_list[count]);
		}
		buf_ptr += cmd->num_chans * sizeof(uint32_t);
	}

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_PLMREQ_START_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send plm start wmi cmd", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_pno_stop_cmd_tlv() - PNO stop request
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * This function request FW to stop ongoing PNO operation.
 *
 * Return: CDF status
 */
static QDF_STATUS send_pno_stop_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id)
{
	wmi_nlo_config_cmd_fixed_param *cmd;
	int32_t len = sizeof(*cmd);
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int ret;

	/*
	 * TLV place holder for array of structures nlo_configured_parameters
	 * TLV place holder for array of uint32_t channel_list
	 * TLV place holder for chnl prediction cfg
	 */
	len += WMI_TLV_HDR_SIZE + WMI_TLV_HDR_SIZE + WMI_TLV_HDR_SIZE;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_nlo_config_cmd_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_nlo_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_nlo_config_cmd_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->flags = WMI_NLO_CONFIG_STOP;
	buf_ptr += sizeof(*cmd);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;


	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_NETWORK_LIST_OFFLOAD_CONFIG_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send nlo wmi cmd", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_set_pno_channel_prediction() - Set PNO channel prediction
 * @buf_ptr:      Buffer passed by upper layers
 * @pno:          Buffer to be sent to the firmware
 *
 * Copy the PNO Channel prediction configuration parameters
 * passed by the upper layers to a WMI format TLV and send it
 * down to the firmware.
 *
 * Return: None
 */
static void wmi_set_pno_channel_prediction(uint8_t *buf_ptr,
		struct pno_scan_req_params *pno)
{
	nlo_channel_prediction_cfg *channel_prediction_cfg =
		(nlo_channel_prediction_cfg *) buf_ptr;
	WMITLV_SET_HDR(&channel_prediction_cfg->tlv_header,
			WMITLV_TAG_ARRAY_BYTE,
			WMITLV_GET_STRUCT_TLVLEN(nlo_channel_prediction_cfg));
#ifdef FEATURE_WLAN_SCAN_PNO
	channel_prediction_cfg->enable = pno->pno_channel_prediction;
	channel_prediction_cfg->top_k_num = pno->top_k_num_of_channels;
	channel_prediction_cfg->stationary_threshold = pno->stationary_thresh;
	channel_prediction_cfg->full_scan_period_ms =
		pno->channel_prediction_full_scan;
#endif
	buf_ptr += sizeof(nlo_channel_prediction_cfg);
	WMI_LOGD("enable: %d, top_k_num: %d, stat_thresh: %d, full_scan: %d",
			channel_prediction_cfg->enable,
			channel_prediction_cfg->top_k_num,
			channel_prediction_cfg->stationary_threshold,
			channel_prediction_cfg->full_scan_period_ms);
}

/**
 * send_pno_start_cmd_tlv() - PNO start request
 * @wmi_handle: wmi handle
 * @pno: PNO request
 *
 * This function request FW to start PNO request.
 * Request: CDF status
 */
static QDF_STATUS send_pno_start_cmd_tlv(wmi_unified_t wmi_handle,
		   struct pno_scan_req_params *pno)
{
	wmi_nlo_config_cmd_fixed_param *cmd;
	nlo_configured_parameters *nlo_list;
	uint32_t *channel_list;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint8_t i;
	int ret;

	/*
	 * TLV place holder for array nlo_configured_parameters(nlo_list)
	 * TLV place holder for array of uint32_t channel_list
	 * TLV place holder for chnnl prediction cfg
	 */
	len = sizeof(*cmd) +
		WMI_TLV_HDR_SIZE + WMI_TLV_HDR_SIZE + WMI_TLV_HDR_SIZE;

	len += sizeof(uint32_t) * QDF_MIN(pno->networks_list[0].channel_cnt,
					  WMI_NLO_MAX_CHAN);
	len += sizeof(nlo_configured_parameters) *
	       QDF_MIN(pno->networks_cnt, WMI_NLO_MAX_SSIDS);
	len += sizeof(nlo_channel_prediction_cfg);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_nlo_config_cmd_fixed_param *) wmi_buf_data(buf);

	buf_ptr = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_nlo_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_nlo_config_cmd_fixed_param));
	cmd->vdev_id = pno->vdev_id;
	cmd->flags = WMI_NLO_CONFIG_START | WMI_NLO_CONFIG_SSID_HIDE_EN;

#ifdef FEATURE_WLAN_SCAN_PNO
	WMI_SCAN_SET_DWELL_MODE(cmd->flags,
			pno->adaptive_dwell_mode);
#endif
	/* Current FW does not support min-max range for dwell time */
	cmd->active_dwell_time = pno->active_dwell_time;
	cmd->passive_dwell_time = pno->passive_dwell_time;

	/* Copy scan interval */
	cmd->fast_scan_period = pno->fast_scan_period;
	cmd->slow_scan_period = pno->slow_scan_period;
	cmd->delay_start_time = WMI_SEC_TO_MSEC(pno->delay_start_time);
	cmd->fast_scan_max_cycles = pno->fast_scan_max_cycles;
	WMI_LOGD("fast_scan_period: %d msec slow_scan_period: %d msec",
			cmd->fast_scan_period, cmd->slow_scan_period);
	WMI_LOGD("fast_scan_max_cycles: %d", cmd->fast_scan_max_cycles);

	buf_ptr += sizeof(wmi_nlo_config_cmd_fixed_param);

	cmd->no_of_ssids = QDF_MIN(pno->networks_cnt, WMI_NLO_MAX_SSIDS);
	WMI_LOGD("SSID count : %d", cmd->no_of_ssids);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       cmd->no_of_ssids * sizeof(nlo_configured_parameters));
	buf_ptr += WMI_TLV_HDR_SIZE;

	nlo_list = (nlo_configured_parameters *) buf_ptr;
	for (i = 0; i < cmd->no_of_ssids; i++) {
		WMITLV_SET_HDR(&nlo_list[i].tlv_header,
			       WMITLV_TAG_ARRAY_BYTE,
			       WMITLV_GET_STRUCT_TLVLEN
				       (nlo_configured_parameters));
		/* Copy ssid and it's length */
		nlo_list[i].ssid.valid = true;
		nlo_list[i].ssid.ssid.ssid_len =
			pno->networks_list[i].ssid.length;
		qdf_mem_copy(nlo_list[i].ssid.ssid.ssid,
			     pno->networks_list[i].ssid.ssid,
			     nlo_list[i].ssid.ssid.ssid_len);
		WMI_LOGD("index: %d ssid: %.*s len: %d", i,
			 nlo_list[i].ssid.ssid.ssid_len,
			 (char *)nlo_list[i].ssid.ssid.ssid,
			 nlo_list[i].ssid.ssid.ssid_len);

		/* Copy rssi threshold */
		if (pno->networks_list[i].rssi_thresh &&
		    pno->networks_list[i].rssi_thresh >
		    WMI_RSSI_THOLD_DEFAULT) {
			nlo_list[i].rssi_cond.valid = true;
			nlo_list[i].rssi_cond.rssi =
				pno->networks_list[i].rssi_thresh;
			WMI_LOGD("RSSI threshold : %d dBm",
				 nlo_list[i].rssi_cond.rssi);
		}
		nlo_list[i].bcast_nw_type.valid = true;
		nlo_list[i].bcast_nw_type.bcast_nw_type =
			pno->networks_list[i].bc_new_type;
		WMI_LOGI("Broadcast NW type (%u)",
			 nlo_list[i].bcast_nw_type.bcast_nw_type);
	}
	buf_ptr += cmd->no_of_ssids * sizeof(nlo_configured_parameters);

	/* Copy channel info */
	cmd->num_of_channels = QDF_MIN(pno->networks_list[0].channel_cnt,
				       WMI_NLO_MAX_CHAN);
	WMI_LOGD("Channel count: %d", cmd->num_of_channels);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (cmd->num_of_channels * sizeof(uint32_t)));
	buf_ptr += WMI_TLV_HDR_SIZE;

	channel_list = (uint32_t *) buf_ptr;
	for (i = 0; i < cmd->num_of_channels; i++) {
		channel_list[i] = pno->networks_list[0].channels[i];

		if (channel_list[i] < WMI_NLO_FREQ_THRESH)
			channel_list[i] =
				wlan_chan_to_freq(pno->
					networks_list[0].channels[i]);

		WMI_LOGD("Ch[%d]: %d MHz", i, channel_list[i]);
	}
	buf_ptr += cmd->num_of_channels * sizeof(uint32_t);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			sizeof(nlo_channel_prediction_cfg));
	buf_ptr += WMI_TLV_HDR_SIZE;
	wmi_set_pno_channel_prediction(buf_ptr, pno);
	buf_ptr += WMI_TLV_HDR_SIZE;
	/** TODO: Discrete firmware doesn't have command/option to configure
	 * App IE which comes from wpa_supplicant as of part PNO start request.
	 */
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_NETWORK_LIST_OFFLOAD_CONFIG_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send nlo wmi cmd", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/* send_set_ric_req_cmd_tlv() - set ric request element
 * @wmi_handle: wmi handle
 * @msg: message
 * @is_add_ts: is addts required
 *
 * This function sets ric request element for 11r roaming.
 *
 * Return: CDF status
 */
static QDF_STATUS send_set_ric_req_cmd_tlv(wmi_unified_t wmi_handle,
			void *msg, uint8_t is_add_ts)
{
	wmi_ric_request_fixed_param *cmd;
	wmi_ric_tspec *tspec_param;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	struct mac_tspec_ie *ptspecIE = NULL;
	int32_t len = sizeof(wmi_ric_request_fixed_param) +
		      WMI_TLV_HDR_SIZE + sizeof(wmi_ric_tspec);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);

	cmd = (wmi_ric_request_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		   WMITLV_TAG_STRUC_wmi_ric_request_fixed_param,
		   WMITLV_GET_STRUCT_TLVLEN(wmi_ric_request_fixed_param));
	if (is_add_ts)
		cmd->vdev_id = ((struct add_ts_param *) msg)->sessionId;
	else
		cmd->vdev_id = ((struct del_ts_params *) msg)->sessionId;
	cmd->num_ric_request = 1;
	cmd->is_add_ric = is_add_ts;

	buf_ptr += sizeof(wmi_ric_request_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, sizeof(wmi_ric_tspec));

	buf_ptr += WMI_TLV_HDR_SIZE;
	tspec_param = (wmi_ric_tspec *) buf_ptr;
	WMITLV_SET_HDR(&tspec_param->tlv_header,
		       WMITLV_TAG_STRUC_wmi_ric_tspec,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_ric_tspec));

	if (is_add_ts)
		ptspecIE = &(((struct add_ts_param *) msg)->tspec);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	else
		ptspecIE = &(((struct del_ts_params *) msg)->delTsInfo.tspec);
#endif
	if (ptspecIE) {
		/* Fill the tsinfo in the format expected by firmware */
#ifndef ANI_LITTLE_BIT_ENDIAN
		qdf_mem_copy(((uint8_t *) &tspec_param->ts_info) + 1,
			     ((uint8_t *) &ptspecIE->tsinfo) + 1, 2);
#else
		qdf_mem_copy(((uint8_t *) &tspec_param->ts_info),
			     ((uint8_t *) &ptspecIE->tsinfo) + 1, 2);
#endif /* ANI_LITTLE_BIT_ENDIAN */

		tspec_param->nominal_msdu_size = ptspecIE->nomMsduSz;
		tspec_param->maximum_msdu_size = ptspecIE->maxMsduSz;
		tspec_param->min_service_interval = ptspecIE->minSvcInterval;
		tspec_param->max_service_interval = ptspecIE->maxSvcInterval;
		tspec_param->inactivity_interval = ptspecIE->inactInterval;
		tspec_param->suspension_interval = ptspecIE->suspendInterval;
		tspec_param->svc_start_time = ptspecIE->svcStartTime;
		tspec_param->min_data_rate = ptspecIE->minDataRate;
		tspec_param->mean_data_rate = ptspecIE->meanDataRate;
		tspec_param->peak_data_rate = ptspecIE->peakDataRate;
		tspec_param->max_burst_size = ptspecIE->maxBurstSz;
		tspec_param->delay_bound = ptspecIE->delayBound;
		tspec_param->min_phy_rate = ptspecIE->minPhyRate;
		tspec_param->surplus_bw_allowance = ptspecIE->surplusBw;
		tspec_param->medium_time = 0;
	}
	WMI_LOGI("%s: Set RIC Req is_add_ts:%d", __func__, is_add_ts);

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_ROAM_SET_RIC_REQUEST_CMDID)) {
		WMI_LOGP("%s: Failed to send vdev Set RIC Req command",
			 __func__);
		if (is_add_ts)
			((struct add_ts_param *) msg)->status =
					    QDF_STATUS_E_FAILURE;
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_process_ll_stats_clear_cmd_tlv() - clear link layer stats
 * @wmi_handle: wmi handle
 * @clear_req: ll stats clear request command params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_process_ll_stats_clear_cmd_tlv(wmi_unified_t wmi_handle,
		const struct ll_stats_clear_params *clear_req,
		uint8_t addr[IEEE80211_ADDR_LEN])
{
	wmi_clear_link_stats_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_clear_link_stats_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_clear_link_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_clear_link_stats_cmd_fixed_param));

	cmd->stop_stats_collection_req = clear_req->stop_req;
	cmd->vdev_id = clear_req->sta_id;
	cmd->stats_clear_req_mask = clear_req->stats_clear_mask;

	WMI_CHAR_ARRAY_TO_MAC_ADDR(addr,
				   &cmd->peer_macaddr);

	WMI_LOGD("LINK_LAYER_STATS - Clear Request Params");
	WMI_LOGD("StopReq         : %d", cmd->stop_stats_collection_req);
	WMI_LOGD("Vdev Id         : %d", cmd->vdev_id);
	WMI_LOGD("Clear Stat Mask : %d", cmd->stats_clear_req_mask);
	/* WMI_LOGD("Peer MAC Addr   : %pM",
		 cmd->peer_macaddr); */

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_CLEAR_LINK_STATS_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send clear link stats req", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	WMI_LOGD("Clear Link Layer Stats request sent successfully");
	return QDF_STATUS_SUCCESS;
}

/**
 * send_process_ll_stats_set_cmd_tlv() - link layer stats set request
 * @wmi_handle:       wmi handle
 * @setReq:  ll stats set request command params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_process_ll_stats_set_cmd_tlv(wmi_unified_t wmi_handle,
		const struct ll_stats_set_params *set_req)
{
	wmi_start_link_stats_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_start_link_stats_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_start_link_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_start_link_stats_cmd_fixed_param));

	cmd->mpdu_size_threshold = set_req->mpdu_size_threshold;
	cmd->aggressive_statistics_gathering =
		set_req->aggressive_statistics_gathering;

	WMI_LOGD("LINK_LAYER_STATS - Start/Set Request Params");
	WMI_LOGD("MPDU Size Thresh : %d", cmd->mpdu_size_threshold);
	WMI_LOGD("Aggressive Gather: %d", cmd->aggressive_statistics_gathering);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_START_LINK_STATS_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send set link stats request", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_process_ll_stats_get_cmd_tlv() - link layer stats get request
 * @wmi_handle:wmi handle
 * @get_req:ll stats get request command params
 * @addr: mac address
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_process_ll_stats_get_cmd_tlv(wmi_unified_t wmi_handle,
		 const struct ll_stats_get_params  *get_req,
		 uint8_t addr[IEEE80211_ADDR_LEN])
{
	wmi_request_link_stats_cmd_fixed_param *cmd;
	int32_t len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s: buf allocation failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_request_link_stats_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_link_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_link_stats_cmd_fixed_param));

	cmd->request_id = get_req->req_id;
	cmd->stats_type = get_req->param_id_mask;
	cmd->vdev_id = get_req->sta_id;

	WMI_CHAR_ARRAY_TO_MAC_ADDR(addr,
				   &cmd->peer_macaddr);

	WMI_LOGD("LINK_LAYER_STATS - Get Request Params");
	WMI_LOGD("Request ID      : %u", cmd->request_id);
	WMI_LOGD("Stats Type      : %0x", cmd->stats_type);
	WMI_LOGD("Vdev ID         : %d", cmd->vdev_id);
	WMI_LOGD("Peer MAC Addr   : %pM", addr);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_REQUEST_LINK_STATS_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send get link stats request", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_get_stats_cmd_tlv() - get stats request
 * @wmi_handle: wmi handle
 * @get_stats_param: stats params
 * @addr: mac address
 *
 * Return: CDF status
 */
static QDF_STATUS send_get_stats_cmd_tlv(wmi_unified_t wmi_handle,
		       struct pe_stats_req  *get_stats_param,
			   uint8_t addr[IEEE80211_ADDR_LEN])
{
	wmi_buf_t buf;
	wmi_request_stats_cmd_fixed_param *cmd;
	uint8_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed to allocate wmi buffer", __func__);
		return QDF_STATUS_E_FAILURE;
	}


	cmd = (wmi_request_stats_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));
	cmd->stats_id =
		WMI_REQUEST_PEER_STAT | WMI_REQUEST_PDEV_STAT |
		WMI_REQUEST_VDEV_STAT | WMI_REQUEST_RSSI_PER_CHAIN_STAT;
	cmd->vdev_id = get_stats_param->session_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(addr, &cmd->peer_macaddr);
	WMI_LOGD("STATS REQ VDEV_ID:%d-->", cmd->vdev_id);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_STATS_CMDID)) {

		WMI_LOGE("%s: Failed to send WMI_REQUEST_STATS_CMDID",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;

}

/**
 * send_congestion_cmd_tlv() - send request to fw to get CCA
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: CDF status
 */
static QDF_STATUS send_congestion_cmd_tlv(wmi_unified_t wmi_handle,
			A_UINT8 vdev_id)
{
	wmi_buf_t buf;
	wmi_request_stats_cmd_fixed_param *cmd;
	uint8_t len;
	uint8_t *buf_ptr;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed to allocate wmi buffer", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = wmi_buf_data(buf);
	cmd = (wmi_request_stats_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));

	cmd->stats_id = WMI_REQUEST_CONGESTION_STAT;
	cmd->vdev_id = vdev_id;
	WMI_LOGD("STATS REQ VDEV_ID:%d stats_id %d -->",
			cmd->vdev_id, cmd->stats_id);

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_STATS_CMDID)) {
		WMI_LOGE("%s: Failed to send WMI_REQUEST_STATS_CMDID",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_snr_request_cmd_tlv() - send request to fw to get RSSI stats
 * @wmi_handle: wmi handle
 * @rssi_req: get RSSI request
 *
 * Return: CDF status
 */
static QDF_STATUS send_snr_request_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_buf_t buf;
	wmi_request_stats_cmd_fixed_param *cmd;
	uint8_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_request_stats_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));
	cmd->stats_id = WMI_REQUEST_VDEV_STAT;
	if (wmi_unified_cmd_send
		    (wmi_handle, buf, len, WMI_REQUEST_STATS_CMDID)) {
		WMI_LOGE("Failed to send host stats request to fw");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_snr_cmd_tlv() - get RSSI from fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: CDF status
 */
static QDF_STATUS send_snr_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id)
{
	wmi_buf_t buf;
	wmi_request_stats_cmd_fixed_param *cmd;
	uint8_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_request_stats_cmd_fixed_param *) wmi_buf_data(buf);
	cmd->vdev_id = vdev_id;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));
	cmd->stats_id = WMI_REQUEST_VDEV_STAT;
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_STATS_CMDID)) {
		WMI_LOGE("Failed to send host stats request to fw");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_link_status_req_cmd_tlv() - process link status request from UMAC
 * @wmi_handle: wmi handle
 * @link_status: get link params
 *
 * Return: CDF status
 */
static QDF_STATUS send_link_status_req_cmd_tlv(wmi_unified_t wmi_handle,
				 struct link_status_params *link_status)
{
	wmi_buf_t buf;
	wmi_request_stats_cmd_fixed_param *cmd;
	uint8_t len = sizeof(wmi_request_stats_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_request_stats_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_request_stats_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_request_stats_cmd_fixed_param));
	cmd->stats_id = WMI_REQUEST_VDEV_RATE_STAT;
	cmd->vdev_id = link_status->session_id;
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_REQUEST_STATS_CMDID)) {
		WMI_LOGE("Failed to send WMI link  status request to fw");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_process_dhcp_ind_cmd_tlv() - process dhcp indication from SME
 * @wmi_handle: wmi handle
 * @ta_dhcp_ind: DHCP indication parameter
 *
 * Return: CDF Status
 */
static QDF_STATUS send_process_dhcp_ind_cmd_tlv(wmi_unified_t wmi_handle,
				wmi_peer_set_param_cmd_fixed_param *ta_dhcp_ind)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_peer_set_param_cmd_fixed_param *peer_set_param_fp;
	int len = sizeof(wmi_peer_set_param_cmd_fixed_param);


	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	peer_set_param_fp = (wmi_peer_set_param_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&peer_set_param_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_set_param_cmd_fixed_param));

	/* fill in values */
	peer_set_param_fp->vdev_id = ta_dhcp_ind->vdev_id;
	peer_set_param_fp->param_id = ta_dhcp_ind->param_id;
	peer_set_param_fp->param_value = ta_dhcp_ind->param_value;
	qdf_mem_copy(&peer_set_param_fp->peer_macaddr,
				   &ta_dhcp_ind->peer_macaddr,
				   sizeof(ta_dhcp_ind->peer_macaddr));

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_PEER_SET_PARAM_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("%s: wmi_unified_cmd_send WMI_PEER_SET_PARAM_CMD"
			 " returned Error %d", __func__, status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_get_link_speed_cmd_tlv() -send command to get linkspeed
 * @wmi_handle: wmi handle
 * @pLinkSpeed: link speed info
 *
 * Return: CDF status
 */
static QDF_STATUS send_get_link_speed_cmd_tlv(wmi_unified_t wmi_handle,
		wmi_mac_addr peer_macaddr)
{
	wmi_peer_get_estimated_linkspeed_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;

	len = sizeof(wmi_peer_get_estimated_linkspeed_cmd_fixed_param);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_peer_get_estimated_linkspeed_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_peer_get_estimated_linkspeed_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (wmi_peer_get_estimated_linkspeed_cmd_fixed_param));

	/* Copy the peer macaddress to the wma buffer */
	qdf_mem_copy(&cmd->peer_macaddr,
				   &peer_macaddr,
				   sizeof(peer_macaddr));


	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_PEER_GET_ESTIMATED_LINKSPEED_CMDID)) {
		WMI_LOGE("%s: failed to send link speed command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_egap_conf_params_cmd_tlv() - send wmi cmd of egap configuration params
 * @wmi_handle:	 wmi handler
 * @egap_params: pointer to egap_params
 *
 * Return:	 0 for success, otherwise appropriate error code
 */
static QDF_STATUS send_egap_conf_params_cmd_tlv(wmi_unified_t wmi_handle,
		     wmi_ap_ps_egap_param_cmd_fixed_param *egap_params)
{
	wmi_ap_ps_egap_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t err;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGE("Failed to allocate buffer to send ap_ps_egap cmd");
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_ap_ps_egap_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_ap_ps_egap_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			       wmi_ap_ps_egap_param_cmd_fixed_param));

	cmd->enable = egap_params->enable;
	cmd->inactivity_time = egap_params->inactivity_time;
	cmd->wait_time = egap_params->wait_time;
	cmd->flags = egap_params->flags;
	err = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(*cmd), WMI_AP_PS_EGAP_PARAM_CMDID);
	if (err) {
		WMI_LOGE("Failed to send ap_ps_egap cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_fw_profiling_cmd_tlv() - send FW profiling cmd to WLAN FW
 * @wmi_handl: wmi handle
 * @cmd: Profiling command index
 * @value1: parameter1 value
 * @value2: parameter2 value
 *
 * Return: QDF_STATUS_SUCCESS for success else error code
 */
static QDF_STATUS send_fw_profiling_cmd_tlv(wmi_unified_t wmi_handle,
			uint32_t cmd, uint32_t value1, uint32_t value2)
{
	wmi_buf_t buf;
	int32_t len = 0;
	int ret;
	wmi_wlan_profile_trigger_cmd_fixed_param *prof_trig_cmd;
	wmi_wlan_profile_set_hist_intvl_cmd_fixed_param *hist_intvl_cmd;
	wmi_wlan_profile_enable_profile_id_cmd_fixed_param *profile_enable_cmd;
	wmi_wlan_profile_get_prof_data_cmd_fixed_param *profile_getdata_cmd;

	switch (cmd) {
	case WMI_WLAN_PROFILE_TRIGGER_CMDID:
		len = sizeof(wmi_wlan_profile_trigger_cmd_fixed_param);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			WMI_LOGP("%s: wmi_buf_alloc Failed", __func__);
			return QDF_STATUS_E_NOMEM;
		}
		prof_trig_cmd =
			(wmi_wlan_profile_trigger_cmd_fixed_param *)
				wmi_buf_data(buf);
		WMITLV_SET_HDR(&prof_trig_cmd->tlv_header,
		     WMITLV_TAG_STRUC_wmi_wlan_profile_trigger_cmd_fixed_param,
		     WMITLV_GET_STRUCT_TLVLEN
		    (wmi_wlan_profile_trigger_cmd_fixed_param));
		prof_trig_cmd->enable = value1;
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_WLAN_PROFILE_TRIGGER_CMDID);
		if (ret) {
			WMI_LOGE("PROFILE_TRIGGER cmd Failed with value %d",
					value1);
			wmi_buf_free(buf);
			return ret;
		}
		break;

	case WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID:
		len = sizeof(wmi_wlan_profile_get_prof_data_cmd_fixed_param);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			WMI_LOGP("%s: wmi_buf_alloc Failed", __func__);
			return QDF_STATUS_E_NOMEM;
		}
		profile_getdata_cmd =
			(wmi_wlan_profile_get_prof_data_cmd_fixed_param *)
				wmi_buf_data(buf);
		WMITLV_SET_HDR(&profile_getdata_cmd->tlv_header,
		      WMITLV_TAG_STRUC_wmi_wlan_profile_get_prof_data_cmd_fixed_param,
		      WMITLV_GET_STRUCT_TLVLEN
		      (wmi_wlan_profile_get_prof_data_cmd_fixed_param));
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID);
		if (ret) {
			WMI_LOGE("PROFILE_DATA cmd Failed for id %d value %d",
					value1, value2);
			wmi_buf_free(buf);
			return ret;
		}
		break;

	case WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID:
		len = sizeof(wmi_wlan_profile_set_hist_intvl_cmd_fixed_param);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			WMI_LOGP("%s: wmi_buf_alloc Failed", __func__);
			return QDF_STATUS_E_NOMEM;
		}
		hist_intvl_cmd =
			(wmi_wlan_profile_set_hist_intvl_cmd_fixed_param *)
				wmi_buf_data(buf);
		WMITLV_SET_HDR(&hist_intvl_cmd->tlv_header,
		      WMITLV_TAG_STRUC_wmi_wlan_profile_set_hist_intvl_cmd_fixed_param,
		      WMITLV_GET_STRUCT_TLVLEN
		      (wmi_wlan_profile_set_hist_intvl_cmd_fixed_param));
		hist_intvl_cmd->profile_id = value1;
		hist_intvl_cmd->value = value2;
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID);
		if (ret) {
			WMI_LOGE("HIST_INTVL cmd Failed for id %d value %d",
					value1, value2);
			wmi_buf_free(buf);
			return ret;
		}
		break;

	case WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID:
		len =
		sizeof(wmi_wlan_profile_enable_profile_id_cmd_fixed_param);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			WMI_LOGP("%s: wmi_buf_alloc Failed", __func__);
			return QDF_STATUS_E_NOMEM;
		}
		profile_enable_cmd =
			(wmi_wlan_profile_enable_profile_id_cmd_fixed_param *)
				wmi_buf_data(buf);
		WMITLV_SET_HDR(&profile_enable_cmd->tlv_header,
		      WMITLV_TAG_STRUC_wmi_wlan_profile_enable_profile_id_cmd_fixed_param,
		      WMITLV_GET_STRUCT_TLVLEN
		      (wmi_wlan_profile_enable_profile_id_cmd_fixed_param));
		profile_enable_cmd->profile_id = value1;
		profile_enable_cmd->enable = value2;
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID);
		if (ret) {
			WMI_LOGE("enable cmd Failed for id %d value %d",
					value1, value2);
			wmi_buf_free(buf);
			return ret;
		}
		break;

	default:
		WMI_LOGD("%s: invalid profiling command", __func__);
		break;
	}

	return 0;
}

/**
 * send_nat_keepalive_en_cmd_tlv() - enable NAT keepalive filter
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_nat_keepalive_en_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id)
{
	WMI_VDEV_IPSEC_NATKEEPALIVE_FILTER_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	WMI_LOGD("%s: vdev_id %d", __func__, vdev_id);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (WMI_VDEV_IPSEC_NATKEEPALIVE_FILTER_CMD_fixed_param *)
		wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_WMI_VDEV_IPSEC_NATKEEPALIVE_FILTER_CMD_fixed_param,
		  WMITLV_GET_STRUCT_TLVLEN
		  (WMI_VDEV_IPSEC_NATKEEPALIVE_FILTER_CMD_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->action = IPSEC_NATKEEPALIVE_FILTER_ENABLE;
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_IPSEC_NATKEEPALIVE_FILTER_CMDID)) {
		WMI_LOGP("%s: Failed to send NAT keepalive enable command",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * wmi_unified_csa_offload_enable() - sen CSA offload enable command
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_csa_offload_enable_cmd_tlv(wmi_unified_t wmi_handle,
			uint8_t vdev_id)
{
	wmi_csa_offload_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	WMI_LOGD("%s: vdev_id %d", __func__, vdev_id);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_csa_offload_enable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_csa_offload_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_csa_offload_enable_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->csa_offload_enable = WMI_CSA_OFFLOAD_ENABLE;
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_CSA_OFFLOAD_ENABLE_CMDID)) {
		WMI_LOGP("%s: Failed to send CSA offload enable command",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

#ifdef WLAN_FEATURE_CIF_CFR
/**
 * send_oem_dma_cfg_cmd_tlv() - configure OEM DMA rings
 * @wmi_handle: wmi handle
 * @data_len: len of dma cfg req
 * @data: dma cfg req
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
static QDF_STATUS send_oem_dma_cfg_cmd_tlv(wmi_unified_t wmi_handle,
				wmi_oem_dma_ring_cfg_req_fixed_param *cfg)
{
	wmi_buf_t buf;
	uint8_t *cmd;
	QDF_STATUS ret;

	WMITLV_SET_HDR(cfg,
		WMITLV_TAG_STRUC_wmi_oem_dma_ring_cfg_req_fixed_param,
		(sizeof(*cfg) - WMI_TLV_HDR_SIZE));

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cfg));
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_copy(cmd, cfg, sizeof(*cfg));
	WMI_LOGI(FL("Sending OEM Data Request to target, data len %lu"),
		sizeof(*cfg));
	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cfg),
				WMI_OEM_DMA_RING_CFG_REQ_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL(":wmi cmd send failed"));
		wmi_buf_free(buf);
	}

	return ret;
}
#endif

/**
 * send_start_oem_data_cmd_tlv() - start OEM data request to target
 * @wmi_handle: wmi handle
 * @startOemDataReq: start request params
 *
 * Return: CDF status
 */
static QDF_STATUS send_start_oem_data_cmd_tlv(wmi_unified_t wmi_handle,
			  uint32_t data_len,
			  uint8_t *data)
{
	wmi_buf_t buf;
	uint8_t *cmd;
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle,
			    (data_len + WMI_TLV_HDR_SIZE));
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (uint8_t *) wmi_buf_data(buf);

	WMITLV_SET_HDR(cmd, WMITLV_TAG_ARRAY_BYTE, data_len);
	cmd += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(cmd, data,
		     data_len);

	WMI_LOGI(FL("Sending OEM Data Request to target, data len %d"),
		 data_len);

	ret = wmi_unified_cmd_send(wmi_handle, buf,
				   (data_len +
				    WMI_TLV_HDR_SIZE), WMI_OEM_REQ_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL(":wmi cmd send failed"));
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_dfs_phyerr_filter_offload_en_cmd_tlv() - enable dfs phyerr filter
 * @wmi_handle: wmi handle
 * @dfs_phyerr_filter_offload: is dfs phyerr filter offload
 *
 * Send WMI_DFS_PHYERR_FILTER_ENA_CMDID or
 * WMI_DFS_PHYERR_FILTER_DIS_CMDID command
 * to firmware based on phyerr filtering
 * offload status.
 *
 * Return: 1 success, 0 failure
 */
static QDF_STATUS
send_dfs_phyerr_filter_offload_en_cmd_tlv(wmi_unified_t wmi_handle,
			bool dfs_phyerr_filter_offload)
{
	wmi_dfs_phyerr_filter_ena_cmd_fixed_param *enable_phyerr_offload_cmd;
	wmi_dfs_phyerr_filter_dis_cmd_fixed_param *disable_phyerr_offload_cmd;
	wmi_buf_t buf;
	uint16_t len;
	QDF_STATUS ret;


	if (false == dfs_phyerr_filter_offload) {
		WMI_LOGD("%s:Phyerror Filtering offload is Disabled in ini",
			 __func__);
		len = sizeof(*disable_phyerr_offload_cmd);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
			return 0;
		}
		disable_phyerr_offload_cmd =
			(wmi_dfs_phyerr_filter_dis_cmd_fixed_param *)
			wmi_buf_data(buf);

		WMITLV_SET_HDR(&disable_phyerr_offload_cmd->tlv_header,
		     WMITLV_TAG_STRUC_wmi_dfs_phyerr_filter_dis_cmd_fixed_param,
		     WMITLV_GET_STRUCT_TLVLEN
		     (wmi_dfs_phyerr_filter_dis_cmd_fixed_param));

		/*
		 * Send WMI_DFS_PHYERR_FILTER_DIS_CMDID
		 * to the firmware to disable the phyerror
		 * filtering offload.
		 */
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					   WMI_DFS_PHYERR_FILTER_DIS_CMDID);
		if (QDF_IS_STATUS_ERROR(ret)) {
			WMI_LOGE("%s: Failed to send WMI_DFS_PHYERR_FILTER_DIS_CMDID ret=%d",
				__func__, ret);
			wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
		}
		WMI_LOGD("%s: WMI_DFS_PHYERR_FILTER_DIS_CMDID Send Success",
			 __func__);
	} else {
		WMI_LOGD("%s:Phyerror Filtering offload is Enabled in ini",
			 __func__);

		len = sizeof(*enable_phyerr_offload_cmd);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
		}

		enable_phyerr_offload_cmd =
			(wmi_dfs_phyerr_filter_ena_cmd_fixed_param *)
			wmi_buf_data(buf);

		WMITLV_SET_HDR(&enable_phyerr_offload_cmd->tlv_header,
		     WMITLV_TAG_STRUC_wmi_dfs_phyerr_filter_ena_cmd_fixed_param,
		     WMITLV_GET_STRUCT_TLVLEN
		     (wmi_dfs_phyerr_filter_ena_cmd_fixed_param));

		/*
		 * Send a WMI_DFS_PHYERR_FILTER_ENA_CMDID
		 * to the firmware to enable the phyerror
		 * filtering offload.
		 */
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					   WMI_DFS_PHYERR_FILTER_ENA_CMDID);

		if (QDF_IS_STATUS_ERROR(ret)) {
			WMI_LOGE("%s: Failed to send DFS PHYERR CMD ret=%d",
				__func__, ret);
			wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
		}
		WMI_LOGD("%s: WMI_DFS_PHYERR_FILTER_ENA_CMDID Send Success",
			 __func__);
	}

	return QDF_STATUS_SUCCESS;
}

#if !defined(REMOVE_PKT_LOG)
/**
 * send_pktlog_wmi_send_cmd_tlv() - send pktlog enable/disable command to target
 * @wmi_handle: wmi handle
 * @pktlog_event: pktlog event
 * @cmd_id: pktlog cmd id
 *
 * Return: CDF status
 */
static QDF_STATUS send_pktlog_wmi_send_cmd_tlv(wmi_unified_t wmi_handle,
				   WMI_PKTLOG_EVENT pktlog_event,
				   WMI_CMD_ID cmd_id, uint8_t user_triggered)
{
	WMI_PKTLOG_EVENT PKTLOG_EVENT;
	WMI_CMD_ID CMD_ID;
	wmi_pdev_pktlog_enable_cmd_fixed_param *cmd;
	wmi_pdev_pktlog_disable_cmd_fixed_param *disable_cmd;
	int len = 0;
	wmi_buf_t buf;

	PKTLOG_EVENT = pktlog_event;
	CMD_ID = cmd_id;

	switch (CMD_ID) {
	case WMI_PDEV_PKTLOG_ENABLE_CMDID:
		len = sizeof(*cmd);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
			return QDF_STATUS_E_NOMEM;
		}
		cmd = (wmi_pdev_pktlog_enable_cmd_fixed_param *)
			wmi_buf_data(buf);
		WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_pktlog_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_pdev_pktlog_enable_cmd_fixed_param));
		cmd->evlist = PKTLOG_EVENT;
		cmd->enable = user_triggered ? WMI_PKTLOG_ENABLE_FORCE
					: WMI_PKTLOG_ENABLE_AUTO;
		cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							WMI_HOST_PDEV_ID_SOC);
		if (wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_PDEV_PKTLOG_ENABLE_CMDID)) {
			WMI_LOGE("failed to send pktlog enable cmdid");
			goto wmi_send_failed;
		}
		break;
	case WMI_PDEV_PKTLOG_DISABLE_CMDID:
		len = sizeof(*disable_cmd);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
			return QDF_STATUS_E_NOMEM;
		}
		disable_cmd = (wmi_pdev_pktlog_disable_cmd_fixed_param *)
			      wmi_buf_data(buf);
		WMITLV_SET_HDR(&disable_cmd->tlv_header,
		     WMITLV_TAG_STRUC_wmi_pdev_pktlog_disable_cmd_fixed_param,
		     WMITLV_GET_STRUCT_TLVLEN
		     (wmi_pdev_pktlog_disable_cmd_fixed_param));
		disable_cmd->pdev_id =
			wmi_handle->ops->convert_pdev_id_host_to_target(
							WMI_HOST_PDEV_ID_SOC);
		if (wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_PDEV_PKTLOG_DISABLE_CMDID)) {
			WMI_LOGE("failed to send pktlog disable cmdid");
			goto wmi_send_failed;
		}
		break;
	default:
		WMI_LOGD("%s: invalid PKTLOG command", __func__);
		break;
	}

	return QDF_STATUS_SUCCESS;

wmi_send_failed:
	wmi_buf_free(buf);
	return QDF_STATUS_E_FAILURE;
}
#endif /* REMOVE_PKT_LOG */

/**
 * send_wow_delete_pattern_cmd_tlv() - delete wow pattern in target
 * @wmi_handle: wmi handle
 * @ptrn_id: pattern id
 * @vdev_id: vdev id
 *
 * Return: CDF status
 */
static QDF_STATUS send_wow_delete_pattern_cmd_tlv(wmi_unified_t wmi_handle,
			uint8_t ptrn_id, uint8_t vdev_id)
{
	WMI_WOW_DEL_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	int ret;

	len = sizeof(WMI_WOW_DEL_PATTERN_CMD_fixed_param);


	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_WOW_DEL_PATTERN_CMD_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_WOW_DEL_PATTERN_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				WMI_WOW_DEL_PATTERN_CMD_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->pattern_id = ptrn_id;
	cmd->pattern_type = WOW_BITMAP_PATTERN;

	WMI_LOGI("Deleting pattern id: %d vdev id %d in fw",
		cmd->pattern_id, vdev_id);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WOW_DEL_WAKE_PATTERN_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to delete wow ptrn from fw", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_host_wakeup_ind_to_fw_cmd_tlv() - send wakeup ind to fw
 * @wmi_handle: wmi handle
 *
 * Sends host wakeup indication to FW. On receiving this indication,
 * FW will come out of WOW.
 *
 * Return: CDF status
 */
static QDF_STATUS send_host_wakeup_ind_to_fw_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_wow_hostwakeup_from_sleep_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	int32_t len;
	int ret;

	len = sizeof(wmi_wow_hostwakeup_from_sleep_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_wow_hostwakeup_from_sleep_cmd_fixed_param *)
	      wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_wow_hostwakeup_from_sleep_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
	       (wmi_wow_hostwakeup_from_sleep_cmd_fixed_param));


	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WOW_HOSTWAKEUP_FROM_SLEEP_CMDID);
	if (ret) {
		WMI_LOGE("Failed to send host wakeup indication to fw");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return qdf_status;
}

/**
 * send_del_ts_cmd_tlv() - send DELTS request to fw
 * @wmi_handle: wmi handle
 * @msg: delts params
 *
 * Return: CDF status
 */
static QDF_STATUS send_del_ts_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id,
				uint8_t ac)
{
	wmi_vdev_wmm_delts_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_wmm_delts_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_wmm_delts_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_wmm_delts_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->ac = ac;

	WMI_LOGD("Delts vdev:%d, ac:%d, %s:%d",
		 cmd->vdev_id, cmd->ac, __func__, __LINE__);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_WMM_DELTS_CMDID)) {
		WMI_LOGP("%s: Failed to send vdev DELTS command", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_aggr_qos_cmd_tlv() - send aggr qos request to fw
 * @wmi_handle: handle to wmi
 * @aggr_qos_rsp_msg - combined struct for all ADD_TS requests.
 *
 * A function to handle WMI_AGGR_QOS_REQ. This will send out
 * ADD_TS requestes to firmware in loop for all the ACs with
 * active flow.
 *
 * Return: CDF status
 */
static QDF_STATUS send_aggr_qos_cmd_tlv(wmi_unified_t wmi_handle,
		      struct aggr_add_ts_param *aggr_qos_rsp_msg)
{
	int i = 0;
	wmi_vdev_wmm_addts_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	for (i = 0; i < WMI_QOS_NUM_AC_MAX; i++) {
		/* if flow in this AC is active */
		if (((1 << i) & aggr_qos_rsp_msg->tspecIdx)) {
			/*
			 * as per implementation of wma_add_ts_req() we
			 * are not waiting any response from firmware so
			 * apart from sending ADDTS to firmware just send
			 * success to upper layers
			 */
			aggr_qos_rsp_msg->status[i] = QDF_STATUS_SUCCESS;

			buf = wmi_buf_alloc(wmi_handle, len);
			if (!buf) {
				WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
				return QDF_STATUS_E_NOMEM;
			}
			cmd = (wmi_vdev_wmm_addts_cmd_fixed_param *)
				wmi_buf_data(buf);
			WMITLV_SET_HDR(&cmd->tlv_header,
			       WMITLV_TAG_STRUC_wmi_vdev_wmm_addts_cmd_fixed_param,
			       WMITLV_GET_STRUCT_TLVLEN
				       (wmi_vdev_wmm_addts_cmd_fixed_param));
			cmd->vdev_id = aggr_qos_rsp_msg->sessionId;
			cmd->ac =
				WMI_TID_TO_AC(aggr_qos_rsp_msg->tspec[i].tsinfo.
					      traffic.userPrio);
			cmd->medium_time_us =
				aggr_qos_rsp_msg->tspec[i].mediumTime * 32;
			cmd->downgrade_type = WMM_AC_DOWNGRADE_DEPRIO;
			WMI_LOGD("%s:%d: Addts vdev:%d, ac:%d, mediumTime:%d downgrade_type:%d",
				__func__, __LINE__, cmd->vdev_id, cmd->ac,
				cmd->medium_time_us, cmd->downgrade_type);
			if (wmi_unified_cmd_send
				    (wmi_handle, buf, len,
				    WMI_VDEV_WMM_ADDTS_CMDID)) {
				WMI_LOGP("%s: Failed to send vdev ADDTS command",
					__func__);
				aggr_qos_rsp_msg->status[i] =
					QDF_STATUS_E_FAILURE;
				wmi_buf_free(buf);
				return QDF_STATUS_E_FAILURE;
			}
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_add_ts_cmd_tlv() - send ADDTS request to fw
 * @wmi_handle: wmi handle
 * @msg: ADDTS params
 *
 * Return: CDF status
 */
static QDF_STATUS send_add_ts_cmd_tlv(wmi_unified_t wmi_handle,
		 struct add_ts_param *msg)
{
	wmi_vdev_wmm_addts_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	msg->status = QDF_STATUS_SUCCESS;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_wmm_addts_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_wmm_addts_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_wmm_addts_cmd_fixed_param));
	cmd->vdev_id = msg->sme_session_id;
	cmd->ac = msg->tspec.tsinfo.traffic.userPrio;
	cmd->medium_time_us = msg->tspec.mediumTime * 32;
	cmd->downgrade_type = WMM_AC_DOWNGRADE_DROP;
	WMI_LOGD("Addts vdev:%d, ac:%d, mediumTime:%d, downgrade_type:%d %s:%d",
		 cmd->vdev_id, cmd->ac, cmd->medium_time_us,
		 cmd->downgrade_type, __func__, __LINE__);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_WMM_ADDTS_CMDID)) {
		WMI_LOGP("%s: Failed to send vdev ADDTS command", __func__);
		msg->status = QDF_STATUS_E_FAILURE;
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_process_add_periodic_tx_ptrn_cmd_tlv - add periodic tx ptrn
 * @wmi_handle: wmi handle
 * @pAddPeriodicTxPtrnParams: tx ptrn params
 *
 * Retrun: CDF status
 */
static QDF_STATUS send_process_add_periodic_tx_ptrn_cmd_tlv(wmi_unified_t wmi_handle,
						struct periodic_tx_pattern  *
						pAddPeriodicTxPtrnParams,
						uint8_t vdev_id)
{
	WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;
	uint32_t ptrn_len, ptrn_len_aligned;
	int j;

	ptrn_len = pAddPeriodicTxPtrnParams->ucPtrnSize;
	ptrn_len_aligned = roundup(ptrn_len, sizeof(uint32_t));
	len = sizeof(WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param) +
	      WMI_TLV_HDR_SIZE + ptrn_len_aligned;

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param));

	/* Pass the pattern id to delete for the corresponding vdev id */
	cmd->vdev_id = vdev_id;
	cmd->pattern_id = pAddPeriodicTxPtrnParams->ucPtrnId;
	cmd->timeout = pAddPeriodicTxPtrnParams->usPtrnIntervalMs;
	cmd->length = pAddPeriodicTxPtrnParams->ucPtrnSize;

	/* Pattern info */
	buf_ptr += sizeof(WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, ptrn_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, pAddPeriodicTxPtrnParams->ucPattern, ptrn_len);
	for (j = 0; j < pAddPeriodicTxPtrnParams->ucPtrnSize; j++)
		WMI_LOGD("%s: Add Ptrn: %02x", __func__, buf_ptr[j] & 0xff);

	WMI_LOGD("%s: Add ptrn id: %d vdev_id: %d",
		 __func__, cmd->pattern_id, cmd->vdev_id);

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_ADD_PROACTIVE_ARP_RSP_PATTERN_CMDID)) {
		WMI_LOGE("%s: failed to add pattern set state command",
			 __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_process_del_periodic_tx_ptrn_cmd_tlv - del periodic tx ptrn
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @pattern_id: pattern id
 *
 * Retrun: CDF status
 */
static QDF_STATUS send_process_del_periodic_tx_ptrn_cmd_tlv(wmi_unified_t wmi_handle,
						uint8_t vdev_id,
						uint8_t pattern_id)
{
	WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len =
		sizeof(WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param);

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param *)
		wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMD_fixed_param));

	/* Pass the pattern id to delete for the corresponding vdev id */
	cmd->vdev_id = vdev_id;
	cmd->pattern_id = pattern_id;
	WMI_LOGD("%s: Del ptrn id: %d vdev_id: %d",
		 __func__, cmd->pattern_id, cmd->vdev_id);

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_DEL_PROACTIVE_ARP_RSP_PATTERN_CMDID)) {
		WMI_LOGE("%s: failed to send del pattern command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_stats_ext_req_cmd_tlv() - request ext stats from fw
 * @wmi_handle: wmi handle
 * @preq: stats ext params
 *
 * Return: CDF status
 */
static QDF_STATUS send_stats_ext_req_cmd_tlv(wmi_unified_t wmi_handle,
			struct stats_ext_params *preq)
{
	QDF_STATUS ret;
	wmi_req_stats_ext_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len;
	uint8_t *buf_ptr;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE + preq->request_data_len;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_req_stats_ext_cmd_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_req_stats_ext_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_req_stats_ext_cmd_fixed_param));
	cmd->vdev_id = preq->vdev_id;
	cmd->data_len = preq->request_data_len;

	WMI_LOGD("%s: The data len value is %u and vdev id set is %u ",
		 __func__, preq->request_data_len, preq->vdev_id);

	buf_ptr += sizeof(wmi_req_stats_ext_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, cmd->data_len);

	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, preq->request_data, cmd->data_len);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_REQUEST_STATS_EXT_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send notify cmd ret = %d", __func__,
			 ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_enable_ext_wow_cmd_tlv() - enable ext wow in fw
 * @wmi_handle: wmi handle
 * @params: ext wow params
 *
 * Return:0 for success or error code
 */
static QDF_STATUS send_enable_ext_wow_cmd_tlv(wmi_unified_t wmi_handle,
			struct ext_wow_params *params)
{
	wmi_extwow_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	int ret;

	len = sizeof(wmi_extwow_enable_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_extwow_enable_cmd_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_extwow_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_extwow_enable_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	cmd->type = params->type;
	cmd->wakeup_pin_num = params->wakeup_pin_num;

	WMI_LOGD("%s: vdev_id %d type %d Wakeup_pin_num %x",
		 __func__, cmd->vdev_id, cmd->type, cmd->wakeup_pin_num);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_EXTWOW_ENABLE_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to set EXTWOW Enable", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;

}

/**
 * send_app_type1_params_in_fw_cmd_tlv() - set app type1 params in fw
 * @wmi_handle: wmi handle
 * @app_type1_params: app type1 params
 *
 * Return: CDF status
 */
static QDF_STATUS send_app_type1_params_in_fw_cmd_tlv(wmi_unified_t wmi_handle,
				   struct app_type1_params *app_type1_params)
{
	wmi_extwow_set_app_type1_params_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	int ret;

	len = sizeof(wmi_extwow_set_app_type1_params_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_extwow_set_app_type1_params_cmd_fixed_param *)
	      wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_extwow_set_app_type1_params_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (wmi_extwow_set_app_type1_params_cmd_fixed_param));

	cmd->vdev_id = app_type1_params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(app_type1_params->wakee_mac_addr.bytes,
				   &cmd->wakee_mac);
	qdf_mem_copy(cmd->ident, app_type1_params->identification_id, 8);
	cmd->ident_len = app_type1_params->id_length;
	qdf_mem_copy(cmd->passwd, app_type1_params->password, 16);
	cmd->passwd_len = app_type1_params->pass_length;

	WMI_LOGD("%s: vdev_id %d wakee_mac_addr %pM "
		 "identification_id %.8s id_length %u "
		 "password %.16s pass_length %u",
		 __func__, cmd->vdev_id, app_type1_params->wakee_mac_addr.bytes,
		 cmd->ident, cmd->ident_len, cmd->passwd, cmd->passwd_len);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_EXTWOW_SET_APP_TYPE1_PARAMS_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to set APP TYPE1 PARAMS", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_app_type2_params_in_fw_cmd_tlv() - set app type2 params in fw
 * @wmi_handle: wmi handle
 * @appType2Params: app type2 params
 *
 * Return: CDF status
 */
static QDF_STATUS send_set_app_type2_params_in_fw_cmd_tlv(wmi_unified_t wmi_handle,
			  struct app_type2_params *appType2Params)
{
	wmi_extwow_set_app_type2_params_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	int ret;

	len = sizeof(wmi_extwow_set_app_type2_params_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_extwow_set_app_type2_params_cmd_fixed_param *)
	      wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_extwow_set_app_type2_params_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (wmi_extwow_set_app_type2_params_cmd_fixed_param));

	cmd->vdev_id = appType2Params->vdev_id;

	qdf_mem_copy(cmd->rc4_key, appType2Params->rc4_key, 16);
	cmd->rc4_key_len = appType2Params->rc4_key_len;

	cmd->ip_id = appType2Params->ip_id;
	cmd->ip_device_ip = appType2Params->ip_device_ip;
	cmd->ip_server_ip = appType2Params->ip_server_ip;

	cmd->tcp_src_port = appType2Params->tcp_src_port;
	cmd->tcp_dst_port = appType2Params->tcp_dst_port;
	cmd->tcp_seq = appType2Params->tcp_seq;
	cmd->tcp_ack_seq = appType2Params->tcp_ack_seq;

	cmd->keepalive_init = appType2Params->keepalive_init;
	cmd->keepalive_min = appType2Params->keepalive_min;
	cmd->keepalive_max = appType2Params->keepalive_max;
	cmd->keepalive_inc = appType2Params->keepalive_inc;

	WMI_CHAR_ARRAY_TO_MAC_ADDR(appType2Params->gateway_mac.bytes,
				   &cmd->gateway_mac);
	cmd->tcp_tx_timeout_val = appType2Params->tcp_tx_timeout_val;
	cmd->tcp_rx_timeout_val = appType2Params->tcp_rx_timeout_val;

	WMI_LOGD("%s: vdev_id %d gateway_mac %pM "
		 "rc4_key %.16s rc4_key_len %u "
		 "ip_id %x ip_device_ip %x ip_server_ip %x "
		 "tcp_src_port %u tcp_dst_port %u tcp_seq %u "
		 "tcp_ack_seq %u keepalive_init %u keepalive_min %u "
		 "keepalive_max %u keepalive_inc %u "
		 "tcp_tx_timeout_val %u tcp_rx_timeout_val %u",
		 __func__, cmd->vdev_id, appType2Params->gateway_mac.bytes,
		 cmd->rc4_key, cmd->rc4_key_len,
		 cmd->ip_id, cmd->ip_device_ip, cmd->ip_server_ip,
		 cmd->tcp_src_port, cmd->tcp_dst_port, cmd->tcp_seq,
		 cmd->tcp_ack_seq, cmd->keepalive_init, cmd->keepalive_min,
		 cmd->keepalive_max, cmd->keepalive_inc,
		 cmd->tcp_tx_timeout_val, cmd->tcp_rx_timeout_val);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_EXTWOW_SET_APP_TYPE2_PARAMS_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to set APP TYPE2 PARAMS", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;

}

/**
 * send_set_auto_shutdown_timer_cmd_tlv() - sets auto shutdown timer in firmware
 * @wmi_handle: wmi handle
 * @timer_val: auto shutdown timer value
 *
 * Return: CDF status
 */
static QDF_STATUS send_set_auto_shutdown_timer_cmd_tlv(wmi_unified_t wmi_handle,
						  uint32_t timer_val)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_host_auto_shutdown_cfg_cmd_fixed_param *wmi_auto_sh_cmd;
	int len = sizeof(wmi_host_auto_shutdown_cfg_cmd_fixed_param);

	WMI_LOGD("%s: Set WMI_HOST_AUTO_SHUTDOWN_CFG_CMDID:TIMER_VAL=%d",
		 __func__, timer_val);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	wmi_auto_sh_cmd =
		(wmi_host_auto_shutdown_cfg_cmd_fixed_param *) buf_ptr;
	wmi_auto_sh_cmd->timer_value = timer_val;

	WMITLV_SET_HDR(&wmi_auto_sh_cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_host_auto_shutdown_cfg_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (wmi_host_auto_shutdown_cfg_cmd_fixed_param));

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_HOST_AUTO_SHUTDOWN_CFG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("%s: WMI_HOST_AUTO_SHUTDOWN_CFG_CMDID Err %d",
			 __func__, status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_nan_req_cmd_tlv() - to send nan request to target
 * @wmi_handle: wmi handle
 * @nan_req: request data which will be non-null
 *
 * Return: CDF status
 */
static QDF_STATUS send_nan_req_cmd_tlv(wmi_unified_t wmi_handle,
			struct nan_req_params *nan_req)
{
	QDF_STATUS ret;
	wmi_nan_cmd_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);
	uint16_t nan_data_len, nan_data_len_aligned;
	uint8_t *buf_ptr;

	/*
	 *    <----- cmd ------------><-- WMI_TLV_HDR_SIZE --><--- data ---->
	 *    +------------+----------+-----------------------+--------------+
	 *    | tlv_header | data_len | WMITLV_TAG_ARRAY_BYTE | nan_req_data |
	 *    +------------+----------+-----------------------+--------------+
	 */
	if (!nan_req) {
		WMI_LOGE("%s:nan req is not valid", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	nan_data_len = nan_req->request_data_len;
	nan_data_len_aligned = roundup(nan_req->request_data_len,
				       sizeof(uint32_t));
	len += WMI_TLV_HDR_SIZE + nan_data_len_aligned;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_nan_cmd_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_nan_cmd_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_nan_cmd_param));
	cmd->data_len = nan_req->request_data_len;
	WMI_LOGD("%s: The data len value is %u",
		 __func__, nan_req->request_data_len);
	buf_ptr += sizeof(wmi_nan_cmd_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, nan_data_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;
	qdf_mem_copy(buf_ptr, nan_req->request_data, cmd->data_len);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_NAN_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s Failed to send set param command ret = %d",
			 __func__, ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_process_dhcpserver_offload_cmd_tlv() - enable DHCP server offload
 * @wmi_handle: wmi handle
 * @pDhcpSrvOffloadInfo: DHCP server offload info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_process_dhcpserver_offload_cmd_tlv(wmi_unified_t wmi_handle,
		struct dhcp_offload_info_params *pDhcpSrvOffloadInfo)
{
	wmi_set_dhcp_server_offload_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGE("Failed to allocate buffer to send "
			 "set_dhcp_server_offload cmd");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_set_dhcp_server_offload_cmd_fixed_param *) wmi_buf_data(buf);
	qdf_mem_zero(cmd, sizeof(*cmd));

	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_set_dhcp_server_offload_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (wmi_set_dhcp_server_offload_cmd_fixed_param));
	cmd->vdev_id = pDhcpSrvOffloadInfo->vdev_id;
	cmd->enable = pDhcpSrvOffloadInfo->dhcpSrvOffloadEnabled;
	cmd->num_client = pDhcpSrvOffloadInfo->dhcpClientNum;
	cmd->srv_ipv4 = pDhcpSrvOffloadInfo->dhcpSrvIP;
	cmd->start_lsb = 0;
	status = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(*cmd),
				   WMI_SET_DHCP_SERVER_OFFLOAD_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("Failed to send set_dhcp_server_offload cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("Set dhcp server offload to vdevId %d",
		 pDhcpSrvOffloadInfo->vdev_id);

	return status;
}

/**
 * send_set_led_flashing_cmd_tlv() - set led flashing in fw
 * @wmi_handle: wmi handle
 * @flashing: flashing request
 *
 * Return: CDF status
 */
static QDF_STATUS send_set_led_flashing_cmd_tlv(wmi_unified_t wmi_handle,
				struct flashing_req_params *flashing)
{
	wmi_set_led_flashing_cmd_fixed_param *cmd;
	QDF_STATUS status;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t len = sizeof(wmi_set_led_flashing_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_set_led_flashing_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_set_led_flashing_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_set_led_flashing_cmd_fixed_param));
	cmd->pattern_id = flashing->pattern_id;
	cmd->led_x0 = flashing->led_x0;
	cmd->led_x1 = flashing->led_x1;

	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_PDEV_SET_LED_FLASHING_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("%s: wmi_unified_cmd_send WMI_PEER_SET_PARAM_CMD"
			 " returned Error %d", __func__, status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_process_ch_avoid_update_cmd_tlv() - handles channel avoid update request
 * @wmi_handle: wmi handle
 * @ch_avoid_update_req: channel avoid update params
 *
 * Return: CDF status
 */
static QDF_STATUS send_process_ch_avoid_update_cmd_tlv(wmi_unified_t wmi_handle)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_chan_avoid_update_cmd_param *ch_avoid_update_fp;
	int len = sizeof(wmi_chan_avoid_update_cmd_param);


	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	ch_avoid_update_fp = (wmi_chan_avoid_update_cmd_param *) buf_ptr;
	WMITLV_SET_HDR(&ch_avoid_update_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_chan_avoid_update_cmd_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_chan_avoid_update_cmd_param));

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_CHAN_AVOID_UPDATE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("wmi_unified_cmd_send"
			 " WMITLV_TABLE_WMI_CHAN_AVOID_UPDATE"
			 " returned Error %d", status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_pdev_set_regdomain_cmd_tlv() - send set regdomain command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev regdomain params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_set_regdomain_cmd_tlv(wmi_unified_t wmi_handle,
				struct pdev_set_regdomain_params *param)
{
	wmi_buf_t buf;
	wmi_pdev_set_regdomain_cmd_fixed_param *cmd;
	int32_t len = sizeof(*cmd);


	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_pdev_set_regdomain_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_regdomain_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_set_regdomain_cmd_fixed_param));

	cmd->reg_domain = param->currentRDinuse;
	cmd->reg_domain_2G = param->currentRD2G;
	cmd->reg_domain_5G = param->currentRD5G;
	cmd->conformance_test_limit_2G = param->ctl_2G;
	cmd->conformance_test_limit_5G = param->ctl_5G;
	cmd->dfs_domain = param->dfsDomain;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							param->pdev_id);

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_REGDOMAIN_CMDID)) {
		WMI_LOGE("%s: Failed to send pdev set regdomain command",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_regdomain_info_to_fw_cmd_tlv() - send regdomain info to fw
 * @wmi_handle: wmi handle
 * @reg_dmn: reg domain
 * @regdmn2G: 2G reg domain
 * @regdmn5G: 5G reg domain
 * @ctl2G: 2G test limit
 * @ctl5G: 5G test limit
 *
 * Return: none
 */
static QDF_STATUS send_regdomain_info_to_fw_cmd_tlv(wmi_unified_t wmi_handle,
				   uint32_t reg_dmn, uint16_t regdmn2G,
				   uint16_t regdmn5G, int8_t ctl2G,
				   int8_t ctl5G)
{
	wmi_buf_t buf;
	wmi_pdev_set_regdomain_cmd_fixed_param *cmd;
	int32_t len = sizeof(*cmd);


	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_pdev_set_regdomain_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_regdomain_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_set_regdomain_cmd_fixed_param));
	cmd->reg_domain = reg_dmn;
	cmd->reg_domain_2G = regdmn2G;
	cmd->reg_domain_5G = regdmn5G;
	cmd->conformance_test_limit_2G = ctl2G;
	cmd->conformance_test_limit_5G = ctl5G;

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_REGDOMAIN_CMDID)) {
		WMI_LOGP("%s: Failed to send pdev set regdomain command",
			 __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}


/**
 * send_set_tdls_offchan_mode_cmd_tlv() - set tdls off channel mode
 * @wmi_handle: wmi handle
 * @chan_switch_params: Pointer to tdls channel switch parameter structure
 *
 * This function sets tdls off channel mode
 *
 * Return: 0 on success; Negative errno otherwise
 */
static QDF_STATUS send_set_tdls_offchan_mode_cmd_tlv(wmi_unified_t wmi_handle,
	      struct tdls_channel_switch_params *chan_switch_params)
{
	wmi_tdls_set_offchan_mode_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	u_int16_t len = sizeof(wmi_tdls_set_offchan_mode_cmd_fixed_param);

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_tdls_set_offchan_mode_cmd_fixed_param *)
		wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_tdls_set_offchan_mode_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_tdls_set_offchan_mode_cmd_fixed_param));

	WMI_CHAR_ARRAY_TO_MAC_ADDR(chan_switch_params->peer_mac_addr,
				&cmd->peer_macaddr);
	cmd->vdev_id = chan_switch_params->vdev_id;
	cmd->offchan_mode = chan_switch_params->tdls_sw_mode;
	cmd->is_peer_responder = chan_switch_params->is_responder;
	cmd->offchan_num = chan_switch_params->tdls_off_ch;
	cmd->offchan_bw_bitmap = chan_switch_params->tdls_off_ch_bw_offset;
	cmd->offchan_oper_class = chan_switch_params->oper_class;

	WMI_LOGD(FL("Peer MAC Addr mac_addr31to0: 0x%x, mac_addr47to32: 0x%x"),
		 cmd->peer_macaddr.mac_addr31to0,
		 cmd->peer_macaddr.mac_addr47to32);

	WMI_LOGD(FL(
		 "vdev_id: %d, off channel mode: %d, off channel Num: %d, "
		 "off channel offset: 0x%x, is_peer_responder: %d, operating class: %d"
		  ),
		 cmd->vdev_id,
		 cmd->offchan_mode,
		 cmd->offchan_num,
		 cmd->offchan_bw_bitmap,
		 cmd->is_peer_responder,
		 cmd->offchan_oper_class);

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
		WMI_TDLS_SET_OFFCHAN_MODE_CMDID)) {
		WMI_LOGP(FL("failed to send tdls off chan command"));
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}


	return QDF_STATUS_SUCCESS;
}

/**
 * send_update_fw_tdls_state_cmd_tlv() - send enable/disable tdls for a vdev
 * @wmi_handle: wmi handle
 * @pwmaTdlsparams: TDLS params
 *
 * Return: 0 for sucess or error code
 */
static QDF_STATUS send_update_fw_tdls_state_cmd_tlv(wmi_unified_t wmi_handle,
					 void *tdls_param, uint8_t tdls_state)
{
	wmi_tdls_set_state_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;

	struct wmi_tdls_params *wmi_tdls = (struct wmi_tdls_params *) tdls_param;
	uint16_t len = sizeof(wmi_tdls_set_state_cmd_fixed_param);

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmai_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_tdls_set_state_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		  WMITLV_TAG_STRUC_wmi_tdls_set_state_cmd_fixed_param,
		  WMITLV_GET_STRUCT_TLVLEN
		  (wmi_tdls_set_state_cmd_fixed_param));
	cmd->vdev_id = wmi_tdls->vdev_id;
	cmd->state = tdls_state;
	cmd->notification_interval_ms = wmi_tdls->notification_interval_ms;
	cmd->tx_discovery_threshold = wmi_tdls->tx_discovery_threshold;
	cmd->tx_teardown_threshold = wmi_tdls->tx_teardown_threshold;
	cmd->rssi_teardown_threshold = wmi_tdls->rssi_teardown_threshold;
	cmd->rssi_delta = wmi_tdls->rssi_delta;
	cmd->tdls_options = wmi_tdls->tdls_options;
	cmd->tdls_peer_traffic_ind_window = wmi_tdls->peer_traffic_ind_window;
	cmd->tdls_peer_traffic_response_timeout_ms =
		wmi_tdls->peer_traffic_response_timeout;
	cmd->tdls_puapsd_mask = wmi_tdls->puapsd_mask;
	cmd->tdls_puapsd_inactivity_time_ms = wmi_tdls->puapsd_inactivity_time;
	cmd->tdls_puapsd_rx_frame_threshold =
		wmi_tdls->puapsd_rx_frame_threshold;
	cmd->teardown_notification_ms =
		wmi_tdls->teardown_notification_ms;
	cmd->tdls_peer_kickout_threshold =
		wmi_tdls->tdls_peer_kickout_threshold;

	WMI_LOGD("%s: tdls_state: %d, state: %d, "
		 "notification_interval_ms: %d, "
		 "tx_discovery_threshold: %d, "
		 "tx_teardown_threshold: %d, "
		 "rssi_teardown_threshold: %d, "
		 "rssi_delta: %d, "
		 "tdls_options: 0x%x, "
		 "tdls_peer_traffic_ind_window: %d, "
		 "tdls_peer_traffic_response_timeout: %d, "
		 "tdls_puapsd_mask: 0x%x, "
		 "tdls_puapsd_inactivity_time: %d, "
		 "tdls_puapsd_rx_frame_threshold: %d, "
		 "teardown_notification_ms: %d, "
		 "tdls_peer_kickout_threshold: %d",
		 __func__, tdls_state, cmd->state,
		 cmd->notification_interval_ms,
		 cmd->tx_discovery_threshold,
		 cmd->tx_teardown_threshold,
		 cmd->rssi_teardown_threshold,
		 cmd->rssi_delta,
		 cmd->tdls_options,
		 cmd->tdls_peer_traffic_ind_window,
		 cmd->tdls_peer_traffic_response_timeout_ms,
		 cmd->tdls_puapsd_mask,
		 cmd->tdls_puapsd_inactivity_time_ms,
		 cmd->tdls_puapsd_rx_frame_threshold,
		 cmd->teardown_notification_ms,
		 cmd->tdls_peer_kickout_threshold);

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_TDLS_SET_STATE_CMDID)) {
		WMI_LOGP("%s: failed to send tdls set state command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	WMI_LOGD("%s: vdev_id %d", __func__, wmi_tdls->vdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * send_update_tdls_peer_state_cmd_tlv() - update TDLS peer state
 * @wmi_handle: wmi handle
 * @peerStateParams: TDLS peer state params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_update_tdls_peer_state_cmd_tlv(wmi_unified_t wmi_handle,
			       struct tdls_peer_state_params *peerStateParams,
				   uint32_t *ch_mhz)
{
	wmi_tdls_peer_update_cmd_fixed_param *cmd;
	wmi_tdls_peer_capabilities *peer_cap;
	wmi_channel *chan_info;
	wmi_buf_t wmi_buf;
	uint8_t *buf_ptr;
	uint32_t i;
	int32_t len = sizeof(wmi_tdls_peer_update_cmd_fixed_param) +
		      sizeof(wmi_tdls_peer_capabilities);


	len += WMI_TLV_HDR_SIZE +
	       sizeof(wmi_channel) * peerStateParams->peerCap.peerChanLen;

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_tdls_peer_update_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_tdls_peer_update_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_tdls_peer_update_cmd_fixed_param));

	cmd->vdev_id = peerStateParams->vdevId;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peerStateParams->peerMacAddr,
				   &cmd->peer_macaddr);


	cmd->peer_state = peerStateParams->peerState;

	WMI_LOGD("%s: vdev_id: %d, peerStateParams->peerMacAddr: %pM, "
		 "peer_macaddr.mac_addr31to0: 0x%x, "
		 "peer_macaddr.mac_addr47to32: 0x%x, peer_state: %d",
		 __func__, cmd->vdev_id, peerStateParams->peerMacAddr,
		 cmd->peer_macaddr.mac_addr31to0,
		 cmd->peer_macaddr.mac_addr47to32, cmd->peer_state);

	buf_ptr += sizeof(wmi_tdls_peer_update_cmd_fixed_param);
	peer_cap = (wmi_tdls_peer_capabilities *) buf_ptr;
	WMITLV_SET_HDR(&peer_cap->tlv_header,
		       WMITLV_TAG_STRUC_wmi_tdls_peer_capabilities,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_tdls_peer_capabilities));

	if ((peerStateParams->peerCap.peerUapsdQueue & 0x08) >> 3)
		WMI_SET_TDLS_PEER_VO_UAPSD(peer_cap);
	if ((peerStateParams->peerCap.peerUapsdQueue & 0x04) >> 2)
		WMI_SET_TDLS_PEER_VI_UAPSD(peer_cap);
	if ((peerStateParams->peerCap.peerUapsdQueue & 0x02) >> 1)
		WMI_SET_TDLS_PEER_BK_UAPSD(peer_cap);
	if (peerStateParams->peerCap.peerUapsdQueue & 0x01)
		WMI_SET_TDLS_PEER_BE_UAPSD(peer_cap);

	/* Ack and More Data Ack are sent as 0, so no need to set
	 * but fill SP
	 */
	WMI_SET_TDLS_PEER_SP_UAPSD(peer_cap,
				   peerStateParams->peerCap.peerMaxSp);

	peer_cap->buff_sta_support =
		peerStateParams->peerCap.peerBuffStaSupport;
	peer_cap->off_chan_support =
		peerStateParams->peerCap.peerOffChanSupport;
	peer_cap->peer_curr_operclass =
		peerStateParams->peerCap.peerCurrOperClass;
	/* self curr operclass is not being used and so pass op class for
	 * preferred off chan in it.
	 */
	peer_cap->self_curr_operclass =
		peerStateParams->peerCap.opClassForPrefOffChan;
	peer_cap->peer_chan_len = peerStateParams->peerCap.peerChanLen;
	peer_cap->peer_operclass_len =
		peerStateParams->peerCap.peerOperClassLen;

	WMI_LOGD("%s: peer_operclass_len: %d",
		 __func__, peer_cap->peer_operclass_len);
	for (i = 0; i < WMI_TDLS_MAX_SUPP_OPER_CLASSES; i++) {
		peer_cap->peer_operclass[i] =
			peerStateParams->peerCap.peerOperClass[i];
		WMI_LOGD("%s: peer_operclass[%d]: %d",
			 __func__, i, peer_cap->peer_operclass[i]);
	}

	peer_cap->is_peer_responder = peerStateParams->peerCap.isPeerResponder;
	peer_cap->pref_offchan_num = peerStateParams->peerCap.prefOffChanNum;
	peer_cap->pref_offchan_bw =
		peerStateParams->peerCap.prefOffChanBandwidth;

	WMI_LOGD
		("%s: peer_qos: 0x%x, buff_sta_support: %d, off_chan_support: %d, "
		 "peer_curr_operclass: %d, self_curr_operclass: %d, peer_chan_len: "
		 "%d, peer_operclass_len: %d, is_peer_responder: %d, pref_offchan_num:"
		 " %d, pref_offchan_bw: %d",
		__func__, peer_cap->peer_qos, peer_cap->buff_sta_support,
		peer_cap->off_chan_support, peer_cap->peer_curr_operclass,
		peer_cap->self_curr_operclass, peer_cap->peer_chan_len,
		peer_cap->peer_operclass_len, peer_cap->is_peer_responder,
		peer_cap->pref_offchan_num, peer_cap->pref_offchan_bw);

	/* next fill variable size array of peer chan info */
	buf_ptr += sizeof(wmi_tdls_peer_capabilities);
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_channel) *
		       peerStateParams->peerCap.peerChanLen);
	chan_info = (wmi_channel *) (buf_ptr + WMI_TLV_HDR_SIZE);

	for (i = 0; i < peerStateParams->peerCap.peerChanLen; ++i) {
		WMITLV_SET_HDR(&chan_info->tlv_header,
			       WMITLV_TAG_STRUC_wmi_channel,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
		chan_info->mhz = ch_mhz[i];
		chan_info->band_center_freq1 = chan_info->mhz;
		chan_info->band_center_freq2 = 0;

		WMI_LOGD("%s: chan[%d] = %u", __func__, i, chan_info->mhz);

		if (peerStateParams->peerCap.peerChan[i].dfsSet) {
			WMI_SET_CHANNEL_FLAG(chan_info, WMI_CHAN_FLAG_PASSIVE);
			WMI_LOGI("chan[%d] DFS[%d]\n",
				 peerStateParams->peerCap.peerChan[i].chanId,
				 peerStateParams->peerCap.peerChan[i].dfsSet);
		}

		if (chan_info->mhz < WMI_2_4_GHZ_MAX_FREQ)
			WMI_SET_CHANNEL_MODE(chan_info, MODE_11G);
		else
			WMI_SET_CHANNEL_MODE(chan_info, MODE_11A);

		WMI_SET_CHANNEL_MAX_TX_POWER(chan_info,
					     peerStateParams->peerCap.
					     peerChan[i].pwr);

		WMI_SET_CHANNEL_REG_POWER(chan_info,
					  peerStateParams->peerCap.peerChan[i].
					  pwr);
		WMI_LOGD("Channel TX power[%d] = %u: %d", i, chan_info->mhz,
			 peerStateParams->peerCap.peerChan[i].pwr);

		chan_info++;
	}

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_TDLS_PEER_UPDATE_CMDID)) {
		WMI_LOGE("%s: failed to send tdls peer update state command",
			 __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}


	return QDF_STATUS_SUCCESS;
}

/*
 * send_process_fw_mem_dump_cmd_tlv() - Function to request fw memory dump from
 * firmware
 * @wmi_handle:         Pointer to wmi handle
 * @mem_dump_req:       Pointer for mem_dump_req
 *
 * This function sends memory dump request to firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
static QDF_STATUS send_process_fw_mem_dump_cmd_tlv(wmi_unified_t wmi_handle,
					struct fw_dump_req_param *mem_dump_req)
{
	wmi_get_fw_mem_dump_fixed_param *cmd;
	wmi_fw_mem_dump *dump_params;
	struct wmi_fw_dump_seg_req *seg_req;
	int32_t len;
	wmi_buf_t buf;
	u_int8_t *buf_ptr;
	int ret, loop;

	/*
	 * len = sizeof(fixed param) that includes tlv header +
	 *       tlv header for array of struc +
	 *       sizeof (each struct)
	 */
	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += mem_dump_req->num_seg * sizeof(wmi_fw_mem_dump);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE(FL("Failed allocate wmi buffer"));
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (u_int8_t *) wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_get_fw_mem_dump_fixed_param *) buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_get_fw_mem_dump_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_get_fw_mem_dump_fixed_param));

	cmd->request_id = mem_dump_req->request_id;
	cmd->num_fw_mem_dump_segs = mem_dump_req->num_seg;

	/* TLV indicating array of structures to follow */
	buf_ptr += sizeof(wmi_get_fw_mem_dump_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_fw_mem_dump) *
		       cmd->num_fw_mem_dump_segs);

	buf_ptr += WMI_TLV_HDR_SIZE;
	dump_params = (wmi_fw_mem_dump *) buf_ptr;

	WMI_LOGI(FL("request_id:%d num_seg:%d"),
		    mem_dump_req->request_id, mem_dump_req->num_seg);
	for (loop = 0; loop < cmd->num_fw_mem_dump_segs; loop++) {
		seg_req = (struct wmi_fw_dump_seg_req *)
			  ((uint8_t *)(mem_dump_req->segment) +
			    loop * sizeof(*seg_req));
		WMITLV_SET_HDR(&dump_params->tlv_header,
			    WMITLV_TAG_STRUC_wmi_fw_mem_dump_params,
			    WMITLV_GET_STRUCT_TLVLEN(wmi_fw_mem_dump));
		dump_params->seg_id = seg_req->seg_id;
		dump_params->seg_start_addr_lo = seg_req->seg_start_addr_lo;
		dump_params->seg_start_addr_hi = seg_req->seg_start_addr_hi;
		dump_params->seg_length = seg_req->seg_length;
		dump_params->dest_addr_lo = seg_req->dst_addr_lo;
		dump_params->dest_addr_hi = seg_req->dst_addr_hi;
		WMI_LOGI(FL("seg_number:%d"), loop);
		WMI_LOGI(FL("seg_id:%d start_addr_lo:0x%x start_addr_hi:0x%x"),
			 dump_params->seg_id, dump_params->seg_start_addr_lo,
			 dump_params->seg_start_addr_hi);
		WMI_LOGI(FL("seg_length:%d dst_addr_lo:0x%x dst_addr_hi:0x%x"),
			 dump_params->seg_length, dump_params->dest_addr_lo,
			 dump_params->dest_addr_hi);
		dump_params++;
	}

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_GET_FW_MEM_DUMP_CMDID);
	if (ret) {
		WMI_LOGE(FL("Failed to send get firmware mem dump request"));
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	WMI_LOGI(FL("Get firmware mem dump request sent successfully"));
	return QDF_STATUS_SUCCESS;
}

/*
 * send_process_set_ie_info_cmd_tlv() - Function to send IE info to firmware
 * @wmi_handle:    Pointer to WMi handle
 * @ie_data:       Pointer for ie data
 *
 * This function sends IE information to firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
static QDF_STATUS send_process_set_ie_info_cmd_tlv(wmi_unified_t wmi_handle,
				   struct vdev_ie_info_param *ie_info)
{
	wmi_vdev_set_ie_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len, ie_len_aligned;
	QDF_STATUS ret;


	ie_len_aligned = roundup(ie_info->length, sizeof(uint32_t));
	/* Allocate memory for the WMI command */
	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE + ie_len_aligned;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE(FL("wmi_buf_alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);

	/* Populate the WMI command */
	cmd = (wmi_vdev_set_ie_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_ie_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			wmi_vdev_set_ie_cmd_fixed_param));
	cmd->vdev_id = ie_info->vdev_id;
	cmd->ie_id = ie_info->ie_id;
	cmd->ie_len = ie_info->length;
	cmd->band = ie_info->band;

	WMI_LOGD(FL("IE:%d of size:%d sent for vdev:%d"), ie_info->ie_id,
		 ie_info->length, ie_info->vdev_id);

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, ie_len_aligned);
	buf_ptr += WMI_TLV_HDR_SIZE;

	qdf_mem_copy(buf_ptr, ie_info->data, cmd->ie_len);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SET_IE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE(FL("Failed to send set IE command ret = %d"), ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_smart_ant_enable_cmd_tlv() - WMI smart ant enable function
 *
 *  @param wmi_handle  : handle to WMI.
 *  @param param       : pointer to antenna param
 *
 *  This function sends smart antenna enable command to FW
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_enable_cmd_tlv(wmi_unified_t wmi_handle,
				struct smart_ant_enable_params *param)
{
	/* Send WMI COMMAND to Enable */
	wmi_pdev_smart_ant_enable_cmd_fixed_param *cmd;
	wmi_pdev_smart_ant_gpio_handle *gpio_param;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int len = 0;
	QDF_STATUS ret;
	int loop = 0;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += WMI_HAL_MAX_SANTENNA * sizeof(wmi_pdev_smart_ant_gpio_handle);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
			WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
			return QDF_STATUS_E_NOMEM;
		}

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_pdev_smart_ant_enable_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_smart_ant_enable_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_smart_ant_enable_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	cmd->enable = param->enable;
	cmd->mode = param->mode;
	cmd->rx_antenna = param->rx_antenna;
	cmd->tx_default_antenna = param->rx_antenna;

	/* TLV indicating array of structures to follow */
	buf_ptr += sizeof(wmi_pdev_smart_ant_enable_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       WMI_HAL_MAX_SANTENNA *
		       sizeof(wmi_pdev_smart_ant_gpio_handle));

	buf_ptr += WMI_TLV_HDR_SIZE;
	gpio_param = (wmi_pdev_smart_ant_gpio_handle *)buf_ptr;

	for (loop = 0; loop < WMI_HAL_MAX_SANTENNA; loop++) {
		WMITLV_SET_HDR(&gpio_param->tlv_header,
			       WMITLV_TAG_STRUC_wmi_pdev_smart_ant_gpio_handle,
			       WMITLV_GET_STRUCT_TLVLEN(
			       wmi_pdev_smart_ant_gpio_handle));
		if (param->mode == SMART_ANT_MODE_SERIAL) {
			if (loop < WMI_HOST_MAX_SERIAL_ANTENNA) {
				gpio_param->gpio_pin = param->gpio_pin[loop];
				gpio_param->gpio_func = param->gpio_func[loop];
			} else {
				gpio_param->gpio_pin = 0;
				gpio_param->gpio_func = 0;
			}
		} else if (param->mode == SMART_ANT_MODE_PARALLEL) {
			gpio_param->gpio_pin = param->gpio_pin[loop];
			gpio_param->gpio_func = param->gpio_func[loop];
		}
		/* Setting it to 0 for now */
		gpio_param->pdev_id =
			wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
		gpio_param++;
	}

	ret = wmi_unified_cmd_send(wmi_handle,
				buf,
				len,
				WMI_PDEV_SMART_ANT_ENABLE_CMDID);

	if (ret != 0) {
		WMI_LOGE(" %s :WMI Failed\n", __func__);
		WMI_LOGE("enable:%d mode:%d  rx_antenna: 0x%08x PINS: [%d %d %d %d] Func[%d %d %d %d] cmdstatus=%d\n",
			 cmd->enable,
			 cmd->mode,
			 cmd->rx_antenna,
			 param->gpio_pin[0], param->gpio_pin[1],
			 param->gpio_pin[2], param->gpio_pin[3],
			 param->gpio_func[0], param->gpio_func[1],
			 param->gpio_func[2], param->gpio_func[3],
			 ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_smart_ant_set_rx_ant_cmd_tlv() - WMI set rx antenna function
 *
 *  @param wmi_handle     : handle to WMI.
 *  @param param          : pointer to rx antenna param
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_set_rx_ant_cmd_tlv(wmi_unified_t wmi_handle,
				struct smart_ant_rx_ant_params *param)
{
	wmi_pdev_smart_ant_set_rx_antenna_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	WMI_LOGD("%s:\n", __func__);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	cmd = (wmi_pdev_smart_ant_set_rx_antenna_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	    WMITLV_TAG_STRUC_wmi_pdev_smart_ant_set_rx_antenna_cmd_fixed_param,
	    WMITLV_GET_STRUCT_TLVLEN(
		wmi_pdev_smart_ant_set_rx_antenna_cmd_fixed_param));
	cmd->rx_antenna = param->antenna;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);

	ret = wmi_unified_cmd_send(wmi_handle,
				buf,
				len,
				WMI_PDEV_SMART_ANT_SET_RX_ANTENNA_CMDID);

	if (ret != 0) {
		WMI_LOGE(" %s :WMI Failed\n", __func__);
		WMI_LOGE("%s: rx_antenna: 0x%08x cmdstatus=%d\n",
			 __func__,
			 cmd->rx_antenna,
			 ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_ctl_table_cmd_tlv() - send ctl table cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold ctl table param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS
send_set_ctl_table_cmd_tlv(wmi_unified_t wmi_handle,
			   struct ctl_table_params *param)
{
	uint16_t len, ctl_tlv_len;
	uint8_t *buf_ptr;
	wmi_buf_t buf;
	wmi_pdev_set_ctl_table_cmd_fixed_param *cmd;
	uint32_t *ctl_array;

	if (!param->ctl_array)
		return QDF_STATUS_E_FAILURE;

	if (param->ctl_cmd_len !=
		WMI_HOST_NUM_CTLS_2G * WMI_HOST_NUM_BAND_EDGES_2G * 2 +
		WMI_HOST_NUM_CTLS_5G * WMI_HOST_NUM_BAND_EDGES_5G * 2) {
		qdf_print("CTL array len not correct\n");
		return QDF_STATUS_E_FAILURE;
	}

	ctl_tlv_len = WMI_TLV_HDR_SIZE +
		roundup(param->ctl_cmd_len, sizeof(A_UINT32));
	len = sizeof(*cmd) + ctl_tlv_len;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);

	cmd = (wmi_pdev_set_ctl_table_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_ctl_table_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_set_ctl_table_cmd_fixed_param));
	cmd->ctl_len = param->ctl_cmd_len;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (cmd->ctl_len));
	buf_ptr += WMI_TLV_HDR_SIZE;
	ctl_array = (uint32_t *)buf_ptr;

	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(&ctl_array[0], &param->ctl_band,
					sizeof(param->ctl_band));
	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(&ctl_array[1], param->ctl_array,
					param->ctl_cmd_len -
					sizeof(param->ctl_band));

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_CTL_TABLE_CMDID)) {
		WMI_LOGE("%s:Failed to send command\n", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_mimogain_table_cmd_tlv() - send mimogain table cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold mimogain table param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS
send_set_mimogain_table_cmd_tlv(wmi_unified_t wmi_handle,
				struct mimogain_table_params *param)
{
	uint16_t len, table_tlv_len;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	wmi_pdev_set_mimogain_table_cmd_fixed_param *cmd;
	uint32_t *gain_table;

	if (!param->array_gain)
		return QDF_STATUS_E_FAILURE;

	/* len must be multiple of a single array gain table */
	if (param->tbl_len %
	    ((WMI_HOST_TX_NUM_CHAIN-1) * WMI_HOST_TPC_REGINDEX_MAX *
	     WMI_HOST_ARRAY_GAIN_NUM_STREAMS) != 0) {
		WMI_LOGE("Array gain table len not correct\n");
		return QDF_STATUS_E_FAILURE;
	}

	table_tlv_len = WMI_TLV_HDR_SIZE +
		roundup(param->tbl_len, sizeof(uint32_t));
	len = sizeof(*cmd) + table_tlv_len;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);

	cmd = (wmi_pdev_set_mimogain_table_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_set_mimogain_table_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		       wmi_pdev_set_mimogain_table_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	WMI_MIMOGAIN_ARRAY_GAIN_LEN_SET(cmd->mimogain_info, param->tbl_len);
	WMI_MIMOGAIN_MULTI_CHAIN_BYPASS_SET(cmd->mimogain_info,
					    param->multichain_gain_bypass);

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (param->tbl_len));
	buf_ptr += WMI_TLV_HDR_SIZE;
	gain_table = (uint32_t *)buf_ptr;

	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(gain_table,
					param->array_gain,
					param->tbl_len);

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_MIMOGAIN_TABLE_CMDID)) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_packet_power_info_get_cmd_tlv() - send request to get packet power
 * info to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold packet power info param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS
send_packet_power_info_get_cmd_tlv(wmi_unified_t wmi_handle,
				   struct packet_power_info_params *param)
{
	wmi_pdev_get_tpc_cmd_fixed_param *cmd;
	wmi_buf_t wmibuf;
	uint8_t *buf_ptr;
	u_int32_t len = sizeof(wmi_pdev_get_tpc_cmd_fixed_param);

	wmibuf = wmi_buf_alloc(wmi_handle, len);
	if (wmibuf == NULL)
		return QDF_STATUS_E_NOMEM;

	buf_ptr = (uint8_t *)wmi_buf_data(wmibuf);

	cmd = (wmi_pdev_get_tpc_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_get_tpc_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_get_tpc_cmd_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	cmd->rate_flags = param->rate_flags;
	cmd->nss = param->nss;
	cmd->preamble = param->preamble;
	cmd->hw_rate = param->hw_rate;
	WMI_LOGI("%s[%d] commandID %d, wmi_pdev_get_tpc_cmd=0x%x\n",
		 __func__,
		 __LINE__,
		 WMI_PDEV_GET_TPC_CMDID,
		 *((u_int32_t *)cmd));
	if (wmi_unified_cmd_send(wmi_handle, wmibuf, len,
				 WMI_PDEV_GET_TPC_CMDID)) {
			WMI_LOGE(FL("Failed to get tpc command\n"));
			wmi_buf_free(wmibuf);
			return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_vdev_config_ratemask_cmd_tlv() - config ratemask param in fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold config ratemask params
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_vdev_config_ratemask_cmd_tlv(wmi_unified_t wmi_handle,
					struct config_ratemask_params *param)
{
	wmi_vdev_config_ratemask_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_vdev_config_ratemask_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_config_ratemask_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_vdev_config_ratemask_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->type = param->type;
	cmd->mask_lower32 = param->lower32;
	cmd->mask_higher32 = param->higher32;
	WMI_LOGI("Setting vdev ratemask vdev id = 0x%X, type = 0x%X, mask_l32 = 0x%X mask_h32 = 0x%X\n",
		 param->vdev_id, param->type, param->lower32, param->higher32);

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_RATEMASK_CMDID)) {
			WMI_LOGE("Seting vdev ratemask failed\n");
			wmi_buf_free(buf);
			return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_vap_dscp_tid_map_cmd_tlv() - send vap dscp tid map cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vap dscp tid map param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS
send_set_vap_dscp_tid_map_cmd_tlv(wmi_unified_t wmi_handle,
				  struct vap_dscp_tid_map_params *param)
{
	wmi_buf_t buf;
	wmi_vdev_set_dscp_tid_map_cmd_fixed_param *cmd;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_vdev_set_dscp_tid_map_cmd_fixed_param *)wmi_buf_data(buf);
	qdf_mem_copy(cmd->dscp_to_tid_map, param->dscp_to_tid_map,
		     sizeof(A_UINT32) * WMI_DSCP_MAP_MAX);

	cmd->vdev_id = param->vdev_id;
	cmd->enable_override = 0;

	WMI_LOGI("Setting dscp for vap id: %d\n", cmd->vdev_id);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_SET_DSCP_TID_MAP_CMDID)) {
			WMI_LOGE("Failed to set dscp cmd\n");
			wmi_buf_free(buf);
			return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_vdev_set_neighbour_rx_cmd_tlv() - set neighbour rx param in fw
 * @wmi_handle: wmi handle
 * @macaddr: vdev mac address
 * @param: pointer to hold neigbour rx param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_vdev_set_neighbour_rx_cmd_tlv(wmi_unified_t wmi_handle,
					uint8_t macaddr[IEEE80211_ADDR_LEN],
					struct set_neighbour_rx_params *param)
{
	wmi_vdev_filter_nrp_config_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_vdev_filter_nrp_config_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_filter_nrp_config_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_vdev_filter_nrp_config_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	cmd->bssid_idx = param->idx;
	cmd->action = param->action;
	cmd->type = param->type;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->addr);
	cmd->flag = 0;

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_FILTER_NEIGHBOR_RX_PACKETS_CMDID)) {
			WMI_LOGE("Failed to set neighbour rx param\n");
			wmi_buf_free(buf);
			return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 *  send_smart_ant_set_tx_ant_cmd_tlv() - WMI set tx antenna function
 *  @param wmi_handle  : handle to WMI.
 *  @param macaddr     : vdev mac address
 *  @param param       : pointer to tx antenna param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_set_tx_ant_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct smart_ant_tx_ant_params *param)
{
	wmi_peer_smart_ant_set_tx_antenna_cmd_fixed_param *cmd;
	wmi_peer_smart_ant_set_tx_antenna_series *ant_tx_series;
	wmi_buf_t buf;
	int32_t len = 0;
	int i;
	uint8_t *buf_ptr;
	QDF_STATUS ret;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += (WMI_SMART_ANT_MAX_RATE_SERIES) *
		sizeof(wmi_peer_smart_ant_set_tx_antenna_series);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_peer_smart_ant_set_tx_antenna_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
	    WMITLV_TAG_STRUC_wmi_peer_smart_ant_set_tx_antenna_cmd_fixed_param,
	    WMITLV_GET_STRUCT_TLVLEN(
			wmi_peer_smart_ant_set_tx_antenna_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);

	buf_ptr += sizeof(wmi_peer_smart_ant_set_tx_antenna_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_peer_smart_ant_set_tx_antenna_series));
	buf_ptr += WMI_TLV_HDR_SIZE;
	ant_tx_series = (wmi_peer_smart_ant_set_tx_antenna_series *)buf_ptr;

	for (i = 0; i < WMI_SMART_ANT_MAX_RATE_SERIES; i++) {
		WMITLV_SET_HDR(&ant_tx_series->tlv_header,
		    WMITLV_TAG_STRUC_wmi_peer_smart_ant_set_tx_antenna_series,
		    WMITLV_GET_STRUCT_TLVLEN(
				wmi_peer_smart_ant_set_tx_antenna_series));
		ant_tx_series->antenna_series = param->antenna_array[i];
		ant_tx_series++;
	}

	ret = wmi_unified_cmd_send(wmi_handle,
				   buf,
				   len,
				   WMI_PEER_SMART_ANT_SET_TX_ANTENNA_CMDID);

	if (ret != 0) {
		WMI_LOGE(" %s :WMI Failed\n", __func__);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_ant_switch_tbl_cmd_tlv() - send ant switch tbl cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold ant switch tbl param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS
send_set_ant_switch_tbl_cmd_tlv(wmi_unified_t wmi_handle,
				struct ant_switch_tbl_params *param)
{
	uint8_t len;
	wmi_buf_t buf;
	wmi_pdev_set_ant_switch_tbl_cmd_fixed_param *cmd;
	wmi_pdev_set_ant_ctrl_chain *ctrl_chain;
	uint8_t *buf_ptr;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += sizeof(wmi_pdev_set_ant_ctrl_chain);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_pdev_set_ant_switch_tbl_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_set_ant_switch_tbl_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_pdev_set_ant_switch_tbl_cmd_fixed_param));

	cmd->antCtrlCommon1 = param->ant_ctrl_common1;
	cmd->antCtrlCommon2 = param->ant_ctrl_common2;
	cmd->mac_id =
		wmi_handle->ops->convert_pdev_id_host_to_target(param->pdev_id);

	/* TLV indicating array of structures to follow */
	buf_ptr += sizeof(wmi_pdev_set_ant_switch_tbl_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_pdev_set_ant_ctrl_chain));
	buf_ptr += WMI_TLV_HDR_SIZE;
	ctrl_chain = (wmi_pdev_set_ant_ctrl_chain *)buf_ptr;

	ctrl_chain->pdev_id =
		wmi_handle->ops->convert_pdev_id_host_to_target(param->pdev_id);
	ctrl_chain->antCtrlChain = param->antCtrlChain;

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_ANTENNA_SWITCH_TABLE_CMDID)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 *  send_smart_ant_set_training_info_cmd_tlv() - WMI set smart antenna
 *  training information function
 *  @param wmi_handle  : handle to WMI.
 *  @macaddr           : vdev mac address
 *  @param param       : pointer to tx antenna param
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_set_training_info_cmd_tlv(
				wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct smart_ant_training_info_params *param)
{
	wmi_peer_smart_ant_set_train_antenna_cmd_fixed_param *cmd;
	wmi_peer_smart_ant_set_train_antenna_param *train_param;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t len = 0;
	QDF_STATUS ret;
	int loop;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += (WMI_SMART_ANT_MAX_RATE_SERIES) *
		 sizeof(wmi_peer_smart_ant_set_train_antenna_param);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_peer_smart_ant_set_train_antenna_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_peer_smart_ant_set_train_antenna_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_peer_smart_ant_set_train_antenna_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->num_pkts = param->numpkts;

	buf_ptr += sizeof(wmi_peer_smart_ant_set_train_antenna_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_peer_smart_ant_set_train_antenna_param) *
		       WMI_SMART_ANT_MAX_RATE_SERIES);

	buf_ptr += WMI_TLV_HDR_SIZE;
	train_param = (wmi_peer_smart_ant_set_train_antenna_param *)buf_ptr;

	for (loop = 0; loop < WMI_SMART_ANT_MAX_RATE_SERIES; loop++) {
		WMITLV_SET_HDR(&train_param->tlv_header,
		WMITLV_TAG_STRUC_wmi_peer_smart_ant_set_train_antenna_param,
			    WMITLV_GET_STRUCT_TLVLEN(
				wmi_peer_smart_ant_set_train_antenna_param));
		train_param->train_rate_series = param->rate_array[loop];
		train_param->train_antenna_series = param->antenna_array[loop];
		train_param->rc_flags = 0;
		WMI_LOGI(FL("Series number:%d\n"), loop);
		WMI_LOGI(FL("Rate [0x%02x] Tx_Antenna [0x%08x]\n"),
			 train_param->train_rate_series,
			 train_param->train_antenna_series);
		train_param++;
	}

	ret = wmi_unified_cmd_send(wmi_handle,
				buf,
				len,
				WMI_PEER_SMART_ANT_SET_TRAIN_INFO_CMDID);

	if (ret != 0) {
		WMI_LOGE(" %s :WMI Failed\n", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return ret;
}

/**
 *  send_smart_ant_set_node_config_cmd_tlv() - WMI set node
 *  configuration function
 *  @param wmi_handle		   : handle to WMI.
 *  @macaddr			   : vdev mad address
 *  @param param		   : pointer to tx antenna param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_set_node_config_cmd_tlv(
				wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct smart_ant_node_config_params *param)
{
	wmi_peer_smart_ant_set_node_config_ops_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t len = 0, args_tlv_len;
	int ret;
	int i = 0;
	A_UINT32 *node_config_args;

	args_tlv_len = WMI_TLV_HDR_SIZE + param->args_count * sizeof(A_UINT32);
	len = sizeof(*cmd) + args_tlv_len;

	if ((param->args_count == 0)) {
		WMI_LOGE("%s: Can't send a command with %d arguments\n",
			  __func__, param->args_count);
		return QDF_STATUS_E_FAILURE;
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_peer_smart_ant_set_node_config_ops_cmd_fixed_param *)
						wmi_buf_data(buf);
	buf_ptr = (uint8_t *)cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_peer_smart_ant_set_node_config_ops_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		wmi_peer_smart_ant_set_node_config_ops_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->cmd_id = param->cmd_id;
	cmd->args_count = param->args_count;
	buf_ptr += sizeof(
		wmi_peer_smart_ant_set_node_config_ops_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
			(cmd->args_count * sizeof(A_UINT32)));
	buf_ptr += WMI_TLV_HDR_SIZE;
	node_config_args = (A_UINT32 *)buf_ptr;

	for (i = 0; i < param->args_count; i++) {
		node_config_args[i] = param->args_arr[i];
		WMI_LOGI("%d", param->args_arr[i]);
	}

	ret = wmi_unified_cmd_send(wmi_handle,
			   buf,
			   len,
			   WMI_PEER_SMART_ANT_SET_NODE_CONFIG_OPS_CMDID);

	if (ret != 0) {
		WMI_LOGE("%s: WMI FAILED:Sent cmd_id: 0x%x\n Node: %02x:%02x:%02x:%02x:%02x:%02x cmdstatus=%d\n",
			 __func__, param->cmd_id, macaddr[0],
			 macaddr[1], macaddr[2], macaddr[3],
			 macaddr[4], macaddr[5], ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_atf_cmd_tlv() - send set atf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set atf param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS
send_set_atf_cmd_tlv(wmi_unified_t wmi_handle,
		     struct set_atf_params *param)
{
	wmi_atf_peer_info *peer_info;
	wmi_peer_atf_request_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int i;
	int32_t len = 0;
	QDF_STATUS retval;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += param->num_peers * sizeof(wmi_atf_peer_info);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_peer_atf_request_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_atf_request_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_peer_atf_request_fixed_param));
	cmd->num_peers = param->num_peers;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_atf_peer_info) *
		       cmd->num_peers);
	buf_ptr += WMI_TLV_HDR_SIZE;
	peer_info = (wmi_atf_peer_info *)buf_ptr;

	for (i = 0; i < cmd->num_peers; i++) {
		WMITLV_SET_HDR(&peer_info->tlv_header,
			    WMITLV_TAG_STRUC_wmi_atf_peer_info,
			    WMITLV_GET_STRUCT_TLVLEN(
				wmi_atf_peer_info));
		qdf_mem_copy(&(peer_info->peer_macaddr),
				&(param->peer_info[i].peer_macaddr),
				sizeof(wmi_mac_addr));
		peer_info->atf_units = param->peer_info[i].percentage_peer;
		peer_info->vdev_id = param->peer_info[i].vdev_id;
		peer_info->pdev_id =
			wmi_handle->ops->convert_pdev_id_host_to_target(
				param->peer_info[i].pdev_id);
		/*
		 * TLV definition for peer atf request fixed param combines
		 * extension stats. Legacy FW for WIN (Non-TLV) has peer atf
		 * stats and atf extension stats as two different
		 * implementations.
		 * Need to discuss with FW on this.
		 *
		 * peer_info->atf_groupid = param->peer_ext_info[i].group_index;
		 * peer_info->atf_units_reserved =
		 *		param->peer_ext_info[i].atf_index_reserved;
		 */
		peer_info++;
	}

	retval = wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PEER_ATF_REQUEST_CMDID);

	if (retval != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s : WMI Failed\n", __func__);
		wmi_buf_free(buf);
	}

	return retval;
}

/**
 * send_vdev_set_fwtest_param_cmd_tlv() - send fwtest param in fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold fwtest param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_vdev_set_fwtest_param_cmd_tlv(wmi_unified_t wmi_handle,
				struct set_fwtest_params *param)
{
	wmi_fwtest_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_fwtest_set_param_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_fwtest_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_fwtest_set_param_cmd_fixed_param));
	cmd->param_id = param->arg;
	cmd->param_value = param->value;

	if (wmi_unified_cmd_send(wmi_handle, buf, len, WMI_FWTEST_CMDID)) {
		WMI_LOGE("Setting FW test param failed\n");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_qboost_param_cmd_tlv() - send set qboost command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to qboost params
 * @macaddr: vdev mac address
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS
send_set_qboost_param_cmd_tlv(wmi_unified_t wmi_handle,
			      uint8_t macaddr[IEEE80211_ADDR_LEN],
			      struct set_qboost_params *param)
{
	WMI_QBOOST_CFG_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (WMI_QBOOST_CFG_CMD_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_QBOOST_CFG_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				WMI_QBOOST_CFG_CMD_fixed_param));
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->qb_enable = param->value;

	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_QBOOST_CFG_CMDID);

	if (ret != 0) {
		WMI_LOGE("Setting qboost cmd failed\n");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_gpio_config_cmd_tlv() - send gpio config to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold gpio config param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_gpio_config_cmd_tlv(wmi_unified_t wmi_handle,
			 struct gpio_config_params *param)
{
	wmi_gpio_config_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);

	/* Sanity Checks */
	if (param->pull_type > WMI_GPIO_PULL_DOWN ||
	    param->intr_mode > WMI_GPIO_INTTYPE_LEVEL_HIGH) {
		return QDF_STATUS_E_FAILURE;
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_gpio_config_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_gpio_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_gpio_config_cmd_fixed_param));
	cmd->gpio_num = param->gpio_num;
	cmd->input = param->input;
	cmd->pull_type = param->pull_type;
	cmd->intr_mode = param->intr_mode;

	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_GPIO_CONFIG_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending GPIO config cmd failed\n");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_gpio_output_cmd_tlv() - send gpio output to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold gpio output param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_gpio_output_cmd_tlv(wmi_unified_t wmi_handle,
			 struct gpio_output_params *param)
{
	wmi_gpio_output_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_gpio_output_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_gpio_output_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_gpio_output_cmd_fixed_param));
	cmd->gpio_num = param->gpio_num;
	cmd->set = param->set;

	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_GPIO_OUTPUT_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending GPIO output cmd failed\n");
		wmi_buf_free(buf);
	}

	return ret;

}

/**
 *  send_phyerr_disable_cmd_tlv() - WMI phyerr disable function
 *
 *  @param wmi_handle     : handle to WMI.
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_phyerr_disable_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_pdev_dfs_disable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_pdev_dfs_disable_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_dfs_disable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_dfs_disable_cmd_fixed_param));
	/* Filling it with WMI_PDEV_ID_SOC for now */
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							WMI_HOST_PDEV_ID_SOC);

	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_PDEV_DFS_DISABLE_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending PDEV DFS disable cmd failed\n");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_phyerr_enable_cmd_tlv() - WMI phyerr disable function
 *
 *  @param wmi_handle     : handle to WMI.
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_phyerr_enable_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_pdev_dfs_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_pdev_dfs_enable_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_dfs_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_dfs_enable_cmd_fixed_param));
	/* Reserved for future use */
	cmd->reserved0 = 0;

	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_PDEV_DFS_ENABLE_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending PDEV DFS enable cmd failed\n");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_nf_dbr_dbm_info_get_cmd_tlv() - send request to get nf to fw
 * @wmi_handle: wmi handle
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_nf_dbr_dbm_info_get_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_buf_t buf;
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle, 0);
	if (buf == NULL)
		return QDF_STATUS_E_NOMEM;

	ret = wmi_unified_cmd_send(wmi_handle, buf, 0,
				   WMI_PDEV_GET_NFCAL_POWER_CMDID);
	if (ret != 0) {
		WMI_LOGE("Sending get nfcal power cmd failed\n");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_ht_ie_cmd_tlv() - send ht ie command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to ht ie param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_ht_ie_cmd_tlv(wmi_unified_t wmi_handle,
		       struct ht_ie_params *param)
{
	wmi_pdev_set_ht_ie_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;
	uint8_t *buf_ptr;

	len = sizeof(*cmd)  + WMI_TLV_HDR_SIZE +
	      roundup(param->ie_len, sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_pdev_set_ht_ie_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_ht_ie_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_set_ht_ie_cmd_fixed_param));
	cmd->reserved0 = 0;
	cmd->ie_len = param->ie_len;
	cmd->tx_streams = param->tx_streams;
	cmd->rx_streams = param->rx_streams;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, cmd->ie_len);
	buf_ptr += WMI_TLV_HDR_SIZE;
	if (param->ie_len)
		WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(buf_ptr, param->ie_data,
						cmd->ie_len);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PDEV_SET_HT_CAP_IE_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending set ht ie cmd failed\n");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_vht_ie_cmd_tlv() - send vht ie command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to vht ie param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_vht_ie_cmd_tlv(wmi_unified_t wmi_handle,
			struct vht_ie_params *param)
{
	wmi_pdev_set_vht_ie_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;
	uint8_t *buf_ptr;

	len = sizeof(*cmd)  + WMI_TLV_HDR_SIZE +
	      roundup(param->ie_len, sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_pdev_set_vht_ie_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_vht_ie_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_set_vht_ie_cmd_fixed_param));
	cmd->reserved0 = 0;
	cmd->ie_len = param->ie_len;
	cmd->tx_streams = param->tx_streams;
	cmd->rx_streams = param->rx_streams;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, cmd->ie_len);
	buf_ptr += WMI_TLV_HDR_SIZE;
	if (param->ie_len)
		WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(buf_ptr, param->ie_data,
						cmd->ie_len);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PDEV_SET_VHT_CAP_IE_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending set vht ie cmd failed\n");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_quiet_mode_cmd_tlv() - send set quiet mode command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to quiet mode params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_quiet_mode_cmd_tlv(wmi_unified_t wmi_handle,
			    struct set_quiet_mode_params *param)
{
	wmi_pdev_set_quiet_cmd_fixed_param *quiet_cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*quiet_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	quiet_cmd = (wmi_pdev_set_quiet_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&quiet_cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_quiet_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_set_quiet_cmd_fixed_param));
	quiet_cmd = (wmi_pdev_set_quiet_cmd_fixed_param *)wmi_buf_data(buf);
	quiet_cmd->enabled = param->enabled;
	quiet_cmd->period = (param->period)*(param->intval);
	quiet_cmd->duration = param->duration;
	quiet_cmd->next_start = param->offset;
	quiet_cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							WMI_HOST_PDEV_ID_SOC);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PDEV_SET_QUIET_MODE_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending set quiet cmd failed\n");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_bwf_cmd_tlv() - send set bwf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set bwf param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_bwf_cmd_tlv(wmi_unified_t wmi_handle,
		     struct set_bwf_params *param)
{
	wmi_bwf_peer_info *peer_info;
	wmi_peer_bwf_request_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS retval;
	int32_t len;
	uint8_t *buf_ptr;
	int i;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += param->num_peers * sizeof(wmi_bwf_peer_info);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_peer_bwf_request_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_bwf_request_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_peer_bwf_request_fixed_param));
	cmd->num_peers = param->num_peers;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_bwf_peer_info) *
		       cmd->num_peers);
	buf_ptr += WMI_TLV_HDR_SIZE;
	peer_info = (wmi_bwf_peer_info *)buf_ptr;

	for (i = 0; i < cmd->num_peers; i++) {
		WMITLV_SET_HDR(&peer_info->tlv_header,
			       WMITLV_TAG_STRUC_wmi_bwf_peer_info,
			       WMITLV_GET_STRUCT_TLVLEN(wmi_bwf_peer_info));
		peer_info->bwf_guaranteed_bandwidth =
				param->peer_info[i].throughput;
		peer_info->bwf_max_airtime =
				param->peer_info[i].max_airtime;
		peer_info->bwf_peer_priority =
				param->peer_info[i].priority;
		qdf_mem_copy(&peer_info->peer_macaddr,
			     &param->peer_info[i].peer_macaddr,
			     sizeof(param->peer_info[i].peer_macaddr));
		peer_info->vdev_id =
				param->peer_info[i].vdev_id;
		peer_info->pdev_id =
			wmi_handle->ops->convert_pdev_id_host_to_target(
				param->peer_info[i].pdev_id);
		peer_info++;
	}

	retval = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_PEER_BWF_REQUEST_CMDID);

	if (retval != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s : WMI Failed\n", __func__);
		wmi_buf_free(buf);
	}

	return retval;
}

/**
 * send_mcast_group_update_cmd_tlv() - send mcast group update cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold mcast update param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_mcast_group_update_cmd_tlv(wmi_unified_t wmi_handle,
				struct mcast_group_update_params *param)
{
	wmi_peer_mcast_group_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;
	int offset = 0;
	static char dummymask[4] = { 0xFF, 0xFF, 0xFF, 0xFF};

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_peer_mcast_group_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_mcast_group_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_peer_mcast_group_cmd_fixed_param));
	/* confirm the buffer is 4-byte aligned */
	QDF_ASSERT((((size_t) cmd) & 0x3) == 0);
	qdf_mem_zero(cmd, sizeof(*cmd));

	cmd->vdev_id = param->vap_id;
	/* construct the message assuming our endianness matches the target */
	cmd->flags |= WMI_PEER_MCAST_GROUP_FLAG_ACTION_M &
		(param->action << WMI_PEER_MCAST_GROUP_FLAG_ACTION_S);
	cmd->flags |= WMI_PEER_MCAST_GROUP_FLAG_WILDCARD_M &
		(param->wildcard << WMI_PEER_MCAST_GROUP_FLAG_WILDCARD_S);
	if (param->is_action_delete)
		cmd->flags |= WMI_PEER_MCAST_GROUP_FLAG_DELETEALL_M;

	if (param->is_mcast_addr_len)
		cmd->flags |=  WMI_PEER_MCAST_GROUP_FLAG_IPV6_M;

	if (param->is_filter_mode_snoop)
		cmd->flags |= WMI_PEER_MCAST_GROUP_FLAG_SRC_FILTER_EXCLUDE_M;

	/* unicast address spec only applies for non-wildcard cases */
	if (!param->wildcard && param->ucast_mac_addr) {
		WMI_CHAR_ARRAY_TO_MAC_ADDR(param->ucast_mac_addr,
					   &cmd->ucast_mac_addr);
	}
	if (param->mcast_ip_addr) {
		QDF_ASSERT(param->mcast_ip_addr_bytes <=
			   sizeof(cmd->mcast_ip_addr));
		offset = sizeof(cmd->mcast_ip_addr) -
			 param->mcast_ip_addr_bytes;
		qdf_mem_copy(((uint8_t *)&cmd->mcast_ip_addr) + offset,
			     param->mcast_ip_addr,
			     param->mcast_ip_addr_bytes);
	}
	if (!param->mask)
		param->mask = &dummymask[0];

	qdf_mem_copy(((uint8_t *)&cmd->mcast_ip_mask) + offset,
		     param->mask,
		     param->mcast_ip_addr_bytes);

	if (param->srcs && param->nsrcs) {
		cmd->num_filter_addr = param->nsrcs;
		QDF_ASSERT((param->nsrcs * param->mcast_ip_addr_bytes) <=
			sizeof(cmd->filter_addr));

		qdf_mem_copy(((uint8_t *) &cmd->filter_addr), param->srcs,
			     param->nsrcs * param->mcast_ip_addr_bytes);
	}

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_PEER_MCAST_GROUP_CMDID);

	if (ret != QDF_STATUS_SUCCESS) {
		WMI_LOGE("%s : WMI Failed\n", __func__);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_vdev_spectral_configure_cmd_tlv() - send VDEV spectral configure
 * command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold spectral config parameter
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_spectral_configure_cmd_tlv(wmi_unified_t wmi_handle,
				struct vdev_spectral_configure_params *param)
{
	wmi_vdev_spectral_configure_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_vdev_spectral_configure_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_spectral_configure_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		WMITLV_TAG_STRUC_wmi_vdev_spectral_configure_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	cmd->spectral_scan_count = param->count;
	cmd->spectral_scan_period = param->period;
	cmd->spectral_scan_priority = param->spectral_pri;
	cmd->spectral_scan_fft_size = param->fft_size;
	cmd->spectral_scan_gc_ena = param->gc_enable;
	cmd->spectral_scan_restart_ena = param->restart_enable;
	cmd->spectral_scan_noise_floor_ref = param->noise_floor_ref;
	cmd->spectral_scan_init_delay = param->init_delay;
	cmd->spectral_scan_nb_tone_thr = param->nb_tone_thr;
	cmd->spectral_scan_str_bin_thr = param->str_bin_thr;
	cmd->spectral_scan_wb_rpt_mode = param->wb_rpt_mode;
	cmd->spectral_scan_rssi_rpt_mode = param->rssi_rpt_mode;
	cmd->spectral_scan_rssi_thr = param->rssi_thr;
	cmd->spectral_scan_pwr_format = param->pwr_format;
	cmd->spectral_scan_rpt_mode = param->rpt_mode;
	cmd->spectral_scan_bin_scale = param->bin_scale;
	cmd->spectral_scan_dBm_adj = param->dBm_adj;
	cmd->spectral_scan_chn_mask = param->chn_mask;

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SPECTRAL_SCAN_CONFIGURE_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending set quiet cmd failed\n");
		wmi_buf_free(buf);
	}

	WMI_LOGI("%s: Sent WMI_VDEV_SPECTRAL_SCAN_CONFIGURE_CMDID\n",
		 __func__);

	WMI_LOGI("vdev_id = %u\n"
		 "spectral_scan_count = %u\n"
		 "spectral_scan_period = %u\n"
		 "spectral_scan_priority = %u\n"
		 "spectral_scan_fft_size = %u\n"
		 "spectral_scan_gc_ena = %u\n"
		 "spectral_scan_restart_ena = %u\n"
		 "spectral_scan_noise_floor_ref = %u\n"
		 "spectral_scan_init_delay = %u\n"
		 "spectral_scan_nb_tone_thr = %u\n"
		 "spectral_scan_str_bin_thr = %u\n"
		 "spectral_scan_wb_rpt_mode = %u\n"
		 "spectral_scan_rssi_rpt_mode = %u\n"
		 "spectral_scan_rssi_thr = %u\n"
		 "spectral_scan_pwr_format = %u\n"
		 "spectral_scan_rpt_mode = %u\n"
		 "spectral_scan_bin_scale = %u\n"
		 "spectral_scan_dBm_adj = %u\n"
		 "spectral_scan_chn_mask = %u\n",
		 param->vdev_id,
		 param->count,
		 param->period,
		 param->spectral_pri,
		 param->fft_size,
		 param->gc_enable,
		 param->restart_enable,
		 param->noise_floor_ref,
		 param->init_delay,
		 param->nb_tone_thr,
		 param->str_bin_thr,
		 param->wb_rpt_mode,
		 param->rssi_rpt_mode,
		 param->rssi_thr,
		 param->pwr_format,
		 param->rpt_mode,
		 param->bin_scale,
		 param->dBm_adj,
		 param->chn_mask);
	WMI_LOGI("%s: Status: %d\n\n", __func__, ret);

	return ret;
}

/**
 * send_vdev_spectral_enable_cmd_tlv() - send VDEV spectral configure
 * command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold spectral enable parameter
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_spectral_enable_cmd_tlv(wmi_unified_t wmi_handle,
				struct vdev_spectral_enable_params *param)
{
	wmi_vdev_spectral_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_vdev_spectral_enable_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_vdev_spectral_enable_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		WMITLV_TAG_STRUC_wmi_vdev_spectral_enable_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;

	if (param->active_valid) {
		cmd->trigger_cmd = param->active ? 1 : 2;
		/* 1: Trigger, 2: Clear Trigger */
	} else {
		cmd->trigger_cmd = 0; /* 0: Ignore */
	}

	if (param->enabled_valid) {
		cmd->enable_cmd = param->enabled ? 1 : 2;
		/* 1: Enable 2: Disable */
	} else {
		cmd->enable_cmd = 0; /* 0: Ignore */
	}

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SPECTRAL_SCAN_ENABLE_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending scan enable CMD failed\n");
		wmi_buf_free(buf);
	}

	WMI_LOGI("%s: Sent WMI_VDEV_SPECTRAL_SCAN_ENABLE_CMDID\n", __func__);

	WMI_LOGI("vdev_id = %u\n"
				 "trigger_cmd = %u\n"
				 "enable_cmd = %u\n",
				 cmd->vdev_id,
				 cmd->trigger_cmd,
				 cmd->enable_cmd);

	WMI_LOGI("%s: Status: %d\n\n", __func__, ret);

	return ret;
}

/**
 * send_thermal_mitigation_param_cmd_tlv() - configure thermal mitigation params
 * @param wmi_handle : handle to WMI.
 * @param param : pointer to hold thermal mitigation param
 *
 * @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_thermal_mitigation_param_cmd_tlv(
		wmi_unified_t wmi_handle,
		struct thermal_mitigation_params *param)
{
	wmi_therm_throt_config_request_fixed_param *tt_conf = NULL;
	wmi_therm_throt_level_config_info *lvl_conf = NULL;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr = NULL;
	int error;
	int32_t len;
	int i;

	len = sizeof(*tt_conf) + WMI_TLV_HDR_SIZE +
			THERMAL_LEVELS * sizeof(wmi_therm_throt_level_config_info);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	tt_conf = (wmi_therm_throt_config_request_fixed_param *) wmi_buf_data(buf);

	/* init fixed params */
	WMITLV_SET_HDR(tt_conf,
		WMITLV_TAG_STRUC_wmi_therm_throt_config_request_fixed_param,
		(WMITLV_GET_STRUCT_TLVLEN(wmi_therm_throt_config_request_fixed_param)));

	tt_conf->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	tt_conf->enable = param->enable;
	tt_conf->dc = param->dc;
	tt_conf->dc_per_event = param->dc_per_event;
	tt_conf->therm_throt_levels = THERMAL_LEVELS;

	buf_ptr = (uint8_t *) ++tt_conf;
	/* init TLV params */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			(THERMAL_LEVELS * sizeof(wmi_therm_throt_level_config_info)));

	lvl_conf = (wmi_therm_throt_level_config_info *) (buf_ptr +  WMI_TLV_HDR_SIZE);
	for (i = 0; i < THERMAL_LEVELS; i++) {
		WMITLV_SET_HDR(&lvl_conf->tlv_header,
			WMITLV_TAG_STRUC_wmi_therm_throt_level_config_info,
			WMITLV_GET_STRUCT_TLVLEN(wmi_therm_throt_level_config_info));
		lvl_conf->temp_lwm = param->levelconf[i].tmplwm;
		lvl_conf->temp_hwm = param->levelconf[i].tmphwm;
		lvl_conf->dc_off_percent = param->levelconf[i].dcoffpercent;
		lvl_conf->prio = param->levelconf[i].priority;
		lvl_conf++;
	}

	error = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_THERM_THROT_SET_CONF_CMDID);
	if (QDF_IS_STATUS_ERROR(error)) {
		wmi_buf_free(buf);
		WMI_LOGE("Failed to send WMI_THERM_THROT_SET_CONF_CMDID command");
	}

	return error;
}

/**
 * send_pdev_qvit_cmd_tlv() - send qvit command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev_qvit_params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_qvit_cmd_tlv(wmi_unified_t wmi_handle,
		       struct pdev_qvit_params *param)
{
	wmi_buf_t buf;
	QDF_STATUS ret = QDF_STATUS_E_INVAL;
	uint8_t *cmd;
	static uint8_t msgref = 1;
	uint8_t segnumber = 0, seginfo, numsegments;
	uint16_t chunk_len, total_bytes;
	uint8_t *bufpos;
	QVIT_SEG_HDR_INFO_STRUCT seghdrinfo;

	bufpos = param->utf_payload;
	total_bytes = param->len;
	ASSERT(total_bytes / MAX_WMI_QVIT_LEN ==
	       (uint8_t) (total_bytes / MAX_WMI_QVIT_LEN));
	numsegments = (uint8_t) (total_bytes / MAX_WMI_QVIT_LEN);

	if (param->len - (numsegments * MAX_WMI_QVIT_LEN))
		numsegments++;

	while (param->len) {
		if (param->len > MAX_WMI_QVIT_LEN)
			chunk_len = MAX_WMI_QVIT_LEN;    /* MAX messsage */
		else
			chunk_len = param->len;

		buf = wmi_buf_alloc(wmi_handle,
				    (chunk_len + sizeof(seghdrinfo) +
				     WMI_TLV_HDR_SIZE));
		if (!buf) {
			WMI_LOGE("%s:wmi_buf_alloc failed", __func__);
			return QDF_STATUS_E_NOMEM;
		}

		cmd = (uint8_t *) wmi_buf_data(buf);

		seghdrinfo.len = total_bytes;
		seghdrinfo.msgref = msgref;
		seginfo = ((numsegments << 4) & 0xF0) | (segnumber & 0xF);
		seghdrinfo.segmentInfo = seginfo;

		segnumber++;

		WMITLV_SET_HDR(cmd, WMITLV_TAG_ARRAY_BYTE,
			       (chunk_len + sizeof(seghdrinfo)));
		cmd += WMI_TLV_HDR_SIZE;
		qdf_mem_copy(cmd, &seghdrinfo, sizeof(seghdrinfo));
		qdf_mem_copy(&cmd[sizeof(seghdrinfo)], bufpos, chunk_len);

		ret = wmi_unified_cmd_send(wmi_handle, buf,
					   (chunk_len + sizeof(seghdrinfo) +
					    WMI_TLV_HDR_SIZE),
					   WMI_PDEV_QVIT_CMDID);

		if (ret != 0) {
			WMI_LOGE("Failed to send WMI_PDEV_QVIT_CMDID command");
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
 * send_wmm_update_cmd_tlv() - send wmm update command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to wmm update param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_wmm_update_cmd_tlv(wmi_unified_t wmi_handle,
			struct wmm_update_params *param)
{
	wmi_pdev_set_wmm_params_cmd_fixed_param *cmd;
	wmi_wmm_params *wmm_param;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;
	int ac = 0;
	struct wmi_host_wmeParams *wmep;
	uint8_t *buf_ptr;

	len = sizeof(*cmd) + (WME_NUM_AC * sizeof(*wmm_param));
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_pdev_set_wmm_params_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_pdev_set_wmm_params_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_set_wmm_params_cmd_fixed_param));

	cmd->reserved0 = WMI_HOST_PDEV_ID_SOC;

	buf_ptr += sizeof(wmi_pdev_set_wmm_params_cmd_fixed_param);

	for (ac = 0; ac < WME_NUM_AC; ac++) {
		wmep = &param->wmep_array[ac];
		wmm_param = (wmi_wmm_params *)buf_ptr;
		WMITLV_SET_HDR(&wmm_param->tlv_header,
			WMITLV_TAG_STRUC_wmi_wmm_params,
			WMITLV_GET_STRUCT_TLVLEN(wmi_wmm_params));
		wmm_param->aifs = wmep->wmep_aifsn;
		wmm_param->cwmin = ATH_EXPONENT_TO_VALUE(wmep->wmep_logcwmin);
		wmm_param->cwmax = ATH_EXPONENT_TO_VALUE(wmep->wmep_logcwmax);
		wmm_param->txoplimit = ATH_TXOP_TO_US(wmep->wmep_txopLimit);
		wmm_param->acm = wmep->wmep_acm;
		wmm_param->no_ack = wmep->wmep_noackPolicy;
		buf_ptr += sizeof(wmi_wmm_params);
	}
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PDEV_SET_WMM_PARAMS_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending WMM update CMD failed\n");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_coex_config_cmd_tlv() - send coex config command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to coex config param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_coex_config_cmd_tlv(wmi_unified_t wmi_handle,
			 struct coex_config_params *param)
{
	WMI_COEX_CONFIG_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS ret;
	int32_t len;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (WMI_COEX_CONFIG_CMD_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_COEX_CONFIG_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
		       WMI_COEX_CONFIG_CMD_fixed_param));

	cmd->vdev_id = param->vdev_id;
	cmd->config_type = param->config_type;
	cmd->config_arg1 = param->config_arg1;
	cmd->config_arg2 = param->config_arg2;
	cmd->config_arg3 = param->config_arg3;
	cmd->config_arg4 = param->config_arg4;
	cmd->config_arg5 = param->config_arg5;
	cmd->config_arg6 = param->config_arg6;

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_COEX_CONFIG_CMDID);

	if (ret != 0) {
		WMI_LOGE("Sending COEX CONFIG CMD failed\n");
		wmi_buf_free(buf);
	}

	return ret;
}

static
void wmi_copy_resource_config(wmi_resource_config *resource_cfg,
				target_resource_config *tgt_res_cfg)
{
	resource_cfg->num_vdevs = tgt_res_cfg->num_vdevs;
	resource_cfg->num_peers = tgt_res_cfg->num_peers;
	resource_cfg->num_offload_peers = tgt_res_cfg->num_offload_peers;
	resource_cfg->num_offload_reorder_buffs =
			tgt_res_cfg->num_offload_reorder_buffs;
	resource_cfg->num_peer_keys = tgt_res_cfg->num_peer_keys;
	resource_cfg->num_tids = tgt_res_cfg->num_tids;
	resource_cfg->ast_skid_limit = tgt_res_cfg->ast_skid_limit;
	resource_cfg->tx_chain_mask = tgt_res_cfg->tx_chain_mask;
	resource_cfg->rx_chain_mask = tgt_res_cfg->rx_chain_mask;
	resource_cfg->rx_timeout_pri[0] = tgt_res_cfg->rx_timeout_pri[0];
	resource_cfg->rx_timeout_pri[1] = tgt_res_cfg->rx_timeout_pri[1];
	resource_cfg->rx_timeout_pri[2] = tgt_res_cfg->rx_timeout_pri[2];
	resource_cfg->rx_timeout_pri[3] = tgt_res_cfg->rx_timeout_pri[3];
	resource_cfg->rx_decap_mode = tgt_res_cfg->rx_decap_mode;
	resource_cfg->scan_max_pending_req =
			tgt_res_cfg->scan_max_pending_req;
	resource_cfg->bmiss_offload_max_vdev =
			tgt_res_cfg->bmiss_offload_max_vdev;
	resource_cfg->roam_offload_max_vdev =
			tgt_res_cfg->roam_offload_max_vdev;
	resource_cfg->roam_offload_max_ap_profiles =
			tgt_res_cfg->roam_offload_max_ap_profiles;
	resource_cfg->num_mcast_groups = tgt_res_cfg->num_mcast_groups;
	resource_cfg->num_mcast_table_elems =
			tgt_res_cfg->num_mcast_table_elems;
	resource_cfg->mcast2ucast_mode = tgt_res_cfg->mcast2ucast_mode;
	resource_cfg->tx_dbg_log_size = tgt_res_cfg->tx_dbg_log_size;
	resource_cfg->num_wds_entries = tgt_res_cfg->num_wds_entries;
	resource_cfg->dma_burst_size = tgt_res_cfg->dma_burst_size;
	resource_cfg->mac_aggr_delim = tgt_res_cfg->mac_aggr_delim;
	resource_cfg->rx_skip_defrag_timeout_dup_detection_check =
		tgt_res_cfg->rx_skip_defrag_timeout_dup_detection_check;
	resource_cfg->vow_config = tgt_res_cfg->vow_config;
	resource_cfg->gtk_offload_max_vdev = tgt_res_cfg->gtk_offload_max_vdev;
	resource_cfg->num_msdu_desc = tgt_res_cfg->num_msdu_desc;
	resource_cfg->max_frag_entries = tgt_res_cfg->max_frag_entries;
	resource_cfg->num_tdls_vdevs = tgt_res_cfg->num_tdls_vdevs;
	resource_cfg->num_tdls_conn_table_entries =
			tgt_res_cfg->num_tdls_conn_table_entries;
	resource_cfg->beacon_tx_offload_max_vdev =
			tgt_res_cfg->beacon_tx_offload_max_vdev;
	resource_cfg->num_multicast_filter_entries =
			tgt_res_cfg->num_multicast_filter_entries;
	resource_cfg->num_wow_filters =
			tgt_res_cfg->num_wow_filters;
	resource_cfg->num_keep_alive_pattern =
			tgt_res_cfg->num_keep_alive_pattern;
	resource_cfg->keep_alive_pattern_size =
			tgt_res_cfg->keep_alive_pattern_size;
	resource_cfg->max_tdls_concurrent_sleep_sta =
			tgt_res_cfg->max_tdls_concurrent_sleep_sta;
	resource_cfg->max_tdls_concurrent_buffer_sta =
			tgt_res_cfg->max_tdls_concurrent_buffer_sta;
	resource_cfg->wmi_send_separate =
			tgt_res_cfg->wmi_send_separate;
	resource_cfg->num_ocb_vdevs =
			tgt_res_cfg->num_ocb_vdevs;
	resource_cfg->num_ocb_channels =
			tgt_res_cfg->num_ocb_channels;
	resource_cfg->num_ocb_schedules =
			tgt_res_cfg->num_ocb_schedules;
	resource_cfg->bpf_instruction_size = tgt_res_cfg->bpf_instruction_size;
	resource_cfg->max_bssid_rx_filters = tgt_res_cfg->max_bssid_rx_filters;
	resource_cfg->use_pdev_id = tgt_res_cfg->use_pdev_id;

	WMI_RSRC_CFG_FLAG_ATF_CONFIG_ENABLE_SET(resource_cfg->flag1,
						tgt_res_cfg->atf_config);
}
#ifdef CONFIG_MCL
/**
 * send_init_cmd_tlv() - wmi init command
 * @wmi_handle:      pointer to wmi handle
 * @res_cfg:         resource config
 * @num_mem_chunks:  no of mem chunck
 * @mem_chunk:       pointer to mem chunck structure
 *
 * This function sends IE information to firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
static QDF_STATUS send_init_cmd_tlv(wmi_unified_t wmi_handle,
		wmi_resource_config *tgt_res_cfg,
		uint8_t num_mem_chunks, struct wmi_host_mem_chunk *mem_chunks,
		bool action)
{
	wmi_buf_t buf;
	wmi_init_cmd_fixed_param *cmd;
	wmi_abi_version my_vers;
	int num_whitelist;
	uint8_t *buf_ptr;
	wmi_resource_config *resource_cfg;
	wlan_host_memory_chunk *host_mem_chunks;
	uint32_t mem_chunk_len = 0;
	uint16_t idx;
	int len;
	int ret;

	len = sizeof(*cmd) + sizeof(wmi_resource_config) + WMI_TLV_HDR_SIZE;
	mem_chunk_len = (sizeof(wlan_host_memory_chunk) * MAX_MEM_CHUNKS);
	buf = wmi_buf_alloc(wmi_handle, len + mem_chunk_len);
	if (!buf) {
		WMI_LOGD("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_init_cmd_fixed_param *) buf_ptr;
	resource_cfg = (wmi_resource_config *) (buf_ptr + sizeof(*cmd));

	host_mem_chunks = (wlan_host_memory_chunk *)
		(buf_ptr + sizeof(*cmd) + sizeof(wmi_resource_config)
		 + WMI_TLV_HDR_SIZE);

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_init_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(wmi_init_cmd_fixed_param));

	qdf_mem_copy(resource_cfg, tgt_res_cfg, sizeof(wmi_resource_config));
	WMITLV_SET_HDR(&resource_cfg->tlv_header,
			WMITLV_TAG_STRUC_wmi_resource_config,
			WMITLV_GET_STRUCT_TLVLEN(wmi_resource_config));

	for (idx = 0; idx < num_mem_chunks; ++idx) {
		WMITLV_SET_HDR(&(host_mem_chunks[idx].tlv_header),
				WMITLV_TAG_STRUC_wlan_host_memory_chunk,
				WMITLV_GET_STRUCT_TLVLEN
				(wlan_host_memory_chunk));
		host_mem_chunks[idx].ptr = mem_chunks[idx].paddr;
		host_mem_chunks[idx].size = mem_chunks[idx].len;
		host_mem_chunks[idx].req_id = mem_chunks[idx].req_id;
		WMI_LOGD("chunk %d len %d requested ,ptr  0x%x ",
				idx, host_mem_chunks[idx].size,
				host_mem_chunks[idx].ptr);
	}
	cmd->num_host_mem_chunks = num_mem_chunks;
	len += (num_mem_chunks * sizeof(wlan_host_memory_chunk));
	WMITLV_SET_HDR((buf_ptr + sizeof(*cmd) + sizeof(wmi_resource_config)),
			WMITLV_TAG_ARRAY_STRUC,
			(sizeof(wlan_host_memory_chunk) *
			 num_mem_chunks));

	num_whitelist = sizeof(version_whitelist) /
		sizeof(wmi_whitelist_version_info);
	my_vers.abi_version_0 = WMI_ABI_VERSION_0;
	my_vers.abi_version_1 = WMI_ABI_VERSION_1;
	my_vers.abi_version_ns_0 = WMI_ABI_VERSION_NS_0;
	my_vers.abi_version_ns_1 = WMI_ABI_VERSION_NS_1;
	my_vers.abi_version_ns_2 = WMI_ABI_VERSION_NS_2;
	my_vers.abi_version_ns_3 = WMI_ABI_VERSION_NS_3;
#ifdef CONFIG_MCL
	/* This needs to be enabled for WIN Lithium after removing dependency
	 * on wmi_unified.h from priv.h for using wmi_abi_version type */
	wmi_cmp_and_set_abi_version(num_whitelist, version_whitelist,
			&my_vers,
			&wmi_handle->fw_abi_version,
			&cmd->host_abi_vers);
#endif
	WMI_LOGD("%s: INIT_CMD version: %d, %d, 0x%x, 0x%x, 0x%x, 0x%x",
		__func__, WMI_VER_GET_MAJOR(cmd->host_abi_vers.abi_version_0),
			WMI_VER_GET_MINOR(cmd->host_abi_vers.abi_version_0),
			cmd->host_abi_vers.abi_version_ns_0,
			cmd->host_abi_vers.abi_version_ns_1,
			cmd->host_abi_vers.abi_version_ns_2,
			cmd->host_abi_vers.abi_version_ns_3);
#ifdef CONFIG_MCL
	/* Save version sent from host -
	 * Will be used to check ready event
	 */
	qdf_mem_copy(&wmi_handle->final_abi_vers, &cmd->host_abi_vers,
			sizeof(wmi_abi_version));
#endif
	if (action) {
		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_INIT_CMDID);
		if (ret) {
			WMI_LOGE(FL("Failed to send set WMI INIT command ret = %d"), ret);
			wmi_buf_free(buf);
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		wmi_handle->saved_wmi_init_cmd.buf = buf;
		wmi_handle->saved_wmi_init_cmd.buf_len = len;
	}

	return QDF_STATUS_SUCCESS;

}
#endif
/**
 * send_saved_init_cmd_tlv() - wmi init command
 * @wmi_handle:      pointer to wmi handle
 *
 * This function sends IE information to firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
static QDF_STATUS send_saved_init_cmd_tlv(wmi_unified_t wmi_handle)
{
	int status;

	if (!wmi_handle->saved_wmi_init_cmd.buf ||
			!wmi_handle->saved_wmi_init_cmd.buf_len) {
		WMI_LOGP("Service ready ext event w/o WMI_SERVICE_EXT_MSG!");
		return QDF_STATUS_E_FAILURE;
	}
	status = wmi_unified_cmd_send(wmi_handle,
				wmi_handle->saved_wmi_init_cmd.buf,
				wmi_handle->saved_wmi_init_cmd.buf_len,
				WMI_INIT_CMDID);
	if (status) {
		WMI_LOGE(FL("Failed to send set WMI INIT command ret = %d"), status);
		wmi_buf_free(wmi_handle->saved_wmi_init_cmd.buf);
		return QDF_STATUS_E_FAILURE;
	}
	wmi_handle->saved_wmi_init_cmd.buf = NULL;
	wmi_handle->saved_wmi_init_cmd.buf_len = 0;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS save_fw_version_cmd_tlv(wmi_unified_t wmi_handle, void *evt_buf)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_event_fixed_param *ev;


	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	ev = (wmi_service_ready_event_fixed_param *) param_buf->fixed_param;
	if (!ev)
		return QDF_STATUS_E_FAILURE;

#ifdef CONFIG_MCL
	/*Save fw version from service ready message */
	/*This will be used while sending INIT message */
	qdf_mem_copy(&wmi_handle->fw_abi_version, &ev->fw_abi_vers,
			sizeof(wmi_handle->fw_abi_version));
#endif
	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_unified_save_fw_version_cmd() - save fw version
 * @wmi_handle:      pointer to wmi handle
 * @res_cfg:         resource config
 * @num_mem_chunks:  no of mem chunck
 * @mem_chunk:       pointer to mem chunck structure
 *
 * This function sends IE information to firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
static QDF_STATUS check_and_update_fw_version_cmd_tlv(wmi_unified_t wmi_handle,
					  void *evt_buf)
{
	WMI_READY_EVENTID_param_tlvs *param_buf = NULL;
	wmi_ready_event_fixed_param *ev = NULL;

	param_buf = (WMI_READY_EVENTID_param_tlvs *) evt_buf;
	ev = param_buf->fixed_param;
#ifdef CONFIG_MCL
	if (!wmi_versions_are_compatible(&wmi_handle->final_abi_vers,
				&ev->fw_abi_vers)) {
		/*
		 * Error: Our host version and the given firmware version
		 * are incompatible.
		 **/
		WMI_LOGD("%s: Error: Incompatible WMI version."
			"Host: %d,%d,0x%x 0x%x 0x%x 0x%x, FW: %d,%d,0x%x 0x%x 0x%x 0x%x\n",
				__func__,
			WMI_VER_GET_MAJOR(wmi_handle->final_abi_vers.
				abi_version_0),
			WMI_VER_GET_MINOR(wmi_handle->final_abi_vers.
				abi_version_0),
			wmi_handle->final_abi_vers.abi_version_ns_0,
			wmi_handle->final_abi_vers.abi_version_ns_1,
			wmi_handle->final_abi_vers.abi_version_ns_2,
			wmi_handle->final_abi_vers.abi_version_ns_3,
			WMI_VER_GET_MAJOR(ev->fw_abi_vers.abi_version_0),
			WMI_VER_GET_MINOR(ev->fw_abi_vers.abi_version_0),
			ev->fw_abi_vers.abi_version_ns_0,
			ev->fw_abi_vers.abi_version_ns_1,
			ev->fw_abi_vers.abi_version_ns_2,
			ev->fw_abi_vers.abi_version_ns_3);

		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_copy(&wmi_handle->final_abi_vers, &ev->fw_abi_vers,
			sizeof(wmi_abi_version));
	qdf_mem_copy(&wmi_handle->fw_abi_version, &ev->fw_abi_vers,
			sizeof(wmi_abi_version));
#endif

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_base_macaddr_indicate_cmd_tlv() - set base mac address in fw
 * @wmi_handle: wmi handle
 * @custom_addr: base mac address
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_set_base_macaddr_indicate_cmd_tlv(wmi_unified_t wmi_handle,
					 uint8_t *custom_addr)
{
	wmi_pdev_set_base_macaddr_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int err;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGE("Failed to allocate buffer to send base macaddr cmd");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_set_base_macaddr_cmd_fixed_param *) wmi_buf_data(buf);
	qdf_mem_zero(cmd, sizeof(*cmd));

	WMITLV_SET_HDR(&cmd->tlv_header,
		   WMITLV_TAG_STRUC_wmi_pdev_set_base_macaddr_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_pdev_set_base_macaddr_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(custom_addr, &cmd->base_macaddr);
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							WMI_HOST_PDEV_ID_SOC);
	err = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(*cmd),
				   WMI_PDEV_SET_BASE_MACADDR_CMDID);
	if (err) {
		WMI_LOGE("Failed to send set_base_macaddr cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return 0;
}

/**
 * send_log_supported_evt_cmd_tlv() - Enable/Disable FW diag/log events
 * @handle: wmi handle
 * @event:  Event received from FW
 * @len:    Length of the event
 *
 * Enables the low frequency events and disables the high frequency
 * events. Bit 17 indicates if the event if low/high frequency.
 * 1 - high frequency, 0 - low frequency
 *
 * Return: 0 on successfully enabling/disabling the events
 */
static QDF_STATUS send_log_supported_evt_cmd_tlv(wmi_unified_t wmi_handle,
		uint8_t *event,
		uint32_t len)
{
	uint32_t num_of_diag_events_logs;
	wmi_diag_event_log_config_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t *cmd_args, *evt_args;
	uint32_t buf_len, i;

	WMI_DIAG_EVENT_LOG_SUPPORTED_EVENTID_param_tlvs *param_buf;
	wmi_diag_event_log_supported_event_fixed_params *wmi_event;

	WMI_LOGI("Received WMI_DIAG_EVENT_LOG_SUPPORTED_EVENTID");

	param_buf = (WMI_DIAG_EVENT_LOG_SUPPORTED_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMI_LOGE("Invalid log supported event buffer");
		return QDF_STATUS_E_INVAL;
	}
	wmi_event = param_buf->fixed_param;
	num_of_diag_events_logs = wmi_event->num_of_diag_events_logs;
	evt_args = param_buf->diag_events_logs_list;
	if (!evt_args) {
		WMI_LOGE("%s: Event list is empty, num_of_diag_events_logs=%d",
				__func__, num_of_diag_events_logs);
		return QDF_STATUS_E_INVAL;
	}

	WMI_LOGD("%s: num_of_diag_events_logs=%d",
			__func__, num_of_diag_events_logs);

	/* Free any previous allocation */
	if (wmi_handle->events_logs_list)
		qdf_mem_free(wmi_handle->events_logs_list);

	/* Store the event list for run time enable/disable */
	wmi_handle->events_logs_list = qdf_mem_malloc(num_of_diag_events_logs *
			sizeof(uint32_t));
	if (!wmi_handle->events_logs_list) {
		WMI_LOGE("%s: event log list memory allocation failed",
				__func__);
		return QDF_STATUS_E_NOMEM;
	}
	wmi_handle->num_of_diag_events_logs = num_of_diag_events_logs;

	/* Prepare the send buffer */
	buf_len = sizeof(*cmd) + WMI_TLV_HDR_SIZE +
		(num_of_diag_events_logs * sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, buf_len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		qdf_mem_free(wmi_handle->events_logs_list);
		wmi_handle->events_logs_list = NULL;
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_diag_event_log_config_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_diag_event_log_config_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
				wmi_diag_event_log_config_fixed_param));

	cmd->num_of_diag_events_logs = num_of_diag_events_logs;

	buf_ptr += sizeof(wmi_diag_event_log_config_fixed_param);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
			(num_of_diag_events_logs * sizeof(uint32_t)));

	cmd_args = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);

	/* Populate the events */
	for (i = 0; i < num_of_diag_events_logs; i++) {
		/* Low freq (0) - Enable (1) the event
		 * High freq (1) - Disable (0) the event
		 */
		WMI_DIAG_ID_ENABLED_DISABLED_SET(cmd_args[i],
				!(WMI_DIAG_FREQUENCY_GET(evt_args[i])));
		/* Set the event ID */
		WMI_DIAG_ID_SET(cmd_args[i],
				WMI_DIAG_ID_GET(evt_args[i]));
		/* Set the type */
		WMI_DIAG_TYPE_SET(cmd_args[i],
				WMI_DIAG_TYPE_GET(evt_args[i]));
		/* Storing the event/log list in WMI */
		wmi_handle->events_logs_list[i] = evt_args[i];
	}

	if (wmi_unified_cmd_send(wmi_handle, buf, buf_len,
				WMI_DIAG_EVENT_LOG_CONFIG_CMDID)) {
		WMI_LOGE("%s: WMI_DIAG_EVENT_LOG_CONFIG_CMDID failed",
				__func__);
		wmi_buf_free(buf);
		/* Not clearing events_logs_list, though wmi cmd failed.
		 * Host can still have this list
		 */
		return QDF_STATUS_E_INVAL;
	}

	return 0;
}

/**
 * send_enable_specific_fw_logs_cmd_tlv() - Start/Stop logging of diag log id
 * @wmi_handle: wmi handle
 * @start_log: Start logging related parameters
 *
 * Send the command to the FW based on which specific logging of diag
 * event/log id can be started/stopped
 *
 * Return: None
 */
static QDF_STATUS send_enable_specific_fw_logs_cmd_tlv(wmi_unified_t wmi_handle,
		struct wmi_wifi_start_log *start_log)
{
	wmi_diag_event_log_config_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len, count, log_level, i;
	uint32_t *cmd_args;
	uint32_t total_len;
	count = 0;

	if (!wmi_handle->events_logs_list) {
		WMI_LOGE("%s: Not received event/log list from FW, yet",
				__func__);
		return QDF_STATUS_E_NOMEM;
	}
	/* total_len stores the number of events where BITS 17 and 18 are set.
	 * i.e., events of high frequency (17) and for extended debugging (18)
	 */
	total_len = 0;
	for (i = 0; i < wmi_handle->num_of_diag_events_logs; i++) {
		if ((WMI_DIAG_FREQUENCY_GET(wmi_handle->events_logs_list[i])) &&
		    (WMI_DIAG_EXT_FEATURE_GET(wmi_handle->events_logs_list[i])))
			total_len++;
	}

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE +
		(total_len * sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_diag_event_log_config_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_diag_event_log_config_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
				wmi_diag_event_log_config_fixed_param));

	cmd->num_of_diag_events_logs = total_len;

	buf_ptr += sizeof(wmi_diag_event_log_config_fixed_param);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
			(total_len * sizeof(uint32_t)));

	cmd_args = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);

	if (start_log->verbose_level >= WMI_LOG_LEVEL_ACTIVE)
		log_level = 1;
	else
		log_level = 0;

	WMI_LOGD("%s: Length:%d, Log_level:%d", __func__, total_len, log_level);
	for (i = 0; i < wmi_handle->num_of_diag_events_logs; i++) {
		uint32_t val = wmi_handle->events_logs_list[i];
		if ((WMI_DIAG_FREQUENCY_GET(val)) &&
				(WMI_DIAG_EXT_FEATURE_GET(val))) {

			WMI_DIAG_ID_SET(cmd_args[count],
					WMI_DIAG_ID_GET(val));
			WMI_DIAG_TYPE_SET(cmd_args[count],
					WMI_DIAG_TYPE_GET(val));
			WMI_DIAG_ID_ENABLED_DISABLED_SET(cmd_args[count],
					log_level);
			WMI_LOGD("%s: Idx:%d, val:%x", __func__, i, val);
			count++;
		}
	}

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_DIAG_EVENT_LOG_CONFIG_CMDID)) {
		WMI_LOGE("%s: WMI_DIAG_EVENT_LOG_CONFIG_CMDID failed",
				__func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_flush_logs_to_fw_cmd_tlv() - Send log flush command to FW
 * @wmi_handle: WMI handle
 *
 * This function is used to send the flush command to the FW,
 * that will flush the fw logs that are residue in the FW
 *
 * Return: None
 */
static QDF_STATUS send_flush_logs_to_fw_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_debug_mesg_flush_fixed_param *cmd;
	wmi_buf_t buf;
	int len = sizeof(*cmd);
	QDF_STATUS ret;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_debug_mesg_flush_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_debug_mesg_flush_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
				wmi_debug_mesg_flush_fixed_param));
	cmd->reserved0 = 0;

	ret = wmi_unified_cmd_send(wmi_handle,
			buf,
			len,
			WMI_DEBUG_MESG_FLUSH_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("Failed to send WMI_DEBUG_MESG_FLUSH_CMDID");
		wmi_buf_free(buf);
		return QDF_STATUS_E_INVAL;
	}
	WMI_LOGI("Sent WMI_DEBUG_MESG_FLUSH_CMDID to FW");

	return ret;
}

/**
 * send_pdev_set_pcl_cmd_tlv() - Send WMI_SOC_SET_PCL_CMDID to FW
 * @wmi_handle: wmi handle
 * @msg: PCL structure containing the PCL and the number of channels
 *
 * WMI_PDEV_SET_PCL_CMDID provides a Preferred Channel List (PCL) to the WLAN
 * firmware. The DBS Manager is the consumer of this information in the WLAN
 * firmware. The channel list will be used when a Virtual DEVice (VDEV) needs
 * to migrate to a new channel without host driver involvement. An example of
 * this behavior is Legacy Fast Roaming (LFR 3.0). Generally, the host will
 * manage the channel selection without firmware involvement.
 *
 * WMI_PDEV_SET_PCL_CMDID will carry only the weight list and not the actual
 * channel list. The weights corresponds to the channels sent in
 * WMI_SCAN_CHAN_LIST_CMDID. The channels from PCL would be having a higher
 * weightage compared to the non PCL channels.
 *
 * Return: Success if the cmd is sent successfully to the firmware
 */
static QDF_STATUS send_pdev_set_pcl_cmd_tlv(wmi_unified_t wmi_handle,
				struct wmi_pcl_chan_weights *msg)
{
	wmi_pdev_set_pcl_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t *cmd_args, i, len;
	uint32_t chan_len;

	chan_len = msg->saved_num_chan;

	len = sizeof(*cmd) +
		WMI_TLV_HDR_SIZE + (chan_len * sizeof(uint32_t));

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_set_pcl_cmd_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_set_pcl_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_pdev_set_pcl_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							WMI_HOST_PDEV_ID_SOC);
	cmd->num_chan = chan_len;
	WMI_LOGI("%s: Total chan (PCL) len:%d", __func__, cmd->num_chan);

	buf_ptr += sizeof(wmi_pdev_set_pcl_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
			(chan_len * sizeof(uint32_t)));
	cmd_args = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);
	for (i = 0; i < chan_len ; i++) {
		cmd_args[i] = msg->weighed_valid_list[i];
		WMI_LOGI("%s: chan:%d weight:%d", __func__,
			msg->saved_chan_list[i], cmd_args[i]);
	}
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_PDEV_SET_PCL_CMDID)) {
		WMI_LOGE("%s: Failed to send WMI_PDEV_SET_PCL_CMDID", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_pdev_set_hw_mode_cmd_tlv() - Send WMI_PDEV_SET_HW_MODE_CMDID to FW
 * @wmi_handle: wmi handle
 * @msg: Structure containing the following parameters
 *
 * - hw_mode_index: The HW_Mode field is a enumerated type that is selected
 * from the HW_Mode table, which is returned in the WMI_SERVICE_READY_EVENTID.
 *
 * Provides notification to the WLAN firmware that host driver is requesting a
 * HardWare (HW) Mode change. This command is needed to support iHelium in the
 * configurations that include the Dual Band Simultaneous (DBS) feature.
 *
 * Return: Success if the cmd is sent successfully to the firmware
 */
static QDF_STATUS send_pdev_set_hw_mode_cmd_tlv(wmi_unified_t wmi_handle,
				uint32_t hw_mode_index)
{
	wmi_pdev_set_hw_mode_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_set_hw_mode_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_set_hw_mode_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(wmi_pdev_set_hw_mode_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							WMI_HOST_PDEV_ID_SOC);
	cmd->hw_mode_index = hw_mode_index;
	WMI_LOGI("%s: HW mode index:%d", __func__, cmd->hw_mode_index);

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_PDEV_SET_HW_MODE_CMDID)) {
		WMI_LOGE("%s: Failed to send WMI_PDEV_SET_HW_MODE_CMDID",
			__func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_pdev_set_dual_mac_config_cmd_tlv() - Set dual mac config to FW
 * @wmi_handle: wmi handle
 * @msg: Dual MAC config parameters
 *
 * Configures WLAN firmware with the dual MAC features
 *
 * Return: QDF_STATUS. 0 on success.
 */
static
QDF_STATUS send_pdev_set_dual_mac_config_cmd_tlv(wmi_unified_t wmi_handle,
		struct wmi_dual_mac_config *msg)
{
	wmi_pdev_set_mac_config_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint32_t len;

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_pdev_set_mac_config_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_set_mac_config_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_pdev_set_mac_config_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
							WMI_HOST_PDEV_ID_SOC);
	cmd->concurrent_scan_config_bits = msg->scan_config;
	cmd->fw_mode_config_bits = msg->fw_mode_config;
	WMI_LOGI("%s: scan_config:%x fw_mode_config:%x",
			__func__, msg->scan_config, msg->fw_mode_config);

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_PDEV_SET_MAC_CONFIG_CMDID)) {
		WMI_LOGE("%s: Failed to send WMI_PDEV_SET_MAC_CONFIG_CMDID",
				__func__);
		wmi_buf_free(buf);
	}
	return QDF_STATUS_SUCCESS;
}

#ifdef BIG_ENDIAN_HOST
/**
* fips_conv_data_be() - LE to BE conversion of FIPS ev data
* @param data_len - data length
* @param data - pointer to data
*
* Return: QDF_STATUS - success or error status
*/
static QDF_STATUS fips_align_data_be(wmi_unified_t wmi_handle,
			struct fips_params *param)
{
	unsigned char *key_unaligned, *data_unaligned;
	int c;
	u_int8_t *key_aligned = NULL;
	u_int8_t *data_aligned = NULL;

	/* Assigning unaligned space to copy the key */
	key_unaligned = qdf_mem_malloc(
		sizeof(u_int8_t)*param->key_len + FIPS_ALIGN);
	data_unaligned = qdf_mem_malloc(
		sizeof(u_int8_t)*param->data_len + FIPS_ALIGN);

	/* Checking if kmalloc is succesful to allocate space */
	if (key_unaligned == NULL)
		return QDF_STATUS_SUCCESS;
	/* Checking if space is aligned */
	if (!FIPS_IS_ALIGNED(key_unaligned, FIPS_ALIGN)) {
		/* align to 4 */
		key_aligned =
		(u_int8_t *)FIPS_ALIGNTO(key_unaligned,
			FIPS_ALIGN);
	} else {
		key_aligned = (u_int8_t *)key_unaligned;
	}

	/* memset and copy content from key to key aligned */
	OS_MEMSET(key_aligned, 0, param->key_len);
	OS_MEMCPY(key_aligned, param->key, param->key_len);

	/* print a hexdump for host debug */
	print_hex_dump(KERN_DEBUG,
		"\t Aligned and Copied Key:@@@@ ",
		DUMP_PREFIX_NONE,
		16, 1, key_aligned, param->key_len, true);

	/* Checking if kmalloc is succesful to allocate space */
	if (data_unaligned == NULL)
		return QDF_STATUS_SUCCESS;
	/* Checking of space is aligned */
	if (!FIPS_IS_ALIGNED(data_unaligned, FIPS_ALIGN)) {
		/* align to 4 */
		data_aligned =
		(u_int8_t *)FIPS_ALIGNTO(data_unaligned,
				FIPS_ALIGN);
	} else {
		data_aligned = (u_int8_t *)data_unaligned;
	}

	/* memset and copy content from data to data aligned */
	OS_MEMSET(data_aligned, 0, param->data_len);
	OS_MEMCPY(data_aligned, param->data, param->data_len);

	/* print a hexdump for host debug */
	print_hex_dump(KERN_DEBUG,
		"\t Properly Aligned and Copied Data:@@@@ ",
	DUMP_PREFIX_NONE,
	16, 1, data_aligned, param->data_len, true);

	/* converting to little Endian both key_aligned and
	* data_aligned*/
	for (c = 0; c < param->key_len/4; c++) {
		*((u_int32_t *)key_aligned+c) =
		qdf_cpu_to_le32(*((u_int32_t *)key_aligned+c));
	}
	for (c = 0; c < param->data_len/4; c++) {
		*((u_int32_t *)data_aligned+c) =
		qdf_cpu_to_le32(*((u_int32_t *)data_aligned+c));
	}

	/* update endian data to key and data vectors */
	OS_MEMCPY(param->key, key_aligned, param->key_len);
	OS_MEMCPY(param->data, data_aligned, param->data_len);

	/* clean up allocated spaces */
	qdf_mem_free(key_unaligned);
	key_unaligned = NULL;
	key_aligned = NULL;

	qdf_mem_free(data_unaligned);
	data_unaligned = NULL;
	data_aligned = NULL;

	return QDF_STATUS_SUCCESS;
}
#else
/**
* fips_align_data_be() - DUMMY for LE platform
*
* Return: QDF_STATUS - success
*/
static QDF_STATUS fips_align_data_be(wmi_unified_t wmi_handle,
		struct fips_params *param)
{
	return QDF_STATUS_SUCCESS;
}
#endif


/**
 * send_pdev_fips_cmd_tlv() - send pdev fips cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold pdev fips param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_fips_cmd_tlv(wmi_unified_t wmi_handle,
		struct fips_params *param)
{
	wmi_pdev_fips_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len = sizeof(wmi_pdev_fips_cmd_fixed_param);
	QDF_STATUS retval = QDF_STATUS_SUCCESS;

	/* Length TLV placeholder for array of bytes */
	len += WMI_TLV_HDR_SIZE;
	if (param->data_len)
		len += (param->data_len*sizeof(uint8_t));

	/*
	* Data length must be multiples of 16 bytes - checked against 0xF -
	* and must be less than WMI_SVC_MSG_SIZE - static size of
	* wmi_pdev_fips_cmd structure
	*/

	/* do sanity on the input */
	if (!(((param->data_len & 0xF) == 0) &&
			((param->data_len > 0) &&
			(param->data_len < (WMI_HOST_MAX_BUFFER_SIZE -
		sizeof(wmi_pdev_fips_cmd_fixed_param)))))) {
		return QDF_STATUS_E_INVAL;
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_pdev_fips_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_fips_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_pdev_fips_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	if (param->key != NULL && param->data != NULL) {
		cmd->key_len = param->key_len;
		cmd->data_len = param->data_len;
		cmd->fips_cmd = !!(param->op);

		if (fips_align_data_be(wmi_handle, param) != QDF_STATUS_SUCCESS)
			return QDF_STATUS_E_FAILURE;

		qdf_mem_copy(cmd->key, param->key, param->key_len);

		if (param->mode == FIPS_ENGINE_AES_CTR ||
			param->mode == FIPS_ENGINE_AES_MIC) {
			cmd->mode = param->mode;
		} else {
			cmd->mode = FIPS_ENGINE_AES_CTR;
		}
		qdf_print(KERN_ERR "Key len = %d, Data len = %d\n",
			cmd->key_len, cmd->data_len);

		print_hex_dump(KERN_DEBUG, "Key: ", DUMP_PREFIX_NONE, 16, 1,
				cmd->key, cmd->key_len, true);
		buf_ptr += sizeof(*cmd);

		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE, param->data_len);

		buf_ptr += WMI_TLV_HDR_SIZE;
		if (param->data_len)
			qdf_mem_copy(buf_ptr,
				(uint8_t *) param->data, param->data_len);

		print_hex_dump(KERN_DEBUG, "Plain text: ", DUMP_PREFIX_NONE,
			16, 1, buf_ptr, cmd->data_len, true);

		buf_ptr += param->data_len;

		retval = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_FIPS_CMDID);
		qdf_print("%s return value %d\n", __func__, retval);
	} else {
		qdf_print("\n%s:%d Key or Data is NULL\n", __func__, __LINE__);
		wmi_buf_free(buf);
		retval = -QDF_STATUS_E_BADMSG;
	}

	return retval;
}

#ifdef WLAN_PMO_ENABLE
/**
 * send_add_wow_wakeup_event_cmd_tlv() -  Configures wow wakeup events.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @bitmap: Event bitmap
 * @enable: enable/disable
 *
 * Return: CDF status
 */
static QDF_STATUS send_add_wow_wakeup_event_cmd_tlv(wmi_unified_t wmi_handle,
					uint32_t vdev_id,
					uint32_t bitmap,
					bool enable)
{
	WMI_WOW_ADD_DEL_EVT_CMD_fixed_param *cmd;
	uint16_t len;
	wmi_buf_t buf;
	int ret;

	len = sizeof(WMI_WOW_ADD_DEL_EVT_CMD_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (WMI_WOW_ADD_DEL_EVT_CMD_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_WOW_ADD_DEL_EVT_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_WOW_ADD_DEL_EVT_CMD_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->is_add = enable;
	cmd->event_bitmap = bitmap;

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WOW_ENABLE_DISABLE_WAKE_EVENT_CMDID);
	if (ret) {
		WMI_LOGE("Failed to config wow wakeup event");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	WMI_LOGD("Wakeup pattern 0x%x %s in fw", bitmap,
		 enable ? "enabled" : "disabled");

	return QDF_STATUS_SUCCESS;
}

/**
 * send_wow_patterns_to_fw_cmd_tlv() - Sends WOW patterns to FW.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @ptrn_id: pattern id
 * @ptrn: pattern
 * @ptrn_len: pattern length
 * @ptrn_offset: pattern offset
 * @mask: mask
 * @mask_len: mask length
 * @user: true for user configured pattern and false for default pattern
 * @default_patterns: default patterns
 *
 * Return: CDF status
 */
static QDF_STATUS send_wow_patterns_to_fw_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t vdev_id, uint8_t ptrn_id,
				const uint8_t *ptrn, uint8_t ptrn_len,
				uint8_t ptrn_offset, const uint8_t *mask,
				uint8_t mask_len, bool user,
				uint8_t default_patterns)
{
	WMI_WOW_ADD_PATTERN_CMD_fixed_param *cmd;
	WOW_BITMAP_PATTERN_T *bitmap_pattern;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t len;
	int ret;

	len = sizeof(WMI_WOW_ADD_PATTERN_CMD_fixed_param) +
		WMI_TLV_HDR_SIZE +
		1 * sizeof(WOW_BITMAP_PATTERN_T) +
		WMI_TLV_HDR_SIZE +
		0 * sizeof(WOW_IPV4_SYNC_PATTERN_T) +
		WMI_TLV_HDR_SIZE +
		0 * sizeof(WOW_IPV6_SYNC_PATTERN_T) +
		WMI_TLV_HDR_SIZE +
		0 * sizeof(WOW_MAGIC_PATTERN_CMD) +
		WMI_TLV_HDR_SIZE +
		0 * sizeof(A_UINT32) + WMI_TLV_HDR_SIZE + 1 * sizeof(A_UINT32);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_WOW_ADD_PATTERN_CMD_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_WOW_ADD_PATTERN_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_WOW_ADD_PATTERN_CMD_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->pattern_id = ptrn_id;

	cmd->pattern_type = WOW_BITMAP_PATTERN;
	buf_ptr += sizeof(WMI_WOW_ADD_PATTERN_CMD_fixed_param);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(WOW_BITMAP_PATTERN_T));
	buf_ptr += WMI_TLV_HDR_SIZE;
	bitmap_pattern = (WOW_BITMAP_PATTERN_T *) buf_ptr;

	WMITLV_SET_HDR(&bitmap_pattern->tlv_header,
		       WMITLV_TAG_STRUC_WOW_BITMAP_PATTERN_T,
		       WMITLV_GET_STRUCT_TLVLEN(WOW_BITMAP_PATTERN_T));

	qdf_mem_copy(&bitmap_pattern->patternbuf[0], ptrn, ptrn_len);
	qdf_mem_copy(&bitmap_pattern->bitmaskbuf[0], mask, mask_len);

	bitmap_pattern->pattern_offset = ptrn_offset;
	bitmap_pattern->pattern_len = ptrn_len;

	if (bitmap_pattern->pattern_len > WOW_DEFAULT_BITMAP_PATTERN_SIZE)
		bitmap_pattern->pattern_len = WOW_DEFAULT_BITMAP_PATTERN_SIZE;

	if (bitmap_pattern->pattern_len > WOW_DEFAULT_BITMASK_SIZE)
		bitmap_pattern->pattern_len = WOW_DEFAULT_BITMASK_SIZE;

	bitmap_pattern->bitmask_len = bitmap_pattern->pattern_len;
	bitmap_pattern->pattern_id = ptrn_id;

	WMI_LOGI("vdev: %d, ptrn id: %d, ptrn len: %d, ptrn offset: %d user %d",
		 cmd->vdev_id, cmd->pattern_id, bitmap_pattern->pattern_len,
		 bitmap_pattern->pattern_offset, user);
	WMI_LOGI("Pattern : ");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_INFO,
		&bitmap_pattern->patternbuf[0], bitmap_pattern->pattern_len);

	WMI_LOGI("Mask : ");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_INFO,
		&bitmap_pattern->bitmaskbuf[0], bitmap_pattern->pattern_len);

	buf_ptr += sizeof(WOW_BITMAP_PATTERN_T);

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_IPV4_SYNC_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_IPV6_SYNC_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_MAGIC_PATTERN_CMD but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for pattern_info_timeout but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for ratelimit_interval with dummy data as this fix elem */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, 1 * sizeof(A_UINT32));
	buf_ptr += WMI_TLV_HDR_SIZE;
	*(A_UINT32 *) buf_ptr = 0;

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WOW_ADD_WAKE_PATTERN_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send wow ptrn to fw", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * fill_arp_offload_params_tlv() - Fill ARP offload data
 * @wmi_handle: wmi handle
 * @offload_req: offload request
 * @buf_ptr: buffer pointer
 *
 * To fill ARP offload data to firmware
 * when target goes to wow mode.
 *
 * Return: None
 */
static void fill_arp_offload_params_tlv(wmi_unified_t wmi_handle,
		struct pmo_arp_offload_params *offload_req, uint8_t **buf_ptr)
{

	int i;
	WMI_ARP_OFFLOAD_TUPLE *arp_tuple;
	bool enable_or_disable = offload_req->enable;

	WMITLV_SET_HDR(*buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		(WMI_MAX_ARP_OFFLOADS*sizeof(WMI_ARP_OFFLOAD_TUPLE)));
	*buf_ptr += WMI_TLV_HDR_SIZE;
	for (i = 0; i < WMI_MAX_ARP_OFFLOADS; i++) {
		arp_tuple = (WMI_ARP_OFFLOAD_TUPLE *)*buf_ptr;
		WMITLV_SET_HDR(&arp_tuple->tlv_header,
			WMITLV_TAG_STRUC_WMI_ARP_OFFLOAD_TUPLE,
			WMITLV_GET_STRUCT_TLVLEN(WMI_ARP_OFFLOAD_TUPLE));

		/* Fill data for ARP and NS in the first tupple for LA */
		if ((enable_or_disable & PMO_OFFLOAD_ENABLE) && (i == 0)) {
			/* Copy the target ip addr and flags */
			arp_tuple->flags = WMI_ARPOFF_FLAGS_VALID;
			qdf_mem_copy(&arp_tuple->target_ipaddr,
					offload_req->host_ipv4_addr,
					WMI_IPV4_ADDR_LEN);
			WMI_LOGD("ARPOffload IP4 address: %pI4",
					offload_req->host_ipv4_addr);
		}
		*buf_ptr += sizeof(WMI_ARP_OFFLOAD_TUPLE);
	}
}

#ifdef WLAN_NS_OFFLOAD
/**
 * fill_ns_offload_params_tlv() - Fill NS offload data
 * @wmi|_handle: wmi handle
 * @offload_req: offload request
 * @buf_ptr: buffer pointer
 *
 * To fill NS offload data to firmware
 * when target goes to wow mode.
 *
 * Return: None
 */
static void fill_ns_offload_params_tlv(wmi_unified_t wmi_handle,
		struct pmo_ns_offload_params *ns_req, uint8_t **buf_ptr)
{

	int i;
	WMI_NS_OFFLOAD_TUPLE *ns_tuple;

	WMITLV_SET_HDR(*buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		(WMI_MAX_NS_OFFLOADS * sizeof(WMI_NS_OFFLOAD_TUPLE)));
	*buf_ptr += WMI_TLV_HDR_SIZE;
	for (i = 0; i < WMI_MAX_NS_OFFLOADS; i++) {
		ns_tuple = (WMI_NS_OFFLOAD_TUPLE *)*buf_ptr;
		WMITLV_SET_HDR(&ns_tuple->tlv_header,
			WMITLV_TAG_STRUC_WMI_NS_OFFLOAD_TUPLE,
			(sizeof(WMI_NS_OFFLOAD_TUPLE) - WMI_TLV_HDR_SIZE));

		/*
		 * Fill data only for NS offload in the first ARP tuple for LA
		 */
		if ((ns_req->enable & PMO_OFFLOAD_ENABLE)) {
			ns_tuple->flags |= WMI_NSOFF_FLAGS_VALID;
			/* Copy the target/solicitation/remote ip addr */
			if (ns_req->target_ipv6_addr_valid[i])
				qdf_mem_copy(&ns_tuple->target_ipaddr[0],
					&ns_req->target_ipv6_addr[i],
					sizeof(WMI_IPV6_ADDR));
			qdf_mem_copy(&ns_tuple->solicitation_ipaddr,
				&ns_req->self_ipv6_addr[i],
				sizeof(WMI_IPV6_ADDR));
			if (ns_req->target_ipv6_addr_ac_type[i]) {
				ns_tuple->flags |=
					WMI_NSOFF_FLAGS_IS_IPV6_ANYCAST;
			}
			WMI_LOGD("Index %d NS solicitedIp %pI6, targetIp %pI6",
				i, &ns_req->self_ipv6_addr[i],
				&ns_req->target_ipv6_addr[i]);

			/* target MAC is optional, check if it is valid,
			 * if this is not valid, the target will use the known
			 * local MAC address rather than the tuple
			 */
			WMI_CHAR_ARRAY_TO_MAC_ADDR(
				ns_req->self_macaddr.bytes,
				&ns_tuple->target_mac);
			if ((ns_tuple->target_mac.mac_addr31to0 != 0) ||
				(ns_tuple->target_mac.mac_addr47to32 != 0)) {
				ns_tuple->flags |= WMI_NSOFF_FLAGS_MAC_VALID;
			}
		}
		*buf_ptr += sizeof(WMI_NS_OFFLOAD_TUPLE);
	}
}


/**
 * fill_nsoffload_ext_tlv() - Fill NS offload ext data
 * @wmi: wmi handle
 * @offload_req: offload request
 * @buf_ptr: buffer pointer
 *
 * To fill extended NS offload extended data to firmware
 * when target goes to wow mode.
 *
 * Return: None
 */
static void fill_nsoffload_ext_tlv(wmi_unified_t wmi_handle,
		struct pmo_ns_offload_params *ns_req, uint8_t **buf_ptr)
{
	int i;
	WMI_NS_OFFLOAD_TUPLE *ns_tuple;
	uint32_t count, num_ns_ext_tuples;

	count = ns_req->num_ns_offload_count;
	num_ns_ext_tuples = ns_req->num_ns_offload_count -
		WMI_MAX_NS_OFFLOADS;

	/* Populate extended NS offload tuples */
	WMITLV_SET_HDR(*buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		(num_ns_ext_tuples * sizeof(WMI_NS_OFFLOAD_TUPLE)));
	*buf_ptr += WMI_TLV_HDR_SIZE;
	for (i = WMI_MAX_NS_OFFLOADS; i < count; i++) {
		ns_tuple = (WMI_NS_OFFLOAD_TUPLE *)*buf_ptr;
		WMITLV_SET_HDR(&ns_tuple->tlv_header,
			WMITLV_TAG_STRUC_WMI_NS_OFFLOAD_TUPLE,
			(sizeof(WMI_NS_OFFLOAD_TUPLE)-WMI_TLV_HDR_SIZE));

		/*
		 * Fill data only for NS offload in the first ARP tuple for LA
		 */
		if ((ns_req->enable & PMO_OFFLOAD_ENABLE)) {
			ns_tuple->flags |= WMI_NSOFF_FLAGS_VALID;
			/* Copy the target/solicitation/remote ip addr */
			if (ns_req->target_ipv6_addr_valid[i])
				qdf_mem_copy(&ns_tuple->target_ipaddr[0],
					&ns_req->target_ipv6_addr[i],
					sizeof(WMI_IPV6_ADDR));
			qdf_mem_copy(&ns_tuple->solicitation_ipaddr,
				&ns_req->self_ipv6_addr[i],
				sizeof(WMI_IPV6_ADDR));
			if (ns_req->target_ipv6_addr_ac_type[i]) {
				ns_tuple->flags |=
					WMI_NSOFF_FLAGS_IS_IPV6_ANYCAST;
			}
			WMI_LOGD("Index %d NS solicitedIp %pI6, targetIp %pI6",
				i, &ns_req->self_ipv6_addr[i],
				&ns_req->target_ipv6_addr[i]);

			/* target MAC is optional, check if it is valid,
			 * if this is not valid, the target will use the
			 * known local MAC address rather than the tuple
			 */
			 WMI_CHAR_ARRAY_TO_MAC_ADDR(
				ns_req->self_macaddr.bytes,
				&ns_tuple->target_mac);
			if ((ns_tuple->target_mac.mac_addr31to0 != 0) ||
				(ns_tuple->target_mac.mac_addr47to32 != 0)) {
				ns_tuple->flags |= WMI_NSOFF_FLAGS_MAC_VALID;
			}
		}
		*buf_ptr += sizeof(WMI_NS_OFFLOAD_TUPLE);
	}
}
#else
static void fill_ns_offload_params_tlv(wmi_unified_t wmi_handle,
		struct pmo_ns_offload_params *ns_req, uint8_t **buf_ptr)
{
}

static void fill_nsoffload_ext_tlv(wmi_unified_t wmi_handle,
		struct pmo_ns_offload_params *ns_req, uint8_t **buf_ptr)
{
}
#endif

/**
 * send_enable_arp_ns_offload_cmd_tlv() - enable ARP NS offload
 * @wma: wmi handle
 * @arp_offload_req: arp offload request
 * @ns_offload_req: ns offload request
 * @arp_only: flag
 *
 * To configure ARP NS off load data to firmware
 * when target goes to wow mode.
 *
 * Return: QDF Status
 */
static QDF_STATUS send_enable_arp_ns_offload_cmd_tlv(wmi_unified_t wmi_handle,
			   struct pmo_arp_offload_params *arp_offload_req,
			   struct pmo_ns_offload_params *ns_offload_req,
			   uint8_t vdev_id)
{
	int32_t res;
	WMI_SET_ARP_NS_OFFLOAD_CMD_fixed_param *cmd;
	A_UINT8 *buf_ptr;
	wmi_buf_t buf;
	int32_t len;
	uint32_t count = 0, num_ns_ext_tuples = 0;

	count = ns_offload_req->num_ns_offload_count;

	/*
	 * TLV place holder size for array of NS tuples
	 * TLV place holder size for array of ARP tuples
	 */
	len = sizeof(WMI_SET_ARP_NS_OFFLOAD_CMD_fixed_param) +
		WMI_TLV_HDR_SIZE +
		WMI_MAX_NS_OFFLOADS * sizeof(WMI_NS_OFFLOAD_TUPLE) +
		WMI_TLV_HDR_SIZE +
		WMI_MAX_ARP_OFFLOADS * sizeof(WMI_ARP_OFFLOAD_TUPLE);

	/*
	 * If there are more than WMI_MAX_NS_OFFLOADS addresses then allocate
	 * extra length for extended NS offload tuples which follows ARP offload
	 * tuples. Host needs to fill this structure in following format:
	 * 2 NS ofload tuples
	 * 2 ARP offload tuples
	 * N numbers of extended NS offload tuples if HDD has given more than
	 * 2 NS offload addresses
	 */
	if (count > WMI_MAX_NS_OFFLOADS) {
		num_ns_ext_tuples = count - WMI_MAX_NS_OFFLOADS;
		len += WMI_TLV_HDR_SIZE + num_ns_ext_tuples
			   * sizeof(WMI_NS_OFFLOAD_TUPLE);
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (A_UINT8 *) wmi_buf_data(buf);
	cmd = (WMI_SET_ARP_NS_OFFLOAD_CMD_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_SET_ARP_NS_OFFLOAD_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_SET_ARP_NS_OFFLOAD_CMD_fixed_param));
	cmd->flags = 0;
	cmd->vdev_id = vdev_id;
	cmd->num_ns_ext_tuples = num_ns_ext_tuples;

	WMI_LOGD("ARP NS Offload vdev_id: %d", cmd->vdev_id);

	buf_ptr += sizeof(WMI_SET_ARP_NS_OFFLOAD_CMD_fixed_param);
	fill_ns_offload_params_tlv(wmi_handle, ns_offload_req, &buf_ptr);
	fill_arp_offload_params_tlv(wmi_handle, arp_offload_req, &buf_ptr);
	if (num_ns_ext_tuples)
		fill_nsoffload_ext_tlv(wmi_handle, ns_offload_req, &buf_ptr);

	res = wmi_unified_cmd_send(wmi_handle, buf, len,
				     WMI_SET_ARP_NS_OFFLOAD_CMDID);
	if (res) {
		WMI_LOGE("Failed to enable ARP NDP/NSffload");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_enable_enhance_multicast_offload_tlv() - send enhance multicast offload
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @action: true for enable else false
 *
 * To enable enhance multicast offload to firmware
 * when target goes to wow mode.
 *
 * Return: QDF Status
 */

static
QDF_STATUS send_enable_enhance_multicast_offload_tlv(
		wmi_unified_t wmi_handle,
		uint8_t vdev_id, bool action)
{
	QDF_STATUS status;
	wmi_buf_t buf;
	wmi_config_enhanced_mcast_filter_cmd_fixed_param *cmd;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGE("Failed to allocate buffer to send set key cmd");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_config_enhanced_mcast_filter_cmd_fixed_param *)
							wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_config_enhanced_mcast_filter_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_config_enhanced_mcast_filter_cmd_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->enable = ((action == 0) ? ENHANCED_MCAST_FILTER_DISABLED :
			ENHANCED_MCAST_FILTER_ENABLED);
	WMI_LOGD("%s: config enhance multicast offload action %d for vdev %d",
		__func__, action, vdev_id);
	status = wmi_unified_cmd_send(wmi_handle, buf,
			sizeof(*cmd), WMI_CONFIG_ENHANCED_MCAST_FILTER_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		qdf_nbuf_free(buf);
		WMI_LOGE("%s:Failed to send ENHANCED_MCAST_FILTER_CMDID",
			__func__);
	}

	return status;
}

/**
 * extract_gtk_rsp_event_tlv() - extract gtk rsp params from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param hdr: Pointer to hold header
 * @param bufp: Pointer to hold pointer to rx param buffer
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_gtk_rsp_event_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, struct pmo_gtk_rsp_params *gtk_rsp_param, uint32_t len)
{
	WMI_GTK_OFFLOAD_STATUS_EVENT_fixed_param *fixed_param;
	WMI_GTK_OFFLOAD_STATUS_EVENTID_param_tlvs *param_buf;

	param_buf = (WMI_GTK_OFFLOAD_STATUS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("gtk param_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (len < sizeof(WMI_GTK_OFFLOAD_STATUS_EVENT_fixed_param)) {
		WMI_LOGE("Invalid length for GTK status");
		return QDF_STATUS_E_INVAL;
	}

	fixed_param = (WMI_GTK_OFFLOAD_STATUS_EVENT_fixed_param *)
		param_buf->fixed_param;
	gtk_rsp_param->vdev_id = fixed_param->vdev_id;
	gtk_rsp_param->status_flag = QDF_STATUS_SUCCESS;
	gtk_rsp_param->refresh_cnt = fixed_param->refresh_cnt;
	qdf_mem_copy(&gtk_rsp_param->replay_counter,
		&fixed_param->replay_counter,
		GTK_REPLAY_COUNTER_BYTES);

	return QDF_STATUS_SUCCESS;

}

#ifdef FEATURE_WLAN_RA_FILTERING
/**
 * send_wow_sta_ra_filter_cmd_tlv() - set RA filter pattern in fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: CDF status
 */
static QDF_STATUS send_wow_sta_ra_filter_cmd_tlv(wmi_unified_t wmi_handle,
		   uint8_t vdev_id, uint8_t default_pattern,
		   uint16_t rate_limit_interval)
{

	WMI_WOW_ADD_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t len;
	int ret;

	len = sizeof(WMI_WOW_ADD_PATTERN_CMD_fixed_param) +
	      WMI_TLV_HDR_SIZE +
	      0 * sizeof(WOW_BITMAP_PATTERN_T) +
	      WMI_TLV_HDR_SIZE +
	      0 * sizeof(WOW_IPV4_SYNC_PATTERN_T) +
	      WMI_TLV_HDR_SIZE +
	      0 * sizeof(WOW_IPV6_SYNC_PATTERN_T) +
	      WMI_TLV_HDR_SIZE +
	      0 * sizeof(WOW_MAGIC_PATTERN_CMD) +
	      WMI_TLV_HDR_SIZE +
	      0 * sizeof(A_UINT32) + WMI_TLV_HDR_SIZE + 1 * sizeof(A_UINT32);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_WOW_ADD_PATTERN_CMD_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_WOW_ADD_PATTERN_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_WOW_ADD_PATTERN_CMD_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->pattern_id = default_pattern,
	cmd->pattern_type = WOW_IPV6_RA_PATTERN;
	buf_ptr += sizeof(WMI_WOW_ADD_PATTERN_CMD_fixed_param);

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_BITMAP_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_IPV4_SYNC_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_IPV6_SYNC_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_MAGIC_PATTERN_CMD but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for pattern_info_timeout but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for ra_ratelimit_interval. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, sizeof(A_UINT32));
	buf_ptr += WMI_TLV_HDR_SIZE;

	*((A_UINT32 *) buf_ptr) = rate_limit_interval;

	WMI_LOGD("%s: send RA rate limit [%d] to fw vdev = %d", __func__,
		 rate_limit_interval, vdev_id);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WOW_ADD_WAKE_PATTERN_CMDID);
	if (ret) {
		WMI_LOGE("%s: Failed to send RA rate limit to fw", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;

}
#endif /* FEATURE_WLAN_RA_FILTERING */

/**
 * send_add_clear_mcbc_filter_cmd_tlv() - set mcast filter command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @multicastAddr: mcast address
 * @clearList: clear list flag
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_add_clear_mcbc_filter_cmd_tlv(wmi_unified_t wmi_handle,
				     uint8_t vdev_id,
				     struct qdf_mac_addr multicast_addr,
				     bool clearList)
{
	WMI_SET_MCASTBCAST_FILTER_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	int err;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGE("Failed to allocate buffer to send set_param cmd");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_SET_MCASTBCAST_FILTER_CMD_fixed_param *) wmi_buf_data(buf);
	qdf_mem_zero(cmd, sizeof(*cmd));

	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_WMI_SET_MCASTBCAST_FILTER_CMD_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (WMI_SET_MCASTBCAST_FILTER_CMD_fixed_param));
	cmd->action =
		(clearList ? WMI_MCAST_FILTER_DELETE : WMI_MCAST_FILTER_SET);
	cmd->vdev_id = vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(multicast_addr.bytes, &cmd->mcastbdcastaddr);

	WMI_LOGD("Action:%d; vdev_id:%d; clearList:%d; MCBC MAC Addr: %pM",
		 cmd->action, vdev_id, clearList, multicast_addr.bytes);

	err = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(*cmd),
				   WMI_SET_MCASTBCAST_FILTER_CMDID);
	if (err) {
		WMI_LOGE("Failed to send set_param cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_gtk_offload_cmd_tlv() - send GTK offload command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @params: GTK offload parameters
 *
 * Return: CDF status
 */
static
QDF_STATUS send_gtk_offload_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id,
					   struct pmo_gtk_req *params,
					   bool enable_offload,
					   uint32_t gtk_offload_opcode)
{
	int len;
	wmi_buf_t buf;
	WMI_GTK_OFFLOAD_CMD_fixed_param *cmd;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	WMI_LOGD("%s Enter", __func__);

	len = sizeof(*cmd);

	/* alloc wmi buffer */
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("wmi_buf_alloc failed for WMI_GTK_OFFLOAD_CMD");
		status = QDF_STATUS_E_NOMEM;
		goto out;
	}

	cmd = (WMI_GTK_OFFLOAD_CMD_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_GTK_OFFLOAD_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_GTK_OFFLOAD_CMD_fixed_param));

	cmd->vdev_id = vdev_id;

	/* Request target to enable GTK offload */
	if (enable_offload == PMO_GTK_OFFLOAD_ENABLE) {
		cmd->flags = gtk_offload_opcode;

		/* Copy the keys and replay counter */
		qdf_mem_copy(cmd->KCK, params->kck, PMO_KCK_LEN);
		qdf_mem_copy(cmd->KEK, params->kek, PMO_KEK_LEN);
		qdf_mem_copy(cmd->replay_counter, &params->replay_counter,
			     GTK_REPLAY_COUNTER_BYTES);
	} else {
		cmd->flags = gtk_offload_opcode;
	}

	WMI_LOGD("VDEVID: %d, GTK_FLAGS: x%x", vdev_id, cmd->flags);

	/* send the wmi command */
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_GTK_OFFLOAD_CMDID)) {
		WMI_LOGE("Failed to send WMI_GTK_OFFLOAD_CMDID");
		wmi_buf_free(buf);
		status = QDF_STATUS_E_FAILURE;
	}

out:
	WMI_LOGD("%s Exit", __func__);
	return status;
}

/**
 * send_process_gtk_offload_getinfo_cmd_tlv() - send GTK offload cmd to fw
 * @wmi_handle: wmi handle
 * @params: GTK offload params
 *
 * Return: CDF status
 */
static QDF_STATUS send_process_gtk_offload_getinfo_cmd_tlv(
			wmi_unified_t wmi_handle,
			uint8_t vdev_id,
			uint64_t offload_req_opcode)
{
	int len;
	wmi_buf_t buf;
	WMI_GTK_OFFLOAD_CMD_fixed_param *cmd;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	len = sizeof(*cmd);

	/* alloc wmi buffer */
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("wmi_buf_alloc failed for WMI_GTK_OFFLOAD_CMD");
		status = QDF_STATUS_E_NOMEM;
		goto out;
	}

	cmd = (WMI_GTK_OFFLOAD_CMD_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_GTK_OFFLOAD_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_GTK_OFFLOAD_CMD_fixed_param));

	/* Request for GTK offload status */
	cmd->flags = offload_req_opcode;
	cmd->vdev_id = vdev_id;

	/* send the wmi command */
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_GTK_OFFLOAD_CMDID)) {
		WMI_LOGE("Failed to send WMI_GTK_OFFLOAD_CMDID for req info");
		wmi_buf_free(buf);
		status = QDF_STATUS_E_FAILURE;
	}

out:
	return status;
}

/**
 * send_action_frame_patterns_cmd_tlv() - send wmi cmd of action filter params
 * @wmi_handle: wmi handler
 * @action_params: pointer to action_params
 *
 * Return: 0 for success, otherwise appropriate error code
 */
static QDF_STATUS send_action_frame_patterns_cmd_tlv(wmi_unified_t wmi_handle,
		struct pmo_action_wakeup_set_params *action_params)
{
	WMI_WOW_SET_ACTION_WAKE_UP_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	int i;
	int32_t err;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMI_LOGE("Failed to allocate buffer to send action filter cmd");
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (WMI_WOW_SET_ACTION_WAKE_UP_CMD_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_wow_set_action_wake_up_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
				WMI_WOW_SET_ACTION_WAKE_UP_CMD_fixed_param));

	cmd->vdev_id = action_params->vdev_id;
	cmd->operation = action_params->operation;

	for (i = 0; i < MAX_SUPPORTED_ACTION_CATEGORY_ELE_LIST; i++)
		cmd->action_category_map[i] =
				action_params->action_category_map[i];

	err = wmi_unified_cmd_send(wmi_handle, buf,
			sizeof(*cmd), WMI_WOW_SET_ACTION_WAKE_UP_CMDID);
	if (err) {
		WMI_LOGE("Failed to send ap_ps_egap cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_LPHB

/**
 * send_lphb_config_hbenable_cmd_tlv() - enable command of LPHB configuration
 * @wmi_handle: wmi handle
 * @lphb_conf_req: configuration info
 *
 * Return: CDF status
 */
static QDF_STATUS send_lphb_config_hbenable_cmd_tlv(wmi_unified_t wmi_handle,
				wmi_hb_set_enable_cmd_fixed_param *params)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_hb_set_enable_cmd_fixed_param *hb_enable_fp;
	int len = sizeof(wmi_hb_set_enable_cmd_fixed_param);


	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	hb_enable_fp = (wmi_hb_set_enable_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&hb_enable_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_hb_set_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_hb_set_enable_cmd_fixed_param));

	/* fill in values */
	hb_enable_fp->vdev_id = params->session;
	hb_enable_fp->enable = params->enable;
	hb_enable_fp->item = params->item;
	hb_enable_fp->session = params->session;

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_HB_SET_ENABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("cmd_send WMI_HB_SET_ENABLE returned Error %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_lphb_config_tcp_params_cmd_tlv() - set tcp params of LPHB configuration
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: CDF status
 */
static QDF_STATUS send_lphb_config_tcp_params_cmd_tlv(wmi_unified_t wmi_handle,
	    wmi_hb_set_tcp_params_cmd_fixed_param *lphb_conf_req)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_hb_set_tcp_params_cmd_fixed_param *hb_tcp_params_fp;
	int len = sizeof(wmi_hb_set_tcp_params_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	hb_tcp_params_fp = (wmi_hb_set_tcp_params_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&hb_tcp_params_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_hb_set_tcp_params_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_hb_set_tcp_params_cmd_fixed_param));

	/* fill in values */
	hb_tcp_params_fp->vdev_id = lphb_conf_req->vdev_id;
	hb_tcp_params_fp->srv_ip = lphb_conf_req->srv_ip;
	hb_tcp_params_fp->dev_ip = lphb_conf_req->dev_ip;
	hb_tcp_params_fp->seq = lphb_conf_req->seq;
	hb_tcp_params_fp->src_port = lphb_conf_req->src_port;
	hb_tcp_params_fp->dst_port = lphb_conf_req->dst_port;
	hb_tcp_params_fp->interval = lphb_conf_req->interval;
	hb_tcp_params_fp->timeout = lphb_conf_req->timeout;
	hb_tcp_params_fp->session = lphb_conf_req->session;
	qdf_mem_copy(&hb_tcp_params_fp->gateway_mac,
				   &lphb_conf_req->gateway_mac,
				   sizeof(hb_tcp_params_fp->gateway_mac));

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_HB_SET_TCP_PARAMS_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("cmd_send WMI_HB_SET_TCP_PARAMS returned Error %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_lphb_config_tcp_pkt_filter_cmd_tlv() - configure tcp packet filter cmd
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: CDF status
 */
static
QDF_STATUS send_lphb_config_tcp_pkt_filter_cmd_tlv(wmi_unified_t wmi_handle,
		wmi_hb_set_tcp_pkt_filter_cmd_fixed_param *g_hb_tcp_filter_fp)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_hb_set_tcp_pkt_filter_cmd_fixed_param *hb_tcp_filter_fp;
	int len = sizeof(wmi_hb_set_tcp_pkt_filter_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	hb_tcp_filter_fp =
		(wmi_hb_set_tcp_pkt_filter_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&hb_tcp_filter_fp->tlv_header,
		WMITLV_TAG_STRUC_wmi_hb_set_tcp_pkt_filter_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		       (wmi_hb_set_tcp_pkt_filter_cmd_fixed_param));

	/* fill in values */
	hb_tcp_filter_fp->vdev_id = g_hb_tcp_filter_fp->vdev_id;
	hb_tcp_filter_fp->length = g_hb_tcp_filter_fp->length;
	hb_tcp_filter_fp->offset = g_hb_tcp_filter_fp->offset;
	hb_tcp_filter_fp->session = g_hb_tcp_filter_fp->session;
	memcpy((void *)&hb_tcp_filter_fp->filter,
	       (void *)&g_hb_tcp_filter_fp->filter,
	       WMI_WLAN_HB_MAX_FILTER_SIZE);

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_HB_SET_TCP_PKT_FILTER_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("cmd_send WMI_HB_SET_TCP_PKT_FILTER returned Error %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_lphb_config_udp_params_cmd_tlv() - configure udp param command of LPHB
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: CDF status
 */
static QDF_STATUS send_lphb_config_udp_params_cmd_tlv(wmi_unified_t wmi_handle,
		   wmi_hb_set_udp_params_cmd_fixed_param *lphb_conf_req)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_hb_set_udp_params_cmd_fixed_param *hb_udp_params_fp;
	int len = sizeof(wmi_hb_set_udp_params_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	hb_udp_params_fp = (wmi_hb_set_udp_params_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&hb_udp_params_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_hb_set_udp_params_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_hb_set_udp_params_cmd_fixed_param));

	/* fill in values */
	hb_udp_params_fp->vdev_id = lphb_conf_req->vdev_id;
	hb_udp_params_fp->srv_ip = lphb_conf_req->srv_ip;
	hb_udp_params_fp->dev_ip = lphb_conf_req->dev_ip;
	hb_udp_params_fp->src_port = lphb_conf_req->src_port;
	hb_udp_params_fp->dst_port = lphb_conf_req->dst_port;
	hb_udp_params_fp->interval = lphb_conf_req->interval;
	hb_udp_params_fp->timeout = lphb_conf_req->timeout;
	hb_udp_params_fp->session = lphb_conf_req->session;
	qdf_mem_copy(&hb_udp_params_fp->gateway_mac,
				   &lphb_conf_req->gateway_mac,
				   sizeof(lphb_conf_req->gateway_mac));

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_HB_SET_UDP_PARAMS_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("cmd_send WMI_HB_SET_UDP_PARAMS returned Error %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_lphb_config_udp_pkt_filter_cmd_tlv() - configure udp pkt filter command
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: CDF status
 */
static
QDF_STATUS send_lphb_config_udp_pkt_filter_cmd_tlv(wmi_unified_t wmi_handle,
		wmi_hb_set_udp_pkt_filter_cmd_fixed_param *lphb_conf_req)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_hb_set_udp_pkt_filter_cmd_fixed_param *hb_udp_filter_fp;
	int len = sizeof(wmi_hb_set_udp_pkt_filter_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	hb_udp_filter_fp =
		(wmi_hb_set_udp_pkt_filter_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&hb_udp_filter_fp->tlv_header,
		WMITLV_TAG_STRUC_wmi_hb_set_udp_pkt_filter_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		       (wmi_hb_set_udp_pkt_filter_cmd_fixed_param));

	/* fill in values */
	hb_udp_filter_fp->vdev_id = lphb_conf_req->vdev_id;
	hb_udp_filter_fp->length = lphb_conf_req->length;
	hb_udp_filter_fp->offset = lphb_conf_req->offset;
	hb_udp_filter_fp->session = lphb_conf_req->session;
	memcpy((void *)&hb_udp_filter_fp->filter,
	       (void *)&lphb_conf_req->filter,
	       WMI_WLAN_HB_MAX_FILTER_SIZE);

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_HB_SET_UDP_PKT_FILTER_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("cmd_send WMI_HB_SET_UDP_PKT_FILTER returned Error %d",
			status);
		wmi_buf_free(buf);
	}

	return status;
}
#endif /* FEATURE_WLAN_LPHB */

static QDF_STATUS send_conf_hw_filter_cmd_tlv(wmi_unified_t wmi,
					      struct pmo_hw_filter_params *req)
{
	QDF_STATUS status;
	wmi_hw_data_filter_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;

	if (!req) {
		WMI_LOGE("req is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_buf = wmi_buf_alloc(wmi, sizeof(*cmd));
	if (!wmi_buf) {
		WMI_LOGE(FL("Out of memory"));
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_hw_data_filter_cmd_fixed_param *)wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		  WMITLV_TAG_STRUC_wmi_hw_data_filter_cmd_fixed_param,
		  WMITLV_GET_STRUCT_TLVLEN(wmi_hw_data_filter_cmd_fixed_param));
	cmd->vdev_id = req->vdev_id;
	cmd->enable = req->mode != PMO_HW_FILTER_DISABLED;
	cmd->hw_filter_bitmap = req->mode;

	WMI_LOGD("configure hw filter (vdev_id: %d, mode: %d)",
		 req->vdev_id, req->mode);

	status = wmi_unified_cmd_send(wmi, wmi_buf, sizeof(*cmd),
				      WMI_HW_DATA_FILTER_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("Failed to configure hw filter");
		wmi_buf_free(wmi_buf);
	}

	return status;
}

/**
 * send_enable_disable_packet_filter_cmd_tlv() - enable/disable packet filter
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @enable: Flag to enable/disable packet filter
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_enable_disable_packet_filter_cmd_tlv(
		wmi_unified_t wmi_handle, uint8_t vdev_id, bool enable)
{
	int32_t len;
	int ret = 0;
	wmi_buf_t buf;
	WMI_PACKET_FILTER_ENABLE_CMD_fixed_param *cmd;

	len = sizeof(WMI_PACKET_FILTER_ENABLE_CMD_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s: Failed allocate wmi buffer", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_PACKET_FILTER_ENABLE_CMD_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_packet_filter_enable_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		WMI_PACKET_FILTER_ENABLE_CMD_fixed_param));

	cmd->vdev_id = vdev_id;
	if (enable)
		cmd->enable = PACKET_FILTER_SET_ENABLE;
	else
		cmd->enable = PACKET_FILTER_SET_DISABLE;

	WMI_LOGE("%s: Packet filter enable %d for vdev_id %d",
		__func__, cmd->enable, vdev_id);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			 WMI_PACKET_FILTER_ENABLE_CMDID);
	if (ret) {
		WMI_LOGE("Failed to send packet filter wmi cmd to fw");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_config_packet_filter_cmd_tlv() - configure packet filter in target
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @rcv_filter_param: Packet filter parameters
 * @filter_id: Filter id
 * @enable: Flag to add/delete packet filter configuration
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_config_packet_filter_cmd_tlv(wmi_unified_t wmi_handle,
		uint8_t vdev_id, struct pmo_rcv_pkt_fltr_cfg *rcv_filter_param,
		uint8_t filter_id, bool enable)
{
	int len, i;
	int err = 0;
	wmi_buf_t buf;
	WMI_PACKET_FILTER_CONFIG_CMD_fixed_param *cmd;


	/* allocate the memory */
	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("Failed to allocate buffer to send set_param cmd");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_PACKET_FILTER_CONFIG_CMD_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_packet_filter_config_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			       (WMI_PACKET_FILTER_CONFIG_CMD_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->filter_id = filter_id;
	if (enable)
		cmd->filter_action = PACKET_FILTER_SET_ACTIVE;
	else
		cmd->filter_action = PACKET_FILTER_SET_INACTIVE;

	if (enable) {
		cmd->num_params = QDF_MIN(
			WMI_PACKET_FILTER_MAX_CMP_PER_PACKET_FILTER,
			rcv_filter_param->num_params);
		cmd->filter_type = rcv_filter_param->filter_type;
		cmd->coalesce_time = rcv_filter_param->coalesce_time;

		for (i = 0; i < cmd->num_params; i++) {
			cmd->paramsData[i].proto_type =
				rcv_filter_param->params_data[i].protocol_layer;
			cmd->paramsData[i].cmp_type =
				rcv_filter_param->params_data[i].compare_flag;
			cmd->paramsData[i].data_length =
				rcv_filter_param->params_data[i].data_length;
			cmd->paramsData[i].data_offset =
				rcv_filter_param->params_data[i].data_offset;
			memcpy(&cmd->paramsData[i].compareData,
				rcv_filter_param->params_data[i].compare_data,
				sizeof(cmd->paramsData[i].compareData));
			memcpy(&cmd->paramsData[i].dataMask,
				rcv_filter_param->params_data[i].data_mask,
				sizeof(cmd->paramsData[i].dataMask));
		}
	}

	WMI_LOGE("Packet filter action %d filter with id: %d, num_params=%d",
		cmd->filter_action, cmd->filter_id, cmd->num_params);
	/* send the command along with data */
	err = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_PACKET_FILTER_CONFIG_CMDID);
	if (err) {
		WMI_LOGE("Failed to send pkt_filter cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* End of WLAN_PMO_ENABLE */

/**
 * send_set_ssid_hotlist_cmd_tlv() - Handle an SSID hotlist set request
 * @wmi_handle: wmi handle
 * @request: SSID hotlist set request
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS
send_set_ssid_hotlist_cmd_tlv(wmi_unified_t wmi_handle,
		     struct ssid_hotlist_request_params *request)
{
	wmi_extscan_configure_hotlist_ssid_monitor_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint32_t array_size;
	uint8_t *buf_ptr;

	/* length of fixed portion */
	len = sizeof(*cmd);

	/* length of variable portion */
	array_size =
		request->ssid_count * sizeof(wmi_extscan_hotlist_ssid_entry);
	len += WMI_TLV_HDR_SIZE + array_size;

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_extscan_configure_hotlist_ssid_monitor_cmd_fixed_param *)
						buf_ptr;
	WMITLV_SET_HDR
		(&cmd->tlv_header,
		 WMITLV_TAG_STRUC_wmi_extscan_configure_hotlist_ssid_monitor_cmd_fixed_param,
		 WMITLV_GET_STRUCT_TLVLEN
			(wmi_extscan_configure_hotlist_ssid_monitor_cmd_fixed_param));

	cmd->request_id = request->request_id;
	cmd->requestor_id = 0;
	cmd->vdev_id = request->session_id;
	cmd->table_id = 0;
	cmd->lost_ap_scan_count = request->lost_ssid_sample_size;
	cmd->total_entries = request->ssid_count;
	cmd->num_entries_in_page = request->ssid_count;
	cmd->first_entry_index = 0;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, array_size);

	if (request->ssid_count) {
		wmi_extscan_hotlist_ssid_entry *entry;
		int i;

		buf_ptr += WMI_TLV_HDR_SIZE;
		entry = (wmi_extscan_hotlist_ssid_entry *)buf_ptr;
		for (i = 0; i < request->ssid_count; i++) {
			WMITLV_SET_HDR
				(entry,
				 WMITLV_TAG_ARRAY_STRUC,
				 WMITLV_GET_STRUCT_TLVLEN
					(wmi_extscan_hotlist_ssid_entry));
			entry->ssid.ssid_len = request->ssids[i].ssid.length;
			qdf_mem_copy(entry->ssid.ssid,
				     request->ssids[i].ssid.mac_ssid,
				     request->ssids[i].ssid.length);
			entry->band = request->ssids[i].band;
			entry->min_rssi = request->ssids[i].rssi_low;
			entry->max_rssi = request->ssids[i].rssi_high;
			entry++;
		}
		cmd->mode = WMI_EXTSCAN_MODE_START;
	} else {
		cmd->mode = WMI_EXTSCAN_MODE_STOP;
	}

	if (wmi_unified_cmd_send
		(wmi_handle, wmi_buf, len,
		 WMI_EXTSCAN_CONFIGURE_HOTLIST_SSID_MONITOR_CMDID)) {
		WMI_LOGE("%s: failed to send command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_process_roam_synch_complete_cmd_tlv() - roam synch complete command to fw.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * This function sends roam synch complete event to fw.
 *
 * Return: CDF STATUS
 */
static QDF_STATUS send_process_roam_synch_complete_cmd_tlv(wmi_unified_t wmi_handle,
		 uint8_t vdev_id)
{
	wmi_roam_synch_complete_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint8_t *buf_ptr;
	uint16_t len;
	len = sizeof(wmi_roam_synch_complete_fixed_param);

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_roam_synch_complete_fixed_param *) wmi_buf_data(wmi_buf);
	buf_ptr = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_roam_synch_complete_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_roam_synch_complete_fixed_param));
	cmd->vdev_id = vdev_id;
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_ROAM_SYNCH_COMPLETE)) {
		WMI_LOGP("%s: failed to send roam synch confirmation",
			 __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_fw_test_cmd_tlv() - send fw test command to fw.
 * @wmi_handle: wmi handle
 * @wmi_fwtest: fw test command
 *
 * This function sends fw test command to fw.
 *
 * Return: CDF STATUS
 */
static
QDF_STATUS send_fw_test_cmd_tlv(wmi_unified_t wmi_handle,
			       struct set_fwtest_params *wmi_fwtest)
{
	wmi_fwtest_set_param_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint16_t len;

	len = sizeof(*cmd);

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmai_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_fwtest_set_param_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_fwtest_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
		       wmi_fwtest_set_param_cmd_fixed_param));
	cmd->param_id = wmi_fwtest->arg;
	cmd->param_value = wmi_fwtest->value;

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_FWTEST_CMDID)) {
		WMI_LOGP("%s: failed to send fw test command", __func__);
		qdf_nbuf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_unit_test_cmd_tlv() - send unit test command to fw.
 * @wmi_handle: wmi handle
 * @wmi_utest: unit test command
 *
 * This function send unit test command to fw.
 *
 * Return: CDF STATUS
 */
static QDF_STATUS send_unit_test_cmd_tlv(wmi_unified_t wmi_handle,
			       struct wmi_unit_test_cmd *wmi_utest)
{
	wmi_unit_test_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint8_t *buf_ptr;
	int i;
	uint16_t len, args_tlv_len;
	A_UINT32 *unit_test_cmd_args;

	args_tlv_len =
		WMI_TLV_HDR_SIZE + wmi_utest->num_args * sizeof(A_UINT32);
	len = sizeof(wmi_unit_test_cmd_fixed_param) + args_tlv_len;

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmai_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_unit_test_cmd_fixed_param *) wmi_buf_data(wmi_buf);
	buf_ptr = (uint8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_unit_test_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_unit_test_cmd_fixed_param));
	cmd->vdev_id = wmi_utest->vdev_id;
	cmd->module_id = wmi_utest->module_id;
	cmd->num_args = wmi_utest->num_args;
	buf_ptr += sizeof(wmi_unit_test_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (wmi_utest->num_args * sizeof(uint32_t)));
	unit_test_cmd_args = (A_UINT32 *) (buf_ptr + WMI_TLV_HDR_SIZE);
	WMI_LOGI("%s: %d num of args = ", __func__, wmi_utest->num_args);
	for (i = 0; (i < wmi_utest->num_args && i < WMI_MAX_NUM_ARGS); i++) {
		unit_test_cmd_args[i] = wmi_utest->args[i];
		WMI_LOGI("%d,", wmi_utest->args[i]);
	}
	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_UNIT_TEST_CMDID)) {
		WMI_LOGP("%s: failed to send unit test command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_roam_invoke_cmd_tlv() - send roam invoke command to fw.
 * @wmi_handle: wma handle
 * @roaminvoke: roam invoke command
 *
 * Send roam invoke command to fw for fastreassoc.
 *
 * Return: CDF STATUS
 */
static QDF_STATUS send_roam_invoke_cmd_tlv(wmi_unified_t wmi_handle,
		struct wmi_roam_invoke_cmd *roaminvoke,
		uint32_t ch_hz)
{
	wmi_roam_invoke_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	u_int8_t *buf_ptr;
	u_int16_t len, args_tlv_len;
	A_UINT32 *channel_list;
	wmi_mac_addr *bssid_list;
	wmi_tlv_buf_len_param *buf_len_tlv;

	/* Host sends only one channel and one bssid */
	args_tlv_len = (4 * WMI_TLV_HDR_SIZE) + sizeof(A_UINT32) +
			sizeof(wmi_mac_addr) + sizeof(wmi_tlv_buf_len_param) +
			roundup(roaminvoke->frame_len, sizeof(uint32_t));
	len = sizeof(wmi_roam_invoke_cmd_fixed_param) + args_tlv_len;
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		WMI_LOGE("%s: wmai_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_roam_invoke_cmd_fixed_param *)wmi_buf_data(wmi_buf);
	buf_ptr = (u_int8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_roam_invoke_cmd_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN(wmi_roam_invoke_cmd_fixed_param));
	cmd->vdev_id = roaminvoke->vdev_id;
	cmd->flags = 0;

	if (roaminvoke->frame_len)
		cmd->roam_scan_mode = WMI_ROAM_INVOKE_SCAN_MODE_SKIP;
	else
		cmd->roam_scan_mode = WMI_ROAM_INVOKE_SCAN_MODE_FIXED_CH;

	cmd->roam_ap_sel_mode = 0;
	cmd->roam_delay = 0;
	cmd->num_chan = 1;
	cmd->num_bssid = 1;
	/* packing 1 beacon/probe_rsp frame with WMI cmd */
	cmd->num_buf = 1;

	buf_ptr += sizeof(wmi_roam_invoke_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
				(sizeof(u_int32_t)));
	channel_list = (A_UINT32 *)(buf_ptr + WMI_TLV_HDR_SIZE);
	*channel_list = ch_hz;
	buf_ptr += sizeof(A_UINT32) + WMI_TLV_HDR_SIZE;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
				(sizeof(wmi_mac_addr)));
	bssid_list = (wmi_mac_addr *)(buf_ptr + WMI_TLV_HDR_SIZE);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(roaminvoke->bssid, bssid_list);

	/* move to next tlv i.e. bcn_prb_buf_list */
	buf_ptr += WMI_TLV_HDR_SIZE + sizeof(wmi_mac_addr);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
			sizeof(wmi_tlv_buf_len_param));

	buf_len_tlv = (wmi_tlv_buf_len_param *)(buf_ptr + WMI_TLV_HDR_SIZE);
	buf_len_tlv->buf_len = roaminvoke->frame_len;

	/* move to next tlv i.e. bcn_prb_frm */
	buf_ptr += WMI_TLV_HDR_SIZE + sizeof(wmi_tlv_buf_len_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
		roundup(roaminvoke->frame_len, sizeof(uint32_t)));

	/* copy frame after the header */
	qdf_mem_copy(buf_ptr + WMI_TLV_HDR_SIZE,
			roaminvoke->frame_buf,
			roaminvoke->frame_len);

	WMI_LOGD(FL("bcn/prb_rsp frame, length: %d"), roaminvoke->frame_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG,
			buf_ptr + WMI_TLV_HDR_SIZE,
			roaminvoke->frame_len);

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
					WMI_ROAM_INVOKE_CMDID)) {
		WMI_LOGP("%s: failed to send roam invoke command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_roam_scan_offload_cmd_tlv() - set roam offload command
 * @wmi_handle: wmi handle
 * @command: command
 * @vdev_id: vdev id
 *
 * This function set roam offload command to fw.
 *
 * Return: CDF status
 */
static QDF_STATUS send_roam_scan_offload_cmd_tlv(wmi_unified_t wmi_handle,
					 uint32_t command, uint32_t vdev_id)
{
	QDF_STATUS status;
	wmi_roam_scan_cmd_fixed_param *cmd_fp;
	wmi_buf_t buf = NULL;
	int len;
	uint8_t *buf_ptr;

	len = sizeof(wmi_roam_scan_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);

	cmd_fp = (wmi_roam_scan_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_roam_scan_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_roam_scan_cmd_fixed_param));
	cmd_fp->vdev_id = vdev_id;
	cmd_fp->command_arg = command;

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_ROAM_SCAN_CMD);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("wmi_unified_cmd_send WMI_ROAM_SCAN_CMD returned Error %d",
			status);
		goto error;
	}

	WMI_LOGI("%s: WMI --> WMI_ROAM_SCAN_CMD", __func__);
	return QDF_STATUS_SUCCESS;

error:
	wmi_buf_free(buf);

	return status;
}

/**
 * send_roam_scan_offload_ap_profile_cmd_tlv() - set roam ap profile in fw
 * @wmi_handle: wmi handle
 * @ap_profile_p: ap profile
 * @vdev_id: vdev id
 *
 * Send WMI_ROAM_AP_PROFILE to firmware
 *
 * Return: CDF status
 */
static QDF_STATUS send_roam_scan_offload_ap_profile_cmd_tlv(wmi_unified_t wmi_handle,
					    wmi_ap_profile *ap_profile_p,
					    uint32_t vdev_id)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	int len;
	uint8_t *buf_ptr;
	wmi_roam_ap_profile_fixed_param *roam_ap_profile_fp;

	len = sizeof(wmi_roam_ap_profile_fixed_param) + sizeof(wmi_ap_profile);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	roam_ap_profile_fp = (wmi_roam_ap_profile_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&roam_ap_profile_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_roam_ap_profile_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_roam_ap_profile_fixed_param));
	/* fill in threshold values */
	roam_ap_profile_fp->vdev_id = vdev_id;
	roam_ap_profile_fp->id = 0;
	buf_ptr += sizeof(wmi_roam_ap_profile_fixed_param);

	qdf_mem_copy(buf_ptr, ap_profile_p, sizeof(wmi_ap_profile));
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_STRUC_wmi_ap_profile,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_ap_profile));
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_ROAM_AP_PROFILE);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("wmi_unified_cmd_send WMI_ROAM_AP_PROFILE returned Error %d",
			status);
		wmi_buf_free(buf);
	}

	WMI_LOGI("WMI --> WMI_ROAM_AP_PROFILE and other parameters");

	return status;
}

/**
 * send_roam_scan_offload_scan_period_cmd_tlv() - set roam offload scan period
 * @wmi_handle: wmi handle
 * @scan_period: scan period
 * @scan_age: scan age
 * @vdev_id: vdev id
 *
 * Send WMI_ROAM_SCAN_PERIOD parameters to fw.
 *
 * Return: CDF status
 */
static QDF_STATUS send_roam_scan_offload_scan_period_cmd_tlv(wmi_unified_t wmi_handle,
					     uint32_t scan_period,
					     uint32_t scan_age,
					     uint32_t vdev_id)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	int len;
	uint8_t *buf_ptr;
	wmi_roam_scan_period_fixed_param *scan_period_fp;

	/* Send scan period values */
	len = sizeof(wmi_roam_scan_period_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	scan_period_fp = (wmi_roam_scan_period_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&scan_period_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_roam_scan_period_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_roam_scan_period_fixed_param));
	/* fill in scan period values */
	scan_period_fp->vdev_id = vdev_id;
	scan_period_fp->roam_scan_period = scan_period; /* 20 seconds */
	scan_period_fp->roam_scan_age = scan_age;

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_ROAM_SCAN_PERIOD);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("wmi_unified_cmd_send WMI_ROAM_SCAN_PERIOD returned Error %d",
			status);
		goto error;
	}

	WMI_LOGI("%s: WMI --> WMI_ROAM_SCAN_PERIOD roam_scan_period=%d, roam_scan_age=%d",
		__func__, scan_period, scan_age);
	return QDF_STATUS_SUCCESS;
error:
	wmi_buf_free(buf);

	return status;
}

/**
 * send_roam_scan_offload_chan_list_cmd_tlv() - set roam offload channel list
 * @wmi_handle: wmi handle
 * @chan_count: channel count
 * @chan_list: channel list
 * @list_type: list type
 * @vdev_id: vdev id
 *
 * Set roam offload channel list.
 *
 * Return: CDF status
 */
static QDF_STATUS send_roam_scan_offload_chan_list_cmd_tlv(wmi_unified_t wmi_handle,
				   uint8_t chan_count,
				   uint32_t *chan_list,
				   uint8_t list_type, uint32_t vdev_id)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	int len, list_tlv_len;
	int i;
	uint8_t *buf_ptr;
	wmi_roam_chan_list_fixed_param *chan_list_fp;
	A_UINT32 *roam_chan_list_array;

	if (chan_count == 0) {
		WMI_LOGD("%s : invalid number of channels %d", __func__,
			 chan_count);
		return QDF_STATUS_E_EMPTY;
	}
	/* Channel list is a table of 2 TLV's */
	list_tlv_len = WMI_TLV_HDR_SIZE + chan_count * sizeof(A_UINT32);
	len = sizeof(wmi_roam_chan_list_fixed_param) + list_tlv_len;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	chan_list_fp = (wmi_roam_chan_list_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&chan_list_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_roam_chan_list_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_roam_chan_list_fixed_param));
	chan_list_fp->vdev_id = vdev_id;
	chan_list_fp->num_chan = chan_count;
	if (chan_count > 0 && list_type == WMI_CHANNEL_LIST_STATIC) {
		/* external app is controlling channel list */
		chan_list_fp->chan_list_type =
			WMI_ROAM_SCAN_CHAN_LIST_TYPE_STATIC;
	} else {
		/* umac supplied occupied channel list in LFR */
		chan_list_fp->chan_list_type =
			WMI_ROAM_SCAN_CHAN_LIST_TYPE_DYNAMIC;
	}

	buf_ptr += sizeof(wmi_roam_chan_list_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (chan_list_fp->num_chan * sizeof(uint32_t)));
	roam_chan_list_array = (A_UINT32 *) (buf_ptr + WMI_TLV_HDR_SIZE);
	WMI_LOGI("%s: %d channels = ", __func__, chan_list_fp->num_chan);
	for (i = 0; ((i < chan_list_fp->num_chan) &&
		     (i < WMI_ROAM_MAX_CHANNELS)); i++) {
		roam_chan_list_array[i] = chan_list[i];
		WMI_LOGI("%d,", roam_chan_list_array[i]);
	}

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_ROAM_CHAN_LIST);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("wmi_unified_cmd_send WMI_ROAM_CHAN_LIST returned Error %d",
			status);
		goto error;
	}

	WMI_LOGI("%s: WMI --> WMI_ROAM_SCAN_CHAN_LIST", __func__);
	return QDF_STATUS_SUCCESS;
error:
	wmi_buf_free(buf);

	return status;
}

/**
 * send_per_roam_config_cmd_tlv() - set per roaming config to FW
 * @wmi_handle: wmi handle
 * @req_buf: per roam config buffer
 *
 * Return: QDF status
 */
static QDF_STATUS send_per_roam_config_cmd_tlv(wmi_unified_t wmi_handle,
		struct wmi_per_roam_config_req *req_buf)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	int len;
	uint8_t *buf_ptr;
	wmi_roam_per_config_fixed_param *wmi_per_config;

	len = sizeof(wmi_roam_per_config_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	wmi_per_config =
		(wmi_roam_per_config_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&wmi_per_config->tlv_header,
			WMITLV_TAG_STRUC_wmi_roam_per_config_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN
			(wmi_roam_per_config_fixed_param));

	/* fill in per roam config values */
	wmi_per_config->vdev_id = req_buf->vdev_id;

	wmi_per_config->enable = req_buf->per_config.enable;
	wmi_per_config->high_rate_thresh =
		(req_buf->per_config.tx_high_rate_thresh << 16) |
		(req_buf->per_config.rx_high_rate_thresh & 0x0000ffff);
	wmi_per_config->low_rate_thresh =
		(req_buf->per_config.tx_low_rate_thresh << 16) |
		(req_buf->per_config.rx_low_rate_thresh & 0x0000ffff);
	wmi_per_config->pkt_err_rate_thresh_pct =
		(req_buf->per_config.tx_rate_thresh_percnt << 16) |
		(req_buf->per_config.rx_rate_thresh_percnt & 0x0000ffff);
	wmi_per_config->per_rest_time = req_buf->per_config.per_rest_time;
	wmi_per_config->pkt_err_rate_mon_time =
			(req_buf->per_config.tx_per_mon_time << 16) |
			(req_buf->per_config.rx_per_mon_time & 0x0000ffff);

	/* Send per roam config parameters */
	status = wmi_unified_cmd_send(wmi_handle, buf,
			len, WMI_ROAM_PER_CONFIG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("WMI_ROAM_PER_CONFIG_CMDID failed, Error %d",
				status);
		wmi_buf_free(buf);
		return status;
	}

	WMI_LOGI(FL("per roam enable=%d, vdev=%d"),
			req_buf->per_config.enable, req_buf->vdev_id);
	return QDF_STATUS_SUCCESS;
}

/**
 * send_roam_scan_offload_rssi_change_cmd_tlv() - set roam offload RSSI th
 * @wmi_handle: wmi handle
 * @rssi_change_thresh: RSSI Change threshold
 * @bcn_rssi_weight: beacon RSSI weight
 * @vdev_id: vdev id
 *
 * Send WMI_ROAM_SCAN_RSSI_CHANGE_THRESHOLD parameters to fw.
 *
 * Return: CDF status
 */
static QDF_STATUS send_roam_scan_offload_rssi_change_cmd_tlv(wmi_unified_t wmi_handle,
	uint32_t vdev_id,
	int32_t rssi_change_thresh,
	uint32_t bcn_rssi_weight,
	uint32_t hirssi_delay_btw_scans)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	int len;
	uint8_t *buf_ptr;
	wmi_roam_scan_rssi_change_threshold_fixed_param *rssi_change_fp;

	/* Send rssi change parameters */
	len = sizeof(wmi_roam_scan_rssi_change_threshold_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	rssi_change_fp =
		(wmi_roam_scan_rssi_change_threshold_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&rssi_change_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_roam_scan_rssi_change_threshold_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_roam_scan_rssi_change_threshold_fixed_param));
	/* fill in rssi change threshold (hysteresis) values */
	rssi_change_fp->vdev_id = vdev_id;
	rssi_change_fp->roam_scan_rssi_change_thresh = rssi_change_thresh;
	rssi_change_fp->bcn_rssi_weight = bcn_rssi_weight;
	rssi_change_fp->hirssi_delay_btw_scans = hirssi_delay_btw_scans;

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_ROAM_SCAN_RSSI_CHANGE_THRESHOLD);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("wmi_unified_cmd_send WMI_ROAM_SCAN_RSSI_CHANGE_THRESHOLD returned Error %d",
			status);
		goto error;
	}

	WMI_LOGI(FL("roam_scan_rssi_change_thresh=%d, bcn_rssi_weight=%d"),
		rssi_change_thresh, bcn_rssi_weight);
	WMI_LOGI(FL("hirssi_delay_btw_scans=%d"), hirssi_delay_btw_scans);
	return QDF_STATUS_SUCCESS;
error:
	wmi_buf_free(buf);

	return status;
}

/** wmi_get_hotlist_entries_per_page() - hotlist entries per page
 * @wmi_handle: wmi handle.
 * @cmd: size of command structure.
 * @per_entry_size: per entry size.
 *
 * This utility function calculates how many hotlist entries can
 * fit in one page.
 *
 * Return: number of entries
 */
static inline int wmi_get_hotlist_entries_per_page(wmi_unified_t wmi_handle,
						   size_t cmd_size,
						   size_t per_entry_size)
{
	uint32_t avail_space = 0;
	int num_entries = 0;
	uint16_t max_msg_len = wmi_get_max_msg_len(wmi_handle);

	/* Calculate number of hotlist entries that can
	 * be passed in wma message request.
	 */
	avail_space = max_msg_len - cmd_size;
	num_entries = avail_space / per_entry_size;
	return num_entries;
}

/**
 * send_get_buf_extscan_hotlist_cmd_tlv() - prepare hotlist command
 * @wmi_handle: wmi handle
 * @photlist: hotlist command params
 * @buf_len: buffer length
 *
 * This function fills individual elements for  hotlist request and
 * TLV for bssid entries
 *
 * Return: CDF Status.
 */
static QDF_STATUS send_get_buf_extscan_hotlist_cmd_tlv(wmi_unified_t wmi_handle,
					   struct ext_scan_setbssi_hotlist_params *
					   photlist, int *buf_len)
{
	wmi_extscan_configure_hotlist_monitor_cmd_fixed_param *cmd = NULL;
	wmi_extscan_hotlist_entry *dest_hotlist;
	struct ap_threshold_params *src_ap = photlist->ap;
	wmi_buf_t buf;
	uint8_t *buf_ptr;

	int j, index = 0;
	int cmd_len = 0;
	int num_entries;
	int min_entries = 0;
	uint32_t numap = photlist->numAp;
	int len = sizeof(*cmd);

	len += WMI_TLV_HDR_SIZE;
	cmd_len = len;

	num_entries = wmi_get_hotlist_entries_per_page(wmi_handle,
							cmd_len,
							sizeof(*dest_hotlist));
	/* setbssid hotlist expects the bssid list
	 * to be non zero value
	 */
	if (!numap || (numap > WMI_WLAN_EXTSCAN_MAX_HOTLIST_APS)) {
		WMI_LOGE("Invalid number of APs: %d", numap);
		return QDF_STATUS_E_INVAL;
	}

	/* Split the hot list entry pages and send multiple command
	 * requests if the buffer reaches the maximum request size
	 */
	while (index < numap) {
		min_entries = QDF_MIN(num_entries, numap);
		len += min_entries * sizeof(wmi_extscan_hotlist_entry);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			WMI_LOGP("%s: wmi_buf_alloc failed", __func__);
			return QDF_STATUS_E_FAILURE;
		}
		buf_ptr = (uint8_t *) wmi_buf_data(buf);
		cmd = (wmi_extscan_configure_hotlist_monitor_cmd_fixed_param *)
		      buf_ptr;
		WMITLV_SET_HDR(&cmd->tlv_header,
			       WMITLV_TAG_STRUC_wmi_extscan_configure_hotlist_monitor_cmd_fixed_param,
			       WMITLV_GET_STRUCT_TLVLEN
				       (wmi_extscan_configure_hotlist_monitor_cmd_fixed_param));

		/* Multiple requests are sent until the num_entries_in_page
		 * matches the total_entries
		 */
		cmd->request_id = photlist->requestId;
		cmd->vdev_id = photlist->sessionId;
		cmd->total_entries = numap;
		cmd->mode = 1;
		cmd->num_entries_in_page = min_entries;
		cmd->lost_ap_scan_count = photlist->lost_ap_sample_size;
		cmd->first_entry_index = index;

		WMI_LOGD("%s: vdev id:%d total_entries: %d num_entries: %d lost_ap_sample_size: %d",
			__func__, cmd->vdev_id, cmd->total_entries,
			cmd->num_entries_in_page,
			cmd->lost_ap_scan_count);

		buf_ptr += sizeof(*cmd);
		WMITLV_SET_HDR(buf_ptr,
			       WMITLV_TAG_ARRAY_STRUC,
			       min_entries * sizeof(wmi_extscan_hotlist_entry));
		dest_hotlist = (wmi_extscan_hotlist_entry *)
			       (buf_ptr + WMI_TLV_HDR_SIZE);

		/* Populate bssid, channel info and rssi
		 * for the bssid's that are sent as hotlists.
		 */
		for (j = 0; j < min_entries; j++) {
			WMITLV_SET_HDR(dest_hotlist,
				       WMITLV_TAG_STRUC_wmi_extscan_bucket_cmd_fixed_param,
				       WMITLV_GET_STRUCT_TLVLEN
					       (wmi_extscan_hotlist_entry));

			dest_hotlist->min_rssi = src_ap->low;
			WMI_CHAR_ARRAY_TO_MAC_ADDR(src_ap->bssid.bytes,
						   &dest_hotlist->bssid);

			WMI_LOGD("%s:channel:%d min_rssi %d",
				 __func__, dest_hotlist->channel,
				 dest_hotlist->min_rssi);
			WMI_LOGD
				("%s: bssid mac_addr31to0: 0x%x, mac_addr47to32: 0x%x",
				__func__, dest_hotlist->bssid.mac_addr31to0,
				dest_hotlist->bssid.mac_addr47to32);
			dest_hotlist++;
			src_ap++;
		}
		buf_ptr += WMI_TLV_HDR_SIZE +
			   (min_entries * sizeof(wmi_extscan_hotlist_entry));

		if (wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_EXTSCAN_CONFIGURE_HOTLIST_MONITOR_CMDID)) {
			WMI_LOGE("%s: failed to send command", __func__);
			wmi_buf_free(buf);
			return QDF_STATUS_E_FAILURE;
		}
		index = index + min_entries;
		num_entries = numap - min_entries;
		len = cmd_len;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_active_bpf_mode_cmd_tlv() - configure active BPF mode in FW
 * @wmi_handle: the WMI handle
 * @vdev_id: the Id of the vdev to apply the configuration to
 * @ucast_mode: the active BPF mode to configure for unicast packets
 * @mcast_bcast_mode: the active BPF mode to configure for multicast/broadcast
 *	packets
 *
 * Return: QDF status
 */
static QDF_STATUS send_set_active_bpf_mode_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t vdev_id,
				enum wmi_host_active_bpf_mode ucast_mode,
				enum wmi_host_active_bpf_mode mcast_bcast_mode)
{
	const WMITLV_TAG_ID tag_id =
		WMITLV_TAG_STRUC_wmi_bpf_set_vdev_active_mode_cmd_fixed_param;
	const uint32_t tlv_len = WMITLV_GET_STRUCT_TLVLEN(
				wmi_bpf_set_vdev_active_mode_cmd_fixed_param);
	QDF_STATUS status;
	wmi_bpf_set_vdev_active_mode_cmd_fixed_param *cmd;
	wmi_buf_t buf;

	WMI_LOGI("Sending WMI_BPF_SET_VDEV_ACTIVE_MODE_CMDID(%u, %d, %d)",
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

	WMI_LOGI("Sent WMI_BPF_SET_VDEV_ACTIVE_MODE_CMDID successfully");

	return QDF_STATUS_SUCCESS;
}

/**
 * send_power_dbg_cmd_tlv() - send power debug commands
 * @wmi_handle: wmi handle
 * @param: wmi power debug parameter
 *
 * Send WMI_POWER_DEBUG_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS send_power_dbg_cmd_tlv(wmi_unified_t wmi_handle,
					 struct wmi_power_dbg_params *param)
{
	wmi_buf_t buf = NULL;
	QDF_STATUS status;
	int len, args_tlv_len;
	uint8_t *buf_ptr;
	uint8_t i;
	wmi_pdev_wal_power_debug_cmd_fixed_param *cmd;
	uint32_t *cmd_args;

	/* Prepare and send power debug cmd parameters */
	args_tlv_len = WMI_TLV_HDR_SIZE + param->num_args * sizeof(uint32_t);
	len = sizeof(*cmd) + args_tlv_len;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_pdev_wal_power_debug_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		  WMITLV_TAG_STRUC_wmi_pdev_wal_power_debug_cmd_fixed_param,
		  WMITLV_GET_STRUCT_TLVLEN
		  (wmi_pdev_wal_power_debug_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	cmd->module_id = param->module_id;
	cmd->num_args = param->num_args;
	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (param->num_args * sizeof(uint32_t)));
	cmd_args = (uint32_t *) (buf_ptr + WMI_TLV_HDR_SIZE);
	WMI_LOGI("%s: %d num of args = ", __func__, param->num_args);
	for (i = 0; (i < param->num_args && i < WMI_MAX_NUM_ARGS); i++) {
		cmd_args[i] = param->args[i];
		WMI_LOGI("%d,", param->args[i]);
	}

	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_PDEV_WAL_POWER_DEBUG_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("wmi_unified_cmd_send WMI_PDEV_WAL_POWER_DEBUG_CMDID returned Error %d",
			status);
		goto error;
	}

	return QDF_STATUS_SUCCESS;
error:
	wmi_buf_free(buf);

	return status;
}

/* copy_hw_mode_id_in_init_cmd() - Helper routine to copy hw_mode in init cmd
 * @wmi_handle: pointer to wmi handle
 * @buf_ptr: pointer to current position in init command buffer
 * @len: pointer to length. This will be updated with current lenght of cmd
 * @param: point host parameters for init command
 *
 * Return: Updated pointer of buf_ptr.
 */
static inline uint8_t *copy_hw_mode_in_init_cmd(struct wmi_unified *wmi_handle,
		uint8_t *buf_ptr, int *len, struct wmi_init_cmd_param *param)
{
	uint16_t idx;

	if (param->hw_mode_id != WMI_HOST_HW_MODE_MAX) {
		wmi_pdev_set_hw_mode_cmd_fixed_param *hw_mode;
		wmi_pdev_band_to_mac *band_to_mac;

		hw_mode = (wmi_pdev_set_hw_mode_cmd_fixed_param *)
			(buf_ptr + sizeof(wmi_init_cmd_fixed_param) +
			 sizeof(wmi_resource_config) +
			 WMI_TLV_HDR_SIZE + (param->num_mem_chunks *
				 sizeof(wlan_host_memory_chunk)));

		WMITLV_SET_HDR(&hw_mode->tlv_header,
			WMITLV_TAG_STRUC_wmi_pdev_set_hw_mode_cmd_fixed_param,
			(WMITLV_GET_STRUCT_TLVLEN
			 (wmi_pdev_set_hw_mode_cmd_fixed_param)));

		hw_mode->hw_mode_index = param->hw_mode_id;
		hw_mode->num_band_to_mac = param->num_band_to_mac;

		buf_ptr = (uint8_t *) (hw_mode + 1);
		band_to_mac = (wmi_pdev_band_to_mac *) (buf_ptr +
				WMI_TLV_HDR_SIZE);
		for (idx = 0; idx < param->num_band_to_mac; idx++) {
			WMITLV_SET_HDR(&band_to_mac[idx].tlv_header,
					WMITLV_TAG_STRUC_wmi_pdev_band_to_mac,
					WMITLV_GET_STRUCT_TLVLEN
					(wmi_pdev_band_to_mac));
			band_to_mac[idx].pdev_id =
				wmi_handle->ops->convert_pdev_id_host_to_target(
					param->band_to_mac[idx].pdev_id);
			band_to_mac[idx].start_freq =
				param->band_to_mac[idx].start_freq;
			band_to_mac[idx].end_freq =
				param->band_to_mac[idx].end_freq;
		}
		*len += sizeof(wmi_pdev_set_hw_mode_cmd_fixed_param) +
			(param->num_band_to_mac *
			 sizeof(wmi_pdev_band_to_mac)) +
			WMI_TLV_HDR_SIZE;

		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
				(param->num_band_to_mac *
				 sizeof(wmi_pdev_band_to_mac)));
	}

	return buf_ptr;
}

/**
 * send_multiple_vdev_restart_req_cmd_tlv() - send multiple vdev restart req
 * @wmi_handle: wmi handle
 * @param: wmi multiple vdev restart req param
 *
 * Send WMI_PDEV_MULTIPLE_VDEV_RESTART_REQUEST_CMDID parameters to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS send_multiple_vdev_restart_req_cmd_tlv(
				wmi_unified_t wmi_handle,
				struct multiple_vdev_restart_params *param)
{
	wmi_buf_t buf;
	QDF_STATUS qdf_status;
	wmi_pdev_multiple_vdev_restart_request_cmd_fixed_param *cmd;
	int i;
	uint8_t *buf_ptr;
	uint32_t *vdev_ids;
	wmi_channel *chan_info;
	struct channel_param *tchan_info;
	uint16_t len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;

	len += sizeof(wmi_channel);
	if (param->num_vdevs)
		len += sizeof(uint32_t) * param->num_vdevs;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("Failed to allocate memory\n");
		qdf_status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_pdev_multiple_vdev_restart_request_cmd_fixed_param *)
	       buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_pdev_multiple_vdev_restart_request_cmd_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN
		(wmi_pdev_multiple_vdev_restart_request_cmd_fixed_param));
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	cmd->requestor_id = param->requestor_id;
	cmd->disable_hw_ack = param->disable_hw_ack;
	cmd->cac_duration_ms = param->cac_duration_ms;
	cmd->num_vdevs = param->num_vdevs;

	buf_ptr += sizeof(*cmd);

	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_UINT32,
		       sizeof(A_UINT32) * param->num_vdevs);
	vdev_ids = (uint32_t *)(buf_ptr + WMI_TLV_HDR_SIZE);
	for (i = 0; i < param->num_vdevs; i++) {
		vdev_ids[i] = param->vdev_ids[i];
	}

	buf_ptr += (sizeof(A_UINT32) * param->num_vdevs) + WMI_TLV_HDR_SIZE;

	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_STRUC_wmi_channel,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
	chan_info = (wmi_channel *)buf_ptr;
	tchan_info = &(param->ch_param);
	chan_info->mhz = tchan_info->mhz;
	chan_info->band_center_freq1 = tchan_info->cfreq1;
	chan_info->band_center_freq2 = tchan_info->cfreq2;
	if (tchan_info->is_chan_passive)
		WMI_SET_CHANNEL_FLAG(chan_info,
				     WMI_CHAN_FLAG_PASSIVE);
	if (tchan_info->allow_vht)
		WMI_SET_CHANNEL_FLAG(chan_info,
				     WMI_CHAN_FLAG_ALLOW_VHT);
	else  if (tchan_info->allow_ht)
		WMI_SET_CHANNEL_FLAG(chan_info,
				     WMI_CHAN_FLAG_ALLOW_HT);
	WMI_SET_CHANNEL_MODE(chan_info, tchan_info->phy_mode);
	WMI_SET_CHANNEL_MIN_POWER(chan_info, tchan_info->minpower);
	WMI_SET_CHANNEL_MAX_POWER(chan_info, tchan_info->maxpower);
	WMI_SET_CHANNEL_REG_POWER(chan_info, tchan_info->maxregpower);
	WMI_SET_CHANNEL_ANTENNA_MAX(chan_info, tchan_info->antennamax);
	WMI_SET_CHANNEL_REG_CLASSID(chan_info, tchan_info->reg_class_id);

	qdf_status = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_PDEV_MULTIPLE_VDEV_RESTART_REQUEST_CMDID);

	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		WMI_LOGE("%s: Failed to send\n", __func__);
		wmi_buf_free(buf);
	}

end:
	return qdf_status;
}

/**
 * send_dfs_phyerr_offload_en_cmd_tlv() - send dfs phyerr offload enable cmd
 * @wmi_handle: wmi handle
 * @pdev_id: pdev id
 *
 * Send WMI_PDEV_DFS_PHYERR_OFFLOAD_ENABLE_CMDID command to firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS send_dfs_phyerr_offload_en_cmd_tlv(wmi_unified_t wmi_handle,
		uint32_t pdev_id)
{
	wmi_pdev_dfs_phyerr_offload_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	WMI_LOGI("%s: pdev_id=%d", __func__, pdev_id);

	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_dfs_phyerr_offload_enable_cmd_fixed_param *)
		wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_pdev_dfs_phyerr_offload_enable_cmd_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN(
		wmi_pdev_dfs_phyerr_offload_enable_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(pdev_id);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_DFS_PHYERR_OFFLOAD_ENABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send cmd to fw, ret=%d, pdev_id=%d",
			__func__, ret, pdev_id);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_dfs_phyerr_offload_dis_cmd_tlv() - send dfs phyerr offload disable cmd
 * @wmi_handle: wmi handle
 * @pdev_id: pdev id
 *
 * Send WMI_PDEV_DFS_PHYERR_OFFLOAD_DISABLE_CMDID command to firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS send_dfs_phyerr_offload_dis_cmd_tlv(wmi_unified_t wmi_handle,
		uint32_t pdev_id)
{
	wmi_pdev_dfs_phyerr_offload_disable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);

	WMI_LOGI("%s: pdev_id=%d", __func__, pdev_id);

	if (!buf) {
		WMI_LOGE("%s : wmi_buf_alloc failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_dfs_phyerr_offload_disable_cmd_fixed_param *)
		wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_pdev_dfs_phyerr_offload_disable_cmd_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN(
		wmi_pdev_dfs_phyerr_offload_disable_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(pdev_id);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_DFS_PHYERR_OFFLOAD_DISABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("%s: Failed to send cmd to fw, ret=%d, pdev_id=%d",
			__func__, ret, pdev_id);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * init_cmd_send_tlv() - send initialization cmd to fw
 * @wmi_handle: wmi handle
 * @param param: pointer to wmi init param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS init_cmd_send_tlv(wmi_unified_t wmi_handle,
				struct wmi_init_cmd_param *param)
{
	wmi_buf_t buf;
	wmi_init_cmd_fixed_param *cmd;
	wmi_abi_version my_vers;
	int num_whitelist;
	uint8_t *buf_ptr;
	wmi_resource_config *resource_cfg;
	wlan_host_memory_chunk *host_mem_chunks;
	uint32_t mem_chunk_len = 0, hw_mode_len = 0;
	uint16_t idx;
	int len;
	QDF_STATUS ret;

	len = sizeof(*cmd) + sizeof(wmi_resource_config) +
		WMI_TLV_HDR_SIZE;
	mem_chunk_len = (sizeof(wlan_host_memory_chunk) * MAX_MEM_CHUNKS);

	if (param->hw_mode_id != WMI_HOST_HW_MODE_MAX)
		hw_mode_len = sizeof(wmi_pdev_set_hw_mode_cmd_fixed_param) +
			WMI_TLV_HDR_SIZE +
			(param->num_band_to_mac * sizeof(wmi_pdev_band_to_mac));

	buf = wmi_buf_alloc(wmi_handle, len + mem_chunk_len + hw_mode_len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_init_cmd_fixed_param *) buf_ptr;
	resource_cfg = (wmi_resource_config *) (buf_ptr + sizeof(*cmd));

	host_mem_chunks = (wlan_host_memory_chunk *)
		(buf_ptr + sizeof(*cmd) + sizeof(wmi_resource_config)
		 + WMI_TLV_HDR_SIZE);

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_init_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(wmi_init_cmd_fixed_param));

	wmi_copy_resource_config(resource_cfg, param->res_cfg);
	WMITLV_SET_HDR(&resource_cfg->tlv_header,
			WMITLV_TAG_STRUC_wmi_resource_config,
			WMITLV_GET_STRUCT_TLVLEN(wmi_resource_config));

	for (idx = 0; idx < param->num_mem_chunks; ++idx) {
		WMITLV_SET_HDR(&(host_mem_chunks[idx].tlv_header),
				WMITLV_TAG_STRUC_wlan_host_memory_chunk,
				WMITLV_GET_STRUCT_TLVLEN
				(wlan_host_memory_chunk));
		host_mem_chunks[idx].ptr = param->mem_chunks[idx].paddr;
		host_mem_chunks[idx].size = param->mem_chunks[idx].len;
		host_mem_chunks[idx].req_id = param->mem_chunks[idx].req_id;
		qdf_print("chunk %d len %d requested ,ptr  0x%x ",
				idx, host_mem_chunks[idx].size,
				host_mem_chunks[idx].ptr);
	}
	cmd->num_host_mem_chunks = param->num_mem_chunks;
	len += (param->num_mem_chunks * sizeof(wlan_host_memory_chunk));

	WMITLV_SET_HDR((buf_ptr + sizeof(*cmd) + sizeof(wmi_resource_config)),
			WMITLV_TAG_ARRAY_STRUC,
			(sizeof(wlan_host_memory_chunk) *
			 param->num_mem_chunks));

	/* Fill hw mode id config */
	buf_ptr = copy_hw_mode_in_init_cmd(wmi_handle, buf_ptr, &len, param);

	num_whitelist = sizeof(version_whitelist) /
		sizeof(wmi_whitelist_version_info);
	my_vers.abi_version_0 = WMI_ABI_VERSION_0;
	my_vers.abi_version_1 = WMI_ABI_VERSION_1;
	my_vers.abi_version_ns_0 = WMI_ABI_VERSION_NS_0;
	my_vers.abi_version_ns_1 = WMI_ABI_VERSION_NS_1;
	my_vers.abi_version_ns_2 = WMI_ABI_VERSION_NS_2;
	my_vers.abi_version_ns_3 = WMI_ABI_VERSION_NS_3;

#ifdef CONFIG_MCL
	wmi_cmp_and_set_abi_version(num_whitelist, version_whitelist,
			&my_vers,
			(struct _wmi_abi_version *)&wmi_handle->fw_abi_version,
			&cmd->host_abi_vers);
#endif
	qdf_print("%s: INIT_CMD version: %d, %d, 0x%x, 0x%x, 0x%x, 0x%x",
			__func__,
			WMI_VER_GET_MAJOR(cmd->host_abi_vers.abi_version_0),
			WMI_VER_GET_MINOR(cmd->host_abi_vers.abi_version_0),
			cmd->host_abi_vers.abi_version_ns_0,
			cmd->host_abi_vers.abi_version_ns_1,
			cmd->host_abi_vers.abi_version_ns_2,
			cmd->host_abi_vers.abi_version_ns_3);

	/* Save version sent from host -
	 * Will be used to check ready event
	 */
#ifdef CONFIG_MCL
	qdf_mem_copy(&wmi_handle->final_abi_vers, &cmd->host_abi_vers,
			sizeof(wmi_abi_version));
#endif
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_INIT_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMI_LOGE("wmi_unified_cmd_send WMI_INIT_CMDID returned Error %d",
			ret);
		wmi_buf_free(buf);
	}
	return ret;

}

/**
 * save_service_bitmap_tlv() - save service bitmap
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param bitmap_buf: bitmap buffer, for converged legacy support
 *
 * Return: None
 */
#ifndef CONFIG_MCL
static
void save_service_bitmap_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			     void *bitmap_buf)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	qdf_mem_copy(wmi_handle->wmi_service_bitmap,
			param_buf->wmi_service_bitmap,
			(WMI_SERVICE_BM_SIZE * sizeof(uint32_t)));

	if (bitmap_buf)
		qdf_mem_copy(bitmap_buf,
			     param_buf->wmi_service_bitmap,
			     (WMI_SERVICE_BM_SIZE * sizeof(uint32_t)));
}
#else
static
void save_service_bitmap_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			     void *bitmap_buf)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	if (bitmap_buf)
		qdf_mem_copy(bitmap_buf,
			     param_buf->wmi_service_bitmap,
			     (WMI_SERVICE_BM_SIZE * sizeof(uint32_t)));
}
#endif

/**
 * is_service_enabled_tlv() - Check if service enabled
 * @param wmi_handle: wmi handle
 * @param service_id: service identifier
 *
 * Return: 1 enabled, 0 disabled
 */
#ifndef CONFIG_MCL
static bool is_service_enabled_tlv(wmi_unified_t wmi_handle,
		uint32_t service_id)
{
	return WMI_SERVICE_IS_ENABLED(wmi_handle->wmi_service_bitmap,
						service_id);
}
#else
static bool is_service_enabled_tlv(wmi_unified_t wmi_handle,
		uint32_t service_id)
{
	return false;
}
#endif

/**
 * extract_service_ready_tlv() - extract service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to received event buffer
 * @param cap: pointer to hold target capability information extracted from even
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_service_ready_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, struct wlan_psoc_target_capability_info *cap)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_event_fixed_param *ev;


	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	ev = (wmi_service_ready_event_fixed_param *) param_buf->fixed_param;
	if (!ev) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cap->phy_capability = ev->phy_capability;
	cap->max_frag_entry = ev->max_frag_entry;
	cap->num_rf_chains = ev->num_rf_chains;
	cap->ht_cap_info = ev->ht_cap_info;
	cap->vht_cap_info = ev->vht_cap_info;
	cap->vht_supp_mcs = ev->vht_supp_mcs;
	cap->hw_min_tx_power = ev->hw_min_tx_power;
	cap->hw_max_tx_power = ev->hw_max_tx_power;
	cap->sys_cap_info = ev->sys_cap_info;
	cap->min_pkt_size_enable = ev->min_pkt_size_enable;
	cap->max_bcn_ie_size = ev->max_bcn_ie_size;
	cap->max_num_scan_channels = ev->max_num_scan_channels;
	cap->max_supported_macs = ev->max_supported_macs;
	cap->wmi_fw_sub_feat_caps = ev->wmi_fw_sub_feat_caps;
	cap->txrx_chainmask = ev->txrx_chainmask;
	cap->default_dbs_hw_mode_index = ev->default_dbs_hw_mode_index;
	cap->num_msdu_desc = ev->num_msdu_desc;

	return QDF_STATUS_SUCCESS;
}

/* convert_wireless_modes_tlv() - Convert REGDMN_MODE values sent by target
 *         to host internal WMI_HOST_REGDMN_MODE values.
 *         REGULATORY TODO : REGDMN_MODE_11AC_VHT*_2G values are not used by the
 *         host currently. Add this in the future if required.
 *         11AX (Phase II) : 11ax related values are not currently
 *         advertised separately by FW. As part of phase II regulatory bring-up,
 *         finalize the advertisement mechanism.
 * @target_wireless_mode: target wireless mode received in message
 *
 * Return: returns the host internal wireless mode.
 */
static inline uint32_t convert_wireless_modes_tlv(uint32_t target_wireless_mode)
{

	uint32_t wireless_modes = 0;

	if (target_wireless_mode & REGDMN_MODE_11A)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11A;

	if (target_wireless_mode & REGDMN_MODE_TURBO)
		wireless_modes |= WMI_HOST_REGDMN_MODE_TURBO;

	if (target_wireless_mode & REGDMN_MODE_11B)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11B;

	if (target_wireless_mode & REGDMN_MODE_PUREG)
		wireless_modes |= WMI_HOST_REGDMN_MODE_PUREG;

	if (target_wireless_mode & REGDMN_MODE_11G)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11G;

	if (target_wireless_mode & REGDMN_MODE_108G)
		wireless_modes |= WMI_HOST_REGDMN_MODE_108G;

	if (target_wireless_mode & REGDMN_MODE_108A)
		wireless_modes |= WMI_HOST_REGDMN_MODE_108A;

	if (target_wireless_mode & REGDMN_MODE_XR)
		wireless_modes |= WMI_HOST_REGDMN_MODE_XR;

	if (target_wireless_mode & REGDMN_MODE_11A_HALF_RATE)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11A_HALF_RATE;

	if (target_wireless_mode & REGDMN_MODE_11A_QUARTER_RATE)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11A_QUARTER_RATE;

	if (target_wireless_mode & REGDMN_MODE_11NG_HT20)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11NG_HT20;

	if (target_wireless_mode & REGDMN_MODE_11NA_HT20)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11NA_HT20;

	if (target_wireless_mode & REGDMN_MODE_11NG_HT40PLUS)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11NG_HT40PLUS;

	if (target_wireless_mode & REGDMN_MODE_11NG_HT40MINUS)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11NG_HT40MINUS;

	if (target_wireless_mode & REGDMN_MODE_11NA_HT40PLUS)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11NA_HT40PLUS;

	if (target_wireless_mode & REGDMN_MODE_11NA_HT40MINUS)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11NA_HT40MINUS;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT20)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT20;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT40PLUS)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT40PLUS;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT40MINUS)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT40MINUS;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT80)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT80;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT160)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT160;

	if (target_wireless_mode & REGDMN_MODE_11AC_VHT80_80)
		wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT80_80;

	return wireless_modes;
}

/**
 * extract_hal_reg_cap_tlv() - extract HAL registered capabilities
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param cap: pointer to hold HAL reg capabilities
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_hal_reg_cap_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, struct wlan_psoc_hal_reg_capability *cap)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;

	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	qdf_mem_copy(cap, (((uint8_t *)param_buf->hal_reg_capabilities) +
		sizeof(uint32_t)),
		sizeof(struct wlan_psoc_hal_reg_capability));

	cap->wireless_modes = convert_wireless_modes_tlv(
			param_buf->hal_reg_capabilities->wireless_modes);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_host_mem_req_tlv() - Extract host memory request event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param num_entries: pointer to hold number of entries requested
 *
 * Return: Number of entries requested
 */
static host_mem_req *extract_host_mem_req_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint8_t *num_entries)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_event_fixed_param *ev;

	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	ev = (wmi_service_ready_event_fixed_param *) param_buf->fixed_param;
	if (!ev) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return NULL;
	}

	*num_entries = ev->num_mem_reqs;

	return (host_mem_req *)param_buf->mem_reqs;
}

/**
 * save_fw_version_in_service_ready_tlv() - Save fw version in service
 * ready function
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
save_fw_version_in_service_ready_tlv(wmi_unified_t wmi_handle, void *evt_buf)
{
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_event_fixed_param *ev;


	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) evt_buf;

	ev = (wmi_service_ready_event_fixed_param *) param_buf->fixed_param;
	if (!ev) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

#ifdef CONFIG_MCL
	/*Save fw version from service ready message */
	/*This will be used while sending INIT message */
	qdf_mem_copy(&wmi_handle->fw_abi_version, &ev->fw_abi_vers,
			sizeof(wmi_handle->fw_abi_version));
#endif
	return QDF_STATUS_SUCCESS;
}

/**
 * ready_extract_init_status_tlv() - Extract init status from ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 *
 * Return: ready status
 */
static uint32_t ready_extract_init_status_tlv(wmi_unified_t wmi_handle,
	void *evt_buf)
{
	WMI_READY_EVENTID_param_tlvs *param_buf = NULL;
	wmi_ready_event_fixed_param *ev = NULL;


	param_buf = (WMI_READY_EVENTID_param_tlvs *) evt_buf;
	ev = param_buf->fixed_param;

	qdf_print("%s:%d\n", __func__, ev->status);

	return ev->status;
}

/**
 * ready_extract_mac_addr_tlv() - extract mac address from ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param macaddr: Pointer to hold MAC address
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS ready_extract_mac_addr_tlv(wmi_unified_t wmi_hamdle,
	void *evt_buf, uint8_t *macaddr)
{
	WMI_READY_EVENTID_param_tlvs *param_buf = NULL;
	wmi_ready_event_fixed_param *ev = NULL;


	param_buf = (WMI_READY_EVENTID_param_tlvs *) evt_buf;
	ev = param_buf->fixed_param;

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->mac_addr, macaddr);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_dbglog_data_len_tlv() - extract debuglog data length
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: length
 */
static uint8_t *extract_dbglog_data_len_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t *len)
{
	 WMI_DEBUG_MESG_EVENTID_param_tlvs *param_buf;

	 param_buf = (WMI_DEBUG_MESG_EVENTID_param_tlvs *) evt_buf;

	 *len = param_buf->num_bufp;

	 return param_buf->bufp;
}

/**
 * extract_vdev_start_resp_tlv() - extract vdev start response
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_rsp: Pointer to hold vdev response
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_vdev_start_resp_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_vdev_start_resp *vdev_rsp)
{
	WMI_VDEV_START_RESP_EVENTID_param_tlvs *param_buf;
	wmi_vdev_start_response_event_fixed_param *ev;

	param_buf = (WMI_VDEV_START_RESP_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf) {
		qdf_print("Invalid start response event buffer\n");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->fixed_param;
	if (!ev) {
		qdf_print("Invalid start response event buffer\n");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_zero(vdev_rsp, sizeof(*vdev_rsp));

	vdev_rsp->vdev_id = ev->vdev_id;
	vdev_rsp->requestor_id = ev->requestor_id;
	vdev_rsp->resp_type = ev->resp_type;
	vdev_rsp->status = ev->status;
	vdev_rsp->chain_mask = ev->chain_mask;
	vdev_rsp->smps_mode = ev->smps_mode;
	vdev_rsp->mac_id = ev->mac_id;
	vdev_rsp->cfgd_tx_streams = ev->cfgd_tx_streams;
	vdev_rsp->cfgd_rx_streams = ev->cfgd_rx_streams;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_tbttoffset_num_vdevs_tlv() - extract tbtt offset num vdev
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param num_vdevs: Pointer to hold num vdev
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_tbttoffset_num_vdevs_tlv(void *wmi_hdl,
	void *evt_buf, uint32_t *num_vdevs)
{
	WMI_TBTTOFFSET_UPDATE_EVENTID_param_tlvs *param_buf;
	wmi_tbtt_offset_event_fixed_param *tbtt_offset_event;
	uint32_t vdev_map;

	param_buf = (WMI_TBTTOFFSET_UPDATE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		qdf_print("Invalid tbtt update ext event buffer\n");
		return QDF_STATUS_E_INVAL;
	}
	tbtt_offset_event = param_buf->fixed_param;
	vdev_map = tbtt_offset_event->vdev_map;
	*num_vdevs = wmi_vdev_map_to_num_vdevs(vdev_map);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_ext_tbttoffset_num_vdevs_tlv() - extract ext tbtt offset num vdev
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param num_vdevs: Pointer to hold num vdev
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_ext_tbttoffset_num_vdevs_tlv(void *wmi_hdl,
	void *evt_buf, uint32_t *num_vdevs)
{
	WMI_TBTTOFFSET_EXT_UPDATE_EVENTID_param_tlvs *param_buf;
	wmi_tbtt_offset_ext_event_fixed_param *tbtt_offset_ext_event;

	param_buf = (WMI_TBTTOFFSET_EXT_UPDATE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		qdf_print("Invalid tbtt update ext event buffer\n");
		return QDF_STATUS_E_INVAL;
	}
	tbtt_offset_ext_event = param_buf->fixed_param;

	*num_vdevs = tbtt_offset_ext_event->num_vdevs;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_tbttoffset_update_params_tlv() - extract tbtt offset param
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index refering to a vdev
 * @param tbtt_param: Pointer to tbttoffset event param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_tbttoffset_update_params_tlv(void *wmi_hdl,
	void *evt_buf, uint8_t idx,
	struct tbttoffset_params *tbtt_param)
{
	WMI_TBTTOFFSET_UPDATE_EVENTID_param_tlvs *param_buf;
	wmi_tbtt_offset_event_fixed_param *tbtt_offset_event;
	uint32_t vdev_map;

	param_buf = (WMI_TBTTOFFSET_UPDATE_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf) {
		qdf_print("Invalid tbtt update event buffer\n");
		return QDF_STATUS_E_INVAL;
	}

	tbtt_offset_event = param_buf->fixed_param;
	vdev_map = tbtt_offset_event->vdev_map;
	tbtt_param->vdev_id = wmi_vdev_map_to_vdev_id(vdev_map, idx);
	if (tbtt_param->vdev_id == WLAN_INVALID_VDEV_ID)
		return QDF_STATUS_E_INVAL;
	tbtt_param->tbttoffset =
		param_buf->tbttoffset_list[tbtt_param->vdev_id];

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_ext_tbttoffset_update_params_tlv() - extract ext tbtt offset param
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index refering to a vdev
 * @param tbtt_param: Pointer to tbttoffset event param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_ext_tbttoffset_update_params_tlv(void *wmi_hdl,
	void *evt_buf, uint8_t idx,
	struct tbttoffset_params *tbtt_param)
{
	WMI_TBTTOFFSET_EXT_UPDATE_EVENTID_param_tlvs *param_buf;
	wmi_tbtt_offset_info *tbtt_offset_info;

	param_buf = (WMI_TBTTOFFSET_EXT_UPDATE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		qdf_print("Invalid tbtt update event buffer\n");
		return QDF_STATUS_E_INVAL;
	}
	tbtt_offset_info = &param_buf->tbtt_offset_info[idx];

	tbtt_param->vdev_id = tbtt_offset_info->vdev_id;
	tbtt_param->tbttoffset = tbtt_offset_info->tbttoffset;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_mgmt_rx_params_tlv() - extract management rx params from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param hdr: Pointer to hold header
 * @param bufp: Pointer to hold pointer to rx param buffer
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_mgmt_rx_params_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, struct mgmt_rx_event_params *hdr,
	uint8_t **bufp)
{
	WMI_MGMT_RX_EVENTID_param_tlvs *param_tlvs = NULL;
	wmi_mgmt_rx_hdr *ev_hdr = NULL;

	param_tlvs = (WMI_MGMT_RX_EVENTID_param_tlvs *) evt_buf;
	if (!param_tlvs) {
		WMI_LOGE("Get NULL point message from FW");
		return QDF_STATUS_E_INVAL;
	}

	ev_hdr = param_tlvs->hdr;
	if (!hdr) {
		WMI_LOGE("Rx event is NULL");
		return QDF_STATUS_E_INVAL;
	}

	hdr->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							ev_hdr->pdev_id);

	hdr->channel = ev_hdr->channel;
	hdr->snr = ev_hdr->snr;
	hdr->rate = ev_hdr->rate;
	hdr->phy_mode = ev_hdr->phy_mode;
	hdr->buf_len = ev_hdr->buf_len;
	hdr->status = ev_hdr->status;
	hdr->flags = ev_hdr->flags;
	hdr->rssi = ev_hdr->rssi;
	hdr->tsf_delta = ev_hdr->tsf_delta;
	qdf_mem_copy(hdr->rssi_ctl, ev_hdr->rssi_ctl, sizeof(hdr->rssi_ctl));

	*bufp = param_tlvs->bufp;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_stopped_param_tlv() - extract vdev stop param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_id: Pointer to hold vdev identifier
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_vdev_stopped_param_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t *vdev_id)
{
	WMI_VDEV_STOPPED_EVENTID_param_tlvs *param_buf;
	wmi_vdev_stopped_event_fixed_param *resp_event;

	param_buf = (WMI_VDEV_STOPPED_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid event buffer");
		return QDF_STATUS_E_INVAL;
	}
	resp_event = param_buf->fixed_param;
	*vdev_id = resp_event->vdev_id;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_roam_param_tlv() - extract vdev roam param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold roam param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_vdev_roam_param_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_roam_event *param)
{
	WMI_ROAM_EVENTID_param_tlvs *param_buf;
	wmi_roam_event_fixed_param *evt;

	param_buf = (WMI_ROAM_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid roam event buffer");
		return QDF_STATUS_E_INVAL;
	}

	evt = param_buf->fixed_param;
	qdf_mem_zero(param, sizeof(*param));

	param->vdev_id = evt->vdev_id;
	param->reason = evt->reason;
	param->rssi = evt->rssi;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_scan_ev_param_tlv() - extract vdev scan param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold vdev scan param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_vdev_scan_ev_param_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, struct scan_event *param)
{
	WMI_SCAN_EVENTID_param_tlvs *param_buf = NULL;
	wmi_scan_event_fixed_param *evt = NULL;

	param_buf = (WMI_SCAN_EVENTID_param_tlvs *) evt_buf;
	evt = param_buf->fixed_param;

	qdf_mem_zero(param, sizeof(*param));

	switch (evt->event) {
	case WMI_SCAN_EVENT_STARTED:
		param->type = SCAN_EVENT_TYPE_STARTED;
		break;
	case WMI_SCAN_EVENT_COMPLETED:
		param->type = SCAN_EVENT_TYPE_COMPLETED;
		break;
	case WMI_SCAN_EVENT_BSS_CHANNEL:
		param->type = SCAN_EVENT_TYPE_BSS_CHANNEL;
		break;
	case WMI_SCAN_EVENT_FOREIGN_CHANNEL:
		param->type = SCAN_EVENT_TYPE_FOREIGN_CHANNEL;
		break;
	case WMI_SCAN_EVENT_DEQUEUED:
		param->type = SCAN_EVENT_TYPE_DEQUEUED;
		break;
	case WMI_SCAN_EVENT_PREEMPTED:
		param->type = SCAN_EVENT_TYPE_PREEMPTED;
		break;
	case WMI_SCAN_EVENT_START_FAILED:
		param->type = SCAN_EVENT_TYPE_START_FAILED;
		break;
	case WMI_SCAN_EVENT_RESTARTED:
		param->type = SCAN_EVENT_TYPE_RESTARTED;
		break;
	case WMI_HOST_SCAN_EVENT_FOREIGN_CHANNEL_EXIT:
		param->type = SCAN_EVENT_TYPE_FOREIGN_CHANNEL_EXIT;
		break;
	case WMI_SCAN_EVENT_MAX:
	default:
		param->type = SCAN_EVENT_TYPE_MAX;
		break;
	};

	switch (evt->reason) {
	case WMI_SCAN_REASON_NONE:
		param->reason = SCAN_REASON_NONE;
		break;
	case WMI_SCAN_REASON_COMPLETED:
		param->reason = SCAN_REASON_COMPLETED;
		break;
	case WMI_SCAN_REASON_CANCELLED:
		param->reason = SCAN_REASON_CANCELLED;
		break;
	case WMI_SCAN_REASON_PREEMPTED:
		param->reason = SCAN_REASON_PREEMPTED;
		break;
	case WMI_SCAN_REASON_TIMEDOUT:
		param->reason = SCAN_REASON_TIMEDOUT;
		break;
	case WMI_SCAN_REASON_INTERNAL_FAILURE:
		param->reason = SCAN_REASON_INTERNAL_FAILURE;
		break;
	case WMI_SCAN_REASON_SUSPENDED:
		param->reason = SCAN_REASON_SUSPENDED;
		break;
	case WMI_SCAN_REASON_MAX:
		param->reason = SCAN_REASON_MAX;
		break;
	default:
		param->reason = SCAN_REASON_MAX;
		break;
	};

	param->chan_freq = evt->channel_freq;
	param->requester = evt->requestor;
	param->scan_id = evt->scan_id;
	param->vdev_id = evt->vdev_id;

	return QDF_STATUS_SUCCESS;
}

#ifdef CONVERGED_TDLS_ENABLE
/**
 * extract_vdev_tdls_ev_param_tlv() - extract vdev tdls param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold vdev tdls param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_vdev_tdls_ev_param_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, struct tdls_event_info *param)
{
	WMI_TDLS_PEER_EVENTID_param_tlvs *param_buf;
	wmi_tdls_peer_event_fixed_param *evt;

	param_buf = (WMI_TDLS_PEER_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("%s: NULL param_buf", __func__);
		return QDF_STATUS_E_NULL_VALUE;
	}

	evt = param_buf->fixed_param;

	qdf_mem_zero(param, sizeof(*param));

	param->vdev_id = evt->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&evt->peer_macaddr,
				   param->peermac.bytes);
	switch (evt->peer_status) {
	case WMI_TDLS_SHOULD_DISCOVER:
		param->message_type = TDLS_SHOULD_DISCOVER;
		break;
	case WMI_TDLS_SHOULD_TEARDOWN:
		param->message_type = TDLS_SHOULD_TEARDOWN;
		break;
	case WMI_TDLS_PEER_DISCONNECTED:
		param->message_type = TDLS_PEER_DISCONNECTED;
		break;
	case WMI_TDLS_CONNECTION_TRACKER_NOTIFICATION:
		param->message_type = TDLS_CONNECTION_TRACKER_NOTIFY;
		break;
	default:
		WMI_LOGE("%s: Discarding unknown tdls event %d from target",
			 __func__, evt->peer_status);
		return QDF_STATUS_E_INVAL;
	};

	switch (evt->peer_reason) {
	case WMI_TDLS_TEARDOWN_REASON_TX:
		param->peer_reason = TDLS_TEARDOWN_TX;
		break;
	case WMI_TDLS_TEARDOWN_REASON_RSSI:
		param->peer_reason = TDLS_TEARDOWN_RSSI;
		break;
	case WMI_TDLS_TEARDOWN_REASON_SCAN:
		param->peer_reason = TDLS_TEARDOWN_SCAN;
		break;
	case WMI_TDLS_DISCONNECTED_REASON_PEER_DELETE:
		param->peer_reason = TDLS_DISCONNECTED_PEER_DELETE;
		break;
	case WMI_TDLS_TEARDOWN_REASON_PTR_TIMEOUT:
		param->peer_reason = TDLS_TEARDOWN_PTR_TIMEOUT;
		break;
	case WMI_TDLS_TEARDOWN_REASON_BAD_PTR:
		param->peer_reason = TDLS_TEARDOWN_BAD_PTR;
		break;
	case WMI_TDLS_TEARDOWN_REASON_NO_RESPONSE:
		param->peer_reason = TDLS_TEARDOWN_NO_RSP;
		break;
	case WMI_TDLS_ENTER_BUF_STA:
		param->peer_reason = TDLS_PEER_ENTER_BUF_STA;
		break;
	case WMI_TDLS_EXIT_BUF_STA:
		param->peer_reason = TDLS_PEER_EXIT_BUF_STA;
		break;
	case WMI_TDLS_ENTER_BT_BUSY_MODE:
		param->peer_reason = TDLS_ENTER_BT_BUSY;
		break;
	case WMI_TDLS_EXIT_BT_BUSY_MODE:
		param->peer_reason = TDLS_EXIT_BT_BUSY;
		break;
	case WMI_TDLS_SCAN_STARTED_EVENT:
		param->peer_reason = TDLS_SCAN_STARTED;
		break;
	case WMI_TDLS_SCAN_COMPLETED_EVENT:
		param->peer_reason = TDLS_SCAN_COMPLETED;
		break;

	default:
		WMI_LOGE("%s: unknown reason %d in tdls event %d from target",
			 __func__, evt->peer_reason, evt->peer_status);
		return QDF_STATUS_E_INVAL;
	};

	WMI_LOGD("%s: tdls event, peer: %pM, type: 0x%x, reason: %d, vdev: %d",
		 __func__, param->peermac.bytes, param->message_type,
		 param->peer_reason, param->vdev_id);

	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * extract_mgmt_tx_compl_param_tlv() - extract MGMT tx completion event params
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold MGMT TX completion params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_mgmt_tx_compl_param_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_mgmt_tx_compl_event *param)
{
	WMI_MGMT_TX_COMPLETION_EVENTID_param_tlvs *param_buf;
	wmi_mgmt_tx_compl_event_fixed_param *cmpl_params;

	param_buf = (WMI_MGMT_TX_COMPLETION_EVENTID_param_tlvs *)
		evt_buf;
	if (!param_buf) {
		WMI_LOGE("%s: Invalid mgmt Tx completion event", __func__);
		return QDF_STATUS_E_INVAL;
	}
	cmpl_params = param_buf->fixed_param;

	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							cmpl_params->pdev_id);
	param->desc_id = cmpl_params->desc_id;
	param->status = cmpl_params->status;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_offchan_data_tx_compl_param_tlv() -
 *            extract Offchan data tx completion event params
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold offchan data TX completion params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_offchan_data_tx_compl_param_tlv(
		wmi_unified_t wmi_handle, void *evt_buf,
		struct wmi_host_offchan_data_tx_compl_event *param)
{
	WMI_OFFCHAN_DATA_TX_COMPLETION_EVENTID_param_tlvs *param_buf;
	wmi_offchan_data_tx_compl_event_fixed_param *cmpl_params;

	param_buf = (WMI_OFFCHAN_DATA_TX_COMPLETION_EVENTID_param_tlvs *)
		evt_buf;
	if (!param_buf) {
		WMI_LOGE("%s: Invalid offchan data Tx compl event", __func__);
		return QDF_STATUS_E_INVAL;
	}
	cmpl_params = param_buf->fixed_param;

	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							cmpl_params->pdev_id);
	param->desc_id = cmpl_params->desc_id;
	param->status = cmpl_params->status;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_csa_switch_count_status_tlv() - extract pdev csa switch count
 *                                              status tlv
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold csa switch count status event param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_pdev_csa_switch_count_status_tlv(
				wmi_unified_t wmi_handle,
				void *evt_buf,
				struct pdev_csa_switch_count_status *param)
{
	WMI_PDEV_CSA_SWITCH_COUNT_STATUS_EVENTID_param_tlvs *param_buf;
	wmi_pdev_csa_switch_count_status_event_fixed_param *csa_status;

	param_buf = (WMI_PDEV_CSA_SWITCH_COUNT_STATUS_EVENTID_param_tlvs *)
		     evt_buf;
	if (!param_buf) {
		WMI_LOGE("%s: Invalid CSA status event\n", __func__);
		return QDF_STATUS_E_INVAL;
	}

	csa_status = param_buf->fixed_param;

	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							csa_status->pdev_id);
	param->current_switch_count = csa_status->current_switch_count;
	param->num_vdevs = csa_status->num_vdevs;
	param->vdev_ids = param_buf->vdev_ids;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_swba_num_vdevs_tlv() - extract swba num vdevs from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param num_vdevs: Pointer to hold num vdevs
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_swba_num_vdevs_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t *num_vdevs)
{
	WMI_HOST_SWBA_EVENTID_param_tlvs *param_buf;
	wmi_host_swba_event_fixed_param *swba_event;
	uint32_t vdev_map;

	param_buf = (WMI_HOST_SWBA_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid swba event buffer");
		return QDF_STATUS_E_INVAL;
	}

	swba_event = param_buf->fixed_param;
	*num_vdevs = swba_event->num_vdevs;
	if (!(*num_vdevs)) {
		vdev_map = swba_event->vdev_map;
		*num_vdevs = wmi_vdev_map_to_num_vdevs(vdev_map);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_swba_tim_info_tlv() - extract swba tim info from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index to bcn info
 * @param tim_info: Pointer to hold tim info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_swba_tim_info_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t idx, wmi_host_tim_info *tim_info)
{
	WMI_HOST_SWBA_EVENTID_param_tlvs *param_buf;
	wmi_tim_info *tim_info_ev;

	param_buf = (WMI_HOST_SWBA_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid swba event buffer");
		return QDF_STATUS_E_INVAL;
	}

	tim_info_ev = &param_buf->tim_info[idx];

	tim_info->tim_len = tim_info_ev->tim_len;
	tim_info->tim_mcast = tim_info_ev->tim_mcast;
	qdf_mem_copy(tim_info->tim_bitmap, tim_info_ev->tim_bitmap,
			(sizeof(uint32_t) * WMI_TIM_BITMAP_ARRAY_SIZE));
	tim_info->tim_changed = tim_info_ev->tim_changed;
	tim_info->tim_num_ps_pending = tim_info_ev->tim_num_ps_pending;
	tim_info->vdev_id = tim_info_ev->vdev_id;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_swba_noa_info_tlv() - extract swba NoA information from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index to bcn info
 * @param p2p_desc: Pointer to hold p2p NoA info
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_swba_noa_info_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t idx, wmi_host_p2p_noa_info *p2p_desc)
{
	WMI_HOST_SWBA_EVENTID_param_tlvs *param_buf;
	wmi_p2p_noa_info *p2p_noa_info;
	uint8_t i = 0;

	param_buf = (WMI_HOST_SWBA_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf) {
		WMI_LOGE("Invalid swba event buffer");
		return QDF_STATUS_E_INVAL;
	}

	p2p_noa_info = &param_buf->p2p_noa_info[idx];

	p2p_desc->modified = false;
	p2p_desc->num_descriptors = 0;
	if (WMI_UNIFIED_NOA_ATTR_IS_MODIFIED(p2p_noa_info)) {
		p2p_desc->modified = true;
		p2p_desc->index =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_INDEX_GET(p2p_noa_info);
		p2p_desc->oppPS =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_OPP_PS_GET(p2p_noa_info);
		p2p_desc->ctwindow =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_CTWIN_GET(p2p_noa_info);
		p2p_desc->num_descriptors =
			(uint8_t) WMI_UNIFIED_NOA_ATTR_NUM_DESC_GET
							(p2p_noa_info);
		for (i = 0; i < p2p_desc->num_descriptors; i++) {
			p2p_desc->noa_descriptors[i].type_count =
				(uint8_t) p2p_noa_info->noa_descriptors[i].
				type_count;
			p2p_desc->noa_descriptors[i].duration =
				p2p_noa_info->noa_descriptors[i].duration;
			p2p_desc->noa_descriptors[i].interval =
				p2p_noa_info->noa_descriptors[i].interval;
			p2p_desc->noa_descriptors[i].start_time =
				p2p_noa_info->noa_descriptors[i].start_time;
		}
		p2p_desc->vdev_id = p2p_noa_info->vdev_id;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef CONVERGED_P2P_ENABLE
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

	param_tlvs = (WMI_P2P_NOA_EVENTID_param_tlvs *) evt_buf;
	if (!param_tlvs) {
		WMI_LOGE("%s: Invalid P2P NoA event buffer", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!param) {
		WMI_LOGE("noa information param is null");
		return QDF_STATUS_E_INVAL;
	}

	fixed_param = param_tlvs->fixed_param;
	buf_ptr = (uint8_t *) fixed_param;
	buf_ptr += sizeof(wmi_p2p_noa_event_fixed_param);
	wmi_noa_info = (wmi_p2p_noa_info *) (buf_ptr);

	if (!WMI_UNIFIED_NOA_ATTR_IS_MODIFIED(wmi_noa_info)) {
		WMI_LOGE("%s: noa attr is not modified", __func__);
		return QDF_STATUS_E_INVAL;
	}

	param->vdev_id = fixed_param->vdev_id;
	param->index =
		(uint8_t) WMI_UNIFIED_NOA_ATTR_INDEX_GET(wmi_noa_info);
	param->opps_ps =
		(uint8_t) WMI_UNIFIED_NOA_ATTR_OPP_PS_GET(wmi_noa_info);
	param->ct_window =
		(uint8_t) WMI_UNIFIED_NOA_ATTR_CTWIN_GET(wmi_noa_info);
	descriptors = WMI_UNIFIED_NOA_ATTR_NUM_DESC_GET(wmi_noa_info);
	param->num_desc = (uint8_t) descriptors;

	WMI_LOGD("%s:index %u, opps_ps %u, ct_window %u, num_descriptors = %u", __func__,
		param->index, param->opps_ps, param->ct_window,
		param->num_desc);
	for (i = 0; i < param->num_desc; i++) {
		param->noa_desc[i].type_count =
			(uint8_t) wmi_noa_info->noa_descriptors[i].
			type_count;
		param->noa_desc[i].duration =
			wmi_noa_info->noa_descriptors[i].duration;
		param->noa_desc[i].interval =
			wmi_noa_info->noa_descriptors[i].interval;
		param->noa_desc[i].start_time =
			wmi_noa_info->noa_descriptors[i].start_time;
		WMI_LOGD("%s:NoA descriptor[%d] type_count %u, duration %u, interval %u, start_time = %u",
			__func__, i, param->noa_desc[i].type_count,
			param->noa_desc[i].duration,
			param->noa_desc[i].interval,
			param->noa_desc[i].start_time);
	}

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
		WMI_LOGE("%s: Invalid P2P lo stop event buffer", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (!param) {
		WMI_LOGE("lo stop event param is null");
		return QDF_STATUS_E_INVAL;
	}

	lo_param = param_tlvs->fixed_param;
	param->vdev_id = lo_param->vdev_id;
	param->reason_code = lo_param->reason;
	WMI_LOGD("%s: vdev_id:%d, reason:%d", __func__,
		param->vdev_id, param->reason_code);

	return QDF_STATUS_SUCCESS;
}
#endif /* End of CONVERGED_P2P_ENABLE */

/**
 * extract_peer_sta_kickout_ev_tlv() - extract peer sta kickout event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold peer param
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_peer_sta_kickout_ev_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_peer_sta_kickout_event *ev)
{
	WMI_PEER_STA_KICKOUT_EVENTID_param_tlvs *param_buf = NULL;
	wmi_peer_sta_kickout_event_fixed_param *kickout_event = NULL;

	param_buf = (WMI_PEER_STA_KICKOUT_EVENTID_param_tlvs *) evt_buf;
	kickout_event = param_buf->fixed_param;

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&kickout_event->peer_macaddr,
							ev->peer_macaddr);

	ev->reason = kickout_event->reason;
	ev->rssi = kickout_event->rssi;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_all_stats_counts_tlv() - extract all stats count from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param stats_param: Pointer to hold stats count
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_all_stats_counts_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_stats_event *stats_param)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;

	ev = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	if (!ev) {
		WMI_LOGE("%s: Failed to alloc memory\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	switch (ev->stats_id) {
	case WMI_REQUEST_PEER_STAT:
		stats_param->stats_id = WMI_HOST_REQUEST_PEER_STAT;
		break;

	case WMI_REQUEST_AP_STAT:
		stats_param->stats_id = WMI_HOST_REQUEST_AP_STAT;
		break;

	case WMI_REQUEST_PDEV_STAT:
		stats_param->stats_id = WMI_HOST_REQUEST_PDEV_STAT;
		break;

	case WMI_REQUEST_VDEV_STAT:
		stats_param->stats_id = WMI_HOST_REQUEST_VDEV_STAT;
		break;

	case WMI_REQUEST_BCNFLT_STAT:
		stats_param->stats_id = WMI_HOST_REQUEST_BCNFLT_STAT;
		break;

	case WMI_REQUEST_VDEV_RATE_STAT:
		stats_param->stats_id = WMI_HOST_REQUEST_VDEV_RATE_STAT;
		break;

	default:
		stats_param->stats_id = 0;
		break;

	}

	stats_param->num_pdev_stats = ev->num_pdev_stats;
	stats_param->num_pdev_ext_stats = 0;
	stats_param->num_vdev_stats = ev->num_vdev_stats;
	stats_param->num_peer_stats = ev->num_peer_stats;
	stats_param->num_bcnflt_stats = ev->num_bcnflt_stats;
	stats_param->num_chan_stats = ev->num_chan_stats;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_stats_tlv() - extract pdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into pdev stats
 * @param pdev_stats: Pointer to hold pdev stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_pdev_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_pdev_stats *pdev_stats)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_ext_stats_tlv() - extract extended pdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended pdev stats
 * @param pdev_ext_stats: Pointer to hold extended pdev stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_pdev_ext_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_pdev_ext_stats *pdev_ext_stats)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_stats_tlv() - extract vdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into vdev stats
 * @param vdev_stats: Pointer to hold vdev stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_vdev_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_vdev_stats *vdev_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	data = (uint8_t *) param_buf->data;

	if (index < ev_param->num_vdev_stats) {
		wmi_vdev_stats *ev = (wmi_vdev_stats *) ((data) +
				((ev_param->num_pdev_stats) *
				sizeof(wmi_pdev_stats)) +
				(index * sizeof(wmi_vdev_stats)));

		vdev_stats->vdev_id = ev->vdev_id;
		vdev_stats->vdev_snr.bcn_snr = ev->vdev_snr.bcn_snr;
		vdev_stats->vdev_snr.dat_snr = ev->vdev_snr.dat_snr;

		OS_MEMCPY(vdev_stats->tx_frm_cnt, ev->tx_frm_cnt,
			sizeof(ev->tx_frm_cnt));
		vdev_stats->rx_frm_cnt = ev->rx_frm_cnt;
		OS_MEMCPY(vdev_stats->multiple_retry_cnt,
				ev->multiple_retry_cnt,
				sizeof(ev->multiple_retry_cnt));
		OS_MEMCPY(vdev_stats->fail_cnt, ev->fail_cnt,
				sizeof(ev->fail_cnt));
		vdev_stats->rts_fail_cnt = ev->rts_fail_cnt;
		vdev_stats->rts_succ_cnt = ev->rts_succ_cnt;
		vdev_stats->rx_err_cnt = ev->rx_err_cnt;
		vdev_stats->rx_discard_cnt = ev->rx_discard_cnt;
		vdev_stats->ack_fail_cnt = ev->ack_fail_cnt;
		OS_MEMCPY(vdev_stats->tx_rate_history, ev->tx_rate_history,
			sizeof(ev->tx_rate_history));
		OS_MEMCPY(vdev_stats->bcn_rssi_history, ev->bcn_rssi_history,
			sizeof(ev->bcn_rssi_history));

	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_stats_tlv() - extract peer stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into peer stats
 * @param peer_stats: Pointer to hold peer stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_peer_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_peer_stats *peer_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	data = (uint8_t *) param_buf->data;

	if (index < ev_param->num_peer_stats) {
		wmi_peer_stats *ev = (wmi_peer_stats *) ((data) +
			((ev_param->num_pdev_stats) * sizeof(wmi_pdev_stats)) +
			((ev_param->num_vdev_stats) * sizeof(wmi_vdev_stats)) +
			(index * sizeof(wmi_peer_stats)));

		OS_MEMSET(peer_stats, 0, sizeof(wmi_host_peer_stats));

		OS_MEMCPY(&(peer_stats->peer_macaddr),
			&(ev->peer_macaddr), sizeof(wmi_mac_addr));

		peer_stats->peer_rssi = ev->peer_rssi;
		peer_stats->peer_tx_rate = ev->peer_tx_rate;
		peer_stats->peer_rx_rate = ev->peer_rx_rate;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_bcnflt_stats_tlv() - extract bcn fault stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into bcn fault stats
 * @param bcnflt_stats: Pointer to hold bcn fault stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_bcnflt_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_bcnflt_stats *peer_stats)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_extd_stats_tlv() - extract extended peer stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended peer stats
 * @param peer_extd_stats: Pointer to hold extended peer stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_peer_extd_stats_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint32_t index,
		wmi_host_peer_extd_stats *peer_extd_stats)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_chan_stats_tlv() - extract chan stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into chan stats
 * @param vdev_extd_stats: Pointer to hold chan stats
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_chan_stats_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_chan_stats *chan_stats)
{
	WMI_UPDATE_STATS_EVENTID_param_tlvs *param_buf;
	wmi_stats_event_fixed_param *ev_param;
	uint8_t *data;

	param_buf = (WMI_UPDATE_STATS_EVENTID_param_tlvs *) evt_buf;
	ev_param = (wmi_stats_event_fixed_param *) param_buf->fixed_param;
	data = (uint8_t *) param_buf->data;

	if (index < ev_param->num_chan_stats) {
		wmi_chan_stats *ev = (wmi_chan_stats *) ((data) +
			((ev_param->num_pdev_stats) * sizeof(wmi_pdev_stats)) +
			((ev_param->num_vdev_stats) * sizeof(wmi_vdev_stats)) +
			((ev_param->num_peer_stats) * sizeof(wmi_peer_stats)) +
			(index * sizeof(wmi_chan_stats)));


		/* Non-TLV doesnt have num_chan_stats */
		chan_stats->chan_mhz = ev->chan_mhz;
		chan_stats->sampling_period_us = ev->sampling_period_us;
		chan_stats->rx_clear_count = ev->rx_clear_count;
		chan_stats->tx_duration_us = ev->tx_duration_us;
		chan_stats->rx_duration_us = ev->rx_duration_us;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_profile_ctx_tlv() - extract profile context from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @idx: profile stats index to extract
 * @param profile_ctx: Pointer to hold profile context
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_profile_ctx_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_wlan_profile_ctx_t *profile_ctx)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_profile_data_tlv() - extract profile data from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param profile_data: Pointer to hold profile data
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_profile_data_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint8_t idx, wmi_host_wlan_profile_t *profile_data)
{

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_chan_info_event_tlv() - extract chan information from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param chan_info: Pointer to hold chan information
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_chan_info_event_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_chan_info_event *chan_info)
{
	WMI_CHAN_INFO_EVENTID_param_tlvs *param_buf;
	wmi_chan_info_event_fixed_param *ev;

	param_buf = (WMI_CHAN_INFO_EVENTID_param_tlvs *) evt_buf;

	ev = (wmi_chan_info_event_fixed_param *) param_buf->fixed_param;
	if (!ev) {
		WMI_LOGE("%s: Failed to allocmemory\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	chan_info->err_code = ev->err_code;
	chan_info->freq = ev->freq;
	chan_info->cmd_flags = ev->cmd_flags;
	chan_info->noise_floor = ev->noise_floor;
	chan_info->rx_clear_count = ev->rx_clear_count;
	chan_info->cycle_count = ev->cycle_count;
	chan_info->pdev_id = wlan_get_pdev_id_from_vdev_id(
			(struct wlan_objmgr_psoc *)wmi_handle->soc->wmi_psoc,
			ev->vdev_id, WLAN_SCAN_ID);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_utf_event_tlv() - extract UTF data info from event
 * @wmi_handle: WMI handle
 * @param evt_buf: Pointer to event buffer
 * @param param: Pointer to hold data
 *
 * Return : QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_pdev_utf_event_tlv(wmi_unified_t wmi_handle,
			     uint8_t *evt_buf,
			     struct wmi_host_pdev_utf_event *event)
{
	WMI_PDEV_UTF_EVENTID_param_tlvs *param_buf;
	struct wmi_host_utf_seg_header_info *seg_hdr;

	param_buf = (WMI_PDEV_UTF_EVENTID_param_tlvs *)evt_buf;
	event->data = param_buf->data;
	event->datalen = param_buf->num_data;
	seg_hdr = (struct wmi_host_utf_seg_header_info *)param_buf->data;
	/* Set pdev_id=1 until FW adds support to include pdev_id */
	event->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							seg_hdr->pdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_chainmask_tables_tlv() - extract chain mask tables from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_chainmask_tables_tlv(wmi_unified_t wmi_handle,
		uint8_t *event, struct wlan_psoc_host_chainmask_table *chainmask_table)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	WMI_MAC_PHY_CHAINMASK_CAPABILITY *chainmask_caps;
	WMI_SOC_MAC_PHY_HW_MODE_CAPS *hw_caps;
	uint8_t i = 0, j = 0;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *) event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	hw_caps = param_buf->soc_hw_mode_caps;
	if (!hw_caps)
		return QDF_STATUS_E_INVAL;

	if (!hw_caps->num_chainmask_tables)
		return QDF_STATUS_E_INVAL;

	chainmask_caps = param_buf->mac_phy_chainmask_caps;

	if (chainmask_caps == NULL)
		return QDF_STATUS_E_INVAL;

	for (i = 0; i < hw_caps->num_chainmask_tables; i++) {

		qdf_print("Dumping chain mask combo data for table : %d\n", i);
		for (j = 0; j < chainmask_table[i].num_valid_chainmasks; j++) {

			chainmask_table[i].cap_list[j].chainmask =
				chainmask_caps->chainmask;

			chainmask_table[i].cap_list[j].supports_chan_width_20 =
				WMI_SUPPORT_CHAN_WIDTH_20_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].supports_chan_width_40 =
				WMI_SUPPORT_CHAN_WIDTH_40_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].supports_chan_width_80 =
				WMI_SUPPORT_CHAN_WIDTH_80_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].supports_chan_width_160 =
				WMI_SUPPORT_CHAN_WIDTH_160_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].supports_chan_width_80P80 =
				WMI_SUPPORT_CHAN_WIDTH_80P80_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].chain_mask_2G =
				WMI_SUPPORT_CHAIN_MASK_2G_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].chain_mask_5G =
				WMI_SUPPORT_CHAIN_MASK_5G_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].chain_mask_tx =
				WMI_SUPPORT_CHAIN_MASK_TX_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].chain_mask_rx =
				WMI_SUPPORT_CHAIN_MASK_RX_GET(chainmask_caps->supported_flags);

			chainmask_table[i].cap_list[j].supports_aDFS =
				WMI_SUPPORT_CHAIN_MASK_ADFS_GET(chainmask_caps->supported_flags);

			qdf_print("supported_flags: 0x%08x  chainmasks: 0x%08x\n",
					chainmask_caps->supported_flags,
					chainmask_caps->chainmask
				 );
			chainmask_caps++;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_service_ready_ext_tlv() - extract basic extended service ready params
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_service_ready_ext_tlv(wmi_unified_t wmi_handle,
		uint8_t *event, struct wlan_psoc_host_service_ext_param *param)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_ext_event_fixed_param *ev;
	WMI_SOC_MAC_PHY_HW_MODE_CAPS *hw_caps;
	WMI_SOC_HAL_REG_CAPABILITIES *reg_caps;
	WMI_MAC_PHY_CHAINMASK_COMBO *chain_mask_combo;
	uint8_t i = 0;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *) event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	ev = param_buf->fixed_param;
	if (!ev)
		return QDF_STATUS_E_INVAL;

	/* Move this to host based bitmap */
	param->default_conc_scan_config_bits =
				ev->default_conc_scan_config_bits;
	param->default_fw_config_bits = ev->default_fw_config_bits;
	param->he_cap_info = ev->he_cap_info;
	param->mpdu_density = ev->mpdu_density;
	param->max_bssid_rx_filters = ev->max_bssid_rx_filters;
	qdf_mem_copy(&param->ppet, &ev->ppet, sizeof(param->ppet));

	hw_caps = param_buf->soc_hw_mode_caps;
	if (hw_caps)
		param->num_hw_modes = hw_caps->num_hw_modes;
	else
		param->num_hw_modes = 0;

	reg_caps = param_buf->soc_hal_reg_caps;
	if (reg_caps)
		param->num_phy = reg_caps->num_phy;
	else
		param->num_phy = 0;

	if (hw_caps) {
		param->num_chainmask_tables = hw_caps->num_chainmask_tables;
		qdf_print("Num chain mask tables: %d\n", hw_caps->num_chainmask_tables);
	} else
		param->num_chainmask_tables = 0;

	chain_mask_combo = param_buf->mac_phy_chainmask_combo;

	if (chain_mask_combo == NULL)
		return QDF_STATUS_SUCCESS;

	qdf_print("Dumping chain mask combo data\n");

	for (i = 0; i < param->num_chainmask_tables; i++) {

		qdf_print("table_id : %d Num valid chainmasks: %d\n",
				chain_mask_combo->chainmask_table_id,
				chain_mask_combo->num_valid_chainmask
			 );

		param->chainmask_table[i].table_id =
			chain_mask_combo->chainmask_table_id;
		param->chainmask_table[i].num_valid_chainmasks =
			chain_mask_combo->num_valid_chainmask;
		chain_mask_combo++;
	}
	qdf_print("chain mask combo end\n");

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_hw_mode_cap_service_ready_ext_tlv() -
 *       extract HW mode cap from service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 * @param hw_mode_idx: hw mode idx should be less than num_mode
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_hw_mode_cap_service_ready_ext_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event, uint8_t hw_mode_idx,
			struct wlan_psoc_host_hw_mode_caps *param)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	WMI_SOC_MAC_PHY_HW_MODE_CAPS *hw_caps;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *) event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	hw_caps = param_buf->soc_hw_mode_caps;
	if (!hw_caps)
		return QDF_STATUS_E_INVAL;

	if (hw_mode_idx >= hw_caps->num_hw_modes)
		return QDF_STATUS_E_INVAL;

	param->hw_mode_id = param_buf->hw_mode_caps[hw_mode_idx].hw_mode_id;
	param->phy_id_map = param_buf->hw_mode_caps[hw_mode_idx].phy_id_map;

	param->hw_mode_config_type =
		param_buf->hw_mode_caps[hw_mode_idx].hw_mode_config_type;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_mac_phy_cap_service_ready_ext_tlv() -
 *       extract MAC phy cap from service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 * @param hw_mode_idx: hw mode idx should be less than num_mode
 * @param phy_id: phy id within hw_mode
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_mac_phy_cap_service_ready_ext_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event, uint8_t hw_mode_id, uint8_t phy_id,
			struct wlan_psoc_host_mac_phy_caps *param)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	WMI_MAC_PHY_CAPABILITIES *mac_phy_caps;
	WMI_SOC_MAC_PHY_HW_MODE_CAPS *hw_caps;
	uint32_t phy_map;
	uint8_t hw_idx, phy_idx = 0;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *) event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	hw_caps = param_buf->soc_hw_mode_caps;
	if (!hw_caps)
		return QDF_STATUS_E_INVAL;

	for (hw_idx = 0; hw_idx < hw_caps->num_hw_modes; hw_idx++) {
		if (hw_mode_id == param_buf->hw_mode_caps[hw_idx].hw_mode_id)
			break;

		phy_map = param_buf->hw_mode_caps[hw_idx].phy_id_map;
		while (phy_map) {
			phy_map >>= 1;
			phy_idx++;
		}
	}

	if (hw_idx == hw_caps->num_hw_modes)
		return QDF_STATUS_E_INVAL;

	phy_idx += phy_id;
	if (phy_idx >= param_buf->num_mac_phy_caps)
		return QDF_STATUS_E_INVAL;

	mac_phy_caps = &param_buf->mac_phy_caps[phy_idx];

	param->hw_mode_id = mac_phy_caps->hw_mode_id;
	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							mac_phy_caps->pdev_id);
	param->phy_id = mac_phy_caps->phy_id;
	param->supports_11b =
			WMI_SUPPORT_11B_GET(mac_phy_caps->supported_flags);
	param->supports_11g =
			WMI_SUPPORT_11G_GET(mac_phy_caps->supported_flags);
	param->supports_11a =
			WMI_SUPPORT_11A_GET(mac_phy_caps->supported_flags);
	param->supports_11n =
			WMI_SUPPORT_11N_GET(mac_phy_caps->supported_flags);
	param->supports_11ac =
			WMI_SUPPORT_11AC_GET(mac_phy_caps->supported_flags);
	param->supports_11ax =
			WMI_SUPPORT_11AX_GET(mac_phy_caps->supported_flags);

	param->supported_bands = mac_phy_caps->supported_bands;
	param->ampdu_density = mac_phy_caps->ampdu_density;
	param->max_bw_supported_2G = mac_phy_caps->max_bw_supported_2G;
	param->ht_cap_info_2G = mac_phy_caps->ht_cap_info_2G;
	param->vht_cap_info_2G = mac_phy_caps->vht_cap_info_2G;
	param->vht_supp_mcs_2G = mac_phy_caps->vht_supp_mcs_2G;
	param->he_cap_info_2G = mac_phy_caps->he_cap_info_2G;
	param->he_supp_mcs_2G = mac_phy_caps->he_supp_mcs_2G;
	param->tx_chain_mask_2G = mac_phy_caps->tx_chain_mask_2G;
	param->rx_chain_mask_2G = mac_phy_caps->rx_chain_mask_2G;
	param->max_bw_supported_5G = mac_phy_caps->max_bw_supported_5G;
	param->ht_cap_info_5G = mac_phy_caps->ht_cap_info_5G;
	param->vht_cap_info_5G = mac_phy_caps->vht_cap_info_5G;
	param->vht_supp_mcs_5G = mac_phy_caps->vht_supp_mcs_5G;
	param->he_cap_info_5G = mac_phy_caps->he_cap_info_5G;
	param->he_supp_mcs_5G = mac_phy_caps->he_supp_mcs_5G;
	param->tx_chain_mask_5G = mac_phy_caps->tx_chain_mask_5G;
	param->rx_chain_mask_5G = mac_phy_caps->rx_chain_mask_5G;
	qdf_mem_copy(&param->he_cap_phy_info_2G,
			&mac_phy_caps->he_cap_phy_info_2G,
			sizeof(param->he_cap_phy_info_2G));
	qdf_mem_copy(&param->he_cap_phy_info_5G,
			&mac_phy_caps->he_cap_phy_info_5G,
			sizeof(param->he_cap_phy_info_5G));
	qdf_mem_copy(&param->he_ppet2G, &mac_phy_caps->he_ppet2G,
				 sizeof(param->he_ppet2G));
	qdf_mem_copy(&param->he_ppet5G, &mac_phy_caps->he_ppet5G,
				sizeof(param->he_ppet5G));
	param->chainmask_table_id = mac_phy_caps->chainmask_table_id;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_reg_cap_service_ready_ext_tlv() -
 *       extract REG cap from service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold evt buf
 * @param phy_idx: phy idx should be less than num_mode
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_reg_cap_service_ready_ext_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *event, uint8_t phy_idx,
			struct wlan_psoc_host_hal_reg_capabilities_ext *param)
{
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	WMI_SOC_HAL_REG_CAPABILITIES *reg_caps;
	WMI_HAL_REG_CAPABILITIES_EXT *ext_reg_cap;

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *) event;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	reg_caps = param_buf->soc_hal_reg_caps;
	if (!reg_caps)
		return QDF_STATUS_E_INVAL;

	if (phy_idx >= reg_caps->num_phy)
		return QDF_STATUS_E_INVAL;

	ext_reg_cap = &param_buf->hal_reg_caps[phy_idx];

	param->phy_id = ext_reg_cap->phy_id;
	param->eeprom_reg_domain = ext_reg_cap->eeprom_reg_domain;
	param->eeprom_reg_domain_ext = ext_reg_cap->eeprom_reg_domain_ext;
	param->regcap1 = ext_reg_cap->regcap1;
	param->regcap2 = ext_reg_cap->regcap2;
	param->wireless_modes = convert_wireless_modes_tlv(
						ext_reg_cap->wireless_modes);
	param->low_2ghz_chan = ext_reg_cap->low_2ghz_chan;
	param->high_2ghz_chan = ext_reg_cap->high_2ghz_chan;
	param->low_5ghz_chan = ext_reg_cap->low_5ghz_chan;
	param->high_5ghz_chan = ext_reg_cap->high_5ghz_chan;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_dcs_interference_type_tlv() - extract dcs interference type
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold dcs interference param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_dcs_interference_type_tlv(
		wmi_unified_t wmi_handle,
		void *evt_buf, struct wmi_host_dcs_interference_param *param)
{
	WMI_DCS_INTERFERENCE_EVENTID_param_tlvs *param_buf;

	param_buf = (WMI_DCS_INTERFERENCE_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	param->interference_type = param_buf->fixed_param->interference_type;
	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
					param_buf->fixed_param->pdev_id);

	return QDF_STATUS_SUCCESS;
}

/*
 * extract_dcs_cw_int_tlv() - extract dcs cw interference from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param cw_int: Pointer to hold cw interference
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_dcs_cw_int_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_ath_dcs_cw_int *cw_int)
{
	WMI_DCS_INTERFERENCE_EVENTID_param_tlvs *param_buf;
	wlan_dcs_cw_int *ev;

	param_buf = (WMI_DCS_INTERFERENCE_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	ev = param_buf->cw_int;

	cw_int->channel = ev->channel;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_dcs_im_tgt_stats_tlv() - extract dcs im target stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param wlan_stat: Pointer to hold wlan stats
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_dcs_im_tgt_stats_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_dcs_im_tgt_stats_t *wlan_stat)
{
	WMI_DCS_INTERFERENCE_EVENTID_param_tlvs *param_buf;
	wlan_dcs_im_tgt_stats_t *ev;

	param_buf = (WMI_DCS_INTERFERENCE_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	ev = param_buf->wlan_stat;
	wlan_stat->reg_tsf32 = ev->reg_tsf32;
	wlan_stat->last_ack_rssi = ev->last_ack_rssi;
	wlan_stat->tx_waste_time = ev->tx_waste_time;
	wlan_stat->rx_time = ev->rx_time;
	wlan_stat->phyerr_cnt = ev->phyerr_cnt;
	wlan_stat->mib_stats.listen_time = ev->listen_time;
	wlan_stat->mib_stats.reg_tx_frame_cnt = ev->reg_tx_frame_cnt;
	wlan_stat->mib_stats.reg_rx_frame_cnt = ev->reg_rx_frame_cnt;
	wlan_stat->mib_stats.reg_rxclr_cnt = ev->reg_rxclr_cnt;
	wlan_stat->mib_stats.reg_cycle_cnt = ev->reg_cycle_cnt;
	wlan_stat->mib_stats.reg_rxclr_ext_cnt = ev->reg_rxclr_ext_cnt;
	wlan_stat->mib_stats.reg_ofdm_phyerr_cnt = ev->reg_ofdm_phyerr_cnt;
	wlan_stat->mib_stats.reg_cck_phyerr_cnt = ev->reg_cck_phyerr_cnt;
	wlan_stat->chan_nf = ev->chan_nf;
	wlan_stat->my_bss_rx_cycle_count = ev->my_bss_rx_cycle_count;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_thermal_stats_tlv() - extract thermal stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param temp: Pointer to hold extracted temperature
 * @param level: Pointer to hold extracted level
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
extract_thermal_stats_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint32_t *temp,
		uint32_t *level, uint32_t *pdev_id)
{
	WMI_THERM_THROT_STATS_EVENTID_param_tlvs *param_buf;
	wmi_therm_throt_stats_event_fixed_param *tt_stats_event;

	param_buf =
		(WMI_THERM_THROT_STATS_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	tt_stats_event = param_buf->fixed_param;

	*pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
						tt_stats_event->pdev_id);
	*temp = tt_stats_event->temp;
	*level = tt_stats_event->level;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_thermal_level_stats_tlv() - extract thermal level stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index to level stats
 * @param levelcount: Pointer to hold levelcount
 * @param dccount: Pointer to hold dccount
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
extract_thermal_level_stats_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint8_t idx, uint32_t *levelcount,
		uint32_t *dccount)
{
	WMI_THERM_THROT_STATS_EVENTID_param_tlvs *param_buf;
	wmi_therm_throt_level_stats_info *tt_level_info;

	param_buf =
		(WMI_THERM_THROT_STATS_EVENTID_param_tlvs *) evt_buf;
	if (!param_buf)
		return QDF_STATUS_E_INVAL;

	tt_level_info = param_buf->therm_throt_level_stats_info;

	if (idx < THERMAL_LEVELS) {
		*levelcount = tt_level_info[idx].level_count;
		*dccount = tt_level_info[idx].dc_count;
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}
#ifdef BIG_ENDIAN_HOST
/**
 * fips_conv_data_be() - LE to BE conversion of FIPS ev data
 * @param data_len - data length
 * @param data - pointer to data
 *
 * Return: QDF_STATUS - success or error status
 */
static QDF_STATUS fips_conv_data_be(uint32_t data_len, uint8_t *data)
{
	uint8_t *data_aligned = NULL;
	int c;
	unsigned char *data_unaligned;

	data_unaligned = qdf_mem_malloc(((sizeof(uint8_t) * data_len) +
					FIPS_ALIGN));
	/* Assigning unaligned space to copy the data */
	/* Checking if kmalloc does succesful allocation */
	if (data_unaligned == NULL)
		return QDF_STATUS_E_FAILURE;

	/* Checking if space is alligned */
	if (!FIPS_IS_ALIGNED(data_unaligned, FIPS_ALIGN)) {
		/* align the data space */
		data_aligned =
			(uint8_t *)FIPS_ALIGNTO(data_unaligned, FIPS_ALIGN);
	} else {
		data_aligned = (u_int8_t *)data_unaligned;
	}

	/* memset and copy content from data to data aligned */
	OS_MEMSET(data_aligned, 0, data_len);
	OS_MEMCPY(data_aligned, data, data_len);
	/* Endianness to LE */
	for (c = 0; c < data_len/4; c++) {
		*((u_int32_t *)data_aligned + c) =
			qdf_os_le32_to_cpu(*((u_int32_t *)data_aligned + c));
	}

	/* Copy content to event->data */
	OS_MEMCPY(data, data_aligned, data_len);

	/* clean up allocated space */
	qdf_mem_free(data_unaligned);
	data_aligned = NULL;
	data_unaligned = NULL;

	/*************************************************************/

	return QDF_STATUS_SUCCESS;
}
#else
/**
 * fips_conv_data_be() - DUMMY for LE platform
 *
 * Return: QDF_STATUS - success
 */
static QDF_STATUS fips_conv_data_be(uint32_t data_len, uint8_t *data)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * extract_fips_event_data_tlv() - extract fips event data
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: pointer FIPS event params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_fips_event_data_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, struct wmi_host_fips_event_param *param)
{
	WMI_PDEV_FIPS_EVENTID_param_tlvs *param_buf;
	wmi_pdev_fips_event_fixed_param *event;

	param_buf = (WMI_PDEV_FIPS_EVENTID_param_tlvs *) evt_buf;
	event = (wmi_pdev_fips_event_fixed_param *) param_buf->fixed_param;

	if (fips_conv_data_be(event->data_len, param_buf->data) !=
							QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	param->data = (uint32_t *)param_buf->data;
	param->data_len = event->data_len;
	param->error_status = event->error_status;
	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
								event->pdev_id);

	return QDF_STATUS_SUCCESS;
}

/*
 * extract_peer_delete_response_event_tlv() - extract peer delete response event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_id: Pointer to hold vdev_id
 * @param mac_addr: Pointer to hold peer mac address
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_peer_delete_response_event_tlv(wmi_unified_t wmi_hdl,
	void *evt_buf, struct wmi_host_peer_delete_response_event *param)
{
	WMI_PEER_DELETE_RESP_EVENTID_param_tlvs *param_buf;
	wmi_peer_delete_resp_event_fixed_param *ev;

	param_buf = (WMI_PEER_DELETE_RESP_EVENTID_param_tlvs *)evt_buf;

	ev = (wmi_peer_delete_resp_event_fixed_param *) param_buf->fixed_param;
	if (!ev) {
		WMI_LOGE("%s: Invalid peer_delete response\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	param->vdev_id = ev->vdev_id;
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->peer_macaddr,
			&param->mac_address.bytes[0]);

	return QDF_STATUS_SUCCESS;
}

static bool is_management_record_tlv(uint32_t cmd_id)
{
	if (cmd_id == WMI_MGMT_TX_COMPLETION_EVENTID)
		return true;

	return false;
}

static uint16_t wmi_tag_vdev_set_cmd(wmi_unified_t wmi_hdl, wmi_buf_t buf)
{
	wmi_vdev_set_param_cmd_fixed_param *set_cmd;

	set_cmd = (wmi_vdev_set_param_cmd_fixed_param *)wmi_buf_data(buf);

	switch (set_cmd->param_id) {
	case WMI_VDEV_PARAM_LISTEN_INTERVAL:
	case WMI_VDEV_PARAM_DTIM_POLICY:
		return HTC_TX_PACKET_TAG_AUTO_PM;
	default:
		break;
	}

	return 0;
}

static uint16_t wmi_tag_sta_powersave_cmd(wmi_unified_t wmi_hdl, wmi_buf_t buf)
{
	wmi_sta_powersave_param_cmd_fixed_param *ps_cmd;

	ps_cmd = (wmi_sta_powersave_param_cmd_fixed_param *)wmi_buf_data(buf);

	switch (ps_cmd->param) {
	case WMI_STA_PS_PARAM_TX_WAKE_THRESHOLD:
	case WMI_STA_PS_PARAM_INACTIVITY_TIME:
	case WMI_STA_PS_ENABLE_QPOWER:
		return HTC_TX_PACKET_TAG_AUTO_PM;
	default:
		break;
	}

	return 0;
}

static uint16_t wmi_tag_common_cmd(wmi_unified_t wmi_hdl, wmi_buf_t buf,
				   uint32_t cmd_id)
{
	if (qdf_atomic_read(&wmi_hdl->is_wow_bus_suspended))
		return 0;

	switch (cmd_id) {
	case WMI_VDEV_SET_PARAM_CMDID:
		return wmi_tag_vdev_set_cmd(wmi_hdl, buf);
	case WMI_STA_POWERSAVE_PARAM_CMDID:
		return wmi_tag_sta_powersave_cmd(wmi_hdl, buf);
	default:
		break;
	}

	return 0;
}

static uint16_t wmi_tag_fw_hang_cmd(wmi_unified_t wmi_handle)
{
	uint16_t tag = 0;

	if (qdf_atomic_read(&wmi_handle->is_target_suspended)) {
		pr_err("%s: Target is already suspended, Ignore FW Hang Command\n",
			__func__);
		return tag;
	}

	if (wmi_handle->tag_crash_inject)
		tag = HTC_TX_PACKET_TAG_AUTO_PM;

	wmi_handle->tag_crash_inject = false;
	return tag;
}

/**
 * wmi_set_htc_tx_tag_tlv() - set HTC TX tag for WMI commands
 * @wmi_handle: WMI handle
 * @buf:	WMI buffer
 * @cmd_id:	WMI command Id
 *
 * Return htc_tx_tag
 */
static uint16_t wmi_set_htc_tx_tag_tlv(wmi_unified_t wmi_handle,
				wmi_buf_t buf,
				uint32_t cmd_id)
{
	uint16_t htc_tx_tag = 0;

	switch (cmd_id) {
	case WMI_WOW_ENABLE_CMDID:
	case WMI_PDEV_SUSPEND_CMDID:
	case WMI_WOW_ENABLE_DISABLE_WAKE_EVENT_CMDID:
	case WMI_WOW_ADD_WAKE_PATTERN_CMDID:
	case WMI_WOW_HOSTWAKEUP_FROM_SLEEP_CMDID:
	case WMI_PDEV_RESUME_CMDID:
	case WMI_WOW_DEL_WAKE_PATTERN_CMDID:
	case WMI_WOW_SET_ACTION_WAKE_UP_CMDID:
#ifdef FEATURE_WLAN_D0WOW
	case WMI_D0_WOW_ENABLE_DISABLE_CMDID:
#endif
		htc_tx_tag = HTC_TX_PACKET_TAG_AUTO_PM;
		break;
	case WMI_FORCE_FW_HANG_CMDID:
		htc_tx_tag = wmi_tag_fw_hang_cmd(wmi_handle);
		break;
	case WMI_VDEV_SET_PARAM_CMDID:
	case WMI_STA_POWERSAVE_PARAM_CMDID:
		htc_tx_tag = wmi_tag_common_cmd(wmi_handle, buf, cmd_id);
	default:
		break;
	}

	return htc_tx_tag;
}

/**
 * extract_channel_hopping_event_tlv() - extract channel hopping param
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ch_hopping: Pointer to hold channel hopping param
 *
 * @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS extract_channel_hopping_event_tlv(
	wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_pdev_channel_hopping_event *ch_hopping)
{
	WMI_PDEV_CHANNEL_HOPPING_EVENTID_param_tlvs *param_buf;
	wmi_pdev_channel_hopping_event_fixed_param *event;

	param_buf = (WMI_PDEV_CHANNEL_HOPPING_EVENTID_param_tlvs *)evt_buf;
	event = (wmi_pdev_channel_hopping_event_fixed_param *)
						param_buf->fixed_param;

	ch_hopping->noise_floor_report_iter = event->noise_floor_report_iter;
	ch_hopping->noise_floor_total_iter = event->noise_floor_total_iter;
	ch_hopping->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
								event->pdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_tpc_ev_param_tlv() - extract tpc param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold tpc param
 *
 * @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS extract_pdev_tpc_ev_param_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_pdev_tpc_event *param)
{
	WMI_PDEV_TPC_EVENTID_param_tlvs *param_buf;
	wmi_pdev_tpc_event_fixed_param *event;

	param_buf = (WMI_PDEV_TPC_EVENTID_param_tlvs *)evt_buf;
	event = (wmi_pdev_tpc_event_fixed_param *)param_buf->fixed_param;

	param->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
								event->pdev_id);
	qdf_mem_copy(param->tpc, param_buf->tpc, sizeof(param->tpc));

	return QDF_STATUS_SUCCESS;
}


#ifdef BIG_ENDIAN_HOST
/**
 * wds_addr_ev_conv_data_be() - LE to BE conversion of wds addr event
 * @param data_len - data length
 * @param data - pointer to data
 *
 * Return: QDF_STATUS - success or error status
 */
static QDF_STATUS wds_addr_ev_conv_data_be(uint16_t data_len, uint8_t *ev)
{
	uint8_t *datap = (uint8_t *)ev;
	/* Skip swapping the first word */
	datap += sizeof(uint32_t);
	for (i = 0; i < ((data_len / sizeof(uint32_t))-1);
			i++, datap += sizeof(uint32_t)) {
		*(uint32_t *)datap = qdf_le32_to_cpu(*(uint32_t *)datap);
	}

	return QDF_STATUS_SUCCESS;
}
#else
/**
 * wds_addr_ev_conv_data_be() - Dummy operation for LE platforms
 * @param data_len - data length
 * @param data - pointer to data
 *
 * Return: QDF_STATUS - success or error status
 */
static QDF_STATUS wds_addr_ev_conv_data_be(uint32_t data_len, uint8_t *ev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * extract_wds_addr_event_tlv() - extract wds address from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param wds_ev: Pointer to hold wds address
 *
 * @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS extract_wds_addr_event_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		uint16_t len, wds_addr_event_t *wds_ev)
{
	WMI_WDS_PEER_EVENTID_param_tlvs *param_buf;
	wmi_wds_addr_event_fixed_param *ev;
	int i;

	param_buf = (WMI_WDS_PEER_EVENTID_param_tlvs *)evt_buf;
	ev = (wmi_wds_addr_event_fixed_param *)param_buf->fixed_param;

	if (wds_addr_ev_conv_data_be(len, (uint8_t *)ev) != QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_copy(wds_ev->event_type, ev->event_type,
		     sizeof(wds_ev->event_type));
	for (i = 0; i < 4; i++) {
		wds_ev->peer_mac[i] =
			((u_int8_t *)&(ev->peer_mac.mac_addr31to0))[i];
		wds_ev->dest_mac[i] =
			((u_int8_t *)&(ev->dest_mac.mac_addr31to0))[i];
	}
	for (i = 0; i < 2; i++) {
		wds_ev->peer_mac[4+i] =
			((u_int8_t *)&(ev->peer_mac.mac_addr47to32))[i];
		wds_ev->dest_mac[4+i] =
			((u_int8_t *)&(ev->dest_mac.mac_addr47to32))[i];
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_sta_ps_statechange_ev_tlv() - extract peer sta ps state
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold peer param and ps state
 *
 * @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS extract_peer_sta_ps_statechange_ev_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_peer_sta_ps_statechange_event *ev)
{
	WMI_PEER_STA_PS_STATECHG_EVENTID_param_tlvs *param_buf;
	wmi_peer_sta_ps_statechange_event_fixed_param *event;

	param_buf = (WMI_PEER_STA_PS_STATECHG_EVENTID_param_tlvs *)evt_buf;
	event = (wmi_peer_sta_ps_statechange_event_fixed_param *)
						param_buf->fixed_param;

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&event->peer_macaddr, ev->peer_macaddr);
	ev->peer_ps_state = event->peer_ps_state;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_inst_rssi_stats_event_tlv() - extract inst rssi stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param inst_rssi_resp: Pointer to hold inst rssi response
 *
 * @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS extract_inst_rssi_stats_event_tlv(
	wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_inst_stats_resp *inst_rssi_resp)
{
	WMI_INST_RSSI_STATS_EVENTID_param_tlvs *param_buf;
	wmi_inst_rssi_stats_resp_fixed_param *event;

	param_buf = (WMI_INST_RSSI_STATS_EVENTID_param_tlvs *)evt_buf;
	event = (wmi_inst_rssi_stats_resp_fixed_param *)param_buf->fixed_param;

	qdf_mem_copy(&(inst_rssi_resp->peer_macaddr),
		     &(event->peer_macaddr), sizeof(wmi_mac_addr));
	inst_rssi_resp->iRSSI = event->iRSSI;

	return QDF_STATUS_SUCCESS;
}

static struct cur_reg_rule
*create_reg_rules_from_wmi(uint32_t num_reg_rules,
		wmi_regulatory_rule_struct *wmi_reg_rule)
{
	struct cur_reg_rule *reg_rule_ptr;
	uint32_t count;

	reg_rule_ptr = qdf_mem_malloc(num_reg_rules * sizeof(*reg_rule_ptr));

	if (NULL == reg_rule_ptr) {
		WMI_LOGE("memory allocation failure");
		return NULL;
	}

	for (count = 0; count < num_reg_rules; count++) {
		reg_rule_ptr[count].start_freq =
			WMI_REG_RULE_START_FREQ_GET(
					wmi_reg_rule[count].freq_info);
		reg_rule_ptr[count].end_freq =
			WMI_REG_RULE_END_FREQ_GET(
					wmi_reg_rule[count].freq_info);
		reg_rule_ptr[count].max_bw =
			WMI_REG_RULE_MAX_BW_GET(
					wmi_reg_rule[count].bw_pwr_info);
		reg_rule_ptr[count].reg_power =
			WMI_REG_RULE_REG_POWER_GET(
					wmi_reg_rule[count].bw_pwr_info);
		reg_rule_ptr[count].ant_gain =
			WMI_REG_RULE_ANTENNA_GAIN_GET(
					wmi_reg_rule[count].bw_pwr_info);
		reg_rule_ptr[count].flags =
			WMI_REG_RULE_FLAGS_GET(
					wmi_reg_rule[count].flag_info);
	}

	return reg_rule_ptr;
}

static QDF_STATUS extract_reg_chan_list_update_event_tlv(
	wmi_unified_t wmi_handle, uint8_t *evt_buf,
	struct cur_regulatory_info *reg_info, uint32_t len)
{
	WMI_REG_CHAN_LIST_CC_EVENTID_param_tlvs *param_buf;
	wmi_reg_chan_list_cc_event_fixed_param *chan_list_event_hdr;
	wmi_regulatory_rule_struct *wmi_reg_rule;
	uint32_t num_2g_reg_rules, num_5g_reg_rules;

	WMI_LOGD("processing regulatory channel list");

	param_buf = (WMI_REG_CHAN_LIST_CC_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		WMI_LOGE("invalid channel list event buf");
		return QDF_STATUS_E_FAILURE;
	}

	chan_list_event_hdr = param_buf->fixed_param;

	reg_info->num_2g_reg_rules = chan_list_event_hdr->num_2g_reg_rules;
	reg_info->num_5g_reg_rules = chan_list_event_hdr->num_5g_reg_rules;
	qdf_mem_copy(reg_info->alpha2, &(chan_list_event_hdr->alpha2),
			REG_ALPHA2_LEN);
	reg_info->dfs_region = chan_list_event_hdr->dfs_region;
	reg_info->phybitmap = chan_list_event_hdr->phybitmap;
	reg_info->offload_enabled = true;
	reg_info->num_phy = chan_list_event_hdr->num_phy;
	reg_info->ctry_code = chan_list_event_hdr->country_id;
	reg_info->reg_dmn_pair = chan_list_event_hdr->domain_code;
	if (chan_list_event_hdr->status_code == WMI_REG_SET_CC_STATUS_PASS)
		reg_info->status_code = REG_SET_CC_STATUS_PASS;
	else if (chan_list_event_hdr->status_code ==
		 WMI_REG_CURRENT_ALPHA2_NOT_FOUND)
		reg_info->status_code = REG_CURRENT_ALPHA2_NOT_FOUND;
	else if (chan_list_event_hdr->status_code ==
		 WMI_REG_INIT_ALPHA2_NOT_FOUND)
		reg_info->status_code = REG_INIT_ALPHA2_NOT_FOUND;
	else if (chan_list_event_hdr->status_code ==
		 WMI_REG_SET_CC_CHANGE_NOT_ALLOWED)
		reg_info->status_code = REG_SET_CC_CHANGE_NOT_ALLOWED;
	else if (chan_list_event_hdr->status_code ==
		 WMI_REG_SET_CC_STATUS_NO_MEMORY)
		reg_info->status_code = REG_SET_CC_STATUS_NO_MEMORY;
	else if (chan_list_event_hdr->status_code ==
		 WMI_REG_SET_CC_STATUS_FAIL)
		reg_info->status_code = REG_SET_CC_STATUS_FAIL;

	reg_info->min_bw_2g = chan_list_event_hdr->min_bw_2g;
	reg_info->max_bw_2g = chan_list_event_hdr->max_bw_2g;
	reg_info->min_bw_5g = chan_list_event_hdr->min_bw_5g;
	reg_info->max_bw_5g = chan_list_event_hdr->max_bw_5g;

	num_2g_reg_rules = reg_info->num_2g_reg_rules;
	num_5g_reg_rules = reg_info->num_5g_reg_rules;

	WMI_LOGD("%s:cc %s dsf %d BW: min_2g %d max_2g %d min_5g %d max_5g %d",
			__func__, reg_info->alpha2, reg_info->dfs_region,
			reg_info->min_bw_2g, reg_info->max_bw_2g,
			reg_info->min_bw_5g, reg_info->max_bw_5g);

	WMI_LOGD("%s: num_2g_reg_rules %d num_5g_reg_rules %d", __func__,
			num_2g_reg_rules, num_5g_reg_rules);
	wmi_reg_rule =
		(wmi_regulatory_rule_struct *)((uint8_t *)chan_list_event_hdr
			+ sizeof(wmi_reg_chan_list_cc_event_fixed_param)
			+ WMI_TLV_HDR_SIZE);
	reg_info->reg_rules_2g_ptr = create_reg_rules_from_wmi(num_2g_reg_rules,
			wmi_reg_rule);
	wmi_reg_rule += num_2g_reg_rules;

	reg_info->reg_rules_5g_ptr = create_reg_rules_from_wmi(num_5g_reg_rules,
			wmi_reg_rule);

	WMI_LOGD("processed regulatory channel list");

	return QDF_STATUS_SUCCESS;
}

#ifdef DFS_COMPONENT_ENABLE
/**
 * extract_dfs_cac_complete_event_tlv() - extract cac complete event
 * @wmi_handle: wma handle
 * @evt_buf: event buffer
 * @vdev_id: vdev id
 * @len: length of buffer
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_dfs_cac_complete_event_tlv(wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		uint32_t *vdev_id,
		uint32_t len)
{
	WMI_VDEV_DFS_CAC_COMPLETE_EVENTID_param_tlvs *param_tlvs;
	wmi_vdev_dfs_cac_complete_event_fixed_param  *cac_event;

	param_tlvs = (WMI_VDEV_DFS_CAC_COMPLETE_EVENTID_param_tlvs *) evt_buf;
	if (!param_tlvs) {
		WMI_LOGE("invalid cac complete event buf");
		return QDF_STATUS_E_FAILURE;
	}

	cac_event = param_tlvs->fixed_param;
	*vdev_id = cac_event->vdev_id;
	WMI_LOGD("processed cac complete event vdev %d", *vdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_dfs_radar_detection_event_tlv() - extract radar found event
 * @wmi_handle: wma handle
 * @evt_buf: event buffer
 * @radar_found: radar found event info
 * @len: length of buffer
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_dfs_radar_detection_event_tlv(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct radar_found_info *radar_found,
		uint32_t len)
{
	WMI_PDEV_DFS_RADAR_DETECTION_EVENTID_param_tlvs *param_tlv;
	wmi_pdev_dfs_radar_detection_event_fixed_param *radar_event;

	param_tlv = (WMI_PDEV_DFS_RADAR_DETECTION_EVENTID_param_tlvs *) evt_buf;
	if (!param_tlv) {
		WMI_LOGE("invalid radar detection event buf");
		return QDF_STATUS_E_FAILURE;
	}

	radar_event = param_tlv->fixed_param;
	radar_found->pdev_id = wmi_handle->ops->convert_pdev_id_target_to_host(
							radar_event->pdev_id);
	radar_found->detection_mode = radar_event->detection_mode;
	radar_found->freq_offset = radar_event->chan_freq;
	radar_found->chan_width = radar_event->chan_width;
	radar_found->detector_id = radar_event->detector_id;
	radar_found->segment_id = radar_event->segment_id;
	radar_found->timestamp = radar_event->timestamp;
	radar_found->is_chirp = radar_event->is_chirp;

	WMI_LOGD("processed radar found event pdev %d", radar_event->pdev_id);

	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * convert_host_pdev_id_to_target_pdev_id_legacy() - Convert pdev_id from
 *           host to target defines. For legacy there is not conversion
 *           required. Just return pdev_id as it is.
 * @param pdev_id: host pdev_id to be converted.
 * Return: target pdev_id after conversion.
 */
static uint32_t convert_host_pdev_id_to_target_pdev_id_legacy(
							uint32_t pdev_id)
{
	if (pdev_id == WMI_HOST_PDEV_ID_SOC)
		return WMI_PDEV_ID_SOC;

	/*No conversion required*/
	return pdev_id;
}

/**
 * convert_target_pdev_id_to_host_pdev_id_legacy() - Convert pdev_id from
 *           target to host defines. For legacy there is not conversion
 *           required. Just return pdev_id as it is.
 * @param pdev_id: target pdev_id to be converted.
 * Return: host pdev_id after conversion.
 */
static uint32_t convert_target_pdev_id_to_host_pdev_id_legacy(
							uint32_t pdev_id)
{
	/*No conversion required*/
	return pdev_id;
}

/**
 *  send_set_country_cmd_tlv() - WMI scan channel list function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to hold scan channel list parameter
 *
 *  Return: 0  on success and -ve on failure.
 */
static QDF_STATUS send_set_country_cmd_tlv(wmi_unified_t wmi_handle,
				struct set_country *params)
{
	wmi_buf_t buf;
	QDF_STATUS qdf_status;
	wmi_set_current_country_cmd_fixed_param *cmd;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("Failed to allocate memory");
		qdf_status = QDF_STATUS_E_NOMEM;
		goto end;
	}

	cmd = (wmi_set_current_country_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_set_current_country_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_set_current_country_cmd_fixed_param));

	WMI_LOGD("setting cuurnet country to  %s", params->country);

	qdf_mem_copy((uint8_t *)&cmd->new_alpha2, params->country, 3);

	cmd->pdev_id = params->pdev_id;

	qdf_status = wmi_unified_cmd_send(wmi_handle,
			buf, len, WMI_SET_CURRENT_COUNTRY_CMDID);

	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		WMI_LOGE("Failed to send WMI_SET_CURRENT_COUNTRY_CMDID");
		wmi_buf_free(buf);
	}

end:
	return qdf_status;
}

struct wmi_ops tlv_ops =  {
	.send_vdev_create_cmd = send_vdev_create_cmd_tlv,
	.send_vdev_delete_cmd = send_vdev_delete_cmd_tlv,
	.send_vdev_down_cmd = send_vdev_down_cmd_tlv,
	.send_vdev_start_cmd = send_vdev_start_cmd_tlv,
	.send_hidden_ssid_vdev_restart_cmd =
		send_hidden_ssid_vdev_restart_cmd_tlv,
	.send_peer_flush_tids_cmd = send_peer_flush_tids_cmd_tlv,
	.send_peer_param_cmd = send_peer_param_cmd_tlv,
	.send_vdev_up_cmd = send_vdev_up_cmd_tlv,
	.send_vdev_stop_cmd = send_vdev_stop_cmd_tlv,
	.send_peer_create_cmd = send_peer_create_cmd_tlv,
	.send_peer_delete_cmd = send_peer_delete_cmd_tlv,
	.send_peer_rx_reorder_queue_setup_cmd =
		send_peer_rx_reorder_queue_setup_cmd_tlv,
	.send_peer_rx_reorder_queue_remove_cmd =
		send_peer_rx_reorder_queue_remove_cmd_tlv,
	.send_peer_add_wds_entry_cmd = send_peer_add_wds_entry_cmd_tlv,
	.send_peer_del_wds_entry_cmd = send_peer_del_wds_entry_cmd_tlv,
	.send_peer_update_wds_entry_cmd = send_peer_update_wds_entry_cmd_tlv,
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
#ifndef CONFIG_MCL
	.send_beacon_tmpl_send_cmd = send_beacon_tmpl_send_cmd_tlv,
#endif
	.send_peer_assoc_cmd = send_peer_assoc_cmd_tlv,
	.send_scan_start_cmd = send_scan_start_cmd_tlv,
	.send_scan_stop_cmd = send_scan_stop_cmd_tlv,
	.send_scan_chan_list_cmd = send_scan_chan_list_cmd_tlv,
	.send_mgmt_cmd = send_mgmt_cmd_tlv,
	.send_offchan_data_tx_cmd = send_offchan_data_tx_cmd_tlv,
	.send_modem_power_state_cmd = send_modem_power_state_cmd_tlv,
	.send_set_sta_ps_mode_cmd = send_set_sta_ps_mode_cmd_tlv,
	.send_set_sta_uapsd_auto_trig_cmd =
		send_set_sta_uapsd_auto_trig_cmd_tlv,
	.send_get_temperature_cmd = send_get_temperature_cmd_tlv,
	.send_set_p2pgo_oppps_req_cmd = send_set_p2pgo_oppps_req_cmd_tlv,
	.send_set_p2pgo_noa_req_cmd = send_set_p2pgo_noa_req_cmd_tlv,
#ifdef CONVERGED_P2P_ENABLE
	.send_p2p_lo_start_cmd = send_p2p_lo_start_cmd_tlv,
	.send_p2p_lo_stop_cmd = send_p2p_lo_stop_cmd_tlv,
#endif
	.send_set_smps_params_cmd = send_set_smps_params_cmd_tlv,
	.send_set_mimops_cmd = send_set_mimops_cmd_tlv,
	.send_ocb_set_utc_time_cmd = send_ocb_set_utc_time_cmd_tlv,
	.send_ocb_get_tsf_timer_cmd = send_ocb_get_tsf_timer_cmd_tlv,
	.send_dcc_clear_stats_cmd = send_dcc_clear_stats_cmd_tlv,
	.send_dcc_get_stats_cmd = send_dcc_get_stats_cmd_tlv,
	.send_dcc_update_ndl_cmd = send_dcc_update_ndl_cmd_tlv,
	.send_ocb_set_config_cmd = send_ocb_set_config_cmd_tlv,
	.send_ocb_stop_timing_advert_cmd = send_ocb_stop_timing_advert_cmd_tlv,
	.send_ocb_start_timing_advert_cmd =
		send_ocb_start_timing_advert_cmd_tlv,
	.send_set_enable_disable_mcc_adaptive_scheduler_cmd =
		 send_set_enable_disable_mcc_adaptive_scheduler_cmd_tlv,
	.send_set_mcc_channel_time_latency_cmd =
			 send_set_mcc_channel_time_latency_cmd_tlv,
	.send_set_mcc_channel_time_quota_cmd =
			 send_set_mcc_channel_time_quota_cmd_tlv,
	.send_set_thermal_mgmt_cmd = send_set_thermal_mgmt_cmd_tlv,
	.send_lro_config_cmd = send_lro_config_cmd_tlv,
	.send_peer_rate_report_cmd = send_peer_rate_report_cmd_tlv,
	.send_set_sta_sa_query_param_cmd = send_set_sta_sa_query_param_cmd_tlv,
	.send_set_sta_keep_alive_cmd = send_set_sta_keep_alive_cmd_tlv,
	.send_vdev_set_gtx_cfg_cmd = send_vdev_set_gtx_cfg_cmd_tlv,
	.send_probe_rsp_tmpl_send_cmd =
				send_probe_rsp_tmpl_send_cmd_tlv,
	.send_p2p_go_set_beacon_ie_cmd =
				send_p2p_go_set_beacon_ie_cmd_tlv,
	.send_setup_install_key_cmd =
				send_setup_install_key_cmd_tlv,
	.send_set_gateway_params_cmd =
				send_set_gateway_params_cmd_tlv,
	.send_set_rssi_monitoring_cmd =
			 send_set_rssi_monitoring_cmd_tlv,
	.send_scan_probe_setoui_cmd =
				send_scan_probe_setoui_cmd_tlv,
	.send_reset_passpoint_network_list_cmd =
				send_reset_passpoint_network_list_cmd_tlv,
	.send_set_passpoint_network_list_cmd =
			 send_set_passpoint_network_list_cmd_tlv,
	.send_roam_scan_offload_rssi_thresh_cmd =
			send_roam_scan_offload_rssi_thresh_cmd_tlv,
	.send_roam_scan_filter_cmd =
			send_roam_scan_filter_cmd_tlv,
	.send_set_epno_network_list_cmd =
			 send_set_epno_network_list_cmd_tlv,
	.send_ipa_offload_control_cmd =
			 send_ipa_offload_control_cmd_tlv,
	.send_extscan_get_capabilities_cmd =
			 send_extscan_get_capabilities_cmd_tlv,
	.send_extscan_get_cached_results_cmd =
		 send_extscan_get_cached_results_cmd_tlv,
	.send_extscan_stop_change_monitor_cmd =
		  send_extscan_stop_change_monitor_cmd_tlv,
	.send_extscan_start_change_monitor_cmd =
		  send_extscan_start_change_monitor_cmd_tlv,
	.send_extscan_stop_hotlist_monitor_cmd =
		  send_extscan_stop_hotlist_monitor_cmd_tlv,
	.send_stop_extscan_cmd = send_stop_extscan_cmd_tlv,
	.send_start_extscan_cmd = send_start_extscan_cmd_tlv,
	.send_plm_stop_cmd = send_plm_stop_cmd_tlv,
	.send_plm_start_cmd = send_plm_start_cmd_tlv,
	.send_pno_stop_cmd = send_pno_stop_cmd_tlv,
	.send_pno_start_cmd = send_pno_start_cmd_tlv,
	.send_set_ric_req_cmd = send_set_ric_req_cmd_tlv,
	.send_process_ll_stats_clear_cmd = send_process_ll_stats_clear_cmd_tlv,
	.send_process_ll_stats_set_cmd = send_process_ll_stats_set_cmd_tlv,
	.send_process_ll_stats_get_cmd = send_process_ll_stats_get_cmd_tlv,
	.send_get_stats_cmd = send_get_stats_cmd_tlv,
	.send_congestion_cmd = send_congestion_cmd_tlv,
	.send_snr_request_cmd = send_snr_request_cmd_tlv,
	.send_snr_cmd = send_snr_cmd_tlv,
	.send_link_status_req_cmd = send_link_status_req_cmd_tlv,
#ifdef WLAN_PMO_ENABLE
	.send_add_wow_wakeup_event_cmd = send_add_wow_wakeup_event_cmd_tlv,
	.send_wow_patterns_to_fw_cmd = send_wow_patterns_to_fw_cmd_tlv,
	.send_enable_arp_ns_offload_cmd = send_enable_arp_ns_offload_cmd_tlv,
	.send_add_clear_mcbc_filter_cmd = send_add_clear_mcbc_filter_cmd_tlv,
	.send_conf_hw_filter_cmd = send_conf_hw_filter_cmd_tlv,
	.send_gtk_offload_cmd = send_gtk_offload_cmd_tlv,
	.send_process_gtk_offload_getinfo_cmd =
		send_process_gtk_offload_getinfo_cmd_tlv,
	.send_enable_enhance_multicast_offload_cmd =
		send_enable_enhance_multicast_offload_tlv,
	.extract_gtk_rsp_event = extract_gtk_rsp_event_tlv,
#ifdef FEATURE_WLAN_RA_FILTERING
	.send_wow_sta_ra_filter_cmd = send_wow_sta_ra_filter_cmd_tlv,
#endif
	.send_action_frame_patterns_cmd = send_action_frame_patterns_cmd_tlv,
	.send_lphb_config_hbenable_cmd = send_lphb_config_hbenable_cmd_tlv,
	.send_lphb_config_tcp_params_cmd = send_lphb_config_tcp_params_cmd_tlv,
	.send_lphb_config_tcp_pkt_filter_cmd =
		send_lphb_config_tcp_pkt_filter_cmd_tlv,
	.send_lphb_config_udp_params_cmd = send_lphb_config_udp_params_cmd_tlv,
	.send_lphb_config_udp_pkt_filter_cmd =
		send_lphb_config_udp_pkt_filter_cmd_tlv,
	.send_enable_disable_packet_filter_cmd =
		send_enable_disable_packet_filter_cmd_tlv,
	.send_config_packet_filter_cmd = send_config_packet_filter_cmd_tlv,
#endif /* End of WLAN_PMO_ENABLE */
#ifdef CONFIG_MCL
	.send_process_dhcp_ind_cmd = send_process_dhcp_ind_cmd_tlv,
	.send_get_link_speed_cmd = send_get_link_speed_cmd_tlv,
	.send_egap_conf_params_cmd = send_egap_conf_params_cmd_tlv,
	.send_bcn_buf_ll_cmd = send_bcn_buf_ll_cmd_tlv,
	.send_process_update_edca_param_cmd =
				 send_process_update_edca_param_cmd_tlv,
	.send_roam_scan_offload_mode_cmd =
			send_roam_scan_offload_mode_cmd_tlv,
	.send_pktlog_wmi_send_cmd = send_pktlog_wmi_send_cmd_tlv,
	.send_roam_scan_offload_ap_profile_cmd =
			send_roam_scan_offload_ap_profile_cmd_tlv,
#endif
	.send_fw_profiling_cmd = send_fw_profiling_cmd_tlv,
	.send_csa_offload_enable_cmd = send_csa_offload_enable_cmd_tlv,
	.send_nat_keepalive_en_cmd = send_nat_keepalive_en_cmd_tlv,
	.send_start_oem_data_cmd = send_start_oem_data_cmd_tlv,
#ifdef WLAN_FEATURE_CIF_CFR
	.send_oem_dma_cfg_cmd = send_oem_dma_cfg_cmd_tlv,
#endif
	.send_dfs_phyerr_filter_offload_en_cmd =
		 send_dfs_phyerr_filter_offload_en_cmd_tlv,
	.send_wow_delete_pattern_cmd = send_wow_delete_pattern_cmd_tlv,
	.send_host_wakeup_ind_to_fw_cmd = send_host_wakeup_ind_to_fw_cmd_tlv,
	.send_del_ts_cmd = send_del_ts_cmd_tlv,
	.send_aggr_qos_cmd = send_aggr_qos_cmd_tlv,
	.send_add_ts_cmd = send_add_ts_cmd_tlv,
	.send_process_add_periodic_tx_ptrn_cmd =
		send_process_add_periodic_tx_ptrn_cmd_tlv,
	.send_process_del_periodic_tx_ptrn_cmd =
		send_process_del_periodic_tx_ptrn_cmd_tlv,
	.send_stats_ext_req_cmd = send_stats_ext_req_cmd_tlv,
	.send_enable_ext_wow_cmd = send_enable_ext_wow_cmd_tlv,
	.send_set_app_type2_params_in_fw_cmd =
		send_set_app_type2_params_in_fw_cmd_tlv,
	.send_set_auto_shutdown_timer_cmd =
		send_set_auto_shutdown_timer_cmd_tlv,
	.send_nan_req_cmd = send_nan_req_cmd_tlv,
	.send_process_dhcpserver_offload_cmd =
		send_process_dhcpserver_offload_cmd_tlv,
	.send_set_led_flashing_cmd = send_set_led_flashing_cmd_tlv,
	.send_process_ch_avoid_update_cmd =
		send_process_ch_avoid_update_cmd_tlv,
	.send_pdev_set_regdomain_cmd =
				send_pdev_set_regdomain_cmd_tlv,
	.send_regdomain_info_to_fw_cmd = send_regdomain_info_to_fw_cmd_tlv,
	.send_set_tdls_offchan_mode_cmd = send_set_tdls_offchan_mode_cmd_tlv,
	.send_update_fw_tdls_state_cmd = send_update_fw_tdls_state_cmd_tlv,
	.send_update_tdls_peer_state_cmd = send_update_tdls_peer_state_cmd_tlv,
	.send_process_fw_mem_dump_cmd = send_process_fw_mem_dump_cmd_tlv,
	.send_process_set_ie_info_cmd = send_process_set_ie_info_cmd_tlv,
#ifdef CONFIG_MCL
	.send_init_cmd = send_init_cmd_tlv,
#endif
	.save_fw_version_cmd = save_fw_version_cmd_tlv,
	.check_and_update_fw_version =
		 check_and_update_fw_version_cmd_tlv,
	.send_saved_init_cmd = send_saved_init_cmd_tlv,
	.send_set_base_macaddr_indicate_cmd =
		 send_set_base_macaddr_indicate_cmd_tlv,
	.send_log_supported_evt_cmd = send_log_supported_evt_cmd_tlv,
	.send_enable_specific_fw_logs_cmd =
		 send_enable_specific_fw_logs_cmd_tlv,
	.send_flush_logs_to_fw_cmd = send_flush_logs_to_fw_cmd_tlv,
	.send_pdev_set_pcl_cmd = send_pdev_set_pcl_cmd_tlv,
	.send_pdev_set_hw_mode_cmd = send_pdev_set_hw_mode_cmd_tlv,
	.send_pdev_set_dual_mac_config_cmd =
		 send_pdev_set_dual_mac_config_cmd_tlv,
	.send_app_type1_params_in_fw_cmd =
		 send_app_type1_params_in_fw_cmd_tlv,
	.send_set_ssid_hotlist_cmd = send_set_ssid_hotlist_cmd_tlv,
	.send_process_roam_synch_complete_cmd =
		 send_process_roam_synch_complete_cmd_tlv,
	.send_unit_test_cmd = send_unit_test_cmd_tlv,
	.send_roam_invoke_cmd = send_roam_invoke_cmd_tlv,
	.send_roam_scan_offload_cmd = send_roam_scan_offload_cmd_tlv,
	.send_roam_scan_offload_scan_period_cmd =
		 send_roam_scan_offload_scan_period_cmd_tlv,
	.send_roam_scan_offload_chan_list_cmd =
		 send_roam_scan_offload_chan_list_cmd_tlv,
	.send_roam_scan_offload_rssi_change_cmd =
		 send_roam_scan_offload_rssi_change_cmd_tlv,
	.send_get_buf_extscan_hotlist_cmd =
		 send_get_buf_extscan_hotlist_cmd_tlv,
	.send_set_active_bpf_mode_cmd = send_set_active_bpf_mode_cmd_tlv,
	.send_adapt_dwelltime_params_cmd =
		send_adapt_dwelltime_params_cmd_tlv,
	.init_cmd_send = init_cmd_send_tlv,
	.send_smart_ant_enable_cmd = send_smart_ant_enable_cmd_tlv,
	.send_smart_ant_set_rx_ant_cmd = send_smart_ant_set_rx_ant_cmd_tlv,
	.send_set_ctl_table_cmd = send_set_ctl_table_cmd_tlv,
	.send_set_mimogain_table_cmd = send_set_mimogain_table_cmd_tlv,
	.send_packet_power_info_get_cmd = send_packet_power_info_get_cmd_tlv,
	.send_vdev_config_ratemask_cmd = send_vdev_config_ratemask_cmd_tlv,
	.send_set_vap_dscp_tid_map_cmd = send_set_vap_dscp_tid_map_cmd_tlv,
	.send_vdev_set_neighbour_rx_cmd = send_vdev_set_neighbour_rx_cmd_tlv,
	.send_smart_ant_set_tx_ant_cmd = send_smart_ant_set_tx_ant_cmd_tlv,
	.send_set_ant_switch_tbl_cmd = send_set_ant_switch_tbl_cmd_tlv,
	.send_smart_ant_set_training_info_cmd =
		send_smart_ant_set_training_info_cmd_tlv,
	.send_smart_ant_set_node_config_cmd =
		send_smart_ant_set_node_config_cmd_tlv,
	.send_set_atf_cmd = send_set_atf_cmd_tlv,
	.send_vdev_set_fwtest_param_cmd = send_vdev_set_fwtest_param_cmd_tlv,
	.send_set_qboost_param_cmd = send_set_qboost_param_cmd_tlv,
	.send_gpio_config_cmd = send_gpio_config_cmd_tlv,
	.send_gpio_output_cmd = send_gpio_output_cmd_tlv,
	.send_phyerr_disable_cmd = send_phyerr_disable_cmd_tlv,
	.send_phyerr_enable_cmd = send_phyerr_enable_cmd_tlv,
	.send_nf_dbr_dbm_info_get_cmd = send_nf_dbr_dbm_info_get_cmd_tlv,
	.send_set_ht_ie_cmd = send_set_ht_ie_cmd_tlv,
	.send_set_vht_ie_cmd = send_set_vht_ie_cmd_tlv,
	.send_set_quiet_mode_cmd = send_set_quiet_mode_cmd_tlv,
	.send_set_bwf_cmd = send_set_bwf_cmd_tlv,
	.send_mcast_group_update_cmd = send_mcast_group_update_cmd_tlv,
	.send_vdev_spectral_configure_cmd =
				send_vdev_spectral_configure_cmd_tlv,
	.send_vdev_spectral_enable_cmd =
				send_vdev_spectral_enable_cmd_tlv,
	.send_thermal_mitigation_param_cmd =
		send_thermal_mitigation_param_cmd_tlv,
	.send_pdev_qvit_cmd = send_pdev_qvit_cmd_tlv,
	.send_wmm_update_cmd = send_wmm_update_cmd_tlv,
	.send_coex_config_cmd = send_coex_config_cmd_tlv,
	.send_set_country_cmd = send_set_country_cmd_tlv,
	.get_target_cap_from_service_ready = extract_service_ready_tlv,
	.extract_hal_reg_cap = extract_hal_reg_cap_tlv,
	.extract_host_mem_req = extract_host_mem_req_tlv,
	.save_service_bitmap = save_service_bitmap_tlv,
	.is_service_enabled = is_service_enabled_tlv,
	.save_fw_version = save_fw_version_in_service_ready_tlv,
	.ready_extract_init_status = ready_extract_init_status_tlv,
	.ready_extract_mac_addr = ready_extract_mac_addr_tlv,
	.extract_dbglog_data_len = extract_dbglog_data_len_tlv,
	.extract_vdev_start_resp = extract_vdev_start_resp_tlv,
	.extract_tbttoffset_update_params =
				extract_tbttoffset_update_params_tlv,
	.extract_ext_tbttoffset_update_params =
				extract_ext_tbttoffset_update_params_tlv,
	.extract_tbttoffset_num_vdevs =
				extract_tbttoffset_num_vdevs_tlv,
	.extract_ext_tbttoffset_num_vdevs =
				extract_ext_tbttoffset_num_vdevs_tlv,
	.extract_mgmt_rx_params = extract_mgmt_rx_params_tlv,
	.extract_vdev_stopped_param = extract_vdev_stopped_param_tlv,
	.extract_vdev_roam_param = extract_vdev_roam_param_tlv,
	.extract_vdev_scan_ev_param = extract_vdev_scan_ev_param_tlv,
#ifdef CONVERGED_TDLS_ENABLE
	.extract_vdev_tdls_ev_param = extract_vdev_tdls_ev_param_tlv,
#endif
	.extract_mgmt_tx_compl_param = extract_mgmt_tx_compl_param_tlv,
	.extract_swba_num_vdevs = extract_swba_num_vdevs_tlv,
	.extract_swba_tim_info = extract_swba_tim_info_tlv,
	.extract_swba_noa_info = extract_swba_noa_info_tlv,
#ifdef CONVERGED_P2P_ENABLE
	.extract_p2p_noa_ev_param = extract_p2p_noa_ev_param_tlv,
	.extract_p2p_lo_stop_ev_param =
				extract_p2p_lo_stop_ev_param_tlv,
#endif
	.extract_offchan_data_tx_compl_param =
				extract_offchan_data_tx_compl_param_tlv,
	.extract_peer_sta_kickout_ev = extract_peer_sta_kickout_ev_tlv,
	.extract_all_stats_count = extract_all_stats_counts_tlv,
	.extract_pdev_stats = extract_pdev_stats_tlv,
	.extract_pdev_ext_stats = extract_pdev_ext_stats_tlv,
	.extract_vdev_stats = extract_vdev_stats_tlv,
	.extract_peer_stats = extract_peer_stats_tlv,
	.extract_bcnflt_stats = extract_bcnflt_stats_tlv,
	.extract_peer_extd_stats = extract_peer_extd_stats_tlv,
	.extract_chan_stats = extract_chan_stats_tlv,
	.extract_profile_ctx = extract_profile_ctx_tlv,
	.extract_profile_data = extract_profile_data_tlv,
	.extract_chan_info_event = extract_chan_info_event_tlv,
	.extract_channel_hopping_event = extract_channel_hopping_event_tlv,
	.send_fw_test_cmd = send_fw_test_cmd_tlv,
	.send_encrypt_decrypt_send_cmd =
				send_encrypt_decrypt_send_cmd_tlv,
	.send_sar_limit_cmd = send_sar_limit_cmd_tlv,
	.send_power_dbg_cmd = send_power_dbg_cmd_tlv,
	.send_multiple_vdev_restart_req_cmd =
				send_multiple_vdev_restart_req_cmd_tlv,
	.extract_service_ready_ext = extract_service_ready_ext_tlv,
	.extract_hw_mode_cap_service_ready_ext =
				extract_hw_mode_cap_service_ready_ext_tlv,
	.extract_mac_phy_cap_service_ready_ext =
				extract_mac_phy_cap_service_ready_ext_tlv,
	.extract_reg_cap_service_ready_ext =
				extract_reg_cap_service_ready_ext_tlv,
	.extract_pdev_utf_event = extract_pdev_utf_event_tlv,
	.wmi_set_htc_tx_tag = wmi_set_htc_tx_tag_tlv,
	.extract_dcs_interference_type = extract_dcs_interference_type_tlv,
	.extract_dcs_cw_int = extract_dcs_cw_int_tlv,
	.extract_dcs_im_tgt_stats = extract_dcs_im_tgt_stats_tlv,
	.extract_fips_event_data = extract_fips_event_data_tlv,
	.send_pdev_fips_cmd = send_pdev_fips_cmd_tlv,
	.extract_peer_delete_response_event =
				extract_peer_delete_response_event_tlv,
	.is_management_record = is_management_record_tlv,
	.extract_pdev_csa_switch_count_status =
				extract_pdev_csa_switch_count_status_tlv,
	.extract_pdev_tpc_ev_param = extract_pdev_tpc_ev_param_tlv,
	.extract_wds_addr_event = extract_wds_addr_event_tlv,
	.extract_peer_sta_ps_statechange_ev =
		extract_peer_sta_ps_statechange_ev_tlv,
	.extract_inst_rssi_stats_event = extract_inst_rssi_stats_event_tlv,
	.send_per_roam_config_cmd = send_per_roam_config_cmd_tlv,
	.send_dfs_phyerr_offload_en_cmd = send_dfs_phyerr_offload_en_cmd_tlv,
	.send_dfs_phyerr_offload_dis_cmd = send_dfs_phyerr_offload_dis_cmd_tlv,
	.extract_reg_chan_list_update_event =
		extract_reg_chan_list_update_event_tlv,
	.extract_chainmask_tables =
		extract_chainmask_tables_tlv,
	.extract_thermal_stats = extract_thermal_stats_tlv,
	.extract_thermal_level_stats = extract_thermal_level_stats_tlv,
#ifdef DFS_COMPONENT_ENABLE
	.extract_dfs_cac_complete_event = extract_dfs_cac_complete_event_tlv,
	.extract_dfs_radar_detection_event =
		extract_dfs_radar_detection_event_tlv,
#endif
	.convert_pdev_id_host_to_target =
		convert_host_pdev_id_to_target_pdev_id_legacy,
	.convert_pdev_id_target_to_host =
		convert_target_pdev_id_to_host_pdev_id_legacy,
};

/**
 * populate_tlv_event_id() - populates wmi event ids
 *
 * @param event_ids: Pointer to hold event ids
 * Return: None
 */
static void populate_tlv_events_id(uint32_t *event_ids)
{
	event_ids[wmi_service_ready_event_id] = WMI_SERVICE_READY_EVENTID;
	event_ids[wmi_ready_event_id] = WMI_READY_EVENTID;
	event_ids[wmi_scan_event_id] = WMI_SCAN_EVENTID;
	event_ids[wmi_pdev_tpc_config_event_id] = WMI_PDEV_TPC_CONFIG_EVENTID;
	event_ids[wmi_chan_info_event_id] = WMI_CHAN_INFO_EVENTID;
	event_ids[wmi_phyerr_event_id] = WMI_PHYERR_EVENTID;
	event_ids[wmi_pdev_dump_event_id] = WMI_PDEV_DUMP_EVENTID;
	event_ids[wmi_tx_pause_event_id] = WMI_TX_PAUSE_EVENTID;
	event_ids[wmi_dfs_radar_event_id] = WMI_DFS_RADAR_EVENTID;
	event_ids[wmi_pdev_l1ss_track_event_id] = WMI_PDEV_L1SS_TRACK_EVENTID;
	event_ids[wmi_pdev_temperature_event_id] = WMI_PDEV_TEMPERATURE_EVENTID;
	event_ids[wmi_service_ready_ext_event_id] =
						WMI_SERVICE_READY_EXT_EVENTID;
	event_ids[wmi_vdev_start_resp_event_id] = WMI_VDEV_START_RESP_EVENTID;
	event_ids[wmi_vdev_stopped_event_id] = WMI_VDEV_STOPPED_EVENTID;
	event_ids[wmi_vdev_install_key_complete_event_id] =
				WMI_VDEV_INSTALL_KEY_COMPLETE_EVENTID;
	event_ids[wmi_vdev_mcc_bcn_intvl_change_req_event_id] =
				WMI_VDEV_MCC_BCN_INTERVAL_CHANGE_REQ_EVENTID;

	event_ids[wmi_vdev_tsf_report_event_id] = WMI_VDEV_TSF_REPORT_EVENTID;
	event_ids[wmi_peer_sta_kickout_event_id] = WMI_PEER_STA_KICKOUT_EVENTID;
	event_ids[wmi_peer_info_event_id] = WMI_PEER_INFO_EVENTID;
	event_ids[wmi_peer_tx_fail_cnt_thr_event_id] =
				WMI_PEER_TX_FAIL_CNT_THR_EVENTID;
	event_ids[wmi_peer_estimated_linkspeed_event_id] =
				WMI_PEER_ESTIMATED_LINKSPEED_EVENTID;
	event_ids[wmi_peer_state_event_id] = WMI_PEER_STATE_EVENTID;
	event_ids[wmi_peer_delete_response_event_id] =
					WMI_PEER_DELETE_RESP_EVENTID;
	event_ids[wmi_mgmt_rx_event_id] = WMI_MGMT_RX_EVENTID;
	event_ids[wmi_host_swba_event_id] = WMI_HOST_SWBA_EVENTID;
	event_ids[wmi_tbttoffset_update_event_id] =
					WMI_TBTTOFFSET_UPDATE_EVENTID;
	event_ids[wmi_ext_tbttoffset_update_event_id] =
					WMI_TBTTOFFSET_EXT_UPDATE_EVENTID;
	event_ids[wmi_offload_bcn_tx_status_event_id] =
				WMI_OFFLOAD_BCN_TX_STATUS_EVENTID;
	event_ids[wmi_offload_prob_resp_tx_status_event_id] =
				WMI_OFFLOAD_PROB_RESP_TX_STATUS_EVENTID;
	event_ids[wmi_mgmt_tx_completion_event_id] =
				WMI_MGMT_TX_COMPLETION_EVENTID;

	event_ids[wmi_tx_delba_complete_event_id] =
					WMI_TX_DELBA_COMPLETE_EVENTID;
	event_ids[wmi_tx_addba_complete_event_id] =
					WMI_TX_ADDBA_COMPLETE_EVENTID;
	event_ids[wmi_ba_rsp_ssn_event_id] = WMI_BA_RSP_SSN_EVENTID;

	event_ids[wmi_aggr_state_trig_event_id] = WMI_AGGR_STATE_TRIG_EVENTID;

	event_ids[wmi_roam_event_id] = WMI_ROAM_EVENTID;
	event_ids[wmi_profile_match] = WMI_PROFILE_MATCH;

	event_ids[wmi_roam_synch_event_id] = WMI_ROAM_SYNCH_EVENTID;

	event_ids[wmi_p2p_disc_event_id] = WMI_P2P_DISC_EVENTID;

	event_ids[wmi_p2p_noa_event_id] = WMI_P2P_NOA_EVENTID;
	event_ids[wmi_p2p_lo_stop_event_id] =
				WMI_P2P_LISTEN_OFFLOAD_STOPPED_EVENTID;
	event_ids[wmi_pdev_resume_event_id] = WMI_PDEV_RESUME_EVENTID;
	event_ids[wmi_wow_wakeup_host_event_id] = WMI_WOW_WAKEUP_HOST_EVENTID;
	event_ids[wmi_do_wow_disable_ack_event_id] =
				WMI_D0_WOW_DISABLE_ACK_EVENTID;
	event_ids[wmi_wow_initial_wakeup_event_id] =
				WMI_WOW_INITIAL_WAKEUP_EVENTID;

	event_ids[wmi_rtt_meas_report_event_id] =
				WMI_RTT_MEASUREMENT_REPORT_EVENTID;
	event_ids[wmi_tsf_meas_report_event_id] =
				WMI_TSF_MEASUREMENT_REPORT_EVENTID;
	event_ids[wmi_rtt_error_report_event_id] = WMI_RTT_ERROR_REPORT_EVENTID;
	event_ids[wmi_stats_ext_event_id] = WMI_STATS_EXT_EVENTID;
	event_ids[wmi_iface_link_stats_event_id] = WMI_IFACE_LINK_STATS_EVENTID;
	event_ids[wmi_peer_link_stats_event_id] = WMI_PEER_LINK_STATS_EVENTID;
	event_ids[wmi_radio_link_stats_link] = WMI_RADIO_LINK_STATS_EVENTID;
	event_ids[wmi_update_fw_mem_dump_event_id] =
				WMI_UPDATE_FW_MEM_DUMP_EVENTID;
	event_ids[wmi_diag_event_id_log_supported_event_id] =
				WMI_DIAG_EVENT_LOG_SUPPORTED_EVENTID;
	event_ids[wmi_nlo_match_event_id] = WMI_NLO_MATCH_EVENTID;
	event_ids[wmi_nlo_scan_complete_event_id] =
					WMI_NLO_SCAN_COMPLETE_EVENTID;
	event_ids[wmi_apfind_event_id] = WMI_APFIND_EVENTID;
	event_ids[wmi_passpoint_match_event_id] = WMI_PASSPOINT_MATCH_EVENTID;

	event_ids[wmi_gtk_offload_status_event_id] =
				WMI_GTK_OFFLOAD_STATUS_EVENTID;
	event_ids[wmi_gtk_rekey_fail_event_id] = WMI_GTK_REKEY_FAIL_EVENTID;
	event_ids[wmi_csa_handling_event_id] = WMI_CSA_HANDLING_EVENTID;
	event_ids[wmi_chatter_pc_query_event_id] = WMI_CHATTER_PC_QUERY_EVENTID;

	event_ids[wmi_echo_event_id] = WMI_ECHO_EVENTID;

	event_ids[wmi_pdev_utf_event_id] = WMI_PDEV_UTF_EVENTID;

	event_ids[wmi_dbg_msg_event_id] = WMI_DEBUG_MESG_EVENTID;
	event_ids[wmi_update_stats_event_id] = WMI_UPDATE_STATS_EVENTID;
	event_ids[wmi_debug_print_event_id] = WMI_DEBUG_PRINT_EVENTID;
	event_ids[wmi_dcs_interference_event_id] = WMI_DCS_INTERFERENCE_EVENTID;
	event_ids[wmi_pdev_qvit_event_id] = WMI_PDEV_QVIT_EVENTID;
	event_ids[wmi_wlan_profile_data_event_id] =
						WMI_WLAN_PROFILE_DATA_EVENTID;
	event_ids[wmi_pdev_ftm_intg_event_id] = WMI_PDEV_FTM_INTG_EVENTID;
	event_ids[wmi_wlan_freq_avoid_event_id] = WMI_WLAN_FREQ_AVOID_EVENTID;
	event_ids[wmi_vdev_get_keepalive_event_id] =
				WMI_VDEV_GET_KEEPALIVE_EVENTID;
	event_ids[wmi_thermal_mgmt_event_id] = WMI_THERMAL_MGMT_EVENTID;

	event_ids[wmi_diag_container_event_id] =
						WMI_DIAG_DATA_CONTAINER_EVENTID;

	event_ids[wmi_host_auto_shutdown_event_id] =
				WMI_HOST_AUTO_SHUTDOWN_EVENTID;

	event_ids[wmi_update_whal_mib_stats_event_id] =
				WMI_UPDATE_WHAL_MIB_STATS_EVENTID;

	/*update ht/vht info based on vdev (rx and tx NSS and preamble) */
	event_ids[wmi_update_vdev_rate_stats_event_id] =
				WMI_UPDATE_VDEV_RATE_STATS_EVENTID;

	event_ids[wmi_diag_event_id] = WMI_DIAG_EVENTID;

	/** Set OCB Sched Response, deprecated */
	event_ids[wmi_ocb_set_sched_event_id] = WMI_OCB_SET_SCHED_EVENTID;

	event_ids[wmi_dbg_mesg_flush_complete_event_id] =
				WMI_DEBUG_MESG_FLUSH_COMPLETE_EVENTID;
	event_ids[wmi_rssi_breach_event_id] = WMI_RSSI_BREACH_EVENTID;

	/* GPIO Event */
	event_ids[wmi_gpio_input_event_id] = WMI_GPIO_INPUT_EVENTID;
	event_ids[wmi_uploadh_event_id] = WMI_UPLOADH_EVENTID;

	event_ids[wmi_captureh_event_id] = WMI_CAPTUREH_EVENTID;
	event_ids[wmi_rfkill_state_change_event_id] =
				WMI_RFKILL_STATE_CHANGE_EVENTID;

	/* TDLS Event */
	event_ids[wmi_tdls_peer_event_id] = WMI_TDLS_PEER_EVENTID;

	event_ids[wmi_batch_scan_enabled_event_id] =
				WMI_BATCH_SCAN_ENABLED_EVENTID;
	event_ids[wmi_batch_scan_result_event_id] =
				WMI_BATCH_SCAN_RESULT_EVENTID;
	/* OEM Event */
	event_ids[wmi_oem_cap_event_id] = WMI_OEM_CAPABILITY_EVENTID;
	event_ids[wmi_oem_meas_report_event_id] =
				WMI_OEM_MEASUREMENT_REPORT_EVENTID;
	event_ids[wmi_oem_report_event_id] = WMI_OEM_ERROR_REPORT_EVENTID;

	/* NAN Event */
	event_ids[wmi_nan_event_id] = WMI_NAN_EVENTID;

	/* LPI Event */
	event_ids[wmi_lpi_result_event_id] = WMI_LPI_RESULT_EVENTID;
	event_ids[wmi_lpi_status_event_id] = WMI_LPI_STATUS_EVENTID;
	event_ids[wmi_lpi_handoff_event_id] = WMI_LPI_HANDOFF_EVENTID;

	/* ExtScan events */
	event_ids[wmi_extscan_start_stop_event_id] =
				WMI_EXTSCAN_START_STOP_EVENTID;
	event_ids[wmi_extscan_operation_event_id] =
				WMI_EXTSCAN_OPERATION_EVENTID;
	event_ids[wmi_extscan_table_usage_event_id] =
				WMI_EXTSCAN_TABLE_USAGE_EVENTID;
	event_ids[wmi_extscan_cached_results_event_id] =
				WMI_EXTSCAN_CACHED_RESULTS_EVENTID;
	event_ids[wmi_extscan_wlan_change_results_event_id] =
				WMI_EXTSCAN_WLAN_CHANGE_RESULTS_EVENTID;
	event_ids[wmi_extscan_hotlist_match_event_id] =
				WMI_EXTSCAN_HOTLIST_MATCH_EVENTID;
	event_ids[wmi_extscan_capabilities_event_id] =
				WMI_EXTSCAN_CAPABILITIES_EVENTID;
	event_ids[wmi_extscan_hotlist_ssid_match_event_id] =
				WMI_EXTSCAN_HOTLIST_SSID_MATCH_EVENTID;

	/* mDNS offload events */
	event_ids[wmi_mdns_stats_event_id] = WMI_MDNS_STATS_EVENTID;

	/* SAP Authentication offload events */
	event_ids[wmi_sap_ofl_add_sta_event_id] = WMI_SAP_OFL_ADD_STA_EVENTID;
	event_ids[wmi_sap_ofl_del_sta_event_id] = WMI_SAP_OFL_DEL_STA_EVENTID;

	/** Out-of-context-of-bss (OCB) events */
	event_ids[wmi_ocb_set_config_resp_event_id] =
				WMI_OCB_SET_CONFIG_RESP_EVENTID;
	event_ids[wmi_ocb_get_tsf_timer_resp_event_id] =
				WMI_OCB_GET_TSF_TIMER_RESP_EVENTID;
	event_ids[wmi_dcc_get_stats_resp_event_id] =
				WMI_DCC_GET_STATS_RESP_EVENTID;
	event_ids[wmi_dcc_update_ndl_resp_event_id] =
				WMI_DCC_UPDATE_NDL_RESP_EVENTID;
	event_ids[wmi_dcc_stats_event_id] = WMI_DCC_STATS_EVENTID;
	/* System-On-Chip events */
	event_ids[wmi_soc_set_hw_mode_resp_event_id] =
				WMI_SOC_SET_HW_MODE_RESP_EVENTID;
	event_ids[wmi_soc_hw_mode_transition_event_id] =
				WMI_SOC_HW_MODE_TRANSITION_EVENTID;
	event_ids[wmi_soc_set_dual_mac_config_resp_event_id] =
				WMI_SOC_SET_DUAL_MAC_CONFIG_RESP_EVENTID;
	event_ids[wmi_pdev_fips_event_id] = WMI_PDEV_FIPS_EVENTID;
	event_ids[wmi_pdev_csa_switch_count_status_event_id] =
				WMI_PDEV_CSA_SWITCH_COUNT_STATUS_EVENTID;
	event_ids[wmi_reg_chan_list_cc_event_id] = WMI_REG_CHAN_LIST_CC_EVENTID;
	event_ids[wmi_inst_rssi_stats_event_id] = WMI_INST_RSSI_STATS_EVENTID;
	event_ids[wmi_pdev_tpc_config_event_id] = WMI_PDEV_TPC_CONFIG_EVENTID;
	event_ids[wmi_peer_sta_ps_statechg_event_id] =
					WMI_PEER_STA_PS_STATECHG_EVENTID;
	event_ids[wmi_pdev_channel_hopping_event_id] =
					WMI_PDEV_CHANNEL_HOPPING_EVENTID;
	event_ids[wmi_offchan_data_tx_completion_event] =
				WMI_OFFCHAN_DATA_TX_COMPLETION_EVENTID;
	event_ids[wmi_dfs_cac_complete_id] = WMI_VDEV_DFS_CAC_COMPLETE_EVENTID;
	event_ids[wmi_dfs_radar_detection_event_id] =
		WMI_PDEV_DFS_RADAR_DETECTION_EVENTID;
	event_ids[wmi_tt_stats_event_id] = WMI_THERM_THROT_STATS_EVENTID;
}

#ifndef CONFIG_MCL
/**
 * populate_tlv_service() - populates wmi services
 *
 * @param wmi_service: Pointer to hold wmi_service
 * Return: None
 */
static void populate_tlv_service(uint32_t *wmi_service)
{
	wmi_service[wmi_service_beacon_offload] = WMI_SERVICE_BEACON_OFFLOAD;
	wmi_service[wmi_service_scan_offload] = WMI_SERVICE_SCAN_OFFLOAD;
	wmi_service[wmi_service_roam_scan_offload] =
					WMI_SERVICE_ROAM_SCAN_OFFLOAD;
	wmi_service[wmi_service_bcn_miss_offload] =
					WMI_SERVICE_BCN_MISS_OFFLOAD;
	wmi_service[wmi_service_sta_pwrsave] = WMI_SERVICE_STA_PWRSAVE;
	wmi_service[wmi_service_sta_advanced_pwrsave] =
				WMI_SERVICE_STA_ADVANCED_PWRSAVE;
	wmi_service[wmi_service_ap_uapsd] = WMI_SERVICE_AP_UAPSD;
	wmi_service[wmi_service_ap_dfs] = WMI_SERVICE_AP_DFS;
	wmi_service[wmi_service_11ac] = WMI_SERVICE_11AC;
	wmi_service[wmi_service_blockack] = WMI_SERVICE_BLOCKACK;
	wmi_service[wmi_service_phyerr] = WMI_SERVICE_PHYERR;
	wmi_service[wmi_service_bcn_filter] = WMI_SERVICE_BCN_FILTER;
	wmi_service[wmi_service_rtt] = WMI_SERVICE_RTT;
	wmi_service[wmi_service_wow] = WMI_SERVICE_WOW;
	wmi_service[wmi_service_ratectrl_cache] = WMI_SERVICE_RATECTRL_CACHE;
	wmi_service[wmi_service_iram_tids] = WMI_SERVICE_IRAM_TIDS;
	wmi_service[wmi_service_arpns_offload] = WMI_SERVICE_ARPNS_OFFLOAD;
	wmi_service[wmi_service_nlo] = WMI_SERVICE_NLO;
	wmi_service[wmi_service_gtk_offload] = WMI_SERVICE_GTK_OFFLOAD;
	wmi_service[wmi_service_scan_sch] = WMI_SERVICE_SCAN_SCH;
	wmi_service[wmi_service_csa_offload] = WMI_SERVICE_CSA_OFFLOAD;
	wmi_service[wmi_service_chatter] = WMI_SERVICE_CHATTER;
	wmi_service[wmi_service_coex_freqavoid] = WMI_SERVICE_COEX_FREQAVOID;
	wmi_service[wmi_service_packet_power_save] =
					WMI_SERVICE_PACKET_POWER_SAVE;
	wmi_service[wmi_service_force_fw_hang] = WMI_SERVICE_FORCE_FW_HANG;
	wmi_service[wmi_service_gpio] = WMI_SERVICE_GPIO;
	wmi_service[wmi_service_sta_dtim_ps_modulated_dtim] =
				WMI_SERVICE_STA_DTIM_PS_MODULATED_DTIM;
	wmi_service[wmi_sta_uapsd_basic_auto_trig] =
					WMI_STA_UAPSD_BASIC_AUTO_TRIG;
	wmi_service[wmi_sta_uapsd_var_auto_trig] = WMI_STA_UAPSD_VAR_AUTO_TRIG;
	wmi_service[wmi_service_sta_keep_alive] = WMI_SERVICE_STA_KEEP_ALIVE;
	wmi_service[wmi_service_tx_encap] = WMI_SERVICE_TX_ENCAP;
	wmi_service[wmi_service_ap_ps_detect_out_of_sync] =
				WMI_SERVICE_AP_PS_DETECT_OUT_OF_SYNC;
	wmi_service[wmi_service_early_rx] = WMI_SERVICE_EARLY_RX;
	wmi_service[wmi_service_sta_smps] = WMI_SERVICE_STA_SMPS;
	wmi_service[wmi_service_fwtest] = WMI_SERVICE_FWTEST;
	wmi_service[wmi_service_sta_wmmac] = WMI_SERVICE_STA_WMMAC;
	wmi_service[wmi_service_tdls] = WMI_SERVICE_TDLS;
	wmi_service[wmi_service_burst] = WMI_SERVICE_BURST;
	wmi_service[wmi_service_mcc_bcn_interval_change] =
				WMI_SERVICE_MCC_BCN_INTERVAL_CHANGE;
	wmi_service[wmi_service_adaptive_ocs] = WMI_SERVICE_ADAPTIVE_OCS;
	wmi_service[wmi_service_ba_ssn_support] = WMI_SERVICE_BA_SSN_SUPPORT;
	wmi_service[wmi_service_filter_ipsec_natkeepalive] =
				WMI_SERVICE_FILTER_IPSEC_NATKEEPALIVE;
	wmi_service[wmi_service_wlan_hb] = WMI_SERVICE_WLAN_HB;
	wmi_service[wmi_service_lte_ant_share_support] =
				WMI_SERVICE_LTE_ANT_SHARE_SUPPORT;
	wmi_service[wmi_service_batch_scan] = WMI_SERVICE_BATCH_SCAN;
	wmi_service[wmi_service_qpower] = WMI_SERVICE_QPOWER;
	wmi_service[wmi_service_plmreq] = WMI_SERVICE_PLMREQ;
	wmi_service[wmi_service_thermal_mgmt] = WMI_SERVICE_THERMAL_MGMT;
	wmi_service[wmi_service_rmc] = WMI_SERVICE_RMC;
	wmi_service[wmi_service_mhf_offload] = WMI_SERVICE_MHF_OFFLOAD;
	wmi_service[wmi_service_coex_sar] = WMI_SERVICE_COEX_SAR;
	wmi_service[wmi_service_bcn_txrate_override] =
				WMI_SERVICE_BCN_TXRATE_OVERRIDE;
	wmi_service[wmi_service_nan] = WMI_SERVICE_NAN;
	wmi_service[wmi_service_l1ss_stat] = WMI_SERVICE_L1SS_STAT;
	wmi_service[wmi_service_estimate_linkspeed] =
				WMI_SERVICE_ESTIMATE_LINKSPEED;
	wmi_service[wmi_service_obss_scan] = WMI_SERVICE_OBSS_SCAN;
	wmi_service[wmi_service_tdls_offchan] = WMI_SERVICE_TDLS_OFFCHAN;
	wmi_service[wmi_service_tdls_uapsd_buffer_sta] =
				WMI_SERVICE_TDLS_UAPSD_BUFFER_STA;
	wmi_service[wmi_service_tdls_uapsd_sleep_sta] =
				WMI_SERVICE_TDLS_UAPSD_SLEEP_STA;
	wmi_service[wmi_service_ibss_pwrsave] = WMI_SERVICE_IBSS_PWRSAVE;
	wmi_service[wmi_service_lpass] = WMI_SERVICE_LPASS;
	wmi_service[wmi_service_extscan] = WMI_SERVICE_EXTSCAN;
	wmi_service[wmi_service_d0wow] = WMI_SERVICE_D0WOW;
	wmi_service[wmi_service_hsoffload] = WMI_SERVICE_HSOFFLOAD;
	wmi_service[wmi_service_roam_ho_offload] = WMI_SERVICE_ROAM_HO_OFFLOAD;
	wmi_service[wmi_service_rx_full_reorder] = WMI_SERVICE_RX_FULL_REORDER;
	wmi_service[wmi_service_dhcp_offload] = WMI_SERVICE_DHCP_OFFLOAD;
	wmi_service[wmi_service_sta_rx_ipa_offload_support] =
				WMI_SERVICE_STA_RX_IPA_OFFLOAD_SUPPORT;
	wmi_service[wmi_service_mdns_offload] = WMI_SERVICE_MDNS_OFFLOAD;
	wmi_service[wmi_service_sap_auth_offload] =
					WMI_SERVICE_SAP_AUTH_OFFLOAD;
	wmi_service[wmi_service_dual_band_simultaneous_support] =
				WMI_SERVICE_DUAL_BAND_SIMULTANEOUS_SUPPORT;
	wmi_service[wmi_service_ocb] = WMI_SERVICE_OCB;
	wmi_service[wmi_service_ap_arpns_offload] =
					WMI_SERVICE_AP_ARPNS_OFFLOAD;
	wmi_service[wmi_service_per_band_chainmask_support] =
				WMI_SERVICE_PER_BAND_CHAINMASK_SUPPORT;
	wmi_service[wmi_service_packet_filter_offload] =
				WMI_SERVICE_PACKET_FILTER_OFFLOAD;
	wmi_service[wmi_service_mgmt_tx_htt] = WMI_SERVICE_MGMT_TX_HTT;
	wmi_service[wmi_service_mgmt_tx_wmi] = WMI_SERVICE_MGMT_TX_WMI;
	wmi_service[wmi_service_ext_msg] = WMI_SERVICE_EXT_MSG;
	wmi_service[wmi_service_mawc] = WMI_SERVICE_MAWC;
	wmi_service[wmi_service_multiple_vdev_restart] =
			WMI_SERVICE_MULTIPLE_VDEV_RESTART;

	wmi_service[wmi_service_roam_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_ratectrl] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_smart_antenna_sw_support] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_smart_antenna_hw_support] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_enhanced_proxy_sta] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tt] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_atf] = WMI_SERVICE_ATF;
	wmi_service[wmi_service_peer_caching] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_coex_gpio] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_aux_spectral_intf] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_aux_chan_load_intf] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_bss_channel_info_64] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_ext_res_cfg_support] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mesh] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_restrt_chnl_support] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_peer_stats] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mesh_11s] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_periodic_chan_stat_support] =
			WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tx_mode_push_only] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tx_mode_push_pull] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tx_mode_dynamic] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_btcoex_duty_cycle] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_4_wire_coex_support] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mesh] = WMI_SERVICE_ENTERPRISE_MESH;
	wmi_service[wmi_service_peer_assoc_conf] = WMI_SERVICE_PEER_ASSOC_CONF;
	wmi_service[wmi_service_egap] = WMI_SERVICE_EGAP;
	wmi_service[wmi_service_sta_pmf_offload] = WMI_SERVICE_STA_PMF_OFFLOAD;
	wmi_service[wmi_service_unified_wow_capability] =
				WMI_SERVICE_UNIFIED_WOW_CAPABILITY;
	wmi_service[wmi_service_enterprise_mesh] = WMI_SERVICE_ENTERPRISE_MESH;
	wmi_service[wmi_service_bpf_offload] = WMI_SERVICE_BPF_OFFLOAD;
	wmi_service[wmi_service_sync_delete_cmds] =
				WMI_SERVICE_SYNC_DELETE_CMDS;
	wmi_service[wmi_service_ratectrl_limit_max_min_rates] =
				WMI_SERVICE_RATECTRL_LIMIT_MAX_MIN_RATES;
	wmi_service[wmi_service_nan_data] = WMI_SERVICE_NAN_DATA;
	wmi_service[wmi_service_nan_rtt] = WMI_SERVICE_NAN_RTT;
	wmi_service[wmi_service_11ax] = WMI_SERVICE_11AX;
	wmi_service[wmi_service_deprecated_replace] =
				WMI_SERVICE_DEPRECATED_REPLACE;
	wmi_service[wmi_service_tdls_conn_tracker_in_host_mode] =
				WMI_SERVICE_TDLS_CONN_TRACKER_IN_HOST_MODE;
	wmi_service[wmi_service_enhanced_mcast_filter] =
				WMI_SERVICE_ENHANCED_MCAST_FILTER;
	wmi_service[wmi_service_half_rate_quarter_rate_support] =
				WMI_SERVICE_HALF_RATE_QUARTER_RATE_SUPPORT;
	wmi_service[wmi_service_vdev_rx_filter] = WMI_SERVICE_VDEV_RX_FILTER;
	wmi_service[wmi_service_p2p_listen_offload_support] =
				WMI_SERVICE_P2P_LISTEN_OFFLOAD_SUPPORT;
	wmi_service[wmi_service_mark_first_wakeup_packet] =
				WMI_SERVICE_MARK_FIRST_WAKEUP_PACKET;
	wmi_service[wmi_service_multiple_mcast_filter_set] =
				WMI_SERVICE_MULTIPLE_MCAST_FILTER_SET;
	wmi_service[wmi_service_host_managed_rx_reorder] =
				WMI_SERVICE_HOST_MANAGED_RX_REORDER;
	wmi_service[wmi_service_flash_rdwr_support] =
				WMI_SERVICE_FLASH_RDWR_SUPPORT;
	wmi_service[wmi_service_wlan_stats_report] =
				WMI_SERVICE_WLAN_STATS_REPORT;
	wmi_service[wmi_service_tx_msdu_id_new_partition_support] =
				WMI_SERVICE_TX_MSDU_ID_NEW_PARTITION_SUPPORT;
	wmi_service[wmi_service_dfs_phyerr_offload] =
				WMI_SERVICE_DFS_PHYERR_OFFLOAD;
	wmi_service[wmi_service_rcpi_support] = WMI_SERVICE_RCPI_SUPPORT;
	wmi_service[wmi_service_fw_mem_dump_support] =
				WMI_SERVICE_FW_MEM_DUMP_SUPPORT;
	wmi_service[wmi_service_peer_stats_info] = WMI_SERVICE_PEER_STATS_INFO;
	wmi_service[wmi_service_regulatory_db] = WMI_SERVICE_REGULATORY_DB;
	wmi_service[wmi_service_11d_offload] = WMI_SERVICE_11D_OFFLOAD;
	wmi_service[wmi_service_hw_data_filtering] =
				WMI_SERVICE_HW_DATA_FILTERING;
	wmi_service[wmi_service_pkt_routing] = WMI_SERVICE_PKT_ROUTING;
	wmi_service[wmi_service_offchan_tx_wmi] = WMI_SERVICE_OFFCHAN_TX_WMI;
}

/**
 * populate_pdev_param_tlv() - populates pdev params
 *
 * @param pdev_param: Pointer to hold pdev params
 * Return: None
 */
static void populate_pdev_param_tlv(uint32_t *pdev_param)
{
	pdev_param[wmi_pdev_param_tx_chain_mask] = WMI_PDEV_PARAM_TX_CHAIN_MASK;
	pdev_param[wmi_pdev_param_rx_chain_mask] = WMI_PDEV_PARAM_RX_CHAIN_MASK;
	pdev_param[wmi_pdev_param_txpower_limit2g] =
				WMI_PDEV_PARAM_TXPOWER_LIMIT2G;
	pdev_param[wmi_pdev_param_txpower_limit5g] =
				WMI_PDEV_PARAM_TXPOWER_LIMIT5G;
	pdev_param[wmi_pdev_param_txpower_scale] = WMI_PDEV_PARAM_TXPOWER_SCALE;
	pdev_param[wmi_pdev_param_beacon_gen_mode] =
				WMI_PDEV_PARAM_BEACON_GEN_MODE;
	pdev_param[wmi_pdev_param_beacon_tx_mode] =
				WMI_PDEV_PARAM_BEACON_TX_MODE;
	pdev_param[wmi_pdev_param_resmgr_offchan_mode] =
				WMI_PDEV_PARAM_RESMGR_OFFCHAN_MODE;
	pdev_param[wmi_pdev_param_protection_mode] =
				WMI_PDEV_PARAM_PROTECTION_MODE;
	pdev_param[wmi_pdev_param_dynamic_bw] = WMI_PDEV_PARAM_DYNAMIC_BW;
	pdev_param[wmi_pdev_param_non_agg_sw_retry_th] =
				WMI_PDEV_PARAM_NON_AGG_SW_RETRY_TH;
	pdev_param[wmi_pdev_param_agg_sw_retry_th] =
				WMI_PDEV_PARAM_AGG_SW_RETRY_TH;
	pdev_param[wmi_pdev_param_sta_kickout_th] =
				WMI_PDEV_PARAM_STA_KICKOUT_TH;
	pdev_param[wmi_pdev_param_ac_aggrsize_scaling] =
				WMI_PDEV_PARAM_AC_AGGRSIZE_SCALING;
	pdev_param[wmi_pdev_param_ltr_enable] = WMI_PDEV_PARAM_LTR_ENABLE;
	pdev_param[wmi_pdev_param_ltr_ac_latency_be] =
				WMI_PDEV_PARAM_LTR_AC_LATENCY_BE;
	pdev_param[wmi_pdev_param_ltr_ac_latency_bk] =
				WMI_PDEV_PARAM_LTR_AC_LATENCY_BK;
	pdev_param[wmi_pdev_param_ltr_ac_latency_vi] =
				WMI_PDEV_PARAM_LTR_AC_LATENCY_VI;
	pdev_param[wmi_pdev_param_ltr_ac_latency_vo] =
				WMI_PDEV_PARAM_LTR_AC_LATENCY_VO;
	pdev_param[wmi_pdev_param_ltr_ac_latency_timeout] =
				WMI_PDEV_PARAM_LTR_AC_LATENCY_TIMEOUT;
	pdev_param[wmi_pdev_param_ltr_sleep_override] =
				WMI_PDEV_PARAM_LTR_SLEEP_OVERRIDE;
	pdev_param[wmi_pdev_param_ltr_rx_override] =
				WMI_PDEV_PARAM_LTR_RX_OVERRIDE;
	pdev_param[wmi_pdev_param_ltr_tx_activity_timeout] =
				WMI_PDEV_PARAM_LTR_TX_ACTIVITY_TIMEOUT;
	pdev_param[wmi_pdev_param_l1ss_enable] = WMI_PDEV_PARAM_L1SS_ENABLE;
	pdev_param[wmi_pdev_param_dsleep_enable] = WMI_PDEV_PARAM_DSLEEP_ENABLE;
	pdev_param[wmi_pdev_param_pcielp_txbuf_flush] =
				WMI_PDEV_PARAM_PCIELP_TXBUF_FLUSH;
	pdev_param[wmi_pdev_param_pcielp_txbuf_watermark] =
				WMI_PDEV_PARAM_PCIELP_TXBUF_WATERMARK;
	pdev_param[wmi_pdev_param_pcielp_txbuf_tmo_en] =
				WMI_PDEV_PARAM_PCIELP_TXBUF_TMO_EN;
	pdev_param[wmi_pdev_param_pcielp_txbuf_tmo_value] =
				WMI_PDEV_PARAM_PCIELP_TXBUF_TMO_VALUE;
	pdev_param[wmi_pdev_param_pdev_stats_update_period] =
				WMI_PDEV_PARAM_PDEV_STATS_UPDATE_PERIOD;
	pdev_param[wmi_pdev_param_vdev_stats_update_period] =
				WMI_PDEV_PARAM_VDEV_STATS_UPDATE_PERIOD;
	pdev_param[wmi_pdev_param_peer_stats_update_period] =
				WMI_PDEV_PARAM_PEER_STATS_UPDATE_PERIOD;
	pdev_param[wmi_pdev_param_bcnflt_stats_update_period] =
				WMI_PDEV_PARAM_BCNFLT_STATS_UPDATE_PERIOD;
	pdev_param[wmi_pdev_param_pmf_qos] = WMI_PDEV_PARAM_PMF_QOS;
	pdev_param[wmi_pdev_param_arp_ac_override] =
				WMI_PDEV_PARAM_ARP_AC_OVERRIDE;
	pdev_param[wmi_pdev_param_dcs] = WMI_PDEV_PARAM_DCS;
	pdev_param[wmi_pdev_param_ani_enable] = WMI_PDEV_PARAM_ANI_ENABLE;
	pdev_param[wmi_pdev_param_ani_poll_period] =
				WMI_PDEV_PARAM_ANI_POLL_PERIOD;
	pdev_param[wmi_pdev_param_ani_listen_period] =
				WMI_PDEV_PARAM_ANI_LISTEN_PERIOD;
	pdev_param[wmi_pdev_param_ani_ofdm_level] =
				WMI_PDEV_PARAM_ANI_OFDM_LEVEL;
	pdev_param[wmi_pdev_param_ani_cck_level] = WMI_PDEV_PARAM_ANI_CCK_LEVEL;
	pdev_param[wmi_pdev_param_dyntxchain] = WMI_PDEV_PARAM_DYNTXCHAIN;
	pdev_param[wmi_pdev_param_proxy_sta] = WMI_PDEV_PARAM_PROXY_STA;
	pdev_param[wmi_pdev_param_idle_ps_config] =
				WMI_PDEV_PARAM_IDLE_PS_CONFIG;
	pdev_param[wmi_pdev_param_power_gating_sleep] =
				WMI_PDEV_PARAM_POWER_GATING_SLEEP;
	pdev_param[wmi_pdev_param_rfkill_enable] = WMI_PDEV_PARAM_RFKILL_ENABLE;
	pdev_param[wmi_pdev_param_burst_dur] = WMI_PDEV_PARAM_BURST_DUR;
	pdev_param[wmi_pdev_param_burst_enable] = WMI_PDEV_PARAM_BURST_ENABLE;
	pdev_param[wmi_pdev_param_hw_rfkill_config] =
				WMI_PDEV_PARAM_HW_RFKILL_CONFIG;
	pdev_param[wmi_pdev_param_low_power_rf_enable] =
				WMI_PDEV_PARAM_LOW_POWER_RF_ENABLE;
	pdev_param[wmi_pdev_param_l1ss_track] = WMI_PDEV_PARAM_L1SS_TRACK;
	pdev_param[wmi_pdev_param_hyst_en] = WMI_PDEV_PARAM_HYST_EN;
	pdev_param[wmi_pdev_param_power_collapse_enable] =
				WMI_PDEV_PARAM_POWER_COLLAPSE_ENABLE;
	pdev_param[wmi_pdev_param_led_sys_state] = WMI_PDEV_PARAM_LED_SYS_STATE;
	pdev_param[wmi_pdev_param_led_enable] = WMI_PDEV_PARAM_LED_ENABLE;
	pdev_param[wmi_pdev_param_audio_over_wlan_latency] =
				WMI_PDEV_PARAM_AUDIO_OVER_WLAN_LATENCY;
	pdev_param[wmi_pdev_param_audio_over_wlan_enable] =
				WMI_PDEV_PARAM_AUDIO_OVER_WLAN_ENABLE;
	pdev_param[wmi_pdev_param_whal_mib_stats_update_enable] =
				WMI_PDEV_PARAM_WHAL_MIB_STATS_UPDATE_ENABLE;
	pdev_param[wmi_pdev_param_vdev_rate_stats_update_period] =
				WMI_PDEV_PARAM_VDEV_RATE_STATS_UPDATE_PERIOD;
	pdev_param[wmi_pdev_param_cts_cbw] = WMI_PDEV_PARAM_CTS_CBW;
	pdev_param[wmi_pdev_param_wnts_config] = WMI_PDEV_PARAM_WNTS_CONFIG;
	pdev_param[wmi_pdev_param_adaptive_early_rx_enable] =
				WMI_PDEV_PARAM_ADAPTIVE_EARLY_RX_ENABLE;
	pdev_param[wmi_pdev_param_adaptive_early_rx_min_sleep_slop] =
				WMI_PDEV_PARAM_ADAPTIVE_EARLY_RX_MIN_SLEEP_SLOP;
	pdev_param[wmi_pdev_param_adaptive_early_rx_inc_dec_step] =
				WMI_PDEV_PARAM_ADAPTIVE_EARLY_RX_INC_DEC_STEP;
	pdev_param[wmi_pdev_param_early_rx_fix_sleep_slop] =
				WMI_PDEV_PARAM_EARLY_RX_FIX_SLEEP_SLOP;
	pdev_param[wmi_pdev_param_bmiss_based_adaptive_bto_enable] =
				WMI_PDEV_PARAM_BMISS_BASED_ADAPTIVE_BTO_ENABLE;
	pdev_param[wmi_pdev_param_bmiss_bto_min_bcn_timeout] =
				WMI_PDEV_PARAM_BMISS_BTO_MIN_BCN_TIMEOUT;
	pdev_param[wmi_pdev_param_bmiss_bto_inc_dec_step] =
				WMI_PDEV_PARAM_BMISS_BTO_INC_DEC_STEP;
	pdev_param[wmi_pdev_param_bto_fix_bcn_timeout] =
				WMI_PDEV_PARAM_BTO_FIX_BCN_TIMEOUT;
	pdev_param[wmi_pdev_param_ce_based_adaptive_bto_enable] =
				WMI_PDEV_PARAM_CE_BASED_ADAPTIVE_BTO_ENABLE;
	pdev_param[wmi_pdev_param_ce_bto_combo_ce_value] =
				WMI_PDEV_PARAM_CE_BTO_COMBO_CE_VALUE;
	pdev_param[wmi_pdev_param_tx_chain_mask_2g] =
				WMI_PDEV_PARAM_TX_CHAIN_MASK_2G;
	pdev_param[wmi_pdev_param_rx_chain_mask_2g] =
				WMI_PDEV_PARAM_RX_CHAIN_MASK_2G;
	pdev_param[wmi_pdev_param_tx_chain_mask_5g] =
				WMI_PDEV_PARAM_TX_CHAIN_MASK_5G;
	pdev_param[wmi_pdev_param_rx_chain_mask_5g] =
				WMI_PDEV_PARAM_RX_CHAIN_MASK_5G;
	pdev_param[wmi_pdev_param_tx_chain_mask_cck] =
				WMI_PDEV_PARAM_TX_CHAIN_MASK_CCK;
	pdev_param[wmi_pdev_param_tx_chain_mask_1ss] =
				WMI_PDEV_PARAM_TX_CHAIN_MASK_1SS;
	pdev_param[wmi_pdev_param_rx_filter] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_set_mcast_to_ucast_tid] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_mgmt_retry_limit] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_aggr_burst] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_peer_sta_ps_statechg_enable] =
						WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_proxy_sta_mode] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_mu_group_policy] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_noise_detection] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_noise_threshold] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_dpd_enable] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_set_mcast_bcast_echo] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_atf_strict_sch] =
		WMI_PDEV_PARAM_ATF_STRICT_SCH;
	pdev_param[wmi_pdev_param_atf_sched_duration] =
		WMI_PDEV_PARAM_ATF_SCHED_DURATION;
	pdev_param[wmi_pdev_param_ant_plzn] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_sensitivity_level] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_signed_txpower_2g] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_signed_txpower_5g] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_enable_per_tid_amsdu] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_enable_per_tid_ampdu] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_cca_threshold] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_rts_fixed_rate] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_cal_period] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_pdev_reset] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_wapi_mbssid_offset] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_arp_srcaddr] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_arp_dstaddr] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_txpower_decr_db] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_rx_batchmode] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_packet_aggr_delay] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_atf_obss_noise_sch] =
		WMI_PDEV_PARAM_ATF_OBSS_NOISE_SCH;
	pdev_param[wmi_pdev_param_atf_obss_noise_scaling_factor] =
		WMI_PDEV_PARAM_ATF_OBSS_NOISE_SCALING_FACTOR;
	pdev_param[wmi_pdev_param_cust_txpower_scale] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_atf_dynamic_enable] =
		WMI_PDEV_PARAM_ATF_DYNAMIC_ENABLE;
	pdev_param[wmi_pdev_param_atf_ssid_group_policy] =
						WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_igmpmld_override] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_igmpmld_tid] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_antenna_gain] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_block_interbss] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_set_disable_reset_cmdid] =
						WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_set_msdu_ttl_cmdid] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_txbf_sound_period_cmdid] =
						WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_set_burst_mode_cmdid] =
					WMI_PDEV_PARAM_SET_BURST_MODE_CMDID;
	pdev_param[wmi_pdev_param_en_stats] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_mesh_mcast_enable] =
					WMI_PDEV_PARAM_MESH_MCAST_ENABLE;
	pdev_param[wmi_pdev_param_set_promisc_mode_cmdid] =
					WMI_PDEV_PARAM_SET_PROMISC_MODE_CMDID;
	pdev_param[wmi_pdev_param_set_ppdu_duration_cmdid] =
					WMI_PDEV_PARAM_SET_PPDU_DURATION_CMDID;
	pdev_param[wmi_pdev_param_igmpmld_ac_override] =
					WMI_PDEV_PARAM_IGMPMLD_AC_OVERRIDE;
	pdev_param[wmi_pdev_param_remove_mcast2ucast_buffer] =
				WMI_PDEV_PARAM_REMOVE_MCAST2UCAST_BUFFER;
	pdev_param[wmi_pdev_param_set_mcast2ucast_buffer] =
				WMI_PDEV_PARAM_SET_MCAST2UCAST_BUFFER;
	pdev_param[wmi_pdev_param_set_mcast2ucast_mode] =
				WMI_PDEV_PARAM_SET_MCAST2UCAST_MODE;
	pdev_param[wmi_pdev_param_smart_antenna_default_antenna] =
				WMI_PDEV_PARAM_SMART_ANTENNA_DEFAULT_ANTENNA;
	pdev_param[wmi_pdev_param_fast_channel_reset] =
				WMI_PDEV_PARAM_FAST_CHANNEL_RESET;
	pdev_param[wmi_pdev_param_rx_decap_mode] = WMI_PDEV_PARAM_RX_DECAP_MODE;
}

/**
 * populate_vdev_param_tlv() - populates vdev params
 *
 * @param vdev_param: Pointer to hold vdev params
 * Return: None
 */
static void populate_vdev_param_tlv(uint32_t *vdev_param)
{
	vdev_param[wmi_vdev_param_rts_threshold] = WMI_VDEV_PARAM_RTS_THRESHOLD;
	vdev_param[wmi_vdev_param_fragmentation_threshold] =
				WMI_VDEV_PARAM_FRAGMENTATION_THRESHOLD;
	vdev_param[wmi_vdev_param_beacon_interval] =
				WMI_VDEV_PARAM_BEACON_INTERVAL;
	vdev_param[wmi_vdev_param_listen_interval] =
				WMI_VDEV_PARAM_LISTEN_INTERVAL;
	vdev_param[wmi_vdev_param_multicast_rate] =
				WMI_VDEV_PARAM_MULTICAST_RATE;
	vdev_param[wmi_vdev_param_mgmt_tx_rate] = WMI_VDEV_PARAM_MGMT_TX_RATE;
	vdev_param[wmi_vdev_param_slot_time] = WMI_VDEV_PARAM_SLOT_TIME;
	vdev_param[wmi_vdev_param_preamble] = WMI_VDEV_PARAM_PREAMBLE;
	vdev_param[wmi_vdev_param_swba_time] = WMI_VDEV_PARAM_SWBA_TIME;
	vdev_param[wmi_vdev_stats_update_period] = WMI_VDEV_STATS_UPDATE_PERIOD;
	vdev_param[wmi_vdev_pwrsave_ageout_time] = WMI_VDEV_PWRSAVE_AGEOUT_TIME;
	vdev_param[wmi_vdev_host_swba_interval] = WMI_VDEV_HOST_SWBA_INTERVAL;
	vdev_param[wmi_vdev_param_dtim_period] = WMI_VDEV_PARAM_DTIM_PERIOD;
	vdev_param[wmi_vdev_oc_scheduler_air_time_limit] =
				WMI_VDEV_OC_SCHEDULER_AIR_TIME_LIMIT;
	vdev_param[wmi_vdev_param_wds] = WMI_VDEV_PARAM_WDS;
	vdev_param[wmi_vdev_param_atim_window] = WMI_VDEV_PARAM_ATIM_WINDOW;
	vdev_param[wmi_vdev_param_bmiss_count_max] =
				WMI_VDEV_PARAM_BMISS_COUNT_MAX;
	vdev_param[wmi_vdev_param_bmiss_first_bcnt] =
				WMI_VDEV_PARAM_BMISS_FIRST_BCNT;
	vdev_param[wmi_vdev_param_bmiss_final_bcnt] =
				WMI_VDEV_PARAM_BMISS_FINAL_BCNT;
	vdev_param[wmi_vdev_param_feature_wmm] = WMI_VDEV_PARAM_FEATURE_WMM;
	vdev_param[wmi_vdev_param_chwidth] = WMI_VDEV_PARAM_CHWIDTH;
	vdev_param[wmi_vdev_param_chextoffset] = WMI_VDEV_PARAM_CHEXTOFFSET;
	vdev_param[wmi_vdev_param_disable_htprotection] =
				WMI_VDEV_PARAM_DISABLE_HTPROTECTION;
	vdev_param[wmi_vdev_param_sta_quickkickout] =
				WMI_VDEV_PARAM_STA_QUICKKICKOUT;
	vdev_param[wmi_vdev_param_mgmt_rate] = WMI_VDEV_PARAM_MGMT_RATE;
	vdev_param[wmi_vdev_param_protection_mode] =
				WMI_VDEV_PARAM_PROTECTION_MODE;
	vdev_param[wmi_vdev_param_fixed_rate] = WMI_VDEV_PARAM_FIXED_RATE;
	vdev_param[wmi_vdev_param_sgi] = WMI_VDEV_PARAM_SGI;
	vdev_param[wmi_vdev_param_ldpc] = WMI_VDEV_PARAM_LDPC;
	vdev_param[wmi_vdev_param_tx_stbc] = WMI_VDEV_PARAM_TX_STBC;
	vdev_param[wmi_vdev_param_rx_stbc] = WMI_VDEV_PARAM_RX_STBC;
	vdev_param[wmi_vdev_param_intra_bss_fwd] = WMI_VDEV_PARAM_INTRA_BSS_FWD;
	vdev_param[wmi_vdev_param_def_keyid] = WMI_VDEV_PARAM_DEF_KEYID;
	vdev_param[wmi_vdev_param_nss] = WMI_VDEV_PARAM_NSS;
	vdev_param[wmi_vdev_param_bcast_data_rate] =
				WMI_VDEV_PARAM_BCAST_DATA_RATE;
	vdev_param[wmi_vdev_param_mcast_data_rate] =
				WMI_VDEV_PARAM_MCAST_DATA_RATE;
	vdev_param[wmi_vdev_param_mcast_indicate] =
				WMI_VDEV_PARAM_MCAST_INDICATE;
	vdev_param[wmi_vdev_param_dhcp_indicate] =
				WMI_VDEV_PARAM_DHCP_INDICATE;
	vdev_param[wmi_vdev_param_unknown_dest_indicate] =
				WMI_VDEV_PARAM_UNKNOWN_DEST_INDICATE;
	vdev_param[wmi_vdev_param_ap_keepalive_min_idle_inactive_time_secs] =
		WMI_VDEV_PARAM_AP_KEEPALIVE_MIN_IDLE_INACTIVE_TIME_SECS;
	vdev_param[wmi_vdev_param_ap_keepalive_max_idle_inactive_time_secs] =
		WMI_VDEV_PARAM_AP_KEEPALIVE_MAX_IDLE_INACTIVE_TIME_SECS;
	vdev_param[wmi_vdev_param_ap_keepalive_max_unresponsive_time_secs] =
		WMI_VDEV_PARAM_AP_KEEPALIVE_MAX_UNRESPONSIVE_TIME_SECS;
	vdev_param[wmi_vdev_param_ap_enable_nawds] =
				WMI_VDEV_PARAM_AP_ENABLE_NAWDS;
	vdev_param[wmi_vdev_param_enable_rtscts] = WMI_VDEV_PARAM_ENABLE_RTSCTS;
	vdev_param[wmi_vdev_param_txbf] = WMI_VDEV_PARAM_TXBF;
	vdev_param[wmi_vdev_param_packet_powersave] =
				WMI_VDEV_PARAM_PACKET_POWERSAVE;
	vdev_param[wmi_vdev_param_drop_unencry] = WMI_VDEV_PARAM_DROP_UNENCRY;
	vdev_param[wmi_vdev_param_tx_encap_type] = WMI_VDEV_PARAM_TX_ENCAP_TYPE;
	vdev_param[wmi_vdev_param_ap_detect_out_of_sync_sleeping_sta_time_secs] =
		WMI_VDEV_PARAM_AP_DETECT_OUT_OF_SYNC_SLEEPING_STA_TIME_SECS;
	vdev_param[wmi_vdev_param_early_rx_adjust_enable] =
				WMI_VDEV_PARAM_EARLY_RX_ADJUST_ENABLE;
	vdev_param[wmi_vdev_param_early_rx_tgt_bmiss_num] =
				WMI_VDEV_PARAM_EARLY_RX_TGT_BMISS_NUM;
	vdev_param[wmi_vdev_param_early_rx_bmiss_sample_cycle] =
				WMI_VDEV_PARAM_EARLY_RX_BMISS_SAMPLE_CYCLE;
	vdev_param[wmi_vdev_param_early_rx_slop_step] =
				WMI_VDEV_PARAM_EARLY_RX_SLOP_STEP;
	vdev_param[wmi_vdev_param_early_rx_init_slop] =
				WMI_VDEV_PARAM_EARLY_RX_INIT_SLOP;
	vdev_param[wmi_vdev_param_early_rx_adjust_pause] =
				WMI_VDEV_PARAM_EARLY_RX_ADJUST_PAUSE;
	vdev_param[wmi_vdev_param_tx_pwrlimit] = WMI_VDEV_PARAM_TX_PWRLIMIT;
	vdev_param[wmi_vdev_param_snr_num_for_cal] =
				WMI_VDEV_PARAM_SNR_NUM_FOR_CAL;
	vdev_param[wmi_vdev_param_roam_fw_offload] =
				WMI_VDEV_PARAM_ROAM_FW_OFFLOAD;
	vdev_param[wmi_vdev_param_enable_rmc] = WMI_VDEV_PARAM_ENABLE_RMC;
	vdev_param[wmi_vdev_param_ibss_max_bcn_lost_ms] =
				WMI_VDEV_PARAM_IBSS_MAX_BCN_LOST_MS;
	vdev_param[wmi_vdev_param_max_rate] = WMI_VDEV_PARAM_MAX_RATE;
	vdev_param[wmi_vdev_param_early_rx_drift_sample] =
				WMI_VDEV_PARAM_EARLY_RX_DRIFT_SAMPLE;
	vdev_param[wmi_vdev_param_set_ibss_tx_fail_cnt_thr] =
				WMI_VDEV_PARAM_SET_IBSS_TX_FAIL_CNT_THR;
	vdev_param[wmi_vdev_param_ebt_resync_timeout] =
				WMI_VDEV_PARAM_EBT_RESYNC_TIMEOUT;
	vdev_param[wmi_vdev_param_aggr_trig_event_enable] =
				WMI_VDEV_PARAM_AGGR_TRIG_EVENT_ENABLE;
	vdev_param[wmi_vdev_param_is_ibss_power_save_allowed] =
				WMI_VDEV_PARAM_IS_IBSS_POWER_SAVE_ALLOWED;
	vdev_param[wmi_vdev_param_is_power_collapse_allowed] =
				WMI_VDEV_PARAM_IS_POWER_COLLAPSE_ALLOWED;
	vdev_param[wmi_vdev_param_is_awake_on_txrx_enabled] =
				WMI_VDEV_PARAM_IS_AWAKE_ON_TXRX_ENABLED;
	vdev_param[wmi_vdev_param_inactivity_cnt] =
		WMI_VDEV_PARAM_INACTIVITY_CNT;
	vdev_param[wmi_vdev_param_txsp_end_inactivity_time_ms] =
				WMI_VDEV_PARAM_TXSP_END_INACTIVITY_TIME_MS;
	vdev_param[wmi_vdev_param_dtim_policy] = WMI_VDEV_PARAM_DTIM_POLICY;
	vdev_param[wmi_vdev_param_ibss_ps_warmup_time_secs] =
				WMI_VDEV_PARAM_IBSS_PS_WARMUP_TIME_SECS;
	vdev_param[wmi_vdev_param_ibss_ps_1rx_chain_in_atim_window_enable] =
			WMI_VDEV_PARAM_IBSS_PS_1RX_CHAIN_IN_ATIM_WINDOW_ENABLE;
	vdev_param[wmi_vdev_param_rx_leak_window] =
			WMI_VDEV_PARAM_RX_LEAK_WINDOW;
	vdev_param[wmi_vdev_param_stats_avg_factor] =
				WMI_VDEV_PARAM_STATS_AVG_FACTOR;
	vdev_param[wmi_vdev_param_disconnect_th] = WMI_VDEV_PARAM_DISCONNECT_TH;
	vdev_param[wmi_vdev_param_rtscts_rate] = WMI_VDEV_PARAM_RTSCTS_RATE;
	vdev_param[wmi_vdev_param_mcc_rtscts_protection_enable] =
				WMI_VDEV_PARAM_MCC_RTSCTS_PROTECTION_ENABLE;
	vdev_param[wmi_vdev_param_mcc_broadcast_probe_enable] =
				WMI_VDEV_PARAM_MCC_BROADCAST_PROBE_ENABLE;
	vdev_param[wmi_vdev_param_mgmt_tx_power] = WMI_VDEV_PARAM_MGMT_TX_POWER;
	vdev_param[wmi_vdev_param_beacon_rate] = WMI_VDEV_PARAM_BEACON_RATE;
	vdev_param[wmi_vdev_param_rx_decap_type] = WMI_VDEV_PARAM_RX_DECAP_TYPE;
	vdev_param[wmi_vdev_param_he_dcm_enable] = WMI_VDEV_PARAM_HE_DCM;
	vdev_param[wmi_vdev_param_he_range_ext_enable] =
				 WMI_VDEV_PARAM_HE_RANGE_EXT;
	vdev_param[wmi_vdev_param_he_bss_color] = WMI_VDEV_PARAM_BSS_COLOR;
	vdev_param[wmi_vdev_param_set_hemu_mode] = WMI_VDEV_PARAM_SET_HEMU_MODE;
	vdev_param[wmi_vdev_param_sensor_ap] = WMI_VDEV_PARAM_SENSOR_AP;
	vdev_param[wmi_vdev_param_dtim_enable_cts] =
					WMI_VDEV_PARAM_DTIM_ENABLE_CTS;
	vdev_param[wmi_vdev_param_atf_ssid_sched_policy] =
					WMI_VDEV_PARAM_ATF_SSID_SCHED_POLICY;
	vdev_param[wmi_vdev_param_disable_dyn_bw_rts] =
					WMI_VDEV_PARAM_DISABLE_DYN_BW_RTS;
	vdev_param[wmi_vdev_param_mcast2ucast_set] =
					WMI_VDEV_PARAM_MCAST2UCAST_SET;
	vdev_param[wmi_vdev_param_rc_num_retries] =
					WMI_VDEV_PARAM_RC_NUM_RETRIES;
	vdev_param[wmi_vdev_param_cabq_maxdur] = WMI_VDEV_PARAM_CABQ_MAXDUR;
	vdev_param[wmi_vdev_param_mfptest_set] = WMI_VDEV_PARAM_MFPTEST_SET;
	vdev_param[wmi_vdev_param_rts_fixed_rate] =
					WMI_VDEV_PARAM_RTS_FIXED_RATE;
	vdev_param[wmi_vdev_param_vht_sgimask] = WMI_VDEV_PARAM_VHT_SGIMASK;
	vdev_param[wmi_vdev_param_vht80_ratemask] =
					WMI_VDEV_PARAM_VHT80_RATEMASK;
	vdev_param[wmi_vdev_param_proxy_sta] = WMI_VDEV_PARAM_PROXY_STA;
	vdev_param[wmi_vdev_param_bw_nss_ratemask] =
					WMI_VDEV_PARAM_BW_NSS_RATEMASK;
}
#endif

/**
 * populate_target_defines_tlv() - Populate target defines and params
 * @wmi_handle: pointer to wmi handle
 *
 * Return: None
 */
#ifndef CONFIG_MCL
static void populate_target_defines_tlv(struct wmi_unified *wmi_handle)
{
	populate_tlv_service(wmi_handle->services);
	populate_pdev_param_tlv(wmi_handle->pdev_param);
	populate_vdev_param_tlv(wmi_handle->vdev_param);
}
#else
static void populate_target_defines_tlv(struct wmi_unified *wmi_handle)
{ }
#endif

/**
 * convert_host_pdev_id_to_target_pdev_id() - Convert pdev_id from
 *           host to target defines.
 * @param pdev_id: host pdev_id to be converted.
 * Return: target pdev_id after conversion.
 */
static uint32_t convert_host_pdev_id_to_target_pdev_id(uint32_t pdev_id)
{
	switch (pdev_id) {
	case WMI_HOST_PDEV_ID_SOC:
		return WMI_PDEV_ID_SOC;
	case WMI_HOST_PDEV_ID_0:
		return WMI_PDEV_ID_1ST;
	case WMI_HOST_PDEV_ID_1:
		return WMI_PDEV_ID_2ND;
	case WMI_HOST_PDEV_ID_2:
		return WMI_PDEV_ID_3RD;
	}

	QDF_ASSERT(0);

	return WMI_PDEV_ID_SOC;
}

/**
 * convert_target_pdev_id_to_host_pdev_id() - Convert pdev_id from
 *           target to host defines.
 * @param pdev_id: target pdev_id to be converted.
 * Return: host pdev_id after conversion.
 */
static uint32_t convert_target_pdev_id_to_host_pdev_id(uint32_t pdev_id)
{
	switch (pdev_id) {
	case WMI_PDEV_ID_SOC:
		return WMI_HOST_PDEV_ID_SOC;
	case WMI_PDEV_ID_1ST:
		return WMI_HOST_PDEV_ID_0;
	case WMI_PDEV_ID_2ND:
		return WMI_HOST_PDEV_ID_1;
	case WMI_PDEV_ID_3RD:
		return WMI_HOST_PDEV_ID_2;
	}

	QDF_ASSERT(0);

	return WMI_HOST_PDEV_ID_SOC;
}

/**
 * wmi_tlv_pdev_id_conversion_enable() - Enable pdev_id conversion
 *
 * Return None.
 */
void wmi_tlv_pdev_id_conversion_enable(wmi_unified_t wmi_handle)
{
	wmi_handle->ops->convert_pdev_id_host_to_target =
		convert_host_pdev_id_to_target_pdev_id;
	wmi_handle->ops->convert_pdev_id_target_to_host =
		convert_target_pdev_id_to_host_pdev_id;
}
/**
 * wmi_tlv_attach() - Attach TLV APIs
 *
 * Return: None
 */
void wmi_tlv_attach(wmi_unified_t wmi_handle)
{
	wmi_handle->ops = &tlv_ops;
#ifdef WMI_INTERFACE_EVENT_LOGGING
	wmi_handle->log_info.buf_offset_command = 2;
	wmi_handle->log_info.buf_offset_event = 4;
#endif
	populate_tlv_events_id(wmi_handle->wmi_events);
	populate_target_defines_tlv(wmi_handle);
}
