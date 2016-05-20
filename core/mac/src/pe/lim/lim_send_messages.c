/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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
#include "cfg_api.h"
#include "lim_trace.h"
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
#include "host_diag_core_log.h"
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

/* When beacon filtering is enabled, firmware will
 * analyze the selected beacons received during BMPS,
 * and monitor any changes in the IEs as listed below.
 * The format of the table is:
 *    - EID
 *    - Check for IE presence
 *    - Byte offset
 *    - Byte value
 *    - Bit Mask
 *    - Byte refrence
 */
static tBeaconFilterIe beacon_filter_table[] = {
	{SIR_MAC_DS_PARAM_SET_EID, 0, {0, 0, DS_PARAM_CHANNEL_MASK, 0} },
	{SIR_MAC_ERP_INFO_EID, 0, {0, 0, ERP_FILTER_MASK, 0} },
	{SIR_MAC_EDCA_PARAM_SET_EID, 0, {0, 0, EDCA_FILTER_MASK, 0} },
	{SIR_MAC_QOS_CAPABILITY_EID, 0, {0, 0, QOS_FILTER_MASK, 0} },
	{SIR_MAC_CHNL_SWITCH_ANN_EID, 1, {0, 0, 0, 0} },
	{SIR_MAC_HT_INFO_EID, 0, {0, 0, HT_BYTE0_FILTER_MASK, 0} },
	{SIR_MAC_HT_INFO_EID, 0, {2, 0, HT_BYTE2_FILTER_MASK, 0} },
	{SIR_MAC_HT_INFO_EID, 0, {5, 0, HT_BYTE5_FILTER_MASK, 0} },
	{SIR_MAC_PWR_CONSTRAINT_EID, 0, {0, 0, 0, 0} },
	{SIR_MAC_VHT_OPMODE_EID, 0, {0, 0, 0, 0} },
	{SIR_MAC_VHT_OPERATION_EID, 0, {0, 0, VHTOP_CHWIDTH_MASK, 0} }
};

/**
 * lim_send_cf_params()
 *
 ***FUNCTION:
 * This function is called to send CFP Parameters to WMA, when they are changed.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param pMac  pointer to Global Mac structure.
 * @param bssIdx Bss Index of the BSS to which STA is associated.
 * @param cfpCount CFP Count, if that is changed.
 * @param cfpPeriod CFP Period if that is changed.
 *
 * @return success if message send is ok, else false.
 */
tSirRetStatus lim_send_cf_params(tpAniSirGlobal pMac, uint8_t bssIdx,
				 uint8_t cfpCount, uint8_t cfpPeriod)
{
	tpUpdateCFParams pCFParams = NULL;
	tSirRetStatus retCode = eSIR_SUCCESS;
	tSirMsgQ msgQ;

	pCFParams = qdf_mem_malloc(sizeof(tUpdateCFParams));
	if (NULL == pCFParams) {
		lim_log(pMac, LOGP,
			FL("Unable to allocate memory during Update CF Params"));
		retCode = eSIR_MEM_ALLOC_FAILED;
		goto returnFailure;
	}
	qdf_mem_set((uint8_t *) pCFParams, sizeof(tUpdateCFParams), 0);
	pCFParams->cfpCount = cfpCount;
	pCFParams->cfpPeriod = cfpPeriod;
	pCFParams->bssIdx = bssIdx;

	msgQ.type = WMA_UPDATE_CF_IND;
	msgQ.reserved = 0;
	msgQ.bodyptr = pCFParams;
	msgQ.bodyval = 0;
	lim_log(pMac, LOG3, FL("Sending WMA_UPDATE_CF_IND..."));
	MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		qdf_mem_free(pCFParams);
		lim_log(pMac, LOGP,
			FL("Posting WMA_UPDATE_CF_IND failed, reason=%X"),
			retCode);
	}
returnFailure:
	return retCode;
}

/**
 * lim_send_beacon_params() - updates bcn params to WMA
 *
 * @pMac                 : pointer to Global Mac structure.
 * @tpUpdateBeaconParams : pointer to the structure, which contains the beacon
 * parameters which are changed.
 *
 * This function is called to send beacon interval, short preamble or other
 * parameters to WMA, which are changed and indication is received in beacon.
 *
 * @return success if message send is ok, else false.
 */
tSirRetStatus lim_send_beacon_params(tpAniSirGlobal pMac,
				     tpUpdateBeaconParams pUpdatedBcnParams,
				     tpPESession psessionEntry)
{
	tpUpdateBeaconParams pBcnParams = NULL;
	tSirRetStatus retCode = eSIR_SUCCESS;
	tSirMsgQ msgQ;

	pBcnParams = qdf_mem_malloc(sizeof(*pBcnParams));
	if (NULL == pBcnParams) {
		lim_log(pMac, LOGP,
			FL("Unable to allocate memory during Update Beacon Params"));
		return eSIR_MEM_ALLOC_FAILED;
	}
	qdf_mem_copy((uint8_t *) pBcnParams, pUpdatedBcnParams,
		     sizeof(*pBcnParams));
	msgQ.type = WMA_UPDATE_BEACON_IND;
	msgQ.reserved = 0;
	msgQ.bodyptr = pBcnParams;
	msgQ.bodyval = 0;
	PELOG3(lim_log(pMac, LOG3,
	       FL("Sending WMA_UPDATE_BEACON_IND, paramChangeBitmap in hex = %x"),
	       pUpdatedBcnParams->paramChangeBitmap);)
	if (NULL == psessionEntry) {
		qdf_mem_free(pBcnParams);
		MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msgQ.type));
		return eSIR_FAILURE;
	} else {
		MTRACE(mac_trace_msg_tx(pMac,
					psessionEntry->peSessionId,
					msgQ.type));
	}
	pBcnParams->smeSessionId = psessionEntry->smeSessionId;
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		qdf_mem_free(pBcnParams);
		lim_log(pMac, LOGP,
			FL("Posting WMA_UPDATE_BEACON_IND, reason=%X"),
			retCode);
	}
	lim_send_beacon_ind(pMac, psessionEntry);
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
 * @param pMac  pointer to Global Mac structure.
 * @param chnlNumber New Channel Number to be switched to.
 * @param ch_width an enum for channel width.
 * @param localPowerConstraint 11h local power constraint value
 *
 * @return success if message send is ok, else false.
 */
tSirRetStatus lim_send_switch_chnl_params(tpAniSirGlobal pMac,
					  uint8_t chnlNumber,
					  uint8_t ch_center_freq_seg0,
					  uint8_t ch_center_freq_seg1,
					  enum phy_ch_width ch_width,
					  int8_t maxTxPower,
					  uint8_t peSessionId,
					  uint8_t is_restart)
{
	tpSwitchChannelParams pChnlParams = NULL;
	tSirMsgQ msgQ;
	tpPESession pSessionEntry;
	pSessionEntry = pe_find_session_by_session_id(pMac, peSessionId);
	if (pSessionEntry == NULL) {
		lim_log(pMac, LOGP, FL(
				"Unable to get Session for session Id %d"),
				peSessionId);
		return eSIR_FAILURE;
	}
	pChnlParams = qdf_mem_malloc(sizeof(tSwitchChannelParams));
	if (NULL == pChnlParams) {
		lim_log(pMac, LOGP, FL(
			"Unable to allocate memory for Switch Ch Params"));
		return eSIR_MEM_ALLOC_FAILED;
	}
	qdf_mem_set((uint8_t *) pChnlParams, sizeof(tSwitchChannelParams), 0);
	pChnlParams->channelNumber = chnlNumber;
	pChnlParams->ch_center_freq_seg0 = ch_center_freq_seg0;
	pChnlParams->ch_center_freq_seg1 = ch_center_freq_seg1;
	pChnlParams->ch_width = ch_width;
	qdf_mem_copy(pChnlParams->selfStaMacAddr, pSessionEntry->selfMacAddr,
		     sizeof(tSirMacAddr));
	pChnlParams->maxTxPower = maxTxPower;
	qdf_mem_copy(pChnlParams->bssId, pSessionEntry->bssId,
		     sizeof(tSirMacAddr));
	pChnlParams->peSessionId = peSessionId;
	pChnlParams->vhtCapable = pSessionEntry->vhtCapability;
	pChnlParams->dot11_mode = pSessionEntry->dot11mode;
	pChnlParams->nss = pSessionEntry->nss;
	lim_log(pMac, LOG2, FL("nss value: %d"), pChnlParams->nss);

	/*Set DFS flag for DFS channel */
	if (ch_width == CH_WIDTH_160MHZ) {
		pChnlParams->isDfsChannel = true;
	} else if (ch_width == CH_WIDTH_80P80MHZ) {
		pChnlParams->isDfsChannel = false;
		if (cds_get_channel_state(chnlNumber) == CHANNEL_STATE_DFS ||
		    cds_get_channel_state(pChnlParams->ch_center_freq_seg1 -
				SIR_80MHZ_START_CENTER_CH_DIFF) ==
							CHANNEL_STATE_DFS)
			pChnlParams->isDfsChannel = true;
	} else {
		if (cds_get_channel_state(chnlNumber) == CHANNEL_STATE_DFS)
			pChnlParams->isDfsChannel = true;
		else
			pChnlParams->isDfsChannel = false;
	}

	pChnlParams->restart_on_chan_switch = is_restart;

	if (cds_is_5_mhz_enabled())
		pChnlParams->ch_width = CH_WIDTH_5MHZ;
	else if (cds_is_10_mhz_enabled())
		pChnlParams->ch_width = CH_WIDTH_10MHZ;

	/* we need to defer the message until we
	 * get the response back from WMA
	 */
	SET_LIM_PROCESS_DEFD_MESGS(pMac, false);
	msgQ.type = WMA_CHNL_SWITCH_REQ;
	msgQ.reserved = 0;
	msgQ.bodyptr = pChnlParams;
	msgQ.bodyval = 0;
	PELOG3(lim_log(pMac, LOG3, FL(
		"Sending CH_SWITCH_REQ, ch_width %d, ch_num %d, maxTxPower %d"),
		       pChnlParams->ch_width,
		       pChnlParams->channelNumber, pChnlParams->maxTxPower);)
	MTRACE(mac_trace_msg_tx(pMac, peSessionId, msgQ.type));
	if (eSIR_SUCCESS != wma_post_ctrl_msg(pMac, &msgQ)) {
		qdf_mem_free(pChnlParams);
		lim_log(pMac, LOGP, FL(
			"Posting  CH_SWITCH_REQ to WMA failed"));
		return eSIR_FAILURE;
	}
	return eSIR_SUCCESS;
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
 * @param pMac  pointer to Global Mac structure.
 * @param tpUpdatedEdcaParams pointer to the structure which contains
 *                                       dynamically changing EDCA parameters.
 * @param highPerformance  If the peer is Airgo (taurus) then switch to highPerformance is true.
 *
 * @return success if message send is ok, else false.
 */
tSirRetStatus lim_send_edca_params(tpAniSirGlobal pMac,
				   tSirMacEdcaParamRecord *pUpdatedEdcaParams,
				   uint16_t bssIdx)
{
	tEdcaParams *pEdcaParams = NULL;
	tSirRetStatus retCode = eSIR_SUCCESS;
	tSirMsgQ msgQ;
	uint8_t i;

	pEdcaParams = qdf_mem_malloc(sizeof(tEdcaParams));
	if (NULL == pEdcaParams) {
		lim_log(pMac, LOGP,
			FL("Unable to allocate memory during Update EDCA Params"));
		retCode = eSIR_MEM_ALLOC_FAILED;
		return retCode;
	}
	pEdcaParams->bssIdx = bssIdx;
	pEdcaParams->acbe = pUpdatedEdcaParams[EDCA_AC_BE];
	pEdcaParams->acbk = pUpdatedEdcaParams[EDCA_AC_BK];
	pEdcaParams->acvi = pUpdatedEdcaParams[EDCA_AC_VI];
	pEdcaParams->acvo = pUpdatedEdcaParams[EDCA_AC_VO];
	msgQ.type = WMA_UPDATE_EDCA_PROFILE_IND;
	msgQ.reserved = 0;
	msgQ.bodyptr = pEdcaParams;
	msgQ.bodyval = 0;
	PELOG1(lim_log(pMac, LOG1,
	       FL("Sending WMA_UPDATE_EDCA_PROFILE_IND, EDCA Parameters:"));)
	for (i = 0; i < MAX_NUM_AC; i++) {
		PELOG1(lim_log(pMac, LOG1,
		       FL("AC[%d]:  AIFSN %d, ACM %d, CWmin %d, CWmax %d, TxOp %d "),
		       i, pUpdatedEdcaParams[i].aci.aifsn,
		       pUpdatedEdcaParams[i].aci.acm,
		       pUpdatedEdcaParams[i].cw.min,
		       pUpdatedEdcaParams[i].cw.max,
		       pUpdatedEdcaParams[i].txoplimit);)
	}
	MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		qdf_mem_free(pEdcaParams);
		lim_log(pMac, LOGP,
			FL("Posting WMA_UPDATE_EDCA_PROFILE_IND failed, reason=%X"),
			retCode);
	}
	return retCode;
}

/**
 * lim_set_active_edca_params() - Choose best EDCA parameters
 *
 * @mac_ctx:  pointer to Global Mac structure.
 * @edca_params: pointer to the local EDCA parameters
 * @pe_session: point to the session entry
 *
 *  This function is called to set the most up-to-date EDCA parameters
 *  given the default local EDCA parameters.  The rules are as following:
 *  - If ACM bit is set for all ACs, then downgrade everything to Best Effort.
 *  - If ACM is not set for any AC, then PE will use the default EDCA
 *    parameters as advertised by AP.
 *  - If ACM is set in any of the ACs, PE will use the EDCA parameters
 *    from the next best AC for which ACM is not enabled.
 *
 * Return: none
 */

void lim_set_active_edca_params(tpAniSirGlobal mac_ctx,
				tSirMacEdcaParamRecord *edca_params,
				tpPESession pe_session)
{
	uint8_t ac, new_ac, i;
	uint8_t ac_admitted;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	host_log_qos_edca_pkt_type *log_ptr = NULL;
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
	/* Initialize gLimEdcaParamsActive[] to be same as localEdcaParams */
	pe_session->gLimEdcaParamsActive[EDCA_AC_BE] = edca_params[EDCA_AC_BE];
	pe_session->gLimEdcaParamsActive[EDCA_AC_BK] = edca_params[EDCA_AC_BK];
	pe_session->gLimEdcaParamsActive[EDCA_AC_VI] = edca_params[EDCA_AC_VI];
	pe_session->gLimEdcaParamsActive[EDCA_AC_VO] = edca_params[EDCA_AC_VO];
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
	lim_log(mac_ctx, LOG1, FL("adAdmitMask[UPLINK] = 0x%x "),
		pe_session->gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK]);
	lim_log(mac_ctx, LOG1, FL("adAdmitMask[DOWNLINK] = 0x%x "),
		pe_session->gAcAdmitMask[SIR_MAC_DIRECTION_DNLINK]);
	for (ac = EDCA_AC_BK; ac <= EDCA_AC_VO; ac++) {
		ac_admitted =
			((pe_session->gAcAdmitMask[SIR_MAC_DIRECTION_UPLINK] &
			 (1 << ac)) >> ac);

		lim_log(mac_ctx, LOG1,
			FL("For AC[%d]: acm=%d,  ac_admitted=%d "),
			ac, edca_params[ac].aci.acm, ac_admitted);
		if ((edca_params[ac].aci.acm == 1) && (ac_admitted == 0)) {
			lim_log(mac_ctx, LOG1,
				FL("We need to downgrade AC %d!! "), ac);
			/* Loop backwards through AC values until it finds
			 * acm == 0 or reaches EDCA_AC_BE.
			 * Note that for block has no executable statements.
			 */
			for (i = ac - 1;
			    (i > EDCA_AC_BE &&
				(edca_params[i].aci.acm != 0));
			     i--)
				;
			new_ac = i;
			lim_log(mac_ctx, LOGW,
				FL("Downgrading AC %d ---> AC %d "),
				ac, new_ac);
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

		rec = &pe_session->gLimEdcaParamsActive[EDCA_AC_BE];
		log_ptr->aci_be = rec->aci.aci;
		log_ptr->cw_be = rec->cw.max << 4 | rec->cw.min;
		log_ptr->txoplimit_be = rec->txoplimit;

		rec = &pe_session->gLimEdcaParamsActive[EDCA_AC_BK];
		log_ptr->aci_bk = rec->aci.aci;
		log_ptr->cw_bk = rec->cw.max << 4 | rec->cw.min;
		log_ptr->txoplimit_bk = rec->txoplimit;

		rec = &pe_session->gLimEdcaParamsActive[EDCA_AC_VI];
		log_ptr->aci_vi = rec->aci.aci;
		log_ptr->cw_vi = rec->cw.max << 4 | rec->cw.min;
		log_ptr->txoplimit_vi = rec->txoplimit;

		rec = &pe_session->gLimEdcaParamsActive[EDCA_AC_VO];
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
   \param   tpAniSirGlobal  pMac
   \param   tSirLinkState      state
   \return  None
   -----------------------------------------------------------*/
tSirRetStatus lim_set_link_state(tpAniSirGlobal pMac, tSirLinkState state,
				 tSirMacAddr bssId, tSirMacAddr selfMacAddr,
				 tpSetLinkStateCallback callback,
				 void *callbackArg)
{
	tSirMsgQ msgQ;
	tSirRetStatus retCode;
	tpLinkStateParams pLinkStateParams = NULL;
	/* Allocate memory. */
	pLinkStateParams = qdf_mem_malloc(sizeof(tLinkStateParams));
	if (NULL == pLinkStateParams) {
		lim_log(pMac, LOGP,
			FL
				("Unable to allocate memory while sending Set Link State"));
		retCode = eSIR_MEM_ALLOC_FAILED;
		return retCode;
	}
	qdf_mem_set((uint8_t *) pLinkStateParams, sizeof(tLinkStateParams), 0);
	pLinkStateParams->state = state;
	pLinkStateParams->callback = callback;
	pLinkStateParams->callbackArg = callbackArg;

	/* Copy Mac address */
	sir_copy_mac_addr(pLinkStateParams->bssid, bssId);
	sir_copy_mac_addr(pLinkStateParams->selfMacAddr, selfMacAddr);

	msgQ.type = WMA_SET_LINK_STATE;
	msgQ.reserved = 0;
	msgQ.bodyptr = pLinkStateParams;
	msgQ.bodyval = 0;

	MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msgQ.type));

	retCode = (uint32_t) wma_post_ctrl_msg(pMac, &msgQ);
	if (retCode != eSIR_SUCCESS) {
		qdf_mem_free(pLinkStateParams);
		lim_log(pMac, LOGP,
			FL("Posting link state %d failed, reason = %x "), state,
			retCode);
	}
	return retCode;
}

extern tSirRetStatus lim_set_link_state_ft(tpAniSirGlobal pMac, tSirLinkState
					   state, tSirMacAddr bssId,
					   tSirMacAddr selfMacAddr, int ft,
					   tpPESession psessionEntry)
{
	tSirMsgQ msgQ;
	tSirRetStatus retCode;
	tpLinkStateParams pLinkStateParams = NULL;
	/* Allocate memory. */
	pLinkStateParams = qdf_mem_malloc(sizeof(tLinkStateParams));
	if (NULL == pLinkStateParams) {
		lim_log(pMac, LOGP,
			FL
				("Unable to allocate memory while sending Set Link State"));
		retCode = eSIR_MEM_ALLOC_FAILED;
		return retCode;
	}
	qdf_mem_set((uint8_t *) pLinkStateParams, sizeof(tLinkStateParams), 0);
	pLinkStateParams->state = state;
	/* Copy Mac address */
	sir_copy_mac_addr(pLinkStateParams->bssid, bssId);
	sir_copy_mac_addr(pLinkStateParams->selfMacAddr, selfMacAddr);
	pLinkStateParams->ft = 1;
	pLinkStateParams->session = psessionEntry;

	msgQ.type = WMA_SET_LINK_STATE;
	msgQ.reserved = 0;
	msgQ.bodyptr = pLinkStateParams;
	msgQ.bodyval = 0;
	if (NULL == psessionEntry) {
		MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msgQ.type));
	} else {
		MTRACE(mac_trace_msg_tx
			       (pMac, psessionEntry->peSessionId, msgQ.type));
	}

	retCode = (uint32_t) wma_post_ctrl_msg(pMac, &msgQ);
	if (retCode != eSIR_SUCCESS) {
		qdf_mem_free(pLinkStateParams);
		lim_log(pMac, LOGP,
			FL("Posting link state %d failed, reason = %x "), state,
			retCode);
	}
	return retCode;
}

/** ---------------------------------------------------------
   \fn      lim_send_beacon_filter_info
   \brief   LIM sends beacon filtering info to WMA
   \param   tpAniSirGlobal  pMac
   \return  None
   -----------------------------------------------------------*/
tSirRetStatus lim_send_beacon_filter_info(tpAniSirGlobal pMac,
					  tpPESession psessionEntry)
{
	tpBeaconFilterMsg pBeaconFilterMsg = NULL;
	tSirRetStatus retCode = eSIR_SUCCESS;
	tSirMsgQ msgQ;
	uint8_t *ptr;
	uint32_t i;
	uint32_t msgSize;
	tpBeaconFilterIe pIe;

	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE, FL("Fail to find the right session "));
		retCode = eSIR_FAILURE;
		return retCode;
	}
	msgSize = sizeof(tBeaconFilterMsg) + sizeof(beacon_filter_table);
	pBeaconFilterMsg = qdf_mem_malloc(msgSize);
	if (NULL == pBeaconFilterMsg) {
		lim_log(pMac, LOGP,
			FL("Fail to allocate memory for beaconFiilterMsg "));
		retCode = eSIR_MEM_ALLOC_FAILED;
		return retCode;
	}
	qdf_mem_set((uint8_t *) pBeaconFilterMsg, msgSize, 0);
	/* Fill in capability Info and mask */
	/* Don't send this message if no active Infra session is found. */
	pBeaconFilterMsg->capabilityInfo = psessionEntry->limCurrentBssCaps;
	pBeaconFilterMsg->capabilityMask = CAPABILITY_FILTER_MASK;
	pBeaconFilterMsg->beaconInterval =
		(uint16_t) psessionEntry->beaconParams.beaconInterval;
	/* Fill in number of IEs in beacon_filter_table */
	pBeaconFilterMsg->ieNum =
		(uint16_t) (sizeof(beacon_filter_table) / sizeof(tBeaconFilterIe));
	/* Fill the BSSIDX */
	pBeaconFilterMsg->bssIdx = psessionEntry->bssIdx;

	/* Fill message with info contained in the beacon_filter_table */
	ptr = (uint8_t *) pBeaconFilterMsg + sizeof(tBeaconFilterMsg);
	for (i = 0; i < (pBeaconFilterMsg->ieNum); i++) {
		pIe = (tpBeaconFilterIe) ptr;
		pIe->elementId = beacon_filter_table[i].elementId;
		pIe->checkIePresence = beacon_filter_table[i].checkIePresence;
		pIe->byte.offset = beacon_filter_table[i].byte.offset;
		pIe->byte.value = beacon_filter_table[i].byte.value;
		pIe->byte.bitMask = beacon_filter_table[i].byte.bitMask;
		pIe->byte.ref = beacon_filter_table[i].byte.ref;
		ptr += sizeof(tBeaconFilterIe);
	}
	msgQ.type = WMA_BEACON_FILTER_IND;
	msgQ.reserved = 0;
	msgQ.bodyptr = pBeaconFilterMsg;
	msgQ.bodyval = 0;
	lim_log(pMac, LOG3, FL("Sending WMA_BEACON_FILTER_IND..."));
	MTRACE(mac_trace_msg_tx(pMac, psessionEntry->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		qdf_mem_free(pBeaconFilterMsg);
		lim_log(pMac, LOGP,
			FL("Posting WMA_BEACON_FILTER_IND failed, reason=%X"),
			retCode);
		return retCode;
	}
	return retCode;
}

tSirRetStatus lim_send_mode_update(tpAniSirGlobal pMac,
				   tUpdateVHTOpMode *pTempParam,
				   tpPESession psessionEntry)
{
	tUpdateVHTOpMode *pVhtOpMode = NULL;
	tSirRetStatus retCode = eSIR_SUCCESS;
	tSirMsgQ msgQ;

	pVhtOpMode = qdf_mem_malloc(sizeof(tUpdateVHTOpMode));
	if (NULL == pVhtOpMode) {
		lim_log(pMac, LOGP,
			FL("Unable to allocate memory during Update Op Mode"));
		return eSIR_MEM_ALLOC_FAILED;
	}
	qdf_mem_copy((uint8_t *) pVhtOpMode, pTempParam,
		     sizeof(tUpdateVHTOpMode));
	msgQ.type = WMA_UPDATE_OP_MODE;
	msgQ.reserved = 0;
	msgQ.bodyptr = pVhtOpMode;
	msgQ.bodyval = 0;
	lim_log(pMac, LOG3, FL(
			"Sending WMA_UPDATE_OP_MODE, op_mode %d, sta_id %d"),
			pVhtOpMode->opMode, pVhtOpMode->staId);
	if (NULL == psessionEntry)
		MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msgQ.type));
	else
		MTRACE(mac_trace_msg_tx(pMac,
					psessionEntry->peSessionId,
					msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		qdf_mem_free(pVhtOpMode);
		lim_log(pMac, LOGP,
			FL("Posting WMA_UPDATE_OP_MODE failed, reason=%X"),
			retCode);
	}

	return retCode;
}

tSirRetStatus lim_send_rx_nss_update(tpAniSirGlobal pMac,
				     tUpdateRxNss *pTempParam,
				     tpPESession psessionEntry)
{
	tUpdateRxNss *pRxNss = NULL;
	tSirRetStatus retCode = eSIR_SUCCESS;
	tSirMsgQ msgQ;

	pRxNss = qdf_mem_malloc(sizeof(tUpdateRxNss));
	if (NULL == pRxNss) {
		lim_log(pMac, LOGP,
			FL("Unable to allocate memory during Update Rx Nss"));
		return eSIR_MEM_ALLOC_FAILED;
	}
	qdf_mem_copy((uint8_t *) pRxNss, pTempParam, sizeof(tUpdateRxNss));
	msgQ.type = WMA_UPDATE_RX_NSS;
	msgQ.reserved = 0;
	msgQ.bodyptr = pRxNss;
	msgQ.bodyval = 0;
	PELOG3(lim_log(pMac, LOG3, FL("Sending WMA_UPDATE_RX_NSS"));)
	if (NULL == psessionEntry)
		MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msgQ.type));
	else
		MTRACE(mac_trace_msg_tx(pMac,
					psessionEntry->peSessionId,
					msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		qdf_mem_free(pRxNss);
		lim_log(pMac, LOGP,
			FL("Posting WMA_UPDATE_RX_NSS failed, reason=%X"),
			retCode);
	}

	return retCode;
}

tSirRetStatus lim_set_membership(tpAniSirGlobal pMac,
				 tUpdateMembership *pTempParam,
				 tpPESession psessionEntry)
{
	tUpdateMembership *pMembership = NULL;
	tSirRetStatus retCode = eSIR_SUCCESS;
	tSirMsgQ msgQ;

	pMembership = qdf_mem_malloc(sizeof(tUpdateMembership));
	if (NULL == pMembership) {
		lim_log(pMac, LOGP,
			FL("Unable to allocate memory during Update Membership Mode"));
		return eSIR_MEM_ALLOC_FAILED;
	}
	qdf_mem_copy((uint8_t *) pMembership, pTempParam,
		     sizeof(tUpdateMembership));

	msgQ.type = WMA_UPDATE_MEMBERSHIP;
	msgQ.reserved = 0;
	msgQ.bodyptr = pMembership;
	msgQ.bodyval = 0;
	PELOG3(lim_log(pMac, LOG3, FL("Sending WMA_UPDATE_MEMBERSHIP"));)
	if (NULL == psessionEntry)
		MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msgQ.type));
	else
		MTRACE(mac_trace_msg_tx(pMac,
					psessionEntry->peSessionId,
					msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		qdf_mem_free(pMembership);
		lim_log(pMac, LOGP,
			FL("Posting WMA_UPDATE_MEMBERSHIP failed, reason=%X"),
			retCode);
	}

	return retCode;
}

tSirRetStatus lim_set_user_pos(tpAniSirGlobal pMac,
			       tUpdateUserPos *pTempParam,
			       tpPESession psessionEntry)
{
	tUpdateUserPos *pUserPos = NULL;
	tSirRetStatus retCode = eSIR_SUCCESS;
	tSirMsgQ msgQ;

	pUserPos = qdf_mem_malloc(sizeof(tUpdateUserPos));
	if (NULL == pUserPos) {
		lim_log(pMac, LOGP,
			FL("Unable to allocate memory during Update User Position"));
		return eSIR_MEM_ALLOC_FAILED;
	}
	qdf_mem_copy((uint8_t *) pUserPos, pTempParam, sizeof(tUpdateUserPos));

	msgQ.type = WMA_UPDATE_USERPOS;
	msgQ.reserved = 0;
	msgQ.bodyptr = pUserPos;
	msgQ.bodyval = 0;
	PELOG3(lim_log(pMac, LOG3, FL("Sending WMA_UPDATE_USERPOS"));)
	if (NULL == psessionEntry)
		MTRACE(mac_trace_msg_tx(pMac, NO_SESSION, msgQ.type));
	else
		MTRACE(mac_trace_msg_tx(pMac,
					psessionEntry->peSessionId,
					msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		qdf_mem_free(pUserPos);
		lim_log(pMac, LOGP,
			FL("Posting WMA_UPDATE_USERPOS failed, reason=%X"),
			retCode);
	}

	return retCode;
}

#ifdef WLAN_FEATURE_11W
/**
 * lim_send_exclude_unencrypt_ind() - sends WMA_EXCLUDE_UNENCRYPTED_IND to HAL
 * @pMac:          mac global context
 * @excludeUnenc:  true: ignore, false: indicate
 * @psessionEntry: session context
 *
 * LIM sends a message to HAL to indicate whether to ignore or indicate the
 * unprotected packet error.
 *
 * Return: status of operation
 */
tSirRetStatus lim_send_exclude_unencrypt_ind(tpAniSirGlobal pMac,
					     bool excludeUnenc,
					     tpPESession psessionEntry)
{
	tSirRetStatus retCode = eSIR_SUCCESS;
	tSirMsgQ msgQ;
	tSirWlanExcludeUnencryptParam *pExcludeUnencryptParam;

	pExcludeUnencryptParam =
		qdf_mem_malloc(sizeof(tSirWlanExcludeUnencryptParam));
	if (NULL == pExcludeUnencryptParam) {
		lim_log(pMac, LOGP,
			FL("Unable to allocate memory during lim_send_exclude_unencrypt_ind"));
		return eSIR_MEM_ALLOC_FAILED;
	}

	pExcludeUnencryptParam->excludeUnencrypt = excludeUnenc;
	qdf_mem_copy(pExcludeUnencryptParam->bssid.bytes, psessionEntry->bssId,
			QDF_MAC_ADDR_SIZE);

	msgQ.type = WMA_EXCLUDE_UNENCRYPTED_IND;
	msgQ.reserved = 0;
	msgQ.bodyptr = pExcludeUnencryptParam;
	msgQ.bodyval = 0;
	PELOG3(lim_log(pMac, LOG3, FL("Sending WMA_EXCLUDE_UNENCRYPTED_IND"));)
	MTRACE(mac_trace_msg_tx(pMac, psessionEntry->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		qdf_mem_free(pExcludeUnencryptParam);
		lim_log(pMac, LOGP,
			FL("Posting WMA_EXCLUDE_UNENCRYPTED_IND failed, reason=%X"),
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
tSirRetStatus lim_send_ht40_obss_scanind(tpAniSirGlobal mac_ctx,
						struct sPESession *session)
{
	enum eSirRetStatus ret = eSIR_SUCCESS;
	struct obss_ht40_scanind *ht40_obss_scanind;
	uint32_t channelnum;
	struct sSirMsgQ msg;
	uint8_t chan_list[WNI_CFG_VALID_CHANNEL_LIST_LEN];
	uint8_t channel24gnum, count;

	ht40_obss_scanind = qdf_mem_malloc(sizeof(struct obss_ht40_scanind));
	if (NULL == ht40_obss_scanind) {
		QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
			"Memory allocation failed");
		return eSIR_FAILURE;
	}
	QDF_TRACE(QDF_MODULE_ID_PE, QDF_TRACE_LEVEL_ERROR,
		"OBSS Scan Indication bssIdx- %d staId %d",
		session->bssIdx, session->staId);

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
		cds_reg_dmn_get_opclass_from_channel(
			mac_ctx->scan.countryCodeCurrent,
			session->currentOperChannel,
			session->ch_width);
	channelnum = WNI_CFG_VALID_CHANNEL_LIST_LEN;
	if (wlan_cfg_get_str(mac_ctx, WNI_CFG_VALID_CHANNEL_LIST,
			chan_list, &channelnum) != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGE,
			FL("could not retrieve Valid channel list"));
		qdf_mem_free(ht40_obss_scanind);
		return eSIR_FAILURE;
	}
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
	ht40_obss_scanind->bss_id = session->bssIdx;
	ht40_obss_scanind->fortymhz_intolerent = 0;
	ht40_obss_scanind->iefield_len = 0;
	msg.type = WMA_HT40_OBSS_SCAN_IND;
	msg.reserved = 0;
	msg.bodyptr = (void *)ht40_obss_scanind;
	msg.bodyval = 0;
	lim_log(mac_ctx, LOG1,
		FL("Sending WDA_HT40_OBSS_SCAN_IND to WDA"
		"Obss Scan trigger width = %d, delay factor = %d"),
		ht40_obss_scanind->obss_width_trigger_interval,
		ht40_obss_scanind->bsswidth_ch_trans_delay);
	ret = wma_post_ctrl_msg(mac_ctx, &msg);
	if (eSIR_SUCCESS != ret) {
		lim_log(mac_ctx, LOGP,
			FL("WDA_HT40_OBSS_SCAN_IND msg failed, reason=%X"),
			ret);
		qdf_mem_free(ht40_obss_scanind);
	}
	return ret;
}
