/*
 * Copyright (c) 2014-2020 The Linux Foundation. All rights reserved.
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

/**
 * DOC: i_qdf_nbuf.h
 * This file provides OS dependent nbuf API's.
 */

#ifndef _I_QDF_NBUF_H
#define _I_QDF_NBUF_H

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/dma-mapping.h>
#include <linux/version.h>
#include <asm/cacheflush.h>
#include <qdf_types.h>
#include <qdf_net_types.h>
#include <qdf_status.h>
#include <qdf_util.h>
#include <qdf_mem.h>
#include <linux/tcp.h>
#include <qdf_util.h>
#include <qdf_nbuf_frag.h>

/*
 * Use socket buffer as the underlying implementation as skbuf .
 * Linux use sk_buff to represent both packet and data,
 * so we use sk_buffer to represent both skbuf .
 */
typedef struct sk_buff *__qdf_nbuf_t;

/**
 * typedef __qdf_nbuf_queue_head_t - abstraction for sk_buff_head linux struct
 *
 * This is used for skb queue management via linux skb buff head APIs
 */
typedef struct sk_buff_head __qdf_nbuf_queue_head_t;

#define QDF_NBUF_CB_TX_MAX_OS_FRAGS 1

#define QDF_SHINFO_SIZE    SKB_DATA_ALIGN(sizeof(struct skb_shared_info))

/* QDF_NBUF_CB_TX_MAX_EXTRA_FRAGS -
 * max tx fragments added by the driver
 * The driver will always add one tx fragment (the tx descriptor)
 */
#define QDF_NBUF_CB_TX_MAX_EXTRA_FRAGS 2
#define QDF_NBUF_CB_PACKET_TYPE_EAPOL  1
#define QDF_NBUF_CB_PACKET_TYPE_ARP    2
#define QDF_NBUF_CB_PACKET_TYPE_WAPI   3
#define QDF_NBUF_CB_PACKET_TYPE_DHCP   4
#define QDF_NBUF_CB_PACKET_TYPE_ICMP   5
#define QDF_NBUF_CB_PACKET_TYPE_ICMPv6 6


/* mark the first packet after wow wakeup */
#define QDF_MARK_FIRST_WAKEUP_PACKET   0x80000000

/*
 * Make sure that qdf_dma_addr_t in the cb block is always 64 bit aligned
 */
typedef union {
	uint64_t       u64;
	qdf_dma_addr_t dma_addr;
} qdf_paddr_t;

/**
 * struct qdf_nbuf_cb - network buffer control block contents (skb->cb)
 *                    - data passed between layers of the driver.
 *
 * Notes:
 *   1. Hard limited to 48 bytes. Please count your bytes
 *   2. The size of this structure has to be easily calculatable and
 *      consistently so: do not use any conditional compile flags
 *   3. Split into a common part followed by a tx/rx overlay
 *   4. There is only one extra frag, which represents the HTC/HTT header
 *   5. "ext_cb_pt" must be the first member in both TX and RX unions
 *      for the priv_cb_w since it must be at same offset for both
 *      TX and RX union
 *   6. "ipa.owned" bit must be first member in both TX and RX unions
 *      for the priv_cb_m since it must be at same offset for both
 *      TX and RX union.
 *
 * @paddr   : physical addressed retrieved by dma_map of nbuf->data
 *
 * @rx.dev.priv_cb_w.ext_cb_ptr: extended cb pointer
 * @rx.dev.priv_cb_w.fctx: ctx to handle special pkts defined by ftype
 * @rx.dev.priv_cb_w.msdu_len: length of RX packet
 * @rx.dev.priv_cb_w.peer_id: peer_id for RX packet
 * @rx.dev.priv_cb_w.protocol_tag: protocol tag set by app for rcvd packet type
 * @rx.dev.priv_cb_w.flow_tag: flow tag set by application for 5 tuples rcvd
 *
 * @rx.dev.priv_cb_m.peer_cached_buf_frm: peer cached buffer
 * @rx.dev.priv_cb_m.flush_ind: flush indication
 * @rx.dev.priv_cb_m.packet_buf_pool:  packet buff bool
 * @rx.dev.priv_cb_m.l3_hdr_pad: L3 header padding offset
 * @rx.dev.priv_cb_m.exc_frm: exception frame
 * @rx.dev.priv_cb_m.ipa_smmu_map: do IPA smmu map
 * @rx.dev.priv_cb_m.reo_dest_ind: reo destination indication
 * @rx.dev.priv_cb_m.tcp_seq_num: TCP sequence number
 * @rx.dev.priv_cb_m.tcp_ack_num: TCP ACK number
 * @rx.dev.priv_cb_m.lro_ctx: LRO context
 * @rx.dev.priv_cb_m.dp.wifi3.msdu_len: length of RX packet
 * @rx.dev.priv_cb_m.dp.wifi3.peer_id:  peer_id for RX packet
 * @rx.dev.priv_cb_m.dp.wifi2.map_index:
 * @rx.dev.priv_cb_m.ipa_owned: packet owned by IPA
 *
 * @rx.lro_eligible: flag to indicate whether the MSDU is LRO eligible
 * @rx.tcp_proto: L4 protocol is TCP
 * @rx.tcp_pure_ack: A TCP ACK packet with no payload
 * @rx.ipv6_proto: L3 protocol is IPV6
 * @rx.ip_offset: offset to IP header
 * @rx.tcp_offset: offset to TCP header
 * @rx_ctx_id: Rx context id
 * @num_elements_in_list: number of elements in the nbuf list
 *
 * @rx.tcp_udp_chksum: L4 payload checksum
 * @rx.tcp_wim: TCP window size
 *
 * @rx.flow_id: 32bit flow id
 *
 * @rx.flag_chfrag_start: first MSDU in an AMSDU
 * @rx.flag_chfrag_cont: middle or part of MSDU in an AMSDU
 * @rx.flag_chfrag_end: last MSDU in an AMSDU
 * @rx.flag_retry: flag to indicate MSDU is retried
 * @rx.flag_da_mcbc: flag to indicate mulicast or broadcast packets
 * @rx.flag_da_valid: flag to indicate DA is valid for RX packet
 * @rx.flag_sa_valid: flag to indicate SA is valid for RX packet
 * @rx.flag_is_frag: flag to indicate skb has frag list
 * @rx.rsrvd: reserved
 *
 * @rx.trace: combined structure for DP and protocol trace
 * @rx.trace.packet_stat: {NBUF_TX_PKT_[(HDD)|(TXRX_ENQUEUE)|(TXRX_DEQUEUE)|
 *                       +          (TXRX)|(HTT)|(HTC)|(HIF)|(CE)|(FREE)]
 * @rx.trace.dp_trace: flag (Datapath trace)
 * @rx.trace.packet_track: RX_DATA packet
 * @rx.trace.rsrvd: enable packet logging
 *
 * @rx.vdev_id: vdev_id for RX pkt
 * @rx.is_raw_frame: RAW frame
 * @rx.fcs_err: FCS error
 * @rx.tid_val: tid value
 * @rx.reserved: reserved
 * @rx.ftype: mcast2ucast, TSO, SG, MESH
 *
 * @tx.dev.priv_cb_w.fctx: ctx to handle special pkts defined by ftype
 * @tx.dev.priv_cb_w.ext_cb_ptr: extended cb pointer
 *
 * @tx.dev.priv_cb_w.data_attr: value that is programmed in CE descr, includes
 *                 + (1) CE classification enablement bit
 *                 + (2) packet type (802.3 or Ethernet type II)
 *                 + (3) packet offset (usually length of HTC/HTT descr)
 * @tx.dev.priv_cb_m.ipa.owned: packet owned by IPA
 * @tx.dev.priv_cb_m.ipa.priv: private data, used by IPA
 * @tx.dev.priv_cb_m.desc_id: tx desc id, used to sync between host and fw
 * @tx.dev.priv_cb_m.mgmt_desc_id: mgmt descriptor for tx completion cb
 * @tx.dev.priv_cb_m.dma_option.bi_map: flag to do bi-direction dma map
 * @tx.dev.priv_cb_m.dma_option.reserved: reserved bits for future use
 * @tx.dev.priv_cb_m.reserved: reserved
 *
 * @tx.ftype: mcast2ucast, TSO, SG, MESH
 * @tx.vdev_id: vdev (for protocol trace)
 * @tx.len: length of efrag pointed by the above pointers
 *
 * @tx.flags.bits.flag_efrag: flag, efrag payload to be swapped (wordstream)
 * @tx.flags.bits.num: number of extra frags ( 0 or 1)
 * @tx.flags.bits.nbuf: flag, nbuf payload to be swapped (wordstream)
 * @tx.flags.bits.flag_chfrag_start: first MSDU in an AMSDU
 * @tx.flags.bits.flag_chfrag_cont: middle or part of MSDU in an AMSDU
 * @tx.flags.bits.flag_chfrag_end: last MSDU in an AMSDU
 * @tx.flags.bits.flag_ext_header: extended flags
 * @tx.flags.bits.reserved: reserved
 * @tx.trace: combined structure for DP and protocol trace
 * @tx.trace.packet_stat: {NBUF_TX_PKT_[(HDD)|(TXRX_ENQUEUE)|(TXRX_DEQUEUE)|
 *                       +          (TXRX)|(HTT)|(HTC)|(HIF)|(CE)|(FREE)]
 * @tx.trace.is_packet_priv:
 * @tx.trace.packet_track: {NBUF_TX_PKT_[(DATA)|(MGMT)]_TRACK}
 * @tx.trace.proto_type: bitmap of NBUF_PKT_TRAC_TYPE[(EAPOL)|(DHCP)|
 *                          + (MGMT_ACTION)] - 4 bits
 * @tx.trace.dp_trace: flag (Datapath trace)
 * @tx.trace.is_bcast: flag (Broadcast packet)
 * @tx.trace.is_mcast: flag (Multicast packet)
 * @tx.trace.packet_type: flag (Packet type)
 * @tx.trace.htt2_frm: flag (high-latency path only)
 * @tx.trace.print: enable packet logging
 *
 * @tx.vaddr: virtual address of ~
 * @tx.paddr: physical/DMA address of ~
 */
struct qdf_nbuf_cb {
	/* common */
	qdf_paddr_t paddr; /* of skb->data */
	/* valid only in one direction */
	union {
		/* Note: MAX: 40 bytes */
		struct {
			union {
				struct {
					void *ext_cb_ptr;
					void *fctx;
					uint16_t msdu_len;
					uint16_t peer_id;
					uint16_t protocol_tag;
					uint16_t flow_tag;
				} priv_cb_w;
				struct {
					/* ipa_owned bit is common between rx
					 * control block and tx control block.
					 * Do not change location of this bit.
					 */
					uint32_t ipa_owned:1,
						 peer_cached_buf_frm:1,
						 flush_ind:1,
						 packet_buf_pool:1,
						 l3_hdr_pad:3,
						 /* exception frame flag */
						 exc_frm:1,
						 ipa_smmu_map:1,
						 reo_dest_ind:5,
						 reserved:2,
						 reserved1:16;
					uint32_t tcp_seq_num;
					uint32_t tcp_ack_num;
					union {
						struct {
							uint16_t msdu_len;
							uint16_t peer_id;
						} wifi3;
						struct {
							uint32_t map_index;
						} wifi2;
					} dp;
					unsigned char *lro_ctx;
				} priv_cb_m;
			} dev;
			uint32_t lro_eligible:1,
				tcp_proto:1,
				tcp_pure_ack:1,
				ipv6_proto:1,
				ip_offset:7,
				tcp_offset:7,
				rx_ctx_id:4,
				fcs_err:1,
				is_raw_frame:1,
				num_elements_in_list:8;
			uint32_t tcp_udp_chksum:16,
				 tcp_win:16;
			uint32_t flow_id;
			uint8_t flag_chfrag_start:1,
				flag_chfrag_cont:1,
				flag_chfrag_end:1,
				flag_retry:1,
				flag_da_mcbc:1,
				flag_da_valid:1,
				flag_sa_valid:1,
				flag_is_frag:1;
			union {
				uint8_t packet_state;
				uint8_t dp_trace:1,
					packet_track:4,
					rsrvd:3;
			} trace;
			uint16_t vdev_id:8,
				 tid_val:4,
				 ftype:4;
		} rx;

		/* Note: MAX: 40 bytes */
		struct {
			union {
				struct {
					void *ext_cb_ptr;
					void *fctx;
				} priv_cb_w;
				struct {
					/* ipa_owned bit is common between rx
					 * control block and tx control block.
					 * Do not change location of this bit.
					 */
					struct {
						uint32_t owned:1,
							priv:31;
					} ipa;
					uint32_t data_attr;
					uint16_t desc_id;
					uint16_t mgmt_desc_id;
					struct {
						uint8_t bi_map:1,
							reserved:7;
					} dma_option;
					uint8_t reserved[3];
				} priv_cb_m;
			} dev;
			uint8_t ftype;
			uint8_t vdev_id;
			uint16_t len;
			union {
				struct {
					uint8_t flag_efrag:1,
						flag_nbuf:1,
						num:1,
						flag_chfrag_start:1,
						flag_chfrag_cont:1,
						flag_chfrag_end:1,
						flag_ext_header:1,
						flag_notify_comp:1;
				} bits;
				uint8_t u8;
			} flags;
			struct {
				uint8_t packet_state:7,
					is_packet_priv:1;
				uint8_t packet_track:4,
					proto_type:4;
				uint8_t dp_trace:1,
					is_bcast:1,
					is_mcast:1,
					packet_type:3,
					/* used only for hl*/
					htt2_frm:1,
					print:1;
			} trace;
			unsigned char *vaddr;
			qdf_paddr_t paddr;
		} tx;
	} u;
}; /* struct qdf_nbuf_cb: MAX 48 bytes */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0))
QDF_COMPILE_TIME_ASSERT(qdf_nbuf_cb_size,
			(sizeof(struct qdf_nbuf_cb)) <=
			sizeof_field(struct sk_buff, cb));
#else
QDF_COMPILE_TIME_ASSERT(qdf_nbuf_cb_size,
			(sizeof(struct qdf_nbuf_cb)) <=
			FIELD_SIZEOF(struct sk_buff, cb));
#endif

/**
 *  access macros to qdf_nbuf_cb
 *  Note: These macros can be used as L-values as well as R-values.
 *        When used as R-values, they effectively function as "get" macros
 *        When used as L_values, they effectively function as "set" macros
 */

#define QDF_NBUF_CB_PADDR(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->paddr.dma_addr)

#define QDF_NBUF_CB_RX_LRO_ELIGIBLE(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.lro_eligible)
#define QDF_NBUF_CB_RX_TCP_PROTO(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_proto)
#define QDF_NBUF_CB_RX_TCP_PURE_ACK(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_pure_ack)
#define QDF_NBUF_CB_RX_IPV6_PROTO(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.ipv6_proto)
#define QDF_NBUF_CB_RX_IP_OFFSET(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.ip_offset)
#define QDF_NBUF_CB_RX_TCP_OFFSET(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_offset)
#define QDF_NBUF_CB_RX_CTX_ID(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.rx_ctx_id)
#define QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(skb) \
		(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.num_elements_in_list)

#define QDF_NBUF_CB_RX_TCP_CHKSUM(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_udp_chksum)
#define QDF_NBUF_CB_RX_TCP_WIN(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.tcp_win)

#define QDF_NBUF_CB_RX_FLOW_ID(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.flow_id)

#define QDF_NBUF_CB_RX_PACKET_STATE(skb)\
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.trace.packet_state)
#define QDF_NBUF_CB_RX_DP_TRACE(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.trace.dp_trace)

#define QDF_NBUF_CB_RX_FTYPE(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.ftype)

#define QDF_NBUF_CB_RX_VDEV_ID(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.rx.vdev_id)

#define QDF_NBUF_CB_RX_CHFRAG_START(skb) \
	(((struct qdf_nbuf_cb *) \
	((skb)->cb))->u.rx.flag_chfrag_start)
#define QDF_NBUF_CB_RX_CHFRAG_CONT(skb) \
	(((struct qdf_nbuf_cb *) \
	((skb)->cb))->u.rx.flag_chfrag_cont)
#define QDF_NBUF_CB_RX_CHFRAG_END(skb) \
		(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.rx.flag_chfrag_end)

#define QDF_NBUF_CB_RX_DA_MCBC(skb) \
	(((struct qdf_nbuf_cb *) \
	((skb)->cb))->u.rx.flag_da_mcbc)

#define QDF_NBUF_CB_RX_DA_VALID(skb) \
	(((struct qdf_nbuf_cb *) \
	((skb)->cb))->u.rx.flag_da_valid)

#define QDF_NBUF_CB_RX_SA_VALID(skb) \
	(((struct qdf_nbuf_cb *) \
	((skb)->cb))->u.rx.flag_sa_valid)

#define QDF_NBUF_CB_RX_RETRY_FLAG(skb) \
	(((struct qdf_nbuf_cb *) \
	((skb)->cb))->u.rx.flag_retry)

#define QDF_NBUF_CB_RX_RAW_FRAME(skb) \
	(((struct qdf_nbuf_cb *) \
	((skb)->cb))->u.rx.is_raw_frame)

#define QDF_NBUF_CB_RX_TID_VAL(skb) \
	(((struct qdf_nbuf_cb *) \
	((skb)->cb))->u.rx.tid_val)

#define QDF_NBUF_CB_RX_IS_FRAG(skb) \
	(((struct qdf_nbuf_cb *) \
	((skb)->cb))->u.rx.flag_is_frag)

#define QDF_NBUF_CB_RX_FCS_ERR(skb) \
	(((struct qdf_nbuf_cb *) \
	((skb)->cb))->u.rx.fcs_err)

#define QDF_NBUF_UPDATE_TX_PKT_COUNT(skb, PACKET_STATE) \
	qdf_nbuf_set_state(skb, PACKET_STATE)

#define QDF_NBUF_CB_TX_DATA_ATTR(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.dev.priv_cb_m.data_attr)

#define QDF_NBUF_CB_TX_FTYPE(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.ftype)


#define QDF_NBUF_CB_TX_EXTRA_FRAG_LEN(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.len)
#define QDF_NBUF_CB_TX_VDEV_CTX(skb) \
		(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.vdev_id)

/* Tx Flags Accessor Macros*/
#define QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_EFRAG(skb) \
	(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.tx.flags.bits.flag_efrag)
#define QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_NBUF(skb) \
	(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.tx.flags.bits.flag_nbuf)
#define QDF_NBUF_CB_TX_NUM_EXTRA_FRAGS(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.flags.bits.num)
#define QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_NOTIFY_COMP(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.flags.bits.flag_notify_comp)
#define QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_CHFRAG_START(skb) \
	(((struct qdf_nbuf_cb *) \
	((skb)->cb))->u.tx.flags.bits.flag_chfrag_start)
#define QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_CHFRAG_CONT(skb) \
	(((struct qdf_nbuf_cb *) \
	((skb)->cb))->u.tx.flags.bits.flag_chfrag_cont)
#define QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_CHFRAG_END(skb) \
		(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.tx.flags.bits.flag_chfrag_end)
#define QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_EXT_HEADER(skb) \
		(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.tx.flags.bits.flag_ext_header)
#define QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_FLAGS(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.flags.u8)
/* End of Tx Flags Accessor Macros */

/* Tx trace accessor macros */
#define QDF_NBUF_CB_TX_PACKET_STATE(skb)\
	(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.tx.trace.packet_state)

#define QDF_NBUF_CB_TX_IS_PACKET_PRIV(skb) \
	(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.tx.trace.is_packet_priv)

#define QDF_NBUF_CB_TX_PACKET_TRACK(skb)\
	(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.tx.trace.packet_track)

#define QDF_NBUF_CB_RX_PACKET_TRACK(skb)\
		(((struct qdf_nbuf_cb *) \
			((skb)->cb))->u.rx.trace.packet_track)

#define QDF_NBUF_CB_TX_PROTO_TYPE(skb)\
	(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.tx.trace.proto_type)

#define QDF_NBUF_CB_TX_DP_TRACE(skb)\
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.trace.dp_trace)

#define QDF_NBUF_CB_DP_TRACE_PRINT(skb)	\
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.trace.print)

#define QDF_NBUF_CB_TX_HL_HTT2_FRM(skb)	\
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.trace.htt2_frm)

#define QDF_NBUF_CB_GET_IS_BCAST(skb)\
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.trace.is_bcast)

#define QDF_NBUF_CB_GET_IS_MCAST(skb)\
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.trace.is_mcast)

#define QDF_NBUF_CB_GET_PACKET_TYPE(skb)\
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.trace.packet_type)

#define QDF_NBUF_CB_SET_BCAST(skb) \
	(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.tx.trace.is_bcast = true)

#define QDF_NBUF_CB_SET_MCAST(skb) \
	(((struct qdf_nbuf_cb *) \
		((skb)->cb))->u.tx.trace.is_mcast = true)
/* End of Tx trace accessor macros */


#define QDF_NBUF_CB_TX_EXTRA_FRAG_VADDR(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.vaddr)
#define QDF_NBUF_CB_TX_EXTRA_FRAG_PADDR(skb) \
	(((struct qdf_nbuf_cb *)((skb)->cb))->u.tx.paddr.dma_addr)

/* assume the OS provides a single fragment */
#define __qdf_nbuf_get_num_frags(skb)		   \
	(QDF_NBUF_CB_TX_NUM_EXTRA_FRAGS(skb) + 1)

#define __qdf_nbuf_reset_num_frags(skb) \
	(QDF_NBUF_CB_TX_NUM_EXTRA_FRAGS(skb) = 0)

/**
 *   end of nbuf->cb access macros
 */

typedef void (*qdf_nbuf_trace_update_t)(char *);
typedef void (*qdf_nbuf_free_t)(__qdf_nbuf_t);

#define __qdf_nbuf_mapped_paddr_get(skb) QDF_NBUF_CB_PADDR(skb)

#define __qdf_nbuf_mapped_paddr_set(skb, paddr)	\
	(QDF_NBUF_CB_PADDR(skb) = paddr)

#define __qdf_nbuf_frag_push_head(					\
	skb, frag_len, frag_vaddr, frag_paddr)				\
	do {					\
		QDF_NBUF_CB_TX_NUM_EXTRA_FRAGS(skb) = 1;		\
		QDF_NBUF_CB_TX_EXTRA_FRAG_VADDR(skb) = frag_vaddr;	\
		QDF_NBUF_CB_TX_EXTRA_FRAG_PADDR(skb) = frag_paddr;	\
		QDF_NBUF_CB_TX_EXTRA_FRAG_LEN(skb) = frag_len;		\
	} while (0)

#define __qdf_nbuf_get_frag_vaddr(skb, frag_num)		\
	((frag_num < QDF_NBUF_CB_TX_NUM_EXTRA_FRAGS(skb)) ?		\
	 QDF_NBUF_CB_TX_EXTRA_FRAG_VADDR(skb) : ((skb)->data))

#define __qdf_nbuf_get_frag_vaddr_always(skb)       \
			QDF_NBUF_CB_TX_EXTRA_FRAG_VADDR(skb)

#define __qdf_nbuf_get_frag_paddr(skb, frag_num)			\
	((frag_num < QDF_NBUF_CB_TX_NUM_EXTRA_FRAGS(skb)) ?		\
	 QDF_NBUF_CB_TX_EXTRA_FRAG_PADDR(skb) :				\
	 /* assume that the OS only provides a single fragment */	\
	 QDF_NBUF_CB_PADDR(skb))

#define __qdf_nbuf_get_tx_frag_paddr(skb) QDF_NBUF_CB_TX_EXTRA_FRAG_PADDR(skb)

#define __qdf_nbuf_get_frag_len(skb, frag_num)			\
	((frag_num < QDF_NBUF_CB_TX_NUM_EXTRA_FRAGS(skb)) ?		\
	 QDF_NBUF_CB_TX_EXTRA_FRAG_LEN(skb) : (skb)->len)

#define __qdf_nbuf_get_frag_is_wordstream(skb, frag_num)		\
	((frag_num < QDF_NBUF_CB_TX_NUM_EXTRA_FRAGS(skb))		\
	 ? (QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_EFRAG(skb))		\
	 : (QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_NBUF(skb)))

#define __qdf_nbuf_set_frag_is_wordstream(skb, frag_num, is_wstrm)	\
	do {								\
		if (frag_num >= QDF_NBUF_CB_TX_NUM_EXTRA_FRAGS(skb))	\
			frag_num = QDF_NBUF_CB_TX_MAX_EXTRA_FRAGS;	\
		if (frag_num)						\
			QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_EFRAG(skb) =  \
							      is_wstrm; \
		else					\
			QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_NBUF(skb) =   \
							      is_wstrm; \
	} while (0)

#define __qdf_nbuf_set_vdev_ctx(skb, vdev_id) \
	do { \
		QDF_NBUF_CB_TX_VDEV_CTX((skb)) = (vdev_id); \
	} while (0)

#define __qdf_nbuf_get_vdev_ctx(skb) \
	QDF_NBUF_CB_TX_VDEV_CTX((skb))

#define __qdf_nbuf_set_tx_ftype(skb, type) \
	do { \
		QDF_NBUF_CB_TX_FTYPE((skb)) = (type); \
	} while (0)

#define __qdf_nbuf_get_tx_ftype(skb) \
		 QDF_NBUF_CB_TX_FTYPE((skb))


#define __qdf_nbuf_set_rx_ftype(skb, type) \
	do { \
		QDF_NBUF_CB_RX_FTYPE((skb)) = (type); \
	} while (0)

#define __qdf_nbuf_get_rx_ftype(skb) \
		 QDF_NBUF_CB_RX_FTYPE((skb))

#define __qdf_nbuf_set_rx_chfrag_start(skb, val) \
	((QDF_NBUF_CB_RX_CHFRAG_START((skb))) = val)

#define __qdf_nbuf_is_rx_chfrag_start(skb) \
	(QDF_NBUF_CB_RX_CHFRAG_START((skb)))

#define __qdf_nbuf_set_rx_chfrag_cont(skb, val) \
	do { \
		(QDF_NBUF_CB_RX_CHFRAG_CONT((skb))) = val; \
	} while (0)

#define __qdf_nbuf_is_rx_chfrag_cont(skb) \
	(QDF_NBUF_CB_RX_CHFRAG_CONT((skb)))

#define __qdf_nbuf_set_rx_chfrag_end(skb, val) \
	((QDF_NBUF_CB_RX_CHFRAG_END((skb))) = val)

#define __qdf_nbuf_is_rx_chfrag_end(skb) \
	(QDF_NBUF_CB_RX_CHFRAG_END((skb)))

#define __qdf_nbuf_set_da_mcbc(skb, val) \
	((QDF_NBUF_CB_RX_DA_MCBC((skb))) = val)

#define __qdf_nbuf_is_da_mcbc(skb) \
	(QDF_NBUF_CB_RX_DA_MCBC((skb)))

#define __qdf_nbuf_set_da_valid(skb, val) \
	((QDF_NBUF_CB_RX_DA_VALID((skb))) = val)

#define __qdf_nbuf_is_da_valid(skb) \
	(QDF_NBUF_CB_RX_DA_VALID((skb)))

#define __qdf_nbuf_set_sa_valid(skb, val) \
	((QDF_NBUF_CB_RX_SA_VALID((skb))) = val)

#define __qdf_nbuf_is_sa_valid(skb) \
	(QDF_NBUF_CB_RX_SA_VALID((skb)))

#define __qdf_nbuf_set_rx_retry_flag(skb, val) \
	((QDF_NBUF_CB_RX_RETRY_FLAG((skb))) = val)

#define __qdf_nbuf_is_rx_retry_flag(skb) \
	(QDF_NBUF_CB_RX_RETRY_FLAG((skb)))

#define __qdf_nbuf_set_raw_frame(skb, val) \
	((QDF_NBUF_CB_RX_RAW_FRAME((skb))) = val)

#define __qdf_nbuf_is_raw_frame(skb) \
	(QDF_NBUF_CB_RX_RAW_FRAME((skb)))

#define __qdf_nbuf_get_tid_val(skb) \
	(QDF_NBUF_CB_RX_TID_VAL((skb)))

#define __qdf_nbuf_set_tid_val(skb, val) \
	((QDF_NBUF_CB_RX_TID_VAL((skb))) = val)

#define __qdf_nbuf_set_is_frag(skb, val) \
	((QDF_NBUF_CB_RX_IS_FRAG((skb))) = val)

#define __qdf_nbuf_is_frag(skb) \
	(QDF_NBUF_CB_RX_IS_FRAG((skb)))

#define __qdf_nbuf_set_tx_chfrag_start(skb, val) \
	((QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_CHFRAG_START((skb))) = val)

#define __qdf_nbuf_is_tx_chfrag_start(skb) \
	(QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_CHFRAG_START((skb)))

#define __qdf_nbuf_set_tx_chfrag_cont(skb, val) \
	do { \
		(QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_CHFRAG_CONT((skb))) = val; \
	} while (0)

#define __qdf_nbuf_is_tx_chfrag_cont(skb) \
	(QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_CHFRAG_CONT((skb)))

#define __qdf_nbuf_set_tx_chfrag_end(skb, val) \
	((QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_CHFRAG_END((skb))) = val)

#define __qdf_nbuf_is_tx_chfrag_end(skb) \
	(QDF_NBUF_CB_TX_EXTRA_FRAG_FLAGS_CHFRAG_END((skb)))

#define __qdf_nbuf_trace_set_proto_type(skb, proto_type)  \
	(QDF_NBUF_CB_TX_PROTO_TYPE(skb) = (proto_type))

#define __qdf_nbuf_trace_get_proto_type(skb) \
	QDF_NBUF_CB_TX_PROTO_TYPE(skb)

#define __qdf_nbuf_data_attr_get(skb)		\
	QDF_NBUF_CB_TX_DATA_ATTR(skb)
#define __qdf_nbuf_data_attr_set(skb, data_attr) \
	(QDF_NBUF_CB_TX_DATA_ATTR(skb) = (data_attr))

#define __qdf_nbuf_queue_walk_safe(queue, var, tvar)	\
		skb_queue_walk_safe(queue, var, tvar)

/**
 * __qdf_nbuf_num_frags_init() - init extra frags
 * @skb: sk buffer
 *
 * Return: none
 */
static inline
void __qdf_nbuf_num_frags_init(struct sk_buff *skb)
{
	QDF_NBUF_CB_TX_NUM_EXTRA_FRAGS(skb) = 0;
}

/*
 * prototypes. Implemented in qdf_nbuf.c
 */

/**
 * __qdf_nbuf_alloc() - Allocate nbuf
 * @osdev: Device handle
 * @size: Netbuf requested size
 * @reserve: headroom to start with
 * @align: Align
 * @prio: Priority
 * @func: Function name of the call site
 * @line: line number of the call site
 *
 * This allocates an nbuf aligns if needed and reserves some space in the front,
 * since the reserve is done after alignment the reserve value if being
 * unaligned will result in an unaligned address.
 *
 * Return: nbuf or %NULL if no memory
 */
__qdf_nbuf_t
__qdf_nbuf_alloc(__qdf_device_t osdev, size_t size, int reserve, int align,
		 int prio, const char *func, uint32_t line);

/**
 * __qdf_nbuf_alloc_no_recycler() - Allocates skb
 * @size: Size to be allocated for skb
 * @reserve: Reserve headroom size
 * @align: Align data
 * @func: Function name of the call site
 * @line: Line number of the callsite
 *
 * This API allocates a nbuf and aligns it if needed and reserves some headroom
 * space after the alignment where nbuf is not allocated from skb recycler pool.
 *
 * Return: Allocated nbuf pointer
 */
__qdf_nbuf_t __qdf_nbuf_alloc_no_recycler(size_t size, int reserve, int align,
					  const char *func, uint32_t line);

/**
 * __qdf_nbuf_clone() - clone the nbuf (copy is readonly)
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
__qdf_nbuf_t __qdf_nbuf_clone(__qdf_nbuf_t nbuf);

void __qdf_nbuf_free(struct sk_buff *skb);
QDF_STATUS __qdf_nbuf_map(__qdf_device_t osdev,
			struct sk_buff *skb, qdf_dma_dir_t dir);
void __qdf_nbuf_unmap(__qdf_device_t osdev,
			struct sk_buff *skb, qdf_dma_dir_t dir);
QDF_STATUS __qdf_nbuf_map_single(__qdf_device_t osdev,
				 struct sk_buff *skb, qdf_dma_dir_t dir);
void __qdf_nbuf_unmap_single(__qdf_device_t osdev,
			struct sk_buff *skb, qdf_dma_dir_t dir);
void __qdf_nbuf_reg_trace_cb(qdf_nbuf_trace_update_t cb_func_ptr);
void __qdf_nbuf_reg_free_cb(qdf_nbuf_free_t cb_func_ptr);

QDF_STATUS __qdf_nbuf_dmamap_create(qdf_device_t osdev, __qdf_dma_map_t *dmap);
void __qdf_nbuf_dmamap_destroy(qdf_device_t osdev, __qdf_dma_map_t dmap);
void __qdf_nbuf_dmamap_set_cb(__qdf_dma_map_t dmap, void *cb, void *arg);
QDF_STATUS __qdf_nbuf_map_nbytes(qdf_device_t osdev, struct sk_buff *skb,
	qdf_dma_dir_t dir, int nbytes);
void __qdf_nbuf_unmap_nbytes(qdf_device_t osdev, struct sk_buff *skb,
	qdf_dma_dir_t dir, int nbytes);

void __qdf_nbuf_sync_for_cpu(qdf_device_t osdev, struct sk_buff *skb,
	qdf_dma_dir_t dir);

void __qdf_nbuf_dma_map_info(__qdf_dma_map_t bmap, qdf_dmamap_info_t *sg);
uint32_t __qdf_nbuf_get_frag_size(__qdf_nbuf_t nbuf, uint32_t cur_frag);
void __qdf_nbuf_frag_info(struct sk_buff *skb, qdf_sglist_t  *sg);
QDF_STATUS __qdf_nbuf_frag_map(
	qdf_device_t osdev, __qdf_nbuf_t nbuf,
	int offset, qdf_dma_dir_t dir, int cur_frag);
void qdf_nbuf_classify_pkt(struct sk_buff *skb);

bool __qdf_nbuf_is_ipv4_wapi_pkt(struct sk_buff *skb);
bool __qdf_nbuf_is_ipv4_tdls_pkt(struct sk_buff *skb);
bool __qdf_nbuf_data_is_ipv4_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_ipv6_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_ipv4_mcast_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_ipv6_mcast_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_icmp_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_icmpv6_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_ipv4_udp_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_ipv4_tcp_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_ipv6_udp_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_ipv6_tcp_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_ipv4_dhcp_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_ipv6_dhcp_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_ipv6_mdns_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_ipv4_eapol_pkt(uint8_t *data);
bool __qdf_nbuf_data_is_ipv4_arp_pkt(uint8_t *data);
bool __qdf_nbuf_is_bcast_pkt(__qdf_nbuf_t nbuf);
bool __qdf_nbuf_data_is_arp_req(uint8_t *data);
bool __qdf_nbuf_data_is_arp_rsp(uint8_t *data);
uint32_t __qdf_nbuf_get_arp_src_ip(uint8_t *data);
uint32_t __qdf_nbuf_get_arp_tgt_ip(uint8_t *data);
uint8_t *__qdf_nbuf_get_dns_domain_name(uint8_t *data, uint32_t len);
bool __qdf_nbuf_data_is_dns_query(uint8_t *data);
bool __qdf_nbuf_data_is_dns_response(uint8_t *data);
bool __qdf_nbuf_data_is_tcp_syn(uint8_t *data);
bool __qdf_nbuf_data_is_tcp_syn_ack(uint8_t *data);
bool __qdf_nbuf_data_is_tcp_ack(uint8_t *data);
uint16_t __qdf_nbuf_data_get_tcp_src_port(uint8_t *data);
uint16_t __qdf_nbuf_data_get_tcp_dst_port(uint8_t *data);
bool __qdf_nbuf_data_is_icmpv4_req(uint8_t *data);
bool __qdf_nbuf_data_is_icmpv4_rsp(uint8_t *data);
uint32_t __qdf_nbuf_get_icmpv4_src_ip(uint8_t *data);
uint32_t __qdf_nbuf_get_icmpv4_tgt_ip(uint8_t *data);
enum qdf_proto_subtype  __qdf_nbuf_data_get_dhcp_subtype(uint8_t *data);
enum qdf_proto_subtype  __qdf_nbuf_data_get_eapol_subtype(uint8_t *data);
enum qdf_proto_subtype  __qdf_nbuf_data_get_arp_subtype(uint8_t *data);
enum qdf_proto_subtype  __qdf_nbuf_data_get_icmp_subtype(uint8_t *data);
enum qdf_proto_subtype  __qdf_nbuf_data_get_icmpv6_subtype(uint8_t *data);
uint8_t __qdf_nbuf_data_get_ipv4_proto(uint8_t *data);
uint8_t __qdf_nbuf_data_get_ipv6_proto(uint8_t *data);

#ifdef QDF_NBUF_GLOBAL_COUNT
int __qdf_nbuf_count_get(void);
void __qdf_nbuf_count_inc(struct sk_buff *skb);
void __qdf_nbuf_count_dec(struct sk_buff *skb);
void __qdf_nbuf_mod_init(void);
void __qdf_nbuf_mod_exit(void);

#else

static inline int __qdf_nbuf_count_get(void)
{
	return 0;
}

static inline void __qdf_nbuf_count_inc(struct sk_buff *skb)
{
	return;
}

static inline void __qdf_nbuf_count_dec(struct sk_buff *skb)
{
	return;
}

static inline void __qdf_nbuf_mod_init(void)
{
	return;
}

static inline void __qdf_nbuf_mod_exit(void)
{
	return;
}
#endif

/**
 * __qdf_to_status() - OS to QDF status conversion
 * @error : OS error
 *
 * Return: QDF status
 */
static inline QDF_STATUS __qdf_to_status(signed int error)
{
	switch (error) {
	case 0:
		return QDF_STATUS_SUCCESS;
	case ENOMEM:
	case -ENOMEM:
		return QDF_STATUS_E_NOMEM;
	default:
		return QDF_STATUS_E_NOSUPPORT;
	}
}

/**
 * __qdf_nbuf_len() - return the amount of valid data in the skb
 * @skb: Pointer to network buffer
 *
 * This API returns the amount of valid data in the skb, If there are frags
 * then it returns total length.
 *
 * Return: network buffer length
 */
static inline size_t __qdf_nbuf_len(struct sk_buff *skb)
{
	int i, extra_frag_len = 0;

	i = QDF_NBUF_CB_TX_NUM_EXTRA_FRAGS(skb);
	if (i > 0)
		extra_frag_len = QDF_NBUF_CB_TX_EXTRA_FRAG_LEN(skb);

	return extra_frag_len + skb->len;
}

/**
 * __qdf_nbuf_cat() - link two nbufs
 * @dst: Buffer to piggyback into
 * @src: Buffer to put
 *
 * Concat two nbufs, the new buf(src) is piggybacked into the older one.
 * It is callers responsibility to free the src skb.
 *
 * Return: QDF_STATUS (status of the call) if failed the src skb
 *         is released
 */
static inline QDF_STATUS
__qdf_nbuf_cat(struct sk_buff *dst, struct sk_buff *src)
{
	QDF_STATUS error = 0;

	qdf_assert(dst && src);

	/*
	 * Since pskb_expand_head unconditionally reallocates the skb->head
	 * buffer, first check whether the current buffer is already large
	 * enough.
	 */
	if (skb_tailroom(dst) < src->len) {
		error = pskb_expand_head(dst, 0, src->len, GFP_ATOMIC);
		if (error)
			return __qdf_to_status(error);
	}

	memcpy(skb_tail_pointer(dst), src->data, src->len);
	skb_put(dst, src->len);
	return __qdf_to_status(error);
}

/*
 * nbuf manipulation routines
 */
/**
 * __qdf_nbuf_headroom() - return the amount of tail space available
 * @buf: Pointer to network buffer
 *
 * Return: amount of tail room
 */
static inline int __qdf_nbuf_headroom(struct sk_buff *skb)
{
	return skb_headroom(skb);
}

/**
 * __qdf_nbuf_tailroom() - return the amount of tail space available
 * @buf: Pointer to network buffer
 *
 * Return: amount of tail room
 */
static inline uint32_t __qdf_nbuf_tailroom(struct sk_buff *skb)
{
	return skb_tailroom(skb);
}

/**
 * __qdf_nbuf_put_tail() - Puts data in the end
 * @skb: Pointer to network buffer
 * @size: size to be pushed
 *
 * Return: data pointer of this buf where new data has to be
 *         put, or NULL if there is not enough room in this buf.
 */
static inline uint8_t *__qdf_nbuf_put_tail(struct sk_buff *skb, size_t size)
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
 * __qdf_nbuf_trim_tail() - trim data out from the end
 * @skb: Pointer to network buffer
 * @size: size to be popped
 *
 * Return: none
 */
static inline void __qdf_nbuf_trim_tail(struct sk_buff *skb, size_t size)
{
	return skb_trim(skb, skb->len - size);
}


/*
 * prototypes. Implemented in qdf_nbuf.c
 */
qdf_nbuf_tx_cksum_t __qdf_nbuf_get_tx_cksum(struct sk_buff *skb);
QDF_STATUS __qdf_nbuf_set_rx_cksum(struct sk_buff *skb,
				qdf_nbuf_rx_cksum_t *cksum);
uint8_t __qdf_nbuf_get_tid(struct sk_buff *skb);
void __qdf_nbuf_set_tid(struct sk_buff *skb, uint8_t tid);
uint8_t __qdf_nbuf_get_exemption_type(struct sk_buff *skb);
void __qdf_nbuf_ref(struct sk_buff *skb);
int __qdf_nbuf_shared(struct sk_buff *skb);

/**
 * __qdf_nbuf_get_nr_frags() - return the number of fragments in an skb,
 * @skb: sk buff
 *
 * Return: number of fragments
 */
static inline size_t __qdf_nbuf_get_nr_frags(struct sk_buff *skb)
{
	return skb_shinfo(skb)->nr_frags;
}

/*
 * qdf_nbuf_pool_delete() implementation - do nothing in linux
 */
#define __qdf_nbuf_pool_delete(osdev)

/**
 * __qdf_nbuf_copy() - returns a private copy of the skb
 * @skb: Pointer to network buffer
 *
 * This API returns a private copy of the skb, the skb returned is completely
 *  modifiable by callers
 *
 * Return: skb or NULL
 */
static inline struct sk_buff *__qdf_nbuf_copy(struct sk_buff *skb)
{
	struct sk_buff *skb_new = NULL;

	skb_new = skb_copy(skb, GFP_ATOMIC);
	if (skb_new) {
		__qdf_nbuf_count_inc(skb_new);
	}
	return skb_new;
}

#define __qdf_nbuf_reserve      skb_reserve

/**
 * __qdf_nbuf_set_data_pointer() - set buffer data pointer
 * @skb: Pointer to network buffer
 * @data: data pointer
 *
 * Return: none
 */
static inline void
__qdf_nbuf_set_data_pointer(struct sk_buff *skb, uint8_t *data)
{
	skb->data = data;
}

/**
 * __qdf_nbuf_set_len() - set buffer data length
 * @skb: Pointer to network buffer
 * @len: data length
 *
 * Return: none
 */
static inline void
__qdf_nbuf_set_len(struct sk_buff *skb, uint32_t len)
{
	skb->len = len;
}

/**
 * __qdf_nbuf_set_tail_pointer() - set buffer data tail pointer
 * @skb: Pointer to network buffer
 * @len: skb data length
 *
 * Return: none
 */
static inline void
__qdf_nbuf_set_tail_pointer(struct sk_buff *skb, int len)
{
	skb_set_tail_pointer(skb, len);
}

/**
 * __qdf_nbuf_unlink_no_lock() - unlink an skb from skb queue
 * @skb: Pointer to network buffer
 * @list: list to use
 *
 * This is a lockless version, driver must acquire locks if it
 * needs to synchronize
 *
 * Return: none
 */
static inline void
__qdf_nbuf_unlink_no_lock(struct sk_buff *skb, struct sk_buff_head *list)
{
	__skb_unlink(skb, list);
}

/**
 * __qdf_nbuf_reset() - reset the buffer data and pointer
 * @buf: Network buf instance
 * @reserve: reserve
 * @align: align
 *
 * Return: none
 */
static inline void
__qdf_nbuf_reset(struct sk_buff *skb, int reserve, int align)
{
	int offset;

	skb_push(skb, skb_headroom(skb));
	skb_put(skb, skb_tailroom(skb));
	memset(skb->data, 0x0, skb->len);
	skb_trim(skb, 0);
	skb_reserve(skb, NET_SKB_PAD);
	memset(skb->cb, 0x0, sizeof(skb->cb));

	/*
	 * The default is for netbuf fragments to be interpreted
	 * as wordstreams rather than bytestreams.
	 */
	QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_EFRAG(skb) = 1;
	QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_NBUF(skb) = 1;

	/*
	 * Align & make sure that the tail & data are adjusted properly
	 */

	if (align) {
		offset = ((unsigned long)skb->data) % align;
		if (offset)
			skb_reserve(skb, align - offset);
	}

	skb_reserve(skb, reserve);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
/**
 * qdf_nbuf_dev_scratch_is_supported() - dev_scratch support for network buffer
 *                                       in kernel
 *
 * Return: true if dev_scratch is supported
 *         false if dev_scratch is not supported
 */
static inline bool __qdf_nbuf_is_dev_scratch_supported(void)
{
	return true;
}

/**
 * qdf_nbuf_get_dev_scratch() - get dev_scratch of network buffer
 * @skb: Pointer to network buffer
 *
 * Return: dev_scratch if dev_scratch supported
 *         0 if dev_scratch not supported
 */
static inline unsigned long __qdf_nbuf_get_dev_scratch(struct sk_buff *skb)
{
	return skb->dev_scratch;
}

/**
 * qdf_nbuf_set_dev_scratch() - set dev_scratch of network buffer
 * @skb: Pointer to network buffer
 * @value: value to be set in dev_scratch of network buffer
 *
 * Return: void
 */
static inline void
__qdf_nbuf_set_dev_scratch(struct sk_buff *skb, unsigned long value)
{
	skb->dev_scratch = value;
}
#else
static inline bool __qdf_nbuf_is_dev_scratch_supported(void)
{
	return false;
}

static inline unsigned long __qdf_nbuf_get_dev_scratch(struct sk_buff *skb)
{
	return 0;
}

static inline void
__qdf_nbuf_set_dev_scratch(struct sk_buff *skb, unsigned long value)
{
}
#endif /* KERNEL_VERSION(4, 14, 0) */

/**
 * __qdf_nbuf_head() - return the pointer the skb's head pointer
 * @skb: Pointer to network buffer
 *
 * Return: Pointer to head buffer
 */
static inline uint8_t *__qdf_nbuf_head(struct sk_buff *skb)
{
	return skb->head;
}

/**
 * __qdf_nbuf_data() - return the pointer to data header in the skb
 * @skb: Pointer to network buffer
 *
 * Return: Pointer to skb data
 */
static inline uint8_t *__qdf_nbuf_data(struct sk_buff *skb)
{
	return skb->data;
}

static inline uint8_t *__qdf_nbuf_data_addr(struct sk_buff *skb)
{
	return (uint8_t *)&skb->data;
}

/**
 * __qdf_nbuf_get_protocol() - return the protocol value of the skb
 * @skb: Pointer to network buffer
 *
 * Return: skb protocol
 */
static inline uint16_t __qdf_nbuf_get_protocol(struct sk_buff *skb)
{
	return skb->protocol;
}

/**
 * __qdf_nbuf_get_ip_summed() - return the ip checksum value of the skb
 * @skb: Pointer to network buffer
 *
 * Return: skb ip_summed
 */
static inline uint8_t __qdf_nbuf_get_ip_summed(struct sk_buff *skb)
{
	return skb->ip_summed;
}

/**
 * __qdf_nbuf_set_ip_summed() - sets the ip_summed value of the skb
 * @skb: Pointer to network buffer
 * @ip_summed: ip checksum
 *
 * Return: none
 */
static inline void __qdf_nbuf_set_ip_summed(struct sk_buff *skb,
		 uint8_t ip_summed)
{
	skb->ip_summed = ip_summed;
}

/**
 * __qdf_nbuf_get_priority() - return the priority value of the skb
 * @skb: Pointer to network buffer
 *
 * Return: skb priority
 */
static inline uint32_t __qdf_nbuf_get_priority(struct sk_buff *skb)
{
	return skb->priority;
}

/**
 * __qdf_nbuf_set_priority() - sets the priority value of the skb
 * @skb: Pointer to network buffer
 * @p: priority
 *
 * Return: none
 */
static inline void __qdf_nbuf_set_priority(struct sk_buff *skb, uint32_t p)
{
	skb->priority = p;
}

/**
 * __qdf_nbuf_set_next() - sets the next skb pointer of the current skb
 * @skb: Current skb
 * @next_skb: Next skb
 *
 * Return: void
 */
static inline void
__qdf_nbuf_set_next(struct sk_buff *skb, struct sk_buff *skb_next)
{
	skb->next = skb_next;
}

/**
 * __qdf_nbuf_next() - return the next skb pointer of the current skb
 * @skb: Current skb
 *
 * Return: the next skb pointed to by the current skb
 */
static inline struct sk_buff *__qdf_nbuf_next(struct sk_buff *skb)
{
	return skb->next;
}

/**
 * __qdf_nbuf_set_next_ext() - sets the next skb pointer of the current skb
 * @skb: Current skb
 * @next_skb: Next skb
 *
 * This fn is used to link up extensions to the head skb. Does not handle
 * linking to the head
 *
 * Return: none
 */
static inline void
__qdf_nbuf_set_next_ext(struct sk_buff *skb, struct sk_buff *skb_next)
{
	skb->next = skb_next;
}

/**
 * __qdf_nbuf_next_ext() - return the next skb pointer of the current skb
 * @skb: Current skb
 *
 * Return: the next skb pointed to by the current skb
 */
static inline struct sk_buff *__qdf_nbuf_next_ext(struct sk_buff *skb)
{
	return skb->next;
}

/**
 * __qdf_nbuf_append_ext_list() - link list of packet extensions to the head
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
__qdf_nbuf_append_ext_list(struct sk_buff *skb_head,
			struct sk_buff *ext_list, size_t ext_len)
{
	skb_shinfo(skb_head)->frag_list = ext_list;
	skb_head->data_len += ext_len;
	skb_head->len += ext_len;
}

/**
 * __qdf_nbuf_get_ext_list() - Get the link to extended nbuf list.
 * @head_buf: Network buf holding head segment (single)
 *
 * This ext_list is populated when we have Jumbo packet, for example in case of
 * monitor mode amsdu packet reception, and are stiched using frags_list.
 *
 * Return: Network buf list holding linked extensions from head buf.
 */
static inline struct sk_buff *__qdf_nbuf_get_ext_list(struct sk_buff *head_buf)
{
	return (skb_shinfo(head_buf)->frag_list);
}

/**
 * __qdf_nbuf_get_age() - return the checksum value of the skb
 * @skb: Pointer to network buffer
 *
 * Return: checksum value
 */
static inline uint32_t __qdf_nbuf_get_age(struct sk_buff *skb)
{
	return skb->csum;
}

/**
 * __qdf_nbuf_set_age() - sets the checksum value of the skb
 * @skb: Pointer to network buffer
 * @v: Value
 *
 * Return: none
 */
static inline void __qdf_nbuf_set_age(struct sk_buff *skb, uint32_t v)
{
	skb->csum = v;
}

/**
 * __qdf_nbuf_adj_age() - adjusts the checksum/age value of the skb
 * @skb: Pointer to network buffer
 * @adj: Adjustment value
 *
 * Return: none
 */
static inline void __qdf_nbuf_adj_age(struct sk_buff *skb, uint32_t adj)
{
	skb->csum -= adj;
}

/**
 * __qdf_nbuf_copy_bits() - return the length of the copy bits for skb
 * @skb: Pointer to network buffer
 * @offset: Offset value
 * @len: Length
 * @to: Destination pointer
 *
 * Return: length of the copy bits for skb
 */
static inline int32_t
__qdf_nbuf_copy_bits(struct sk_buff *skb, int32_t offset, int32_t len, void *to)
{
	return skb_copy_bits(skb, offset, to, len);
}

/**
 * __qdf_nbuf_set_pktlen() - sets the length of the skb and adjust the tail
 * @skb: Pointer to network buffer
 * @len:  Packet length
 *
 * Return: none
 */
static inline void __qdf_nbuf_set_pktlen(struct sk_buff *skb, uint32_t len)
{
	if (skb->len > len) {
		skb_trim(skb, len);
	} else {
		if (skb_tailroom(skb) < len - skb->len) {
			if (unlikely(pskb_expand_head(skb, 0,
				len - skb->len - skb_tailroom(skb),
				GFP_ATOMIC))) {
				dev_kfree_skb_any(skb);
				qdf_assert(0);
			}
		}
		skb_put(skb, (len - skb->len));
	}
}

/**
 * __qdf_nbuf_set_protocol() - sets the protocol value of the skb
 * @skb: Pointer to network buffer
 * @protocol: Protocol type
 *
 * Return: none
 */
static inline void
__qdf_nbuf_set_protocol(struct sk_buff *skb, uint16_t protocol)
{
	skb->protocol = protocol;
}

#define __qdf_nbuf_set_tx_htt2_frm(skb, candi) \
	(QDF_NBUF_CB_TX_HL_HTT2_FRM(skb) = (candi))

#define __qdf_nbuf_get_tx_htt2_frm(skb)	\
	QDF_NBUF_CB_TX_HL_HTT2_FRM(skb)

void __qdf_dmaaddr_to_32s(qdf_dma_addr_t dmaaddr,
				      uint32_t *lo, uint32_t *hi);

uint32_t __qdf_nbuf_get_tso_info(qdf_device_t osdev, struct sk_buff *skb,
	struct qdf_tso_info_t *tso_info);

void __qdf_nbuf_unmap_tso_segment(qdf_device_t osdev,
			  struct qdf_tso_seg_elem_t *tso_seg,
			  bool is_last_seg);

#ifdef FEATURE_TSO
/**
 * __qdf_nbuf_get_tcp_payload_len() - function to return the tcp
 *                                    payload len
 * @skb: buffer
 *
 * Return: size
 */
size_t __qdf_nbuf_get_tcp_payload_len(struct sk_buff *skb);
uint32_t __qdf_nbuf_get_tso_num_seg(struct sk_buff *skb);

#else
static inline
size_t __qdf_nbuf_get_tcp_payload_len(struct sk_buff *skb)
{
	return 0;
}

static inline uint32_t __qdf_nbuf_get_tso_num_seg(struct sk_buff *skb)
{
	return 0;
}

#endif /* FEATURE_TSO */

static inline bool __qdf_nbuf_is_tso(struct sk_buff *skb)
{
	if (skb_is_gso(skb) &&
		(skb_is_gso_v6(skb) ||
		(skb_shinfo(skb)->gso_type & SKB_GSO_TCPV4)))
		return true;
	else
		return false;
}

struct sk_buff *__qdf_nbuf_inc_users(struct sk_buff *skb);

int __qdf_nbuf_get_users(struct sk_buff *skb);

/**
 * __qdf_nbuf_tx_info_get() - Modify pkt_type, set pkt_subtype,
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
#define __qdf_nbuf_tx_info_get(skb, pkt_type,			\
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
 * __qdf_nbuf_peek_header() - return the header's addr & m_len
 * @skb: Pointer to network buffer
 * @addr: Pointer to store header's addr
 * @m_len: network buffer length
 *
 * Return: none
 */
static inline void
__qdf_nbuf_peek_header(struct sk_buff *skb, uint8_t **addr, uint32_t *len)
{
	*addr = skb->data;
	*len = skb->len;
}

/**
 * typedef struct __qdf_nbuf_queue_t -  network buffer queue
 * @head: Head pointer
 * @tail: Tail pointer
 * @qlen: Queue length
 */
typedef struct __qdf_nbuf_qhead {
	struct sk_buff *head;
	struct sk_buff *tail;
	unsigned int qlen;
} __qdf_nbuf_queue_t;

/******************Functions *************/

/**
 * __qdf_nbuf_queue_init() - initiallize the queue head
 * @qhead: Queue head
 *
 * Return: QDF status
 */
static inline QDF_STATUS __qdf_nbuf_queue_init(__qdf_nbuf_queue_t *qhead)
{
	memset(qhead, 0, sizeof(struct __qdf_nbuf_qhead));
	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_nbuf_queue_add() - add an skb in the tail of the queue
 * @qhead: Queue head
 * @skb: Pointer to network buffer
 *
 * This is a lockless version, driver must acquire locks if it
 * needs to synchronize
 *
 * Return: none
 */
static inline void
__qdf_nbuf_queue_add(__qdf_nbuf_queue_t *qhead, struct sk_buff *skb)
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
 * __qdf_nbuf_queue_append() - Append src list at the end of dest list
 * @dest: target netbuf queue
 * @src:  source netbuf queue
 *
 * Return: target netbuf queue
 */
static inline __qdf_nbuf_queue_t *
__qdf_nbuf_queue_append(__qdf_nbuf_queue_t *dest, __qdf_nbuf_queue_t *src)
{
	if (!dest)
		return NULL;
	else if (!src || !(src->head))
		return dest;

	if (!(dest->head))
		dest->head = src->head;
	else
		dest->tail->next = src->head;

	dest->tail = src->tail;
	dest->qlen += src->qlen;
	return dest;
}

/**
 * __qdf_nbuf_queue_insert_head() - add an skb at  the head  of the queue
 * @qhead: Queue head
 * @skb: Pointer to network buffer
 *
 * This is a lockless version, driver must acquire locks if it needs to
 * synchronize
 *
 * Return: none
 */
static inline void
__qdf_nbuf_queue_insert_head(__qdf_nbuf_queue_t *qhead, __qdf_nbuf_t skb)
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
 * __qdf_nbuf_queue_remove() - remove a skb from the head of the queue
 * @qhead: Queue head
 *
 * This is a lockless version. Driver should take care of the locks
 *
 * Return: skb or NULL
 */
static inline
struct sk_buff *__qdf_nbuf_queue_remove(__qdf_nbuf_queue_t *qhead)
{
	__qdf_nbuf_t tmp = NULL;

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
 * __qdf_nbuf_queue_first() - returns the first skb in the queue
 * @qhead: head of queue
 *
 * Return: NULL if the queue is empty
 */
static inline struct sk_buff *
__qdf_nbuf_queue_first(__qdf_nbuf_queue_t *qhead)
{
	return qhead->head;
}

/**
 * __qdf_nbuf_queue_last() - returns the last skb in the queue
 * @qhead: head of queue
 *
 * Return: NULL if the queue is empty
 */
static inline struct sk_buff *
__qdf_nbuf_queue_last(__qdf_nbuf_queue_t *qhead)
{
	return qhead->tail;
}

/**
 * __qdf_nbuf_queue_len() - return the queue length
 * @qhead: Queue head
 *
 * Return: Queue length
 */
static inline uint32_t __qdf_nbuf_queue_len(__qdf_nbuf_queue_t *qhead)
{
	return qhead->qlen;
}

/**
 * __qdf_nbuf_queue_next() - return the next skb from packet chain
 * @skb: Pointer to network buffer
 *
 * This API returns the next skb from packet chain, remember the skb is
 * still in the queue
 *
 * Return: NULL if no packets are there
 */
static inline struct sk_buff *__qdf_nbuf_queue_next(struct sk_buff *skb)
{
	return skb->next;
}

/**
 * __qdf_nbuf_is_queue_empty() - check if the queue is empty or not
 * @qhead: Queue head
 *
 * Return: true if length is 0 else false
 */
static inline bool __qdf_nbuf_is_queue_empty(__qdf_nbuf_queue_t *qhead)
{
	return qhead->qlen == 0;
}

/*
 * Use sk_buff_head as the implementation of qdf_nbuf_queue_t.
 * Because the queue head will most likely put in some structure,
 * we don't use pointer type as the definition.
 */

/*
 * Use sk_buff_head as the implementation of qdf_nbuf_queue_t.
 * Because the queue head will most likely put in some structure,
 * we don't use pointer type as the definition.
 */

static inline void
__qdf_nbuf_set_send_complete_flag(struct sk_buff *skb, bool flag)
{
}

/**
 * __qdf_nbuf_realloc_headroom() - This keeps the skb shell intact
 *        expands the headroom
 *        in the data region. In case of failure the skb is released.
 * @skb: sk buff
 * @headroom: size of headroom
 *
 * Return: skb or NULL
 */
static inline struct sk_buff *
__qdf_nbuf_realloc_headroom(struct sk_buff *skb, uint32_t headroom)
{
	if (pskb_expand_head(skb, headroom, 0, GFP_ATOMIC)) {
		dev_kfree_skb_any(skb);
		skb = NULL;
	}
	return skb;
}

/**
 * __qdf_nbuf_realloc_tailroom() - This keeps the skb shell intact
 *        exapnds the tailroom
 *        in data region. In case of failure it releases the skb.
 * @skb: sk buff
 * @tailroom: size of tailroom
 *
 * Return: skb or NULL
 */
static inline struct sk_buff *
__qdf_nbuf_realloc_tailroom(struct sk_buff *skb, uint32_t tailroom)
{
	if (likely(!pskb_expand_head(skb, 0, tailroom, GFP_ATOMIC)))
		return skb;
	/**
	 * unlikely path
	 */
	dev_kfree_skb_any(skb);
	return NULL;
}

/**
 * __qdf_nbuf_linearize() - skb linearize
 * @skb: sk buff
 *
 * create a version of the specified nbuf whose contents
 * can be safely modified without affecting other
 * users.If the nbuf is non-linear then this function
 * linearize. if unable to linearize returns -ENOMEM on
 * success 0 is returned
 *
 * Return: 0 on Success, -ENOMEM on failure is returned.
 */
static inline int
__qdf_nbuf_linearize(struct sk_buff *skb)
{
	return skb_linearize(skb);
}

/**
 * __qdf_nbuf_unshare() - skb unshare
 * @skb: sk buff
 *
 * create a version of the specified nbuf whose contents
 * can be safely modified without affecting other
 * users.If the nbuf is a clone then this function
 * creates a new copy of the data. If the buffer is not
 * a clone the original buffer is returned.
 *
 * Return: skb or NULL
 */
static inline struct sk_buff *
__qdf_nbuf_unshare(struct sk_buff *skb)
{
	struct sk_buff *skb_new;

	__qdf_frag_count_dec(__qdf_nbuf_get_nr_frags(skb));

	skb_new = skb_unshare(skb, GFP_ATOMIC);
	if (skb_new)
		__qdf_frag_count_inc(__qdf_nbuf_get_nr_frags(skb_new));

	return skb_new;
}

/**
 * __qdf_nbuf_is_cloned() - test whether the nbuf is cloned or not
 *@buf: sk buff
 *
 * Return: true/false
 */
static inline bool __qdf_nbuf_is_cloned(struct sk_buff *skb)
{
	return skb_cloned(skb);
}

/**
 * __qdf_nbuf_pool_init() - init pool
 * @net: net handle
 *
 * Return: QDF status
 */
static inline QDF_STATUS __qdf_nbuf_pool_init(qdf_net_handle_t net)
{
	return QDF_STATUS_SUCCESS;
}

/*
 * adf_nbuf_pool_delete() implementation - do nothing in linux
 */
#define __qdf_nbuf_pool_delete(osdev)

/**
 * __qdf_nbuf_expand() - Expand both tailroom & headroom. In case of failure
 *        release the skb.
 * @skb: sk buff
 * @headroom: size of headroom
 * @tailroom: size of tailroom
 *
 * Return: skb or NULL
 */
static inline struct sk_buff *
__qdf_nbuf_expand(struct sk_buff *skb, uint32_t headroom, uint32_t tailroom)
{
	if (likely(!pskb_expand_head(skb, headroom, tailroom, GFP_ATOMIC)))
		return skb;

	dev_kfree_skb_any(skb);
	return NULL;
}

/**
 * __qdf_nbuf_copy_expand() - copy and expand nbuf
 * @buf: Network buf instance
 * @headroom: Additional headroom to be added
 * @tailroom: Additional tailroom to be added
 *
 * Return: New nbuf that is a copy of buf, with additional head and tailroom
 *	or NULL if there is no memory
 */
static inline struct sk_buff *
__qdf_nbuf_copy_expand(struct sk_buff *buf, int headroom, int tailroom)
{
	return skb_copy_expand(buf, headroom, tailroom, GFP_ATOMIC);
}

/**
 * __qdf_nbuf_get_ref_fraglist() - get reference to fragments
 * @buf: Network buf instance
 *
 * Return: void
 */
static inline void
__qdf_nbuf_get_ref_fraglist(struct sk_buff *buf)
{
	struct sk_buff *list;

	skb_walk_frags(buf, list)
		skb_get(list);
}

/**
 * __qdf_nbuf_tx_cksum_info() - tx checksum info
 *
 * Return: true/false
 */
static inline bool
__qdf_nbuf_tx_cksum_info(struct sk_buff *skb, uint8_t **hdr_off,
			 uint8_t **where)
{
	qdf_assert(0);
	return false;
}

/**
 * __qdf_nbuf_reset_ctxt() - mem zero control block
 * @nbuf: buffer
 *
 * Return: none
 */
static inline void __qdf_nbuf_reset_ctxt(__qdf_nbuf_t nbuf)
{
	qdf_mem_zero(nbuf->cb, sizeof(nbuf->cb));
}

/**
 * __qdf_nbuf_network_header() - get network header
 * @buf: buffer
 *
 * Return: network header pointer
 */
static inline void *__qdf_nbuf_network_header(__qdf_nbuf_t buf)
{
	return skb_network_header(buf);
}

/**
 * __qdf_nbuf_transport_header() - get transport header
 * @buf: buffer
 *
 * Return: transport header pointer
 */
static inline void *__qdf_nbuf_transport_header(__qdf_nbuf_t buf)
{
	return skb_transport_header(buf);
}

/**
 *  __qdf_nbuf_tcp_tso_size() - return the size of TCP segment size (MSS),
 *  passed as part of network buffer by network stack
 * @skb: sk buff
 *
 * Return: TCP MSS size
 *
 */
static inline size_t __qdf_nbuf_tcp_tso_size(struct sk_buff *skb)
{
	return skb_shinfo(skb)->gso_size;
}

/**
 * __qdf_nbuf_init() - Re-initializes the skb for re-use
 * @nbuf: sk buff
 *
 * Return: none
 */
void __qdf_nbuf_init(__qdf_nbuf_t nbuf);

/*
 *  __qdf_nbuf_get_cb() - returns a pointer to skb->cb
 * @nbuf: sk buff
 *
 * Return: void ptr
 */
static inline void *
__qdf_nbuf_get_cb(__qdf_nbuf_t nbuf)
{
	return (void *)nbuf->cb;
}

/**
 * __qdf_nbuf_headlen() - return the length of linear buffer of the skb
 * @skb: sk buff
 *
 * Return: head size
 */
static inline size_t
__qdf_nbuf_headlen(struct sk_buff *skb)
{
	return skb_headlen(skb);
}

/**
 * __qdf_nbuf_tso_tcp_v4() - to check if the TSO TCP pkt is a IPv4 or not.
 * @buf: sk buff
 *
 * Return: true/false
 */
static inline bool __qdf_nbuf_tso_tcp_v4(struct sk_buff *skb)
{
	return skb_shinfo(skb)->gso_type == SKB_GSO_TCPV4 ? 1 : 0;
}

/**
 * __qdf_nbuf_tso_tcp_v6() - to check if the TSO TCP pkt is a IPv6 or not.
 * @buf: sk buff
 *
 * Return: true/false
 */
static inline bool __qdf_nbuf_tso_tcp_v6(struct sk_buff *skb)
{
	return skb_shinfo(skb)->gso_type == SKB_GSO_TCPV6 ? 1 : 0;
}

/**
 * __qdf_nbuf_l2l3l4_hdr_len() - return the l2+l3+l4 hdr length of the skb
 * @skb: sk buff
 *
 * Return: size of l2+l3+l4 header length
 */
static inline size_t __qdf_nbuf_l2l3l4_hdr_len(struct sk_buff *skb)
{
	return skb_transport_offset(skb) + tcp_hdrlen(skb);
}

/**
 * __qdf_nbuf_is_nonlinear() - test whether the nbuf is nonlinear or not
 * @buf: sk buff
 *
 * Return:  true/false
 */
static inline bool __qdf_nbuf_is_nonlinear(struct sk_buff *skb)
{
	if (skb_is_nonlinear(skb))
		return true;
	else
		return false;
}

/**
 * __qdf_nbuf_tcp_seq() - get the TCP sequence number of the  skb
 * @buf: sk buff
 *
 * Return: TCP sequence number
 */
static inline uint32_t __qdf_nbuf_tcp_seq(struct sk_buff *skb)
{
	return ntohl(tcp_hdr(skb)->seq);
}

/**
 * __qdf_nbuf_get_priv_ptr() - get the priv pointer from the nbuf'f private space
 *@buf: sk buff
 *
 * Return: data pointer to typecast into your priv structure
 */
static inline uint8_t *
__qdf_nbuf_get_priv_ptr(struct sk_buff *skb)
{
	return &skb->cb[8];
}

/**
 * __qdf_nbuf_mark_wakeup_frame() - mark wakeup frame.
 * @buf: Pointer to nbuf
 *
 * Return: None
 */
static inline void
__qdf_nbuf_mark_wakeup_frame(__qdf_nbuf_t buf)
{
	buf->mark |= QDF_MARK_FIRST_WAKEUP_PACKET;
}

/**
 * __qdf_nbuf_record_rx_queue() - set rx queue in skb
 *
 * @buf: sk buff
 * @queue_id: Queue id
 *
 * Return: void
 */
static inline void
__qdf_nbuf_record_rx_queue(struct sk_buff *skb, uint16_t queue_id)
{
	skb_record_rx_queue(skb, queue_id);
}

/**
 * __qdf_nbuf_get_queue_mapping() - get the queue mapping set by linux kernel
 *
 * @buf: sk buff
 *
 * Return: Queue mapping
 */
static inline uint16_t
__qdf_nbuf_get_queue_mapping(struct sk_buff *skb)
{
	return skb->queue_mapping;
}

/**
 * __qdf_nbuf_set_timestamp() - set the timestamp for frame
 *
 * @buf: sk buff
 *
 * Return: void
 */
static inline void
__qdf_nbuf_set_timestamp(struct sk_buff *skb)
{
	__net_timestamp(skb);
}

/**
 * __qdf_nbuf_get_timestamp() - get the timestamp for frame
 *
 * @buf: sk buff
 *
 * Return: timestamp stored in skb in ms
 */
static inline uint64_t
__qdf_nbuf_get_timestamp(struct sk_buff *skb)
{
	return ktime_to_ms(skb_get_ktime(skb));
}

/**
 * __qdf_nbuf_get_timedelta_ms() - get time difference in ms
 *
 * @buf: sk buff
 *
 * Return: time difference in ms
 */
static inline uint64_t
__qdf_nbuf_get_timedelta_ms(struct sk_buff *skb)
{
	return ktime_to_ms(net_timedelta(skb->tstamp));
}

/**
 * __qdf_nbuf_get_timedelta_us() - get time difference in micro seconds
 *
 * @buf: sk buff
 *
 * Return: time difference in micro seconds
 */
static inline uint64_t
__qdf_nbuf_get_timedelta_us(struct sk_buff *skb)
{
	return ktime_to_us(net_timedelta(skb->tstamp));
}

/**
 * __qdf_nbuf_orphan() - orphan a nbuf
 * @skb: sk buff
 *
 * If a buffer currently has an owner then we call the
 * owner's destructor function
 *
 * Return: void
 */
static inline void __qdf_nbuf_orphan(struct sk_buff *skb)
{
	return skb_orphan(skb);
}

/**
 * __qdf_nbuf_map_nbytes_single() - map nbytes
 * @osdev: os device
 * @buf: buffer
 * @dir: direction
 * @nbytes: number of bytes
 *
 * Return: QDF_STATUS
 */
#ifdef A_SIMOS_DEVHOST
static inline QDF_STATUS __qdf_nbuf_map_nbytes_single(
		qdf_device_t osdev, struct sk_buff *buf,
		qdf_dma_dir_t dir, int nbytes)
{
	qdf_dma_addr_t paddr;

	QDF_NBUF_CB_PADDR(buf) = paddr = buf->data;
	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS __qdf_nbuf_map_nbytes_single(
		qdf_device_t osdev, struct sk_buff *buf,
		qdf_dma_dir_t dir, int nbytes)
{
	qdf_dma_addr_t paddr;

	/* assume that the OS only provides a single fragment */
	QDF_NBUF_CB_PADDR(buf) = paddr =
		dma_map_single(osdev->dev, buf->data,
			       nbytes, __qdf_dma_dir_to_os(dir));
	return dma_mapping_error(osdev->dev, paddr) ?
		QDF_STATUS_E_FAULT : QDF_STATUS_SUCCESS;
}
#endif
/**
 * __qdf_nbuf_unmap_nbytes_single() - unmap nbytes
 * @osdev: os device
 * @buf: buffer
 * @dir: direction
 * @nbytes: number of bytes
 *
 * Return: none
 */
#if defined(A_SIMOS_DEVHOST)
static inline void
__qdf_nbuf_unmap_nbytes_single(qdf_device_t osdev, struct sk_buff *buf,
			       qdf_dma_dir_t dir, int nbytes)
{
}

#else
static inline void
__qdf_nbuf_unmap_nbytes_single(qdf_device_t osdev, struct sk_buff *buf,
			       qdf_dma_dir_t dir, int nbytes)
{
	qdf_dma_addr_t paddr = QDF_NBUF_CB_PADDR(buf);

	if (qdf_likely(paddr)) {
		dma_unmap_single(osdev->dev, paddr, nbytes,
				 __qdf_dma_dir_to_os(dir));
		return;
	}
}
#endif

static inline struct sk_buff *
__qdf_nbuf_queue_head_dequeue(struct sk_buff_head *skb_queue_head)
{
	return skb_dequeue(skb_queue_head);
}

static inline
uint32_t __qdf_nbuf_queue_head_qlen(struct sk_buff_head *skb_queue_head)
{
	return skb_queue_head->qlen;
}

static inline
void __qdf_nbuf_queue_head_enqueue_tail(struct sk_buff_head *skb_queue_head,
					struct sk_buff *skb)
{
	return skb_queue_tail(skb_queue_head, skb);
}

static inline
void __qdf_nbuf_queue_head_init(struct sk_buff_head *skb_queue_head)
{
	return skb_queue_head_init(skb_queue_head);
}

static inline
void __qdf_nbuf_queue_head_purge(struct sk_buff_head *skb_queue_head)
{
	return skb_queue_purge(skb_queue_head);
}

/**
 * __qdf_nbuf_queue_head_lock() - Acquire the skb list lock
 * @head: skb list for which lock is to be acquired
 *
 * Return: void
 */
static inline
void __qdf_nbuf_queue_head_lock(struct sk_buff_head *skb_queue_head)
{
	spin_lock_bh(&skb_queue_head->lock);
}

/**
 * __qdf_nbuf_queue_head_unlock() - Release the skb list lock
 * @head: skb list for which lock is to be release
 *
 * Return: void
 */
static inline
void __qdf_nbuf_queue_head_unlock(struct sk_buff_head *skb_queue_head)
{
	spin_unlock_bh(&skb_queue_head->lock);
}

/**
 * __qdf_nbuf_get_frag_size_by_idx() - Get nbuf frag size at index idx
 * @nbuf: qdf_nbuf_t
 * @idx: Index for which frag size is requested
 *
 * Return: Frag size
 */
static inline unsigned int __qdf_nbuf_get_frag_size_by_idx(__qdf_nbuf_t nbuf,
							   uint8_t idx)
{
	unsigned int size = 0;

	if (likely(idx < __QDF_NBUF_MAX_FRAGS))
		size = skb_frag_size(&skb_shinfo(nbuf)->frags[idx]);
	return size;
}

/**
 * __qdf_nbuf_get_frag_address() - Get nbuf frag address at index idx
 * @nbuf: qdf_nbuf_t
 * @idx: Index for which frag address is requested
 *
 * Return: Frag address in success, else NULL
 */
static inline __qdf_frag_t __qdf_nbuf_get_frag_addr(__qdf_nbuf_t nbuf,
						    uint8_t idx)
{
	__qdf_frag_t frag_addr = NULL;

	if (likely(idx < __QDF_NBUF_MAX_FRAGS))
		frag_addr = skb_frag_address(&skb_shinfo(nbuf)->frags[idx]);
	return frag_addr;
}

/**
 * __qdf_nbuf_trim_add_frag_size() - Increase/Decrease frag_size by size
 * @nbuf: qdf_nbuf_t
 * @idx: Frag index
 * @size: Size by which frag_size needs to be increased/decreased
 *        +Ve means increase, -Ve means decrease
 * @truesize: truesize
 */
static inline void __qdf_nbuf_trim_add_frag_size(__qdf_nbuf_t nbuf, uint8_t idx,
						 int size,
						 unsigned int truesize)
{
	skb_coalesce_rx_frag(nbuf, idx, size, truesize);
}

/**
 * __qdf_nbuf_move_frag_page_offset() - Move frag page_offset by size
 *          and adjust length by size.
 * @nbuf: qdf_nbuf_t
 * @idx: Frag index
 * @offset: Frag page offset should be moved by offset.
 *      +Ve - Move offset forward.
 *      -Ve - Move offset backward.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS __qdf_nbuf_move_frag_page_offset(__qdf_nbuf_t nbuf, uint8_t idx,
					    int offset);

/**
 * __qdf_nbuf_add_rx_frag() - Add frag to nbuf at nr_frag index
 * @buf: Frag pointer needs to be added in nbuf frag
 * @nbuf: qdf_nbuf_t where frag will be added
 * @offset: Offset in frag to be added to nbuf_frags
 * @frag_len: Frag length
 * @truesize: truesize
 * @take_frag_ref: Whether to take ref for frag or not
 *      This bool must be set as per below comdition:
 *      1. False: If this frag is being added in any nbuf
 *              for the first time after allocation.
 *      2. True: If frag is already attached part of any
 *              nbuf.
 *
 * It takes ref_count based on boolean flag take_frag_ref
 */
void __qdf_nbuf_add_rx_frag(__qdf_frag_t buf, __qdf_nbuf_t nbuf,
			    int offset, int frag_len,
			    unsigned int truesize, bool take_frag_ref);

/**
 * __qdf_nbuf_set_mark() - Set nbuf mark
 * @buf: Pointer to nbuf
 * @mark: Value to set mark
 *
 * Return: None
 */
static inline void __qdf_nbuf_set_mark(__qdf_nbuf_t buf, uint32_t mark)
{
	buf->mark = mark;
}

/**
 * __qdf_nbuf_get_mark() - Get nbuf mark
 * @buf: Pointer to nbuf
 *
 * Return: Value of mark
 */
static inline uint32_t __qdf_nbuf_get_mark(__qdf_nbuf_t buf)
{
	return buf->mark;
}

/**
 * __qdf_nbuf_get_data_len() - Return the size of the nbuf from
 * the data pointer to the end pointer
 * @nbuf: qdf_nbuf_t
 *
 * Return: size of skb from data pointer to end pointer
 */
static inline qdf_size_t __qdf_nbuf_get_data_len(__qdf_nbuf_t nbuf)
{
	return (skb_end_pointer(nbuf) - nbuf->data);
}

#ifdef CONFIG_NBUF_AP_PLATFORM
#include <i_qdf_nbuf_w.h>
#else
#include <i_qdf_nbuf_m.h>
#endif
#endif /*_I_QDF_NET_BUF_H */
