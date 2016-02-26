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
 * This file sch_beacon_process.cc contains beacon processing related
 * functions
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "cds_api.h"
#include "wni_cfg.h"

#include "cfg_api.h"
#include "lim_api.h"
#include "utils_api.h"
#include "sch_debug.h"
#include "sch_api.h"

#include "lim_utils.h"
#include "lim_send_messages.h"
#include "lim_sta_hash_api.h"

#if defined WLAN_FEATURE_VOWIFI
#include "rrm_api.h"
#endif

#ifdef FEATURE_WLAN_DIAG_SUPPORT
#include "host_diag_core_log.h"
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

#include "wma.h"
/**
 * Number of bytes of variation in beacon length from the last beacon
 * to trigger reprogramming of rx delay register
 */
#define SCH_BEACON_LEN_DELTA       3

/* calculate 2^cw - 1 */
#define CW_GET(cw) (((cw) == 0) ? 1 : ((1 << (cw)) - 1))

static void
ap_beacon_process_5_ghz(tpAniSirGlobal mac_ctx, uint8_t *rx_pkt_info,
			tpSchBeaconStruct bcn_struct,
			tpUpdateBeaconParams bcn_prm, tpPESession session,
			uint32_t phy_mode)
{
	tpSirMacMgmtHdr mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	if (!session->htCapability)
		return;

	if (bcn_struct->channelNumber != session->currentOperChannel)
		return;

	/* 11a (non HT) AP  overlaps or */
	/* HT AP with HT op mode as mixed overlaps. */
	/* HT AP with HT op mode as overlap legacy overlaps. */
	if (!bcn_struct->HTInfo.present
	    || (eSIR_HT_OP_MODE_MIXED == bcn_struct->HTInfo.opMode)
	    || (eSIR_HT_OP_MODE_OVERLAP_LEGACY == bcn_struct->HTInfo.opMode)) {
		lim_update_overlap_sta_param(mac_ctx, mac_hdr->bssId,
					&(mac_ctx->lim.gLimOverlap11aParams));

		if (mac_ctx->lim.gLimOverlap11aParams.numSta
		    && !mac_ctx->lim.gLimOverlap11aParams.protectionEnabled) {
			lim_update_11a_protection(mac_ctx, true, true,
						 bcn_prm, session);
		}
		return;
	}
	/* HT AP with HT20 op mode overlaps. */
	if (eSIR_HT_OP_MODE_NO_LEGACY_20MHZ_HT != bcn_struct->HTInfo.opMode)
		return;

	lim_update_overlap_sta_param(mac_ctx, mac_hdr->bssId,
				     &(mac_ctx->lim.gLimOverlapHt20Params));

	if (mac_ctx->lim.gLimOverlapHt20Params.numSta
	    && !mac_ctx->lim.gLimOverlapHt20Params.protectionEnabled)
		lim_enable_ht20_protection(mac_ctx, true, true,
					   bcn_prm, session);
}

static void
ap_beacon_process_24_ghz(tpAniSirGlobal mac_ctx, uint8_t *rx_pkt_info,
			 tpSchBeaconStruct bcn_struct,
			 tpUpdateBeaconParams bcn_prm, tpPESession session,
			 uint32_t phy_mode)
{
	tpSirMacMgmtHdr mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	bool tmp_exp = false;
	/* We are 11G AP. */
	if ((phy_mode == WNI_CFG_PHY_MODE_11G) &&
	    (false == session->htCapability)) {
		if (bcn_struct->channelNumber != session->currentOperChannel)
			return;

		tmp_exp = (!bcn_struct->erpPresent
			    && !bcn_struct->HTInfo.present)
			    /* if erp not present then  11B AP overlapping */
			    || (!mac_ctx->roam.configParam.ignore_peer_erp_info &&
					bcn_struct->erpPresent &&
					(bcn_struct->erpIEInfo.useProtection
				    || bcn_struct->erpIEInfo.nonErpPresent));
		if (!tmp_exp)
			return;
#ifdef FEATURE_WLAN_ESE
		if (session->isESEconnection)
			CDF_TRACE(CDF_MODULE_ID_PE,
				  CDF_TRACE_LEVEL_INFO,
				  FL("[INFOLOG]ESE 11g erpPresent=%d useProtection=%d nonErpPresent=%d"),
				  bcn_struct->erpPresent,
				  bcn_struct->erpIEInfo.useProtection,
				  bcn_struct->erpIEInfo.nonErpPresent);
#endif
		lim_enable_overlap11g_protection(mac_ctx, bcn_prm,
						 mac_hdr, session);
		return;
	}
	/* handling the case when HT AP has overlapping legacy BSS. */
	if (!session->htCapability)
		return;

	if (bcn_struct->channelNumber != session->currentOperChannel)
		return;

	tmp_exp = (!bcn_struct->erpPresent && !bcn_struct->HTInfo.present)
		    /* if erp not present then  11B AP overlapping */
		    || (!mac_ctx->roam.configParam.ignore_peer_erp_info &&
				bcn_struct->erpPresent &&
				(bcn_struct->erpIEInfo.useProtection
			    || bcn_struct->erpIEInfo.nonErpPresent));
	if (tmp_exp) {
#ifdef FEATURE_WLAN_ESE
		if (session->isESEconnection) {
			CDF_TRACE(CDF_MODULE_ID_PE, CDF_TRACE_LEVEL_INFO,
				  FL("[INFOLOG]ESE 11g erpPresent=%d useProtection=%d nonErpPresent=%d"),
				  bcn_struct->erpPresent,
				  bcn_struct->erpIEInfo.useProtection,
				  bcn_struct->erpIEInfo.nonErpPresent);
		}
#endif
		lim_enable_overlap11g_protection(mac_ctx, bcn_prm,
						 mac_hdr, session);
	}
	/* 11g device overlaps */
	tmp_exp = bcn_struct->erpPresent
		&& !(bcn_struct->erpIEInfo.useProtection
		     || bcn_struct->erpIEInfo.nonErpPresent)
		&& !(bcn_struct->HTInfo.present);
	if (tmp_exp) {
		lim_update_overlap_sta_param(mac_ctx, mac_hdr->bssId,
					     &(session->gLimOverlap11gParams));

		if (session->gLimOverlap11gParams.numSta
		    && !session->gLimOverlap11gParams.protectionEnabled)
			lim_enable_ht_protection_from11g(mac_ctx, true, true,
							 bcn_prm, session);
	}
	/* ht device overlaps.
	 * here we will check for HT related devices only which might need
	 * protection. check for 11b and 11g is already done in the previous
	 * blocks. so we will not check for HT operating mode as MIXED.
	 */
	if (!bcn_struct->HTInfo.present)
		return;

	/*
	 * if we are not already in mixed mode or legacy mode as HT operating
	 * mode and received beacon has HT operating mode as legacy then we need
	 * to enable protection from 11g station. we don't need protection from
	 * 11b because if that's needed then our operating mode would have
	 * already been set to legacy in the previous blocks.
	 */
	if (eSIR_HT_OP_MODE_OVERLAP_LEGACY == bcn_struct->HTInfo.opMode) {
		if (eSIR_HT_OP_MODE_OVERLAP_LEGACY == mac_ctx->lim.gHTOperMode
		    || eSIR_HT_OP_MODE_MIXED == mac_ctx->lim.gHTOperMode)
			return;
		lim_update_overlap_sta_param(mac_ctx, mac_hdr->bssId,
					     &(session->gLimOverlap11gParams));
		if (session->gLimOverlap11gParams.numSta
		    && !session->gLimOverlap11gParams.protectionEnabled)
			lim_enable_ht_protection_from11g(mac_ctx, true, true,
							 bcn_prm, session);
		return;
	}

	if (eSIR_HT_OP_MODE_NO_LEGACY_20MHZ_HT == bcn_struct->HTInfo.opMode) {
		lim_update_overlap_sta_param(mac_ctx, mac_hdr->bssId,
					     &(session->gLimOverlapHt20Params));
		if (session->gLimOverlapHt20Params.numSta
		    && !session->gLimOverlapHt20Params.protectionEnabled)
			lim_enable_ht20_protection(mac_ctx, true, true,
						   bcn_prm, session);
	}
}

/**
 * ap_beacon_process() - processes incoming beacons
 *
 * @mac_ctx:         mac global context
 * @rx_pkt_info:     incoming beacon packet
 * @bcn_struct:      beacon struct
 * @bcn_prm:         beacon params
 * @session:         pe session entry
 *
 * Return: void
 */
static void
ap_beacon_process(tpAniSirGlobal mac_ctx, uint8_t *rx_pkt_info,
		  tpSchBeaconStruct bcn_struct,
		  tpUpdateBeaconParams bcn_prm, tpPESession session)
{
	uint32_t phy_mode;
	tSirRFBand rf_band = SIR_BAND_UNKNOWN;
	/* Get RF band from session */
	rf_band = session->limRFBand;

	lim_get_phy_mode(mac_ctx, &phy_mode, session);

	if (SIR_BAND_5_GHZ == rf_band)
		ap_beacon_process_5_ghz(mac_ctx, rx_pkt_info, bcn_struct,
					bcn_prm, session, phy_mode);
	else if (SIR_BAND_2_4_GHZ == rf_band)
		ap_beacon_process_24_ghz(mac_ctx, rx_pkt_info, bcn_struct,
					 bcn_prm, session, phy_mode);
	mac_ctx->sch.gSchBcnIgnored++;
}

/* -------------------------------------------------------------------- */

/**
 * __sch_beacon_process_no_session
 *
 * FUNCTION:
 * Process the received beacon frame when
 *  -- Station is not scanning
 *  -- No corresponding session is found
 *
 * LOGIC:
 *        Following scenarios exist when Session Does not exist:
 *             * IBSS Beacons, when IBSS session already exists with same SSID,
 *                but from STA which has not yet joined and has a different BSSID.
 *                - invoke lim_handle_ibs_scoalescing with the session context of existing IBSS session.
 *
 *             * IBSS Beacons when IBSS session does not exist, only Infra or BT-AMP session exists,
 *                then save the beacon in the scan results and throw it away.
 *
 *             * Infra Beacons
 *                - beacons received when no session active
 *                    should not come here, it should be handled as part of scanning,
 *                    else they should not be getting received, should update scan results and drop it if that happens.
 *                - beacons received when IBSS session active:
 *                    update scan results and drop it.
 *                - beacons received when Infra session(STA) is active:
 *                    update scan results and drop it
 *                - beacons received when BT-STA session is active:
 *                    update scan results and drop it.
 *                - beacons received when Infra/BT-STA  or Infra/IBSS is active.
 *                    update scan results and drop it.
 *

 */
static void __sch_beacon_process_no_session(tpAniSirGlobal pMac,
					    tpSchBeaconStruct pBeacon,
					    uint8_t *pRxPacketInfo)
{
	tpPESession psessionEntry = NULL;

	if ((psessionEntry = lim_is_ibss_session_active(pMac)) != NULL) {
		lim_handle_ibss_coalescing(pMac, pBeacon, pRxPacketInfo,
					   psessionEntry);
	}
	/* If station(STA/BT-STA/BT-AP/IBSS) mode, Always save the beacon in the scan results, if atleast one session is active */
	/* sch_beacon_processNoSession will be called only when there is atleast one session active, so not checking */
	/* it again here. */
	lim_check_and_add_bss_description(pMac, pBeacon, pRxPacketInfo, false,
					  false);
	return;
}

/**
 * get_operating_channel_width() - Get operating channel width
 * @pStaDs - station entry.
 *
 * This function returns the oeprating channgl width based on
 * the supported channel width entry.
 *
 * Return: tSirMacHTChannelWidth on success
 */
tSirMacHTChannelWidth get_operating_channel_width(tpDphHashNode stads)
{
	tSirMacHTChannelWidth ch_width = eHT_CHANNEL_WIDTH_20MHZ;

	if (stads->vhtSupportedChannelWidthSet ==
			WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ)
		ch_width = eHT_CHANNEL_WIDTH_160MHZ;
	else if (stads->vhtSupportedChannelWidthSet ==
			WNI_CFG_VHT_CHANNEL_WIDTH_80_PLUS_80MHZ)
		ch_width = eHT_CHANNEL_WIDTH_160MHZ;
	else if (stads->vhtSupportedChannelWidthSet ==
			WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ)
		ch_width = eHT_CHANNEL_WIDTH_80MHZ;
	else if (stads->htSupportedChannelWidthSet)
		ch_width = eHT_CHANNEL_WIDTH_40MHZ;
	else
		ch_width = eHT_CHANNEL_WIDTH_20MHZ;

	return ch_width;
}

/*
 * sch_bcn_process_sta_bt_amp_sta() - Process the received beacon frame for sta,
 * bt_amp_sta
 *
 * @mac_ctx:        mac_ctx
 * @bcn:            beacon struct
 * @rx_pkt_info:    received packet info
 * @session:        pe session pointer
 * @bssIdx:         bss index
 * @beaconParams:   update beacon params
 * @sendProbeReq:   out flag to indicate if probe rsp is to be sent
 * @pMh:            mac header
 *
 * Process the received beacon frame for sta, bt_amp_sta
 *
 * Return: success of failure of operation
 */
static bool
sch_bcn_process_sta_bt_amp_sta(tpAniSirGlobal mac_ctx,
			       tpSchBeaconStruct bcn,
			       uint8_t *rx_pkt_info,
			       tpPESession session, uint8_t *bssIdx,
			       tUpdateBeaconParams *beaconParams,
			       uint8_t *sendProbeReq, tpSirMacMgmtHdr pMh)
{
	uint32_t bi;
	tpDphHashNode pStaDs = NULL;
	/*
	 *  This handles two cases:
	 *  -- Infra STA receving beacons from AP
	 *  -- BTAMP_STA receving beacons from BTAMP_AP
	 */
	/* Always save the beacon into LIM's cached scan results */
	lim_check_and_add_bss_description(mac_ctx, bcn, rx_pkt_info,
					  false, false);

	/**
	 * This is the Beacon received from the AP  we're currently associated
	 * with. Check if there are any changes in AP's capabilities
	 */
	if ((uint8_t) bcn->channelNumber != session->currentOperChannel) {
		PELOGE(sch_log(mac_ctx, LOGE,
		       FL("Channel Change from %d --> %d - Ignoring beacon!"),
		       session->currentOperChannel,
		       bcn->channelNumber);)
		return false;
	}
	lim_detect_change_in_ap_capabilities(mac_ctx, bcn, session);
	if (lim_get_sta_hash_bssidx(mac_ctx, DPH_STA_HASH_INDEX_PEER, bssIdx,
				    session) != eSIR_SUCCESS)
		return false;

	beaconParams->bssIdx = *bssIdx;
	cdf_mem_copy((uint8_t *) &session->lastBeaconTimeStamp,
			(uint8_t *) bcn->timeStamp, sizeof(uint64_t));
	session->lastBeaconDtimCount = bcn->tim.dtimCount;
	session->lastBeaconDtimPeriod = bcn->tim.dtimPeriod;
	session->currentBssBeaconCnt++;

	MTRACE(mac_trace(mac_ctx, TRACE_CODE_RX_MGMT_TSF,
	       session->peSessionId, bcn->timeStamp[0]);)
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_RX_MGMT_TSF,
	       session->peSessionId, bcn->timeStamp[1]);)

	/* Read beacon interval session Entry */
	bi = session->beaconParams.beaconInterval;
	if (bi != bcn->beaconInterval) {
		PELOG1(sch_log(mac_ctx, LOG1,
		       FL("Beacon interval changed from %d to %d"),
		       bcn->beaconInterval, bi);)

		bi = bcn->beaconInterval;
		session->beaconParams.beaconInterval = (uint16_t) bi;
		beaconParams->paramChangeBitmap |= PARAM_BCN_INTERVAL_CHANGED;
		beaconParams->beaconInterval = (uint16_t) bi;
	}

	if (bcn->cfPresent) {
		cfg_set_int(mac_ctx, WNI_CFG_CFP_PERIOD,
			    bcn->cfParamSet.cfpPeriod);
		lim_send_cf_params(mac_ctx, *bssIdx,
				   bcn->cfParamSet.cfpCount,
				   bcn->cfParamSet.cfpPeriod);
	}

	/* No need to send DTIM Period and Count to HAL/SMAC */
	/* SMAC already parses TIM bit. */
	if (bcn->timPresent)
		cfg_set_int(mac_ctx, WNI_CFG_DTIM_PERIOD, bcn->tim.dtimPeriod);

	if (mac_ctx->lim.gLimProtectionControl !=
			WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
		lim_decide_sta_protection(mac_ctx, bcn, beaconParams, session);

	if (bcn->erpPresent) {
		if (bcn->erpIEInfo.barkerPreambleMode)
			lim_enable_short_preamble(mac_ctx, false,
						  beaconParams, session);
		else
			lim_enable_short_preamble(mac_ctx, true,
						  beaconParams, session);
	}
	lim_update_short_slot(mac_ctx, bcn, beaconParams, session);

	pStaDs = dph_get_hash_entry(mac_ctx, DPH_STA_HASH_INDEX_PEER,
				    &session->dph.dphHashTable);
	if ((bcn->wmeEdcaPresent && (session->limWmeEnabled))
	    || (bcn->edcaPresent && (session->limQosEnabled))) {
		if (bcn->edcaParams.qosInfo.count !=
		    session->gLimEdcaParamSetCount) {
			if (sch_beacon_edca_process(mac_ctx, &bcn->edcaParams,
						    session) != eSIR_SUCCESS) {
				PELOGE(sch_log(mac_ctx, LOGE,
				       FL("EDCA parameter processing error"));)
			} else if (pStaDs != NULL) {
				/* If needed, downgrade the EDCA parameters */
				lim_set_active_edca_params(mac_ctx,
					session->gLimEdcaParams, session);
				lim_send_edca_params(mac_ctx,
					session->gLimEdcaParamsActive,
					pStaDs->bssId);
			} else {
				PELOGE(sch_log(mac_ctx, LOGE,
				       FL("Self Entry missing in Hash Table"));)
			}
		}
		return true;
	}

	if ((bcn->qosCapabilityPresent && session->limQosEnabled)
	    && (bcn->qosCapability.qosInfo.count !=
		session->gLimEdcaParamSetCount))
		*sendProbeReq = true;

	return true;
}

/**
 * update_nss() - Function to update NSS
 * @mac_ctx: pointer to Global Mac structure
 * @sta_ds: pointer to tpDphHashNode
 * @beacon: pointer to tpSchBeaconStruct
 * @session_entry: pointer to tpPESession
 * @mgmt_hdr: pointer to tpSirMacMgmtHdr
 *
 * function to update NSS
 *
 * Return: none
 */
void update_nss(tpAniSirGlobal mac_ctx, tpDphHashNode sta_ds,
		tpSchBeaconStruct beacon, tpPESession session_entry,
		tpSirMacMgmtHdr mgmt_hdr)
{
	if (sta_ds->vhtSupportedRxNss != (beacon->OperatingMode.rxNSS + 1)) {
		sta_ds->vhtSupportedRxNss =
			beacon->OperatingMode.rxNSS + 1;
		lim_set_nss_change(mac_ctx, session_entry,
			sta_ds->vhtSupportedRxNss, sta_ds->staIndex,
			mgmt_hdr->sa);
	}
}

/*
 * sch_bcn_process_sta_bt_amp_sta_ibss() - Process the received beacon frame
 * for sta, bt_amp_sta and ibss
 *
 * @mac_ctx:        mac_ctx
 * @bcn:            beacon struct
 * @rx_pkt_info:    received packet info
 * @session:        pe session pointer
 * @bssIdx:         bss index
 * @beaconParams:   update beacon params
 * @sendProbeReq:   out flag to indicate if probe rsp is to be sent
 * @pMh:            mac header
 *
 * Process the received beacon frame for sta, bt_amp_sta and ibss
 *
 * Return: void
 */
static void
sch_bcn_process_sta_bt_amp_sta_ibss(tpAniSirGlobal mac_ctx,
				    tpSchBeaconStruct bcn,
				    uint8_t *rx_pkt_info,
				    tpPESession session, uint8_t *bssIdx,
				    tUpdateBeaconParams *beaconParams,
				    uint8_t *sendProbeReq, tpSirMacMgmtHdr pMh)
{
	tpDphHashNode pStaDs = NULL;
	uint16_t aid;
	uint8_t operMode;
	uint8_t chWidth = 0;
	uint8_t cb_mode;
	uint32_t fw_vht_ch_wd = wma_get_vht_ch_width();
	bool skip_opmode_update = false;

	if (RF_CHAN_14 >= session->currentOperChannel)
		cb_mode = mac_ctx->roam.configParam.channelBondingMode24GHz;
	else
		cb_mode = mac_ctx->roam.configParam.channelBondingMode5GHz;
	/* check for VHT capability */
	pStaDs = dph_lookup_hash_entry(mac_ctx, pMh->sa, &aid,
			&session->dph.dphHashTable);
	if ((NULL == pStaDs) ||
	  (WNI_CFG_CHANNEL_BONDING_MODE_DISABLE == cb_mode) ||
	  ((NULL != pStaDs) &&
	   (STA_INVALID_IDX == pStaDs->staIndex)))
		return;

	if (session->vhtCapability && bcn->OperatingMode.present) {
		operMode = get_operating_channel_width(pStaDs);
		if ((operMode == eHT_CHANNEL_WIDTH_80MHZ) &&
		    (bcn->OperatingMode.chanWidth > eHT_CHANNEL_WIDTH_80MHZ))
			skip_opmode_update = true;

		if (!skip_opmode_update &&
		    (operMode != bcn->OperatingMode.chanWidth)) {
			PELOGE(sch_log(mac_ctx, LOGE,
			       FL("received OpMode Chanwidth %d, staIdx = %d"),
			       bcn->OperatingMode.chanWidth, pStaDs->staIndex);)
			PELOGE(sch_log(mac_ctx, LOGE,
			       FL("MAC - %0x:%0x:%0x:%0x:%0x:%0x"),
			       pMh->sa[0], pMh->sa[1],
			       pMh->sa[2], pMh->sa[3],
			       pMh->sa[4], pMh->sa[5]);)

			if ((bcn->OperatingMode.chanWidth >=
				eHT_CHANNEL_WIDTH_160MHZ) &&
				(fw_vht_ch_wd > eHT_CHANNEL_WIDTH_80MHZ)) {
				PELOGE(sch_log(mac_ctx, LOGE,
				       FL("Updating the CH Width to 160MHz"));)
				pStaDs->vhtSupportedChannelWidthSet =
					WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ;
				pStaDs->htSupportedChannelWidthSet =
					eHT_CHANNEL_WIDTH_40MHZ;
				chWidth = eHT_CHANNEL_WIDTH_160MHZ;
			} else if (bcn->OperatingMode.chanWidth >=
				eHT_CHANNEL_WIDTH_80MHZ) {
				PELOGE(sch_log(mac_ctx, LOGE,
				       FL("Updating the CH Width to 80MHz"));)
				pStaDs->vhtSupportedChannelWidthSet =
					WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ;
				pStaDs->htSupportedChannelWidthSet =
					eHT_CHANNEL_WIDTH_40MHZ;
				chWidth = eHT_CHANNEL_WIDTH_80MHZ;
			} else if (bcn->OperatingMode.chanWidth ==
				eHT_CHANNEL_WIDTH_40MHZ) {
				PELOGE(sch_log(mac_ctx, LOGE,
				       FL("Updating the CH Width to 40MHz"));)
				pStaDs->vhtSupportedChannelWidthSet =
					WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ;
				pStaDs->htSupportedChannelWidthSet =
					eHT_CHANNEL_WIDTH_40MHZ;
				chWidth = eHT_CHANNEL_WIDTH_40MHZ;
			} else if (bcn->OperatingMode.chanWidth ==
				eHT_CHANNEL_WIDTH_20MHZ) {
				PELOGE(sch_log(mac_ctx, LOGE,
				       FL("Updating the CH Width to 20MHz"));)
				pStaDs->vhtSupportedChannelWidthSet =
					WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ;
				pStaDs->htSupportedChannelWidthSet =
					eHT_CHANNEL_WIDTH_20MHZ;
				chWidth = eHT_CHANNEL_WIDTH_20MHZ;
			}
			lim_check_vht_op_mode_change(mac_ctx, session,
					chWidth, pStaDs->staIndex, pMh->sa);
			update_nss(mac_ctx, pStaDs, bcn, session, pMh);
		}
		return;
	}

	if (!(session->vhtCapability && bcn->VHTOperation.present))
		return;

	operMode = pStaDs->vhtSupportedChannelWidthSet;
	if ((operMode == WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ) &&
	    (operMode < bcn->VHTOperation.chanWidth))
		skip_opmode_update = true;

	if (!skip_opmode_update &&
	    (operMode != bcn->VHTOperation.chanWidth)) {
		PELOGE(sch_log(mac_ctx, LOGE,
		       FL("received VHTOP CHWidth %d staIdx = %d"),
		       bcn->VHTOperation.chanWidth, pStaDs->staIndex);)
		PELOGE(sch_log(mac_ctx, LOGE,
		       FL(" MAC - %0x:%0x:%0x:%0x:%0x:%0x"),
		       pMh->sa[0], pMh->sa[1],
		       pMh->sa[2], pMh->sa[3],
		       pMh->sa[4], pMh->sa[5]);)

		if ((bcn->VHTOperation.chanWidth >=
			WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ) &&
			(fw_vht_ch_wd > eHT_CHANNEL_WIDTH_80MHZ)) {
			PELOGE(sch_log(mac_ctx, LOGE,
				FL("Updating the CH Width to 160MHz"));)
			pStaDs->vhtSupportedChannelWidthSet =
				bcn->VHTOperation.chanWidth;
			pStaDs->htSupportedChannelWidthSet =
				eHT_CHANNEL_WIDTH_40MHZ;
			chWidth = eHT_CHANNEL_WIDTH_160MHZ;
		} else if (bcn->VHTOperation.chanWidth >=
			WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ) {
			PELOGE(sch_log(mac_ctx, LOGE,
			       FL("Updating the CH Width to 80MHz"));)
			pStaDs->vhtSupportedChannelWidthSet =
				WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ;
			pStaDs->htSupportedChannelWidthSet =
				eHT_CHANNEL_WIDTH_40MHZ;
			chWidth = eHT_CHANNEL_WIDTH_80MHZ;
		} else if (bcn->VHTOperation.chanWidth ==
			WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ) {
			pStaDs->vhtSupportedChannelWidthSet =
				WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ;
			if (bcn->HTCaps.supportedChannelWidthSet) {
				PELOGE(sch_log(mac_ctx, LOGE,
				       FL("Updating the CH Width to 40MHz"));)
				pStaDs->htSupportedChannelWidthSet =
					eHT_CHANNEL_WIDTH_40MHZ;
				chWidth = eHT_CHANNEL_WIDTH_40MHZ;
			} else {
				PELOGE(sch_log(mac_ctx, LOGE,
				       FL("Updating the CH Width to 20MHz"));)
				pStaDs->htSupportedChannelWidthSet =
					eHT_CHANNEL_WIDTH_20MHZ;
				chWidth = eHT_CHANNEL_WIDTH_20MHZ;
			}
		}
		lim_check_vht_op_mode_change(mac_ctx, session, chWidth,
						pStaDs->staIndex, pMh->sa);
	}
	return;
}

/*
 * __sch_beacon_process_for_session() - Process the received beacon frame when
 * station is not scanning and corresponding session is found
 *
 *
 * @mac_ctx:        mac_ctx
 * @bcn:            beacon struct
 * @rx_pkt_info:    received packet info
 * @session:        pe session pointer
 *
 * Following scenarios exist when Session exists
 *   IBSS STA receving beacons from IBSS Peers, who are part of IBSS.
 *     - call lim_handle_ibs_scoalescing with that session context.
 *   Infra STA receving beacons from AP to which it is connected
 *     - call sch_beacon_processFromAP with that session's context.
 *   BTAMP STA receving beacons from BTAMP AP
 *     - call sch_beacon_processFromAP with that session's context.
 *   BTAMP AP receiving beacons from BTAMP STA
 *     (here need to make sure BTAP creates session entry for BT STA)
 *     - just update the beacon count for heart beat purposes for now,
 *       for now, don't process the beacon.
 *   Infra/IBSS both active and receives IBSS beacon:
 *     - call lim_handle_ibs_scoalescing with that session context.
 *   Infra/IBSS both active and receives Infra beacon:
 *     - call sch_beacon_processFromAP with that session's context.
 *        any updates to EDCA parameters will be effective for IBSS as well,
 *        even though no WMM for IBSS ?? Need to figure out how to handle
 *        this scenario.
 *   Infra/BTSTA both active and receive Infra beacon.
 *     - change in EDCA parameters on Infra affect the BTSTA link.
 *        Update the same parameters on BT link
 *   Infra/BTSTA both active and receive BT-AP beacon.
 *     - update beacon cnt for heartbeat
 *   Infra/BTAP both active and receive Infra beacon.
 *     - BT-AP starts advertising BE parameters from Infra AP, if they get
 *       changed.
 *   Infra/BTAP both active and receive BTSTA beacon.
 *       - update beacon cnt for heartbeat
 *
 * Return: void
 */
static void __sch_beacon_process_for_session(tpAniSirGlobal mac_ctx,
					     tpSchBeaconStruct bcn,
					     uint8_t *rx_pkt_info,
					     tpPESession session)
{
	tPowerdBm localRRMConstraint = 0;
	uint8_t bssIdx = 0;
	tUpdateBeaconParams beaconParams;
	uint8_t sendProbeReq = false;
#ifdef WLAN_FEATURE_11AC
	tpSirMacMgmtHdr pMh = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
#endif
#if defined FEATURE_WLAN_ESE || defined WLAN_FEATURE_VOWIFI
	tPowerdBm regMax = 0, maxTxPower = 0;
#endif
	cdf_mem_zero(&beaconParams, sizeof(tUpdateBeaconParams));
	beaconParams.paramChangeBitmap = 0;

	if (LIM_IS_IBSS_ROLE(session)) {
		lim_handle_ibss_coalescing(mac_ctx, bcn, rx_pkt_info, session);
	} else if (LIM_IS_STA_ROLE(session)
	    || LIM_IS_BT_AMP_STA_ROLE(session)) {
		if (false == sch_bcn_process_sta_bt_amp_sta(mac_ctx, bcn,
				rx_pkt_info, session, &bssIdx,
				&beaconParams, &sendProbeReq, pMh))
			return;
	}

	if (session->htCapability && bcn->HTInfo.present)
		lim_update_sta_run_time_ht_switch_chnl_params(mac_ctx,
						&bcn->HTInfo, bssIdx, session);

	if (LIM_IS_STA_ROLE(session)
	    || LIM_IS_BT_AMP_STA_ROLE(session)
	    || LIM_IS_IBSS_ROLE(session)) {
		/* Channel Switch information element updated */
		if (bcn->channelSwitchPresent) {
			/*
			 * on receiving channel switch announcement from AP,
			 * delete all TDLS peers before leaving BSS and proceed
			 * for channel switch
			 */
			if (LIM_IS_STA_ROLE(session))
				lim_delete_tdls_peers(mac_ctx, session);

			lim_update_channel_switch(mac_ctx, bcn, session);
		} else if (session->gLimSpecMgmt.dot11hChanSwState ==
				eLIM_11H_CHANSW_RUNNING) {
			lim_cancel_dot11h_channel_switch(mac_ctx, session);
		}
	}
#ifdef WLAN_FEATURE_11AC
	if (LIM_IS_STA_ROLE(session)
	    || LIM_IS_BT_AMP_STA_ROLE(session)
	    || LIM_IS_IBSS_ROLE(session))
		sch_bcn_process_sta_bt_amp_sta_ibss(mac_ctx, bcn,
					rx_pkt_info, session, &bssIdx,
					&beaconParams, &sendProbeReq, pMh);
#endif

#if defined (FEATURE_WLAN_ESE) || defined (WLAN_FEATURE_VOWIFI)
	/* Obtain the Max Tx power for the current regulatory  */
	regMax = cfg_get_regulatory_max_transmit_power(mac_ctx,
					session->currentOperChannel);
#endif

#if defined WLAN_FEATURE_VOWIFI
	if (mac_ctx->rrm.rrmPEContext.rrmEnable
	    && bcn->powerConstraintPresent)
		localRRMConstraint =
			bcn->localPowerConstraint.localPowerConstraints;
	else
		localRRMConstraint = 0;
	maxTxPower = lim_get_max_tx_power(regMax, regMax - localRRMConstraint,
					mac_ctx->roam.configParam.nTxPowerCap);
#elif defined FEATURE_WLAN_ESE
	maxTxPower = regMax;
#endif

#if defined FEATURE_WLAN_ESE
	if (session->isESEconnection) {
		tPowerdBm localESEConstraint = 0;
		if (bcn->eseTxPwr.present) {
			localESEConstraint = bcn->eseTxPwr.power_limit;
			maxTxPower = lim_get_max_tx_power(maxTxPower,
					localESEConstraint,
					mac_ctx->roam.configParam.nTxPowerCap);
		}
		sch_log(mac_ctx, LOG1,
			FL("RegMax = %d, localEseCons = %d, MaxTx = %d"),
			regMax, localESEConstraint, maxTxPower);
	}
#endif

#if defined (FEATURE_WLAN_ESE) || defined (WLAN_FEATURE_VOWIFI)
	/* If maxTxPower is increased or decreased */
	if (maxTxPower != session->maxTxPower) {
		sch_log(mac_ctx, LOG1,
			FL("Local power constraint change..updating new maxTx power %d to HAL"),
			maxTxPower);
		if (lim_send_set_max_tx_power_req(mac_ctx, maxTxPower, session)
		    == eSIR_SUCCESS)
			session->maxTxPower = maxTxPower;
	}
#endif

	/* Indicate to LIM that Beacon is received */
	if (bcn->HTInfo.present)
		lim_received_hb_handler(mac_ctx,
				(uint8_t) bcn->HTInfo.primaryChannel, session);
	else
		lim_received_hb_handler(mac_ctx, (uint8_t) bcn->channelNumber,
				session);

	/*
	 * I don't know if any additional IE is required here. Currently, not
	 * include addIE.
	 */
	if (sendProbeReq)
		lim_send_probe_req_mgmt_frame(mac_ctx, &session->ssId,
			session->bssId, session->currentOperChannel,
			session->selfMacAddr, session->dot11mode, 0, NULL);

	if ((false == mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running)
	    && beaconParams.paramChangeBitmap) {
		PELOGW(sch_log(mac_ctx, LOGW,
		       FL("Beacon for session[%d] got changed."),
		       session->peSessionId);)
		PELOGW(sch_log(mac_ctx, LOGW,
		       FL("sending beacon param change bitmap: 0x%x "),
		       beaconParams.paramChangeBitmap);)
		lim_send_beacon_params(mac_ctx, &beaconParams, session);
	}
}

/**
 * sch_beacon_process() - process the beacon frame
 *
 * @mac_ctx:        mac global context
 * @rx_pkt_info:  pointer to buffer descriptor
 *
 * @return None
 */
void
sch_beacon_process(tpAniSirGlobal mac_ctx, uint8_t *rx_pkt_info,
		   tpPESession session)
{
	static tSchBeaconStruct bcn;
	tUpdateBeaconParams bcn_prm;
	tpPESession ap_session = NULL;
#ifdef WLAN_FEATURE_MBSSID
	uint8_t i;
#endif

	cdf_mem_zero(&bcn_prm, sizeof(tUpdateBeaconParams));
	bcn_prm.paramChangeBitmap = 0;
	mac_ctx->sch.gSchBcnRcvCnt++;
	/* Convert the beacon frame into a structure */
	if (sir_convert_beacon_frame2_struct(mac_ctx, (uint8_t *) rx_pkt_info,
		&bcn) != eSIR_SUCCESS) {
		PELOGE(sch_log(mac_ctx, LOGE, FL("beacon parsing failed"));)
		mac_ctx->sch.gSchBcnParseErrorCnt++;
		return;
	}

	if (bcn.ssidPresent)
		bcn.ssId.ssId[bcn.ssId.length] = 0;
	/*
	 * First process the beacon in the context of any existing AP or BTAP
	 * session. This takes cares of following two scenarios:
	 *  - session = NULL:
	 * e.g. beacon received from a neighboring BSS, you want to apply the
	 * protection settings to BTAP/InfraAP beacons
	 *  - session is non NULL:
	 * e.g. beacon received is from the INFRA AP to which you are connected
	 * on another concurrent link. In this case also, we want to apply the
	 * protection settings(as advertised by Infra AP) to BTAP beacons
	 */
#ifdef WLAN_FEATURE_MBSSID
	for (i = 0; i < mac_ctx->lim.maxBssId; i++) {
		ap_session = pe_find_session_by_session_id(mac_ctx, i);
		if (!((ap_session != NULL) &&
			(!(WMA_GET_OFFLOADSCANLEARN(rx_pkt_info)))))
			continue;

		if (!LIM_IS_AP_ROLE(ap_session))
			continue;

		bcn_prm.bssIdx = ap_session->bssIdx;
		if (ap_session->gLimProtectionControl !=
		    WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
			ap_beacon_process(mac_ctx, rx_pkt_info,
					  &bcn, &bcn_prm, ap_session);

		if ((false == mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running)
		    && bcn_prm.paramChangeBitmap) {
			/* Update the bcn and apply the new settings to HAL */
			sch_set_fixed_beacon_fields(mac_ctx, ap_session);
			PELOG1(sch_log(mac_ctx, LOG1,
			       FL("Beacon for PE session[%d] got changed."),
			       ap_session->peSessionId);)
			PELOG1(sch_log(mac_ctx, LOG1,
			       FL("sending beacon param change bitmap: 0x%x"),
			       bcn_prm.paramChangeBitmap);)
			lim_send_beacon_params(mac_ctx, &bcn_prm, ap_session);
		}
	}
#else
	ap_session = lim_is_ap_session_active(mac_ctx);
	if ((ap_session != NULL)
	    && (!(WMA_GET_OFFLOADSCANLEARN(rx_pkt_info)))) {
		bcn_prm.bssIdx = ap_session->bssIdx;
		if (ap_session->gLimProtectionControl !=
		    WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
			ap_beacon_process(mac_ctx, rx_pkt_info, &bcn,
					  &bcn_prm, ap_session);

		if ((false == mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running)
		    && bcn_prm.paramChangeBitmap) {
			/* Update the bcn and apply the new settings to HAL */
			sch_set_fixed_beacon_fields(mac_ctx, ap_session);
			PELOG1(sch_log(mac_ctx, LOG1,
			       FL("Beacon for PE session[%d] got changed."),
			       ap_session->peSessionId);)
			PELOG1(sch_log(mac_ctx, LOG1,
			       FL("sending beacon param change bitmap: 0x%x "),
			       bcn_prm.paramChangeBitmap);)
			lim_send_beacon_params(mac_ctx, &bcn_prm, ap_session);
		}
	}
#endif
	/*
	 * Now process the beacon in the context of the BSS which is
	 * transmitting the beacons, if one is found
	 */
	if (session == NULL)
		__sch_beacon_process_no_session(mac_ctx, &bcn, rx_pkt_info);
	else
		__sch_beacon_process_for_session(mac_ctx, &bcn, rx_pkt_info,
						 session);
}

/**
 * sch_beacon_edca_process(): Process the EDCA parameter set in the received
 * beacon frame
 *
 * @mac_ctx:    mac global context
 * @edca:       reference to edca parameters in beacon struct
 * @session :   pesession entry
 *
 * @return status of operation
 */
tSirRetStatus
sch_beacon_edca_process(tpAniSirGlobal pMac, tSirMacEdcaParamSetIE *edca,
			tpPESession session)
{
	uint8_t i;
#ifdef FEATURE_WLAN_DIAG_SUPPORT
	host_log_qos_edca_pkt_type *log_ptr = NULL;
#endif /* FEATURE_WLAN_DIAG_SUPPORT */

	PELOG1(sch_log(pMac, LOG1,
		FL("Updating parameter set count: Old %d ---> new %d"),
		session->gLimEdcaParamSetCount, edca->qosInfo.count);)

	session->gLimEdcaParamSetCount = edca->qosInfo.count;
	session->gLimEdcaParams[EDCA_AC_BE] = edca->acbe;
	session->gLimEdcaParams[EDCA_AC_BK] = edca->acbk;
	session->gLimEdcaParams[EDCA_AC_VI] = edca->acvi;
	session->gLimEdcaParams[EDCA_AC_VO] = edca->acvo;
#ifdef FEATURE_WLAN_DIAG_SUPPORT
	WLAN_HOST_DIAG_LOG_ALLOC(log_ptr, host_log_qos_edca_pkt_type,
				 LOG_WLAN_QOS_EDCA_C);
	if (log_ptr) {
		log_ptr->aci_be = session->gLimEdcaParams[EDCA_AC_BE].aci.aci;
		log_ptr->cw_be =
			session->gLimEdcaParams[EDCA_AC_BE].cw.max << 4
				| session->gLimEdcaParams[EDCA_AC_BE].cw.min;
		log_ptr->txoplimit_be =
			session->gLimEdcaParams[EDCA_AC_BE].txoplimit;
		log_ptr->aci_bk =
			session->gLimEdcaParams[EDCA_AC_BK].aci.aci;
		log_ptr->cw_bk =
			session->gLimEdcaParams[EDCA_AC_BK].cw.max << 4
				| session->gLimEdcaParams[EDCA_AC_BK].cw.min;
		log_ptr->txoplimit_bk =
			session->gLimEdcaParams[EDCA_AC_BK].txoplimit;
		log_ptr->aci_vi =
			session->gLimEdcaParams[EDCA_AC_VI].aci.aci;
		log_ptr->cw_vi =
			session->gLimEdcaParams[EDCA_AC_VI].cw.max << 4
				| session->gLimEdcaParams[EDCA_AC_VI].cw.min;
		log_ptr->txoplimit_vi =
			session->gLimEdcaParams[EDCA_AC_VI].txoplimit;
		log_ptr->aci_vo =
			session->gLimEdcaParams[EDCA_AC_VO].aci.aci;
		log_ptr->cw_vo =
			session->gLimEdcaParams[EDCA_AC_VO].cw.max << 4
				| session->gLimEdcaParams[EDCA_AC_VO].cw.min;
		log_ptr->txoplimit_vo =
			session->gLimEdcaParams[EDCA_AC_VO].txoplimit;
	}
	WLAN_HOST_DIAG_LOG_REPORT(log_ptr);
#endif /* FEATURE_WLAN_DIAG_SUPPORT */
	PELOG1(sch_log(pMac, LOGE,
	       FL("Updating Local EDCA Params(gLimEdcaParams) to: "));)
	for (i = 0; i < MAX_NUM_AC; i++) {
		PELOG1(sch_log(pMac, LOG1,
		       FL("AC[%d]:  AIFSN: %d, ACM %d, CWmin %d, CWmax %d, TxOp %d"),
		       i, session->gLimEdcaParams[i].aci.aifsn,
		       session->gLimEdcaParams[i].aci.acm,
		       session->gLimEdcaParams[i].cw.min,
		       session->gLimEdcaParams[i].cw.max,
		       session->gLimEdcaParams[i].txoplimit);)
	}
	return eSIR_SUCCESS;
}
