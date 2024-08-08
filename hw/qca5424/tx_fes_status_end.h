
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
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
                      dpdtrain_done                                           :  1;  
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint16_t reserved_after_struct16                                 :  4,  
                      brp_info_valid                                          :  1,  
                      reserved_1a                                             :  6,  
                      phytx_pkt_end_info_valid                                :  1,  
                      phytx_abort_request_info_valid                          :  1,  
                      fes_in_11ax_trigger_response_config                     :  1,  
                      null_delim_inserted_before_mpdus                        :  1,  
                      only_null_delim_sent                                    :  1;  
             uint32_t start_of_frame_timestamp_15_0                           : 16,  
                      start_of_frame_timestamp_31_16                          : 16;  
             uint32_t end_of_frame_timestamp_15_0                             : 16,  
                      end_of_frame_timestamp_31_16                            : 16;  
             uint32_t terminate_ranging_sequence                              :  1,  
                      reserved_4a                                             :  7,  
                      timing_status                                           :  2,  
                      response_type                                           :  5,  
                      r2r_end_status_to_follow                                :  1,  
                      transmit_delay                                          : 16;  
             uint32_t tx_group_delay                                          : 12,  
                      reserved_5a                                             :  4,  
                      tpc_dbg_info_cmn_15_0                                   : 16;  
             uint32_t tpc_dbg_info_cmn_31_16                                  : 16,  
                      tpc_dbg_info_47_32                                      : 16;  
             uint32_t tpc_dbg_info_chn1_15_0                                  : 16,  
                      tpc_dbg_info_chn1_31_16                                 : 16;  
             uint32_t tpc_dbg_info_chn1_47_32                                 : 16,  
                      tpc_dbg_info_chn1_63_48                                 : 16;  
             uint32_t tpc_dbg_info_chn1_79_64                                 : 16,  
                      tpc_dbg_info_chn2_15_0                                  : 16;  
             uint32_t tpc_dbg_info_chn2_31_16                                 : 16,  
                      tpc_dbg_info_chn2_47_32                                 : 16;  
             uint32_t tpc_dbg_info_chn2_63_48                                 : 16,  
                      tpc_dbg_info_chn2_79_64                                 : 16;  
             uint32_t phytx_tx_end_sw_info_15_0                               : 16,  
                      phytx_tx_end_sw_info_31_16                              : 16;  
             uint32_t phytx_tx_end_sw_info_47_32                              : 16,  
                      phytx_tx_end_sw_info_63_48                              : 16;  
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
             uint32_t ranging_active_user_map                                 : 16,  
                      ranging_sent_dummy_tx                                   :  1,  
                      ranging_ftm_frame_sent                                  :  1,  
                      reserved_20a                                            :  6,  
                      cv_corr_status                                          :  8;  
             uint32_t current_tx_duration                                     : 16,  
                      reserved_21a                                            : 16;  
#else
             uint32_t dpdtrain_done                                           :  1,  
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
                      reserved_1a                                             :  6,  
                      brp_info_valid                                          :  1,  
                      reserved_after_struct16                                 :  4;  
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint32_t start_of_frame_timestamp_31_16                          : 16,  
                      start_of_frame_timestamp_15_0                           : 16;  
             uint32_t end_of_frame_timestamp_31_16                            : 16,  
                      end_of_frame_timestamp_15_0                             : 16;  
             uint32_t transmit_delay                                          : 16,  
                      r2r_end_status_to_follow                                :  1,  
                      response_type                                           :  5,  
                      timing_status                                           :  2,  
                      reserved_4a                                             :  7,  
                      terminate_ranging_sequence                              :  1;  
             uint32_t tpc_dbg_info_cmn_15_0                                   : 16,  
                      reserved_5a                                             :  4,  
                      tx_group_delay                                          : 12;  
             uint32_t tpc_dbg_info_47_32                                      : 16,  
                      tpc_dbg_info_cmn_31_16                                  : 16;  
             uint32_t tpc_dbg_info_chn1_31_16                                 : 16,  
                      tpc_dbg_info_chn1_15_0                                  : 16;  
             uint32_t tpc_dbg_info_chn1_63_48                                 : 16,  
                      tpc_dbg_info_chn1_47_32                                 : 16;  
             uint32_t tpc_dbg_info_chn2_15_0                                  : 16,  
                      tpc_dbg_info_chn1_79_64                                 : 16;  
             uint32_t tpc_dbg_info_chn2_47_32                                 : 16,  
                      tpc_dbg_info_chn2_31_16                                 : 16;  
             uint32_t tpc_dbg_info_chn2_79_64                                 : 16,  
                      tpc_dbg_info_chn2_63_48                                 : 16;  
             uint32_t phytx_tx_end_sw_info_31_16                              : 16,  
                      phytx_tx_end_sw_info_15_0                               : 16;  
             uint32_t phytx_tx_end_sw_info_63_48                              : 16,  
                      phytx_tx_end_sw_info_47_32                              : 16;  
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
                      reserved_20a                                            :  6,  
                      ranging_ftm_frame_sent                                  :  1,  
                      ranging_sent_dummy_tx                                   :  1,  
                      ranging_active_user_map                                 : 16;  
             uint32_t reserved_21a                                            : 16,  
                      current_tx_duration                                     : 16;  
#endif
};


 

#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_TX_OFFSET                      0x0000000000000000
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_TX_LSB                         0
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_TX_MSB                         0
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_TX_MASK                        0x0000000000000001


 

#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_RX_OFFSET                      0x0000000000000000
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_RX_LSB                         1
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_RX_MSB                         1
#define TX_FES_STATUS_END_PROT_COEX_BT_TX_WHILE_WLAN_RX_MASK                        0x0000000000000002


 

#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_TX_OFFSET                     0x0000000000000000
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_TX_LSB                        2
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_TX_MSB                        2
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_TX_MASK                       0x0000000000000004


 

#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_RX_OFFSET                     0x0000000000000000
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_RX_LSB                        3
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_RX_MSB                        3
#define TX_FES_STATUS_END_PROT_COEX_WAN_TX_WHILE_WLAN_RX_MASK                       0x0000000000000008


 

#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_TX_OFFSET                    0x0000000000000000
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_TX_LSB                       4
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_TX_MSB                       4
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_TX_MASK                      0x0000000000000010


 

#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_RX_OFFSET                    0x0000000000000000
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_RX_LSB                       5
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_RX_MSB                       5
#define TX_FES_STATUS_END_PROT_COEX_WLAN_TX_WHILE_WLAN_RX_MASK                      0x0000000000000020


 

#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_TX_OFFSET                           0x0000000000000000
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_TX_LSB                              6
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_TX_MSB                              6
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_TX_MASK                             0x0000000000000040


 

#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_RX_OFFSET                           0x0000000000000000
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_RX_LSB                              7
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_RX_MSB                              7
#define TX_FES_STATUS_END_COEX_BT_TX_WHILE_WLAN_RX_MASK                             0x0000000000000080


 

#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_TX_OFFSET                          0x0000000000000000
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_TX_LSB                             8
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_TX_MSB                             8
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_TX_MASK                            0x0000000000000100


 

#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_RX_OFFSET                          0x0000000000000000
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_RX_LSB                             9
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_RX_MSB                             9
#define TX_FES_STATUS_END_COEX_WAN_TX_WHILE_WLAN_RX_MASK                            0x0000000000000200


 

#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_TX_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_TX_LSB                            10
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_TX_MSB                            10
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_TX_MASK                           0x0000000000000400


 

#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_RX_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_RX_LSB                            11
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_RX_MSB                            11
#define TX_FES_STATUS_END_COEX_WLAN_TX_WHILE_WLAN_RX_MASK                           0x0000000000000800


 

#define TX_FES_STATUS_END_GLOBAL_DATA_UNDERFLOW_WARNING_OFFSET                      0x0000000000000000
#define TX_FES_STATUS_END_GLOBAL_DATA_UNDERFLOW_WARNING_LSB                         12
#define TX_FES_STATUS_END_GLOBAL_DATA_UNDERFLOW_WARNING_MSB                         12
#define TX_FES_STATUS_END_GLOBAL_DATA_UNDERFLOW_WARNING_MASK                        0x0000000000001000


 

#define TX_FES_STATUS_END_GLOBAL_FES_TRANSMIT_RESULT_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_END_GLOBAL_FES_TRANSMIT_RESULT_LSB                            13
#define TX_FES_STATUS_END_GLOBAL_FES_TRANSMIT_RESULT_MSB                            16
#define TX_FES_STATUS_END_GLOBAL_FES_TRANSMIT_RESULT_MASK                           0x000000000001e000


 

#define TX_FES_STATUS_END_CBF_BW_RECEIVED_VALID_OFFSET                              0x0000000000000000
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_VALID_LSB                                 17
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_VALID_MSB                                 17
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_VALID_MASK                                0x0000000000020000


 

#define TX_FES_STATUS_END_CBF_BW_RECEIVED_OFFSET                                    0x0000000000000000
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_LSB                                       18
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_MSB                                       20
#define TX_FES_STATUS_END_CBF_BW_RECEIVED_MASK                                      0x00000000001c0000


 

#define TX_FES_STATUS_END_ACTUAL_RECEIVED_ACK_TYPE_OFFSET                           0x0000000000000000
#define TX_FES_STATUS_END_ACTUAL_RECEIVED_ACK_TYPE_LSB                              21
#define TX_FES_STATUS_END_ACTUAL_RECEIVED_ACK_TYPE_MSB                              24
#define TX_FES_STATUS_END_ACTUAL_RECEIVED_ACK_TYPE_MASK                             0x0000000001e00000


 

#define TX_FES_STATUS_END_STA_RESPONSE_COUNT_OFFSET                                 0x0000000000000000
#define TX_FES_STATUS_END_STA_RESPONSE_COUNT_LSB                                    25
#define TX_FES_STATUS_END_STA_RESPONSE_COUNT_MSB                                    30
#define TX_FES_STATUS_END_STA_RESPONSE_COUNT_MASK                                   0x000000007e000000


 

#define TX_FES_STATUS_END_DPDTRAIN_DONE_OFFSET                                      0x0000000000000000
#define TX_FES_STATUS_END_DPDTRAIN_DONE_LSB                                         31
#define TX_FES_STATUS_END_DPDTRAIN_DONE_MSB                                         31
#define TX_FES_STATUS_END_DPDTRAIN_DONE_MASK                                        0x0000000080000000


 


 

#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_OFFSET 0x0000000000000000
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_LSB   32
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MSB   39
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MASK  0x000000ff00000000


 

#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_OFFSET       0x0000000000000000
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_LSB          40
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MSB          45
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MASK         0x00003f0000000000


 

#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_OFFSET          0x0000000000000000
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_LSB             46
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MSB             47
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MASK            0x0000c00000000000


 

#define TX_FES_STATUS_END_RESERVED_AFTER_STRUCT16_OFFSET                            0x0000000000000000
#define TX_FES_STATUS_END_RESERVED_AFTER_STRUCT16_LSB                               48
#define TX_FES_STATUS_END_RESERVED_AFTER_STRUCT16_MSB                               51
#define TX_FES_STATUS_END_RESERVED_AFTER_STRUCT16_MASK                              0x000f000000000000


 

#define TX_FES_STATUS_END_BRP_INFO_VALID_OFFSET                                     0x0000000000000000
#define TX_FES_STATUS_END_BRP_INFO_VALID_LSB                                        52
#define TX_FES_STATUS_END_BRP_INFO_VALID_MSB                                        52
#define TX_FES_STATUS_END_BRP_INFO_VALID_MASK                                       0x0010000000000000


 

#define TX_FES_STATUS_END_RESERVED_1A_OFFSET                                        0x0000000000000000
#define TX_FES_STATUS_END_RESERVED_1A_LSB                                           53
#define TX_FES_STATUS_END_RESERVED_1A_MSB                                           58
#define TX_FES_STATUS_END_RESERVED_1A_MASK                                          0x07e0000000000000


 

#define TX_FES_STATUS_END_PHYTX_PKT_END_INFO_VALID_OFFSET                           0x0000000000000000
#define TX_FES_STATUS_END_PHYTX_PKT_END_INFO_VALID_LSB                              59
#define TX_FES_STATUS_END_PHYTX_PKT_END_INFO_VALID_MSB                              59
#define TX_FES_STATUS_END_PHYTX_PKT_END_INFO_VALID_MASK                             0x0800000000000000


 

#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_VALID_OFFSET                     0x0000000000000000
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_VALID_LSB                        60
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_VALID_MSB                        60
#define TX_FES_STATUS_END_PHYTX_ABORT_REQUEST_INFO_VALID_MASK                       0x1000000000000000


 

#define TX_FES_STATUS_END_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_OFFSET                0x0000000000000000
#define TX_FES_STATUS_END_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_LSB                   61
#define TX_FES_STATUS_END_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_MSB                   61
#define TX_FES_STATUS_END_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_MASK                  0x2000000000000000


 

#define TX_FES_STATUS_END_NULL_DELIM_INSERTED_BEFORE_MPDUS_OFFSET                   0x0000000000000000
#define TX_FES_STATUS_END_NULL_DELIM_INSERTED_BEFORE_MPDUS_LSB                      62
#define TX_FES_STATUS_END_NULL_DELIM_INSERTED_BEFORE_MPDUS_MSB                      62
#define TX_FES_STATUS_END_NULL_DELIM_INSERTED_BEFORE_MPDUS_MASK                     0x4000000000000000


 

#define TX_FES_STATUS_END_ONLY_NULL_DELIM_SENT_OFFSET                               0x0000000000000000
#define TX_FES_STATUS_END_ONLY_NULL_DELIM_SENT_LSB                                  63
#define TX_FES_STATUS_END_ONLY_NULL_DELIM_SENT_MSB                                  63
#define TX_FES_STATUS_END_ONLY_NULL_DELIM_SENT_MASK                                 0x8000000000000000


 

#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_15_0_OFFSET                      0x0000000000000008
#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_15_0_LSB                         0
#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_15_0_MSB                         15
#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_15_0_MASK                        0x000000000000ffff


 

#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_31_16_OFFSET                     0x0000000000000008
#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_31_16_LSB                        16
#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_31_16_MSB                        31
#define TX_FES_STATUS_END_START_OF_FRAME_TIMESTAMP_31_16_MASK                       0x00000000ffff0000


 

#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_15_0_OFFSET                        0x0000000000000008
#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_15_0_LSB                           32
#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_15_0_MSB                           47
#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_15_0_MASK                          0x0000ffff00000000


 

#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_31_16_OFFSET                       0x0000000000000008
#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_31_16_LSB                          48
#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_31_16_MSB                          63
#define TX_FES_STATUS_END_END_OF_FRAME_TIMESTAMP_31_16_MASK                         0xffff000000000000


 

#define TX_FES_STATUS_END_TERMINATE_RANGING_SEQUENCE_OFFSET                         0x0000000000000010
#define TX_FES_STATUS_END_TERMINATE_RANGING_SEQUENCE_LSB                            0
#define TX_FES_STATUS_END_TERMINATE_RANGING_SEQUENCE_MSB                            0
#define TX_FES_STATUS_END_TERMINATE_RANGING_SEQUENCE_MASK                           0x0000000000000001


 

#define TX_FES_STATUS_END_RESERVED_4A_OFFSET                                        0x0000000000000010
#define TX_FES_STATUS_END_RESERVED_4A_LSB                                           1
#define TX_FES_STATUS_END_RESERVED_4A_MSB                                           7
#define TX_FES_STATUS_END_RESERVED_4A_MASK                                          0x00000000000000fe


 

#define TX_FES_STATUS_END_TIMING_STATUS_OFFSET                                      0x0000000000000010
#define TX_FES_STATUS_END_TIMING_STATUS_LSB                                         8
#define TX_FES_STATUS_END_TIMING_STATUS_MSB                                         9
#define TX_FES_STATUS_END_TIMING_STATUS_MASK                                        0x0000000000000300


 

#define TX_FES_STATUS_END_RESPONSE_TYPE_OFFSET                                      0x0000000000000010
#define TX_FES_STATUS_END_RESPONSE_TYPE_LSB                                         10
#define TX_FES_STATUS_END_RESPONSE_TYPE_MSB                                         14
#define TX_FES_STATUS_END_RESPONSE_TYPE_MASK                                        0x0000000000007c00


 

#define TX_FES_STATUS_END_R2R_END_STATUS_TO_FOLLOW_OFFSET                           0x0000000000000010
#define TX_FES_STATUS_END_R2R_END_STATUS_TO_FOLLOW_LSB                              15
#define TX_FES_STATUS_END_R2R_END_STATUS_TO_FOLLOW_MSB                              15
#define TX_FES_STATUS_END_R2R_END_STATUS_TO_FOLLOW_MASK                             0x0000000000008000


 

#define TX_FES_STATUS_END_TRANSMIT_DELAY_OFFSET                                     0x0000000000000010
#define TX_FES_STATUS_END_TRANSMIT_DELAY_LSB                                        16
#define TX_FES_STATUS_END_TRANSMIT_DELAY_MSB                                        31
#define TX_FES_STATUS_END_TRANSMIT_DELAY_MASK                                       0x00000000ffff0000


 

#define TX_FES_STATUS_END_TX_GROUP_DELAY_OFFSET                                     0x0000000000000010
#define TX_FES_STATUS_END_TX_GROUP_DELAY_LSB                                        32
#define TX_FES_STATUS_END_TX_GROUP_DELAY_MSB                                        43
#define TX_FES_STATUS_END_TX_GROUP_DELAY_MASK                                       0x00000fff00000000


 

#define TX_FES_STATUS_END_RESERVED_5A_OFFSET                                        0x0000000000000010
#define TX_FES_STATUS_END_RESERVED_5A_LSB                                           44
#define TX_FES_STATUS_END_RESERVED_5A_MSB                                           47
#define TX_FES_STATUS_END_RESERVED_5A_MASK                                          0x0000f00000000000


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_15_0_OFFSET                              0x0000000000000010
#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_15_0_LSB                                 48
#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_15_0_MSB                                 63
#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_15_0_MASK                                0xffff000000000000


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_31_16_OFFSET                             0x0000000000000018
#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_31_16_LSB                                0
#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_31_16_MSB                                15
#define TX_FES_STATUS_END_TPC_DBG_INFO_CMN_31_16_MASK                               0x000000000000ffff


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_47_32_OFFSET                                 0x0000000000000018
#define TX_FES_STATUS_END_TPC_DBG_INFO_47_32_LSB                                    16
#define TX_FES_STATUS_END_TPC_DBG_INFO_47_32_MSB                                    31
#define TX_FES_STATUS_END_TPC_DBG_INFO_47_32_MASK                                   0x00000000ffff0000


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_15_0_OFFSET                             0x0000000000000018
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_15_0_LSB                                32
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_15_0_MSB                                47
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_15_0_MASK                               0x0000ffff00000000


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_31_16_OFFSET                            0x0000000000000018
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_31_16_LSB                               48
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_31_16_MSB                               63
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_31_16_MASK                              0xffff000000000000


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_47_32_OFFSET                            0x0000000000000020
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_47_32_LSB                               0
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_47_32_MSB                               15
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_47_32_MASK                              0x000000000000ffff


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_63_48_OFFSET                            0x0000000000000020
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_63_48_LSB                               16
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_63_48_MSB                               31
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_63_48_MASK                              0x00000000ffff0000


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_79_64_OFFSET                            0x0000000000000020
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_79_64_LSB                               32
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_79_64_MSB                               47
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN1_79_64_MASK                              0x0000ffff00000000


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_15_0_OFFSET                             0x0000000000000020
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_15_0_LSB                                48
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_15_0_MSB                                63
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_15_0_MASK                               0xffff000000000000


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_31_16_OFFSET                            0x0000000000000028
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_31_16_LSB                               0
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_31_16_MSB                               15
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_31_16_MASK                              0x000000000000ffff


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_47_32_OFFSET                            0x0000000000000028
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_47_32_LSB                               16
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_47_32_MSB                               31
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_47_32_MASK                              0x00000000ffff0000


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_63_48_OFFSET                            0x0000000000000028
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_63_48_LSB                               32
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_63_48_MSB                               47
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_63_48_MASK                              0x0000ffff00000000


 

#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_79_64_OFFSET                            0x0000000000000028
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_79_64_LSB                               48
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_79_64_MSB                               63
#define TX_FES_STATUS_END_TPC_DBG_INFO_CHN2_79_64_MASK                              0xffff000000000000


 

#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_15_0_OFFSET                          0x0000000000000030
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_15_0_LSB                             0
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_15_0_MSB                             15
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_15_0_MASK                            0x000000000000ffff


 

#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_31_16_OFFSET                         0x0000000000000030
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_31_16_LSB                            16
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_31_16_MSB                            31
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_31_16_MASK                           0x00000000ffff0000


 

#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_47_32_OFFSET                         0x0000000000000030
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_47_32_LSB                            32
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_47_32_MSB                            47
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_47_32_MASK                           0x0000ffff00000000


 

#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_63_48_OFFSET                         0x0000000000000030
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_63_48_LSB                            48
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_63_48_MSB                            63
#define TX_FES_STATUS_END_PHYTX_TX_END_SW_INFO_63_48_MASK                           0xffff000000000000


 

#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_15_0_OFFSET                   0x0000000000000038
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_15_0_LSB                      0
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_15_0_MSB                      15
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_15_0_MASK                     0x000000000000ffff


 

#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_31_16_OFFSET                  0x0000000000000038
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_31_16_LSB                     16
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_31_16_MSB                     31
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_31_16_MASK                    0x00000000ffff0000


 

#define TX_FES_STATUS_END_CBF_SEGMENT_REQUEST_MASK_OFFSET                           0x0000000000000038
#define TX_FES_STATUS_END_CBF_SEGMENT_REQUEST_MASK_LSB                              32
#define TX_FES_STATUS_END_CBF_SEGMENT_REQUEST_MASK_MSB                              39
#define TX_FES_STATUS_END_CBF_SEGMENT_REQUEST_MASK_MASK                             0x000000ff00000000


 

#define TX_FES_STATUS_END_CBF_SEGMENT_SENT_MASK_OFFSET                              0x0000000000000038
#define TX_FES_STATUS_END_CBF_SEGMENT_SENT_MASK_LSB                                 40
#define TX_FES_STATUS_END_CBF_SEGMENT_SENT_MASK_MSB                                 47
#define TX_FES_STATUS_END_CBF_SEGMENT_SENT_MASK_MASK                                0x0000ff0000000000


 

#define TX_FES_STATUS_END_HIGHEST_ACHIEVED_DATA_NULL_RATIO_OFFSET                   0x0000000000000038
#define TX_FES_STATUS_END_HIGHEST_ACHIEVED_DATA_NULL_RATIO_LSB                      48
#define TX_FES_STATUS_END_HIGHEST_ACHIEVED_DATA_NULL_RATIO_MSB                      52
#define TX_FES_STATUS_END_HIGHEST_ACHIEVED_DATA_NULL_RATIO_MASK                     0x001f000000000000


 

#define TX_FES_STATUS_END_USE_ALT_POWER_SR_OFFSET                                   0x0000000000000038
#define TX_FES_STATUS_END_USE_ALT_POWER_SR_LSB                                      53
#define TX_FES_STATUS_END_USE_ALT_POWER_SR_MSB                                      53
#define TX_FES_STATUS_END_USE_ALT_POWER_SR_MASK                                     0x0020000000000000


 

#define TX_FES_STATUS_END_STATIC_2_PWR_MODE_STATUS_OFFSET                           0x0000000000000038
#define TX_FES_STATUS_END_STATIC_2_PWR_MODE_STATUS_LSB                              54
#define TX_FES_STATUS_END_STATIC_2_PWR_MODE_STATUS_MSB                              54
#define TX_FES_STATUS_END_STATIC_2_PWR_MODE_STATUS_MASK                             0x0040000000000000


 

#define TX_FES_STATUS_END_OBSS_SRG_OPPORT_TRANSMIT_STATUS_OFFSET                    0x0000000000000038
#define TX_FES_STATUS_END_OBSS_SRG_OPPORT_TRANSMIT_STATUS_LSB                       55
#define TX_FES_STATUS_END_OBSS_SRG_OPPORT_TRANSMIT_STATUS_MSB                       55
#define TX_FES_STATUS_END_OBSS_SRG_OPPORT_TRANSMIT_STATUS_MASK                      0x0080000000000000


 

#define TX_FES_STATUS_END_SRP_BASED_TRANSMIT_STATUS_OFFSET                          0x0000000000000038
#define TX_FES_STATUS_END_SRP_BASED_TRANSMIT_STATUS_LSB                             56
#define TX_FES_STATUS_END_SRP_BASED_TRANSMIT_STATUS_MSB                             56
#define TX_FES_STATUS_END_SRP_BASED_TRANSMIT_STATUS_MASK                            0x0100000000000000


 

#define TX_FES_STATUS_END_OBSS_PD_BASED_TRANSMIT_STATUS_OFFSET                      0x0000000000000038
#define TX_FES_STATUS_END_OBSS_PD_BASED_TRANSMIT_STATUS_LSB                         57
#define TX_FES_STATUS_END_OBSS_PD_BASED_TRANSMIT_STATUS_MSB                         57
#define TX_FES_STATUS_END_OBSS_PD_BASED_TRANSMIT_STATUS_MASK                        0x0200000000000000


 

#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_36_32_OFFSET                  0x0000000000000038
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_36_32_LSB                     58
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_36_32_MSB                     62
#define TX_FES_STATUS_END_BEAMFORM_MASKED_USER_BITMAP_36_32_MASK                    0x7c00000000000000


 

#define TX_FES_STATUS_END_PDG_MPDU_READY_OFFSET                                     0x0000000000000038
#define TX_FES_STATUS_END_PDG_MPDU_READY_LSB                                        63
#define TX_FES_STATUS_END_PDG_MPDU_READY_MSB                                        63
#define TX_FES_STATUS_END_PDG_MPDU_READY_MASK                                       0x8000000000000000


 

#define TX_FES_STATUS_END_PDG_MPDU_COUNT_OFFSET                                     0x0000000000000040
#define TX_FES_STATUS_END_PDG_MPDU_COUNT_LSB                                        0
#define TX_FES_STATUS_END_PDG_MPDU_COUNT_MSB                                        15
#define TX_FES_STATUS_END_PDG_MPDU_COUNT_MASK                                       0x000000000000ffff


 

#define TX_FES_STATUS_END_PDG_EST_MPDU_TX_COUNT_OFFSET                              0x0000000000000040
#define TX_FES_STATUS_END_PDG_EST_MPDU_TX_COUNT_LSB                                 16
#define TX_FES_STATUS_END_PDG_EST_MPDU_TX_COUNT_MSB                                 31
#define TX_FES_STATUS_END_PDG_EST_MPDU_TX_COUNT_MASK                                0x00000000ffff0000


 

#define TX_FES_STATUS_END_PDG_OVERVIEW_LENGTH_OFFSET                                0x0000000000000040
#define TX_FES_STATUS_END_PDG_OVERVIEW_LENGTH_LSB                                   32
#define TX_FES_STATUS_END_PDG_OVERVIEW_LENGTH_MSB                                   55
#define TX_FES_STATUS_END_PDG_OVERVIEW_LENGTH_MASK                                  0x00ffffff00000000


 

#define TX_FES_STATUS_END_TXOP_DURATION_OFFSET                                      0x0000000000000040
#define TX_FES_STATUS_END_TXOP_DURATION_LSB                                         56
#define TX_FES_STATUS_END_TXOP_DURATION_MSB                                         62
#define TX_FES_STATUS_END_TXOP_DURATION_MASK                                        0x7f00000000000000


 

#define TX_FES_STATUS_END_PDG_DROPPED_MPDU_WARNING_OFFSET                           0x0000000000000040
#define TX_FES_STATUS_END_PDG_DROPPED_MPDU_WARNING_LSB                              63
#define TX_FES_STATUS_END_PDG_DROPPED_MPDU_WARNING_MSB                              63
#define TX_FES_STATUS_END_PDG_DROPPED_MPDU_WARNING_MASK                             0x8000000000000000


 

#define TX_FES_STATUS_END_PACKET_EXTENSION_A_FACTOR_OFFSET                          0x0000000000000048
#define TX_FES_STATUS_END_PACKET_EXTENSION_A_FACTOR_LSB                             0
#define TX_FES_STATUS_END_PACKET_EXTENSION_A_FACTOR_MSB                             1
#define TX_FES_STATUS_END_PACKET_EXTENSION_A_FACTOR_MASK                            0x0000000000000003


 

#define TX_FES_STATUS_END_PACKET_EXTENSION_PE_DISAMBIGUITY_OFFSET                   0x0000000000000048
#define TX_FES_STATUS_END_PACKET_EXTENSION_PE_DISAMBIGUITY_LSB                      2
#define TX_FES_STATUS_END_PACKET_EXTENSION_PE_DISAMBIGUITY_MSB                      2
#define TX_FES_STATUS_END_PACKET_EXTENSION_PE_DISAMBIGUITY_MASK                     0x0000000000000004


 

#define TX_FES_STATUS_END_PACKET_EXTENSION_OFFSET                                   0x0000000000000048
#define TX_FES_STATUS_END_PACKET_EXTENSION_LSB                                      3
#define TX_FES_STATUS_END_PACKET_EXTENSION_MSB                                      5
#define TX_FES_STATUS_END_PACKET_EXTENSION_MASK                                     0x0000000000000038


 

#define TX_FES_STATUS_END_FEC_TYPE_OFFSET                                           0x0000000000000048
#define TX_FES_STATUS_END_FEC_TYPE_LSB                                              6
#define TX_FES_STATUS_END_FEC_TYPE_MSB                                              6
#define TX_FES_STATUS_END_FEC_TYPE_MASK                                             0x0000000000000040


 

#define TX_FES_STATUS_END_STBC_OFFSET                                               0x0000000000000048
#define TX_FES_STATUS_END_STBC_LSB                                                  7
#define TX_FES_STATUS_END_STBC_MSB                                                  7
#define TX_FES_STATUS_END_STBC_MASK                                                 0x0000000000000080


 

#define TX_FES_STATUS_END_NUM_DATA_SYMBOLS_OFFSET                                   0x0000000000000048
#define TX_FES_STATUS_END_NUM_DATA_SYMBOLS_LSB                                      8
#define TX_FES_STATUS_END_NUM_DATA_SYMBOLS_MSB                                      23
#define TX_FES_STATUS_END_NUM_DATA_SYMBOLS_MASK                                     0x0000000000ffff00


 

#define TX_FES_STATUS_END_RU_SIZE_OFFSET                                            0x0000000000000048
#define TX_FES_STATUS_END_RU_SIZE_LSB                                               24
#define TX_FES_STATUS_END_RU_SIZE_MSB                                               27
#define TX_FES_STATUS_END_RU_SIZE_MASK                                              0x000000000f000000


 

#define TX_FES_STATUS_END_RESERVED_17A_OFFSET                                       0x0000000000000048
#define TX_FES_STATUS_END_RESERVED_17A_LSB                                          28
#define TX_FES_STATUS_END_RESERVED_17A_MSB                                          31
#define TX_FES_STATUS_END_RESERVED_17A_MASK                                         0x00000000f0000000


 

#define TX_FES_STATUS_END_NUM_LTF_SYMBOLS_OFFSET                                    0x0000000000000048
#define TX_FES_STATUS_END_NUM_LTF_SYMBOLS_LSB                                       32
#define TX_FES_STATUS_END_NUM_LTF_SYMBOLS_MSB                                       34
#define TX_FES_STATUS_END_NUM_LTF_SYMBOLS_MASK                                      0x0000000700000000


 

#define TX_FES_STATUS_END_LTF_SIZE_OFFSET                                           0x0000000000000048
#define TX_FES_STATUS_END_LTF_SIZE_LSB                                              35
#define TX_FES_STATUS_END_LTF_SIZE_MSB                                              36
#define TX_FES_STATUS_END_LTF_SIZE_MASK                                             0x0000001800000000


 

#define TX_FES_STATUS_END_CP_SETTING_OFFSET                                         0x0000000000000048
#define TX_FES_STATUS_END_CP_SETTING_LSB                                            37
#define TX_FES_STATUS_END_CP_SETTING_MSB                                            38
#define TX_FES_STATUS_END_CP_SETTING_MASK                                           0x0000006000000000


 

#define TX_FES_STATUS_END_RESERVED_18A_OFFSET                                       0x0000000000000048
#define TX_FES_STATUS_END_RESERVED_18A_LSB                                          39
#define TX_FES_STATUS_END_RESERVED_18A_MSB                                          43
#define TX_FES_STATUS_END_RESERVED_18A_MASK                                         0x00000f8000000000


 

#define TX_FES_STATUS_END_DCM_OFFSET                                                0x0000000000000048
#define TX_FES_STATUS_END_DCM_LSB                                                   44
#define TX_FES_STATUS_END_DCM_MSB                                                   44
#define TX_FES_STATUS_END_DCM_MASK                                                  0x0000100000000000


 

#define TX_FES_STATUS_END_LDPC_EXTRA_SYMBOL_OFFSET                                  0x0000000000000048
#define TX_FES_STATUS_END_LDPC_EXTRA_SYMBOL_LSB                                     45
#define TX_FES_STATUS_END_LDPC_EXTRA_SYMBOL_MSB                                     45
#define TX_FES_STATUS_END_LDPC_EXTRA_SYMBOL_MASK                                    0x0000200000000000


 

#define TX_FES_STATUS_END_FORCE_EXTRA_SYMBOL_OFFSET                                 0x0000000000000048
#define TX_FES_STATUS_END_FORCE_EXTRA_SYMBOL_LSB                                    46
#define TX_FES_STATUS_END_FORCE_EXTRA_SYMBOL_MSB                                    46
#define TX_FES_STATUS_END_FORCE_EXTRA_SYMBOL_MASK                                   0x0000400000000000


 

#define TX_FES_STATUS_END_RESERVED_18B_OFFSET                                       0x0000000000000048
#define TX_FES_STATUS_END_RESERVED_18B_LSB                                          47
#define TX_FES_STATUS_END_RESERVED_18B_MSB                                          47
#define TX_FES_STATUS_END_RESERVED_18B_MASK                                         0x0000800000000000


 

#define TX_FES_STATUS_END_TX_PWR_SHARED_OFFSET                                      0x0000000000000048
#define TX_FES_STATUS_END_TX_PWR_SHARED_LSB                                         48
#define TX_FES_STATUS_END_TX_PWR_SHARED_MSB                                         55
#define TX_FES_STATUS_END_TX_PWR_SHARED_MASK                                        0x00ff000000000000


 

#define TX_FES_STATUS_END_TX_PWR_UNSHARED_OFFSET                                    0x0000000000000048
#define TX_FES_STATUS_END_TX_PWR_UNSHARED_LSB                                       56
#define TX_FES_STATUS_END_TX_PWR_UNSHARED_MSB                                       63
#define TX_FES_STATUS_END_TX_PWR_UNSHARED_MASK                                      0xff00000000000000


 

#define TX_FES_STATUS_END_RANGING_ACTIVE_USER_MAP_OFFSET                            0x0000000000000050
#define TX_FES_STATUS_END_RANGING_ACTIVE_USER_MAP_LSB                               0
#define TX_FES_STATUS_END_RANGING_ACTIVE_USER_MAP_MSB                               15
#define TX_FES_STATUS_END_RANGING_ACTIVE_USER_MAP_MASK                              0x000000000000ffff


 

#define TX_FES_STATUS_END_RANGING_SENT_DUMMY_TX_OFFSET                              0x0000000000000050
#define TX_FES_STATUS_END_RANGING_SENT_DUMMY_TX_LSB                                 16
#define TX_FES_STATUS_END_RANGING_SENT_DUMMY_TX_MSB                                 16
#define TX_FES_STATUS_END_RANGING_SENT_DUMMY_TX_MASK                                0x0000000000010000


 

#define TX_FES_STATUS_END_RANGING_FTM_FRAME_SENT_OFFSET                             0x0000000000000050
#define TX_FES_STATUS_END_RANGING_FTM_FRAME_SENT_LSB                                17
#define TX_FES_STATUS_END_RANGING_FTM_FRAME_SENT_MSB                                17
#define TX_FES_STATUS_END_RANGING_FTM_FRAME_SENT_MASK                               0x0000000000020000


 

#define TX_FES_STATUS_END_RESERVED_20A_OFFSET                                       0x0000000000000050
#define TX_FES_STATUS_END_RESERVED_20A_LSB                                          18
#define TX_FES_STATUS_END_RESERVED_20A_MSB                                          23
#define TX_FES_STATUS_END_RESERVED_20A_MASK                                         0x0000000000fc0000


 

#define TX_FES_STATUS_END_CV_CORR_STATUS_OFFSET                                     0x0000000000000050
#define TX_FES_STATUS_END_CV_CORR_STATUS_LSB                                        24
#define TX_FES_STATUS_END_CV_CORR_STATUS_MSB                                        31
#define TX_FES_STATUS_END_CV_CORR_STATUS_MASK                                       0x00000000ff000000


 

#define TX_FES_STATUS_END_CURRENT_TX_DURATION_OFFSET                                0x0000000000000050
#define TX_FES_STATUS_END_CURRENT_TX_DURATION_LSB                                   32
#define TX_FES_STATUS_END_CURRENT_TX_DURATION_MSB                                   47
#define TX_FES_STATUS_END_CURRENT_TX_DURATION_MASK                                  0x0000ffff00000000


 

#define TX_FES_STATUS_END_RESERVED_21A_OFFSET                                       0x0000000000000050
#define TX_FES_STATUS_END_RESERVED_21A_LSB                                          48
#define TX_FES_STATUS_END_RESERVED_21A_MSB                                          63
#define TX_FES_STATUS_END_RESERVED_21A_MASK                                         0xffff000000000000



#endif    
