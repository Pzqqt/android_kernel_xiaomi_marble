/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
#include "wmi_unified_priv.h"
#include "target_type.h"

#if defined(WMI_NON_TLV_SUPPORT) || defined(WMI_TLV_AND_NON_TLV_SUPPORT)
#include "wmi.h"
#include "wmi_unified.h"
#include <htc_services.h>

/* pdev_id is used to distinguish the radio for which event
 * is recieved. Since non-tlv target has only one radio, setting
 * default pdev_id to one to keep rest of the code using WMI APIs unfiorm.
 */
#define WMI_NON_TLV_DEFAULT_PDEV_ID WMI_HOST_PDEV_ID_0

/* HTC service id for WMI */
static const uint32_t svc_ids[] = {WMI_CONTROL_SVC};

/**
 * send_vdev_create_cmd_non_tlv() - send VDEV create command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vdev create parameter
 * @macaddr: vdev mac address
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_create_cmd_non_tlv(wmi_unified_t wmi_handle,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param)
{
	wmi_vdev_create_cmd *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(wmi_vdev_create_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_create_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->if_id;
	cmd->vdev_type = param->type;
	cmd->vdev_subtype = param->subtype;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->vdev_macaddr);
	qdf_print("%s: ID = %d Type = %d, Subtype = %d "
			"VAP Addr = %02x:%02x:%02x:%02x:%02x:%02x:\n",
			__func__, param->if_id, param->type, param->subtype,
			macaddr[0], macaddr[1], macaddr[2],
			macaddr[3], macaddr[4], macaddr[5]);
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_VDEV_CREATE_CMDID);
}

/**
 * send_vdev_delete_cmd_non_tlv() - send VDEV delete command to fw
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_delete_cmd_non_tlv(wmi_unified_t wmi_handle,
					  uint8_t if_id)
{
	wmi_vdev_delete_cmd *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(wmi_vdev_delete_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_delete_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = if_id;
	qdf_print("%s for vap %d (%pK)\n", __func__, if_id, wmi_handle);
	return wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_VDEV_DELETE_CMDID);
}

/**
 * send_vdev_stop_cmd_non_tlv() - send vdev stop command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or erro code
 */
static QDF_STATUS send_vdev_stop_cmd_non_tlv(wmi_unified_t wmi,
					uint8_t vdev_id)
{
	wmi_vdev_stop_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_vdev_stop_cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_stop_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = vdev_id;

	return wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_STOP_CMDID);
}

/**
 * send_vdev_down_cmd_non_tlv() - send vdev down command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_down_cmd_non_tlv(wmi_unified_t wmi_handle,
						uint8_t vdev_id)
{
	wmi_vdev_down_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_vdev_down_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_down_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = vdev_id;
	qdf_print("%s for vap %d (%pK)\n", __func__, vdev_id, wmi_handle);
	return wmi_unified_cmd_send(wmi_handle, buf, len, WMI_VDEV_DOWN_CMDID);
}

/**
 * send_vdev_start_cmd_non_tlv() - send vdev start command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_start_cmd_non_tlv(wmi_unified_t wmi,
				struct vdev_start_params *param)
{
	wmi_vdev_start_request_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_vdev_start_request_cmd);
	int ret;

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_vdev_start_request_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;

	cmd->chan.mhz = param->channel.mhz;

	WMI_SET_CHANNEL_MODE(&cmd->chan, param->channel.phy_mode);

	cmd->chan.band_center_freq1 = param->channel.cfreq1;
	cmd->chan.band_center_freq2 = param->channel.cfreq2;
	cmd->disable_hw_ack = param->disable_hw_ack;

	WMI_SET_CHANNEL_MIN_POWER(&cmd->chan, param->channel.minpower);
	WMI_SET_CHANNEL_MAX_POWER(&cmd->chan, param->channel.maxpower);
	WMI_SET_CHANNEL_REG_POWER(&cmd->chan, param->channel.maxregpower);
	WMI_SET_CHANNEL_ANTENNA_MAX(&cmd->chan, param->channel.antennamax);
	WMI_SET_CHANNEL_REG_CLASSID(&cmd->chan, param->channel.reg_class_id);

	if (param->channel.dfs_set)
		WMI_SET_CHANNEL_FLAG(&cmd->chan, WMI_CHAN_FLAG_DFS);

	if (param->channel.dfs_set_cfreq2)
		WMI_SET_CHANNEL_FLAG(&cmd->chan, WMI_CHAN_FLAG_DFS_CFREQ2);

	if (param->channel.set_agile)
		WMI_SET_CHANNEL_FLAG(&cmd->chan, WMI_CHAN_FLAG_AGILE_MODE);

	if (param->channel.half_rate)
		WMI_SET_CHANNEL_FLAG(&cmd->chan, WMI_CHAN_FLAG_HALF);

	if (param->channel.quarter_rate)
		WMI_SET_CHANNEL_FLAG(&cmd->chan, WMI_CHAN_FLAG_QUARTER);

	if (param->is_restart) {
		qdf_print("VDEV RESTART\n");
		ret =  wmi_unified_cmd_send(wmi, buf, len,
				WMI_VDEV_RESTART_REQUEST_CMDID);
	} else {
		qdf_print("VDEV START\n");
		ret =  wmi_unified_cmd_send(wmi, buf, len,
				WMI_VDEV_START_REQUEST_CMDID);
	}
	return ret;

/*
For VDEV_RESTART command, the sequence of code remains the same except the
command sent as WMI_VDEV_RESTART_REQUEST_CMDID instead of START_REQUEST.

In that case, can we introduce a flag that takes in to check if start or
restart and use the same function?? Currently implemented as two separate
functions in OL layer
*/
}

/**
 * send_vdev_set_nac_rssi_cmd_non_tlv() - send set NAC_RSSI command to fw
 * @wmi: wmi handle
 * @param: Pointer to hold nac rssi stats
 *
 * Return: 0 for success or error code
 */
QDF_STATUS send_vdev_set_nac_rssi_cmd_non_tlv(wmi_unified_t wmi,
				struct vdev_scan_nac_rssi_params *param)
{
	wmi_vdev_scan_nac_rssi_config_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_vdev_scan_nac_rssi_config_cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_vdev_scan_nac_rssi_config_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	cmd->action = param->action;
	cmd->chan_num = param->chan_num;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->bssid_addr, &cmd->bssid_addr);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->client_addr, &cmd->client_addr);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_SET_SCAN_NAC_RSSI_CMDID)) {
		qdf_print("%s: ERROR: Host unable to send LOWI request to FW\n", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_vdev_set_neighbour_rx_cmd_non_tlv() - set neighbour rx param in fw
 * @wmi_handle: wmi handle
 * @macaddr: vdev mac address
 * @param: pointer to hold neigbour rx param
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_set_neighbour_rx_cmd_non_tlv(wmi_unified_t wmi_handle,
					uint8_t macaddr[IEEE80211_ADDR_LEN],
					struct set_neighbour_rx_params *param)
{
	wmi_vdev_filter_nrp_config_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_vdev_filter_nrp_config_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_vdev_filter_nrp_config_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	cmd->bssid_idx = param->idx;
	cmd->action = param->action;
	cmd->type = param->type;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->addr);
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_VDEV_FILTER_NEIGHBOR_RX_PACKETS_CMDID);
}

/**
 * send_vdev_set_fwtest_param_cmd_non_tlv() - send fwtest param in fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold fwtest param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_set_fwtest_param_cmd_non_tlv(wmi_unified_t wmi_handle,
					struct set_fwtest_params *param)
{
	wmi_fwtest_set_param_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_fwtest_set_param_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_fwtest_set_param_cmd *)wmi_buf_data(buf);
	cmd->param_id = param->arg;
	cmd->param_value = param->value;

	return wmi_unified_cmd_send(wmi_handle, buf, len, WMI_FWTEST_CMDID);
}

/**
 * send_vdev_config_ratemask_cmd_non_tlv() - config ratemask param in fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold config ratemask params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_config_ratemask_cmd_non_tlv(wmi_unified_t wmi_handle,
					struct config_ratemask_params *param)
{
	wmi_vdev_config_ratemask *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_vdev_config_ratemask);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_vdev_config_ratemask *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	cmd->type	= param->type;
	cmd->mask_lower32 = param->lower32;
	cmd->mask_higher32 = param->higher32;
	qdf_print("Setting vdev ratemask vdev id = 0x%X, type = 0x%X,"
		"mask_l32 = 0x%X mask_h32 = 0x%X\n",
		param->vdev_id, param->type, param->lower32, param->higher32);
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_VDEV_RATEMASK_CMDID);
}

/**
 * send_setup_install_key_cmd_non_tlv() - config security key in fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold key params
 * @macaddr: vdev mac address
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_setup_install_key_cmd_non_tlv(wmi_unified_t wmi_handle,
					struct set_key_params  *param)
{
	wmi_vdev_install_key_cmd *cmd;
	wmi_buf_t buf;
	/* length depends on ieee key length */
	int len = sizeof(wmi_vdev_install_key_cmd) + param->key_len;
	uint8_t	wmi_cipher_type;
	int i;

	wmi_cipher_type = param->key_cipher;

	/* ieee_key length does not have mic keylen */
	if ((wmi_cipher_type == WMI_CIPHER_TKIP) ||
		(wmi_cipher_type == WMI_CIPHER_WAPI))
		len = len + IEEE80211_MICBUF_SIZE;

	len = roundup(len, sizeof(u_int32_t));
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_install_key_cmd *)wmi_buf_data(buf);

	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_mac, &cmd->peer_macaddr);

	cmd->key_ix = param->key_idx;

	/* If this WEP key is the default xmit key, TX_USAGE flag is enabled */
		cmd->key_flags  = param->key_flags;

		cmd->key_len = param->key_len;
		cmd->key_cipher = wmi_cipher_type;
		cmd->key_txmic_len = param->key_txmic_len;
		cmd->key_rxmic_len = param->key_rxmic_len;

	/* target will use the same rsc counter for
	   various tids from from ieee key rsc */
	if ((wmi_cipher_type == WMI_CIPHER_TKIP) ||
			(wmi_cipher_type == WMI_CIPHER_AES_OCB)
		|| (wmi_cipher_type == WMI_CIPHER_AES_CCM)) {
		qdf_mem_copy(&cmd->key_rsc_counter, &param->key_rsc_counter[0],
			sizeof(param->key_rsc_counter[0]));
		qdf_mem_copy(&cmd->key_tsc_counter, &param->key_tsc_counter,
				sizeof(param->key_tsc_counter));
	}

#ifdef ATH_SUPPORT_WAPI
	if (wmi_cipher_type == WMI_CIPHER_WAPI) {
		int j;
		/* For WAPI, TSC and RSC has to be initialized with predefined
		 * value.Here, Indicating TSC, RSC to target as part of set
		 * key message
		*/
		/* since wk_recviv and wk_txiv initialized in reverse order,
		 * Before indicating the Target FW, Reversing TSC and RSC
		 */
		for (i = (WPI_IV_LEN-1), j = 0; i >= 0; i--, j++) {
			cmd->wpi_key_rsc_counter[j] =
			    param->rx_iv[i];
			cmd->wpi_key_tsc_counter[j] =
			    param->tx_iv[i];
		}

		qdf_print("RSC:");
		for (i = 0; i < 16; i++)
			qdf_print("0x%x ",
				 *(((uint8_t *)&cmd->wpi_key_rsc_counter)+i));
		qdf_print("\n");

		qdf_print("TSC:");
		for (i = 0; i < 16; i++)
			qdf_print("0x%x ",
				*(((uint8_t *)&cmd->wpi_key_tsc_counter)+i));
		qdf_print("\n");
	}
#endif

	/* for big endian host, copy engine byte_swap is enabled
	 * But the key data content is in network byte order
	 * Need to byte swap the key data content - so when copy engine
	 * does byte_swap - target gets key_data content in the correct order
	 */

	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(cmd->key_data, param->key_data,
			cmd->key_len);

	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_VDEV_INSTALL_KEY_CMDID);

}

/**
 * send_peer_flush_tids_cmd_non_tlv() - flush peer tids packets in fw
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac address
 * @param: pointer to hold peer flush tid parameter
 *
 * Return: 0 for sucess or error code
 */
static QDF_STATUS send_peer_flush_tids_cmd_non_tlv(wmi_unified_t wmi_handle,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 struct peer_flush_params *param)
{
	wmi_peer_flush_tids_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_peer_flush_tids_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_peer_flush_tids_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->peer_tid_bitmap = param->peer_tid_bitmap;
	cmd->vdev_id = param->vdev_id;
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_FLUSH_TIDS_CMDID);
}

/**
 * send_peer_delete_cmd_non_tlv() - send PEER delete command to fw
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac addr
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_peer_delete_cmd_non_tlv(wmi_unified_t wmi_handle,
					uint8_t
					peer_addr[IEEE80211_ADDR_LEN],
					uint8_t vdev_id)
{
	wmi_peer_delete_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_peer_delete_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_peer_delete_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->vdev_id = vdev_id;
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_DELETE_CMDID);
}

/**
 * convert_host_peer_id_to_target_id_non_tlv - convert host peer param_id
 * to target id.
 * @targ_paramid: Target parameter id to hold the result.
 * @peer_param_id: host param id.
 *
 * Return: QDF_STATUS_SUCCESS for success
 *	 QDF_STATUS_E_NOSUPPORT when the param_id in not supported in tareget
 */
static QDF_STATUS convert_host_peer_id_to_target_id_non_tlv(
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
	case WMI_HOST_PEER_USE_FIXED_PWR:
		*targ_paramid = WMI_PEER_USE_FIXED_PWR;
		break;
	case WMI_HOST_PEER_PARAM_FIXED_RATE:
		*targ_paramid = WMI_PEER_PARAM_FIXED_RATE;
		break;
	case WMI_HOST_PEER_SET_MU_WHITELIST:
		*targ_paramid = WMI_PEER_SET_MU_WHITELIST;
		break;
	case WMI_HOST_PEER_EXT_STATS_ENABLE:
		*targ_paramid = WMI_PEER_EXT_STATS_ENABLE;
		break;
	case WMI_HOST_PEER_NSS_VHT160:
		*targ_paramid = WMI_PEER_NSS_VHT160;
		break;
	case WMI_HOST_PEER_NSS_VHT80_80:
		*targ_paramid = WMI_PEER_NSS_VHT80_80;
		break;
	default:
		return QDF_STATUS_E_NOSUPPORT;
	}

	return QDF_STATUS_SUCCESS;
}
/**
 * send_peer_param_cmd_non_tlv() - set peer parameter in fw
 * @wmi_handle: wmi handle
 * @peer_addr: peer mac address
 * @param	: pointer to hold peer set parameter
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_peer_param_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param)
{
	wmi_peer_set_param_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_peer_set_param_cmd);
	uint32_t param_id;

	if (convert_host_peer_id_to_target_id_non_tlv(&param_id,
					param->param_id) != QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_NOSUPPORT;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_peer_set_param_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->param_id = param_id;
	cmd->param_value = param->param_value;
	cmd->vdev_id = param->vdev_id;
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_SET_PARAM_CMDID);
}

/**
 * send_vdev_up_cmd_non_tlv() - send vdev up command in fw
 * @wmi_handle: wmi handle
 * @bssid: bssid
 * @vdev_up_params: pointer to hold vdev up parameter
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_up_cmd_non_tlv(wmi_unified_t wmi_handle,
				 uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *param)
{
	wmi_vdev_up_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_vdev_up_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_vdev_up_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	cmd->vdev_assoc_id = param->assoc_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(bssid, &cmd->vdev_bssid);
	qdf_print("%s for vap %d (%pK)\n", __func__, param->vdev_id, wmi_handle);
	return wmi_unified_cmd_send(wmi_handle, buf, len, WMI_VDEV_UP_CMDID);
}

/**
 * send_peer_create_cmd_non_tlv() - send peer create command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold peer create parameter
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_peer_create_cmd_non_tlv(wmi_unified_t wmi_handle,
					struct peer_create_params *param)
{
	wmi_peer_create_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_peer_create_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_peer_create_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_addr, &cmd->peer_macaddr);
	cmd->vdev_id = param->vdev_id;
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_CREATE_CMDID);
}

/**
 * send_peer_add_wds_entry_cmd_non_tlv() - send peer add command to fw
 * @wmi_handle: wmi handle
 * @param: pointer holding peer details
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_peer_add_wds_entry_cmd_non_tlv(wmi_unified_t wmi_handle,
					struct peer_add_wds_entry_params *param)
{
	wmi_peer_add_wds_entry_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_peer_add_wds_entry_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_peer_add_wds_entry_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->dest_addr, &cmd->wds_macaddr);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_addr, &cmd->peer_macaddr);
	cmd->flags = param->flags;
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_ADD_WDS_ENTRY_CMDID);
}

/**
 * send_peer_del_wds_entry_cmd_non_tlv() - send peer delete command to fw
 * @wmi_handle: wmi handle
 * @param: pointer holding peer details
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_peer_del_wds_entry_cmd_non_tlv(wmi_unified_t wmi_handle,
					struct peer_del_wds_entry_params *param)
{
	wmi_peer_remove_wds_entry_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_peer_remove_wds_entry_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_peer_remove_wds_entry_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->dest_addr, &cmd->wds_macaddr);
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_REMOVE_WDS_ENTRY_CMDID);
}

/**
 * send_set_bridge_mac_addr_cmd_non_tlv() - send set bridge MAC addr command to fw
 * @wmi_handle: wmi handle
 * @param: pointer holding bridge addr details
 *
 * Return: 0 for success or error code
 */
QDF_STATUS send_set_bridge_mac_addr_cmd_non_tlv(wmi_unified_t wmi_handle,
					struct set_bridge_mac_addr_params *param)
{
	wmi_peer_add_wds_entry_cmd *cmd;
	wmi_buf_t buf;
	uint8_t null_macaddr[IEEE80211_ADDR_LEN];
	int len = sizeof(wmi_peer_add_wds_entry_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_zero(null_macaddr, IEEE80211_ADDR_LEN);
	cmd = (wmi_peer_add_wds_entry_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->bridge_addr, &cmd->wds_macaddr);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(null_macaddr, &cmd->peer_macaddr);
	cmd->flags = 0xffffffff;

	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_ADD_WDS_ENTRY_CMDID);
}

/**
 * send_peer_update_wds_entry_cmd_non_tlv() - send peer update command to fw
 * @wmi_handle: wmi handle
 * @param: pointer holding peer details
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_peer_update_wds_entry_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct peer_update_wds_entry_params *param)
{
	wmi_peer_update_wds_entry_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_peer_update_wds_entry_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	/* wmi_buf_alloc returns zeroed command buffer */
	cmd = (wmi_peer_update_wds_entry_cmd *)wmi_buf_data(buf);
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

#ifdef WLAN_SUPPORT_GREEN_AP
/**
 * send_green_ap_ps_cmd_non_tlv() - enable green ap powersave command
 * @wmi_handle: wmi handle
 * @value: value
 * @pdev_id: pdev id to have radio context
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_green_ap_ps_cmd_non_tlv(wmi_unified_t wmi_handle,
						uint32_t value, uint8_t pdev_id)
{
	wmi_pdev_green_ap_ps_enable_cmd *cmd;
	wmi_buf_t buf;
	int len = 0;
	int ret;

	len = sizeof(wmi_pdev_green_ap_ps_enable_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_green_ap_ps_enable_cmd *)wmi_buf_data(buf);
	cmd->enable = value;

	ret = wmi_unified_cmd_send(wmi_handle,
				   buf,
				   len,
				   WMI_PDEV_GREEN_AP_PS_ENABLE_CMDID);

#ifdef OL_GREEN_AP_DEBUG_CONFIG_INTERACTIONS
	qdf_print("%s: Sent WMI_PDEV_GREEN_AP_PS_ENABLE_CMDID.\n"
				 "enable=%u status=%d\n",
				 __func__,
				 value,
				 ret);
#endif /* OL_GREEN_AP_DEBUG_CONFIG_INTERACTIONS */
	return ret;
}
#endif

/**
 * send_pdev_utf_cmd_non_tlv() - send utf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev_utf_params
 * @mac_id: mac id to have radio context
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_utf_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct pdev_utf_params *param,
				uint8_t mac_id)
{
	wmi_buf_t buf;
	u_int8_t *cmd;
	int ret = 0;
	/* We can initialize the value and increment.*/
	static uint8_t msgref = 1;
	uint8_t segNumber = 0, segInfo, numSegments;
	uint16_t  chunkLen, totalBytes;
	uint8_t *bufpos;
	struct seg_hdr_info segHdrInfo;

	bufpos = param->utf_payload;
	totalBytes = param->len;
	numSegments = (uint8_t) (totalBytes / MAX_WMI_UTF_LEN);

	if (param->len - (numSegments * MAX_WMI_UTF_LEN))
		numSegments++;

	while (param->len) {
		if (param->len > MAX_WMI_UTF_LEN)
			chunkLen = MAX_WMI_UTF_LEN; /* MAX messsage.. */
		else
			chunkLen = param->len;

		buf = wmi_buf_alloc(wmi_handle,
				(chunkLen + sizeof(segHdrInfo)));
		if (!buf) {
			qdf_print("%s:wmi_buf_alloc failed\n", __func__);
			return QDF_STATUS_E_FAILURE;
		}

		cmd = (uint8_t *)wmi_buf_data(buf);

		segHdrInfo.len = totalBytes;
		segHdrInfo.msgref =  msgref;
		segInfo = ((numSegments << 4) & 0xF0) | (segNumber & 0xF);
		segHdrInfo.segmentInfo = segInfo;

		segNumber++;

		qdf_mem_copy(cmd, &segHdrInfo, sizeof(segHdrInfo));
#ifdef BIG_ENDIAN_HOST
		if (param->is_ar900b) {

			/* for big endian host, copy engine byte_swap is
			 * enable But this ART command frame buffer content is
			 * in network byte order.
			 * Need to byte swap the mgmt frame buffer content - so
			 * when copy engine does byte_swap - target gets buffer
			 * content in the correct order
			*/
			int i;
			uint32_t *destp, *srcp;
			destp = (uint32_t *)(&(cmd[sizeof(segHdrInfo)]));
			srcp = (uint32_t *)bufpos;
			for (i = 0; i < (roundup(chunkLen,
						sizeof(uint32_t)) / 4); i++) {
				*destp = qdf_le32_to_cpu(*srcp);
				destp++; srcp++;
			}
		} else {
			qdf_mem_copy(&cmd[sizeof(segHdrInfo)],
					bufpos, chunkLen);
		}
#else
		qdf_mem_copy(&cmd[sizeof(segHdrInfo)], bufpos, chunkLen);
#endif

		ret =  wmi_unified_cmd_send(wmi_handle, buf,
				(chunkLen + sizeof(segHdrInfo)),
			WMI_PDEV_UTF_CMDID);

		if (ret != 0)
			break;

		param->len -= chunkLen;
		bufpos += chunkLen;
	}

	msgref++;

	return ret;
}

/**
 * send_pdev_qvit_cmd_non_tlv() - send qvit command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev_qvit_params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_qvit_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct pdev_qvit_params *param)
{
	wmi_buf_t buf;
	u_int8_t *cmd;
	int ret = 0;
	/* We can initialize the value and increment.*/
	static u_int8_t msgref = 1;
	u_int8_t segNumber = 0, segInfo, numSegments;
	u_int16_t  chunkLen, totalBytes;
	u_int8_t *bufpos;
	QVIT_SEG_HDR_INFO_STRUCT segHdrInfo;

/*
#ifdef QVIT_DEBUG
	qdf_print(KERN_INFO "QVIT: %s: called\n", __func__);
#endif
*/
	bufpos = param->utf_payload;
	totalBytes = param->len;
	numSegments = (totalBytes / MAX_WMI_QVIT_LEN);

	if (param->len - (numSegments * MAX_WMI_QVIT_LEN))
		numSegments++;

	while (param->len) {
		if (param->len > MAX_WMI_QVIT_LEN)
			chunkLen = MAX_WMI_QVIT_LEN; /* MAX messsage.. */
		else
			chunkLen = param->len;

		buf = wmi_buf_alloc(wmi_handle,
				(chunkLen + sizeof(segHdrInfo)));
		if (!buf) {
			qdf_print(KERN_ERR "QVIT: %s: wmi_buf_alloc failed\n",
					__func__);
			return QDF_STATUS_E_FAILURE;
		}

		cmd = (u_int8_t *)wmi_buf_data(buf);

		segHdrInfo.len = totalBytes;
		segHdrInfo.msgref =  msgref;
		segInfo = ((numSegments << 4) & 0xF0) | (segNumber & 0xF);
		segHdrInfo.segmentInfo = segInfo;

		segNumber++;

		qdf_mem_copy(cmd, &segHdrInfo, sizeof(segHdrInfo));
		qdf_mem_copy(&cmd[sizeof(segHdrInfo)], bufpos, chunkLen);

		ret =  wmi_unified_cmd_send(wmi_handle, buf,
				(chunkLen + sizeof(segHdrInfo)),
			WMI_PDEV_QVIT_CMDID);
		if (ret != 0) {
			qdf_print
			(KERN_ERR "QVIT: %s: wmi_unified_cmd_send failed\n",
					__func__);
			break;
		}

		param->len -= chunkLen;
		bufpos += chunkLen;
	}

	msgref++;

	return ret;
}

/**
 * send_pdev_param_cmd_non_tlv() - set pdev parameters
 * @wmi_handle: wmi handle
 * @param: pointer to pdev parameter
 * @mac_id: radio context
 *
 * Return: 0 on success, errno on failure
 */
static QDF_STATUS
send_pdev_param_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct pdev_params *param, uint8_t mac_id)
{
	wmi_pdev_set_param_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_pdev_set_param_cmd);

	if ((param->param_id < wmi_pdev_param_max) &&
		(wmi_handle->pdev_param[param->param_id]
				!= WMI_UNAVAILABLE_PARAM)) {

		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			qdf_print("%s:wmi_buf_alloc failed\n", __func__);
			return QDF_STATUS_E_FAILURE;
		}
		cmd = (wmi_pdev_set_param_cmd *)wmi_buf_data(buf);
		cmd->param_id = wmi_handle->pdev_param[param->param_id];
		cmd->param_value = param->param_value;
		return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_SET_PARAM_CMDID);
	}
	return QDF_STATUS_E_FAILURE;
}

/**
 *  send_suspend_cmd_non_tlv() - WMI suspend function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold suspend parameter
 *  @mac_id: radio context
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_suspend_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct suspend_params *param,
				uint8_t mac_id)
{
	wmi_pdev_suspend_cmd *cmd;
	wmi_buf_t wmibuf;
	uint32_t len = sizeof(wmi_pdev_suspend_cmd);

	/*send the comand to Target to ignore the
	* PCIE reset so as to ensure that Host and target
	* states are in sync*/
	wmibuf = wmi_buf_alloc(wmi_handle, len);
	if (wmibuf == NULL)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_pdev_suspend_cmd *)wmi_buf_data(wmibuf);
	if (param->disable_target_intr)
		cmd->suspend_opt = WMI_PDEV_SUSPEND_AND_DISABLE_INTR;
	else
		cmd->suspend_opt = WMI_PDEV_SUSPEND;

	/*
	 * Flush pending packets in HTC endpoint queue
	 *
	 */
	wmi_flush_endpoint(wmi_handle);

	return wmi_unified_cmd_send(wmi_handle, wmibuf, len,
			WMI_PDEV_SUSPEND_CMDID);
}

/**
 *  send_resume_cmd_non_tlv() - WMI resume function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @mac_id: radio context
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_resume_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t mac_id)
{
	wmi_buf_t wmibuf;

	wmibuf = wmi_buf_alloc(wmi_handle, 0);
	if (wmibuf == NULL)
		return QDF_STATUS_E_NOMEM;
	return wmi_unified_cmd_send(wmi_handle, wmibuf, 0,
			WMI_PDEV_RESUME_CMDID);
}

/**
 *  send_wow_enable_cmd_non_tlv() - WMI wow enable function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold wow enable parameter
 *  @mac_id: radio context
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_wow_enable_cmd_non_tlv(wmi_unified_t wmi_handle,
		struct wow_cmd_params *param, uint8_t mac_id)
{
	QDF_STATUS res;
	wmi_buf_t buf = NULL;

	buf = wmi_buf_alloc(wmi_handle, 4);
	if (!buf) {
		qdf_print("buf alloc failed\n");
		return QDF_STATUS_E_NOMEM;
	}
	res = wmi_unified_cmd_send(wmi_handle, buf, 4, WMI_WOW_ENABLE_CMDID);
	qdf_print("send_wow_enable result: %d\n", res);
	return (res == QDF_STATUS_SUCCESS) ?
		QDF_STATUS_SUCCESS : QDF_STATUS_E_FAILURE;
}

/**
 *  send_wow_wakeup_cmd_non_tlv() - WMI wow wakeup function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_wow_wakeup_cmd_non_tlv(wmi_unified_t wmi_handle)
{
	QDF_STATUS res;
	wmi_buf_t buf = NULL;

	buf = wmi_buf_alloc(wmi_handle, 4);
	if (!buf) {
		qdf_print("buf alloc failed\n");
		return QDF_STATUS_E_NOMEM;
	}
	res = wmi_unified_cmd_send(wmi_handle, buf, 4,
			WMI_WOW_HOSTWAKEUP_FROM_SLEEP_CMDID);
	qdf_print("ol_wow_wakeup result: %d\n", res);
	return (res == QDF_STATUS_SUCCESS) ?
		QDF_STATUS_SUCCESS : QDF_STATUS_E_FAILURE;
}

/**
 *  send_wow_add_wakeup_event_cmd_non_tlv() - WMI wow add wakeup event function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold wow wakeup event parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_wow_add_wakeup_event_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct wow_add_wakeup_params *param)
{
	QDF_STATUS res;
	WMI_WOW_ADD_DEL_EVT_CMD *cmd;
	wmi_buf_t buf = NULL;
	int len = sizeof(WMI_WOW_ADD_DEL_EVT_CMD);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("buf alloc failed\n");
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (WMI_WOW_ADD_DEL_EVT_CMD *)wmi_buf_data(buf);
	cmd->is_add = 1;
	cmd->event_bitmap = param->type;
	res = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_WOW_ENABLE_DISABLE_WAKE_EVENT_CMDID);
	return (res == QDF_STATUS_SUCCESS) ?
		QDF_STATUS_SUCCESS : QDF_STATUS_E_FAILURE;
}

/**
 *  send_wow_add_wakeup_pattern_cmd_non_tlv() - WMI wow add wakeup pattern function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold wow wakeup pattern parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_wow_add_wakeup_pattern_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct wow_add_wakeup_pattern_params *param)
{
	WOW_BITMAP_PATTERN_T bitmap_pattern;
	uint32_t j;
	/*
	struct ol_wow_info  *wowInfo;
	OL_WOW_PATTERN *pattern;
	struct ol_ath_softc_net80211 *scn = OL_ATH_SOFTC_NET80211(ic);
	*/
	QDF_STATUS res;
	WMI_WOW_ADD_PATTERN_CMD *cmd;
	wmi_buf_t buf = NULL;
	int len = sizeof(WMI_WOW_ADD_PATTERN_CMD);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("buf alloc failed\n");
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (WMI_WOW_ADD_PATTERN_CMD *)wmi_buf_data(buf);
	cmd->pattern_id = param->pattern_id;
	cmd->pattern_type = WOW_BITMAP_PATTERN;

	for (j = 0; j < WOW_DEFAULT_BITMAP_PATTERN_SIZE; j++)
		bitmap_pattern.patternbuf[j] = param->pattern_bytes[j];

	for (j = 0; j < WOW_DEFAULT_BITMASK_SIZE; j++)
		bitmap_pattern.bitmaskbuf[j] = param->mask_bytes[j];

	bitmap_pattern.pattern_offset = 0;

	cmd->pattern_info.bitmap = bitmap_pattern;
	res = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_WOW_ADD_WAKE_PATTERN_CMDID);

	return (res == QDF_STATUS_SUCCESS) ?
		QDF_STATUS_SUCCESS : QDF_STATUS_E_FAILURE;
}

/**
 *  send_wow_remove_wakeup_pattern_cmd_non_tlv() - WMI wow remove wakeup
 *  pattern function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold wow wakeup pattern parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_wow_remove_wakeup_pattern_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct wow_remove_wakeup_pattern_params *param)
{
	WMI_WOW_DEL_PATTERN_CMD *cmd;
	QDF_STATUS res;
	wmi_buf_t buf = NULL;
	int len = sizeof(WMI_WOW_DEL_PATTERN_CMD);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("buf alloc failed\n");
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (WMI_WOW_DEL_PATTERN_CMD *)wmi_buf_data(buf);
	cmd->pattern_id = param->pattern_id;
	cmd->pattern_type = WOW_BITMAP_PATTERN;
	res = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_WOW_DEL_WAKE_PATTERN_CMDID);
	return (res == QDF_STATUS_SUCCESS) ?
		QDF_STATUS_SUCCESS : QDF_STATUS_E_FAILURE;
}

/**
 * send_set_ap_ps_param_cmd_non_tlv() - set ap powersave parameters
 * @param wmi_handle	  : handle to WMI.
 * @peer_addr: peer mac address
 * @param: pointer to ap_ps parameter structure
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_set_ap_ps_param_cmd_non_tlv(wmi_unified_t wmi_handle,
					   uint8_t *peer_addr,
					   struct ap_ps_params *param)
{
	wmi_ap_ps_peer_cmd *cmd;
	wmi_buf_t buf;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_ap_ps_peer_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->param = param->param;
	cmd->value = param->value;

	return wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_AP_PS_PEER_PARAM_CMDID);
}

/**
 * send_set_sta_ps_param_cmd_non_tlv() - set sta powersave parameters
 * @param wmi_handle	  : handle to WMI.
 * @param: pointer to sta_ps parameter structure
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_set_sta_ps_param_cmd_non_tlv(wmi_unified_t wmi_handle,
					   struct sta_ps_params *param)
{
	wmi_sta_powersave_param_cmd *cmd;
	wmi_buf_t buf;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_sta_powersave_param_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	cmd->param = param->param;
	cmd->value = param->value;

	return wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_STA_POWERSAVE_PARAM_CMDID);
}

/**
 * send_set_ps_mode_cmd_non_tlv() - set powersave mode
 * @wmi_handle: wmi handle
 * @param: pointer to ps_mode parameter structure
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_set_ps_mode_cmd_non_tlv(wmi_unified_t wmi_handle,
					   struct set_ps_mode_params *param)
{
	wmi_sta_powersave_mode_cmd *cmd;
	wmi_buf_t buf;
	int ret;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_print("%s:set psmode=%d\n", __func__, param->psmode);
	cmd = (wmi_sta_powersave_mode_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	cmd->sta_ps_mode = param->psmode;

	ret =  wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_STA_POWERSAVE_MODE_CMDID);

	return ret;
}

/**
 * send_crash_inject_cmd_non_tlv() - inject fw crash
 * @param wmi_handle	  : handle to WMI.
 * @param: ponirt to crash inject paramter structure
 *
 * Return: 0 for success or return error
 */
static QDF_STATUS send_crash_inject_cmd_non_tlv(wmi_unified_t wmi_handle,
			 struct crash_inject *param)
{
	WMI_FORCE_FW_HANG_CMD *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(WMI_FORCE_FW_HANG_CMD);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (WMI_FORCE_FW_HANG_CMD *)wmi_buf_data(buf);
	cmd->type = 1;
	/* Should this be param->type ? */
	cmd->delay_time_ms = param->delay_time_ms;
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_FORCE_FW_HANG_CMDID);
}

/**
 *  send_dbglog_cmd_non_tlv() - set debug log level
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold dbglog level parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS
send_dbglog_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct dbglog_params *dbglog_param)
{
	wmi_buf_t osbuf;
	WMI_DBGLOG_CFG_CMD *cmd;
	QDF_STATUS status;

	osbuf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (osbuf == NULL)
		return QDF_STATUS_E_NOMEM;

	qdf_nbuf_put_tail(osbuf, sizeof(*cmd));

	cmd = (WMI_DBGLOG_CFG_CMD *)(wmi_buf_data(osbuf));

	qdf_print("wmi_dbg_cfg_send: mod[0]%08x dbgcfg%08x cfgvalid[0] %08x"
			" cfgvalid[1] %08x\n",
		dbglog_param->module_id_bitmap[0],
		dbglog_param->val, dbglog_param->cfgvalid[0],
		dbglog_param->cfgvalid[1]);

	cmd->config.cfgvalid[0] = dbglog_param->cfgvalid[0];
	cmd->config.cfgvalid[1] = dbglog_param->cfgvalid[1];
	qdf_mem_copy(&cmd->config.config.mod_id[0],
			dbglog_param->module_id_bitmap,
			sizeof(cmd->config.config.mod_id));
	cmd->config.config.dbg_config = dbglog_param->val;

	status = wmi_unified_cmd_send(wmi_handle, osbuf,
				   sizeof(WMI_DBGLOG_CFG_CMD),
				   WMI_DBGLOG_CFG_CMDID);

	return status;
}

/**
 *  send_vdev_set_param_cmd_non_tlv() - WMI vdev set parameter function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold vdev set parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_vdev_set_param_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct vdev_set_params *param)
{
	wmi_vdev_set_param_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_vdev_set_param_cmd);

	if ((param->param_id < wmi_vdev_param_max) &&
		(wmi_handle->vdev_param[param->param_id] !=
				WMI_UNAVAILABLE_PARAM)) {

		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			qdf_print("%s:wmi_buf_alloc failed\n", __func__);
			return QDF_STATUS_E_FAILURE;
		}
		cmd = (wmi_vdev_set_param_cmd *)wmi_buf_data(buf);
		cmd->vdev_id = param->if_id;
		cmd->param_id = wmi_handle->vdev_param[param->param_id];
		cmd->param_value = param->param_value;
		return wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_VDEV_SET_PARAM_CMDID);
	}
	return QDF_STATUS_E_FAILURE;
}

/**
 *  get_stats_id_non_tlv() - Get stats identifier function
 *
 *  @param host_stats_id: host stats identifier value
 *  @return stats_id based on host_stats_id
 */
static uint32_t get_stats_id_non_tlv(wmi_host_stats_id host_stats_id)
{
	uint32_t stats_id = 0;

	if (host_stats_id & WMI_HOST_REQUEST_PEER_STAT)
		stats_id |= WMI_REQUEST_PEER_STAT;
	if (host_stats_id & WMI_HOST_REQUEST_AP_STAT)
		stats_id |= WMI_REQUEST_AP_STAT;
	if (host_stats_id & WMI_HOST_REQUEST_INST_STAT)
		stats_id |= WMI_REQUEST_INST_STAT;
	if (host_stats_id & WMI_HOST_REQUEST_PEER_EXTD_STAT)
		stats_id |= WMI_REQUEST_PEER_EXTD_STAT;
	if (host_stats_id & WMI_HOST_REQUEST_NAC_RSSI)
		stats_id |= WMI_REQUEST_NAC_RSSI_STAT;

	return stats_id;
}
/**
 *  send_stats_request_cmd_non_tlv() - WMI request stats function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param macaddr		: MAC address
 *  @param param	: pointer to hold stats request parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_stats_request_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param)
{
	wmi_buf_t buf;
	wmi_request_stats_cmd *cmd;
	uint8_t len = sizeof(wmi_request_stats_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_INVAL;
	}

	cmd = (wmi_request_stats_cmd *)wmi_buf_data(buf);
	cmd->stats_id = get_stats_id_non_tlv(param->stats_id);
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->inst_rssi_args.cfg_retry_count = param->rssi_args.cfg_retry_count;
	cmd->inst_rssi_args.retry_count = param->rssi_args.retry_count;

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_REQUEST_STATS_CMDID)) {
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 *  send_bss_chan_info_request_cmd_non_tlv() - WMI request bss chan info
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold bss chan info request parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_bss_chan_info_request_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct bss_chan_info_request_params *param)
{
	wmi_buf_t buf;
	wmi_pdev_bss_chan_info_request *cmd;
	u_int8_t len = sizeof(wmi_pdev_bss_chan_info_request);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_INVAL;
	}

	cmd = (wmi_pdev_bss_chan_info_request *)wmi_buf_data(buf);
	cmd->param = param->param;
	cmd->reserved = 0;

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_PDEV_BSS_CHAN_INFO_REQUEST_CMDID)) {
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 *  send_packet_log_enable_cmd_non_tlv() - WMI request stats function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param PKTLOG_EVENT	: packet log event
 *  @mac_id: mac id to have radio context
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_packet_log_enable_cmd_non_tlv(wmi_unified_t wmi_handle,
			WMI_HOST_PKTLOG_EVENT PKTLOG_EVENT, uint8_t mac_id)
{
	wmi_pdev_pktlog_enable_cmd *cmd;
	int len = 0;
	wmi_buf_t buf;

	len = sizeof(wmi_pdev_pktlog_enable_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_pdev_pktlog_enable_cmd *)wmi_buf_data(buf);
	cmd->evlist = PKTLOG_EVENT;
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
		    WMI_PDEV_PKTLOG_ENABLE_CMDID)) {
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 *  send_packet_log_disable_cmd_non_tlv() - WMI disable packet log send function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @mac_id: mac id to have radio context
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_packet_log_disable_cmd_non_tlv(wmi_unified_t wmi_handle,
	uint8_t mac_id)
{
	int len = 0;
	wmi_buf_t buf;

	buf = wmi_buf_alloc(wmi_handle, 0);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
		    WMI_PDEV_PKTLOG_DISABLE_CMDID)) {
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 *  send_beacon_send_cmd_non_tlv() - WMI beacon send function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold beacon send cmd parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_beacon_send_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct beacon_params *param)
{
	if (param->is_high_latency) {
		wmi_bcn_tx_cmd *cmd;
		wmi_buf_t wmi_buf;
		int bcn_len = qdf_nbuf_len(param->wbuf);
		int len = sizeof(wmi_bcn_tx_hdr) + bcn_len;

		/*************************************************************
		 * TODO: Once we have the host target transport framework for
		 * sending management frames this wmi function will be replaced
		 * with calls to HTT. The buffer will changed to match the right
		 * format to be used with HTT.
		 *************************************************************/
		wmi_buf = wmi_buf_alloc(wmi_handle, roundup(len,
			    sizeof(u_int32_t)));
		if (!wmi_buf) {
			qdf_print("%s: wmi_buf_alloc failed\n", __func__);
			return QDF_STATUS_E_NOMEM;
		}
		cmd = (wmi_bcn_tx_cmd *)wmi_buf_data(wmi_buf);
		cmd->hdr.vdev_id = param->vdev_id;
		cmd->hdr.buf_len = bcn_len;

#ifdef BIG_ENDIAN_HOST
		{
			/* for big endian host, copy engine byte_swap is enabled
			 * But the beacon buffer content is in network byte
			 * order Need to byte swap the beacon buffer content -
			 * so when copy engine does byte_swap - target gets
			 * buffer content in the correct order
			 */
			int i;
			u_int32_t *destp, *srcp;
			destp = (u_int32_t *)cmd->bufp;
			srcp =  (u_int32_t *)wmi_buf_data(param->wbuf);
			for (i = 0; i < (roundup(bcn_len,
						sizeof(u_int32_t))/4); i++) {
				*destp = qdf_le32_to_cpu(*srcp);
				destp++; srcp++;
			}
		}
#else
		qdf_mem_copy(cmd->bufp, wmi_buf_data(param->wbuf), bcn_len);
#endif
#ifdef DEBUG_BEACON
		qdf_print("%s frm length %d\n", __func__, bcn_len);
#endif
		wmi_unified_cmd_send(wmi_handle, wmi_buf,
			roundup(len, sizeof(u_int32_t)), WMI_BCN_TX_CMDID);
	} else {
		wmi_bcn_send_from_host_cmd_t  *cmd;
		wmi_buf_t wmi_buf;
		int bcn_len = qdf_nbuf_len(param->wbuf);
		int len = sizeof(wmi_bcn_send_from_host_cmd_t);
		A_UINT32   dtim_flag = 0;

		/* get the DTIM count */

		if (param->is_dtim_count_zero) {
			dtim_flag |= WMI_BCN_SEND_DTIM_ZERO;
			if (param->is_bitctl_reqd) {
				/* deliver CAB traffic in next DTIM beacon */
				dtim_flag |= WMI_BCN_SEND_DTIM_BITCTL_SET;
			}
		}
		/* Map the beacon buffer to DMA region */

		wmi_buf = wmi_buf_alloc(wmi_handle, roundup(len,
			    sizeof(u_int32_t)));
		if (!wmi_buf) {
			qdf_print("%s: wmi_buf_alloc failed\n", __func__);
			return QDF_STATUS_E_NOMEM;
		}


		cmd = (wmi_bcn_send_from_host_cmd_t *)wmi_buf_data(wmi_buf);
		cmd->vdev_id = param->vdev_id;
		cmd->data_len = bcn_len;
		cmd->frame_ctrl = param->frame_ctrl;
		cmd->dtim_flag = dtim_flag;
		cmd->frag_ptr = qdf_nbuf_get_frag_paddr(param->wbuf, 0);
		cmd->virt_addr = (uintptr_t)param->wbuf;
		cmd->bcn_antenna = param->bcn_txant;
		wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				WMI_PDEV_SEND_BCN_CMDID);
	}
	return QDF_STATUS_SUCCESS;
}

#if 0
/**
 *  send_bcn_prb_template_cmd_non_tlv() - WMI beacon probe template function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param macaddr		: MAC address
 *  @param param	: pointer to hold beacon prb template cmd parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_bcn_prb_template_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct bcn_prb_template_params *param)
{
	wmi_bcn_prb_tmpl_cmd *cmd;
	wmi_buf_t buf;
	wmi_bcn_prb_info *template;
	int len = sizeof(wmi_bcn_prb_tmpl_cmd);
	int ret;

	/*
	 * The target will store this  information for use with
	 * the beacons and probes.
	 */
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_bcn_prb_tmpl_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	cmd->buf_len = param->buf_len;
	template = &cmd->bcn_prb_info;
	template->caps = param->caps;
	template->erp  = param->erp;

	/* TODO: Few more elements to be added and copied to the template
	 * buffer */

	/* Send the beacon probe template to the target */
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_BCN_PRB_TMPL_CMDID);
	return ret;
}
#endif

/**
 *  send_peer_assoc_cmd_non_tlv() - WMI peer assoc function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to peer assoc parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_peer_assoc_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct peer_assoc_params *param)
{
	wmi_peer_assoc_complete_cmd *cmd;
	int len = sizeof(wmi_peer_assoc_complete_cmd);
#ifdef BIG_ENDIAN_HOST
	int i;
#endif

	wmi_buf_t buf;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_peer_assoc_complete_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->peer_mac, &cmd->peer_macaddr);
	cmd->vdev_id = param->vdev_id;
	cmd->peer_new_assoc = param->peer_new_assoc;
	cmd->peer_associd = param->peer_associd;
	cmd->peer_bw_rxnss_override = 0;

	/*
	 * The target only needs a subset of the flags maintained in the host.
	 * Just populate those flags and send it down
	 */
	cmd->peer_flags = 0;

	if (param->is_pmf_enabled)
		cmd->peer_flags |= WMI_PEER_PMF_ENABLED;

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
		 * for HT as well */
		if (param->stbc_flag)
			cmd->peer_flags |= WMI_PEER_STBC;

		/* Typically if LDPC is enabled for VHT it should be enabled
		 * for HT as well */
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
		if (param->vht_ng_flag)
			cmd->peer_flags |= WMI_PEER_VHT_2G;
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
	 * it */
	if (param->amsdu_disable)
		cmd->peer_flags |= WMI_PEER_AMSDU_DISABLE;
	cmd->peer_caps = param->peer_caps;
	cmd->peer_listen_intval = param->peer_listen_intval;
	cmd->peer_ht_caps = param->peer_ht_caps;
	cmd->peer_max_mpdu = param->peer_max_mpdu;
	cmd->peer_mpdu_density = param->peer_mpdu_density;
	cmd->peer_vht_caps = param->peer_vht_caps;

	/* Update peer rate information */
	cmd->peer_rate_caps = param->peer_rate_caps;
	cmd->peer_legacy_rates.num_rates = param->peer_legacy_rates.num_rates;
	/* NOTE: cmd->peer_legacy_rates.rates is of type A_UINT32 */
	/* ni->ni_rates.rs_rates is of type u_int8_t */
	/**
	 * for cmd->peer_legacy_rates.rates:
	 * rates (each 8bit value) packed into a 32 bit word.
	 * the rates are filled from least significant byte to most
	 * significant byte.
	 */
	qdf_mem_copy(cmd->peer_legacy_rates.rates,
			param->peer_legacy_rates.rates,
			param->peer_legacy_rates.num_rates);
#ifdef BIG_ENDIAN_HOST
	for (i = 0;
		i < param->peer_legacy_rates.num_rates/sizeof(A_UINT32) + 1;
		i++)
		cmd->peer_legacy_rates.rates[i] =
		    qdf_le32_to_cpu(cmd->peer_legacy_rates.rates[i]);
#endif

	cmd->peer_ht_rates.num_rates = param->peer_ht_rates.num_rates;
	qdf_mem_copy(cmd->peer_ht_rates.rates, param->peer_ht_rates.rates,
			param->peer_ht_rates.num_rates);

#ifdef BIG_ENDIAN_HOST
	for (i = 0; i < param->peer_ht_rates.num_rates/sizeof(A_UINT32) + 1;
		i++)
		cmd->peer_ht_rates.rates[i] =
		    qdf_le32_to_cpu(cmd->peer_ht_rates.rates[i]);
#endif

	if (param->ht_flag &&
		(param->peer_ht_rates.num_rates == 0)) {
		/* Workaround for EV 116382: The node is marked HT but with
		 * supported rx mcs set is set to 0. 11n spec mandates MCS0-7
		 * for a HT STA. So forcing the supported rx mcs rate to MCS
		 * 0-7.
		 * This workaround will be removed once we get clarification
		 * from WFA regarding this STA behavior
		 */
		 u_int8_t temp_ni_rates[8] = {
			0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7};
		 cmd->peer_ht_rates.num_rates = 8;
		 qdf_mem_copy(cmd->peer_ht_rates.rates, temp_ni_rates,
			 cmd->peer_ht_rates.num_rates);
	}
	/* Target asserts if node is marked HT and all MCS is set to 0.
	   Mark the node as non-HT if all the mcs rates are disabled through
	   iwpriv */
	if (cmd->peer_ht_rates.num_rates == 0)
		cmd->peer_flags &= ~WMI_PEER_HT;

	cmd->peer_nss = param->peer_nss;

	if (param->vht_capable) {
		wmi_vht_rate_set *mcs;
		mcs = &cmd->peer_vht_rates;
		mcs->rx_max_rate = param->rx_max_rate;
		mcs->rx_mcs_set  = param->rx_mcs_set;
		mcs->tx_max_rate = param->tx_max_rate;
		mcs->tx_mcs_set  = param->tx_mcs_set;
		mcs->tx_max_mcs_nss = param->tx_max_mcs_nss;
	}

	cmd->peer_phymode = param->peer_phymode;
	/*Send bandwidth-NSS mapping to FW*/
	cmd->peer_bw_rxnss_override |= param->peer_bw_rxnss_override;

	return wmi_unified_cmd_send(wmi_handle, buf, len, WMI_PEER_ASSOC_CMDID);
}

/**
 *  send_scan_start_cmd_non_tlv() - WMI scan start function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold scan start cmd parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_scan_start_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct scan_req_params *param)
{
	wmi_start_scan_cmd *cmd;
	wmi_buf_t buf;
	wmi_chan_list *chan_list;
	wmi_bssid_list *bssid_list;
	wmi_ssid_list *ssid_list;
	wmi_ie_data *ie_data;
	A_UINT32 *tmp_ptr;
	int i, len = sizeof(wmi_start_scan_cmd);

#ifdef TEST_CODE
	len += sizeof(wmi_chan_list) + 3 * sizeof(A_UINT32);
#else
	if (param->chan_list.num_chan) {
		len += sizeof(wmi_chan_list) + (param->chan_list.num_chan - 1)
		    * sizeof(A_UINT32);
	}
#endif
	if (param->num_ssids) {
		len += sizeof(wmi_ssid_list) + (param->num_ssids - 1)
		    * sizeof(wmi_ssid);
	}
	if (param->num_bssid) {
		len += sizeof(wmi_bssid_list) + (param->num_bssid - 1)
		    * sizeof(wmi_mac_addr);
	}
	if (param->extraie.len) {
		i = param->extraie.len % sizeof(A_UINT32);
		if (i)
			len += sizeof(A_UINT32) - i;
		len += 2 * sizeof(A_UINT32) + param->extraie.len;
	}
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_start_scan_cmd *)wmi_buf_data(buf);
	OS_MEMZERO(cmd, len);
	cmd->vdev_id = param->vdev_id;
	cmd->scan_priority = param->scan_priority;
	cmd->scan_id = param->scan_id;
	cmd->scan_req_id = param->scan_req_id;

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
	if (param->scan_ev_invalid)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_INVALID;
	if (param->scan_ev_gpio_timeout)
		cmd->notify_scan_events |= WMI_SCAN_EVENT_GPIO_TIMEOUT;

	/** Max. active channel dwell time */
	cmd->dwell_time_active = param->dwell_time_active;
	/** Passive channel dwell time */
	cmd->dwell_time_passive = param->dwell_time_passive;

	/** Scan control flags */
	cmd->scan_ctrl_flags = 0;
	if (param->scan_f_passive)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_PASSIVE;
	if (param->scan_f_strict_passive_pch)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_STRICT_PASSIVE_ON_PCHN;
	if (param->scan_f_promisc_mode)
		cmd->scan_ctrl_flags |= WMI_SCAN_PROMISCOUS_MODE;
	if (param->scan_f_capture_phy_err)
		cmd->scan_ctrl_flags |= WMI_SCAN_CAPTURE_PHY_ERROR;
	if (param->scan_f_half_rate)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_HALF_RATE_SUPPORT;
	if (param->scan_f_quarter_rate)
		cmd->scan_ctrl_flags |= WMI_SCAN_FLAG_QUARTER_RATE_SUPPORT;
	if (param->scan_f_cck_rates)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_CCK_RATES;
	if (param->scan_f_chan_stat_evnt)
		cmd->scan_ctrl_flags |= WMI_SCAN_CHAN_STAT_EVENT;
	if (param->scan_f_bcast_probe)
		cmd->scan_ctrl_flags |= WMI_SCAN_ADD_BCAST_PROBE_REQ;
	if (param->scan_f_offchan_mgmt_tx)
		cmd->scan_ctrl_flags |= WMI_SCAN_OFFCHAN_MGMT_TX;
	if (param->scan_f_offchan_data_tx)
		cmd->scan_ctrl_flags |= WMI_SCAN_OFFCHAN_DATA_TX;
	/* Always enable ofdm rates */
	cmd->scan_ctrl_flags |= WMI_SCAN_ADD_OFDM_RATES;

	/** send multiple braodcast probe req with this delay in between */
	cmd->repeat_probe_time = param->repeat_probe_time;
	cmd->probe_spacing_time = param->probe_spacing_time;
	/** delay between channel change and first probe request */
	cmd->probe_delay = param->probe_delay;
	/** idle time on channel for which if no traffic is seen
		then scanner can switch to off channel */
	cmd->idle_time = param->idle_time;
	cmd->min_rest_time = param->min_rest_time;
	/** maximum rest time allowed on bss channel, overwrites
	 *  other conditions and changes channel to off channel
	 *   even if min beacon count, idle time requirements are not met.
	 */
	cmd->max_rest_time = param->max_rest_time;
	/** maxmimum scan time allowed */
#if IPQ4019_EMU
	cmd->max_scan_time = 0xffffffff;
#else
	cmd->max_scan_time = param->max_scan_time;
#endif
	tmp_ptr = (A_UINT32 *)  (cmd + 1);
#ifdef TEST_CODE
#define DEFAULT_TIME 150
	 cmd->min_rest_time = DEFAULT_TIME;
	 cmd->idle_time = 10*DEFAULT_TIME;
	 cmd->max_rest_time = 30*DEFAULT_TIME;
	 chan_list  = (wmi_chan_list *) tmp_ptr;
	 chan_list->tag = WMI_CHAN_LIST_TAG;
	 chan_list->num_chan = 4;
	 chan_list->channel_list[0] = 2412;  /* 1 */
	 chan_list->channel_list[1] = 2437;  /* 6 */
	 chan_list->channel_list[2] = 5180;  /* 36 */-
	 chan_list->channel_list[3] = 5680;  /* 136 */
	 tmp_ptr +=  (2 + chan_list->num_chan); /* increase by words */-
#else
#define FREQUENCY_THRESH 1000
	if (param->chan_list.num_chan) {
		chan_list  = (wmi_chan_list *) tmp_ptr;
		chan_list->tag = WMI_CHAN_LIST_TAG;
		chan_list->num_chan = param->chan_list.num_chan;
		for (i = 0; i < param->chan_list.num_chan; ++i)
			chan_list->channel_list[i] =
				param->chan_list.chan[i].freq;
		tmp_ptr += (2 + param->chan_list.num_chan);
	}
#endif
	if (param->num_ssids) {
		ssid_list  = (wmi_ssid_list *) tmp_ptr;
		ssid_list->tag = WMI_SSID_LIST_TAG;
		ssid_list->num_ssids = param->num_ssids;
		for (i = 0; i < param->num_ssids; ++i) {
			ssid_list->ssids[i].ssid_len = param->ssid[i].length;
			WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(
					ssid_list->ssids[i].ssid,
					param->ssid[i].ssid,
					param->ssid[i].length);
		}
		tmp_ptr +=  (2 + (sizeof(wmi_ssid) *
			    param->num_ssids)/sizeof(A_UINT32));
	}
	if (param->num_bssid) {
		bssid_list  = (wmi_bssid_list *) tmp_ptr;
		bssid_list->tag = WMI_BSSID_LIST_TAG;
		bssid_list->num_bssid = param->num_bssid;
		for (i = 0; i < param->num_bssid; ++i) {
			WMI_CHAR_ARRAY_TO_MAC_ADDR(
				&(param->bssid_list[i].bytes[0]),
				&bssid_list->bssid_list[i]);
		}
		tmp_ptr +=  (2 + (sizeof(wmi_mac_addr) *
			    param->num_bssid)/sizeof(A_UINT32));
	}
	if (param->extraie.len) {
		ie_data  = (wmi_ie_data *) tmp_ptr;
		ie_data->tag = WMI_IE_TAG;
		ie_data->ie_len = param->extraie.len;
		WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(ie_data->ie_data,
				param->extraie.ptr, param->extraie.len);
	}
	qdf_print("Sending SCAN START cmd\n");
	return wmi_unified_cmd_send(wmi_handle, buf, len, WMI_START_SCAN_CMDID);
}

/**
 *  send_scan_stop_cmd_non_tlv() - WMI scan stop function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold scan start cmd parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_scan_stop_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct scan_cancel_param *param)
{
	wmi_stop_scan_cmd *cmd = NULL;
	wmi_buf_t buf;
	u_int32_t len = sizeof(wmi_stop_scan_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_stop_scan_cmd *)wmi_buf_data(buf);
	OS_MEMZERO(cmd, len);
	/* scan scheduler is not supportd yet */
	cmd->scan_id = param->scan_id;
	cmd->requestor = param->requester;
	cmd->vdev_id = param->vdev_id;

	if (param->req_type == WLAN_SCAN_CANCEL_PDEV_ALL) {
		/* Cancelling all scans - always match scan id */
		cmd->req_type = WMI_SCAN_STOP_ALL;
	} else if (param->req_type == WLAN_SCAN_CANCEL_VDEV_ALL) {
		/*-
		 * Cancelling VAP scans - report a match if scan was requested
		 * by the same VAP trying to cancel it.
		 */
		cmd->req_type = WMI_SCN_STOP_VAP_ALL;
	} else if (param->req_type == WLAN_SCAN_CANCEL_SINGLE) {
		/*-
		 * Cancelling specific scan - report a match if specified scan
		 * id matches the request's scan id.
		 */
		cmd->req_type = WMI_SCAN_STOP_ONE;
	}

	wmi_unified_cmd_send(wmi_handle, buf, len, WMI_STOP_SCAN_CMDID);

	return QDF_STATUS_SUCCESS;
}

/**
 *  send_scan_chan_list_cmd_non_tlv() - WMI scan channel list function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold scan channel list parameter
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_scan_chan_list_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct scan_chan_list_params *param)
{
	uint32_t i;
	wmi_buf_t buf;
	wmi_scan_chan_list_cmd *cmd;
	int len = sizeof(wmi_scan_chan_list_cmd);

	len = sizeof(wmi_scan_chan_list_cmd) +
	    sizeof(wmi_channel)*param->nallchans;
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_scan_chan_list_cmd *)wmi_buf_data(buf);
	cmd->num_scan_chans = param->nallchans;
	OS_MEMZERO(cmd->chan_info, sizeof(wmi_channel)*cmd->num_scan_chans);


	for (i = 0; i < param->nallchans; ++i) {
		cmd->chan_info[i].mhz = param->ch_param[i].mhz;

		if (param->ch_param[i].is_chan_passive)
			WMI_SET_CHANNEL_FLAG(&(cmd->chan_info[i]),
					WMI_CHAN_FLAG_PASSIVE);

		if (param->ch_param[i].allow_vht)
			WMI_SET_CHANNEL_FLAG(&(cmd->chan_info[i]),
					WMI_CHAN_FLAG_ALLOW_VHT);
		else  if (param->ch_param[i].allow_ht)
			WMI_SET_CHANNEL_FLAG(&(cmd->chan_info[i]),
					WMI_CHAN_FLAG_ALLOW_HT);

		cmd->chan_info[i].band_center_freq1 =
			param->ch_param[i].cfreq1;
		cmd->chan_info[i].band_center_freq2 =
			param->ch_param[i].cfreq2;
		WMI_SET_CHANNEL_MODE(&cmd->chan_info[i],
				param->ch_param[i].phy_mode);

		if (param->ch_param[i].half_rate)
			WMI_SET_CHANNEL_FLAG(&(cmd->chan_info[i]),
					WMI_CHAN_FLAG_HALF);
		if (param->ch_param[i].quarter_rate)
			WMI_SET_CHANNEL_FLAG(&(cmd->chan_info[i]),
					WMI_CHAN_FLAG_QUARTER);

		/* also fill in power information */
		WMI_SET_CHANNEL_MIN_POWER(&cmd->chan_info[i],
				param->ch_param[i].minpower);
		WMI_SET_CHANNEL_MAX_POWER(&cmd->chan_info[i],
				param->ch_param[i].maxpower);
		WMI_SET_CHANNEL_REG_POWER(&cmd->chan_info[i],
				param->ch_param[i].maxregpower);
		WMI_SET_CHANNEL_ANTENNA_MAX(&cmd->chan_info[i],
				param->ch_param[i].antennamax);
		WMI_SET_CHANNEL_REG_CLASSID(&cmd->chan_info[i],
				param->ch_param[i].reg_class_id);
	}

	wmi_unified_cmd_send(wmi_handle, buf, len, WMI_SCAN_CHAN_LIST_CMDID);
	return QDF_STATUS_SUCCESS;
}

/**
 *  send_thermal_mitigation_param_cmd_non_tlv() - WMI scan channel list function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to hold thermal mitigation param
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_thermal_mitigation_param_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct thermal_mitigation_params *param)
{
	wmi_buf_t buf = NULL;
	tt_config_t *cmd = NULL;
	int error = 0;
	int32_t len = 0;
	int i = 0;

	len = sizeof(tt_config_t);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (tt_config_t *) wmi_buf_data(buf);
	cmd->enable = param->enable;
	cmd->dc = param->dc;
	cmd->dc_per_event = param->dc_per_event;
	for (i = 0; i < THERMAL_LEVELS; i++) {
		cmd->levelconf[i].tmplwm = param->levelconf[i].tmplwm;
		cmd->levelconf[i].tmphwm = param->levelconf[i].tmphwm;
		cmd->levelconf[i].dcoffpercent =
		    param->levelconf[i].dcoffpercent;
		cmd->levelconf[i].prio = param->levelconf[i].priority;
	}

	error = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_TT_SET_CONF_CMDID);
	return error;
}

/**
 *  send_phyerr_enable_cmd_non_tlv() - WMI phyerr enable function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_phyerr_enable_cmd_non_tlv(wmi_unified_t wmi_handle)
{
	wmi_buf_t buf;

	/*
	 * Passing a NULL pointer to wmi_unified_cmd_send() panics it,
	 * so let's just use a 32 byte fake array for now.
	 */
	buf = wmi_buf_alloc(wmi_handle, 32);
	if (buf == NULL) {
		/* XXX error? */
		return QDF_STATUS_E_NOMEM;
	}

	qdf_print("%s: about to send\n", __func__);
	if (wmi_unified_cmd_send(wmi_handle, buf, 32,
	  WMI_PDEV_DFS_ENABLE_CMDID) != QDF_STATUS_SUCCESS) {
		qdf_print("%s: send failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 *  send_phyerr_disable_cmd_non_tlv() - WMI phyerr disable function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_phyerr_disable_cmd_non_tlv(wmi_unified_t wmi_handle)
{
	wmi_buf_t buf;

	/*
	 * Passing a NULL pointer to wmi_unified_cmd_send() panics it,
	 * so let's just use a 32 byte fake array for now.
	 */
	buf = wmi_buf_alloc(wmi_handle, 32);
	if (buf == NULL) {
		/* XXX error? */
		return QDF_STATUS_E_NOMEM;
	}

	qdf_print("%s: about to send\n", __func__);
	if (wmi_unified_cmd_send(wmi_handle, buf, 32,
	  WMI_PDEV_DFS_DISABLE_CMDID) != QDF_STATUS_SUCCESS) {
		qdf_print("%s: send failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 *  send_smart_ant_enable_cmd_non_tlv() - WMI smart ant enable function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param	: pointer to antenna param
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_enable_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct smart_ant_enable_params *param)
{
	/* Send WMI COMMAND to Enable */
	wmi_pdev_smart_ant_enable_cmd *cmd;
	wmi_buf_t buf;
	int len = 0;
	int ret;

	len = sizeof(wmi_pdev_smart_ant_enable_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_smart_ant_enable_cmd *)wmi_buf_data(buf);
	cmd->enable = param->enable;
	cmd->mode = param->mode;
	cmd->rx_antenna = param->rx_antenna;
	cmd->tx_default_antenna = param->rx_antenna;

	if (param->mode == SMART_ANT_MODE_SERIAL) {
		cmd->gpio_pin[0] = param->gpio_pin[0];
		cmd->gpio_pin[1] = param->gpio_pin[1];
		cmd->gpio_pin[2] = 0;
		cmd->gpio_pin[3] = 0;

		cmd->gpio_func[0] = param->gpio_func[0];
		cmd->gpio_func[1] = param->gpio_func[1];
		cmd->gpio_func[2] = 0;
		cmd->gpio_func[3] = 0;

	} else if (param->mode == SMART_ANT_MODE_PARALLEL) {
		cmd->gpio_pin[0] = param->gpio_pin[0];
		cmd->gpio_pin[1] = param->gpio_pin[1];
		cmd->gpio_pin[2] = param->gpio_pin[2];
		cmd->gpio_pin[3] = param->gpio_pin[3];

		cmd->gpio_func[0] = param->gpio_func[0];
		cmd->gpio_func[1] = param->gpio_func[1];
		cmd->gpio_func[2] = param->gpio_func[2];
		cmd->gpio_func[3] = param->gpio_func[3];
	}

	ret = wmi_unified_cmd_send(wmi_handle,
				   buf,
				   len,
				   WMI_PDEV_SMART_ANT_ENABLE_CMDID);

	if (ret != 0) {
		qdf_print(" %s :WMI Failed\n", __func__);
		qdf_print("%s: Failed to send WMI_PDEV_SMART_ANT_ENABLE_CMDID.\n"
			  "enable:%d mode:%d  rx_antenna: 0x%08x PINS: "
			  "[%d %d %d %d] Func[%d %d %d %d] cmdstatus=%d\n",
			  __func__,
			  cmd->enable,
			  cmd->mode,
			  cmd->rx_antenna,
			  cmd->gpio_pin[0],
			  cmd->gpio_pin[1],
			  cmd->gpio_pin[2],
			  cmd->gpio_pin[3],
			  cmd->gpio_func[0],
			  cmd->gpio_func[1],
			  cmd->gpio_func[2],
			  cmd->gpio_func[3],
			  ret);
		wmi_buf_free(buf);
	}
	return ret;
}
/**
 *  send_smart_ant_set_rx_ant_cmd_non_tlv() - WMI set rx antenna function
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @param param		   : pointer to rx antenna param
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_set_rx_ant_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct smart_ant_rx_ant_params *param)
{
	wmi_pdev_smart_ant_set_rx_antenna_cmd *cmd;
	wmi_buf_t buf;
	int len = 0;
	int ret;

	len = sizeof(wmi_pdev_smart_ant_set_rx_antenna_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_smart_ant_set_rx_antenna_cmd *)wmi_buf_data(buf);
	cmd->rx_antenna = param->antenna;
	ret = wmi_unified_cmd_send(wmi_handle,
				   buf,
				   len,
				   WMI_PDEV_SMART_ANT_SET_RX_ANTENNA_CMDID);

	if (ret != 0) {
		qdf_print(" %s :WMI Failed\n", __func__);
		qdf_print("%s: Failed to send WMI_PDEV_SMART_ANT_SET_RX_ANTENNA_CMDID.\n"
			  " rx_antenna: 0x%08x cmdstatus=%d\n",
			  __func__,
			  cmd->rx_antenna,
			  ret);
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 *  send_smart_ant_set_tx_ant_cmd_non_tlv() - WMI set tx antenna function
 *  @param wmi_handle	  : handle to WMI.
 *  @param macaddr	: vdev mac address
 *  @param param		   : pointer to tx antenna param
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_set_tx_ant_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct smart_ant_tx_ant_params *param)
{
	wmi_peer_sant_set_tx_antenna_cmd *cmd;
	wmi_buf_t buf;
	int len = 0;
	int ret;

	len = sizeof(wmi_peer_sant_set_tx_antenna_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_peer_sant_set_tx_antenna_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);

	cmd->antenna_series[0] = param->antenna_array[0];
	cmd->antenna_series[1] = param->antenna_array[1];
	ret = wmi_unified_cmd_send(wmi_handle,
				   buf,
				   len,
				   WMI_PEER_SMART_ANT_SET_TX_ANTENNA_CMDID);

	if (ret != 0) {
		qdf_print(" %s :WMI Failed\n", __func__);
		qdf_print("%s: Failed to send WMI_PEER_SMART_ANT_SET_TX_ANTENNA_CMDID.\n"
			" Node: %s tx_antennas: [0x%08x 0x%08x] cmdstatus=%d\n",
				__func__,
				ether_sprintf(macaddr),
				cmd->antenna_series[0],
				cmd->antenna_series[1],
				ret);
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 *  send_smart_ant_set_training_info_cmd_non_tlv() - WMI set smart antenna
 *  training information function
 *  @param wmi_handle	  : handle to WMI.
 *  @macaddr			   : vdev mac address
 *  @param param		   : pointer to tx antenna param
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_set_training_info_cmd_non_tlv(
				wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct smart_ant_training_info_params *param)
{
	wmi_peer_sant_set_train_antenna_cmd *cmd;
	wmi_buf_t buf;
	int len = 0;
	int ret;

	len = sizeof(wmi_peer_sant_set_train_antenna_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_peer_sant_set_train_antenna_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	qdf_mem_copy(&cmd->train_rate_series[0], &param->rate_array[0],
			(sizeof(uint32_t)*SMART_ANT_MAX_RATE_SERIES));
	qdf_mem_copy(&cmd->train_antenna_series[0], &param->antenna_array[0],
			(sizeof(uint32_t)*SMART_ANT_MAX_RATE_SERIES));
	cmd->num_pkts = param->numpkts;
	ret = wmi_unified_cmd_send(wmi_handle,
				   buf,
				   len,
				   WMI_PEER_SMART_ANT_SET_TRAIN_INFO_CMDID);

	if (ret != 0) {
		qdf_print(" %s :WMI Failed\n", __func__);
		qdf_print("%s: Failed to Send WMI_PEER_SMART_ANT_SET_TRAIN_INFO_CMDID.\n"
			" Train Node: %s rate_array[0x%02x 0x%02x] "
			"tx_antennas: [0x%08x 0x%08x] cmdstatus=%d\n",
			__func__,
			ether_sprintf(macaddr),
			cmd->train_rate_series[0],
			cmd->train_rate_series[1],
			cmd->train_antenna_series[0],
			cmd->train_antenna_series[1],
			ret);
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 *  send_smart_ant_set_node_config_cmd_non_tlv() - WMI set node
 *  configuration function
 *  @param wmi_handle	  : handle to WMI.
 *  @macaddr			   : vdev mad address
 *  @param param		   : pointer to tx antenna param
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_set_node_config_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct smart_ant_node_config_params *param)
{
	wmi_peer_sant_set_node_config_ops_cmd *cmd;
	wmi_buf_t buf;
	int len = 0;
	int ret;
	int i = 0;

	len = sizeof(wmi_peer_sant_set_node_config_ops_cmd);

	if ((param->args_count == 0) || (param->args_count >
		   (sizeof(cmd->args) / sizeof(cmd->args[0])))) {
		qdf_print("%s: Can't send a command with %d arguments\n",
				__func__, param->args_count);
		return QDF_STATUS_E_FAILURE;
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_peer_sant_set_node_config_ops_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->cmd_id = param->cmd_id;
	cmd->args_count = param->args_count;
	for (i = 0; i < param->args_count; i++)
		cmd->args[i] = param->args_arr[i];

	ret = wmi_unified_cmd_send(wmi_handle,
			   buf,
			   len,
			   WMI_PEER_SMART_ANT_SET_NODE_CONFIG_OPS_CMDID);

	if (ret != 0) {
		qdf_print(" %s :WMI Failed\n", __func__);
		qdf_print("%s: Sent "
			"WMI_PEER_SMART_ANT_SET_NODE_CONFIG_OPS_CMDID, cmd_id:"
			" 0x%x\n Node: %s cmdstatus=%d\n",
			__func__, param->cmd_id, ether_sprintf(macaddr), ret);
	}
	return ret;
}

/**
 *  send_smart_ant_enable_tx_feedback_cmd_non_tlv() - WMI enable smart antenna
 *  tx feedback function
 *  @param wmi_handle	  : handle to WMI.
 *  @param param		   : pointer to hold enable param
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_enable_tx_feedback_cmd_non_tlv(
			wmi_unified_t wmi_handle,
			struct smart_ant_enable_tx_feedback_params *param)
{
	uint32_t types = 0;
	int len = 0;
	wmi_buf_t buf;
	wmi_pdev_pktlog_enable_cmd *cmd;

	if (param->enable == 1) {
		types |= WMI_PKTLOG_EVENT_TX;
		types |= WMI_PKTLOG_EVENT_SMART_ANTENNA;

		len = sizeof(wmi_pdev_pktlog_enable_cmd);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			qdf_print("%s:wmi_buf_alloc failed\n", __func__);
			return QDF_STATUS_E_FAILURE;
		}
		cmd = (wmi_pdev_pktlog_enable_cmd *)wmi_buf_data(buf);
		cmd->evlist = types;
		/*enabling the pktlog for smart antenna tx feedback*/
		if (wmi_unified_cmd_send(wmi_handle, buf, len,
					WMI_PDEV_PKTLOG_ENABLE_CMDID))
			return QDF_STATUS_E_FAILURE;
		return QDF_STATUS_SUCCESS;
	} else if (param->enable == 0) {
		buf = wmi_buf_alloc(wmi_handle, 0);
		if (!buf) {
			qdf_print("%s:wmi_buf_alloc failed\n", __func__);
			return QDF_STATUS_E_FAILURE;
		}
		if (!wmi_unified_cmd_send(wmi_handle, buf, len,
					WMI_PDEV_PKTLOG_DISABLE_CMDID))
			return QDF_STATUS_E_FAILURE;
		return QDF_STATUS_SUCCESS;
	} else
		return QDF_STATUS_E_FAILURE;
}

/**
 * send_vdev_spectral_configure_cmd_non_tlv() - send VDEV spectral configure
 * command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold spectral config parameter
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_spectral_configure_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct vdev_spectral_configure_params *param)
{
	wmi_vdev_spectral_configure_cmd *cmd;
	wmi_buf_t buf;
	int len = 0;
	int ret;

	len = sizeof(wmi_vdev_spectral_configure_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_vdev_spectral_configure_cmd *)wmi_buf_data(buf);

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
	cmd->spectral_scan_dBm_adj = param->dbm_adj;
	cmd->spectral_scan_chn_mask = param->chn_mask;

	ret = wmi_unified_cmd_send(wmi_handle,
			   buf,
			   len,
			   WMI_VDEV_SPECTRAL_SCAN_CONFIGURE_CMDID);
#ifdef OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS
	qdf_print("%s: Sent "
		 "WMI_VDEV_SPECTRAL_SCAN_CONFIGURE_CMDID\n", __func__);

	qdf_print("vdev_id = %u\n"
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
			 param->dbm_adj,
			 param->chn_mask);
	qdf_print("%s: Status: %d\n\n", __func__, ret);
#endif  /* OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS */

	return ret;
}

#ifdef WLAN_SUPPORT_FILS
/**
 *  send_fils_discovery_send_cmd_non_tlv() - WMI FILS Discovery send function
 *  @wmi_handle:  handle to WMI
 *  @param:  pointer to hold FD send cmd parameter
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS error code on failure.
 */
static QDF_STATUS
send_fils_discovery_send_cmd_non_tlv(wmi_unified_t wmi_handle,
				     struct fd_params *param)
{
	wmi_fd_send_from_host_cmd_t  *cmd;
	wmi_buf_t wmi_buf;
	QDF_STATUS status;
	int fd_len = qdf_nbuf_len(param->wbuf);
	int len = sizeof(wmi_fd_send_from_host_cmd_t);

	wmi_buf = wmi_buf_alloc(wmi_handle, roundup(len, sizeof(u_int32_t)));
	if (!wmi_buf) {
		WMI_LOGE("wmi_buf_alloc failed\n");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_fd_send_from_host_cmd_t *)wmi_buf_data(wmi_buf);
	cmd->vdev_id = param->vdev_id;
	cmd->data_len = fd_len;
	cmd->frag_ptr = qdf_nbuf_get_frag_paddr(param->wbuf, 0);
	cmd->frame_ctrl = param->frame_ctrl;
	status = wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				      WMI_PDEV_SEND_FD_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_vdev_fils_enable_cmd_non_tlv() - enable/Disable FD Frame command to fw
 * @wmi_handle:  wmi handle
 * @param:  pointer to hold FILS discovery enable param
 *
 * Return: QDF_STATUS_SUCCESS on success or QDF_STATUS error code on failure
 */
static QDF_STATUS
send_vdev_fils_enable_cmd_non_tlv(wmi_unified_t wmi_handle,
				  struct config_fils_params *param)
{
	wmi_enable_fils_cmd *cmd;
	wmi_buf_t buf;
	QDF_STATUS status;
	int len = sizeof(wmi_enable_fils_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("wmi_buf_alloc failed\n");
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_enable_fils_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	cmd->fd_period = param->fd_period;
	WMI_LOGI("Setting FD period to %d vdev id : %d\n",
		 param->fd_period, param->vdev_id);

	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_ENABLE_FILS_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_swfda_vdev_id_non_tlv() - extract swfda vdev id from event
 * @wmi_handle:  wmi handle
 * @evt_buf:  pointer to event buffer
 * @vdev_id:  pointer to hold vdev id
 *
 * Return: QDF_STATUS_SUCCESS
 */
static QDF_STATUS
extract_swfda_vdev_id_non_tlv(wmi_unified_t wmi_handle,
			      void *evt_buf, uint32_t *vdev_id)
{
	wmi_host_swfda_event *swfda_event = (wmi_host_swfda_event *)evt_buf;

	*vdev_id = swfda_event->vdev_id;

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUPPORT_FILS */

/**
 * send_vdev_spectral_enable_cmd_non_tlv() - send VDEV spectral configure
 * command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold spectral enable parameter
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_vdev_spectral_enable_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct vdev_spectral_enable_params *param)
{
	wmi_vdev_spectral_enable_cmd *cmd;
	wmi_buf_t buf;
	int len = 0;
	int ret;

	len = sizeof(wmi_vdev_spectral_enable_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_vdev_spectral_enable_cmd *)wmi_buf_data(buf);

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

#ifdef OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS
	qdf_print
		("%s: Sent WMI_VDEV_SPECTRAL_SCAN_ENABLE_CMDID\n", __func__);

	qdf_print("vdev_id = %u\n"
				 "trigger_cmd = %u\n"
				 "enable_cmd = %u\n",
				 cmd->vdev_id,
				 cmd->trigger_cmd,
				 cmd->enable_cmd);

	qdf_print("%s: Status: %d\n\n", __func__, ret);
#endif /* OL_SPECTRAL_DEBUG_CONFIG_INTERACTIONS */

	ret = wmi_unified_cmd_send(wmi_handle,
				   buf,
				   len,
				   WMI_VDEV_SPECTRAL_SCAN_ENABLE_CMDID);
	return ret;
}

/**
 * send_pdev_set_regdomain_cmd_non_tlv() - send set regdomain command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to pdev regdomain params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_set_regdomain_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct pdev_set_regdomain_params *param)
{
	wmi_pdev_set_regdomain_cmd *cmd;
	wmi_buf_t buf;

	int len = sizeof(wmi_pdev_set_regdomain_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_pdev_set_regdomain_cmd *)wmi_buf_data(buf);

	cmd->reg_domain = param->currentRDinuse;
	cmd->reg_domain_2G = param->currentRD2G;
	cmd->reg_domain_5G = param->currentRD5G;
	cmd->conformance_test_limit_2G = param->ctl_2G;
	cmd->conformance_test_limit_5G = param->ctl_5G;
	cmd->dfs_domain = param->dfsDomain;

	return wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_PDEV_SET_REGDOMAIN_CMDID);
}

/**
 * send_set_quiet_mode_cmd_non_tlv() - send set quiet mode command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to quiet mode params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_quiet_mode_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct set_quiet_mode_params *param)
{
	wmi_buf_t buf;
	wmi_pdev_set_quiet_cmd *quiet_cmd;
	int len = sizeof(wmi_pdev_set_quiet_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	quiet_cmd = (wmi_pdev_set_quiet_cmd *)wmi_buf_data(buf);
	quiet_cmd->enabled = param->enabled;
	quiet_cmd->period = (param->period)*(param->intval);
	quiet_cmd->duration = param->duration;
	quiet_cmd->next_start = param->offset;
	wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_SET_QUIET_MODE_CMDID);
	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_beacon_filter_cmd_non_tlv() - send beacon filter command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to beacon filter params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_beacon_filter_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct set_beacon_filter_params *param)
{
	/* Issue WMI command to set beacon filter */
	int i;
	wmi_add_bcn_filter_cmd_t *cmd;
	QDF_STATUS res;
	wmi_buf_t buf = NULL;
	int len = sizeof(wmi_add_bcn_filter_cmd_t);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("buf alloc failed\n");
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_add_bcn_filter_cmd_t *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	qdf_print("vdev_id: %d\n", cmd->vdev_id);

	for (i = 0; i < BCN_FLT_MAX_ELEMS_IE_LIST; i++)
		cmd->ie_map[i] = 0;

	if (param->ie) {
		for (i = 0; i < BCN_FLT_MAX_ELEMS_IE_LIST; i++)
			cmd->ie_map[i] = param->ie[i];
	}
	res = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_ADD_BCN_FILTER_CMDID);
	return (res == QDF_STATUS_SUCCESS) ?
		QDF_STATUS_SUCCESS : QDF_STATUS_E_FAILURE;
}

/**
 * send_remove_beacon_filter_cmd_non_tlv() - send remove beacon filter command
 * to fw
 * @wmi_handle: wmi handle
 * @param: pointer to remove beacon filter params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_remove_beacon_filter_cmd_non_tlv(wmi_unified_t wmi_handle,
			struct remove_beacon_filter_params *param)
{
	wmi_rmv_bcn_filter_cmd_t *cmd;
	QDF_STATUS res;
	wmi_buf_t buf = NULL;
	int len = sizeof(wmi_rmv_bcn_filter_cmd_t);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("buf alloc failed\n");
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_rmv_bcn_filter_cmd_t *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	res = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_RMV_BCN_FILTER_CMDID);
	return (res == QDF_STATUS_SUCCESS) ?
		QDF_STATUS_SUCCESS : QDF_STATUS_E_FAILURE;
}

/**
 * send_mgmt_cmd_non_tlv() - send mgmt command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to mgmt params
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_mgmt_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct wmi_mgmt_params *param)
{
	wmi_mgmt_tx_cmd *cmd;
	wmi_buf_t wmi_buf;
	int len = sizeof(wmi_mgmt_tx_hdr) + param->frm_len;

	wmi_buf = wmi_buf_alloc(wmi_handle, roundup(len, sizeof(u_int32_t)));
	if (!wmi_buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_mgmt_tx_cmd *)wmi_buf_data(wmi_buf);
	cmd->hdr.vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->macaddr, &cmd->hdr.peer_macaddr);
	cmd->hdr.buf_len = param->frm_len;


#ifdef BIG_ENDIAN_HOST
	{
		/* for big endian host, copy engine byte_swap is enabled
		 * But the mgmt frame buffer content is in network byte order
		 * Need to byte swap the mgmt frame buffer content - so when
		 * copy engine does byte_swap - target gets buffer content in
		 * the correct order
		 */
		int i;
		u_int32_t *destp, *srcp;
		destp = (u_int32_t *)cmd->bufp;
		srcp =  (u_int32_t *)wmi_buf_data(param->tx_frame);
		for (i = 0; i < (roundup(param->frm_len,
				sizeof(u_int32_t))/4); i++) {
			*destp = qdf_le32_to_cpu(*srcp);
			destp++; srcp++;
		}
	}
#else
	qdf_mem_copy(cmd->bufp, wmi_buf_data(param->tx_frame), param->frm_len);
#endif

	/* Send the management frame buffer to the target */
	wmi_unified_cmd_send(wmi_handle, wmi_buf, roundup(len,
		    sizeof(u_int32_t)), WMI_MGMT_TX_CMDID);
	return QDF_STATUS_SUCCESS;
}

/**
 * send_addba_clearresponse_cmd_non_tlv() - send addba clear response command
 * to fw
 * @wmi_handle: wmi handle
 * @param: pointer to addba clearresp params
 * @macaddr: vdev mac address
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_addba_clearresponse_cmd_non_tlv(wmi_unified_t wmi_handle,
			uint8_t macaddr[IEEE80211_ADDR_LEN],
			struct addba_clearresponse_params *param)
{
	wmi_addba_clear_resp_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_addba_clear_resp_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_addba_clear_resp_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);

	/* Send the management frame buffer to the target */
	wmi_unified_cmd_send(wmi_handle, buf, len, WMI_ADDBA_CLEAR_RESP_CMDID);
	return QDF_STATUS_SUCCESS;
}

/**
 * send_addba_send_cmd_non_tlv() - send addba send command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to addba send params
 * @macaddr: vdev mac address
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_addba_send_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct addba_send_params *param)
{
	wmi_addba_send_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_addba_send_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_addba_send_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->tid = param->tidno;
	cmd->buffersize = param->buffersize;

	/* Send the management frame buffer to the target */
	wmi_unified_cmd_send(wmi_handle, buf, len, WMI_ADDBA_SEND_CMDID);
	return QDF_STATUS_SUCCESS;
}

/**
 * send_delba_send_cmd_non_tlv() - send delba send command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to delba send params
 * @macaddr: vdev mac address
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_delba_send_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct delba_send_params *param)
{
	wmi_delba_send_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_delba_send_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_delba_send_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->tid = param->tidno;
	cmd->initiator = param->initiator;
	cmd->reasoncode = param->reasoncode;

	/* send the management frame buffer to the target */
	wmi_unified_cmd_send(wmi_handle, buf, len, WMI_DELBA_SEND_CMDID);
	return QDF_STATUS_SUCCESS;
}

/**
 * send_addba_setresponse_cmd_non_tlv() - send addba set response command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to addba setresp params
 * @macaddr: vdev mac address
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_addba_setresponse_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct addba_setresponse_params *param)
{
	wmi_addba_setresponse_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_addba_setresponse_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_addba_setresponse_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->tid = param->tidno;
	cmd->statuscode = param->statuscode;

	/* send the management frame buffer to the target */
	wmi_unified_cmd_send(wmi_handle, buf, len, WMI_ADDBA_SET_RESP_CMDID);
	return QDF_STATUS_SUCCESS;
}

/**
 * send_singleamsdu_cmd_non_tlv() - send single amsdu command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to single amsdu params
 * @macaddr: vdev mac address
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_singleamsdu_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct singleamsdu_params *param)
{
	wmi_send_singleamsdu_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_send_singleamsdu_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_send_singleamsdu_cmd *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->tid = param->tidno;

	/* send the management frame buffer to the target */
	wmi_unified_cmd_send(wmi_handle, buf, len, WMI_SEND_SINGLEAMSDU_CMDID);
	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_qboost_param_cmd_non_tlv() - send set qboost command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to qboost params
 * @macaddr: vdev mac address
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_qboost_param_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct set_qboost_params *param)
{

	WMI_QBOOST_CFG_CMD *cmd;
	wmi_buf_t buf;
	int ret;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (WMI_QBOOST_CFG_CMD *)wmi_buf_data(buf);
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->qb_enable = param->value;

	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
			WMI_QBOOST_CFG_CMDID);
	return ret;
}

/**
 * send_mu_scan_cmd_non_tlv() - send mu scan command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to mu scan params
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_mu_scan_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct mu_scan_params *param)
{
	wmi_mu_start_cmd *cmd;
	wmi_buf_t buf;

	buf = wmi_buf_alloc(wmi_handle, sizeof(wmi_mu_start_cmd));
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_mu_start_cmd *)wmi_buf_data(buf);
	cmd->mu_request_id = param->id;
	cmd->mu_duration = param->duration;
	cmd->mu_type = param->type;
	cmd->lteu_tx_power = param->lteu_tx_power;
	cmd->rssi_thr_bssid = param->rssi_thr_bssid;
	cmd->rssi_thr_sta = param->rssi_thr_sta;
	cmd->rssi_thr_sc = param->rssi_thr_sc;
	cmd->plmn_id = param->plmn_id;
	cmd->alpha_num_bssid = param->alpha_num_bssid;
	return wmi_unified_cmd_send(wmi_handle, buf,
				sizeof(wmi_mu_start_cmd),
				WMI_MU_CAL_START_CMDID);
}

/**
 * send_lteu_config_cmd_non_tlv() - send lteu config command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to lteu config params
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_lteu_config_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct lteu_config_params *param)
{
	wmi_set_lteu_config *cmd;
	wmi_buf_t buf;
	int i;

	buf = wmi_buf_alloc(wmi_handle, sizeof(wmi_set_lteu_config));
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_set_lteu_config *)wmi_buf_data(buf);
	cmd->gpio_enable = param->lteu_gpio_start;
	cmd->num_lteu_bins = param->lteu_num_bins;
	for (i = 0; i < cmd->num_lteu_bins; i++) {
		cmd->mu_rssi_threshold[i] = param->lteu_thresh[i];
		cmd->mu_weight[i] = param->lteu_weight[i];
		cmd->mu_gamma[i] = param->lteu_gamma[i];
	}
	cmd->mu_scan_timeout = param->lteu_scan_timeout;
	cmd->alpha_num_bssid = param->alpha_num_bssid;
	cmd->use_actual_nf = param->use_actual_nf;
	cmd->wifi_tx_power = param->wifi_tx_power;
	cmd->allow_err_packets = param->allow_err_packets;
	return wmi_unified_cmd_send(wmi_handle, buf,
				sizeof(wmi_set_lteu_config),
				WMI_SET_LTEU_CONFIG_CMDID);
}

/**
 * send_pdev_get_tpc_config_cmd_non_tlv() - send get tpc config command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to get tpc config params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_get_tpc_config_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint32_t param)
{
	wmi_pdev_get_tpc_config_cmd *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(wmi_pdev_get_tpc_config_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_pdev_get_tpc_config_cmd *)wmi_buf_data(buf);
	cmd->param = param;
	return wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_GET_TPC_CONFIG_CMDID);
}

/**
 * send_set_bwf_cmd_non_tlv() - send set bwf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set bwf param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_bwf_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct set_bwf_params *param)
{
	struct wmi_bwf_peer_info   *peer_info;
	wmi_peer_bwf_request *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_peer_bwf_request);
	int i, retval = 0;

	len += param->num_peers * sizeof(struct wmi_bwf_peer_info);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_peer_bwf_request *)wmi_buf_data(buf);
	qdf_mem_copy(&(cmd->num_peers), &(param->num_peers), sizeof(uint32_t));
	peer_info = (struct wmi_bwf_peer_info *)&(cmd->peer_info[0]);
	for (i = 0; i < param->num_peers; i++) {
		qdf_mem_copy(&(peer_info[i].peer_macaddr),
			&(param->peer_info[i].peer_macaddr),
			sizeof(wmi_mac_addr));
		peer_info[i].bwf_guaranteed_bandwidth =
			param->peer_info[i].throughput;
		peer_info[i].bwf_max_airtime = param->peer_info[i].max_airtime;
		peer_info[i].bwf_peer_priority = param->peer_info[i].priority;
	}

	retval = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PEER_BWF_REQUEST_CMDID);

	if (retval)
		wmi_buf_free(buf);

	return retval;
}

/**
 * send_set_atf_cmd_non_tlv() - send set atf command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set atf param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_atf_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct set_atf_params *param)
{
	struct wmi_atf_peer_info   *peer_info;
	wmi_peer_atf_request *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_peer_atf_request);
	int i, retval = 0;

	len += param->num_peers * sizeof(struct wmi_atf_peer_info);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_peer_atf_request *)wmi_buf_data(buf);
	qdf_mem_copy(&(cmd->num_peers), &(param->num_peers), sizeof(uint32_t));
	peer_info = (struct wmi_atf_peer_info *)&(cmd->peer_info[0]);
	for (i = 0; i < param->num_peers; i++) {
		qdf_mem_copy(&(peer_info[i].peer_macaddr),
			&(param->peer_info[i].peer_macaddr),
			sizeof(wmi_mac_addr));
		peer_info[i].atf_units = param->peer_info[i].percentage_peer;
	}
/*	qdf_print("wmi_unified_pdev_set_atf peer_num=%d\n", cmd->num_peers); */
	retval = wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PEER_ATF_REQUEST_CMDID);
	return retval;
}

/**
 * send_atf_peer_request_cmd_non_tlv() - send atf peer request command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to atf peer request param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_atf_peer_request_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct atf_peer_request_params *param)
{
	struct wmi_atf_peer_ext_info *peer_ext_info;
	wmi_peer_atf_ext_request *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_peer_atf_ext_request);
	int i, retval = 0;

	len += param->num_peers * sizeof(struct wmi_atf_peer_ext_info);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_peer_atf_ext_request *)wmi_buf_data(buf);
	qdf_mem_copy(&(cmd->num_peers), &(param->num_peers), sizeof(uint32_t));
	peer_ext_info =
	    (struct wmi_atf_peer_ext_info *)&(cmd->peer_ext_info[0]);
	for (i = 0; i < param->num_peers; i++) {
		qdf_mem_copy(&(peer_ext_info[i].peer_macaddr),
			&(param->peer_ext_info[i].peer_macaddr),
			sizeof(wmi_mac_addr));
		peer_ext_info[i].atf_groupid =
			param->peer_ext_info[i].group_index;
		peer_ext_info[i].atf_units_reserved =
			param->peer_ext_info[i].atf_index_reserved;
	}
	retval = wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PEER_ATF_EXT_REQUEST_CMDID);

	return retval;
}

/**
 * send_set_atf_grouping_cmd_non_tlv() - send set atf grouping command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set atf grouping param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_atf_grouping_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct atf_grouping_params *param)
{
	struct wmi_atf_group_info *group_info;
	wmi_atf_ssid_grp_request *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_atf_ssid_grp_request);
	int i, retval = 0;

	len += param->num_groups * sizeof(struct wmi_atf_group_info);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_atf_ssid_grp_request *)wmi_buf_data(buf);
	qdf_mem_copy(&(cmd->num_groups), &(param->num_groups),
		sizeof(uint32_t));
	group_info = (struct wmi_atf_group_info *)&(cmd->group_info[0]);
	for (i = 0; i < param->num_groups; i++)	{
		group_info[i].atf_group_units =
			param->group_info[i].percentage_group;
		group_info[i].atf_group_units_reserved =
			param->group_info[i].atf_group_units_reserved;
	}
	retval = wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_ATF_SSID_GROUPING_REQUEST_CMDID);

	return retval;
}

/**
 * send_wlan_profile_enable_cmd_non_tlv() - send wlan profile enable command
 * to fw
 * @wmi_handle: wmi handle
 * @param: pointer to wlan profile param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_wlan_profile_enable_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct wlan_profile_params *param)
{
	wmi_buf_t buf;
	uint16_t len;
	wmi_wlan_profile_enable_profile_id_cmd *cmd;

	len = sizeof(wmi_wlan_profile_enable_profile_id_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_wlan_profile_enable_profile_id_cmd *)wmi_buf_data(buf);
	cmd->profile_id = param->profile_id;
	cmd->enable = param->enable;
	return wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID);
}

/**
 * send_wlan_profile_trigger_cmd_non_tlv() - send wlan profile trigger command
 * to fw
 * @wmi_handle: wmi handle
 * @param: pointer to wlan profile param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_wlan_profile_trigger_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct wlan_profile_params *param)
{
	wmi_buf_t buf;
	uint16_t len;
	wmi_wlan_profile_trigger_cmd *cmd;

	len = sizeof(wmi_wlan_profile_trigger_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_wlan_profile_trigger_cmd *)wmi_buf_data(buf);
	cmd->enable = param->enable;
	return wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_WLAN_PROFILE_TRIGGER_CMDID);
}

#ifdef BIG_ENDIAN_HOST
void wmi_host_swap_bytes(void *pv, size_t n)
{
	int noWords;
	int i;
	A_UINT32 *wordPtr;

	noWords =   n/sizeof(u_int32_t);
	wordPtr = (u_int32_t *)pv;
	for (i = 0; i < noWords; i++)
		*(wordPtr + i) = __cpu_to_le32(*(wordPtr + i));
}
#define WMI_HOST_SWAPME(x, len) wmi_host_swap_bytes(&x, len);
#endif

/**
 * send_set_ht_ie_cmd_non_tlv() - send ht ie command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to ht ie param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_ht_ie_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct ht_ie_params *param)
{
	wmi_pdev_set_ht_ie_cmd *cmd;
	wmi_buf_t buf;
	/* adjust length to be next multiple of four */
	int len = (param->ie_len + (sizeof(uint32_t) - 1)) &
	    (~(sizeof(uint32_t) - 1));

	/* to account for extra four bytes of ie data in the struct */
	len += (sizeof(wmi_pdev_set_ht_ie_cmd) - 4);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_pdev_set_ht_ie_cmd  *)wmi_buf_data(buf);
	cmd->ie_len = param->ie_len;
	qdf_mem_copy(cmd->ie_data, param->ie_data, param->ie_len);
#ifdef BIG_ENDIAN_HOST
	WMI_HOST_SWAPME(cmd->ie_data, len-(offsetof(wmi_pdev_set_ht_ie_cmd,
			ie_data)));
#endif
	return wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PDEV_SET_HT_CAP_IE_CMDID);
}

/**
 * send_set_vht_ie_cmd_non_tlv() - send vht ie command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to vht ie param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_vht_ie_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct vht_ie_params *param)
{
	wmi_pdev_set_vht_ie_cmd *cmd;
	wmi_buf_t buf;
	/* adjust length to be next multiple of four */
	int len = (param->ie_len + (sizeof(u_int32_t) - 1)) &
	    (~(sizeof(u_int32_t) - 1));

	/* to account for extra four bytes of ie data in the struct */
	len += (sizeof(wmi_pdev_set_vht_ie_cmd) - 4);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_pdev_set_vht_ie_cmd *)wmi_buf_data(buf);
	cmd->ie_len = param->ie_len;
	qdf_mem_copy(cmd->ie_data, param->ie_data, param->ie_len);
#ifdef BIG_ENDIAN_HOST
	WMI_HOST_SWAPME(cmd->ie_data, len-(offsetof(wmi_pdev_set_vht_ie_cmd,
			ie_data)));
#endif
	return wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PDEV_SET_VHT_CAP_IE_CMDID);
}

/**
 * send_wmm_update_cmd_non_tlv() - send wmm update command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to wmm update param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_wmm_update_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct wmm_update_params *param)
{
	wmi_buf_t buf;
	wmi_pdev_set_wmm_params_cmd *cmd;
	wmi_wmm_params *wmi_param = 0;
	int ac;
	int len = sizeof(wmi_pdev_set_wmm_params_cmd);
	struct wmi_host_wmeParams *wmep;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_SUCCESS;
	}
	cmd = (wmi_pdev_set_wmm_params_cmd *)wmi_buf_data(buf);

	for (ac = 0; ac < WME_NUM_AC; ac++) {
		wmep = &param->wmep_array[ac];
		switch (ac) {
		case WMI_HOST_AC_BE:
			wmi_param = &cmd->wmm_params_ac_be;
			break;
		case WMI_HOST_AC_BK:
			wmi_param = &cmd->wmm_params_ac_bk;
			break;
		case WMI_HOST_AC_VI:
			wmi_param = &cmd->wmm_params_ac_vi;
			break;
		case WMI_HOST_AC_VO:
			wmi_param = &cmd->wmm_params_ac_vo;
			break;
		default:
			break;
		}

		wmi_param->aifs = wmep->wmep_aifsn;
		wmi_param->cwmin = ATH_EXPONENT_TO_VALUE(wmep->wmep_logcwmin);
		wmi_param->cwmax = ATH_EXPONENT_TO_VALUE(wmep->wmep_logcwmax);
		wmi_param->txoplimit = ATH_TXOP_TO_US(wmep->wmep_txopLimit);
		wmi_param->acm = wmep->wmep_acm;
		wmi_param->no_ack = wmep->wmep_noackPolicy;

	}

	wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PDEV_SET_WMM_PARAMS_CMDID);
	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_ant_switch_tbl_cmd_non_tlv() - send ant switch tbl cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold ant switch tbl param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_ant_switch_tbl_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct ant_switch_tbl_params *param)
{
	uint8_t len;
	wmi_buf_t buf;
	wmi_pdev_set_ant_switch_tbl_cmd *cmd;

	len = sizeof(wmi_pdev_set_ant_switch_tbl_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_pdev_set_ant_switch_tbl_cmd *)wmi_buf_data(buf);
	cmd->antCtrlCommon1 = param->ant_ctrl_common1;
	cmd->antCtrlCommon2 = param->ant_ctrl_common2;

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PDEV_SET_ANTENNA_SWITCH_TABLE_CMDID)) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_ratepwr_table_cmd_non_tlv() - send rate power table cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold rate power table param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_ratepwr_table_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct ratepwr_table_params *param)
{
	uint16_t len;
	wmi_buf_t buf;
	wmi_pdev_ratepwr_table_cmd *cmd;

	if (!param->ratepwr_tbl)
		return QDF_STATUS_E_FAILURE;

	len = sizeof(wmi_pdev_ratepwr_table_cmd);
	len += roundup(param->ratepwr_len, sizeof(A_UINT32)) - sizeof(A_UINT32);
	/* already 4 bytes in cmd structure */
	qdf_print("wmi buf len = %d\n", len);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_pdev_ratepwr_table_cmd *)wmi_buf_data(buf);

	cmd->op = RATEPWR_TABLE_OPS_SET;
	cmd->ratepwr_len = param->ratepwr_len;
	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(&cmd->ratepwr_tbl[0],
		param->ratepwr_tbl, param->ratepwr_len);

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PDEV_RATEPWR_TABLE_CMDID)) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_get_ratepwr_table_cmd_non_tlv() - send rate power table cmd to fw
 * @wmi_handle: wmi handle
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_get_ratepwr_table_cmd_non_tlv(wmi_unified_t wmi_handle)
{
	uint16_t len;
	wmi_buf_t buf;
	wmi_pdev_ratepwr_table_cmd *cmd;

	len = sizeof(wmi_pdev_ratepwr_table_cmd);
	qdf_print("wmi buf len = %d\n", len);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_pdev_ratepwr_table_cmd *)wmi_buf_data(buf);

	cmd->op = RATEPWR_TABLE_OPS_GET;
	cmd->ratepwr_len = 0;
	cmd->ratepwr_tbl[0] = 0;

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PDEV_RATEPWR_TABLE_CMDID)) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_ctl_table_cmd_non_tlv() - send ctl table cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold ctl table param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_ctl_table_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct ctl_table_params *param)
{
	uint16_t len;
	wmi_buf_t buf;
	wmi_pdev_set_ctl_table_cmd *cmd;

	if (!param->ctl_array)
		return QDF_STATUS_E_FAILURE;

	/* CTL array length check for Beeliner family */
	if (param->target_type == TARGET_TYPE_AR900B ||
			param->target_type == TARGET_TYPE_QCA9984 ||
			param->target_type == TARGET_TYPE_IPQ4019 ||
			param->target_type == TARGET_TYPE_QCA9888) {
		if (param->is_2g) {
			/* For 2G, CTL array length should be 688*/
			if (param->ctl_cmd_len !=
				(4 + (WMI_HOST_NUM_CTLS_2G_11B * 2) +
				 (WMI_HOST_NUM_BAND_EDGES_2G_11B * 3) +
				 1 + (WMI_HOST_NUM_CTLS_2G_11B *
					 WMI_HOST_NUM_BAND_EDGES_2G_11B) +
				 (WMI_HOST_NUM_CTLS_2G_20MHZ * 2) +
				 (WMI_HOST_NUM_BAND_EDGES_2G_20MHZ * 3) +
				 1 + (WMI_HOST_NUM_CTLS_2G_20MHZ *
					 WMI_HOST_NUM_BAND_EDGES_2G_20MHZ) +
				 (WMI_HOST_NUM_CTLS_2G_40MHZ * 2) +
				 (WMI_HOST_NUM_BAND_EDGES_2G_40MHZ * 3) +
				 (WMI_HOST_NUM_CTLS_2G_40MHZ *
				  WMI_HOST_NUM_BAND_EDGES_2G_40MHZ) + 4)) {
				qdf_print("CTL array len not correct\n");
				return QDF_STATUS_E_FAILURE;
			}
		} else {
			/* For 5G, CTL array length should be 1540 */
			if (param->ctl_cmd_len !=
				(4 + (WMI_HOST_NUM_CTLS_5G_11A * 2) +
				(WMI_HOST_NUM_BAND_EDGES_5G_11A * 3) +
				 1 + (WMI_HOST_NUM_CTLS_5G_11A *
					 WMI_HOST_NUM_BAND_EDGES_5G_11A) + 1
				 + (WMI_HOST_NUM_CTLS_5G_HT20 * 2) +
				 (WMI_HOST_NUM_BAND_EDGES_5G_HT20 * 3) +
				 1 + (WMI_HOST_NUM_CTLS_5G_HT20 *
					 WMI_HOST_NUM_BAND_EDGES_5G_HT20) +
				 (WMI_HOST_NUM_CTLS_5G_HT40 * 2) +
				 (WMI_HOST_NUM_BAND_EDGES_5G_HT40 * 3) +
				 (WMI_HOST_NUM_CTLS_5G_HT40 *
				  WMI_HOST_NUM_BAND_EDGES_5G_HT40) +
				 (WMI_HOST_NUM_CTLS_5G_HT80 * 2) +
				 (WMI_HOST_NUM_BAND_EDGES_5G_HT80 * 3) +
				 (WMI_HOST_NUM_CTLS_5G_HT80 *
				  WMI_HOST_NUM_BAND_EDGES_5G_HT80) +
				 (WMI_HOST_NUM_CTLS_5G_HT160 * 2) +
				 (WMI_HOST_NUM_BAND_EDGES_5G_HT160 * 3) +
				 (WMI_HOST_NUM_CTLS_5G_HT160 *
				  WMI_HOST_NUM_BAND_EDGES_5G_HT160))) {
				qdf_print("CTL array len not correct\n");
				return QDF_STATUS_E_FAILURE;
			}
		}
	} else {
		if (param->ctl_cmd_len !=
			WMI_HOST_NUM_CTLS_2G * WMI_HOST_NUM_BAND_EDGES_2G * 2 +
			WMI_HOST_NUM_CTLS_5G * WMI_HOST_NUM_BAND_EDGES_5G * 2) {
			qdf_print("CTL array len not correct\n");
			return QDF_STATUS_E_FAILURE;
		}
	}

	len = sizeof(wmi_pdev_set_ctl_table_cmd);
	len += roundup(param->ctl_cmd_len, sizeof(A_UINT32)) - sizeof(A_UINT32);
	qdf_print("wmi buf len = %d\n", len);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_pdev_set_ctl_table_cmd *)wmi_buf_data(buf);

	cmd->ctl_len = param->ctl_cmd_len;
	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(&cmd->ctl_info[0], &param->ctl_band,
		sizeof(param->ctl_band));
	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(&cmd->ctl_info[1], param->ctl_array,
		param->ctl_cmd_len - sizeof(param->ctl_band));

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_SET_CTL_TABLE_CMDID)) {
		qdf_print("%s:Failed to send command\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_mimogain_table_cmd_non_tlv() - send mimogain table cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold mimogain table param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_mimogain_table_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct mimogain_table_params *param)
{
	uint16_t len;
	wmi_buf_t buf;
	wmi_pdev_set_mimogain_table_cmd *cmd;

	if (!param->array_gain)
		return QDF_STATUS_E_FAILURE;

	/* len must be multiple of a single array gain table */
	if (param->tbl_len %
	    ((WMI_HOST_TX_NUM_CHAIN-1) * WMI_HOST_TPC_REGINDEX_MAX *
	     WMI_HOST_ARRAY_GAIN_NUM_STREAMS) != 0) {
		qdf_print("Array gain table len not correct\n");
		return QDF_STATUS_E_FAILURE;
	}

	len = sizeof(wmi_pdev_set_mimogain_table_cmd);
	len += roundup(param->tbl_len, sizeof(A_UINT32)) - sizeof(A_UINT32);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_pdev_set_mimogain_table_cmd *)wmi_buf_data(buf);

	WMI_MIMOGAIN_ARRAY_GAIN_LEN_SET(cmd->mimogain_info, param->tbl_len);
	WMI_MIMOGAIN_MULTI_CHAIN_BYPASS_SET(cmd->mimogain_info,
		param->multichain_gain_bypass);
	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(&cmd->arraygain_tbl[0],
		param->array_gain,
		param->tbl_len);

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PDEV_SET_MIMOGAIN_TABLE_CMDID)) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_ratepwr_chainmsk_cmd_non_tlv() - send ratepwr chainmask cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold ratepwr chainmask param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_ratepwr_chainmsk_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct ratepwr_chainmsk_params *param)
{
#define RC_CCK_OFDM_RATES		0
#define RC_HT_RATES			1
#define RC_VHT_RATES			2
	uint16_t len;
	wmi_buf_t buf;
	wmi_pdev_ratepwr_chainmsk_tbl_cmd *cmd;

	if (!param->ratepwr_chain_tbl)
		return QDF_STATUS_E_FAILURE;

	len = sizeof(wmi_pdev_ratepwr_chainmsk_tbl_cmd);
	len += roundup(param->num_rate*sizeof(uint32_t), sizeof(A_UINT32));
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_pdev_ratepwr_chainmsk_tbl_cmd *)wmi_buf_data(buf);
	cmd->op = param->ops;
	cmd->pream_type = param->pream_type;
	cmd->rate_len = param->num_rate;

	if (param->ops == RATEPWR_CHAINMSK_TABLE_OPS_EN) {
		qdf_mem_copy(&cmd->ratepwr_chaintbl[0],
				param->ratepwr_chain_tbl,
				param->num_rate*sizeof(u_int32_t));
	}

	wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_PDEV_RATEPWR_CHAINMSK_TABLE_CMDID);
	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_macaddr_cmd_non_tlv() - send set macaddr cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold macaddr param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_macaddr_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct macaddr_params *param)
{
	uint8_t len;
	wmi_buf_t buf;
	wmi_pdev_set_base_macaddr_cmd *cmd;

	len = sizeof(wmi_pdev_set_base_macaddr_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_pdev_set_base_macaddr_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->macaddr, &cmd->base_macaddr);

	if (wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PDEV_SET_BASE_MACADDR_CMDID)) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_pdev_scan_start_cmd_non_tlv() - send pdev scan start cmd to fw
 * @wmi_handle: wmi handle
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_scan_start_cmd_non_tlv(wmi_unified_t wmi_handle)
{
	/*
	 * this command was added to support host scan egine which is
	 * deprecated. now  the scan engine is in FW and host directly
	 * isssues a scan request to perform scan and provide results back
	 * to host
	 */
	wmi_buf_t buf;
	wmi_pdev_scan_cmd *cmd;
	int len = sizeof(wmi_pdev_scan_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	qdf_print("%s:\n", __func__);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_pdev_scan_cmd *)wmi_buf_data(buf);
	cmd->scan_start = TRUE;
#if DEPRECATE_WMI
	wmi_unified_cmd_send(wmi_handle, buf, len, WMI_PDEV_SCAN_CMDID);
#endif
	return QDF_STATUS_SUCCESS;
}

/**
 * send_pdev_scan_end_cmd_non_tlv() - send pdev scan end cmd to fw
 * @wmi_handle: wmi handle
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_scan_end_cmd_non_tlv(wmi_unified_t wmi_handle)
{
	/*
	 * this command was added to support host scan egine which is
	 * deprecated. now  the scan engine is in FW and host directly isssues
	 * a scan request to perform scan and provide results back to host
	 */
	wmi_pdev_scan_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_pdev_scan_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	qdf_print("%s:\n", __func__);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_pdev_scan_cmd *)wmi_buf_data(buf);
	cmd->scan_start = FALSE;
#if DEPRECATE_WMI
	wmi_unified_cmd_send(wmi_handle, buf, len, WMI_PDEV_SCAN_CMDID);
#endif
	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_acparams_cmd_non_tlv() - send acparams cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold acparams
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_acparams_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct acparams_params *param)
{
	wmi_pdev_set_param_cmd *cmd;
	wmi_buf_t buf;
	uint32_t param_value = 0;
	int len = sizeof(wmi_pdev_set_param_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_pdev_set_param_cmd *)wmi_buf_data(buf);
	cmd->param_id = WMI_PDEV_PARAM_AC_AGGRSIZE_SCALING;
	param_value = param->ac;
	param_value |= (param->aggrsize_scaling << 8);
	cmd->param_value = param_value;

	wmi_unified_cmd_send(wmi_handle, buf, len, WMI_PDEV_SET_PARAM_CMDID);
	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_vap_dscp_tid_map_cmd_non_tlv() - send vap dscp tid map cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold vap dscp tid map param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_set_vap_dscp_tid_map_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct vap_dscp_tid_map_params *param)
{
	wmi_buf_t buf;
	wmi_vdev_set_dscp_tid_map_cmd *cmd_vdev;
	int len_vdev = sizeof(wmi_vdev_set_dscp_tid_map_cmd);

	buf = wmi_buf_alloc(wmi_handle, len_vdev);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd_vdev = (wmi_vdev_set_dscp_tid_map_cmd *)wmi_buf_data(buf);
	qdf_mem_copy(cmd_vdev->dscp_to_tid_map, param->dscp_to_tid_map,
		sizeof(A_UINT32) * WMI_DSCP_MAP_MAX);

	cmd_vdev->vdev_id = param->vdev_id;

	qdf_print("Setting dscp for vap id: %d\n", cmd_vdev->vdev_id);
	return wmi_unified_cmd_send(wmi_handle, buf, len_vdev,
		WMI_VDEV_SET_DSCP_TID_MAP_CMDID);
}

/**
 * send_proxy_ast_reserve_cmd_non_tlv() - send proxy ast reserve cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold proxy ast reserve param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_proxy_ast_reserve_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct proxy_ast_reserve_params *param)
{
	wmi_pdev_reserve_ast_entry_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_pdev_reserve_ast_entry_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_pdev_reserve_ast_entry_cmd *)wmi_buf_data(buf);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->macaddr, &cmd->mac_addr);
	cmd->key_id = 0;
	cmd->mcast = 0;

	qdf_print("%s macaddr=%s key_id=%d mcast=%d\n", __func__,
		ether_sprintf(param->macaddr), cmd->key_id, cmd->mcast);

	return wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_PDEV_RESERVE_AST_ENTRY_CMDID);
}

/**
 * send_pdev_fips_cmd_non_tlv() - send pdev fips cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold pdev fips param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_fips_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct fips_params *param)
{
	wmi_pdev_fips_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_pdev_fips_cmd) + param->data_len;
	int retval = 0;

	/* Data length must be multiples of 16 bytes - checked against 0xF -
	 *  and must be less than WMI_SVC_MSG_SIZE - static size of
	 *  wmi_pdev_fips_cmd structure
	 */
	/* do sanity on the input */
	if (!(((param->data_len & 0xF) == 0) &&
			((param->data_len > 0) &&
			 (param->data_len < (WMI_HOST_MAX_BUFFER_SIZE -
			 sizeof(wmi_pdev_fips_cmd)))))) {
		return QDF_STATUS_E_INVAL;
	}

	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_pdev_fips_cmd *)wmi_buf_data(buf);
	if (param->key != NULL && param->data != NULL) {
		cmd->key_len = param->key_len;
		cmd->data_len = param->data_len;
		cmd->fips_cmd = !!(param->op);

#ifdef BIG_ENDIAN_HOST
		{
			/****************BE to LE conversion*****************/
			/* Assigning unaligned space to copy the key */
			unsigned char *key_unaligned = qdf_mem_malloc(
				sizeof(u_int8_t)*param->key_len + FIPS_ALIGN);

			u_int8_t *key_aligned = NULL;

			unsigned char *data_unaligned = qdf_mem_malloc(
				sizeof(u_int8_t)*param->data_len + FIPS_ALIGN);
			u_int8_t *data_aligned = NULL;

			int c;

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

			/*****************************************************/
		}
#endif
		qdf_mem_copy(cmd->key, param->key, param->key_len);
		qdf_mem_copy(cmd->data, param->data, param->data_len);

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
		print_hex_dump(KERN_DEBUG, "Plain text: ", DUMP_PREFIX_NONE,
			16, 1, cmd->data, cmd->data_len, true);

		retval = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_FIPS_CMDID);
		qdf_print("%s return value %d\n", __func__, retval);
	} else {
		qdf_print("\n%s:%d Key or Data is NULL\n", __func__, __LINE__);
		retval = -EFAULT;
	}

	return retval;
}

/**
 * send_pdev_set_chan_cmd_non_tlv() - send pdev set chan cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold set chan param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_set_chan_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct channel_param *param)
{
	wmi_set_channel_cmd *cmd;
	wmi_buf_t buf;
	int len = sizeof(wmi_set_channel_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_set_channel_cmd *)wmi_buf_data(buf);

	cmd->chan.mhz = param->mhz;

	WMI_SET_CHANNEL_MODE(&cmd->chan, param->phy_mode);

	cmd->chan.band_center_freq1 = param->cfreq1;
	cmd->chan.band_center_freq2 = param->cfreq2;

	WMI_SET_CHANNEL_MIN_POWER(&cmd->chan, param->minpower);
	WMI_SET_CHANNEL_MAX_POWER(&cmd->chan, param->maxpower);
	WMI_SET_CHANNEL_REG_POWER(&cmd->chan, param->maxregpower);
	WMI_SET_CHANNEL_ANTENNA_MAX(&cmd->chan, param->antennamax);
	WMI_SET_CHANNEL_REG_CLASSID(&cmd->chan, param->reg_class_id);

	if (param->dfs_set)
		WMI_SET_CHANNEL_FLAG(&cmd->chan, WMI_CHAN_FLAG_DFS);

	if (param->dfs_set_cfreq2)
		WMI_SET_CHANNEL_FLAG(&cmd->chan, WMI_CHAN_FLAG_DFS_CFREQ2);

	if (param->half_rate)
		WMI_SET_CHANNEL_FLAG(&cmd->chan, WMI_CHAN_FLAG_HALF);

	if (param->quarter_rate)
		WMI_SET_CHANNEL_FLAG(&cmd->chan, WMI_CHAN_FLAG_QUARTER);

	if ((param->phy_mode == MODE_11AC_VHT80_80) ||
			(param->phy_mode == MODE_11AC_VHT160)) {
		qdf_print(
		"WMI channel freq=%d, mode=%x band_center_freq1=%d band_center_freq2=%d\n",
		cmd->chan.mhz,
		WMI_GET_CHANNEL_MODE(&cmd->chan), cmd->chan.band_center_freq1,
		cmd->chan.band_center_freq2);
	} else {
		qdf_print("WMI channel freq=%d, mode=%x band_center_freq1=%d\n"
			, cmd->chan.mhz,
			WMI_GET_CHANNEL_MODE(&cmd->chan),
			cmd->chan.band_center_freq1);
	}

	return wmi_unified_cmd_send(wmi_handle, buf, len,
					WMI_PDEV_SET_CHANNEL_CMDID);
}

/**
 * send_mcast_group_update_cmd_non_tlv() - send mcast group update cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold mcast update param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_mcast_group_update_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct mcast_group_update_params *param)
{
	wmi_peer_mcast_group_cmd *cmd;
	wmi_buf_t buf;
	int len;
	int offset = 0;
	static char dummymask[4] = { 0xFF, 0xFF, 0xFF, 0xFF};

	len = sizeof(wmi_peer_mcast_group_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_peer_mcast_group_cmd *) wmi_buf_data(buf);
	/* confirm the buffer is 4-byte aligned */
	ASSERT((((size_t) cmd) & 0x3) == 0);
	OS_MEMZERO(cmd, sizeof(wmi_peer_mcast_group_cmd));

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
		qdf_mem_copy(
				&cmd->ucast_mac_addr,
				param->ucast_mac_addr,
				sizeof(cmd->ucast_mac_addr));
	}
	if (param->mcast_ip_addr) {
		ASSERT(param->mcast_ip_addr_bytes <=
			sizeof(cmd->mcast_ip_addr));
		offset = sizeof(cmd->mcast_ip_addr) -
			param->mcast_ip_addr_bytes;
		qdf_mem_copy(((u_int8_t *) &cmd->mcast_ip_addr) + offset,
				param->mcast_ip_addr,
				param->mcast_ip_addr_bytes);
	}
	if (!param->mask)
		param->mask = &dummymask[0];

	qdf_mem_copy(((u_int8_t *) &cmd->mcast_ip_mask) + offset, param->mask,
		param->mcast_ip_addr_bytes);

	if (param->srcs && param->nsrcs) {
		cmd->num_filter_addr = param->nsrcs;
		ASSERT((param->nsrcs * param->mcast_ip_addr_bytes) <=
			sizeof(cmd->srcs));

		qdf_mem_copy(((u_int8_t *) &cmd->filter_addr), param->srcs,
			param->nsrcs * param->mcast_ip_addr_bytes);
	}
	/* now correct for endianness, if necessary */
	/*
	 * For Little Endian, N/w Stack gives packets in Network byte order and
	 * issue occurs if both Host and Target happens to be in Little Endian.
	 * Target when compares IP addresses in packet with MCAST_GROUP_CMDID
	 * given IP addresses, it fails. Hence swap only mcast_ip_addr
	 * (16 bytes) for now.
	 * TODO : filter
	 */
/* TBD in OL Layer
#ifdef BIG_ENDIAN_HOST
	ol_bytestream_endian_fix(
		(u_int32_t *)&cmd->ucast_mac_addr,
		(sizeof(*cmd)-4) / sizeof(u_int32_t));
#else
	ol_bytestream_endian_fix(
		(u_int32_t *)&cmd->mcast_ip_addr,
		(sizeof(cmd->mcast_ip_addr)) / sizeof(u_int32_t));
#endif  Little Endian */
	wmi_unified_cmd_send(
			wmi_handle, buf, len, WMI_PEER_MCAST_GROUP_CMDID);

	return QDF_STATUS_SUCCESS;
}

/**
 * send_periodic_chan_stats_config_cmd_non_tlv() - send periodic chan stats cmd
 * to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold periodic chan stats param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_periodic_chan_stats_config_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct periodic_chan_stats_params *param)
{
	wmi_buf_t buf = NULL;
	wmi_set_periodic_channel_stats_config *cmd = NULL;
	QDF_STATUS error = 0;
	int32_t len = 0;

	len = sizeof(wmi_set_periodic_channel_stats_config);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s: Unable to allocate merory\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_set_periodic_channel_stats_config *) wmi_buf_data(buf);
	cmd->enable = param->enable;
	cmd->stats_period = param->stats_period;

	error = wmi_unified_cmd_send(wmi_handle, buf, len,
					 WMI_SET_PERIODIC_CHANNEL_STATS_CONFIG);

	if (error)
		qdf_print(" %s :WMI Failed\n", __func__);

	return error;
}

/**
 * send_nf_dbr_dbm_info_get_cmd_non_tlv() - send request to get nf to fw
 * @wmi_handle: wmi handle
 * @mac_id: radio context
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_nf_dbr_dbm_info_get_cmd_non_tlv(wmi_unified_t wmi_handle, uint8_t mac_id)
{
	wmi_buf_t wmibuf;

	wmibuf = wmi_buf_alloc(wmi_handle, 0);
	if (wmibuf == NULL)
		return QDF_STATUS_E_NOMEM;

	return wmi_unified_cmd_send(wmi_handle, wmibuf, 0,
				WMI_PDEV_GET_NFCAL_POWER_CMDID);
}

/**
 * enum packet_power_non_tlv_flags: Target defined
 * packet power rate flags
 * @WMI_NON_TLV_FLAG_ONE_CHAIN: one chain
 * @WMI_NON_TLV_FLAG_TWO_CHAIN: two chain
 * @WMI_NON_TLV_FLAG_THREE_CHAIN: three chain
 * @WMI_NON_TLV_FLAG_FOUR_CHAIN: four chain
 * @WMI_NON_TLV_FLAG_STBC: STBC is set
 * @WMI_NON_TLV_FLAG_40MHZ: 40MHz channel width
 * @WMI_NON_TLV_FLAG_80MHZ: 80MHz channel width
 * @WMI_NON_TLV_FLAG_1600MHZ: 1600MHz channel width
 * @WMI_NON_TLV_FLAG_TXBF: Tx Bf enabled
 * @WMI_NON_TLV_FLAG_RTSENA: RTS enabled
 * @WMI_NON_TLV_FLAG_CTSENA: CTS enabled
 * @WMI_NON_TLV_FLAG_LDPC: LDPC is set
 * @WMI_NON_TLV_FLAG_SERIES1: Rate series 1
 * @WMI_NON_TLV_FLAG_SGI: Short gaurd interval
 * @WMI_NON_TLV_FLAG_MU2: MU2 data
 * @WMI_NON_TLV_FLAG_MU3: MU3 data
 */
enum packet_power_non_tlv_flags {
	WMI_NON_TLV_FLAG_ONE_CHAIN     = 0x0001,
	WMI_NON_TLV_FLAG_TWO_CHAIN     = 0x0005,
	WMI_NON_TLV_FLAG_THREE_CHAIN   = 0x0007,
	WMI_NON_TLV_FLAG_FOUR_CHAIN    = 0x000F,
	WMI_NON_TLV_FLAG_STBC          = 0x0010,
	WMI_NON_TLV_FLAG_40MHZ         = 0x0020,
	WMI_NON_TLV_FLAG_80MHZ         = 0x0040,
	WMI_NON_TLV_FLAG_160MHZ        = 0x0080,
	WMI_NON_TLV_FLAG_TXBF          = 0x0100,
	WMI_NON_TLV_FLAG_RTSENA        = 0x0200,
	WMI_NON_TLV_FLAG_CTSENA        = 0x0400,
	WMI_NON_TLV_FLAG_LDPC          = 0x0800,
	WMI_NON_TLV_FLAG_SERIES1       = 0x1000,
	WMI_NON_TLV_FLAG_SGI           = 0x2000,
	WMI_NON_TLV_FLAG_MU2           = 0x4000,
	WMI_NON_TLV_FLAG_MU3           = 0x8000,
};

/**
 * convert_to_power_info_rate_flags() - convert packet_power_info_params
 * to FW understandable format
 * @param: pointer to hold packet power info param
 *
 * @return FW understandable 16 bit rate flags
 */
static uint16_t
convert_to_power_info_rate_flags(struct packet_power_info_params *param)
{
	uint16_t rateflags = 0;

	if (param->chainmask)
		rateflags |= (param->chainmask & 0xf);
	if (param->chan_width == WMI_HOST_CHAN_WIDTH_40)
		rateflags |= WMI_NON_TLV_FLAG_40MHZ;
	if (param->chan_width == WMI_HOST_CHAN_WIDTH_80)
		rateflags |= WMI_NON_TLV_FLAG_80MHZ;
	if (param->chan_width == WMI_HOST_CHAN_WIDTH_160)
		rateflags |= WMI_NON_TLV_FLAG_160MHZ;
	if (param->rate_flags & WMI_HOST_FLAG_STBC)
		rateflags |= WMI_NON_TLV_FLAG_STBC;
	if (param->rate_flags & WMI_HOST_FLAG_LDPC)
		rateflags |= WMI_NON_TLV_FLAG_LDPC;
	if (param->rate_flags & WMI_HOST_FLAG_TXBF)
		rateflags |= WMI_NON_TLV_FLAG_TXBF;
	if (param->rate_flags & WMI_HOST_FLAG_RTSENA)
		rateflags |= WMI_NON_TLV_FLAG_RTSENA;
	if (param->rate_flags & WMI_HOST_FLAG_CTSENA)
		rateflags |= WMI_NON_TLV_FLAG_CTSENA;
	if (param->rate_flags & WMI_HOST_FLAG_SGI)
		rateflags |= WMI_NON_TLV_FLAG_SGI;
	if (param->rate_flags & WMI_HOST_FLAG_SERIES1)
		rateflags |= WMI_NON_TLV_FLAG_SERIES1;
	if (param->rate_flags & WMI_HOST_FLAG_MU2)
		rateflags |= WMI_NON_TLV_FLAG_MU2;
	if (param->rate_flags & WMI_HOST_FLAG_MU3)
		rateflags |= WMI_NON_TLV_FLAG_MU3;

	return rateflags;
}

/**
 * send_packet_power_info_get_cmd_non_tlv() - send request to get packet power
 * info to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold packet power info param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_packet_power_info_get_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct packet_power_info_params *param)
{
	wmi_pdev_get_tpc_cmd *cmd;
	wmi_buf_t wmibuf;
	u_int32_t len = sizeof(wmi_pdev_get_tpc_cmd);

	wmibuf = wmi_buf_alloc(wmi_handle, len);
	if (wmibuf == NULL)
		return QDF_STATUS_E_NOMEM;

	cmd = (wmi_pdev_get_tpc_cmd *)wmi_buf_data(wmibuf);
	cmd->rate_flags = convert_to_power_info_rate_flags(param);
	cmd->nss = param->nss;
	cmd->preamble = param->preamble;
	cmd->hw_rate = param->hw_rate;
	cmd->rsvd = 0x0;

	WMI_LOGD("%s[%d] commandID %d, wmi_pdev_get_tpc_cmd=0x%x,"
		"rate_flags: 0x%x, nss: %d, preamble: %d, hw_rate: %d\n",
		__func__, __LINE__, WMI_PDEV_GET_TPC_CMDID, *((u_int32_t *)cmd),
		cmd->rate_flags, cmd->nss, cmd->preamble, cmd->hw_rate);

	return wmi_unified_cmd_send(wmi_handle, wmibuf, len,
				   WMI_PDEV_GET_TPC_CMDID);
}

/**
 * send_gpio_config_cmd_non_tlv() - send gpio config to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold gpio config param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_gpio_config_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct gpio_config_params *param)
{
	wmi_gpio_config_cmd *cmd;
	wmi_buf_t wmibuf;
	u_int32_t len = sizeof(wmi_gpio_config_cmd);

	/* Sanity Checks */
	if (param->pull_type > WMI_GPIO_PULL_DOWN ||
		param->intr_mode > WMI_GPIO_INTTYPE_LEVEL_HIGH) {
		return QDF_STATUS_E_FAILURE;
	}

	wmibuf = wmi_buf_alloc(wmi_handle, len);
	if (wmibuf == NULL)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_gpio_config_cmd *)wmi_buf_data(wmibuf);
	cmd->gpio_num = param->gpio_num;
	cmd->input = param->input;
	cmd->pull_type = param->pull_type;
	cmd->intr_mode = param->intr_mode;
	return wmi_unified_cmd_send(wmi_handle, wmibuf, len,
		WMI_GPIO_CONFIG_CMDID);
}

/**
 * send_gpio_output_cmd_non_tlv() - send gpio output to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold gpio output param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_gpio_output_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct gpio_output_params *param)
{
	wmi_gpio_output_cmd *cmd;
	wmi_buf_t wmibuf;
	u_int32_t len = sizeof(wmi_gpio_output_cmd);

	wmibuf = wmi_buf_alloc(wmi_handle, len);
	if (wmibuf == NULL)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_gpio_output_cmd *)wmi_buf_data(wmibuf);
	cmd->gpio_num = param->gpio_num;
	cmd->set = param->set;
	return wmi_unified_cmd_send(wmi_handle, wmibuf, len,
		WMI_GPIO_OUTPUT_CMDID);
}

/*
 * send_rtt_meas_req_test_cmd_non_tlv() - send rtt meas req test cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold rtt meas req test param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_rtt_meas_req_test_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct rtt_meas_req_test_params *param)
{
	wmi_buf_t buf;
	u_int8_t *p;
	int ret;
	u_int16_t len;
	wmi_rtt_measreq_head *head;
	wmi_rtt_measreq_body *body;
	wmi_channel *w_chan;

	qdf_print("%s: The request ID is: %d\n", __func__, param->req_id);

	len = sizeof(wmi_rtt_measreq_head) + param->req_num_req *
	    sizeof(wmi_rtt_measreq_body);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("No WMI resource!");
		return QDF_STATUS_E_NOMEM;
	}

	p = (u_int8_t *) wmi_buf_data(buf);
	qdf_mem_set(p, len, 0);

	head = (wmi_rtt_measreq_head *) p;
	WMI_RTT_REQ_ID_SET(head->req_id, param->req_id);
	WMI_RTT_SPS_SET(head->req_id, 1);

	WMI_RTT_NUM_STA_SET(head->sta_num, param->req_num_req);

	body = &(head->body[0]);
	WMI_RTT_VDEV_ID_SET(body->measure_info, 0);
	WMI_RTT_TIMEOUT_SET(body->measure_info, 100);
	WMI_RTT_REPORT_TYPE_SET(body->measure_info, param->req_report_type);
	WMI_RTT_FRAME_TYPE_SET(body->control_flag, param->req_frame_type);

	WMI_RTT_TX_CHAIN_SET(body->control_flag, 001);
	WMI_RTT_QCA_PEER_SET(body->control_flag, 1);
	if (param->req_preamble == WMI_RTT_PREAM_LEGACY)
		WMI_RTT_MCS_SET(body->control_flag, 3);
	else
		WMI_RTT_MCS_SET(body->control_flag, 0);
	WMI_RTT_RETRIES_SET(body->control_flag, 1);

	/*
	qdf_mem_copy(peer, param->mac_addr, 6);

	qdf_print("The mac_addr is"
		 " %.2x:%.2x:%.2x:%.2x:%.2x:%.2x extra=%d\n",
		 peer[0], peer[1], peer[2],
		 peer[3], peer[4], peer[5], param->extra);
	*/

	/* start from here, embed the first req in each RTT measurement
	 * Command */
	/*peer[5] = 0x12;
	peer[4] = 0x90;
	peer[3] = 0x78;
	peer[2] = 0x56;
	peer[1] = 0x34;
	peer[0] = 0x12;
>---*/
	head->channel.mhz = param->channel.mhz;
	head->channel.band_center_freq1 = param->channel.cfreq1;
	head->channel.band_center_freq2 = param->channel.cfreq2;


	w_chan = (wmi_channel *)&head->channel;
	WMI_SET_CHANNEL_MODE(w_chan, param->channel.phy_mode);
	WMI_SET_CHANNEL_MIN_POWER(w_chan, param->channel.minpower);
	WMI_SET_CHANNEL_MAX_POWER(w_chan, param->channel.maxpower);
	WMI_SET_CHANNEL_REG_POWER(w_chan, param->channel.maxregpower);
	WMI_SET_CHANNEL_ANTENNA_MAX(w_chan, param->channel.antennamax);
	WMI_SET_CHANNEL_REG_CLASSID(w_chan, param->channel.reg_class_id);

	WMI_CHAR_ARRAY_TO_MAC_ADDR(((u_int8_t *)param->peer), &body->dest_mac);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(((u_int8_t *)param->peer),
		&body->spoof_bssid);

	WMI_RTT_BW_SET(body->control_flag, param->req_bw);
	WMI_RTT_PREAMBLE_SET(body->control_flag, param->req_preamble);
	WMI_RTT_MEAS_NUM_SET(body->measure_info, param->num_measurements);

	body->measure_params_1 = 0;
	body->measure_params_2 = 0;

	WMI_RTT_ASAP_MODE_SET(body->measure_params_1, param->asap_mode);
	WMI_RTT_LCI_REQ_SET(body->measure_params_1, param->lci_requested);
	WMI_RTT_LOC_CIV_REQ_SET(body->measure_params_1,
		param->loc_civ_requested);
	WMI_RTT_NUM_BURST_EXP_SET(body->measure_params_1, 0);
	WMI_RTT_BURST_DUR_SET(body->measure_params_1, 15);
	WMI_RTT_BURST_PERIOD_SET(body->measure_params_1, 0);
	WMI_RTT_TSF_DELTA_VALID_SET(body->measure_params_1, 1);
	WMI_RTT_TSF_DELTA_SET(body->measure_params_2, 0);

	/** other requests are same with first request */
	p = (u_int8_t *) body;
	while (--param->req_num_req) {
		body++;
		qdf_mem_copy(body, p, sizeof(wmi_rtt_measreq_body));
	}

	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_RTT_MEASREQ_CMDID);
	qdf_print("send rtt cmd to FW with length %d and return %d\n",
		len, ret);
	return QDF_STATUS_SUCCESS;
}

/**
 * send_rtt_meas_req_cmd_non_tlv() - send rtt meas req cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold rtt meas req param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_rtt_meas_req_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct rtt_meas_req_params *param)
{
	wmi_buf_t buf;
	uint8_t *p;
	int ret;
	uint16_t len;
	uint8_t peer[6];
	uint8_t spoof[6];
	wmi_rtt_measreq_head *head;
	wmi_rtt_measreq_body *body;
	int req_frame_type, req_preamble;
	wmi_channel *w_chan;

	/* Temporarily, hardcoding peer mac address for test purpose will be
	 * removed once RTT host has been developed for even req_id, like
	 * 0, 2, 4, there is no channel_swicth for odd req_id, like 1, 3 , 5,
	 * there is channel switch currently, for both cases, we have 3 req in
	 * each command please change here if you only have one (or just let
	 * it be). Even == HC, odd == OC.
	 */
	if (!(param->req_id & 0x1)) {
		len = sizeof(wmi_rtt_measreq_head);
		/* + 2 * sizeof(wmi_rtt_measreq_body);*/
	} else {
		len = sizeof(wmi_rtt_measreq_head);
		/* + 2 * sizeof(wmi_rtt_measreq_body);*/
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("No WMI resource!");
		return QDF_STATUS_E_FAILURE;
	}

	p = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_set(p, len, 0);

	/* encode header */
	head = (wmi_rtt_measreq_head *) p;
	/* head->req_id = req_id;*/
	WMI_RTT_REQ_ID_SET(head->req_id, param->req_id);
	/* WMI_RTT_SPS_SET(head->req_id, 1);*/

	if (!(param->req_id & 0x1)) { /*even req id */
#ifndef RTT_TEST
		/* we actually only have 3 sta to measure
		this is used to test over limit request protection
		XIN:WMI_RTT_NUM_STA_SET(head->sta_num, 5);*/
#else
		/* XIN:WMI_RTT_NUM_STA_SET(head->sta_num, 2);*/
		WMI_RTT_NUM_STA_SET(head->sta_num, 1);
#endif
		WMI_RTT_NUM_STA_SET(head->sta_num, 1);
	} else { /* odd req id */
		/* XIN:WMI_RTT_NUM_STA_SET(head->sta_num, 3); */
		WMI_RTT_NUM_STA_SET(head->sta_num, 1);

	}

	req_frame_type = RTT_MEAS_FRAME_NULL;
	/* MS(extra, RTT_REQ_FRAME_TYPE);*/
	/* req_bw		 = //MS(extra, RTT_REQ_BW);*/
	req_preamble   = WMI_RTT_PREAM_LEGACY;/*MS(extra, RTT_REQ_PREAMBLE);*/

	/*encode common parts for each RTT measurement command body
	The value here can be overwrite in following each req hardcoding */
	body = &(head->body[0]);
	WMI_RTT_VDEV_ID_SET(body->measure_info, param->vdev_id);
	WMI_RTT_TIMEOUT_SET(body->measure_info, RTT_TIMEOUT_MS);
	WMI_RTT_REPORT_TYPE_SET(body->measure_info, 1);
	WMI_RTT_FRAME_TYPE_SET(body->control_flag, req_frame_type);
	WMI_RTT_TX_CHAIN_SET(body->control_flag, 001);
	WMI_RTT_QCA_PEER_SET(body->control_flag, 1);
	if (req_preamble == WMI_RTT_PREAM_LEGACY)
		WMI_RTT_MCS_SET(body->control_flag, 3);
	else
		WMI_RTT_MCS_SET(body->control_flag, 0);
	WMI_RTT_RETRIES_SET(body->control_flag, 1);

	if (!(param->req_id & 0x1)) { /* even time */
	qdf_mem_copy(peer, param->sta_mac_addr, 6);
	} else { /* odd time */
		qdf_mem_copy(peer, param->sta_mac_addr, 6);
	}
	head->channel.mhz = param->channel.mhz;
	head->channel.band_center_freq1 = param->channel.cfreq1;
	head->channel.band_center_freq2 = param->channel.cfreq2;

	w_chan = (wmi_channel *)&head->channel;
	WMI_SET_CHANNEL_MAX_POWER(w_chan, param->channel.phy_mode);
	WMI_SET_CHANNEL_MIN_POWER(w_chan, param->channel.minpower);
	WMI_SET_CHANNEL_MAX_POWER(w_chan, param->channel.maxpower);
	WMI_SET_CHANNEL_REG_POWER(w_chan, param->channel.maxregpower);
	WMI_SET_CHANNEL_ANTENNA_MAX(w_chan, param->channel.antennamax);
	WMI_SET_CHANNEL_REG_CLASSID(w_chan, param->channel.reg_class_id);

	if (param->is_mode_na)
		WMI_SET_CHANNEL_MODE(w_chan, MODE_11NG_HT20);
	else if (param->is_mode_ac)
		WMI_SET_CHANNEL_MODE(w_chan, MODE_11NA_HT20);

	if (param->channel.dfs_set)
		WMI_SET_CHANNEL_FLAG(w_chan, WMI_CHAN_FLAG_DFS);

	WMI_CHAR_ARRAY_TO_MAC_ADDR(((uint8_t *)peer), &body->dest_mac);
	qdf_mem_set(spoof, IEEE80211_ADDR_LEN, 0);
	WMI_CHAR_ARRAY_TO_MAC_ADDR(((uint8_t *)param->spoof_mac_addr),
		&body->spoof_bssid);

	/** embedded varing part of each request
	set Preamble, BW, measurement times */
	if (param->is_bw_20)
		WMI_RTT_BW_SET(body->control_flag, WMI_RTT_BW_20);
	else if (param->is_bw_40)
		WMI_RTT_BW_SET(body->control_flag, WMI_RTT_BW_40);
	else if (param->is_bw_80)
		WMI_RTT_BW_SET(body->control_flag, WMI_RTT_BW_80);
	else
		WMI_RTT_BW_SET(body->control_flag, WMI_RTT_BW_20);
	WMI_RTT_PREAMBLE_SET(body->control_flag, req_preamble);
	WMI_RTT_MEAS_NUM_SET(body->measure_info, param->num_probe_rqst);


	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_RTT_MEASREQ_CMDID);
	qdf_print("send rtt cmd to FW with length %d and return %d\n",
		len, ret);
	return ret;
}
/**
 * send_rtt_keepalive_req_cmd_non_tlv() - send rtt keepalive req cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold rtt keepalive req param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_rtt_keepalive_req_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct rtt_keepalive_req_params *param)
{
	wmi_buf_t buf;
	wmi_rtt_keepalive_cmd *cmd;
	int ret;
	uint16_t len;
	uint8_t *ptr;

	len = sizeof(wmi_rtt_keepalive_cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("No WMI resource\n");
		return QDF_STATUS_E_FAILURE;
	}
	ptr = (uint8_t *)wmi_buf_data(buf);
	OS_MEMSET(ptr, 0, len);

	cmd = (wmi_rtt_keepalive_cmd *)wmi_buf_data(buf);

	WMI_RTT_REQ_ID_SET(cmd->req_id, param->req_id);
	WMI_RTT_KEEPALIVE_ACTION_SET(cmd->req_id, param->stop);
	WMI_RTT_VDEV_ID_SET(cmd->probe_info, param->vdev_id);
	/* 3ms probe interval by default */
	WMI_RTT_KEEPALIVE_PERIOD_SET(cmd->probe_info, 3);
	/* max retry of 50 by default */
	WMI_RTT_TIMEOUT_SET(cmd->probe_info, 20);
	/* set frame type */
	WMI_RTT_FRAME_TYPE_SET(cmd->control_flag, RTT_MEAS_FRAME_KEEPALIVE);

	WMI_CHAR_ARRAY_TO_MAC_ADDR(param->macaddr, &cmd->sta_mac);

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
		WMI_RTT_KEEPALIVE_CMDID);
	qdf_print("send rtt keepalive cmd to FW with length %d and return %d\n"
		, len, ret);
	param->req_id++;

	return QDF_STATUS_SUCCESS;
}
/**
 * send_lci_set_cmd_non_tlv() - send lci cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold lci param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_lci_set_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct lci_set_params *param)
{
	wmi_buf_t buf;
	uint8_t *p;
	wmi_oem_measreq_head *head;
	int len;
	int colocated_bss_len = 0;
	wmi_rtt_lci_cfg_head *rtt_req;

	rtt_req = (wmi_rtt_lci_cfg_head *) param->lci_data;

	len = param->msg_len;

	/* colocated_bss[1] contains num of vaps */
	/* Provide colocated bssid subIE only when there are 2 vaps or more */
	if (param->colocated_bss[1] > 1) {
		qdf_print("%s: Adding %d co-located BSSIDs to LCI data\n",
				 __func__, param->colocated_bss[1]);
		/* Convert num_vaps to octets:
		   6*Num_of_vap + 1 (Max BSSID Indicator field) */
		param->colocated_bss[1] =
			(param->colocated_bss[1]*IEEE80211_ADDR_LEN)+1;
		colocated_bss_len =  param->colocated_bss[1]+2;
		qdf_mem_copy(rtt_req->colocated_bssids_info,
				param->colocated_bss,
				colocated_bss_len);
		rtt_req->co_located_bssid_len = colocated_bss_len;
		qdf_print("%s: co_located_bssid_len: %d\n", __func__,
				param->colocated_bss[1]+2);
	} else {
		qdf_print("No co-located BSSID was added to LCI data\n");
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("No WMI resource!");
		return QDF_STATUS_E_FAILURE;
	}

	p = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_set(p, len, 0);

	head = (wmi_oem_measreq_head *)p;
	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(head, param->lci_data, len);
	if (wmi_unified_cmd_send(wmi_handle, buf, len, WMI_OEM_REQ_CMDID))
		return QDF_STATUS_E_FAILURE;

	/* Save LCI data in host buffer */
	{

		param->latitude_unc = WMI_RTT_LCI_LAT_UNC_GET(
			rtt_req->lci_cfg_param_info);
		param->latitude_0_1 = ((uint32_t)(rtt_req->latitude & 0x3));
		param->latitude_2_33 = (uint32_t)
		    (((uint64_t)(rtt_req->latitude)) >> 2);
		param->longitude_unc =
		    WMI_RTT_LCI_LON_UNC_GET(rtt_req->lci_cfg_param_info);
		param->longitude_0_1 = ((uint32_t)(rtt_req->longitude & 0x3));
		param->longitude_2_33 =
		    (uint32_t)(((uint64_t)(rtt_req->longitude)) >> 2);
		param->altitude_type =
		    WMI_RTT_LCI_ALT_TYPE_GET(rtt_req->altitude_info);
		param->altitude_unc_0_3 =
		    (WMI_RTT_LCI_ALT_UNC_GET(rtt_req->altitude_info) & 0xF);
		param->altitude_unc_4_5 =
		    ((WMI_RTT_LCI_ALT_UNC_GET(rtt_req->altitude_info) >> 4) &
		     0x3);
		param->altitude = (rtt_req->altitude & RTT_LCI_ALTITUDE_MASK);
		param->datum =
		    WMI_RTT_LCI_DATUM_GET(rtt_req->lci_cfg_param_info);
		param->reg_loc_agmt =
		    WMI_RTT_LCI_REG_LOC_AGMT_GET(rtt_req->lci_cfg_param_info);
		param->reg_loc_dse =
		    WMI_RTT_LCI_REG_LOC_DSE_GET(rtt_req->lci_cfg_param_info);
		param->dep_sta =
		    WMI_RTT_LCI_DEP_STA_GET(rtt_req->lci_cfg_param_info);
		param->version =
		    WMI_RTT_LCI_VERSION_GET(rtt_req->lci_cfg_param_info);

	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_lcr_set_cmd_non_tlv() - send lcr cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold lcr param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_lcr_set_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct lcr_set_params *param)
{
	wmi_buf_t buf;
	uint8_t *p;
	wmi_oem_measreq_head *head;
	int len;

	len = param->msg_len;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("No WMI resource!");
		return QDF_STATUS_E_FAILURE;
	}

	p = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_set(p, len, 0);

	head = (wmi_oem_measreq_head *)p;
	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(head, param->lcr_data, len);

	if (wmi_unified_cmd_send(wmi_handle, buf, len, WMI_OEM_REQ_CMDID))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

 /**
 * send_start_oem_data_cmd_non_tlv() - send oem req cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold oem req param
 */
static QDF_STATUS
send_start_oem_data_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint32_t data_len,
				uint8_t *data)
{
	wmi_buf_t buf;
	uint8_t *p;
	wmi_oem_measreq_head *head;

	buf = wmi_buf_alloc(wmi_handle, data_len);
	if (!buf) {
		qdf_print("%s: No WMI resource!\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	p = (uint8_t *) wmi_buf_data(buf);
	qdf_mem_set(p, data_len, 0);

	head = (wmi_oem_measreq_head *)p;
	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(head, data, data_len);

	if (wmi_unified_cmd_send(wmi_handle, buf,
				data_len, WMI_OEM_REQ_CMDID)) {
		qdf_print("%s: ERROR: Host unable to send LOWI request to FW\n",
				__func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_get_user_position_cmd_non_tlv() - send cmd get user position from fw
 * @wmi_handle: wmi handle
 * @value: user pos value
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_get_user_position_cmd_non_tlv(wmi_unified_t wmi_handle, uint32_t value)
{
	wmi_buf_t buf;
	wmi_peer_gid_userpos_list_cmd *cmd;

	buf = wmi_buf_alloc(wmi_handle, sizeof(wmi_peer_gid_userpos_list_cmd));
	if (!buf) {
		qdf_print("No WMI resource!");
		return QDF_STATUS_E_FAILURE;
	}
	qdf_nbuf_put_tail(buf, sizeof(wmi_peer_gid_userpos_list_cmd));
	cmd = (wmi_peer_gid_userpos_list_cmd *)(wmi_buf_data(buf));
	cmd->aid = value;

	if (wmi_unified_cmd_send(wmi_handle, buf,
		sizeof(wmi_peer_gid_userpos_list_cmd),
		WMI_PEER_GID_USERPOS_LIST_CMDID)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_reset_peer_mumimo_tx_count_cmd_non_tlv() - send mumimo reset tx count fw
 * @wmi_handle: wmi handle
 * @value:	reset tx count
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_reset_peer_mumimo_tx_count_cmd_non_tlv(wmi_unified_t wmi_handle,
						uint32_t value)
{
	wmi_buf_t buf;
	wmi_peer_txmu_rstcnt_cmd *cmd;

	buf = wmi_buf_alloc(wmi_handle, sizeof(wmi_peer_txmu_rstcnt_cmd));
	if (!buf) {
		qdf_print("No WMI resource!");
		return QDF_STATUS_E_FAILURE;
	}
	qdf_nbuf_put_tail(buf, sizeof(wmi_peer_txmu_rstcnt_cmd));
	cmd = (wmi_peer_txmu_rstcnt_cmd *)(wmi_buf_data(buf));
	cmd->aid = value;

	if (wmi_unified_cmd_send(wmi_handle, buf,
		sizeof(wmi_peer_txmu_rstcnt_cmd),
		WMI_PEER_TX_MU_TXMIT_RSTCNT_CMDID)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_get_peer_mumimo_tx_count_cmd_non_tlv() - send cmd to get mumimo tx count from fw
 * @wmi_handle: wmi handle
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_get_peer_mumimo_tx_count_cmd_non_tlv(wmi_unified_t wmi_handle,
						uint32_t value)
{
	wmi_buf_t buf;
	wmi_peer_txmu_cnt_cmd *cmd;

	buf = wmi_buf_alloc(wmi_handle, sizeof(wmi_peer_txmu_cnt_cmd));
	if (!buf) {
		qdf_print("No WMI resource!");
		return QDF_STATUS_E_FAILURE;
	}
	qdf_nbuf_put_tail(buf, sizeof(wmi_peer_txmu_cnt_cmd));
	cmd = (wmi_peer_txmu_cnt_cmd *)(wmi_buf_data(buf));
	cmd->aid = value;

	if (wmi_unified_cmd_send(wmi_handle, buf,
		sizeof(wmi_peer_txmu_cnt_cmd),
		WMI_PEER_TX_MU_TXMIT_COUNT_CMDID)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_pdev_caldata_version_check_cmd_non_tlv() - send caldata check cmd to fw
 * @wmi_handle: wmi handle
 * @param:	reserved param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_pdev_caldata_version_check_cmd_non_tlv(wmi_unified_t wmi_handle,
						uint32_t param)
{
	wmi_pdev_check_cal_version_cmd *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(wmi_pdev_check_cal_version_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_pdev_check_cal_version_cmd *)wmi_buf_data(buf);
	cmd->reserved = param; /* set to 0x0 as expected from FW */
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_PDEV_CHECK_CAL_VERSION_CMDID)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_btcoex_wlan_priority_cmd_non_tlv() - send btcoex wlan priority fw
 * @wmi_handle: wmi handle
 * @param: btcoex config params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_btcoex_wlan_priority_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct btcoex_cfg_params *param)
{
	wmi_buf_t buf;
	wmi_btcoex_cfg_cmd *cmd;
	int len = sizeof(wmi_btcoex_cfg_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_btcoex_cfg_cmd *) wmi_buf_data(buf);

	cmd->btcoex_wlan_priority_bitmap = param->btcoex_wlan_priority_bitmap;
	cmd->btcoex_param_flags = param->btcoex_param_flags;
	if (wmi_unified_cmd_send(wmi_handle, buf, len, WMI_BTCOEX_CFG_CMDID)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_btcoex_duty_cycle_cmd_non_tlv() - send btcoex wlan priority fw
 * @wmi_handle: wmi handle
 * @param:	period and duration
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_btcoex_duty_cycle_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct btcoex_cfg_params *param)
{
	wmi_buf_t buf;
	wmi_btcoex_cfg_cmd *cmd;
	int len = sizeof(wmi_btcoex_cfg_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (wmi_btcoex_cfg_cmd *) wmi_buf_data(buf);
	cmd->wlan_duration = param->wlan_duration;
	cmd->period = param->period;
	cmd->btcoex_param_flags = param->btcoex_param_flags;
	if (wmi_unified_cmd_send(wmi_handle, buf, len, WMI_BTCOEX_CFG_CMDID)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_coex_ver_cfg_cmd_non_tlv() - send coex ver cfg
 * @wmi_handle: wmi handle
 * @param:     coex ver and configuration
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS
send_coex_ver_cfg_cmd_non_tlv(wmi_unified_t wmi_handle, coex_ver_cfg_t *param)
{
	wmi_buf_t buf;
	coex_ver_cfg_t *cmd;
	int len = sizeof(wmi_coex_ver_cfg_cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	cmd = (coex_ver_cfg_t *)wmi_buf_data(buf);
	cmd->coex_version = param->coex_version;
	cmd->length = param->length;
	qdf_mem_copy(cmd->config_buf, param->config_buf,
		     sizeof(cmd->config_buf));
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_COEX_VERSION_CFG_CMID)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_copy_resource_config_non_tlv() - copy resource configuration function
 * @param resource_cfg: pointer to resource configuration
 * @param tgt_res_cfg: pointer to target resource configuration
 *
 * Return: None
 */
static void wmi_copy_resource_config_non_tlv(wmi_resource_config *resource_cfg,
					target_resource_config *tgt_res_cfg)
{
	resource_cfg->num_vdevs = tgt_res_cfg->num_vdevs;
	resource_cfg->num_peers = tgt_res_cfg->num_peers;
	resource_cfg->num_active_peers = tgt_res_cfg->num_active_peers;
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
	resource_cfg->scan_max_pending_req = tgt_res_cfg->scan_max_pending_req;
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
	resource_cfg->max_peer_ext_stats = tgt_res_cfg->max_peer_ext_stats;
	resource_cfg->smart_ant_cap = tgt_res_cfg->smart_ant_cap;
	resource_cfg->BK_Minfree = tgt_res_cfg->BK_Minfree;
	resource_cfg->BE_Minfree = tgt_res_cfg->BE_Minfree;
	resource_cfg->VI_Minfree = tgt_res_cfg->VI_Minfree;
	resource_cfg->VO_Minfree = tgt_res_cfg->VO_Minfree;
	resource_cfg->rx_batchmode = tgt_res_cfg->rx_batchmode;
	resource_cfg->tt_support = tgt_res_cfg->tt_support;
	resource_cfg->atf_config = tgt_res_cfg->atf_config;
	resource_cfg->iphdr_pad_config = tgt_res_cfg->iphdr_pad_config;
	WMI_SET_QWRAP(resource_cfg, tgt_res_cfg->qwrap_config);
	WMI_SET_ALLOC_FRAG(resource_cfg,
		tgt_res_cfg->alloc_frag_desc_for_data_pkt);
}

/**
 * init_cmd_send_non_tlv() - send initialization cmd to fw
 * @wmi_handle: wmi handle
 * @param param: pointer to wmi init param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS init_cmd_send_non_tlv(wmi_unified_t wmi_handle,
				struct wmi_init_cmd_param *param)
{
	wmi_buf_t buf;
	wmi_init_cmd *cmd;
	wlan_host_memory_chunk *host_mem_chunks;
	uint32_t mem_chunk_len = 0;
	uint16_t idx;
	int len;

	len = sizeof(*cmd);
	mem_chunk_len = (sizeof(wlan_host_memory_chunk) * MAX_MEM_CHUNKS);
	buf = wmi_buf_alloc(wmi_handle, len + mem_chunk_len);
	if (!buf) {
		qdf_print("%s: wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = (wmi_init_cmd *) wmi_buf_data(buf);

	wmi_copy_resource_config_non_tlv(&cmd->resource_config, param->res_cfg);

	host_mem_chunks = cmd->host_mem_chunks;
	for (idx = 0; idx < param->num_mem_chunks; ++idx) {
		host_mem_chunks[idx].ptr = param->mem_chunks[idx].paddr;
		host_mem_chunks[idx].size = param->mem_chunks[idx].len;
		host_mem_chunks[idx].req_id = param->mem_chunks[idx].req_id;
		qdf_print("chunk %d len %d requested , ptr  0x%x\n",
				idx, cmd->host_mem_chunks[idx].size,
				cmd->host_mem_chunks[idx].ptr);
	}
	cmd->num_host_mem_chunks = param->num_mem_chunks;
	if (param->num_mem_chunks > 1)
		len += ((param->num_mem_chunks-1) *
				sizeof(wlan_host_memory_chunk));

	if (wmi_unified_cmd_send(wmi_handle, buf, len, WMI_INIT_CMDID) < 0) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_ext_resource_config_non_tlv() - send extended resource configuration
 * @wmi_handle: wmi handle
 * @param ext_cfg: pointer to extended resource configuration
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS send_ext_resource_config_non_tlv(wmi_unified_t wmi_handle,
	wmi_host_ext_resource_config *ext_cfg)
{
	wmi_buf_t buf;
	int len = 0;
	wmi_ext_resource_config *cmd_cfg;

#define PAD_LENGTH 100
	buf = wmi_buf_alloc(wmi_handle,
			len + (sizeof(wmi_ext_resource_config) + PAD_LENGTH));
	if (!buf) {
		qdf_print("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	cmd_cfg = (wmi_ext_resource_config *)wmi_buf_data(buf);
	qdf_mem_copy(cmd_cfg, ext_cfg, sizeof(wmi_ext_resource_config));
	qdf_print("\nSending Ext resource cfg: HOST PLATFORM as %d\n"
			"fw_feature_bitmap as %x to TGT\n",
			cmd_cfg->host_platform_config,
			cmd_cfg->fw_feature_bitmap);
	if (wmi_unified_cmd_send(wmi_handle, buf,
			  sizeof(wmi_ext_resource_config),
			  WMI_EXT_RESOURCE_CFG_CMDID) < 0) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * save_service_bitmap_non_tlv() - save service bitmap
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param bitmap_buf: bitmap buffer for converged legacy support
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS save_service_bitmap_non_tlv(wmi_unified_t wmi_handle,
				 void *evt_buf, void *bitmap_buf)
{
	wmi_service_ready_event *ev;
	struct wmi_soc *soc = wmi_handle->soc;

	ev = (wmi_service_ready_event *) evt_buf;

	/* If it is already allocated, use that buffer. This can happen
	 * during target stop/start scenarios where host allocation is skipped.
	 */
	if (!soc->wmi_service_bitmap) {
		soc->wmi_service_bitmap =
			qdf_mem_malloc(WMI_SERVICE_BM_SIZE * sizeof(uint32_t));
		if (!soc->wmi_service_bitmap) {
			WMI_LOGE("Failed memory alloc for service bitmap\n");
			return QDF_STATUS_E_NOMEM;
		}
	}

	qdf_mem_copy(soc->wmi_service_bitmap, ev->wmi_service_bitmap,
				(WMI_SERVICE_BM_SIZE * sizeof(uint32_t)));

	if (bitmap_buf)
		qdf_mem_copy(bitmap_buf, ev->wmi_service_bitmap,
				(WMI_SERVICE_BM_SIZE * sizeof(uint32_t)));

	return QDF_STATUS_SUCCESS;
}

/**
 * is_service_enabled_non_tlv() - Check if service enabled
 * @param wmi_handle: wmi handle
 * @param service_id: service identifier
 *
 * Return: 1 enabled, 0 disabled
 */
static bool is_service_enabled_non_tlv(wmi_unified_t wmi_handle,
				uint32_t service_id)
{
	struct wmi_soc *soc = wmi_handle->soc;

	if (!soc->wmi_service_bitmap) {
		WMI_LOGE("WMI service bit map is not saved yet\n");
		return false;
	}

	return WMI_SERVICE_IS_ENABLED(soc->wmi_service_bitmap,
			service_id);
}

static inline void copy_ht_cap_info(uint32_t ev_target_cap,
				struct wlan_psoc_target_capability_info *cap)
{
	cap->ht_cap_info |= ev_target_cap & (
					WMI_HT_CAP_ENABLED
					| WMI_HT_CAP_HT20_SGI
					| WMI_HT_CAP_DYNAMIC_SMPS
					| WMI_HT_CAP_TX_STBC
					| WMI_HT_CAP_TX_STBC_MASK_SHIFT
					| WMI_HT_CAP_RX_STBC
					| WMI_HT_CAP_RX_STBC_MASK_SHIFT
					| WMI_HT_CAP_LDPC
					| WMI_HT_CAP_L_SIG_TXOP_PROT
					| WMI_HT_CAP_MPDU_DENSITY
					| WMI_HT_CAP_MPDU_DENSITY_MASK_SHIFT
					| WMI_HT_CAP_HT40_SGI
					| WMI_HT_CAP_IBF_BFER);
	if (ev_target_cap & WMI_HT_CAP_IBF_BFER)
			cap->ht_cap_info |= WMI_HOST_HT_CAP_IBF_BFER;
}

/**
 * extract_service_ready_non_tlv() - extract service ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to received event buffer
 * @param cap: pointer to hold target capability information extracted from even
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_service_ready_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		struct wlan_psoc_target_capability_info *cap)
{
	wmi_service_ready_event *ev;

	ev = (wmi_service_ready_event *) evt_buf;

	cap->phy_capability = ev->phy_capability;
	cap->max_frag_entry = ev->max_frag_entry;
	cap->num_rf_chains = ev->num_rf_chains;
	copy_ht_cap_info(ev->ht_cap_info, cap);
	cap->vht_cap_info = ev->vht_cap_info;
	cap->vht_supp_mcs = ev->vht_supp_mcs;
	cap->hw_min_tx_power = ev->hw_min_tx_power;
	cap->hw_max_tx_power = ev->hw_max_tx_power;
	cap->sys_cap_info = ev->sys_cap_info;
	cap->min_pkt_size_enable = ev->min_pkt_size_enable;
	cap->max_bcn_ie_size = ev->max_bcn_ie_size;
	cap->fw_version = ev->sw_version;
	cap->fw_version_1 = ev->sw_version_1;
	/* Following caps not recieved in older fw/hw
	 * Initialize it as zero(default). */
	cap->max_num_scan_channels = 0;
	cap->max_supported_macs = 0;
	cap->wmi_fw_sub_feat_caps = 0;
	cap->txrx_chainmask = 0;
	cap->default_dbs_hw_mode_index = 0;
	cap->num_msdu_desc = 0;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_fw_version_non_tlv() - extract fw version
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param fw_ver: Pointer to hold fw version
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_fw_version_non_tlv(wmi_unified_t wmi_handle,
				void *evt_buf, struct wmi_host_fw_ver *fw_ver)
{
	wmi_service_ready_event *ev;

	ev = (wmi_service_ready_event *) evt_buf;

	fw_ver->sw_version = ev->sw_version;
	fw_ver->sw_version_1 = ev->sw_version_1;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_fw_abi_version_non_tlv() - extract fw abi version
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param fw_ver: Pointer to hold fw abi version
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_fw_abi_version_non_tlv(wmi_unified_t wmi_handle,
			void *evt_buf, struct wmi_host_fw_abi_ver *fw_ver)
{
	wmi_ready_event *ev;

	ev = (wmi_ready_event *) evt_buf;

	fw_ver->sw_version = ev->sw_version;
	fw_ver->abi_version = ev->abi_version;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_hal_reg_cap_non_tlv() - extract HAL registered capabilities
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param cap: pointer to hold HAL reg capabilities
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_hal_reg_cap_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		struct wlan_psoc_hal_reg_capability *cap)
{
	wmi_service_ready_event *ev;
	u_int32_t wireless_modes_orig = 0;

	ev = (wmi_service_ready_event *) evt_buf;

	qdf_mem_copy(cap, &ev->hal_reg_capabilities,
			sizeof(struct wlan_psoc_hal_reg_capability));

	/* Convert REGDMN_MODE values sent by target to host internal
	 * WMI_HOST_REGDMN_MODE values.
	 *
	 * REGULATORY TODO :
	 * REGDMN_MODE_11AC_VHT*_2G values are not used by the
	 * host currently. Add this in the future if required.
	 */
	wireless_modes_orig = ev->hal_reg_capabilities.wireless_modes;
	cap->wireless_modes = 0;

	if (wireless_modes_orig & REGDMN_MODE_11A)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11A;

	if (wireless_modes_orig & REGDMN_MODE_TURBO)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_TURBO;

	if (wireless_modes_orig & REGDMN_MODE_11B)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11B;

	if (wireless_modes_orig & REGDMN_MODE_PUREG)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_PUREG;

	if (wireless_modes_orig & REGDMN_MODE_11G)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11G;

	if (wireless_modes_orig & REGDMN_MODE_108G)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_108G;

	if (wireless_modes_orig & REGDMN_MODE_108A)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_108A;

	if (wireless_modes_orig & REGDMN_MODE_XR)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_XR;

	if (wireless_modes_orig & REGDMN_MODE_11A_HALF_RATE)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11A_HALF_RATE;

	if (wireless_modes_orig & REGDMN_MODE_11A_QUARTER_RATE)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11A_QUARTER_RATE;

	if (wireless_modes_orig & REGDMN_MODE_11NG_HT20)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11NG_HT20;

	if (wireless_modes_orig & REGDMN_MODE_11NA_HT20)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11NA_HT20;

	if (wireless_modes_orig & REGDMN_MODE_11NG_HT40PLUS)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11NG_HT40PLUS;

	if (wireless_modes_orig & REGDMN_MODE_11NG_HT40MINUS)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11NG_HT40MINUS;

	if (wireless_modes_orig & REGDMN_MODE_11NA_HT40PLUS)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11NA_HT40PLUS;

	if (wireless_modes_orig & REGDMN_MODE_11NA_HT40MINUS)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11NA_HT40MINUS;

	if (wireless_modes_orig & REGDMN_MODE_11AC_VHT20)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT20;

	if (wireless_modes_orig & REGDMN_MODE_11AC_VHT40PLUS)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT40PLUS;

	if (wireless_modes_orig & REGDMN_MODE_11AC_VHT40MINUS)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT40MINUS;

	if (wireless_modes_orig & REGDMN_MODE_11AC_VHT80)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT80;

	if (wireless_modes_orig & REGDMN_MODE_11AC_VHT160)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT160;

	if (wireless_modes_orig & REGDMN_MODE_11AC_VHT80_80)
		cap->wireless_modes |= WMI_HOST_REGDMN_MODE_11AC_VHT80_80;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_host_mem_req_non_tlv() - Extract host memory request event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param num_entries: pointer to hold number of entries requested
 *
 * Return: Number of entries requested
 */
static host_mem_req *extract_host_mem_req_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint8_t *num_entries)
{
	wmi_service_ready_event *ev;

	ev = (wmi_service_ready_event *) evt_buf;

	*num_entries = ev->num_mem_reqs;
	return (host_mem_req *)ev->mem_reqs;
}

/**
 * save_fw_version_in_service_ready_non_tlv() - Save fw version in service
 * ready function
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS save_fw_version_in_service_ready_non_tlv(
		wmi_unified_t wmi_handle,
		void *evt_buf)
{
	/* Version check and exchange is not present in non-tlv implementation*/
	return QDF_STATUS_SUCCESS;
}

/**
 * ready_check_and_update_fw_version_non_tlv() - Ready and fw version check
 * function
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS ready_check_and_update_fw_version_non_tlv(
			wmi_unified_t wmi_handle,
			void *evt_buf)
{
	/* Version check and exchange is not present in non-tlv implementation*/
	return QDF_STATUS_SUCCESS;
}

/**
 * ready_extract_init_status_non_tlv() - Extract init status from ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 *
 * Return: ready status
 */
static uint32_t ready_extract_init_status_non_tlv(wmi_unified_t wmi_hdl,
						   void *evt_buf)
{
	 wmi_ready_event *ev = (wmi_ready_event *) evt_buf;
	 qdf_print("Version = %d %d  status = %d\n", ev->sw_version,
		 ev->abi_version, ev->status);
	 return ev->status;
}

/**
 * ready_extract_mac_addr_non_tlv() - extract mac address from ready event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param macaddr: Pointer to hold MAC address
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS ready_extract_mac_addr_non_tlv(wmi_unified_t wmi_hdl,
			void *evt_buf,
			uint8_t *macaddr)
{
	 wmi_ready_event *ev = (wmi_ready_event *) evt_buf;

	 WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->mac_addr, macaddr);
	 return QDF_STATUS_SUCCESS;
}

/**
 * extract_ready_params_non_tlv() - Extract data from ready event apart from
 *                     status, macaddr and version.
 * @wmi_handle: Pointer to WMI handle.
 * @evt_buf: Pointer to Ready event buffer.
 * @ev_param: Pointer to host defined struct to copy the data from event.
 *
 * Return: QDF_STATUS_SUCCESS on success.
 */
static QDF_STATUS extract_ready_event_params_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, struct wmi_host_ready_ev_param *ev_param)
{
	wmi_ready_event *ev = (wmi_ready_event *) evt_buf;

	ev_param->status = ev->status;
	ev_param->num_dscp_table = ev->num_dscp_table;
	if (ev->agile_capability)
		ev_param->agile_capability = true;
	else
		ev_param->agile_capability = false;
	/* Following params not present in non-TLV target. Set Defaults */
	ev_param->num_extra_mac_addr = 0;
	ev_param->num_total_peer = 0;
	ev_param->num_extra_peer = 0;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_dbglog_data_len_non_tlv() - extract debuglog data length
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 *
 * Return: length
 */
static uint8_t *extract_dbglog_data_len_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		uint32_t *len)
{
	/*Len is already valid from event. No need to change it */
	 return evt_buf;
}

/**
 * extract_wds_addr_event_non_tlv() - extract wds address from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param wds_ev: Pointer to hold wds address
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_wds_addr_event_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		uint16_t len, wds_addr_event_t *wds_ev)
{
	wmi_wds_addr_event_t *ev = (wmi_wds_addr_event_t *) evt_buf;
	int i;

#ifdef BIG_ENDIAN_HOST
	{
		uint8_t *datap = (uint8_t *) ev;
		/*Skip swapping the first long word*/
		datap += sizeof(uint32_t);
		for (i = 0; i < ((len / sizeof(uint32_t))-1);
				i++, datap += sizeof(uint32_t))
			*(uint32_t *)datap =
			    qdf_le32_to_cpu(*(uint32_t *)datap);
	}
#endif

	qdf_mem_copy(wds_ev->event_type, ev->event_type,
		sizeof(wds_ev->event_type));
	for (i = 0; i < 4; i++)	{
		wds_ev->peer_mac[i] =
			((u_int8_t *)&(ev->peer_mac.mac_addr31to0))[i];
		wds_ev->dest_mac[i] =
			((u_int8_t *)&(ev->dest_mac.mac_addr31to0))[i];
	}
	for (i = 0; i < 2; i++)	{
		wds_ev->peer_mac[4+i] =
			((u_int8_t *)&(ev->peer_mac.mac_addr47to32))[i];
		wds_ev->dest_mac[4+i] =
			((u_int8_t *)&(ev->dest_mac.mac_addr47to32))[i];
	}
	/* vdev_id is not available in legacy. It is required only to get
	 * pdev, hence setting it to zero as legacy as only one pdev.
	 */
	wds_ev->vdev_id = 0;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_dcs_interference_type_non_tlv() - extract dcs interference type
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold dcs interference param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_dcs_interference_type_non_tlv(
		wmi_unified_t wmi_handle,
		void *evt_buf, struct wmi_host_dcs_interference_param *param)
{
	wmi_dcs_interference_event_t *ev =
	    (wmi_dcs_interference_event_t *) evt_buf;

	param->interference_type = ev->interference_type;
	param->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;

	return QDF_STATUS_SUCCESS;
}

/*
 * extract_dcs_cw_int_non_tlv() - extract dcs cw interference from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param cw_int: Pointer to hold cw interference
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_dcs_cw_int_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_ath_dcs_cw_int *cw_int)
{
	wmi_dcs_interference_event_t *ev =
	    (wmi_dcs_interference_event_t *) evt_buf;

	qdf_mem_copy(cw_int, &ev->int_event.cw_int, sizeof(*cw_int));
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_dcs_im_tgt_stats_non_tlv() - extract dcs im target stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param wlan_stat: Pointer to hold wlan stats
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_dcs_im_tgt_stats_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_dcs_im_tgt_stats_t *wlan_stat)
{
	wmi_dcs_interference_event_t *ev =
		(wmi_dcs_interference_event_t *) evt_buf;

	qdf_mem_copy(wlan_stat, &ev->int_event.wlan_stat,
		sizeof(wmi_host_dcs_im_tgt_stats_t));
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_fips_event_data_non_tlv() - extract fips event data
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: pointer FIPS event params
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_fips_event_data_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		struct wmi_host_fips_event_param *param)
{
	wmi_pdev_fips_event *event = (wmi_pdev_fips_event *)evt_buf;

	param->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;
#ifdef BIG_ENDIAN_HOST
	{
		/*****************LE to BE conversion*************************/

		/* Assigning unaligned space to copy the data */
		unsigned char *data_unaligned = qdf_mem_malloc(
			(sizeof(u_int8_t)*event->data_len + FIPS_ALIGN));

		u_int8_t *data_aligned = NULL;
		int c;

		/* Checking if kmalloc does succesful allocation */
		if (data_unaligned == NULL)
			return QDF_STATUS_E_FAILURE;

		/* Checking if space is alligned */
		if (!FIPS_IS_ALIGNED(data_unaligned, FIPS_ALIGN)) {
			/* align the data space */
			data_aligned =
			   (u_int8_t *)FIPS_ALIGNTO(data_unaligned, FIPS_ALIGN);
		} else {
			data_aligned = (u_int8_t *)data_unaligned;
		}

		/* memset and copy content from data to data aligned */
		OS_MEMSET(data_aligned, 0, event->data_len);
		OS_MEMCPY(data_aligned, event->data, event->data_len);
		/* Endianness to LE */
		for (c = 0; c < event->data_len/4; c++) {
			*((u_int32_t *)data_aligned+c) =
			    qdf_le32_to_cpu(*((u_int32_t *)data_aligned+c));
		}

		/* Copy content to event->data */
		OS_MEMCPY(event->data, data_aligned, event->data_len);

		/* clean up allocated space */
		qdf_mem_free(data_unaligned);
		data_aligned = NULL;
		data_unaligned = NULL;

		/*************************************************************/
	}
#endif
	param->data = event->data;
	param->data_len = event->data_len;
	param->error_status = event->error_status;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_start_resp_non_tlv() - extract vdev start response
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_rsp: Pointer to hold vdev response
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_vdev_start_resp_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_vdev_start_resp *vdev_rsp)
{
	wmi_vdev_start_response_event *ev =
	    (wmi_vdev_start_response_event *) evt_buf;

	qdf_mem_zero(vdev_rsp, sizeof(*vdev_rsp));

	vdev_rsp->vdev_id = ev->vdev_id;
	vdev_rsp->requestor_id = ev->requestor_id;
	vdev_rsp->resp_type = ev->resp_type;
	vdev_rsp->status = ev->status;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_tbttoffset_num_vdevs_non_tlv() - extract tbtt offset num vdevs
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param num_vdev: Pointer to hold num vdev
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_tbttoffset_num_vdevs_non_tlv(void *wmi_hdl,
		void *evt_buf,
		uint32_t *num_vdevs)
{
	wmi_tbtt_offset_event *tbtt_offset_event =
		(wmi_tbtt_offset_event *)evt_buf;
	uint32_t vdev_map;

	vdev_map = tbtt_offset_event->vdev_map;
	*num_vdevs = wmi_vdev_map_to_num_vdevs(vdev_map);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_tbttoffset_update_params_non_tlv() - extract tbtt offset update param
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index refering to a vdev
 * @param tbtt_param: Pointer to tbttoffset event param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_tbttoffset_update_params_non_tlv(void *wmi_hdl,
		void *evt_buf, uint8_t idx,
		struct tbttoffset_params *tbtt_param)
{
	wmi_tbtt_offset_event *tbtt_offset_event =
		(wmi_tbtt_offset_event *)evt_buf;
	uint32_t vdev_map;

	vdev_map = tbtt_offset_event->vdev_map;

	tbtt_param->vdev_id = wmi_vdev_map_to_vdev_id(vdev_map, idx);
	if (tbtt_param->vdev_id == WLAN_INVALID_VDEV_ID)
		return QDF_STATUS_E_INVAL;
	tbtt_param->tbttoffset =
		tbtt_offset_event->tbttoffset_list[tbtt_param->vdev_id];

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_mgmt_rx_params_non_tlv() - extract management rx params from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param hdr: Pointer to hold header
 * @param bufp: Pointer to hold pointer to rx param buffer
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_mgmt_rx_params_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		struct mgmt_rx_event_params *hdr, uint8_t **bufp)
{
	wmi_mgmt_rx_event *ev = (wmi_mgmt_rx_event *)evt_buf;

	hdr->channel = ev->hdr.channel;
	hdr->snr = ev->hdr.snr;
	hdr->rssi = ev->hdr.snr;
	hdr->rate = ev->hdr.rate;
	hdr->phy_mode = ev->hdr.phy_mode;
	hdr->buf_len = ev->hdr.buf_len;
	hdr->status = ev->hdr.status;
	hdr->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;
	qdf_mem_copy(hdr->rssi_ctl, ev->hdr.rssi_ctl, sizeof(hdr->rssi_ctl));
	*bufp = ev->bufp;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_stopped_param_non_tlv() - extract vdev stop param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_id: Pointer to hold vdev identifier
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_vdev_stopped_param_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		uint32_t *vdev_id)
{
	wmi_vdev_stopped_event *event = (wmi_vdev_stopped_event *)evt_buf;

	*vdev_id = event->vdev_id;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_roam_param_non_tlv() - extract vdev roam param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold roam param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_vdev_roam_param_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_roam_event *param)
{
	wmi_roam_event *evt = (wmi_roam_event *)evt_buf;

	qdf_mem_zero(param, sizeof(*param));
	param->vdev_id = evt->vdev_id;
	param->reason = evt->reason;
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_scan_ev_param_non_tlv() - extract vdev scan param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold vdev scan param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_vdev_scan_ev_param_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
	struct scan_event *param)
{
	wmi_scan_event *evt = (wmi_scan_event *)evt_buf;

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
	case WMI_SCAN_EVENT_INVALID:
		param->type = SCAN_EVENT_TYPE_INVALID;
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
	case WMI_SCAN_REASON_MAX:
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

/**
 * extract_mu_ev_param_non_tlv() - extract mu param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold mu report
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_mu_ev_param_non_tlv(wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_mu_report_event *param)
{
	wmi_mu_report_event *event = (wmi_mu_report_event *)evt_buf;

	param->mu_request_id = event->mu_request_id;
	param->status_reason = event->status_reason;
	qdf_mem_copy(param->total_mu, event->total_mu, sizeof(param->total_mu));
	param->num_active_bssid = event->num_active_bssid;
	qdf_mem_copy(param->hidden_node_mu, event->hidden_node_mu,
				sizeof(param->hidden_node_mu));
	param->num_TA_entries = event->num_TA_entries;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_mu_db_entry_non_tlv() - extract mu db entry from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param profile_data: Pointer to hold mu_db_entry
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_mu_db_entry_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint8_t idx,
		wmi_host_mu_db_entry *db_entry)
{
	wmi_mu_report_event *event = (wmi_mu_report_event *)evt_buf;

	if (idx > event->num_TA_entries)
		return QDF_STATUS_E_INVAL;

	qdf_mem_copy(db_entry, &event->mu_entry[idx],
		sizeof(wmi_host_mu_db_entry));

	return QDF_STATUS_SUCCESS;
}


/**
 * extract_mumimo_tx_count_ev_param_non_tlv() - extract mumimo tx count from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold mu report
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_mumimo_tx_count_ev_param_non_tlv(wmi_unified_t wmi_handle,
			void *evt_buf, wmi_host_peer_txmu_cnt_event *param)
{
	wmi_peer_txmu_cnt_event *event = (wmi_peer_txmu_cnt_event *)evt_buf;

	param->tx_mu_transmitted = event->tx_mu_transmitted;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_gid_userpos_list_ev_param_non_tlv() - extract gid user position
 * 						      from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold peer user position list
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_peer_gid_userpos_list_ev_param_non_tlv(
			wmi_unified_t wmi_handle,
			void *evt_buf,
			wmi_host_peer_gid_userpos_list_event *param)
{
	wmi_peer_gid_userpos_list_event *event =
			(wmi_peer_gid_userpos_list_event *)evt_buf;

	qdf_mem_copy(param->usr_list, event->usr_list, sizeof(param->usr_list));

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_caldata_version_check_ev_param_non_tlv() - extract caldata from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold peer caldata version data
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_pdev_caldata_version_check_ev_param_non_tlv(
			wmi_unified_t wmi_handle,
			void *evt_buf,
			wmi_host_pdev_check_cal_version_event *param)
{
	wmi_pdev_check_cal_version_event *event =
			(wmi_pdev_check_cal_version_event *)evt_buf;

	param->software_cal_version = event->software_cal_version;
	param->board_cal_version = event->board_cal_version;
	param->cal_ok = event->cal_ok;

	if (event->board_mcn_detail[WMI_BOARD_MCN_STRING_MAX_SIZE] != '\0')
		event->board_mcn_detail[WMI_BOARD_MCN_STRING_MAX_SIZE] = '\0';

	WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(param->board_mcn_detail,
		event->board_mcn_detail, WMI_BOARD_MCN_STRING_BUF_SIZE);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_tpc_config_ev_param_non_tlv() - extract pdev tpc configuration
 * param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold tpc configuration
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_pdev_tpc_config_ev_param_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_pdev_tpc_config_event *param)
{
	wmi_pdev_tpc_config_event *event = (wmi_pdev_tpc_config_event *)evt_buf;

	param->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;
	param->regDomain = event->regDomain;
	param->chanFreq = event->chanFreq;
	param->phyMode = event->phyMode;
	param->twiceAntennaReduction = event->twiceAntennaReduction;
	param->twiceMaxRDPower = event->twiceMaxRDPower;
	param->powerLimit = event->powerLimit;
	param->rateMax = event->rateMax;
	param->numTxChain = event->numTxChain;
	param->ctl = event->ctl;
	param->flags = event->flags;

	qdf_mem_copy(param->maxRegAllowedPower, event->maxRegAllowedPower,
		sizeof(param->maxRegAllowedPower));
	qdf_mem_copy(param->maxRegAllowedPowerAGCDD,
		event->maxRegAllowedPowerAGCDD,
		sizeof(param->maxRegAllowedPowerAGCDD));
	qdf_mem_copy(param->maxRegAllowedPowerAGSTBC,
		event->maxRegAllowedPowerAGSTBC,
		sizeof(param->maxRegAllowedPowerAGSTBC));
	qdf_mem_copy(param->maxRegAllowedPowerAGTXBF,
		event->maxRegAllowedPowerAGTXBF,
		sizeof(param->maxRegAllowedPowerAGTXBF));
	qdf_mem_copy(param->ratesArray, event->ratesArray,
		sizeof(param->ratesArray));

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_nfcal_power_ev_param_non_tlv() - extract noise floor calibration
 * power param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold nf cal power param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_nfcal_power_ev_param_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_pdev_nfcal_power_all_channels_event *param)
{
	wmi_pdev_nfcal_power_all_channels_event *event =
	    (wmi_pdev_nfcal_power_all_channels_event *)evt_buf;

	if ((sizeof(event->nfdBr) == sizeof(param->nfdbr)) &&
	    (sizeof(event->nfdBm) == sizeof(param->nfdbm)) &&
	    (sizeof(event->freqNum) == sizeof(param->freqnum))) {
		qdf_mem_copy(param->nfdbr, event->nfdBr, sizeof(param->nfdbr));
		qdf_mem_copy(param->nfdbm, event->nfdBm, sizeof(param->nfdbm));
		qdf_mem_copy(param->freqnum, event->freqNum,
			     sizeof(param->freqnum));
	} else {
		WMI_LOGE("%s: %d Failed copy out of bound memory!\n", __func__, __LINE__);
		return QDF_STATUS_E_RESOURCES;
	}

	param->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_tpc_ev_param_non_tlv() - extract tpc param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold tpc param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_pdev_tpc_ev_param_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_pdev_tpc_event *param)
{
	wmi_pdev_tpc_event *event = (wmi_pdev_tpc_event *)evt_buf;

	qdf_mem_copy(param->tpc, event->tpc, sizeof(param->tpc));
	param->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_generic_buffer_ev_param_non_tlv() - extract pdev generic buffer
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to generic buffer param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_pdev_generic_buffer_ev_param_non_tlv(
		wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_pdev_generic_buffer_event *param)
{
	wmi_pdev_generic_buffer_event *event =
	    (wmi_pdev_generic_buffer_event *)evt_buf;

	param->buf_type = event->buf_type;
	param->frag_id = event->frag_id;
	param->more_frag = event->more_frag;
	param->buf_len = event->buf_len;

	qdf_mem_copy(param->buf_info, event->buf_info, event->buf_len);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_gpio_input_ev_param_non_tlv() - extract gpio input param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param gpio_num: Pointer to hold gpio number
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_gpio_input_ev_param_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint32_t *gpio_num)
{
	wmi_gpio_input_event *ev = (wmi_gpio_input_event *) evt_buf;

	*gpio_num = ev->gpio_num;
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_reserve_ast_ev_param_non_tlv() - extract reserve ast entry
 * param from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param result: Pointer to hold reserve ast entry param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_pdev_reserve_ast_ev_param_non_tlv(
		wmi_unified_t wmi_handle,
		void *evt_buf, struct wmi_host_proxy_ast_reserve_param *param)
{
	wmi_pdev_reserve_ast_entry_event *ev =
	    (wmi_pdev_reserve_ast_entry_event *) evt_buf;

	param->result = ev->result;
	param->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_swba_num_vdevs_non_tlv() - extract swba num vdevs from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param num_vdevs: Pointer to hold num vdevs
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_swba_num_vdevs_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		uint32_t *num_vdevs)
{
	wmi_host_swba_event *swba_event = (wmi_host_swba_event *)evt_buf;
	uint32_t vdev_map;

	vdev_map = swba_event->vdev_map;
	*num_vdevs = wmi_vdev_map_to_num_vdevs(vdev_map);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_swba_tim_info_non_tlv() - extract swba tim info from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index to bcn info
 * @param tim_info: Pointer to hold tim info
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_swba_tim_info_non_tlv(wmi_unified_t wmi_handle,
			void *evt_buf,
			uint32_t idx, wmi_host_tim_info *tim_info)
{
	wmi_host_swba_event *swba_event = (wmi_host_swba_event *)evt_buf;
	wmi_bcn_info *bcn_info;
	uint32_t vdev_map;

	bcn_info = &swba_event->bcn_info[idx];
	vdev_map = swba_event->vdev_map;

	tim_info->vdev_id = wmi_vdev_map_to_vdev_id(vdev_map, idx);
	if (tim_info->vdev_id == WLAN_INVALID_VDEV_ID)
		return QDF_STATUS_E_INVAL;
	tim_info->tim_len = bcn_info->tim_info.tim_len;
	tim_info->tim_mcast = bcn_info->tim_info.tim_mcast;
	qdf_mem_copy(tim_info->tim_bitmap, bcn_info->tim_info.tim_bitmap,
			(sizeof(uint32_t) * WMI_TIM_BITMAP_ARRAY_SIZE));
	tim_info->tim_changed = bcn_info->tim_info.tim_changed;
	tim_info->tim_num_ps_pending = bcn_info->tim_info.tim_num_ps_pending;
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_swba_noa_info_non_tlv() - extract swba NoA information from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index to bcn info
 * @param p2p_desc: Pointer to hold p2p NoA info
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_swba_noa_info_non_tlv(wmi_unified_t wmi_handle,
			void *evt_buf,
			uint32_t idx, wmi_host_p2p_noa_info *p2p_desc)
{
	wmi_host_swba_event *swba_event = (wmi_host_swba_event *)evt_buf;
	wmi_p2p_noa_info *p2p_noa_info;
	wmi_bcn_info *bcn_info;
	uint8_t i = 0;
	uint32_t vdev_map;

	bcn_info = &swba_event->bcn_info[idx];
	vdev_map = swba_event->vdev_map;
	p2p_noa_info = &bcn_info->p2p_noa_info;

	p2p_desc->vdev_id = wmi_vdev_map_to_vdev_id(vdev_map, idx);
	if (p2p_desc->vdev_id == WLAN_INVALID_VDEV_ID)
		return QDF_STATUS_E_INVAL;
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
		(uint8_t) WMI_UNIFIED_NOA_ATTR_NUM_DESC_GET(p2p_noa_info);

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
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_sta_ps_statechange_ev_non_tlv() - extract peer sta ps state
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold peer param and ps state
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_peer_sta_ps_statechange_ev_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, wmi_host_peer_sta_ps_statechange_event *ev)
{
	wmi_peer_sta_ps_statechange_event *event =
	    (wmi_peer_sta_ps_statechange_event *)evt_buf;

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&event->peer_macaddr, ev->peer_macaddr);
	ev->peer_ps_state = event->peer_ps_state;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_sta_kickout_ev_non_tlv() - extract peer sta kickout event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold peer param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_peer_sta_kickout_ev_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_peer_sta_kickout_event *ev)
{
	wmi_peer_sta_kickout_event *kickout_event =
	    (wmi_peer_sta_kickout_event *)evt_buf;

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&kickout_event->peer_macaddr,
		ev->peer_macaddr);

	/**Following not available in legacy wmi*/
	ev->reason = 0;
	ev->rssi = 0;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_ratecode_list_ev_non_tlv() - extract peer ratecode from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param peer_mac: Pointer to hold peer mac address
 * @param rate_cap: Pointer to hold ratecode
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_peer_ratecode_list_ev_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		uint8_t *peer_mac, wmi_sa_rate_cap *rate_cap)
{
	wmi_peer_ratecode_list_event_t *rate_event =
	    (wmi_peer_ratecode_list_event_t *)evt_buf;
	int i, htindex, j;
	uint8_t shift = 0;

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&rate_event->peer_macaddr, peer_mac);

	htindex = 0;
	rate_cap->ratecount[0] =
	    ((rate_event->peer_rate_info.ratecount) & SA_MASK_BYTE);
	rate_cap->ratecount[1] =
	    ((rate_event->peer_rate_info.ratecount >> 8) & SA_MASK_BYTE);
	rate_cap->ratecount[2] =
	    ((rate_event->peer_rate_info.ratecount >> 16) & SA_MASK_BYTE);
	rate_cap->ratecount[3] =
	    ((rate_event->peer_rate_info.ratecount >> 24) & SA_MASK_BYTE);

	if (rate_cap->ratecount[0]) {
		for (i = 0; i < SA_MAX_LEGACY_RATE_DWORDS; i++) {
			for (j = 0; j < SA_BYTES_IN_DWORD; j++) {
				rate_cap->ratecode_legacy[htindex] =
			 ((rate_event->peer_rate_info.ratecode_legacy[i]
			   >> (8*j)) & SA_MASK_BYTE);
				htindex++;
			}
		}
	}

	htindex = 0;
	for (i = 0; i < SA_MAX_HT_RATE_DWORDS; i++) {
		for (j = 0; j < SA_BYTES_IN_DWORD; j++) {
			shift = (8*j);
			rate_cap->ratecode_20[htindex] =
			   ((rate_event->peer_rate_info.ratecode_20[i]
			     >> (shift)) & SA_MASK_BYTE);
			rate_cap->ratecode_40[htindex] =
			   ((rate_event->peer_rate_info.ratecode_40[i]
			     >> (shift)) & SA_MASK_BYTE);
			rate_cap->ratecode_80[htindex] =
			    ((rate_event->peer_rate_info.ratecode_80[i]
			      >> (shift)) & SA_MASK_BYTE);
			htindex++;
		}
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_rtt_header_internal_non_tlv() - extract internal rtt header from
 * event
 * @param ev: pointer to internal rtt event header
 * @param hdr: Pointer to received rtt event header
 *
 * Return: None
 */
static void extract_rtt_header_internal_non_tlv(wmi_host_rtt_event_hdr *ev,
		wmi_rtt_event_hdr *hdr)
{
	ev->req_id = WMI_RTT_REQ_ID_GET(hdr->req_id);
	ev->result = (hdr->req_id & 0xffff0000) >> 16;
	ev->meas_type = WMI_RTT_REPORT_MEAS_TYPE_GET(hdr->req_id);
	ev->report_type = WMI_RTT_REPORT_REPORT_TYPE_GET(hdr->req_id);
	ev->v3_status = WMI_RTT_REPORT_V3_STATUS_GET(hdr->req_id);
	ev->v3_finish = WMI_RTT_REPORT_V3_FINISH_GET(hdr->req_id);
	ev->v3_tm_start = WMI_RTT_REPORT_V3_TM_START_GET(hdr->req_id);
	ev->num_ap = WMI_RTT_REPORT_NUM_AP_GET(hdr->req_id);

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&hdr->dest_mac, ev->dest_mac);
}

/**
 * extract_rtt_error_report_ev_non_tlv() - extract rtt error report from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param wds_ev: Pointer to hold rtt error report
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_rtt_error_report_ev_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_rtt_error_report_event *ev)
{
	wmi_rtt_error_report_event *error_report =
	    (wmi_rtt_error_report_event *) evt_buf;

	extract_rtt_header_internal_non_tlv(&ev->hdr, &error_report->header);
	ev->reject_reason = error_report->reject_reason;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_rtt_hdr_non_tlv() - extract rtt header from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold rtt header
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_rtt_hdr_non_tlv(wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_rtt_event_hdr *ev)
{
	wmi_rtt_event_hdr *hdr = (wmi_rtt_event_hdr *) evt_buf;

	extract_rtt_header_internal_non_tlv(ev, hdr);

	return QDF_STATUS_SUCCESS;
}

/**
 * copy_rtt_report_cfr
 * @ev: pointer to destination event pointer
 * @report_type: report type recieved in event
 * @p: pointer to event data
 * @hdump: pointer to destination buffer
 * @hdump_len: length of dest buffer
 *
 * Return: Pointer to current offset in p
 */
static uint8_t *copy_rtt_report_cfr(wmi_host_rtt_meas_event *ev,
		uint8_t report_type, uint8_t *p,
		uint8_t *hdump, int16_t hdump_len)
{
	uint8_t index, i;
	uint8_t *tmp, *temp1, *temp2;
#define TONE_LEGACY_20M 53
#define TONE_VHT_20M 56
#define TONE_VHT_40M 117
#define TONE_VHT_80M 242
	int tone_number[4] = {
		TONE_LEGACY_20M, TONE_VHT_20M, TONE_VHT_40M, TONE_VHT_80M};
#define MEM_ALIGN(x) ((((x)<<1)+3) & 0xFFFC)
	/* the buffer size of 1 chain for each BW 0-3 */
	u_int16_t bw_size[4] =	{
		MEM_ALIGN(TONE_LEGACY_20M),
		MEM_ALIGN(TONE_VHT_20M),
		MEM_ALIGN(TONE_VHT_40M),
		MEM_ALIGN(TONE_VHT_80M)
	};
	if (hdump == NULL) {
		qdf_print("Destination buffer is NULL\n");
		return p;
	}
	temp1 = temp2 = hdump;

	for (index = 0; index < 4; index++) {
		if (ev->chain_mask & (1 << index)) {
			if (index == 0)
				ev->rssi0 = *((u_int32_t *)p);
			if (index == 1)
				ev->rssi1 = *((u_int32_t *)p);
			if (index == 2)
				ev->rssi2 = *((u_int32_t *)p);
			if (index == 3)
				ev->rssi3 = *((u_int32_t *)p);

			p += sizeof(u_int32_t);
			if (report_type == WMI_RTT_REPORT_CFR) {
				tmp = p + bw_size[ev->bw];
				ev->txrxchain_mask = tone_number[ev->bw];
				temp2 = temp2 + bw_size[ev->bw];
				for (i = 0; (i < tone_number[ev->bw]); i++) {
					qdf_mem_copy(temp1, p, 2);
					temp1 += 2;
					p += 2;
					hdump_len -= 2;
					if (hdump_len <= 0)
						break;
				}
				temp1 = temp2;
				p = tmp;
			}
		}
	}
	return p;
}

/**
 * extract_rtt_ev_non_tlv() - extract rtt event
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param ev: Pointer to hold rtt event
 * @param hdump: Pointer to hold hex dump
 * @param hdump_len: hex dump length
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_rtt_ev_non_tlv(wmi_unified_t wmi_handle, void *evt_buf,
		wmi_host_rtt_meas_event *ev, uint8_t *hdump, uint16_t h_len)
{
	wmi_rtt_meas_event *body = (wmi_rtt_meas_event *) evt_buf;
	uint8_t meas_type, report_type;
	uint8_t *p;
	int16_t hdump_len = h_len;

	A_TIME64 *time;

	if (body) {
		meas_type = WMI_RTT_REPORT_MEAS_TYPE_GET(body->header.req_id);
		report_type =
		    WMI_RTT_REPORT_REPORT_TYPE_GET(body->header.req_id);

		ev->chain_mask = WMI_RTT_REPORT_RX_CHAIN_GET(body->rx_chain);
		ev->bw = WMI_RTT_REPORT_RX_BW_GET(body->rx_chain);
		/* If report type is not WMI_RTT_REPORT_CFR */
		ev->txrxchain_mask = 0;

		ev->tod = ((u_int64_t) body->tod.time32) << 32;
		ev->tod |= body->tod.time0; /*tmp1 is the 64 bit tod*/
		ev->toa = ((u_int64_t) body->toa.time32) << 32;
		ev->toa |= body->toa.time0;

		p = (u_int8_t *) (++body);

		/* if the measurement is TMR, we should have T3 and T4 */
		if (meas_type == RTT_MEAS_FRAME_TMR) {
			time = (A_TIME64 *) body;
			ev->t3 = (u_int64_t) (time->time32) << 32;
			ev->t3 |= time->time0;

			time++;
			ev->t4 = (u_int64_t)(time->time32) << 32;
			ev->t4 |= time->time0;

			p = (u_int8_t *) (++time);
		} else {
			ev->t3 = 0;
			ev->t4 = 0;
		}

		ev->rssi0 = 0;
		ev->rssi1 = 0;
		ev->rssi2 = 0;
		ev->rssi3 = 0;
		p = copy_rtt_report_cfr(ev, report_type, p, hdump, hdump_len);
	} else {
		qdf_print("Error!body is NULL\n");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_thermal_stats_non_tlv() - extract thermal stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: Pointer to event buffer
 * @param temp: Pointer to hold extracted temperature
 * @param level: Pointer to hold extracted level
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_thermal_stats_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		uint32_t *temp, uint32_t *level, uint32_t *pdev_id)
{
	tt_stats_t *tt_stats_event = NULL;

	tt_stats_event = (tt_stats_t *) evt_buf;

	*pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;
	*temp = tt_stats_event->temp;
	*level = tt_stats_event->level;
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_thermal_level_stats_non_tlv() - extract thermal level stats from
 * event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index to level stats
 * @param levelcount: Pointer to hold levelcount
 * @param dccount: Pointer to hold dccount
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_thermal_level_stats_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		uint8_t idx, uint32_t *levelcount, uint32_t *dccount)
{
	tt_stats_t *tt_stats_event = NULL;

	tt_stats_event = (tt_stats_t *) evt_buf;

	if (idx < TT_LEVELS) {
		*levelcount = tt_stats_event->levstats[idx].levelcount;
		*dccount = tt_stats_event->levstats[idx].dccount;
		return QDF_STATUS_SUCCESS;
	}
	return QDF_STATUS_E_FAILURE;
}

/**
 * extract_comb_phyerr_non_tlv() - extract comb phy error from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param datalen: data length of event buffer
 * @param buf_offset: Pointer to hold value of current event buffer offset
 * post extraction
 * @param phyer: Pointer to hold phyerr
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_comb_phyerr_non_tlv(wmi_unified_t wmi_handle, void *evt_buf,
		uint16_t datalen, uint16_t *buf_offset,
		wmi_host_phyerr_t *phyerr)
{
	wmi_comb_phyerr_rx_event *pe;
#if ATH_PHYERR_DEBUG
	int i;
#endif /* ATH_PHYERR_DEBUG */
	uint8_t *data;

	data = (uint8_t *) evt_buf;

#if ATH_PHYERR_DEBUG
	qdf_print("%s: data=%pK, datalen=%d\n", __func__, data, datalen);
	/* XXX for now */

	for (i = 0; i < datalen; i++) {
		qdf_print("%02X ", data[i]);
		if (i % 32 == 31)
				qdf_print("\n");
	}
	qdf_print("\n");
#endif /* ATH_PHYERR_DEBUG */

	/* Ensure it's at least the size of the header */
	if (datalen < sizeof(*pe)) {
		return QDF_STATUS_E_FAILURE;
		/* XXX what should errors be? */
	}

	pe = (wmi_comb_phyerr_rx_event *) data;
#if ATH_PHYERR_DEBUG
	qdf_print("%s: pe->hdr.num_phyerr_events=%d\n",
	   __func__,
	   pe->hdr.num_phyerr_events);
#endif /* ATH_PHYERR_DEBUG */

	/*
	 * Reconstruct the 64 bit event TSF.  This isn't from the MAC, it's
	 * at the time the event was sent to us, the TSF value will be
	 * in the future.
	 */
	phyerr->tsf64 = pe->hdr.tsf_l32;
	phyerr->tsf64 |= (((uint64_t) pe->hdr.tsf_u32) << 32);

	*buf_offset = sizeof(pe->hdr);
	phyerr->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_single_phyerr_non_tlv() - extract single phy error from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param datalen: data length of event buffer
 * @param buf_offset: Pointer to hold value of current event buffer offset
 * post extraction
 * @param phyerr: Pointer to hold phyerr
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_single_phyerr_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		uint16_t datalen, uint16_t *buf_offset,
		wmi_host_phyerr_t *phyerr)
{
	wmi_single_phyerr_rx_event *ev;
#if ATH_PHYERR_DEBUG
	int i;
#endif /* ATH_PHYERR_DEBUG */
	int n = 0;
	uint8_t *data;

	phyerr->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;

	n = (int) *buf_offset;
	data = (uint8_t *) evt_buf;

	/* Loop over the bufp, extracting out phyerrors */
	/*
	 * XXX wmi_unified_comb_phyerr_rx_event.bufp is a char pointer,
	 * which isn't correct here - what we have received here
	 * is an array of TLV-style PHY errors.
	 */
	if (n < datalen) {
		/* ensure there's at least space for the header */
		if ((datalen - n) < sizeof(ev->hdr)) {
			qdf_print(
			"%s: not enough space? (datalen=%d, n=%d, hdr=%zd bytes\n",
				  __func__,
				  datalen,
				  n,
				  sizeof(ev->hdr));
			return QDF_STATUS_SUCCESS;
		}

		/*
		 * Obtain a pointer to the beginning of the current event.
		 * data[0] is the beginning of the WMI payload.
		 */
		 ev = (wmi_single_phyerr_rx_event *) &data[n];

		/*
		 * Sanity check the buffer length of the event against
		 * what we currently have.
		 *
		 * Since buf_len is 32 bits, we check if it overflows
		 * a large 32 bit value.  It's not 0x7fffffff because
		 * we increase n by (buf_len + sizeof(hdr)), which would
		 * in itself cause n to overflow.
		 *
		 * If "int" is 64 bits then this becomes a moot point.
		 */
		 if (ev->hdr.buf_len > PHYERROR_MAX_BUFFER_LENGTH) {
			qdf_print("%s: buf_len is garbage? (0x%x\n)\n",
				__func__,
				ev->hdr.buf_len);
			return QDF_STATUS_SUCCESS;
		 }
		 if (n + ev->hdr.buf_len > datalen) {
			qdf_print("%s: buf_len exceeds available space "
				"(n=%d, buf_len=%d, datalen=%d\n",
				__func__,
				n,
				ev->hdr.buf_len,
				datalen);
			return QDF_STATUS_SUCCESS;
		 }

		 phyerr->phy_err_code = WMI_UNIFIED_PHYERRCODE_GET(&ev->hdr);

#if ATH_PHYERR_DEBUG
		qdf_print("%s: len=%d, tsf=0x%08x, rssi = 0x%x/0x%x/0x%x/0x%x, "
				"comb rssi = 0x%x, phycode=%d\n",
				__func__,
				ev->hdr.buf_len,
				ev->hdr.tsf_timestamp,
				ev->hdr.rssi_chain0,
				ev->hdr.rssi_chain1,
				ev->hdr.rssi_chain2,
				ev->hdr.rssi_chain3,
				WMI_UNIFIED_RSSI_COMB_GET(&ev->hdr),
					  phyerr->phy_err_code);

		/*
		 * For now, unroll this loop - the chain 'value' field isn't
		 * a variable but glued together into a macro field definition.
		 * Grr. :-)
		 */
		qdf_print(
		"%s: chain 0: raw=0x%08x; pri20=%d sec20=%d sec40=%d sec80=%d\n",
				__func__,
				ev->hdr.rssi_chain0,
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 0, PRI20),
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 0, SEC20),
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 0, SEC40),
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 0, SEC80));

		qdf_print(
		"%s: chain 1: raw=0x%08x: pri20=%d sec20=%d sec40=%d sec80=%d\n",
				__func__,
				ev->hdr.rssi_chain1,
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 1, PRI20),
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 1, SEC20),
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 1, SEC40),
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 1, SEC80));

		qdf_print(
		"%s: chain 2: raw=0x%08x: pri20=%d sec20=%d sec40=%d sec80=%d\n",
				__func__,
				ev->hdr.rssi_chain2,
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 2, PRI20),
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 2, SEC20),
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 2, SEC40),
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 2, SEC80));

		qdf_print(
		"%s: chain 3: raw=0x%08x: pri20=%d sec20=%d sec40=%d sec80=%d\n",
				__func__,
				ev->hdr.rssi_chain3,
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 3, PRI20),
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 3, SEC20),
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 3, SEC40),
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 3, SEC80));


		qdf_print(
			"%s: freq_info_1=0x%08x, freq_info_2=0x%08x\n",
			   __func__, ev->hdr.freq_info_1, ev->hdr.freq_info_2);

	   /*
		* The NF chain values are signed and are negative - hence
		* the cast evilness.
		*/
		qdf_print(
			"%s: nfval[1]=0x%08x, nfval[2]=0x%08x, nf=%d/%d/%d/%d, "
				"freq1=%d, freq2=%d, cw=%d\n",
				__func__,
				ev->hdr.nf_list_1,
				ev->hdr.nf_list_2,
				(int) WMI_UNIFIED_NF_CHAIN_GET(&ev->hdr, 0),
				(int) WMI_UNIFIED_NF_CHAIN_GET(&ev->hdr, 1),
				(int) WMI_UNIFIED_NF_CHAIN_GET(&ev->hdr, 2),
				(int) WMI_UNIFIED_NF_CHAIN_GET(&ev->hdr, 3),
				WMI_UNIFIED_FREQ_INFO_GET(&ev->hdr, 1),
				WMI_UNIFIED_FREQ_INFO_GET(&ev->hdr, 2),
				WMI_UNIFIED_CHWIDTH_GET(&ev->hdr));
#endif /* ATH_PHYERR_DEBUG */

#if ATH_SUPPORT_DFS
		/*
		 * If required, pass radar events to the dfs pattern matching
		 * code.
		 *
		 * Don't pass radar events with no buffer payload.
		 */
		phyerr->tsf_timestamp = ev->hdr.tsf_timestamp;
		phyerr->bufp = &ev->bufp[0];
		phyerr->buf_len = ev->hdr.buf_len;
#endif /* ATH_SUPPORT_DFS */

		/* populate the rf info */
		phyerr->rf_info.rssi_comb =
			WMI_UNIFIED_RSSI_COMB_GET(&ev->hdr);

#ifdef WLAN_CONV_SPECTRAL_ENABLE

	   /*
		* If required, pass spectral events to the spectral module
		*
		*/
		if (phyerr->phy_err_code == WMI_HOST_PHY_ERROR_FALSE_RADAR_EXT
		|| phyerr->phy_err_code == WMI_HOST_PHY_ERROR_SPECTRAL_SCAN) {
			if (ev->hdr.buf_len > 0) {

				/* Initialize the NF values to Zero. */
				phyerr->rf_info.noise_floor[0] =
				    WMI_UNIFIED_NF_CHAIN_GET(&ev->hdr, 0);
				phyerr->rf_info.noise_floor[1] =
				    WMI_UNIFIED_NF_CHAIN_GET(&ev->hdr, 1);
				phyerr->rf_info.noise_floor[2] =
				    WMI_UNIFIED_NF_CHAIN_GET(&ev->hdr, 2);
				phyerr->rf_info.noise_floor[3] =
				    WMI_UNIFIED_NF_CHAIN_GET(&ev->hdr, 3);

				/* Need to unroll loop due to macro
				 * constraints
				 * chain 0 */
				phyerr->rf_info.pc_rssi_info[0].rssi_pri20 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 0, PRI20);
				phyerr->rf_info.pc_rssi_info[0].rssi_sec20 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 0, SEC20);
				phyerr->rf_info.pc_rssi_info[0].rssi_sec40 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 0, SEC40);
				phyerr->rf_info.pc_rssi_info[0].rssi_sec80 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 0, SEC80);

				/* chain 1 */
				phyerr->rf_info.pc_rssi_info[1].rssi_pri20 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 1, PRI20);
				phyerr->rf_info.pc_rssi_info[1].rssi_sec20 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 1, SEC20);
				phyerr->rf_info.pc_rssi_info[1].rssi_sec40 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 1, SEC40);
				phyerr->rf_info.pc_rssi_info[1].rssi_sec80 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 1, SEC80);

				/* chain 2 */
				phyerr->rf_info.pc_rssi_info[2].rssi_pri20 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 2, PRI20);
				phyerr->rf_info.pc_rssi_info[2].rssi_sec20 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 2, SEC20);
				phyerr->rf_info.pc_rssi_info[2].rssi_sec40 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 2, SEC40);
				phyerr->rf_info.pc_rssi_info[2].rssi_sec80 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 2, SEC80);

				/* chain 3 */
				phyerr->rf_info.pc_rssi_info[3].rssi_pri20 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 3, PRI20);
				phyerr->rf_info.pc_rssi_info[3].rssi_sec20 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 3, SEC20);
				phyerr->rf_info.pc_rssi_info[3].rssi_sec40 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 3, SEC40);
				phyerr->rf_info.pc_rssi_info[3].rssi_sec80 =
				WMI_UNIFIED_RSSI_CHAN_GET(&ev->hdr, 3, SEC80);

				phyerr->chan_info.center_freq1 =
				    WMI_UNIFIED_FREQ_INFO_GET(&ev->hdr, 1);
				phyerr->chan_info.center_freq2 =
				    WMI_UNIFIED_FREQ_INFO_GET(&ev->hdr, 2);

			}
		}
#endif  /* WLAN_CONV_SPECTRAL_ENABLE */

		/*
		 * Advance the buffer pointer to the next PHY error.
		 * buflen is the length of this payload, so we need to
		 * advance past the current header _AND_ the payload.
		 */
		 n += sizeof(*ev) + ev->hdr.buf_len;
	}
	*buf_offset = n;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_composite_phyerr_non_tlv() - extract composite phy error from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param datalen: Length of event buffer
 * @param phyerr: Pointer to hold phy error
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_composite_phyerr_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		uint16_t datalen, wmi_host_phyerr_t *phyerr)
{
	wmi_composite_phyerr_rx_event *pe;
	wmi_composite_phyerr_rx_hdr *ph;

	/* Ensure it's at least the size of the header */
	if (datalen < sizeof(*pe)) {
		return QDF_STATUS_E_FAILURE;
		/* XXX what should errors be? */
	}

	phyerr->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;

	pe = (wmi_composite_phyerr_rx_event *) evt_buf;
	ph = &pe->hdr;

	/*
	 * Reconstruct the 64 bit event TSF.  This isn't from the MAC, it's
	 * at the time the event was sent to us, the TSF value will be
	 * in the future.
	 */
	phyerr->tsf64 = ph->tsf_l32;
	phyerr->tsf64 |= (((uint64_t) ph->tsf_u32) << 32);

	phyerr->tsf_timestamp = ph->tsf_timestamp;
	phyerr->bufp = &pe->bufp[0];
	phyerr->buf_len = ph->buf_len;

	phyerr->phy_err_mask0 = ph->phy_err_mask0;
	phyerr->phy_err_mask1 = ph->phy_err_mask1;

	phyerr->rf_info.rssi_comb =
	    WMI_UNIFIED_RSSI_COMB_GET(ph);

	/* Handle Spectral PHY Error */
	if ((ph->phy_err_mask0 & WMI_HOST_AR900B_SPECTRAL_PHYERR_MASK)) {
#ifdef WLAN_CONV_SPECTRAL_ENABLE
		if (ph->buf_len > 0) {

			/* Initialize the NF values to Zero. */
			phyerr->rf_info.noise_floor[0] =
			    WMI_UNIFIED_NF_CHAIN_GET(ph, 0);
			phyerr->rf_info.noise_floor[1] =
			    WMI_UNIFIED_NF_CHAIN_GET(ph, 1);
			phyerr->rf_info.noise_floor[2] =
			    WMI_UNIFIED_NF_CHAIN_GET(ph, 2);
			phyerr->rf_info.noise_floor[3] =
			    WMI_UNIFIED_NF_CHAIN_GET(ph, 3);

			/* populate the rf info */
			/* Need to unroll loop due to macro constraints */
			/* chain 0 */
			phyerr->rf_info.pc_rssi_info[0].rssi_pri20 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 0, PRI20);
			phyerr->rf_info.pc_rssi_info[0].rssi_sec20 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 0, SEC20);
			phyerr->rf_info.pc_rssi_info[0].rssi_sec40 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 0, SEC40);
			phyerr->rf_info.pc_rssi_info[0].rssi_sec80 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 0, SEC80);

			/* chain 1 */
			phyerr->rf_info.pc_rssi_info[1].rssi_pri20 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 1, PRI20);
			phyerr->rf_info.pc_rssi_info[1].rssi_sec20 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 1, SEC20);
			phyerr->rf_info.pc_rssi_info[1].rssi_sec40 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 1, SEC40);
			phyerr->rf_info.pc_rssi_info[1].rssi_sec80 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 1, SEC80);

			/* chain 2 */
			phyerr->rf_info.pc_rssi_info[2].rssi_pri20 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 2, PRI20);
			phyerr->rf_info.pc_rssi_info[2].rssi_sec20 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 2, SEC20);
			phyerr->rf_info.pc_rssi_info[2].rssi_sec40 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 2, SEC40);
			phyerr->rf_info.pc_rssi_info[2].rssi_sec80 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 2, SEC80);

			/* chain 3 */
			phyerr->rf_info.pc_rssi_info[3].rssi_pri20 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 3, PRI20);
			phyerr->rf_info.pc_rssi_info[3].rssi_sec20 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 3, SEC20);
			phyerr->rf_info.pc_rssi_info[3].rssi_sec40 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 3, SEC40);
			phyerr->rf_info.pc_rssi_info[3].rssi_sec80 =
					WMI_UNIFIED_RSSI_CHAN_GET(ph, 3, SEC80);

			phyerr->chan_info.center_freq1 =
			    WMI_UNIFIED_FREQ_INFO_GET(ph, 1);
			phyerr->chan_info.center_freq2 =
			    WMI_UNIFIED_FREQ_INFO_GET(ph, 2);

		}
#endif  /* WLAN_CONV_SPECTRAL_ENABLE */

	}
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_all_stats_counts_non_tlv() - extract all stats count from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param stats_param: Pointer to hold stats count
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_all_stats_counts_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_stats_event *stats_param)
{
	wmi_stats_event *ev = (wmi_stats_event *) evt_buf;
	wmi_stats_id stats_id = ev->stats_id;
	wmi_host_stats_id nac_rssi_ev = 0;

	if (stats_id & WMI_REQUEST_NAC_RSSI_STAT) {
		nac_rssi_ev = WMI_HOST_REQUEST_NAC_RSSI;
		stats_id &= ~WMI_REQUEST_NAC_RSSI_STAT;
	}

	switch (stats_id) {
	case WMI_REQUEST_PEER_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_PEER_STAT;
		break;

	case WMI_REQUEST_AP_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_AP_STAT;
		break;

	case WMI_REQUEST_INST_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_INST_STAT;
		break;

	case WMI_REQUEST_PEER_EXTD_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_PEER_EXTD_STAT;
		break;

	case WMI_REQUEST_VDEV_EXTD_STAT:
		stats_param->stats_id |= WMI_HOST_REQUEST_VDEV_EXTD_STAT;
		break;

	case WMI_REQUEST_PDEV_EXT2_STAT:
		stats_param->stats_id |= nac_rssi_ev;
		break;
	default:
		stats_param->stats_id = 0;
		break;

	}

	stats_param->num_pdev_stats = ev->num_pdev_stats;
	stats_param->num_pdev_ext_stats = ev->num_pdev_ext_stats;
	stats_param->num_vdev_stats = ev->num_vdev_stats;
	stats_param->num_peer_stats = ev->num_peer_stats;
	stats_param->num_bcnflt_stats = ev->num_bcnflt_stats;
	stats_param->num_chan_stats = 0;
	stats_param->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_stats_non_tlv() - extract pdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into pdev stats
 * @param pdev_stats: Pointer to hold pdev stats
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_pdev_stats_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		 uint32_t index, wmi_host_pdev_stats *pdev_stats)
{
	if (index < ((wmi_stats_event *)evt_buf)->num_pdev_stats) {

		wmi_pdev_stats *ev =
		    (wmi_pdev_stats *) ((((wmi_stats_event *)evt_buf)->data) +
			    (index * sizeof(wmi_pdev_stats)));

		/* direct copy possible since wmi_host_pdev_stats is same as
		 *  wmi_pdev_stats for non-tlv */
		/* qdf_mem_copy(pdev_stats, ev, sizeof(wmi_pdev_stats));*/

		pdev_stats->chan_nf = ev->chan_nf;
		pdev_stats->tx_frame_count = ev->tx_frame_count;
		pdev_stats->rx_frame_count = ev->rx_frame_count;
		pdev_stats->rx_clear_count = ev->rx_clear_count;
		pdev_stats->cycle_count = ev->cycle_count;
		pdev_stats->phy_err_count = ev->phy_err_count;
		pdev_stats->chan_tx_pwr = ev->chan_tx_pwr;

#define tx_stats (pdev_stats->pdev_stats.tx)
#define ev_tx_stats (ev->pdev_stats.tx)

		/* Tx Stats */
		tx_stats.comp_queued = ev_tx_stats.comp_queued;
		tx_stats.comp_delivered = ev_tx_stats.comp_delivered;
		tx_stats.msdu_enqued = ev_tx_stats.msdu_enqued;
		tx_stats.mpdu_enqued = ev_tx_stats.mpdu_enqued;
		tx_stats.wmm_drop = ev_tx_stats.wmm_drop;
		tx_stats.local_enqued = ev_tx_stats.local_enqued;
		tx_stats.local_freed = ev_tx_stats.local_freed;
		tx_stats.hw_queued = ev_tx_stats.hw_queued;
		tx_stats.hw_reaped = ev_tx_stats.hw_reaped;
		tx_stats.underrun = ev_tx_stats.underrun;
		tx_stats.hw_paused = ev_tx_stats.hw_paused;
		tx_stats.tx_abort = ev_tx_stats.tx_abort;
		tx_stats.mpdus_requed = ev_tx_stats.mpdus_requed;
		tx_stats.tx_xretry = ev_tx_stats.tx_xretry;
		tx_stats.data_rc = ev_tx_stats.data_rc;
		tx_stats.self_triggers = ev_tx_stats.self_triggers;
		tx_stats.sw_retry_failure = ev_tx_stats.sw_retry_failure;
		tx_stats.illgl_rate_phy_err = ev_tx_stats.illgl_rate_phy_err;
		tx_stats.pdev_cont_xretry = ev_tx_stats.pdev_cont_xretry;
		tx_stats.pdev_tx_timeout = ev_tx_stats.pdev_tx_timeout;
		tx_stats.pdev_resets = ev_tx_stats.pdev_resets;
		tx_stats.stateless_tid_alloc_failure =
		    ev_tx_stats.stateless_tid_alloc_failure;
		tx_stats.phy_underrun = ev_tx_stats.phy_underrun;
		tx_stats.txop_ovf = ev_tx_stats.txop_ovf;
		tx_stats.seq_posted = ev_tx_stats.seq_posted;
		tx_stats.seq_failed_queueing = ev_tx_stats.seq_failed_queueing;
		tx_stats.seq_completed = ev_tx_stats.seq_completed;
		tx_stats.seq_restarted = ev_tx_stats.seq_restarted;
		tx_stats.mu_seq_posted = ev_tx_stats.mu_seq_posted;
		tx_stats.mpdus_sw_flush = ev_tx_stats.mpdus_sw_flush;
		tx_stats.mpdus_hw_filter = ev_tx_stats.mpdus_hw_filter;
		tx_stats.mpdus_truncated = ev_tx_stats.mpdus_truncated;
		tx_stats.mpdus_ack_failed = ev_tx_stats.mpdus_ack_failed;
		tx_stats.mpdus_expired = ev_tx_stats.mpdus_expired;
		/* Only NON-TLV */
		tx_stats.mc_drop = ev_tx_stats.mc_drop;
		/* Only TLV */
		tx_stats.tx_ko = 0;

#define rx_stats (pdev_stats->pdev_stats.rx)
#define ev_rx_stats (ev->pdev_stats.rx)

		/* Rx Stats */
		rx_stats.mid_ppdu_route_change =
		    ev_rx_stats.mid_ppdu_route_change;
		rx_stats.status_rcvd = ev_rx_stats.status_rcvd;
		rx_stats.r0_frags = ev_rx_stats.r0_frags;
		rx_stats.r1_frags = ev_rx_stats.r1_frags;
		rx_stats.r2_frags = ev_rx_stats.r2_frags;
		/* Only TLV */
		rx_stats.r3_frags = 0;
		rx_stats.htt_msdus = ev_rx_stats.htt_msdus;
		rx_stats.htt_mpdus = ev_rx_stats.htt_mpdus;
		rx_stats.loc_msdus = ev_rx_stats.loc_msdus;
		rx_stats.loc_mpdus = ev_rx_stats.loc_mpdus;
		rx_stats.oversize_amsdu = ev_rx_stats.oversize_amsdu;
		rx_stats.phy_errs = ev_rx_stats.phy_errs;
		rx_stats.phy_err_drop = ev_rx_stats.phy_err_drop;
		rx_stats.mpdu_errs = ev_rx_stats.mpdu_errs;
		rx_stats.pdev_rx_timeout = ev_rx_stats.pdev_rx_timeout;
		rx_stats.rx_ovfl_errs = ev_rx_stats.rx_ovfl_errs;

		/* mem stats */
		pdev_stats->pdev_stats.mem.iram_free_size =
			ev->pdev_stats.mem.iram_free_size;
		pdev_stats->pdev_stats.mem.dram_free_size =
			ev->pdev_stats.mem.dram_free_size;
		/* Only Non-TLV */
		pdev_stats->pdev_stats.mem.sram_free_size =
		    ev->pdev_stats.mem.sram_free_size;

		/* Peer stats */
		/* Only TLV */
		pdev_stats->pdev_stats.peer.dummy = 0;
		/* Only NON-TLV */
		pdev_stats->ackRcvBad = ev->ackRcvBad;
		pdev_stats->rtsBad = ev->rtsBad;
		pdev_stats->rtsGood = ev->rtsGood;
		pdev_stats->fcsBad = ev->fcsBad;
		pdev_stats->noBeacons = ev->noBeacons;
		pdev_stats->mib_int_count = ev->mib_int_count;

	}
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_ext_stats_non_tlv() - extract extended pdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended pdev stats
 * @param pdev_ext_stats: Pointer to hold extended pdev stats
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_pdev_ext_stats_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		 uint32_t index, wmi_host_pdev_ext_stats *pdev_ext_stats)
{
	if (index < ((wmi_stats_event *)evt_buf)->num_pdev_ext_stats) {
		wmi_pdev_ext_stats *ev =
		(wmi_pdev_ext_stats *) ((((wmi_stats_event *)evt_buf)->data) +
		((((wmi_stats_event *)evt_buf)->num_pdev_stats) *
						 sizeof(wmi_pdev_stats)) +
					(index * sizeof(wmi_pdev_ext_stats)));
		/* Copy content to event->data */
		OS_MEMCPY(pdev_ext_stats, ev, sizeof(wmi_pdev_ext_stats));

	}
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_stats_non_tlv() - extract vdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into vdev stats
 * @param vdev_stats: Pointer to hold vdev stats
 *
 * Return: 0 for success or error code
 */

static QDF_STATUS extract_vdev_stats_non_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_vdev_stats *vdev_stats)
{
	if (index < ((wmi_stats_event *)evt_buf)->num_vdev_stats) {
		wmi_vdev_stats *ev =
		(wmi_vdev_stats *) ((((wmi_stats_event *)evt_buf)->data) +
		((((wmi_stats_event *)evt_buf)->num_pdev_stats) *
						 sizeof(wmi_pdev_stats)) +
		((((wmi_stats_event *)evt_buf)->num_pdev_ext_stats) *
						 sizeof(wmi_pdev_ext_stats)) +
					 (index * sizeof(wmi_vdev_stats)));

		OS_MEMSET(vdev_stats, 0, sizeof(wmi_host_vdev_stats));
		vdev_stats->vdev_id = ev->vdev_id;

	}
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_stats_non_tlv() - extract peer stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into peer stats
 * @param peer_stats: Pointer to hold peer stats
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_peer_stats_non_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_peer_stats *peer_stats)
{
	if (index < ((wmi_stats_event *)evt_buf)->num_peer_stats) {
		wmi_peer_stats *ev =
		(wmi_peer_stats *) ((((wmi_stats_event *)evt_buf)->data) +
		((((wmi_stats_event *)evt_buf)->num_pdev_stats) *
						 sizeof(wmi_pdev_stats)) +
		((((wmi_stats_event *)evt_buf)->num_pdev_ext_stats) *
						 sizeof(wmi_pdev_ext_stats)) +
		((((wmi_stats_event *)evt_buf)->num_vdev_stats) *
						 sizeof(wmi_vdev_stats)) +
					  (index * sizeof(wmi_peer_stats)));

		OS_MEMCPY(&(peer_stats->peer_macaddr), &(ev->peer_macaddr),
						sizeof(wmi_mac_addr));

		peer_stats->peer_rssi = ev->peer_rssi;
		peer_stats->peer_rssi_seq_num = ev->peer_rssi_seq_num;
		peer_stats->peer_tx_rate = ev->peer_tx_rate;
		peer_stats->peer_rx_rate = ev->peer_rx_rate;
		peer_stats->currentper = ev->currentper;
		peer_stats->retries = ev->retries;
		peer_stats->txratecount = ev->txratecount;
		peer_stats->max4msframelen = ev->max4msframelen;
		peer_stats->totalsubframes = ev->totalsubframes;
		peer_stats->txbytes = ev->txbytes;

		OS_MEMCPY(peer_stats->nobuffs, ev->nobuffs,
					sizeof(peer_stats->nobuffs));
		OS_MEMCPY(peer_stats->excretries, ev->excretries,
					sizeof(peer_stats->excretries));
		peer_stats->peer_rssi_changed = ev->peer_rssi_changed;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_bcnflt_stats_non_tlv() - extract bcn fault stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into bcn fault stats
 * @param bcnflt_stats: Pointer to hold bcn fault stats
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_bcnflt_stats_non_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index, wmi_host_bcnflt_stats *bcnflt_stats)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_peer_extd_stats_non_tlv() - extract extended peer stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended peer stats
 * @param peer_extd_stats: Pointer to hold extended peer stats
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_peer_extd_stats_non_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index,
	wmi_host_peer_extd_stats *peer_extd_stats)
{
	uint8_t *pdata = ((wmi_stats_event *)evt_buf)->data;

	if (WMI_REQUEST_PEER_EXTD_STAT &
		((wmi_stats_event *)evt_buf)->stats_id) {
		if (index < ((wmi_stats_event *)evt_buf)->num_peer_stats) {
			wmi_peer_extd_stats *ev = (wmi_peer_extd_stats *)
			((pdata) +
			((((wmi_stats_event *)evt_buf)->num_pdev_stats)	*
						sizeof(wmi_pdev_stats)) +
			((((wmi_stats_event *)evt_buf)->num_pdev_ext_stats) *
						 sizeof(wmi_pdev_ext_stats)) +
			((((wmi_stats_event *)evt_buf)->num_vdev_stats) *
						 sizeof(wmi_vdev_stats)) +
			((((wmi_stats_event *)evt_buf)->num_peer_stats) *
						 sizeof(wmi_peer_stats)) +
				(index * sizeof(wmi_peer_extd_stats)));

			OS_MEMCPY(peer_extd_stats, ev,
				sizeof(wmi_host_peer_extd_stats));
		}
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_vdev_extd_stats_non_tlv() - extract extended vdev stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into extended vdev stats
 * @param vdev_extd_stats: Pointer to hold extended vdev stats
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_vdev_extd_stats_non_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, uint32_t index,
	wmi_host_vdev_extd_stats *vdev_extd_stats)
{
	uint8_t *pdata = ((wmi_stats_event *)evt_buf)->data;

	if (WMI_REQUEST_PEER_EXTD_STAT &
		((wmi_stats_event *)evt_buf)->stats_id) {

		if (index < ((wmi_stats_event *)evt_buf)->num_vdev_stats) {

			wmi_vdev_extd_stats *ev = (wmi_vdev_extd_stats *)
			((pdata) +
			((((wmi_stats_event *)evt_buf)->num_pdev_stats) *
						 sizeof(wmi_pdev_stats)) +
			((((wmi_stats_event *)evt_buf)->num_pdev_ext_stats) *
						 sizeof(wmi_pdev_ext_stats)) +
			((((wmi_stats_event *)evt_buf)->num_vdev_stats) *
						 sizeof(wmi_vdev_stats)) +
			((((wmi_stats_event *)evt_buf)->num_peer_stats) *
						 sizeof(wmi_peer_stats)) +
			((((wmi_stats_event *)evt_buf)->num_peer_stats) *
						 sizeof(wmi_peer_extd_stats)) +
				(index * sizeof(wmi_vdev_extd_stats)));

		OS_MEMCPY(vdev_extd_stats, ev,
				sizeof(wmi_host_vdev_extd_stats));
		}
	}
	return QDF_STATUS_SUCCESS;
}
/**
 * extract_vdev_nac_rssi_stats_non_tlv() - extract vdev NAC_RSSI stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param vdev_nac_rssi_event: Pointer to hold vdev NAC_RSSI stats
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_vdev_nac_rssi_stats_non_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, struct wmi_host_vdev_nac_rssi_event *vdev_nac_rssi_stats)
{
	uint8_t *pdata = ((wmi_stats_event *)evt_buf)->data;

	if (WMI_REQUEST_NAC_RSSI_STAT &
		((wmi_stats_event *)evt_buf)->stats_id) {


		struct wmi_host_vdev_nac_rssi_event *ev = (struct wmi_host_vdev_nac_rssi_event *)
		((pdata) +
		((((wmi_stats_event *)evt_buf)->num_pdev_stats) *
					 sizeof(wmi_pdev_stats)) +
		((((wmi_stats_event *)evt_buf)->num_pdev_ext_stats) *
					 sizeof(wmi_pdev_ext_stats)) +
		((((wmi_stats_event *)evt_buf)->num_vdev_stats) *
					 sizeof(wmi_vdev_stats)) +
		((((wmi_stats_event *)evt_buf)->num_peer_stats) *
					 sizeof(wmi_peer_stats)) +
		((((wmi_stats_event *)evt_buf)->num_bcnflt_stats) *
					 sizeof(wmi_bcnfilter_stats_t)) +
		((WMI_REQUEST_PEER_EXTD_STAT &
		((wmi_stats_event *)evt_buf)->stats_id) ? ((((wmi_stats_event *)evt_buf)->num_peer_stats) *
					 sizeof(wmi_peer_extd_stats)) : 0) +
		((WMI_REQUEST_VDEV_EXTD_STAT &
		((wmi_stats_event *)evt_buf)->stats_id) ? ((((wmi_stats_event *)evt_buf)->num_vdev_stats)*
				       sizeof(wmi_vdev_extd_stats)) : 0) +
		((((wmi_stats_event *)evt_buf)->num_pdev_stats) *
					 (sizeof(wmi_pdev_ext2_stats))));

		OS_MEMCPY(vdev_nac_rssi_stats, ev,
				sizeof(struct wmi_host_vdev_nac_rssi_event));
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_chan_stats_non_tlv() - extract chan stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param index: Index into chan stats
 * @param vdev_extd_stats: Pointer to hold chan stats
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_chan_stats_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		uint32_t index, wmi_host_chan_stats *chan_stats)
{
	/* Non-TLV doesnt have num_chan_stats */
	return QDF_STATUS_SUCCESS;
}

/**
 * extract_profile_ctx_non_tlv() - extract profile context from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param profile_ctx: Pointer to hold profile context
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_profile_ctx_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_wlan_profile_ctx_t *profile_ctx)
{
	wmi_profile_stats_event *profile_ev =
		(wmi_profile_stats_event *)evt_buf;

	qdf_mem_copy(profile_ctx, &(profile_ev->profile_ctx),
		sizeof(wmi_host_wlan_profile_ctx_t));

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_profile_data_non_tlv() - extract profile data from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param profile_data: Pointer to hold profile data
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_profile_data_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf, uint8_t idx,
		wmi_host_wlan_profile_t *profile_data)
{
	wmi_profile_stats_event *profile_ev =
		(wmi_profile_stats_event *)evt_buf;

	if (idx > profile_ev->profile_ctx.bin_count)
		return QDF_STATUS_E_INVAL;

	qdf_mem_copy(profile_data, &profile_ev->profile_data[idx],
		sizeof(wmi_host_wlan_profile_t));

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_chan_info_event_non_tlv() - extract chan information from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param chan_info: Pointer to hold chan information
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_chan_info_event_non_tlv(wmi_unified_t wmi_handle,
		void *evt_buf,
		wmi_host_chan_info_event *chan_info)
{
	wmi_chan_info_event *chan_info_ev = (wmi_chan_info_event *)evt_buf;

	chan_info->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;
	chan_info->err_code = chan_info_ev->err_code;
	chan_info->freq = chan_info_ev->freq;
	chan_info->cmd_flags = chan_info_ev->cmd_flags;
	chan_info->noise_floor = chan_info_ev->noise_floor;
	chan_info->rx_clear_count = chan_info_ev->rx_clear_count;
	chan_info->cycle_count = chan_info_ev->cycle_count;
	chan_info->rx_11b_mode_data_duration =
			chan_info_ev->rx_11b_mode_data_duration;
	/* ONLY NON-TLV */
	chan_info->chan_tx_pwr_range = chan_info_ev->chan_tx_pwr_range;
	chan_info->chan_tx_pwr_tp = chan_info_ev->chan_tx_pwr_tp;
	chan_info->rx_frame_count = chan_info_ev->rx_frame_count;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_channel_hopping_event_non_tlv() - extract channel hopping param
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ch_hopping: Pointer to hold channel hopping param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_channel_hopping_event_non_tlv(
	wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_pdev_channel_hopping_event *ch_hopping)
{
	wmi_pdev_channel_hopping_event *event =
		(wmi_pdev_channel_hopping_event *)evt_buf;

	ch_hopping->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;
	ch_hopping->noise_floor_report_iter = event->noise_floor_report_iter;
	ch_hopping->noise_floor_total_iter = event->noise_floor_total_iter;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_bss_chan_info_event_non_tlv() - extract bss channel information
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param bss_chan_info: Pointer to hold bss channel information
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_bss_chan_info_event_non_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, wmi_host_pdev_bss_chan_info_event *bss_chan_info)
{
	wmi_pdev_bss_chan_info_event *event =
		(wmi_pdev_bss_chan_info_event *)evt_buf;

	bss_chan_info->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;
	bss_chan_info->freq = event->freq;
	bss_chan_info->noise_floor = event->noise_floor;
	bss_chan_info->rx_clear_count_low = event->rx_clear_count_low;
	bss_chan_info->rx_clear_count_high = event->rx_clear_count_high;
	bss_chan_info->cycle_count_low = event->cycle_count_low;
	bss_chan_info->cycle_count_high = event->cycle_count_high;
	bss_chan_info->tx_cycle_count_low = event->tx_cycle_count_low;
	bss_chan_info->tx_cycle_count_high = event->tx_cycle_count_high;
	bss_chan_info->rx_cycle_count_low = event->rx_cycle_count_low;
	bss_chan_info->rx_cycle_count_high = event->rx_cycle_count_high;
	bss_chan_info->rx_bss_cycle_count_low = event->rx_bss_cycle_count_low;
	bss_chan_info->rx_bss_cycle_count_high = event->rx_bss_cycle_count_high;
	bss_chan_info->reserved = event->reserved;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_inst_rssi_stats_event_non_tlv() - extract inst rssi stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param inst_rssi_resp: Pointer to hold inst rssi response
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_inst_rssi_stats_event_non_tlv(
	wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_inst_stats_resp *inst_rssi_resp)
{
	wmi_inst_stats_resp *event = (wmi_inst_stats_resp *)evt_buf;

	inst_rssi_resp->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;
	qdf_mem_copy(inst_rssi_resp, event, sizeof(wmi_inst_stats_resp));

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_tx_data_traffic_ctrl_ev_non_tlv() - extract tx data traffic control
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold data traffic control
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_tx_data_traffic_ctrl_ev_non_tlv(
	wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_tx_data_traffic_ctrl_event *ev)
{
	wmi_tx_data_traffic_ctrl_event *evt =
		(wmi_tx_data_traffic_ctrl_event *)evt_buf;

	ev->peer_ast_idx = evt->peer_ast_idx;
	ev->vdev_id = evt->vdev_id;
	ev->ctrl_cmd = evt->ctrl_cmd;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_atf_peer_stats_ev_non_tlv() - extract atf peer stats
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold atf stats event data
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_atf_peer_stats_ev_non_tlv(
	wmi_unified_t wmi_handle, void *evt_buf,
	wmi_host_atf_peer_stats_event *ev)
{
	wmi_atf_peer_stats_event *evt =
		(wmi_atf_peer_stats_event *)evt_buf;

	ev->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;
	ev->num_atf_peers = evt->num_atf_peers;
	ev->comp_usable_airtime = evt->comp_usable_airtime;
	qdf_mem_copy(&ev->reserved[0], &evt->reserved[0],
				sizeof(evt->reserved));

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_atf_token_info_ev_non_tlv() - extract atf token info
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @idx: Index indicating the peer number
 * @param atf_token_info: Pointer to hold atf token info
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_atf_token_info_ev_non_tlv(
	wmi_unified_t wmi_handle, void *evt_buf,
	uint8_t idx, wmi_host_atf_peer_stats_info *atf_token_info)
{
	wmi_atf_peer_stats_event *evt =
		(wmi_atf_peer_stats_event *)evt_buf;

	if (idx > evt->num_atf_peers)
		return QDF_STATUS_E_INVAL;

	atf_token_info->field1 = evt->token_info_list[idx].field1;
	atf_token_info->field2 = evt->token_info_list[idx].field2;
	atf_token_info->field3 = evt->token_info_list[idx].field3;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_utf_event_non_tlv() - extract UTF data info from event
 * @wmi_handle: WMI handle
 * @param evt_buf: Pointer to event buffer
 * @param param: Pointer to hold data
 *
 * Return : QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_pdev_utf_event_non_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct wmi_host_pdev_utf_event *event)
{
	event->data = evt_buf;
	event->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_pdev_qvit_event_non_tlv() - extract QVIT data info from event
 * @wmi_handle: WMI handle
 * @param evt_buf: Pointer to event buffer
 * @param param: Pointer to hold data
 *
 * Return : QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_pdev_qvit_event_non_tlv(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct wmi_host_pdev_qvit_event *event)
{
	event->data = evt_buf;
	event->pdev_id = WMI_NON_TLV_DEFAULT_PDEV_ID;

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_wds_entry_non_tlv() - extract wds entry from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @wds_entry: wds entry
 * @idx: index to point wds entry in event buffer
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_wds_entry_non_tlv(wmi_unified_t wmi_handle,
			u_int8_t *evt_buf,
			struct wdsentry *wds_entry,
			u_int32_t idx)
{
	wmi_pdev_wds_entry_dump_event *wds_entry_dump_event =
			(wmi_pdev_wds_entry_dump_event *)evt_buf;

	if (idx >= wds_entry_dump_event->num_entries)
		return QDF_STATUS_E_INVAL;
	qdf_mem_zero(wds_entry, sizeof(struct wdsentry));
	WMI_MAC_ADDR_TO_CHAR_ARRAY(
			&(wds_entry_dump_event->wds_entry[idx].peer_macaddr),
			wds_entry->peer_mac);
	WMI_MAC_ADDR_TO_CHAR_ARRAY(
			&(wds_entry_dump_event->wds_entry[idx].wds_macaddr),
			wds_entry->wds_mac);
	wds_entry->flags = wds_entry_dump_event->wds_entry[idx].flags;

	return QDF_STATUS_SUCCESS;
}


static bool is_management_record_non_tlv(uint32_t cmd_id)
{
	if ((cmd_id == WMI_BCN_TX_CMDID) ||
		(cmd_id == WMI_PDEV_SEND_BCN_CMDID) ||
		(cmd_id == WMI_MGMT_TX_CMDID) ||
		(cmd_id == WMI_GPIO_OUTPUT_CMDID) ||
		(cmd_id == WMI_HOST_SWBA_EVENTID)) {
		return true;
	}

	return false;
}

/**
 * wmi_set_htc_tx_tag_non_tlv() - set HTC TX tag for WMI commands
 * @wmi_handle: WMI handle
 * @buf:	WMI buffer
 * @cmd_id:	WMI command Id
 *
 * Return htc_tx_tag
 */
static uint16_t wmi_set_htc_tx_tag_non_tlv(wmi_unified_t wmi_handle,
				wmi_buf_t buf, uint32_t cmd_id)
{
	return 0;
}

/**
 * send_dfs_phyerr_offload_en_cmd_non_tlv() - send dfs phyerr offload en cmd
 * @wmi_handle: wmi handle
 * @pdev_id: pdev id
 *
 * Send WMI_PDEV_DFS_PHYERR_OFFLOAD_ENABLE_CMDID command to firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS send_dfs_phyerr_offload_en_cmd_non_tlv(
		wmi_unified_t wmi_handle,
		uint32_t pdev_id)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * send_dfs_phyerr_offload_dis_cmd_non_tlv() - send dfs phyerr offload dis cmd
 * @wmi_handle: wmi handle
 * @pdev_id: pdev id
 *
 * Send WMI_PDEV_DFS_PHYERR_OFFLOAD_DISABLE_CMDID command to firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS send_dfs_phyerr_offload_dis_cmd_non_tlv(
		wmi_unified_t wmi_handle,
		uint32_t pdev_id)
{
	return QDF_STATUS_SUCCESS;
}

/**
 *  send_wds_entry_list_cmd_non_tlv() - WMI function to get list of
 *  wds entries from FW
 *
 *  @param wmi_handle	  : handle to WMI.
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
QDF_STATUS send_wds_entry_list_cmd_non_tlv(wmi_unified_t wmi_handle)
{
	wmi_buf_t buf;

	/*
	 * Passing a NULL pointer to wmi_unified_cmd_send() panics it,
	 * so let's just use a 32 byte fake array for now.
	 */
	buf = wmi_buf_alloc(wmi_handle, 32);
	if (buf == NULL)
		return QDF_STATUS_E_NOMEM;

	if (wmi_unified_cmd_send(wmi_handle, buf, 32,
	  WMI_PDEV_WDS_ENTRY_LIST_CMDID) != QDF_STATUS_SUCCESS) {
		qdf_print("%s: send failed\n", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}


/**
 * wmi_non_tlv_pdev_id_conversion_enable() - Enable pdev_id conversion
 *
 * Return None.
 */
void wmi_non_tlv_pdev_id_conversion_enable(wmi_unified_t wmi_handle)
{
	qdf_print("PDEV conversion Not Available");
}

struct wmi_ops non_tlv_ops =  {
	.send_vdev_create_cmd = send_vdev_create_cmd_non_tlv,
	.send_vdev_delete_cmd = send_vdev_delete_cmd_non_tlv,
	.send_vdev_down_cmd = send_vdev_down_cmd_non_tlv,
	.send_peer_flush_tids_cmd = send_peer_flush_tids_cmd_non_tlv,
	.send_peer_param_cmd = send_peer_param_cmd_non_tlv,
	.send_vdev_up_cmd = send_vdev_up_cmd_non_tlv,
	.send_peer_create_cmd = send_peer_create_cmd_non_tlv,
	.send_peer_delete_cmd = send_peer_delete_cmd_non_tlv,
#ifdef WLAN_SUPPORT_GREEN_AP
	.send_green_ap_ps_cmd = send_green_ap_ps_cmd_non_tlv,
#endif
	.send_pdev_utf_cmd = send_pdev_utf_cmd_non_tlv,
	.send_pdev_param_cmd = send_pdev_param_cmd_non_tlv,
	.send_suspend_cmd = send_suspend_cmd_non_tlv,
	.send_resume_cmd = send_resume_cmd_non_tlv,
	.send_wow_enable_cmd = send_wow_enable_cmd_non_tlv,
	.send_set_ap_ps_param_cmd = send_set_ap_ps_param_cmd_non_tlv,
	.send_set_sta_ps_param_cmd = send_set_sta_ps_param_cmd_non_tlv,
	.send_crash_inject_cmd = send_crash_inject_cmd_non_tlv,
	.send_dbglog_cmd = send_dbglog_cmd_non_tlv,
	.send_vdev_set_param_cmd = send_vdev_set_param_cmd_non_tlv,
	.send_stats_request_cmd = send_stats_request_cmd_non_tlv,
	.send_packet_log_enable_cmd = send_packet_log_enable_cmd_non_tlv,
	.send_packet_log_disable_cmd = send_packet_log_disable_cmd_non_tlv,
	.send_beacon_send_cmd = send_beacon_send_cmd_non_tlv,
	.send_peer_assoc_cmd = send_peer_assoc_cmd_non_tlv,
	.send_scan_start_cmd = send_scan_start_cmd_non_tlv,
	.send_scan_stop_cmd = send_scan_stop_cmd_non_tlv,
	.send_scan_chan_list_cmd = send_scan_chan_list_cmd_non_tlv,
	.send_pdev_get_tpc_config_cmd = send_pdev_get_tpc_config_cmd_non_tlv,
	.send_set_atf_cmd = send_set_atf_cmd_non_tlv,
	.send_atf_peer_request_cmd = send_atf_peer_request_cmd_non_tlv,
	.send_set_atf_grouping_cmd = send_set_atf_grouping_cmd_non_tlv,
	.send_set_bwf_cmd = send_set_bwf_cmd_non_tlv,
	.send_pdev_fips_cmd = send_pdev_fips_cmd_non_tlv,
	.send_wlan_profile_enable_cmd = send_wlan_profile_enable_cmd_non_tlv,
	.send_wlan_profile_trigger_cmd = send_wlan_profile_trigger_cmd_non_tlv,
	.send_pdev_set_chan_cmd = send_pdev_set_chan_cmd_non_tlv,
	.send_set_ht_ie_cmd = send_set_ht_ie_cmd_non_tlv,
	.send_set_vht_ie_cmd = send_set_vht_ie_cmd_non_tlv,
	.send_wmm_update_cmd = send_wmm_update_cmd_non_tlv,
	.send_set_ant_switch_tbl_cmd = send_set_ant_switch_tbl_cmd_non_tlv,
	.send_set_ratepwr_table_cmd = send_set_ratepwr_table_cmd_non_tlv,
	.send_get_ratepwr_table_cmd = send_get_ratepwr_table_cmd_non_tlv,
	.send_set_ctl_table_cmd = send_set_ctl_table_cmd_non_tlv,
	.send_set_mimogain_table_cmd = send_set_mimogain_table_cmd_non_tlv,
	.send_set_ratepwr_chainmsk_cmd = send_set_ratepwr_chainmsk_cmd_non_tlv,
	.send_set_macaddr_cmd = send_set_macaddr_cmd_non_tlv,
	.send_pdev_scan_start_cmd = send_pdev_scan_start_cmd_non_tlv,
	.send_pdev_scan_end_cmd = send_pdev_scan_end_cmd_non_tlv,
	.send_set_acparams_cmd = send_set_acparams_cmd_non_tlv,
	.send_set_vap_dscp_tid_map_cmd = send_set_vap_dscp_tid_map_cmd_non_tlv,
	.send_proxy_ast_reserve_cmd = send_proxy_ast_reserve_cmd_non_tlv,
	.send_pdev_qvit_cmd = send_pdev_qvit_cmd_non_tlv,
	.send_mcast_group_update_cmd = send_mcast_group_update_cmd_non_tlv,
	.send_peer_add_wds_entry_cmd = send_peer_add_wds_entry_cmd_non_tlv,
	.send_peer_del_wds_entry_cmd = send_peer_del_wds_entry_cmd_non_tlv,
	.send_set_bridge_mac_addr_cmd = send_set_bridge_mac_addr_cmd_non_tlv,
	.send_peer_update_wds_entry_cmd =
				send_peer_update_wds_entry_cmd_non_tlv,
	.send_phyerr_enable_cmd = send_phyerr_enable_cmd_non_tlv,
	.send_phyerr_disable_cmd = send_phyerr_disable_cmd_non_tlv,
	.send_smart_ant_enable_cmd = send_smart_ant_enable_cmd_non_tlv,
	.send_smart_ant_set_rx_ant_cmd = send_smart_ant_set_rx_ant_cmd_non_tlv,
	.send_smart_ant_set_tx_ant_cmd = send_smart_ant_set_tx_ant_cmd_non_tlv,
	.send_smart_ant_set_training_info_cmd =
			send_smart_ant_set_training_info_cmd_non_tlv,
	.send_smart_ant_set_node_config_cmd =
			send_smart_ant_set_node_config_cmd_non_tlv,
	.send_smart_ant_enable_tx_feedback_cmd =
			send_smart_ant_enable_tx_feedback_cmd_non_tlv,
	.send_vdev_spectral_configure_cmd =
			send_vdev_spectral_configure_cmd_non_tlv,
	.send_vdev_spectral_enable_cmd =
				send_vdev_spectral_enable_cmd_non_tlv,
	.send_bss_chan_info_request_cmd =
				send_bss_chan_info_request_cmd_non_tlv,
	.send_thermal_mitigation_param_cmd =
				send_thermal_mitigation_param_cmd_non_tlv,
	.send_vdev_start_cmd = send_vdev_start_cmd_non_tlv,
	.send_vdev_set_nac_rssi_cmd = send_vdev_set_nac_rssi_cmd_non_tlv,
	.send_vdev_stop_cmd = send_vdev_stop_cmd_non_tlv,
	.send_vdev_set_neighbour_rx_cmd =
			send_vdev_set_neighbour_rx_cmd_non_tlv,
	.send_vdev_set_fwtest_param_cmd =
			send_vdev_set_fwtest_param_cmd_non_tlv,
	.send_vdev_config_ratemask_cmd = send_vdev_config_ratemask_cmd_non_tlv,
	.send_setup_install_key_cmd =
				send_setup_install_key_cmd_non_tlv,
	.send_wow_wakeup_cmd = send_wow_wakeup_cmd_non_tlv,
	.send_wow_add_wakeup_event_cmd = send_wow_add_wakeup_event_cmd_non_tlv,
	.send_wow_add_wakeup_pattern_cmd =
			send_wow_add_wakeup_pattern_cmd_non_tlv,
	.send_wow_remove_wakeup_pattern_cmd =
			send_wow_remove_wakeup_pattern_cmd_non_tlv,
	.send_pdev_set_regdomain_cmd =
				send_pdev_set_regdomain_cmd_non_tlv,
	.send_set_quiet_mode_cmd = send_set_quiet_mode_cmd_non_tlv,
	.send_set_beacon_filter_cmd = send_set_beacon_filter_cmd_non_tlv,
	.send_remove_beacon_filter_cmd = send_remove_beacon_filter_cmd_non_tlv,
	.send_mgmt_cmd = send_mgmt_cmd_non_tlv,
	.send_addba_clearresponse_cmd = send_addba_clearresponse_cmd_non_tlv,
	.send_addba_send_cmd = send_addba_send_cmd_non_tlv,
	.send_delba_send_cmd = send_delba_send_cmd_non_tlv,
	.send_addba_setresponse_cmd = send_addba_setresponse_cmd_non_tlv,
	.send_singleamsdu_cmd = send_singleamsdu_cmd_non_tlv,
	.send_set_qboost_param_cmd = send_set_qboost_param_cmd_non_tlv,
	.send_mu_scan_cmd = send_mu_scan_cmd_non_tlv,
	.send_lteu_config_cmd = send_lteu_config_cmd_non_tlv,
	.send_set_ps_mode_cmd = send_set_ps_mode_cmd_non_tlv,
	.init_cmd_send = init_cmd_send_non_tlv,
	.send_ext_resource_config = send_ext_resource_config_non_tlv,
#if 0
	.send_bcn_prb_template_cmd = send_bcn_prb_template_cmd_non_tlv,
#endif
	.send_nf_dbr_dbm_info_get_cmd = send_nf_dbr_dbm_info_get_cmd_non_tlv,
	.send_packet_power_info_get_cmd =
			send_packet_power_info_get_cmd_non_tlv,
	.send_gpio_config_cmd = send_gpio_config_cmd_non_tlv,
	.send_gpio_output_cmd = send_gpio_output_cmd_non_tlv,
	.send_rtt_meas_req_test_cmd = send_rtt_meas_req_test_cmd_non_tlv,
	.send_rtt_meas_req_cmd = send_rtt_meas_req_cmd_non_tlv,
	.send_lci_set_cmd = send_lci_set_cmd_non_tlv,
	.send_lcr_set_cmd = send_lcr_set_cmd_non_tlv,
	.send_start_oem_data_cmd = send_start_oem_data_cmd_non_tlv,
	.send_rtt_keepalive_req_cmd = send_rtt_keepalive_req_cmd_non_tlv,
	.send_periodic_chan_stats_config_cmd =
			send_periodic_chan_stats_config_cmd_non_tlv,
	.send_get_user_position_cmd = send_get_user_position_cmd_non_tlv,
	.send_reset_peer_mumimo_tx_count_cmd =
			send_reset_peer_mumimo_tx_count_cmd_non_tlv,
	.send_get_peer_mumimo_tx_count_cmd =
			send_get_peer_mumimo_tx_count_cmd_non_tlv,
	.send_pdev_caldata_version_check_cmd =
			send_pdev_caldata_version_check_cmd_non_tlv,
	.send_btcoex_wlan_priority_cmd = send_btcoex_wlan_priority_cmd_non_tlv,
	.send_btcoex_duty_cycle_cmd = send_btcoex_duty_cycle_cmd_non_tlv,
	.send_coex_ver_cfg_cmd = send_coex_ver_cfg_cmd_non_tlv,

	.get_target_cap_from_service_ready = extract_service_ready_non_tlv,
	.extract_fw_version = extract_fw_version_non_tlv,
	.extract_fw_abi_version = extract_fw_abi_version_non_tlv,
	.extract_hal_reg_cap = extract_hal_reg_cap_non_tlv,
	.extract_host_mem_req = extract_host_mem_req_non_tlv,
	.save_service_bitmap = save_service_bitmap_non_tlv,
	.is_service_enabled = is_service_enabled_non_tlv,
	.save_fw_version = save_fw_version_in_service_ready_non_tlv,
	.check_and_update_fw_version =
			ready_check_and_update_fw_version_non_tlv,
	.extract_dbglog_data_len = extract_dbglog_data_len_non_tlv,
	.ready_extract_init_status = ready_extract_init_status_non_tlv,
	.ready_extract_mac_addr = ready_extract_mac_addr_non_tlv,
	.extract_ready_event_params = extract_ready_event_params_non_tlv,
	.extract_wds_addr_event = extract_wds_addr_event_non_tlv,
	.extract_dcs_interference_type = extract_dcs_interference_type_non_tlv,
	.extract_dcs_cw_int = extract_dcs_cw_int_non_tlv,
	.extract_dcs_im_tgt_stats = extract_dcs_im_tgt_stats_non_tlv,
	.extract_vdev_start_resp = extract_vdev_start_resp_non_tlv,
	.extract_tbttoffset_update_params =
			extract_tbttoffset_update_params_non_tlv,
	.extract_tbttoffset_num_vdevs =
			extract_tbttoffset_num_vdevs_non_tlv,
	.extract_mgmt_rx_params = extract_mgmt_rx_params_non_tlv,
	.extract_vdev_stopped_param =  extract_vdev_stopped_param_non_tlv,
	.extract_vdev_roam_param = extract_vdev_roam_param_non_tlv,
	.extract_vdev_scan_ev_param = extract_vdev_scan_ev_param_non_tlv,
	.extract_mu_ev_param = extract_mu_ev_param_non_tlv,
	.extract_pdev_tpc_config_ev_param =
			extract_pdev_tpc_config_ev_param_non_tlv,
	.extract_nfcal_power_ev_param = extract_nfcal_power_ev_param_non_tlv,
	.extract_pdev_tpc_ev_param = extract_pdev_tpc_ev_param_non_tlv,
	.extract_pdev_generic_buffer_ev_param =
			extract_pdev_generic_buffer_ev_param_non_tlv,
	.extract_gpio_input_ev_param = extract_gpio_input_ev_param_non_tlv,
	.extract_pdev_reserve_ast_ev_param =
			extract_pdev_reserve_ast_ev_param_non_tlv,
	.extract_swba_num_vdevs = extract_swba_num_vdevs_non_tlv,
	.extract_swba_tim_info = extract_swba_tim_info_non_tlv,
	.extract_swba_noa_info = extract_swba_noa_info_non_tlv,
	.extract_peer_sta_ps_statechange_ev =
			extract_peer_sta_ps_statechange_ev_non_tlv,
	.extract_peer_sta_kickout_ev = extract_peer_sta_kickout_ev_non_tlv,
	.extract_peer_ratecode_list_ev = extract_peer_ratecode_list_ev_non_tlv,
	.extract_comb_phyerr = extract_comb_phyerr_non_tlv,
	.extract_single_phyerr = extract_single_phyerr_non_tlv,
	.extract_composite_phyerr = extract_composite_phyerr_non_tlv,
	.extract_rtt_hdr = extract_rtt_hdr_non_tlv,
	.extract_rtt_ev = extract_rtt_ev_non_tlv,
	.extract_rtt_error_report_ev = extract_rtt_error_report_ev_non_tlv,
	.extract_all_stats_count = extract_all_stats_counts_non_tlv,
	.extract_pdev_stats = extract_pdev_stats_non_tlv,
	.extract_pdev_ext_stats = extract_pdev_ext_stats_non_tlv,
	.extract_vdev_stats = extract_vdev_stats_non_tlv,
	.extract_peer_stats = extract_peer_stats_non_tlv,
	.extract_bcnflt_stats = extract_bcnflt_stats_non_tlv,
	.extract_peer_extd_stats = extract_peer_extd_stats_non_tlv,
	.extract_chan_stats = extract_chan_stats_non_tlv,
	.extract_thermal_stats = extract_thermal_stats_non_tlv,
	.extract_thermal_level_stats = extract_thermal_level_stats_non_tlv,
	.extract_profile_ctx = extract_profile_ctx_non_tlv,
	.extract_profile_data = extract_profile_data_non_tlv,
	.extract_chan_info_event = extract_chan_info_event_non_tlv,
	.extract_channel_hopping_event = extract_channel_hopping_event_non_tlv,
	.extract_bss_chan_info_event = extract_bss_chan_info_event_non_tlv,
	.extract_inst_rssi_stats_event = extract_inst_rssi_stats_event_non_tlv,
	.extract_tx_data_traffic_ctrl_ev =
				extract_tx_data_traffic_ctrl_ev_non_tlv,
	.extract_vdev_extd_stats = extract_vdev_extd_stats_non_tlv,
	.extract_vdev_nac_rssi_stats = extract_vdev_nac_rssi_stats_non_tlv,
	.extract_fips_event_data = extract_fips_event_data_non_tlv,
	.extract_mumimo_tx_count_ev_param =
				extract_mumimo_tx_count_ev_param_non_tlv,
	.extract_peer_gid_userpos_list_ev_param =
				extract_peer_gid_userpos_list_ev_param_non_tlv,
	.extract_pdev_caldata_version_check_ev_param =
			extract_pdev_caldata_version_check_ev_param_non_tlv,
	.extract_mu_db_entry = extract_mu_db_entry_non_tlv,
	.extract_atf_peer_stats_ev = extract_atf_peer_stats_ev_non_tlv,
	.extract_atf_token_info_ev = extract_atf_token_info_ev_non_tlv,
	.extract_pdev_utf_event = extract_pdev_utf_event_non_tlv,
	.wmi_set_htc_tx_tag = wmi_set_htc_tx_tag_non_tlv,
	.is_management_record = is_management_record_non_tlv,
	.send_dfs_phyerr_offload_en_cmd =
		send_dfs_phyerr_offload_en_cmd_non_tlv,
	.send_dfs_phyerr_offload_dis_cmd =
		send_dfs_phyerr_offload_dis_cmd_non_tlv,
	.send_wds_entry_list_cmd = send_wds_entry_list_cmd_non_tlv,
	.extract_wds_entry = extract_wds_entry_non_tlv,
#ifdef WLAN_SUPPORT_FILS
	.send_vdev_fils_enable_cmd = send_vdev_fils_enable_cmd_non_tlv,
	.send_fils_discovery_send_cmd = send_fils_discovery_send_cmd_non_tlv,
	.extract_swfda_vdev_id = extract_swfda_vdev_id_non_tlv,
#endif /* WLAN_SUPPORT_FILS */
	.wmi_pdev_id_conversion_enable = wmi_non_tlv_pdev_id_conversion_enable,
};

/**
 * populate_non_tlv_service() - populates wmi services
 *
 * @param wmi_service: Pointer to hold wmi_service
 * Return: None
 */
static void populate_non_tlv_service(uint32_t *wmi_service)
{
	wmi_service[wmi_service_beacon_offload] = WMI_SERVICE_BEACON_OFFLOAD;
	wmi_service[wmi_service_scan_offload] = WMI_SERVICE_SCAN_OFFLOAD;
	wmi_service[wmi_service_roam_offload] = WMI_SERVICE_ROAM_OFFLOAD;
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
	wmi_service[wmi_service_ratectrl] = WMI_SERVICE_RATECTRL;
	wmi_service[wmi_service_wow] = WMI_SERVICE_WOW;
	wmi_service[wmi_service_ratectrl_cache] = WMI_SERVICE_RATECTRL_CACHE;
	wmi_service[wmi_service_iram_tids] = WMI_SERVICE_IRAM_TIDS;
	wmi_service[wmi_service_burst] = WMI_SERVICE_BURST;
	wmi_service[wmi_service_smart_antenna_sw_support] =
				WMI_SERVICE_SMART_ANTENNA_SW_SUPPORT;
	wmi_service[wmi_service_gtk_offload] = WMI_SERVICE_GTK_OFFLOAD;
	wmi_service[wmi_service_scan_sch] = WMI_SERVICE_SCAN_SCH;
	wmi_service[wmi_service_csa_offload] = WMI_SERVICE_CSA_OFFLOAD;
	wmi_service[wmi_service_chatter] = WMI_SERVICE_CHATTER;
	wmi_service[wmi_service_coex_freqavoid] = WMI_SERVICE_COEX_FREQAVOID;
	wmi_service[wmi_service_packet_power_save] =
					WMI_SERVICE_PACKET_POWER_SAVE;
	wmi_service[wmi_service_force_fw_hang] = WMI_SERVICE_FORCE_FW_HANG;
	wmi_service[wmi_service_smart_antenna_hw_support] =
					WMI_SERVICE_SMART_ANTENNA_HW_SUPPORT;
	wmi_service[wmi_service_gpio] = WMI_SERVICE_GPIO;
	wmi_service[wmi_sta_uapsd_basic_auto_trig] =
					WMI_STA_UAPSD_BASIC_AUTO_TRIG;
	wmi_service[wmi_sta_uapsd_var_auto_trig] = WMI_STA_UAPSD_VAR_AUTO_TRIG;
	wmi_service[wmi_service_sta_keep_alive] = WMI_SERVICE_STA_KEEP_ALIVE;
	wmi_service[wmi_service_tx_encap] = WMI_SERVICE_TX_ENCAP;
	wmi_service[wmi_service_ap_ps_detect_out_of_sync] =
					WMI_SERVICE_AP_PS_DETECT_OUT_OF_SYNC;
	wmi_service[wmi_service_early_rx] =
					WMI_SERVICE_EARLY_RX;
	wmi_service[wmi_service_enhanced_proxy_sta] =
					WMI_SERVICE_ENHANCED_PROXY_STA;
	wmi_service[wmi_service_tt] = WMI_SERVICE_TT;
	wmi_service[wmi_service_atf] = WMI_SERVICE_ATF;
	wmi_service[wmi_service_peer_caching] = WMI_SERVICE_PEER_CACHING;
	wmi_service[wmi_service_coex_gpio] = WMI_SERVICE_COEX_GPIO;
	wmi_service[wmi_service_aux_spectral_intf] =
				WMI_SERVICE_AUX_SPECTRAL_INTF;
	wmi_service[wmi_service_aux_chan_load_intf] =
					WMI_SERVICE_AUX_CHAN_LOAD_INTF;
	wmi_service[wmi_service_bss_channel_info_64] =
					WMI_SERVICE_BSS_CHANNEL_INFO_64;
	wmi_service[wmi_service_ext_res_cfg_support] =
					WMI_SERVICE_EXT_RES_CFG_SUPPORT;
	wmi_service[wmi_service_mesh] = WMI_SERVICE_MESH;
	wmi_service[wmi_service_restrt_chnl_support] =
					WMI_SERVICE_RESTRT_CHNL_SUPPORT;
	wmi_service[wmi_service_peer_stats] = WMI_SERVICE_PEER_STATS;
	wmi_service[wmi_service_mesh_11s] = WMI_SERVICE_MESH_11S;
	wmi_service[wmi_service_periodic_chan_stat_support] =
				WMI_SERVICE_PERIODIC_CHAN_STAT_SUPPORT;
	wmi_service[wmi_service_tx_mode_push_only] =
				WMI_SERVICE_TX_MODE_PUSH_ONLY;
	wmi_service[wmi_service_tx_mode_push_pull] =
				WMI_SERVICE_TX_MODE_PUSH_PULL;
	wmi_service[wmi_service_tx_mode_dynamic] = WMI_SERVICE_TX_MODE_DYNAMIC;
	wmi_service[wmi_service_check_cal_version] =
				WMI_SERVICE_CHECK_CAL_VERSION;
	wmi_service[wmi_service_btcoex_duty_cycle] =
				WMI_SERVICE_BTCOEX_DUTY_CYCLE;
	wmi_service[wmi_service_4_wire_coex_support] =
				WMI_SERVICE_4_WIRE_COEX_SUPPORT;
	wmi_service[wmi_service_extended_nss_support] =
				WMI_SERVICE_EXTENDED_NSS_SUPPORT;

	wmi_service[wmi_service_roam_scan_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_arpns_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_nlo] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_sta_dtim_ps_modulated_dtim] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_sta_smps] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_fwtest] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_sta_wmmac] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tdls] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mcc_bcn_interval_change] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_adaptive_ocs] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_ba_ssn_support] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_filter_ipsec_natkeepalive] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_wlan_hb] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_lte_ant_share_support] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_batch_scan] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_qpower] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_plmreq] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_thermal_mgmt] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_rmc] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mhf_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_coex_sar] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_bcn_txrate_override] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_nan] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_l1ss_stat] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_estimate_linkspeed] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_obss_scan] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tdls_offchan] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tdls_uapsd_buffer_sta] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tdls_uapsd_sleep_sta] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_ibss_pwrsave] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_lpass] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_extscan] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_d0wow] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_hsoffload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_roam_ho_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_rx_full_reorder] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_dhcp_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_sta_rx_ipa_offload_support] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mdns_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_sap_auth_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_dual_band_simultaneous_support] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_ocb] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_ap_arpns_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_per_band_chainmask_support] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_packet_filter_offload] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mgmt_tx_htt] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mgmt_tx_wmi] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_ext_msg] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mawc] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_multiple_vdev_restart] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_peer_assoc_conf] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_egap] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_sta_pmf_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_unified_wow_capability] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_enterprise_mesh] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_bpf_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_sync_delete_cmds] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_ratectrl_limit_max_min_rates] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_nan_data] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_nan_rtt] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_11ax] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_deprecated_replace] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tdls_conn_tracker_in_host_mode] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_enhanced_mcast_filter] =WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_half_rate_quarter_rate_support] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_vdev_rx_filter] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_p2p_listen_offload_support] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_mark_first_wakeup_packet] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_multiple_mcast_filter_set] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_host_managed_rx_reorder] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_flash_rdwr_support] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_wlan_stats_report] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_tx_msdu_id_new_partition_support] =
				WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_dfs_phyerr_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_rcpi_support] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_fw_mem_dump_support] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_peer_stats_info] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_regulatory_db] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_11d_offload] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_hw_data_filtering] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_pkt_routing] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_offchan_tx_wmi] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_chan_load_info] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_ack_timeout] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_widebw_scan] = WMI_SERVICE_UNAVAILABLE;
	wmi_service[wmi_service_support_dma] =
				WMI_SERVICE_UNAVAILABLE;
}

/**
 * populate_non_tlv_event_id() - populates wmi event ids
 *
 * @param event_ids: Pointer to hold event ids
 * Return: None
 */
static void populate_non_tlv_events_id(uint32_t *event_ids)
{
	event_ids[wmi_service_ready_event_id] = WMI_SERVICE_READY_EVENTID;
	event_ids[wmi_ready_event_id] = WMI_READY_EVENTID;
	event_ids[wmi_dbg_msg_event_id] = WMI_DEBUG_MESG_EVENTID;
	event_ids[wmi_scan_event_id] = WMI_SCAN_EVENTID;
	event_ids[wmi_echo_event_id] = WMI_ECHO_EVENTID;
	event_ids[wmi_update_stats_event_id] = WMI_UPDATE_STATS_EVENTID;
	event_ids[wmi_inst_rssi_stats_event_id] = WMI_INST_RSSI_STATS_EVENTID;
	event_ids[wmi_vdev_start_resp_event_id] = WMI_VDEV_START_RESP_EVENTID;
	event_ids[wmi_vdev_standby_req_event_id] = WMI_VDEV_STANDBY_REQ_EVENTID;
	event_ids[wmi_vdev_resume_req_event_id] = WMI_VDEV_RESUME_REQ_EVENTID;
	event_ids[wmi_vdev_stopped_event_id] = WMI_VDEV_STOPPED_EVENTID;
	event_ids[wmi_peer_sta_kickout_event_id] = WMI_PEER_STA_KICKOUT_EVENTID;
	event_ids[wmi_host_swba_event_id] = WMI_HOST_SWBA_EVENTID;
	event_ids[wmi_tbttoffset_update_event_id] =
				WMI_TBTTOFFSET_UPDATE_EVENTID;
	event_ids[wmi_mgmt_rx_event_id] = WMI_MGMT_RX_EVENTID;
	event_ids[wmi_chan_info_event_id] = WMI_CHAN_INFO_EVENTID;
	event_ids[wmi_phyerr_event_id] = WMI_PHYERR_EVENTID;
	event_ids[wmi_roam_event_id] = WMI_ROAM_EVENTID;
	event_ids[wmi_profile_match] = WMI_PROFILE_MATCH;
	event_ids[wmi_debug_print_event_id] = WMI_DEBUG_PRINT_EVENTID;
	event_ids[wmi_pdev_qvit_event_id] = WMI_PDEV_QVIT_EVENTID;
	event_ids[wmi_wlan_profile_data_event_id] =
				WMI_WLAN_PROFILE_DATA_EVENTID;
	event_ids[wmi_rtt_meas_report_event_id] =
				WMI_RTT_MEASUREMENT_REPORT_EVENTID;
	event_ids[wmi_tsf_meas_report_event_id] =
				WMI_TSF_MEASUREMENT_REPORT_EVENTID;
	event_ids[wmi_rtt_error_report_event_id] = WMI_RTT_ERROR_REPORT_EVENTID;
	event_ids[wmi_rtt_keepalive_event_id] = WMI_RTT_KEEPALIVE_EVENTID;
	event_ids[wmi_oem_cap_event_id] = WMI_OEM_CAPABILITY_EVENTID;
	event_ids[wmi_oem_meas_report_event_id] =
					WMI_OEM_MEASUREMENT_REPORT_EVENTID;
	event_ids[wmi_oem_report_event_id] = WMI_OEM_ERROR_REPORT_EVENTID;
	event_ids[wmi_nan_event_id] = WMI_NAN_EVENTID;
	event_ids[wmi_wow_wakeup_host_event_id] = WMI_WOW_WAKEUP_HOST_EVENTID;
	event_ids[wmi_gtk_offload_status_event_id] =
					WMI_GTK_OFFLOAD_STATUS_EVENTID;
	event_ids[wmi_gtk_rekey_fail_event_id] = WMI_GTK_REKEY_FAIL_EVENTID;
	event_ids[wmi_dcs_interference_event_id] = WMI_DCS_INTERFERENCE_EVENTID;
	event_ids[wmi_pdev_tpc_config_event_id] = WMI_PDEV_TPC_CONFIG_EVENTID;
	event_ids[wmi_csa_handling_event_id] = WMI_CSA_HANDLING_EVENTID;
	event_ids[wmi_gpio_input_event_id] = WMI_GPIO_INPUT_EVENTID;
	event_ids[wmi_peer_ratecode_list_event_id] =
				WMI_PEER_RATECODE_LIST_EVENTID;
	event_ids[wmi_generic_buffer_event_id] = WMI_GENERIC_BUFFER_EVENTID;
	event_ids[wmi_mcast_buf_release_event_id] =
					WMI_MCAST_BUF_RELEASE_EVENTID;
	event_ids[wmi_mcast_list_ageout_event_id] =
					WMI_MCAST_LIST_AGEOUT_EVENTID;
	event_ids[wmi_vdev_get_keepalive_event_id] =
					WMI_VDEV_GET_KEEPALIVE_EVENTID;
	event_ids[wmi_wds_peer_event_id] = WMI_WDS_PEER_EVENTID;
	event_ids[wmi_peer_sta_ps_statechg_event_id] =
				WMI_PEER_STA_PS_STATECHG_EVENTID;
	event_ids[wmi_pdev_fips_event_id] = WMI_PDEV_FIPS_EVENTID;
	event_ids[wmi_tt_stats_event_id] = WMI_TT_STATS_EVENTID;
	event_ids[wmi_pdev_channel_hopping_event_id] =
					WMI_PDEV_CHANNEL_HOPPING_EVENTID;
	event_ids[wmi_pdev_ani_cck_level_event_id] =
					WMI_PDEV_ANI_CCK_LEVEL_EVENTID;
	event_ids[wmi_pdev_ani_ofdm_level_event_id] =
					WMI_PDEV_ANI_OFDM_LEVEL_EVENTID;
	event_ids[wmi_pdev_reserve_ast_entry_event_id] =
					WMI_PDEV_RESERVE_AST_ENTRY_EVENTID;
	event_ids[wmi_pdev_nfcal_power_event_id] = WMI_PDEV_NFCAL_POWER_EVENTID;
	event_ids[wmi_pdev_tpc_event_id] = WMI_PDEV_TPC_EVENTID;
	event_ids[wmi_pdev_get_ast_info_event_id] =
					WMI_PDEV_GET_AST_INFO_EVENTID;
	event_ids[wmi_pdev_temperature_event_id] = WMI_PDEV_TEMPERATURE_EVENTID;
	event_ids[wmi_pdev_nfcal_power_all_channels_event_id] =
				WMI_PDEV_NFCAL_POWER_ALL_CHANNELS_EVENTID;
	event_ids[wmi_pdev_bss_chan_info_event_id] =
					WMI_PDEV_BSS_CHAN_INFO_EVENTID;
	event_ids[wmi_mu_report_event_id] = WMI_MU_REPORT_EVENTID;
	event_ids[wmi_tx_data_traffic_ctrl_event_id] =
					WMI_TX_DATA_TRAFFIC_CTRL_EVENTID;
	event_ids[wmi_pdev_utf_event_id] = WMI_PDEV_UTF_EVENTID;
	event_ids[wmi_peer_tx_mu_txmit_count_event_id] =
					WMI_PEER_TX_MU_TXMIT_COUNT_EVENTID;
	event_ids[wmi_peer_gid_userpos_list_event_id] =
					WMI_PEER_GID_USERPOS_LIST_EVENTID;
	event_ids[wmi_pdev_check_cal_version_event_id] =
					WMI_PDEV_CHECK_CAL_VERSION_EVENTID;
	event_ids[wmi_atf_peer_stats_event_id] =
					WMI_ATF_PEER_STATS_EVENTID;
	event_ids[wmi_pdev_wds_entry_list_event_id] =
					WMI_PDEV_WDS_ENTRY_LIST_EVENTID;
	event_ids[wmi_host_swfda_event_id] = WMI_HOST_SWFDA_EVENTID;
}

/**
 * populate_pdev_param_non_tlv() - populates pdev params
 *
 * @param pdev_param: Pointer to hold pdev params
 * Return: None
 */
static void populate_pdev_param_non_tlv(uint32_t *pdev_param)
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
	pdev_param[wmi_pdev_param_pmf_qos] =
		WMI_PDEV_PARAM_PMF_QOS;
	pdev_param[wmi_pdev_param_arp_ac_override] =
		WMI_PDEV_PARAM_ARP_AC_OVERRIDE;
	pdev_param[wmi_pdev_param_dcs] =
		WMI_PDEV_PARAM_DCS;
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
	pdev_param[wmi_pdev_param_aggr_burst] = WMI_PDEV_PARAM_AGGR_BURST;
	pdev_param[wmi_pdev_param_rx_decap_mode] = WMI_PDEV_PARAM_RX_DECAP_MODE;
	pdev_param[wmi_pdev_param_fast_channel_reset] =
		WMI_PDEV_PARAM_FAST_CHANNEL_RESET;
	pdev_param[wmi_pdev_param_burst_dur] = WMI_PDEV_PARAM_BURST_DUR;
	pdev_param[wmi_pdev_param_burst_enable] = WMI_PDEV_PARAM_BURST_ENABLE;
	pdev_param[wmi_pdev_param_smart_antenna_default_antenna] =
		WMI_PDEV_PARAM_SMART_ANTENNA_DEFAULT_ANTENNA;
	pdev_param[wmi_pdev_param_igmpmld_override] =
		WMI_PDEV_PARAM_IGMPMLD_OVERRIDE;
	pdev_param[wmi_pdev_param_igmpmld_tid] =
		WMI_PDEV_PARAM_IGMPMLD_TID;
	pdev_param[wmi_pdev_param_antenna_gain] = WMI_PDEV_PARAM_ANTENNA_GAIN;
	pdev_param[wmi_pdev_param_rx_filter] = WMI_PDEV_PARAM_RX_FILTER;
	pdev_param[wmi_pdev_set_mcast_to_ucast_tid] =
		WMI_PDEV_SET_MCAST_TO_UCAST_TID;
	pdev_param[wmi_pdev_param_proxy_sta_mode] =
		WMI_PDEV_PARAM_PROXY_STA_MODE;
	pdev_param[wmi_pdev_param_set_mcast2ucast_mode] =
		WMI_PDEV_PARAM_SET_MCAST2UCAST_MODE;
	pdev_param[wmi_pdev_param_set_mcast2ucast_buffer] =
		WMI_PDEV_PARAM_SET_MCAST2UCAST_BUFFER;
	pdev_param[wmi_pdev_param_remove_mcast2ucast_buffer] =
		WMI_PDEV_PARAM_REMOVE_MCAST2UCAST_BUFFER;
	pdev_param[wmi_pdev_peer_sta_ps_statechg_enable] =
		WMI_PDEV_PEER_STA_PS_STATECHG_ENABLE;
	pdev_param[wmi_pdev_param_igmpmld_ac_override] =
		WMI_PDEV_PARAM_IGMPMLD_AC_OVERRIDE;
	pdev_param[wmi_pdev_param_block_interbss] =
		WMI_PDEV_PARAM_BLOCK_INTERBSS;
	pdev_param[wmi_pdev_param_set_disable_reset_cmdid] =
		WMI_PDEV_PARAM_SET_DISABLE_RESET_CMDID;
	pdev_param[wmi_pdev_param_set_msdu_ttl_cmdid] =
		WMI_PDEV_PARAM_SET_MSDU_TTL_CMDID;
	pdev_param[wmi_pdev_param_set_ppdu_duration_cmdid] =
		WMI_PDEV_PARAM_SET_PPDU_DURATION_CMDID;
	pdev_param[wmi_pdev_param_txbf_sound_period_cmdid] =
		WMI_PDEV_PARAM_TXBF_SOUND_PERIOD_CMDID;
	pdev_param[wmi_pdev_param_set_promisc_mode_cmdid] =
		WMI_PDEV_PARAM_SET_PROMISC_MODE_CMDID;
	pdev_param[wmi_pdev_param_set_burst_mode_cmdid] =
		WMI_PDEV_PARAM_SET_BURST_MODE_CMDID;
	pdev_param[wmi_pdev_param_en_stats] = WMI_PDEV_PARAM_EN_STATS;
	pdev_param[wmi_pdev_param_mu_group_policy] =
		WMI_PDEV_PARAM_MU_GROUP_POLICY;
	pdev_param[wmi_pdev_param_noise_detection] =
		WMI_PDEV_PARAM_NOISE_DETECTION;
	pdev_param[wmi_pdev_param_noise_threshold] =
		WMI_PDEV_PARAM_NOISE_THRESHOLD;
	pdev_param[wmi_pdev_param_dpd_enable] =
		WMI_PDEV_PARAM_DPD_ENABLE;
	pdev_param[wmi_pdev_param_set_mcast_bcast_echo] =
		WMI_PDEV_PARAM_SET_MCAST_BCAST_ECHO;
	pdev_param[wmi_pdev_param_atf_strict_sch] =
		WMI_PDEV_PARAM_ATF_STRICT_SCH;
	pdev_param[wmi_pdev_param_atf_sched_duration] =
		WMI_PDEV_PARAM_ATF_SCHED_DURATION;
	pdev_param[wmi_pdev_param_ant_plzn] = WMI_PDEV_PARAM_ANT_PLZN;
	pdev_param[wmi_pdev_param_mgmt_retry_limit] =
		WMI_PDEV_PARAM_MGMT_RETRY_LIMIT;
	pdev_param[wmi_pdev_param_sensitivity_level] =
		WMI_PDEV_PARAM_SENSITIVITY_LEVEL;
	pdev_param[wmi_pdev_param_signed_txpower_2g] =
		WMI_PDEV_PARAM_SIGNED_TXPOWER_2G;
	pdev_param[wmi_pdev_param_signed_txpower_5g] =
		WMI_PDEV_PARAM_SIGNED_TXPOWER_5G;
	pdev_param[wmi_pdev_param_enable_per_tid_amsdu] =
		WMI_PDEV_PARAM_ENABLE_PER_TID_AMSDU;
	pdev_param[wmi_pdev_param_enable_per_tid_ampdu] =
		WMI_PDEV_PARAM_ENABLE_PER_TID_AMPDU;
	pdev_param[wmi_pdev_param_cca_threshold] = WMI_PDEV_PARAM_CCA_THRESHOLD;
	pdev_param[wmi_pdev_param_rts_fixed_rate] =
		WMI_PDEV_PARAM_RTS_FIXED_RATE;
	pdev_param[wmi_pdev_param_cal_period] = WMI_PDEV_PARAM_CAL_PERIOD;
	pdev_param[wmi_pdev_param_pdev_reset] = WMI_PDEV_PARAM_PDEV_RESET;
	pdev_param[wmi_pdev_param_wapi_mbssid_offset] =
		WMI_PDEV_PARAM_WAPI_MBSSID_OFFSET;
	pdev_param[wmi_pdev_param_arp_srcaddr] = WMI_PDEV_PARAM_ARP_SRCADDR;
	pdev_param[wmi_pdev_param_arp_dstaddr] = WMI_PDEV_PARAM_ARP_DSTADDR;
	pdev_param[wmi_pdev_param_txpower_decr_db] =
		WMI_PDEV_PARAM_TXPOWER_DECR_DB;
	pdev_param[wmi_pdev_param_rx_batchmode] = WMI_PDEV_PARAM_RX_BATCHMODE;
	pdev_param[wmi_pdev_param_packet_aggr_delay] =
		WMI_PDEV_PARAM_PACKET_AGGR_DELAY;
	pdev_param[wmi_pdev_param_atf_obss_noise_sch] =
		WMI_PDEV_PARAM_ATF_OBSS_NOISE_SCH;
	pdev_param[wmi_pdev_param_atf_obss_noise_scaling_factor] =
		WMI_PDEV_PARAM_ATF_OBSS_NOISE_SCALING_FACTOR;
	pdev_param[wmi_pdev_param_cust_txpower_scale] =
		WMI_PDEV_PARAM_CUST_TXPOWER_SCALE;
	pdev_param[wmi_pdev_param_atf_dynamic_enable] =
		WMI_PDEV_PARAM_ATF_DYNAMIC_ENABLE;
	pdev_param[wmi_pdev_param_atf_ssid_group_policy] =
		WMI_PDEV_PARAM_ATF_SSID_GROUP_POLICY;
	pdev_param[wmi_pdev_param_enable_btcoex] =
		WMI_PDEV_PARAM_ENABLE_BTCOEX;
	pdev_param[wmi_pdev_param_atf_peer_stats] =
		WMI_PDEV_PARAM_ATF_PEER_STATS;
	pdev_param[wmi_pdev_param_tx_ack_timeout] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_soft_tx_chain_mask] =
		WMI_PDEV_PARAM_SOFT_TX_CHAIN_MASK;
	pdev_param[wmi_pdev_param_rfkill_enable] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_hw_rfkill_config] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_low_power_rf_enable] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_l1ss_track] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_hyst_en] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_power_collapse_enable] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_led_sys_state] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_led_enable] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_audio_over_wlan_latency] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_audio_over_wlan_enable] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_whal_mib_stats_update_enable] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_vdev_rate_stats_update_period] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_cts_cbw] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_wnts_config] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_adaptive_early_rx_enable] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_adaptive_early_rx_min_sleep_slop] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_adaptive_early_rx_inc_dec_step] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_early_rx_fix_sleep_slop] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_bmiss_based_adaptive_bto_enable] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_bmiss_bto_min_bcn_timeout] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_bmiss_bto_inc_dec_step] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_bto_fix_bcn_timeout] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_ce_based_adaptive_bto_enable] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_ce_bto_combo_ce_value] =
		WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_tx_chain_mask_2g] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_rx_chain_mask_2g] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_tx_chain_mask_5g] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_rx_chain_mask_5g] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_tx_chain_mask_cck] = WMI_UNAVAILABLE_PARAM;
	pdev_param[wmi_pdev_param_tx_chain_mask_1ss] = WMI_UNAVAILABLE_PARAM;
}

/**
 * populate_vdev_param_non_tlv() - populates vdev params
 *
 * @param vdev_param: Pointer to hold vdev params
 * Return: None
 */
static void populate_vdev_param_non_tlv(uint32_t *vdev_param)
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
	vdev_param[wmi_vdev_param_mgmt_tx_rate] =
			WMI_VDEV_PARAM_MGMT_TX_RATE;
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
	vdev_param[wmi_vdev_param_dhcp_indicate] = WMI_VDEV_PARAM_DHCP_INDICATE;
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
	vdev_param[wmi_vdev_param_mcast2ucast_set] =
		WMI_VDEV_PARAM_MCAST2UCAST_SET;
	vdev_param[wmi_vdev_param_enable_rtscts] = WMI_VDEV_PARAM_ENABLE_RTSCTS;
	vdev_param[wmi_vdev_param_rc_num_retries] =
		WMI_VDEV_PARAM_RC_NUM_RETRIES;
	vdev_param[wmi_vdev_param_txbf] = WMI_VDEV_PARAM_TXBF;
	vdev_param[wmi_vdev_param_packet_powersave] =
		WMI_VDEV_PARAM_PACKET_POWERSAVE;
	vdev_param[wmi_vdev_param_drop_unencry] = WMI_VDEV_PARAM_DROP_UNENCRY;
	vdev_param[wmi_vdev_param_tx_encap_type] = WMI_VDEV_PARAM_TX_ENCAP_TYPE;
	vdev_param[wmi_vdev_param_ap_detect_out_of_sync_sleeping_sta_time_secs]
		= WMI_VDEV_PARAM_AP_DETECT_OUT_OF_SYNC_SLEEPING_STA_TIME_SECS;
	vdev_param[wmi_vdev_param_cabq_maxdur] = WMI_VDEV_PARAM_CABQ_MAXDUR;
	vdev_param[wmi_vdev_param_mfptest_set] = WMI_VDEV_PARAM_MFPTEST_SET;
	vdev_param[wmi_vdev_param_rts_fixed_rate] =
		WMI_VDEV_PARAM_RTS_FIXED_RATE;
	vdev_param[wmi_vdev_param_vht_sgimask] = WMI_VDEV_PARAM_VHT_SGIMASK;
	vdev_param[wmi_vdev_param_vht80_ratemask] =
		WMI_VDEV_PARAM_VHT80_RATEMASK;
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
	vdev_param[wmi_vdev_param_proxy_sta] = WMI_VDEV_PARAM_PROXY_STA;
	vdev_param[wmi_vdev_param_meru_vc] = WMI_VDEV_PARAM_MERU_VC;
	vdev_param[wmi_vdev_param_rx_decap_type] = WMI_VDEV_PARAM_RX_DECAP_TYPE;
	vdev_param[wmi_vdev_param_bw_nss_ratemask] =
		WMI_VDEV_PARAM_BW_NSS_RATEMASK;
	vdev_param[wmi_vdev_param_sensor_ap] = WMI_VDEV_PARAM_SENSOR_AP;
	vdev_param[wmi_vdev_param_beacon_rate] = WMI_VDEV_PARAM_BEACON_RATE;
	vdev_param[wmi_vdev_param_dtim_enable_cts] =
		WMI_VDEV_PARAM_DTIM_ENABLE_CTS;
	vdev_param[wmi_vdev_param_sta_kickout] = WMI_VDEV_PARAM_STA_KICKOUT;
	vdev_param[wmi_vdev_param_capabilities] =
		WMI_VDEV_PARAM_CAPABILITIES;
	vdev_param[wmi_vdev_param_mgmt_tx_power] = WMI_VDEV_PARAM_MGMT_TX_POWER;
	vdev_param[wmi_vdev_param_tx_power] = WMI_VDEV_PARAM_TX_POWER;
	vdev_param[wmi_vdev_param_atf_ssid_sched_policy] =
		WMI_VDEV_PARAM_ATF_SSID_SCHED_POLICY;
	vdev_param[wmi_vdev_param_disable_dyn_bw_rts] =
		WMI_VDEV_PARAM_DISABLE_DYN_BW_RTS;
	vdev_param[wmi_vdev_param_ampdu_subframe_size_per_ac] =
		WMI_VDEV_PARAM_AMPDU_SUBFRAME_SIZE_PER_AC;
	vdev_param[wmi_vdev_param_disable_cabq] =
		WMI_VDEV_PARAM_DISABLE_CABQ;
}
#endif

/**
 * wmi_get_non_tlv_ops() - gives pointer to wmi tlv ops
 *
 * Return: pointer to wmi tlv ops
 */
void wmi_non_tlv_attach(struct wmi_unified *wmi_handle)
{
#if defined(WMI_NON_TLV_SUPPORT) || defined(WMI_TLV_AND_NON_TLV_SUPPORT)
	wmi_handle->ops = &non_tlv_ops;
	wmi_handle->soc->svc_ids = &svc_ids[0];
	populate_non_tlv_service(wmi_handle->services);
	populate_non_tlv_events_id(wmi_handle->wmi_events);
	populate_pdev_param_non_tlv(wmi_handle->pdev_param);
	populate_vdev_param_non_tlv(wmi_handle->vdev_param);

#ifdef WMI_INTERFACE_EVENT_LOGGING
	wmi_handle->log_info.buf_offset_command = 0;
	wmi_handle->log_info.buf_offset_event = 0;
	/*(uint8 *)(*wmi_id_to_name)(uint32_t cmd_id);*/
#endif
#else
	qdf_print("%s: Not supported\n", __func__);
#endif
}
EXPORT_SYMBOL(wmi_non_tlv_attach);

/**
 * wmi_non_tlv_init() - Initialize WMI NON TLV module by registering Non TLV
 * attach routine.
 *
 * Return: None
 */
void wmi_non_tlv_init(void)
{
	wmi_unified_register_module(WMI_NON_TLV_TARGET, &wmi_non_tlv_attach);
}
