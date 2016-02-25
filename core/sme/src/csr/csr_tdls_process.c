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

/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *

    \file csr_tdls_process.c

    Implementation for the TDLS interface to PE.
   ========================================================================== */

#ifdef FEATURE_WLAN_TDLS

#include "ani_global.h"          /* for tpAniSirGlobal */
#include "cds_mq.h"
#include "csr_inside_api.h"
#include "sme_inside.h"
#include "sms_debug.h"

#include "csr_support.h"

#include "host_diag_core_log.h"
#include "host_diag_core_event.h"
#include "csr_internal.h"

/*
 * common routine to remove TDLS cmd from SME command list..
 * commands are removed after getting reponse from PE.
 */
QDF_STATUS csr_tdls_remove_sme_cmd(tpAniSirGlobal pMac, eSmeCommandType cmdType)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tListElem *pEntry;
	tSmeCmd *pCommand;

	pEntry = csr_ll_peek_head(&pMac->sme.smeCmdActiveList, LL_ACCESS_LOCK);
	if (pEntry) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		if (cmdType == pCommand->command) {
			if (csr_ll_remove_entry(&pMac->sme.smeCmdActiveList,
						pEntry, LL_ACCESS_LOCK)) {
				cdf_mem_zero(&pCommand->u.tdlsCmd,
					     sizeof(tTdlsCmd));
				csr_release_command(pMac, pCommand);
				sme_process_pending_queue(pMac);
				status = QDF_STATUS_SUCCESS;
			}
		}
	}
	return status;
}

/**
 * csr_tdls_send_mgmt_req() - to send a TDLS frame to PE through SME
 * @hHal: HAL context
 * @sessionId: SME session id
 * @tdlsSendMgmt: tdls mgmt pointer
 *
 * TDLS request API, called from HDD to send a TDLS frame in SME/CSR and
 * send message to PE to trigger TDLS discovery procedure.
 */
QDF_STATUS csr_tdls_send_mgmt_req(tHalHandle hHal, uint8_t sessionId,
				  tCsrTdlsSendMgmt *tdlsSendMgmt)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSmeCmd *tdlsSendMgmtCmd;
	tTdlsSendMgmtCmdInfo *tdlsSendMgmtCmdInfo;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	/* If connected and in Infra. Only then allow this */
	if (!CSR_IS_SESSION_VALID(pMac, sessionId) ||
			!csr_is_conn_state_connected_infra(pMac, sessionId) ||
			(NULL == tdlsSendMgmt))
		return status;
	tdlsSendMgmtCmd = csr_get_command_buffer(pMac);
	if (!tdlsSendMgmtCmd)
		return status;
	tdlsSendMgmtCmdInfo = &tdlsSendMgmtCmd->u.tdlsCmd.u.tdlsSendMgmtCmdInfo;
	tdlsSendMgmtCmd->sessionId = sessionId;
	tdlsSendMgmtCmdInfo->frameType = tdlsSendMgmt->frameType;
	tdlsSendMgmtCmdInfo->dialog = tdlsSendMgmt->dialog;
	tdlsSendMgmtCmdInfo->statusCode = tdlsSendMgmt->statusCode;
	tdlsSendMgmtCmdInfo->responder = tdlsSendMgmt->responder;
	tdlsSendMgmtCmdInfo->peerCapability = tdlsSendMgmt->peerCapability;
	cdf_mem_copy(tdlsSendMgmtCmdInfo->peerMac, tdlsSendMgmt->peerMac,
			sizeof(tSirMacAddr));

	if ((0 != tdlsSendMgmt->len) && (NULL != tdlsSendMgmt->buf)) {
		tdlsSendMgmtCmdInfo->buf = cdf_mem_malloc(tdlsSendMgmt->len);
		if (NULL == tdlsSendMgmtCmdInfo->buf) {
			status = QDF_STATUS_E_NOMEM;
			sms_log(pMac, LOGE, FL("Alloc Failed"));
			QDF_ASSERT(0);
			return status;
		}
		cdf_mem_copy(tdlsSendMgmtCmdInfo->buf, tdlsSendMgmt->buf,
				tdlsSendMgmt->len);
		tdlsSendMgmtCmdInfo->len = tdlsSendMgmt->len;
	} else {
		tdlsSendMgmtCmdInfo->buf = NULL;
		tdlsSendMgmtCmdInfo->len = 0;
	}

	tdlsSendMgmtCmd->command = eSmeCommandTdlsSendMgmt;
	tdlsSendMgmtCmd->u.tdlsCmd.size = sizeof(tTdlsSendMgmtCmdInfo);
	sme_push_command(pMac, tdlsSendMgmtCmd, false);
	status = QDF_STATUS_SUCCESS;
	return status;
}

/*
 * TDLS request API, called from HDD to add a TDLS peer
 */
QDF_STATUS csr_tdls_change_peer_sta(tHalHandle hHal, uint8_t sessionId,
				    const tSirMacAddr peerMac,
				    tCsrStaParams *pstaParams)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSmeCmd *tdlsAddStaCmd;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	/* If connected and in Infra. Only then allow this */
	if (CSR_IS_SESSION_VALID(pMac, sessionId) &&
	    csr_is_conn_state_connected_infra(pMac, sessionId) &&
	    (NULL != peerMac) && (NULL != pstaParams)) {

		tdlsAddStaCmd = csr_get_command_buffer(pMac);

		if (tdlsAddStaCmd) {
			tTdlsAddStaCmdInfo *tdlsAddStaCmdInfo =
				&tdlsAddStaCmd->u.tdlsCmd.u.tdlsAddStaCmdInfo;

			tdlsAddStaCmdInfo->tdlsAddOper = TDLS_OPER_UPDATE;

			tdlsAddStaCmd->sessionId = sessionId;

			cdf_mem_copy(tdlsAddStaCmdInfo->peermac.bytes,
				     peerMac, QDF_MAC_ADDR_SIZE);
			tdlsAddStaCmdInfo->capability = pstaParams->capability;
			tdlsAddStaCmdInfo->uapsdQueues =
				pstaParams->uapsd_queues;
			tdlsAddStaCmdInfo->maxSp = pstaParams->max_sp;
			cdf_mem_copy(tdlsAddStaCmdInfo->extnCapability,
				     pstaParams->extn_capability,
				     sizeof(pstaParams->extn_capability));

			tdlsAddStaCmdInfo->htcap_present =
				pstaParams->htcap_present;
			if (pstaParams->htcap_present)
				cdf_mem_copy(&tdlsAddStaCmdInfo->HTCap,
					     &pstaParams->HTCap,
					     sizeof(pstaParams->HTCap));
			else
				cdf_mem_set(&tdlsAddStaCmdInfo->HTCap,
					    sizeof(pstaParams->HTCap), 0);

			tdlsAddStaCmdInfo->vhtcap_present =
				pstaParams->vhtcap_present;
			if (pstaParams->vhtcap_present)
				cdf_mem_copy(&tdlsAddStaCmdInfo->VHTCap,
					     &pstaParams->VHTCap,
					     sizeof(pstaParams->VHTCap));
			else
				cdf_mem_set(&tdlsAddStaCmdInfo->VHTCap,
					    sizeof(pstaParams->VHTCap), 0);

			tdlsAddStaCmdInfo->supportedRatesLen =
				pstaParams->supported_rates_len;

			if (0 != pstaParams->supported_rates_len)
				cdf_mem_copy(&tdlsAddStaCmdInfo->supportedRates,
					     pstaParams->supported_rates,
					     pstaParams->supported_rates_len);

			tdlsAddStaCmd->command = eSmeCommandTdlsAddPeer;
			tdlsAddStaCmd->u.tdlsCmd.size =
				sizeof(tTdlsAddStaCmdInfo);
			sme_push_command(pMac, tdlsAddStaCmd, false);
			status = QDF_STATUS_SUCCESS;
		}
	}

	return status;
}

/*
 * TDLS request API, called from HDD to Send Link Establishment Parameters
 */
QDF_STATUS csr_tdls_send_link_establish_params(tHalHandle hHal,
					       uint8_t sessionId,
					       const tSirMacAddr peerMac,
					       tCsrTdlsLinkEstablishParams *
					       tdlsLinkEstablishParams)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSmeCmd *tdlsLinkEstablishCmd;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	/* If connected and in Infra. Only then allow this */
	if (CSR_IS_SESSION_VALID(pMac, sessionId) &&
	    csr_is_conn_state_connected_infra(pMac, sessionId) &&
	    (NULL != peerMac)) {
		tdlsLinkEstablishCmd = csr_get_command_buffer(pMac);

		if (tdlsLinkEstablishCmd) {
			tTdlsLinkEstablishCmdInfo *tdlsLinkEstablishCmdInfo =
				&tdlsLinkEstablishCmd->u.tdlsCmd.u.
				tdlsLinkEstablishCmdInfo;

			tdlsLinkEstablishCmd->sessionId = sessionId;

			cdf_mem_copy(tdlsLinkEstablishCmdInfo->peermac.bytes,
				     peerMac, QDF_MAC_ADDR_SIZE);
			tdlsLinkEstablishCmdInfo->isBufSta =
				tdlsLinkEstablishParams->isBufSta;
			tdlsLinkEstablishCmdInfo->isResponder =
				tdlsLinkEstablishParams->isResponder;
			tdlsLinkEstablishCmdInfo->maxSp =
				tdlsLinkEstablishParams->maxSp;
			tdlsLinkEstablishCmdInfo->uapsdQueues =
				tdlsLinkEstablishParams->uapsdQueues;
			tdlsLinkEstablishCmdInfo->isOffChannelSupported =
				tdlsLinkEstablishParams->isOffChannelSupported;
			cdf_mem_copy(tdlsLinkEstablishCmdInfo->
				     supportedChannels,
				     tdlsLinkEstablishParams->supportedChannels,
				     tdlsLinkEstablishParams->
				     supportedChannelsLen);
			tdlsLinkEstablishCmdInfo->supportedChannelsLen =
				tdlsLinkEstablishParams->supportedChannelsLen;
			cdf_mem_copy(tdlsLinkEstablishCmdInfo->
				     supportedOperClasses,
				     tdlsLinkEstablishParams->
				     supportedOperClasses,
				     tdlsLinkEstablishParams->
				     supportedOperClassesLen);
			tdlsLinkEstablishCmdInfo->supportedOperClassesLen =
				tdlsLinkEstablishParams->supportedOperClassesLen;
			tdlsLinkEstablishCmdInfo->isResponder =
				tdlsLinkEstablishParams->isResponder;
			tdlsLinkEstablishCmdInfo->maxSp =
				tdlsLinkEstablishParams->maxSp;
			tdlsLinkEstablishCmdInfo->uapsdQueues =
				tdlsLinkEstablishParams->uapsdQueues;
			tdlsLinkEstablishCmd->command =
				eSmeCommandTdlsLinkEstablish;
			tdlsLinkEstablishCmd->u.tdlsCmd.size =
				sizeof(tTdlsLinkEstablishCmdInfo);
			sme_push_command(pMac, tdlsLinkEstablishCmd, false);
			status = QDF_STATUS_SUCCESS;
		}
	}

	return status;
}

/*
 * TDLS request API, called from HDD to add a TDLS peer
 */
QDF_STATUS csr_tdls_add_peer_sta(tHalHandle hHal, uint8_t sessionId,
				 const tSirMacAddr peerMac)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSmeCmd *tdlsAddStaCmd;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	/* If connected and in Infra. Only then allow this */
	if (CSR_IS_SESSION_VALID(pMac, sessionId) &&
	    csr_is_conn_state_connected_infra(pMac, sessionId) &&
	    (NULL != peerMac)) {
		tdlsAddStaCmd = csr_get_command_buffer(pMac);

		if (tdlsAddStaCmd) {
			tTdlsAddStaCmdInfo *tdlsAddStaCmdInfo =
				&tdlsAddStaCmd->u.tdlsCmd.u.tdlsAddStaCmdInfo;

			tdlsAddStaCmd->sessionId = sessionId;
			tdlsAddStaCmdInfo->tdlsAddOper = TDLS_OPER_ADD;

			cdf_mem_copy(tdlsAddStaCmdInfo->peermac.bytes,
				     peerMac, QDF_MAC_ADDR_SIZE);

			tdlsAddStaCmd->command = eSmeCommandTdlsAddPeer;
			tdlsAddStaCmd->u.tdlsCmd.size =
				sizeof(tTdlsAddStaCmdInfo);
			sme_push_command(pMac, tdlsAddStaCmd, false);
			status = QDF_STATUS_SUCCESS;
		}
	}

	return status;
}

/*
 * TDLS request API, called from HDD to delete a TDLS peer
 */
QDF_STATUS csr_tdls_del_peer_sta(tHalHandle hHal, uint8_t sessionId,
				 const tSirMacAddr peerMac)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSmeCmd *tdlsDelStaCmd;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	/* If connected and in Infra. Only then allow this */
	if (CSR_IS_SESSION_VALID(pMac, sessionId) &&
	    csr_is_conn_state_connected_infra(pMac, sessionId) &&
	    (NULL != peerMac)) {
		tdlsDelStaCmd = csr_get_command_buffer(pMac);

		if (tdlsDelStaCmd) {
			tTdlsDelStaCmdInfo *tdlsDelStaCmdInfo =
				&tdlsDelStaCmd->u.tdlsCmd.u.tdlsDelStaCmdInfo;

			tdlsDelStaCmd->sessionId = sessionId;

			cdf_mem_copy(tdlsDelStaCmdInfo->peermac.bytes,
				     peerMac, QDF_MAC_ADDR_SIZE);

			tdlsDelStaCmd->command = eSmeCommandTdlsDelPeer;
			tdlsDelStaCmd->u.tdlsCmd.size =
				sizeof(tTdlsDelStaCmdInfo);
			sme_push_command(pMac, tdlsDelStaCmd, false);
			status = QDF_STATUS_SUCCESS;
		}
	}

	return status;
}

/*
 * TDLS messages sent to PE .
 */
QDF_STATUS tdls_send_message(tpAniSirGlobal pMac, uint16_t msg_type,
			     void *msg_data, uint32_t msg_size)
{

	tSirMbMsg *pMsg = (tSirMbMsg *) msg_data;
	pMsg->type = msg_type;
	pMsg->msgLen = (uint16_t) (msg_size);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  ("sending msg = %d"), pMsg->type);
	/* Send message. */
	if (cds_send_mb_message_to_mac(pMsg) != QDF_STATUS_SUCCESS) {
		sms_log(pMac, LOGE, FL("Cannot send message"));
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_tdls_process_send_mgmt(tpAniSirGlobal pMac, tSmeCmd *cmd)
{
	tTdlsSendMgmtCmdInfo *tdlsSendMgmtCmdInfo =
		&cmd->u.tdlsCmd.u.tdlsSendMgmtCmdInfo;
	tSirTdlsSendMgmtReq *tdlsSendMgmtReq = NULL;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, cmd->sessionId);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (NULL == pSession) {
		sms_log(pMac, LOGE, FL("pSession is NULL"));
		return QDF_STATUS_E_FAILURE;
	}
	if (NULL == pSession->pConnectBssDesc) {
		sms_log(pMac, LOGE, FL("BSS Description is not present"));
		return QDF_STATUS_E_FAILURE;
	}

	tdlsSendMgmtReq =
		cdf_mem_malloc(sizeof(tSirTdlsSendMgmtReq) +
			       tdlsSendMgmtCmdInfo->len);
	if (NULL == tdlsSendMgmtReq)
		status = QDF_STATUS_E_NOMEM;
	else
		status = QDF_STATUS_SUCCESS;

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, FL("alloc failed"));
		QDF_ASSERT(0);
		return status;
	}
	tdlsSendMgmtReq->sessionId = cmd->sessionId;
	/* Using dialog as transactionId. This can be used to match response with request */
	tdlsSendMgmtReq->transactionId = tdlsSendMgmtCmdInfo->dialog;
	tdlsSendMgmtReq->reqType = tdlsSendMgmtCmdInfo->frameType;
	tdlsSendMgmtReq->dialog = tdlsSendMgmtCmdInfo->dialog;
	tdlsSendMgmtReq->statusCode = tdlsSendMgmtCmdInfo->statusCode;
	tdlsSendMgmtReq->responder = tdlsSendMgmtCmdInfo->responder;
	tdlsSendMgmtReq->peerCapability = tdlsSendMgmtCmdInfo->peerCapability;

	cdf_mem_copy(tdlsSendMgmtReq->bssid.bytes,
		     pSession->pConnectBssDesc->bssId, QDF_MAC_ADDR_SIZE);

	cdf_mem_copy(tdlsSendMgmtReq->peer_mac.bytes,
		     tdlsSendMgmtCmdInfo->peerMac, QDF_MAC_ADDR_SIZE);

	if (tdlsSendMgmtCmdInfo->len && tdlsSendMgmtCmdInfo->buf) {
		cdf_mem_copy(tdlsSendMgmtReq->addIe, tdlsSendMgmtCmdInfo->buf,
			     tdlsSendMgmtCmdInfo->len);

	}
	/* Send the request to PE. */
	sms_log(pMac, LOG1, "sending TDLS Mgmt Frame req to PE ");
	status = tdls_send_message(pMac, eWNI_SME_TDLS_SEND_MGMT_REQ,
				   (void *)tdlsSendMgmtReq,
				   sizeof(tSirTdlsSendMgmtReq) +
				   tdlsSendMgmtCmdInfo->len);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, FL("Failed to send request to MAC"));
	}
	if (tdlsSendMgmtCmdInfo->len && tdlsSendMgmtCmdInfo->buf) {
		/* Done with the buf. Free it. */
		cdf_mem_free(tdlsSendMgmtCmdInfo->buf);
		tdlsSendMgmtCmdInfo->buf = NULL;
		tdlsSendMgmtCmdInfo->len = 0;
	}

	return status;
}

QDF_STATUS csr_tdls_process_add_sta(tpAniSirGlobal pMac, tSmeCmd *cmd)
{
	tTdlsAddStaCmdInfo *tdlsAddStaCmdInfo =
		&cmd->u.tdlsCmd.u.tdlsAddStaCmdInfo;
	tSirTdlsAddStaReq *tdlsAddStaReq = NULL;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, cmd->sessionId);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (NULL == pSession) {
		sms_log(pMac, LOGE, FL("pSession is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	if (NULL == pSession->pConnectBssDesc) {
		sms_log(pMac, LOGE, FL("BSS description is not present"));
		return QDF_STATUS_E_FAILURE;
	}

	tdlsAddStaReq = cdf_mem_malloc(sizeof(tSirTdlsAddStaReq));
	if (NULL == tdlsAddStaReq)
		status = QDF_STATUS_E_NOMEM;
	else
		status = QDF_STATUS_SUCCESS;

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, FL("alloc failed"));
		QDF_ASSERT(0);
		return status;
	}
	tdlsAddStaReq->sessionId = cmd->sessionId;
	tdlsAddStaReq->tdlsAddOper = tdlsAddStaCmdInfo->tdlsAddOper;
	/* Using dialog as transactionId. This can be used to match response with request */
	tdlsAddStaReq->transactionId = 0;

	cdf_mem_copy(tdlsAddStaReq->bssid.bytes,
		     pSession->pConnectBssDesc->bssId, QDF_MAC_ADDR_SIZE);

	qdf_copy_macaddr(&tdlsAddStaReq->peermac,
			 &tdlsAddStaCmdInfo->peermac);

	tdlsAddStaReq->capability = tdlsAddStaCmdInfo->capability;
	tdlsAddStaReq->uapsd_queues = tdlsAddStaCmdInfo->uapsdQueues;
	tdlsAddStaReq->max_sp = tdlsAddStaCmdInfo->maxSp;

	cdf_mem_copy(tdlsAddStaReq->extn_capability,
		     tdlsAddStaCmdInfo->extnCapability, SIR_MAC_MAX_EXTN_CAP);
	tdlsAddStaReq->htcap_present = tdlsAddStaCmdInfo->htcap_present;
	cdf_mem_copy(&tdlsAddStaReq->htCap,
		     &tdlsAddStaCmdInfo->HTCap,
		     sizeof(tdlsAddStaCmdInfo->HTCap));
	tdlsAddStaReq->vhtcap_present = tdlsAddStaCmdInfo->vhtcap_present;
	cdf_mem_copy(&tdlsAddStaReq->vhtCap,
		     &tdlsAddStaCmdInfo->VHTCap,
		     sizeof(tdlsAddStaCmdInfo->VHTCap));
	tdlsAddStaReq->supported_rates_length =
		tdlsAddStaCmdInfo->supportedRatesLen;
	cdf_mem_copy(&tdlsAddStaReq->supported_rates,
		     tdlsAddStaCmdInfo->supportedRates,
		     tdlsAddStaCmdInfo->supportedRatesLen);

	/* Send the request to PE. */
	sms_log(pMac, LOGE, "sending TDLS Add Sta req to PE ");
	status = tdls_send_message(pMac, eWNI_SME_TDLS_ADD_STA_REQ,
				   (void *)tdlsAddStaReq,
				   sizeof(tSirTdlsAddStaReq));
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, FL("Failed to send request to MAC"));
	}
	return status;
}

QDF_STATUS csr_tdls_process_del_sta(tpAniSirGlobal pMac, tSmeCmd *cmd)
{
	tTdlsDelStaCmdInfo *tdlsDelStaCmdInfo =
		&cmd->u.tdlsCmd.u.tdlsDelStaCmdInfo;
	tSirTdlsDelStaReq *tdlsDelStaReq = NULL;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, cmd->sessionId);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (NULL == pSession) {
		sms_log(pMac, LOGE, FL("pSession is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	if (NULL == pSession->pConnectBssDesc) {
		sms_log(pMac, LOGE, FL("BSS description is not present"));
		return QDF_STATUS_E_FAILURE;
	}

	tdlsDelStaReq = cdf_mem_malloc(sizeof(tSirTdlsDelStaReq));
	if (NULL == tdlsDelStaReq)
		status = QDF_STATUS_E_NOMEM;
	else
		status = QDF_STATUS_SUCCESS;

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, FL("alloc failed"));
		QDF_ASSERT(0);
		return status;
	}
	tdlsDelStaReq->sessionId = cmd->sessionId;
	/* Using dialog as transactionId. This can be used to match response with request */
	tdlsDelStaReq->transactionId = 0;

	cdf_mem_copy(tdlsDelStaReq->bssid.bytes,
		     pSession->pConnectBssDesc->bssId, QDF_MAC_ADDR_SIZE);

	qdf_copy_macaddr(&tdlsDelStaReq->peermac,
			 &tdlsDelStaCmdInfo->peermac);

	/* Send the request to PE. */
	sms_log(pMac, LOG1,
		"sending TDLS Del Sta " MAC_ADDRESS_STR " req to PE",
		MAC_ADDR_ARRAY(tdlsDelStaCmdInfo->peermac.bytes));
	status = tdls_send_message(pMac, eWNI_SME_TDLS_DEL_STA_REQ,
				   (void *)tdlsDelStaReq,
				   sizeof(tSirTdlsDelStaReq));
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, FL("Failed to send request to MAC"));
	}
	return status;
}

/*
 * commands received from CSR
 */
QDF_STATUS csr_tdls_process_cmd(tpAniSirGlobal pMac, tSmeCmd *cmd)
{
	eSmeCommandType cmdType = cmd->command;
	bool status = true;
	switch (cmdType) {
	case eSmeCommandTdlsSendMgmt:
	{
		status = csr_tdls_process_send_mgmt(pMac, cmd);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			status = false;
		}
	}
	break;
	case eSmeCommandTdlsAddPeer:
	{
		status = csr_tdls_process_add_sta(pMac, cmd);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			status = false;
		}
	}
	break;
	case eSmeCommandTdlsDelPeer:
	{
		status = csr_tdls_process_del_sta(pMac, cmd);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			status = false;
		}
	}
	break;
	case eSmeCommandTdlsLinkEstablish:
	{
		status = csr_tdls_process_link_establish(pMac, cmd);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			status = false;
		}
	}
	break;
	default:
	{
		/* TODO: Add defualt handling */
		break;
	}

	}
	return status;
}

QDF_STATUS csr_tdls_process_link_establish(tpAniSirGlobal pMac, tSmeCmd *cmd)
{
	tTdlsLinkEstablishCmdInfo *tdlsLinkEstablishCmdInfo =
		&cmd->u.tdlsCmd.u.tdlsLinkEstablishCmdInfo;
	tSirTdlsLinkEstablishReq *tdlsLinkEstablishReq = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, cmd->sessionId);

	if (NULL == pSession) {
		sms_log(pMac, LOGE, FL("pSession is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	tdlsLinkEstablishReq = cdf_mem_malloc(sizeof(tSirTdlsLinkEstablishReq));

	if (tdlsLinkEstablishReq == NULL) {
		sms_log(pMac, LOGE, FL("alloc failed"));
		QDF_ASSERT(0);
		return QDF_STATUS_E_NOMEM;
	}
	tdlsLinkEstablishReq->sessionId = cmd->sessionId;
	/* Using dialog as transactionId. This can be used to match response with request */
	tdlsLinkEstablishReq->transactionId = 0;
	qdf_copy_macaddr(&tdlsLinkEstablishReq->peermac,
			 &tdlsLinkEstablishCmdInfo->peermac);
	cdf_mem_copy(tdlsLinkEstablishReq->bssid.bytes,
		     pSession->pConnectBssDesc->bssId, QDF_MAC_ADDR_SIZE);
	cdf_mem_copy(tdlsLinkEstablishReq->supportedChannels,
		     tdlsLinkEstablishCmdInfo->supportedChannels,
		     tdlsLinkEstablishCmdInfo->supportedChannelsLen);
	tdlsLinkEstablishReq->supportedChannelsLen =
		tdlsLinkEstablishCmdInfo->supportedChannelsLen;
	cdf_mem_copy(tdlsLinkEstablishReq->supportedOperClasses,
		     tdlsLinkEstablishCmdInfo->supportedOperClasses,
		     tdlsLinkEstablishCmdInfo->supportedOperClassesLen);
	tdlsLinkEstablishReq->supportedOperClassesLen =
		tdlsLinkEstablishCmdInfo->supportedOperClassesLen;
	tdlsLinkEstablishReq->isBufSta = tdlsLinkEstablishCmdInfo->isBufSta;
	tdlsLinkEstablishReq->isResponder =
		tdlsLinkEstablishCmdInfo->isResponder;
	tdlsLinkEstablishReq->uapsdQueues =
		tdlsLinkEstablishCmdInfo->uapsdQueues;
	tdlsLinkEstablishReq->maxSp = tdlsLinkEstablishCmdInfo->maxSp;

	/* Send the request to PE. */
	sms_log(pMac, LOGE, "sending TDLS Link Establish Request to PE \n");
	status = tdls_send_message(pMac, eWNI_SME_TDLS_LINK_ESTABLISH_REQ,
				   (void *)tdlsLinkEstablishReq,
				   sizeof(tSirTdlsLinkEstablishReq));
				   if (!QDF_IS_STATUS_SUCCESS(status)) {
					   sms_log(pMac, LOGE, FL("Failed to send request to MAC\n"));
				   }
				   return status;
}

/*
 * TDLS Message processor, will be called after TDLS message recieved from
 * PE
 */
QDF_STATUS tdls_msg_processor(tpAniSirGlobal pMac, uint16_t msgType,
			      void *pMsgBuf)
{
	tCsrRoamInfo roamInfo = { 0 };
	eCsrRoamResult roamResult;
	tSirTdlsAddStaRsp *addStaRsp = (tSirTdlsAddStaRsp *) pMsgBuf;
	tSirTdlsDelStaRsp *delStaRsp = (tSirTdlsDelStaRsp *) pMsgBuf;
	tpSirTdlsDelStaInd pSirTdlsDelStaInd = (tpSirTdlsDelStaInd) pMsgBuf;
	tpSirTdlsDelAllPeerInd pSirTdlsDelAllPeerInd =
		(tpSirTdlsDelAllPeerInd) pMsgBuf;
	tpSirMgmtTxCompletionInd tdls_tx_comp_ind =
		(tpSirMgmtTxCompletionInd) pMsgBuf;
	tSirTdlsLinkEstablishReqRsp *linkEstablishReqRsp =
		(tSirTdlsLinkEstablishReqRsp *) pMsgBuf;
	tSirTdlsEventnotify *tevent = (tSirTdlsEventnotify *) pMsgBuf;

	switch (msgType) {
	case eWNI_SME_TDLS_SEND_MGMT_RSP:
		/* remove pending eSmeCommandTdlsDiscovery command */
		csr_tdls_remove_sme_cmd(pMac, eSmeCommandTdlsSendMgmt);
		break;
	case eWNI_SME_TDLS_ADD_STA_RSP:
		qdf_copy_macaddr(&roamInfo.peerMac, &addStaRsp->peermac);
		roamInfo.staId = addStaRsp->staId;
		roamInfo.ucastSig = addStaRsp->ucastSig;
		roamInfo.bcastSig = addStaRsp->bcastSig;
		roamInfo.statusCode = addStaRsp->statusCode;
		/*
		 * register peer with TL, we have to go through HDD as
		 * this is the only way to register any STA with TL.
		 */
		if (addStaRsp->tdlsAddOper == TDLS_OPER_ADD)
			roamResult = eCSR_ROAM_RESULT_ADD_TDLS_PEER;
		else    /* addStaRsp->tdlsAddOper must be TDLS_OPER_UPDATE */
			roamResult = eCSR_ROAM_RESULT_UPDATE_TDLS_PEER;
		csr_roam_call_callback(pMac, addStaRsp->sessionId,
				&roamInfo, 0, eCSR_ROAM_TDLS_STATUS_UPDATE,
				roamResult);

		/* remove pending eSmeCommandTdlsDiscovery command */
		csr_tdls_remove_sme_cmd(pMac, eSmeCommandTdlsAddPeer);
		break;
	case eWNI_SME_TDLS_DEL_STA_RSP:
		qdf_copy_macaddr(&roamInfo.peerMac, &delStaRsp->peermac);
		roamInfo.staId = delStaRsp->staId;
		roamInfo.statusCode = delStaRsp->statusCode;
		/*
		 * register peer with TL, we have to go through HDD as
		 * this is the only way to register any STA with TL.
		 */
		csr_roam_call_callback(pMac, delStaRsp->sessionId,
				&roamInfo, 0,
				eCSR_ROAM_TDLS_STATUS_UPDATE,
				eCSR_ROAM_RESULT_DELETE_TDLS_PEER);

		csr_tdls_remove_sme_cmd(pMac, eSmeCommandTdlsDelPeer);
		break;
	case eWNI_SME_TDLS_DEL_STA_IND:
		qdf_copy_macaddr(&roamInfo.peerMac,
				 &pSirTdlsDelStaInd->peermac);
		roamInfo.staId = pSirTdlsDelStaInd->staId;
		roamInfo.reasonCode = pSirTdlsDelStaInd->reasonCode;

		/* Sending the TEARDOWN indication to HDD. */
		csr_roam_call_callback(pMac,
				pSirTdlsDelStaInd->sessionId, &roamInfo,
				0, eCSR_ROAM_TDLS_STATUS_UPDATE,
				eCSR_ROAM_RESULT_TEARDOWN_TDLS_PEER_IND);
		break;
	case eWNI_SME_TDLS_DEL_ALL_PEER_IND:
		/* Sending the TEARDOWN indication to HDD. */
		csr_roam_call_callback(pMac,
				pSirTdlsDelAllPeerInd->sessionId,
				&roamInfo, 0,
				eCSR_ROAM_TDLS_STATUS_UPDATE,
				eCSR_ROAM_RESULT_DELETE_ALL_TDLS_PEER_IND);
		break;
	case eWNI_SME_MGMT_FRM_TX_COMPLETION_IND:
		roamInfo.reasonCode =
			tdls_tx_comp_ind->txCompleteStatus;

		csr_roam_call_callback(pMac,
				tdls_tx_comp_ind->sessionId,
				&roamInfo, 0,
				eCSR_ROAM_RESULT_MGMT_TX_COMPLETE_IND,
				0);
		break;
	case eWNI_SME_TDLS_LINK_ESTABLISH_RSP:
		csr_roam_call_callback(pMac,
				linkEstablishReqRsp->sessionId,
				&roamInfo, 0,
				eCSR_ROAM_TDLS_STATUS_UPDATE,
				eCSR_ROAM_RESULT_LINK_ESTABLISH_REQ_RSP);
		/* remove pending eSmeCommandTdlsLinkEstablish command */
		csr_tdls_remove_sme_cmd(pMac, eSmeCommandTdlsLinkEstablish);
		break;
	case eWNI_SME_TDLS_SHOULD_DISCOVER:
		qdf_copy_macaddr(&roamInfo.peerMac, &tevent->peermac);
		roamInfo.reasonCode = tevent->peer_reason;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				"%s: eWNI_SME_TDLS_SHOULD_DISCOVER for peer mac: "
				MAC_ADDRESS_STR " peer_reason: %d",
				__func__, MAC_ADDR_ARRAY(tevent->peermac.bytes),
				tevent->peer_reason);
		csr_roam_call_callback(pMac, tevent->sessionId, &roamInfo,
				0, eCSR_ROAM_TDLS_STATUS_UPDATE,
				eCSR_ROAM_RESULT_TDLS_SHOULD_DISCOVER);
		break;
	case eWNI_SME_TDLS_SHOULD_TEARDOWN:
		qdf_copy_macaddr(&roamInfo.peerMac, &tevent->peermac);
		roamInfo.reasonCode = tevent->peer_reason;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				"%s: eWNI_SME_TDLS_SHOULD_TEARDOWN for peer mac: "
				MAC_ADDRESS_STR " peer_reason: %d",
				__func__, MAC_ADDR_ARRAY(tevent->peermac.bytes),
				tevent->peer_reason);
		csr_roam_call_callback(pMac, tevent->sessionId, &roamInfo,
				0, eCSR_ROAM_TDLS_STATUS_UPDATE,
				eCSR_ROAM_RESULT_TDLS_SHOULD_TEARDOWN);
		break;
	case eWNI_SME_TDLS_PEER_DISCONNECTED:
		qdf_copy_macaddr(&roamInfo.peerMac, &tevent->peermac);
		roamInfo.reasonCode = tevent->peer_reason;
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				"%s: eWNI_SME_TDLS_PEER_DISCONNECTED for peer mac: "
				MAC_ADDRESS_STR " peer_reason: %d",
				__func__, MAC_ADDR_ARRAY(tevent->peermac.bytes),
				tevent->peer_reason);
		csr_roam_call_callback(pMac, tevent->sessionId, &roamInfo,
				0, eCSR_ROAM_TDLS_STATUS_UPDATE,
				eCSR_ROAM_RESULT_TDLS_SHOULD_PEER_DISCONNECTED);
		break;
	default:
		break;
	}

	return QDF_STATUS_SUCCESS;
}
#endif
