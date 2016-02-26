/*
 * Copyright (c) 2011-2015 The Linux Foundation. All rights reserved.
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

#include "cds_api.h"
#include "wni_cfg.h"
#include "ani_global.h"
#include "sir_api.h"
#include "sir_params.h"
#include "cfg_api.h"

#include "sch_api.h"
#include "utils_api.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_prop_exts_utils.h"
#include "lim_security_utils.h"
#include "lim_send_messages.h"
#include "lim_send_messages.h"
#include "lim_session_utils.h"
#ifdef WLAN_FEATURE_VOWIFI_11R
#include <lim_ft.h>
#endif
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
#include "host_diag_core_log.h"
#endif
#include "wma_if.h"

static void lim_process_mlm_start_req(tpAniSirGlobal, uint32_t *);
#ifdef FEATURE_OEM_DATA_SUPPORT
static void lim_process_mlm_oem_data_req(tpAniSirGlobal, uint32_t *);
#endif
static void lim_process_mlm_join_req(tpAniSirGlobal, uint32_t *);
static void lim_process_mlm_auth_req(tpAniSirGlobal, uint32_t *);
static void lim_process_mlm_assoc_req(tpAniSirGlobal, uint32_t *);
static void lim_process_mlm_reassoc_req(tpAniSirGlobal, uint32_t *);
static void lim_process_mlm_disassoc_req(tpAniSirGlobal, uint32_t *);
static void lim_process_mlm_deauth_req(tpAniSirGlobal, uint32_t *);
static void lim_process_mlm_set_keys_req(tpAniSirGlobal, uint32_t *);

/* MLM Timeout event handler templates */
static void lim_process_periodic_probe_req_timer(tpAniSirGlobal mac_ctx);
static void lim_process_join_failure_timeout(tpAniSirGlobal);
static void lim_process_auth_failure_timeout(tpAniSirGlobal);
static void lim_process_auth_rsp_timeout(tpAniSirGlobal, uint32_t);
static void lim_process_assoc_failure_timeout(tpAniSirGlobal, uint32_t);
static void lim_process_periodic_join_probe_req_timer(tpAniSirGlobal);

/**
 * lim_process_mlm_req_messages() - process mlm request messages
 * @mac_ctx: global MAC context
 * @msg: mlm request message
 *
 * This function is called by lim_post_mlm_message(). This
 * function handles MLM primitives invoked by SME.
 * Depending on the message type, corresponding function will be
 * called.
 * ASSUMPTIONS:
 * 1. Upon receiving Beacon in WT_JOIN_STATE, MLM module invokes
 *    APIs exposed by Beacon Processing module for setting parameters
 *    at MAC hardware.
 * 2. If attempt to Reassociate with an AP fails, link with current
 *    AP is restored back.
 *
 * Return: None
 */
void lim_process_mlm_req_messages(tpAniSirGlobal mac_ctx, tpSirMsgQ msg)
{
	MTRACE(mac_trace_msg_rx(mac_ctx, NO_SESSION, msg->type));
	switch (msg->type) {
	case LIM_MLM_START_REQ:
		lim_process_mlm_start_req(mac_ctx, msg->bodyptr);
		break;
#ifdef FEATURE_OEM_DATA_SUPPORT
	case LIM_MLM_OEM_DATA_REQ:
		lim_process_mlm_oem_data_req(mac_ctx, msg->bodyptr);
		break;
#endif
	case LIM_MLM_JOIN_REQ:
		lim_process_mlm_join_req(mac_ctx, msg->bodyptr);
		break;
	case LIM_MLM_AUTH_REQ:
		lim_process_mlm_auth_req(mac_ctx, msg->bodyptr);
		break;
	case LIM_MLM_ASSOC_REQ:
		lim_process_mlm_assoc_req(mac_ctx, msg->bodyptr);
		break;
	case LIM_MLM_REASSOC_REQ:
		lim_process_mlm_reassoc_req(mac_ctx, msg->bodyptr);
		break;
	case LIM_MLM_DISASSOC_REQ:
		lim_process_mlm_disassoc_req(mac_ctx, msg->bodyptr);
		break;
	case LIM_MLM_DEAUTH_REQ:
		lim_process_mlm_deauth_req(mac_ctx, msg->bodyptr);
		break;
	case LIM_MLM_SETKEYS_REQ:
		lim_process_mlm_set_keys_req(mac_ctx, msg->bodyptr);
		break;
	case SIR_LIM_PERIODIC_PROBE_REQ_TIMEOUT:
		lim_process_periodic_probe_req_timer(mac_ctx);
		break;
	case SIR_LIM_JOIN_FAIL_TIMEOUT:
		lim_process_join_failure_timeout(mac_ctx);
		break;
	case SIR_LIM_PERIODIC_JOIN_PROBE_REQ_TIMEOUT:
		lim_process_periodic_join_probe_req_timer(mac_ctx);
		break;
	case SIR_LIM_AUTH_FAIL_TIMEOUT:
		lim_process_auth_failure_timeout(mac_ctx);
		break;
	case SIR_LIM_AUTH_RSP_TIMEOUT:
		lim_process_auth_rsp_timeout(mac_ctx, msg->bodyval);
		break;
	case SIR_LIM_ASSOC_FAIL_TIMEOUT:
		lim_process_assoc_failure_timeout(mac_ctx, msg->bodyval);
		break;
#ifdef WLAN_FEATURE_VOWIFI_11R
	case SIR_LIM_FT_PREAUTH_RSP_TIMEOUT:
		lim_process_ft_preauth_rsp_timeout(mac_ctx);
		break;
#endif
	case SIR_LIM_REMAIN_CHN_TIMEOUT:
		lim_process_remain_on_chn_timeout(mac_ctx);
		break;
	case SIR_LIM_INSERT_SINGLESHOT_NOA_TIMEOUT:
		lim_process_insert_single_shot_noa_timeout(mac_ctx);
		break;
	case SIR_LIM_CONVERT_ACTIVE_CHANNEL_TO_PASSIVE:
		lim_convert_active_channel_to_passive_channel(mac_ctx);
		break;
	case SIR_LIM_DISASSOC_ACK_TIMEOUT:
		lim_process_disassoc_ack_timeout(mac_ctx);
		break;
	case SIR_LIM_DEAUTH_ACK_TIMEOUT:
		lim_process_deauth_ack_timeout(mac_ctx);
		break;
	case LIM_MLM_TSPEC_REQ:
	default:
		break;
	} /* switch (msg->type) */
}

/* WLAN_SUSPEND_LINK Related */

/**
 * lim_is_link_suspended()- check if link is suspended
 * @mac_ctx: global MAC context
 *
 * This function returns is link is suspended or not.
 * Since Suspend link uses init scan, it just returns
 * gLimSystemInScanLearnMode flag.
 *
 * Return: uint8_t(gLimSystemInScanLearnMode flag)
 */
uint8_t lim_is_link_suspended(tpAniSirGlobal mac_ctx)
{
	return mac_ctx->lim.gLimSystemInScanLearnMode;
}

/**
 * lim_change_channel_with_callback() - change channel and register callback
 * @mac_ctx: global MAC context
 * @new_chan: new channel to switch
 * @callback: Callback function
 * @cbdata: callback data
 * @session_entry: PE session pointer
 *
 * This function is called to change channel and perform off channel operation
 * if required. The caller registers a callback to be called at the end of the
 * channel change.
 *
 * Return: None
 */
void
lim_change_channel_with_callback(tpAniSirGlobal mac_ctx, uint8_t new_chan,
				 CHANGE_CHANNEL_CALLBACK callback,
				 uint32_t *cbdata, tpPESession session_entry)
{
	/* Sanity checks for the current and new channel */
#if defined WLAN_VOWIFI_DEBUG
	lim_log(mac_ctx, LOGE, FL("Switching channel to %d"), new_chan);
#endif
	session_entry->channelChangeReasonCode =
		LIM_SWITCH_CHANNEL_OPERATION;

	mac_ctx->lim.gpchangeChannelCallback = callback;
	mac_ctx->lim.gpchangeChannelData = cbdata;

	lim_send_switch_chnl_params(mac_ctx, new_chan, 0, 0,
		CH_WIDTH_20MHZ, session_entry->maxTxPower,
		session_entry->peSessionId, false);

	return;
}

/**
 * lim_covert_channel_scan_type() - switch between ACTIVE and PASSIVE scan type
 * @mac_ctx: global MAC context
 * @chan_num: channel number to change the scan type
 * @passive_to_active: flag to indicate if switch allowed
 *
 * This function is called to get the list,
 * change the channel type and set again.
 * NOTE: If a channel is ACTIVE, this function will make it as PASSIVE
 *       If a channel is PASSIVE, this fucntion will make it as ACTIVE
 *
 * Return: None
 */

void lim_covert_channel_scan_type(tpAniSirGlobal mac_ctx, uint8_t chan_num,
				  bool passive_to_active)
{

	uint32_t i;
	uint8_t chan_pair[WNI_CFG_SCAN_CONTROL_LIST_LEN];
	uint32_t len = WNI_CFG_SCAN_CONTROL_LIST_LEN;
	tSirRetStatus status;

	status  = wlan_cfg_get_str(mac_ctx, WNI_CFG_SCAN_CONTROL_LIST,
				   chan_pair, &len);
	if (eSIR_SUCCESS != status) {
		lim_log(mac_ctx, LOGE, FL("Unable to get scan control list"));
		return;
	}
	if (len > WNI_CFG_SCAN_CONTROL_LIST_LEN) {
		lim_log(mac_ctx, LOGE,
			FL("Invalid scan control list length:%d"), len);
		return;
	}
	for (i = 0; (i + 1) < len; i += 2) {
		if (chan_pair[i] != chan_num) /* skip this channel */
			continue;
		if ((eSIR_PASSIVE_SCAN == chan_pair[i + 1]) &&
		     true == passive_to_active) {
			lim_log(mac_ctx, LOG1, FL
				("Channel %d changed from Passive to Active"),
				chan_num);
			chan_pair[i + 1] = eSIR_ACTIVE_SCAN;
			break;
		}
		if ((eSIR_ACTIVE_SCAN == chan_pair[i + 1]) &&
		     false == passive_to_active) {
			lim_log(mac_ctx, LOG1, FL
				("Channel %d changed from Active to Passive"),
				chan_num);
			chan_pair[i + 1] = eSIR_PASSIVE_SCAN;
			break;
		}
	}

	cfg_set_str_notify(mac_ctx, WNI_CFG_SCAN_CONTROL_LIST,
			   (uint8_t *) chan_pair, len, false);
	return;
}

/**
 * lim_set_dfs_channel_list() - convert dfs channel list to active channel list
 * @mac_ctx: global MAC context.
 * @chan_num: channel number
 * @dfs_ch_list: list of DFS channels
 *
 * This function is called to convert DFS channel list to active channel list
 * when any beacon is present on that channel. This function store time for
 * passive channels which help to know that for how much time channel has been
 * passive.
 *
 * NOTE: If a channel is ACTIVE, it won't store any time
 *       If a channel is PAssive, it will store time as timestamp
 *
 * Return: None
 */
void lim_set_dfs_channel_list(tpAniSirGlobal mac_ctx, uint8_t chan_num,
			      tSirDFSChannelList *dfs_ch_list)
{
	bool pass_to_active = true;

	if (!((1 <= chan_num) && (165 >= chan_num))) {
		lim_log(mac_ctx, LOGE, FL("Invalid Channel: %d"), chan_num);
		return;
	}

	if (true == lim_isconnected_on_dfs_channel(chan_num)) {
		if (dfs_ch_list->timeStamp[chan_num] == 0) {
			/*
			 * Received first beacon;
			 * Convert DFS channel to Active channel.
			 */
			lim_log(mac_ctx, LOG1,
				FL("Received first beacon on DFS channel: %d"),
				chan_num);
			lim_covert_channel_scan_type(mac_ctx, chan_num,
						     pass_to_active);
		}
		dfs_ch_list->timeStamp[chan_num] =
					cdf_mc_timer_get_system_time();
	} else {
		lim_log(mac_ctx, LOG1, FL("Channel %d is Active"), chan_num);
		return;
	}

	if (!tx_timer_running
		    (&mac_ctx->lim.limTimers.gLimActiveToPassiveChannelTimer)) {
		tx_timer_activate(
		    &mac_ctx->lim.limTimers.gLimActiveToPassiveChannelTimer);
	}

	return;
}

/**
 * lim_restore_pre_scan_state() - restore HW state prior to scan
 *
 * @mac_ctx: global MAC context
 *
 * This function is called by lim_continue_channel_scan()
 * to restore HW state prior to entering 'scan state'
 *
 * Return: None
 */
void lim_restore_pre_scan_state(tpAniSirGlobal mac_ctx)
{
	/* Deactivate MIN/MAX channel timers if running */
	lim_deactivate_and_change_timer(mac_ctx, eLIM_MIN_CHANNEL_TIMER);
	lim_deactivate_and_change_timer(mac_ctx, eLIM_MAX_CHANNEL_TIMER);

	mac_ctx->lim.gLimSystemInScanLearnMode = 0;
	lim_log(mac_ctx, LOG1, FL("Scan ended, took %llu tu"),
		(tx_time_get() - mac_ctx->lim.scanStartTime));
}

#ifdef FEATURE_OEM_DATA_SUPPORT
/**
 * lim_send_hal_oem_data_req() - send oem data request
 * @mac_ctx: global MAC context
 *
 * This function is used to send OEM data request to HAL.
 *
 * Return: None
 */
void lim_send_hal_oem_data_req(tpAniSirGlobal mac_ctx)
{
	tSirMsgQ msg;
	tpStartOemDataReq start_oem_data_req = NULL;
	tSirRetStatus rc = eSIR_SUCCESS;
	tpLimMlmOemDataRsp mlm_oem_data_rsp;
	uint32_t reqlen = 0;

	if (NULL == mac_ctx->lim.gpLimMlmOemDataReq) {
		lim_log(mac_ctx, LOGE, FL("Null pointer"));
		goto error;
	}

	reqlen = sizeof(tStartOemDataReq);

	start_oem_data_req = cdf_mem_malloc(reqlen);
	if (NULL == start_oem_data_req) {
		lim_log(mac_ctx, LOGE, FL
			("Could not allocate memory for start_oem_data_req"));
		goto error;
	}

	cdf_mem_set((uint8_t *) (start_oem_data_req), reqlen, 0);

	/* Now copy over the information to the OEM DATA REQ to HAL */
	cdf_copy_macaddr(&start_oem_data_req->selfMacAddr,
			 &mac_ctx->lim.gpLimMlmOemDataReq->selfMacAddr);

	cdf_mem_copy(start_oem_data_req->oemDataReq,
		     mac_ctx->lim.gpLimMlmOemDataReq->oemDataReq,
		     OEM_DATA_REQ_SIZE);

	/* Create the message to be passed to HAL */
	msg.type = WMA_START_OEM_DATA_REQ;
	msg.bodyptr = start_oem_data_req;
	msg.bodyval = 0;

	SET_LIM_PROCESS_DEFD_MESGS(mac_ctx, false);
	MTRACE(mac_trace_msg_tx(mac_ctx, NO_SESSION, msg.type));

	rc = wma_post_ctrl_msg(mac_ctx, &msg);
	if (rc == eSIR_SUCCESS)
		return;

	SET_LIM_PROCESS_DEFD_MESGS(mac_ctx, true);
	cdf_mem_free(start_oem_data_req);
	lim_log(mac_ctx, LOGE,
		FL("OEM_DATA: posting WMA_START_OEM_DATA_REQ to HAL failed"));

error:
	mac_ctx->lim.gLimMlmState = mac_ctx->lim.gLimPrevMlmState;
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE, NO_SESSION,
		       mac_ctx->lim.gLimMlmState));

	mlm_oem_data_rsp = cdf_mem_malloc(sizeof(tLimMlmOemDataRsp));
	if (NULL == mlm_oem_data_rsp) {
		lim_log(mac_ctx->hHdd, LOGP, FL
			("memory allocation for mlm_oem_data_rsp"));
		return;
	}

	if (NULL != mac_ctx->lim.gpLimMlmOemDataReq) {
		cdf_mem_free(mac_ctx->lim.gpLimMlmOemDataReq);
		mac_ctx->lim.gpLimMlmOemDataReq = NULL;
	}

	lim_post_sme_message(mac_ctx, LIM_MLM_OEM_DATA_CNF,
			     (uint32_t *) mlm_oem_data_rsp);

	return;
}
#endif /* FEATURE_OEM_DATA_SUPPORT */

/**
 * mlm_add_sta() - MLM add sta
 * @mac_ctx: global MAC context
 * @sta_param: Add sta params
 * @bssid: BSSID
 * @ht_capable: HT capability
 * @session_entry: PE session entry
 *
 * This function is called to update station parameters
 *
 * Return: None
 */
static void mlm_add_sta(tpAniSirGlobal mac_ctx, tpAddStaParams sta_param,
		uint8_t *bssid, uint8_t ht_capable, tpPESession session_entry)
{
	uint32_t val;
	uint32_t self_dot11mode = 0;

	wlan_cfg_get_int(mac_ctx, WNI_CFG_DOT11_MODE, &self_dot11mode);
	sta_param->staType = STA_ENTRY_SELF; /* Identifying self */

	cdf_mem_copy(sta_param->bssId, bssid, sizeof(tSirMacAddr));
	cdf_mem_copy(sta_param->staMac, session_entry->selfMacAddr,
		     sizeof(tSirMacAddr));

	/* Configuration related parameters to be changed to support BT-AMP */

	if (eSIR_SUCCESS != wlan_cfg_get_int(mac_ctx, WNI_CFG_LISTEN_INTERVAL,
					     &val))
		lim_log(mac_ctx, LOGP, FL("Couldn't get LISTEN_INTERVAL"));
	sta_param->listenInterval = (uint16_t) val;

	if (eSIR_SUCCESS != wlan_cfg_get_int(mac_ctx, WNI_CFG_SHORT_PREAMBLE,
					     &val))
		lim_log(mac_ctx, LOGP, FL("Couldn't get SHORT_PREAMBLE"));
	sta_param->shortPreambleSupported = (uint8_t) val;

	sta_param->assocId = 0;      /* Is SMAC OK with this? */
	sta_param->wmmEnabled = 0;
	sta_param->uAPSD = 0;
	sta_param->maxSPLen = 0;
	sta_param->us32MaxAmpduDuration = 0;
	sta_param->maxAmpduSize = 0; /* 0: 8k, 1: 16k,2: 32k,3: 64k, 4:128k */

	/* For Self STA get the LDPC capability from config.ini */
	sta_param->htLdpcCapable =
		(session_entry->txLdpcIniFeatureEnabled & 0x01);
	sta_param->vhtLdpcCapable =
		((session_entry->txLdpcIniFeatureEnabled >> 1) & 0x01);

	if (IS_DOT11_MODE_HT(session_entry->dot11mode)) {
		sta_param->htCapable = ht_capable;
		sta_param->greenFieldCapable =
			lim_get_ht_capability(mac_ctx, eHT_GREENFIELD,
					      session_entry);
		sta_param->ch_width =
			lim_get_ht_capability(mac_ctx,
				eHT_SUPPORTED_CHANNEL_WIDTH_SET, session_entry);
		sta_param->mimoPS =
			(tSirMacHTMIMOPowerSaveState)lim_get_ht_capability(
				mac_ctx, eHT_MIMO_POWER_SAVE, session_entry);
		sta_param->rifsMode =
			lim_get_ht_capability(mac_ctx, eHT_RIFS_MODE,
					      session_entry);
		sta_param->lsigTxopProtection =
			lim_get_ht_capability(mac_ctx, eHT_LSIG_TXOP_PROTECTION,
					      session_entry);
		sta_param->maxAmpduDensity =
			lim_get_ht_capability(mac_ctx, eHT_MPDU_DENSITY,
					      session_entry);
		sta_param->maxAmsduSize =
			lim_get_ht_capability(mac_ctx, eHT_MAX_AMSDU_LENGTH,
					      session_entry);
		sta_param->fDsssCckMode40Mhz =
			lim_get_ht_capability(mac_ctx, eHT_DSSS_CCK_MODE_40MHZ,
					      session_entry);
		sta_param->fShortGI20Mhz =
			lim_get_ht_capability(mac_ctx, eHT_SHORT_GI_20MHZ,
					      session_entry);
		sta_param->fShortGI40Mhz =
			lim_get_ht_capability(mac_ctx, eHT_SHORT_GI_40MHZ,
					      session_entry);
	}
	if (session_entry->vhtCapability) {
		sta_param->vhtCapable = true;
		sta_param->vhtTxBFCapable =
				session_entry->txBFIniFeatureEnabled;
		sta_param->vhtTxMUBformeeCapable = session_entry->txMuBformee;
		sta_param->enable_su_tx_bformer =
				session_entry->enable_su_tx_bformer;
	}
	/*
	 * Since this is Self-STA, need to populate Self MAX_AMPDU_SIZE
	 * capabilities
	 */
	if (IS_DOT11_MODE_VHT(self_dot11mode)) {
		val = 0;        /* Default 8K AMPDU size */
		if (eSIR_SUCCESS != wlan_cfg_get_int(mac_ctx,
					WNI_CFG_VHT_AMPDU_LEN_EXPONENT, &val))
			lim_log(mac_ctx, LOGE, FL
			    ("Couldn't get WNI_CFG_VHT_AMPDU_LEN_EXPONENT"));
		sta_param->maxAmpduSize = (uint8_t) val;
	}
	sta_param->enableVhtpAid = session_entry->enableVhtpAid;
	sta_param->enableAmpduPs = session_entry->enableAmpduPs;
	sta_param->enableHtSmps = session_entry->enableHtSmps;
	sta_param->htSmpsconfig = session_entry->htSmpsvalue;

#ifdef WLAN_FEATURE_11AC
	lim_populate_own_rate_set(mac_ctx, &sta_param->supportedRates, NULL,
				  false, session_entry, NULL);
#else
	lim_populate_own_rate_set(mac_ctx, &sta_param->supportedRates, NULL,
				  false, session_entry);
#endif
	lim_fill_supported_rates_info(mac_ctx, NULL, &sta_param->supportedRates,
				      session_entry);

	lim_log(mac_ctx, LOGE, FL(
		"GF: %d, ChnlWidth: %d, MimoPS: %d, lsigTXOP: %d, dsssCCK: %d,"
		" SGI20: %d, SGI40%d"), sta_param->greenFieldCapable,
		sta_param->ch_width, sta_param->mimoPS,
		sta_param->lsigTxopProtection, sta_param->fDsssCckMode40Mhz,
		sta_param->fShortGI20Mhz, sta_param->fShortGI40Mhz);

	if (CDF_P2P_GO_MODE == session_entry->pePersona)
		sta_param->p2pCapableSta = 1;
}

/**
 * lim_mlm_add_bss() - HAL interface for WMA_ADD_BSS_REQ
 * @mac_ctx: global MAC context
 * @mlm_start_req: MLM start request
 * @session: PE session entry
 *
 * Package WMA_ADD_BSS_REQ to HAL, in order to start a BSS
 *
 * Return: eSIR_SME_SUCCESS on success, other error codes otherwise
 */
tSirResultCodes
lim_mlm_add_bss(tpAniSirGlobal mac_ctx,
		tLimMlmStartReq *mlm_start_req, tpPESession session)
{
	tSirMsgQ msg_buf;
	tpAddBssParams addbss_param = NULL;
	uint32_t retcode;

	/* Package WMA_ADD_BSS_REQ message parameters */
	addbss_param = cdf_mem_malloc(sizeof(tAddBssParams));
	if (NULL == addbss_param) {
		lim_log(mac_ctx, LOGE,
			FL("Unable to allocate memory during ADD_BSS"));
		/* Respond to SME with LIM_MLM_START_CNF */
		return eSIR_MEM_ALLOC_FAILED;
	}

	cdf_mem_set(addbss_param, sizeof(tAddBssParams), 0);
	/* Fill in tAddBssParams members */
	cdf_mem_copy(addbss_param->bssId, mlm_start_req->bssId,
		     sizeof(tSirMacAddr));

	/* Fill in tAddBssParams selfMacAddr */
	cdf_mem_copy(addbss_param->selfMacAddr,
		     session->selfMacAddr, sizeof(tSirMacAddr));

	addbss_param->bssType = mlm_start_req->bssType;
	if ((mlm_start_req->bssType == eSIR_IBSS_MODE) ||
	    (mlm_start_req->bssType == eSIR_BTAMP_AP_MODE) ||
	    (mlm_start_req->bssType == eSIR_BTAMP_STA_MODE)) {
		addbss_param->operMode = BSS_OPERATIONAL_MODE_STA;
	} else if (mlm_start_req->bssType == eSIR_INFRA_AP_MODE) {
		addbss_param->operMode = BSS_OPERATIONAL_MODE_AP;
	}

	addbss_param->shortSlotTimeSupported = session->shortSlotTimeSupported;
	addbss_param->beaconInterval = mlm_start_req->beaconPeriod;
	addbss_param->dtimPeriod = mlm_start_req->dtimPeriod;
	addbss_param->wps_state = mlm_start_req->wps_state;
	addbss_param->cfParamSet.cfpCount = mlm_start_req->cfParamSet.cfpCount;
	addbss_param->cfParamSet.cfpPeriod =
		mlm_start_req->cfParamSet.cfpPeriod;
	addbss_param->cfParamSet.cfpMaxDuration =
		mlm_start_req->cfParamSet.cfpMaxDuration;
	addbss_param->cfParamSet.cfpDurRemaining =
		mlm_start_req->cfParamSet.cfpDurRemaining;

	addbss_param->rateSet.numRates = mlm_start_req->rateSet.numRates;
	cdf_mem_copy(addbss_param->rateSet.rate, mlm_start_req->rateSet.rate,
		     mlm_start_req->rateSet.numRates);

	addbss_param->nwType = mlm_start_req->nwType;
	addbss_param->htCapable = mlm_start_req->htCapable;
	addbss_param->vhtCapable = session->vhtCapability;
	addbss_param->ch_width = session->ch_width;
	addbss_param->ch_center_freq_seg0 =
		session->ch_center_freq_seg0;
	addbss_param->ch_center_freq_seg1 =
		session->ch_center_freq_seg1;
	addbss_param->htOperMode = mlm_start_req->htOperMode;
	addbss_param->dualCTSProtection = mlm_start_req->dualCTSProtection;
	addbss_param->txChannelWidthSet = mlm_start_req->txChannelWidthSet;

	addbss_param->currentOperChannel = mlm_start_req->channelNumber;
#ifdef WLAN_FEATURE_11W
	addbss_param->rmfEnabled = session->limRmfEnabled;
#endif

	/* Update PE sessionId */
	addbss_param->sessionId = mlm_start_req->sessionId;

	/* Send the SSID to HAL to enable SSID matching for IBSS */
	cdf_mem_copy(&(addbss_param->ssId.ssId),
		     mlm_start_req->ssId.ssId, mlm_start_req->ssId.length);
	addbss_param->ssId.length = mlm_start_req->ssId.length;
	addbss_param->bHiddenSSIDEn = mlm_start_req->ssidHidden;
	lim_log(mac_ctx, LOGE, FL("TRYING TO HIDE SSID %d"),
		addbss_param->bHiddenSSIDEn);
	/* CR309183. Disable Proxy Probe Rsp.  Host handles Probe Requests.  Until FW fixed. */
	addbss_param->bProxyProbeRespEn = 0;
	addbss_param->obssProtEnabled = mlm_start_req->obssProtEnabled;

#if defined WLAN_FEATURE_VOWIFI
	addbss_param->maxTxPower = session->maxTxPower;
#endif
	mlm_add_sta(mac_ctx, &addbss_param->staContext,
		    addbss_param->bssId, addbss_param->htCapable,
		    session);

	addbss_param->status = CDF_STATUS_SUCCESS;
	addbss_param->respReqd = 1;

	/* Set a new state for MLME */
	session->limMlmState = eLIM_MLM_WT_ADD_BSS_RSP_STATE;
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE, session->peSessionId,
			 session->limMlmState));

	/* pass on the session persona to hal */
	addbss_param->halPersona = session->pePersona;

	addbss_param->bSpectrumMgtEnabled = session->spectrumMgtEnabled ||
		lim_isconnected_on_dfs_channel(mlm_start_req->channelNumber);
#if defined WLAN_FEATURE_VOWIFI_11R
	addbss_param->extSetStaKeyParamValid = 0;
#endif

	addbss_param->dot11_mode = session->dot11mode;
	addbss_param->nss = session->nss;
	lim_log(mac_ctx, LOG2, FL("dot11_mode:%d nss value:%d"),
			addbss_param->dot11_mode, addbss_param->nss);

	msg_buf.type = WMA_ADD_BSS_REQ;
	msg_buf.reserved = 0;
	msg_buf.bodyptr = addbss_param;
	msg_buf.bodyval = 0;
	MTRACE(mac_trace_msg_tx(mac_ctx, session->peSessionId, msg_buf.type));

	lim_log(mac_ctx, LOGW, FL("Sending WMA_ADD_BSS_REQ..."));
	retcode = wma_post_ctrl_msg(mac_ctx, &msg_buf);
	if (eSIR_SUCCESS != retcode) {
		lim_log(mac_ctx, LOGE,
			FL("Posting ADD_BSS_REQ to HAL failed, reason=%X"),
			retcode);
		cdf_mem_free(addbss_param);
		return eSIR_SME_HAL_SEND_MESSAGE_FAIL;
	}

	return eSIR_SME_SUCCESS;
}

/**
 * lim_process_mlm_start_req() - process MLM_START_REQ message
 *
 * @mac_ctx: global MAC context
 * @msg_buf: Pointer to MLM message buffer
 *
 * This function is called to process MLM_START_REQ message
 * from SME
 * 1) MLME receives LIM_MLM_START_REQ from LIM
 * 2) MLME sends WMA_ADD_BSS_REQ to HAL
 * 3) MLME changes state to eLIM_MLM_WT_ADD_BSS_RSP_STATE
 * MLME now waits for HAL to send WMA_ADD_BSS_RSP
 *
 * Return: None
 */
static void lim_process_mlm_start_req(tpAniSirGlobal mac_ctx, uint32_t *msg_buf)
{
	tLimMlmStartReq *mlm_start_req;
	tLimMlmStartCnf mlm_start_cnf;
	tpPESession session = NULL;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	mlm_start_req = (tLimMlmStartReq *) msg_buf;
	session = pe_find_session_by_session_id(mac_ctx,
				mlm_start_req->sessionId);
	if (NULL == session) {
		lim_log(mac_ctx, LOGP,
			FL("Session Does not exist for given sessionID"));
		mlm_start_cnf.resultCode = eSIR_SME_REFUSED;
		goto end;
	}

	if (session->limMlmState != eLIM_MLM_IDLE_STATE) {
		/*
		 * Should not have received Start req in states other than idle.
		 * Return Start confirm with failure code.
		 */
		lim_log(mac_ctx, LOGE,
			FL("received unexpected MLM_START_REQ in state %X"),
			session->limMlmState);
		lim_print_mlm_state(mac_ctx, LOGE, session->limMlmState);
		mlm_start_cnf.resultCode =
				eSIR_SME_BSS_ALREADY_STARTED_OR_JOINED;
		goto end;
	}

	mlm_start_cnf.resultCode =
		lim_mlm_add_bss(mac_ctx, mlm_start_req, session);

end:
	/* Update PE session Id */
	mlm_start_cnf.sessionId = mlm_start_req->sessionId;

	/* Free up buffer allocated for LimMlmScanReq */
	cdf_mem_free(msg_buf);

	/*
	 * Respond immediately to LIM, only if MLME has not been
	 * successfully able to send WMA_ADD_BSS_REQ to HAL.
	 * Else, LIM_MLM_START_CNF will be sent after receiving
	 * WMA_ADD_BSS_RSP from HAL
	 */
	if (eSIR_SME_SUCCESS != mlm_start_cnf.resultCode)
		lim_post_sme_message(mac_ctx, LIM_MLM_START_CNF,
				     (uint32_t *) &mlm_start_cnf);
}

#ifdef FEATURE_OEM_DATA_SUPPORT
/**
 * lim_process_mlm_oem_data_req() - process MLM OEM_DATA_REQ message
 * @mac_ctx: global MAC context
 * @msg_buf: MLM message buffer
 *
 * This function process MLM OEM_DATA_REQ message.
 *
 * Return: None
 */
static void lim_process_mlm_oem_data_req(tpAniSirGlobal mac_ctx,
					 uint32_t *msg_buf)
{
	tLimMlmOemDataRsp *mlm_oem_data_rsp;

	if (((mac_ctx->lim.gLimMlmState == eLIM_MLM_IDLE_STATE) ||
	     (mac_ctx->lim.gLimMlmState == eLIM_MLM_JOINED_STATE) ||
	     (mac_ctx->lim.gLimMlmState == eLIM_MLM_AUTHENTICATED_STATE) ||
	     (mac_ctx->lim.gLimMlmState == eLIM_MLM_BSS_STARTED_STATE) ||
	     (mac_ctx->lim.gLimMlmState == eLIM_MLM_LINK_ESTABLISHED_STATE))) {
		/*
		 * Hold onto the oem data request criteria
		 * Free gpLimMlmOemDataReq to avoid memory leak due to
		 * second OEM data request
		 */
		if (mac_ctx->lim.gpLimMlmOemDataReq) {
			cdf_mem_free(mac_ctx->lim.gpLimMlmOemDataReq);
			mac_ctx->lim.gpLimMlmOemDataReq = NULL;
		}

		mac_ctx->lim.gpLimMlmOemDataReq = (tLimMlmOemDataReq *) msg_buf;
		mac_ctx->lim.gLimPrevMlmState = mac_ctx->lim.gLimMlmState;

		lim_log(mac_ctx, LOG2, FL("Calling lim_send_hal_oem_data_req"));
		lim_send_hal_oem_data_req(mac_ctx);
	} else {
		/* Should not have received oem data req in other states */
		lim_log(mac_ctx, LOGW, FL
			("unexpected LIM_MLM_OEM_DATA_REQ in invalid state %X"),
			mac_ctx->lim.gLimMlmState);
		lim_print_mlm_state(mac_ctx, LOGW, mac_ctx->lim.gLimMlmState);

		/* Free up buffer allocated */
		cdf_mem_free(msg_buf);

		/* Return Meas confirm with INVALID_PARAMETERS */
		mlm_oem_data_rsp = cdf_mem_malloc(sizeof(tLimMlmOemDataRsp));
		if (mlm_oem_data_rsp != NULL) {
			lim_post_sme_message(mac_ctx, LIM_MLM_OEM_DATA_CNF,
					     (uint32_t *) mlm_oem_data_rsp);
			cdf_mem_free(mlm_oem_data_rsp);
		} else {
			lim_log(mac_ctx, LOGP, FL
			    ("Could not allocate memory for mlm_oem_data_rsp"));
			return;
		}
	}

	return;
}
#endif /* FEATURE_OEM_DATA_SUPPORT */
/**
 * lim_post_join_set_link_state_callback()- registered callback to perform post
 * peer creation operations
 *
 * @mac: pointer to global mac structure
 * @callback_arg: registered callback argument
 * @status: peer creation status
 *
 * this is registered callback function during association to perform
 * post peer creation operation based on the peer creation status
 *
 * Return: none
 */
void lim_post_join_set_link_state_callback(tpAniSirGlobal mac,
		void *callback_arg, bool status)
{
	uint8_t chan_num, sec_chan_offset;
	tpPESession session_entry = (tpPESession) callback_arg;
	tLimMlmJoinCnf mlm_join_cnf;

	lim_log(mac, LOG1, FL("Sessionid %d set link state(%d) cb status:%d"),
			session_entry->peSessionId, session_entry->limMlmState,
			status);

	if (!status) {
		lim_log(mac, LOGE,
			FL("failed to find pe session for session id:%d"),
			session_entry->peSessionId);
		goto failure;
	}

	chan_num = session_entry->currentOperChannel;
	sec_chan_offset = session_entry->htSecondaryChannelOffset;
	/*
	 * store the channel switch session_entry in the lim
	 * global variable
	 */
	session_entry->channelChangeReasonCode =
			 LIM_SWITCH_CHANNEL_JOIN;
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_ESE) || \
	 defined(FEATURE_WLAN_LFR)
	session_entry->pLimMlmReassocRetryReq = NULL;
#endif
	lim_log(mac, LOGE,
		FL("[lim_process_mlm_join_req]: suspend link success(%d) "
		"on sessionid: %d setting channel to: %d with ch_width :%d "
		"and maxtxPower: %d"), status, session_entry->peSessionId,
		session_entry->currentOperChannel,
		session_entry->ch_width,
		session_entry->maxTxPower);
	lim_set_channel(mac, session_entry->currentOperChannel,
		session_entry->ch_center_freq_seg0,
		session_entry->ch_center_freq_seg1,
		session_entry->ch_width,
		session_entry->maxTxPower,
		session_entry->peSessionId);
	return;

failure:
	MTRACE(mac_trace(mac, TRACE_CODE_MLM_STATE, session_entry->peSessionId,
			 session_entry->limMlmState));
	session_entry->limMlmState = eLIM_MLM_IDLE_STATE;
	mlm_join_cnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
	mlm_join_cnf.sessionId = session_entry->peSessionId;
	mlm_join_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
	lim_post_sme_message(mac, LIM_MLM_JOIN_CNF, (uint32_t *) &mlm_join_cnf);
}

/**
 * lim_process_mlm_post_join_suspend_link() - This function is called after the
 * suspend link while joining off channel.
 *
 * @mac_ctx:    Pointer to Global MAC structure
 * @status:  status of suspend link.
 * @ctx:     passed while calling suspend link(session)
 *
 * This function does following:
 *   Check for suspend state.
 *   If success, proceed with setting link state to recieve the
 *   probe response/beacon from intended AP.
 *   Switch to the APs channel.
 *   On an error case, send the MLM_JOIN_CNF with error status.
 *
 * @Return None
 */
static void
lim_process_mlm_post_join_suspend_link(tpAniSirGlobal mac_ctx,
				       CDF_STATUS status,
				       uint32_t *ctx)
{
	tLimMlmJoinCnf mlm_join_cnf;
	tpPESession session = (tpPESession) ctx;
	tSirLinkState lnk_state;

	if (CDF_STATUS_SUCCESS != status) {
		lim_log(mac_ctx, LOGE,
			FL("Sessionid %d Suspend link(NOTIFY_BSS) failed. Still proceeding with join"),
			session->peSessionId);
	}
	lim_deactivate_and_change_timer(mac_ctx, eLIM_JOIN_FAIL_TIMER);

	/* assign appropriate sessionId to the timer object */
	mac_ctx->lim.limTimers.gLimJoinFailureTimer.sessionId =
		session->peSessionId;

	lnk_state = (LIM_IS_BT_AMP_STA_ROLE(session) ?
		eSIR_LINK_BTAMP_PREASSOC_STATE : eSIR_LINK_PREASSOC_STATE);
	lim_log(mac_ctx, LOG1, FL("[lim_process_mlm_join_req]: lnk_state:%d"),
		lnk_state);

	if (lim_set_link_state(mac_ctx, lnk_state,
			session->pLimMlmJoinReq->bssDescription.bssId,
			session->selfMacAddr,
			lim_post_join_set_link_state_callback,
			session) != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGE,
			FL("SessionId:%d lim_set_link_state to eSIR_LINK_PREASSOC_STATE Failed!!"),
			session->peSessionId);
		lim_print_mac_addr(mac_ctx,
			session->pLimMlmJoinReq->bssDescription.bssId, LOGE);
		mlm_join_cnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
		session->limMlmState = eLIM_MLM_IDLE_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
				 session->peSessionId, session->limMlmState));
		goto error;
	}

	return;
error:
	mlm_join_cnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
	mlm_join_cnf.sessionId = session->peSessionId;
	mlm_join_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
	lim_post_sme_message(mac_ctx, LIM_MLM_JOIN_CNF,
			     (uint32_t *) &mlm_join_cnf);
}

/**
 * lim_process_mlm_join_req() - process mlm join request.
 *
 * @mac_ctx:    Pointer to Global MAC structure
 * @msg:        Pointer to the MLM message buffer
 *
 * This function is called to process MLM_JOIN_REQ message
 * from SME. It does following:
 * 1) Initialize LIM, HAL, DPH
 * 2) Configure the BSS for which the JOIN REQ was received
 *   a) Send WMA_ADD_BSS_REQ to HAL -
 *   This will identify the BSS that we are interested in
 *   --AND--
 *   Add a STA entry for the AP (in a STA context)
 *   b) Wait for WMA_ADD_BSS_RSP
 *   c) Send WMA_ADD_STA_REQ to HAL
 *   This will add the "local STA" entry to the STA table
 * 3) Continue as before, i.e,
 *   a) Send a PROBE REQ
 *   b) Wait for PROBE RSP/BEACON containing the SSID that
 *   we are interested in
 *   c) Then start an AUTH seq
 *   d) Followed by the ASSOC seq
 *
 * @Return: None
 */
static void lim_process_mlm_join_req(tpAniSirGlobal mac_ctx, uint32_t *msg)
{
	tLimMlmJoinCnf mlmjoin_cnf;
	uint8_t sessionid;
	tpPESession session;

	sessionid = ((tpLimMlmJoinReq) msg)->sessionId;

	session = pe_find_session_by_session_id(mac_ctx, sessionid);
	if (NULL == session) {
		lim_log(mac_ctx, LOGE, FL("SessionId:%d does not exist"),
			sessionid);
		goto error;
	}

	if ((!LIM_IS_AP_ROLE(session) &&
	     !LIM_IS_BT_AMP_AP_ROLE(session)) &&
	     ((session->limMlmState == eLIM_MLM_IDLE_STATE) ||
	     (session->limMlmState == eLIM_MLM_JOINED_STATE)) &&
	     (SIR_MAC_GET_ESS
		(((tpLimMlmJoinReq) msg)->bssDescription.capabilityInfo) !=
		SIR_MAC_GET_IBSS(((tpLimMlmJoinReq) msg)->bssDescription.
			capabilityInfo))) {
		/* Hold onto Join request parameters */

		session->pLimMlmJoinReq = (tpLimMlmJoinReq) msg;
		if (is_lim_session_off_channel(mac_ctx, sessionid)) {
			lim_log(mac_ctx, LOG1,
				"SessionId:%d LimSession is on OffChannel",
				sessionid);
			/* suspend link */
			lim_log(mac_ctx, LOG1,
				FL("Suspend link, sessionid %d is off channel"),
				sessionid);
			if (lim_is_link_suspended(mac_ctx)) {
				lim_log(mac_ctx, LOGE, FL(
					"link is already suspended, session %d"
					), sessionid);
				goto error;
			}
			lim_process_mlm_post_join_suspend_link(mac_ctx,
				CDF_STATUS_SUCCESS, (uint32_t *)session);
		} else {
			lim_log(mac_ctx, LOG1, FL("No need to Suspend link"));
			 /*
			  * No need to Suspend link as LimSession is not
			  * off channel, calling
			  * lim_process_mlm_post_join_suspend_link with
			  * status as SUCCESS.
			  */
			lim_log(mac_ctx, LOG1,
				FL("SessionId:%d Join req on current chan"),
				sessionid);
			lim_process_mlm_post_join_suspend_link(mac_ctx,
				CDF_STATUS_SUCCESS, (uint32_t *)session);
		}
		return;
	} else {
		/**
		 * Should not have received JOIN req in states other than
		 * Idle state or on AP.
		 * Return join confirm with invalid parameters code.
		 */
		lim_log(mac_ctx, LOGE,
			FL("Session:%d Unexpected Join req, role %d state %X"),
			session->peSessionId, GET_LIM_SYSTEM_ROLE(session),
			session->limMlmState);
		lim_print_mlm_state(mac_ctx, LOGE, session->limMlmState);
	}

error:
	cdf_mem_free(msg);
	if (session != NULL)
		session->pLimMlmJoinReq = NULL;
	mlmjoin_cnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
	mlmjoin_cnf.sessionId = sessionid;
	mlmjoin_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
	lim_post_sme_message(mac_ctx, LIM_MLM_JOIN_CNF,
		(uint32_t *)&mlmjoin_cnf);

}

/**
 * lim_is_auth_req_expected() - check if auth request is expected
 *
 * @mac_ctx: global MAC context
 * @session: PE session entry
 *
 * This function is called by lim_process_mlm_auth_req to check
 * if auth request is expected.
 *
 * Return: true if expected and false otherwise
 */
static bool lim_is_auth_req_expected(tpAniSirGlobal mac_ctx,
				     tpPESession session)
{
	bool flag = false;

	/*
	 * Expect Auth request only when:
	 * 1. STA joined/associated with a BSS or
	 * 2. STA is in IBSS mode
	 * and STA is going to authenticate with a unicast
	 * address and requested authentication algorithm is
	 * supported.
	 */

	flag = ((((LIM_IS_STA_ROLE(session) ||
		   LIM_IS_BT_AMP_STA_ROLE(session)) &&
		 ((session->limMlmState == eLIM_MLM_JOINED_STATE) ||
		  (session->limMlmState ==
					eLIM_MLM_LINK_ESTABLISHED_STATE))) ||
		  (LIM_IS_IBSS_ROLE(session) &&
		  (session->limMlmState ==
					eLIM_MLM_BSS_STARTED_STATE))) &&
		(!lim_is_group_addr(mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr))
		 && lim_is_auth_algo_supported(mac_ctx,
			mac_ctx->lim.gpLimMlmAuthReq->authType, session));

	return flag;
}

/**
 * lim_is_preauth_ctx_exisits() - check if preauth context exists
 *
 * @mac_ctx:          global MAC context
 * @session:          PE session entry
 * @preauth_node_ptr: pointer to preauth node pointer
 *
 * This function is called by lim_process_mlm_auth_req to check
 * if preauth context already exists
 *
 * Return: true if exists and false otherwise
 */
static bool lim_is_preauth_ctx_exists(tpAniSirGlobal mac_ctx,
				      tpPESession session,
				      struct tLimPreAuthNode **preauth_node_ptr)
{
	bool fl = false;
	struct tLimPreAuthNode *preauth_node;
	tpDphHashNode stads;
	tSirMacAddr curr_bssid;

	preauth_node = *preauth_node_ptr;
	sir_copy_mac_addr(curr_bssid, session->bssId);
	stads = dph_get_hash_entry(mac_ctx, DPH_STA_HASH_INDEX_PEER,
				   &session->dph.dphHashTable);
	preauth_node = lim_search_pre_auth_list(mac_ctx,
				mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr);

	fl = (((LIM_IS_STA_ROLE(session) || LIM_IS_BT_AMP_STA_ROLE(session)) &&
	       (session->limMlmState == eLIM_MLM_LINK_ESTABLISHED_STATE) &&
	      ((stads != NULL) &&
	       (mac_ctx->lim.gpLimMlmAuthReq->authType ==
			stads->mlmStaContext.authType)) &&
	       (cdf_mem_compare(mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr,
			curr_bssid, sizeof(tSirMacAddr)))) ||
	      ((preauth_node != NULL) &&
	       (preauth_node->authType ==
			mac_ctx->lim.gpLimMlmAuthReq->authType)));

	return fl;
}

/**
 * lim_process_mlm_auth_req() - process lim auth request
 *
 * @mac_ctx:   global MAC context
 * @msg:       MLM auth request message
 *
 * This function is called to process MLM_AUTH_REQ message from SME
 *
 * @Return: None
 */
static void lim_process_mlm_auth_req(tpAniSirGlobal mac_ctx, uint32_t *msg)
{
	uint32_t num_preauth_ctx;
	tSirMacAddr curr_bssid;
	tSirMacAuthFrameBody auth_frame_body;
	tLimMlmAuthCnf mlm_auth_cnf;
	struct tLimPreAuthNode *preauth_node = NULL;
	uint8_t session_id;
	tpPESession session;

	if (msg == NULL) {
		lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	mac_ctx->lim.gpLimMlmAuthReq = (tLimMlmAuthReq *) msg;
	session_id = mac_ctx->lim.gpLimMlmAuthReq->sessionId;
	session = pe_find_session_by_session_id(mac_ctx, session_id);
	if (NULL == session) {
		lim_log(mac_ctx, LOGP, FL("SessionId:%d does not exist"),
			session_id);
		return;
	}

	lim_log(mac_ctx, LOG1, FL("Process Auth Req sessionID %d Systemrole %d"
			       "mlmstate %d from: " MAC_ADDRESS_STR
			       " with authtype %d"), session_id,
		GET_LIM_SYSTEM_ROLE(session), session->limMlmState,
		MAC_ADDR_ARRAY(mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr),
		mac_ctx->lim.gpLimMlmAuthReq->authType);

	sir_copy_mac_addr(curr_bssid, session->bssId);

	if (!lim_is_auth_req_expected(mac_ctx, session)) {
		/*
		 * Unexpected auth request.
		 * Return Auth confirm with Invalid parameters code.
		 */
		mlm_auth_cnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		goto end;
	}

	/*
	 * This is a request for pre-authentication. Check if there exists
	 * context already for the requested peer OR
	 * if this request is for the AP we're currently associated with.
	 * If yes, return auth confirm immediately when
	 * requested auth type is same as the one used before.
	 */
	if (lim_is_preauth_ctx_exists(mac_ctx, session, &preauth_node)) {
		lim_log(mac_ctx, LOG2,
		    FL("Already have pre-auth context with peer: "
		    MAC_ADDRESS_STR),
		    MAC_ADDR_ARRAY(mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr));
		mlm_auth_cnf.resultCode = (tSirResultCodes)
						eSIR_MAC_SUCCESS_STATUS;
		goto end;
	} else {
		if (wlan_cfg_get_int(mac_ctx, WNI_CFG_MAX_NUM_PRE_AUTH,
			(uint32_t *) &num_preauth_ctx) != eSIR_SUCCESS)
			lim_log(mac_ctx, LOGP,
			    FL("Could not retrieve NumPreAuthLimit from CFG"));

		if (mac_ctx->lim.gLimNumPreAuthContexts == num_preauth_ctx) {
			lim_log(mac_ctx, LOGW,
				FL("Number of pre-auth reached max limit"));
			/* Return Auth confirm with reject code */
			mlm_auth_cnf.resultCode =
				eSIR_SME_MAX_NUM_OF_PRE_AUTH_REACHED;
			goto end;
		}
	}

	/* Delete pre-auth node if exists */
	if (preauth_node)
		lim_delete_pre_auth_node(mac_ctx,
			 mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr);

	session->limPrevMlmState = session->limMlmState;
	session->limMlmState = eLIM_MLM_WT_AUTH_FRAME2_STATE;
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE, session->peSessionId,
		       session->limMlmState));

	/* Prepare & send Authentication frame */
	auth_frame_body.authAlgoNumber =
		(uint8_t) mac_ctx->lim.gpLimMlmAuthReq->authType;
	auth_frame_body.authTransactionSeqNumber = SIR_MAC_AUTH_FRAME_1;
	auth_frame_body.authStatusCode = 0;
#ifdef FEATURE_WLAN_DIAG_SUPPORT
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_AUTH_START_EVENT, session,
			      eSIR_SUCCESS, auth_frame_body.authStatusCode);
#endif

	lim_send_auth_mgmt_frame(mac_ctx,
		&auth_frame_body, mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr,
		LIM_NO_WEP_IN_FC, session);

	/* assign appropriate session_id to the timer object */
	mac_ctx->lim.limTimers.gLimAuthFailureTimer.sessionId = session_id;

	/* Activate Auth failure timer */
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_TIMER_ACTIVATE,
			 session->peSessionId, eLIM_AUTH_FAIL_TIMER));
	if (tx_timer_activate(&mac_ctx->lim.limTimers.gLimAuthFailureTimer)
	    != TX_SUCCESS) {
		/* Could not start Auth failure timer. */
		lim_log(mac_ctx, LOGP,
			FL("could not start Auth failure timer"));
		/* Cleanup as if auth timer expired */
		lim_process_auth_failure_timeout(mac_ctx);
	}
	return;
end:
	cdf_mem_copy((uint8_t *) &mlm_auth_cnf.peerMacAddr,
		     (uint8_t *) &mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr,
		     sizeof(tSirMacAddr));

	mlm_auth_cnf.authType = mac_ctx->lim.gpLimMlmAuthReq->authType;
	mlm_auth_cnf.sessionId = session_id;

	cdf_mem_free(mac_ctx->lim.gpLimMlmAuthReq);
	mac_ctx->lim.gpLimMlmAuthReq = NULL;
	lim_log(mac_ctx, LOG1, "SessionId:%d LimPostSme LIM_MLM_AUTH_CNF ",
		session_id);
	lim_post_sme_message(mac_ctx, LIM_MLM_AUTH_CNF,
			     (uint32_t *) &mlm_auth_cnf);
}

/**
 * lim_process_mlm_assoc_req() - This function is called to process
 * MLM_ASSOC_REQ message from SME
 *
 * @mac_ctx:       Pointer to Global MAC structure
 * @msg_buf:       A pointer to the MLM message buffer
 *
 * This function is called to process MLM_ASSOC_REQ message from SME
 *
 * @Return None
 */

static void lim_process_mlm_assoc_req(tpAniSirGlobal mac_ctx, uint32_t *msg_buf)
{
	tSirMacAddr curr_bssId;
	tLimMlmAssocReq *mlm_assoc_req;
	tLimMlmAssocCnf mlm_assoc_cnf;
	tpPESession session_entry;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	mlm_assoc_req = (tLimMlmAssocReq *) msg_buf;
	session_entry = pe_find_session_by_session_id(mac_ctx,
						      mlm_assoc_req->sessionId);
	if (session_entry == NULL) {
		lim_log(mac_ctx, LOGP,
			FL("SessionId:%d Session Does not exist"),
			mlm_assoc_req->sessionId);
		cdf_mem_free(mlm_assoc_req);
		return;
	}

	sir_copy_mac_addr(curr_bssId, session_entry->bssId);

	if (!((!LIM_IS_AP_ROLE(session_entry) &&
	    !LIM_IS_BT_AMP_AP_ROLE(session_entry)) &&
		(session_entry->limMlmState == eLIM_MLM_AUTHENTICATED_STATE ||
		 session_entry->limMlmState == eLIM_MLM_JOINED_STATE) &&
		(cdf_mem_compare(mlm_assoc_req->peerMacAddr,
		 curr_bssId, sizeof(tSirMacAddr))))) {
		/*
		 * Received Association request either in invalid state
		 * or to a peer MAC entity whose address is different
		 * from one that STA is currently joined with or on AP.
		 * Return Assoc confirm with Invalid parameters code.
		 */
		lim_log(mac_ctx, LOGW,
			FL("received unexpected MLM_ASSOC_CNF in state %X for role=%d, MAC addr= "
			   MAC_ADDRESS_STR), session_entry->limMlmState,
			GET_LIM_SYSTEM_ROLE(session_entry),
			MAC_ADDR_ARRAY(mlm_assoc_req->peerMacAddr));
		lim_print_mlm_state(mac_ctx, LOGW, session_entry->limMlmState);
		mlm_assoc_cnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		mlm_assoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
		goto end;
	}

	/* map the session entry pointer to the AssocFailureTimer */
	mac_ctx->lim.limTimers.gLimAssocFailureTimer.sessionId =
		mlm_assoc_req->sessionId;
#ifdef WLAN_FEATURE_11W
	/*
	 * Store current MLM state in case ASSOC response returns with
	 * TRY_AGAIN_LATER return code.
	 */
	if (session_entry->limRmfEnabled) {
		session_entry->pmfComebackTimerInfo.limPrevMlmState =
			session_entry->limPrevMlmState;
		session_entry->pmfComebackTimerInfo.limMlmState =
			session_entry->limMlmState;
	}
#endif /* WLAN_FEATURE_11W */

	session_entry->limPrevMlmState = session_entry->limMlmState;
	session_entry->limMlmState = eLIM_MLM_WT_ASSOC_RSP_STATE;
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
			 session_entry->peSessionId,
			 session_entry->limMlmState));
	lim_log(mac_ctx, LOG1, FL("SessionId:%d Sending Assoc_Req Frame"),
		session_entry->peSessionId);

	/* Prepare and send Association request frame */
	lim_send_assoc_req_mgmt_frame(mac_ctx, mlm_assoc_req, session_entry);

	/*
	 * Set the link state to postAssoc, so HW can start receiving frames
	 * from AP.
	 */
	if ((session_entry->bssType == eSIR_BTAMP_STA_MODE) ||
	    ((session_entry->bssType == eSIR_BTAMP_AP_MODE)
	     && LIM_IS_BT_AMP_STA_ROLE(session_entry))) {
		if (lim_set_link_state(mac_ctx, eSIR_LINK_BTAMP_POSTASSOC_STATE,
					curr_bssId, session_entry->selfMacAddr,
					NULL, NULL) != eSIR_SUCCESS)
			lim_log(mac_ctx, LOGE,
				FL("Failed to set the LinkState"));
	}
	/* Start association failure timer */
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_TIMER_ACTIVATE,
			 session_entry->peSessionId, eLIM_ASSOC_FAIL_TIMER));
	if (tx_timer_activate(&mac_ctx->lim.limTimers.gLimAssocFailureTimer)
	    != TX_SUCCESS) {
		lim_log(mac_ctx, LOGP,
			FL("SessionId:%d couldn't start Assoc failure timer"),
			session_entry->peSessionId);
		/* Cleanup as if assoc timer expired */
		lim_process_assoc_failure_timeout(mac_ctx, LIM_ASSOC);
	}

	return;
end:
	/* Update PE session Id */
	mlm_assoc_cnf.sessionId = mlm_assoc_req->sessionId;
	/* Free up buffer allocated for assocReq */
	cdf_mem_free(mlm_assoc_req);
	lim_post_sme_message(mac_ctx, LIM_MLM_ASSOC_CNF,
			     (uint32_t *) &mlm_assoc_cnf);
}

/**
 * lim_process_mlm_reassoc_req() - process mlm reassoc request.
 *
 * @mac_ctx:     pointer to Global MAC structure
 * @msg:  pointer to the MLM message buffer
 *
 * This function is called to process MLM_REASSOC_REQ message
 * from SME
 *
 * Return: None
 */
static void lim_process_mlm_reassoc_req(tpAniSirGlobal mac_ctx, uint32_t *msg)
{
	uint8_t channel, sec_ch_offset;
	struct tLimPreAuthNode *auth_node;
	tLimMlmReassocReq *reassoc_req;
	tLimMlmReassocCnf reassoc_cnf;
	tpPESession session;

	if (msg == NULL) {
		lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	reassoc_req = (tLimMlmReassocReq *) msg;
	session = pe_find_session_by_session_id(mac_ctx,
			reassoc_req->sessionId);
	if (NULL == session) {
		lim_log(mac_ctx, LOGE,
			FL("Session Does not exist for given sessionId %d"),
			reassoc_req->sessionId);
		cdf_mem_free(reassoc_req);
		return;
	}

	lim_log(mac_ctx, LOG1,
		FL("Process ReAssoc Req on sessionID %d Systemrole %d mlmstate %d from: " MAC_ADDRESS_STR),
		reassoc_req->sessionId, GET_LIM_SYSTEM_ROLE(session),
		session->limMlmState, MAC_ADDR_ARRAY(reassoc_req->peerMacAddr));

	if ((LIM_IS_AP_ROLE(session) ||
		LIM_IS_BT_AMP_AP_ROLE(session)) ||
		(session->limMlmState !=
		eLIM_MLM_LINK_ESTABLISHED_STATE)) {
		/*
		 * Received Reassoc request in invalid state or
		 * in AP role.Return Reassoc confirm with Invalid
		 * parameters code.
		 */

		lim_log(mac_ctx, LOGW,
			FL("received unexpected MLM_REASSOC_CNF in state %X for role=%d, MAC addr= " MAC_ADDRESS_STR),
			session->limMlmState, GET_LIM_SYSTEM_ROLE(session),
			MAC_ADDR_ARRAY(reassoc_req->peerMacAddr));
		lim_print_mlm_state(mac_ctx, LOGW, session->limMlmState);
		reassoc_cnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		reassoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
		goto end;
	}

	if (session->pLimMlmReassocReq)
		cdf_mem_free(session->pLimMlmReassocReq);

	/*
	 * Hold Re-Assoc request as part of Session, knock-out mac_ctx
	 * Hold onto Reassoc request parameters
	 */
	session->pLimMlmReassocReq = reassoc_req;

	/* See if we have pre-auth context with new AP */
	auth_node = lim_search_pre_auth_list(mac_ctx, session->limReAssocbssId);

	if (!auth_node && (!cdf_mem_compare(reassoc_req->peerMacAddr,
					    session->bssId,
					    sizeof(tSirMacAddr)))) {
		/*
		 * Either pre-auth context does not exist AND
		 * we are not reassociating with currently
		 * associated AP.
		 * Return Reassoc confirm with not authenticated
		 */
		reassoc_cnf.resultCode = eSIR_SME_STA_NOT_AUTHENTICATED;
		reassoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;

		goto end;
	}
	/* assign the sessionId to the timer object */
	mac_ctx->lim.limTimers.gLimReassocFailureTimer.sessionId =
		reassoc_req->sessionId;
	session->limPrevMlmState = session->limMlmState;
	session->limMlmState = eLIM_MLM_WT_REASSOC_RSP_STATE;
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE, session->peSessionId,
			 session->limMlmState));

	/* Derive channel from BSS description and store it at CFG. */
	channel = session->limReassocChannelId;
	sec_ch_offset = session->reAssocHtSecondaryChannelOffset;

	/* Apply previously set configuration at HW */
	lim_apply_configuration(mac_ctx, session);

	/* store the channel switch sessionEntry in the lim global var */
	session->channelChangeReasonCode =
		LIM_SWITCH_CHANNEL_REASSOC;

	/* Switch channel to the new Operating channel for Reassoc */
	lim_set_channel(mac_ctx, channel,
			session->ch_center_freq_seg0,
			session->ch_center_freq_seg1,
			session->ch_width,
			session->maxTxPower,
			session->peSessionId);

	return;
end:
	reassoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
	/* Update PE sessio Id */
	reassoc_cnf.sessionId = reassoc_req->sessionId;
	/* Free up buffer allocated for reassocReq */
	cdf_mem_free(reassoc_req);
	session->pLimReAssocReq = NULL;
	lim_post_sme_message(mac_ctx, LIM_MLM_REASSOC_CNF,
			     (uint32_t *) &reassoc_cnf);
}

/**
 * lim_process_mlm_disassoc_req_ntf() - process disassoc request notification
 *
 * @mac_ctx:        global MAC context
 * @suspend_status: suspend status
 * @msg:            mlm message buffer
 *
 * This function is used to process MLM disassoc notification
 *
 * Return: None
 */
static void
lim_process_mlm_disassoc_req_ntf(tpAniSirGlobal mac_ctx,
				 CDF_STATUS suspend_status, uint32_t *msg)
{
	uint16_t aid;
	tSirMacAddr curr_bssid;
	tpDphHashNode stads;
	tLimMlmDisassocReq *mlm_disassocreq;
	tLimMlmDisassocCnf mlm_disassoccnf;
	tpPESession session;
	extern bool send_disassoc_frame;
	tLimMlmStates mlm_state;
	tSirSmeDisassocRsp *sme_disassoc_rsp;

	if (CDF_STATUS_SUCCESS != suspend_status)
		lim_log(mac_ctx, LOGE, FL("Suspend Status is not success %X"),
			suspend_status);

	mlm_disassocreq = (tLimMlmDisassocReq *) msg;

	session = pe_find_session_by_session_id(mac_ctx,
				mlm_disassocreq->sessionId);
	if (NULL == session) {
		lim_log(mac_ctx, LOGE,
			FL("session does not exist for given sessionId %d"),
			mlm_disassocreq->sessionId);
		mlm_disassoccnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		goto end;
	}

	lim_log(mac_ctx, LOG1,
		FL("Process DisAssoc Req on sessionID %d Systemrole %d"
		   "mlmstate %d from: " MAC_ADDRESS_STR),
		mlm_disassocreq->sessionId, GET_LIM_SYSTEM_ROLE(session),
		session->limMlmState,
		MAC_ADDR_ARRAY(mlm_disassocreq->peerMacAddr));

	sir_copy_mac_addr(curr_bssid, session->bssId);

	switch (GET_LIM_SYSTEM_ROLE(session)) {
	case eLIM_STA_ROLE:
	case eLIM_BT_AMP_STA_ROLE:
		if (!cdf_mem_compare(mlm_disassocreq->peerMacAddr,
				     curr_bssid, sizeof(tSirMacAddr))) {
			lim_log(mac_ctx, LOGW,
			  FL("received MLM_DISASSOC_REQ with invalid BSS id"));
			lim_print_mac_addr(mac_ctx,
				mlm_disassocreq->peerMacAddr, LOGW);

			/*
			 * Disassociation response due to host triggered
			 * disassociation
			 */
			sme_disassoc_rsp =
				cdf_mem_malloc(sizeof(tSirSmeDisassocRsp));
			if (NULL == sme_disassoc_rsp) {
				lim_log(mac_ctx, LOGP,
					FL("memory allocation failed for disassoc rsp"));
				return;
			}

			lim_log(mac_ctx, LOG1,
				FL("send disassoc rsp with ret code %d for" MAC_ADDRESS_STR),
				eSIR_SME_DEAUTH_STATUS,
				MAC_ADDR_ARRAY(mlm_disassocreq->peerMacAddr));

			sme_disassoc_rsp->messageType = eWNI_SME_DISASSOC_RSP;
			sme_disassoc_rsp->length = sizeof(tSirSmeDisassocRsp);
			sme_disassoc_rsp->sessionId =
					mlm_disassocreq->sessionId;
			sme_disassoc_rsp->transactionId = 0;
			sme_disassoc_rsp->statusCode = eSIR_SME_DEAUTH_STATUS;

			cdf_mem_copy(sme_disassoc_rsp->peerMacAddr,
				mlm_disassocreq->peerMacAddr,
				sizeof(tSirMacAddr));
			msg = (uint32_t *)sme_disassoc_rsp;

			lim_send_sme_disassoc_deauth_ntf(mac_ctx,
					CDF_STATUS_SUCCESS, msg);
			return;

		}
		break;
	case eLIM_STA_IN_IBSS_ROLE:
		break;
	case eLIM_AP_ROLE:
	case eLIM_P2P_DEVICE_GO:
		if (true ==
			 mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running) {
			lim_log(mac_ctx, LOGE,
				FL("CAC timer is running, drop disassoc from going out"));
			mlm_disassoccnf.resultCode = eSIR_SME_SUCCESS;
			goto end;
		}
		break;
	default:
		break;
	} /* end switch (GET_LIM_SYSTEM_ROLE(session)) */

	/*
	 * Check if there exists a context for the peer entity
	 * to be disassociated with.
	 */
	stads = dph_lookup_hash_entry(mac_ctx, mlm_disassocreq->peerMacAddr,
				      &aid, &session->dph.dphHashTable);
	if (stads)
		mlm_state = stads->mlmStaContext.mlmState;

	if ((stads == NULL) ||
	    (stads &&
	     ((mlm_state != eLIM_MLM_LINK_ESTABLISHED_STATE) &&
	      (mlm_state != eLIM_MLM_WT_ASSOC_CNF_STATE) &&
	      (mlm_state != eLIM_MLM_ASSOCIATED_STATE)))) {
		/*
		 * Received LIM_MLM_DISASSOC_REQ for STA that does not
		 * have context or in some transit state.
		 */
		lim_log(mac_ctx, LOGW,
			FL("Invalid MLM_DISASSOC_REQ, Addr= " MAC_ADDRESS_STR),
			MAC_ADDR_ARRAY(mlm_disassocreq->peerMacAddr));
		if (stads != NULL)
			lim_log(mac_ctx, LOGE, FL("Sta MlmState : %d"),
				stads->mlmStaContext.mlmState);

		/* Prepare and Send LIM_MLM_DISASSOC_CNF */
		mlm_disassoccnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		goto end;
	}

	stads->mlmStaContext.disassocReason = (tSirMacReasonCodes)
					       mlm_disassocreq->reasonCode;
	stads->mlmStaContext.cleanupTrigger = mlm_disassocreq->disassocTrigger;

	/*
	 * Set state to mlm State to eLIM_MLM_WT_DEL_STA_RSP_STATE
	 * This is to address the issue of race condition between
	 * disconnect request from the HDD and deauth from AP
	 */

	stads->mlmStaContext.mlmState = eLIM_MLM_WT_DEL_STA_RSP_STATE;

	/* Send Disassociate frame to peer entity */
	if (send_disassoc_frame && (mlm_disassocreq->reasonCode !=
		eSIR_MAC_DISASSOC_DUE_TO_FTHANDOFF_REASON)) {
		mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDisassocReq =
			mlm_disassocreq;
		/*
		 * Set state to mlm State to eLIM_MLM_WT_DEL_STA_RSP_STATE
		 * This is to address the issue of race condition between
		 * disconnect request from the HDD and deauth from AP
		 */
		stads->mlmStaContext.mlmState = eLIM_MLM_WT_DEL_STA_RSP_STATE;

		lim_send_disassoc_mgmt_frame(mac_ctx,
			mlm_disassocreq->reasonCode,
			mlm_disassocreq->peerMacAddr, session, true);
		/*
		 * Abort Tx so that data frames won't be sent to the AP
		 * after sending Disassoc.
		 */
		if (LIM_IS_STA_ROLE(session))
			wma_tx_abort(session->smeSessionId);
	} else {
		/* Disassoc frame is not sent OTA */
		send_disassoc_frame = 1;
		/* Receive path cleanup with dummy packet */
		if (eSIR_SUCCESS !=
		    lim_cleanup_rx_path(mac_ctx, stads, session)) {
			mlm_disassoccnf.resultCode =
				eSIR_SME_RESOURCES_UNAVAILABLE;
			goto end;
		}
		/* Free up buffer allocated for mlmDisassocReq */
		cdf_mem_free(mlm_disassocreq);
	}

	return;

end:
	cdf_mem_copy((uint8_t *) &mlm_disassoccnf.peerMacAddr,
		     (uint8_t *) mlm_disassocreq->peerMacAddr,
		     sizeof(tSirMacAddr));
	mlm_disassoccnf.aid = mlm_disassocreq->aid;
	mlm_disassoccnf.disassocTrigger = mlm_disassocreq->disassocTrigger;

	/* Update PE session ID */
	mlm_disassoccnf.sessionId = mlm_disassocreq->sessionId;

	/* Free up buffer allocated for mlmDisassocReq */
	cdf_mem_free(mlm_disassocreq);

	lim_post_sme_message(mac_ctx, LIM_MLM_DISASSOC_CNF,
			     (uint32_t *) &mlm_disassoccnf);
}

/**
 * lim_check_disassoc_deauth_ack_pending() - check if deauth is pending
 *
 * @mac_ctx - global MAC context
 * @sta_mac - station MAC
 *
 * This function checks if diassociation or deauthentication is pending for
 * given station MAC address.
 *
 * Return: true if pending and false otherwise.
 */
bool lim_check_disassoc_deauth_ack_pending(tpAniSirGlobal mac_ctx,
					   uint8_t *sta_mac)
{
	tLimMlmDisassocReq *disassoc_req;
	tLimMlmDeauthReq *deauth_req;

	disassoc_req = mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDisassocReq;
	deauth_req = mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDeauthReq;
	if ((disassoc_req && (cdf_mem_compare((uint8_t *) sta_mac,
			      (uint8_t *) &disassoc_req->peerMacAddr,
			       sizeof(tSirMacAddr)))) ||
	    (deauth_req && (cdf_mem_compare((uint8_t *) sta_mac,
			      (uint8_t *) &deauth_req->peerMacAddr,
			       sizeof(tSirMacAddr))))) {
		PELOG1(lim_log(mac_ctx, LOG1,
			       FL("Disassoc/Deauth ack pending"));)
		return true;
	} else {
		PELOG1(lim_log(mac_ctx, LOG1,
			       FL("Disassoc/Deauth Ack not pending"));)
		return false;
	}
}

/*
 * lim_clean_up_disassoc_deauth_req() - cleans up pending disassoc or deauth req
 *
 * @mac_ctx:        mac_ctx
 * @sta_mac:        sta mac address
 * @clean_rx_path:  flag to indicate whether to cleanup rx path or not
 *
 * This function cleans up pending disassoc or deauth req
 *
 * Return: void
 */
void lim_clean_up_disassoc_deauth_req(tpAniSirGlobal mac_ctx,
				      uint8_t *sta_mac, bool clean_rx_path)
{
	tLimMlmDisassocReq *mlm_disassoc_req;
	tLimMlmDeauthReq *mlm_deauth_req;
	mlm_disassoc_req = mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDisassocReq;
	if (mlm_disassoc_req &&
	    (cdf_mem_compare((uint8_t *) sta_mac,
			     (uint8_t *) &mlm_disassoc_req->peerMacAddr,
			     sizeof(tSirMacAddr)))) {
		if (clean_rx_path) {
			lim_process_disassoc_ack_timeout(mac_ctx);
		} else {
			if (tx_timer_running(
				&mac_ctx->lim.limTimers.gLimDisassocAckTimer)) {
				lim_deactivate_and_change_timer(mac_ctx,
						eLIM_DISASSOC_ACK_TIMER);
			}
			cdf_mem_free(mlm_disassoc_req);
			mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDisassocReq =
				NULL;
		}
	}

	mlm_deauth_req = mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDeauthReq;
	if (mlm_deauth_req &&
	    (cdf_mem_compare((uint8_t *) sta_mac,
			     (uint8_t *) &mlm_deauth_req->peerMacAddr,
			     sizeof(tSirMacAddr)))) {
		if (clean_rx_path) {
			lim_process_deauth_ack_timeout(mac_ctx);
		} else {
			if (tx_timer_running(
				&mac_ctx->lim.limTimers.gLimDeauthAckTimer)) {
				lim_deactivate_and_change_timer(mac_ctx,
						eLIM_DEAUTH_ACK_TIMER);
			}
			cdf_mem_free(mlm_deauth_req);
			mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDeauthReq =
				NULL;
		}
	}
}

/*
 * lim_process_disassoc_ack_timeout() - wrapper function around
 * lim_send_disassoc_cnf
 *
 * @mac_ctx:        mac_ctx
 *
 * wrapper function around lim_send_disassoc_cnf
 *
 * Return: void
 */
void lim_process_disassoc_ack_timeout(tpAniSirGlobal mac_ctx)
{
	lim_log(mac_ctx, LOG1, FL(""));
	lim_send_disassoc_cnf(mac_ctx);
}

/**
 * lim_process_mlm_disassoc_req() - This function is called to process
 * MLM_DISASSOC_REQ message from SME
 *
 * @mac_ctx:      Pointer to Global MAC structure
 * @msg_buf:      A pointer to the MLM message buffer
 *
 * This function is called to process MLM_DISASSOC_REQ message from SME
 *
 * @Return: None
 */
static void
lim_process_mlm_disassoc_req(tpAniSirGlobal mac_ctx, uint32_t *msg_buf)
{
	tLimMlmDisassocReq *mlm_disassoc_req;
	tpPESession session;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("Buffer is Pointing to NULL"));
		return;
	}

	mlm_disassoc_req = (tLimMlmDisassocReq *) msg_buf;
	lim_log(mac_ctx, LOG1,
		FL("Process disassoc req, sessionID %d from: "MAC_ADDRESS_STR),
		mlm_disassoc_req->sessionId,
		MAC_ADDR_ARRAY(mlm_disassoc_req->peerMacAddr));

	session = pe_find_session_by_session_id(mac_ctx,
				mlm_disassoc_req->sessionId);
	if (NULL == session) {
		lim_log(mac_ctx, LOGE,
			FL("session does not exist for given sessionId %d"),
			mlm_disassoc_req->sessionId);
		return;
	}

	lim_process_mlm_disassoc_req_ntf(mac_ctx, CDF_STATUS_SUCCESS,
					 (uint32_t *) msg_buf);
}

/**
 * lim_process_mlm_deauth_req_ntf() - This function is process mlm deauth req
 * notification
 *
 * @mac_ctx:         Pointer to Global MAC structure
 * @suspend_status:  suspend status
 * @msg_buf:         A pointer to the MLM message buffer
 *
 * This function is process mlm deauth req notification
 *
 * @Return: None
 */
static void
lim_process_mlm_deauth_req_ntf(tpAniSirGlobal mac_ctx,
			       CDF_STATUS suspend_status, uint32_t *msg_buf)
{
	uint16_t aid;
	tSirMacAddr curr_bssId;
	tpDphHashNode sta_ds;
	struct tLimPreAuthNode *auth_node;
	tLimMlmDeauthReq *mlm_deauth_req;
	tLimMlmDeauthCnf mlm_deauth_cnf;
	tpPESession session;
	tSirSmeDeauthRsp *sme_deauth_rsp;

	if (CDF_STATUS_SUCCESS != suspend_status)
		lim_log(mac_ctx, LOGE, FL("Suspend Status is not success %X"),
			suspend_status);

	mlm_deauth_req = (tLimMlmDeauthReq *) msg_buf;
	session = pe_find_session_by_session_id(mac_ctx,
				mlm_deauth_req->sessionId);
	if (NULL == session) {
		lim_log(mac_ctx, LOGE,
			FL("session does not exist for given sessionId %d"),
			mlm_deauth_req->sessionId);
		cdf_mem_free(mlm_deauth_req);
		return;
	}
	lim_log(mac_ctx, LOG1, FL("Process Deauth Req on sessionID %d Systemrole %d"
			       "mlmstate %d from: " MAC_ADDRESS_STR),
		mlm_deauth_req->sessionId,
		GET_LIM_SYSTEM_ROLE(session),
		session->limMlmState,
		MAC_ADDR_ARRAY(mlm_deauth_req->peerMacAddr));
	sir_copy_mac_addr(curr_bssId, session->bssId);

	switch (GET_LIM_SYSTEM_ROLE(session)) {
	case eLIM_STA_ROLE:
	case eLIM_BT_AMP_STA_ROLE:
		switch (session->limMlmState) {
		case eLIM_MLM_IDLE_STATE:
			/*
			 * Attempting to Deauthenticate with a pre-authenticated
			 * peer. Deauthetiate with peer if there exists a
			 * pre-auth context below.
			 */
			break;
		case eLIM_MLM_AUTHENTICATED_STATE:
		case eLIM_MLM_WT_ASSOC_RSP_STATE:
		case eLIM_MLM_LINK_ESTABLISHED_STATE:
			if (!cdf_mem_compare(mlm_deauth_req->peerMacAddr,
					curr_bssId, sizeof(tSirMacAddr))) {
				lim_log(mac_ctx, LOGE,
					FL("received MLM_DEAUTH_REQ with invalid BSS id "
					   "Peer MAC: "MAC_ADDRESS_STR
					   " CFG BSSID Addr : "MAC_ADDRESS_STR),
					MAC_ADDR_ARRAY(
						mlm_deauth_req->peerMacAddr),
					MAC_ADDR_ARRAY(curr_bssId));
				/*
				 * Deauthentication response to host triggered
				 * deauthentication
				 */
				sme_deauth_rsp =
				    cdf_mem_malloc(sizeof(tSirSmeDeauthRsp));
				if (NULL == sme_deauth_rsp) {
					lim_log(mac_ctx, LOGP,
						FL("memory allocation failed for deauth rsp"));
					return;
				}

				lim_log(mac_ctx, LOG1,
					FL("send deauth rsp with ret code %d for" MAC_ADDRESS_STR),
					eSIR_SME_DEAUTH_STATUS,
					MAC_ADDR_ARRAY(mlm_deauth_req->peerMacAddr));

				sme_deauth_rsp->messageType =
						eWNI_SME_DEAUTH_RSP;
				sme_deauth_rsp->length =
						sizeof(tSirSmeDeauthRsp);
				sme_deauth_rsp->statusCode =
						eSIR_SME_DEAUTH_STATUS;
				sme_deauth_rsp->sessionId =
						mlm_deauth_req->sessionId;
				sme_deauth_rsp->transactionId = 0;

				cdf_mem_copy(sme_deauth_rsp->peerMacAddr,
						mlm_deauth_req->peerMacAddr,
						sizeof(tSirMacAddr));

				msg_buf = (uint32_t *)sme_deauth_rsp;

				lim_send_sme_disassoc_deauth_ntf(mac_ctx,
						CDF_STATUS_SUCCESS, msg_buf);
				return;
			}

			if ((session->limMlmState ==
			     eLIM_MLM_AUTHENTICATED_STATE) ||
			    (session->limMlmState ==
			     eLIM_MLM_WT_ASSOC_RSP_STATE)) {
				/* Send deauth frame to peer entity */
				lim_send_deauth_mgmt_frame(mac_ctx,
						mlm_deauth_req->reasonCode,
						mlm_deauth_req->peerMacAddr,
						session, false);
				/* Prepare and Send LIM_MLM_DEAUTH_CNF */
				mlm_deauth_cnf.resultCode = eSIR_SME_SUCCESS;
				session->limMlmState = eLIM_MLM_IDLE_STATE;
				MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
						 session->peSessionId,
						 session->limMlmState));
				goto end;
			}
			break;
		default:
			lim_log(mac_ctx, LOGW,
				FL("received MLM_DEAUTH_REQ with in state %d for peer "
				   MAC_ADDRESS_STR),
				session->limMlmState,
				MAC_ADDR_ARRAY(mlm_deauth_req->peerMacAddr));
			lim_print_mlm_state(mac_ctx, LOGW,
					    session->limMlmState);
			/* Prepare and Send LIM_MLM_DEAUTH_CNF */
			mlm_deauth_cnf.resultCode =
				eSIR_SME_STA_NOT_AUTHENTICATED;

			goto end;
		}
		break;
	case eLIM_STA_IN_IBSS_ROLE:
		lim_log(mac_ctx, LOGE, FL("received MLM_DEAUTH_REQ IBSS Mode"));
		mlm_deauth_cnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		goto end;
	case eLIM_AP_ROLE:
	case eLIM_P2P_DEVICE_GO:
		if (true ==
			mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running) {
			lim_log(mac_ctx, LOGE,
				FL("CAC timer is running, drop disassoc from going out"));
			mlm_deauth_cnf.resultCode = eSIR_SME_SUCCESS;
			goto end;
		}
		break;

	default:
		break;
	} /* end switch (GET_LIM_SYSTEM_ROLE(session)) */

	/*
	 * Check if there exists a context for the peer entity
	 * to be deauthenticated with.
	 */
	sta_ds = dph_lookup_hash_entry(mac_ctx, mlm_deauth_req->peerMacAddr,
				       &aid, &session->dph.dphHashTable);

	if (sta_ds == NULL) {
		/* Check if there exists pre-auth context for this STA */
		auth_node = lim_search_pre_auth_list(mac_ctx,
						mlm_deauth_req->peerMacAddr);
		if (auth_node == NULL) {
			/*
			 * Received DEAUTH REQ for a STA that is neither
			 * Associated nor Pre-authenticated. Log error,
			 * Prepare and Send LIM_MLM_DEAUTH_CNF
			 */
			lim_log(mac_ctx, LOGW,
				FL("received MLM_DEAUTH_REQ in mlme state %d for STA that "
				   "does not have context, Addr="
				   MAC_ADDRESS_STR),
				session->limMlmState,
				MAC_ADDR_ARRAY(mlm_deauth_req->peerMacAddr));
			mlm_deauth_cnf.resultCode =
				eSIR_SME_STA_NOT_AUTHENTICATED;
		} else {
			mlm_deauth_cnf.resultCode = eSIR_SME_SUCCESS;
			/* Delete STA from pre-auth STA list */
			lim_delete_pre_auth_node(mac_ctx,
						 mlm_deauth_req->peerMacAddr);
			/* Send Deauthentication frame to peer entity */
			lim_send_deauth_mgmt_frame(mac_ctx,
						   mlm_deauth_req->reasonCode,
						   mlm_deauth_req->peerMacAddr,
						   session, false);
		}
		goto end;
	} else if ((sta_ds->mlmStaContext.mlmState !=
		    eLIM_MLM_LINK_ESTABLISHED_STATE) &&
		   (sta_ds->mlmStaContext.mlmState !=
		    eLIM_MLM_WT_ASSOC_CNF_STATE)) {
		/*
		 * received MLM_DEAUTH_REQ for STA that either has no context or
		 * in some transit state
		 */
		lim_log(mac_ctx, LOGW,
			FL("Invalid MLM_DEAUTH_REQ, Addr="MAC_ADDRESS_STR),
			MAC_ADDR_ARRAY(mlm_deauth_req->peerMacAddr));
		/* Prepare and Send LIM_MLM_DEAUTH_CNF */
		mlm_deauth_cnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		goto end;
	}
	/* sta_ds->mlmStaContext.rxPurgeReq     = 1; */
	sta_ds->mlmStaContext.disassocReason = (tSirMacReasonCodes)
					       mlm_deauth_req->reasonCode;
	sta_ds->mlmStaContext.cleanupTrigger = mlm_deauth_req->deauthTrigger;
	mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDeauthReq = mlm_deauth_req;
	/*
	 * Set state to mlm State to eLIM_MLM_WT_DEL_STA_RSP_STATE
	 * This is to address the issue of race condition between
	 * disconnect request from the HDD and disassoc from
	 * inactivity timer. This will make sure that we will not
	 * process disassoc if deauth is in progress for the station
	 * and thus mlmStaContext.cleanupTrigger will not be overwritten.
	 */
	sta_ds->mlmStaContext.mlmState = eLIM_MLM_WT_DEL_STA_RSP_STATE;
	/* Send Deauthentication frame to peer entity */
	lim_send_deauth_mgmt_frame(mac_ctx, mlm_deauth_req->reasonCode,
				   mlm_deauth_req->peerMacAddr, session, true);
	return;
end:
	cdf_mem_copy((uint8_t *) &mlm_deauth_cnf.peerMacAddr,
		     (uint8_t *) mlm_deauth_req->peerMacAddr,
		     sizeof(tSirMacAddr));
	mlm_deauth_cnf.deauthTrigger = mlm_deauth_req->deauthTrigger;
	mlm_deauth_cnf.aid = mlm_deauth_req->aid;
	mlm_deauth_cnf.sessionId = mlm_deauth_req->sessionId;

	/* Free up buffer allocated for mlmDeauthReq */
	cdf_mem_free(mlm_deauth_req);
	lim_post_sme_message(mac_ctx,
			     LIM_MLM_DEAUTH_CNF, (uint32_t *) &mlm_deauth_cnf);
}

/*
 * lim_process_deauth_ack_timeout() - wrapper function around
 * lim_send_deauth_cnf
 *
 * @mac_ctx:        mac_ctx
 *
 * wrapper function around lim_send_deauth_cnf
 *
 * Return: void
 */
void lim_process_deauth_ack_timeout(tpAniSirGlobal mac_ctx)
{
	lim_log(mac_ctx, LOG1, FL(""));
	lim_send_deauth_cnf(mac_ctx);
}

/*
 * lim_process_mlm_deauth_req() - This function is called to process
 * MLM_DEAUTH_REQ message from SME
 *
 * @mac_ctx:      Pointer to Global MAC structure
 * @msg_buf:      A pointer to the MLM message buffer
 *
 * This function is called to process MLM_DEAUTH_REQ message from SME
 *
 * @Return: None
 */
static void
lim_process_mlm_deauth_req(tpAniSirGlobal mac_ctx, uint32_t *msg_buf)
{
	tLimMlmDeauthReq *mlm_deauth_req;
	tpPESession session;

	if (msg_buf == NULL) {
		PELOGE(lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));)
		return;
	}

	mlm_deauth_req = (tLimMlmDeauthReq *) msg_buf;
	lim_log(mac_ctx, LOG1,
		FL("Process Deauth Req on sessionID %d from: "
		   MAC_ADDRESS_STR),
		mlm_deauth_req->sessionId,
		MAC_ADDR_ARRAY(mlm_deauth_req->peerMacAddr));

	session = pe_find_session_by_session_id(mac_ctx,
				mlm_deauth_req->sessionId);
	if (NULL == session) {
		lim_log(mac_ctx, LOGE,
			FL("session does not exist for given sessionId %d"),
			mlm_deauth_req->sessionId);
		return;
	}
	lim_process_mlm_deauth_req_ntf(mac_ctx, CDF_STATUS_SUCCESS,
				       (uint32_t *) msg_buf);
}

/**
 * lim_process_mlm_set_keys_req() - This function is called to process
 * MLM_SETKEYS_REQ message from SME
 *
 * @mac_ctx:      Pointer to Global MAC structure
 * @msg_buf:      A pointer to the MLM message buffer
 *
 * This function is called to process MLM_SETKEYS_REQ message from SME
 *
 * @Return: None
 */
static void
lim_process_mlm_set_keys_req(tpAniSirGlobal mac_ctx, uint32_t *msg_buf)
{
	uint16_t aid;
	uint16_t sta_idx = 0;
	uint32_t default_key_id = 0;
	tSirMacAddr curr_bssid;
	tpDphHashNode sta_ds;
	tLimMlmSetKeysReq *mlm_set_keys_req;
	tLimMlmSetKeysCnf mlm_set_keys_cnf;
	tpPESession session;

	if (msg_buf == NULL) {
		lim_log(mac_ctx, LOGE, FL("Buffer is Pointing to NULL"));
		return;
	}

	mlm_set_keys_req = (tLimMlmSetKeysReq *) msg_buf;
	/* Hold onto the SetKeys request parameters */
	mac_ctx->lim.gpLimMlmSetKeysReq = (void *)mlm_set_keys_req;
	session = pe_find_session_by_session_id(mac_ctx,
				mlm_set_keys_req->sessionId);
	if (NULL == session) {
		lim_log(mac_ctx, LOGE,
			FL("session does not exist for given sessionId"));
		return;
	}

	lim_log(mac_ctx, LOGW,
		FL("Received MLM_SETKEYS_REQ with parameters:"
		   "AID [%d], ED Type [%d], # Keys [%d] & Peer MAC Addr - "),
		mlm_set_keys_req->aid, mlm_set_keys_req->edType,
		mlm_set_keys_req->numKeys);
	lim_print_mac_addr(mac_ctx, mlm_set_keys_req->peerMacAddr, LOGW);
	sir_copy_mac_addr(curr_bssid, session->bssId);

	switch (GET_LIM_SYSTEM_ROLE(session)) {
	case eLIM_STA_ROLE:
	case eLIM_BT_AMP_STA_ROLE:
		/*
		 * In case of TDLS, peerMac address need not be BssId. Skip this
		 * check if TDLS is enabled.
		 */
#ifndef FEATURE_WLAN_TDLS
		if ((!lim_is_addr_bc(mlm_set_keys_req->peerMacAddr)) &&
		    (!cdf_mem_compare(mlm_set_keys_req->peerMacAddr,
				      curr_bssid, sizeof(tSirMacAddr)))) {
			lim_log(mac_ctx, LOGW,
				FL("Received MLM_SETKEYS_REQ with invalid BSSID"
				MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(mlm_set_keys_req->peerMacAddr));
			/*
			 * Prepare and Send LIM_MLM_SETKEYS_CNF with error code
			 */
			mlm_set_keys_cnf.resultCode =
				eSIR_SME_INVALID_PARAMETERS;
			goto end;
		}
#endif
		break;
	case eLIM_STA_IN_IBSS_ROLE:
		/*
		 * update the IBSS PE session encrption type based on the
		 * key type
		 */
		session->encryptType = mlm_set_keys_req->edType;
		break;
	default:
		break;
	}

	/*
	 * Use the "unicast" parameter to determine if the "Group Keys"
	 * are being set.
	 * mlm_set_keys_req->key.unicast = 0 -> Multicast/broadcast
	 * mlm_set_keys_req->key.unicast - 1 -> Unicast keys are being set
	 */
	if (lim_is_addr_bc(mlm_set_keys_req->peerMacAddr)) {
		lim_log(mac_ctx, LOG1, FL("Trying to set Group Keys...%d "),
			mlm_set_keys_req->sessionId);
		/*
		 * When trying to set Group Keys for any security mode other
		 * than WEP, use the STA Index corresponding to the AP...
		 */
		switch (mlm_set_keys_req->edType) {
		case eSIR_ED_CCMP:
#ifdef WLAN_FEATURE_11W
		case eSIR_ED_AES_128_CMAC:
#endif
			sta_idx = session->staId;
			break;
		default:
			break;
		}
	} else {
		lim_log(mac_ctx, LOG1, FL("Trying to set Unicast Keys..."));
		/*
		 * Check if there exists a context for the
		 * peer entity for which keys need to be set.
		 */
		sta_ds = dph_lookup_hash_entry(mac_ctx,
				mlm_set_keys_req->peerMacAddr, &aid,
				&session->dph.dphHashTable);
		if ((sta_ds == NULL) ||
		    ((sta_ds->mlmStaContext.mlmState !=
		    eLIM_MLM_LINK_ESTABLISHED_STATE) &&
		    !LIM_IS_AP_ROLE(session))) {
			/*
			 * Received LIM_MLM_SETKEYS_REQ for STA that does not
			 * have context or in some transit state.
			 */
			lim_log(mac_ctx, LOG1,
				FL("Invalid MLM_SETKEYS_REQ, Addr = "
				   MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(mlm_set_keys_req->peerMacAddr));
			/* Prepare and Send LIM_MLM_SETKEYS_CNF */
			mlm_set_keys_cnf.resultCode =
				eSIR_SME_INVALID_PARAMETERS;
			goto end;
		} else {
			sta_idx = sta_ds->staIndex;
		}
	}

	if ((mlm_set_keys_req->numKeys == 0)
	    && (mlm_set_keys_req->edType != eSIR_ED_NONE)) {
		/*
		 * Broadcast/Multicast Keys (for WEP!!) are NOT sent
		 * via this interface!! This indicates to HAL that the WEP Keys
		 * need to be extracted from the CFG and applied to hardware
		 */
		default_key_id = 0xff;
	} else if (mlm_set_keys_req->key[0].keyId &&
		   ((mlm_set_keys_req->edType == eSIR_ED_WEP40) ||
		    (mlm_set_keys_req->edType == eSIR_ED_WEP104))) {
		/*
		 * If the Key Id is non zero and encryption mode is WEP,
		 * the key index is coming from the upper layers so that key
		 * only need to be used as the default tx key, This is being
		 * used only in case of WEP mode in HAL
		 */
		default_key_id = mlm_set_keys_req->key[0].keyId;
	} else {
		default_key_id = 0;
	}
	lim_log(mac_ctx, LOG1,
		FL("Trying to set keys for STA Index [%d], using default_key_id [%d]"),
		sta_idx, default_key_id);

	if (lim_is_addr_bc(mlm_set_keys_req->peerMacAddr)) {
		session->limPrevMlmState = session->limMlmState;
		session->limMlmState = eLIM_MLM_WT_SET_BSS_KEY_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
				 session->peSessionId, session->limMlmState));
		lim_log(mac_ctx, LOG1, FL("Trying to set Group Keys...%d "),
			session->peSessionId);
		/* Package WMA_SET_BSSKEY_REQ message parameters */
		lim_send_set_bss_key_req(mac_ctx, mlm_set_keys_req, session);
		return;
	} else {
		/*
		 * Package WMA_SET_STAKEY_REQ / WMA_SET_STA_BCASTKEY_REQ message
		 * parameters
		 */
		lim_send_set_sta_key_req(mac_ctx, mlm_set_keys_req, sta_idx,
					 (uint8_t) default_key_id, session,
					 true);
		return;
	}
end:
	mlm_set_keys_cnf.sessionId = mlm_set_keys_req->sessionId;
	lim_post_sme_set_keys_cnf(mac_ctx, mlm_set_keys_req, &mlm_set_keys_cnf);
}

/**
 * lim_process_periodic_probe_req_timer() - This function is called to process
 * periodic probe request to send during scan.
 *
 * @mac_ctx:      Pointer to Global MAC structure
 *
 * This function is called to process periodic probe request to send during scan
 *
 * @Return None
 */
static void lim_process_periodic_probe_req_timer(tpAniSirGlobal mac_ctx)
{
	uint8_t channel_num;
	uint8_t i = 0;
	tLimMlmScanReq *mlm_scan_req;
	tSirRetStatus status = eSIR_SUCCESS;
	TX_TIMER *probe_req_timer =
		&mac_ctx->lim.limTimers.gLimPeriodicProbeReqTimer;

	if (cdf_mc_timer_get_current_state(&probe_req_timer->cdf_timer)
					   != CDF_TIMER_STATE_STOPPED) {
		lim_log(mac_ctx, LOG1, FL("Invalid state of timer"));
		return;
	}

	if (!((mac_ctx->lim.gLimMlmState == eLIM_MLM_WT_PROBE_RESP_STATE)
	    && (probe_req_timer->sessionId != 0xff)
	    && (mac_ctx->lim.probeCounter < mac_ctx->lim.maxProbe))) {
		lim_log(mac_ctx, LOG1,
			FL("received unexpected Periodic scan timeout in state %X"),
			mac_ctx->lim.gLimMlmState);
		return;
	}

	mlm_scan_req = mac_ctx->lim.gpLimMlmScanReq;
	lim_log(mac_ctx, LOG1, FL("Scanning : Periodic scanning"));
	mac_ctx->lim.probeCounter++;
	/* Periodic channel timer timed out to send probe request. */
	channel_num = lim_get_current_scan_channel(mac_ctx);
	do {
		/*
		 * Prepare and send Probe Request frame for all the SSIDs
		 * present in the saved MLM
		 */
		status = lim_send_probe_req_mgmt_frame(mac_ctx,
				&mlm_scan_req->ssId[i], mlm_scan_req->bssId,
				channel_num, mac_ctx->lim.gSelfMacAddr,
				mlm_scan_req->dot11mode,
				mlm_scan_req->uIEFieldLen,
				(uint8_t *) (mlm_scan_req) +
					mlm_scan_req->uIEFieldOffset);
		if (status != eSIR_SUCCESS) {
			lim_log(mac_ctx, LOGE,
				FL("send ProbeReq failed for SSID %s on channel: %d"),
				mlm_scan_req->ssId[i].ssId, channel_num);
			return;
		}
		i++;
	} while (i < mlm_scan_req->numSsid);
	/* Activate timer again */
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_TIMER_ACTIVATE,
			 probe_req_timer->sessionId,
			 eLIM_PERIODIC_PROBE_REQ_TIMER));
	if (tx_timer_activate(probe_req_timer) != TX_SUCCESS) {
		lim_log(mac_ctx, LOGP,
			FL("could not start periodic probe req timer"));
		return;
	}
}

/**
 * lim_process_join_failure_timeout() - This function is called to process
 * JoinFailureTimeout
 *
 * @mac_ctx:      Pointer to Global MAC structure
 *
 * This function is called to process JoinFailureTimeout
 *
 * @Return None
 */
static void lim_process_join_failure_timeout(tpAniSirGlobal mac_ctx)
{
	tLimMlmJoinCnf mlm_join_cnf;
	uint32_t len;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	host_log_rssi_pkt_type *rssi_log = NULL;
#endif
	tpPESession session;

	session = pe_find_session_by_session_id(mac_ctx,
			mac_ctx->lim.limTimers.gLimJoinFailureTimer.sessionId);
	if (NULL == session) {
		lim_log(mac_ctx, LOGE,
			FL("Session Does not exist for given sessionID"));
		return;
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	WLAN_HOST_DIAG_LOG_ALLOC(rssi_log,
				host_log_rssi_pkt_type, LOG_WLAN_RSSI_UPDATE_C);
	if (rssi_log)
		rssi_log->rssi = session->rssi;
	WLAN_HOST_DIAG_LOG_REPORT(rssi_log);
#endif

	if (session->limMlmState == eLIM_MLM_WT_JOIN_BEACON_STATE) {
		len = sizeof(tSirMacAddr);
		/* Change timer for future activations */
		lim_deactivate_and_change_timer(mac_ctx, eLIM_JOIN_FAIL_TIMER);
		/* Change Periodic probe req timer for future activation */
		lim_deactivate_and_change_timer(mac_ctx,
					eLIM_PERIODIC_JOIN_PROBE_REQ_TIMER);
		/* Issue MLM join confirm with timeout reason code */
		lim_log(mac_ctx, LOGE,
			FL("Join Failure Timeout, In eLIM_MLM_WT_JOIN_BEACON_STATE session:%d "
			   MAC_ADDRESS_STR),
			session->peSessionId, MAC_ADDR_ARRAY(session->bssId));

		mlm_join_cnf.resultCode = eSIR_SME_JOIN_TIMEOUT_RESULT_CODE;
		mlm_join_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
		session->limMlmState = eLIM_MLM_IDLE_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
				 session->peSessionId, session->limMlmState));
		if (lim_set_link_state(mac_ctx, eSIR_LINK_IDLE_STATE,
				       session->bssId, session->selfMacAddr,
				       NULL, NULL) != eSIR_SUCCESS)
			lim_log(mac_ctx, LOGE,
				("Failed to set the LinkState"));
		/* Update PE session Id */
		mlm_join_cnf.sessionId = session->peSessionId;
		/* Freeup buffer allocated to join request */
		if (session->pLimMlmJoinReq) {
			cdf_mem_free(session->pLimMlmJoinReq);
			session->pLimMlmJoinReq = NULL;
		}
		lim_post_sme_message(mac_ctx, LIM_MLM_JOIN_CNF,
				     (uint32_t *) &mlm_join_cnf);
		return;
	} else {
		lim_log(mac_ctx, LOGW,
			FL("received unexpected JOIN failure timeout in state %X"),
			session->limMlmState);
		lim_print_mlm_state(mac_ctx, LOGW, session->limMlmState);
	}
}

/**
 * lim_process_periodic_join_probe_req_timer() - This function is called to
 * process periodic probe request send during joining process.
 *
 * @mac_ctx:      Pointer to Global MAC structure
 *
 * This function is called to process periodic probe request send during
 * joining process.
 *
 * @Return None
 */
static void lim_process_periodic_join_probe_req_timer(tpAniSirGlobal mac_ctx)
{
	tpPESession session;
	tSirMacSSid ssid;

	session = pe_find_session_by_session_id(mac_ctx,
		mac_ctx->lim.limTimers.gLimPeriodicJoinProbeReqTimer.sessionId);
	if (NULL == session) {
		lim_log(mac_ctx, LOGE,
			FL("session does not exist for given SessionId : %d"),
			mac_ctx->lim.limTimers.gLimPeriodicJoinProbeReqTimer.
			sessionId);
		return;
	}

	if ((true ==
	    tx_timer_running(&mac_ctx->lim.limTimers.gLimJoinFailureTimer))
		&& (session->limMlmState == eLIM_MLM_WT_JOIN_BEACON_STATE)) {
		cdf_mem_copy(ssid.ssId, session->ssId.ssId,
			     session->ssId.length);
		ssid.length = session->ssId.length;
		lim_send_probe_req_mgmt_frame(mac_ctx, &ssid,
			session->pLimMlmJoinReq->bssDescription.bssId,
			session->currentOperChannel /*chanNum */,
			session->selfMacAddr, session->dot11mode,
			session->pLimJoinReq->addIEScan.length,
			session->pLimJoinReq->addIEScan.addIEdata);
		lim_deactivate_and_change_timer(mac_ctx,
				eLIM_PERIODIC_JOIN_PROBE_REQ_TIMER);
		/* Activate Join Periodic Probe Req timer */
		if (tx_timer_activate(
		    &mac_ctx->lim.limTimers.gLimPeriodicJoinProbeReqTimer) !=
		    TX_SUCCESS) {
			lim_log(mac_ctx, LOGP,
				FL("could not activate Periodic Join req failure timer"));
			return;
		}
	}
}

/**
 * lim_process_auth_failure_timeout() - This function is called to process Min
 * Channel Timeout during channel scan.
 *
 * @mac_ctx:      Pointer to Global MAC structure
 *
 * This function is called to process Min Channel Timeout during channel scan.
 *
 * @Return: None
 */
static void lim_process_auth_failure_timeout(tpAniSirGlobal mac_ctx)
{
	/* fetch the sessionEntry based on the sessionId */
	tpPESession session;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	host_log_rssi_pkt_type *rssi_log = NULL;
#endif

	session = pe_find_session_by_session_id(mac_ctx,
			mac_ctx->lim.limTimers.gLimAuthFailureTimer.sessionId);
	if (NULL == session) {
		lim_log(mac_ctx, LOGP,
			FL("Session Does not exist for given sessionID"));
		return;
	}

	lim_log(mac_ctx, LOGE,
		FL("received AUTH failure timeout in sessionid %d "
		   "limMlmstate %X limSmeState %X"),
		session->peSessionId, session->limMlmState,
		session->limSmeState);
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	WLAN_HOST_DIAG_LOG_ALLOC(rssi_log, host_log_rssi_pkt_type,
				 LOG_WLAN_RSSI_UPDATE_C);
	if (rssi_log)
		rssi_log->rssi = session->rssi;
	WLAN_HOST_DIAG_LOG_REPORT(rssi_log);
#endif

	switch (session->limMlmState) {
	case eLIM_MLM_WT_AUTH_FRAME2_STATE:
	case eLIM_MLM_WT_AUTH_FRAME4_STATE:
		/*
		 * Requesting STA did not receive next auth frame before Auth
		 * Failure timeout. Issue MLM auth confirm with timeout reason
		 * code. Restore default failure timeout
		 */
		if (CDF_P2P_CLIENT_MODE == session->pePersona
		    && session->defaultAuthFailureTimeout)
			cfg_set_int(mac_ctx,
				    WNI_CFG_AUTHENTICATE_FAILURE_TIMEOUT,
				    session->defaultAuthFailureTimeout);
		lim_restore_from_auth_state(mac_ctx,
				eSIR_SME_AUTH_TIMEOUT_RESULT_CODE,
				eSIR_MAC_UNSPEC_FAILURE_REASON, session);
		break;
	default:
		/*
		 * Auth failure timer should not have timed out
		 * in states other than wt_auth_frame2/4
		 */
		lim_log(mac_ctx, LOGE,
			FL("received unexpected AUTH failure timeout in state %X"),
			session->limMlmState);
		lim_print_mlm_state(mac_ctx, LOGE, session->limMlmState);
		break;
	}
}

/**
 * lim_process_auth_rsp_timeout() - This function is called to process Min
 * Channel Timeout during channel scan.
 *
 * @mac_ctx:      Pointer to Global MAC structure
 *
 * This function is called to process Min Channel Timeout during channel scan.
 *
 * @Return: None
 */
static void
lim_process_auth_rsp_timeout(tpAniSirGlobal mac_ctx, uint32_t auth_idx)
{
	struct tLimPreAuthNode *auth_node;
	tpPESession session;
	uint8_t session_id;

	auth_node = lim_get_pre_auth_node_from_index(mac_ctx,
				&mac_ctx->lim.gLimPreAuthTimerTable, auth_idx);
	if (NULL == auth_node) {
		lim_log(mac_ctx, LOGW, FL("Invalid auth node"));
		return;
	}

	session = pe_find_session_by_bssid(mac_ctx, auth_node->peerMacAddr,
					   &session_id);
	if (NULL == session) {
		lim_log(mac_ctx, LOGW,
			FL("session does not exist for given BSSID "));
		return;
	}

	if (LIM_IS_AP_ROLE(session) || LIM_IS_IBSS_ROLE(session)) {
		if (auth_node->mlmState != eLIM_MLM_WT_AUTH_FRAME3_STATE) {
			lim_log(mac_ctx, LOGE,
				FL("received AUTH rsp timeout in unexpected "
				   "state for MAC address: " MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(auth_node->peerMacAddr));
		} else {
			auth_node->mlmState = eLIM_MLM_AUTH_RSP_TIMEOUT_STATE;
			auth_node->fTimerStarted = 0;
			lim_log(mac_ctx, LOG1,
				FL("AUTH rsp timedout for MAC address "
				   MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(auth_node->peerMacAddr));
			/* Change timer to reactivate it in future */
			lim_deactivate_and_change_per_sta_id_timer(mac_ctx,
				eLIM_AUTH_RSP_TIMER, auth_node->authNodeIdx);
			lim_delete_pre_auth_node(mac_ctx,
						 auth_node->peerMacAddr);
		}
	}
}

/**
 * lim_process_assoc_failure_timeout() - This function is called to process Min
 * Channel Timeout during channel scan.
 *
 * @mac_ctx      Pointer to Global MAC structure
 *
 * This function is called to process Min Channel Timeout during channel scan.
 *
 * @Return: None
 */
static void
lim_process_assoc_failure_timeout(tpAniSirGlobal mac_ctx, uint32_t msg_type)
{

	tLimMlmAssocCnf mlm_assoc_cnf;
	tpPESession session;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	host_log_rssi_pkt_type *rssi_log = NULL;
#endif
	/*
	 * to fetch the lim/mlm state based on the session_id, use the
	 * below sessionEntry
	 */
	uint8_t session_id;

	if (msg_type == LIM_ASSOC)
		session_id =
		    mac_ctx->lim.limTimers.gLimAssocFailureTimer.sessionId;
	else
		session_id =
		    mac_ctx->lim.limTimers.gLimReassocFailureTimer.sessionId;

	session = pe_find_session_by_session_id(mac_ctx, session_id);
	if (NULL == session) {
		lim_log(mac_ctx, LOGP,
			FL("Session Does not exist for given sessionID"));
		return;
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	WLAN_HOST_DIAG_LOG_ALLOC(rssi_log,
				 host_log_rssi_pkt_type,
				 LOG_WLAN_RSSI_UPDATE_C);
	if (rssi_log)
		rssi_log->rssi = session->rssi;
	WLAN_HOST_DIAG_LOG_REPORT(rssi_log);
#endif

	lim_log(mac_ctx, LOG1,
		FL("Re/Association Response not received before timeout "));

	if ((LIM_IS_AP_ROLE(session) || LIM_IS_BT_AMP_AP_ROLE(session)) ||
	    ((session->limMlmState != eLIM_MLM_WT_ASSOC_RSP_STATE) &&
	    (session->limMlmState != eLIM_MLM_WT_REASSOC_RSP_STATE) &&
	    (session->limMlmState != eLIM_MLM_WT_FT_REASSOC_RSP_STATE))) {
		/*
		 * Re/Assoc failure timer should not have timedout on AP
		 * or in a state other than wt_re/assoc_response.
		 */
		lim_log(mac_ctx, LOGW,
			FL("received unexpected REASSOC failure timeout in state %X for role %d"),
			session->limMlmState,
			GET_LIM_SYSTEM_ROLE(session));
		lim_print_mlm_state(mac_ctx, LOGW, session->limMlmState);
		return;
	}

	if ((msg_type == LIM_ASSOC) || ((msg_type == LIM_REASSOC)
	     && (session->limMlmState == eLIM_MLM_WT_FT_REASSOC_RSP_STATE))) {
		lim_log(mac_ctx, LOGE,
			FL("(Re)Assoc Failure Timeout occurred."));
		session->limMlmState = eLIM_MLM_IDLE_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
			session->peSessionId, session->limMlmState));
		/* Change timer for future activations */
		lim_deactivate_and_change_timer(mac_ctx, eLIM_ASSOC_FAIL_TIMER);
		/*
		 * Free up buffer allocated for JoinReq held by
		 * MLM state machine
		 */
		if (session->pLimMlmJoinReq) {
			cdf_mem_free(session->pLimMlmJoinReq);
			session->pLimMlmJoinReq = NULL;
		}
		/* To remove the preauth node in case of fail to associate */
		if (lim_search_pre_auth_list(mac_ctx, session->bssId)) {
			lim_log(mac_ctx, LOG1,
				FL(" delete pre auth node for "MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(session->bssId));
			lim_delete_pre_auth_node(mac_ctx,
						 session->bssId);
		}

		mlm_assoc_cnf.resultCode = eSIR_SME_ASSOC_TIMEOUT_RESULT_CODE;
		mlm_assoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
		/* Update PE session Id */
		mlm_assoc_cnf.sessionId = session->peSessionId;
		if (msg_type == LIM_ASSOC) {
			lim_post_sme_message(mac_ctx, LIM_MLM_ASSOC_CNF,
					     (uint32_t *) &mlm_assoc_cnf);
		} else {
			/*
			 * Will come here only in case of 11r, Ese FT
			 * when reassoc rsp is not received and we
			 * receive a reassoc - timesout
			 */
			mlm_assoc_cnf.resultCode =
				eSIR_SME_FT_REASSOC_TIMEOUT_FAILURE;
			lim_post_sme_message(mac_ctx, LIM_MLM_REASSOC_CNF,
					     (uint32_t *) &mlm_assoc_cnf);
		}
	} else {
		/*
		 * Restore pre-reassoc req state.
		 * Set BSSID to currently associated AP address.
		 */
		session->limMlmState = session->limPrevMlmState;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
				 session->peSessionId, session->limMlmState));
		lim_restore_pre_reassoc_state(mac_ctx,
				eSIR_SME_REASSOC_TIMEOUT_RESULT_CODE,
				eSIR_MAC_UNSPEC_FAILURE_STATUS, session);
	}
}

/**
 * lim_complete_mlm_scan() - This function is called to send MLM_SCAN_CNF
 * message to SME state machine.
 *
 * @mac_ctx:      Pointer to Global MAC structure
 * @ret_code:     Result code to be sent
 *
 * This function is called to send MLM_SCAN_CNF message to SME state machine.
 *
 * @Return: None
 */

void lim_complete_mlm_scan(tpAniSirGlobal mac_ctx, tSirResultCodes ret_code)
{
	tLimMlmScanCnf mlm_scan_cnf;

	/* Restore previous MLM state */
	mac_ctx->lim.gLimMlmState = mac_ctx->lim.gLimPrevMlmState;
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE, NO_SESSION,
			 mac_ctx->lim.gLimMlmState));
	lim_restore_pre_scan_state(mac_ctx);
	/* Free up mac_ctx->lim.gLimMlmScanReq */
	if (NULL != mac_ctx->lim.gpLimMlmScanReq) {
		cdf_mem_free(mac_ctx->lim.gpLimMlmScanReq);
		mac_ctx->lim.gpLimMlmScanReq = NULL;
	}

	mlm_scan_cnf.resultCode = ret_code;
	lim_post_sme_message(mac_ctx, LIM_MLM_SCAN_CNF,
			     (uint32_t *) &mlm_scan_cnf);
}

/**
 * lim_set_channel() - set channel api for lim
 *
 * @mac_ctx:                Pointer to Global MAC structure
 * @channel:                power save state
 * @ch_center_freq_seg0:    center freq seq 0
 * @ch_center_freq_seg1:    center freq seq 1
 * @ch_width:               channel width
 * @max_tx_power:           max tx power
 * @pe_session_id:          pe session id
 *
 * set channel api for lim
 *
 * @Return: None
 */
void lim_set_channel(tpAniSirGlobal mac_ctx, uint8_t channel,
		     uint8_t ch_center_freq_seg0, uint8_t ch_center_freq_seg1,
		     phy_ch_width ch_width, tPowerdBm max_tx_power,
		     uint8_t pe_session_id)
{
#if !defined WLAN_FEATURE_VOWIFI
	uint32_t localPwrConstraint;
#endif
	tpPESession pe_session;
	pe_session = pe_find_session_by_session_id(mac_ctx, pe_session_id);

	if (NULL == pe_session) {
		lim_log(mac_ctx, LOGP, FL("Invalid PE session = %d"),
			pe_session_id);
		return;
	}
#if defined WLAN_FEATURE_VOWIFI
	lim_send_switch_chnl_params(mac_ctx, channel, ch_center_freq_seg0,
				    ch_center_freq_seg1, ch_width,
				    max_tx_power, pe_session_id, false);
#else
	if (wlan_cfg_get_int(mac_ctx, WNI_CFG_LOCAL_POWER_CONSTRAINT,
			    &localPwrConstraint) != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGP,
			FL("couldn't read CFG_LOCAL_POWER_CONSTRAINT"));
		return;
	}
	/* Send WMA_CHNL_SWITCH_IND to HAL */
	lim_send_switch_chnl_params(mac_ctx, channel, ch_center_freq_seg0,
				    ch_center_freq_seg1, ch_width,
				    (tPowerdBm) localPwrConstraint,
				    pe_session_id, false);
#endif
}
