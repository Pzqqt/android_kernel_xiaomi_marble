/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
 *
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


#define ATH_MODULE_NAME hif
#include <qdf_types.h>
#include <qdf_status.h>
#include <qdf_timer.h>
#include <qdf_time.h>
#include <qdf_lock.h>
#include <qdf_mem.h>
#include <qdf_util.h>
#include <qdf_defer.h>
#include <qdf_atomic.h>
#include <qdf_nbuf.h>
#include <athdefs.h>
#include <qdf_net_types.h>
#include <a_types.h>
#include <athdefs.h>
#include <a_osapi.h>
#include <hif.h>
#include <htc_services.h>
#include <a_debug.h>
#include <htc_internal.h>
#include "hif_sdio_internal.h"
#include "transfer.h"

/**
 * hif_dev_rw_completion_handler() - Completion routine
 * for ALL HIF layer async I/O
 * @context: hif send context
 * @status: completion routine sync/async context
 *
 * Return: 0 for success and non-zero for failure
 */

QDF_STATUS hif_dev_rw_completion_handler(void *ctx, QDF_STATUS status)
{
	QDF_STATUS (*txCompHandler)(void *, qdf_nbuf_t, uint32_t, uint32_t);
	struct hif_sendContext *sctx = (struct hif_sendContext *)ctx;
	struct hif_sdio_device *pdev = sctx->pDev;
	unsigned int xfer_id = sctx->transferID;
	uint32_t toeplitz_hash_result = 0;
	qdf_nbuf_t buf = sctx->netbuf;

	if (sctx->bNewAlloc)
		qdf_mem_free(ctx);
	else
		qdf_nbuf_pull_head(buf, sctx->head_data_len);

	txCompHandler = pdev->hif_callbacks.txCompletionHandler;
	if (txCompHandler) {
		txCompHandler(pdev->hif_callbacks.Context, buf,
			      xfer_id, toeplitz_hash_result);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_dev_send_buffer() - send buffer to sdio device
 * @pDev: HIf device object
 * @xfer_id: transfer id
 * @pipe: ul/dl pipe
 * @nbytes: no of bytes to transfer
 * @buf: pointer to buffer
 *
 * Return: 0 for success and non-zero for failure
 */
QDF_STATUS hif_dev_send_buffer(struct hif_sdio_device *pdev, uint32_t xfer_id,
			       uint8_t pipe, uint32_t nbytes, qdf_nbuf_t buf)
{
	QDF_STATUS status;
	unsigned char *pData;
	struct hif_sendContext *sctx;
	uint32_t request = hif_get_send_buffer_flags(pdev);
	uint32_t padded_length;
	unsigned long addr = 0;
	int frag_count = 0, i, count, head_len;

	if (hif_get_send_address(pdev, pipe, &addr)) {
		hif_err("%s: Invalid address map for pipe 0x%x",
			__func__, pipe);

		return QDF_STATUS_E_INVAL;
	}

	padded_length = DEV_CALC_SEND_PADDED_LEN(pdev, nbytes);
	A_ASSERT(padded_length - nbytes < HIF_DUMMY_SPACE_MASK + 1);

	request |= ((padded_length - nbytes) << 16);

	frag_count = qdf_nbuf_get_num_frags(buf);

	if (frag_count > 1) {
		/* Header data length should be total sending length.
		 * Subtract internal data length of netbuf
		 */
		head_len = sizeof(struct hif_sendContext) +
			(nbytes - qdf_nbuf_get_frag_len(buf, frag_count - 1));
	} else {
		/*
		 * | hif_sendContext | netbuf->data
		 */
		head_len = sizeof(struct hif_sendContext);
	}

	/* Check whether head room is enough to save extra head data */
	if ((head_len <= qdf_nbuf_headroom(buf)) &&
	    (qdf_nbuf_tailroom(buf) >= (padded_length - nbytes))) {
		sctx = (struct hif_sendContext *)qdf_nbuf_push_head(buf,
								    head_len);
		sctx->bNewAlloc = false;
	} else {
		sctx = (struct hif_sendContext *)qdf_mem_malloc(sizeof(*sctx) +
								padded_length);
		if (sctx)
			sctx->bNewAlloc = true;
		else
			return QDF_STATUS_E_NOMEM;
	}

	sctx->netbuf = buf;
	sctx->pDev = pdev;
	sctx->transferID = xfer_id;
	sctx->head_data_len = head_len;
	/*
	 * Copy data to head part of netbuf or head of allocated buffer.
	 * if buffer is new allocated, the last buffer should be copied also.
	 * It assume last fragment is internal buffer of netbuf
	 * sometime total length of fragments larger than nbytes
	 */
	pData = (unsigned char *)sctx + sizeof(struct hif_sendContext);
	for (i = 0, count = sctx->bNewAlloc ? frag_count : frag_count - 1;
	     i < count;
	     i++) {
		int frag_len = qdf_nbuf_get_frag_len(buf, i);
		unsigned char *frag_addr = qdf_nbuf_get_frag_vaddr(buf, i);

		if (frag_len > nbytes)
			frag_len = nbytes;
		memcpy(pData, frag_addr, frag_len);
		pData += frag_len;
		nbytes -= frag_len;
		if (nbytes <= 0)
			break;
	}

	/* Reset pData pointer and sctx out */
	pData = (unsigned char *)sctx + sizeof(struct hif_sendContext);

	status = hif_read_write(pdev->HIFDevice, addr, (char *)pData,
				padded_length, request, (void *)sctx);

	if (status == QDF_STATUS_E_PENDING) {
		/*
		 * it will return QDF_STATUS_E_PENDING in native HIF
		 * implementation, which should be treated as successful
		 * result here.
		 */
		status = QDF_STATUS_SUCCESS;
	}

	/* release buffer or move back data pointer when failed */
	if (status != QDF_STATUS_SUCCESS) {
		if (sctx->bNewAlloc)
			qdf_mem_free(sctx);
		else
			qdf_nbuf_pull_head(buf, head_len);
	}

	return status;
}

/**
 * hif_dev_alloc_and_prepare_rx_packets() - Allocate packets for recv frames.
 * @pdev : HIF device object
 * @look_aheads : Look ahead information on the frames
 * @messages : Number of messages
 * @queue : Queue to put the allocated frames
 *
 * Return : QDF_STATUS_SUCCESS on success else error value
 */
QDF_STATUS hif_dev_alloc_and_prepare_rx_packets(struct hif_sdio_device *pdev,
						uint32_t look_aheads[],
						int messages,
						HTC_PACKET_QUEUE *queue)
{
	int i, j;
	bool no_recycle;
	int num_messages;
	HTC_PACKET *packet;
	HTC_FRAME_HDR *hdr;
	uint32_t full_length;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* lock RX while we assemble the packet buffers */
	LOCK_HIF_DEV_RX(pdev);

	for (i = 0; i < messages; i++) {
		hdr = (HTC_FRAME_HDR *)&look_aheads[i];
		if (hdr->EndpointID >= ENDPOINT_MAX) {
			hif_err("%s: Invalid Endpoint:%d\n",
				__func__, hdr->EndpointID);
			status = QDF_STATUS_E_INVAL;
			break;
		}

		if (hdr->PayloadLen > HTC_MAX_PAYLOAD_LENGTH) {
			hif_err("%s: Payload length %d exceeds max HTC : %u",
				__func__,
				hdr->PayloadLen,
				(uint32_t)HTC_MAX_PAYLOAD_LENGTH);
			status = QDF_STATUS_E_INVAL;
			break;
		}

		if ((hdr->Flags & HTC_FLAGS_RECV_BUNDLE_CNT_MASK) == 0) {
			/* HTC header only indicates 1 message to fetch */
			num_messages = 1;
		} else {
			/* HTC header indicates that every packet to follow
			 * has the same padded length so that it can
			 * be optimally fetched as a full bundle
			 */
			num_messages = GET_RECV_BUNDLE_COUNT(hdr->Flags);
			/* the count doesn't include the starter frame, just
			 * a count of frames to follow
			 */
			num_messages++;

			hif_info("%s: HTC header : %u messages in bundle",
				 __func__, num_messages);
		}

		full_length = DEV_CALC_RECV_PADDED_LEN(pdev,
						       hdr->PayloadLen +
						       sizeof(HTC_FRAME_HDR));

		/* get packet buffers for each message, if there was a
		 * bundle detected in the header,
		 * use pHdr as a template to fetch all packets in the bundle
		 */
		for (j = 0; j < num_messages; j++) {
			/* reset flag, any packets allocated using the
			 * RecvAlloc() API cannot be recycled on cleanup,
			 * they must be explicitly returned
			 */
			no_recycle = false;
			packet = hif_dev_alloc_rx_buffer(pdev);

			if (!packet) {
				/* No error, simply need to mark that
				 * we are waiting for buffers.
				 */
				pdev->RecvStateFlags |= HTC_RECV_WAIT_BUFFERS;
				/* pDev->EpWaitingForBuffers = pEndpoint->Id; */
				status = QDF_STATUS_E_RESOURCES;
				break;
			}
			/* clear flags */
			packet->PktInfo.AsRx.HTCRxFlags = 0;
			packet->PktInfo.AsRx.IndicationFlags = 0;
			packet->Status = QDF_STATUS_SUCCESS;

			if (no_recycle) {
				/* flag that these packets cannot be recycled,
				 * they have to be returned to the user
				 */
				packet->PktInfo.AsRx.HTCRxFlags |=
					HTC_RX_PKT_NO_RECYCLE;
			}
			/* add packet to queue (also incase we need to
			 * cleanup down below)
			 */
			HTC_PACKET_ENQUEUE(queue, packet);

			/* if (HTC_STOPPING(target)) {
			 *      status = QDF_STATUS_E_CANCELED;
			 *      break;
			 *  }
			 */

			/* make sure  message can fit in the endpoint buffer */
			if (full_length > packet->BufferLength) {
				hif_err("%s: Payload Length Error", __func__);
				hif_err("%s: header reports payload: %u(%u)",
					__func__, hdr->PayloadLen,
					full_length);
				hif_err("%s: endpoint buffer size: %d\n",
					__func__, packet->BufferLength);
				status = QDF_STATUS_E_INVAL;
				break;
			}

			if (j > 0) {
				/* for messages fetched in a bundle the expected
				 * lookahead is unknown as we are only using the
				 * lookahead of the first packet as a template
				 * of what to expect for lengths
				 */
				packet->PktInfo.AsRx.HTCRxFlags |=
					HTC_RX_PKT_REFRESH_HDR;
				/* set it to something invalid */
				packet->PktInfo.AsRx.ExpectedHdr = 0xFFFFFFFF;
			} else {
				packet->PktInfo.AsRx.ExpectedHdr =
					look_aheads[i];
			}
			/* set the amount of data to fetch */
			packet->ActualLength =
				hdr->PayloadLen + HTC_HDR_LENGTH;
			if ((j == (num_messages - 1)) &&
			    ((hdr->Flags) & HTC_FLAGS_RECV_1MORE_BLOCK))
				packet->PktInfo.AsRx.HTCRxFlags |=
				HTC_RX_PKT_LAST_BUNDLED_PKT_HAS_ADDTIONAL_BLOCK;
			packet->Endpoint = hdr->EndpointID;
			packet->Completion = NULL;
		}

		if (QDF_IS_STATUS_ERROR(status))
			break;
	}

	UNLOCK_HIF_DEV_RX(pdev);

	/* for NO RESOURCE error, no need to flush data queue */
	if (QDF_IS_STATUS_ERROR(status)	&&
	    (status != QDF_STATUS_E_RESOURCES)) {
		while (!HTC_QUEUE_EMPTY(queue)) {
			qdf_nbuf_t netbuf;

			packet = htc_packet_dequeue(queue);
			if (!packet)
				break;
			netbuf = (qdf_nbuf_t)packet->pNetBufContext;
			if (netbuf)
				qdf_nbuf_free(netbuf);
		}
	}
	if (status == QDF_STATUS_E_RESOURCES)
		status = QDF_STATUS_SUCCESS;
	return status;
}

/**
 * hif_dev_process_trailer() - Process the receive frame trailer
 * @pdev : HIF device object
 * @buffer : The buffer containing the trailer
 * @length : Length of the buffer
 * @next_look_aheads : The lookahead that is next
 * @num_look_aheads : Number of lookahead information
 * @from_endpoint : The endpoint on which the trailer is received
 */
QDF_STATUS hif_dev_process_trailer(struct hif_sdio_device *pdev,
				   uint8_t *buffer, int length,
				   uint32_t *next_look_aheads,
				   int *num_look_aheads,
				   HTC_ENDPOINT_ID from_endpoint)
{
	int orig_length;
	QDF_STATUS status;
	uint8_t *record_buf;
	uint8_t *orig_buffer;
	HTC_RECORD_HDR *record;
	HTC_LOOKAHEAD_REPORT *look_ahead;

	hif_debug("%s: length:%d", __func__, length);

	orig_buffer = buffer;
	orig_length = length;
	status = QDF_STATUS_SUCCESS;

	while (length > 0) {
		if (length < sizeof(HTC_RECORD_HDR)) {
			status = QDF_STATUS_E_PROTO;
			break;
		}
		/* these are byte aligned structs */
		record = (HTC_RECORD_HDR *)buffer;
		length -= sizeof(HTC_RECORD_HDR);
		buffer += sizeof(HTC_RECORD_HDR);

		if (record->Length > length) {
			/* no room left in buffer for record */
			hif_err("%s: invalid record len: (%u, %u)",
				__func__, record->Length,
				record->RecordID);
			hif_err("%s: buffer has %d bytes left",
				__func__, length);
			status = QDF_STATUS_E_PROTO;
			break;
		}
		/* start of record follows the header */
		record_buf = buffer;

		switch (record->RecordID) {
		case HTC_RECORD_CREDITS:
			/* Process in HTC, ignore here */
			break;
		case HTC_RECORD_LOOKAHEAD:
			A_ASSERT(record->Length >= sizeof(*look_ahead));
			look_ahead = (HTC_LOOKAHEAD_REPORT *)record_buf;
			if ((look_ahead->PreValid ==
			     ((~look_ahead->PostValid) & 0xFF)) &&
			    next_look_aheads) {
				hif_debug("%s: look_ahead Report", __func__);
				hif_debug("%s:prevalid:0x%x, postvalid:0x%x",
					  __func__, look_ahead->PreValid,
					  look_ahead->PostValid);
				hif_debug("%s:from endpoint %d : %u",
					  __func__, from_endpoint,
					  look_ahead->LookAhead0);
				/* look ahead bytes are valid, copy them over */
				((uint8_t *)(&next_look_aheads[0]))[0] =
					look_ahead->LookAhead0;
				((uint8_t *)(&next_look_aheads[0]))[1] =
					look_ahead->LookAhead1;
				((uint8_t *)(&next_look_aheads[0]))[2] =
					look_ahead->LookAhead2;
				((uint8_t *)(&next_look_aheads[0]))[3] =
					look_ahead->LookAhead3;

				if (AR_DEBUG_LVL_CHECK(ATH_DEBUG_RECV)) {
					debug_dump_bytes((uint8_t *)
							 next_look_aheads, 4,
							 "Next Look Ahead");
				}
				/* just one normal lookahead */
				if (num_look_aheads)
					*num_look_aheads = 1;
			}
			break;
		case HTC_RECORD_LOOKAHEAD_BUNDLE:
			A_ASSERT(record->Length >=
					sizeof(HTC_BUNDLED_LOOKAHEAD_REPORT));
			if ((record->Length >=
			     sizeof(HTC_BUNDLED_LOOKAHEAD_REPORT)) &&
			    next_look_aheads) {
				HTC_BUNDLED_LOOKAHEAD_REPORT
				*pBundledLookAheadRpt;
				int i;

				pBundledLookAheadRpt =
				(HTC_BUNDLED_LOOKAHEAD_REPORT *)record_buf;

				if (AR_DEBUG_LVL_CHECK(ATH_DEBUG_RECV)) {
					debug_dump_bytes(record_buf,
							 record->Length,
							 "Bundle look_ahead");
				}

				if ((record->Length /
				     (sizeof(HTC_BUNDLED_LOOKAHEAD_REPORT)))
					> HTC_MAX_MSG_PER_BUNDLE_RX) {
					/* this should never happen, the target
					 * restricts the number of messages per
					 * bundle configured by the host
					 */
					A_ASSERT(false);
					status = QDF_STATUS_E_PROTO;
					break;
				}
				for (i = 0;
				     i <
				     (int)(record->Length /
					   (sizeof
					    (HTC_BUNDLED_LOOKAHEAD_REPORT)));
				     i++) {
					((uint8_t *)(&next_look_aheads[i]))[0] =
					   pBundledLookAheadRpt->LookAhead0;
					((uint8_t *)(&next_look_aheads[i]))[1] =
					   pBundledLookAheadRpt->LookAhead1;
					((uint8_t *)(&next_look_aheads[i]))[2] =
					   pBundledLookAheadRpt->LookAhead2;
					((uint8_t *)(&next_look_aheads[i]))[3] =
					   pBundledLookAheadRpt->LookAhead3;
					pBundledLookAheadRpt++;
				}
				if (num_look_aheads)
					*num_look_aheads = i;
			}
			break;
		default:
			hif_err("%s: HIF unhandled record: id:%u length:%u",
				__func__, record->RecordID, record->Length);
			break;
		}

		if (QDF_IS_STATUS_ERROR(status))
			break;

		/* advance buffer past this record for next time around */
		buffer += record->Length;
		length -= record->Length;
	}

	if (QDF_IS_STATUS_ERROR(status))
		debug_dump_bytes(orig_buffer, orig_length,
				 "BAD Recv Trailer");

	hif_debug("%s: status = %d", __func__, status);

	return status;
}

/* process a received message (i.e. strip off header,
 * process any trailer data).
 * note : locks must be released when this function is called
 */
QDF_STATUS hif_dev_process_recv_header(struct hif_sdio_device *pdev,
				       HTC_PACKET *packet,
				       uint32_t *next_look_aheads,
				       int *num_look_aheads)
{
	uint8_t temp;
	uint8_t *buf;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint16_t payloadLen;
	uint32_t look_ahead, actual_length;

	buf = packet->pBuffer;
	actual_length = packet->ActualLength;

	if (num_look_aheads)
		*num_look_aheads = 0;

	AR_DEBUG_PRINTF(ATH_DEBUG_RECV, ("+HTCProcessRecvHeader\n"));

	if (AR_DEBUG_LVL_CHECK(ATH_DEBUG_RECV))
		AR_DEBUG_PRINTBUF(buf, packet->ActualLength, "HTC Recv PKT");

	do {
		/* note, we cannot assume the alignment of pBuffer,
		 * so we use the safe macros to
		 * retrieve 16 bit fields
		 */
		payloadLen = HTC_GET_FIELD(buf, HTC_FRAME_HDR,
					   PAYLOADLEN);

		((uint8_t *)&look_ahead)[0] = buf[0];
		((uint8_t *)&look_ahead)[1] = buf[1];
		((uint8_t *)&look_ahead)[2] = buf[2];
		((uint8_t *)&look_ahead)[3] = buf[3];

		if (packet->PktInfo.AsRx.HTCRxFlags & HTC_RX_PKT_REFRESH_HDR) {
			/* refresh expected hdr, since this was unknown
			 * at the time we grabbed the packets
			 * as part of a bundle
			 */
			packet->PktInfo.AsRx.ExpectedHdr = look_ahead;
			/* refresh actual length since we now have the
			 * real header
			 */
			packet->ActualLength = payloadLen + HTC_HDR_LENGTH;

			/* validate the actual header that was refreshed  */
			if (packet->ActualLength > packet->BufferLength) {
				hif_err("%s: Bundled RECV Look ahead: 0x%X",
					__func__, look_ahead);
				hif_err("%s: Invalid HDR payload length(%d)",
					__func__,  payloadLen);
				/* limit this to max buffer just to print out
				 * some of the buffer
				 */
				packet->ActualLength =
					min(packet->ActualLength,
					    packet->BufferLength);
				status = QDF_STATUS_E_PROTO;
				break;
			}

			if (packet->Endpoint
			    != HTC_GET_FIELD(buf, HTC_FRAME_HDR, ENDPOINTID)) {
				hif_err("%s: Refreshed HDR EP (%d)",
					__func__,
					HTC_GET_FIELD(buf, HTC_FRAME_HDR,
						      ENDPOINTID));
				hif_err("%s: doesn't match expected EP (%d)",
					__func__, packet->Endpoint);
				status = QDF_STATUS_E_PROTO;
				break;
			}
		}

		if (look_ahead != packet->PktInfo.AsRx.ExpectedHdr) {
			/* somehow the lookahead that gave us the full read
			 * length did not reflect the actual header
			 * in the pending message
			 */
			hif_err("%s: lookahead mismatch!", __func__);
			hif_err("%s: pPkt:0x%lX flags:0x%X",
				__func__, (unsigned long)packet,
				packet->PktInfo.AsRx.HTCRxFlags);
			hif_err("%s: look_ahead 0x%08X != 0x%08X",
				__func__, look_ahead,
				packet->PktInfo.AsRx.ExpectedHdr);
#ifdef ATH_DEBUG_MODULE
			debug_dump_bytes((uint8_t *)&packet->PktInfo.AsRx.
					 ExpectedHdr, 4,
					 "Expected Message look_ahead");
			debug_dump_bytes(buf, sizeof(HTC_FRAME_HDR),
					 "Current Frame Header");
#ifdef HTC_CAPTURE_LAST_FRAME
			debug_dump_bytes((uint8_t *)&target->LastFrameHdr,
					 sizeof(HTC_FRAME_HDR),
					 "Last Frame Header");
			if (target->LastTrailerLength != 0)
				debug_dump_bytes(target->LastTrailer,
						 target->LastTrailerLength,
						 "Last trailer");
#endif
#endif
			status = QDF_STATUS_E_PROTO;
			break;
		}

		/* get flags */
		temp = HTC_GET_FIELD(buf, HTC_FRAME_HDR, FLAGS);

		if (temp & HTC_FLAGS_RECV_TRAILER) {
			/* this packet has a trailer */

			/* extract the trailer length in control byte 0 */
			temp = HTC_GET_FIELD(buf, HTC_FRAME_HDR, CONTROLBYTES0);

			if ((temp < sizeof(HTC_RECORD_HDR)) ||
			    (temp > payloadLen)) {
				hif_err("%s: invalid header",
					__func__);
				hif_err("%s: payloadlength should be :%d",
					__func__, payloadLen);
				hif_err("%s: But control bytes is :%d)",
					__func__, temp);
				status = QDF_STATUS_E_PROTO;
				break;
			}

			if (packet->PktInfo.AsRx.
			    HTCRxFlags & HTC_RX_PKT_IGNORE_LOOKAHEAD) {
				/* this packet was fetched as part of an HTC
				 * bundle as the lookahead is not valid.
				 * Next packet may have already been fetched as
				 * part of the bundle
				 */
				next_look_aheads = NULL;
				num_look_aheads = NULL;
			}

			/* process trailer data that follows HDR and
			 * application payload
			 */
			status =
			hif_dev_process_trailer(pdev,
						(buf + HTC_HDR_LENGTH +
						 payloadLen - temp),
						temp,
						next_look_aheads,
						num_look_aheads,
						packet->Endpoint);

			if (QDF_IS_STATUS_ERROR(status))
				break;
		}
	} while (false);

	if (QDF_IS_STATUS_ERROR(status)) {
		/* dump the whole packet */
		debug_dump_bytes(buf, packet->ActualLength,
				 "BAD HTC Recv PKT");
	} else {
		if (AR_DEBUG_LVL_CHECK(ATH_DEBUG_RECV)) {
			if (packet->ActualLength > 0) {
				AR_DEBUG_PRINTBUF(packet->pBuffer,
						  packet->ActualLength,
						  "HTC - Application Msg");
			}
		}
	}
	AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
			("-hif_dev_process_recv_header\n"));
	return status;
}

/**
 * hif_dev_free_recv_pkt() - Free the allocated recv packets in the queue
 * @recv_pkt_queue : The queue that contains the packets to be queued
 *
 * Return : NONE
 */
void hif_dev_free_recv_pkt_queue(HTC_PACKET_QUEUE *recv_pkt_queue)
{
	HTC_PACKET *packet;
	qdf_nbuf_t netbuf;

	while (!HTC_QUEUE_EMPTY(recv_pkt_queue)) {
		packet = htc_packet_dequeue(recv_pkt_queue);
		if (!packet)
			break;
		netbuf = (qdf_nbuf_t)packet->pNetBufContext;
		if (netbuf)
			qdf_nbuf_free(netbuf);
	}
}
