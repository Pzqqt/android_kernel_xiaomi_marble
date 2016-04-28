/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

/**
 *  DOC:  wma_main.c
 *
 *  This file contains wma initialization and FW exchange
 *  related functions.
 */

/* Header files */

#include "wma.h"
#include "wma_api.h"
#include "cds_api.h"
#include "wmi_unified_api.h"
#include "wlan_qct_sys.h"
#include "wni_api.h"
#include "ani_global.h"
#include "wmi_unified.h"
#include "wni_cfg.h"
#include "cfg_api.h"
#include "wlan_tgt_def_config.h"

#include "qdf_nbuf.h"
#include "qdf_types.h"
#include "qdf_mem.h"
#include "ol_txrx_peer_find.h"

#include "wma_types.h"
#include "lim_api.h"
#include "lim_session_utils.h"

#include "cds_utils.h"

#if !defined(REMOVE_PKT_LOG)
#include "pktlog_ac.h"
#endif /* REMOVE_PKT_LOG */

#include "dbglog_host.h"
#include "wmi_version_whitelist.h"
#include "csr_api.h"
#include "ol_fw.h"

#include "dfs.h"
#include "wma_internal.h"

#include "wma_ocb.h"
#include "cds_concurrency.h"
#include "cdp_txrx_cfg.h"
#include "cdp_txrx_flow_ctrl_legacy.h"
#include "cdp_txrx_flow_ctrl_v2.h"
#include "cdp_txrx_ipa.h"

#define WMA_LOG_COMPLETION_TIMER 10000 /* 10 seconds */

static uint32_t g_fw_wlan_feat_caps;

/**
 * wma_get_fw_wlan_feat_caps() - get fw feature capablity
 * @featEnumValue: feature enum value
 *
 * Return: true/false
 */
uint8_t wma_get_fw_wlan_feat_caps(uint8_t featEnumValue)
{
	return (g_fw_wlan_feat_caps & (1 << featEnumValue)) ? true : false;
}

/**
 * wma_service_ready_ext_evt_timeout() - Service ready extended event timeout
 * @data: Timeout handler data
 *
 * This function is called when the FW fails to send WMI_SERVICE_READY_EXT_EVENT
 * message
 *
 * Return: None
 */
static void wma_service_ready_ext_evt_timeout(void *data)
{
	tp_wma_handle wma_handle;

	WMA_LOGA("%s: Timeout waiting for WMI_SERVICE_READY_EXT_EVENT",
			__func__);

	wma_handle = (tp_wma_handle) data;

	if (!wma_handle) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		goto end;
	}

end:
	/* Panic so that we can debug why FW is not responding */
	QDF_BUG(0);
}

/**
 * wma_get_ini_handle() - API to get WMA ini info handle
 * @wma: WMA Handle
 *
 * Returns the pointer to WMA ini structure.
 * Return: struct wma_ini_config
 */
struct wma_ini_config *wma_get_ini_handle(tp_wma_handle wma)
{
	if (!wma) {
		WMA_LOGE("%s: Invalid WMA context\n", __func__);
		return NULL;
	}

	return &wma->ini_config;
}

#define MAX_SUPPORTED_PEERS_REV1_1 14
#define MAX_SUPPORTED_PEERS_REV1_3 32
#define MIN_NO_OF_PEERS 1

/**
 * wma_get_number_of_peers_supported - API to query for number of peers
 * supported
 * @wma: WMA Handle
 *
 * Return: Max Number of Peers Supported
 */
static uint8_t wma_get_number_of_peers_supported(tp_wma_handle wma)
{
	struct hif_target_info *tgt_info;
	struct wma_ini_config *cfg = wma_get_ini_handle(wma);
	uint8_t max_no_of_peers = cfg ? cfg->max_no_of_peers : MIN_NO_OF_PEERS;
	struct hif_opaque_softc *scn = cds_get_context(QDF_MODULE_ID_HIF);

	if (!scn) {
		WMA_LOGE("%s: Invalid wma handle", __func__);
		return 0;
	}

	tgt_info = hif_get_target_info_handle(scn);

	switch (tgt_info->target_version) {
	case AR6320_REV1_1_VERSION:
		if (max_no_of_peers > MAX_SUPPORTED_PEERS_REV1_1)
			max_no_of_peers = MAX_SUPPORTED_PEERS_REV1_1;
		break;
	default:
		if (max_no_of_peers > MAX_SUPPORTED_PEERS_REV1_3)
			max_no_of_peers = MAX_SUPPORTED_PEERS_REV1_3;
		break;
	}

	return max_no_of_peers;
}

/**
 * wma_set_default_tgt_config() - set default tgt config
 * @wma_handle: wma handle
 *
 * Return: none
 */
static void wma_set_default_tgt_config(tp_wma_handle wma_handle)
{
	uint8_t no_of_peers_supported;
	wmi_resource_config tgt_cfg = {
		0,              /* Filling zero for TLV Tag and Length fields */
		CFG_TGT_NUM_VDEV,
		CFG_TGT_NUM_PEERS + CFG_TGT_NUM_VDEV + 2,
		CFG_TGT_NUM_OFFLOAD_PEERS,
		CFG_TGT_NUM_OFFLOAD_REORDER_BUFFS,
		CFG_TGT_NUM_PEER_KEYS,
		CFG_TGT_NUM_TIDS,
		CFG_TGT_AST_SKID_LIMIT,
		CFG_TGT_DEFAULT_TX_CHAIN_MASK,
		CFG_TGT_DEFAULT_RX_CHAIN_MASK,
		{CFG_TGT_RX_TIMEOUT_LO_PRI, CFG_TGT_RX_TIMEOUT_LO_PRI,
		 CFG_TGT_RX_TIMEOUT_LO_PRI, CFG_TGT_RX_TIMEOUT_HI_PRI},
		CFG_TGT_RX_DECAP_MODE,
		CFG_TGT_DEFAULT_SCAN_MAX_REQS,
		CFG_TGT_DEFAULT_BMISS_OFFLOAD_MAX_VDEV,
		CFG_TGT_DEFAULT_ROAM_OFFLOAD_MAX_VDEV,
		CFG_TGT_DEFAULT_ROAM_OFFLOAD_MAX_PROFILES,
		CFG_TGT_DEFAULT_NUM_MCAST_GROUPS,
		CFG_TGT_DEFAULT_NUM_MCAST_TABLE_ELEMS,
		CFG_TGT_DEFAULT_MCAST2UCAST_MODE,
		CFG_TGT_DEFAULT_TX_DBG_LOG_SIZE,
		CFG_TGT_WDS_ENTRIES,
		CFG_TGT_DEFAULT_DMA_BURST_SIZE,
		CFG_TGT_DEFAULT_MAC_AGGR_DELIM,
		CFG_TGT_DEFAULT_RX_SKIP_DEFRAG_TIMEOUT_DUP_DETECTION_CHECK,
		CFG_TGT_DEFAULT_VOW_CONFIG,
		CFG_TGT_DEFAULT_GTK_OFFLOAD_MAX_VDEV,
		CFG_TGT_NUM_MSDU_DESC,
		CFG_TGT_MAX_FRAG_TABLE_ENTRIES,
		CFG_TGT_NUM_TDLS_VDEVS,
		CFG_TGT_NUM_TDLS_CONN_TABLE_ENTRIES,
		CFG_TGT_DEFAULT_BEACON_TX_OFFLOAD_MAX_VDEV,
		CFG_TGT_MAX_MULTICAST_FILTER_ENTRIES,
		0,
		0,
		0,
		CFG_TGT_NUM_TDLS_CONC_SLEEP_STAS,
		CFG_TGT_NUM_TDLS_CONC_BUFFER_STAS,
		0,
		CFG_TGT_NUM_OCB_VDEVS,
		CFG_TGT_NUM_OCB_CHANNELS,
		CFG_TGT_NUM_OCB_SCHEDULES,
	};

	no_of_peers_supported = wma_get_number_of_peers_supported(wma_handle);
	tgt_cfg.num_peers = no_of_peers_supported + CFG_TGT_NUM_VDEV + 2;
	tgt_cfg.num_tids = (2 * (no_of_peers_supported + CFG_TGT_NUM_VDEV + 2));
	tgt_cfg.scan_max_pending_req = wma_handle->max_scan;

	WMITLV_SET_HDR(&tgt_cfg.tlv_header,
		       WMITLV_TAG_STRUC_wmi_resource_config,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_resource_config));
	/* reduce the peer/vdev if CFG_TGT_NUM_MSDU_DESC exceeds 1000 */
#ifdef PERE_IP_HDR_ALIGNMENT_WAR
	if (scn->host_80211_enable) {
		/*
		 * To make the IP header begins at dword aligned address,
		 * we make the decapsulation mode as Native Wifi.
		 */
		tgt_cfg.rx_decap_mode = CFG_TGT_RX_DECAP_MODE_NWIFI;
	}
#endif /* PERE_IP_HDR_ALIGNMENT_WAR */
	if (QDF_GLOBAL_MONITOR_MODE == cds_get_conparam())
		tgt_cfg.rx_decap_mode = CFG_TGT_RX_DECAP_MODE_RAW;

	wma_handle->wlan_resource_config = tgt_cfg;
}

/**
 * wma_cli_get_command() - WMA "get" command processor
 * @vdev_id: virtual device for the command
 * @param_id: parameter id
 * @vpdev: parameter category
 *
 * Return: parameter value on success, -EINVAL on failure
 */
int wma_cli_get_command(int vdev_id, int param_id, int vpdev)
{
	int ret = 0;
	tp_wma_handle wma;
	struct wma_txrx_node *intr = NULL;

	wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wma) {
		WMA_LOGE("%s: Invalid wma handle", __func__);
		return -EINVAL;
	}

	intr = wma->interfaces;

	if (VDEV_CMD == vpdev) {
		switch (param_id) {
		case WMI_VDEV_PARAM_NSS:
			ret = intr[vdev_id].config.nss;
			break;
#ifdef QCA_SUPPORT_GTX
		case WMI_VDEV_PARAM_GTX_HT_MCS:
			ret = intr[vdev_id].config.gtx_info.gtxRTMask[0];
			break;
		case WMI_VDEV_PARAM_GTX_VHT_MCS:
			ret = intr[vdev_id].config.gtx_info.gtxRTMask[1];
			break;
		case WMI_VDEV_PARAM_GTX_USR_CFG:
			ret = intr[vdev_id].config.gtx_info.gtxUsrcfg;
			break;
		case WMI_VDEV_PARAM_GTX_THRE:
			ret = intr[vdev_id].config.gtx_info.gtxPERThreshold;
			break;
		case WMI_VDEV_PARAM_GTX_MARGIN:
			ret = intr[vdev_id].config.gtx_info.gtxPERMargin;
			break;
		case WMI_VDEV_PARAM_GTX_STEP:
			ret = intr[vdev_id].config.gtx_info.gtxTPCstep;
			break;
		case WMI_VDEV_PARAM_GTX_MINTPC:
			ret = intr[vdev_id].config.gtx_info.gtxTPCMin;
			break;
		case WMI_VDEV_PARAM_GTX_BW_MASK:
			ret = intr[vdev_id].config.gtx_info.gtxBWMask;
			break;
#endif /* QCA_SUPPORT_GTX */
		case WMI_VDEV_PARAM_LDPC:
			ret = intr[vdev_id].config.ldpc;
			break;
		case WMI_VDEV_PARAM_TX_STBC:
			ret = intr[vdev_id].config.tx_stbc;
			break;
		case WMI_VDEV_PARAM_RX_STBC:
			ret = intr[vdev_id].config.rx_stbc;
			break;
		case WMI_VDEV_PARAM_SGI:
			ret = intr[vdev_id].config.shortgi;
			break;
		case WMI_VDEV_PARAM_ENABLE_RTSCTS:
			ret = intr[vdev_id].config.rtscts_en;
			break;
		case WMI_VDEV_PARAM_CHWIDTH:
			ret = intr[vdev_id].config.chwidth;
			break;
		case WMI_VDEV_PARAM_FIXED_RATE:
			ret = intr[vdev_id].config.tx_rate;
			break;
		default:
			WMA_LOGE("Invalid cli_get vdev command/Not"
				 " yet implemented 0x%x", param_id);
			return -EINVAL;
		}
	} else if (PDEV_CMD == vpdev) {
		switch (param_id) {
		case WMI_PDEV_PARAM_ANI_ENABLE:
			ret = wma->pdevconfig.ani_enable;
			break;
		case WMI_PDEV_PARAM_ANI_POLL_PERIOD:
			ret = wma->pdevconfig.ani_poll_len;
			break;
		case WMI_PDEV_PARAM_ANI_LISTEN_PERIOD:
			ret = wma->pdevconfig.ani_listen_len;
			break;
		case WMI_PDEV_PARAM_ANI_OFDM_LEVEL:
			ret = wma->pdevconfig.ani_ofdm_level;
			break;
		case WMI_PDEV_PARAM_ANI_CCK_LEVEL:
			ret = wma->pdevconfig.ani_cck_level;
			break;
		case WMI_PDEV_PARAM_DYNAMIC_BW:
			ret = wma->pdevconfig.cwmenable;
			break;
		case WMI_PDEV_PARAM_CTS_CBW:
			ret = wma->pdevconfig.cts_cbw;
			break;
		case WMI_PDEV_PARAM_TX_CHAIN_MASK:
			ret = wma->pdevconfig.txchainmask;
			break;
		case WMI_PDEV_PARAM_RX_CHAIN_MASK:
			ret = wma->pdevconfig.rxchainmask;
			break;
		case WMI_PDEV_PARAM_TXPOWER_LIMIT2G:
			ret = wma->pdevconfig.txpow2g;
			break;
		case WMI_PDEV_PARAM_TXPOWER_LIMIT5G:
			ret = wma->pdevconfig.txpow5g;
			break;
		case WMI_PDEV_PARAM_BURST_ENABLE:
			ret = wma->pdevconfig.burst_enable;
			break;
		case WMI_PDEV_PARAM_BURST_DUR:
			ret = wma->pdevconfig.burst_dur;
			break;
		default:
			WMA_LOGE("Invalid cli_get pdev command/Not"
				 " yet implemented 0x%x", param_id);
			return -EINVAL;
		}
	} else if (GEN_CMD == vpdev) {
		switch (param_id) {
		case GEN_VDEV_PARAM_AMPDU:
			ret = intr[vdev_id].config.ampdu;
			break;
		case GEN_VDEV_PARAM_AMSDU:
			ret = intr[vdev_id].config.amsdu;
			break;
		case GEN_VDEV_ROAM_SYNCH_DELAY:
			ret = intr[vdev_id].roam_synch_delay;
			break;
		default:
			WMA_LOGE("Invalid generic vdev command/Not"
				 " yet implemented 0x%x", param_id);
			return -EINVAL;
		}
	} else if (PPS_CMD == vpdev) {
		switch (param_id) {
		case WMI_VDEV_PPS_PAID_MATCH:
			ret = intr[vdev_id].config.pps_params.paid_match_enable;
			break;
		case WMI_VDEV_PPS_GID_MATCH:
			ret = intr[vdev_id].config.pps_params.gid_match_enable;
			break;
		case WMI_VDEV_PPS_EARLY_TIM_CLEAR:
			ret = intr[vdev_id].config.pps_params.tim_clear;
			break;
		case WMI_VDEV_PPS_EARLY_DTIM_CLEAR:
			ret = intr[vdev_id].config.pps_params.dtim_clear;
			break;
		case WMI_VDEV_PPS_EOF_PAD_DELIM:
			ret = intr[vdev_id].config.pps_params.eof_delim;
			break;
		case WMI_VDEV_PPS_MACADDR_MISMATCH:
			ret = intr[vdev_id].config.pps_params.mac_match;
			break;
		case WMI_VDEV_PPS_DELIM_CRC_FAIL:
			ret = intr[vdev_id].config.pps_params.delim_fail;
			break;
		case WMI_VDEV_PPS_GID_NSTS_ZERO:
			ret = intr[vdev_id].config.pps_params.nsts_zero;
			break;
		case WMI_VDEV_PPS_RSSI_CHECK:
			ret = intr[vdev_id].config.pps_params.rssi_chk;
			break;
		default:
			WMA_LOGE("Invalid pps vdev command/Not"
				 " yet implemented 0x%x", param_id);
			return -EINVAL;
		}
	} else if (QPOWER_CMD == vpdev) {
		switch (param_id) {
		case WMI_STA_PS_PARAM_QPOWER_PSPOLL_COUNT:
			ret = intr[vdev_id].config.qpower_params.
			      max_ps_poll_cnt;
			break;
		case WMI_STA_PS_PARAM_QPOWER_MAX_TX_BEFORE_WAKE:
			ret = intr[vdev_id].config.qpower_params.
			      max_tx_before_wake;
			break;
		case WMI_STA_PS_PARAM_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL:
			ret = intr[vdev_id].config.qpower_params.
			      spec_ps_poll_wake_interval;
			break;
		case WMI_STA_PS_PARAM_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL:
			ret = intr[vdev_id].config.qpower_params.
			      max_spec_nodata_ps_poll;
			break;
		default:
			WMA_LOGE("Invalid generic vdev command/Not"
				 " yet implemented 0x%x", param_id);
			return -EINVAL;
		}
	} else if (GTX_CMD == vpdev) {
		switch (param_id) {
		case WMI_VDEV_PARAM_GTX_HT_MCS:
			ret = intr[vdev_id].config.gtx_info.gtxRTMask[0];
			break;
		case WMI_VDEV_PARAM_GTX_VHT_MCS:
			ret = intr[vdev_id].config.gtx_info.gtxRTMask[1];
			break;
		case WMI_VDEV_PARAM_GTX_USR_CFG:
			ret = intr[vdev_id].config.gtx_info.gtxUsrcfg;
			break;
		case WMI_VDEV_PARAM_GTX_THRE:
			ret = intr[vdev_id].config.gtx_info.gtxPERThreshold;
			break;
		case WMI_VDEV_PARAM_GTX_MARGIN:
			ret = intr[vdev_id].config.gtx_info.gtxPERMargin;
			break;
		case WMI_VDEV_PARAM_GTX_STEP:
			ret = intr[vdev_id].config.gtx_info.gtxTPCstep;
			break;
		case WMI_VDEV_PARAM_GTX_MINTPC:
			ret = intr[vdev_id].config.gtx_info.gtxTPCMin;
			break;
		case WMI_VDEV_PARAM_GTX_BW_MASK:
			ret = intr[vdev_id].config.gtx_info.gtxBWMask;
			break;
		default:
			WMA_LOGE("Invalid generic vdev command/Not"
				 " yet implemented 0x%x", param_id);
			return -EINVAL;
		}
	}
	return ret;
}

/**
 * wma_cli_set2_command() - WMA "set 2 params" command processor
 * @vdev_id: virtual device for the command
 * @param_id: parameter id
 * @sval1: first parameter value
 * @sval2: second parameter value
 * @vpdev: parameter category
 *
 * Command handler for set operations which require 2 parameters
 *
 * Return: 0 on success, errno on failure
 */
int wma_cli_set2_command(int vdev_id, int param_id, int sval1,
			 int sval2, int vpdev)
{
	cds_msg_t msg = { 0 };
	wma_cli_set_cmd_t *iwcmd;

	iwcmd = qdf_mem_malloc(sizeof(*iwcmd));
	if (!iwcmd) {
		WMA_LOGE("%s: Failed alloc memory for iwcmd", __func__);
		return -ENOMEM;
	}

	qdf_mem_zero(iwcmd, sizeof(*iwcmd));
	iwcmd->param_value = sval1;
	iwcmd->param_sec_value = sval2;
	iwcmd->param_vdev_id = vdev_id;
	iwcmd->param_id = param_id;
	iwcmd->param_vp_dev = vpdev;
	msg.type = WMA_CLI_SET_CMD;
	msg.reserved = 0;
	msg.bodyptr = iwcmd;

	if (QDF_STATUS_SUCCESS !=
	    cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		WMA_LOGP("%s: Failed to post WMA_CLI_SET_CMD msg",
			  __func__);
		qdf_mem_free(iwcmd);
		return -EIO;
	}
	return 0;
}

/**
 * wma_cli_set_command() - WMA "set" command processor
 * @vdev_id: virtual device for the command
 * @param_id: parameter id
 * @sval: parameter value
 * @vpdev: parameter category
 *
 * Command handler for set operations
 *
 * Return: 0 on success, errno on failure
 */
int wma_cli_set_command(int vdev_id, int param_id, int sval, int vpdev)
{
	return wma_cli_set2_command(vdev_id, param_id, sval, 0, vpdev);

}

/**
 * wma_set_priv_cfg() - set private config parameters
 * @wma_handle: wma handle
 * @privcmd: private command
 *
 * Return: 0 for success or error code
 */
static int32_t wma_set_priv_cfg(tp_wma_handle wma_handle,
				wma_cli_set_cmd_t *privcmd)
{
	int32_t ret = 0;

	switch (privcmd->param_id) {
	case WMA_VDEV_TXRX_FWSTATS_ENABLE_CMDID:
		ret = wma_set_txrx_fw_stats_level(wma_handle,
						  privcmd->param_vdev_id,
						  privcmd->param_value);
		break;
	case WMA_VDEV_TXRX_FWSTATS_RESET_CMDID:
		ret = wma_txrx_fw_stats_reset(wma_handle,
					      privcmd->param_vdev_id,
					      privcmd->param_value);
		break;
	case WMI_STA_SMPS_FORCE_MODE_CMDID:
		ret = wma_set_mimops(wma_handle,
				     privcmd->param_vdev_id,
				     privcmd->param_value);
		break;
	case WMI_STA_SMPS_PARAM_CMDID:
		wma_set_smps_params(wma_handle, privcmd->param_vdev_id,
				    privcmd->param_value);
		break;
	case WMA_VDEV_MCC_SET_TIME_LATENCY:
	{
		/* Extract first MCC adapter/vdev channel number and latency */
		uint8_t mcc_channel = privcmd->param_value & 0x000000FF;
		uint8_t mcc_channel_latency =
			(privcmd->param_value & 0x0000FF00) >> 8;
		int ret = -1;
		WMA_LOGD("%s: Parsed input: Channel #1:%d, latency:%dms",
			__func__, mcc_channel, mcc_channel_latency);
		ret = wma_set_mcc_channel_time_latency(wma_handle,
						       mcc_channel,
						       mcc_channel_latency);
	}
		break;
	case WMA_VDEV_MCC_SET_TIME_QUOTA:
	{
		/* Extract the MCC 2 adapters/vdevs channel numbers and time
		 * quota value for the first adapter only (which is specified
		 * in iwpriv command.
		 */
		uint8_t adapter_2_chan_number =
			privcmd->param_value & 0x000000FF;
		uint8_t adapter_1_chan_number =
			(privcmd->param_value & 0x0000FF00) >> 8;
		uint8_t adapter_1_quota =
			(privcmd->param_value & 0x00FF0000) >> 16;
		int ret = -1;

		WMA_LOGD("%s: Parsed input: Channel #1:%d, Channel #2:%d, quota 1:%dms",
			  __func__, adapter_1_chan_number,
			 adapter_2_chan_number, adapter_1_quota);

		ret = wma_set_mcc_channel_time_quota(wma_handle,
						     adapter_1_chan_number,
						     adapter_1_quota,
						     adapter_2_chan_number);
	}
		break;
	case WMA_VDEV_IBSS_SET_ATIM_WINDOW_SIZE:
	{
		wma_handle->wma_ibss_power_save_params.atimWindowLength =
							privcmd->param_value;
		WMA_LOGD("%s: IBSS power save ATIM Window = %d",
			 __func__, wma_handle->wma_ibss_power_save_params.
			 atimWindowLength);
	}
		break;
	case WMA_VDEV_IBSS_SET_POWER_SAVE_ALLOWED:
	{
		wma_handle->wma_ibss_power_save_params.isPowerSaveAllowed =
							privcmd->param_value;
		WMA_LOGD("%s: IBSS is Power Save Allowed = %d",
			 __func__, wma_handle->wma_ibss_power_save_params.
			 isPowerSaveAllowed);
	}
		break;
	case WMA_VDEV_IBSS_SET_POWER_COLLAPSE_ALLOWED:
	{
		wma_handle->wma_ibss_power_save_params.	isPowerCollapseAllowed =
							 privcmd->param_value;
		WMA_LOGD("%s: IBSS is Power Collapse Allowed = %d",
			 __func__, wma_handle->wma_ibss_power_save_params.
			 isPowerCollapseAllowed);
	}
		break;
	case WMA_VDEV_IBSS_SET_AWAKE_ON_TX_RX:
	{
		wma_handle->wma_ibss_power_save_params.isAwakeonTxRxEnabled =
							 privcmd->param_value;
		WMA_LOGD("%s: IBSS Power Save Awake on Tx/Rx Enabled = %d",
			__func__, wma_handle->wma_ibss_power_save_params.
			isAwakeonTxRxEnabled);
	}
		break;
	case WMA_VDEV_IBSS_SET_INACTIVITY_TIME:
	{
		wma_handle->wma_ibss_power_save_params.inactivityCount =
							privcmd->param_value;
		WMA_LOGD("%s: IBSS Power Save Data Inactivity Count = %d",
			__func__, wma_handle->wma_ibss_power_save_params.
			inactivityCount);
	}
		break;
	case WMA_VDEV_IBSS_SET_TXSP_END_INACTIVITY_TIME:
	{
		wma_handle->wma_ibss_power_save_params.txSPEndInactivityTime =
							 privcmd->param_value;
		WMA_LOGD("%s: IBSS Power Save Transmit EOSP inactivity time out = %d",
			__func__, wma_handle->wma_ibss_power_save_params.
			txSPEndInactivityTime);
	}
		break;
	case WMA_VDEV_DFS_CONTROL_CMDID:
	{
		struct ieee80211com *dfs_ic = wma_handle->dfs_ic;
		struct ath_dfs *dfs;

		if (!dfs_ic) {
			ret = -ENOENT;
		} else {
			if (dfs_ic->ic_curchan) {
				WMA_LOGD("%s: Debug cmd: %s received on ch: %d",
					__func__, "WMA_VDEV_DFS_CONTROL_CMDID",
					dfs_ic->ic_curchan->ic_ieee);

				if (dfs_ic->ic_curchan->ic_flagext &
				    IEEE80211_CHAN_DFS) {
					dfs = (struct ath_dfs *)dfs_ic->ic_dfs;
					dfs->dfs_bangradar = 1;
					dfs->ath_radar_tasksched = 1;
					OS_SET_TIMER(&dfs->ath_dfs_task_timer,
						     0);
				} else {
					ret = -ENOENT;
				}
			} else {
				ret = -ENOENT;
			}
		}

		if (ret == -ENOENT) {
			WMA_LOGE("%s: Operating channel is not DFS capable,ignoring %s",
				  __func__, "WMA_VDEV_DFS_CONTROL_CMDID");
		} else if (ret) {
			WMA_LOGE("%s: Sending command %s failed with %d\n",
				__func__, "WMA_VDEV_DFS_CONTROL_CMDID",
				ret);
		}
	}
		break;
	case WMA_VDEV_IBSS_PS_SET_WARMUP_TIME_SECS:
	{
		wma_handle->wma_ibss_power_save_params.ibssPsWarmupTime =
							privcmd->param_value;
		WMA_LOGD("%s: IBSS Power Save Warm Up Time in Seconds = %d",
			__func__, wma_handle->wma_ibss_power_save_params.
			ibssPsWarmupTime);
	}
		break;
	case WMA_VDEV_IBSS_PS_SET_1RX_CHAIN_IN_ATIM_WINDOW:
	{
		wma_handle->wma_ibss_power_save_params.ibssPs1RxChainInAtimEnable
							 = privcmd->param_value;
		WMA_LOGD("%s: IBSS Power Save single RX Chain Enable In ATIM  = %d",
			__func__, wma_handle->wma_ibss_power_save_params.
			ibssPs1RxChainInAtimEnable);
	}
		break;

	case WMA_VDEV_TXRX_GET_IPA_UC_FW_STATS_CMDID:
	{
		ol_txrx_pdev_handle pdev;

		pdev = cds_get_context(QDF_MODULE_ID_TXRX);
		if (!pdev) {
			WMA_LOGE("pdev NULL for uc stat");
			return -EINVAL;
		}
		ol_txrx_ipa_uc_get_stat(pdev);
	}
		break;

	default:
		WMA_LOGE("Invalid wma config command id:%d", privcmd->param_id);
		ret = -EINVAL;
	}
	return ret;
}

/**
 * wma_set_modulated_dtim() - function to configure modulated dtim
 * @wma: wma handle
 * @privcmd: structure containing parameters
 *
 * This function configures the modulated dtim in firmware
 *
 * Return: none
 */
static void wma_set_modulated_dtim(tp_wma_handle wma,
				   wma_cli_set_cmd_t *privcmd)
{
	uint8_t vdev_id = privcmd->param_vdev_id;
	struct wma_txrx_node *iface =
		&wma->interfaces[vdev_id];
	bool prev_dtim_enabled;
	uint32_t listen_interval;
	QDF_STATUS ret;

	iface->alt_modulated_dtim = privcmd->param_value;

	prev_dtim_enabled = iface->alt_modulated_dtim_enabled;

	if (1 != privcmd->param_value)
		iface->alt_modulated_dtim_enabled = true;
	else
		iface->alt_modulated_dtim_enabled = false;

	if ((true == iface->alt_modulated_dtim_enabled) ||
	    (true == prev_dtim_enabled)) {

		listen_interval = iface->alt_modulated_dtim
			* iface->dtimPeriod;

		ret = wma_vdev_set_param(wma->wmi_handle,
						privcmd->param_vdev_id,
						WMI_VDEV_PARAM_LISTEN_INTERVAL,
						listen_interval);
		if (QDF_IS_STATUS_ERROR(ret))
			/* Even if it fails, continue */
			WMA_LOGW("Failed to set listen interval %d",
				 listen_interval);

		ret = wma_vdev_set_param(wma->wmi_handle,
						privcmd->param_vdev_id,
						WMI_VDEV_PARAM_DTIM_POLICY ,
						NORMAL_DTIM);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to Set to Normal DTIM policy");
	}
}


/**
 * wma_process_cli_set_cmd() - set parameters to fw
 * @wma: wma handle
 * @privcmd: command
 *
 * Return: none
 */
static void wma_process_cli_set_cmd(tp_wma_handle wma,
				    wma_cli_set_cmd_t *privcmd)
{
	int vid = privcmd->param_vdev_id, pps_val = 0;
	QDF_STATUS ret;
	struct wma_txrx_node *intr = wma->interfaces;
	tpAniSirGlobal pMac = cds_get_context(QDF_MODULE_ID_PE);
	struct qpower_params *qparams = &intr[vid].config.qpower_params;
	struct pdev_params pdev_param;

	WMA_LOGD("wmihandle %p", wma->wmi_handle);

	if (NULL == pMac) {
		WMA_LOGE("%s: Failed to get pMac", __func__);
		return;
	}

	if (privcmd->param_id >= WMI_CMDID_MAX) {
		/*
		 * This configuration setting is not done using any wmi
		 * command, call appropriate handler.
		 */
		if (wma_set_priv_cfg(wma, privcmd))
			WMA_LOGE("Failed to set wma priv congiuration");
		return;
	}

	switch (privcmd->param_vp_dev) {
	case VDEV_CMD:
		WMA_LOGD("vdev id %d pid %d pval %d", privcmd->param_vdev_id,
			 privcmd->param_id, privcmd->param_value);
		ret = wma_vdev_set_param(wma->wmi_handle,
						      privcmd->param_vdev_id,
						      privcmd->param_id,
						      privcmd->param_value);
		if (QDF_IS_STATUS_ERROR(ret)) {
			WMA_LOGE("wma_vdev_set_param failed ret %d",
				  ret);
			return;
		}
		break;
	case PDEV_CMD:
		WMA_LOGD("pdev pid %d pval %d", privcmd->param_id,
			 privcmd->param_value);
		if ((privcmd->param_id == WMI_PDEV_PARAM_RX_CHAIN_MASK) ||
		    (privcmd->param_id == WMI_PDEV_PARAM_TX_CHAIN_MASK)) {
			wma_update_txrx_chainmask(wma->num_rf_chains,
						  &privcmd->param_value);
		}
		pdev_param.param_id = privcmd->param_id;
		pdev_param.param_value = privcmd->param_value;
		ret = wmi_unified_pdev_param_send(wma->wmi_handle,
						 &pdev_param,
						 WMA_WILDCARD_PDEV_ID);
		if (QDF_IS_STATUS_ERROR(ret)) {
			WMA_LOGE("wma_vdev_set_param failed ret %d",
				 ret);
			return;
		}
		break;
	case GEN_CMD:
	{
		ol_txrx_vdev_handle vdev = NULL;
		struct wma_txrx_node *intr = wma->interfaces;

		vdev = wma_find_vdev_by_id(wma, privcmd->param_vdev_id);
		if (!vdev) {
			WMA_LOGE("%s:Invalid vdev handle", __func__);
			return;
		}

		WMA_LOGD("gen pid %d pval %d", privcmd->param_id,
			 privcmd->param_value);

		switch (privcmd->param_id) {
		case GEN_VDEV_PARAM_AMPDU:
			ret = ol_txrx_aggr_cfg(vdev, privcmd->param_value, 0);
			if (ret)
				WMA_LOGE("ol_txrx_aggr_cfg set ampdu failed ret %d",
					 ret);
			else
				intr[privcmd->param_vdev_id].config.ampdu =
							 privcmd->param_value;
			break;
		case GEN_VDEV_PARAM_AMSDU:
			ret = ol_txrx_aggr_cfg(vdev, 0, privcmd->param_value);
			if (ret)
				WMA_LOGE("ol_txrx_aggr_cfg set amsdu failed ret %d",
					 ret);
			else
				intr[privcmd->param_vdev_id].config.
				amsdu = privcmd->param_value;
			break;
		case GEN_PARAM_DUMP_AGC_START:
			htc_dump(wma->htc_handle, AGC_DUMP, true);
			break;
		case GEN_PARAM_DUMP_AGC:
			htc_dump(wma->htc_handle, AGC_DUMP, false);
			break;
		case GEN_PARAM_DUMP_CHANINFO_START:
			htc_dump(wma->htc_handle, CHAN_DUMP, true);
			break;
		case GEN_PARAM_DUMP_CHANINFO:
			htc_dump(wma->htc_handle, CHAN_DUMP, false);
			break;
		case GEN_PARAM_DUMP_WATCHDOG:
			htc_dump(wma->htc_handle, WD_DUMP, false);
			break;
		case GEN_PARAM_CRASH_INJECT:
			if (QDF_GLOBAL_FTM_MODE  == cds_get_conparam())
				WMA_LOGE("Crash inject not allowed in FTM mode");
			else
				ret = wma_crash_inject(wma,
						privcmd->param_value,
						privcmd->param_sec_value);
			break;
		case GEN_PARAM_CAPTURE_TSF:
			ret = wma_capture_tsf(wma, privcmd->param_value);
			break;
		case GEN_PARAM_RESET_TSF_GPIO:
			ret = wma_reset_tsf_gpio(wma, privcmd->param_value);
			break;
#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
		case GEN_PARAM_DUMP_PCIE_ACCESS_LOG:
			htc_dump(wma->htc_handle, PCIE_DUMP, false);
			break;
#endif /* CONFIG_ATH_PCIE_ACCESS_DEBUG */
		case GEN_PARAM_MODULATED_DTIM:
			wma_set_modulated_dtim(wma, privcmd);
			break;
		default:
			WMA_LOGE("Invalid param id 0x%x",
				 privcmd->param_id);
			break;
		}
		break;
	}
	case DBG_CMD:
		WMA_LOGD("dbg pid %d pval %d", privcmd->param_id,
			 privcmd->param_value);
		switch (privcmd->param_id) {
		case WMI_DBGLOG_LOG_LEVEL:
			ret = dbglog_set_log_lvl(wma->wmi_handle,
						   privcmd->param_value);
			if (ret)
				WMA_LOGE("dbglog_set_log_lvl failed ret %d",
					 ret);
			break;
		case WMI_DBGLOG_VAP_ENABLE:
			ret = dbglog_vap_log_enable(wma->wmi_handle,
						    privcmd->param_value, true);
			if (ret)
				WMA_LOGE("dbglog_vap_log_enable failed ret %d",
					 ret);
			break;
		case WMI_DBGLOG_VAP_DISABLE:
			ret = dbglog_vap_log_enable(wma->wmi_handle,
						privcmd->param_value, false);
			if (ret)
				WMA_LOGE("dbglog_vap_log_enable failed ret %d",
					 ret);
			break;
		case WMI_DBGLOG_MODULE_ENABLE:
			ret = dbglog_module_log_enable(wma->wmi_handle,
						privcmd->param_value, true);
			if (ret)
				WMA_LOGE("dbglog_module_log_enable failed ret %d",
					 ret);
			break;
		case WMI_DBGLOG_MODULE_DISABLE:
			ret = dbglog_module_log_enable(wma->wmi_handle,
						privcmd->param_value, false);
			if (ret)
				WMA_LOGE("dbglog_module_log_enable failed ret %d",
					 ret);
			break;
		case WMI_DBGLOG_MOD_LOG_LEVEL:
			ret = dbglog_set_mod_log_lvl(wma->wmi_handle,
						       privcmd->param_value);
			if (ret)
				WMA_LOGE("dbglog_module_log_enable failed ret %d",
					 ret);
			break;
		case WMI_DBGLOG_TYPE:
			ret = dbglog_parser_type_init(wma->wmi_handle,
							privcmd->param_value);
			if (ret)
				WMA_LOGE("dbglog_parser_type_init failed ret %d",
					 ret);
			break;
		case WMI_DBGLOG_REPORT_ENABLE:
			ret = dbglog_report_enable(wma->wmi_handle,
						     privcmd->param_value);
			if (ret)
				WMA_LOGE("dbglog_report_enable failed ret %d",
					 ret);
			break;
		case WMI_WLAN_PROFILE_TRIGGER_CMDID:
			ret = wma_unified_fw_profiling_cmd(wma->wmi_handle,
					 WMI_WLAN_PROFILE_TRIGGER_CMDID,
					 privcmd->param_value, 0);
			if (ret)
				WMA_LOGE("Profile cmd failed for %d ret %d",
					WMI_WLAN_PROFILE_TRIGGER_CMDID, ret);
			break;
		case WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID:
			ret = wma_unified_fw_profiling_cmd(wma->wmi_handle,
				  WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID,
				  privcmd->param_value,
				  privcmd->param_sec_value);
			if (ret)
				WMA_LOGE("Profile cmd failed for %d ret %d",
				   WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID,
				   ret);
			break;
		case WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID:
			ret = wma_unified_fw_profiling_cmd(wma->wmi_handle,
					 WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID,
					 privcmd->param_value,
					 privcmd->param_sec_value);
			if (ret)
				WMA_LOGE("Profile cmd failed for %d ret %d",
					WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID,
					ret);
			break;
		case WMI_WLAN_PROFILE_LIST_PROFILE_ID_CMDID:
			ret = wma_unified_fw_profiling_cmd(wma->wmi_handle,
					 WMI_WLAN_PROFILE_LIST_PROFILE_ID_CMDID,
					 0, 0);
			if (ret)
				WMA_LOGE("Profile cmd failed for %d ret %d",
					WMI_WLAN_PROFILE_LIST_PROFILE_ID_CMDID,
					ret);
			break;
		case WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID:
			ret = wma_unified_fw_profiling_cmd(wma->wmi_handle,
					WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID,
					0, 0);
			if (ret)
				WMA_LOGE("Profile cmd failed for %d ret %d",
				   WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID,
				   ret);
			break;
		case WMI_PDEV_GREEN_AP_PS_ENABLE_CMDID:
			/* Set the Green AP */
			ret = wmi_unified_green_ap_ps_send
					(wma->wmi_handle, privcmd->param_value,
					 WMA_WILDCARD_PDEV_ID);
			if (ret) {
				WMA_LOGE("Set GreenAP Failed val %d",
					 privcmd->param_value);
			}
			break;

		default:
			WMA_LOGE("Invalid param id 0x%x", privcmd->param_id);
			break;
		}
		break;
	case PPS_CMD:
		WMA_LOGD("dbg pid %d pval %d", privcmd->param_id,
			 privcmd->param_value);
		switch (privcmd->param_id) {

		case WMI_VDEV_PPS_PAID_MATCH:
			pps_val = ((privcmd->param_value << 31) & 0xffff0000) |
				  (PKT_PWR_SAVE_PAID_MATCH & 0xffff);
			intr[vid].config.pps_params.paid_match_enable =
				privcmd->param_value;
			break;
		case WMI_VDEV_PPS_GID_MATCH:
			pps_val = ((privcmd->param_value << 31) & 0xffff0000) |
				  (PKT_PWR_SAVE_GID_MATCH & 0xffff);
			intr[vid].config.pps_params.gid_match_enable =
				privcmd->param_value;
			break;
		case WMI_VDEV_PPS_EARLY_TIM_CLEAR:
			pps_val = ((privcmd->param_value << 31) & 0xffff0000) |
				  (PKT_PWR_SAVE_EARLY_TIM_CLEAR & 0xffff);
			intr[vid].config.pps_params.tim_clear =
				privcmd->param_value;
			break;
		case WMI_VDEV_PPS_EARLY_DTIM_CLEAR:
			pps_val = ((privcmd->param_value << 31) & 0xffff0000) |
				  (PKT_PWR_SAVE_EARLY_DTIM_CLEAR & 0xffff);
			intr[vid].config.pps_params.dtim_clear =
				privcmd->param_value;
			break;
		case WMI_VDEV_PPS_EOF_PAD_DELIM:
			pps_val = ((privcmd->param_value << 31) & 0xffff0000) |
				  (PKT_PWR_SAVE_EOF_PAD_DELIM & 0xffff);
			intr[vid].config.pps_params.eof_delim =
				privcmd->param_value;
			break;
		case WMI_VDEV_PPS_MACADDR_MISMATCH:
			pps_val = ((privcmd->param_value << 31) & 0xffff0000) |
				  (PKT_PWR_SAVE_MACADDR_MISMATCH & 0xffff);
			intr[vid].config.pps_params.mac_match =
				privcmd->param_value;
			break;
		case WMI_VDEV_PPS_DELIM_CRC_FAIL:
			pps_val = ((privcmd->param_value << 31) & 0xffff0000) |
				  (PKT_PWR_SAVE_DELIM_CRC_FAIL & 0xffff);
			intr[vid].config.pps_params.delim_fail =
				privcmd->param_value;
			break;
		case WMI_VDEV_PPS_GID_NSTS_ZERO:
			pps_val = ((privcmd->param_value << 31) & 0xffff0000) |
				  (PKT_PWR_SAVE_GID_NSTS_ZERO & 0xffff);
			intr[vid].config.pps_params.nsts_zero =
				privcmd->param_value;
			break;
		case WMI_VDEV_PPS_RSSI_CHECK:
			pps_val = ((privcmd->param_value << 31) & 0xffff0000) |
				  (PKT_PWR_SAVE_RSSI_CHECK & 0xffff);
			intr[vid].config.pps_params.rssi_chk =
				privcmd->param_value;
			break;
		case WMI_VDEV_PPS_5G_EBT:
			pps_val = ((privcmd->param_value << 31) & 0xffff0000) |
				  (PKT_PWR_SAVE_5G_EBT & 0xffff);
			intr[vid].config.pps_params.ebt_5g =
				privcmd->param_value;
			break;
		default:
			WMA_LOGE("Invalid param id 0x%x", privcmd->param_id);
			break;
		}
		break;

	case QPOWER_CMD:
		WMA_LOGD("QPOWER CLI CMD pid %d pval %d", privcmd->param_id,
			 privcmd->param_value);
		switch (privcmd->param_id) {
		case WMI_STA_PS_PARAM_QPOWER_PSPOLL_COUNT:
			WMA_LOGD("QPOWER CLI CMD:Ps Poll Cnt val %d",
				 privcmd->param_value);
			/* Set the QPower Ps Poll Count */
			ret = wma_unified_set_sta_ps_param(wma->wmi_handle,
				vid, WMI_STA_PS_PARAM_QPOWER_PSPOLL_COUNT,
				privcmd->param_value);
			if (ret) {
				WMA_LOGE("Set Q-PsPollCnt Failed vdevId %d val %d",
					vid, privcmd->param_value);
			} else {
				qparams->max_ps_poll_cnt = privcmd->param_value;
			}
			break;
		case WMI_STA_PS_PARAM_QPOWER_MAX_TX_BEFORE_WAKE:
			WMA_LOGD("QPOWER CLI CMD:Max Tx Before wake val %d",
				 privcmd->param_value);
			/* Set the QPower Max Tx Before Wake */
			ret = wma_unified_set_sta_ps_param(wma->wmi_handle,
				vid, WMI_STA_PS_PARAM_QPOWER_MAX_TX_BEFORE_WAKE,
				privcmd->param_value);
			if (ret) {
				WMA_LOGE("Set Q-MaxTxBefWake Failed vId %d val %d",
					vid, privcmd->param_value);
			} else {
				qparams->max_tx_before_wake =
						privcmd->param_value;
			}
			break;
		case WMI_STA_PS_PARAM_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL:
			WMA_LOGD("QPOWER CLI CMD:Ps Poll Wake Inv val %d",
				 privcmd->param_value);
			/* Set the QPower Spec Ps Poll Wake Inv */
			ret = wma_unified_set_sta_ps_param(wma->wmi_handle,
				vid, WMI_STA_PS_PARAM_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL,
				privcmd->param_value);
			if (ret) {
				WMA_LOGE("Set Q-PsPoll WakeIntv Failed vId %d val %d",
					vid, privcmd->param_value);
			} else {
				qparams->spec_ps_poll_wake_interval =
					privcmd->param_value;
			}
			break;
		case WMI_STA_PS_PARAM_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL:
			WMA_LOGD("QPOWER CLI CMD:Spec NoData Ps Poll val %d",
				 privcmd->param_value);
			/* Set the QPower Spec NoData PsPoll */
			ret = wma_unified_set_sta_ps_param(wma->wmi_handle,
				vid, WMI_STA_PS_PARAM_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL,
				privcmd->param_value);
			if (ret) {
				WMA_LOGE("Set Q-SpecNoDataPsPoll Failed vId %d val %d",
					vid, privcmd->param_value);
			} else {
				qparams->max_spec_nodata_ps_poll =
					privcmd->param_value;
			}
			break;

		default:
			WMA_LOGE("Invalid param id 0x%x", privcmd->param_id);
			break;
		}
		break;
	case GTX_CMD:
		WMA_LOGD("vdev id %d pid %d pval %d", privcmd->param_vdev_id,
			 privcmd->param_id, privcmd->param_value);
		switch (privcmd->param_id) {
		case WMI_VDEV_PARAM_GTX_HT_MCS:
			intr[vid].config.gtx_info.gtxRTMask[0] =
				privcmd->param_value;
			ret = wmi_unified_vdev_set_gtx_cfg_send(wma->wmi_handle,
					privcmd->param_vdev_id,
					&intr[vid].config.gtx_info);
			break;
		case WMI_VDEV_PARAM_GTX_VHT_MCS:
			intr[vid].config.gtx_info.gtxRTMask[1] =
				privcmd->param_value;
			ret = wmi_unified_vdev_set_gtx_cfg_send(wma->wmi_handle,
					privcmd->param_vdev_id,
					&intr[vid].config.gtx_info);
			break;

		case WMI_VDEV_PARAM_GTX_USR_CFG:
			intr[vid].config.gtx_info.gtxUsrcfg =
				privcmd->param_value;
			ret = wmi_unified_vdev_set_gtx_cfg_send(wma->wmi_handle,
					privcmd->param_vdev_id,
					&intr[vid].config.gtx_info);
			break;

		case WMI_VDEV_PARAM_GTX_THRE:
			intr[vid].config.gtx_info.gtxPERThreshold =
				privcmd->param_value;
			ret = wmi_unified_vdev_set_gtx_cfg_send(wma->wmi_handle,
					privcmd->param_vdev_id,
					&intr[vid].config.gtx_info);
			break;

		case WMI_VDEV_PARAM_GTX_MARGIN:
			intr[vid].config.gtx_info.gtxPERMargin =
				privcmd->param_value;
			ret = wmi_unified_vdev_set_gtx_cfg_send(wma->wmi_handle,
					privcmd->param_vdev_id,
					&intr[vid].config.gtx_info);
			break;

		case WMI_VDEV_PARAM_GTX_STEP:
			intr[vid].config.gtx_info.gtxTPCstep =
				privcmd->param_value;
			ret = wmi_unified_vdev_set_gtx_cfg_send(wma->wmi_handle,
					privcmd->param_vdev_id,
					&intr[vid].config.gtx_info);
			break;

		case WMI_VDEV_PARAM_GTX_MINTPC:
			intr[vid].config.gtx_info.gtxTPCMin =
				privcmd->param_value;
			ret = wmi_unified_vdev_set_gtx_cfg_send(wma->wmi_handle,
					privcmd->param_vdev_id,
					&intr[vid].config.gtx_info);
			break;

		case WMI_VDEV_PARAM_GTX_BW_MASK:
			intr[vid].config.gtx_info.gtxBWMask =
				privcmd->param_value;
			ret = wmi_unified_vdev_set_gtx_cfg_send(wma->wmi_handle,
					privcmd->param_vdev_id,
					&intr[vid].config.gtx_info);
			if (ret) {
				WMA_LOGE("wma_vdev_set_param"
					 " failed ret %d", ret);
				return;
			}
			break;
		default:
			break;
		}
		break;

	default:
		WMA_LOGE("Invalid vpdev command id");
	}
	if (1 == privcmd->param_vp_dev) {
		switch (privcmd->param_id) {
		case WMI_VDEV_PARAM_NSS:
			intr[vid].config.nss = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_LDPC:
			intr[vid].config.ldpc = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_TX_STBC:
			intr[vid].config.tx_stbc = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_RX_STBC:
			intr[vid].config.rx_stbc = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_SGI:
			intr[vid].config.shortgi = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_ENABLE_RTSCTS:
			intr[vid].config.rtscts_en = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_CHWIDTH:
			intr[vid].config.chwidth = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_FIXED_RATE:
			intr[vid].config.tx_rate = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_EARLY_RX_ADJUST_ENABLE:
			intr[vid].config.erx_adjust = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_EARLY_RX_TGT_BMISS_NUM:
			intr[vid].config.erx_bmiss_num = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_EARLY_RX_BMISS_SAMPLE_CYCLE:
			intr[vid].config.erx_bmiss_cycle = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_EARLY_RX_SLOP_STEP:
			intr[vid].config.erx_slop_step = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_EARLY_RX_INIT_SLOP:
			intr[vid].config.erx_init_slop = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_EARLY_RX_ADJUST_PAUSE:
			intr[vid].config.erx_adj_pause = privcmd->param_value;
			break;
		case WMI_VDEV_PARAM_EARLY_RX_DRIFT_SAMPLE:
			intr[vid].config.erx_dri_sample = privcmd->param_value;
			break;
		default:
			WMA_LOGE("Invalid wma_cli_set vdev command/Not"
				 " yet implemented 0x%x", privcmd->param_id);
			break;
		}
	} else if (2 == privcmd->param_vp_dev) {
		switch (privcmd->param_id) {
		case WMI_PDEV_PARAM_ANI_ENABLE:
			wma->pdevconfig.ani_enable = privcmd->param_value;
			break;
		case WMI_PDEV_PARAM_ANI_POLL_PERIOD:
			wma->pdevconfig.ani_poll_len = privcmd->param_value;
			break;
		case WMI_PDEV_PARAM_ANI_LISTEN_PERIOD:
			wma->pdevconfig.ani_listen_len = privcmd->param_value;
			break;
		case WMI_PDEV_PARAM_ANI_OFDM_LEVEL:
			wma->pdevconfig.ani_ofdm_level = privcmd->param_value;
			break;
		case WMI_PDEV_PARAM_ANI_CCK_LEVEL:
			wma->pdevconfig.ani_cck_level = privcmd->param_value;
			break;
		case WMI_PDEV_PARAM_DYNAMIC_BW:
			wma->pdevconfig.cwmenable = privcmd->param_value;
			break;
		case WMI_PDEV_PARAM_CTS_CBW:
			wma->pdevconfig.cts_cbw = privcmd->param_value;
			break;
		case WMI_PDEV_PARAM_TX_CHAIN_MASK:
			wma->pdevconfig.txchainmask = privcmd->param_value;
			break;
		case WMI_PDEV_PARAM_RX_CHAIN_MASK:
			wma->pdevconfig.rxchainmask = privcmd->param_value;
			break;
		case WMI_PDEV_PARAM_BURST_ENABLE:
			wma->pdevconfig.burst_enable = privcmd->param_value;
			if ((wma->pdevconfig.burst_enable == 1) &&
			    (wma->pdevconfig.burst_dur == 0))
				wma->pdevconfig.burst_dur =
					WMA_DEFAULT_SIFS_BURST_DURATION;
			else if (wma->pdevconfig.burst_enable == 0)
				wma->pdevconfig.burst_dur = 0;
			break;
		case WMI_PDEV_PARAM_BURST_DUR:
			wma->pdevconfig.burst_dur = privcmd->param_value;
			break;
		case WMI_PDEV_PARAM_TXPOWER_LIMIT2G:
			wma->pdevconfig.txpow2g = privcmd->param_value;
			if ((pMac->roam.configParam.bandCapability ==
			     eCSR_BAND_ALL) ||
			    (pMac->roam.configParam.bandCapability ==
			     eCSR_BAND_24)) {
				if (cfg_set_int(pMac,
						WNI_CFG_CURRENT_TX_POWER_LEVEL,
						privcmd->param_value) !=
								eSIR_SUCCESS)
					WMA_LOGE("could not set WNI_CFG_CURRENT_TX_POWER_LEVEL");

			} else {
				WMA_LOGE("Current band is not 2G");
			}
			break;
		case WMI_PDEV_PARAM_TXPOWER_LIMIT5G:
			wma->pdevconfig.txpow5g = privcmd->param_value;
			if ((pMac->roam.configParam.bandCapability ==
			     eCSR_BAND_ALL) ||
			    (pMac->roam.configParam.bandCapability ==
			     eCSR_BAND_5G)) {
				if (cfg_set_int(pMac,
						WNI_CFG_CURRENT_TX_POWER_LEVEL,
						privcmd->param_value) !=
							    eSIR_SUCCESS)
					WMA_LOGE("could not set WNI_CFG_CURRENT_TX_POWER_LEVEL");

			} else {
				WMA_LOGE("Current band is not 5G");
			}
			break;
		default:
			WMA_LOGE("Invalid wma_cli_set pdev command/Not yet implemented 0x%x",
				 privcmd->param_id);
			break;
		}
	} else if (5 == privcmd->param_vp_dev) {
		ret = wma_vdev_set_param(wma->wmi_handle,
						      privcmd->param_vdev_id,
						      WMI_VDEV_PARAM_PACKET_POWERSAVE,
						      pps_val);
		if (ret)
			WMA_LOGE("Failed to send wmi packet power save cmd");
		else
			WMA_LOGD("Sent packet power save cmd %d value %x to target",
				privcmd->param_id, pps_val);
	}
}

/**
 * wma_process_fw_event() - process any fw event
 * @wma: wma handle
 * @buf: fw event buffer
 *
 * This function process any fw event to serialize it through mc thread.
 *
 * Return: none
 */
static int wma_process_fw_event(tp_wma_handle wma,
				wma_process_fw_event_params *buf)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)buf->wmi_handle;

	wmi_process_fw_event(wmi_handle, buf->evt_buf);
	return 0;
}

/**
 * wmi_process_fw_event_tasklet_ctx() - process in tasklet context
 * @ctx: handle to wmi
 * @ev: wmi event buffer
 *
 * Event process by below function will be in tasket context,
 * need to use this method only for time sensitive functions.
 *
 * Return: none
 */
static int wma_process_fw_event_tasklet_ctx(void *ctx, void *ev)
{
	wmi_process_fw_event(ctx, ev);

	return 0;
}

/**
 * wma_process_fw_event_handler() - common event handler to serialize
 *                                  event processing through mc_thread
 * @ctx: wmi context
 * @ev: event buffer
 * @rx_ctx: rx execution context
 *
 * Return: 0 on success, errno on failure
 */
static int wma_process_fw_event_mc_thread_ctx(void *ctx, void *ev)
{
	wma_process_fw_event_params *params_buf;
	cds_msg_t cds_msg = { 0 };

	params_buf = qdf_mem_malloc(sizeof(wma_process_fw_event_params));
	if (!params_buf) {
		WMA_LOGE("%s: Failed alloc memory for params_buf", __func__);
		qdf_nbuf_free(ev);
		return -ENOMEM;
	}

	params_buf->wmi_handle = (struct wmi_unified *)ctx;
	params_buf->evt_buf = (wmi_buf_t *)ev;

	cds_msg.type = WMA_PROCESS_FW_EVENT;
	cds_msg.bodyptr = params_buf;
	cds_msg.bodyval = 0;

	if (QDF_STATUS_SUCCESS !=
		cds_mq_post_message(CDS_MQ_ID_WMA, &cds_msg)) {
		WMA_LOGP("%s: Failed to post WMA_PROCESS_FW_EVENT msg",
			 __func__);
		qdf_nbuf_free(ev);
		qdf_mem_free(params_buf);
		return -EFAULT;
	}
	WMA_LOGD("WMA_PROCESS_FW_EVENT posted");
	return 0;

}

/**
 * wma_process_fw_event_handler() - common event handler to serialize
 *                                  event processing through mc_thread
 * @ctx: wmi context
 * @ev: event buffer
 * @rx_ctx: rx execution context
 *
 * Return: 0 on success, errno on failure
 */
int wma_process_fw_event_handler(void *ctx, void *ev, uint8_t rx_ctx)
{
	int err = 0;

	if (rx_ctx == WMA_RX_SERIALIZER_CTX) {
		err = wma_process_fw_event_mc_thread_ctx(ctx, ev);
	} else if (rx_ctx == WMA_RX_TASKLET_CTX) {
		wma_process_fw_event_tasklet_ctx(ctx, ev);
	} else {
		WMA_LOGE("%s: invalid wmi event execution context", __func__);
		qdf_nbuf_free(ev);
	}

	return err;
}

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
/**
 * ol_cfg_set_flow_control_parameters() - set flow control parameters
 * @olCfg: cfg parameters
 * @mac_params: mac parameters
 *
 * Return: none
 */
static
void ol_cfg_set_flow_control_parameters(struct txrx_pdev_cfg_param_t *olCfg,
		tMacOpenParameters *mac_params)
{
	olCfg->tx_flow_start_queue_offset =
				mac_params->tx_flow_start_queue_offset;
	olCfg->tx_flow_stop_queue_th =
				mac_params->tx_flow_stop_queue_th;
}
#else
static
void ol_cfg_set_flow_control_parameters(struct txrx_pdev_cfg_param_t *olCfg,
		tMacOpenParameters *mac_params)
{
	return;
}
#endif

#ifdef WLAN_FEATURE_NAN
/**
 * wma_set_nan_enable() - set nan enable flag in WMA handle
 * @wma_handle: Pointer to wma handle
 * @mac_param: Pointer to mac_param
 *
 * Return: none
 */
static void wma_set_nan_enable(tp_wma_handle wma_handle,
				tMacOpenParameters *mac_param)
{
	wma_handle->is_nan_enabled = mac_param->is_nan_enabled;
}
#else
static void wma_set_nan_enable(tp_wma_handle wma_handle,
				tMacOpenParameters *mac_param)
{
}
#endif

/**
 * wma_init_max_no_of_peers - API to initialize wma configuration params
 * @wma_handle: WMA Handle
 * @max_peers: Max Peers supported
 *
 * Return: void
 */
static void wma_init_max_no_of_peers(tp_wma_handle wma_handle,
				     uint16_t max_peers)
{
	struct wma_ini_config *cfg = wma_get_ini_handle(wma_handle);

	cfg->max_no_of_peers = max_peers;
}

/**
 * wma_open() - Allocate wma context and initialize it.
 * @cds_context:  cds context
 * @wma_tgt_cfg_cb: tgt config callback fun
 * @radar_ind_cb: dfs radar indication callback
 * @mac_params:  mac parameters
 *
 * Return: 0 on success, errno on failure
 */
QDF_STATUS wma_open(void *cds_context,
		    wma_tgt_cfg_cb tgt_cfg_cb,
		    wma_dfs_radar_indication_cb radar_ind_cb,
		    tMacOpenParameters *mac_params)
{
	tp_wma_handle wma_handle;
	HTC_HANDLE htc_handle;
	qdf_device_t qdf_dev;
	void *wmi_handle;
	QDF_STATUS qdf_status;
	struct txrx_pdev_cfg_param_t olCfg = { 0 };
	struct wmi_rx_ops ops;

	bool use_cookie = false;

	WMA_LOGD("%s: Enter", __func__);

	qdf_dev = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	htc_handle = cds_get_context(QDF_MODULE_ID_HTC);

	if (!htc_handle) {
		WMA_LOGP("%s: Invalid HTC handle", __func__);
		return QDF_STATUS_E_INVAL;
	}

	/* Alloc memory for WMA Context */
	qdf_status = cds_alloc_context(cds_context, QDF_MODULE_ID_WMA,
				       (void **)&wma_handle,
				       sizeof(t_wma_handle));

	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMA_LOGP("%s: Memory allocation failed for wma_handle",
			 __func__);
		return qdf_status;
	}

	qdf_mem_zero(wma_handle, sizeof(t_wma_handle));

	if (cds_get_conparam() != QDF_GLOBAL_FTM_MODE) {
#ifdef FEATURE_WLAN_SCAN_PNO
		qdf_wake_lock_create(&wma_handle->pno_wake_lock, "wlan_pno_wl");
#endif /* FEATURE_WLAN_SCAN_PNO */
#ifdef FEATURE_WLAN_EXTSCAN
		qdf_wake_lock_create(&wma_handle->extscan_wake_lock,
					"wlan_extscan_wl");
#endif /* FEATURE_WLAN_EXTSCAN */
		qdf_wake_lock_create(&wma_handle->wow_wake_lock, "wlan_wow_wl");
	}

	/* Attach mc_thread context processing function */
	ops.wma_process_fw_event_handler_cbk = wma_process_fw_event_handler;
	/* attach the wmi */
	wmi_handle = wmi_unified_attach(wma_handle, NULL,
					WMI_TLV_TARGET, use_cookie, &ops);
	if (!wmi_handle) {
		WMA_LOGP("%s: failed to attach WMI", __func__);
		qdf_status = QDF_STATUS_E_NOMEM;
		goto err_wma_handle;
	}

	WMA_LOGA("WMA --> wmi_unified_attach - success");
	wmi_unified_register_event_handler(wmi_handle,
					   WMI_SERVICE_READY_EVENTID,
					   wma_rx_service_ready_event,
					   WMA_RX_SERIALIZER_CTX);
	wmi_unified_register_event_handler(wmi_handle,
					   WMI_SERVICE_READY_EXT_EVENTID,
					   wma_rx_service_ready_ext_event,
					   WMA_RX_SERIALIZER_CTX);
	wmi_unified_register_event_handler(wmi_handle,
					   WMI_READY_EVENTID,
					   wma_rx_ready_event,
					   WMA_RX_SERIALIZER_CTX);
	/* Save the WMI & HTC handle */
	wma_handle->wmi_handle = wmi_handle;
	wma_handle->htc_handle = htc_handle;
	wma_handle->cds_context = cds_context;
	wma_handle->qdf_dev = qdf_dev;
	wma_handle->max_scan = mac_params->max_scan;

	wma_handle->wma_runtime_resume_lock =
		qdf_runtime_lock_init("wma_runtime_resume");

	/* Initialize max_no_of_peers for wma_get_number_of_peers_supported() */
	wma_init_max_no_of_peers(wma_handle, mac_params->maxStation);
	/* Cap maxStation based on the target version */
	mac_params->maxStation = wma_get_number_of_peers_supported(wma_handle);
	/* Reinitialize max_no_of_peers based on the capped maxStation value */
	wma_init_max_no_of_peers(wma_handle, mac_params->maxStation);

	/* initialize default target config */
	wma_set_default_tgt_config(wma_handle);

	olCfg.is_uc_offload_enabled = mac_params->ucOffloadEnabled;
	olCfg.uc_tx_buffer_count = mac_params->ucTxBufCount;
	olCfg.uc_tx_buffer_size = mac_params->ucTxBufSize;
	olCfg.uc_rx_indication_ring_count = mac_params->ucRxIndRingCount;
	olCfg.uc_tx_partition_base = mac_params->ucTxPartitionBase;


	wma_handle->tx_chain_mask_cck = mac_params->tx_chain_mask_cck;
	wma_handle->self_gen_frm_pwr = mac_params->self_gen_frm_pwr;

	/* Allocate cfg handle */

	/* RX Full reorder should enable for PCIe, ROME3.X project only now
	 * MDM should enable later, schedule TBD
	 * HL also sdould be enabled, schedule TBD
	 */
#ifdef WLAN_FEATURE_RX_FULL_REORDER_OL
	olCfg.is_full_reorder_offload = mac_params->reorderOffload;
#else
	olCfg.is_full_reorder_offload = 0;
#endif /* WLAN_FEATURE_RX_FULL_REORDER_OL */
	olCfg.enable_rxthread = mac_params->enable_rxthread;
	olCfg.ip_tcp_udp_checksum_offload =
			mac_params->ip_tcp_udp_checksum_offload;
	olCfg.ce_classify_enabled = mac_params->ce_classify_enabled;

	ol_cfg_set_flow_control_parameters(&olCfg, mac_params);

	((p_cds_contextType) cds_context)->cfg_ctx =
		ol_pdev_cfg_attach(((p_cds_contextType) cds_context)->qdf_ctx,
				   olCfg);
	if (!(((p_cds_contextType) cds_context)->cfg_ctx)) {
		WMA_LOGP("%s: failed to init cfg handle", __func__);
		qdf_status = QDF_STATUS_E_NOMEM;
		goto err_wmi_handle;
	}

	/* adjust the cfg_ctx default value based on setting */
	ol_set_cfg_rx_fwd_disabled((ol_pdev_handle)
				   ((p_cds_contextType) cds_context)->cfg_ctx,
				   (uint8_t) mac_params->apDisableIntraBssFwd);

	/* adjust the packet log enable default value based on CFG INI setting */
	ol_set_cfg_packet_log_enabled((ol_pdev_handle)
					((p_cds_contextType) cds_context)->
						cfg_ctx,
				      (uint8_t)cds_is_packet_log_enabled());

	/* Allocate dfs_ic and initialize DFS */
	wma_handle->dfs_ic = wma_dfs_attach(wma_handle->dfs_ic);
	if (wma_handle->dfs_ic == NULL) {
		WMA_LOGE("%s: Memory allocation failed for dfs_ic", __func__);
		goto err_wmi_handle;
	}
#if defined(QCA_WIFI_FTM)
	if (cds_get_conparam() == QDF_GLOBAL_FTM_MODE)
		wma_utf_attach(wma_handle);
#endif /* QCA_WIFI_FTM */

	mac_params->maxBssId = WMA_MAX_SUPPORTED_BSS;
	mac_params->frameTransRequired = 0;

	wma_handle->wlan_resource_config.num_wow_filters =
		mac_params->maxWoWFilters;
	wma_handle->wlan_resource_config.num_keep_alive_pattern =
		WMA_MAXNUM_PERIODIC_TX_PTRNS;

	/* The current firmware implementation requires the number of
	 * offload peers should be (number of vdevs + 1).
	 */
	wma_handle->wlan_resource_config.num_offload_peers =
		mac_params->apMaxOffloadPeers + 1;

	wma_handle->wlan_resource_config.num_offload_reorder_buffs =
		mac_params->apMaxOffloadReorderBuffs + 1;

	wma_handle->ol_ini_info = mac_params->olIniInfo;
	wma_handle->max_station = mac_params->maxStation;
	wma_handle->max_bssid = mac_params->maxBssId;
	wma_handle->frame_xln_reqd = mac_params->frameTransRequired;
	wma_handle->driver_type = mac_params->driverType;
	wma_handle->ssdp = mac_params->ssdp;
#ifdef FEATURE_WLAN_RA_FILTERING
	wma_handle->IsRArateLimitEnabled = mac_params->IsRArateLimitEnabled;
	wma_handle->RArateLimitInterval = mac_params->RArateLimitInterval;
#endif /* FEATURE_WLAN_RA_FILTERING */
#ifdef WLAN_FEATURE_LPSS
	wma_handle->is_lpass_enabled = mac_params->is_lpass_enabled;
#endif
	wma_set_nan_enable(wma_handle, mac_params);
	/*
	 * Indicates if DFS Phyerr filtering offload
	 * is Enabled/Disabed from ini
	 */
	wma_handle->dfs_phyerr_filter_offload =
		mac_params->dfsPhyerrFilterOffload;
	wma_handle->dfs_pri_multiplier = mac_params->dfsRadarPriMultiplier;
	wma_handle->interfaces = qdf_mem_malloc(sizeof(struct wma_txrx_node) *
						wma_handle->max_bssid);
	if (!wma_handle->interfaces) {
		WMA_LOGP("%s: failed to allocate interface table", __func__);
		qdf_status = QDF_STATUS_E_NOMEM;
		goto err_scn_context;
	}
	qdf_mem_zero(wma_handle->interfaces, sizeof(struct wma_txrx_node) *
		     wma_handle->max_bssid);
	/* Register the debug print event handler */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					WMI_DEBUG_PRINT_EVENTID,
					wma_unified_debug_print_event_handler,
					WMA_RX_SERIALIZER_CTX);
	/* Register profiling event Handler */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					WMI_WLAN_PROFILE_DATA_EVENTID,
					wma_profile_data_report_event_handler,
					WMA_RX_SERIALIZER_CTX);

	wma_handle->tgt_cfg_update_cb = tgt_cfg_cb;
	wma_handle->dfs_radar_indication_cb = radar_ind_cb;
	wma_handle->old_hw_mode_index = WMA_DEFAULT_HW_MODE_INDEX;
	wma_handle->new_hw_mode_index = WMA_DEFAULT_HW_MODE_INDEX;
	wma_handle->saved_chan.num_channels = 0;

	qdf_status = qdf_event_create(&wma_handle->wma_ready_event);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMA_LOGP("%s: wma_ready_event initialization failed", __func__);
		goto err_event_init;
	}

	qdf_status = qdf_mc_timer_init(&wma_handle->service_ready_ext_timer,
					QDF_TIMER_TYPE_SW,
					wma_service_ready_ext_evt_timeout,
					wma_handle);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGE("Failed to initialize service ready ext timeout");
		goto err_event_init;
	}

	qdf_status = qdf_event_create(&wma_handle->target_suspend);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMA_LOGP("%s: target suspend event initialization failed",
			 __func__);
		goto err_event_init;
	}

	/* Init Tx Frame Complete event */
	qdf_status = qdf_event_create(&wma_handle->tx_frm_download_comp_event);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGP("%s: failed to init tx_frm_download_comp_event",
			 __func__);
		goto err_event_init;
	}

	/* Init tx queue empty check event */
	qdf_status = qdf_event_create(&wma_handle->tx_queue_empty_event);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		WMA_LOGP("%s: failed to init tx_queue_empty_event", __func__);
		goto err_event_init;
	}

	qdf_status = qdf_event_create(&wma_handle->wma_resume_event);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMA_LOGP("%s: wma_resume_event initialization failed",
			 __func__);
		goto err_event_init;
	}

	qdf_status = qdf_event_create(&wma_handle->runtime_suspend);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMA_LOGP("%s: runtime_suspend event initialization failed",
			 __func__);
		goto err_event_init;
	}

	qdf_status = qdf_event_create(&wma_handle->recovery_event);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMA_LOGP("%s: recovery event initialization failed", __func__);
		goto err_event_init;
	}

	qdf_list_create(&wma_handle->vdev_resp_queue,
		      MAX_ENTRY_VDEV_RESP_QUEUE);
	qdf_spinlock_create(&wma_handle->vdev_respq_lock);
	qdf_list_create(&wma_handle->wma_hold_req_queue,
		      MAX_ENTRY_HOLD_REQ_QUEUE);
	qdf_spinlock_create(&wma_handle->wma_hold_req_q_lock);
	qdf_atomic_init(&wma_handle->is_wow_bus_suspended);
	qdf_atomic_init(&wma_handle->scan_id_counter);

	/* Register vdev start response event handler */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_VDEV_START_RESP_EVENTID,
					   wma_vdev_start_resp_handler,
					   WMA_RX_SERIALIZER_CTX);

	/* Register vdev stop response event handler */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_VDEV_STOPPED_EVENTID,
					   wma_vdev_stop_resp_handler,
					   WMA_RX_SERIALIZER_CTX);

	/* register for STA kickout function */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_PEER_STA_KICKOUT_EVENTID,
					   wma_peer_sta_kickout_event_handler,
					   WMA_RX_SERIALIZER_CTX);

	/* register for stats response event */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_UPDATE_STATS_EVENTID,
					   wma_stats_event_handler,
					   WMA_RX_SERIALIZER_CTX);
	/* register for linkspeed response event */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_PEER_ESTIMATED_LINKSPEED_EVENTID,
					   wma_link_speed_event_handler,
					   WMA_RX_SERIALIZER_CTX);

#ifdef FEATURE_OEM_DATA_SUPPORT
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_OEM_CAPABILITY_EVENTID,
					   wma_oem_capability_event_callback,
					   WMA_RX_SERIALIZER_CTX);

	wmi_unified_register_event_handler(wma_handle->wmi_handle,
				WMI_OEM_MEASUREMENT_REPORT_EVENTID,
				wma_oem_measurement_report_event_callback,
				WMA_RX_SERIALIZER_CTX);

	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_OEM_ERROR_REPORT_EVENTID,
					   wma_oem_error_report_event_callback,
					   WMA_RX_SERIALIZER_CTX);

	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_OEM_RESPONSE_EVENTID,
					   wma_oem_data_response_handler,
					   WMA_RX_SERIALIZER_CTX);
#endif /* FEATURE_OEM_DATA_SUPPORT */
	/*
	 * Register appropriate DFS phyerr event handler for
	 * Phyerror events. Handlers differ for phyerr filtering
	 * offload enable and disable cases.
	 */
	wma_register_dfs_event_handler(wma_handle);

	/* Register peer change event handler */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_PEER_STATE_EVENTID,
					   wma_peer_state_change_event_handler,
					   WMA_RX_WORK_CTX);

	/* Register beacon tx complete event id. The event is required
	 * for sending channel switch announcement frames
	 */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					WMI_OFFLOAD_BCN_TX_STATUS_EVENTID,
					wma_unified_bcntx_status_event_handler,
					WMA_RX_SERIALIZER_CTX);

	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_UPDATE_VDEV_RATE_STATS_EVENTID,
					   wma_link_status_event_handler,
					   WMA_RX_SERIALIZER_CTX);
#ifdef WLAN_FEATURE_LINK_LAYER_STATS
	/* Register event handler for processing Link Layer Stats
	 * response from the FW
	 */
	wma_register_ll_stats_event_handler(wma_handle);

#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

	/*
	 * Register event handler to receive firmware mem dump
	 * copy complete indication
	 */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
			WMI_UPDATE_FW_MEM_DUMP_EVENTID,
			wma_fw_mem_dump_event_handler,
			WMA_RX_SERIALIZER_CTX);

	/* Firmware debug log */
	qdf_status = dbglog_init(wma_handle->wmi_handle);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMA_LOGP("%s: Firmware Dbglog initialization failed", __func__);
		goto err_dbglog_init;
	}

	/*
	 * Update Powersave mode
	 * 1 - Legacy Powersave + Deepsleep Disabled
	 * 2 - QPower + Deepsleep Disabled
	 * 3 - Legacy Powersave + Deepsleep Enabled
	 * 4 - QPower + Deepsleep Enabled
	 */
	wma_handle->powersave_mode = mac_params->powersaveOffloadEnabled;
	wma_handle->staMaxLIModDtim = mac_params->staMaxLIModDtim;
	wma_handle->staModDtim = mac_params->staModDtim;
	wma_handle->staDynamicDtim = mac_params->staDynamicDtim;

	/*
	 * Value of mac_params->wowEnable can be,
	 * 0 - Disable both magic pattern match and pattern byte match.
	 * 1 - Enable magic pattern match on all interfaces.
	 * 2 - Enable pattern byte match on all interfaces.
	 * 3 - Enable both magic patter and pattern byte match on
	 *     all interfaces.
	 */
	wma_handle->wow.magic_ptrn_enable =
		(mac_params->wowEnable & 0x01) ? true : false;
	wma_handle->ptrn_match_enable_all_vdev =
		(mac_params->wowEnable & 0x02) ? true : false;

#ifdef FEATURE_WLAN_TDLS
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_TDLS_PEER_EVENTID,
					   wma_tdls_event_handler,
					   WMA_RX_SERIALIZER_CTX);
#endif /* FEATURE_WLAN_TDLS */

	/* register for install key completion event */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
				WMI_VDEV_INSTALL_KEY_COMPLETE_EVENTID,
				wma_vdev_install_key_complete_event_handler,
				WMA_RX_SERIALIZER_CTX);
#ifdef WLAN_FEATURE_NAN
	/* register for nan response event */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_NAN_EVENTID,
					   wma_nan_rsp_event_handler,
					   WMA_RX_SERIALIZER_CTX);
#endif /* WLAN_FEATURE_NAN */

#ifdef WLAN_FEATURE_STATS_EXT
	/* register for extended stats event */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_STATS_EXT_EVENTID,
					   wma_stats_ext_event_handler,
					   WMA_RX_SERIALIZER_CTX);
#endif /* WLAN_FEATURE_STATS_EXT */
#ifdef FEATURE_WLAN_EXTSCAN
	wma_register_extscan_event_handler(wma_handle);
#endif /* WLAN_FEATURE_STATS_EXT */

	WMA_LOGD("%s: Exit", __func__);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_ROAM_SYNCH_EVENTID,
					   wma_roam_synch_event_handler,
					   WMA_RX_SERIALIZER_CTX);
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
				WMI_RSSI_BREACH_EVENTID,
				wma_rssi_breached_event_handler,
				WMA_RX_SERIALIZER_CTX);

	qdf_wake_lock_create(&wma_handle->wmi_cmd_rsp_wake_lock,
					"wlan_fw_rsp_wakelock");
	wma_handle->wmi_cmd_rsp_runtime_lock =
			qdf_runtime_lock_init("wlan_fw_rsp_runtime_lock");

	/* Register peer assoc conf event handler */
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_PEER_ASSOC_CONF_EVENTID,
					   wma_peer_assoc_conf_handler,
					   WMA_RX_SERIALIZER_CTX);
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_VDEV_DELETE_RESP_EVENTID,
					   wma_vdev_delete_handler,
					   WMA_RX_SERIALIZER_CTX);
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_PEER_DELETE_RESP_EVENTID,
					   wma_peer_delete_handler,
					   WMA_RX_SERIALIZER_CTX);
	wmi_unified_register_event_handler(wma_handle->wmi_handle,
					   WMI_BPF_CAPABILIY_INFO_EVENTID,
					   wma_get_bpf_caps_event_handler,
					   WMA_RX_SERIALIZER_CTX);
	return QDF_STATUS_SUCCESS;

err_dbglog_init:
	qdf_wake_lock_destroy(&wma_handle->wmi_cmd_rsp_wake_lock);
	qdf_runtime_lock_deinit(wma_handle->wmi_cmd_rsp_runtime_lock);
	qdf_spinlock_destroy(&wma_handle->vdev_respq_lock);
	qdf_spinlock_destroy(&wma_handle->wma_hold_req_q_lock);
err_event_init:
	wmi_unified_unregister_event_handler(wma_handle->wmi_handle,
					     WMI_DEBUG_PRINT_EVENTID);
	qdf_mem_free(wma_handle->interfaces);
err_scn_context:
	wma_dfs_detach(wma_handle->dfs_ic);
#if defined(QCA_WIFI_FTM)
	wma_utf_detach(wma_handle);
#endif /* QCA_WIFI_FTM */
err_wmi_handle:
	qdf_mem_free(((p_cds_contextType) cds_context)->cfg_ctx);
	OS_FREE(wmi_handle);

err_wma_handle:

	if (cds_get_conparam() != QDF_GLOBAL_FTM_MODE) {
#ifdef FEATURE_WLAN_SCAN_PNO
		qdf_wake_lock_destroy(&wma_handle->pno_wake_lock);
#endif /* FEATURE_WLAN_SCAN_PNO */
#ifdef FEATURE_WLAN_EXTSCAN
		qdf_wake_lock_destroy(&wma_handle->extscan_wake_lock);
#endif /* FEATURE_WLAN_EXTSCAN */
		qdf_wake_lock_destroy(&wma_handle->wow_wake_lock);
	}

	qdf_runtime_lock_deinit(wma_handle->wma_runtime_resume_lock);
	cds_free_context(cds_context, QDF_MODULE_ID_WMA, wma_handle);

	WMA_LOGD("%s: Exit", __func__);

	return qdf_status;
}

/**
 * wma_pre_start() - wma pre start
 * @cds_ctx:  cds context
 *
 * Return: 0 on success, errno on failure
 */
QDF_STATUS wma_pre_start(void *cds_ctx)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	A_STATUS status = A_OK;
	tp_wma_handle wma_handle;
	cds_msg_t wma_msg = { 0 };

	WMA_LOGD("%s: Enter", __func__);

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	/* Validate the wma_handle */
	if (NULL == wma_handle) {
		WMA_LOGP("%s: invalid argument", __func__);
		qdf_status = QDF_STATUS_E_INVAL;
		goto end;
	}
	/* Open endpoint for ctrl path - WMI <--> HTC */
	status = wmi_unified_connect_htc_service(wma_handle->wmi_handle,
						 wma_handle->htc_handle);
	if (A_OK != status) {
		WMA_LOGP("%s: wmi_unified_connect_htc_service", __func__);
		qdf_status = QDF_STATUS_E_FAULT;
		goto end;
	}

	WMA_LOGA("WMA --> wmi_unified_connect_htc_service - success");

	/* Trigger the CFG DOWNLOAD */
	wma_msg.type = WNI_CFG_DNLD_REQ;
	wma_msg.bodyptr = NULL;
	wma_msg.bodyval = 0;

	qdf_status = cds_mq_post_message(CDS_MQ_ID_WMA, &wma_msg);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		WMA_LOGP("%s: Failed to post WNI_CFG_DNLD_REQ msg", __func__);
		QDF_ASSERT(0);
		qdf_status = QDF_STATUS_E_FAILURE;
	}
end:
	WMA_LOGD("%s: Exit", __func__);
	return qdf_status;
}

/**
 * wma_send_msg() - Send wma message to PE.
 * @wma_handle: wma handle
 * @msg_type: message type
 * @body_ptr: message body ptr
 * @body_val: message body value
 *
 * Return: none
 */
void wma_send_msg(tp_wma_handle wma_handle, uint16_t msg_type,
			 void *body_ptr, uint32_t body_val)
{
	tSirMsgQ msg = { 0 };
	uint32_t status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = cds_get_context(QDF_MODULE_ID_PE);
	msg.type = msg_type;
	msg.bodyval = body_val;
	msg.bodyptr = body_ptr;
	status = lim_post_msg_api(pMac, &msg);
	if (QDF_STATUS_SUCCESS != status) {
		if (NULL != body_ptr)
			qdf_mem_free(body_ptr);
		QDF_ASSERT(0);
	}
	return;
}

/**
 * wma_set_base_macaddr_indicate() - set base mac address in fw
 * @wma_handle: wma handle
 * @customAddr: base mac address
 *
 * Return: 0 for success or error code
 */
static int wma_set_base_macaddr_indicate(tp_wma_handle wma_handle,
					 tSirMacAddr *customAddr)
{
	int err;

	err = wmi_unified_set_base_macaddr_indicate_cmd(wma_handle->wmi_handle,
				     (uint8_t *)customAddr);
	if (err)
		return -EIO;
	WMA_LOGD("Base MAC Addr: " MAC_ADDRESS_STR,
		 MAC_ADDR_ARRAY((*customAddr)));

	return 0;
}

/**
 * wma_log_supported_evt_handler() - Enable/Disable FW diag/log events
 * @handle: WMA handle
 * @event:  Event received from FW
 * @len:    Length of the event
 *
 * Enables the low frequency events and disables the high frequency
 * events. Bit 17 indicates if the event if low/high frequency.
 * 1 - high frequency, 0 - low frequency
 *
 * Return: 0 on successfully enabling/disabling the events
 */
static int wma_log_supported_evt_handler(void *handle,
		uint8_t *event,
		uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;

	if (wmi_unified_log_supported_evt_cmd(wma->wmi_handle,
				event, len))
		return -EINVAL;

	return 0;
}

/**
 * wma_flush_complete_evt_handler() - FW log flush complete event handler
 * @handle: WMI handle
 * @event:  Event recevied from FW
 * @len:    Length of the event
 *
 */
static int wma_flush_complete_evt_handler(void *handle,
		u_int8_t *event,
		u_int32_t len)
{
	QDF_STATUS status;
	tp_wma_handle wma = (tp_wma_handle) handle;

	WMI_DEBUG_MESG_FLUSH_COMPLETE_EVENTID_param_tlvs *param_buf;
	wmi_debug_mesg_flush_complete_fixed_param *wmi_event;
	uint32_t reason_code;

	param_buf = (WMI_DEBUG_MESG_FLUSH_COMPLETE_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("Invalid log flush complete event buffer");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_event = param_buf->fixed_param;
	reason_code = wmi_event->reserved0;

	/*
	 * reason_code = 0; Flush event in response to flush command
	 * reason_code = other value; Asynchronous flush event for fatal events
	 */
	if (!reason_code && (cds_is_log_report_in_progress() == false)) {
		WMA_LOGE("Received WMI flush event without sending CMD");
		return -EINVAL;
	} else if (!reason_code && cds_is_log_report_in_progress() == true) {
		/* Flush event in response to flush command */
		WMA_LOGI("Received WMI flush event in response to flush CMD");
		status = qdf_mc_timer_stop(&wma->log_completion_timer);
		if (status != QDF_STATUS_SUCCESS)
			WMA_LOGE("Failed to stop the log completion timeout");
		cds_logging_set_fw_flush_complete();
	} else if (reason_code && cds_is_log_report_in_progress() == false) {
		/* Asynchronous flush event for fatal events */
		WMA_LOGE("Received asynchronous WMI flush event: reason=%d",
				reason_code);
		status = cds_set_log_completion(WLAN_LOG_TYPE_FATAL,
				WLAN_LOG_INDICATOR_FIRMWARE,
				reason_code, false);
		if (QDF_STATUS_SUCCESS != status) {
			WMA_LOGE("%s: Failed to set log trigger params",
					__func__);
			return QDF_STATUS_E_FAILURE;
		}
		cds_logging_set_fw_flush_complete();
		return status;
	} else {
		/* Asynchronous flush event for fatal event,
		 * but, report in progress already
		 */
		WMA_LOGI("%s: Bug report already in progress - dropping! type:%d, indicator=%d reason_code=%d",
				__func__, WLAN_LOG_TYPE_FATAL,
				WLAN_LOG_INDICATOR_FIRMWARE, reason_code);
		return QDF_STATUS_E_FAILURE;
	}
	return 0;
}

/**
 * wma_pdev_set_hw_mode_resp_evt_handler() - Set HW mode resp evt handler
 * @handle: WMI handle
 * @event:  Event recevied from FW
 * @len:    Length of the event
 *
 * Event handler for WMI_PDEV_SET_HW_MODE_RESP_EVENTID that is sent to host
 * driver in response to a WMI_PDEV_SET_HW_MODE_CMDID being sent to WLAN
 * firmware
 *
 * Return: Success on receiving valid params from FW
 */
static int wma_pdev_set_hw_mode_resp_evt_handler(void *handle,
		uint8_t *event,
		uint32_t len)
{
	WMI_PDEV_SET_HW_MODE_RESP_EVENTID_param_tlvs *param_buf;
	wmi_pdev_set_hw_mode_response_event_fixed_param *wmi_event;
	wmi_pdev_set_hw_mode_response_vdev_mac_entry *vdev_mac_entry;
	uint32_t i;
	struct sir_set_hw_mode_resp *hw_mode_resp;
	tp_wma_handle wma = (tp_wma_handle) handle;

	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		/* Since WMA handle itself is NULL, we cannot send fail
		 * response back to LIM here
		 */
		return QDF_STATUS_E_NULL_VALUE;
	}

	hw_mode_resp = qdf_mem_malloc(sizeof(*hw_mode_resp));
	if (!hw_mode_resp) {
		WMA_LOGI("%s: Memory allocation failed", __func__);
		/* Since this memory allocation itself failed, we cannot
		 * send fail response back to LIM here
		 */
		return QDF_STATUS_E_NULL_VALUE;
	}

	param_buf = (WMI_PDEV_SET_HW_MODE_RESP_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGE("Invalid WMI_PDEV_SET_HW_MODE_RESP_EVENTID event");
		/* Need to send response back to upper layer to free
		 * active command list
		 */
		goto fail;
	}

	wmi_event = param_buf->fixed_param;
	hw_mode_resp->status = wmi_event->status;
	hw_mode_resp->cfgd_hw_mode_index = wmi_event->cfgd_hw_mode_index;
	hw_mode_resp->num_vdev_mac_entries = wmi_event->num_vdev_mac_entries;

	WMA_LOGI("%s: status:%d cfgd_hw_mode_index:%d num_vdev_mac_entries:%d",
			__func__, wmi_event->status,
			wmi_event->cfgd_hw_mode_index,
			wmi_event->num_vdev_mac_entries);
	vdev_mac_entry =
		param_buf->wmi_pdev_set_hw_mode_response_vdev_mac_mapping;

	/* Store the vdev-mac map in WMA and prepare to send to PE  */
	for (i = 0; i < wmi_event->num_vdev_mac_entries; i++) {
		uint32_t vdev_id, mac_id, pdev_id;
		vdev_id = vdev_mac_entry[i].vdev_id;
		pdev_id = vdev_mac_entry[i].pdev_id;
		if (pdev_id == WMI_PDEV_ID_SOC) {
			WMA_LOGE("%s: soc level id received for mac id)",
				__func__);
			QDF_BUG(0);
			goto fail;
		}
		mac_id = WMA_PDEV_TO_MAC_MAP(vdev_mac_entry[i].pdev_id);

		WMA_LOGI("%s: vdev_id:%d mac_id:%d",
			__func__, vdev_id, mac_id);

		hw_mode_resp->vdev_mac_map[i].vdev_id = vdev_id;
		hw_mode_resp->vdev_mac_map[i].mac_id = mac_id;
		wma_update_intf_hw_mode_params(vdev_id, mac_id,
				wmi_event->cfgd_hw_mode_index);
	}

	if (hw_mode_resp->status == SET_HW_MODE_STATUS_OK) {
		if (WMA_DEFAULT_HW_MODE_INDEX == wma->new_hw_mode_index) {
			wma->new_hw_mode_index = wmi_event->cfgd_hw_mode_index;
		} else {
			wma->old_hw_mode_index = wma->new_hw_mode_index;
			wma->new_hw_mode_index = wmi_event->cfgd_hw_mode_index;
		}
	}

	WMA_LOGI("%s: Updated: old_hw_mode_index:%d new_hw_mode_index:%d",
		__func__, wma->old_hw_mode_index, wma->new_hw_mode_index);

	wma_send_msg(wma, SIR_HAL_PDEV_SET_HW_MODE_RESP,
		     (void *) hw_mode_resp, 0);

	return QDF_STATUS_SUCCESS;

fail:
	WMA_LOGE("%s: Sending fail response to LIM", __func__);
	hw_mode_resp->status = SET_HW_MODE_STATUS_ECANCELED;
	hw_mode_resp->cfgd_hw_mode_index = 0;
	hw_mode_resp->num_vdev_mac_entries = 0;
	wma_send_msg(wma, SIR_HAL_PDEV_SET_HW_MODE_RESP,
			(void *) hw_mode_resp, 0);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wma_pdev_hw_mode_transition_evt_handler() - HW mode transition evt handler
 * @handle: WMI handle
 * @event:  Event recevied from FW
 * @len:    Length of the event
 *
 * Event handler for WMI_PDEV_HW_MODE_TRANSITION_EVENTID that indicates an
 * asynchronous hardware mode transition. This event notifies the host driver
 * that firmware independently changed the hardware mode for some reason, such
 * as Coex, LFR 3.0, etc
 *
 * Return: Success on receiving valid params from FW
 */
static int wma_pdev_hw_mode_transition_evt_handler(void *handle,
		uint8_t *event,
		uint32_t len)
{
	uint32_t i;
	WMI_PDEV_HW_MODE_TRANSITION_EVENTID_param_tlvs *param_buf;
	wmi_pdev_hw_mode_transition_event_fixed_param *wmi_event;
	wmi_pdev_set_hw_mode_response_vdev_mac_entry *vdev_mac_entry;
	struct sir_hw_mode_trans_ind *hw_mode_trans_ind;
	tp_wma_handle wma = (tp_wma_handle) handle;

	if (!wma) {
		/* This is an async event. So, not sending any event to LIM */
		WMA_LOGE("Invalid WMA handle");
		return QDF_STATUS_E_NULL_VALUE;
	}

	param_buf = (WMI_PDEV_HW_MODE_TRANSITION_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		/* This is an async event. So, not sending any event to LIM */
		WMA_LOGE("Invalid WMI_PDEV_HW_MODE_TRANSITION_EVENTID event");
		return QDF_STATUS_E_FAILURE;
	}

	hw_mode_trans_ind = qdf_mem_malloc(sizeof(*hw_mode_trans_ind));
	if (!hw_mode_trans_ind) {
		WMA_LOGI("%s: Memory allocation failed", __func__);
		return QDF_STATUS_E_NULL_VALUE;
	}

	wmi_event = param_buf->fixed_param;
	hw_mode_trans_ind->old_hw_mode_index = wmi_event->old_hw_mode_index;
	hw_mode_trans_ind->new_hw_mode_index = wmi_event->new_hw_mode_index;
	hw_mode_trans_ind->num_vdev_mac_entries =
						wmi_event->num_vdev_mac_entries;
	WMA_LOGI("%s: old_hw_mode_index:%d new_hw_mode_index:%d entries=%d",
		__func__, wmi_event->old_hw_mode_index,
		wmi_event->new_hw_mode_index, wmi_event->num_vdev_mac_entries);

	vdev_mac_entry =
		param_buf->wmi_pdev_set_hw_mode_response_vdev_mac_mapping;

	/* Store the vdev-mac map in WMA and prepare to send to HDD  */
	for (i = 0; i < wmi_event->num_vdev_mac_entries; i++) {
		uint32_t vdev_id, mac_id, pdev_id;
		vdev_id = vdev_mac_entry[i].vdev_id;
		pdev_id = vdev_mac_entry[i].pdev_id;

		if (pdev_id == WMI_PDEV_ID_SOC) {
			WMA_LOGE("%s: soc level id received for mac id)",
					__func__);
			QDF_BUG(0);
			return QDF_STATUS_E_FAILURE;
		}

		mac_id = WMA_PDEV_TO_MAC_MAP(vdev_mac_entry[i].pdev_id);

		WMA_LOGI("%s: vdev_id:%d mac_id:%d",
				__func__, vdev_id, mac_id);

		hw_mode_trans_ind->vdev_mac_map[i].vdev_id = vdev_id;
		hw_mode_trans_ind->vdev_mac_map[i].mac_id = mac_id;
		wma_update_intf_hw_mode_params(vdev_id, mac_id,
				wmi_event->new_hw_mode_index);
	}
	wma->old_hw_mode_index = wmi_event->old_hw_mode_index;
	wma->new_hw_mode_index = wmi_event->new_hw_mode_index;

	WMA_LOGI("%s: Updated: old_hw_mode_index:%d new_hw_mode_index:%d",
		__func__, wma->old_hw_mode_index, wma->new_hw_mode_index);

	/* Pass the message to PE */
	wma_send_msg(wma, SIR_HAL_PDEV_HW_MODE_TRANS_IND,
		     (void *) hw_mode_trans_ind, 0);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_pdev_set_dual_mode_config_resp_evt_handler() - Dual mode evt handler
 * @handle: WMI handle
 * @event:  Event received from FW
 * @len:    Length of the event
 *
 * Notifies the host driver of the completion or failure of a
 * WMI_PDEV_SET_MAC_CONFIG_CMDID command. This event would be returned to
 * the host driver once the firmware has completed a reconfiguration of the Scan
 * and FW mode configuration. This changes could include entering or leaving a
 * dual mac configuration for either scan and/or more permanent firmware mode.
 *
 * Return: Success on receiving valid params from FW
 */
static int wma_pdev_set_dual_mode_config_resp_evt_handler(void *handle,
		uint8_t *event,
		uint32_t len)
{
	WMI_PDEV_SET_MAC_CONFIG_RESP_EVENTID_param_tlvs *param_buf;
	wmi_pdev_set_mac_config_response_event_fixed_param *wmi_event;
	tp_wma_handle wma = (tp_wma_handle) handle;
	struct sir_dual_mac_config_resp *dual_mac_cfg_resp;

	if (!wma) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		/* Since the WMA handle is NULL, we cannot send resp to LIM.
		 * So, returning from here.
		 */
		return QDF_STATUS_E_NULL_VALUE;
	}

	dual_mac_cfg_resp = qdf_mem_malloc(sizeof(*dual_mac_cfg_resp));
	if (!dual_mac_cfg_resp) {
		WMA_LOGE("%s: Memory allocation failed", __func__);
		/* Since the mem alloc failed, we cannot send resp to LIM.
		 * So, returning from here.
		 */
		return QDF_STATUS_E_NULL_VALUE;
	}

	param_buf = (WMI_PDEV_SET_MAC_CONFIG_RESP_EVENTID_param_tlvs *)
		event;
	if (!param_buf) {
		WMA_LOGE("%s: Invalid event", __func__);
		goto fail;
	}

	wmi_event = param_buf->fixed_param;
	WMA_LOGI("%s: status:%d", __func__, wmi_event->status);
	dual_mac_cfg_resp->status = wmi_event->status;

	if (SET_HW_MODE_STATUS_OK == dual_mac_cfg_resp->status) {
		wma->dual_mac_cfg.prev_scan_config =
			wma->dual_mac_cfg.cur_scan_config;
		wma->dual_mac_cfg.prev_fw_mode_config =
			wma->dual_mac_cfg.cur_fw_mode_config;
		wma->dual_mac_cfg.cur_scan_config =
			wma->dual_mac_cfg.req_scan_config;
		wma->dual_mac_cfg.cur_fw_mode_config =
			wma->dual_mac_cfg.req_fw_mode_config;
	}

	/* Pass the message to PE */
	wma_send_msg(wma, SIR_HAL_PDEV_MAC_CFG_RESP,
			(void *) dual_mac_cfg_resp, 0);

	return QDF_STATUS_SUCCESS;

fail:
	WMA_LOGE("%s: Sending fail response to LIM", __func__);
	dual_mac_cfg_resp->status = SET_HW_MODE_STATUS_ECANCELED;
	wma_send_msg(wma, SIR_HAL_PDEV_MAC_CFG_RESP,
			(void *) dual_mac_cfg_resp, 0);

	return QDF_STATUS_E_FAILURE;

}

/**
 * wma_start() - wma start function.
 *               Intialize event handlers and timers.
 * @cds_ctx: cds context
 *
 * Return: 0 on success, QDF Error on failure
 */
QDF_STATUS wma_start(void *cds_ctx)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tp_wma_handle wma_handle;
	int status;
	WMA_LOGD("%s: Enter", __func__);

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	/* validate the wma_handle */
	if (NULL == wma_handle) {
		WMA_LOGP("%s: Invalid handle", __func__);
		qdf_status = QDF_STATUS_E_INVAL;
		goto end;
	}

	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						    WMI_SCAN_EVENTID,
						    wma_scan_event_callback,
						    WMA_RX_SERIALIZER_CTX);
	if (0 != status) {
		WMA_LOGP("%s: Failed to register scan callback", __func__);
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						    WMI_ROAM_EVENTID,
						    wma_roam_event_callback,
						    WMA_RX_WORK_CTX);
	if (0 != status) {
		WMA_LOGP("%s: Failed to register Roam callback", __func__);
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						    WMI_WOW_WAKEUP_HOST_EVENTID,
						    wma_wow_wakeup_host_event,
						    WMA_RX_TASKLET_CTX);
	if (status) {
		WMA_LOGP("%s: Failed to register wow wakeup host event handler",
			 __func__);
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
				WMI_PDEV_RESUME_EVENTID,
				wma_pdev_resume_event_handler,
				WMA_RX_TASKLET_CTX);
	if (status) {
		WMA_LOGP("%s: Failed to register PDEV resume event handler",
			 __func__);
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}

#ifdef FEATURE_WLAN_SCAN_PNO
	if (WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				   WMI_SERVICE_NLO)) {

		WMA_LOGD("FW supports pno offload, registering nlo match handler");

		status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						WMI_NLO_MATCH_EVENTID,
						wma_nlo_match_evt_handler,
						WMA_RX_SERIALIZER_CTX);
		if (status) {
			WMA_LOGE("Failed to register nlo match event cb");
			qdf_status = QDF_STATUS_E_FAILURE;
			goto end;
		}

		status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						WMI_NLO_SCAN_COMPLETE_EVENTID,
						wma_nlo_scan_cmp_evt_handler,
						WMA_RX_SERIALIZER_CTX);
		if (status) {
			WMA_LOGE("Failed to register nlo scan comp event cb");
			qdf_status = QDF_STATUS_E_FAILURE;
			goto end;
		}
	}
#endif /* FEATURE_WLAN_SCAN_PNO */

#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
	WMA_LOGE("MCC TX Pause Event Handler register");
	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
					WMI_TX_PAUSE_EVENTID,
					wma_mcc_vdev_tx_pause_evt_handler,
					WMA_RX_TASKLET_CTX);
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

#ifdef FEATURE_WLAN_CH_AVOID
	WMA_LOGD("Registering channel to avoid handler");

	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						WMI_WLAN_FREQ_AVOID_EVENTID,
						wma_channel_avoid_evt_handler,
						WMA_RX_SERIALIZER_CTX);
	if (status) {
		WMA_LOGE("Failed to register channel to avoid event cb");
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}
#endif /* FEATURE_WLAN_CH_AVOID */
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	WMA_LOGD("Registering auto shutdown handler");
	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						WMI_HOST_AUTO_SHUTDOWN_EVENTID,
						wma_auto_shutdown_event_handler,
						WMA_RX_SERIALIZER_CTX);
	if (status) {
		WMA_LOGE("Failed to register WMI Auto shutdown event handler");
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}
#endif /* FEATURE_WLAN_AUTO_SHUTDOWN */
	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						WMI_THERMAL_MGMT_EVENTID,
						wma_thermal_mgmt_evt_handler,
						WMA_RX_SERIALIZER_CTX);
	if (status) {
		WMA_LOGE("Failed to register thermal mitigation event cb");
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	status = wma_ocb_register_event_handlers(wma_handle);
	if (status) {
		WMA_LOGE("Failed to register ocb event handlers");
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	qdf_status = QDF_STATUS_SUCCESS;

#ifdef QCA_WIFI_FTM
	/*
	 * Tx mgmt attach requires TXRX context which is not created
	 * in FTM mode. So skip the TX mgmt attach.
	 */
	if (cds_get_conparam() == QDF_GLOBAL_FTM_MODE)
		goto end;
#endif /* QCA_WIFI_FTM */

	if (WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				   WMI_SERVICE_RMC)) {

		WMA_LOGD("FW supports cesium network, registering event handlers");

		status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
							   WMI_PEER_INFO_EVENTID,
							   wma_ibss_peer_info_event_handler,
							   WMA_RX_SERIALIZER_CTX);
		if (status) {
			WMA_LOGE("Failed to register ibss peer info event cb");
			qdf_status = QDF_STATUS_E_FAILURE;
			goto end;
		}

		status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
							   WMI_PEER_TX_FAIL_CNT_THR_EVENTID,
							   wma_fast_tx_fail_event_handler,
							   WMA_RX_SERIALIZER_CTX);
		if (status) {
			WMA_LOGE("Failed to register peer fast tx failure event cb");
			qdf_status = QDF_STATUS_E_FAILURE;
			goto end;
		}
	} else {
		WMA_LOGE("Target does not support cesium network");
	}

	qdf_status = wma_tx_attach(wma_handle);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMA_LOGP("%s: Failed to register tx management", __func__);
		goto end;
	}

	/* Initialize log completion timeout */
	qdf_status = qdf_mc_timer_init(&wma_handle->log_completion_timer,
			QDF_TIMER_TYPE_SW,
			wma_log_completion_timeout,
			wma_handle);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to initialize log completion timeout");
		goto end;
	}

	/* Initialize the get temperature event handler */
	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
					WMI_PDEV_TEMPERATURE_EVENTID,
					wma_pdev_temperature_evt_handler,
					WMA_RX_SERIALIZER_CTX);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to register get_temperature event cb");
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						WMI_VDEV_TSF_REPORT_EVENTID,
						wma_vdev_tsf_handler,
						WMA_RX_SERIALIZER_CTX);
	if (0 != status) {
		WMA_LOGP("%s: Failed to register tsf callback", __func__);
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	/* Initialize the log flush complete event handler */
	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
			WMI_DEBUG_MESG_FLUSH_COMPLETE_EVENTID,
			wma_flush_complete_evt_handler,
			WMA_RX_SERIALIZER_CTX);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to register log flush complete event cb");
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	/* Initialize the wma_pdev_set_hw_mode_resp_evt_handler event handler */
	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
			WMI_PDEV_SET_HW_MODE_RESP_EVENTID,
			wma_pdev_set_hw_mode_resp_evt_handler,
			WMA_RX_SERIALIZER_CTX);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to register set hw mode resp event cb");
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	/* Initialize the WMI_SOC_HW_MODE_TRANSITION_EVENTID event handler */
	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
			WMI_PDEV_HW_MODE_TRANSITION_EVENTID,
			wma_pdev_hw_mode_transition_evt_handler,
			WMA_RX_SERIALIZER_CTX);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to register hw mode transition event cb");
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	/* Initialize the set dual mac configuration event handler */
	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
			WMI_PDEV_SET_MAC_CONFIG_RESP_EVENTID,
			wma_pdev_set_dual_mode_config_resp_evt_handler,
			WMA_RX_SERIALIZER_CTX);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to register hw mode transition event cb");
		qdf_status = QDF_STATUS_E_FAILURE;
		goto end;
	}

end:
	WMA_LOGD("%s: Exit", __func__);
	return qdf_status;
}

/**
 * wma_stop() - wma stop function.
 *              cleanup timers and suspend target.
 * @cds_ctx: cds context
 * @reason: reason for wma_stop.
 *
 * Return: 0 on success, QDF Error on failure
 */
QDF_STATUS wma_stop(void *cds_ctx, uint8_t reason)
{
	tp_wma_handle wma_handle;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	int i;
	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	WMA_LOGD("%s: Enter", __func__);

	/* validate the wma_handle */
	if (NULL == wma_handle) {
		WMA_LOGP("%s: Invalid handle", __func__);
		qdf_status = QDF_STATUS_E_INVAL;
		goto end;
	}
#ifdef QCA_WIFI_FTM
	/*
	 * Tx mgmt detach requires TXRX context which is not created
	 * in FTM mode. So skip the TX mgmt detach.
	 */
	if (cds_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		qdf_status = QDF_STATUS_SUCCESS;
		goto end;
	}
#endif /* QCA_WIFI_FTM */

	if (wma_handle->ack_work_ctx) {
		cds_flush_work(&wma_handle->ack_work_ctx->ack_cmp_work);
		qdf_mem_free(wma_handle->ack_work_ctx);
		wma_handle->ack_work_ctx = NULL;
	}

	/* Destroy the timer for log completion */
	qdf_status = qdf_mc_timer_destroy(&wma_handle->log_completion_timer);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to destroy the log completion timer");
	}

	/* There's no need suspend target which is already down during SSR. */
	if (!cds_is_driver_recovering()) {
#ifdef HIF_USB
		/* Suspend the target and enable interrupt */
		if (wma_suspend_target(wma_handle, 0))
			WMA_LOGE("Failed to suspend target");
#else
		/* Suspend the target and disable interrupt */
		if (wma_suspend_target(wma_handle, 1))
			WMA_LOGE("Failed to suspend target");
#endif /* HIF_USB */
	}

	/* clean up ll-queue for all vdev */
	for (i = 0; i < wma_handle->max_bssid; i++) {
		if (wma_handle->interfaces[i].handle &&
				wma_handle->interfaces[i].vdev_up) {
			ol_txrx_vdev_flush(wma_handle->interfaces[i].handle);
		}
	}
	qdf_status = wma_tx_detach(wma_handle);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		WMA_LOGP("%s: Failed to deregister tx management", __func__);
		goto end;
	}

end:
	WMA_LOGD("%s: Exit", __func__);
	return qdf_status;
}

/**
 * wma_cleanup_hold_req() - cleanup hold request queue
 * @wma: wma handle
 *
 * Return: none
 */
static void wma_cleanup_hold_req(tp_wma_handle wma)
{
	struct wma_target_req *req_msg = NULL;
	qdf_list_node_t *node1 = NULL;
	QDF_STATUS status;

	qdf_spin_lock_bh(&wma->wma_hold_req_q_lock);
	if (!qdf_list_size(&wma->wma_hold_req_queue)) {
		qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
		WMA_LOGI(FL("request queue is empty"));
		return;
	}

	while (QDF_STATUS_SUCCESS !=
			qdf_list_peek_front(&wma->wma_hold_req_queue, &node1)) {
		req_msg = qdf_container_of(node1, struct wma_target_req, node);
		status = qdf_list_remove_node(&wma->wma_hold_req_queue, node1);
		if (QDF_STATUS_SUCCESS != status) {
			qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
			WMA_LOGE(FL("Failed to remove request for vdev_id %d type %d"),
				 req_msg->vdev_id, req_msg->type);
			return;
		}
		qdf_mc_timer_destroy(&req_msg->event_timeout);
		qdf_mem_free(req_msg);
	}
	qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
}

/**
 * wma_cleanup_vdev_resp() - cleanup vdev response queue
 * @wma: wma handle
 *
 * Return: none
 */
static void wma_cleanup_vdev_resp(tp_wma_handle wma)
{
	struct wma_target_req *req_msg = NULL;
	qdf_list_node_t *node1 = NULL;
	QDF_STATUS status;

	qdf_spin_lock_bh(&wma->vdev_respq_lock);
	if (!qdf_list_size(&wma->vdev_resp_queue)) {
		qdf_spin_unlock_bh(&wma->vdev_respq_lock);
		WMA_LOGI(FL("request queue maybe empty"));
		return;
	}

	while (QDF_STATUS_SUCCESS != qdf_list_peek_front(&wma->vdev_resp_queue,
						      &node1)) {
		req_msg = qdf_container_of(node1, struct wma_target_req, node);
		status = qdf_list_remove_node(&wma->vdev_resp_queue, node1);
		if (QDF_STATUS_SUCCESS != status) {
			qdf_spin_unlock_bh(&wma->vdev_respq_lock);
			WMA_LOGE(FL("Failed to remove request for vdev_id %d type %d"),
				 req_msg->vdev_id, req_msg->type);
			return;
		}
		qdf_mc_timer_destroy(&req_msg->event_timeout);
		qdf_mem_free(req_msg);
	}
	qdf_spin_unlock_bh(&wma->vdev_respq_lock);
}

/**
 * wma_wmi_service_close() - close wma wmi service interface.
 * @cds_ctx: cds context
 *
 * Return: 0 on success, QDF Error on failure
 */
QDF_STATUS wma_wmi_service_close(void *cds_ctx)
{
	tp_wma_handle wma_handle;
	struct beacon_info *bcn;
	int i;

	WMA_LOGD("%s: Enter", __func__);

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	/* validate the wma_handle */
	if (NULL == wma_handle) {
		WMA_LOGE("%s: Invalid wma handle", __func__);
		return QDF_STATUS_E_INVAL;
	}

	/* validate the wmi handle */
	if (NULL == wma_handle->wmi_handle) {
		WMA_LOGE("%s: Invalid wmi handle", __func__);
		return QDF_STATUS_E_INVAL;
	}

	/* dettach the wmi serice */
	WMA_LOGD("calling wmi_unified_detach");
	wmi_unified_detach(wma_handle->wmi_handle);
	wma_handle->wmi_handle = NULL;

	for (i = 0; i < wma_handle->max_bssid; i++) {
		bcn = wma_handle->interfaces[i].beacon;

		if (bcn) {
			if (bcn->dma_mapped)
				qdf_nbuf_unmap_single(wma_handle->qdf_dev,
					bcn->buf, QDF_DMA_TO_DEVICE);
			qdf_nbuf_free(bcn->buf);
			qdf_mem_free(bcn);
			wma_handle->interfaces[i].beacon = NULL;
		}

		if (wma_handle->interfaces[i].handle) {
			qdf_mem_free(wma_handle->interfaces[i].handle);
			wma_handle->interfaces[i].handle = NULL;
		}

		if (wma_handle->interfaces[i].addBssStaContext) {
			qdf_mem_free(wma_handle->
				     interfaces[i].addBssStaContext);
			wma_handle->interfaces[i].addBssStaContext = NULL;
		}

		if (wma_handle->interfaces[i].del_staself_req) {
			qdf_mem_free(wma_handle->interfaces[i].del_staself_req);
			wma_handle->interfaces[i].del_staself_req = NULL;
		}
	}

	qdf_mem_free(wma_handle->interfaces);
	/* free the wma_handle */
	cds_free_context(wma_handle->cds_context, QDF_MODULE_ID_WMA,
			 wma_handle);

	qdf_mem_free(((p_cds_contextType) cds_ctx)->cfg_ctx);
	WMA_LOGD("%s: Exit", __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_wmi_work_close() - close the work queue items associated with WMI
 * @cds_ctx:	Pointer to cds context
 *
 * This function closes work queue items associated with WMI, but not fully
 * closes WMI service.
 *
 * Return: QDF_STATUS_SUCCESS if work close is successful. Otherwise
 *	proper error codes.
 */
QDF_STATUS wma_wmi_work_close(void *cds_ctx)
{
	tp_wma_handle wma_handle;

	WMA_LOGD("%s: Enter", __func__);

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	/* validate the wma_handle */
	if (NULL == wma_handle) {
		WMA_LOGE("%s: Invalid wma handle", __func__);
		return QDF_STATUS_E_INVAL;
	}

	/* validate the wmi handle */
	if (NULL == wma_handle->wmi_handle) {
		WMA_LOGE("%s: Invalid wmi handle", __func__);
		return QDF_STATUS_E_INVAL;
	}

	/* remove the wmi work */
	WMA_LOGD("calling wmi_unified_remove_work");
	wmi_unified_remove_work(wma_handle->wmi_handle);

	return QDF_STATUS_SUCCESS;
}


/**
 * wma_close() - wma close function.
 *               cleanup resources attached with wma.
 * @cds_ctx: cds context
 *
 * Return: 0 on success, QDF Error on failure
 */
QDF_STATUS wma_close(void *cds_ctx)
{
	tp_wma_handle wma_handle;
	uint32_t idx;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	WMA_LOGD("%s: Enter", __func__);

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	/* validate the wma_handle */
	if (NULL == wma_handle) {
		WMA_LOGE("%s: Invalid wma handle", __func__);
		return QDF_STATUS_E_INVAL;
	}

	/* validate the wmi handle */
	if (NULL == wma_handle->wmi_handle) {
		WMA_LOGP("%s: Invalid wmi handle", __func__);
		return QDF_STATUS_E_INVAL;
	}

	/* Free DBS list */
	if (wma_handle->hw_mode.hw_mode_list) {
		qdf_mem_free(wma_handle->hw_mode.hw_mode_list);
		wma_handle->hw_mode.hw_mode_list = NULL;
		WMA_LOGI("%s: DBS list is freed", __func__);
	}

	if (cds_get_conparam() != QDF_GLOBAL_FTM_MODE) {
#ifdef FEATURE_WLAN_SCAN_PNO
		qdf_wake_lock_destroy(&wma_handle->pno_wake_lock);
#endif /* FEATURE_WLAN_SCAN_PNO */
#ifdef FEATURE_WLAN_EXTSCAN
		qdf_wake_lock_destroy(&wma_handle->extscan_wake_lock);
#endif /* FEATURE_WLAN_EXTSCAN */
		qdf_wake_lock_destroy(&wma_handle->wow_wake_lock);
	}

	/* unregister Firmware debug log */
	qdf_status = dbglog_deinit(wma_handle->wmi_handle);
	if (qdf_status != QDF_STATUS_SUCCESS)
		WMA_LOGP("%s: dbglog_deinit failed", __func__);

	/* close the qdf events */
	qdf_event_destroy(&wma_handle->wma_ready_event);
	qdf_status = qdf_mc_timer_destroy(&wma_handle->service_ready_ext_timer);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		WMA_LOGP("%s: Failed to destroy service ready ext event timer",
			__func__);

	qdf_event_destroy(&wma_handle->target_suspend);
	qdf_event_destroy(&wma_handle->wma_resume_event);
	qdf_event_destroy(&wma_handle->runtime_suspend);
	qdf_event_destroy(&wma_handle->recovery_event);
	wma_cleanup_vdev_resp(wma_handle);
	wma_cleanup_hold_req(wma_handle);
	qdf_wake_lock_destroy(&wma_handle->wmi_cmd_rsp_wake_lock);
	qdf_runtime_lock_deinit(wma_handle->wmi_cmd_rsp_runtime_lock);
	for (idx = 0; idx < wma_handle->num_mem_chunks; ++idx) {
		qdf_mem_free_consistent(wma_handle->qdf_dev,
					wma_handle->qdf_dev->dev,
					   wma_handle->mem_chunks[idx].len,
					   wma_handle->mem_chunks[idx].vaddr,
					   wma_handle->mem_chunks[idx].paddr,
					   qdf_get_dma_mem_context(
						(&(wma_handle->mem_chunks[idx])),
								   memctx));
	}

#if defined(QCA_WIFI_FTM)
	/* Detach UTF and unregister the handler */
	if (cds_get_conparam() == QDF_GLOBAL_FTM_MODE)
		wma_utf_detach(wma_handle);
#endif /* QCA_WIFI_FTM */

	if (NULL != wma_handle->dfs_ic) {
		wma_dfs_detach(wma_handle->dfs_ic);
		wma_handle->dfs_ic = NULL;
	}

	if (NULL != wma_handle->pGetRssiReq) {
		qdf_mem_free(wma_handle->pGetRssiReq);
		wma_handle->pGetRssiReq = NULL;
	}
	if (WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				   WMI_SERVICE_MGMT_TX_WMI)) {
		wmi_desc_pool_deinit(wma_handle);
	}

	WMA_LOGD("%s: Exit", __func__);
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_update_fw_config() - update fw configuration
 * @wma_handle: wma handle
 * @tgt_cap: target capabality
 *
 * Return: none
 */
static void wma_update_fw_config(tp_wma_handle wma_handle,
				 struct wma_target_cap *tgt_cap)
{
	/*
	 * tgt_cap contains default target resource configuration
	 * which can be modified here, if required
	 */
	/* Override the no. of max fragments as per platform configuration */
	tgt_cap->wlan_resource_config.max_frag_entries =
					QDF_MIN(QCA_OL_11AC_TX_MAX_FRAGS,
						wma_handle->max_frag_entry);
	wma_handle->max_frag_entry =
		tgt_cap->wlan_resource_config.max_frag_entries;
}

/**
 * wma_alloc_host_mem_chunk() - allocate host memory
 * @wma_handle: wma handle
 * @req_id: request id
 * @idx: index
 * @num_units: number of units
 * @unit_len: unit length
 *
 * allocate a chunk of memory at the index indicated and
 * if allocation fail allocate smallest size possiblr and
 * return number of units allocated.
 *
 * Return: number of units or 0 for error.
 */
static uint32_t wma_alloc_host_mem_chunk(tp_wma_handle wma_handle,
					 uint32_t req_id, uint32_t idx,
					 uint32_t num_units, uint32_t unit_len)
{
	qdf_dma_addr_t paddr;
	if (!num_units || !unit_len) {
		return 0;
	}
	wma_handle->mem_chunks[idx].vaddr = NULL;
	/** reduce the requested allocation by half until allocation succeeds */
	while (wma_handle->mem_chunks[idx].vaddr == NULL && num_units) {
		wma_handle->mem_chunks[idx].vaddr =
			qdf_mem_alloc_consistent(wma_handle->qdf_dev,
						 wma_handle->qdf_dev->dev,
						    num_units * unit_len,
						    &paddr);
		if (wma_handle->mem_chunks[idx].vaddr == NULL) {
			num_units = (num_units >> 1);/* reduce length by half */
		} else {
			wma_handle->mem_chunks[idx].paddr = paddr;
			wma_handle->mem_chunks[idx].len = num_units * unit_len;
			wma_handle->mem_chunks[idx].req_id = req_id;
		}
	}
	return num_units;
}

#define HOST_MEM_SIZE_UNIT 4
/**
 * wma_alloc_host_mem() - allocate amount of memory requested by FW.
 * @wma_handle: wma handle
 * @req_id: request id
 * @num_units: number of units
 * @unit_len: unit length
 *
 * Return: none
 */
static void wma_alloc_host_mem(tp_wma_handle wma_handle, uint32_t req_id,
			       uint32_t num_units, uint32_t unit_len)
{
	uint32_t remaining_units, allocated_units, idx;

	/* adjust the length to nearest multiple of unit size */
	unit_len = (unit_len + (HOST_MEM_SIZE_UNIT - 1)) &
		   (~(HOST_MEM_SIZE_UNIT - 1));
	idx = wma_handle->num_mem_chunks;
	remaining_units = num_units;
	while (remaining_units) {
		allocated_units = wma_alloc_host_mem_chunk(wma_handle, req_id,
							   idx, remaining_units,
							   unit_len);
		if (allocated_units == 0) {
			WMA_LOGE("FAILED TO ALLOCATED memory unit len %d"
				 " units requested %d units allocated %d ",
				 unit_len, num_units,
				 (num_units - remaining_units));
			wma_handle->num_mem_chunks = idx;
			break;
		}
		remaining_units -= allocated_units;
		++idx;
		if (idx == MAX_MEM_CHUNKS) {
			WMA_LOGE("RWACHED MAX CHUNK LIMIT for memory units %d"
				 " unit len %d requested by FW,"
				 " only allocated %d ",
				 num_units, unit_len,
				 (num_units - remaining_units));
			wma_handle->num_mem_chunks = idx;
			break;
		}
	}
	wma_handle->num_mem_chunks = idx;
}

/**
 * wma_update_target_services() - update target services from wma handle
 * @wh: wma handle
 * @cfg: target services
 *
 * Return: none
 */
static inline void wma_update_target_services(tp_wma_handle wh,
					      struct wma_tgt_services *cfg)
{
	/* STA power save */
	cfg->sta_power_save = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
						     WMI_SERVICE_STA_PWRSAVE);

	/* Enable UAPSD */
	cfg->uapsd = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
					    WMI_SERVICE_AP_UAPSD);

	/* Update AP DFS service */
	cfg->ap_dfs = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
					     WMI_SERVICE_AP_DFS);

	/* Enable 11AC */
	cfg->en_11ac = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
					      WMI_SERVICE_11AC);
	if (cfg->en_11ac)
		g_fw_wlan_feat_caps |= (1 << DOT11AC);

	/* Proactive ARP response */
	g_fw_wlan_feat_caps |= (1 << WLAN_PERIODIC_TX_PTRN);

	/* Enable WOW */
	g_fw_wlan_feat_caps |= (1 << WOW);

	/* ARP offload */
	cfg->arp_offload = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
						  WMI_SERVICE_ARPNS_OFFLOAD);

	/* Adaptive early-rx */
	cfg->early_rx = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
					       WMI_SERVICE_EARLY_RX);
#ifdef FEATURE_WLAN_SCAN_PNO
	/* PNO offload */
	if (WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap, WMI_SERVICE_NLO))
		cfg->pno_offload = true;
#endif /* FEATURE_WLAN_SCAN_PNO */

#ifdef FEATURE_WLAN_EXTSCAN
	if (WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap, WMI_SERVICE_EXTSCAN)) {
		g_fw_wlan_feat_caps |= (1 << EXTENDED_SCAN);
	}
#endif /* FEATURE_WLAN_EXTSCAN */
	cfg->lte_coex_ant_share = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
					WMI_SERVICE_LTE_ANT_SHARE_SUPPORT);
#ifdef FEATURE_WLAN_TDLS
	/* Enable TDLS */
	if (WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap, WMI_SERVICE_TDLS)) {
		cfg->en_tdls = 1;
		g_fw_wlan_feat_caps |= (1 << TDLS);
	}
	/* Enable advanced TDLS features */
	if (WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
				   WMI_SERVICE_TDLS_OFFCHAN)) {
		cfg->en_tdls_offchan = 1;
		g_fw_wlan_feat_caps |= (1 << TDLS_OFF_CHANNEL);
	}

	cfg->en_tdls_uapsd_buf_sta =
		WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
				       WMI_SERVICE_TDLS_UAPSD_BUFFER_STA);
	cfg->en_tdls_uapsd_sleep_sta =
		WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
				       WMI_SERVICE_TDLS_UAPSD_SLEEP_STA);
#endif /* FEATURE_WLAN_TDLS */
	if (WMI_SERVICE_IS_ENABLED
		    (wh->wmi_service_bitmap, WMI_SERVICE_BEACON_OFFLOAD))
		cfg->beacon_offload = true;
	if (WMI_SERVICE_IS_ENABLED
		    (wh->wmi_service_bitmap, WMI_SERVICE_STA_PMF_OFFLOAD))
		cfg->pmf_offload = true;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	/* Enable Roam Offload */
	cfg->en_roam_offload = WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap,
						      WMI_SERVICE_ROAM_HO_OFFLOAD);
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
#ifdef WLAN_FEATURE_NAN
	if (WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap, WMI_SERVICE_NAN))
		g_fw_wlan_feat_caps |= (1 << NAN);
#endif /* WLAN_FEATURE_NAN */

	if (WMI_SERVICE_IS_ENABLED(wh->wmi_service_bitmap, WMI_SERVICE_RTT))
		g_fw_wlan_feat_caps |= (1 << RTT);
}

/**
 * wma_update_target_ht_cap() - update ht capabality from wma handle
 * @wh: wma handle
 * @cfg: ht capabality
 *
 * Return: none
 */
static inline void wma_update_target_ht_cap(tp_wma_handle wh,
					    struct wma_tgt_ht_cap *cfg)
{
	/* RX STBC */
	cfg->ht_rx_stbc = !!(wh->ht_cap_info & WMI_HT_CAP_RX_STBC);

	/* TX STBC */
	cfg->ht_tx_stbc = !!(wh->ht_cap_info & WMI_HT_CAP_TX_STBC);

	/* MPDU density */
	cfg->mpdu_density = wh->ht_cap_info & WMI_HT_CAP_MPDU_DENSITY;

	/* HT RX LDPC */
	cfg->ht_rx_ldpc = !!(wh->ht_cap_info & WMI_HT_CAP_LDPC);

	/* HT SGI */
	cfg->ht_sgi_20 = !!(wh->ht_cap_info & WMI_HT_CAP_HT20_SGI);

	cfg->ht_sgi_40 = !!(wh->ht_cap_info & WMI_HT_CAP_HT40_SGI);

	/* RF chains */
	cfg->num_rf_chains = wh->num_rf_chains;

	WMA_LOGD("%s: ht_cap_info - %x ht_rx_stbc - %d, ht_tx_stbc - %d\n\
		 mpdu_density - %d ht_rx_ldpc - %d ht_sgi_20 - %d\n\
		 ht_sgi_40 - %d num_rf_chains - %d ", __func__, wh->ht_cap_info,
		 cfg->ht_rx_stbc, cfg->ht_tx_stbc, cfg->mpdu_density,
		 cfg->ht_rx_ldpc, cfg->ht_sgi_20, cfg->ht_sgi_40,
		 cfg->num_rf_chains);

}

/**
 * wma_update_target_vht_cap() - update vht capabality from wma handle
 * @wh: wma handle
 * @cfg: vht capabality
 *
 * Return: none
 */
static inline void wma_update_target_vht_cap(tp_wma_handle wh,
					     struct wma_tgt_vht_cap *cfg)
{

	if (wh->vht_cap_info & WMI_VHT_CAP_MAX_MPDU_LEN_11454)
		cfg->vht_max_mpdu = WMI_VHT_CAP_MAX_MPDU_LEN_11454;
	else if (wh->vht_cap_info & WMI_VHT_CAP_MAX_MPDU_LEN_7935)
		cfg->vht_max_mpdu = WMI_VHT_CAP_MAX_MPDU_LEN_7935;
	else
		cfg->vht_max_mpdu = 0;


	if (wh->vht_cap_info & WMI_VHT_CAP_CH_WIDTH_80P80_160MHZ) {
		cfg->supp_chan_width = 1 << eHT_CHANNEL_WIDTH_80P80MHZ;
		cfg->supp_chan_width |= 1 << eHT_CHANNEL_WIDTH_160MHZ;
	} else if (wh->vht_cap_info & WMI_VHT_CAP_CH_WIDTH_160MHZ)
		cfg->supp_chan_width = 1 << eHT_CHANNEL_WIDTH_160MHZ;
	else
		cfg->supp_chan_width = 1 << eHT_CHANNEL_WIDTH_80MHZ;

	cfg->vht_rx_ldpc = wh->vht_cap_info & WMI_VHT_CAP_RX_LDPC;

	cfg->vht_short_gi_80 = wh->vht_cap_info & WMI_VHT_CAP_SGI_80MHZ;
	cfg->vht_short_gi_160 = wh->vht_cap_info & WMI_VHT_CAP_SGI_160MHZ;

	cfg->vht_tx_stbc = wh->vht_cap_info & WMI_VHT_CAP_TX_STBC;

	cfg->vht_rx_stbc = (wh->vht_cap_info & WMI_VHT_CAP_RX_STBC_1SS) |
		(wh->vht_cap_info & WMI_VHT_CAP_RX_STBC_2SS) |
		(wh->vht_cap_info & WMI_VHT_CAP_RX_STBC_3SS) ;

	cfg->vht_max_ampdu_len_exp = (wh->vht_cap_info &
				      WMI_VHT_CAP_MAX_AMPDU_LEN_EXP)
				     >> WMI_VHT_CAP_MAX_AMPDU_LEN_EXP_SHIFT;

	cfg->vht_su_bformer = wh->vht_cap_info & WMI_VHT_CAP_SU_BFORMER;

	cfg->vht_su_bformee = wh->vht_cap_info & WMI_VHT_CAP_SU_BFORMEE;

	cfg->vht_mu_bformer = wh->vht_cap_info & WMI_VHT_CAP_MU_BFORMER;

	cfg->vht_mu_bformee = wh->vht_cap_info & WMI_VHT_CAP_MU_BFORMEE;

	cfg->vht_txop_ps = wh->vht_cap_info & WMI_VHT_CAP_TXOP_PS;

	WMA_LOGD(" %s: max_mpdu %d supp_chan_width %x rx_ldpc %x\n \
		 short_gi_80 %x tx_stbc %x rx_stbc %x txop_ps %x\n \
		 su_bformee %x mu_bformee %x max_ampdu_len_exp %d", __func__,
		 cfg->vht_max_mpdu, cfg->supp_chan_width, cfg->vht_rx_ldpc,
		 cfg->vht_short_gi_80, cfg->vht_tx_stbc, cfg->vht_rx_stbc,
		 cfg->vht_txop_ps, cfg->vht_su_bformee, cfg->vht_mu_bformee,
		 cfg->vht_max_ampdu_len_exp);
}

/**
 * wma_update_hdd_cfg() - update HDD config
 * @wma_handle: wma handle
 *
 * Return: none
 */
static void wma_update_hdd_cfg(tp_wma_handle wma_handle)
{
	struct wma_tgt_cfg tgt_cfg;
	void *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	qdf_mem_zero(&tgt_cfg, sizeof(struct wma_tgt_cfg));
	tgt_cfg.reg_domain = wma_handle->reg_cap.eeprom_rd;
	tgt_cfg.eeprom_rd_ext = wma_handle->reg_cap.eeprom_rd_ext;

	switch (wma_handle->phy_capability) {
	case WMI_11G_CAPABILITY:
	case WMI_11NG_CAPABILITY:
		tgt_cfg.band_cap = eCSR_BAND_24;
		break;
	case WMI_11A_CAPABILITY:
	case WMI_11NA_CAPABILITY:
	case WMI_11AC_CAPABILITY:
		tgt_cfg.band_cap = eCSR_BAND_5G;
		break;
	case WMI_11AG_CAPABILITY:
	case WMI_11NAG_CAPABILITY:
	default:
		tgt_cfg.band_cap = eCSR_BAND_ALL;
	}

	tgt_cfg.max_intf_count = wma_handle->wlan_resource_config.num_vdevs;

	qdf_mem_copy(tgt_cfg.hw_macaddr.bytes, wma_handle->hwaddr,
		     ATH_MAC_LEN);

	wma_update_target_services(wma_handle, &tgt_cfg.services);
	wma_update_target_ht_cap(wma_handle, &tgt_cfg.ht_cap);
	wma_update_target_vht_cap(wma_handle, &tgt_cfg.vht_cap);

	tgt_cfg.target_fw_version = wma_handle->target_fw_version;
#ifdef WLAN_FEATURE_LPSS
	tgt_cfg.lpss_support = wma_handle->lpss_support;
#endif /* WLAN_FEATURE_LPSS */
	tgt_cfg.ap_arpns_support = wma_handle->ap_arpns_support;
	tgt_cfg.fine_time_measurement_cap =
		wma_handle->fine_time_measurement_cap;
	wma_setup_egap_support(&tgt_cfg, wma_handle);

	wma_handle->tgt_cfg_update_cb(hdd_ctx, &tgt_cfg);
}

/**
 * wma_setup_wmi_init_msg() - fill wmi init message buffer
 * @wma_handle: wma handle
 * @ev: ready event fixed params
 * @param_buf: redy event TLVs
 * @len: buffer length
 *
 * Return: wmi buffer or NULL for error
 */
static int wma_setup_wmi_init_msg(tp_wma_handle wma_handle,
				wmi_service_ready_event_fixed_param *ev,
				WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf)
{
	wlan_host_mem_req *ev_mem_reqs;
	wmi_abi_version my_vers;
	wmi_abi_version host_abi_vers;
	int num_whitelist;
	uint16_t idx;
	uint32_t num_units;

	ev_mem_reqs = param_buf->mem_reqs;

	/* allocate memory requested by FW */
	if (ev->num_mem_reqs > WMI_MAX_MEM_REQS) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_NOMEM;
	}

	for (idx = 0; idx < ev->num_mem_reqs; ++idx) {
		num_units = ev_mem_reqs[idx].num_units;
		if (ev_mem_reqs[idx].num_unit_info & NUM_UNITS_IS_NUM_PEERS) {
			/*
			 * number of units to allocate is number
			 * of peers, 1 extra for self peer on
			 * target. this needs to be fied, host
			 * and target can get out of sync
			 */
			num_units = wma_handle->wlan_resource_config.num_peers + 1;
		}
		WMA_LOGD
			("idx %d req %d  num_units %d num_unit_info %d unit size %d actual units %d ",
			idx, ev_mem_reqs[idx].req_id,
			ev_mem_reqs[idx].num_units,
			ev_mem_reqs[idx].num_unit_info,
			ev_mem_reqs[idx].unit_size, num_units);
		wma_alloc_host_mem(wma_handle, ev_mem_reqs[idx].req_id,
				   num_units, ev_mem_reqs[idx].unit_size);
	}

	qdf_mem_copy(&wma_handle->target_abi_vers,
		     &param_buf->fixed_param->fw_abi_vers,
		     sizeof(wmi_abi_version));
	num_whitelist = sizeof(version_whitelist) /
			sizeof(wmi_whitelist_version_info);
	my_vers.abi_version_0 = WMI_ABI_VERSION_0;
	my_vers.abi_version_1 = WMI_ABI_VERSION_1;
	my_vers.abi_version_ns_0 = WMI_ABI_VERSION_NS_0;
	my_vers.abi_version_ns_1 = WMI_ABI_VERSION_NS_1;
	my_vers.abi_version_ns_2 = WMI_ABI_VERSION_NS_2;
	my_vers.abi_version_ns_3 = WMI_ABI_VERSION_NS_3;

	wmi_cmp_and_set_abi_version(num_whitelist, version_whitelist,
				    &my_vers,
				    &param_buf->fixed_param->fw_abi_vers,
				    &host_abi_vers);

	qdf_mem_copy(&wma_handle->final_abi_vers, &host_abi_vers,
		     sizeof(wmi_abi_version));

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_dump_dbs_hw_mode() - Print the DBS HW modes
 * @wma_handle: WMA handle
 *
 * Prints the DBS HW modes sent by the FW as part
 * of WMI ready event
 *
 * Return: None
 */
void wma_dump_dbs_hw_mode(tp_wma_handle wma_handle)
{
	uint32_t i, param;

	if (!wma_handle) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return;
	}

	for (i = 0; i < wma_handle->num_dbs_hw_modes; i++) {
		param = wma_handle->hw_mode.hw_mode_list[i];
		WMA_LOGA("%s:[%d]-MAC0: tx_ss:%d rx_ss:%d bw_idx:%d",
			__func__, i,
			WMI_DBS_HW_MODE_MAC0_TX_STREAMS_GET(param),
			WMI_DBS_HW_MODE_MAC0_RX_STREAMS_GET(param),
			WMI_DBS_HW_MODE_MAC0_BANDWIDTH_GET(param));
		WMA_LOGA("%s:[%d]-MAC1: tx_ss:%d rx_ss:%d bw_idx:%d",
			__func__, i,
			WMI_DBS_HW_MODE_MAC1_TX_STREAMS_GET(param),
			WMI_DBS_HW_MODE_MAC1_RX_STREAMS_GET(param),
			WMI_DBS_HW_MODE_MAC1_BANDWIDTH_GET(param));
		WMA_LOGA("%s:[%d] DBS:%d Agile DFS:%d", __func__, i,
			WMI_DBS_HW_MODE_DBS_MODE_GET(param),
			WMI_DBS_HW_MODE_AGILE_DFS_GET(param));
	}
}

/**
 * wma_init_scan_fw_mode_config() - Initialize scan/fw mode config
 * @wma_handle: WMA handle
 * @scan_config: Scam mode configuration
 * @fw_config: FW mode configuration
 *
 * Enables all the valid bits of concurrent_scan_config_bits and
 * fw_mode_config_bits.
 *
 * Return: None
 */
void wma_init_scan_fw_mode_config(tp_wma_handle wma_handle,
				uint32_t scan_config,
				uint32_t fw_config)
{
	tpAniSirGlobal mac = cds_get_context(QDF_MODULE_ID_PE);

	WMA_LOGD("%s: Enter", __func__);

	if (!mac) {
		WMA_LOGE("%s: Invalid mac handle", __func__);
		return;
	}

	if (!wma_handle) {
		WMA_LOGE("%s: Invalid WMA handle", __func__);
		return;
	}

	wma_handle->dual_mac_cfg.cur_scan_config = 0;
	wma_handle->dual_mac_cfg.cur_fw_mode_config = 0;

	/* If dual mac features are disabled in the INI, we
	 * need not proceed further
	 */
	if (mac->dual_mac_feature_disable) {
		WMA_LOGE("%s: Disabling dual mac capabilities", __func__);
		/* All capabilites are initialized to 0. We can return */
		goto done;
	}

	/* Initialize concurrent_scan_config_bits with default FW value */
	WMI_DBS_CONC_SCAN_CFG_DBS_SCAN_SET(
			wma_handle->dual_mac_cfg.cur_scan_config,
			WMI_DBS_CONC_SCAN_CFG_DBS_SCAN_GET(scan_config));
	WMI_DBS_CONC_SCAN_CFG_AGILE_SCAN_SET(
			wma_handle->dual_mac_cfg.cur_scan_config,
			WMI_DBS_CONC_SCAN_CFG_AGILE_SCAN_GET(scan_config));
	WMI_DBS_CONC_SCAN_CFG_AGILE_DFS_SCAN_SET(
			wma_handle->dual_mac_cfg.cur_scan_config,
			WMI_DBS_CONC_SCAN_CFG_AGILE_DFS_SCAN_GET(scan_config));

	/* Initialize fw_mode_config_bits with default FW value */
	WMI_DBS_FW_MODE_CFG_DBS_SET(
			wma_handle->dual_mac_cfg.cur_fw_mode_config,
			WMI_DBS_FW_MODE_CFG_DBS_GET(fw_config));
	WMI_DBS_FW_MODE_CFG_AGILE_DFS_SET(
			wma_handle->dual_mac_cfg.cur_fw_mode_config,
			WMI_DBS_FW_MODE_CFG_AGILE_DFS_GET(fw_config));
done:
	/* Initialize the previous scan/fw mode config */
	wma_handle->dual_mac_cfg.prev_scan_config =
		wma_handle->dual_mac_cfg.cur_scan_config;
	wma_handle->dual_mac_cfg.prev_fw_mode_config =
		wma_handle->dual_mac_cfg.cur_fw_mode_config;

	WMA_LOGD("%s: cur_scan_config:%x cur_fw_mode_config:%x",
			__func__,
			wma_handle->dual_mac_cfg.cur_scan_config,
			wma_handle->dual_mac_cfg.cur_fw_mode_config);
}

/**
 * wma_rx_service_ready_event() - event handler to process
 *                                wmi rx sevice ready event.
 * @handle: wma handle
 * @cmd_param_info: command params info
 *
 * Return: none
 */
int wma_rx_service_ready_event(void *handle, uint8_t *cmd_param_info,
					uint32_t length)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	struct wma_target_cap target_cap;
	WMI_SERVICE_READY_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_event_fixed_param *ev;
	int status;
	uint32_t *ev_wlan_dbs_hw_mode_list;
	QDF_STATUS ret;

	WMA_LOGD("%s: Enter", __func__);

	param_buf = (WMI_SERVICE_READY_EVENTID_param_tlvs *) cmd_param_info;
	if (!(handle && param_buf)) {
		WMA_LOGP("%s: Invalid arguments", __func__);
		return -EINVAL;
	}

	ev = param_buf->fixed_param;
	if (!ev) {
		WMA_LOGP("%s: Invalid buffer", __func__);
		return -EINVAL;
	}

	WMA_LOGA("WMA <-- WMI_SERVICE_READY_EVENTID");

	wma_handle->num_dbs_hw_modes = ev->num_dbs_hw_modes;
	ev_wlan_dbs_hw_mode_list = param_buf->wlan_dbs_hw_mode_list;
	wma_handle->hw_mode.hw_mode_list =
		qdf_mem_malloc(sizeof(*wma_handle->hw_mode.hw_mode_list) *
				wma_handle->num_dbs_hw_modes);
	if (!wma_handle->hw_mode.hw_mode_list) {
		WMA_LOGE("%s: Memory allocation failed for DBS", __func__);
		/* Continuing with the rest of the processing */
	}
	qdf_mem_copy(wma_handle->hw_mode.hw_mode_list,
			ev_wlan_dbs_hw_mode_list,
			(sizeof(*wma_handle->hw_mode.hw_mode_list) *
						wma_handle->num_dbs_hw_modes));

	wma_dump_dbs_hw_mode(wma_handle);

	/* Initializes the fw_mode and scan_config to zero.
	 * If ext service ready event is present it will set
	 * the actual values of these two params.
	 * This is to ensure that no garbage values would be
	 * present in the absence of ext service ready event.
	 */
	wma_init_scan_fw_mode_config(wma_handle, 0, 0);

	wma_handle->phy_capability = ev->phy_capability;
	wma_handle->max_frag_entry = ev->max_frag_entry;
	wma_handle->num_rf_chains = ev->num_rf_chains;
	qdf_mem_copy(&wma_handle->reg_cap, param_buf->hal_reg_capabilities,
		     sizeof(HAL_REG_CAPABILITIES));
	wma_handle->ht_cap_info = ev->ht_cap_info;
	wma_handle->vht_cap_info = ev->vht_cap_info;
	wma_handle->vht_supp_mcs = ev->vht_supp_mcs;
	wma_handle->num_rf_chains = ev->num_rf_chains;

	wma_handle->target_fw_version = ev->fw_build_vers;
	wma_handle->new_hw_mode_index = ev->default_dbs_hw_mode_index;
	wma_handle->fine_time_measurement_cap = ev->wmi_fw_sub_feat_caps;

	WMA_LOGD("%s: Firmware default hw mode index : %d",
		 __func__, ev->default_dbs_hw_mode_index);
	WMA_LOGE("%s: Firmware build version : %08x",
		 __func__, ev->fw_build_vers);
	WMA_LOGD(FL("FW fine time meas cap: 0x%x"), ev->wmi_fw_sub_feat_caps);

	if (ev->hw_bd_id) {
		wma_handle->hw_bd_id = ev->hw_bd_id;
		qdf_mem_copy(wma_handle->hw_bd_info,
			     ev->hw_bd_info, sizeof(ev->hw_bd_info));

		WMA_LOGE("%s: Board version: %x.%x",
			 __func__,
			 wma_handle->hw_bd_info[0], wma_handle->hw_bd_info[1]);
	} else {
		wma_handle->hw_bd_id = 0;
		qdf_mem_zero(wma_handle->hw_bd_info,
			     sizeof(wma_handle->hw_bd_info));
		WMA_LOGE("%s: Board version is unknown!", __func__);
	}
	wma_handle->dfs_ic->dfs_hw_bd_id = wma_handle->hw_bd_id;

	/* TODO: Recheck below line to dump service ready event */
	/* dbg_print_wmi_service_11ac(ev); */

	/* wmi service is ready */
	qdf_mem_copy(wma_handle->wmi_service_bitmap,
		     param_buf->wmi_service_bitmap,
		     sizeof(wma_handle->wmi_service_bitmap));

	ol_tx_set_is_mgmt_over_wmi_enabled(
		WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				       WMI_SERVICE_MGMT_TX_WMI));
	ol_tx_set_desc_global_pool_size(ev->num_msdu_desc);

	/* SWBA event handler for beacon transmission */
	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						    WMI_HOST_SWBA_EVENTID,
						    wma_beacon_swba_handler,
						    WMA_RX_SERIALIZER_CTX);
	if (status) {
		WMA_LOGE("Failed to register swba beacon event cb");
		return -EINVAL;
	}
#ifdef WLAN_FEATURE_LPSS
	wma_handle->lpss_support =
		WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				       WMI_SERVICE_LPASS);
#endif /* WLAN_FEATURE_LPSS */

	/*
	 * This Service bit is added to check for ARP/NS Offload
	 * support for LL/HL targets
	 */
	wma_handle->ap_arpns_support =
		WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				WMI_SERVICE_AP_ARPNS_OFFLOAD);

	if (WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				   WMI_SERVICE_CSA_OFFLOAD)) {
		WMA_LOGD("%s: FW support CSA offload capability", __func__);
		status =
			wmi_unified_register_event_handler(wma_handle->wmi_handle,
						WMI_CSA_HANDLING_EVENTID,
						wma_csa_offload_handler,
						WMA_RX_SERIALIZER_CTX);
		if (status) {
			WMA_LOGE("Failed to register CSA offload event cb");
			return -EINVAL;
		}
	}

	if (WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				   WMI_SERVICE_MGMT_TX_WMI)) {
		WMA_LOGE("Firmware supports management TX over WMI,use WMI interface instead of HTT for management Tx");
		status = wmi_desc_pool_init(wma_handle, WMI_DESC_POOL_MAX);
		if (status) {
			WMA_LOGE("Failed to initialize wmi descriptor pool");
			return -EINVAL;
		}
		/*
		 * Register Tx completion event handler for MGMT Tx over WMI
		 * case
		 */
		status = wmi_unified_register_event_handler(
					wma_handle->wmi_handle,
					WMI_MGMT_TX_COMPLETION_EVENTID,
					wma_mgmt_tx_completion_handler,
					WMA_RX_SERIALIZER_CTX);
		if (status) {
			WMA_LOGE("Failed to register MGMT over WMI completion handler");
			return -EINVAL;
		}

	} else {
		WMA_LOGE("FW doesnot support WMI_SERVICE_MGMT_TX_WMI, Use HTT interface for Management Tx");
	}
#ifdef WLAN_FEATURE_GTK_OFFLOAD
	if (WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				   WMI_SERVICE_GTK_OFFLOAD)) {
		status =
			wmi_unified_register_event_handler(wma_handle->wmi_handle,
						WMI_GTK_OFFLOAD_STATUS_EVENTID,
						wma_gtk_offload_status_event,
						WMA_RX_SERIALIZER_CTX);
		if (status) {
			WMA_LOGE("Failed to register GTK offload event cb");
			return -EINVAL;
		}
	}
#endif /* WLAN_FEATURE_GTK_OFFLOAD */

	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
						    WMI_P2P_NOA_EVENTID,
						    wma_p2p_noa_event_handler,
						    WMA_RX_SERIALIZER_CTX);
	if (status) {
		WMA_LOGE("Failed to register WMI_P2P_NOA_EVENTID callback");
		return -EINVAL;
	}
	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
				WMI_TBTTOFFSET_UPDATE_EVENTID,
				wma_tbttoffset_update_event_handler,
				WMA_RX_SERIALIZER_CTX);
	if (status) {
		WMA_LOGE
			("Failed to register WMI_TBTTOFFSET_UPDATE_EVENTID callback");
		return -EINVAL;
	}

	/* mac_id is replaced with pdev_id in converged firmware to have
	 * multi-radio support. In order to maintain backward compatibility
	 * with old fw, host needs to check WMI_SERVICE_DEPRECATED_REPLACE
	 * in service bitmap from FW and host needs to set use_pdev_id in
	 * wmi_resource_config to true. If WMI_SERVICE_DEPRECATED_REPLACE
	 * service is not set, then host shall not expect MAC ID from FW in
	 * VDEV START RESPONSE event and host shall use PDEV ID.
	 */
	 if (WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
			WMI_SERVICE_DEPRECATED_REPLACE))
		wma_handle->wlan_resource_config.use_pdev_id = true;
	else
		wma_handle->wlan_resource_config.use_pdev_id = false;

	/* register the Enhanced Green AP event handler */
	wma_register_egap_event_handle(wma_handle);

	/* Initialize the log supported event handler */
	status = wmi_unified_register_event_handler(wma_handle->wmi_handle,
			WMI_DIAG_EVENT_LOG_SUPPORTED_EVENTID,
			wma_log_supported_evt_handler,
			WMA_RX_SERIALIZER_CTX);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to register log supported event cb");
		return -EINVAL;
	}

	qdf_mem_copy(target_cap.wmi_service_bitmap,
		     param_buf->wmi_service_bitmap,
		     sizeof(wma_handle->wmi_service_bitmap));
	target_cap.wlan_resource_config = wma_handle->wlan_resource_config;
	wma_update_fw_config(wma_handle, &target_cap);
	qdf_mem_copy(wma_handle->wmi_service_bitmap,
		     target_cap.wmi_service_bitmap,
		     sizeof(wma_handle->wmi_service_bitmap));
	wma_handle->wlan_resource_config = target_cap.wlan_resource_config;

	status = wmi_unified_save_fw_version_cmd(wma_handle->wmi_handle,
				param_buf);
	if (status != EOK) {
		WMA_LOGE("Failed to send WMI_INIT_CMDID command");
		return -EINVAL;
	}

	status = wma_setup_wmi_init_msg(wma_handle, ev, param_buf);
	if (status != EOK) {
		WMA_LOGE("Failed to setup for wma init command");
		return -EINVAL;
	}

	/* A host, which supports WMI_SERVICE_READY_EXT_EVENTID, would need to
	 * check the WMI_SERVICE_READY message for an "extension" flag, and if
	 * this flag is set, then hold off on sending the WMI_INIT message until
	 * WMI_SERVICE_READY_EXT_EVENTID is received.
	 */
	if (!WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
				WMI_SERVICE_EXT_MSG)) {
		/* No service extended message support.
		 * Send INIT command immediately
		 */
		WMA_LOGA("WMA --> WMI_INIT_CMDID");
		status = wmi_unified_send_init_cmd(wma_handle->wmi_handle,
				&wma_handle->wlan_resource_config,
				wma_handle->num_mem_chunks,
				wma_handle->mem_chunks, 1);
		if (status != EOK) {
			WMA_LOGE("Failed to send WMI_INIT_CMDID command");
			return -EINVAL;
		}
	} else {
		status = wmi_unified_send_init_cmd(wma_handle->wmi_handle,
				&wma_handle->wlan_resource_config,
				wma_handle->num_mem_chunks,
				wma_handle->mem_chunks, 0);
		if (status != EOK) {
			WMA_LOGE("Failed to save WMI_INIT_CMDID command parameter");
			return -EINVAL;
		}
		/* The saved 'buf' will be freed after sending INIT command or
		 * in other cases as required
		 */
		ret = qdf_mc_timer_start(&wma_handle->service_ready_ext_timer,
				WMA_SERVICE_READY_EXT_TIMEOUT);
		if (!QDF_IS_STATUS_SUCCESS(ret))
			WMA_LOGP("Failed to start the service ready ext timer");

		WMA_LOGA("%s: WMA waiting for WMI_SERVICE_READY_EXT_EVENTID",
				__func__);
	}

	return 0;
}

/**
 * wma_rx_service_ready_ext_event() - evt handler for sevice ready ext event.
 * @handle: wma handle
 * @event: params of the service ready extended event
 * @length: param length
 *
 * Return: none
 */
int wma_rx_service_ready_ext_event(void *handle, uint8_t *event,
					uint32_t length)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *param_buf;
	wmi_service_ready_ext_event_fixed_param *ev;
	int status;
	QDF_STATUS ret;

	WMA_LOGD("%s: Enter", __func__);

	if (!wma_handle) {
		WMA_LOGP("%s: Invalid WMA handle", __func__);
		return -EINVAL;
	}

	param_buf = (WMI_SERVICE_READY_EXT_EVENTID_param_tlvs *) event;
	if (!param_buf) {
		WMA_LOGP("%s: Invalid event", __func__);
		return -EINVAL;
	}

	ev = param_buf->fixed_param;
	if (!ev) {
		WMA_LOGP("%s: Invalid buffer", __func__);
		return -EINVAL;
	}

	WMA_LOGA("WMA <-- WMI_SERVICE_READY_EXT_EVENTID");

	WMA_LOGA("%s: Defaults: scan config:%x FW mode config:%x",
			__func__, ev->default_conc_scan_config_bits,
			ev->default_fw_config_bits);

	ret = qdf_mc_timer_stop(&wma_handle->service_ready_ext_timer);
	if (!QDF_IS_STATUS_SUCCESS(ret)) {
		WMA_LOGP("Failed to stop the service ready ext timer");
		return -EINVAL;
	}

	WMA_LOGA("WMA --> WMI_INIT_CMDID");
	status = wmi_unified_send_saved_init_cmd(wma_handle->wmi_handle);
	if (status != EOK)
		/* In success case, WMI layer will free after getting copy
		 * engine TX complete interrupt
		 */
		WMA_LOGE("Failed to send WMI_INIT_CMDID command");

	wma_init_scan_fw_mode_config(wma_handle,
				ev->default_conc_scan_config_bits,
				ev->default_fw_config_bits);

	return 0;
}

/**
 * wma_rx_ready_event() - event handler to process
 *                        wmi rx ready event.
 * @handle: wma handle
 * @cmd_param_info: command params info
 * @length: param length
 *
 * Return: none
 */
int wma_rx_ready_event(void *handle, uint8_t *cmd_param_info,
					uint32_t length)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	WMI_READY_EVENTID_param_tlvs *param_buf = NULL;
	wmi_ready_event_fixed_param *ev = NULL;

	WMA_LOGD("%s: Enter", __func__);

	param_buf = (WMI_READY_EVENTID_param_tlvs *) cmd_param_info;
	if (!(wma_handle && param_buf)) {
		WMA_LOGP("%s: Invalid arguments", __func__);
		QDF_ASSERT(0);
		return -EINVAL;
	}

	WMA_LOGA("WMA <-- WMI_READY_EVENTID");

	ev = param_buf->fixed_param;
	/* Indicate to the waiting thread that the ready
	 * event was received */
	wma_handle->wmi_ready = true;
	wma_handle->wlan_init_status = ev->status;

	/*
	 * We need to check the WMI versions and make sure both
	 * host and fw are compatible.
	 */
	if (!wmi_versions_are_compatible(&wma_handle->final_abi_vers,
					 &ev->fw_abi_vers)) {
		/*
		 * Error: Our host version and the given firmware version
		 * are incompatible.
		 */
		WMA_LOGE("%s: Error: Incompatible WMI version."
			 "Host: %d,%d,0x%x 0x%x 0x%x 0x%x, FW: %d,%d,0x%x 0x%x 0x%x 0x%x",
			 __func__,
			 WMI_VER_GET_MAJOR(wma_handle->final_abi_vers.
					   abi_version_0),
			 WMI_VER_GET_MINOR(wma_handle->final_abi_vers.
					   abi_version_0),
			 wma_handle->final_abi_vers.abi_version_ns_0,
			 wma_handle->final_abi_vers.abi_version_ns_1,
			 wma_handle->final_abi_vers.abi_version_ns_2,
			 wma_handle->final_abi_vers.abi_version_ns_3,
			 WMI_VER_GET_MAJOR(ev->fw_abi_vers.abi_version_0),
			 WMI_VER_GET_MINOR(ev->fw_abi_vers.abi_version_0),
			 ev->fw_abi_vers.abi_version_ns_0,
			 ev->fw_abi_vers.abi_version_ns_1,
			 ev->fw_abi_vers.abi_version_ns_2,
			 ev->fw_abi_vers.abi_version_ns_3);
		if (wma_handle->wlan_init_status == WLAN_INIT_STATUS_SUCCESS) {
			/* Failed this connection to FW */
			wma_handle->wlan_init_status =
				WLAN_INIT_STATUS_GEN_FAILED;
		}
	}
	qdf_mem_copy(&wma_handle->final_abi_vers, &ev->fw_abi_vers,
		     sizeof(wmi_abi_version));
	qdf_mem_copy(&wma_handle->target_abi_vers, &ev->fw_abi_vers,
		     sizeof(wmi_abi_version));

	/* copy the mac addr */
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->mac_addr, wma_handle->myaddr);
	WMI_MAC_ADDR_TO_CHAR_ARRAY(&ev->mac_addr, wma_handle->hwaddr);

	wma_update_hdd_cfg(wma_handle);

	qdf_event_set(&wma_handle->wma_ready_event);

	WMA_LOGD("Exit");

	return 0;
}

/**
 * wma_setneedshutdown() - setting wma needshutdown flag
 * @cds_ctx: cds context
 *
 * Return: none
 */
void wma_setneedshutdown(void *cds_ctx)
{
	tp_wma_handle wma_handle;

	WMA_LOGD("%s: Enter", __func__);

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wma_handle) {
		WMA_LOGP("%s: Invalid arguments", __func__);
		QDF_ASSERT(0);
		return;
	}

	wma_handle->needShutdown = true;
	WMA_LOGD("%s: Exit", __func__);
}

/**
 * wma_needshutdown() - Is wma needs shutdown?
 * @cds_ctx: cds context
 *
 * Return: returns true/false
 */
bool wma_needshutdown(void *cds_ctx)
{
	tp_wma_handle wma_handle;

	WMA_LOGD("%s: Enter", __func__);

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wma_handle) {
		WMA_LOGP("%s: Invalid arguments", __func__);
		QDF_ASSERT(0);
		return false;
	}

	WMA_LOGD("%s: Exit", __func__);
	return wma_handle->needShutdown;
}

/**
 * wma_wait_for_ready_event() - wait for wma ready event
 * @handle: wma handle
 *
 * Return: 0 for success or QDF error
 */
QDF_STATUS wma_wait_for_ready_event(WMA_HANDLE handle)
{
	tp_wma_handle wma_handle = (tp_wma_handle) handle;
	QDF_STATUS qdf_status;

	/* wait until WMI_READY_EVENTID received from FW */
	qdf_status = qdf_wait_single_event(&(wma_handle->wma_ready_event),
					   WMA_READY_EVENTID_TIMEOUT);

	if (QDF_STATUS_SUCCESS != qdf_status) {
		WMA_LOGP("%s: Timeout waiting for ready event from FW",
			 __func__);
		qdf_status = QDF_STATUS_E_FAILURE;
	}
	return qdf_status;
}

/**
 * wma_set_ppsconfig() - set pps config in fw
 * @vdev_id: vdev id
 * @pps_param: pps params
 * @val : param value
 *
 * Return: 0 for success or QDF error
 */
QDF_STATUS wma_set_ppsconfig(uint8_t vdev_id, uint16_t pps_param,
				    int val)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	int ret = -EIO;
	uint32_t pps_val;

	if (NULL == wma) {
		WMA_LOGE("%s: Failed to get wma", __func__);
		return QDF_STATUS_E_INVAL;
	}

	switch (pps_param) {
	case WMA_VHT_PPS_PAID_MATCH:
		pps_val = ((val << 31) & 0xffff0000) |
			  (PKT_PWR_SAVE_PAID_MATCH & 0xffff);
		goto pkt_pwr_save_config;
	case WMA_VHT_PPS_GID_MATCH:
		pps_val = ((val << 31) & 0xffff0000) |
			  (PKT_PWR_SAVE_GID_MATCH & 0xffff);
		goto pkt_pwr_save_config;
	case WMA_VHT_PPS_DELIM_CRC_FAIL:
		pps_val = ((val << 31) & 0xffff0000) |
			  (PKT_PWR_SAVE_DELIM_CRC_FAIL & 0xffff);
		goto pkt_pwr_save_config;

		/* Enable the code below as and when the functionality
		 * is supported/added in host.
		 */
#ifdef NOT_YET
	case WMA_VHT_PPS_EARLY_TIM_CLEAR:
		pps_val = ((val << 31) & 0xffff0000) |
			  (PKT_PWR_SAVE_EARLY_TIM_CLEAR & 0xffff);
		goto pkt_pwr_save_config;
	case WMA_VHT_PPS_EARLY_DTIM_CLEAR:
		pps_val = ((val << 31) & 0xffff0000) |
			  (PKT_PWR_SAVE_EARLY_DTIM_CLEAR & 0xffff);
		goto pkt_pwr_save_config;
	case WMA_VHT_PPS_EOF_PAD_DELIM:
		pps_val = ((val << 31) & 0xffff0000) |
			  (PKT_PWR_SAVE_EOF_PAD_DELIM & 0xffff);
		goto pkt_pwr_save_config;
	case WMA_VHT_PPS_MACADDR_MISMATCH:
		pps_val = ((val << 31) & 0xffff0000) |
			  (PKT_PWR_SAVE_MACADDR_MISMATCH & 0xffff);
		goto pkt_pwr_save_config;
	case WMA_VHT_PPS_GID_NSTS_ZERO:
		pps_val = ((val << 31) & 0xffff0000) |
			  (PKT_PWR_SAVE_GID_NSTS_ZERO & 0xffff);
		goto pkt_pwr_save_config;
	case WMA_VHT_PPS_RSSI_CHECK:
		pps_val = ((val << 31) & 0xffff0000) |
			  (PKT_PWR_SAVE_RSSI_CHECK & 0xffff);
		goto pkt_pwr_save_config;
#endif /* NOT_YET */
pkt_pwr_save_config:
		WMA_LOGD("vdev_id:%d val:0x%x pps_val:0x%x", vdev_id,
			 val, pps_val);
		ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
						      WMI_VDEV_PARAM_PACKET_POWERSAVE,
						      pps_val);
		break;
	default:
		WMA_LOGE("%s:INVALID PPS CONFIG", __func__);
	}

	return (ret) ? QDF_STATUS_E_FAILURE : QDF_STATUS_SUCCESS;
}

/**
 * wma_process_set_mas() - Function to enable/disable MAS
 * @wma:	Pointer to WMA handle
 * @mas_val:	1-Enable MAS, 0-Disable MAS
 *
 * This function enables/disables the MAS value
 *
 * Return: QDF_SUCCESS for success otherwise failure
 */
QDF_STATUS wma_process_set_mas(tp_wma_handle wma,
		uint32_t *mas_val)
{
	uint32_t val;

	if (NULL == wma || NULL == mas_val) {
		WMA_LOGE("%s: Invalid input to enable/disable MAS", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	val = (*mas_val);

	if (QDF_STATUS_SUCCESS !=
			wma_set_enable_disable_mcc_adaptive_scheduler(val)) {
		WMA_LOGE("%s: Unable to enable/disable MAS", __func__);
		return QDF_STATUS_E_FAILURE;
	} else {
		WMA_LOGE("%s: Value is %d", __func__, val);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_process_set_miracast() - Function to set miracast value in WMA
 * @wma:		Pointer to WMA handle
 * @miracast_val:	0-Disabled,1-Source,2-Sink
 *
 * This function stores the miracast value in WMA
 *
 * Return: QDF_SUCCESS for success otherwise failure
 *
 */
QDF_STATUS wma_process_set_miracast(tp_wma_handle wma, uint32_t *miracast_val)
{
	if (NULL == wma || NULL == miracast_val) {
		WMA_LOGE("%s: Invalid input to store miracast value", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	wma->miracast_value = *miracast_val;
	WMA_LOGE("%s: Miracast value is %d", __func__, wma->miracast_value);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_config_stats_factor() - Function to configure stats avg. factor
 * @wma:  pointer to WMA handle
 * @avg_factor:	stats. avg. factor passed down by userspace
 *
 * This function configures the avg. stats value in firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
static QDF_STATUS wma_config_stats_factor(tp_wma_handle wma,
				      struct sir_stats_avg_factor *avg_factor)
{
	QDF_STATUS ret;

	if (NULL == wma || NULL == avg_factor) {
		WMA_LOGE("%s: Invalid input of stats avg factor", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_vdev_set_param(wma->wmi_handle,
					    avg_factor->vdev_id,
					    WMI_VDEV_PARAM_STATS_AVG_FACTOR,
					    avg_factor->stats_avg_factor);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE(" failed to set avg_factor for vdev_id %d",
			 avg_factor->vdev_id);
	}

	WMA_LOGD("%s: Set stats_avg_factor %d for vdev_id %d", __func__,
		 avg_factor->stats_avg_factor, avg_factor->vdev_id);

	return ret;
}

/**
 * wma_config_guard_time() - Function to set guard time in firmware
 * @wma:  pointer to WMA handle
 * @guard_time:  guard time passed down by userspace
 *
 * This function configures the guard time in firmware
 *
 * Return: QDF_STATUS_SUCCESS for success otherwise failure
 *
 */
static QDF_STATUS wma_config_guard_time(tp_wma_handle wma,
				   struct sir_guard_time_request *guard_time)
{
	QDF_STATUS ret;

	if (NULL == wma || NULL == guard_time) {
		WMA_LOGE("%s: Invalid input of guard time", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	ret = wma_vdev_set_param(wma->wmi_handle,
					      guard_time->vdev_id,
					      WMI_VDEV_PARAM_RX_LEAK_WINDOW,
					      guard_time->guard_time);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE(" failed to set guard time for vdev_id %d",
			 guard_time->vdev_id);
	}

	WMA_LOGD("Set guard time %d for vdev_id %d",
		 guard_time->guard_time, guard_time->vdev_id);

	return ret;
}

/**
 * wma_enable_specific_fw_logs() - Start/Stop logging of diag event/log id
 * @wma_handle: WMA handle
 * @start_log: Start logging related parameters
 *
 * Send the command to the FW based on which specific logging of diag
 * event/log id can be started/stopped
 *
 * Return: None
 */
void wma_enable_specific_fw_logs(tp_wma_handle wma_handle,
		struct sir_wifi_start_log *start_log)
{

	if (!start_log) {
		WMA_LOGE("%s: start_log pointer is NULL", __func__);
		return;
	}
	if (!wma_handle) {
		WMA_LOGE("%s: Invalid wma handle", __func__);
		return;
	}

	if (!((start_log->ring_id == RING_ID_CONNECTIVITY) ||
			(start_log->ring_id == RING_ID_FIRMWARE_DEBUG))) {
		WMA_LOGD("%s: Not connectivity or fw debug ring: %d",
				__func__, start_log->ring_id);
		return;
	}

	wmi_unified_enable_specific_fw_logs_cmd(wma_handle->wmi_handle,
				(struct wmi_wifi_start_log *)start_log);

	return;
}

#if !defined(REMOVE_PKT_LOG)
/**
 * wma_set_wifi_start_packet_stats() - Start/stop packet stats
 * @wma_handle: WMA handle
 * @start_log: Struture containing the start wifi logger params
 *
 * This function is used to send the WMA commands to start/stop logging
 * of per packet statistics
 *
 * Return: None
 *
 */
void wma_set_wifi_start_packet_stats(void *wma_handle,
		struct sir_wifi_start_log *start_log)
{
	struct hif_opaque_softc *scn;
	uint32_t log_state;

	if (!start_log) {
		WMA_LOGE("%s: start_log pointer is NULL", __func__);
		return;
	}
	if (!wma_handle) {
		WMA_LOGE("%s: Invalid wma handle", __func__);
		return;
	}

	/* No need to register for ring IDs other than packet stats */
	if (start_log->ring_id != RING_ID_PER_PACKET_STATS) {
		WMA_LOGI("%s: Ring id is not for per packet stats: %d",
			__func__, start_log->ring_id);
		return;
	}

	scn = cds_get_context(QDF_MODULE_ID_HIF);

	log_state = ATH_PKTLOG_ANI | ATH_PKTLOG_RCUPDATE | ATH_PKTLOG_RCFIND |
		ATH_PKTLOG_RX | ATH_PKTLOG_TX | ATH_PKTLOG_TEXT;

	if (start_log->verbose_level == WLAN_LOG_LEVEL_ACTIVE) {
		pktlog_enable(scn, log_state);
		WMA_LOGI("%s: Enabling per packet stats", __func__);
	} else {
		pktlog_enable(scn, 0);
		WMA_LOGI("%s: Disabling per packet stats", __func__);
	}
}
#endif

/**
 * wma_send_flush_logs_to_fw() - Send log flush command to FW
 * @wma_handle: WMI handle
 *
 * This function is used to send the flush command to the FW,
 * that will flush the fw logs that are residue in the FW
 *
 * Return: None
 */
void wma_send_flush_logs_to_fw(tp_wma_handle wma_handle)
{
	QDF_STATUS status;
	int ret;

	ret = wmi_unified_flush_logs_to_fw_cmd(wma_handle->wmi_handle);
	if (ret != EOK)
		return;

	status = qdf_mc_timer_start(&wma_handle->log_completion_timer,
			WMA_LOG_COMPLETION_TIMER);
	if (status != QDF_STATUS_SUCCESS)
		WMA_LOGE("Failed to start the log completion timer");
}

/**
 * wma_mc_process_msg() - process wma messages and call appropriate function.
 * @cds_context: cds context
 * @msg: message
 *
 * Return: QDF_SUCCESS for success otherwise failure
 */
QDF_STATUS wma_mc_process_msg(void *cds_context, cds_msg_t *msg)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tp_wma_handle wma_handle;
	ol_txrx_vdev_handle txrx_vdev_handle = NULL;
	extern uint8_t *mac_trace_get_wma_msg_string(uint16_t wmaMsg);

	WMA_LOGI("%s: Enter", __func__);
	if (NULL == msg) {
		WMA_LOGE("msg is NULL");
		QDF_ASSERT(0);
		qdf_status = QDF_STATUS_E_INVAL;
		goto end;
	}

	WMA_LOGD("msg->type = %x %s", msg->type,
		 mac_trace_get_wma_msg_string(msg->type));

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wma_handle) {
		WMA_LOGP("%s: wma_handle is NULL", __func__);
		QDF_ASSERT(0);
		qdf_mem_free(msg->bodyptr);
		qdf_status = QDF_STATUS_E_INVAL;
		goto end;
	}

	switch (msg->type) {

	/* Message posted by wmi for all control path related
	 * FW events to serialize through mc_thread.
	 */
	case WMA_PROCESS_FW_EVENT:
		wma_process_fw_event(wma_handle,
				(wma_process_fw_event_params *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;

#ifdef FEATURE_WLAN_ESE
	case WMA_TSM_STATS_REQ:
		WMA_LOGA("McThread: WMA_TSM_STATS_REQ");
		wma_process_tsm_stats_req(wma_handle, (void *)msg->bodyptr);
		break;
#endif /* FEATURE_WLAN_ESE */
	case WNI_CFG_DNLD_REQ:
		WMA_LOGA("McThread: WNI_CFG_DNLD_REQ");
		qdf_status = wma_wni_cfg_dnld(wma_handle);
		if (QDF_IS_STATUS_SUCCESS(qdf_status)) {
			cds_wma_complete_cback(cds_context);
		} else {
			WMA_LOGD("config download failure");
		}
		break;
	case WMA_ADD_STA_SELF_REQ:
		txrx_vdev_handle =
			wma_vdev_attach(wma_handle,
				(struct add_sta_self_params *) msg->
				bodyptr, 1);
		if (!txrx_vdev_handle) {
			WMA_LOGE("Failed to attach vdev");
		} else {
			/* Register with TxRx Module for Data Ack Complete Cb */
			ol_txrx_data_tx_cb_set(txrx_vdev_handle,
					      wma_data_tx_ack_comp_hdlr,
					      wma_handle);
		}
		break;
	case WMA_DEL_STA_SELF_REQ:
		wma_vdev_detach(wma_handle,
				(struct del_sta_self_params *) msg->bodyptr, 1);
		break;
	case WMA_START_SCAN_OFFLOAD_REQ:
		wma_start_scan(wma_handle, msg->bodyptr, msg->type);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_STOP_SCAN_OFFLOAD_REQ:
		wma_stop_scan(wma_handle, msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_UPDATE_CHAN_LIST_REQ:
		wma_update_channel_list(wma_handle,
					(tSirUpdateChanList *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_SET_LINK_STATE:
		wma_set_linkstate(wma_handle, (tpLinkStateParams) msg->bodyptr);
		break;
	case WMA_CHNL_SWITCH_REQ:
		wma_set_channel(wma_handle,
				(tpSwitchChannelParams) msg->bodyptr);
		break;
	case WMA_ADD_BSS_REQ:
		wma_add_bss(wma_handle, (tpAddBssParams) msg->bodyptr);
		break;
	case WMA_ADD_STA_REQ:
		wma_add_sta(wma_handle, (tpAddStaParams) msg->bodyptr);
		break;
	case WMA_SET_BSSKEY_REQ:
		wma_set_bsskey(wma_handle, (tpSetBssKeyParams) msg->bodyptr);
		break;
	case WMA_SET_STAKEY_REQ:
		wma_set_stakey(wma_handle, (tpSetStaKeyParams) msg->bodyptr);
		break;
	case WMA_DELETE_STA_REQ:
		wma_delete_sta(wma_handle, (tpDeleteStaParams) msg->bodyptr);
		break;
	case WMA_DELETE_BSS_REQ:
		wma_delete_bss(wma_handle, (tpDeleteBssParams) msg->bodyptr);
		break;
	case WMA_UPDATE_EDCA_PROFILE_IND:
		wma_process_update_edca_param_req(wma_handle,
						  (tEdcaParams *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_SEND_BEACON_REQ:
		wma_send_beacon(wma_handle, (tpSendbeaconParams) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_SEND_PROBE_RSP_TMPL:
		wma_send_probe_rsp_tmpl(wma_handle,
					(tpSendProbeRespParams) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_CLI_SET_CMD:
		wma_process_cli_set_cmd(wma_handle,
					(wma_cli_set_cmd_t *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#if !defined(REMOVE_PKT_LOG)
	case WMA_PKTLOG_ENABLE_REQ:
		wma_pktlog_wmi_send_cmd(wma_handle,
			(struct ath_pktlog_wmi_params *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#endif /* REMOVE_PKT_LOG */
#if defined(QCA_WIFI_FTM)
	case WMA_FTM_CMD_REQ:
		wma_process_ftm_command(wma_handle,
				(struct ar6k_testmode_cmd_data *)msg->bodyptr);
		break;
#endif /* QCA_WIFI_FTM */
	case WMA_ENTER_PS_REQ:
		wma_enable_sta_ps_mode(wma_handle,
				       (tpEnablePsParams) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_EXIT_PS_REQ:
		wma_disable_sta_ps_mode(wma_handle,
					(tpDisablePsParams) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_ENABLE_UAPSD_REQ:
		wma_enable_uapsd_mode(wma_handle,
				      (tpEnableUapsdParams) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_DISABLE_UAPSD_REQ:
		wma_disable_uapsd_mode(wma_handle,
				       (tpDisableUapsdParams) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_SET_TX_POWER_REQ:
		wma_set_tx_power(wma_handle, (tpMaxTxPowerParams) msg->bodyptr);
		break;
	case WMA_SET_MAX_TX_POWER_REQ:
		wma_set_max_tx_power(wma_handle,
				     (tpMaxTxPowerParams) msg->bodyptr);
		break;
	case WMA_SET_KEEP_ALIVE:
		wma_set_keepalive_req(wma_handle,
				      (tSirKeepAliveReq *) msg->bodyptr);
		break;
#ifdef FEATURE_WLAN_SCAN_PNO
	case WMA_SET_PNO_REQ:
		wma_config_pno(wma_handle, (tpSirPNOScanReq) msg->bodyptr);
		break;

	case WMA_SME_SCAN_CACHE_UPDATED:
		wma_scan_cache_updated_ind(wma_handle, msg->bodyval);
		break;
#endif /* FEATURE_WLAN_SCAN_PNO */
#ifdef FEATURE_WLAN_ESE
	case WMA_SET_PLM_REQ:
		wma_config_plm(wma_handle, (tpSirPlmReq) msg->bodyptr);
		break;
#endif
	case WMA_GET_STATISTICS_REQ:
		wma_get_stats_req(wma_handle,
				  (tAniGetPEStatsReq *) msg->bodyptr);
		break;

	case WMA_CONFIG_PARAM_UPDATE_REQ:
		wma_update_cfg_params(wma_handle, (tSirMsgQ *) msg);
		break;

	case WMA_UPDATE_OP_MODE:
		wma_process_update_opmode(wma_handle,
					  (tUpdateVHTOpMode *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_UPDATE_RX_NSS:
		wma_process_update_rx_nss(wma_handle,
					  (tUpdateRxNss *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_UPDATE_MEMBERSHIP:
		wma_process_update_membership(wma_handle,
			(tUpdateMembership *) msg->bodyptr);
		break;
	case WMA_UPDATE_USERPOS:
		wma_process_update_userpos(wma_handle,
					   (tUpdateUserPos *) msg->bodyptr);
		break;
	case WMA_UPDATE_BEACON_IND:
		wma_process_update_beacon_params(wma_handle,
			(tUpdateBeaconParams *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;

	case WMA_ADD_TS_REQ:
		wma_add_ts_req(wma_handle, (tAddTsParams *) msg->bodyptr);
		break;

	case WMA_DEL_TS_REQ:
		wma_del_ts_req(wma_handle, (tDelTsParams *) msg->bodyptr);
		break;

	case WMA_AGGR_QOS_REQ:
		wma_aggr_qos_req(wma_handle, (tAggrAddTsParams *) msg->bodyptr);
		break;

	case WMA_RECEIVE_FILTER_SET_FILTER_REQ:
		wma_process_receive_filter_set_filter_req(wma_handle,
				(tSirRcvPktFilterCfgType *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;

	case WMA_RECEIVE_FILTER_CLEAR_FILTER_REQ:
		wma_process_receive_filter_clear_filter_req(wma_handle,
				(tSirRcvFltPktClearParam *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;

	case WMA_WOW_ADD_PTRN:
		wma_wow_add_pattern(wma_handle,
				    (struct wow_add_pattern *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_WOW_DEL_PTRN:
		wma_wow_delete_user_pattern(wma_handle,
				    (struct wow_delete_pattern *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_WOWL_ENTER_REQ:
		wma_wow_enter(wma_handle,
			      (tpSirHalWowlEnterParams) msg->bodyptr);
		break;
	case WMA_WOWL_EXIT_REQ:
		wma_wow_exit(wma_handle, (tpSirHalWowlExitParams) msg->bodyptr);
		break;

	case WMA_RUNTIME_PM_SUSPEND_IND:
		wma_calculate_and_update_conn_state(wma_handle);
		wma_suspend_req(wma_handle, QDF_RUNTIME_SUSPEND);
		break;

	case WMA_RUNTIME_PM_RESUME_IND:
		wma_resume_req(wma_handle, QDF_RUNTIME_SUSPEND);
		break;

	case WMA_WLAN_SUSPEND_IND:
		wma_update_conn_state(wma_handle, msg->bodyval);
		wma_suspend_req(wma_handle, QDF_SYSTEM_SUSPEND);
		break;
	case WMA_8023_MULTICAST_LIST_REQ:
		wma_process_mcbc_set_filter_req(wma_handle,
				(tpSirRcvFltMcAddrList) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#ifdef WLAN_FEATURE_GTK_OFFLOAD
	case WMA_GTK_OFFLOAD_REQ:
		wma_process_gtk_offload_req(wma_handle,
				(tpSirGtkOffloadParams) msg->bodyptr);
		break;

	case WMA_GTK_OFFLOAD_GETINFO_REQ:
		wma_process_gtk_offload_getinfo_req(wma_handle,
				(tpSirGtkOffloadGetInfoRspParams)msg->bodyptr);
		break;
#endif /* WLAN_FEATURE_GTK_OFFLOAD */
#ifdef FEATURE_OEM_DATA_SUPPORT
	case WMA_START_OEM_DATA_REQ:
		wma_start_oem_data_req(wma_handle,
				       (tStartOemDataReq *) msg->bodyptr);
		break;
#endif /* FEATURE_OEM_DATA_SUPPORT */
	case WMA_SET_HOST_OFFLOAD:
		wma_enable_arp_ns_offload(wma_handle,
					  (tpSirHostOffloadReq) msg->bodyptr,
					  true);
		break;
#ifdef WLAN_NS_OFFLOAD
	case WMA_SET_NS_OFFLOAD:
		wma_enable_arp_ns_offload(wma_handle,
					  (tpSirHostOffloadReq) msg->bodyptr,
					  false);
		break;
#endif /*WLAN_NS_OFFLOAD */
	case WMA_ROAM_SCAN_OFFLOAD_REQ:
		/*
		 * Main entry point or roaming directives from CSR.
		 */
		wma_process_roaming_config(wma_handle,
				(tSirRoamOffloadScanReq *) msg->bodyptr);
		break;

	case WMA_RATE_UPDATE_IND:
		wma_process_rate_update_indicate(wma_handle,
				(tSirRateUpdateInd *) msg->bodyptr);
		break;

#ifdef FEATURE_WLAN_TDLS
	case WMA_UPDATE_FW_TDLS_STATE:
		wma_update_fw_tdls_state(wma_handle,
					 (t_wma_tdls_params *) msg->bodyptr);
		break;
	case WMA_UPDATE_TDLS_PEER_STATE:
		wma_update_tdls_peer_state(wma_handle,
				(tTdlsPeerStateParams *) msg->bodyptr);
		break;
	case WMA_TDLS_SET_OFFCHAN_MODE:
		wma_set_tdls_offchan_mode(wma_handle,
			(tdls_chan_switch_params *)msg->bodyptr);
		break;
#endif /* FEATURE_WLAN_TDLS */
	case WMA_ADD_PERIODIC_TX_PTRN_IND:
		wma_process_add_periodic_tx_ptrn_ind(wma_handle,
				(tSirAddPeriodicTxPtrn *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_DEL_PERIODIC_TX_PTRN_IND:
		wma_process_del_periodic_tx_ptrn_ind(wma_handle,
				(tSirDelPeriodicTxPtrn *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_TX_POWER_LIMIT:
		wma_process_tx_power_limits(wma_handle,
					    (tSirTxPowerLimit *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#ifdef FEATURE_WLAN_LPHB
	case WMA_LPHB_CONF_REQ:
		wma_process_lphb_conf_req(wma_handle,
					  (tSirLPHBReq *) msg->bodyptr);
		break;
#endif /* FEATURE_WLAN_LPHB */

#ifdef FEATURE_WLAN_CH_AVOID
	case WMA_CH_AVOID_UPDATE_REQ:
		wma_process_ch_avoid_update_req(wma_handle,
				(tSirChAvoidUpdateReq *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#endif /* FEATURE_WLAN_CH_AVOID */
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	case WMA_SET_AUTO_SHUTDOWN_TIMER_REQ:
		wma_set_auto_shutdown_timer_req(wma_handle, msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#endif /* FEATURE_WLAN_AUTO_SHUTDOWN */
	case WMA_DHCP_START_IND:
	case WMA_DHCP_STOP_IND:
		wma_process_dhcp_ind(wma_handle, (tAniDHCPInd *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;

	case WMA_IBSS_CESIUM_ENABLE_IND:
		wma_process_cesium_enable_ind(wma_handle);
		break;
	case WMA_GET_IBSS_PEER_INFO_REQ:
		wma_process_get_peer_info_req(wma_handle,
					      (tSirIbssGetPeerInfoReqParams *)
					      msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_TX_FAIL_MONITOR_IND:
		wma_process_tx_fail_monitor_ind(wma_handle,
				(tAniTXFailMonitorInd *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;

	case WMA_RMC_ENABLE_IND:
		wma_process_rmc_enable_ind(wma_handle);
		break;
	case WMA_RMC_DISABLE_IND:
		wma_process_rmc_disable_ind(wma_handle);
		break;
	case WMA_RMC_ACTION_PERIOD_IND:
		wma_process_rmc_action_period_ind(wma_handle);
		break;
	case WMA_INIT_THERMAL_INFO_CMD:
		wma_process_init_thermal_info(wma_handle,
					      (t_thermal_mgmt *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;

	case WMA_SET_THERMAL_LEVEL:
		wma_process_set_thermal_level(wma_handle, msg->bodyval);
		break;

	case WMA_SET_P2P_GO_NOA_REQ:
		wma_process_set_p2pgo_noa_req(wma_handle,
					      (tP2pPsParams *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_SET_MIMOPS_REQ:
		wma_process_set_mimops_req(wma_handle,
					   (tSetMIMOPS *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_SET_SAP_INTRABSS_DIS:
		wma_set_vdev_intrabss_fwd(wma_handle,
					  (tDisableIntraBssFwd *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_GET_LINK_SPEED:
		wma_get_link_speed(wma_handle, msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_MODEM_POWER_STATE_IND:
		wma_notify_modem_power_state(wma_handle,
				(tSirModemPowerStateInd *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_WLAN_RESUME_REQ:
		wma_resume_req(wma_handle, QDF_SYSTEM_SUSPEND);
		break;

#ifdef WLAN_FEATURE_STATS_EXT
	case WMA_STATS_EXT_REQUEST:
		wma_stats_ext_req(wma_handle,
				  (tpStatsExtRequest) (msg->bodyptr));
		qdf_mem_free(msg->bodyptr);
		break;
#endif /* WLAN_FEATURE_STATS_EXT */
	case WMA_HIDDEN_SSID_VDEV_RESTART:
		wma_hidden_ssid_vdev_restart(wma_handle,
				(tHalHiddenSsidVdevRestart *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
	case WMA_WLAN_EXT_WOW:
		wma_enable_ext_wow(wma_handle,
				   (tSirExtWoWParams *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_WLAN_SET_APP_TYPE1_PARAMS:
		wma_set_app_type1_params_in_fw(wma_handle,
				(tSirAppType1Params *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_WLAN_SET_APP_TYPE2_PARAMS:
		wma_set_app_type2_params_in_fw(wma_handle,
				(tSirAppType2Params *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#endif /* WLAN_FEATURE_EXTWOW_SUPPORT */
#ifdef FEATURE_WLAN_EXTSCAN
	case WMA_EXTSCAN_START_REQ:
		wma_start_extscan(wma_handle,
				  (tSirWifiScanCmdReqParams *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_EXTSCAN_STOP_REQ:
		wma_stop_extscan(wma_handle,
				 (tSirExtScanStopReqParams *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_EXTSCAN_SET_BSSID_HOTLIST_REQ:
		wma_extscan_start_hotlist_monitor(wma_handle,
			(tSirExtScanSetBssidHotListReqParams *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_EXTSCAN_RESET_BSSID_HOTLIST_REQ:
		wma_extscan_stop_hotlist_monitor(wma_handle,
			(tSirExtScanResetBssidHotlistReqParams *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_EXTSCAN_SET_SIGNF_CHANGE_REQ:
		wma_extscan_start_change_monitor(wma_handle,
			(tSirExtScanSetSigChangeReqParams *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_EXTSCAN_RESET_SIGNF_CHANGE_REQ:
		wma_extscan_stop_change_monitor(wma_handle,
			(tSirExtScanResetSignificantChangeReqParams *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_EXTSCAN_GET_CACHED_RESULTS_REQ:
		wma_extscan_get_cached_results(wma_handle,
			(tSirExtScanGetCachedResultsReqParams *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_EXTSCAN_GET_CAPABILITIES_REQ:
		wma_extscan_get_capabilities(wma_handle,
			(tSirGetExtScanCapabilitiesReqParams *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_SET_EPNO_LIST_REQ:
		wma_set_epno_network_list(wma_handle,
			(struct wifi_epno_params *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_SET_PASSPOINT_LIST_REQ:
		/* Issue reset passpoint network list first and clear
		 * the entries */
		wma_reset_passpoint_network_list(wma_handle,
			(struct wifi_passpoint_req *)msg->bodyptr);

		wma_set_passpoint_network_list(wma_handle,
			(struct wifi_passpoint_req *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_RESET_PASSPOINT_LIST_REQ:
		wma_reset_passpoint_network_list(wma_handle,
			(struct wifi_passpoint_req *)msg->bodyptr);
		break;
	case WMA_EXTSCAN_SET_SSID_HOTLIST_REQ:
		wma_set_ssid_hotlist(wma_handle,
			(struct sir_set_ssid_hotlist_request *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#endif /* FEATURE_WLAN_EXTSCAN */
	case WMA_SET_SCAN_MAC_OUI_REQ:
		wma_scan_probe_setoui(wma_handle, msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#ifdef WLAN_FEATURE_LINK_LAYER_STATS
	case WMA_LINK_LAYER_STATS_CLEAR_REQ:
		wma_process_ll_stats_clear_req(wma_handle,
			(tpSirLLStatsClearReq) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_LINK_LAYER_STATS_SET_REQ:
		wma_process_ll_stats_set_req(wma_handle,
					     (tpSirLLStatsSetReq) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_LINK_LAYER_STATS_GET_REQ:
		wma_process_ll_stats_get_req(wma_handle,
					     (tpSirLLStatsGetReq) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */
	case SIR_HAL_UNIT_TEST_CMD:
		wma_process_unit_test_cmd(wma_handle,
					  (t_wma_unit_test_cmd *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	case WMA_ROAM_OFFLOAD_SYNCH_FAIL:
		wma_process_roam_synch_fail(wma_handle,
			(struct roam_offload_synch_fail *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case SIR_HAL_ROAM_INVOKE:
		wma_process_roam_invoke(wma_handle,
			(struct wma_roam_invoke_cmd *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
#ifdef WLAN_FEATURE_NAN
	case WMA_NAN_REQUEST:
		wma_nan_req(wma_handle, (tNanRequest *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#endif /* WLAN_FEATURE_NAN */
	case SIR_HAL_SET_BASE_MACADDR_IND:
		wma_set_base_macaddr_indicate(wma_handle,
					      (tSirMacAddr *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_LINK_STATUS_GET_REQ:
		wma_process_link_status_req(wma_handle,
					    (tAniGetLinkStatus *) msg->bodyptr);
		break;
	case WMA_GET_TEMPERATURE_REQ:
		wma_get_temperature(wma_handle);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_TSF_GPIO_PIN:
		wma_set_tsf_gpio_pin(wma_handle, msg->bodyval);
		break;

#ifdef DHCP_SERVER_OFFLOAD
	case WMA_SET_DHCP_SERVER_OFFLOAD_CMD:
		wma_process_dhcpserver_offload(wma_handle,
			(tSirDhcpSrvOffloadInfo *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#endif /* DHCP_SERVER_OFFLOAD */
#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
	case WMA_LED_FLASHING_REQ:
		wma_set_led_flashing(wma_handle,
				     (tSirLedFlashingReq *) msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
#endif /* WLAN_FEATURE_GPIO_LED_FLASHING */
	case SIR_HAL_SET_MAS:
		wma_process_set_mas(wma_handle,
				(uint32_t *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case SIR_HAL_SET_MIRACAST:
		wma_process_set_miracast(wma_handle,
				(uint32_t *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case SIR_HAL_CONFIG_STATS_FACTOR:
		wma_config_stats_factor(wma_handle,
					(struct sir_stats_avg_factor *)
					msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case SIR_HAL_CONFIG_GUARD_TIME:
		wma_config_guard_time(wma_handle,
				      (struct sir_guard_time_request *)
				      msg->bodyptr);
	case WMA_IPA_OFFLOAD_ENABLE_DISABLE:
		wma_ipa_offload_enable_disable(wma_handle,
			(struct sir_ipa_offload_enable_disable *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case SIR_HAL_START_STOP_LOGGING:
		wma_set_wifi_start_packet_stats(wma_handle,
				(struct sir_wifi_start_log *)msg->bodyptr);
		wma_enable_specific_fw_logs(wma_handle,
				(struct sir_wifi_start_log *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case SIR_HAL_FLUSH_LOG_TO_FW:
		wma_send_flush_logs_to_fw(wma_handle);
		/* Body ptr is NULL here */
		break;
	case WMA_SET_RSSI_MONITOR_REQ:
		wma_set_rssi_monitoring(wma_handle,
			(struct rssi_monitor_req *)msg->bodyptr);
		break;
	case WMA_FW_MEM_DUMP_REQ:
		wma_process_fw_mem_dump_req(wma_handle,
			(struct fw_dump_req *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case SIR_HAL_PDEV_SET_PCL_TO_FW:
		wma_send_pdev_set_pcl_cmd(wma_handle,
				(struct wmi_pcl_chan_weights *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case SIR_HAL_PDEV_SET_HW_MODE:
		wma_send_pdev_set_hw_mode_cmd(wma_handle,
				(struct sir_hw_mode *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_OCB_SET_CONFIG_CMD:
		wma_ocb_set_config_req(wma_handle,
			(struct sir_ocb_config *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_OCB_SET_UTC_TIME_CMD:
		wma_ocb_set_utc_time(wma_handle,
			(struct sir_ocb_utc *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_OCB_START_TIMING_ADVERT_CMD:
		wma_ocb_start_timing_advert(wma_handle,
			(struct sir_ocb_timing_advert *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_OCB_STOP_TIMING_ADVERT_CMD:
		wma_ocb_stop_timing_advert(wma_handle,
			(struct sir_ocb_timing_advert *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_DCC_CLEAR_STATS_CMD:
		wma_dcc_clear_stats(wma_handle,
			(struct sir_dcc_clear_stats *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_OCB_GET_TSF_TIMER_CMD:
		wma_ocb_get_tsf_timer(wma_handle,
			(struct sir_ocb_get_tsf_timer *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_SET_WISA_PARAMS:
		wma_set_wisa_params(wma_handle,
			(struct sir_wisa_params *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_DCC_GET_STATS_CMD:
		wma_dcc_get_stats(wma_handle,
			(struct sir_dcc_get_stats *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_DCC_UPDATE_NDL_CMD:
		wma_dcc_update_ndl(wma_handle,
			(struct sir_dcc_update_ndl *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case SIR_HAL_PDEV_DUAL_MAC_CFG_REQ:
		wma_send_pdev_set_dual_mac_config(wma_handle,
				(struct sir_dual_mac_config *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_SET_IE_INFO:
		wma_process_set_ie_info(wma_handle,
			(struct vdev_ie_info *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case SIR_HAL_SOC_ANTENNA_MODE_REQ:
		wma_send_pdev_set_antenna_mode(wma_handle,
			(struct sir_antenna_mode_param *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_LRO_CONFIG_CMD:
		wma_lro_config_cmd(wma_handle,
			(struct wma_lro_config_cmd_t *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_GW_PARAM_UPDATE_REQ:
		wma_set_gateway_params(wma_handle,
			(struct gateway_param_update_req *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_SET_EGAP_CONF_PARAMS:
		wma_send_egap_conf_params(wma_handle,
			(struct egap_conf_params *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_HT40_OBSS_SCAN_IND:
		wma_send_ht40_obss_scanind(wma_handle,
			(struct obss_ht40_scanind *)msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_ADD_BCN_FILTER_CMDID:
		wma_add_beacon_filter(wma_handle, msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WMA_REMOVE_BCN_FILTER_CMDID:
		wma_remove_beacon_filter(wma_handle, msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case WDA_BPF_GET_CAPABILITIES_REQ:
		wma_get_bpf_capabilities(wma_handle);
		break;
	case WDA_BPF_SET_INSTRUCTIONS_REQ:
		wma_set_bpf_instructions(wma_handle, msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;

	default:
		WMA_LOGD("unknow msg type %x", msg->type);
		/* Do Nothing? MSG Body should be freed at here */
		if (NULL != msg->bodyptr) {
			qdf_mem_free(msg->bodyptr);
		}
	}
end:
	WMA_LOGI("%s: Exit", __func__);
	return qdf_status;
}

/**
 * wma_log_completion_timeout() - Log completion timeout
 * @data: Timeout handler data
 *
 * This function is called when log completion timer expires
 *
 * Return: None
 */
void wma_log_completion_timeout(void *data)
{
	tp_wma_handle wma_handle;

	WMA_LOGE("%s: Timeout occured for log completion command", __func__);

	wma_handle = (tp_wma_handle) data;
	if (!wma_handle)
		WMA_LOGE("%s: Invalid WMA handle", __func__);

	/* Though we did not receive any event from FW,
	 * we can flush whatever logs we have with us */
	cds_logging_set_fw_flush_complete();

	return;
}

/**
 * wma_send_pdev_set_pcl_cmd() - Send WMI_SOC_SET_PCL_CMDID to FW
 * @wma_handle: WMA handle
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
QDF_STATUS wma_send_pdev_set_pcl_cmd(tp_wma_handle wma_handle,
				struct wmi_pcl_chan_weights *msg)
{
	uint32_t i;
	QDF_STATUS status;

	if (!wma_handle) {
		WMA_LOGE("%s: WMA handle is NULL. Cannot issue command",
				__func__);
		return QDF_STATUS_E_NULL_VALUE;
	}

	for (i = 0; i < wma_handle->saved_chan.num_channels; i++) {
		msg->saved_chan_list[i] =
			wma_handle->saved_chan.channel_list[i];
	}

	msg->saved_num_chan = wma_handle->saved_chan.num_channels;
	status = cds_get_valid_chan_weights((struct sir_pcl_chan_weights *)msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		WMA_LOGE("%s: Error in creating weighed pcl", __func__);
		return status;
	}

	if (wmi_unified_pdev_set_pcl_cmd(wma_handle->wmi_handle, msg))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_send_pdev_set_hw_mode_cmd() - Send WMI_PDEV_SET_HW_MODE_CMDID to FW
 * @wma_handle: WMA handle
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
QDF_STATUS wma_send_pdev_set_hw_mode_cmd(tp_wma_handle wma_handle,
				struct sir_hw_mode *msg)
{
	struct sir_set_hw_mode_resp *param;

	if (!wma_handle) {
		WMA_LOGE("%s: WMA handle is NULL. Cannot issue command",
				__func__);
		/* Handle is NULL. Will not be able to send failure
		 * response as well
		 */
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!msg) {
		WMA_LOGE("%s: Set HW mode param is NULL", __func__);
		/* Lets try to free the active command list */
		goto fail;
	}

	if (wmi_unified_soc_set_hw_mode_cmd(wma_handle->wmi_handle,
				msg->hw_mode_index))
		goto fail;

	return QDF_STATUS_SUCCESS;
fail:
	param = qdf_mem_malloc(sizeof(*param));
	if (!param) {
		WMA_LOGE("%s: Memory allocation failed", __func__);
		return QDF_STATUS_E_NULL_VALUE;
	}
	param->status = SET_HW_MODE_STATUS_ECANCELED;
	param->cfgd_hw_mode_index = 0;
	param->num_vdev_mac_entries = 0;
	WMA_LOGE("%s: Sending HW mode fail response to LIM", __func__);
	wma_send_msg(wma_handle, SIR_HAL_PDEV_SET_HW_MODE_RESP,
			(void *) param, 0);
	return QDF_STATUS_SUCCESS;
}

/**
 * wma_send_pdev_set_dual_mac_config() - Set dual mac config to FW
 * @wma_handle: WMA handle
 * @msg: Dual MAC config parameters
 *
 * Configures WLAN firmware with the dual MAC features
 *
 * Return: QDF_STATUS. 0 on success.
 */
QDF_STATUS wma_send_pdev_set_dual_mac_config(tp_wma_handle wma_handle,
		struct sir_dual_mac_config *msg)
{
	QDF_STATUS status;

	if (!wma_handle) {
		WMA_LOGE("%s: WMA handle is NULL. Cannot issue command",
				__func__);
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!msg) {
		WMA_LOGE("%s: Set dual mode config is NULL", __func__);
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = wmi_unified_pdev_set_dual_mac_config_cmd(
				wma_handle->wmi_handle,
				(struct wmi_dual_mac_config *)msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("%s: Failed to send WMI_PDEV_SET_DUAL_MAC_CONFIG_CMDID: %d",
				__func__, status);
		return status;
	}

	wma_handle->dual_mac_cfg.req_scan_config = msg->scan_config;
	wma_handle->dual_mac_cfg.req_fw_mode_config = msg->fw_mode_config;

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_send_pdev_set_antenna_mode() - Set antenna mode to FW
 * @wma_handle: WMA handle
 * @msg: Antenna mode parameters
 *
 * Send WMI_PDEV_SET_ANTENNA_MODE_CMDID to FW requesting to
 * modify the number of TX/RX chains from host
 *
 * Return: QDF_STATUS. 0 on success.
 */
QDF_STATUS wma_send_pdev_set_antenna_mode(tp_wma_handle wma_handle,
		struct sir_antenna_mode_param *msg)
{
	wmi_pdev_set_antenna_mode_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint32_t len;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct sir_antenna_mode_resp *param;

	if (!wma_handle) {
		WMA_LOGE("%s: WMA handle is NULL. Cannot issue command",
				__func__);
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!msg) {
		WMA_LOGE("%s: Set antenna mode param is NULL", __func__);
		return QDF_STATUS_E_NULL_VALUE;
	}

	len = sizeof(*cmd);

	buf = wmi_buf_alloc(wma_handle->wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s: wmi_buf_alloc failed", __func__);
		status = QDF_STATUS_E_NOMEM;
		goto resp;
	}

	cmd = (wmi_pdev_set_antenna_mode_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_set_antenna_mode_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_pdev_set_antenna_mode_cmd_fixed_param));

	cmd->pdev_id = WMI_PDEV_ID_SOC;
	/* Bits 0-15 is num of RX chains 16-31 is num of TX chains */
	cmd->num_txrx_chains = msg->num_rx_chains;
	cmd->num_txrx_chains |= (msg->num_tx_chains << 16);

	WMA_LOGI("%s: Num of chains TX: %d RX: %d txrx_chains: 0x%x",
		 __func__, msg->num_tx_chains,
		 msg->num_rx_chains, cmd->num_txrx_chains);

	if (wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len,
				 WMI_PDEV_SET_ANTENNA_MODE_CMDID)) {
		WMA_LOGE("%s: Failed to send WMI_PDEV_SET_ANTENNA_MODE_CMDID",
				__func__);
		wmi_buf_free(buf);
		status = QDF_STATUS_E_FAILURE;
		goto resp;
	}
	status = QDF_STATUS_SUCCESS;

resp:
	param = qdf_mem_malloc(sizeof(*param));
	if (!param) {
		WMA_LOGE("%s: Memory allocation failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	param->status = (status) ?
		SET_ANTENNA_MODE_STATUS_ECANCELED :
		SET_ANTENNA_MODE_STATUS_OK;
	WMA_LOGE("%s: Send antenna mode resp to LIM status: %d",
		 __func__, param->status);
	wma_send_msg(wma_handle, SIR_HAL_SOC_ANTENNA_MODE_RESP,
			(void *) param, 0);
	return status;
}

/**
 * wma_crash_inject() - sends command to FW to simulate crash
 * @wma_handle:         pointer of WMA context
 * @type:               subtype of the command
 * @delay_time_ms:      time in milliseconds for FW to delay the crash
 *
 * This function will send a command to FW in order to simulate different
 * kinds of FW crashes.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_crash_inject(tp_wma_handle wma_handle, uint32_t type,
			uint32_t delay_time_ms)
{
	struct crash_inject param;
	param.type = type;
	param.delay_time_ms = delay_time_ms;

	return wmi_crash_inject(wma_handle->wmi_handle, &param);
}

#if defined(FEATURE_LRO)
/**
 * wma_lro_init() - sends LRO configuration to FW
 * @lro_config:         pointer to the config parameters
 *
 * This function ends LRO configuration to FW.
 *
 * Return: 0 for success or reasons for failure
 */
int wma_lro_init(struct wma_lro_config_cmd_t *lro_config)
{
	cds_msg_t msg = {0};
	struct wma_lro_config_cmd_t *iwcmd;

	iwcmd = qdf_mem_malloc(sizeof(*iwcmd));
	if (!iwcmd) {
		WMA_LOGE("memory allocation for WMA_LRO_CONFIG_CMD failed!");
		return -ENOMEM;
	}

	*iwcmd = *lro_config;

	msg.type = WMA_LRO_CONFIG_CMD;
	msg.reserved = 0;
	msg.bodyptr = iwcmd;

	if (QDF_STATUS_SUCCESS !=
		cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
		WMA_LOGE("Failed to post WMA_LRO_CONFIG_CMD msg!");
		qdf_mem_free(iwcmd);
		return -EAGAIN;
	}

	WMA_LOGD("sending the LRO configuration to the fw");
	return 0;
}
#endif
