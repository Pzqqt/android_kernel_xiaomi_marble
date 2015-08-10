/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
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
 * This file lim_process_assoc_req_frame.cc contains the code
 * for processing Re/Association Request Frame.
 * Author:        Chandra Modumudi
 * Date:          03/18/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 * 05/26/10       js             WPA handling in (Re)Assoc frames
 *
 */
#include "cds_api.h"
#include "ani_global.h"
#include "wni_cfg.h"
#include "sir_api.h"
#include "cfg_api.h"

#include "sch_api.h"
#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_security_utils.h"
#include "lim_ser_des_utils.h"
#include "lim_sta_hash_api.h"
#include "lim_admit_control.h"
#include "cds_packet.h"
#include "lim_session_utils.h"

#include "cdf_types.h"
#include "cds_utils.h"

/**
 * lim_convert_supported_channels - Parses channel support IE
 *
 * @mac_ctx - A pointer to Global MAC structure
 * @assoc_ind - A pointer to SME ASSOC/REASSOC IND
 * @assoc_req - A pointer to ASSOC/REASSOC Request frame
 *
 * This function is called by lim_process_assoc_req_frame() to
 * parse the channel support IE in the Assoc/Reassoc Request
 * frame, and send relevant information in the SME_ASSOC_IND
 *
 * return None
 */
static void
lim_convert_supported_channels(tpAniSirGlobal mac_ctx,
		tpLimMlmAssocInd assoc_ind, tSirAssocReq *assoc_req)
{
	uint16_t i, j, index = 0;
	uint8_t first_chn_no;
	uint8_t chn_count;
	uint8_t next_chn_no;
	uint8_t channel_offset = 0;

	if (assoc_req->supportedChannels.length >=
		SIR_MAX_SUPPORTED_CHANNEL_LIST) {
		lim_log(mac_ctx, LOG1,
			FL("Number of supported channels:%d is more than MAX"),
			assoc_req->supportedChannels.length);
		assoc_ind->supportedChannels.numChnl = 0;
		return;
	}

	for (i = 0; i < (assoc_req->supportedChannels.length); i++) {
		/* Get First Channel Number */
		first_chn_no =
			assoc_req->supportedChannels.supportedChannels[i];
		assoc_ind->supportedChannels.channelList[index] =
			first_chn_no;
		i++;
		index++;

		/* Get Number of Channels in a Subband */
		chn_count =
			assoc_req->supportedChannels.supportedChannels[i];
		PELOG2(lim_log(mac_ctx, LOG2,
			FL("Rcv assoc_req: chnl=%d, numOfChnl=%d "),
			first_chn_no, chn_count);)
		if (index >= SIR_MAX_SUPPORTED_CHANNEL_LIST) {
			PELOG2(lim_log(mac_ctx, LOGW,
			FL("Channel count is more than max supported =%d "),
			chn_count);)
			assoc_ind->supportedChannels.numChnl = 0;
			return;
		}
		if (chn_count <= 1)
			continue;
		next_chn_no = first_chn_no;
		if (SIR_BAND_5_GHZ == lim_get_rf_band(first_chn_no))
			channel_offset =  SIR_11A_FREQUENCY_OFFSET;
		else if (SIR_BAND_2_4_GHZ == lim_get_rf_band(first_chn_no))
			channel_offset = SIR_11B_FREQUENCY_OFFSET;
		else
			continue;

		for (j = 1; j < chn_count; j++) {
			next_chn_no += channel_offset;
			assoc_ind->supportedChannels.channelList[index]
				= next_chn_no;
			index++;
			if (index >= SIR_MAX_SUPPORTED_CHANNEL_LIST) {
				PELOG2(lim_log(mac_ctx, LOGW,
				FL("Channel count is more than supported =%d "),
				chn_count);)
				assoc_ind->supportedChannels.numChnl = 0;
				return;
			}
		}
	}

	assoc_ind->supportedChannels.numChnl = (uint8_t) index;
	PELOG2(lim_log(mac_ctx, LOG2,
		FL("Send AssocInd to WSM:  minPwr %d, maxPwr %d, numChnl %d"),
		assoc_ind->powerCap.minTxPower,
		assoc_ind->powerCap.maxTxPower,
		assoc_ind->supportedChannels.numChnl);)
}

/**---------------------------------------------------------------
   \fn     lim_check_sta_in_pe_entries
   \brief  This function is called by lim_process_assoc_req_frame()
 \       to check if STA entry already exists in any of the
 \       PE entries of the AP. If it exists, deauth will be
 \       sent on that session and the STA deletion will
 \       happen. After this, the ASSOC request will be
 \       processed
 \
   \param pMac - A pointer to Global MAC structure
   \param pHdr - A pointer to the MAC header
   \return None
   ------------------------------------------------------------------*/
void lim_check_sta_in_pe_entries(tpAniSirGlobal pMac, tpSirMacMgmtHdr pHdr)
{
	uint8_t i;
	uint16_t assocId = 0;
	tpDphHashNode pStaDs = NULL;
	tpPESession psessionEntry = NULL;

	for (i = 0; i < pMac->lim.maxBssId; i++) {
		if ((&pMac->lim.gpSession[i] != NULL) &&
		    (pMac->lim.gpSession[i].valid) &&
		    (pMac->lim.gpSession[i].pePersona == CDF_SAP_MODE)) {

			psessionEntry = &pMac->lim.gpSession[i];
			pStaDs = dph_lookup_hash_entry(pMac, pHdr->sa, &assocId,
						       &psessionEntry->dph.
						       dphHashTable);
			if (pStaDs
#ifdef WLAN_FEATURE_11W
			    && !pStaDs->rmfEnabled
#endif
			    ) {
				lim_log(pMac, LOGE,
					FL
						("Sending Deauth and Deleting existing STA entry: "
						MAC_ADDRESS_STR),
					MAC_ADDR_ARRAY(psessionEntry->
						       selfMacAddr));
				lim_send_deauth_mgmt_frame(pMac,
							   eSIR_MAC_UNSPEC_FAILURE_REASON,
							   (uint8_t *) pHdr->sa,
							   psessionEntry, false);
				lim_trigger_sta_deletion(pMac, pStaDs,
							 psessionEntry);
				break;
			}
		}
	}
}

/**---------------------------------------------------------------
   \fn     lim_process_assoc_req_frame
   \brief  This function is called by limProcessMessageQueue()
 \       upon Re/Association Request frame reception in
 \       BTAMP AP or Soft AP role.
 \
   \param pMac
   \param *pRxPacketInfo    - A pointer to Buffer descriptor + associated PDUs
   \param subType - Indicates whether it is Association Request(=0)
 \                or Reassociation Request(=1) frame
   \return None
   ------------------------------------------------------------------*/
void
lim_process_assoc_req_frame(tpAniSirGlobal pMac, uint8_t *pRxPacketInfo,
			    uint8_t subType, tpPESession psessionEntry)
{
	uint8_t updateContext;
	uint8_t *pBody;
	uint16_t peerIdx, temp;
	uint32_t val;
	int32_t framelen;
	tSirRetStatus status;
	tpSirMacMgmtHdr pHdr;
	struct tLimPreAuthNode *pStaPreAuthContext;
	tAniAuthType authType;
	tSirMacCapabilityInfo localCapabilities;
	tpDphHashNode pStaDs = NULL;
	tpSirAssocReq pAssocReq, pTempAssocReq;
	tLimMlmStates mlmPrevState;
	tDot11fIERSN Dot11fIERSN;
	tDot11fIEWPA Dot11fIEWPA;
	uint32_t phyMode;
	tHalBitVal qosMode;
	tHalBitVal wsmMode, wmeMode;
	uint8_t *wpsIe = NULL;
	uint8_t *ht_cap_ie = NULL;
	tSirMacRateSet basicRates;
	uint8_t i = 0, j = 0;
	bool pmfConnection = false;
#ifdef WLAN_FEATURE_11W
	tPmfSaQueryTimerId timerId;
	uint32_t retryInterval;
#endif
	uint16_t assoc_id = 0;
	bool assoc_req_copied = false;

	lim_get_phy_mode(pMac, &phyMode, psessionEntry);

	limGetQosMode(psessionEntry, &qosMode);

	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	framelen = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

	lim_log(pMac, LOG1,
		FL("Received %s Req Frame on sessionid: %d systemrole %d"
		   " limMlmState %d from: " MAC_ADDRESS_STR),
		(LIM_ASSOC == subType) ? "Assoc" : "ReAssoc",
		psessionEntry->peSessionId, GET_LIM_SYSTEM_ROLE(psessionEntry),
		psessionEntry->limMlmState, MAC_ADDR_ARRAY(pHdr->sa));

	if (LIM_IS_STA_ROLE(psessionEntry) ||
	    LIM_IS_BT_AMP_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE,
			FL("received unexpected ASSOC REQ on sessionid: %d "
			   "sys subType=%d for role=%d from: " MAC_ADDRESS_STR),
			psessionEntry->peSessionId, subType,
			GET_LIM_SYSTEM_ROLE(psessionEntry),
			MAC_ADDR_ARRAY(pHdr->sa));
		sir_dump_buf(pMac, SIR_LIM_MODULE_ID, LOG3,
			     WMA_GET_RX_MPDU_DATA(pRxPacketInfo), framelen);
		return;
	}

	/*
	 * If a STA is already present in DPH and it
	 * is initiating a Assoc re-transmit, do not
	 * process it. This can happen when first Assoc Req frame
	 * is received but ACK lost at STA side. The ACK for this
	 * dropped Assoc Req frame should be sent by HW. Host simply
	 * does not process it once the entry for the STA is already
	 * present in DPH.
	 */
	pStaDs = dph_lookup_hash_entry(pMac, pHdr->sa, &assoc_id,
					&psessionEntry->dph.dphHashTable);
	if ((NULL != pStaDs)) {
		if (pHdr->fc.retry > 0) {
			lim_log(pMac, LOGE,
			FL("STA is initiating Assoc Req after ACK lost. Do not process"
			" sessionid: %d sys subType=%d for role=%d from: "
			MAC_ADDRESS_STR), psessionEntry->peSessionId,
			subType, GET_LIM_SYSTEM_ROLE(psessionEntry),
			MAC_ADDR_ARRAY(pHdr->sa));
		} else {
			/*
			 * STA might have missed the assoc response,
			 * so it is sending assoc request frame again.
			 */
			lim_send_assoc_rsp_mgmt_frame(pMac, eSIR_SUCCESS,
				pStaDs->assocId, pStaDs->staAddr,
				pStaDs->mlmStaContext.subType, pStaDs,
				psessionEntry);
			lim_log(pMac, LOGE,
			FL("DUT already received an assoc request frame "
			"and STA is sending another assoc req.So, do not "
			"Process sessionid: %d sys subType=%d for role=%d "
			"from: "MAC_ADDRESS_STR),
			psessionEntry->peSessionId, subType,
			psessionEntry->limSystemRole,
			MAC_ADDR_ARRAY(pHdr->sa));
		}
		return;
	}

	lim_check_sta_in_pe_entries(pMac, pHdr);

	/* Get pointer to Re/Association Request frame body */
	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);

	if (lim_is_group_addr(pHdr->sa)) {
		/* Received Re/Assoc Req frame from a BC/MC address */
		/* Log error and ignore it */
		lim_log(pMac, LOGE,
			FL("Received %s Req on sessionid: %d frame from a "
			   "BC/MC address" MAC_ADDRESS_STR),
			(LIM_ASSOC == subType) ? "Assoc" : "ReAssoc",
			psessionEntry->peSessionId, MAC_ADDR_ARRAY(pHdr->sa));
		return;
	}

	sir_dump_buf(pMac, SIR_LIM_MODULE_ID, LOG2, (uint8_t *) pBody, framelen);

	if (cdf_mem_compare((uint8_t *) pHdr->sa, (uint8_t *) pHdr->da,
			    (uint8_t) (sizeof(tSirMacAddr)))) {
		lim_log(pMac, LOGE,
			FL("Rejected Assoc Req frame Since same mac as"
			   " SAP/GO"));
		lim_send_assoc_rsp_mgmt_frame(pMac, eSIR_MAC_UNSPEC_FAILURE_STATUS,
					      1, pHdr->sa, subType, 0,
					      psessionEntry);
		return;
	}
	/* If TKIP counter measures active send Assoc Rsp frame to station with eSIR_MAC_MIC_FAILURE_REASON */
	if (psessionEntry->bTkipCntrMeasActive &&
	    LIM_IS_AP_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("TKIP counter measure is active"));
		lim_send_assoc_rsp_mgmt_frame(pMac,
					      eSIR_MAC_MIC_FAILURE_REASON,
					      1, pHdr->sa, subType,
					      0, psessionEntry);
		return;
	}
	/* Allocate memory for the Assoc Request frame */
	pAssocReq = cdf_mem_malloc(sizeof(*pAssocReq));
	if (NULL == pAssocReq) {
		lim_log(pMac, LOGP, FL("Allocate Memory failed in assoc_req"));
		return;
	}
	cdf_mem_set((void *)pAssocReq, sizeof(*pAssocReq), 0);

	/* Parse Assoc Request frame */
	if (subType == LIM_ASSOC)
		status =
			sir_convert_assoc_req_frame2_struct(pMac, pBody, framelen,
							    pAssocReq);
	else
		status =
			sir_convert_reassoc_req_frame2_struct(pMac, pBody, framelen,
							      pAssocReq);

	if (status != eSIR_SUCCESS) {
		lim_log(pMac, LOGE,
			FL("Parse error AssocRequest, length=%d from "
			   MAC_ADDRESS_STR), framelen, MAC_ADDR_ARRAY(pHdr->sa));
		lim_send_assoc_rsp_mgmt_frame(pMac, eSIR_MAC_UNSPEC_FAILURE_STATUS,
					      1, pHdr->sa, subType, 0,
					      psessionEntry);
		goto error;
	}

	pAssocReq->assocReqFrame = cdf_mem_malloc(framelen);
	if (NULL == pAssocReq->assocReqFrame) {
		lim_log(pMac, LOGE,
			FL("Unable to allocate memory for the assoc req, "
			   "length=%d from "), framelen);
		goto error;
	}

	cdf_mem_copy((uint8_t *) pAssocReq->assocReqFrame,
		     (uint8_t *) pBody, framelen);
	pAssocReq->assocReqFrameLength = framelen;

	if (cfg_get_capability_info(pMac, &temp, psessionEntry) != eSIR_SUCCESS) {
		lim_log(pMac, LOGP, FL("could not retrieve Capabilities"));
		goto error;
	}
	lim_copy_u16((uint8_t *) &localCapabilities, temp);

	if (lim_compare_capabilities(pMac,
				     pAssocReq,
				     &localCapabilities, psessionEntry) == false)
	{
		lim_log(pMac, LOGE, FL("local caps mismatch received caps"));
		lim_log(pMac, LOGE, FL("Received %s Req with unsupported "
				       "capabilities from" MAC_ADDRESS_STR),
			(LIM_ASSOC == subType) ? "Assoc" : "ReAssoc",
			MAC_ADDR_ARRAY(pHdr->sa));
		/**
		 * Capabilities of requesting STA does not match with
		 * local capabilities. Respond with 'unsupported capabilities'
		 * status code.
		 */
		lim_send_assoc_rsp_mgmt_frame(pMac,
					      eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS,
					      1,
					      pHdr->sa, subType, 0, psessionEntry);

		goto error;
	}

	updateContext = false;

	if (lim_cmp_s_sid(pMac, &pAssocReq->ssId, psessionEntry) == false) {
		lim_log(pMac, LOGE,
			FL("Received %s Req with unmatched ssid ( Received"
			   " SSID: %.*s current SSID: %.*s ) from "
			   MAC_ADDRESS_STR),
			(LIM_ASSOC == subType) ? "Assoc" : "ReAssoc",
			pAssocReq->ssId.length, pAssocReq->ssId.ssId,
			psessionEntry->ssId.length, psessionEntry->ssId.ssId,
			MAC_ADDR_ARRAY(pHdr->sa));

		/**
		 * Received Re/Association Request with either
		 * Broadcast SSID OR with SSID that does not
		 * match with local one.
		 * Respond with unspecified status code.
		 */
		lim_send_assoc_rsp_mgmt_frame(pMac,
					      eSIR_MAC_UNSPEC_FAILURE_STATUS,
					      1,
					      pHdr->sa, subType, 0, psessionEntry);

		goto error;
	}

	/***************************************************************
	** Verify if the requested rates are available in supported rate
	** set or Extended rate set. Some APs are adding basic rates in
	** Extended rateset IE
	***************************************************************/
	basicRates.numRates = 0;

	for (i = 0;
	     i < pAssocReq->supportedRates.numRates
	     && (i < SIR_MAC_RATESET_EID_MAX); i++) {
		basicRates.rate[i] = pAssocReq->supportedRates.rate[i];
		basicRates.numRates++;
	}

	for (j = 0;
	     (j < pAssocReq->extendedRates.numRates)
	     && (i < SIR_MAC_RATESET_EID_MAX); i++, j++) {
		basicRates.rate[i] = pAssocReq->extendedRates.rate[j];
		basicRates.numRates++;
	}
	if (lim_check_rx_basic_rates(pMac, basicRates, psessionEntry) == false) {
		lim_log(pMac, LOGE, FL("Received %s Req with unsupported "
				       "rates from" MAC_ADDRESS_STR),
			(LIM_ASSOC == subType) ? "Assoc" : "ReAssoc",
			MAC_ADDR_ARRAY(pHdr->sa));
		/**
		 * Requesting STA does not support ALL BSS basic
		 * rates. Respond with 'basic rates not supported'
		 * status code.
		 */
		lim_send_assoc_rsp_mgmt_frame(pMac,
					      eSIR_MAC_BASIC_RATES_NOT_SUPPORTED_STATUS,
					      1,
					      pHdr->sa, subType, 0, psessionEntry);

		goto error;
	}

	if (LIM_IS_AP_ROLE(psessionEntry) &&
	    (psessionEntry->dot11mode == WNI_CFG_DOT11_MODE_11G_ONLY) &&
	    (pAssocReq->HTCaps.present))
	{
		lim_log(pMac, LOGE,
			FL("SOFTAP was in 11G only mode, rejecting legacy "
			   "STA : " MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));
		lim_send_assoc_rsp_mgmt_frame(pMac,
					      eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS,
					      1, pHdr->sa, subType, 0,
					      psessionEntry);
		goto error;

	} /* end if phyMode == 11G_only */

	if (LIM_IS_AP_ROLE(psessionEntry) &&
	    (psessionEntry->dot11mode == WNI_CFG_DOT11_MODE_11N_ONLY) &&
	    (!pAssocReq->HTCaps.present)) {
		lim_log(pMac, LOGE,
			FL("SOFTAP was in 11N only mode, rejecting legacy "
			   "STA : " MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));
		lim_send_assoc_rsp_mgmt_frame(pMac,
					      eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS,
					      1, pHdr->sa, subType, 0,
					      psessionEntry);
		goto error;
	} /* end if PhyMode == 11N_only */

	if (LIM_IS_AP_ROLE(psessionEntry) &&
	    (psessionEntry->dot11mode == WNI_CFG_DOT11_MODE_11AC_ONLY) &&
	    (!pAssocReq->VHTCaps.present)) {
		lim_send_assoc_rsp_mgmt_frame(pMac,
					      eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS,
					      1, pHdr->sa, subType, 0,
					      psessionEntry);
		lim_log(pMac, LOGE, FL("SOFTAP was in 11AC only mode, reject"));
		goto error;
	} /* end if PhyMode == 11AC_only */

	/* Spectrum Management (11h) specific checks */
	if (localCapabilities.spectrumMgt) {
		tSirRetStatus status = eSIR_SUCCESS;

		/* If station is 11h capable, then it SHOULD send all mandatory
		 * IEs in assoc request frame. Let us verify that
		 */
		if (pAssocReq->capabilityInfo.spectrumMgt) {
			if (!
			    ((pAssocReq->powerCapabilityPresent)
			     && (pAssocReq->supportedChannelsPresent))) {
				/* One or more required information elements are missing, log the peers error */
				if (!pAssocReq->powerCapabilityPresent) {
					lim_log(pMac, LOG1,
						FL
							("LIM Info: Missing Power capability "
							"IE in %s Req from "
							MAC_ADDRESS_STR),
						(LIM_ASSOC ==
						 subType) ? "Assoc" : "ReAssoc",
						MAC_ADDR_ARRAY(pHdr->sa));
				}
				if (!pAssocReq->supportedChannelsPresent) {
					lim_log(pMac, LOGW,
						FL
							("LIM Info: Missing Supported channel "
							"IE in %s Req from "
							MAC_ADDRESS_STR),
						(LIM_ASSOC ==
						 subType) ? "Assoc" : "ReAssoc",
						MAC_ADDR_ARRAY(pHdr->sa));

				}
			} else {
				/* Assoc request has mandatory fields */
				status =
					lim_is_dot11h_power_capabilities_in_range(pMac,
										  pAssocReq,
										  psessionEntry);
				if (eSIR_SUCCESS != status) {
					lim_log(pMac, LOGW,
						FL("LIM Info: MinTxPower(STA) > "
						   "MaxTxPower(AP) in %s Req from "
						   MAC_ADDRESS_STR),
						(LIM_ASSOC ==
						 subType) ? "Assoc" : "ReAssoc",
						MAC_ADDR_ARRAY(pHdr->sa));

				}
				status =
					lim_is_dot11h_supported_channels_valid(pMac,
									       pAssocReq);
				if (eSIR_SUCCESS != status) {
					lim_log(pMac, LOGW,
						FL("LIM Info: wrong supported "
						   "channels (STA) in %s Req from "
						   MAC_ADDRESS_STR),
						(LIM_ASSOC ==
						 subType) ? "Assoc" : "ReAssoc",
						MAC_ADDR_ARRAY(pHdr->sa));

				}
				/* IEs are valid, use them if needed */
			}
		} /* if(assoc.capabilityInfo.spectrumMgt) */
		else {
			/* As per the capabiities, the spectrum management is not enabled on the station
			 * The AP may allow the associations to happen even if spectrum management is not
			 * allowed, if the transmit power of station is below the regulatory maximum
			 */

			/* TODO: presently, this is not handled. In the current implemetation, the AP would
			 * allow the station to associate even if it doesn't support spectrum management.
			 */
		}
	} /* end of spectrum management related processing */

	if ((pAssocReq->HTCaps.present)
	    && (lim_check_mcs_set(pMac, pAssocReq->HTCaps.supportedMCSSet) ==
		false)) {
		lim_log(pMac, LOGE,
			FL("received %s req with unsupported"
			   "MCS Rate Set from " MAC_ADDRESS_STR),
			(LIM_ASSOC == subType) ? "Assoc" : "ReAssoc",
			MAC_ADDR_ARRAY(pHdr->sa));

		/**
		 * Requesting STA does not support ALL BSS MCS basic Rate set rates.
		 * Spec does not define any status code for this scenario.
		 */
		lim_send_assoc_rsp_mgmt_frame(pMac,
					      eSIR_MAC_OUTSIDE_SCOPE_OF_SPEC_STATUS,
					      1,
					      pHdr->sa, subType, 0, psessionEntry);

		goto error;
	}

	if (phyMode == WNI_CFG_PHY_MODE_11G) {

		if (wlan_cfg_get_int(pMac, WNI_CFG_11G_ONLY_POLICY, &val) !=
		    eSIR_SUCCESS) {
			lim_log(pMac, LOGP,
				FL("could not retrieve 11g-only flag"));
			goto error;
		}

		if (!pAssocReq->extendedRatesPresent && val) {
			/**
			 * Received Re/Association Request from
			 * 11b STA when 11g only policy option
			 * is set.
			 * Reject with unspecified status code.
			 */
			lim_send_assoc_rsp_mgmt_frame(pMac,
						      eSIR_MAC_BASIC_RATES_NOT_SUPPORTED_STATUS,
						      1,
						      pHdr->sa,
						      subType, 0, psessionEntry);

			lim_log(pMac, LOGE,
				FL("Rejecting Re/Assoc req from 11b STA: "));
			lim_print_mac_addr(pMac, pHdr->sa, LOGW);

#ifdef WLAN_DEBUG
			pMac->lim.gLim11bStaAssocRejectCount++;
#endif
			goto error;
		}
	}

	/* Check for 802.11n HT caps compatibility; are HT Capabilities */
	/* turned on in lim? */
	if (psessionEntry->htCapability) {
		/* There are; are they turned on in the STA? */
		if (pAssocReq->HTCaps.present) {
			/* The station *does* support 802.11n HT capability... */

			lim_log(pMac, LOG1, FL("AdvCodingCap:%d ChaWidthSet:%d "
					       "PowerSave:%d greenField:%d "
					       "shortGI20:%d shortGI40:%d"
					       "txSTBC:%d rxSTBC:%d delayBA:%d"
					       "maxAMSDUsize:%d DSSS/CCK:%d "
					       "PSMP:%d stbcCntl:%d lsigTXProt:%d"),
				pAssocReq->HTCaps.advCodingCap,
				pAssocReq->HTCaps.supportedChannelWidthSet,
				pAssocReq->HTCaps.mimoPowerSave,
				pAssocReq->HTCaps.greenField,
				pAssocReq->HTCaps.shortGI20MHz,
				pAssocReq->HTCaps.shortGI40MHz,
				pAssocReq->HTCaps.txSTBC,
				pAssocReq->HTCaps.rxSTBC,
				pAssocReq->HTCaps.delayedBA,
				pAssocReq->HTCaps.maximalAMSDUsize,
				pAssocReq->HTCaps.dsssCckMode40MHz,
				pAssocReq->HTCaps.psmp,
				pAssocReq->HTCaps.stbcControlFrame,
				pAssocReq->HTCaps.lsigTXOPProtection);

			/* Make sure the STA's caps are compatible with our own: */
			/* 11.15.2 Support of DSSS/CCK in 40 MHz */
			/* the AP shall refuse association requests from an HT STA that has the DSSS/CCK */
			/* Mode in 40 MHz subfield set to 1; */
		}
	} /* End if on HT caps turned on in lim. */

	/* Clear the buffers so that frame parser knows that there isn't a previously decoded IE in these buffers */
	cdf_mem_set((uint8_t *) &Dot11fIERSN, sizeof(Dot11fIERSN), 0);
	cdf_mem_set((uint8_t *) &Dot11fIEWPA, sizeof(Dot11fIEWPA), 0);

	/* if additional IE is present, check if it has WscIE */
	if (pAssocReq->addIEPresent && pAssocReq->addIE.length)
		wpsIe =
			limGetWscIEPtr(pMac, pAssocReq->addIE.addIEdata,
				       pAssocReq->addIE.length);
	else {
		lim_log(pMac, LOG1, FL("Assoc req addIEPresent = %d "
				       "addIE length = %d"),
			pAssocReq->addIEPresent, pAssocReq->addIE.length);
	}
	/* when wpsIe is present, RSN/WPA IE is ignored */
	if (wpsIe == NULL) {
		/** check whether as RSN IE is present */
		if (LIM_IS_AP_ROLE(psessionEntry) &&
		    psessionEntry->pLimStartBssReq->privacy &&
		    psessionEntry->pLimStartBssReq->rsnIE.length) {
			lim_log(pMac, LOG1,
				FL("RSN enabled auth, Re/Assoc req from STA: "
				   MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));
			if (pAssocReq->rsnPresent) {
				if (pAssocReq->rsn.length) {
					/* Unpack the RSN IE */
					dot11f_unpack_ie_rsn(pMac,
							     &pAssocReq->rsn.
							     info[0],
							     pAssocReq->rsn.length,
							     &Dot11fIERSN);

					/* Check RSN version is supported or not */
					if (SIR_MAC_OUI_VERSION_1 ==
					    Dot11fIERSN.version) {
						/* check the groupwise and pairwise cipher suites */
						if (eSIR_SUCCESS !=
						    (status =
							     lim_check_rx_rsn_ie_match(pMac,
										       Dot11fIERSN,
										       psessionEntry,
										       pAssocReq->
										       HTCaps.
										       present,
										       &pmfConnection)))
						{
							lim_log(pMac, LOGE,
								FL("RSN Mismatch."
									"Rejecting Re/Assoc req from "
									"STA: "
									MAC_ADDRESS_STR),
								MAC_ADDR_ARRAY
									(pHdr->sa));

							/* some IE is not properly sent */
							/* received Association req frame with RSN IE but length is 0 */
							lim_send_assoc_rsp_mgmt_frame
								(pMac, status, 1,
								pHdr->sa, subType,
								0, psessionEntry);

							goto error;

						}
					} else {
						lim_log(pMac, LOGE,
							FL("RSN length not correct."
								"Rejecting Re/Assoc req from "
								"STA: "
								MAC_ADDRESS_STR),
							MAC_ADDR_ARRAY(pHdr->
								       sa));

						/* received Association req frame with RSN IE version wrong */
						lim_send_assoc_rsp_mgmt_frame(pMac,
									      eSIR_MAC_UNSUPPORTED_RSN_IE_VERSION_STATUS,
									      1,
									      pHdr->
									      sa,
									      subType,
									      0,
									      psessionEntry);
						goto error;

					}
				} else {
					lim_log(pMac, LOGW,
						FL
							("Rejecting Re/Assoc req from STA:"
							MAC_ADDRESS_STR),
						MAC_ADDR_ARRAY(pHdr->sa));
					/* received Association req frame with RSN IE but length is 0 */
					lim_send_assoc_rsp_mgmt_frame(pMac,
								      eSIR_MAC_INVALID_INFORMATION_ELEMENT_STATUS,
								      1,
								      pHdr->sa,
								      subType, 0,
								      psessionEntry);

					goto error;

				}
			} /* end - if(pAssocReq->rsnPresent) */
			if ((!pAssocReq->rsnPresent) && pAssocReq->wpaPresent) {
				/* Unpack the WPA IE */
				if (pAssocReq->wpa.length) {
					dot11f_unpack_ie_wpa(pMac, &pAssocReq->wpa.info[4],        /* OUI is not taken care */
							     pAssocReq->wpa.length,
							     &Dot11fIEWPA);
					/* check the groupwise and pairwise cipher suites */
					if (eSIR_SUCCESS !=
					    (status =
						     lim_check_rx_wpa_ie_match(pMac,
									       Dot11fIEWPA,
									       psessionEntry,
									       pAssocReq->
									       HTCaps.
									       present))) {
						lim_log(pMac, LOGW,
							FL("WPA IE mismatch"
								"Rejecting Re/Assoc req from "
								"STA: "
								MAC_ADDRESS_STR),
							MAC_ADDR_ARRAY(pHdr->
								       sa));
						/* received Association req frame with WPA IE but mismatch */
						lim_send_assoc_rsp_mgmt_frame(pMac,
									      status,
									      1,
									      pHdr->
									      sa,
									      subType,
									      0,
									      psessionEntry);
						goto error;

					}
				} else {
					lim_log(pMac, LOGW,
						FL("WPA len incorrect."
						"Rejecting Re/Assoc req from"
						"STA: "MAC_ADDRESS_STR),
						MAC_ADDR_ARRAY(pHdr->sa));
					/* received Association req frame with invalid WPA IE */
					lim_send_assoc_rsp_mgmt_frame(pMac,
								      eSIR_MAC_INVALID_INFORMATION_ELEMENT_STATUS,
								      1,
								      pHdr->sa,
								      subType, 0,
								      psessionEntry);

					goto error;
				} /* end - if(pAssocReq->wpa.length) */
			} /* end - if(pAssocReq->wpaPresent) */
		}
		/* end of if(psessionEntry->pLimStartBssReq->privacy
		   && psessionEntry->pLimStartBssReq->rsnIE->length) */
	} /* end of     if( ! pAssocReq->wscInfo.present ) */
	else {
		lim_log(pMac, LOG1, FL("Assoc req WSE IE is present"));
	}

	/**
	 * Extract 'associated' context for STA, if any.
	 * This is maintained by DPH and created by LIM.
	 */
	pStaDs =
		dph_lookup_hash_entry(pMac, pHdr->sa, &peerIdx,
				      &psessionEntry->dph.dphHashTable);

	/* / Extract pre-auth context for the STA, if any. */
	pStaPreAuthContext = lim_search_pre_auth_list(pMac, pHdr->sa);

	if (pStaDs == NULL) {
		/* / Requesting STA is not currently associated */
		if (pe_get_current_stas_count(pMac) ==
				pMac->lim.gLimAssocStaLimit) {
			/**
			 * Maximum number of STAs that AP can handle reached.
			 * Send Association response to peer MAC entity
			 */
			lim_log(pMac, LOGE, FL("Max Sta count reached : %d"),
					pMac->lim.maxStation);
			lim_reject_association(pMac, pHdr->sa,
					       subType, false,
					       (tAniAuthType) 0, 0,
					       false,
					       (tSirResultCodes)
					       eSIR_MAC_UNSPEC_FAILURE_STATUS,
					       psessionEntry);

			goto error;
		}
		/* / Check if STA is pre-authenticated. */
		if ((pStaPreAuthContext == NULL) ||
		    (pStaPreAuthContext &&
		     (pStaPreAuthContext->mlmState !=
		      eLIM_MLM_AUTHENTICATED_STATE))) {
			/**
			 * STA is not pre-authenticated yet requesting
			 * Re/Association before Authentication.
			 * OR STA is in the process of getting authenticated
			 * and sent Re/Association request.
			 * Send Deauthentication frame with 'prior
			 * authentication required' reason code.
			 */
			lim_send_deauth_mgmt_frame(pMac, eSIR_MAC_STA_NOT_PRE_AUTHENTICATED_REASON, /* =9 */
						   pHdr->sa, psessionEntry, false);

			lim_log(pMac, LOGE,
				FL("received %s req on sessionid: %d from STA "
				   "that does not have pre-auth context"
				   MAC_ADDRESS_STR),
				(LIM_ASSOC == subType) ? "Assoc" : "ReAssoc",
				psessionEntry->peSessionId,
				MAC_ADDR_ARRAY(pHdr->sa));
			goto error;
		}
		/* / Delete 'pre-auth' context of STA */
		authType = pStaPreAuthContext->authType;
		lim_delete_pre_auth_node(pMac, pHdr->sa);

		/* All is well. Assign AID (after else part) */

	} /* if (pStaDs == NULL) */
	else {
		/* STA context does exist for this STA */

		if (pStaDs->mlmStaContext.mlmState !=
		    eLIM_MLM_LINK_ESTABLISHED_STATE) {
			/**
			 * Requesting STA is in some 'transient' state?
			 * Ignore the Re/Assoc Req frame by incrementing
			 * debug counter & logging error.
			 */
			if (subType == LIM_ASSOC) {

#ifdef WLAN_DEBUG
				pMac->lim.gLimNumAssocReqDropInvldState++;
#endif
				lim_log(pMac, LOGE,
					FL("received Assoc req in state "
					   "%X from "),
					pStaDs->mlmStaContext.mlmState);
			} else {
#ifdef WLAN_DEBUG
				pMac->lim.gLimNumReassocReqDropInvldState++;
#endif
				lim_log(pMac, LOGE,
					FL("received ReAssoc req in state %X"
					   " from "),
					pStaDs->mlmStaContext.mlmState);
			}
			lim_print_mac_addr(pMac, pHdr->sa, LOG1);
			lim_print_mlm_state(pMac, LOG1,
					    (tLimMlmStates) pStaDs->mlmStaContext.
					    mlmState);

			goto error;
		} /* if (pStaDs->mlmStaContext.mlmState != eLIM_MLM_LINK_ESTABLISHED_STATE) */

		/* STA sent association Request frame while already in
		 * 'associated' state */

#ifdef WLAN_FEATURE_11W
		lim_log(pMac, LOG1,
			FL
				("Re/Assoc request from station that is already associated"));
		lim_log(pMac, LOG1, FL("PMF enabled %d, SA Query state %d"),
			pStaDs->rmfEnabled, pStaDs->pmfSaQueryState);
		if (pStaDs->rmfEnabled) {
			switch (pStaDs->pmfSaQueryState) {

			/* start SA Query procedure, respond to Association Request */
			/* with try again later */
			case DPH_SA_QUERY_NOT_IN_PROGRESS:
				/*
				 * We should reset the retry counter before we start
				 * the SA query procedure, otherwise in next set of SA query
				 * procedure we will end up using the stale value.
				 */
				pStaDs->pmfSaQueryRetryCount = 0;
				lim_send_assoc_rsp_mgmt_frame(pMac,
							      eSIR_MAC_TRY_AGAIN_LATER,
							      1, pHdr->sa, subType,
							      pStaDs, psessionEntry);
				lim_send_sa_query_request_frame(pMac,
								(uint8_t *) &
								(pStaDs->
								 pmfSaQueryCurrentTransId),
								pHdr->sa,
								psessionEntry);
				pStaDs->pmfSaQueryStartTransId =
					pStaDs->pmfSaQueryCurrentTransId;
				pStaDs->pmfSaQueryCurrentTransId++;

				/* start timer for SA Query retry */
				if (tx_timer_activate(&pStaDs->pmfSaQueryTimer)
				    != TX_SUCCESS) {
					lim_log(pMac, LOGE,
						FL
							("PMF SA Query timer activation failed!"));
					goto error;
				}

				pStaDs->pmfSaQueryState =
					DPH_SA_QUERY_IN_PROGRESS;
				goto error;

			/* SA Query procedure still going, respond to Association */
			/* Request with try again later */
			case DPH_SA_QUERY_IN_PROGRESS:
				lim_send_assoc_rsp_mgmt_frame(pMac,
							      eSIR_MAC_TRY_AGAIN_LATER,
							      1, pHdr->sa, subType,
							      0, psessionEntry);
				goto error;

			/* SA Query procedure timed out, accept Association Request */
			/* normally */
			case DPH_SA_QUERY_TIMED_OUT:
				pStaDs->pmfSaQueryState =
					DPH_SA_QUERY_NOT_IN_PROGRESS;
				break;
			}
		}
#endif

		/* no change in the capability so drop the frame */
		if ((true ==
		     cdf_mem_compare(&pStaDs->mlmStaContext.capabilityInfo,
				     &pAssocReq->capabilityInfo,
				     sizeof(tSirMacCapabilityInfo)))
		    && (subType == LIM_ASSOC)) {
			lim_log(pMac, LOGE,
				FL(" Received Assoc req in state %X STAid=%d"),
				pStaDs->mlmStaContext.mlmState, peerIdx);
			goto error;
		} else {
			/**
			 * STA sent Re/association Request frame while already in
			 * 'associated' state. Update STA capabilities and
			 * send Association response frame with same AID
			 */
			lim_log(pMac, LOG1,
				FL("Recved Assoc req from STA already connected"
				" UpdateConext"));
			pStaDs->mlmStaContext.capabilityInfo =
				pAssocReq->capabilityInfo;
			if (pStaPreAuthContext
			    && (pStaPreAuthContext->mlmState ==
				eLIM_MLM_AUTHENTICATED_STATE)) {
				/* / STA has triggered pre-auth again */
				authType = pStaPreAuthContext->authType;
				lim_delete_pre_auth_node(pMac, pHdr->sa);
			} else
				authType = pStaDs->mlmStaContext.authType;

			updateContext = true;
			if (dph_init_sta_state
				    (pMac, pHdr->sa, peerIdx, true,
				    &psessionEntry->dph.dphHashTable)
			    == NULL) {
				lim_log(pMac, LOGE,
					FL("could not Init STAid=%d"), peerIdx);
				goto error;
			}
		}
		goto sendIndToSme;
	} /* end if (lookup for STA in perStaDs fails) */

	/* check if sta is allowed per QoS AC rules */
	limGetWmeMode(psessionEntry, &wmeMode);
	if ((qosMode == eHAL_SET) || (wmeMode == eHAL_SET)) {
		/* for a qsta, check if the requested Traffic spec */
		/* is admissible */
		/* for a non-qsta check if the sta can be admitted */
		if (pAssocReq->addtsPresent) {
			uint8_t tspecIdx = 0;   /* index in the sch tspec table. */
			if (lim_admit_control_add_ts
				    (pMac, pHdr->sa, &(pAssocReq->addtsReq),
				    &(pAssocReq->qosCapability), 0, false, NULL,
				    &tspecIdx, psessionEntry) != eSIR_SUCCESS) {
				lim_log(pMac, LOGE,
					FL("AdmitControl: TSPEC rejected"));
				lim_send_assoc_rsp_mgmt_frame(pMac,
							      eSIR_MAC_QAP_NO_BANDWIDTH_REASON,
							      1, pHdr->sa, subType,
							      0, psessionEntry);
#ifdef WLAN_DEBUG
				pMac->lim.gLimNumAssocReqDropACRejectTS++;
#endif
				goto error;
			}
		} else if (lim_admit_control_add_sta(pMac, pHdr->sa, false)
			   != eSIR_SUCCESS) {
			lim_log(pMac, LOGE, FL("AdmitControl: Sta rejected"));
			lim_send_assoc_rsp_mgmt_frame(pMac,
						      eSIR_MAC_QAP_NO_BANDWIDTH_REASON,
						      1,
						      pHdr->sa,
						      subType, 0, psessionEntry);
#ifdef WLAN_DEBUG
			pMac->lim.gLimNumAssocReqDropACRejectSta++;
#endif
			goto error;
		}
		/* else all ok */
		lim_log(pMac, LOG1, FL("AdmitControl: Sta OK!"));
	}

	/**
	 * STA is Associated !
	 */
	lim_log(pMac, LOGE,
		FL("Received %s Req  successful from " MAC_ADDRESS_STR),
		(LIM_ASSOC == subType) ? "Assoc" : "ReAssoc",
		MAC_ADDR_ARRAY(pHdr->sa));

	/**
	 * AID for this association will be same as the peer Index used in DPH table.
	 * Assign unused/least recently used peer Index from perStaDs.
	 * NOTE: lim_assign_peer_idx() assigns AID values ranging
	 * between 1 - cfg_item(WNI_CFG_ASSOC_STA_LIMIT)
	 */

	peerIdx = lim_assign_peer_idx(pMac, psessionEntry);

	if (!peerIdx) {
		/* Could not assign AID */
		/* Reject association */
		lim_log(pMac, LOGE,
			FL("PeerIdx not avaialble. Reject associaton"));
		lim_reject_association(pMac, pHdr->sa,
				       subType, true, authType,
				       peerIdx, false,
				       (tSirResultCodes)
				       eSIR_MAC_UNSPEC_FAILURE_STATUS,
				       psessionEntry);

		goto error;
	}

	/**
	 * Add an entry to hash table maintained by DPH module
	 */

	pStaDs =
		dph_add_hash_entry(pMac, pHdr->sa, peerIdx,
				   &psessionEntry->dph.dphHashTable);

	if (pStaDs == NULL) {
		/* Could not add hash table entry at DPH */
		lim_log(pMac, LOGE,
			FL("could not add hash entry at DPH for aid=%d, MacAddr:"
			   MAC_ADDRESS_STR), peerIdx, MAC_ADDR_ARRAY(pHdr->sa));

		/* Release AID */
		lim_release_peer_idx(pMac, peerIdx, psessionEntry);

		lim_reject_association(pMac, pHdr->sa,
				       subType, true, authType, peerIdx, false,
				       (tSirResultCodes)
				       eSIR_MAC_UNSPEC_FAILURE_STATUS,
				       psessionEntry);

		goto error;
	}

sendIndToSme:
	/*
	 * check here if the parsedAssocReq already
	 * pointing to the assoc_req and free it before
	 * assigning this new pAssocReq
	 */
	if (psessionEntry->parsedAssocReq != NULL) {
		pTempAssocReq = psessionEntry->parsedAssocReq[pStaDs->assocId];
		if (pTempAssocReq != NULL) {
			if (pTempAssocReq->assocReqFrame) {
				cdf_mem_free(pTempAssocReq->assocReqFrame);
				pTempAssocReq->assocReqFrame = NULL;
				pTempAssocReq->assocReqFrameLength = 0;
			}
			cdf_mem_free(pTempAssocReq);
			pTempAssocReq = NULL;
		}

		psessionEntry->parsedAssocReq[pStaDs->assocId] = pAssocReq;
		assoc_req_copied = true;
	}

	pStaDs->mlmStaContext.htCapability = pAssocReq->HTCaps.present;
#ifdef WLAN_FEATURE_11AC
	pStaDs->mlmStaContext.vhtCapability = pAssocReq->VHTCaps.present;
#endif
	pStaDs->qos.addtsPresent =
		(pAssocReq->addtsPresent == 0) ? false : true;
	pStaDs->qos.addts = pAssocReq->addtsReq;
	pStaDs->qos.capability = pAssocReq->qosCapability;
	pStaDs->versionPresent = 0;
	/* short slot and short preamble should be updated before doing limaddsta */
	pStaDs->shortPreambleEnabled =
		(uint8_t) pAssocReq->capabilityInfo.shortPreamble;
	pStaDs->shortSlotTimeEnabled =
		(uint8_t) pAssocReq->capabilityInfo.shortSlotTime;

	pStaDs->valid = 0;
	pStaDs->mlmStaContext.authType = authType;
	pStaDs->staType = STA_ENTRY_PEER;

	/* TODO: If listen interval is more than certain limit, reject the association. */
	/* Need to check customer requirements and then implement. */
	pStaDs->mlmStaContext.listenInterval = pAssocReq->listenInterval;
	pStaDs->mlmStaContext.capabilityInfo = pAssocReq->capabilityInfo;

	/* The following count will be used to knock-off the station if it doesn't
	 * come back to receive the buffered data. The AP will wait for numTimSent number
	 * of beacons after sending TIM information for the station, before assuming that
	 * the station is no more associated and disassociates it
	 */

	/** timWaitCount is used by PMM for monitoring the STA's in PS for LINK*/
	pStaDs->timWaitCount =
		(uint8_t) GET_TIM_WAIT_COUNT(pAssocReq->listenInterval);

	/** Initialise the Current successful MPDU's tranfered to this STA count as 0 */
	pStaDs->curTxMpduCnt = 0;

	if (IS_DOT11_MODE_HT(psessionEntry->dot11mode) &&
	    pAssocReq->HTCaps.present && pAssocReq->wmeInfoPresent) {
		pStaDs->htGreenfield = (uint8_t) pAssocReq->HTCaps.greenField;
		pStaDs->htAMpduDensity = pAssocReq->HTCaps.mpduDensity;
		pStaDs->htDsssCckRate40MHzSupport =
			(uint8_t) pAssocReq->HTCaps.dsssCckMode40MHz;
		pStaDs->htLsigTXOPProtection =
			(uint8_t) pAssocReq->HTCaps.lsigTXOPProtection;
		pStaDs->htMaxAmsduLength =
			(uint8_t) pAssocReq->HTCaps.maximalAMSDUsize;
		pStaDs->htMaxRxAMpduFactor = pAssocReq->HTCaps.maxRxAMPDUFactor;
		pStaDs->htMIMOPSState = pAssocReq->HTCaps.mimoPowerSave;

		/* pAssocReq will be copied to psessionEntry->parsedAssocReq
		 * later
		 */
		ht_cap_ie = ((uint8_t *) &pAssocReq->HTCaps) + 1;

		/* check whether AP is enabled with shortGI */
		if (wlan_cfg_get_int(pMac, WNI_CFG_SHORT_GI_20MHZ, &val) !=
						eSIR_SUCCESS) {
			PELOGE(lim_log(pMac, LOGE,
				FL("could not retrieve shortGI 20Mhz CFG"));)
			goto error;
		}
		if (val) {
			pStaDs->htShortGI20Mhz =
					(uint8_t)pAssocReq->HTCaps.shortGI20MHz;
		} else {
			/* Unset htShortGI20Mhz in ht_caps*/
			*ht_cap_ie &= ~(1 << SIR_MAC_HT_CAP_SHORTGI20MHZ_S);
			pStaDs->htShortGI20Mhz = 0;
		}

		if (wlan_cfg_get_int(pMac, WNI_CFG_SHORT_GI_40MHZ, &val) !=
						eSIR_SUCCESS) {
			PELOGE(lim_log(pMac, LOGE,
				FL("could not retrieve shortGI 40Mhz CFG"));)
			goto error;
		}
		if (val) {
			pStaDs->htShortGI40Mhz =
				(uint8_t)pAssocReq->HTCaps.shortGI40MHz;
		} else {
			/* Unset htShortGI40Mhz in ht_caps */
			*ht_cap_ie &= ~(1 << SIR_MAC_HT_CAP_SHORTGI40MHZ_S);
			pStaDs->htShortGI40Mhz = 0;
		}

		pStaDs->htSupportedChannelWidthSet =
			(uint8_t) pAssocReq->HTCaps.supportedChannelWidthSet;
		/*
		 * peer just follows AP; so when we are softAP/GO,
		 * we just store our session entry's secondary channel offset
		 * here in peer INFRA STA. However, if peer's 40MHz channel
		 * width support is disabled then secondary channel will be zero
		 */
		pStaDs->htSecondaryChannelOffset =
			(pStaDs->htSupportedChannelWidthSet) ? psessionEntry->
			htSecondaryChannelOffset : 0;
#ifdef WLAN_FEATURE_11AC
		if (pAssocReq->operMode.present) {
			pStaDs->vhtSupportedChannelWidthSet =
				(uint8_t) ((pAssocReq->operMode.chanWidth ==
					    eHT_CHANNEL_WIDTH_80MHZ) ?
					   WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ :
					   WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ);
			pStaDs->htSupportedChannelWidthSet =
				(uint8_t) (pAssocReq->operMode.
					   chanWidth ? eHT_CHANNEL_WIDTH_40MHZ :
					   eHT_CHANNEL_WIDTH_20MHZ);
		} else if (pAssocReq->VHTCaps.present) {
			/* Check if STA has enabled it's channel bonding mode. */
			/* If channel bonding mode is enabled, we decide based on SAP's current configuration. */
			/* else, we set it to VHT20. */
			pStaDs->vhtSupportedChannelWidthSet =
				(uint8_t) ((pStaDs->htSupportedChannelWidthSet ==
					    eHT_CHANNEL_WIDTH_20MHZ) ?
					   WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ :
					   psessionEntry->ch_width - 1);
			pStaDs->htMaxRxAMpduFactor =
				pAssocReq->VHTCaps.maxAMPDULenExp;
		}
		/* Lesser among the AP and STA bandwidth of operation. */
		pStaDs->htSupportedChannelWidthSet =
			(pStaDs->htSupportedChannelWidthSet <
			 psessionEntry->htSupportedChannelWidthSet) ?
				pStaDs->htSupportedChannelWidthSet :
				psessionEntry->htSupportedChannelWidthSet;

#endif
		pStaDs->baPolicyFlag = 0xFF;
		pStaDs->htLdpcCapable =
			(uint8_t) pAssocReq->HTCaps.advCodingCap;
	}

	if (pAssocReq->VHTCaps.present && pAssocReq->wmeInfoPresent) {
		pStaDs->vhtLdpcCapable =
			(uint8_t) pAssocReq->VHTCaps.ldpcCodingCap;
	}

	if (!pAssocReq->wmeInfoPresent) {
		pStaDs->mlmStaContext.htCapability = 0;
		pStaDs->mlmStaContext.vhtCapability = 0;
	}
	if (pStaDs->mlmStaContext.vhtCapability) {
		if (psessionEntry->txBFIniFeatureEnabled &&
				pAssocReq->VHTCaps.suBeamFormerCap)
			pStaDs->vhtBeamFormerCapable = 1;
		else
			pStaDs->vhtBeamFormerCapable = 0;
		if (psessionEntry->enable_su_tx_bformer &&
				pAssocReq->VHTCaps.suBeamformeeCap)
			pStaDs->vht_su_bfee_capable = 1;
		else
			pStaDs->vht_su_bfee_capable = 0;
	}
#ifdef WLAN_FEATURE_11AC
	if (lim_populate_matching_rate_set(pMac,
					   pStaDs,
					   &(pAssocReq->supportedRates),
					   &(pAssocReq->extendedRates),
					   pAssocReq->HTCaps.supportedMCSSet,
					   psessionEntry, &pAssocReq->VHTCaps)
	    != eSIR_SUCCESS)
#else

	if (lim_populate_matching_rate_set(pMac,
					   pStaDs,
					   &(pAssocReq->supportedRates),
					   &(pAssocReq->extendedRates),
					   pAssocReq->HTCaps.supportedMCSSet,
					   psessionEntry) != eSIR_SUCCESS)
#endif
	{
		/* Could not update hash table entry at DPH with rateset */
		lim_log(pMac, LOGE,
			FL
				("could not update hash entry at DPH for aid=%d, MacAddr: "
				MAC_ADDRESS_STR), peerIdx, MAC_ADDR_ARRAY(pHdr->sa));

		/* Release AID */
		lim_release_peer_idx(pMac, peerIdx, psessionEntry);

		lim_reject_association(pMac, pHdr->sa,
				       subType, true, authType, peerIdx, true,
				       (tSirResultCodes)
				       eSIR_MAC_UNSPEC_FAILURE_STATUS,
				       psessionEntry);

		if (psessionEntry->parsedAssocReq)
			pAssocReq =
				psessionEntry->parsedAssocReq[pStaDs->assocId];
		goto error;
	}
#ifdef WLAN_FEATURE_11AC
	if (pAssocReq->operMode.present) {
		pStaDs->vhtSupportedRxNss = pAssocReq->operMode.rxNSS + 1;
	} else {
		pStaDs->vhtSupportedRxNss =
			((pStaDs->supportedRates.vhtRxMCSMap & MCSMAPMASK2x2)
			 == MCSMAPMASK2x2) ? 1 : 2;
	}
#endif

	/* / Add STA context at MAC HW (BMU, RHP & TFP) */

	pStaDs->qosMode = false;
	pStaDs->lleEnabled = false;
	if (pAssocReq->capabilityInfo.qos && (qosMode == eHAL_SET)) {
		pStaDs->lleEnabled = true;
		pStaDs->qosMode = true;
	}

	pStaDs->wmeEnabled = false;
	pStaDs->wsmEnabled = false;
	limGetWmeMode(psessionEntry, &wmeMode);
	if ((!pStaDs->lleEnabled) && pAssocReq->wmeInfoPresent
	    && (wmeMode == eHAL_SET)) {
		pStaDs->wmeEnabled = true;
		pStaDs->qosMode = true;
		limGetWsmMode(psessionEntry, &wsmMode);
		/* WMM_APSD - WMM_SA related processing should be separate; WMM_SA and WMM_APSD
		   can coexist */
		if (pAssocReq->WMMInfoStation.present) {
			/* check whether AP supports or not */
			if (LIM_IS_AP_ROLE(psessionEntry) &&
			    (psessionEntry->apUapsdEnable == 0) &&
			    (pAssocReq->WMMInfoStation.acbe_uapsd ||
			    pAssocReq->WMMInfoStation.acbk_uapsd ||
			    pAssocReq->WMMInfoStation.acvo_uapsd ||
			    pAssocReq->WMMInfoStation.acvi_uapsd)) {

				/**
				 * Received Re/Association Request from
				 * STA when UPASD is not supported.
				 */
				lim_log(pMac, LOGE,
					FL("AP do not support UAPSD so reply "
					   "to STA accordingly"));
				/* update UAPSD and send it to LIM to add STA */
				pStaDs->qos.capability.qosInfo.acbe_uapsd = 0;
				pStaDs->qos.capability.qosInfo.acbk_uapsd = 0;
				pStaDs->qos.capability.qosInfo.acvo_uapsd = 0;
				pStaDs->qos.capability.qosInfo.acvi_uapsd = 0;
				pStaDs->qos.capability.qosInfo.maxSpLen = 0;

			} else {
				/* update UAPSD and send it to LIM to add STA */
				pStaDs->qos.capability.qosInfo.acbe_uapsd =
					pAssocReq->WMMInfoStation.acbe_uapsd;
				pStaDs->qos.capability.qosInfo.acbk_uapsd =
					pAssocReq->WMMInfoStation.acbk_uapsd;
				pStaDs->qos.capability.qosInfo.acvo_uapsd =
					pAssocReq->WMMInfoStation.acvo_uapsd;
				pStaDs->qos.capability.qosInfo.acvi_uapsd =
					pAssocReq->WMMInfoStation.acvi_uapsd;
				pStaDs->qos.capability.qosInfo.maxSpLen =
					pAssocReq->WMMInfoStation.max_sp_length;
			}
		}
		if (pAssocReq->wsmCapablePresent && (wsmMode == eHAL_SET))
			pStaDs->wsmEnabled = true;

	}
	/* Re/Assoc Response frame to requesting STA */
	pStaDs->mlmStaContext.subType = subType;

#ifdef WLAN_FEATURE_11W
	pStaDs->rmfEnabled = (pmfConnection) ? 1 : 0;
	pStaDs->pmfSaQueryState = DPH_SA_QUERY_NOT_IN_PROGRESS;
	timerId.fields.sessionId = psessionEntry->peSessionId;
	timerId.fields.peerIdx = peerIdx;
	if (wlan_cfg_get_int(pMac, WNI_CFG_PMF_SA_QUERY_RETRY_INTERVAL,
			     &retryInterval) != eSIR_SUCCESS) {
		lim_log(pMac, LOGE,
			FL
				("Could not retrieve PMF SA Query retry interval value"));
		lim_reject_association(pMac, pHdr->sa, subType, true, authType,
				       peerIdx, false,
				       (tSirResultCodes)
				       eSIR_MAC_UNSPEC_FAILURE_STATUS,
				       psessionEntry);
		goto error;
	}
	if (WNI_CFG_PMF_SA_QUERY_RETRY_INTERVAL_STAMIN > retryInterval) {
		retryInterval = WNI_CFG_PMF_SA_QUERY_RETRY_INTERVAL_STADEF;
	}
	if (tx_timer_create(&pStaDs->pmfSaQueryTimer, "PMF SA Query timer",
			    lim_pmf_sa_query_timer_handler, timerId.value,
			    SYS_MS_TO_TICKS((retryInterval * 1024) / 1000),
			    0, TX_NO_ACTIVATE) != TX_SUCCESS) {
		lim_log(pMac, LOGE, FL("could not create PMF SA Query timer"));
		lim_reject_association(pMac, pHdr->sa,
				       subType, true, authType,
				       peerIdx, false,
				       (tSirResultCodes)
				       eSIR_MAC_UNSPEC_FAILURE_STATUS,
				       psessionEntry);
		goto error;
	}
#endif

	if (pAssocReq->ExtCap.present) {
		lim_set_stads_rtt_cap(pStaDs,
			(struct s_ext_cap *) pAssocReq->ExtCap.bytes, pMac);
	} else {
		pStaDs->timingMeasCap = 0;
		PELOG1(lim_log(pMac, LOG1, FL("ExtCap not present"));)
	}

	/* BTAMP: Storing the parsed assoc request in the psessionEntry array */
	if (psessionEntry->parsedAssocReq)
		psessionEntry->parsedAssocReq[pStaDs->assocId] = pAssocReq;
	assoc_req_copied = true;

	/* BTAMP: If STA context already exist (ie. updateContext = 1)
	 * for this STA, then we should delete the old one, and add
	 * the new STA. This is taken care of in the lim_del_sta() routine.
	 *
	 * Prior to BTAMP, we were setting this flag so that when
	 * PE receives SME_ASSOC_CNF, and if this flag is set, then
	 * PE shall delete the old station and then add. But now in
	 * BTAMP, we're directly adding station before waiting for
	 * SME_ASSOC_CNF, so we can do this now.
	 */
	if (!updateContext) {
		pStaDs->mlmStaContext.updateContext = 0;

		/* BTAMP: Add STA context at HW - issue WMA_ADD_STA_REQ to HAL */
		if (lim_add_sta(pMac, pStaDs, false, psessionEntry) !=
		    eSIR_SUCCESS) {
			lim_log(pMac, LOGE,
				FL("could not Add STA with assocId=%d"),
				pStaDs->assocId);
			lim_reject_association(pMac, pStaDs->staAddr,
					       pStaDs->mlmStaContext.subType,
					       true,
					       pStaDs->mlmStaContext.authType,
					       pStaDs->assocId, true,
					       (tSirResultCodes)
					       eSIR_MAC_UNSPEC_FAILURE_STATUS,
					       psessionEntry);

			if (psessionEntry->parsedAssocReq)
				pAssocReq = psessionEntry->parsedAssocReq[
							pStaDs->assocId];
			goto error;
		}
	} else {
		pStaDs->mlmStaContext.updateContext = 1;

		mlmPrevState = pStaDs->mlmStaContext.mlmState;

		/* As per the HAL/FW needs the reassoc req need not be calling lim_del_sta */
		if (subType != LIM_REASSOC) {
			/* we need to set the mlmState here in order differentiate in lim_del_sta. */
			pStaDs->mlmStaContext.mlmState =
				eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE;
			if (lim_del_sta(pMac, pStaDs, true, psessionEntry) !=
			    eSIR_SUCCESS) {
				lim_log(pMac, LOGE,
					FL
						("could not DEL STA with assocId=%d staId %d"),
					pStaDs->assocId, pStaDs->staIndex);
				lim_reject_association(pMac, pStaDs->staAddr,
						       pStaDs->mlmStaContext.
						       subType, true,
						       pStaDs->mlmStaContext.
						       authType, pStaDs->assocId,
						       true,
						       (tSirResultCodes)
						       eSIR_MAC_UNSPEC_FAILURE_STATUS,
						       psessionEntry);

				/* Restoring the state back. */
				pStaDs->mlmStaContext.mlmState = mlmPrevState;
				if (psessionEntry->parsedAssocReq)
					pAssocReq =
						psessionEntry->parsedAssocReq[
							pStaDs->assocId];
				goto error;
			}
		} else {
			/* mlmState is changed in lim_add_sta context */
			/* use the same AID, already allocated */
			if (lim_add_sta(pMac, pStaDs, false, psessionEntry) !=
			    eSIR_SUCCESS) {
				lim_log(pMac, LOGE,
					FL("Could not AddSta with assocId= %d"
					   "staId %d"),
					pStaDs->assocId, pStaDs->staIndex);
				lim_reject_association(pMac, pStaDs->staAddr,
						       pStaDs->mlmStaContext.
						       subType, true,
						       pStaDs->mlmStaContext.
						       authType, pStaDs->assocId,
						       true,
						       (tSirResultCodes)
						       eSIR_MAC_WME_REFUSED_STATUS,
						       psessionEntry);

				/* Restoring the state back. */
				pStaDs->mlmStaContext.mlmState = mlmPrevState;
				if (psessionEntry->parsedAssocReq)
					pAssocReq =
					    psessionEntry->parsedAssocReq[
							pStaDs->assocId];
				goto error;
			}

		}

	}

	/* AddSta is sucess here */
	if (LIM_IS_AP_ROLE(psessionEntry) &&
	    IS_DOT11_MODE_HT(psessionEntry->dot11mode) &&
	    pAssocReq->HTCaps.present && pAssocReq->wmeInfoPresent) {
		/** Update in the HAL Station Table for the Update of the Protection Mode */
		lim_post_sm_state_update(pMac, pStaDs->staIndex,
					 pStaDs->htMIMOPSState,
					 pStaDs->staAddr,
					 psessionEntry->smeSessionId);
	}

	return;

error:
	if (pAssocReq != NULL) {
		if (pAssocReq->assocReqFrame) {
			cdf_mem_free(pAssocReq->assocReqFrame);
			pAssocReq->assocReqFrame = NULL;
			pAssocReq->assocReqFrameLength = 0;
		}

		cdf_mem_free(pAssocReq);
		/* to avoid double free */
		if (assoc_req_copied && psessionEntry->parsedAssocReq)
			psessionEntry->parsedAssocReq[pStaDs->assocId] = NULL;
	}

	/* If it is not duplicate Assoc request then only make to Null */
	if ((pStaDs != NULL) &&
	    (pStaDs->mlmStaContext.mlmState != eLIM_MLM_WT_ADD_STA_RSP_STATE)) {
		if (psessionEntry->parsedAssocReq != NULL) {
			pTempAssocReq =
				psessionEntry->parsedAssocReq[pStaDs->assocId];
			if (pTempAssocReq != NULL) {
				if (pTempAssocReq->assocReqFrame) {
					cdf_mem_free(pTempAssocReq->
							assocReqFrame);
					pTempAssocReq->assocReqFrame = NULL;
					pTempAssocReq->assocReqFrameLength = 0;
				}
				cdf_mem_free(pTempAssocReq);
				psessionEntry->
					parsedAssocReq[pStaDs->assocId] = NULL;
			}
		}
	}

	return;

} /*** end lim_process_assoc_req_frame() ***/

#ifdef FEATURE_WLAN_WAPI
/**
 * lim_fill_assoc_ind_wapi_info()- Updates WAPI data in assoc indication
 * @mac_ctx: Global Mac context
 * @assoc_req: pointer to association request
 * @assoc_ind: Pointer to association indication
 * @wpsie: WPS IE
 *
 * This function updates WAPI meta data in association indication message
 * sent to SME.
 *
 * Return: None
 */
static void lim_fill_assoc_ind_wapi_info(tpAniSirGlobal mac_ctx,
	tpSirAssocReq assoc_req, tpLimMlmAssocInd assoc_ind,
	uint8_t *wpsie)
{
	if (assoc_req->wapiPresent && (NULL == wpsie)) {
		lim_log(mac_ctx, LOG2,
			FL("Received WAPI IE length in Assoc Req is %d"),
			assoc_req->wapi.length);
		assoc_ind->wapiIE.wapiIEdata[0] = SIR_MAC_WAPI_EID;
		assoc_ind->wapiIE.wapiIEdata[1] = assoc_req->wapi.length;
		cdf_mem_copy(&assoc_ind->wapiIE.wapiIEdata[2],
			assoc_req->wapi.info, assoc_req->wapi.length);
		assoc_ind->wapiIE.length =
			2 + assoc_req->wapi.length;
	}
	return;
}
#endif

/**
 * lim_fill_assoc_ind_vht_info() - Updates VHT information in assoc indication
 * @mac_ctx: Global Mac context
 * @assoc_req: pointer to association request
 * @session_entry: PE session entry
 * @assoc_ind: Pointer to association indication
 *
 * This function updates VHT information in association indication message
 * sent to SME.
 *
 * Return: None
 */
static void lim_fill_assoc_ind_vht_info(tpAniSirGlobal mac_ctx,
	tpPESession session_entry, tpSirAssocReq assoc_req,
	tpLimMlmAssocInd assoc_ind)
{
	uint8_t chan;

	if (session_entry->limRFBand == SIR_BAND_2_4_GHZ) {
		if (session_entry->vhtCapability
			&& assoc_req->VHTCaps.present)
			assoc_ind->chan_info.info = MODE_11AC_VHT20;
		else if (session_entry->htCapability
				&& assoc_req->HTCaps.present)
			assoc_ind->chan_info.info = MODE_11NG_HT20;
		else
			assoc_ind->chan_info.info = MODE_11G;
	} else {
		if (session_entry->vhtCapability
			&& assoc_req->VHTCaps.present) {
			if ((session_entry->ch_width > CH_WIDTH_40MHZ)
				&& assoc_req->HTCaps.supportedChannelWidthSet) {
				chan = session_entry->ch_center_freq_seg0;
				assoc_ind->chan_info.band_center_freq1 =
					cds_chan_to_freq(chan);
				assoc_ind->chan_info.info = MODE_11AC_VHT80;
			} else if ((session_entry->ch_width == CH_WIDTH_40MHZ)
				&& assoc_req->HTCaps.supportedChannelWidthSet) {
				assoc_ind->chan_info.info = MODE_11AC_VHT40;
				if (session_entry->htSecondaryChannelOffset ==
					PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
					assoc_ind->chan_info.band_center_freq1
						+= 10;
				else
					assoc_ind->chan_info.band_center_freq1
						-= 10;
			} else {
				assoc_ind->chan_info.info = MODE_11AC_VHT20;
			}
		} else if (session_entry->htCapability
				&& assoc_req->HTCaps.present) {
			if ((session_entry->ch_width == CH_WIDTH_40MHZ)
			&& assoc_req->HTCaps.supportedChannelWidthSet) {
				assoc_ind->chan_info.info = MODE_11NA_HT40;
				if (session_entry->htSecondaryChannelOffset ==
					PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
					assoc_ind->chan_info.band_center_freq1
						+= 10;
				else
					assoc_ind->chan_info.band_center_freq1
						-= 10;
			} else {
				assoc_ind->chan_info.info = MODE_11NA_HT20;
			}
		} else {
			assoc_ind->chan_info.info = MODE_11A;
		}
	}
	return;
}

/**
 * lim_send_mlm_assoc_ind() - Sends assoc indication to SME
 * @mac_ctx: Global Mac context
 * @sta_ds: Station DPH hash entry
 * @session_entry: PE session entry
 *
 * This function sends either LIM_MLM_ASSOC_IND
 * or LIM_MLM_REASSOC_IND to SME.
 *
 * Return: None
 */
void lim_send_mlm_assoc_ind(tpAniSirGlobal mac_ctx,
	tpDphHashNode sta_ds, tpPESession session_entry)
{
	tpLimMlmAssocInd assoc_ind = NULL;
	tpSirAssocReq assoc_req;
	uint16_t temp, rsn_len;
	uint32_t phy_mode;
	uint8_t sub_type;
	uint8_t *wpsie = NULL;
	uint32_t tmp;

	/* Get a copy of the already parsed Assoc Request */
	assoc_req =
		(tpSirAssocReq) session_entry->parsedAssocReq[sta_ds->assocId];

	/* Get the phy_mode */
	lim_get_phy_mode(mac_ctx, &phy_mode, session_entry);

	/* Determine if its Assoc or ReAssoc Request */
	if (assoc_req->reassocRequest == 1)
		sub_type = LIM_REASSOC;
	else
		sub_type = LIM_ASSOC;

	lim_log(mac_ctx, LOG1,
		FL("Sessionid %d ssid %s sub_type %d Associd %d staAddr "
		 MAC_ADDRESS_STR), session_entry->peSessionId,
		assoc_req->ssId.ssId, sub_type, sta_ds->assocId,
		MAC_ADDR_ARRAY(sta_ds->staAddr));

	if (sub_type == LIM_ASSOC || sub_type == LIM_REASSOC) {
		temp = sizeof(tLimMlmAssocInd);

		assoc_ind = cdf_mem_malloc(temp);
		if (NULL == assoc_ind) {
			lim_release_peer_idx(mac_ctx, sta_ds->assocId,
				session_entry);
			lim_log(mac_ctx, LOGP,
				FL("AllocateMemory failed for assoc_ind"));
			return;
		}
		cdf_mem_set(assoc_ind, temp, 0);
		cdf_mem_copy((uint8_t *) assoc_ind->peerMacAddr,
			(uint8_t *) sta_ds->staAddr, sizeof(tSirMacAddr));
		assoc_ind->aid = sta_ds->assocId;
		cdf_mem_copy((uint8_t *) &assoc_ind->ssId,
			(uint8_t *) &(assoc_req->ssId),
			assoc_req->ssId.length + 1);
		assoc_ind->sessionId = session_entry->peSessionId;
		assoc_ind->authType = sta_ds->mlmStaContext.authType;
		assoc_ind->capabilityInfo = assoc_req->capabilityInfo;

		/* Fill in RSN IE information */
		assoc_ind->rsnIE.length = 0;
		/* if WPS IE is present, ignore RSN IE */
		if (assoc_req->addIEPresent && assoc_req->addIE.length) {
			wpsie = limGetWscIEPtr(mac_ctx,
				assoc_req->addIE.addIEdata,
				assoc_req->addIE.length);
		}
		if (assoc_req->rsnPresent && (NULL == wpsie)) {
			lim_log(mac_ctx, LOG2, FL("Assoc Req RSN IE len = %d"),
				assoc_req->rsn.length);
			assoc_ind->rsnIE.length = 2 + assoc_req->rsn.length;
			assoc_ind->rsnIE.rsnIEdata[0] = SIR_MAC_RSN_EID;
			assoc_ind->rsnIE.rsnIEdata[1] =
				assoc_req->rsn.length;
			cdf_mem_copy(&assoc_ind->rsnIE.rsnIEdata[2],
				assoc_req->rsn.info,
				assoc_req->rsn.length);
		}
		/* Fill in 802.11h related info */
		if (assoc_req->powerCapabilityPresent
			&& assoc_req->supportedChannelsPresent) {
			assoc_ind->spectrumMgtIndicator = eSIR_TRUE;
			assoc_ind->powerCap.minTxPower =
				assoc_req->powerCapability.minTxPower;
			assoc_ind->powerCap.maxTxPower =
				assoc_req->powerCapability.maxTxPower;
			lim_convert_supported_channels(mac_ctx, assoc_ind,
				 assoc_req);
		} else {
			assoc_ind->spectrumMgtIndicator = eSIR_FALSE;
		}

		/* This check is to avoid extra Sec IEs present incase of WPS */
		if (assoc_req->wpaPresent && (NULL == wpsie)) {
			rsn_len = assoc_ind->rsnIE.length;
			if ((rsn_len + assoc_req->wpa.length)
				>= SIR_MAC_MAX_IE_LENGTH) {
				lim_log(mac_ctx, LOGE,
					FL("rsnIEdata index out of bounds %d"),
					rsn_len);
				cdf_mem_free(assoc_ind);
				return;
			}
			assoc_ind->rsnIE.rsnIEdata[rsn_len] =
				SIR_MAC_WPA_EID;
			assoc_ind->rsnIE.rsnIEdata[rsn_len + 1]
				= assoc_req->wpa.length;
			cdf_mem_copy(
				&assoc_ind->rsnIE.rsnIEdata[rsn_len + 2],
				assoc_req->wpa.info, assoc_req->wpa.length);
			assoc_ind->rsnIE.length += 2 + assoc_req->wpa.length;
		}
#ifdef FEATURE_WLAN_WAPI
		lim_fill_assoc_ind_wapi_info(mac_ctx, assoc_req, assoc_ind,
			wpsie);
#endif

		assoc_ind->addIE.length = 0;
		if (assoc_req->addIEPresent) {
			cdf_mem_copy(&assoc_ind->addIE.addIEdata,
				assoc_req->addIE.addIEdata,
				assoc_req->addIE.length);
			assoc_ind->addIE.length = assoc_req->addIE.length;
		}
		/*
		 * Add HT Capabilities into addIE for OBSS
		 * processing in hostapd
		 */
		if (assoc_req->HTCaps.present) {
			rsn_len = assoc_ind->addIE.length;
			if (assoc_ind->addIE.length + DOT11F_IE_HTCAPS_MIN_LEN
				+ 2 < SIR_MAC_MAX_IE_LENGTH) {
				assoc_ind->addIE.addIEdata[rsn_len] =
					SIR_MAC_HT_CAPABILITIES_EID;
				assoc_ind->addIE.addIEdata[rsn_len + 1] =
					DOT11F_IE_HTCAPS_MIN_LEN;
				cdf_mem_copy(
					&assoc_ind->addIE.addIEdata[rsn_len+2],
					((uint8_t *)&assoc_req->HTCaps) + 1,
					DOT11F_IE_HTCAPS_MIN_LEN);
				assoc_ind->addIE.length +=
					2 + DOT11F_IE_HTCAPS_MIN_LEN;
			} else {
				lim_log(mac_ctx, LOGP,
					FL("Fail:HT capabilities IE to addIE"));
			}
		}

		if (assoc_req->wmeInfoPresent) {
			if (wlan_cfg_get_int (mac_ctx,
				(uint16_t) WNI_CFG_WME_ENABLED, &tmp)
				!= eSIR_SUCCESS)
				lim_log(mac_ctx, LOGP,
					FL("wlan_cfg_get_int failed for id %d"),
					WNI_CFG_WME_ENABLED);

			/* check whether AP is enabled with WMM */
			if (tmp)
				assoc_ind->WmmStaInfoPresent = 1;
			else
				assoc_ind->WmmStaInfoPresent = 0;
			/*
			 * Note: we are not rejecting association here
			 * because IOT will fail
			 */
		}
		/* Required for indicating the frames to upper layer */
		assoc_ind->assocReqLength = assoc_req->assocReqFrameLength;
		assoc_ind->assocReqPtr = assoc_req->assocReqFrame;

		assoc_ind->beaconPtr = session_entry->beacon;
		assoc_ind->beaconLength = session_entry->bcnLen;

		assoc_ind->chan_info.chan_id =
			session_entry->currentOperChannel;
		assoc_ind->chan_info.mhz =
			cds_chan_to_freq(session_entry->currentOperChannel);
		assoc_ind->chan_info.band_center_freq1 =
			cds_chan_to_freq(session_entry->currentOperChannel);
		assoc_ind->chan_info.band_center_freq2 = 0;
		assoc_ind->chan_info.reg_info_1 =
			(session_entry->maxTxPower << 16);
		assoc_ind->chan_info.reg_info_2 =
			(session_entry->maxTxPower << 8);
		/* updates VHT information in assoc indication */
		lim_fill_assoc_ind_vht_info(mac_ctx, session_entry, assoc_req,
			assoc_ind);
		lim_post_sme_message(mac_ctx, LIM_MLM_ASSOC_IND,
			 (uint32_t *) assoc_ind);
		cdf_mem_free(assoc_ind);
	}
	return;
}
