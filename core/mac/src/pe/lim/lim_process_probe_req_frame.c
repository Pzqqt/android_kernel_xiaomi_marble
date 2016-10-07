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
 * This file lim_process_probe_req_frame.cc contains the code
 * for processing Probe Request Frame.
 * Author:        Chandra Modumudi
 * Date:          02/28/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "wni_cfg.h"
#include "ani_global.h"
#include "cfg_api.h"

#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_ser_des_utils.h"
#include "parser_api.h"
#include "lim_ft_defs.h"
#include "lim_session.h"

void

lim_send_sme_probe_req_ind(tpAniSirGlobal pMac,
			   tSirMacAddr peerMacAddr,
			   uint8_t *pProbeReqIE,
			   uint32_t ProbeReqIELen, tpPESession psessionEntry);

/**
 * lim_get_wpspbc_sessions() - to get wps pbs sessions
 * @mac_ctx: Pointer to Global MAC structure
 * @addr: probe request source MAC addresss
 * @uuid_e: A pointer to UUIDE element of WPS IE in WPS PBC probe request
 * @session: A pointer to station PE session
 *
 * This function is called to query the WPS PBC overlap. This function
 * check WPS PBC probe request link list for PBC overlap
 *
 * @return None
 */

void lim_get_wpspbc_sessions(tpAniSirGlobal mac_ctx, struct qdf_mac_addr addr,
		uint8_t *uuid_e, eWPSPBCOverlap *overlap,
		tpPESession session)
{
	int count = 0;
	tSirWPSPBCSession *pbc;
	uint32_t cur_time;

	cur_time = (uint32_t) (qdf_mc_timer_get_system_ticks() /
						QDF_TICKS_PER_SECOND);
	qdf_zero_macaddr(&addr);
	qdf_mem_set((uint8_t *) uuid_e, SIR_WPS_UUID_LEN, 0);
	for (pbc = session->pAPWPSPBCSession; pbc; pbc = pbc->next) {
		if (cur_time > pbc->timestamp + SIR_WPS_PBC_WALK_TIME)
			break;
		count++;
		if (count > 1)
			break;
		qdf_copy_macaddr(&addr, &pbc->addr);
		qdf_mem_copy((uint8_t *) uuid_e, (uint8_t *) pbc->uuid_e,
				SIR_WPS_UUID_LEN);
	}
	if (count > 1)
		/* Overlap */
		*overlap = eSAP_WPSPBC_OVERLAP_IN120S;
	else if (count == 0)
		/* no WPS probe request in 120 second */
		*overlap = eSAP_WPSPBC_NO_WPSPBC_PROBE_REQ_IN120S;
	else
		/* One WPS probe request in 120 second */
		*overlap = eSAP_WPSPBC_ONE_WPSPBC_PROBE_REQ_IN120S;

	lim_log(mac_ctx, LOGE, FL("overlap = %d"), *overlap);
	sir_dump_buf(mac_ctx, SIR_LIM_MODULE_ID, LOGE, addr.bytes,
			QDF_MAC_ADDR_SIZE);
	sir_dump_buf(mac_ctx, SIR_LIM_MODULE_ID, LOGE, uuid_e,
			SIR_WPS_UUID_LEN);
	return;
}

/**
 * lim_remove_timeout_pbc_sessions() - remove pbc probe req entries.
 * @pMac - Pointer to Global MAC structure
 * @pbc - The beginning entry in WPS PBC probe request link list
 *
 * This function is called to remove the WPS PBC probe request entries from
 * specific entry to end.
 *
 * Return - None
 */
static void lim_remove_timeout_pbc_sessions(tpAniSirGlobal pMac,
					    tSirWPSPBCSession *pbc)
{
	tSirWPSPBCSession *prev;

	while (pbc) {
		prev = pbc;
		pbc = pbc->next;

		PELOG4(lim_log(pMac, LOG4, FL("WPS PBC sessions remove"));)
		PELOG4(sir_dump_buf
			       (pMac, SIR_LIM_MODULE_ID, LOG4, prev->addr,
			       sizeof(tSirMacAddr));
		       )
		PELOG4(sir_dump_buf
			       (pMac, SIR_LIM_MODULE_ID, LOG4, prev->uuid_e,
			       SIR_WPS_UUID_LEN);
		       )

		qdf_mem_free(prev);
	}
}

/**
 * lim_remove_pbc_sessions() - Remove PBC sessions
 * @mac: Pointer to Global MAC structure
 * @remove_mac: MAC Address of STA in WPS Session to be removed
 * @session_entry: session entry
 *
 * Return: none
 */
void lim_remove_pbc_sessions(tpAniSirGlobal mac, struct qdf_mac_addr remove_mac,
			     tpPESession session_entry)
{
	tSirWPSPBCSession *pbc, *prev = NULL;
	prev = pbc = session_entry->pAPWPSPBCSession;

	while (pbc) {
		if (qdf_is_macaddr_equal(&pbc->addr, &remove_mac)) {
			prev->next = pbc->next;
			if (pbc == session_entry->pAPWPSPBCSession)
				session_entry->pAPWPSPBCSession = pbc->next;
			qdf_mem_free(pbc);
			return;
		}
		prev = pbc;
		pbc = pbc->next;
	}
}

/**
 * lim_update_pbc_session_entry
 *
 ***FUNCTION:
 * This function is called when probe request with WPS PBC IE is received
 *
 ***LOGIC:
 * This function add the WPS PBC probe request in the WPS PBC probe request link list
 * The link list is in decreased time order of probe request that is received.
 * The entry that is more than 120 second is removed.
 *
 ***ASSUMPTIONS:
 *
 *
 ***NOTE:
 *
 * @param  pMac   Pointer to Global MAC structure
 * @param  addr   A pointer to probe request source MAC addresss
 * @param  uuid_e A pointer to UUIDE element of WPS IE
 * @param  psessionEntry   A pointer to station PE session
 *
 * @return None
 */

static void lim_update_pbc_session_entry(tpAniSirGlobal pMac,
					 uint8_t *addr, uint8_t *uuid_e,
					 tpPESession psessionEntry)
{
	tSirWPSPBCSession *pbc, *prev = NULL;

	uint32_t curTime;

	curTime =
		(uint32_t) (qdf_mc_timer_get_system_ticks() /
			    QDF_TICKS_PER_SECOND);

	PELOG4(lim_log
		       (pMac, LOG4, FL("Receive WPS probe reques curTime=%d"), curTime);
	       )
	PELOG4(sir_dump_buf
		       (pMac, SIR_LIM_MODULE_ID, LOG4, addr, sizeof(tSirMacAddr));
	       )
	PELOG4(sir_dump_buf
		       (pMac, SIR_LIM_MODULE_ID, LOG4, uuid_e, SIR_WPS_UUID_LEN);
	       )

	pbc = psessionEntry->pAPWPSPBCSession;

	while (pbc) {
		if ((!qdf_mem_cmp
			    ((uint8_t *) pbc->addr.bytes, (uint8_t *) addr,
			    QDF_MAC_ADDR_SIZE))
		    && (!qdf_mem_cmp((uint8_t *) pbc->uuid_e,
				       (uint8_t *) uuid_e, SIR_WPS_UUID_LEN))) {
			if (prev)
				prev->next = pbc->next;
			else
				psessionEntry->pAPWPSPBCSession = pbc->next;
			break;
		}
		prev = pbc;
		pbc = pbc->next;
	}

	if (!pbc) {
		pbc = qdf_mem_malloc(sizeof(tSirWPSPBCSession));
		if (NULL == pbc) {
			PELOGE(lim_log
				       (pMac, LOGE, FL("memory allocate failed!"));
			       )
			return;
		}
		qdf_mem_copy((uint8_t *) pbc->addr.bytes, (uint8_t *) addr,
			     QDF_MAC_ADDR_SIZE);

		if (uuid_e)
			qdf_mem_copy((uint8_t *) pbc->uuid_e,
				     (uint8_t *) uuid_e, SIR_WPS_UUID_LEN);
	}

	pbc->next = psessionEntry->pAPWPSPBCSession;
	psessionEntry->pAPWPSPBCSession = pbc;
	pbc->timestamp = curTime;

	/* remove entries that have timed out */
	prev = pbc;
	pbc = pbc->next;

	while (pbc) {
		if (curTime > pbc->timestamp + SIR_WPS_PBC_WALK_TIME) {
			prev->next = NULL;
			lim_remove_timeout_pbc_sessions(pMac, pbc);
			break;
		}
		prev = pbc;
		pbc = pbc->next;
	}
}

/**
 * lim_wpspbc_close
 *
 ***FUNCTION:
 * This function is called when BSS is closed
 *
 ***LOGIC:
 * This function remove all the WPS PBC entries
 *
 ***ASSUMPTIONS:
 *
 *
 ***NOTE:
 *
 * @param  pMac   Pointer to Global MAC structure
 * @param  psessionEntry   A pointer to station PE session
 *
 * @return None
 */

void lim_wpspbc_close(tpAniSirGlobal pMac, tpPESession psessionEntry)
{

	lim_remove_timeout_pbc_sessions(pMac, psessionEntry->pAPWPSPBCSession);

}

/**
 * lim_check11b_rates
 *
 ***FUNCTION:
 * This function is called by lim_process_probe_req_frame() upon
 * Probe Request frame reception.
 *
 ***LOGIC:
 * This function check 11b rates in supportedRates and extendedRates rates
 *
 ***NOTE:
 *
 * @param  rate
 *
 * @return BOOLEAN
 */

static bool lim_check11b_rates(uint8_t rate)
{
	if ((0x02 == (rate))
	    || (0x04 == (rate))
	    || (0x0b == (rate))
	    || (0x16 == (rate))
	    ) {
		return true;
	}
	return false;
}

/**
 * lim_process_probe_req_frame: to process probe req frame
 * @mac_ctx: Pointer to Global MAC structure
 * @rx_pkt_info: A pointer to Buffer descriptor + associated PDUs
 * @session: a ponter to session entry
 *
 * This function is called by limProcessMessageQueue() upon
 * Probe Request frame reception. This function processes received
 * Probe Request frame and responds with Probe Response.
 * Only AP or STA in IBSS mode that sent last Beacon will respond to
 * Probe Request.
 * ASSUMPTIONS:
 * 1. AP or STA in IBSS mode that sent last Beacon will always respond
 *    to Probe Request received with broadcast SSID.
 * NOTE:
 * 1. Dunno what to do with Rates received in Probe Request frame
 * 2. Frames with out-of-order fields/IEs are dropped.
 *
 *
 * Return: none
 */

void
lim_process_probe_req_frame(tpAniSirGlobal mac_ctx, uint8_t *rx_pkt_info,
		tpPESession session)
{
	uint8_t *body_ptr;
	tpSirMacMgmtHdr mac_hdr;
	uint32_t frame_len;
	tSirProbeReq probe_req;
	tAniSSID ssid;

	/* Don't send probe responses if disabled */
	if (mac_ctx->lim.gLimProbeRespDisableFlag)
		return;

	/*
	 * Don't send probe response if P2P go is scanning till scan
	 * come to idle state.
	 */
	if ((session->pePersona == QDF_P2P_GO_MODE) &&
		((mac_ctx->lim.gpLimRemainOnChanReq) ||
		 (mac_ctx->lim.gLimHalScanState != eLIM_HAL_IDLE_SCAN_STATE))) {
		lim_log(mac_ctx, LOG3,
			FL("GO is scanning, don't send probersp on diff chnl"));
		return;
	}
	mac_hdr = WMA_GET_RX_MAC_HEADER(rx_pkt_info);
	if (LIM_IS_AP_ROLE(session) ||
		(LIM_IS_IBSS_ROLE(session) &&
			 (WMA_GET_RX_BEACON_SENT(rx_pkt_info)))) {
		frame_len = WMA_GET_RX_PAYLOAD_LEN(rx_pkt_info);

		lim_log(mac_ctx, LOG3,
			FL("Received Probe Request %d bytes from "),
			frame_len);
			lim_print_mac_addr(mac_ctx, mac_hdr->sa, LOG3);
		/* Get pointer to Probe Request frame body */
		body_ptr = WMA_GET_RX_MPDU_DATA(rx_pkt_info);

		/* check for vendor IE presence */
		if ((session->access_policy_vendor_ie) &&
			(session->access_policy ==
			LIM_ACCESS_POLICY_RESPOND_IF_IE_IS_PRESENT)) {
			if (!cfg_get_vendor_ie_ptr_from_oui(mac_ctx,
				&session->access_policy_vendor_ie[2],
				3, body_ptr, frame_len)) {
				lim_log(mac_ctx, LOG1, FL(
					"Vendor IE is not present and access policy is %x, dropping probe request"),
					session->access_policy);
				return;
			}
		}

		/* Parse Probe Request frame */
		if (sir_convert_probe_req_frame2_struct(mac_ctx, body_ptr,
				frame_len, &probe_req) == eSIR_FAILURE) {
			lim_log(mac_ctx, LOGE,
				FL("Parse error ProbeReq, length=%d, SA is: "
					MAC_ADDRESS_STR), frame_len,
					MAC_ADDR_ARRAY(mac_hdr->sa));
			mac_ctx->sys.probeError++;
			return;
		}
		if (session->pePersona == QDF_P2P_GO_MODE) {
			uint8_t i = 0, rate_11b = 0, other_rates = 0;
			/* Check 11b rates in supported rates */
			for (i = 0; i < probe_req.supportedRates.numRates;
				i++) {
				if (lim_check11b_rates(
					probe_req.supportedRates.rate[i] &
								0x7f))
					rate_11b++;
				else
					other_rates++;
			}

			/* Check 11b rates in extended rates */
			for (i = 0; i < probe_req.extendedRates.numRates; i++) {
				if (lim_check11b_rates(
					probe_req.extendedRates.rate[i] & 0x7f))
					rate_11b++;
				else
					other_rates++;
			}

			if ((rate_11b > 0) && (other_rates == 0)) {
				lim_log(mac_ctx, LOG3,
					FL("Received a probe req frame with only 11b rates, SA is: "));
					lim_print_mac_addr(mac_ctx,
						mac_hdr->sa, LOG3);
					return;
			}
		}
		if (LIM_IS_AP_ROLE(session) &&
			((session->APWPSIEs.SirWPSProbeRspIE.FieldPresent
				& SIR_WPS_PROBRSP_VER_PRESENT)
			&& (probe_req.wscIePresent == 1)
			&& (probe_req.probeReqWscIeInfo.DevicePasswordID.id ==
				WSC_PASSWD_ID_PUSH_BUTTON)
			&& (probe_req.probeReqWscIeInfo.UUID_E.present == 1))) {
			if (session->fwdWPSPBCProbeReq) {
				sir_dump_buf(mac_ctx, SIR_LIM_MODULE_ID,
					LOG4, mac_hdr->sa, sizeof(tSirMacAddr));
				sir_dump_buf(mac_ctx, SIR_LIM_MODULE_ID,
					LOG4, body_ptr, frame_len);
				lim_send_sme_probe_req_ind(mac_ctx, mac_hdr->sa,
					body_ptr, frame_len, session);
			} else {
				lim_update_pbc_session_entry(mac_ctx,
					mac_hdr->sa,
					probe_req.probeReqWscIeInfo.UUID_E.uuid,
					session);
			}
		}
		ssid.length = session->ssId.length;
		/* Copy the SSID from sessio entry to local variable */
		qdf_mem_copy(ssid.ssId, session->ssId.ssId,
				session->ssId.length);

		/*
		 * Compare received SSID with current SSID. If they match,
		 * reply with Probe Response
		 */
		if (probe_req.ssId.length) {
			if (!ssid.length)
				goto multipleSSIDcheck;

			if (!qdf_mem_cmp((uint8_t *) &ssid,
						(uint8_t *) &(probe_req.ssId),
						(uint8_t) (ssid.length + 1))) {
				lim_send_probe_rsp_mgmt_frame(mac_ctx,
						mac_hdr->sa, &ssid,
						DPH_USE_MGMT_STAID,
						DPH_NON_KEEPALIVE_FRAME,
						session,
						probe_req.p2pIePresent);
				return;
			} else if (session->pePersona ==
					QDF_P2P_GO_MODE) {
				uint8_t direct_ssid[7] = "DIRECT-";
				uint8_t direct_ssid_len = 7;
				if (!qdf_mem_cmp((uint8_t *) &direct_ssid,
					(uint8_t *) &(probe_req.ssId.ssId),
					(uint8_t) (direct_ssid_len))) {
					lim_send_probe_rsp_mgmt_frame(mac_ctx,
							mac_hdr->sa,
							&ssid,
							DPH_USE_MGMT_STAID,
							DPH_NON_KEEPALIVE_FRAME,
							session,
							probe_req.p2pIePresent);
					return;
				}
			} else {
				lim_log(mac_ctx, LOG3,
					FL("Ignore ProbeReq frm with unmatch SSID received from "));
					lim_print_mac_addr(mac_ctx, mac_hdr->sa,
						LOG3);
					mac_ctx->sys.probeBadSsid++;
			}
		} else {
			/*
			 * Broadcast SSID in the Probe Request.
			 * Reply with SSID we're configured with.
			 * Turn off the SSID length to 0 if hidden SSID feature
			 * is present
			 */
			if (session->ssidHidden)
				/*
				 * We are returning from here as probe request
				 * contains the broadcast SSID. So no need to
				 * send the probe resp
				 */
				return;
			lim_send_probe_rsp_mgmt_frame(mac_ctx, mac_hdr->sa,
					&ssid,
					DPH_USE_MGMT_STAID,
					DPH_NON_KEEPALIVE_FRAME,
					session,
					probe_req.p2pIePresent);
			return;
		}
multipleSSIDcheck:
		lim_log(mac_ctx, LOG3,
			FL("Ignore ProbeReq frm with unmatch SSID rcved from"));
			lim_print_mac_addr(mac_ctx, mac_hdr->sa, LOG3);
		mac_ctx->sys.probeBadSsid++;
	} else {
		/* Ignore received Probe Request frame */
		lim_log(mac_ctx, LOG3,
			FL("Ignoring Probe Request frame received from "));
		lim_print_mac_addr(mac_ctx, mac_hdr->sa, LOG3);
		mac_ctx->sys.probeIgnore++;
	}
	return;
}

/**
 * lim_indicate_probe_req_to_hdd
 *
 ***FUNCTION:
 * This function is called by lim_process_probe_req_frame_multiple_bss() upon
 * Probe Request frame reception.
 *
 ***LOGIC:
 * This function processes received Probe Request frame and Pass
 * Probe Request Frame to HDD.
 *
 * @param  pMac              Pointer to Global MAC structure
 * @param  *pBd              A pointer to Buffer descriptor + associated PDUs
 * @param  psessionEntry     A pointer to PE session
 *
 * @return None
 */

static void
lim_indicate_probe_req_to_hdd(tpAniSirGlobal pMac, uint8_t *pBd,
			      tpPESession psessionEntry)
{
	tpSirMacMgmtHdr pHdr;
	uint32_t frameLen;

	lim_log(pMac, LOG1, "Received a probe request frame");

	pHdr = WMA_GET_RX_MAC_HEADER(pBd);
	frameLen = WMA_GET_RX_PAYLOAD_LEN(pBd);

	/* send the probe req to SME. */
	lim_send_sme_mgmt_frame_ind(pMac, pHdr->fc.subType,
				    (uint8_t *) pHdr,
				    (frameLen + sizeof(tSirMacMgmtHdr)),
				    psessionEntry->smeSessionId, WMA_GET_RX_CH(pBd),
				    psessionEntry, 0);
} /*** end lim_indicate_probe_req_to_hdd() ***/

/**
 * lim_process_probe_req_frame_multiple_bss() - to process probe req
 * @mac_ctx: Pointer to Global MAC structure
 * @buf_descr: A pointer to Buffer descriptor + associated PDUs
 * @session: A pointer to PE session
 *
 * This function is called by limProcessMessageQueue() upon
 * Probe Request frame reception. This function call
 * lim_indicate_probe_req_to_hdd function to indicate
 * Probe Request frame to HDD. It also call lim_process_probe_req_frame
 * function which process received Probe Request frame and responds
 * with Probe Response.
 *
 * @return None
 */
void
lim_process_probe_req_frame_multiple_bss(tpAniSirGlobal mac_ctx,
			uint8_t *buf_descr, tpPESession session)
{
	uint8_t i;

	if (session != NULL) {
		if (LIM_IS_AP_ROLE(session)) {
			lim_indicate_probe_req_to_hdd(mac_ctx,
					buf_descr, session);
		}
		lim_process_probe_req_frame(mac_ctx, buf_descr, session);
		return;
	}

	for (i = 0; i < mac_ctx->lim.maxBssId; i++) {
		session = pe_find_session_by_session_id(mac_ctx, i);
		if (session == NULL)
			continue;
		if (LIM_IS_AP_ROLE(session))
			lim_indicate_probe_req_to_hdd(mac_ctx,
					buf_descr, session);
		if (LIM_IS_AP_ROLE(session) ||
			LIM_IS_IBSS_ROLE(session))
			lim_process_probe_req_frame(mac_ctx,
					buf_descr, session);
	}
}

/**
 * lim_send_sme_probe_req_ind()
 *
 ***FUNCTION:
 * This function is to send
 *  eWNI_SME_WPS_PBC_PROBE_REQ_IND message to host
 *
 ***PARAMS:
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * This function is used for sending  eWNI_SME_WPS_PBC_PROBE_REQ_IND
 * to host.
 *
 * @param peerMacAddr       Indicates the peer MAC addr that the probe request
 *                          is generated.
 * @param pProbeReqIE       pointer to RAW probe request IE
 * @param ProbeReqIELen     The length of probe request IE.
 * @param psessionEntry     A pointer to PE session
 *
 * @return None
 */
void
lim_send_sme_probe_req_ind(tpAniSirGlobal pMac,
			   tSirMacAddr peerMacAddr,
			   uint8_t *pProbeReqIE,
			   uint32_t ProbeReqIELen, tpPESession psessionEntry)
{
	tSirSmeProbeReqInd *pSirSmeProbeReqInd;
	tSirMsgQ msgQ;

	pSirSmeProbeReqInd = qdf_mem_malloc(sizeof(tSirSmeProbeReqInd));
	if (NULL == pSirSmeProbeReqInd) {
		/* Log error */
		lim_log(pMac, LOGP,
			FL
				("call to AllocateMemory failed for eWNI_SME_PROBE_REQ_IND"));
		return;
	}

	msgQ.type = eWNI_SME_WPS_PBC_PROBE_REQ_IND;
	msgQ.bodyval = 0;
	msgQ.bodyptr = pSirSmeProbeReqInd;

	pSirSmeProbeReqInd->messageType = eWNI_SME_WPS_PBC_PROBE_REQ_IND;
	pSirSmeProbeReqInd->length = sizeof(tSirSmeProbeReq);
	pSirSmeProbeReqInd->sessionId = psessionEntry->smeSessionId;

	qdf_mem_copy(pSirSmeProbeReqInd->bssid.bytes, psessionEntry->bssId,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(pSirSmeProbeReqInd->WPSPBCProbeReq.peer_macaddr.bytes,
		     peerMacAddr, QDF_MAC_ADDR_SIZE);

	MTRACE(mac_trace(pMac, TRACE_CODE_TX_SME_MSG,
				psessionEntry->peSessionId, msgQ.type));
	pSirSmeProbeReqInd->WPSPBCProbeReq.probeReqIELen =
		(uint16_t) ProbeReqIELen;
	qdf_mem_copy(pSirSmeProbeReqInd->WPSPBCProbeReq.probeReqIE, pProbeReqIE,
		     ProbeReqIELen);

	if (lim_sys_process_mmh_msg_api(pMac, &msgQ, ePROT) != eSIR_SUCCESS) {
		PELOGE(lim_log
			       (pMac, LOGE, FL("couldnt send the probe req to hdd"));
		       )
	}

} /*** end lim_send_sme_probe_req_ind() ***/
