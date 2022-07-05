
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

 
 
 
 
 
 
 


#ifndef _RX_MSDU_END_H_
#define _RX_MSDU_END_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_MSDU_END 32

#define NUM_OF_QWORDS_RX_MSDU_END 16


struct rx_msdu_end {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rxpcu_mpdu_filter_in_category                           :  2,  
                      sw_frame_group_id                                       :  7,  
                      reserved_0                                              :  7,  
                      phy_ppdu_id                                             : 16;  
             uint32_t ip_hdr_chksum                                           : 16,  
                      reported_mpdu_length                                    : 14,  
                      reserved_1a                                             :  2;  
             uint32_t reserved_2a                                             :  8,  
                      cce_super_rule                                          :  6,  
                      cce_classify_not_done_truncate                          :  1,  
                      cce_classify_not_done_cce_dis                           :  1,  
                      cumulative_l3_checksum                                  : 16;  
             uint32_t rule_indication_31_0                                    : 32;  
             uint32_t ipv6_options_crc                                        : 32;  
             uint32_t da_offset                                               :  6,  
                      sa_offset                                               :  6,  
                      da_offset_valid                                         :  1,  
                      sa_offset_valid                                         :  1,  
                      reserved_5a                                             :  2,  
                      l3_type                                                 : 16;  
             uint32_t rule_indication_63_32                                   : 32;  
             uint32_t tcp_seq_number                                          : 32;  
             uint32_t tcp_ack_number                                          : 32;  
             uint32_t tcp_flag                                                :  9,  
                      lro_eligible                                            :  1,  
                      reserved_9a                                             :  6,  
                      window_size                                             : 16;  
             uint32_t sa_sw_peer_id                                           : 16,  
                      sa_idx_timeout                                          :  1,  
                      da_idx_timeout                                          :  1,  
                      to_ds                                                   :  1,  
                      tid                                                     :  4,  
                      sa_is_valid                                             :  1,  
                      da_is_valid                                             :  1,  
                      da_is_mcbc                                              :  1,  
                      l3_header_padding                                       :  2,  
                      first_msdu                                              :  1,  
                      last_msdu                                               :  1,  
                      fr_ds                                                   :  1,  
                      ip_chksum_fail_copy                                     :  1;  
             uint32_t sa_idx                                                  : 16,  
                      da_idx_or_sw_peer_id                                    : 16;  
             uint32_t msdu_drop                                               :  1,  
                      reo_destination_indication                              :  5,  
                      flow_idx                                                : 20,  
                      use_ppe                                                 :  1,  
                      mesh_sta                                                :  2,  
                      vlan_ctag_stripped                                      :  1,  
                      vlan_stag_stripped                                      :  1,  
                      fragment_flag                                           :  1;  
             uint32_t fse_metadata                                            : 32;  
             uint32_t cce_metadata                                            : 16,  
                      tcp_udp_chksum                                          : 16;  
             uint32_t aggregation_count                                       :  8,  
                      flow_aggregation_continuation                           :  1,  
                      fisa_timeout                                            :  1,  
                      tcp_udp_chksum_fail_copy                                :  1,  
                      msdu_limit_error                                        :  1,  
                      flow_idx_timeout                                        :  1,  
                      flow_idx_invalid                                        :  1,  
                      cce_match                                               :  1,  
                      amsdu_parser_error                                      :  1,  
                      cumulative_ip_length                                    : 16;  
             uint32_t key_id_octet                                            :  8,  
                      reserved_16a                                            : 24;  
             uint32_t reserved_17a                                            :  6,  
                      service_code                                            :  9,  
                      priority_valid                                          :  1,  
                      intra_bss                                               :  1,  
                      dest_chip_id                                            :  2,  
                      multicast_echo                                          :  1,  
                      wds_learning_event                                      :  1,  
                      wds_roaming_event                                       :  1,  
                      wds_keep_alive_event                                    :  1,  
                      dest_chip_pmac_id                                       :  1,  
                      reserved_17b                                            :  8;  
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
             uint32_t vlan_ctag_ci                                            : 16,  
                      vlan_stag_ci                                            : 16;  
             uint32_t peer_meta_data                                          : 32;  
             uint32_t user_rssi                                               :  8,  
                      pkt_type                                                :  4,  
                      sgi                                                     :  2,  
                      rate_mcs                                                :  4,  
                      receive_bandwidth                                       :  3,  
                      reception_type                                          :  3,  
                      mimo_ss_bitmap                                          :  7,  
                      msdu_done_copy                                          :  1;  
             uint32_t flow_id_toeplitz                                        : 32;  
             uint32_t ppdu_start_timestamp_63_32                              : 32;  
             uint32_t sw_phy_meta_data                                        : 32;  
             uint32_t ppdu_start_timestamp_31_0                               : 32;  
             uint32_t toeplitz_hash_2_or_4                                    : 32;  
             uint32_t reserved_28a                                            : 16,  
                      sa_15_0                                                 : 16;  
             uint32_t sa_47_16                                                : 32;  
             uint32_t first_mpdu                                              :  1,  
                      reserved_30a                                            :  1,  
                      mcast_bcast                                             :  1,  
                      ast_index_not_found                                     :  1,  
                      ast_index_timeout                                       :  1,  
                      power_mgmt                                              :  1,  
                      non_qos                                                 :  1,  
                      null_data                                               :  1,  
                      mgmt_type                                               :  1,  
                      ctrl_type                                               :  1,  
                      more_data                                               :  1,  
                      eosp                                                    :  1,  
                      a_msdu_error                                            :  1,  
                      reserved_30b                                            :  1,  
                      order                                                   :  1,  
                      wifi_parser_error                                       :  1,  
                      overflow_err                                            :  1,  
                      msdu_length_err                                         :  1,  
                      tcp_udp_chksum_fail                                     :  1,  
                      ip_chksum_fail                                          :  1,  
                      sa_idx_invalid                                          :  1,  
                      da_idx_invalid                                          :  1,  
                      amsdu_addr_mismatch                                     :  1,  
                      rx_in_tx_decrypt_byp                                    :  1,  
                      encrypt_required                                        :  1,  
                      directed                                                :  1,  
                      buffer_fragment                                         :  1,  
                      mpdu_length_err                                         :  1,  
                      tkip_mic_err                                            :  1,  
                      decrypt_err                                             :  1,  
                      unencrypted_frame_err                                   :  1,  
                      fcs_err                                                 :  1;  
             uint32_t reserved_31a                                            : 10,  
                      decrypt_status_code                                     :  3,  
                      rx_bitmap_not_updated                                   :  1,  
                      reserved_31b                                            : 17,  
                      msdu_done                                               :  1;  
#else
             uint32_t phy_ppdu_id                                             : 16,  
                      reserved_0                                              :  7,  
                      sw_frame_group_id                                       :  7,  
                      rxpcu_mpdu_filter_in_category                           :  2;  
             uint32_t reserved_1a                                             :  2,  
                      reported_mpdu_length                                    : 14,  
                      ip_hdr_chksum                                           : 16;  
             uint32_t cumulative_l3_checksum                                  : 16,  
                      cce_classify_not_done_cce_dis                           :  1,  
                      cce_classify_not_done_truncate                          :  1,  
                      cce_super_rule                                          :  6,  
                      reserved_2a                                             :  8;  
             uint32_t rule_indication_31_0                                    : 32;  
             uint32_t ipv6_options_crc                                        : 32;  
             uint32_t l3_type                                                 : 16,  
                      reserved_5a                                             :  2,  
                      sa_offset_valid                                         :  1,  
                      da_offset_valid                                         :  1,  
                      sa_offset                                               :  6,  
                      da_offset                                               :  6;  
             uint32_t rule_indication_63_32                                   : 32;  
             uint32_t tcp_seq_number                                          : 32;  
             uint32_t tcp_ack_number                                          : 32;  
             uint32_t window_size                                             : 16,  
                      reserved_9a                                             :  6,  
                      lro_eligible                                            :  1,  
                      tcp_flag                                                :  9;  
             uint32_t ip_chksum_fail_copy                                     :  1,  
                      fr_ds                                                   :  1,  
                      last_msdu                                               :  1,  
                      first_msdu                                              :  1,  
                      l3_header_padding                                       :  2,  
                      da_is_mcbc                                              :  1,  
                      da_is_valid                                             :  1,  
                      sa_is_valid                                             :  1,  
                      tid                                                     :  4,  
                      to_ds                                                   :  1,  
                      da_idx_timeout                                          :  1,  
                      sa_idx_timeout                                          :  1,  
                      sa_sw_peer_id                                           : 16;  
             uint32_t da_idx_or_sw_peer_id                                    : 16,  
                      sa_idx                                                  : 16;  
             uint32_t fragment_flag                                           :  1,  
                      vlan_stag_stripped                                      :  1,  
                      vlan_ctag_stripped                                      :  1,  
                      mesh_sta                                                :  2,  
                      use_ppe                                                 :  1,  
                      flow_idx                                                : 20,  
                      reo_destination_indication                              :  5,  
                      msdu_drop                                               :  1;  
             uint32_t fse_metadata                                            : 32;  
             uint32_t tcp_udp_chksum                                          : 16,  
                      cce_metadata                                            : 16;  
             uint32_t cumulative_ip_length                                    : 16,  
                      amsdu_parser_error                                      :  1,  
                      cce_match                                               :  1,  
                      flow_idx_invalid                                        :  1,  
                      flow_idx_timeout                                        :  1,  
                      msdu_limit_error                                        :  1,  
                      tcp_udp_chksum_fail_copy                                :  1,  
                      fisa_timeout                                            :  1,  
                      flow_aggregation_continuation                           :  1,  
                      aggregation_count                                       :  8;  
             uint32_t reserved_16a                                            : 24,  
                      key_id_octet                                            :  8;  
             uint32_t reserved_17b                                            :  8,  
                      dest_chip_pmac_id                                       :  1,  
                      wds_keep_alive_event                                    :  1,  
                      wds_roaming_event                                       :  1,  
                      wds_learning_event                                      :  1,  
                      multicast_echo                                          :  1,  
                      dest_chip_id                                            :  2,  
                      intra_bss                                               :  1,  
                      priority_valid                                          :  1,  
                      service_code                                            :  9,  
                      reserved_17a                                            :  6;  
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
             uint32_t vlan_stag_ci                                            : 16,  
                      vlan_ctag_ci                                            : 16;  
             uint32_t peer_meta_data                                          : 32;  
             uint32_t msdu_done_copy                                          :  1,  
                      mimo_ss_bitmap                                          :  7,  
                      reception_type                                          :  3,  
                      receive_bandwidth                                       :  3,  
                      rate_mcs                                                :  4,  
                      sgi                                                     :  2,  
                      pkt_type                                                :  4,  
                      user_rssi                                               :  8;  
             uint32_t flow_id_toeplitz                                        : 32;  
             uint32_t ppdu_start_timestamp_63_32                              : 32;  
             uint32_t sw_phy_meta_data                                        : 32;  
             uint32_t ppdu_start_timestamp_31_0                               : 32;  
             uint32_t toeplitz_hash_2_or_4                                    : 32;  
             uint32_t sa_15_0                                                 : 16,  
                      reserved_28a                                            : 16;  
             uint32_t sa_47_16                                                : 32;  
             uint32_t fcs_err                                                 :  1,  
                      unencrypted_frame_err                                   :  1,  
                      decrypt_err                                             :  1,  
                      tkip_mic_err                                            :  1,  
                      mpdu_length_err                                         :  1,  
                      buffer_fragment                                         :  1,  
                      directed                                                :  1,  
                      encrypt_required                                        :  1,  
                      rx_in_tx_decrypt_byp                                    :  1,  
                      amsdu_addr_mismatch                                     :  1,  
                      da_idx_invalid                                          :  1,  
                      sa_idx_invalid                                          :  1,  
                      ip_chksum_fail                                          :  1,  
                      tcp_udp_chksum_fail                                     :  1,  
                      msdu_length_err                                         :  1,  
                      overflow_err                                            :  1,  
                      wifi_parser_error                                       :  1,  
                      order                                                   :  1,  
                      reserved_30b                                            :  1,  
                      a_msdu_error                                            :  1,  
                      eosp                                                    :  1,  
                      more_data                                               :  1,  
                      ctrl_type                                               :  1,  
                      mgmt_type                                               :  1,  
                      null_data                                               :  1,  
                      non_qos                                                 :  1,  
                      power_mgmt                                              :  1,  
                      ast_index_timeout                                       :  1,  
                      ast_index_not_found                                     :  1,  
                      mcast_bcast                                             :  1,  
                      reserved_30a                                            :  1,  
                      first_mpdu                                              :  1;  
             uint32_t msdu_done                                               :  1,  
                      reserved_31b                                            : 17,  
                      rx_bitmap_not_updated                                   :  1,  
                      decrypt_status_code                                     :  3,  
                      reserved_31a                                            : 10;  
#endif
};


 

#define RX_MSDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET                            0x0000000000000000
#define RX_MSDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB                               0
#define RX_MSDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_MSB                               1
#define RX_MSDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK                              0x0000000000000003


 

#define RX_MSDU_END_SW_FRAME_GROUP_ID_OFFSET                                        0x0000000000000000
#define RX_MSDU_END_SW_FRAME_GROUP_ID_LSB                                           2
#define RX_MSDU_END_SW_FRAME_GROUP_ID_MSB                                           8
#define RX_MSDU_END_SW_FRAME_GROUP_ID_MASK                                          0x00000000000001fc


 

#define RX_MSDU_END_RESERVED_0_OFFSET                                               0x0000000000000000
#define RX_MSDU_END_RESERVED_0_LSB                                                  9
#define RX_MSDU_END_RESERVED_0_MSB                                                  15
#define RX_MSDU_END_RESERVED_0_MASK                                                 0x000000000000fe00


 

#define RX_MSDU_END_PHY_PPDU_ID_OFFSET                                              0x0000000000000000
#define RX_MSDU_END_PHY_PPDU_ID_LSB                                                 16
#define RX_MSDU_END_PHY_PPDU_ID_MSB                                                 31
#define RX_MSDU_END_PHY_PPDU_ID_MASK                                                0x00000000ffff0000


 

#define RX_MSDU_END_IP_HDR_CHKSUM_OFFSET                                            0x0000000000000000
#define RX_MSDU_END_IP_HDR_CHKSUM_LSB                                               32
#define RX_MSDU_END_IP_HDR_CHKSUM_MSB                                               47
#define RX_MSDU_END_IP_HDR_CHKSUM_MASK                                              0x0000ffff00000000


 

#define RX_MSDU_END_REPORTED_MPDU_LENGTH_OFFSET                                     0x0000000000000000
#define RX_MSDU_END_REPORTED_MPDU_LENGTH_LSB                                        48
#define RX_MSDU_END_REPORTED_MPDU_LENGTH_MSB                                        61
#define RX_MSDU_END_REPORTED_MPDU_LENGTH_MASK                                       0x3fff000000000000


 

#define RX_MSDU_END_RESERVED_1A_OFFSET                                              0x0000000000000000
#define RX_MSDU_END_RESERVED_1A_LSB                                                 62
#define RX_MSDU_END_RESERVED_1A_MSB                                                 63
#define RX_MSDU_END_RESERVED_1A_MASK                                                0xc000000000000000


 

#define RX_MSDU_END_RESERVED_2A_OFFSET                                              0x0000000000000008
#define RX_MSDU_END_RESERVED_2A_LSB                                                 0
#define RX_MSDU_END_RESERVED_2A_MSB                                                 7
#define RX_MSDU_END_RESERVED_2A_MASK                                                0x00000000000000ff


 

#define RX_MSDU_END_CCE_SUPER_RULE_OFFSET                                           0x0000000000000008
#define RX_MSDU_END_CCE_SUPER_RULE_LSB                                              8
#define RX_MSDU_END_CCE_SUPER_RULE_MSB                                              13
#define RX_MSDU_END_CCE_SUPER_RULE_MASK                                             0x0000000000003f00


 

#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_TRUNCATE_OFFSET                           0x0000000000000008
#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_TRUNCATE_LSB                              14
#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_TRUNCATE_MSB                              14
#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_TRUNCATE_MASK                             0x0000000000004000


 

#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_CCE_DIS_OFFSET                            0x0000000000000008
#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_CCE_DIS_LSB                               15
#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_CCE_DIS_MSB                               15
#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_CCE_DIS_MASK                              0x0000000000008000


 

#define RX_MSDU_END_CUMULATIVE_L3_CHECKSUM_OFFSET                                   0x0000000000000008
#define RX_MSDU_END_CUMULATIVE_L3_CHECKSUM_LSB                                      16
#define RX_MSDU_END_CUMULATIVE_L3_CHECKSUM_MSB                                      31
#define RX_MSDU_END_CUMULATIVE_L3_CHECKSUM_MASK                                     0x00000000ffff0000


 

#define RX_MSDU_END_RULE_INDICATION_31_0_OFFSET                                     0x0000000000000008
#define RX_MSDU_END_RULE_INDICATION_31_0_LSB                                        32
#define RX_MSDU_END_RULE_INDICATION_31_0_MSB                                        63
#define RX_MSDU_END_RULE_INDICATION_31_0_MASK                                       0xffffffff00000000


 

#define RX_MSDU_END_IPV6_OPTIONS_CRC_OFFSET                                         0x0000000000000010
#define RX_MSDU_END_IPV6_OPTIONS_CRC_LSB                                            0
#define RX_MSDU_END_IPV6_OPTIONS_CRC_MSB                                            31
#define RX_MSDU_END_IPV6_OPTIONS_CRC_MASK                                           0x00000000ffffffff


 

#define RX_MSDU_END_DA_OFFSET_OFFSET                                                0x0000000000000010
#define RX_MSDU_END_DA_OFFSET_LSB                                                   32
#define RX_MSDU_END_DA_OFFSET_MSB                                                   37
#define RX_MSDU_END_DA_OFFSET_MASK                                                  0x0000003f00000000


 

#define RX_MSDU_END_SA_OFFSET_OFFSET                                                0x0000000000000010
#define RX_MSDU_END_SA_OFFSET_LSB                                                   38
#define RX_MSDU_END_SA_OFFSET_MSB                                                   43
#define RX_MSDU_END_SA_OFFSET_MASK                                                  0x00000fc000000000


 

#define RX_MSDU_END_DA_OFFSET_VALID_OFFSET                                          0x0000000000000010
#define RX_MSDU_END_DA_OFFSET_VALID_LSB                                             44
#define RX_MSDU_END_DA_OFFSET_VALID_MSB                                             44
#define RX_MSDU_END_DA_OFFSET_VALID_MASK                                            0x0000100000000000


 

#define RX_MSDU_END_SA_OFFSET_VALID_OFFSET                                          0x0000000000000010
#define RX_MSDU_END_SA_OFFSET_VALID_LSB                                             45
#define RX_MSDU_END_SA_OFFSET_VALID_MSB                                             45
#define RX_MSDU_END_SA_OFFSET_VALID_MASK                                            0x0000200000000000


 

#define RX_MSDU_END_RESERVED_5A_OFFSET                                              0x0000000000000010
#define RX_MSDU_END_RESERVED_5A_LSB                                                 46
#define RX_MSDU_END_RESERVED_5A_MSB                                                 47
#define RX_MSDU_END_RESERVED_5A_MASK                                                0x0000c00000000000


 

#define RX_MSDU_END_L3_TYPE_OFFSET                                                  0x0000000000000010
#define RX_MSDU_END_L3_TYPE_LSB                                                     48
#define RX_MSDU_END_L3_TYPE_MSB                                                     63
#define RX_MSDU_END_L3_TYPE_MASK                                                    0xffff000000000000


 

#define RX_MSDU_END_RULE_INDICATION_63_32_OFFSET                                    0x0000000000000018
#define RX_MSDU_END_RULE_INDICATION_63_32_LSB                                       0
#define RX_MSDU_END_RULE_INDICATION_63_32_MSB                                       31
#define RX_MSDU_END_RULE_INDICATION_63_32_MASK                                      0x00000000ffffffff


 

#define RX_MSDU_END_TCP_SEQ_NUMBER_OFFSET                                           0x0000000000000018
#define RX_MSDU_END_TCP_SEQ_NUMBER_LSB                                              32
#define RX_MSDU_END_TCP_SEQ_NUMBER_MSB                                              63
#define RX_MSDU_END_TCP_SEQ_NUMBER_MASK                                             0xffffffff00000000


 

#define RX_MSDU_END_TCP_ACK_NUMBER_OFFSET                                           0x0000000000000020
#define RX_MSDU_END_TCP_ACK_NUMBER_LSB                                              0
#define RX_MSDU_END_TCP_ACK_NUMBER_MSB                                              31
#define RX_MSDU_END_TCP_ACK_NUMBER_MASK                                             0x00000000ffffffff


 

#define RX_MSDU_END_TCP_FLAG_OFFSET                                                 0x0000000000000020
#define RX_MSDU_END_TCP_FLAG_LSB                                                    32
#define RX_MSDU_END_TCP_FLAG_MSB                                                    40
#define RX_MSDU_END_TCP_FLAG_MASK                                                   0x000001ff00000000


 

#define RX_MSDU_END_LRO_ELIGIBLE_OFFSET                                             0x0000000000000020
#define RX_MSDU_END_LRO_ELIGIBLE_LSB                                                41
#define RX_MSDU_END_LRO_ELIGIBLE_MSB                                                41
#define RX_MSDU_END_LRO_ELIGIBLE_MASK                                               0x0000020000000000


 

#define RX_MSDU_END_RESERVED_9A_OFFSET                                              0x0000000000000020
#define RX_MSDU_END_RESERVED_9A_LSB                                                 42
#define RX_MSDU_END_RESERVED_9A_MSB                                                 47
#define RX_MSDU_END_RESERVED_9A_MASK                                                0x0000fc0000000000


 

#define RX_MSDU_END_WINDOW_SIZE_OFFSET                                              0x0000000000000020
#define RX_MSDU_END_WINDOW_SIZE_LSB                                                 48
#define RX_MSDU_END_WINDOW_SIZE_MSB                                                 63
#define RX_MSDU_END_WINDOW_SIZE_MASK                                                0xffff000000000000


 

#define RX_MSDU_END_SA_SW_PEER_ID_OFFSET                                            0x0000000000000028
#define RX_MSDU_END_SA_SW_PEER_ID_LSB                                               0
#define RX_MSDU_END_SA_SW_PEER_ID_MSB                                               15
#define RX_MSDU_END_SA_SW_PEER_ID_MASK                                              0x000000000000ffff


 

#define RX_MSDU_END_SA_IDX_TIMEOUT_OFFSET                                           0x0000000000000028
#define RX_MSDU_END_SA_IDX_TIMEOUT_LSB                                              16
#define RX_MSDU_END_SA_IDX_TIMEOUT_MSB                                              16
#define RX_MSDU_END_SA_IDX_TIMEOUT_MASK                                             0x0000000000010000


 

#define RX_MSDU_END_DA_IDX_TIMEOUT_OFFSET                                           0x0000000000000028
#define RX_MSDU_END_DA_IDX_TIMEOUT_LSB                                              17
#define RX_MSDU_END_DA_IDX_TIMEOUT_MSB                                              17
#define RX_MSDU_END_DA_IDX_TIMEOUT_MASK                                             0x0000000000020000


 

#define RX_MSDU_END_TO_DS_OFFSET                                                    0x0000000000000028
#define RX_MSDU_END_TO_DS_LSB                                                       18
#define RX_MSDU_END_TO_DS_MSB                                                       18
#define RX_MSDU_END_TO_DS_MASK                                                      0x0000000000040000


 

#define RX_MSDU_END_TID_OFFSET                                                      0x0000000000000028
#define RX_MSDU_END_TID_LSB                                                         19
#define RX_MSDU_END_TID_MSB                                                         22
#define RX_MSDU_END_TID_MASK                                                        0x0000000000780000


 

#define RX_MSDU_END_SA_IS_VALID_OFFSET                                              0x0000000000000028
#define RX_MSDU_END_SA_IS_VALID_LSB                                                 23
#define RX_MSDU_END_SA_IS_VALID_MSB                                                 23
#define RX_MSDU_END_SA_IS_VALID_MASK                                                0x0000000000800000


 

#define RX_MSDU_END_DA_IS_VALID_OFFSET                                              0x0000000000000028
#define RX_MSDU_END_DA_IS_VALID_LSB                                                 24
#define RX_MSDU_END_DA_IS_VALID_MSB                                                 24
#define RX_MSDU_END_DA_IS_VALID_MASK                                                0x0000000001000000


 

#define RX_MSDU_END_DA_IS_MCBC_OFFSET                                               0x0000000000000028
#define RX_MSDU_END_DA_IS_MCBC_LSB                                                  25
#define RX_MSDU_END_DA_IS_MCBC_MSB                                                  25
#define RX_MSDU_END_DA_IS_MCBC_MASK                                                 0x0000000002000000


 

#define RX_MSDU_END_L3_HEADER_PADDING_OFFSET                                        0x0000000000000028
#define RX_MSDU_END_L3_HEADER_PADDING_LSB                                           26
#define RX_MSDU_END_L3_HEADER_PADDING_MSB                                           27
#define RX_MSDU_END_L3_HEADER_PADDING_MASK                                          0x000000000c000000


 

#define RX_MSDU_END_FIRST_MSDU_OFFSET                                               0x0000000000000028
#define RX_MSDU_END_FIRST_MSDU_LSB                                                  28
#define RX_MSDU_END_FIRST_MSDU_MSB                                                  28
#define RX_MSDU_END_FIRST_MSDU_MASK                                                 0x0000000010000000


 

#define RX_MSDU_END_LAST_MSDU_OFFSET                                                0x0000000000000028
#define RX_MSDU_END_LAST_MSDU_LSB                                                   29
#define RX_MSDU_END_LAST_MSDU_MSB                                                   29
#define RX_MSDU_END_LAST_MSDU_MASK                                                  0x0000000020000000


 

#define RX_MSDU_END_FR_DS_OFFSET                                                    0x0000000000000028
#define RX_MSDU_END_FR_DS_LSB                                                       30
#define RX_MSDU_END_FR_DS_MSB                                                       30
#define RX_MSDU_END_FR_DS_MASK                                                      0x0000000040000000


 

#define RX_MSDU_END_IP_CHKSUM_FAIL_COPY_OFFSET                                      0x0000000000000028
#define RX_MSDU_END_IP_CHKSUM_FAIL_COPY_LSB                                         31
#define RX_MSDU_END_IP_CHKSUM_FAIL_COPY_MSB                                         31
#define RX_MSDU_END_IP_CHKSUM_FAIL_COPY_MASK                                        0x0000000080000000


 

#define RX_MSDU_END_SA_IDX_OFFSET                                                   0x0000000000000028
#define RX_MSDU_END_SA_IDX_LSB                                                      32
#define RX_MSDU_END_SA_IDX_MSB                                                      47
#define RX_MSDU_END_SA_IDX_MASK                                                     0x0000ffff00000000


 

#define RX_MSDU_END_DA_IDX_OR_SW_PEER_ID_OFFSET                                     0x0000000000000028
#define RX_MSDU_END_DA_IDX_OR_SW_PEER_ID_LSB                                        48
#define RX_MSDU_END_DA_IDX_OR_SW_PEER_ID_MSB                                        63
#define RX_MSDU_END_DA_IDX_OR_SW_PEER_ID_MASK                                       0xffff000000000000


 

#define RX_MSDU_END_MSDU_DROP_OFFSET                                                0x0000000000000030
#define RX_MSDU_END_MSDU_DROP_LSB                                                   0
#define RX_MSDU_END_MSDU_DROP_MSB                                                   0
#define RX_MSDU_END_MSDU_DROP_MASK                                                  0x0000000000000001


 

#define RX_MSDU_END_REO_DESTINATION_INDICATION_OFFSET                               0x0000000000000030
#define RX_MSDU_END_REO_DESTINATION_INDICATION_LSB                                  1
#define RX_MSDU_END_REO_DESTINATION_INDICATION_MSB                                  5
#define RX_MSDU_END_REO_DESTINATION_INDICATION_MASK                                 0x000000000000003e


 

#define RX_MSDU_END_FLOW_IDX_OFFSET                                                 0x0000000000000030
#define RX_MSDU_END_FLOW_IDX_LSB                                                    6
#define RX_MSDU_END_FLOW_IDX_MSB                                                    25
#define RX_MSDU_END_FLOW_IDX_MASK                                                   0x0000000003ffffc0


 

#define RX_MSDU_END_USE_PPE_OFFSET                                                  0x0000000000000030
#define RX_MSDU_END_USE_PPE_LSB                                                     26
#define RX_MSDU_END_USE_PPE_MSB                                                     26
#define RX_MSDU_END_USE_PPE_MASK                                                    0x0000000004000000


 

#define RX_MSDU_END_MESH_STA_OFFSET                                                 0x0000000000000030
#define RX_MSDU_END_MESH_STA_LSB                                                    27
#define RX_MSDU_END_MESH_STA_MSB                                                    28
#define RX_MSDU_END_MESH_STA_MASK                                                   0x0000000018000000


 

#define RX_MSDU_END_VLAN_CTAG_STRIPPED_OFFSET                                       0x0000000000000030
#define RX_MSDU_END_VLAN_CTAG_STRIPPED_LSB                                          29
#define RX_MSDU_END_VLAN_CTAG_STRIPPED_MSB                                          29
#define RX_MSDU_END_VLAN_CTAG_STRIPPED_MASK                                         0x0000000020000000


 

#define RX_MSDU_END_VLAN_STAG_STRIPPED_OFFSET                                       0x0000000000000030
#define RX_MSDU_END_VLAN_STAG_STRIPPED_LSB                                          30
#define RX_MSDU_END_VLAN_STAG_STRIPPED_MSB                                          30
#define RX_MSDU_END_VLAN_STAG_STRIPPED_MASK                                         0x0000000040000000


 

#define RX_MSDU_END_FRAGMENT_FLAG_OFFSET                                            0x0000000000000030
#define RX_MSDU_END_FRAGMENT_FLAG_LSB                                               31
#define RX_MSDU_END_FRAGMENT_FLAG_MSB                                               31
#define RX_MSDU_END_FRAGMENT_FLAG_MASK                                              0x0000000080000000


 

#define RX_MSDU_END_FSE_METADATA_OFFSET                                             0x0000000000000030
#define RX_MSDU_END_FSE_METADATA_LSB                                                32
#define RX_MSDU_END_FSE_METADATA_MSB                                                63
#define RX_MSDU_END_FSE_METADATA_MASK                                               0xffffffff00000000


 

#define RX_MSDU_END_CCE_METADATA_OFFSET                                             0x0000000000000038
#define RX_MSDU_END_CCE_METADATA_LSB                                                0
#define RX_MSDU_END_CCE_METADATA_MSB                                                15
#define RX_MSDU_END_CCE_METADATA_MASK                                               0x000000000000ffff


 

#define RX_MSDU_END_TCP_UDP_CHKSUM_OFFSET                                           0x0000000000000038
#define RX_MSDU_END_TCP_UDP_CHKSUM_LSB                                              16
#define RX_MSDU_END_TCP_UDP_CHKSUM_MSB                                              31
#define RX_MSDU_END_TCP_UDP_CHKSUM_MASK                                             0x00000000ffff0000


 

#define RX_MSDU_END_AGGREGATION_COUNT_OFFSET                                        0x0000000000000038
#define RX_MSDU_END_AGGREGATION_COUNT_LSB                                           32
#define RX_MSDU_END_AGGREGATION_COUNT_MSB                                           39
#define RX_MSDU_END_AGGREGATION_COUNT_MASK                                          0x000000ff00000000


 

#define RX_MSDU_END_FLOW_AGGREGATION_CONTINUATION_OFFSET                            0x0000000000000038
#define RX_MSDU_END_FLOW_AGGREGATION_CONTINUATION_LSB                               40
#define RX_MSDU_END_FLOW_AGGREGATION_CONTINUATION_MSB                               40
#define RX_MSDU_END_FLOW_AGGREGATION_CONTINUATION_MASK                              0x0000010000000000


 

#define RX_MSDU_END_FISA_TIMEOUT_OFFSET                                             0x0000000000000038
#define RX_MSDU_END_FISA_TIMEOUT_LSB                                                41
#define RX_MSDU_END_FISA_TIMEOUT_MSB                                                41
#define RX_MSDU_END_FISA_TIMEOUT_MASK                                               0x0000020000000000


 

#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_COPY_OFFSET                                 0x0000000000000038
#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_COPY_LSB                                    42
#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_COPY_MSB                                    42
#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_COPY_MASK                                   0x0000040000000000


 

#define RX_MSDU_END_MSDU_LIMIT_ERROR_OFFSET                                         0x0000000000000038
#define RX_MSDU_END_MSDU_LIMIT_ERROR_LSB                                            43
#define RX_MSDU_END_MSDU_LIMIT_ERROR_MSB                                            43
#define RX_MSDU_END_MSDU_LIMIT_ERROR_MASK                                           0x0000080000000000


 

#define RX_MSDU_END_FLOW_IDX_TIMEOUT_OFFSET                                         0x0000000000000038
#define RX_MSDU_END_FLOW_IDX_TIMEOUT_LSB                                            44
#define RX_MSDU_END_FLOW_IDX_TIMEOUT_MSB                                            44
#define RX_MSDU_END_FLOW_IDX_TIMEOUT_MASK                                           0x0000100000000000


 

#define RX_MSDU_END_FLOW_IDX_INVALID_OFFSET                                         0x0000000000000038
#define RX_MSDU_END_FLOW_IDX_INVALID_LSB                                            45
#define RX_MSDU_END_FLOW_IDX_INVALID_MSB                                            45
#define RX_MSDU_END_FLOW_IDX_INVALID_MASK                                           0x0000200000000000


 

#define RX_MSDU_END_CCE_MATCH_OFFSET                                                0x0000000000000038
#define RX_MSDU_END_CCE_MATCH_LSB                                                   46
#define RX_MSDU_END_CCE_MATCH_MSB                                                   46
#define RX_MSDU_END_CCE_MATCH_MASK                                                  0x0000400000000000


 

#define RX_MSDU_END_AMSDU_PARSER_ERROR_OFFSET                                       0x0000000000000038
#define RX_MSDU_END_AMSDU_PARSER_ERROR_LSB                                          47
#define RX_MSDU_END_AMSDU_PARSER_ERROR_MSB                                          47
#define RX_MSDU_END_AMSDU_PARSER_ERROR_MASK                                         0x0000800000000000


 

#define RX_MSDU_END_CUMULATIVE_IP_LENGTH_OFFSET                                     0x0000000000000038
#define RX_MSDU_END_CUMULATIVE_IP_LENGTH_LSB                                        48
#define RX_MSDU_END_CUMULATIVE_IP_LENGTH_MSB                                        63
#define RX_MSDU_END_CUMULATIVE_IP_LENGTH_MASK                                       0xffff000000000000


 

#define RX_MSDU_END_KEY_ID_OCTET_OFFSET                                             0x0000000000000040
#define RX_MSDU_END_KEY_ID_OCTET_LSB                                                0
#define RX_MSDU_END_KEY_ID_OCTET_MSB                                                7
#define RX_MSDU_END_KEY_ID_OCTET_MASK                                               0x00000000000000ff


 

#define RX_MSDU_END_RESERVED_16A_OFFSET                                             0x0000000000000040
#define RX_MSDU_END_RESERVED_16A_LSB                                                8
#define RX_MSDU_END_RESERVED_16A_MSB                                                31
#define RX_MSDU_END_RESERVED_16A_MASK                                               0x00000000ffffff00


 

#define RX_MSDU_END_RESERVED_17A_OFFSET                                             0x0000000000000040
#define RX_MSDU_END_RESERVED_17A_LSB                                                32
#define RX_MSDU_END_RESERVED_17A_MSB                                                37
#define RX_MSDU_END_RESERVED_17A_MASK                                               0x0000003f00000000


 

#define RX_MSDU_END_SERVICE_CODE_OFFSET                                             0x0000000000000040
#define RX_MSDU_END_SERVICE_CODE_LSB                                                38
#define RX_MSDU_END_SERVICE_CODE_MSB                                                46
#define RX_MSDU_END_SERVICE_CODE_MASK                                               0x00007fc000000000


 

#define RX_MSDU_END_PRIORITY_VALID_OFFSET                                           0x0000000000000040
#define RX_MSDU_END_PRIORITY_VALID_LSB                                              47
#define RX_MSDU_END_PRIORITY_VALID_MSB                                              47
#define RX_MSDU_END_PRIORITY_VALID_MASK                                             0x0000800000000000


 

#define RX_MSDU_END_INTRA_BSS_OFFSET                                                0x0000000000000040
#define RX_MSDU_END_INTRA_BSS_LSB                                                   48
#define RX_MSDU_END_INTRA_BSS_MSB                                                   48
#define RX_MSDU_END_INTRA_BSS_MASK                                                  0x0001000000000000


 

#define RX_MSDU_END_DEST_CHIP_ID_OFFSET                                             0x0000000000000040
#define RX_MSDU_END_DEST_CHIP_ID_LSB                                                49
#define RX_MSDU_END_DEST_CHIP_ID_MSB                                                50
#define RX_MSDU_END_DEST_CHIP_ID_MASK                                               0x0006000000000000


 

#define RX_MSDU_END_MULTICAST_ECHO_OFFSET                                           0x0000000000000040
#define RX_MSDU_END_MULTICAST_ECHO_LSB                                              51
#define RX_MSDU_END_MULTICAST_ECHO_MSB                                              51
#define RX_MSDU_END_MULTICAST_ECHO_MASK                                             0x0008000000000000


 

#define RX_MSDU_END_WDS_LEARNING_EVENT_OFFSET                                       0x0000000000000040
#define RX_MSDU_END_WDS_LEARNING_EVENT_LSB                                          52
#define RX_MSDU_END_WDS_LEARNING_EVENT_MSB                                          52
#define RX_MSDU_END_WDS_LEARNING_EVENT_MASK                                         0x0010000000000000


 

#define RX_MSDU_END_WDS_ROAMING_EVENT_OFFSET                                        0x0000000000000040
#define RX_MSDU_END_WDS_ROAMING_EVENT_LSB                                           53
#define RX_MSDU_END_WDS_ROAMING_EVENT_MSB                                           53
#define RX_MSDU_END_WDS_ROAMING_EVENT_MASK                                          0x0020000000000000


 

#define RX_MSDU_END_WDS_KEEP_ALIVE_EVENT_OFFSET                                     0x0000000000000040
#define RX_MSDU_END_WDS_KEEP_ALIVE_EVENT_LSB                                        54
#define RX_MSDU_END_WDS_KEEP_ALIVE_EVENT_MSB                                        54
#define RX_MSDU_END_WDS_KEEP_ALIVE_EVENT_MASK                                       0x0040000000000000


 

#define RX_MSDU_END_DEST_CHIP_PMAC_ID_OFFSET                                        0x0000000000000040
#define RX_MSDU_END_DEST_CHIP_PMAC_ID_LSB                                           55
#define RX_MSDU_END_DEST_CHIP_PMAC_ID_MSB                                           55
#define RX_MSDU_END_DEST_CHIP_PMAC_ID_MASK                                          0x0080000000000000


 

#define RX_MSDU_END_RESERVED_17B_OFFSET                                             0x0000000000000040
#define RX_MSDU_END_RESERVED_17B_LSB                                                56
#define RX_MSDU_END_RESERVED_17B_MSB                                                63
#define RX_MSDU_END_RESERVED_17B_MASK                                               0xff00000000000000


 

#define RX_MSDU_END_MSDU_LENGTH_OFFSET                                              0x0000000000000048
#define RX_MSDU_END_MSDU_LENGTH_LSB                                                 0
#define RX_MSDU_END_MSDU_LENGTH_MSB                                                 13
#define RX_MSDU_END_MSDU_LENGTH_MASK                                                0x0000000000003fff


 

#define RX_MSDU_END_STBC_OFFSET                                                     0x0000000000000048
#define RX_MSDU_END_STBC_LSB                                                        14
#define RX_MSDU_END_STBC_MSB                                                        14
#define RX_MSDU_END_STBC_MASK                                                       0x0000000000004000


 

#define RX_MSDU_END_IPSEC_ESP_OFFSET                                                0x0000000000000048
#define RX_MSDU_END_IPSEC_ESP_LSB                                                   15
#define RX_MSDU_END_IPSEC_ESP_MSB                                                   15
#define RX_MSDU_END_IPSEC_ESP_MASK                                                  0x0000000000008000


 

#define RX_MSDU_END_L3_OFFSET_OFFSET                                                0x0000000000000048
#define RX_MSDU_END_L3_OFFSET_LSB                                                   16
#define RX_MSDU_END_L3_OFFSET_MSB                                                   22
#define RX_MSDU_END_L3_OFFSET_MASK                                                  0x00000000007f0000


 

#define RX_MSDU_END_IPSEC_AH_OFFSET                                                 0x0000000000000048
#define RX_MSDU_END_IPSEC_AH_LSB                                                    23
#define RX_MSDU_END_IPSEC_AH_MSB                                                    23
#define RX_MSDU_END_IPSEC_AH_MASK                                                   0x0000000000800000


 

#define RX_MSDU_END_L4_OFFSET_OFFSET                                                0x0000000000000048
#define RX_MSDU_END_L4_OFFSET_LSB                                                   24
#define RX_MSDU_END_L4_OFFSET_MSB                                                   31
#define RX_MSDU_END_L4_OFFSET_MASK                                                  0x00000000ff000000


 

#define RX_MSDU_END_MSDU_NUMBER_OFFSET                                              0x0000000000000048
#define RX_MSDU_END_MSDU_NUMBER_LSB                                                 32
#define RX_MSDU_END_MSDU_NUMBER_MSB                                                 39
#define RX_MSDU_END_MSDU_NUMBER_MASK                                                0x000000ff00000000


 

#define RX_MSDU_END_DECAP_FORMAT_OFFSET                                             0x0000000000000048
#define RX_MSDU_END_DECAP_FORMAT_LSB                                                40
#define RX_MSDU_END_DECAP_FORMAT_MSB                                                41
#define RX_MSDU_END_DECAP_FORMAT_MASK                                               0x0000030000000000


 

#define RX_MSDU_END_IPV4_PROTO_OFFSET                                               0x0000000000000048
#define RX_MSDU_END_IPV4_PROTO_LSB                                                  42
#define RX_MSDU_END_IPV4_PROTO_MSB                                                  42
#define RX_MSDU_END_IPV4_PROTO_MASK                                                 0x0000040000000000


 

#define RX_MSDU_END_IPV6_PROTO_OFFSET                                               0x0000000000000048
#define RX_MSDU_END_IPV6_PROTO_LSB                                                  43
#define RX_MSDU_END_IPV6_PROTO_MSB                                                  43
#define RX_MSDU_END_IPV6_PROTO_MASK                                                 0x0000080000000000


 

#define RX_MSDU_END_TCP_PROTO_OFFSET                                                0x0000000000000048
#define RX_MSDU_END_TCP_PROTO_LSB                                                   44
#define RX_MSDU_END_TCP_PROTO_MSB                                                   44
#define RX_MSDU_END_TCP_PROTO_MASK                                                  0x0000100000000000


 

#define RX_MSDU_END_UDP_PROTO_OFFSET                                                0x0000000000000048
#define RX_MSDU_END_UDP_PROTO_LSB                                                   45
#define RX_MSDU_END_UDP_PROTO_MSB                                                   45
#define RX_MSDU_END_UDP_PROTO_MASK                                                  0x0000200000000000


 

#define RX_MSDU_END_IP_FRAG_OFFSET                                                  0x0000000000000048
#define RX_MSDU_END_IP_FRAG_LSB                                                     46
#define RX_MSDU_END_IP_FRAG_MSB                                                     46
#define RX_MSDU_END_IP_FRAG_MASK                                                    0x0000400000000000


 

#define RX_MSDU_END_TCP_ONLY_ACK_OFFSET                                             0x0000000000000048
#define RX_MSDU_END_TCP_ONLY_ACK_LSB                                                47
#define RX_MSDU_END_TCP_ONLY_ACK_MSB                                                47
#define RX_MSDU_END_TCP_ONLY_ACK_MASK                                               0x0000800000000000


 

#define RX_MSDU_END_DA_IS_BCAST_MCAST_OFFSET                                        0x0000000000000048
#define RX_MSDU_END_DA_IS_BCAST_MCAST_LSB                                           48
#define RX_MSDU_END_DA_IS_BCAST_MCAST_MSB                                           48
#define RX_MSDU_END_DA_IS_BCAST_MCAST_MASK                                          0x0001000000000000


 

#define RX_MSDU_END_TOEPLITZ_HASH_SEL_OFFSET                                        0x0000000000000048
#define RX_MSDU_END_TOEPLITZ_HASH_SEL_LSB                                           49
#define RX_MSDU_END_TOEPLITZ_HASH_SEL_MSB                                           50
#define RX_MSDU_END_TOEPLITZ_HASH_SEL_MASK                                          0x0006000000000000


 

#define RX_MSDU_END_IP_FIXED_HEADER_VALID_OFFSET                                    0x0000000000000048
#define RX_MSDU_END_IP_FIXED_HEADER_VALID_LSB                                       51
#define RX_MSDU_END_IP_FIXED_HEADER_VALID_MSB                                       51
#define RX_MSDU_END_IP_FIXED_HEADER_VALID_MASK                                      0x0008000000000000


 

#define RX_MSDU_END_IP_EXTN_HEADER_VALID_OFFSET                                     0x0000000000000048
#define RX_MSDU_END_IP_EXTN_HEADER_VALID_LSB                                        52
#define RX_MSDU_END_IP_EXTN_HEADER_VALID_MSB                                        52
#define RX_MSDU_END_IP_EXTN_HEADER_VALID_MASK                                       0x0010000000000000


 

#define RX_MSDU_END_TCP_UDP_HEADER_VALID_OFFSET                                     0x0000000000000048
#define RX_MSDU_END_TCP_UDP_HEADER_VALID_LSB                                        53
#define RX_MSDU_END_TCP_UDP_HEADER_VALID_MSB                                        53
#define RX_MSDU_END_TCP_UDP_HEADER_VALID_MASK                                       0x0020000000000000


 

#define RX_MSDU_END_MESH_CONTROL_PRESENT_OFFSET                                     0x0000000000000048
#define RX_MSDU_END_MESH_CONTROL_PRESENT_LSB                                        54
#define RX_MSDU_END_MESH_CONTROL_PRESENT_MSB                                        54
#define RX_MSDU_END_MESH_CONTROL_PRESENT_MASK                                       0x0040000000000000


 

#define RX_MSDU_END_LDPC_OFFSET                                                     0x0000000000000048
#define RX_MSDU_END_LDPC_LSB                                                        55
#define RX_MSDU_END_LDPC_MSB                                                        55
#define RX_MSDU_END_LDPC_MASK                                                       0x0080000000000000


 

#define RX_MSDU_END_IP4_PROTOCOL_IP6_NEXT_HEADER_OFFSET                             0x0000000000000048
#define RX_MSDU_END_IP4_PROTOCOL_IP6_NEXT_HEADER_LSB                                56
#define RX_MSDU_END_IP4_PROTOCOL_IP6_NEXT_HEADER_MSB                                63
#define RX_MSDU_END_IP4_PROTOCOL_IP6_NEXT_HEADER_MASK                               0xff00000000000000


 

#define RX_MSDU_END_VLAN_CTAG_CI_OFFSET                                             0x0000000000000050
#define RX_MSDU_END_VLAN_CTAG_CI_LSB                                                0
#define RX_MSDU_END_VLAN_CTAG_CI_MSB                                                15
#define RX_MSDU_END_VLAN_CTAG_CI_MASK                                               0x000000000000ffff


 

#define RX_MSDU_END_VLAN_STAG_CI_OFFSET                                             0x0000000000000050
#define RX_MSDU_END_VLAN_STAG_CI_LSB                                                16
#define RX_MSDU_END_VLAN_STAG_CI_MSB                                                31
#define RX_MSDU_END_VLAN_STAG_CI_MASK                                               0x00000000ffff0000


 

#define RX_MSDU_END_PEER_META_DATA_OFFSET                                           0x0000000000000050
#define RX_MSDU_END_PEER_META_DATA_LSB                                              32
#define RX_MSDU_END_PEER_META_DATA_MSB                                              63
#define RX_MSDU_END_PEER_META_DATA_MASK                                             0xffffffff00000000


 

#define RX_MSDU_END_USER_RSSI_OFFSET                                                0x0000000000000058
#define RX_MSDU_END_USER_RSSI_LSB                                                   0
#define RX_MSDU_END_USER_RSSI_MSB                                                   7
#define RX_MSDU_END_USER_RSSI_MASK                                                  0x00000000000000ff


 

#define RX_MSDU_END_PKT_TYPE_OFFSET                                                 0x0000000000000058
#define RX_MSDU_END_PKT_TYPE_LSB                                                    8
#define RX_MSDU_END_PKT_TYPE_MSB                                                    11
#define RX_MSDU_END_PKT_TYPE_MASK                                                   0x0000000000000f00


 

#define RX_MSDU_END_SGI_OFFSET                                                      0x0000000000000058
#define RX_MSDU_END_SGI_LSB                                                         12
#define RX_MSDU_END_SGI_MSB                                                         13
#define RX_MSDU_END_SGI_MASK                                                        0x0000000000003000


 

#define RX_MSDU_END_RATE_MCS_OFFSET                                                 0x0000000000000058
#define RX_MSDU_END_RATE_MCS_LSB                                                    14
#define RX_MSDU_END_RATE_MCS_MSB                                                    17
#define RX_MSDU_END_RATE_MCS_MASK                                                   0x000000000003c000


 

#define RX_MSDU_END_RECEIVE_BANDWIDTH_OFFSET                                        0x0000000000000058
#define RX_MSDU_END_RECEIVE_BANDWIDTH_LSB                                           18
#define RX_MSDU_END_RECEIVE_BANDWIDTH_MSB                                           20
#define RX_MSDU_END_RECEIVE_BANDWIDTH_MASK                                          0x00000000001c0000


 

#define RX_MSDU_END_RECEPTION_TYPE_OFFSET                                           0x0000000000000058
#define RX_MSDU_END_RECEPTION_TYPE_LSB                                              21
#define RX_MSDU_END_RECEPTION_TYPE_MSB                                              23
#define RX_MSDU_END_RECEPTION_TYPE_MASK                                             0x0000000000e00000


 

#define RX_MSDU_END_MIMO_SS_BITMAP_OFFSET                                           0x0000000000000058
#define RX_MSDU_END_MIMO_SS_BITMAP_LSB                                              24
#define RX_MSDU_END_MIMO_SS_BITMAP_MSB                                              30
#define RX_MSDU_END_MIMO_SS_BITMAP_MASK                                             0x000000007f000000


 

#define RX_MSDU_END_MSDU_DONE_COPY_OFFSET                                           0x0000000000000058
#define RX_MSDU_END_MSDU_DONE_COPY_LSB                                              31
#define RX_MSDU_END_MSDU_DONE_COPY_MSB                                              31
#define RX_MSDU_END_MSDU_DONE_COPY_MASK                                             0x0000000080000000


 

#define RX_MSDU_END_FLOW_ID_TOEPLITZ_OFFSET                                         0x0000000000000058
#define RX_MSDU_END_FLOW_ID_TOEPLITZ_LSB                                            32
#define RX_MSDU_END_FLOW_ID_TOEPLITZ_MSB                                            63
#define RX_MSDU_END_FLOW_ID_TOEPLITZ_MASK                                           0xffffffff00000000


 

#define RX_MSDU_END_PPDU_START_TIMESTAMP_63_32_OFFSET                               0x0000000000000060
#define RX_MSDU_END_PPDU_START_TIMESTAMP_63_32_LSB                                  0
#define RX_MSDU_END_PPDU_START_TIMESTAMP_63_32_MSB                                  31
#define RX_MSDU_END_PPDU_START_TIMESTAMP_63_32_MASK                                 0x00000000ffffffff


 

#define RX_MSDU_END_SW_PHY_META_DATA_OFFSET                                         0x0000000000000060
#define RX_MSDU_END_SW_PHY_META_DATA_LSB                                            32
#define RX_MSDU_END_SW_PHY_META_DATA_MSB                                            63
#define RX_MSDU_END_SW_PHY_META_DATA_MASK                                           0xffffffff00000000


 

#define RX_MSDU_END_PPDU_START_TIMESTAMP_31_0_OFFSET                                0x0000000000000068
#define RX_MSDU_END_PPDU_START_TIMESTAMP_31_0_LSB                                   0
#define RX_MSDU_END_PPDU_START_TIMESTAMP_31_0_MSB                                   31
#define RX_MSDU_END_PPDU_START_TIMESTAMP_31_0_MASK                                  0x00000000ffffffff


 

#define RX_MSDU_END_TOEPLITZ_HASH_2_OR_4_OFFSET                                     0x0000000000000068
#define RX_MSDU_END_TOEPLITZ_HASH_2_OR_4_LSB                                        32
#define RX_MSDU_END_TOEPLITZ_HASH_2_OR_4_MSB                                        63
#define RX_MSDU_END_TOEPLITZ_HASH_2_OR_4_MASK                                       0xffffffff00000000


 

#define RX_MSDU_END_RESERVED_28A_OFFSET                                             0x0000000000000070
#define RX_MSDU_END_RESERVED_28A_LSB                                                0
#define RX_MSDU_END_RESERVED_28A_MSB                                                15
#define RX_MSDU_END_RESERVED_28A_MASK                                               0x000000000000ffff


 

#define RX_MSDU_END_SA_15_0_OFFSET                                                  0x0000000000000070
#define RX_MSDU_END_SA_15_0_LSB                                                     16
#define RX_MSDU_END_SA_15_0_MSB                                                     31
#define RX_MSDU_END_SA_15_0_MASK                                                    0x00000000ffff0000


 

#define RX_MSDU_END_SA_47_16_OFFSET                                                 0x0000000000000070
#define RX_MSDU_END_SA_47_16_LSB                                                    32
#define RX_MSDU_END_SA_47_16_MSB                                                    63
#define RX_MSDU_END_SA_47_16_MASK                                                   0xffffffff00000000


 

#define RX_MSDU_END_FIRST_MPDU_OFFSET                                               0x0000000000000078
#define RX_MSDU_END_FIRST_MPDU_LSB                                                  0
#define RX_MSDU_END_FIRST_MPDU_MSB                                                  0
#define RX_MSDU_END_FIRST_MPDU_MASK                                                 0x0000000000000001


 

#define RX_MSDU_END_RESERVED_30A_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_RESERVED_30A_LSB                                                1
#define RX_MSDU_END_RESERVED_30A_MSB                                                1
#define RX_MSDU_END_RESERVED_30A_MASK                                               0x0000000000000002


 

#define RX_MSDU_END_MCAST_BCAST_OFFSET                                              0x0000000000000078
#define RX_MSDU_END_MCAST_BCAST_LSB                                                 2
#define RX_MSDU_END_MCAST_BCAST_MSB                                                 2
#define RX_MSDU_END_MCAST_BCAST_MASK                                                0x0000000000000004


 

#define RX_MSDU_END_AST_INDEX_NOT_FOUND_OFFSET                                      0x0000000000000078
#define RX_MSDU_END_AST_INDEX_NOT_FOUND_LSB                                         3
#define RX_MSDU_END_AST_INDEX_NOT_FOUND_MSB                                         3
#define RX_MSDU_END_AST_INDEX_NOT_FOUND_MASK                                        0x0000000000000008


 

#define RX_MSDU_END_AST_INDEX_TIMEOUT_OFFSET                                        0x0000000000000078
#define RX_MSDU_END_AST_INDEX_TIMEOUT_LSB                                           4
#define RX_MSDU_END_AST_INDEX_TIMEOUT_MSB                                           4
#define RX_MSDU_END_AST_INDEX_TIMEOUT_MASK                                          0x0000000000000010


 

#define RX_MSDU_END_POWER_MGMT_OFFSET                                               0x0000000000000078
#define RX_MSDU_END_POWER_MGMT_LSB                                                  5
#define RX_MSDU_END_POWER_MGMT_MSB                                                  5
#define RX_MSDU_END_POWER_MGMT_MASK                                                 0x0000000000000020


 

#define RX_MSDU_END_NON_QOS_OFFSET                                                  0x0000000000000078
#define RX_MSDU_END_NON_QOS_LSB                                                     6
#define RX_MSDU_END_NON_QOS_MSB                                                     6
#define RX_MSDU_END_NON_QOS_MASK                                                    0x0000000000000040


 

#define RX_MSDU_END_NULL_DATA_OFFSET                                                0x0000000000000078
#define RX_MSDU_END_NULL_DATA_LSB                                                   7
#define RX_MSDU_END_NULL_DATA_MSB                                                   7
#define RX_MSDU_END_NULL_DATA_MASK                                                  0x0000000000000080


 

#define RX_MSDU_END_MGMT_TYPE_OFFSET                                                0x0000000000000078
#define RX_MSDU_END_MGMT_TYPE_LSB                                                   8
#define RX_MSDU_END_MGMT_TYPE_MSB                                                   8
#define RX_MSDU_END_MGMT_TYPE_MASK                                                  0x0000000000000100


 

#define RX_MSDU_END_CTRL_TYPE_OFFSET                                                0x0000000000000078
#define RX_MSDU_END_CTRL_TYPE_LSB                                                   9
#define RX_MSDU_END_CTRL_TYPE_MSB                                                   9
#define RX_MSDU_END_CTRL_TYPE_MASK                                                  0x0000000000000200


 

#define RX_MSDU_END_MORE_DATA_OFFSET                                                0x0000000000000078
#define RX_MSDU_END_MORE_DATA_LSB                                                   10
#define RX_MSDU_END_MORE_DATA_MSB                                                   10
#define RX_MSDU_END_MORE_DATA_MASK                                                  0x0000000000000400


 

#define RX_MSDU_END_EOSP_OFFSET                                                     0x0000000000000078
#define RX_MSDU_END_EOSP_LSB                                                        11
#define RX_MSDU_END_EOSP_MSB                                                        11
#define RX_MSDU_END_EOSP_MASK                                                       0x0000000000000800


 

#define RX_MSDU_END_A_MSDU_ERROR_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_A_MSDU_ERROR_LSB                                                12
#define RX_MSDU_END_A_MSDU_ERROR_MSB                                                12
#define RX_MSDU_END_A_MSDU_ERROR_MASK                                               0x0000000000001000


 

#define RX_MSDU_END_RESERVED_30B_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_RESERVED_30B_LSB                                                13
#define RX_MSDU_END_RESERVED_30B_MSB                                                13
#define RX_MSDU_END_RESERVED_30B_MASK                                               0x0000000000002000


 

#define RX_MSDU_END_ORDER_OFFSET                                                    0x0000000000000078
#define RX_MSDU_END_ORDER_LSB                                                       14
#define RX_MSDU_END_ORDER_MSB                                                       14
#define RX_MSDU_END_ORDER_MASK                                                      0x0000000000004000


 

#define RX_MSDU_END_WIFI_PARSER_ERROR_OFFSET                                        0x0000000000000078
#define RX_MSDU_END_WIFI_PARSER_ERROR_LSB                                           15
#define RX_MSDU_END_WIFI_PARSER_ERROR_MSB                                           15
#define RX_MSDU_END_WIFI_PARSER_ERROR_MASK                                          0x0000000000008000


 

#define RX_MSDU_END_OVERFLOW_ERR_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_OVERFLOW_ERR_LSB                                                16
#define RX_MSDU_END_OVERFLOW_ERR_MSB                                                16
#define RX_MSDU_END_OVERFLOW_ERR_MASK                                               0x0000000000010000


 

#define RX_MSDU_END_MSDU_LENGTH_ERR_OFFSET                                          0x0000000000000078
#define RX_MSDU_END_MSDU_LENGTH_ERR_LSB                                             17
#define RX_MSDU_END_MSDU_LENGTH_ERR_MSB                                             17
#define RX_MSDU_END_MSDU_LENGTH_ERR_MASK                                            0x0000000000020000


 

#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_OFFSET                                      0x0000000000000078
#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_LSB                                         18
#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_MSB                                         18
#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_MASK                                        0x0000000000040000


 

#define RX_MSDU_END_IP_CHKSUM_FAIL_OFFSET                                           0x0000000000000078
#define RX_MSDU_END_IP_CHKSUM_FAIL_LSB                                              19
#define RX_MSDU_END_IP_CHKSUM_FAIL_MSB                                              19
#define RX_MSDU_END_IP_CHKSUM_FAIL_MASK                                             0x0000000000080000


 

#define RX_MSDU_END_SA_IDX_INVALID_OFFSET                                           0x0000000000000078
#define RX_MSDU_END_SA_IDX_INVALID_LSB                                              20
#define RX_MSDU_END_SA_IDX_INVALID_MSB                                              20
#define RX_MSDU_END_SA_IDX_INVALID_MASK                                             0x0000000000100000


 

#define RX_MSDU_END_DA_IDX_INVALID_OFFSET                                           0x0000000000000078
#define RX_MSDU_END_DA_IDX_INVALID_LSB                                              21
#define RX_MSDU_END_DA_IDX_INVALID_MSB                                              21
#define RX_MSDU_END_DA_IDX_INVALID_MASK                                             0x0000000000200000


 

#define RX_MSDU_END_AMSDU_ADDR_MISMATCH_OFFSET                                      0x0000000000000078
#define RX_MSDU_END_AMSDU_ADDR_MISMATCH_LSB                                         22
#define RX_MSDU_END_AMSDU_ADDR_MISMATCH_MSB                                         22
#define RX_MSDU_END_AMSDU_ADDR_MISMATCH_MASK                                        0x0000000000400000


 

#define RX_MSDU_END_RX_IN_TX_DECRYPT_BYP_OFFSET                                     0x0000000000000078
#define RX_MSDU_END_RX_IN_TX_DECRYPT_BYP_LSB                                        23
#define RX_MSDU_END_RX_IN_TX_DECRYPT_BYP_MSB                                        23
#define RX_MSDU_END_RX_IN_TX_DECRYPT_BYP_MASK                                       0x0000000000800000


 

#define RX_MSDU_END_ENCRYPT_REQUIRED_OFFSET                                         0x0000000000000078
#define RX_MSDU_END_ENCRYPT_REQUIRED_LSB                                            24
#define RX_MSDU_END_ENCRYPT_REQUIRED_MSB                                            24
#define RX_MSDU_END_ENCRYPT_REQUIRED_MASK                                           0x0000000001000000


 

#define RX_MSDU_END_DIRECTED_OFFSET                                                 0x0000000000000078
#define RX_MSDU_END_DIRECTED_LSB                                                    25
#define RX_MSDU_END_DIRECTED_MSB                                                    25
#define RX_MSDU_END_DIRECTED_MASK                                                   0x0000000002000000


 

#define RX_MSDU_END_BUFFER_FRAGMENT_OFFSET                                          0x0000000000000078
#define RX_MSDU_END_BUFFER_FRAGMENT_LSB                                             26
#define RX_MSDU_END_BUFFER_FRAGMENT_MSB                                             26
#define RX_MSDU_END_BUFFER_FRAGMENT_MASK                                            0x0000000004000000


 

#define RX_MSDU_END_MPDU_LENGTH_ERR_OFFSET                                          0x0000000000000078
#define RX_MSDU_END_MPDU_LENGTH_ERR_LSB                                             27
#define RX_MSDU_END_MPDU_LENGTH_ERR_MSB                                             27
#define RX_MSDU_END_MPDU_LENGTH_ERR_MASK                                            0x0000000008000000


 

#define RX_MSDU_END_TKIP_MIC_ERR_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_TKIP_MIC_ERR_LSB                                                28
#define RX_MSDU_END_TKIP_MIC_ERR_MSB                                                28
#define RX_MSDU_END_TKIP_MIC_ERR_MASK                                               0x0000000010000000


 

#define RX_MSDU_END_DECRYPT_ERR_OFFSET                                              0x0000000000000078
#define RX_MSDU_END_DECRYPT_ERR_LSB                                                 29
#define RX_MSDU_END_DECRYPT_ERR_MSB                                                 29
#define RX_MSDU_END_DECRYPT_ERR_MASK                                                0x0000000020000000


 

#define RX_MSDU_END_UNENCRYPTED_FRAME_ERR_OFFSET                                    0x0000000000000078
#define RX_MSDU_END_UNENCRYPTED_FRAME_ERR_LSB                                       30
#define RX_MSDU_END_UNENCRYPTED_FRAME_ERR_MSB                                       30
#define RX_MSDU_END_UNENCRYPTED_FRAME_ERR_MASK                                      0x0000000040000000


 

#define RX_MSDU_END_FCS_ERR_OFFSET                                                  0x0000000000000078
#define RX_MSDU_END_FCS_ERR_LSB                                                     31
#define RX_MSDU_END_FCS_ERR_MSB                                                     31
#define RX_MSDU_END_FCS_ERR_MASK                                                    0x0000000080000000


 

#define RX_MSDU_END_RESERVED_31A_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_RESERVED_31A_LSB                                                32
#define RX_MSDU_END_RESERVED_31A_MSB                                                41
#define RX_MSDU_END_RESERVED_31A_MASK                                               0x000003ff00000000


 

#define RX_MSDU_END_DECRYPT_STATUS_CODE_OFFSET                                      0x0000000000000078
#define RX_MSDU_END_DECRYPT_STATUS_CODE_LSB                                         42
#define RX_MSDU_END_DECRYPT_STATUS_CODE_MSB                                         44
#define RX_MSDU_END_DECRYPT_STATUS_CODE_MASK                                        0x00001c0000000000


 

#define RX_MSDU_END_RX_BITMAP_NOT_UPDATED_OFFSET                                    0x0000000000000078
#define RX_MSDU_END_RX_BITMAP_NOT_UPDATED_LSB                                       45
#define RX_MSDU_END_RX_BITMAP_NOT_UPDATED_MSB                                       45
#define RX_MSDU_END_RX_BITMAP_NOT_UPDATED_MASK                                      0x0000200000000000


 

#define RX_MSDU_END_RESERVED_31B_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_RESERVED_31B_LSB                                                46
#define RX_MSDU_END_RESERVED_31B_MSB                                                62
#define RX_MSDU_END_RESERVED_31B_MASK                                               0x7fffc00000000000


 

#define RX_MSDU_END_MSDU_DONE_OFFSET                                                0x0000000000000078
#define RX_MSDU_END_MSDU_DONE_LSB                                                   63
#define RX_MSDU_END_MSDU_DONE_MSB                                                   63
#define RX_MSDU_END_MSDU_DONE_MASK                                                  0x8000000000000000



#endif    
