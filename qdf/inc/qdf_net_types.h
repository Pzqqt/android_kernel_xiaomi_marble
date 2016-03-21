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
 * DOC: qdf_net_types
 * This file defines types used in the networking stack abstraction.
 */

#ifndef _QDF_NET_TYPES_H
#define _QDF_NET_TYPES_H

#include <qdf_types.h>          /* uint8_t, etc. */
#include <i_qdf_net_types.h>

/* Extended Traffic ID  passed to target if the TID is unknown */
#define QDF_NBUF_TX_EXT_TID_INVALID     0x1f

/**
 * qdf_nbuf_exemption_type - QDF net buf exemption types for encryption
 * @QDF_NBUF_EXEMPT_NO_EXEMPTION: No exemption
 * @QDF_NBUF_EXEMPT_ALWAYS: Exempt always
 * @QDF_NBUF_EXEMPT_ON_KEY_MAPPING_KEY_UNAVAILABLE: Exempt on key mapping
 */
enum qdf_nbuf_exemption_type {
	QDF_NBUF_EXEMPT_NO_EXEMPTION = 0,
	QDF_NBUF_EXEMPT_ALWAYS,
	QDF_NBUF_EXEMPT_ON_KEY_MAPPING_KEY_UNAVAILABLE
};

typedef __wsum_t wsum_t;
typedef __in6_addr_t in6_addr_t;


#define QDF_NET_MAC_ADDR_MAX_LEN 6
#define QDF_NET_IF_NAME_SIZE     64
#define QDF_NET_ETH_LEN          QDF_NET_MAC_ADDR_MAX_LEN
#define QDF_NET_MAX_MCAST_ADDR   64

/* Extended Traffic ID  passed to target if the TID is unknown */
#define QDF_NBUF_TX_EXT_TID_INVALID    0x1f

#define QDF_ETH_TYPE_IPV4              0x0800  /* IPV4 */
#define QDF_ETH_TYPE_IPV6              0x86dd  /* IPV6 */

typedef struct qdf_net_ethaddr {
	uint8_t addr[QDF_NET_ETH_LEN];
} qdf_net_ethaddr_t;


typedef struct {
	uint16_t  source;
	uint16_t  dest;
	uint32_t  seq;
	uint32_t  ack_seq;
#if defined(QDF_LITTLE_ENDIAN_MACHINE)
	uint16_t   res1:4,
		   doff:4,
		   fin:1,
		   syn:1,
		   rst:1,
		   psh:1,
		   ack:1,
		   urg:1,
		   ece:1,
		   cwr:1;
#elif defined(QDF_BIG_ENDIAN_MACHINE)
	uint16_t   doff:4,
		   res1:4,
		   cwr:1,
		   ece:1,
		   urg:1,
		   ack:1,
		   psh:1,
		   rst:1,
		   syn:1,
		   fin:1;
#else
#error  "Adjust your byte order"
#endif
	uint16_t  window;
	uint16_t  check;
	uint16_t  urg_ptr;
} qdf_net_tcphdr_t;

typedef struct {
#if defined(QDF_LITTLE_ENDIAN_MACHINE)
	uint8_t	ip_hl:4,
		ip_version:4;
#elif defined(QDF_BIG_ENDIAN_MACHINE)
	uint8_t	ip_version:4,
		ip_hl:4;
#else
#error  "Please fix"
#endif
	uint8_t       ip_tos;
	uint16_t      ip_len;
	uint16_t      ip_id;
	uint16_t      ip_frag_off;
	uint8_t       ip_ttl;
	uint8_t       ip_proto;
	uint16_t      ip_check;
	uint32_t      ip_saddr;
	uint32_t      ip_daddr;
    /*The options start here. */
} qdf_net_iphdr_t;

/* V3 group record types [grec_type] */
#define IGMPV3_MODE_IS_INCLUDE     1
#define IGMPV3_MODE_IS_EXCLUDE     2
#define IGMPV3_CHANGE_TO_INCLUDE   3
#define IGMPV3_CHANGE_TO_EXCLUDE   4
#define IGMPV3_ALLOW_NEW_SOURCES   5
#define IGMPV3_BLOCK_OLD_SOURCES   6

/**
 * qdf_net_cmd_vid_t - Command for set/unset vid
 */
typedef uint16_t qdf_net_cmd_vid_t ;        /*get/set vlan id*/

/**
 * typedef qdf_nbuf_tx_cksum_t - transmit checksum offload types
 * @QDF_NBUF_TX_CKSUM_NONE: No checksum offload
 * @QDF_NBUF_TX_CKSUM_IP: IP header checksum offload
 * @QDF_NBUF_TX_CKSUM_TCP_UDP: TCP/UDP checksum offload
 * @QDF_NBUF_TX_CKSUM_TCP_UDP_IP: TCP/UDP and IP header checksum offload
 */

typedef enum {
	QDF_NBUF_TX_CKSUM_NONE,
	QDF_NBUF_TX_CKSUM_IP,
	QDF_NBUF_TX_CKSUM_TCP_UDP,
	QDF_NBUF_TX_CKSUM_TCP_UDP_IP,

} qdf_nbuf_tx_cksum_t;

/**
 * typedef qdf_nbuf_l4_rx_cksum_type_t - receive checksum API types
 * @QDF_NBUF_RX_CKSUM_TCP: Rx checksum TCP
 * @QDF_NBUF_RX_CKSUM_UDP: Rx checksum UDP
 * @QDF_NBUF_RX_CKSUM_TCPIPV6: Rx checksum TCP IPV6
 * @QDF_NBUF_RX_CKSUM_UDPIPV6: Rx checksum UDP IPV6
 * @QDF_NBUF_RX_CKSUM_TCP_NOPSEUDOHEADER: Rx checksum TCP no pseudo header
 * @QDF_NBUF_RX_CKSUM_UDP_NOPSEUDOHEADER: Rx checksum UDP no pseudo header
 * @QDF_NBUF_RX_CKSUM_TCPSUM16: Rx checksum TCP SUM16
 */
typedef enum {
	QDF_NBUF_RX_CKSUM_TCP = 0x0001,
	QDF_NBUF_RX_CKSUM_UDP = 0x0002,
	QDF_NBUF_RX_CKSUM_TCPIPV6 = 0x0010,
	QDF_NBUF_RX_CKSUM_UDPIPV6 = 0x0020,
	QDF_NBUF_RX_CKSUM_TCP_NOPSEUDOHEADER = 0x0100,
	QDF_NBUF_RX_CKSUM_UDP_NOPSEUDOHEADER = 0x0200,
	QDF_NBUF_RX_CKSUM_TCPSUM16 = 0x1000,
} qdf_nbuf_l4_rx_cksum_type_t;

/**
 * typedef qdf_nbuf_l4_rx_cksum_result_t - receive checksum status types
 * @QDF_NBUF_RX_CKSUM_NONE: Device failed to checksum
 * @QDF_NBUF_RX_CKSUM_TCP_UDP_HW: TCP/UDP cksum successful and value returned
 * @QDF_NBUF_RX_CKSUM_TCP_UDP_UNNECESSARY: TCP/UDP cksum successful, no value
 */
typedef enum {
	QDF_NBUF_RX_CKSUM_NONE = 0x0000,
	QDF_NBUF_RX_CKSUM_TCP_UDP_HW = 0x0010,
	QDF_NBUF_RX_CKSUM_TCP_UDP_UNNECESSARY = 0x0020,
} qdf_nbuf_l4_rx_cksum_result_t;

/**
 * typedef qdf_nbuf_rx_cksum_t - receive checksum type
 * @l4_type: L4 type
 * @l4_result: L4 result
 */
typedef struct {
	qdf_nbuf_l4_rx_cksum_type_t l4_type;
	qdf_nbuf_l4_rx_cksum_result_t l4_result;
	uint32_t val;
} qdf_nbuf_rx_cksum_t;

#define QDF_ARP_REQ       1 /* ARP request */
#define QDF_ARP_RSP       2 /* ARP response */
#define QDF_ARP_RREQ      3 /* RARP request */
#define QDF_ARP_RRSP      4 /* RARP response */

#define QDF_NEXTHDR_ICMP  58 /* ICMP for IPv6. */

/* Neighbor Discovery */
#define QDF_ND_RSOL       133 /* Router Solicitation */
#define QDF_ND_RADVT      134 /* Router Advertisement */
#define QDF_ND_NSOL       135 /* Neighbor Solicitation */
#define QDF_ND_NADVT      136 /* Neighbor Advertisement */

/**
 * typedef qdf_net_udphdr_t - UDP header info
 * @src_port: source port
 * @dst_port: destination port
 * @udp_len: length
 * @udp_cksum: checksum
 */
typedef struct {
	uint16_t src_port;
	uint16_t dst_port;
	uint16_t udp_len;
	uint16_t udp_cksum;
} qdf_net_udphdr_t;

/**
 * typedef qdf_net_dhcphdr_t - DHCP header info
 * @dhcp_msg_type: message type
 * @dhcp_hw_type: hardware type
 * @dhcp_hw_addr_len: hardware address length
 * @dhcp_num_hops: number of hops
 * @dhcp_transc_id: transaction id
 * @dhcp_secs_elapsed: time elapsed
 * @dhcp_flags: flags
 * @dhcp_ciaddr: client IP
 * @dhcp_yiaddr: device IP
 * @dhcp_siaddr_nip: Server IP
 * @dhcp_gateway_nip: relay agent IP
 * @dhcp_chaddr: LLC hardware address
 * @dhcp_sname: server host name
 * @dhcp_file: boot file name
 * @dhcp_cookie: cookie
 */
typedef struct {
	uint8_t  dhcp_msg_type;
	uint8_t  dhcp_hw_type;
	uint8_t  dhcp_hw_addr_len;
	uint8_t  dhcp_num_hops;
	uint32_t dhcp_transc_id;
	uint16_t dhcp_secs_elapsed;
	uint16_t dhcp_flags;
	uint32_t dhcp_ciaddr;
	uint32_t dhcp_yiaddr;
	uint32_t dhcp_siaddr_nip;
	uint32_t dhcp_gateway_nip;
	uint8_t  dhcp_chaddr[16];
	uint8_t  dhcp_sname[64];
	uint8_t  dhcp_file[128];
	uint8_t  dhcp_cookie[4];
} qdf_net_dhcphdr_t;


/**
 * qdf_net_vlanhdr_t - Vlan header
 */
typedef struct qdf_net_vlanhdr {
	uint16_t tpid;
#if defined(QDF_LITTLE_ENDIAN_MACHINE)
	uint16_t vid:12; /* Vlan id*/
	uint8_t  cfi:1; /* reserved for CFI, don't use*/
	uint8_t  prio:3; /* Priority*/
#elif defined(QDF_BIG_ENDIAN_MACHINE)
	uint8_t  prio:3; /* Priority*/
	uint8_t  cfi:1; /* reserved for CFI, don't use*/
	uint16_t vid:12; /* Vlan id*/
#else
#error  "Please fix"
#endif
} qdf_net_vlanhdr_t;

typedef struct qdf_net_vid {
#if defined(QDF_LITTLE_ENDIAN_MACHINE)
	uint16_t val:12;
	uint8_t  res:4;
#elif defined(QDF_BIG_ENDIAN_MACHINE)
	uint8_t  res:4;
	uint16_t val:12;
#else
#error  "Please fix"
#endif
} qdf_net_vid_t;

typedef enum {
	QDF_NET_TSO_NONE,
	QDF_NET_TSO_IPV4,     /**< for tsp ipv4 only*/
	QDF_NET_TSO_ALL,      /**< ip4 & ipv6*/
} qdf_net_tso_type_t;

/**
 * qdf_net_dev_info_t - Basic device info
 */
typedef struct {
	uint8_t  if_name[QDF_NET_IF_NAME_SIZE];
	uint8_t  dev_addr[QDF_NET_MAC_ADDR_MAX_LEN];
	uint16_t header_len;
	uint16_t mtu_size;
	uint32_t unit;
} qdf_net_dev_info_t;

/**
 * qdf_nbuf_tso_t - For TCP large Segment Offload
 */
typedef struct {
	qdf_net_tso_type_t  type;
	uint16_t mss;
	uint8_t  hdr_off;
} qdf_nbuf_tso_t;

/**
 * qdf_net_wireless_event_t - Wireless events
 * QDF_IEEE80211_ASSOC = station associate (bss mode)
 * QDF_IEEE80211_REASSOC = station re-associate (bss mode)
 * QDF_IEEE80211_DISASSOC = station disassociate (bss mode)
 * QDF_IEEE80211_JOIN = station join (ap mode)
 * QDF_IEEE80211_LEAVE = station leave (ap mode)
 * QDF_IEEE80211_SCAN = scan complete, results available
 * QDF_IEEE80211_REPLAY = sequence counter replay detected
 * QDF_IEEE80211_MICHAEL = Michael MIC failure detected
 * QDF_IEEE80211_REJOIN = station re-associate (ap mode)
 * QDF_CUSTOM_PUSH_BUTTON = WPS push button
 */
typedef enum qdf_net_wireless_events {
	QDF_IEEE80211_ASSOC = __QDF_IEEE80211_ASSOC,
	QDF_IEEE80211_REASSOC = __QDF_IEEE80211_REASSOC,
	QDF_IEEE80211_DISASSOC = __QDF_IEEE80211_DISASSOC,
	QDF_IEEE80211_JOIN = __QDF_IEEE80211_JOIN,
	QDF_IEEE80211_LEAVE = __QDF_IEEE80211_LEAVE,
	QDF_IEEE80211_SCAN = __QDF_IEEE80211_SCAN,
	QDF_IEEE80211_REPLAY = __QDF_IEEE80211_REPLAY,
	QDF_IEEE80211_MICHAEL = __QDF_IEEE80211_MICHAEL,
	QDF_IEEE80211_REJOIN = __QDF_IEEE80211_REJOIN,
	QDF_CUSTOM_PUSH_BUTTON = __QDF_CUSTOM_PUSH_BUTTON
} qdf_net_wireless_event_t;

/**
 * qdf_net_ipv6_addr_t - IPv6 Address
 */
typedef struct {
	union {
		uint8_t  u6_addr8[16];
		uint16_t u6_addr16[8];
		uint32_t u6_addr32[4];
	} in6_u;
#define s6_addr32   in6_u.u6_addr32
} qdf_net_ipv6_addr_t;

/**
 * qdf_net_ipv6hdr_t - IPv6 Header
 */
typedef struct {
#if defined(QDF_LITTLE_ENDIAN_MACHINE)
	uint8_t ipv6_priority:4,
		ipv6_version:4;
#elif defined(QDF_BIG_ENDIAN_MACHINE)
	uint8_t ipv6_version:4,
		ipv6_priority:4;
#else
#error  "Please fix"
#endif
	uint8_t ipv6_flow_lbl[3];

	uint16_t ipv6_payload_len;
	uint8_t  ipv6_nexthdr,
		 ipv6_hop_limit;

	qdf_net_ipv6_addr_t ipv6_saddr,
			    ipv6_daddr;
} qdf_net_ipv6hdr_t;

/**
 * qdf_net_icmpv6hdr_t - ICMPv6 Header
 */
typedef struct {
	uint8_t	 icmp6_type;
	uint8_t	 icmp6_code;
	uint16_t icmp6_cksum;

	union {
		uint32_t un_data32[1];
		uint16_t un_data16[2];
		uint8_t  un_data8[4];

		struct {
			uint16_t identifier;
			uint16_t sequence;
		} u_echo;

		struct {
#if defined(QDF_LITTLE_ENDIAN_MACHINE)
			uint32_t reserved:5,
				 override:1,
				 solicited:1,
				 router:1,
				 reserved2:24;
#elif defined(QDF_BIG_ENDIAN_MACHINE)
			uint32_t router:1,
				 solicited:1,
				 override:1,
				 reserved:29;
#else
#error  "Please fix"
#endif
		} u_nd_advt;

		struct {
			uint8_t	hop_limit;
#if defined(QDF_LITTLE_ENDIAN_MACHINE)
			uint8_t	reserved:6,
				other:1,
				managed:1;

#elif defined(QDF_BIG_ENDIAN_MACHINE)
			uint8_t	managed:1,
				other:1,
				reserved:6;
#else
#error  "Please fix"
#endif
			uint16_t rt_lifetime;
		} u_nd_ra;

	} icmp6_dataun;

} qdf_net_icmpv6hdr_t;

/**
 * qdf_net_nd_msg_t - Neighbor Discovery Message
 */
typedef struct {
	qdf_net_icmpv6hdr_t nd_icmph;
	qdf_net_ipv6_addr_t nd_target;
	uint8_t	nd_opt[0];
} qdf_net_nd_msg_t;


static inline int32_t qdf_csum_ipv6(const in6_addr_t *saddr,
				    const in6_addr_t *daddr,
				    __u32 len, unsigned short proto,
				    wsum_t sum)
{
	return (int32_t)__qdf_csum_ipv6(saddr, daddr, len, proto, sum);
}

#endif /*_QDF_NET_TYPES_H*/
