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
#include <cdf_types.h>
#include <cdf_status.h>

/*
 * Use socket buffer as the underlying implentation as skbuf .
 * Linux use sk_buff to represent both packet and data,
 * so we use sk_buffer to represent both skbuf .
 */
typedef struct sk_buff *__cdf_nbuf_t;

/* NBUFCB_TX_MAX_OS_FRAGS
 * max tx fragments provided by the OS
 */
#define NBUF_CB_TX_MAX_OS_FRAGS 1

/* NBUF_CB_TX_MAX_EXTRA_FRAGS -
 * max tx fragments added by the driver
 * The driver will always add one tx fragment (the tx descriptor)
 */
#define NBUF_CB_TX_MAX_EXTRA_FRAGS 2

/*
 * Make sure that cdf_dma_addr_t in the cb block is always 64 bit aligned
 */
typedef union {
	uint64_t       u64;
	cdf_dma_addr_t dma_addr;
} cdf_paddr_t;

/**
 * struct cdf_nbuf_cb - network buffer control block contents (skb->cb)
 *                    - data passed between layers of the driver.
 *
 * Notes:
 *   1. Hard limited to 48 bytes. Please count your bytes
 *   2. The size of this structure has to be easily calculatable and
 *      consistently so: do not use any conditional compile flags
 *   3. Split into a common part followed by a tx/rx overlay
 *   4. There is only one extra frag, which represents the HTC/HTT header
 *
 * @common.paddr   : physical addressed retrived by dma_map of nbuf->data
 * @rx.lro_flags   : hardware assisted flags:
 *   @rx.lro_eligible    : flag to indicate whether the MSDU is LRO eligible
 *   @rx.tcp_proto       : L4 protocol is TCP
 *   @rx.tcp_pure_ack    : A TCP ACK packet with no payload
 *   @rx.ipv6_proto      : L3 protocol is IPV6
 *   @rx.ip_offset       : offset to IP header
 *   @rx.tcp_offset      : offset to TCP header
 *   @rx.tcp_udp_chksum  : L4 payload checksum
 *   @rx.tcp_seq_num     : TCP sequence number
 *   @rx.tcp_ack_num     : TCP ACK number
 *   @rx.flow_id_toeplitz: 32-bit 5-tuple Toeplitz hash
 * @tx.extra_frag  : represent HTC/HTT header
 * @tx.efrag.vaddr       : virtual address of ~
 * @tx.efrag.paddr       : physical/DMA address of ~
 * @tx.efrag.len         : length of efrag pointed by the above pointers
 * @tx.efrag.num         : number of extra frags ( 0 or 1)
 * @tx.efrag.flags.nbuf  : flag, nbuf payload to be swapped (wordstream)
 * @tx.efrag.flags.efrag : flag, efrag payload to be swapped (wordstream)
 * @tx.efrag.flags.chfrag_start: used by WIN
 * @tx.efrags.flags.chfrag_end:   used by WIN
 * @tx.data_attr   : value that is programmed into CE descr, includes:
 *                 + (1) CE classification enablement bit
 *                 + (2) packet type (802.3 or Ethernet type II)
 *                 + (3) packet offset (usually length of HTC/HTT descr)
 * @tx.trace       : combined structure for DP and protocol trace
 * @tx.trace.packet_state: {NBUF_TX_PKT_[(HDD)|(TXRX_ENQUEUE)|(TXRX_DEQUEUE)|
 *                       +               (TXRX)|(HTT)|(HTC)|(HIF)|(CE)|(FREE)]
 * @tx.trace.packet_track: {NBUF_TX_PKT_[(DATA)|(MGMT)]_TRACK}
 * @tx.trace.proto_type  : bitmap of NBUF_PKT_TRAC_TYPE[(EAPOL)|(DHCP)|
 *                       +                              (MGMT_ACTION)] - 4 bits
 * @tx.trace.dp_trace    : flag (Datapath trace)
 * @tx.trace.htt2_frm    : flag (high-latency path only)
 * @tx.trace.vdev_id     : vdev (for protocol trace)
 * @tx.ipa.owned   : packet owned by IPA
 * @tx.ipa.priv    : private data, used by IPA
 */
struct cdf_nbuf_cb {
	/* common */
	cdf_paddr_t paddr; /* of skb->data */
	/* valid only in one direction */
	union {
		/* Note: MAX: 40 bytes */
		struct {
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
		} rx; /* 20 bytes */

		/* Note: MAX: 40 bytes */
		struct {
			struct {
				unsigned char *vaddr;
				cdf_paddr_t paddr;
				uint16_t len;
				uint8_t num; /* 0: cmn.addr; 1: tx.efrag */
				union {
					struct {
						uint8_t flag_efrag:1,
							flag_nbuf:1,
							/* following for WIN */
							flag_chfrag_start:1,
							flag_chfrag_end:1,
							reserved:4;
					} bits;
					uint8_t u8;
				} flags;
			} extra_frag; /* 20 bytes */
			uint32_t data_attr; /* 4 bytes */
			union {
				struct {
					uint8_t packet_state;
					uint8_t packet_track:4,
						proto_type:4;
					uint8_t dp_trace:1,
						htt2_frm:1,
						rsrvd:6;
					uint8_t vdev_id;
				} hl;
				struct {
					uint8_t packet_state;
					uint8_t packet_track:4,
						proto_type:4;
					uint8_t dp_trace:1,
						rsrvd:7;
					uint8_t vdev_id;
				} ll; /* low latency */
			} trace; /* 4 bytes */
			struct {
				uint32_t owned:1,
					priv:31;
			} ipa; /* 4 */
		} tx; /* 32 bytes */
	} u;
}; /* struct cdf_nbuf_cb: MAX 48 bytes */

/**
 *  access macros to cdf_nbuf_cb
 *  Note: These macros can be used as L-values as well as R-values.
 *        When used as R-values, they effectively function as "get" macros
 *        When used as L_values, they effectively function as "set" macros
 */
#define NBUF_CB_PADDR(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->paddr.dma_addr)
#define NBUF_CB_RX_LRO_ELIGIBLE(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.rx.lro_eligible)
#define NBUF_CB_RX_TCP_PROTO(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_proto)
#define NBUF_CB_RX_TCP_PURE_ACK(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_pure_ack)
#define NBUF_CB_RX_IPV6_PROTO(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.rx.ipv6_proto)
#define NBUF_CB_RX_IP_OFFSET(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.rx.ip_offset)
#define NBUF_CB_RX_TCP_OFFSET(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_offset)
#define NBUF_CB_RX_TCP_CHKSUM(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_udp_chksum)
#define NBUF_CB_RX_TCP_OFFSET(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_offset)
#define NBUF_CB_RX_TCP_WIN(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_win)
#define NBUF_CB_RX_TCP_SEQ_NUM(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_seq_num)
#define NBUF_CB_RX_TCP_ACK_NUM(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_ack_num)
#define NBUF_CB_RX_FLOW_ID_TOEPLITZ(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.rx.flow_id_toeplitz)

#define NBUF_CB_TX_EXTRA_FRAG_VADDR(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.extra_frag.vaddr)
#define NBUF_CB_TX_EXTRA_FRAG_PADDR(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.extra_frag.paddr.dma_addr)
#define NBUF_CB_TX_EXTRA_FRAG_LEN(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.extra_frag.len)
#define NBUF_CB_TX_NUM_EXTRA_FRAGS(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.extra_frag.num)
#define NBUF_CB_TX_EXTRA_FRAG_WORDSTR_FLAGS(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.extra_frag.flags.u8)
#define NBUF_CB_TX_EXTRA_FRAG_WORDSTR_EFRAG(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.extra_frag.flags.bits.flag_efrag)
#define NBUF_CB_TX_EXTRA_FRAG_WORDSTR_NBUF(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.extra_frag.flags.bits.flag_nbuf)
#define NBUF_CB_TX_DATA_ATTR(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.data_attr)
#define NBUF_CB_TX_PACKET_STATE(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.trace.ll.packet_state)
#define NBUF_CB_TX_PACKET_TRACK(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.trace.ll.packet_track)
#define NBUF_CB_TX_PROTO_TYPE(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.trace.ll.proto_type)
#define NBUF_UPDATE_TX_PKT_COUNT(skb, PACKET_STATE) \
	cdf_nbuf_set_state(skb, PACKET_STATE)
#define NBUF_CB_TX_DP_TRACE(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.trace.ll.dp_trace)
#define NBUF_CB_TX_HL_HTT2_FRM(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.trace.hl.htt2_frm)
#define NBUF_CB_TX_VDEV_ID(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.trace.ll.vdev_id)
#define NBUF_CB_TX_IPA_OWNED(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.ipa.owned)
#define NBUF_CB_TX_IPA_PRIV(skb) \
	(((struct cdf_nbuf_cb *)((skb)->cb))->u.tx.ipa.priv)

#define __cdf_nbuf_get_num_frags(skb) \
	(NBUF_CB_TX_NUM_EXTRA_FRAGS(skb) + 1)

#if defined(FEATURE_TSO)
#define __cdf_nbuf_reset_num_frags(skb) \
	(NBUF_CB_TX_NUM_EXTRA_FRAGS(skb) = 0)
#endif
/**
 *   end of nbuf->cb access macros
 */

typedef void (*cdf_nbuf_trace_update_t)(char *);

#define __cdf_nbuf_mapped_paddr_get(skb) \
	NBUF_CB_PADDR(skb)

#define __cdf_nbuf_mapped_paddr_set(skb, paddr)	\
	(NBUF_CB_PADDR(skb) = (paddr))

#define __cdf_nbuf_frag_push_head(					\
	skb, frag_len, frag_vaddr, frag_paddr)				\
	do {								\
		NBUF_CB_TX_NUM_EXTRA_FRAGS(skb) = 1;			\
		NBUF_CB_TX_EXTRA_FRAG_VADDR(skb) = frag_vaddr;		\
		NBUF_CB_TX_EXTRA_FRAG_PADDR(skb) = frag_paddr;		\
		NBUF_CB_TX_EXTRA_FRAG_LEN(skb) = frag_len;		\
	} while (0)

#define __cdf_nbuf_get_frag_vaddr(skb, frag_num)		\
	((frag_num < NBUF_CB_TX_NUM_EXTRA_FRAGS(skb)) ?		\
	 NBUF_CB_TX_EXTRA_FRAG_VADDR(skb) : ((skb)->data))

#define __cdf_nbuf_get_frag_paddr(skb, frag_num)			\
	((frag_num < NBUF_CB_TX_NUM_EXTRA_FRAGS(skb)) ?			\
	 NBUF_CB_TX_EXTRA_FRAG_PADDR(skb) :				\
	 /* assume that the OS only provides a single fragment */	\
	 NBUF_CB_PADDR(skb))

#define __cdf_nbuf_get_frag_len(skb, frag_num)			\
	((frag_num < NBUF_CB_TX_NUM_EXTRA_FRAGS(skb)) ?		\
	 NBUF_CB_TX_EXTRA_FRAG_LEN(skb) : (skb)->len)

#define __cdf_nbuf_get_frag_is_wordstream(skb, frag)			\
	((frag_num < NBUF_CB_TX_NUM_EXTRA_FRAGS(skb))			\
	 ? (NBUF_CB_TX_EXTRA_FRAG_WORDSTR_EFRAG(skb))		\
	 : (NBUF_CB_TX_EXTRA_FRAG_WORDSTR_NBUF(skb)))

#define __cdf_nbuf_set_frag_is_wordstream(skb, frag_num, is_wstrm)	\
	do {								\
		if (frag_num >= NBUF_CB_TX_NUM_EXTRA_FRAGS(skb))	\
			frag_num = NBUF_CB_TX_MAX_EXTRA_FRAGS;		\
		if (frag_num)						\
			NBUF_CB_TX_EXTRA_FRAG_WORDSTR_EFRAG(skb) = is_wstrm; \
		else							\
			NBUF_CB_TX_EXTRA_FRAG_WORDSTR_NBUF(skb) = is_wstrm; \
	} while (0)

#define __cdf_nbuf_trace_set_proto_type(skb, proto_type) \
	(NBUF_CB_TX_PROTO_TYPE(skb) = (proto_type))
#define __cdf_nbuf_trace_get_proto_type(skb) \
	NBUF_CB_TX_PROTO_TYPE(skb)

#define __cdf_nbuf_data_attr_get(skb)		\
	NBUF_CB_TX_DATA_ATTR(skb)
#define __cdf_nbuf_data_attr_set(skb, data_attr)	\
	(NBUF_CB_TX_DATA_ATTR(skb) = (data_attr))

#define __cdf_nbuf_ipa_owned_get(skb) \
	NBUF_CB_TX_IPA_OWNED(skb)

#define __cdf_nbuf_ipa_owned_set(skb) \
	(NBUF_CB_TX_IPA_OWNED(skb) = 1)

#define __cdf_nbuf_ipa_priv_get(skb)	\
	NBUF_CB_TX_IPA_PRIV(skb)

#define __cdf_nbuf_ipa_priv_set(skb, priv)	\
	(NBUF_CB_TX_IPA_PRIV(skb) = (priv))

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

	i = NBUF_CB_TX_NUM_EXTRA_FRAGS(skb);
	if (i > 0)
		extra_frag_len = NBUF_CB_TX_EXTRA_FRAG_LEN(skb);

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

/*
 * nbuf manipulation routines
 */

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
	if (NBUF_CB_PADDR(skb))
		NBUF_CB_PADDR(skb) -= size;

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
	if (NBUF_CB_PADDR(skb))
		NBUF_CB_PADDR(skb) += size;

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
	(NBUF_CB_TX_HL_HTT2_FRM(skb) = (candi))
#define __cdf_nbuf_get_tx_htt2_frm(skb)	\
	NBUF_CB_TX_HL_HTT2_FRM(skb)

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

/**
 * nbuf private buffer routines
 */

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

/******************Functions *************/

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
 * Use sk_buff_head as the implementation of cdf_nbuf_queue_t.
 * Because the queue head will most likely put in some structure,
 * we don't use pointer type as the definition.
 */
#endif /*_I_CDF_NET_BUF_H */
