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
 * This file lim_process_auth_frame.cc contains the code
 * for processing received Authentication Frame.
 * Author:        Chandra Modumudi
 * Date:          03/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 * 05/12/2010     js             To support Shared key authentication at AP side
 *
 */

#include "wni_api.h"
#include "wni_cfg.h"
#include "ani_global.h"
#include "cfg_api.h"

#include "utils_api.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_security_utils.h"
#include "lim_ser_des_utils.h"
#ifdef WLAN_FEATURE_VOWIFI_11R
#include "lim_ft.h"
#endif
#include "cds_utils.h"

/**
 * is_auth_valid
 *
 ***FUNCTION:
 * This function is called by lim_process_auth_frame() upon Authentication
 * frame reception.
 *
 ***LOGIC:
 * This function is used to test validity of auth frame:
 * - AUTH1 and AUTH3 must be received in AP mode
 * - AUTH2 and AUTH4 must be received in STA mode
 * - AUTH3 and AUTH4 must have challenge text IE, that is,'type' field has been set to
 *                 SIR_MAC_CHALLENGE_TEXT_EID by parser
 * -
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  *auth - Pointer to extracted auth frame body
 *
 * @return 0 or 1 (Valid)
 */

static inline unsigned int is_auth_valid(tpAniSirGlobal pMac,
					 tpSirMacAuthFrameBody auth,
					 tpPESession sessionEntry)
{
	unsigned int valid = 1;

	if (((auth->authTransactionSeqNumber == SIR_MAC_AUTH_FRAME_1) ||
	    (auth->authTransactionSeqNumber == SIR_MAC_AUTH_FRAME_3)) &&
	    (LIM_IS_STA_ROLE(sessionEntry) ||
	    LIM_IS_BT_AMP_STA_ROLE(sessionEntry)))
		valid = 0;

	if (((auth->authTransactionSeqNumber == SIR_MAC_AUTH_FRAME_2) ||
	    (auth->authTransactionSeqNumber == SIR_MAC_AUTH_FRAME_4)) &&
	    (LIM_IS_AP_ROLE(sessionEntry) ||
	    LIM_IS_BT_AMP_AP_ROLE(sessionEntry)))
		valid = 0;

	if (((auth->authTransactionSeqNumber == SIR_MAC_AUTH_FRAME_3) ||
	    (auth->authTransactionSeqNumber == SIR_MAC_AUTH_FRAME_4)) &&
	    (auth->type != SIR_MAC_CHALLENGE_TEXT_EID) &&
	    (auth->authAlgoNumber != eSIR_SHARED_KEY))
		valid = 0;

	return valid;
}

/**
 * lim_process_auth_frame
 *
 ***FUNCTION:
 * This function is called by limProcessMessageQueue() upon Authentication
 * frame reception.
 *
 ***LOGIC:
 * This function processes received Authentication frame and responds
 * with either next Authentication frame in sequence to peer MAC entity
 * or LIM_MLM_AUTH_IND on AP or LIM_MLM_AUTH_CNF on STA.
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 * 1. Authentication failures are reported to SME with same status code
 *    received from the peer MAC entity.
 * 2. Authentication frame2/4 received with alogirthm number other than
 *    one requested in frame1/3 are logged with an error and auth confirm
 *    will be sent to SME only after auth failure timeout.
 * 3. Inconsistency in the spec:
 *    On receiving Auth frame2, specs says that if WEP key mapping key
 *    or default key is NULL, Auth frame3 with a status code 15 (challenge
 *    failure to be returned to peer entity. However, section 7.2.3.10,
 *    table 14 says that status code field is 'reserved' for frame3 !
 *    In the current implementation, Auth frame3 is returned with status
 *    code 15 overriding section 7.2.3.10.
 * 4. If number pre-authentications reach configrable max limit,
 *    Authentication frame with 'unspecified failure' status code is
 *    returned to requesting entity.
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  *pRxPacketInfo - A pointer to Rx packet info structure
 * @return None
 */

void
lim_process_auth_frame(tpAniSirGlobal pMac, uint8_t *pRxPacketInfo,
		       tpPESession psessionEntry)
{
	uint8_t *pBody, keyId, cfgPrivacyOptImp,
		 defaultKey[SIR_MAC_KEY_LENGTH],
		 encrAuthFrame[LIM_ENCR_AUTH_BODY_LEN], plainBody[256];
	uint16_t frameLen;
	uint32_t maxNumPreAuth, val;
	tSirMacAuthFrameBody *pRxAuthFrameBody, rxAuthFrame, authFrame;
	tpSirMacMgmtHdr pHdr;
	struct tLimPreAuthNode *pAuthNode;
	uint8_t decryptResult;
	uint8_t *pChallenge;
	uint32_t key_length = 8;
	uint8_t challengeTextArray[SIR_MAC_AUTH_CHALLENGE_LENGTH];
	tpDphHashNode pStaDs = NULL;
	uint16_t assocId = 0;
	uint16_t curr_seq_num = 0;

	/* Get pointer to Authentication frame header and body */
	pHdr = WMA_GET_RX_MAC_HEADER(pRxPacketInfo);
	frameLen = WMA_GET_RX_PAYLOAD_LEN(pRxPacketInfo);

	if (!frameLen) {
		/* Log error */
		lim_log(pMac, LOGE,
			FL("received Authentication frame with no body from "));
		lim_print_mac_addr(pMac, pHdr->sa, LOGE);

		return;
	}

	if (lim_is_group_addr(pHdr->sa)) {
		/* Received Auth frame from a BC/MC address */
		/* Log error and ignore it */
		lim_log(pMac, LOGE, FL (
				"received Auth frame from a BC/MC addr - "));
		PELOGE(lim_print_mac_addr(pMac, pHdr->sa, LOGE);)

		return;
	}
	curr_seq_num = (pHdr->seqControl.seqNumHi << 4) |
			(pHdr->seqControl.seqNumLo);

	lim_log(pMac, LOG1,
		FL("Sessionid: %d System role : %d limMlmState: %d :Auth "
		   "Frame Received: BSSID: " MAC_ADDRESS_STR " (RSSI %d)"),
		psessionEntry->peSessionId, GET_LIM_SYSTEM_ROLE(psessionEntry),
		psessionEntry->limMlmState, MAC_ADDR_ARRAY(pHdr->bssId),
		(uint) abs((int8_t) WMA_GET_RX_RSSI_NORMALIZED(pRxPacketInfo)));

	pBody = WMA_GET_RX_MPDU_DATA(pRxPacketInfo);

	/* Restore default failure timeout */
	if (CDF_P2P_CLIENT_MODE == psessionEntry->pePersona
	    && psessionEntry->defaultAuthFailureTimeout) {
		lim_log(pMac, LOG1, FL("Restore default failure timeout"));
		cfg_set_int(pMac, WNI_CFG_AUTHENTICATE_FAILURE_TIMEOUT,
				psessionEntry->defaultAuthFailureTimeout);
	}
	/* / Determine if WEP bit is set in the FC or received MAC header */
	if (pHdr->fc.wep) {
		/**
		 * WEP bit is set in FC of MAC header.
		 */

		/* If TKIP counter measures enabled issue Deauth frame to station */
		if (psessionEntry->bTkipCntrMeasActive &&
		    LIM_IS_AP_ROLE(psessionEntry)) {
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL
					       ("Tkip counter measures Enabled, sending Deauth frame to"));
			       )
			lim_print_mac_addr(pMac, pHdr->sa, LOGE);

			lim_send_deauth_mgmt_frame(pMac,
						   eSIR_MAC_MIC_FAILURE_REASON,
						   pHdr->sa, psessionEntry, false);
			return;
		}
		/* Extract key ID from IV (most 2 bits of 4th byte of IV) */

		keyId = (*(pBody + 3)) >> 6;

		/**
		 * On STA in infrastructure BSS, Authentication frames received
		 * with WEP bit set in the FC must be rejected with challenge
		 * failure status code (wierd thing in the spec - this should have
		 * been rejected with unspecified failure or unexpected assertion
		 * of wep bit (this status code does not exist though) or
		 * Out-of-sequence-Authentication-Frame status code.
		 */

		if (LIM_IS_STA_ROLE(psessionEntry) ||
		    LIM_IS_BT_AMP_STA_ROLE(psessionEntry)) {
			authFrame.authAlgoNumber = eSIR_SHARED_KEY;
			authFrame.authTransactionSeqNumber =
				SIR_MAC_AUTH_FRAME_4;
			authFrame.authStatusCode =
				eSIR_MAC_CHALLENGE_FAILURE_STATUS;
			/* Log error */
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("received Authentication frame with wep bit set on role=%d "
					       MAC_ADDRESS_STR),
				       GET_LIM_SYSTEM_ROLE(psessionEntry),
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			lim_send_auth_mgmt_frame(pMac, &authFrame,
						 pHdr->sa,
						 LIM_NO_WEP_IN_FC,
						 psessionEntry);
			return;
		}

		if (frameLen < LIM_ENCR_AUTH_BODY_LEN) {
			/* Log error */
			lim_log(pMac, LOGE,
				FL
					("Not enough size [%d] to decrypt received Auth frame"),
				frameLen);
			lim_print_mac_addr(pMac, pHdr->sa, LOGE);

			return;
		}
		if (LIM_IS_AP_ROLE(psessionEntry)) {
			val = psessionEntry->privacy;
		} else
		/* Accept Authentication frame only if Privacy is implemented */
		if (wlan_cfg_get_int(pMac, WNI_CFG_PRIVACY_ENABLED,
				     &val) != eSIR_SUCCESS) {
			/**
			 * Could not get Privacy option
			 * from CFG. Log error.
			 */
			lim_log(pMac, LOGP,
				FL("could not retrieve Privacy option"));
		}

		cfgPrivacyOptImp = (uint8_t) val;
		if (cfgPrivacyOptImp) {
			/**
			 * Privacy option is implemented.
			 * Check if the received frame is Authentication
			 * frame3 and there is a context for requesting STA.
			 * If not, reject with unspecified failure status code
			 */
			pAuthNode = lim_search_pre_auth_list(pMac, pHdr->sa);

			if (pAuthNode == NULL) {
				/* Log error */
				PELOGE(lim_log(pMac, LOGE,
					       FL
						       ("received Authentication frame from peer that has no preauth context with WEP bit set "
						       MAC_ADDRESS_STR),
					       MAC_ADDR_ARRAY(pHdr->sa));
				       )

				/**
				 * No 'pre-auth' context exists for this STA that sent
				 * an Authentication frame with FC bit set.
				 * Send Auth frame4 with 'out of sequence' status code.
				 */
				authFrame.authAlgoNumber = eSIR_SHARED_KEY;
				authFrame.authTransactionSeqNumber =
					SIR_MAC_AUTH_FRAME_4;
				authFrame.authStatusCode =
					eSIR_MAC_AUTH_FRAME_OUT_OF_SEQ_STATUS;

				lim_send_auth_mgmt_frame(pMac, &authFrame,
							 pHdr->sa,
							 LIM_NO_WEP_IN_FC,
							 psessionEntry);
				return;
			} else {
				/* / Change the auth-response timeout */
				lim_deactivate_and_change_per_sta_id_timer(pMac,
									   eLIM_AUTH_RSP_TIMER,
									   pAuthNode->
									   authNodeIdx);

				/* / 'Pre-auth' status exists for STA */
				if ((pAuthNode->mlmState !=
				     eLIM_MLM_WT_AUTH_FRAME3_STATE) &&
				    (pAuthNode->mlmState !=
				     eLIM_MLM_AUTH_RSP_TIMEOUT_STATE)) {
					/* Log error */
					PELOGE(lim_log(pMac, LOGE,
					       FL
					       ("received Authentication frame from peer that is in state %d "
					       MAC_ADDRESS_STR),
					       pAuthNode->mlmState,
					       MAC_ADDR_ARRAY(pHdr->sa));)
					/**
					 * Should not have received Authentication frame
					 * with WEP bit set in FC in other states.
					 * Reject by sending Authenticaton frame with
					 * out of sequence Auth frame status code.
					 */
					authFrame.authAlgoNumber =
						eSIR_SHARED_KEY;
					authFrame.authTransactionSeqNumber =
						SIR_MAC_AUTH_FRAME_4;
					authFrame.authStatusCode =
						eSIR_MAC_AUTH_FRAME_OUT_OF_SEQ_STATUS;

					lim_send_auth_mgmt_frame(pMac, &authFrame,
								 pHdr->sa,
								 LIM_NO_WEP_IN_FC,
								 psessionEntry);
					return;
				}
			}

			val = SIR_MAC_KEY_LENGTH;

			if (LIM_IS_AP_ROLE(psessionEntry)) {
				tpSirKeys pKey;
				pKey =
					&psessionEntry->
					WEPKeyMaterial[keyId].key[0];
				cdf_mem_copy(defaultKey, pKey->key,
					     pKey->keyLength);
				val = pKey->keyLength;
			} else if (wlan_cfg_get_str(pMac,
					(uint16_t) (WNI_CFG_WEP_DEFAULT_KEY_1 +
					keyId), defaultKey,
				    &val) != eSIR_SUCCESS) {
				/* / Could not get Default key from CFG. */
				/* Log error. */
				lim_log(pMac, LOGP,
					FL
					("could not retrieve Default key"));

				/**
				 * Send Authentication frame
				 * with challenge failure status code
				 */

				authFrame.authAlgoNumber =
					eSIR_SHARED_KEY;
				authFrame.authTransactionSeqNumber =
					SIR_MAC_AUTH_FRAME_4;
				authFrame.authStatusCode =
					eSIR_MAC_CHALLENGE_FAILURE_STATUS;

				lim_send_auth_mgmt_frame(pMac, &authFrame,
							 pHdr->sa,
							 LIM_NO_WEP_IN_FC,
							 psessionEntry);

				return;
			}

			key_length = val;

			decryptResult = lim_decrypt_auth_frame(pMac, defaultKey,
						pBody, plainBody, key_length,
						(uint16_t) (frameLen -
						SIR_MAC_WEP_IV_LENGTH));
			if (decryptResult == LIM_DECRYPT_ICV_FAIL) {
				/* Log error */
				PELOGE(lim_log(pMac, LOGE,
					       FL
					       ("received Authentication frame from peer that failed decryption: "
					       MAC_ADDRESS_STR),
					       MAC_ADDR_ARRAY(pHdr->sa));
				       )
				/* / ICV failure */
				lim_delete_pre_auth_node(pMac,
							 pHdr->sa);
				authFrame.authAlgoNumber =
					eSIR_SHARED_KEY;
				authFrame.authTransactionSeqNumber =
					SIR_MAC_AUTH_FRAME_4;
				authFrame.authStatusCode =
					eSIR_MAC_CHALLENGE_FAILURE_STATUS;

				lim_send_auth_mgmt_frame(pMac, &authFrame,
							 pHdr->sa,
							 LIM_NO_WEP_IN_FC,
							 psessionEntry);
				return;
			}
			if ((sir_convert_auth_frame2_struct
				(pMac, plainBody, frameLen - 8,
				     &rxAuthFrame) != eSIR_SUCCESS) ||
			    (!is_auth_valid
				     (pMac, &rxAuthFrame, psessionEntry))) {
				lim_log(pMac, LOGE,
					FL
					("failed to convert Auth Frame to structure or Auth is not valid "));
				return;
			}
		} else {
			/* Log error */
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("received Authentication frame3 from peer that while privacy option is turned OFF "
					       MAC_ADDRESS_STR),
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )
			/**
			 * Privacy option is not implemented.
			 * So reject Authentication frame received with
			 * WEP bit set by sending Authentication frame
			 * with 'challenge failure' status code. This is
			 * another strange thing in the spec. Status code
			 * should have been 'unsupported algorithm' status code.
			 */
			authFrame.authAlgoNumber = eSIR_SHARED_KEY;
			authFrame.authTransactionSeqNumber =
				SIR_MAC_AUTH_FRAME_4;
			authFrame.authStatusCode =
				eSIR_MAC_CHALLENGE_FAILURE_STATUS;

			lim_send_auth_mgmt_frame(pMac, &authFrame,
						 pHdr->sa,
						 LIM_NO_WEP_IN_FC, psessionEntry);
			return;
		} /* else if (wlan_cfg_get_int(CFG_PRIVACY_OPTION_IMPLEMENTED)) */
	} /* if (fc.wep) */
	else {
		if ((sir_convert_auth_frame2_struct(pMac, pBody,
						    frameLen,
						    &rxAuthFrame) != eSIR_SUCCESS)
		    || (!is_auth_valid(pMac, &rxAuthFrame, psessionEntry))) {
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL
					       ("failed to convert Auth Frame to structure or Auth is not valid "));
			       )
			return;
		}
	}

	pRxAuthFrameBody = &rxAuthFrame;

	PELOGW(lim_log(pMac, LOGW,
		       FL
			       ("Received Auth frame with type=%d seqnum=%d, status=%d (%d)"),
		       (uint32_t) pRxAuthFrameBody->authAlgoNumber,
		       (uint32_t) pRxAuthFrameBody->authTransactionSeqNumber,
		       (uint32_t) pRxAuthFrameBody->authStatusCode,
		       (uint32_t) pMac->lim.gLimNumPreAuthContexts);
	       )

	switch (pRxAuthFrameBody->authTransactionSeqNumber) {
	case SIR_MAC_AUTH_FRAME_1:
		/* AuthFrame 1 */

		pStaDs = dph_lookup_hash_entry(pMac, pHdr->sa,
					       &assocId,
					       &psessionEntry->dph.dphHashTable);
		if (pStaDs) {
			tLimMlmDisassocReq *pMlmDisassocReq = NULL;
			tLimMlmDeauthReq *pMlmDeauthReq = NULL;
			tAniBool isConnected = eSIR_TRUE;

			pMlmDisassocReq =
				pMac->lim.limDisassocDeauthCnfReq.pMlmDisassocReq;
			if (pMlmDisassocReq
			    &&
			    (cdf_mem_compare
				     ((uint8_t *) pHdr->sa,
				     (uint8_t *) &pMlmDisassocReq->peerMacAddr,
				     sizeof(tSirMacAddr)))) {
				PELOGE(lim_log
					       (pMac, LOGE,
					       FL("TODO:Ack for disassoc "
						  "frame is pending Issue delsta for "
						  MAC_ADDRESS_STR),
					       MAC_ADDR_ARRAY(pMlmDisassocReq->
							      peerMacAddr));
				       )
				lim_process_disassoc_ack_timeout(pMac);
				isConnected = eSIR_FALSE;
			}
			pMlmDeauthReq =
				pMac->lim.limDisassocDeauthCnfReq.pMlmDeauthReq;
			if (pMlmDeauthReq
			    &&
			    (cdf_mem_compare
				     ((uint8_t *) pHdr->sa,
				     (uint8_t *) &pMlmDeauthReq->peer_macaddr.bytes,
				     CDF_MAC_ADDR_SIZE))) {
				PELOGE(lim_log
				      (pMac, LOGE,
				       FL("TODO:Ack for deauth frame is pending Issue delsta for "
					  MAC_ADDRESS_STR),
					  MAC_ADDR_ARRAY(pMlmDeauthReq->
							 peer_macaddr.bytes));
				       )
				lim_process_deauth_ack_timeout(pMac);
				isConnected = eSIR_FALSE;
			}

			/* pStaDS != NULL and isConnected = 1 means the STA is already
			 * connected, But SAP received the Auth from that station.
			 * For non PMF connection send Deauth frame as STA will retry
			 * to connect back.
			 *
			 * For PMF connection the AP should not tear down or otherwise
			 * modify the state of the existing association until the
			 * SA-Query procedure determines that the original SA is
			 * invalid.
			 */
			if (isConnected
#ifdef WLAN_FEATURE_11W
			    && !pStaDs->rmfEnabled
#endif
			    ) {
				lim_log(pMac, LOGE,
					FL
						("STA is already connected but received auth frame"
						"Send the Deauth and lim Delete Station Context"
						"(staId: %d, assocId: %d) "),
					pStaDs->staIndex, assocId);
				lim_send_deauth_mgmt_frame(pMac,
							   eSIR_MAC_UNSPEC_FAILURE_REASON,
							   (uint8_t *) pHdr->sa,
							   psessionEntry, false);
				lim_trigger_sta_deletion(pMac, pStaDs,
							 psessionEntry);
				return;
			}
		}
		/* / Check if there exists pre-auth context for this STA */
		pAuthNode = lim_search_pre_auth_list(pMac, pHdr->sa);
		if (pAuthNode) {
			/* / Pre-auth context exists for the STA */
			if (pHdr->fc.retry == 0 ||
					pAuthNode->seq_num != curr_seq_num) {
				/**
				 * STA is initiating brand-new Authentication
				 * sequence after local Auth Response timeout.
				 * Or STA retrying to transmit First Auth frame due to packet drop OTA
				 * Delete Pre-auth node and fall through.
				 */
				if (pAuthNode->fTimerStarted) {
					lim_deactivate_and_change_per_sta_id_timer
						(pMac, eLIM_AUTH_RSP_TIMER,
						pAuthNode->authNodeIdx);
				}
				PELOGE(lim_log
					       (pMac, LOGE,
					       FL
						       ("STA is initiating brand-new Authentication ..."));
				       )
				lim_delete_pre_auth_node(pMac, pHdr->sa);
				/**
				 *  SAP Mode:Disassociate the station and
				 *  delete its entry if we have its entry
				 *  already and received "auth" from the
				 *  same station.
				 */

				for (assocId = 0; assocId < psessionEntry->dph.dphHashTable.size; assocId++)    /* Softap dphHashTable.size = 8 */
				{
					pStaDs =
						dph_get_hash_entry(pMac, assocId,
								   &psessionEntry->dph.
								   dphHashTable);

					if (NULL == pStaDs)
						continue;

					if (pStaDs->valid) {
						if (cdf_mem_compare
							    ((uint8_t *) &pStaDs->
							    staAddr,
							    (uint8_t *) &(pHdr->sa),
							    (uint8_t) (sizeof
								       (tSirMacAddr))))
							break;
					}

					pStaDs = NULL;
				}

				if (NULL != pStaDs
#ifdef WLAN_FEATURE_11W
				    && !pStaDs->rmfEnabled
#endif
				    ) {
					PELOGE(lim_log(pMac, LOGE,
						       FL
							       ("lim Delete Station Context (staId: %d, assocId: %d) "),
						       pStaDs->staIndex,
						       assocId);
					       )
					lim_send_deauth_mgmt_frame(pMac,
								   eSIR_MAC_UNSPEC_FAILURE_REASON,
								   (uint8_t *)
								   pAuthNode->
								   peerMacAddr,
								   psessionEntry,
								   false);
					lim_trigger_sta_deletion(pMac, pStaDs,
								 psessionEntry);
					return;
				}
			} else {
				/*
				 * This can happen when first authentication frame is received
				 * but ACK lost at STA side, in this case 2nd auth frame is already
				 * in transmission queue
				 * */
				PELOGE(lim_log
					       (pMac, LOGE,
					       FL
						       ("STA is initiating Authentication after ACK lost..."));
				       )
				return;
			}
		}
		if (wlan_cfg_get_int(pMac, WNI_CFG_MAX_NUM_PRE_AUTH,
				     (uint32_t *) &maxNumPreAuth) !=
		    eSIR_SUCCESS) {
			/**
			 * Could not get MaxNumPreAuth
			 * from CFG. Log error.
			 */
			lim_log(pMac, LOGP,
				FL("could not retrieve MaxNumPreAuth"));
		}
		if (pMac->lim.gLimNumPreAuthContexts == maxNumPreAuth &&
			!lim_delete_open_auth_pre_auth_node(pMac)) {
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("Max number of preauth context reached"));
			       )
			/**
			 * Maximum number of pre-auth contexts
			 * reached. Send Authentication frame
			 * with unspecified failure
			 */
			authFrame.authAlgoNumber =
				pRxAuthFrameBody->authAlgoNumber;
			authFrame.authTransactionSeqNumber =
				pRxAuthFrameBody->authTransactionSeqNumber + 1;
			authFrame.authStatusCode =
				eSIR_MAC_UNSPEC_FAILURE_STATUS;

			lim_send_auth_mgmt_frame(pMac, &authFrame,
						 pHdr->sa,
						 LIM_NO_WEP_IN_FC, psessionEntry);

			return;
		}
		/* / No Pre-auth context exists for the STA. */
		if (lim_is_auth_algo_supported(pMac, (tAniAuthType)
					       pRxAuthFrameBody->authAlgoNumber,
					       psessionEntry)) {
			switch (pRxAuthFrameBody->authAlgoNumber) {
			case eSIR_OPEN_SYSTEM:
				PELOGW(lim_log
					       (pMac, LOGW,
					       FL("=======> eSIR_OPEN_SYSTEM  ..."));
				       )
				/* / Create entry for this STA in pre-auth list */
				pAuthNode =
					lim_acquire_free_pre_auth_node(pMac,
								       &pMac->lim.
								       gLimPreAuthTimerTable);
				if (pAuthNode == NULL) {
					/* Log error */
					lim_log(pMac, LOGW,
						FL
							("Max pre-auth nodes reached "));
					lim_print_mac_addr(pMac, pHdr->sa, LOGW);

					return;
				}

				PELOG1(lim_log
					       (pMac, LOG1,
					       FL("Alloc new data: %x peer "),
					       pAuthNode);
				       lim_print_mac_addr(pMac, pHdr->sa, LOG1);
				       )

				cdf_mem_copy((uint8_t *) pAuthNode->
					     peerMacAddr, pHdr->sa,
					     sizeof(tSirMacAddr));

				pAuthNode->mlmState =
					eLIM_MLM_AUTHENTICATED_STATE;
				pAuthNode->authType = (tAniAuthType)
						      pRxAuthFrameBody->authAlgoNumber;
				pAuthNode->fSeen = 0;
				pAuthNode->fTimerStarted = 0;
				pAuthNode->seq_num =
					((pHdr->seqControl.seqNumHi << 4) |
					 (pHdr->seqControl.seqNumLo));
				pAuthNode->timestamp =
					cdf_mc_timer_get_system_ticks();
				lim_add_pre_auth_node(pMac, pAuthNode);

				/**
				 * Send Authenticaton frame with Success
				 * status code.
				 */

				authFrame.authAlgoNumber =
					pRxAuthFrameBody->authAlgoNumber;
				authFrame.authTransactionSeqNumber =
					pRxAuthFrameBody->authTransactionSeqNumber +
					1;
				authFrame.authStatusCode =
					eSIR_MAC_SUCCESS_STATUS;
				lim_send_auth_mgmt_frame(pMac, &authFrame, pHdr->sa,
							 LIM_NO_WEP_IN_FC,
							 psessionEntry);
				break;

			case eSIR_SHARED_KEY:
				PELOGW(lim_log
					       (pMac, LOGW,
					       FL("=======> eSIR_SHARED_KEY  ..."));
				       )
				if (LIM_IS_AP_ROLE(psessionEntry)) {
					val = psessionEntry->privacy;
				} else
				if (wlan_cfg_get_int
					    (pMac, WNI_CFG_PRIVACY_ENABLED,
					    &val) != eSIR_SUCCESS) {
					/**
					 * Could not get Privacy option
					 * from CFG. Log error.
					 */
					lim_log(pMac, LOGP,
						FL
							("could not retrieve Privacy option"));
				}
				cfgPrivacyOptImp = (uint8_t) val;
				if (!cfgPrivacyOptImp) {
					/* Log error */
					PELOGE(lim_log(pMac, LOGE,
						       FL
							       ("received Auth frame for unsupported auth algorithm %d "
							       MAC_ADDRESS_STR),
						       pRxAuthFrameBody->
						       authAlgoNumber,
						       MAC_ADDR_ARRAY(pHdr->sa));
					       )

					/**
					 * Authenticator does not have WEP
					 * implemented.
					 * Reject by sending Authentication frame
					 * with Auth algorithm not supported status
					 * code.
					 */
					authFrame.authAlgoNumber =
						pRxAuthFrameBody->authAlgoNumber;
					authFrame.authTransactionSeqNumber =
						pRxAuthFrameBody->
						authTransactionSeqNumber + 1;
					authFrame.authStatusCode =
						eSIR_MAC_AUTH_ALGO_NOT_SUPPORTED_STATUS;

					lim_send_auth_mgmt_frame(pMac, &authFrame,
								 pHdr->sa,
								 LIM_NO_WEP_IN_FC,
								 psessionEntry);
					return;
				} else {
					/* Create entry for this STA */
					/* in pre-auth list */
					pAuthNode =
						lim_acquire_free_pre_auth_node(pMac,
									       &pMac->
									       lim.
									       gLimPreAuthTimerTable);
					if (pAuthNode == NULL) {
						/* Log error */
						lim_log(pMac, LOGW,
							FL
								("Max pre-auth nodes reached "));
						lim_print_mac_addr(pMac, pHdr->sa,
								   LOGW);

						return;
					}

					cdf_mem_copy((uint8_t *) pAuthNode->
						     peerMacAddr, pHdr->sa,
						     sizeof(tSirMacAddr));

					pAuthNode->mlmState =
						eLIM_MLM_WT_AUTH_FRAME3_STATE;
					pAuthNode->authType = (tAniAuthType)
							      pRxAuthFrameBody->authAlgoNumber;
					pAuthNode->fSeen = 0;
					pAuthNode->fTimerStarted = 0;
					pAuthNode->seq_num =
						((pHdr->seqControl.seqNumHi <<
						  4) |
						 (pHdr->seqControl.seqNumLo));
					pAuthNode->timestamp =
						cdf_mc_timer_get_system_ticks();
					lim_add_pre_auth_node(pMac, pAuthNode);

					PELOG1(lim_log
						       (pMac, LOG1,
						       FL
							       ("Alloc new data: %x id %d peer "),
						       pAuthNode,
						       pAuthNode->authNodeIdx);
					       )
					PELOG1(lim_print_mac_addr
						       (pMac, pHdr->sa, LOG1);
					       )
					/* / Create and activate Auth Response timer */
					if (tx_timer_change_context
						    (&pAuthNode->timer,
						    pAuthNode->authNodeIdx) !=
					    TX_SUCCESS) {
						/* / Could not start Auth response timer. */
						/* Log error */
						lim_log(pMac, LOGP,
							FL
								("Unable to chg context auth response timer for peer "));
						lim_print_mac_addr(pMac, pHdr->sa,
								   LOGP);

						/**
						 * Send Authenticaton frame with
						 * unspecified failure status code.
						 */

						authFrame.authAlgoNumber =
							pRxAuthFrameBody->
							authAlgoNumber;
						authFrame.
						authTransactionSeqNumber =
							pRxAuthFrameBody->
							authTransactionSeqNumber +
							1;
						authFrame.authStatusCode =
							eSIR_MAC_UNSPEC_FAILURE_STATUS;

						lim_send_auth_mgmt_frame(pMac,
									 &authFrame,
									 pHdr->sa,
									 LIM_NO_WEP_IN_FC,
									 psessionEntry);

						lim_delete_pre_auth_node(pMac,
									 pHdr->sa);
						return;
					}

					lim_activate_auth_rsp_timer(pMac,
								    pAuthNode);

					pAuthNode->fTimerStarted = 1;

					/* get random bytes and use as */
					/* challenge text. If it fails we already have random stack bytes. */
					if (!CDF_IS_STATUS_SUCCESS
						    (cds_rand_get_bytes
							    (0, (uint8_t *) challengeTextArray,
							    SIR_MAC_AUTH_CHALLENGE_LENGTH))) {
						lim_log(pMac, LOGE,
							FL
								("Challenge text preparation failed in lim_process_auth_frame"));
					}

					pChallenge = pAuthNode->challengeText;

					cdf_mem_copy(pChallenge,
						     (uint8_t *)
						     challengeTextArray,
						     sizeof
						     (challengeTextArray));

					/**
					 * Sending Authenticaton frame with challenge.
					 */

					authFrame.authAlgoNumber =
						pRxAuthFrameBody->authAlgoNumber;
					authFrame.authTransactionSeqNumber =
						pRxAuthFrameBody->
						authTransactionSeqNumber + 1;
					authFrame.authStatusCode =
						eSIR_MAC_SUCCESS_STATUS;
					authFrame.type =
						SIR_MAC_CHALLENGE_TEXT_EID;
					authFrame.length =
						SIR_MAC_AUTH_CHALLENGE_LENGTH;
					cdf_mem_copy(authFrame.challengeText,
						     pAuthNode->challengeText,
						     SIR_MAC_AUTH_CHALLENGE_LENGTH);

					lim_send_auth_mgmt_frame(pMac, &authFrame,
								 pHdr->sa,
								 LIM_NO_WEP_IN_FC,
								 psessionEntry);
				} /* if (wlan_cfg_get_int(CFG_PRIVACY_OPTION_IMPLEMENTED)) */

				break;

			default:
				/* Log error */
				PELOGE(lim_log(pMac, LOGE,
					       FL
						       ("received Auth frame for unsupported auth algorithm %d "
						       MAC_ADDRESS_STR),
					       pRxAuthFrameBody->authAlgoNumber,
					       MAC_ADDR_ARRAY(pHdr->sa));
				       )

				/**
				 * Responding party does not support the
				 * authentication algorithm requested by
				 * sending party.
				 * Reject by sending Authentication frame
				 * with auth algorithm not supported status code
				 */
				authFrame.authAlgoNumber =
					pRxAuthFrameBody->authAlgoNumber;
				authFrame.authTransactionSeqNumber =
					pRxAuthFrameBody->authTransactionSeqNumber +
					1;
				authFrame.authStatusCode =
					eSIR_MAC_AUTH_ALGO_NOT_SUPPORTED_STATUS;

				lim_send_auth_mgmt_frame(pMac, &authFrame,
							 pHdr->sa,
							 LIM_NO_WEP_IN_FC,
							 psessionEntry);
				return;
			} /* end switch(pRxAuthFrameBody->authAlgoNumber) */
		} /* if (lim_is_auth_algo_supported(pRxAuthFrameBody->authAlgoNumber)) */
		else {
			/* Log error */
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("received Authentication frame for unsupported auth algorithm %d "
					       MAC_ADDRESS_STR),
				       pRxAuthFrameBody->authAlgoNumber,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			/**
			 * Responding party does not support the
			 * authentication algorithm requested by sending party.
			 * Reject Authentication with StatusCode=13.
			 */
			authFrame.authAlgoNumber =
				pRxAuthFrameBody->authAlgoNumber;
			authFrame.authTransactionSeqNumber =
				pRxAuthFrameBody->authTransactionSeqNumber + 1;
			authFrame.authStatusCode =
				eSIR_MAC_AUTH_ALGO_NOT_SUPPORTED_STATUS;

			lim_send_auth_mgmt_frame(pMac, &authFrame,
						 pHdr->sa,
						 LIM_NO_WEP_IN_FC, psessionEntry);
			return;
		} /* end if (lim_is_auth_algo_supported(pRxAuthFrameBody->authAlgoNumber)) */
		break;

	case SIR_MAC_AUTH_FRAME_2:
		/* AuthFrame 2 */

		if (psessionEntry->limMlmState != eLIM_MLM_WT_AUTH_FRAME2_STATE) {
#ifdef WLAN_FEATURE_VOWIFI_11R
			/**
			 * Check if a Reassociation is in progress and this is a
			 * Pre-Auth frame
			 */
			if ((LIM_IS_STA_ROLE(psessionEntry) ||
			     LIM_IS_BT_AMP_STA_ROLE(psessionEntry))
			    && (psessionEntry->limSmeState ==
				eLIM_SME_WT_REASSOC_STATE)
			    && (pRxAuthFrameBody->authStatusCode ==
				eSIR_MAC_SUCCESS_STATUS)
			    && (psessionEntry->ftPEContext.pFTPreAuthReq !=
				NULL)
			    &&
			    (cdf_mem_compare
				     (psessionEntry->ftPEContext.pFTPreAuthReq->
				     preAuthbssId, pHdr->sa, sizeof(tSirMacAddr)))) {
				/* Update the FTIEs in the saved auth response */
				PELOGW(lim_log
					       (pMac, LOGW,
					       FL("received another PreAuth frame2"
						  " from peer " MAC_ADDRESS_STR
						  " in Smestate %d"),
					       MAC_ADDR_ARRAY(pHdr->sa),
					       psessionEntry->limSmeState);
				       )

				psessionEntry->ftPEContext.
				saved_auth_rsp_length = 0;
				if ((pBody != NULL)
				    && (frameLen < MAX_FTIE_SIZE)) {
					cdf_mem_copy(psessionEntry->ftPEContext.
						     saved_auth_rsp, pBody,
						     frameLen);
					psessionEntry->ftPEContext.
					saved_auth_rsp_length = frameLen;
				}
			} else
#endif
			{
				/**
				 * Received Authentication frame2 in an unexpected state.
				 * Log error and ignore the frame.
				 */

				/* Log error */
				PELOG1(lim_log(pMac, LOG1,
					       FL
						       ("received Auth frame2 from peer in state %d, addr "),
					       psessionEntry->limMlmState);
				       )
				PELOG1(lim_print_mac_addr
					       (pMac, pHdr->sa, LOG1);
				       )
			}

			return;

		}

		if (!cdf_mem_compare((uint8_t *) pHdr->sa,
				     (uint8_t *) &pMac->lim.gpLimMlmAuthReq->
				     peerMacAddr, sizeof(tSirMacAddr))) {
			/**
			 * Received Authentication frame from an entity
			 * other than one request was initiated.
			 * Wait until Authentication Failure Timeout.
			 */

			/* Log error */
			PELOGW(lim_log(pMac, LOGW,
				       FL
					       ("received Auth frame2 from unexpected peer "
					       MAC_ADDRESS_STR),
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			break;
		}

		if (pRxAuthFrameBody->authStatusCode ==
		    eSIR_MAC_AUTH_ALGO_NOT_SUPPORTED_STATUS) {
			/**
			 * Interoperability workaround: Linksys WAP4400N is returning
			 * wrong authType in OpenAuth response in case of
			 * SharedKey AP configuration. Pretend we don't see that,
			 * so upper layer can fallback to SharedKey authType,
			 * and successfully connect to the AP.
			 */
			if (pRxAuthFrameBody->authAlgoNumber !=
			    pMac->lim.gpLimMlmAuthReq->authType) {
				pRxAuthFrameBody->authAlgoNumber =
					pMac->lim.gpLimMlmAuthReq->authType;
			}
		}

		if (pRxAuthFrameBody->authAlgoNumber !=
		    pMac->lim.gpLimMlmAuthReq->authType) {
			/**
			 * Received Authentication frame with an auth
			 * algorithm other than one requested.
			 * Wait until Authentication Failure Timeout.
			 */

			/* Log error */
			PELOGW(lim_log(pMac, LOGW,
				       FL
					       ("received Auth frame2 for unexpected auth algo number %d "
					       MAC_ADDRESS_STR),
				       pRxAuthFrameBody->authAlgoNumber,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			break;
		}

		if (pRxAuthFrameBody->authStatusCode == eSIR_MAC_SUCCESS_STATUS) {
			if (pRxAuthFrameBody->authAlgoNumber ==
			    eSIR_OPEN_SYSTEM) {
				psessionEntry->limCurrentAuthType =
					eSIR_OPEN_SYSTEM;

				pAuthNode =
					lim_acquire_free_pre_auth_node(pMac,
								       &pMac->lim.
								       gLimPreAuthTimerTable);

				if (pAuthNode == NULL) {
					/* Log error */
					lim_log(pMac, LOGW,
						FL
							("Max pre-auth nodes reached "));
					lim_print_mac_addr(pMac, pHdr->sa, LOGW);

					return;
				}

				PELOG1(lim_log
					       (pMac, LOG1,
					       FL("Alloc new data: %x peer "),
					       pAuthNode);
				       )
				PELOG1(lim_print_mac_addr
					       (pMac, pHdr->sa, LOG1);
				       )

				cdf_mem_copy((uint8_t *) pAuthNode->
					     peerMacAddr,
					     pMac->lim.gpLimMlmAuthReq->
					     peerMacAddr,
					     sizeof(tSirMacAddr));
				pAuthNode->fTimerStarted = 0;
				pAuthNode->authType =
					pMac->lim.gpLimMlmAuthReq->authType;
				pAuthNode->seq_num =
					((pHdr->seqControl.seqNumHi << 4) |
					 (pHdr->seqControl.seqNumLo));
				pAuthNode->timestamp =
					cdf_mc_timer_get_system_ticks();
				lim_add_pre_auth_node(pMac, pAuthNode);

				lim_restore_from_auth_state(pMac, eSIR_SME_SUCCESS,
							    pRxAuthFrameBody->
							    authStatusCode,
							    psessionEntry);
			} /* if (pRxAuthFrameBody->authAlgoNumber == eSIR_OPEN_SYSTEM) */
			else {
				/* Shared key authentication */

				if (LIM_IS_AP_ROLE(psessionEntry)) {
					val = psessionEntry->privacy;
				} else
				if (wlan_cfg_get_int
					    (pMac, WNI_CFG_PRIVACY_ENABLED,
					    &val) != eSIR_SUCCESS) {
					/**
					 * Could not get Privacy option
					 * from CFG. Log error.
					 */
					lim_log(pMac, LOGP,
						FL
							("could not retrieve Privacy option"));
				}
				cfgPrivacyOptImp = (uint8_t) val;
				if (!cfgPrivacyOptImp) {
					/**
					 * Requesting STA does not have WEP implemented.
					 * Reject with unsupported authentication algorithm
					 * Status code and wait until auth failure timeout
					 */

					/* Log error */
					PELOGE(lim_log(pMac, LOGE,
						       FL
							       ("received Auth frame from peer for unsupported auth algo %d "
							       MAC_ADDRESS_STR),
						       pRxAuthFrameBody->
						       authAlgoNumber,
						       MAC_ADDR_ARRAY(pHdr->sa));
					       )

					authFrame.authAlgoNumber =
						pRxAuthFrameBody->authAlgoNumber;
					authFrame.authTransactionSeqNumber =
						pRxAuthFrameBody->
						authTransactionSeqNumber + 1;
					authFrame.authStatusCode =
						eSIR_MAC_AUTH_ALGO_NOT_SUPPORTED_STATUS;

					lim_send_auth_mgmt_frame(pMac, &authFrame,
								 pHdr->sa,
								 LIM_NO_WEP_IN_FC,
								 psessionEntry);
					return;
				} else {

					if (pRxAuthFrameBody->type !=
					    SIR_MAC_CHALLENGE_TEXT_EID) {
						/* Log error */
						PELOGE(lim_log(pMac, LOGE,
							       FL
								       ("received Auth frame with invalid challenge text IE"));
						       )

						return;
					}

					if (wlan_cfg_get_int
						    (pMac,
						    WNI_CFG_WEP_DEFAULT_KEYID,
						    &val) != eSIR_SUCCESS) {
						/**
						 * Could not get Default keyId
						 * from CFG. Log error.
						 */
						lim_log(pMac, LOGP,
							FL
							("could not retrieve Default keyId"));
					}
					keyId = (uint8_t) val;

					val = SIR_MAC_KEY_LENGTH;

					if (LIM_IS_AP_ROLE(psessionEntry)) {
						tpSirKeys pKey;
						pKey =
							&psessionEntry->
							WEPKeyMaterial
							[keyId].key[0];
						cdf_mem_copy(defaultKey,
							     pKey->key,
							     pKey->
							     keyLength);
					} else
					if (wlan_cfg_get_str
						    (pMac,
						    (uint16_t)
						    (WNI_CFG_WEP_DEFAULT_KEY_1
						     + keyId), defaultKey,
						    &val)
					    != eSIR_SUCCESS) {
						/* / Could not get Default key from CFG. */
						/* Log error. */
						lim_log(pMac, LOGP,
							FL
							("could not retrieve Default key"));

						authFrame.authAlgoNumber =
							pRxAuthFrameBody->
							authAlgoNumber;
						authFrame.
						authTransactionSeqNumber =
								pRxAuthFrameBody->
								authTransactionSeqNumber
								+ 1;
						authFrame.authStatusCode =
							eSIR_MAC_CHALLENGE_FAILURE_STATUS;

						lim_send_auth_mgmt_frame
							(pMac, &authFrame,
							pHdr->sa,
							LIM_NO_WEP_IN_FC,
							psessionEntry);

						lim_restore_from_auth_state
							(pMac,
							eSIR_SME_INVALID_WEP_DEFAULT_KEY,
							eSIR_MAC_UNSPEC_FAILURE_REASON,
							psessionEntry);

						break;
					}
					key_length = val;
					((tpSirMacAuthFrameBody)
					 plainBody)->authAlgoNumber =
						sir_swap_u16if_needed(pRxAuthFrameBody->
								      authAlgoNumber);
					((tpSirMacAuthFrameBody)
					 plainBody)->
					authTransactionSeqNumber =
						sir_swap_u16if_needed((uint16_t)
								      (pRxAuthFrameBody->
								       authTransactionSeqNumber + 1));
					((tpSirMacAuthFrameBody)
					 plainBody)->authStatusCode =
						eSIR_MAC_SUCCESS_STATUS;
					((tpSirMacAuthFrameBody)
					 plainBody)->type =
						SIR_MAC_CHALLENGE_TEXT_EID;
					((tpSirMacAuthFrameBody)
					 plainBody)->length =
						SIR_MAC_AUTH_CHALLENGE_LENGTH;
					cdf_mem_copy((uint8_t*) ((tpSirMacAuthFrameBody) plainBody)->challengeText,
							pRxAuthFrameBody->challengeText,
							SIR_MAC_AUTH_CHALLENGE_LENGTH);

					lim_encrypt_auth_frame(pMac, keyId,
							       defaultKey,
							       plainBody,
							       encrAuthFrame,
							       key_length);

					psessionEntry->limMlmState =
						eLIM_MLM_WT_AUTH_FRAME4_STATE;
					MTRACE(mac_trace
						       (pMac,
						       TRACE_CODE_MLM_STATE,
						       psessionEntry->
						       peSessionId,
						       psessionEntry->
						       limMlmState));

					lim_send_auth_mgmt_frame(pMac,
								 (tpSirMacAuthFrameBody)
								 encrAuthFrame,
								 pHdr->sa,
								 LIM_WEP_IN_FC,
								 psessionEntry);

					break;
				} /* end if (!wlan_cfg_get_int(CFG_PRIVACY_OPTION_IMPLEMENTED)) */
			} /* end if (pRxAuthFrameBody->authAlgoNumber == eSIR_OPEN_SYSTEM) */
		} /* if (pRxAuthFrameBody->authStatusCode == eSIR_MAC_SUCCESS_STATUS) */
		else {
			/**
			 * Authentication failure.
			 * Return Auth confirm with received failure code to SME
			 */

			/* Log error */
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("received Auth frame from peer with failure code %d "
					       MAC_ADDRESS_STR),
				       pRxAuthFrameBody->authStatusCode,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			lim_restore_from_auth_state(pMac, eSIR_SME_AUTH_REFUSED,
						    pRxAuthFrameBody->
						    authStatusCode,
						    psessionEntry);
		} /* end if (pRxAuthFrameBody->authStatusCode == eSIR_MAC_SUCCESS_STATUS) */

		break;

	case SIR_MAC_AUTH_FRAME_3:
		/* AuthFrame 3 */

		if (pRxAuthFrameBody->authAlgoNumber != eSIR_SHARED_KEY) {
			/* Log error */
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("received Auth frame3 from peer with auth algo number %d "
					       MAC_ADDRESS_STR),
				       pRxAuthFrameBody->authAlgoNumber,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			/**
			 * Received Authentication frame3 with algorithm other than
			 * Shared Key authentication type. Reject with Auth frame4
			 * with 'out of sequence' status code.
			 */
			authFrame.authAlgoNumber = eSIR_SHARED_KEY;
			authFrame.authTransactionSeqNumber =
				SIR_MAC_AUTH_FRAME_4;
			authFrame.authStatusCode =
				eSIR_MAC_AUTH_FRAME_OUT_OF_SEQ_STATUS;

			lim_send_auth_mgmt_frame(pMac, &authFrame,
						 pHdr->sa,
						 LIM_NO_WEP_IN_FC, psessionEntry);
			return;
		}

		if (LIM_IS_AP_ROLE(psessionEntry) ||
		    LIM_IS_BT_AMP_AP_ROLE(psessionEntry) ||
		    LIM_IS_IBSS_ROLE(psessionEntry)) {
			/**
			 * Check if wep bit was set in FC. If not set,
			 * reject with Authentication frame4 with
			 * 'challenge failure' status code.
			 */
			if (!pHdr->fc.wep) {
				/* Log error */
				PELOGE(lim_log(pMac, LOGE,
					       FL
						       ("received Auth frame3 from peer with no WEP bit set "
						       MAC_ADDRESS_STR),
					       MAC_ADDR_ARRAY(pHdr->sa));
				       )
				/* / WEP bit is not set in FC of Auth Frame3 */
				authFrame.authAlgoNumber = eSIR_SHARED_KEY;
				authFrame.authTransactionSeqNumber =
					SIR_MAC_AUTH_FRAME_4;
				authFrame.authStatusCode =
					eSIR_MAC_CHALLENGE_FAILURE_STATUS;

				lim_send_auth_mgmt_frame(pMac, &authFrame,
							 pHdr->sa,
							 LIM_NO_WEP_IN_FC,
							 psessionEntry);
				return;
			}

			pAuthNode = lim_search_pre_auth_list(pMac, pHdr->sa);
			if (pAuthNode == NULL) {
				/* Log error */
				PELOGE(lim_log(pMac, LOGW,
					       FL
						       ("received AuthFrame3 from peer that has no preauth context "
						       MAC_ADDRESS_STR),
					       MAC_ADDR_ARRAY(pHdr->sa));
				       )
				/**
				 * No 'pre-auth' context exists for
				 * this STA that sent an Authentication
				 * frame3.
				 * Send Auth frame4 with 'out of sequence'
				 * status code.
				 */
				authFrame.authAlgoNumber = eSIR_SHARED_KEY;
				authFrame.authTransactionSeqNumber =
					SIR_MAC_AUTH_FRAME_4;
				authFrame.authStatusCode =
					eSIR_MAC_AUTH_FRAME_OUT_OF_SEQ_STATUS;

				lim_send_auth_mgmt_frame(pMac, &authFrame,
							 pHdr->sa,
							 LIM_NO_WEP_IN_FC,
							 psessionEntry);
				return;
			}

			if (pAuthNode->mlmState ==
			    eLIM_MLM_AUTH_RSP_TIMEOUT_STATE) {
				/* Log error */
				lim_log(pMac, LOGW,
					FL
						("auth response timer timedout for peer "
						MAC_ADDRESS_STR),
					MAC_ADDR_ARRAY(pHdr->sa));

				/**
				 * Received Auth Frame3 after Auth Response timeout.
				 * Reject by sending Auth Frame4 with
				 * Auth respone timeout Status Code.
				 */
				authFrame.authAlgoNumber = eSIR_SHARED_KEY;
				authFrame.authTransactionSeqNumber =
					SIR_MAC_AUTH_FRAME_4;
				authFrame.authStatusCode =
					eSIR_MAC_AUTH_RSP_TIMEOUT_STATUS;

				lim_send_auth_mgmt_frame(pMac, &authFrame,
							 pHdr->sa,
							 LIM_NO_WEP_IN_FC,
							 psessionEntry);

				/* / Delete pre-auth context of STA */
				lim_delete_pre_auth_node(pMac, pHdr->sa);

				return;
			} /* end switch (pAuthNode->mlmState) */

			if (pRxAuthFrameBody->authStatusCode !=
			    eSIR_MAC_SUCCESS_STATUS) {
				/**
				 * Received Authenetication Frame 3 with status code
				 * other than success. Wait until Auth response timeout
				 * to delete STA context.
				 */

				/* Log error */
				PELOGE(lim_log(pMac, LOGE,
					       FL
						       ("received Auth frame3 from peer with status code %d "
						       MAC_ADDRESS_STR),
					       pRxAuthFrameBody->authStatusCode,
					       MAC_ADDR_ARRAY(pHdr->sa));
				       )

				return;
			}

			/**
			 * Check if received challenge text is same as one sent in
			 * Authentication frame3
			 */

			if (cdf_mem_compare(pRxAuthFrameBody->challengeText,
					    pAuthNode->challengeText,
					    SIR_MAC_AUTH_CHALLENGE_LENGTH)) {
				/* / Challenge match. STA is autheticated ! */

				/* / Delete Authentication response timer if running */
				lim_deactivate_and_change_per_sta_id_timer(pMac,
									   eLIM_AUTH_RSP_TIMER,
									   pAuthNode->
									   authNodeIdx);

				pAuthNode->fTimerStarted = 0;
				pAuthNode->mlmState =
					eLIM_MLM_AUTHENTICATED_STATE;

				/**
				 * Send Authentication Frame4 with 'success' Status Code.
				 */
				authFrame.authAlgoNumber = eSIR_SHARED_KEY;
				authFrame.authTransactionSeqNumber =
					SIR_MAC_AUTH_FRAME_4;
				authFrame.authStatusCode =
					eSIR_MAC_SUCCESS_STATUS;

				lim_send_auth_mgmt_frame(pMac, &authFrame,
							 pHdr->sa,
							 LIM_NO_WEP_IN_FC,
							 psessionEntry);
				break;
			} else {
				/* Log error */
				PELOGE(lim_log(pMac, LOGW,
					       FL("Challenge failure for peer "
						  MAC_ADDRESS_STR),
					       MAC_ADDR_ARRAY(pHdr->sa));
				       )
				/**
				 * Challenge Failure.
				 * Send Authentication frame4 with 'challenge failure'
				 * status code and wait until Auth response timeout to
				 * delete STA context.
				 */
				authFrame.authAlgoNumber =
					pRxAuthFrameBody->authAlgoNumber;
				authFrame.authTransactionSeqNumber =
					SIR_MAC_AUTH_FRAME_4;
				authFrame.authStatusCode =
					eSIR_MAC_CHALLENGE_FAILURE_STATUS;

				lim_send_auth_mgmt_frame(pMac, &authFrame,
							 pHdr->sa,
							 LIM_NO_WEP_IN_FC,
							 psessionEntry);
				return;
			}
		} /* if (pMac->lim.gLimSystemRole == eLIM_AP_ROLE || ... */

		break;

	case SIR_MAC_AUTH_FRAME_4:
		/* AuthFrame 4 */
		if (psessionEntry->limMlmState != eLIM_MLM_WT_AUTH_FRAME4_STATE) {
			/**
			 * Received Authentication frame4 in an unexpected state.
			 * Log error and ignore the frame.
			 */

			/* Log error */
			PELOG1(lim_log(pMac, LOG1,
				       FL
					       ("received unexpected Auth frame4 from peer in state %d, addr "
					       MAC_ADDRESS_STR),
				       psessionEntry->limMlmState,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			return;
		}

		if (pRxAuthFrameBody->authAlgoNumber != eSIR_SHARED_KEY) {
			/**
			 * Received Authentication frame4 with algorithm other than
			 * Shared Key authentication type.
			 * Wait until Auth failure timeout to report authentication
			 * failure to SME.
			 */

			/* Log error */
			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("received Auth frame4 from peer with invalid auth algo %d "
					       MAC_ADDRESS_STR),
				       pRxAuthFrameBody->authAlgoNumber,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			return;
		}

		if (!cdf_mem_compare((uint8_t *) pHdr->sa,
				     (uint8_t *) &pMac->lim.gpLimMlmAuthReq->
				     peerMacAddr, sizeof(tSirMacAddr))) {
			/**
			 * Received Authentication frame from an entity
			 * other than one to which request was initiated.
			 * Wait until Authentication Failure Timeout.
			 */

			/* Log error */
			PELOGE(lim_log(pMac, LOGW,
				       FL
					       ("received Auth frame4 from unexpected peer "
					       MAC_ADDRESS_STR),
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			break;
		}

		if (pRxAuthFrameBody->authAlgoNumber !=
		    pMac->lim.gpLimMlmAuthReq->authType) {
			/**
			 * Received Authentication frame with an auth algorithm
			 * other than one requested.
			 * Wait until Authentication Failure Timeout.
			 */

			PELOGE(lim_log(pMac, LOGE,
				       FL
					       ("received Authentication frame from peer with invalid auth seq number %d "
					       MAC_ADDRESS_STR),
				       pRxAuthFrameBody->
				       authTransactionSeqNumber,
				       MAC_ADDR_ARRAY(pHdr->sa));
			       )

			break;
		}

		if (pRxAuthFrameBody->authStatusCode == eSIR_MAC_SUCCESS_STATUS) {
			/**
			 * Authentication Success !
			 * Inform SME of same.
			 */
			psessionEntry->limCurrentAuthType = eSIR_SHARED_KEY;

			pAuthNode =
				lim_acquire_free_pre_auth_node(pMac,
							       &pMac->lim.
							       gLimPreAuthTimerTable);
			if (pAuthNode == NULL) {
				/* Log error */
				lim_log(pMac, LOGW,
					FL("Max pre-auth nodes reached "));
				lim_print_mac_addr(pMac, pHdr->sa, LOGW);

				return;
			}
			PELOG1(lim_log
				       (pMac, LOG1, FL("Alloc new data: %x peer "),
				       pAuthNode);
			       lim_print_mac_addr(pMac, pHdr->sa, LOG1);
			       )

			cdf_mem_copy((uint8_t *) pAuthNode->peerMacAddr,
				     pMac->lim.gpLimMlmAuthReq->peerMacAddr,
				     sizeof(tSirMacAddr));
			pAuthNode->fTimerStarted = 0;
			pAuthNode->authType =
				pMac->lim.gpLimMlmAuthReq->authType;
			pAuthNode->seq_num =
				((pHdr->seqControl.seqNumHi << 4) |
				 (pHdr->seqControl.seqNumLo));
			pAuthNode->timestamp = cdf_mc_timer_get_system_ticks();
			lim_add_pre_auth_node(pMac, pAuthNode);

			lim_restore_from_auth_state(pMac, eSIR_SME_SUCCESS,
						    pRxAuthFrameBody->
						    authStatusCode, psessionEntry);

		} /* if (pRxAuthFrameBody->authStatusCode == eSIR_MAC_SUCCESS_STATUS) */
		else {
			/**
			 * Authentication failure.
			 * Return Auth confirm with received failure code to SME
			 */

			/* Log error */
			PELOGE(lim_log
				       (pMac, LOGE,
				       FL("Authentication failure from peer "
					  MAC_ADDRESS_STR), MAC_ADDR_ARRAY(pHdr->sa));
			       )

			lim_restore_from_auth_state(pMac, eSIR_SME_AUTH_REFUSED,
						    pRxAuthFrameBody->
						    authStatusCode,
						    psessionEntry);
		} /* end if (pRxAuthFrameBody->Status == 0) */

		break;

	default:
		/* / Invalid Authentication Frame received. Ignore it. */

		/* Log error */
		PELOGE(lim_log(pMac, LOGE,
			       FL
				       ("received Auth frame from peer with invalid auth seq "
				       "number %d " MAC_ADDRESS_STR),
			       pRxAuthFrameBody->authTransactionSeqNumber,
			       MAC_ADDR_ARRAY(pHdr->sa));
		       )

		break;
	} /* end switch (pRxAuthFrameBody->authTransactionSeqNumber) */
} /*** end lim_process_auth_frame() ***/

#ifdef WLAN_FEATURE_VOWIFI_11R

/*----------------------------------------------------------------------
 *
 * Pass the received Auth frame. This is possibly the pre-auth from the
 * neighbor AP, in the same mobility domain.
 * This will be used in case of 11r FT.
 *
 * !!!! This is going to be renoved for the next checkin. We will be creating
 * the session before sending out the Auth. Thus when auth response
 * is received we will have a session in progress. !!!!!
 ***----------------------------------------------------------------------
 */
tSirRetStatus lim_process_auth_frame_no_session(tpAniSirGlobal pMac, uint8_t *pBd,
						void *body)
{
	tpSirMacMgmtHdr pHdr;
	tpPESession psessionEntry = NULL;
	uint8_t *pBody;
	uint16_t frameLen;
	tSirMacAuthFrameBody rxAuthFrame;
	tSirMacAuthFrameBody *pRxAuthFrameBody = NULL;
	tSirRetStatus ret_status = eSIR_FAILURE;
	int i;

	pHdr = WMA_GET_RX_MAC_HEADER(pBd);
	pBody = WMA_GET_RX_MPDU_DATA(pBd);
	frameLen = WMA_GET_RX_PAYLOAD_LEN(pBd);

	lim_log(pMac, LOG1,
		FL("Auth Frame Received: BSSID " MAC_ADDRESS_STR " (RSSI %d)"),
		MAC_ADDR_ARRAY(pHdr->bssId),
		(uint) abs((int8_t) WMA_GET_RX_RSSI_NORMALIZED(pBd)));

	/* Auth frame has come on a new BSS, however, we need to find the session
	 * from where the auth-req was sent to the new AP
	 */
	for (i = 0; i < pMac->lim.maxBssId; i++) {
		/* Find first free room in session table */
		if (pMac->lim.gpSession[i].valid == true &&
		    pMac->lim.gpSession[i].ftPEContext.ftPreAuthSession ==
		    true) {
			/* Found the session */
			psessionEntry = &pMac->lim.gpSession[i];
			pMac->lim.gpSession[i].ftPEContext.ftPreAuthSession =
				false;
		}
	}

	if (psessionEntry == NULL) {
		lim_log(pMac, LOGE,
			FL
				("Error: Unable to find session id while in pre-auth phase for FT"));
		return eSIR_FAILURE;
	}

	if (psessionEntry->ftPEContext.pFTPreAuthReq == NULL) {
		lim_log(pMac, LOGE, FL("Error: No FT"));
		/* No FT in progress. */
		return eSIR_FAILURE;
	}

	if (frameLen == 0) {
		lim_log(pMac, LOGE, FL("Error: Frame len = 0"));
		return eSIR_FAILURE;
	}
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
	lim_print_mac_addr(pMac, pHdr->bssId, LOG2);
	lim_print_mac_addr(pMac,
			   psessionEntry->ftPEContext.pFTPreAuthReq->preAuthbssId,
			   LOG2);
	lim_log(pMac, LOG2, FL("seqControl 0x%X"),
		((pHdr->seqControl.seqNumHi << 8) | (pHdr->seqControl.
						     seqNumLo << 4) | (pHdr->
								       seqControl.
								       fragNum)));
#endif

	/* Check that its the same bssId we have for preAuth */
	if (!cdf_mem_compare
		    (psessionEntry->ftPEContext.pFTPreAuthReq->preAuthbssId,
		    pHdr->bssId, sizeof(tSirMacAddr))) {
		lim_log(pMac, LOGE, FL("Error: Same bssid as preauth BSSID"));
		/* In this case SME if indeed has triggered a */
		/* pre auth it will time out. */
		return eSIR_FAILURE;
	}

	if (true ==
	    psessionEntry->ftPEContext.pFTPreAuthReq->bPreAuthRspProcessed) {
		/*
		 * This is likely a duplicate for the same pre-auth request.
		 * PE/LIM already posted a response to SME. Hence, drop it.
		 * TBD:
		 * 1) How did we even receive multiple auth responses?
		 * 2) Do we need to delete pre-auth session? Suppose we
		 * previously received an auth resp with failure which
		 * would not have created the session and forwarded to SME.
		 * And, we subsequently received an auth resp with success
		 * which would have created the session. This will now be
		 * dropped without being forwarded to SME! However, it is
		 * very unlikely to receive auth responses from the same
		 * AP with different reason codes.
		 * NOTE: return eSIR_SUCCESS so that the packet is dropped
		 * as this was indeed a response from the BSSID we tried to
		 * pre-auth.
		 */
		PELOGE(lim_log(pMac, LOG1, "Auth rsp already posted to SME"
			       " (session %p, FT session %p)", psessionEntry,
			       psessionEntry);
		       );
		return eSIR_SUCCESS;
	} else {
		PELOGE(lim_log(pMac, LOGW, "Auth rsp not yet posted to SME"
			       " (session %p, FT session %p)", psessionEntry,
			       psessionEntry);
		       );
		psessionEntry->ftPEContext.pFTPreAuthReq->bPreAuthRspProcessed =
			true;
	}

#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
	lim_log(pMac, LOG1, FL("Pre-Auth response received from neighbor"));
	lim_log(pMac, LOG1, FL("Pre-Auth done state"));
#endif
	/* Stopping timer now, that we have our unicast from the AP */
	/* of our choice. */
	lim_deactivate_and_change_timer(pMac, eLIM_FT_PREAUTH_RSP_TIMER);

	/* Save off the auth resp. */
	if ((sir_convert_auth_frame2_struct(pMac, pBody, frameLen, &rxAuthFrame) !=
	     eSIR_SUCCESS)) {
		lim_log(pMac, LOGE,
			FL("failed to convert Auth frame to struct"));
		lim_handle_ft_pre_auth_rsp(pMac, eSIR_FAILURE, NULL, 0,
					   psessionEntry);
		return eSIR_FAILURE;
	}
	pRxAuthFrameBody = &rxAuthFrame;

#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
	PELOGE(lim_log(pMac, LOG1,
		       FL
			       ("Received Auth frame with type=%d seqnum=%d, status=%d (%d)"),
		       (uint32_t) pRxAuthFrameBody->authAlgoNumber,
		       (uint32_t) pRxAuthFrameBody->authTransactionSeqNumber,
		       (uint32_t) pRxAuthFrameBody->authStatusCode,
		       (uint32_t) pMac->lim.gLimNumPreAuthContexts);
	       )
#endif
	switch (pRxAuthFrameBody->authTransactionSeqNumber) {
	case SIR_MAC_AUTH_FRAME_2:
		if (pRxAuthFrameBody->authStatusCode != eSIR_MAC_SUCCESS_STATUS) {
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
			PELOGE(lim_log
				       (pMac, LOGE, "Auth status code received is %d",
				       (uint32_t) pRxAuthFrameBody->authStatusCode);
			       );
#endif
			if (eSIR_MAC_MAX_ASSOC_STA_REACHED_STATUS ==
			    pRxAuthFrameBody->authStatusCode)
				ret_status = eSIR_LIM_MAX_STA_REACHED_ERROR;
		} else {
			ret_status = eSIR_SUCCESS;
		}
		break;

	default:
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
		PELOGE(lim_log
			       (pMac, LOGE, "Seq. no incorrect expected 2 received %d",
			       (uint32_t) pRxAuthFrameBody->authTransactionSeqNumber);
		       )
#endif
		break;
	}

	/* Send the Auth response to SME */
	lim_handle_ft_pre_auth_rsp(pMac, ret_status, pBody, frameLen, psessionEntry);

	return ret_status;
}

#endif /* WLAN_FEATURE_VOWIFI_11R */
