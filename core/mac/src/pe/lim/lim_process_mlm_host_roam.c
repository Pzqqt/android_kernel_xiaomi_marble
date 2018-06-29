/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: lim_process_mlm_host_roam.c
 *
 * Host based roaming MLM implementation
 */
#include "cds_api.h"
#include "wni_cfg.h"
#include "ani_global.h"
#include "sir_api.h"
#include "sir_params.h"
#include "cfg_api.h"

#include "sch_api.h"
#include "utils_api.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_prop_exts_utils.h"
#include "lim_security_utils.h"
#include "lim_send_messages.h"
#include "lim_send_messages.h"
#include "lim_session_utils.h"
#include <lim_ft.h>
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM
#include "host_diag_core_log.h"
#endif
#include "wma_if.h"
#include "rrm_api.h"
static void lim_handle_sme_reaasoc_result(tpAniSirGlobal, tSirResultCodes,
		uint16_t, tpPESession);
/**
 * lim_process_mlm_reassoc_req() - process mlm reassoc request.
 *
 * @mac_ctx:     pointer to Global MAC structure
 * @msg:  pointer to the MLM message buffer
 *
 * This function is called to process MLM_REASSOC_REQ message
 * from SME
 *
 * Return: None
 */
void lim_process_mlm_reassoc_req(tpAniSirGlobal mac_ctx, uint32_t *msg)
{
	uint8_t channel, sec_ch_offset;
	struct tLimPreAuthNode *auth_node;
	tLimMlmReassocReq *reassoc_req;
	tLimMlmReassocCnf reassoc_cnf;
	tpPESession session;

	if (msg == NULL) {
		pe_err("Buffer is Pointing to NULL");
		return;
	}

	reassoc_req = (tLimMlmReassocReq *) msg;
	session = pe_find_session_by_session_id(mac_ctx,
			reassoc_req->sessionId);
	if (NULL == session) {
		pe_err("Session Does not exist for given sessionId: %d",
			reassoc_req->sessionId);
		qdf_mem_free(reassoc_req);
		return;
	}

	pe_debug("ReAssoc Req on session: %d role: %d mlm: %d " MAC_ADDRESS_STR,
		reassoc_req->sessionId, GET_LIM_SYSTEM_ROLE(session),
		session->limMlmState, MAC_ADDR_ARRAY(reassoc_req->peerMacAddr));

	if (LIM_IS_AP_ROLE(session) ||
		(session->limMlmState !=
		eLIM_MLM_LINK_ESTABLISHED_STATE)) {
		/*
		 * Received Reassoc request in invalid state or
		 * in AP role.Return Reassoc confirm with Invalid
		 * parameters code.
		 */

		pe_warn("unexpect msg state: %X role: %d MAC" MAC_ADDRESS_STR,
			session->limMlmState, GET_LIM_SYSTEM_ROLE(session),
			MAC_ADDR_ARRAY(reassoc_req->peerMacAddr));
		lim_print_mlm_state(mac_ctx, LOGW, session->limMlmState);
		reassoc_cnf.resultCode = eSIR_SME_INVALID_PARAMETERS;
		reassoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
		goto end;
	}

	if (session->pLimMlmReassocReq)
		qdf_mem_free(session->pLimMlmReassocReq);

	/*
	 * Hold Re-Assoc request as part of Session, knock-out mac_ctx
	 * Hold onto Reassoc request parameters
	 */
	session->pLimMlmReassocReq = reassoc_req;

	/* See if we have pre-auth context with new AP */
	auth_node = lim_search_pre_auth_list(mac_ctx, session->limReAssocbssId);

	if (!auth_node && (qdf_mem_cmp(reassoc_req->peerMacAddr,
					    session->bssId,
					    sizeof(tSirMacAddr)))) {
		/*
		 * Either pre-auth context does not exist AND
		 * we are not reassociating with currently
		 * associated AP.
		 * Return Reassoc confirm with not authenticated
		 */
		reassoc_cnf.resultCode = eSIR_SME_STA_NOT_AUTHENTICATED;
		reassoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;

		goto end;
	}
	/* assign the sessionId to the timer object */
	mac_ctx->lim.limTimers.gLimReassocFailureTimer.sessionId =
		reassoc_req->sessionId;
	session->limPrevMlmState = session->limMlmState;
	session->limMlmState = eLIM_MLM_WT_REASSOC_RSP_STATE;
	MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE, session->peSessionId,
			 session->limMlmState));

	/* Derive channel from BSS description and store it at CFG. */
	channel = session->limReassocChannelId;
	sec_ch_offset = session->reAssocHtSecondaryChannelOffset;

	/* Apply previously set configuration at HW */
	lim_apply_configuration(mac_ctx, session);

	/* store the channel switch sessionEntry in the lim global var */
	session->channelChangeReasonCode =
		LIM_SWITCH_CHANNEL_REASSOC;

	/* Switch channel to the new Operating channel for Reassoc */
	lim_set_channel(mac_ctx, channel,
			session->ch_center_freq_seg0,
			session->ch_center_freq_seg1,
			session->ch_width,
			session->maxTxPower,
			session->peSessionId, 0, 0);

	return;
end:
	reassoc_cnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
	/* Update PE sessio Id */
	reassoc_cnf.sessionId = reassoc_req->sessionId;
	/* Free up buffer allocated for reassocReq */
	qdf_mem_free(reassoc_req);
	session->pLimReAssocReq = NULL;
	lim_post_sme_message(mac_ctx, LIM_MLM_REASSOC_CNF,
			     (uint32_t *) &reassoc_cnf);
}

/**
 * lim_handle_sme_reaasoc_result() - Handle the reassoc result
 * @pMac: Global MAC Context
 * @resultCode: Result code
 * @protStatusCode: Protocol Status Code
 * @psessionEntry: PE Session
 *
 * This function is called to process reassoc failures
 * upon receiving REASSOC_CNF with a failure code or
 * MLM_REASSOC_CNF with a success code in case of STA role
 *
 * Return: None
 */
static void lim_handle_sme_reaasoc_result(tpAniSirGlobal pMac,
		tSirResultCodes resultCode, uint16_t protStatusCode,
		tpPESession psessionEntry)
{
	tpDphHashNode pStaDs = NULL;
	uint8_t smesessionId;
	uint16_t smetransactionId;

	if (psessionEntry == NULL) {
		pe_err("psessionEntry is NULL");
		return;
	}
	smesessionId = psessionEntry->smeSessionId;
	smetransactionId = psessionEntry->transactionId;
	if (resultCode != eSIR_SME_SUCCESS) {
		pStaDs =
			dph_get_hash_entry(pMac, DPH_STA_HASH_INDEX_PEER,
					   &psessionEntry->dph.dphHashTable);
		if (pStaDs != NULL) {
			pStaDs->mlmStaContext.disassocReason =
				eSIR_MAC_UNSPEC_FAILURE_REASON;
			pStaDs->mlmStaContext.cleanupTrigger =
				eLIM_JOIN_FAILURE;
			pStaDs->mlmStaContext.resultCode = resultCode;
			pStaDs->mlmStaContext.protStatusCode = protStatusCode;
			lim_cleanup_rx_path(pMac, pStaDs, psessionEntry);
			/* Cleanup if add bss failed */
			if (psessionEntry->add_bss_failed) {
				dph_delete_hash_entry(pMac,
					 pStaDs->staAddr, pStaDs->assocId,
					 &psessionEntry->dph.dphHashTable);
				goto error;
			}
			return;
		}
	}
error:
	/* Delete the session if REASSOC failure occurred. */
	if (resultCode != eSIR_SME_SUCCESS) {
		if (NULL != psessionEntry) {
			pe_delete_session(pMac, psessionEntry);
			psessionEntry = NULL;
		}
	}
	lim_send_sme_join_reassoc_rsp(pMac, eWNI_SME_REASSOC_RSP, resultCode,
		protStatusCode, psessionEntry, smesessionId, smetransactionId);
}

/**
 * lim_process_mlm_reassoc_cnf() - process mlm reassoc cnf msg
 *
 * @mac_ctx:       Pointer to Global MAC structure
 * @msg_buf:       A pointer to the MLM message buffer
 *
 * This function is called to process MLM_REASSOC_CNF message from MLM State
 * machine.
 *
 * @Return: void
 */
void lim_process_mlm_reassoc_cnf(tpAniSirGlobal mac_ctx, uint32_t *msg_buf)
{
	tpPESession session;
	tLimMlmReassocCnf *lim_mlm_reassoc_cnf;

	if (msg_buf == NULL) {
		pe_err("Buffer is Pointing to NULL");
		return;
	}
	lim_mlm_reassoc_cnf = (tLimMlmReassocCnf *) msg_buf;
	session = pe_find_session_by_session_id(mac_ctx,
				lim_mlm_reassoc_cnf->sessionId);
	if (session == NULL) {
		pe_err("session Does not exist for given session Id");
		return;
	}
	if ((session->limSmeState != eLIM_SME_WT_REASSOC_STATE) ||
	    LIM_IS_AP_ROLE(session)) {
		/*
		 * Should not have received Reassocication confirm
		 * from MLM in other states OR on AP.
		 */
		pe_err("Rcv unexpected MLM_REASSOC_CNF role: %d sme 0x%X",
			GET_LIM_SYSTEM_ROLE(session), session->limSmeState);
		return;
	}

	/*
	 * Upon Reassoc success or failure, freeup the cached preauth request,
	 * to ensure that channel switch is now allowed following any change in
	 * HT params.
	 */
	if (session->ftPEContext.pFTPreAuthReq) {
		pe_debug("Freeing pFTPreAuthReq: %pK",
			session->ftPEContext.pFTPreAuthReq);
		if (session->ftPEContext.pFTPreAuthReq->pbssDescription) {
			qdf_mem_free(
			  session->ftPEContext.pFTPreAuthReq->pbssDescription);
			session->ftPEContext.pFTPreAuthReq->pbssDescription =
									NULL;
		}
		qdf_mem_free(session->ftPEContext.pFTPreAuthReq);
		session->ftPEContext.pFTPreAuthReq = NULL;
		session->ftPEContext.ftPreAuthSession = false;
	}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (session->bRoamSynchInProgress) {
		pe_debug("LFR3:Re-set the LIM Ctxt Roam Synch In Progress");
		session->bRoamSynchInProgress = false;
	}
#endif

	pe_debug("Rcv MLM_REASSOC_CNF with result code: %d",
		lim_mlm_reassoc_cnf->resultCode);
	if (lim_mlm_reassoc_cnf->resultCode == eSIR_SME_SUCCESS) {
		/* Successful Reassociation */
		pe_debug("*** Reassociated with new BSS ***");

		session->limSmeState = eLIM_SME_LINK_EST_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_SME_STATE,
		      session->peSessionId, session->limSmeState));

		/* Need to send Reassoc rsp with Reassoc success to Host. */
		lim_send_sme_join_reassoc_rsp(mac_ctx, eWNI_SME_REASSOC_RSP,
					lim_mlm_reassoc_cnf->resultCode,
					lim_mlm_reassoc_cnf->protStatusCode,
					session, session->smeSessionId,
					session->transactionId);
	} else if (lim_mlm_reassoc_cnf->resultCode
			== eSIR_SME_REASSOC_REFUSED) {
		/*
		 * Reassociation failure With the New AP but we still have the
		 * link with the Older AP
		 */
		session->limSmeState = eLIM_SME_LINK_EST_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_SME_STATE,
		       session->peSessionId, session->limSmeState));

		/* Need to send Reassoc rsp with Assoc failure to Host. */
		lim_send_sme_join_reassoc_rsp(mac_ctx, eWNI_SME_REASSOC_RSP,
					lim_mlm_reassoc_cnf->resultCode,
					lim_mlm_reassoc_cnf->protStatusCode,
					session, session->smeSessionId,
					session->transactionId);
	} else {
		/* Reassociation failure */
		session->limSmeState = eLIM_SME_JOIN_FAILURE_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_SME_STATE,
					   session->peSessionId, session->limSmeState));
		/* Need to send Reassoc rsp with Assoc failure to Host. */
		lim_handle_sme_reaasoc_result(mac_ctx,
					lim_mlm_reassoc_cnf->resultCode,
					lim_mlm_reassoc_cnf->protStatusCode,
					session);
	}

	if (session->pLimReAssocReq) {
		qdf_mem_free(session->pLimReAssocReq);
		session->pLimReAssocReq = NULL;
	}
}

/**
 * lim_process_sta_mlm_add_bss_rsp_ft() - Handle the ADD BSS response
 * @pMac: Global MAC context
 * @limMsgQ: ADD BSS Parameters
 * @psessionEntry: PE Session
 *
 * Function to handle WMA_ADD_BSS_RSP, in FT reassoc state.
 * Send ReAssociation Request.
 *
 *Return: None
 */
void lim_process_sta_mlm_add_bss_rsp_ft(tpAniSirGlobal pMac,
		struct scheduler_msg *limMsgQ, tpPESession psessionEntry)
{
	tLimMlmReassocCnf mlmReassocCnf; /* keep sme */
	tpDphHashNode pStaDs = NULL;
	tpAddStaParams pAddStaParams = NULL;
	uint32_t listenInterval = WNI_CFG_LISTEN_INTERVAL_STADEF;
	tpAddBssParams pAddBssParams = (tpAddBssParams) limMsgQ->bodyptr;
	uint32_t selfStaDot11Mode = 0;

	/* Sanity Checks */

	if (pAddBssParams == NULL) {
		pe_err("Invalid parameters");
		goto end;
	}
	if (eLIM_MLM_WT_ADD_BSS_RSP_FT_REASSOC_STATE !=
	    psessionEntry->limMlmState) {
		goto end;
	}

	pStaDs = dph_add_hash_entry(pMac, pAddBssParams->bssId,
					DPH_STA_HASH_INDEX_PEER,
					&psessionEntry->dph.dphHashTable);
	if (pStaDs == NULL) {
		/* Could not add hash table entry */
		pe_err("could not add hash entry at DPH for");
		lim_print_mac_addr(pMac, pAddBssParams->staContext.staMac,
				   LOGE);
		goto end;
	}
	/* Prepare and send Reassociation request frame */
	/* start reassoc timer. */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (psessionEntry->bRoamSynchInProgress != true) {
#endif
		pMac->lim.limTimers.gLimReassocFailureTimer.sessionId =
			psessionEntry->peSessionId;
		/* / Start reassociation failure timer */
		MTRACE(mac_trace
			(pMac, TRACE_CODE_TIMER_ACTIVATE,
			 psessionEntry->peSessionId, eLIM_REASSOC_FAIL_TIMER));
		if (tx_timer_activate
			(&pMac->lim.limTimers.gLimReassocFailureTimer)
			!= TX_SUCCESS) {
			/* / Could not start reassoc failure timer. */
			/* Log error */
			pe_err("could not start Reassoc failure timer");
			/* Return Reassoc confirm with */
			/* Resources Unavailable */
			mlmReassocCnf.resultCode =
				eSIR_SME_RESOURCES_UNAVAILABLE;
			mlmReassocCnf.protStatusCode =
				eSIR_MAC_UNSPEC_FAILURE_STATUS;
			goto end;
		}
		pMac->lim.pSessionEntry = psessionEntry;
		if (NULL == pMac->lim.pSessionEntry->pLimMlmReassocRetryReq) {
			/* Take a copy of reassoc request for retrying */
			pMac->lim.pSessionEntry->pLimMlmReassocRetryReq =
				qdf_mem_malloc(sizeof(tLimMlmReassocReq));
			if (NULL ==
				pMac->lim.pSessionEntry->pLimMlmReassocRetryReq)
				goto end;
			qdf_mem_copy(pMac->lim.pSessionEntry->
					pLimMlmReassocRetryReq,
					psessionEntry->pLimMlmReassocReq,
					sizeof(tLimMlmReassocReq));
		}
		pMac->lim.reAssocRetryAttempt = 0;
		lim_send_reassoc_req_with_ft_ies_mgmt_frame(pMac,
				psessionEntry->
				pLimMlmReassocReq,
				psessionEntry);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	} else {
		pe_debug("LFR3:Do not activate timer and dont send the reassoc");
	}
#endif
	psessionEntry->limPrevMlmState = psessionEntry->limMlmState;
	psessionEntry->limMlmState = eLIM_MLM_WT_FT_REASSOC_RSP_STATE;
	MTRACE(mac_trace
		       (pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId,
		       eLIM_MLM_WT_FT_REASSOC_RSP_STATE));
	pe_debug("Set the mlm state: %d session: %d",
		       psessionEntry->limMlmState, psessionEntry->peSessionId);

	psessionEntry->bssIdx = (uint8_t) pAddBssParams->bssIdx;

	/* Success, handle below */
	pStaDs->bssId = pAddBssParams->bssIdx;
	/* STA Index(genr by HAL) for the BSS entry is stored here */
	pStaDs->staIndex = pAddBssParams->staContext.staIdx;

	rrm_cache_mgmt_tx_power(pMac, pAddBssParams->txMgmtPower,
			psessionEntry);

	pAddStaParams = qdf_mem_malloc(sizeof(tAddStaParams));
	if (NULL == pAddStaParams) {
		pe_err("Unable to allocate memory during ADD_STA");
		goto end;
	}

	/* / Add STA context at MAC HW (BMU, RHP & TFP) */
	qdf_mem_copy((uint8_t *) pAddStaParams->staMac,
		     (uint8_t *) psessionEntry->selfMacAddr,
		     sizeof(tSirMacAddr));

	qdf_mem_copy((uint8_t *) pAddStaParams->bssId,
		     psessionEntry->bssId, sizeof(tSirMacAddr));

	pAddStaParams->staType = STA_ENTRY_SELF;
	pAddStaParams->status = QDF_STATUS_SUCCESS;
	pAddStaParams->respReqd = 1;

	/* Update  PE session ID */
	pAddStaParams->sessionId = psessionEntry->peSessionId;
	pAddStaParams->smesessionId = psessionEntry->smeSessionId;

	/* This will indicate HAL to "allocate" a new STA index */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (psessionEntry->bRoamSynchInProgress != true)
#endif
		pAddStaParams->staIdx = STA_INVALID_IDX;
	pAddStaParams->updateSta = false;

	pAddStaParams->shortPreambleSupported =
		(uint8_t) psessionEntry->beaconParams.fShortPreamble;
	lim_populate_peer_rate_set(pMac, &pAddStaParams->supportedRates, NULL,
				   false, psessionEntry, NULL, NULL);

	if (psessionEntry->htCapability) {
		pAddStaParams->htCapable = psessionEntry->htCapability;
		pAddStaParams->vhtCapable = psessionEntry->vhtCapability;
		pAddStaParams->ch_width = psessionEntry->ch_width;
		pAddStaParams->greenFieldCapable =
			lim_get_ht_capability(pMac, eHT_GREENFIELD,
					      psessionEntry);
		pAddStaParams->mimoPS =
			lim_get_ht_capability(pMac, eHT_MIMO_POWER_SAVE,
					      psessionEntry);
		pAddStaParams->rifsMode =
			lim_get_ht_capability(pMac, eHT_RIFS_MODE,
					psessionEntry);
		pAddStaParams->lsigTxopProtection =
			lim_get_ht_capability(pMac, eHT_LSIG_TXOP_PROTECTION,
					      psessionEntry);
		pAddStaParams->maxAmpduDensity =
			lim_get_ht_capability(pMac, eHT_MPDU_DENSITY,
					psessionEntry);
		pAddStaParams->maxAmpduSize =
			lim_get_ht_capability(pMac, eHT_MAX_RX_AMPDU_FACTOR,
					      psessionEntry);
		pAddStaParams->maxAmsduSize =
			lim_get_ht_capability(pMac, eHT_MAX_AMSDU_LENGTH,
					      psessionEntry);
		pAddStaParams->max_amsdu_num =
			lim_get_ht_capability(pMac, eHT_MAX_AMSDU_NUM,
					      psessionEntry);
		pAddStaParams->fDsssCckMode40Mhz =
			lim_get_ht_capability(pMac, eHT_DSSS_CCK_MODE_40MHZ,
					      psessionEntry);
		pAddStaParams->fShortGI20Mhz =
			lim_get_ht_capability(pMac, eHT_SHORT_GI_20MHZ,
					psessionEntry);
		pAddStaParams->fShortGI40Mhz =
			lim_get_ht_capability(pMac, eHT_SHORT_GI_40MHZ,
					psessionEntry);
	}

	if (wlan_cfg_get_int(pMac, WNI_CFG_LISTEN_INTERVAL, &listenInterval) !=
	    QDF_STATUS_SUCCESS)
		pe_err("Couldn't get LISTEN_INTERVAL");
	pAddStaParams->listenInterval = (uint16_t) listenInterval;

	wlan_cfg_get_int(pMac, WNI_CFG_DOT11_MODE, &selfStaDot11Mode);
	pAddStaParams->encryptType = psessionEntry->encryptType;
	pAddStaParams->maxTxPower = psessionEntry->maxTxPower;

	/* Lets save this for when we receive the Reassoc Rsp */
	psessionEntry->ftPEContext.pAddStaReq = pAddStaParams;

	if (pAddBssParams != NULL) {
		qdf_mem_free(pAddBssParams);
		pAddBssParams = NULL;
		limMsgQ->bodyptr = NULL;
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (psessionEntry->bRoamSynchInProgress) {
		pe_debug("LFR3:Prep and save AddStaReq for post-assoc-rsp");
		lim_process_assoc_rsp_frame(pMac, pMac->roam.pReassocResp,
					    LIM_REASSOC, psessionEntry);
	}
#endif
	return;

end:
	/* Free up buffer allocated for reassocReq */
	if (psessionEntry != NULL)
		if (psessionEntry->pLimMlmReassocReq != NULL) {
			qdf_mem_free(psessionEntry->pLimMlmReassocReq);
			psessionEntry->pLimMlmReassocReq = NULL;
		}

	if (pAddBssParams != NULL) {
		qdf_mem_free(pAddBssParams);
		pAddBssParams = NULL;
		limMsgQ->bodyptr = NULL;
	}

	mlmReassocCnf.resultCode = eSIR_SME_FT_REASSOC_FAILURE;
	mlmReassocCnf.protStatusCode = eSIR_MAC_UNSPEC_FAILURE_STATUS;
	/* Update PE session Id */
	if (psessionEntry != NULL)
		mlmReassocCnf.sessionId = psessionEntry->peSessionId;
	else
		mlmReassocCnf.sessionId = 0;

	lim_post_sme_message(pMac, LIM_MLM_REASSOC_CNF,
			     (uint32_t *) &mlmReassocCnf);
}

/**
 * lim_process_mlm_ft_reassoc_req() - Handle the Reassoc request
 * @pMac: Global MAC context
 * @pMsgBuf: Buffer which holds the data
 * @psessionEntry: PE Session
 *
 *  This function handles the Reassoc Req from SME
 *
 *  Return: None
 */
void lim_process_mlm_ft_reassoc_req(tpAniSirGlobal pMac, uint32_t *pMsgBuf,
				    tpPESession psessionEntry)
{
	uint8_t smeSessionId = 0;
	uint16_t transactionId = 0;
	uint8_t chanNum = 0;
	tLimMlmReassocReq *pMlmReassocReq;
	uint16_t caps;
	uint32_t val;
	struct scheduler_msg msgQ = {0};
	QDF_STATUS retCode;
	uint32_t teleBcnEn = 0;

	chanNum = psessionEntry->currentOperChannel;
	lim_get_session_info(pMac, (uint8_t *) pMsgBuf, &smeSessionId,
			     &transactionId);
	psessionEntry->smeSessionId = smeSessionId;
	psessionEntry->transactionId = transactionId;

#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(pMac, WLAN_PE_DIAG_REASSOCIATING,
			psessionEntry, 0, 0);
#endif

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		pe_err("psessionEntry is not in STA mode");
		return;
	}

	if (NULL == psessionEntry->ftPEContext.pAddBssReq) {
		pe_err("pAddBssReq is NULL");
		return;
	}
	pMlmReassocReq = qdf_mem_malloc(sizeof(tLimMlmReassocReq));
	if (NULL == pMlmReassocReq) {
		pe_err("call to AllocateMemory failed for mlmReassocReq");
		return;
	}

	qdf_mem_copy(pMlmReassocReq->peerMacAddr,
		     psessionEntry->bssId, sizeof(tSirMacAddr));

	if (wlan_cfg_get_int(pMac, WNI_CFG_REASSOCIATION_FAILURE_TIMEOUT,
			(uint32_t *) &pMlmReassocReq->reassocFailureTimeout)
	    != QDF_STATUS_SUCCESS) {
		/**
		 * Could not get ReassocFailureTimeout value
		 * from CFG. Log error.
		 */
		pe_err("could not retrieve ReassocFailureTimeout value");
		qdf_mem_free(pMlmReassocReq);
		return;
	}

	if (cfg_get_capability_info(pMac, &caps, psessionEntry) !=
			QDF_STATUS_SUCCESS) {
		/**
		 * Could not get Capabilities value
		 * from CFG. Log error.
		 */
		pe_err("could not get Capabilities value");
		qdf_mem_free(pMlmReassocReq);
		return;
	}

	lim_update_caps_info_for_bss(pMac, &caps,
		psessionEntry->pLimReAssocReq->bssDescription.capabilityInfo);
	pe_debug("Capabilities info FT Reassoc: 0x%X", caps);

	pMlmReassocReq->capabilityInfo = caps;

	/* Update PE sessionId */
	pMlmReassocReq->sessionId = psessionEntry->peSessionId;

	/* If telescopic beaconing is enabled, set listen interval
	   to WNI_CFG_TELE_BCN_MAX_LI
	 */
	if (wlan_cfg_get_int(pMac, WNI_CFG_TELE_BCN_WAKEUP_EN, &teleBcnEn) !=
	    QDF_STATUS_SUCCESS) {
		pe_err("Couldn't get WNI_CFG_TELE_BCN_WAKEUP_EN");
		qdf_mem_free(pMlmReassocReq);
		return;
	}

	if (teleBcnEn) {
		if (wlan_cfg_get_int(pMac, WNI_CFG_TELE_BCN_MAX_LI, &val) !=
		    QDF_STATUS_SUCCESS) {
			/**
			 * Could not get ListenInterval value
			 * from CFG. Log error.
			 */
			pe_err("could not retrieve ListenInterval");
			qdf_mem_free(pMlmReassocReq);
			return;
		}
	} else {
		if (wlan_cfg_get_int(pMac, WNI_CFG_LISTEN_INTERVAL, &val) !=
		    QDF_STATUS_SUCCESS) {
			/**
			 * Could not get ListenInterval value
			 * from CFG. Log error.
			 */
			pe_err("could not retrieve ListenInterval");
			qdf_mem_free(pMlmReassocReq);
			return;
		}
	}
	if (lim_set_link_state
		    (pMac, eSIR_LINK_PREASSOC_STATE, psessionEntry->bssId,
		    psessionEntry->selfMacAddr, NULL, NULL) != QDF_STATUS_SUCCESS) {
		qdf_mem_free(pMlmReassocReq);
		return;
	}

	pMlmReassocReq->listenInterval = (uint16_t) val;
	psessionEntry->pLimMlmReassocReq = pMlmReassocReq;

	/* we need to defer the message until we get response back from HAL */
	SET_LIM_PROCESS_DEFD_MESGS(pMac, false);

	msgQ.type = SIR_HAL_ADD_BSS_REQ;
	msgQ.reserved = 0;
	msgQ.bodyptr = psessionEntry->ftPEContext.pAddBssReq;
	msgQ.bodyval = 0;

	pe_debug("Sending SIR_HAL_ADD_BSS_REQ");
	MTRACE(mac_trace_msg_tx(pMac, psessionEntry->peSessionId, msgQ.type));
	retCode = wma_post_ctrl_msg(pMac, &msgQ);
	if (QDF_STATUS_SUCCESS != retCode) {
		qdf_mem_free(psessionEntry->ftPEContext.pAddBssReq);
		pe_err("Posting ADD_BSS_REQ to HAL failed, reason: %X",
			retCode);
	}

	psessionEntry->ftPEContext.pAddBssReq = NULL;
	return;
}

