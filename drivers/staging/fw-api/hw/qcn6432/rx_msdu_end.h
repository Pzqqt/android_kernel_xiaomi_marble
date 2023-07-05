/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
             uint32_t rxpcu_mpdu_filter_in_category                           :  2, // [1:0]
                      sw_frame_group_id                                       :  7, // [8:2]
                      reserved_0                                              :  7, // [15:9]
                      phy_ppdu_id                                             : 16; // [31:16]
             uint32_t ip_hdr_chksum                                           : 16, // [15:0]
                      reported_mpdu_length                                    : 14, // [29:16]
                      reserved_1a                                             :  2; // [31:30]
             uint32_t reserved_2a                                             :  8, // [7:0]
                      cce_super_rule                                          :  6, // [13:8]
                      cce_classify_not_done_truncate                          :  1, // [14:14]
                      cce_classify_not_done_cce_dis                           :  1, // [15:15]
                      cumulative_l3_checksum                                  : 16; // [31:16]
             uint32_t rule_indication_31_0                                    : 32; // [31:0]
             uint32_t ipv6_options_crc                                        : 32; // [31:0]
             uint32_t da_offset                                               :  6, // [5:0]
                      sa_offset                                               :  6, // [11:6]
                      da_offset_valid                                         :  1, // [12:12]
                      sa_offset_valid                                         :  1, // [13:13]
                      reserved_5a                                             :  2, // [15:14]
                      l3_type                                                 : 16; // [31:16]
             uint32_t rule_indication_63_32                                   : 32; // [31:0]
             uint32_t tcp_seq_number                                          : 32; // [31:0]
             uint32_t tcp_ack_number                                          : 32; // [31:0]
             uint32_t tcp_flag                                                :  9, // [8:0]
                      lro_eligible                                            :  1, // [9:9]
                      reserved_9a                                             :  6, // [15:10]
                      window_size                                             : 16; // [31:16]
             uint32_t sa_sw_peer_id                                           : 16, // [15:0]
                      sa_idx_timeout                                          :  1, // [16:16]
                      da_idx_timeout                                          :  1, // [17:17]
                      to_ds                                                   :  1, // [18:18]
                      tid                                                     :  4, // [22:19]
                      sa_is_valid                                             :  1, // [23:23]
                      da_is_valid                                             :  1, // [24:24]
                      da_is_mcbc                                              :  1, // [25:25]
                      l3_header_padding                                       :  2, // [27:26]
                      first_msdu                                              :  1, // [28:28]
                      last_msdu                                               :  1, // [29:29]
                      fr_ds                                                   :  1, // [30:30]
                      ip_chksum_fail_copy                                     :  1; // [31:31]
             uint32_t sa_idx                                                  : 16, // [15:0]
                      da_idx_or_sw_peer_id                                    : 16; // [31:16]
             uint32_t msdu_drop                                               :  1, // [0:0]
                      reo_destination_indication                              :  5, // [5:1]
                      flow_idx                                                : 20, // [25:6]
                      use_ppe                                                 :  1, // [26:26]
                      mesh_sta                                                :  2, // [28:27]
                      vlan_ctag_stripped                                      :  1, // [29:29]
                      vlan_stag_stripped                                      :  1, // [30:30]
                      fragment_flag                                           :  1; // [31:31]
             uint32_t fse_metadata                                            : 32; // [31:0]
             uint32_t cce_metadata                                            : 16, // [15:0]
                      tcp_udp_chksum                                          : 16; // [31:16]
             uint32_t aggregation_count                                       :  8, // [7:0]
                      flow_aggregation_continuation                           :  1, // [8:8]
                      fisa_timeout                                            :  1, // [9:9]
                      tcp_udp_chksum_fail_copy                                :  1, // [10:10]
                      msdu_limit_error                                        :  1, // [11:11]
                      flow_idx_timeout                                        :  1, // [12:12]
                      flow_idx_invalid                                        :  1, // [13:13]
                      cce_match                                               :  1, // [14:14]
                      amsdu_parser_error                                      :  1, // [15:15]
                      cumulative_ip_length                                    : 16; // [31:16]
             uint32_t key_id_octet                                            :  8, // [7:0]
                      reserved_16a                                            : 24; // [31:8]
             uint32_t reserved_17a                                            :  6, // [5:0]
                      service_code                                            :  9, // [14:6]
                      priority_valid                                          :  1, // [15:15]
                      intra_bss                                               :  1, // [16:16]
                      dest_chip_id                                            :  2, // [18:17]
                      multicast_echo                                          :  1, // [19:19]
                      wds_learning_event                                      :  1, // [20:20]
                      wds_roaming_event                                       :  1, // [21:21]
                      wds_keep_alive_event                                    :  1, // [22:22]
                      dest_chip_pmac_id                                       :  1, // [23:23]
                      reserved_17b                                            :  8; // [31:24]
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
             uint32_t vlan_ctag_ci                                            : 16, // [15:0]
                      vlan_stag_ci                                            : 16; // [31:16]
             uint32_t peer_meta_data                                          : 32; // [31:0]
             uint32_t user_rssi                                               :  8, // [7:0]
                      pkt_type                                                :  4, // [11:8]
                      sgi                                                     :  2, // [13:12]
                      rate_mcs                                                :  4, // [17:14]
                      receive_bandwidth                                       :  3, // [20:18]
                      reception_type                                          :  3, // [23:21]
                      mimo_ss_bitmap                                          :  7, // [30:24]
                      msdu_done_copy                                          :  1; // [31:31]
             uint32_t flow_id_toeplitz                                        : 32; // [31:0]
             uint32_t ppdu_start_timestamp_63_32                              : 32; // [31:0]
             uint32_t sw_phy_meta_data                                        : 32; // [31:0]
             uint32_t ppdu_start_timestamp_31_0                               : 32; // [31:0]
             uint32_t toeplitz_hash_2_or_4                                    : 32; // [31:0]
             uint32_t reserved_28a                                            : 16, // [15:0]
                      sa_15_0                                                 : 16; // [31:16]
             uint32_t sa_47_16                                                : 32; // [31:0]
             uint32_t first_mpdu                                              :  1, // [0:0]
                      reserved_30a                                            :  1, // [1:1]
                      mcast_bcast                                             :  1, // [2:2]
                      ast_index_not_found                                     :  1, // [3:3]
                      ast_index_timeout                                       :  1, // [4:4]
                      power_mgmt                                              :  1, // [5:5]
                      non_qos                                                 :  1, // [6:6]
                      null_data                                               :  1, // [7:7]
                      mgmt_type                                               :  1, // [8:8]
                      ctrl_type                                               :  1, // [9:9]
                      more_data                                               :  1, // [10:10]
                      eosp                                                    :  1, // [11:11]
                      a_msdu_error                                            :  1, // [12:12]
                      reserved_30b                                            :  1, // [13:13]
                      order                                                   :  1, // [14:14]
                      wifi_parser_error                                       :  1, // [15:15]
                      overflow_err                                            :  1, // [16:16]
                      msdu_length_err                                         :  1, // [17:17]
                      tcp_udp_chksum_fail                                     :  1, // [18:18]
                      ip_chksum_fail                                          :  1, // [19:19]
                      sa_idx_invalid                                          :  1, // [20:20]
                      da_idx_invalid                                          :  1, // [21:21]
                      amsdu_addr_mismatch                                     :  1, // [22:22]
                      rx_in_tx_decrypt_byp                                    :  1, // [23:23]
                      encrypt_required                                        :  1, // [24:24]
                      directed                                                :  1, // [25:25]
                      buffer_fragment                                         :  1, // [26:26]
                      mpdu_length_err                                         :  1, // [27:27]
                      tkip_mic_err                                            :  1, // [28:28]
                      decrypt_err                                             :  1, // [29:29]
                      unencrypted_frame_err                                   :  1, // [30:30]
                      fcs_err                                                 :  1; // [31:31]
             uint32_t reserved_31a                                            : 10, // [9:0]
                      decrypt_status_code                                     :  3, // [12:10]
                      rx_bitmap_not_updated                                   :  1, // [13:13]
                      reserved_31b                                            : 17, // [30:14]
                      msdu_done                                               :  1; // [31:31]
#else
             uint32_t phy_ppdu_id                                             : 16, // [31:16]
                      reserved_0                                              :  7, // [15:9]
                      sw_frame_group_id                                       :  7, // [8:2]
                      rxpcu_mpdu_filter_in_category                           :  2; // [1:0]
             uint32_t reserved_1a                                             :  2, // [31:30]
                      reported_mpdu_length                                    : 14, // [29:16]
                      ip_hdr_chksum                                           : 16; // [15:0]
             uint32_t cumulative_l3_checksum                                  : 16, // [31:16]
                      cce_classify_not_done_cce_dis                           :  1, // [15:15]
                      cce_classify_not_done_truncate                          :  1, // [14:14]
                      cce_super_rule                                          :  6, // [13:8]
                      reserved_2a                                             :  8; // [7:0]
             uint32_t rule_indication_31_0                                    : 32; // [31:0]
             uint32_t ipv6_options_crc                                        : 32; // [31:0]
             uint32_t l3_type                                                 : 16, // [31:16]
                      reserved_5a                                             :  2, // [15:14]
                      sa_offset_valid                                         :  1, // [13:13]
                      da_offset_valid                                         :  1, // [12:12]
                      sa_offset                                               :  6, // [11:6]
                      da_offset                                               :  6; // [5:0]
             uint32_t rule_indication_63_32                                   : 32; // [31:0]
             uint32_t tcp_seq_number                                          : 32; // [31:0]
             uint32_t tcp_ack_number                                          : 32; // [31:0]
             uint32_t window_size                                             : 16, // [31:16]
                      reserved_9a                                             :  6, // [15:10]
                      lro_eligible                                            :  1, // [9:9]
                      tcp_flag                                                :  9; // [8:0]
             uint32_t ip_chksum_fail_copy                                     :  1, // [31:31]
                      fr_ds                                                   :  1, // [30:30]
                      last_msdu                                               :  1, // [29:29]
                      first_msdu                                              :  1, // [28:28]
                      l3_header_padding                                       :  2, // [27:26]
                      da_is_mcbc                                              :  1, // [25:25]
                      da_is_valid                                             :  1, // [24:24]
                      sa_is_valid                                             :  1, // [23:23]
                      tid                                                     :  4, // [22:19]
                      to_ds                                                   :  1, // [18:18]
                      da_idx_timeout                                          :  1, // [17:17]
                      sa_idx_timeout                                          :  1, // [16:16]
                      sa_sw_peer_id                                           : 16; // [15:0]
             uint32_t da_idx_or_sw_peer_id                                    : 16, // [31:16]
                      sa_idx                                                  : 16; // [15:0]
             uint32_t fragment_flag                                           :  1, // [31:31]
                      vlan_stag_stripped                                      :  1, // [30:30]
                      vlan_ctag_stripped                                      :  1, // [29:29]
                      mesh_sta                                                :  2, // [28:27]
                      use_ppe                                                 :  1, // [26:26]
                      flow_idx                                                : 20, // [25:6]
                      reo_destination_indication                              :  5, // [5:1]
                      msdu_drop                                               :  1; // [0:0]
             uint32_t fse_metadata                                            : 32; // [31:0]
             uint32_t tcp_udp_chksum                                          : 16, // [31:16]
                      cce_metadata                                            : 16; // [15:0]
             uint32_t cumulative_ip_length                                    : 16, // [31:16]
                      amsdu_parser_error                                      :  1, // [15:15]
                      cce_match                                               :  1, // [14:14]
                      flow_idx_invalid                                        :  1, // [13:13]
                      flow_idx_timeout                                        :  1, // [12:12]
                      msdu_limit_error                                        :  1, // [11:11]
                      tcp_udp_chksum_fail_copy                                :  1, // [10:10]
                      fisa_timeout                                            :  1, // [9:9]
                      flow_aggregation_continuation                           :  1, // [8:8]
                      aggregation_count                                       :  8; // [7:0]
             uint32_t reserved_16a                                            : 24, // [31:8]
                      key_id_octet                                            :  8; // [7:0]
             uint32_t reserved_17b                                            :  8, // [31:24]
                      dest_chip_pmac_id                                       :  1, // [23:23]
                      wds_keep_alive_event                                    :  1, // [22:22]
                      wds_roaming_event                                       :  1, // [21:21]
                      wds_learning_event                                      :  1, // [20:20]
                      multicast_echo                                          :  1, // [19:19]
                      dest_chip_id                                            :  2, // [18:17]
                      intra_bss                                               :  1, // [16:16]
                      priority_valid                                          :  1, // [15:15]
                      service_code                                            :  9, // [14:6]
                      reserved_17a                                            :  6; // [5:0]
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
             uint32_t vlan_stag_ci                                            : 16, // [31:16]
                      vlan_ctag_ci                                            : 16; // [15:0]
             uint32_t peer_meta_data                                          : 32; // [31:0]
             uint32_t msdu_done_copy                                          :  1, // [31:31]
                      mimo_ss_bitmap                                          :  7, // [30:24]
                      reception_type                                          :  3, // [23:21]
                      receive_bandwidth                                       :  3, // [20:18]
                      rate_mcs                                                :  4, // [17:14]
                      sgi                                                     :  2, // [13:12]
                      pkt_type                                                :  4, // [11:8]
                      user_rssi                                               :  8; // [7:0]
             uint32_t flow_id_toeplitz                                        : 32; // [31:0]
             uint32_t ppdu_start_timestamp_63_32                              : 32; // [31:0]
             uint32_t sw_phy_meta_data                                        : 32; // [31:0]
             uint32_t ppdu_start_timestamp_31_0                               : 32; // [31:0]
             uint32_t toeplitz_hash_2_or_4                                    : 32; // [31:0]
             uint32_t sa_15_0                                                 : 16, // [31:16]
                      reserved_28a                                            : 16; // [15:0]
             uint32_t sa_47_16                                                : 32; // [31:0]
             uint32_t fcs_err                                                 :  1, // [31:31]
                      unencrypted_frame_err                                   :  1, // [30:30]
                      decrypt_err                                             :  1, // [29:29]
                      tkip_mic_err                                            :  1, // [28:28]
                      mpdu_length_err                                         :  1, // [27:27]
                      buffer_fragment                                         :  1, // [26:26]
                      directed                                                :  1, // [25:25]
                      encrypt_required                                        :  1, // [24:24]
                      rx_in_tx_decrypt_byp                                    :  1, // [23:23]
                      amsdu_addr_mismatch                                     :  1, // [22:22]
                      da_idx_invalid                                          :  1, // [21:21]
                      sa_idx_invalid                                          :  1, // [20:20]
                      ip_chksum_fail                                          :  1, // [19:19]
                      tcp_udp_chksum_fail                                     :  1, // [18:18]
                      msdu_length_err                                         :  1, // [17:17]
                      overflow_err                                            :  1, // [16:16]
                      wifi_parser_error                                       :  1, // [15:15]
                      order                                                   :  1, // [14:14]
                      reserved_30b                                            :  1, // [13:13]
                      a_msdu_error                                            :  1, // [12:12]
                      eosp                                                    :  1, // [11:11]
                      more_data                                               :  1, // [10:10]
                      ctrl_type                                               :  1, // [9:9]
                      mgmt_type                                               :  1, // [8:8]
                      null_data                                               :  1, // [7:7]
                      non_qos                                                 :  1, // [6:6]
                      power_mgmt                                              :  1, // [5:5]
                      ast_index_timeout                                       :  1, // [4:4]
                      ast_index_not_found                                     :  1, // [3:3]
                      mcast_bcast                                             :  1, // [2:2]
                      reserved_30a                                            :  1, // [1:1]
                      first_mpdu                                              :  1; // [0:0]
             uint32_t msdu_done                                               :  1, // [31:31]
                      reserved_31b                                            : 17, // [30:14]
                      rx_bitmap_not_updated                                   :  1, // [13:13]
                      decrypt_status_code                                     :  3, // [12:10]
                      reserved_31a                                            : 10; // [9:0]
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

#define RX_MSDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET                            0x0000000000000000
#define RX_MSDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB                               0
#define RX_MSDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_MSB                               1
#define RX_MSDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK                              0x0000000000000003



#define RX_MSDU_END_SW_FRAME_GROUP_ID_OFFSET                                        0x0000000000000000
#define RX_MSDU_END_SW_FRAME_GROUP_ID_LSB                                           2
#define RX_MSDU_END_SW_FRAME_GROUP_ID_MSB                                           8
#define RX_MSDU_END_SW_FRAME_GROUP_ID_MASK                                          0x00000000000001fc


/* Description		RESERVED_0

			<legal 0>
*/

#define RX_MSDU_END_RESERVED_0_OFFSET                                               0x0000000000000000
#define RX_MSDU_END_RESERVED_0_LSB                                                  9
#define RX_MSDU_END_RESERVED_0_MSB                                                  15
#define RX_MSDU_END_RESERVED_0_MASK                                                 0x000000000000fe00


/* Description		PHY_PPDU_ID

			A ppdu counter value that PHY increments for every PPDU 
			received. The counter value wraps around  
			<legal all>
*/

#define RX_MSDU_END_PHY_PPDU_ID_OFFSET                                              0x0000000000000000
#define RX_MSDU_END_PHY_PPDU_ID_LSB                                                 16
#define RX_MSDU_END_PHY_PPDU_ID_MSB                                                 31
#define RX_MSDU_END_PHY_PPDU_ID_MASK                                                0x00000000ffff0000


/* Description		IP_HDR_CHKSUM

			This can include the IP header checksum or the pseudo header
			 checksum used by TCP/UDP checksum. 
			(with the first byte in the MSB and the second byte in the
			 LSB, i.e. requiring a byte-swap for little-endian FW/SW
			 w.r.t. the byte order in a packet)
*/

#define RX_MSDU_END_IP_HDR_CHKSUM_OFFSET                                            0x0000000000000000
#define RX_MSDU_END_IP_HDR_CHKSUM_LSB                                               32
#define RX_MSDU_END_IP_HDR_CHKSUM_MSB                                               47
#define RX_MSDU_END_IP_HDR_CHKSUM_MASK                                              0x0000ffff00000000


/* Description		REPORTED_MPDU_LENGTH

			MPDU length before decapsulation.  Only valid when first_msdu
			 is set.  This field is taken directly from the length field
			 of the A-MPDU delimiter or the preamble length field for
			 non-A-MPDU frames.
*/

#define RX_MSDU_END_REPORTED_MPDU_LENGTH_OFFSET                                     0x0000000000000000
#define RX_MSDU_END_REPORTED_MPDU_LENGTH_LSB                                        48
#define RX_MSDU_END_REPORTED_MPDU_LENGTH_MSB                                        61
#define RX_MSDU_END_REPORTED_MPDU_LENGTH_MASK                                       0x3fff000000000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define RX_MSDU_END_RESERVED_1A_OFFSET                                              0x0000000000000000
#define RX_MSDU_END_RESERVED_1A_LSB                                                 62
#define RX_MSDU_END_RESERVED_1A_MSB                                                 63
#define RX_MSDU_END_RESERVED_1A_MASK                                                0xc000000000000000


#define RX_MSDU_END_RESERVED_2A_OFFSET                                              0x0000000000000008
#define RX_MSDU_END_RESERVED_2A_LSB                                                 0
#define RX_MSDU_END_RESERVED_2A_MSB                                                 7
#define RX_MSDU_END_RESERVED_2A_MASK                                                0x00000000000000ff


/* Description		CCE_SUPER_RULE

			Indicates the super filter rule 
*/

#define RX_MSDU_END_CCE_SUPER_RULE_OFFSET                                           0x0000000000000008
#define RX_MSDU_END_CCE_SUPER_RULE_LSB                                              8
#define RX_MSDU_END_CCE_SUPER_RULE_MSB                                              13
#define RX_MSDU_END_CCE_SUPER_RULE_MASK                                             0x0000000000003f00


/* Description		CCE_CLASSIFY_NOT_DONE_TRUNCATE

			Classification failed due to truncated frame
*/

#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_TRUNCATE_OFFSET                           0x0000000000000008
#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_TRUNCATE_LSB                              14
#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_TRUNCATE_MSB                              14
#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_TRUNCATE_MASK                             0x0000000000004000


/* Description		CCE_CLASSIFY_NOT_DONE_CCE_DIS

			Classification failed due to CCE global disable
*/

#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_CCE_DIS_OFFSET                            0x0000000000000008
#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_CCE_DIS_LSB                               15
#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_CCE_DIS_MSB                               15
#define RX_MSDU_END_CCE_CLASSIFY_NOT_DONE_CCE_DIS_MASK                              0x0000000000008000


/* Description		CUMULATIVE_L3_CHECKSUM

			FISA: IP header checksum including the total MSDU length
			 that is part of this flow aggregated so far, reported if
			 'RXOLE_R0_FISA_CTRL. CHKSUM_CUM_IP_LEN_EN' is set
			
			Set to zero in chips not supporting FISA
			<legal all>
*/

#define RX_MSDU_END_CUMULATIVE_L3_CHECKSUM_OFFSET                                   0x0000000000000008
#define RX_MSDU_END_CUMULATIVE_L3_CHECKSUM_LSB                                      16
#define RX_MSDU_END_CUMULATIVE_L3_CHECKSUM_MSB                                      31
#define RX_MSDU_END_CUMULATIVE_L3_CHECKSUM_MASK                                     0x00000000ffff0000


/* Description		RULE_INDICATION_31_0

			Bitmap indicating which of rules 31-0 have matched
			
			In chips with more than 64 CCE rules, RXOLE
			 shall have a configuration to report any two rule_indication_* 
			in 'RX_MSDU_END.'
*/

#define RX_MSDU_END_RULE_INDICATION_31_0_OFFSET                                     0x0000000000000008
#define RX_MSDU_END_RULE_INDICATION_31_0_LSB                                        32
#define RX_MSDU_END_RULE_INDICATION_31_0_MSB                                        63
#define RX_MSDU_END_RULE_INDICATION_31_0_MASK                                       0xffffffff00000000


#define RX_MSDU_END_IPV6_OPTIONS_CRC_OFFSET                                         0x0000000000000010
#define RX_MSDU_END_IPV6_OPTIONS_CRC_LSB                                            0
#define RX_MSDU_END_IPV6_OPTIONS_CRC_MSB                                            31
#define RX_MSDU_END_IPV6_OPTIONS_CRC_MASK                                           0x00000000ffffffff


/* Description		DA_OFFSET

			Offset into MSDU buffer for DA
*/

#define RX_MSDU_END_DA_OFFSET_OFFSET                                                0x0000000000000010
#define RX_MSDU_END_DA_OFFSET_LSB                                                   32
#define RX_MSDU_END_DA_OFFSET_MSB                                                   37
#define RX_MSDU_END_DA_OFFSET_MASK                                                  0x0000003f00000000


/* Description		SA_OFFSET

			Offset into MSDU buffer for SA
*/

#define RX_MSDU_END_SA_OFFSET_OFFSET                                                0x0000000000000010
#define RX_MSDU_END_SA_OFFSET_LSB                                                   38
#define RX_MSDU_END_SA_OFFSET_MSB                                                   43
#define RX_MSDU_END_SA_OFFSET_MASK                                                  0x00000fc000000000


/* Description		DA_OFFSET_VALID

			da_offset field is valid. This will be set to 0 in case 
			of a dynamic A-MSDU when DA is compressed
*/

#define RX_MSDU_END_DA_OFFSET_VALID_OFFSET                                          0x0000000000000010
#define RX_MSDU_END_DA_OFFSET_VALID_LSB                                             44
#define RX_MSDU_END_DA_OFFSET_VALID_MSB                                             44
#define RX_MSDU_END_DA_OFFSET_VALID_MASK                                            0x0000100000000000


/* Description		SA_OFFSET_VALID

			sa_offset field is valid. This will be set to 0 in case 
			of a dynamic A-MSDU when SA is compressed
*/

#define RX_MSDU_END_SA_OFFSET_VALID_OFFSET                                          0x0000000000000010
#define RX_MSDU_END_SA_OFFSET_VALID_LSB                                             45
#define RX_MSDU_END_SA_OFFSET_VALID_MSB                                             45
#define RX_MSDU_END_SA_OFFSET_VALID_MASK                                            0x0000200000000000


/* Description		RESERVED_5A

			<legal 0>
*/

#define RX_MSDU_END_RESERVED_5A_OFFSET                                              0x0000000000000010
#define RX_MSDU_END_RESERVED_5A_LSB                                                 46
#define RX_MSDU_END_RESERVED_5A_MSB                                                 47
#define RX_MSDU_END_RESERVED_5A_MASK                                                0x0000c00000000000


/* Description		L3_TYPE

			The 16-bit type value indicating the type of L3 later extracted
			 from LLC/SNAP, set to zero if SNAP is not available
*/

#define RX_MSDU_END_L3_TYPE_OFFSET                                                  0x0000000000000010
#define RX_MSDU_END_L3_TYPE_LSB                                                     48
#define RX_MSDU_END_L3_TYPE_MSB                                                     63
#define RX_MSDU_END_L3_TYPE_MASK                                                    0xffff000000000000


/* Description		RULE_INDICATION_63_32

			Bitmap indicating which of rules 63-32 have matched
			
			In chips with more than 64 CCE rules, RXOLE
			 shall have a configuration to report any two rule_indication_* 
			in 'RX_MSDU_END.'
			
*/

#define RX_MSDU_END_RULE_INDICATION_63_32_OFFSET                                    0x0000000000000018
#define RX_MSDU_END_RULE_INDICATION_63_32_LSB                                       0
#define RX_MSDU_END_RULE_INDICATION_63_32_MSB                                       31
#define RX_MSDU_END_RULE_INDICATION_63_32_MASK                                      0x00000000ffffffff


/* Description		TCP_SEQ_NUMBER

			TCP sequence number (as a number assembled from a TCP packet
			 in big-endian order, i.e. requiring a byte-swap for little-endian
			 FW/SW w.r.t. the byte order in a packet)
			
			If 'RXOLE_R0_MISC_CONFIG. OVERRIDE_MSDU_END_FIELDS'
			is set, toeplitz_hash_2_or_4 from 'RX_MSDU_START' will be
			 reported here:
			Controlled by multiple RxOLE registers for TCP/UDP over 
			IPv4/IPv6 - Either Toeplitz hash computed over 2-tuple IPv4
			 or IPv6 src/dest addresses is reported; or, Toeplitz hash
			 computed over 4-tuple IPv4 or IPv6 src/dest addresses and
			 src/dest ports is reported. The Flow_id_toeplitz hash can
			 also be reported here. Usually the hash reported here is
			 the one used for hash-based REO routing (see use_flow_id_toeplitz_clfy
			 in 'RXPT_CLASSIFY_INFO'). Optionally the 3-tuple Toeplitz
			 hash over IPv4 or IPv6 src/dest addresses and L4 protocol
			 can be reported here.
*/

#define RX_MSDU_END_TCP_SEQ_NUMBER_OFFSET                                           0x0000000000000018
#define RX_MSDU_END_TCP_SEQ_NUMBER_LSB                                              32
#define RX_MSDU_END_TCP_SEQ_NUMBER_MSB                                              63
#define RX_MSDU_END_TCP_SEQ_NUMBER_MASK                                             0xffffffff00000000


/* Description		TCP_ACK_NUMBER

			TCP acknowledge number (as a number assembled from a TCP
			 packet in big-endian order, i.e. requiring a byte-swap 
			for little-endian FW/SW w.r.t. the byte order in a packet)
			
			
			If 'RXOLE_R0_MISC_CONFIG. OVERRIDE_MSDU_END_FIELDS'
			is set, flow_id_toeplitz from 'RX_MSDU_START' will be reported
			 here:
			Toeplitz hash of 5-tuple {IP source address, IP destination
			 address, IP source port, IP destination port, L4 protocol} 
			 in case of non-IPSec. In case of IPSec - Toeplitz hash 
			of 4-tuple {IP source address, IP destination address, SPI, 
			L4 protocol}. Optionally the 3-tuple Toeplitz hash over 
			IPv4 or IPv6 src/dest addresses and L4 protocol can be reported
			 here. 
			The relevant Toeplitz key registers are provided in RxOLE's
			 instance of common parser module. These registers are separate
			 from the Toeplitz keys used by ASE/FSE modules inside RxOLE. 
			The actual value will be passed on from common parser module
			 to RxOLE in one of the WHO_* TLVs.
*/

#define RX_MSDU_END_TCP_ACK_NUMBER_OFFSET                                           0x0000000000000020
#define RX_MSDU_END_TCP_ACK_NUMBER_LSB                                              0
#define RX_MSDU_END_TCP_ACK_NUMBER_MSB                                              31
#define RX_MSDU_END_TCP_ACK_NUMBER_MASK                                             0x00000000ffffffff


/* Description		TCP_FLAG

			TCP flags
			{NS,CWR,ECE,URG,ACK,PSH, RST ,SYN,FIN}(with the NS bit in
			 bit 8 and the FIN bit in bit 0, i.e. in big-endian order, 
			i.e. requiring a byte-swap for little-endian FW/SW w.r.t. 
			the byte order in a packet)
*/

#define RX_MSDU_END_TCP_FLAG_OFFSET                                                 0x0000000000000020
#define RX_MSDU_END_TCP_FLAG_LSB                                                    32
#define RX_MSDU_END_TCP_FLAG_MSB                                                    40
#define RX_MSDU_END_TCP_FLAG_MASK                                                   0x000001ff00000000


/* Description		LRO_ELIGIBLE

			Computed out of TCP and IP fields to indicate that this 
			MSDU is eligible for  LRO
*/

#define RX_MSDU_END_LRO_ELIGIBLE_OFFSET                                             0x0000000000000020
#define RX_MSDU_END_LRO_ELIGIBLE_LSB                                                41
#define RX_MSDU_END_LRO_ELIGIBLE_MSB                                                41
#define RX_MSDU_END_LRO_ELIGIBLE_MASK                                               0x0000020000000000


/* Description		RESERVED_9A

			NOTE: DO not assign a field... Internally used in RXOLE..
			
			<legal 0>
*/

#define RX_MSDU_END_RESERVED_9A_OFFSET                                              0x0000000000000020
#define RX_MSDU_END_RESERVED_9A_LSB                                                 42
#define RX_MSDU_END_RESERVED_9A_MSB                                                 47
#define RX_MSDU_END_RESERVED_9A_MASK                                                0x0000fc0000000000


/* Description		WINDOW_SIZE

			TCP receive window size (as a number assembled from a TCP
			 packet in big-endian order, i.e. requiring a byte-swap 
			for little-endian FW/SW w.r.t. the byte order in a packet)
			
			
			If 'RXOLE_R0_MISC_CONFIG. OVERRIDE_MSDU_END_FIELDS'
			is set, msdu_length from 'RX_MSDU_START' will be reported
			 in the 14 LSBs here:
			MSDU length in bytes after decapsulation. This field is 
			still valid for MPDU frames without A-MSDU.  It still represents
			 MSDU length after decapsulation.
*/

#define RX_MSDU_END_WINDOW_SIZE_OFFSET                                              0x0000000000000020
#define RX_MSDU_END_WINDOW_SIZE_LSB                                                 48
#define RX_MSDU_END_WINDOW_SIZE_MSB                                                 63
#define RX_MSDU_END_WINDOW_SIZE_MASK                                                0xffff000000000000


/* Description		SA_SW_PEER_ID

			sw_peer_id from the address search entry corresponding to
			 the source address of the MSDU
			
			<legal all>
*/

#define RX_MSDU_END_SA_SW_PEER_ID_OFFSET                                            0x0000000000000028
#define RX_MSDU_END_SA_SW_PEER_ID_LSB                                               0
#define RX_MSDU_END_SA_SW_PEER_ID_MSB                                               15
#define RX_MSDU_END_SA_SW_PEER_ID_MASK                                              0x000000000000ffff


/* Description		SA_IDX_TIMEOUT

			Indicates an unsuccessful MAC source address search due 
			to the expiring of the search timer.
*/

#define RX_MSDU_END_SA_IDX_TIMEOUT_OFFSET                                           0x0000000000000028
#define RX_MSDU_END_SA_IDX_TIMEOUT_LSB                                              16
#define RX_MSDU_END_SA_IDX_TIMEOUT_MSB                                              16
#define RX_MSDU_END_SA_IDX_TIMEOUT_MASK                                             0x0000000000010000


/* Description		DA_IDX_TIMEOUT

			Indicates an unsuccessful MAC destination address search
			 due to the expiring of the search timer.
*/

#define RX_MSDU_END_DA_IDX_TIMEOUT_OFFSET                                           0x0000000000000028
#define RX_MSDU_END_DA_IDX_TIMEOUT_LSB                                              17
#define RX_MSDU_END_DA_IDX_TIMEOUT_MSB                                              17
#define RX_MSDU_END_DA_IDX_TIMEOUT_MASK                                             0x0000000000020000


/* Description		TO_DS

			Set if the to DS bit is set in the frame control.
			
			RXOLE copies this from the 'Struct RX_MPDU_INFO' in 'RX_MPDU_PCU_START.'
			
			
			<legal all>
*/

#define RX_MSDU_END_TO_DS_OFFSET                                                    0x0000000000000028
#define RX_MSDU_END_TO_DS_LSB                                                       18
#define RX_MSDU_END_TO_DS_MSB                                                       18
#define RX_MSDU_END_TO_DS_MASK                                                      0x0000000000040000



#define RX_MSDU_END_TID_OFFSET                                                      0x0000000000000028
#define RX_MSDU_END_TID_LSB                                                         19
#define RX_MSDU_END_TID_MSB                                                         22
#define RX_MSDU_END_TID_MASK                                                        0x0000000000780000


/* Description		SA_IS_VALID

			Indicates that OLE found a valid SA entry
*/

#define RX_MSDU_END_SA_IS_VALID_OFFSET                                              0x0000000000000028
#define RX_MSDU_END_SA_IS_VALID_LSB                                                 23
#define RX_MSDU_END_SA_IS_VALID_MSB                                                 23
#define RX_MSDU_END_SA_IS_VALID_MASK                                                0x0000000000800000


/* Description		DA_IS_VALID

			Indicates that OLE found a valid DA entry
*/

#define RX_MSDU_END_DA_IS_VALID_OFFSET                                              0x0000000000000028
#define RX_MSDU_END_DA_IS_VALID_LSB                                                 24
#define RX_MSDU_END_DA_IS_VALID_MSB                                                 24
#define RX_MSDU_END_DA_IS_VALID_MASK                                                0x0000000001000000


/* Description		DA_IS_MCBC

			Field Only valid if "da_is_valid" is set
			
			Indicates the DA address was a Multicast of Broadcast address.
			
*/

#define RX_MSDU_END_DA_IS_MCBC_OFFSET                                               0x0000000000000028
#define RX_MSDU_END_DA_IS_MCBC_LSB                                                  25
#define RX_MSDU_END_DA_IS_MCBC_MSB                                                  25
#define RX_MSDU_END_DA_IS_MCBC_MASK                                                 0x0000000002000000


/* Description		L3_HEADER_PADDING

			Number of bytes padded  to make sure that the L3 header 
			will always start of a Dword   boundary
*/

#define RX_MSDU_END_L3_HEADER_PADDING_OFFSET                                        0x0000000000000028
#define RX_MSDU_END_L3_HEADER_PADDING_LSB                                           26
#define RX_MSDU_END_L3_HEADER_PADDING_MSB                                           27
#define RX_MSDU_END_L3_HEADER_PADDING_MASK                                          0x000000000c000000


/* Description		FIRST_MSDU

			Indicates the first MSDU of A-MSDU.  If both first_msdu 
			and last_msdu are set in the MSDU then this is a non-aggregated
			 MSDU frame: normal MPDU.  Interior MSDU in an A-MSDU shall
			 have both first_mpdu and last_mpdu bits set to 0.
*/

#define RX_MSDU_END_FIRST_MSDU_OFFSET                                               0x0000000000000028
#define RX_MSDU_END_FIRST_MSDU_LSB                                                  28
#define RX_MSDU_END_FIRST_MSDU_MSB                                                  28
#define RX_MSDU_END_FIRST_MSDU_MASK                                                 0x0000000010000000


/* Description		LAST_MSDU

			Indicates the last MSDU of the A-MSDU.  MPDU end status 
			is only valid when last_msdu is set.
*/

#define RX_MSDU_END_LAST_MSDU_OFFSET                                                0x0000000000000028
#define RX_MSDU_END_LAST_MSDU_LSB                                                   29
#define RX_MSDU_END_LAST_MSDU_MSB                                                   29
#define RX_MSDU_END_LAST_MSDU_MASK                                                  0x0000000020000000


/* Description		FR_DS

			Set if the from DS bit is set in the frame control.
			
			RXOLE copies this from the 'Struct RX_MPDU_INFO' in 'RX_MPDU_PCU_START.'
			
			<legal all>
*/

#define RX_MSDU_END_FR_DS_OFFSET                                                    0x0000000000000028
#define RX_MSDU_END_FR_DS_LSB                                                       30
#define RX_MSDU_END_FR_DS_MSB                                                       30
#define RX_MSDU_END_FR_DS_MASK                                                      0x0000000040000000


/* Description		IP_CHKSUM_FAIL_COPY

			If 'RXOLE_R0_MISC_CONFIG. OVERRIDE_MSDU_END_FIELDS' is set, 
			ip_chksum_fail from 'RX_ATTENTION' will be reported in the
			 MSB here:
			Indicates that the computed checksum (ip_hdr_chksum) did
			 not match the checksum in the IP header.
*/

#define RX_MSDU_END_IP_CHKSUM_FAIL_COPY_OFFSET                                      0x0000000000000028
#define RX_MSDU_END_IP_CHKSUM_FAIL_COPY_LSB                                         31
#define RX_MSDU_END_IP_CHKSUM_FAIL_COPY_MSB                                         31
#define RX_MSDU_END_IP_CHKSUM_FAIL_COPY_MASK                                        0x0000000080000000


/* Description		SA_IDX

			The offset in the address table which matches the MAC source
			 address.
*/

#define RX_MSDU_END_SA_IDX_OFFSET                                                   0x0000000000000028
#define RX_MSDU_END_SA_IDX_LSB                                                      32
#define RX_MSDU_END_SA_IDX_MSB                                                      47
#define RX_MSDU_END_SA_IDX_MASK                                                     0x0000ffff00000000


/* Description		DA_IDX_OR_SW_PEER_ID

			Based on a register configuration in RXOLE, this field will
			 contain:
			The offset in the address table which matches the MAC destination
			 address
			OR:
			sw_peer_id from the address search entry corresponding to
			 the destination address of the MSDU
*/

#define RX_MSDU_END_DA_IDX_OR_SW_PEER_ID_OFFSET                                     0x0000000000000028
#define RX_MSDU_END_DA_IDX_OR_SW_PEER_ID_LSB                                        48
#define RX_MSDU_END_DA_IDX_OR_SW_PEER_ID_MSB                                        63
#define RX_MSDU_END_DA_IDX_OR_SW_PEER_ID_MASK                                       0xffff000000000000


/* Description		MSDU_DROP

			When set, REO shall drop this MSDU and not forward it to
			 any other ring...
			<legal all>
*/

#define RX_MSDU_END_MSDU_DROP_OFFSET                                                0x0000000000000030
#define RX_MSDU_END_MSDU_DROP_LSB                                                   0
#define RX_MSDU_END_MSDU_DROP_MSB                                                   0
#define RX_MSDU_END_MSDU_DROP_MASK                                                  0x0000000000000001


/* Description		REO_DESTINATION_INDICATION

			The ID of the REO exit ring where the MSDU frame shall push
			 after (MPDU level) reordering has finished.
			
			<enum 0 reo_destination_sw0> Reo will push the frame into
			 the REO2SW0 ring
			<enum 1 reo_destination_sw1> Reo will push the frame into
			 the REO2SW1 ring
			<enum 2 reo_destination_sw2> Reo will push the frame into
			 the REO2SW2 ring
			<enum 3 reo_destination_sw3> Reo will push the frame into
			 the REO2SW3 ring
			<enum 4 reo_destination_sw4> Reo will push the frame into
			 the REO2SW4 ring
			<enum 5 reo_destination_release> Reo will push the frame
			 into the REO_release ring
			<enum 6 reo_destination_fw> Reo will push the frame into
			 the REO2FW ring
			<enum 7 reo_destination_sw5> Reo will push the frame into
			 the REO2SW5 ring (REO remaps this in chips without REO2SW5
			 ring)
			<enum 8 reo_destination_sw6> Reo will push the frame into
			 the REO2SW6 ring (REO remaps this in chips without REO2SW6
			 ring)
			<enum 9 reo_destination_sw7> Reo will push the frame into
			 the REO2SW7 ring (REO remaps this in chips without REO2SW7
			 ring)
			<enum 10 reo_destination_sw8> Reo will push the frame into
			 the REO2SW8 ring (REO remaps this in chips without REO2SW8
			 ring)
			<enum 11 reo_destination_11> REO remaps this 
			<enum 12 reo_destination_12> REO remaps this <enum 13 reo_destination_13> 
			REO remaps this 
			<enum 14 reo_destination_14> REO remaps this 
			<enum 15 reo_destination_15> REO remaps this 
			<enum 16 reo_destination_16> REO remaps this 
			<enum 17 reo_destination_17> REO remaps this 
			<enum 18 reo_destination_18> REO remaps this 
			<enum 19 reo_destination_19> REO remaps this 
			<enum 20 reo_destination_20> REO remaps this 
			<enum 21 reo_destination_21> REO remaps this 
			<enum 22 reo_destination_22> REO remaps this 
			<enum 23 reo_destination_23> REO remaps this 
			<enum 24 reo_destination_24> REO remaps this 
			<enum 25 reo_destination_25> REO remaps this 
			<enum 26 reo_destination_26> REO remaps this 
			<enum 27 reo_destination_27> REO remaps this 
			<enum 28 reo_destination_28> REO remaps this 
			<enum 29 reo_destination_29> REO remaps this 
			<enum 30 reo_destination_30> REO remaps this 
			<enum 31 reo_destination_31> REO remaps this 
			
			<legal all>
*/

#define RX_MSDU_END_REO_DESTINATION_INDICATION_OFFSET                               0x0000000000000030
#define RX_MSDU_END_REO_DESTINATION_INDICATION_LSB                                  1
#define RX_MSDU_END_REO_DESTINATION_INDICATION_MSB                                  5
#define RX_MSDU_END_REO_DESTINATION_INDICATION_MASK                                 0x000000000000003e


/* Description		FLOW_IDX

			Flow table index
			<legal all>
*/

#define RX_MSDU_END_FLOW_IDX_OFFSET                                                 0x0000000000000030
#define RX_MSDU_END_FLOW_IDX_LSB                                                    6
#define RX_MSDU_END_FLOW_IDX_MSB                                                    25
#define RX_MSDU_END_FLOW_IDX_MASK                                                   0x0000000003ffffc0


/* Description		USE_PPE

			Indicates to RXDMA to ignore the REO_destination_indication
			 and use a programmed value corresponding to the REO2PPE
			 ring
			
			This override to REO2PPE for packets requiring multiple 
			buffers shall be disabled based on an RXDMA configuration, 
			as PPE may not support such packets.
			<legal all>
*/

#define RX_MSDU_END_USE_PPE_OFFSET                                                  0x0000000000000030
#define RX_MSDU_END_USE_PPE_LSB                                                     26
#define RX_MSDU_END_USE_PPE_MSB                                                     26
#define RX_MSDU_END_USE_PPE_MASK                                                    0x0000000004000000


/* Description		MESH_STA

			When set, this is a Mesh (11s) STA.
			
			The interpretation of the A-MSDU 'Length' field in the MPDU
			 (if any) is decided by the e-numerations below.
			
			<enum 0 MESH_DISABLE>
			<enum 1 MESH_Q2Q> A-MSDU 'Length' is big endian and includes
			 the length of Mesh Control.
			<enum 2 MESH_11S_BE> A-MSDU 'Length' is big endian and excludes
			 the length of Mesh Control.
			<enum 3 MESH_11S_LE> A-MSDU 'Length' is little endian and
			 excludes the length of Mesh Control. This is 802.11s-compliant.
			
			<legal all>
*/

#define RX_MSDU_END_MESH_STA_OFFSET                                                 0x0000000000000030
#define RX_MSDU_END_MESH_STA_LSB                                                    27
#define RX_MSDU_END_MESH_STA_MSB                                                    28
#define RX_MSDU_END_MESH_STA_MASK                                                   0x0000000018000000


/* Description		VLAN_CTAG_STRIPPED

			Set by RXOLE if it stripped 4-bytes of C-VLAN Tag from the
			 packet
			<legal all>
*/

#define RX_MSDU_END_VLAN_CTAG_STRIPPED_OFFSET                                       0x0000000000000030
#define RX_MSDU_END_VLAN_CTAG_STRIPPED_LSB                                          29
#define RX_MSDU_END_VLAN_CTAG_STRIPPED_MSB                                          29
#define RX_MSDU_END_VLAN_CTAG_STRIPPED_MASK                                         0x0000000020000000


/* Description		VLAN_STAG_STRIPPED

			Set by RXOLE if it stripped 4-bytes of S-VLAN Tag from the
			 packet
			<legal all>
*/

#define RX_MSDU_END_VLAN_STAG_STRIPPED_OFFSET                                       0x0000000000000030
#define RX_MSDU_END_VLAN_STAG_STRIPPED_LSB                                          30
#define RX_MSDU_END_VLAN_STAG_STRIPPED_MSB                                          30
#define RX_MSDU_END_VLAN_STAG_STRIPPED_MASK                                         0x0000000040000000


/* Description		FRAGMENT_FLAG

			Indicates that this is an 802.11 fragment frame.  This is
			 set when either the more_frag bit is set in the frame control
			 or the fragment number is not zero.  Only set when first_msdu
			 is set.
*/

#define RX_MSDU_END_FRAGMENT_FLAG_OFFSET                                            0x0000000000000030
#define RX_MSDU_END_FRAGMENT_FLAG_LSB                                               31
#define RX_MSDU_END_FRAGMENT_FLAG_MSB                                               31
#define RX_MSDU_END_FRAGMENT_FLAG_MASK                                              0x0000000080000000


/* Description		FSE_METADATA

			FSE related meta data:
			<legal all>
*/

#define RX_MSDU_END_FSE_METADATA_OFFSET                                             0x0000000000000030
#define RX_MSDU_END_FSE_METADATA_LSB                                                32
#define RX_MSDU_END_FSE_METADATA_MSB                                                63
#define RX_MSDU_END_FSE_METADATA_MASK                                               0xffffffff00000000


/* Description		CCE_METADATA

			CCE related meta data:
			<legal all>
*/

#define RX_MSDU_END_CCE_METADATA_OFFSET                                             0x0000000000000038
#define RX_MSDU_END_CCE_METADATA_LSB                                                0
#define RX_MSDU_END_CCE_METADATA_MSB                                                15
#define RX_MSDU_END_CCE_METADATA_MASK                                               0x000000000000ffff



#define RX_MSDU_END_TCP_UDP_CHKSUM_OFFSET                                           0x0000000000000038
#define RX_MSDU_END_TCP_UDP_CHKSUM_LSB                                              16
#define RX_MSDU_END_TCP_UDP_CHKSUM_MSB                                              31
#define RX_MSDU_END_TCP_UDP_CHKSUM_MASK                                             0x00000000ffff0000


/* Description		AGGREGATION_COUNT

			FISA: Number of MSDU's aggregated so far
			
			Set to zero in chips not supporting FISA
			<legal all>
*/

#define RX_MSDU_END_AGGREGATION_COUNT_OFFSET                                        0x0000000000000038
#define RX_MSDU_END_AGGREGATION_COUNT_LSB                                           32
#define RX_MSDU_END_AGGREGATION_COUNT_MSB                                           39
#define RX_MSDU_END_AGGREGATION_COUNT_MASK                                          0x000000ff00000000


/* Description		FLOW_AGGREGATION_CONTINUATION

			FISA: To indicate that this MSDU can be aggregated with 
			the previous packet with the same flow id
			
			Set to zero in chips not supporting FISA
			<legal all>
*/

#define RX_MSDU_END_FLOW_AGGREGATION_CONTINUATION_OFFSET                            0x0000000000000038
#define RX_MSDU_END_FLOW_AGGREGATION_CONTINUATION_LSB                               40
#define RX_MSDU_END_FLOW_AGGREGATION_CONTINUATION_MSB                               40
#define RX_MSDU_END_FLOW_AGGREGATION_CONTINUATION_MASK                              0x0000010000000000


/* Description		FISA_TIMEOUT

			FISA: To indicate that the aggregation has restarted for
			 this flow due to timeout
			
			Set to zero in chips not supporting FISA
			<legal all>
*/

#define RX_MSDU_END_FISA_TIMEOUT_OFFSET                                             0x0000000000000038
#define RX_MSDU_END_FISA_TIMEOUT_LSB                                                41
#define RX_MSDU_END_FISA_TIMEOUT_MSB                                                41
#define RX_MSDU_END_FISA_TIMEOUT_MASK                                               0x0000020000000000


/* Description		TCP_UDP_CHKSUM_FAIL_COPY

			if 'RXOLE_R0_MISC_CONFIG. OVERRIDE_MSDU_END_FIELDS' is set, 
			tcp_udp_chksum_fail from 'RX_ATTENTION' will be reported
			 here:
			Indicates that the computed checksum (tcp_udp_chksum) did
			 not match the checksum in the TCP/UDP header.
*/

#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_COPY_OFFSET                                 0x0000000000000038
#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_COPY_LSB                                    42
#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_COPY_MSB                                    42
#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_COPY_MASK                                   0x0000040000000000


/* Description		MSDU_LIMIT_ERROR

			Indicates that the MSDU threshold was exceeded and thus 
			all the rest of the MSDUs will not be scattered and will
			 not be decapsulated but will be DMA'ed in RAW format as
			 a single MSDU buffer
*/

#define RX_MSDU_END_MSDU_LIMIT_ERROR_OFFSET                                         0x0000000000000038
#define RX_MSDU_END_MSDU_LIMIT_ERROR_LSB                                            43
#define RX_MSDU_END_MSDU_LIMIT_ERROR_MSB                                            43
#define RX_MSDU_END_MSDU_LIMIT_ERROR_MASK                                           0x0000080000000000


/* Description		FLOW_IDX_TIMEOUT

			Indicates an unsuccessful flow search due to the expiring
			 of the search timer.
			<legal all>
*/

#define RX_MSDU_END_FLOW_IDX_TIMEOUT_OFFSET                                         0x0000000000000038
#define RX_MSDU_END_FLOW_IDX_TIMEOUT_LSB                                            44
#define RX_MSDU_END_FLOW_IDX_TIMEOUT_MSB                                            44
#define RX_MSDU_END_FLOW_IDX_TIMEOUT_MASK                                           0x0000100000000000


/* Description		FLOW_IDX_INVALID

			flow id is not valid
			<legal all>
*/

#define RX_MSDU_END_FLOW_IDX_INVALID_OFFSET                                         0x0000000000000038
#define RX_MSDU_END_FLOW_IDX_INVALID_LSB                                            45
#define RX_MSDU_END_FLOW_IDX_INVALID_MSB                                            45
#define RX_MSDU_END_FLOW_IDX_INVALID_MASK                                           0x0000200000000000


/* Description		CCE_MATCH

			Indicates that this status has a corresponding MSDU that
			 requires FW processing.  The OLE will have classification
			 ring mask registers which will indicate the ring(s) for
			 packets and descriptors which need FW attention.
*/

#define RX_MSDU_END_CCE_MATCH_OFFSET                                                0x0000000000000038
#define RX_MSDU_END_CCE_MATCH_LSB                                                   46
#define RX_MSDU_END_CCE_MATCH_MSB                                                   46
#define RX_MSDU_END_CCE_MATCH_MASK                                                  0x0000400000000000


/* Description		AMSDU_PARSER_ERROR

			A-MSDU could not be properly de-agregated.
			<legal all>
*/

#define RX_MSDU_END_AMSDU_PARSER_ERROR_OFFSET                                       0x0000000000000038
#define RX_MSDU_END_AMSDU_PARSER_ERROR_LSB                                          47
#define RX_MSDU_END_AMSDU_PARSER_ERROR_MSB                                          47
#define RX_MSDU_END_AMSDU_PARSER_ERROR_MASK                                         0x0000800000000000


/* Description		CUMULATIVE_IP_LENGTH

			FISA: Total MSDU length that is part of this flow aggregated
			 so far
			
			Set to zero in chips not supporting FISA
			<legal all>
*/

#define RX_MSDU_END_CUMULATIVE_IP_LENGTH_OFFSET                                     0x0000000000000038
#define RX_MSDU_END_CUMULATIVE_IP_LENGTH_LSB                                        48
#define RX_MSDU_END_CUMULATIVE_IP_LENGTH_MSB                                        63
#define RX_MSDU_END_CUMULATIVE_IP_LENGTH_MASK                                       0xffff000000000000


/* Description		KEY_ID_OCTET

			The key ID octet from the IV.  Only valid when first_msdu
			 is set.
*/

#define RX_MSDU_END_KEY_ID_OCTET_OFFSET                                             0x0000000000000040
#define RX_MSDU_END_KEY_ID_OCTET_LSB                                                0
#define RX_MSDU_END_KEY_ID_OCTET_MSB                                                7
#define RX_MSDU_END_KEY_ID_OCTET_MASK                                               0x00000000000000ff



#define RX_MSDU_END_RESERVED_16A_OFFSET                                             0x0000000000000040
#define RX_MSDU_END_RESERVED_16A_LSB                                                8
#define RX_MSDU_END_RESERVED_16A_MSB                                                31
#define RX_MSDU_END_RESERVED_16A_MASK                                               0x00000000ffffff00


/* Description		RESERVED_17A

			<legal 0>
*/

#define RX_MSDU_END_RESERVED_17A_OFFSET                                             0x0000000000000040
#define RX_MSDU_END_RESERVED_17A_LSB                                                32
#define RX_MSDU_END_RESERVED_17A_MSB                                                37
#define RX_MSDU_END_RESERVED_17A_MASK                                               0x0000003f00000000


/* Description		SERVICE_CODE

			Opaque service code between PPE and Wi-Fi
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_END_SERVICE_CODE_OFFSET                                             0x0000000000000040
#define RX_MSDU_END_SERVICE_CODE_LSB                                                38
#define RX_MSDU_END_SERVICE_CODE_MSB                                                46
#define RX_MSDU_END_SERVICE_CODE_MASK                                               0x00007fc000000000


/* Description		PRIORITY_VALID

			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_END_PRIORITY_VALID_OFFSET                                           0x0000000000000040
#define RX_MSDU_END_PRIORITY_VALID_LSB                                              47
#define RX_MSDU_END_PRIORITY_VALID_MSB                                              47
#define RX_MSDU_END_PRIORITY_VALID_MASK                                             0x0000800000000000


/* Description		INTRA_BSS

			This packet needs intra-BSS routing by SW as the 'vdev_id' 
			for the destination is the same as the 'vdev_id' (from 'RX_MPDU_PCU_START') 
			that this MSDU was got in.
			
			<legal all>
*/

#define RX_MSDU_END_INTRA_BSS_OFFSET                                                0x0000000000000040
#define RX_MSDU_END_INTRA_BSS_LSB                                                   48
#define RX_MSDU_END_INTRA_BSS_MSB                                                   48
#define RX_MSDU_END_INTRA_BSS_MASK                                                  0x0001000000000000


/* Description		DEST_CHIP_ID

			If intra_bss is set, copied by RXOLE from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which chip's TCL the packet should be
			 queued.
			
			<legal all>
*/

#define RX_MSDU_END_DEST_CHIP_ID_OFFSET                                             0x0000000000000040
#define RX_MSDU_END_DEST_CHIP_ID_LSB                                                49
#define RX_MSDU_END_DEST_CHIP_ID_MSB                                                50
#define RX_MSDU_END_DEST_CHIP_ID_MASK                                               0x0006000000000000


/* Description		MULTICAST_ECHO

			If set, this packet is a multicast echo, i.e. the DA is 
			multicast and Rx OLE SA search with mcast_echo_check = 1
			 passed. RXDMA should release such packets to WBM.
			
			<legal all>
*/

#define RX_MSDU_END_MULTICAST_ECHO_OFFSET                                           0x0000000000000040
#define RX_MSDU_END_MULTICAST_ECHO_LSB                                              51
#define RX_MSDU_END_MULTICAST_ECHO_MSB                                              51
#define RX_MSDU_END_MULTICAST_ECHO_MASK                                             0x0008000000000000


/* Description		WDS_LEARNING_EVENT

			If set, this packet has an SA search failure with WDS learning
			 enabled for the peer. RXOLE should route this TLV to the
			 RXDMA0 status ring to notify FW.
			
			<legal all>
*/

#define RX_MSDU_END_WDS_LEARNING_EVENT_OFFSET                                       0x0000000000000040
#define RX_MSDU_END_WDS_LEARNING_EVENT_LSB                                          52
#define RX_MSDU_END_WDS_LEARNING_EVENT_MSB                                          52
#define RX_MSDU_END_WDS_LEARNING_EVENT_MASK                                         0x0010000000000000


/* Description		WDS_ROAMING_EVENT

			If set, this packet's SA 'Sw_peer_id' mismatches the 'Sw_peer_id' 
			of the peer through which the packet was got, indicating
			 the SA node has roamed. RXOLE should route this TLV to 
			the RXDMA0 status ring to notify FW.
			
			<legal all>
*/

#define RX_MSDU_END_WDS_ROAMING_EVENT_OFFSET                                        0x0000000000000040
#define RX_MSDU_END_WDS_ROAMING_EVENT_LSB                                           53
#define RX_MSDU_END_WDS_ROAMING_EVENT_MSB                                           53
#define RX_MSDU_END_WDS_ROAMING_EVENT_MASK                                          0x0020000000000000


/* Description		WDS_KEEP_ALIVE_EVENT

			If set, the AST timestamp for this packet's SA is older 
			than the current timestamp by more than a threshold programmed
			 in RXOLE. RXOLE should route this TLV to the RXDMA0 status
			 ring to notify FW to keep the AST entry for the SA alive.
			
			
			<legal all>
*/

#define RX_MSDU_END_WDS_KEEP_ALIVE_EVENT_OFFSET                                     0x0000000000000040
#define RX_MSDU_END_WDS_KEEP_ALIVE_EVENT_LSB                                        54
#define RX_MSDU_END_WDS_KEEP_ALIVE_EVENT_MSB                                        54
#define RX_MSDU_END_WDS_KEEP_ALIVE_EVENT_MASK                                       0x0040000000000000


/* Description		DEST_CHIP_PMAC_ID

			If intra_bss is set, copied by RXOLE from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which link/'vdev' the packet should 
			be queued in TCL.
			
			<legal all>
*/

#define RX_MSDU_END_DEST_CHIP_PMAC_ID_OFFSET                                        0x0000000000000040
#define RX_MSDU_END_DEST_CHIP_PMAC_ID_LSB                                           55
#define RX_MSDU_END_DEST_CHIP_PMAC_ID_MSB                                           55
#define RX_MSDU_END_DEST_CHIP_PMAC_ID_MASK                                          0x0080000000000000


/* Description		RESERVED_17B

			<legal 0>
*/

#define RX_MSDU_END_RESERVED_17B_OFFSET                                             0x0000000000000040
#define RX_MSDU_END_RESERVED_17B_LSB                                                56
#define RX_MSDU_END_RESERVED_17B_MSB                                                63
#define RX_MSDU_END_RESERVED_17B_MASK                                               0xff00000000000000


/* Description		MSDU_LENGTH

			MSDU length in bytes after decapsulation. 
			
			This field is still valid for MPDU frames without A-MSDU. 
			 It still represents MSDU length after decapsulation
*/

#define RX_MSDU_END_MSDU_LENGTH_OFFSET                                              0x0000000000000048
#define RX_MSDU_END_MSDU_LENGTH_LSB                                                 0
#define RX_MSDU_END_MSDU_LENGTH_MSB                                                 13
#define RX_MSDU_END_MSDU_LENGTH_MASK                                                0x0000000000003fff


/* Description		STBC

			When set, use STBC transmission rates
*/

#define RX_MSDU_END_STBC_OFFSET                                                     0x0000000000000048
#define RX_MSDU_END_STBC_LSB                                                        14
#define RX_MSDU_END_STBC_MSB                                                        14
#define RX_MSDU_END_STBC_MASK                                                       0x0000000000004000


/* Description		IPSEC_ESP

			Set if IPv4/v6 packet is using IPsec ESP
*/

#define RX_MSDU_END_IPSEC_ESP_OFFSET                                                0x0000000000000048
#define RX_MSDU_END_IPSEC_ESP_LSB                                                   15
#define RX_MSDU_END_IPSEC_ESP_MSB                                                   15
#define RX_MSDU_END_IPSEC_ESP_MASK                                                  0x0000000000008000


/* Description		L3_OFFSET

			Depending upon mode bit, this field either indicates the
			 L3 offset in bytes from the start of the RX_HEADER or the
			 IP offset in bytes from the start of the packet after decapsulation. 
			 The latter is only valid if ipv4_proto or ipv6_proto is
			 set.
*/

#define RX_MSDU_END_L3_OFFSET_OFFSET                                                0x0000000000000048
#define RX_MSDU_END_L3_OFFSET_LSB                                                   16
#define RX_MSDU_END_L3_OFFSET_MSB                                                   22
#define RX_MSDU_END_L3_OFFSET_MASK                                                  0x00000000007f0000


/* Description		IPSEC_AH

			Set if IPv4/v6 packet is using IPsec AH
*/

#define RX_MSDU_END_IPSEC_AH_OFFSET                                                 0x0000000000000048
#define RX_MSDU_END_IPSEC_AH_LSB                                                    23
#define RX_MSDU_END_IPSEC_AH_MSB                                                    23
#define RX_MSDU_END_IPSEC_AH_MASK                                                   0x0000000000800000


/* Description		L4_OFFSET

			Depending upon mode bit, this field either indicates the
			 L4 offset nin bytes from the start of RX_HEADER(only valid
			 if either ipv4_proto or ipv6_proto is set to 1) or indicates
			 the offset in bytes to the start of TCP or UDP header from
			 the start of the IP header after decapsulation(Only valid
			 if tcp_proto or udp_proto is set).  The value 0 indicates
			 that the offset is longer than 127 bytes.
*/

#define RX_MSDU_END_L4_OFFSET_OFFSET                                                0x0000000000000048
#define RX_MSDU_END_L4_OFFSET_LSB                                                   24
#define RX_MSDU_END_L4_OFFSET_MSB                                                   31
#define RX_MSDU_END_L4_OFFSET_MASK                                                  0x00000000ff000000


/* Description		MSDU_NUMBER

			Indicates the MSDU number within a MPDU.  This value is 
			reset to zero at the start of each MPDU.  If the number 
			of MSDU exceeds 255 this number will wrap using modulo 256.
			
*/

#define RX_MSDU_END_MSDU_NUMBER_OFFSET                                              0x0000000000000048
#define RX_MSDU_END_MSDU_NUMBER_LSB                                                 32
#define RX_MSDU_END_MSDU_NUMBER_MSB                                                 39
#define RX_MSDU_END_MSDU_NUMBER_MASK                                                0x000000ff00000000


/* Description		DECAP_FORMAT

			Indicates the format after decapsulation:
			
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			<legal all>
*/

#define RX_MSDU_END_DECAP_FORMAT_OFFSET                                             0x0000000000000048
#define RX_MSDU_END_DECAP_FORMAT_LSB                                                40
#define RX_MSDU_END_DECAP_FORMAT_MSB                                                41
#define RX_MSDU_END_DECAP_FORMAT_MASK                                               0x0000030000000000


/* Description		IPV4_PROTO

			Set if L2 layer indicates IPv4 protocol.
*/

#define RX_MSDU_END_IPV4_PROTO_OFFSET                                               0x0000000000000048
#define RX_MSDU_END_IPV4_PROTO_LSB                                                  42
#define RX_MSDU_END_IPV4_PROTO_MSB                                                  42
#define RX_MSDU_END_IPV4_PROTO_MASK                                                 0x0000040000000000


/* Description		IPV6_PROTO

			Set if L2 layer indicates IPv6 protocol.
*/

#define RX_MSDU_END_IPV6_PROTO_OFFSET                                               0x0000000000000048
#define RX_MSDU_END_IPV6_PROTO_LSB                                                  43
#define RX_MSDU_END_IPV6_PROTO_MSB                                                  43
#define RX_MSDU_END_IPV6_PROTO_MASK                                                 0x0000080000000000


/* Description		TCP_PROTO

			Set if the ipv4_proto or ipv6_proto are set and the IP protocol
			 indicates TCP.
*/

#define RX_MSDU_END_TCP_PROTO_OFFSET                                                0x0000000000000048
#define RX_MSDU_END_TCP_PROTO_LSB                                                   44
#define RX_MSDU_END_TCP_PROTO_MSB                                                   44
#define RX_MSDU_END_TCP_PROTO_MASK                                                  0x0000100000000000


/* Description		UDP_PROTO

			Set if the ipv4_proto or ipv6_proto are set and the IP protocol
			 indicates UDP.
*/

#define RX_MSDU_END_UDP_PROTO_OFFSET                                                0x0000000000000048
#define RX_MSDU_END_UDP_PROTO_LSB                                                   45
#define RX_MSDU_END_UDP_PROTO_MSB                                                   45
#define RX_MSDU_END_UDP_PROTO_MASK                                                  0x0000200000000000


/* Description		IP_FRAG

			Indicates that either the IP More frag bit is set or IP 
			frag number is non-zero.  If set indicates that this is 
			a fragmented IP packet.
*/

#define RX_MSDU_END_IP_FRAG_OFFSET                                                  0x0000000000000048
#define RX_MSDU_END_IP_FRAG_LSB                                                     46
#define RX_MSDU_END_IP_FRAG_MSB                                                     46
#define RX_MSDU_END_IP_FRAG_MASK                                                    0x0000400000000000


/* Description		TCP_ONLY_ACK

			Set if only the TCP Ack bit is set in the TCP flags and 
			if the TCP payload is 0.
*/

#define RX_MSDU_END_TCP_ONLY_ACK_OFFSET                                             0x0000000000000048
#define RX_MSDU_END_TCP_ONLY_ACK_LSB                                                47
#define RX_MSDU_END_TCP_ONLY_ACK_MSB                                                47
#define RX_MSDU_END_TCP_ONLY_ACK_MASK                                               0x0000800000000000


/* Description		DA_IS_BCAST_MCAST

			The destination address is broadcast or multicast.
*/

#define RX_MSDU_END_DA_IS_BCAST_MCAST_OFFSET                                        0x0000000000000048
#define RX_MSDU_END_DA_IS_BCAST_MCAST_LSB                                           48
#define RX_MSDU_END_DA_IS_BCAST_MCAST_MSB                                           48
#define RX_MSDU_END_DA_IS_BCAST_MCAST_MASK                                          0x0001000000000000


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

#define RX_MSDU_END_TOEPLITZ_HASH_SEL_OFFSET                                        0x0000000000000048
#define RX_MSDU_END_TOEPLITZ_HASH_SEL_LSB                                           49
#define RX_MSDU_END_TOEPLITZ_HASH_SEL_MSB                                           50
#define RX_MSDU_END_TOEPLITZ_HASH_SEL_MASK                                          0x0006000000000000


/* Description		IP_FIXED_HEADER_VALID

			Fixed 20-byte IPv4 header or 40-byte IPv6 header parsed 
			fully within first 256 bytes of the packet
*/

#define RX_MSDU_END_IP_FIXED_HEADER_VALID_OFFSET                                    0x0000000000000048
#define RX_MSDU_END_IP_FIXED_HEADER_VALID_LSB                                       51
#define RX_MSDU_END_IP_FIXED_HEADER_VALID_MSB                                       51
#define RX_MSDU_END_IP_FIXED_HEADER_VALID_MASK                                      0x0008000000000000


/* Description		IP_EXTN_HEADER_VALID

			IPv6/IPv6 header, including IPv4 options and recognizable
			 extension headers parsed fully within first 256 bytes of
			 the packet
*/

#define RX_MSDU_END_IP_EXTN_HEADER_VALID_OFFSET                                     0x0000000000000048
#define RX_MSDU_END_IP_EXTN_HEADER_VALID_LSB                                        52
#define RX_MSDU_END_IP_EXTN_HEADER_VALID_MSB                                        52
#define RX_MSDU_END_IP_EXTN_HEADER_VALID_MASK                                       0x0010000000000000


/* Description		TCP_UDP_HEADER_VALID

			Fixed 20-byte TCP (excluding TCP options) or 8-byte UDP 
			header parsed fully within first 256 bytes of the packet
			
*/

#define RX_MSDU_END_TCP_UDP_HEADER_VALID_OFFSET                                     0x0000000000000048
#define RX_MSDU_END_TCP_UDP_HEADER_VALID_LSB                                        53
#define RX_MSDU_END_TCP_UDP_HEADER_VALID_MSB                                        53
#define RX_MSDU_END_TCP_UDP_HEADER_VALID_MASK                                       0x0020000000000000


/* Description		MESH_CONTROL_PRESENT

			When set, this MSDU includes the 'Mesh Control' field
			<legal all>
*/

#define RX_MSDU_END_MESH_CONTROL_PRESENT_OFFSET                                     0x0000000000000048
#define RX_MSDU_END_MESH_CONTROL_PRESENT_LSB                                        54
#define RX_MSDU_END_MESH_CONTROL_PRESENT_MSB                                        54
#define RX_MSDU_END_MESH_CONTROL_PRESENT_MASK                                       0x0040000000000000


/* Description		LDPC

			When set, indicates that LDPC coding was used.
			<legal all>
*/

#define RX_MSDU_END_LDPC_OFFSET                                                     0x0000000000000048
#define RX_MSDU_END_LDPC_LSB                                                        55
#define RX_MSDU_END_LDPC_MSB                                                        55
#define RX_MSDU_END_LDPC_MASK                                                       0x0080000000000000


/* Description		IP4_PROTOCOL_IP6_NEXT_HEADER

			For IPv4 this is the 8 bit protocol field (when ipv4_proto
			 is set).  For IPv6 this is the 8 bit next_header field (when
			 ipv6_proto is set).
*/

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


/* Description		USER_RSSI

			RSSI for this user
			<legal all>
*/

#define RX_MSDU_END_USER_RSSI_OFFSET                                                0x0000000000000058
#define RX_MSDU_END_USER_RSSI_LSB                                                   0
#define RX_MSDU_END_USER_RSSI_MSB                                                   7
#define RX_MSDU_END_USER_RSSI_MASK                                                  0x00000000000000ff


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

#define RX_MSDU_END_PKT_TYPE_OFFSET                                                 0x0000000000000058
#define RX_MSDU_END_PKT_TYPE_LSB                                                    8
#define RX_MSDU_END_PKT_TYPE_MSB                                                    11
#define RX_MSDU_END_PKT_TYPE_MASK                                                   0x0000000000000f00


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

#define RX_MSDU_END_SGI_OFFSET                                                      0x0000000000000058
#define RX_MSDU_END_SGI_LSB                                                         12
#define RX_MSDU_END_SGI_MSB                                                         13
#define RX_MSDU_END_SGI_MASK                                                        0x0000000000003000


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define RX_MSDU_END_RATE_MCS_OFFSET                                                 0x0000000000000058
#define RX_MSDU_END_RATE_MCS_LSB                                                    14
#define RX_MSDU_END_RATE_MCS_MSB                                                    17
#define RX_MSDU_END_RATE_MCS_MASK                                                   0x000000000003c000


/* Description		RECEIVE_BANDWIDTH

			Full receive Bandwidth
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define RX_MSDU_END_RECEIVE_BANDWIDTH_OFFSET                                        0x0000000000000058
#define RX_MSDU_END_RECEIVE_BANDWIDTH_LSB                                           18
#define RX_MSDU_END_RECEIVE_BANDWIDTH_MSB                                           20
#define RX_MSDU_END_RECEIVE_BANDWIDTH_MASK                                          0x00000000001c0000


#define RX_MSDU_END_RECEPTION_TYPE_OFFSET                                           0x0000000000000058
#define RX_MSDU_END_RECEPTION_TYPE_LSB                                              21
#define RX_MSDU_END_RECEPTION_TYPE_MSB                                              23
#define RX_MSDU_END_RECEPTION_TYPE_MASK                                             0x0000000000e00000


/* Description		MIMO_SS_BITMAP

			Field only valid when Reception_type for the MPDU from this
			 STA is some form of MIMO reception
			
			Bitmap, with each bit indicating if the related spatial 
			stream is used for this STA
			LSB related to SS 0
			
			0: spatial stream not used for this reception
			1: spatial stream used for this reception
			
			Note: Only 7 bits are reported here to accommodate field
			 'msdu_done_copy.'
			<legal all>
*/

#define RX_MSDU_END_MIMO_SS_BITMAP_OFFSET                                           0x0000000000000058
#define RX_MSDU_END_MIMO_SS_BITMAP_LSB                                              24
#define RX_MSDU_END_MIMO_SS_BITMAP_MSB                                              30
#define RX_MSDU_END_MIMO_SS_BITMAP_MASK                                             0x000000007f000000


/* Description		MSDU_DONE_COPY

			If set indicates that the RX packet data, RX header data, 
			RX PPDU start descriptor, RX MPDU start/end descriptor, 
			RX MSDU start/end descriptors and RX Attention descriptor
			are all valid.
			
			<legal 1>
*/

#define RX_MSDU_END_MSDU_DONE_COPY_OFFSET                                           0x0000000000000058
#define RX_MSDU_END_MSDU_DONE_COPY_LSB                                              31
#define RX_MSDU_END_MSDU_DONE_COPY_MSB                                              31
#define RX_MSDU_END_MSDU_DONE_COPY_MASK                                             0x0000000080000000


#define RX_MSDU_END_FLOW_ID_TOEPLITZ_OFFSET                                         0x0000000000000058
#define RX_MSDU_END_FLOW_ID_TOEPLITZ_LSB                                            32
#define RX_MSDU_END_FLOW_ID_TOEPLITZ_MSB                                            63
#define RX_MSDU_END_FLOW_ID_TOEPLITZ_MASK                                           0xffffffff00000000


/* Description		PPDU_START_TIMESTAMP_63_32

			Timestamp that indicates when the PPDU that contained this
			 MPDU started on the medium, upper 32 bits
			<legal all>
*/

#define RX_MSDU_END_PPDU_START_TIMESTAMP_63_32_OFFSET                               0x0000000000000060
#define RX_MSDU_END_PPDU_START_TIMESTAMP_63_32_LSB                                  0
#define RX_MSDU_END_PPDU_START_TIMESTAMP_63_32_MSB                                  31
#define RX_MSDU_END_PPDU_START_TIMESTAMP_63_32_MASK                                 0x00000000ffffffff


/* Description		SW_PHY_META_DATA

			SW programmed Meta data provided by the PHY.
			
			Can be used for SW to indicate the channel the device is
			 on.
			<legal all>
*/

#define RX_MSDU_END_SW_PHY_META_DATA_OFFSET                                         0x0000000000000060
#define RX_MSDU_END_SW_PHY_META_DATA_LSB                                            32
#define RX_MSDU_END_SW_PHY_META_DATA_MSB                                            63
#define RX_MSDU_END_SW_PHY_META_DATA_MASK                                           0xffffffff00000000


#define RX_MSDU_END_PPDU_START_TIMESTAMP_31_0_OFFSET                                0x0000000000000068
#define RX_MSDU_END_PPDU_START_TIMESTAMP_31_0_LSB                                   0
#define RX_MSDU_END_PPDU_START_TIMESTAMP_31_0_MSB                                   31
#define RX_MSDU_END_PPDU_START_TIMESTAMP_31_0_MASK                                  0x00000000ffffffff


/* Description		TOEPLITZ_HASH_2_OR_4

			Controlled by multiple RxOLE registers for TCP/UDP over 
			IPv4/IPv6 - Either, Toeplitz hash computed over 2-tuple 
			IPv4 or IPv6 src/dest addresses is reported; or, Toeplitz
			 hash computed over 4-tuple IPv4 or IPv6 src/dest addresses
			 and src/dest ports is reported. The Flow_id_toeplitz hash
			 can also be reported here. Usually the hash reported here
			 is the one used for hash-based REO routing (see use_flow_id_toeplitz_clfy
			 in 'RXPT_CLASSIFY_INFO').
			
			Optionally the 3-tuple Toeplitz hash over IPv4
			or IPv6 src/dest addresses and L4 protocol can be reported
*/

#define RX_MSDU_END_TOEPLITZ_HASH_2_OR_4_OFFSET                                     0x0000000000000068
#define RX_MSDU_END_TOEPLITZ_HASH_2_OR_4_LSB                                        32
#define RX_MSDU_END_TOEPLITZ_HASH_2_OR_4_MSB                                        63
#define RX_MSDU_END_TOEPLITZ_HASH_2_OR_4_MASK                                       0xffffffff00000000


/* Description		RESERVED_28A

			<legal 0>
*/

#define RX_MSDU_END_RESERVED_28A_OFFSET                                             0x0000000000000070
#define RX_MSDU_END_RESERVED_28A_LSB                                                0
#define RX_MSDU_END_RESERVED_28A_MSB                                                15
#define RX_MSDU_END_RESERVED_28A_MASK                                               0x000000000000ffff


/* Description		SA_15_0

			Source MAC address bits [15:0] (with the fifth byte in the
			 MSB and the last byte in the LSB, i.e. requiring a byte-swap
			 for little-endian FW)
*/

#define RX_MSDU_END_SA_15_0_OFFSET                                                  0x0000000000000070
#define RX_MSDU_END_SA_15_0_LSB                                                     16
#define RX_MSDU_END_SA_15_0_MSB                                                     31
#define RX_MSDU_END_SA_15_0_MASK                                                    0x00000000ffff0000


/* Description		SA_47_16

			Source MAC address bits [47:16] (with the first byte in 
			the MSB and the fourth byte in the LSB, i.e. requiring a
			 byte-swap for little-endian FW)
*/

#define RX_MSDU_END_SA_47_16_OFFSET                                                 0x0000000000000070
#define RX_MSDU_END_SA_47_16_LSB                                                    32
#define RX_MSDU_END_SA_47_16_MSB                                                    63
#define RX_MSDU_END_SA_47_16_MASK                                                   0xffffffff00000000


#define RX_MSDU_END_FIRST_MPDU_OFFSET                                               0x0000000000000078
#define RX_MSDU_END_FIRST_MPDU_LSB                                                  0
#define RX_MSDU_END_FIRST_MPDU_MSB                                                  0
#define RX_MSDU_END_FIRST_MPDU_MASK                                                 0x0000000000000001


/* Description		RESERVED_30A

			<legal 0>
*/

#define RX_MSDU_END_RESERVED_30A_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_RESERVED_30A_LSB                                                1
#define RX_MSDU_END_RESERVED_30A_MSB                                                1
#define RX_MSDU_END_RESERVED_30A_MASK                                               0x0000000000000002


/* Description		MCAST_BCAST

			Multicast / broadcast indicator.  Only set when the MAC 
			address 1 bit 0 is set indicating mcast/bcast and the BSSID
			 matches one of the 4 BSSID registers. Only set when first_msdu
			 is set.
*/

#define RX_MSDU_END_MCAST_BCAST_OFFSET                                              0x0000000000000078
#define RX_MSDU_END_MCAST_BCAST_LSB                                                 2
#define RX_MSDU_END_MCAST_BCAST_MSB                                                 2
#define RX_MSDU_END_MCAST_BCAST_MASK                                                0x0000000000000004


/* Description		AST_INDEX_NOT_FOUND

			Only valid when first_msdu is set.
			
			Indicates no AST matching entries within the the max search
			 count.  
*/

#define RX_MSDU_END_AST_INDEX_NOT_FOUND_OFFSET                                      0x0000000000000078
#define RX_MSDU_END_AST_INDEX_NOT_FOUND_LSB                                         3
#define RX_MSDU_END_AST_INDEX_NOT_FOUND_MSB                                         3
#define RX_MSDU_END_AST_INDEX_NOT_FOUND_MASK                                        0x0000000000000008


/* Description		AST_INDEX_TIMEOUT

			Only valid when first_msdu is set.
			
			Indicates an unsuccessful search in the address seach table
			 due to timeout.  
*/

#define RX_MSDU_END_AST_INDEX_TIMEOUT_OFFSET                                        0x0000000000000078
#define RX_MSDU_END_AST_INDEX_TIMEOUT_LSB                                           4
#define RX_MSDU_END_AST_INDEX_TIMEOUT_MSB                                           4
#define RX_MSDU_END_AST_INDEX_TIMEOUT_MASK                                          0x0000000000000010


/* Description		POWER_MGMT

			Power management bit set in the 802.11 header.  Only set
			 when first_msdu is set.
*/

#define RX_MSDU_END_POWER_MGMT_OFFSET                                               0x0000000000000078
#define RX_MSDU_END_POWER_MGMT_LSB                                                  5
#define RX_MSDU_END_POWER_MGMT_MSB                                                  5
#define RX_MSDU_END_POWER_MGMT_MASK                                                 0x0000000000000020


/* Description		NON_QOS

			Set if packet is not a non-QoS data frame.  Only set when
			 first_msdu is set.
*/

#define RX_MSDU_END_NON_QOS_OFFSET                                                  0x0000000000000078
#define RX_MSDU_END_NON_QOS_LSB                                                     6
#define RX_MSDU_END_NON_QOS_MSB                                                     6
#define RX_MSDU_END_NON_QOS_MASK                                                    0x0000000000000040


/* Description		NULL_DATA

			Set if frame type indicates either null data or QoS null
			 data format.  Only set when first_msdu is set.
*/

#define RX_MSDU_END_NULL_DATA_OFFSET                                                0x0000000000000078
#define RX_MSDU_END_NULL_DATA_LSB                                                   7
#define RX_MSDU_END_NULL_DATA_MSB                                                   7
#define RX_MSDU_END_NULL_DATA_MASK                                                  0x0000000000000080


/* Description		MGMT_TYPE

			Set if packet is a management packet.  Only set when first_msdu
			 is set.
*/

#define RX_MSDU_END_MGMT_TYPE_OFFSET                                                0x0000000000000078
#define RX_MSDU_END_MGMT_TYPE_LSB                                                   8
#define RX_MSDU_END_MGMT_TYPE_MSB                                                   8
#define RX_MSDU_END_MGMT_TYPE_MASK                                                  0x0000000000000100


/* Description		CTRL_TYPE

			Set if packet is a control packet.  Only set when first_msdu
			 is set.
*/

#define RX_MSDU_END_CTRL_TYPE_OFFSET                                                0x0000000000000078
#define RX_MSDU_END_CTRL_TYPE_LSB                                                   9
#define RX_MSDU_END_CTRL_TYPE_MSB                                                   9
#define RX_MSDU_END_CTRL_TYPE_MASK                                                  0x0000000000000200


/* Description		MORE_DATA

			Set if more bit in frame control is set.  Only set when 
			first_msdu is set.
*/

#define RX_MSDU_END_MORE_DATA_OFFSET                                                0x0000000000000078
#define RX_MSDU_END_MORE_DATA_LSB                                                   10
#define RX_MSDU_END_MORE_DATA_MSB                                                   10
#define RX_MSDU_END_MORE_DATA_MASK                                                  0x0000000000000400


/* Description		EOSP

			Set if the EOSP (end of service period) bit in the QoS control
			 field is set.  Only set when first_msdu is set.
*/

#define RX_MSDU_END_EOSP_OFFSET                                                     0x0000000000000078
#define RX_MSDU_END_EOSP_LSB                                                        11
#define RX_MSDU_END_EOSP_MSB                                                        11
#define RX_MSDU_END_EOSP_MASK                                                       0x0000000000000800


/* Description		A_MSDU_ERROR

			Set if number of MSDUs in A-MSDU is above a threshold or
			 if the size of the MSDU is invalid.  This receive buffer
			 will contain all of the remainder of the MSDUs in this 
			MPDU without decapsulation.
*/

#define RX_MSDU_END_A_MSDU_ERROR_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_A_MSDU_ERROR_LSB                                                12
#define RX_MSDU_END_A_MSDU_ERROR_MSB                                                12
#define RX_MSDU_END_A_MSDU_ERROR_MASK                                               0x0000000000001000


#define RX_MSDU_END_RESERVED_30B_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_RESERVED_30B_LSB                                                13
#define RX_MSDU_END_RESERVED_30B_MSB                                                13
#define RX_MSDU_END_RESERVED_30B_MASK                                               0x0000000000002000


/* Description		ORDER

			Set if the order bit in the frame control is set.  Only 
			set when first_msdu is set.
*/

#define RX_MSDU_END_ORDER_OFFSET                                                    0x0000000000000078
#define RX_MSDU_END_ORDER_LSB                                                       14
#define RX_MSDU_END_ORDER_MSB                                                       14
#define RX_MSDU_END_ORDER_MASK                                                      0x0000000000004000


/* Description		WIFI_PARSER_ERROR

			Indicates that the WiFi frame has one of the following errors
			
			o has less than minimum allowed bytes as per standard
			o has incomplete VLAN LLC/SNAP (only for non A-MSDUs)
			<legal all>
*/

#define RX_MSDU_END_WIFI_PARSER_ERROR_OFFSET                                        0x0000000000000078
#define RX_MSDU_END_WIFI_PARSER_ERROR_LSB                                           15
#define RX_MSDU_END_WIFI_PARSER_ERROR_MSB                                           15
#define RX_MSDU_END_WIFI_PARSER_ERROR_MASK                                          0x0000000000008000


/* Description		OVERFLOW_ERR

			RXPCU Receive FIFO ran out of space to receive the full 
			MPDU. Therefor this MPDU is terminated early and is thus
			 corrupted.  
			
			This MPDU will not be ACKed.
			RXPCU might still be able to correctly receive the following
			 MPDUs in the PPDU if enough fifo space became available
			 in time
*/

#define RX_MSDU_END_OVERFLOW_ERR_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_OVERFLOW_ERR_LSB                                                16
#define RX_MSDU_END_OVERFLOW_ERR_MSB                                                16
#define RX_MSDU_END_OVERFLOW_ERR_MASK                                               0x0000000000010000


/* Description		MSDU_LENGTH_ERR

			Indicates that the MSDU length from the 802.3 encapsulated
			 length field extends beyond the MPDU boundary or if the
			 length is less than 14 bytes.
			Merged with original "other_msdu_err": Indicates that the
			 MSDU threshold was exceeded and thus all the rest of the
			 MSDUs will not be scattered and will not be decasulated
			 but will be DMA'ed in RAW format as a single MSDU buffer
			
*/

#define RX_MSDU_END_MSDU_LENGTH_ERR_OFFSET                                          0x0000000000000078
#define RX_MSDU_END_MSDU_LENGTH_ERR_LSB                                             17
#define RX_MSDU_END_MSDU_LENGTH_ERR_MSB                                             17
#define RX_MSDU_END_MSDU_LENGTH_ERR_MASK                                            0x0000000000020000


/* Description		TCP_UDP_CHKSUM_FAIL

			Indicates that the computed checksum (tcp_udp_chksum in 'RX_MSDU_END') 
			did not match the checksum in the TCP/UDP header.
*/

#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_OFFSET                                      0x0000000000000078
#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_LSB                                         18
#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_MSB                                         18
#define RX_MSDU_END_TCP_UDP_CHKSUM_FAIL_MASK                                        0x0000000000040000


/* Description		IP_CHKSUM_FAIL

			Indicates that the computed checksum (ip_hdr_chksum in 'RX_MSDU_END') 
			did not match the checksum in the IP header.
*/

#define RX_MSDU_END_IP_CHKSUM_FAIL_OFFSET                                           0x0000000000000078
#define RX_MSDU_END_IP_CHKSUM_FAIL_LSB                                              19
#define RX_MSDU_END_IP_CHKSUM_FAIL_MSB                                              19
#define RX_MSDU_END_IP_CHKSUM_FAIL_MASK                                             0x0000000000080000


/* Description		SA_IDX_INVALID

			Indicates no matching entry was found in the address search
			 table for the source MAC address.
*/

#define RX_MSDU_END_SA_IDX_INVALID_OFFSET                                           0x0000000000000078
#define RX_MSDU_END_SA_IDX_INVALID_LSB                                              20
#define RX_MSDU_END_SA_IDX_INVALID_MSB                                              20
#define RX_MSDU_END_SA_IDX_INVALID_MASK                                             0x0000000000100000


/* Description		DA_IDX_INVALID

			Indicates no matching entry was found in the address search
			 table for the destination MAC address.
*/

#define RX_MSDU_END_DA_IDX_INVALID_OFFSET                                           0x0000000000000078
#define RX_MSDU_END_DA_IDX_INVALID_LSB                                              21
#define RX_MSDU_END_DA_IDX_INVALID_MSB                                              21
#define RX_MSDU_END_DA_IDX_INVALID_MASK                                             0x0000000000200000


/* Description		AMSDU_ADDR_MISMATCH

			Indicates that an A-MSDU with 'from DS = 0' had an SA mismatching
			 TA or an A-MDU with 'to DS = 0' had a DA mismatching RA
			
*/

#define RX_MSDU_END_AMSDU_ADDR_MISMATCH_OFFSET                                      0x0000000000000078
#define RX_MSDU_END_AMSDU_ADDR_MISMATCH_LSB                                         22
#define RX_MSDU_END_AMSDU_ADDR_MISMATCH_MSB                                         22
#define RX_MSDU_END_AMSDU_ADDR_MISMATCH_MASK                                        0x0000000000400000


/* Description		RX_IN_TX_DECRYPT_BYP

			Indicates that RX packet is not decrypted as Crypto is busy
			 with TX packet processing.
*/

#define RX_MSDU_END_RX_IN_TX_DECRYPT_BYP_OFFSET                                     0x0000000000000078
#define RX_MSDU_END_RX_IN_TX_DECRYPT_BYP_LSB                                        23
#define RX_MSDU_END_RX_IN_TX_DECRYPT_BYP_MSB                                        23
#define RX_MSDU_END_RX_IN_TX_DECRYPT_BYP_MASK                                       0x0000000000800000


/* Description		ENCRYPT_REQUIRED

			Indicates that this data type frame is not encrypted even
			 if the policy for this MPDU requires encryption as indicated
			 in the peer entry key type.
*/

#define RX_MSDU_END_ENCRYPT_REQUIRED_OFFSET                                         0x0000000000000078
#define RX_MSDU_END_ENCRYPT_REQUIRED_LSB                                            24
#define RX_MSDU_END_ENCRYPT_REQUIRED_MSB                                            24
#define RX_MSDU_END_ENCRYPT_REQUIRED_MASK                                           0x0000000001000000


/* Description		DIRECTED

			MPDU is a directed packet which means that the RA matched
			 our STA addresses.  In proxySTA it means that the TA matched
			 an entry in our address search table with the corresponding
			 "no_ack" bit is the address search entry cleared.
*/

#define RX_MSDU_END_DIRECTED_OFFSET                                                 0x0000000000000078
#define RX_MSDU_END_DIRECTED_LSB                                                    25
#define RX_MSDU_END_DIRECTED_MSB                                                    25
#define RX_MSDU_END_DIRECTED_MASK                                                   0x0000000002000000


/* Description		BUFFER_FRAGMENT

			Indicates that at least one of the rx buffers has been fragmented. 
			 If set the FW should look at the rx_frag_info descriptor
			 described below.
*/

#define RX_MSDU_END_BUFFER_FRAGMENT_OFFSET                                          0x0000000000000078
#define RX_MSDU_END_BUFFER_FRAGMENT_LSB                                             26
#define RX_MSDU_END_BUFFER_FRAGMENT_MSB                                             26
#define RX_MSDU_END_BUFFER_FRAGMENT_MASK                                            0x0000000004000000


/* Description		MPDU_LENGTH_ERR

			Indicates that the MPDU was pre-maturely terminated resulting
			 in a truncated MPDU.  Don't trust the MPDU length field.
			
*/

#define RX_MSDU_END_MPDU_LENGTH_ERR_OFFSET                                          0x0000000000000078
#define RX_MSDU_END_MPDU_LENGTH_ERR_LSB                                             27
#define RX_MSDU_END_MPDU_LENGTH_ERR_MSB                                             27
#define RX_MSDU_END_MPDU_LENGTH_ERR_MASK                                            0x0000000008000000


/* Description		TKIP_MIC_ERR

			Indicates that the MPDU Michael integrity check failed
*/

#define RX_MSDU_END_TKIP_MIC_ERR_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_TKIP_MIC_ERR_LSB                                                28
#define RX_MSDU_END_TKIP_MIC_ERR_MSB                                                28
#define RX_MSDU_END_TKIP_MIC_ERR_MASK                                               0x0000000010000000


/* Description		DECRYPT_ERR

			Indicates that the MPDU decrypt integrity check failed or
			 CRYPTO received an encrypted frame, but did not get a valid
			 corresponding key id in the peer entry.
*/

#define RX_MSDU_END_DECRYPT_ERR_OFFSET                                              0x0000000000000078
#define RX_MSDU_END_DECRYPT_ERR_LSB                                                 29
#define RX_MSDU_END_DECRYPT_ERR_MSB                                                 29
#define RX_MSDU_END_DECRYPT_ERR_MASK                                                0x0000000020000000


/* Description		UNENCRYPTED_FRAME_ERR

			Copied here by RX OLE from the RX_MPDU_END TLV
*/

#define RX_MSDU_END_UNENCRYPTED_FRAME_ERR_OFFSET                                    0x0000000000000078
#define RX_MSDU_END_UNENCRYPTED_FRAME_ERR_LSB                                       30
#define RX_MSDU_END_UNENCRYPTED_FRAME_ERR_MSB                                       30
#define RX_MSDU_END_UNENCRYPTED_FRAME_ERR_MASK                                      0x0000000040000000


/* Description		FCS_ERR

			Indicates that the MPDU FCS check failed
*/

#define RX_MSDU_END_FCS_ERR_OFFSET                                                  0x0000000000000078
#define RX_MSDU_END_FCS_ERR_LSB                                                     31
#define RX_MSDU_END_FCS_ERR_MSB                                                     31
#define RX_MSDU_END_FCS_ERR_MASK                                                    0x0000000080000000


/* Description		RESERVED_31A

			<legal 0>
*/

#define RX_MSDU_END_RESERVED_31A_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_RESERVED_31A_LSB                                                32
#define RX_MSDU_END_RESERVED_31A_MSB                                                41
#define RX_MSDU_END_RESERVED_31A_MASK                                               0x000003ff00000000


/* Description		DECRYPT_STATUS_CODE

			Field provides insight into the decryption performed
			
			<enum 0 decrypt_ok> Frame had protection enabled and decrypted
			 properly 
			<enum 1 decrypt_unprotected_frame > Frame is unprotected
			 and hence bypassed 
			<enum 2 decrypt_data_err > Frame has protection enabled 
			and could not be properly d   ecrypted due to MIC/ICV mismatch
			 etc. 
			<enum 3 decrypt_key_invalid > Frame has protection enabled
			 but the key that was required to decrypt this frame was
			 not valid 
			<enum 4 decrypt_peer_entry_invalid > Frame has protection
			 enabled but the key that was required to decrypt this frame
			 was not valid
			<enum 5 decrypt_other > Reserved for other indications
			
			<legal 0 - 5>
*/

#define RX_MSDU_END_DECRYPT_STATUS_CODE_OFFSET                                      0x0000000000000078
#define RX_MSDU_END_DECRYPT_STATUS_CODE_LSB                                         42
#define RX_MSDU_END_DECRYPT_STATUS_CODE_MSB                                         44
#define RX_MSDU_END_DECRYPT_STATUS_CODE_MASK                                        0x00001c0000000000


/* Description		RX_BITMAP_NOT_UPDATED

			Frame is received, but RXPCU could not update the receive
			 bitmap due to (temporary) fifo contraints.
			<legal all>
*/

#define RX_MSDU_END_RX_BITMAP_NOT_UPDATED_OFFSET                                    0x0000000000000078
#define RX_MSDU_END_RX_BITMAP_NOT_UPDATED_LSB                                       45
#define RX_MSDU_END_RX_BITMAP_NOT_UPDATED_MSB                                       45
#define RX_MSDU_END_RX_BITMAP_NOT_UPDATED_MASK                                      0x0000200000000000


/* Description		RESERVED_31B

			<legal 0>
*/

#define RX_MSDU_END_RESERVED_31B_OFFSET                                             0x0000000000000078
#define RX_MSDU_END_RESERVED_31B_LSB                                                46
#define RX_MSDU_END_RESERVED_31B_MSB                                                62
#define RX_MSDU_END_RESERVED_31B_MASK                                               0x7fffc00000000000


#define RX_MSDU_END_MSDU_DONE_OFFSET                                                0x0000000000000078
#define RX_MSDU_END_MSDU_DONE_LSB                                                   63
#define RX_MSDU_END_MSDU_DONE_MSB                                                   63
#define RX_MSDU_END_MSDU_DONE_MASK                                                  0x8000000000000000



#endif   // RX_MSDU_END
