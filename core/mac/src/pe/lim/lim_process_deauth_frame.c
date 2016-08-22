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
 * This file lim_process_deauth_frame.cc contains the code
 * for processing Deauthentication Frame.
 * Author:        Chandra Modumudi
 * Date:          03/24/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#include "cds_api.h"
#include "ani_global.h"

#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_security_utils.h"
#include "lim_ser_des_utils.h"
#include "sch_api.h"
#include "lim_send_messages.h"

/**
 * lim_process_deauth_frame
 *
 ***FUNCTION:
 * This function is called by limProcessMessageQueue() upon
 * Deauthentication frame reception.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  *pRxPacketInfo - A pointer to Buffer descriptor + associated PDUs
 * @return None
 */

void
lim_process_deauth_frame(tpAniSirGlobal pMac, uint8_t *pRxPacketInfo,
			 tpPESession psessionEntry)
{
	uint8_t *pBody;
	uint16_t aid, reasonCode;
	tpSirMacMgmtHdr pHdr;
	tLimMlmAssocCnf mlmAssocCnf;
	tLimMlmDeauthInd mlmDeauthInd;
	tpDphHashNode pStaDs;
	tpPESession pRoamSessionEntry = NULL;
	uint8_t roamSessionId;
#ifdef WLAN_FEATURE_11W
	uint32_t frameLen;
#endif

	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);

	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);

	if (LIM_IS_STA_ROLE(psessionEntry) &&
	    ((eLIM_SME_WT_DISASSOC_STATE == psessionEntry->limSmeState) ||
	     (eLIM_SME_WT_DEAUTH_STATE == psessionEntry->limSmeState))) {
		/*Every 15th deauth frame will be logged in kmsg */
		if (!(pMac->lim.deauthMsgCnt & 0xF)) {
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("received Deauth frame in DEAUTH_WT_STATE"
					       "(already processing previously received DEAUTH frame).."
					       "Dropping this.. Deauth Failed %d"),
				       ++pMac->lim.deauthMsgCnt);
			       )
		} else {
			pMac->lim.deauthMsgCnt++;
		}
		return;
	}

	if (lim_is_group_addr(pHdr->sa)) {
		/* Received Deauth frame from a BC/MC address */
		/* Log error and ignore it */
		PELOGE(lim_log(pMac, LOGE,
			       FL("received Deauth frame from a BC/MC address"));
		       )

		return;
	}

	if (lim_is_group_addr(pHdr->da) && !lim_is_addr_bc(pHdr->da)) {
		/* Received Deauth frame for a MC address */
		/* Log error and ignore it */
		PELOGE(lim_log(pMac, LOGE,
			       FL("received Deauth frame for a MC address"));
		       )

		return;
	}
	if (!lim_validate_received_frame_a1_addr(pMac,
			pHdr->da, psessionEntry)) {
		lim_log(pMac, LOGE,
			FL("rx frame doesn't have valid a1 address, drop it"));
		return;
	}
#ifdef WLAN_FEATURE_11W
	/* PMF: If this session is a PMF session, then ensure that this frame was protected */
	if (psessionEntry->limRmfEnabled
	    && (WMA_GET_RX_DPU_FEEDBACK(pRxPacketInfo) &
		DPU_FEEDBACK_UNPROTECTED_ERROR)) {
		PELOGE(lim_log
			       (pMac, LOGE,
			       FL("received an unprotected deauth from AP"));
		       )

		/*
		 * When 11w offload is enabled then
		 * firmware should not fwd this frame
		 */
		if (LIM_IS_STA_ROLE(psessionEntry) && pMac->pmf_offload) {
			lim_log(pMac, LOGE,
				FL("11w offload is enable,unprotected deauth is not expected")
				);
			return;
		}

		/* If the frame received is unprotected, forward it to the supplicant to initiate */
		/* an SA query */
		frameLen = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

		/* send the unprotected frame indication to SME */
		lim_send_sme_unprotected_mgmt_frame_ind(pMac, pHdr->fc.subType,
							(uint8_t *) pHdr,
							(frameLen +
							 sizeof(tSirMacMgmtHdr)),
							psessionEntry->smeSessionId,
							psessionEntry);
		return;
	}
#endif

	/* Get reasonCode from Deauthentication frame body */
	reasonCode = sir_read_u16(pBody);

	PELOGE(lim_log(pMac, LOGE,
		       FL("Received Deauth frame for Addr: " MAC_ADDRESS_STR
			" (mlm state = %s,"
			" sme state = %d systemrole  = %d) with reason code %d [%s] from "
			MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->da),
			lim_mlm_state_str(psessionEntry->limMlmState),
			psessionEntry->limSmeState,
			GET_LIM_SYSTEM_ROLE(psessionEntry),
			reasonCode, lim_dot11_reason_str(reasonCode),
			MAC_ADDR_ARRAY(pHdr->sa));
	       )

	if (lim_check_disassoc_deauth_ack_pending(pMac, (uint8_t *) pHdr->sa)) {
		PELOGE(lim_log(pMac, LOGE,
			       FL
				       ("Ignore the Deauth received, while waiting for ack of "
				       "disassoc/deauth"));
		       )
		lim_clean_up_disassoc_deauth_req(pMac, (uint8_t *) pHdr->sa, 1);
		return;
	}

	if (LIM_IS_AP_ROLE(psessionEntry)) {
		switch (reasonCode) {
		case eSIR_MAC_UNSPEC_FAILURE_REASON:
		case eSIR_MAC_DEAUTH_LEAVING_BSS_REASON:
			/* Valid reasonCode in received Deauthentication frame */
			break;

		default:
			/* Invalid reasonCode in received Deauthentication frame */
			/* Log error and ignore the frame */
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("received Deauth frame with invalid reasonCode %d from "
					       MAC_ADDRESS_STR), reasonCode,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			break;
		}
	} else if (LIM_IS_STA_ROLE(psessionEntry)) {
		switch (reasonCode) {
		case eSIR_MAC_UNSPEC_FAILURE_REASON:
		case eSIR_MAC_PREV_AUTH_NOT_VALID_REASON:
		case eSIR_MAC_DEAUTH_LEAVING_BSS_REASON:
		case eSIR_MAC_CLASS2_FRAME_FROM_NON_AUTH_STA_REASON:
		case eSIR_MAC_CLASS3_FRAME_FROM_NON_ASSOC_STA_REASON:
		case eSIR_MAC_STA_NOT_PRE_AUTHENTICATED_REASON:
			/* Valid reasonCode in received Deauth frame */
			break;

		default:
			/* Invalid reasonCode in received Deauth frame */
			/* Log error and ignore the frame */
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("received Deauth frame with invalid reasonCode %d from "
					       MAC_ADDRESS_STR), reasonCode,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			break;
		}
	} else {
		/* Received Deauth frame in either IBSS */
		/* or un-known role. Log and ignore it */
		lim_log(pMac, LOGE,
			FL
			("received Deauth frame with reasonCode %d in role %d from "
			MAC_ADDRESS_STR), reasonCode,
			GET_LIM_SYSTEM_ROLE(psessionEntry),
			MAC_ADDR_ARRAY(pHdr->sa));

		return;
	}

	/** If we are in the middle of ReAssoc, a few things could happen:
	 *  - STA is reassociating to current AP, and receives deauth from:
	 *         a) current AP
	 *         b) other AP
	 *  - STA is reassociating to a new AP, and receives deauth from:
	 *         c) current AP
	 *         d) reassoc AP
	 *         e) other AP
	 *
	 *  The logic is:
	 *  1) If rcv deauth from an AP other than the one we're trying to
	 *     reassociate with, then drop the deauth frame (case b, c, e)
	 *  2) If rcv deauth from the "new" reassoc AP (case d), then restore
	 *     context with previous AP and send SME_REASSOC_RSP failure.
	 *  3) If rcv deauth from the reassoc AP, which is also the same
	 *     AP we're currently associated with (case a), then proceed
	 *     with normal deauth processing.
	 */
	if (psessionEntry->limReAssocbssId != NULL) {
		pRoamSessionEntry =
			pe_find_session_by_bssid(pMac, psessionEntry->limReAssocbssId,
						 &roamSessionId);
	}
	if (lim_is_reassoc_in_progress(pMac, psessionEntry)
	    || lim_is_reassoc_in_progress(pMac, pRoamSessionEntry)) {
		if (!IS_REASSOC_BSSID(pMac, pHdr->sa, psessionEntry)) {
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("Rcv Deauth from unknown/different "
					  "AP while ReAssoc. Ignore " MAC_ADDRESS_STR),
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL(" limReAssocbssId : " MAC_ADDRESS_STR),
				       MAC_ADDR_ARRAY(psessionEntry->
						      limReAssocbssId));
			       )
			return;
		}

		/** Received deauth from the new AP to which we tried to ReAssociate.
		 *  Drop ReAssoc and Restore the Previous context( current connected AP).
		 */
		if (!IS_CURRENT_BSSID(pMac, pHdr->sa, psessionEntry)) {
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("received DeAuth from the New AP to "
					  "which ReAssoc is sent " MAC_ADDRESS_STR),
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL(" psessionEntry->bssId: "
					  MAC_ADDRESS_STR),
				       MAC_ADDR_ARRAY(psessionEntry->bssId));
			       )
			lim_restore_pre_reassoc_state(pMac,
						      eSIR_SME_REASSOC_REFUSED,
						      reasonCode,
						      psessionEntry);
			return;
		}
	}

	/* If received DeAuth from AP other than the one we're trying to join with
	 * nor associated with, then ignore deauth and delete Pre-auth entry.
	 */
	if (!LIM_IS_AP_ROLE(psessionEntry)) {
		if (!IS_CURRENT_BSSID(pMac, pHdr->bssId, psessionEntry)) {
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("received DeAuth from an AP other "
					  "than we're trying to join. Ignore. "
					  MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));
			       )
			if (lim_search_pre_auth_list(pMac, pHdr->sa)) {
				PELOG1(lim_log
					       (pMac, LOG1,
					       FL("Preauth entry exist. "
						  "Deleting... "));
				       )
				lim_delete_pre_auth_node(pMac, pHdr->sa);
			}
			return;
		}
	}

	pStaDs =
		dph_lookup_hash_entry(pMac, pHdr->sa, &aid,
				      &psessionEntry->dph.dphHashTable);

	/* Check for pre-assoc states */
	switch (GET_LIM_SYSTEM_ROLE(psessionEntry)) {
	case eLIM_STA_ROLE:
		switch (psessionEntry->limMlmState) {
		case eLIM_MLM_WT_AUTH_FRAME2_STATE:
			/**
			 * AP sent Deauth frame while waiting
			 * for Auth frame2. Report Auth failure
			 * to SME.
			 */

			/* Log error */
			PELOG1(lim_log(pMac, LOG1,
				       FL
					       ("received Deauth frame state %X with failure "
					       "code %d from " MAC_ADDRESS_STR),
				       psessionEntry->limMlmState, reasonCode,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			lim_restore_from_auth_state(pMac,
						    eSIR_SME_DEAUTH_WHILE_JOIN,
						    reasonCode, psessionEntry);

			return;

		case eLIM_MLM_AUTHENTICATED_STATE:
			lim_log(pMac, LOG1,
				FL("received Deauth frame state %X with "
				   "reasonCode=%d from " MAC_ADDRESS_STR),
				psessionEntry->limMlmState, reasonCode,
				MAC_ADDR_ARRAY(pHdr->sa));
			/* / Issue Deauth Indication to SME. */
			qdf_mem_copy((uint8_t *) &mlmDeauthInd.peerMacAddr,
				     pHdr->sa, sizeof(tSirMacAddr));
			mlmDeauthInd.reasonCode = reasonCode;

			psessionEntry->limMlmState = eLIM_MLM_IDLE_STATE;
			MTRACE(mac_trace
				       (pMac, TRACE_CODE_MLM_STATE,
				       psessionEntry->peSessionId,
				       psessionEntry->limMlmState));

			lim_post_sme_message(pMac,
					     LIM_MLM_DEAUTH_IND,
					     (uint32_t *) &mlmDeauthInd);
			return;

		case eLIM_MLM_WT_ASSOC_RSP_STATE:
			/**
			 * AP may have 'aged-out' our Pre-auth
			 * context. Delete local pre-auth context
			 * if any and issue ASSOC_CNF to SME.
			 */
			lim_log(pMac, LOG1,
				FL("received Deauth frame state %X with "
				   "reasonCode=%d from " MAC_ADDRESS_STR),
				psessionEntry->limMlmState, reasonCode,
				MAC_ADDR_ARRAY(pHdr->sa));
			if (lim_search_pre_auth_list(pMac, pHdr->sa))
				lim_delete_pre_auth_node(pMac, pHdr->sa);

			if (psessionEntry->pLimMlmJoinReq) {
				qdf_mem_free(psessionEntry->pLimMlmJoinReq);
				psessionEntry->pLimMlmJoinReq = NULL;
			}

			mlmAssocCnf.resultCode = eSIR_SME_DEAUTH_WHILE_JOIN;
			mlmAssocCnf.protStatusCode = reasonCode;

			/* PE session Id */
			mlmAssocCnf.sessionId = psessionEntry->peSessionId;

			psessionEntry->limMlmState =
				psessionEntry->limPrevMlmState;
			MTRACE(mac_trace
				       (pMac, TRACE_CODE_MLM_STATE,
				       psessionEntry->peSessionId,
				       psessionEntry->limMlmState));

			/* Deactive Association response timeout */
			lim_deactivate_and_change_timer(pMac,
							eLIM_ASSOC_FAIL_TIMER);

			lim_post_sme_message(pMac,
					     LIM_MLM_ASSOC_CNF,
					     (uint32_t *) &mlmAssocCnf);

			return;

		case eLIM_MLM_WT_ADD_STA_RSP_STATE:
			psessionEntry->fDeauthReceived = true;
			PELOGW(lim_log(pMac, LOGW,
				       FL
					       ("Received Deauth frame in state %X with Reason "
					       "Code %d from Peer" MAC_ADDRESS_STR),
				       psessionEntry->limMlmState, reasonCode,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )
			return;

		case eLIM_MLM_IDLE_STATE:
		case eLIM_MLM_LINK_ESTABLISHED_STATE:
#ifdef FEATURE_WLAN_TDLS
			if ((NULL != pStaDs)
			    && (STA_ENTRY_TDLS_PEER == pStaDs->staType)) {
				PELOGE(lim_log
					       (pMac, LOGE,
					       FL
						       ("received Deauth frame in state %X with "
						       "reason code %d from Tdls peer"
						       MAC_ADDRESS_STR),
					       psessionEntry->limMlmState, reasonCode,
					       MAC_ADDR_ARRAY(pHdr->sa));
				       )
				lim_send_sme_tdls_del_sta_ind(pMac, pStaDs,
							      psessionEntry,
							      reasonCode);
				return;
			} else {

				/*
				 * Delete all the TDLS peers only if Deauth
				 * is received from the AP
				 */
				if (IS_CURRENT_BSSID(pMac, pHdr->sa, psessionEntry))
					lim_delete_tdls_peers(pMac, psessionEntry);
#endif
			/**
			 * This could be Deauthentication frame from
			 * a BSS with which pre-authentication was
			 * performed. Delete Pre-auth entry if found.
			 */
			if (lim_search_pre_auth_list(pMac, pHdr->sa))
				lim_delete_pre_auth_node(pMac, pHdr->sa);
#ifdef FEATURE_WLAN_TDLS
		}
#endif
			break;

		case eLIM_MLM_WT_REASSOC_RSP_STATE:
			lim_log(pMac, LOGE,
				FL("received Deauth frame state %X with "
				   "reasonCode=%d from " MAC_ADDRESS_STR),
				psessionEntry->limMlmState, reasonCode,
				MAC_ADDR_ARRAY(pHdr->sa));
			break;

		case eLIM_MLM_WT_FT_REASSOC_RSP_STATE:
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("received Deauth frame in FT state %X with "
					       "reasonCode=%d from " MAC_ADDRESS_STR),
				       psessionEntry->limMlmState, reasonCode,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )
			break;

		default:
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("received Deauth frame in state %X with "
					       "reasonCode=%d from " MAC_ADDRESS_STR),
				       psessionEntry->limMlmState, reasonCode,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )
			return;
		}
		break;

	case eLIM_STA_IN_IBSS_ROLE:
		break;

	case eLIM_AP_ROLE:
		break;

	default:

		return;
	} /* end switch (pMac->lim.gLimSystemRole) */

	/**
	 * Extract 'associated' context for STA, if any.
	 * This is maintained by DPH and created by LIM.
	 */
	if (NULL == pStaDs) {
		lim_log(pMac, LOGE, FL("pStaDs is NULL"));
		return;
	}

	if ((pStaDs->mlmStaContext.mlmState == eLIM_MLM_WT_DEL_STA_RSP_STATE) ||
	    (pStaDs->mlmStaContext.mlmState == eLIM_MLM_WT_DEL_BSS_RSP_STATE)) {
		/**
		 * Already in the process of deleting context for the peer
		 * and received Deauthentication frame. Log and Ignore.
		 */
		PELOGE(lim_log(pMac, LOGE,
			       FL
				       ("received Deauth frame from peer that is in state %X, addr "
				       MAC_ADDRESS_STR), pStaDs->mlmStaContext.mlmState,
			       MAC_ADDR_ARRAY(pHdr->sa));
		       )
		return;
	}
	pStaDs->mlmStaContext.disassocReason = (tSirMacReasonCodes) reasonCode;
	pStaDs->mlmStaContext.cleanupTrigger = eLIM_PEER_ENTITY_DEAUTH;

	/* / Issue Deauth Indication to SME. */
	qdf_mem_copy((uint8_t *) &mlmDeauthInd.peerMacAddr,
		     pStaDs->staAddr, sizeof(tSirMacAddr));
	mlmDeauthInd.reasonCode =
		(uint8_t) pStaDs->mlmStaContext.disassocReason;
	mlmDeauthInd.deauthTrigger = eLIM_PEER_ENTITY_DEAUTH;

	/*
	 * If we're in the middle of ReAssoc and received deauth from
	 * the ReAssoc AP, then notify SME by sending REASSOC_RSP with
	 * failure result code. SME will post the disconnect to the
	 * supplicant and the latter would start a fresh assoc.
	 */
	if (lim_is_reassoc_in_progress(pMac, psessionEntry)) {
		/**
		 * AP may have 'aged-out' our Pre-auth
		 * context. Delete local pre-auth context
		 * if any and issue REASSOC_CNF to SME.
		 */
		if (lim_search_pre_auth_list(pMac, pHdr->sa))
			lim_delete_pre_auth_node(pMac, pHdr->sa);

		if (psessionEntry->limAssocResponseData) {
			qdf_mem_free(psessionEntry->limAssocResponseData);
			psessionEntry->limAssocResponseData = NULL;
		}

		PELOGE(lim_log(pMac, LOGE, FL("Rcv Deauth from ReAssoc AP. "
					      "Issue REASSOC_CNF. "));
		       )
		/*
		 * TODO: Instead of overloading eSIR_SME_FT_REASSOC_TIMEOUT_FAILURE
		 * it would have been good to define/use a different failure type.
		 * Using eSIR_SME_FT_REASSOC_FAILURE does not seem to clean-up
		 * properly and we end up seeing "transmit queue timeout".
		 */
		lim_post_reassoc_failure(pMac,
					 eSIR_SME_FT_REASSOC_TIMEOUT_FAILURE,
					 eSIR_MAC_UNSPEC_FAILURE_STATUS,
					 psessionEntry);
		return;
	}
	/* reset the deauthMsgCnt here since we are able to Process
	* the deauth frame and sending up the indication as well */
	if (pMac->lim.deauthMsgCnt != 0) {
		pMac->lim.deauthMsgCnt = 0;
	}
	if (LIM_IS_STA_ROLE(psessionEntry))
		wma_tx_abort(psessionEntry->smeSessionId);

	/* / Deauthentication from peer MAC entity */
	if (LIM_IS_STA_ROLE(psessionEntry))
		lim_post_sme_message(pMac, LIM_MLM_DEAUTH_IND,
			     (uint32_t *) &mlmDeauthInd);

	/* send eWNI_SME_DEAUTH_IND to SME */
	lim_send_sme_deauth_ind(pMac, pStaDs, psessionEntry);
	return;

} /*** end lim_process_deauth_frame() ***/
