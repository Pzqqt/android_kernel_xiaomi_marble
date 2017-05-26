/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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
		qdf_mem_set(&psessionEntry->ftPEContext, sizeof(tftPEContext),
			    0);
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
		pe_err("psessionEntry is NULL");
		return;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		pe_err("psessionEntry is not in STA mode");
		return;
	}

	if (NULL != psessionEntry->ftPEContext.pFTPreAuthReq) {
		pe_debug("Freeing pFTPreAuthReq: %p",
			       psessionEntry->ftPEContext.pFTPreAuthReq);
		if (NULL !=
		    psessionEntry->ftPEContext.pFTPreAuthReq->
		    pbssDescription) {
			qdf_mem_free(psessionEntry->ftPEContext.pFTPreAuthReq->
				     pbssDescription);
			psessionEntry->ftPEContext.pFTPreAuthReq->
			pbssDescription = NULL;
		}
		qdf_mem_free(psessionEntry->ftPEContext.pFTPreAuthReq);
		psessionEntry->ftPEContext.pFTPreAuthReq = NULL;
	}

	if (psessionEntry->ftPEContext.pAddBssReq) {
		qdf_mem_free(psessionEntry->ftPEContext.pAddBssReq);
		psessionEntry->ftPEContext.pAddBssReq = NULL;
	}

	if (psessionEntry->ftPEContext.pAddStaReq) {
		qdf_mem_free(psessionEntry->ftPEContext.pAddStaReq);
		psessionEntry->ftPEContext.pAddStaReq = NULL;
	}

	/* The session is being deleted, cleanup the contents */
	qdf_mem_set(&psessionEntry->ftPEContext, sizeof(tftPEContext), 0);
}

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
/*------------------------------------------------------------------
 *
 * Create the new Add Bss Req to the new AP.
 * This will be used when we are ready to FT to the new AP.
 * The newly created ft Session entry is passed to this function
 *
 *------------------------------------------------------------------*/
void lim_ft_prepare_add_bss_req(tpAniSirGlobal pMac,
		uint8_t updateEntry, tpPESession pftSessionEntry,
		tpSirBssDescription bssDescription)
{
	tpAddBssParams pAddBssParams = NULL;
	tAddStaParams *sta_ctx;
	uint8_t chanWidthSupp = 0;
	tSchBeaconStruct *pBeaconStruct;

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(pftSessionEntry)) {
		pe_err("psessionEntry is not in STA mode");
		return;
	}

	pBeaconStruct = qdf_mem_malloc(sizeof(tSchBeaconStruct));
	if (NULL == pBeaconStruct) {
		pe_err("Unable to allocate memory for creating ADD_BSS");
		return;
	}
	/* Package SIR_HAL_ADD_BSS_REQ message parameters */
	pAddBssParams = qdf_mem_malloc(sizeof(tAddBssParams));
	if (NULL == pAddBssParams) {
		qdf_mem_free(pBeaconStruct);
		pe_err("Unable to allocate memory for creating ADD_BSS");
		return;
	}

	lim_extract_ap_capabilities(pMac, (uint8_t *) bssDescription->ieFields,
			lim_get_ielen_from_bss_description(bssDescription),
			pBeaconStruct);

	if (pMac->lim.gLimProtectionControl !=
	    WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
		lim_decide_sta_protection_on_assoc(pMac, pBeaconStruct,
						   pftSessionEntry);

	qdf_mem_copy(pAddBssParams->bssId, bssDescription->bssId,
		     sizeof(tSirMacAddr));

	/* Fill in tAddBssParams selfMacAddr */
	qdf_mem_copy(pAddBssParams->selfMacAddr, pftSessionEntry->selfMacAddr,
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
	qdf_mem_copy(pAddBssParams->rateSet.rate,
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
		qdf_mem_copy(&pAddBssParams->staContext.capab_info,
			     &pBeaconStruct->capabilityInfo,
			     sizeof(pAddBssParams->staContext.capab_info));
		qdf_mem_copy(&pAddBssParams->staContext.ht_caps,
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

	pe_debug("SIR_HAL_ADD_BSS_REQ with channel: %d",
		pAddBssParams->currentOperChannel);

	/* Populate the STA-related parameters here */
	/* Note that the STA here refers to the AP */
	{
		pAddBssParams->staContext.staType = STA_ENTRY_OTHER;

		qdf_mem_copy(pAddBssParams->staContext.bssId,
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
				    pftSessionEntry->vht_config.su_beam_formee)
					sta_ctx->vhtTxBFCapable
						= 1;
				if (pBeaconStruct->VHTCaps.suBeamformeeCap &&
				    pftSessionEntry->vht_config.su_beam_former)
					sta_ctx->enable_su_tx_bformer = 1;
			}
			if (lim_is_session_he_capable(pftSessionEntry) &&
				pBeaconStruct->vendor_he_cap.present)
				lim_intersect_ap_he_caps(pftSessionEntry,
					pAddBssParams, pBeaconStruct, NULL);

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
		lim_populate_peer_rate_set(pMac,
					   &pAddBssParams->staContext.
					   supportedRates,
					   pBeaconStruct->HTCaps.supportedMCSSet,
					   false, pftSessionEntry,
					   &pBeaconStruct->VHTCaps,
					   &pBeaconStruct->vendor_he_cap);
	}

	pAddBssParams->maxTxPower = pftSessionEntry->maxTxPower;

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
	if (!lim_is_roam_synch_in_progress(pftSessionEntry)) {
		pftSessionEntry->limMlmState =
			eLIM_MLM_WT_ADD_BSS_RSP_FT_REASSOC_STATE;
		MTRACE(mac_trace
			(pMac, TRACE_CODE_MLM_STATE,
			pftSessionEntry->peSessionId,
			eLIM_MLM_WT_ADD_BSS_RSP_FT_REASSOC_STATE));
	}
	pAddBssParams->halPersona = (uint8_t) pftSessionEntry->pePersona;

	pftSessionEntry->ftPEContext.pAddBssReq = pAddBssParams;

	pe_debug("Saving SIR_HAL_ADD_BSS_REQ for pre-auth ap");

	qdf_mem_free(pBeaconStruct);
	return;
}
#endif

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
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
	ePhyChanBondState cbEnabledMode;

	pBeaconStruct = qdf_mem_malloc(sizeof(tSchBeaconStruct));
	if (NULL == pBeaconStruct) {
		pe_err("No memory for creating lim_fill_ft_session");
		return;
	}

	/* Retrieve the session that was already created and update the entry */
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
	qdf_mem_copy(pftSessionEntry->rateSet.rate,
		     pBeaconStruct->supportedRates.rate,
		     pBeaconStruct->supportedRates.numRates);

	pftSessionEntry->extRateSet.numRates =
		pBeaconStruct->extendedRates.numRates;
	qdf_mem_copy(pftSessionEntry->extRateSet.rate,
		     pBeaconStruct->extendedRates.rate,
		     pftSessionEntry->extRateSet.numRates);

	pftSessionEntry->ssId.length = pBeaconStruct->ssId.length;
	qdf_mem_copy(pftSessionEntry->ssId.ssId, pBeaconStruct->ssId.ssId,
		     pftSessionEntry->ssId.length);

	pftSessionEntry->dot11mode =
			psessionEntry->ftPEContext.pFTPreAuthReq->dot11mode;
	pe_debug("dot11mode: %d", pftSessionEntry->dot11mode);
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

	if (IS_BSS_VHT_CAPABLE(pBeaconStruct->VHTCaps) &&
		pBeaconStruct->VHTOperation.present &&
		pftSessionEntry->vhtCapability) {
		pftSessionEntry->vhtCapabilityPresentInBeacon = 1;
	} else {
		pftSessionEntry->vhtCapabilityPresentInBeacon = 0;
	}
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
				pe_warn("Invalid sec ch offset");
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

	/* Store beaconInterval */
	pftSessionEntry->beaconParams.beaconInterval =
		pbssDescription->beaconInterval;
	pftSessionEntry->bssType = psessionEntry->bssType;

	pftSessionEntry->statypeForBss = STA_ENTRY_PEER;
	pftSessionEntry->nwType = pbssDescription->nwType;


	if (pftSessionEntry->bssType == eSIR_INFRASTRUCTURE_MODE) {
		pftSessionEntry->limSystemRole = eLIM_STA_ROLE;
	} else {
		/* Throw an error & return & make sure to delete the session */
		pe_warn("Invalid bss type");
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

	pftSessionEntry->is11Rconnection = psessionEntry->is11Rconnection;
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

	pe_debug("Reg max: %d local pwr: %d, ini tx pwr: %d max tx pwr: %d",
		regMax, localPowerConstraint,
		pMac->roam.configParam.nTxPowerCap,
		pftSessionEntry->maxTxPower);
	if (!lim_is_roam_synch_in_progress(psessionEntry)) {
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
	if ((pftSessionEntry->limRFBand == SIR_BAND_2_4_GHZ) &&
		(pftSessionEntry->htSupportedChannelWidthSet ==
		eHT_CHANNEL_WIDTH_40MHZ))
		lim_init_obss_params(pMac, pftSessionEntry);

	pftSessionEntry->enableHtSmps = psessionEntry->enableHtSmps;
	pftSessionEntry->htSmpsvalue = psessionEntry->htSmpsvalue;
	/*
	 * By default supported NSS 1x1 is set to true
	 * and later on updated while determining session
	 * supported rates which is the intersection of
	 * self and peer rates
	 */
	pftSessionEntry->supported_nss_1x1 = true;
	pe_debug("FT enable smps: %d mode: %d supported nss 1x1: %d",
		pftSessionEntry->enableHtSmps,
		pftSessionEntry->htSmpsvalue,
		pftSessionEntry->supported_nss_1x1);

	qdf_mem_free(pBeaconStruct);
}
#endif

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
		pe_err("%s: Unable to find session for the following bssid",
			       __func__);
		lim_print_mac_addr(pMac, pKeyInfo->bssid.bytes, LOGE);
		return false;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		pe_err("psessionEntry is not in STA mode");
		return false;
	}

	if (NULL == psessionEntry->ftPEContext.pAddBssReq) {
		/* AddBss Req is NULL, save the keys to configure them later. */
		tpLimMlmSetKeysReq pMlmSetKeysReq =
			&psessionEntry->ftPEContext.PreAuthKeyInfo.
			extSetStaKeyParam;

		qdf_mem_zero(pMlmSetKeysReq, sizeof(tLimMlmSetKeysReq));
		qdf_copy_macaddr(&pMlmSetKeysReq->peer_macaddr,
				 &pKeyInfo->bssid);
		pMlmSetKeysReq->sessionId = psessionEntry->peSessionId;
		pMlmSetKeysReq->smesessionId = psessionEntry->smeSessionId;
		pMlmSetKeysReq->edType = pKeyInfo->keyMaterial.edType;
		pMlmSetKeysReq->numKeys = pKeyInfo->keyMaterial.numKeys;
		qdf_mem_copy((uint8_t *) &pMlmSetKeysReq->key,
			     (uint8_t *) &pKeyInfo->keyMaterial.key,
			     sizeof(tSirKeys));

		psessionEntry->ftPEContext.PreAuthKeyInfo.
		extSetStaKeyParamValid = true;

		if (psessionEntry->ftPEContext.pAddStaReq == NULL) {
			pe_err("pAddStaReq is NULL");
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
		qdf_mem_copy((uint8_t *) &pAddBssParams->extSetStaKeyParam.key,
			     (uint8_t *) &pKeyInfo->keyMaterial.key,
			     sizeof(tSirKeys));
		if (eSIR_SUCCESS !=
		    wlan_cfg_get_int(pMac, WNI_CFG_SINGLE_TID_RC, &val)) {
			pe_warn("Unable to read WNI_CFG_SINGLE_TID_RC");
		}

		pAddBssParams->extSetStaKeyParam.singleTidRc = val;
		pe_debug("Key valid: %d keyLength: %d",
			pAddBssParams->extSetStaKeyParamValid,
			pAddBssParams->extSetStaKeyParam.key[0].keyLength);

		pAddBssParams->extSetStaKeyParam.staIdx = 0;

		pe_debug("BSSID: " MAC_ADDRESS_STR,
			       MAC_ADDR_ARRAY(pKeyInfo->bssid.bytes));

		qdf_copy_macaddr(&pAddBssParams->extSetStaKeyParam.peer_macaddr,
				 &pKeyInfo->bssid);

		pAddBssParams->extSetStaKeyParam.sendRsp = false;

	}
	return true;
}

static void
lim_ft_send_aggr_qos_rsp(tpAniSirGlobal pMac, uint8_t rspReqd,
			 tpAggrAddTsParams aggrQosRsp, uint8_t smesessionId)
{
	tpSirAggrQosRsp rsp;
	int i = 0;
	if (!rspReqd) {
		return;
	}
	rsp = qdf_mem_malloc(sizeof(tSirAggrQosRsp));
	if (NULL == rsp) {
		pe_err("AllocateMemory failed for tSirAggrQosRsp");
		return;
	}
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

void lim_process_ft_aggr_qo_s_rsp(tpAniSirGlobal pMac,
				  struct scheduler_msg *limMsg)
{
	tpAggrAddTsParams pAggrQosRspMsg = NULL;
	tAddTsParams addTsParam = { 0 };
	tpDphHashNode pSta = NULL;
	uint16_t assocId = 0;
	tSirMacAddr peerMacAddr;
	uint8_t rspReqd = 1;
	tpPESession psessionEntry = NULL;
	int i = 0;
	pe_debug(" Received AGGR_QOS_RSP from HAL");
	SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
	pAggrQosRspMsg = (tpAggrAddTsParams) (limMsg->bodyptr);
	if (NULL == pAggrQosRspMsg) {
		pe_err("NULL pAggrQosRspMsg");
		return;
	}
	psessionEntry =
		pe_find_session_by_session_id(pMac, pAggrQosRspMsg->sessionId);
	if (NULL == psessionEntry) {
		pe_err("Cant find session entry for %s", __func__);
		if (pAggrQosRspMsg != NULL) {
			qdf_mem_free(pAggrQosRspMsg);
		}
		return;
	}
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		pe_err("psessionEntry is not in STA mode");
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
		qdf_mem_free(pAggrQosRspMsg);
	}
	return;
}

tSirRetStatus lim_process_ft_aggr_qos_req(tpAniSirGlobal pMac, uint32_t *pMsgBuf)
{
	struct scheduler_msg msg = {0};
	tSirAggrQosReq *aggrQosReq = (tSirAggrQosReq *) pMsgBuf;
	tpAggrAddTsParams pAggrAddTsParam;
	tpPESession psessionEntry = NULL;
	tpLimTspecInfo tspecInfo;
	uint8_t ac;
	tpDphHashNode pSta;
	uint16_t aid;
	uint8_t sessionId;
	int i;

	pAggrAddTsParam = qdf_mem_malloc(sizeof(tAggrAddTsParams));
	if (NULL == pAggrAddTsParam) {
		pe_err("AllocateMemory() failed");
		return eSIR_MEM_ALLOC_FAILED;
	}

	psessionEntry = pe_find_session_by_bssid(pMac, aggrQosReq->bssid.bytes,
						 &sessionId);

	if (psessionEntry == NULL) {
		pe_err("psession Entry Null for sessionId: %d",
			       aggrQosReq->sessionId);
		qdf_mem_free(pAggrAddTsParam);
		return eSIR_FAILURE;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		pe_err("psessionEntry is not in STA mode");
		qdf_mem_free(pAggrAddTsParam);
		return eSIR_FAILURE;
	}

	pSta = dph_lookup_hash_entry(pMac, aggrQosReq->bssid.bytes, &aid,
				     &psessionEntry->dph.dphHashTable);
	if (pSta == NULL) {
		pe_err("Station context not found - ignoring AddTsRsp");
		qdf_mem_free(pAggrAddTsParam);
		return eSIR_FAILURE;
	}

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
				pe_err("Adding entry in lim Tspec Table failed");
				pMac->lim.gLimAddtsSent = false;
				qdf_mem_free(pAggrAddTsParam);
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
			pe_warn("wma_post_ctrl_msg() failed");
			SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
			qdf_mem_free(pAggrAddTsParam);
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
			qdf_mem_free(pAggrAddTsParam);
		}
	}
#endif

	return eSIR_SUCCESS;
}
