/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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
 * This file lim_utils.cc contains the utility functions
 * LIM uses.
 * Author:        Chandra Modumudi
 * Date:          02/13/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#include "ani_global.h"
#include "wni_api.h"

#include "sir_common.h"
#include "wni_cfg.h"

#include "utils_api.h"
#include "lim_utils.h"
#include "lim_security_utils.h"
#include "lim_ft_defs.h"
#include "lim_session.h"

#define LIM_SEED_LENGTH 16
/*
 * preauth node timeout value in interval of 10msec
 */
#define LIM_OPENAUTH_TIMEOUT 500

/**
 * lim_is_auth_algo_supported()
 *
 ***FUNCTION:
 * This function is called in various places within LIM code
 * to determine whether passed authentication algorithm is enabled
 * or not
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param authType Indicates MAC based authentication type
 *                 (eSIR_OPEN_SYSTEM or eSIR_SHARED_KEY)
 *                 If Shared Key authentication to be used,
 *                 'Privacy Option Implemented' flag is also
 *                 checked.
 *
 * @return true if passed authType is enabled else false
 */
uint8_t
lim_is_auth_algo_supported(struct mac_context *mac, tAniAuthType authType,
			   struct pe_session *pe_session)
{
	bool algoEnable, privacyOptImp;
	struct wlan_mlme_wep_cfg *wep_params = &mac->mlme_cfg->wep_params;

	if (authType == eSIR_OPEN_SYSTEM) {

		if (LIM_IS_AP_ROLE(pe_session)) {
			if ((pe_session->authType == eSIR_OPEN_SYSTEM)
			    || (pe_session->authType == eSIR_AUTO_SWITCH))
				return true;
			else
				return false;
		}

		algoEnable = wep_params->is_auth_open_system;
		return algoEnable > 0 ? true : false;

	} else {

		if (LIM_IS_AP_ROLE(pe_session)) {
			if ((pe_session->authType == eSIR_SHARED_KEY)
			    || (pe_session->authType == eSIR_AUTO_SWITCH))
				algoEnable = true;
			else
				algoEnable = false;

		} else {
			algoEnable = wep_params->is_shared_key_auth;
		}

		if (LIM_IS_AP_ROLE(pe_session))
			privacyOptImp = pe_session->privacy;
		else
			privacyOptImp = wep_params->is_privacy_enabled;

		return algoEnable && privacyOptImp;
	}
} /****** end lim_is_auth_algo_supported() ******/

/**
 * lim_init_pre_auth_list
 *
 ***FUNCTION:
 * This function is called while starting a BSS at AP
 * to initialize MAC authenticated STA list. This may also be called
 * while joining/starting an IBSS if MAC authentication is allowed
 * in IBSS mode.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  mac - Pointer to Global MAC structure
 * @return None
 */

void lim_init_pre_auth_list(struct mac_context *mac)
{
	mac->lim.pLimPreAuthList = NULL;

} /*** end lim_init_pre_auth_list() ***/

/**
 * lim_delete_pre_auth_list
 *
 ***FUNCTION:
 * This function is called cleanup Pre-auth list either on
 * AP or on STA when moving from one persona to other.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  mac - Pointer to Global MAC structure
 * @return None
 */

void lim_delete_pre_auth_list(struct mac_context *mac)
{
	struct tLimPreAuthNode *pCurrNode, *pTempNode;

	pCurrNode = pTempNode = mac->lim.pLimPreAuthList;
	while (pCurrNode) {
		pTempNode = pCurrNode->next;
		lim_release_pre_auth_node(mac, pCurrNode);

		pCurrNode = pTempNode;
	}
	mac->lim.pLimPreAuthList = NULL;
} /*** end lim_delete_pre_auth_list() ***/

/**
 * lim_search_pre_auth_list
 *
 ***FUNCTION:
 * This function is called when Authentication frame is received
 * by AP (or at a STA in IBSS supporting MAC based authentication)
 * to search if a STA is in the middle of MAC Authentication
 * transaction sequence.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  macAddr - MAC address of the STA that sent
 *                       Authentication frame.
 *
 * @return Pointer to pre-auth node if found, else NULL
 */

struct tLimPreAuthNode *lim_search_pre_auth_list(struct mac_context *mac,
						 tSirMacAddr macAddr)
{
	struct tLimPreAuthNode *pTempNode = mac->lim.pLimPreAuthList;

	while (pTempNode) {
		if (!qdf_mem_cmp((uint8_t *) macAddr,
				    (uint8_t *) &pTempNode->peerMacAddr,
				    sizeof(tSirMacAddr)))
			break;

		pTempNode = pTempNode->next;
	}

	return pTempNode;
} /*** end lim_search_pre_auth_list() ***/

/**
 * lim_delete_open_auth_pre_auth_node() - delete any stale preauth nodes
 * @mac_ctx: Pointer to Global MAC structure
 *
 * This function is called to delete any stale preauth nodes on
 * receiving authentication frame and existing preauth nodes
 * reached the maximum allowed limit.
 *
 * Return: return true if any preauthnode deleted else false
 */
uint8_t
lim_delete_open_auth_pre_auth_node(struct mac_context *mac_ctx)
{
	struct tLimPreAuthNode    *prev_node, *temp_node, *found_node;
	uint8_t auth_node_freed = false;

	temp_node = prev_node = mac_ctx->lim.pLimPreAuthList;

	if (!temp_node)
		return auth_node_freed;

	while (temp_node) {
		if (temp_node->mlmState == eLIM_MLM_AUTHENTICATED_STATE &&
		    temp_node->authType == eSIR_OPEN_SYSTEM &&
		    (qdf_mc_timer_get_system_ticks() >
		    (LIM_OPENAUTH_TIMEOUT + temp_node->timestamp) ||
		    qdf_mc_timer_get_system_ticks() < temp_node->timestamp)) {
			/* Found node to be deleted */
			auth_node_freed = true;
			found_node = temp_node;
			if (mac_ctx->lim.pLimPreAuthList == temp_node) {
				prev_node = mac_ctx->lim.pLimPreAuthList =
					temp_node = found_node->next;
			} else {
				prev_node->next = temp_node->next;
				temp_node = prev_node->next;
			}

			lim_release_pre_auth_node(mac_ctx, found_node);
		} else {
			prev_node = temp_node;
			temp_node = prev_node->next;
		}
	}

	return auth_node_freed;
}

/**
 * lim_add_pre_auth_node
 *
 ***FUNCTION:
 * This function is called at AP while sending Authentication
 * frame2.
 * This may also be called on a STA in IBSS if MAC authentication is
 * allowed in IBSS mode.
 *
 ***LOGIC:
 * Node is always added to the front of the list
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  mac - Pointer to Global MAC structure
 * @param  pAuthNode - Pointer to pre-auth node to be added to the list.
 *
 * @return None
 */

void lim_add_pre_auth_node(struct mac_context *mac, struct tLimPreAuthNode *pAuthNode)
{
	mac->lim.gLimNumPreAuthContexts++;

	pAuthNode->next = mac->lim.pLimPreAuthList;

	mac->lim.pLimPreAuthList = pAuthNode;
} /*** end lim_add_pre_auth_node() ***/

/**
 * lim_release_pre_auth_node
 *
 ***FUNCTION:
 * This function is called to realease the acquired
 * pre auth node from list.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  mac - Pointer to Global MAC structure
 * @param  pAuthNode - Pointer to Pre Auth node to be released
 * @return None
 */

void lim_release_pre_auth_node(struct mac_context *mac,
			       tpLimPreAuthNode pAuthNode)
{
	pAuthNode->fFree = 1;
	if (pAuthNode->authType == eSIR_AUTH_TYPE_SAE &&
	    pAuthNode->assoc_req.present) {
		tpSirAssocReq assoc =
			 (tpSirAssocReq)pAuthNode->assoc_req.assoc_req;

		if (assoc->assocReqFrameLength)
			qdf_mem_free(assoc->assocReqFrame);
		qdf_mem_free(assoc);
		pAuthNode->assoc_req.present = false;
	}
	MTRACE(mac_trace
		       (mac, TRACE_CODE_TIMER_DEACTIVATE, NO_SESSION,
		       eLIM_PRE_AUTH_CLEANUP_TIMER));
	tx_timer_deactivate(&pAuthNode->timer);
	mac->lim.gLimNumPreAuthContexts--;
} /*** end lim_release_pre_auth_node() ***/

/**
 * lim_delete_pre_auth_node
 *
 ***FUNCTION:
 * This function is called at AP when a pre-authenticated STA is
 * Associated/Reassociated or when AuthFrame4 is received after
 * Auth Response timeout.
 * This may also be called on a STA in IBSS if MAC authentication and
 * Association/Reassociation is allowed in IBSS mode.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  mac - Pointer to Global MAC structure
 * @param  peerMacAddr - MAC address of the STA that need to be deleted
 *                       from pre-auth node list.
 *
 * @return None
 */

void lim_delete_pre_auth_node(struct mac_context *mac, tSirMacAddr macAddr)
{
	struct tLimPreAuthNode *pPrevNode, *pTempNode;

	pTempNode = pPrevNode = mac->lim.pLimPreAuthList;

	if (!pTempNode)
		return;

	if (!qdf_mem_cmp((uint8_t *) macAddr,
			    (uint8_t *) &pTempNode->peerMacAddr,
			    sizeof(tSirMacAddr))) {
		/* First node to be deleted */

		mac->lim.pLimPreAuthList = pTempNode->next;

		pe_debug("first node to delete, Release data entry: %pK id %d peer",
			       pTempNode, pTempNode->authNodeIdx);
		lim_print_mac_addr(mac, macAddr, LOGD);
		lim_release_pre_auth_node(mac, pTempNode);

		return;
	}

	pTempNode = pTempNode->next;

	while (pTempNode) {
		if (!qdf_mem_cmp((uint8_t *) macAddr,
				    (uint8_t *) &pTempNode->peerMacAddr,
				    sizeof(tSirMacAddr))) {
			/* Found node to be deleted */

			pPrevNode->next = pTempNode->next;

			pe_debug("subsequent node to delete, Release data entry: %pK id %d peer",
				       pTempNode, pTempNode->authNodeIdx);
			       lim_print_mac_addr(mac, macAddr, LOG1);
			lim_release_pre_auth_node(mac, pTempNode);

			return;
		}

		pPrevNode = pTempNode;
		pTempNode = pTempNode->next;
	}

	pe_err("peer not found in pre-auth list, addr= ");
	lim_print_mac_addr(mac, macAddr, LOGE);

} /*** end lim_delete_pre_auth_node() ***/

/**
 * limRestoreFromPreAuthState
 *
 ***FUNCTION:
 * This function is called on STA whenever an Authentication
 * sequence is complete and state prior to auth need to be
 * restored.
 *
 ***LOGIC:
 * MLM_AUTH_CNF is prepared and sent to SME state machine.
 * In case of restoring from pre-auth:
 *     - Channel Id is programmed at LO/RF synthesizer
 *     - BSSID is programmed at RHP
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  mac       - Pointer to Global MAC structure
 * @param  resultCode - result of authentication attempt
 * @return None
 */

void
lim_restore_from_auth_state(struct mac_context *mac, tSirResultCodes resultCode,
			    uint16_t protStatusCode, struct pe_session *pe_session)
{
	tSirMacAddr currentBssId;
	tLimMlmAuthCnf mlmAuthCnf;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
	lim_diag_event_report(mac, WLAN_PE_DIAG_AUTH_COMP_EVENT, pe_session,
			      resultCode, protStatusCode);
#endif

	qdf_mem_copy((uint8_t *) &mlmAuthCnf.peerMacAddr,
		     (uint8_t *) &mac->lim.gpLimMlmAuthReq->peerMacAddr,
		     sizeof(tSirMacAddr));
	mlmAuthCnf.authType = mac->lim.gpLimMlmAuthReq->authType;
	mlmAuthCnf.resultCode = resultCode;
	mlmAuthCnf.protStatusCode = protStatusCode;

	/* Update PE session ID */
	mlmAuthCnf.sessionId = pe_session->peSessionId;

	/* / Free up buffer allocated */
	/* / for mac->lim.gLimMlmAuthReq */
	qdf_mem_free(mac->lim.gpLimMlmAuthReq);
	mac->lim.gpLimMlmAuthReq = NULL;

	pe_session->limMlmState = pe_session->limPrevMlmState;

	MTRACE(mac_trace
		       (mac, TRACE_CODE_MLM_STATE, pe_session->peSessionId,
		       pe_session->limMlmState));

	/*
	 * Set the auth_ack_status status flag as success as
	 * host have received the auth rsp and no longer auth
	 * retry is needed also cancel the auth rety timer
	 */
	mac->auth_ack_status = LIM_AUTH_ACK_RCD_SUCCESS;

	/* Auth retry and AUth failure timers are not started for SAE */
	/* 'Change' timer for future activations */
	if (tx_timer_running(&mac->lim.limTimers.
	    g_lim_periodic_auth_retry_timer))
		lim_deactivate_and_change_timer(mac,
				eLIM_AUTH_RETRY_TIMER);
	/* 'Change' timer for future activations */
	if (tx_timer_running(&mac->lim.limTimers.gLimAuthFailureTimer))
		lim_deactivate_and_change_timer(mac,
				eLIM_AUTH_FAIL_TIMER);

	sir_copy_mac_addr(currentBssId, pe_session->bssId);

	if (pe_session->limSmeState == eLIM_SME_WT_PRE_AUTH_STATE) {
		mac->lim.gLimPreAuthChannelNumber = 0;
	}

	lim_post_sme_message(mac, LIM_MLM_AUTH_CNF, (uint32_t *) &mlmAuthCnf);
} /*** end lim_restore_from_auth_state() ***/

/**
 * lim_encrypt_auth_frame()
 *
 ***FUNCTION:
 * This function is called in lim_process_auth_frame() function
 * to encrypt Authentication frame3 body.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  mac           Pointer to Global MAC structure
 * @param  keyId          key id to used
 * @param  pKey           Pointer to the key to be used for encryption
 * @param  pPlainText     Pointer to the body to be encrypted
 * @param  pEncrBody      Pointer to the encrypted auth frame body
 * @param  keyLength      8 (WEP40) or 16 (WEP104)
 * @return None
 */

void
lim_encrypt_auth_frame(struct mac_context *mac, uint8_t keyId, uint8_t *pKey,
		       uint8_t *pPlainText, uint8_t *pEncrBody,
		       uint32_t keyLength)
{
	uint8_t seed[LIM_SEED_LENGTH], icv[SIR_MAC_WEP_ICV_LENGTH];
	uint16_t frame_len;

	frame_len = ((tpSirMacAuthFrameBody)pPlainText)->length +
			SIR_MAC_AUTH_FRAME_INFO_LEN + SIR_MAC_CHALLENGE_ID_LEN;
	keyLength += 3;

	/* Bytes 3-7 of seed is key */
	qdf_mem_copy((uint8_t *) &seed[3], pKey, keyLength - 3);

	/* Compute CRC-32 and place them in last 4 bytes of plain text */
	lim_compute_crc32(icv, pPlainText, frame_len);

	qdf_mem_copy(pPlainText + frame_len,
		     icv, SIR_MAC_WEP_ICV_LENGTH);

	/* Run RC4 on plain text with the seed */
	lim_rc4(pEncrBody + SIR_MAC_WEP_IV_LENGTH,
		(uint8_t *) pPlainText, seed, keyLength,
		frame_len + SIR_MAC_WEP_ICV_LENGTH);

	/* Prepare IV */
	pEncrBody[0] = seed[0];
	pEncrBody[1] = seed[1];
	pEncrBody[2] = seed[2];
	pEncrBody[3] = keyId << 6;
} /****** end lim_encrypt_auth_frame() ******/

/**
 * lim_compute_crc32()
 *
 ***FUNCTION:
 * This function is called to compute CRC-32 on a given source.
 * Used while encrypting/decrypting Authentication frame 3.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  pDest    Destination location for computed CRC
 * @param  pSrc     Source location to be CRC computed
 * @param  len      Length over which CRC to be computed
 * @return None
 */

void lim_compute_crc32(uint8_t *pDest, uint8_t *pSrc, uint16_t len)
{
	uint32_t crc;
	int i;

	crc = 0;
	crc = ~crc;

	while (len-- > 0)
		crc = lim_crc_update(crc, *pSrc++);

	crc = ~crc;

	for (i = 0; i < SIR_MAC_WEP_IV_LENGTH; i++) {
		pDest[i] = (uint8_t) crc;
		crc >>= 8;
	}
} /****** end lim_compute_crc32() ******/

/**
 * lim_rc4()
 *
 ***FUNCTION:
 * This function is called to run RC4 algorithm. Called while
 * encrypting/decrypting Authentication frame 3.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  pDest          Destination location for encrypted text
 * @param  pSrc           Source location to be encrypted
 * @param  seed           Contains seed (IV + key) for PRNG
 * @param  keyLength      8 (WEP40) or 16 (WEP104)
 * @param  frameLen       Length of the frame
 *
 * @return None
 */

void
lim_rc4(uint8_t *pDest, uint8_t *pSrc, uint8_t *seed, uint32_t keyLength,
	uint16_t frameLen)
{
	typedef struct {
		uint8_t i, j;
		uint8_t sbox[256];
	} tRC4Context;

	tRC4Context ctx;

	{
		uint16_t i, j, k;

		/* */
		/* Initialize sbox using seed */
		/* */

		ctx.i = ctx.j = 0;
		for (i = 0; i < 256; i++)
			ctx.sbox[i] = (uint8_t) i;

		j = 0;
		k = 0;
		for (i = 0; i < 256; i++) {
			uint8_t temp;

			if (k < LIM_SEED_LENGTH)
				j = (uint8_t) (j + ctx.sbox[i] + seed[k]);
			temp = ctx.sbox[i];
			ctx.sbox[i] = ctx.sbox[j];
			ctx.sbox[j] = temp;

			if (++k >= keyLength)
				k = 0;
		}
	}

	{
		uint8_t i = ctx.i;
		uint8_t j = ctx.j;
		uint16_t len = frameLen;

		while (len-- > 0) {
			uint8_t temp1, temp2;

			i = (uint8_t) (i + 1);
			temp1 = ctx.sbox[i];
			j = (uint8_t) (j + temp1);

			ctx.sbox[i] = temp2 = ctx.sbox[j];
			ctx.sbox[j] = temp1;

			temp1 = (uint8_t) (temp1 + temp2);
			temp1 = ctx.sbox[temp1];
			temp2 = (uint8_t) (pSrc ? *pSrc++ : 0);

			*pDest++ = (uint8_t) (temp1 ^ temp2);
		}

		ctx.i = i;
		ctx.j = j;
	}
} /****** end lim_rc4() ******/

/**
 * lim_decrypt_auth_frame()
 *
 ***FUNCTION:
 * This function is called in lim_process_auth_frame() function
 * to decrypt received Authentication frame3 body.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param mac       Pointer to Global MAC structure
 * @param pKey       Pointer to the key to be used for decryption
 * @param pEncrBody  Pointer to the body to be decrypted
 * @param pPlainBody Pointer to the decrypted body
 * @param keyLength  8 (WEP40) or 16 (WEP104)
 *
 * @return Decrypt result - QDF_STATUS_SUCCESS for success and
 *                          LIM_DECRYPT_ICV_FAIL for ICV mismatch.
 *                          If decryption is a success, pBody will
 *                          have decrypted auth frame body.
 */

uint8_t
lim_decrypt_auth_frame(struct mac_context *mac, uint8_t *pKey, uint8_t *pEncrBody,
		       uint8_t *pPlainBody, uint32_t keyLength, uint16_t frameLen)
{
	uint8_t seed[LIM_SEED_LENGTH], icv[SIR_MAC_WEP_ICV_LENGTH];
	int i;

	keyLength += 3;

	/* Bytes 0-2 of seed is received IV */
	qdf_mem_copy((uint8_t *) seed, pEncrBody, SIR_MAC_WEP_IV_LENGTH - 1);

	/* Bytes 3-7 of seed is key */
	qdf_mem_copy((uint8_t *) &seed[3], pKey, keyLength - 3);

	/* Run RC4 on encrypted text with the seed */
	lim_rc4(pPlainBody,
		pEncrBody + SIR_MAC_WEP_IV_LENGTH, seed, keyLength, frameLen);

	/* Compute CRC-32 and place them in last 4 bytes of encrypted body */
	lim_compute_crc32(icv,
			  (uint8_t *) pPlainBody,
			  (frameLen - SIR_MAC_WEP_ICV_LENGTH));

	/* Compare RX_ICV with computed ICV */
	for (i = 0; i < SIR_MAC_WEP_ICV_LENGTH; i++) {
		pe_debug("computed ICV%d[%x], rxed ICV%d[%x]",
			i, icv[i], i,
			pPlainBody[frameLen - SIR_MAC_WEP_ICV_LENGTH + i]);

		if (icv[i] !=
		    pPlainBody[frameLen - SIR_MAC_WEP_ICV_LENGTH + i])
			return LIM_DECRYPT_ICV_FAIL;
	}

	return QDF_STATUS_SUCCESS;
} /****** end lim_decrypt_auth_frame() ******/

/**
 * lim_post_sme_set_keys_cnf
 *
 * A utility API to send MLM_SETKEYS_CNF to SME
 */
void lim_post_sme_set_keys_cnf(struct mac_context *mac,
			       tLimMlmSetKeysReq *pMlmSetKeysReq,
			       tLimMlmSetKeysCnf *mlmSetKeysCnf)
{
	/* Prepare and Send LIM_MLM_SETKEYS_CNF */
	qdf_copy_macaddr(&mlmSetKeysCnf->peer_macaddr,
			 &pMlmSetKeysReq->peer_macaddr);

	/* Free up buffer allocated for mlmSetKeysReq */
	qdf_mem_zero(pMlmSetKeysReq, sizeof(tLimMlmSetKeysReq));
	qdf_mem_free(pMlmSetKeysReq);
	mac->lim.gpLimMlmSetKeysReq = NULL;

	lim_post_sme_message(mac,
			     LIM_MLM_SETKEYS_CNF, (uint32_t *) mlmSetKeysCnf);
}

/**
 * lim_send_set_bss_key_req()
 *
 ***FUNCTION:
 * This function is called from lim_process_mlm_set_keys_req(),
 * when PE is trying to setup the Group Keys related
 * to a specified encryption type
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param mac           Pointer to Global MAC structure
 * @param pMlmSetKeysReq Pointer to MLM_SETKEYS_REQ buffer
 * @return none
 */
void lim_send_set_bss_key_req(struct mac_context *mac,
			      tLimMlmSetKeysReq *pMlmSetKeysReq,
			      struct pe_session *pe_session)
{
	struct scheduler_msg msgQ = {0};
	tpSetBssKeyParams pSetBssKeyParams = NULL;
	tLimMlmSetKeysCnf mlmSetKeysCnf;
	QDF_STATUS retCode;

	if (pMlmSetKeysReq->numKeys > SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS) {
		pe_debug("numKeys = %d is more than SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS",
			pMlmSetKeysReq->numKeys);

		/* Respond to SME with error code */
		mlmSetKeysCnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		goto end;
	}
	/* Package WMA_SET_BSSKEY_REQ message parameters */

	pSetBssKeyParams = qdf_mem_malloc(sizeof(tSetBssKeyParams));
	if (!pSetBssKeyParams) {
		/* Respond to SME with error code */
		mlmSetKeysCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
		goto end;
	}

	/* Update the WMA_SET_BSSKEY_REQ parameters */
	pSetBssKeyParams->bssIdx = pe_session->bssIdx;
	pSetBssKeyParams->encType = pMlmSetKeysReq->edType;

	pSetBssKeyParams->singleTidRc =
		(uint8_t)(mac->mlme_cfg->sta.single_tid);
	/* Update PE session Id */
	pSetBssKeyParams->sessionId = pe_session->peSessionId;

	pSetBssKeyParams->smesessionId = pMlmSetKeysReq->smesessionId;

	if (pMlmSetKeysReq->key[0].keyId &&
	    ((pMlmSetKeysReq->edType == eSIR_ED_WEP40) ||
	     (pMlmSetKeysReq->edType == eSIR_ED_WEP104))
	    ) {
		/* IF the key id is non-zero and encryption type is WEP, Send all the 4
		 * keys to HAL with filling the key at right index in pSetBssKeyParams->key. */
		pSetBssKeyParams->numKeys = SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS;
		qdf_mem_copy((uint8_t *) &pSetBssKeyParams->
			     key[pMlmSetKeysReq->key[0].keyId],
			     (uint8_t *) &pMlmSetKeysReq->key[0],
			     sizeof(pMlmSetKeysReq->key[0]));

	} else {
		pSetBssKeyParams->numKeys = pMlmSetKeysReq->numKeys;
		qdf_mem_copy((uint8_t *) &pSetBssKeyParams->key,
			     (uint8_t *) &pMlmSetKeysReq->key,
			     sizeof(tSirKeys) * pMlmSetKeysReq->numKeys);
	}

	SET_LIM_PROCESS_DEFD_MESGS(mac, false);
	msgQ.type = WMA_SET_BSSKEY_REQ;
	msgQ.reserved = 0;
	msgQ.bodyptr = pSetBssKeyParams;
	msgQ.bodyval = 0;

	pe_debug("Sending WMA_SET_BSSKEY_REQ...");
	MTRACE(mac_trace_msg_tx(mac, pe_session->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(mac, &msgQ);
	if (QDF_STATUS_SUCCESS != retCode) {
		pe_err("Posting SET_BSSKEY to HAL failed, reason=%X",
			retCode);

		/* Respond to SME with LIM_MLM_SETKEYS_CNF */
		mlmSetKeysCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
		qdf_mem_zero(pSetBssKeyParams, sizeof(tSetBssKeyParams));
		qdf_mem_free(pSetBssKeyParams);
	} else
		return;         /* Continue after WMA_SET_BSSKEY_RSP... */

end:
	lim_post_sme_set_keys_cnf(mac, pMlmSetKeysReq, &mlmSetKeysCnf);

}

/**
 * @function : lim_send_set_sta_key_req()
 *
 * @brief :  This function is called from lim_process_mlm_set_keys_req(),
 * when PE is trying to setup the Unicast Keys related
 * to a specified STA with specified encryption type
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param mac           Pointer to Global MAC structure
 * @param pMlmSetKeysReq Pointer to MLM_SETKEYS_REQ buffer
 * @param staIdx         STA index for which the keys are being set
 * @param defWEPIdx      The default WEP key index [0..3]
 * @return none
 */
void lim_send_set_sta_key_req(struct mac_context *mac,
			      tLimMlmSetKeysReq *pMlmSetKeysReq,
			      uint16_t staIdx,
			      uint8_t defWEPIdx,
			      struct pe_session *pe_session, bool sendRsp)
{
	struct scheduler_msg msgQ = {0};
	tpSetStaKeyParams pSetStaKeyParams = NULL;
	tLimMlmSetKeysCnf mlmSetKeysCnf;
	QDF_STATUS retCode;

	/* Package WMA_SET_STAKEY_REQ message parameters */
	pSetStaKeyParams = qdf_mem_malloc(sizeof(tSetStaKeyParams));
	if (!pSetStaKeyParams)
		goto fail;

	/* Update the WMA_SET_STAKEY_REQ parameters */
	pSetStaKeyParams->staIdx = staIdx;
	pSetStaKeyParams->encType = pMlmSetKeysReq->edType;

	pSetStaKeyParams->singleTidRc =
		(uint8_t)(mac->mlme_cfg->sta.single_tid);
	/* Update  PE session ID */
	pSetStaKeyParams->sessionId = pe_session->peSessionId;

	/**
	 * For WEP - defWEPIdx indicates the default WEP
	 * Key to be used for TX
	 * For all others, there's just one key that can
	 * be used and hence it is assumed that
	 * defWEPIdx = 0 (from the caller)
	 */

	pSetStaKeyParams->defWEPIdx = defWEPIdx;

	pSetStaKeyParams->smesessionId = pMlmSetKeysReq->smesessionId;
	qdf_copy_macaddr(&pSetStaKeyParams->peer_macaddr,
			 &pMlmSetKeysReq->peer_macaddr);

	if (sendRsp == true) {
		/** Store the Previous MlmState*/
		pe_session->limPrevMlmState = pe_session->limMlmState;
		SET_LIM_PROCESS_DEFD_MESGS(mac, false);
	}

	if (LIM_IS_IBSS_ROLE(pe_session)
	    && !pMlmSetKeysReq->key[0].unicast) {
		if (sendRsp == true)
			pe_session->limMlmState =
				eLIM_MLM_WT_SET_STA_BCASTKEY_STATE;
		msgQ.type = WMA_SET_STA_BCASTKEY_REQ;
	} else {
		if (sendRsp == true)
			pe_session->limMlmState =
				eLIM_MLM_WT_SET_STA_KEY_STATE;
		msgQ.type = WMA_SET_STAKEY_REQ;
	}
	MTRACE(mac_trace
		       (mac, TRACE_CODE_MLM_STATE, pe_session->peSessionId,
		       pe_session->limMlmState));

	/**
	 * In the Case of WEP_DYNAMIC, ED_TKIP and ED_CCMP
	 * the Key[0] contains the KEY, so just copy that alone,
	 * for the case of WEP_STATIC the hal gets the key from cfg
	 */
	switch (pMlmSetKeysReq->edType) {
	case eSIR_ED_WEP40:
	case eSIR_ED_WEP104:
		/* FIXME! Is this OK? */
		if (0 == pMlmSetKeysReq->numKeys) {
			uint32_t i;

			for (i = 0; i < SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS; i++) {
				qdf_mem_copy((uint8_t *) &pSetStaKeyParams->
					     key[i],
					     (uint8_t *) &pMlmSetKeysReq->
					     key[i], sizeof(tSirKeys));
			}
			pe_session->limMlmState =
				eLIM_MLM_WT_SET_STA_KEY_STATE;
			MTRACE(mac_trace
				       (mac, TRACE_CODE_MLM_STATE,
				       pe_session->peSessionId,
				       pe_session->limMlmState));
		} else {
			/*This case the keys are coming from upper layer so need to fill the
			 * key at the default wep key index and send to the HAL */
			if (defWEPIdx < SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS) {
				qdf_mem_copy((uint8_t *) &pSetStaKeyParams->
					     key[defWEPIdx],
					     (uint8_t *) &pMlmSetKeysReq->
					     key[0],
					     sizeof(pMlmSetKeysReq->key[0]));
				pMlmSetKeysReq->numKeys =
					SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS;
			} else {
				pe_err("Wrong Key Index %d", defWEPIdx);
				goto free_sta_key;
			}
		}
		break;
	case eSIR_ED_TKIP:
	case eSIR_ED_CCMP:
	case eSIR_ED_GCMP:
	case eSIR_ED_GCMP_256:
#ifdef FEATURE_WLAN_WAPI
	case eSIR_ED_WPI:
#endif
		{
			qdf_mem_copy((uint8_t *) &pSetStaKeyParams->key,
				     (uint8_t *) &pMlmSetKeysReq->key[0],
				     sizeof(tSirKeys));
		}
		break;
	default:
		break;
	}

	pSetStaKeyParams->sendRsp = sendRsp;

	msgQ.reserved = 0;
	msgQ.bodyptr = pSetStaKeyParams;
	msgQ.bodyval = 0;

	pe_debug("Sending WMA_SET_STAKEY_REQ...");
	MTRACE(mac_trace_msg_tx(mac, pe_session->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(mac, &msgQ);
	if (QDF_STATUS_SUCCESS != retCode) {
		pe_err("Posting SET_STAKEY to HAL failed, reason=%X",
			retCode);
		goto free_sta_key;
	} else
		return;         /* Continue after WMA_SET_STAKEY_RSP... */

free_sta_key:
	qdf_mem_zero(pSetStaKeyParams, sizeof(tSetStaKeyParams));
	qdf_mem_free(pSetStaKeyParams);
fail:
	/* Respond to SME with LIM_MLM_SETKEYS_CNF */
	mlmSetKeysCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
	if (sendRsp == true)
		lim_post_sme_set_keys_cnf(mac, pMlmSetKeysReq, &mlmSetKeysCnf);
}
