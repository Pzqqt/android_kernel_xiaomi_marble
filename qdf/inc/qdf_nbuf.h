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
 * DOC: qdf_nbuf_public network buffer API
 * This file defines the network buffer abstraction.
 */

#ifndef _QDF_NBUF_H
#define _QDF_NBUF_H

#include <qdf_util.h>
#include <qdf_types.h>
#include <qdf_lock.h>
#include <i_qdf_trace.h>
#include <i_qdf_nbuf.h>
#include <qdf_net_types.h>

#define IPA_NBUF_OWNER_ID			0xaa55aa55
#define QDF_NBUF_PKT_TRAC_TYPE_EAPOL		0x02
#define QDF_NBUF_PKT_TRAC_TYPE_DHCP		0x04
#define QDF_NBUF_PKT_TRAC_TYPE_MGMT_ACTION	0x08
#define QDF_NBUF_PKT_TRAC_TYPE_ARP		0x10
#define QDF_NBUF_PKT_TRAC_TYPE_ICMP		0x20
#define QDF_NBUF_PKT_TRAC_TYPE_ICMPv6		0x40
#define QDF_HL_CREDIT_TRACKING			0x80

#define QDF_NBUF_PKT_TRAC_MAX_STRING		12
#define QDF_NBUF_PKT_TRAC_PROTO_STRING		4
#define QDF_NBUF_PKT_ERROR			1

#define QDF_NBUF_TRAC_IPV4_OFFSET		14
#define QDF_NBUF_TRAC_IPV4_HEADER_MASK		0xF
#define QDF_NBUF_TRAC_IPV4_HEADER_SIZE		20
#define QDF_NBUF_TRAC_DHCP_SRV_PORT		67
#define QDF_NBUF_TRAC_DHCP_CLI_PORT		68
#define QDF_NBUF_TRAC_ETH_TYPE_OFFSET		12
#define QDF_NBUF_TRAC_EAPOL_ETH_TYPE		0x888E
#define QDF_NBUF_TRAC_WAPI_ETH_TYPE		0x88b4
#define QDF_NBUF_TRAC_ARP_ETH_TYPE		0x0806
#define QDF_NBUF_PKT_IPV4_DSCP_MASK     0xFC
#define QDF_NBUF_PKT_IPV4_DSCP_SHIFT  0x02
#define QDF_NBUF_TRAC_TDLS_ETH_TYPE		0x890D
#define QDF_NBUF_TRAC_IPV4_ETH_TYPE     0x0800
#define QDF_NBUF_TRAC_IPV6_ETH_TYPE     0x86dd
#define QDF_NBUF_DEST_MAC_OFFSET		0
#define QDF_NBUF_SRC_MAC_OFFSET			6
#define QDF_NBUF_TRAC_IPV4_PROTO_TYPE_OFFSET  23
#define QDF_NBUF_TRAC_IPV4_DEST_ADDR_OFFSET   30
#define QDF_NBUF_TRAC_IPV4_SRC_ADDR_OFFSET    26
#define QDF_NBUF_TRAC_IPV6_PROTO_TYPE_OFFSET  20
#define QDF_NBUF_TRAC_IPV4_ADDR_MCAST_MASK    0xE0000000
#define QDF_NBUF_TRAC_IPV4_ADDR_BCAST_MASK    0xF0000000
#define QDF_NBUF_TRAC_IPV6_DEST_ADDR_OFFSET   38
#define QDF_NBUF_TRAC_IPV6_DEST_ADDR          0xFF00
#define QDF_NBUF_TRAC_IPV6_OFFSET		14
#define QDF_NBUF_TRAC_IPV6_HEADER_SIZE   40
#define QDF_NBUF_TRAC_ICMP_TYPE         1
#define QDF_NBUF_TRAC_TCP_TYPE          6
#define QDF_NBUF_TRAC_TCP_FLAGS_OFFSET       (47 - 34)
#define QDF_NBUF_TRAC_TCP_ACK_OFFSET         (42 - 34)
#define QDF_NBUF_TRAC_TCP_HEADER_LEN_OFFSET  (46 - 34)
#define QDF_NBUF_TRAC_TCP_ACK_MASK           0x10
#define QDF_NBUF_TRAC_TCP_SPORT_OFFSET       (34 - 34)
#define QDF_NBUF_TRAC_TCP_DPORT_OFFSET       (36 - 34)
#define QDF_NBUF_TRAC_UDP_TYPE          17
#define QDF_NBUF_TRAC_ICMPV6_TYPE       0x3a
#define QDF_NBUF_TRAC_DHCP6_SRV_PORT		547
#define QDF_NBUF_TRAC_DHCP6_CLI_PORT		546
#define QDF_NBUF_TRAC_MDNS_SRC_N_DST_PORT	5353


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

/* ARP Related MASK */
#define QDF_NBUF_PKT_ARP_OPCODE_OFFSET	20
#define QDF_NBUF_PKT_ARPOP_REQ		1
#define QDF_NBUF_PKT_ARPOP_REPLY	2
#define QDF_NBUF_PKT_ARP_SRC_IP_OFFSET	28
#define QDF_NBUF_PKT_ARP_TGT_IP_OFFSET	38

/* ICMPv4 Related MASK */
#define QDF_NBUF_PKT_ICMPv4_OPCODE_OFFSET	34
#define QDF_NBUF_PKT_ICMPv4OP_REQ		0x08
#define QDF_NBUF_PKT_ICMPv4OP_REPLY		0x00
#define QDF_NBUF_PKT_ICMPv4_SRC_IP_OFFSET	26
#define QDF_NBUF_PKT_ICMPv4_TGT_IP_OFFSET	30

/* TCP Related MASK */
#define QDF_NBUF_PKT_TCP_OPCODE_OFFSET		47
#define QDF_NBUF_PKT_TCPOP_SYN			0x02
#define QDF_NBUF_PKT_TCPOP_SYN_ACK		0x12
#define QDF_NBUF_PKT_TCPOP_ACK			0x10
#define QDF_NBUF_PKT_TCP_SRC_PORT_OFFSET	34
#define QDF_NBUF_PKT_TCP_DST_PORT_OFFSET	36

/* DNS Related MASK */
#define QDF_NBUF_PKT_DNS_OVER_UDP_OPCODE_OFFSET	44
#define QDF_NBUF_PKT_DNSOP_BITMAP		0xF800
#define QDF_NBUF_PKT_DNSOP_STANDARD_QUERY	0x0000
#define QDF_NBUF_PKT_DNSOP_STANDARD_RESPONSE	0x8000
#define QDF_NBUF_PKT_DNS_SRC_PORT_OFFSET	34
#define QDF_NBUF_PKT_DNS_DST_PORT_OFFSET	36
#define QDF_NBUF_PKT_DNS_NAME_OVER_UDP_OFFSET	54
#define QDF_NBUF_PKT_DNS_STANDARD_PORT		53

/* Tracked Packet types */
#define QDF_NBUF_TX_PKT_INVALID              0
#define QDF_NBUF_TX_PKT_DATA_TRACK           1
#define QDF_NBUF_TX_PKT_MGMT_TRACK           2
#define QDF_NBUF_RX_PKT_DATA_TRACK           3

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
#define QDF_NBUF_TX_PKT_LI_DP                11

/* qdf_nbuf allocate and map max retry threshold when failed */
#define QDF_NBUF_ALLOC_MAP_RETRY_THRESHOLD      20

/* Enable flag to print TSO specific prints in datapath */
#ifdef TSO_DEBUG_LOG_ENABLE
#define TSO_DEBUG(fmt, args ...) \
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_NONE, \
		fmt, ## args)
#else
#define TSO_DEBUG(fmt, args ...)
#endif

#define IEEE80211_AMPDU_FLAG    0x01

#ifdef GET_MSDU_AGGREGATION
#define IEEE80211_AMSDU_FLAG    0x02
#endif

#define MAX_CHAIN 8
#define QDF_MON_STATUS_MPDU_FCS_BMAP_NWORDS 8

/**
 * struct mon_rx_status - This will have monitor mode rx_status extracted from
 * htt_rx_desc used later to update radiotap information.
 * @tsft: Time Synchronization Function timer
 * @ppdu_timestamp: Timestamp in the PPDU_START TLV
 * @preamble_type: Preamble type in radio header
 * @chan_freq: Capture channel frequency
 * @chan_num: Capture channel number
 * @chan_flags: Bitmap of Channel flags, IEEE80211_CHAN_TURBO,
 *              IEEE80211_CHAN_CCK...
 * @ht_flags: HT flags, only present for HT frames.
 * @vht_flags: VHT flags, only present for VHT frames.
 * @vht_flag_values1-5: Contains corresponding data for flags field
 * @he_flags: HE (11ax) flags, only present in HE frames
 * @he_mu_flags: HE-MU (11ax) flags, only present in HE frames
 * @he_mu_other_flags: HE-MU-OTHER (11ax) flags, only present in HE frames
 * @he_sig_A1_known: HE (11ax) sig A1 known field
 * @he_sig_A2_known: HE (11ax) sig A2 known field
 * @he_sig_b_common: HE (11ax) sig B common field
 * @he_sig_b_common_known: HE (11ax) sig B common known field
 * @l_sig_a_info: L_SIG_A value coming in Rx descriptor
 * @l_sig_b_info: L_SIG_B value coming in Rx descriptor
 * @rate: Rate in terms 500Kbps
 * @rtap_flags: Bit map of available fields in the radiotap
 * @ant_signal_db: Rx packet RSSI
 * @nr_ant: Number of Antennas used for streaming
 * @mcs: MCS index of Rx frame
 * @ht_mcs: MCS index for HT RX frames
 * @nss: Number of spatial streams
 * @bw: bandwidth of rx frame
 * @is_stbc: Is STBC enabled
 * @sgi: Rx frame short guard interval
 * @he_re: HE range extension
 * @ldpc: ldpc enabled
 * @beamformed: Is frame beamformed.
 * @he_sig_b_common_RU[4]: HE (11ax) common RU assignment index
 * @rssi_comb: Combined RSSI
 * @rssi[MAX_CHAIN]: 8 bits RSSI per 20Mhz per chain
 * @duration: 802.11 Duration
 * @frame_control_info_valid: field indicates if fc value is valid
 * @frame_control: frame control field
 * @ast_index: AST table hash index
 * @tid: QoS traffic tid number
 * @rs_fcs_err: FCS error flag
 * @rs_flags: Flags to indicate AMPDU or AMSDU aggregation
 * @cck_flag: Flag to indicate CCK modulation
 * @ofdm_flag: Flag to indicate OFDM modulation
 * @ulofdma_flag: Flag to indicate UL OFDMA PPDU
 * @he_per_user_1: HE per user info1
 * @he_per_user_2: HE per user info2
 * @he_per_user_position: HE per user position info
 * @he_per_user_known: HE per user known info
 * @he_flags1: HE flags
 * @he_flags2: HE flags
 * @he_RU[4]: HE RU assignment index
 * @he_data1: HE property of received frame
 * @he_data2: HE property of received frame
 * @he_data3: HE property of received frame
 * @he_data4: HE property of received frame
 * @he_data5: HE property of received frame
 * @prev_ppdu_id: ppdu_id in previously received message
 * @ppdu_id: Id of the PLCP protocol data unit
 *
 * The following variables are not coming from the TLVs.
 * These variables are placeholders for passing information to update_radiotap
 * function.
 * @device_id: Device ID coming from sub-system (PCI, AHB etc..)
 * @chan_noise_floor: Channel Noise Floor for the pdev
 * @data_sequence_control_info_valid: field to indicate validity of seq control
 * @first_data_seq_ctrl: Sequence ctrl field of first data frame
 * @rxpcu_filter_pass: Flag which indicates whether RX packets are received in
 *						BSS mode(not in promisc mode)
 * @rssi_chain: Rssi chain per nss per bw
 */
struct mon_rx_status {
	uint64_t tsft;
	uint32_t ppdu_timestamp;
	uint32_t preamble_type;
	qdf_freq_t chan_freq;
	uint16_t chan_num;
	uint16_t chan_flags;
	uint16_t ht_flags;
	uint16_t vht_flags;
	uint16_t vht_flag_values6;
	uint16_t he_flags;
	uint16_t he_mu_flags;
	uint16_t he_mu_other_flags;
	uint16_t he_sig_A1_known;
	uint16_t he_sig_A2_known;
	uint16_t he_sig_b_common;
	uint16_t he_sig_b_common_known;
	uint32_t l_sig_a_info;
	uint32_t l_sig_b_info;
	uint8_t  rate;
	uint8_t  rtap_flags;
	uint8_t  ant_signal_db;
	uint8_t  nr_ant;
	uint8_t  mcs;
	uint8_t  ht_mcs;
	uint8_t  nss;
	uint16_t  tcp_msdu_count;
	uint16_t  udp_msdu_count;
	uint16_t  other_msdu_count;
	uint8_t  bw;
	uint8_t  vht_flag_values1;
	uint8_t  vht_flag_values2;
	uint8_t  vht_flag_values3[4];
	uint8_t  vht_flag_values4;
	uint8_t  vht_flag_values5;
	uint8_t  is_stbc;
	uint8_t  sgi;
	uint8_t  he_re;
	uint8_t  ldpc;
	uint8_t  beamformed;
	uint8_t  he_sig_b_common_RU[4];
	int8_t   rssi_comb;
	uint64_t rssi[MAX_CHAIN];
	uint8_t  reception_type;
	uint16_t duration;
	uint8_t frame_control_info_valid;
	uint16_t frame_control;
	uint32_t ast_index;
	uint32_t tid;
	uint8_t  rs_fcs_err;
	uint8_t      rs_flags;
	uint8_t cck_flag;
	uint8_t ofdm_flag;
	uint8_t ulofdma_flag;
	/* New HE radiotap fields */
	uint16_t he_per_user_1;
	uint16_t he_per_user_2;
	uint8_t he_per_user_position;
	uint8_t he_per_user_known;
	uint16_t he_flags1;
	uint16_t he_flags2;
	uint8_t he_RU[4];
	uint16_t he_data1;
	uint16_t he_data2;
	uint16_t he_data3;
	uint16_t he_data4;
	uint16_t he_data5;
	uint16_t he_data6;
	uint32_t ppdu_len;
	uint32_t prev_ppdu_id;
	uint32_t ppdu_id;
	uint32_t device_id;
	int16_t chan_noise_floor;
	uint8_t monitor_direct_used;
	uint8_t data_sequence_control_info_valid;
	uint16_t first_data_seq_ctrl;
	uint8_t ltf_size;
	uint8_t rxpcu_filter_pass;
	int8_t rssi_chain[8][8];
	uint32_t rx_antenna;
};

/**
 * struct mon_rx_user_status - This will have monitor mode per user rx_status
 * extracted from hardware TLV.
 * @mcs: MCS index of Rx frame
 * @nss: Number of spatial streams
 * @mu_ul_info_valid: MU UL info below is valid
 * @ofdma_ru_start_index: OFDMA RU start index
 * @ofdma_ru_width: OFDMA total RU width
 * @ofdma_ru_size: OFDMA RU size index
 * @mu_ul_user_v0_word0: MU UL user info word 0
 * @mu_ul_user_v0_word1: MU UL user info word 1
 * @ast_index: AST table hash index
 * @tid: QoS traffic tid number
 * @tcp_msdu_count: tcp protocol msdu count
 * @udp_msdu_count: udp protocol msdu count
 * @other_msdu_count: other protocol msdu count
 * @frame_control: frame control field
 * @frame_control_info_valid: field indicates if fc value is valid
 * @data_sequence_control_info_valid: field to indicate validity of seq control
 * @first_data_seq_ctrl: Sequence ctrl field of first data frame
 * @preamble_type: Preamble type in radio header
 * @ht_flags: HT flags, only present for HT frames.
 * @vht_flags: VHT flags, only present for VHT frames.
 * @he_flags: HE (11ax) flags, only present in HE frames
 * @rtap_flags: Bit map of available fields in the radiotap
 * @rs_flags: Flags to indicate AMPDU or AMSDU aggregation
 * @mpdu_cnt_fcs_ok: mpdu count received with fcs ok
 * @mpdu_cnt_fcs_err: mpdu count received with fcs ok bitmap
 * @mpdu_fcs_ok_bitmap: mpdu with fcs ok bitmap
 * @mpdu_ok_byte_count: mpdu byte count with fcs ok
 * @mpdu_err_byte_count: mpdu byte count with fcs err
 */
struct mon_rx_user_status {
	uint32_t mcs:4,
		 nss:3,
		 mu_ul_info_valid:1,
		 ofdma_ru_start_index:7,
		 ofdma_ru_width:7,
		 ofdma_ru_size:8;
	uint32_t mu_ul_user_v0_word0;
	uint32_t mu_ul_user_v0_word1;
	uint32_t ast_index;
	uint32_t tid;
	uint16_t tcp_msdu_count;
	uint16_t udp_msdu_count;
	uint16_t other_msdu_count;
	uint16_t frame_control;
	uint8_t frame_control_info_valid;
	uint8_t data_sequence_control_info_valid;
	uint16_t first_data_seq_ctrl;
	uint32_t preamble_type;
	uint16_t ht_flags;
	uint16_t vht_flags;
	uint16_t he_flags;
	uint8_t rtap_flags;
	uint8_t rs_flags;
	uint32_t mpdu_cnt_fcs_ok;
	uint32_t mpdu_cnt_fcs_err;
	uint32_t mpdu_fcs_ok_bitmap[QDF_MON_STATUS_MPDU_FCS_BMAP_NWORDS];
	uint32_t mpdu_ok_byte_count;
	uint32_t mpdu_err_byte_count;
};

/**
 * struct qdf_radiotap_vendor_ns - Vendor Namespace header as per
 * Radiotap spec: https://www.radiotap.org/fields/Vendor%20Namespace.html
 * @oui: Vendor OUI
 * @selector: sub_namespace selector
 * @skip_length: How many bytes of Vendor Namespace data that follows
 */
struct qdf_radiotap_vendor_ns {
	uint8_t oui[3];
	uint8_t selector;
	uint16_t skip_length;
} __attribute__((__packed__));

/**
 * strcut qdf_radiotap_vendor_ns_ath - Combined QTI Vendor NS
 * including the Radiotap specified Vendor Namespace header and
 * QTI specific Vendor Namespace data
 * @lsig: L_SIG_A (or L_SIG)
 * @device_id: Device Identification
 * @lsig_b: L_SIG_B
 * @ppdu_start_timestamp: Timestamp from RX_PPDU_START TLV
 */
struct qdf_radiotap_vendor_ns_ath {
	struct qdf_radiotap_vendor_ns hdr;
	/* QTI specific data follows */
	uint32_t lsig;
	uint32_t device_id;
	uint32_t lsig_b;
	uint32_t ppdu_start_timestamp;
} __attribute__((__packed__));

/* Masks for HE SIG known fields in mon_rx_status structure */
#define QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU0	0x00000001
#define QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU1	0x00000002
#define QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU2	0x00000004
#define QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU3	0x00000008
#define QDF_MON_STATUS_HE_SIG_B_USER_KNOWN_SIG_B_ALL   0x00fe0000
#define QDF_MON_STATUS_HE_SIG_A1_HE_FORMAT_SU		0x00000000
#define QDF_MON_STATUS_HE_SIG_A1_HE_FORMAT_EXT_SU	0x40000000
#define QDF_MON_STATUS_HE_SIG_A1_HE_FORMAT_TRIG		0xc0000000

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

/* IPV4 header fields offset values */
#define IPV4_PKT_LEN_OFFSET           16
#define IPV4_TCP_SEQ_NUM_OFFSET       38
#define IPV4_SRC_ADDR_OFFSET          26
#define IPV4_DST_ADDR_OFFSET          30
#define IPV4_SRC_PORT_OFFSET          34
#define IPV4_DST_PORT_OFFSET          36

/* IPV4 ICMP Related Mask */
#define ICMP_SEQ_NUM_OFFSET           40
#define ICMP_SUBTYPE_OFFSET           34
#define ICMP_REQUEST                  0x08
#define ICMP_RESPONSE                 0x00

#define IPV6_ADDR_STR "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:"\
			"%02x%02x:%02x%02x"

/* IPV6 header fields offset values */
#define IPV6_PKT_LEN_OFFSET           18
#define IPV6_TCP_SEQ_NUM_OFFSET       58
#define IPV6_SRC_ADDR_OFFSET          22
#define IPV6_DST_ADDR_OFFSET          38
#define IPV6_SRC_PORT_OFFSET          54
#define IPV6_DST_PORT_OFFSET          56

/* IPV6 ICMPV6 Related Mask */
#define ICMPV6_SEQ_NUM_OFFSET         60
#define ICMPV6_SUBTYPE_OFFSET         54
#define ICMPV6_REQUEST                0x80
#define ICMPV6_RESPONSE               0x81
#define ICMPV6_RS                     0x85
#define ICMPV6_RA                     0x86
#define ICMPV6_NS                     0x87
#define ICMPV6_NA                     0x88

#define QDF_NBUF_IPA_CHECK_MASK		0x80000000

/* HE Radiotap data1 Mask */
#define QDF_MON_STATUS_HE_SU_FORMAT_TYPE 0x0000
#define QDF_MON_STATUS_HE_EXT_SU_FORMAT_TYPE 0x0001
#define QDF_MON_STATUS_HE_MU_FORMAT_TYPE 0x0002
#define QDF_MON_STATUS_HE_TRIG_FORMAT_TYPE 0x0003


#define QDF_MON_STATUS_HE_BEAM_CHANGE_KNOWN 0x0008
#define QDF_MON_STATUS_HE_DL_UL_KNOWN 0x0010
#define QDF_MON_STATUS_HE_MCS_KNOWN 0x0020
#define QDF_MON_STATUS_HE_DCM_KNOWN 0x0040
#define QDF_MON_STATUS_HE_CODING_KNOWN 0x0080
#define QDF_MON_STATUS_HE_LDPC_EXTRA_SYMBOL_KNOWN 0x0100
#define QDF_MON_STATUS_HE_STBC_KNOWN 0x0200
#define QDF_MON_STATUS_HE_DATA_BW_RU_KNOWN 0x4000
#define QDF_MON_STATUS_HE_DOPPLER_KNOWN 0x8000
#define QDF_MON_STATUS_HE_BSS_COLOR_KNOWN 0x0004

/* HE Radiotap data2 Mask */
#define QDF_MON_STATUS_HE_GI_KNOWN 0x0002
#define QDF_MON_STATUS_TXBF_KNOWN 0x0010
#define QDF_MON_STATUS_PE_DISAMBIGUITY_KNOWN 0x0020
#define QDF_MON_STATUS_TXOP_KNOWN 0x0040
#define QDF_MON_STATUS_LTF_SYMBOLS_KNOWN 0x0004
#define QDF_MON_STATUS_PRE_FEC_PADDING_KNOWN 0x0008
#define QDF_MON_STATUS_MIDABLE_PERIODICITY_KNOWN 0x0080

/* HE radiotap data3 shift values */
#define QDF_MON_STATUS_BEAM_CHANGE_SHIFT 6
#define QDF_MON_STATUS_DL_UL_SHIFT 7
#define QDF_MON_STATUS_TRANSMIT_MCS_SHIFT 8
#define QDF_MON_STATUS_DCM_SHIFT 12
#define QDF_MON_STATUS_CODING_SHIFT 13
#define QDF_MON_STATUS_LDPC_EXTRA_SYMBOL_SHIFT 14
#define QDF_MON_STATUS_STBC_SHIFT 15

/* HE radiotap data4 shift values */
#define QDF_MON_STATUS_STA_ID_SHIFT 4

/* HE radiotap data5 */
#define QDF_MON_STATUS_GI_SHIFT 4
#define QDF_MON_STATUS_HE_LTF_SIZE_SHIFT 6
#define QDF_MON_STATUS_HE_LTF_SYM_SHIFT 8
#define QDF_MON_STATUS_TXBF_SHIFT 14
#define QDF_MON_STATUS_PE_DISAMBIGUITY_SHIFT 15
#define QDF_MON_STATUS_PRE_FEC_PAD_SHIFT 12

/* HE radiotap data6 */
#define QDF_MON_STATUS_DOPPLER_SHIFT 4
#define QDF_MON_STATUS_TXOP_SHIFT 8

/* HE radiotap HE-MU flags1 */
#define QDF_MON_STATUS_SIG_B_MCS_KNOWN 0x0010
#define QDF_MON_STATUS_SIG_B_DCM_KNOWN 0x0040
#define QDF_MON_STATUS_SIG_B_SYM_NUM_KNOWN 0x8000
#define QDF_MON_STATUS_RU_0_KNOWN 0x0100
#define QDF_MON_STATUS_RU_1_KNOWN 0x0200
#define QDF_MON_STATUS_RU_2_KNOWN 0x0400
#define QDF_MON_STATUS_RU_3_KNOWN 0x0800
#define QDF_MON_STATUS_DCM_FLAG_1_SHIFT 5
#define QDF_MON_STATUS_SPATIAL_REUSE_MU_KNOWN 0x0100
#define QDF_MON_STATUS_SIG_B_COMPRESSION_FLAG_1_KNOWN 0x4000

/* HE radiotap HE-MU flags2 */
#define QDF_MON_STATUS_SIG_B_COMPRESSION_FLAG_2_SHIFT 3
#define QDF_MON_STATUS_BW_KNOWN 0x0004
#define QDF_MON_STATUS_NUM_SIG_B_SYMBOLS_SHIFT 4
#define QDF_MON_STATUS_SIG_B_COMPRESSION_FLAG_2_KNOWN 0x0100
#define QDF_MON_STATUS_NUM_SIG_B_FLAG_2_SHIFT 9
#define QDF_MON_STATUS_LTF_FLAG_2_SYMBOLS_SHIFT 12
#define QDF_MON_STATUS_LTF_KNOWN 0x8000

/* HE radiotap per_user_1 */
#define QDF_MON_STATUS_STA_SPATIAL_SHIFT 11
#define QDF_MON_STATUS_TXBF_SHIFT 14
#define QDF_MON_STATUS_RESERVED_SET_TO_1_SHIFT 19
#define QDF_MON_STATUS_STA_CODING_SHIFT 20

/* HE radiotap per_user_2 */
#define QDF_MON_STATUS_STA_MCS_SHIFT 4
#define QDF_MON_STATUS_STA_DCM_SHIFT 5

/* HE radiotap per user known */
#define QDF_MON_STATUS_USER_FIELD_POSITION_KNOWN 0x01
#define QDF_MON_STATUS_STA_ID_PER_USER_KNOWN 0x02
#define QDF_MON_STATUS_STA_NSTS_KNOWN 0x04
#define QDF_MON_STATUS_STA_TX_BF_KNOWN 0x08
#define QDF_MON_STATUS_STA_SPATIAL_CONFIG_KNOWN 0x10
#define QDF_MON_STATUS_STA_MCS_KNOWN 0x20
#define QDF_MON_STATUS_STA_DCM_KNOWN 0x40
#define QDF_MON_STATUS_STA_CODING_KNOWN 0x80

/**
 * qdf_proto_type - protocol type
 * @QDF_PROTO_TYPE_DHCP - DHCP
 * @QDF_PROTO_TYPE_EAPOL - EAPOL
 * @QDF_PROTO_TYPE_ARP - ARP
 * @QDF_PROTO_TYPE_MGMT - MGMT
 * @QDF_PROTO_TYPE_ICMP - ICMP
 * @QDF_PROTO_TYPE_ICMPv6 - ICMPv6
 * QDF_PROTO_TYPE_EVENT - EVENT
 */
enum qdf_proto_type {
	QDF_PROTO_TYPE_DHCP,
	QDF_PROTO_TYPE_EAPOL,
	QDF_PROTO_TYPE_ARP,
	QDF_PROTO_TYPE_MGMT,
	QDF_PROTO_TYPE_ICMP,
	QDF_PROTO_TYPE_ICMPv6,
	QDF_PROTO_TYPE_EVENT,
	QDF_PROTO_TYPE_MAX
};

/**
 * qdf_reception_type - reception type used by lithium phy TLV
 * @QDF_RECEPTION_TYPE_ULOFMDA - UL OFDMA
 * @QDF_RECEPTION_TYPE_ULMIMO - UL MIMO
 * @QQDF_RECEPTION_TYPE_FRAMELESS - Frame less
 * @QDF_RECEPTION_TYPE_OTHER - All the other types
 */
enum qdf_reception_type {
	QDF_RECEPTION_TYPE_ULOFMDA,
	QDF_RECEPTION_TYPE_ULMIMO,
	QDF_RECEPTION_TYPE_OTHER,
	QDF_RECEPTION_TYPE_FRAMELESS
};

/**
 * cb_ftype - Frame type information in skb cb
 * @CB_FTYPE_INVALID - Invalid
 * @CB_FTYPE_MCAST2UCAST - Multicast to Unicast converted packet
 * @CB_FTYPE_TSO - TCP Segmentation Offload
 * @CB_FTYPE_TSO_SG - TSO Scatter Gather
 * @CB_FTYPE_SG - Scatter Gather
 * @CB_FTYPE_INTRABSS_FWD - Intra BSS forwarding
 * @CB_FTYPE_RX_INFO - Rx information
 * @CB_FTYPE_MESH_RX_INFO - Mesh Rx information
 * @CB_FTYPE_MESH_TX_INFO - Mesh Tx information
 * @CB_FTYPE_DMS - Directed Multicast Service
 */
enum cb_ftype {
	CB_FTYPE_INVALID = 0,
	CB_FTYPE_MCAST2UCAST = 1,
	CB_FTYPE_TSO = 2,
	CB_FTYPE_TSO_SG = 3,
	CB_FTYPE_SG = 4,
	CB_FTYPE_INTRABSS_FWD = 5,
	CB_FTYPE_RX_INFO = 6,
	CB_FTYPE_MESH_RX_INFO = 7,
	CB_FTYPE_MESH_TX_INFO = 8,
	CB_FTYPE_DMS = 9,
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
 * @QDF_PROTO_ICMPV6_RS - icmpv6 rs packet
 * @QDF_PROTO_ICMPV6_RA - icmpv6 ra packet
 * @QDF_PROTO_ICMPV6_NS - icmpv6 ns packet
 * @QDF_PROTO_ICMPV6_NA - icmpv6 na packet
 * @QDF_PROTO_IPV4_UDP - ipv4 udp
 * @QDF_PROTO_IPV4_TCP - ipv4 tcp
 * @QDF_PROTO_IPV6_UDP - ipv6 udp
 * @QDF_PROTO_IPV6_TCP - ipv6 tcp
 * @QDF_PROTO_MGMT_ASSOC -assoc
 * @QDF_PROTO_MGMT_DISASSOC - disassoc
 * @QDF_PROTO_MGMT_AUTH - auth
 * @QDF_PROTO_MGMT_DEAUTH - deauth
 * QDF_ROAM_SYNCH - roam synch indication from fw
 * QDF_ROAM_COMPLETE - roam complete cmd to fw
 * QDF_ROAM_EVENTID - roam eventid from fw
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
	QDF_PROTO_ICMPV6_RS,
	QDF_PROTO_ICMPV6_RA,
	QDF_PROTO_ICMPV6_NS,
	QDF_PROTO_ICMPV6_NA,
	QDF_PROTO_IPV4_UDP,
	QDF_PROTO_IPV4_TCP,
	QDF_PROTO_IPV6_UDP,
	QDF_PROTO_IPV6_TCP,
	QDF_PROTO_MGMT_ASSOC,
	QDF_PROTO_MGMT_DISASSOC,
	QDF_PROTO_MGMT_AUTH,
	QDF_PROTO_MGMT_DEAUTH,
	QDF_ROAM_SYNCH,
	QDF_ROAM_COMPLETE,
	QDF_ROAM_EVENTID,
	QDF_PROTO_SUBTYPE_MAX
};

/**
 * @qdf_nbuf_t - Platform indepedent packet abstraction
 */
typedef __qdf_nbuf_t qdf_nbuf_t;

/**
 * typedef qdf_nbuf_queue_head_t - Platform indepedent nbuf queue head
 */
typedef __qdf_nbuf_queue_head_t qdf_nbuf_queue_head_t;

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

#define QDF_NBUF_QUEUE_WALK_SAFE(queue, var, tvar)	\
		__qdf_nbuf_queue_walk_safe(queue, var, tvar)

#ifdef NBUF_MAP_UNMAP_DEBUG
/**
 * qdf_nbuf_map_check_for_leaks() - check for nbut map leaks
 *
 * Check for net buffers that have been mapped, but never unmapped.
 *
 * Returns: None
 */
void qdf_nbuf_map_check_for_leaks(void);

QDF_STATUS qdf_nbuf_map_debug(qdf_device_t osdev,
			      qdf_nbuf_t buf,
			      qdf_dma_dir_t dir,
			      const char *func,
			      uint32_t line);

#define qdf_nbuf_map(osdev, buf, dir) \
	qdf_nbuf_map_debug(osdev, buf, dir, __func__, __LINE__)

void qdf_nbuf_unmap_debug(qdf_device_t osdev,
			  qdf_nbuf_t buf,
			  qdf_dma_dir_t dir,
			  const char *func,
			  uint32_t line);

#define qdf_nbuf_unmap(osdev, buf, dir) \
	qdf_nbuf_unmap_debug(osdev, buf, dir, __func__, __LINE__)

QDF_STATUS qdf_nbuf_map_single_debug(qdf_device_t osdev,
				     qdf_nbuf_t buf,
				     qdf_dma_dir_t dir,
				     const char *func,
				     uint32_t line);

#define qdf_nbuf_map_single(osdev, buf, dir) \
	qdf_nbuf_map_single_debug(osdev, buf, dir, __func__, __LINE__)

void qdf_nbuf_unmap_single_debug(qdf_device_t osdev,
				 qdf_nbuf_t buf,
				 qdf_dma_dir_t dir,
				 const char *func,
				 uint32_t line);

#define qdf_nbuf_unmap_single(osdev, buf, dir) \
	qdf_nbuf_unmap_single_debug(osdev, buf, dir, __func__, __LINE__)

QDF_STATUS qdf_nbuf_map_nbytes_debug(qdf_device_t osdev,
				     qdf_nbuf_t buf,
				     qdf_dma_dir_t dir,
				     int nbytes,
				     const char *func,
				     uint32_t line);

#define qdf_nbuf_map_nbytes(osdev, buf, dir, nbytes) \
	qdf_nbuf_map_nbytes_debug(osdev, buf, dir, nbytes, __func__, __LINE__)

void qdf_nbuf_unmap_nbytes_debug(qdf_device_t osdev,
				 qdf_nbuf_t buf,
				 qdf_dma_dir_t dir,
				 int nbytes,
				 const char *func,
				 uint32_t line);

#define qdf_nbuf_unmap_nbytes(osdev, buf, dir, nbytes) \
	qdf_nbuf_unmap_nbytes_debug(osdev, buf, dir, nbytes, __func__, __LINE__)

QDF_STATUS qdf_nbuf_map_nbytes_single_debug(qdf_device_t osdev,
					    qdf_nbuf_t buf,
					    qdf_dma_dir_t dir,
					    int nbytes,
					    const char *func,
					    uint32_t line);

#define qdf_nbuf_map_nbytes_single(osdev, buf, dir, nbytes) \
	qdf_nbuf_map_nbytes_single_debug(osdev, buf, dir, nbytes, \
					 __func__, __LINE__)

void qdf_nbuf_unmap_nbytes_single_debug(qdf_device_t osdev,
					qdf_nbuf_t buf,
					qdf_dma_dir_t dir,
					int nbytes,
					const char *func,
					uint32_t line);

#define qdf_nbuf_unmap_nbytes_single(osdev, buf, dir, nbytes) \
	qdf_nbuf_unmap_nbytes_single_debug(osdev, buf, dir, nbytes, \
					   __func__, __LINE__)

#else /* NBUF_MAP_UNMAP_DEBUG */

static inline void qdf_nbuf_map_check_for_leaks(void) {}

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
qdf_nbuf_map_single(qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	return __qdf_nbuf_map_single(osdev, buf, dir);
}

static inline void
qdf_nbuf_unmap_single(qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	__qdf_nbuf_unmap_single(osdev, buf, dir);
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

static inline QDF_STATUS
qdf_nbuf_map_nbytes_single(
	qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir, int nbytes)
{
	return __qdf_nbuf_map_nbytes_single(osdev, buf, dir, nbytes);
}

static inline void
qdf_nbuf_unmap_nbytes_single(
	qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir, int nbytes)
{
	return __qdf_nbuf_unmap_nbytes_single(osdev, buf, dir, nbytes);
}
#endif /* NBUF_MAP_UNMAP_DEBUG */

/**
 * qdf_nbuf_queue_head_dequeue() - dequeue nbuf from the head of queue
 * @nbuf_queue_head: pointer to nbuf queue head
 *
 * Return: pointer to network buffer dequeued
 */
static inline
qdf_nbuf_t qdf_nbuf_queue_head_dequeue(qdf_nbuf_queue_head_t *nbuf_queue_head)
{
	return __qdf_nbuf_queue_head_dequeue(nbuf_queue_head);
}

/**
 * qdf_nbuf_queue_head_qlen() - length of the queue
 * @nbuf_queue_head: pointer to nbuf queue head
 *
 * Return: length of queue (number of nbufs) pointed by qdf_nbuf_queue_head_t
 */
static inline
uint32_t qdf_nbuf_queue_head_qlen(qdf_nbuf_queue_head_t *nbuf_queue_head)
{
	return __qdf_nbuf_queue_head_qlen(nbuf_queue_head);
}

/**
 * qdf_nbuf_queue_head_enqueue_tail() - enqueue nbuf into queue tail
 * @nbuf_queue_head: pointer to nbuf queue head
 * @nbuf: nbuf to be enqueued
 *
 * Return: None
 */
static inline
void qdf_nbuf_queue_head_enqueue_tail(qdf_nbuf_queue_head_t *nbuf_queue_head,
				      qdf_nbuf_t nbuf)
{
	return __qdf_nbuf_queue_head_enqueue_tail(nbuf_queue_head, nbuf);
}

/**
 * qdf_nbuf_queue_head_init() - initialize qdf_nbuf_queue_head_t
 * @nbuf_queue_head: pointer to nbuf queue head to be initialized
 *
 * Return: None
 */
static inline
void qdf_nbuf_queue_head_init(qdf_nbuf_queue_head_t *nbuf_queue_head)
{
	return __qdf_nbuf_queue_head_init(nbuf_queue_head);
}

/**
 * qdf_nbuf_queue_head_purge() - purge qdf_nbuf_queue_head_t
 * @nbuf_queue_head: pointer to nbuf queue head to be purged
 *
 * Return: None
 */
static inline
void qdf_nbuf_queue_head_purge(qdf_nbuf_queue_head_t *nbuf_queue_head)
{
	return __qdf_nbuf_queue_head_purge(nbuf_queue_head);
}

/**
 * qdf_nbuf_queue_head_lock() - Acquire the nbuf_queue_head lock
 * @head: nbuf_queue_head of the nbuf_list for which lock is to be acquired
 *
 * Return: void
 */
static inline void qdf_nbuf_queue_head_lock(qdf_nbuf_queue_head_t *head)
{
	__qdf_nbuf_queue_head_lock(head);
}

/**
 * qdf_nbuf_queue_head_unlock() - Release the nbuf queue lock
 * @head: nbuf_queue_head of the nbuf_list for which lock is to be release
 *
 * Return: void
 */
static inline void qdf_nbuf_queue_head_unlock(qdf_nbuf_queue_head_t *head)
{
	__qdf_nbuf_queue_head_unlock(head);
}

static inline void
qdf_nbuf_sync_for_cpu(qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	__qdf_nbuf_sync_for_cpu(osdev, buf, dir);
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
 * qdf_nbuf_get_frag_paddr() - get physical address for skb linear buffer
 *                              or skb fragment, based on frag_num passed
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
 * qdf_nbuf_get_tx_frag_paddr() - get physical address for skb fragments only
 * @buf: Network buffer
 *
 * Return: Fragment physical address
 * Usage guideline: Use “qdf_nbuf_frag_map()” to dma map the specific
 *                  skb fragment , followed by “qdf_nbuf_get_tx_frag_paddr”
 */
static inline qdf_dma_addr_t qdf_nbuf_get_tx_frag_paddr(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_tx_frag_paddr(buf);
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
qdf_nbuf_set_vdev_ctx(qdf_nbuf_t buf, uint8_t vdev_id)
{
	__qdf_nbuf_set_vdev_ctx(buf, vdev_id);
}

static inline void
qdf_nbuf_set_tx_ftype(qdf_nbuf_t buf, enum cb_ftype type)
{
	__qdf_nbuf_set_tx_ftype(buf, type);
}

static inline void
qdf_nbuf_set_rx_ftype(qdf_nbuf_t buf, enum cb_ftype type)
{
	__qdf_nbuf_set_rx_ftype(buf, type);
}



static inline uint8_t
qdf_nbuf_get_vdev_ctx(qdf_nbuf_t buf)
{
	return  __qdf_nbuf_get_vdev_ctx(buf);
}

static inline enum cb_ftype qdf_nbuf_get_tx_ftype(qdf_nbuf_t buf)
{
	return  __qdf_nbuf_get_tx_ftype(buf);
}

static inline enum cb_ftype qdf_nbuf_get_rx_ftype(qdf_nbuf_t buf)
{
	return  __qdf_nbuf_get_rx_ftype(buf);
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

/**
 * qdf_nbuf_set_rx_chfrag_start() - set msdu start bit
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void
qdf_nbuf_set_rx_chfrag_start(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_rx_chfrag_start(buf, val);
}

/**
 * qdf_nbuf_is_rx_chfrag_start() - get msdu start bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_rx_chfrag_start(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_rx_chfrag_start(buf);
}

/**
 * qdf_nbuf_set_rx_chfrag_cont() - set msdu continuation bit
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void
qdf_nbuf_set_rx_chfrag_cont(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_rx_chfrag_cont(buf, val);
}

/**
 * qdf_nbuf_is_rx_chfrag_cont() - get msdu continuation bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_rx_chfrag_cont(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_rx_chfrag_cont(buf);
}

/**
 * qdf_nbuf_set_rx_chfrag_end() - set msdu end bit
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void qdf_nbuf_set_rx_chfrag_end(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_rx_chfrag_end(buf, val);
}

/**
 * qdf_nbuf_is_rx_chfrag_end() - set msdu end bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_rx_chfrag_end(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_rx_chfrag_end(buf);
}

/**
 * qdf_nbuf_set_da_mcbc() - set da is mcbc
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void
qdf_nbuf_set_da_mcbc(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_da_mcbc(buf, val);
}

/**
 * qdf_nbuf_is_da_mcbc() - get da is mcbc bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_da_mcbc(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_da_mcbc(buf);
}

/**
 * qdf_nbuf_set_da_valid() - set  da valid bit
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void qdf_nbuf_set_da_valid(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_da_valid(buf, val);
}

/**
 * qdf_nbuf_is_da_valid() - get da valid bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_da_valid(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_da_valid(buf);
}

/**
 * qdf_nbuf_set_sa_valid() - set  sa valid bit
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void qdf_nbuf_set_sa_valid(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_sa_valid(buf, val);
}

/**
 * qdf_nbuf_is_sa_valid() - get da valid bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_sa_valid(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_sa_valid(buf);
}

/**
 * qdf_nbuf_set_rx_retry_flag() - set rx retry flag bit
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void qdf_nbuf_set_rx_retry_flag(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_rx_retry_flag(buf, val);
}

/**
 * qdf_nbuf_is_rx_retry_flag() - get rx retry flag bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_rx_retry_flag(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_rx_retry_flag(buf);
}

/**
 * qdf_nbuf_set_raw_frame() - set  raw_frame bit
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void qdf_nbuf_set_raw_frame(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_raw_frame(buf, val);
}

/**
 * qdf_nbuf_is_raw_frame() -  get raw_frame bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_raw_frame(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_raw_frame(buf);
}

/**
 * qdf_nbuf_set_tid_val() - set  tid_val
 * @buf: Network buffer
 * @val: 4 bits tid value
 */
static inline void qdf_nbuf_set_tid_val(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_tid_val(buf, val);
}

/**
 * qdf_nbuf_get_tid_val() - get  tid_val
 * @buf: Network buffer
 *
 * Return: integer value[4 bits tid value]
 */
static inline uint8_t qdf_nbuf_get_tid_val(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_tid_val(buf);
}

/**
 * qdf_nbuf_set_frag_list() - set  frag list bit
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void qdf_nbuf_set_is_frag(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_is_frag(buf, val);
}

/**
 * qdf_nbuf_is_sa_valid() - get da frag list bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_frag(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_frag(buf);
}

/**
 * qdf_nbuf_set_tx_chfrag_start() - set msdu start bit
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void
qdf_nbuf_set_tx_chfrag_start(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_tx_chfrag_start(buf, val);
}

/**
 * qdf_nbuf_is_tx_chfrag_start() - get msdu start bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_tx_chfrag_start(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_tx_chfrag_start(buf);
}

/**
 * qdf_nbuf_set_tx_chfrag_cont() - set msdu continuation bit
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void
qdf_nbuf_set_tx_chfrag_cont(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_tx_chfrag_cont(buf, val);
}

/**
 * qdf_nbuf_is_tx_chfrag_cont() - get msdu continuation bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_tx_chfrag_cont(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_tx_chfrag_cont(buf);
}

/**
 * qdf_nbuf_set_tx_chfrag_end() - set msdu end bit
 * @buf: Network buffer
 * @val: 0/1
 *
 * Return: void
 */
static inline void qdf_nbuf_set_tx_chfrag_end(qdf_nbuf_t buf, uint8_t val)
{
	__qdf_nbuf_set_tx_chfrag_end(buf, val);
}

/**
 * qdf_nbuf_is_tx_chfrag_end() - set msdu end bit
 * @buf: Network buffer
 *
 * Return: integer value - 0/1
 */
static inline int qdf_nbuf_is_tx_chfrag_end(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_tx_chfrag_end(buf);
}

static inline void
qdf_nbuf_dma_map_info(qdf_dma_map_t bmap, qdf_dmamap_info_t *sg)
{
	__qdf_nbuf_dma_map_info(bmap, sg);
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
 * qdf_nbuf_get_users() - function to get the number of users referencing this
 * network buffer
 *
 * @nbuf:   network buffer
 *
 * Return: number of user references to nbuf.
 */
static inline int qdf_nbuf_get_users(qdf_nbuf_t nbuf)
{
	return __qdf_nbuf_get_users(nbuf);
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

#ifdef NBUF_MEMORY_DEBUG
void qdf_net_buf_debug_init(void);
void qdf_net_buf_debug_exit(void);
void qdf_net_buf_debug_clean(void);
void qdf_net_buf_debug_add_node(qdf_nbuf_t net_buf, size_t size,
				const char *func_name, uint32_t line_num);
/**
 * qdf_net_buf_debug_update_node() - update nbuf in debug hash table
 *
 * Return: none
 */
void qdf_net_buf_debug_update_node(qdf_nbuf_t net_buf, const char *func_name,
				   uint32_t line_num);
void qdf_net_buf_debug_delete_node(qdf_nbuf_t net_buf);

/**
 * qdf_net_buf_debug_acquire_skb() - acquire skb to avoid memory leak
 * @net_buf: Network buf holding head segment (single)
 * @func_name: pointer to function name
 * @line_num: line number
 *
 * WLAN driver module's SKB which are allocated by network stack are
 * suppose to call this API before freeing it such that the SKB
 * is not reported as memory leak.
 *
 * Return: none
 */
void qdf_net_buf_debug_acquire_skb(qdf_nbuf_t net_buf,
				   const char *func_name,
				   uint32_t line_num);
void qdf_net_buf_debug_release_skb(qdf_nbuf_t net_buf);

/* nbuf allocation rouines */

#define qdf_nbuf_alloc(d, s, r, a, p) \
	qdf_nbuf_alloc_debug(d, s, r, a, p, __func__, __LINE__)

qdf_nbuf_t qdf_nbuf_alloc_debug(qdf_device_t osdev, qdf_size_t size,
				int reserve, int align, int prio,
				const char *func, uint32_t line);

#define qdf_nbuf_free(d) \
	qdf_nbuf_free_debug(d, __func__, __LINE__)

void qdf_nbuf_free_debug(qdf_nbuf_t nbuf, const char *func, uint32_t line);

#define qdf_nbuf_clone(buf)     \
	qdf_nbuf_clone_debug(buf, __func__, __LINE__)

/**
 * qdf_nbuf_clone_debug() - clone the nbuf (copy is readonly)
 * @buf: nbuf to clone from
 * @func: name of the calling function
 * @line: line number of the callsite
 *
 * This function clones the nbuf and creates a memory tracking
 * node corresponding to that cloned skbuff structure.
 *
 * Return: cloned buffer
 */
qdf_nbuf_t qdf_nbuf_clone_debug(qdf_nbuf_t buf, const char *func,
				uint32_t line);

#define qdf_nbuf_copy(buf)     \
	qdf_nbuf_copy_debug(buf, __func__, __LINE__)

/**
 * qdf_nbuf_copy_debug() - returns a private copy of the buf
 * @buf: nbuf to copy from
 * @func: name of the calling function
 * @line: line number of the callsite
 *
 * This API returns a private copy of the buf, the buf returned is completely
 * modifiable by callers. It also creates a memory tracking node corresponding
 * to that new skbuff structure.
 *
 * Return: copied buffer
 */
qdf_nbuf_t qdf_nbuf_copy_debug(qdf_nbuf_t buf, const char *func, uint32_t line);

#define qdf_nbuf_copy_expand(buf, headroom, tailroom)     \
	qdf_nbuf_copy_expand_debug(buf, headroom, tailroom, __func__, __LINE__)

/**
 * qdf_nbuf_copy_expand_debug() - copy and expand nbuf
 * @buf: Network buf instance
 * @headroom: Additional headroom to be added
 * @tailroom: Additional tailroom to be added
 * @func: name of the calling function
 * @line: line number of the callsite
 *
 * Return: New nbuf that is a copy of buf, with additional head and tailroom
 *	or NULL if there is no memory
 */
qdf_nbuf_t
qdf_nbuf_copy_expand_debug(qdf_nbuf_t buf, int headroom, int tailroom,
			   const char *func, uint32_t line);

#else /* NBUF_MEMORY_DEBUG */

static inline void qdf_net_buf_debug_init(void) {}
static inline void qdf_net_buf_debug_exit(void) {}

static inline void qdf_net_buf_debug_acquire_skb(qdf_nbuf_t net_buf,
						 const char *func_name,
						 uint32_t line_num)
{
}

static inline void qdf_net_buf_debug_release_skb(qdf_nbuf_t net_buf)
{
}

static inline void
qdf_net_buf_debug_update_node(qdf_nbuf_t net_buf, const char *func_name,
			      uint32_t line_num)
{
}

/* Nbuf allocation rouines */

#define qdf_nbuf_alloc(osdev, size, reserve, align, prio) \
	qdf_nbuf_alloc_fl(osdev, size, reserve, align, prio, \
			  __func__, __LINE__)
static inline qdf_nbuf_t
qdf_nbuf_alloc_fl(qdf_device_t osdev, qdf_size_t size, int reserve, int align,
		  int prio, const char *func, uint32_t line)
{
	return __qdf_nbuf_alloc(osdev, size, reserve, align, prio, func, line);
}

static inline void qdf_nbuf_free(qdf_nbuf_t buf)
{
	if (qdf_likely(buf))
		__qdf_nbuf_free(buf);
}

/**
 * qdf_nbuf_clone() - clone the nbuf (copy is readonly)
 * @buf: Pointer to network buffer
 *
 * This function clones the nbuf and returns new sk_buff
 * structure.
 *
 * Return: cloned skb
 */
static inline qdf_nbuf_t qdf_nbuf_clone(qdf_nbuf_t buf)
{
	return __qdf_nbuf_clone(buf);
}

/**
 * qdf_nbuf_copy() - returns a private copy of the buf
 * @buf: Pointer to network buffer
 *
 * This API returns a private copy of the buf, the buf returned is completely
 *  modifiable by callers
 *
 * Return: skb or NULL
 */
static inline qdf_nbuf_t qdf_nbuf_copy(qdf_nbuf_t buf)
{
	return __qdf_nbuf_copy(buf);
}

/**
 * qdf_nbuf_copy_expand() - copy and expand nbuf
 * @buf: Network buf instance
 * @headroom: Additional headroom to be added
 * @tailroom: Additional tailroom to be added
 *
 * Return: New nbuf that is a copy of buf, with additional head and tailroom
 *	or NULL if there is no memory
 */
static inline qdf_nbuf_t qdf_nbuf_copy_expand(qdf_nbuf_t buf, int headroom,
					      int tailroom)
{
	return __qdf_nbuf_copy_expand(buf, headroom, tailroom);
}
#endif /* NBUF_MEMORY_DEBUG */

#ifdef WLAN_FEATURE_FASTPATH
/**
 * qdf_nbuf_init_fast() - before put buf into pool,turn it to init state
 *
 * @buf: buf instance
 * Return: data pointer of this buf where new data has to be
 *         put, or NULL if there is not enough room in this buf.
 */
void qdf_nbuf_init_fast(qdf_nbuf_t nbuf);
#endif /* WLAN_FEATURE_FASTPATH */

/**
 * @qdf_nbuf_list_free() - free a list of nbufs
 * @buf_list: A list of nbufs to be freed
 *
 * Return: none
 */

static inline void qdf_nbuf_list_free(qdf_nbuf_t buf_list)
{
	while (buf_list) {
		qdf_nbuf_t next = qdf_nbuf_next(buf_list);
		qdf_nbuf_free(buf_list);
		buf_list = next;
	}
}

static inline void qdf_nbuf_tx_free(qdf_nbuf_t buf_list, int tx_err)
{
	qdf_nbuf_list_free(buf_list);
}

static inline void qdf_nbuf_ref(qdf_nbuf_t buf)
{
	__qdf_nbuf_ref(buf);
}

static inline int qdf_nbuf_shared(qdf_nbuf_t buf)
{
	return __qdf_nbuf_shared(buf);
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
 * qdf_nbuf_set_data_pointer() - set data pointer
 * @buf: Network buf instance
 * @data: data pointer
 *
 * Return: none
 */
static inline void qdf_nbuf_set_data_pointer(qdf_nbuf_t buf, uint8_t *data)
{
	__qdf_nbuf_set_data_pointer(buf, data);
}

/**
 * qdf_nbuf_set_len() - set data length
 * @buf: Network buf instance
 * @len: data length
 * Return: none
 */
static inline void qdf_nbuf_set_len(qdf_nbuf_t buf, uint32_t len)
{
	__qdf_nbuf_set_len(buf, len);
}

/**
 * qdf_nbuf_set_tail_pointer() - set data tail pointer
 * @buf: Network buf instance
 * @len: length of data
 *
 * Return: none
 */
static inline void qdf_nbuf_set_tail_pointer(qdf_nbuf_t buf, int len)
{
	__qdf_nbuf_set_tail_pointer(buf, len);
}

/**
 * qdf_nbuf_unlink_no_lock() - unlink a nbuf from nbuf list
 * @buf: Network buf instance
 * @list: list to use
 *
 * This is a lockless version, driver must acquire locks if it
 * needs to synchronize
 *
 * Return: none
 */
static inline void
qdf_nbuf_unlink_no_lock(qdf_nbuf_t buf, qdf_nbuf_queue_head_t *list)
{
	__qdf_nbuf_unlink_no_lock(buf, list);
}

/**
 * qdf_nbuf_reset() - reset the buffer data and pointer
 * @buf: Network buf instance
 * @reserve: reserve
 * @align: align
 *
 * Return: none
 */
static inline void qdf_nbuf_reset(qdf_nbuf_t buf, int reserve, int align)
{
	__qdf_nbuf_reset(buf, reserve, align);
}

/**
 * qdf_nbuf_dev_scratch_is_supported() - dev_scratch support for network buffer
 *                                       in kernel
 *
 * Return: true if dev_scratch is supported
 *         false if dev_scratch is not supported
 */
static inline bool qdf_nbuf_is_dev_scratch_supported(void)
{
	return __qdf_nbuf_is_dev_scratch_supported();
}

/**
 * qdf_nbuf_get_dev_scratch() - get dev_scratch of network buffer
 * @buf: Pointer to network buffer
 *
 * Return: dev_scratch if dev_scratch supported
 *         0 if dev_scratch not supported
 */
static inline unsigned long qdf_nbuf_get_dev_scratch(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_dev_scratch(buf);
}

/**
 * qdf_nbuf_set_dev_scratch() - set dev_scratch of network buffer
 * @buf: Pointer to network buffer
 * @value: value to be set in dev_scratch of network buffer
 *
 * Return: void
 */
static inline void qdf_nbuf_set_dev_scratch(qdf_nbuf_t buf, unsigned long value)
{
	__qdf_nbuf_set_dev_scratch(buf, value);
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

static inline qdf_nbuf_t
qdf_nbuf_queue_last(qdf_nbuf_queue_t *head)
{
	return __qdf_nbuf_queue_last(head);
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
 * qdf_nbuf_data_is_ipv6_mdsn_pkt() - check if it is MDNS packet.
 * @data: Pointer to packet data buffer
 *
 * This func. checks whether it is a MDNS packet or not.
 *
 * Return: true if it is a MDNS packet, false if not
 */
static inline
bool qdf_nbuf_is_ipv6_mdns_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_ipv6_mdns_pkt(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_ipv6_dhcp_pkt() - check if it is DHCP packet.
 * @data: Pointer to DHCP packet data buffer
 *
 * This func. checks whether it is a DHCP packet or not.
 *
 * Return: true if it is a DHCP packet
 *         false if not
 */
static inline
bool qdf_nbuf_is_ipv6_dhcp_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_ipv6_dhcp_pkt(qdf_nbuf_data(buf));
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
 * qdf_nbuf_is_ipv4_tdls_pkt() - check if packet is a tdls packet or not
 * @buf:  buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: true if packet is TDLS packet
 */
static inline
bool qdf_nbuf_is_ipv4_tdls_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_ipv4_tdls_pkt(buf);
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
 * qdf_nbuf_data_is_arp_req() - check if ARP packet is request.
 * @buf:  buffer
 *
 * This func. checks whether it is a ARP request or not.
 *
 * Return: true if it is a ARP request or FALSE if not
 */
static inline
bool qdf_nbuf_data_is_arp_req(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_arp_req(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_arp_rsp() - check if ARP packet is response.
 * @buf:  buffer
 *
 * This func. checks whether it is a ARP response or not.
 *
 * Return: true if it is a ARP response or FALSE if not
 */
static inline
bool qdf_nbuf_data_is_arp_rsp(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_arp_rsp(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_get_arp_src_ip() - get ARP packet source IP gateway.
 * @buf:  buffer
 *
 * Return: ARP packet source IP value.
 */
static inline
uint32_t qdf_nbuf_get_arp_src_ip(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_arp_src_ip(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_get_arp_tgt_ip() - get ARP packet target IP gateway.
 * @buf:  buffer
 *
 * Return: ARP packet target IP value.
 */
static inline
uint32_t qdf_nbuf_get_arp_tgt_ip(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_arp_tgt_ip(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_get_dns_domain_name() - get dns domain name of required length
 * @buf: buffer
 * @len: length to copy
 *
 * Return: dns domain name
 */
static inline
uint8_t *qdf_nbuf_get_dns_domain_name(qdf_nbuf_t buf, uint32_t len)
{
	return __qdf_nbuf_get_dns_domain_name(qdf_nbuf_data(buf), len);
}

/**
 * qdf_nbuf_data_is_dns_query() - check if skb data is a dns query
 * @buf: buffer
 *
 * Return: true if packet is dns query packet.
 *	   false otherwise.
 */
static inline
bool qdf_nbuf_data_is_dns_query(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_dns_query(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_dns_response() - check if skb data is a dns response
 * @buf:  buffer
 *
 * Return: true if packet is dns response packet.
 *	   false otherwise.
 */
static inline
bool qdf_nbuf_data_is_dns_response(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_dns_response(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_tcp_syn() - check if skb data is a tcp syn
 * @buf:  buffer
 *
 * Return: true if packet is tcp syn packet.
 *	   false otherwise.
 */
static inline
bool qdf_nbuf_data_is_tcp_syn(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_tcp_syn(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_tcp_syn_ack() - check if skb data is a tcp syn ack
 * @buf:  buffer
 *
 * Return: true if packet is tcp syn ack packet.
 *	   false otherwise.
 */
static inline
bool qdf_nbuf_data_is_tcp_syn_ack(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_tcp_syn_ack(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_tcp_ack() - check if skb data is a tcp ack
 * @buf:  buffer
 *
 * Return: true if packet is tcp ack packet.
 *	   false otherwise.
 */
static inline
bool qdf_nbuf_data_is_tcp_ack(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_tcp_ack(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_get_tcp_src_port() - get tcp src port
 * @buf:  buffer
 *
 * Return: tcp source port value.
 */
static inline
uint16_t qdf_nbuf_data_get_tcp_src_port(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_get_tcp_src_port(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_get_tcp_dst_port() - get tcp dst port
 * @buf:  buffer
 *
 * Return: tcp destination port value.
 */
static inline
uint16_t qdf_nbuf_data_get_tcp_dst_port(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_get_tcp_dst_port(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_icmpv4_req() - check if ICMPv4 packet is request.
 * @buf:  buffer
 *
 * This func. checks whether it is a ICMPv4 request or not.
 *
 * Return: true if it is a ICMPv4 request or fALSE if not
 */
static inline
bool qdf_nbuf_data_is_icmpv4_req(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_icmpv4_req(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_is_icmpv4_rsp() - check if ICMPv4 packet is res
 * @buf:  buffer
 *
 * Return: true if packet is icmpv4 response
 *	   false otherwise.
 */
static inline
bool qdf_nbuf_data_is_icmpv4_rsp(qdf_nbuf_t buf)
{
	return __qdf_nbuf_data_is_icmpv4_rsp(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_get_icmpv4_src_ip() - get icmpv4 src IP
 * @buf:  buffer
 *
 * Return: icmpv4 packet source IP value.
 */
static inline
uint32_t qdf_nbuf_get_icmpv4_src_ip(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_icmpv4_src_ip(qdf_nbuf_data(buf));
}

/**
 * qdf_nbuf_data_get_icmpv4_tgt_ip() - get icmpv4 target IP
 * @buf:  buffer
 *
 * Return: icmpv4 packet target IP value.
 */
static inline
uint32_t qdf_nbuf_get_icmpv4_tgt_ip(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_icmpv4_tgt_ip(qdf_nbuf_data(buf));
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
 * qdf_nbuf_data_is_ipv6_mcast_pkt() - check if it is IPV6 multicast packet.
 * @data: Pointer to IPV6 packet data buffer
 *
 * This func. checks whether it is a IPV6 multicast packet or not.
 *
 * Return: TRUE if it is a IPV6 multicast packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_data_is_ipv6_mcast_pkt(uint8_t *data)
{
	return __qdf_nbuf_data_is_ipv6_mcast_pkt(data);
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
 * qdf_nbuf_is_bcast_pkt() - check if it is broadcast packet.
 * @buf: Network buffer
 *
 * This func. checks whether packet is broadcast or not.
 *
 * Return: TRUE if it is broadcast packet
 *         FALSE if not
 */
static inline
bool qdf_nbuf_is_bcast_pkt(qdf_nbuf_t buf)
{
	return __qdf_nbuf_is_bcast_pkt(buf);
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
 * qdf_nbuf_unmap_tso_segment() - function to dma unmap TSO segment element
 *
 * @osdev: qdf device handle
 * @tso_seg: TSO segment element to be unmapped
 * @is_last_seg: whether this is last tso seg or not
 *
 * Return: none
 */
static inline void qdf_nbuf_unmap_tso_segment(qdf_device_t osdev,
			  struct qdf_tso_seg_elem_t *tso_seg,
			  bool is_last_seg)
{
	return __qdf_nbuf_unmap_tso_segment(osdev, tso_seg, is_last_seg);
}

/**
 * qdf_nbuf_get_tcp_payload_len() - function to return the tso payload len
 * @nbuf: network buffer
 *
 * Return: size of the tso packet
 */
static inline size_t qdf_nbuf_get_tcp_payload_len(qdf_nbuf_t nbuf)
{
	return __qdf_nbuf_get_tcp_payload_len(nbuf);
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

static inline int
qdf_nbuf_linearize(qdf_nbuf_t buf)
{
	return __qdf_nbuf_linearize(buf);
}

#ifdef NBUF_MEMORY_DEBUG
#define qdf_nbuf_unshare(d) \
	qdf_nbuf_unshare_debug(d, __func__, __LINE__)

static inline qdf_nbuf_t
qdf_nbuf_unshare_debug(qdf_nbuf_t buf, const char *func_name, uint32_t line_num)
{
	qdf_nbuf_t unshared_buf;

	unshared_buf = __qdf_nbuf_unshare(buf);

	if (qdf_likely(buf != unshared_buf)) {
		qdf_net_buf_debug_delete_node(buf);

		if (unshared_buf)
			qdf_net_buf_debug_add_node(unshared_buf, 0,
						   func_name, line_num);
	}

	return unshared_buf;
}

#else
static inline qdf_nbuf_t
qdf_nbuf_unshare(qdf_nbuf_t buf)
{
	return __qdf_nbuf_unshare(buf);
}
#endif

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

static inline void qdf_nbuf_record_rx_queue(qdf_nbuf_t buf, uint32_t queue_id)
{
	__qdf_nbuf_record_rx_queue(buf, queue_id);
}

static inline uint16_t
qdf_nbuf_get_queue_mapping(qdf_nbuf_t buf)
{
	return __qdf_nbuf_get_queue_mapping(buf);
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

/**
 * qdf_nbuf_mark_wakeup_frame() - mark wakeup frame.
 * @buf: Pointer to nbuf
 *
 * Return: None
 */
static inline void
qdf_nbuf_mark_wakeup_frame(qdf_nbuf_t buf)
{
	 __qdf_nbuf_mark_wakeup_frame(buf);
}

/**
 * qdf_nbuf_reg_free_cb - Registers nbuf free callback
 * @cb_func_ptr: Callback pointer
 *
 * This function registers nbuf free callback
 *
 * Return: void
 */
static inline void
qdf_nbuf_reg_free_cb(qdf_nbuf_free_t cb_func_ptr)
{
	 __qdf_nbuf_reg_free_cb(cb_func_ptr);
}

/**
 * qdf_nbuf_set_timestamp() - set the timestamp for frame
 *
 * @buf: sk buff
 *
 * Return: void
 */
static inline void
qdf_nbuf_set_timestamp(struct sk_buff *skb)
{
	__qdf_nbuf_set_timestamp(skb);
}

/**
 * qdf_nbuf_get_timestamp() - get the timestamp for frame
 *
 * @buf: sk buff
 *
 * Return: timestamp stored in skb in ms
 */
static inline uint64_t
qdf_nbuf_get_timestamp(struct sk_buff *skb)
{
	return __qdf_nbuf_get_timestamp(skb);
}

/**
 * qdf_nbuf_get_timedelta_ms() - get time difference in ms
 *
 * @buf: sk buff
 *
 * Return: time difference ms
 */
static inline uint64_t
qdf_nbuf_get_timedelta_ms(struct sk_buff *skb)
{
	return __qdf_nbuf_get_timedelta_ms(skb);
}

/**
 * qdf_nbuf_get_timedelta_us() - get time difference in micro seconds
 *
 * @buf: sk buff
 *
 * Return: time difference in micro seconds
 */
static inline uint64_t
qdf_nbuf_get_timedelta_us(struct sk_buff *skb)
{
	return __qdf_nbuf_get_timedelta_us(skb);
}

/**
 * qdf_nbuf_count_get() - get global nbuf gauge
 *
 * Return: global nbuf gauge
 */
static inline int qdf_nbuf_count_get(void)
{
	return __qdf_nbuf_count_get();
}

/**
 * qdf_nbuf_count_inc() - increment nbuf global count
 *
 * @buf: sk buff
 *
 * Return: void
 */
static inline void qdf_nbuf_count_inc(qdf_nbuf_t buf)
{
	return __qdf_nbuf_count_inc(buf);
}

/**
 * qdf_nbuf_count_dec() - decrement nbuf global count
 *
 * @buf: sk buff
 *
 * Return: void
 */
static inline void qdf_nbuf_count_dec(qdf_nbuf_t buf)
{
	return __qdf_nbuf_count_dec(buf);
}

/**
 * qdf_nbuf_mod_init() - Intialization routine for qdf_nbuf
 *
 * Return void
 */
static inline void qdf_nbuf_mod_init(void)
{
	return __qdf_nbuf_mod_init();
}

/**
 * qdf_nbuf_mod_init() - Unintialization routine for qdf_nbuf
 *
 * Return void
 */
static inline void qdf_nbuf_mod_exit(void)
{
	return __qdf_nbuf_mod_exit();
}

/**
 * qdf_nbuf_orphan() - orphan a nbuf
 * @buf: Pointer to network buffer
 *
 * If a buffer currently has an owner then we call the
 * owner's destructor function
 *
 * Return: void
 */
static inline void qdf_nbuf_orphan(qdf_nbuf_t buf)
{
	return __qdf_nbuf_orphan(buf);
}

#ifdef CONFIG_NBUF_AP_PLATFORM
#include <i_qdf_nbuf_api_w.h>
#else
#include <i_qdf_nbuf_api_m.h>
#endif
#endif /* _QDF_NBUF_H */
