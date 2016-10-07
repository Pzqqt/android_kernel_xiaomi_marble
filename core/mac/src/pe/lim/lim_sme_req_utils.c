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
 * This file lim_sme_req_utils.cc contains the utility functions
 * for processing SME request messages.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 * 05/26/10       js             WPA handling in (Re)Assoc frames
 *
 */

#include "wni_api.h"
#include "wni_cfg.h"
#include "cfg_api.h"
#include "sir_api.h"
#include "sch_api.h"
#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_security_utils.h"
#include "lim_ser_des_utils.h"
#include "lim_sme_req_utils.h"

/**
 * lim_is_rs_nie_valid_in_sme_req_message()
 *
 * @mac_ctx   Pointer to Global MAC structure
 * @rsn_ie    Pointer to received RSN IE
 *
 * This function is called to verify if the RSN IE received in various SME_REQ
 * messages is valid or not
 *
 * Return: true when RSN IE is valid, false otherwise
 *
 */

static uint8_t
lim_is_rsn_ie_valid_in_sme_req_message(tpAniSirGlobal mac_ctx, tpSirRSNie rsn_ie)
{
	uint8_t start = 0;
	uint32_t privacy, val;
	int len;

	if (wlan_cfg_get_int(mac_ctx, WNI_CFG_PRIVACY_ENABLED,
			     &privacy) != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGP, FL("Unable to retrieve POI from CFG"));
	}

	if (wlan_cfg_get_int(mac_ctx, WNI_CFG_RSN_ENABLED, &val)
		!= eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGP,
			FL("Unable to retrieve RSN_ENABLED from CFG"));
	}

	if (rsn_ie->length && (!privacy || !val)) {
		/* Privacy & RSN not enabled in CFG.
		 * In order to allow mixed mode for Guest access
		 * allow BSS creation/join with no Privacy capability
		 * yet advertising WPA IE
		 */
		PELOG1(lim_log(mac_ctx, LOG1,
			       FL("RSN ie len %d PRIVACY %d RSN %d"),
			       rsn_ie->length, privacy, val);)
	}

	if (!rsn_ie->length)
		return true;

	if ((rsn_ie->rsnIEdata[0] != DOT11F_EID_RSN)
#ifdef FEATURE_WLAN_WAPI
	    && (rsn_ie->rsnIEdata[0] != DOT11F_EID_WAPI)
#endif
	    && (rsn_ie->rsnIEdata[0] != DOT11F_EID_WPA)) {
		lim_log(mac_ctx, LOGE, FL("RSN/WPA/WAPI EID %d not [%d || %d]"),
			rsn_ie->rsnIEdata[0], DOT11F_EID_RSN,
			DOT11F_EID_WPA);
		return false;
	}

	len = rsn_ie->length;
	start = 0;
	while (len > 0) {
		switch (rsn_ie->rsnIEdata[start]) {
		case DOT11F_EID_RSN:
		/* Check validity of RSN IE */
			if ((rsn_ie->rsnIEdata[start + 1] >
			    DOT11F_IE_RSN_MAX_LEN)
			    || (rsn_ie->rsnIEdata[start + 1] <
				DOT11F_IE_RSN_MIN_LEN)) {
				lim_log(mac_ctx, LOGE,
					FL("RSN IE len %d not [%d,%d]"),
					rsn_ie->rsnIEdata[start + 1],
					DOT11F_IE_RSN_MIN_LEN,
					DOT11F_IE_RSN_MAX_LEN);
				return false;
			}
			break;
		case DOT11F_EID_WPA:
			/* Check validity of WPA IE */
			if (SIR_MAC_MAX_IE_LENGTH <= start)
				break;

			if (start <= (SIR_MAC_MAX_IE_LENGTH - sizeof(uint32_t)))
				val = sir_read_u32((uint8_t *) &
					rsn_ie->rsnIEdata[start + 2]);

			if ((rsn_ie->rsnIEdata[start + 1] <
			     DOT11F_IE_WPA_MIN_LEN)
			    || (rsn_ie->rsnIEdata[start + 1] >
				DOT11F_IE_WPA_MAX_LEN)
			    || (SIR_MAC_WPA_OUI != val)) {
				lim_log(mac_ctx, LOGE,
					FL("WPA IE len %d not [%d,%d] OR data 0x%x not 0x%x"),
					rsn_ie->rsnIEdata[start + 1],
					DOT11F_IE_WPA_MIN_LEN,
					DOT11F_IE_WPA_MAX_LEN,
					val, SIR_MAC_WPA_OUI);
				return false;
			}
			break;
#ifdef FEATURE_WLAN_WAPI
		case DOT11F_EID_WAPI:
			if ((rsn_ie->rsnIEdata[start + 1] >
			    DOT11F_IE_WAPI_MAX_LEN)
			    || (rsn_ie->rsnIEdata[start + 1] <
				DOT11F_IE_WAPI_MIN_LEN)) {
				lim_log(mac_ctx, LOGE,
					FL("WAPI IE len %d not [%d,%d]"),
					rsn_ie->rsnIEdata[start + 1],
					DOT11F_IE_WAPI_MIN_LEN,
					DOT11F_IE_WAPI_MAX_LEN);
				return false;
			}
			break;
#endif
		default:
			/* we will never be here, simply for completeness */
			return false;
		} /* end of switch */
		/* EID + length field + length */
		start += 2 + rsn_ie->rsnIEdata[start + 1];
		len -= start;
	} /* end while loop */
	return true;
} /*** end lim_is_rs_nie_valid_in_sme_req_message() ***/

/**
 * lim_is_addie_valid_in_sme_req_message()
 *
 ***FUNCTION:
 * This function is called to verify if the Add IE
 * received in various SME_REQ messages is valid or not
 *
 ***LOGIC:
 * Add IE validity checks are performed on only length
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac   Pointer to Global MAC structure
 * @param  pWSCie Pointer to received WSC IE
 * @return true when WSC IE is valid, false otherwise
 */

static uint8_t
lim_is_addie_valid_in_sme_req_message(tpAniSirGlobal pMac, tpSirAddie pAddie)
{
	int left = pAddie->length;
	uint8_t *ptr = pAddie->addIEdata;
	uint8_t elem_id, elem_len;

	if (left == 0)
		return true;

	while (left >= 2) {
		elem_id = ptr[0];
		elem_len = ptr[1];
		left -= 2;
		if (elem_len > left) {
			lim_log(pMac, LOGE,
				FL
					("****Invalid Add IEs eid = %d elem_len=%d left=%d*****"),
				elem_id, elem_len, left);
			return false;
		}

		left -= elem_len;
		ptr += (elem_len + 2);
	}
	/* there shouldn't be any left byte */

	return true;
} /*** end lim_is_addie_valid_in_sme_req_message() ***/

/**
 * lim_set_rs_nie_wp_aiefrom_sme_start_bss_req_message() - to set rsnie/wpaie
 *
 * @mac_ctx : Pointer to Global MAC structure
 * @rsn_ie  : Pointer to received RSN IE
 * @session : Pointer to pe session
 *
 * This function is called to verify if the RSN IE received in various
 * SME_REQ messages is valid or not. RSN IE validity checks are performed in
 * this function
 *
 * Return: true when RSN IE is valid, false otherwise
 */
uint8_t
lim_set_rs_nie_wp_aiefrom_sme_start_bss_req_message(tpAniSirGlobal mac_ctx,
						    tpSirRSNie rsn_ie,
						    tpPESession session)
{
	uint8_t wpa_idx = 0;
	uint32_t privacy, val;

	if (wlan_cfg_get_int(mac_ctx, WNI_CFG_PRIVACY_ENABLED,
			     &privacy) != eSIR_SUCCESS)
		lim_log(mac_ctx, LOGP, FL("Unable to retrieve POI from CFG"));

	if (wlan_cfg_get_int(mac_ctx, WNI_CFG_RSN_ENABLED,
			     &val) != eSIR_SUCCESS)
		lim_log(mac_ctx, LOGP,
			FL("Unable to retrieve RSN_ENABLED from CFG"));

	if (rsn_ie->length && (!privacy || !val)) {
		/*
		 * Privacy & RSN not enabled in CFG.
		 * In order to allow mixed mode for Guest access
		 * allow BSS creation/join with no Privacy capability
		 * yet advertising WPA IE
		 */
		lim_log(mac_ctx, LOG1,
			FL("RSN ie len %d but PRIVACY %d RSN %d"),
			rsn_ie->length, privacy, val);
	}

	if (!rsn_ie->length)
		return true;

	if ((rsn_ie->rsnIEdata[0] != SIR_MAC_RSN_EID) &&
	    (rsn_ie->rsnIEdata[0] != SIR_MAC_WPA_EID)) {
		lim_log(mac_ctx, LOGE, FL("RSN/WPA EID %d not [%d || %d]"),
			rsn_ie->rsnIEdata[0], SIR_MAC_RSN_EID,
			SIR_MAC_WPA_EID);
		return false;
	}
	/* Check validity of RSN IE */
	if ((rsn_ie->rsnIEdata[0] == SIR_MAC_RSN_EID) &&
	    (rsn_ie->rsnIEdata[1] < SIR_MAC_RSN_IE_MIN_LENGTH)) {
		lim_log(mac_ctx, LOGE, FL("RSN IE len %d not [%d,%d]"),
			rsn_ie->rsnIEdata[1], SIR_MAC_RSN_IE_MIN_LENGTH,
			SIR_MAC_RSN_IE_MAX_LENGTH);
		return false;
	}

	if (rsn_ie->length > rsn_ie->rsnIEdata[1] + 2) {
		if (rsn_ie->rsnIEdata[0] != SIR_MAC_RSN_EID) {
			lim_log(mac_ctx, LOGE,
				FL("First byte[%d] in rsnIEdata isn't RSN_EID"),
				rsn_ie->rsnIEdata[1]);
			return false;
		}
		lim_log(mac_ctx, LOG1,
			FL("WPA IE is present along with WPA2 IE"));
		wpa_idx = 2 + rsn_ie->rsnIEdata[1];
	} else if ((rsn_ie->length == rsn_ie->rsnIEdata[1] + 2) &&
		   (rsn_ie->rsnIEdata[0] == SIR_MAC_RSN_EID)) {
		lim_log(mac_ctx, LOG1, FL("Only RSN IE is present"));
		dot11f_unpack_ie_rsn(mac_ctx, &rsn_ie->rsnIEdata[2],
				     (uint8_t) rsn_ie->length,
				     &session->gStartBssRSNIe);
	} else if ((rsn_ie->length == rsn_ie->rsnIEdata[1] + 2)
		   && (rsn_ie->rsnIEdata[0] == SIR_MAC_WPA_EID)) {
		lim_log(mac_ctx, LOG1, FL("Only WPA IE is present"));
		dot11f_unpack_ie_wpa(mac_ctx, &rsn_ie->rsnIEdata[6],
				     (uint8_t) rsn_ie->length - 4,
				     &session->gStartBssWPAIe);
	}
	/* Check validity of WPA IE */
	if (wpa_idx + 6 >= SIR_MAC_MAX_IE_LENGTH)
		return false;

	val = sir_read_u32((uint8_t *)&rsn_ie->rsnIEdata[wpa_idx + 2]);
	if ((rsn_ie->rsnIEdata[wpa_idx] == SIR_MAC_WPA_EID)
	    && ((rsn_ie->rsnIEdata[wpa_idx + 1] < SIR_MAC_WPA_IE_MIN_LENGTH)
	     || (SIR_MAC_WPA_OUI != val))) {
		lim_log(mac_ctx, LOGE,
			FL("WPA IE len %d not [%d,%d] OR data 0x%x not 0x%x"),
			rsn_ie->rsnIEdata[1],
			SIR_MAC_RSN_IE_MIN_LENGTH,
			SIR_MAC_RSN_IE_MAX_LENGTH, val,
			SIR_MAC_WPA_OUI);
		return false;
	} else {
		/* Both RSN and WPA IEs are present */
		dot11f_unpack_ie_rsn(mac_ctx, &rsn_ie->rsnIEdata[2],
				     (uint8_t) rsn_ie->length,
				     &session->gStartBssRSNIe);
		dot11f_unpack_ie_wpa(mac_ctx, &rsn_ie->rsnIEdata[wpa_idx + 6],
				     rsn_ie->rsnIEdata[wpa_idx + 1] - 4,
				     &session->gStartBssWPAIe);
	}
	return true;
}

/**
 * lim_is_bss_descr_valid_in_sme_req_message()
 *
 ***FUNCTION:
 * This function is called to verify if the BSS Descr
 * received in various SME_REQ messages is valid or not
 *
 ***LOGIC:
 * BSS Descritipion validity checks are performed in this function
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac      Pointer to Global MAC structure
 * @param  pBssDescr Pointer to received Bss Descritipion
 * @return true when BSS description is valid, false otherwise
 */

static uint8_t
lim_is_bss_descr_valid_in_sme_req_message(tpAniSirGlobal pMac,
					  tpSirBssDescription pBssDescr)
{
	uint8_t valid = true;

	if (lim_is_addr_bc(pBssDescr->bssId) || !pBssDescr->channelId) {
		valid = false;
		goto end;
	}

end:
	return valid;
} /*** end lim_is_bss_descr_valid_in_sme_req_message() ***/

/**
 * lim_is_sme_start_bss_req_valid() - To validate sme start bss request
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @start_bss_req: Pointer to received SME_START_BSS_REQ message
 *
 * This function is called by lim_process_sme_req_messages() upon
 * receiving SME_START_BSS_REQ message from application.
 *
 * Return: true when received SME_START_BSS_REQ is formatted correctly false
 *         otherwise
 */

uint8_t
lim_is_sme_start_bss_req_valid(tpAniSirGlobal mac_ctx,
			       tpSirSmeStartBssReq start_bss_req)
{
	uint8_t i = 0;
	tSirMacRateSet *opr_rates = &start_bss_req->operationalRateSet;

	PELOG1(lim_log(mac_ctx, LOG1,
	       FL("Parsed START_BSS_REQ fields are bssType=%s (%d), channelId=%d, SSID len=%d, rsnIE len=%d, nwType=%d, rateset len=%d"),
	       lim_bss_type_to_string(start_bss_req->bssType),
	       start_bss_req->bssType, start_bss_req->channelId,
	       start_bss_req->ssId.length, start_bss_req->rsnIE.length,
	       start_bss_req->nwType, opr_rates->numRates);)

	switch (start_bss_req->bssType) {
	case eSIR_INFRASTRUCTURE_MODE:
		/**
		 * Should not have received start BSS req with bssType
		 * Infrastructure on STA.
		 */
		lim_log(mac_ctx, LOGE,
			FL("Invalid bssType %d in eWNI_SME_START_BSS_REQ"),
			start_bss_req->bssType);
		return false;
		break;
	case eSIR_IBSS_MODE:
		break;
	case eSIR_INFRA_AP_MODE:
		break;
	case eSIR_NDI_MODE:
		break;
	default:
		/**
		 * Should not have received start BSS req with bssType
		 * other than Infrastructure/IBSS.
		 */
		lim_log(mac_ctx, LOGW,
			FL("Invalid bssType %d in eWNI_SME_START_BSS_REQ"),
			start_bss_req->bssType);
		return false;
	}

	if (start_bss_req->bssType == eSIR_IBSS_MODE
	    && (!start_bss_req->ssId.length
		|| start_bss_req->ssId.length > SIR_MAC_MAX_SSID_LENGTH)) {
		lim_log(mac_ctx, LOGW,
			FL("Invalid SSID length in eWNI_SME_START_BSS_REQ"));
		return false;
	}

	if (!lim_is_rsn_ie_valid_in_sme_req_message(mac_ctx,
						    &start_bss_req->rsnIE))
		return false;

	if (start_bss_req->nwType != eSIR_11A_NW_TYPE
	    && start_bss_req->nwType != eSIR_11B_NW_TYPE
	    && start_bss_req->nwType != eSIR_11G_NW_TYPE)
		return false;

	if (start_bss_req->nwType == eSIR_11A_NW_TYPE) {
		for (i = 0; i < opr_rates->numRates; i++) {
			if (sirIsArate(opr_rates->rate[i] & 0x7F))
				continue;

			lim_log(mac_ctx, LOGW,
				FL("Invalid operational 11A rates"));
			sir_dump_buf(mac_ctx, SIR_LIM_MODULE_ID, LOG2,
				     opr_rates->rate, opr_rates->numRates);
			return false;
		}
		return true;
	}
	/* check if all the rates in the opr rate set are legal 11G rates */
	if (start_bss_req->nwType == eSIR_11G_NW_TYPE) {
		for (i = 0; i < opr_rates->numRates; i++) {
			if (sirIsGrate(opr_rates->rate[i] & 0x7F))
				continue;

			lim_log(mac_ctx, LOGW,
				FL("Invalid operational 11G rates"));
			sir_dump_buf(mac_ctx, SIR_LIM_MODULE_ID, LOG2,
				     opr_rates->rate, opr_rates->numRates);
			return false;
		}
		return true;
	}

	for (i = 0; i < opr_rates->numRates; i++) {
		if (sirIsBrate(opr_rates->rate[i] & 0x7F))
			continue;

		lim_log(mac_ctx, LOGW,
			FL("Invalid operational 11B rates"));
		sir_dump_buf(mac_ctx, SIR_LIM_MODULE_ID, LOG2,
			     opr_rates->rate, opr_rates->numRates);
		return false;
	}
	return true;
}

/**
 * lim_is_sme_join_req_valid()
 *
 ***FUNCTION:
 * This function is called by lim_process_sme_req_messages() upon
 * receiving SME_JOIN_REQ message from application.
 *
 ***LOGIC:
 * Message validity checks are performed in this function
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac       Pointer to Global MAC structure
 * @param  pJoinReq   Pointer to received SME_JOIN_REQ message
 * @return true  when received SME_JOIN_REQ is formatted correctly
 *         false otherwise
 */

uint8_t lim_is_sme_join_req_valid(tpAniSirGlobal pMac, tpSirSmeJoinReq pJoinReq)
{
	uint8_t valid = true;

	if (!lim_is_rsn_ie_valid_in_sme_req_message(pMac, &pJoinReq->rsnIE)) {
		lim_log(pMac, LOGE,
			FL("received SME_JOIN_REQ with invalid RSNIE"));
		valid = false;
		goto end;
	}

	if (!lim_is_addie_valid_in_sme_req_message(pMac, &pJoinReq->addIEScan)) {
		lim_log(pMac, LOGE,
			FL
				("received SME_JOIN_REQ with invalid additional IE for scan"));
		valid = false;
		goto end;
	}

	if (!lim_is_addie_valid_in_sme_req_message(pMac, &pJoinReq->addIEAssoc)) {
		lim_log(pMac, LOGE,
			FL
				("received SME_JOIN_REQ with invalid additional IE for assoc"));
		valid = false;
		goto end;
	}

	if (!lim_is_bss_descr_valid_in_sme_req_message(pMac, &pJoinReq->bssDescription)) {
		/* / Received eWNI_SME_JOIN_REQ with invalid BSS Info */
		/* Log the event */
		lim_log(pMac, LOGE,
			FL("received SME_JOIN_REQ with invalid bssInfo"));

		valid = false;
		goto end;
	}

	/*
	   Reject Join Req if the Self Mac Address and
	   the Ap's Mac Address is same
	 */
	if (!qdf_mem_cmp((uint8_t *) pJoinReq->selfMacAddr,
			    (uint8_t *) pJoinReq->bssDescription.bssId,
			    (uint8_t) (sizeof(tSirMacAddr)))) {
		/* Log the event */
		lim_log(pMac, LOGE,
			FL
				("received SME_JOIN_REQ with Self Mac and BSSID Same"));

		valid = false;
		goto end;
	}

end:
	return valid;
} /*** end lim_is_sme_join_req_valid() ***/

/**
 * lim_is_sme_disassoc_req_valid()
 *
 ***FUNCTION:
 * This function is called by lim_process_sme_req_messages() upon
 * receiving SME_DISASSOC_REQ message from application.
 *
 ***LOGIC:
 * Message validity checks are performed in this function
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac         Pointer to Global MAC structure
 * @param  pDisassocReq Pointer to received SME_DISASSOC_REQ message
 * @return true         When received SME_DISASSOC_REQ is formatted
 *                      correctly
 *         false        otherwise
 */

uint8_t
lim_is_sme_disassoc_req_valid(tpAniSirGlobal pMac,
			      tpSirSmeDisassocReq pDisassocReq,
			      tpPESession psessionEntry)
{
	if (qdf_is_macaddr_group(&pDisassocReq->peer_macaddr) &&
	    !qdf_is_macaddr_broadcast(&pDisassocReq->peer_macaddr))
		return false;

	return true;
} /*** end lim_is_sme_disassoc_req_valid() ***/

/**
 * lim_is_sme_disassoc_cnf_valid()
 *
 ***FUNCTION:
 * This function is called by lim_process_sme_req_messages() upon
 * receiving SME_DISASSOC_CNF message from application.
 *
 ***LOGIC:
 * Message validity checks are performed in this function
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac         Pointer to Global MAC structure
 * @param  pDisassocCnf Pointer to received SME_DISASSOC_REQ message
 * @return true         When received SME_DISASSOC_CNF is formatted
 *                      correctly
 *         false        otherwise
 */

uint8_t
lim_is_sme_disassoc_cnf_valid(tpAniSirGlobal pMac,
			      tpSirSmeDisassocCnf pDisassocCnf,
			      tpPESession psessionEntry)
{
	if (qdf_is_macaddr_group(&pDisassocCnf->peer_macaddr))
		return false;

	return true;
} /*** end lim_is_sme_disassoc_cnf_valid() ***/

/**
 * lim_is_sme_deauth_req_valid()
 *
 ***FUNCTION:
 * This function is called by lim_process_sme_req_messages() upon
 * receiving SME_DEAUTH_REQ message from application.
 *
 ***LOGIC:
 * Message validity checks are performed in this function
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac       Pointer to Global MAC structure
 * @param  pDeauthReq Pointer to received SME_DEAUTH_REQ message
 * @return true       When received SME_DEAUTH_REQ is formatted correctly
 *         false      otherwise
 */

uint8_t
lim_is_sme_deauth_req_valid(tpAniSirGlobal pMac, tpSirSmeDeauthReq pDeauthReq,
			    tpPESession psessionEntry)
{
	if (qdf_is_macaddr_group(&pDeauthReq->peer_macaddr) &&
	    !qdf_is_macaddr_broadcast(&pDeauthReq->peer_macaddr))
		return false;

	return true;
} /*** end lim_is_sme_deauth_req_valid() ***/

/**
 * lim_is_sme_scan_req_valid()
 *
 ***FUNCTION:
 * This function is called by lim_process_sme_req_messages() upon
 * receiving SME_SCAN_REQ message from application.
 *
 ***LOGIC:
 * Message validity checks are performed in this function
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pScanReq Pointer to received SME_SCAN_REQ message
 * @return true  when received SME_SCAN_REQ is formatted correctly
 *         false otherwise
 */

uint8_t lim_is_sme_scan_req_valid(tpAniSirGlobal pMac, tpSirSmeScanReq pScanReq)
{
	uint8_t valid = true;
	uint8_t i = 0;

	if (pScanReq->numSsid > SIR_SCAN_MAX_NUM_SSID) {
		valid = false;
		lim_log(pMac, LOGE,
			FL("Number of SSIDs > SIR_SCAN_MAX_NUM_SSID"));
		goto end;
	}

	for (i = 0; i < pScanReq->numSsid; i++) {
		if (pScanReq->ssId[i].length > SIR_MAC_MAX_SSID_LENGTH) {
			lim_log(pMac, LOGE,
				FL
					("Requested SSID length > SIR_MAC_MAX_SSID_LENGTH"));
			valid = false;
			goto end;
		}
	}
	if ((pScanReq->bssType < 0) || (pScanReq->bssType > eSIR_AUTO_MODE)) {
		lim_log(pMac, LOGE, FL("Invalid BSS Type"));
		valid = false;
	}
	if (qdf_is_macaddr_group(&pScanReq->bssId) &&
		!qdf_is_macaddr_broadcast(&pScanReq->bssId)) {
		valid = false;
		lim_log(pMac, LOGE,
			FL("BSSID is group addr and is not Broadcast Addr"));
	}
	if (!
	    (pScanReq->scanType == eSIR_PASSIVE_SCAN
	     || pScanReq->scanType == eSIR_ACTIVE_SCAN)) {
		valid = false;
		lim_log(pMac, LOGE, FL("Invalid Scan Type"));
	}
	if (pScanReq->channelList.numChannels > SIR_MAX_NUM_CHANNELS) {
		valid = false;
		lim_log(pMac, LOGE,
			FL("Number of Channels > SIR_MAX_NUM_CHANNELS"));
	}

	/*
	** check min/max channelTime range
	**/
	if (valid) {
		if ((pScanReq->scanType == eSIR_ACTIVE_SCAN) &&
		    (pScanReq->maxChannelTime < pScanReq->minChannelTime)) {
			lim_log(pMac, LOGE,
				FL("Max Channel Time < Min Channel Time"));
			valid = false;
			goto end;
		}
	}

end:
	return valid;
} /*** end lim_is_sme_scan_req_valid() ***/

/**
 * lim_is_sme_set_context_req_valid()
 *
 ***FUNCTION:
 * This function is called by lim_process_sme_req_messages() upon
 * receiving SME_SET_CONTEXT_REQ message from application.
 *
 ***LOGIC:
 * Message validity checks are performed in this function
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMsg - Pointer to received SME_SET_CONTEXT_REQ message
 * @return true  when received SME_SET_CONTEXT_REQ is formatted correctly
 *         false otherwise
 */

uint8_t
lim_is_sme_set_context_req_valid(tpAniSirGlobal pMac,
				 tpSirSmeSetContextReq pSetContextReq)
{
	uint8_t i = 0;
	uint8_t valid = true;
	tpSirKeys pKey = pSetContextReq->keyMaterial.key;

	if ((pSetContextReq->keyMaterial.edType != eSIR_ED_WEP40) &&
	    (pSetContextReq->keyMaterial.edType != eSIR_ED_WEP104) &&
	    (pSetContextReq->keyMaterial.edType != eSIR_ED_NONE) &&
#ifdef FEATURE_WLAN_WAPI
	    (pSetContextReq->keyMaterial.edType != eSIR_ED_WPI) &&
#endif
	    !pSetContextReq->keyMaterial.numKeys) {
		/**
		 * No keys present in case of TKIP or CCMP
		 * Log error.
		 */
		lim_log(pMac, LOGW,
			FL
				("No keys present in SME_SETCONTEXT_REQ for edType=%d"),
			pSetContextReq->keyMaterial.edType);

		valid = false;
		goto end;
	}

	if (pSetContextReq->keyMaterial.numKeys &&
	    (pSetContextReq->keyMaterial.edType == eSIR_ED_NONE)) {
		/**
		 * Keys present in case of no ED policy
		 * Log error.
		 */
		lim_log(pMac, LOGW,
			FL("Keys present in SME_SETCONTEXT_REQ for edType=%d"),
			pSetContextReq->keyMaterial.edType);

		valid = false;
		goto end;
	}

	if (pSetContextReq->keyMaterial.edType >= eSIR_ED_NOT_IMPLEMENTED) {
		/**
		 * Invalid edType in the message
		 * Log error.
		 */
		lim_log(pMac, LOGW,
			FL("Invalid edType=%d in SME_SETCONTEXT_REQ"),
			pSetContextReq->keyMaterial.edType);

		valid = false;
		goto end;
	} else if (pSetContextReq->keyMaterial.edType > eSIR_ED_NONE) {
		uint32_t poi;

		if (wlan_cfg_get_int(pMac, WNI_CFG_PRIVACY_ENABLED,
				     &poi) != eSIR_SUCCESS) {
			lim_log(pMac, LOGP,
				FL("Unable to retrieve POI from CFG"));
		}

		if (!poi) {
			/**
			 * Privacy is not enabled
			 * In order to allow mixed mode for Guest access
			 * allow BSS creation/join with no Privacy capability
			 * yet advertising WPA IE
			 */
			PELOG1(lim_log(pMac, LOG1,
				       FL
					       ("Privacy is not enabled, yet non-None EDtype=%d in SME_SETCONTEXT_REQ"),
				       pSetContextReq->keyMaterial.edType);
			       )
		}
	}

	for (i = 0; i < pSetContextReq->keyMaterial.numKeys; i++) {
		if (((pSetContextReq->keyMaterial.edType == eSIR_ED_WEP40) &&
		     (pKey->keyLength != 5)) ||
		    ((pSetContextReq->keyMaterial.edType == eSIR_ED_WEP104) &&
		     (pKey->keyLength != 13)) ||
		    ((pSetContextReq->keyMaterial.edType == eSIR_ED_TKIP) &&
		     (pKey->keyLength != 32)) ||
#ifdef FEATURE_WLAN_WAPI
		    ((pSetContextReq->keyMaterial.edType == eSIR_ED_WPI) &&
		     (pKey->keyLength != 32)) ||
#endif
		    ((pSetContextReq->keyMaterial.edType == eSIR_ED_CCMP) &&
		     (pKey->keyLength != 16))) {
			/**
			 * Invalid key length for a given ED type
			 * Log error.
			 */
			lim_log(pMac, LOGW,
				FL
					("Invalid keyLength =%d for edType=%d in SME_SETCONTEXT_REQ"),
				pKey->keyLength,
				pSetContextReq->keyMaterial.edType);

			valid = false;
			goto end;
		}
		pKey++;
	}

end:
	return valid;
} /*** end lim_is_sme_set_context_req_valid() ***/

/**
 * lim_is_sme_stop_bss_req_valid()
 *
 ***FUNCTION:
 * This function is called by lim_process_sme_req_messages() upon
 * receiving SME_STOP_BSS_REQ message from application.
 *
 ***LOGIC:
 * Message validity checks are performed in this function
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMsg - Pointer to received SME_STOP_BSS_REQ message
 * @return true  when received SME_STOP_BSS_REQ is formatted correctly
 *         false otherwise
 */

uint8_t lim_is_sme_stop_bss_req_valid(uint32_t *pMsg)
{
	uint8_t valid = true;

	return valid;
} /*** end lim_is_sme_stop_bss_req_valid() ***/

/**
 * lim_get_bss_id_from_sme_join_req_msg()
 *
 ***FUNCTION:
 * This function is called in various places to get BSSID
 * from BSS description/Neighbor BSS Info in the SME_JOIN_REQ/
 * SME_REASSOC_REQ message.
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param     pBuf   - Pointer to received SME_JOIN/SME_REASSOC_REQ
 *                     message
 * @return    pBssId - Pointer to BSSID
 */

uint8_t *lim_get_bss_id_from_sme_join_req_msg(uint8_t *pBuf)
{
	if (!pBuf)
		return NULL;

	pBuf += sizeof(uint32_t);       /* skip message header */

	pBuf += lim_get_u16(pBuf) + sizeof(uint16_t);     /* skip RSN IE */

	pBuf += sizeof(uint16_t);       /* skip length of BSS description */

	return pBuf;
} /*** end lim_get_bss_id_from_sme_join_req_msg() ***/
