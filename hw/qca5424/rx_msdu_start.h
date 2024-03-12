
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _RX_MSDU_START_H_
#define _RX_MSDU_START_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_MSDU_START 10

#define NUM_OF_QWORDS_RX_MSDU_START 5


struct rx_msdu_start {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rxpcu_mpdu_filter_in_category                           :  2,  
                      sw_frame_group_id                                       :  7,  
                      reserved_0                                              :  7,  
                      phy_ppdu_id                                             : 16;  
             uint32_t msdu_length                                             : 14,  
                      stbc                                                    :  1,  
                      ipsec_esp                                               :  1,  
                      l3_offset                                               :  7,  
                      ipsec_ah                                                :  1,  
                      l4_offset                                               :  8;  
             uint32_t msdu_number                                             :  8,  
                      decap_format                                            :  2,  
                      ipv4_proto                                              :  1,  
                      ipv6_proto                                              :  1,  
                      tcp_proto                                               :  1,  
                      udp_proto                                               :  1,  
                      ip_frag                                                 :  1,  
                      tcp_only_ack                                            :  1,  
                      da_is_bcast_mcast                                       :  1,  
                      toeplitz_hash_sel                                       :  2,  
                      ip_fixed_header_valid                                   :  1,  
                      ip_extn_header_valid                                    :  1,  
                      tcp_udp_header_valid                                    :  1,  
                      mesh_control_present                                    :  1,  
                      ldpc                                                    :  1,  
                      ip4_protocol_ip6_next_header                            :  8;  
             uint32_t toeplitz_hash_2_or_4                                    : 32;  
             uint32_t flow_id_toeplitz                                        : 32;  
             uint32_t user_rssi                                               :  8,  
                      pkt_type                                                :  4,  
                      sgi                                                     :  2,  
                      rate_mcs                                                :  4,  
                      receive_bandwidth                                       :  3,  
                      reception_type                                          :  3,  
                      mimo_ss_bitmap                                          :  8;  
             uint32_t ppdu_start_timestamp_31_0                               : 32;  
             uint32_t ppdu_start_timestamp_63_32                              : 32;  
             uint32_t sw_phy_meta_data                                        : 32;  
             uint32_t vlan_ctag_ci                                            : 16,  
                      vlan_stag_ci                                            : 16;  
#else
             uint32_t phy_ppdu_id                                             : 16,  
                      reserved_0                                              :  7,  
                      sw_frame_group_id                                       :  7,  
                      rxpcu_mpdu_filter_in_category                           :  2;  
             uint32_t l4_offset                                               :  8,  
                      ipsec_ah                                                :  1,  
                      l3_offset                                               :  7,  
                      ipsec_esp                                               :  1,  
                      stbc                                                    :  1,  
                      msdu_length                                             : 14;  
             uint32_t ip4_protocol_ip6_next_header                            :  8,  
                      ldpc                                                    :  1,  
                      mesh_control_present                                    :  1,  
                      tcp_udp_header_valid                                    :  1,  
                      ip_extn_header_valid                                    :  1,  
                      ip_fixed_header_valid                                   :  1,  
                      toeplitz_hash_sel                                       :  2,  
                      da_is_bcast_mcast                                       :  1,  
                      tcp_only_ack                                            :  1,  
                      ip_frag                                                 :  1,  
                      udp_proto                                               :  1,  
                      tcp_proto                                               :  1,  
                      ipv6_proto                                              :  1,  
                      ipv4_proto                                              :  1,  
                      decap_format                                            :  2,  
                      msdu_number                                             :  8;  
             uint32_t toeplitz_hash_2_or_4                                    : 32;  
             uint32_t flow_id_toeplitz                                        : 32;  
             uint32_t mimo_ss_bitmap                                          :  8,  
                      reception_type                                          :  3,  
                      receive_bandwidth                                       :  3,  
                      rate_mcs                                                :  4,  
                      sgi                                                     :  2,  
                      pkt_type                                                :  4,  
                      user_rssi                                               :  8;  
             uint32_t ppdu_start_timestamp_31_0                               : 32;  
             uint32_t ppdu_start_timestamp_63_32                              : 32;  
             uint32_t sw_phy_meta_data                                        : 32;  
             uint32_t vlan_stag_ci                                            : 16,  
                      vlan_ctag_ci                                            : 16;  
#endif
};


 

#define RX_MSDU_START_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET                          0x0000000000000000
#define RX_MSDU_START_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB                             0
#define RX_MSDU_START_RXPCU_MPDU_FILTER_IN_CATEGORY_MSB                             1
#define RX_MSDU_START_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK                            0x0000000000000003


 

#define RX_MSDU_START_SW_FRAME_GROUP_ID_OFFSET                                      0x0000000000000000
#define RX_MSDU_START_SW_FRAME_GROUP_ID_LSB                                         2
#define RX_MSDU_START_SW_FRAME_GROUP_ID_MSB                                         8
#define RX_MSDU_START_SW_FRAME_GROUP_ID_MASK                                        0x00000000000001fc


 

#define RX_MSDU_START_RESERVED_0_OFFSET                                             0x0000000000000000
#define RX_MSDU_START_RESERVED_0_LSB                                                9
#define RX_MSDU_START_RESERVED_0_MSB                                                15
#define RX_MSDU_START_RESERVED_0_MASK                                               0x000000000000fe00


 

#define RX_MSDU_START_PHY_PPDU_ID_OFFSET                                            0x0000000000000000
#define RX_MSDU_START_PHY_PPDU_ID_LSB                                               16
#define RX_MSDU_START_PHY_PPDU_ID_MSB                                               31
#define RX_MSDU_START_PHY_PPDU_ID_MASK                                              0x00000000ffff0000


 

#define RX_MSDU_START_MSDU_LENGTH_OFFSET                                            0x0000000000000000
#define RX_MSDU_START_MSDU_LENGTH_LSB                                               32
#define RX_MSDU_START_MSDU_LENGTH_MSB                                               45
#define RX_MSDU_START_MSDU_LENGTH_MASK                                              0x00003fff00000000


 

#define RX_MSDU_START_STBC_OFFSET                                                   0x0000000000000000
#define RX_MSDU_START_STBC_LSB                                                      46
#define RX_MSDU_START_STBC_MSB                                                      46
#define RX_MSDU_START_STBC_MASK                                                     0x0000400000000000


 

#define RX_MSDU_START_IPSEC_ESP_OFFSET                                              0x0000000000000000
#define RX_MSDU_START_IPSEC_ESP_LSB                                                 47
#define RX_MSDU_START_IPSEC_ESP_MSB                                                 47
#define RX_MSDU_START_IPSEC_ESP_MASK                                                0x0000800000000000


 

#define RX_MSDU_START_L3_OFFSET_OFFSET                                              0x0000000000000000
#define RX_MSDU_START_L3_OFFSET_LSB                                                 48
#define RX_MSDU_START_L3_OFFSET_MSB                                                 54
#define RX_MSDU_START_L3_OFFSET_MASK                                                0x007f000000000000


 

#define RX_MSDU_START_IPSEC_AH_OFFSET                                               0x0000000000000000
#define RX_MSDU_START_IPSEC_AH_LSB                                                  55
#define RX_MSDU_START_IPSEC_AH_MSB                                                  55
#define RX_MSDU_START_IPSEC_AH_MASK                                                 0x0080000000000000


 

#define RX_MSDU_START_L4_OFFSET_OFFSET                                              0x0000000000000000
#define RX_MSDU_START_L4_OFFSET_LSB                                                 56
#define RX_MSDU_START_L4_OFFSET_MSB                                                 63
#define RX_MSDU_START_L4_OFFSET_MASK                                                0xff00000000000000


 

#define RX_MSDU_START_MSDU_NUMBER_OFFSET                                            0x0000000000000008
#define RX_MSDU_START_MSDU_NUMBER_LSB                                               0
#define RX_MSDU_START_MSDU_NUMBER_MSB                                               7
#define RX_MSDU_START_MSDU_NUMBER_MASK                                              0x00000000000000ff


 

#define RX_MSDU_START_DECAP_FORMAT_OFFSET                                           0x0000000000000008
#define RX_MSDU_START_DECAP_FORMAT_LSB                                              8
#define RX_MSDU_START_DECAP_FORMAT_MSB                                              9
#define RX_MSDU_START_DECAP_FORMAT_MASK                                             0x0000000000000300


 

#define RX_MSDU_START_IPV4_PROTO_OFFSET                                             0x0000000000000008
#define RX_MSDU_START_IPV4_PROTO_LSB                                                10
#define RX_MSDU_START_IPV4_PROTO_MSB                                                10
#define RX_MSDU_START_IPV4_PROTO_MASK                                               0x0000000000000400


 

#define RX_MSDU_START_IPV6_PROTO_OFFSET                                             0x0000000000000008
#define RX_MSDU_START_IPV6_PROTO_LSB                                                11
#define RX_MSDU_START_IPV6_PROTO_MSB                                                11
#define RX_MSDU_START_IPV6_PROTO_MASK                                               0x0000000000000800


 

#define RX_MSDU_START_TCP_PROTO_OFFSET                                              0x0000000000000008
#define RX_MSDU_START_TCP_PROTO_LSB                                                 12
#define RX_MSDU_START_TCP_PROTO_MSB                                                 12
#define RX_MSDU_START_TCP_PROTO_MASK                                                0x0000000000001000


 

#define RX_MSDU_START_UDP_PROTO_OFFSET                                              0x0000000000000008
#define RX_MSDU_START_UDP_PROTO_LSB                                                 13
#define RX_MSDU_START_UDP_PROTO_MSB                                                 13
#define RX_MSDU_START_UDP_PROTO_MASK                                                0x0000000000002000


 

#define RX_MSDU_START_IP_FRAG_OFFSET                                                0x0000000000000008
#define RX_MSDU_START_IP_FRAG_LSB                                                   14
#define RX_MSDU_START_IP_FRAG_MSB                                                   14
#define RX_MSDU_START_IP_FRAG_MASK                                                  0x0000000000004000


 

#define RX_MSDU_START_TCP_ONLY_ACK_OFFSET                                           0x0000000000000008
#define RX_MSDU_START_TCP_ONLY_ACK_LSB                                              15
#define RX_MSDU_START_TCP_ONLY_ACK_MSB                                              15
#define RX_MSDU_START_TCP_ONLY_ACK_MASK                                             0x0000000000008000


 

#define RX_MSDU_START_DA_IS_BCAST_MCAST_OFFSET                                      0x0000000000000008
#define RX_MSDU_START_DA_IS_BCAST_MCAST_LSB                                         16
#define RX_MSDU_START_DA_IS_BCAST_MCAST_MSB                                         16
#define RX_MSDU_START_DA_IS_BCAST_MCAST_MASK                                        0x0000000000010000


 

#define RX_MSDU_START_TOEPLITZ_HASH_SEL_OFFSET                                      0x0000000000000008
#define RX_MSDU_START_TOEPLITZ_HASH_SEL_LSB                                         17
#define RX_MSDU_START_TOEPLITZ_HASH_SEL_MSB                                         18
#define RX_MSDU_START_TOEPLITZ_HASH_SEL_MASK                                        0x0000000000060000


 

#define RX_MSDU_START_IP_FIXED_HEADER_VALID_OFFSET                                  0x0000000000000008
#define RX_MSDU_START_IP_FIXED_HEADER_VALID_LSB                                     19
#define RX_MSDU_START_IP_FIXED_HEADER_VALID_MSB                                     19
#define RX_MSDU_START_IP_FIXED_HEADER_VALID_MASK                                    0x0000000000080000


 

#define RX_MSDU_START_IP_EXTN_HEADER_VALID_OFFSET                                   0x0000000000000008
#define RX_MSDU_START_IP_EXTN_HEADER_VALID_LSB                                      20
#define RX_MSDU_START_IP_EXTN_HEADER_VALID_MSB                                      20
#define RX_MSDU_START_IP_EXTN_HEADER_VALID_MASK                                     0x0000000000100000


 

#define RX_MSDU_START_TCP_UDP_HEADER_VALID_OFFSET                                   0x0000000000000008
#define RX_MSDU_START_TCP_UDP_HEADER_VALID_LSB                                      21
#define RX_MSDU_START_TCP_UDP_HEADER_VALID_MSB                                      21
#define RX_MSDU_START_TCP_UDP_HEADER_VALID_MASK                                     0x0000000000200000


 

#define RX_MSDU_START_MESH_CONTROL_PRESENT_OFFSET                                   0x0000000000000008
#define RX_MSDU_START_MESH_CONTROL_PRESENT_LSB                                      22
#define RX_MSDU_START_MESH_CONTROL_PRESENT_MSB                                      22
#define RX_MSDU_START_MESH_CONTROL_PRESENT_MASK                                     0x0000000000400000


 

#define RX_MSDU_START_LDPC_OFFSET                                                   0x0000000000000008
#define RX_MSDU_START_LDPC_LSB                                                      23
#define RX_MSDU_START_LDPC_MSB                                                      23
#define RX_MSDU_START_LDPC_MASK                                                     0x0000000000800000


 

#define RX_MSDU_START_IP4_PROTOCOL_IP6_NEXT_HEADER_OFFSET                           0x0000000000000008
#define RX_MSDU_START_IP4_PROTOCOL_IP6_NEXT_HEADER_LSB                              24
#define RX_MSDU_START_IP4_PROTOCOL_IP6_NEXT_HEADER_MSB                              31
#define RX_MSDU_START_IP4_PROTOCOL_IP6_NEXT_HEADER_MASK                             0x00000000ff000000


 

#define RX_MSDU_START_TOEPLITZ_HASH_2_OR_4_OFFSET                                   0x0000000000000008
#define RX_MSDU_START_TOEPLITZ_HASH_2_OR_4_LSB                                      32
#define RX_MSDU_START_TOEPLITZ_HASH_2_OR_4_MSB                                      63
#define RX_MSDU_START_TOEPLITZ_HASH_2_OR_4_MASK                                     0xffffffff00000000


 

#define RX_MSDU_START_FLOW_ID_TOEPLITZ_OFFSET                                       0x0000000000000010
#define RX_MSDU_START_FLOW_ID_TOEPLITZ_LSB                                          0
#define RX_MSDU_START_FLOW_ID_TOEPLITZ_MSB                                          31
#define RX_MSDU_START_FLOW_ID_TOEPLITZ_MASK                                         0x00000000ffffffff


 

#define RX_MSDU_START_USER_RSSI_OFFSET                                              0x0000000000000010
#define RX_MSDU_START_USER_RSSI_LSB                                                 32
#define RX_MSDU_START_USER_RSSI_MSB                                                 39
#define RX_MSDU_START_USER_RSSI_MASK                                                0x000000ff00000000


 

#define RX_MSDU_START_PKT_TYPE_OFFSET                                               0x0000000000000010
#define RX_MSDU_START_PKT_TYPE_LSB                                                  40
#define RX_MSDU_START_PKT_TYPE_MSB                                                  43
#define RX_MSDU_START_PKT_TYPE_MASK                                                 0x00000f0000000000


 

#define RX_MSDU_START_SGI_OFFSET                                                    0x0000000000000010
#define RX_MSDU_START_SGI_LSB                                                       44
#define RX_MSDU_START_SGI_MSB                                                       45
#define RX_MSDU_START_SGI_MASK                                                      0x0000300000000000


 

#define RX_MSDU_START_RATE_MCS_OFFSET                                               0x0000000000000010
#define RX_MSDU_START_RATE_MCS_LSB                                                  46
#define RX_MSDU_START_RATE_MCS_MSB                                                  49
#define RX_MSDU_START_RATE_MCS_MASK                                                 0x0003c00000000000


 

#define RX_MSDU_START_RECEIVE_BANDWIDTH_OFFSET                                      0x0000000000000010
#define RX_MSDU_START_RECEIVE_BANDWIDTH_LSB                                         50
#define RX_MSDU_START_RECEIVE_BANDWIDTH_MSB                                         52
#define RX_MSDU_START_RECEIVE_BANDWIDTH_MASK                                        0x001c000000000000


 

#define RX_MSDU_START_RECEPTION_TYPE_OFFSET                                         0x0000000000000010
#define RX_MSDU_START_RECEPTION_TYPE_LSB                                            53
#define RX_MSDU_START_RECEPTION_TYPE_MSB                                            55
#define RX_MSDU_START_RECEPTION_TYPE_MASK                                           0x00e0000000000000


 

#define RX_MSDU_START_MIMO_SS_BITMAP_OFFSET                                         0x0000000000000010
#define RX_MSDU_START_MIMO_SS_BITMAP_LSB                                            56
#define RX_MSDU_START_MIMO_SS_BITMAP_MSB                                            63
#define RX_MSDU_START_MIMO_SS_BITMAP_MASK                                           0xff00000000000000


 

#define RX_MSDU_START_PPDU_START_TIMESTAMP_31_0_OFFSET                              0x0000000000000018
#define RX_MSDU_START_PPDU_START_TIMESTAMP_31_0_LSB                                 0
#define RX_MSDU_START_PPDU_START_TIMESTAMP_31_0_MSB                                 31
#define RX_MSDU_START_PPDU_START_TIMESTAMP_31_0_MASK                                0x00000000ffffffff


 

#define RX_MSDU_START_PPDU_START_TIMESTAMP_63_32_OFFSET                             0x0000000000000018
#define RX_MSDU_START_PPDU_START_TIMESTAMP_63_32_LSB                                32
#define RX_MSDU_START_PPDU_START_TIMESTAMP_63_32_MSB                                63
#define RX_MSDU_START_PPDU_START_TIMESTAMP_63_32_MASK                               0xffffffff00000000


 

#define RX_MSDU_START_SW_PHY_META_DATA_OFFSET                                       0x0000000000000020
#define RX_MSDU_START_SW_PHY_META_DATA_LSB                                          0
#define RX_MSDU_START_SW_PHY_META_DATA_MSB                                          31
#define RX_MSDU_START_SW_PHY_META_DATA_MASK                                         0x00000000ffffffff


 

#define RX_MSDU_START_VLAN_CTAG_CI_OFFSET                                           0x0000000000000020
#define RX_MSDU_START_VLAN_CTAG_CI_LSB                                              32
#define RX_MSDU_START_VLAN_CTAG_CI_MSB                                              47
#define RX_MSDU_START_VLAN_CTAG_CI_MASK                                             0x0000ffff00000000


 

#define RX_MSDU_START_VLAN_STAG_CI_OFFSET                                           0x0000000000000020
#define RX_MSDU_START_VLAN_STAG_CI_LSB                                              48
#define RX_MSDU_START_VLAN_STAG_CI_MSB                                              63
#define RX_MSDU_START_VLAN_STAG_CI_MASK                                             0xffff000000000000



#endif    
