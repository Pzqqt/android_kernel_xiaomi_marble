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

#include "htc_debug.h"
#include "htc_internal.h"
#include "htc_credit_history.h"
#include <qdf_nbuf.h>           /* qdf_nbuf_t */

/* HTC Control message receive timeout msec */
#define HTC_CONTROL_RX_TIMEOUT     6000

#if defined(WLAN_DEBUG) || defined(DEBUG)
void debug_dump_bytes(uint8_t *buffer, uint16_t length, char *pDescription)
{
	int8_t stream[60];
	int8_t byteOffsetStr[10];
	uint32_t i;
	uint16_t offset, count, byteOffset;

	A_PRINTF("<---------Dumping %d Bytes : %s ------>\n", length,
		 pDescription);

	count = 0;
	offset = 0;
	byteOffset = 0;
	for (i = 0; i < length; i++) {
		A_SNPRINTF(stream + offset, (sizeof(stream) - offset),
			   "%02X ", buffer[i]);
		count++;
		offset += 3;

		if (count == 16) {
			count = 0;
			offset = 0;
			A_SNPRINTF(byteOffsetStr, sizeof(byteOffset), "%4.4X",
				   byteOffset);
			A_PRINTF("[%s]: %s\n", byteOffsetStr, stream);
			qdf_mem_zero(stream, 60);
			byteOffset += 16;
		}
	}

	if (offset != 0) {
		A_SNPRINTF(byteOffsetStr, sizeof(byteOffset), "%4.4X",
			   byteOffset);
		A_PRINTF("[%s]: %s\n", byteOffsetStr, stream);
	}

	A_PRINTF("<------------------------------------------------->\n");
}
#else
void debug_dump_bytes(uint8_t *buffer, uint16_t length, char *pDescription)
{
}
#endif

static A_STATUS htc_process_trailer(HTC_TARGET *target,
				    uint8_t *pBuffer,
				    int Length, HTC_ENDPOINT_ID FromEndpoint);

static void do_recv_completion_pkt(HTC_ENDPOINT *pEndpoint,
				   HTC_PACKET *pPacket)
{
	if (!pEndpoint->EpCallBacks.EpRecv) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("HTC ep %d has NULL recv callback on packet %pK\n",
				 pEndpoint->Id,
				 pPacket));
		if (pPacket)
			qdf_nbuf_free(pPacket->pPktContext);
	} else {
		AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
				("HTC calling ep %d recv callback on packet %pK\n",
				 pEndpoint->Id, pPacket));
		pEndpoint->EpCallBacks.EpRecv(pEndpoint->EpCallBacks.pContext,
					      pPacket);
	}
}

static void do_recv_completion(HTC_ENDPOINT *pEndpoint,
			       HTC_PACKET_QUEUE *pQueueToIndicate)
{
	HTC_PACKET *pPacket;

	if (HTC_QUEUE_EMPTY(pQueueToIndicate)) {
		/* nothing to indicate */
		return;
	}

	while (!HTC_QUEUE_EMPTY(pQueueToIndicate)) {
		pPacket = htc_packet_dequeue(pQueueToIndicate);
		do_recv_completion_pkt(pEndpoint, pPacket);
	}
}

void htc_control_rx_complete(void *Context, HTC_PACKET *pPacket)
{
	/* TODO, can't really receive HTC control messages yet.... */
	AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			("Invalid call to  htc_control_rx_complete\n"));
}

void htc_unblock_recv(HTC_HANDLE HTCHandle)
{
	/* TODO  find the Need in new model */
}

void htc_enable_recv(HTC_HANDLE HTCHandle)
{

	/* TODO  find the Need in new model */
}

void htc_disable_recv(HTC_HANDLE HTCHandle)
{

	/* TODO  find the Need in new model */
}

int htc_get_num_recv_buffers(HTC_HANDLE HTCHandle, HTC_ENDPOINT_ID Endpoint)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);

	HTC_ENDPOINT *pEndpoint = &target->endpoint[Endpoint];
	return HTC_PACKET_QUEUE_DEPTH(&pEndpoint->RxBufferHoldQueue);
}

HTC_PACKET *allocate_htc_packet_container(HTC_TARGET *target)
{
	HTC_PACKET *pPacket;

	LOCK_HTC_RX(target);

	if (!target->pHTCPacketStructPool) {
		UNLOCK_HTC_RX(target);
		return NULL;
	}

	pPacket = target->pHTCPacketStructPool;
	target->pHTCPacketStructPool = (HTC_PACKET *) pPacket->ListLink.pNext;

	UNLOCK_HTC_RX(target);

	pPacket->ListLink.pNext = NULL;
	return pPacket;
}

void free_htc_packet_container(HTC_TARGET *target, HTC_PACKET *pPacket)
{
	pPacket->ListLink.pPrev = NULL;

	LOCK_HTC_RX(target);
	if (!target->pHTCPacketStructPool) {
		target->pHTCPacketStructPool = pPacket;
		pPacket->ListLink.pNext = NULL;
	} else {
		pPacket->ListLink.pNext =
			(DL_LIST *) target->pHTCPacketStructPool;
		target->pHTCPacketStructPool = pPacket;
	}

	UNLOCK_HTC_RX(target);
}

#ifdef RX_SG_SUPPORT
qdf_nbuf_t rx_sg_to_single_netbuf(HTC_TARGET *target)
{
	qdf_nbuf_t skb;
	uint8_t *anbdata;
	uint8_t *anbdata_new;
	uint32_t anblen;
	qdf_nbuf_t new_skb = NULL;
	uint32_t sg_queue_len;
	qdf_nbuf_queue_t *rx_sg_queue = &target->RxSgQueue;

	sg_queue_len = qdf_nbuf_queue_len(rx_sg_queue);

	if (sg_queue_len <= 1) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("rx_sg_to_single_netbuf: invalid sg queue len %u\n"));
		goto _failed;
	}

	new_skb = qdf_nbuf_alloc(target->ExpRxSgTotalLen, 0, 4, false);
	if (!new_skb) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("rx_sg_to_single_netbuf: can't allocate %u size netbuf\n",
				 target->ExpRxSgTotalLen));
		goto _failed;
	}

	qdf_nbuf_peek_header(new_skb, &anbdata_new, &anblen);

	skb = qdf_nbuf_queue_remove(rx_sg_queue);
	do {
		qdf_nbuf_peek_header(skb, &anbdata, &anblen);
		qdf_mem_copy(anbdata_new, anbdata, qdf_nbuf_len(skb));
		qdf_nbuf_put_tail(new_skb, qdf_nbuf_len(skb));
		anbdata_new += qdf_nbuf_len(skb);
		qdf_nbuf_free(skb);
		skb = qdf_nbuf_queue_remove(rx_sg_queue);
	} while (skb);

	RESET_RX_SG_CONFIG(target);
	return new_skb;

_failed:

	while ((skb = qdf_nbuf_queue_remove(rx_sg_queue)) != NULL)
		qdf_nbuf_free(skb);

	RESET_RX_SG_CONFIG(target);
	return NULL;
}
#endif

#ifdef CONFIG_WIN
#define HTC_MSG_NACK_SUSPEND 7
#endif

QDF_STATUS htc_rx_completion_handler(void *Context, qdf_nbuf_t netbuf,
				   uint8_t pipeID)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	HTC_FRAME_HDR *HtcHdr;
	HTC_TARGET *target = (HTC_TARGET *) Context;
	uint8_t *netdata;
	uint32_t netlen;
	HTC_ENDPOINT *pEndpoint, *currendpoint;
	HTC_PACKET *pPacket;
	uint16_t payloadLen;
	uint32_t trailerlen = 0;
	uint8_t htc_ep_id;
	int i;
#ifdef HTC_MSG_WAKEUP_FROM_SUSPEND_ID
	struct htc_init_info *info;
#endif

#ifdef RX_SG_SUPPORT
	LOCK_HTC_RX(target);
	if (target->IsRxSgInprogress) {
		target->CurRxSgTotalLen += qdf_nbuf_len(netbuf);
		qdf_nbuf_queue_add(&target->RxSgQueue, netbuf);
		if (target->CurRxSgTotalLen == target->ExpRxSgTotalLen) {
			netbuf = rx_sg_to_single_netbuf(target);
			if (!netbuf) {
				UNLOCK_HTC_RX(target);
				goto _out;
			}
		} else {
			netbuf = NULL;
			UNLOCK_HTC_RX(target);
			goto _out;
		}
	}
	UNLOCK_HTC_RX(target);
#endif

	netdata = qdf_nbuf_data(netbuf);
	netlen = qdf_nbuf_len(netbuf);

	HtcHdr = (HTC_FRAME_HDR *) netdata;

	do {

		htc_ep_id = HTC_GET_FIELD(HtcHdr, HTC_FRAME_HDR, ENDPOINTID);

		if (htc_ep_id >= ENDPOINT_MAX) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("HTC Rx: invalid EndpointID=%d\n",
					 htc_ep_id));
			debug_dump_bytes((uint8_t *) HtcHdr,
					sizeof(HTC_FRAME_HDR),
					"BAD HTC Header");
			status = QDF_STATUS_E_FAILURE;
			DPTRACE(qdf_dp_trace(
					    netbuf,
					    QDF_DP_TRACE_HTC_PACKET_PTR_RECORD,
					    QDF_TRACE_DEFAULT_PDEV_ID,
					    qdf_nbuf_data_addr(netbuf),
					    sizeof(qdf_nbuf_data(netbuf)),
					    QDF_RX));
			break;
		}

		pEndpoint = &target->endpoint[htc_ep_id];

		/*
		 * If this endpoint that received a message from the target has
		 * a to-target HIF pipe whose send completions are polled rather
		 * than interrupt driven, this is a good point to ask HIF to
		 * check whether it has any completed sends to handle.
		 */
		if (pEndpoint->ul_is_polled) {
			for (i = 0; i < ENDPOINT_MAX; i++) {
				currendpoint = &target->endpoint[i];
				if ((currendpoint->DL_PipeID ==
				     pEndpoint->DL_PipeID) &&
				    currendpoint->ul_is_polled) {
					htc_send_complete_check(currendpoint,
								1);
				}
			}
		}

		payloadLen = HTC_GET_FIELD(HtcHdr, HTC_FRAME_HDR, PAYLOADLEN);

		if (netlen < (payloadLen + HTC_HDR_LENGTH)) {
#ifdef RX_SG_SUPPORT
			LOCK_HTC_RX(target);
			target->IsRxSgInprogress = true;
			qdf_nbuf_queue_init(&target->RxSgQueue);
			qdf_nbuf_queue_add(&target->RxSgQueue, netbuf);
			target->ExpRxSgTotalLen = (payloadLen + HTC_HDR_LENGTH);
			target->CurRxSgTotalLen += netlen;
			UNLOCK_HTC_RX(target);
			netbuf = NULL;
			break;
#else
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("HTC Rx: insufficient length, got:%d expected =%zu\n",
					 netlen, payloadLen + HTC_HDR_LENGTH));
			debug_dump_bytes((uint8_t *) HtcHdr,
					 sizeof(HTC_FRAME_HDR),
					 "BAD RX packet length");
			status = QDF_STATUS_E_FAILURE;
			DPTRACE(qdf_dp_trace(
					    netbuf,
					    QDF_DP_TRACE_HTC_PACKET_PTR_RECORD,
					    QDF_TRACE_DEFAULT_PDEV_ID,
					    qdf_nbuf_data_addr(netbuf),
					    sizeof(qdf_nbuf_data(netbuf)),
					    QDF_RX));
			break;
#endif
		}
#ifdef HTC_EP_STAT_PROFILING
		LOCK_HTC_RX(target);
		INC_HTC_EP_STAT(pEndpoint, RxReceived, 1);
		UNLOCK_HTC_RX(target);
#endif

		/* if (IS_TX_CREDIT_FLOW_ENABLED(pEndpoint)) { */
		{
			uint8_t temp;
			A_STATUS temp_status;
			/* get flags to check for trailer */
			temp = HTC_GET_FIELD(HtcHdr, HTC_FRAME_HDR, FLAGS);
			if (temp & HTC_FLAGS_RECV_TRAILER) {
				/* extract the trailer length */
				temp =
					HTC_GET_FIELD(HtcHdr, HTC_FRAME_HDR,
						      CONTROLBYTES0);
				if ((temp < sizeof(HTC_RECORD_HDR))
				    || (temp > payloadLen)) {
					AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
						("htc_rx_completion_handler, invalid header (payloadlength should be :%d, CB[0] is:%d)\n",
						payloadLen, temp));
					status = QDF_STATUS_E_INVAL;
					break;
				}

				trailerlen = temp;
				/* process trailer data that follows HDR +
				 * application payload
				 */
				temp_status = htc_process_trailer(target,
						((uint8_t *) HtcHdr +
							HTC_HDR_LENGTH +
							payloadLen - temp),
						temp, htc_ep_id);
				if (A_FAILED(temp_status)) {
					status = QDF_STATUS_E_FAILURE;
					break;
				}

			}
		}

		if (((int)payloadLen - (int)trailerlen) <= 0) {
			/* 0 length packet with trailer data, just drop these */
			break;
		}

		if (htc_ep_id == ENDPOINT_0) {
			uint16_t message_id;
			HTC_UNKNOWN_MSG *htc_msg;
			bool wow_nack;

			/* remove HTC header */
			qdf_nbuf_pull_head(netbuf, HTC_HDR_LENGTH);
			netdata = qdf_nbuf_data(netbuf);
			netlen = qdf_nbuf_len(netbuf);

			htc_msg = (HTC_UNKNOWN_MSG *) netdata;
			message_id = HTC_GET_FIELD(htc_msg, HTC_UNKNOWN_MSG,
						   MESSAGEID);

			switch (message_id) {
			default:
				/* handle HTC control message */
				if (target->CtrlResponseProcessing) {
					/* this is a fatal error, target should
					 * not be sending unsolicited messages
					 * on the endpoint 0
					 */
					AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
							("HTC Rx Ctrl still processing\n"));
					status = QDF_STATUS_E_FAILURE;
					QDF_BUG(false);
					break;
				}

				LOCK_HTC_RX(target);
				target->CtrlResponseLength =
					min((int)netlen,
					    HTC_MAX_CONTROL_MESSAGE_LENGTH);
				qdf_mem_copy(target->CtrlResponseBuffer,
					     netdata,
					     target->CtrlResponseLength);

				/* Requester will clear this flag */
				target->CtrlResponseProcessing = true;
				UNLOCK_HTC_RX(target);

				qdf_event_set(&target->ctrl_response_valid);
				break;
#ifdef HTC_MSG_WAKEUP_FROM_SUSPEND_ID
			case HTC_MSG_WAKEUP_FROM_SUSPEND_ID:
				AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
					("Received initial wake up"));
				htc_credit_record(HTC_INITIAL_WAKE_UP,
					pEndpoint->TxCredits,
					HTC_PACKET_QUEUE_DEPTH(
					&pEndpoint->TxQueue));
				info = &target->HTCInitInfo;
				if (info && info->target_initial_wakeup_cb)
					info->target_initial_wakeup_cb(
						info->target_psoc);
				else
					AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
						("No initial wake up cb"));
				break;
#endif
			case HTC_MSG_SEND_SUSPEND_COMPLETE:
				wow_nack = false;
				htc_credit_record(HTC_SUSPEND_ACK,
					pEndpoint->TxCredits,
					HTC_PACKET_QUEUE_DEPTH(
					&pEndpoint->TxQueue));
				target->HTCInitInfo.TargetSendSuspendComplete(
					target->HTCInitInfo.target_psoc,
					wow_nack);

				break;
			case HTC_MSG_NACK_SUSPEND:
				wow_nack = true;
				htc_credit_record(HTC_SUSPEND_ACK,
					pEndpoint->TxCredits,
					HTC_PACKET_QUEUE_DEPTH(
					&pEndpoint->TxQueue));
				target->HTCInitInfo.TargetSendSuspendComplete(
					target->HTCInitInfo.target_psoc,
					wow_nack);
				break;
			}

			qdf_nbuf_free(netbuf);
			netbuf = NULL;
			break;
		}

		/* the current message based HIF architecture allocates net bufs
		 * for recv packets since this layer bridges that HIF to upper
		 * layers , which expects HTC packets, we form the packets here
		 * TODO_FIXME
		 */
		pPacket = allocate_htc_packet_container(target);
		if (!pPacket) {
			status = QDF_STATUS_E_RESOURCES;
			break;
		}
		pPacket->Status = QDF_STATUS_SUCCESS;
		pPacket->Endpoint = htc_ep_id;
		pPacket->pPktContext = netbuf;
		pPacket->pBuffer = qdf_nbuf_data(netbuf) + HTC_HDR_LENGTH;
		pPacket->ActualLength = netlen - HTC_HEADER_LEN - trailerlen;

		qdf_nbuf_pull_head(netbuf, HTC_HEADER_LEN);
		qdf_nbuf_set_pktlen(netbuf, pPacket->ActualLength);

		do_recv_completion_pkt(pEndpoint, pPacket);

		/* recover the packet container */
		free_htc_packet_container(target, pPacket);

		netbuf = NULL;

	} while (false);

#ifdef RX_SG_SUPPORT
_out:
#endif

	if (netbuf)
		qdf_nbuf_free(netbuf);

	return status;

}

A_STATUS htc_add_receive_pkt_multiple(HTC_HANDLE HTCHandle,
				      HTC_PACKET_QUEUE *pPktQueue)
{
	HTC_TARGET *target = GET_HTC_TARGET_FROM_HANDLE(HTCHandle);
	HTC_ENDPOINT *pEndpoint;
	HTC_PACKET *pFirstPacket;
	A_STATUS status = A_OK;
	HTC_PACKET *pPacket;

	pFirstPacket = htc_get_pkt_at_head(pPktQueue);

	if (!pFirstPacket) {
		A_ASSERT(false);
		return A_EINVAL;
	}

	if (pFirstPacket->Endpoint >= ENDPOINT_MAX) {
		A_ASSERT(false);
		return A_EINVAL;
	}

	AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
			("+- htc_add_receive_pkt_multiple : endPointId: %d, cnt:%d, length: %d\n",
			 pFirstPacket->Endpoint,
			 HTC_PACKET_QUEUE_DEPTH(pPktQueue),
			 pFirstPacket->BufferLength));

	pEndpoint = &target->endpoint[pFirstPacket->Endpoint];

	LOCK_HTC_RX(target);

	do {

		if (HTC_STOPPING(target)) {
			status = A_ERROR;
			break;
		}

		/* store receive packets */
		HTC_PACKET_QUEUE_TRANSFER_TO_TAIL(&pEndpoint->RxBufferHoldQueue,
						  pPktQueue);

	} while (false);

	UNLOCK_HTC_RX(target);

	if (A_FAILED(status)) {
		/* walk through queue and mark each one canceled */
		HTC_PACKET_QUEUE_ITERATE_ALLOW_REMOVE(pPktQueue, pPacket) {
			pPacket->Status = QDF_STATUS_E_CANCELED;
		}
		HTC_PACKET_QUEUE_ITERATE_END;

		do_recv_completion(pEndpoint, pPktQueue);
	}

	return status;
}

void htc_flush_rx_hold_queue(HTC_TARGET *target, HTC_ENDPOINT *pEndpoint)
{
	HTC_PACKET *pPacket;

	LOCK_HTC_RX(target);

	while (1) {
		pPacket = htc_packet_dequeue(&pEndpoint->RxBufferHoldQueue);
		if (!pPacket)
			break;
		UNLOCK_HTC_RX(target);
		pPacket->Status = QDF_STATUS_E_CANCELED;
		pPacket->ActualLength = 0;
		AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
				("Flushing RX packet:%pK, length:%d, ep:%d\n",
				 pPacket, pPacket->BufferLength,
				 pPacket->Endpoint));
		/* give the packet back */
		do_recv_completion_pkt(pEndpoint, pPacket);
		LOCK_HTC_RX(target);
	}

	UNLOCK_HTC_RX(target);
}

void htc_recv_init(HTC_TARGET *target)
{
	/* Initialize ctrl_response_valid to block */
	qdf_event_create(&target->ctrl_response_valid);
}

/* polling routine to wait for a control packet to be received */
QDF_STATUS htc_wait_recv_ctrl_message(HTC_TARGET *target)
{
/*    int count = HTC_TARGET_MAX_RESPONSE_POLL; */

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("+HTCWaitCtrlMessageRecv\n"));

	/* Wait for BMI request/response transaction to complete */
	if (qdf_wait_single_event(&target->ctrl_response_valid,
				  HTC_CONTROL_RX_TIMEOUT)) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			("Failed to receive control message\n"));
		return QDF_STATUS_E_FAILURE;
	}

	LOCK_HTC_RX(target);
	/* caller will clear this flag */
	target->CtrlResponseProcessing = true;

	UNLOCK_HTC_RX(target);

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("-HTCWaitCtrlMessageRecv success\n"));
	return QDF_STATUS_SUCCESS;
}

static A_STATUS htc_process_trailer(HTC_TARGET *target,
				    uint8_t *pBuffer,
				    int Length, HTC_ENDPOINT_ID FromEndpoint)
{
	HTC_RECORD_HDR *pRecord;
	uint8_t htc_rec_id;
	uint8_t htc_rec_len;
	uint8_t *pRecordBuf;
	uint8_t *pOrigBuffer;
	int origLength;
	A_STATUS status;

	AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
			("+htc_process_trailer (length:%d)\n", Length));

	if (AR_DEBUG_LVL_CHECK(ATH_DEBUG_RECV))
		AR_DEBUG_PRINTBUF(pBuffer, Length, "Recv Trailer");

	pOrigBuffer = pBuffer;
	origLength = Length;
	status = A_OK;

	while (Length > 0) {

		if (Length < sizeof(HTC_RECORD_HDR)) {
			status = A_EPROTO;
			break;
		}
		/* these are byte aligned structs */
		pRecord = (HTC_RECORD_HDR *) pBuffer;
		Length -= sizeof(HTC_RECORD_HDR);
		pBuffer += sizeof(HTC_RECORD_HDR);

		htc_rec_len = HTC_GET_FIELD(pRecord, HTC_RECORD_HDR, LENGTH);
		htc_rec_id = HTC_GET_FIELD(pRecord, HTC_RECORD_HDR, RECORDID);

		if (htc_rec_len > Length) {
			/* no room left in buffer for record */
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("invalid record length: %d (id:%d) buffer has: %d bytes left\n",
					 htc_rec_len, htc_rec_id, Length));
			status = A_EPROTO;
			break;
		}
		/* start of record follows the header */
		pRecordBuf = pBuffer;

		switch (htc_rec_id) {
		case HTC_RECORD_CREDITS:
			AR_DEBUG_ASSERT(htc_rec_len >=
					sizeof(HTC_CREDIT_REPORT));
			htc_process_credit_rpt(target,
					       (HTC_CREDIT_REPORT *) pRecordBuf,
					       htc_rec_len /
					       (sizeof(HTC_CREDIT_REPORT)),
					       FromEndpoint);
			break;

#ifdef HIF_SDIO
		case HTC_RECORD_LOOKAHEAD:
			/* Process in HIF layer */
			break;

		case HTC_RECORD_LOOKAHEAD_BUNDLE:
			/* Process in HIF layer */
			break;
#endif /* HIF_SDIO */

		default:
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("HTC unhandled record: id:%d length:%d\n",
					 htc_rec_id, htc_rec_len));
			break;
		}

		if (A_FAILED(status)) {
			break;
		}

		/* advance buffer past this record for next time around */
		pBuffer += htc_rec_len;
		Length -= htc_rec_len;
	}

	if (A_FAILED(status))
		debug_dump_bytes(pOrigBuffer, origLength, "BAD Recv Trailer");

	AR_DEBUG_PRINTF(ATH_DEBUG_RECV, ("-htc_process_trailer\n"));
	return status;

}
