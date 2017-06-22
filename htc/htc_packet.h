/*
 * Copyright (c) 2013-2014, 2016-2017 The Linux Foundation. All rights reserved.
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

#ifndef HTC_PACKET_H_
#define HTC_PACKET_H_

#include <osdep.h>
#include "dl_list.h"

/* ------ Endpoint IDS ------ */
typedef enum {
	ENDPOINT_UNUSED = -1,
	ENDPOINT_0 = 0,
	ENDPOINT_1 = 1,
	ENDPOINT_2 = 2,
	ENDPOINT_3,
	ENDPOINT_4,
	ENDPOINT_5,
	ENDPOINT_6,
	ENDPOINT_7,
	ENDPOINT_8,
	ENDPOINT_MAX,
} HTC_ENDPOINT_ID;

struct _HTC_PACKET;

typedef void (*HTC_PACKET_COMPLETION)(void *, struct _HTC_PACKET *);

typedef uint16_t HTC_TX_TAG;

/**
 * struct htc_tx_packet_info - HTC TX packet information
 * @Tag: tag used to selective flush packets
 * @CreditsUsed: number of credits used for this TX packet (HTC internal)
 * @SendFlags: send flags (HTC internal)
 * @SeqNo: internal seq no for debugging (HTC internal)
 * @Flags: Internal use
 */
struct htc_tx_packet_info {
	HTC_TX_TAG Tag;
	int CreditsUsed;
	uint8_t SendFlags;
	int SeqNo;
	uint32_t Flags;
};

/**
 * HTC_TX_PACKET_TAG_XXX - #defines for tagging packets for special handling
 * HTC_TX_PACKET_TAG_ALL: zero is reserved and used to flush ALL packets
 * HTC_TX_PACKET_TAG_INTERNAL: internal tags start here
 * HTC_TX_PACKET_TAG_USER_DEFINED: user-defined tags start here
 * HTC_TX_PACKET_TAG_BUNDLED: indicate this is a bundled tx packet
 * HTC_TX_PACKET_TAG_AUTO_PM: indicate a power management wmi command
 */
#define HTC_TX_PACKET_TAG_ALL          0
#define HTC_TX_PACKET_TAG_INTERNAL     1
#define HTC_TX_PACKET_TAG_USER_DEFINED (HTC_TX_PACKET_TAG_INTERNAL + 9)
#define HTC_TX_PACKET_TAG_BUNDLED      (HTC_TX_PACKET_TAG_USER_DEFINED + 1)
#define HTC_TX_PACKET_TAG_AUTO_PM      (HTC_TX_PACKET_TAG_USER_DEFINED + 2)

/* Tag packet for runtime put after sending */
#define HTC_TX_PACKET_TAG_RUNTIME_PUT  (HTC_TX_PACKET_TAG_USER_DEFINED + 3)


#define HTC_TX_PACKET_FLAG_FIXUP_NETBUF (1 << 0)

/**
 * struct htc_rx_packet_info - HTC RX Packet information
 * @ExpectedHdr: HTC Internal use
 * @HTCRxFlags: HTC Internal use
 * @IndicationFlags: indication flags set on each RX packet indication
 */
struct htc_rx_packet_info {
	uint32_t ExpectedHdr;
	uint32_t HTCRxFlags;
	uint32_t IndicationFlags;
};

/* more packets on this endpoint are being fetched */
#define HTC_RX_FLAGS_INDICATE_MORE_PKTS  (1 << 0)
#define HTC_PACKET_MAGIC_COOKIE          0xdeadbeef

/* wrapper around endpoint-specific packets */
/**
 * struct _HTC_PACKET - HTC Packet data structure
 * @ListLink: double link
 * @pPktContext: caller's per packet specific context
 * @pBufferStart: The true buffer start, the caller can store the real buffer
 *                start here.  In receive callbacks, the HTC layer sets pBuffer
 *                to the start of the payload past the header. This field allows
 *                the caller to reset pBuffer when it recycles receive packets
 *                back to HTC
 * @pBuffer: payload start (RX/TX)
 * @BufferLength: length of buffer
 * @ActualLength: actual length of payload
 * @Endpoint: endpoint that this packet was sent/recv'd from
 * @Status: completion status
 * @PktInfo: Packet specific info
 * @netbufOrigHeadRoom: Original head room of skb
 * @Completion: completion
 * @pContext: HTC private completion context
 * @pNetBufContext: optimization for network-oriented data, the HTC packet can
 *                  pass the network buffer corresponding to the HTC packet
 *                  lower layers may optimized the transfer knowing this is a
 *                  network buffer
 * @magic_cookie: HTC Magic cookie
 */
typedef struct _HTC_PACKET {
	DL_LIST ListLink;
	void *pPktContext;
	uint8_t *pBufferStart;
	/*
	 * Pointer to the start of the buffer. In the transmit
	 * direction this points to the start of the payload. In the
	 * receive direction, however, the buffer when queued up
	 * points to the start of the HTC header but when returned
	 * to the caller points to the start of the payload
	 */
	uint8_t *pBuffer;
	uint32_t BufferLength;
	uint32_t ActualLength;
	HTC_ENDPOINT_ID Endpoint;
	QDF_STATUS Status;
	union {
		struct htc_tx_packet_info AsTx;
		struct htc_rx_packet_info AsRx;
	} PktInfo;
	/* the following fields are for internal HTC use */
	uint32_t netbufOrigHeadRoom;
	HTC_PACKET_COMPLETION Completion;
	void *pContext;
	void *pNetBufContext;
	uint32_t magic_cookie;
} HTC_PACKET;

#define COMPLETE_HTC_PACKET(p, status)	     \
	{					     \
		(p)->Status = (status);			 \
		(p)->Completion((p)->pContext, (p));	 \
	}

#define INIT_HTC_PACKET_INFO(p, b, len)		  \
	{						  \
		(p)->pBufferStart = (b);		      \
		(p)->BufferLength = (len);		      \
	}

/* macro to set an initial RX packet for refilling HTC */
#define SET_HTC_PACKET_INFO_RX_REFILL(p, c, b, len, ep) \
	do { \
		(p)->pPktContext = (c);			      \
		(p)->pBuffer = (b);			      \
		(p)->pBufferStart = (b);		      \
		(p)->BufferLength = (len);		      \
		(p)->Endpoint = (ep);			      \
	} while (0)

/* fast macro to recycle an RX packet that will be re-queued to HTC */
#define HTC_PACKET_RESET_RX(p)		    \
	{ (p)->pBuffer = (p)->pBufferStart; (p)->ActualLength = 0; }

/* macro to set packet parameters for TX */
#define SET_HTC_PACKET_INFO_TX(p, c, b, len, ep, tag)  \
	do {						  \
		(p)->pPktContext = (c);			      \
		(p)->pBuffer = (b);			      \
		(p)->ActualLength = (len);		      \
		(p)->Endpoint = (ep);			      \
		(p)->PktInfo.AsTx.Tag = (tag);		      \
		(p)->PktInfo.AsTx.Flags = 0;		      \
		(p)->PktInfo.AsTx.SendFlags = 0;	      \
	} while (0)

#define SET_HTC_PACKET_NET_BUF_CONTEXT(p, nb) \
	{ \
		(p)->pNetBufContext = (nb); \
	}

#define GET_HTC_PACKET_NET_BUF_CONTEXT(p)  (p)->pNetBufContext

/* HTC Packet Queueing Macros */
typedef struct _HTC_PACKET_QUEUE {
	DL_LIST QueueHead;
	int Depth;
} HTC_PACKET_QUEUE;

/* initialize queue */
#define INIT_HTC_PACKET_QUEUE(pQ)   \
	{				    \
		DL_LIST_INIT(&(pQ)->QueueHead); \
		(pQ)->Depth = 0;		\
	}

/* enqueue HTC packet to the tail of the queue */
#define HTC_PACKET_ENQUEUE(pQ, p)			\
	{   dl_list_insert_tail(&(pQ)->QueueHead, &(p)->ListLink); \
	    (pQ)->Depth++;					 \
	}

/* enqueue HTC packet to the tail of the queue */
#define HTC_PACKET_ENQUEUE_TO_HEAD(pQ, p)		\
	{   dl_list_insert_head(&(pQ)->QueueHead, &(p)->ListLink); \
	    (pQ)->Depth++;					 \
	}
/* test if a queue is empty */
#define HTC_QUEUE_EMPTY(pQ)       ((pQ)->Depth == 0)
/* get packet at head without removing it */
static inline HTC_PACKET *htc_get_pkt_at_head(HTC_PACKET_QUEUE *queue)
{
	if (queue->Depth == 0)
		return NULL;

	return A_CONTAINING_STRUCT((DL_LIST_GET_ITEM_AT_HEAD(
					&queue->QueueHead)),
				    HTC_PACKET, ListLink);
}

/* remove a packet from a queue, where-ever it is in the queue */
#define HTC_PACKET_REMOVE(pQ, p)	    \
	{				    \
		dl_list_remove(&(p)->ListLink);  \
		(pQ)->Depth--;			 \
	}

/* dequeue an HTC packet from the head of the queue */
static inline HTC_PACKET *htc_packet_dequeue(HTC_PACKET_QUEUE *queue)
{
	DL_LIST *pItem = dl_list_remove_item_from_head(&queue->QueueHead);

	if (pItem != NULL) {
		queue->Depth--;
		return A_CONTAINING_STRUCT(pItem, HTC_PACKET, ListLink);
	}
	return NULL;
}

/* dequeue an HTC packet from the tail of the queue */
static inline HTC_PACKET *htc_packet_dequeue_tail(HTC_PACKET_QUEUE *queue)
{
	DL_LIST *pItem = dl_list_remove_item_from_tail(&queue->QueueHead);

	if (pItem != NULL) {
		queue->Depth--;
		return A_CONTAINING_STRUCT(pItem, HTC_PACKET, ListLink);
	}
	return NULL;
}

#define HTC_PACKET_QUEUE_DEPTH(pQ) (pQ)->Depth

#define HTC_GET_ENDPOINT_FROM_PKT(p) (p)->Endpoint
#define HTC_GET_TAG_FROM_PKT(p)      (p)->PktInfo.AsTx.Tag

/* transfer the packets from one queue to the tail of another queue */
#define HTC_PACKET_QUEUE_TRANSFER_TO_TAIL(pQDest, pQSrc) \
	{ \
		dl_list_transfer_items_to_tail(&(pQDest)->QueueHead, \
					       &(pQSrc)->QueueHead); \
		(pQDest)->Depth += (pQSrc)->Depth; \
		(pQSrc)->Depth = 0; \
	}

/*
 * Transfer the packets from one queue to the head of another queue.
 * This xfer_to_head(q1,q2) is basically equivalent to xfer_to_tail(q2,q1),
 * but it updates the queue descriptor object for the initial queue to refer
 * to the concatenated queue.
 */
#define HTC_PACKET_QUEUE_TRANSFER_TO_HEAD(pQDest, pQSrc)  \
	{ \
		dl_list_transfer_items_to_head(&(pQDest)->QueueHead, \
					       &(pQSrc)->QueueHead); \
		(pQDest)->Depth += (pQSrc)->Depth; \
		(pQSrc)->Depth = 0; \
	}

/* fast version to init and add a single packet to a queue */
#define INIT_HTC_PACKET_QUEUE_AND_ADD(pQ, pP) \
	{					     \
		DL_LIST_INIT_AND_ADD(&(pQ)->QueueHead, &(pP)->ListLink)	\
		(pQ)->Depth = 1;					\
	}

#define HTC_PACKET_QUEUE_ITERATE_ALLOW_REMOVE(pQ, pPTemp) \
		ITERATE_OVER_LIST_ALLOW_REMOVE(&(pQ)->QueueHead, \
						(pPTemp), HTC_PACKET, ListLink)

#define HTC_PACKET_QUEUE_ITERATE_IS_VALID(pQ) ITERATE_IS_VALID(&(pQ)->QueueHead)
#define HTC_PACKET_QUEUE_ITERATE_RESET(pQ) ITERATE_RESET(&(pQ)->QueueHead)

#define HTC_PACKET_QUEUE_ITERATE_END ITERATE_END

/**
 * htc_packet_set_magic_cookie() - set magic cookie in htc packet
 * htc_pkt - pointer to htc packet
 * value - value to set in magic cookie
 *
 * This API sets the magic cookie passed in htc packet.
 *
 * Return : None
 */
static inline void htc_packet_set_magic_cookie(HTC_PACKET *htc_pkt,
			uint32_t value)
{
	htc_pkt->magic_cookie = value;
}

/**
 * htc_packet_set_magic_cookie() - get magic cookie in htc packet
 * htc_pkt - pointer to htc packet
 *
 * This API returns the magic cookie in htc packet.
 *
 * Return : magic cookie
 */
static inline uint32_t htc_packet_get_magic_cookie(HTC_PACKET *htc_pkt)
{
	return htc_pkt->magic_cookie;
}

#endif /*HTC_PACKET_H_ */
