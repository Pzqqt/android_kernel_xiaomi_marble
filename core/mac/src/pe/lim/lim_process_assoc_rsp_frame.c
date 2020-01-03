/*
 * Copyright (c) 2011-2020 The Linux Foundation. All rights reserved.
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
#include "sch_api.h"

#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_security_utils.h"
#include "lim_ser_des_utils.h"
#include "lim_send_messages.h"
#include "lim_process_fils.h"

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
static void lim_update_stads_htcap(struct mac_context *mac_ctx,
		tpDphHashNode sta_ds, tpSirAssocRsp assoc_rsp,
		struct pe_session *session_entry)
{
	uint16_t highest_rxrate = 0;
	tDot11fIEHTCaps *ht_caps;

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

		/* Check if we have support for gShortGI20Mhz and
		 * gShortGI40Mhz from ini file
		 */
		if (session_entry->ht_config.ht_sgi20)
			sta_ds->htShortGI20Mhz =
			      (uint8_t)assoc_rsp->HTCaps.shortGI20MHz;
		else
			sta_ds->htShortGI20Mhz = false;

		if (session_entry->ht_config.ht_sgi40)
			sta_ds->htShortGI40Mhz =
				      (uint8_t)assoc_rsp->HTCaps.shortGI40MHz;
		else
			sta_ds->htShortGI40Mhz = false;
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
void lim_update_assoc_sta_datas(struct mac_context *mac_ctx,
	tpDphHashNode sta_ds, tpSirAssocRsp assoc_rsp,
	struct pe_session *session_entry)
{
	uint32_t phy_mode;
	bool qos_mode;
	tDot11fIEVHTCaps *vht_caps = NULL;
	tDot11fIEhe_cap *he_cap = NULL;

	lim_get_phy_mode(mac_ctx, &phy_mode, session_entry);
	sta_ds->staType = STA_ENTRY_SELF;
	limGetQosMode(session_entry, &qos_mode);
	sta_ds->mlmStaContext.authType = session_entry->limCurrentAuthType;

	/* Add capabilities information, rates and AID */
	sta_ds->mlmStaContext.capabilityInfo = assoc_rsp->capabilityInfo;
	sta_ds->shortPreambleEnabled =
		(uint8_t) assoc_rsp->capabilityInfo.shortPreamble;

	/* Update HT Capabilities only when the self mode supports HT */
	if (IS_DOT11_MODE_HT(session_entry->dot11mode))
		lim_update_stads_htcap(mac_ctx, sta_ds, assoc_rsp,
				       session_entry);

	if (assoc_rsp->VHTCaps.present)
		vht_caps = &assoc_rsp->VHTCaps;
	else if (assoc_rsp->vendor_vht_ie.VHTCaps.present)
		vht_caps = &assoc_rsp->vendor_vht_ie.VHTCaps;

	if ((IS_DOT11_MODE_VHT(session_entry->dot11mode)) &&
	    ((vht_caps) && vht_caps->present)) {
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
		sta_ds->vht_mcs_10_11_supp = 0;
		if (mac_ctx->mlme_cfg->vht_caps.vht_cap_info.
		    vht_mcs_10_11_supp &&
		    assoc_rsp->qcn_ie.present &&
		    assoc_rsp->qcn_ie.vht_mcs11_attr.present)
			sta_ds->vht_mcs_10_11_supp =
				assoc_rsp->qcn_ie.vht_mcs11_attr.
				vht_mcs_10_11_supp;
	}

	if (IS_DOT11_MODE_HE(session_entry->dot11mode))
		lim_update_stads_he_caps(sta_ds, assoc_rsp, session_entry);

	if (lim_is_sta_he_capable(sta_ds))
		he_cap = &assoc_rsp->he_cap;

	if (lim_populate_peer_rate_set(mac_ctx, &sta_ds->supportedRates,
				assoc_rsp->HTCaps.supportedMCSSet,
				false, session_entry,
				vht_caps, he_cap, sta_ds) !=
				QDF_STATUS_SUCCESS) {
		pe_err("could not get rateset and extended rate set");
		return;
	}
	sta_ds->vhtSupportedRxNss =
		((sta_ds->supportedRates.vhtRxMCSMap & MCSMAPMASK2x2)
		 == MCSMAPMASK2x2) ? 1 : 2;

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
			QDF_STATUS status;

			status =
				sch_beacon_edca_process(mac_ctx,
					&assoc_rsp->edca, session_entry);
			pe_debug("Edca set update based on AssocRsp: status %d",
				status);
			if (status != QDF_STATUS_SUCCESS) {
				pe_err("Edca error in AssocResp");
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
		QDF_STATUS status;

		status = sch_beacon_edca_process(mac_ctx, &assoc_rsp->edca,
				session_entry);
		pe_debug("WME Edca set update based on AssocRsp: status %d",
			status);

		if (status != QDF_STATUS_SUCCESS)
			pe_err("WME Edca error in AssocResp - ignoring");

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
static void lim_update_ric_data(struct mac_context *mac_ctx,
	 struct pe_session *session_entry, tpSirAssocRsp assoc_rsp)
{
	if (session_entry->ricData) {
		qdf_mem_free(session_entry->ricData);
		session_entry->ricData = NULL;
		session_entry->RICDataLen = 0;
	}
	if (assoc_rsp->ricPresent) {
		session_entry->RICDataLen =
			assoc_rsp->num_RICData * sizeof(tDot11fIERICDataDesc);
		if (session_entry->RICDataLen) {
			session_entry->ricData =
				qdf_mem_malloc(session_entry->RICDataLen);
			if (!session_entry->ricData)
				session_entry->RICDataLen = 0;
			else
				qdf_mem_copy(session_entry->ricData,
					&assoc_rsp->RICData[0],
					session_entry->RICDataLen);
		} else {
			pe_err("RIC data not present");
		}
	} else {
		pe_debug("Ric is not present");
		session_entry->RICDataLen = 0;
		session_entry->ricData = NULL;
	}
	return;
}

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
static void lim_update_ese_tspec(struct mac_context *mac_ctx,
	 struct pe_session *session_entry, tpSirAssocRsp assoc_rsp)
{
	if (session_entry->tspecIes) {
		qdf_mem_free(session_entry->tspecIes);
		session_entry->tspecIes = NULL;
		session_entry->tspecLen = 0;
	}
	if (assoc_rsp->tspecPresent) {
		pe_debug("Tspec EID present in assoc rsp");
		session_entry->tspecLen =
			assoc_rsp->num_tspecs * sizeof(tDot11fIEWMMTSPEC);
		if (session_entry->tspecLen) {
			session_entry->tspecIes =
				qdf_mem_malloc(session_entry->tspecLen);
			if (!session_entry->tspecIes)
				session_entry->tspecLen = 0;
			else
				qdf_mem_copy(session_entry->tspecIes,
						&assoc_rsp->TSPECInfo[0],
						session_entry->tspecLen);
		} else {
			pe_err("TSPEC has Zero length");
		}
	} else {
		session_entry->tspecLen = 0;
		session_entry->tspecIes = NULL;
		pe_debug("Tspec EID *NOT* present in assoc rsp");
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
static void lim_update_ese_tsm(struct mac_context *mac_ctx,
	 struct pe_session *session_entry, tpSirAssocRsp assoc_rsp)
{
	uint8_t cnt = 0;
	tpEseTSMContext tsm_ctx;

	pe_debug("TSM IE Present in Reassoc Rsp");
	/*
	 * Start the TSM  timer only if the TSPEC
	 * Ie is present in the reassoc rsp
	 */
	if (!assoc_rsp->tspecPresent) {
		pe_debug("TSM present but TSPEC IE not present");
		return;
	}
	tsm_ctx = &session_entry->eseContext.tsm;
	/* Find the TSPEC IE with VO user priority */
	for (cnt = 0; cnt < assoc_rsp->num_tspecs; cnt++) {
		if (upToAc(assoc_rsp->TSPECInfo[cnt].user_priority) ==
			QCA_WLAN_AC_VO) {
			tsm_ctx->tid =
				assoc_rsp->TSPECInfo[cnt].user_priority;
			qdf_mem_copy(&tsm_ctx->tsmInfo,
				&assoc_rsp->tsmIE, sizeof(struct ese_tsm_ie));
			lim_send_sme_tsm_ie_ind(mac_ctx,
				session_entry, assoc_rsp->tsmIE.tsid,
				assoc_rsp->tsmIE.state,
				assoc_rsp->tsmIE.msmt_interval);
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
static void lim_update_stads_ext_cap(struct mac_context *mac_ctx,
	struct pe_session *session_entry, tpSirAssocRsp assoc_rsp,
	tpDphHashNode sta_ds)
{
	struct s_ext_cap *ext_cap;

	if (!assoc_rsp->ExtCap.present) {
		sta_ds->timingMeasCap = 0;
#ifdef FEATURE_WLAN_TDLS
		session_entry->tdls_prohibited = false;
		session_entry->tdls_chan_swit_prohibited = false;
#endif
		pe_debug("ExtCap not present");
		return;
	}

	ext_cap = (struct s_ext_cap *)assoc_rsp->ExtCap.bytes;
	lim_set_stads_rtt_cap(sta_ds, ext_cap, mac_ctx);
#ifdef FEATURE_WLAN_TDLS
	session_entry->tdls_prohibited = ext_cap->tdls_prohibited;
	session_entry->tdls_chan_swit_prohibited =
		ext_cap->tdls_chan_swit_prohibited;
	pe_debug("ExtCap: tdls_prohibited: %d tdls_chan_swit_prohibited: %d",
		ext_cap->tdls_prohibited,
		ext_cap->tdls_chan_swit_prohibited);
#endif
	lim_set_peer_twt_cap(session_entry, ext_cap);
}

/**
 * lim_stop_reassoc_retry_timer() - Cleanup after reassoc response is received
 *  @mac_ctx: Global MAC context
 *
 *  Stop the reassoc retry timer and release the stored reassoc request.
 *
 *  Return: None
 */
static void lim_stop_reassoc_retry_timer(struct mac_context *mac_ctx)
{
	mac_ctx->lim.reAssocRetryAttempt = 0;
	if ((mac_ctx->lim.pe_session)
		&& (NULL !=
			mac_ctx->lim.pe_session->pLimMlmReassocRetryReq)) {
		qdf_mem_free(
			mac_ctx->lim.pe_session->pLimMlmReassocRetryReq);
		mac_ctx->lim.pe_session->pLimMlmReassocRetryReq = NULL;
	}
	lim_deactivate_and_change_timer(mac_ctx, eLIM_REASSOC_FAIL_TIMER);
}

#ifdef WLAN_FEATURE_11W
static void
lim_handle_assoc_reject_status(struct mac_context *mac_ctx,
				struct pe_session *session_entry,
				tpSirAssocRsp assoc_rsp,
				tSirMacAddr source_addr)
{
	struct sir_rssi_disallow_lst ap_info = {{0}};
	uint32_t timeout_value =
		assoc_rsp->TimeoutInterval.timeoutValue;

	if (!(session_entry->limRmfEnabled &&
		    assoc_rsp->status_code == eSIR_MAC_TRY_AGAIN_LATER &&
		   (assoc_rsp->TimeoutInterval.present &&
			(assoc_rsp->TimeoutInterval.timeoutType ==
				SIR_MAC_TI_TYPE_ASSOC_COMEBACK))))
		return;

	/*
	 * Add to rssi reject list, which takes care of retry
	 * delay too. Fill the RSSI as 0, so the only param
	 * which will allow the bssid to connect is retry delay.
	 */
	ap_info.retry_delay = timeout_value;
	qdf_mem_copy(ap_info.bssid.bytes, source_addr, QDF_MAC_ADDR_SIZE);
	ap_info.expected_rssi = LIM_MIN_RSSI;
	lim_add_bssid_to_reject_list(mac_ctx->pdev, &ap_info);

	pe_debug("ASSOC res with eSIR_MAC_TRY_AGAIN_LATER recvd. Add to time reject list(rssi reject in mac_ctx %d",
		timeout_value);
}
#else
static void
lim_handle_assoc_reject_status(struct mac_context *mac_ctx,
				struct pe_session *session_entry,
				tpSirAssocRsp assoc_rsp,
				tSirMacAddr source_addr)
{
}
#endif

/**
 * lim_get_nss_supported_by_ap() - finds out nss from AP's beacons
 * @vht_caps: VHT capabilities
 * @ht_caps: HT capabilities
 *
 * Return: nss advertised by AP in beacon
 */
static uint8_t lim_get_nss_supported_by_ap(tDot11fIEVHTCaps *vht_caps,
					   tDot11fIEHTCaps *ht_caps,
					   tDot11fIEhe_cap *he_caps)
{

	if (he_caps->present) {
		if ((he_caps->rx_he_mcs_map_lt_80 & 0xC0) != 0xC0)
			return NSS_4x4_MODE;

		if ((he_caps->rx_he_mcs_map_lt_80 & 0x30) != 0x30)
			return NSS_3x3_MODE;

		if ((he_caps->rx_he_mcs_map_lt_80 & 0x0C) != 0x0C)
			return NSS_2x2_MODE;
	} else if (vht_caps->present) {
		if ((vht_caps->rxMCSMap & 0xC0) != 0xC0)
			return NSS_4x4_MODE;

		if ((vht_caps->rxMCSMap & 0x30) != 0x30)
			return NSS_3x3_MODE;

		if ((vht_caps->rxMCSMap & 0x0C) != 0x0C)
			return NSS_2x2_MODE;
	} else if (ht_caps->present) {
		if (ht_caps->supportedMCSSet[3])
			return NSS_4x4_MODE;

		if (ht_caps->supportedMCSSet[2])
			return NSS_3x3_MODE;

		if (ht_caps->supportedMCSSet[1])
			return NSS_2x2_MODE;
	}

	return NSS_1x1_MODE;
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
lim_process_assoc_rsp_frame(struct mac_context *mac_ctx,
	uint8_t *rx_pkt_info, uint8_t subtype, struct pe_session *session_entry)
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
	uint8_t vdev_id = 0;
	struct csr_roam_session *roam_session;
#endif
	uint8_t ap_nss;

	/* Initialize status code to success. */
	if (lim_is_roam_synch_in_progress(session_entry))
		hdr = (tpSirMacMgmtHdr) mac_ctx->roam.pReassocResp;
	else
		hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	vdev_id = session_entry->vdev_id;
#endif
	assoc_cnf.resultCode = eSIR_SME_SUCCESS;
	/* Update PE session Id */
	assoc_cnf.sessionId = session_entry->peSessionId;
	if (!hdr) {
		pe_err("LFR3: Reassoc response packet header is NULL");
		return;
	}

	pe_debug("received Re/Assoc: %d resp on sessionid: %d systemrole: %d"
		" and mlmstate: %d RSSI: %d from "QDF_MAC_ADDR_STR, subtype,
		session_entry->peSessionId, GET_LIM_SYSTEM_ROLE(session_entry),
		session_entry->limMlmState,
		(uint) abs((int8_t) WMA_GET_RX_RSSI_NORMALIZED(rx_pkt_info)),
		QDF_MAC_ADDR_ARRAY(hdr->sa));

	beacon = qdf_mem_malloc(sizeof(tSchBeaconStruct));
	if (!beacon)
		return;

	if (LIM_IS_AP_ROLE(session_entry)) {
		/*
		 * Should not have received Re/Association
		 * Response frame on AP. Log error
		 */
		pe_err("Should not received Re/Assoc Response in role: %d",
			GET_LIM_SYSTEM_ROLE(session_entry));
		qdf_mem_free(beacon);
		return;
	}
	if (lim_is_roam_synch_in_progress(session_entry)) {
		hdr = (tpSirMacMgmtHdr) mac_ctx->roam.pReassocResp;
		frame_len = mac_ctx->roam.reassocRespLen - SIR_MAC_HDR_LEN_3A;
	} else {
		hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
		frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);
	}
	if (((subtype == LIM_ASSOC) &&
		(session_entry->limMlmState != eLIM_MLM_WT_ASSOC_RSP_STATE)) ||
		((subtype == LIM_REASSOC) &&
		 !lim_is_roam_synch_in_progress(session_entry) &&
		((session_entry->limMlmState != eLIM_MLM_WT_REASSOC_RSP_STATE)
		&& (session_entry->limMlmState !=
		eLIM_MLM_WT_FT_REASSOC_RSP_STATE)
		))) {
		/* Received unexpected Re/Association Response frame */
		pe_debug("Received Re/Assoc rsp in unexpected state: %d on session: %d",
			session_entry->limMlmState, session_entry->peSessionId);
		if (!hdr->fc.retry) {
			if (!(mac_ctx->lim.retry_packet_cnt & 0xf)) {
				pe_err("recvd Re/Assoc rsp:not a retry frame");
				lim_print_mlm_state(mac_ctx, LOGE,
						session_entry->limMlmState);
			} else {
				mac_ctx->lim.retry_packet_cnt++;
			}
		}
		qdf_mem_free(beacon);
		return;
	}
	sir_copy_mac_addr(current_bssid, session_entry->bssId);
	if (subtype == LIM_ASSOC) {
		if (qdf_mem_cmp
			(hdr->sa, current_bssid, sizeof(tSirMacAddr))) {
			/*
			 * Received Association Response frame from an entity
			 * other than one to which request was initiated.
			 * Ignore this and wait until Assoc Failure Timeout
			 */
			pe_warn("received AssocRsp from unexpected peer "QDF_MAC_ADDR_STR,
				QDF_MAC_ADDR_ARRAY(hdr->sa));
			qdf_mem_free(beacon);
			return;
		}
	} else {
		if (qdf_mem_cmp
			(hdr->sa, session_entry->limReAssocbssId,
			sizeof(tSirMacAddr))) {
			/*
			 * Received Reassociation Response frame from an entity
			 * other than one to which request was initiated.
			 * Ignore this and wait until Reassoc Failure Timeout.
			 */
			pe_warn("received ReassocRsp from unexpected peer "QDF_MAC_ADDR_STR,
				QDF_MAC_ADDR_ARRAY(hdr->sa));
			qdf_mem_free(beacon);
			return;
		}
	}

	assoc_rsp = qdf_mem_malloc(sizeof(*assoc_rsp));
	if (!assoc_rsp) {
		qdf_mem_free(beacon);
		return;
	}
	/* Get pointer to Re/Association Response frame body */
	if (lim_is_roam_synch_in_progress(session_entry))
		body = mac_ctx->roam.pReassocResp + SIR_MAC_HDR_LEN_3A;
	else
		body = WMA_GET_RX_MPDU_DATA(rx_pkt_info);
	/* parse Re/Association Response frame. */
	if (sir_convert_assoc_resp_frame2_struct(mac_ctx, session_entry, body,
		frame_len, assoc_rsp) == QDF_STATUS_E_FAILURE) {
		qdf_mem_free(assoc_rsp);
		pe_err("Parse error Assoc resp subtype: %d" "length: %d",
			frame_len, subtype);
		qdf_mem_free(beacon);
		return;
	}

	if (!assoc_rsp->suppRatesPresent) {
		pe_debug("assoc response does not have supported rate set");
		qdf_mem_copy(&assoc_rsp->supportedRates,
			&session_entry->rateSet,
			sizeof(tSirMacRateSet));
	}

	assoc_cnf.protStatusCode = assoc_rsp->status_code;
	if (session_entry->assocRsp) {
		pe_warn("session_entry->assocRsp is not NULL freeing it and setting NULL");
		qdf_mem_free(session_entry->assocRsp);
		session_entry->assocRsp = NULL;
		session_entry->assocRspLen = 0;
	}

	if (frame_len) {
		session_entry->assocRsp = qdf_mem_malloc(frame_len);
		if (session_entry->assocRsp) {
			/*
			 * Store the Assoc response. This is sent
			 * to csr/hdd in join cnf response.
			 */
			qdf_mem_copy(session_entry->assocRsp, body, frame_len);
			session_entry->assocRspLen = frame_len;
		}
	}

	lim_update_ric_data(mac_ctx, session_entry, assoc_rsp);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	roam_session =
		&mac_ctx->roam.roamSession[vdev_id];
	if (assoc_rsp->sha384_ft_subelem.r0kh_id.present) {
		roam_session->ftSmeContext.r0kh_id_len =
			assoc_rsp->sha384_ft_subelem.r0kh_id.num_PMK_R0_ID;
		qdf_mem_copy(roam_session->ftSmeContext.r0kh_id,
			     assoc_rsp->sha384_ft_subelem.r0kh_id.PMK_R0_ID,
			     roam_session->ftSmeContext.r0kh_id_len);
	} else if (assoc_rsp->FTInfo.R0KH_ID.present) {
		roam_session->ftSmeContext.r0kh_id_len =
			assoc_rsp->FTInfo.R0KH_ID.num_PMK_R0_ID;
		qdf_mem_copy(roam_session->ftSmeContext.r0kh_id,
			assoc_rsp->FTInfo.R0KH_ID.PMK_R0_ID,
			roam_session->ftSmeContext.r0kh_id_len);
	} else {
		roam_session->ftSmeContext.r0kh_id_len = 0;
		qdf_mem_zero(roam_session->ftSmeContext.r0kh_id,
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
		pe_err("received Re/AssocRsp frame with IBSS capability");
		qdf_mem_free(assoc_rsp);
		qdf_mem_free(beacon);
		return;
	}

	if (lim_get_capability_info(mac_ctx, &caps, session_entry)
		!= QDF_STATUS_SUCCESS) {
		qdf_mem_free(assoc_rsp);
		qdf_mem_free(beacon);
		pe_err("could not retrieve Capabilities");
		return;
	}
	lim_copy_u16((uint8_t *) &mac_capab, caps);

	/* Stop Association failure timer */
	if (subtype == LIM_ASSOC)
		lim_deactivate_and_change_timer(mac_ctx, eLIM_ASSOC_FAIL_TIMER);
	else
		lim_stop_reassoc_retry_timer(mac_ctx);

	lim_handle_assoc_reject_status(mac_ctx, session_entry, assoc_rsp,
				       hdr->sa);

	if (eSIR_MAC_XS_FRAME_LOSS_POOR_CHANNEL_RSSI_STATUS ==
	   assoc_rsp->status_code &&
	    assoc_rsp->rssi_assoc_rej.present) {
		struct sir_rssi_disallow_lst ap_info = {{0}};

		ap_info.retry_delay = assoc_rsp->rssi_assoc_rej.retry_delay *
							QDF_MC_TIMER_TO_MS_UNIT;
		qdf_mem_copy(ap_info.bssid.bytes, hdr->sa, QDF_MAC_ADDR_SIZE);
		ap_info.expected_rssi = assoc_rsp->rssi_assoc_rej.delta_rssi +
					WMA_GET_RX_RSSI_NORMALIZED(rx_pkt_info);
		lim_add_bssid_to_reject_list(mac_ctx->pdev, &ap_info);
	}
	if (assoc_rsp->status_code != eSIR_MAC_SUCCESS_STATUS) {
		/*
		 *Re/Association response was received
		 * either with failure code.
		*/
		pe_err("received Re/AssocRsp frame failure code: %d",
			 assoc_rsp->status_code);
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
		pe_err("received Re/AssocRsp frame with invalid aid: %X",
			assoc_rsp->aid);
		assoc_cnf.resultCode = eSIR_SME_INVALID_ASSOC_RSP_RXED;
		assoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
		/* Send advisory Disassociation frame to AP */
		lim_send_disassoc_mgmt_frame(mac_ctx,
			eSIR_MAC_UNSPEC_FAILURE_REASON,
			hdr->sa, session_entry, false);
		goto assocReject;
	}

	/*
	 * If it is FILS connection, check is FILS params are matching
	 * with Authentication stage.
	 */
	if (!lim_verify_fils_params_assoc_rsp(mac_ctx, session_entry,
						assoc_rsp, &assoc_cnf)) {
		pe_err("FILS params doesnot match");
		assoc_cnf.resultCode = eSIR_SME_INVALID_ASSOC_RSP_RXED;
		assoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
		/* Send advisory Disassociation frame to AP */
		lim_send_disassoc_mgmt_frame(mac_ctx,
			eSIR_MAC_UNSPEC_FAILURE_REASON,
			hdr->sa, session_entry, false);
		goto assocReject;
	}

	if (assoc_rsp->QosMapSet.present)
		qdf_mem_copy(&session_entry->QosMapSet,
			     &assoc_rsp->QosMapSet,
			     sizeof(struct qos_map_set));
	else
		qdf_mem_zero(&session_entry->QosMapSet,
			     sizeof(struct qos_map_set));

	if (assoc_rsp->obss_scanparams.present)
		lim_update_obss_scanparams(session_entry,
				&assoc_rsp->obss_scanparams);

	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_ROAM_ASSOC_COMP_EVENT,
			      session_entry,
			      (assoc_rsp->status_code ? QDF_STATUS_E_FAILURE :
			       QDF_STATUS_SUCCESS), assoc_rsp->status_code);

	ap_nss = lim_get_nss_supported_by_ap(&assoc_rsp->VHTCaps,
					     &assoc_rsp->HTCaps,
					     &assoc_rsp->he_cap);

	if (subtype == LIM_REASSOC) {
		pe_debug("Successfully Reassociated with BSS");
#ifdef FEATURE_WLAN_ESE
	if (assoc_rsp->tsmPresent)
		lim_update_ese_tsm(mac_ctx, session_entry, assoc_rsp);
#endif
		if (session_entry->pLimMlmJoinReq) {
			qdf_mem_free(session_entry->pLimMlmJoinReq);
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
			pe_err("could not get hash entry at DPH for");
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

		if (ap_nss < session_entry->nss)
			session_entry->nss = ap_nss;

		lim_objmgr_update_vdev_nss(mac_ctx->psoc,
					   session_entry->smeSessionId,
					   session_entry->nss);

		if ((session_entry->limMlmState ==
		    eLIM_MLM_WT_FT_REASSOC_RSP_STATE) ||
			lim_is_roam_synch_in_progress(session_entry)) {
			pe_debug("Sending self sta");
			lim_update_assoc_sta_datas(mac_ctx, sta_ds, assoc_rsp,
				session_entry);
			lim_update_stads_ext_cap(mac_ctx, session_entry,
						 assoc_rsp, sta_ds);
			/* Store assigned AID for TIM processing */
			session_entry->limAID = assoc_rsp->aid & 0x3FFF;
			/* Downgrade the EDCA parameters if needed */
			lim_set_active_edca_params(mac_ctx,
				session_entry->gLimEdcaParams,
				session_entry);
			/* Send the active EDCA parameters to HAL */
			if (!lim_is_roam_synch_in_progress(session_entry)) {
				lim_send_edca_params(mac_ctx,
					session_entry->gLimEdcaParamsActive,
					session_entry->vdev_id, false);
				lim_add_ft_sta_self(mac_ctx,
					(assoc_rsp->aid & 0x3FFF),
					session_entry);
			}
			qdf_mem_free(beacon);
			return;
		}

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
				!= QDF_STATUS_SUCCESS) {
				pe_err("Could not cleanup the rx path");
				goto assocReject;
			}
		}
		qdf_mem_free(beacon);
		return;
	}
	pe_debug("Successfully Associated with BSS " QDF_MAC_ADDR_STR,
		 QDF_MAC_ADDR_ARRAY(hdr->sa));
#ifdef FEATURE_WLAN_ESE
	if (session_entry->eseContext.tsm.tsmInfo.state)
		session_entry->eseContext.tsm.tsmMetrics.RoamingCount = 0;
#endif
	/* Store assigned AID for TIM processing */
	session_entry->limAID = assoc_rsp->aid & 0x3FFF;

	/* STA entry was created during pre-assoc state. */
	sta_ds = dph_get_hash_entry(mac_ctx, DPH_STA_HASH_INDEX_PEER,
			&session_entry->dph.dphHashTable);
	if (!sta_ds) {
		/* Could not add hash table entry */
		pe_err("could not get hash entry at DPH");
		lim_print_mac_addr(mac_ctx, hdr->sa, LOGE);
		assoc_cnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
		assoc_cnf.protStatusCode = eSIR_SME_SUCCESS;
		lim_post_sme_message(mac_ctx, LIM_MLM_ASSOC_CNF,
			(uint32_t *) &assoc_cnf);
		qdf_mem_free(assoc_rsp);
		qdf_mem_free(beacon);
		return;
	}
	/* Delete Pre-auth context for the associated BSS */
	if (lim_search_pre_auth_list(mac_ctx, hdr->sa))
		lim_delete_pre_auth_node(mac_ctx, hdr->sa);

	if (ap_nss < session_entry->nss)
		session_entry->nss = ap_nss;

	lim_objmgr_update_vdev_nss(mac_ctx->psoc,
				   session_entry->smeSessionId,
				   session_entry->nss);

	lim_update_assoc_sta_datas(mac_ctx, sta_ds, assoc_rsp, session_entry);
	/*
	 * Extract the AP capabilities from the beacon that
	 * was received earlier
	*/
	ie_len = lim_get_ielen_from_bss_description(
		&session_entry->lim_join_req->bssDescription);
	lim_extract_ap_capabilities(mac_ctx,
		(uint8_t *)session_entry->lim_join_req->bssDescription.ieFields,
		ie_len, beacon);

	if (lim_is_session_he_capable(session_entry)) {
		session_entry->mu_edca_present = assoc_rsp->mu_edca_present;
		if (session_entry->mu_edca_present) {
			pe_debug("Save MU EDCA params to session");
			session_entry->ap_mu_edca_params[QCA_WLAN_AC_BE] =
				assoc_rsp->mu_edca.acbe;
			session_entry->ap_mu_edca_params[QCA_WLAN_AC_BK] =
				assoc_rsp->mu_edca.acbk;
			session_entry->ap_mu_edca_params[QCA_WLAN_AC_VI] =
				assoc_rsp->mu_edca.acvi;
			session_entry->ap_mu_edca_params[QCA_WLAN_AC_VO] =
				assoc_rsp->mu_edca.acvo;
		}

	}

	if (beacon->VHTCaps.present)
		sta_ds->parsed_ies.vht_caps = beacon->VHTCaps;
	if (beacon->HTCaps.present)
		sta_ds->parsed_ies.ht_caps = beacon->HTCaps;
	if (beacon->hs20vendor_ie.present)
		sta_ds->parsed_ies.hs20vendor_ie = beacon->hs20vendor_ie;
	if (beacon->HTInfo.present)
		sta_ds->parsed_ies.ht_operation = beacon->HTInfo;
	if (beacon->VHTOperation.present)
		sta_ds->parsed_ies.vht_operation = beacon->VHTOperation;

	if (mac_ctx->lim.gLimProtectionControl !=
	    MLME_FORCE_POLICY_PROTECTION_DISABLE)
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
			      QDF_STATUS_SUCCESS, QDF_STATUS_SUCCESS);
#endif
	lim_update_stads_ext_cap(mac_ctx, session_entry, assoc_rsp, sta_ds);
	/* Update the BSS Entry, this entry was added during preassoc. */
	if (QDF_STATUS_SUCCESS == lim_sta_send_add_bss(mac_ctx, assoc_rsp,
			beacon,
			&session_entry->lim_join_req->bssDescription, true,
			 session_entry)) {
		qdf_mem_free(assoc_rsp);
		qdf_mem_free(beacon);
		return;
	} else {
		pe_err("could not update the bss entry");
		assoc_cnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
		assoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
	}

assocReject:
	if ((subtype == LIM_ASSOC)
		|| ((subtype == LIM_REASSOC)
		&& (session_entry->limMlmState ==
		    eLIM_MLM_WT_FT_REASSOC_RSP_STATE))) {
		pe_err("Assoc Rejected by the peer mlmestate: %d sessionid: %d Reason: %d MACADDR:"
			QDF_MAC_ADDR_STR,
			session_entry->limMlmState,
			session_entry->peSessionId,
			assoc_cnf.resultCode, QDF_MAC_ADDR_ARRAY(hdr->sa));
		session_entry->limMlmState = eLIM_MLM_IDLE_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
			session_entry->peSessionId,
			session_entry->limMlmState));
		if (session_entry->pLimMlmJoinReq) {
			qdf_mem_free(session_entry->pLimMlmJoinReq);
			session_entry->pLimMlmJoinReq = NULL;
		}
		if (subtype == LIM_ASSOC) {
			lim_post_sme_message(mac_ctx, LIM_MLM_ASSOC_CNF,
				(uint32_t *) &assoc_cnf);
		} else {
			assoc_cnf.resultCode = eSIR_SME_FT_REASSOC_FAILURE;
			lim_post_sme_message(mac_ctx, LIM_MLM_REASSOC_CNF,
					(uint32_t *)&assoc_cnf);
		}
	} else {
		lim_restore_pre_reassoc_state(mac_ctx,
			eSIR_SME_REASSOC_REFUSED,
			assoc_cnf.protStatusCode,
			session_entry);
	}

	qdf_mem_free(beacon);
	qdf_mem_free(assoc_rsp->sha384_ft_subelem.gtk);
	qdf_mem_free(assoc_rsp->sha384_ft_subelem.igtk);
	qdf_mem_free(assoc_rsp);
	return;
}
