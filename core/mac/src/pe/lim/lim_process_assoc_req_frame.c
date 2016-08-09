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

/*
 * This file lim_process_assoc_req_frame.c contains the code
 * for processing Re/Association Request Frame.
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

#include "qdf_types.h"
#include "cds_utils.h"

/**
 * lim_convert_supported_channels - Parses channel support IE
 * @mac_ctx: A pointer to Global MAC structure
 * @assoc_ind: A pointer to SME ASSOC/REASSOC IND
 * @assoc_req: A pointer to ASSOC/REASSOC Request frame
 *
 * This function is called by lim_process_assoc_req_frame() to
 * parse the channel support IE in the Assoc/Reassoc Request
 * frame, and send relevant information in the SME_ASSOC_IND
 *
 * Return: None
 */
static void lim_convert_supported_channels(tpAniSirGlobal mac_ctx,
					   tpLimMlmAssocInd assoc_ind,
					   tSirAssocReq *assoc_req)
{
	uint16_t i, j, index = 0;
	uint8_t first_ch_no;
	uint8_t chn_count;
	uint8_t next_ch_no;
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
		first_ch_no = assoc_req->supportedChannels.supportedChannels[i];
		assoc_ind->supportedChannels.channelList[index] = first_ch_no;
		i++;
		index++;

		/* Get Number of Channels in a Subband */
		chn_count = assoc_req->supportedChannels.supportedChannels[i];
		lim_log(mac_ctx, LOG2,
			FL("Rcv assoc_req: chnl=%d, numOfChnl=%d "),
			first_ch_no, chn_count);
		if (index >= SIR_MAX_SUPPORTED_CHANNEL_LIST) {
			lim_log(mac_ctx, LOGW,
				FL("Ch count > max supported =%d "),
				chn_count);
			assoc_ind->supportedChannels.numChnl = 0;
			return;
		}
		if (chn_count <= 1)
			continue;
		next_ch_no = first_ch_no;
		if (SIR_BAND_5_GHZ == lim_get_rf_band(first_ch_no))
			channel_offset =  SIR_11A_FREQUENCY_OFFSET;
		else if (SIR_BAND_2_4_GHZ == lim_get_rf_band(first_ch_no))
			channel_offset = SIR_11B_FREQUENCY_OFFSET;
		else
			continue;

		for (j = 1; j < chn_count; j++) {
			next_ch_no += channel_offset;
			assoc_ind->supportedChannels.channelList[index]
				= next_ch_no;
			index++;
			if (index >= SIR_MAX_SUPPORTED_CHANNEL_LIST) {
				lim_log(mac_ctx, LOGW,
					FL("Ch count > supported =%d "),
					chn_count);
				assoc_ind->supportedChannels.numChnl = 0;
				return;
			}
		}
	}

	assoc_ind->supportedChannels.numChnl = (uint8_t) index;
	lim_log(mac_ctx, LOG2,
		FL("Send AssocInd to WSM:  minPwr %d, maxPwr %d, numChnl %d"),
		assoc_ind->powerCap.minTxPower,
		assoc_ind->powerCap.maxTxPower,
		assoc_ind->supportedChannels.numChnl);
}

/**
 * lim_check_sta_in_pe_entries() - checks if sta exists in any dph tables.
 * @mac_ctx: Pointer to Global MAC structure
 * @hdr: A pointer to the MAC header
 * @sessionid - session id for which session is initiated
 *
 * This function is called by lim_process_assoc_req_frame() to check if STA
 * entry already exists in any of the PE entries of the AP. If it exists, deauth
 * will be sent on that session and the STA deletion will happen. After this,
 * the ASSOC request will be processed
 *
 * Return: void
 */
void lim_check_sta_in_pe_entries(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
				 uint16_t sessionid)
{
	uint8_t i;
	uint16_t assoc_id = 0;
	tpDphHashNode sta_ds = NULL;
	tpPESession session = NULL;

	for (i = 0; i < mac_ctx->lim.maxBssId; i++) {
		if ((&mac_ctx->lim.gpSession[i] != NULL) &&
		    (mac_ctx->lim.gpSession[i].valid) &&
		    (mac_ctx->lim.gpSession[i].pePersona == QDF_SAP_MODE)) {
			session = &mac_ctx->lim.gpSession[i];
			sta_ds = dph_lookup_hash_entry(mac_ctx, hdr->sa,
					&assoc_id, &session->dph.dphHashTable);
			if (sta_ds
#ifdef WLAN_FEATURE_11W
				&& (!sta_ds->rmfEnabled ||
				    (sessionid != session->peSessionId))
#endif
			    ) {
				lim_log(mac_ctx, LOGE,
					FL("Sending Disassoc and Deleting existing STA entry: "
					   MAC_ADDRESS_STR),
					MAC_ADDR_ARRAY(session->selfMacAddr));
				lim_send_disassoc_mgmt_frame(mac_ctx,
					eSIR_MAC_UNSPEC_FAILURE_REASON,
					(uint8_t *) hdr->sa, session, false);
				/*
				 * Cleanup Rx path posts eWNI_SME_DISASSOC_RSP
				 * msg to SME after delete sta which will update
				 * the userspace with disconnect
				 */
				lim_cleanup_rx_path(mac_ctx, sta_ds, session);
				break;
			}
		}
	}
}

/**
 * lim_chk_sa_da() - checks source addr to destination addr of assoc req frame
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 *
 * Checks source addr to destination addr of assoc req frame
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_sa_da(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
			  tpPESession session, uint8_t sub_type)
{
	int result = qdf_mem_cmp((uint8_t *) hdr->sa,
					(uint8_t *) hdr->da,
					(uint8_t) (sizeof(tSirMacAddr)));
	if (0 != result)
		return true;

	lim_log(mac_ctx, LOGE, FL("Assoc Req rejected: wlan.sa = wlan.da"));
	lim_send_assoc_rsp_mgmt_frame(mac_ctx, eSIR_MAC_UNSPEC_FAILURE_STATUS,
				      1, hdr->sa, sub_type, 0, session);
	return false;
}

/**
 * lim_chk_tkip() - checks TKIP counter measure is active
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 *
 * Checks TKIP counter measure is active
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_tkip(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
			 tpPESession session, uint8_t sub_type)
{
	/*
	 * If TKIP counter measures active send Assoc Rsp frame to station
	 * with eSIR_MAC_MIC_FAILURE_REASON
	 */
	if (!(session->bTkipCntrMeasActive && LIM_IS_AP_ROLE(session)))
		return true;

	lim_log(mac_ctx, LOGE,
		FL("Assoc Req rejected: TKIP counter measure is active"));
	lim_send_assoc_rsp_mgmt_frame(mac_ctx, eSIR_MAC_MIC_FAILURE_REASON, 1,
				      hdr->sa, sub_type, 0, session);
	return false;
}

/**
 * lim_chk_assoc_req_parse_error() - checks for error in frame parsing
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 * @frm_body: frame body
 * @frame_len: frame len
 *
 * Checks for error in frame parsing
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_assoc_req_parse_error(tpAniSirGlobal mac_ctx,
					  tpSirMacMgmtHdr hdr,
					  tpPESession session,
					  tpSirAssocReq assoc_req,
					  uint8_t sub_type, uint8_t *frm_body,
					  uint32_t frame_len)
{
	tSirRetStatus status;

	if (sub_type == LIM_ASSOC)
		status = sir_convert_assoc_req_frame2_struct(mac_ctx, frm_body,
							frame_len, assoc_req);
	else
		status = sir_convert_reassoc_req_frame2_struct(mac_ctx,
						frm_body, frame_len, assoc_req);

	if (status == eSIR_SUCCESS)
		return true;

	lim_log(mac_ctx, LOGW,
		FL("Assoc Req rejected: frame parsing error. source addr:"
			MAC_ADDRESS_STR), MAC_ADDR_ARRAY(hdr->sa));
	lim_send_assoc_rsp_mgmt_frame(mac_ctx, eSIR_MAC_UNSPEC_FAILURE_STATUS,
				      1, hdr->sa, sub_type, 0, session);
	return false;
}

/**
 * lim_chk_capab() - checks for capab match
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 * @local_cap: local capabilities of SAP
 *
 * Checks for capab match
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_capab(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
			  tpPESession session, tpSirAssocReq assoc_req,
			  uint8_t sub_type, tSirMacCapabilityInfo *local_cap)
{
	uint16_t temp;

	if (cfg_get_capability_info(mac_ctx, &temp, session) != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGP, FL("could not retrieve Capabilities"));
		return false;
	}

	lim_copy_u16((uint8_t *) local_cap, temp);

	if (lim_compare_capabilities(mac_ctx, assoc_req,
				     local_cap, session) == false) {
		lim_log(mac_ctx, LOGW,
			FL("Rcvd %s Req with unsupported capab from"
				MAC_ADDRESS_STR),
			(LIM_ASSOC == sub_type) ? "Assoc" : "ReAssoc",
			MAC_ADDR_ARRAY(hdr->sa));
		/*
		 * Capabilities of requesting STA does not match with
		 * local capabilities. Respond with 'unsupported capabilities'
		 * status code.
		 */
		lim_send_assoc_rsp_mgmt_frame(mac_ctx,
			eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS,
			1, hdr->sa, sub_type, 0, session);
		return false;
	}
	return true;
}

/**
 * lim_chk_ssid() - checks for SSID match
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 *
 * Checks for SSID match
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_ssid(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
			 tpPESession session, tpSirAssocReq assoc_req,
			 uint8_t sub_type)
{
	if (lim_cmp_ssid(&assoc_req->ssId, session) != true)
		return true;

	lim_log(mac_ctx, LOGE,
		FL("%s Req with ssid wrong(Rcvd: %.*s self: %.*s) from "
			MAC_ADDRESS_STR),
		(LIM_ASSOC == sub_type) ? "Assoc" : "ReAssoc",
		assoc_req->ssId.length, assoc_req->ssId.ssId,
		session->ssId.length, session->ssId.ssId,
		MAC_ADDR_ARRAY(hdr->sa));

	/*
	 * Received Re/Association Request with either Broadcast SSID OR with
	 * SSID that does not match with local one. Respond with unspecified
	 * status code.
	 */
	lim_send_assoc_rsp_mgmt_frame(mac_ctx, eSIR_MAC_UNSPEC_FAILURE_STATUS,
				      1, hdr->sa, sub_type, 0, session);
	return false;
}

/**
 * lim_chk_rates() - checks for supported rates
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 *
 * Checks for supported rates
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_rates(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
			  tpPESession session, tpSirAssocReq assoc_req,
			  uint8_t sub_type)
{
	uint8_t i = 0, j = 0;
	tSirMacRateSet basic_rates;
	/*
	 * Verify if the requested rates are available in supported rate
	 * set or Extended rate set. Some APs are adding basic rates in
	 * Extended rateset IE
	 */
	basic_rates.numRates = 0;

	for (i = 0; i < assoc_req->supportedRates.numRates
			&& (i < SIR_MAC_RATESET_EID_MAX); i++) {
		basic_rates.rate[i] = assoc_req->supportedRates.rate[i];
		basic_rates.numRates++;
	}

	for (j = 0; (j < assoc_req->extendedRates.numRates)
			&& (i < SIR_MAC_RATESET_EID_MAX); i++, j++) {
		basic_rates.rate[i] = assoc_req->extendedRates.rate[j];
		basic_rates.numRates++;
	}

	if (lim_check_rx_basic_rates(mac_ctx, basic_rates, session) == true)
		return true;

	lim_log(mac_ctx, LOGW,
		FL("Assoc Req rejected: unsupported rates, soruce addr: %s"
			MAC_ADDRESS_STR),
		(LIM_ASSOC == sub_type) ? "Assoc" : "ReAssoc",
		MAC_ADDR_ARRAY(hdr->sa));
	/*
	 * Requesting STA does not support ALL BSS basic rates. Respond with
	 * 'basic rates not supported' status code.
	 */
	lim_send_assoc_rsp_mgmt_frame(mac_ctx,
				eSIR_MAC_BASIC_RATES_NOT_SUPPORTED_STATUS, 1,
				hdr->sa, sub_type, 0, session);
	return false;
}

/**
 * lim_chk_11g_only() - checks for non 11g STA
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 *
 * Checks for non 11g STA
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_11g_only(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
			     tpPESession session, tpSirAssocReq assoc_req,
			     uint8_t sub_type)
{
	if (LIM_IS_AP_ROLE(session) &&
	    (session->dot11mode == WNI_CFG_DOT11_MODE_11G_ONLY) &&
	    (assoc_req->HTCaps.present)) {
		lim_log(mac_ctx, LOGE,
			FL("SOFTAP was in 11G only mode, rejecting legacy STA: "
				MAC_ADDRESS_STR),
			MAC_ADDR_ARRAY(hdr->sa));
		lim_send_assoc_rsp_mgmt_frame(mac_ctx,
				eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS,
				1, hdr->sa, sub_type, 0, session);
		return false;
	}
	return true;
}

/**
 * lim_chk_11n_only() - checks for non 11n STA
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 *
 * Checks for non 11n STA
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_11n_only(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
			     tpPESession session, tpSirAssocReq assoc_req,
			     uint8_t sub_type)
{
	if (LIM_IS_AP_ROLE(session) &&
	    (session->dot11mode == WNI_CFG_DOT11_MODE_11N_ONLY) &&
	    (!assoc_req->HTCaps.present)) {
		lim_log(mac_ctx, LOGE,
			FL("SOFTAP was in 11N only mode, rejecting legacy STA: "
				MAC_ADDRESS_STR),
			MAC_ADDR_ARRAY(hdr->sa));
		lim_send_assoc_rsp_mgmt_frame(mac_ctx,
			eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS,
			1, hdr->sa, sub_type, 0, session);
		return false;
	}
	return true;
}

/**
 * lim_chk_11ac_only() - checks for non 11ac STA
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 *
 * Checks for non 11ac STA
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_11ac_only(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
			      tpPESession session, tpSirAssocReq assoc_req,
			      uint8_t sub_type)
{
	if (LIM_IS_AP_ROLE(session) &&
		(session->dot11mode == WNI_CFG_DOT11_MODE_11AC_ONLY) &&
		(!assoc_req->VHTCaps.present)) {
		lim_send_assoc_rsp_mgmt_frame(mac_ctx,
			eSIR_MAC_CAPABILITIES_NOT_SUPPORTED_STATUS,
			1, hdr->sa, sub_type, 0, session);
		lim_log(mac_ctx, LOGE,
			FL("SOFTAP was in 11AC only mode, reject"));
		return false;
	}
	return true;
}

/**
 * lim_process_for_spectrum_mgmt() - process assoc req for spectrum mgmt
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 * @local_cap: local capabilities of SAP
 *
 * Checks for SSID match
 *
 * process assoc req for spectrum mgmt
 */
static void
lim_process_for_spectrum_mgmt(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
			      tpPESession session, tpSirAssocReq assoc_req,
			      uint8_t sub_type, tSirMacCapabilityInfo local_cap)
{
	if (local_cap.spectrumMgt) {
		tSirRetStatus status = eSIR_SUCCESS;
		/*
		 * If station is 11h capable, then it SHOULD send all mandatory
		 * IEs in assoc request frame. Let us verify that
		 */
		if (assoc_req->capabilityInfo.spectrumMgt) {
			if (!((assoc_req->powerCapabilityPresent)
			     && (assoc_req->supportedChannelsPresent))) {
				/*
				 * One or more required information elements are
				 * missing, log the peers error
				 */
				if (!assoc_req->powerCapabilityPresent) {
					lim_log(mac_ctx, LOG1,
						FL("LIM Info: Missing Power capability IE in %s Req from "
							MAC_ADDRESS_STR),
						(LIM_ASSOC == sub_type) ?
							"Assoc" : "ReAssoc",
						MAC_ADDR_ARRAY(hdr->sa));
				}
				if (!assoc_req->supportedChannelsPresent) {
					lim_log(mac_ctx, LOGW,
						FL("LIM Info: Missing Supported channel IE in %s Req from "
							MAC_ADDRESS_STR),
						(LIM_ASSOC == sub_type) ?
							"Assoc" : "ReAssoc",
						MAC_ADDR_ARRAY(hdr->sa));
				}
			} else {
				/* Assoc request has mandatory fields */
				status =
				    lim_is_dot11h_power_capabilities_in_range(
					mac_ctx, assoc_req, session);
				if (eSIR_SUCCESS != status) {
					lim_log(mac_ctx, LOGW,
						FL("LIM Info: MinTxPower(STA) > MaxTxPower(AP) in %s Req from "
						MAC_ADDRESS_STR),
						(LIM_ASSOC == sub_type) ?
							"Assoc" : "ReAssoc",
						MAC_ADDR_ARRAY(hdr->sa));
				}
				status = lim_is_dot11h_supported_channels_valid(
							mac_ctx, assoc_req);
				if (eSIR_SUCCESS != status) {
					lim_log(mac_ctx, LOGW,
						FL("LIM Info: wrong supported channels (STA) in %s Req from "
						MAC_ADDRESS_STR),
						(LIM_ASSOC == sub_type) ?
							"Assoc" : "ReAssoc",
						MAC_ADDR_ARRAY(hdr->sa));
				}
				/* IEs are valid, use them if needed */
			}
		} /* if(assoc.capabilityInfo.spectrumMgt) */
		else {
			/*
			 * As per the capabiities, the spectrum management is
			 * not enabled on the station. The AP may allow the
			 * associations to happen even if spectrum management
			 * is not allowed, if the transmit power of station is
			 * below the regulatory maximum
			 */

			/*
			 * TODO: presently, this is not handled. In the current
			 * implemetation, the AP would allow the station to
			 * associate even if it doesn't support spectrum
			 * management.
			 */
		}
	} /* end of spectrum management related processing */
}

/**
 * lim_chk_mcs() - checks for supported MCS
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 *
 * Checks for supported MCS
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_mcs(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
			tpPESession session, tpSirAssocReq assoc_req,
			uint8_t sub_type)
{
	if ((assoc_req->HTCaps.present) && (lim_check_mcs_set(mac_ctx,
			assoc_req->HTCaps.supportedMCSSet) == false)) {
		lim_log(mac_ctx, LOGW,
			FL("rcvd %s req with unsupported MCS Rate Set from "
				MAC_ADDRESS_STR),
			(LIM_ASSOC == sub_type) ? "Assoc" : "ReAssoc",
			MAC_ADDR_ARRAY(hdr->sa));
		/*
		 * Requesting STA does not support ALL BSS MCS basic Rate set
		 * rates. Spec does not define any status code for this
		 * scenario.
		 */
		lim_send_assoc_rsp_mgmt_frame(mac_ctx,
					eSIR_MAC_OUTSIDE_SCOPE_OF_SPEC_STATUS,
					1, hdr->sa, sub_type, 0, session);
		return false;
	}
	return true;
}

/**
 * lim_chk_is_11b_sta_supported() - checks if STA is 11b
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 * @phy_mode: phy mode
 *
 * Checks if STA is 11b
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_is_11b_sta_supported(tpAniSirGlobal mac_ctx,
					 tpSirMacMgmtHdr hdr,
					 tpPESession session,
					 tpSirAssocReq assoc_req,
					 uint8_t sub_type, uint32_t phy_mode)
{
	uint32_t cfg_11g_only;

	if (phy_mode == WNI_CFG_PHY_MODE_11G) {
		if (wlan_cfg_get_int(mac_ctx, WNI_CFG_11G_ONLY_POLICY,
			&cfg_11g_only) != eSIR_SUCCESS) {
			lim_log(mac_ctx, LOGP,
				FL("couldn't get 11g-only flag"));
			return false;
		}

		if (!assoc_req->extendedRatesPresent && cfg_11g_only) {
			/*
			 * Received Re/Association Request from 11b STA when 11g
			 * only policy option is set. Reject with unspecified
			 * status code.
			 */
			lim_send_assoc_rsp_mgmt_frame(mac_ctx,
				eSIR_MAC_BASIC_RATES_NOT_SUPPORTED_STATUS,
				1, hdr->sa, sub_type, 0, session);

			lim_log(mac_ctx, LOGW,
				FL("Rejecting Re/Assoc req from 11b STA: "));
			lim_print_mac_addr(mac_ctx, hdr->sa, LOGW);

#ifdef WLAN_DEBUG
			mac_ctx->lim.gLim11bStaAssocRejectCount++;
#endif
			return false;
		}
	}
	return true;
}

/**
 * lim_print_ht_cap() - prints HT caps
 * @mac_ctx: pointer to Global MAC structure
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 *
 * Prints HT caps
 *
 * Return: void
 */
static void lim_print_ht_cap(tpAniSirGlobal mac_ctx, tpPESession session,
			     tpSirAssocReq assoc_req)
{
	if (!session->htCapability)
		return;

	/* There are; are they turned on in the STA? */
	if (assoc_req->HTCaps.present) {
		/* The station *does* support 802.11n HT capability... */
		lim_log(mac_ctx, LOG1,
			FL("AdvCodingCap:%d ChaWidthSet:%d PowerSave:%d greenField:%d shortGI20:%d shortGI40:%d txSTBC:%d rxSTBC:%d delayBA:%d maxAMSDUsize:%d DSSS/CCK:%d  PSMP:%d stbcCntl:%d lsigTXProt:%d"),
			assoc_req->HTCaps.advCodingCap,
			assoc_req->HTCaps.supportedChannelWidthSet,
			assoc_req->HTCaps.mimoPowerSave,
			assoc_req->HTCaps.greenField,
			assoc_req->HTCaps.shortGI20MHz,
			assoc_req->HTCaps.shortGI40MHz,
			assoc_req->HTCaps.txSTBC,
			assoc_req->HTCaps.rxSTBC,
			assoc_req->HTCaps.delayedBA,
			assoc_req->HTCaps.maximalAMSDUsize,
			assoc_req->HTCaps.dsssCckMode40MHz,
			assoc_req->HTCaps.psmp,
			assoc_req->HTCaps.stbcControlFrame,
			assoc_req->HTCaps.lsigTXOPProtection);
		/*
		 * Make sure the STA's caps are compatible with our own:
		 * 11.15.2 Support of DSSS/CCK in 40 MHz the AP shall refuse
		 * association requests from an HT STA that has the DSSS/CCK
		 * Mode in 40 MHz subfield set to 1;
		 */
	}
}

/**
 * lim_chk_n_process_wpa_rsn_ie() - wpa ie related checks
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 * @pmf_connection: flag indicating pmf connection
 *
 * wpa ie related checks
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_n_process_wpa_rsn_ie(tpAniSirGlobal mac_ctx,
					 tpSirMacMgmtHdr hdr,
					 tpPESession session,
					 tpSirAssocReq assoc_req,
					 uint8_t sub_type, bool *pmf_connection)
{
	uint8_t *wps_ie = NULL;
	tDot11fIEWPA dot11f_ie_wpa;
	tDot11fIERSN dot11f_ie_rsn;
	tSirRetStatus status = eSIR_SUCCESS;
	/*
	 * Clear the buffers so that frame parser knows that there isn't a
	 * previously decoded IE in these buffers
	 */
	qdf_mem_set((uint8_t *) &dot11f_ie_rsn, sizeof(dot11f_ie_rsn), 0);
	qdf_mem_set((uint8_t *) &dot11f_ie_wpa, sizeof(dot11f_ie_wpa), 0);

	/* if additional IE is present, check if it has WscIE */
	if (assoc_req->addIEPresent && assoc_req->addIE.length)
		wps_ie = limGetWscIEPtr(mac_ctx, assoc_req->addIE.addIEdata,
					assoc_req->addIE.length);
	else
		lim_log(mac_ctx, LOG1,
			FL("Assoc req addIEPresent = %d addIE length = %d"),
			assoc_req->addIEPresent, assoc_req->addIE.length);

	/* when wps_ie is present, RSN/WPA IE is ignored */
	if (wps_ie == NULL) {
		/* check whether as RSN IE is present */
		if (LIM_IS_AP_ROLE(session) &&
		    session->pLimStartBssReq->privacy &&
		    session->pLimStartBssReq->rsnIE.length) {
			lim_log(mac_ctx, LOGE,
				FL("RSN enabled auth, Re/Assoc req from STA: "
					MAC_ADDRESS_STR),
				MAC_ADDR_ARRAY(hdr->sa));
			if (assoc_req->rsnPresent) {
				if (assoc_req->rsn.length) {
					/* Unpack the RSN IE */
					dot11f_unpack_ie_rsn(mac_ctx,
						&assoc_req->rsn.info[0],
						assoc_req->rsn.length,
						&dot11f_ie_rsn);

					/* Check RSN version is supported */
					if (SIR_MAC_OUI_VERSION_1 ==
						dot11f_ie_rsn.version) {
						/*
						 * check the groupwise and
						 * pairwise cipher suites
						 */
						status =
						    lim_check_rx_rsn_ie_match(
						      mac_ctx, dot11f_ie_rsn,
						      session,
						      assoc_req->HTCaps.present,
						      pmf_connection);
						if (eSIR_SUCCESS != status) {
							lim_log(mac_ctx, LOGW,
								FL("Re/Assoc rejected from: " MAC_ADDRESS_STR),
							MAC_ADDR_ARRAY(
								hdr->sa));

							/*
							 * some IE is not
							 * properly sent
							 * received Association
							 * req frame with RSN IE
							 * but length is 0
							 */
							lim_send_assoc_rsp_mgmt_frame(
								mac_ctx,
								status, 1,
								hdr->sa,
								sub_type, 0,
								session);
							return false;
						}
					} else {
						lim_log(mac_ctx, LOGW,
							FL("Re/Assoc rejected from: " MAC_ADDRESS_STR),
							MAC_ADDR_ARRAY(
								hdr->sa));
						/*
						 * rcvd Assoc req frame with RSN
						 * IE version wrong
						 */
						lim_send_assoc_rsp_mgmt_frame(
							mac_ctx,
							eSIR_MAC_UNSUPPORTED_RSN_IE_VERSION_STATUS,
							1, hdr->sa, sub_type, 0,
							session);
						return false;
					}
				} else {
					lim_log(mac_ctx, LOGW,
						FL("Re/Assoc rejected from: "
							MAC_ADDRESS_STR),
						MAC_ADDR_ARRAY(hdr->sa));
					/*
					 * rcvd Assoc req frame with RSN IE but
					 * length is 0
					 */
					lim_send_assoc_rsp_mgmt_frame(mac_ctx,
						eSIR_MAC_INVALID_INFORMATION_ELEMENT_STATUS,
						1, hdr->sa, sub_type, 0,
						session);
					return false;
				}
			} /* end - if(assoc_req->rsnPresent) */
			if ((!assoc_req->rsnPresent) && assoc_req->wpaPresent) {
				/* Unpack the WPA IE */
				if (assoc_req->wpa.length) {
					/* OUI is not taken care */
					dot11f_unpack_ie_wpa(mac_ctx,
						&assoc_req->wpa.info[4],
						assoc_req->wpa.length,
						&dot11f_ie_wpa);
					/*
					 * check the groupwise and pairwise
					 * cipher suites
					 */
					status = lim_check_rx_wpa_ie_match(
						     mac_ctx, dot11f_ie_wpa,
						     session,
						     assoc_req->HTCaps.present);
					if (eSIR_SUCCESS != status) {
						lim_log(mac_ctx, LOGW,
							FL("Re/Assoc rejected from: "
							   MAC_ADDRESS_STR),
							MAC_ADDR_ARRAY(
								hdr->sa));
						/*
						 * rcvd Assoc req frame with WPA
						 * IE but mismatch
						 */
						lim_send_assoc_rsp_mgmt_frame(
							mac_ctx, status, 1,
							hdr->sa, sub_type, 0,
							session);
						return false;
					}
				} else {
					lim_log(mac_ctx, LOGW,
						FL("Re/Assoc rejected from: "
						   MAC_ADDRESS_STR),
						MAC_ADDR_ARRAY(hdr->sa));
					/*
					 * rcvd Assoc req frame with invalid WPA
					 * IE
					 */
					lim_send_assoc_rsp_mgmt_frame(mac_ctx,
						eSIR_MAC_INVALID_INFORMATION_ELEMENT_STATUS,
						1, hdr->sa, sub_type, 0,
						session);
					return false;
				} /* end - if(assoc_req->wpa.length) */
			} /* end - if(assoc_req->wpaPresent) */
		}
		/*
		 * end of if(session->pLimStartBssReq->privacy
		 * && session->pLimStartBssReq->rsnIE->length)
		 */
	} /* end of if( ! assoc_req->wscInfo.present ) */
	else {
		lim_log(mac_ctx, LOG1, FL("Assoc req WSE IE is present"));
	}
	return true;
}

/**
 * lim_process_assoc_req_no_sta_ctx() - process assoc req for no sta ctx present
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 * @sta_pre_auth_ctx: sta pre auth context
 * @sta_ds: station dph entry
 * @auth_type: indicates security type
 *
 * Process assoc req for no sta ctx present
 *
 * Return: true of no error, false otherwise
 */
static bool lim_process_assoc_req_no_sta_ctx(tpAniSirGlobal mac_ctx,
				tpSirMacMgmtHdr hdr, tpPESession session,
				tpSirAssocReq assoc_req, uint8_t sub_type,
				struct tLimPreAuthNode *sta_pre_auth_ctx,
				tpDphHashNode sta_ds, tAniAuthType *auth_type)
{
	/* Requesting STA is not currently associated */
	if (pe_get_current_stas_count(mac_ctx) ==
			mac_ctx->lim.gLimAssocStaLimit) {
		/*
		 * Maximum number of STAs that AP can handle reached.
		 * Send Association response to peer MAC entity
		 */
		lim_log(mac_ctx, LOGE, FL("Max Sta count reached : %d"),
				mac_ctx->lim.maxStation);
		lim_reject_association(mac_ctx, hdr->sa, sub_type, false,
			(tAniAuthType) 0, 0, false,
			(tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS,
			session);
		return false;
	}
	/* Check if STA is pre-authenticated. */
	if ((sta_pre_auth_ctx == NULL) || (sta_pre_auth_ctx &&
		(sta_pre_auth_ctx->mlmState != eLIM_MLM_AUTHENTICATED_STATE))) {
		/*
		 * STA is not pre-authenticated yet requesting Re/Association
		 * before Authentication. OR STA is in the process of getting
		 * authenticated and sent Re/Association request. Send
		 * Deauthentication frame with 'prior authentication required'
		 * reason code.
		 */
		lim_send_deauth_mgmt_frame(mac_ctx,
				eSIR_MAC_STA_NOT_PRE_AUTHENTICATED_REASON,
				hdr->sa, session, false);

		lim_log(mac_ctx, LOGW,
			FL("rcvd %s req, sessionid: %d, without pre-auth ctx"
				MAC_ADDRESS_STR),
			(LIM_ASSOC == sub_type) ? "Assoc" : "ReAssoc",
			session->peSessionId, MAC_ADDR_ARRAY(hdr->sa));
		return false;
	}
	/* Delete 'pre-auth' context of STA */
	*auth_type = sta_pre_auth_ctx->authType;
	lim_delete_pre_auth_node(mac_ctx, hdr->sa);
	/* All is well. Assign AID (after else part) */
	return true;
}

/**
 * lim_process_assoc_req_sta_ctx() - process assoc req for sta context present
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 * @sta_pre_auth_ctx: sta pre auth context
 * @sta_ds: station dph entry
 * @peer_idx: peer index
 * @auth_type: indicates security type
 * @update_ctx: indicates if STA context already exist
 *
 * Process assoc req for sta context present
 *
 * Return: true of no error, false otherwise
 */
static bool lim_process_assoc_req_sta_ctx(tpAniSirGlobal mac_ctx,
				tpSirMacMgmtHdr hdr, tpPESession session,
				tpSirAssocReq assoc_req, uint8_t sub_type,
				struct tLimPreAuthNode *sta_pre_auth_ctx,
				tpDphHashNode sta_ds, uint16_t peer_idx,
				tAniAuthType *auth_type, uint8_t *update_ctx)
{
	/* STA context does exist for this STA */
	if (sta_ds->mlmStaContext.mlmState != eLIM_MLM_LINK_ESTABLISHED_STATE) {
		/*
		 * Requesting STA is in some 'transient' state? Ignore the
		 * Re/Assoc Req frame by incrementing debug counter & logging
		 * error.
		 */
		if (sub_type == LIM_ASSOC) {
#ifdef WLAN_DEBUG
			mac_ctx->lim.gLimNumAssocReqDropInvldState++;
#endif
			lim_log(mac_ctx, LOG1,
				FL("received Assoc req in state %X from "),
				sta_ds->mlmStaContext.mlmState);
		} else {
#ifdef WLAN_DEBUG
			mac_ctx->lim.gLimNumReassocReqDropInvldState++;
#endif
			lim_log(mac_ctx, LOG1,
				FL("received ReAssoc req in state %X from "),
				sta_ds->mlmStaContext.mlmState);
		}
		lim_print_mac_addr(mac_ctx, hdr->sa, LOG1);
		lim_print_mlm_state(mac_ctx, LOG1,
			(tLimMlmStates) sta_ds->mlmStaContext.mlmState);
		return false;
	}

	/* STA sent assoc req frame while already in 'associated' state */

#ifdef WLAN_FEATURE_11W
	lim_log(mac_ctx, LOG1,
		FL("Re/Assoc request from station that is already associated"));
	lim_log(mac_ctx, LOG1, FL("PMF enabled %d, SA Query state %d"),
		sta_ds->rmfEnabled, sta_ds->pmfSaQueryState);
	if (sta_ds->rmfEnabled) {
		switch (sta_ds->pmfSaQueryState) {
		/*
		 * start SA Query procedure, respond to Association Request with
		 * try again later
		 */
		case DPH_SA_QUERY_NOT_IN_PROGRESS:
			/*
			 * We should reset the retry counter before we start
			 * the SA query procedure, otherwise in next set of SA
			 * query procedure we will end up using the stale value.
			 */
			sta_ds->pmfSaQueryRetryCount = 0;
			lim_send_assoc_rsp_mgmt_frame(mac_ctx,
					eSIR_MAC_TRY_AGAIN_LATER, 1, hdr->sa,
					sub_type, sta_ds, session);
			lim_send_sa_query_request_frame(mac_ctx,
				(uint8_t *) &(sta_ds->pmfSaQueryCurrentTransId),
				hdr->sa, session);
			sta_ds->pmfSaQueryStartTransId =
				sta_ds->pmfSaQueryCurrentTransId;
			sta_ds->pmfSaQueryCurrentTransId++;

			/* start timer for SA Query retry */
			if (tx_timer_activate(&sta_ds->pmfSaQueryTimer)
					!= TX_SUCCESS) {
				lim_log(mac_ctx, LOGE,
					FL("PMF SA Query timer start failed!"));
				return false;
			}
			sta_ds->pmfSaQueryState = DPH_SA_QUERY_IN_PROGRESS;
			return false;
		/*
		 * SA Query procedure still going, respond to Association
		 * Request with try again later
		 */
		case DPH_SA_QUERY_IN_PROGRESS:
			lim_send_assoc_rsp_mgmt_frame(mac_ctx,
						eSIR_MAC_TRY_AGAIN_LATER, 1,
						hdr->sa, sub_type, 0, session);
			return false;

		/*
		 * SA Query procedure timed out, accept Association
		 * Request normally
		 */
		case DPH_SA_QUERY_TIMED_OUT:
			sta_ds->pmfSaQueryState = DPH_SA_QUERY_NOT_IN_PROGRESS;
			break;
		}
	}
#endif

	/* no change in the capability so drop the frame */
	if ((sub_type == LIM_ASSOC) &&
		(0 == qdf_mem_cmp(&sta_ds->mlmStaContext.capabilityInfo,
			&assoc_req->capabilityInfo,
			sizeof(tSirMacCapabilityInfo)))) {
		lim_log(mac_ctx, LOGE,
			FL(" Received Assoc req in state %X STAid=%d"),
			sta_ds->mlmStaContext.mlmState, peer_idx);
		return false;
	} else {
		/*
		 * STA sent Re/association Request frame while already in
		 * 'associated' state. Update STA capabilities and send
		 * Association response frame with same AID
		 */
		lim_log(mac_ctx, LOG1,
			FL("Rcvd Assoc req from STA already connected"));
		sta_ds->mlmStaContext.capabilityInfo =
			assoc_req->capabilityInfo;
		if (sta_pre_auth_ctx && (sta_pre_auth_ctx->mlmState ==
			eLIM_MLM_AUTHENTICATED_STATE)) {
			/* STA has triggered pre-auth again */
			*auth_type = sta_pre_auth_ctx->authType;
			lim_delete_pre_auth_node(mac_ctx, hdr->sa);
		} else {
			*auth_type = sta_ds->mlmStaContext.authType;
		}

		*update_ctx = true;
		if (dph_init_sta_state(mac_ctx, hdr->sa, peer_idx, true,
			&session->dph.dphHashTable) == NULL) {
			lim_log(mac_ctx, LOGE,
				FL("could not Init STAid=%d"), peer_idx);
			return false;
		}
	}
	return true;
}

/**
 * lim_chk_wmm() - wmm related checks
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 * @qos_mode: qos mode
 *
 * wmm related checks
 *
 * Return: true of no error, false otherwise
 */
static bool lim_chk_wmm(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
			tpPESession session, tpSirAssocReq assoc_req,
			uint8_t sub_type, tHalBitVal qos_mode)
{
	tHalBitVal wme_mode;
	limGetWmeMode(session, &wme_mode);
	if ((qos_mode == eHAL_SET) || (wme_mode == eHAL_SET)) {
		/*
		 * for a qsta, check if the requested Traffic spec is admissible
		 * for a non-qsta check if the sta can be admitted
		 */
		if (assoc_req->addtsPresent) {
			uint8_t tspecIdx = 0;
			if (lim_admit_control_add_ts(mac_ctx, hdr->sa,
				&(assoc_req->addtsReq),
				&(assoc_req->qosCapability),
				0, false, NULL, &tspecIdx, session) !=
					eSIR_SUCCESS) {
				lim_log(mac_ctx, LOGW,
					FL("AdmitControl: TSPEC rejected"));
				lim_send_assoc_rsp_mgmt_frame(mac_ctx,
					eSIR_MAC_QAP_NO_BANDWIDTH_REASON,
					1, hdr->sa, sub_type, 0, session);
#ifdef WLAN_DEBUG
				mac_ctx->lim.gLimNumAssocReqDropACRejectTS++;
#endif
				return false;
			}
		} else if (lim_admit_control_add_sta(mac_ctx, hdr->sa, false)
				!= eSIR_SUCCESS) {
			lim_log(mac_ctx, LOGW,
				FL("AdmitControl: Sta rejected"));
			lim_send_assoc_rsp_mgmt_frame(mac_ctx,
				eSIR_MAC_QAP_NO_BANDWIDTH_REASON, 1,
				hdr->sa, sub_type, 0, session);
#ifdef WLAN_DEBUG
			mac_ctx->lim.gLimNumAssocReqDropACRejectSta++;
#endif
			return false;
		}
		/* else all ok */
		lim_log(mac_ctx, LOG1, FL("AdmitControl: Sta OK!"));
	}
	return true;
}

/**
 * lim_update_sta_ds() - updates ds dph entry
 * @mac_ctx: pointer to Global MAC structure
 * @hdr: pointer to the MAC head
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 * @sta_ds: station dph entry
 * @tmp_assoc_req: pointer to tmp ASSOC/REASSOC Request frame
 * @auth_type: indicates security type
 * @assoc_req_copied: boolean to indicate if assoc req was copied to tmp above
 * @peer_idx: peer index
 * @qos_mode: qos mode
 * @pmf_connection: flag indicating pmf connection
 *
 * Updates ds dph entry
 *
 * Return: true of no error, false otherwise
 */
static bool lim_update_sta_ds(tpAniSirGlobal mac_ctx, tpSirMacMgmtHdr hdr,
			      tpPESession session, tpSirAssocReq assoc_req,
			      uint8_t sub_type, tpDphHashNode sta_ds,
			      tpSirAssocReq tmp_assoc_req,
			      tAniAuthType auth_type,
			      bool *assoc_req_copied, uint16_t peer_idx,
			      tHalBitVal qos_mode, bool pmf_connection)
{
	tHalBitVal wme_mode, wsm_mode;
	uint8_t *ht_cap_ie = NULL;
#ifdef WLAN_FEATURE_11W
	tPmfSaQueryTimerId timer_id;
	uint32_t retry_interval;
#endif
	/*
	 * check here if the parsedAssocReq already pointing to the assoc_req
	 * and free it before assigning this new assoc_req
	 */
	if (session->parsedAssocReq != NULL) {
		tmp_assoc_req = session->parsedAssocReq[sta_ds->assocId];
		if (tmp_assoc_req != NULL) {
			if (tmp_assoc_req->assocReqFrame) {
				qdf_mem_free(tmp_assoc_req->assocReqFrame);
				tmp_assoc_req->assocReqFrame = NULL;
				tmp_assoc_req->assocReqFrameLength = 0;
			}
			qdf_mem_free(tmp_assoc_req);
			tmp_assoc_req = NULL;
		}

		session->parsedAssocReq[sta_ds->assocId] = assoc_req;
		*assoc_req_copied = true;
	}

	sta_ds->mlmStaContext.htCapability = assoc_req->HTCaps.present;
	sta_ds->mlmStaContext.vhtCapability = assoc_req->VHTCaps.present;
	sta_ds->qos.addtsPresent =
		(assoc_req->addtsPresent == 0) ? false : true;
	sta_ds->qos.addts = assoc_req->addtsReq;
	sta_ds->qos.capability = assoc_req->qosCapability;
	sta_ds->versionPresent = 0;
	/*
	 * short slot and short preamble should be updated before doing
	 * limaddsta
	 */
	sta_ds->shortPreambleEnabled =
		(uint8_t) assoc_req->capabilityInfo.shortPreamble;
	sta_ds->shortSlotTimeEnabled =
		(uint8_t) assoc_req->capabilityInfo.shortSlotTime;

	sta_ds->valid = 0;
	sta_ds->mlmStaContext.authType = auth_type;
	sta_ds->staType = STA_ENTRY_PEER;

	/*
	 * TODO: If listen interval is more than certain limit, reject the
	 * association. Need to check customer requirements and then implement.
	 */
	sta_ds->mlmStaContext.listenInterval = assoc_req->listenInterval;
	sta_ds->mlmStaContext.capabilityInfo = assoc_req->capabilityInfo;

	/*
	 * The following count will be used to knock-off the station if it
	 * doesn't come back to receive the buffered data. The AP will wait
	 * for numTimSent number of beacons after sending TIM information for
	 * the station, before assuming that the station is no more associated
	 * and disassociates it
	 */

	/* timWaitCount used by PMM for monitoring the STA's in PS for LINK */
	sta_ds->timWaitCount =
		(uint8_t) GET_TIM_WAIT_COUNT(assoc_req->listenInterval);

	/* Init the Current successful MPDU's tranfered to this STA count = 0 */
	sta_ds->curTxMpduCnt = 0;

	if (IS_DOT11_MODE_HT(session->dot11mode) &&
	    assoc_req->HTCaps.present && assoc_req->wmeInfoPresent) {
		sta_ds->htGreenfield = (uint8_t) assoc_req->HTCaps.greenField;
		sta_ds->htAMpduDensity = assoc_req->HTCaps.mpduDensity;
		sta_ds->htDsssCckRate40MHzSupport =
			(uint8_t) assoc_req->HTCaps.dsssCckMode40MHz;
		sta_ds->htLsigTXOPProtection =
			(uint8_t) assoc_req->HTCaps.lsigTXOPProtection;
		sta_ds->htMaxAmsduLength =
			(uint8_t) assoc_req->HTCaps.maximalAMSDUsize;
		sta_ds->htMaxRxAMpduFactor = assoc_req->HTCaps.maxRxAMPDUFactor;
		sta_ds->htMIMOPSState = assoc_req->HTCaps.mimoPowerSave;

		/* assoc_req will be copied to session->parsedAssocReq later */
		ht_cap_ie = ((uint8_t *) &assoc_req->HTCaps) + 1;

		if (session->htConfig.ht_sgi20) {
			sta_ds->htShortGI20Mhz =
				(uint8_t)assoc_req->HTCaps.shortGI20MHz;
		} else {
			/* Unset htShortGI20Mhz in ht_caps*/
			*ht_cap_ie &= ~(1 << SIR_MAC_HT_CAP_SHORTGI20MHZ_S);
			sta_ds->htShortGI20Mhz = 0;
		}

		if (session->htConfig.ht_sgi40) {
			sta_ds->htShortGI40Mhz =
				(uint8_t)assoc_req->HTCaps.shortGI40MHz;
		} else {
			/* Unset htShortGI40Mhz in ht_caps */
			*ht_cap_ie &= ~(1 << SIR_MAC_HT_CAP_SHORTGI40MHZ_S);
			sta_ds->htShortGI40Mhz = 0;
		}

		sta_ds->htSupportedChannelWidthSet =
			(uint8_t) assoc_req->HTCaps.supportedChannelWidthSet;
		/*
		 * peer just follows AP; so when we are softAP/GO,
		 * we just store our session entry's secondary channel offset
		 * here in peer INFRA STA. However, if peer's 40MHz channel
		 * width support is disabled then secondary channel will be zero
		 */
		sta_ds->htSecondaryChannelOffset =
			(sta_ds->htSupportedChannelWidthSet) ?
				session->htSecondaryChannelOffset : 0;
		if (assoc_req->operMode.present) {
			sta_ds->vhtSupportedChannelWidthSet =
				(uint8_t) ((assoc_req->operMode.chanWidth ==
				eHT_CHANNEL_WIDTH_80MHZ) ?
				WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ :
				WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ);
			sta_ds->htSupportedChannelWidthSet =
				(uint8_t) (assoc_req->operMode.chanWidth ?
				eHT_CHANNEL_WIDTH_40MHZ :
				eHT_CHANNEL_WIDTH_20MHZ);
		} else if (assoc_req->VHTCaps.present) {
			/*
			 * Check if STA has enabled it's channel bonding mode.
			 * If channel bonding mode is enabled, we decide based
			 * on SAP's current configuration. else, we set it to
			 * VHT20. */
			sta_ds->vhtSupportedChannelWidthSet =
				(uint8_t) ((sta_ds->htSupportedChannelWidthSet
					== eHT_CHANNEL_WIDTH_20MHZ) ?
					WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ :
					session->ch_width - 1);
			sta_ds->htMaxRxAMpduFactor =
				assoc_req->VHTCaps.maxAMPDULenExp;
		}
		/* Lesser among the AP and STA bandwidth of operation. */
		sta_ds->htSupportedChannelWidthSet =
			(sta_ds->htSupportedChannelWidthSet <
			session->htSupportedChannelWidthSet) ?
			sta_ds->htSupportedChannelWidthSet :
			session->htSupportedChannelWidthSet;
		sta_ds->baPolicyFlag = 0xFF;
		sta_ds->htLdpcCapable =
			(uint8_t) assoc_req->HTCaps.advCodingCap;
	}

	if (assoc_req->VHTCaps.present && assoc_req->wmeInfoPresent) {
		sta_ds->vhtLdpcCapable =
			(uint8_t) assoc_req->VHTCaps.ldpcCodingCap;
	}

	if (!assoc_req->wmeInfoPresent) {
		sta_ds->mlmStaContext.htCapability = 0;
		sta_ds->mlmStaContext.vhtCapability = 0;
	}
	if (sta_ds->mlmStaContext.vhtCapability) {
		if (session->vht_config.su_beam_formee &&
				assoc_req->VHTCaps.suBeamFormerCap)
			sta_ds->vhtBeamFormerCapable = 1;
		else
			sta_ds->vhtBeamFormerCapable = 0;
		if (session->vht_config.su_beam_former &&
				assoc_req->VHTCaps.suBeamformeeCap)
			sta_ds->vht_su_bfee_capable = 1;
		else
			sta_ds->vht_su_bfee_capable = 0;
	}
	if (lim_populate_matching_rate_set(mac_ctx, sta_ds,
			&(assoc_req->supportedRates),
			&(assoc_req->extendedRates),
			assoc_req->HTCaps.supportedMCSSet,
			session, &assoc_req->VHTCaps) != eSIR_SUCCESS)
	{
		/* Could not update hash table entry at DPH with rateset */
		lim_log(mac_ctx, LOGE,
			FL("Couldn't update hash entry for aid=%d, MacAddr: "
				MAC_ADDRESS_STR),
			peer_idx, MAC_ADDR_ARRAY(hdr->sa));

		/* Release AID */
		lim_release_peer_idx(mac_ctx, peer_idx, session);

		lim_reject_association(mac_ctx, hdr->sa,
			sub_type, true, auth_type, peer_idx, true,
			(tSirResultCodes)eSIR_MAC_UNSPEC_FAILURE_STATUS,
			session);

		if (session->parsedAssocReq)
			assoc_req = session->parsedAssocReq[sta_ds->assocId];
		return false;
	}
	if (assoc_req->operMode.present) {
		sta_ds->vhtSupportedRxNss = assoc_req->operMode.rxNSS + 1;
	} else {
		sta_ds->vhtSupportedRxNss =
			((sta_ds->supportedRates.vhtRxMCSMap & MCSMAPMASK2x2)
				== MCSMAPMASK2x2) ? 1 : 2;
	}

	/* Add STA context at MAC HW (BMU, RHP & TFP) */
	sta_ds->qosMode = false;
	sta_ds->lleEnabled = false;
	if (assoc_req->capabilityInfo.qos && (qos_mode == eHAL_SET)) {
		sta_ds->lleEnabled = true;
		sta_ds->qosMode = true;
	}

	sta_ds->wmeEnabled = false;
	sta_ds->wsmEnabled = false;
	limGetWmeMode(session, &wme_mode);
	if ((!sta_ds->lleEnabled) && assoc_req->wmeInfoPresent
	    && (wme_mode == eHAL_SET)) {
		sta_ds->wmeEnabled = true;
		sta_ds->qosMode = true;
		limGetWsmMode(session, &wsm_mode);
		/*
		 * WMM_APSD - WMM_SA related processing should be separate;
		 * WMM_SA and WMM_APSD can coexist
		 */
		if (assoc_req->WMMInfoStation.present) {
			/* check whether AP supports or not */
			if (LIM_IS_AP_ROLE(session) &&
				(session->apUapsdEnable == 0) &&
				(assoc_req->WMMInfoStation.acbe_uapsd ||
					assoc_req->WMMInfoStation.acbk_uapsd ||
					assoc_req->WMMInfoStation.acvo_uapsd ||
					assoc_req->WMMInfoStation.acvi_uapsd)) {
				/*
				 * Rcvd Re/Assoc Req from STA when UPASD is
				 * not supported.
				 */
				lim_log(mac_ctx, LOGE,
					FL("UAPSD not supported, reply accordingly"));
				/* update UAPSD and send it to LIM to add STA */
				sta_ds->qos.capability.qosInfo.acbe_uapsd = 0;
				sta_ds->qos.capability.qosInfo.acbk_uapsd = 0;
				sta_ds->qos.capability.qosInfo.acvo_uapsd = 0;
				sta_ds->qos.capability.qosInfo.acvi_uapsd = 0;
				sta_ds->qos.capability.qosInfo.maxSpLen = 0;
			} else {
				/* update UAPSD and send it to LIM to add STA */
				sta_ds->qos.capability.qosInfo.acbe_uapsd =
					assoc_req->WMMInfoStation.acbe_uapsd;
				sta_ds->qos.capability.qosInfo.acbk_uapsd =
					assoc_req->WMMInfoStation.acbk_uapsd;
				sta_ds->qos.capability.qosInfo.acvo_uapsd =
					assoc_req->WMMInfoStation.acvo_uapsd;
				sta_ds->qos.capability.qosInfo.acvi_uapsd =
					assoc_req->WMMInfoStation.acvi_uapsd;
				sta_ds->qos.capability.qosInfo.maxSpLen =
					assoc_req->WMMInfoStation.max_sp_length;
			}
		}
		if (assoc_req->wsmCapablePresent && (wsm_mode == eHAL_SET))
			sta_ds->wsmEnabled = true;
	}
	/* Re/Assoc Response frame to requesting STA */
	sta_ds->mlmStaContext.subType = sub_type;

#ifdef WLAN_FEATURE_11W
	sta_ds->rmfEnabled = (pmf_connection) ? 1 : 0;
	sta_ds->pmfSaQueryState = DPH_SA_QUERY_NOT_IN_PROGRESS;
	timer_id.fields.sessionId = session->peSessionId;
	timer_id.fields.peerIdx = peer_idx;
	if (wlan_cfg_get_int(mac_ctx, WNI_CFG_PMF_SA_QUERY_RETRY_INTERVAL,
			&retry_interval) != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGE,
			FL("Couldn't get PMF SA Query retry interval value"));
		lim_reject_association(mac_ctx, hdr->sa, sub_type, true,
			auth_type, peer_idx, false,
			(tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS,
			session);
		return false;
	}
	if (WNI_CFG_PMF_SA_QUERY_RETRY_INTERVAL_STAMIN > retry_interval) {
		retry_interval = WNI_CFG_PMF_SA_QUERY_RETRY_INTERVAL_STADEF;
	}
	if (tx_timer_create(mac_ctx, &sta_ds->pmfSaQueryTimer,
			"PMF SA Query timer", lim_pmf_sa_query_timer_handler,
			timer_id.value,
			SYS_MS_TO_TICKS((retry_interval * 1024) / 1000),
			0, TX_NO_ACTIVATE) != TX_SUCCESS) {
		lim_log(mac_ctx, LOGE,
			FL("could not create PMF SA Query timer"));
		lim_reject_association(mac_ctx, hdr->sa, sub_type,
			true, auth_type, peer_idx, false,
			(tSirResultCodes)eSIR_MAC_UNSPEC_FAILURE_STATUS,
			session);
		return false;
	}
#endif

	if (assoc_req->ExtCap.present) {
		lim_set_stads_rtt_cap(sta_ds,
			(struct s_ext_cap *) assoc_req->ExtCap.bytes, mac_ctx);
	} else {
		sta_ds->timingMeasCap = 0;
		PELOG1(lim_log(mac_ctx, LOG1, FL("ExtCap not present"));)
	}
	return true;
}

/**
 * lim_update_sta_ctx() - add/del sta depending on connection state machine
 * @mac_ctx: pointer to Global MAC structure
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sub_type: Assoc(=0) or Reassoc(=1) Requestframe
 * @sta_ds: station dph entry
 * @update_ctx: indicates if STA context already exist
 *
 * Checks for SSID match
 *
 * Return: true of no error, false otherwise
 */
static bool lim_update_sta_ctx(tpAniSirGlobal mac_ctx, tpPESession session,
			       tpSirAssocReq assoc_req, uint8_t sub_type,
			       tpDphHashNode sta_ds, uint8_t update_ctx)
{
	tLimMlmStates mlm_prev_state;
	/*
	 * BTAMP: If STA context already exist (ie. update_ctx = 1) for this STA
	 * then we should delete the old one, and add the new STA. This is taken
	 * care of in the lim_del_sta() routine.
	 *
	 * Prior to BTAMP, we were setting this flag so that when PE receives
	 * SME_ASSOC_CNF, and if this flag is set, then PE shall delete the old
	 * station and then add. But now in BTAMP, we're directly adding station
	 * before waiting for SME_ASSOC_CNF, so we can do this now.
	 */
	if (!(update_ctx)) {
		sta_ds->mlmStaContext.updateContext = 0;

		/*
		 * BTAMP: Add STA context at HW - issue WMA_ADD_STA_REQ to HAL
		 */
		if (lim_add_sta(mac_ctx, sta_ds, false, session) !=
		    eSIR_SUCCESS) {
			lim_log(mac_ctx, LOGE,
				FL("could not Add STA with assocId=%d"),
				sta_ds->assocId);
			lim_reject_association(mac_ctx, sta_ds->staAddr,
				sta_ds->mlmStaContext.subType, true,
				sta_ds->mlmStaContext.authType,
				sta_ds->assocId, true,
				(tSirResultCodes)eSIR_MAC_UNSPEC_FAILURE_STATUS,
				session);

			if (session->parsedAssocReq)
				assoc_req =
				    session->parsedAssocReq[sta_ds->assocId];
			return false;
		}
	} else {
		sta_ds->mlmStaContext.updateContext = 1;
		mlm_prev_state = sta_ds->mlmStaContext.mlmState;

		/*
		 * As per the HAL/FW needs the reassoc req need not be calling
		 * lim_del_sta
		 */
		if (sub_type != LIM_REASSOC) {
			/*
			 * we need to set the mlmState here in order
			 * differentiate in lim_del_sta.
			 */
			sta_ds->mlmStaContext.mlmState =
				eLIM_MLM_WT_ASSOC_DEL_STA_RSP_STATE;
			if (lim_del_sta(mac_ctx, sta_ds, true, session)
					!= eSIR_SUCCESS) {
				lim_log(mac_ctx, LOGE,
					FL("Couldn't DEL STA, assocId=%d staId %d"),
					sta_ds->assocId, sta_ds->staIndex);
				lim_reject_association(mac_ctx, sta_ds->staAddr,
					sta_ds->mlmStaContext.subType, true,
					sta_ds->mlmStaContext.authType,
					sta_ds->assocId, true,
					(tSirResultCodes)
						eSIR_MAC_UNSPEC_FAILURE_STATUS,
					session);

				/* Restoring the state back. */
				sta_ds->mlmStaContext.mlmState = mlm_prev_state;
				if (session->parsedAssocReq)
					assoc_req = session->parsedAssocReq[
						sta_ds->assocId];
				return false;
			}
		} else {
			/*
			 * mlmState is changed in lim_add_sta context use the
			 * same AID, already allocated
			 */
			if (lim_add_sta(mac_ctx, sta_ds, false, session)
				!= eSIR_SUCCESS) {
				lim_log(mac_ctx, LOGE,
					FL("UPASD not supported, REASSOC Failed"));
				lim_reject_association(mac_ctx, sta_ds->staAddr,
					sta_ds->mlmStaContext.subType, true,
					sta_ds->mlmStaContext.authType,
					sta_ds->assocId, true,
					(tSirResultCodes)
						eSIR_MAC_WME_REFUSED_STATUS,
					session);

				/* Restoring the state back. */
				sta_ds->mlmStaContext.mlmState = mlm_prev_state;
				if (session->parsedAssocReq)
					assoc_req = session->parsedAssocReq[
							sta_ds->assocId];
				return false;
			}
		}
	}
	return true;
}

/**
 * lim_process_assoc_cleanup() - frees up resources used in function
 * lim_process_assoc_req_frame()
 * @mac_ctx: pointer to Global MAC structure
 * @session: pointer to pe session entry
 * @assoc_req: pointer to ASSOC/REASSOC Request frame
 * @sta_ds: station dph entry
 * @tmp_assoc_req: pointer to tmp ASSOC/REASSOC Request frame
 * @assoc_req_copied: boolean to indicate if assoc req was copied to tmp above
 *
 * Frees up resources used in function lim_process_assoc_req_frame
 *
 * Return: void
 */
static void lim_process_assoc_cleanup(tpAniSirGlobal mac_ctx,
				      tpPESession session,
				      tpSirAssocReq assoc_req,
				      tpDphHashNode sta_ds,
				      tpSirAssocReq tmp_assoc_req,
				      bool *assoc_req_copied)
{
	if (assoc_req != NULL) {
		if (assoc_req->assocReqFrame) {
			qdf_mem_free(assoc_req->assocReqFrame);
			assoc_req->assocReqFrame = NULL;
			assoc_req->assocReqFrameLength = 0;
		}

		qdf_mem_free(assoc_req);
		/* to avoid double free */
		if (*assoc_req_copied && session->parsedAssocReq)
			session->parsedAssocReq[sta_ds->assocId] = NULL;
	}

	/* If it is not duplicate Assoc request then only make to Null */
	if ((sta_ds != NULL) &&
	    (sta_ds->mlmStaContext.mlmState != eLIM_MLM_WT_ADD_STA_RSP_STATE)) {
		if (session->parsedAssocReq != NULL) {
			tmp_assoc_req =
				session->parsedAssocReq[sta_ds->assocId];
			if (tmp_assoc_req != NULL) {
				if (tmp_assoc_req->assocReqFrame) {
					qdf_mem_free(
						tmp_assoc_req->assocReqFrame);
					tmp_assoc_req->assocReqFrame = NULL;
					tmp_assoc_req->assocReqFrameLength = 0;
				}
				qdf_mem_free(tmp_assoc_req);
				session->parsedAssocReq[sta_ds->assocId] = NULL;
			}
		}
	}
}

/**
 * lim_process_assoc_req_frame() - Process RE/ASSOC Request frame.
 * @mac_ctx: Pointer to Global MAC structure
 * @rx_pkt_info: A pointer to Buffer descriptor + associated PDUs
 * @sub_type: Indicates whether it is Association Request(=0) or Reassociation
 *            Request(=1) frame
 * @session: pe session entry
 *
 * This function is called to process RE/ASSOC Request frame.
 *
 * @Return: void
 */
void lim_process_assoc_req_frame(tpAniSirGlobal mac_ctx, uint8_t *rx_pkt_info,
				 uint8_t sub_type, tpPESession session)
{
	bool pmf_connection = false, assoc_req_copied = false;
	uint8_t update_ctx, *frm_body;
	uint16_t peer_idx, assoc_id = 0;
	uint32_t frame_len;
	uint32_t phy_mode;
	tHalBitVal qos_mode;
	tpSirMacMgmtHdr hdr;
	struct tLimPreAuthNode *sta_pre_auth_ctx;
	tAniAuthType auth_type;
	tSirMacCapabilityInfo local_cap;
	tpDphHashNode sta_ds = NULL;
	tpSirAssocReq assoc_req, tmp_assoc_req;

	lim_get_phy_mode(mac_ctx, &phy_mode, session);

	limGetQosMode(session, &qos_mode);

	hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);

	lim_log(mac_ctx, LOG1,
		FL("Rcvd %s Req Frame, sessionid: %d systemrole %d MlmState %d from: "
		   MAC_ADDRESS_STR),
		(LIM_ASSOC == sub_type) ? "Assoc" : "ReAssoc",
		session->peSessionId, GET_LIM_SYSTEM_ROLE(session),
		session->limMlmState, MAC_ADDR_ARRAY(hdr->sa));

	if (LIM_IS_STA_ROLE(session)) {
		lim_log(mac_ctx, LOGE,
			FL("Rcvd unexpected ASSOC REQ, sessionid: %d sys sub_type=%d for role=%d from: "
			   MAC_ADDRESS_STR),
			session->peSessionId, sub_type,
			GET_LIM_SYSTEM_ROLE(session),
			MAC_ADDR_ARRAY(hdr->sa));
		sir_dump_buf(mac_ctx, SIR_LIM_MODULE_ID, LOG3,
			     WMA_GET_RX_MPDU_DATA(rx_pkt_info), frame_len);
		return;
	}

	/*
	 * If a STA is already present in DPH and it is initiating a Assoc
	 * re-transmit, do not process it. This can happen when first Assoc Req
	 * frame is received but ACK lost at STA side. The ACK for this dropped
	 * Assoc Req frame should be sent by HW. Host simply does not process it
	 * once the entry for the STA is already present in DPH.
	 */
	sta_ds = dph_lookup_hash_entry(mac_ctx, hdr->sa, &assoc_id,
				&session->dph.dphHashTable);
	if (NULL != sta_ds) {
		if (hdr->fc.retry > 0) {
			lim_log(mac_ctx, LOGE,
			FL("STA is initiating Assoc Req after ACK lost. Do not process sessionid: %d sys sub_type=%d for role=%d from: "
				MAC_ADDRESS_STR), session->peSessionId,
			sub_type, GET_LIM_SYSTEM_ROLE(session),
			MAC_ADDR_ARRAY(hdr->sa));
		} else if (!sta_ds->rmfEnabled) {
			/*
			 * Do this only for non PMF case.
			 * STA might have missed the assoc response, so it is
			 * sending assoc request frame again.
			 */
			lim_send_assoc_rsp_mgmt_frame(mac_ctx, eSIR_SUCCESS,
					sta_ds->assocId, sta_ds->staAddr,
					sta_ds->mlmStaContext.subType,
					sta_ds, session);
			lim_log(mac_ctx, LOGE,
				FL("DUT already received an assoc request frame and STA is sending another assoc req.So, do not Process sessionid: %d sys sub_type=%d for role=%d from: "
					MAC_ADDRESS_STR),
				session->peSessionId, sub_type,
				session->limSystemRole,
				MAC_ADDR_ARRAY(hdr->sa));
		}
		return;
	}

	lim_check_sta_in_pe_entries(mac_ctx, hdr, session->peSessionId);

	/* Get pointer to Re/Association Request frame body */
	frm_body = WMA_GET_RX_MPDU_DATA(rx_pkt_info);

	if (lim_is_group_addr(hdr->sa)) {
		/*
		 * Rcvd Re/Assoc Req frame from BC/MC address Log error and
		 * ignore it
		 */
		lim_log(mac_ctx, LOGE,
			FL("Rcvd %s Req, sessionid: %d from a BC/MC address"
				MAC_ADDRESS_STR),
			(LIM_ASSOC == sub_type) ? "Assoc" : "ReAssoc",
			session->peSessionId, MAC_ADDR_ARRAY(hdr->sa));
		return;
	}

	sir_dump_buf(mac_ctx, SIR_LIM_MODULE_ID, LOG2,
		(uint8_t *) frm_body, frame_len);

	if (false == lim_chk_sa_da(mac_ctx, hdr, session, sub_type))
		return;

	if (false == lim_chk_tkip(mac_ctx, hdr, session, sub_type))
		return;

	/* Allocate memory for the Assoc Request frame */
	assoc_req = qdf_mem_malloc(sizeof(*assoc_req));
	if (NULL == assoc_req) {
		lim_log(mac_ctx, LOGP,
			FL("Allocate Memory failed in assoc_req"));
		return;
	}
	qdf_mem_set((void *)assoc_req, sizeof(*assoc_req), 0);

	/* Parse Assoc Request frame */
	if (false == lim_chk_assoc_req_parse_error(mac_ctx, hdr, session,
				assoc_req, sub_type, frm_body, frame_len))
		goto error;

	assoc_req->assocReqFrame = qdf_mem_malloc(frame_len);
	if (NULL == assoc_req->assocReqFrame) {
		lim_log(mac_ctx, LOGE,
			FL("Memory alloc failed for the assoc req, len=%d"),
			frame_len);
		goto error;
	}

	qdf_mem_copy((uint8_t *) assoc_req->assocReqFrame,
		(uint8_t *) frm_body, frame_len);
	assoc_req->assocReqFrameLength = frame_len;

	if (false == lim_chk_capab(mac_ctx, hdr, session, assoc_req,
				sub_type, &local_cap))
		goto error;

	update_ctx = false;

	if (false == lim_chk_ssid(mac_ctx, hdr, session, assoc_req, sub_type))
		goto error;

	if (false == lim_chk_rates(mac_ctx, hdr, session, assoc_req, sub_type))
		goto error;

	if (false == lim_chk_11g_only(mac_ctx, hdr, session, assoc_req,
				sub_type))
		goto error;

	if (false == lim_chk_11n_only(mac_ctx, hdr, session, assoc_req,
				sub_type))
		goto error;

	if (false == lim_chk_11ac_only(mac_ctx, hdr, session, assoc_req,
				sub_type))
		goto error;

	/* Spectrum Management (11h) specific checks */
	lim_process_for_spectrum_mgmt(mac_ctx, hdr, session,
				assoc_req, sub_type, local_cap);

	if (false == lim_chk_mcs(mac_ctx, hdr, session, assoc_req, sub_type))
		goto error;

	if (false == lim_chk_is_11b_sta_supported(mac_ctx, hdr, session,
				assoc_req, sub_type, phy_mode))
		goto error;

	/*
	 * Check for 802.11n HT caps compatibility; are HT Capabilities
	 * turned on in lim?
	 */
	lim_print_ht_cap(mac_ctx, session, assoc_req);

	if (false == lim_chk_n_process_wpa_rsn_ie(mac_ctx, hdr, session,
				assoc_req, sub_type, &pmf_connection))
		goto error;

	/* Extract 'associated' context for STA, if any. */
	sta_ds = dph_lookup_hash_entry(mac_ctx, hdr->sa, &peer_idx,
				&session->dph.dphHashTable);

	/* Extract pre-auth context for the STA, if any. */
	sta_pre_auth_ctx = lim_search_pre_auth_list(mac_ctx, hdr->sa);

	if (sta_ds == NULL) {
		if (false == lim_process_assoc_req_no_sta_ctx(mac_ctx, hdr,
				session, assoc_req, sub_type, sta_pre_auth_ctx,
				sta_ds, &auth_type))
			goto error;
	} else {
		if (false == lim_process_assoc_req_sta_ctx(mac_ctx, hdr,
				session, assoc_req, sub_type, sta_pre_auth_ctx,
				sta_ds, peer_idx, &auth_type, &update_ctx))
			goto error;
		goto sendIndToSme;
	}

	/* check if sta is allowed per QoS AC rules */
	if (false == lim_chk_wmm(mac_ctx, hdr, session,
				assoc_req, sub_type, qos_mode))
		goto error;

	/* STA is Associated ! */
	lim_log(mac_ctx, LOGE,
		FL("Received %s Req  successful from " MAC_ADDRESS_STR),
		(LIM_ASSOC == sub_type) ? "Assoc" : "ReAssoc",
		MAC_ADDR_ARRAY(hdr->sa));

	/*
	 * AID for this association will be same as the peer Index used in DPH
	 * table. Assign unused/least recently used peer Index from perStaDs.
	 * NOTE: lim_assign_peer_idx() assigns AID values ranging between
	 * 1 - cfg_item(WNI_CFG_ASSOC_STA_LIMIT)
	 */

	peer_idx = lim_assign_peer_idx(mac_ctx, session);

	if (!peer_idx) {
		/* Could not assign AID. Reject association */
		lim_log(mac_ctx, LOGE,
			FL("PeerIdx not avaialble. Reject associaton"));
		lim_reject_association(mac_ctx, hdr->sa, sub_type,
				true, auth_type, peer_idx, false,
				(tSirResultCodes)eSIR_MAC_UNSPEC_FAILURE_STATUS,
				session);
		goto error;
	}

	/* Add an entry to hash table maintained by DPH module */

	sta_ds = dph_add_hash_entry(mac_ctx, hdr->sa, peer_idx,
				&session->dph.dphHashTable);

	if (sta_ds == NULL) {
		/* Could not add hash table entry at DPH */
		lim_log(mac_ctx, LOGE,
			FL("couldn't add hash entry at DPH for aid=%d, MacAddr:"
			   MAC_ADDRESS_STR), peer_idx, MAC_ADDR_ARRAY(hdr->sa));

		/* Release AID */
		lim_release_peer_idx(mac_ctx, peer_idx, session);

		lim_reject_association(mac_ctx, hdr->sa, sub_type,
			true, auth_type, peer_idx, false,
			(tSirResultCodes) eSIR_MAC_UNSPEC_FAILURE_STATUS,
			session);
		goto error;
	}

sendIndToSme:
	if (false == lim_update_sta_ds(mac_ctx, hdr, session, assoc_req,
				sub_type, sta_ds, tmp_assoc_req, auth_type,
				&assoc_req_copied, peer_idx, qos_mode,
				pmf_connection))
		goto error;

	/* BTAMP: Storing the parsed assoc request in the session array */
	if (session->parsedAssocReq)
		session->parsedAssocReq[sta_ds->assocId] = assoc_req;
	assoc_req_copied = true;

	if (false == lim_update_sta_ctx(mac_ctx, session, assoc_req,
					sub_type, sta_ds, update_ctx))
		goto error;

	/* AddSta is sucess here */
	if (LIM_IS_AP_ROLE(session) && IS_DOT11_MODE_HT(session->dot11mode) &&
		assoc_req->HTCaps.present && assoc_req->wmeInfoPresent) {
		/*
		 * Update in the HAL Sta Table for the Update of the Protection
		 * Mode
		 */
		lim_post_sm_state_update(mac_ctx, sta_ds->staIndex,
					 sta_ds->htMIMOPSState, sta_ds->staAddr,
					 session->smeSessionId);
	}

	return;

error:

	lim_process_assoc_cleanup(mac_ctx, session, assoc_req, sta_ds,
				  tmp_assoc_req, &assoc_req_copied);
	return;
}

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
		qdf_mem_copy(&assoc_ind->wapiIE.wapiIEdata[2],
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
					tpPESession session_entry,
					tpSirAssocReq assoc_req,
					tpLimMlmAssocInd assoc_ind)
{
	uint8_t chan;

	if (session_entry->limRFBand == SIR_BAND_2_4_GHZ) {
		if (session_entry->vhtCapability && assoc_req->VHTCaps.present)
			assoc_ind->chan_info.info = MODE_11AC_VHT20_2G;
		else if (session_entry->htCapability
			    && assoc_req->HTCaps.present)
			assoc_ind->chan_info.info = MODE_11NG_HT20;
		else
			assoc_ind->chan_info.info = MODE_11G;
		return;
	}

	if (session_entry->vhtCapability && assoc_req->VHTCaps.present) {
		if ((session_entry->ch_width > CH_WIDTH_40MHZ)
		    && assoc_req->HTCaps.supportedChannelWidthSet) {
			chan = session_entry->ch_center_freq_seg0;
			assoc_ind->chan_info.band_center_freq1 =
				cds_chan_to_freq(chan);
			assoc_ind->chan_info.info = MODE_11AC_VHT80;
			return;
		}

		if ((session_entry->ch_width == CH_WIDTH_40MHZ)
			&& assoc_req->HTCaps.supportedChannelWidthSet) {
			assoc_ind->chan_info.info = MODE_11AC_VHT40;
			if (session_entry->htSecondaryChannelOffset ==
			    PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
				assoc_ind->chan_info.band_center_freq1 += 10;
			else
				assoc_ind->chan_info.band_center_freq1 -= 10;
			return;
		}

		assoc_ind->chan_info.info = MODE_11AC_VHT20;
		return;
	}

	if (session_entry->htCapability && assoc_req->HTCaps.present) {
		if ((session_entry->ch_width == CH_WIDTH_40MHZ)
		    && assoc_req->HTCaps.supportedChannelWidthSet) {
			assoc_ind->chan_info.info = MODE_11NA_HT40;
			if (session_entry->htSecondaryChannelOffset ==
			    PHY_DOUBLE_CHANNEL_LOW_PRIMARY)
				assoc_ind->chan_info.band_center_freq1 += 10;
			else
				assoc_ind->chan_info.band_center_freq1 -= 10;
			return;
		}

		assoc_ind->chan_info.info = MODE_11NA_HT20;
		return;
	}

	assoc_ind->chan_info.info = MODE_11A;
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

		assoc_ind = qdf_mem_malloc(temp);
		if (NULL == assoc_ind) {
			lim_release_peer_idx(mac_ctx, sta_ds->assocId,
				session_entry);
			lim_log(mac_ctx, LOGP,
				FL("AllocateMemory failed for assoc_ind"));
			return;
		}
		qdf_mem_set(assoc_ind, temp, 0);
		qdf_mem_copy((uint8_t *) assoc_ind->peerMacAddr,
			(uint8_t *) sta_ds->staAddr, sizeof(tSirMacAddr));
		assoc_ind->aid = sta_ds->assocId;
		qdf_mem_copy((uint8_t *) &assoc_ind->ssId,
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
			qdf_mem_copy(&assoc_ind->rsnIE.rsnIEdata[2],
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
				qdf_mem_free(assoc_ind);
				return;
			}
			assoc_ind->rsnIE.rsnIEdata[rsn_len] =
				SIR_MAC_WPA_EID;
			assoc_ind->rsnIE.rsnIEdata[rsn_len + 1]
				= assoc_req->wpa.length;
			qdf_mem_copy(
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
			qdf_mem_copy(&assoc_ind->addIE.addIEdata,
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
				qdf_mem_copy(
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
		assoc_ind->chan_info.nss = sta_ds->nss;
		assoc_ind->chan_info.rate_flags =
			lim_get_max_rate_flags(mac_ctx, sta_ds);
		/* updates VHT information in assoc indication */
		lim_fill_assoc_ind_vht_info(mac_ctx, session_entry, assoc_req,
			assoc_ind);
		lim_post_sme_message(mac_ctx, LIM_MLM_ASSOC_IND,
			 (uint32_t *) assoc_ind);
		qdf_mem_free(assoc_ind);
	}
	return;
}
