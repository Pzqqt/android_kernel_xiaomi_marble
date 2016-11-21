/*
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
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
#include <qdf_threads.h>
#include <athdefs.h>
#include <qdf_net_types.h>
#include <a_types.h>
#include <athdefs.h>
#include <a_osapi.h>
#include <hif.h>
#include <htc_services.h>
#include "hif_sdio_internal.h"
#include <htc_internal.h>
#include "regtable_sdio.h"
#include "if_sdio.h"

#define NBUF_ALLOC_FAIL_WAIT_TIME 100

static void hif_dev_dump_registers(struct hif_sdio_device *pdev,
				struct MBOX_IRQ_PROC_REGISTERS *irq_proc_regs,
				struct MBOX_IRQ_ENABLE_REGISTERS *
				irq_enable_regs,
				struct MBOX_COUNTER_REGISTERS *
				mailbox_counter_registers)
{

	AR_DEBUG_PRINTF(ATH_DEBUG_ANY, ("RegTable->"));

	if (irq_proc_regs != NULL) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("HostIntStatus: 0x%x ",
				 irq_proc_regs->host_int_status));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("CPUIntStatus: 0x%x ",
				 irq_proc_regs->cpu_int_status));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("ErrorIntStatus: 0x%x ",
				 irq_proc_regs->error_int_status));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("CounterIntStatus: 0x%x ",
				 irq_proc_regs->counter_int_status));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("MboxFrame: 0x%x ",
				 irq_proc_regs->mbox_frame));

		AR_DEBUG_PRINTF(ATH_DEBUG_ANY, ("\nRegTable->"));

		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("RxLKAValid: 0x%x ",
				 irq_proc_regs->rx_lookahead_valid));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("RxLKA0: 0x%x",
				 irq_proc_regs->rx_lookahead[0]));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("RxLKA1: 0x%x ",
				 irq_proc_regs->rx_lookahead[1]));

		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("RxLKA2: 0x%x ",
				 irq_proc_regs->rx_lookahead[2]));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("RxLKA3: 0x%x",
				 irq_proc_regs->rx_lookahead[3]));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY, ("\nRegTable->"));

		if (pdev->MailBoxInfo.gmbox_address != 0) {
			/* if the target supports GMBOX hardware,
			 * dump some additional state */
			AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
					("GMBOX-HostIntStatus2:  0x%x ",
					 irq_proc_regs->host_int_status2));
			AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
					("GMBOX-RX-Avail: 0x%x ",
					 irq_proc_regs->gmbox_rx_avail));
		}
	}

	if (irq_enable_regs != NULL) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("Int Status Enable:         0x%x\n",
				 irq_enable_regs->int_status_enable));
		AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
				("Counter Int Status Enable: 0x%x\n",
				 irq_enable_regs->counter_int_status_enable));
	}

	if (mailbox_counter_registers != NULL) {
		int i;
		for (i = 0; i < 4; i++) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
					("Counter[%d]:               0x%x\n", i,
					 mailbox_counter_registers->
								counter[i]));
		}
	}
	AR_DEBUG_PRINTF(ATH_DEBUG_ANY,
			("<------------------------------->\n"));
}

static
QDF_STATUS hif_dev_alloc_and_prepare_rx_packets(struct hif_sdio_device *pdev,
						uint32_t look_aheads[],
						int messages,
						HTC_PACKET_QUEUE *queue)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	HTC_PACKET *packet;
	HTC_FRAME_HDR *hdr;
	int i, j;
	int num_messages;
	int full_length;
	bool no_recycle;

	/* lock RX while we assemble the packet buffers */
	LOCK_HIF_DEV_RX(pdev);

	for (i = 0; i < messages; i++) {

		hdr = (HTC_FRAME_HDR *) &look_aheads[i];
		if (hdr->EndpointID >= ENDPOINT_MAX) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("Invalid Endpoint in look-ahead: %d\n",
					 hdr->EndpointID));
			/* invalid endpoint */
			status = QDF_STATUS_E_PROTO;
			break;
		}

		if (hdr->PayloadLen > HTC_MAX_PAYLOAD_LENGTH) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("Payload length %d exceeds max HTC : %d !\n",
				 hdr->PayloadLen,
				 (uint32_t) HTC_MAX_PAYLOAD_LENGTH));
			status = QDF_STATUS_E_PROTO;
			break;
		}

		if ((hdr->Flags & HTC_FLAGS_RECV_BUNDLE_CNT_MASK) == 0) {
			/* HTC header only indicates 1 message to fetch */
			num_messages = 1;
		} else {
			/* HTC header indicates that every packet to follow
			 * has the same padded length so that it can
			 * be optimally fetched as a full bundle */
			num_messages =
				(hdr->Flags & HTC_FLAGS_RECV_BUNDLE_CNT_MASK)
				>> HTC_FLAGS_RECV_BUNDLE_CNT_SHIFT;
			/* the count doesn't include the starter frame, just
			 * a count of frames to follow */
			num_messages++;
			/* A_ASSERT(numMessages <= target->MaxMsgPerBundle); */
			AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
				("HTC header indicates :%d messages can be"
				 " fetched as a bundle\n",
				 num_messages));
		}

		full_length =
			DEV_CALC_RECV_PADDED_LEN(pdev,
						 hdr->PayloadLen +
						 sizeof(HTC_FRAME_HDR));

		/* get packet buffers for each message, if there was a
		 * bundle detected in the header,
		 * use pHdr as a template to fetch all packets in the bundle */
		for (j = 0; j < num_messages; j++) {

			/* reset flag, any packets allocated using the
			 * RecvAlloc() API cannot be recycled on cleanup,
			 * they must be explicitly returned */
			no_recycle = false;
			packet = hif_dev_alloc_rx_buffer(pdev);

			if (packet == NULL) {
				/* No error, simply need to mark that
				 * we are waiting for buffers. */
				pdev->RecvStateFlags |= HTC_RECV_WAIT_BUFFERS;
				/* pDev->EpWaitingForBuffers = pEndpoint->Id; */
				status = QDF_STATUS_E_RESOURCES;
				break;
			}
			/* AR_DEBUG_ASSERT(pPacket->Endpoint ==
					   pEndpoint->Id); */
			/* clear flags */
			packet->PktInfo.AsRx.HTCRxFlags = 0;
			packet->PktInfo.AsRx.IndicationFlags = 0;
			packet->Status = QDF_STATUS_SUCCESS;

			if (no_recycle)
				/* flag that these packets cannot be recycled,
				 * they have to be returned to the user */
				packet->PktInfo.AsRx.HTCRxFlags |=
					HTC_RX_PKT_NO_RECYCLE;
			/* add packet to queue (also incase we need to
			 * cleanup down below)  */
			HTC_PACKET_ENQUEUE(queue, packet);

			/*
			   if (HTC_STOPPING(target)) {
			   status = QDF_STATUS_E_CANCELED;
			   break;
			   }
			 */

			/* make sure  message can fit in the endpoint buffer */
			if ((uint32_t) full_length > packet->BufferLength) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("Payload Length Error : header reports payload"
				 " of: %d (%d) endpoint buffer size: %d\n",
					 hdr->PayloadLen, full_length,
					 packet->BufferLength));
				status = QDF_STATUS_E_PROTO;
				break;
			}

			if (j > 0) {
				/* for messages fetched in a bundle the expected
				 * lookahead is unknown as we are only using the
				 * lookahead of the first packet as a template
				 * of what to expect for lengths */
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
			packet->Endpoint = hdr->EndpointID;
			packet->Completion = NULL;
		}

		if (QDF_IS_STATUS_ERROR(status)) {
			if (QDF_STATUS_E_RESOURCES == status) {
				/* this is actually okay */
				status = QDF_STATUS_SUCCESS;
			}
			break;
		}

	}

	UNLOCK_HIF_DEV_RX(pdev);

	if (QDF_IS_STATUS_ERROR(status)) {
		while (!HTC_QUEUE_EMPTY(queue))
			packet = htc_packet_dequeue(queue);
	}

	return status;
}

static inline QDF_STATUS hif_dev_recv_packet(struct hif_sdio_device *pdev,
				   HTC_PACKET *packet,
				   uint32_t recv_length, uint8_t mbox_index)
{
	uint32_t padded_length;
	QDF_STATUS status;
	bool sync = (packet->Completion == NULL) ? true : false;

	/* adjust the length to be a multiple of block size if appropriate */
	padded_length = DEV_CALC_RECV_PADDED_LEN(pdev, recv_length);

	if (padded_length > packet->BufferLength) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("DevRecvPacket, Not enough space for"
				 " padlen:%d recvlen:%d bufferlen:%d\n",
				 padded_length, recv_length,
				 packet->BufferLength));
		if (packet->Completion != NULL) {
			COMPLETE_HTC_PACKET(packet, QDF_STATUS_E_INVAL);
			return QDF_STATUS_SUCCESS;
		}
		return QDF_STATUS_E_INVAL;
	}

	/* mailbox index is saved in Endpoint member */
	AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
			("hif_dev_recv_packet (0x%lX : hdr:0x%X) Len:%d,"
			 " Padded Length: %d Mbox:0x%X\n",
			 (unsigned long)packet,
			 packet->PktInfo.AsRx.ExpectedHdr, recv_length,
			 padded_length,
			 pdev->MailBoxInfo.mbox_addresses[mbox_index]));
	status = hif_read_write(pdev->HIFDevice,
				pdev->MailBoxInfo.mbox_addresses[mbox_index],
				packet->pBuffer, padded_length,
				(sync ? HIF_RD_SYNC_BLOCK_FIX :
							HIF_RD_ASYNC_BLOCK_FIX),
				sync ? NULL : packet);
	AR_DEBUG_PRINTF(ATH_DEBUG_RECV, ("EP%d, Seq:%d\n",
					 ((HTC_FRAME_HDR *) packet->pBuffer)->
					 EndpointID,
					 ((HTC_FRAME_HDR *) packet->pBuffer)->
					 ControlBytes1));
	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
			("hif_dev_recv_packet (0x%lX : hdr:0x%X) Failed\n",
			 (unsigned long)packet,
			 packet->PktInfo.AsRx.ExpectedHdr));
	}
	if (sync) {
		packet->Status = status;
		if (status == QDF_STATUS_SUCCESS) {
			HTC_FRAME_HDR *hdr =
				(HTC_FRAME_HDR *) packet->pBuffer;
			AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
				("hif_dev_recv_packet "
				 "EP:%d,Len:%d,Flag:%d,CB:0x%02X,0x%02X\n",
				 hdr->EndpointID, hdr->PayloadLen,
				 hdr->Flags, hdr->ControlBytes0,
				 hdr->ControlBytes1));
		}
	}

	return status;
}

static inline QDF_STATUS hif_dev_process_trailer(struct hif_sdio_device *pdev,
				       uint8_t *buffer, int length,
				       uint32_t *next_look_aheads,
				       int *num_look_aheads,
				       HTC_ENDPOINT_ID from_endpoint)
{
	HTC_RECORD_HDR *record;
	uint8_t *record_buf;
	HTC_LOOKAHEAD_REPORT *look_ahead;
	uint8_t *orig_buffer;
	int orig_length;
	QDF_STATUS status;

	AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
			("+htc_process_trailer (length:%d)\n", length));

	if (AR_DEBUG_LVL_CHECK(ATH_DEBUG_RECV))
		AR_DEBUG_PRINTBUF(buffer, length, "Recv Trailer");

	orig_buffer = buffer;
	orig_length = length;
	status = QDF_STATUS_SUCCESS;

	while (length > 0) {

		if (length < sizeof(HTC_RECORD_HDR)) {
			status = QDF_STATUS_E_PROTO;
			break;
		}
		/* these are byte aligned structs */
		record = (HTC_RECORD_HDR *) buffer;
		length -= sizeof(HTC_RECORD_HDR);
		buffer += sizeof(HTC_RECORD_HDR);

		if (record->Length > length) {
			/* no room left in buffer for record */
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				(" invalid record len: %d (id:%d) buffer has:"
				 "%d bytes left\n",
				 record->Length, record->RecordID,
				 length));
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
			AR_DEBUG_ASSERT(record->Length >=
					sizeof(HTC_LOOKAHEAD_REPORT));
			look_ahead = (HTC_LOOKAHEAD_REPORT *) record_buf;
			if ((look_ahead->PreValid ==
			     ((~look_ahead->PostValid) & 0xFF))
			    && (next_look_aheads != NULL)) {

				AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
					(" look_ahead Report (pre valid:0x%X,"
					" post valid:0x%X) %d %d\n",
					 look_ahead->PreValid,
					 look_ahead->PostValid,
					 from_endpoint,
					 look_ahead->LookAhead0));
				/* look ahead bytes are valid, copy them over */
				((uint8_t *) (&next_look_aheads[0]))[0] =
					look_ahead->LookAhead0;
				((uint8_t *) (&next_look_aheads[0]))[1] =
					look_ahead->LookAhead1;
				((uint8_t *) (&next_look_aheads[0]))[2] =
					look_ahead->LookAhead2;
				((uint8_t *) (&next_look_aheads[0]))[3] =
					look_ahead->LookAhead3;

				if (AR_DEBUG_LVL_CHECK(ATH_DEBUG_RECV)) {
					debug_dump_bytes((uint8_t *)
							 next_look_aheads, 4,
							 "Next Look Ahead");
				}
				/* just one normal lookahead */
				if (num_look_aheads != NULL)
					*num_look_aheads = 1;
			}
			break;
		case HTC_RECORD_LOOKAHEAD_BUNDLE:
			AR_DEBUG_ASSERT(record->Length >=
					sizeof(HTC_BUNDLED_LOOKAHEAD_REPORT));
			if (record->Length >=
			    sizeof(HTC_BUNDLED_LOOKAHEAD_REPORT)
			    && (next_look_aheads != NULL)) {
				HTC_BUNDLED_LOOKAHEAD_REPORT
				*pBundledLookAheadRpt;
				int i;

				pBundledLookAheadRpt =
				(HTC_BUNDLED_LOOKAHEAD_REPORT *) record_buf;

				if (AR_DEBUG_LVL_CHECK(ATH_DEBUG_RECV)) {
					debug_dump_bytes(record_buf,
							 record->Length,
							 "Bundle look_ahead");
				}

				if ((record->Length /
				     (sizeof(HTC_BUNDLED_LOOKAHEAD_REPORT)))
				    > HTC_MAX_MSG_PER_BUNDLE) {
					/* this should never happen, the target
					 * restricts the number of messages per
					 * bundle configured by the host */
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

				*num_look_aheads = i;
			}
			break;
		default:
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				(" HIF unhandled record: id:%d length:%d\n",
				 record->RecordID, record->Length));
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

	AR_DEBUG_PRINTF(ATH_DEBUG_RECV, ("-htc_process_trailer\n"));
	return status;

}

/* process a received message (i.e. strip off header,
 * process any trailer data).
 * note : locks must be released when this function is called */
static QDF_STATUS hif_dev_process_recv_header(struct hif_sdio_device *pdev,
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

	if (num_look_aheads != NULL)
		*num_look_aheads = 0;

	AR_DEBUG_PRINTF(ATH_DEBUG_RECV, ("+HTCProcessRecvHeader\n"));

	if (AR_DEBUG_LVL_CHECK(ATH_DEBUG_RECV))
		AR_DEBUG_PRINTBUF(buf, packet->ActualLength, "HTC Recv PKT");

	do {
		/* note, we cannot assume the alignment of pBuffer,
		 * so we use the safe macros to
		 * retrieve 16 bit fields */
		payloadLen = HTC_GET_FIELD(buf, HTC_FRAME_HDR,
					PAYLOADLEN);

		((uint8_t *) &look_ahead)[0] = buf[0];
		((uint8_t *) &look_ahead)[1] = buf[1];
		((uint8_t *) &look_ahead)[2] = buf[2];
		((uint8_t *) &look_ahead)[3] = buf[3];

		if (packet->PktInfo.AsRx.HTCRxFlags & HTC_RX_PKT_REFRESH_HDR) {
			/* refresh expected hdr, since this was unknown
			 * at the time we grabbed the packets
			 * as part of a bundle */
			packet->PktInfo.AsRx.ExpectedHdr = look_ahead;
			/* refresh actual length since we now have the
			 * real header */
			packet->ActualLength = payloadLen + HTC_HDR_LENGTH;

			/* validate the actual header that was refreshed  */
			if (packet->ActualLength > packet->BufferLength) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("Invalid  HDR payload length (%d)"
					 " in bundled RECV (hdr: 0x%X)\n",
						 payloadLen, look_ahead));
				/* limit this to max buffer just to print out
				 * some of the buffer */
				packet->ActualLength =
					min(packet->ActualLength,
					    packet->BufferLength);
				status = QDF_STATUS_E_PROTO;
				break;
			}

			if (packet->Endpoint
			    != HTC_GET_FIELD(buf, HTC_FRAME_HDR, ENDPOINTID)) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("Refreshed HDR endpoint (%d) does not "
					 " match expected endpoint (%d)\n",
					 HTC_GET_FIELD(buf,
							       HTC_FRAME_HDR,
							       ENDPOINTID),
						 packet->Endpoint));
				status = QDF_STATUS_E_PROTO;
				break;
			}
		}

		if (look_ahead != packet->PktInfo.AsRx.ExpectedHdr) {
			/* somehow the lookahead that gave us the full read
			 * length did not reflect the actual header
			 * in the pending message */
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			   ("hif_dev_process_recv_header, lookahead mismatch!"
			    " (pPkt:0x%lX flags:0x%X), 0x%08X != 0x%08X\n",
				 (unsigned long)packet,
				 packet->PktInfo.AsRx.HTCRxFlags,
				 look_ahead,
				 packet->PktInfo.AsRx.ExpectedHdr));
#ifdef ATH_DEBUG_MODULE
			debug_dump_bytes((uint8_t *) &packet->PktInfo.AsRx.
				 ExpectedHdr, 4,
				 "Expected Message look_ahead");
			debug_dump_bytes(buf, sizeof(HTC_FRAME_HDR),
				 "Current Frame Header");
#ifdef HTC_CAPTURE_LAST_FRAME
			debug_dump_bytes((uint8_t *) &target->LastFrameHdr,
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
			temp =
				HTC_GET_FIELD(buf, HTC_FRAME_HDR,
					CONTROLBYTES0);

			if ((temp < sizeof(HTC_RECORD_HDR))
			    || (temp > payloadLen)) {
				AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("hif_dev_process_recv_header, invalid header"
				 "(payloadlength should be :%d, CB[0] is:%d)\n",
					 payloadLen, temp));
				status = QDF_STATUS_E_PROTO;
				break;
			}

			if (packet->PktInfo.AsRx.
			    HTCRxFlags & HTC_RX_PKT_IGNORE_LOOKAHEAD) {
				/* this packet was fetched as part of an HTC
				 * bundle as the lookahead is not valid.
				 * Next packet may have already been fetched as
				 * part of the bundle */
				next_look_aheads = NULL;
				num_look_aheads = NULL;
			}

			/* process trailer data that follows HDR and
			 * application payload */
			status = hif_dev_process_trailer(pdev,
						 (buf + HTC_HDR_LENGTH +
						  payloadLen - temp), temp,
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

static QDF_STATUS hif_dev_issue_recv_packet_bundle(struct hif_sdio_device *pdev,
					 HTC_PACKET_QUEUE *recv_pkt_queue,
					 HTC_PACKET_QUEUE *
					 sync_completion_queue,
					 uint8_t mail_box_index,
					 int *num_packets_fetched,
					 bool partial_bundle)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int i, total_length = 0;
	unsigned char *bundle_buffer = NULL;
	HTC_PACKET *packet, *packet_rx_bundle;
	HTC_TARGET *target = NULL;
	uint32_t padded_length;

	int bundleSpaceRemaining = 0;
	target = (HTC_TARGET *) pdev->pTarget;

	if ((HTC_PACKET_QUEUE_DEPTH(recv_pkt_queue) - HTC_MAX_MSG_PER_BUNDLE) >
	    0) {
		partial_bundle = true;
		AR_DEBUG_PRINTF(ATH_DEBUG_WARN,
				("%s, partial bundle detected num: %d, %d\n",
				 __func__,
				 HTC_PACKET_QUEUE_DEPTH(recv_pkt_queue),
				 HTC_MAX_MSG_PER_BUNDLE));
	}

	bundleSpaceRemaining =
		HTC_MAX_MSG_PER_BUNDLE * target->TargetCreditSize;
	packet_rx_bundle = allocate_htc_bundle_packet(target);
	if (!packet_rx_bundle) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s: packet_rx_bundle is NULL\n", __func__));
		qdf_sleep(NBUF_ALLOC_FAIL_WAIT_TIME);  /* 100 msec sleep */
		return QDF_STATUS_E_NOMEM;
	}
	bundle_buffer = packet_rx_bundle->pBuffer;

	for (i = 0;
	     !HTC_QUEUE_EMPTY(recv_pkt_queue) && i < HTC_MAX_MSG_PER_BUNDLE;
	     i++) {
		packet = htc_packet_dequeue(recv_pkt_queue);
		A_ASSERT(packet != NULL);
		padded_length =
			DEV_CALC_RECV_PADDED_LEN(pdev, packet->ActualLength);

		if ((bundleSpaceRemaining - padded_length) < 0) {
			/* exceeds what we can transfer, put the packet back */
			HTC_PACKET_ENQUEUE_TO_HEAD(recv_pkt_queue, packet);
			break;
		}
		bundleSpaceRemaining -= padded_length;

		if (partial_bundle ||
			HTC_PACKET_QUEUE_DEPTH(recv_pkt_queue) > 0) {
			packet->PktInfo.AsRx.HTCRxFlags |=
				HTC_RX_PKT_IGNORE_LOOKAHEAD;
		}
		packet->PktInfo.AsRx.HTCRxFlags |= HTC_RX_PKT_PART_OF_BUNDLE;

		HTC_PACKET_ENQUEUE(sync_completion_queue, packet);

		total_length += padded_length;
	}
#ifdef DEBUG_BUNDLE
	qdf_print("Recv bundle count %d, length %d.\n",
		  HTC_PACKET_QUEUE_DEPTH(sync_completion_queue), total_length);
#endif

	status = hif_read_write(pdev->HIFDevice,
				pdev->MailBoxInfo.
				mbox_addresses[(int)mail_box_index],
				bundle_buffer, total_length,
				HIF_RD_SYNC_BLOCK_FIX, NULL);

	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s, hif_send Failed status:%d\n",
				 __func__, status));
	} else {
		unsigned char *buffer = bundle_buffer;
		*num_packets_fetched = i;
		HTC_PACKET_QUEUE_ITERATE_ALLOW_REMOVE(sync_completion_queue,
						      packet) {
			padded_length =
				DEV_CALC_RECV_PADDED_LEN(pdev,
							 packet->ActualLength);
			A_MEMCPY(packet->pBuffer, buffer, padded_length);
			buffer += padded_length;
		} HTC_PACKET_QUEUE_ITERATE_END;
	}
	/* free bundle space under Sync mode */
	free_htc_bundle_packet(target, packet_rx_bundle);
	return status;
}

QDF_STATUS hif_dev_recv_message_pending_handler(struct hif_sdio_device *pdev,
				      uint8_t mail_box_index,
				      uint32_t msg_look_aheads[],
				      int num_look_aheads,
				      bool *async_proc,
				      int *num_pkts_fetched)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	HTC_PACKET *packet;
	bool asyncProc = false;
	uint32_t look_aheads[HTC_MAX_MSG_PER_BUNDLE];
	int pkts_fetched;
	HTC_PACKET_QUEUE recv_pkt_queue, sync_completed_pkts_queue;
	bool partial_bundle;
	HTC_ENDPOINT_ID id;
	int total_fetched = 0;

	AR_DEBUG_PRINTF(ATH_DEBUG_RECV,
			("+HTCRecvMessagePendingHandler NumLookAheads: %d\n",
			 num_look_aheads));

	if (num_pkts_fetched != NULL)
		*num_pkts_fetched = 0;

	if (IS_DEV_IRQ_PROCESSING_ASYNC_ALLOWED(pdev)) {
		/* We use async mode to get the packets if the
		 * device layer supports it. The device layer
		 * interfaces with HIF in which HIF may have
		 * restrictions on how interrupts are processed */
		asyncProc = true;
	}

	if (async_proc != NULL)
		/* indicate to caller how we decided to process this */
		*async_proc = asyncProc;
	if (num_look_aheads > HTC_MAX_MSG_PER_BUNDLE) {
		A_ASSERT(false);
		return QDF_STATUS_E_PROTO;
	}
	A_MEMCPY(look_aheads, msg_look_aheads,
		(sizeof(uint32_t)) * num_look_aheads);
	while (true) {

		/* reset packets queues */
		INIT_HTC_PACKET_QUEUE(&recv_pkt_queue);
		INIT_HTC_PACKET_QUEUE(&sync_completed_pkts_queue);
		if (num_look_aheads > HTC_MAX_MSG_PER_BUNDLE) {
			status = QDF_STATUS_E_PROTO;
			A_ASSERT(false);
			break;
		}

		/* first lookahead sets the expected endpoint IDs for
		 * all packets in a bundle */
		id = ((HTC_FRAME_HDR *) &look_aheads[0])->EndpointID;

		if (id >= ENDPOINT_MAX) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("MsgPend, Invalid Endpoint in lookahead: %d\n",
				 id));
			status = QDF_STATUS_E_PROTO;
			break;
		}
		/* try to allocate as many HTC RX packets indicated
		 * by the lookaheads these packets are stored
		 * in the recvPkt queue */
		status = hif_dev_alloc_and_prepare_rx_packets(pdev,
							      look_aheads,
							      num_look_aheads,
							      &recv_pkt_queue);
		if (QDF_IS_STATUS_ERROR(status))
			break;
		total_fetched += HTC_PACKET_QUEUE_DEPTH(&recv_pkt_queue);

		/* we've got packet buffers for all we can currently fetch,
		 * this count is not valid anymore  */
		num_look_aheads = 0;
		partial_bundle = false;

		/* now go fetch the list of HTC packets */
		while (!HTC_QUEUE_EMPTY(&recv_pkt_queue)) {

			pkts_fetched = 0;
			if ((HTC_PACKET_QUEUE_DEPTH(&recv_pkt_queue) > 1)) {
				/* there are enough packets to attempt a bundle
				 * transfer and recv bundling is allowed  */
				status = hif_dev_issue_recv_packet_bundle(pdev,
						  &recv_pkt_queue,
						  asyncProc ?
						  NULL :
						  &sync_completed_pkts_queue,
						  mail_box_index,
						  &pkts_fetched,
						  partial_bundle);
				if (QDF_IS_STATUS_ERROR(status)) {
					while (!HTC_QUEUE_EMPTY(
							&recv_pkt_queue)) {
						qdf_nbuf_t netbuf;

						packet = htc_packet_dequeue(
							 &recv_pkt_queue);
						if (packet == NULL)
							break;
						netbuf = (qdf_nbuf_t) packet->
								pNetBufContext;
						if (netbuf)
							qdf_nbuf_free(netbuf);
					}
					break;
				}

				if (HTC_PACKET_QUEUE_DEPTH(&recv_pkt_queue) !=
					0) {
					/* we couldn't fetch all packets at one,
					 * time this creates a broken
					 * bundle  */
					partial_bundle = true;
				}
			}

			/* see if the previous operation fetched any
			 * packets using bundling */
			if (0 == pkts_fetched) {
				/* dequeue one packet */
				packet = htc_packet_dequeue(&recv_pkt_queue);
				A_ASSERT(packet != NULL);
				packet->Completion = NULL;

				if (HTC_PACKET_QUEUE_DEPTH(&recv_pkt_queue) >
				    0) {
					/* lookaheads in all packets except the
					 * last one in must be ignored */
					packet->PktInfo.AsRx.HTCRxFlags |=
						HTC_RX_PKT_IGNORE_LOOKAHEAD;
				}

				/* go fetch the packet */
				status =
					hif_dev_recv_packet(pdev, packet,
						    packet->ActualLength,
						    mail_box_index);
				if (QDF_IS_STATUS_ERROR(status))
					break;
				/* sent synchronously, queue this packet for
				 * synchronous completion */
				HTC_PACKET_ENQUEUE(&sync_completed_pkts_queue,
						   packet);
			}
		}

		/* synchronous handling */
		if (pdev->DSRCanYield) {
			/* for the SYNC case, increment count that tracks
			 * when the DSR should yield */
			pdev->CurrentDSRRecvCount++;
		}

		/* in the sync case, all packet buffers are now filled,
		 * we can process each packet, check lookahead , then repeat */

		/* unload sync completion queue */
		while (!HTC_QUEUE_EMPTY(&sync_completed_pkts_queue)) {
			uint8_t pipeid;
			qdf_nbuf_t netbuf;

			packet = htc_packet_dequeue(&sync_completed_pkts_queue);
			A_ASSERT(packet != NULL);

			num_look_aheads = 0;
			status =
				hif_dev_process_recv_header(pdev, packet,
							    look_aheads,
							    &num_look_aheads);
			if (QDF_IS_STATUS_ERROR(status))
				break;

			netbuf = (qdf_nbuf_t) packet->pNetBufContext;
			/* set data length */
			qdf_nbuf_put_tail(netbuf, packet->ActualLength);

			if (pdev->hif_callbacks.rxCompletionHandler) {
				pipeid =
					hif_dev_map_mail_box_to_pipe(pdev,
							mail_box_index,
							true);
				pdev->hif_callbacks.rxCompletionHandler(pdev->
								hif_callbacks.
								Context,
								netbuf,
								pipeid);
			}
		}
		if (QDF_IS_STATUS_ERROR(status))
			break;

		if (num_look_aheads == 0) {
			/* no more look aheads */
			break;
		}
		/* check whether other OS contexts have queued any WMI
		 * command/data for WLAN. This check is needed only if WLAN
		 * Tx and Rx happens in same thread context */
		/* A_CHECK_DRV_TX(); */
	}
	if (num_pkts_fetched != NULL)
		*num_pkts_fetched = total_fetched;

	AR_DEBUG_PRINTF(ATH_DEBUG_RECV, ("-HTCRecvMessagePendingHandler\n"));
	return status;
}

/**
 * hif_dev_service_cpu_interrupt() - service fatal interrupts
 * synchronously
 *
 * @pDev: hif sdio device context
 *
 * Return: QDF_STATUS_SUCCESS for success
 */
static QDF_STATUS hif_dev_service_cpu_interrupt(struct hif_sdio_device *pdev)
{
	QDF_STATUS status;
	uint8_t cpu_int_status;
	uint8_t reg_buffer[4];

	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ, ("CPU Interrupt\n"));
	cpu_int_status = pdev->IrqProcRegisters.cpu_int_status
			 & pdev->IrqEnableRegisters.cpu_int_status_enable;
	A_ASSERT(cpu_int_status);
	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
			("Valid interrupt source(s) in CPU_INT_STATUS: 0x%x\n",
			 cpu_int_status));

	/* Clear the interrupt */
	pdev->IrqProcRegisters.cpu_int_status &= ~cpu_int_status;

	/*set up the register transfer buffer to hit the register
	 * 4 times , this is done to make the access 4-byte aligned
	 * to mitigate issues with host bus interconnects that
	 * restrict bus transfer lengths to be a multiple of 4-bytes
	 * set W1C value to clear the interrupt, this hits the register
	 * first */
	reg_buffer[0] = cpu_int_status;
	/* the remaining 4 values are set to zero which have no-effect  */
	reg_buffer[1] = 0;
	reg_buffer[2] = 0;
	reg_buffer[3] = 0;

	status = hif_read_write(pdev->HIFDevice,
				CPU_INT_STATUS_ADDRESS,
				reg_buffer, 4, HIF_WR_SYNC_BYTE_FIX, NULL);

	A_ASSERT(status == QDF_STATUS_SUCCESS);

	/* The Interrupt sent to the Host is generated via bit0
	 * of CPU INT register */
	if (cpu_int_status & 0x1) {
		if (pdev && pdev->hif_callbacks.fwEventHandler)
			/* It calls into HTC which propagates this
			 * to ol_target_failure() */
			pdev->hif_callbacks.fwEventHandler(pdev->hif_callbacks.
						Context, QDF_STATUS_E_FAILURE);
	} else
		AR_DEBUG_PRINTF(ATH_DEBUG_ERROR,
				("%s: Unable to call fwEventHandler,"
				" invalid input arguments\n",
				 __func__));

	return status;
}

/**
 * hif_dev_service_error_interrupt() - service error interrupts
 * synchronously
 *
 * @pDev: hif sdio device context
 *
 * Return: QDF_STATUS_SUCCESS for success
 */
static QDF_STATUS hif_dev_service_error_interrupt(struct hif_sdio_device *pdev)
{
	QDF_STATUS status;
	uint8_t error_int_status;
	uint8_t reg_buffer[4];

	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ, ("Error Interrupt\n"));
	error_int_status = pdev->IrqProcRegisters.error_int_status & 0x0F;
	A_ASSERT(error_int_status);
	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
			("Valid interrupt source in ERROR_INT_STATUS: 0x%x\n",
			 error_int_status));

	if (ERROR_INT_STATUS_WAKEUP_GET(error_int_status)) {
		/* Wakeup */
		AR_DEBUG_PRINTF(ATH_DEBUG_IRQ, ("Error : Wakeup\n"));
	}

	if (ERROR_INT_STATUS_RX_UNDERFLOW_GET(error_int_status)) {
		/* Rx Underflow */
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR, ("Error : Rx Underflow\n"));
	}

	if (ERROR_INT_STATUS_TX_OVERFLOW_GET(error_int_status)) {
		/* Tx Overflow */
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR, ("Error : Tx Overflow\n"));
	}

	/* Clear the interrupt */
	pdev->IrqProcRegisters.error_int_status &= ~error_int_status;

	/* set up the register transfer buffer to hit the register
	 * 4 times , this is done to make the access 4-byte
	 * aligned to mitigate issues with host bus interconnects that
	 * restrict bus transfer lengths to be a multiple of 4-bytes */

	/* set W1C value to clear the interrupt */
	reg_buffer[0] = error_int_status;
	/* the remaining 4 values are set to zero which have no-effect  */
	reg_buffer[1] = 0;
	reg_buffer[2] = 0;
	reg_buffer[3] = 0;

	status = hif_read_write(pdev->HIFDevice,
				ERROR_INT_STATUS_ADDRESS,
				reg_buffer, 4, HIF_WR_SYNC_BYTE_FIX, NULL);

	A_ASSERT(status == QDF_STATUS_SUCCESS);
	return status;
}

/**
 * hif_dev_service_debug_interrupt() - service debug interrupts
 * synchronously
 *
 * @pDev: hif sdio device context
 *
 * Return: QDF_STATUS_SUCCESS for success
 */
static QDF_STATUS hif_dev_service_debug_interrupt(struct hif_sdio_device *pdev)
{
	uint32_t dummy;
	QDF_STATUS status;

	/* Send a target failure event to the application */
	AR_DEBUG_PRINTF(ATH_DEBUG_ERR, ("Target debug interrupt\n"));

	/* clear the interrupt , the debug error interrupt is
	 * counter 0 */
	/* read counter to clear interrupt */
	status = hif_read_write(pdev->HIFDevice,
				COUNT_DEC_ADDRESS,
				(uint8_t *) &dummy,
				4, HIF_RD_SYNC_BYTE_INC, NULL);

	A_ASSERT(status == QDF_STATUS_SUCCESS);
	return status;
}

/**
 * hif_dev_service_counter_interrupt() - service counter interrupts
 * synchronously
 *
 * @pDev: hif sdio device context
 *
 * Return: QDF_STATUS_SUCCESS for success
 */
static
QDF_STATUS hif_dev_service_counter_interrupt(struct hif_sdio_device *pdev)
{
	uint8_t counter_int_status;

	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ, ("Counter Interrupt\n"));

	counter_int_status = pdev->IrqProcRegisters.counter_int_status &
			     pdev->IrqEnableRegisters.counter_int_status_enable;

	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
			("Valid interrupt source in COUNTER_INT_STATUS: 0x%x\n",
			 counter_int_status));

	/* Check if the debug interrupt is pending
	 * NOTE: other modules like GMBOX may use the counter interrupt
	 * for credit flow control on other counters, we only need to
	 * check for the debug assertion counter interrupt */
	if (counter_int_status & AR6K_TARGET_DEBUG_INTR_MASK)
		return hif_dev_service_debug_interrupt(pdev);

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_dev_process_pending_irqs() - process pending interrupts
 * synchronously
 *
 * @pDev: hif sdio device context
 * @pDone: pending irq completion status
 * @pASyncProcessing: sync/async processing flag
 * Return: QDF_STATUS_SUCCESS for success
 */
static QDF_STATUS hif_dev_process_pending_irqs(struct hif_sdio_device *pdev,
					      bool *done,
					      bool *async_processing)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t host_int_status = 0;
	uint32_t look_ahead[MAILBOX_USED_COUNT];
	int i;

	qdf_mem_zero(&look_ahead, sizeof(look_ahead));
	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
			("+ProcessPendingIRQs: (dev: 0x%lX)\n",
			 (unsigned long)pdev));

	/* NOTE: the HIF implementation guarantees that the context
	 * of this call allows us to perform SYNCHRONOUS I/O,
	 * that is we can block, sleep or call any API that
	 * can block or switch thread/task ontexts.
	 * This is a fully schedulable context. */
	do {

		if (pdev->IrqEnableRegisters.int_status_enable == 0) {
			/* interrupt enables have been cleared, do not try
			 * to process any pending interrupts that
			 * may result in more bus transactions.
			 * The target may be unresponsive at this point. */
			break;
		}
		status = hif_read_write(pdev->HIFDevice,
					HOST_INT_STATUS_ADDRESS,
					(uint8_t *) &pdev->IrqProcRegisters,
					sizeof(pdev->IrqProcRegisters),
					HIF_RD_SYNC_BYTE_INC, NULL);

		if (QDF_IS_STATUS_ERROR(status))
			break;

		if (AR_DEBUG_LVL_CHECK(ATH_DEBUG_IRQ)) {
			hif_dev_dump_registers(pdev,
					       &pdev->IrqProcRegisters,
					       &pdev->IrqEnableRegisters,
					       &pdev->MailBoxCounterRegisters);
		}

		/* Update only those registers that are enabled */
		host_int_status = pdev->IrqProcRegisters.host_int_status
				  & pdev->IrqEnableRegisters.int_status_enable;

		/* only look at mailbox status if the HIF layer did not
		 * provide this function, on some HIF interfaces reading
		 * the RX lookahead is not valid to do */
		for (i = 0; i < MAILBOX_USED_COUNT; i++) {
			look_ahead[i] = 0;
			if (host_int_status & (1 << i)) {
				/* mask out pending mailbox value, we use
				 * "lookAhead" as the real flag for
				 * mailbox processing below */
				host_int_status &= ~(1 << i);
				if (pdev->IrqProcRegisters.
				    rx_lookahead_valid & (1 << i)) {
					/* mailbox has a message and the
					 * look ahead is valid */
					look_ahead[i] =
						pdev->
						IrqProcRegisters.rx_lookahead[
						MAILBOX_LOOKAHEAD_SIZE_IN_WORD *
						i];
				}
			}
		} /*end of for loop */
	} while (false);

	do {
		bool bLookAheadValid = false;
		/* did the interrupt status fetches succeed? */
		if (QDF_IS_STATUS_ERROR(status))
			break;

		for (i = 0; i < MAILBOX_USED_COUNT; i++) {
			if (look_ahead[i] != 0) {
				bLookAheadValid = true;
				break;
			}
		}

		if ((0 == host_int_status) && !bLookAheadValid) {
			/* nothing to process, the caller can use this
			 * to break out of a loop */
			*done = true;
			break;
		}

		if (bLookAheadValid) {
			for (i = 0; i < MAILBOX_USED_COUNT; i++) {
				int fetched = 0;
				if (look_ahead[i] == 0)
					continue;
				AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
					("Pending mailbox[%d] message, look_ahead: 0x%X\n",
					 i, look_ahead[i]));
				/* Mailbox Interrupt, the HTC layer may issue
				 * async requests to empty the mailbox...
				 * When emptying the recv mailbox we use the
				 * async handler from the completion routine of
				 * routine of the callers read request.
				 * This can improve performance by reducing
				 * the  context switching when we rapidly
				 * pull packets */
				status = hif_dev_recv_message_pending_handler(
							pdev, i,
							&look_ahead
							[i], 1,
							async_processing,
							&fetched);
				if (QDF_IS_STATUS_ERROR(status))
					break;

				if (!fetched) {
					/* HTC could not pull any messages out
					 * due to lack of resources force DSR
					 * handle to ack the interrupt */
					*async_processing = false;
					pdev->RecheckIRQStatusCnt = 0;
				}
			}
		}

		/* now handle the rest of them */
		AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
			(" Valid interrupt source for OTHER interrupts: 0x%x\n",
			 host_int_status));

		if (HOST_INT_STATUS_CPU_GET(host_int_status)) {
			/* CPU Interrupt */
			status = hif_dev_service_cpu_interrupt(pdev);
			if (QDF_IS_STATUS_ERROR(status))
				break;
		}

		if (HOST_INT_STATUS_ERROR_GET(host_int_status)) {
			/* Error Interrupt */
			status = hif_dev_service_error_interrupt(pdev);
			if (QDF_IS_STATUS_ERROR(status))
				break;
		}

		if (HOST_INT_STATUS_COUNTER_GET(host_int_status)) {
			/* Counter Interrupt */
			status = hif_dev_service_counter_interrupt(pdev);
			if (QDF_IS_STATUS_ERROR(status))
				break;
		}

	} while (false);

	/* an optimization to bypass reading the IRQ status registers
	 * unecessarily which can re-wake the target, if upper layers
	 * determine that we are in a low-throughput mode, we can
	 * rely on taking another interrupt rather than re-checking
	 * the status registers which can re-wake the target.
	 *
	 * NOTE : for host interfaces that use the special
	 * GetPendingEventsFunc, this optimization cannot be used due to
	 * possible side-effects.  For example, SPI requires the host
	 * to drain all messages from the mailbox before exiting
	  * the ISR routine. */
	if (!(*async_processing) && (pdev->RecheckIRQStatusCnt == 0)) {
		AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
			("Bypassing IRQ Status re-check, forcing done\n"));
		*done = true;
	}

	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
			("-ProcessPendingIRQs: (done:%d, async:%d) status=%d\n",
			 *done, *async_processing, status));

	return status;
}

#define DEV_CHECK_RECV_YIELD(pdev) \
	((pdev)->CurrentDSRRecvCount >= \
	 (pdev)->HifIRQYieldParams.recv_packet_yield_count)

/**
 * hif_dev_dsr_handler() - Synchronous interrupt handler
 *
 * @context: hif send context
 *
 * Return: 0 for success and non-zero for failure
 */
QDF_STATUS hif_dev_dsr_handler(void *context)
{
	struct hif_sdio_device *pdev = (struct hif_sdio_device *) context;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	bool done = false;
	bool async_proc = false;

	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
			("+DevDsrHandler: (dev: 0x%lX)\n",
			 (unsigned long)pdev));

	/* reset the recv counter that tracks when we need
	 * to yield from the DSR */
	pdev->CurrentDSRRecvCount = 0;
	/* reset counter used to flag a re-scan of IRQ
	 * status registers on the target */
	pdev->RecheckIRQStatusCnt = 0;

	while (!done) {
		status = hif_dev_process_pending_irqs(pdev, &done, &async_proc);
		if (QDF_IS_STATUS_ERROR(status))
			break;

		if (HIF_DEVICE_IRQ_SYNC_ONLY == pdev->HifIRQProcessingMode) {
			/* the HIF layer does not allow async IRQ processing,
			 * override the asyncProc flag */
			async_proc = false;
			/* this will cause us to re-enter ProcessPendingIRQ()
			 * and re-read interrupt status registers.
			 * This has a nice side effect of blocking us until all
			 * async read requests are completed. This behavior is
			 * required as we  do not allow ASYNC processing
			 * in interrupt handlers (like Windows CE) */

			if (pdev->DSRCanYield && DEV_CHECK_RECV_YIELD(pdev))
				/* ProcessPendingIRQs() pulled enough recv
				 * messages to satisfy the yield count, stop
				 * checking for more messages and return */
				break;
		}

		if (async_proc) {
			/* the function does some async I/O for performance,
			 * we need to exit the ISR immediately, the check below
			 * will prevent the interrupt from being
			 * Ack'd while we handle it asynchronously */
			break;
		}

	}

	if (QDF_IS_STATUS_SUCCESS(status) && !async_proc) {
		/* Ack the interrupt only if :
		 *  1. we did not get any errors in processing interrupts
		 *  2. there are no outstanding async processing requests */
		if (pdev->DSRCanYield) {
			/* if the DSR can yield do not ACK the interrupt, there
			 * could be more pending messages. The HIF layer
			 * must ACK the interrupt on behalf of HTC */
			AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
				(" Yield in effect (cur RX count: %d)\n",
				 pdev->CurrentDSRRecvCount));
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_IRQ,
				(" Acking interrupt from DevDsrHandler\n"));
			hif_ack_interrupt(pdev->HIFDevice);
		}
	}

	AR_DEBUG_PRINTF(ATH_DEBUG_IRQ, ("-DevDsrHandler\n"));
	return status;
}
