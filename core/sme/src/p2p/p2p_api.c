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

#include "sme_api.h"
#include "sms_debug.h"
#include "csr_inside_api.h"
#include "sme_inside.h"
#include "p2p_api.h"
#include "cfg_api.h"
#include "wma.h"

/*------------------------------------------------------------------
 *
 * handle SME remain on channel request.
 *
 *------------------------------------------------------------------*/

QDF_STATUS p2p_process_remain_on_channel_cmd(tpAniSirGlobal pMac,
					     tSmeCmd *p2pRemainonChn)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tSirRemainOnChnReq *pMsg;
	uint32_t len;
	tCsrRoamSession *pSession =
		CSR_GET_SESSION(pMac, p2pRemainonChn->sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("  session %d not found "),
			p2pRemainonChn->sessionId);
		goto error;
	}

	if (!pSession->sessionActive) {
		sms_log(pMac, LOGE,
			FL("  session %d is invalid or listen is disabled "),
			p2pRemainonChn->sessionId);
		goto error;
	}
	len = sizeof(tSirRemainOnChnReq) + pMac->p2pContext.probeRspIeLength;

	if (len > 0xFFFF) {
		/*In coming len for Msg is more then 16bit value */
		sms_log(pMac, LOGE, FL("  Message length is very large, %d"),
			len);
		goto error;
	}

	pMsg = qdf_mem_malloc(len);
	if (NULL == pMsg)
		goto error;
	else {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO, "%s call",
			  __func__);
		pMsg->messageType = eWNI_SME_REMAIN_ON_CHANNEL_REQ;
		pMsg->length = (uint16_t) len;
		qdf_copy_macaddr(&pMsg->selfMacAddr, &pSession->selfMacAddr);
		pMsg->chnNum = p2pRemainonChn->u.remainChlCmd.chn;
		pMsg->phyMode = p2pRemainonChn->u.remainChlCmd.phyMode;
		pMsg->duration = p2pRemainonChn->u.remainChlCmd.duration;
		pMsg->sessionId = p2pRemainonChn->sessionId;
		pMsg->isProbeRequestAllowed =
			p2pRemainonChn->u.remainChlCmd.isP2PProbeReqAllowed;
		pMsg->scan_id = p2pRemainonChn->u.remainChlCmd.scan_id;
		if (pMac->p2pContext.probeRspIeLength)
			qdf_mem_copy((void *)pMsg->probeRspIe,
				     (void *)pMac->p2pContext.probeRspIe,
				     pMac->p2pContext.probeRspIeLength);
		status = cds_send_mb_message_to_mac(pMsg);
	}
error:
	if (QDF_STATUS_E_FAILURE == status)
		csr_release_roc_req_cmd(pMac);
	return status;
}

/*------------------------------------------------------------------
 *
 * handle LIM remain on channel rsp: Success/failure.
 *
 *------------------------------------------------------------------*/

QDF_STATUS sme_remain_on_chn_rsp(tpAniSirGlobal pMac, uint8_t *pMsg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tListElem *pEntry = NULL;
	tSmeCmd *pCommand = NULL;
	bool fFound;
	remainOnChanCallback callback = NULL;
	struct sir_roc_rsp *rsp = (struct sir_roc_rsp *)pMsg;

	csr_get_active_scan_entry(pMac, rsp->scan_id, &pEntry);
	if (!pEntry) {
		sms_log(pMac, LOGE, FL("No cmd found in active list."));
		return status;
	}

	pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
	if (eSmeCommandRemainOnChannel != pCommand->command)
		return status;

	callback = pCommand->u.remainChlCmd.callback;
	if (callback)
		callback(pMac, pCommand->u.remainChlCmd.callbackCtx,
			rsp->status, rsp->scan_id);

	fFound = csr_ll_remove_entry(&pMac->sme.smeScanCmdActiveList, pEntry,
				     LL_ACCESS_LOCK);
	if (fFound) {
		/* Now put this command back on the avilable command list */
		sme_release_command(pMac, pCommand);
	}
	sme_process_pending_queue(pMac);
	return status;
}

/*------------------------------------------------------------------
 *
 * Handle the remain on channel ready indication from PE
 *
 *------------------------------------------------------------------*/

QDF_STATUS sme_remain_on_chn_ready(tHalHandle hHal, uint8_t *pMsg)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tListElem *pEntry = NULL;
	tSmeCmd *pCommand = NULL;
	tCsrRoamInfo RoamInfo;
	struct sir_roc_rsp *rsp =  (struct sir_roc_rsp *)pMsg;

	csr_get_active_scan_entry(pMac, rsp->scan_id, &pEntry);
	if (!pEntry) {
		sms_log(pMac, LOGE, FL("No cmd found in active list."));
		return status;
	}
	sms_log(pMac, LOG1,
		FL("Ready Ind %d %d"), rsp->session_id, rsp->scan_id);
	pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
	if (eSmeCommandRemainOnChannel == pCommand->command) {
		RoamInfo.roc_scan_id = rsp->scan_id;
		/* forward the indication to HDD */
		RoamInfo.pRemainCtx = pCommand->u.remainChlCmd.callbackCtx;
		csr_roam_call_callback(pMac, rsp->session_id,
				       &RoamInfo, 0,
				       eCSR_ROAM_REMAIN_CHAN_READY, 0);
	}

	return status;
}

QDF_STATUS sme_p2p_open(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* If static structure is too big, Need to change this function to allocate memory dynamically */
	qdf_mem_zero(&pMac->p2pContext, sizeof(tp2pContext));

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_p2p_close(hHal);
	}

	return status;
}

QDF_STATUS p2p_stop(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (pMac->p2pContext.probeRspIe) {
		qdf_mem_free(pMac->p2pContext.probeRspIe);
		pMac->p2pContext.probeRspIe = NULL;
	}

	pMac->p2pContext.probeRspIeLength = 0;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS sme_p2p_close(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (pMac->p2pContext.probeRspIe) {
		qdf_mem_free(pMac->p2pContext.probeRspIe);
		pMac->p2pContext.probeRspIe = NULL;
	}

	pMac->p2pContext.probeRspIeLength = 0;

	return QDF_STATUS_SUCCESS;
}

tSirRFBand get_rf_band(uint8_t channel)
{
	if ((channel >= SIR_11A_CHANNEL_BEGIN) &&
	    (channel <= SIR_11A_CHANNEL_END))
		return SIR_BAND_5_GHZ;

	if ((channel >= SIR_11B_CHANNEL_BEGIN) &&
	    (channel <= SIR_11B_CHANNEL_END))
		return SIR_BAND_2_4_GHZ;

	return SIR_BAND_UNKNOWN;
}

/* ---------------------------------------------------------------------------

    \fn p2p_remain_on_channel
    \brief  API to post the remain on channel command.
    \param  hHal - The handle returned by mac_open.
    \param  sessinId - HDD session ID.
    \param  channel - Channel to remain on channel.
    \param  duration - Duration for which we should remain on channel
    \param  callback - callback function.
    \param  pContext - argument to the callback function
    \return QDF_STATUS

   -------------------------------------------------------------------------------*/
QDF_STATUS p2p_remain_on_channel(tHalHandle hHal, uint8_t sessionId,
				 uint8_t channel, uint32_t duration,
				 remainOnChanCallback callback,
				 void *pContext, uint8_t isP2PProbeReqAllowed,
				 uint32_t scan_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tSmeCmd *pRemainChlCmd = NULL;
	uint32_t phyMode;

	pRemainChlCmd = sme_get_command_buffer(pMac);
	if (pRemainChlCmd == NULL)
		return QDF_STATUS_E_FAILURE;

	if (SIR_BAND_5_GHZ == get_rf_band(channel)) {
		phyMode = WNI_CFG_PHY_MODE_11A;
	} else {
		phyMode = WNI_CFG_PHY_MODE_11G;
	}

	cfg_set_int(pMac, WNI_CFG_PHY_MODE, phyMode);

	do {
		/* call set in context */
		pRemainChlCmd->command = eSmeCommandRemainOnChannel;
		pRemainChlCmd->sessionId = sessionId;
		pRemainChlCmd->u.remainChlCmd.chn = channel;
		pRemainChlCmd->u.remainChlCmd.duration = duration;
		pRemainChlCmd->u.remainChlCmd.isP2PProbeReqAllowed =
			isP2PProbeReqAllowed;
		pRemainChlCmd->u.remainChlCmd.callback = callback;
		pRemainChlCmd->u.remainChlCmd.callbackCtx = pContext;
		pRemainChlCmd->u.remainChlCmd.scan_id = scan_id;

		/* Put it at the head of the Q if we just finish finding the peer and ready to send a frame */
		status = csr_queue_sme_command(pMac, pRemainChlCmd, false);
	} while (0);

	sms_log(pMac, LOGW, "exiting function %s", __func__);

	return status;
}

QDF_STATUS p2p_send_action(tHalHandle hHal, uint8_t sessionId,
			   const uint8_t *pBuf, uint32_t len, uint16_t wait,
			   bool noack, uint16_t channel_freq)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirMbMsgP2p *pMsg;
	uint16_t msgLen;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_MED,
		  " %s sends action frame", __func__);
	msgLen = (uint16_t) ((sizeof(tSirMbMsgP2p)) + len);
	pMsg = qdf_mem_malloc(msgLen);
	if (NULL == pMsg)
		status = QDF_STATUS_E_NOMEM;
	else {
		pMsg->type = eWNI_SME_SEND_ACTION_FRAME_IND;
		pMsg->msgLen = msgLen;
		pMsg->sessionId = sessionId;
		pMsg->noack = noack;
		pMsg->channel_freq = channel_freq;
		pMsg->wait = (uint16_t) wait;
		qdf_mem_copy(pMsg->data, pBuf, len);
		status = cds_send_mb_message_to_mac(pMsg);
	}
	return status;
}

QDF_STATUS p2p_cancel_remain_on_channel(tHalHandle hHal,
	uint8_t sessionId, uint32_t scan_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirMbMsgP2p *pMsg;
	uint16_t msgLen;

	/* Need to check session ID to support concurrency */

	msgLen = (uint16_t) (sizeof(tSirMbMsgP2p));
	pMsg = qdf_mem_malloc(msgLen);
	if (NULL == pMsg)
		status = QDF_STATUS_E_NOMEM;
	else {
		qdf_mem_set((void *)pMsg, msgLen, 0);
		pMsg->type = eWNI_SME_ABORT_REMAIN_ON_CHAN_IND;
		pMsg->msgLen = msgLen;
		pMsg->sessionId = sessionId;
		pMsg->scan_id = scan_id;
		status = cds_send_mb_message_to_mac(pMsg);
	}

	return status;
}

QDF_STATUS p2p_set_ps(tHalHandle hHal, tP2pPsConfig *pNoA)
{
	tpP2pPsConfig pNoAParam;
	tSirMsgQ msg;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	pNoAParam = qdf_mem_malloc(sizeof(tP2pPsConfig));
	if (NULL == pNoAParam)
		status = QDF_STATUS_E_NOMEM;
	else {
		qdf_mem_copy(pNoAParam, pNoA, sizeof(tP2pPsConfig));
		msg.type = eWNI_SME_UPDATE_NOA;
		msg.bodyval = 0;
		msg.bodyptr = pNoAParam;
		lim_post_msg_api(pMac, &msg);
	}
	return status;
}
