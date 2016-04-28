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

#include "htc_debug.h"
#include "htc_internal.h"
#include <hif.h>
#include <qdf_nbuf.h>           /* qdf_nbuf_t */

/* use credit flow control over HTC */
unsigned int htc_credit_flow = 1;
#ifndef DEBUG_CREDIT
#define DEBUG_CREDIT 0
#endif

/* HTC credit flow global disable */
void htc_global_credit_flow_disable(void)
{
	htc_credit_flow = 0;
}

/* HTC credit flow global enable */
void htc_global_credit_flow_enable(void)
{
	htc_credit_flow = 1;
}

#ifdef HIF_SDIO

/**
 * htc_alt_data_credit_size_update() - update tx credit size info
 *				on max bundle size
 * @target: hif context
 * @ul_pipe: endpoint ul pipe id
 * @dl_pipe: endpoint dl pipe id
 * @txCreditSize: endpoint tx credit size
 *
 *
 * When AltDataCreditSize is non zero, it indicates the credit size for
 * HTT and all other services on Mbox0. Mbox1 has WMI_CONTROL_SVC which
 * uses the default credit size. Use AltDataCreditSize only when
 * mailbox is swapped. Mailbox swap bit is set by bmi_target_ready at
 * the end of BMI phase.
 *
 * The Credit Size is a parameter associated with the mbox rather than
 * a service. Multiple services can run on this mbox.
 *
 * If AltDataCreditSize is 0, that means the firmware doesn't support
 * this feature. Default to the TargetCreditSize
 *
 * Return: None
 */
static inline void
htc_alt_data_credit_size_update(HTC_TARGET *target,
				uint8_t *ul_pipe,
				uint8_t *dl_pipe,
				int *txCreditSize)
{
	if ((target->AltDataCreditSize) &&
	    (*ul_pipe == 1) && (*dl_pipe == 0))
		*txCreditSize = target->AltDataCreditSize;

	return;
}
#else

static inline void
htc_alt_data_credit_size_update(HTC_TARGET *target,
				uint8_t *ul_pipe,
				uint8_t *dl_pipe,
				int *txCreditSize)
{
	return;
}
#endif

A_STATUS htc_connect_service(HTC_HANDLE HTCHandle,
			     HTC_SERVICE_CONNECT_REQ *pConnectReq,
			     HTC_SERVICE_CONNECT_RESP *pConnectResp)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	A_STATUS status = A_OK;
	HTC_PACKET *pSendPacket = NULL;
	HTC_CONNECT_SERVICE_RESPONSE_MSG *pResponseMsg;
	HTC_CONNECT_SERVICE_MSG *pConnectMsg;
	HTC_ENDPOINT_ID assignedEndpoint = ENDPOINT_MAX;
	HTC_ENDPOINT *pEndpoint;
	unsigned int maxMsgSize = 0;
	qdf_nbuf_t netbuf;
	uint8_t txAlloc;
	int length;
	bool disableCreditFlowCtrl = false;
	uint16_t conn_flags;
	uint16_t rsp_msg_id, rsp_msg_serv_id, rsp_msg_max_msg_size;
	uint8_t rsp_msg_status, rsp_msg_end_id, rsp_msg_serv_meta_len;

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC,
			("+htc_connect_service, target:%p SvcID:0x%X\n", target,
			 pConnectReq->service_id));

	do {

		AR_DEBUG_ASSERT(pConnectReq->service_id != 0);

		if (HTC_CTRL_RSVD_SVC == pConnectReq->service_id) {
			/* special case for pseudo control service */
			assignedEndpoint = ENDPOINT_0;
			maxMsgSize = HTC_MAX_CONTROL_MESSAGE_LENGTH;
			txAlloc = 0;

		} else {

			txAlloc = htc_get_credit_allocation(target,
					pConnectReq->service_id);

			if (!txAlloc) {
				AR_DEBUG_PRINTF(ATH_DEBUG_TRC,
						("Service %d does not allocate target credits!\n",
						 pConnectReq->service_id));
			}

			/* allocate a packet to send to the target */
			pSendPacket = htc_alloc_control_tx_packet(target);

			if (NULL == pSendPacket) {
				AR_DEBUG_ASSERT(false);
				status = A_NO_MEMORY;
				break;
			}

			netbuf =
				(qdf_nbuf_t)
				GET_HTC_PACKET_NET_BUF_CONTEXT(pSendPacket);
			length =
				sizeof(HTC_CONNECT_SERVICE_MSG) +
				pConnectReq->MetaDataLength;

			/* assemble connect service message */
			qdf_nbuf_put_tail(netbuf, length);
			pConnectMsg =
			    (HTC_CONNECT_SERVICE_MSG *) qdf_nbuf_data(netbuf);

			if (NULL == pConnectMsg) {
				AR_DEBUG_ASSERT(0);
				status = A_EFAULT;
				break;
			}

			qdf_mem_zero(pConnectMsg,
				     sizeof(HTC_CONNECT_SERVICE_MSG));

			conn_flags =
				(pConnectReq->
				 ConnectionFlags & ~HTC_SET_RECV_ALLOC_MASK) |
				HTC_CONNECT_FLAGS_SET_RECV_ALLOCATION(txAlloc);
			HTC_SET_FIELD(pConnectMsg, HTC_CONNECT_SERVICE_MSG,
				      MESSAGEID, HTC_MSG_CONNECT_SERVICE_ID);
			HTC_SET_FIELD(pConnectMsg, HTC_CONNECT_SERVICE_MSG,
				      SERVICE_ID, pConnectReq->service_id);
			HTC_SET_FIELD(pConnectMsg, HTC_CONNECT_SERVICE_MSG,
				      CONNECTIONFLAGS, conn_flags);

			if (pConnectReq->
			    ConnectionFlags &
			    HTC_CONNECT_FLAGS_DISABLE_CREDIT_FLOW_CTRL) {
				disableCreditFlowCtrl = true;
			}

			if (!htc_credit_flow) {
				disableCreditFlowCtrl = true;
			}

			/* check caller if it wants to transfer meta data */
			if ((pConnectReq->pMetaData != NULL) &&
			    (pConnectReq->MetaDataLength <=
			     HTC_SERVICE_META_DATA_MAX_LENGTH)) {
				/* copy meta data into message buffer (after header ) */
				qdf_mem_copy((uint8_t *) pConnectMsg +
					 sizeof(HTC_CONNECT_SERVICE_MSG),
					 pConnectReq->pMetaData,
					 pConnectReq->MetaDataLength);

				HTC_SET_FIELD(pConnectMsg,
					      HTC_CONNECT_SERVICE_MSG,
					      SERVICEMETALENGTH,
					      pConnectReq->MetaDataLength);
			}

			SET_HTC_PACKET_INFO_TX(pSendPacket,
					       NULL,
					       (uint8_t *) pConnectMsg,
					       length,
					       ENDPOINT_0,
					       HTC_SERVICE_TX_PACKET_TAG);

			status = htc_send_pkt((HTC_HANDLE) target, pSendPacket);
			/* we don't own it anymore */
			pSendPacket = NULL;
			if (A_FAILED(status)) {
				break;
			}

			/* wait for response */
			status = htc_wait_recv_ctrl_message(target);
			if (A_FAILED(status)) {
				break;
			}
			/* we controlled the buffer creation so it has to be properly aligned */
			pResponseMsg =
				(HTC_CONNECT_SERVICE_RESPONSE_MSG *) target->
				CtrlResponseBuffer;

			rsp_msg_id = HTC_GET_FIELD(pResponseMsg,
						   HTC_CONNECT_SERVICE_RESPONSE_MSG,
						   MESSAGEID);
			rsp_msg_serv_id =
				HTC_GET_FIELD(pResponseMsg,
					      HTC_CONNECT_SERVICE_RESPONSE_MSG,
					      SERVICEID);
			rsp_msg_status =
				HTC_GET_FIELD(pResponseMsg,
					      HTC_CONNECT_SERVICE_RESPONSE_MSG,
					      STATUS);
			rsp_msg_end_id =
				HTC_GET_FIELD(pResponseMsg,
					      HTC_CONNECT_SERVICE_RESPONSE_MSG,
					      ENDPOINTID);
			rsp_msg_max_msg_size =
				HTC_GET_FIELD(pResponseMsg,
					      HTC_CONNECT_SERVICE_RESPONSE_MSG,
					      MAXMSGSIZE);
			rsp_msg_serv_meta_len =
				HTC_GET_FIELD(pResponseMsg,
					      HTC_CONNECT_SERVICE_RESPONSE_MSG,
					      SERVICEMETALENGTH);

			if ((rsp_msg_id != HTC_MSG_CONNECT_SERVICE_RESPONSE_ID)
			    || (target->CtrlResponseLength <
				sizeof(HTC_CONNECT_SERVICE_RESPONSE_MSG))) {
				/* this message is not valid */
				AR_DEBUG_ASSERT(false);
				status = A_EPROTO;
				break;
			}

			AR_DEBUG_PRINTF(ATH_DEBUG_TRC,
					("htc_connect_service, service 0x%X connect response from target status:%d, assigned ep: %d\n",
					 rsp_msg_serv_id, rsp_msg_status,
					 rsp_msg_end_id));

			pConnectResp->ConnectRespCode = rsp_msg_status;

			/* check response status */
			if (rsp_msg_status != HTC_SERVICE_SUCCESS) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
						(" Target failed service 0x%X connect request (status:%d)\n",
						 rsp_msg_serv_id,
						 rsp_msg_status));
				status = A_EPROTO;
/* TODO: restore the ifdef when FW supports services 301 and 302 (HTT_MSG_DATA[23]_MSG_SVC)
#ifdef QCA_TX_HTT2_SUPPORT
*/
				/* Keep work and not to block the control message. */
				target->CtrlResponseProcessing = false;
/*#endif */ /* QCA_TX_HTT2_SUPPORT */
				break;
			}

			assignedEndpoint = (HTC_ENDPOINT_ID) rsp_msg_end_id;
			maxMsgSize = rsp_msg_max_msg_size;

			if ((pConnectResp->pMetaData != NULL) &&
			    (rsp_msg_serv_meta_len > 0) &&
			    (rsp_msg_serv_meta_len <=
			     HTC_SERVICE_META_DATA_MAX_LENGTH)) {
				/* caller supplied a buffer and the target responded with data */
				int copyLength =
					min((int)pConnectResp->BufferLength,
					    (int)rsp_msg_serv_meta_len);
				/* copy the meta data */
				qdf_mem_copy(pConnectResp->pMetaData,
					 ((uint8_t *) pResponseMsg) +
					 sizeof
					 (HTC_CONNECT_SERVICE_RESPONSE_MSG),
					 copyLength);
				pConnectResp->ActualLength = copyLength;
			}
			/* done processing response buffer */
			target->CtrlResponseProcessing = false;
		}

		/* the rest of these are parameter checks so set the error status */
		status = A_EPROTO;

		if (assignedEndpoint >= ENDPOINT_MAX) {
			AR_DEBUG_ASSERT(false);
			break;
		}

		if (0 == maxMsgSize) {
			AR_DEBUG_ASSERT(false);
			break;
		}

		pEndpoint = &target->endpoint[assignedEndpoint];
		pEndpoint->Id = assignedEndpoint;
		if (pEndpoint->service_id != 0) {
			/* endpoint already in use! */
			AR_DEBUG_ASSERT(false);
			break;
		}

		/* return assigned endpoint to caller */
		pConnectResp->Endpoint = assignedEndpoint;
		pConnectResp->MaxMsgLength = maxMsgSize;

		/* setup the endpoint */
		/* service_id marks the endpoint in use */
		pEndpoint->service_id = pConnectReq->service_id;
		pEndpoint->MaxTxQueueDepth = pConnectReq->MaxSendQueueDepth;
		pEndpoint->MaxMsgLength = maxMsgSize;
		pEndpoint->TxCredits = txAlloc;
		pEndpoint->TxCreditSize = target->TargetCreditSize;
		pEndpoint->TxCreditsPerMaxMsg =
			maxMsgSize / target->TargetCreditSize;
		if (maxMsgSize % target->TargetCreditSize) {
			pEndpoint->TxCreditsPerMaxMsg++;
		}
#if DEBUG_CREDIT
		qdf_print(" Endpoint%d initial credit:%d, size:%d.\n",
			  pEndpoint->Id, pEndpoint->TxCredits,
			  pEndpoint->TxCreditSize);
#endif

		/* copy all the callbacks */
		pEndpoint->EpCallBacks = pConnectReq->EpCallbacks;

		status = hif_map_service_to_pipe(target->hif_dev,
						 pEndpoint->service_id,
						 &pEndpoint->UL_PipeID,
						 &pEndpoint->DL_PipeID,
						 &pEndpoint->ul_is_polled,
						 &pEndpoint->dl_is_polled);
		if (A_FAILED(status)) {
			break;
		}

		htc_alt_data_credit_size_update(target,
						&pEndpoint->UL_PipeID,
						&pEndpoint->DL_PipeID,
						&pEndpoint->TxCreditSize);

		qdf_assert(!pEndpoint->dl_is_polled);   /* not currently supported */

		if (pEndpoint->ul_is_polled) {
			qdf_timer_init(target->osdev,
				&pEndpoint->ul_poll_timer,
				htc_send_complete_check_cleanup,
				pEndpoint,
				QDF_TIMER_TYPE_SW);
		}

		AR_DEBUG_PRINTF(ATH_DEBUG_SETUP,
				("HTC Service:0x%4.4X, ULpipe:%d DLpipe:%d id:%d Ready\n",
				 pEndpoint->service_id, pEndpoint->UL_PipeID,
				 pEndpoint->DL_PipeID, pEndpoint->Id));

		if (disableCreditFlowCtrl && pEndpoint->TxCreditFlowEnabled) {
			pEndpoint->TxCreditFlowEnabled = false;
			AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
					("HTC Service:0x%4.4X ep:%d TX flow control disabled\n",
					 pEndpoint->service_id,
					 assignedEndpoint));
		}

	} while (false);

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("-htc_connect_service\n"));

	return status;
}

void htc_set_credit_distribution(HTC_HANDLE HTCHandle,
				 void *pCreditDistContext,
				 HTC_CREDIT_DIST_CALLBACK CreditDistFunc,
				 HTC_CREDIT_INIT_CALLBACK CreditInitFunc,
				 HTC_SERVICE_ID ServicePriorityOrder[],
				 int ListLength)
{
	/* NOT Supported, this transport does not use a credit based flow control mechanism */

}

void htc_fw_event_handler(void *context, QDF_STATUS status)
{
	HTC_TARGET *target = (HTC_TARGET *) context;
	HTC_INIT_INFO *initInfo = &target->HTCInitInfo;

	/* check if target failure handler exists and pass error code to it. */
	if (target->HTCInitInfo.TargetFailure != NULL) {
		initInfo->TargetFailure(initInfo->pContext, status);
	}
}
