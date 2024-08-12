
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _PCU_PPDU_SETUP_INIT_H_
#define _PCU_PPDU_SETUP_INIT_H_
#if !defined(__ASSEMBLER__)
#endif

#include "pdg_response_rate_setting.h"
#define NUM_OF_DWORDS_PCU_PPDU_SETUP_INIT 58

#define NUM_OF_QWORDS_PCU_PPDU_SETUP_INIT 29


struct pcu_ppdu_setup_init {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t medium_prot_type                                        :  3,  
                      response_type                                           :  5,  
                      response_info_part2_required                            :  1,  
                      response_to_response                                    :  3,  
                      mba_user_order                                          :  2,  
                      expected_mba_size                                       : 11,  
                      required_ul_mu_resp_user_count                          :  6,  
                      transmitted_bssid_check_en                              :  1;  
             uint32_t mprot_required_bw1                                      :  1,  
                      mprot_required_bw20                                     :  1,  
                      mprot_required_bw40                                     :  1,  
                      mprot_required_bw80                                     :  1,  
                      mprot_required_bw160                                    :  1,  
                      mprot_required_bw240                                    :  1,  
                      mprot_required_bw320                                    :  1,  
                      ppdu_allowed_bw1                                        :  1,  
                      ppdu_allowed_bw20                                       :  1,  
                      ppdu_allowed_bw40                                       :  1,  
                      ppdu_allowed_bw80                                       :  1,  
                      ppdu_allowed_bw160                                      :  1,  
                      ppdu_allowed_bw240                                      :  1,  
                      ppdu_allowed_bw320                                      :  1,  
                      set_fc_pwr_mgt                                          :  1,  
                      use_cts_duration_for_data_tx                            :  1,  
                      update_timestamp_64                                     :  1,  
                      update_timestamp_32_lower                               :  1,  
                      update_timestamp_32_upper                               :  1,  
                      reserved_1a                                             : 13;  
             uint32_t insert_timestamp_offset_0                               : 16,  
                      insert_timestamp_offset_1                               : 16;  
             uint32_t max_bw40_try_count                                      :  4,  
                      max_bw80_try_count                                      :  4,  
                      max_bw160_try_count                                     :  4,  
                      max_bw240_try_count                                     :  4,  
                      max_bw320_try_count                                     :  4,  
                      insert_wur_timestamp_offset                             :  6,  
                      update_wur_timestamp                                    :  1,  
                      wur_embedded_bssid_present                              :  1,  
                      insert_wur_fcs                                          :  1,  
                      reserved_3b                                             :  3;  
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw20;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw40;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw80;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw160;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw240;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw320;
             uint32_t r2r_hw_response_tx_duration                             : 16,  
                      r2r_rx_duration_field                                   : 16;  
             uint32_t r2r_group_id                                            :  6,  
                      r2r_response_frame_type                                 :  4,  
                      r2r_sta_partial_aid                                     : 11,  
                      use_address_fields_for_protection                       :  1,  
                      r2r_set_required_response_time                          :  1,  
                      reserved_29a                                            :  3,  
                      r2r_bw20_active_channel                                 :  3,  
                      r2r_bw40_active_channel                                 :  3;  
             uint32_t r2r_bw80_active_channel                                 :  3,  
                      r2r_bw160_active_channel                                :  3,  
                      r2r_bw240_active_channel                                :  3,  
                      r2r_bw320_active_channel                                :  3,  
                      r2r_bw20                                                :  3,  
                      r2r_bw40                                                :  3,  
                      r2r_bw80                                                :  3,  
                      r2r_bw160                                               :  3,  
                      r2r_bw240                                               :  3,  
                      r2r_bw320                                               :  3,  
                      reserved_30a                                            :  2;  
             uint32_t mu_response_expected_bitmap_31_0                        : 32;  
             uint32_t mu_response_expected_bitmap_36_32                       :  5,  
                      mu_expected_response_cbf_count                          :  6,  
                      mu_expected_response_sta_count                          :  6,  
                      transmit_includes_multidestination                      :  1,  
                      insert_prev_tx_start_timing_info                        :  1,  
                      insert_current_tx_start_timing_info                     :  1,  
                      tx_start_transmit_time_byte_offset                      : 12;  
             uint32_t protection_frame_ad1_31_0                               : 32;  
             uint32_t protection_frame_ad1_47_32                              : 16,  
                      protection_frame_ad2_15_0                               : 16;  
             uint32_t protection_frame_ad2_47_16                              : 32;  
             uint32_t dynamic_medium_prot_threshold                           : 24,  
                      dynamic_medium_prot_type                                :  1,  
                      reserved_54a                                            :  7;  
             uint32_t protection_frame_ad3_31_0                               : 32;  
             uint32_t protection_frame_ad3_47_32                              : 16,  
                      protection_frame_ad4_15_0                               : 16;  
             uint32_t protection_frame_ad4_47_16                              : 32;  
#else
             uint32_t transmitted_bssid_check_en                              :  1,  
                      required_ul_mu_resp_user_count                          :  6,  
                      expected_mba_size                                       : 11,  
                      mba_user_order                                          :  2,  
                      response_to_response                                    :  3,  
                      response_info_part2_required                            :  1,  
                      response_type                                           :  5,  
                      medium_prot_type                                        :  3;  
             uint32_t reserved_1a                                             : 13,  
                      update_timestamp_32_upper                               :  1,  
                      update_timestamp_32_lower                               :  1,  
                      update_timestamp_64                                     :  1,  
                      use_cts_duration_for_data_tx                            :  1,  
                      set_fc_pwr_mgt                                          :  1,  
                      ppdu_allowed_bw320                                      :  1,  
                      ppdu_allowed_bw240                                      :  1,  
                      ppdu_allowed_bw160                                      :  1,  
                      ppdu_allowed_bw80                                       :  1,  
                      ppdu_allowed_bw40                                       :  1,  
                      ppdu_allowed_bw20                                       :  1,  
                      ppdu_allowed_bw1                                        :  1,  
                      mprot_required_bw320                                    :  1,  
                      mprot_required_bw240                                    :  1,  
                      mprot_required_bw160                                    :  1,  
                      mprot_required_bw80                                     :  1,  
                      mprot_required_bw40                                     :  1,  
                      mprot_required_bw20                                     :  1,  
                      mprot_required_bw1                                      :  1;  
             uint32_t insert_timestamp_offset_1                               : 16,  
                      insert_timestamp_offset_0                               : 16;  
             uint32_t reserved_3b                                             :  3,  
                      insert_wur_fcs                                          :  1,  
                      wur_embedded_bssid_present                              :  1,  
                      update_wur_timestamp                                    :  1,  
                      insert_wur_timestamp_offset                             :  6,  
                      max_bw320_try_count                                     :  4,  
                      max_bw240_try_count                                     :  4,  
                      max_bw160_try_count                                     :  4,  
                      max_bw80_try_count                                      :  4,  
                      max_bw40_try_count                                      :  4;  
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw20;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw40;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw80;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw160;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw240;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw320;
             uint32_t r2r_rx_duration_field                                   : 16,  
                      r2r_hw_response_tx_duration                             : 16;  
             uint32_t r2r_bw40_active_channel                                 :  3,  
                      r2r_bw20_active_channel                                 :  3,  
                      reserved_29a                                            :  3,  
                      r2r_set_required_response_time                          :  1,  
                      use_address_fields_for_protection                       :  1,  
                      r2r_sta_partial_aid                                     : 11,  
                      r2r_response_frame_type                                 :  4,  
                      r2r_group_id                                            :  6;  
             uint32_t reserved_30a                                            :  2,  
                      r2r_bw320                                               :  3,  
                      r2r_bw240                                               :  3,  
                      r2r_bw160                                               :  3,  
                      r2r_bw80                                                :  3,  
                      r2r_bw40                                                :  3,  
                      r2r_bw20                                                :  3,  
                      r2r_bw320_active_channel                                :  3,  
                      r2r_bw240_active_channel                                :  3,  
                      r2r_bw160_active_channel                                :  3,  
                      r2r_bw80_active_channel                                 :  3;  
             uint32_t mu_response_expected_bitmap_31_0                        : 32;  
             uint32_t tx_start_transmit_time_byte_offset                      : 12,  
                      insert_current_tx_start_timing_info                     :  1,  
                      insert_prev_tx_start_timing_info                        :  1,  
                      transmit_includes_multidestination                      :  1,  
                      mu_expected_response_sta_count                          :  6,  
                      mu_expected_response_cbf_count                          :  6,  
                      mu_response_expected_bitmap_36_32                       :  5;  
             uint32_t protection_frame_ad1_31_0                               : 32;  
             uint32_t protection_frame_ad2_15_0                               : 16,  
                      protection_frame_ad1_47_32                              : 16;  
             uint32_t protection_frame_ad2_47_16                              : 32;  
             uint32_t reserved_54a                                            :  7,  
                      dynamic_medium_prot_type                                :  1,  
                      dynamic_medium_prot_threshold                           : 24;  
             uint32_t protection_frame_ad3_31_0                               : 32;  
             uint32_t protection_frame_ad4_15_0                               : 16,  
                      protection_frame_ad3_47_32                              : 16;  
             uint32_t protection_frame_ad4_47_16                              : 32;  
#endif
};


 

#define PCU_PPDU_SETUP_INIT_MEDIUM_PROT_TYPE_OFFSET                                 0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MEDIUM_PROT_TYPE_LSB                                    0
#define PCU_PPDU_SETUP_INIT_MEDIUM_PROT_TYPE_MSB                                    2
#define PCU_PPDU_SETUP_INIT_MEDIUM_PROT_TYPE_MASK                                   0x0000000000000007


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TYPE_OFFSET                                    0x0000000000000000
#define PCU_PPDU_SETUP_INIT_RESPONSE_TYPE_LSB                                       3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TYPE_MSB                                       7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TYPE_MASK                                      0x00000000000000f8


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_INFO_PART2_REQUIRED_OFFSET                     0x0000000000000000
#define PCU_PPDU_SETUP_INIT_RESPONSE_INFO_PART2_REQUIRED_LSB                        8
#define PCU_PPDU_SETUP_INIT_RESPONSE_INFO_PART2_REQUIRED_MSB                        8
#define PCU_PPDU_SETUP_INIT_RESPONSE_INFO_PART2_REQUIRED_MASK                       0x0000000000000100


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_OFFSET                             0x0000000000000000
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_LSB                                9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_MSB                                11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_MASK                               0x0000000000000e00


 

#define PCU_PPDU_SETUP_INIT_MBA_USER_ORDER_OFFSET                                   0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MBA_USER_ORDER_LSB                                      12
#define PCU_PPDU_SETUP_INIT_MBA_USER_ORDER_MSB                                      13
#define PCU_PPDU_SETUP_INIT_MBA_USER_ORDER_MASK                                     0x0000000000003000


 

#define PCU_PPDU_SETUP_INIT_EXPECTED_MBA_SIZE_OFFSET                                0x0000000000000000
#define PCU_PPDU_SETUP_INIT_EXPECTED_MBA_SIZE_LSB                                   14
#define PCU_PPDU_SETUP_INIT_EXPECTED_MBA_SIZE_MSB                                   24
#define PCU_PPDU_SETUP_INIT_EXPECTED_MBA_SIZE_MASK                                  0x0000000001ffc000


 

#define PCU_PPDU_SETUP_INIT_REQUIRED_UL_MU_RESP_USER_COUNT_OFFSET                   0x0000000000000000
#define PCU_PPDU_SETUP_INIT_REQUIRED_UL_MU_RESP_USER_COUNT_LSB                      25
#define PCU_PPDU_SETUP_INIT_REQUIRED_UL_MU_RESP_USER_COUNT_MSB                      30
#define PCU_PPDU_SETUP_INIT_REQUIRED_UL_MU_RESP_USER_COUNT_MASK                     0x000000007e000000


 

#define PCU_PPDU_SETUP_INIT_TRANSMITTED_BSSID_CHECK_EN_OFFSET                       0x0000000000000000
#define PCU_PPDU_SETUP_INIT_TRANSMITTED_BSSID_CHECK_EN_LSB                          31
#define PCU_PPDU_SETUP_INIT_TRANSMITTED_BSSID_CHECK_EN_MSB                          31
#define PCU_PPDU_SETUP_INIT_TRANSMITTED_BSSID_CHECK_EN_MASK                         0x0000000080000000


 

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW1_OFFSET                               0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW1_LSB                                  32
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW1_MSB                                  32
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW1_MASK                                 0x0000000100000000


 

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW20_OFFSET                              0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW20_LSB                                 33
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW20_MSB                                 33
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW20_MASK                                0x0000000200000000


 

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW40_OFFSET                              0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW40_LSB                                 34
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW40_MSB                                 34
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW40_MASK                                0x0000000400000000


 

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW80_OFFSET                              0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW80_LSB                                 35
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW80_MSB                                 35
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW80_MASK                                0x0000000800000000


 

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW160_OFFSET                             0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW160_LSB                                36
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW160_MSB                                36
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW160_MASK                               0x0000001000000000


 

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW240_OFFSET                             0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW240_LSB                                37
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW240_MSB                                37
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW240_MASK                               0x0000002000000000


 

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW320_OFFSET                             0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW320_LSB                                38
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW320_MSB                                38
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW320_MASK                               0x0000004000000000


 

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW1_OFFSET                                 0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW1_LSB                                    39
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW1_MSB                                    39
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW1_MASK                                   0x0000008000000000


 

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW20_OFFSET                                0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW20_LSB                                   40
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW20_MSB                                   40
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW20_MASK                                  0x0000010000000000


 

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW40_OFFSET                                0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW40_LSB                                   41
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW40_MSB                                   41
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW40_MASK                                  0x0000020000000000


 

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW80_OFFSET                                0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW80_LSB                                   42
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW80_MSB                                   42
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW80_MASK                                  0x0000040000000000


 

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW160_OFFSET                               0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW160_LSB                                  43
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW160_MSB                                  43
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW160_MASK                                 0x0000080000000000


 

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW240_OFFSET                               0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW240_LSB                                  44
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW240_MSB                                  44
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW240_MASK                                 0x0000100000000000


 

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW320_OFFSET                               0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW320_LSB                                  45
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW320_MSB                                  45
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW320_MASK                                 0x0000200000000000


 

#define PCU_PPDU_SETUP_INIT_SET_FC_PWR_MGT_OFFSET                                   0x0000000000000000
#define PCU_PPDU_SETUP_INIT_SET_FC_PWR_MGT_LSB                                      46
#define PCU_PPDU_SETUP_INIT_SET_FC_PWR_MGT_MSB                                      46
#define PCU_PPDU_SETUP_INIT_SET_FC_PWR_MGT_MASK                                     0x0000400000000000


 

#define PCU_PPDU_SETUP_INIT_USE_CTS_DURATION_FOR_DATA_TX_OFFSET                     0x0000000000000000
#define PCU_PPDU_SETUP_INIT_USE_CTS_DURATION_FOR_DATA_TX_LSB                        47
#define PCU_PPDU_SETUP_INIT_USE_CTS_DURATION_FOR_DATA_TX_MSB                        47
#define PCU_PPDU_SETUP_INIT_USE_CTS_DURATION_FOR_DATA_TX_MASK                       0x0000800000000000


 

#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_64_OFFSET                              0x0000000000000000
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_64_LSB                                 48
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_64_MSB                                 48
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_64_MASK                                0x0001000000000000


 

#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_LOWER_OFFSET                        0x0000000000000000
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_LOWER_LSB                           49
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_LOWER_MSB                           49
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_LOWER_MASK                          0x0002000000000000


 

#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_UPPER_OFFSET                        0x0000000000000000
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_UPPER_LSB                           50
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_UPPER_MSB                           50
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_UPPER_MASK                          0x0004000000000000


 

#define PCU_PPDU_SETUP_INIT_RESERVED_1A_OFFSET                                      0x0000000000000000
#define PCU_PPDU_SETUP_INIT_RESERVED_1A_LSB                                         51
#define PCU_PPDU_SETUP_INIT_RESERVED_1A_MSB                                         63
#define PCU_PPDU_SETUP_INIT_RESERVED_1A_MASK                                        0xfff8000000000000


 

#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_0_OFFSET                        0x0000000000000008
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_0_LSB                           0
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_0_MSB                           15
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_0_MASK                          0x000000000000ffff


 

#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_1_OFFSET                        0x0000000000000008
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_1_LSB                           16
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_1_MSB                           31
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_1_MASK                          0x00000000ffff0000


 

#define PCU_PPDU_SETUP_INIT_MAX_BW40_TRY_COUNT_OFFSET                               0x0000000000000008
#define PCU_PPDU_SETUP_INIT_MAX_BW40_TRY_COUNT_LSB                                  32
#define PCU_PPDU_SETUP_INIT_MAX_BW40_TRY_COUNT_MSB                                  35
#define PCU_PPDU_SETUP_INIT_MAX_BW40_TRY_COUNT_MASK                                 0x0000000f00000000


 

#define PCU_PPDU_SETUP_INIT_MAX_BW80_TRY_COUNT_OFFSET                               0x0000000000000008
#define PCU_PPDU_SETUP_INIT_MAX_BW80_TRY_COUNT_LSB                                  36
#define PCU_PPDU_SETUP_INIT_MAX_BW80_TRY_COUNT_MSB                                  39
#define PCU_PPDU_SETUP_INIT_MAX_BW80_TRY_COUNT_MASK                                 0x000000f000000000


 

#define PCU_PPDU_SETUP_INIT_MAX_BW160_TRY_COUNT_OFFSET                              0x0000000000000008
#define PCU_PPDU_SETUP_INIT_MAX_BW160_TRY_COUNT_LSB                                 40
#define PCU_PPDU_SETUP_INIT_MAX_BW160_TRY_COUNT_MSB                                 43
#define PCU_PPDU_SETUP_INIT_MAX_BW160_TRY_COUNT_MASK                                0x00000f0000000000


 

#define PCU_PPDU_SETUP_INIT_MAX_BW240_TRY_COUNT_OFFSET                              0x0000000000000008
#define PCU_PPDU_SETUP_INIT_MAX_BW240_TRY_COUNT_LSB                                 44
#define PCU_PPDU_SETUP_INIT_MAX_BW240_TRY_COUNT_MSB                                 47
#define PCU_PPDU_SETUP_INIT_MAX_BW240_TRY_COUNT_MASK                                0x0000f00000000000


 

#define PCU_PPDU_SETUP_INIT_MAX_BW320_TRY_COUNT_OFFSET                              0x0000000000000008
#define PCU_PPDU_SETUP_INIT_MAX_BW320_TRY_COUNT_LSB                                 48
#define PCU_PPDU_SETUP_INIT_MAX_BW320_TRY_COUNT_MSB                                 51
#define PCU_PPDU_SETUP_INIT_MAX_BW320_TRY_COUNT_MASK                                0x000f000000000000


 

#define PCU_PPDU_SETUP_INIT_INSERT_WUR_TIMESTAMP_OFFSET_OFFSET                      0x0000000000000008
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_TIMESTAMP_OFFSET_LSB                         52
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_TIMESTAMP_OFFSET_MSB                         57
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_TIMESTAMP_OFFSET_MASK                        0x03f0000000000000


 

#define PCU_PPDU_SETUP_INIT_UPDATE_WUR_TIMESTAMP_OFFSET                             0x0000000000000008
#define PCU_PPDU_SETUP_INIT_UPDATE_WUR_TIMESTAMP_LSB                                58
#define PCU_PPDU_SETUP_INIT_UPDATE_WUR_TIMESTAMP_MSB                                58
#define PCU_PPDU_SETUP_INIT_UPDATE_WUR_TIMESTAMP_MASK                               0x0400000000000000


 

#define PCU_PPDU_SETUP_INIT_WUR_EMBEDDED_BSSID_PRESENT_OFFSET                       0x0000000000000008
#define PCU_PPDU_SETUP_INIT_WUR_EMBEDDED_BSSID_PRESENT_LSB                          59
#define PCU_PPDU_SETUP_INIT_WUR_EMBEDDED_BSSID_PRESENT_MSB                          59
#define PCU_PPDU_SETUP_INIT_WUR_EMBEDDED_BSSID_PRESENT_MASK                         0x0800000000000000


 

#define PCU_PPDU_SETUP_INIT_INSERT_WUR_FCS_OFFSET                                   0x0000000000000008
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_FCS_LSB                                      60
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_FCS_MSB                                      60
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_FCS_MASK                                     0x1000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESERVED_3B_OFFSET                                      0x0000000000000008
#define PCU_PPDU_SETUP_INIT_RESERVED_3B_LSB                                         61
#define PCU_PPDU_SETUP_INIT_RESERVED_3B_MSB                                         63
#define PCU_PPDU_SETUP_INIT_RESERVED_3B_MASK                                        0xe000000000000000


 


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_0A_OFFSET  0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_0A_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_0A_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_0A_MASK    0x0000000000000001


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_ANTENNA_SECTOR_CTRL_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_ANTENNA_SECTOR_CTRL_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_ANTENNA_SECTOR_CTRL_MASK 0x0000000001fffffe


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_PKT_TYPE_OFFSET     0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_PKT_TYPE_LSB        25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_PKT_TYPE_MSB        28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_PKT_TYPE_MASK       0x000000001e000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SMOOTHING_OFFSET    0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SMOOTHING_LSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SMOOTHING_MSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SMOOTHING_MASK      0x0000000020000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_LDPC_OFFSET         0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_LDPC_LSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_LDPC_MSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_LDPC_MASK           0x0000000040000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STBC_OFFSET         0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STBC_LSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STBC_MSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STBC_MASK           0x0000000080000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_OFFSET   0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_LSB      32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_MSB      39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_MASK     0x000000ff00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_MIN_TX_PWR_OFFSET 0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_MIN_TX_PWR_LSB  40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_MIN_TX_PWR_MSB  47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_MIN_TX_PWR_MASK 0x0000ff0000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_NSS_OFFSET      0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_NSS_LSB         48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_NSS_MSB         50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_NSS_MASK        0x0007000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_CHAIN_MASK_OFFSET 0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_CHAIN_MASK_LSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_CHAIN_MASK_MSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_CHAIN_MASK_MASK 0x07f8000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_BW_OFFSET       0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_BW_LSB          59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_BW_MSB          61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_BW_MASK         0x3800000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STF_LTF_3DB_BOOST_OFFSET 0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STF_LTF_3DB_BOOST_LSB 62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STF_LTF_3DB_BOOST_MSB 62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STF_LTF_3DB_BOOST_MASK 0x4000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_FORCE_EXTRA_SYMBOL_OFFSET 0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_FORCE_EXTRA_SYMBOL_LSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_FORCE_EXTRA_SYMBOL_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_FORCE_EXTRA_SYMBOL_MASK 0x8000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_RATE_MCS_OFFSET 0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_RATE_MCS_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_RATE_MCS_MSB    3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_RATE_MCS_MASK   0x000000000000000f


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NSS_OFFSET          0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NSS_LSB             4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NSS_MSB             6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NSS_MASK            0x0000000000000070


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DPD_ENABLE_OFFSET   0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DPD_ENABLE_LSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DPD_ENABLE_MSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DPD_ENABLE_MASK     0x0000000000000080


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_OFFSET       0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_LSB          8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_MSB          15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_MASK         0x000000000000ff00


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MIN_TX_PWR_OFFSET   0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MIN_TX_PWR_LSB      16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MIN_TX_PWR_MSB      23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MIN_TX_PWR_MASK     0x0000000000ff0000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_CHAIN_MASK_OFFSET 0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_CHAIN_MASK_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_CHAIN_MASK_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_CHAIN_MASK_MASK  0x00000000ff000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3A_OFFSET  0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3A_LSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3A_MSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3A_MASK    0x000000ff00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SGI_OFFSET          0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SGI_LSB             40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SGI_MSB             41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SGI_MASK            0x0000030000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RATE_MCS_OFFSET     0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RATE_MCS_LSB        42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RATE_MCS_MSB        45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RATE_MCS_MASK       0x00003c0000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3B_OFFSET  0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3B_LSB     46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3B_MSB     47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3B_MASK    0x0000c00000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_1_OFFSET     0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_1_LSB        48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_1_MSB        55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_1_MASK       0x00ff000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_1_OFFSET 0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_1_LSB    56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_1_MSB    63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_1_MASK   0xff00000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_AGGREGATION_OFFSET  0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_AGGREGATION_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_AGGREGATION_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_AGGREGATION_MASK    0x0000000000000001


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_BSS_COLOR_ID_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_BSS_COLOR_ID_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_BSS_COLOR_ID_MSB 6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_BSS_COLOR_ID_MASK 0x000000000000007e


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SPATIAL_REUSE_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SPATIAL_REUSE_LSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SPATIAL_REUSE_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SPATIAL_REUSE_MASK 0x0000000000000780


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CP_LTF_SIZE_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CP_LTF_SIZE_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CP_LTF_SIZE_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CP_LTF_SIZE_MASK 0x0000000000001800


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DCM_OFFSET  0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DCM_LSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DCM_MSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DCM_MASK    0x0000000000002000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DOPPLER_INDICATION_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DOPPLER_INDICATION_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DOPPLER_INDICATION_MSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DOPPLER_INDICATION_MASK 0x0000000000004000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SU_EXTENDED_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SU_EXTENDED_LSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SU_EXTENDED_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SU_EXTENDED_MASK 0x0000000000008000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_MIN_PACKET_EXTENSION_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_MIN_PACKET_EXTENSION_MSB 17
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x0000000000030000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_NSS_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_NSS_LSB  18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_NSS_MSB  20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_NSS_MASK 0x00000000001c0000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CONTENT_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CONTENT_LSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CONTENT_MSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CONTENT_MASK 0x0000000000200000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_LTF_SIZE_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_LTF_SIZE_LSB 22
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_LTF_SIZE_MSB 23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_LTF_SIZE_MASK 0x0000000000c00000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CHAIN_CSD_EN_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CHAIN_CSD_EN_LSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CHAIN_CSD_EN_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CHAIN_CSD_EN_MASK 0x0000000001000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CHAIN_CSD_EN_LSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CHAIN_CSD_EN_MSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x0000000002000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DL_UL_FLAG_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DL_UL_FLAG_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DL_UL_FLAG_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DL_UL_FLAG_MASK 0x0000000004000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_4A_OFFSET  0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_4A_LSB     27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_4A_MSB     31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_4A_MASK    0x00000000f8000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_START_INDEX_LSB 32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_START_INDEX_MSB 35
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_START_INDEX_MASK 0x0000000f00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_SIZE_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_SIZE_LSB 36
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_SIZE_MSB 39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_SIZE_MASK 0x000000f000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_EHT_DUPLICATE_MODE_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_EHT_DUPLICATE_MODE_LSB 40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_EHT_DUPLICATE_MODE_MSB 41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_EHT_DUPLICATE_MODE_MASK 0x0000030000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_DCM_OFFSET  0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_DCM_LSB     42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_DCM_MSB     42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_DCM_MASK    0x0000040000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_0_MCS_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_0_MCS_LSB   43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_0_MCS_MSB   45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_0_MCS_MASK  0x0000380000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NUM_HE_SIGB_SYM_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NUM_HE_SIGB_SYM_LSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NUM_HE_SIGB_SYM_MSB 50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NUM_HE_SIGB_SYM_MASK 0x0007c00000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_SOURCE_LSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_SOURCE_MSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x0008000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_5A_OFFSET  0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_5A_LSB     52
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_5A_MSB     57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_5A_MASK    0x03f0000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0xfc00000000000000


 


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x00000000000003ff


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000000000000400


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000000000000800


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000000000001000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x000000000000e000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_MSB 27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_MASK 0x000000000fff0000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11BE_PARAMS_PLACEHOLDER_LSB 28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11BE_PARAMS_PLACEHOLDER_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11BE_PARAMS_PLACEHOLDER_MASK 0x00000000f0000000


 


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_0A_OFFSET  0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_0A_LSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_0A_MSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_0A_MASK    0x0000000100000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_ANTENNA_SECTOR_CTRL_LSB 33
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_ANTENNA_SECTOR_CTRL_MSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_ANTENNA_SECTOR_CTRL_MASK 0x01fffffe00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_PKT_TYPE_OFFSET     0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_PKT_TYPE_LSB        57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_PKT_TYPE_MSB        60
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_PKT_TYPE_MASK       0x1e00000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SMOOTHING_OFFSET    0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SMOOTHING_LSB       61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SMOOTHING_MSB       61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SMOOTHING_MASK      0x2000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_LDPC_OFFSET         0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_LDPC_LSB            62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_LDPC_MSB            62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_LDPC_MASK           0x4000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STBC_OFFSET         0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STBC_LSB            63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STBC_MSB            63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STBC_MASK           0x8000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_OFFSET   0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_LSB      0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_MSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_MASK     0x00000000000000ff


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_MIN_TX_PWR_OFFSET 0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_MIN_TX_PWR_LSB  8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_MIN_TX_PWR_MSB  15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_MIN_TX_PWR_MASK 0x000000000000ff00


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_NSS_OFFSET      0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_NSS_LSB         16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_NSS_MSB         18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_NSS_MASK        0x0000000000070000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_CHAIN_MASK_OFFSET 0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_CHAIN_MASK_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_CHAIN_MASK_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_CHAIN_MASK_MASK 0x0000000007f80000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_BW_OFFSET       0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_BW_LSB          27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_BW_MSB          29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_BW_MASK         0x0000000038000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STF_LTF_3DB_BOOST_OFFSET 0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STF_LTF_3DB_BOOST_LSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STF_LTF_3DB_BOOST_MSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STF_LTF_3DB_BOOST_MASK 0x0000000040000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_FORCE_EXTRA_SYMBOL_OFFSET 0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_FORCE_EXTRA_SYMBOL_LSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_FORCE_EXTRA_SYMBOL_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_FORCE_EXTRA_SYMBOL_MASK 0x0000000080000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_RATE_MCS_OFFSET 0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_RATE_MCS_LSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_RATE_MCS_MSB    35
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_RATE_MCS_MASK   0x0000000f00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NSS_OFFSET          0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NSS_LSB             36
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NSS_MSB             38
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NSS_MASK            0x0000007000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DPD_ENABLE_OFFSET   0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DPD_ENABLE_LSB      39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DPD_ENABLE_MSB      39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DPD_ENABLE_MASK     0x0000008000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_OFFSET       0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_LSB          40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_MSB          47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_MASK         0x0000ff0000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MIN_TX_PWR_OFFSET   0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MIN_TX_PWR_LSB      48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MIN_TX_PWR_MSB      55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MIN_TX_PWR_MASK     0x00ff000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_CHAIN_MASK_OFFSET 0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_CHAIN_MASK_LSB   56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_CHAIN_MASK_MSB   63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_CHAIN_MASK_MASK  0xff00000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3A_OFFSET  0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3A_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3A_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3A_MASK    0x00000000000000ff


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SGI_OFFSET          0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SGI_LSB             8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SGI_MSB             9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SGI_MASK            0x0000000000000300


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RATE_MCS_OFFSET     0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RATE_MCS_LSB        10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RATE_MCS_MSB        13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RATE_MCS_MASK       0x0000000000003c00


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3B_OFFSET  0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3B_LSB     14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3B_MSB     15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3B_MASK    0x000000000000c000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_1_OFFSET     0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_1_LSB        16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_1_MSB        23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_1_MASK       0x0000000000ff0000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_1_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_1_LSB    24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_1_MSB    31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_1_MASK   0x00000000ff000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_AGGREGATION_OFFSET  0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_AGGREGATION_LSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_AGGREGATION_MSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_AGGREGATION_MASK    0x0000000100000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_BSS_COLOR_ID_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_BSS_COLOR_ID_LSB 33
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_BSS_COLOR_ID_MSB 38
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_BSS_COLOR_ID_MASK 0x0000007e00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SPATIAL_REUSE_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SPATIAL_REUSE_LSB 39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SPATIAL_REUSE_MSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SPATIAL_REUSE_MASK 0x0000078000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CP_LTF_SIZE_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CP_LTF_SIZE_LSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CP_LTF_SIZE_MSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CP_LTF_SIZE_MASK 0x0000180000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DCM_OFFSET  0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DCM_LSB     45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DCM_MSB     45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DCM_MASK    0x0000200000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DOPPLER_INDICATION_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DOPPLER_INDICATION_LSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DOPPLER_INDICATION_MSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DOPPLER_INDICATION_MASK 0x0000400000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SU_EXTENDED_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SU_EXTENDED_LSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SU_EXTENDED_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SU_EXTENDED_MASK 0x0000800000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_MIN_PACKET_EXTENSION_LSB 48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_MIN_PACKET_EXTENSION_MSB 49
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x0003000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_NSS_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_NSS_LSB  50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_NSS_MSB  52
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_NSS_MASK 0x001c000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CONTENT_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CONTENT_LSB 53
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CONTENT_MSB 53
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CONTENT_MASK 0x0020000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_LTF_SIZE_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_LTF_SIZE_LSB 54
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_LTF_SIZE_MSB 55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_LTF_SIZE_MASK 0x00c0000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CHAIN_CSD_EN_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CHAIN_CSD_EN_LSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CHAIN_CSD_EN_MSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CHAIN_CSD_EN_MASK 0x0100000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CHAIN_CSD_EN_LSB 57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CHAIN_CSD_EN_MSB 57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x0200000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DL_UL_FLAG_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DL_UL_FLAG_LSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DL_UL_FLAG_MSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DL_UL_FLAG_MASK 0x0400000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_4A_OFFSET  0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_4A_LSB     59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_4A_MSB     63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_4A_MASK    0xf800000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_START_INDEX_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_START_INDEX_MSB 3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_START_INDEX_MASK 0x000000000000000f


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_SIZE_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_SIZE_LSB 4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_SIZE_MSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_SIZE_MASK 0x00000000000000f0


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_EHT_DUPLICATE_MODE_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_EHT_DUPLICATE_MODE_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_EHT_DUPLICATE_MODE_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_EHT_DUPLICATE_MODE_MASK 0x0000000000000300


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_DCM_OFFSET  0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_DCM_LSB     10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_DCM_MSB     10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_DCM_MASK    0x0000000000000400


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_0_MCS_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_0_MCS_LSB   11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_0_MCS_MSB   13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_0_MCS_MASK  0x0000000000003800


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NUM_HE_SIGB_SYM_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NUM_HE_SIGB_SYM_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NUM_HE_SIGB_SYM_MSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NUM_HE_SIGB_SYM_MASK 0x000000000007c000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_SOURCE_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_SOURCE_MSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x0000000000080000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_5A_OFFSET  0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_5A_LSB     20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_5A_MSB     25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_5A_MASK    0x0000000003f00000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0x00000000fc000000


 


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x000003ff00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000040000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000080000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000100000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x0000e00000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_LSB 48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_MSB 59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_MASK 0x0fff000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11BE_PARAMS_PLACEHOLDER_LSB 60
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11BE_PARAMS_PLACEHOLDER_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11BE_PARAMS_PLACEHOLDER_MASK 0xf000000000000000


 


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_0A_OFFSET  0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_0A_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_0A_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_0A_MASK    0x0000000000000001


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_ANTENNA_SECTOR_CTRL_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_ANTENNA_SECTOR_CTRL_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_ANTENNA_SECTOR_CTRL_MASK 0x0000000001fffffe


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_PKT_TYPE_OFFSET     0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_PKT_TYPE_LSB        25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_PKT_TYPE_MSB        28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_PKT_TYPE_MASK       0x000000001e000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SMOOTHING_OFFSET    0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SMOOTHING_LSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SMOOTHING_MSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SMOOTHING_MASK      0x0000000020000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_LDPC_OFFSET         0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_LDPC_LSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_LDPC_MSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_LDPC_MASK           0x0000000040000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STBC_OFFSET         0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STBC_LSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STBC_MSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STBC_MASK           0x0000000080000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_OFFSET   0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_LSB      32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_MSB      39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_MASK     0x000000ff00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_MIN_TX_PWR_OFFSET 0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_MIN_TX_PWR_LSB  40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_MIN_TX_PWR_MSB  47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_MIN_TX_PWR_MASK 0x0000ff0000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_NSS_OFFSET      0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_NSS_LSB         48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_NSS_MSB         50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_NSS_MASK        0x0007000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_CHAIN_MASK_OFFSET 0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_CHAIN_MASK_LSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_CHAIN_MASK_MSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_CHAIN_MASK_MASK 0x07f8000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_BW_OFFSET       0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_BW_LSB          59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_BW_MSB          61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_BW_MASK         0x3800000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STF_LTF_3DB_BOOST_OFFSET 0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STF_LTF_3DB_BOOST_LSB 62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STF_LTF_3DB_BOOST_MSB 62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STF_LTF_3DB_BOOST_MASK 0x4000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_FORCE_EXTRA_SYMBOL_OFFSET 0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_FORCE_EXTRA_SYMBOL_LSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_FORCE_EXTRA_SYMBOL_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_FORCE_EXTRA_SYMBOL_MASK 0x8000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_RATE_MCS_OFFSET 0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_RATE_MCS_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_RATE_MCS_MSB    3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_RATE_MCS_MASK   0x000000000000000f


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NSS_OFFSET          0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NSS_LSB             4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NSS_MSB             6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NSS_MASK            0x0000000000000070


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DPD_ENABLE_OFFSET   0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DPD_ENABLE_LSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DPD_ENABLE_MSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DPD_ENABLE_MASK     0x0000000000000080


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_OFFSET       0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_LSB          8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_MSB          15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_MASK         0x000000000000ff00


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MIN_TX_PWR_OFFSET   0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MIN_TX_PWR_LSB      16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MIN_TX_PWR_MSB      23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MIN_TX_PWR_MASK     0x0000000000ff0000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_CHAIN_MASK_OFFSET 0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_CHAIN_MASK_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_CHAIN_MASK_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_CHAIN_MASK_MASK  0x00000000ff000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3A_OFFSET  0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3A_LSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3A_MSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3A_MASK    0x000000ff00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SGI_OFFSET          0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SGI_LSB             40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SGI_MSB             41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SGI_MASK            0x0000030000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RATE_MCS_OFFSET     0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RATE_MCS_LSB        42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RATE_MCS_MSB        45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RATE_MCS_MASK       0x00003c0000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3B_OFFSET  0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3B_LSB     46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3B_MSB     47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3B_MASK    0x0000c00000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_1_OFFSET     0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_1_LSB        48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_1_MSB        55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_1_MASK       0x00ff000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_1_OFFSET 0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_1_LSB    56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_1_MSB    63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_1_MASK   0xff00000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_AGGREGATION_OFFSET  0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_AGGREGATION_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_AGGREGATION_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_AGGREGATION_MASK    0x0000000000000001


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_BSS_COLOR_ID_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_BSS_COLOR_ID_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_BSS_COLOR_ID_MSB 6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_BSS_COLOR_ID_MASK 0x000000000000007e


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SPATIAL_REUSE_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SPATIAL_REUSE_LSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SPATIAL_REUSE_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SPATIAL_REUSE_MASK 0x0000000000000780


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CP_LTF_SIZE_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CP_LTF_SIZE_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CP_LTF_SIZE_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CP_LTF_SIZE_MASK 0x0000000000001800


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DCM_OFFSET  0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DCM_LSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DCM_MSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DCM_MASK    0x0000000000002000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DOPPLER_INDICATION_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DOPPLER_INDICATION_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DOPPLER_INDICATION_MSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DOPPLER_INDICATION_MASK 0x0000000000004000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SU_EXTENDED_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SU_EXTENDED_LSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SU_EXTENDED_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SU_EXTENDED_MASK 0x0000000000008000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_MIN_PACKET_EXTENSION_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_MIN_PACKET_EXTENSION_MSB 17
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x0000000000030000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_NSS_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_NSS_LSB  18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_NSS_MSB  20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_NSS_MASK 0x00000000001c0000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CONTENT_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CONTENT_LSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CONTENT_MSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CONTENT_MASK 0x0000000000200000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_LTF_SIZE_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_LTF_SIZE_LSB 22
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_LTF_SIZE_MSB 23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_LTF_SIZE_MASK 0x0000000000c00000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CHAIN_CSD_EN_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CHAIN_CSD_EN_LSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CHAIN_CSD_EN_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CHAIN_CSD_EN_MASK 0x0000000001000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CHAIN_CSD_EN_LSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CHAIN_CSD_EN_MSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x0000000002000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DL_UL_FLAG_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DL_UL_FLAG_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DL_UL_FLAG_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DL_UL_FLAG_MASK 0x0000000004000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_4A_OFFSET  0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_4A_LSB     27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_4A_MSB     31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_4A_MASK    0x00000000f8000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_START_INDEX_LSB 32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_START_INDEX_MSB 35
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_START_INDEX_MASK 0x0000000f00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_SIZE_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_SIZE_LSB 36
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_SIZE_MSB 39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_SIZE_MASK 0x000000f000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_EHT_DUPLICATE_MODE_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_EHT_DUPLICATE_MODE_LSB 40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_EHT_DUPLICATE_MODE_MSB 41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_EHT_DUPLICATE_MODE_MASK 0x0000030000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_DCM_OFFSET  0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_DCM_LSB     42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_DCM_MSB     42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_DCM_MASK    0x0000040000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_0_MCS_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_0_MCS_LSB   43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_0_MCS_MSB   45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_0_MCS_MASK  0x0000380000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NUM_HE_SIGB_SYM_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NUM_HE_SIGB_SYM_LSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NUM_HE_SIGB_SYM_MSB 50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NUM_HE_SIGB_SYM_MASK 0x0007c00000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_SOURCE_LSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_SOURCE_MSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x0008000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_5A_OFFSET  0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_5A_LSB     52
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_5A_MSB     57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_5A_MASK    0x03f0000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0xfc00000000000000


 


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x00000000000003ff


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000000000000400


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000000000000800


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000000000001000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x000000000000e000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_MSB 27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_MASK 0x000000000fff0000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11BE_PARAMS_PLACEHOLDER_LSB 28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11BE_PARAMS_PLACEHOLDER_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11BE_PARAMS_PLACEHOLDER_MASK 0x00000000f0000000


 


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_0A_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_0A_LSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_0A_MSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_0A_MASK   0x0000000100000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_ANTENNA_SECTOR_CTRL_LSB 33
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_ANTENNA_SECTOR_CTRL_MSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_ANTENNA_SECTOR_CTRL_MASK 0x01fffffe00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_PKT_TYPE_OFFSET    0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_PKT_TYPE_LSB       57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_PKT_TYPE_MSB       60
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_PKT_TYPE_MASK      0x1e00000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SMOOTHING_OFFSET   0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SMOOTHING_LSB      61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SMOOTHING_MSB      61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SMOOTHING_MASK     0x2000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_LDPC_OFFSET        0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_LDPC_LSB           62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_LDPC_MSB           62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_LDPC_MASK          0x4000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STBC_OFFSET        0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STBC_LSB           63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STBC_MSB           63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STBC_MASK          0x8000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_OFFSET  0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_MASK    0x00000000000000ff


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_MIN_TX_PWR_OFFSET 0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_MIN_TX_PWR_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_MIN_TX_PWR_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_MIN_TX_PWR_MASK 0x000000000000ff00


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_NSS_OFFSET     0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_NSS_LSB        16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_NSS_MSB        18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_NSS_MASK       0x0000000000070000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_CHAIN_MASK_OFFSET 0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_CHAIN_MASK_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_CHAIN_MASK_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_CHAIN_MASK_MASK 0x0000000007f80000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_BW_OFFSET      0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_BW_LSB         27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_BW_MSB         29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_BW_MASK        0x0000000038000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STF_LTF_3DB_BOOST_OFFSET 0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STF_LTF_3DB_BOOST_LSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STF_LTF_3DB_BOOST_MSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STF_LTF_3DB_BOOST_MASK 0x0000000040000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_FORCE_EXTRA_SYMBOL_OFFSET 0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_FORCE_EXTRA_SYMBOL_LSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_FORCE_EXTRA_SYMBOL_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_FORCE_EXTRA_SYMBOL_MASK 0x0000000080000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_RATE_MCS_OFFSET 0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_RATE_MCS_LSB   32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_RATE_MCS_MSB   35
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_RATE_MCS_MASK  0x0000000f00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NSS_OFFSET         0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NSS_LSB            36
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NSS_MSB            38
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NSS_MASK           0x0000007000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DPD_ENABLE_OFFSET  0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DPD_ENABLE_LSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DPD_ENABLE_MSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DPD_ENABLE_MASK    0x0000008000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_OFFSET      0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_LSB         40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_MSB         47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_MASK        0x0000ff0000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MIN_TX_PWR_OFFSET  0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MIN_TX_PWR_LSB     48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MIN_TX_PWR_MSB     55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MIN_TX_PWR_MASK    0x00ff000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_CHAIN_MASK_OFFSET 0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_CHAIN_MASK_LSB  56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_CHAIN_MASK_MSB  63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_CHAIN_MASK_MASK 0xff00000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3A_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3A_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3A_MSB    7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3A_MASK   0x00000000000000ff


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SGI_OFFSET         0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SGI_LSB            8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SGI_MSB            9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SGI_MASK           0x0000000000000300


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RATE_MCS_OFFSET    0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RATE_MCS_LSB       10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RATE_MCS_MSB       13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RATE_MCS_MASK      0x0000000000003c00


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3B_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3B_LSB    14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3B_MSB    15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3B_MASK   0x000000000000c000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_1_OFFSET    0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_1_LSB       16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_1_MSB       23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_1_MASK      0x0000000000ff0000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_1_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_1_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_1_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_1_MASK  0x00000000ff000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_AGGREGATION_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_AGGREGATION_LSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_AGGREGATION_MSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_AGGREGATION_MASK   0x0000000100000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_BSS_COLOR_ID_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_BSS_COLOR_ID_LSB 33
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_BSS_COLOR_ID_MSB 38
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_BSS_COLOR_ID_MASK 0x0000007e00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SPATIAL_REUSE_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SPATIAL_REUSE_LSB 39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SPATIAL_REUSE_MSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SPATIAL_REUSE_MASK 0x0000078000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CP_LTF_SIZE_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CP_LTF_SIZE_LSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CP_LTF_SIZE_MSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CP_LTF_SIZE_MASK 0x0000180000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DCM_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DCM_LSB    45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DCM_MSB    45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DCM_MASK   0x0000200000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DOPPLER_INDICATION_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DOPPLER_INDICATION_LSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DOPPLER_INDICATION_MSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DOPPLER_INDICATION_MASK 0x0000400000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SU_EXTENDED_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SU_EXTENDED_LSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SU_EXTENDED_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SU_EXTENDED_MASK 0x0000800000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_MIN_PACKET_EXTENSION_LSB 48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_MIN_PACKET_EXTENSION_MSB 49
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x0003000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_NSS_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_NSS_LSB 50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_NSS_MSB 52
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_NSS_MASK 0x001c000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CONTENT_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CONTENT_LSB 53
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CONTENT_MSB 53
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CONTENT_MASK 0x0020000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_LTF_SIZE_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_LTF_SIZE_LSB 54
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_LTF_SIZE_MSB 55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_LTF_SIZE_MASK 0x00c0000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CHAIN_CSD_EN_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CHAIN_CSD_EN_LSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CHAIN_CSD_EN_MSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CHAIN_CSD_EN_MASK 0x0100000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CHAIN_CSD_EN_LSB 57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CHAIN_CSD_EN_MSB 57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x0200000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DL_UL_FLAG_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DL_UL_FLAG_LSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DL_UL_FLAG_MSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DL_UL_FLAG_MASK 0x0400000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_4A_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_4A_LSB    59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_4A_MSB    63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_4A_MASK   0xf800000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_START_INDEX_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_START_INDEX_MSB 3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_START_INDEX_MASK 0x000000000000000f


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_SIZE_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_SIZE_LSB 4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_SIZE_MSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_SIZE_MASK 0x00000000000000f0


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_EHT_DUPLICATE_MODE_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_EHT_DUPLICATE_MODE_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_EHT_DUPLICATE_MODE_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_EHT_DUPLICATE_MODE_MASK 0x0000000000000300


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_DCM_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_DCM_LSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_DCM_MSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_DCM_MASK   0x0000000000000400


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_0_MCS_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_0_MCS_LSB  11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_0_MCS_MSB  13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_0_MCS_MASK 0x0000000000003800


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NUM_HE_SIGB_SYM_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NUM_HE_SIGB_SYM_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NUM_HE_SIGB_SYM_MSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NUM_HE_SIGB_SYM_MASK 0x000000000007c000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_SOURCE_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_SOURCE_MSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x0000000000080000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_5A_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_5A_LSB    20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_5A_MSB    25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_5A_MASK   0x0000000003f00000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0x00000000fc000000


 


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x000003ff00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000040000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000080000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000100000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x0000e00000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_LSB 48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_MSB 59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_MASK 0x0fff000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11BE_PARAMS_PLACEHOLDER_LSB 60
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11BE_PARAMS_PLACEHOLDER_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11BE_PARAMS_PLACEHOLDER_MASK 0xf000000000000000


 


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_0A_OFFSET 0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_0A_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_0A_MSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_0A_MASK   0x0000000000000001


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_ANTENNA_SECTOR_CTRL_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_ANTENNA_SECTOR_CTRL_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_ANTENNA_SECTOR_CTRL_MASK 0x0000000001fffffe


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_PKT_TYPE_OFFSET    0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_PKT_TYPE_LSB       25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_PKT_TYPE_MSB       28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_PKT_TYPE_MASK      0x000000001e000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SMOOTHING_OFFSET   0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SMOOTHING_LSB      29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SMOOTHING_MSB      29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SMOOTHING_MASK     0x0000000020000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_LDPC_OFFSET        0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_LDPC_LSB           30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_LDPC_MSB           30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_LDPC_MASK          0x0000000040000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STBC_OFFSET        0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STBC_LSB           31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STBC_MSB           31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STBC_MASK          0x0000000080000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_OFFSET  0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_LSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_MSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_MASK    0x000000ff00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_MIN_TX_PWR_OFFSET 0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_MIN_TX_PWR_LSB 40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_MIN_TX_PWR_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_MIN_TX_PWR_MASK 0x0000ff0000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_NSS_OFFSET     0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_NSS_LSB        48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_NSS_MSB        50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_NSS_MASK       0x0007000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_CHAIN_MASK_OFFSET 0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_CHAIN_MASK_LSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_CHAIN_MASK_MSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_CHAIN_MASK_MASK 0x07f8000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_BW_OFFSET      0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_BW_LSB         59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_BW_MSB         61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_BW_MASK        0x3800000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STF_LTF_3DB_BOOST_OFFSET 0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STF_LTF_3DB_BOOST_LSB 62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STF_LTF_3DB_BOOST_MSB 62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STF_LTF_3DB_BOOST_MASK 0x4000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_FORCE_EXTRA_SYMBOL_OFFSET 0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_FORCE_EXTRA_SYMBOL_LSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_FORCE_EXTRA_SYMBOL_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_FORCE_EXTRA_SYMBOL_MASK 0x8000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_RATE_MCS_OFFSET 0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_RATE_MCS_LSB   0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_RATE_MCS_MSB   3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_RATE_MCS_MASK  0x000000000000000f


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NSS_OFFSET         0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NSS_LSB            4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NSS_MSB            6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NSS_MASK           0x0000000000000070


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DPD_ENABLE_OFFSET  0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DPD_ENABLE_LSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DPD_ENABLE_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DPD_ENABLE_MASK    0x0000000000000080


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_OFFSET      0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_LSB         8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_MSB         15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_MASK        0x000000000000ff00


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MIN_TX_PWR_OFFSET  0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MIN_TX_PWR_LSB     16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MIN_TX_PWR_MSB     23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MIN_TX_PWR_MASK    0x0000000000ff0000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_CHAIN_MASK_OFFSET 0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_CHAIN_MASK_LSB  24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_CHAIN_MASK_MSB  31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_CHAIN_MASK_MASK 0x00000000ff000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3A_OFFSET 0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3A_LSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3A_MSB    39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3A_MASK   0x000000ff00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SGI_OFFSET         0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SGI_LSB            40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SGI_MSB            41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SGI_MASK           0x0000030000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RATE_MCS_OFFSET    0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RATE_MCS_LSB       42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RATE_MCS_MSB       45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RATE_MCS_MASK      0x00003c0000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3B_OFFSET 0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3B_LSB    46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3B_MSB    47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3B_MASK   0x0000c00000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_1_OFFSET    0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_1_LSB       48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_1_MSB       55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_1_MASK      0x00ff000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_1_OFFSET 0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_1_LSB   56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_1_MSB   63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_1_MASK  0xff00000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_AGGREGATION_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_AGGREGATION_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_AGGREGATION_MSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_AGGREGATION_MASK   0x0000000000000001


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_BSS_COLOR_ID_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_BSS_COLOR_ID_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_BSS_COLOR_ID_MSB 6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_BSS_COLOR_ID_MASK 0x000000000000007e


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SPATIAL_REUSE_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SPATIAL_REUSE_LSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SPATIAL_REUSE_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SPATIAL_REUSE_MASK 0x0000000000000780


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CP_LTF_SIZE_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CP_LTF_SIZE_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CP_LTF_SIZE_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CP_LTF_SIZE_MASK 0x0000000000001800


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DCM_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DCM_LSB    13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DCM_MSB    13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DCM_MASK   0x0000000000002000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DOPPLER_INDICATION_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DOPPLER_INDICATION_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DOPPLER_INDICATION_MSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DOPPLER_INDICATION_MASK 0x0000000000004000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SU_EXTENDED_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SU_EXTENDED_LSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SU_EXTENDED_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SU_EXTENDED_MASK 0x0000000000008000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_MIN_PACKET_EXTENSION_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_MIN_PACKET_EXTENSION_MSB 17
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x0000000000030000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_NSS_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_NSS_LSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_NSS_MSB 20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_NSS_MASK 0x00000000001c0000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CONTENT_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CONTENT_LSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CONTENT_MSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CONTENT_MASK 0x0000000000200000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_LTF_SIZE_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_LTF_SIZE_LSB 22
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_LTF_SIZE_MSB 23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_LTF_SIZE_MASK 0x0000000000c00000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CHAIN_CSD_EN_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CHAIN_CSD_EN_LSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CHAIN_CSD_EN_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CHAIN_CSD_EN_MASK 0x0000000001000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CHAIN_CSD_EN_LSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CHAIN_CSD_EN_MSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x0000000002000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DL_UL_FLAG_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DL_UL_FLAG_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DL_UL_FLAG_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DL_UL_FLAG_MASK 0x0000000004000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_4A_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_4A_LSB    27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_4A_MSB    31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_4A_MASK   0x00000000f8000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_START_INDEX_LSB 32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_START_INDEX_MSB 35
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_START_INDEX_MASK 0x0000000f00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_SIZE_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_SIZE_LSB 36
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_SIZE_MSB 39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_SIZE_MASK 0x000000f000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_EHT_DUPLICATE_MODE_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_EHT_DUPLICATE_MODE_LSB 40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_EHT_DUPLICATE_MODE_MSB 41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_EHT_DUPLICATE_MODE_MASK 0x0000030000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_DCM_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_DCM_LSB    42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_DCM_MSB    42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_DCM_MASK   0x0000040000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_0_MCS_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_0_MCS_LSB  43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_0_MCS_MSB  45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_0_MCS_MASK 0x0000380000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NUM_HE_SIGB_SYM_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NUM_HE_SIGB_SYM_LSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NUM_HE_SIGB_SYM_MSB 50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NUM_HE_SIGB_SYM_MASK 0x0007c00000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_SOURCE_LSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_SOURCE_MSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x0008000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_5A_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_5A_LSB    52
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_5A_MSB    57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_5A_MASK   0x03f0000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0xfc00000000000000


 


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x00000000000003ff


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000000000000400


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000000000000800


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000000000001000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x000000000000e000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_MSB 27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_MASK 0x000000000fff0000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11BE_PARAMS_PLACEHOLDER_LSB 28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11BE_PARAMS_PLACEHOLDER_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11BE_PARAMS_PLACEHOLDER_MASK 0x00000000f0000000


 


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_0A_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_0A_LSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_0A_MSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_0A_MASK   0x0000000100000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_ANTENNA_SECTOR_CTRL_LSB 33
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_ANTENNA_SECTOR_CTRL_MSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_ANTENNA_SECTOR_CTRL_MASK 0x01fffffe00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_PKT_TYPE_OFFSET    0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_PKT_TYPE_LSB       57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_PKT_TYPE_MSB       60
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_PKT_TYPE_MASK      0x1e00000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SMOOTHING_OFFSET   0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SMOOTHING_LSB      61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SMOOTHING_MSB      61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SMOOTHING_MASK     0x2000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_LDPC_OFFSET        0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_LDPC_LSB           62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_LDPC_MSB           62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_LDPC_MASK          0x4000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STBC_OFFSET        0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STBC_LSB           63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STBC_MSB           63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STBC_MASK          0x8000000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_OFFSET  0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_MASK    0x00000000000000ff


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_MIN_TX_PWR_OFFSET 0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_MIN_TX_PWR_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_MIN_TX_PWR_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_MIN_TX_PWR_MASK 0x000000000000ff00


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_NSS_OFFSET     0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_NSS_LSB        16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_NSS_MSB        18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_NSS_MASK       0x0000000000070000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_CHAIN_MASK_OFFSET 0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_CHAIN_MASK_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_CHAIN_MASK_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_CHAIN_MASK_MASK 0x0000000007f80000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_BW_OFFSET      0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_BW_LSB         27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_BW_MSB         29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_BW_MASK        0x0000000038000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STF_LTF_3DB_BOOST_OFFSET 0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STF_LTF_3DB_BOOST_LSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STF_LTF_3DB_BOOST_MSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STF_LTF_3DB_BOOST_MASK 0x0000000040000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_FORCE_EXTRA_SYMBOL_OFFSET 0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_FORCE_EXTRA_SYMBOL_LSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_FORCE_EXTRA_SYMBOL_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_FORCE_EXTRA_SYMBOL_MASK 0x0000000080000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_RATE_MCS_OFFSET 0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_RATE_MCS_LSB   32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_RATE_MCS_MSB   35
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_RATE_MCS_MASK  0x0000000f00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NSS_OFFSET         0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NSS_LSB            36
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NSS_MSB            38
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NSS_MASK           0x0000007000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DPD_ENABLE_OFFSET  0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DPD_ENABLE_LSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DPD_ENABLE_MSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DPD_ENABLE_MASK    0x0000008000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_OFFSET      0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_LSB         40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_MSB         47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_MASK        0x0000ff0000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MIN_TX_PWR_OFFSET  0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MIN_TX_PWR_LSB     48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MIN_TX_PWR_MSB     55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MIN_TX_PWR_MASK    0x00ff000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_CHAIN_MASK_OFFSET 0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_CHAIN_MASK_LSB  56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_CHAIN_MASK_MSB  63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_CHAIN_MASK_MASK 0xff00000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3A_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3A_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3A_MSB    7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3A_MASK   0x00000000000000ff


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SGI_OFFSET         0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SGI_LSB            8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SGI_MSB            9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SGI_MASK           0x0000000000000300


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RATE_MCS_OFFSET    0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RATE_MCS_LSB       10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RATE_MCS_MSB       13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RATE_MCS_MASK      0x0000000000003c00


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3B_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3B_LSB    14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3B_MSB    15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3B_MASK   0x000000000000c000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_1_OFFSET    0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_1_LSB       16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_1_MSB       23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_1_MASK      0x0000000000ff0000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_1_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_1_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_1_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_1_MASK  0x00000000ff000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_AGGREGATION_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_AGGREGATION_LSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_AGGREGATION_MSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_AGGREGATION_MASK   0x0000000100000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_BSS_COLOR_ID_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_BSS_COLOR_ID_LSB 33
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_BSS_COLOR_ID_MSB 38
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_BSS_COLOR_ID_MASK 0x0000007e00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SPATIAL_REUSE_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SPATIAL_REUSE_LSB 39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SPATIAL_REUSE_MSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SPATIAL_REUSE_MASK 0x0000078000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CP_LTF_SIZE_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CP_LTF_SIZE_LSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CP_LTF_SIZE_MSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CP_LTF_SIZE_MASK 0x0000180000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DCM_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DCM_LSB    45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DCM_MSB    45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DCM_MASK   0x0000200000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DOPPLER_INDICATION_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DOPPLER_INDICATION_LSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DOPPLER_INDICATION_MSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DOPPLER_INDICATION_MASK 0x0000400000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SU_EXTENDED_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SU_EXTENDED_LSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SU_EXTENDED_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SU_EXTENDED_MASK 0x0000800000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_MIN_PACKET_EXTENSION_LSB 48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_MIN_PACKET_EXTENSION_MSB 49
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x0003000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_NSS_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_NSS_LSB 50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_NSS_MSB 52
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_NSS_MASK 0x001c000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CONTENT_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CONTENT_LSB 53
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CONTENT_MSB 53
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CONTENT_MASK 0x0020000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_LTF_SIZE_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_LTF_SIZE_LSB 54
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_LTF_SIZE_MSB 55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_LTF_SIZE_MASK 0x00c0000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CHAIN_CSD_EN_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CHAIN_CSD_EN_LSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CHAIN_CSD_EN_MSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CHAIN_CSD_EN_MASK 0x0100000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CHAIN_CSD_EN_LSB 57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CHAIN_CSD_EN_MSB 57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x0200000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DL_UL_FLAG_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DL_UL_FLAG_LSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DL_UL_FLAG_MSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DL_UL_FLAG_MASK 0x0400000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_4A_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_4A_LSB    59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_4A_MSB    63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_4A_MASK   0xf800000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_START_INDEX_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_START_INDEX_MSB 3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_START_INDEX_MASK 0x000000000000000f


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_SIZE_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_SIZE_LSB 4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_SIZE_MSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_SIZE_MASK 0x00000000000000f0


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_EHT_DUPLICATE_MODE_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_EHT_DUPLICATE_MODE_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_EHT_DUPLICATE_MODE_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_EHT_DUPLICATE_MODE_MASK 0x0000000000000300


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_DCM_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_DCM_LSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_DCM_MSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_DCM_MASK   0x0000000000000400


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_0_MCS_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_0_MCS_LSB  11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_0_MCS_MSB  13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_0_MCS_MASK 0x0000000000003800


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NUM_HE_SIGB_SYM_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NUM_HE_SIGB_SYM_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NUM_HE_SIGB_SYM_MSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NUM_HE_SIGB_SYM_MASK 0x000000000007c000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_SOURCE_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_SOURCE_MSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x0000000000080000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_5A_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_5A_LSB    20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_5A_MSB    25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_5A_MASK   0x0000000003f00000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0x00000000fc000000


 


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x000003ff00000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000040000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000080000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000100000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x0000e00000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_LSB 48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_MSB 59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_MASK 0x0fff000000000000


 

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11BE_PARAMS_PLACEHOLDER_LSB 60
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11BE_PARAMS_PLACEHOLDER_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11BE_PARAMS_PLACEHOLDER_MASK 0xf000000000000000


 

#define PCU_PPDU_SETUP_INIT_R2R_HW_RESPONSE_TX_DURATION_OFFSET                      0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_HW_RESPONSE_TX_DURATION_LSB                         0
#define PCU_PPDU_SETUP_INIT_R2R_HW_RESPONSE_TX_DURATION_MSB                         15
#define PCU_PPDU_SETUP_INIT_R2R_HW_RESPONSE_TX_DURATION_MASK                        0x000000000000ffff


 

#define PCU_PPDU_SETUP_INIT_R2R_RX_DURATION_FIELD_OFFSET                            0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_RX_DURATION_FIELD_LSB                               16
#define PCU_PPDU_SETUP_INIT_R2R_RX_DURATION_FIELD_MSB                               31
#define PCU_PPDU_SETUP_INIT_R2R_RX_DURATION_FIELD_MASK                              0x00000000ffff0000


 

#define PCU_PPDU_SETUP_INIT_R2R_GROUP_ID_OFFSET                                     0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_GROUP_ID_LSB                                        32
#define PCU_PPDU_SETUP_INIT_R2R_GROUP_ID_MSB                                        37
#define PCU_PPDU_SETUP_INIT_R2R_GROUP_ID_MASK                                       0x0000003f00000000


 

#define PCU_PPDU_SETUP_INIT_R2R_RESPONSE_FRAME_TYPE_OFFSET                          0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_RESPONSE_FRAME_TYPE_LSB                             38
#define PCU_PPDU_SETUP_INIT_R2R_RESPONSE_FRAME_TYPE_MSB                             41
#define PCU_PPDU_SETUP_INIT_R2R_RESPONSE_FRAME_TYPE_MASK                            0x000003c000000000


 

#define PCU_PPDU_SETUP_INIT_R2R_STA_PARTIAL_AID_OFFSET                              0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_STA_PARTIAL_AID_LSB                                 42
#define PCU_PPDU_SETUP_INIT_R2R_STA_PARTIAL_AID_MSB                                 52
#define PCU_PPDU_SETUP_INIT_R2R_STA_PARTIAL_AID_MASK                                0x001ffc0000000000


 

#define PCU_PPDU_SETUP_INIT_USE_ADDRESS_FIELDS_FOR_PROTECTION_OFFSET                0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_USE_ADDRESS_FIELDS_FOR_PROTECTION_LSB                   53
#define PCU_PPDU_SETUP_INIT_USE_ADDRESS_FIELDS_FOR_PROTECTION_MSB                   53
#define PCU_PPDU_SETUP_INIT_USE_ADDRESS_FIELDS_FOR_PROTECTION_MASK                  0x0020000000000000


 

#define PCU_PPDU_SETUP_INIT_R2R_SET_REQUIRED_RESPONSE_TIME_OFFSET                   0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_SET_REQUIRED_RESPONSE_TIME_LSB                      54
#define PCU_PPDU_SETUP_INIT_R2R_SET_REQUIRED_RESPONSE_TIME_MSB                      54
#define PCU_PPDU_SETUP_INIT_R2R_SET_REQUIRED_RESPONSE_TIME_MASK                     0x0040000000000000


 

#define PCU_PPDU_SETUP_INIT_RESERVED_29A_OFFSET                                     0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_RESERVED_29A_LSB                                        55
#define PCU_PPDU_SETUP_INIT_RESERVED_29A_MSB                                        57
#define PCU_PPDU_SETUP_INIT_RESERVED_29A_MASK                                       0x0380000000000000


 

#define PCU_PPDU_SETUP_INIT_R2R_BW20_ACTIVE_CHANNEL_OFFSET                          0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_BW20_ACTIVE_CHANNEL_LSB                             58
#define PCU_PPDU_SETUP_INIT_R2R_BW20_ACTIVE_CHANNEL_MSB                             60
#define PCU_PPDU_SETUP_INIT_R2R_BW20_ACTIVE_CHANNEL_MASK                            0x1c00000000000000


 

#define PCU_PPDU_SETUP_INIT_R2R_BW40_ACTIVE_CHANNEL_OFFSET                          0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_BW40_ACTIVE_CHANNEL_LSB                             61
#define PCU_PPDU_SETUP_INIT_R2R_BW40_ACTIVE_CHANNEL_MSB                             63
#define PCU_PPDU_SETUP_INIT_R2R_BW40_ACTIVE_CHANNEL_MASK                            0xe000000000000000


 

#define PCU_PPDU_SETUP_INIT_R2R_BW80_ACTIVE_CHANNEL_OFFSET                          0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW80_ACTIVE_CHANNEL_LSB                             0
#define PCU_PPDU_SETUP_INIT_R2R_BW80_ACTIVE_CHANNEL_MSB                             2
#define PCU_PPDU_SETUP_INIT_R2R_BW80_ACTIVE_CHANNEL_MASK                            0x0000000000000007


 

#define PCU_PPDU_SETUP_INIT_R2R_BW160_ACTIVE_CHANNEL_OFFSET                         0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW160_ACTIVE_CHANNEL_LSB                            3
#define PCU_PPDU_SETUP_INIT_R2R_BW160_ACTIVE_CHANNEL_MSB                            5
#define PCU_PPDU_SETUP_INIT_R2R_BW160_ACTIVE_CHANNEL_MASK                           0x0000000000000038


 

#define PCU_PPDU_SETUP_INIT_R2R_BW240_ACTIVE_CHANNEL_OFFSET                         0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW240_ACTIVE_CHANNEL_LSB                            6
#define PCU_PPDU_SETUP_INIT_R2R_BW240_ACTIVE_CHANNEL_MSB                            8
#define PCU_PPDU_SETUP_INIT_R2R_BW240_ACTIVE_CHANNEL_MASK                           0x00000000000001c0


 

#define PCU_PPDU_SETUP_INIT_R2R_BW320_ACTIVE_CHANNEL_OFFSET                         0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW320_ACTIVE_CHANNEL_LSB                            9
#define PCU_PPDU_SETUP_INIT_R2R_BW320_ACTIVE_CHANNEL_MSB                            11
#define PCU_PPDU_SETUP_INIT_R2R_BW320_ACTIVE_CHANNEL_MASK                           0x0000000000000e00


 

#define PCU_PPDU_SETUP_INIT_R2R_BW20_OFFSET                                         0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW20_LSB                                            12
#define PCU_PPDU_SETUP_INIT_R2R_BW20_MSB                                            14
#define PCU_PPDU_SETUP_INIT_R2R_BW20_MASK                                           0x0000000000007000


 

#define PCU_PPDU_SETUP_INIT_R2R_BW40_OFFSET                                         0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW40_LSB                                            15
#define PCU_PPDU_SETUP_INIT_R2R_BW40_MSB                                            17
#define PCU_PPDU_SETUP_INIT_R2R_BW40_MASK                                           0x0000000000038000


 

#define PCU_PPDU_SETUP_INIT_R2R_BW80_OFFSET                                         0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW80_LSB                                            18
#define PCU_PPDU_SETUP_INIT_R2R_BW80_MSB                                            20
#define PCU_PPDU_SETUP_INIT_R2R_BW80_MASK                                           0x00000000001c0000


 

#define PCU_PPDU_SETUP_INIT_R2R_BW160_OFFSET                                        0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW160_LSB                                           21
#define PCU_PPDU_SETUP_INIT_R2R_BW160_MSB                                           23
#define PCU_PPDU_SETUP_INIT_R2R_BW160_MASK                                          0x0000000000e00000


 

#define PCU_PPDU_SETUP_INIT_R2R_BW240_OFFSET                                        0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW240_LSB                                           24
#define PCU_PPDU_SETUP_INIT_R2R_BW240_MSB                                           26
#define PCU_PPDU_SETUP_INIT_R2R_BW240_MASK                                          0x0000000007000000


 

#define PCU_PPDU_SETUP_INIT_R2R_BW320_OFFSET                                        0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW320_LSB                                           27
#define PCU_PPDU_SETUP_INIT_R2R_BW320_MSB                                           29
#define PCU_PPDU_SETUP_INIT_R2R_BW320_MASK                                          0x0000000038000000


 

#define PCU_PPDU_SETUP_INIT_RESERVED_30A_OFFSET                                     0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_RESERVED_30A_LSB                                        30
#define PCU_PPDU_SETUP_INIT_RESERVED_30A_MSB                                        31
#define PCU_PPDU_SETUP_INIT_RESERVED_30A_MASK                                       0x00000000c0000000


 

#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_31_0_OFFSET                 0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_31_0_LSB                    32
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_31_0_MSB                    63
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_31_0_MASK                   0xffffffff00000000


 

#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_36_32_OFFSET                0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_36_32_LSB                   0
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_36_32_MSB                   4
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_36_32_MASK                  0x000000000000001f


 

#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_CBF_COUNT_OFFSET                   0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_CBF_COUNT_LSB                      5
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_CBF_COUNT_MSB                      10
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_CBF_COUNT_MASK                     0x00000000000007e0


 

#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_STA_COUNT_OFFSET                   0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_STA_COUNT_LSB                      11
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_STA_COUNT_MSB                      16
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_STA_COUNT_MASK                     0x000000000001f800


 

#define PCU_PPDU_SETUP_INIT_TRANSMIT_INCLUDES_MULTIDESTINATION_OFFSET               0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_TRANSMIT_INCLUDES_MULTIDESTINATION_LSB                  17
#define PCU_PPDU_SETUP_INIT_TRANSMIT_INCLUDES_MULTIDESTINATION_MSB                  17
#define PCU_PPDU_SETUP_INIT_TRANSMIT_INCLUDES_MULTIDESTINATION_MASK                 0x0000000000020000


 

#define PCU_PPDU_SETUP_INIT_INSERT_PREV_TX_START_TIMING_INFO_OFFSET                 0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_INSERT_PREV_TX_START_TIMING_INFO_LSB                    18
#define PCU_PPDU_SETUP_INIT_INSERT_PREV_TX_START_TIMING_INFO_MSB                    18
#define PCU_PPDU_SETUP_INIT_INSERT_PREV_TX_START_TIMING_INFO_MASK                   0x0000000000040000


 

#define PCU_PPDU_SETUP_INIT_INSERT_CURRENT_TX_START_TIMING_INFO_OFFSET              0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_INSERT_CURRENT_TX_START_TIMING_INFO_LSB                 19
#define PCU_PPDU_SETUP_INIT_INSERT_CURRENT_TX_START_TIMING_INFO_MSB                 19
#define PCU_PPDU_SETUP_INIT_INSERT_CURRENT_TX_START_TIMING_INFO_MASK                0x0000000000080000


 

#define PCU_PPDU_SETUP_INIT_TX_START_TRANSMIT_TIME_BYTE_OFFSET_OFFSET               0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_TX_START_TRANSMIT_TIME_BYTE_OFFSET_LSB                  20
#define PCU_PPDU_SETUP_INIT_TX_START_TRANSMIT_TIME_BYTE_OFFSET_MSB                  31
#define PCU_PPDU_SETUP_INIT_TX_START_TRANSMIT_TIME_BYTE_OFFSET_MASK                 0x00000000fff00000


 

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_31_0_OFFSET                        0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_31_0_LSB                           32
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_31_0_MSB                           63
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_31_0_MASK                          0xffffffff00000000


 

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_47_32_OFFSET                       0x00000000000000d0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_47_32_LSB                          0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_47_32_MSB                          15
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_47_32_MASK                         0x000000000000ffff


 

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_15_0_OFFSET                        0x00000000000000d0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_15_0_LSB                           16
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_15_0_MSB                           31
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_15_0_MASK                          0x00000000ffff0000


 

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_47_16_OFFSET                       0x00000000000000d0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_47_16_LSB                          32
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_47_16_MSB                          63
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_47_16_MASK                         0xffffffff00000000


 

#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_THRESHOLD_OFFSET                    0x00000000000000d8
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_THRESHOLD_LSB                       0
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_THRESHOLD_MSB                       23
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_THRESHOLD_MASK                      0x0000000000ffffff


 

#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_TYPE_OFFSET                         0x00000000000000d8
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_TYPE_LSB                            24
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_TYPE_MSB                            24
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_TYPE_MASK                           0x0000000001000000


 

#define PCU_PPDU_SETUP_INIT_RESERVED_54A_OFFSET                                     0x00000000000000d8
#define PCU_PPDU_SETUP_INIT_RESERVED_54A_LSB                                        25
#define PCU_PPDU_SETUP_INIT_RESERVED_54A_MSB                                        31
#define PCU_PPDU_SETUP_INIT_RESERVED_54A_MASK                                       0x00000000fe000000


 

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_31_0_OFFSET                        0x00000000000000d8
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_31_0_LSB                           32
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_31_0_MSB                           63
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_31_0_MASK                          0xffffffff00000000


 

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_47_32_OFFSET                       0x00000000000000e0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_47_32_LSB                          0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_47_32_MSB                          15
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_47_32_MASK                         0x000000000000ffff


 

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_15_0_OFFSET                        0x00000000000000e0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_15_0_LSB                           16
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_15_0_MSB                           31
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_15_0_MASK                          0x00000000ffff0000


 

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_47_16_OFFSET                       0x00000000000000e0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_47_16_LSB                          32
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_47_16_MSB                          63
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_47_16_MASK                         0xffffffff00000000



#endif    
