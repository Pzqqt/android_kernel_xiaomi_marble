/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

/**
 * DOC: i_cdf_nbuf.h
 *
 * Linux implementation of skbuf
 */
#ifndef _I_CDF_NET_BUF_H
#define _I_CDF_NET_BUF_H

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/dma-mapping.h>
#include <linux/types.h>
#include <linux/scatterlist.h>
#include <cdf_types.h>
#include <cdf_status.h>

#define __CDF_NBUF_NULL   NULL


/*
 * Use socket buffer as the underlying implentation as skbuf .
 * Linux use sk_buff to represent both packet and data,
 * so we use sk_buffer to represent both skbuf .
 */
typedef struct sk_buff *__cdf_nbuf_t;

typedef void (*__cdf_nbuf_callback_fn)(struct sk_buff *skb);
#define OSDEP_EAPOL_TID 6       /* send it on VO queue */

/* CVG_NBUF_MAX_OS_FRAGS -
 * max tx fragments provided by the OS
 */
#define CVG_NBUF_MAX_OS_FRAGS 1

/* CVG_NBUF_MAX_EXTRA_FRAGS -
 * max tx fragments added by the driver
 * The driver will always add one tx fragment (the tx descriptor) and may
 * add a second tx fragment (e.g. a TSO segment's modified IP header).
 */
#define CVG_NBUF_MAX_EXTRA_FRAGS 2

typedef void (*cdf_nbuf_trace_update_t)(char *);

/**
 * struct cvg_nbuf_cb - network buffer control block
 * @data_attr: Value that is programmed in CE descriptor, contains:
 *		1) CE classification enablement bit
 *		2) Pkt type (802.3 or Ethernet Type II)
 *		3) Pkt Offset (Usually the length of HTT/HTC desc.)
 * @trace: info for DP tracing
 * @mapped_paddr_lo: DMA mapping info
 * @extra_frags: Extra tx fragments
 * @owner_id: Owner id
 * @cdf_nbuf_callback_fn: Callback function
 * @priv_data: IPA specific priv data
 * @proto_type: Protocol type
 * @vdev_id: vdev id
 * @tx_htt2_frm: HTT 2 frame
 * @tx_htt2_reserved: HTT 2 reserved bits
 */
struct cvg_nbuf_cb {
	uint32_t data_attr;
	/*
	 * Store info for data path tracing
	 */
	struct {
		uint8_t packet_state;
		uint8_t packet_track;
		uint8_t dp_trace;
	} trace;

	/*
	 * Store the DMA mapping info for the network buffer fragments
	 * provided by the OS.
	 */
	uint32_t mapped_paddr_lo[CVG_NBUF_MAX_OS_FRAGS];
#ifdef DEBUG_RX_RING_BUFFER
	uint32_t map_index;
#endif

	/* store extra tx fragments provided by the driver */
	struct {
		/* vaddr -
		 * CPU address (a.k.a. virtual address) of the tx fragments
		 * added by the driver
		 */
		unsigned char *vaddr[CVG_NBUF_MAX_EXTRA_FRAGS];
		/* paddr_lo -
		 * bus address (a.k.a. physical address) of the tx fragments
		 * added by the driver
		 */
		uint32_t paddr_lo[CVG_NBUF_MAX_EXTRA_FRAGS];
		uint16_t len[CVG_NBUF_MAX_EXTRA_FRAGS];
		uint8_t num;    /* how many extra frags has the driver added */
		uint8_t
		/*
		 * Store a wordstream vs. bytestream flag for each extra
		 * fragment, plus one more flag for the original fragment(s)
		 * of the netbuf.
		 */
wordstream_flags:CVG_NBUF_MAX_EXTRA_FRAGS + 1;
	} extra_frags;
	uint32_t owner_id;
	__cdf_nbuf_callback_fn cdf_nbuf_callback_fn;
	unsigned long priv_data;
#ifdef QCA_PKT_PROTO_TRACE
	unsigned char proto_type;
	unsigned char vdev_id;
#endif /* QCA_PKT_PROTO_TRACE */
#ifdef QCA_TX_HTT2_SUPPORT
	unsigned char tx_htt2_frm:1;
	unsigned char tx_htt2_reserved:7;
#endif /* QCA_TX_HTT2_SUPPORT */
};
#ifdef DEBUG_RX_RING_BUFFER
#define NBUF_MAP_ID(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->map_index)
#endif
#define NBUF_OWNER_ID(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->owner_id)
#define NBUF_OWNER_PRIV_DATA(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->priv_data)
#define NBUF_CALLBACK_FN(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->cdf_nbuf_callback_fn)
#define NBUF_CALLBACK_FN_EXEC(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->cdf_nbuf_callback_fn)(skb)
#define NBUF_MAPPED_PADDR_LO(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->mapped_paddr_lo[0])
#define NBUF_NUM_EXTRA_FRAGS(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->extra_frags.num)
#define NBUF_EXTRA_FRAG_VADDR(skb, frag_num) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->extra_frags.vaddr[(frag_num)])
#define NBUF_EXTRA_FRAG_PADDR_LO(skb, frag_num)	\
	(((struct cvg_nbuf_cb *)((skb)->cb))->extra_frags.paddr_lo[(frag_num)])
#define NBUF_EXTRA_FRAG_LEN(skb, frag_num) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->extra_frags.len[(frag_num)])
#define NBUF_EXTRA_FRAG_WORDSTREAM_FLAGS(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->extra_frags.wordstream_flags)

#ifdef QCA_PKT_PROTO_TRACE
#define NBUF_SET_PROTO_TYPE(skb, proto_type) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->proto_type = proto_type)
#define NBUF_GET_PROTO_TYPE(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->proto_type)
#else
#define NBUF_SET_PROTO_TYPE(skb, proto_type);
#define NBUF_GET_PROTO_TYPE(skb) 0;
#endif /* QCA_PKT_PROTO_TRACE */

#ifdef QCA_TX_HTT2_SUPPORT
#define NBUF_SET_TX_HTT2_FRM(skb, candi) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->tx_htt2_frm = candi)
#define NBUF_GET_TX_HTT2_FRM(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->tx_htt2_frm)
#else
#define NBUF_SET_TX_HTT2_FRM(skb, candi)
#define NBUF_GET_TX_HTT2_FRM(skb) 0
#endif /* QCA_TX_HTT2_SUPPORT */

#define NBUF_DATA_ATTR_SET(skb, data_attr)	\
	(((struct cvg_nbuf_cb *)((skb)->cb))->data_attr = data_attr)

#define NBUF_DATA_ATTR_GET(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->data_attr)

#if defined(FEATURE_LRO)
/**
 * struct nbuf_rx_cb - network buffer control block
 * on the receive path of the skb
 * @lro_eligible: indicates whether the msdu is LRO eligible
 * @tcp_proto: indicates if this is a TCP packet
 * @ipv6_proto: indicates if this is an IPv6 packet
 * @ip_offset: offset to the IP header
 * @tcp_offset: offset to the TCP header
 * @tcp_udp_chksum: TCP payload checksum
 * @tcp_seq_num: TCP sequence number
 * @tcp_ack_num: TCP acknowledgement number
 * @flow_id_toeplitz: 32 bit 5-tuple flow id toeplitz hash
 */
struct nbuf_rx_cb {
	uint32_t lro_eligible:1,
		tcp_proto:1,
		tcp_pure_ack:1,
		ipv6_proto:1,
		ip_offset:7,
		tcp_offset:7;
	uint32_t tcp_udp_chksum:16,
		tcp_win:16;
	uint32_t tcp_seq_num;
	uint32_t tcp_ack_num;
	uint32_t flow_id_toeplitz;
};

#define NBUF_LRO_ELIGIBLE(skb) \
	(((struct nbuf_rx_cb *)((skb)->cb))->lro_eligible)
#define NBUF_TCP_PROTO(skb) \
	(((struct nbuf_rx_cb *)((skb)->cb))->tcp_proto)
#define NBUF_TCP_PURE_ACK(skb) \
	(((struct nbuf_rx_cb *)((skb)->cb))->tcp_pure_ack)
#define NBUF_IPV6_PROTO(skb) \
	(((struct nbuf_rx_cb *)((skb)->cb))->ipv6_proto)
#define NBUF_IP_OFFSET(skb) \
	(((struct nbuf_rx_cb *)((skb)->cb))->ip_offset)
#define NBUF_TCP_OFFSET(skb) \
	(((struct nbuf_rx_cb *)((skb)->cb))->tcp_offset)
#define NBUF_TCP_CHKSUM(skb) \
	(((struct nbuf_rx_cb *)((skb)->cb))->tcp_udp_chksum)
#define NBUF_TCP_SEQ_NUM(skb) \
	(((struct nbuf_rx_cb *)((skb)->cb))->tcp_seq_num)
#define NBUF_TCP_ACK_NUM(skb) \
	(((struct nbuf_rx_cb *)((skb)->cb))->tcp_ack_num)
#define NBUF_TCP_WIN(skb) \
	(((struct nbuf_rx_cb *)((skb)->cb))->tcp_win)
#define NBUF_FLOW_ID_TOEPLITZ(skb)	\
	(((struct nbuf_rx_cb *)((skb)->cb))->flow_id_toeplitz)
#endif /* FEATURE_LRO */

#define NBUF_SET_PACKET_STATE(skb, pkt_state) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->trace.packet_state = \
								pkt_state)
#define NBUF_GET_PACKET_STATE(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->trace.packet_state)

#define NBUF_SET_PACKET_TRACK(skb, pkt_track) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->trace.packet_track = \
								pkt_track)
#define NBUF_GET_PACKET_TRACK(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->trace.packet_track)

#define NBUF_UPDATE_TX_PKT_COUNT(skb, PACKET_STATE) \
	cdf_nbuf_set_state(skb, PACKET_STATE)

#define CDF_NBUF_SET_DP_TRACE(skb, enable) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->trace.dp_trace \
								= enable)
#define CDF_NBUF_GET_DP_TRACE(skb) \
	(((struct cvg_nbuf_cb *)((skb)->cb))->trace.dp_trace)

#define __cdf_nbuf_get_num_frags(skb)		   \
	/* assume the OS provides a single fragment */ \
	(NBUF_NUM_EXTRA_FRAGS(skb) + 1)

#if defined(FEATURE_TSO)
#define __cdf_nbuf_dec_num_frags(skb)		   \
	(NBUF_NUM_EXTRA_FRAGS(skb)--)
#endif

#define __cdf_nbuf_frag_push_head( \
		skb, frag_len, frag_vaddr, frag_paddr_lo, frag_paddr_hi) \
	do { \
		int frag_num = NBUF_NUM_EXTRA_FRAGS(skb)++; \
		NBUF_EXTRA_FRAG_VADDR(skb, frag_num) = frag_vaddr; \
		NBUF_EXTRA_FRAG_PADDR_LO(skb, frag_num) = frag_paddr_lo; \
		NBUF_EXTRA_FRAG_LEN(skb, frag_num) = frag_len; \
	} while (0)

#define __cdf_nbuf_get_frag_len(skb, frag_num)		 \
	((frag_num < NBUF_NUM_EXTRA_FRAGS(skb)) ?	     \
	 NBUF_EXTRA_FRAG_LEN(skb, frag_num) : (skb)->len)

#define __cdf_nbuf_get_frag_vaddr(skb, frag_num)	      \
	((frag_num < NBUF_NUM_EXTRA_FRAGS(skb)) ?		  \
	 NBUF_EXTRA_FRAG_VADDR(skb, frag_num) : ((skb)->data))

#define __cdf_nbuf_get_frag_paddr_lo(skb, frag_num)		 \
	((frag_num < NBUF_NUM_EXTRA_FRAGS(skb)) ?		     \
	 NBUF_EXTRA_FRAG_PADDR_LO(skb, frag_num) :		  \
	/* assume that the OS only provides a single fragment */ \
	 NBUF_MAPPED_PADDR_LO(skb))

#define __cdf_nbuf_get_frag_is_wordstream(skb, frag_num) \
	((frag_num < NBUF_NUM_EXTRA_FRAGS(skb)) ?	     \
	 (NBUF_EXTRA_FRAG_WORDSTREAM_FLAGS(skb) >>	  \
	  (frag_num)) & 0x1 :			       \
	 (NBUF_EXTRA_FRAG_WORDSTREAM_FLAGS(skb) >>	  \
	  (CVG_NBUF_MAX_EXTRA_FRAGS)) & 0x1)

#define __cdf_nbuf_set_frag_is_wordstream(skb, frag_num, is_wordstream)	\
	do {								    \
		if (frag_num >= NBUF_NUM_EXTRA_FRAGS(skb)) {			\
			frag_num = CVG_NBUF_MAX_EXTRA_FRAGS;			    \
		}								\
		/* clear the old value */					\
		NBUF_EXTRA_FRAG_WORDSTREAM_FLAGS(skb) &= ~(1 << frag_num);	\
		/* set the new value */						\
		NBUF_EXTRA_FRAG_WORDSTREAM_FLAGS(skb) |=			\
			((is_wordstream) << frag_num);				    \
	} while (0)

#define __cdf_nbuf_trace_set_proto_type(skb, proto_type) \
	NBUF_SET_PROTO_TYPE(skb, proto_type)
#define __cdf_nbuf_trace_get_proto_type(skb) \
	NBUF_GET_PROTO_TYPE(skb);

/**
 * __cdf_nbuf_data_attr_get() -  Retrieves the data_attr value
 *				 from cvg_nbuf_cb (skb->cb)
 * @skb: Pointer to struct sk_buff
 *
 * Return: data_attr
 */
#define __cdf_nbuf_data_attr_get(skb)		\
	NBUF_DATA_ATTR_GET(skb)

/**
 * __cdf_nbuf_data_attr_set()  -  Sets the data_attr value
 *				  in cvg_nbuf_cb (skb->cb)
 * @skb: Pointer to struct sk_buff
 * @data_attr: packet type from the enum cdf_txrx_pkt_type
 *
 * Return:
 */
static inline void
__cdf_nbuf_data_attr_set(struct sk_buff *skb,
			     uint32_t data_attr)
{
	NBUF_DATA_ATTR_SET(skb, data_attr);
}

/**
 * typedef struct __cdf_nbuf_queue_t -  network buffer queue
 * @head: Head pointer
 * @tail: Tail pointer
 * @qlen: Queue length
 */
typedef struct __cdf_nbuf_qhead {
	struct sk_buff *head;
	struct sk_buff *tail;
	unsigned int qlen;
} __cdf_nbuf_queue_t;

/*
 * Use sk_buff_head as the implementation of cdf_nbuf_queue_t.
 * Because the queue head will most likely put in some structure,
 * we don't use pointer type as the definition.
 */

/*
 * prototypes. Implemented in cdf_nbuf.c
 */
__cdf_nbuf_t __cdf_nbuf_alloc(__cdf_device_t osdev, size_t size, int reserve,
			      int align, int prio);
void __cdf_nbuf_free(struct sk_buff *skb);
CDF_STATUS __cdf_nbuf_map(__cdf_device_t osdev,
			  struct sk_buff *skb, cdf_dma_dir_t dir);
void __cdf_nbuf_unmap(__cdf_device_t osdev,
		      struct sk_buff *skb, cdf_dma_dir_t dir);
CDF_STATUS __cdf_nbuf_map_single(__cdf_device_t osdev,
				 struct sk_buff *skb, cdf_dma_dir_t dir);
void __cdf_nbuf_unmap_single(__cdf_device_t osdev,
			     struct sk_buff *skb, cdf_dma_dir_t dir);
void __cdf_nbuf_reg_trace_cb(cdf_nbuf_trace_update_t cb_func_ptr);

#ifdef QCA_PKT_PROTO_TRACE
void __cdf_nbuf_trace_update(struct sk_buff *buf, char *event_string);
#else
#define __cdf_nbuf_trace_update(skb, event_string)
#endif /* QCA_PKT_PROTO_TRACE */

/**
 * __cdf_os_to_status() - OS to CDF status conversion
 * @error : OS error
 *
 * Return: CDF status
 */
static inline CDF_STATUS __cdf_os_to_status(signed int error)
{
	switch (error) {
	case 0:
		return CDF_STATUS_SUCCESS;
	case ENOMEM:
	case -ENOMEM:
		return CDF_STATUS_E_NOMEM;
	default:
		return CDF_STATUS_E_NOSUPPORT;
	}
}

/**
 * __cdf_nbuf_len() - return the amount of valid data in the skb
 * @skb: Pointer to network buffer
 *
 * This API returns the amount of valid data in the skb, If there are frags
 * then it returns total length.
 *
 * Return: network buffer length
 */
static inline size_t __cdf_nbuf_len(struct sk_buff *skb)
{
	int i, extra_frag_len = 0;

	i = NBUF_NUM_EXTRA_FRAGS(skb);
	while (i-- > 0)
		extra_frag_len += NBUF_EXTRA_FRAG_LEN(skb, i);

	return extra_frag_len + skb->len;
}

/**
 * __cdf_nbuf_cat() - link two nbufs
 * @dst: Buffer to piggyback into
 * @src: Buffer to put
 *
 * Link tow nbufs the new buf is piggybacked into the older one. The older
 * (src) skb is released.
 *
 * Return: CDF_STATUS (status of the call) if failed the src skb
 *	   is released
 */
static inline CDF_STATUS
__cdf_nbuf_cat(struct sk_buff *dst, struct sk_buff *src)
{
	CDF_STATUS error = 0;

	cdf_assert(dst && src);

	/*
	 * Since pskb_expand_head unconditionally reallocates the skb->head
	 * buffer, first check whether the current buffer is already large
	 * enough.
	 */
	if (skb_tailroom(dst) < src->len) {
		error = pskb_expand_head(dst, 0, src->len, GFP_ATOMIC);
		if (error)
			return __cdf_os_to_status(error);
	}
	memcpy(skb_tail_pointer(dst), src->data, src->len);

	skb_put(dst, src->len);
	dev_kfree_skb_any(src);

	return __cdf_os_to_status(error);
}

/**************************nbuf manipulation routines*****************/

/**
 * __cdf_nbuf_headroom() - return the amount of tail space available
 * @buf: Pointer to network buffer
 *
 * Return: amount of tail room
 */
static inline int __cdf_nbuf_headroom(struct sk_buff *skb)
{
	return skb_headroom(skb);
}

/**
 * __cdf_nbuf_tailroom() - return the amount of tail space available
 * @buf: Pointer to network buffer
 *
 * Return: amount of tail room
 */
static inline uint32_t __cdf_nbuf_tailroom(struct sk_buff *skb)
{
	return skb_tailroom(skb);
}

/**
 * __cdf_nbuf_push_head() - Push data in the front
 * @skb: Pointer to network buffer
 * @size: size to be pushed
 *
 * Return: New data pointer of this buf after data has been pushed,
 *	   or NULL if there is not enough room in this buf.
 */
static inline uint8_t *__cdf_nbuf_push_head(struct sk_buff *skb, size_t size)
{
	if (NBUF_MAPPED_PADDR_LO(skb))
		NBUF_MAPPED_PADDR_LO(skb) -= size;

	return skb_push(skb, size);
}

/**
 * __cdf_nbuf_put_tail() - Puts data in the end
 * @skb: Pointer to network buffer
 * @size: size to be pushed
 *
 * Return: data pointer of this buf where new data has to be
 *	   put, or NULL if there is not enough room in this buf.
 */
static inline uint8_t *__cdf_nbuf_put_tail(struct sk_buff *skb, size_t size)
{
	if (skb_tailroom(skb) < size) {
		if (unlikely(pskb_expand_head(skb, 0,
			size - skb_tailroom(skb), GFP_ATOMIC))) {
			dev_kfree_skb_any(skb);
			return NULL;
		}
	}
	return skb_put(skb, size);
}

/**
 * __cdf_nbuf_pull_head() - pull data out from the front
 * @skb: Pointer to network buffer
 * @size: size to be popped
 *
 * Return: New data pointer of this buf after data has been popped,
 *	   or NULL if there is not sufficient data to pull.
 */
static inline uint8_t *__cdf_nbuf_pull_head(struct sk_buff *skb, size_t size)
{
	if (NBUF_MAPPED_PADDR_LO(skb))
		NBUF_MAPPED_PADDR_LO(skb) += size;

	return skb_pull(skb, size);
}

/**
 * __cdf_nbuf_trim_tail() - trim data out from the end
 * @skb: Pointer to network buffer
 * @size: size to be popped
 *
 * Return: none
 */
static inline void __cdf_nbuf_trim_tail(struct sk_buff *skb, size_t size)
{
	return skb_trim(skb, skb->len - size);
}

/*********************nbuf private buffer routines*************/

/**
 * __cdf_nbuf_peek_header() - return the header's addr & m_len
 * @skb: Pointer to network buffer
 * @addr: Pointer to store header's addr
 * @m_len: network buffer length
 *
 * Return: none
 */
static inline void
__cdf_nbuf_peek_header(struct sk_buff *skb, uint8_t **addr, uint32_t *len)
{
	*addr = skb->data;
	*len = skb->len;
}

/******************Custom queue*************/

/**
 * __cdf_nbuf_queue_init() - initiallize the queue head
 * @qhead: Queue head
 *
 * Return: CDF status
 */
static inline CDF_STATUS __cdf_nbuf_queue_init(__cdf_nbuf_queue_t *qhead)
{
	memset(qhead, 0, sizeof(struct __cdf_nbuf_qhead));
	return CDF_STATUS_SUCCESS;
}

/**
 * __cdf_nbuf_queue_add() - add an skb in the tail of the queue
 * @qhead: Queue head
 * @skb: Pointer to network buffer
 *
 * This is a lockless version, driver must acquire locks if it
 * needs to synchronize
 *
 * Return: none
 */
static inline void
__cdf_nbuf_queue_add(__cdf_nbuf_queue_t *qhead, struct sk_buff *skb)
{
	skb->next = NULL;       /*Nullify the next ptr */

	if (!qhead->head)
		qhead->head = skb;
	else
		qhead->tail->next = skb;

	qhead->tail = skb;
	qhead->qlen++;
}

/**
 * __cdf_nbuf_queue_insert_head() - add an skb at  the head  of the queue
 * @qhead: Queue head
 * @skb: Pointer to network buffer
 *
 * This is a lockless version, driver must acquire locks if it needs to
 * synchronize
 *
 * Return: none
 */
static inline void
__cdf_nbuf_queue_insert_head(__cdf_nbuf_queue_t *qhead, __cdf_nbuf_t skb)
{
	if (!qhead->head) {
		/*Empty queue Tail pointer Must be updated */
		qhead->tail = skb;
	}
	skb->next = qhead->head;
	qhead->head = skb;
	qhead->qlen++;
}

/**
 * __cdf_nbuf_queue_remove() - remove a skb from the head of the queue
 * @qhead: Queue head
 *
 * This is a lockless version. Driver should take care of the locks
 *
 * Return: skb or NULL
 */
static inline
struct sk_buff *__cdf_nbuf_queue_remove(__cdf_nbuf_queue_t *qhead)
{
	__cdf_nbuf_t tmp = NULL;

	if (qhead->head) {
		qhead->qlen--;
		tmp = qhead->head;
		if (qhead->head == qhead->tail) {
			qhead->head = NULL;
			qhead->tail = NULL;
		} else {
			qhead->head = tmp->next;
		}
		tmp->next = NULL;
	}
	return tmp;
}

/**
 * __cdf_nbuf_queue_len() - return the queue length
 * @qhead: Queue head
 *
 * Return: Queue length
 */
static inline uint32_t __cdf_nbuf_queue_len(__cdf_nbuf_queue_t *qhead)
{
	return qhead->qlen;
}

/**
 * __cdf_nbuf_queue_next() - return the next skb from packet chain
 * @skb: Pointer to network buffer
 *
 * This API returns the next skb from packet chain, remember the skb is
 * still in the queue
 *
 * Return: NULL if no packets are there
 */
static inline struct sk_buff *__cdf_nbuf_queue_next(struct sk_buff *skb)
{
	return skb->next;
}

/**
 * __cdf_nbuf_is_queue_empty() - check if the queue is empty or not
 * @qhead: Queue head
 *
 * Return: true if length is 0 else false
 */
static inline bool __cdf_nbuf_is_queue_empty(__cdf_nbuf_queue_t *qhead)
{
	return qhead->qlen == 0;
}

/*
 * Use sk_buff_head as the implementation of cdf_nbuf_queue_t.
 * Because the queue head will most likely put in some structure,
 * we don't use pointer type as the definition.
 */

/*
 * prototypes. Implemented in cdf_nbuf.c
 */
cdf_nbuf_tx_cksum_t __cdf_nbuf_get_tx_cksum(struct sk_buff *skb);
CDF_STATUS __cdf_nbuf_set_rx_cksum(struct sk_buff *skb,
				   cdf_nbuf_rx_cksum_t *cksum);
uint8_t __cdf_nbuf_get_tid(struct sk_buff *skb);
void __cdf_nbuf_set_tid(struct sk_buff *skb, uint8_t tid);
uint8_t __cdf_nbuf_get_exemption_type(struct sk_buff *skb);

/*
 * cdf_nbuf_pool_delete() implementation - do nothing in linux
 */
#define __cdf_nbuf_pool_delete(osdev)

/**
 * __cdf_nbuf_clone() - clone the nbuf (copy is readonly)
 * @skb: Pointer to network buffer
 *
 * if GFP_ATOMIC is overkill then we can check whether its
 * called from interrupt context and then do it or else in
 * normal case use GFP_KERNEL
 *
 * example     use "in_irq() || irqs_disabled()"
 *
 * Return: cloned skb
 */
static inline struct sk_buff *__cdf_nbuf_clone(struct sk_buff *skb)
{
	return skb_clone(skb, GFP_ATOMIC);
}

/**
 * __cdf_nbuf_copy() - returns a private copy of the skb
 * @skb: Pointer to network buffer
 *
 * This API returns a private copy of the skb, the skb returned is completely
 *  modifiable by callers
 *
 * Return: skb or NULL
 */
static inline struct sk_buff *__cdf_nbuf_copy(struct sk_buff *skb)
{
	return skb_copy(skb, GFP_ATOMIC);
}

#define __cdf_nbuf_reserve      skb_reserve

/***********************XXX: misc api's************************/

/**
 * __cdf_nbuf_head() - return the pointer the skb's head pointer
 * @skb: Pointer to network buffer
 *
 * Return: Pointer to head buffer
 */
static inline uint8_t *__cdf_nbuf_head(struct sk_buff *skb)
{
	return skb->head;
}

/**
 * __cdf_nbuf_data() - return the pointer to data header in the skb
 * @skb: Pointer to network buffer
 *
 * Return: Pointer to skb data
 */
static inline uint8_t *__cdf_nbuf_data(struct sk_buff *skb)
{
	return skb->data;
}

/**
 * __cdf_nbuf_get_protocol() - return the protocol value of the skb
 * @skb: Pointer to network buffer
 *
 * Return: skb protocol
 */
static inline uint16_t __cdf_nbuf_get_protocol(struct sk_buff *skb)
{
	return skb->protocol;
}

/**
 * __cdf_nbuf_get_ip_summed() - return the ip checksum value of the skb
 * @skb: Pointer to network buffer
 *
 * Return: skb ip_summed
 */
static inline uint8_t __cdf_nbuf_get_ip_summed(struct sk_buff *skb)
{
	return skb->ip_summed;
}

/**
 * __cdf_nbuf_set_ip_summed() - sets the ip_summed value of the skb
 * @skb: Pointer to network buffer
 * @ip_summed: ip checksum
 *
 * Return: none
 */
static inline void __cdf_nbuf_set_ip_summed(struct sk_buff *skb, uint8_t ip_summed)
{
	skb->ip_summed = ip_summed;
}

/**
 * __cdf_nbuf_get_priority() - return the priority value of the skb
 * @skb: Pointer to network buffer
 *
 * Return: skb priority
 */
static inline uint32_t __cdf_nbuf_get_priority(struct sk_buff *skb)
{
	return skb->priority;
}

/**
 * __cdf_nbuf_set_priority() - sets the priority value of the skb
 * @skb: Pointer to network buffer
 * @p: priority
 *
 * Return: none
 */
static inline void __cdf_nbuf_set_priority(struct sk_buff *skb, uint32_t p)
{
	skb->priority = p;
}

/**
 * __cdf_nbuf_set_next() - sets the next skb pointer of the current skb
 * @skb: Current skb
 * @next_skb: Next skb
 *
 * Return: void
 */
static inline void
__cdf_nbuf_set_next(struct sk_buff *skb, struct sk_buff *skb_next)
{
	skb->next = skb_next;
}

/**
 * __cdf_nbuf_next() - return the next skb pointer of the current skb
 * @skb: Current skb
 *
 * Return: the next skb pointed to by the current skb
 */
static inline struct sk_buff *__cdf_nbuf_next(struct sk_buff *skb)
{
	return skb->next;
}

/**
 * __cdf_nbuf_set_next_ext() - sets the next skb pointer of the current skb
 * @skb: Current skb
 * @next_skb: Next skb
 *
 * This fn is used to link up extensions to the head skb. Does not handle
 * linking to the head
 *
 * Return: none
 */
static inline void
__cdf_nbuf_set_next_ext(struct sk_buff *skb, struct sk_buff *skb_next)
{
	skb->next = skb_next;
}

/**
 * __cdf_nbuf_next_ext() - return the next skb pointer of the current skb
 * @skb: Current skb
 *
 * Return: the next skb pointed to by the current skb
 */
static inline struct sk_buff *__cdf_nbuf_next_ext(struct sk_buff *skb)
{
	return skb->next;
}

/**
 * __cdf_nbuf_append_ext_list() - link list of packet extensions to the head
 * @skb_head: head_buf nbuf holding head segment (single)
 * @ext_list: nbuf list holding linked extensions to the head
 * @ext_len: Total length of all buffers in the extension list
 *
 * This function is used to link up a list of packet extensions (seg1, 2,*  ...)
 * to the nbuf holding the head segment (seg0)
 *
 * Return: none
 */
static inline void
__cdf_nbuf_append_ext_list(struct sk_buff *skb_head,
			   struct sk_buff *ext_list, size_t ext_len)
{
	skb_shinfo(skb_head)->frag_list = ext_list;
	skb_head->data_len = ext_len;
	skb_head->len += skb_head->data_len;
}

/**
 * __cdf_nbuf_tx_free() - free skb list
 * @skb: Pointer to network buffer
 * @tx_err: TX error
 *
 * Return: none
 */
static inline void __cdf_nbuf_tx_free(struct sk_buff *bufs, int tx_err)
{
	while (bufs) {
		struct sk_buff *next = __cdf_nbuf_next(bufs);
		__cdf_nbuf_free(bufs);
		bufs = next;
	}
}

/**
 * __cdf_nbuf_get_age() - return the checksum value of the skb
 * @skb: Pointer to network buffer
 *
 * Return: checksum value
 */
static inline uint32_t __cdf_nbuf_get_age(struct sk_buff *skb)
{
	return skb->csum;
}

/**
 * __cdf_nbuf_set_age() - sets the checksum value of the skb
 * @skb: Pointer to network buffer
 * @v: Value
 *
 * Return: none
 */
static inline void __cdf_nbuf_set_age(struct sk_buff *skb, uint32_t v)
{
	skb->csum = v;
}

/**
 * __cdf_nbuf_adj_age() - adjusts the checksum/age value of the skb
 * @skb: Pointer to network buffer
 * @adj: Adjustment value
 *
 * Return: none
 */
static inline void __cdf_nbuf_adj_age(struct sk_buff *skb, uint32_t adj)
{
	skb->csum -= adj;
}

/**
 * __cdf_nbuf_copy_bits() - return the length of the copy bits for skb
 * @skb: Pointer to network buffer
 * @offset: Offset value
 * @len: Length
 * @to: Destination pointer
 *
 * Return: length of the copy bits for skb
 */
static inline int32_t
__cdf_nbuf_copy_bits(struct sk_buff *skb, int32_t offset, int32_t len, void *to)
{
	return skb_copy_bits(skb, offset, to, len);
}

/**
 * __cdf_nbuf_set_pktlen() - sets the length of the skb and adjust the tail
 * @skb: Pointer to network buffer
 * @len:  Packet length
 *
 * Return: none
 */
static inline void __cdf_nbuf_set_pktlen(struct sk_buff *skb, uint32_t len)
{
	if (skb->len > len) {
		skb_trim(skb, len);
	} else {
		if (skb_tailroom(skb) < len - skb->len) {
			if (unlikely(pskb_expand_head(skb, 0,
				len - skb->len - skb_tailroom(skb),
				GFP_ATOMIC))) {
				dev_kfree_skb_any(skb);
				cdf_assert(0);
			}
		}
		skb_put(skb, (len - skb->len));
	}
}

/**
 * __cdf_nbuf_set_protocol() - sets the protocol value of the skb
 * @skb: Pointer to network buffer
 * @protocol: Protocol type
 *
 * Return: none
 */
static inline void
__cdf_nbuf_set_protocol(struct sk_buff *skb, uint16_t protocol)
{
	skb->protocol = protocol;
}

#define __cdf_nbuf_set_tx_htt2_frm(skb, candi) \
	NBUF_SET_TX_HTT2_FRM(skb, candi)
#define __cdf_nbuf_get_tx_htt2_frm(skb)	\
	NBUF_GET_TX_HTT2_FRM(skb)

#if defined(FEATURE_TSO)
uint32_t __cdf_nbuf_get_tso_info(cdf_device_t osdev, struct sk_buff *skb,
	struct cdf_tso_info_t *tso_info);

uint32_t __cdf_nbuf_get_tso_num_seg(struct sk_buff *skb);

static inline bool __cdf_nbuf_is_tso(struct sk_buff *skb)
{
	return skb_is_gso(skb);
}

static inline void __cdf_nbuf_inc_users(struct sk_buff *skb)
{
	atomic_inc(&skb->users);
	return;
}
#endif /* TSO */

/**
 * __cdf_nbuf_tx_info_get() - Modify pkt_type, set pkt_subtype,
 *			      and get hw_classify by peeking
 *			      into packet
 * @nbuf:		Network buffer (skb on Linux)
 * @pkt_type:		Pkt type (from enum htt_pkt_type)
 * @pkt_subtype:	Bit 4 of this field in HTT descriptor
 *			needs to be set in case of CE classification support
 *			Is set by this macro.
 * @hw_classify:	This is a flag which is set to indicate
 *			CE classification is enabled.
 *			Do not set this bit for VLAN packets
 *			OR for mcast / bcast frames.
 *
 * This macro parses the payload to figure out relevant Tx meta-data e.g.
 * whether to enable tx_classify bit in CE.
 *
 * Overrides pkt_type only if required for 802.3 frames (original ethernet)
 * If protocol is less than ETH_P_802_3_MIN (0x600), then
 * it is the length and a 802.3 frame else it is Ethernet Type II
 * (RFC 894).
 * Bit 4 in pkt_subtype is the tx_classify bit
 *
 * Return:	void
 */
#define __cdf_nbuf_tx_info_get(skb, pkt_type,			\
				pkt_subtype, hw_classify)	\
do {								\
	struct ethhdr *eh = (struct ethhdr *)skb->data;		\
	uint16_t ether_type = ntohs(eh->h_proto);		\
	bool is_mc_bc;						\
								\
	is_mc_bc = is_broadcast_ether_addr((uint8_t *)eh) ||	\
		   is_multicast_ether_addr((uint8_t *)eh);	\
								\
	if (likely((ether_type != ETH_P_8021Q) && !is_mc_bc)) {	\
		hw_classify = 1;				\
		pkt_subtype = 0x01 <<				\
			HTT_TX_CLASSIFY_BIT_S;			\
	}							\
								\
	if (unlikely(ether_type < ETH_P_802_3_MIN))		\
		pkt_type = htt_pkt_type_ethernet;		\
								\
} while (0)
#endif /*_I_CDF_NET_BUF_H */
