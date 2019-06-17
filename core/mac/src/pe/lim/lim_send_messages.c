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

/*
 *
 * lim_send_messages.c: Provides functions to send messages or Indications to HAL.
 * Author:    Sunit Bhatia
 * Date:       09/21/2006
 * History:-
 * Date        Modified by            Modification Information
 *
 * --------------------------------------------------------------------------
 *
 */
#include "lim_send_messages.h"
#include "lim_trace.h"
#include "wlan_reg_services_api.h"
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
#include "host_diag_core_log.h"
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
#include "lim_utils.h"

/**
 * lim_send_beacon_params() - updates bcn params to WMA
 *
 * @mac                 : pointer to Global Mac structure.
 * @tpUpdateBeaconParams : pointer to the structure, which contains the beacon
 * parameters which are changed.
 *
 * This function is called to send beacon interval, short preamble or other
 * parameters to WMA, which are changed and indication is received in beacon.
 *
 * @return success if message send is ok, else false.
 */
QDF_STATUS lim_send_beacon_params(struct mac_context *mac,
				     tpUpdateBeaconParams pUpdatedBcnParams,
				     struct pe_session *pe_session)
{
	tpUpdateBeaconParams pBcnParams = NULL;
	QDF_STATUS retCode = QDF_STATUS_SUCCESS;
	struct scheduler_msg msgQ = {0};

	pBcnParams = qdf_mem_malloc(sizeof(*pBcnParams));
	if (!pBcnParams)
		return QDF_STATUS_E_NOMEM;
	qdf_mem_copy((uint8_t *) pBcnParams, pUpdatedBcnParams,
		     sizeof(*pBcnParams));
	msgQ.type = WMA_UPDATE_BEACON_IND;
	msgQ.reserved = 0;
	msgQ.bodyptr = pBcnParams;
	msgQ.bodyval = 0;
	pe_debug("Sending WMA_UPDATE_BEACON_IND, paramChangeBitmap in hex: %x",
	       pUpdatedBcnParams->paramChangeBitmap);
	if (!pe_session) {
		qdf_mem_free(pBcnParams);
		MTRACE(mac_trace_msg_tx(mac, NO_SESSION, msgQ.type));
		return QDF_STATUS_E_FAILURE;
	} else {
		MTRACE(mac_trace_msg_tx(mac,
					pe_session->peSessionId,
					msgQ.type));
	}
	pBcnParams->smeSessionId = pe_session->smeSessionId;
	retCode = wma_post_ctrl_msg(mac, &msgQ);
	if (QDF_STATUS_SUCCESS != retCode) {
		qdf_mem_free(pBcnParams);
		pe_err("Posting WMA_UPDATE_BEACON_IND, reason=%X",
			retCode);
	}
	lim_send_beacon_ind(mac, pe_session, REASON_DEFAULT);
	return retCode;
}

/**
 * lim_send_switch_chnl_params()
 *
 ***FUNCTION:
 * This function is called to send Channel Switch Indication to WMA
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param mac  pointer to Global Mac structure.
 * @param chnlNumber New Channel Number to be switched to.
 * @param ch_width an enum for channel width.
 * @param localPowerConstraint 11h local power constraint value
 *
 * @return success if message send is ok, else false.
 */
QDF_STATUS lim_send_switch_chnl_params(struct mac_context *mac,
					  uint8_t chnlNumber,
					  uint8_t ch_center_freq_seg0,
					  uint8_t ch_center_freq_seg1,
					  enum phy_ch_width ch_width,
					  int8_t maxTxPower,
					  uint8_t peSessionId,
					  uint8_t is_restart,
					  uint32_t cac_duration_ms,
					  uint32_t dfs_regdomain)
{
	tpSwitchChannelParams pChnlParams = NULL;
	struct scheduler_msg msgQ = {0};
	struct pe_session *pe_session;

	pe_session = pe_find_session_by_session_id(mac, peSessionId);
	if (!pe_session) {
		pe_err("Unable to get Session for session Id %d",
				peSessionId);
		return QDF_STATUS_E_FAILURE;
	}
	pChnlParams = qdf_mem_malloc(sizeof(tSwitchChannelParams));
	if (!pChnlParams)
		return QDF_STATUS_E_NOMEM;
	pChnlParams->channelNumber = chnlNumber;
	pChnlParams->ch_center_freq_seg0 = ch_center_freq_seg0;
	pChnlParams->ch_center_freq_seg1 = ch_center_freq_seg1;
	pChnlParams->ch_width = ch_width;
	qdf_mem_copy(pChnlParams->selfStaMacAddr, pe_session->self_mac_addr,
		     sizeof(tSirMacAddr));
	pChnlParams->maxTxPower = maxTxPower;
	qdf_mem_copy(pChnlParams->bssId, pe_session->bssId,
		     sizeof(tSirMacAddr));
	pChnlParams->peSessionId = peSessionId;
	pChnlParams->vhtCapable = pe_session->vhtCapability;
	if (lim_is_session_he_capable(pe_session))
		lim_update_chan_he_capable(mac, pChnlParams);
	pChnlParams->dot11_mode = pe_session->dot11mode;
	pChnlParams->nss = pe_session->nss;
	pe_debug("dot11mode: %d, vht_capable: %d nss value: %d",
		pChnlParams->dot11_mode, pChnlParams->vhtCapable,
		pChnlParams->nss);

	/*Set DFS flag for DFS channel */
	if (ch_width == CH_WIDTH_160MHZ) {
		pChnlParams->isDfsChannel = true;
	} else if (ch_width == CH_WIDTH_80P80MHZ) {
		pChnlParams->isDfsChannel = false;
		if (wlan_reg_get_channel_state(mac->pdev, chnlNumber) ==
				CHANNEL_STATE_DFS ||
		    wlan_reg_get_channel_state(mac->pdev,
			    pChnlParams->ch_center_freq_seg1 -
				SIR_80MHZ_START_CENTER_CH_DIFF) ==
							CHANNEL_STATE_DFS)
			pChnlParams->isDfsChannel = true;
	} else {
		if (wlan_reg_get_channel_state(mac->pdev, chnlNumber) ==
				CHANNEL_STATE_DFS)
			pChnlParams->isDfsChannel = true;
		else
			pChnlParams->isDfsChannel = false;
	}

	pChnlParams->restart_on_chan_switch = is_restart;
	pChnlParams->cac_duration_ms = cac_duration_ms;
	pChnlParams->dfs_regdomain = dfs_regdomain;
	pChnlParams->reduced_beacon_interval =
		mac->sap.SapDfsInfo.reduced_beacon_interval;

	if (cds_is_5_mhz_enabled())
		pChnlParams->ch_width = CH_WIDTH_5MHZ;
	else if (cds_is_10_mhz_enabled())
		pChnlParams->ch_width = CH_WIDTH_10MHZ;

	/* we need to defer the message until we
	 * get the response back from WMA
	 */
	SET_LIM_PROCESS_DEFD_MESGS(mac, false);
	msgQ.type = WMA_CHNL_SWITCH_REQ;
	msgQ.reserved = 0;
	msgQ.bodyptr = pChnlParams;
	msgQ.bodyval = 0;
	pe_debug("Sending CH_SWITCH_REQ, ch_width %d, ch_num %d, maxTxPower %d",
		       pChnlParams->ch_width,
		       pChnlParams->channelNumber, pChnlParams->maxTxPower);
	MTRACE(mac_trace_msg_tx(mac, peSessionId, msgQ.type));
	if (QDF_STATUS_SUCCESS != wma_post_ctrl_msg(mac, &msgQ)) {
		qdf_mem_free(pChnlParams);
		pe_err("Posting  CH_SWITCH_REQ to WMA failed");
		return QDF_STATUS_E_FAILURE;
	}
	pe_session->ch_switch_in_progress = true;

	return QDF_STATUS_SUCCESS;
}

/**
 * lim_send_edca_params()
 *
 ***FUNCTION:
 * This function is called to send dynamically changing EDCA Parameters to WMA.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param mac  pointer to Global Mac structure.
 * @param tpUpdatedEdcaParams pointer to the structure which contains
 *                                       dynamically changing EDCA parameters.
 * @param highPerformance  If the peer is Airgo (taurus) then switch to highPerformance is true.
 *
 * @return success if message send is ok, else false.
 */
QDF_STATUS lim_send_edca_params(struct mac_context *mac,
				   tSirMacEdcaParamRecord *pUpdatedEdcaParams,
				   uint16_t bss_idx, bool mu_edca)
{
	tEdcaParams *pEdcaParams = NULL;
	QDF_STATUS retCode = QDF_STATUS_SUCCESS;
	struct scheduler_msg msgQ = {0};

	pEdcaParams = qdf_mem_malloc(sizeof(tEdcaParams));
	if (!pEdcaParams)
		return QDF_STATUS_E_NOMEM;
	pEdcaParams->bss_idx = bss_idx;
	pEdcaParams->acbe = pUpdatedEdcaParams[QCA_WLAN_AC_BE];
	pEdcaParams->acbk = pUpdatedEdcaParams[QCA_WLAN_AC_BK];
	pEdcaParams->acvi = pUpdatedEdcaParams[QCA_WLAN_AC_VI];
	pEdcaParams->acvo = pUpdatedEdcaParams[QCA_WLAN_AC_VO];
	pEdcaParams->mu_edca_params = mu_edca;
	msgQ.type = WMA_UPDATE_EDCA_PROFILE_IND;
	msgQ.reserved = 0;
	msgQ.bodyptr = pEdcaParams;
	msgQ.bodyval = 0;
	pe_debug("Sending WMA_UPDATE_EDCA_PROFILE_IND");
	MTRACE(mac_trace_msg_tx(mac, NO_SESSION, msgQ.type));
	retCode = wma_post_ctrl_msg(mac, &msgQ);
	if (QDF_STATUS_SUCCESS != retCode) {
		qdf_mem_free(pEdcaParams);
		pe_err("Posting WMA_UPDATE_EDCA_PROFILE_IND failed, reason=%X",
			retCode);
	}
	return retCode;
}

void lim_set_active_edca_params(struct mac_context *mac_ctx,
				tSirMacEdcaParamRecord *edca_params,
				struct pe_session *pe_session)
{
	uint8_t ac, new_ac, i;
	uint8_t ac_admitted;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	host_log_qos_edca_pkt_type *log_ptr = NULL;
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
	/* Initialize gLimEdcaParamsActive[] to be same as localEdcaParams */
	pe_session->gLimEdcaParamsActive[QCA_WLAN_AC_BE] = edca_params[QCA_WLAN_AC_BE];
	pe_session->gLimEdcaParamsActive[QCA_WLAN_AC_BK] = edca_params[QCA_WLAN_AC_BK];
	pe_session->gLimEdcaParamsActive[QCA_WLAN_AC_VI] = edca_params[QCA_WLAN_AC_VI];
	pe_session->gLimEdcaParamsActive[QCA_WLAN_AC_VO] = edca_params[QCA_WLAN_AC_VO];

	pe_session->gLimEdcaParamsActive[QCA_WLAN_AC_BE].no_ack =
					mac_ctx->no_ack_policy_cfg[QCA_WLAN_AC_BE];
	pe_session->gLimEdcaParamsActive[QCA_WLAN_AC_BK].no_ack =
					mac_ctx->no_ack_policy_cfg[QCA_WLAN_AC_BK];
	pe_session->gLimEdcaParamsActive[QCA_WLAN_AC_VI].no_ack =
					mac_ctx->no_ack_policy_cfg[QCA_WLAN_AC_VI];
	pe_session->gLimEdcaParamsActive[QCA_WLAN_AC_VO].no_ack =
					mac_ctx->no_ack_policy_cfg[QCA_WLAN_AC_VO];

	/* An AC requires downgrade if the ACM bit is set, and the AC has not
	 * yet been admitted in uplink or bi-directions.
	 * If an AC requires downgrade, it will downgrade to the next beset AC
	 * for which ACM is not enabled.
	 *
	 * - There's no need to downgrade AC_BE since it IS the lowest AC. Hence
	 *   start the for loop with AC_BK.
	 * - If ACM bit is set for an AC, initially downgrade it to AC_BE. Then
	 *   traverse thru the AC list. If we do find the next best AC which is
	 *   better than AC_BE, then use that one. For example, if ACM bits are set
	 *   such that: BE_ACM=1, BK_ACM=1, VI_ACM=1, VO_ACM=0
	 *   then all AC will be downgraded to AC_BE.
	 */
	pe_debug("adAdmitMask[UPLINK] = 0x%x ",
		pe_session->gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK]);
	pe_debug("adAdmitMask[DOWNLINK] = 0x%x ",
		pe_session->gAcAdmitMask[SIR_MAC_DIRECTION_DNLINK]);
	for (ac = QCA_WLAN_AC_BK; ac <= QCA_WLAN_AC_VO; ac++) {
		ac_admitted =
			((pe_session->gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK] &
			 (1 << ac)) >> ac);

		pe_debug("For AC[%d]: acm=%d,  ac_admitted=%d ",
			ac, edca_params[ac].aci.acm, ac_admitted);
		if ((edca_params[ac].aci.acm == 1) && (ac_admitted == 0)) {
			pe_debug("We need to downgrade AC %d!!", ac);
			/* Loop backwards through AC values until it finds
			 * acm == 0 or reaches QCA_WLAN_AC_BE.
			 * Note that for block has no executable statements.
			 */
			for (i = ac - 1;
			    (i > QCA_WLAN_AC_BE &&
				(edca_params[i].aci.acm != 0));
			     i--)
				;
			new_ac = i;
			pe_debug("Downgrading AC %d ---> AC %d ", ac, new_ac);
			pe_session->gLimEdcaParamsActive[ac] =
				edca_params[new_ac];
		}
	}
/* log: LOG_WLAN_QOS_EDCA_C */
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	WLAN_HOST_DIAG_LOG_ALLOC(log_ptr, host_log_qos_edca_pkt_type,
				 LOG_WLAN_QOS_EDCA_C);
	if (log_ptr) {
		tSirMacEdcaParamRecord *rec;

		rec = &pe_session->gLimEdcaParamsActive[QCA_WLAN_AC_BE];
		log_ptr->aci_be = rec->aci.aci;
		log_ptr->cw_be = rec->cw.max << 4 | rec->cw.min;
		log_ptr->txoplimit_be = rec->txoplimit;

		rec = &pe_session->gLimEdcaParamsActive[QCA_WLAN_AC_BK];
		log_ptr->aci_bk = rec->aci.aci;
		log_ptr->cw_bk = rec->cw.max << 4 | rec->cw.min;
		log_ptr->txoplimit_bk = rec->txoplimit;

		rec = &pe_session->gLimEdcaParamsActive[QCA_WLAN_AC_VI];
		log_ptr->aci_vi = rec->aci.aci;
		log_ptr->cw_vi = rec->cw.max << 4 | rec->cw.min;
		log_ptr->txoplimit_vi = rec->txoplimit;

		rec = &pe_session->gLimEdcaParamsActive[QCA_WLAN_AC_VO];
		log_ptr->aci_vo = rec->aci.aci;
		log_ptr->cw_vo = rec->cw.max << 4 | rec->cw.min;
		log_ptr->txoplimit_vo = rec->txoplimit;
	}
	WLAN_HOST_DIAG_LOG_REPORT(log_ptr);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	return;
}

/** ---------------------------------------------------------
   \fn      lim_set_link_state
   \brief   LIM sends a message to WMA to set the link state
   \param   struct mac_context * mac
   \param   tSirLinkState      state
   \return  None
   -----------------------------------------------------------*/
QDF_STATUS lim_set_link_state(struct mac_context *mac, tSirLinkState state,
				 tSirMacAddr bssId, tSirMacAddr self_mac_addr,
				 tpSetLinkStateCallback callback,
				 void *callbackArg)
{
	struct scheduler_msg msgQ = {0};
	QDF_STATUS retCode;
	tpLinkStateParams pLinkStateParams = NULL;
	/* Allocate memory. */
	pLinkStateParams = qdf_mem_malloc(sizeof(tLinkStateParams));
	if (!pLinkStateParams)
		return QDF_STATUS_E_NOMEM;
	pLinkStateParams->state = state;
	pLinkStateParams->callback = callback;
	pLinkStateParams->callbackArg = callbackArg;

	/* Copy Mac address */
	sir_copy_mac_addr(pLinkStateParams->bssid, bssId);
	sir_copy_mac_addr(pLinkStateParams->self_mac_addr, self_mac_addr);

	msgQ.type = WMA_SET_LINK_STATE;
	msgQ.reserved = 0;
	msgQ.bodyptr = pLinkStateParams;
	msgQ.bodyval = 0;

	MTRACE(mac_trace_msg_tx(mac, NO_SESSION, msgQ.type));

	retCode = (uint32_t) wma_post_ctrl_msg(mac, &msgQ);
	if (retCode != QDF_STATUS_SUCCESS) {
		qdf_mem_free(pLinkStateParams);
		pe_err("Posting link state %d failed, reason = %x", state,
			retCode);
	}
	return retCode;
}

QDF_STATUS lim_send_mode_update(struct mac_context *mac,
				   tUpdateVHTOpMode *pTempParam,
				   struct pe_session *pe_session)
{
	tUpdateVHTOpMode *pVhtOpMode = NULL;
	QDF_STATUS retCode = QDF_STATUS_SUCCESS;
	struct scheduler_msg msgQ = {0};

	pVhtOpMode = qdf_mem_malloc(sizeof(tUpdateVHTOpMode));
	if (!pVhtOpMode)
		return QDF_STATUS_E_NOMEM;
	qdf_mem_copy((uint8_t *) pVhtOpMode, pTempParam,
		     sizeof(tUpdateVHTOpMode));
	msgQ.type = WMA_UPDATE_OP_MODE;
	msgQ.reserved = 0;
	msgQ.bodyptr = pVhtOpMode;
	msgQ.bodyval = 0;
	pe_debug("Sending WMA_UPDATE_OP_MODE, op_mode %d, sta_id %d",
			pVhtOpMode->opMode, pVhtOpMode->staId);
	if (!pe_session)
		MTRACE(mac_trace_msg_tx(mac, NO_SESSION, msgQ.type));
	else
		MTRACE(mac_trace_msg_tx(mac,
					pe_session->peSessionId,
					msgQ.type));
	retCode = wma_post_ctrl_msg(mac, &msgQ);
	if (QDF_STATUS_SUCCESS != retCode) {
		qdf_mem_free(pVhtOpMode);
		pe_err("Posting WMA_UPDATE_OP_MODE failed, reason=%X",
			retCode);
	}

	return retCode;
}

QDF_STATUS lim_send_rx_nss_update(struct mac_context *mac,
				     tUpdateRxNss *pTempParam,
				     struct pe_session *pe_session)
{
	tUpdateRxNss *pRxNss = NULL;
	QDF_STATUS retCode = QDF_STATUS_SUCCESS;
	struct scheduler_msg msgQ = {0};

	pRxNss = qdf_mem_malloc(sizeof(tUpdateRxNss));
	if (!pRxNss)
		return QDF_STATUS_E_NOMEM;
	qdf_mem_copy((uint8_t *) pRxNss, pTempParam, sizeof(tUpdateRxNss));
	msgQ.type = WMA_UPDATE_RX_NSS;
	msgQ.reserved = 0;
	msgQ.bodyptr = pRxNss;
	msgQ.bodyval = 0;
	pe_debug("Sending WMA_UPDATE_RX_NSS");
	if (!pe_session)
		MTRACE(mac_trace_msg_tx(mac, NO_SESSION, msgQ.type));
	else
		MTRACE(mac_trace_msg_tx(mac,
					pe_session->peSessionId,
					msgQ.type));
	retCode = wma_post_ctrl_msg(mac, &msgQ);
	if (QDF_STATUS_SUCCESS != retCode) {
		qdf_mem_free(pRxNss);
		pe_err("Posting WMA_UPDATE_RX_NSS failed, reason=%X",
			retCode);
	}

	return retCode;
}

QDF_STATUS lim_set_membership(struct mac_context *mac,
				 tUpdateMembership *pTempParam,
				 struct pe_session *pe_session)
{
	tUpdateMembership *pMembership = NULL;
	QDF_STATUS retCode = QDF_STATUS_SUCCESS;
	struct scheduler_msg msgQ = {0};

	pMembership = qdf_mem_malloc(sizeof(tUpdateMembership));
	if (!pMembership)
		return QDF_STATUS_E_NOMEM;
	qdf_mem_copy((uint8_t *) pMembership, pTempParam,
		     sizeof(tUpdateMembership));

	msgQ.type = WMA_UPDATE_MEMBERSHIP;
	msgQ.reserved = 0;
	msgQ.bodyptr = pMembership;
	msgQ.bodyval = 0;
	pe_debug("Sending WMA_UPDATE_MEMBERSHIP");
	if (!pe_session)
		MTRACE(mac_trace_msg_tx(mac, NO_SESSION, msgQ.type));
	else
		MTRACE(mac_trace_msg_tx(mac,
					pe_session->peSessionId,
					msgQ.type));
	retCode = wma_post_ctrl_msg(mac, &msgQ);
	if (QDF_STATUS_SUCCESS != retCode) {
		qdf_mem_free(pMembership);
		pe_err("Posting WMA_UPDATE_MEMBERSHIP failed, reason=%X",
			retCode);
	}

	return retCode;
}

QDF_STATUS lim_set_user_pos(struct mac_context *mac,
			       tUpdateUserPos *pTempParam,
			       struct pe_session *pe_session)
{
	tUpdateUserPos *pUserPos = NULL;
	QDF_STATUS retCode = QDF_STATUS_SUCCESS;
	struct scheduler_msg msgQ = {0};

	pUserPos = qdf_mem_malloc(sizeof(tUpdateUserPos));
	if (!pUserPos)
		return QDF_STATUS_E_NOMEM;
	qdf_mem_copy((uint8_t *) pUserPos, pTempParam, sizeof(tUpdateUserPos));

	msgQ.type = WMA_UPDATE_USERPOS;
	msgQ.reserved = 0;
	msgQ.bodyptr = pUserPos;
	msgQ.bodyval = 0;
	pe_debug("Sending WMA_UPDATE_USERPOS");
	if (!pe_session)
		MTRACE(mac_trace_msg_tx(mac, NO_SESSION, msgQ.type));
	else
		MTRACE(mac_trace_msg_tx(mac,
					pe_session->peSessionId,
					msgQ.type));
	retCode = wma_post_ctrl_msg(mac, &msgQ);
	if (QDF_STATUS_SUCCESS != retCode) {
		qdf_mem_free(pUserPos);
		pe_err("Posting WMA_UPDATE_USERPOS failed, reason=%X",
			retCode);
	}

	return retCode;
}

#ifdef WLAN_FEATURE_11W
/**
 * lim_send_exclude_unencrypt_ind() - sends WMA_EXCLUDE_UNENCRYPTED_IND to HAL
 * @mac:          mac global context
 * @excludeUnenc:  true: ignore, false: indicate
 * @pe_session: session context
 *
 * LIM sends a message to HAL to indicate whether to ignore or indicate the
 * unprotected packet error.
 *
 * Return: status of operation
 */
QDF_STATUS lim_send_exclude_unencrypt_ind(struct mac_context *mac,
					     bool excludeUnenc,
					     struct pe_session *pe_session)
{
	QDF_STATUS retCode = QDF_STATUS_SUCCESS;
	struct scheduler_msg msgQ = {0};
	tSirWlanExcludeUnencryptParam *pExcludeUnencryptParam;

	pExcludeUnencryptParam =
		qdf_mem_malloc(sizeof(tSirWlanExcludeUnencryptParam));
	if (!pExcludeUnencryptParam)
		return QDF_STATUS_E_NOMEM;

	pExcludeUnencryptParam->excludeUnencrypt = excludeUnenc;
	qdf_mem_copy(pExcludeUnencryptParam->bssid.bytes, pe_session->bssId,
			QDF_MAC_ADDR_SIZE);

	msgQ.type = WMA_EXCLUDE_UNENCRYPTED_IND;
	msgQ.reserved = 0;
	msgQ.bodyptr = pExcludeUnencryptParam;
	msgQ.bodyval = 0;
	pe_debug("Sending WMA_EXCLUDE_UNENCRYPTED_IND");
	MTRACE(mac_trace_msg_tx(mac, pe_session->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(mac, &msgQ);
	if (QDF_STATUS_SUCCESS != retCode) {
		qdf_mem_free(pExcludeUnencryptParam);
		pe_err("Posting WMA_EXCLUDE_UNENCRYPTED_IND failed, reason=%X",
			retCode);
	}

	return retCode;
}
#endif

/**
 * lim_send_ht40_obss_scanind() - send ht40 obss start scan request
 * mac: mac context
 * session  PE session handle
 *
 * LIM sends a HT40 start scan message to WMA
 *
 * Return: status of operation
 */
QDF_STATUS lim_send_ht40_obss_scanind(struct mac_context *mac_ctx,
						struct pe_session *session)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	struct obss_ht40_scanind *ht40_obss_scanind;
	uint32_t channelnum;
	struct scheduler_msg msg = {0};
	uint8_t chan_list[CFG_VALID_CHANNEL_LIST_LEN];
	uint8_t channel24gnum, count;

	ht40_obss_scanind = qdf_mem_malloc(sizeof(struct obss_ht40_scanind));
	if (!ht40_obss_scanind)
		return QDF_STATUS_E_FAILURE;
	QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
		"OBSS Scan Indication bss_idx- %d staId %d",
		session->bss_idx, session->staId);

	ht40_obss_scanind->cmd = HT40_OBSS_SCAN_PARAM_START;
	ht40_obss_scanind->scan_type = eSIR_ACTIVE_SCAN;
	ht40_obss_scanind->obss_passive_dwelltime =
		session->obss_ht40_scanparam.obss_passive_dwelltime;
	ht40_obss_scanind->obss_active_dwelltime =
		session->obss_ht40_scanparam.obss_active_dwelltime;
	ht40_obss_scanind->obss_width_trigger_interval =
		session->obss_ht40_scanparam.obss_width_trigger_interval;
	ht40_obss_scanind->obss_passive_total_per_channel =
		session->obss_ht40_scanparam.obss_passive_total_per_channel;
	ht40_obss_scanind->obss_active_total_per_channel =
		session->obss_ht40_scanparam.obss_active_total_per_channel;
	ht40_obss_scanind->bsswidth_ch_trans_delay =
		session->obss_ht40_scanparam.bsswidth_ch_trans_delay;
	ht40_obss_scanind->obss_activity_threshold =
		session->obss_ht40_scanparam.obss_activity_threshold;
	ht40_obss_scanind->current_operatingclass =
		wlan_reg_dmn_get_opclass_from_channel(
			mac_ctx->scan.countryCodeCurrent,
			session->currentOperChannel,
			session->ch_width);
	channelnum = mac_ctx->mlme_cfg->reg.valid_channel_list_num;
	qdf_mem_copy(chan_list, mac_ctx->mlme_cfg->reg.valid_channel_list,
		     channelnum);
	/* Extract 24G channel list */
	channel24gnum = 0;
	for (count = 0; count < channelnum &&
		(channel24gnum < SIR_ROAM_MAX_CHANNELS); count++) {
		if ((chan_list[count] > CHAN_ENUM_1) &&
			(chan_list[count] < CHAN_ENUM_14)) {
			ht40_obss_scanind->channels[channel24gnum] =
				chan_list[count];
			channel24gnum++;
		}
	}
	ht40_obss_scanind->channel_count = channel24gnum;
	/* FW API requests BSS IDX */
	ht40_obss_scanind->self_sta_idx = session->staId;
	ht40_obss_scanind->bss_id = session->bss_idx;
	ht40_obss_scanind->fortymhz_intolerent = 0;
	ht40_obss_scanind->iefield_len = 0;
	msg.type = WMA_HT40_OBSS_SCAN_IND;
	msg.reserved = 0;
	msg.bodyptr = (void *)ht40_obss_scanind;
	msg.bodyval = 0;
	pe_debug("Sending WDA_HT40_OBSS_SCAN_IND to WDA"
		"Obss Scan trigger width: %d, delay factor: %d",
		ht40_obss_scanind->obss_width_trigger_interval,
		ht40_obss_scanind->bsswidth_ch_trans_delay);
	ret = wma_post_ctrl_msg(mac_ctx, &msg);
	if (QDF_STATUS_SUCCESS != ret) {
		pe_err("WDA_HT40_OBSS_SCAN_IND msg failed, reason=%X",
			ret);
		qdf_mem_free(ht40_obss_scanind);
	}
	return ret;
}
