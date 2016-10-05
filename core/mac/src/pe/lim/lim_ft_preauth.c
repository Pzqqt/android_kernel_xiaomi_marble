/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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
 * DOC: lim_ft_preauth.c
 *
 * Pre-Authentication implementation for host based roaming
 */
#include <lim_send_messages.h>
#include <lim_types.h>
#include <lim_ft.h>
#include <lim_ft_defs.h>
#include <lim_utils.h>
#include <lim_prop_exts_utils.h>
#include <lim_assoc_utils.h>
#include <lim_session.h>
#include <lim_admit_control.h>
#include "wma.h"

/**
 * lim_ft_cleanup_pre_auth_info() - Cleanup preauth related information
 * @pMac: Global MAC Context
 * @psessionEntry: PE Session
 *
 * This routine is called to free the FT context, session and other
 * information used during preauth operation.
 *
 * Return: None
 */
void lim_ft_cleanup_pre_auth_info(tpAniSirGlobal pMac,
		tpPESession psessionEntry)
{
	tpPESession pReAssocSessionEntry = NULL;
	uint8_t sessionId = 0;

	if (!psessionEntry) {
		lim_log(pMac, LOGE, FL("psessionEntry is NULL"));
		return;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return;
	}

	if (psessionEntry->ftPEContext.pFTPreAuthReq) {
		pReAssocSessionEntry =
			pe_find_session_by_bssid(pMac,
						 psessionEntry->ftPEContext.
						 pFTPreAuthReq->preAuthbssId,
						 &sessionId);

		lim_log(pMac, LOG1, FL("Freeing pFTPreAuthReq= %p"),
			       psessionEntry->ftPEContext.pFTPreAuthReq);
		if (psessionEntry->ftPEContext.pFTPreAuthReq->
		    pbssDescription) {
			qdf_mem_free(psessionEntry->ftPEContext.pFTPreAuthReq->
				     pbssDescription);
			psessionEntry->ftPEContext.pFTPreAuthReq->
			pbssDescription = NULL;
		}
		qdf_mem_free(psessionEntry->ftPEContext.pFTPreAuthReq);
		psessionEntry->ftPEContext.pFTPreAuthReq = NULL;
	}

	if (psessionEntry->ftPEContext.pAddBssReq) {
		qdf_mem_free(psessionEntry->ftPEContext.pAddBssReq);
		psessionEntry->ftPEContext.pAddBssReq = NULL;
	}

	if (psessionEntry->ftPEContext.pAddStaReq) {
		qdf_mem_free(psessionEntry->ftPEContext.pAddStaReq);
		psessionEntry->ftPEContext.pAddStaReq = NULL;
	}

	/* The session is being deleted, cleanup the contents */
	qdf_mem_set(&psessionEntry->ftPEContext, sizeof(tftPEContext), 0);

	/* Delete the session created while handling pre-auth response */
	if (pReAssocSessionEntry) {
		/* If we have successful pre-auth response, then we would have
		 * created a session on which reassoc request will be sent
		 */
		if (pReAssocSessionEntry->valid &&
		    pReAssocSessionEntry->limSmeState ==
		    eLIM_SME_WT_REASSOC_STATE) {
			QDF_TRACE(QDF_MODULE_ID_PE,
				  QDF_TRACE_LEVEL_DEBUG,
				  FL("Deleting Preauth session(%d)"),
				  pReAssocSessionEntry->peSessionId);
			pe_delete_session(pMac, pReAssocSessionEntry);
		}
	}
}

/*
 * lim_process_ft_pre_auth_req() - process ft pre auth req
 *
 * @mac_ctx:    global mac ctx
 * @msg:        pointer to message
 *
 * In this function, we process the FT Pre Auth Req:
 *   We receive Pre-Auth, suspend link, register a call back. In the call back,
 *   we will need to accept frames from the new bssid. Send out the auth req to
 *   new AP. Start timer and when the timer is done or if we receive the Auth
 *   response. We change channel. Resume link
 *
 * Return: value to indicate if buffer was consumed
 */
int lim_process_ft_pre_auth_req(tpAniSirGlobal mac_ctx, tpSirMsgQ msg)
{
	int buf_consumed = false;
	tpPESession session;
	uint8_t session_id;
	tpSirFTPreAuthReq ft_pre_auth_req = (tSirFTPreAuthReq *) msg->bodyptr;

	if (NULL == ft_pre_auth_req) {
		lim_log(mac_ctx, LOGE, FL("tSirFTPreAuthReq is NULL"));
		return buf_consumed;
	}

	/* Get the current session entry */
	session = pe_find_session_by_bssid(mac_ctx,
					   ft_pre_auth_req->currbssId,
					   &session_id);
	if (session == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("Unable to find session for the bssid"
			   MAC_ADDRESS_STR),
			   MAC_ADDR_ARRAY(ft_pre_auth_req->currbssId));
		/* Post the FT Pre Auth Response to SME */
		lim_post_ft_pre_auth_rsp(mac_ctx, eSIR_FAILURE, NULL, 0,
					 session);
		/*
		 * return FALSE, since the Pre-Auth Req will be freed in
		 * limPostFTPreAuthRsp on failure
		 */
		return buf_consumed;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(session)) {
		lim_log(mac_ctx, LOGE, FL("session is not in STA mode"));
		buf_consumed = true;
		return buf_consumed;
	}

	/* Can set it only after sending auth */
	session->ftPEContext.ftPreAuthStatus = eSIR_FAILURE;
	session->ftPEContext.ftPreAuthSession = true;

	/* Indicate that this is the session on which preauth is being done */
	if (session->ftPEContext.pFTPreAuthReq) {
		if (session->ftPEContext.pFTPreAuthReq->pbssDescription) {
			qdf_mem_free(
			  session->ftPEContext.pFTPreAuthReq->pbssDescription);
			session->ftPEContext.pFTPreAuthReq->pbssDescription =
									NULL;
		}
		qdf_mem_free(session->ftPEContext.pFTPreAuthReq);
		session->ftPEContext.pFTPreAuthReq = NULL;
	}

	/* We need information from the Pre-Auth Req. Lets save that */
	session->ftPEContext.pFTPreAuthReq = ft_pre_auth_req;

	lim_log(mac_ctx, LOG1, FL("PRE Auth ft_ies_length=%02x%02x%02x"),
		session->ftPEContext.pFTPreAuthReq->ft_ies[0],
		session->ftPEContext.pFTPreAuthReq->ft_ies[1],
		session->ftPEContext.pFTPreAuthReq->ft_ies[2]);
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_PRE_AUTH_REQ_EVENT,
			      session, 0, 0);
#endif

	/* Dont need to suspend if APs are in same channel */
	if (session->currentOperChannel !=
	    session->ftPEContext.pFTPreAuthReq->preAuthchannelNum) {
		/* Need to suspend link only if the channels are different */
		lim_log(mac_ctx, LOG2,
			FL("Performing pre-auth on diff channel(session %p)"),
			session);
		lim_send_preauth_scan_offload(mac_ctx, session->peSessionId,
				session->ftPEContext.pFTPreAuthReq);
	} else {
		lim_log(mac_ctx, LOG2,
			FL("Performing pre-auth on same channel (session %p)"),
			session);
		/* We are in the same channel. Perform pre-auth */
		lim_perform_ft_pre_auth(mac_ctx, QDF_STATUS_SUCCESS, NULL,
					session);
	}

	return buf_consumed;
}

/**
 * lim_perform_ft_pre_auth() - Perform preauthentication
 * @pMac: Global MAC Context
 * @status: Status Code
 * @data: pre-auth data
 * @psessionEntry: PE Session
 *
 * This routine will trigger the sending of authentication frame
 * to the peer.
 *
 * Return: None
 */
void lim_perform_ft_pre_auth(tpAniSirGlobal pMac, QDF_STATUS status,
			     uint32_t *data, tpPESession psessionEntry)
{
	tSirMacAuthFrameBody authFrame;

	if (NULL == psessionEntry) {
		PELOGE(lim_log(pMac, LOGE, FL("psessionEntry is NULL"));)
		return;
	}

	if (psessionEntry->is11Rconnection &&
	    psessionEntry->ftPEContext.pFTPreAuthReq) {
		/* Only 11r assoc has FT IEs */
		if (psessionEntry->ftPEContext.pFTPreAuthReq->ft_ies == NULL) {
			lim_log(pMac, LOGE,
				FL("FTIEs for Auth Req Seq 1 is absent"));
			goto preauth_fail;
		}
	}

	if (status != QDF_STATUS_SUCCESS) {
		lim_log(pMac, LOGE,
			FL(" Change channel not successful for FT pre-auth"));
		goto preauth_fail;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return;
	}
	lim_log(pMac, LOG2, "Entered wait auth2 state for FT (old session %p)",
				 psessionEntry);
	if (psessionEntry->is11Rconnection) {
		/* Now we are on the right channel and need to send out Auth1
		 * and receive Auth2
		 */
		authFrame.authAlgoNumber = eSIR_FT_AUTH;
	} else {
		/* Will need to make isESEconnection a enum may be for further
		 * improvements to this to match this algorithm number
		 */
		authFrame.authAlgoNumber = eSIR_OPEN_SYSTEM;
	}
	authFrame.authTransactionSeqNumber = SIR_MAC_AUTH_FRAME_1;
	authFrame.authStatusCode = 0;

	pMac->lim.limTimers.g_lim_periodic_auth_retry_timer.sessionId =
				psessionEntry->peSessionId;

	/* Start timer here to come back to operating channel */
	pMac->lim.limTimers.gLimFTPreAuthRspTimer.sessionId =
		psessionEntry->peSessionId;
	if (TX_SUCCESS !=
	    tx_timer_activate(&pMac->lim.limTimers.gLimFTPreAuthRspTimer)) {
		lim_log(pMac, LOGE, FL("FT Auth Rsp Timer Start Failed"));
		goto preauth_fail;
	}
	MTRACE(mac_trace(pMac, TRACE_CODE_TIMER_ACTIVATE,
		psessionEntry->peSessionId, eLIM_FT_PREAUTH_RSP_TIMER));

	lim_log(pMac, LOG1, FL("FT Auth Rsp Timer Started"));
#ifdef FEATURE_WLAN_DIAG_SUPPORT
	lim_diag_event_report(pMac, WLAN_PE_DIAG_ROAM_AUTH_START_EVENT,
			pMac->lim.pSessionEntry, eSIR_SUCCESS, eSIR_SUCCESS);
#endif

	lim_send_auth_mgmt_frame(pMac, &authFrame,
		 psessionEntry->ftPEContext.pFTPreAuthReq->preAuthbssId,
		 LIM_NO_WEP_IN_FC, psessionEntry, false);

	return;

preauth_fail:
	lim_handle_ft_pre_auth_rsp(pMac, eSIR_FAILURE, NULL, 0, psessionEntry);
	return;
}

/**
 * lim_ft_setup_auth_session() - Fill the FT Session
 * @pMac: Global MAC Context
 * @psessionEntry: PE Session
 *
 * Setup the session and the add bss req for the pre-auth AP.
 *
 * Return: Success or Failure Status
 */
tSirRetStatus lim_ft_setup_auth_session(tpAniSirGlobal pMac,
					tpPESession psessionEntry)
{
	tpPESession pftSessionEntry = NULL;
	uint8_t sessionId = 0;

	pftSessionEntry =
		pe_find_session_by_bssid(pMac, psessionEntry->limReAssocbssId,
					 &sessionId);
	if (pftSessionEntry == NULL) {
		lim_log(pMac, LOGE, FL("No session found for bssid"));
		lim_print_mac_addr(pMac, psessionEntry->limReAssocbssId, LOGE);
		return eSIR_FAILURE;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return eSIR_FAILURE;
	}

	if (psessionEntry->ftPEContext.pFTPreAuthReq &&
	    psessionEntry->ftPEContext.pFTPreAuthReq->pbssDescription) {
		lim_fill_ft_session(pMac,
				    psessionEntry->ftPEContext.pFTPreAuthReq->
				    pbssDescription, pftSessionEntry,
				    psessionEntry);

		lim_ft_prepare_add_bss_req(pMac, false, pftSessionEntry,
		     psessionEntry->ftPEContext.pFTPreAuthReq->pbssDescription);
	}

	return eSIR_SUCCESS;
}

/**
 * lim_ft_process_pre_auth_result() - Process the Auth frame
 * @pMac: Global MAC context
 * @status: Status code
 * psessionEntry: PE Session
 *
 * Return: None
 */
void lim_ft_process_pre_auth_result(tpAniSirGlobal pMac, QDF_STATUS status,
				    tpPESession psessionEntry)
{
	if (NULL == psessionEntry ||
	    NULL == psessionEntry->ftPEContext.pFTPreAuthReq)
		return;

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return;
	}

	if (psessionEntry->ftPEContext.ftPreAuthStatus == eSIR_SUCCESS) {
		psessionEntry->ftPEContext.ftPreAuthStatus =
			lim_ft_setup_auth_session(pMac, psessionEntry);
	}
	/* Post the FT Pre Auth Response to SME */
	lim_post_ft_pre_auth_rsp(pMac,
		psessionEntry->ftPEContext.ftPreAuthStatus,
		psessionEntry->ftPEContext.saved_auth_rsp,
		psessionEntry->ftPEContext.saved_auth_rsp_length,
		psessionEntry);
}

/**
 * lim_handle_ft_pre_auth_rsp() - Handle the Auth response
 * @pMac: Global MAC Context
 * @status: Status Code
 * @auth_rsp: Auth Response
 * @auth_rsp_length: Auth response length
 * @psessionEntry: PE Session
 *
 * Send the FT Pre Auth Response to SME whenever we have a status
 * ready to be sent to SME
 *
 * SME will be the one to send it up to the supplicant to receive
 * FTIEs which will be required for Reassoc Req.
 *
 * @Return: None
 */
void lim_handle_ft_pre_auth_rsp(tpAniSirGlobal pMac, tSirRetStatus status,
				uint8_t *auth_rsp, uint16_t auth_rsp_length,
				tpPESession psessionEntry)
{
	tpPESession pftSessionEntry = NULL;
	uint8_t sessionId = 0;
	tpSirBssDescription pbssDescription = NULL;
#ifdef FEATURE_WLAN_DIAG_SUPPORT
	lim_diag_event_report(pMac, WLAN_PE_DIAG_PRE_AUTH_RSP_EVENT,
			      psessionEntry, (uint16_t) status, 0);
#endif

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		lim_log(pMac, LOGE, FL("psessionEntry is not in STA mode"));
		return;
	}

	/* Save the status of pre-auth */
	psessionEntry->ftPEContext.ftPreAuthStatus = status;

	/* Save the auth rsp, so we can send it to
	 * SME once we resume link
	 */
	psessionEntry->ftPEContext.saved_auth_rsp_length = 0;
	if ((auth_rsp != NULL) && (auth_rsp_length < MAX_FTIE_SIZE)) {
		qdf_mem_copy(psessionEntry->ftPEContext.saved_auth_rsp,
			     auth_rsp, auth_rsp_length);
		psessionEntry->ftPEContext.saved_auth_rsp_length =
			auth_rsp_length;
	}

	if (!psessionEntry->ftPEContext.pFTPreAuthReq ||
	    !psessionEntry->ftPEContext.pFTPreAuthReq->pbssDescription) {
		lim_log(pMac, LOGE,
			FL("pFTPreAuthReq or pbssDescription is NULL"));
		return;
	}

	/* Create FT session for the re-association at this point */
	if (psessionEntry->ftPEContext.ftPreAuthStatus == eSIR_SUCCESS) {
		pbssDescription =
		      psessionEntry->ftPEContext.pFTPreAuthReq->pbssDescription;
		lim_print_mac_addr(pMac, pbssDescription->bssId, LOG1);
		pftSessionEntry =
			pe_create_session(pMac, pbssDescription->bssId,
					&sessionId, pMac->lim.maxStation,
					psessionEntry->bssType);
		if (pftSessionEntry == NULL) {
			lim_log(pMac, LOGE, FL(
				"Session not created for pre-auth 11R AP"));
			status = eSIR_FAILURE;
			psessionEntry->ftPEContext.ftPreAuthStatus = status;
			goto send_rsp;
		}
		pftSessionEntry->peSessionId = sessionId;
		pftSessionEntry->smeSessionId = psessionEntry->smeSessionId;
		sir_copy_mac_addr(pftSessionEntry->selfMacAddr,
				  psessionEntry->selfMacAddr);
		sir_copy_mac_addr(pftSessionEntry->limReAssocbssId,
				  pbssDescription->bssId);
		pftSessionEntry->bssType = psessionEntry->bssType;

		if (pftSessionEntry->bssType == eSIR_INFRASTRUCTURE_MODE)
			pftSessionEntry->limSystemRole = eLIM_STA_ROLE;
		else
			lim_log(pMac, LOGE, FL("Invalid bss type"));
		pftSessionEntry->limPrevSmeState = pftSessionEntry->limSmeState;
		qdf_mem_copy(&(pftSessionEntry->htConfig),
			     &(psessionEntry->htConfig),
			     sizeof(psessionEntry->htConfig));
		pftSessionEntry->limSmeState = eLIM_SME_WT_REASSOC_STATE;

		lim_log(pMac, LOG1, FL("created session (%p) with id = %d"),
			pftSessionEntry, pftSessionEntry->peSessionId);

		/* Update the ReAssoc BSSID of the current session */
		sir_copy_mac_addr(psessionEntry->limReAssocbssId,
				  pbssDescription->bssId);
		lim_print_mac_addr(pMac, psessionEntry->limReAssocbssId, LOG1);
	}
send_rsp:
	if (psessionEntry->currentOperChannel !=
	    psessionEntry->ftPEContext.pFTPreAuthReq->preAuthchannelNum) {
		/* Need to move to the original AP channel */
		lim_process_abort_scan_ind(pMac, psessionEntry->peSessionId,
			psessionEntry->ftPEContext.pFTPreAuthReq->scan_id,
			PREAUTH_REQUESTOR_ID);
	} else {
		lim_log(pMac, LOG1,
			"Pre auth on same channel as connected AP channel %d",
			psessionEntry->ftPEContext.pFTPreAuthReq->
			preAuthchannelNum);
		lim_ft_process_pre_auth_result(pMac, status, psessionEntry);
	}
}

/*
 * lim_process_ft_preauth_rsp_timeout() - process ft preauth rsp timeout
 *
 * @mac_ctx:		global mac ctx
 *
 * This function is called if preauth response is not received from the AP
 * within this timeout while FT in progress
 *
 * Return: void
 */
void lim_process_ft_preauth_rsp_timeout(tpAniSirGlobal mac_ctx)
{
	tpPESession session;

	/*
	 * We have failed pre auth. We need to resume link and get back on
	 * home channel
	 */
	lim_log(mac_ctx, LOGE, FL("FT Pre-Auth Time Out!!!!"));
	session = pe_find_session_by_session_id(mac_ctx,
			mac_ctx->lim.limTimers.gLimFTPreAuthRspTimer.sessionId);
	if (NULL == session) {
		lim_log(mac_ctx, LOGE,
			FL("Session Does not exist for given sessionID"));
		return;
	}

	/* Nothing to be done if the session is not in STA mode */
	if (!LIM_IS_STA_ROLE(session)) {
		lim_log(mac_ctx, LOGE, FL("session is not in STA mode"));
		return;
	}

	/* Reset the flag to indicate preauth request session */
	session->ftPEContext.ftPreAuthSession = false;

	if (NULL == session->ftPEContext.pFTPreAuthReq) {
		/* Auth Rsp might already be posted to SME and ftcleanup done */
		lim_log(mac_ctx, LOGE, FL("pFTPreAuthReq is NULL sessionId:%d"),
			mac_ctx->lim.limTimers.gLimFTPreAuthRspTimer.sessionId);
		return;
	}

	/*
	 * To handle the race condition where we recieve preauth rsp after
	 * timer has expired.
	 */
	if (true ==
	    session->ftPEContext.pFTPreAuthReq->bPreAuthRspProcessed) {
		lim_log(mac_ctx, LOGE,
			FL("Auth rsp already posted to SME (session %p)"),
			session);
		return;
	} else {
		/*
		 * Here we are sending preauth rsp with failure state
		 * and which is forwarded to SME. Now, if we receive an preauth
		 * resp from AP with success it would create a FT pesession, but
		 * will be dropped in SME leaving behind the pesession. Mark
		 * Preauth rsp processed so that any rsp from AP is dropped in
		 * lim_process_auth_frame_no_session.
		 */
		lim_log(mac_ctx, LOG1,
			FL("Auth rsp not yet posted to SME (session %p)"),
			session);
		session->ftPEContext.pFTPreAuthReq->bPreAuthRspProcessed = true;
	}

	/*
	 * Attempted at Pre-Auth and failed. If we are off channel. We need
	 * to get back to home channel
	 */
	lim_handle_ft_pre_auth_rsp(mac_ctx, eSIR_FAILURE, NULL, 0, session);
}

/*
 * lim_post_ft_pre_auth_rsp() - post ft pre auth response to SME.
 *
 * @mac_ctx:		global mac ctx
 * @status:		status code to post in auth rsp
 * @auth_rsp:		pointer to auth rsp FT ie
 * @auth_rsp_length:	len of the IE field
 * @session:	        pe session
 *
 * post pre auth response to SME.
 *
 * Return: void
 */
void lim_post_ft_pre_auth_rsp(tpAniSirGlobal mac_ctx,
			      tSirRetStatus status,
			      uint8_t *auth_rsp,
			      uint16_t auth_rsp_length,
			      tpPESession session)
{
	tpSirFTPreAuthRsp ft_pre_auth_rsp;
	tSirMsgQ mmh_msg;
	uint16_t rsp_len = sizeof(tSirFTPreAuthRsp);

	ft_pre_auth_rsp = (tpSirFTPreAuthRsp) qdf_mem_malloc(rsp_len);
	if (NULL == ft_pre_auth_rsp) {
		lim_log(mac_ctx, LOGE, "Failed to allocate memory");
		QDF_ASSERT(ft_pre_auth_rsp != NULL);
		return;
	}
	qdf_mem_zero(ft_pre_auth_rsp, rsp_len);

	lim_log(mac_ctx, LOG1, FL("Auth Rsp = %p"), ft_pre_auth_rsp);
	if (session) {
		/* Nothing to be done if the session is not in STA mode */
		if (!LIM_IS_STA_ROLE(session)) {
			lim_log(mac_ctx, LOGE,
				FL("session is not in STA mode"));
			qdf_mem_free(ft_pre_auth_rsp);
			return;
		}
		ft_pre_auth_rsp->smeSessionId = session->smeSessionId;
		/* The bssid of the AP we are sending Auth1 to. */
		if (session->ftPEContext.pFTPreAuthReq)
			sir_copy_mac_addr(ft_pre_auth_rsp->preAuthbssId,
			    session->ftPEContext.pFTPreAuthReq->preAuthbssId);
	}

	ft_pre_auth_rsp->messageType = eWNI_SME_FT_PRE_AUTH_RSP;
	ft_pre_auth_rsp->length = (uint16_t) rsp_len;
	ft_pre_auth_rsp->status = status;

	/* Attach the auth response now back to SME */
	ft_pre_auth_rsp->ft_ies_length = 0;
	if ((auth_rsp != NULL) && (auth_rsp_length < MAX_FTIE_SIZE)) {
		/* Only 11r assoc has FT IEs */
		qdf_mem_copy(ft_pre_auth_rsp->ft_ies,
			     auth_rsp, auth_rsp_length);
		ft_pre_auth_rsp->ft_ies_length = auth_rsp_length;
	}

	if (status != eSIR_SUCCESS) {
		/*
		 * Ensure that on Pre-Auth failure the cached Pre-Auth Req and
		 * other allocated memory is freed up before returning.
		 */
		lim_log(mac_ctx, LOG1, "Pre-Auth Failed, Cleanup!");
		lim_ft_cleanup(mac_ctx, session);
	}

	mmh_msg.type = ft_pre_auth_rsp->messageType;
	mmh_msg.bodyptr = ft_pre_auth_rsp;
	mmh_msg.bodyval = 0;

	lim_log(mac_ctx, LOG1, FL("Posted Auth Rsp to SME with status of 0x%x"),
		status);
#ifdef FEATURE_WLAN_DIAG_SUPPORT_LIM    /* FEATURE_WLAN_DIAG_SUPPORT */
	if (status == eSIR_SUCCESS)
		lim_diag_event_report(mac_ctx, WLAN_PE_DIAG_PREAUTH_DONE,
				      session, status, 0);
#endif
	lim_sys_process_mmh_msg_api(mac_ctx, &mmh_msg, ePROT);
}

/**
 * lim_send_preauth_scan_offload() - Send scan command to handle preauth.
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: pe session id
 * @ft_preauth_req: Preauth request with parameters
 *
 * Builds a single channel scan request and sends it to WMA.
 * Scan dwell time is the time allocated to go to preauth candidate
 * channel for auth frame exchange.
 *
 * Return: Status of sending message to WMA.
 */
QDF_STATUS lim_send_preauth_scan_offload(tpAniSirGlobal mac_ctx,
			uint8_t session_id,
			tSirFTPreAuthReq *ft_preauth_req)
{
	tSirScanOffloadReq *scan_offload_req;
	tSirRetStatus rc = eSIR_SUCCESS;
	tSirMsgQ msg;

	scan_offload_req = qdf_mem_malloc(sizeof(tSirScanOffloadReq));
	if (NULL == scan_offload_req) {
		lim_log(mac_ctx, LOGE,
			FL("Memory allocation failed for pScanOffloadReq"));
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(scan_offload_req, sizeof(tSirScanOffloadReq));

	msg.type = WMA_START_SCAN_OFFLOAD_REQ;
	msg.bodyptr = scan_offload_req;
	msg.bodyval = 0;

	qdf_mem_copy((uint8_t *) &scan_offload_req->selfMacAddr.bytes,
		     (uint8_t *) ft_preauth_req->self_mac_addr,
		     sizeof(tSirMacAddr));

	qdf_mem_copy((uint8_t *) &scan_offload_req->bssId.bytes,
		     (uint8_t *) ft_preauth_req->currbssId,
		     sizeof(tSirMacAddr));
	scan_offload_req->scanType = eSIR_PASSIVE_SCAN;
	/*
	 * P2P_SCAN_TYPE_LISTEN tells firmware to allow mgt frames to/from
	 * mac address that is not of connected AP.
	 */
	scan_offload_req->p2pScanType = P2P_SCAN_TYPE_LISTEN;
	scan_offload_req->restTime = 0;
	scan_offload_req->minChannelTime = LIM_FT_PREAUTH_SCAN_TIME;
	scan_offload_req->maxChannelTime = LIM_FT_PREAUTH_SCAN_TIME;
	scan_offload_req->sessionId = session_id;
	scan_offload_req->channelList.numChannels = 1;
	scan_offload_req->channelList.channelNumber[0] =
		ft_preauth_req->preAuthchannelNum;
	wma_get_scan_id(&ft_preauth_req->scan_id);
	scan_offload_req->scan_id = ft_preauth_req->scan_id;
	scan_offload_req->scan_requestor_id = PREAUTH_REQUESTOR_ID;

	lim_log(mac_ctx, LOG1,
		FL("Scan request: duration %u, session %hu, chan %hu"),
		scan_offload_req->maxChannelTime, session_id,
		ft_preauth_req->preAuthchannelNum);

	rc = wma_post_ctrl_msg(mac_ctx, &msg);
	if (rc != eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGE, FL("START_SCAN_OFFLOAD failed %u"), rc);
		qdf_mem_free(scan_offload_req);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * lim_preauth_scan_event_handler() - Process firmware preauth scan events
 *
 * @mac_ctx:Pointer to global MAC structure
 * @event: Scan event
 * @session_id: session entry
 * @scan_id: scan id from WMA scan event.
 *
 * If scan event signifies failure or successful completion, operation
 * is complete.
 * If scan event signifies that STA is on foreign channel, send auth frame
 *
 * Return: void
 */

void lim_preauth_scan_event_handler(tpAniSirGlobal mac_ctx,
				enum sir_scan_event_type event,
				uint8_t session_id,
				uint32_t scan_id)
{
	tpPESession session_entry;

	session_entry = pe_find_session_by_session_id(mac_ctx, session_id);
	if (session_entry == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("SessionId:%d Session Does not exist"), session_id);
		return;
	}

	switch (event) {
	case SIR_SCAN_EVENT_START_FAILED:
		/* Scan command is rejected by firmware */
		lim_log(mac_ctx, LOGE, FL("Failed to start preauth scan"));
		lim_post_ft_pre_auth_rsp(mac_ctx, eSIR_FAILURE, NULL, 0,
					 session_entry);
		return;

	case SIR_SCAN_EVENT_COMPLETED:
		/*
		 * Scan either completed succesfully or or got terminated
		 * after successful auth, or timed out. Either way, STA
		 * is back to home channel. Data traffic can continue.
		 */
		lim_ft_process_pre_auth_result(mac_ctx, QDF_STATUS_SUCCESS,
			session_entry);
		break;

	case SIR_SCAN_EVENT_FOREIGN_CHANNEL:
		/* Sta is on candidate channel. Send auth */
		lim_perform_ft_pre_auth(mac_ctx, QDF_STATUS_SUCCESS, NULL,
					session_entry);
		break;
	default:
		/* Don't print message for scan events that are ignored */
		break;
	}
}

