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

#ifndef _RX_MPDU_INFO_H_
#define _RX_MPDU_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#include "rxpt_classify_info.h"
#define NUM_OF_DWORDS_RX_MPDU_INFO 30


struct rx_mpdu_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   rxpt_classify_info                                        rxpt_classify_info_details;
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32; // [31:0]
             uint32_t rx_reo_queue_desc_addr_39_32                            :  8, // [7:0]
                      receive_queue_number                                    : 16, // [23:8]
                      pre_delim_err_warning                                   :  1, // [24:24]
                      first_delim_err                                         :  1, // [25:25]
                      reserved_2a                                             :  6; // [31:26]
             uint32_t pn_31_0                                                 : 32; // [31:0]
             uint32_t pn_63_32                                                : 32; // [31:0]
             uint32_t pn_95_64                                                : 32; // [31:0]
             uint32_t pn_127_96                                               : 32; // [31:0]
             uint32_t epd_en                                                  :  1, // [0:0]
                      all_frames_shall_be_encrypted                           :  1, // [1:1]
                      encrypt_type                                            :  4, // [5:2]
                      wep_key_width_for_variable_key                          :  2, // [7:6]
                      mesh_sta                                                :  2, // [9:8]
                      bssid_hit                                               :  1, // [10:10]
                      bssid_number                                            :  4, // [14:11]
                      tid                                                     :  4, // [18:15]
                      reserved_7a                                             : 13; // [31:19]
             uint32_t peer_meta_data                                          : 32; // [31:0]
             uint32_t rxpcu_mpdu_filter_in_category                           :  2, // [1:0]
                      sw_frame_group_id                                       :  7, // [8:2]
                      ndp_frame                                               :  1, // [9:9]
                      phy_err                                                 :  1, // [10:10]
                      phy_err_during_mpdu_header                              :  1, // [11:11]
                      protocol_version_err                                    :  1, // [12:12]
                      ast_based_lookup_valid                                  :  1, // [13:13]
                      ranging                                                 :  1, // [14:14]
                      reserved_9a                                             :  1, // [15:15]
                      phy_ppdu_id                                             : 16; // [31:16]
             uint32_t ast_index                                               : 16, // [15:0]
                      sw_peer_id                                              : 16; // [31:16]
             uint32_t mpdu_frame_control_valid                                :  1, // [0:0]
                      mpdu_duration_valid                                     :  1, // [1:1]
                      mac_addr_ad1_valid                                      :  1, // [2:2]
                      mac_addr_ad2_valid                                      :  1, // [3:3]
                      mac_addr_ad3_valid                                      :  1, // [4:4]
                      mac_addr_ad4_valid                                      :  1, // [5:5]
                      mpdu_sequence_control_valid                             :  1, // [6:6]
                      mpdu_qos_control_valid                                  :  1, // [7:7]
                      mpdu_ht_control_valid                                   :  1, // [8:8]
                      frame_encryption_info_valid                             :  1, // [9:9]
                      mpdu_fragment_number                                    :  4, // [13:10]
                      more_fragment_flag                                      :  1, // [14:14]
                      reserved_11a                                            :  1, // [15:15]
                      fr_ds                                                   :  1, // [16:16]
                      to_ds                                                   :  1, // [17:17]
                      encrypted                                               :  1, // [18:18]
                      mpdu_retry                                              :  1, // [19:19]
                      mpdu_sequence_number                                    : 12; // [31:20]
             uint32_t key_id_octet                                            :  8, // [7:0]
                      new_peer_entry                                          :  1, // [8:8]
                      decrypt_needed                                          :  1, // [9:9]
                      decap_type                                              :  2, // [11:10]
                      rx_insert_vlan_c_tag_padding                            :  1, // [12:12]
                      rx_insert_vlan_s_tag_padding                            :  1, // [13:13]
                      strip_vlan_c_tag_decap                                  :  1, // [14:14]
                      strip_vlan_s_tag_decap                                  :  1, // [15:15]
                      pre_delim_count                                         : 12, // [27:16]
                      ampdu_flag                                              :  1, // [28:28]
                      bar_frame                                               :  1, // [29:29]
                      raw_mpdu                                                :  1, // [30:30]
                      reserved_12                                             :  1; // [31:31]
             uint32_t mpdu_length                                             : 14, // [13:0]
                      first_mpdu                                              :  1, // [14:14]
                      mcast_bcast                                             :  1, // [15:15]
                      ast_index_not_found                                     :  1, // [16:16]
                      ast_index_timeout                                       :  1, // [17:17]
                      power_mgmt                                              :  1, // [18:18]
                      non_qos                                                 :  1, // [19:19]
                      null_data                                               :  1, // [20:20]
                      mgmt_type                                               :  1, // [21:21]
                      ctrl_type                                               :  1, // [22:22]
                      more_data                                               :  1, // [23:23]
                      eosp                                                    :  1, // [24:24]
                      fragment_flag                                           :  1, // [25:25]
                      order                                                   :  1, // [26:26]
                      u_apsd_trigger                                          :  1, // [27:27]
                      encrypt_required                                        :  1, // [28:28]
                      directed                                                :  1, // [29:29]
                      amsdu_present                                           :  1, // [30:30]
                      reserved_13                                             :  1; // [31:31]
             uint32_t mpdu_frame_control_field                                : 16, // [15:0]
                      mpdu_duration_field                                     : 16; // [31:16]
             uint32_t mac_addr_ad1_31_0                                       : 32; // [31:0]
             uint32_t mac_addr_ad1_47_32                                      : 16, // [15:0]
                      mac_addr_ad2_15_0                                       : 16; // [31:16]
             uint32_t mac_addr_ad2_47_16                                      : 32; // [31:0]
             uint32_t mac_addr_ad3_31_0                                       : 32; // [31:0]
             uint32_t mac_addr_ad3_47_32                                      : 16, // [15:0]
                      mpdu_sequence_control_field                             : 16; // [31:16]
             uint32_t mac_addr_ad4_31_0                                       : 32; // [31:0]
             uint32_t mac_addr_ad4_47_32                                      : 16, // [15:0]
                      mpdu_qos_control_field                                  : 16; // [31:16]
             uint32_t mpdu_ht_control_field                                   : 32; // [31:0]
             uint32_t vdev_id                                                 :  8, // [7:0]
                      service_code                                            :  9, // [16:8]
                      priority_valid                                          :  1, // [17:17]
                      src_info                                                : 12, // [29:18]
                      reserved_23a                                            :  1, // [30:30]
                      multi_link_addr_ad1_ad2_valid                           :  1; // [31:31]
             uint32_t multi_link_addr_ad1_31_0                                : 32; // [31:0]
             uint32_t multi_link_addr_ad1_47_32                               : 16, // [15:0]
                      multi_link_addr_ad2_15_0                                : 16; // [31:16]
             uint32_t multi_link_addr_ad2_47_16                               : 32; // [31:0]
             uint32_t authorized_to_send_wds                                  :  1, // [0:0]
                      reserved_27a                                            : 31; // [31:1]
             uint32_t reserved_28a                                            : 32; // [31:0]
             uint32_t reserved_29a                                            : 32; // [31:0]
#else
             struct   rxpt_classify_info                                        rxpt_classify_info_details;
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32; // [31:0]
             uint32_t reserved_2a                                             :  6, // [31:26]
                      first_delim_err                                         :  1, // [25:25]
                      pre_delim_err_warning                                   :  1, // [24:24]
                      receive_queue_number                                    : 16, // [23:8]
                      rx_reo_queue_desc_addr_39_32                            :  8; // [7:0]
             uint32_t pn_31_0                                                 : 32; // [31:0]
             uint32_t pn_63_32                                                : 32; // [31:0]
             uint32_t pn_95_64                                                : 32; // [31:0]
             uint32_t pn_127_96                                               : 32; // [31:0]
             uint32_t reserved_7a                                             : 13, // [31:19]
                      tid                                                     :  4, // [18:15]
                      bssid_number                                            :  4, // [14:11]
                      bssid_hit                                               :  1, // [10:10]
                      mesh_sta                                                :  2, // [9:8]
                      wep_key_width_for_variable_key                          :  2, // [7:6]
                      encrypt_type                                            :  4, // [5:2]
                      all_frames_shall_be_encrypted                           :  1, // [1:1]
                      epd_en                                                  :  1; // [0:0]
             uint32_t peer_meta_data                                          : 32; // [31:0]
             uint32_t phy_ppdu_id                                             : 16, // [31:16]
                      reserved_9a                                             :  1, // [15:15]
                      ranging                                                 :  1, // [14:14]
                      ast_based_lookup_valid                                  :  1, // [13:13]
                      protocol_version_err                                    :  1, // [12:12]
                      phy_err_during_mpdu_header                              :  1, // [11:11]
                      phy_err                                                 :  1, // [10:10]
                      ndp_frame                                               :  1, // [9:9]
                      sw_frame_group_id                                       :  7, // [8:2]
                      rxpcu_mpdu_filter_in_category                           :  2; // [1:0]
             uint32_t sw_peer_id                                              : 16, // [31:16]
                      ast_index                                               : 16; // [15:0]
             uint32_t mpdu_sequence_number                                    : 12, // [31:20]
                      mpdu_retry                                              :  1, // [19:19]
                      encrypted                                               :  1, // [18:18]
                      to_ds                                                   :  1, // [17:17]
                      fr_ds                                                   :  1, // [16:16]
                      reserved_11a                                            :  1, // [15:15]
                      more_fragment_flag                                      :  1, // [14:14]
                      mpdu_fragment_number                                    :  4, // [13:10]
                      frame_encryption_info_valid                             :  1, // [9:9]
                      mpdu_ht_control_valid                                   :  1, // [8:8]
                      mpdu_qos_control_valid                                  :  1, // [7:7]
                      mpdu_sequence_control_valid                             :  1, // [6:6]
                      mac_addr_ad4_valid                                      :  1, // [5:5]
                      mac_addr_ad3_valid                                      :  1, // [4:4]
                      mac_addr_ad2_valid                                      :  1, // [3:3]
                      mac_addr_ad1_valid                                      :  1, // [2:2]
                      mpdu_duration_valid                                     :  1, // [1:1]
                      mpdu_frame_control_valid                                :  1; // [0:0]
             uint32_t reserved_12                                             :  1, // [31:31]
                      raw_mpdu                                                :  1, // [30:30]
                      bar_frame                                               :  1, // [29:29]
                      ampdu_flag                                              :  1, // [28:28]
                      pre_delim_count                                         : 12, // [27:16]
                      strip_vlan_s_tag_decap                                  :  1, // [15:15]
                      strip_vlan_c_tag_decap                                  :  1, // [14:14]
                      rx_insert_vlan_s_tag_padding                            :  1, // [13:13]
                      rx_insert_vlan_c_tag_padding                            :  1, // [12:12]
                      decap_type                                              :  2, // [11:10]
                      decrypt_needed                                          :  1, // [9:9]
                      new_peer_entry                                          :  1, // [8:8]
                      key_id_octet                                            :  8; // [7:0]
             uint32_t reserved_13                                             :  1, // [31:31]
                      amsdu_present                                           :  1, // [30:30]
                      directed                                                :  1, // [29:29]
                      encrypt_required                                        :  1, // [28:28]
                      u_apsd_trigger                                          :  1, // [27:27]
                      order                                                   :  1, // [26:26]
                      fragment_flag                                           :  1, // [25:25]
                      eosp                                                    :  1, // [24:24]
                      more_data                                               :  1, // [23:23]
                      ctrl_type                                               :  1, // [22:22]
                      mgmt_type                                               :  1, // [21:21]
                      null_data                                               :  1, // [20:20]
                      non_qos                                                 :  1, // [19:19]
                      power_mgmt                                              :  1, // [18:18]
                      ast_index_timeout                                       :  1, // [17:17]
                      ast_index_not_found                                     :  1, // [16:16]
                      mcast_bcast                                             :  1, // [15:15]
                      first_mpdu                                              :  1, // [14:14]
                      mpdu_length                                             : 14; // [13:0]
             uint32_t mpdu_duration_field                                     : 16, // [31:16]
                      mpdu_frame_control_field                                : 16; // [15:0]
             uint32_t mac_addr_ad1_31_0                                       : 32; // [31:0]
             uint32_t mac_addr_ad2_15_0                                       : 16, // [31:16]
                      mac_addr_ad1_47_32                                      : 16; // [15:0]
             uint32_t mac_addr_ad2_47_16                                      : 32; // [31:0]
             uint32_t mac_addr_ad3_31_0                                       : 32; // [31:0]
             uint32_t mpdu_sequence_control_field                             : 16, // [31:16]
                      mac_addr_ad3_47_32                                      : 16; // [15:0]
             uint32_t mac_addr_ad4_31_0                                       : 32; // [31:0]
             uint32_t mpdu_qos_control_field                                  : 16, // [31:16]
                      mac_addr_ad4_47_32                                      : 16; // [15:0]
             uint32_t mpdu_ht_control_field                                   : 32; // [31:0]
             uint32_t multi_link_addr_ad1_ad2_valid                           :  1, // [31:31]
                      reserved_23a                                            :  1, // [30:30]
                      src_info                                                : 12, // [29:18]
                      priority_valid                                          :  1, // [17:17]
                      service_code                                            :  9, // [16:8]
                      vdev_id                                                 :  8; // [7:0]
             uint32_t multi_link_addr_ad1_31_0                                : 32; // [31:0]
             uint32_t multi_link_addr_ad2_15_0                                : 16, // [31:16]
                      multi_link_addr_ad1_47_32                               : 16; // [15:0]
             uint32_t multi_link_addr_ad2_47_16                               : 32; // [31:0]
             uint32_t reserved_27a                                            : 31, // [31:1]
                      authorized_to_send_wds                                  :  1; // [0:0]
             uint32_t reserved_28a                                            : 32; // [31:0]
             uint32_t reserved_29a                                            : 32; // [31:0]
#endif
};


/* Description		RXPT_CLASSIFY_INFO_DETAILS

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			RXOLE related classification info
			<legal all
*/


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

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_REO_DESTINATION_INDICATION_OFFSET   0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_REO_DESTINATION_INDICATION_LSB      0
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_REO_DESTINATION_INDICATION_MSB      4
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_REO_DESTINATION_INDICATION_MASK     0x0000001f


/* Description		LMAC_PEER_ID_MSB

			If use_flow_id_toeplitz_clfy is set and lmac_peer_id_'sb
			 is 2'b00, Rx OLE uses a REO desination indicati'n of {1'b1, 
			hash[3:0]} using the chosen Toeplitz hash from Common Parser
			 if flow search fails.
			If use_flow_id_toeplitz_clfy is set and lmac_peer_id_msb
			 's not 2'b00, Rx OLE uses a REO desination indication of
			 {lmac_peer_id_msb, hash[2:0]} using the chosen Toeplitz
			 hash from Common Parser if flow search fails.
			<legal all>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_LMAC_PEER_ID_MSB_OFFSET             0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_LMAC_PEER_ID_MSB_LSB                5
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_LMAC_PEER_ID_MSB_MSB                6
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_LMAC_PEER_ID_MSB_MASK               0x00000060


/* Description		USE_FLOW_ID_TOEPLITZ_CLFY

			Indication to Rx OLE to enable REO destination routing based
			 on the chosen Toeplitz hash from Common Parser, in case
			 flow search fails
			<legal all>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_USE_FLOW_ID_TOEPLITZ_CLFY_OFFSET    0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_USE_FLOW_ID_TOEPLITZ_CLFY_LSB       7
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_USE_FLOW_ID_TOEPLITZ_CLFY_MSB       7
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_USE_FLOW_ID_TOEPLITZ_CLFY_MASK      0x00000080


/* Description		PKT_SELECTION_FP_UCAST_DATA

			Filter pass Unicast data frame (matching rxpcu_filter_pass
			 and sw_frame_group_Unicast_data) routing selection
			TODO: What about 'rxpcu_filter_pass_monior_ovrd'?
			
			1'b0: source and destination rings are selected from the
			 RxOLE register settings for the packet type
			
			1'b1: source ring and destination ring is selected from 
			the rxdma0_source_ring_selection and rxdma0_destination_ring_selection
			 fields in this STRUCT
			<legal all>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_UCAST_DATA_OFFSET  0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_UCAST_DATA_LSB     8
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_UCAST_DATA_MSB     8
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_UCAST_DATA_MASK    0x00000100


/* Description		PKT_SELECTION_FP_MCAST_DATA

			Filter pass Multicast data frame (matching rxpcu_filter_pass
			 and sw_frame_group_Multicast_data) routing selection
			TODO: What about 'rxpcu_filter_pass_monior_ovrd'?
			
			1'b0: source and destination rings are selected from the
			 RxOLE register settings for the packet type
			
			1'b1: source ring and destination ring is selected from 
			the rxdma0_source_ring_selection and rxdma0_destination_ring_selection
			 fields in this STRUCT
			<legal all>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_MCAST_DATA_OFFSET  0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_MCAST_DATA_LSB     9
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_MCAST_DATA_MSB     9
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_MCAST_DATA_MASK    0x00000200


/* Description		PKT_SELECTION_FP_1000

			Filter pass BAR frame (matching rxpcu_filter_pass and sw_frame_group_ctrl_1000) 
			routing selection
			TODO: What about 'rxpcu_filter_pass_monior_ovrd'?
			
			1'b0: source and destination rings are selected from the
			 RxOLE register settings for the packet type
			
			1'b1: source ring and destination ring is selected from 
			the rxdma0_source_ring_selection and rxdma0_destination_ring_selection
			 fields in this STRUCT
			<legal all>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_1000_OFFSET        0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_1000_LSB           10
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_1000_MSB           10
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PKT_SELECTION_FP_1000_MASK          0x00000400


/* Description		RXDMA0_SOURCE_RING_SELECTION

			Field only valid when for the received frame type the corresponding
			 pkt_selection_fp_... bit is set
			
			<enum 0 sw2rxdma0_0_buf_source_ring> The data buffer for
			 this frame shall be sourced by sw2rxdma0 buffer source 
			ring.
			<enum 1 fw2rxdma0_pmac0_buf_source_ring> The data buffer
			 for this frame shall be sourced by fw2rxdma buffer source
			 ring for PMAC0.
			<enum 2 sw2rxdma0_1_buf_source_ring> The data buffer for
			 this frame shall be sourced by sw2rxdma1 buffer source 
			ring.
			<enum 3 no_buffer_rxdma0_ring> The frame shall not be written
			 to any data buffer.
			<enum 4 sw2rxdma0_exception_buf_source_ring> The data buffer
			 for this frame shall be sourced by sw2rxdma_exception buffer
			 source ring.
			<enum 5 fw2rxdma0_pmac1_buf_source_ring> The data buffer
			 for this frame shall be sourced by fw2rxdma buffer source
			 ring for PMAC1.
			
			<legal 0-5>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_SOURCE_RING_SELECTION_OFFSET 0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_SOURCE_RING_SELECTION_LSB    11
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_SOURCE_RING_SELECTION_MSB    13
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_SOURCE_RING_SELECTION_MASK   0x00003800


/* Description		RXDMA0_DESTINATION_RING_SELECTION

			Field only valid when for the received frame type the corresponding
			 pkt_selection_fp_... bit is set
			
			<enum 0  rxdma_release_ring> RXDMA0 shall push the frame
			 to the Release ring. Effectively this means the frame needs
			 to be dropped.
			<enum 1  rxdma2fw_pmac0_ring> RXDMA0 shall push the frame
			 to the FW ring for PMAC0.
			<enum 2  rxdma2sw_ring> RXDMA0 shall push the frame to the
			 SW ring.
			<enum 3  rxdma2reo_ring> RXDMA0 shall push the frame to 
			the REO entrance ring.
			<enum 4  rxdma2fw_pmac1_ring> RXDMA0 shall push the frame
			 to the FW ring for PMAC1.
			<enum 5 rxdma2reo_remote0_ring> RXDMA0 shall push the frame
			 to the first MLO REO entrance ring.
			<enum 6 rxdma2reo_remote1_ring> RXDMA0 shall push the frame
			 to the second MLO REO entrance ring.
			
			<legal 0-6>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_DESTINATION_RING_SELECTION_OFFSET 0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_DESTINATION_RING_SELECTION_LSB 14
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_DESTINATION_RING_SELECTION_MSB 16
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_RXDMA0_DESTINATION_RING_SELECTION_MASK 0x0001c000


/* Description		MCAST_ECHO_DROP_ENABLE

			If set, for multicast packets, multicast echo check (i.e. 
			SA search with mcast_echo_check = 1) shall be performed 
			by RXOLE, and any multicast echo packets should be indicated
			 to RXDMA for release to WBM
			
			<legal all>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_MCAST_ECHO_DROP_ENABLE_OFFSET       0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_MCAST_ECHO_DROP_ENABLE_LSB          17
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_MCAST_ECHO_DROP_ENABLE_MSB          17
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_MCAST_ECHO_DROP_ENABLE_MASK         0x00020000


/* Description		WDS_LEARNING_DETECT_EN

			If set, WDS learning detection based on SA search and notification
			 to FW (using RXDMA0 status ring) is enabled and the "timestamp" 
			field in address search failure cache-only entry should 
			be used to avoid multiple WDS learning notifications.
			
			<legal all>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_WDS_LEARNING_DETECT_EN_OFFSET       0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_WDS_LEARNING_DETECT_EN_LSB          18
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_WDS_LEARNING_DETECT_EN_MSB          18
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_WDS_LEARNING_DETECT_EN_MASK         0x00040000


/* Description		INTRABSS_CHECK_EN

			If set, intra-BSS routing detection is enabled
			
			<legal all>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_INTRABSS_CHECK_EN_OFFSET            0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_INTRABSS_CHECK_EN_LSB               19
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_INTRABSS_CHECK_EN_MSB               19
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_INTRABSS_CHECK_EN_MASK              0x00080000


/* Description		USE_PPE

			Indicates to RXDMA to ignore the REO_destination_indication
			 and use a programmed value corresponding to the REO2PPE
			 ring
			
			This override to REO2PPE for packets requiring multiple 
			buffers shall be disabled based on an RXDMA configuration, 
			as PPE may not support such packets.
			
			<legal all>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_USE_PPE_OFFSET                      0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_USE_PPE_LSB                         20
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_USE_PPE_MSB                         20
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_USE_PPE_MASK                        0x00100000


/* Description		PPE_ROUTING_ENABLE

			Global enable/disable bit for routing to PPE, used to disable
			 PPE routing even if RXOLE CCE or flow search indicate 'Use_PPE'
			
			
			This is set by SW for peers which are being handled by a
			 host SW/accelerator subsystem that also handles packet 
			buffer management for WiFi-to-PPE routing.
			
			This is cleared by SW for peers which are being handled 
			by a different subsystem, completely disabling WiFi-to-PPE
			 routing for such peers.
			
			<legal all>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PPE_ROUTING_ENABLE_OFFSET           0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PPE_ROUTING_ENABLE_LSB              21
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PPE_ROUTING_ENABLE_MSB              21
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_PPE_ROUTING_ENABLE_MASK             0x00200000


/* Description		RESERVED_0B

			<legal 0>
*/

#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_RESERVED_0B_OFFSET                  0x00000000
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_RESERVED_0B_LSB                     22
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_RESERVED_0B_MSB                     31
#define RX_MPDU_INFO_RXPT_CLASSIFY_INFO_DETAILS_RESERVED_0B_MASK                    0xffc00000


/* Description		RX_REO_QUEUE_DESC_ADDR_31_0

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			Address (lower 32 bits) of the REO queue descriptor. 
			
			If no Peer entry lookup happened for this frame, the value
			 wil be set to 0, and the frame shall never be pushed to
			 REO entrance ring.
			<legal all>
*/

#define RX_MPDU_INFO_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET                             0x00000004
#define RX_MPDU_INFO_RX_REO_QUEUE_DESC_ADDR_31_0_LSB                                0
#define RX_MPDU_INFO_RX_REO_QUEUE_DESC_ADDR_31_0_MSB                                31
#define RX_MPDU_INFO_RX_REO_QUEUE_DESC_ADDR_31_0_MASK                               0xffffffff


/* Description		RX_REO_QUEUE_DESC_ADDR_39_32

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			Address (upper 8 bits) of the REO queue descriptor. 
			
			If no Peer entry lookup happened for this frame, the value
			 wil be set to 0, and the frame shall never be pushed to
			 REO entrance ring.
			<legal all>
*/

#define RX_MPDU_INFO_RX_REO_QUEUE_DESC_ADDR_39_32_OFFSET                            0x00000008
#define RX_MPDU_INFO_RX_REO_QUEUE_DESC_ADDR_39_32_LSB                               0
#define RX_MPDU_INFO_RX_REO_QUEUE_DESC_ADDR_39_32_MSB                               7
#define RX_MPDU_INFO_RX_REO_QUEUE_DESC_ADDR_39_32_MASK                              0x000000ff


/* Description		RECEIVE_QUEUE_NUMBER

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			Indicates the MPDU queue ID to which this MPDU link descriptor
			 belongs
			Used for tracking and debugging
			<legal all>
*/

#define RX_MPDU_INFO_RECEIVE_QUEUE_NUMBER_OFFSET                                    0x00000008
#define RX_MPDU_INFO_RECEIVE_QUEUE_NUMBER_LSB                                       8
#define RX_MPDU_INFO_RECEIVE_QUEUE_NUMBER_MSB                                       23
#define RX_MPDU_INFO_RECEIVE_QUEUE_NUMBER_MASK                                      0x00ffff00


/* Description		PRE_DELIM_ERR_WARNING

			Indicates that a delimiter FCS error was found in between
			 the Previous MPDU and this MPDU.
			
			Note that this is just a warning, and does not mean that
			 this MPDU is corrupted in any way. If it is, there will
			 be other errors indicated such as FCS or decrypt errors
			
			
			In case of ndp or phy_err, this field will indicate at least
			 one of delimiters located after the last MPDU in the previous
			 PPDU has been corrupted.
*/

#define RX_MPDU_INFO_PRE_DELIM_ERR_WARNING_OFFSET                                   0x00000008
#define RX_MPDU_INFO_PRE_DELIM_ERR_WARNING_LSB                                      24
#define RX_MPDU_INFO_PRE_DELIM_ERR_WARNING_MSB                                      24
#define RX_MPDU_INFO_PRE_DELIM_ERR_WARNING_MASK                                     0x01000000


/* Description		FIRST_DELIM_ERR

			Indicates that the first delimiter had a FCS failure.  Only
			 valid when first_mpdu and first_msdu are set.
			
			In case of ndp or phy_err, this field will never be set.
			
*/

#define RX_MPDU_INFO_FIRST_DELIM_ERR_OFFSET                                         0x00000008
#define RX_MPDU_INFO_FIRST_DELIM_ERR_LSB                                            25
#define RX_MPDU_INFO_FIRST_DELIM_ERR_MSB                                            25
#define RX_MPDU_INFO_FIRST_DELIM_ERR_MASK                                           0x02000000


/* Description		RESERVED_2A

			<legal 0>
*/

#define RX_MPDU_INFO_RESERVED_2A_OFFSET                                             0x00000008
#define RX_MPDU_INFO_RESERVED_2A_LSB                                                26
#define RX_MPDU_INFO_RESERVED_2A_MSB                                                31
#define RX_MPDU_INFO_RESERVED_2A_MASK                                               0xfc000000


/* Description		PN_31_0

			Field only valid when Frame_encryption_info_valid is set
			
			
			Bits [31:0] of the PN number extracted from the IV field
			
			WEP: IV = {key_id_octet, pn2, pn1, pn0}.  Only pn[23:0] 
			is valid.
			TKIP: IV = {pn5, pn4, pn3, pn2, key_id_octet, pn0, WEPSeed[1], 
			pn1}.  Only pn[47:0] is valid.
			AES-CCM: IV = {pn5, pn4, pn3, pn2, key_id_octet, 0x0, pn1, 
			pn0}.  Only pn[47:0] is valid.
			WAPI: IV = {key_id_octet, 0x0, pn15, pn14, pn13, pn12, pn11, 
			pn10, pn9, pn8, pn7, pn6, pn5, pn4, pn3, pn2, pn1, pn0}. 
			 pn[127:0] are valid.
			
			In case of ndp or phy_err, this field will never be set.
			
*/

#define RX_MPDU_INFO_PN_31_0_OFFSET                                                 0x0000000c
#define RX_MPDU_INFO_PN_31_0_LSB                                                    0
#define RX_MPDU_INFO_PN_31_0_MSB                                                    31
#define RX_MPDU_INFO_PN_31_0_MASK                                                   0xffffffff


/* Description		PN_63_32

			Field only valid when Frame_encryption_info_valid is set
			
			
			Bits [63:32] of the PN number.   See description for pn_31_0.
			
			
			In case of ndp or phy_err, this field will never be set.
			
*/

#define RX_MPDU_INFO_PN_63_32_OFFSET                                                0x00000010
#define RX_MPDU_INFO_PN_63_32_LSB                                                   0
#define RX_MPDU_INFO_PN_63_32_MSB                                                   31
#define RX_MPDU_INFO_PN_63_32_MASK                                                  0xffffffff


/* Description		PN_95_64

			Field only valid when Frame_encryption_info_valid is set
			
			
			Bits [95:64] of the PN number.  See description for pn_31_0.
			
			
			In case of ndp or phy_err, this field will never be set.
			
*/

#define RX_MPDU_INFO_PN_95_64_OFFSET                                                0x00000014
#define RX_MPDU_INFO_PN_95_64_LSB                                                   0
#define RX_MPDU_INFO_PN_95_64_MSB                                                   31
#define RX_MPDU_INFO_PN_95_64_MASK                                                  0xffffffff


/* Description		PN_127_96

			Field only valid when Frame_encryption_info_valid is set
			
			
			Bits [127:96] of the PN number.  See description for pn_31_0.
			
			
			In case of ndp or phy_err, this field will never be set.
			
*/

#define RX_MPDU_INFO_PN_127_96_OFFSET                                               0x00000018
#define RX_MPDU_INFO_PN_127_96_LSB                                                  0
#define RX_MPDU_INFO_PN_127_96_MSB                                                  31
#define RX_MPDU_INFO_PN_127_96_MASK                                                 0xffffffff


/* Description		EPD_EN

			Field only valid when AST_based_lookup_valid == 1.
			
			
			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			If set to one use EPD instead of LPD
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_INFO_EPD_EN_OFFSET                                                  0x0000001c
#define RX_MPDU_INFO_EPD_EN_LSB                                                     0
#define RX_MPDU_INFO_EPD_EN_MSB                                                     0
#define RX_MPDU_INFO_EPD_EN_MASK                                                    0x00000001


/* Description		ALL_FRAMES_SHALL_BE_ENCRYPTED

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			When set, all frames (data only ?) shall be encrypted. If
			 not, RX CRYPTO shall set an error flag.
			<legal all>
*/

#define RX_MPDU_INFO_ALL_FRAMES_SHALL_BE_ENCRYPTED_OFFSET                           0x0000001c
#define RX_MPDU_INFO_ALL_FRAMES_SHALL_BE_ENCRYPTED_LSB                              1
#define RX_MPDU_INFO_ALL_FRAMES_SHALL_BE_ENCRYPTED_MSB                              1
#define RX_MPDU_INFO_ALL_FRAMES_SHALL_BE_ENCRYPTED_MASK                             0x00000002


/* Description		ENCRYPT_TYPE

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			Indicates type of decrypt cipher used (as defined in the
			 peer entry)
			
			<enum 0 wep_40> WEP 40-bit
			<enum 1 wep_104> WEP 104-bit
			<enum 2 tkip_no_mic> TKIP without MIC
			<enum 3 wep_128> WEP 128-bit
			<enum 4 tkip_with_mic> TKIP with MIC
			<enum 5 wapi> WAPI
			<enum 6 aes_ccmp_128> AES CCMP 128
			<enum 7 no_cipher> No crypto
			<enum 8 aes_ccmp_256> AES CCMP 256
			<enum 9 aes_gcmp_128> AES CCMP 128
			<enum 10 aes_gcmp_256> AES CCMP 256
			<enum 11 wapi_gcm_sm4> WAPI GCM SM4
			
			<enum 12 wep_varied_width> WEP encryption. As for WEP per
			 keyid the key bit width can vary, the key bit width for
			 this MPDU will be indicated in field wep_key_width_for_variable
			 key
			<legal 0-12>
*/

#define RX_MPDU_INFO_ENCRYPT_TYPE_OFFSET                                            0x0000001c
#define RX_MPDU_INFO_ENCRYPT_TYPE_LSB                                               2
#define RX_MPDU_INFO_ENCRYPT_TYPE_MSB                                               5
#define RX_MPDU_INFO_ENCRYPT_TYPE_MASK                                              0x0000003c


/* Description		WEP_KEY_WIDTH_FOR_VARIABLE_KEY

			Field only valid when key_type is set to wep_varied_width. 
			
			
			This field indicates the size of the wep key for this MPDU.
			
			 
			<enum 0 wep_varied_width_40> WEP 40-bit
			<enum 1 wep_varied_width_104> WEP 104-bit
			<enum 2 wep_varied_width_128> WEP 128-bit
			
			<legal 0-2>
*/

#define RX_MPDU_INFO_WEP_KEY_WIDTH_FOR_VARIABLE_KEY_OFFSET                          0x0000001c
#define RX_MPDU_INFO_WEP_KEY_WIDTH_FOR_VARIABLE_KEY_LSB                             6
#define RX_MPDU_INFO_WEP_KEY_WIDTH_FOR_VARIABLE_KEY_MSB                             7
#define RX_MPDU_INFO_WEP_KEY_WIDTH_FOR_VARIABLE_KEY_MASK                            0x000000c0


/* Description		MESH_STA

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
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

#define RX_MPDU_INFO_MESH_STA_OFFSET                                                0x0000001c
#define RX_MPDU_INFO_MESH_STA_LSB                                                   8
#define RX_MPDU_INFO_MESH_STA_MSB                                                   9
#define RX_MPDU_INFO_MESH_STA_MASK                                                  0x00000300


/* Description		BSSID_HIT

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			When set, the BSSID of the incoming frame matched one of
			 the 8 BSSID register values
			
			<legal all>
*/

#define RX_MPDU_INFO_BSSID_HIT_OFFSET                                               0x0000001c
#define RX_MPDU_INFO_BSSID_HIT_LSB                                                  10
#define RX_MPDU_INFO_BSSID_HIT_MSB                                                  10
#define RX_MPDU_INFO_BSSID_HIT_MASK                                                 0x00000400


/* Description		BSSID_NUMBER

			Field only valid when bssid_hit is set.
			
			This number indicates which one out of the 8 BSSID register
			 values matched the incoming frame
			<legal all>
*/

#define RX_MPDU_INFO_BSSID_NUMBER_OFFSET                                            0x0000001c
#define RX_MPDU_INFO_BSSID_NUMBER_LSB                                               11
#define RX_MPDU_INFO_BSSID_NUMBER_MSB                                               14
#define RX_MPDU_INFO_BSSID_NUMBER_MASK                                              0x00007800


/* Description		TID

			Field only valid when mpdu_qos_control_valid is set
			
			The TID field in the QoS control field
			<legal all>
*/

#define RX_MPDU_INFO_TID_OFFSET                                                     0x0000001c
#define RX_MPDU_INFO_TID_LSB                                                        15
#define RX_MPDU_INFO_TID_MSB                                                        18
#define RX_MPDU_INFO_TID_MASK                                                       0x00078000


/* Description		RESERVED_7A

			<legal 0>
*/

#define RX_MPDU_INFO_RESERVED_7A_OFFSET                                             0x0000001c
#define RX_MPDU_INFO_RESERVED_7A_LSB                                                19
#define RX_MPDU_INFO_RESERVED_7A_MSB                                                31
#define RX_MPDU_INFO_RESERVED_7A_MASK                                               0xfff80000


/* Description		PEER_META_DATA

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			Meta data that SW has programmed in the Peer table entry
			 of the transmitting STA.
			<legal all>
*/

#define RX_MPDU_INFO_PEER_META_DATA_OFFSET                                          0x00000020
#define RX_MPDU_INFO_PEER_META_DATA_LSB                                             0
#define RX_MPDU_INFO_PEER_META_DATA_MSB                                             31
#define RX_MPDU_INFO_PEER_META_DATA_MASK                                            0xffffffff


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
			
			Note: for ndp frame, if it was expected because the preceding
			 NDPA was filter_pass, the setting  rxpcu_filter_pass will
			 be used. This setting will also be used for every ndp frame
			 in case Promiscuous mode is enabled.
			
			In case promiscuous is not enabled, and an NDP is not preceded
			 by a NPDA filter pass frame, the only other setting that
			 could appear here for the NDP is rxpcu_monitor_other. 
			(rxpcu has a configuration bit specifically for this scenario)
			
			
			Note: for 
			<legal 0-3>
*/

#define RX_MPDU_INFO_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET                           0x00000024
#define RX_MPDU_INFO_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB                              0
#define RX_MPDU_INFO_RXPCU_MPDU_FILTER_IN_CATEGORY_MSB                              1
#define RX_MPDU_INFO_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK                             0x00000003


/* Description		SW_FRAME_GROUP_ID

			SW processes frames based on certain classifications. This
			 field indicates to what sw classification this MPDU is 
			mapped.
			The classification is given in priority order
			
			<enum 0 sw_frame_group_NDP_frame> Note: The corresponding
			 Rxpcu_Mpdu_filter_in_category can be rxpcu_filter_pass 
			or rxpcu_monitor_other
			
			<enum 1 sw_frame_group_Multicast_data> 
			<enum 2 sw_frame_group_Unicast_data> 
			<enum 3 sw_frame_group_Null_data > This includes mpdus of
			 type Data Null.
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
			Note: The corresponding Rxpcu_Mpdu_filter_in_category can
			 only be rxpcu_monitor_other
			
			<enum 37 sw_frame_group_phy_error> PHY reported an error
			
			Note: The corresponding Rxpcu_Mpdu_filter_in_category can
			 be rxpcu_filter_pass
			
			<legal 0-39>
*/

#define RX_MPDU_INFO_SW_FRAME_GROUP_ID_OFFSET                                       0x00000024
#define RX_MPDU_INFO_SW_FRAME_GROUP_ID_LSB                                          2
#define RX_MPDU_INFO_SW_FRAME_GROUP_ID_MSB                                          8
#define RX_MPDU_INFO_SW_FRAME_GROUP_ID_MASK                                         0x000001fc


/* Description		NDP_FRAME

			When set, the received frame was an NDP frame, and thus 
			there will be no MPDU data.
			TODO: Should this be extended to 2-bit e-num?
			<legal all>
*/

#define RX_MPDU_INFO_NDP_FRAME_OFFSET                                               0x00000024
#define RX_MPDU_INFO_NDP_FRAME_LSB                                                  9
#define RX_MPDU_INFO_NDP_FRAME_MSB                                                  9
#define RX_MPDU_INFO_NDP_FRAME_MASK                                                 0x00000200


/* Description		PHY_ERR

			When set, a PHY error was received before MAC received any
			 data, and thus there will be no MPDU data.
			<legal all>
*/

#define RX_MPDU_INFO_PHY_ERR_OFFSET                                                 0x00000024
#define RX_MPDU_INFO_PHY_ERR_LSB                                                    10
#define RX_MPDU_INFO_PHY_ERR_MSB                                                    10
#define RX_MPDU_INFO_PHY_ERR_MASK                                                   0x00000400


/* Description		PHY_ERR_DURING_MPDU_HEADER

			When set, a PHY error was received before MAC received the
			 complete MPDU header which was needed for proper decoding
			
			<legal all>
*/

#define RX_MPDU_INFO_PHY_ERR_DURING_MPDU_HEADER_OFFSET                              0x00000024
#define RX_MPDU_INFO_PHY_ERR_DURING_MPDU_HEADER_LSB                                 11
#define RX_MPDU_INFO_PHY_ERR_DURING_MPDU_HEADER_MSB                                 11
#define RX_MPDU_INFO_PHY_ERR_DURING_MPDU_HEADER_MASK                                0x00000800


/* Description		PROTOCOL_VERSION_ERR

			Set when RXPCU detected a version error in the Frame control
			 field
			<legal all>
*/

#define RX_MPDU_INFO_PROTOCOL_VERSION_ERR_OFFSET                                    0x00000024
#define RX_MPDU_INFO_PROTOCOL_VERSION_ERR_LSB                                       12
#define RX_MPDU_INFO_PROTOCOL_VERSION_ERR_MSB                                       12
#define RX_MPDU_INFO_PROTOCOL_VERSION_ERR_MASK                                      0x00001000


/* Description		AST_BASED_LOOKUP_VALID

			When set, AST based lookup for this frame has found a valid
			 result.
			
			Note that for NDP frame this will never be set
			<legal all>
*/

#define RX_MPDU_INFO_AST_BASED_LOOKUP_VALID_OFFSET                                  0x00000024
#define RX_MPDU_INFO_AST_BASED_LOOKUP_VALID_LSB                                     13
#define RX_MPDU_INFO_AST_BASED_LOOKUP_VALID_MSB                                     13
#define RX_MPDU_INFO_AST_BASED_LOOKUP_VALID_MASK                                    0x00002000


/* Description		RANGING

			When set, a ranging NDPA or a ranging NDP was received.
			
			This field is only for FW visibility. HW is not expected
			 to take any action on this.
			<legal all>
*/

#define RX_MPDU_INFO_RANGING_OFFSET                                                 0x00000024
#define RX_MPDU_INFO_RANGING_LSB                                                    14
#define RX_MPDU_INFO_RANGING_MSB                                                    14
#define RX_MPDU_INFO_RANGING_MASK                                                   0x00004000


/* Description		RESERVED_9A

			<legal 0>
*/

#define RX_MPDU_INFO_RESERVED_9A_OFFSET                                             0x00000024
#define RX_MPDU_INFO_RESERVED_9A_LSB                                                15
#define RX_MPDU_INFO_RESERVED_9A_MSB                                                15
#define RX_MPDU_INFO_RESERVED_9A_MASK                                               0x00008000


/* Description		PHY_PPDU_ID

			A ppdu counter value that PHY increments for every PPDU 
			received. The counter value wraps around  
			<legal all>
*/

#define RX_MPDU_INFO_PHY_PPDU_ID_OFFSET                                             0x00000024
#define RX_MPDU_INFO_PHY_PPDU_ID_LSB                                                16
#define RX_MPDU_INFO_PHY_PPDU_ID_MSB                                                31
#define RX_MPDU_INFO_PHY_PPDU_ID_MASK                                               0xffff0000


/* Description		AST_INDEX

			This field indicates the index of the AST entry corresponding
			 to this MPDU. It is provided by the GSE module instantiated
			 in RXPCU.
			A value of 0xFFFF indicates an invalid AST index, meaning
			 that No AST entry was found or NO AST search was performed
			
			
			In case of ndp or phy_err, this field will be set to 0xFFFF
			
			<legal all>
*/

#define RX_MPDU_INFO_AST_INDEX_OFFSET                                               0x00000028
#define RX_MPDU_INFO_AST_INDEX_LSB                                                  0
#define RX_MPDU_INFO_AST_INDEX_MSB                                                  15
#define RX_MPDU_INFO_AST_INDEX_MASK                                                 0x0000ffff


/* Description		SW_PEER_ID

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			This field indicates a unique peer identifier. It is set
			 equal to field 'sw_peer_id' from the AST entry
			
			<legal all>
*/

#define RX_MPDU_INFO_SW_PEER_ID_OFFSET                                              0x00000028
#define RX_MPDU_INFO_SW_PEER_ID_LSB                                                 16
#define RX_MPDU_INFO_SW_PEER_ID_MSB                                                 31
#define RX_MPDU_INFO_SW_PEER_ID_MASK                                                0xffff0000


/* Description		MPDU_FRAME_CONTROL_VALID

			When set, the field Mpdu_Frame_control_field has valid information
			
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_FRAME_CONTROL_VALID_OFFSET                                0x0000002c
#define RX_MPDU_INFO_MPDU_FRAME_CONTROL_VALID_LSB                                   0
#define RX_MPDU_INFO_MPDU_FRAME_CONTROL_VALID_MSB                                   0
#define RX_MPDU_INFO_MPDU_FRAME_CONTROL_VALID_MASK                                  0x00000001


/* Description		MPDU_DURATION_VALID

			When set, the field Mpdu_duration_field has valid information
			
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_DURATION_VALID_OFFSET                                     0x0000002c
#define RX_MPDU_INFO_MPDU_DURATION_VALID_LSB                                        1
#define RX_MPDU_INFO_MPDU_DURATION_VALID_MSB                                        1
#define RX_MPDU_INFO_MPDU_DURATION_VALID_MASK                                       0x00000002


/* Description		MAC_ADDR_AD1_VALID

			When set, the fields mac_addr_ad1_..... have valid information
			
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_INFO_MAC_ADDR_AD1_VALID_OFFSET                                      0x0000002c
#define RX_MPDU_INFO_MAC_ADDR_AD1_VALID_LSB                                         2
#define RX_MPDU_INFO_MAC_ADDR_AD1_VALID_MSB                                         2
#define RX_MPDU_INFO_MAC_ADDR_AD1_VALID_MASK                                        0x00000004


/* Description		MAC_ADDR_AD2_VALID

			When set, the fields mac_addr_ad2_..... have valid information
			
			
			For MPDUs without Address 2, this field will not be set.
			
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_INFO_MAC_ADDR_AD2_VALID_OFFSET                                      0x0000002c
#define RX_MPDU_INFO_MAC_ADDR_AD2_VALID_LSB                                         3
#define RX_MPDU_INFO_MAC_ADDR_AD2_VALID_MSB                                         3
#define RX_MPDU_INFO_MAC_ADDR_AD2_VALID_MASK                                        0x00000008


/* Description		MAC_ADDR_AD3_VALID

			When set, the fields mac_addr_ad3_..... have valid information
			
			
			For MPDUs without Address 3, this field will not be set.
			
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_INFO_MAC_ADDR_AD3_VALID_OFFSET                                      0x0000002c
#define RX_MPDU_INFO_MAC_ADDR_AD3_VALID_LSB                                         4
#define RX_MPDU_INFO_MAC_ADDR_AD3_VALID_MSB                                         4
#define RX_MPDU_INFO_MAC_ADDR_AD3_VALID_MASK                                        0x00000010


/* Description		MAC_ADDR_AD4_VALID

			When set, the fields mac_addr_ad4_..... have valid information
			
			
			For MPDUs without Address 4, this field will not be set.
			
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_INFO_MAC_ADDR_AD4_VALID_OFFSET                                      0x0000002c
#define RX_MPDU_INFO_MAC_ADDR_AD4_VALID_LSB                                         5
#define RX_MPDU_INFO_MAC_ADDR_AD4_VALID_MSB                                         5
#define RX_MPDU_INFO_MAC_ADDR_AD4_VALID_MASK                                        0x00000020


/* Description		MPDU_SEQUENCE_CONTROL_VALID

			When set, the fields mpdu_sequence_control_field and mpdu_sequence_number
			 have valid information as well as field 
			
			For MPDUs without a sequence control field, this field will
			 not be set.
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_SEQUENCE_CONTROL_VALID_OFFSET                             0x0000002c
#define RX_MPDU_INFO_MPDU_SEQUENCE_CONTROL_VALID_LSB                                6
#define RX_MPDU_INFO_MPDU_SEQUENCE_CONTROL_VALID_MSB                                6
#define RX_MPDU_INFO_MPDU_SEQUENCE_CONTROL_VALID_MASK                               0x00000040


/* Description		MPDU_QOS_CONTROL_VALID

			When set, the field mpdu_qos_control_field has valid information
			
			
			For MPDUs without a QoS control field, this field will not
			 be set.
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_QOS_CONTROL_VALID_OFFSET                                  0x0000002c
#define RX_MPDU_INFO_MPDU_QOS_CONTROL_VALID_LSB                                     7
#define RX_MPDU_INFO_MPDU_QOS_CONTROL_VALID_MSB                                     7
#define RX_MPDU_INFO_MPDU_QOS_CONTROL_VALID_MASK                                    0x00000080


/* Description		MPDU_HT_CONTROL_VALID

			When set, the field mpdu_HT_control_field has valid information
			
			
			For MPDUs without a HT control field, this field will not
			 be set.
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_HT_CONTROL_VALID_OFFSET                                   0x0000002c
#define RX_MPDU_INFO_MPDU_HT_CONTROL_VALID_LSB                                      8
#define RX_MPDU_INFO_MPDU_HT_CONTROL_VALID_MSB                                      8
#define RX_MPDU_INFO_MPDU_HT_CONTROL_VALID_MASK                                     0x00000100


/* Description		FRAME_ENCRYPTION_INFO_VALID

			When set, the encryption related info fields, like IV and
			 PN are valid
			
			For MPDUs that are not encrypted, this will not be set.
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_INFO_FRAME_ENCRYPTION_INFO_VALID_OFFSET                             0x0000002c
#define RX_MPDU_INFO_FRAME_ENCRYPTION_INFO_VALID_LSB                                9
#define RX_MPDU_INFO_FRAME_ENCRYPTION_INFO_VALID_MSB                                9
#define RX_MPDU_INFO_FRAME_ENCRYPTION_INFO_VALID_MASK                               0x00000200


/* Description		MPDU_FRAGMENT_NUMBER

			Field only valid when Mpdu_sequence_control_valid is set
			 AND Fragment_flag is set 
			
			The fragment number from the 802.11 header
			
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_FRAGMENT_NUMBER_OFFSET                                    0x0000002c
#define RX_MPDU_INFO_MPDU_FRAGMENT_NUMBER_LSB                                       10
#define RX_MPDU_INFO_MPDU_FRAGMENT_NUMBER_MSB                                       13
#define RX_MPDU_INFO_MPDU_FRAGMENT_NUMBER_MASK                                      0x00003c00


/* Description		MORE_FRAGMENT_FLAG

			The More Fragment bit setting from the MPDU header of the
			 received frame
			
			<legal all>
*/

#define RX_MPDU_INFO_MORE_FRAGMENT_FLAG_OFFSET                                      0x0000002c
#define RX_MPDU_INFO_MORE_FRAGMENT_FLAG_LSB                                         14
#define RX_MPDU_INFO_MORE_FRAGMENT_FLAG_MSB                                         14
#define RX_MPDU_INFO_MORE_FRAGMENT_FLAG_MASK                                        0x00004000


/* Description		RESERVED_11A

			<legal 0>
*/

#define RX_MPDU_INFO_RESERVED_11A_OFFSET                                            0x0000002c
#define RX_MPDU_INFO_RESERVED_11A_LSB                                               15
#define RX_MPDU_INFO_RESERVED_11A_MSB                                               15
#define RX_MPDU_INFO_RESERVED_11A_MASK                                              0x00008000


/* Description		FR_DS

			Field only valid when Mpdu_frame_control_valid is set
			
			Set if the from DS bit is set in the frame control.
			<legal all>
*/

#define RX_MPDU_INFO_FR_DS_OFFSET                                                   0x0000002c
#define RX_MPDU_INFO_FR_DS_LSB                                                      16
#define RX_MPDU_INFO_FR_DS_MSB                                                      16
#define RX_MPDU_INFO_FR_DS_MASK                                                     0x00010000


/* Description		TO_DS

			Field only valid when Mpdu_frame_control_valid is set 
			
			Set if the to DS bit is set in the frame control.
			<legal all>
*/

#define RX_MPDU_INFO_TO_DS_OFFSET                                                   0x0000002c
#define RX_MPDU_INFO_TO_DS_LSB                                                      17
#define RX_MPDU_INFO_TO_DS_MSB                                                      17
#define RX_MPDU_INFO_TO_DS_MASK                                                     0x00020000


/* Description		ENCRYPTED

			Field only valid when Mpdu_frame_control_valid is set.
			
			Protected bit from the frame control.  
			<legal all>
*/

#define RX_MPDU_INFO_ENCRYPTED_OFFSET                                               0x0000002c
#define RX_MPDU_INFO_ENCRYPTED_LSB                                                  18
#define RX_MPDU_INFO_ENCRYPTED_MSB                                                  18
#define RX_MPDU_INFO_ENCRYPTED_MASK                                                 0x00040000


/* Description		MPDU_RETRY

			Field only valid when Mpdu_frame_control_valid is set.
			
			Retry bit from the frame control.  Only valid when first_msdu
			 is set.
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_RETRY_OFFSET                                              0x0000002c
#define RX_MPDU_INFO_MPDU_RETRY_LSB                                                 19
#define RX_MPDU_INFO_MPDU_RETRY_MSB                                                 19
#define RX_MPDU_INFO_MPDU_RETRY_MASK                                                0x00080000


/* Description		MPDU_SEQUENCE_NUMBER

			Field only valid when Mpdu_sequence_control_valid is set.
			
			
			The sequence number from the 802.11 header.
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_SEQUENCE_NUMBER_OFFSET                                    0x0000002c
#define RX_MPDU_INFO_MPDU_SEQUENCE_NUMBER_LSB                                       20
#define RX_MPDU_INFO_MPDU_SEQUENCE_NUMBER_MSB                                       31
#define RX_MPDU_INFO_MPDU_SEQUENCE_NUMBER_MASK                                      0xfff00000


/* Description		KEY_ID_OCTET

			Field only valid when Frame_encryption_info_valid is set
			
			
			The key ID octet from the IV.
			
			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			<legal all>
*/

#define RX_MPDU_INFO_KEY_ID_OCTET_OFFSET                                            0x00000030
#define RX_MPDU_INFO_KEY_ID_OCTET_LSB                                               0
#define RX_MPDU_INFO_KEY_ID_OCTET_MSB                                               7
#define RX_MPDU_INFO_KEY_ID_OCTET_MASK                                              0x000000ff


/* Description		NEW_PEER_ENTRY

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			Set if new RX_PEER_ENTRY TLV follows. If clear, RX_PEER_ENTRY
			 doesn't follow so RX DECRYPTION module either uses old 
			peer entry or not decrypt. 
			<legal all>
*/

#define RX_MPDU_INFO_NEW_PEER_ENTRY_OFFSET                                          0x00000030
#define RX_MPDU_INFO_NEW_PEER_ENTRY_LSB                                             8
#define RX_MPDU_INFO_NEW_PEER_ENTRY_MSB                                             8
#define RX_MPDU_INFO_NEW_PEER_ENTRY_MASK                                            0x00000100


/* Description		DECRYPT_NEEDED

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			Set if decryption is needed. 
			
			Note:
			When RXPCU sets bit 'ast_index_not_found' and/or ast_index_timeout', 
			RXPCU will also ensure that this bit is NOT set
			CRYPTO for that reason only needs to evaluate this bit and
			 non of the other ones.
			<legal all>
*/

#define RX_MPDU_INFO_DECRYPT_NEEDED_OFFSET                                          0x00000030
#define RX_MPDU_INFO_DECRYPT_NEEDED_LSB                                             9
#define RX_MPDU_INFO_DECRYPT_NEEDED_MSB                                             9
#define RX_MPDU_INFO_DECRYPT_NEEDED_MASK                                            0x00000200


/* Description		DECAP_TYPE

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			Used by the OLE during decapsulation.
			
			Indicates the decapsulation that HW will perform:
			
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			<legal all>
*/

#define RX_MPDU_INFO_DECAP_TYPE_OFFSET                                              0x00000030
#define RX_MPDU_INFO_DECAP_TYPE_LSB                                                 10
#define RX_MPDU_INFO_DECAP_TYPE_MSB                                                 11
#define RX_MPDU_INFO_DECAP_TYPE_MASK                                                0x00000c00


/* Description		RX_INSERT_VLAN_C_TAG_PADDING

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			Insert 4 byte of all zeros as VLAN tag if the rx payload
			 does not have VLAN. Used during decapsulation. 
			<legal all>
*/

#define RX_MPDU_INFO_RX_INSERT_VLAN_C_TAG_PADDING_OFFSET                            0x00000030
#define RX_MPDU_INFO_RX_INSERT_VLAN_C_TAG_PADDING_LSB                               12
#define RX_MPDU_INFO_RX_INSERT_VLAN_C_TAG_PADDING_MSB                               12
#define RX_MPDU_INFO_RX_INSERT_VLAN_C_TAG_PADDING_MASK                              0x00001000


/* Description		RX_INSERT_VLAN_S_TAG_PADDING

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			Insert 4 byte of all zeros as double VLAN tag if the rx 
			payload does not have VLAN. Used during 
			<legal all>
*/

#define RX_MPDU_INFO_RX_INSERT_VLAN_S_TAG_PADDING_OFFSET                            0x00000030
#define RX_MPDU_INFO_RX_INSERT_VLAN_S_TAG_PADDING_LSB                               13
#define RX_MPDU_INFO_RX_INSERT_VLAN_S_TAG_PADDING_MSB                               13
#define RX_MPDU_INFO_RX_INSERT_VLAN_S_TAG_PADDING_MASK                              0x00002000


/* Description		STRIP_VLAN_C_TAG_DECAP

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			Strip the VLAN during decapsulation.  Used by the OLE.
			<legal all>
*/

#define RX_MPDU_INFO_STRIP_VLAN_C_TAG_DECAP_OFFSET                                  0x00000030
#define RX_MPDU_INFO_STRIP_VLAN_C_TAG_DECAP_LSB                                     14
#define RX_MPDU_INFO_STRIP_VLAN_C_TAG_DECAP_MSB                                     14
#define RX_MPDU_INFO_STRIP_VLAN_C_TAG_DECAP_MASK                                    0x00004000


/* Description		STRIP_VLAN_S_TAG_DECAP

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			Strip the double VLAN during decapsulation.  Used by the
			 OLE.
			<legal all>
*/

#define RX_MPDU_INFO_STRIP_VLAN_S_TAG_DECAP_OFFSET                                  0x00000030
#define RX_MPDU_INFO_STRIP_VLAN_S_TAG_DECAP_LSB                                     15
#define RX_MPDU_INFO_STRIP_VLAN_S_TAG_DECAP_MSB                                     15
#define RX_MPDU_INFO_STRIP_VLAN_S_TAG_DECAP_MASK                                    0x00008000


/* Description		PRE_DELIM_COUNT

			The number of delimiters before this MPDU.  
			
			Note that this number is cleared at PPDU start.
			
			If this MPDU is the first received MPDU in the PPDU and 
			this MPDU gets filtered-in, this field will indicate the
			 number of delimiters located after the last MPDU in the
			 previous PPDU.
			
			If this MPDU is located after the first received MPDU in
			 an PPDU, this field will indicate the number of delimiters
			 located between the previous MPDU and this MPDU.
			
			In case of ndp or phy_err, this field will indicate the 
			number of delimiters located after the last MPDU in the 
			previous PPDU.
			<legal all>
*/

#define RX_MPDU_INFO_PRE_DELIM_COUNT_OFFSET                                         0x00000030
#define RX_MPDU_INFO_PRE_DELIM_COUNT_LSB                                            16
#define RX_MPDU_INFO_PRE_DELIM_COUNT_MSB                                            27
#define RX_MPDU_INFO_PRE_DELIM_COUNT_MASK                                           0x0fff0000


/* Description		AMPDU_FLAG

			When set, received frame was part of an A-MPDU.
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_INFO_AMPDU_FLAG_OFFSET                                              0x00000030
#define RX_MPDU_INFO_AMPDU_FLAG_LSB                                                 28
#define RX_MPDU_INFO_AMPDU_FLAG_MSB                                                 28
#define RX_MPDU_INFO_AMPDU_FLAG_MASK                                                0x10000000


/* Description		BAR_FRAME

			In case of ndp or phy_err or AST_based_lookup_valid == 0, 
			this field will be set to 0
			
			When set, received frame is a BAR frame
			<legal all>
*/

#define RX_MPDU_INFO_BAR_FRAME_OFFSET                                               0x00000030
#define RX_MPDU_INFO_BAR_FRAME_LSB                                                  29
#define RX_MPDU_INFO_BAR_FRAME_MSB                                                  29
#define RX_MPDU_INFO_BAR_FRAME_MASK                                                 0x20000000


/* Description		RAW_MPDU

			Consumer: SW
			Producer: RXOLE
			
			RXPCU sets this field to 0 and RXOLE overwrites it.
			
			Set to 1 by RXOLE when it has not performed any 802.11 to
			 Ethernet/Natvie WiFi header conversion on this MPDU.
			<legal all>
*/

#define RX_MPDU_INFO_RAW_MPDU_OFFSET                                                0x00000030
#define RX_MPDU_INFO_RAW_MPDU_LSB                                                   30
#define RX_MPDU_INFO_RAW_MPDU_MSB                                                   30
#define RX_MPDU_INFO_RAW_MPDU_MASK                                                  0x40000000


/* Description		RESERVED_12

			<legal 0>
*/

#define RX_MPDU_INFO_RESERVED_12_OFFSET                                             0x00000030
#define RX_MPDU_INFO_RESERVED_12_LSB                                                31
#define RX_MPDU_INFO_RESERVED_12_MSB                                                31
#define RX_MPDU_INFO_RESERVED_12_MASK                                               0x80000000


/* Description		MPDU_LENGTH

			In case of ndp or phy_err this field will be set to 0
			
			MPDU length before decapsulation.
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_LENGTH_OFFSET                                             0x00000034
#define RX_MPDU_INFO_MPDU_LENGTH_LSB                                                0
#define RX_MPDU_INFO_MPDU_LENGTH_MSB                                                13
#define RX_MPDU_INFO_MPDU_LENGTH_MASK                                               0x00003fff


/* Description		FIRST_MPDU

			See definition in RX attention descriptor
			
			In case of ndp or phy_err, this field will be set. Note 
			however that there will not actually be any data contents
			 in the MPDU.
			<legal all>
*/

#define RX_MPDU_INFO_FIRST_MPDU_OFFSET                                              0x00000034
#define RX_MPDU_INFO_FIRST_MPDU_LSB                                                 14
#define RX_MPDU_INFO_FIRST_MPDU_MSB                                                 14
#define RX_MPDU_INFO_FIRST_MPDU_MASK                                                0x00004000


/* Description		MCAST_BCAST

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_MCAST_BCAST_OFFSET                                             0x00000034
#define RX_MPDU_INFO_MCAST_BCAST_LSB                                                15
#define RX_MPDU_INFO_MCAST_BCAST_MSB                                                15
#define RX_MPDU_INFO_MCAST_BCAST_MASK                                               0x00008000


/* Description		AST_INDEX_NOT_FOUND

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_AST_INDEX_NOT_FOUND_OFFSET                                     0x00000034
#define RX_MPDU_INFO_AST_INDEX_NOT_FOUND_LSB                                        16
#define RX_MPDU_INFO_AST_INDEX_NOT_FOUND_MSB                                        16
#define RX_MPDU_INFO_AST_INDEX_NOT_FOUND_MASK                                       0x00010000


/* Description		AST_INDEX_TIMEOUT

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_AST_INDEX_TIMEOUT_OFFSET                                       0x00000034
#define RX_MPDU_INFO_AST_INDEX_TIMEOUT_LSB                                          17
#define RX_MPDU_INFO_AST_INDEX_TIMEOUT_MSB                                          17
#define RX_MPDU_INFO_AST_INDEX_TIMEOUT_MASK                                         0x00020000


/* Description		POWER_MGMT

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_POWER_MGMT_OFFSET                                              0x00000034
#define RX_MPDU_INFO_POWER_MGMT_LSB                                                 18
#define RX_MPDU_INFO_POWER_MGMT_MSB                                                 18
#define RX_MPDU_INFO_POWER_MGMT_MASK                                                0x00040000


/* Description		NON_QOS

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 1
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_NON_QOS_OFFSET                                                 0x00000034
#define RX_MPDU_INFO_NON_QOS_LSB                                                    19
#define RX_MPDU_INFO_NON_QOS_MSB                                                    19
#define RX_MPDU_INFO_NON_QOS_MASK                                                   0x00080000


/* Description		NULL_DATA

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_NULL_DATA_OFFSET                                               0x00000034
#define RX_MPDU_INFO_NULL_DATA_LSB                                                  20
#define RX_MPDU_INFO_NULL_DATA_MSB                                                  20
#define RX_MPDU_INFO_NULL_DATA_MASK                                                 0x00100000


/* Description		MGMT_TYPE

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_MGMT_TYPE_OFFSET                                               0x00000034
#define RX_MPDU_INFO_MGMT_TYPE_LSB                                                  21
#define RX_MPDU_INFO_MGMT_TYPE_MSB                                                  21
#define RX_MPDU_INFO_MGMT_TYPE_MASK                                                 0x00200000


/* Description		CTRL_TYPE

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_CTRL_TYPE_OFFSET                                               0x00000034
#define RX_MPDU_INFO_CTRL_TYPE_LSB                                                  22
#define RX_MPDU_INFO_CTRL_TYPE_MSB                                                  22
#define RX_MPDU_INFO_CTRL_TYPE_MASK                                                 0x00400000


/* Description		MORE_DATA

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_MORE_DATA_OFFSET                                               0x00000034
#define RX_MPDU_INFO_MORE_DATA_LSB                                                  23
#define RX_MPDU_INFO_MORE_DATA_MSB                                                  23
#define RX_MPDU_INFO_MORE_DATA_MASK                                                 0x00800000


/* Description		EOSP

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_EOSP_OFFSET                                                    0x00000034
#define RX_MPDU_INFO_EOSP_LSB                                                       24
#define RX_MPDU_INFO_EOSP_MSB                                                       24
#define RX_MPDU_INFO_EOSP_MASK                                                      0x01000000


/* Description		FRAGMENT_FLAG

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_FRAGMENT_FLAG_OFFSET                                           0x00000034
#define RX_MPDU_INFO_FRAGMENT_FLAG_LSB                                              25
#define RX_MPDU_INFO_FRAGMENT_FLAG_MSB                                              25
#define RX_MPDU_INFO_FRAGMENT_FLAG_MASK                                             0x02000000


/* Description		ORDER

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			
			<legal all>
*/

#define RX_MPDU_INFO_ORDER_OFFSET                                                   0x00000034
#define RX_MPDU_INFO_ORDER_LSB                                                      26
#define RX_MPDU_INFO_ORDER_MSB                                                      26
#define RX_MPDU_INFO_ORDER_MASK                                                     0x04000000


/* Description		U_APSD_TRIGGER

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_U_APSD_TRIGGER_OFFSET                                          0x00000034
#define RX_MPDU_INFO_U_APSD_TRIGGER_LSB                                             27
#define RX_MPDU_INFO_U_APSD_TRIGGER_MSB                                             27
#define RX_MPDU_INFO_U_APSD_TRIGGER_MASK                                            0x08000000


/* Description		ENCRYPT_REQUIRED

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_ENCRYPT_REQUIRED_OFFSET                                        0x00000034
#define RX_MPDU_INFO_ENCRYPT_REQUIRED_LSB                                           28
#define RX_MPDU_INFO_ENCRYPT_REQUIRED_MSB                                           28
#define RX_MPDU_INFO_ENCRYPT_REQUIRED_MASK                                          0x10000000


/* Description		DIRECTED

			In case of ndp or phy_err or Phy_err_during_mpdu_header 
			this field will be set to 0
			
			See definition in RX attention descriptor
			<legal all>
*/

#define RX_MPDU_INFO_DIRECTED_OFFSET                                                0x00000034
#define RX_MPDU_INFO_DIRECTED_LSB                                                   29
#define RX_MPDU_INFO_DIRECTED_MSB                                                   29
#define RX_MPDU_INFO_DIRECTED_MASK                                                  0x20000000


/* Description		AMSDU_PRESENT

			Field only valid when Mpdu_qos_control_valid is set
			
			The 'amsdu_present' bit within the QoS control field of 
			the MPDU
			<legal all>
*/

#define RX_MPDU_INFO_AMSDU_PRESENT_OFFSET                                           0x00000034
#define RX_MPDU_INFO_AMSDU_PRESENT_LSB                                              30
#define RX_MPDU_INFO_AMSDU_PRESENT_MSB                                              30
#define RX_MPDU_INFO_AMSDU_PRESENT_MASK                                             0x40000000


/* Description		RESERVED_13

			Field only valid when Mpdu_qos_control_valid is set
			
			This indicates whether the 'Ack policy' field within the
			 QoS control field of the MPDU indicates 'no-Ack.'
			<legal all>
*/

#define RX_MPDU_INFO_RESERVED_13_OFFSET                                             0x00000034
#define RX_MPDU_INFO_RESERVED_13_LSB                                                31
#define RX_MPDU_INFO_RESERVED_13_MSB                                                31
#define RX_MPDU_INFO_RESERVED_13_MASK                                               0x80000000


/* Description		MPDU_FRAME_CONTROL_FIELD

			Field only valid when Mpdu_frame_control_valid is set
			
			The frame control field of this received MPDU.
			
			Field only valid when Ndp_frame and phy_err are NOT set
			
			Bytes 0 + 1 of the received MPDU
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_FRAME_CONTROL_FIELD_OFFSET                                0x00000038
#define RX_MPDU_INFO_MPDU_FRAME_CONTROL_FIELD_LSB                                   0
#define RX_MPDU_INFO_MPDU_FRAME_CONTROL_FIELD_MSB                                   15
#define RX_MPDU_INFO_MPDU_FRAME_CONTROL_FIELD_MASK                                  0x0000ffff


/* Description		MPDU_DURATION_FIELD

			Field only valid when Mpdu_duration_valid is set
			
			The duration field of this received MPDU.
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_DURATION_FIELD_OFFSET                                     0x00000038
#define RX_MPDU_INFO_MPDU_DURATION_FIELD_LSB                                        16
#define RX_MPDU_INFO_MPDU_DURATION_FIELD_MSB                                        31
#define RX_MPDU_INFO_MPDU_DURATION_FIELD_MASK                                       0xffff0000


/* Description		MAC_ADDR_AD1_31_0

			Field only valid when mac_addr_ad1_valid is set
			
			The Least Significant 4 bytes of the Received Frames MAC
			 Address AD1
			<legal all>
*/

#define RX_MPDU_INFO_MAC_ADDR_AD1_31_0_OFFSET                                       0x0000003c
#define RX_MPDU_INFO_MAC_ADDR_AD1_31_0_LSB                                          0
#define RX_MPDU_INFO_MAC_ADDR_AD1_31_0_MSB                                          31
#define RX_MPDU_INFO_MAC_ADDR_AD1_31_0_MASK                                         0xffffffff


/* Description		MAC_ADDR_AD1_47_32

			Field only valid when mac_addr_ad1_valid is set
			
			The 2 most significant bytes of the Received Frames MAC 
			Address AD1
			<legal all>
*/

#define RX_MPDU_INFO_MAC_ADDR_AD1_47_32_OFFSET                                      0x00000040
#define RX_MPDU_INFO_MAC_ADDR_AD1_47_32_LSB                                         0
#define RX_MPDU_INFO_MAC_ADDR_AD1_47_32_MSB                                         15
#define RX_MPDU_INFO_MAC_ADDR_AD1_47_32_MASK                                        0x0000ffff


/* Description		MAC_ADDR_AD2_15_0

			Field only valid when mac_addr_ad2_valid is set
			
			The Least Significant 2 bytes of the Received Frames MAC
			 Address AD2
			<legal all>
*/

#define RX_MPDU_INFO_MAC_ADDR_AD2_15_0_OFFSET                                       0x00000040
#define RX_MPDU_INFO_MAC_ADDR_AD2_15_0_LSB                                          16
#define RX_MPDU_INFO_MAC_ADDR_AD2_15_0_MSB                                          31
#define RX_MPDU_INFO_MAC_ADDR_AD2_15_0_MASK                                         0xffff0000


/* Description		MAC_ADDR_AD2_47_16

			Field only valid when mac_addr_ad2_valid is set
			
			The 4 most significant bytes of the Received Frames MAC 
			Address AD2
			<legal all>
*/

#define RX_MPDU_INFO_MAC_ADDR_AD2_47_16_OFFSET                                      0x00000044
#define RX_MPDU_INFO_MAC_ADDR_AD2_47_16_LSB                                         0
#define RX_MPDU_INFO_MAC_ADDR_AD2_47_16_MSB                                         31
#define RX_MPDU_INFO_MAC_ADDR_AD2_47_16_MASK                                        0xffffffff


/* Description		MAC_ADDR_AD3_31_0

			Field only valid when mac_addr_ad3_valid is set
			
			The Least Significant 4 bytes of the Received Frames MAC
			 Address AD3
			<legal all>
*/

#define RX_MPDU_INFO_MAC_ADDR_AD3_31_0_OFFSET                                       0x00000048
#define RX_MPDU_INFO_MAC_ADDR_AD3_31_0_LSB                                          0
#define RX_MPDU_INFO_MAC_ADDR_AD3_31_0_MSB                                          31
#define RX_MPDU_INFO_MAC_ADDR_AD3_31_0_MASK                                         0xffffffff


/* Description		MAC_ADDR_AD3_47_32

			Field only valid when mac_addr_ad3_valid is set
			
			The 2 most significant bytes of the Received Frames MAC 
			Address AD3
			<legal all>
*/

#define RX_MPDU_INFO_MAC_ADDR_AD3_47_32_OFFSET                                      0x0000004c
#define RX_MPDU_INFO_MAC_ADDR_AD3_47_32_LSB                                         0
#define RX_MPDU_INFO_MAC_ADDR_AD3_47_32_MSB                                         15
#define RX_MPDU_INFO_MAC_ADDR_AD3_47_32_MASK                                        0x0000ffff


/* Description		MPDU_SEQUENCE_CONTROL_FIELD

			Field only valid when mpdu_sequence_control_valid is set
			
			
			The sequence control field of the MPDU
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_SEQUENCE_CONTROL_FIELD_OFFSET                             0x0000004c
#define RX_MPDU_INFO_MPDU_SEQUENCE_CONTROL_FIELD_LSB                                16
#define RX_MPDU_INFO_MPDU_SEQUENCE_CONTROL_FIELD_MSB                                31
#define RX_MPDU_INFO_MPDU_SEQUENCE_CONTROL_FIELD_MASK                               0xffff0000


/* Description		MAC_ADDR_AD4_31_0

			Field only valid when mac_addr_ad4_valid is set
			
			The Least Significant 4 bytes of the Received Frames MAC
			 Address AD4
			<legal all>
*/

#define RX_MPDU_INFO_MAC_ADDR_AD4_31_0_OFFSET                                       0x00000050
#define RX_MPDU_INFO_MAC_ADDR_AD4_31_0_LSB                                          0
#define RX_MPDU_INFO_MAC_ADDR_AD4_31_0_MSB                                          31
#define RX_MPDU_INFO_MAC_ADDR_AD4_31_0_MASK                                         0xffffffff


/* Description		MAC_ADDR_AD4_47_32

			Field only valid when mac_addr_ad4_valid is set
			
			The 2 most significant bytes of the Received Frames MAC 
			Address AD4
			<legal all>
*/

#define RX_MPDU_INFO_MAC_ADDR_AD4_47_32_OFFSET                                      0x00000054
#define RX_MPDU_INFO_MAC_ADDR_AD4_47_32_LSB                                         0
#define RX_MPDU_INFO_MAC_ADDR_AD4_47_32_MSB                                         15
#define RX_MPDU_INFO_MAC_ADDR_AD4_47_32_MASK                                        0x0000ffff


/* Description		MPDU_QOS_CONTROL_FIELD

			Field only valid when mpdu_qos_control_valid is set
			
			The sequence control field of the MPDU
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_QOS_CONTROL_FIELD_OFFSET                                  0x00000054
#define RX_MPDU_INFO_MPDU_QOS_CONTROL_FIELD_LSB                                     16
#define RX_MPDU_INFO_MPDU_QOS_CONTROL_FIELD_MSB                                     31
#define RX_MPDU_INFO_MPDU_QOS_CONTROL_FIELD_MASK                                    0xffff0000


/* Description		MPDU_HT_CONTROL_FIELD

			Field only valid when mpdu_qos_control_valid is set
			
			The HT control field of the MPDU
			<legal all>
*/

#define RX_MPDU_INFO_MPDU_HT_CONTROL_FIELD_OFFSET                                   0x00000058
#define RX_MPDU_INFO_MPDU_HT_CONTROL_FIELD_LSB                                      0
#define RX_MPDU_INFO_MPDU_HT_CONTROL_FIELD_MSB                                      31
#define RX_MPDU_INFO_MPDU_HT_CONTROL_FIELD_MASK                                     0xffffffff


/* Description		VDEV_ID

			Consumer: RXOLE
			Producer: FW
			
			Virtual device associated with this peer
			
			RXOLE uses this to determine intra-BSS routing.
			
			<legal all>
*/

#define RX_MPDU_INFO_VDEV_ID_OFFSET                                                 0x0000005c
#define RX_MPDU_INFO_VDEV_ID_LSB                                                    0
#define RX_MPDU_INFO_VDEV_ID_MSB                                                    7
#define RX_MPDU_INFO_VDEV_ID_MASK                                                   0x000000ff


/* Description		SERVICE_CODE

			Opaque service code between PPE and Wi-Fi
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MPDU_INFO_SERVICE_CODE_OFFSET                                            0x0000005c
#define RX_MPDU_INFO_SERVICE_CODE_LSB                                               8
#define RX_MPDU_INFO_SERVICE_CODE_MSB                                               16
#define RX_MPDU_INFO_SERVICE_CODE_MASK                                              0x0001ff00


/* Description		PRIORITY_VALID

			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MPDU_INFO_PRIORITY_VALID_OFFSET                                          0x0000005c
#define RX_MPDU_INFO_PRIORITY_VALID_LSB                                             17
#define RX_MPDU_INFO_PRIORITY_VALID_MSB                                             17
#define RX_MPDU_INFO_PRIORITY_VALID_MASK                                            0x00020000


/* Description		SRC_INFO

			Source (virtual) device/interface info. associated with 
			this peer
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MPDU_INFO_SRC_INFO_OFFSET                                                0x0000005c
#define RX_MPDU_INFO_SRC_INFO_LSB                                                   18
#define RX_MPDU_INFO_SRC_INFO_MSB                                                   29
#define RX_MPDU_INFO_SRC_INFO_MASK                                                  0x3ffc0000


/* Description		RESERVED_23A

			<legal 0>
*/

#define RX_MPDU_INFO_RESERVED_23A_OFFSET                                            0x0000005c
#define RX_MPDU_INFO_RESERVED_23A_LSB                                               30
#define RX_MPDU_INFO_RESERVED_23A_MSB                                               30
#define RX_MPDU_INFO_RESERVED_23A_MASK                                              0x40000000


/* Description		MULTI_LINK_ADDR_AD1_AD2_VALID

			If set, Rx OLE shall convert Address1 and Address2 of received
			 data frames to multi-link addresses during decapsulation
			 to Ethernet or Native WiFi
			<legal all>
*/

#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD1_AD2_VALID_OFFSET                           0x0000005c
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD1_AD2_VALID_LSB                              31
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD1_AD2_VALID_MSB                              31
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD1_AD2_VALID_MASK                             0x80000000


/* Description		MULTI_LINK_ADDR_AD1_31_0

			Field only valid if Multi_link_addr_ad1_ad2_valid is set
			
			
			Multi-link receiver address (address1), bits [31:0]
*/

#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD1_31_0_OFFSET                                0x00000060
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD1_31_0_LSB                                   0
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD1_31_0_MSB                                   31
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD1_31_0_MASK                                  0xffffffff


/* Description		MULTI_LINK_ADDR_AD1_47_32

			Field only valid if Multi_link_addr_ad1_ad2_valid is set
			
			
			Multi-link receiver address (address1), bits [47:32]
*/

#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD1_47_32_OFFSET                               0x00000064
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD1_47_32_LSB                                  0
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD1_47_32_MSB                                  15
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD1_47_32_MASK                                 0x0000ffff


/* Description		MULTI_LINK_ADDR_AD2_15_0

			Field only valid if Multi_link_addr_ad1_ad2_valid is set
			
			
			Multi-link transmitter address (address2), bits [15:0]
*/

#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD2_15_0_OFFSET                                0x00000064
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD2_15_0_LSB                                   16
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD2_15_0_MSB                                   31
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD2_15_0_MASK                                  0xffff0000


/* Description		MULTI_LINK_ADDR_AD2_47_16

			Field only valid if Multi_link_addr_ad1_ad2_valid is set
			
			
			Multi-link transmitter address (address2), bits [47:16]
*/

#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD2_47_16_OFFSET                               0x00000068
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD2_47_16_LSB                                  0
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD2_47_16_MSB                                  31
#define RX_MPDU_INFO_MULTI_LINK_ADDR_AD2_47_16_MASK                                 0xffffffff


/* Description		AUTHORIZED_TO_SEND_WDS

			If not set, RXDMA shall perform error-routing for WDS packets
			 as the sender is not authorized and might misuse WDS frame
			 format to inject packets with arbitrary DA/SA.
			<legal all>
*/

#define RX_MPDU_INFO_AUTHORIZED_TO_SEND_WDS_OFFSET                                  0x0000006c
#define RX_MPDU_INFO_AUTHORIZED_TO_SEND_WDS_LSB                                     0
#define RX_MPDU_INFO_AUTHORIZED_TO_SEND_WDS_MSB                                     0
#define RX_MPDU_INFO_AUTHORIZED_TO_SEND_WDS_MASK                                    0x00000001


/* Description		RESERVED_27A

			Bit 1: disallow_mcbc_da_in_unicast_mpdu:
			 
			If set, RX OLE shall disallow multicast/broadcast DA in 
			A-MSDU subframes in case of ToDS=0 MPDUs. This may be enabled
			 for TDLS peers.
			<legal 0-1>
*/

#define RX_MPDU_INFO_RESERVED_27A_OFFSET                                            0x0000006c
#define RX_MPDU_INFO_RESERVED_27A_LSB                                               1
#define RX_MPDU_INFO_RESERVED_27A_MSB                                               31
#define RX_MPDU_INFO_RESERVED_27A_MASK                                              0xfffffffe


/* Description		RESERVED_28A

			<legal 0>
*/

#define RX_MPDU_INFO_RESERVED_28A_OFFSET                                            0x00000070
#define RX_MPDU_INFO_RESERVED_28A_LSB                                               0
#define RX_MPDU_INFO_RESERVED_28A_MSB                                               31
#define RX_MPDU_INFO_RESERVED_28A_MASK                                              0xffffffff


/* Description		RESERVED_29A

			<legal 0>
*/

#define RX_MPDU_INFO_RESERVED_29A_OFFSET                                            0x00000074
#define RX_MPDU_INFO_RESERVED_29A_LSB                                               0
#define RX_MPDU_INFO_RESERVED_29A_MSB                                               31
#define RX_MPDU_INFO_RESERVED_29A_MASK                                              0xffffffff



#endif   // RX_MPDU_INFO
