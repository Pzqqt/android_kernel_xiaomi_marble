
/* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

 
 
 
 
 
 
 


#ifndef _RX_MSDU_START_H_
#define _RX_MSDU_START_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_MSDU_START 10

#define NUM_OF_QWORDS_RX_MSDU_START 5


struct rx_msdu_start {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rxpcu_mpdu_filter_in_category                           :  2, // [1:0]
                      sw_frame_group_id                                       :  7, // [8:2]
                      reserved_0                                              :  7, // [15:9]
                      phy_ppdu_id                                             : 16; // [31:16]
             uint32_t msdu_length                                             : 14, // [13:0]
                      stbc                                                    :  1, // [14:14]
                      ipsec_esp                                               :  1, // [15:15]
                      l3_offset                                               :  7, // [22:16]
                      ipsec_ah                                                :  1, // [23:23]
                      l4_offset                                               :  8; // [31:24]
             uint32_t msdu_number                                             :  8, // [7:0]
                      decap_format                                            :  2, // [9:8]
                      ipv4_proto                                              :  1, // [10:10]
                      ipv6_proto                                              :  1, // [11:11]
                      tcp_proto                                               :  1, // [12:12]
                      udp_proto                                               :  1, // [13:13]
                      ip_frag                                                 :  1, // [14:14]
                      tcp_only_ack                                            :  1, // [15:15]
                      da_is_bcast_mcast                                       :  1, // [16:16]
                      toeplitz_hash_sel                                       :  2, // [18:17]
                      ip_fixed_header_valid                                   :  1, // [19:19]
                      ip_extn_header_valid                                    :  1, // [20:20]
                      tcp_udp_header_valid                                    :  1, // [21:21]
                      mesh_control_present                                    :  1, // [22:22]
                      ldpc                                                    :  1, // [23:23]
                      ip4_protocol_ip6_next_header                            :  8; // [31:24]
             uint32_t toeplitz_hash_2_or_4                                    : 32; // [31:0]
             uint32_t flow_id_toeplitz                                        : 32; // [31:0]
             uint32_t user_rssi                                               :  8, // [7:0]
                      pkt_type                                                :  4, // [11:8]
                      sgi                                                     :  2, // [13:12]
                      rate_mcs                                                :  4, // [17:14]
                      receive_bandwidth                                       :  3, // [20:18]
                      reception_type                                          :  3, // [23:21]
                      mimo_ss_bitmap                                          :  8; // [31:24]
             uint32_t ppdu_start_timestamp_31_0                               : 32; // [31:0]
             uint32_t ppdu_start_timestamp_63_32                              : 32; // [31:0]
             uint32_t sw_phy_meta_data                                        : 32; // [31:0]
             uint32_t vlan_ctag_ci                                            : 16, // [15:0]
                      vlan_stag_ci                                            : 16; // [31:16]
#else
             uint32_t phy_ppdu_id                                             : 16, // [31:16]
                      reserved_0                                              :  7, // [15:9]
                      sw_frame_group_id                                       :  7, // [8:2]
                      rxpcu_mpdu_filter_in_category                           :  2; // [1:0]
             uint32_t l4_offset                                               :  8, // [31:24]
                      ipsec_ah                                                :  1, // [23:23]
                      l3_offset                                               :  7, // [22:16]
                      ipsec_esp                                               :  1, // [15:15]
                      stbc                                                    :  1, // [14:14]
                      msdu_length                                             : 14; // [13:0]
             uint32_t ip4_protocol_ip6_next_header                            :  8, // [31:24]
                      ldpc                                                    :  1, // [23:23]
                      mesh_control_present                                    :  1, // [22:22]
                      tcp_udp_header_valid                                    :  1, // [21:21]
                      ip_extn_header_valid                                    :  1, // [20:20]
                      ip_fixed_header_valid                                   :  1, // [19:19]
                      toeplitz_hash_sel                                       :  2, // [18:17]
                      da_is_bcast_mcast                                       :  1, // [16:16]
                      tcp_only_ack                                            :  1, // [15:15]
                      ip_frag                                                 :  1, // [14:14]
                      udp_proto                                               :  1, // [13:13]
                      tcp_proto                                               :  1, // [12:12]
                      ipv6_proto                                              :  1, // [11:11]
                      ipv4_proto                                              :  1, // [10:10]
                      decap_format                                            :  2, // [9:8]
                      msdu_number                                             :  8; // [7:0]
             uint32_t toeplitz_hash_2_or_4                                    : 32; // [31:0]
             uint32_t flow_id_toeplitz                                        : 32; // [31:0]
             uint32_t mimo_ss_bitmap                                          :  8, // [31:24]
                      reception_type                                          :  3, // [23:21]
                      receive_bandwidth                                       :  3, // [20:18]
                      rate_mcs                                                :  4, // [17:14]
                      sgi                                                     :  2, // [13:12]
                      pkt_type                                                :  4, // [11:8]
                      user_rssi                                               :  8; // [7:0]
             uint32_t ppdu_start_timestamp_31_0                               : 32; // [31:0]
             uint32_t ppdu_start_timestamp_63_32                              : 32; // [31:0]
             uint32_t sw_phy_meta_data                                        : 32; // [31:0]
             uint32_t vlan_stag_ci                                            : 16, // [31:16]
                      vlan_ctag_ci                                            : 16; // [15:0]
#endif
};


/* Description		RXPCU_MPDU_FILTER_IN_CATEGORY

			Field indicates what the reason was that this MPDU frame
			 was allowed to come into the receive path by RXPCU
			<enum 0 rxpcu_filter_pass> This MPDU passed the normal frame
			 filter programming of rxpcu
			<enum 1 rxpcu_monitor_client> This MPDU did NOT pass the
			 regular frame filter and would have been dropped, were 
			it not for the frame fitting into the 'monitor_client' category.
			
			<enum 2 rxpcu_monitor_other> This MPDU did NOT pass the 
			regular frame filter and also did not pass the rxpcu_monitor_client
			 filter. It would have been dropped accept that it did pass
			 the 'monitor_other' category.
			<enum 3 rxpcu_filter_pass_monitor_ovrd> This MPDU passed
			 the normal frame filter programming of RXPCU but additionally
			 fit into the 'monitor_override_client' category.
			<legal 0-3>
*/

#define RX_MSDU_START_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET                          0x0000000000000000
#define RX_MSDU_START_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB                             0
#define RX_MSDU_START_RXPCU_MPDU_FILTER_IN_CATEGORY_MSB                             1
#define RX_MSDU_START_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK                            0x0000000000000003


/* Description		SW_FRAME_GROUP_ID

			SW processes frames based on certain classifications. This
			 field indicates to what sw classification this MPDU is 
			mapped.
			The classification is given in priority order
			
			<enum 0 sw_frame_group_NDP_frame> 
			
			<enum 1 sw_frame_group_Multicast_data> 
			<enum 2 sw_frame_group_Unicast_data> 
			<enum 3 sw_frame_group_Null_data > This includes mpdus of
			 type Data Null.
			Hamilton v1 included QoS Data Null as well here.
			<enum 38 sw_frame_group_QoS_Null_data> This includes QoS
			 Null frames except in UL MU or TB PPDUs.
			<enum 39 sw_frame_group_QoS_Null_data_TB> This includes 
			QoS Null frames in UL MU or TB PPDUs.
			
			<enum 4 sw_frame_group_mgmt_0000 > 
			<enum 5 sw_frame_group_mgmt_0001 > 
			<enum 6 sw_frame_group_mgmt_0010 > 
			<enum 7 sw_frame_group_mgmt_0011 > 
			<enum 8 sw_frame_group_mgmt_0100 > 
			<enum 9 sw_frame_group_mgmt_0101 > 
			<enum 10 sw_frame_group_mgmt_0110 > 
			<enum 11 sw_frame_group_mgmt_0111 > 
			<enum 12 sw_frame_group_mgmt_1000 > 
			<enum 13 sw_frame_group_mgmt_1001 > 
			<enum 14 sw_frame_group_mgmt_1010 > 
			<enum 15 sw_frame_group_mgmt_1011 > 
			<enum 16 sw_frame_group_mgmt_1100 > 
			<enum 17 sw_frame_group_mgmt_1101 > 
			<enum 18 sw_frame_group_mgmt_1110 > 
			<enum 19 sw_frame_group_mgmt_1111 > 
			
			<enum 20 sw_frame_group_ctrl_0000 > 
			<enum 21 sw_frame_group_ctrl_0001 > 
			<enum 22 sw_frame_group_ctrl_0010 > 
			<enum 23 sw_frame_group_ctrl_0011 > 
			<enum 24 sw_frame_group_ctrl_0100 > 
			<enum 25 sw_frame_group_ctrl_0101 > 
			<enum 26 sw_frame_group_ctrl_0110 > 
			<enum 27 sw_frame_group_ctrl_0111 > 
			<enum 28 sw_frame_group_ctrl_1000 > 
			<enum 29 sw_frame_group_ctrl_1001 > 
			<enum 30 sw_frame_group_ctrl_1010 > 
			<enum 31 sw_frame_group_ctrl_1011 > 
			<enum 32 sw_frame_group_ctrl_1100 > 
			<enum 33 sw_frame_group_ctrl_1101 > 
			<enum 34 sw_frame_group_ctrl_1110 > 
			<enum 35 sw_frame_group_ctrl_1111 > 
			
			<enum 36 sw_frame_group_unsupported> This covers type 3 
			and protocol version != 0
			
			<enum 37 sw_frame_group_phy_error> PHY reported an error
			
			
			<legal 0-39>
*/

#define RX_MSDU_START_SW_FRAME_GROUP_ID_OFFSET                                      0x0000000000000000
#define RX_MSDU_START_SW_FRAME_GROUP_ID_LSB                                         2
#define RX_MSDU_START_SW_FRAME_GROUP_ID_MSB                                         8
#define RX_MSDU_START_SW_FRAME_GROUP_ID_MASK                                        0x00000000000001fc


/* Description		RESERVED_0

			<legal 0>
*/

#define RX_MSDU_START_RESERVED_0_OFFSET                                             0x0000000000000000
#define RX_MSDU_START_RESERVED_0_LSB                                                9
#define RX_MSDU_START_RESERVED_0_MSB                                                15
#define RX_MSDU_START_RESERVED_0_MASK                                               0x000000000000fe00


/* Description		PHY_PPDU_ID

			A ppdu counter value that PHY increments for every PPDU 
			received. The counter value wraps around  
			<legal all>
*/

#define RX_MSDU_START_PHY_PPDU_ID_OFFSET                                            0x0000000000000000
#define RX_MSDU_START_PHY_PPDU_ID_LSB                                               16
#define RX_MSDU_START_PHY_PPDU_ID_MSB                                               31
#define RX_MSDU_START_PHY_PPDU_ID_MASK                                              0x00000000ffff0000


/* Description		MSDU_LENGTH

			MSDU length in bytes after decapsulation. 
			
			This field is still valid for MPDU frames without A-MSDU. 
			 It still represents MSDU length after decapsulation
*/

#define RX_MSDU_START_MSDU_LENGTH_OFFSET                                            0x0000000000000000
#define RX_MSDU_START_MSDU_LENGTH_LSB                                               32
#define RX_MSDU_START_MSDU_LENGTH_MSB                                               45
#define RX_MSDU_START_MSDU_LENGTH_MASK                                              0x00003fff00000000


/* Description		STBC

			When set, use STBC transmission rates
*/

#define RX_MSDU_START_STBC_OFFSET                                                   0x0000000000000000
#define RX_MSDU_START_STBC_LSB                                                      46
#define RX_MSDU_START_STBC_MSB                                                      46
#define RX_MSDU_START_STBC_MASK                                                     0x0000400000000000


/* Description		IPSEC_ESP

			Set if IPv4/v6 packet is using IPsec ESP
*/

#define RX_MSDU_START_IPSEC_ESP_OFFSET                                              0x0000000000000000
#define RX_MSDU_START_IPSEC_ESP_LSB                                                 47
#define RX_MSDU_START_IPSEC_ESP_MSB                                                 47
#define RX_MSDU_START_IPSEC_ESP_MASK                                                0x0000800000000000


/* Description		L3_OFFSET

			Depending upon mode bit, this field either indicates the
			 L3 offset in bytes from the start of the RX_HEADER or the
			 IP offset in bytes from the start of the packet after decapsulation. 
			 The latter is only valid if ipv4_proto or ipv6_proto is
			 set.
*/

#define RX_MSDU_START_L3_OFFSET_OFFSET                                              0x0000000000000000
#define RX_MSDU_START_L3_OFFSET_LSB                                                 48
#define RX_MSDU_START_L3_OFFSET_MSB                                                 54
#define RX_MSDU_START_L3_OFFSET_MASK                                                0x007f000000000000


/* Description		IPSEC_AH

			Set if IPv4/v6 packet is using IPsec AH
*/

#define RX_MSDU_START_IPSEC_AH_OFFSET                                               0x0000000000000000
#define RX_MSDU_START_IPSEC_AH_LSB                                                  55
#define RX_MSDU_START_IPSEC_AH_MSB                                                  55
#define RX_MSDU_START_IPSEC_AH_MASK                                                 0x0080000000000000


/* Description		L4_OFFSET

			Depending upon mode bit, this field either indicates the
			 L4 offset nin bytes from the start of RX_HEADER(only valid
			 if either ipv4_proto or ipv6_proto is set to 1) or indicates
			 the offset in bytes to the start of TCP or UDP header from
			 the start of the IP header after decapsulation(Only valid
			 if tcp_proto or udp_proto is set).  The value 0 indicates
			 that the offset is longer than 127 bytes.
*/

#define RX_MSDU_START_L4_OFFSET_OFFSET                                              0x0000000000000000
#define RX_MSDU_START_L4_OFFSET_LSB                                                 56
#define RX_MSDU_START_L4_OFFSET_MSB                                                 63
#define RX_MSDU_START_L4_OFFSET_MASK                                                0xff00000000000000


/* Description		MSDU_NUMBER

			Indicates the MSDU number within a MPDU.  This value is 
			reset to zero at the start of each MPDU.  If the number 
			of MSDU exceeds 255 this number will wrap using modulo 256.
			
*/

#define RX_MSDU_START_MSDU_NUMBER_OFFSET                                            0x0000000000000008
#define RX_MSDU_START_MSDU_NUMBER_LSB                                               0
#define RX_MSDU_START_MSDU_NUMBER_MSB                                               7
#define RX_MSDU_START_MSDU_NUMBER_MASK                                              0x00000000000000ff


/* Description		DECAP_FORMAT

			Indicates the format after decapsulation:
			
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			<legal all>
*/

#define RX_MSDU_START_DECAP_FORMAT_OFFSET                                           0x0000000000000008
#define RX_MSDU_START_DECAP_FORMAT_LSB                                              8
#define RX_MSDU_START_DECAP_FORMAT_MSB                                              9
#define RX_MSDU_START_DECAP_FORMAT_MASK                                             0x0000000000000300


/* Description		IPV4_PROTO

			Set if L2 layer indicates IPv4 protocol.
*/

#define RX_MSDU_START_IPV4_PROTO_OFFSET                                             0x0000000000000008
#define RX_MSDU_START_IPV4_PROTO_LSB                                                10
#define RX_MSDU_START_IPV4_PROTO_MSB                                                10
#define RX_MSDU_START_IPV4_PROTO_MASK                                               0x0000000000000400


/* Description		IPV6_PROTO

			Set if L2 layer indicates IPv6 protocol.
*/

#define RX_MSDU_START_IPV6_PROTO_OFFSET                                             0x0000000000000008
#define RX_MSDU_START_IPV6_PROTO_LSB                                                11
#define RX_MSDU_START_IPV6_PROTO_MSB                                                11
#define RX_MSDU_START_IPV6_PROTO_MASK                                               0x0000000000000800


/* Description		TCP_PROTO

			Set if the ipv4_proto or ipv6_proto are set and the IP protocol
			 indicates TCP.
*/

#define RX_MSDU_START_TCP_PROTO_OFFSET                                              0x0000000000000008
#define RX_MSDU_START_TCP_PROTO_LSB                                                 12
#define RX_MSDU_START_TCP_PROTO_MSB                                                 12
#define RX_MSDU_START_TCP_PROTO_MASK                                                0x0000000000001000


/* Description		UDP_PROTO

			Set if the ipv4_proto or ipv6_proto are set and the IP protocol
			 indicates UDP.
*/

#define RX_MSDU_START_UDP_PROTO_OFFSET                                              0x0000000000000008
#define RX_MSDU_START_UDP_PROTO_LSB                                                 13
#define RX_MSDU_START_UDP_PROTO_MSB                                                 13
#define RX_MSDU_START_UDP_PROTO_MASK                                                0x0000000000002000


/* Description		IP_FRAG

			Indicates that either the IP More frag bit is set or IP 
			frag number is non-zero.  If set indicates that this is 
			a fragmented IP packet.
*/

#define RX_MSDU_START_IP_FRAG_OFFSET                                                0x0000000000000008
#define RX_MSDU_START_IP_FRAG_LSB                                                   14
#define RX_MSDU_START_IP_FRAG_MSB                                                   14
#define RX_MSDU_START_IP_FRAG_MASK                                                  0x0000000000004000


/* Description		TCP_ONLY_ACK

			Set if only the TCP Ack bit is set in the TCP flags and 
			if the TCP payload is 0.
*/

#define RX_MSDU_START_TCP_ONLY_ACK_OFFSET                                           0x0000000000000008
#define RX_MSDU_START_TCP_ONLY_ACK_LSB                                              15
#define RX_MSDU_START_TCP_ONLY_ACK_MSB                                              15
#define RX_MSDU_START_TCP_ONLY_ACK_MASK                                             0x0000000000008000


/* Description		DA_IS_BCAST_MCAST

			The destination address is broadcast or multicast.
*/

#define RX_MSDU_START_DA_IS_BCAST_MCAST_OFFSET                                      0x0000000000000008
#define RX_MSDU_START_DA_IS_BCAST_MCAST_LSB                                         16
#define RX_MSDU_START_DA_IS_BCAST_MCAST_MSB                                         16
#define RX_MSDU_START_DA_IS_BCAST_MCAST_MASK                                        0x0000000000010000


/* Description		TOEPLITZ_HASH_SEL

			Actual choosen Hash.
			
			0 -> Toeplitz hash of 2-tuple (IP source address, IP destination
			 address)1 -> Toeplitz hash of 4-tuple (IP source address, 
			IP destination address, L4 (TCP/UDP) source port, L4 (TCP/UDP) 
			destination port)
			2 -> Toeplitz of flow_id
			3 -> "Zero" is used
			<legal all>
*/

#define RX_MSDU_START_TOEPLITZ_HASH_SEL_OFFSET                                      0x0000000000000008
#define RX_MSDU_START_TOEPLITZ_HASH_SEL_LSB                                         17
#define RX_MSDU_START_TOEPLITZ_HASH_SEL_MSB                                         18
#define RX_MSDU_START_TOEPLITZ_HASH_SEL_MASK                                        0x0000000000060000


/* Description		IP_FIXED_HEADER_VALID

			Fixed 20-byte IPv4 header or 40-byte IPv6 header parsed 
			fully within first 256 bytes of the packet
*/

#define RX_MSDU_START_IP_FIXED_HEADER_VALID_OFFSET                                  0x0000000000000008
#define RX_MSDU_START_IP_FIXED_HEADER_VALID_LSB                                     19
#define RX_MSDU_START_IP_FIXED_HEADER_VALID_MSB                                     19
#define RX_MSDU_START_IP_FIXED_HEADER_VALID_MASK                                    0x0000000000080000


/* Description		IP_EXTN_HEADER_VALID

			IPv6/IPv6 header, including IPv4 options and recognizable
			 extension headers parsed fully within first 256 bytes of
			 the packet
*/

#define RX_MSDU_START_IP_EXTN_HEADER_VALID_OFFSET                                   0x0000000000000008
#define RX_MSDU_START_IP_EXTN_HEADER_VALID_LSB                                      20
#define RX_MSDU_START_IP_EXTN_HEADER_VALID_MSB                                      20
#define RX_MSDU_START_IP_EXTN_HEADER_VALID_MASK                                     0x0000000000100000


/* Description		TCP_UDP_HEADER_VALID

			Fixed 20-byte TCP (excluding TCP options) or 8-byte UDP 
			header parsed fully within first 256 bytes of the packet
			
*/

#define RX_MSDU_START_TCP_UDP_HEADER_VALID_OFFSET                                   0x0000000000000008
#define RX_MSDU_START_TCP_UDP_HEADER_VALID_LSB                                      21
#define RX_MSDU_START_TCP_UDP_HEADER_VALID_MSB                                      21
#define RX_MSDU_START_TCP_UDP_HEADER_VALID_MASK                                     0x0000000000200000


/* Description		MESH_CONTROL_PRESENT

			When set, this MSDU includes the 'Mesh Control' field
			<legal all>
*/

#define RX_MSDU_START_MESH_CONTROL_PRESENT_OFFSET                                   0x0000000000000008
#define RX_MSDU_START_MESH_CONTROL_PRESENT_LSB                                      22
#define RX_MSDU_START_MESH_CONTROL_PRESENT_MSB                                      22
#define RX_MSDU_START_MESH_CONTROL_PRESENT_MASK                                     0x0000000000400000


/* Description		LDPC

			When set, indicates that LDPC coding was used.
			<legal all>
*/

#define RX_MSDU_START_LDPC_OFFSET                                                   0x0000000000000008
#define RX_MSDU_START_LDPC_LSB                                                      23
#define RX_MSDU_START_LDPC_MSB                                                      23
#define RX_MSDU_START_LDPC_MASK                                                     0x0000000000800000


/* Description		IP4_PROTOCOL_IP6_NEXT_HEADER

			For IPv4 this is the 8 bit protocol field (when ipv4_proto
			 is set).  For IPv6 this is the 8 bit next_header field (when
			 ipv6_proto is set).
*/

#define RX_MSDU_START_IP4_PROTOCOL_IP6_NEXT_HEADER_OFFSET                           0x0000000000000008
#define RX_MSDU_START_IP4_PROTOCOL_IP6_NEXT_HEADER_LSB                              24
#define RX_MSDU_START_IP4_PROTOCOL_IP6_NEXT_HEADER_MSB                              31
#define RX_MSDU_START_IP4_PROTOCOL_IP6_NEXT_HEADER_MASK                             0x00000000ff000000


/* Description		TOEPLITZ_HASH_2_OR_4

			Controlled by multiple RxOLE registers for TCP/UDP over 
			IPv4/IPv6 - Either, Toeplitz hash computed over 2-tuple 
			IPv4 or IPv6 src/dest addresses is reported; or, Toeplitz
			 hash computed over 4-tuple IPv4 or IPv6 src/dest addresses
			 and src/dest ports is reported. The Flow_id_toeplitz hash
			 can also be reported here. Usually the hash reported here
			 is the one used for hash-based REO routing (see use_flow_id_toeplitz_clfy
			 in 'RXPT_CLASSIFY_INFO').
			
			In Pine, optionally the 3-tuple Toeplitz hash over IPv4 
			or IPv6 src/dest addresses and L4 protocol can be reported
			 here. (Unsupported in HastingsPrime)
*/

#define RX_MSDU_START_TOEPLITZ_HASH_2_OR_4_OFFSET                                   0x0000000000000008
#define RX_MSDU_START_TOEPLITZ_HASH_2_OR_4_LSB                                      32
#define RX_MSDU_START_TOEPLITZ_HASH_2_OR_4_MSB                                      63
#define RX_MSDU_START_TOEPLITZ_HASH_2_OR_4_MASK                                     0xffffffff00000000


/* Description		FLOW_ID_TOEPLITZ

			Toeplitz hash of 5-tuple 
			{IP source address, IP destination address, IP source port, 
			IP destination port, L4 protocol}  in case of non-IPSec.
			
			In case of IPSec - Toeplitz hash of 4-tuple 
			{IP source address, IP destination address, SPI, L4 protocol}
			
			
			In Pine, optionally the 3-tuple Toeplitz hash over IPv4 
			or IPv6 src/dest addresses and L4 protocol can be reported
			 here. (Unsupported in HastingsPrime)
			
			The relevant Toeplitz key registers are provided in RxOLE's
			 instance of common parser module. These registers are separate
			 from the Toeplitz keys used by ASE/FSE modules inside RxOLE.The
			 actual value will be passed on from common parser module
			 to RxOLE in one of the WHO_* TLVs.
			<legal all>
*/

#define RX_MSDU_START_FLOW_ID_TOEPLITZ_OFFSET                                       0x0000000000000010
#define RX_MSDU_START_FLOW_ID_TOEPLITZ_LSB                                          0
#define RX_MSDU_START_FLOW_ID_TOEPLITZ_MSB                                          31
#define RX_MSDU_START_FLOW_ID_TOEPLITZ_MASK                                         0x00000000ffffffff


/* Description		USER_RSSI

			RSSI for this user
			<legal all>
*/

#define RX_MSDU_START_USER_RSSI_OFFSET                                              0x0000000000000010
#define RX_MSDU_START_USER_RSSI_LSB                                                 32
#define RX_MSDU_START_USER_RSSI_MSB                                                 39
#define RX_MSDU_START_USER_RSSI_MASK                                                0x000000ff00000000


/* Description		PKT_TYPE

			Packet type:
			<enum 0 dot11a>802.11a PPDU type
			<enum 1 dot11b>802.11b PPDU type
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			<enum 3 dot11ac>802.11ac PPDU type
			<enum 4 dot11ax>802.11ax PPDU type
			<enum 5 dot11ba>802.11ba (WUR) PPDU type
			<enum 6 dot11be>802.11be PPDU type
			<enum 7 dot11az>802.11az (ranging) PPDU type
			<enum 8 dot11n_gf>802.11n Green Field PPDU type (unsupported
			 & aborted)
*/

#define RX_MSDU_START_PKT_TYPE_OFFSET                                               0x0000000000000010
#define RX_MSDU_START_PKT_TYPE_LSB                                                  40
#define RX_MSDU_START_PKT_TYPE_MSB                                                  43
#define RX_MSDU_START_PKT_TYPE_MASK                                                 0x00000f0000000000


/* Description		SGI

			Field only valid when pkt type is HT, VHT or HE.
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > HE related GI
			<enum 3     3_2_us_sgi > HE related GI
			<legal 0 - 3>
*/

#define RX_MSDU_START_SGI_OFFSET                                                    0x0000000000000010
#define RX_MSDU_START_SGI_LSB                                                       44
#define RX_MSDU_START_SGI_MSB                                                       45
#define RX_MSDU_START_SGI_MASK                                                      0x0000300000000000


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define RX_MSDU_START_RATE_MCS_OFFSET                                               0x0000000000000010
#define RX_MSDU_START_RATE_MCS_LSB                                                  46
#define RX_MSDU_START_RATE_MCS_MSB                                                  49
#define RX_MSDU_START_RATE_MCS_MASK                                                 0x0003c00000000000


/* Description		RECEIVE_BANDWIDTH

			Full receive Bandwidth
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define RX_MSDU_START_RECEIVE_BANDWIDTH_OFFSET                                      0x0000000000000010
#define RX_MSDU_START_RECEIVE_BANDWIDTH_LSB                                         50
#define RX_MSDU_START_RECEIVE_BANDWIDTH_MSB                                         52
#define RX_MSDU_START_RECEIVE_BANDWIDTH_MASK                                        0x001c000000000000


/* Description		RECEPTION_TYPE

			Indicates what type of reception this is.
			<enum 0     reception_type_SU > Basic SU reception (not 
			part of OFDMA or MIMO)
			<enum 1     reception_type_MU_MIMO > This is related to 
			DL type of reception
			<enum 2     reception_type_MU_OFDMA >  This is related to
			 DL type of reception
			<enum 3     reception_type_MU_OFDMA_MIMO >  This is related
			 to DL type of reception
			<enum 4     reception_type_UL_MU_MIMO > This is related 
			to UL type of reception
			<enum 5     reception_type_UL_MU_OFDMA >  This is related
			 to UL type of reception
			<enum 6     reception_type_UL_MU_OFDMA_MIMO >  This is related
			 to UL type of reception
			
			<legal 0-6>
*/

#define RX_MSDU_START_RECEPTION_TYPE_OFFSET                                         0x0000000000000010
#define RX_MSDU_START_RECEPTION_TYPE_LSB                                            53
#define RX_MSDU_START_RECEPTION_TYPE_MSB                                            55
#define RX_MSDU_START_RECEPTION_TYPE_MASK                                           0x00e0000000000000


/* Description		MIMO_SS_BITMAP

			Field only valid when Reception_type for the MPDU from this
			 STA is some form of MIMO reception
			
			Bitmap, with each bit indicating if the related spatial 
			stream is used for this STA
			LSB related to SS 0
			
			0: spatial stream not used for this reception
			1: spatial stream used for this reception
			
			<legal all>
*/

#define RX_MSDU_START_MIMO_SS_BITMAP_OFFSET                                         0x0000000000000010
#define RX_MSDU_START_MIMO_SS_BITMAP_LSB                                            56
#define RX_MSDU_START_MIMO_SS_BITMAP_MSB                                            63
#define RX_MSDU_START_MIMO_SS_BITMAP_MASK                                           0xff00000000000000


/* Description		PPDU_START_TIMESTAMP_31_0

			Timestamp that indicates when the PPDU that contained this
			 MPDU started on the medium, lower 32 bits
			<legal all>
*/

#define RX_MSDU_START_PPDU_START_TIMESTAMP_31_0_OFFSET                              0x0000000000000018
#define RX_MSDU_START_PPDU_START_TIMESTAMP_31_0_LSB                                 0
#define RX_MSDU_START_PPDU_START_TIMESTAMP_31_0_MSB                                 31
#define RX_MSDU_START_PPDU_START_TIMESTAMP_31_0_MASK                                0x00000000ffffffff


/* Description		PPDU_START_TIMESTAMP_63_32

			Timestamp that indicates when the PPDU that contained this
			 MPDU started on the medium, upper 32 bits
			<legal all>
*/

#define RX_MSDU_START_PPDU_START_TIMESTAMP_63_32_OFFSET                             0x0000000000000018
#define RX_MSDU_START_PPDU_START_TIMESTAMP_63_32_LSB                                32
#define RX_MSDU_START_PPDU_START_TIMESTAMP_63_32_MSB                                63
#define RX_MSDU_START_PPDU_START_TIMESTAMP_63_32_MASK                               0xffffffff00000000


/* Description		SW_PHY_META_DATA

			SW programmed Meta data provided by the PHY.
			
			Can be used for SW to indicate the channel the device is
			 on.
			<legal all>
*/

#define RX_MSDU_START_SW_PHY_META_DATA_OFFSET                                       0x0000000000000020
#define RX_MSDU_START_SW_PHY_META_DATA_LSB                                          0
#define RX_MSDU_START_SW_PHY_META_DATA_MSB                                          31
#define RX_MSDU_START_SW_PHY_META_DATA_MASK                                         0x00000000ffffffff


/* Description		VLAN_CTAG_CI

			2 bytes of C-VLAN Tag Control Information from WHO_L2_LLC
			
*/

#define RX_MSDU_START_VLAN_CTAG_CI_OFFSET                                           0x0000000000000020
#define RX_MSDU_START_VLAN_CTAG_CI_LSB                                              32
#define RX_MSDU_START_VLAN_CTAG_CI_MSB                                              47
#define RX_MSDU_START_VLAN_CTAG_CI_MASK                                             0x0000ffff00000000


/* Description		VLAN_STAG_CI

			2 bytes of S-VLAN Tag Control Information from WHO_L2_LLC
			 in case of double VLAN
*/

#define RX_MSDU_START_VLAN_STAG_CI_OFFSET                                           0x0000000000000020
#define RX_MSDU_START_VLAN_STAG_CI_LSB                                              48
#define RX_MSDU_START_VLAN_STAG_CI_MSB                                              63
#define RX_MSDU_START_VLAN_STAG_CI_MASK                                             0xffff000000000000



#endif   // RX_MSDU_START
