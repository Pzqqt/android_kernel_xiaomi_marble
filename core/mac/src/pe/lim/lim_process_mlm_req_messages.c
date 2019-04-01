/*
 * Copyright (c) 2011-2019 The Linux Foundation. All rights reserved.
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

#include "cds_api.h"
#include "wni_cfg.h"
#include "ani_global.h"
#include "sir_api.h"
#include "sir_params.h"
#include "cfg_ucfg_api.h"
#include "sch_api.h"
#include "utils_api.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_prop_exts_utils.h"
#include "lim_security_utils.h"
#include "lim_send_messages.h"
#include "lim_send_messages.h"
#include "lim_session_utils.h"
#include <lim_ft.h>
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
#include "host_diag_core_log.h"
#endif
#include "wma_if.h"
#include "wlan_reg_services_api.h"
#include "lim_process_fils.h"
#include "wlan_mlme_public_struct.h"

static void lim_process_mlm_auth_req(struct mac_context *, uint32_t *);
static void lim_process_mlm_assoc_req(struct mac_context *, uint32_t *);
static void lim_process_mlm_disassoc_req(struct mac_context *, uint32_t *);
static void lim_process_mlm_set_keys_req(struct mac_context *, uint32_t *);

/* MLM Timeout event handler templates */
static void lim_process_auth_rsp_timeout(struct mac_context *, uint32_t);
static void lim_process_periodic_join_probe_req_timer(struct mac_context *);
static void lim_process_auth_retry_timer(struct mac_context *);

/**
 * lim_process_sae_auth_timeout() - This function is called to process sae
 * auth timeout
 * @mac_ctx: Pointer to Global MAC structure
 *
 * @Return: None
 */
static void lim_process_sae_auth_timeout(struct mac_context *mac_ctx)
{
	struct pe_session *session;

	session = pe_find_session_by_session_id(mac_ctx,
			mac_ctx->lim.limTimers.sae_auth_timer.sessionId);
	if (!session) {
		pe_err("Session does not exist for given session id");
		return;
	}

	pe_warn("SAE auth timeout sessionid %d mlmstate %X SmeState %X",
		session->peSessionId, session->limMlmState,
		session->limSmeState);

	switch (session->limMlmState) {
	case eLIM_MLM_WT_SAE_AUTH_STATE:
		/*
		 * SAE authentication is not completed. Restore from
		 * auth state.
		 */
		if (session->pePersona == QDF_STA_MODE)
			lim_restore_from_auth_state(mac_ctx,
				eSIR_SME_AUTH_TIMEOUT_RESULT_CODE,
				eSIR_MAC_UNSPEC_FAILURE_REASON, session);
		break;
	default:
		/* SAE authentication is timed out in unexpected state */
		pe_err("received unexpected SAE auth timeout in state %X",
			session->limMlmState);
		lim_print_mlm_state(mac_ctx, LOGE, session->limMlmState);
		break;
	}
}

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
void lim_process_mlm_req_messages(struct mac_context *mac_ctx,
				  struct scheduler_msg *msg)
{
	switch (msg->type) {
	case LIM_MLM_AUTH_REQ:
		lim_process_mlm_auth_req(mac_ctx, msg->bodyptr);
		break;
	case LIM_MLM_ASSOC_REQ:
		lim_process_mlm_assoc_req(mac_ctx, msg->bodyptr);
		break;
	case LIM_MLM_DISASSOC_REQ:
		lim_process_mlm_disassoc_req(mac_ctx, msg->bodyptr);
		break;
	case LIM_MLM_SETKEYS_REQ:
		lim_process_mlm_set_keys_req(mac_ctx, msg->bodyptr);
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
	case SIR_LIM_FT_PREAUTH_RSP_TIMEOUT:
		lim_process_ft_preauth_rsp_timeout(mac_ctx);
		break;
	case SIR_LIM_DISASSOC_ACK_TIMEOUT:
		lim_process_disassoc_ack_timeout(mac_ctx);
		break;
	case SIR_LIM_DEAUTH_ACK_TIMEOUT:
		lim_process_deauth_ack_timeout(mac_ctx);
		break;
	case SIR_LIM_AUTH_RETRY_TIMEOUT:
		lim_process_auth_retry_timer(mac_ctx);
		break;
	case SIR_LIM_AUTH_SAE_TIMEOUT:
		lim_process_sae_auth_timeout(mac_ctx);
		break;
	case LIM_MLM_TSPEC_REQ:
	default:
		break;
	} /* switch (msg->type) */
}

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
static void mlm_add_sta(struct mac_context *mac_ctx, tpAddStaParams sta_param,
		uint8_t *bssid, uint8_t ht_capable, struct pe_session *session_entry)
{
	uint32_t val;
	uint32_t self_dot11mode = mac_ctx->mlme_cfg->dot11_mode.dot11_mode;

	sta_param->staType = STA_ENTRY_SELF; /* Identifying self */

	qdf_mem_copy(sta_param->bssId, bssid, sizeof(tSirMacAddr));
	qdf_mem_copy(sta_param->staMac, session_entry->selfMacAddr,
		     sizeof(tSirMacAddr));

	/* Configuration related parameters to be changed to support BT-AMP */

	val = mac_ctx->mlme_cfg->sap_cfg.listen_interval;
	sta_param->listenInterval = (uint16_t) val;

	sta_param->shortPreambleSupported =
		mac_ctx->mlme_cfg->ht_caps.short_preamble;

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
		sta_param->max_amsdu_num =
			lim_get_ht_capability(mac_ctx, eHT_MAX_AMSDU_NUM,
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
				session_entry->vht_config.su_beam_formee;
		sta_param->vhtTxMUBformeeCapable =
				session_entry->vht_config.mu_beam_formee;
		sta_param->enable_su_tx_bformer =
				session_entry->vht_config.su_beam_former;
	}

	if (lim_is_session_he_capable(session_entry))
		lim_add_self_he_cap(sta_param, session_entry);

	/*
	 * Since this is Self-STA, need to populate Self MAX_AMPDU_SIZE
	 * capabilities
	 */
	if (IS_DOT11_MODE_VHT(self_dot11mode)) {
		sta_param->maxAmpduSize =
		mac_ctx->mlme_cfg->vht_caps.vht_cap_info.ampdu_len_exponent;
	}
	sta_param->enableVhtpAid = session_entry->enableVhtpAid;
	sta_param->enableAmpduPs = session_entry->enableAmpduPs;
	sta_param->enableHtSmps = session_entry->enableHtSmps;
	sta_param->htSmpsconfig = session_entry->htSmpsvalue;
	sta_param->send_smps_action = session_entry->send_smps_action;

	lim_populate_own_rate_set(mac_ctx, &sta_param->supportedRates, NULL,
				  false, session_entry, NULL, NULL);

	pe_debug("GF: %d, ChnlWidth: %d, MimoPS: %d, lsigTXOP: %d, dsssCCK: %d,"
		" SGI20: %d, SGI40%d", sta_param->greenFieldCapable,
		sta_param->ch_width, sta_param->mimoPS,
		sta_param->lsigTxopProtection, sta_param->fDsssCckMode40Mhz,
		sta_param->fShortGI20Mhz, sta_param->fShortGI40Mhz);

	if (QDF_P2P_GO_MODE == session_entry->pePersona)
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
lim_mlm_add_bss(struct mac_context *mac_ctx,
		tLimMlmStartReq *mlm_start_req, struct pe_session *session)
{
	struct scheduler_msg msg_buf = {0};
	tpAddBssParams addbss_param = NULL;
	struct wlan_mlme_qos *qos_aggr = &mac_ctx->mlme_cfg->qos_mlme_params;
	uint32_t retcode;
	bool is_ch_dfs = false;

	/* Package WMA_ADD_BSS_REQ message parameters */
	addbss_param = qdf_mem_malloc(sizeof(tAddBssParams));
	if (!addbss_param)
		return eSIR_SME_RESOURCES_UNAVAILABLE;

	/* Fill in tAddBssParams members */
	qdf_mem_copy(addbss_param->bssId, mlm_start_req->bssId,
		     sizeof(tSirMacAddr));

	/* Fill in tAddBssParams selfMacAddr */
	qdf_mem_copy(addbss_param->selfMacAddr,
		     session->selfMacAddr, sizeof(tSirMacAddr));

	addbss_param->bssType = mlm_start_req->bssType;
	if (mlm_start_req->bssType == eSIR_IBSS_MODE)
		addbss_param->operMode = BSS_OPERATIONAL_MODE_STA;
	else if (mlm_start_req->bssType == eSIR_INFRA_AP_MODE)
		addbss_param->operMode = BSS_OPERATIONAL_MODE_AP;
	else if (mlm_start_req->bssType == eSIR_NDI_MODE)
		addbss_param->operMode = BSS_OPERATIONAL_MODE_NDI;

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
	if (addbss_param->rateSet.numRates > WLAN_SUPPORTED_RATES_IE_MAX_LEN) {
		pe_warn("num of sup rates %d exceeding the limit %d, resetting",
			addbss_param->rateSet.numRates,
			WLAN_SUPPORTED_RATES_IE_MAX_LEN);
		addbss_param->rateSet.numRates = WLAN_SUPPORTED_RATES_IE_MAX_LEN;
	}
	qdf_mem_copy(addbss_param->rateSet.rate, mlm_start_req->rateSet.rate,
		     addbss_param->rateSet.numRates);

	addbss_param->nwType = mlm_start_req->nwType;
	addbss_param->htCapable = mlm_start_req->htCapable;
	addbss_param->vhtCapable = session->vhtCapability;
	if (lim_is_session_he_capable(session)) {
		lim_update_bss_he_capable(mac_ctx, addbss_param);
		lim_decide_he_op(mac_ctx, addbss_param, session);
		lim_update_usr_he_cap(mac_ctx, session);
	}

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
	addbss_param->ssId.length = mlm_start_req->ssId.length;
	if (addbss_param->ssId.length > WLAN_SSID_MAX_LEN) {
		pe_err("Invalid ssid length %d, max length allowed %d",
		       addbss_param->ssId.length,
		       WLAN_SSID_MAX_LEN);
		qdf_mem_free(addbss_param);
		return eSIR_SME_INVALID_PARAMETERS;
	}
	qdf_mem_copy(addbss_param->ssId.ssId,
		     mlm_start_req->ssId.ssId, addbss_param->ssId.length);
	addbss_param->bHiddenSSIDEn = mlm_start_req->ssidHidden;
	pe_debug("TRYING TO HIDE SSID %d", addbss_param->bHiddenSSIDEn);
	/* CR309183. Disable Proxy Probe Rsp.  Host handles Probe Requests.  Until FW fixed. */
	addbss_param->bProxyProbeRespEn = 0;
	addbss_param->obssProtEnabled = mlm_start_req->obssProtEnabled;

	addbss_param->maxTxPower = session->maxTxPower;

	mlm_add_sta(mac_ctx, &addbss_param->staContext,
		    addbss_param->bssId, addbss_param->htCapable,
		    session);

	addbss_param->status = QDF_STATUS_SUCCESS;
	addbss_param->respReqd = 1;

	/* Set a new state for MLME */
	session->limMlmState = eLIM_MLM_WT_ADD_BSS_RSP_STATE;
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE, session->peSessionId,
			 session->limMlmState));

	/* pass on the session persona to hal */
	addbss_param->halPersona = session->pePersona;

	if (session->ch_width == CH_WIDTH_160MHZ) {
		is_ch_dfs = true;
	} else if (session->ch_width == CH_WIDTH_80P80MHZ) {
		if (wlan_reg_get_channel_state(mac_ctx->pdev,
					mlm_start_req->channelNumber) ==
				CHANNEL_STATE_DFS ||
				wlan_reg_get_channel_state(mac_ctx->pdev,
					session->ch_center_freq_seg1 -
					SIR_80MHZ_START_CENTER_CH_DIFF) ==
				CHANNEL_STATE_DFS)
			is_ch_dfs = true;
	} else {
		if (wlan_reg_get_channel_state(mac_ctx->pdev,
					mlm_start_req->channelNumber) ==
				CHANNEL_STATE_DFS)
			is_ch_dfs = true;
	}

	addbss_param->bSpectrumMgtEnabled =
				session->spectrumMgtEnabled || is_ch_dfs;
	addbss_param->extSetStaKeyParamValid = 0;

	addbss_param->dot11_mode = session->dot11mode;
	addbss_param->nss = session->nss;
	addbss_param->cac_duration_ms = mlm_start_req->cac_duration_ms;
	addbss_param->dfs_regdomain = mlm_start_req->dfs_regdomain;
	addbss_param->beacon_tx_rate = session->beacon_tx_rate;
	if (QDF_IBSS_MODE == addbss_param->halPersona) {
		if (!(mac_ctx->mlme_cfg)) {
			pe_err("Mlme cfg NULL");
			return eSIR_SME_INVALID_PARAMETERS;
		}
		addbss_param->nss_2g = mac_ctx->vdev_type_nss_2g.ibss;
		addbss_param->nss_5g = mac_ctx->vdev_type_nss_5g.ibss;
		addbss_param->tx_aggregation_size =
					qos_aggr->tx_aggregation_size;
		addbss_param->tx_aggregation_size_be =
					qos_aggr->tx_aggregation_size_be;
		addbss_param->tx_aggregation_size_bk =
					qos_aggr->tx_aggregation_size_bk;
		addbss_param->tx_aggregation_size_vi =
					qos_aggr->tx_aggregation_size_vi;
		addbss_param->tx_aggregation_size_vo =
					qos_aggr->tx_aggregation_size_vo;
		addbss_param->rx_aggregation_size =
					qos_aggr->rx_aggregation_size;
	}
	pe_debug("dot11_mode:%d nss value:%d",
			addbss_param->dot11_mode, addbss_param->nss);

	if (cds_is_5_mhz_enabled()) {
		addbss_param->ch_width = CH_WIDTH_5MHZ;
		addbss_param->staContext.ch_width = CH_WIDTH_5MHZ;
	} else if (cds_is_10_mhz_enabled()) {
		addbss_param->ch_width = CH_WIDTH_10MHZ;
		addbss_param->staContext.ch_width = CH_WIDTH_10MHZ;
	}

	msg_buf.type = WMA_ADD_BSS_REQ;
	msg_buf.reserved = 0;
	msg_buf.bodyptr = addbss_param;
	msg_buf.bodyval = 0;
	MTRACE(mac_trace_msg_tx(mac_ctx, session->peSessionId, msg_buf.type));

	pe_debug("Sending WMA_ADD_BSS_REQ...");
	retcode = wma_post_ctrl_msg(mac_ctx, &msg_buf);
	if (QDF_STATUS_SUCCESS != retcode) {
		pe_err("Posting ADD_BSS_REQ to HAL failed, reason=%X",
			retcode);
		qdf_mem_free(addbss_param);
		return eSIR_SME_HAL_SEND_MESSAGE_FAIL;
	}

	return eSIR_SME_SUCCESS;
}

void lim_process_mlm_start_req(struct mac_context *mac_ctx,
			       tLimMlmStartReq *mlm_start_req)
{
	tLimMlmStartCnf mlm_start_cnf;
	struct pe_session *session = NULL;

	if (!mlm_start_req) {
		pe_err("Buffer is Pointing to NULL");
		return;
	}

	session = pe_find_session_by_session_id(mac_ctx,
				mlm_start_req->sessionId);
	if (!session) {
		pe_err("Session Does not exist for given sessionID");
		mlm_start_cnf.resultCode = eSIR_SME_REFUSED;
		goto end;
	}

	if (session->limMlmState != eLIM_MLM_IDLE_STATE) {
		/*
		 * Should not have received Start req in states other than idle.
		 * Return Start confirm with failure code.
		 */
		pe_err("received unexpected MLM_START_REQ in state %X",
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

	/*
	 * Respond immediately to LIM, only if MLME has not been
	 * successfully able to send WMA_ADD_BSS_REQ to HAL.
	 * Else, LIM_MLM_START_CNF will be sent after receiving
	 * WMA_ADD_BSS_RSP from HAL
	 */
	if (eSIR_SME_SUCCESS != mlm_start_cnf.resultCode)
		lim_send_start_bss_confirm(mac_ctx, &mlm_start_cnf);
}

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
static void lim_post_join_set_link_state_callback(struct mac_context *mac,
		void *callback_arg, bool status)
{
	uint8_t chan_num, sec_chan_offset;
	struct session_params *session_cb_param =
					(struct session_params *)callback_arg;
	tLimMlmJoinCnf mlm_join_cnf;

	struct pe_session *session_entry = pe_find_session_by_session_id(mac,
					session_cb_param->session_id);
	if (!session_entry) {
		pe_err("sessionId:%d does not exist",
		       session_cb_param->session_id);
		qdf_mem_free(session_cb_param);
		return;
	}

	qdf_mem_free(session_cb_param);
	pe_debug("Sessionid %d set link state(%d) cb status: %d",
			session_entry->peSessionId, session_entry->limMlmState,
			status);

	if (!status) {
		pe_err("failed to find pe session for session id:%d",
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
	session_entry->pLimMlmReassocRetryReq = NULL;
	pe_debug("[lim_process_mlm_join_req]: suspend link success(%d) "
		"on sessionid: %d setting channel to: %d with ch_width :%d "
		"and maxtxPower: %d", status, session_entry->peSessionId,
		session_entry->currentOperChannel,
		session_entry->ch_width,
		session_entry->maxTxPower);
	lim_set_channel(mac, session_entry->currentOperChannel,
		session_entry->ch_center_freq_seg0,
		session_entry->ch_center_freq_seg1,
		session_entry->ch_width,
		session_entry->maxTxPower,
		session_entry->peSessionId, 0, 0);
	return;

failure:
	MTRACE(mac_trace(mac, TRACE_CODE_MLM_STATE, session_entry->peSessionId,
			 session_entry->limMlmState));
	session_entry->limMlmState = eLIM_MLM_IDLE_STATE;
	mlm_join_cnf.resultCode = eSIR_SME_PEER_CREATE_FAILED;
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
 *   If success, proceed with setting link state to receive the
 *   probe response/beacon from intended AP.
 *   Switch to the APs channel.
 *   On an error case, send the MLM_JOIN_CNF with error status.
 *
 * @Return None
 */
static void
lim_process_mlm_post_join_suspend_link(struct mac_context *mac_ctx,
				       QDF_STATUS status,
				       uint32_t *ctx)
{
	tLimMlmJoinCnf mlm_join_cnf;
	struct pe_session *session = (struct pe_session *) ctx;
	tSirLinkState lnk_state;
	struct session_params *pe_session_param = NULL;

	if (QDF_STATUS_SUCCESS != status) {
		pe_err("Sessionid %d Suspend link(NOTIFY_BSS) failed. Still proceeding with join",
			session->peSessionId);
	}
	lim_deactivate_and_change_timer(mac_ctx, eLIM_JOIN_FAIL_TIMER);

	/* assign appropriate sessionId to the timer object */
	mac_ctx->lim.limTimers.gLimJoinFailureTimer.sessionId =
		session->peSessionId;

	lnk_state = eSIR_LINK_PREASSOC_STATE;
	pe_debug("[lim_process_mlm_join_req]: lnk_state: %d",
		lnk_state);

	pe_session_param = qdf_mem_malloc(sizeof(struct session_params));
	if (!pe_session_param)
		goto error;

	pe_session_param->session_id = session->peSessionId;
	if (lim_set_link_state(mac_ctx, lnk_state,
			session->pLimMlmJoinReq->bssDescription.bssId,
			session->selfMacAddr,
			lim_post_join_set_link_state_callback,
			pe_session_param) != QDF_STATUS_SUCCESS) {
		pe_err("SessionId:%d lim_set_link_state to eSIR_LINK_PREASSOC_STATE Failed!!",
			session->peSessionId);
		lim_print_mac_addr(mac_ctx,
			session->pLimMlmJoinReq->bssDescription.bssId, LOGE);
		mlm_join_cnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
		session->limMlmState = eLIM_MLM_IDLE_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
				 session->peSessionId, session->limMlmState));
		qdf_mem_free(pe_session_param);
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
 * @mlm_join_req:        Pointer to the mlme join request
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
void lim_process_mlm_join_req(struct mac_context *mac_ctx,
			      tLimMlmJoinReq *mlm_join_req)
{
	tLimMlmJoinCnf mlmjoin_cnf;
	uint8_t sessionid;
	struct pe_session *session;

	sessionid = mlm_join_req->sessionId;

	session = pe_find_session_by_session_id(mac_ctx, sessionid);
	if (!session) {
		pe_err("SessionId:%d does not exist", sessionid);
		goto error;
	}

	if (!LIM_IS_AP_ROLE(session) &&
	     ((session->limMlmState == eLIM_MLM_IDLE_STATE) ||
	     (session->limMlmState == eLIM_MLM_JOINED_STATE)) &&
	     (SIR_MAC_GET_ESS
		(mlm_join_req->bssDescription.capabilityInfo) !=
		SIR_MAC_GET_IBSS(mlm_join_req->bssDescription.
			capabilityInfo))) {
		/* Hold onto Join request parameters */

		session->pLimMlmJoinReq = mlm_join_req;
		if (is_lim_session_off_channel(mac_ctx, sessionid)) {
			pe_debug("SessionId:%d LimSession is on OffChannel",
				sessionid);
			/* suspend link */
			pe_debug("Suspend link, sessionid %d is off channel",
				sessionid);
			lim_process_mlm_post_join_suspend_link(mac_ctx,
				QDF_STATUS_SUCCESS, (uint32_t *)session);
		} else {
			pe_debug("No need to Suspend link");
			 /*
			  * No need to Suspend link as LimSession is not
			  * off channel, calling
			  * lim_process_mlm_post_join_suspend_link with
			  * status as SUCCESS.
			  */
			pe_debug("SessionId:%d Join req on current chan",
				sessionid);
			lim_process_mlm_post_join_suspend_link(mac_ctx,
				QDF_STATUS_SUCCESS, (uint32_t *)session);
		}
		return;
	} else {
		/**
		 * Should not have received JOIN req in states other than
		 * Idle state or on AP.
		 * Return join confirm with invalid parameters code.
		 */
		pe_err("Session:%d Unexpected Join req, role %d state %X",
			session->peSessionId, GET_LIM_SYSTEM_ROLE(session),
			session->limMlmState);
		lim_print_mlm_state(mac_ctx, LOGE, session->limMlmState);
	}

error:
	qdf_mem_free(mlm_join_req);
	if (session)
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
static bool lim_is_auth_req_expected(struct mac_context *mac_ctx,
				     struct pe_session *session)
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

	flag = (((LIM_IS_STA_ROLE(session) &&
		 ((session->limMlmState == eLIM_MLM_JOINED_STATE) ||
		  (session->limMlmState ==
					eLIM_MLM_LINK_ESTABLISHED_STATE))) ||
		  (LIM_IS_IBSS_ROLE(session) &&
		  (session->limMlmState ==
					eLIM_MLM_BSS_STARTED_STATE))) &&
		(!IEEE80211_IS_MULTICAST(
			mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr)) &&
		 lim_is_auth_algo_supported(mac_ctx,
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
static bool lim_is_preauth_ctx_exists(struct mac_context *mac_ctx,
				      struct pe_session *session,
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

	fl = (((LIM_IS_STA_ROLE(session)) &&
	       (session->limMlmState == eLIM_MLM_LINK_ESTABLISHED_STATE) &&
	      ((stads) &&
	       (mac_ctx->lim.gpLimMlmAuthReq->authType ==
			stads->mlmStaContext.authType)) &&
	       (!qdf_mem_cmp(mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr,
			curr_bssid, sizeof(tSirMacAddr)))) ||
	      ((preauth_node) &&
	       (preauth_node->authType ==
			mac_ctx->lim.gpLimMlmAuthReq->authType)));

	return fl;
}

#ifdef WLAN_FEATURE_SAE
/**
 * lim_process_mlm_auth_req_sae() - Handle SAE authentication
 * @mac_ctx: global MAC context
 * @session: PE session entry
 *
 * This function is called by lim_process_mlm_auth_req to handle SAE
 * authentication.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS lim_process_mlm_auth_req_sae(struct mac_context *mac_ctx,
		struct pe_session *session)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	struct sir_sae_info *sae_info;
	struct scheduler_msg msg = {0};

	sae_info = qdf_mem_malloc(sizeof(*sae_info));
	if (!sae_info)
		return QDF_STATUS_E_FAILURE;

	sae_info->msg_type = eWNI_SME_TRIGGER_SAE;
	sae_info->msg_len = sizeof(*sae_info);
	sae_info->vdev_id = session->smeSessionId;

	qdf_mem_copy(sae_info->peer_mac_addr.bytes,
		session->bssId,
		QDF_MAC_ADDR_SIZE);

	sae_info->ssid.length = session->ssId.length;
	qdf_mem_copy(sae_info->ssid.ssId,
		session->ssId.ssId,
		session->ssId.length);

	pe_debug("vdev_id %d ssid %.*s "MAC_ADDRESS_STR"",
		sae_info->vdev_id,
		sae_info->ssid.length,
		sae_info->ssid.ssId,
		MAC_ADDR_ARRAY(sae_info->peer_mac_addr.bytes));

	msg.type = eWNI_SME_TRIGGER_SAE;
	msg.bodyptr = sae_info;
	msg.bodyval = 0;

	qdf_status = mac_ctx->lim.sme_msg_callback(mac_ctx, &msg);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pe_err("SAE failed for AUTH frame");
		qdf_mem_free(sae_info);
		return qdf_status;
	}
	session->limMlmState = eLIM_MLM_WT_SAE_AUTH_STATE;

	MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE, session->peSessionId,
		       session->limMlmState));

	mac_ctx->lim.limTimers.sae_auth_timer.sessionId =
					session->peSessionId;

	/* Activate SAE auth timer */
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_TIMER_ACTIVATE,
			 session->peSessionId, eLIM_AUTH_SAE_TIMER));
	if (tx_timer_activate(&mac_ctx->lim.limTimers.sae_auth_timer)
		    != TX_SUCCESS) {
		pe_err("could not start Auth SAE timer");
	}

	return qdf_status;
}
#else
static QDF_STATUS lim_process_mlm_auth_req_sae(struct mac_context *mac_ctx,
		struct pe_session *session)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif


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
static void lim_process_mlm_auth_req(struct mac_context *mac_ctx, uint32_t *msg)
{
	uint32_t num_preauth_ctx;
	tSirMacAddr curr_bssid;
	tSirMacAuthFrameBody auth_frame_body;
	tLimMlmAuthCnf mlm_auth_cnf;
	struct tLimPreAuthNode *preauth_node = NULL;
	uint8_t session_id;
	struct pe_session *session;

	if (!msg) {
		pe_err("Buffer is Pointing to NULL");
		return;
	}

	mac_ctx->lim.gpLimMlmAuthReq = (tLimMlmAuthReq *) msg;
	session_id = mac_ctx->lim.gpLimMlmAuthReq->sessionId;
	session = pe_find_session_by_session_id(mac_ctx, session_id);
	if (!session) {
		pe_err("SessionId:%d does not exist", session_id);
		qdf_mem_free(msg);
		mac_ctx->lim.gpLimMlmAuthReq = NULL;
		return;
	}

	pe_debug("Process Auth Req sessionID %d Systemrole %d"
		       "mlmstate %d from: " MAC_ADDRESS_STR
		       " with authtype %d", session_id,
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
		pe_debug("Already have pre-auth context with peer: "
		    MAC_ADDRESS_STR,
		    MAC_ADDR_ARRAY(mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr));
		mlm_auth_cnf.resultCode = (tSirResultCodes)
						eSIR_MAC_SUCCESS_STATUS;
		goto end;
	} else {
		num_preauth_ctx = mac_ctx->mlme_cfg->lfr.max_num_pre_auth;
		if (mac_ctx->lim.gLimNumPreAuthContexts == num_preauth_ctx) {
			pe_warn("Number of pre-auth reached max limit");
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

	if ((mac_ctx->lim.gpLimMlmAuthReq->authType == eSIR_AUTH_TYPE_SAE) &&
	     !session->sae_pmk_cached) {
		if (lim_process_mlm_auth_req_sae(mac_ctx, session) !=
					QDF_STATUS_SUCCESS) {
			mlm_auth_cnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
			goto end;
		} else {
			pe_debug("lim_process_mlm_auth_req_sae is successful");
			auth_frame_body.authAlgoNumber = eSIR_AUTH_TYPE_SAE;
			auth_frame_body.authTransactionSeqNumber =
							SIR_MAC_AUTH_FRAME_1;
			auth_frame_body.authStatusCode = 0;
			host_log_wlan_auth_info(auth_frame_body.authAlgoNumber,
				auth_frame_body.authTransactionSeqNumber,
				auth_frame_body.authStatusCode);

			return;
		}
	} else
		session->limMlmState = eLIM_MLM_WT_AUTH_FRAME2_STATE;

	MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE, session->peSessionId,
		       session->limMlmState));

	/* Mark auth algo as open when auth type is SAE and PMK is cached */
	if ((mac_ctx->lim.gpLimMlmAuthReq->authType == eSIR_AUTH_TYPE_SAE) &&
	   session->sae_pmk_cached) {
		auth_frame_body.authAlgoNumber = eSIR_OPEN_SYSTEM;
	} else {
		auth_frame_body.authAlgoNumber =
		(uint8_t) mac_ctx->lim.gpLimMlmAuthReq->authType;
	}

	/* Prepare & send Authentication frame */
	auth_frame_body.authTransactionSeqNumber = SIR_MAC_AUTH_FRAME_1;
	auth_frame_body.authStatusCode = 0;
	host_log_wlan_auth_info(auth_frame_body.authAlgoNumber,
				auth_frame_body.authTransactionSeqNumber,
				auth_frame_body.authStatusCode);
	mac_ctx->auth_ack_status = LIM_AUTH_ACK_NOT_RCD;
	lim_send_auth_mgmt_frame(mac_ctx,
		&auth_frame_body, mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr,
		LIM_NO_WEP_IN_FC, session);

	/* assign appropriate session_id to the timer object */
	mac_ctx->lim.limTimers.gLimAuthFailureTimer.sessionId = session_id;

	/* assign appropriate sessionId to the timer object */
	 mac_ctx->lim.limTimers.g_lim_periodic_auth_retry_timer.sessionId =
								  session_id;
	 lim_deactivate_and_change_timer(mac_ctx, eLIM_AUTH_RETRY_TIMER);
	/* Activate Auth failure timer */
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_TIMER_ACTIVATE,
			 session->peSessionId, eLIM_AUTH_FAIL_TIMER));
	 lim_deactivate_and_change_timer(mac_ctx, eLIM_AUTH_FAIL_TIMER);
	if (tx_timer_activate(&mac_ctx->lim.limTimers.gLimAuthFailureTimer)
	    != TX_SUCCESS) {
		pe_err("could not start Auth failure timer");
		/* Cleanup as if auth timer expired */
		lim_process_auth_failure_timeout(mac_ctx);
	} else {
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_TIMER_ACTIVATE,
			   session->peSessionId, eLIM_AUTH_RETRY_TIMER));
		/* Activate Auth Retry timer */
		if (tx_timer_activate
		    (&mac_ctx->lim.limTimers.g_lim_periodic_auth_retry_timer)
							      != TX_SUCCESS)
			pe_err("could not activate Auth Retry timer");
	}

	return;
end:
	qdf_mem_copy((uint8_t *) &mlm_auth_cnf.peerMacAddr,
		     (uint8_t *) &mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr,
		     sizeof(tSirMacAddr));

	mlm_auth_cnf.authType = mac_ctx->lim.gpLimMlmAuthReq->authType;
	mlm_auth_cnf.sessionId = session_id;

	qdf_mem_free(mac_ctx->lim.gpLimMlmAuthReq);
	mac_ctx->lim.gpLimMlmAuthReq = NULL;
	pe_debug("SessionId:%d LimPostSme LIM_MLM_AUTH_CNF",
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

static void lim_process_mlm_assoc_req(struct mac_context *mac_ctx, uint32_t *msg_buf)
{
	tSirMacAddr curr_bssId;
	tLimMlmAssocReq *mlm_assoc_req;
	tLimMlmAssocCnf mlm_assoc_cnf;
	struct pe_session *session_entry;

	if (!msg_buf) {
		pe_err("Buffer is Pointing to NULL");
		return;
	}

	mlm_assoc_req = (tLimMlmAssocReq *) msg_buf;
	session_entry = pe_find_session_by_session_id(mac_ctx,
						      mlm_assoc_req->sessionId);
	if (!session_entry) {
		pe_err("SessionId:%d Session Does not exist",
			mlm_assoc_req->sessionId);
		qdf_mem_free(mlm_assoc_req);
		return;
	}

	sir_copy_mac_addr(curr_bssId, session_entry->bssId);

	if (!(!LIM_IS_AP_ROLE(session_entry) &&
		(session_entry->limMlmState == eLIM_MLM_AUTHENTICATED_STATE ||
		 session_entry->limMlmState == eLIM_MLM_JOINED_STATE) &&
		(!qdf_mem_cmp(mlm_assoc_req->peerMacAddr,
		 curr_bssId, sizeof(tSirMacAddr))))) {
		/*
		 * Received Association request either in invalid state
		 * or to a peer MAC entity whose address is different
		 * from one that STA is currently joined with or on AP.
		 * Return Assoc confirm with Invalid parameters code.
		 */
		pe_warn("received unexpected MLM_ASSOC_CNF in state %X for role=%d, MAC addr= "
			   MAC_ADDRESS_STR, session_entry->limMlmState,
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
		session_entry->pmfComebackTimerInfo.lim_prev_mlm_state =
			session_entry->limPrevMlmState;
		session_entry->pmfComebackTimerInfo.lim_mlm_state =
			session_entry->limMlmState;
	}
#endif /* WLAN_FEATURE_11W */

	session_entry->limPrevMlmState = session_entry->limMlmState;
	session_entry->limMlmState = eLIM_MLM_WT_ASSOC_RSP_STATE;
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
			 session_entry->peSessionId,
			 session_entry->limMlmState));
	pe_debug("SessionId:%d Sending Assoc_Req Frame",
		session_entry->peSessionId);

	/* Prepare and send Association request frame */
	lim_send_assoc_req_mgmt_frame(mac_ctx, mlm_assoc_req, session_entry);

	/* Start association failure timer */
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_TIMER_ACTIVATE,
			 session_entry->peSessionId, eLIM_ASSOC_FAIL_TIMER));
	if (tx_timer_activate(&mac_ctx->lim.limTimers.gLimAssocFailureTimer)
	    != TX_SUCCESS) {
		pe_warn("SessionId:%d couldn't start Assoc failure timer",
			session_entry->peSessionId);
		/* Cleanup as if assoc timer expired */
		lim_process_assoc_failure_timeout(mac_ctx, LIM_ASSOC);
	}

	return;
end:
	/* Update PE session Id */
	mlm_assoc_cnf.sessionId = mlm_assoc_req->sessionId;
	/* Free up buffer allocated for assocReq */
	qdf_mem_free(mlm_assoc_req);
	lim_post_sme_message(mac_ctx, LIM_MLM_ASSOC_CNF,
			     (uint32_t *) &mlm_assoc_cnf);
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
lim_process_mlm_disassoc_req_ntf(struct mac_context *mac_ctx,
				 QDF_STATUS suspend_status, uint32_t *msg)
{
	uint16_t aid;
	struct qdf_mac_addr curr_bssid;
	tpDphHashNode stads;
	tLimMlmDisassocReq *mlm_disassocreq;
	tLimMlmDisassocCnf mlm_disassoccnf;
	struct pe_session *session;
	extern bool send_disassoc_frame;
	tLimMlmStates mlm_state;
	struct disassoc_rsp *sme_disassoc_rsp;

	if (QDF_STATUS_SUCCESS != suspend_status)
		pe_err("Suspend Status is not success %X",
			suspend_status);

	mlm_disassocreq = (tLimMlmDisassocReq *) msg;

	session = pe_find_session_by_session_id(mac_ctx,
				mlm_disassocreq->sessionId);
	if (!session) {
		pe_err("session does not exist for given sessionId %d",
			mlm_disassocreq->sessionId);
		mlm_disassoccnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		goto end;
	}

	pe_debug("Process DisAssoc Req on sessionID %d Systemrole %d"
		   "mlmstate %d from: " MAC_ADDRESS_STR,
		mlm_disassocreq->sessionId, GET_LIM_SYSTEM_ROLE(session),
		session->limMlmState,
		MAC_ADDR_ARRAY(mlm_disassocreq->peer_macaddr.bytes));

	qdf_mem_copy(curr_bssid.bytes, session->bssId, QDF_MAC_ADDR_SIZE);

	switch (GET_LIM_SYSTEM_ROLE(session)) {
	case eLIM_STA_ROLE:
		if (!qdf_is_macaddr_equal(&mlm_disassocreq->peer_macaddr,
				     &curr_bssid)) {
			pe_warn("received MLM_DISASSOC_REQ with invalid BSS id");
			lim_print_mac_addr(mac_ctx,
				mlm_disassocreq->peer_macaddr.bytes, LOGW);

			/*
			 * Disassociation response due to host triggered
			 * disassociation
			 */
			sme_disassoc_rsp =
				qdf_mem_malloc(sizeof(*sme_disassoc_rsp));
			if (!sme_disassoc_rsp) {
				qdf_mem_free(mlm_disassocreq);
				return;
			}

			pe_debug("send disassoc rsp with ret code %d for" MAC_ADDRESS_STR,
				eSIR_SME_DEAUTH_STATUS,
				MAC_ADDR_ARRAY(
					mlm_disassocreq->peer_macaddr.bytes));

			sme_disassoc_rsp->messageType = eWNI_SME_DISASSOC_RSP;
			sme_disassoc_rsp->length = sizeof(*sme_disassoc_rsp);
			sme_disassoc_rsp->sessionId =
					mlm_disassocreq->sessionId;
			sme_disassoc_rsp->statusCode = eSIR_SME_DEAUTH_STATUS;

			qdf_copy_macaddr(&sme_disassoc_rsp->peer_macaddr,
					 &mlm_disassocreq->peer_macaddr);
			msg = (uint32_t *)sme_disassoc_rsp;

			lim_send_sme_disassoc_deauth_ntf(mac_ctx,
					QDF_STATUS_SUCCESS, msg);
			qdf_mem_free(mlm_disassocreq);
			return;

		}
		break;
	case eLIM_STA_IN_IBSS_ROLE:
		break;
	case eLIM_AP_ROLE:
	case eLIM_P2P_DEVICE_GO:
		if (true ==
			 mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running) {
			pe_err("CAC timer is running, drop disassoc from going out");
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
	stads = dph_lookup_hash_entry(mac_ctx,
				      mlm_disassocreq->peer_macaddr.bytes,
				      &aid, &session->dph.dphHashTable);
	if (stads)
		mlm_state = stads->mlmStaContext.mlmState;

	if ((!stads) ||
	    (stads &&
	     ((mlm_state != eLIM_MLM_LINK_ESTABLISHED_STATE) &&
	      (mlm_state != eLIM_MLM_WT_ASSOC_CNF_STATE) &&
	      (mlm_state != eLIM_MLM_ASSOCIATED_STATE)))) {
		/*
		 * Received LIM_MLM_DISASSOC_REQ for STA that does not
		 * have context or in some transit state.
		 */
		pe_warn("Invalid MLM_DISASSOC_REQ, Addr= " MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(mlm_disassocreq->peer_macaddr.bytes));
		if (stads)
			pe_err("Sta MlmState: %d", stads->mlmStaContext.mlmState);

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
		if (mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDisassocReq) {
			pe_err("pMlmDisassocReq is not NULL, freeing");
			qdf_mem_free(mac_ctx->lim.limDisassocDeauthCnfReq.
				     pMlmDisassocReq);
		}
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
			mlm_disassocreq->peer_macaddr.bytes, session, true);
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
		if (QDF_STATUS_SUCCESS !=
		    lim_cleanup_rx_path(mac_ctx, stads, session)) {
			mlm_disassoccnf.resultCode =
				eSIR_SME_RESOURCES_UNAVAILABLE;
			goto end;
		}
		/* Free up buffer allocated for mlmDisassocReq */
		qdf_mem_free(mlm_disassocreq);
	}

	return;

end:
	qdf_mem_copy((uint8_t *) &mlm_disassoccnf.peerMacAddr,
		     (uint8_t *) mlm_disassocreq->peer_macaddr.bytes,
		     QDF_MAC_ADDR_SIZE);
	mlm_disassoccnf.aid = mlm_disassocreq->aid;
	mlm_disassoccnf.disassocTrigger = mlm_disassocreq->disassocTrigger;

	/* Update PE session ID */
	mlm_disassoccnf.sessionId = mlm_disassocreq->sessionId;

	/* Free up buffer allocated for mlmDisassocReq */
	qdf_mem_free(mlm_disassocreq);

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
bool lim_check_disassoc_deauth_ack_pending(struct mac_context *mac_ctx,
					   uint8_t *sta_mac)
{
	tLimMlmDisassocReq *disassoc_req;
	tLimMlmDeauthReq *deauth_req;

	disassoc_req = mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDisassocReq;
	deauth_req = mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDeauthReq;
	if ((disassoc_req && (!qdf_mem_cmp((uint8_t *) sta_mac,
			      (uint8_t *) &disassoc_req->peer_macaddr.bytes,
			       QDF_MAC_ADDR_SIZE))) ||
	    (deauth_req && (!qdf_mem_cmp((uint8_t *) sta_mac,
			      (uint8_t *) &deauth_req->peer_macaddr.bytes,
			       QDF_MAC_ADDR_SIZE)))) {
		pe_debug("Disassoc/Deauth ack pending");
		return true;
	} else {
		pe_debug("Disassoc/Deauth Ack not pending");
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
void lim_clean_up_disassoc_deauth_req(struct mac_context *mac_ctx,
				      uint8_t *sta_mac, bool clean_rx_path)
{
	tLimMlmDisassocReq *mlm_disassoc_req;
	tLimMlmDeauthReq *mlm_deauth_req;

	mlm_disassoc_req = mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDisassocReq;
	if (mlm_disassoc_req &&
	    (!qdf_mem_cmp((uint8_t *) sta_mac,
			     (uint8_t *) &mlm_disassoc_req->peer_macaddr.bytes,
			     QDF_MAC_ADDR_SIZE))) {
		if (clean_rx_path) {
			lim_process_disassoc_ack_timeout(mac_ctx);
		} else {
			if (tx_timer_running(
				&mac_ctx->lim.limTimers.gLimDisassocAckTimer)) {
				lim_deactivate_and_change_timer(mac_ctx,
						eLIM_DISASSOC_ACK_TIMER);
			}
			qdf_mem_free(mlm_disassoc_req);
			mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDisassocReq =
				NULL;
		}
	}

	mlm_deauth_req = mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDeauthReq;
	if (mlm_deauth_req &&
	    (!qdf_mem_cmp((uint8_t *) sta_mac,
			     (uint8_t *) &mlm_deauth_req->peer_macaddr.bytes,
			     QDF_MAC_ADDR_SIZE))) {
		if (clean_rx_path) {
			lim_process_deauth_ack_timeout(mac_ctx);
		} else {
			if (tx_timer_running(
				&mac_ctx->lim.limTimers.gLimDeauthAckTimer)) {
				lim_deactivate_and_change_timer(mac_ctx,
						eLIM_DEAUTH_ACK_TIMER);
			}
			qdf_mem_free(mlm_deauth_req);
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
void lim_process_disassoc_ack_timeout(struct mac_context *mac_ctx)
{
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
lim_process_mlm_disassoc_req(struct mac_context *mac_ctx, uint32_t *msg_buf)
{
	tLimMlmDisassocReq *mlm_disassoc_req;

	if (!msg_buf) {
		pe_err("Buffer is Pointing to NULL");
		return;
	}

	mlm_disassoc_req = (tLimMlmDisassocReq *) msg_buf;
	pe_debug("Process disassoc req, sessionID %d from: "MAC_ADDRESS_STR,
		mlm_disassoc_req->sessionId,
		MAC_ADDR_ARRAY(mlm_disassoc_req->peer_macaddr.bytes));

	lim_process_mlm_disassoc_req_ntf(mac_ctx, QDF_STATUS_SUCCESS,
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
lim_process_mlm_deauth_req_ntf(struct mac_context *mac_ctx,
			       QDF_STATUS suspend_status, uint32_t *msg_buf)
{
	uint16_t aid;
	tSirMacAddr curr_bssId;
	tpDphHashNode sta_ds;
	struct tLimPreAuthNode *auth_node;
	tLimMlmDeauthReq *mlm_deauth_req;
	tLimMlmDeauthCnf mlm_deauth_cnf;
	struct pe_session *session;
	struct deauth_rsp *sme_deauth_rsp;

	if (QDF_STATUS_SUCCESS != suspend_status)
		pe_err("Suspend Status is not success %X",
			suspend_status);

	mlm_deauth_req = (tLimMlmDeauthReq *) msg_buf;
	session = pe_find_session_by_session_id(mac_ctx,
				mlm_deauth_req->sessionId);
	if (!session) {
		pe_err("session does not exist for given sessionId %d",
			mlm_deauth_req->sessionId);
		qdf_mem_free(mlm_deauth_req);
		return;
	}
	pe_debug("Process Deauth Req on sessionID %d Systemrole %d"
		       "mlmstate %d from: " MAC_ADDRESS_STR,
		mlm_deauth_req->sessionId,
		GET_LIM_SYSTEM_ROLE(session),
		session->limMlmState,
		MAC_ADDR_ARRAY(mlm_deauth_req->peer_macaddr.bytes));
	sir_copy_mac_addr(curr_bssId, session->bssId);

	switch (GET_LIM_SYSTEM_ROLE(session)) {
	case eLIM_STA_ROLE:
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
			if (qdf_mem_cmp(mlm_deauth_req->peer_macaddr.bytes,
					curr_bssId, QDF_MAC_ADDR_SIZE)) {
				pe_err("received MLM_DEAUTH_REQ with invalid BSS id "
					   "Peer MAC: "MAC_ADDRESS_STR
					   " CFG BSSID Addr : "MAC_ADDRESS_STR,
					MAC_ADDR_ARRAY(
						mlm_deauth_req->peer_macaddr.bytes),
					MAC_ADDR_ARRAY(curr_bssId));
				/*
				 * Deauthentication response to host triggered
				 * deauthentication
				 */
				sme_deauth_rsp =
				    qdf_mem_malloc(sizeof(*sme_deauth_rsp));
				if (!sme_deauth_rsp) {
					qdf_mem_free(mlm_deauth_req);
					return;
				}

				pe_debug("send deauth rsp with ret code %d for" MAC_ADDRESS_STR,
					eSIR_SME_DEAUTH_STATUS,
					MAC_ADDR_ARRAY(
					  mlm_deauth_req->peer_macaddr.bytes));

				sme_deauth_rsp->messageType =
						eWNI_SME_DEAUTH_RSP;
				sme_deauth_rsp->length =
						sizeof(*sme_deauth_rsp);
				sme_deauth_rsp->statusCode =
						eSIR_SME_DEAUTH_STATUS;
				sme_deauth_rsp->sessionId =
						mlm_deauth_req->sessionId;

				qdf_mem_copy(sme_deauth_rsp->peer_macaddr.bytes,
					     mlm_deauth_req->peer_macaddr.bytes,
					     QDF_MAC_ADDR_SIZE);

				msg_buf = (uint32_t *)sme_deauth_rsp;

				lim_send_sme_disassoc_deauth_ntf(mac_ctx,
						QDF_STATUS_SUCCESS, msg_buf);
				qdf_mem_free(mlm_deauth_req);
				return;
			}

			if ((session->limMlmState ==
			     eLIM_MLM_AUTHENTICATED_STATE) ||
			    (session->limMlmState ==
			     eLIM_MLM_WT_ASSOC_RSP_STATE)) {
				/* Send deauth frame to peer entity */
				lim_send_deauth_mgmt_frame(mac_ctx,
					mlm_deauth_req->reasonCode,
					mlm_deauth_req->peer_macaddr.bytes,
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
			pe_warn("received MLM_DEAUTH_REQ with in state %d for peer "
				   MAC_ADDRESS_STR,
				session->limMlmState,
				MAC_ADDR_ARRAY(
					mlm_deauth_req->peer_macaddr.bytes));
			lim_print_mlm_state(mac_ctx, LOGW,
					    session->limMlmState);
			/* Prepare and Send LIM_MLM_DEAUTH_CNF */
			mlm_deauth_cnf.resultCode =
				eSIR_SME_STA_NOT_AUTHENTICATED;

			goto end;
		}
		break;
	case eLIM_STA_IN_IBSS_ROLE:
		pe_err("received MLM_DEAUTH_REQ IBSS Mode");
		mlm_deauth_cnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		goto end;
	case eLIM_AP_ROLE:
	case eLIM_P2P_DEVICE_GO:
		if (true ==
			mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running) {
			pe_err("CAC timer is running, drop disassoc from going out");
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
	sta_ds = dph_lookup_hash_entry(mac_ctx,
				       mlm_deauth_req->peer_macaddr.bytes,
				       &aid, &session->dph.dphHashTable);

	if (!sta_ds) {
		/* Check if there exists pre-auth context for this STA */
		auth_node = lim_search_pre_auth_list(mac_ctx,
					mlm_deauth_req->peer_macaddr.bytes);
		if (!auth_node) {
			/*
			 * Received DEAUTH REQ for a STA that is neither
			 * Associated nor Pre-authenticated. Log error,
			 * Prepare and Send LIM_MLM_DEAUTH_CNF
			 */
			pe_warn("received MLM_DEAUTH_REQ in mlme state %d for STA that "
				   "does not have context, Addr="
				   MAC_ADDRESS_STR,
				session->limMlmState,
				MAC_ADDR_ARRAY(
					mlm_deauth_req->peer_macaddr.bytes));
			mlm_deauth_cnf.resultCode =
				eSIR_SME_STA_NOT_AUTHENTICATED;
		} else {
			mlm_deauth_cnf.resultCode = eSIR_SME_SUCCESS;
			/* Delete STA from pre-auth STA list */
			lim_delete_pre_auth_node(mac_ctx,
					 mlm_deauth_req->peer_macaddr.bytes);
			/* Send Deauthentication frame to peer entity */
			lim_send_deauth_mgmt_frame(mac_ctx,
					   mlm_deauth_req->reasonCode,
					   mlm_deauth_req->peer_macaddr.bytes,
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
		pe_warn("Invalid MLM_DEAUTH_REQ, Addr="MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(mlm_deauth_req->peer_macaddr.bytes));
		/* Prepare and Send LIM_MLM_DEAUTH_CNF */
		mlm_deauth_cnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		goto end;
	}
	/* sta_ds->mlmStaContext.rxPurgeReq     = 1; */
	sta_ds->mlmStaContext.disassocReason = (tSirMacReasonCodes)
					       mlm_deauth_req->reasonCode;
	sta_ds->mlmStaContext.cleanupTrigger = mlm_deauth_req->deauthTrigger;

	if (mac_ctx->lim.limDisassocDeauthCnfReq.pMlmDeauthReq) {
		pe_err("pMlmDeauthReq is not NULL, freeing");
		qdf_mem_free(mac_ctx->lim.limDisassocDeauthCnfReq.
			     pMlmDeauthReq);
	}
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
				   mlm_deauth_req->peer_macaddr.bytes,
				   session, true);
	return;
end:
	qdf_copy_macaddr(&mlm_deauth_cnf.peer_macaddr,
			 &mlm_deauth_req->peer_macaddr);
	mlm_deauth_cnf.deauthTrigger = mlm_deauth_req->deauthTrigger;
	mlm_deauth_cnf.aid = mlm_deauth_req->aid;
	mlm_deauth_cnf.sessionId = mlm_deauth_req->sessionId;

	/* Free up buffer allocated for mlmDeauthReq */
	qdf_mem_free(mlm_deauth_req);
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
void lim_process_deauth_ack_timeout(struct mac_context *mac_ctx)
{
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
void lim_process_mlm_deauth_req(struct mac_context *mac_ctx, uint32_t *msg_buf)
{
	tLimMlmDeauthReq *mlm_deauth_req;
	struct pe_session *session;

	if (!msg_buf) {
		pe_err("Buffer is Pointing to NULL");
		return;
	}

	mlm_deauth_req = (tLimMlmDeauthReq *) msg_buf;
	pe_debug("Process Deauth Req on sessionID %d from: "
		   MAC_ADDRESS_STR,
		mlm_deauth_req->sessionId,
		MAC_ADDR_ARRAY(mlm_deauth_req->peer_macaddr.bytes));

	session = pe_find_session_by_session_id(mac_ctx,
				mlm_deauth_req->sessionId);
	if (!session) {
		pe_err("session does not exist for given sessionId %d",
			mlm_deauth_req->sessionId);
		qdf_mem_free(mlm_deauth_req);
		return;
	}
	lim_process_mlm_deauth_req_ntf(mac_ctx, QDF_STATUS_SUCCESS,
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
lim_process_mlm_set_keys_req(struct mac_context *mac_ctx, uint32_t *msg_buf)
{
	uint16_t aid;
	uint16_t sta_idx = 0;
	uint32_t default_key_id = 0;
	struct qdf_mac_addr curr_bssid;
	tpDphHashNode sta_ds;
	tLimMlmSetKeysReq *mlm_set_keys_req;
	tLimMlmSetKeysCnf mlm_set_keys_cnf;
	struct pe_session *session;

	if (!msg_buf) {
		pe_err("Buffer is Pointing to NULL");
		return;
	}

	mlm_set_keys_req = (tLimMlmSetKeysReq *) msg_buf;
	if (mac_ctx->lim.gpLimMlmSetKeysReq) {
		qdf_mem_zero(mac_ctx->lim.gpLimMlmSetKeysReq,
			     sizeof(*mlm_set_keys_req));
		qdf_mem_free(mac_ctx->lim.gpLimMlmSetKeysReq);
		mac_ctx->lim.gpLimMlmSetKeysReq = NULL;
	}
	/* Hold onto the SetKeys request parameters */
	mac_ctx->lim.gpLimMlmSetKeysReq = (void *)mlm_set_keys_req;
	session = pe_find_session_by_session_id(mac_ctx,
				mlm_set_keys_req->sessionId);
	if (!session) {
		pe_err("session does not exist for given sessionId");
		qdf_mem_zero(mlm_set_keys_req->key,
			     sizeof(mlm_set_keys_req->key));
		mlm_set_keys_req->numKeys = 0;
		qdf_mem_free(mlm_set_keys_req);
		mac_ctx->lim.gpLimMlmSetKeysReq = NULL;
		return;
	}

	pe_debug("Received MLM_SETKEYS_REQ with parameters:"
		   "AID [%d], ED Type [%d], # Keys [%d] & Peer MAC Addr - ",
		mlm_set_keys_req->aid, mlm_set_keys_req->edType,
		mlm_set_keys_req->numKeys);
	lim_print_mac_addr(mac_ctx, mlm_set_keys_req->peer_macaddr.bytes, LOGD);
	qdf_mem_copy(curr_bssid.bytes, session->bssId, QDF_MAC_ADDR_SIZE);

	switch (GET_LIM_SYSTEM_ROLE(session)) {
	case eLIM_STA_ROLE:
		/*
		 * In case of TDLS, peerMac address need not be BssId. Skip this
		 * check if TDLS is enabled.
		 */
#ifndef FEATURE_WLAN_TDLS
		if ((!qdf_is_macaddr_broadcast(
				&mlm_set_keys_req->peer_macaddr)) &&
		    (!qdf_is_macaddr_equal(&mlm_set_keys_req->peer_macaddr,
					   &curr_bssid))) {
			pe_debug("Received MLM_SETKEYS_REQ with invalid BSSID"
				MAC_ADDRESS_STR,
				MAC_ADDR_ARRAY(mlm_set_keys_req->
						peer_macaddr.bytes));
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
	if (qdf_is_macaddr_broadcast(&mlm_set_keys_req->peer_macaddr)) {
		pe_debug("Trying to set Group Keys...%d",
			mlm_set_keys_req->sessionId);
		/*
		 * When trying to set Group Keys for any security mode other
		 * than WEP, use the STA Index corresponding to the AP...
		 */
		switch (mlm_set_keys_req->edType) {
		case eSIR_ED_CCMP:
		case eSIR_ED_GCMP:
		case eSIR_ED_GCMP_256:
#ifdef WLAN_FEATURE_11W
		case eSIR_ED_AES_128_CMAC:
		case eSIR_ED_AES_GMAC_128:
		case eSIR_ED_AES_GMAC_256:
#endif
			sta_idx = session->staId;
			break;
		default:
			break;
		}
	} else {
		pe_debug("Trying to set Unicast Keys...");
		/*
		 * Check if there exists a context for the
		 * peer entity for which keys need to be set.
		 */
		sta_ds = dph_lookup_hash_entry(mac_ctx,
				mlm_set_keys_req->peer_macaddr.bytes, &aid,
				&session->dph.dphHashTable);
		if ((!sta_ds) ||
		    ((sta_ds->mlmStaContext.mlmState !=
		    eLIM_MLM_LINK_ESTABLISHED_STATE) &&
		    !LIM_IS_AP_ROLE(session))) {
			/*
			 * Received LIM_MLM_SETKEYS_REQ for STA that does not
			 * have context or in some transit state.
			 */
			pe_debug("Invalid MLM_SETKEYS_REQ, Addr = "
				   MAC_ADDRESS_STR,
				MAC_ADDR_ARRAY(mlm_set_keys_req->
						peer_macaddr.bytes));
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
	pe_debug("Trying to set keys for STA Index [%d], using default_key_id [%d]",
		sta_idx, default_key_id);

	if (qdf_is_macaddr_broadcast(&mlm_set_keys_req->peer_macaddr)) {
		session->limPrevMlmState = session->limMlmState;
		session->limMlmState = eLIM_MLM_WT_SET_BSS_KEY_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
				 session->peSessionId, session->limMlmState));
		pe_debug("Trying to set Group Keys...%d",
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

void lim_process_join_failure_timeout(struct mac_context *mac_ctx)
{
	tLimMlmJoinCnf mlm_join_cnf;
	uint32_t len;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	host_log_rssi_pkt_type *rssi_log = NULL;
#endif
	struct pe_session *session;

	session = pe_find_session_by_session_id(mac_ctx,
			mac_ctx->lim.limTimers.gLimJoinFailureTimer.sessionId);
	if (!session) {
		pe_err("Session Does not exist for given sessionID");
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
		pe_err("Join Failure Timeout, In eLIM_MLM_WT_JOIN_BEACON_STATE session:%d "
			   MAC_ADDRESS_STR,
			session->peSessionId, MAC_ADDR_ARRAY(session->bssId));

		mlm_join_cnf.resultCode = eSIR_SME_JOIN_TIMEOUT_RESULT_CODE;
		mlm_join_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
		session->limMlmState = eLIM_MLM_IDLE_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
				 session->peSessionId, session->limMlmState));
		/* Update PE session Id */
		mlm_join_cnf.sessionId = session->peSessionId;
		/* Freeup buffer allocated to join request */
		if (session->pLimMlmJoinReq) {
			qdf_mem_free(session->pLimMlmJoinReq);
			session->pLimMlmJoinReq = NULL;
		}
		lim_post_sme_message(mac_ctx, LIM_MLM_JOIN_CNF,
				     (uint32_t *) &mlm_join_cnf);
		return;
	} else {
		pe_warn("received unexpected JOIN failure timeout in state %X",
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
static void lim_process_periodic_join_probe_req_timer(struct mac_context *mac_ctx)
{
	struct pe_session *session;
	tSirMacSSid ssid;

	session = pe_find_session_by_session_id(mac_ctx,
		mac_ctx->lim.limTimers.gLimPeriodicJoinProbeReqTimer.sessionId);
	if (!session) {
		pe_err("session does not exist for given SessionId: %d",
			mac_ctx->lim.limTimers.gLimPeriodicJoinProbeReqTimer.
			sessionId);
		return;
	}

	if ((true ==
	    tx_timer_running(&mac_ctx->lim.limTimers.gLimJoinFailureTimer))
		&& (session->limMlmState == eLIM_MLM_WT_JOIN_BEACON_STATE)) {
		qdf_mem_copy(ssid.ssId, session->ssId.ssId,
			     session->ssId.length);
		ssid.length = session->ssId.length;
		lim_send_probe_req_mgmt_frame(mac_ctx, &ssid,
			session->pLimMlmJoinReq->bssDescription.bssId,
			session->currentOperChannel /*chanNum */,
			session->selfMacAddr, session->dot11mode,
			&session->pLimJoinReq->addIEScan.length,
			session->pLimJoinReq->addIEScan.addIEdata);
		lim_deactivate_and_change_timer(mac_ctx,
				eLIM_PERIODIC_JOIN_PROBE_REQ_TIMER);
		/* Activate Join Periodic Probe Req timer */
		if (tx_timer_activate(
		    &mac_ctx->lim.limTimers.gLimPeriodicJoinProbeReqTimer) !=
		    TX_SUCCESS) {
			pe_warn("could not activate Periodic Join req failure timer");
			return;
		}
	}
}

/**
 * lim_process_auth_retry_timer()- function to Retry Auth
 * @mac_ctx:pointer to global mac
 *
 * Return: void
 */

static void lim_process_auth_retry_timer(struct mac_context *mac_ctx)
{
	struct pe_session * session_entry;

	session_entry =
	  pe_find_session_by_session_id(mac_ctx,
	  mac_ctx->lim.limTimers.g_lim_periodic_auth_retry_timer.sessionId);
	if (!session_entry) {
		pe_err("session does not exist for given SessionId: %d",
		  mac_ctx->lim.limTimers.
			g_lim_periodic_auth_retry_timer.sessionId);
		return;
	}

	if (tx_timer_running(&mac_ctx->lim.limTimers.gLimAuthFailureTimer) &&
	     (session_entry->limMlmState == eLIM_MLM_WT_AUTH_FRAME2_STATE) &&
	     (LIM_AUTH_ACK_RCD_SUCCESS != mac_ctx->auth_ack_status)) {
		tSirMacAuthFrameBody    auth_frame;

		/*
		 * Send the auth retry only in case we have received ack failure
		 * else just restart the retry timer.
		 */
		if (LIM_AUTH_ACK_RCD_FAILURE == mac_ctx->auth_ack_status) {
			/* Prepare & send Authentication frame */
			auth_frame.authAlgoNumber =
			    (uint8_t) mac_ctx->lim.gpLimMlmAuthReq->authType;
			auth_frame.authTransactionSeqNumber =
						SIR_MAC_AUTH_FRAME_1;
			auth_frame.authStatusCode = 0;
			pe_debug("Retry Auth");
			mac_ctx->auth_ack_status = LIM_AUTH_ACK_NOT_RCD;
			lim_increase_fils_sequence_number(session_entry);
			lim_send_auth_mgmt_frame(mac_ctx,
				&auth_frame,
				mac_ctx->lim.gpLimMlmAuthReq->peerMacAddr,
				LIM_NO_WEP_IN_FC, session_entry);
		}

		lim_deactivate_and_change_timer(mac_ctx, eLIM_AUTH_RETRY_TIMER);

		/* Activate Auth Retry timer */
		if (tx_timer_activate
		     (&mac_ctx->lim.limTimers.g_lim_periodic_auth_retry_timer)
			 != TX_SUCCESS) {
			pe_err("could not activate Auth Retry failure timer");
			return;
		}
	}
	return;
} /*** lim_process_auth_retry_timer() ***/

void lim_process_auth_failure_timeout(struct mac_context *mac_ctx)
{
	/* fetch the pe_session based on the sessionId */
	struct pe_session *session;
	uint32_t val;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	host_log_rssi_pkt_type *rssi_log = NULL;
#endif

	session = pe_find_session_by_session_id(mac_ctx,
			mac_ctx->lim.limTimers.gLimAuthFailureTimer.sessionId);
	if (!session) {
		pe_err("Session Does not exist for given sessionID");
		return;
	}

	pe_warn("received AUTH failure timeout in sessionid %d "
		   "limMlmstate %X limSmeState %X",
		session->peSessionId, session->limMlmState,
		session->limSmeState);
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_AUTH_TIMEOUT, session,
				0, AUTH_FAILURE_TIMEOUT);

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
		if (QDF_P2P_CLIENT_MODE == session->pePersona &&
		    session->defaultAuthFailureTimeout) {
			if (cfg_in_range(CFG_AUTH_FAILURE_TIMEOUT,
					 session->defaultAuthFailureTimeout)) {
				val = session->defaultAuthFailureTimeout;
			} else {
				val = cfg_default(CFG_AUTH_FAILURE_TIMEOUT);
				session->defaultAuthFailureTimeout = val;
			}
			mac_ctx->mlme_cfg->timeouts.auth_failure_timeout = val;
		}

		lim_restore_from_auth_state(mac_ctx,
				eSIR_SME_AUTH_TIMEOUT_RESULT_CODE,
				eSIR_MAC_UNSPEC_FAILURE_REASON, session);
		break;
	default:
		/*
		 * Auth failure timer should not have timed out
		 * in states other than wt_auth_frame2/4
		 */
		pe_err("received unexpected AUTH failure timeout in state %X",
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
lim_process_auth_rsp_timeout(struct mac_context *mac_ctx, uint32_t auth_idx)
{
	struct tLimPreAuthNode *auth_node;
	struct pe_session *session;
	uint8_t session_id;

	auth_node = lim_get_pre_auth_node_from_index(mac_ctx,
				&mac_ctx->lim.gLimPreAuthTimerTable, auth_idx);
	if (!auth_node) {
		pe_warn("Invalid auth node");
		return;
	}

	session = pe_find_session_by_bssid(mac_ctx, auth_node->peerMacAddr,
					   &session_id);
	if (!session) {
		pe_warn("session does not exist for given BSSID");
		return;
	}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
		lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_AUTH_TIMEOUT,
				session, 0, AUTH_RESPONSE_TIMEOUT);
#endif

	if (LIM_IS_AP_ROLE(session) || LIM_IS_IBSS_ROLE(session)) {
		if (auth_node->mlmState != eLIM_MLM_WT_AUTH_FRAME3_STATE) {
			pe_err("received AUTH rsp timeout in unexpected "
				   "state for MAC address: " MAC_ADDRESS_STR,
				MAC_ADDR_ARRAY(auth_node->peerMacAddr));
		} else {
			auth_node->mlmState = eLIM_MLM_AUTH_RSP_TIMEOUT_STATE;
			auth_node->fTimerStarted = 0;
			pe_debug("AUTH rsp timedout for MAC address "
				   MAC_ADDRESS_STR,
				MAC_ADDR_ARRAY(auth_node->peerMacAddr));
			/* Change timer to reactivate it in future */
			lim_deactivate_and_change_per_sta_id_timer(mac_ctx,
				eLIM_AUTH_RSP_TIMER, auth_node->authNodeIdx);
			lim_delete_pre_auth_node(mac_ctx,
						 auth_node->peerMacAddr);
		}
	}
}

void lim_process_assoc_failure_timeout(struct mac_context *mac_ctx,
						     uint32_t msg_type)
{

	tLimMlmAssocCnf mlm_assoc_cnf;
	struct pe_session *session;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	host_log_rssi_pkt_type *rssi_log = NULL;
#endif
	/*
	 * to fetch the lim/mlm state based on the session_id, use the
	 * below pe_session
	 */
	uint8_t session_id;

	if (msg_type == LIM_ASSOC)
		session_id =
		    mac_ctx->lim.limTimers.gLimAssocFailureTimer.sessionId;
	else
		session_id =
		    mac_ctx->lim.limTimers.gLimReassocFailureTimer.sessionId;

	session = pe_find_session_by_session_id(mac_ctx, session_id);
	if (!session) {
		pe_err("Session Does not exist for given sessionID");
		return;
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_ASSOC_TIMEOUT,
				session, 0, 0);

	WLAN_HOST_DIAG_LOG_ALLOC(rssi_log,
				 host_log_rssi_pkt_type,
				 LOG_WLAN_RSSI_UPDATE_C);
	if (rssi_log)
		rssi_log->rssi = session->rssi;
	WLAN_HOST_DIAG_LOG_REPORT(rssi_log);
#endif

	pe_debug("Re/Association Response not received before timeout");

	/*
	 * Send Deauth to handle the scenareo where association timeout happened
	 * when device has missed the assoc resp sent by peer.
	 * By sending deauth try to clear the session created on peer device.
	 */
	pe_debug("Sessionid: %d try sending deauth on channel %d to BSSID: "
		MAC_ADDRESS_STR, session->peSessionId,
		session->currentOperChannel,
		MAC_ADDR_ARRAY(session->bssId));
	lim_send_deauth_mgmt_frame(mac_ctx, eSIR_MAC_UNSPEC_FAILURE_REASON,
		session->bssId, session, false);

	if ((LIM_IS_AP_ROLE(session)) ||
	    ((session->limMlmState != eLIM_MLM_WT_ASSOC_RSP_STATE) &&
	    (session->limMlmState != eLIM_MLM_WT_REASSOC_RSP_STATE) &&
	    (session->limMlmState != eLIM_MLM_WT_FT_REASSOC_RSP_STATE))) {
		/*
		 * Re/Assoc failure timer should not have timedout on AP
		 * or in a state other than wt_re/assoc_response.
		 */
		pe_warn("received unexpected REASSOC failure timeout in state %X for role %d",
			session->limMlmState,
			GET_LIM_SYSTEM_ROLE(session));
		lim_print_mlm_state(mac_ctx, LOGW, session->limMlmState);
		return;
	}

	if ((msg_type == LIM_ASSOC) || ((msg_type == LIM_REASSOC)
	     && (session->limMlmState == eLIM_MLM_WT_FT_REASSOC_RSP_STATE))) {
		pe_err("(Re)Assoc Failure Timeout occurred");
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
			qdf_mem_free(session->pLimMlmJoinReq);
			session->pLimMlmJoinReq = NULL;
		}
		/* To remove the preauth node in case of fail to associate */
		if (lim_search_pre_auth_list(mac_ctx, session->bssId)) {
			pe_debug("delete pre auth node for "MAC_ADDRESS_STR,
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
void lim_set_channel(struct mac_context *mac_ctx, uint8_t channel,
		     uint8_t ch_center_freq_seg0, uint8_t ch_center_freq_seg1,
		     enum phy_ch_width ch_width, int8_t max_tx_power,
		     uint8_t pe_session_id, uint32_t cac_duration_ms,
		     uint32_t dfs_regdomain)
{
	struct pe_session *pe_session;

	pe_session = pe_find_session_by_session_id(mac_ctx, pe_session_id);

	if (!pe_session) {
		pe_err("Invalid PE session: %d", pe_session_id);
		return;
	}
	lim_send_switch_chnl_params(mac_ctx, channel, ch_center_freq_seg0,
				    ch_center_freq_seg1, ch_width,
				    max_tx_power, pe_session_id, false,
				    cac_duration_ms, dfs_regdomain);
}
