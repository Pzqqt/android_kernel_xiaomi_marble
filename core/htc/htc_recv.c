/*
 * Copyright (c) 2013-2015 The Linux Foundation. All rights reserved.
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
#include "cds_api.h"
#include <cdf_nbuf.h>           /* cdf_nbuf_t */
#include "epping_main.h"

/* HTC Control message receive timeout msec */
#define HTC_CONTROL_RX_TIMEOUT     3000

#ifdef DEBUG
void debug_dump_bytes(A_UCHAR *buffer, A_UINT16 length, char *pDescription)
{
	A_CHAR stream[60];
	A_CHAR byteOffsetStr[10];
	A_UINT32 i;
	A_UINT16 offset, count, byteOffset;

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
			A_MEMZERO(stream, 60);
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
void debug_dump_bytes(A_UCHAR *buffer, A_UINT16 length, char *pDescription)
{
}
#endif

static A_STATUS htc_process_trailer(HTC_TARGET *target,
				    A_UINT8 *pBuffer,
				    int Length, HTC_ENDPOINT_ID FromEndpoint);

static void do_recv_completion(HTC_ENDPOINT *pEndpoint,
			       HTC_PACKET_QUEUE *pQueueToIndicate)
{

	do {

		if (HTC_QUEUE_EMPTY(pQueueToIndicate)) {
			/* nothing to indicate */
			break;
		}

		if (pEndpoint->EpCallBacks.EpRecvPktMultiple != NULL) {
			AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
					(" HTC calling ep %d, recv multiple callback (%d pkts) \n",
					 pEndpoint->Id,
					 HTC_PACKET_QUEUE_DEPTH
						 (pQueueToIndicate)));
			/* a recv multiple handler is being used, pass the queue to the handler */
			pEndpoint->EpCallBacks.EpRecvPktMultiple(pEndpoint->
								 EpCallBacks.
								 pContext,
								 pQueueToIndicate);
			INIT_HTC_PACKET_QUEUE(pQueueToIndicate);
		} else {
			HTC_PACKET *pPacket;
			/* using legacy EpRecv */
			while (!HTC_QUEUE_EMPTY(pQueueToIndicate)) {
				pPacket = htc_packet_dequeue(pQueueToIndicate);
				if (pEndpoint->EpCallBacks.EpRecv == NULL) {
					AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
							("HTC ep %d has NULL recv callback on packet %p\n",
							 pEndpoint->Id,
							 pPacket));
					continue;
				}
				AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
						("HTC calling ep %d recv callback on packet %p\n",
						 pEndpoint->Id, pPacket));
				pEndpoint->EpCallBacks.EpRecv(pEndpoint->
							      EpCallBacks.
							      pContext,
							      pPacket);
			}
		}

	} while (false);

}

static void recv_packet_completion(HTC_TARGET *target, HTC_ENDPOINT *pEndpoint,
				   HTC_PACKET *pPacket)
{
	HTC_PACKET_QUEUE container;
	INIT_HTC_PACKET_QUEUE_AND_ADD(&container, pPacket);
	/* do completion */
	do_recv_completion(pEndpoint, &container);
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

	if (NULL == target->pHTCPacketStructPool) {
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
	LOCK_HTC_RX(target);

	if (NULL == target->pHTCPacketStructPool) {
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
cdf_nbuf_t rx_sg_to_single_netbuf(HTC_TARGET *target)
{
	cdf_nbuf_t skb;
	uint8_t *anbdata;
	uint8_t *anbdata_new;
	uint32_t anblen;
	cdf_nbuf_t new_skb = NULL;
	uint32_t sg_queue_len;
	cdf_nbuf_queue_t *rx_sg_queue = &target->RxSgQueue;

	sg_queue_len = cdf_nbuf_queue_len(rx_sg_queue);

	if (sg_queue_len <= 1) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("rx_sg_to_single_netbuf: invalid sg queue len %u\n"));
		goto _failed;
	}

	new_skb = cdf_nbuf_alloc(target->ExpRxSgTotalLen, 0, 4, false);
	if (new_skb == NULL) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("rx_sg_to_single_netbuf: can't allocate %u size netbuf\n",
				 target->ExpRxSgTotalLen));
		goto _failed;
	}

	cdf_nbuf_peek_header(new_skb, &anbdata_new, &anblen);

	skb = cdf_nbuf_queue_remove(rx_sg_queue);
	do {
		cdf_nbuf_peek_header(skb, &anbdata, &anblen);
		cdf_mem_copy(anbdata_new, anbdata, cdf_nbuf_len(skb));
		cdf_nbuf_put_tail(new_skb, cdf_nbuf_len(skb));
		anbdata_new += cdf_nbuf_len(skb);
		cdf_nbuf_free(skb);
		skb = cdf_nbuf_queue_remove(rx_sg_queue);
	} while (skb != NULL);

	RESET_RX_SG_CONFIG(target);
	return new_skb;

_failed:

	while ((skb = cdf_nbuf_queue_remove(rx_sg_queue)) != NULL) {
		cdf_nbuf_free(skb);
	}

	RESET_RX_SG_CONFIG(target);
	return NULL;
}
#endif

CDF_STATUS htc_rx_completion_handler(void *Context, cdf_nbuf_t netbuf,
				   uint8_t pipeID)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	HTC_FRAME_HDR *HtcHdr;
	HTC_TARGET *target = (HTC_TARGET *) Context;
	uint8_t *netdata;
	uint32_t netlen;
	HTC_ENDPOINT *pEndpoint;
	HTC_PACKET *pPacket;
	A_UINT16 payloadLen;
	uint32_t trailerlen = 0;
	A_UINT8 htc_ep_id;

#ifdef RX_SG_SUPPORT
	LOCK_HTC_RX(target);
	if (target->IsRxSgInprogress) {
		target->CurRxSgTotalLen += cdf_nbuf_len(netbuf);
		cdf_nbuf_queue_add(&target->RxSgQueue, netbuf);
		if (target->CurRxSgTotalLen == target->ExpRxSgTotalLen) {
			netbuf = rx_sg_to_single_netbuf(target);
			if (netbuf == NULL) {
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

	netdata = cdf_nbuf_data(netbuf);
	netlen = cdf_nbuf_len(netbuf);

	HtcHdr = (HTC_FRAME_HDR *) netdata;

	do {

		htc_ep_id = HTC_GET_FIELD(HtcHdr, HTC_FRAME_HDR, ENDPOINTID);

		if (htc_ep_id >= ENDPOINT_MAX) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("HTC Rx: invalid EndpointID=%d\n",
					 htc_ep_id));
			debug_dump_bytes((A_UINT8 *) HtcHdr,
					 sizeof(HTC_FRAME_HDR), "BAD HTC Header");
			status = CDF_STATUS_E_FAILURE;
			CDF_BUG(0);
			break;
		}

		pEndpoint = &target->endpoint[htc_ep_id];

		/*
		 * If this endpoint that received a message from the target has
		 * a to-target HIF pipe whose send completions are polled rather
		 * than interrupt-driven, this is a good point to ask HIF to check
		 * whether it has any completed sends to handle.
		 */
		if (pEndpoint->ul_is_polled) {
			htc_send_complete_check(pEndpoint, 1);
		}

		payloadLen = HTC_GET_FIELD(HtcHdr, HTC_FRAME_HDR, PAYLOADLEN);

		if (netlen < (payloadLen + HTC_HDR_LENGTH)) {
#ifdef RX_SG_SUPPORT
			LOCK_HTC_RX(target);
			target->IsRxSgInprogress = true;
			cdf_nbuf_queue_init(&target->RxSgQueue);
			cdf_nbuf_queue_add(&target->RxSgQueue, netbuf);
			target->ExpRxSgTotalLen = (payloadLen + HTC_HDR_LENGTH);
			target->CurRxSgTotalLen += netlen;
			UNLOCK_HTC_RX(target);
			netbuf = NULL;
			break;
#else
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("HTC Rx: insufficient length, got:%d expected =%zu\n",
					 netlen, payloadLen + HTC_HDR_LENGTH));
			debug_dump_bytes((A_UINT8 *) HtcHdr,
					 sizeof(HTC_FRAME_HDR),
					 "BAD RX packet length");
			status = CDF_STATUS_E_FAILURE;
			CDF_BUG(0);
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
			A_UINT8 temp;
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
					status = CDF_STATUS_E_INVAL;
					break;
				}

				trailerlen = temp;
				/* process trailer data that follows HDR + application payload */
				temp_status = htc_process_trailer(target,
							     ((A_UINT8 *) HtcHdr +
							      HTC_HDR_LENGTH +
							      payloadLen - temp),
							     temp, htc_ep_id);
				if (A_FAILED(temp_status)) {
					status = CDF_STATUS_E_FAILURE;
					break;
				}

			}
		}

		if (((int)payloadLen - (int)trailerlen) <= 0) {
			/* zero length packet with trailer data, just drop these */
			break;
		}

		if (htc_ep_id == ENDPOINT_0) {
			A_UINT16 message_id;
			HTC_UNKNOWN_MSG *htc_msg;
			int wow_nack = 0;

			/* remove HTC header */
			cdf_nbuf_pull_head(netbuf, HTC_HDR_LENGTH);
			netdata = cdf_nbuf_data(netbuf);
			netlen = cdf_nbuf_len(netbuf);

			htc_msg = (HTC_UNKNOWN_MSG *) netdata;
			message_id =
				HTC_GET_FIELD(htc_msg, HTC_UNKNOWN_MSG, MESSAGEID);

			switch (message_id) {
			default:
				/* handle HTC control message */
				if (target->CtrlResponseProcessing) {
					/* this is a fatal error, target should not be sending unsolicited messages
					 * on the endpoint 0 */
					AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
							("HTC Rx Ctrl still processing\n"));
					status = CDF_STATUS_E_FAILURE;
					CDF_BUG(false);
					break;
				}

				LOCK_HTC_RX(target);
				target->CtrlResponseLength =
					min((int)netlen,
					    HTC_MAX_CONTROL_MESSAGE_LENGTH);
				A_MEMCPY(target->CtrlResponseBuffer, netdata,
					 target->CtrlResponseLength);

				/* Requester will clear this flag */
				target->CtrlResponseProcessing = true;
				UNLOCK_HTC_RX(target);

				cdf_event_set(&target->ctrl_response_valid);
				break;
			case HTC_MSG_SEND_SUSPEND_COMPLETE:
				wow_nack = 0;
				LOCK_HTC_CREDIT(target);
				htc_credit_record(HTC_SUSPEND_ACK,
					pEndpoint->TxCredits,
					HTC_PACKET_QUEUE_DEPTH(
						&pEndpoint->TxQueue));
				UNLOCK_HTC_CREDIT(target);
				target->HTCInitInfo.
				TargetSendSuspendComplete((void *)
							  &wow_nack);
				break;
			case HTC_MSG_NACK_SUSPEND:
				wow_nack = 1;
				LOCK_HTC_CREDIT(target);
				htc_credit_record(HTC_SUSPEND_ACK,
					pEndpoint->TxCredits,
					HTC_PACKET_QUEUE_DEPTH(
						&pEndpoint->TxQueue));
				UNLOCK_HTC_CREDIT(target);

				target->HTCInitInfo.
				TargetSendSuspendComplete((void *)
							  &wow_nack);
				break;
			}

			cdf_nbuf_free(netbuf);
			netbuf = NULL;
			break;
		}

		/* the current message based HIF architecture allocates net bufs for recv packets
		 * since this layer bridges that HIF to upper layers , which expects HTC packets,
		 * we form the packets here
		 * TODO_FIXME */
		pPacket = allocate_htc_packet_container(target);
		if (NULL == pPacket) {
			status = CDF_STATUS_E_RESOURCES;
			break;
		}
		pPacket->Status = CDF_STATUS_SUCCESS;
		pPacket->Endpoint = htc_ep_id;
		pPacket->pPktContext = netbuf;
		pPacket->pBuffer = cdf_nbuf_data(netbuf) + HTC_HDR_LENGTH;
		pPacket->ActualLength = netlen - HTC_HEADER_LEN - trailerlen;

		cdf_nbuf_pull_head(netbuf, HTC_HEADER_LEN);
		cdf_nbuf_set_pktlen(netbuf, pPacket->ActualLength);

		recv_packet_completion(target, pEndpoint, pPacket);
		/* recover the packet container */
		free_htc_packet_container(target, pPacket);
		netbuf = NULL;

	} while (false);

#ifdef RX_SG_SUPPORT
_out:
#endif

	if (netbuf != NULL) {
		cdf_nbuf_free(netbuf);
	}

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

	if (NULL == pFirstPacket) {
		A_ASSERT(false);
		return A_EINVAL;
	}

	AR_DEBUG_ASSERT(pFirstPacket->Endpoint < ENDPOINT_MAX);

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
			pPacket->Status = A_ECANCELED;
		}
		HTC_PACKET_QUEUE_ITERATE_END;

		do_recv_completion(pEndpoint, pPktQueue);
	}

	return status;
}

A_STATUS htc_add_receive_pkt(HTC_HANDLE HTCHandle, HTC_PACKET *pPacket)
{
	HTC_PACKET_QUEUE queue;
	INIT_HTC_PACKET_QUEUE_AND_ADD(&queue, pPacket);
	return htc_add_receive_pkt_multiple(HTCHandle, &queue);
}

void htc_flush_rx_hold_queue(HTC_TARGET *target, HTC_ENDPOINT *pEndpoint)
{
	HTC_PACKET *pPacket;
	HTC_PACKET_QUEUE container;

	LOCK_HTC_RX(target);

	while (1) {
		pPacket = htc_packet_dequeue(&pEndpoint->RxBufferHoldQueue);
		if (NULL == pPacket) {
			break;
		}
		UNLOCK_HTC_RX(target);
		pPacket->Status = A_ECANCELED;
		pPacket->ActualLength = 0;
		AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
				("  Flushing RX packet:%p, length:%d, ep:%d \n",
				 pPacket, pPacket->BufferLength,
				 pPacket->Endpoint));
		INIT_HTC_PACKET_QUEUE_AND_ADD(&container, pPacket);
		/* give the packet back */
		do_recv_completion(pEndpoint, &container);
		LOCK_HTC_RX(target);
	}

	UNLOCK_HTC_RX(target);
}

void htc_recv_init(HTC_TARGET *target)
{
	/* Initialize ctrl_response_valid to block */
	cdf_event_init(&target->ctrl_response_valid);
}

/* polling routine to wait for a control packet to be received */
A_STATUS htc_wait_recv_ctrl_message(HTC_TARGET *target)
{
/*    int count = HTC_TARGET_MAX_RESPONSE_POLL; */

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("+HTCWaitCtrlMessageRecv\n"));

	/* Wait for BMI request/response transaction to complete */
	if (cdf_wait_single_event(&target->ctrl_response_valid,
		cdf_system_msecs_to_ticks(HTC_CONTROL_RX_TIMEOUT))) {
		CDF_BUG(0);
		return A_ERROR;
	}

	LOCK_HTC_RX(target);
	/* caller will clear this flag */
	target->CtrlResponseProcessing = true;

	UNLOCK_HTC_RX(target);

#if 0
	while (count > 0) {

		LOCK_HTC_RX(target);

		if (target->CtrlResponseValid) {
			target->CtrlResponseValid = false;
			/* caller will clear this flag */
			target->CtrlResponseProcessing = true;
			UNLOCK_HTC_RX(target);
			break;
		}

		UNLOCK_HTC_RX(target);

		count--;
		A_MSLEEP(HTC_TARGET_RESPONSE_POLL_MS);
	}

	if (count <= 0) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("-HTCWaitCtrlMessageRecv: Timeout!\n"));
		return A_ECOMM;
	}
#endif

	AR_DEBUG_PRINTF(ATH_DEBUG_TRC, ("-HTCWaitCtrlMessageRecv success\n"));
	return A_OK;
}

static A_STATUS htc_process_trailer(HTC_TARGET *target,
				    A_UINT8 *pBuffer,
				    int Length, HTC_ENDPOINT_ID FromEndpoint)
{
	HTC_RECORD_HDR *pRecord;
	A_UINT8 htc_rec_id;
	A_UINT8 htc_rec_len;
	A_UINT8 *pRecordBuf;
	A_UINT8 *pOrigBuffer;
	int origLength;
	A_STATUS status;

	AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
			("+htc_process_trailer (length:%d) \n", Length));

	if (AR_DEBUG_LVL_CHECK(ATH_DEBUG_RECV)) {
		AR_DEBUG_PRINTBUF(pBuffer, Length, "Recv Trailer");
	}

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
					(" invalid record length: %d (id:%d) buffer has: %d bytes left \n",
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
					(" HTC unhandled record: id:%d length:%d \n",
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

	if (A_FAILED(status)) {
		debug_dump_bytes(pOrigBuffer, origLength, "BAD Recv Trailer");
	}

	AR_DEBUG_PRINTF(ATH_DEBUG_RECV, ("-htc_process_trailer \n"));
	return status;

}
