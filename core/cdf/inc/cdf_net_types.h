/*
 * Copyright (c) 2014 The Linux Foundation. All rights reserved.
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
 * DOC: cdf_net_types
 * This file defines types used in the networking stack abstraction.
 */

#ifndef _CDF_NET_TYPES_H
#define _CDF_NET_TYPES_H

#include <qdf_types.h>          /* uint8_t, etc. */

#define ADF_NET_MAC_ADDR_MAX_LEN 6
#define ADF_NET_IF_NAME_SIZE    64
#define ADF_NET_ETH_LEN         ADF_NET_MAC_ADDR_MAX_LEN
#define ADF_NET_MAX_MCAST_ADDR  64

/* Extended Traffic ID  passed to target if the TID is unknown */
#define ADF_NBUF_TX_EXT_TID_INVALID     0x1f

/**
 * cdf_nbuf_exemption_type - CDF net buf exemption types for encryption
 * @CDF_NBUF_EXEMPT_NO_EXEMPTION: No exemption
 * @CDF_NBUF_EXEMPT_ALWAYS: Exempt always
 * @CDF_NBUF_EXEMPT_ON_KEY_MAPPING_KEY_UNAVAILABLE: Exempt on key mapping
 */
enum cdf_nbuf_exemption_type {
	CDF_NBUF_EXEMPT_NO_EXEMPTION = 0,
	CDF_NBUF_EXEMPT_ALWAYS,
	CDF_NBUF_EXEMPT_ON_KEY_MAPPING_KEY_UNAVAILABLE
};

/**
 * typedef cdf_nbuf_tx_cksum_t - transmit checksum offload types
 * @CDF_NBUF_TX_CKSUM_NONE: No checksum offload
 * @CDF_NBUF_TX_CKSUM_IP: IP header checksum offload
 * @CDF_NBUF_TX_CKSUM_TCP_UDP: TCP/UDP checksum offload
 * @CDF_NBUF_TX_CKSUM_TCP_UDP_IP: TCP/UDP and IP header checksum offload
 */

typedef enum {
	CDF_NBUF_TX_CKSUM_NONE,
	CDF_NBUF_TX_CKSUM_IP,
	CDF_NBUF_TX_CKSUM_TCP_UDP,
	CDF_NBUF_TX_CKSUM_TCP_UDP_IP,

} cdf_nbuf_tx_cksum_t;

/**
 * typedef cdf_nbuf_l4_rx_cksum_type_t - receive checksum API types
 * @CDF_NBUF_RX_CKSUM_TCP: Rx checksum TCP
 * @CDF_NBUF_RX_CKSUM_UDP: Rx checksum UDP
 * @CDF_NBUF_RX_CKSUM_TCPIPV6: Rx checksum TCP IPV6
 * @CDF_NBUF_RX_CKSUM_UDPIPV6: Rx checksum UDP IPV6
 * @CDF_NBUF_RX_CKSUM_TCP_NOPSEUDOHEADER: Rx checksum TCP no pseudo header
 * @CDF_NBUF_RX_CKSUM_UDP_NOPSEUDOHEADER: Rx checksum UDP no pseudo header
 * @CDF_NBUF_RX_CKSUM_TCPSUM16: Rx checksum TCP SUM16
 */
typedef enum {
	CDF_NBUF_RX_CKSUM_TCP = 0x0001,
	CDF_NBUF_RX_CKSUM_UDP = 0x0002,
	CDF_NBUF_RX_CKSUM_TCPIPV6 = 0x0010,
	CDF_NBUF_RX_CKSUM_UDPIPV6 = 0x0020,
	CDF_NBUF_RX_CKSUM_TCP_NOPSEUDOHEADER = 0x0100,
	CDF_NBUF_RX_CKSUM_UDP_NOPSEUDOHEADER = 0x0200,
	CDF_NBUF_RX_CKSUM_TCPSUM16 = 0x1000,
} cdf_nbuf_l4_rx_cksum_type_t;

/**
 * typedef cdf_nbuf_l4_rx_cksum_result_t - receive checksum status types
 * @CDF_NBUF_RX_CKSUM_NONE: Device failed to checksum
 * @CDF_NBUF_RX_CKSUM_TCP_UDP_HW: TCP/UDP cksum successful and value returned
 * @CDF_NBUF_RX_CKSUM_TCP_UDP_UNNECESSARY: TCP/UDP cksum successful, no value
 */
typedef enum {
	CDF_NBUF_RX_CKSUM_NONE = 0x0000,
	CDF_NBUF_RX_CKSUM_TCP_UDP_HW = 0x0010,
	CDF_NBUF_RX_CKSUM_TCP_UDP_UNNECESSARY = 0x0020,
} cdf_nbuf_l4_rx_cksum_result_t;

/**
 * typedef cdf_nbuf_rx_cksum_t - receive checksum type
 * @l4_type: L4 type
 * @l4_result: L4 result
 */
typedef struct {
	cdf_nbuf_l4_rx_cksum_type_t l4_type;
	cdf_nbuf_l4_rx_cksum_result_t l4_result;
	uint32_t val;
} cdf_nbuf_rx_cksum_t;

#endif /*_CDF_NET_TYPES_H*/
