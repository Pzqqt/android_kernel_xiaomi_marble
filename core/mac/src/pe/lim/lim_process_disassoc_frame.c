/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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
 * This file lim_process_disassoc_frame.cc contains the code
 * for processing Disassocation Frame.
 * Author:        Chandra Modumudi
 * Date:          03/24/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#include "cds_api.h"
#include "wni_api.h"
#include "sir_api.h"
#include "ani_global.h"
#include "wni_cfg.h"

#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_security_utils.h"
#include "lim_ser_des_utils.h"
#include "lim_send_messages.h"
#include "sch_api.h"

/**
 * lim_process_disassoc_frame
 *
 ***FUNCTION:
 * This function is called by limProcessMessageQueue() upon
 * Disassociation frame reception.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * DPH drops packets for STA with 'valid' bit in pStaDs set to '0'.
 *
 ***NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  *pRxPacketInfo - A pointer to Rx packet info structure
 * @return None
 */
void
lim_process_disassoc_frame(tpAniSirGlobal pMac, uint8_t *pRxPacketInfo,
			   tpPESession psessionEntry)
{
	uint8_t *pBody;
	uint16_t aid, reasonCode;
	tpSirMacMgmtHdr pHdr;
	tpDphHashNode pStaDs;
	uint32_t frame_len;
	int32_t frame_rssi;

	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);
	frame_len = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

	frame_rssi = (int32_t)WMA_GET_RX_RSSI_NORMALIZED(pRxPacketInfo);

	if (lim_is_group_addr(pHdr->sa)) {
		/* Received Disassoc frame from a BC/MC address */
		/* Log error and ignore it */
		pe_err("received Disassoc frame from a BC/MC address");
		return;
	}

	if (lim_is_group_addr(pHdr->da) && !lim_is_addr_bc(pHdr->da)) {
		/* Received Disassoc frame for a MC address */
		/* Log error and ignore it */
		pe_err("received Disassoc frame for a MC address");
		return;
	}
	if (!lim_validate_received_frame_a1_addr(pMac,
			pHdr->da, psessionEntry)) {
		pe_err("rx frame doesn't have valid a1 address, drop it");
		return;
	}

	if (LIM_IS_STA_ROLE(psessionEntry) &&
		((eLIM_SME_WT_DISASSOC_STATE == psessionEntry->limSmeState) ||
		(eLIM_SME_WT_DEAUTH_STATE == psessionEntry->limSmeState))) {
		if (!(pMac->lim.disassocMsgCnt & 0xF)) {
			pe_debug("received Disassoc frame in %s"
				"already processing previously received Disassoc frame, dropping this %d",
				 lim_sme_state_str(psessionEntry->limSmeState),
				 ++pMac->lim.disassocMsgCnt);
		} else {
			pMac->lim.disassocMsgCnt++;
		}
		return;
	}
#ifdef WLAN_FEATURE_11W
	/* PMF: If this session is a PMF session, then ensure that this frame was protected */
	if (psessionEntry->limRmfEnabled
	    && (WMA_GET_RX_DPU_FEEDBACK(pRxPacketInfo) &
		DPU_FEEDBACK_UNPROTECTED_ERROR)) {
		pe_err("received an unprotected disassoc from AP");
		/*
		 * When 11w offload is enabled then
		 * firmware should not fwd this frame
		 */
		if (LIM_IS_STA_ROLE(psessionEntry) &&  pMac->pmf_offload) {
			pe_err("11w offload is enable,unprotected disassoc is not expected");
			return;
		}

		/* If the frame received is unprotected, forward it to the supplicant to initiate */
		/* an SA query */
		/* send the unprotected frame indication to SME */
		lim_send_sme_unprotected_mgmt_frame_ind(pMac, pHdr->fc.subType,
							(uint8_t *) pHdr,
							(frame_len +
							 sizeof(tSirMacMgmtHdr)),
							psessionEntry->smeSessionId,
							psessionEntry);
		return;
	}
#endif

	if (frame_len < 2) {
		pe_err("frame len less than 2");
		return;
	}

	/* Get reasonCode from Disassociation frame body */
	reasonCode = sir_read_u16(pBody);

	pe_debug("Received Disassoc frame for Addr: " MAC_ADDRESS_STR
		 "(mlm state=%s, sme state=%d RSSI=%d),"
		 "with reason code %d [%s] from " MAC_ADDRESS_STR,
		 MAC_ADDR_ARRAY(pHdr->da),
		 lim_mlm_state_str(psessionEntry->limMlmState),
		 psessionEntry->limSmeState, frame_rssi, reasonCode,
		 lim_dot11_reason_str(reasonCode), MAC_ADDR_ARRAY(pHdr->sa));
	lim_diag_event_report(pMac, WLAN_PE_DIAG_DISASSOC_FRAME_EVENT,
		psessionEntry, 0, reasonCode);

	if (pMac->roam.configParam.enable_fatal_event &&
		(reasonCode != eSIR_MAC_UNSPEC_FAILURE_REASON &&
		reasonCode != eSIR_MAC_DEAUTH_LEAVING_BSS_REASON &&
		reasonCode != eSIR_MAC_DISASSOC_LEAVING_BSS_REASON)) {
		cds_flush_logs(WLAN_LOG_TYPE_FATAL,
				WLAN_LOG_INDICATOR_HOST_DRIVER,
				WLAN_LOG_REASON_DISCONNECT,
				false, false);
	}
	/**
	 * Extract 'associated' context for STA, if any.
	 * This is maintained by DPH and created by LIM.
	 */
	pStaDs =
		dph_lookup_hash_entry(pMac, pHdr->sa, &aid,
				      &psessionEntry->dph.dphHashTable);

	if (pStaDs == NULL) {
		/**
		 * Disassociating STA is not associated.
		 * Log error.
		 */
		pe_err("received Disassoc frame from STA that does not have context"
			"reasonCode=%d, addr " MAC_ADDRESS_STR,
			reasonCode, MAC_ADDR_ARRAY(pHdr->sa));
		return;
	}

	if (lim_check_disassoc_deauth_ack_pending(pMac, (uint8_t *) pHdr->sa)) {
		pe_err("Ignore the DisAssoc received, while waiting for ack of disassoc/deauth");
		lim_clean_up_disassoc_deauth_req(pMac, (uint8_t *) pHdr->sa, 1);
		return;
	}

	if (pMac->lim.disassocMsgCnt != 0) {
		pMac->lim.disassocMsgCnt = 0;
	}

	/** If we are in the Wait for ReAssoc Rsp state */
	if (lim_is_reassoc_in_progress(pMac, psessionEntry)) {
		/*
		 * For LFR3, the roaming bssid is not known during ROAM_START,
		 * so check if the disassoc is received from current AP when
		 * roaming is being done in the firmware
		 */
		if (psessionEntry->fw_roaming_started &&
		    IS_CURRENT_BSSID(pMac, pHdr->sa, psessionEntry)) {
			pe_debug("Dropping disassoc frame from connected AP");
			psessionEntry->recvd_disassoc_while_roaming = true;
			psessionEntry->deauth_disassoc_rc = reasonCode;
			return;
		}
		/** If we had received the DisAssoc from,
		 *     a. the Current AP during ReAssociate to different AP in same ESS
		 *     b. Unknown AP
		 *   drop/ignore the DisAssoc received
		 */
		if (!IS_REASSOC_BSSID(pMac, pHdr->sa, psessionEntry)) {
			pe_err("Ignore DisAssoc while Processing ReAssoc");
			return;
		}
		/** If the Disassoc is received from the new AP to which we tried to ReAssociate
		 *  Drop ReAssoc and Restore the Previous context( current connected AP).
		 */
		if (!IS_CURRENT_BSSID(pMac, pHdr->sa, psessionEntry)) {
			pe_debug("received Disassoc from the New AP to which ReAssoc is sent");
			lim_restore_pre_reassoc_state(pMac,
						      eSIR_SME_REASSOC_REFUSED,
						      reasonCode,
						      psessionEntry);
			return;
		}
	}

	if (LIM_IS_AP_ROLE(psessionEntry)) {
		switch (reasonCode) {
		case eSIR_MAC_UNSPEC_FAILURE_REASON:
		case eSIR_MAC_DISASSOC_DUE_TO_INACTIVITY_REASON:
		case eSIR_MAC_DISASSOC_LEAVING_BSS_REASON:
		case eSIR_MAC_MIC_FAILURE_REASON:
		case eSIR_MAC_4WAY_HANDSHAKE_TIMEOUT_REASON:
		case eSIR_MAC_GR_KEY_UPDATE_TIMEOUT_REASON:
		case eSIR_MAC_RSN_IE_MISMATCH_REASON:
		case eSIR_MAC_1X_AUTH_FAILURE_REASON:
			/* Valid reasonCode in received Disassociation frame */
			break;

		default:
			/* Invalid reasonCode in received Disassociation frame */
			pe_warn("received Disassoc frame with invalid reasonCode: %d from " MAC_ADDRESS_STR,
				reasonCode, MAC_ADDR_ARRAY(pHdr->sa));
			break;
		}
	} else if (LIM_IS_STA_ROLE(psessionEntry) &&
		   ((psessionEntry->limSmeState != eLIM_SME_WT_JOIN_STATE) &&
		   (psessionEntry->limSmeState != eLIM_SME_WT_AUTH_STATE) &&
		   (psessionEntry->limSmeState != eLIM_SME_WT_ASSOC_STATE) &&
		   (psessionEntry->limSmeState != eLIM_SME_WT_REASSOC_STATE))) {
		switch (reasonCode) {
		case eSIR_MAC_DEAUTH_LEAVING_BSS_REASON:
		case eSIR_MAC_DISASSOC_LEAVING_BSS_REASON:
			/* Valid reasonCode in received Disassociation frame */
			/* as long as we're not about to channel switch */
			if (psessionEntry->gLimChannelSwitch.state !=
			    eLIM_CHANNEL_SWITCH_IDLE) {
				pe_err("Ignoring disassoc frame due to upcoming channel switch, from "MAC_ADDRESS_STR,
					MAC_ADDR_ARRAY(pHdr->sa));
				return;
			}
			break;

		default:
			break;
		}
	} else {
		/* Received Disassociation frame in either IBSS */
		/* or un-known role. Log and ignore it */
		pe_err("received Disassoc frame with invalid reasonCode: %d in role:"
				"%d in sme state: %d from " MAC_ADDRESS_STR, reasonCode,
			GET_LIM_SYSTEM_ROLE(psessionEntry), psessionEntry->limSmeState,
			MAC_ADDR_ARRAY(pHdr->sa));

		return;
	}

	if ((pStaDs->mlmStaContext.mlmState == eLIM_MLM_WT_DEL_STA_RSP_STATE) ||
	    (pStaDs->mlmStaContext.mlmState == eLIM_MLM_WT_DEL_BSS_RSP_STATE) ||
	    pStaDs->sta_deletion_in_progress) {
		/**
		 * Already in the process of deleting context for the peer
		 * and received Disassociation frame. Log and Ignore.
		 */
		pe_debug("Deletion is in progress (%d) for peer:%pM in mlmState %d",
			 pStaDs->sta_deletion_in_progress, pHdr->sa,
			 pStaDs->mlmStaContext.mlmState);
		return;
	}
	pStaDs->sta_deletion_in_progress = true;
	lim_disassoc_tdls_peers(pMac, psessionEntry, pHdr->sa);
	if (pStaDs->mlmStaContext.mlmState != eLIM_MLM_LINK_ESTABLISHED_STATE) {
		/**
		 * Requesting STA is in some 'transient' state?
		 * Log error.
		 */
		if (pStaDs->mlmStaContext.mlmState ==
		    eLIM_MLM_WT_ASSOC_CNF_STATE)
			pStaDs->mlmStaContext.updateContext = 1;

		pe_err("received Disassoc frame from peer that is in state: %X, addr "MAC_ADDRESS_STR,
			pStaDs->mlmStaContext.mlmState,
			       MAC_ADDR_ARRAY(pHdr->sa));

	} /* if (pStaDs->mlmStaContext.mlmState != eLIM_MLM_LINK_ESTABLISHED_STATE) */

	lim_perform_disassoc(pMac, frame_rssi, reasonCode,
			     psessionEntry, pHdr->sa);

} /*** end lim_process_disassoc_frame() ***/

#ifdef FEATURE_WLAN_TDLS
void lim_disassoc_tdls_peers(tpAniSirGlobal mac_ctx,
				    tpPESession pe_session, tSirMacAddr addr)
{
	tpDphHashNode sta_ds;
	uint16_t aid;

	sta_ds = dph_lookup_hash_entry(mac_ctx, addr, &aid,
				       &pe_session->dph.dphHashTable);
	if (sta_ds == NULL) {
		pe_debug("Hash entry not found");
		return;
	}
	/**
	 *  Delete all the TDLS peers only if Disassoc is received
	 *  from the AP
	 */
	if ((LIM_IS_STA_ROLE(pe_session)) &&
	    ((sta_ds->mlmStaContext.mlmState ==
	      eLIM_MLM_LINK_ESTABLISHED_STATE) ||
	     (sta_ds->mlmStaContext.mlmState ==
	      eLIM_MLM_IDLE_STATE)) &&
	    (IS_CURRENT_BSSID(mac_ctx, addr, pe_session)))
		lim_delete_tdls_peers(mac_ctx, pe_session);
}
#endif

void lim_perform_disassoc(tpAniSirGlobal mac_ctx, int32_t frame_rssi,
			  uint16_t rc, tpPESession pe_session, tSirMacAddr addr)
{
	tLimMlmDisassocInd mlmDisassocInd;
	uint16_t aid;
	tpDphHashNode sta_ds;

	sta_ds = dph_lookup_hash_entry(mac_ctx, addr, &aid,
				       &pe_session->dph.dphHashTable);
	if (sta_ds == NULL) {
		pe_debug("Hash entry not found");
		return;
	}
	sta_ds->mlmStaContext.cleanupTrigger = eLIM_PEER_ENTITY_DISASSOC;
	sta_ds->mlmStaContext.disassocReason = (tSirMacReasonCodes) rc;

	/* Issue Disassoc Indication to SME. */
	qdf_mem_copy((uint8_t *) &mlmDisassocInd.peerMacAddr,
			(uint8_t *) sta_ds->staAddr, sizeof(tSirMacAddr));
	mlmDisassocInd.reasonCode =
		(uint8_t) sta_ds->mlmStaContext.disassocReason;
	mlmDisassocInd.disassocTrigger = eLIM_PEER_ENTITY_DISASSOC;

	/* Update PE session Id  */
	mlmDisassocInd.sessionId = pe_session->peSessionId;

	if (lim_is_reassoc_in_progress(mac_ctx, pe_session)) {

		/* If we're in the middle of ReAssoc and received disassoc from
		 * the ReAssoc AP, then notify SME by sending REASSOC_RSP with
		 * failure result code. By design, SME will then issue "Disassoc"
		 * and cleanup will happen at that time.
		 */
		pe_debug("received Disassoc from AP while waiting for Reassoc Rsp");

		if (pe_session->limAssocResponseData) {
			qdf_mem_free(pe_session->limAssocResponseData);
			pe_session->limAssocResponseData = NULL;
		}

		lim_restore_pre_reassoc_state(mac_ctx, eSIR_SME_REASSOC_REFUSED,
				rc, pe_session);
		return;
	}

	lim_update_lost_link_info(mac_ctx, pe_session, frame_rssi);
	lim_post_sme_message(mac_ctx, LIM_MLM_DISASSOC_IND,
			(uint32_t *) &mlmDisassocInd);

	/* send eWNI_SME_DISASSOC_IND to SME */
	lim_send_sme_disassoc_ind(mac_ctx, sta_ds, pe_session);

	return;
}
