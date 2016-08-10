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
 * DOC: qdf_nbuf_public network buffer API
 * This file defines the network buffer abstraction.
 */

#ifndef _QDF_NBUF_H
#define _QDF_NBUF_H

#include <qdf_util.h>
#include <qdf_types.h>
#include <qdf_lock.h>
#include <i_qdf_nbuf.h>
#include <i_qdf_trace.h>
#include <qdf_net_types.h>

#define IPA_NBUF_OWNER_ID			0xaa55aa55
#define QDF_NBUF_PKT_TRAC_TYPE_EAPOL		0x02
#define QDF_NBUF_PKT_TRAC_TYPE_DHCP		0x04
#define QDF_NBUF_PKT_TRAC_TYPE_MGMT_ACTION	0x08
#define QDF_NBUF_PKT_TRAC_TYPE_ARP		0x10
#define QDF_NBUF_PKT_TRAC_MAX_STRING		12
#define QDF_NBUF_PKT_TRAC_PROTO_STRING		4
#define QDF_NBUF_PKT_ERROR			1

#define QDF_NBUF_TRAC_IPV4_OFFSET		14
#define QDF_NBUF_TRAC_IPV4_HEADER_SIZE		20
#define QDF_NBUF_TRAC_DHCP_SRV_PORT		67
#define QDF_NBUF_TRAC_DHCP_CLI_PORT		68
#define QDF_NBUF_TRAC_ETH_TYPE_OFFSET		12
#define QDF_NBUF_TRAC_EAPOL_ETH_TYPE		0x888E
#define QDF_NBUF_TRAC_WAPI_ETH_TYPE		0x88b4
#define QDF_NBUF_TRAC_ARP_ETH_TYPE		0x0806
#define QDF_NBUF_TRAC_IPV4_ETH_TYPE     0x0800
#define QDF_NBUF_TRAC_IPV6_ETH_TYPE     0x86dd
#define QDF_NBUF_DEST_MAC_OFFSET		0
#define QDF_NBUF_SRC_MAC_OFFSET			6
#define QDF_NBUF_TRAC_IPV4_PROTO_TYPE_OFFSET  23
#define QDF_NBUF_TRAC_IPV4_DEST_ADDR_OFFSET   30
#define QDF_NBUF_TRAC_IPV6_PROTO_TYPE_OFFSET  20
#define QDF_NBUF_TRAC_IPV4_ADDR_MCAST_MASK    0xE0000000
#define QDF_NBUF_TRAC_IPV4_ADDR_BCAST_MASK    0xF0000000
#define QDF_NBUF_TRAC_ICMP_TYPE         1
#define QDF_NBUF_TRAC_TCP_TYPE          6
#define QDF_NBUF_TRAC_UDP_TYPE          17
#define QDF_NBUF_TRAC_ICMPV6_TYPE       0x3a

/* EAPOL Related MASK */
#define EAPOL_PACKET_TYPE_OFFSET		15
#define EAPOL_KEY_INFO_OFFSET			19
#define EAPOL_PKT_LEN_OFFSET            16
#define EAPOL_KEY_LEN_OFFSET            21
#define EAPOL_MASK				0x8013
#define EAPOL_M1_BIT_MASK			0x8000
#define EAPOL_M2_BIT_MASK			0x0001
#define EAPOL_M3_BIT_MASK			0x8013
#define EAPOL_M4_BIT_MASK			0x0003

/* Tracked Packet types */
#define QDF_NBUF_TX_PKT_INVALID              0
#define QDF_NBUF_TX_PKT_DATA_TRACK           1
#define QDF_NBUF_TX_PKT_MGMT_TRACK           2

/* Different Packet states */
#define QDF_NBUF_TX_PKT_HDD                  1
#define QDF_NBUF_TX_PKT_TXRX_ENQUEUE         2
#define QDF_NBUF_TX_PKT_TXRX_DEQUEUE         3
#define QDF_NBUF_TX_PKT_TXRX                 4
#define QDF_NBUF_TX_PKT_HTT                  5
#define QDF_NBUF_TX_PKT_HTC                  6
#define QDF_NBUF_TX_PKT_HIF                  7
#define QDF_NBUF_TX_PKT_CE                   8
#define QDF_NBUF_TX_PKT_FREE                 9
#define QDF_NBUF_TX_PKT_STATE_MAX            10

/**
 * struct mon_rx_status - This will have monitor mode rx_status extracted from
 * htt_rx_desc used later to update radiotap information.
 * @tsft: Time Synchronization Function timer
 * @chan_freq: Capture channel frequency
 * @chan_num: Capture channel number
 * @chan_flags: Bitmap of Channel flags, IEEE80211_CHAN_TURBO,
 *              IEEE80211_CHAN_CCK...
 * @vht_flags: VHT flgs, only present for VHT frames.
 * @vht_flag_values1-5: Contains corresponding data for flags field
 * @rate: Rate in terms 500Kbps
 * @rtap_flags: Bit map of available fields in the radiotap
 * @ant_signal_db: Rx packet RSSI
 * @nr_ant: Number of Antennas used for streaming
 * @mcs: MCS index of Rx frame
 * @is_stbc: Is STBC enabled
 * @sgi: Rx frame short guard interval
 * @ldpc: ldpc enabled
 * @beamformed: Is frame beamformed.
 */
struct mon_rx_status {
	uint64_t tsft;
	uint16_t chan_freq;
	uint16_t chan_num;
	uint16_t chan_flags;
	uint16_t vht_flags;
	uint16_t vht_flag_values6;
	uint8_t  rate;
	uint8_t  rtap_flags;
	uint8_t  ant_signal_db;
	uint8_t  nr_ant;
	uint8_t  mcs;
	uint8_t  vht_flag_values1;
	uint8_t  vht_flag_values2;
	uint8_t  vht_flag_values3[4];
	uint8_t  vht_flag_values4;
	uint8_t  vht_flag_values5;
	uint8_t  is_stbc;
	uint8_t  sgi;
	uint8_t  ldpc;
	uint8_t  beamformed;
};

/* DHCP Related Mask */
#define QDF_DHCP_OPTION53			(0x35)
#define QDF_DHCP_OPTION53_LENGTH		(1)
#define QDF_DHCP_OPTION53_OFFSET		(0x11A)
#define QDF_DHCP_OPTION53_LENGTH_OFFSET	(0x11B)
#define QDF_DHCP_OPTION53_STATUS_OFFSET	(0x11C)
#define DHCP_PKT_LEN_OFFSET           16
#define DHCP_TRANSACTION_ID_OFFSET    46
#define QDF_DHCP_DISCOVER			(1)
#define QDF_DHCP_OFFER				(2)
#define QDF_DHCP_REQUEST			(3)
#define QDF_DHCP_DECLINE			(4)
#define QDF_DHCP_ACK				(5)
#define QDF_DHCP_NAK				(6)
#define QDF_DHCP_RELEASE			(7)
#define QDF_DHCP_INFORM				(8)

/* ARP Related Mask */
#define ARP_SUB_TYPE_OFFSET  20
#define ARP_REQUEST			(1)
#define ARP_RESPONSE		(2)

/* IPV4 Related Mask */
#define IPV4_PKT_LEN_OFFSET           16
#define IPV4_TCP_SEQ_NUM_OFFSET       38
#define IPV4_SRC_PORT_OFFSET          34
#define IPV4_DST_PORT_OFFSET          36

/* IPV4 ICMP Related Mask */
#define ICMP_SEQ_NUM_OFFSET           40
#define ICMP_SUBTYPE_OFFSET           34
#define ICMP_REQUEST                  0x08
#define ICMP_RESPONSE                 0x00

/* IPV6 Related Mask */
#define IPV6_PKT_LEN_OFFSET           18
#define IPV6_TCP_SEQ_NUM_OFFSET       58
#define IPV6_SRC_PORT_OFFSET          54
#define IPV6_DST_PORT_OFFSET          56

/* IPV6 ICMPV6 Related Mask */
#define ICMPV6_SEQ_NUM_OFFSET         60
#define ICMPV6_SUBTYPE_OFFSET         54
#define ICMPV6_REQUEST                0x80
#define ICMPV6_RESPONSE               0x81

#define QDF_NBUF_IPA_CHECK_MASK		0x80000000

/**
 * qdf_proto_type - protocol type
 * @QDF_PROTO_TYPE_DHCP - DHCP
 * @QDF_PROTO_TYPE_EAPOL - EAPOL
 * @QDF_PROTO_TYPE_ARP - ARP
 * @QDF_PROTO_TYPE_MGMT - MGMT
 */
enum qdf_proto_type {
	QDF_PROTO_TYPE_DHCP,
	QDF_PROTO_TYPE_EAPOL,
	QDF_PROTO_TYPE_ARP,
	QDF_PROTO_TYPE_MGMT,
	QDF_PROTO_TYPE_MAX
};

/**
 * qdf_proto_subtype - subtype of packet
 * @QDF_PROTO_EAPOL_M1 - EAPOL 1/4
 * @QDF_PROTO_EAPOL_M2 - EAPOL 2/4
 * @QDF_PROTO_EAPOL_M3 - EAPOL 3/4
 * @QDF_PROTO_EAPOL_M4 - EAPOL 4/4
 * @QDF_PROTO_DHCP_DISCOVER - discover
 * @QDF_PROTO_DHCP_REQUEST - request
 * @QDF_PROTO_DHCP_OFFER - offer
 * @QDF_PROTO_DHCP_ACK - ACK
 * @QDF_PROTO_DHCP_NACK - NACK
 * @QDF_PROTO_DHCP_RELEASE - release
 * @QDF_PROTO_DHCP_INFORM - inform
 * @QDF_PROTO_DHCP_DECLINE - decline
 * @QDF_PROTO_ARP_REQ - arp request
 * @QDF_PROTO_ARP_RES - arp response
 * @QDF_PROTO_ICMP_REQ - icmp request
 * @QDF_PROTO_ICMP_RES - icmp response
 * @QDF_PROTO_ICMPV6_REQ - icmpv6 request
 * @QDF_PROTO_ICMPV6_RES - icmpv6 response
 * @QDF_PROTO_IPV4_UDP - ipv4 udp
 * @QDF_PROTO_IPV4_TCP - ipv4 tcp
 * @QDF_PROTO_IPV6_UDP - ipv6 udp
 * @QDF_PROTO_IPV6_TCP - ipv6 tcp
 * @QDF_PROTO_MGMT_ASSOC -assoc
 * @QDF_PROTO_MGMT_DISASSOC - disassoc
 * @QDF_PROTO_MGMT_AUTH - auth
 * @QDF_PROTO_MGMT_DEAUTH - deauth
 */
enum qdf_proto_subtype {
	QDF_PROTO_INVALID,
	QDF_PROTO_EAPOL_M1,
	QDF_PROTO_EAPOL_M2,
	QDF_PROTO_EAPOL_M3,
	QDF_PROTO_EAPOL_M4,
	QDF_PROTO_DHCP_DISCOVER,
	QDF_PROTO_DHCP_REQUEST,
	QDF_PROTO_DHCP_OFFER,
	QDF_PROTO_DHCP_ACK,
	QDF_PROTO_DHCP_NACK,
	QDF_PROTO_DHCP_RELEASE,
	QDF_PROTO_DHCP_INFORM,
	QDF_PROTO_DHCP_DECLINE,
	QDF_PROTO_ARP_REQ,
	QDF_PROTO_ARP_RES,
	QDF_PROTO_ICMP_REQ,
	QDF_PROTO_ICMP_RES,
	QDF_PROTO_ICMPV6_REQ,
	QDF_PROTO_ICMPV6_RES,
	QDF_PROTO_IPV4_UDP,
	QDF_PROTO_IPV4_TCP,
	QDF_PROTO_IPV6_UDP,
	QDF_PROTO_IPV6_TCP,
	QDF_PROTO_MGMT_ASSOC,
	QDF_PROTO_MGMT_DISASSOC,
	QDF_PROTO_MGMT_AUTH,
	QDF_PROTO_MGMT_DEAUTH,
	QDF_PROTO_SUBTYPE_MAX
};

/**
 * @qdf_nbuf_t - Platform indepedent packet abstraction
 */
typedef __qdf_nbuf_t qdf_nbuf_t;

/**
 * @qdf_dma_map_cb_t - Dma map callback prototype
 */
typedef void (*qdf_dma_map_cb_t)(void *arg, qdf_nbuf_t buf,
				 qdf_dma_map_t dmap);

/**
 * @qdf_nbuf_queue_t - Platform independent packet queue abstraction
 */
typedef __qdf_nbuf_queue_t qdf_nbuf_queue_t;

/* BUS/DMA mapping routines */

static inline QDF_STATUS
qdf_nbuf_dmamap_create(qdf_device_t osdev, qdf_dma_map_t *dmap)
{
	return __qdf_nbuf_dmamap_create(osdev, dmap);
}

static inline void
qdf_nbuf_dmamap_destroy(qdf_device_t osdev, qdf_dma_map_t dmap)
{
	__qdf_nbuf_dmamap_destroy(osdev, dmap);
}

static inline void
qdf_nbuf_dmamap_set_cb(qdf_dma_map_t dmap, qdf_dma_map_cb_t cb, void *arg)
{
	__qdf_nbuf_dmamap_set_cb(dmap, cb, arg);
}

static inline void
qdf_nbuf_set_send_complete_flag(qdf_nbuf_t buf, bool flag)
{
	__qdf_nbuf_set_send_complete_flag(buf, flag);
}

static inline QDF_STATUS
qdf_nbuf_map(qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	return __qdf_nbuf_map(osdev, buf, dir);
}

static inline void
qdf_nbuf_unmap(qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	__qdf_nbuf_unmap(osdev, buf, dir);
}

static inline QDF_STATUS
qdf_nbuf_map_nbytes(qdf_device_t osdev, qdf_nbuf_t buf,
		qdf_dma_dir_t dir, int nbytes)
{
	return __qdf_nbuf_map_nbytes(osdev, buf, dir, nbytes);
}

static inline void
qdf_nbuf_unmap_nbytes(qdf_device_t osdev,
	qdf_nbuf_t buf, qdf_dma_dir_t dir, int nbytes)
{
	__qdf_nbuf_unmap_nbytes(osdev, buf, dir, nbytes);
}

#ifndef REMOVE_INIT_DEBUG_CODE
static inline void
qdf_nbuf_sync_for_cpu(qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	__qdf_nbuf_sync_for_cpu(osdev, buf, dir);
}
#endif

static inline QDF_STATUS
qdf_nbuf_map_single(qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	return __qdf_nbuf_map_single(osdev, buf, dir);
}

static inline QDF_STATUS
qdf_nbuf_map_nbytes_single(
	qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir, int nbytes)
{
	return __qdf_nbuf_map_nbytes_single(osdev, buf, dir, nbytes);
}

static inline void
qdf_nbuf_unmap_single(qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	__qdf_nbuf_unmap_single(osdev, buf, dir);
}

static inline void
qdf_nbuf_unmap_nbytes_single(
	qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir, int nbytes)
{
	return __qdf_nbuf_unmap_nbytes_single(osdev, buf, dir, nbytes);
}

static inline int qdf_nbuf_get_num_frags(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_num_frags(buf);
}

/**
 * qdf_nbuf_get_frag_len() - get fragment length
 * @buf: Network buffer
 * @frag_num: Fragment number
 *
 * Return: Fragment length
 */
static inline int qdf_nbuf_get_frag_len(qdf_nbuf_t buf, int frag_num)
{
	QDF_BUG(!(frag_num >= QDF_NBUF_CB_TX_MAX_EXTRA_FRAGS));
	return __qdf_nbuf_get_frag_len(buf, frag_num);
}

/**
 * qdf_nbuf_get_frag_vaddr() - get fragment virtual address
 * @buf: Network buffer
 * @frag_num: Fragment number
 *
 * Return: Fragment virtual address
 */
static inline unsigned char *qdf_nbuf_get_frag_vaddr(qdf_nbuf_t buf,
			int frag_num)
{
	QDF_BUG(!(frag_num >= QDF_NBUF_CB_TX_MAX_EXTRA_FRAGS));
	return __qdf_nbuf_get_frag_vaddr(buf, frag_num);
}

/**
 * qdf_nbuf_get_frag_vaddr_always() - get fragment virtual address
 * @buf: Network buffer
 *
 * Return: Fragment virtual address
 */
static inline unsigned char *
qdf_nbuf_get_frag_vaddr_always(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_frag_vaddr_always(buf);
}

/**
 * qdf_nbuf_get_frag_paddr() - get fragment physical address
 * @buf: Network buffer
 * @frag_num: Fragment number
 *
 * Return: Fragment physical address
 */
static inline qdf_dma_addr_t qdf_nbuf_get_frag_paddr(qdf_nbuf_t buf,
			unsigned int frag_num)
{
	QDF_BUG(!(frag_num >= QDF_NBUF_CB_TX_MAX_EXTRA_FRAGS));
	return __qdf_nbuf_get_frag_paddr(buf, frag_num);
}

/**
 * qdf_nbuf_get_frag_is_wordstream() - is fragment wordstream
 * @buf: Network buffer
 * @frag_num: Fragment number
 *
 * Return: Fragment wordstream or not
 */
static inline int qdf_nbuf_get_frag_is_wordstream(qdf_nbuf_t buf, int frag_num)
{
	QDF_BUG(!(frag_num >= QDF_NBUF_CB_TX_MAX_EXTRA_FRAGS));
	return __qdf_nbuf_get_frag_is_wordstream(buf, frag_num);
}

static inline int qdf_nbuf_ipa_owned_get(qdf_nbuf_t buf)
{
	return __qdf_nbuf_ipa_owned_get(buf);
}

static inline void qdf_nbuf_ipa_owned_set(qdf_nbuf_t buf)
{
	__qdf_nbuf_ipa_owned_set(buf);
}

static inline int qdf_nbuf_ipa_priv_get(qdf_nbuf_t buf)
{
	return __qdf_nbuf_ipa_priv_get(buf);
}

static inline void qdf_nbuf_ipa_priv_set(qdf_nbuf_t buf, uint32_t priv)
{

	QDF_BUG(!(priv & QDF_NBUF_IPA_CHECK_MASK));
	__qdf_nbuf_ipa_priv_set(buf, priv);
}

/**
 * qdf_nbuf_set_frag_is_wordstream() - set fragment wordstream
 * @buf: Network buffer
 * @frag_num: Fragment number
 * @is_wordstream: Wordstream
 *
 * Return: none
 */
static inline void
qdf_nbuf_set_frag_is_wordstream(qdf_nbuf_t buf,
		 int frag_num, int is_wordstream)
{
	QDF_BUG(!(frag_num >= QDF_NBUF_CB_TX_MAX_EXTRA_FRAGS));
	__qdf_nbuf_set_frag_is_wordstream(buf, frag_num, is_wordstream);
}

static inline void
qdf_nbuf_set_vdev_ctx(qdf_nbuf_t buf, void *vdev_ctx)
{
	__qdf_nbuf_set_vdev_ctx(buf, vdev_ctx);
}

static inline void
qdf_nbuf_set_fctx_type(qdf_nbuf_t buf, void *ctx, uint8_t type)
{
	__qdf_nbuf_set_fctx_type(buf, ctx, type);
}

static inline void *
qdf_nbuf_get_vdev_ctx(qdf_nbuf_t buf)
{
	return  __qdf_nbuf_get_vdev_ctx(buf);
}

static inline void *qdf_nbuf_get_fctx(qdf_nbuf_t buf)
{
	return  __qdf_nbuf_get_fctx(buf);
}

static inline uint8_t qdf_nbuf_get_ftype(qdf_nbuf_t buf)
{
	return  __qdf_nbuf_get_ftype(buf);
}

static inline qdf_dma_addr_t
qdf_nbuf_mapped_paddr_get(qdf_nbuf_t buf)
{
	return __qdf_nbuf_mapped_paddr_get(buf);
}

static inline void
qdf_nbuf_mapped_paddr_set(qdf_nbuf_t buf, qdf_dma_addr_t paddr)
{
	__qdf_nbuf_mapped_paddr_set(buf, paddr);
}

static inline void
qdf_nbuf_frag_push_head(qdf_nbuf_t buf,
			int frag_len, char *frag_vaddr,
			qdf_dma_addr_t frag_paddr)
{
	__qdf_nbuf_frag_push_head(buf, frag_len, frag_vaddr, frag_paddr);
}

#define qdf_nbuf_num_frags_init(_nbuf) __qdf_nbuf_num_frags_init((_nbuf))

/* For efficiency, it is the responsibility of the caller to ensure that val
 * is either 0 or 1.
 */
static inline void
qdf_nbuf_set_chfrag_start(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_chfrag_start(buf, val);
}

static inline int qdf_nbuf_is_chfrag_start(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_chfrag_start(buf);
}

/* For efficiency, it is the responsibility of the caller to ensure that val
 * is either 0 or 1.
 */
static inline void qdf_nbuf_set_chfrag_end(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_chfrag_end(buf, val);
}

static inline int qdf_nbuf_is_chfrag_end(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_chfrag_end(buf);
}

static inline void
qdf_nbuf_dma_map_info(qdf_dma_map_t bmap, qdf_dmamap_info_t *sg)
{
	__qdf_nbuf_dma_map_info(bmap, sg);
}

#ifdef MEMORY_DEBUG
void qdf_net_buf_debug_init(void);
void qdf_net_buf_debug_exit(void);
void qdf_net_buf_debug_clean(void);
void qdf_net_buf_debug_add_node(qdf_nbuf_t net_buf, size_t size,
			uint8_t *file_name, uint32_t line_num);
void qdf_net_buf_debug_delete_node(qdf_nbuf_t net_buf);
void qdf_net_buf_debug_release_skb(qdf_nbuf_t net_buf);

/* nbuf allocation rouines */

#define qdf_nbuf_alloc(d, s, r, a, p)			\
	qdf_nbuf_alloc_debug(d, s, r, a, p, __FILE__, __LINE__)
static inline qdf_nbuf_t
qdf_nbuf_alloc_debug(qdf_device_t osdev, qdf_size_t size, int reserve,
		int align, int prio, uint8_t *file_name,
		uint32_t line_num)
{
	qdf_nbuf_t net_buf;
	net_buf = __qdf_nbuf_alloc(osdev, size, reserve, align, prio);

	/* Store SKB in internal QDF tracking table */
	if (qdf_likely(net_buf))
		qdf_net_buf_debug_add_node(net_buf, size, file_name, line_num);

	return net_buf;
}

static inline void qdf_nbuf_free(qdf_nbuf_t net_buf)
{
	/* Remove SKB from internal QDF tracking table */
	if (qdf_likely(net_buf))
		qdf_net_buf_debug_delete_node(net_buf);

	__qdf_nbuf_free(net_buf);
}

#else

static inline void qdf_net_buf_debug_release_skb(qdf_nbuf_t net_buf)
{
	return;
}

/* Nbuf allocation rouines */

static inline qdf_nbuf_t
qdf_nbuf_alloc(qdf_device_t osdev,
		qdf_size_t size, int reserve, int align, int prio)
{
	return __qdf_nbuf_alloc(osdev, size, reserve, align, prio);
}

static inline void qdf_nbuf_free(qdf_nbuf_t buf)
{
	__qdf_nbuf_free(buf);
}

#endif

#ifdef WLAN_FEATURE_FASTPATH
/**
 * qdf_nbuf_init_fast() - before put buf into pool,turn it to init state
 *
 * @buf: buf instance
 * Return: data pointer of this buf where new data has to be
 *         put, or NULL if there is not enough room in this buf.
 */

static inline void qdf_nbuf_init_fast(qdf_nbuf_t nbuf)
{
	atomic_set(&nbuf->users, 1);
	nbuf->data = nbuf->head + NET_SKB_PAD;
	skb_reset_tail_pointer(nbuf);
}
#endif /* WLAN_FEATURE_FASTPATH */

static inline void qdf_nbuf_tx_free(qdf_nbuf_t buf_list, int tx_err)
{
	__qdf_nbuf_tx_free(buf_list, tx_err);
}

static inline void qdf_nbuf_ref(qdf_nbuf_t buf)
{
	__qdf_nbuf_ref(buf);
}

static inline int qdf_nbuf_shared(qdf_nbuf_t buf)
{
	return __qdf_nbuf_shared(buf);
}

static inline qdf_nbuf_t qdf_nbuf_copy(qdf_nbuf_t buf)
{
	return __qdf_nbuf_copy(buf);
}

static inline QDF_STATUS qdf_nbuf_cat(qdf_nbuf_t dst, qdf_nbuf_t src)
{
	return __qdf_nbuf_cat(dst, src);
}

/**
 * @qdf_nbuf_copy_bits() - return the length of the copy bits for skb
 * @skb: SKB pointer
 * @offset: offset
 * @len: Length
 * @to: To
 *
 * Return: int32_t
 */
static inline int32_t
qdf_nbuf_copy_bits(qdf_nbuf_t nbuf, uint32_t offset, uint32_t len, void *to)
{
	return __qdf_nbuf_copy_bits(nbuf, offset, len, to);
}

/**
 * qdf_nbuf_clone() - clone the nbuf (copy is readonly)
 * @buf: nbuf to clone from
 *
 * Return: cloned buffer
 */
static inline qdf_nbuf_t qdf_nbuf_clone(qdf_nbuf_t buf)
{
	return __qdf_nbuf_clone(buf);
}

/* nbuf manipulation routines */

/**
 * @qdf_nbuf_head() - return the address of an nbuf's buffer
 * @buf: netbuf
 *
 * Return: head address
 */
static inline uint8_t *qdf_nbuf_head(qdf_nbuf_t buf)
{
	return __qdf_nbuf_head(buf);
}

/**
 * qdf_nbuf_data() - Return the address of the start of data within an nbuf
 * @buf: Network buffer
 *
 * Return: Data address
 */
static inline uint8_t *qdf_nbuf_data(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data(buf);
}

/**
 * qdf_nbuf_data_addr() - Return the address of skb->data
 * @buf: Network buffer
 *
 * Return: Data address
 */
static inline uint8_t *qdf_nbuf_data_addr(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_addr(buf);
}

/**
 * qdf_nbuf_headroom() - amount of headroom int the current nbuf
 * @buf: Network buffer
 *
 * Return: Amount of head room
 */
static inline uint32_t qdf_nbuf_headroom(qdf_nbuf_t buf)
{
	return __qdf_nbuf_headroom(buf);
}

/**
 * qdf_nbuf_tailroom() - amount of tail space available
 * @buf: Network buffer
 *
 * Return: amount of tail room
 */
static inline uint32_t qdf_nbuf_tailroom(qdf_nbuf_t buf)
{
	return __qdf_nbuf_tailroom(buf);
}

/**
 * qdf_nbuf_push_head() - push data in the front
 * @buf: Network buf instance
 * @size: Size to be pushed
 *
 * Return: New data pointer of this buf after data has been pushed,
 *	or NULL if there is not enough room in this buf.
 */
static inline uint8_t *qdf_nbuf_push_head(qdf_nbuf_t buf, qdf_size_t size)
{
	return __qdf_nbuf_push_head(buf, size);
}

/**
 * qdf_nbuf_put_tail() - puts data in the end
 * @buf: Network buf instance
 * @size: Size to be pushed
 *
 * Return: Data pointer of this buf where new data has to be
 *	put, or NULL if there is not enough room in this buf.
 */
static inline uint8_t *qdf_nbuf_put_tail(qdf_nbuf_t buf, qdf_size_t size)
{
	return __qdf_nbuf_put_tail(buf, size);
}

/**
 * qdf_nbuf_pull_head() - pull data out from the front
 * @buf: Network buf instance
 * @size: Size to be popped
 *
 * Return: New data pointer of this buf after data has been popped,
 *	or NULL if there is not sufficient data to pull.
 */
static inline uint8_t *qdf_nbuf_pull_head(qdf_nbuf_t buf, qdf_size_t size)
{
	return __qdf_nbuf_pull_head(buf, size);
}

/**
 * qdf_nbuf_trim_tail() - trim data out from the end
 * @buf: Network buf instance
 * @size: Size to be popped
 *
 * Return: none
 */
static inline void qdf_nbuf_trim_tail(qdf_nbuf_t buf, qdf_size_t size)
{
	__qdf_nbuf_trim_tail(buf, size);
}

/**
 * qdf_nbuf_len() - get the length of the buf
 * @buf: Network buf instance
 *
 * Return: total length of this buf.
 */
static inline qdf_size_t qdf_nbuf_len(qdf_nbuf_t buf)
{
	return __qdf_nbuf_len(buf);
}

/**
 * qdf_nbuf_set_pktlen() - set the length of the buf
 * @buf: Network buf instance
 * @size: Size to be set
 *
 * Return: none
 */
static inline void qdf_nbuf_set_pktlen(qdf_nbuf_t buf, uint32_t len)
{
	__qdf_nbuf_set_pktlen(buf, len);
}

/**
 * qdf_nbuf_reserve() - trim data out from the end
 * @buf: Network buf instance
 * @size: Size to be popped
 *
 * Return: none
 */
static inline void qdf_nbuf_reserve(qdf_nbuf_t buf, qdf_size_t size)
{
	__qdf_nbuf_reserve(buf, size);
}

/**
 * qdf_nbuf_peek_header() - return the data pointer & length of the header
 * @buf: Network nbuf
 * @addr: Data pointer
 * @len: Length of the data
 *
 * Return: none
 */
static inline void
qdf_nbuf_peek_header(qdf_nbuf_t buf, uint8_t **addr, uint32_t *len)
{
	__qdf_nbuf_peek_header(buf, addr, len);
}

/* nbuf queue routines */

/**
 * qdf_nbuf_queue_init() - initialize buf queue
 * @head: Network buf queue head
 *
 * Return: none
 */
static inline void qdf_nbuf_queue_init(qdf_nbuf_queue_t *head)
{
	__qdf_nbuf_queue_init(head);
}

/**
 * qdf_nbuf_queue_add() - append a nbuf to the tail of the buf queue
 * @head: Network buf queue head
 * @buf: Network buf
 *
 * Return: none
 */
static inline void qdf_nbuf_queue_add(qdf_nbuf_queue_t *head, qdf_nbuf_t buf)
{
	__qdf_nbuf_queue_add(head, buf);
}

/**
 * qdf_nbuf_queue_insert_head() - insert nbuf at the head of queue
 * @head: Network buf queue head
 * @buf: Network buf
 *
 * Return: none
 */
static inline void
qdf_nbuf_queue_insert_head(qdf_nbuf_queue_t *head, qdf_nbuf_t buf)
{
	__qdf_nbuf_queue_insert_head(head, buf);
}

/**
 * qdf_nbuf_queue_remove() - retrieve a buf from the head of the buf queue
 * @head: Network buf queue head
 *
 * Return: The head buf in the buf queue.
 */
static inline qdf_nbuf_t qdf_nbuf_queue_remove(qdf_nbuf_queue_t *head)
{
	return __qdf_nbuf_queue_remove(head);
}

/**
 * qdf_nbuf_queue_len() - get the length of the queue
 * @head: Network buf queue head
 *
 * Return: length of the queue
 */
static inline uint32_t qdf_nbuf_queue_len(qdf_nbuf_queue_t *head)
{
	return __qdf_nbuf_queue_len(head);
}

/**
 * qdf_nbuf_queue_next() - get the next guy/packet of the given buffer
 * @buf: Network buffer
 *
 * Return: next buffer/packet
 */
static inline qdf_nbuf_t qdf_nbuf_queue_next(qdf_nbuf_t buf)
{
	return __qdf_nbuf_queue_next(buf);
}

/**
 * @qdf_nbuf_is_queue_empty() - check if the buf queue is empty
 * @nbq: Network buf queue handle
 *
 * Return: true  if queue is empty
 *	   false if queue is not emty
 */
static inline bool qdf_nbuf_is_queue_empty(qdf_nbuf_queue_t *nbq)
{
	return __qdf_nbuf_is_queue_empty(nbq);
}

static inline qdf_nbuf_queue_t *
qdf_nbuf_queue_append(qdf_nbuf_queue_t *dest, qdf_nbuf_queue_t *src)
{
	return __qdf_nbuf_queue_append(dest, src);
}

static inline void
qdf_nbuf_queue_free(qdf_nbuf_queue_t *head)
{
	__qdf_nbuf_queue_free(head);
}

static inline qdf_nbuf_t
qdf_nbuf_queue_first(qdf_nbuf_queue_t *head)
{
	return __qdf_nbuf_queue_first(head);
}

/**
 * qdf_nbuf_next() - get the next packet in the linked list
 * @buf: Network buffer
 *
 * This function can be used when nbufs are directly linked into a list,
 * rather than using a separate network buffer queue object.
 *
 * Return: next network buffer in the linked list
 */
static inline qdf_nbuf_t qdf_nbuf_next(qdf_nbuf_t buf)
{
	return __qdf_nbuf_next(buf);
}

/**
 * qdf_nbuf_get_protocol() - return the protocol value of the skb
 * @skb: Pointer to network buffer
 *
 * Return: skb protocol
 */
static inline uint16_t qdf_nbuf_get_protocol(struct sk_buff *skb)
{
	return __qdf_nbuf_get_protocol(skb);
}

/**
 * qdf_nbuf_get_ip_summed() - return the ip checksum value of the skb
 * @skb: Pointer to network buffer
 *
 * Return: skb ip_summed
 */
static inline uint8_t qdf_nbuf_get_ip_summed(struct sk_buff *skb)
{
	return __qdf_nbuf_get_ip_summed(skb);
}

/**
 * qdf_nbuf_set_ip_summed() - sets the ip_summed value of the skb
 * @skb: Pointer to network buffer
 * @ip_summed: ip checksum
 *
 * Return: none
 */
static inline void qdf_nbuf_set_ip_summed(struct sk_buff *skb,
	uint8_t ip_summed)
{
	__qdf_nbuf_set_ip_summed(skb, ip_summed);
}

/**
 * qdf_nbuf_set_next() - add a packet to a linked list
 * @this_buf: Predecessor buffer
 * @next_buf: Successor buffer
 *
 * This function can be used to directly link nbufs, rather than using
 * a separate network buffer queue object.
 *
 * Return: none
 */
static inline void qdf_nbuf_set_next(qdf_nbuf_t this_buf, qdf_nbuf_t next_buf)
{
	__qdf_nbuf_set_next(this_buf, next_buf);
}

/* nbuf extension routines */

/**
 * qdf_nbuf_set_next_ext() - link extension of this packet contained in a new
 *			nbuf
 * @this_buf: predecessor buffer
 * @next_buf: successor buffer
 *
 * This function is used to link up many nbufs containing a single logical
 * packet - not a collection of packets. Do not use for linking the first
 * extension to the head
 *
 * Return: none
 */
static inline void
qdf_nbuf_set_next_ext(qdf_nbuf_t this_buf, qdf_nbuf_t next_buf)
{
	__qdf_nbuf_set_next_ext(this_buf, next_buf);
}

/**
 * qdf_nbuf_next_ext() - get the next packet extension in the linked list
 * @buf: Network buffer
 *
 * Return: Next network buffer in the linked list
 */
static inline qdf_nbuf_t qdf_nbuf_next_ext(qdf_nbuf_t buf)
{
	return __qdf_nbuf_next_ext(buf);
}

/**
 * qdf_nbuf_append_ext_list() - link list of packet extensions to the head
 *				segment
 * @head_buf: Network buf holding head segment (single)
 * @ext_list: Network buf list holding linked extensions to the head
 * @ext_len: Total length of all buffers in the extension list
 *
 * This function is used to link up a list of packet extensions (seg1, 2,
 * ...) to the nbuf holding the head segment (seg0)
 *
 * Return: none
 */
static inline void
qdf_nbuf_append_ext_list(qdf_nbuf_t head_buf, qdf_nbuf_t ext_list,
			qdf_size_t ext_len)
{
	__qdf_nbuf_append_ext_list(head_buf, ext_list, ext_len);
}

/**
 * qdf_nbuf_get_ext_list() - Get the link to extended nbuf list.
 * @head_buf: Network buf holding head segment (single)
 *
 * This ext_list is populated when we have Jumbo packet, for example in case of
 * monitor mode amsdu packet reception, and are stiched using frags_list.
 *
 * Return: Network buf list holding linked extensions from head buf.
 */
static inline qdf_nbuf_t qdf_nbuf_get_ext_list(qdf_nbuf_t head_buf)
{
	return (qdf_nbuf_t)__qdf_nbuf_get_ext_list(head_buf);
}

/**
 * qdf_nbuf_get_tx_cksum() - gets the tx checksum offload demand
 * @buf: Network buffer
 *
 * Return: qdf_nbuf_tx_cksum_t checksum offload demand for the frame
 */
static inline qdf_nbuf_tx_cksum_t qdf_nbuf_get_tx_cksum(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_tx_cksum(buf);
}

/**
 * qdf_nbuf_set_rx_cksum() - drivers that support hw checksumming use this to
 *			indicate checksum info to the stack.
 * @buf: Network buffer
 * @cksum: Checksum
 *
 * Return: none
 */
static inline void
qdf_nbuf_set_rx_cksum(qdf_nbuf_t buf, qdf_nbuf_rx_cksum_t *cksum)
{
	__qdf_nbuf_set_rx_cksum(buf, cksum);
}

/**
 * qdf_nbuf_get_tid() - this function extracts the TID value from nbuf
 * @buf: Network buffer
 *
 * Return: TID value
 */
static inline uint8_t qdf_nbuf_get_tid(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_tid(buf);
}

/**
 * qdf_nbuf_set_tid() - this function sets the TID value in nbuf
 * @buf: Network buffer
 * @tid: TID value
 *
 * Return: none
 */
static inline void qdf_nbuf_set_tid(qdf_nbuf_t buf, uint8_t tid)
{
	__qdf_nbuf_set_tid(buf, tid);
}

/**
 * qdf_nbuf_get_exemption_type() - this function extracts the exemption type
 *				from nbuf
 * @buf: Network buffer
 *
 * Return: Exemption type
 */
static inline uint8_t qdf_nbuf_get_exemption_type(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_exemption_type(buf);
}

/**
 * qdf_nbuf_set_protocol() - this function peeks data into the buffer at given
 *			offset
 * @buf: Network buffer
 * @proto: Protocol
 *
 * Return: none
 */
static inline void qdf_nbuf_set_protocol(qdf_nbuf_t buf, uint16_t proto)
{
	__qdf_nbuf_set_protocol(buf, proto);
}

/**
 * qdf_nbuf_trace_get_proto_type() - this function return packet proto type
 * @buf: Network buffer
 *
 * Return: Packet protocol type
 */
static inline uint8_t qdf_nbuf_trace_get_proto_type(qdf_nbuf_t buf)
{
	return __qdf_nbuf_trace_get_proto_type(buf);
}

/**
 * qdf_nbuf_reg_trace_cb() - this function registers protocol trace callback
 * @cb_func_ptr: Callback pointer
 *
 * Return: none
 */
static inline void qdf_nbuf_reg_trace_cb(qdf_nbuf_trace_update_t cb_func_ptr)
{
	__qdf_nbuf_reg_trace_cb(cb_func_ptr);
}


/**
 * qdf_nbuf_set_tx_parallel_dnload_frm() - set tx parallel download
 * @buf: Network buffer
 * @candi: Candidate of parallel download frame
 *
 * This function stores a flag specifying this TX frame is suitable for
 * downloading though a 2nd TX data pipe that is used for short frames for
 * protocols that can accept out-of-order delivery.
 *
 * Return: none
 */
static inline void
qdf_nbuf_set_tx_parallel_dnload_frm(qdf_nbuf_t buf, uint8_t candi)
{
	__qdf_nbuf_set_tx_htt2_frm(buf, candi);
}

/**
 * qdf_nbuf_get_tx_parallel_dnload_frm() - get tx parallel download
 * @buf: Network buffer
 *
 * This function return whether this TX frame is allow to download though a 2nd
 * TX data pipe or not.
 *
 * Return: none
 */
static inline uint8_t qdf_nbuf_get_tx_parallel_dnload_frm(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_tx_htt2_frm(buf);
}

/**
 * qdf_nbuf_get_dhcp_subtype() - get the subtype
 *              of DHCP packet.
 * @buf: Pointer to DHCP packet buffer
 *
 * This func. returns the subtype of DHCP packet.
 *
 * Return: subtype of the DHCP packet.
 */
static inline enum qdf_proto_subtype
qdf_nbuf_get_dhcp_subtype(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_get_dhcp_subtype(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_get_dhcp_subtype() - get the subtype
 *              of DHCP packet.
 * @buf: Pointer to DHCP packet data buffer
 *
 * This func. returns the subtype of DHCP packet.
 *
 * Return: subtype of the DHCP packet.
 */
static inline enum qdf_proto_subtype
qdf_nbuf_data_get_dhcp_subtype(uint8_t *data)
{
	return __qdf_nbuf_data_get_dhcp_subtype(data);
}

/**
 * qdf_nbuf_get_eapol_subtype() - get the subtype
 *            of EAPOL packet.
 * @buf: Pointer to EAPOL packet buffer
 *
 * This func. returns the subtype of EAPOL packet.
 *
 * Return: subtype of the EAPOL packet.
 */
static inline enum qdf_proto_subtype
qdf_nbuf_get_eapol_subtype(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_get_eapol_subtype(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_get_eapol_subtype() - get the subtype
 *            of EAPOL packet.
 * @data: Pointer to EAPOL packet data buffer
 *
 * This func. returns the subtype of EAPOL packet.
 *
 * Return: subtype of the EAPOL packet.
 */
static inline enum qdf_proto_subtype
qdf_nbuf_data_get_eapol_subtype(uint8_t *data)
{
	return __qdf_nbuf_data_get_eapol_subtype(data);
}

/**
 * qdf_nbuf_get_arp_subtype() - get the subtype
 *            of ARP packet.
 * @buf: Pointer to ARP packet buffer
 *
 * This func. returns the subtype of ARP packet.
 *
 * Return: subtype of the ARP packet.
 */
static inline enum qdf_proto_subtype
qdf_nbuf_get_arp_subtype(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_get_arp_subtype(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_get_arp_subtype() - get the subtype
 *            of ARP packet.
 * @data: Pointer to ARP packet data buffer
 *
 * This func. returns the subtype of ARP packet.
 *
 * Return: subtype of the ARP packet.
 */
static inline enum qdf_proto_subtype
qdf_nbuf_data_get_arp_subtype(uint8_t *data)
{
	return __qdf_nbuf_data_get_arp_subtype(data);
}

/**
 * qdf_nbuf_get_icmp_subtype() - get the subtype
 *            of IPV4 ICMP packet.
 * @buf: Pointer to IPV4 ICMP packet buffer
 *
 * This func. returns the subtype of ICMP packet.
 *
 * Return: subtype of the ICMP packet.
 */
static inline enum qdf_proto_subtype
qdf_nbuf_get_icmp_subtype(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_get_icmp_subtype(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_get_icmp_subtype() - get the subtype
 *            of IPV4 ICMP packet.
 * @data: Pointer to IPV4 ICMP packet data buffer
 *
 * This func. returns the subtype of ICMP packet.
 *
 * Return: subtype of the ICMP packet.
 */
static inline enum qdf_proto_subtype
qdf_nbuf_data_get_icmp_subtype(uint8_t *data)
{
	return __qdf_nbuf_data_get_icmp_subtype(data);
}

/**
 * qdf_nbuf_get_icmpv6_subtype() - get the subtype
 *            of IPV6 ICMPV6 packet.
 * @buf: Pointer to IPV6 ICMPV6 packet buffer
 *
 * This func. returns the subtype of ICMPV6 packet.
 *
 * Return: subtype of the ICMPV6 packet.
 */
static inline enum qdf_proto_subtype
qdf_nbuf_get_icmpv6_subtype(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_get_icmpv6_subtype(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_get_icmpv6_subtype() - get the subtype
 *            of IPV6 ICMPV6 packet.
 * @data: Pointer to IPV6 ICMPV6 packet data buffer
 *
 * This func. returns the subtype of ICMPV6 packet.
 *
 * Return: subtype of the ICMPV6 packet.
 */
static inline enum qdf_proto_subtype
qdf_nbuf_data_get_icmpv6_subtype(uint8_t *data)
{
	return __qdf_nbuf_data_get_icmpv6_subtype(data);
}

/**
 * qdf_nbuf_data_get_ipv4_proto() - get the proto type
 *            of IPV4 packet.
 * @data: Pointer to IPV4 packet data buffer
 *
 * This func. returns the proto type of IPV4 packet.
 *
 * Return: proto type of IPV4 packet.
 */
static inline uint8_t
qdf_nbuf_data_get_ipv4_proto(uint8_t *data)
{
	return __qdf_nbuf_data_get_ipv4_proto(data);
}

/**
 * qdf_nbuf_data_get_ipv6_proto() - get the proto type
 *            of IPV6 packet.
 * @data: Pointer to IPV6 packet data buffer
 *
 * This func. returns the proto type of IPV6 packet.
 *
 * Return: proto type of IPV6 packet.
 */
static inline uint8_t
qdf_nbuf_data_get_ipv6_proto(uint8_t *data)
{
	return __qdf_nbuf_data_get_ipv6_proto(data);
}

/**
 * qdf_nbuf_is_ipv4_pkt() - check if packet is a ipv4 packet or not
 * @buf:  buffer
 *
 * This api is for Tx packets.
 *
 * Return: true if packet is ipv4 packet
 */
static inline
bool qdf_nbuf_is_ipv4_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_ipv4_pkt(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_ipv4_pkt() - check if packet is a ipv4 packet or not
 * @data:  data
 *
 * This api is for Tx packets.
 *
 * Return: true if packet is ipv4 packet
 */
static inline
bool qdf_nbuf_data_is_ipv4_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_ipv4_pkt(data);
}

/**
 * qdf_nbuf_is_ipv4_dhcp_pkt() - check if packet is a dhcp packet or not
 * @buf:  buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: true if packet is DHCP packet
 */
static inline
bool qdf_nbuf_is_ipv4_dhcp_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_ipv4_dhcp_pkt(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_ipv4_dhcp_pkt() - check if it is DHCP packet.
 * @data: Pointer to DHCP packet data buffer
 *
 * This func. checks whether it is a DHCP packet or not.
 *
 * Return: true if it is a DHCP packet
 *         false if not
 */
static inline
bool qdf_nbuf_data_is_ipv4_dhcp_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_ipv4_dhcp_pkt(data);
}

/**
 * qdf_nbuf_is_ipv4_eapol_pkt() - check if packet is a eapol packet or not
 * @buf:  buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: true if packet is EAPOL packet
 */
static inline
bool qdf_nbuf_is_ipv4_eapol_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_ipv4_eapol_pkt(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_ipv4_eapol_pkt() - check if it is EAPOL packet.
 * @data: Pointer to EAPOL packet data buffer
 *
 * This func. checks whether it is a EAPOL packet or not.
 *
 * Return: true if it is a EAPOL packet
 *         false if not
 */
static inline
bool qdf_nbuf_data_is_ipv4_eapol_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_ipv4_eapol_pkt(data);
}

/**
 * qdf_nbuf_is_ipv4_wapi_pkt() - check if packet is a wapi packet or not
 * @buf:  buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: true if packet is WAPI packet
 */
static inline
bool qdf_nbuf_is_ipv4_wapi_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_ipv4_wapi_pkt(buf);
}

/**
 * qdf_nbuf_is_ipv4_arp_pkt() - check if packet is a arp packet or not
 * @buf:  buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: true if packet is ARP packet
 */
static inline
bool qdf_nbuf_is_ipv4_arp_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_ipv4_arp_pkt(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_ipv4_arp_pkt() - check if it is ARP packet.
 * @data: Pointer to ARP packet data buffer
 *
 * This func. checks whether it is a ARP packet or not.
 *
 * Return: TRUE if it is a ARP packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_data_is_ipv4_arp_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_ipv4_arp_pkt(data);
}

/**
 * qdf_nbuf_is_ipv6_pkt() - check if it is IPV6 packet.
 * @buf: Pointer to IPV6 packet buffer
 *
 * This func. checks whether it is a IPV6 packet or not.
 *
 * Return: TRUE if it is a IPV6 packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_is_ipv6_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_ipv6_pkt(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_ipv6_pkt() - check if it is IPV6 packet.
 * @data: Pointer to IPV6 packet data buffer
 *
 * This func. checks whether it is a IPV6 packet or not.
 *
 * Return: TRUE if it is a IPV6 packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_data_is_ipv6_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_ipv6_pkt(data);
}

/**
 * qdf_nbuf_data_is_ipv4_mcast_pkt() - check if it is IPV4 multicast packet.
 * @data: Pointer to IPV4 packet data buffer
 *
 * This func. checks whether it is a IPV4 multicast packet or not.
 *
 * Return: TRUE if it is a IPV4 multicast packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_data_is_ipv4_mcast_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_ipv4_mcast_pkt(data);
}

/**
 * qdf_nbuf_is_icmp_pkt() - check if it is IPV4 ICMP packet.
 * @buf: Pointer to IPV4 ICMP packet buffer
 *
 * This func. checks whether it is a ICMP packet or not.
 *
 * Return: TRUE if it is a ICMP packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_is_icmp_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_icmp_pkt(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_icmp_pkt() - check if it is IPV4 ICMP packet.
 * @data: Pointer to IPV4 ICMP packet data buffer
 *
 * This func. checks whether it is a ICMP packet or not.
 *
 * Return: TRUE if it is a ICMP packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_data_is_icmp_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_icmp_pkt(data);
}

/**
 * qdf_nbuf_is_icmpv6_pkt() - check if it is IPV6 ICMPV6 packet.
 * @buf: Pointer to IPV6 ICMPV6 packet buffer
 *
 * This func. checks whether it is a ICMPV6 packet or not.
 *
 * Return: TRUE if it is a ICMPV6 packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_is_icmpv6_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_icmpv6_pkt(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_icmpv6_pkt() - check if it is IPV6 ICMPV6 packet.
 * @data: Pointer to IPV6 ICMPV6 packet data buffer
 *
 * This func. checks whether it is a ICMPV6 packet or not.
 *
 * Return: TRUE if it is a ICMPV6 packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_data_is_icmpv6_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_icmpv6_pkt(data);
}

/**
 * qdf_nbuf_is_ipv4_udp_pkt() - check if it is IPV4 UDP packet.
 * @buf: Pointer to IPV4 UDP packet buffer
 *
 * This func. checks whether it is a IPV4 UDP packet or not.
 *
 * Return: TRUE if it is a IPV4 UDP packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_is_ipv4_udp_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_ipv4_udp_pkt(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_ipv4_udp_pkt() - check if it is IPV4 UDP packet.
 * @data: Pointer to IPV4 UDP packet data buffer
 *
 * This func. checks whether it is a IPV4 UDP packet or not.
 *
 * Return: TRUE if it is a IPV4 UDP packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_data_is_ipv4_udp_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_ipv4_udp_pkt(data);
}

/**
 * qdf_nbuf_is_ipv4_tcp_pkt() - check if it is IPV4 TCP packet.
 * @buf: Pointer to IPV4 TCP packet buffer
 *
 * This func. checks whether it is a IPV4 TCP packet or not.
 *
 * Return: TRUE if it is a IPV4 TCP packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_is_ipv4_tcp_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_ipv4_tcp_pkt(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_ipv4_tcp_pkt() - check if it is IPV4 TCP packet.
 * @data: Pointer to IPV4 TCP packet data buffer
 *
 * This func. checks whether it is a IPV4 TCP packet or not.
 *
 * Return: TRUE if it is a IPV4 TCP packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_data_is_ipv4_tcp_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_ipv4_tcp_pkt(data);
}

/**
 * qdf_nbuf_is_ipv6_udp_pkt() - check if it is IPV6 UDP packet.
 * @buf: Pointer to IPV6 UDP packet buffer
 *
 * This func. checks whether it is a IPV6 UDP packet or not.
 *
 * Return: TRUE if it is a IPV6 UDP packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_is_ipv6_udp_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_ipv6_udp_pkt(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_ipv6_udp_pkt() - check if it is IPV6 UDP packet.
 * @data: Pointer to IPV6 UDP packet data buffer
 *
 * This func. checks whether it is a IPV6 UDP packet or not.
 *
 * Return: TRUE if it is a IPV6 UDP packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_data_is_ipv6_udp_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_ipv6_udp_pkt(data);
}

/**
 * qdf_nbuf_is_ipv6_tcp_pkt() - check if it is IPV6 TCP packet.
 * @buf: Pointer to IPV6 TCP packet buffer
 *
 * This func. checks whether it is a IPV6 TCP packet or not.
 *
 * Return: TRUE if it is a IPV6 TCP packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_is_ipv6_tcp_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_ipv6_tcp_pkt(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_ipv6_tcp_pkt() - check if it is IPV6 TCP packet.
 * @data: Pointer to IPV6 TCP packet data buffer
 *
 * This func. checks whether it is a IPV6 TCP packet or not.
 *
 * Return: TRUE if it is a IPV6 TCP packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_data_is_ipv6_tcp_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_ipv6_tcp_pkt(data);
}

/**
 * qdf_invalidate_range() - invalidate virtual address range
 * @start: start address of the address range
 * @end: end address of the address range
 *
 * Note that this function does not write back the cache entries.
 *
 * Return: none
 */
static inline void qdf_invalidate_range(void *start, void *end)
{
	__qdf_invalidate_range(start, end);
}

/**
 * qdf_nbuf_reset_num_frags() - decrement the number of fragments
 * @buf: Network buffer
 *
 * Return: Number of fragments
 */
static inline void qdf_nbuf_reset_num_frags(qdf_nbuf_t buf)
{
	__qdf_nbuf_reset_num_frags(buf);
}

/**
 * qdf_dmaaddr_to_32s - return high and low parts of dma_addr
 *
 * Returns the high and low 32-bits of the DMA addr in the provided ptrs
 *
 * Return: N/A
 */
static inline void qdf_dmaaddr_to_32s(qdf_dma_addr_t dmaaddr,
				      uint32_t *lo, uint32_t *hi)
{
	return __qdf_dmaaddr_to_32s(dmaaddr, lo, hi);
}

/**
 * qdf_nbuf_is_tso() - is the network buffer a jumbo packet?
 * @buf: Network buffer
 *
 * Return: 1 - this is a jumbo packet 0 - not a jumbo packet
 */
static inline uint8_t qdf_nbuf_is_tso(qdf_nbuf_t nbuf)
{
	return __qdf_nbuf_is_tso(nbuf);
}

/**
 * qdf_nbuf_get_tso_info() - function to divide a jumbo TSO
 * network buffer into segments
 * @nbuf:   network buffer to be segmented
 * @tso_info:  This is the output. The information about the
 *      TSO segments will be populated within this.
 *
 * This function fragments a TCP jumbo packet into smaller
 * segments to be transmitted by the driver. It chains the TSO
 * segments created into a list.
 *
 * Return: number of TSO segments
 */
static inline uint32_t qdf_nbuf_get_tso_info(qdf_device_t osdev,
		 qdf_nbuf_t nbuf, struct qdf_tso_info_t *tso_info)
{
	return __qdf_nbuf_get_tso_info(osdev, nbuf, tso_info);
}

/**
 * qdf_nbuf_get_tso_num_seg() - function to calculate the number
 * of TCP segments within the TSO jumbo packet
 * @nbuf:   TSO jumbo network buffer to be segmented
 *
 * This function calculates the number of TCP segments that the
   network buffer can be divided into.
 *
 * Return: number of TCP segments
 */
static inline uint32_t qdf_nbuf_get_tso_num_seg(qdf_nbuf_t nbuf)
{
	return __qdf_nbuf_get_tso_num_seg(nbuf);
}

/**
 * qdf_nbuf_inc_users() - function to increment the number of
 * users referencing this network buffer
 *
 * @nbuf:   network buffer
 *
 * This function increments the number of users referencing this
 * network buffer
 *
 * Return: the network buffer
 */
static inline qdf_nbuf_t qdf_nbuf_inc_users(qdf_nbuf_t nbuf)
{
	return __qdf_nbuf_inc_users(nbuf);
}

/**
 * qdf_nbuf_data_attr_get() - Get data_attr field from cvg_nbuf_cb
 *
 * @nbuf: Network buffer (skb on linux)
 *
 * This function returns the values of data_attr field
 * in struct cvg_nbuf_cb{}, to which skb->cb is typecast.
 * This value is actually the value programmed in CE descriptor.
 *
 * Return: Value of data_attr
 */
static inline uint32_t qdf_nbuf_data_attr_get(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_attr_get(buf);
}

/**
 * qdf_nbuf_data_attr_set() - Sets data_attr field in cvg_nbuf_cb
 *
 * @nbuf: Network buffer (skb on linux)
 * @data_attr: Value to be stored cvg_nbuf_cb->data_attr
 *
 * This function stores the value to be programmed in CE
 * descriptor as part skb->cb which is typecast to struct cvg_nbuf_cb{}
 *
 * Return: void
 */
static inline
void qdf_nbuf_data_attr_set(qdf_nbuf_t buf, uint32_t data_attr)
{
	__qdf_nbuf_data_attr_set(buf, data_attr);
}

/**
 * qdf_nbuf_tx_info_get() - Parse skb and get Tx metadata
 *
 * @nbuf: Network buffer (skb on linux)
 *
 * This function parses the payload to figure out relevant
 * Tx meta-data e.g. whether to enable tx_classify bit
 * in CE.
 *
 * Return:     void
 */
#define qdf_nbuf_tx_info_get __qdf_nbuf_tx_info_get

void qdf_nbuf_set_state(qdf_nbuf_t nbuf, uint8_t current_state);
void qdf_nbuf_tx_desc_count_display(void);
void qdf_nbuf_tx_desc_count_clear(void);

static inline qdf_nbuf_t
qdf_nbuf_realloc_headroom(qdf_nbuf_t buf, uint32_t headroom)
{
	return __qdf_nbuf_realloc_headroom(buf, headroom);
}

static inline qdf_nbuf_t
qdf_nbuf_realloc_tailroom(qdf_nbuf_t buf, uint32_t tailroom)
{
	return __qdf_nbuf_realloc_tailroom(buf, tailroom);
}

static inline qdf_nbuf_t
qdf_nbuf_expand(qdf_nbuf_t buf, uint32_t headroom, uint32_t tailroom)
{
	return __qdf_nbuf_expand(buf, headroom, tailroom);
}

static inline qdf_nbuf_t
qdf_nbuf_unshare(qdf_nbuf_t buf)
{
	return __qdf_nbuf_unshare(buf);
}

static inline bool
qdf_nbuf_is_cloned(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_cloned(buf);
}

static inline void
qdf_nbuf_frag_info(qdf_nbuf_t buf, qdf_sglist_t *sg)
{
	__qdf_nbuf_frag_info(buf, sg);
}

static inline qdf_nbuf_tx_cksum_t
qdf_nbuf_tx_cksum_info(qdf_nbuf_t buf, uint8_t **hdr_off, uint8_t **where)
{
	return __qdf_nbuf_tx_cksum_info(buf, hdr_off, where);
}

static inline void qdf_nbuf_reset_ctxt(__qdf_nbuf_t nbuf)
{
	__qdf_nbuf_reset_ctxt(nbuf);
}

static inline void
qdf_nbuf_set_rx_info(__qdf_nbuf_t nbuf, void *info, uint32_t len)
{
	__qdf_nbuf_set_rx_info(nbuf, info, len);
}

static inline void *qdf_nbuf_get_rx_info(__qdf_nbuf_t nbuf)
{
	return __qdf_nbuf_get_rx_info(nbuf);
}

static inline void qdf_nbuf_init(qdf_nbuf_t buf)
{
	__qdf_nbuf_init(buf);
}

static inline void *qdf_nbuf_network_header(qdf_nbuf_t buf)
{
	return __qdf_nbuf_network_header(buf);
}

static inline void *qdf_nbuf_transport_header(qdf_nbuf_t buf)
{
	return __qdf_nbuf_transport_header(buf);
}

static inline qdf_size_t qdf_nbuf_tcp_tso_size(qdf_nbuf_t buf)
{
	return __qdf_nbuf_tcp_tso_size(buf);
}

static inline void *qdf_nbuf_get_cb(qdf_nbuf_t nbuf)
{
	return __qdf_nbuf_get_cb(nbuf);
}

static inline uint32_t qdf_nbuf_get_nr_frags(qdf_nbuf_t nbuf)
{
	return __qdf_nbuf_get_nr_frags(nbuf);
}

static inline qdf_size_t qdf_nbuf_headlen(qdf_nbuf_t buf)
{
	return __qdf_nbuf_headlen(buf);
}

static inline QDF_STATUS qdf_nbuf_frag_map(qdf_device_t osdev,
	qdf_nbuf_t buf, int offset,
	qdf_dma_dir_t dir, int cur_frag)
{
	return __qdf_nbuf_frag_map(osdev, buf, offset, dir, cur_frag);
}

static inline bool qdf_nbuf_tso_tcp_v4(qdf_nbuf_t buf)
{
	return __qdf_nbuf_tso_tcp_v4(buf);
}

static inline bool qdf_nbuf_tso_tcp_v6(qdf_nbuf_t buf)
{
	return __qdf_nbuf_tso_tcp_v6(buf);
}

static inline uint32_t qdf_nbuf_tcp_seq(qdf_nbuf_t buf)
{
	return __qdf_nbuf_tcp_seq(buf);
}

static inline qdf_size_t qdf_nbuf_l2l3l4_hdr_len(qdf_nbuf_t buf)
{
	return __qdf_nbuf_l2l3l4_hdr_len(buf);
}

static inline bool qdf_nbuf_is_nonlinear(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_nonlinear(buf);
}

static inline uint32_t
qdf_nbuf_get_frag_size(qdf_nbuf_t buf, uint32_t frag_num)
{
	return __qdf_nbuf_get_frag_size(buf, frag_num);
}

static inline uint32_t qdf_nbuf_get_priority(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_priority(buf);
}

static inline void qdf_nbuf_set_priority(qdf_nbuf_t buf, uint32_t p)
{
	__qdf_nbuf_set_priority(buf, p);
}

static inline uint8_t *
qdf_nbuf_get_priv_ptr(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_priv_ptr(buf);
}

/**
 * qdf_nbuf_update_radiotap() - update radiotap at head of nbuf.
 * @rx_status: rx_status containing required info to update radiotap
 * @nbuf: Pointer to nbuf
 * @headroom_sz: Available headroom size
 *
 * Return: radiotap length.
 */
unsigned int qdf_nbuf_update_radiotap(struct mon_rx_status *rx_status,
				      qdf_nbuf_t nbuf, uint32_t headroom_sz);
#endif /* _QDF_NBUF_H */
