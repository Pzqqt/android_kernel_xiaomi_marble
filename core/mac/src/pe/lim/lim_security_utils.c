/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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
#include "cfg_api.h"

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
lim_is_auth_algo_supported(tpAniSirGlobal pMac, tAniAuthType authType,
			   tpPESession psessionEntry)
{
	uint32_t algoEnable, privacyOptImp;

	if (authType == eSIR_OPEN_SYSTEM) {

		if (LIM_IS_AP_ROLE(psessionEntry)) {
			if ((psessionEntry->authType == eSIR_OPEN_SYSTEM)
			    || (psessionEntry->authType == eSIR_AUTO_SWITCH))
				return true;
			else
				return false;
		}

		if (wlan_cfg_get_int(pMac, WNI_CFG_OPEN_SYSTEM_AUTH_ENABLE,
				     &algoEnable) != eSIR_SUCCESS) {
			/**
			 * Could not get AuthAlgo1 Enable value
			 * from CFG. Log error.
			 */
			lim_log(pMac, LOGE,
				FL("could not retrieve AuthAlgo1 Enable value"));

			return false;
		} else
			return algoEnable > 0 ? true : false;
	} else {

		if (LIM_IS_AP_ROLE(psessionEntry)) {
			if ((psessionEntry->authType == eSIR_SHARED_KEY)
			    || (psessionEntry->authType == eSIR_AUTO_SWITCH))
				algoEnable = true;
			else
				algoEnable = false;

		} else

		if (wlan_cfg_get_int
			    (pMac, WNI_CFG_SHARED_KEY_AUTH_ENABLE,
			    &algoEnable) != eSIR_SUCCESS) {
			/**
			 * Could not get AuthAlgo2 Enable value
			 * from CFG. Log error.
			 */
			lim_log(pMac, LOGE,
				FL("could not retrieve AuthAlgo2 Enable value"));

			return false;
		}

		if (LIM_IS_AP_ROLE(psessionEntry)) {
			privacyOptImp = psessionEntry->privacy;
		} else

		if (wlan_cfg_get_int(pMac, WNI_CFG_PRIVACY_ENABLED,
				     &privacyOptImp) != eSIR_SUCCESS) {
			/**
			 * Could not get PrivacyOptionImplemented value
			 * from CFG. Log error.
			 */
			lim_log(pMac, LOGE,
				FL
					("could not retrieve PrivacyOptImplemented value"));

			return false;
		}
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
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void lim_init_pre_auth_list(tpAniSirGlobal pMac)
{
	pMac->lim.pLimPreAuthList = NULL;

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
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void lim_delete_pre_auth_list(tpAniSirGlobal pMac)
{
	struct tLimPreAuthNode *pCurrNode, *pTempNode;

	pCurrNode = pTempNode = pMac->lim.pLimPreAuthList;
	while (pCurrNode != NULL) {
		pTempNode = pCurrNode->next;

		PELOG1(lim_log(pMac, LOG1, FL("=====> lim_delete_pre_auth_list "));)
		lim_release_pre_auth_node(pMac, pCurrNode);

		pCurrNode = pTempNode;
	}
	pMac->lim.pLimPreAuthList = NULL;
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

struct tLimPreAuthNode *lim_search_pre_auth_list(tpAniSirGlobal pMac,
						 tSirMacAddr macAddr)
{
	struct tLimPreAuthNode *pTempNode = pMac->lim.pLimPreAuthList;

	while (pTempNode != NULL) {
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
lim_delete_open_auth_pre_auth_node(tpAniSirGlobal mac_ctx)
{
	struct tLimPreAuthNode    *prev_node, *temp_node, *found_node;
	uint8_t auth_node_freed = false;

	temp_node = prev_node = mac_ctx->lim.pLimPreAuthList;

	if (temp_node == NULL)
		return auth_node_freed;

	while (temp_node != NULL) {
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
 * @param  pMac - Pointer to Global MAC structure
 * @param  pAuthNode - Pointer to pre-auth node to be added to the list.
 *
 * @return None
 */

void lim_add_pre_auth_node(tpAniSirGlobal pMac, struct tLimPreAuthNode *pAuthNode)
{
	pMac->lim.gLimNumPreAuthContexts++;

	pAuthNode->next = pMac->lim.pLimPreAuthList;

	pMac->lim.pLimPreAuthList = pAuthNode;
} /*** end lim_add_pre_auth_node() ***/

/**
 * lim_release_pre_auth_node
 *
 ***FUNCTION:
 * This function is called to realease the accquired
 * pre auth node from list.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  pAuthNode - Pointer to Pre Auth node to be released
 * @return None
 */

void lim_release_pre_auth_node(tpAniSirGlobal pMac, tpLimPreAuthNode pAuthNode)
{
	pAuthNode->fFree = 1;
	MTRACE(mac_trace
		       (pMac, TRACE_CODE_TIMER_DEACTIVATE, NO_SESSION,
		       eLIM_PRE_AUTH_CLEANUP_TIMER));
	tx_timer_deactivate(&pAuthNode->timer);
	pMac->lim.gLimNumPreAuthContexts--;
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
 * @param  pMac - Pointer to Global MAC structure
 * @param  peerMacAddr - MAC address of the STA that need to be deleted
 *                       from pre-auth node list.
 *
 * @return None
 */

void lim_delete_pre_auth_node(tpAniSirGlobal pMac, tSirMacAddr macAddr)
{
	struct tLimPreAuthNode *pPrevNode, *pTempNode;

	pTempNode = pPrevNode = pMac->lim.pLimPreAuthList;

	if (pTempNode == NULL)
		return;

	if (!qdf_mem_cmp((uint8_t *) macAddr,
			    (uint8_t *) &pTempNode->peerMacAddr,
			    sizeof(tSirMacAddr))) {
		/* First node to be deleted */

		pMac->lim.pLimPreAuthList = pTempNode->next;

		PELOG1(lim_log
			       (pMac, LOG1,
			       FL
				       ("=====> lim_delete_pre_auth_node : first node to delete"));
		       )
		PELOG1(lim_log
			       (pMac, LOG1,
			       FL("Release data entry: %x id %d peer "), pTempNode,
			       pTempNode->authNodeIdx);
		       lim_print_mac_addr(pMac, macAddr, LOG1);
		       )
		lim_release_pre_auth_node(pMac, pTempNode);

		return;
	}

	pTempNode = pTempNode->next;

	while (pTempNode != NULL) {
		if (!qdf_mem_cmp((uint8_t *) macAddr,
				    (uint8_t *) &pTempNode->peerMacAddr,
				    sizeof(tSirMacAddr))) {
			/* Found node to be deleted */

			pPrevNode->next = pTempNode->next;

			PELOG1(lim_log
				       (pMac, LOG1,
				       FL
					       ("=====> lim_delete_pre_auth_node : subsequent node to delete"));
			       lim_log(pMac, LOG1,
				       FL("Release data entry: %x id %d peer "),
				       pTempNode, pTempNode->authNodeIdx);
			       lim_print_mac_addr(pMac, macAddr, LOG1);
			       )
			lim_release_pre_auth_node(pMac, pTempNode);

			return;
		}

		pPrevNode = pTempNode;
		pTempNode = pTempNode->next;
	}

	/* Should not be here */
	/* Log error */
	lim_log(pMac, LOGP, FL("peer not found in pre-auth list, addr= "));
	lim_print_mac_addr(pMac, macAddr, LOGP);

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
 * @param  pMac       - Pointer to Global MAC structure
 * @param  resultCode - result of authentication attempt
 * @return None
 */

void
lim_restore_from_auth_state(tpAniSirGlobal pMac, tSirResultCodes resultCode,
			    uint16_t protStatusCode, tpPESession sessionEntry)
{
	tSirMacAddr currentBssId;
	tLimMlmAuthCnf mlmAuthCnf;

#ifdef FEATURE_WLAN_DIAG_SUPPORT
	lim_diag_event_report(pMac, WLAN_PE_DIAG_AUTH_COMP_EVENT, sessionEntry,
			      resultCode, protStatusCode);
#endif

	qdf_mem_copy((uint8_t *) &mlmAuthCnf.peerMacAddr,
		     (uint8_t *) &pMac->lim.gpLimMlmAuthReq->peerMacAddr,
		     sizeof(tSirMacAddr));
	mlmAuthCnf.authType = pMac->lim.gpLimMlmAuthReq->authType;
	mlmAuthCnf.resultCode = resultCode;
	mlmAuthCnf.protStatusCode = protStatusCode;

	/* Update PE session ID */
	mlmAuthCnf.sessionId = sessionEntry->peSessionId;

	/* / Free up buffer allocated */
	/* / for pMac->lim.gLimMlmAuthReq */
	qdf_mem_free(pMac->lim.gpLimMlmAuthReq);
	pMac->lim.gpLimMlmAuthReq = NULL;

	sessionEntry->limMlmState = sessionEntry->limPrevMlmState;

	MTRACE(mac_trace
		       (pMac, TRACE_CODE_MLM_STATE, sessionEntry->peSessionId,
		       sessionEntry->limMlmState));

	/*
	 * Set the auth_ack_status status flag as success as
	 * host have received the auth rsp and no longer auth
	 * retry is needed also cancel the auth rety timer
	 */
	pMac->auth_ack_status = LIM_AUTH_ACK_RCD_SUCCESS;
	/* 'Change' timer for future activations */
	lim_deactivate_and_change_timer(pMac, eLIM_AUTH_RETRY_TIMER);

	/* 'Change' timer for future activations */
	lim_deactivate_and_change_timer(pMac, eLIM_AUTH_FAIL_TIMER);

	sir_copy_mac_addr(currentBssId, sessionEntry->bssId);

	if (sessionEntry->limSmeState == eLIM_SME_WT_PRE_AUTH_STATE) {
		pMac->lim.gLimPreAuthChannelNumber = 0;
	}

	lim_post_sme_message(pMac, LIM_MLM_AUTH_CNF, (uint32_t *) &mlmAuthCnf);
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
 * @param  pMac           Pointer to Global MAC structure
 * @param  keyId          key id to used
 * @param  pKey           Pointer to the key to be used for encryption
 * @param  pPlainText     Pointer to the body to be encrypted
 * @param  pEncrBody      Pointer to the encrypted auth frame body
 * @param  keyLength      8 (WEP40) or 16 (WEP104)
 * @return None
 */

void
lim_encrypt_auth_frame(tpAniSirGlobal pMac, uint8_t keyId, uint8_t *pKey,
		       uint8_t *pPlainText, uint8_t *pEncrBody,
		       uint32_t keyLength)
{
	uint8_t seed[LIM_SEED_LENGTH], icv[SIR_MAC_WEP_ICV_LENGTH];

	keyLength += 3;

	/* Bytes 3-7 of seed is key */
	qdf_mem_copy((uint8_t *) &seed[3], pKey, keyLength - 3);

	/* Compute CRC-32 and place them in last 4 bytes of plain text */
	lim_compute_crc32(icv, pPlainText, sizeof(tSirMacAuthFrameBody));

	qdf_mem_copy(pPlainText + sizeof(tSirMacAuthFrameBody),
		     icv, SIR_MAC_WEP_ICV_LENGTH);

	/* Run RC4 on plain text with the seed */
	lim_rc4(pEncrBody + SIR_MAC_WEP_IV_LENGTH,
		(uint8_t *) pPlainText, seed, keyLength,
		LIM_ENCR_AUTH_BODY_LEN - SIR_MAC_WEP_IV_LENGTH);

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

void lim_compute_crc32(uint8_t *pDest, uint8_t *pSrc, uint8_t len)
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
		uint8_t len = (uint8_t) frameLen;

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
 * @param pMac       Pointer to Global MAC structure
 * @param pKey       Pointer to the key to be used for decryption
 * @param pEncrBody  Pointer to the body to be decrypted
 * @param pPlainBody Pointer to the decrypted body
 * @param keyLength  8 (WEP40) or 16 (WEP104)
 *
 * @return Decrypt result - eSIR_SUCCESS for success and
 *                          LIM_DECRYPT_ICV_FAIL for ICV mismatch.
 *                          If decryption is a success, pBody will
 *                          have decrypted auth frame body.
 */

uint8_t
lim_decrypt_auth_frame(tpAniSirGlobal pMac, uint8_t *pKey, uint8_t *pEncrBody,
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

	PELOG4(lim_log(pMac, LOG4, FL("plainbody is "));
	       sir_dump_buf(pMac, SIR_LIM_MODULE_ID, LOG4, pPlainBody, frameLen);
	       )
	/* Compute CRC-32 and place them in last 4 bytes of encrypted body */
	lim_compute_crc32(icv,
			  (uint8_t *) pPlainBody,
			  (uint8_t) (frameLen - SIR_MAC_WEP_ICV_LENGTH));

	/* Compare RX_ICV with computed ICV */
	for (i = 0; i < SIR_MAC_WEP_ICV_LENGTH; i++) {
		PELOG4(lim_log
			       (pMac, LOG4, FL(" computed ICV%d[%x], rxed ICV%d[%x]"),
			       i, icv[i], i,
			       pPlainBody[frameLen - SIR_MAC_WEP_ICV_LENGTH + i]);
		       )
		if (icv[i] !=
		    pPlainBody[frameLen - SIR_MAC_WEP_ICV_LENGTH + i])
			return LIM_DECRYPT_ICV_FAIL;
	}

	return eSIR_SUCCESS;
} /****** end lim_decrypt_auth_frame() ******/

/**
 * lim_post_sme_set_keys_cnf
 *
 * A utility API to send MLM_SETKEYS_CNF to SME
 */
void lim_post_sme_set_keys_cnf(tpAniSirGlobal pMac,
			       tLimMlmSetKeysReq *pMlmSetKeysReq,
			       tLimMlmSetKeysCnf *mlmSetKeysCnf)
{
	/* Prepare and Send LIM_MLM_SETKEYS_CNF */
	qdf_copy_macaddr(&mlmSetKeysCnf->peer_macaddr,
			 &pMlmSetKeysReq->peer_macaddr);

	/* Free up buffer allocated for mlmSetKeysReq */
	qdf_mem_free(pMlmSetKeysReq);
	pMac->lim.gpLimMlmSetKeysReq = NULL;

	lim_post_sme_message(pMac,
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
 * @param pMac           Pointer to Global MAC structure
 * @param pMlmSetKeysReq Pointer to MLM_SETKEYS_REQ buffer
 * @return none
 */
void lim_send_set_bss_key_req(tpAniSirGlobal pMac,
			      tLimMlmSetKeysReq *pMlmSetKeysReq,
			      tpPESession psessionEntry)
{
	tSirMsgQ msgQ;
	tpSetBssKeyParams pSetBssKeyParams = NULL;
	tLimMlmSetKeysCnf mlmSetKeysCnf;
	tSirRetStatus retCode;
	uint32_t val = 0;

	if (pMlmSetKeysReq->numKeys > SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS) {
		lim_log(pMac, LOG1,
			FL
				("numKeys = %d is more than SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS"),
			pMlmSetKeysReq->numKeys);

		/* Respond to SME with error code */
		mlmSetKeysCnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		goto end;
	}
	/* Package WMA_SET_BSSKEY_REQ message parameters */

	pSetBssKeyParams = qdf_mem_malloc(sizeof(tSetBssKeyParams));
	if (NULL == pSetBssKeyParams) {
		lim_log(pMac, LOGE,
			FL("Unable to allocate memory during SET_BSSKEY"));

		/* Respond to SME with error code */
		mlmSetKeysCnf.resultCode = eSIR_SME_RESOURCES_UNAVAILABLE;
		goto end;
	}

	/* Update the WMA_SET_BSSKEY_REQ parameters */
	pSetBssKeyParams->bssIdx = psessionEntry->bssIdx;
	pSetBssKeyParams->encType = pMlmSetKeysReq->edType;

	if (eSIR_SUCCESS != wlan_cfg_get_int(pMac, WNI_CFG_SINGLE_TID_RC, &val)) {
		lim_log(pMac, LOGP, FL("Unable to read WNI_CFG_SINGLE_TID_RC"));
	}

	pSetBssKeyParams->singleTidRc = (uint8_t) val;

	/* Update PE session Id */
	pSetBssKeyParams->sessionId = psessionEntry->peSessionId;

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

	SET_LIM_PROCESS_DEFD_MESGS(pMac, false);
	msgQ.type = WMA_SET_BSSKEY_REQ;
	msgQ.reserved = 0;
	msgQ.bodyptr = pSetBssKeyParams;
	msgQ.bodyval = 0;

	lim_log(pMac, LOGW, FL("Sending WMA_SET_BSSKEY_REQ..."));
	MTRACE(mac_trace_msg_tx(pMac, psessionEntry->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		lim_log(pMac, LOGE,
			FL("Posting SET_BSSKEY to HAL failed, reason=%X"),
			retCode);

		/* Respond to SME with LIM_MLM_SETKEYS_CNF */
		mlmSetKeysCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
	} else
		return;         /* Continue after WMA_SET_BSSKEY_RSP... */

end:
	lim_post_sme_set_keys_cnf(pMac, pMlmSetKeysReq, &mlmSetKeysCnf);

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
 * @param pMac           Pointer to Global MAC structure
 * @param pMlmSetKeysReq Pointer to MLM_SETKEYS_REQ buffer
 * @param staIdx         STA index for which the keys are being set
 * @param defWEPIdx      The default WEP key index [0..3]
 * @return none
 */
void lim_send_set_sta_key_req(tpAniSirGlobal pMac,
			      tLimMlmSetKeysReq *pMlmSetKeysReq,
			      uint16_t staIdx,
			      uint8_t defWEPIdx,
			      tpPESession sessionEntry, bool sendRsp)
{
	tSirMsgQ msgQ;
	tpSetStaKeyParams pSetStaKeyParams = NULL;
	tLimMlmSetKeysCnf mlmSetKeysCnf;
	tSirRetStatus retCode;
	uint32_t val = 0;

	/* Package WMA_SET_STAKEY_REQ message parameters */
	pSetStaKeyParams = qdf_mem_malloc(sizeof(tSetStaKeyParams));
	if (NULL == pSetStaKeyParams) {
		lim_log(pMac, LOGP,
			FL("Unable to allocate memory during SET_BSSKEY"));
		return;
	}

	/* Update the WMA_SET_STAKEY_REQ parameters */
	pSetStaKeyParams->staIdx = staIdx;
	pSetStaKeyParams->encType = pMlmSetKeysReq->edType;

	if (eSIR_SUCCESS != wlan_cfg_get_int(pMac, WNI_CFG_SINGLE_TID_RC, &val)) {
		lim_log(pMac, LOGP, FL("Unable to read WNI_CFG_SINGLE_TID_RC"));
	}

	pSetStaKeyParams->singleTidRc = (uint8_t) val;

	/* Update  PE session ID */
	pSetStaKeyParams->sessionId = sessionEntry->peSessionId;

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
		sessionEntry->limPrevMlmState = sessionEntry->limMlmState;
		SET_LIM_PROCESS_DEFD_MESGS(pMac, false);
	}

	if (LIM_IS_IBSS_ROLE(sessionEntry)
	    && !pMlmSetKeysReq->key[0].unicast) {
		if (sendRsp == true)
			sessionEntry->limMlmState =
				eLIM_MLM_WT_SET_STA_BCASTKEY_STATE;
		msgQ.type = WMA_SET_STA_BCASTKEY_REQ;
	} else {
		if (sendRsp == true)
			sessionEntry->limMlmState =
				eLIM_MLM_WT_SET_STA_KEY_STATE;
		msgQ.type = WMA_SET_STAKEY_REQ;
	}
	MTRACE(mac_trace
		       (pMac, TRACE_CODE_MLM_STATE, sessionEntry->peSessionId,
		       sessionEntry->limMlmState));

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
			pSetStaKeyParams->wepType = eSIR_WEP_STATIC;
			sessionEntry->limMlmState =
				eLIM_MLM_WT_SET_STA_KEY_STATE;
			MTRACE(mac_trace
				       (pMac, TRACE_CODE_MLM_STATE,
				       sessionEntry->peSessionId,
				       sessionEntry->limMlmState));
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
				lim_log(pMac, LOGE, FL("Wrong Key Index %d"),
					defWEPIdx);
				qdf_mem_free(pSetStaKeyParams);
				return;
			}
		}
		break;
	case eSIR_ED_TKIP:
	case eSIR_ED_CCMP:
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

	lim_log(pMac, LOG1, FL("Sending WMA_SET_STAKEY_REQ..."));
	MTRACE(mac_trace_msg_tx(pMac, sessionEntry->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (eSIR_SUCCESS != retCode) {
		lim_log(pMac, LOGE,
			FL("Posting SET_STAKEY to HAL failed, reason=%X"),
			retCode);
		/* Respond to SME with LIM_MLM_SETKEYS_CNF */
		mlmSetKeysCnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
	} else
		return;         /* Continue after WMA_SET_STAKEY_RSP... */

	if (sendRsp == true)
		lim_post_sme_set_keys_cnf(pMac, pMlmSetKeysReq, &mlmSetKeysCnf);
}
