
/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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



#ifndef _RX_MSDU_START_H_
#define _RX_MSDU_START_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_MSDU_START 9

struct rx_msdu_start {
             uint32_t rxpcu_mpdu_filter_in_category   :  2,
                      sw_frame_group_id               :  7,
                      reserved_0                      :  7,
                      phy_ppdu_id                     : 16;
             uint32_t msdu_length                     : 14,
                      reserved_1a                     :  1,
                      ipsec_esp                       :  1,
                      l3_offset                       :  7,
                      ipsec_ah                        :  1,
                      l4_offset                       :  8;
             uint32_t msdu_number                     :  8,
                      decap_format                    :  2,
                      ipv4_proto                      :  1,
                      ipv6_proto                      :  1,
                      tcp_proto                       :  1,
                      udp_proto                       :  1,
                      ip_frag                         :  1,
                      tcp_only_ack                    :  1,
                      da_is_bcast_mcast               :  1,
                      toeplitz_hash_sel               :  2,
                      ip_fixed_header_valid           :  1,
                      ip_extn_header_valid            :  1,
                      tcp_udp_header_valid            :  1,
                      mesh_control_present            :  1,
                      ldpc                            :  1,
                      ip4_protocol_ip6_next_header    :  8;
             uint32_t toeplitz_hash_2_or_4            : 32;
             uint32_t flow_id_toeplitz                : 32;
             uint32_t user_rssi                       :  8,
                      pkt_type                        :  4,
                      stbc                            :  1,
                      sgi                             :  2,
                      rate_mcs                        :  4,
                      receive_bandwidth               :  2,
                      reception_type                  :  3,
                      mimo_ss_bitmap                  :  8;
             uint32_t ppdu_start_timestamp            : 32;
             uint32_t sw_phy_meta_data                : 32;
             uint32_t vlan_ctag_ci                    : 16,
                      vlan_stag_ci                    : 16;
};

#define RX_MSDU_START_0_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET         0x00000000
#define RX_MSDU_START_0_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB            0
#define RX_MSDU_START_0_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK           0x00000003

#define RX_MSDU_START_0_SW_FRAME_GROUP_ID_OFFSET                     0x00000000
#define RX_MSDU_START_0_SW_FRAME_GROUP_ID_LSB                        2
#define RX_MSDU_START_0_SW_FRAME_GROUP_ID_MASK                       0x000001fc

#define RX_MSDU_START_0_RESERVED_0_OFFSET                            0x00000000
#define RX_MSDU_START_0_RESERVED_0_LSB                               9
#define RX_MSDU_START_0_RESERVED_0_MASK                              0x0000fe00

#define RX_MSDU_START_0_PHY_PPDU_ID_OFFSET                           0x00000000
#define RX_MSDU_START_0_PHY_PPDU_ID_LSB                              16
#define RX_MSDU_START_0_PHY_PPDU_ID_MASK                             0xffff0000

#define RX_MSDU_START_1_MSDU_LENGTH_OFFSET                           0x00000004
#define RX_MSDU_START_1_MSDU_LENGTH_LSB                              0
#define RX_MSDU_START_1_MSDU_LENGTH_MASK                             0x00003fff

#define RX_MSDU_START_1_RESERVED_1A_OFFSET                           0x00000004
#define RX_MSDU_START_1_RESERVED_1A_LSB                              14
#define RX_MSDU_START_1_RESERVED_1A_MASK                             0x00004000

#define RX_MSDU_START_1_IPSEC_ESP_OFFSET                             0x00000004
#define RX_MSDU_START_1_IPSEC_ESP_LSB                                15
#define RX_MSDU_START_1_IPSEC_ESP_MASK                               0x00008000

#define RX_MSDU_START_1_L3_OFFSET_OFFSET                             0x00000004
#define RX_MSDU_START_1_L3_OFFSET_LSB                                16
#define RX_MSDU_START_1_L3_OFFSET_MASK                               0x007f0000

#define RX_MSDU_START_1_IPSEC_AH_OFFSET                              0x00000004
#define RX_MSDU_START_1_IPSEC_AH_LSB                                 23
#define RX_MSDU_START_1_IPSEC_AH_MASK                                0x00800000

#define RX_MSDU_START_1_L4_OFFSET_OFFSET                             0x00000004
#define RX_MSDU_START_1_L4_OFFSET_LSB                                24
#define RX_MSDU_START_1_L4_OFFSET_MASK                               0xff000000

#define RX_MSDU_START_2_MSDU_NUMBER_OFFSET                           0x00000008
#define RX_MSDU_START_2_MSDU_NUMBER_LSB                              0
#define RX_MSDU_START_2_MSDU_NUMBER_MASK                             0x000000ff

#define RX_MSDU_START_2_DECAP_FORMAT_OFFSET                          0x00000008
#define RX_MSDU_START_2_DECAP_FORMAT_LSB                             8
#define RX_MSDU_START_2_DECAP_FORMAT_MASK                            0x00000300

#define RX_MSDU_START_2_IPV4_PROTO_OFFSET                            0x00000008
#define RX_MSDU_START_2_IPV4_PROTO_LSB                               10
#define RX_MSDU_START_2_IPV4_PROTO_MASK                              0x00000400

#define RX_MSDU_START_2_IPV6_PROTO_OFFSET                            0x00000008
#define RX_MSDU_START_2_IPV6_PROTO_LSB                               11
#define RX_MSDU_START_2_IPV6_PROTO_MASK                              0x00000800

#define RX_MSDU_START_2_TCP_PROTO_OFFSET                             0x00000008
#define RX_MSDU_START_2_TCP_PROTO_LSB                                12
#define RX_MSDU_START_2_TCP_PROTO_MASK                               0x00001000

#define RX_MSDU_START_2_UDP_PROTO_OFFSET                             0x00000008
#define RX_MSDU_START_2_UDP_PROTO_LSB                                13
#define RX_MSDU_START_2_UDP_PROTO_MASK                               0x00002000

#define RX_MSDU_START_2_IP_FRAG_OFFSET                               0x00000008
#define RX_MSDU_START_2_IP_FRAG_LSB                                  14
#define RX_MSDU_START_2_IP_FRAG_MASK                                 0x00004000

#define RX_MSDU_START_2_TCP_ONLY_ACK_OFFSET                          0x00000008
#define RX_MSDU_START_2_TCP_ONLY_ACK_LSB                             15
#define RX_MSDU_START_2_TCP_ONLY_ACK_MASK                            0x00008000

#define RX_MSDU_START_2_DA_IS_BCAST_MCAST_OFFSET                     0x00000008
#define RX_MSDU_START_2_DA_IS_BCAST_MCAST_LSB                        16
#define RX_MSDU_START_2_DA_IS_BCAST_MCAST_MASK                       0x00010000

#define RX_MSDU_START_2_TOEPLITZ_HASH_SEL_OFFSET                     0x00000008
#define RX_MSDU_START_2_TOEPLITZ_HASH_SEL_LSB                        17
#define RX_MSDU_START_2_TOEPLITZ_HASH_SEL_MASK                       0x00060000

#define RX_MSDU_START_2_IP_FIXED_HEADER_VALID_OFFSET                 0x00000008
#define RX_MSDU_START_2_IP_FIXED_HEADER_VALID_LSB                    19
#define RX_MSDU_START_2_IP_FIXED_HEADER_VALID_MASK                   0x00080000

#define RX_MSDU_START_2_IP_EXTN_HEADER_VALID_OFFSET                  0x00000008
#define RX_MSDU_START_2_IP_EXTN_HEADER_VALID_LSB                     20
#define RX_MSDU_START_2_IP_EXTN_HEADER_VALID_MASK                    0x00100000

#define RX_MSDU_START_2_TCP_UDP_HEADER_VALID_OFFSET                  0x00000008
#define RX_MSDU_START_2_TCP_UDP_HEADER_VALID_LSB                     21
#define RX_MSDU_START_2_TCP_UDP_HEADER_VALID_MASK                    0x00200000

#define RX_MSDU_START_2_MESH_CONTROL_PRESENT_OFFSET                  0x00000008
#define RX_MSDU_START_2_MESH_CONTROL_PRESENT_LSB                     22
#define RX_MSDU_START_2_MESH_CONTROL_PRESENT_MASK                    0x00400000

#define RX_MSDU_START_2_LDPC_OFFSET                                  0x00000008
#define RX_MSDU_START_2_LDPC_LSB                                     23
#define RX_MSDU_START_2_LDPC_MASK                                    0x00800000

#define RX_MSDU_START_2_IP4_PROTOCOL_IP6_NEXT_HEADER_OFFSET          0x00000008
#define RX_MSDU_START_2_IP4_PROTOCOL_IP6_NEXT_HEADER_LSB             24
#define RX_MSDU_START_2_IP4_PROTOCOL_IP6_NEXT_HEADER_MASK            0xff000000

#define RX_MSDU_START_3_TOEPLITZ_HASH_2_OR_4_OFFSET                  0x0000000c
#define RX_MSDU_START_3_TOEPLITZ_HASH_2_OR_4_LSB                     0
#define RX_MSDU_START_3_TOEPLITZ_HASH_2_OR_4_MASK                    0xffffffff

#define RX_MSDU_START_4_FLOW_ID_TOEPLITZ_OFFSET                      0x00000010
#define RX_MSDU_START_4_FLOW_ID_TOEPLITZ_LSB                         0
#define RX_MSDU_START_4_FLOW_ID_TOEPLITZ_MASK                        0xffffffff

#define RX_MSDU_START_5_USER_RSSI_OFFSET                             0x00000014
#define RX_MSDU_START_5_USER_RSSI_LSB                                0
#define RX_MSDU_START_5_USER_RSSI_MASK                               0x000000ff

#define RX_MSDU_START_5_PKT_TYPE_OFFSET                              0x00000014
#define RX_MSDU_START_5_PKT_TYPE_LSB                                 8
#define RX_MSDU_START_5_PKT_TYPE_MASK                                0x00000f00

#define RX_MSDU_START_5_STBC_OFFSET                                  0x00000014
#define RX_MSDU_START_5_STBC_LSB                                     12
#define RX_MSDU_START_5_STBC_MASK                                    0x00001000

#define RX_MSDU_START_5_SGI_OFFSET                                   0x00000014
#define RX_MSDU_START_5_SGI_LSB                                      13
#define RX_MSDU_START_5_SGI_MASK                                     0x00006000

#define RX_MSDU_START_5_RATE_MCS_OFFSET                              0x00000014
#define RX_MSDU_START_5_RATE_MCS_LSB                                 15
#define RX_MSDU_START_5_RATE_MCS_MASK                                0x00078000

#define RX_MSDU_START_5_RECEIVE_BANDWIDTH_OFFSET                     0x00000014
#define RX_MSDU_START_5_RECEIVE_BANDWIDTH_LSB                        19
#define RX_MSDU_START_5_RECEIVE_BANDWIDTH_MASK                       0x00180000

#define RX_MSDU_START_5_RECEPTION_TYPE_OFFSET                        0x00000014
#define RX_MSDU_START_5_RECEPTION_TYPE_LSB                           21
#define RX_MSDU_START_5_RECEPTION_TYPE_MASK                          0x00e00000

#define RX_MSDU_START_5_MIMO_SS_BITMAP_OFFSET                        0x00000014
#define RX_MSDU_START_5_MIMO_SS_BITMAP_LSB                           24
#define RX_MSDU_START_5_MIMO_SS_BITMAP_MASK                          0xff000000

#define RX_MSDU_START_6_PPDU_START_TIMESTAMP_OFFSET                  0x00000018
#define RX_MSDU_START_6_PPDU_START_TIMESTAMP_LSB                     0
#define RX_MSDU_START_6_PPDU_START_TIMESTAMP_MASK                    0xffffffff

#define RX_MSDU_START_7_SW_PHY_META_DATA_OFFSET                      0x0000001c
#define RX_MSDU_START_7_SW_PHY_META_DATA_LSB                         0
#define RX_MSDU_START_7_SW_PHY_META_DATA_MASK                        0xffffffff

#define RX_MSDU_START_8_VLAN_CTAG_CI_OFFSET                          0x00000020
#define RX_MSDU_START_8_VLAN_CTAG_CI_LSB                             0
#define RX_MSDU_START_8_VLAN_CTAG_CI_MASK                            0x0000ffff

#define RX_MSDU_START_8_VLAN_STAG_CI_OFFSET                          0x00000020
#define RX_MSDU_START_8_VLAN_STAG_CI_LSB                             16
#define RX_MSDU_START_8_VLAN_STAG_CI_MASK                            0xffff0000

#endif
