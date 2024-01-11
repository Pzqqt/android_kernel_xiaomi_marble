/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "phytx_abort_request_info.h"
#define NUM_OF_DWORDS_TX_FES_STATUS_END 11

struct tx_fes_status_end {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t prot_coex_bt_tx_while_wlan_tx                           :  1,
                      prot_coex_bt_tx_while_wlan_rx                           :  1,
                      prot_coex_wan_tx_while_wlan_tx                          :  1,
                      prot_coex_wan_tx_while_wlan_rx                          :  1,
                      prot_coex_wlan_tx_while_wlan_tx                         :  1,
                      prot_coex_wlan_tx_while_wlan_rx                         :  1,
                      coex_bt_tx_while_wlan_tx                                :  1,
                      coex_bt_tx_while_wlan_rx                                :  1,
                      coex_wan_tx_while_wlan_tx                               :  1,
                      coex_wan_tx_while_wlan_rx                               :  1,
                      coex_wlan_tx_while_wlan_tx                              :  1,
                      coex_wlan_tx_while_wlan_rx                              :  1,
                      global_data_underflow_warning                           :  1,
                      global_fes_transmit_result                              :  4,
                      cbf_bw_received_valid                                   :  1,
                      cbf_bw_received                                         :  3,
                      actual_received_ack_type                                :  4,
                      sta_response_count                                      :  6,
                      more_data_received                                      :  1;
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint16_t reserved_after_struct16                                 :  4,
                      brp_info_valid                                          :  1,
                      qos_null_switch_done_for_eosp                           :  1,
                      reserved_1a                                             :  5,
                      phytx_pkt_end_info_valid                                :  1,
                      phytx_abort_request_info_valid                          :  1,
                      fes_in_11ax_trigger_response_config                     :  1,
                      null_delim_inserted_before_mpdus                        :  1,
                      only_null_delim_sent                                    :  1;
             uint32_t terminate___reserved_g_0005_sequence                              :  1,
                      reserved_2b                                             :  5,
                      response_type                                           :  5,
                      r2r_end_status_to_follow                                :  1,
                      reserved_5a                                             :  3,
                      prot_coex_lte_tx_while_wlan_tx                          :  1,
                      prot_coex_lte_tx_while_wlan_rx                          :  1,
                      reserved_2c                                             : 15;
             uint32_t beamform_masked_user_bitmap_15_0                        : 16,
                      beamform_masked_user_bitmap_31_16                       : 16;
             uint32_t cbf_segment_request_mask                                :  8,
                      cbf_segment_sent_mask                                   :  8,
                      highest_achieved_data_null_ratio                        :  5,
                      use_alt_power_sr                                        :  1,
                      static_2_pwr_mode_status                                :  1,
                      obss_srg_opport_transmit_status                         :  1,
                      srp_based_transmit_status                               :  1,
                      obss_pd_based_transmit_status                           :  1,
                      beamform_masked_user_bitmap_36_32                       :  5,
                      pdg_mpdu_ready                                          :  1;
             uint32_t pdg_mpdu_count                                          : 16,
                      pdg_est_mpdu_tx_count                                   : 16;
             uint32_t pdg_overview_length                                     : 24,
                      txop_duration                                           :  7,
                      pdg_dropped_mpdu_warning                                :  1;
             uint32_t packet_extension_a_factor                               :  2,
                      packet_extension_pe_disambiguity                        :  1,
                      packet_extension                                        :  3,
                      fec_type                                                :  1,
                      stbc                                                    :  1,
                      num_data_symbols                                        : 16,
                      ru_size                                                 :  4,
                      reserved_17a                                            :  4;
             uint32_t num_ltf_symbols                                         :  3,
                      ltf_size                                                :  2,
                      cp_setting                                              :  2,
                      reserved_18a                                            :  5,
                      dcm                                                     :  1,
                      ldpc_extra_symbol                                       :  1,
                      force_extra_symbol                                      :  1,
                      reserved_18b                                            :  1,
                      tx_pwr_shared                                           :  8,
                      tx_pwr_unshared                                         :  8;
             uint32_t __reserved_g_0005_active_user_map                                 : 16,
                      __reserved_g_0005_sent_dummy_tx                                   :  1,
                      __reserved_g_0005_ftm_frame_sent                                  :  1,
                      coex_uwb_tx_while_wlan_tx                               :  1,
                      coex_uwb_tx_while_wlan_rx                               :  1,
                      prot_coex_uwb_tx_while_wlan_tx                          :  1,
                      prot_coex_uwb_tx_while_wlan_rx                          :  1,
                      coex_lte_tx_while_wlan_tx                               :  1,
                      coex_lte_tx_while_wlan_rx                               :  1,
                      cv_corr_status                                          :  8;
             uint32_t current_tx_duration                                     : 16,
                      reserved_21a                                            :  4,
                      hw_qos_null_bitmap                                      :  8,
                      hw_qos_null_setup_missing                               :  1,
                      reserved_21b                                            :  3;
#else
             uint32_t more_data_received                                      :  1,
                      sta_response_count                                      :  6,
                      actual_received_ack_type                                :  4,
                      cbf_bw_received                                         :  3,
                      cbf_bw_received_valid                                   :  1,
                      global_fes_transmit_result                              :  4,
                      global_data_underflow_warning                           :  1,
                      coex_wlan_tx_while_wlan_rx                              :  1,
                      coex_wlan_tx_while_wlan_tx                              :  1,
                      coex_wan_tx_while_wlan_rx                               :  1,
                      coex_wan_tx_while_wlan_tx                               :  1,
                      coex_bt_tx_while_wlan_rx                                :  1,
                      coex_bt_tx_while_wlan_tx                                :  1,
                      prot_coex_wlan_tx_while_wlan_rx                         :  1,
                      prot_coex_wlan_tx_while_wlan_tx                         :  1,
                      prot_coex_wan_tx_while_wlan_rx                          :  1,
                      prot_coex_wan_tx_while_wlan_tx                          :  1,
                      prot_coex_bt_tx_while_wlan_rx                           :  1,
                      prot_coex_bt_tx_while_wlan_tx                           :  1;
             uint32_t only_null_delim_sent                                    :  1,
                      null_delim_inserted_before_mpdus                        :  1,
                      fes_in_11ax_trigger_response_config                     :  1,
                      phytx_abort_request_info_valid                          :  1,
                      phytx_pkt_end_info_valid                                :  1,
                      reserved_1a                                             :  5,
                      qos_null_switch_done_for_eosp                           :  1,
                      brp_info_valid                                          :  1,
                      reserved_after_struct16                                 :  4;
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint32_t reserved_2c                                             : 15,
                      prot_coex_lte_tx_while_wlan_rx                          :  1,
                      prot_coex_lte_tx_while_wlan_tx                          :  1,
                      reserved_5a                                             :  3,
                      r2r_end_status_to_follow                                :  1,
                      response_type                                           :  5,
                      reserved_2b                                             :  5,
                      terminate___reserved_g_0005_sequence                              :  1;
             uint32_t beamform_masked_user_bitmap_31_16                       : 16,
                      beamform_masked_user_bitmap_15_0                        : 16;
             uint32_t pdg_mpdu_ready                                          :  1,
                      beamform_masked_user_bitmap_36_32                       :  5,
                      obss_pd_based_transmit_status                           :  1,
                      srp_based_transmit_status                               :  1,
                      obss_srg_opport_transmit_status                         :  1,
                      static_2_pwr_mode_status                                :  1,
                      use_alt_power_sr                                        :  1,
                      highest_achieved_data_null_ratio                        :  5,
                      cbf_segment_sent_mask                                   :  8,
                      cbf_segment_request_mask                                :  8;
             uint32_t pdg_est_mpdu_tx_count                                   : 16,
                      pdg_mpdu_count                                          : 16;
             uint32_t pdg_dropped_mpdu_warning                                :  1,
                      txop_duration                                           :  7,
                      pdg_overview_length                                     : 24;
             uint32_t reserved_17a                                            :  4,
                      ru_size                                                 :  4,
                      num_data_symbols                                        : 16,
                      stbc                                                    :  1,
                      fec_type                                                :  1,
                      packet_extension                                        :  3,
                      packet_extension_pe_disambiguity                        :  1,
                      packet_extension_a_factor                               :  2;
             uint32_t tx_pwr_unshared                                         :  8,
                      tx_pwr_shared                                           :  8,
                      reserved_18b                                            :  1,
                      force_extra_symbol                                      :  1,
                      ldpc_extra_symbol                                       :  1,
                      dcm                                                     :  1,
                      reserved_18a                                            :  5,
                      cp_setting                                              :  2,
                      ltf_size                                                :  2,
                      num_ltf_symbols                                         :  3;
             uint32_t cv_corr_status                                          :  8,
                      coex_lte_tx_while_wlan_rx                               :  1,
                      coex_lte_tx_while_wlan_tx                               :  1,
                      prot_coex_uwb_tx_while_wlan_rx                          :  1,
                      prot_coex_uwb_tx_while_wlan_tx                          :  1,
                      coex_uwb_tx_while_wlan_rx                               :  1,
                      coex_uwb_tx_while_wlan_tx                               :  1,
                      __reserved_g_0005_ftm_frame_sent                                  :  1,
                      __reserved_g_0005_sent_dummy_tx                                   :  1,
                      __reserved_g_0005_active_user_map                                 : 16;
             uint32_t reserved_21b                                            :  3,
                      hw_qos_null_setup_missing                               :  1,
                      hw_qos_null_bitmap                                      :  8,
                      reserved_21a                                            :  4,
                      current_tx_duration                                     : 16;
#endif
};

#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_TX_OFFSET                      0x00000000
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_TX_LSB                         0
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_TX_MSB                         0
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_TX_MASK                        0x00000001

#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_RX_OFFSET                      0x00000000
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_RX_LSB                         1
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_RX_MSB                         1
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_RX_MASK                        0x00000002

#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_TX_OFFSET                     0x00000000
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_TX_LSB                        2
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_TX_MSB                        2
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_TX_MASK                       0x00000004

#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_RX_OFFSET                     0x00000000
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_RX_LSB                        3
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_RX_MSB                        3
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_RX_MASK                       0x00000008

#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_TX_OFFSET                    0x00000000
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_TX_LSB                       4
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_TX_MSB                       4
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_TX_MASK                      0x00000010

#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_RX_OFFSET                    0x00000000
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_RX_LSB                       5
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_RX_MSB                       5
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_RX_MASK                      0x00000020

#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_TX_OFFSET                           0x00000000
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_TX_LSB                              6
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_TX_MSB                              6
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_TX_MASK                             0x00000040

#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_RX_OFFSET                           0x00000000
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_RX_LSB                              7
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_RX_MSB                              7
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_RX_MASK                             0x00000080

#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_TX_OFFSET                          0x00000000
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_TX_LSB                             8
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_TX_MSB                             8
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_TX_MASK                            0x00000100

#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_RX_OFFSET                          0x00000000
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_RX_LSB                             9
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_RX_MSB                             9
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_RX_MASK                            0x00000200

#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_TX_OFFSET                         0x00000000
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_TX_LSB                            10
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_TX_MSB                            10
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_TX_MASK                           0x00000400

#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_RX_OFFSET                         0x00000000
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_RX_LSB                            11
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_RX_MSB                            11
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_RX_MASK                           0x00000800

#define TX_FES_STATUS_END_GLOBAL_DATA_UNDERFLOW_WARNING_OFFSET                      0x00000000
#define TX_FES_STATUS_END_GLOBAL_DATA_UNDERFLOW_WARNING_LSB                         12
#define TX_FES_STATUS_END_GLOBAL_DATA_UNDERFLOW_WARNING_MSB                         12
#define TX_FES_STATUS_END_GLOBAL_DATA_UNDERFLOW_WARNING_MASK                        0x00001000

#define TX_FES_STATUS_END_GLOBAL_FES_TRANSMIT_RESULT_OFFSET                         0x00000000
#define TX_FES_STATUS_END_GLOBAL_FES_TRANSMIT_RESULT_LSB                            13
#define TX_FES_STATUS_END_GLOBAL_FES_TRANSMIT_RESULT_MSB                            16
#define TX_FES_STATUS_END_GLOBAL_FES_TRANSMIT_RESULT_MASK                           0x0001e000

#define TX_FES_STATUS_END_CBF_BW_RECEIVED_VALID_OFFSET                              0x00000000
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_VALID_LSB                                 17
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_VALID_MSB                                 17
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_VALID_MASK                                0x00020000

#define TX_FES_STATUS_END_CBF_BW_RECEIVED_OFFSET                                    0x00000000
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_LSB                                       18
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_MSB                                       20
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_MASK                                      0x001c0000

#define TX_FES_STATUS_END_ACTUAL_RECEIVED_ACK_TYPE_OFFSET                           0x00000000
#define TX_FES_STATUS_END_ACTUAL_RECEIVED_ACK_TYPE_LSB                              21
#define TX_FES_STATUS_END_ACTUAL_RECEIVED_ACK_TYPE_MSB                              24
#define TX_FES_STATUS_END_ACTUAL_RECEIVED_ACK_TYPE_MASK                             0x01e00000

#define TX_FES_STATUS_END_STA_RESPONSE_COUNT_OFFSET                                 0x00000000
#define TX_FES_STATUS_END_STA_RESPONSE_COUNT_LSB                                    25
#define TX_FES_STATUS_END_STA_RESPONSE_COUNT_MSB                                    30
#define TX_FES_STATUS_END_STA_RESPONSE_COUNT_MASK                                   0x7e000000

#define TX_FES_STATUS_END_MORE_DATA_RECEIVED_OFFSET                                 0x00000000
#define TX_FES_STATUS_END_MORE_DATA_RECEIVED_LSB                                    31
#define TX_FES_STATUS_END_MORE_DATA_RECEIVED_MSB                                    31
#define TX_FES_STATUS_END_MORE_DATA_RECEIVED_MASK                                   0x80000000

#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_OFFSET 0x00000004
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_LSB   0
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MSB   7
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MASK  0x000000ff

#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_OFFSET       0x00000004
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_LSB          8
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MSB          13
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MASK         0x00003f00

#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_OFFSET          0x00000004
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_LSB             14
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MSB             15
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MASK            0x0000c000

#define TX_FES_STATUS_END_RESERVED_AFTER_STRUCT16_OFFSET                            0x00000004
#define TX_FES_STATUS_END_RESERVED_AFTER_STRUCT16_LSB                               16
#define TX_FES_STATUS_END_RESERVED_AFTER_STRUCT16_MSB                               19
#define TX_FES_STATUS_END_RESERVED_AFTER_STRUCT16_MASK                              0x000f0000

#define TX_FES_STATUS_END_BRP_INFO_VALID_OFFSET                                     0x00000004
#define TX_FES_STATUS_END_BRP_INFO_VALID_LSB                                        20
#define TX_FES_STATUS_END_BRP_INFO_VALID_MSB                                        20
#define TX_FES_STATUS_END_BRP_INFO_VALID_MASK                                       0x00100000

#define TX_FES_STATUS_END_QOS_NULL_SWITCH_DONE_FOR_EOSP_OFFSET                      0x00000004
#define TX_FES_STATUS_END_QOS_NULL_SWITCH_DONE_FOR_EOSP_LSB                         21
#define TX_FES_STATUS_END_QOS_NULL_SWITCH_DONE_FOR_EOSP_MSB                         21
#define TX_FES_STATUS_END_QOS_NULL_SWITCH_DONE_FOR_EOSP_MASK                        0x00200000

#define TX_FES_STATUS_END_RESERVED_1A_OFFSET                                        0x00000004
#define TX_FES_STATUS_END_RESERVED_1A_LSB                                           22
#define TX_FES_STATUS_END_RESERVED_1A_MSB                                           26
#define TX_FES_STATUS_END_RESERVED_1A_MASK                                          0x07c00000

#define TX_FES_STATUS_END_PHYTX_PKT_END_INFO_VALID_OFFSET                           0x00000004
#define TX_FES_STATUS_END_PHYTX_PKT_END_INFO_VALID_LSB                              27
#define TX_FES_STATUS_END_PHYTX_PKT_END_INFO_VALID_MSB                              27
#define TX_FES_STATUS_END_PHYTX_PKT_END_INFO_VALID_MASK                             0x08000000

#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_VALID_OFFSET                     0x00000004
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_VALID_LSB                        28
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_VALID_MSB                        28
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_VALID_MASK                       0x10000000

#define TX_FES_STATUS_END_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_OFFSET                0x00000004
#define TX_FES_STATUS_END_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_LSB                   29
#define TX_FES_STATUS_END_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_MSB                   29
#define TX_FES_STATUS_END_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_MASK                  0x20000000

#define TX_FES_STATUS_END_NULL_DELIM_INSERTED_BEFORE_MPDUS_OFFSET                   0x00000004
#define TX_FES_STATUS_END_NULL_DELIM_INSERTED_BEFORE_MPDUS_LSB                      30
#define TX_FES_STATUS_END_NULL_DELIM_INSERTED_BEFORE_MPDUS_MSB                      30
#define TX_FES_STATUS_END_NULL_DELIM_INSERTED_BEFORE_MPDUS_MASK                     0x40000000

#define TX_FES_STATUS_END_ONLY_NULL_DELIM_SENT_OFFSET                               0x00000004
#define TX_FES_STATUS_END_ONLY_NULL_DELIM_SENT_LSB                                  31
#define TX_FES_STATUS_END_ONLY_NULL_DELIM_SENT_MSB                                  31
#define TX_FES_STATUS_END_ONLY_NULL_DELIM_SENT_MASK                                 0x80000000

#define TX_FES_STATUS_END_TERMINATE_RANGING_SEQUENCE_OFFSET                         0x00000008
#define TX_FES_STATUS_END_TERMINATE_RANGING_SEQUENCE_LSB                            0
#define TX_FES_STATUS_END_TERMINATE_RANGING_SEQUENCE_MSB                            0
#define TX_FES_STATUS_END_TERMINATE_RANGING_SEQUENCE_MASK                           0x00000001

#define TX_FES_STATUS_END_RESERVED_2B_OFFSET                                        0x00000008
#define TX_FES_STATUS_END_RESERVED_2B_LSB                                           1
#define TX_FES_STATUS_END_RESERVED_2B_MSB                                           5
#define TX_FES_STATUS_END_RESERVED_2B_MASK                                          0x0000003e

#define TX_FES_STATUS_END_RESPONSE_TYPE_OFFSET                                      0x00000008
#define TX_FES_STATUS_END_RESPONSE_TYPE_LSB                                         6
#define TX_FES_STATUS_END_RESPONSE_TYPE_MSB                                         10
#define TX_FES_STATUS_END_RESPONSE_TYPE_MASK                                        0x000007c0

#define TX_FES_STATUS_END_R2R_END_STATUS_TO_FOLLOW_OFFSET                           0x00000008
#define TX_FES_STATUS_END_R2R_END_STATUS_TO_FOLLOW_LSB                              11
#define TX_FES_STATUS_END_R2R_END_STATUS_TO_FOLLOW_MSB                              11
#define TX_FES_STATUS_END_R2R_END_STATUS_TO_FOLLOW_MASK                             0x00000800

#define TX_FES_STATUS_END_RESERVED_5A_OFFSET                                        0x00000008
#define TX_FES_STATUS_END_RESERVED_5A_LSB                                           12
#define TX_FES_STATUS_END_RESERVED_5A_MSB                                           14
#define TX_FES_STATUS_END_RESERVED_5A_MASK                                          0x00007000

#define TX_FES_STATUS_END_PROT_COEX_LTE_TX_WHILE_WLAN_TX_OFFSET                     0x00000008
#define TX_FES_STATUS_END_PROT_COEX_LTE_TX_WHILE_WLAN_TX_LSB                        15
#define TX_FES_STATUS_END_PROT_COEX_LTE_TX_WHILE_WLAN_TX_MSB                        15
#define TX_FES_STATUS_END_PROT_COEX_LTE_TX_WHILE_WLAN_TX_MASK                       0x00008000

#define TX_FES_STATUS_END_PROT_COEX_LTE_TX_WHILE_WLAN_RX_OFFSET                     0x00000008
#define TX_FES_STATUS_END_PROT_COEX_LTE_TX_WHILE_WLAN_RX_LSB                        16
#define TX_FES_STATUS_END_PROT_COEX_LTE_TX_WHILE_WLAN_RX_MSB                        16
#define TX_FES_STATUS_END_PROT_COEX_LTE_TX_WHILE_WLAN_RX_MASK                       0x00010000

#define TX_FES_STATUS_END_RESERVED_2C_OFFSET                                        0x00000008
#define TX_FES_STATUS_END_RESERVED_2C_LSB                                           17
#define TX_FES_STATUS_END_RESERVED_2C_MSB                                           31
#define TX_FES_STATUS_END_RESERVED_2C_MASK                                          0xfffe0000

#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_15_0_OFFSET                   0x0000000c
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_15_0_LSB                      0
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_15_0_MSB                      15
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_15_0_MASK                     0x0000ffff

#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_31_16_OFFSET                  0x0000000c
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_31_16_LSB                     16
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_31_16_MSB                     31
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_31_16_MASK                    0xffff0000

#define TX_FES_STATUS_END_CBF_SEGMENT_REQUEST_MASK_OFFSET                           0x00000010
#define TX_FES_STATUS_END_CBF_SEGMENT_REQUEST_MASK_LSB                              0
#define TX_FES_STATUS_END_CBF_SEGMENT_REQUEST_MASK_MSB                              7
#define TX_FES_STATUS_END_CBF_SEGMENT_REQUEST_MASK_MASK                             0x000000ff

#define TX_FES_STATUS_END_CBF_SEGMENT_SENT_MASK_OFFSET                              0x00000010
#define TX_FES_STATUS_END_CBF_SEGMENT_SENT_MASK_LSB                                 8
#define TX_FES_STATUS_END_CBF_SEGMENT_SENT_MASK_MSB                                 15
#define TX_FES_STATUS_END_CBF_SEGMENT_SENT_MASK_MASK                                0x0000ff00

#define TX_FES_STATUS_END_HIGHEST_ACHIEVED_DATA_NULL_RATIO_OFFSET                   0x00000010
#define TX_FES_STATUS_END_HIGHEST_ACHIEVED_DATA_NULL_RATIO_LSB                      16
#define TX_FES_STATUS_END_HIGHEST_ACHIEVED_DATA_NULL_RATIO_MSB                      20
#define TX_FES_STATUS_END_HIGHEST_ACHIEVED_DATA_NULL_RATIO_MASK                     0x001f0000

#define TX_FES_STATUS_END_USE_ALT_POWER_SR_OFFSET                                   0x00000010
#define TX_FES_STATUS_END_USE_ALT_POWER_SR_LSB                                      21
#define TX_FES_STATUS_END_USE_ALT_POWER_SR_MSB                                      21
#define TX_FES_STATUS_END_USE_ALT_POWER_SR_MASK                                     0x00200000

#define TX_FES_STATUS_END_STATIC_2_PWR_MODE_STATUS_OFFSET                           0x00000010
#define TX_FES_STATUS_END_STATIC_2_PWR_MODE_STATUS_LSB                              22
#define TX_FES_STATUS_END_STATIC_2_PWR_MODE_STATUS_MSB                              22
#define TX_FES_STATUS_END_STATIC_2_PWR_MODE_STATUS_MASK                             0x00400000

#define TX_FES_STATUS_END_OBSS_SRG_OPPORT_TRANSMIT_STATUS_OFFSET                    0x00000010
#define TX_FES_STATUS_END_OBSS_SRG_OPPORT_TRANSMIT_STATUS_LSB                       23
#define TX_FES_STATUS_END_OBSS_SRG_OPPORT_TRANSMIT_STATUS_MSB                       23
#define TX_FES_STATUS_END_OBSS_SRG_OPPORT_TRANSMIT_STATUS_MASK                      0x00800000

#define TX_FES_STATUS_END_SRP_BASED_TRANSMIT_STATUS_OFFSET                          0x00000010
#define TX_FES_STATUS_END_SRP_BASED_TRANSMIT_STATUS_LSB                             24
#define TX_FES_STATUS_END_SRP_BASED_TRANSMIT_STATUS_MSB                             24
#define TX_FES_STATUS_END_SRP_BASED_TRANSMIT_STATUS_MASK                            0x01000000

#define TX_FES_STATUS_END_OBSS_PD_BASED_TRANSMIT_STATUS_OFFSET                      0x00000010
#define TX_FES_STATUS_END_OBSS_PD_BASED_TRANSMIT_STATUS_LSB                         25
#define TX_FES_STATUS_END_OBSS_PD_BASED_TRANSMIT_STATUS_MSB                         25
#define TX_FES_STATUS_END_OBSS_PD_BASED_TRANSMIT_STATUS_MASK                        0x02000000

#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_36_32_OFFSET                  0x00000010
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_36_32_LSB                     26
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_36_32_MSB                     30
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_36_32_MASK                    0x7c000000

#define TX_FES_STATUS_END_PDG_MPDU_READY_OFFSET                                     0x00000010
#define TX_FES_STATUS_END_PDG_MPDU_READY_LSB                                        31
#define TX_FES_STATUS_END_PDG_MPDU_READY_MSB                                        31
#define TX_FES_STATUS_END_PDG_MPDU_READY_MASK                                       0x80000000

#define TX_FES_STATUS_END_PDG_MPDU_COUNT_OFFSET                                     0x00000014
#define TX_FES_STATUS_END_PDG_MPDU_COUNT_LSB                                        0
#define TX_FES_STATUS_END_PDG_MPDU_COUNT_MSB                                        15
#define TX_FES_STATUS_END_PDG_MPDU_COUNT_MASK                                       0x0000ffff

#define TX_FES_STATUS_END_PDG_EST_MPDU_TX_COUNT_OFFSET                              0x00000014
#define TX_FES_STATUS_END_PDG_EST_MPDU_TX_COUNT_LSB                                 16
#define TX_FES_STATUS_END_PDG_EST_MPDU_TX_COUNT_MSB                                 31
#define TX_FES_STATUS_END_PDG_EST_MPDU_TX_COUNT_MASK                                0xffff0000

#define TX_FES_STATUS_END_PDG_OVERVIEW_LENGTH_OFFSET                                0x00000018
#define TX_FES_STATUS_END_PDG_OVERVIEW_LENGTH_LSB                                   0
#define TX_FES_STATUS_END_PDG_OVERVIEW_LENGTH_MSB                                   23
#define TX_FES_STATUS_END_PDG_OVERVIEW_LENGTH_MASK                                  0x00ffffff

#define TX_FES_STATUS_END_TXOP_DURATION_OFFSET                                      0x00000018
#define TX_FES_STATUS_END_TXOP_DURATION_LSB                                         24
#define TX_FES_STATUS_END_TXOP_DURATION_MSB                                         30
#define TX_FES_STATUS_END_TXOP_DURATION_MASK                                        0x7f000000

#define TX_FES_STATUS_END_PDG_DROPPED_MPDU_WARNING_OFFSET                           0x00000018
#define TX_FES_STATUS_END_PDG_DROPPED_MPDU_WARNING_LSB                              31
#define TX_FES_STATUS_END_PDG_DROPPED_MPDU_WARNING_MSB                              31
#define TX_FES_STATUS_END_PDG_DROPPED_MPDU_WARNING_MASK                             0x80000000

#define TX_FES_STATUS_END_PACKET_EXTENSION_A_FACTOR_OFFSET                          0x0000001c
#define TX_FES_STATUS_END_PACKET_EXTENSION_A_FACTOR_LSB                             0
#define TX_FES_STATUS_END_PACKET_EXTENSION_A_FACTOR_MSB                             1
#define TX_FES_STATUS_END_PACKET_EXTENSION_A_FACTOR_MASK                            0x00000003

#define TX_FES_STATUS_END_PACKET_EXTENSION_PE_DISAMBIGUITY_OFFSET                   0x0000001c
#define TX_FES_STATUS_END_PACKET_EXTENSION_PE_DISAMBIGUITY_LSB                      2
#define TX_FES_STATUS_END_PACKET_EXTENSION_PE_DISAMBIGUITY_MSB                      2
#define TX_FES_STATUS_END_PACKET_EXTENSION_PE_DISAMBIGUITY_MASK                     0x00000004

#define TX_FES_STATUS_END_PACKET_EXTENSION_OFFSET                                   0x0000001c
#define TX_FES_STATUS_END_PACKET_EXTENSION_LSB                                      3
#define TX_FES_STATUS_END_PACKET_EXTENSION_MSB                                      5
#define TX_FES_STATUS_END_PACKET_EXTENSION_MASK                                     0x00000038

#define TX_FES_STATUS_END_FEC_TYPE_OFFSET                                           0x0000001c
#define TX_FES_STATUS_END_FEC_TYPE_LSB                                              6
#define TX_FES_STATUS_END_FEC_TYPE_MSB                                              6
#define TX_FES_STATUS_END_FEC_TYPE_MASK                                             0x00000040

#define TX_FES_STATUS_END_STBC_OFFSET                                               0x0000001c
#define TX_FES_STATUS_END_STBC_LSB                                                  7
#define TX_FES_STATUS_END_STBC_MSB                                                  7
#define TX_FES_STATUS_END_STBC_MASK                                                 0x00000080

#define TX_FES_STATUS_END_NUM_DATA_SYMBOLS_OFFSET                                   0x0000001c
#define TX_FES_STATUS_END_NUM_DATA_SYMBOLS_LSB                                      8
#define TX_FES_STATUS_END_NUM_DATA_SYMBOLS_MSB                                      23
#define TX_FES_STATUS_END_NUM_DATA_SYMBOLS_MASK                                     0x00ffff00

#define TX_FES_STATUS_END_RU_SIZE_OFFSET                                            0x0000001c
#define TX_FES_STATUS_END_RU_SIZE_LSB                                               24
#define TX_FES_STATUS_END_RU_SIZE_MSB                                               27
#define TX_FES_STATUS_END_RU_SIZE_MASK                                              0x0f000000

#define TX_FES_STATUS_END_RESERVED_17A_OFFSET                                       0x0000001c
#define TX_FES_STATUS_END_RESERVED_17A_LSB                                          28
#define TX_FES_STATUS_END_RESERVED_17A_MSB                                          31
#define TX_FES_STATUS_END_RESERVED_17A_MASK                                         0xf0000000

#define TX_FES_STATUS_END_NUM_LTF_SYMBOLS_OFFSET                                    0x00000020
#define TX_FES_STATUS_END_NUM_LTF_SYMBOLS_LSB                                       0
#define TX_FES_STATUS_END_NUM_LTF_SYMBOLS_MSB                                       2
#define TX_FES_STATUS_END_NUM_LTF_SYMBOLS_MASK                                      0x00000007

#define TX_FES_STATUS_END_LTF_SIZE_OFFSET                                           0x00000020
#define TX_FES_STATUS_END_LTF_SIZE_LSB                                              3
#define TX_FES_STATUS_END_LTF_SIZE_MSB                                              4
#define TX_FES_STATUS_END_LTF_SIZE_MASK                                             0x00000018

#define TX_FES_STATUS_END_CP_SETTING_OFFSET                                         0x00000020
#define TX_FES_STATUS_END_CP_SETTING_LSB                                            5
#define TX_FES_STATUS_END_CP_SETTING_MSB                                            6
#define TX_FES_STATUS_END_CP_SETTING_MASK                                           0x00000060

#define TX_FES_STATUS_END_RESERVED_18A_OFFSET                                       0x00000020
#define TX_FES_STATUS_END_RESERVED_18A_LSB                                          7
#define TX_FES_STATUS_END_RESERVED_18A_MSB                                          11
#define TX_FES_STATUS_END_RESERVED_18A_MASK                                         0x00000f80

#define TX_FES_STATUS_END_DCM_OFFSET                                                0x00000020
#define TX_FES_STATUS_END_DCM_LSB                                                   12
#define TX_FES_STATUS_END_DCM_MSB                                                   12
#define TX_FES_STATUS_END_DCM_MASK                                                  0x00001000

#define TX_FES_STATUS_END_LDPC_EXTRA_SYMBOL_OFFSET                                  0x00000020
#define TX_FES_STATUS_END_LDPC_EXTRA_SYMBOL_LSB                                     13
#define TX_FES_STATUS_END_LDPC_EXTRA_SYMBOL_MSB                                     13
#define TX_FES_STATUS_END_LDPC_EXTRA_SYMBOL_MASK                                    0x00002000

#define TX_FES_STATUS_END_FORCE_EXTRA_SYMBOL_OFFSET                                 0x00000020
#define TX_FES_STATUS_END_FORCE_EXTRA_SYMBOL_LSB                                    14
#define TX_FES_STATUS_END_FORCE_EXTRA_SYMBOL_MSB                                    14
#define TX_FES_STATUS_END_FORCE_EXTRA_SYMBOL_MASK                                   0x00004000

#define TX_FES_STATUS_END_RESERVED_18B_OFFSET                                       0x00000020
#define TX_FES_STATUS_END_RESERVED_18B_LSB                                          15
#define TX_FES_STATUS_END_RESERVED_18B_MSB                                          15
#define TX_FES_STATUS_END_RESERVED_18B_MASK                                         0x00008000

#define TX_FES_STATUS_END_TX_PWR_SHARED_OFFSET                                      0x00000020
#define TX_FES_STATUS_END_TX_PWR_SHARED_LSB                                         16
#define TX_FES_STATUS_END_TX_PWR_SHARED_MSB                                         23
#define TX_FES_STATUS_END_TX_PWR_SHARED_MASK                                        0x00ff0000

#define TX_FES_STATUS_END_TX_PWR_UNSHARED_OFFSET                                    0x00000020
#define TX_FES_STATUS_END_TX_PWR_UNSHARED_LSB                                       24
#define TX_FES_STATUS_END_TX_PWR_UNSHARED_MSB                                       31
#define TX_FES_STATUS_END_TX_PWR_UNSHARED_MASK                                      0xff000000

#define TX_FES_STATUS_END_RANGING_ACTIVE_USER_MAP_OFFSET                            0x00000024
#define TX_FES_STATUS_END_RANGING_ACTIVE_USER_MAP_LSB                               0
#define TX_FES_STATUS_END_RANGING_ACTIVE_USER_MAP_MSB                               15
#define TX_FES_STATUS_END_RANGING_ACTIVE_USER_MAP_MASK                              0x0000ffff

#define TX_FES_STATUS_END_RANGING_SENT_DUMMY_TX_OFFSET                              0x00000024
#define TX_FES_STATUS_END_RANGING_SENT_DUMMY_TX_LSB                                 16
#define TX_FES_STATUS_END_RANGING_SENT_DUMMY_TX_MSB                                 16
#define TX_FES_STATUS_END_RANGING_SENT_DUMMY_TX_MASK                                0x00010000

#define TX_FES_STATUS_END_RANGING_FTM_FRAME_SENT_OFFSET                             0x00000024
#define TX_FES_STATUS_END_RANGING_FTM_FRAME_SENT_LSB                                17
#define TX_FES_STATUS_END_RANGING_FTM_FRAME_SENT_MSB                                17
#define TX_FES_STATUS_END_RANGING_FTM_FRAME_SENT_MASK                               0x00020000

#define TX_FES_STATUS_END_COEX_UWB_TX_WHILE_WLAN_TX_OFFSET                          0x00000024
#define TX_FES_STATUS_END_COEX_UWB_TX_WHILE_WLAN_TX_LSB                             18
#define TX_FES_STATUS_END_COEX_UWB_TX_WHILE_WLAN_TX_MSB                             18
#define TX_FES_STATUS_END_COEX_UWB_TX_WHILE_WLAN_TX_MASK                            0x00040000

#define TX_FES_STATUS_END_COEX_UWB_TX_WHILE_WLAN_RX_OFFSET                          0x00000024
#define TX_FES_STATUS_END_COEX_UWB_TX_WHILE_WLAN_RX_LSB                             19
#define TX_FES_STATUS_END_COEX_UWB_TX_WHILE_WLAN_RX_MSB                             19
#define TX_FES_STATUS_END_COEX_UWB_TX_WHILE_WLAN_RX_MASK                            0x00080000

#define TX_FES_STATUS_END_PROT_COEX_UWB_TX_WHILE_WLAN_TX_OFFSET                     0x00000024
#define TX_FES_STATUS_END_PROT_COEX_UWB_TX_WHILE_WLAN_TX_LSB                        20
#define TX_FES_STATUS_END_PROT_COEX_UWB_TX_WHILE_WLAN_TX_MSB                        20
#define TX_FES_STATUS_END_PROT_COEX_UWB_TX_WHILE_WLAN_TX_MASK                       0x00100000

#define TX_FES_STATUS_END_PROT_COEX_UWB_TX_WHILE_WLAN_RX_OFFSET                     0x00000024
#define TX_FES_STATUS_END_PROT_COEX_UWB_TX_WHILE_WLAN_RX_LSB                        21
#define TX_FES_STATUS_END_PROT_COEX_UWB_TX_WHILE_WLAN_RX_MSB                        21
#define TX_FES_STATUS_END_PROT_COEX_UWB_TX_WHILE_WLAN_RX_MASK                       0x00200000

#define TX_FES_STATUS_END_COEX_LTE_TX_WHILE_WLAN_TX_OFFSET                          0x00000024
#define TX_FES_STATUS_END_COEX_LTE_TX_WHILE_WLAN_TX_LSB                             22
#define TX_FES_STATUS_END_COEX_LTE_TX_WHILE_WLAN_TX_MSB                             22
#define TX_FES_STATUS_END_COEX_LTE_TX_WHILE_WLAN_TX_MASK                            0x00400000

#define TX_FES_STATUS_END_COEX_LTE_TX_WHILE_WLAN_RX_OFFSET                          0x00000024
#define TX_FES_STATUS_END_COEX_LTE_TX_WHILE_WLAN_RX_LSB                             23
#define TX_FES_STATUS_END_COEX_LTE_TX_WHILE_WLAN_RX_MSB                             23
#define TX_FES_STATUS_END_COEX_LTE_TX_WHILE_WLAN_RX_MASK                            0x00800000

#define TX_FES_STATUS_END_CV_CORR_STATUS_OFFSET                                     0x00000024
#define TX_FES_STATUS_END_CV_CORR_STATUS_LSB                                        24
#define TX_FES_STATUS_END_CV_CORR_STATUS_MSB                                        31
#define TX_FES_STATUS_END_CV_CORR_STATUS_MASK                                       0xff000000

#define TX_FES_STATUS_END_CURRENT_TX_DURATION_OFFSET                                0x00000028
#define TX_FES_STATUS_END_CURRENT_TX_DURATION_LSB                                   0
#define TX_FES_STATUS_END_CURRENT_TX_DURATION_MSB                                   15
#define TX_FES_STATUS_END_CURRENT_TX_DURATION_MASK                                  0x0000ffff

#define TX_FES_STATUS_END_RESERVED_21A_OFFSET                                       0x00000028
#define TX_FES_STATUS_END_RESERVED_21A_LSB                                          16
#define TX_FES_STATUS_END_RESERVED_21A_MSB                                          19
#define TX_FES_STATUS_END_RESERVED_21A_MASK                                         0x000f0000

#define TX_FES_STATUS_END_HW_QOS_NULL_BITMAP_OFFSET                                 0x00000028
#define TX_FES_STATUS_END_HW_QOS_NULL_BITMAP_LSB                                    20
#define TX_FES_STATUS_END_HW_QOS_NULL_BITMAP_MSB                                    27
#define TX_FES_STATUS_END_HW_QOS_NULL_BITMAP_MASK                                   0x0ff00000

#define TX_FES_STATUS_END_HW_QOS_NULL_SETUP_MISSING_OFFSET                          0x00000028
#define TX_FES_STATUS_END_HW_QOS_NULL_SETUP_MISSING_LSB                             28
#define TX_FES_STATUS_END_HW_QOS_NULL_SETUP_MISSING_MSB                             28
#define TX_FES_STATUS_END_HW_QOS_NULL_SETUP_MISSING_MASK                            0x10000000

#define TX_FES_STATUS_END_RESERVED_21B_OFFSET                                       0x00000028
#define TX_FES_STATUS_END_RESERVED_21B_LSB                                          29
#define TX_FES_STATUS_END_RESERVED_21B_MSB                                          31
#define TX_FES_STATUS_END_RESERVED_21B_MASK                                         0xe0000000

#endif
