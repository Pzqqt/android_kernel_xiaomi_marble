/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

#ifdef WLAN_FEATURE_VOWIFI_11R
/**=========================================================================

   \brief implementation for PE 11r VoWiFi FT Protocol

   ========================================================================*/

/* $Header$ */

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <lim_send_messages.h>
#include <lim_types.h>
#include <lim_ft.h>
#include <lim_ft_defs.h>
#include <lim_utils.h>
#include <lim_prop_exts_utils.h>
#include <lim_assoc_utils.h>
#include <lim_session.h>
#include <lim_admit_control.h>
#include "wmm_apsd.h"
#include "wma.h"

extern void lim_send_set_sta_key_req(tpAniSirGlobal pMac,
				     tLimMlmSetKeysReq * pMlmSetKeysReq,
				     uint16_t staIdx,
				     uint8_t defWEPIdx,
				     tpPESession sessionEntry, bool sendRsp);

/*--------------------------------------------------------------------------
   Initialize the FT variables.
   ------------------------------------------------------------------------*/
void lim_ft_open(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	if (psessionEntry)
		cdf_mem_set(&psessionEntry->ftPEContext, sizeof(tftPEContext),
			    0);
}

/*--------------------------------------------------------------------------
   Cleanup FT variables.
   ------------------------------------------------------------------------*/
void lim_ft_cleanup_pre_auth_info(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	tpPESession pReAssocSessionEntry = NULL;
	uint8_t sessionId = 0;

	if (!psessionEntry) {
		lim_log(pMac, LOGE, FL("psessionEntry is NULL"));
		return;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return;
	}

	if (psessionEntry->ftPEContext.pFTPreAuthReq) {
		pReAssocSessionEntry =
			pe_find_session_by_bssid(pMac,
						 psessionEntry->ftPEContext.
						 pFTPreAuthReq->preAuthbssId,
						 &sessionId);

		lim_log(pMac, LOG1, FL("Freeing pFTPreAuthReq= %p"),
			       psessionEntry->ftPEContext.pFTPreAuthReq);
		if (psessionEntry->ftPEContext.pFTPreAuthReq->
		    pbssDescription) {
			cdf_mem_free(psessionEntry->ftPEContext.pFTPreAuthReq->
				     pbssDescription);
			psessionEntry->ftPEContext.pFTPreAuthReq->
			pbssDescription = NULL;
		}
		cdf_mem_free(psessionEntry->ftPEContext.pFTPreAuthReq);
		psessionEntry->ftPEContext.pFTPreAuthReq = NULL;
	}

	if (psessionEntry->ftPEContext.pAddBssReq) {
		cdf_mem_free(psessionEntry->ftPEContext.pAddBssReq);
		psessionEntry->ftPEContext.pAddBssReq = NULL;
	}

	if (psessionEntry->ftPEContext.pAddStaReq) {
		cdf_mem_free(psessionEntry->ftPEContext.pAddStaReq);
		psessionEntry->ftPEContext.pAddStaReq = NULL;
	}

	/* The session is being deleted, cleanup the contents */
	cdf_mem_set(&psessionEntry->ftPEContext, sizeof(tftPEContext), 0);

	/* Delete the session created while handling pre-auth response */
	if (pReAssocSessionEntry) {
		/* If we have successful pre-auth response, then we would have
		 * created a session on which reassoc request will be sent
		 */
		if (pReAssocSessionEntry->valid &&
		    pReAssocSessionEntry->limSmeState ==
		    eLIM_SME_WT_REASSOC_STATE) {
			CDF_TRACE(QDF_MODULE_ID_PE,
				  CDF_TRACE_LEVEL_DEBUG,
				  FL("Deleting Preauth session(%d)"),
				  pReAssocSessionEntry->peSessionId);
			pe_delete_session(pMac, pReAssocSessionEntry);
		}
	}
}

void lim_ft_cleanup_all_ft_sessions(tpAniSirGlobal pMac)
{
	/* Wrapper function to cleanup all FT sessions */
	int i;

	for (i = 0; i < pMac->lim.maxBssId; i++) {
		if (true == pMac->lim.gpSession[i].valid) {
			/* The session is valid, may have FT data */
			lim_ft_cleanup(pMac, &pMac->lim.gpSession[i]);
		}
	}
}

void lim_ft_cleanup(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	if (NULL == psessionEntry) {
		PELOGE(lim_log(pMac, LOGE, FL("psessionEntry is NULL"));)
		return;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return;
	}

	if (NULL != psessionEntry->ftPEContext.pFTPreAuthReq) {
		lim_log(pMac, LOG1, FL("Freeing pFTPreAuthReq= %p"),
			       psessionEntry->ftPEContext.pFTPreAuthReq);
		if (NULL !=
		    psessionEntry->ftPEContext.pFTPreAuthReq->
		    pbssDescription) {
			cdf_mem_free(psessionEntry->ftPEContext.pFTPreAuthReq->
				     pbssDescription);
			psessionEntry->ftPEContext.pFTPreAuthReq->
			pbssDescription = NULL;
		}
		cdf_mem_free(psessionEntry->ftPEContext.pFTPreAuthReq);
		psessionEntry->ftPEContext.pFTPreAuthReq = NULL;
	}

	if (psessionEntry->ftPEContext.pAddBssReq) {
		cdf_mem_free(psessionEntry->ftPEContext.pAddBssReq);
		psessionEntry->ftPEContext.pAddBssReq = NULL;
	}

	if (psessionEntry->ftPEContext.pAddStaReq) {
		cdf_mem_free(psessionEntry->ftPEContext.pAddStaReq);
		psessionEntry->ftPEContext.pAddStaReq = NULL;
	}

	/* The session is being deleted, cleanup the contents */
	cdf_mem_set(&psessionEntry->ftPEContext, sizeof(tftPEContext), 0);
}

/*
 * lim_process_ft_pre_auth_req() - process ft pre auth req
 *
 * @mac_ctx:    global mac ctx
 * @msg:        pointer to message
 *
 * In this function, we process the FT Pre Auth Req:
 *   We receive Pre-Auth, suspend link, register a call back. In the call back,
 *   we will need to accept frames from the new bssid. Send out the auth req to
 *   new AP. Start timer and when the timer is done or if we receive the Auth
 *   response. We change channel. Resume link
 *
 * Return: value to indicate if buffer was consumed
 */
int lim_process_ft_pre_auth_req(tpAniSirGlobal mac_ctx, tpSirMsgQ msg)
{
	int buf_consumed = false;
	tpPESession session;
	uint8_t session_id;
	tpSirFTPreAuthReq ft_pre_auth_req = (tSirFTPreAuthReq *) msg->bodyptr;

	if (NULL == ft_pre_auth_req) {
		lim_log(mac_ctx, LOGE, FL("tSirFTPreAuthReq is NULL"));
		return buf_consumed;
	}

	/* Get the current session entry */
	session = pe_find_session_by_bssid(mac_ctx,
					   ft_pre_auth_req->currbssId,
					   &session_id);
	if (session == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("Unable to find session for the bssid"
			   MAC_ADDRESS_STR),
			   MAC_ADDR_ARRAY(ft_pre_auth_req->currbssId));
		/* Post the FT Pre Auth Response to SME */
		lim_post_ft_pre_auth_rsp(mac_ctx, eSIR_FAILURE, NULL, 0,
					 session);
		/*
		 * return FALSE, since the Pre-Auth Req will be freed in
		 * limPostFTPreAuthRsp on failure
		 */
		return buf_consumed;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(session)) {
		lim_log(mac_ctx, LOGE, FL("session is not in STA mode"));
		buf_consumed = true;
		return buf_consumed;
	}

	/* Can set it only after sending auth */
	session->ftPEContext.ftPreAuthStatus = eSIR_FAILURE;
	session->ftPEContext.ftPreAuthSession = true;

	/* Indicate that this is the session on which preauth is being done */
	if (session->ftPEContext.pFTPreAuthReq) {
		if (session->ftPEContext.pFTPreAuthReq->pbssDescription) {
			cdf_mem_free(
			  session->ftPEContext.pFTPreAuthReq->pbssDescription);
			session->ftPEContext.pFTPreAuthReq->pbssDescription =
									NULL;
		}
		cdf_mem_free(session->ftPEContext.pFTPreAuthReq);
		session->ftPEContext.pFTPreAuthReq = NULL;
	}

	/* We need information from the Pre-Auth Req. Lets save that */
	session->ftPEContext.pFTPreAuthReq = ft_pre_auth_req;

	lim_log(mac_ctx, LOG1, FL("PRE Auth ft_ies_length=%02x%02x%02x"),
		session->ftPEContext.pFTPreAuthReq->ft_ies[0],
		session->ftPEContext.pFTPreAuthReq->ft_ies[1],
		session->ftPEContext.pFTPreAuthReq->ft_ies[2]);
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_PRE_AUTH_REQ_EVENT,
			      session, 0, 0);
#endif

	/* Dont need to suspend if APs are in same channel */
	if (session->currentOperChannel !=
	    session->ftPEContext.pFTPreAuthReq->preAuthchannelNum) {
		/* Need to suspend link only if the channels are different */
		lim_log(mac_ctx, LOG2,
			FL("Performing pre-auth on diff channel(session %p)"),
			session);
		lim_send_preauth_scan_offload(mac_ctx, session->peSessionId,
				session->ftPEContext.pFTPreAuthReq);
	} else {
		lim_log(mac_ctx, LOG2,
			FL("Performing pre-auth on same channel (session %p)"),
			session);
		/* We are in the same channel. Perform pre-auth */
		lim_perform_ft_pre_auth(mac_ctx, QDF_STATUS_SUCCESS, NULL,
					session);
	}

	return buf_consumed;
}

/*------------------------------------------------------------------
 * Send the Auth1
 * Receive back Auth2
 *------------------------------------------------------------------*/
void lim_perform_ft_pre_auth(tpAniSirGlobal pMac, QDF_STATUS status,
			     uint32_t *data, tpPESession psessionEntry)
{
	tSirMacAuthFrameBody authFrame;

	if (NULL == psessionEntry) {
		PELOGE(lim_log(pMac, LOGE, FL("psessionEntry is NULL"));)
		return;
	}

	if (psessionEntry->is11Rconnection &&
	    psessionEntry->ftPEContext.pFTPreAuthReq) {
		/* Only 11r assoc has FT IEs */
		if (psessionEntry->ftPEContext.pFTPreAuthReq->ft_ies == NULL) {
			lim_log(pMac, LOGE,
				FL("FTIEs for Auth Req Seq 1 is absent"));
			goto preauth_fail;
		}
	}

	if (status != QDF_STATUS_SUCCESS) {
		lim_log(pMac, LOGE,
			FL(" Change channel not successful for FT pre-auth"));
		goto preauth_fail;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return;
	}
	lim_log(pMac, LOG2, "Entered wait auth2 state for FT (old session %p)",
				 psessionEntry);
	if (psessionEntry->is11Rconnection) {
		/* Now we are on the right channel and need to send out Auth1 and
		 * receive Auth2
		 */
		authFrame.authAlgoNumber = eSIR_FT_AUTH;
	} else {
		/* Will need to make isESEconnection a enum may be for further
		 * improvements to this to match this algorithm number
		 */
		authFrame.authAlgoNumber = eSIR_OPEN_SYSTEM;
	}
	authFrame.authTransactionSeqNumber = SIR_MAC_AUTH_FRAME_1;
	authFrame.authStatusCode = 0;

	/* Start timer here to come back to operating channel */
	pMac->lim.limTimers.gLimFTPreAuthRspTimer.sessionId =
		psessionEntry->peSessionId;
	if (TX_SUCCESS !=
	    tx_timer_activate(&pMac->lim.limTimers.gLimFTPreAuthRspTimer)) {
		lim_log(pMac, LOGE, FL("FT Auth Rsp Timer Start Failed"));
		goto preauth_fail;
	}
	MTRACE(mac_trace(pMac, TRACE_CODE_TIMER_ACTIVATE,
		psessionEntry->peSessionId, eLIM_FT_PREAUTH_RSP_TIMER));

	lim_log(pMac, LOG1, FL("FT Auth Rsp Timer Started"));
#ifdef FEATURE_WLAN_DIAG_SUPPORT
	lim_diag_event_report(pMac, WLAN_PE_DIAG_ROAM_AUTH_START_EVENT,
			pMac->lim.pSessionEntry, eSIR_SUCCESS, eSIR_SUCCESS);
#endif

	lim_send_auth_mgmt_frame(pMac, &authFrame,
		 psessionEntry->ftPEContext.pFTPreAuthReq->preAuthbssId,
		 LIM_NO_WEP_IN_FC, psessionEntry);

	return;

preauth_fail:
	lim_handle_ft_pre_auth_rsp(pMac, eSIR_FAILURE, NULL, 0, psessionEntry);
	return;
}

/*------------------------------------------------------------------
 *
 * Create the new Add Bss Req to the new AP.
 * This will be used when we are ready to FT to the new AP.
 * The newly created ft Session entry is passed to this function
 *
 *------------------------------------------------------------------*/
tSirRetStatus lim_ft_prepare_add_bss_req(tpAniSirGlobal pMac,
					 uint8_t updateEntry,
					 tpPESession pftSessionEntry,
					 tpSirBssDescription bssDescription)
{
	tpAddBssParams pAddBssParams = NULL;
	tAddStaParams *sta_ctx;
	uint8_t chanWidthSupp = 0;
	tSchBeaconStruct *pBeaconStruct;

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(pftSessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return eSIR_FAILURE;
	}

	pBeaconStruct = cdf_mem_malloc(sizeof(tSchBeaconStruct));
	if (NULL == pBeaconStruct) {
		lim_log(pMac, LOGE,
			FL("Unable to allocate memory for creating ADD_BSS"));
		return eSIR_MEM_ALLOC_FAILED;
	}
	/* Package SIR_HAL_ADD_BSS_REQ message parameters */
	pAddBssParams = cdf_mem_malloc(sizeof(tAddBssParams));
	if (NULL == pAddBssParams) {
		cdf_mem_free(pBeaconStruct);
		lim_log(pMac, LOGP,
			FL("Unable to allocate memory for creating ADD_BSS"));
		return eSIR_MEM_ALLOC_FAILED;
	}

	cdf_mem_set((uint8_t *) pAddBssParams, sizeof(tAddBssParams), 0);

	lim_extract_ap_capabilities(pMac, (uint8_t *) bssDescription->ieFields,
			lim_get_ielen_from_bss_description(bssDescription),
			pBeaconStruct);

	if (pMac->lim.gLimProtectionControl !=
	    WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
		lim_decide_sta_protection_on_assoc(pMac, pBeaconStruct,
						   pftSessionEntry);

	cdf_mem_copy(pAddBssParams->bssId, bssDescription->bssId,
		     sizeof(tSirMacAddr));

	/* Fill in tAddBssParams selfMacAddr */
	cdf_mem_copy(pAddBssParams->selfMacAddr, pftSessionEntry->selfMacAddr,
		     sizeof(tSirMacAddr));

	pAddBssParams->bssType = pftSessionEntry->bssType;
	pAddBssParams->operMode = BSS_OPERATIONAL_MODE_STA;

	pAddBssParams->beaconInterval = bssDescription->beaconInterval;

	pAddBssParams->dtimPeriod = pBeaconStruct->tim.dtimPeriod;
	pAddBssParams->updateBss = updateEntry;

	pAddBssParams->reassocReq = true;

	pAddBssParams->cfParamSet.cfpCount = pBeaconStruct->cfParamSet.cfpCount;
	pAddBssParams->cfParamSet.cfpPeriod =
		pBeaconStruct->cfParamSet.cfpPeriod;
	pAddBssParams->cfParamSet.cfpMaxDuration =
		pBeaconStruct->cfParamSet.cfpMaxDuration;
	pAddBssParams->cfParamSet.cfpDurRemaining =
		pBeaconStruct->cfParamSet.cfpDurRemaining;

	pAddBssParams->rateSet.numRates =
		pBeaconStruct->supportedRates.numRates;
	cdf_mem_copy(pAddBssParams->rateSet.rate,
		     pBeaconStruct->supportedRates.rate,
		     pBeaconStruct->supportedRates.numRates);

	pAddBssParams->nwType = bssDescription->nwType;

	pAddBssParams->shortSlotTimeSupported =
		(uint8_t) pBeaconStruct->capabilityInfo.shortSlotTime;
	pAddBssParams->llaCoexist =
		(uint8_t) pftSessionEntry->beaconParams.llaCoexist;
	pAddBssParams->llbCoexist =
		(uint8_t) pftSessionEntry->beaconParams.llbCoexist;
	pAddBssParams->llgCoexist =
		(uint8_t) pftSessionEntry->beaconParams.llgCoexist;
	pAddBssParams->ht20Coexist =
		(uint8_t) pftSessionEntry->beaconParams.ht20Coexist;
#ifdef WLAN_FEATURE_11W
	pAddBssParams->rmfEnabled = pftSessionEntry->limRmfEnabled;
#endif

	/* Use the advertised capabilities from the received beacon/PR */
	if (IS_DOT11_MODE_HT(pftSessionEntry->dot11mode) &&
	    (pBeaconStruct->HTCaps.present)) {
		pAddBssParams->htCapable = pBeaconStruct->HTCaps.present;
		cdf_mem_copy(&pAddBssParams->staContext.capab_info,
			     &pBeaconStruct->capabilityInfo,
			     sizeof(pAddBssParams->staContext.capab_info));
		cdf_mem_copy(&pAddBssParams->staContext.ht_caps,
			     (uint8_t *) &pBeaconStruct->HTCaps +
			     sizeof(uint8_t),
			     sizeof(pAddBssParams->staContext.ht_caps));

		if (pBeaconStruct->HTInfo.present) {
			pAddBssParams->htOperMode =
				(tSirMacHTOperatingMode) pBeaconStruct->HTInfo.
				opMode;
			pAddBssParams->dualCTSProtection =
				(uint8_t) pBeaconStruct->HTInfo.dualCTSProtection;

			chanWidthSupp = lim_get_ht_capability(pMac,
							      eHT_SUPPORTED_CHANNEL_WIDTH_SET,
							      pftSessionEntry);
			if ((pBeaconStruct->HTCaps.supportedChannelWidthSet) &&
			    (chanWidthSupp)) {
				pAddBssParams->ch_width = (uint8_t)
					pBeaconStruct->HTInfo.recommendedTxWidthSet;
				if (pBeaconStruct->HTInfo.secondaryChannelOffset ==
						PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
					pAddBssParams->ch_center_freq_seg0 =
						bssDescription->channelId + 2;
				else if (pBeaconStruct->HTInfo.secondaryChannelOffset ==
						PHY_DOUBLE_CHANNEL_HIGH_PRIMARY)
					pAddBssParams->ch_center_freq_seg0 =
						bssDescription->channelId - 2;
			} else {
				pAddBssParams->ch_width = CH_WIDTH_20MHZ;
				pAddBssParams->ch_center_freq_seg0 = 0;
			}
			pAddBssParams->llnNonGFCoexist =
				(uint8_t) pBeaconStruct->HTInfo.nonGFDevicesPresent;
			pAddBssParams->fLsigTXOPProtectionFullSupport =
				(uint8_t) pBeaconStruct->HTInfo.
				lsigTXOPProtectionFullSupport;
			pAddBssParams->fRIFSMode =
				pBeaconStruct->HTInfo.rifsMode;
		}
	}

	pAddBssParams->currentOperChannel = bssDescription->channelId;
	pftSessionEntry->htSecondaryChannelOffset =
		pBeaconStruct->HTInfo.secondaryChannelOffset;
	sta_ctx = &pAddBssParams->staContext;

#ifdef WLAN_FEATURE_11AC
	if (pftSessionEntry->vhtCapability &&
	    pftSessionEntry->vhtCapabilityPresentInBeacon) {
		pAddBssParams->vhtCapable = pBeaconStruct->VHTCaps.present;
		if (pBeaconStruct->VHTOperation.chanWidth && chanWidthSupp) {
			pAddBssParams->ch_width =
				pBeaconStruct->VHTOperation.chanWidth + 1;
			pAddBssParams->ch_center_freq_seg0 =
				pBeaconStruct->VHTOperation.chanCenterFreqSeg1;
			pAddBssParams->ch_center_freq_seg1 =
				pBeaconStruct->VHTOperation.chanCenterFreqSeg2;
		}
		pAddBssParams->staContext.vht_caps =
			((pBeaconStruct->VHTCaps.maxMPDULen <<
			  SIR_MAC_VHT_CAP_MAX_MPDU_LEN) |
			 (pBeaconStruct->VHTCaps.supportedChannelWidthSet <<
			  SIR_MAC_VHT_CAP_SUPP_CH_WIDTH_SET) |
			 (pBeaconStruct->VHTCaps.ldpcCodingCap <<
			  SIR_MAC_VHT_CAP_LDPC_CODING_CAP) |
			 (pBeaconStruct->VHTCaps.shortGI80MHz <<
			  SIR_MAC_VHT_CAP_SHORTGI_80MHZ) |
			 (pBeaconStruct->VHTCaps.shortGI160and80plus80MHz <<
			  SIR_MAC_VHT_CAP_SHORTGI_160_80_80MHZ) |
			 (pBeaconStruct->VHTCaps.txSTBC <<
			  SIR_MAC_VHT_CAP_TXSTBC) |
			 (pBeaconStruct->VHTCaps.rxSTBC <<
			  SIR_MAC_VHT_CAP_RXSTBC) |
			 (pBeaconStruct->VHTCaps.suBeamFormerCap <<
			  SIR_MAC_VHT_CAP_SU_BEAMFORMER_CAP) |
			 (pBeaconStruct->VHTCaps.suBeamformeeCap <<
			  SIR_MAC_VHT_CAP_SU_BEAMFORMEE_CAP) |
			 (pBeaconStruct->VHTCaps.csnofBeamformerAntSup <<
			  SIR_MAC_VHT_CAP_CSN_BEAMORMER_ANT_SUP) |
			 (pBeaconStruct->VHTCaps.numSoundingDim <<
			  SIR_MAC_VHT_CAP_NUM_SOUNDING_DIM) |
			 (pBeaconStruct->VHTCaps.muBeamformerCap <<
			  SIR_MAC_VHT_CAP_NUM_BEAM_FORMER_CAP) |
			 (pBeaconStruct->VHTCaps.muBeamformeeCap <<
			  SIR_MAC_VHT_CAP_NUM_BEAM_FORMEE_CAP) |
			 (pBeaconStruct->VHTCaps.vhtTXOPPS <<
			  SIR_MAC_VHT_CAP_TXOPPS) |
			 (pBeaconStruct->VHTCaps.htcVHTCap <<
			  SIR_MAC_VHT_CAP_HTC_CAP) |
			 (pBeaconStruct->VHTCaps.maxAMPDULenExp <<
			  SIR_MAC_VHT_CAP_MAX_AMDU_LEN_EXPO) |
			 (pBeaconStruct->VHTCaps.vhtLinkAdaptCap <<
			  SIR_MAC_VHT_CAP_LINK_ADAPT_CAP) |
			 (pBeaconStruct->VHTCaps.rxAntPattern <<
			  SIR_MAC_VHT_CAP_RX_ANTENNA_PATTERN) |
			 (pBeaconStruct->VHTCaps.txAntPattern <<
			  SIR_MAC_VHT_CAP_TX_ANTENNA_PATTERN) |
			 (pBeaconStruct->VHTCaps.reserved1 <<
			  SIR_MAC_VHT_CAP_RESERVED2));
	} else {
		pAddBssParams->vhtCapable = 0;
	}
#endif

	lim_log(pMac, LOG1, FL("SIR_HAL_ADD_BSS_REQ with channel = %d..."),
		pAddBssParams->currentOperChannel);

	/* Populate the STA-related parameters here */
	/* Note that the STA here refers to the AP */
	{
		pAddBssParams->staContext.staType = STA_ENTRY_OTHER;

		cdf_mem_copy(pAddBssParams->staContext.bssId,
			     bssDescription->bssId, sizeof(tSirMacAddr));
		pAddBssParams->staContext.listenInterval =
			bssDescription->beaconInterval;

		pAddBssParams->staContext.assocId = 0;
		pAddBssParams->staContext.uAPSD = 0;
		pAddBssParams->staContext.maxSPLen = 0;
		pAddBssParams->staContext.shortPreambleSupported =
			(uint8_t) pBeaconStruct->capabilityInfo.shortPreamble;
		pAddBssParams->staContext.updateSta = updateEntry;
		pAddBssParams->staContext.encryptType =
			pftSessionEntry->encryptType;
#ifdef WLAN_FEATURE_11W
		pAddBssParams->staContext.rmfEnabled =
			pftSessionEntry->limRmfEnabled;
#endif

		if (IS_DOT11_MODE_HT(pftSessionEntry->dot11mode) &&
		    (pBeaconStruct->HTCaps.present)) {
			pAddBssParams->staContext.us32MaxAmpduDuration = 0;
			pAddBssParams->staContext.htCapable = 1;
			pAddBssParams->staContext.greenFieldCapable =
				(uint8_t) pBeaconStruct->HTCaps.greenField;
			pAddBssParams->staContext.lsigTxopProtection =
				(uint8_t) pBeaconStruct->HTCaps.lsigTXOPProtection;
			if ((pBeaconStruct->HTCaps.supportedChannelWidthSet) &&
			    (chanWidthSupp)) {
				pAddBssParams->staContext.ch_width = (uint8_t)
					pBeaconStruct->HTInfo.recommendedTxWidthSet;
			} else {
				pAddBssParams->staContext.ch_width =
					CH_WIDTH_20MHZ;
			}
			if (pftSessionEntry->vhtCapability &&
			    IS_BSS_VHT_CAPABLE(pBeaconStruct->VHTCaps)) {
				pAddBssParams->staContext.vhtCapable = 1;
				if ((pBeaconStruct->VHTCaps.suBeamFormerCap ||
				     pBeaconStruct->VHTCaps.muBeamformerCap) &&
				    pftSessionEntry->txBFIniFeatureEnabled)
					sta_ctx->vhtTxBFCapable
						= 1;
				if (pBeaconStruct->VHTCaps.suBeamformeeCap &&
				    pftSessionEntry->enable_su_tx_bformer)
					sta_ctx->enable_su_tx_bformer = 1;
			}
			if ((pBeaconStruct->HTCaps.supportedChannelWidthSet) &&
			    (chanWidthSupp)) {
				sta_ctx->ch_width = (uint8_t)
					pBeaconStruct->HTInfo.recommendedTxWidthSet;
				if (pAddBssParams->staContext.vhtCapable &&
					pBeaconStruct->VHTOperation.chanWidth)
					sta_ctx->ch_width =
					pBeaconStruct->VHTOperation.chanWidth
						+ 1;
			} else {
				pAddBssParams->staContext.ch_width =
					CH_WIDTH_20MHZ;
			}
			pAddBssParams->staContext.mimoPS =
				(tSirMacHTMIMOPowerSaveState) pBeaconStruct->HTCaps.
				mimoPowerSave;
			pAddBssParams->staContext.maxAmsduSize =
				(uint8_t) pBeaconStruct->HTCaps.maximalAMSDUsize;
			pAddBssParams->staContext.maxAmpduDensity =
				pBeaconStruct->HTCaps.mpduDensity;
			pAddBssParams->staContext.fDsssCckMode40Mhz =
				(uint8_t) pBeaconStruct->HTCaps.dsssCckMode40MHz;
			pAddBssParams->staContext.fShortGI20Mhz =
				(uint8_t) pBeaconStruct->HTCaps.shortGI20MHz;
			pAddBssParams->staContext.fShortGI40Mhz =
				(uint8_t) pBeaconStruct->HTCaps.shortGI40MHz;
			pAddBssParams->staContext.maxAmpduSize =
				pBeaconStruct->HTCaps.maxRxAMPDUFactor;

			if (pBeaconStruct->HTInfo.present)
				pAddBssParams->staContext.rifsMode =
					pBeaconStruct->HTInfo.rifsMode;
		}

		if ((pftSessionEntry->limWmeEnabled
		     && pBeaconStruct->wmeEdcaPresent)
		    || (pftSessionEntry->limQosEnabled
			&& pBeaconStruct->edcaPresent))
			pAddBssParams->staContext.wmmEnabled = 1;
		else
			pAddBssParams->staContext.wmmEnabled = 0;

		pAddBssParams->staContext.wpa_rsn = pBeaconStruct->rsnPresent;
		/* For OSEN Connection AP does not advertise RSN or WPA IE
		 * so from the IEs we get from supplicant we get this info
		 * so for FW to transmit EAPOL message 4 we shall set
		 * wpa_rsn
		 */
		pAddBssParams->staContext.wpa_rsn |=
			(pBeaconStruct->wpaPresent << 1);
		if ((!pAddBssParams->staContext.wpa_rsn)
		    && (pftSessionEntry->isOSENConnection))
			pAddBssParams->staContext.wpa_rsn = 1;
		/* Update the rates */
#ifdef WLAN_FEATURE_11AC
		lim_populate_peer_rate_set(pMac,
					   &pAddBssParams->staContext.
					   supportedRates,
					   pBeaconStruct->HTCaps.supportedMCSSet,
					   false, pftSessionEntry,
					   &pBeaconStruct->VHTCaps);
#else
		lim_populate_peer_rate_set(pMac,
					   &pAddBssParams->staContext.
					   supportedRates,
					   beaconStruct.HTCaps.supportedMCSSet,
					   false, pftSessionEntry);
#endif
		if (pftSessionEntry->htCapability) {
			pAddBssParams->staContext.supportedRates.opRateMode =
				eSTA_11n;
			if (pftSessionEntry->vhtCapability)
				pAddBssParams->staContext.supportedRates.
				opRateMode = eSTA_11ac;
		} else {
			if (pftSessionEntry->limRFBand == SIR_BAND_5_GHZ) {
				pAddBssParams->staContext.supportedRates.
				opRateMode = eSTA_11a;
			} else {
				pAddBssParams->staContext.supportedRates.
				opRateMode = eSTA_11bg;
			}
		}
	}

#if defined WLAN_FEATURE_VOWIFI
	pAddBssParams->maxTxPower = pftSessionEntry->maxTxPower;
#endif

#ifdef WLAN_FEATURE_11W
	if (pftSessionEntry->limRmfEnabled) {
		pAddBssParams->rmfEnabled = 1;
		pAddBssParams->staContext.rmfEnabled = 1;
	}
#endif

	pAddBssParams->status = QDF_STATUS_SUCCESS;
	pAddBssParams->respReqd = true;

	pAddBssParams->staContext.sessionId = pftSessionEntry->peSessionId;
	pAddBssParams->staContext.smesessionId = pftSessionEntry->smeSessionId;
	pAddBssParams->sessionId = pftSessionEntry->peSessionId;

	/* Set a new state for MLME */
	if (!pftSessionEntry->bRoamSynchInProgress) {
		pftSessionEntry->limMlmState =
			eLIM_MLM_WT_ADD_BSS_RSP_FT_REASSOC_STATE;
		MTRACE(mac_trace
			(pMac, TRACE_CODE_MLM_STATE,
			pftSessionEntry->peSessionId,
			eLIM_MLM_WT_ADD_BSS_RSP_FT_REASSOC_STATE));
	}
	pAddBssParams->halPersona = (uint8_t) pftSessionEntry->pePersona;

	pftSessionEntry->ftPEContext.pAddBssReq = pAddBssParams;

	lim_log(pMac, LOG1, FL("Saving SIR_HAL_ADD_BSS_REQ for pre-auth ap..."));

	cdf_mem_free(pBeaconStruct);
	return 0;
}

/*------------------------------------------------------------------
 *
 * Setup the new session for the pre-auth AP.
 * Return the newly created session entry.
 *
 *------------------------------------------------------------------*/
void lim_fill_ft_session(tpAniSirGlobal pMac,
			 tpSirBssDescription pbssDescription,
			 tpPESession pftSessionEntry, tpPESession psessionEntry)
{
	uint8_t currentBssUapsd;
	int8_t localPowerConstraint;
	int8_t regMax;
	tSchBeaconStruct *pBeaconStruct;
	uint32_t selfDot11Mode;
	ePhyChanBondState cbEnabledMode;

	pBeaconStruct = cdf_mem_malloc(sizeof(tSchBeaconStruct));
	if (NULL == pBeaconStruct) {
		lim_log(pMac, LOGE,
			FL("No memory for creating lim_fill_ft_session"));
		return;
	}

	/* Retrieve the session that was already created and update the entry */
	lim_print_mac_addr(pMac, pbssDescription->bssId, LOG1);
	pftSessionEntry->limWmeEnabled = psessionEntry->limWmeEnabled;
	pftSessionEntry->limQosEnabled = psessionEntry->limQosEnabled;
	pftSessionEntry->limWsmEnabled = psessionEntry->limWsmEnabled;
	pftSessionEntry->lim11hEnable = psessionEntry->lim11hEnable;
	pftSessionEntry->isOSENConnection = psessionEntry->isOSENConnection;

	/* Fields to be filled later */
	pftSessionEntry->pLimJoinReq = NULL;
	pftSessionEntry->smeSessionId = psessionEntry->smeSessionId;
	pftSessionEntry->transactionId = 0;

	lim_extract_ap_capabilities(pMac, (uint8_t *) pbssDescription->ieFields,
			lim_get_ielen_from_bss_description(pbssDescription),
			pBeaconStruct);

	pftSessionEntry->rateSet.numRates =
		pBeaconStruct->supportedRates.numRates;
	cdf_mem_copy(pftSessionEntry->rateSet.rate,
		     pBeaconStruct->supportedRates.rate,
		     pBeaconStruct->supportedRates.numRates);

	pftSessionEntry->extRateSet.numRates =
		pBeaconStruct->extendedRates.numRates;
	cdf_mem_copy(pftSessionEntry->extRateSet.rate,
		     pBeaconStruct->extendedRates.rate,
		     pftSessionEntry->extRateSet.numRates);

	pftSessionEntry->ssId.length = pBeaconStruct->ssId.length;
	cdf_mem_copy(pftSessionEntry->ssId.ssId, pBeaconStruct->ssId.ssId,
		     pftSessionEntry->ssId.length);

	wlan_cfg_get_int(pMac, WNI_CFG_DOT11_MODE, &selfDot11Mode);
	lim_log(pMac, LOG1, FL("selfDot11Mode %d"), selfDot11Mode);
	pftSessionEntry->dot11mode = selfDot11Mode;
	pftSessionEntry->vhtCapability =
		(IS_DOT11_MODE_VHT(pftSessionEntry->dot11mode)
		 && IS_BSS_VHT_CAPABLE(pBeaconStruct->VHTCaps));
	pftSessionEntry->htCapability =
		(IS_DOT11_MODE_HT(pftSessionEntry->dot11mode)
		 && pBeaconStruct->HTCaps.present);

	/* Copy The channel Id to the session Table */
	pftSessionEntry->limReassocChannelId = pbssDescription->channelId;
	pftSessionEntry->currentOperChannel = pbssDescription->channelId;

	pftSessionEntry->limRFBand = lim_get_rf_band(
				pftSessionEntry->currentOperChannel);

	if (pftSessionEntry->limRFBand == SIR_BAND_2_4_GHZ) {
		cbEnabledMode = pMac->roam.configParam.channelBondingMode24GHz;
	} else {
		cbEnabledMode = pMac->roam.configParam.channelBondingMode5GHz;
	}
	pftSessionEntry->htSupportedChannelWidthSet =
	    (pBeaconStruct->HTInfo.present) ?
	    (cbEnabledMode && pBeaconStruct->HTInfo.recommendedTxWidthSet) : 0;
	pftSessionEntry->htRecommendedTxWidthSet =
		pftSessionEntry->htSupportedChannelWidthSet;


#ifdef WLAN_FEATURE_11AC
	if (IS_BSS_VHT_CAPABLE(pBeaconStruct->VHTCaps) &&
		pBeaconStruct->VHTOperation.present &&
		pftSessionEntry->vhtCapability) {
		pftSessionEntry->vhtCapabilityPresentInBeacon = 1;
	} else {
		pftSessionEntry->vhtCapabilityPresentInBeacon = 0;
	}
#endif
	if (pftSessionEntry->htRecommendedTxWidthSet) {
		pftSessionEntry->ch_width = CH_WIDTH_40MHZ;
		if (pftSessionEntry->vhtCapabilityPresentInBeacon &&
				pBeaconStruct->VHTOperation.chanWidth) {
			pftSessionEntry->ch_width =
				pBeaconStruct->VHTOperation.chanWidth + 1;
			pftSessionEntry->ch_center_freq_seg0 =
				pBeaconStruct->VHTOperation.chanCenterFreqSeg1;
			pftSessionEntry->ch_center_freq_seg1 =
				pBeaconStruct->VHTOperation.chanCenterFreqSeg2;
		} else {
			if (pBeaconStruct->HTInfo.secondaryChannelOffset ==
					PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
				pftSessionEntry->ch_center_freq_seg0 =
					pbssDescription->channelId + 2;
			else if (pBeaconStruct->HTInfo.secondaryChannelOffset ==
					PHY_DOUBLE_CHANNEL_HIGH_PRIMARY)
				pftSessionEntry->ch_center_freq_seg0 =
					pbssDescription->channelId - 2;
			else
				lim_log(pMac, LOGE, FL("Invalid sec ch offset"));
		}
	} else {
		pftSessionEntry->ch_width = CH_WIDTH_20MHZ;
		pftSessionEntry->ch_center_freq_seg0 = 0;
		pftSessionEntry->ch_center_freq_seg1 = 0;
	}

	sir_copy_mac_addr(pftSessionEntry->selfMacAddr,
			  psessionEntry->selfMacAddr);
	sir_copy_mac_addr(pftSessionEntry->limReAssocbssId,
			  pbssDescription->bssId);
	sir_copy_mac_addr(pftSessionEntry->prev_ap_bssid, psessionEntry->bssId);
	lim_print_mac_addr(pMac, pftSessionEntry->limReAssocbssId, LOG1);

	/* Store beaconInterval */
	pftSessionEntry->beaconParams.beaconInterval =
		pbssDescription->beaconInterval;
	pftSessionEntry->bssType = psessionEntry->bssType;

	pftSessionEntry->statypeForBss = STA_ENTRY_PEER;
	pftSessionEntry->nwType = pbssDescription->nwType;


	if (pftSessionEntry->bssType == eSIR_INFRASTRUCTURE_MODE) {
		pftSessionEntry->limSystemRole = eLIM_STA_ROLE;
	} else if (pftSessionEntry->bssType == eSIR_BTAMP_AP_MODE) {
		pftSessionEntry->limSystemRole = eLIM_BT_AMP_STA_ROLE;
	} else {
		/* Throw an error and return and make sure to delete the session. */
		lim_log(pMac, LOGE, FL("Invalid bss type"));
	}

	pftSessionEntry->limCurrentBssCaps = pbssDescription->capabilityInfo;
	pftSessionEntry->limReassocBssCaps = pbssDescription->capabilityInfo;
	if (pMac->roam.configParam.shortSlotTime &&
	    SIR_MAC_GET_SHORT_SLOT_TIME(pftSessionEntry->limReassocBssCaps)) {
		pftSessionEntry->shortSlotTimeSupported = true;
	}

	regMax = cfg_get_regulatory_max_transmit_power(pMac,
						       pftSessionEntry->
						       currentOperChannel);
	localPowerConstraint = regMax;
	lim_extract_ap_capability(pMac, (uint8_t *) pbssDescription->ieFields,
		lim_get_ielen_from_bss_description(pbssDescription),
		&pftSessionEntry->limCurrentBssQosCaps,
		&pftSessionEntry->limCurrentBssPropCap, &currentBssUapsd,
		&localPowerConstraint, pftSessionEntry);

	pftSessionEntry->limReassocBssQosCaps =
		pftSessionEntry->limCurrentBssQosCaps;
	pftSessionEntry->limReassocBssPropCap =
		pftSessionEntry->limCurrentBssPropCap;

#ifdef WLAN_FEATURE_VOWIFI_11R
	pftSessionEntry->is11Rconnection = psessionEntry->is11Rconnection;
#endif
#ifdef FEATURE_WLAN_ESE
	pftSessionEntry->isESEconnection = psessionEntry->isESEconnection;
	pftSessionEntry->is_ese_version_ie_present =
		pBeaconStruct->is_ese_ver_ie_present;
#endif
	pftSessionEntry->isFastTransitionEnabled =
		psessionEntry->isFastTransitionEnabled;

	pftSessionEntry->isFastRoamIniFeatureEnabled =
		psessionEntry->isFastRoamIniFeatureEnabled;

#ifdef FEATURE_WLAN_ESE
	pftSessionEntry->maxTxPower =
		lim_get_max_tx_power(regMax, localPowerConstraint,
				     pMac->roam.configParam.nTxPowerCap);
#else
	pftSessionEntry->maxTxPower = QDF_MIN(regMax, (localPowerConstraint));
#endif

	lim_log(pMac, LOG1,
		FL("Reg max=%d, local pwr=%d, ini tx pwr=%d, max tx pwr = %d"),
		regMax, localPowerConstraint,
		pMac->roam.configParam.nTxPowerCap,
		pftSessionEntry->maxTxPower);
	if (!psessionEntry->bRoamSynchInProgress) {
		pftSessionEntry->limPrevSmeState = pftSessionEntry->limSmeState;
		pftSessionEntry->limSmeState = eLIM_SME_WT_REASSOC_STATE;
		MTRACE(mac_trace(pMac,
				TRACE_CODE_SME_STATE,
				pftSessionEntry->peSessionId,
				pftSessionEntry->limSmeState));
	}
	pftSessionEntry->encryptType = psessionEntry->encryptType;
#ifdef WLAN_FEATURE_11W
	pftSessionEntry->limRmfEnabled = psessionEntry->limRmfEnabled;
#endif

	cdf_mem_free(pBeaconStruct);
}

/*------------------------------------------------------------------
 *
 * Setup the session and the add bss req for the pre-auth AP.
 *
 *------------------------------------------------------------------*/
tSirRetStatus lim_ft_setup_auth_session(tpAniSirGlobal pMac,
					tpPESession psessionEntry)
{
	tpPESession pftSessionEntry = NULL;
	uint8_t sessionId = 0;

	pftSessionEntry =
		pe_find_session_by_bssid(pMac, psessionEntry->limReAssocbssId,
					 &sessionId);
	if (pftSessionEntry == NULL) {
		PELOGE(lim_log(pMac, LOGE,
			       FL
				       ("Unable to find session for the following bssid"));
		       )
		lim_print_mac_addr(pMac, psessionEntry->limReAssocbssId, LOGE);
		return eSIR_FAILURE;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return eSIR_FAILURE;
	}

	if (psessionEntry->ftPEContext.pFTPreAuthReq &&
	    psessionEntry->ftPEContext.pFTPreAuthReq->pbssDescription) {
		lim_fill_ft_session(pMac,
				    psessionEntry->ftPEContext.pFTPreAuthReq->
				    pbssDescription, pftSessionEntry,
				    psessionEntry);

		lim_ft_prepare_add_bss_req(pMac, false, pftSessionEntry,
					   psessionEntry->ftPEContext.pFTPreAuthReq->
					   pbssDescription);
	}

	return eSIR_SUCCESS;
}

/*------------------------------------------------------------------
 * Resume Link Call Back
 *------------------------------------------------------------------*/
void lim_ft_process_pre_auth_result(tpAniSirGlobal pMac, QDF_STATUS status,
				    tpPESession psessionEntry)
{
	if (NULL == psessionEntry ||
	    NULL == psessionEntry->ftPEContext.pFTPreAuthReq)
		return;

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return;
	}

	if (psessionEntry->ftPEContext.ftPreAuthStatus == eSIR_SUCCESS) {
		psessionEntry->ftPEContext.ftPreAuthStatus =
			lim_ft_setup_auth_session(pMac, psessionEntry);
	}
	/* Post the FT Pre Auth Response to SME */
	lim_post_ft_pre_auth_rsp(pMac, psessionEntry->ftPEContext.ftPreAuthStatus,
				 psessionEntry->ftPEContext.saved_auth_rsp,
				 psessionEntry->ftPEContext.saved_auth_rsp_length,
				 psessionEntry);
}

/*
 * lim_post_ft_pre_auth_rsp() - post ft pre auth response to SME.
 *
 * @mac_ctx:		global mac ctx
 * @status:		status code to post in auth rsp
 * @auth_rsp:		pointer to auth rsp FT ie
 * @auth_rsp_length:	len of the IE field
 * @session:	        pe session
 *
 * post pre auth response to SME.
 *
 * Return: void
 */
void lim_post_ft_pre_auth_rsp(tpAniSirGlobal mac_ctx,
			      tSirRetStatus status,
			      uint8_t *auth_rsp,
			      uint16_t auth_rsp_length,
			      tpPESession session)
{
	tpSirFTPreAuthRsp ft_pre_auth_rsp;
	tSirMsgQ mmh_msg;
	uint16_t rsp_len = sizeof(tSirFTPreAuthRsp);

	ft_pre_auth_rsp = (tpSirFTPreAuthRsp) cdf_mem_malloc(rsp_len);
	if (NULL == ft_pre_auth_rsp) {
		lim_log(mac_ctx, LOGE, "Failed to allocate memory");
		CDF_ASSERT(ft_pre_auth_rsp != NULL);
		return;
	}
	cdf_mem_zero(ft_pre_auth_rsp, rsp_len);

	lim_log(mac_ctx, LOG1, FL("Auth Rsp = %p"), ft_pre_auth_rsp);
	if (session) {
		/* Nothing to be done if the session is not in STA mode */
		if (!LIM_IS_STA_ROLE(session)) {
			lim_log(mac_ctx, LOGE,
				FL("session is not in STA mode"));
			cdf_mem_free(ft_pre_auth_rsp);
			return;
		}
		ft_pre_auth_rsp->smeSessionId = session->smeSessionId;
		/* The bssid of the AP we are sending Auth1 to. */
		if (session->ftPEContext.pFTPreAuthReq)
			sir_copy_mac_addr(ft_pre_auth_rsp->preAuthbssId,
			    session->ftPEContext.pFTPreAuthReq->preAuthbssId);
	}

	ft_pre_auth_rsp->messageType = eWNI_SME_FT_PRE_AUTH_RSP;
	ft_pre_auth_rsp->length = (uint16_t) rsp_len;
	ft_pre_auth_rsp->status = status;

	/* Attach the auth response now back to SME */
	ft_pre_auth_rsp->ft_ies_length = 0;
	if ((auth_rsp != NULL) && (auth_rsp_length < MAX_FTIE_SIZE)) {
		/* Only 11r assoc has FT IEs */
		cdf_mem_copy(ft_pre_auth_rsp->ft_ies,
			     auth_rsp, auth_rsp_length);
		ft_pre_auth_rsp->ft_ies_length = auth_rsp_length;
	}

	if (status != eSIR_SUCCESS) {
		/*
		 * Ensure that on Pre-Auth failure the cached Pre-Auth Req and
		 * other allocated memory is freed up before returning.
		 */
		lim_log(mac_ctx, LOG1, "Pre-Auth Failed, Cleanup!");
		lim_ft_cleanup(mac_ctx, session);
	}

	mmh_msg.type = ft_pre_auth_rsp->messageType;
	mmh_msg.bodyptr = ft_pre_auth_rsp;
	mmh_msg.bodyval = 0;

	lim_log(mac_ctx, LOG1, FL("Posted Auth Rsp to SME with status of 0x%x"),
		status);
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	if (status == eSIR_SUCCESS)
		lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_PREAUTH_DONE,
				      session, status, 0);
#endif
	lim_sys_process_mmh_msg_api(mac_ctx, &mmh_msg, ePROT);
}

/*------------------------------------------------------------------
 *
 * Send the FT Pre Auth Response to SME whenever we have a status
 * ready to be sent to SME
 *
 * SME will be the one to send it up to the supplicant to receive
 * FTIEs which will be required for Reassoc Req.
 *
 *------------------------------------------------------------------*/
void lim_handle_ft_pre_auth_rsp(tpAniSirGlobal pMac, tSirRetStatus status,
				uint8_t *auth_rsp, uint16_t auth_rsp_length,
				tpPESession psessionEntry)
{
	tpPESession pftSessionEntry = NULL;
	uint8_t sessionId = 0;
	tpSirBssDescription pbssDescription = NULL;
#ifdef FEATURE_WLAN_DIAG_SUPPORT
	lim_diag_event_report(pMac, WLAN_PE_DIAG_PRE_AUTH_RSP_EVENT,
			      psessionEntry, (uint16_t) status, 0);
#endif

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return;
	}

	/* Save the status of pre-auth */
	psessionEntry->ftPEContext.ftPreAuthStatus = status;

	/* Save the auth rsp, so we can send it to
	 * SME once we resume link
	 */
	psessionEntry->ftPEContext.saved_auth_rsp_length = 0;
	if ((auth_rsp != NULL) && (auth_rsp_length < MAX_FTIE_SIZE)) {
		cdf_mem_copy(psessionEntry->ftPEContext.saved_auth_rsp,
			     auth_rsp, auth_rsp_length);
		psessionEntry->ftPEContext.saved_auth_rsp_length =
			auth_rsp_length;
	}

	if (!psessionEntry->ftPEContext.pFTPreAuthReq ||
	    !psessionEntry->ftPEContext.pFTPreAuthReq->pbssDescription) {
		lim_log(pMac, LOGE,
			FL("pFTPreAuthReq or pbssDescription is NULL"));
		return;
	}

	/* Create FT session for the re-association at this point */
	if (psessionEntry->ftPEContext.ftPreAuthStatus == eSIR_SUCCESS) {
		pbssDescription =
			psessionEntry->ftPEContext.pFTPreAuthReq->pbssDescription;
		lim_print_mac_addr(pMac, pbssDescription->bssId, LOG1);
		pftSessionEntry =
			     pe_create_session(pMac, pbssDescription->bssId,
					       &sessionId, pMac->lim.maxStation,
					       psessionEntry->bssType);
		if (pftSessionEntry == NULL) {
			lim_log(pMac, LOGE, FL(
				"Session not created for pre-auth 11R AP"));
			status = eSIR_FAILURE;
			psessionEntry->ftPEContext.ftPreAuthStatus = status;
			goto send_rsp;
		}
		pftSessionEntry->peSessionId = sessionId;
		pftSessionEntry->smeSessionId = psessionEntry->smeSessionId;
		sir_copy_mac_addr(pftSessionEntry->selfMacAddr,
				  psessionEntry->selfMacAddr);
		sir_copy_mac_addr(pftSessionEntry->limReAssocbssId,
				  pbssDescription->bssId);
		pftSessionEntry->bssType = psessionEntry->bssType;

		if (pftSessionEntry->bssType == eSIR_INFRASTRUCTURE_MODE) {
			pftSessionEntry->limSystemRole = eLIM_STA_ROLE;
		} else if (pftSessionEntry->bssType == eSIR_BTAMP_AP_MODE) {
			pftSessionEntry->limSystemRole = eLIM_BT_AMP_STA_ROLE;
		} else {
			lim_log(pMac, LOGE, FL("Invalid bss type"));
		}
		pftSessionEntry->limPrevSmeState = pftSessionEntry->limSmeState;
		cdf_mem_copy(&(pftSessionEntry->htConfig),
			     &(psessionEntry->htConfig),
			     sizeof(psessionEntry->htConfig));
		pftSessionEntry->limSmeState = eLIM_SME_WT_REASSOC_STATE;

		PELOGE(lim_log
			       (pMac, LOG1, "%s:created session (%p) with id = %d",
			       __func__, pftSessionEntry,
			       pftSessionEntry->peSessionId);
		       )

		/* Update the ReAssoc BSSID of the current session */
		sir_copy_mac_addr(psessionEntry->limReAssocbssId,
				  pbssDescription->bssId);
		lim_print_mac_addr(pMac, psessionEntry->limReAssocbssId, LOG1);
	}
send_rsp:
	if (psessionEntry->currentOperChannel !=
	    psessionEntry->ftPEContext.pFTPreAuthReq->preAuthchannelNum) {
		/* Need to move to the original AP channel */
		lim_process_abort_scan_ind(pMac, psessionEntry->peSessionId,
			psessionEntry->ftPEContext.pFTPreAuthReq->scan_id,
			PREAUTH_REQUESTOR_ID);
	} else {
		lim_log(pMac, LOG1,
			"Pre auth on same channel as connected AP channel %d",
			psessionEntry->ftPEContext.pFTPreAuthReq->
			preAuthchannelNum);
		lim_ft_process_pre_auth_result(pMac, status, psessionEntry);
	}
}

/*------------------------------------------------------------------
 *
 *  This function handles the 11R Reassoc Req from SME
 *
 *------------------------------------------------------------------*/
void lim_process_mlm_ft_reassoc_req(tpAniSirGlobal pMac, uint32_t *pMsgBuf,
				    tpPESession psessionEntry)
{
	uint8_t smeSessionId = 0;
	uint16_t transactionId = 0;
	uint8_t chanNum = 0;
	tLimMlmReassocReq *pMlmReassocReq;
	uint16_t caps;
	uint32_t val;
	tSirMsgQ msgQ;
	tSirRetStatus retCode;
	uint32_t teleBcnEn = 0;

	chanNum = psessionEntry->currentOperChannel;
	lim_get_session_info(pMac, (uint8_t *) pMsgBuf, &smeSessionId,
			     &transactionId);
	psessionEntry->smeSessionId = smeSessionId;
	psessionEntry->transactionId = transactionId;

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_REASSOCIATING, psessionEntry, 0,
			      0);
#endif

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return;
	}

	if (NULL == psessionEntry->ftPEContext.pAddBssReq) {
		lim_log(pMac, LOGE, FL("pAddBssReq is NULL"));
		return;
	}
	pMlmReassocReq = cdf_mem_malloc(sizeof(tLimMlmReassocReq));
	if (NULL == pMlmReassocReq) {
		lim_log(pMac, LOGE,
			FL("call to AllocateMemory failed for mlmReassocReq"));
		return;
	}

	cdf_mem_copy(pMlmReassocReq->peerMacAddr,
		     psessionEntry->bssId, sizeof(tSirMacAddr));

	if (wlan_cfg_get_int(pMac, WNI_CFG_REASSOCIATION_FAILURE_TIMEOUT,
			     (uint32_t *) &pMlmReassocReq->reassocFailureTimeout)
	    != eSIR_SUCCESS) {
		/**
		 * Could not get ReassocFailureTimeout value
		 * from CFG. Log error.
		 */
		lim_log(pMac, LOGE,
			FL("could not retrieve ReassocFailureTimeout value"));
		cdf_mem_free(pMlmReassocReq);
		return;
	}

	if (cfg_get_capability_info(pMac, &caps, psessionEntry) != eSIR_SUCCESS) {
		/**
		 * Could not get Capabilities value
		 * from CFG. Log error.
		 */
		lim_log(pMac, LOGE, FL("could not retrieve Capabilities value"));
		cdf_mem_free(pMlmReassocReq);
		return;
	}
	pMlmReassocReq->capabilityInfo = caps;

	/* Update PE sessionId */
	pMlmReassocReq->sessionId = psessionEntry->peSessionId;

	/* If telescopic beaconing is enabled, set listen interval
	   to WNI_CFG_TELE_BCN_MAX_LI
	 */
	if (wlan_cfg_get_int(pMac, WNI_CFG_TELE_BCN_WAKEUP_EN, &teleBcnEn) !=
	    eSIR_SUCCESS) {
		lim_log(pMac, LOGP,
			FL("Couldn't get WNI_CFG_TELE_BCN_WAKEUP_EN"));
		cdf_mem_free(pMlmReassocReq);
		return;
	}

	if (teleBcnEn) {
		if (wlan_cfg_get_int(pMac, WNI_CFG_TELE_BCN_MAX_LI, &val) !=
		    eSIR_SUCCESS) {
			/**
			 * Could not get ListenInterval value
			 * from CFG. Log error.
			 */
			lim_log(pMac, LOGE,
				FL("could not retrieve ListenInterval"));
			cdf_mem_free(pMlmReassocReq);
			return;
		}
	} else {
		if (wlan_cfg_get_int(pMac, WNI_CFG_LISTEN_INTERVAL, &val) !=
		    eSIR_SUCCESS) {
			/**
			 * Could not get ListenInterval value
			 * from CFG. Log error.
			 */
			lim_log(pMac, LOGE,
				FL("could not retrieve ListenInterval"));
			cdf_mem_free(pMlmReassocReq);
			return;
		}
	}
	if (lim_set_link_state
		    (pMac, eSIR_LINK_PREASSOC_STATE, psessionEntry->bssId,
		    psessionEntry->selfMacAddr, NULL, NULL) != eSIR_SUCCESS) {
		cdf_mem_free(pMlmReassocReq);
		return;
	}

	pMlmReassocReq->listenInterval = (uint16_t) val;
	psessionEntry->pLimMlmReassocReq = pMlmReassocReq;

	/* we need to defer the message until we get the response back from HAL */
	SET_LIM_PROCESS_DEFD_MESGS(pMac, false);

	msgQ.type = SIR_HAL_ADD_BSS_REQ;
	msgQ.reserved = 0;
	msgQ.bodyptr = psessionEntry->ftPEContext.pAddBssReq;
	msgQ.bodyval = 0;

	lim_log(pMac, LOG1, FL("Sending SIR_HAL_ADD_BSS_REQ..."));
	MTRACE(mac_trace_msg_tx(pMac, psessionEntry->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		cdf_mem_free(psessionEntry->ftPEContext.pAddBssReq);
		lim_log(pMac, LOGE,
			FL("Posting ADD_BSS_REQ to HAL failed, reason=%X"),
			retCode);
	}

	psessionEntry->ftPEContext.pAddBssReq = NULL;
	return;
}

/*
 * lim_process_ft_preauth_rsp_timeout() - process ft preauth rsp timeout
 *
 * @mac_ctx:		global mac ctx
 *
 * This function is called if preauth response is not received from the AP
 * within this timeout while FT in progress
 *
 * Return: void
 */
void lim_process_ft_preauth_rsp_timeout(tpAniSirGlobal mac_ctx)
{
	tpPESession session;

	/*
	 * We have failed pre auth. We need to resume link and get back on
	 * home channel
	 */
	lim_log(mac_ctx, LOGE, FL("FT Pre-Auth Time Out!!!!"));
	session = pe_find_session_by_session_id(mac_ctx,
			mac_ctx->lim.limTimers.gLimFTPreAuthRspTimer.sessionId);
	if (NULL == session) {
		lim_log(mac_ctx, LOGE,
			FL("Session Does not exist for given sessionID"));
		return;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(session)) {
		lim_log(mac_ctx, LOGE, FL("session is not in STA mode"));
		return;
	}

	/* Reset the flag to indicate preauth request session */
	session->ftPEContext.ftPreAuthSession = false;

	if (NULL == session->ftPEContext.pFTPreAuthReq) {
		lim_log(mac_ctx, LOGE,
			FL("pFTPreAuthReq is NULL. Auth Rsp might already be posted to SME and ftcleanup done! sessionId:%d"),
			mac_ctx->lim.limTimers.gLimFTPreAuthRspTimer.sessionId);
		return;
	}

	/*
	 * To handle the race condition where we recieve preauth rsp after
	 * timer has expired.
	 */
	if (true ==
	    session->ftPEContext.pFTPreAuthReq->bPreAuthRspProcessed) {
		lim_log(mac_ctx, LOGE,
			FL("Auth rsp already posted to SME (session %p)"),
			session);
		return;
	} else {
		/*
		 * Here we are sending preauth rsp with failure state
		 * and which is forwarded to SME. Now, if we receive an preauth
		 * resp from AP with success it would create a FT pesession, but
		 * will be dropped in SME leaving behind the pesession. Mark
		 * Preauth rsp processed so that any rsp from AP is dropped in
		 * lim_process_auth_frame_no_session.
		 */
		lim_log(mac_ctx, LOG1,
			FL("Auth rsp not yet posted to SME (session %p)"),
			session);
		session->ftPEContext.pFTPreAuthReq->bPreAuthRspProcessed = true;
	}

	/*
	 * Attempted at Pre-Auth and failed. If we are off channel. We need
	 * to get back to home channel
	 */
	lim_handle_ft_pre_auth_rsp(mac_ctx, eSIR_FAILURE, NULL, 0, session);
}

/*------------------------------------------------------------------
 *
 * This function is called to process the update key request from SME
 *
 *------------------------------------------------------------------*/
bool lim_process_ft_update_key(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tAddBssParams *pAddBssParams;
	tSirFTUpdateKeyInfo *pKeyInfo;
	uint32_t val = 0;
	tpPESession psessionEntry;
	uint8_t sessionId;

	/* Sanity Check */
	if (pMac == NULL || pMsgBuf == NULL) {
		return false;
	}

	pKeyInfo = (tSirFTUpdateKeyInfo *) pMsgBuf;

	psessionEntry = pe_find_session_by_bssid(pMac, pKeyInfo->bssid.bytes,
						 &sessionId);
	if (NULL == psessionEntry) {
		PELOGE(lim_log(pMac, LOGE,
			       "%s: Unable to find session for the following bssid",
			       __func__);
		       )
		lim_print_mac_addr(pMac, pKeyInfo->bssid.bytes, LOGE);
		return false;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return false;
	}

	if (NULL == psessionEntry->ftPEContext.pAddBssReq) {
		/* AddBss Req is NULL, save the keys to configure them later. */
		tpLimMlmSetKeysReq pMlmSetKeysReq =
			&psessionEntry->ftPEContext.PreAuthKeyInfo.
			extSetStaKeyParam;

		cdf_mem_zero(pMlmSetKeysReq, sizeof(tLimMlmSetKeysReq));
		qdf_copy_macaddr(&pMlmSetKeysReq->peer_macaddr,
				 &pKeyInfo->bssid);
		pMlmSetKeysReq->sessionId = psessionEntry->peSessionId;
		pMlmSetKeysReq->smesessionId = psessionEntry->smeSessionId;
		pMlmSetKeysReq->edType = pKeyInfo->keyMaterial.edType;
		pMlmSetKeysReq->numKeys = pKeyInfo->keyMaterial.numKeys;
		cdf_mem_copy((uint8_t *) &pMlmSetKeysReq->key,
			     (uint8_t *) &pKeyInfo->keyMaterial.key,
			     sizeof(tSirKeys));

		psessionEntry->ftPEContext.PreAuthKeyInfo.
		extSetStaKeyParamValid = true;

		lim_log(pMac, LOGE, FL("pAddBssReq is NULL"));

		if (psessionEntry->ftPEContext.pAddStaReq == NULL) {
			lim_log(pMac, LOGE, FL("pAddStaReq is NULL"));
			lim_send_set_sta_key_req(pMac, pMlmSetKeysReq, 0, 0,
						 psessionEntry, false);
			psessionEntry->ftPEContext.PreAuthKeyInfo.
			extSetStaKeyParamValid = false;
		}
	} else {
		pAddBssParams = psessionEntry->ftPEContext.pAddBssReq;

		/* Store the key information in the ADD BSS parameters */
		pAddBssParams->extSetStaKeyParamValid = 1;
		pAddBssParams->extSetStaKeyParam.encType =
			pKeyInfo->keyMaterial.edType;
		cdf_mem_copy((uint8_t *) &pAddBssParams->extSetStaKeyParam.key,
			     (uint8_t *) &pKeyInfo->keyMaterial.key,
			     sizeof(tSirKeys));
		if (eSIR_SUCCESS !=
		    wlan_cfg_get_int(pMac, WNI_CFG_SINGLE_TID_RC, &val)) {
			lim_log(pMac, LOGP,
				FL("Unable to read WNI_CFG_SINGLE_TID_RC"));
		}

		pAddBssParams->extSetStaKeyParam.singleTidRc = val;
		PELOG1(lim_log(pMac, LOG1, FL("Key valid %d"),
			       pAddBssParams->extSetStaKeyParamValid,
			       pAddBssParams->extSetStaKeyParam.key[0].
			       keyLength);
		       )

		pAddBssParams->extSetStaKeyParam.staIdx = 0;

		PELOG1(lim_log(pMac, LOG1,
			       FL("BSSID = " MAC_ADDRESS_STR),
			       MAC_ADDR_ARRAY(pKeyInfo->bssid.bytes));)

		qdf_copy_macaddr(&pAddBssParams->extSetStaKeyParam.peer_macaddr,
				 &pKeyInfo->bssid);

		pAddBssParams->extSetStaKeyParam.sendRsp = false;

		if (pAddBssParams->extSetStaKeyParam.key[0].keyLength == 16) {
			PELOG1(lim_log(pMac, LOG1,
				       FL
					       ("BSS key = %02X-%02X-%02X-%02X-%02X-%02X-%02X- "
					       "%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X"),
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[0],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[1],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[2],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[3],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[4],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[5],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[6],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[7],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[8],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[9],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[10],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[11],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[12],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[13],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[14],
				       pAddBssParams->extSetStaKeyParam.key[0].
				       key[15]);
			       )
		}
	}
	return true;
}

void
lim_ft_send_aggr_qos_rsp(tpAniSirGlobal pMac, uint8_t rspReqd,
			 tpAggrAddTsParams aggrQosRsp, uint8_t smesessionId)
{
	tpSirAggrQosRsp rsp;
	int i = 0;
	if (!rspReqd) {
		return;
	}
	rsp = cdf_mem_malloc(sizeof(tSirAggrQosRsp));
	if (NULL == rsp) {
		lim_log(pMac, LOGP,
			FL("AllocateMemory failed for tSirAggrQosRsp"));
		return;
	}
	cdf_mem_set((uint8_t *) rsp, sizeof(*rsp), 0);
	rsp->messageType = eWNI_SME_FT_AGGR_QOS_RSP;
	rsp->sessionId = smesessionId;
	rsp->length = sizeof(*rsp);
	rsp->aggrInfo.tspecIdx = aggrQosRsp->tspecIdx;
	for (i = 0; i < SIR_QOS_NUM_AC_MAX; i++) {
		if ((1 << i) & aggrQosRsp->tspecIdx) {
			rsp->aggrInfo.aggrRsp[i].status = aggrQosRsp->status[i];
			rsp->aggrInfo.aggrRsp[i].tspec = aggrQosRsp->tspec[i];
		}
	}
	lim_send_sme_aggr_qos_rsp(pMac, rsp, smesessionId);
	return;
}
void lim_process_ft_aggr_qo_s_rsp(tpAniSirGlobal pMac, tpSirMsgQ limMsg)
{
	tpAggrAddTsParams pAggrQosRspMsg = NULL;
	tAddTsParams addTsParam = { 0 };
	tpDphHashNode pSta = NULL;
	uint16_t assocId = 0;
	tSirMacAddr peerMacAddr;
	uint8_t rspReqd = 1;
	tpPESession psessionEntry = NULL;
	int i = 0;
	PELOG1(lim_log(pMac, LOG1, FL(" Received AGGR_QOS_RSP from HAL"));)
	SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
	pAggrQosRspMsg = (tpAggrAddTsParams) (limMsg->bodyptr);
	if (NULL == pAggrQosRspMsg) {
		PELOGE(lim_log(pMac, LOGE, FL("NULL pAggrQosRspMsg"));)
		return;
	}
	psessionEntry =
		pe_find_session_by_session_id(pMac, pAggrQosRspMsg->sessionId);
	if (NULL == psessionEntry) {
		PELOGE(lim_log(pMac, LOGE,
			       FL("Cant find session entry for %s"), __func__);
		       )
		if (pAggrQosRspMsg != NULL) {
			cdf_mem_free(pAggrQosRspMsg);
		}
		return;
	}
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return;
	}
	for (i = 0; i < HAL_QOS_NUM_AC_MAX; i++) {
		if ((((1 << i) & pAggrQosRspMsg->tspecIdx)) &&
		    (pAggrQosRspMsg->status[i] != QDF_STATUS_SUCCESS)) {
			sir_copy_mac_addr(peerMacAddr, psessionEntry->bssId);
			addTsParam.staIdx = pAggrQosRspMsg->staIdx;
			addTsParam.sessionId = pAggrQosRspMsg->sessionId;
			addTsParam.tspec = pAggrQosRspMsg->tspec[i];
			addTsParam.tspecIdx = pAggrQosRspMsg->tspecIdx;
			lim_send_delts_req_action_frame(pMac, peerMacAddr, rspReqd,
							&addTsParam.tspec.tsinfo,
							&addTsParam.tspec,
							psessionEntry);
			pSta =
				dph_lookup_assoc_id(pMac, addTsParam.staIdx, &assocId,
						    &psessionEntry->dph.dphHashTable);
			if (pSta != NULL) {
				lim_admit_control_delete_ts(pMac, assocId,
							    &addTsParam.tspec.
							    tsinfo, NULL,
							    (uint8_t *) &
							    addTsParam.tspecIdx);
			}
		}
	}
	lim_ft_send_aggr_qos_rsp(pMac, rspReqd, pAggrQosRspMsg,
				 psessionEntry->smeSessionId);
	if (pAggrQosRspMsg != NULL) {
		cdf_mem_free(pAggrQosRspMsg);
	}
	return;
}
tSirRetStatus lim_process_ft_aggr_qos_req(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	tSirMsgQ msg;
	tSirAggrQosReq *aggrQosReq = (tSirAggrQosReq *) pMsgBuf;
	tpAggrAddTsParams pAggrAddTsParam;
	tpPESession psessionEntry = NULL;
	tpLimTspecInfo tspecInfo;
	uint8_t ac;
	tpDphHashNode pSta;
	uint16_t aid;
	uint8_t sessionId;
	int i;

	pAggrAddTsParam = cdf_mem_malloc(sizeof(tAggrAddTsParams));
	if (NULL == pAggrAddTsParam) {
		PELOGE(lim_log(pMac, LOGE, FL("AllocateMemory() failed"));)
		return eSIR_MEM_ALLOC_FAILED;
	}

	psessionEntry = pe_find_session_by_bssid(pMac, aggrQosReq->bssid.bytes,
						 &sessionId);

	if (psessionEntry == NULL) {
		PELOGE(lim_log
			       (pMac, LOGE,
			       FL("psession Entry Null for sessionId = %d"),
			       aggrQosReq->sessionId);
		       )
		cdf_mem_free(pAggrAddTsParam);
		return eSIR_FAILURE;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		cdf_mem_free(pAggrAddTsParam);
		return eSIR_FAILURE;
	}

	pSta = dph_lookup_hash_entry(pMac, aggrQosReq->bssid.bytes, &aid,
				     &psessionEntry->dph.dphHashTable);
	if (pSta == NULL) {
		PELOGE(lim_log(pMac, LOGE,
			       FL
				       ("Station context not found - ignoring AddTsRsp"));
		       )
		cdf_mem_free(pAggrAddTsParam);
		return eSIR_FAILURE;
	}

	cdf_mem_set((uint8_t *) pAggrAddTsParam, sizeof(tAggrAddTsParams), 0);
	pAggrAddTsParam->staIdx = psessionEntry->staId;
	/* Fill in the sessionId specific to PE */
	pAggrAddTsParam->sessionId = sessionId;
	pAggrAddTsParam->tspecIdx = aggrQosReq->aggrInfo.tspecIdx;

	for (i = 0; i < HAL_QOS_NUM_AC_MAX; i++) {
		if (aggrQosReq->aggrInfo.tspecIdx & (1 << i)) {
			tSirMacTspecIE *pTspec =
				&aggrQosReq->aggrInfo.aggrAddTsInfo[i].tspec;
			/* Since AddTS response was successful, check for the PSB flag
			 * and directional flag inside the TS Info field.
			 * An AC is trigger enabled AC if the PSB subfield is set to 1
			 * in the uplink direction.
			 * An AC is delivery enabled AC if the PSB subfield is set to 1
			 * in the downlink direction.
			 * An AC is trigger and delivery enabled AC if the PSB subfield
			 * is set to 1 in the bi-direction field.
			 */
			if (pTspec->tsinfo.traffic.psb == 1) {
				lim_set_tspec_uapsd_mask_per_session(pMac,
								     psessionEntry,
								     &pTspec->
								     tsinfo,
								     SET_UAPSD_MASK);
			} else {
				lim_set_tspec_uapsd_mask_per_session(pMac,
								     psessionEntry,
								     &pTspec->
								     tsinfo,
								     CLEAR_UAPSD_MASK);
			}
			/*
			 * ADDTS success, so AC is now admitted.
			 * We shall now use the default
			 * EDCA parameters as advertised by AP and
			 * send the updated EDCA params
			 * to HAL.
			 */
			ac = upToAc(pTspec->tsinfo.traffic.userPrio);
			if (pTspec->tsinfo.traffic.direction ==
			    SIR_MAC_DIRECTION_UPLINK) {
				psessionEntry->
				gAcAdmitMask
				[SIR_MAC_DIRECTION_UPLINK] |=
					(1 << ac);
			} else if (pTspec->tsinfo.traffic.direction ==
				   SIR_MAC_DIRECTION_DNLINK) {
				psessionEntry->
				gAcAdmitMask
				[SIR_MAC_DIRECTION_DNLINK] |=
					(1 << ac);
			} else if (pTspec->tsinfo.traffic.direction ==
				   SIR_MAC_DIRECTION_BIDIR) {
				psessionEntry->
				gAcAdmitMask
				[SIR_MAC_DIRECTION_UPLINK] |=
					(1 << ac);
				psessionEntry->
					gAcAdmitMask
					[SIR_MAC_DIRECTION_DNLINK] |=
					(1 << ac);
			}
			lim_set_active_edca_params(pMac,
						   psessionEntry->gLimEdcaParams,
						   psessionEntry);

				lim_send_edca_params(pMac,
					     psessionEntry->gLimEdcaParamsActive,
					     pSta->bssId);

			if (eSIR_SUCCESS !=
			    lim_tspec_add(pMac, pSta->staAddr, pSta->assocId,
					  pTspec, 0, &tspecInfo)) {
				PELOGE(lim_log
					       (pMac, LOGE,
					       FL
						       ("Adding entry in lim Tspec Table failed "));
				       )
				pMac->lim.gLimAddtsSent = false;
				cdf_mem_free(pAggrAddTsParam);
				return eSIR_FAILURE;
			}

			pAggrAddTsParam->tspec[i] =
				aggrQosReq->aggrInfo.aggrAddTsInfo[i].tspec;
		}
	}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (!pMac->roam.configParam.isRoamOffloadEnabled ||
	    (pMac->roam.configParam.isRoamOffloadEnabled &&
	     !psessionEntry->is11Rconnection))
#endif
	{
	msg.type = WMA_AGGR_QOS_REQ;
	msg.bodyptr = pAggrAddTsParam;
	msg.bodyval = 0;

	/* We need to defer any incoming messages until we get a
	 * WMA_AGGR_QOS_RSP from HAL.
	 */
	SET_LIM_PROCESS_DEFD_MESGS(pMac, false);
	MTRACE(mac_trace_msg_tx(pMac, psessionEntry->peSessionId, msg.type));

	if (eSIR_SUCCESS != wma_post_ctrl_msg(pMac, &msg)) {
			PELOGW(lim_log
				       (pMac, LOGW, FL("wma_post_ctrl_msg() failed"));
			       )
			SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
			cdf_mem_free(pAggrAddTsParam);
			return eSIR_FAILURE;
		}
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	else {
		/* Implies it is a LFR3.0 based 11r connection
		 * so donot send add ts request to fimware since it
		 * already has the RIC IEs */

		/* Send the Aggr QoS response to SME */
		lim_ft_send_aggr_qos_rsp(pMac, true, pAggrAddTsParam,
					 psessionEntry->smeSessionId);
		if (pAggrAddTsParam != NULL) {
			cdf_mem_free(pAggrAddTsParam);
		}
	}
#endif

	return eSIR_SUCCESS;
}
/**
 * lim_send_preauth_scan_offload() - Send scan command to handle preauth.
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: pe session id
 * @ft_preauth_req: Preauth request with parameters
 *
 * Builds a single channel scan request and sends it to WMA.
 * Scan dwell time is the time allocated to go to preauth candidate
 * channel for auth frame exchange.
 *
 * Return: Status of sending message to WMA.
 */
QDF_STATUS lim_send_preauth_scan_offload(tpAniSirGlobal mac_ctx,
			uint8_t session_id,
			tSirFTPreAuthReq *ft_preauth_req)
{
	tSirScanOffloadReq *scan_offload_req;
	tSirRetStatus rc = eSIR_SUCCESS;
	tSirMsgQ msg;

	scan_offload_req = cdf_mem_malloc(sizeof(tSirScanOffloadReq));
	if (NULL == scan_offload_req) {
		lim_log(mac_ctx, LOGE,
			FL("Memory allocation failed for pScanOffloadReq"));
		return QDF_STATUS_E_NOMEM;
	}

	cdf_mem_zero(scan_offload_req, sizeof(tSirScanOffloadReq));

	msg.type = WMA_START_SCAN_OFFLOAD_REQ;
	msg.bodyptr = scan_offload_req;
	msg.bodyval = 0;

	cdf_mem_copy((uint8_t *) &scan_offload_req->selfMacAddr.bytes,
		     (uint8_t *) ft_preauth_req->self_mac_addr,
		     sizeof(tSirMacAddr));

	cdf_mem_copy((uint8_t *) &scan_offload_req->bssId.bytes,
		     (uint8_t *) ft_preauth_req->currbssId,
		     sizeof(tSirMacAddr));
	scan_offload_req->scanType = eSIR_PASSIVE_SCAN;
	/*
	 * P2P_SCAN_TYPE_LISTEN tells firmware to allow mgt frames to/from
	 * mac address that is not of connected AP.
	 */
	scan_offload_req->p2pScanType = P2P_SCAN_TYPE_LISTEN;
	scan_offload_req->restTime = 0;
	scan_offload_req->minChannelTime = LIM_FT_PREAUTH_SCAN_TIME;
	scan_offload_req->maxChannelTime = LIM_FT_PREAUTH_SCAN_TIME;
	scan_offload_req->sessionId = session_id;
	scan_offload_req->channelList.numChannels = 1;
	scan_offload_req->channelList.channelNumber[0] =
		ft_preauth_req->preAuthchannelNum;
	wma_get_scan_id(&ft_preauth_req->scan_id);
	scan_offload_req->scan_id = ft_preauth_req->scan_id;
	scan_offload_req->scan_requestor_id = PREAUTH_REQUESTOR_ID;

	lim_log(mac_ctx, LOG1,
		FL("Scan request: duration %u, session %hu, chan %hu"),
		scan_offload_req->maxChannelTime, session_id,
		ft_preauth_req->preAuthchannelNum);

	rc = wma_post_ctrl_msg(mac_ctx, &msg);
	if (rc != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGE, FL("START_SCAN_OFFLOAD failed %u"), rc);
		cdf_mem_free(scan_offload_req);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}


/**
 * lim_preauth_scan_event_handler() - Process firmware preauth scan events
 *
 * @mac_ctx:Pointer to global MAC structure
 * @event: Scan event
 * @session_id: session entry
 * @scan_id: scan id from WMA scan event.
 *
 * If scan event signifies failure or successful completion, operation
 * is complete.
 * If scan event signifies that STA is on foreign channel, send auth frame
 *
 * Return: void
 */

void lim_preauth_scan_event_handler(tpAniSirGlobal mac_ctx,
				tSirScanEventType event,
				uint8_t session_id,
				uint32_t scan_id)
{
	tpPESession session_entry;

	session_entry = pe_find_session_by_session_id(mac_ctx, session_id);
	if (session_entry == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("SessionId:%d Session Does not exist"), session_id);
		return;
	}

	switch (event) {
	case SCAN_EVENT_START_FAILED:
		/* Scan command is rejected by firmware */
		lim_log(mac_ctx, LOGE, FL("Failed to start preauth scan"));
		lim_post_ft_pre_auth_rsp(mac_ctx, eSIR_FAILURE, NULL, 0,
					 session_entry);
		return;

	case SCAN_EVENT_COMPLETED:
		/*
		 * Scan either completed succesfully or or got terminated
		 * after successful auth, or timed out. Either way, STA
		 * is back to home channel. Data traffic can continue.
		 */
		lim_ft_process_pre_auth_result(mac_ctx, QDF_STATUS_SUCCESS,
			session_entry);
		break;

	case SCAN_EVENT_FOREIGN_CHANNEL:
		/* Sta is on candidate channel. Send auth */
		lim_perform_ft_pre_auth(mac_ctx, QDF_STATUS_SUCCESS, NULL,
					session_entry);
		break;
	default:
		/* Don't print message for scan events that are ignored */
		break;
	}
}

#endif /* WLAN_FEATURE_VOWIFI_11R */
