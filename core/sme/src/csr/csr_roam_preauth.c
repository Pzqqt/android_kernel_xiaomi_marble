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
 * DOC: csr_roam_preauth.c
 *
 * Host based roaming preauthentication implementation
 */

#include "wma_types.h"
#include "cds_mq.h"
#include "csr_inside_api.h"
#include "sms_debug.h"
#include "sme_qos_internal.h"
#include "sme_inside.h"
#include "host_diag_core_event.h"
#include "host_diag_core_log.h"
#include "csr_api.h"
#include "sme_api.h"
#include "csr_neighbor_roam.h"
#include "mac_trace.h"
#include "cds_concurrency.h"

static void csr_reinit_preauth_cmd(tpAniSirGlobal pMac, tSmeCmd *pCommand);
static QDF_STATUS csr_neighbor_roam_add_preauth_fail(tpAniSirGlobal mac_ctx,
			uint8_t session_id, tSirMacAddr bssid);
/**
 * csr_neighbor_roam_state_preauth_done() - Check if state is preauth done
 * @mac_ctx: Global MAC context
 * @session_id: SME session ID
 *
 * Return: True if the state id preauth done, false otherwise
 */
bool csr_neighbor_roam_state_preauth_done(tpAniSirGlobal mac_ctx,
		uint8_t session_id)
{
	return mac_ctx->roam.neighborRoamInfo[session_id].neighborRoamState ==
		eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE;
}

/**
 * csr_neighbor_roam_tranistion_preauth_done_to_disconnected() - Transition
 * the state from preauth done to disconnected
 * @mac_ctx: Global MAC Context
 * @session_id: SME Session ID
 *
 * In the event that we are associated with AP1 and we have
 * completed pre auth with AP2. Then we receive a deauth/disassoc from AP1.
 * At this point neighbor roam is in pre auth done state, pre auth timer
 * is running. We now handle this case by stopping timer and clearing
 * the pre-auth state. We basically clear up and just go to disconnected
 * state
 *
 * Return: None
 */
void csr_neighbor_roam_tranistion_preauth_done_to_disconnected(
		tpAniSirGlobal mac_ctx, uint8_t session_id)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			  FL("session is NULL"));
		return;
	}

	if (pNeighborRoamInfo->neighborRoamState !=
	    eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE)
		return;

	qdf_mc_timer_stop(&session->ftSmeContext.preAuthReassocIntvlTimer);
	csr_neighbor_roam_state_transition(mac_ctx,
		eCSR_NEIGHBOR_ROAM_STATE_INIT, session_id);
	pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived = false;
}

/**
 * csr_reinit_preauth_cmd() - Cleanup the preauth command
 * @mac_ctx: Global MAC context
 * @command: Command to be cleaned up
 *
 * Return: None
 */
static void csr_reinit_preauth_cmd(tpAniSirGlobal mac_ctx, tSmeCmd *command)
{
	command->u.roamCmd.pLastRoamBss = NULL;
	command->u.roamCmd.pRoamBssEntry = NULL;
	qdf_mem_set(&command->u.roamCmd, sizeof(tRoamCmd), 0);
}

/**
 * csr_release_command_preauth() - Release the preauth command
 * @mac_ctx: Global MAC context
 * @command: Command to be released
 *
 * Return: None
 */
void csr_release_command_preauth(tpAniSirGlobal mac_ctx, tSmeCmd *command)
{
	csr_reinit_preauth_cmd(mac_ctx, command);
	csr_release_command(mac_ctx, command);
}

/**
 * csr_roam_enqueue_preauth() - Put the preauth command in the queue
 * @mac_ctx: Global MAC Context
 * @session_id: SME Session ID
 * @bss_desc: BSS descriptor
 * @reason: roaming reason
 * @immediate: High priority in the queue or not
 *
 * Return: Success if queued properly, false otherwise.
 */
QDF_STATUS csr_roam_enqueue_preauth(tpAniSirGlobal mac_ctx,
		uint32_t session_id, tpSirBssDescription bss_desc,
		eCsrRoamReason reason, bool immediate)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *command;

	command = csr_get_command_buffer(mac_ctx);
	if (NULL == command) {
		sms_log(mac_ctx, LOGE, FL(" fail to get command buffer"));
		status = QDF_STATUS_E_RESOURCES;
	} else {
		if (bss_desc) {
			command->command = eSmeCommandRoam;
			command->sessionId = (uint8_t) session_id;
			command->u.roamCmd.roamReason = reason;
			command->u.roamCmd.pLastRoamBss = bss_desc;
			status = csr_queue_sme_command(mac_ctx, command,
					immediate);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				sms_log(mac_ctx, LOGE,
					FL("fail to queue preauth,status=%d"),
						status);
				csr_release_command_preauth(mac_ctx, command);
			}
		} else {
			status = QDF_STATUS_E_RESOURCES;
		}
	}
	return status;
}

/**
 * csr_neighbor_roam_purge_preauth_failed_list() - Purge the preauth fail list
 * @mac_ctx: Global MAC Context
 *
 * Return: None
 */
void csr_neighbor_roam_purge_preauth_failed_list(tpAniSirGlobal mac_ctx)
{
	uint8_t i;
	uint8_t j;
	uint8_t num_mac_addr;
	tpCsrNeighborRoamControlInfo neigh_roam_info = NULL;
	tpCsrPreauthFailListInfo fail_list;

	for (j = 0; j < CSR_ROAM_SESSION_MAX; j++) {
		neigh_roam_info = &mac_ctx->roam.neighborRoamInfo[j];
		fail_list = &neigh_roam_info->FTRoamInfo.preAuthFailList;
		num_mac_addr = fail_list->numMACAddress;
		for (i = 0; i < num_mac_addr; i++)
			qdf_mem_zero(fail_list->macAddress[i],
					sizeof(tSirMacAddr));
		fail_list->numMACAddress = 0;
	}
}

/**
 * @csr_neighbor_roam_reset_preauth_control_info - Reset preauth info
 * @mac_ctx: Global MAC Context
 * @session_id: SME Session ID
 *
 * Return: None
 */
void csr_neighbor_roam_reset_preauth_control_info(tpAniSirGlobal mac_ctx,
		uint8_t session_id)
{
	tpCsrNeighborRoamControlInfo neigh_roam_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];

	neigh_roam_info->is11rAssoc = false;
	csr_neighbor_roam_purge_preauth_failed_list(mac_ctx);

	neigh_roam_info->FTRoamInfo.preauthRspPending = false;
	neigh_roam_info->FTRoamInfo.numPreAuthRetries = 0;
	neigh_roam_info->FTRoamInfo.currentNeighborRptRetryNum = 0;
	neigh_roam_info->FTRoamInfo.neighborRptPending = false;
	neigh_roam_info->FTRoamInfo.numBssFromNeighborReport = 0;
	qdf_mem_zero(neigh_roam_info->FTRoamInfo.neighboReportBssInfo,
		     sizeof(tCsrNeighborReportBssInfo) *
		     MAX_BSS_IN_NEIGHBOR_RPT);
	neigh_roam_info->uOsRequestedHandoff = 0;
	qdf_mem_zero(&neigh_roam_info->handoffReqInfo,
		     sizeof(tCsrHandoffRequest));
}

/**
 * csr_neighbor_roam_preauth_rsp_handler() - handle preauth response
 * @mac_ctx: The handle returned by mac_open.
 * @session_id: SME session
 * @lim_status: eSIR_SUCCESS/eSIR_FAILURE/eSIR_LIM_MAX_STA_REACHED_ERROR/
 *              eSIT_LIM_AUTH_RSP_TIMEOUT status from PE
 *
 * This function handle the Preauth response from PE
 * Every preauth is allowed max 3 tries if it fails. If a bssid failed
 * for more than MAX_TRIES, we will remove it from the list and try
 * with the next node in the roamable AP list and add the BSSID to
 * pre-auth failed list. If no more entries present in roamable AP list,
 * transition to REPORT_SCAN state.
 *
 * Return: QDF_STATUS_SUCCESS on success (i.e. pre-auth processed),
 *         QDF_STATUS_E_FAILURE otherwise
 */
QDF_STATUS csr_neighbor_roam_preauth_rsp_handler(tpAniSirGlobal mac_ctx,
						 uint8_t session_id,
						 tSirRetStatus lim_status)
{
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS preauth_processed = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamBSSInfo preauth_rsp_node = NULL;

	if (false == neighbor_roam_info->FTRoamInfo.preauthRspPending) {
		/*
		 * This can happen when we disconnect immediately
		 * after sending a pre-auth request. During processing
		 * of the disconnect command, we would have reset
		 * preauthRspPending and transitioned to INIT state.
		 */
		sms_log(mac_ctx, LOGW,
			FL("Unexpected pre-auth response in state %d"),
			neighbor_roam_info->neighborRoamState);
		preauth_processed = QDF_STATUS_E_FAILURE;
		goto DEQ_PREAUTH;
	}
	/* We can receive it in these 2 states. */
	if ((neighbor_roam_info->neighborRoamState !=
	     eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING)) {
		sms_log(mac_ctx, LOGW,
			FL("Preauth response received in state %s"),
			mac_trace_get_neighbour_roam_state
				(neighbor_roam_info->neighborRoamState));
		preauth_processed = QDF_STATUS_E_FAILURE;
		goto DEQ_PREAUTH;
	}

	neighbor_roam_info->FTRoamInfo.preauthRspPending = false;

	if (eSIR_SUCCESS == lim_status)
		preauth_rsp_node =
			csr_neighbor_roam_next_roamable_ap(
				mac_ctx, &neighbor_roam_info->roamableAPList,
				NULL);
	if ((eSIR_SUCCESS == lim_status) && (NULL != preauth_rsp_node)) {
		sms_log(mac_ctx, LOG1,
			FL("Preauth completed successfully after %d tries"),
			neighbor_roam_info->FTRoamInfo.numPreAuthRetries);
		sms_log(mac_ctx, LOG1,
			FL("After Pre-Auth: BSSID " MAC_ADDRESS_STR ", Ch:%d"),
			MAC_ADDR_ARRAY(
				preauth_rsp_node->pBssDescription->bssId),
			(int)preauth_rsp_node->pBssDescription->channelId);

		csr_neighbor_roam_send_lfr_metric_event(mac_ctx, session_id,
			preauth_rsp_node->pBssDescription->bssId,
			eCSR_ROAM_PREAUTH_STATUS_SUCCESS);
		/*
		 * Preauth completed successfully. Insert the preauthenticated
		 * node to tail of preAuthDoneList.
		 */
		csr_neighbor_roam_remove_roamable_ap_list_entry(mac_ctx,
			&neighbor_roam_info->roamableAPList,
			preauth_rsp_node);
		csr_ll_insert_tail(
			&neighbor_roam_info->FTRoamInfo.preAuthDoneList,
			&preauth_rsp_node->List, LL_ACCESS_LOCK);

		csr_neighbor_roam_state_transition(mac_ctx,
			eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE, session_id);
		neighbor_roam_info->FTRoamInfo.numPreAuthRetries = 0;

		/*
		 * The caller of this function would start a timer and by
		 * the time it expires, supplicant should have provided
		 * the updated FTIEs to SME. So, when it expires, handoff
		 * will be triggered then.
		 */
	} else {
		tpCsrNeighborRoamBSSInfo neighbor_bss_node = NULL;
		tListElem *entry;
		bool is_dis_pending = false;

		sms_log(mac_ctx, LOGE,
			FL("Preauth failed retry number %d, status = 0x%x"),
			neighbor_roam_info->FTRoamInfo.numPreAuthRetries,
			lim_status);

		/*
		 * Preauth failed. Add the bssId to the preAuth failed list
		 * of MAC Address. Also remove the AP from roamable AP list.
		 */
		if ((neighbor_roam_info->FTRoamInfo.numPreAuthRetries >=
		     CSR_NEIGHBOR_ROAM_MAX_NUM_PREAUTH_RETRIES) ||
		    (eSIR_LIM_MAX_STA_REACHED_ERROR == lim_status)) {
			/*
			 * We are going to remove the node as it fails for
			 * more than MAX tries. Reset this count to 0
			 */
			neighbor_roam_info->FTRoamInfo.numPreAuthRetries = 0;

			/*
			 * The one in the head of the list should be one with
			 * which we issued pre-auth and failed
			 */
			entry = csr_ll_remove_head(
					&neighbor_roam_info->roamableAPList,
					LL_ACCESS_LOCK);
			if (!entry) {
				sms_log(mac_ctx, LOGE,
					FL("Preauth list is empty"));
				goto NEXT_PREAUTH;
			}
			neighbor_bss_node = GET_BASE_ADDR(entry,
					tCsrNeighborRoamBSSInfo,
					List);
			/*
			 * Add the BSSID to pre-auth fail list if
			 * it is not requested by HDD
			 */
			if (!neighbor_roam_info->uOsRequestedHandoff)
				status =
					csr_neighbor_roam_add_preauth_fail(
						mac_ctx, session_id,
						neighbor_bss_node->
							pBssDescription->bssId);
			csr_neighbor_roam_send_lfr_metric_event(mac_ctx,
				session_id,
				neighbor_bss_node->pBssDescription->bssId,
				eCSR_ROAM_PREAUTH_STATUS_FAILURE);
			/* Now we can free this node */
			csr_neighbor_roam_free_neighbor_roam_bss_node(
				mac_ctx, neighbor_bss_node);
		}
NEXT_PREAUTH:
		is_dis_pending = is_disconnect_pending(mac_ctx, session_id);
		if (is_dis_pending) {
			sms_log(mac_ctx, LOGE,
				FL("Disconnect in progress, Abort preauth"));
			goto ABORT_PREAUTH;
		}
		/* Issue preauth request for the same/next entry */
		if (QDF_STATUS_SUCCESS == csr_neighbor_roam_issue_preauth_req(
						mac_ctx, session_id))
			goto DEQ_PREAUTH;
ABORT_PREAUTH:
		if (csr_roam_is_roam_offload_scan_enabled(mac_ctx)) {
			if (neighbor_roam_info->uOsRequestedHandoff) {
				neighbor_roam_info->uOsRequestedHandoff = 0;
				csr_roam_offload_scan(mac_ctx, 0,
					ROAM_SCAN_OFFLOAD_START,
					REASON_PREAUTH_FAILED_FOR_ALL);
			} else {
				csr_roam_offload_scan(mac_ctx, 0,
					ROAM_SCAN_OFFLOAD_RESTART,
					REASON_PREAUTH_FAILED_FOR_ALL);
			}
			csr_neighbor_roam_state_transition(mac_ctx,
				eCSR_NEIGHBOR_ROAM_STATE_CONNECTED, session_id);
		}
	}

DEQ_PREAUTH:
	csr_dequeue_roam_command(mac_ctx, eCsrPerformPreauth);
	return preauth_processed;
}

/**
 * csr_neighbor_roam_add_preauth_fail() - add bssid to preauth failed list
 * @mac_ctx: The handle returned by mac_open.
 * @bssid: BSSID to be added to the preauth fail list
 *
 * This function adds the given BSSID to the Preauth fail list
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE otherwise
 */
static QDF_STATUS csr_neighbor_roam_add_preauth_fail(tpAniSirGlobal mac_ctx,
			uint8_t session_id, tSirMacAddr bssid)
{
	uint8_t i = 0;
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	uint8_t num_mac_addr = neighbor_roam_info->FTRoamInfo.preAuthFailList.
				numMACAddress;

	sms_log(mac_ctx, LOGE,
		FL(" Added BSSID " MAC_ADDRESS_STR " to Preauth failed list"),
		MAC_ADDR_ARRAY(bssid));

	for (i = 0;
	     i < neighbor_roam_info->FTRoamInfo.preAuthFailList.numMACAddress;
	     i++) {
		if (!qdf_mem_cmp(
		   neighbor_roam_info->FTRoamInfo.preAuthFailList.macAddress[i],
		   bssid, sizeof(tSirMacAddr))) {
			sms_log(mac_ctx, LOGW,
				FL("BSSID "MAC_ADDRESS_STR" already fail list"),
			MAC_ADDR_ARRAY(bssid));
			return QDF_STATUS_SUCCESS;
		}
	}

	if ((num_mac_addr + 1) > MAX_NUM_PREAUTH_FAIL_LIST_ADDRESS) {
		sms_log(mac_ctx, LOGE,
			FL("Cannot add, preauth fail list is full."));
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_copy(neighbor_roam_info->FTRoamInfo.preAuthFailList.
		     macAddress[num_mac_addr], bssid, sizeof(tSirMacAddr));
	neighbor_roam_info->FTRoamInfo.preAuthFailList.numMACAddress++;

	return QDF_STATUS_SUCCESS;
}

/**
 * csr_neighbor_roam_is_preauth_candidate()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @bssId : BSSID of the candidate
 *
 * This function checks whether the given MAC address is already present
 * in the preauth fail list and returns true/false accordingly
 *
 * Return: true if preauth candidate, false otherwise
 */
bool csr_neighbor_roam_is_preauth_candidate(tpAniSirGlobal pMac,
		    uint8_t sessionId, tSirMacAddr bssId)
{
	uint8_t i = 0;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];

	if (csr_roam_is_roam_offload_scan_enabled(pMac))
		return true;
	if (0 == pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress)
		return true;

	for (i = 0;
	     i < pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress;
	     i++) {
		if (!qdf_mem_cmp(pNeighborRoamInfo->FTRoamInfo.
				    preAuthFailList.macAddress[i], bssId,
				    sizeof(tSirMacAddr))) {
			sms_log(pMac, LOGE,
				FL("BSSID exists in fail list" MAC_ADDRESS_STR),
					MAC_ADDR_ARRAY(bssId));
			return false;
		}
	}

	return true;
}

/**
 * csr_roam_issue_ft_preauth_req() - Initiate Preauthentication request
 * @hal: Global Handle
 * @session_id: SME Session ID
 * @bss_desc: BSS descriptor
 *
 * Return: Success or Failure
 */
QDF_STATUS csr_roam_issue_ft_preauth_req(tHalHandle hal, uint32_t session_id,
			      tpSirBssDescription bss_desc)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	tpSirFTPreAuthReq preauth_req;
	uint16_t auth_req_len = 0;
	tCsrRoamSession *csr_session = CSR_GET_SESSION(mac_ctx, session_id);

	if (NULL == csr_session) {
		sms_log(mac_ctx, LOGE,
			FL("Session does not exist for session id(%d)"),
				session_id);
		return QDF_STATUS_E_FAILURE;
	}

	auth_req_len = sizeof(tSirFTPreAuthReq);
	preauth_req = (tpSirFTPreAuthReq) qdf_mem_malloc(auth_req_len);
	if (NULL == preauth_req) {
		sms_log(mac_ctx, LOGE,
			FL("Memory allocation for FT Preauth request failed"));
		return QDF_STATUS_E_NOMEM;
	}
	/* Save the SME Session ID. We need it while processing preauth resp */
	csr_session->ftSmeContext.smeSessionId = session_id;
	qdf_mem_zero(preauth_req, auth_req_len);

	preauth_req->pbssDescription =
		(tpSirBssDescription) qdf_mem_malloc(sizeof(bss_desc->length)
				+ bss_desc->length);
	if (NULL == preauth_req->pbssDescription) {
		sms_log(mac_ctx, LOGE,
			FL("Memory allocation for FT Preauth request failed"));
		return QDF_STATUS_E_NOMEM;
	}

	preauth_req->messageType = eWNI_SME_FT_PRE_AUTH_REQ;

	preauth_req->preAuthchannelNum = bss_desc->channelId;

	qdf_mem_copy((void *)&preauth_req->currbssId,
			(void *)csr_session->connectedProfile.bssid.bytes,
			sizeof(tSirMacAddr));
	qdf_mem_copy((void *)&preauth_req->preAuthbssId,
			(void *)bss_desc->bssId, sizeof(tSirMacAddr));
	qdf_mem_copy((void *)&preauth_req->self_mac_addr,
		(void *)&csr_session->selfMacAddr.bytes, sizeof(tSirMacAddr));

	if (csr_roam_is11r_assoc(mac_ctx, session_id) &&
	     (mac_ctx->roam.roamSession[session_id].connectedProfile.AuthType !=
	      eCSR_AUTH_TYPE_OPEN_SYSTEM)) {
		preauth_req->ft_ies_length =
			(uint16_t) csr_session->ftSmeContext.auth_ft_ies_length;
		qdf_mem_copy(preauth_req->ft_ies,
				csr_session->ftSmeContext.auth_ft_ies,
				csr_session->ftSmeContext.auth_ft_ies_length);
	} else {
		preauth_req->ft_ies_length = 0;
	}
	qdf_mem_copy(preauth_req->pbssDescription, bss_desc,
			sizeof(bss_desc->length) + bss_desc->length);
	preauth_req->length = auth_req_len;
	return cds_send_mb_message_to_mac(preauth_req);
}

/**
 * csr_roam_ft_pre_auth_rsp_processor() - Handle the preauth response
 * @hal: Global Handle
 * preauth_rsp: Received preauthentication response
 *
 * Return: None
 */
void csr_roam_ft_pre_auth_rsp_processor(tHalHandle hal,
					tpSirFTPreAuthRsp preauth_rsp)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamInfo roam_info;
	eCsrAuthType conn_Auth_type;
	uint32_t session_id = preauth_rsp->smeSessionId;
	tCsrRoamSession *csr_session = CSR_GET_SESSION(mac_ctx, session_id);

	if (NULL == csr_session) {
		sms_log(mac_ctx, LOGE, FL("CSR session is NULL"));
		return;
	}
	status = csr_neighbor_roam_preauth_rsp_handler(mac_ctx,
			preauth_rsp->smeSessionId, preauth_rsp->status);
	if (status != QDF_STATUS_SUCCESS) {
		sms_log(mac_ctx, LOGE,
			FL("Preauth was not processed: %d SessionID: %d"),
			status, session_id);
		return;
	}

	if (QDF_STATUS_SUCCESS != (QDF_STATUS) preauth_rsp->status)
		return;
	csr_session->ftSmeContext.FTState = eFT_AUTH_COMPLETE;
	csr_session->ftSmeContext.psavedFTPreAuthRsp = preauth_rsp;
	/* No need to notify qos module if this is a non 11r & ESE roam */
	if (csr_roam_is11r_assoc(mac_ctx, preauth_rsp->smeSessionId)
#ifdef FEATURE_WLAN_ESE
		|| csr_roam_is_ese_assoc(mac_ctx, preauth_rsp->smeSessionId)
#endif
	   ) {
		sme_qos_csr_event_ind(mac_ctx,
			csr_session->ftSmeContext.smeSessionId,
			SME_QOS_CSR_PREAUTH_SUCCESS_IND, NULL);
	}
	status =
		qdf_mc_timer_start(
			&csr_session->ftSmeContext.preAuthReassocIntvlTimer,
			60);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac_ctx, LOGE,
			FL("PreauthReassocInterval timer failed status %d"),
			status);
		return;
	}
	qdf_mem_copy((void *)&csr_session->ftSmeContext.preAuthbssId,
		(void *)preauth_rsp->preAuthbssId,
		sizeof(struct qdf_mac_addr));
	if (csr_roam_is11r_assoc(mac_ctx, preauth_rsp->smeSessionId))
		csr_roam_call_callback(mac_ctx, preauth_rsp->smeSessionId,
			NULL, 0, eCSR_ROAM_FT_RESPONSE, eCSR_ROAM_RESULT_NONE);

#ifdef FEATURE_WLAN_ESE
	if (csr_roam_is_ese_assoc(mac_ctx, preauth_rsp->smeSessionId)) {
		csr_roam_read_tsf(mac_ctx, (uint8_t *) roam_info.timestamp,
				preauth_rsp->smeSessionId);
		qdf_mem_copy((void *)&roam_info.bssid,
				(void *)preauth_rsp->preAuthbssId,
				sizeof(struct qdf_mac_addr));
		csr_roam_call_callback(mac_ctx, preauth_rsp->smeSessionId,
				&roam_info, 0, eCSR_ROAM_CCKM_PREAUTH_NOTIFY,
				0);
	}
#endif

	if (csr_roam_is_fast_roam_enabled(mac_ctx, preauth_rsp->smeSessionId)) {
		/* Save the bssid from the received response */
		qdf_mem_copy((void *)&roam_info.bssid,
				(void *)preauth_rsp->preAuthbssId,
				sizeof(struct qdf_mac_addr));
		csr_roam_call_callback(mac_ctx, preauth_rsp->smeSessionId,
				&roam_info, 0, eCSR_ROAM_PMK_NOTIFY, 0);
	}

	/* If its an Open Auth, FT IEs are not provided by supplicant */
	/* Hence populate them here */
	conn_Auth_type =
		mac_ctx->roam.roamSession[session_id].connectedProfile.AuthType;

	csr_session->ftSmeContext.addMDIE = false;

	/* Done with it, init it. */
	csr_session->ftSmeContext.psavedFTPreAuthRsp = NULL;

	if (csr_roam_is11r_assoc(mac_ctx, preauth_rsp->smeSessionId) &&
			(conn_Auth_type == eCSR_AUTH_TYPE_OPEN_SYSTEM)) {
		uint16_t ft_ies_length;
		ft_ies_length = preauth_rsp->ric_ies_length;

		if ((csr_session->ftSmeContext.reassoc_ft_ies) &&
			(csr_session->ftSmeContext.reassoc_ft_ies_length)) {
			qdf_mem_free(csr_session->ftSmeContext.reassoc_ft_ies);
			csr_session->ftSmeContext.reassoc_ft_ies_length = 0;
			csr_session->ftSmeContext.reassoc_ft_ies = NULL;
		}

		if (!ft_ies_length)
			return;

		csr_session->ftSmeContext.reassoc_ft_ies =
			qdf_mem_malloc(ft_ies_length);
		if (NULL == csr_session->ftSmeContext.reassoc_ft_ies) {
			sms_log(mac_ctx, LOGE,
				FL("Memory allocation failed for ft_ies"));
			return;
		} else {
			/* Copy the RIC IEs to reassoc IEs */
			qdf_mem_copy(((uint8_t *) csr_session->ftSmeContext.
						reassoc_ft_ies),
					(uint8_t *) preauth_rsp->ric_ies,
					preauth_rsp->ric_ies_length);
			csr_session->ftSmeContext.reassoc_ft_ies_length =
				ft_ies_length;
			csr_session->ftSmeContext.addMDIE = true;
		}
	}
}

/**
 * csr_neighbor_roam_issue_preauth_req() - Send preauth request to first AP
 * @mac_ctx: The handle returned by mac_open.
 * @session_id: Session information
 *
 * This function issues preauth request to PE with the 1st AP entry in the
 * roamable AP list
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE otherwise
 */
QDF_STATUS csr_neighbor_roam_issue_preauth_req(tpAniSirGlobal mac_ctx,
						      uint8_t session_id)
{
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamBSSInfo neighbor_bss_node;


	if (false != neighbor_roam_info->FTRoamInfo.preauthRspPending) {
		/* This must not be true here */
		QDF_ASSERT(neighbor_roam_info->FTRoamInfo.preauthRspPending ==
			   false);
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * Issue Preauth request to PE here.
	 * Need to issue the preauth request with the BSSID that is in the
	 * head of the roamable AP list. Parameters that should be passed are
	 * BSSID, Channel number and the neighborScanPeriod(probably). If
	 * roamableAPList gets empty, should transition to REPORT_SCAN state
	 */
	neighbor_bss_node = csr_neighbor_roam_next_roamable_ap(mac_ctx,
				&neighbor_roam_info->roamableAPList, NULL);

	if (NULL == neighbor_bss_node) {
		sms_log(mac_ctx, LOGW, FL("Roamable AP list is empty.. "));
		return QDF_STATUS_E_FAILURE;
	} else {
		csr_neighbor_roam_send_lfr_metric_event(mac_ctx, session_id,
			neighbor_bss_node->pBssDescription->bssId,
			eCSR_ROAM_PREAUTH_INIT_NOTIFY);
		status = csr_roam_enqueue_preauth(mac_ctx, session_id,
				neighbor_bss_node->pBssDescription,
				eCsrPerformPreauth, true);

		sms_log(mac_ctx, LOG1,
			FL("Before Pre-Auth: BSSID " MAC_ADDRESS_STR ", Ch:%d"),
			MAC_ADDR_ARRAY(
				neighbor_bss_node->pBssDescription->bssId),
			(int)neighbor_bss_node->pBssDescription->channelId);

		if (QDF_STATUS_SUCCESS != status) {
			sms_log(mac_ctx, LOGE,
				FL("Return failed preauth request status %d"),
				status);
			return status;
		}
	}

	neighbor_roam_info->FTRoamInfo.preauthRspPending = true;
	neighbor_roam_info->FTRoamInfo.numPreAuthRetries++;
	csr_neighbor_roam_state_transition(mac_ctx,
		eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING, session_id);

	return status;
}

