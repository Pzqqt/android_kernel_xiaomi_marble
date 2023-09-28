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

#ifndef _TX_FES_STATUS_END_H_
#define _TX_FES_STATUS_END_H_
#if !defined(__ASSEMBLER__)
#endif

#include "phytx_abort_request_info.h"
#define NUM_OF_DWORDS_TX_FES_STATUS_END 22

#define NUM_OF_QWORDS_TX_FES_STATUS_END 11


struct tx_fes_status_end {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t prot_coex_bt_tx_while_wlan_tx                           :  1, // [0:0]
                      prot_coex_bt_tx_while_wlan_rx                           :  1, // [1:1]
                      prot_coex_wan_tx_while_wlan_tx                          :  1, // [2:2]
                      prot_coex_wan_tx_while_wlan_rx                          :  1, // [3:3]
                      prot_coex_wlan_tx_while_wlan_tx                         :  1, // [4:4]
                      prot_coex_wlan_tx_while_wlan_rx                         :  1, // [5:5]
                      coex_bt_tx_while_wlan_tx                                :  1, // [6:6]
                      coex_bt_tx_while_wlan_rx                                :  1, // [7:7]
                      coex_wan_tx_while_wlan_tx                               :  1, // [8:8]
                      coex_wan_tx_while_wlan_rx                               :  1, // [9:9]
                      coex_wlan_tx_while_wlan_tx                              :  1, // [10:10]
                      coex_wlan_tx_while_wlan_rx                              :  1, // [11:11]
                      global_data_underflow_warning                           :  1, // [12:12]
                      global_fes_transmit_result                              :  4, // [16:13]
                      cbf_bw_received_valid                                   :  1, // [17:17]
                      cbf_bw_received                                         :  3, // [20:18]
                      actual_received_ack_type                                :  4, // [24:21]
                      sta_response_count                                      :  6, // [30:25]
                      dpdtrain_done                                           :  1; // [31:31]
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint16_t reserved_after_struct16                                 :  4, // [19:16]
                      brp_info_valid                                          :  1, // [20:20]
                      reserved_1a                                             :  6, // [26:21]
                      phytx_pkt_end_info_valid                                :  1, // [27:27]
                      phytx_abort_request_info_valid                          :  1, // [28:28]
                      fes_in_11ax_trigger_response_config                     :  1, // [29:29]
                      null_delim_inserted_before_mpdus                        :  1, // [30:30]
                      only_null_delim_sent                                    :  1; // [31:31]
             uint32_t start_of_frame_timestamp_15_0                           : 16, // [15:0]
                      start_of_frame_timestamp_31_16                          : 16; // [31:16]
             uint32_t end_of_frame_timestamp_15_0                             : 16, // [15:0]
                      end_of_frame_timestamp_31_16                            : 16; // [31:16]
             uint32_t terminate_ranging_sequence                              :  1, // [0:0]
                      reserved_4a                                             :  7, // [7:1]
                      timing_status                                           :  2, // [9:8]
                      response_type                                           :  5, // [14:10]
                      r2r_end_status_to_follow                                :  1, // [15:15]
                      transmit_delay                                          : 16; // [31:16]
             uint32_t tx_group_delay                                          : 12, // [11:0]
                      reserved_5a                                             :  4, // [15:12]
                      tpc_dbg_info_cmn_15_0                                   : 16; // [31:16]
             uint32_t tpc_dbg_info_cmn_31_16                                  : 16, // [15:0]
                      tpc_dbg_info_47_32                                      : 16; // [31:16]
             uint32_t tpc_dbg_info_chn1_15_0                                  : 16, // [15:0]
                      tpc_dbg_info_chn1_31_16                                 : 16; // [31:16]
             uint32_t tpc_dbg_info_chn1_47_32                                 : 16, // [15:0]
                      tpc_dbg_info_chn1_63_48                                 : 16; // [31:16]
             uint32_t tpc_dbg_info_chn1_79_64                                 : 16, // [15:0]
                      tpc_dbg_info_chn2_15_0                                  : 16; // [31:16]
             uint32_t tpc_dbg_info_chn2_31_16                                 : 16, // [15:0]
                      tpc_dbg_info_chn2_47_32                                 : 16; // [31:16]
             uint32_t tpc_dbg_info_chn2_63_48                                 : 16, // [15:0]
                      tpc_dbg_info_chn2_79_64                                 : 16; // [31:16]
             uint32_t phytx_tx_end_sw_info_15_0                               : 16, // [15:0]
                      phytx_tx_end_sw_info_31_16                              : 16; // [31:16]
             uint32_t phytx_tx_end_sw_info_47_32                              : 16, // [15:0]
                      phytx_tx_end_sw_info_63_48                              : 16; // [31:16]
             uint32_t beamform_masked_user_bitmap_15_0                        : 16, // [15:0]
                      beamform_masked_user_bitmap_31_16                       : 16; // [31:16]
             uint32_t cbf_segment_request_mask                                :  8, // [7:0]
                      cbf_segment_sent_mask                                   :  8, // [15:8]
                      highest_achieved_data_null_ratio                        :  5, // [20:16]
                      use_alt_power_sr                                        :  1, // [21:21]
                      static_2_pwr_mode_status                                :  1, // [22:22]
                      obss_srg_opport_transmit_status                         :  1, // [23:23]
                      srp_based_transmit_status                               :  1, // [24:24]
                      obss_pd_based_transmit_status                           :  1, // [25:25]
                      beamform_masked_user_bitmap_36_32                       :  5, // [30:26]
                      pdg_mpdu_ready                                          :  1; // [31:31]
             uint32_t pdg_mpdu_count                                          : 16, // [15:0]
                      pdg_est_mpdu_tx_count                                   : 16; // [31:16]
             uint32_t pdg_overview_length                                     : 24, // [23:0]
                      txop_duration                                           :  7, // [30:24]
                      pdg_dropped_mpdu_warning                                :  1; // [31:31]
             uint32_t packet_extension_a_factor                               :  2, // [1:0]
                      packet_extension_pe_disambiguity                        :  1, // [2:2]
                      packet_extension                                        :  3, // [5:3]
                      fec_type                                                :  1, // [6:6]
                      stbc                                                    :  1, // [7:7]
                      num_data_symbols                                        : 16, // [23:8]
                      ru_size                                                 :  4, // [27:24]
                      reserved_17a                                            :  4; // [31:28]
             uint32_t num_ltf_symbols                                         :  3, // [2:0]
                      ltf_size                                                :  2, // [4:3]
                      cp_setting                                              :  2, // [6:5]
                      reserved_18a                                            :  5, // [11:7]
                      dcm                                                     :  1, // [12:12]
                      ldpc_extra_symbol                                       :  1, // [13:13]
                      force_extra_symbol                                      :  1, // [14:14]
                      reserved_18b                                            :  1, // [15:15]
                      tx_pwr_shared                                           :  8, // [23:16]
                      tx_pwr_unshared                                         :  8; // [31:24]
             uint32_t ranging_active_user_map                                 : 16, // [15:0]
                      ranging_sent_dummy_tx                                   :  1, // [16:16]
                      ranging_ftm_frame_sent                                  :  1, // [17:17]
                      reserved_20a                                            :  6, // [23:18]
                      cv_corr_status                                          :  8; // [31:24]
             uint32_t current_tx_duration                                     : 16, // [15:0]
                      reserved_21a                                            : 16; // [31:16]
#else
             uint32_t dpdtrain_done                                           :  1, // [31:31]
                      sta_response_count                                      :  6, // [30:25]
                      actual_received_ack_type                                :  4, // [24:21]
                      cbf_bw_received                                         :  3, // [20:18]
                      cbf_bw_received_valid                                   :  1, // [17:17]
                      global_fes_transmit_result                              :  4, // [16:13]
                      global_data_underflow_warning                           :  1, // [12:12]
                      coex_wlan_tx_while_wlan_rx                              :  1, // [11:11]
                      coex_wlan_tx_while_wlan_tx                              :  1, // [10:10]
                      coex_wan_tx_while_wlan_rx                               :  1, // [9:9]
                      coex_wan_tx_while_wlan_tx                               :  1, // [8:8]
                      coex_bt_tx_while_wlan_rx                                :  1, // [7:7]
                      coex_bt_tx_while_wlan_tx                                :  1, // [6:6]
                      prot_coex_wlan_tx_while_wlan_rx                         :  1, // [5:5]
                      prot_coex_wlan_tx_while_wlan_tx                         :  1, // [4:4]
                      prot_coex_wan_tx_while_wlan_rx                          :  1, // [3:3]
                      prot_coex_wan_tx_while_wlan_tx                          :  1, // [2:2]
                      prot_coex_bt_tx_while_wlan_rx                           :  1, // [1:1]
                      prot_coex_bt_tx_while_wlan_tx                           :  1; // [0:0]
             uint32_t only_null_delim_sent                                    :  1, // [31:31]
                      null_delim_inserted_before_mpdus                        :  1, // [30:30]
                      fes_in_11ax_trigger_response_config                     :  1, // [29:29]
                      phytx_abort_request_info_valid                          :  1, // [28:28]
                      phytx_pkt_end_info_valid                                :  1, // [27:27]
                      reserved_1a                                             :  6, // [26:21]
                      brp_info_valid                                          :  1, // [20:20]
                      reserved_after_struct16                                 :  4; // [19:16]
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint32_t start_of_frame_timestamp_31_16                          : 16, // [31:16]
                      start_of_frame_timestamp_15_0                           : 16; // [15:0]
             uint32_t end_of_frame_timestamp_31_16                            : 16, // [31:16]
                      end_of_frame_timestamp_15_0                             : 16; // [15:0]
             uint32_t transmit_delay                                          : 16, // [31:16]
                      r2r_end_status_to_follow                                :  1, // [15:15]
                      response_type                                           :  5, // [14:10]
                      timing_status                                           :  2, // [9:8]
                      reserved_4a                                             :  7, // [7:1]
                      terminate_ranging_sequence                              :  1; // [0:0]
             uint32_t tpc_dbg_info_cmn_15_0                                   : 16, // [31:16]
                      reserved_5a                                             :  4, // [15:12]
                      tx_group_delay                                          : 12; // [11:0]
             uint32_t tpc_dbg_info_47_32                                      : 16, // [31:16]
                      tpc_dbg_info_cmn_31_16                                  : 16; // [15:0]
             uint32_t tpc_dbg_info_chn1_31_16                                 : 16, // [31:16]
                      tpc_dbg_info_chn1_15_0                                  : 16; // [15:0]
             uint32_t tpc_dbg_info_chn1_63_48                                 : 16, // [31:16]
                      tpc_dbg_info_chn1_47_32                                 : 16; // [15:0]
             uint32_t tpc_dbg_info_chn2_15_0                                  : 16, // [31:16]
                      tpc_dbg_info_chn1_79_64                                 : 16; // [15:0]
             uint32_t tpc_dbg_info_chn2_47_32                                 : 16, // [31:16]
                      tpc_dbg_info_chn2_31_16                                 : 16; // [15:0]
             uint32_t tpc_dbg_info_chn2_79_64                                 : 16, // [31:16]
                      tpc_dbg_info_chn2_63_48                                 : 16; // [15:0]
             uint32_t phytx_tx_end_sw_info_31_16                              : 16, // [31:16]
                      phytx_tx_end_sw_info_15_0                               : 16; // [15:0]
             uint32_t phytx_tx_end_sw_info_63_48                              : 16, // [31:16]
                      phytx_tx_end_sw_info_47_32                              : 16; // [15:0]
             uint32_t beamform_masked_user_bitmap_31_16                       : 16, // [31:16]
                      beamform_masked_user_bitmap_15_0                        : 16; // [15:0]
             uint32_t pdg_mpdu_ready                                          :  1, // [31:31]
                      beamform_masked_user_bitmap_36_32                       :  5, // [30:26]
                      obss_pd_based_transmit_status                           :  1, // [25:25]
                      srp_based_transmit_status                               :  1, // [24:24]
                      obss_srg_opport_transmit_status                         :  1, // [23:23]
                      static_2_pwr_mode_status                                :  1, // [22:22]
                      use_alt_power_sr                                        :  1, // [21:21]
                      highest_achieved_data_null_ratio                        :  5, // [20:16]
                      cbf_segment_sent_mask                                   :  8, // [15:8]
                      cbf_segment_request_mask                                :  8; // [7:0]
             uint32_t pdg_est_mpdu_tx_count                                   : 16, // [31:16]
                      pdg_mpdu_count                                          : 16; // [15:0]
             uint32_t pdg_dropped_mpdu_warning                                :  1, // [31:31]
                      txop_duration                                           :  7, // [30:24]
                      pdg_overview_length                                     : 24; // [23:0]
             uint32_t reserved_17a                                            :  4, // [31:28]
                      ru_size                                                 :  4, // [27:24]
                      num_data_symbols                                        : 16, // [23:8]
                      stbc                                                    :  1, // [7:7]
                      fec_type                                                :  1, // [6:6]
                      packet_extension                                        :  3, // [5:3]
                      packet_extension_pe_disambiguity                        :  1, // [2:2]
                      packet_extension_a_factor                               :  2; // [1:0]
             uint32_t tx_pwr_unshared                                         :  8, // [31:24]
                      tx_pwr_shared                                           :  8, // [23:16]
                      reserved_18b                                            :  1, // [15:15]
                      force_extra_symbol                                      :  1, // [14:14]
                      ldpc_extra_symbol                                       :  1, // [13:13]
                      dcm                                                     :  1, // [12:12]
                      reserved_18a                                            :  5, // [11:7]
                      cp_setting                                              :  2, // [6:5]
                      ltf_size                                                :  2, // [4:3]
                      num_ltf_symbols                                         :  3; // [2:0]
             uint32_t cv_corr_status                                          :  8, // [31:24]
                      reserved_20a                                            :  6, // [23:18]
                      ranging_ftm_frame_sent                                  :  1, // [17:17]
                      ranging_sent_dummy_tx                                   :  1, // [16:16]
                      ranging_active_user_map                                 : 16; // [15:0]
             uint32_t reserved_21a                                            : 16, // [31:16]
                      current_tx_duration                                     : 16; // [15:0]
#endif
};


/* Description		PROT_COEX_BT_TX_WHILE_WLAN_TX

			When set, a BT tx coex event started while wlan was in the
			 middle of TX a transmission.
			
			Field set when coex_status_broadcast TLV received with bt
			 tx activity set and during WLAN tx
			<legal all>
*/

#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_TX_OFFSET                      0x0000000000000000
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_TX_LSB                         0
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_TX_MSB                         0
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_TX_MASK                        0x0000000000000001


/* Description		PROT_COEX_BT_TX_WHILE_WLAN_RX

			When set, a BT tx coex event started while wlan was in the
			 middle of TX a transmission.
			
			Field set when coex broadcast TLV received with bt tx activity
			 set and during WLAN rx
			<legal all>
*/

#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_RX_OFFSET                      0x0000000000000000
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_RX_LSB                         1
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_RX_MSB                         1
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_RX_MASK                        0x0000000000000002


/* Description		PROT_COEX_WAN_TX_WHILE_WLAN_TX

			When set, a WAN tx coex event started while wlan was in 
			the middle of TX a transmission.
			
			Field set when coex_status_broadcast TLV received with WAN
			 tx activity set and during WLAN tx
			<legal all>
*/

#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_TX_OFFSET                     0x0000000000000000
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_TX_LSB                        2
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_TX_MSB                        2
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_TX_MASK                       0x0000000000000004


/* Description		PROT_COEX_WAN_TX_WHILE_WLAN_RX

			When set, a WAN tx coex event started while wlan was in 
			the middle of TX a transmission.
			
			Field set when coex broadcast TLV received with WAN tx activity
			 set and during WLAN rx
			<legal all>
*/

#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_RX_OFFSET                     0x0000000000000000
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_RX_LSB                        3
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_RX_MSB                        3
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_RX_MASK                       0x0000000000000008


/* Description		PROT_COEX_WLAN_TX_WHILE_WLAN_TX

			When set, a WLAN tx coex event started while wlan was in
			 the middle of TX a transmission.
			
			Field set when coex_status_broadcast TLV received with WLAN
			 tx activity set and during WLAN tx
			<legal all>
*/

#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_TX_OFFSET                    0x0000000000000000
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_TX_LSB                       4
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_TX_MSB                       4
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_TX_MASK                      0x0000000000000010


/* Description		PROT_COEX_WLAN_TX_WHILE_WLAN_RX

			When set, a WLAN tx coex event started while wlan was in
			 the middle of TX a transmission.
			
			Field set when coex broadcast TLV received with WLAN tx 
			activity set and during WLAN rx
			<legal all>
*/

#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_RX_OFFSET                    0x0000000000000000
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_RX_LSB                       5
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_RX_MSB                       5
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_RX_MASK                      0x0000000000000020


/* Description		COEX_BT_TX_WHILE_WLAN_TX

			When set, a BT tx coex event started while wlan was in the
			 middle of TX a transmission.
			
			Field set when coex_status_broadcast TLV received with bt
			 tx activity set and during WLAN tx
			<legal all>
*/

#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_TX_OFFSET                           0x0000000000000000
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_TX_LSB                              6
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_TX_MSB                              6
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_TX_MASK                             0x0000000000000040


/* Description		COEX_BT_TX_WHILE_WLAN_RX

			When set, a BT tx coex event started while wlan was in the
			 middle of TX a transmission.
			
			Field set when coex broadcast TLV received with bt tx activity
			 set and during WLAN rx
			<legal all>
*/

#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_RX_OFFSET                           0x0000000000000000
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_RX_LSB                              7
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_RX_MSB                              7
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_RX_MASK                             0x0000000000000080


/* Description		COEX_WAN_TX_WHILE_WLAN_TX

			When set, a WAN tx coex event started while wlan was in 
			the middle of TX a transmission.
			
			Field set when coex_status_broadcast TLV received with WAN
			 tx activity set and during WLAN tx
			<legal all>
*/

#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_TX_OFFSET                          0x0000000000000000
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_TX_LSB                             8
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_TX_MSB                             8
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_TX_MASK                            0x0000000000000100


/* Description		COEX_WAN_TX_WHILE_WLAN_RX

			When set, a WAN tx coex event started while wlan was in 
			the middle of TX a transmission.
			
			Field set when coex broadcast TLV received with WAN tx activity
			 set and during WLAN rx
			<legal all>
*/

#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_RX_OFFSET                          0x0000000000000000
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_RX_LSB                             9
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_RX_MSB                             9
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_RX_MASK                            0x0000000000000200


/* Description		COEX_WLAN_TX_WHILE_WLAN_TX

			When set, a WLAN tx coex event started while wlan was in
			 the middle of TX a transmission.
			
			Field set when coex_status_broadcast TLV received with WLAN
			 tx activity set and during WLAN tx
			<legal all>
*/

#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_TX_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_TX_LSB                            10
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_TX_MSB                            10
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_TX_MASK                           0x0000000000000400


/* Description		COEX_WLAN_TX_WHILE_WLAN_RX

			When set, a WLAN tx coex event started while wlan was in
			 the middle of TX a transmission.
			
			Field set when coex broadcast TLV received with WLAN tx 
			activity set and during WLAN rx
			<legal all>
*/

#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_RX_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_RX_LSB                            11
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_RX_MSB                            11
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_RX_MASK                           0x0000000000000800


/* Description		GLOBAL_DATA_UNDERFLOW_WARNING

			Consumer: SCH/SW
			Producer: TXPCU
			
			When set, during transmission a data underflow occurred 
			for one or more users.<legal all>
*/

#define TX_FES_STATUS_END_GLOBAL_DATA_UNDERFLOW_WARNING_OFFSET                      0x0000000000000000
#define TX_FES_STATUS_END_GLOBAL_DATA_UNDERFLOW_WARNING_LSB                         12
#define TX_FES_STATUS_END_GLOBAL_DATA_UNDERFLOW_WARNING_MSB                         12
#define TX_FES_STATUS_END_GLOBAL_DATA_UNDERFLOW_WARNING_MASK                        0x0000000000001000


/* Description		GLOBAL_FES_TRANSMIT_RESULT

			Consumer: SCH/SW
			Producer: TXPCU
			
			Global Transmit result, not per USER transmit result
			
			Note: field "Response_type" indicates if the expected response
			 was MU related or not.
			
			<enum 0 tx_ok> Successful transmission of entire Frame exchange
			 sequence
			<enum 1 prot_resp_rx_timeout> 
			No Protection response frame received so timeout is triggered. 
			
			<enum 2 ppdu_resp_rx_timeout> No PPDU response frame received
			 so timeout is triggered. 
			<enum 3 resp_frame_crc_err> Response frame was received 
			with an invalid FCS.
			<enum 4 SU_Response_type_mismatch> Response frame is received
			 without CRC error but it's not matched with expected SU_Response_type. 
			
			<enum 5 cbf_mimo_ctrl_mismatch> Set if CBF is received without
			 any error but the Nr, Nc, BW, type or token in VHT MIMO
			 control field is not matched with expected values which
			 are specified by TX_FES_SETUP.cbf_* fields. 
			<enum 7 MU_Response_type_mismatch> Response frame is received
			 without CRC error but it's not matched with expected SU_Response_type. 
			
			<enum 8 MU_Response_mpdu_not_valid>  For this user, no MPDU
			 was received at all, or all received MPDUs had an FCS error.
			
			<enum 9 MU_UL_not_enough_user_response> An MU UL response
			 reception was expected. That response came but the threshold
			 for number of successful user receptions was not met.
			NOTE: This e-num will only be used in the TX_FES_STATUS_END
			 TLV...
			<enum 10 Transmit_data_null_ratio_not_met> transmission 
			was successful and proper responses have been received. 
			But the required ratio between useful MPDU data and null
			 delimiters was not met as specified by field : Fes_continuation_ratio_threshold. 
			The FES (and potentially the SIFS burst) shall be terminated
			 by the SCHeduler
			NOTE 1: This e-num will only be used in the TX_FES_STATUS_END
			 TLV...
			
			<enum 6 TB_ranging_resp_timeout> A TB ranging response was
			 expected for a sounding TF, but the response did not arrive
			 and timeout is triggered.
			NOTE: This e-num will only be used in the TX_FES_STATUS_END
			 TLV...
			<enum 11 tb_ranging_resp_mismatch> A TB ranging response
			 was expected for a sounding TF, but the reception did not
			 match the expected response.
			NOTE: This e-num will only be used in the TX_FES_STATUS_END
			 TLV...
			
			<legal 0-11>
*/

#define TX_FES_STATUS_END_GLOBAL_FES_TRANSMIT_RESULT_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_END_GLOBAL_FES_TRANSMIT_RESULT_LSB                            13
#define TX_FES_STATUS_END_GLOBAL_FES_TRANSMIT_RESULT_MSB                            16
#define TX_FES_STATUS_END_GLOBAL_FES_TRANSMIT_RESULT_MASK                           0x000000000001e000


/* Description		CBF_BW_RECEIVED_VALID

			Field only valid in case of SU reception.
			In MU set to 0
			
			When set, the cbf_bw_received field contains valid info
*/

#define TX_FES_STATUS_END_CBF_BW_RECEIVED_VALID_OFFSET                              0x0000000000000000
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_VALID_LSB                                 17
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_VALID_MSB                                 17
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_VALID_MASK                                0x0000000000020000


/* Description		CBF_BW_RECEIVED

			Field only valid when cbf_bw_received_valid is set.
			
			In MU set to 0
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define TX_FES_STATUS_END_CBF_BW_RECEIVED_OFFSET                                    0x0000000000000000
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_LSB                                       18
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_MSB                                       20
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_MASK                                      0x00000000001c0000


/* Description		ACTUAL_RECEIVED_ACK_TYPE

			Field only valid in case of SU reception.
			In MU set to 0
			
			
			Field indicates what type of ACK was received. Can help 
			determine if unexpected ACK Types (like 256 BA instead of
			 64 BA) is received.
			
			<enum 0  Ack_not_applicable> No ACK type response was received
			 or expected
			<enum 1  ACK_basic_received > a basic ACk frame is received
			
			<enum 2  ACK_BA_0 > An ACK embedded in BA frame is received
			
			<enum 3  ACK_BA_32_received > a 32 bit BA has been received
			
			<enum 4  ACK_BA_64_received > a 64 bit BA has been received
			
			<enum 5  ACK_BA_128_received > a 128 bit BA has been received
			
			
			<enum 6  ACK_BA_256_received > a 256 bit BA has been received
			
			<enum 8 ACK_BA_512_received> a 512-bit BA has been received
			
			<enum 9 ACK_BA_1024_received> a 1024-bit BA has been received
			
			<enum 7  ACK_BA_multiple_received > multiple BA responses
			 have been received. This field to be used in scenarios 
			where multi TID data was send or data with management frames
			 was send
			
			<legal 0-9>
*/

#define TX_FES_STATUS_END_ACTUAL_RECEIVED_ACK_TYPE_OFFSET                           0x0000000000000000
#define TX_FES_STATUS_END_ACTUAL_RECEIVED_ACK_TYPE_LSB                              21
#define TX_FES_STATUS_END_ACTUAL_RECEIVED_ACK_TYPE_MSB                              24
#define TX_FES_STATUS_END_ACTUAL_RECEIVED_ACK_TYPE_MASK                             0x0000000001e00000


/* Description		STA_RESPONSE_COUNT

			In of case of a transmission where a response from multiple
			 STAs in SIFS time is expected, this field indicates how
			 many STAs actually send a response.
			
			<legal 0-63>
*/

#define TX_FES_STATUS_END_STA_RESPONSE_COUNT_OFFSET                                 0x0000000000000000
#define TX_FES_STATUS_END_STA_RESPONSE_COUNT_LSB                                    25
#define TX_FES_STATUS_END_STA_RESPONSE_COUNT_MSB                                    30
#define TX_FES_STATUS_END_STA_RESPONSE_COUNT_MASK                                   0x000000007e000000


/* Description		DPDTRAIN_DONE

			Field only valid when PHYTX_PKT_END_info_valid is set
			
			For DPD Training packets, this bit is set to indicate that
			 DPD Training was successfully run to completion.  Also 
			reused by Implicit BF Calibration Packets. This bit is intended
			 for debug purposes.
			<legal all>
*/

#define TX_FES_STATUS_END_DPDTRAIN_DONE_OFFSET                                      0x0000000000000000
#define TX_FES_STATUS_END_DPDTRAIN_DONE_LSB                                         31
#define TX_FES_STATUS_END_DPDTRAIN_DONE_MSB                                         31
#define TX_FES_STATUS_END_DPDTRAIN_DONE_MASK                                        0x0000000080000000


/* Description		PHYTX_ABORT_REQUEST_INFO_DETAILS

			Field only valid when PHYTX_ABORT_REQUEST_info_valid is 
			set
			
			The reason why PHYTX is requested an abort
*/


/* Description		PHYTX_ABORT_REASON

			Reason for early termination of TX packet by the PHY 
			
			<enum_type PHYTX_ABORT_ENUM>
*/

#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_OFFSET 0x0000000000000000
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_LSB   32
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MSB   39
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MASK  0x000000ff00000000


/* Description		USER_NUMBER

			For some errors, the user for which this error was detected
			 can be indicated in this field.
			<legal 0-36>
*/

#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_OFFSET       0x0000000000000000
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_LSB          40
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MSB          45
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MASK         0x00003f0000000000


/* Description		RESERVED

			<legal 0>
*/

#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_OFFSET          0x0000000000000000
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_LSB             46
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MSB             47
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MASK            0x0000c00000000000


/* Description		RESERVED_AFTER_STRUCT16

			<legal 0>
*/

#define TX_FES_STATUS_END_RESERVED_AFTER_STRUCT16_OFFSET                            0x0000000000000000
#define TX_FES_STATUS_END_RESERVED_AFTER_STRUCT16_LSB                               48
#define TX_FES_STATUS_END_RESERVED_AFTER_STRUCT16_MSB                               51
#define TX_FES_STATUS_END_RESERVED_AFTER_STRUCT16_MASK                              0x000f000000000000


/* Description		BRP_INFO_VALID

			When set, TXPCU sent CBF segments.
			
			Fields cbf_segment_request_mask and cbf_segment_sent_mask
			 contain valid info.
			
			<legal all>
*/

#define TX_FES_STATUS_END_BRP_INFO_VALID_OFFSET                                     0x0000000000000000
#define TX_FES_STATUS_END_BRP_INFO_VALID_LSB                                        52
#define TX_FES_STATUS_END_BRP_INFO_VALID_MSB                                        52
#define TX_FES_STATUS_END_BRP_INFO_VALID_MASK                                       0x0010000000000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define TX_FES_STATUS_END_RESERVED_1A_OFFSET                                        0x0000000000000000
#define TX_FES_STATUS_END_RESERVED_1A_LSB                                           53
#define TX_FES_STATUS_END_RESERVED_1A_MSB                                           58
#define TX_FES_STATUS_END_RESERVED_1A_MASK                                          0x07e0000000000000


/* Description		PHYTX_PKT_END_INFO_VALID

			All the fields originating from PHYTX_PKT_END TLV contain
			 valid info
*/

#define TX_FES_STATUS_END_PHYTX_PKT_END_INFO_VALID_OFFSET                           0x0000000000000000
#define TX_FES_STATUS_END_PHYTX_PKT_END_INFO_VALID_LSB                              59
#define TX_FES_STATUS_END_PHYTX_PKT_END_INFO_VALID_MSB                              59
#define TX_FES_STATUS_END_PHYTX_PKT_END_INFO_VALID_MASK                             0x0800000000000000


/* Description		PHYTX_ABORT_REQUEST_INFO_VALID

			Field Phytx_abort_request_info_details contains valid info
			
*/

#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_VALID_OFFSET                     0x0000000000000000
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_VALID_LSB                        60
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_VALID_MSB                        60
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_VALID_MASK                       0x1000000000000000


/* Description		FES_IN_11AX_TRIGGER_RESPONSE_CONFIG

			When set, this transmission was the result of responding
			 to the reception of an 11ax trigger. This is a copy of 
			field Fes_in_11ax_Trigger_response_config in the TX_FES_SETUP
			 TLV.
			<legal all>
*/

#define TX_FES_STATUS_END_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_OFFSET                0x0000000000000000
#define TX_FES_STATUS_END_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_LSB                   61
#define TX_FES_STATUS_END_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_MSB                   61
#define TX_FES_STATUS_END_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_MASK                  0x2000000000000000


/* Description		NULL_DELIM_INSERTED_BEFORE_MPDUS

			Field only valid when "Fes_in_11ax_Trigger_response_config" 
			is set.
			
			This bit will get set if any NULL delimiter is sent out 
			to PHY, during the whole transmit duration(self_gen + FES).
			
			This bit will NOT be set, if no MPDU data is sent out to
			 PHY and whole transmit duration is filled with NULL delimiters. 
			
			
			Note that SCH does not evaluate this field. It is only for
			 SW to look at.
			
			<legal all>
*/

#define TX_FES_STATUS_END_NULL_DELIM_INSERTED_BEFORE_MPDUS_OFFSET                   0x0000000000000000
#define TX_FES_STATUS_END_NULL_DELIM_INSERTED_BEFORE_MPDUS_LSB                      62
#define TX_FES_STATUS_END_NULL_DELIM_INSERTED_BEFORE_MPDUS_MSB                      62
#define TX_FES_STATUS_END_NULL_DELIM_INSERTED_BEFORE_MPDUS_MASK                     0x4000000000000000


/* Description		ONLY_NULL_DELIM_SENT

			Field only valid when "Fes_in_11ax_Trigger_response_config" 
			is set.
			
			This bit will be set if only NULL delimiters are sent to
			 the PHY and no SCH sourced MPDU data is sent out.
			NOTE here that self-gen MPDU data will not be considered
			 while evaluating this bit. 
			
			Note that SCH does not evaluate this field. It is only for
			 SW to look at.
			
			<legal all>
*/

#define TX_FES_STATUS_END_ONLY_NULL_DELIM_SENT_OFFSET                               0x0000000000000000
#define TX_FES_STATUS_END_ONLY_NULL_DELIM_SENT_LSB                                  63
#define TX_FES_STATUS_END_ONLY_NULL_DELIM_SENT_MSB                                  63
#define TX_FES_STATUS_END_ONLY_NULL_DELIM_SENT_MASK                                 0x8000000000000000


/* Description		START_OF_FRAME_TIMESTAMP_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			bits 15:0 of a 64 bit time stamp 
			Start of frame in the medium @960 MHz
			<legal all> 
*/

#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_15_0_OFFSET                      0x0000000000000008
#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_15_0_LSB                         0
#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_15_0_MSB                         15
#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_15_0_MASK                        0x000000000000ffff


/* Description		START_OF_FRAME_TIMESTAMP_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			bits 31:16 of a 64 bit time stamp 
			Start of frame in the medium @960 MHz
			<legal all>
*/

#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_31_16_OFFSET                     0x0000000000000008
#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_31_16_LSB                        16
#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_31_16_MSB                        31
#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_31_16_MASK                       0x00000000ffff0000


/* Description		END_OF_FRAME_TIMESTAMP_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			bits 15:0 of a 64 bit time stamp 
			End of frame in the medium @960 MHz
			<legal all> 
*/

#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_15_0_OFFSET                        0x0000000000000008
#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_15_0_LSB                           32
#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_15_0_MSB                           47
#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_15_0_MASK                          0x0000ffff00000000


/* Description		END_OF_FRAME_TIMESTAMP_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			bits 31:16 of a 64 bit time stamp 
			End of frame in the medium @960 MHz
			<legal all>
*/

#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_31_16_OFFSET                       0x0000000000000008
#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_31_16_LSB                          48
#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_31_16_MSB                          63
#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_31_16_MASK                         0xffff000000000000


/* Description		TERMINATE_RANGING_SEQUENCE

			Consumer: SW/SCH
			Producer: TXPCU
			
			If set to 1, HWSCH will flush the TX pipeline and terminate
			 the ongoing SIFS sequence for TB Ranging.
			
			TXPCU to set it only in the context of TB Ranging, when 
			the condition to terminate the TB Ranging sequence is met
			
			
			<legal all>
*/

#define TX_FES_STATUS_END_TERMINATE_RANGING_SEQUENCE_OFFSET                         0x0000000000000010
#define TX_FES_STATUS_END_TERMINATE_RANGING_SEQUENCE_LSB                            0
#define TX_FES_STATUS_END_TERMINATE_RANGING_SEQUENCE_MSB                            0
#define TX_FES_STATUS_END_TERMINATE_RANGING_SEQUENCE_MASK                           0x0000000000000001


/* Description		RESERVED_4A

			<legal 0>
*/

#define TX_FES_STATUS_END_RESERVED_4A_OFFSET                                        0x0000000000000010
#define TX_FES_STATUS_END_RESERVED_4A_LSB                                           1
#define TX_FES_STATUS_END_RESERVED_4A_MSB                                           7
#define TX_FES_STATUS_END_RESERVED_4A_MASK                                          0x00000000000000fe


/* Description		TIMING_STATUS

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			<enum 0 No_tx_timing_request> The MAC did not request for
			 the transmission to start at a particular time
			<enum 1 successful_tx_timing > MAC did request for transmission
			 to start at a particular time and PHY was able to do so.
			
			<enum 2 tx_timing_not_honoured> PHY was not able to honour
			 the requested transmit time by the MAC. The transmission
			 started later, and field transmit_delay indicates how much
			 later.
			<legal 0-2>
*/

#define TX_FES_STATUS_END_TIMING_STATUS_OFFSET                                      0x0000000000000010
#define TX_FES_STATUS_END_TIMING_STATUS_LSB                                         8
#define TX_FES_STATUS_END_TIMING_STATUS_MSB                                         9
#define TX_FES_STATUS_END_TIMING_STATUS_MASK                                        0x0000000000000300


/* Description		RESPONSE_TYPE

			The response type that TXPCU was checking for
			
			<enum 0 no_response_expected>After transmission of this 
			frame, no response in SIFS time is expected
			
			When TXPCU sees this setting, it shall not generated the
			 EXPECTED_RESPONSE TLV.
			
			RXPCU should never see this setting
			<enum 1 ack_expected>An ACK frame is expected as response
			
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 2 ba_64_bitmap_expected>BA with 64 bitmap is expected.
			
			
			PDG DOES NOT use the size info to calculated response duration. 
			The length of the response will have to be programmed by
			 SW in the per-BW 'Expected_ppdu_resp_length' field.
			
			For TXPCU only the fact that it is a BA is important. Actual
			 received BA size is not important
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 3 ba_256_expected>BA with 256 bitmap is expected.
			
			PDG DOES NOT use the size info to calculated response duration. 
			The length of the response will have to be programmed by
			 SW in the per-BW 'Expected_ppdu_resp_length' field.
			
			For TXPCU only the fact that it is a BA is important. Actual
			 received BA size is not important
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 4 actionnoack_expected>SW sets this after sending 
			NDP or BR-Poll. 
			
			As PDG has no idea on how long the reception is going to
			 be, the reception time of the response will have to be 
			programmed by SW in the 'Extend_duration_value_bw...' field
			
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 5 ack_ba_expected>PDG uses the size info and assumes
			 single BA format with ACK and 64 bitmap embedded. 
			If SW expects more bitmaps in case of multi-TID, is shall
			 program the 'Extend_duration_value_bw...' field for additional
			 duration time.
			For TXPCU only the fact that an ACK and/or BA is received
			 is important. Reception of only ACK or BA is also considered
			 a success.
			SW also typically sets this when sending VHT single MPDU. 
			Some chip vendors might send BA rather than ACK in response
			 to VHT single MPDU but still we want to accept BA as well. 
			
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 6 cts_expected>SW sets this after queuing RTS frame
			 as standalone packet and sending it.
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 7 ack_data_expected>SW sets this after sending PS-Poll. 
			
			
			For TXPCU either ACK and/or data reception is considered
			 success.
			PDG basis it's response duration calculation on an ACK. 
			For the data portion, SW shall program the 'Extend_duration_value_bw...' 
			field
			<enum 8 ndp_ack_expected>Reserved for 11ah usage. 
			<enum 9 ndp_modified_ack>Reserved for 11ah usage 
			<enum 10 ndp_ba_expected>Reserved for 11ah usage. 
			<enum 11 ndp_cts_expected>Reserved for 11ah usage
			<enum 12 ndp_ack_or_ndp_modified_ack_expected>Reserved for
			 11ah usage
			
			TXPCU expects UL MU OFDMA or UL MU MIMO reception.
			As PDG does not know how RUs are assigned for the uplink
			 portion, PDG can not calculate the uplink duration. Therefor
			 SW shall program the 'Extend_duration_value_bw...' field
			
			
			RXPCU will report any frame received, irrespective of it
			 having been UL MU or SU It is TXPCUs responsibility to 
			distinguish between the UL MU or SU
			
			TXPCU can check in TLV RECEIVED_RESPONSE_INFO MU_Response_BA_bitmap
			 if indeed BA was received
			
			TXPCU expects UL MU OFDMA or UL MU MIMO reception.
			As PDG does not know how RUs are assigned for the uplink
			 portion, PDG can not calculate the uplink duration. Therefor
			 SW shall program the 'Extend_duration_value_bw...' field
			
			
			RXPCU will report any frame received, irrespective of it
			 having been UL MU or SU It is TXPCUs responsibility to 
			distinguish between the UL MU or SU
			
			TXPCU can check in TLV RECEIVED_RESPONSE_INFO, field MU_Response_data_bitmap
			 and MU_Response_BA_bitmap if indeed BA and data was received
			
			When selected, CBF frames are expected to be received in
			 MU reception (uplink OFDMA or uplink MIMO)
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received 
			TXPCU can check in TLV RECEIVED_RESPONSE_INFO, field MU_Response_cbf_bitmap
			 if indeed CBF frames were received.
			<enum 16 ul_mu_frames_expected>When selected, MPDU frames
			 are expected in the MU reception (uplink OFDMA or uplink
			 MIMO)
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received 
			
			TXPCU can check in TLV RECEIVED_RESPONSE_INFO, field MU_Response_bitmap
			 if indeed frames were received.
			<enum 17 any_response_to_this_device>Any response expected
			 to be send to this device in SIFS time is acceptable. 
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received 
			
			For TXPCU, UL MU or SU is both acceptable.
			
			Can be used for complex OFDMA scenarios. PDG can not calculate
			 the uplink duration. Therefor SW shall program the 'Extend_duration_value_bw...' 
			field
			<enum 18 any_response_accepted>Any frame in the medium to
			 this or any other device, is acceptable as response. 
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received 
			
			For TXPCU, UL MU or SU is both acceptable.
			
			Can be used for complex OFDMA scenarios. PDG can not calculate
			 the uplink duration. Therefor SW shall program the 'Extend_duration_value_bw...' 
			field
			<enum 19 frameless_phyrx_response_accepted>Any MU frameless
			 reception generated by the PHY is acceptable. 
			
			PHY indicates this type of reception explicitly in TLV PHYRX_RSSI_LEGACY, 
			field Reception_type == reception_is_frameless
			
			RXPCU will report any frame received, irrespective of it
			 having been UL MU or SU.
			
			This can be used for complex MU-MIMO or OFDMA scenarios, 
			like receiving MU-CTS.
			
			PDG can not calculate the uplink duration. Therefor SW shall
			 program the 'Extend_duration_value_bw...' field
			<enum 20 ranging_ndp_and_lmr_expected>SW sets this after
			 sending ranging NDPA followed by NDP as an ISTA and NDP
			 and LMR (Action No Ack) are expected as back-to-back reception
			 in SIFS.
			
			As PDG has no idea on how long the reception is going to
			 be, the reception time of the response will have to be 
			programmed by SW in the 'Extend_duration_value_bw...' field
			
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 21 ba_512_expected>BA with 512 bitmap is expected.
			
			
			PDG DOES NOT use the size info to calculated response duration. 
			The length of the response will have to be programmed by
			 SW in the per-BW 'Expected_ppdu_resp_length' field.
			
			For TXPCU only the fact that it is a BA is important. Actual
			 received BA size is not important
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 22 ba_1024_expected>BA with 1024 bitmap is expected.
			
			
			PDG DOES NOT use the size info to calculated response duration. 
			The length of the response will have to be programmed by
			 SW in the per-BW 'Expected_ppdu_resp_length' field.
			
			For TXPCU only the fact that it is a BA is important. Actual
			 received BA size is not important
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 23 ul_mu_ranging_cts2s_expected>When selected, CTS2S
			 frames are expected to be received in MU reception (uplink
			 OFDMA)
			
			RXPCU shall check each response for CTS2S and report to 
			TXPCU.
			
			TXPCU can check in the TLV 'RECEIVED_RESPONSE_INFO,' fields
			 'MU_Response_bitmap' and 'TB_Ranging_Resp' if indeed CTS2S
			 frames were received.
			<enum 24 ul_mu_ranging_ndp_expected>When selected, UL NDP
			 frames are expected to be received in MU reception (uplink
			 spatial multiplexing)
			
			RXPCU shall check each response for NDP and report to TXPCU.
			
			
			TXPCU can check in the TLV 'RECEIVED_RESPONSE_INFO,' fields
			 'MU_Response_bitmap' and 'TB_Ranging_Resp' if indeed NDP
			 frames were received.
			<enum 25 ul_mu_ranging_lmr_expected>When selected, LMR frames
			 are expected to be received in MU reception (uplink OFDMA
			 or uplink MIMO)
			
			RXPCU shall check each response for LMR and report to TXPCU.
			
			
			TXPCU can check in the TLV 'RECEIVED_RESPONSE_INFO,' fields
			 'MU_Response_bitmap' and 'TB_Ranging_Resp' if indeed LMR
			 frames were received.
*/

#define TX_FES_STATUS_END_RESPONSE_TYPE_OFFSET                                      0x0000000000000010
#define TX_FES_STATUS_END_RESPONSE_TYPE_LSB                                         10
#define TX_FES_STATUS_END_RESPONSE_TYPE_MSB                                         14
#define TX_FES_STATUS_END_RESPONSE_TYPE_MASK                                        0x0000000000007c00


/* Description		R2R_END_STATUS_TO_FOLLOW

			When set, TXPCU will still generate an R2R frame (typically
			 M-BA), and the 'R2R_STATUS_END' TLV.
			<legal all>
*/

#define TX_FES_STATUS_END_R2R_END_STATUS_TO_FOLLOW_OFFSET                           0x0000000000000010
#define TX_FES_STATUS_END_R2R_END_STATUS_TO_FOLLOW_LSB                              15
#define TX_FES_STATUS_END_R2R_END_STATUS_TO_FOLLOW_MSB                              15
#define TX_FES_STATUS_END_R2R_END_STATUS_TO_FOLLOW_MASK                             0x0000000000008000


/* Description		TRANSMIT_DELAY

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			The number of 480 MHz clock cycles that the transmission
			 started after the actual requested transmit start time.
			
			Value saturates at 0xFFFF
			<legal all>
*/

#define TX_FES_STATUS_END_TRANSMIT_DELAY_OFFSET                                     0x0000000000000010
#define TX_FES_STATUS_END_TRANSMIT_DELAY_LSB                                        16
#define TX_FES_STATUS_END_TRANSMIT_DELAY_MSB                                        31
#define TX_FES_STATUS_END_TRANSMIT_DELAY_MASK                                       0x00000000ffff0000


/* Description		TX_GROUP_DELAY

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Group delay on TxTD+PHYRF path for this PPDU (packet BW 
			dependent), useful for RTT
			
			Unit is 960MHz cycles.
			<legal all>
*/

#define TX_FES_STATUS_END_TX_GROUP_DELAY_OFFSET                                     0x0000000000000010
#define TX_FES_STATUS_END_TX_GROUP_DELAY_LSB                                        32
#define TX_FES_STATUS_END_TX_GROUP_DELAY_MSB                                        43
#define TX_FES_STATUS_END_TX_GROUP_DELAY_MASK                                       0x00000fff00000000


/* Description		RESERVED_5A

			Bits [14:12]: service_cbw:
			
			Field only valid when a response was received
			
			Source of the info here is the 'RECEIVED_RESPONSE_INFO' 
			TLV
			
			This field reflects the BW extracted from the Serivce Field
			 for 11ac mode of operation .
			
			This field is used in the context of Dynamic BW evaluation
			 purposes in SCH in case of SW-queued protection frame.
			
			Please refer 'BW_ENUM' e-num for the values used.
			<legal 0-5>
*/

#define TX_FES_STATUS_END_RESERVED_5A_OFFSET                                        0x0000000000000010
#define TX_FES_STATUS_END_RESERVED_5A_LSB                                           44
#define TX_FES_STATUS_END_RESERVED_5A_MSB                                           47
#define TX_FES_STATUS_END_RESERVED_5A_MASK                                          0x0000f00000000000


/* Description		TPC_DBG_INFO_CMN_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some TPC debug info that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_15_0_OFFSET                              0x0000000000000010
#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_15_0_LSB                                 48
#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_15_0_MSB                                 63
#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_15_0_MASK                                0xffff000000000000


/* Description		TPC_DBG_INFO_CMN_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some TPC debug info that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_31_16_OFFSET                             0x0000000000000018
#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_31_16_LSB                                0
#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_31_16_MSB                                15
#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_31_16_MASK                               0x000000000000ffff


/* Description		TPC_DBG_INFO_47_32

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some TPC debu info that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_47_32_OFFSET                                 0x0000000000000018
#define TX_FES_STATUS_END_TPC_DBG_INFO_47_32_LSB                                    16
#define TX_FES_STATUS_END_TPC_DBG_INFO_47_32_MSB                                    31
#define TX_FES_STATUS_END_TPC_DBG_INFO_47_32_MASK                                   0x00000000ffff0000


/* Description		TPC_DBG_INFO_CHN1_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_15_0_OFFSET                             0x0000000000000018
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_15_0_LSB                                32
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_15_0_MSB                                47
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_15_0_MASK                               0x0000ffff00000000


/* Description		TPC_DBG_INFO_CHN1_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_31_16_OFFSET                            0x0000000000000018
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_31_16_LSB                               48
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_31_16_MSB                               63
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_31_16_MASK                              0xffff000000000000


/* Description		TPC_DBG_INFO_CHN1_47_32

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_47_32_OFFSET                            0x0000000000000020
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_47_32_LSB                               0
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_47_32_MSB                               15
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_47_32_MASK                              0x000000000000ffff


/* Description		TPC_DBG_INFO_CHN1_63_48

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_63_48_OFFSET                            0x0000000000000020
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_63_48_LSB                               16
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_63_48_MSB                               31
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_63_48_MASK                              0x00000000ffff0000


/* Description		TPC_DBG_INFO_CHN1_79_64

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_79_64_OFFSET                            0x0000000000000020
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_79_64_LSB                               32
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_79_64_MSB                               47
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_79_64_MASK                              0x0000ffff00000000


/* Description		TPC_DBG_INFO_CHN2_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_15_0_OFFSET                             0x0000000000000020
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_15_0_LSB                                48
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_15_0_MSB                                63
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_15_0_MASK                               0xffff000000000000


/* Description		TPC_DBG_INFO_CHN2_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_31_16_OFFSET                            0x0000000000000028
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_31_16_LSB                               0
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_31_16_MSB                               15
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_31_16_MASK                              0x000000000000ffff


/* Description		TPC_DBG_INFO_CHN2_47_32

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_47_32_OFFSET                            0x0000000000000028
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_47_32_LSB                               16
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_47_32_MSB                               31
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_47_32_MASK                              0x00000000ffff0000


/* Description		TPC_DBG_INFO_CHN2_63_48

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_63_48_OFFSET                            0x0000000000000028
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_63_48_LSB                               32
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_63_48_MSB                               47
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_63_48_MASK                              0x0000ffff00000000


/* Description		TPC_DBG_INFO_CHN2_79_64

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_79_64_OFFSET                            0x0000000000000028
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_79_64_LSB                               48
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_79_64_MSB                               63
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_79_64_MASK                              0xffff000000000000


/* Description		PHYTX_TX_END_SW_INFO_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some PHY status data that PHY microcode can pass back to
			 MAC FW, for any future requests, e.g. any DMA download 
			time
			<legal all>
*/

#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_15_0_OFFSET                          0x0000000000000030
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_15_0_LSB                             0
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_15_0_MSB                             15
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_15_0_MASK                            0x000000000000ffff


/* Description		PHYTX_TX_END_SW_INFO_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some PHY status data that PHY microcode can pass back to
			 MAC FW, for any future requests, e.g. any DMA download 
			time
			<legal all>
*/

#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_31_16_OFFSET                         0x0000000000000030
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_31_16_LSB                            16
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_31_16_MSB                            31
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_31_16_MASK                           0x00000000ffff0000


/* Description		PHYTX_TX_END_SW_INFO_47_32

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some PHY status data that PHY microcode can pass back to
			 MAC FW, for any future requests, e.g. any DMA download 
			time
			<legal all>
*/

#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_47_32_OFFSET                         0x0000000000000030
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_47_32_LSB                            32
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_47_32_MSB                            47
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_47_32_MASK                           0x0000ffff00000000


/* Description		PHYTX_TX_END_SW_INFO_63_48

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some PHY status data that PHY microcode can pass back to
			 MAC FW, for any future requests, e.g. any DMA download 
			time
			<legal all>
*/

#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_63_48_OFFSET                         0x0000000000000030
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_63_48_LSB                            48
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_63_48_MSB                            63
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_63_48_MASK                           0xffff000000000000


/* Description		BEAMFORM_MASKED_USER_BITMAP_15_0

			Lower 16 bits of 'Beamform_masked_user_bitmap'
			
			PHY indicates in this field for which users it actually 
			did not beamform it's  transmission even though this was
			 requested
			
			Bit 0: user 0, bit 1: user 1, etc.
			
			When 0: No beamform issue for this user
			When 1: PHY could not beamform for this user, but did not
			 terminate the transmission
			
			<legal all>
*/

#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_15_0_OFFSET                   0x0000000000000038
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_15_0_LSB                      0
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_15_0_MSB                      15
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_15_0_MASK                     0x000000000000ffff


/* Description		BEAMFORM_MASKED_USER_BITMAP_31_16

			Middle 16 bits of 'Beamform_masked_user_bitmap'
			See description above.
			<legal all>
*/

#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_31_16_OFFSET                  0x0000000000000038
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_31_16_LSB                     16
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_31_16_MSB                     31
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_31_16_MASK                    0x00000000ffff0000


/* Description		CBF_SEGMENT_REQUEST_MASK

			Field only valid when brp_info_valid is set.
			
			Field equal to the 'Feedback Segment Retransmission Bitmap' 
			from the Beamform Report Poll frame OR Beamform Report Poll
			 Trigger frame
			
			Bit 0 represents segment 0
			Bit 1 represents segment 1
			Etc.
			
			1'b1: Segment is requested
			1'b0: Segment is NOT requested
			
			<legal all>
*/

#define TX_FES_STATUS_END_CBF_SEGMENT_REQUEST_MASK_OFFSET                           0x0000000000000038
#define TX_FES_STATUS_END_CBF_SEGMENT_REQUEST_MASK_LSB                              32
#define TX_FES_STATUS_END_CBF_SEGMENT_REQUEST_MASK_MSB                              39
#define TX_FES_STATUS_END_CBF_SEGMENT_REQUEST_MASK_MASK                             0x000000ff00000000


/* Description		CBF_SEGMENT_SENT_MASK

			Field only valid when brp_info_valid is set.
			
			Bit 0 represents segment 0
			Bit 1 represents segment 1
			Etc.
			
			1'b1: Segment is sent
			1'b0: Segment is not sent
			
			<legal all>
*/

#define TX_FES_STATUS_END_CBF_SEGMENT_SENT_MASK_OFFSET                              0x0000000000000038
#define TX_FES_STATUS_END_CBF_SEGMENT_SENT_MASK_LSB                                 40
#define TX_FES_STATUS_END_CBF_SEGMENT_SENT_MASK_MSB                                 47
#define TX_FES_STATUS_END_CBF_SEGMENT_SENT_MASK_MASK                                0x0000ff0000000000


/* Description		HIGHEST_ACHIEVED_DATA_NULL_RATIO

			Highest DATA:NULL ratio achieved for the current FES
			
			<enum 0 No_Data_Null_ratio_requirement> There was no Data:NULL
			 ratio established.
			<enum 1 Data_Null_ratio_16_1> Best Data:NULL ratio was 16:1. 
			
			<enum 2 Data_Null_ratio_8_1> Best Data:NULL ratio was 8:1. 
			
			<enum 3 Data_Null_ratio_4_1> Best Data:NULL ratio was 4:1. 
			
			<enum 4 Data_Null_ratio_2_1> Best Data:NULL ratio was 2:1. 
			
			<enum 5 Data_Null_ratio_1_1> Best Data:NULL ratio was 1:1. 
			
			terminate FES.
			<enum 6 Data_Null_ratio_1_2> Best Data:NULL ratio was 1:2. 
			
			<enum 7 Data_Null_ratio_1_4> Best Data:NULL ratio was 1:4. 
			
			<enum 8 Data_Null_ratio_1_8> Best Data:NULL ratio was 1:8. 
			
			<enum 9 Data_Null_ratio_1_16> Best Data:NULL ratio was 1:16. 
			
			
			<legal 0-9>
*/

#define TX_FES_STATUS_END_HIGHEST_ACHIEVED_DATA_NULL_RATIO_OFFSET                   0x0000000000000038
#define TX_FES_STATUS_END_HIGHEST_ACHIEVED_DATA_NULL_RATIO_LSB                      48
#define TX_FES_STATUS_END_HIGHEST_ACHIEVED_DATA_NULL_RATIO_MSB                      52
#define TX_FES_STATUS_END_HIGHEST_ACHIEVED_DATA_NULL_RATIO_MASK                     0x001f000000000000


/* Description		USE_ALT_POWER_SR

			0: Primary/default power1: Alternate power
			<legal all>
*/

#define TX_FES_STATUS_END_USE_ALT_POWER_SR_OFFSET                                   0x0000000000000038
#define TX_FES_STATUS_END_USE_ALT_POWER_SR_LSB                                      53
#define TX_FES_STATUS_END_USE_ALT_POWER_SR_MSB                                      53
#define TX_FES_STATUS_END_USE_ALT_POWER_SR_MASK                                     0x0020000000000000


/* Description		STATIC_2_PWR_MODE_STATUS

			0: Static 2 power mode disabled1: Static 2 power mode enabled
			
			<legal all>
*/

#define TX_FES_STATUS_END_STATIC_2_PWR_MODE_STATUS_OFFSET                           0x0000000000000038
#define TX_FES_STATUS_END_STATIC_2_PWR_MODE_STATUS_LSB                              54
#define TX_FES_STATUS_END_STATIC_2_PWR_MODE_STATUS_MSB                              54
#define TX_FES_STATUS_END_STATIC_2_PWR_MODE_STATUS_MASK                             0x0040000000000000


/* Description		OBSS_SRG_OPPORT_TRANSMIT_STATUS

			0: Transmit based on SRG OBSS_PD opportunity initiated1: 
			Transmit based on non-SRG OBSS_PD opportunity initiated
			<legal all>
*/

#define TX_FES_STATUS_END_OBSS_SRG_OPPORT_TRANSMIT_STATUS_OFFSET                    0x0000000000000038
#define TX_FES_STATUS_END_OBSS_SRG_OPPORT_TRANSMIT_STATUS_LSB                       55
#define TX_FES_STATUS_END_OBSS_SRG_OPPORT_TRANSMIT_STATUS_MSB                       55
#define TX_FES_STATUS_END_OBSS_SRG_OPPORT_TRANSMIT_STATUS_MASK                      0x0080000000000000


/* Description		SRP_BASED_TRANSMIT_STATUS

			0: non-SRP based transmit initiated1: SRP based transmit
			 initiated
			<legal all>
*/

#define TX_FES_STATUS_END_SRP_BASED_TRANSMIT_STATUS_OFFSET                          0x0000000000000038
#define TX_FES_STATUS_END_SRP_BASED_TRANSMIT_STATUS_LSB                             56
#define TX_FES_STATUS_END_SRP_BASED_TRANSMIT_STATUS_MSB                             56
#define TX_FES_STATUS_END_SRP_BASED_TRANSMIT_STATUS_MASK                            0x0100000000000000


/* Description		OBSS_PD_BASED_TRANSMIT_STATUS

			0: non-OBSS_PD based transmit initiated1: obss_pd based 
			transmit initiated
			<legal all>
*/

#define TX_FES_STATUS_END_OBSS_PD_BASED_TRANSMIT_STATUS_OFFSET                      0x0000000000000038
#define TX_FES_STATUS_END_OBSS_PD_BASED_TRANSMIT_STATUS_LSB                         57
#define TX_FES_STATUS_END_OBSS_PD_BASED_TRANSMIT_STATUS_MSB                         57
#define TX_FES_STATUS_END_OBSS_PD_BASED_TRANSMIT_STATUS_MASK                        0x0200000000000000


/* Description		BEAMFORM_MASKED_USER_BITMAP_36_32

			Upper 5 bits of 'Beamform_masked_user_bitmap'
			See description above.
			<legal all>
*/

#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_36_32_OFFSET                  0x0000000000000038
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_36_32_LSB                     58
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_36_32_MSB                     62
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_36_32_MASK                    0x7c00000000000000


/* Description		PDG_MPDU_READY

			Field only valid in case of SU transmissions, copied over
			 by TXPCU from 'PCU_PPDU_SETUP_END'
			
			Indicates the 'MPDU_INFO' or 'MPDU_QUEUE_OVERVIEW' ready
			 status in PDG.
			<legal all>
*/

#define TX_FES_STATUS_END_PDG_MPDU_READY_OFFSET                                     0x0000000000000038
#define TX_FES_STATUS_END_PDG_MPDU_READY_LSB                                        63
#define TX_FES_STATUS_END_PDG_MPDU_READY_MSB                                        63
#define TX_FES_STATUS_END_PDG_MPDU_READY_MASK                                       0x8000000000000000


/* Description		PDG_MPDU_COUNT

			Field only valid in case of SU transmissions when pdg_MPDU_ready
			 is set, copied over by TXPCU from 'PCU_PPDU_SETUP_END'
			
			Total MPDU count from 'MPDU_INFO' or 'MPDU_QUEUE_OVERVIEW'
			
			<legal 0-2130>
*/

#define TX_FES_STATUS_END_PDG_MPDU_COUNT_OFFSET                                     0x0000000000000040
#define TX_FES_STATUS_END_PDG_MPDU_COUNT_LSB                                        0
#define TX_FES_STATUS_END_PDG_MPDU_COUNT_MSB                                        15
#define TX_FES_STATUS_END_PDG_MPDU_COUNT_MASK                                       0x000000000000ffff


/* Description		PDG_EST_MPDU_TX_COUNT

			Field only valid in case of SU transmissions when pdg_MPDU_ready
			 is set, copied over by TXPCU from 'PCU_PPDU_SETUP_END'
			
			PDG estimated MPDU Tx count from 'MPDU_INFO' or 'MPDU_QUEUE_OVERVIEW' 
			limited by timing boundaries (HWSCH, COEX, SR, etc.)
			<legal 0-1024>
*/

#define TX_FES_STATUS_END_PDG_EST_MPDU_TX_COUNT_OFFSET                              0x0000000000000040
#define TX_FES_STATUS_END_PDG_EST_MPDU_TX_COUNT_LSB                                 16
#define TX_FES_STATUS_END_PDG_EST_MPDU_TX_COUNT_MSB                                 31
#define TX_FES_STATUS_END_PDG_EST_MPDU_TX_COUNT_MASK                                0x00000000ffff0000


/* Description		PDG_OVERVIEW_LENGTH

			Field only valid in case of SU transmissions when pdg_MPDU_ready
			 is set, copied over by TXPCU from 'PCU_PPDU_SETUP_END'
			
			PDG estimated A-MPDU length from 'MPDU_QUEUE_OVERVIEW' limited
			 by timing boundaries (HWSCH, COEX, SR, etc.)
			<legal all>
*/

#define TX_FES_STATUS_END_PDG_OVERVIEW_LENGTH_OFFSET                                0x0000000000000040
#define TX_FES_STATUS_END_PDG_OVERVIEW_LENGTH_LSB                                   32
#define TX_FES_STATUS_END_PDG_OVERVIEW_LENGTH_MSB                                   55
#define TX_FES_STATUS_END_PDG_OVERVIEW_LENGTH_MASK                                  0x00ffffff00000000


/* Description		TXOP_DURATION

			TXOP_DURATION of HE-SIG-A calculated by PDG, to be copied
			 from 'PCU_PPDU_SETUP_END' by TXPCU
*/

#define TX_FES_STATUS_END_TXOP_DURATION_OFFSET                                      0x0000000000000040
#define TX_FES_STATUS_END_TXOP_DURATION_LSB                                         56
#define TX_FES_STATUS_END_TXOP_DURATION_MSB                                         62
#define TX_FES_STATUS_END_TXOP_DURATION_MASK                                        0x7f00000000000000


/* Description		PDG_DROPPED_MPDU_WARNING

			Warning that PDG has dropped MPDUs due to SFM FIFO full 
			condition, to be copied from 'PCU_PPDU_SETUP_END' by TXPCU
			
*/

#define TX_FES_STATUS_END_PDG_DROPPED_MPDU_WARNING_OFFSET                           0x0000000000000040
#define TX_FES_STATUS_END_PDG_DROPPED_MPDU_WARNING_LSB                              63
#define TX_FES_STATUS_END_PDG_DROPPED_MPDU_WARNING_MSB                              63
#define TX_FES_STATUS_END_PDG_DROPPED_MPDU_WARNING_MASK                             0x8000000000000000


/* Description		PACKET_EXTENSION_A_FACTOR

			The "a-factor" of the trigger-based PPDU response, to be
			 copied from 'PCU_PPDU_SETUP_END' by TXPCU
			
			This affects the packet extension duration.
			
			<enum 0 a_factor_4>
			<enum 1 a_factor_1>
			<enum 2 a_factor_2>
			<enum 3 a_factor_3>
			
			<legal all>
*/

#define TX_FES_STATUS_END_PACKET_EXTENSION_A_FACTOR_OFFSET                          0x0000000000000048
#define TX_FES_STATUS_END_PACKET_EXTENSION_A_FACTOR_LSB                             0
#define TX_FES_STATUS_END_PACKET_EXTENSION_A_FACTOR_MSB                             1
#define TX_FES_STATUS_END_PACKET_EXTENSION_A_FACTOR_MASK                            0x0000000000000003


/* Description		PACKET_EXTENSION_PE_DISAMBIGUITY

			The "PE-Disambiguity" of the trigger-based PPDU response, 
			to be copied from 'PCU_PPDU_SETUP_END' by TXPCU
			
			This affects the packet extension duration.
			
			<legal all>
*/

#define TX_FES_STATUS_END_PACKET_EXTENSION_PE_DISAMBIGUITY_OFFSET                   0x0000000000000048
#define TX_FES_STATUS_END_PACKET_EXTENSION_PE_DISAMBIGUITY_LSB                      2
#define TX_FES_STATUS_END_PACKET_EXTENSION_PE_DISAMBIGUITY_MSB                      2
#define TX_FES_STATUS_END_PACKET_EXTENSION_PE_DISAMBIGUITY_MASK                     0x0000000000000004


/* Description		PACKET_EXTENSION

			Packet extension size, to be copied from 'PCU_PPDU_SETUP_END' 
			by TXPCU
			
			This is valid for all PPDUs including HE-Ranging NDPs (11az) 
			and Short-NDPs.
			
			<enum 0     packet_ext_0> 
			<enum 1     packet_ext_4>
			<enum 2     packet_ext_8>
			<enum 3     packet_ext_12>
			<enum 4     packet_ext_16>
			<enum 5     packet_ext_20>
			<legal 0 - 5>
*/

#define TX_FES_STATUS_END_PACKET_EXTENSION_OFFSET                                   0x0000000000000048
#define TX_FES_STATUS_END_PACKET_EXTENSION_LSB                                      3
#define TX_FES_STATUS_END_PACKET_EXTENSION_MSB                                      5
#define TX_FES_STATUS_END_PACKET_EXTENSION_MASK                                     0x0000000000000038


/* Description		FEC_TYPE

			For trigger-based PPDU response, to be copied from 'PCU_PPDU_SETUP_END' 
			by TXPCU
			0: BCC
			1: LDPC
			<legal all>
*/

#define TX_FES_STATUS_END_FEC_TYPE_OFFSET                                           0x0000000000000048
#define TX_FES_STATUS_END_FEC_TYPE_LSB                                              6
#define TX_FES_STATUS_END_FEC_TYPE_MSB                                              6
#define TX_FES_STATUS_END_FEC_TYPE_MASK                                             0x0000000000000040


/* Description		STBC

			For trigger-based PPDU response, to be copied from 'PCU_PPDU_SETUP_END' 
			by TXPCU
			
			When set, this transmission is based on STBC rates.
*/

#define TX_FES_STATUS_END_STBC_OFFSET                                               0x0000000000000048
#define TX_FES_STATUS_END_STBC_LSB                                                  7
#define TX_FES_STATUS_END_STBC_MSB                                                  7
#define TX_FES_STATUS_END_STBC_MASK                                                 0x0000000000000080


/* Description		NUM_DATA_SYMBOLS

			The number of data symbols in the transmission, to be copied
			 from 'PCU_PPDU_SETUP_END' by TXPCU
			
			This does not include PE_LTF. Also for STBC packets this
			 has to be an even number. This is valid for all PPDUs.
*/

#define TX_FES_STATUS_END_NUM_DATA_SYMBOLS_OFFSET                                   0x0000000000000048
#define TX_FES_STATUS_END_NUM_DATA_SYMBOLS_LSB                                      8
#define TX_FES_STATUS_END_NUM_DATA_SYMBOLS_MSB                                      23
#define TX_FES_STATUS_END_NUM_DATA_SYMBOLS_MASK                                     0x0000000000ffff00


/* Description		RU_SIZE

			The size of the RU for this user, for trigger-based PPDU
			 response, to be copied from 'PCU_PPDU_SETUP_END' by TXPCU
			
			
			<enum 0 RU_26>
			<enum 1 RU_52>
			<enum 2 RU_106>
			<enum 3 RU_242>
			<enum 4 RU_484>
			<enum 5 RU_996>
			<enum 6 RU_1992>
			<enum 7 RU_FULLBW> Set when the RU occupies the full packet
			 bandwidth
			<enum 8 RU_FULLBW_240> Set when the RU occupies the full
			 packet bandwidth
			<enum 9 RU_FULLBW_320> Set when the RU occupies the full
			 packet bandwidth
			<enum 10 RU_MULTI_LARGE> HW will use per-user sub-band-mask
			 to infer the actual RU-size for Multi-large-RU/SU-Puncturing
			
			<enum 11 RU_78> multi small RU
			<enum 12 RU_132> multi small RU
			<legal 0-12>
*/

#define TX_FES_STATUS_END_RU_SIZE_OFFSET                                            0x0000000000000048
#define TX_FES_STATUS_END_RU_SIZE_LSB                                               24
#define TX_FES_STATUS_END_RU_SIZE_MSB                                               27
#define TX_FES_STATUS_END_RU_SIZE_MASK                                              0x000000000f000000



#define TX_FES_STATUS_END_RESERVED_17A_OFFSET                                       0x0000000000000048
#define TX_FES_STATUS_END_RESERVED_17A_LSB                                          28
#define TX_FES_STATUS_END_RESERVED_17A_MSB                                          31
#define TX_FES_STATUS_END_RESERVED_17A_MASK                                         0x00000000f0000000


/* Description		NUM_LTF_SYMBOLS

			Indicates the number of HE-LTF symbols, for trigger-based
			 PPDU response, to be copied from 'PCU_PPDU_SETUP_END' by
			 TXPCU
			
			0: 1 symbol
			1: 2 symbols
			2: 3 symbols
			3: 4 symbols
			4: 5 symbols
			5: 6 symbols
			6: 7 symbols
			7: 8 symbols
			
			NOTE that this encoding is different from what is in "Num_LTF_symbols" 
			in the HE_SIG_A_MU_DL.
			<legal all>
*/

#define TX_FES_STATUS_END_NUM_LTF_SYMBOLS_OFFSET                                    0x0000000000000048
#define TX_FES_STATUS_END_NUM_LTF_SYMBOLS_LSB                                       32
#define TX_FES_STATUS_END_NUM_LTF_SYMBOLS_MSB                                       34
#define TX_FES_STATUS_END_NUM_LTF_SYMBOLS_MASK                                      0x0000000700000000


/* Description		LTF_SIZE

			Ltf size, to be copied from 'PCU_PPDU_SETUP_END' by TXPCU
			
			
			This is valid for all PPDUs including HE-Ranging NDPs (11az) 
			and Short-NDPs.
			
			<enum 0     ltf_1x > 
			<enum 1     ltf_2x > 
			<enum 2     ltf_4x > 
			<legal 0 - 2>
*/

#define TX_FES_STATUS_END_LTF_SIZE_OFFSET                                           0x0000000000000048
#define TX_FES_STATUS_END_LTF_SIZE_LSB                                              35
#define TX_FES_STATUS_END_LTF_SIZE_MSB                                              36
#define TX_FES_STATUS_END_LTF_SIZE_MASK                                             0x0000001800000000


/* Description		CP_SETTING

			Field only valid when pkt type is HT, VHT or HE
			
			GI setting, to be copied from 'PCU_PPDU_SETUP_END' by TXPCU
			
			
			This is valid for all PPDUs including HE-Ranging NDPs (11az) 
			and Short-NDPs.
			
			<enum 0     gi_0_8_us > Legacy normal GI
			<enum 1     gi_0_4_us > Legacy short GI
			<enum 2     gi_1_6_us > HE related GI
			<enum 3     gi_3_2_us > HE related GI
			<legal 0 - 3>
*/

#define TX_FES_STATUS_END_CP_SETTING_OFFSET                                         0x0000000000000048
#define TX_FES_STATUS_END_CP_SETTING_LSB                                            37
#define TX_FES_STATUS_END_CP_SETTING_MSB                                            38
#define TX_FES_STATUS_END_CP_SETTING_MASK                                           0x0000006000000000



#define TX_FES_STATUS_END_RESERVED_18A_OFFSET                                       0x0000000000000048
#define TX_FES_STATUS_END_RESERVED_18A_LSB                                          39
#define TX_FES_STATUS_END_RESERVED_18A_MSB                                          43
#define TX_FES_STATUS_END_RESERVED_18A_MASK                                         0x00000f8000000000


/* Description		DCM

			Field only valid in case of 11ax transmission
			
			Indicates whether dual sub-carrier modulation is applied, 
			for trigger-based PPDU response, to be copied from 'PCU_PPDU_SETUP_END' 
			by TXPCU
			0: No DCM
			1:DCM
			<legal all>
*/

#define TX_FES_STATUS_END_DCM_OFFSET                                                0x0000000000000048
#define TX_FES_STATUS_END_DCM_LSB                                                   44
#define TX_FES_STATUS_END_DCM_MSB                                                   44
#define TX_FES_STATUS_END_DCM_MASK                                                  0x0000100000000000


/* Description		LDPC_EXTRA_SYMBOL

			Set to 1 if the LDPC PPDU encoding process (if an SU PPDU), 
			or at least one LDPC user's PPDU encoding process (if an
			 MU PPDU), results in an extra OFDM symbol (or symbols) 
			as described in 22.3.10.5.4 (LDPC coding) and 22.3.10.5.5
			 (Encoding process for MU PPDUs). Set to 0 otherwise.
			
			To be copied from 'PCU_PPDU_SETUP_END' by TXPCU
			<legal all>
*/

#define TX_FES_STATUS_END_LDPC_EXTRA_SYMBOL_OFFSET                                  0x0000000000000048
#define TX_FES_STATUS_END_LDPC_EXTRA_SYMBOL_LSB                                     45
#define TX_FES_STATUS_END_LDPC_EXTRA_SYMBOL_MSB                                     45
#define TX_FES_STATUS_END_LDPC_EXTRA_SYMBOL_MASK                                    0x0000200000000000


/* Description		FORCE_EXTRA_SYMBOL

			Set to 1 to force an extra OFDM symbol (or symbols) even
			 if none of the users' PPDU encoding process resuls in an
			 extra OFDM symbol (or symbols).
			
			To be copied from 'PCU_PPDU_SETUP_END' by TXPCU
			<legal all>
*/

#define TX_FES_STATUS_END_FORCE_EXTRA_SYMBOL_OFFSET                                 0x0000000000000048
#define TX_FES_STATUS_END_FORCE_EXTRA_SYMBOL_LSB                                    46
#define TX_FES_STATUS_END_FORCE_EXTRA_SYMBOL_MSB                                    46
#define TX_FES_STATUS_END_FORCE_EXTRA_SYMBOL_MASK                                   0x0000400000000000


/* Description		RESERVED_18B

			<legal 0>
*/

#define TX_FES_STATUS_END_RESERVED_18B_OFFSET                                       0x0000000000000048
#define TX_FES_STATUS_END_RESERVED_18B_LSB                                          47
#define TX_FES_STATUS_END_RESERVED_18B_MSB                                          47
#define TX_FES_STATUS_END_RESERVED_18B_MASK                                         0x0000800000000000


/* Description		TX_PWR_SHARED

			Transmit Power (signed value) in units of 0.25 dBm, to be
			 copied from 'PCU_PPDU_SETUP_END' by TXPCU
			<legal all>
*/

#define TX_FES_STATUS_END_TX_PWR_SHARED_OFFSET                                      0x0000000000000048
#define TX_FES_STATUS_END_TX_PWR_SHARED_LSB                                         48
#define TX_FES_STATUS_END_TX_PWR_SHARED_MSB                                         55
#define TX_FES_STATUS_END_TX_PWR_SHARED_MASK                                        0x00ff000000000000


/* Description		TX_PWR_UNSHARED

			Transmit Power (signed value) in units of 0.25 dBm, to be
			 copied from 'PCU_PPDU_SETUP_END' by TXPCU
			<legal all>
*/

#define TX_FES_STATUS_END_TX_PWR_UNSHARED_OFFSET                                    0x0000000000000048
#define TX_FES_STATUS_END_TX_PWR_UNSHARED_LSB                                       56
#define TX_FES_STATUS_END_TX_PWR_UNSHARED_MSB                                       63
#define TX_FES_STATUS_END_TX_PWR_UNSHARED_MASK                                      0xff00000000000000


/* Description		RANGING_ACTIVE_USER_MAP

			Field only valid for TB Ranging transmissions
			
			TXPCU sets this to the current active user bitmap, with 
			each bit set to:
			1: for an active user, and
			0: for any user not part of the ranging.
			
*/

#define TX_FES_STATUS_END_RANGING_ACTIVE_USER_MAP_OFFSET                            0x0000000000000050
#define TX_FES_STATUS_END_RANGING_ACTIVE_USER_MAP_LSB                               0
#define TX_FES_STATUS_END_RANGING_ACTIVE_USER_MAP_MSB                               15
#define TX_FES_STATUS_END_RANGING_ACTIVE_USER_MAP_MASK                              0x000000000000ffff


/* Description		RANGING_SENT_DUMMY_TX

			Field only valid for TB Ranging transmissions
			
			TXPCU sets this bit if some user's 'STA Info' or 'User Info' 
			was sent out as dummy, or the whole transmission was sent
			 out as dummy.
*/

#define TX_FES_STATUS_END_RANGING_SENT_DUMMY_TX_OFFSET                              0x0000000000000050
#define TX_FES_STATUS_END_RANGING_SENT_DUMMY_TX_LSB                                 16
#define TX_FES_STATUS_END_RANGING_SENT_DUMMY_TX_MSB                                 16
#define TX_FES_STATUS_END_RANGING_SENT_DUMMY_TX_MASK                                0x0000000000010000


/* Description		RANGING_FTM_FRAME_SENT

			Field only valid for Ranging transmissions
			
			TXPCU sets this bit if an FTM frame aggregated with an LMR
			 was sent.
*/

#define TX_FES_STATUS_END_RANGING_FTM_FRAME_SENT_OFFSET                             0x0000000000000050
#define TX_FES_STATUS_END_RANGING_FTM_FRAME_SENT_LSB                                17
#define TX_FES_STATUS_END_RANGING_FTM_FRAME_SENT_MSB                                17
#define TX_FES_STATUS_END_RANGING_FTM_FRAME_SENT_MASK                               0x0000000000020000


/* Description		RESERVED_20A

			<legal 0>
*/

#define TX_FES_STATUS_END_RESERVED_20A_OFFSET                                       0x0000000000000050
#define TX_FES_STATUS_END_RESERVED_20A_LSB                                          18
#define TX_FES_STATUS_END_RESERVED_20A_MSB                                          23
#define TX_FES_STATUS_END_RESERVED_20A_MASK                                         0x0000000000fc0000


/* Description		CV_CORR_STATUS

			CV correlation status from 'PHYTX_CV_CORR_STATUS,' to be
			 copied from 'PCU_PPDU_SETUP_END' by TXPCU
			<legal all>
*/

#define TX_FES_STATUS_END_CV_CORR_STATUS_OFFSET                                     0x0000000000000050
#define TX_FES_STATUS_END_CV_CORR_STATUS_LSB                                        24
#define TX_FES_STATUS_END_CV_CORR_STATUS_MSB                                        31
#define TX_FES_STATUS_END_CV_CORR_STATUS_MASK                                       0x00000000ff000000


/* Description		CURRENT_TX_DURATION

			The duration of the transmission in us, copied over from
			 PCU_PPDU_SETUP_{END, START} as  the case may be
			<legal all>
*/

#define TX_FES_STATUS_END_CURRENT_TX_DURATION_OFFSET                                0x0000000000000050
#define TX_FES_STATUS_END_CURRENT_TX_DURATION_LSB                                   32
#define TX_FES_STATUS_END_CURRENT_TX_DURATION_MSB                                   47
#define TX_FES_STATUS_END_CURRENT_TX_DURATION_MASK                                  0x0000ffff00000000


/* Description		RESERVED_21A

			Bits [19:16]: num_cts2self_transmitted:
			
			Number of CTS2SELF frames transmitted in this FES
			
			<legal 0-15>
*/

#define TX_FES_STATUS_END_RESERVED_21A_OFFSET                                       0x0000000000000050
#define TX_FES_STATUS_END_RESERVED_21A_LSB                                          48
#define TX_FES_STATUS_END_RESERVED_21A_MSB                                          63
#define TX_FES_STATUS_END_RESERVED_21A_MASK                                         0xffff000000000000



#endif   // TX_FES_STATUS_END
