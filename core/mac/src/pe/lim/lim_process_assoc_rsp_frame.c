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

/*
 *
 * This file lim_process_assoc_rsp_frame.cc contains the code
 * for processing Re/Association Response Frame.
 * Author:        Chandra Modumudi
 * Date:          03/18/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "wni_api.h"
#include "wni_cfg.h"
#include "ani_global.h"
#include "cfg_api.h"
#include "sch_api.h"

#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_security_utils.h"
#include "lim_ser_des_utils.h"
#include "lim_sta_hash_api.h"
#include "lim_send_messages.h"


extern tSirRetStatus sch_beacon_edca_process(tpAniSirGlobal pMac,
	tSirMacEdcaParamSetIE *edca, tpPESession psessionEntry);

/**
 * lim_update_stads_htcap() - Updates station Descriptor HT capability
 * @mac_ctx: Pointer to Global MAC structure
 * @sta_ds: Station Descriptor in DPH
 * @assoc_rsp: Pointer to Association Response Structure
 * @session_entry : PE session Entry
 *
 * This function is called to Update the HT capabilities in
 * Station Descriptor (dph) Details from
 * Association / ReAssociation Response Frame
 *
 * Return: None
 */
static void lim_update_stads_htcap(tpAniSirGlobal mac_ctx,
		tpDphHashNode sta_ds, tpSirAssocRsp assoc_rsp,
		tpPESession session_entry)
{
	uint16_t highest_rxrate = 0;
	tDot11fIEHTCaps *ht_caps;
	uint32_t shortgi_20mhz_support;
	uint32_t shortgi_40mhz_support;

	ht_caps = &assoc_rsp->HTCaps;
	sta_ds->mlmStaContext.htCapability = assoc_rsp->HTCaps.present;
	if (assoc_rsp->HTCaps.present) {
		sta_ds->htGreenfield =
			(uint8_t) ht_caps->greenField;
		if (session_entry->htSupportedChannelWidthSet) {
			sta_ds->htSupportedChannelWidthSet =
				(uint8_t) (ht_caps->supportedChannelWidthSet ?
				assoc_rsp->HTInfo.recommendedTxWidthSet :
				ht_caps->supportedChannelWidthSet);
		} else
			sta_ds->htSupportedChannelWidthSet =
				eHT_CHANNEL_WIDTH_20MHZ;
		sta_ds->htLsigTXOPProtection =
			(uint8_t) ht_caps->lsigTXOPProtection;
		sta_ds->htMIMOPSState =
			(tSirMacHTMIMOPowerSaveState)ht_caps->mimoPowerSave;
		sta_ds->htMaxAmsduLength =
			(uint8_t) ht_caps->maximalAMSDUsize;
		sta_ds->htAMpduDensity = ht_caps->mpduDensity;
		sta_ds->htDsssCckRate40MHzSupport =
			(uint8_t) ht_caps->dsssCckMode40MHz;
		sta_ds->htMaxRxAMpduFactor =
			ht_caps->maxRxAMPDUFactor;
		lim_fill_rx_highest_supported_rate(mac_ctx, &highest_rxrate,
			ht_caps->supportedMCSSet);
		sta_ds->supportedRates.rxHighestDataRate =
			highest_rxrate;
		/*
		 * This is for AP as peer STA and we are INFRA STA
		 *.We will put APs offset in dph node which is peer STA
		 */
		sta_ds->htSecondaryChannelOffset =
			(uint8_t) assoc_rsp->HTInfo.secondaryChannelOffset;
		/*
		 * FIXME_AMPDU
		 * In the future, may need to check for
		 * "assoc.HTCaps.delayedBA"
		 * For now, it is IMMEDIATE BA only on ALL TID's
		 */
		sta_ds->baPolicyFlag = 0xFF;

		/* Check if we have support for gShortGI20Mhz and
		 * gShortGI40Mhz from ini file
		 */
		if (eSIR_SUCCESS == wlan_cfg_get_int(mac_ctx,
						WNI_CFG_SHORT_GI_20MHZ,
						&shortgi_20mhz_support)) {
			if (true == shortgi_20mhz_support)
				sta_ds->htShortGI20Mhz =
				      (uint8_t)assoc_rsp->HTCaps.shortGI20MHz;
			else
				sta_ds->htShortGI20Mhz = false;
		} else {
			lim_log(mac_ctx, LOGE,
				FL("could not retrieve shortGI 20Mhz CFG, setting value to default"));
			sta_ds->htShortGI20Mhz =
				WNI_CFG_SHORT_GI_20MHZ_STADEF;
		}

		if (eSIR_SUCCESS == wlan_cfg_get_int(mac_ctx,
						WNI_CFG_SHORT_GI_40MHZ,
						&shortgi_40mhz_support)) {
			if (true == shortgi_40mhz_support)
				sta_ds->htShortGI40Mhz =
				      (uint8_t)assoc_rsp->HTCaps.shortGI40MHz;
			else
				sta_ds->htShortGI40Mhz = false;
		} else {
			lim_log(mac_ctx, LOGE,
				FL("could not retrieve shortGI 40Mhz CFG,setting value to default"));
			sta_ds->htShortGI40Mhz =
				WNI_CFG_SHORT_GI_40MHZ_STADEF;
		}
	}
}

/**
 * lim_update_assoc_sta_datas() - Updates station Descriptor
 * mac_ctx: Pointer to Global MAC structure
 * sta_ds: Station Descriptor in DPH
 * assoc_rsp: Pointer to Association Response Structure
 * session_entry : PE session Entry
 *
 * This function is called to Update the Station Descriptor (dph) Details from
 * Association / ReAssociation Response Frame
 *
 * Return: None
 */
void lim_update_assoc_sta_datas(tpAniSirGlobal mac_ctx,
	tpDphHashNode sta_ds, tpSirAssocRsp assoc_rsp,
	tpPESession session_entry)
{
	uint32_t phy_mode;
	bool qos_mode;
	tDot11fIEVHTCaps *vht_caps = NULL;

	lim_get_phy_mode(mac_ctx, &phy_mode, session_entry);
	sta_ds->staType = STA_ENTRY_SELF;
	limGetQosMode(session_entry, &qos_mode);
	sta_ds->mlmStaContext.authType = session_entry->limCurrentAuthType;

	/* Add capabilities information, rates and AID */
	sta_ds->mlmStaContext.capabilityInfo = assoc_rsp->capabilityInfo;
	sta_ds->shortPreambleEnabled =
		(uint8_t) assoc_rsp->capabilityInfo.shortPreamble;

	/* Update HT Capabilites only when the self mode supports HT */
	if (IS_DOT11_MODE_HT(session_entry->dot11mode))
		lim_update_stads_htcap(mac_ctx, sta_ds, assoc_rsp,
				       session_entry);

#ifdef WLAN_FEATURE_11AC
	if (assoc_rsp->VHTCaps.present)
		vht_caps = &assoc_rsp->VHTCaps;
	else if (assoc_rsp->vendor2_ie.VHTCaps.present)
		vht_caps = &assoc_rsp->vendor2_ie.VHTCaps;

	if (IS_DOT11_MODE_VHT(session_entry->dot11mode)) {
		if ((vht_caps != NULL) && vht_caps->present) {
			sta_ds->mlmStaContext.vhtCapability =
				vht_caps->present;
			/*
			 * If 11ac is supported and if the peer is
			 * sending VHT capabilities,
			 * then htMaxRxAMpduFactor should be
			 * overloaded with VHT maxAMPDULenExp
			 */
			sta_ds->htMaxRxAMpduFactor = vht_caps->maxAMPDULenExp;
			if (session_entry->htSupportedChannelWidthSet) {
				if (assoc_rsp->VHTOperation.present)
					sta_ds->vhtSupportedChannelWidthSet =
					assoc_rsp->VHTOperation.chanWidth;
				else
					sta_ds->vhtSupportedChannelWidthSet =
						eHT_CHANNEL_WIDTH_40MHZ;
			}
		}
	}
	if (lim_populate_peer_rate_set(mac_ctx, &sta_ds->supportedRates,
				assoc_rsp->HTCaps.supportedMCSSet,
				false, session_entry,
				vht_caps) != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGP,
			FL("could not get rateset and extended rate set"));
		return;
	}
#else
	if (lim_populate_peer_rate_set(mac_ctx, &sta_ds->supportedRates,
		assoc_rsp->HTCaps.supportedMCSSet,
		false, session_entry) != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGP,
			FL("could not get rateset and extended rate set"));
		return;
	}
#endif
#ifdef WLAN_FEATURE_11AC
	sta_ds->vhtSupportedRxNss =
		((sta_ds->supportedRates.vhtRxMCSMap & MCSMAPMASK2x2)
		 == MCSMAPMASK2x2) ? 1 : 2;
#endif
	/* If one of the rates is 11g rates, set the ERP mode. */
	if ((phy_mode == WNI_CFG_PHY_MODE_11G) &&
		sirIsArate(sta_ds->supportedRates.llaRates[0] & 0x7f))
		sta_ds->erpEnabled = eHAL_SET;

	/* Could not get prop rateset from CFG. Log error. */
	sta_ds->qosMode = 0;
	sta_ds->lleEnabled = 0;

	/* update TSID to UP mapping */
	if (qos_mode) {
		if (assoc_rsp->edcaPresent) {
			tSirRetStatus status;
			status =
				sch_beacon_edca_process(mac_ctx,
					&assoc_rsp->edca, session_entry);
			lim_log(mac_ctx, LOG2,
				"Edca set update based on AssocRsp: status %d",
				status);
			if (status != eSIR_SUCCESS) {
				lim_log(mac_ctx, LOGE,
					FL("Edca error in AssocResp "));
			} else {
				/* update default tidmap based on ACM */
				sta_ds->qosMode = 1;
				sta_ds->lleEnabled = 1;
			}
		}
	}

	sta_ds->wmeEnabled = 0;
	sta_ds->wsmEnabled = 0;
	if (session_entry->limWmeEnabled && assoc_rsp->wmeEdcaPresent) {
		tSirRetStatus status;
		status = sch_beacon_edca_process(mac_ctx, &assoc_rsp->edca,
				session_entry);
		lim_log(mac_ctx, LOGW,
			"WME Edca set update based on AssocRsp: status %d",
			status);

		if (status != eSIR_SUCCESS)
			lim_log(mac_ctx, LOGE,
				FL("WME Edca error in AssocResp - ignoring"));

			else {
				/* update default tidmap based on HashACM */
				sta_ds->qosMode = 1;
				sta_ds->wmeEnabled = 1;
			}
	} else {
		/*
		 * We received assoc rsp from a legacy AP.
		 * So fill in the default  local EDCA params.
		 * This is needed (refer to bug #14989) as we'll
		 * be passing the gLimEdcaParams to HAL in
		 * lim_process_sta_mlm_add_bss_rsp().
		 */
		sch_set_default_edca_params(mac_ctx, session_entry);
	}

	if (qos_mode && (!sta_ds->qosMode) &&
		 sta_ds->mlmStaContext.htCapability) {
		/*
		 * Enable QOS for all HT AP's even though WMM
		 * or 802.11E IE is not present
		 */
		sta_ds->qosMode = 1;
		sta_ds->wmeEnabled = 1;
	}
#ifdef WLAN_FEATURE_11W
	if (session_entry->limRmfEnabled)
		sta_ds->rmfEnabled = 1;
#endif
}

/**
 * @function : lim_update_re_assoc_globals
 *
 * @brief :  This function is called to Update the Globals (LIM) during ReAssoc.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac    - Pointer to Global MAC structure
 * @param  pAssocRsp    - Pointer to Association Response Structure
 *
 * @return None
 */

void lim_update_re_assoc_globals(tpAniSirGlobal pMac, tpSirAssocRsp pAssocRsp,
				 tpPESession psessionEntry)
{
	/* Update the current Bss Information */
	cdf_mem_copy(psessionEntry->bssId,
		     psessionEntry->limReAssocbssId, sizeof(tSirMacAddr));
	psessionEntry->currentOperChannel = psessionEntry->limReassocChannelId;
	psessionEntry->htSecondaryChannelOffset =
		psessionEntry->reAssocHtSupportedChannelWidthSet;
	psessionEntry->htRecommendedTxWidthSet =
		psessionEntry->reAssocHtRecommendedTxWidthSet;
	psessionEntry->htSecondaryChannelOffset =
		psessionEntry->reAssocHtSecondaryChannelOffset;
	psessionEntry->limCurrentBssCaps = psessionEntry->limReassocBssCaps;
	psessionEntry->limCurrentBssQosCaps =
		psessionEntry->limReassocBssQosCaps;
	psessionEntry->limCurrentBssPropCap =
		psessionEntry->limReassocBssPropCap;

	cdf_mem_copy((uint8_t *) &psessionEntry->ssId,
		     (uint8_t *) &psessionEntry->limReassocSSID,
		     psessionEntry->limReassocSSID.length + 1);

	/* Store assigned AID for TIM processing */
	psessionEntry->limAID = pAssocRsp->aid & 0x3FFF;
	/** Set the State Back to ReAssoc Rsp*/
	psessionEntry->limMlmState = eLIM_MLM_WT_REASSOC_RSP_STATE;
	MTRACE(mac_trace
		       (pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId,
		       psessionEntry->limMlmState));

}
#ifdef WLAN_FEATURE_VOWIFI_11R
/**
 * lim_update_ric_data() - update session with ric data
 * @mac_ctx: Pointer to Global MAC structure
 * @session_entry: PE session handle
 * @assoc_rsp:  pointer to assoc response
 *
 * This function is called by lim_process_assoc_rsp_frame() to
 * update PE session context with RIC data.
 *
 * Return: None
 */
static void lim_update_ric_data(tpAniSirGlobal mac_ctx,
	 tpPESession session_entry, tpSirAssocRsp assoc_rsp)
{
	if (session_entry->ricData != NULL) {
		cdf_mem_free(session_entry->ricData);
		session_entry->ricData = NULL;
	}
	if (assoc_rsp->ricPresent) {
		session_entry->RICDataLen =
			assoc_rsp->num_RICData * sizeof(tDot11fIERICDataDesc);
		if (session_entry->RICDataLen) {
			session_entry->ricData =
				cdf_mem_malloc(session_entry->RICDataLen);
			if (NULL == session_entry->ricData) {
				lim_log(mac_ctx, LOGE,
					FL("No memory for RIC data"));
				session_entry->RICDataLen = 0;
			} else {
				cdf_mem_copy(session_entry->ricData,
					&assoc_rsp->RICData[0],
					session_entry->RICDataLen);
			}
		} else {
			lim_log(mac_ctx, LOGE, FL("RIC data not present"));
		}
	} else {
		lim_log(mac_ctx, LOG1,
				FL("Ric is not present"));
		session_entry->RICDataLen = 0;
		session_entry->ricData = NULL;
	}
	return;
}
#endif

#ifdef FEATURE_WLAN_ESE

/**
 * lim_update_ese_tspec() - update session with Tspec info.
 * @mac_ctx: Pointer to Global MAC structure
 * @session_entry: PE session handle
 * @assoc_rsp:  pointer to assoc response
 *
 * This function is called by lim_process_assoc_rsp_frame() to
 * update PE session context with Tspec data.
 *
 * Return: None
 */
static void lim_update_ese_tspec(tpAniSirGlobal mac_ctx,
	 tpPESession session_entry, tpSirAssocRsp assoc_rsp)
{
	if (session_entry->tspecIes != NULL) {
		cdf_mem_free(session_entry->tspecIes);
		session_entry->tspecIes = NULL;
	}
	if (assoc_rsp->tspecPresent) {
		lim_log(mac_ctx, LOG1, FL("Tspec EID present in assoc rsp"));
		session_entry->tspecLen =
			assoc_rsp->num_tspecs * sizeof(tDot11fIEWMMTSPEC);
		if (session_entry->tspecLen) {
			session_entry->tspecIes =
				cdf_mem_malloc(session_entry->tspecLen);
			if (NULL == session_entry->tspecIes) {
				lim_log(mac_ctx, LOGE,
					FL("Tspec IE:Fail to allocate memory"));
				session_entry->tspecLen = 0;
			} else {
				cdf_mem_copy(session_entry->tspecIes,
						&assoc_rsp->TSPECInfo[0],
						session_entry->tspecLen);
			}
		} else {
			lim_log(mac_ctx, LOGE, FL("TSPEC has Zero length"));
		}
	} else {
		session_entry->tspecLen = 0;
		session_entry->tspecIes = NULL;
		lim_log(mac_ctx, LOG1,
			FL("Tspec EID *NOT* present in assoc rsp"));
	}
	return;
}

/**
 * lim_update_ese_tsm() - update session with TSM info.
 * @mac_ctx: Pointer to Global MAC structure
 * @session_entry: PE session handle
 * @assoc_rsp:  pointer to assoc response
 *
 * This function is called by lim_process_assoc_rsp_frame() to
 * update PE session context with TSM IE data and send
 * eWNI_TSM_IE_IND to SME.
 *
 * Return: None
 */
static void lim_update_ese_tsm(tpAniSirGlobal mac_ctx,
	 tpPESession session_entry, tpSirAssocRsp assoc_rsp)
{
	uint8_t cnt = 0;
	tpEseTSMContext tsm_ctx;

	lim_log(mac_ctx, LOG1,
		FL("TSM IE Present in Reassoc Rsp"));
	/*
	 * Start the TSM  timer only if the TSPEC
	 * Ie is present in the reassoc rsp
	 */
	if (!assoc_rsp->tspecPresent) {
		lim_log(mac_ctx, LOGE,
			FL("TSM present but TSPEC IE not present"));
		return;
	}
	tsm_ctx = &session_entry->eseContext.tsm;
	/* Find the TSPEC IE with VO user priority */
	for (cnt = 0; cnt < assoc_rsp->num_tspecs; cnt++) {
		if (upToAc(assoc_rsp->TSPECInfo[cnt].user_priority) ==
			EDCA_AC_VO) {
			tsm_ctx->tid =
				assoc_rsp->TSPECInfo[cnt].user_priority;
			cdf_mem_copy(&tsm_ctx->tsmInfo,
				&assoc_rsp->tsmIE, sizeof(tSirMacESETSMIE));
#ifdef FEATURE_WLAN_ESE_UPLOAD
			lim_send_sme_tsm_ie_ind(mac_ctx,
				session_entry, assoc_rsp->tsmIE.tsid,
				assoc_rsp->tsmIE.state,
				assoc_rsp->tsmIE.msmt_interval);
#else
			limActivateTSMStatsTimer(mac_ctx, session_entry);
#endif
			if (tsm_ctx->tsmInfo.state)
				tsm_ctx->tsmMetrics.RoamingCount++;
			break;
		}
	}
}
#endif

/**
 * lim_update_stads_ext_cap() - update sta ds with ext cap
 * @mac_ctx: Pointer to Global MAC structure
 * @session_entry: PE session handle
 * @assoc_rsp:  pointer to assoc response
 *
 * This function is called by lim_process_assoc_rsp_frame() to
 * update STA DS with ext capablities.
 *
 * Return: None
 */
static void lim_update_stads_ext_cap(tpAniSirGlobal mac_ctx,
	tpPESession session_entry, tpSirAssocRsp assoc_rsp,
	tpDphHashNode sta_ds)
{
	struct s_ext_cap *ext_cap;
	if (!assoc_rsp->ExtCap.present) {
		sta_ds->timingMeasCap = 0;
#ifdef FEATURE_WLAN_TDLS
		session_entry->tdls_prohibited = false;
		session_entry->tdls_chan_swit_prohibited = false;
#endif
		lim_log(mac_ctx, LOG1, FL("ExtCap not present"));
		return;
	}

	ext_cap = (struct s_ext_cap *)assoc_rsp->ExtCap.bytes;
	lim_set_stads_rtt_cap(sta_ds, ext_cap, mac_ctx);
#ifdef FEATURE_WLAN_TDLS
	session_entry->tdls_prohibited = ext_cap->tdls_prohibited;
	session_entry->tdls_chan_swit_prohibited =
		ext_cap->tdls_chan_swit_prohibited;
	lim_log(mac_ctx, LOG1,
		FL("ExtCap: tdls_prohibited:%d, tdls_chan_swit_prohibited: %d"),
		ext_cap->tdls_prohibited,
		ext_cap->tdls_chan_swit_prohibited);
#endif
}

/**
 * lim_process_assoc_rsp_frame() - Processes assoc response
 * @mac_ctx: Pointer to Global MAC structure
 * @rx_packet_info    - A pointer to Rx packet info structure
 * @sub_type - Indicates whether it is Association Response (=0) or
 *                   Reassociation Response (=1) frame
 *
 * This function is called by limProcessMessageQueue() upon
 * Re/Association Response frame reception.
 *
 * Return: None
 */

void
lim_process_assoc_rsp_frame(tpAniSirGlobal mac_ctx,
	uint8_t *rx_pkt_info, uint8_t subtype, tpPESession session_entry)
{
	uint8_t *body;
	uint16_t caps, ie_len;
	uint32_t frame_len;
	tSirMacAddr current_bssid;
	tpSirMacMgmtHdr hdr = NULL;
	tSirMacCapabilityInfo mac_capab;
	tpDphHashNode sta_ds;
	tpSirAssocRsp assoc_rsp;
	tLimMlmAssocCnf assoc_cnf;
	tSchBeaconStruct *beacon;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	uint8_t sme_sessionid = 0;
#endif
	tCsrRoamSession *roam_session;

	/* Initialize status code to success. */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (session_entry->bRoamSynchInProgress)
		hdr = (tpSirMacMgmtHdr) mac_ctx->roam.pReassocResp;
	else
#endif
	hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	sme_sessionid = session_entry->smeSessionId;
#endif
	assoc_cnf.resultCode = eSIR_SME_SUCCESS;
	/* Update PE session Id */
	assoc_cnf.sessionId = session_entry->peSessionId;
	if (hdr == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("LFR3: Reassoc response packet header is NULL"));
		return;
	} else if (hdr->sa == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("LFR3: Reassoc resp packet source address is NULL"));
		return;
	}

	lim_log(mac_ctx, LOG1,
		FL("received Re/Assoc(%d) resp on sessionid: %d systemrole: %d"
		"and mlmstate: %d RSSI %d from " MAC_ADDRESS_STR), subtype,
		session_entry->peSessionId, GET_LIM_SYSTEM_ROLE(session_entry),
		session_entry->limMlmState,
		(uint) abs((int8_t) WMA_GET_RX_RSSI_DB(rx_pkt_info)),
		MAC_ADDR_ARRAY(hdr->sa));

	beacon = cdf_mem_malloc(sizeof(tSchBeaconStruct));
	if (NULL == beacon) {
		lim_log(mac_ctx, LOGE, FL("Unable to allocate memory"));
		return;
	}

	if (LIM_IS_AP_ROLE(session_entry) ||
		LIM_IS_BT_AMP_AP_ROLE(session_entry)) {
		/*
		 * Should not have received Re/Association
		 * Response frame on AP. Log error
		 */
		lim_log(mac_ctx, LOGE,
			FL("Should not recieved Re/Assoc Response in role %d "),
			GET_LIM_SYSTEM_ROLE(session_entry));
		cdf_mem_free(beacon);
		return;
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (session_entry->bRoamSynchInProgress) {
		hdr = (tpSirMacMgmtHdr) mac_ctx->roam.pReassocResp;
		frame_len = mac_ctx->roam.reassocRespLen - SIR_MAC_HDR_LEN_3A;
	} else {
#endif
	hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
}
#endif
	if (((subtype == LIM_ASSOC) &&
		(session_entry->limMlmState != eLIM_MLM_WT_ASSOC_RSP_STATE)) ||
		((subtype == LIM_REASSOC) &&
		((session_entry->limMlmState != eLIM_MLM_WT_REASSOC_RSP_STATE)
#if defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_ESE) || defined(FEATURE_WLAN_LFR)
		&& (session_entry->limMlmState !=
		eLIM_MLM_WT_FT_REASSOC_RSP_STATE)
#endif
		))) {
		/* Received unexpected Re/Association Response frame */

#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
		lim_log(mac_ctx, LOG1,
			FL("Recieved Re/Assoc rsp in unexpected "
			"state %d on session=%d"),
			session_entry->limMlmState, session_entry->peSessionId);
#endif
		if (!hdr->fc.retry) {
			if (!(mac_ctx->lim.retry_packet_cnt & 0xf)) {
				lim_log(mac_ctx, LOGE,
				    FL("recvd Re/Assoc rsp:not a retry frame"));
				lim_print_mlm_state(mac_ctx, LOGE,
						session_entry->limMlmState);
			} else {
				mac_ctx->lim.retry_packet_cnt++;
			}
		}
		cdf_mem_free(beacon);
		return;
	}
	sir_copy_mac_addr(current_bssid, session_entry->bssId);
	if (subtype == LIM_ASSOC) {
		if (!cdf_mem_compare
			(hdr->sa, current_bssid, sizeof(tSirMacAddr))) {
			/*
			 * Received Association Response frame from an entity
			 * other than one to which request was initiated.
			 * Ignore this and wait until Assoc Failure Timeout
			 */
			lim_log(mac_ctx, LOGW,
				 FL("received AssocRsp from unexpected peer "
				MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(hdr->sa));
			cdf_mem_free(beacon);
			return;
		}
	} else {
		if (!cdf_mem_compare
			(hdr->sa, session_entry->limReAssocbssId,
			sizeof(tSirMacAddr))) {
			/*
			 * Received Reassociation Response frame from an entity
			 * other than one to which request was initiated.
			 * Ignore this and wait until Reassoc Failure Timeout.
			 */
			lim_log(mac_ctx, LOGW,
				FL("received ReassocRsp from unexpected peer "
				MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(hdr->sa));
			cdf_mem_free(beacon);
			return;
		}
	}

	assoc_rsp = cdf_mem_malloc(sizeof(*assoc_rsp));
	if (NULL == assoc_rsp) {
		lim_log(mac_ctx, LOGP,
			FL("Allocate Memory failed in AssocRsp"));
		cdf_mem_free(beacon);
		return;
	}
	/* Get pointer to Re/Association Response frame body */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (session_entry->bRoamSynchInProgress)
		body = mac_ctx->roam.pReassocResp + SIR_MAC_HDR_LEN_3A;
	else
#endif
	body = WMA_GET_RX_MPDU_DATA(rx_pkt_info);
	/* parse Re/Association Response frame. */
	if (sir_convert_assoc_resp_frame2_struct(mac_ctx, body,
		frame_len, assoc_rsp) == eSIR_FAILURE) {
		cdf_mem_free(assoc_rsp);
		lim_log(mac_ctx, LOGE,
			FL("Parse error Assoc resp subtype %d," "length=%d"),
			frame_len, subtype);
		cdf_mem_free(beacon);
		return;
	}

	if (!assoc_rsp->suppRatesPresent) {
		lim_log(mac_ctx, LOGE,
		FL("assoc response does not have supported rate set"));
		cdf_mem_copy(&assoc_rsp->supportedRates,
			&session_entry->rateSet,
			sizeof(tSirMacRateSet));
	}

	assoc_cnf.protStatusCode = assoc_rsp->statusCode;
	if (session_entry->assocRsp != NULL) {
		lim_log(mac_ctx, LOGW,
			FL("session_entry->assocRsp is not NULL freeing it "
			 "and setting NULL"));
		cdf_mem_free(session_entry->assocRsp);
		session_entry->assocRsp = NULL;
	}

	session_entry->assocRsp = cdf_mem_malloc(frame_len);
	if (NULL == session_entry->assocRsp) {
		lim_log(mac_ctx, LOGE,
			FL("Unable to allocate memory for assoc res,len=%d"),
			 frame_len);
	} else {
		/*
		 * Store the Assoc response. This is sent
		 * to csr/hdd in join cnf response.
		 */
		cdf_mem_copy(session_entry->assocRsp, body, frame_len);
		session_entry->assocRspLen = frame_len;
	}

#ifdef WLAN_FEATURE_VOWIFI_11R
	lim_update_ric_data(mac_ctx, session_entry, assoc_rsp);
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	roam_session =
		&mac_ctx->roam.roamSession[sme_sessionid];
	if (assoc_rsp->FTInfo.R0KH_ID.present) {
		roam_session->ftSmeContext.r0kh_id_len =
			assoc_rsp->FTInfo.R0KH_ID.num_PMK_R0_ID;
		cdf_mem_copy(roam_session->ftSmeContext.r0kh_id,
			assoc_rsp->FTInfo.R0KH_ID.PMK_R0_ID,
			roam_session->ftSmeContext.r0kh_id_len);
	} else {
		roam_session->ftSmeContext.r0kh_id_len = 0;
		cdf_mem_zero(roam_session->ftSmeContext.r0kh_id,
			SIR_ROAM_R0KH_ID_MAX_LEN);
	}
#endif

#ifdef FEATURE_WLAN_ESE
	lim_update_ese_tspec(mac_ctx, session_entry, assoc_rsp);
#endif

	if (assoc_rsp->capabilityInfo.ibss) {
		/*
		 * Received Re/Association Response from peer
		 * with IBSS capability set.
		 * Ignore the frame and wait until Re/assoc
		 * failure timeout.
		 */
		lim_log(mac_ctx, LOGE,
			FL("received Re/AssocRsp frame with IBSS capability"));
		cdf_mem_free(assoc_rsp);
		cdf_mem_free(beacon);
		return;
	}

	if (cfg_get_capability_info(mac_ctx, &caps, session_entry)
		!= eSIR_SUCCESS) {
		cdf_mem_free(assoc_rsp);
		cdf_mem_free(beacon);
		lim_log(mac_ctx, LOGP, FL("could not retrieve Capabilities "));
		return;
	}
	lim_copy_u16((uint8_t *) &mac_capab, caps);

	/* Stop Association failure timer */
	if (subtype == LIM_ASSOC)
		lim_deactivate_and_change_timer(mac_ctx, eLIM_ASSOC_FAIL_TIMER);
	else
	{
		/* Stop Reassociation failure timer */
#if  defined (WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_ESE) || defined(FEATURE_WLAN_LFR)
		mac_ctx->lim.reAssocRetryAttempt = 0;
		if ((NULL != mac_ctx->lim.pSessionEntry)
		    && (NULL !=
			mac_ctx->lim.pSessionEntry->pLimMlmReassocRetryReq)) {
			cdf_mem_free(
			mac_ctx->lim.pSessionEntry->pLimMlmReassocRetryReq);
			mac_ctx->lim.pSessionEntry->pLimMlmReassocRetryReq =
				NULL;
		}
#endif
		lim_deactivate_and_change_timer(mac_ctx,
			eLIM_REASSOC_FAIL_TIMER);
	}

	if (assoc_rsp->statusCode != eSIR_MAC_SUCCESS_STATUS
#ifdef WLAN_FEATURE_11W
		&& (session_entry->limRmfEnabled ||
			assoc_rsp->statusCode != eSIR_MAC_TRY_AGAIN_LATER)
#endif
	    ) {
		/*
		 *Re/Association response was received
		 * either with failure code.
		*/
		lim_log(mac_ctx, LOGE,
			FL("received Re/AssocRsp frame failure code %d"),
			 assoc_rsp->statusCode);
		/*
		 * Need to update 'association failure' error counter
		 * along with STATUS CODE
		 * Return Assoc confirm to SME with received failure code
		*/
		assoc_cnf.resultCode = eSIR_SME_ASSOC_REFUSED;
		/* Delete Pre-auth context for the associated BSS */
		if (lim_search_pre_auth_list(mac_ctx, hdr->sa))
			lim_delete_pre_auth_node(mac_ctx, hdr->sa);
		goto assocReject;
	} else if ((assoc_rsp->aid & 0x3FFF) > 2007) {
		/*
		 * Re/Association response was received
		 * with invalid AID value
		*/
		lim_log(mac_ctx, LOGE, FL("received Re/AssocRsp frame with"
			"invalid aid %X"), assoc_rsp->aid);
		assoc_cnf.resultCode = eSIR_SME_INVALID_ASSOC_RSP_RXED;
		assoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
		/* Send advisory Disassociation frame to AP */
		lim_send_disassoc_mgmt_frame(mac_ctx,
			eSIR_MAC_UNSPEC_FAILURE_REASON,
			hdr->sa, session_entry, false);
		goto assocReject;
	}
	/*
	 * Association Response received with success code
	 * Set the link state to POSTASSOC now that we have received
	 * assoc/reassoc response
	 * NOTE: for BTAMP case, it is being handled in
	 * lim_process_mlm_assoc_req
	 */
#ifdef WLAN_FEATURE_11W
	if (session_entry->limRmfEnabled &&
		assoc_rsp->statusCode == eSIR_MAC_TRY_AGAIN_LATER) {
		if (assoc_rsp->TimeoutInterval.present &&
		(assoc_rsp->TimeoutInterval.timeoutType ==
			SIR_MAC_TI_TYPE_ASSOC_COMEBACK)) {
			uint16_t timeout_value =
				assoc_rsp->TimeoutInterval.timeoutValue;
			if (timeout_value < 10) {
				/*
				 * if this value is less than 10 then our timer
				 * will fail to start and due to this we will
				 * never re-attempt. Better modify the timer
				 * value here.
				 */
				timeout_value = 10;
			}
			lim_log(mac_ctx, LOG1,
				FL("ASSOC res with eSIR_MAC_TRY_AGAIN_LATER "
				" recvd.Starting timer to wait timeout=%d."),
				timeout_value);
			if (CDF_STATUS_SUCCESS !=
				cdf_mc_timer_start(
					&session_entry->pmfComebackTimer,
					timeout_value)) {
				lim_log(mac_ctx, LOGE,
					FL("Failed to start comeback timer."));
			}
		} else {
			lim_log(mac_ctx, LOGW,
				FL("ASSOC resp with try again event recvd. "
				"But try again time interval IE is wrong."));
		}
		cdf_mem_free(beacon);
		cdf_mem_free(assoc_rsp);
		return;
	}
#endif
	if (!((session_entry->bssType == eSIR_BTAMP_STA_MODE) ||
		((session_entry->bssType == eSIR_BTAMP_AP_MODE) &&
		LIM_IS_BT_AMP_STA_ROLE(session_entry)))) {
		if (lim_set_link_state
			(mac_ctx, eSIR_LINK_POSTASSOC_STATE,
			session_entry->bssId,
			session_entry->selfMacAddr, NULL,
			NULL) != eSIR_SUCCESS) {
			lim_log(mac_ctx, LOGE,
				FL("Set link state to POSTASSOC failed"));
			cdf_mem_free(beacon);
			cdf_mem_free(assoc_rsp);
			return;
		}
	}
	if (subtype == LIM_REASSOC) {
		lim_log
		(mac_ctx, LOG1, FL("Successfully Reassociated with BSS"));
#ifdef FEATURE_WLAN_DIAG_SUPPORT
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_ROAM_ASSOC_COMP_EVENT,
			      session_entry, eSIR_SUCCESS, eSIR_SUCCESS);
#endif
#ifdef FEATURE_WLAN_ESE
	if (assoc_rsp->tsmPresent)
		lim_update_ese_tsm(mac_ctx, session_entry, assoc_rsp);
#endif
		if (session_entry->pLimMlmJoinReq) {
			cdf_mem_free(session_entry->pLimMlmJoinReq);
			session_entry->pLimMlmJoinReq = NULL;
		}

		session_entry->limAssocResponseData = (void *)assoc_rsp;
		/*
		 * Store the ReAssocRsp Frame in DphTable
		 * to be used during processing DelSta and
		 * DelBss to send AddBss again
		 */
		sta_ds =
			dph_get_hash_entry(mac_ctx, DPH_STA_HASH_INDEX_PEER,
				&session_entry->dph.dphHashTable);

		if (!sta_ds) {
			lim_log(mac_ctx, LOGE,
				FL("could not get hash entry at DPH for"));
			lim_print_mac_addr(mac_ctx, hdr->sa, LOGE);
			assoc_cnf.resultCode =
				eSIR_SME_INVALID_ASSOC_RSP_RXED;
			assoc_cnf.protStatusCode =
				eSIR_MAC_UNSPEC_FAILURE_STATUS;

			/* Send advisory Disassociation frame to AP */
			lim_send_disassoc_mgmt_frame(mac_ctx,
				eSIR_MAC_UNSPEC_FAILURE_REASON, hdr->sa,
				session_entry, false);
			goto assocReject;
		}
#if defined(WLAN_FEATURE_VOWIFI_11R) || defined (FEATURE_WLAN_ESE) || defined(FEATURE_WLAN_LFR)
		if (session_entry->limMlmState ==
		    eLIM_MLM_WT_FT_REASSOC_RSP_STATE) {
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
			lim_log(mac_ctx, LOG1, FL("Sending self sta"));
#endif
			lim_update_assoc_sta_datas(mac_ctx, sta_ds, assoc_rsp,
				session_entry);
			/* Store assigned AID for TIM processing */
			session_entry->limAID = assoc_rsp->aid & 0x3FFF;
			/* Downgrade the EDCA parameters if needed */
			lim_set_active_edca_params(mac_ctx,
				session_entry->gLimEdcaParams,
				session_entry);
			/* Send the active EDCA parameters to HAL */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
			if (!session_entry->bRoamSynchInProgress) {
#endif
			lim_send_edca_params(mac_ctx,
				session_entry->gLimEdcaParamsActive,
				sta_ds->bssId);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		}
#endif
			lim_add_ft_sta_self(mac_ctx, (assoc_rsp->aid & 0x3FFF),
				session_entry);
			cdf_mem_free(beacon);
			return;
		}
#endif /* WLAN_FEATURE_VOWIFI_11R */

		/*
		 * If we're re-associating to the same BSS,
		 * we don't want to invoke delete STA, delete
		 * BSS, as that would remove the already
		 * established TSPEC. Just go ahead and re-add
		 * the BSS, STA with new capability information.
		 * However, if we're re-associating to a different
		 * BSS, then follow thru with del STA, del BSS,
		 * add BSS, add STA.
		 */
		if (sir_compare_mac_addr(session_entry->bssId,
			session_entry->limReAssocbssId))
			lim_handle_add_bss_in_re_assoc_context(mac_ctx, sta_ds,
				 session_entry);
		else {
			/*
			 * reset the uapsd mask settings since
			 * we're re-associating to new AP
			 */
			session_entry->gUapsdPerAcDeliveryEnableMask = 0;
			session_entry->gUapsdPerAcTriggerEnableMask = 0;

			if (lim_cleanup_rx_path(mac_ctx, sta_ds, session_entry)
				!= eSIR_SUCCESS) {
				lim_log(mac_ctx, LOGE,
					FL("Could not cleanup the rx path"));
				goto assocReject;
			}
		}
		cdf_mem_free(beacon);
		return;
	}
	lim_log(mac_ctx, LOG1,
		FL("Successfully Associated with BSS " MAC_ADDRESS_STR),
		 MAC_ADDR_ARRAY(hdr->sa));
#ifdef FEATURE_WLAN_ESE
	if (session_entry->eseContext.tsm.tsmInfo.state)
		session_entry->eseContext.tsm.tsmMetrics.RoamingCount = 0;
#endif
	/* Store assigned AID for TIM processing */
	session_entry->limAID = assoc_rsp->aid & 0x3FFF;

	/* STA entry was created during pre-assoc state. */
	sta_ds = dph_get_hash_entry(mac_ctx, DPH_STA_HASH_INDEX_PEER,
			&session_entry->dph.dphHashTable);
	if (sta_ds == NULL) {
		/* Could not add hash table entry */
		lim_log(mac_ctx, LOGE, FL("could not get hash entry at DPH "));
		lim_print_mac_addr(mac_ctx, hdr->sa, LOGE);
		assoc_cnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
		assoc_cnf.protStatusCode = eSIR_SME_SUCCESS;
		lim_post_sme_message(mac_ctx, LIM_MLM_ASSOC_CNF,
			(uint32_t *) &assoc_cnf);
		cdf_mem_free(assoc_rsp);
		cdf_mem_free(beacon);
		return;
	}
	/* Delete Pre-auth context for the associated BSS */
	if (lim_search_pre_auth_list(mac_ctx, hdr->sa))
		lim_delete_pre_auth_node(mac_ctx, hdr->sa);

	lim_update_assoc_sta_datas(mac_ctx, sta_ds, assoc_rsp, session_entry);
	/*
	 * Extract the AP capabilities from the beacon that
	 * was received earlier
	*/
	ie_len = lim_get_ielen_from_bss_description(
		&session_entry->pLimJoinReq->bssDescription);
	lim_extract_ap_capabilities(mac_ctx,
		(uint8_t *) session_entry->pLimJoinReq->bssDescription.ieFields,
		ie_len,
		beacon);

	if (mac_ctx->lim.gLimProtectionControl !=
		WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
		lim_decide_sta_protection_on_assoc(mac_ctx, beacon,
			session_entry);

	if (beacon->erpPresent) {
		if (beacon->erpIEInfo.barkerPreambleMode)
			session_entry->beaconParams.fShortPreamble = false;
		else
			session_entry->beaconParams.fShortPreamble = true;
	}

#ifdef FEATURE_WLAN_DIAG_SUPPORT
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_CONNECTED, session_entry,
			      eSIR_SUCCESS, eSIR_SUCCESS);
#endif
	if (assoc_rsp->QosMapSet.present)
		cdf_mem_copy(&session_entry->QosMapSet,
			&assoc_rsp->QosMapSet, sizeof(tSirQosMapSet));
	 else
		cdf_mem_zero(&session_entry->QosMapSet, sizeof(tSirQosMapSet));

	lim_update_stads_ext_cap(mac_ctx, session_entry, assoc_rsp, sta_ds);
	/* Update the BSS Entry, this entry was added during preassoc. */
	if (eSIR_SUCCESS == lim_sta_send_add_bss(mac_ctx, assoc_rsp,
			beacon,
			&session_entry->pLimJoinReq->bssDescription, true,
			 session_entry)) {
		cdf_mem_free(assoc_rsp);
		cdf_mem_free(beacon);
		return;
	} else {
		lim_log(mac_ctx, LOGE, FL("could not update the bss entry"));
		assoc_cnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
		assoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
	}

assocReject:
	if ((subtype == LIM_ASSOC)
#ifdef WLAN_FEATURE_VOWIFI_11R
		|| ((subtype == LIM_REASSOC)
		&& (session_entry->limMlmState ==
		    eLIM_MLM_WT_FT_REASSOC_RSP_STATE))
#endif
	    ) {
		lim_log(mac_ctx, LOGE, FL("Assoc Rejected by the peer. "
			"mlmestate: %d sessionid %d Reason: %d MACADDR:"
			MAC_ADDRESS_STR),
			session_entry->limMlmState,
			session_entry->peSessionId,
			assoc_cnf.resultCode, MAC_ADDR_ARRAY(hdr->sa));
		session_entry->limMlmState = eLIM_MLM_IDLE_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
			session_entry->peSessionId,
			session_entry->limMlmState));
		if (session_entry->pLimMlmJoinReq) {
			cdf_mem_free(session_entry->pLimMlmJoinReq);
			session_entry->pLimMlmJoinReq = NULL;
		}
		if (subtype == LIM_ASSOC) {
			lim_post_sme_message(mac_ctx, LIM_MLM_ASSOC_CNF,
				(uint32_t *) &assoc_cnf);
		}
#ifdef WLAN_FEATURE_VOWIFI_11R
		else {
			assoc_cnf.resultCode = eSIR_SME_FT_REASSOC_FAILURE;
			lim_post_sme_message(mac_ctx, LIM_MLM_REASSOC_CNF,
					(uint32_t *)&assoc_cnf);
		}
#endif
	} else {
		lim_restore_pre_reassoc_state(mac_ctx,
			eSIR_SME_REASSOC_REFUSED,
			assoc_cnf.protStatusCode,
			session_entry);
	}

	cdf_mem_free(beacon);
	cdf_mem_free(assoc_rsp);
	return;
}
