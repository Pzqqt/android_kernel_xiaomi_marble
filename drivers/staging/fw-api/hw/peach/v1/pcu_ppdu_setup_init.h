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


#ifndef _PCU_PPDU_SETUP_INIT_H_
#define _PCU_PPDU_SETUP_INIT_H_

#include "pdg_response_rate_setting.h"
#define NUM_OF_DWORDS_PCU_PPDU_SETUP_INIT 58

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

#define PCU_PPDU_SETUP_INIT_MEDIUM_PROT_TYPE_OFFSET                                 0x00000000
#define PCU_PPDU_SETUP_INIT_MEDIUM_PROT_TYPE_LSB                                    0
#define PCU_PPDU_SETUP_INIT_MEDIUM_PROT_TYPE_MSB                                    2
#define PCU_PPDU_SETUP_INIT_MEDIUM_PROT_TYPE_MASK                                   0x00000007

#define PCU_PPDU_SETUP_INIT_RESPONSE_TYPE_OFFSET                                    0x00000000
#define PCU_PPDU_SETUP_INIT_RESPONSE_TYPE_LSB                                       3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TYPE_MSB                                       7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TYPE_MASK                                      0x000000f8

#define PCU_PPDU_SETUP_INIT_RESPONSE_INFO_PART2_REQUIRED_OFFSET                     0x00000000
#define PCU_PPDU_SETUP_INIT_RESPONSE_INFO_PART2_REQUIRED_LSB                        8
#define PCU_PPDU_SETUP_INIT_RESPONSE_INFO_PART2_REQUIRED_MSB                        8
#define PCU_PPDU_SETUP_INIT_RESPONSE_INFO_PART2_REQUIRED_MASK                       0x00000100

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_OFFSET                             0x00000000
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_LSB                                9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_MSB                                11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_MASK                               0x00000e00

#define PCU_PPDU_SETUP_INIT_MBA_USER_ORDER_OFFSET                                   0x00000000
#define PCU_PPDU_SETUP_INIT_MBA_USER_ORDER_LSB                                      12
#define PCU_PPDU_SETUP_INIT_MBA_USER_ORDER_MSB                                      13
#define PCU_PPDU_SETUP_INIT_MBA_USER_ORDER_MASK                                     0x00003000

#define PCU_PPDU_SETUP_INIT_EXPECTED_MBA_SIZE_OFFSET                                0x00000000
#define PCU_PPDU_SETUP_INIT_EXPECTED_MBA_SIZE_LSB                                   14
#define PCU_PPDU_SETUP_INIT_EXPECTED_MBA_SIZE_MSB                                   24
#define PCU_PPDU_SETUP_INIT_EXPECTED_MBA_SIZE_MASK                                  0x01ffc000

#define PCU_PPDU_SETUP_INIT_REQUIRED_UL_MU_RESP_USER_COUNT_OFFSET                   0x00000000
#define PCU_PPDU_SETUP_INIT_REQUIRED_UL_MU_RESP_USER_COUNT_LSB                      25
#define PCU_PPDU_SETUP_INIT_REQUIRED_UL_MU_RESP_USER_COUNT_MSB                      30
#define PCU_PPDU_SETUP_INIT_REQUIRED_UL_MU_RESP_USER_COUNT_MASK                     0x7e000000

#define PCU_PPDU_SETUP_INIT_TRANSMITTED_BSSID_CHECK_EN_OFFSET                       0x00000000
#define PCU_PPDU_SETUP_INIT_TRANSMITTED_BSSID_CHECK_EN_LSB                          31
#define PCU_PPDU_SETUP_INIT_TRANSMITTED_BSSID_CHECK_EN_MSB                          31
#define PCU_PPDU_SETUP_INIT_TRANSMITTED_BSSID_CHECK_EN_MASK                         0x80000000

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW1_OFFSET                               0x00000004
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW1_LSB                                  0
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW1_MSB                                  0
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW1_MASK                                 0x00000001

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW20_OFFSET                              0x00000004
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW20_LSB                                 1
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW20_MSB                                 1
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW20_MASK                                0x00000002

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW40_OFFSET                              0x00000004
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW40_LSB                                 2
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW40_MSB                                 2
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW40_MASK                                0x00000004

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW80_OFFSET                              0x00000004
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW80_LSB                                 3
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW80_MSB                                 3
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW80_MASK                                0x00000008

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW160_OFFSET                             0x00000004
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW160_LSB                                4
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW160_MSB                                4
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW160_MASK                               0x00000010

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW240_OFFSET                             0x00000004
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW240_LSB                                5
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW240_MSB                                5
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW240_MASK                               0x00000020

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW320_OFFSET                             0x00000004
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW320_LSB                                6
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW320_MSB                                6
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW320_MASK                               0x00000040

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW1_OFFSET                                 0x00000004
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW1_LSB                                    7
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW1_MSB                                    7
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW1_MASK                                   0x00000080

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW20_OFFSET                                0x00000004
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW20_LSB                                   8
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW20_MSB                                   8
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW20_MASK                                  0x00000100

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW40_OFFSET                                0x00000004
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW40_LSB                                   9
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW40_MSB                                   9
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW40_MASK                                  0x00000200

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW80_OFFSET                                0x00000004
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW80_LSB                                   10
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW80_MSB                                   10
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW80_MASK                                  0x00000400

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW160_OFFSET                               0x00000004
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW160_LSB                                  11
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW160_MSB                                  11
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW160_MASK                                 0x00000800

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW240_OFFSET                               0x00000004
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW240_LSB                                  12
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW240_MSB                                  12
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW240_MASK                                 0x00001000

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW320_OFFSET                               0x00000004
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW320_LSB                                  13
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW320_MSB                                  13
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW320_MASK                                 0x00002000

#define PCU_PPDU_SETUP_INIT_SET_FC_PWR_MGT_OFFSET                                   0x00000004
#define PCU_PPDU_SETUP_INIT_SET_FC_PWR_MGT_LSB                                      14
#define PCU_PPDU_SETUP_INIT_SET_FC_PWR_MGT_MSB                                      14
#define PCU_PPDU_SETUP_INIT_SET_FC_PWR_MGT_MASK                                     0x00004000

#define PCU_PPDU_SETUP_INIT_USE_CTS_DURATION_FOR_DATA_TX_OFFSET                     0x00000004
#define PCU_PPDU_SETUP_INIT_USE_CTS_DURATION_FOR_DATA_TX_LSB                        15
#define PCU_PPDU_SETUP_INIT_USE_CTS_DURATION_FOR_DATA_TX_MSB                        15
#define PCU_PPDU_SETUP_INIT_USE_CTS_DURATION_FOR_DATA_TX_MASK                       0x00008000

#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_64_OFFSET                              0x00000004
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_64_LSB                                 16
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_64_MSB                                 16
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_64_MASK                                0x00010000

#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_LOWER_OFFSET                        0x00000004
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_LOWER_LSB                           17
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_LOWER_MSB                           17
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_LOWER_MASK                          0x00020000

#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_UPPER_OFFSET                        0x00000004
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_UPPER_LSB                           18
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_UPPER_MSB                           18
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_UPPER_MASK                          0x00040000

#define PCU_PPDU_SETUP_INIT_RESERVED_1A_OFFSET                                      0x00000004
#define PCU_PPDU_SETUP_INIT_RESERVED_1A_LSB                                         19
#define PCU_PPDU_SETUP_INIT_RESERVED_1A_MSB                                         31
#define PCU_PPDU_SETUP_INIT_RESERVED_1A_MASK                                        0xfff80000

#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_0_OFFSET                        0x00000008
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_0_LSB                           0
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_0_MSB                           15
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_0_MASK                          0x0000ffff

#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_1_OFFSET                        0x00000008
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_1_LSB                           16
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_1_MSB                           31
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_1_MASK                          0xffff0000

#define PCU_PPDU_SETUP_INIT_MAX_BW40_TRY_COUNT_OFFSET                               0x0000000c
#define PCU_PPDU_SETUP_INIT_MAX_BW40_TRY_COUNT_LSB                                  0
#define PCU_PPDU_SETUP_INIT_MAX_BW40_TRY_COUNT_MSB                                  3
#define PCU_PPDU_SETUP_INIT_MAX_BW40_TRY_COUNT_MASK                                 0x0000000f

#define PCU_PPDU_SETUP_INIT_MAX_BW80_TRY_COUNT_OFFSET                               0x0000000c
#define PCU_PPDU_SETUP_INIT_MAX_BW80_TRY_COUNT_LSB                                  4
#define PCU_PPDU_SETUP_INIT_MAX_BW80_TRY_COUNT_MSB                                  7
#define PCU_PPDU_SETUP_INIT_MAX_BW80_TRY_COUNT_MASK                                 0x000000f0

#define PCU_PPDU_SETUP_INIT_MAX_BW160_TRY_COUNT_OFFSET                              0x0000000c
#define PCU_PPDU_SETUP_INIT_MAX_BW160_TRY_COUNT_LSB                                 8
#define PCU_PPDU_SETUP_INIT_MAX_BW160_TRY_COUNT_MSB                                 11
#define PCU_PPDU_SETUP_INIT_MAX_BW160_TRY_COUNT_MASK                                0x00000f00

#define PCU_PPDU_SETUP_INIT_MAX_BW240_TRY_COUNT_OFFSET                              0x0000000c
#define PCU_PPDU_SETUP_INIT_MAX_BW240_TRY_COUNT_LSB                                 12
#define PCU_PPDU_SETUP_INIT_MAX_BW240_TRY_COUNT_MSB                                 15
#define PCU_PPDU_SETUP_INIT_MAX_BW240_TRY_COUNT_MASK                                0x0000f000

#define PCU_PPDU_SETUP_INIT_MAX_BW320_TRY_COUNT_OFFSET                              0x0000000c
#define PCU_PPDU_SETUP_INIT_MAX_BW320_TRY_COUNT_LSB                                 16
#define PCU_PPDU_SETUP_INIT_MAX_BW320_TRY_COUNT_MSB                                 19
#define PCU_PPDU_SETUP_INIT_MAX_BW320_TRY_COUNT_MASK                                0x000f0000

#define PCU_PPDU_SETUP_INIT_INSERT_WUR_TIMESTAMP_OFFSET_OFFSET                      0x0000000c
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_TIMESTAMP_OFFSET_LSB                         20
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_TIMESTAMP_OFFSET_MSB                         25
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_TIMESTAMP_OFFSET_MASK                        0x03f00000

#define PCU_PPDU_SETUP_INIT_UPDATE_WUR_TIMESTAMP_OFFSET                             0x0000000c
#define PCU_PPDU_SETUP_INIT_UPDATE_WUR_TIMESTAMP_LSB                                26
#define PCU_PPDU_SETUP_INIT_UPDATE_WUR_TIMESTAMP_MSB                                26
#define PCU_PPDU_SETUP_INIT_UPDATE_WUR_TIMESTAMP_MASK                               0x04000000

#define PCU_PPDU_SETUP_INIT_WUR_EMBEDDED_BSSID_PRESENT_OFFSET                       0x0000000c
#define PCU_PPDU_SETUP_INIT_WUR_EMBEDDED_BSSID_PRESENT_LSB                          27
#define PCU_PPDU_SETUP_INIT_WUR_EMBEDDED_BSSID_PRESENT_MSB                          27
#define PCU_PPDU_SETUP_INIT_WUR_EMBEDDED_BSSID_PRESENT_MASK                         0x08000000

#define PCU_PPDU_SETUP_INIT_INSERT_WUR_FCS_OFFSET                                   0x0000000c
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_FCS_LSB                                      28
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_FCS_MSB                                      28
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_FCS_MASK                                     0x10000000

#define PCU_PPDU_SETUP_INIT_RESERVED_3B_OFFSET                                      0x0000000c
#define PCU_PPDU_SETUP_INIT_RESERVED_3B_LSB                                         29
#define PCU_PPDU_SETUP_INIT_RESERVED_3B_MSB                                         31
#define PCU_PPDU_SETUP_INIT_RESERVED_3B_MASK                                        0xe0000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_0A_OFFSET  0x00000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_0A_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_0A_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_0A_MASK    0x00000001

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x00000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_ANTENNA_SECTOR_CTRL_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_ANTENNA_SECTOR_CTRL_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_ANTENNA_SECTOR_CTRL_MASK 0x01fffffe

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_PKT_TYPE_OFFSET     0x00000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_PKT_TYPE_LSB        25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_PKT_TYPE_MSB        28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_PKT_TYPE_MASK       0x1e000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SMOOTHING_OFFSET    0x00000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SMOOTHING_LSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SMOOTHING_MSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SMOOTHING_MASK      0x20000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_LDPC_OFFSET         0x00000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_LDPC_LSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_LDPC_MSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_LDPC_MASK           0x40000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STBC_OFFSET         0x00000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STBC_LSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STBC_MSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STBC_MASK           0x80000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_OFFSET   0x00000014
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_LSB      0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_MSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_MASK     0x000000ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_MIN_TX_PWR_OFFSET 0x00000014
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_MIN_TX_PWR_LSB  8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_MIN_TX_PWR_MSB  15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_MIN_TX_PWR_MASK 0x0000ff00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_NSS_OFFSET      0x00000014
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_NSS_LSB         16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_NSS_MSB         18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_NSS_MASK        0x00070000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_CHAIN_MASK_OFFSET 0x00000014
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_CHAIN_MASK_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_CHAIN_MASK_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_CHAIN_MASK_MASK 0x07f80000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_BW_OFFSET       0x00000014
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_BW_LSB          27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_BW_MSB          29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_BW_MASK         0x38000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STF_LTF_3DB_BOOST_OFFSET 0x00000014
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STF_LTF_3DB_BOOST_LSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STF_LTF_3DB_BOOST_MSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STF_LTF_3DB_BOOST_MASK 0x40000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_FORCE_EXTRA_SYMBOL_OFFSET 0x00000014
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_FORCE_EXTRA_SYMBOL_LSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_FORCE_EXTRA_SYMBOL_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_FORCE_EXTRA_SYMBOL_MASK 0x80000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_RATE_MCS_OFFSET 0x00000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_RATE_MCS_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_RATE_MCS_MSB    3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_RATE_MCS_MASK   0x0000000f

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NSS_OFFSET          0x00000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NSS_LSB             4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NSS_MSB             6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NSS_MASK            0x00000070

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DPD_ENABLE_OFFSET   0x00000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DPD_ENABLE_LSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DPD_ENABLE_MSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DPD_ENABLE_MASK     0x00000080

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_OFFSET       0x00000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_LSB          8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_MSB          15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_MASK         0x0000ff00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MIN_TX_PWR_OFFSET   0x00000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MIN_TX_PWR_LSB      16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MIN_TX_PWR_MSB      23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MIN_TX_PWR_MASK     0x00ff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_CHAIN_MASK_OFFSET 0x00000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_CHAIN_MASK_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_CHAIN_MASK_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_CHAIN_MASK_MASK  0xff000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3A_OFFSET  0x0000001c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3A_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3A_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3A_MASK    0x000000ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SGI_OFFSET          0x0000001c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SGI_LSB             8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SGI_MSB             9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SGI_MASK            0x00000300

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RATE_MCS_OFFSET     0x0000001c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RATE_MCS_LSB        10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RATE_MCS_MSB        13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RATE_MCS_MASK       0x00003c00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3B_OFFSET  0x0000001c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3B_LSB     14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3B_MSB     15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3B_MASK    0x0000c000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_1_OFFSET     0x0000001c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_1_LSB        16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_1_MSB        23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_1_MASK       0x00ff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_1_OFFSET 0x0000001c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_1_LSB    24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_1_MSB    31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_1_MASK   0xff000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_AGGREGATION_OFFSET  0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_AGGREGATION_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_AGGREGATION_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_AGGREGATION_MASK    0x00000001

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_BSS_COLOR_ID_OFFSET 0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_BSS_COLOR_ID_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_BSS_COLOR_ID_MSB 6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_BSS_COLOR_ID_MASK 0x0000007e

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SPATIAL_REUSE_OFFSET 0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SPATIAL_REUSE_LSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SPATIAL_REUSE_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SPATIAL_REUSE_MASK 0x00000780

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CP_LTF_SIZE_OFFSET 0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CP_LTF_SIZE_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CP_LTF_SIZE_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CP_LTF_SIZE_MASK 0x00001800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DCM_OFFSET  0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DCM_LSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DCM_MSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DCM_MASK    0x00002000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DOPPLER_INDICATION_OFFSET 0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DOPPLER_INDICATION_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DOPPLER_INDICATION_MSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DOPPLER_INDICATION_MASK 0x00004000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SU_EXTENDED_OFFSET 0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SU_EXTENDED_LSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SU_EXTENDED_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SU_EXTENDED_MASK 0x00008000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_MIN_PACKET_EXTENSION_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_MIN_PACKET_EXTENSION_MSB 17
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x00030000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_NSS_OFFSET 0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_NSS_LSB  18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_NSS_MSB  20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_NSS_MASK 0x001c0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CONTENT_OFFSET 0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CONTENT_LSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CONTENT_MSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CONTENT_MASK 0x00200000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_LTF_SIZE_OFFSET 0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_LTF_SIZE_LSB 22
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_LTF_SIZE_MSB 23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_LTF_SIZE_MASK 0x00c00000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CHAIN_CSD_EN_OFFSET 0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CHAIN_CSD_EN_LSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CHAIN_CSD_EN_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CHAIN_CSD_EN_MASK 0x01000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CHAIN_CSD_EN_LSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CHAIN_CSD_EN_MSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x02000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DL_UL_FLAG_OFFSET 0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DL_UL_FLAG_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DL_UL_FLAG_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DL_UL_FLAG_MASK 0x04000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_4A_OFFSET  0x00000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_4A_LSB     27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_4A_MSB     31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_4A_MASK    0xf8000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x00000024
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_START_INDEX_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_START_INDEX_MSB 3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_START_INDEX_MASK 0x0000000f

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_SIZE_OFFSET 0x00000024
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_SIZE_LSB 4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_SIZE_MSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_SIZE_MASK 0x000000f0

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_EHT_DUPLICATE_MODE_OFFSET 0x00000024
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_EHT_DUPLICATE_MODE_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_EHT_DUPLICATE_MODE_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_EHT_DUPLICATE_MODE_MASK 0x00000300

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_DCM_OFFSET  0x00000024
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_DCM_LSB     10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_DCM_MSB     10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_DCM_MASK    0x00000400

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_0_MCS_OFFSET 0x00000024
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_0_MCS_LSB   11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_0_MCS_MSB   13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_0_MCS_MASK  0x00003800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NUM_HE_SIGB_SYM_OFFSET 0x00000024
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NUM_HE_SIGB_SYM_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NUM_HE_SIGB_SYM_MSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NUM_HE_SIGB_SYM_MASK 0x0007c000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x00000024
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_SOURCE_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_SOURCE_MSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x00080000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_5A_OFFSET  0x00000024
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_5A_LSB     20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_5A_MSB     25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_5A_MASK    0x03f00000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x00000024
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0xfc000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x00000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x000003ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x00000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x00000400

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x00000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x00000800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x00000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x00001000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x00000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x0000e000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_OFFSET 0x00000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_MSB 27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_MASK 0x0fff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x00000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11BE_PARAMS_PLACEHOLDER_LSB 28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11BE_PARAMS_PLACEHOLDER_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11BE_PARAMS_PLACEHOLDER_MASK 0xf0000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_0A_OFFSET  0x0000002c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_0A_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_0A_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_0A_MASK    0x00000001

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000002c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_ANTENNA_SECTOR_CTRL_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_ANTENNA_SECTOR_CTRL_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_ANTENNA_SECTOR_CTRL_MASK 0x01fffffe

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_PKT_TYPE_OFFSET     0x0000002c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_PKT_TYPE_LSB        25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_PKT_TYPE_MSB        28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_PKT_TYPE_MASK       0x1e000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SMOOTHING_OFFSET    0x0000002c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SMOOTHING_LSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SMOOTHING_MSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SMOOTHING_MASK      0x20000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_LDPC_OFFSET         0x0000002c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_LDPC_LSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_LDPC_MSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_LDPC_MASK           0x40000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STBC_OFFSET         0x0000002c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STBC_LSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STBC_MSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STBC_MASK           0x80000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_OFFSET   0x00000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_LSB      0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_MSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_MASK     0x000000ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_MIN_TX_PWR_OFFSET 0x00000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_MIN_TX_PWR_LSB  8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_MIN_TX_PWR_MSB  15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_MIN_TX_PWR_MASK 0x0000ff00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_NSS_OFFSET      0x00000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_NSS_LSB         16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_NSS_MSB         18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_NSS_MASK        0x00070000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_CHAIN_MASK_OFFSET 0x00000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_CHAIN_MASK_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_CHAIN_MASK_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_CHAIN_MASK_MASK 0x07f80000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_BW_OFFSET       0x00000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_BW_LSB          27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_BW_MSB          29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_BW_MASK         0x38000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STF_LTF_3DB_BOOST_OFFSET 0x00000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STF_LTF_3DB_BOOST_LSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STF_LTF_3DB_BOOST_MSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STF_LTF_3DB_BOOST_MASK 0x40000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_FORCE_EXTRA_SYMBOL_OFFSET 0x00000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_FORCE_EXTRA_SYMBOL_LSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_FORCE_EXTRA_SYMBOL_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_FORCE_EXTRA_SYMBOL_MASK 0x80000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_RATE_MCS_OFFSET 0x00000034
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_RATE_MCS_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_RATE_MCS_MSB    3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_RATE_MCS_MASK   0x0000000f

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NSS_OFFSET          0x00000034
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NSS_LSB             4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NSS_MSB             6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NSS_MASK            0x00000070

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DPD_ENABLE_OFFSET   0x00000034
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DPD_ENABLE_LSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DPD_ENABLE_MSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DPD_ENABLE_MASK     0x00000080

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_OFFSET       0x00000034
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_LSB          8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_MSB          15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_MASK         0x0000ff00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MIN_TX_PWR_OFFSET   0x00000034
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MIN_TX_PWR_LSB      16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MIN_TX_PWR_MSB      23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MIN_TX_PWR_MASK     0x00ff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_CHAIN_MASK_OFFSET 0x00000034
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_CHAIN_MASK_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_CHAIN_MASK_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_CHAIN_MASK_MASK  0xff000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3A_OFFSET  0x00000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3A_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3A_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3A_MASK    0x000000ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SGI_OFFSET          0x00000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SGI_LSB             8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SGI_MSB             9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SGI_MASK            0x00000300

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RATE_MCS_OFFSET     0x00000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RATE_MCS_LSB        10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RATE_MCS_MSB        13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RATE_MCS_MASK       0x00003c00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3B_OFFSET  0x00000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3B_LSB     14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3B_MSB     15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3B_MASK    0x0000c000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_1_OFFSET     0x00000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_1_LSB        16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_1_MSB        23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_1_MASK       0x00ff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_1_OFFSET 0x00000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_1_LSB    24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_1_MSB    31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_1_MASK   0xff000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_AGGREGATION_OFFSET  0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_AGGREGATION_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_AGGREGATION_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_AGGREGATION_MASK    0x00000001

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_BSS_COLOR_ID_OFFSET 0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_BSS_COLOR_ID_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_BSS_COLOR_ID_MSB 6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_BSS_COLOR_ID_MASK 0x0000007e

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SPATIAL_REUSE_OFFSET 0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SPATIAL_REUSE_LSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SPATIAL_REUSE_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SPATIAL_REUSE_MASK 0x00000780

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CP_LTF_SIZE_OFFSET 0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CP_LTF_SIZE_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CP_LTF_SIZE_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CP_LTF_SIZE_MASK 0x00001800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DCM_OFFSET  0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DCM_LSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DCM_MSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DCM_MASK    0x00002000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DOPPLER_INDICATION_OFFSET 0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DOPPLER_INDICATION_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DOPPLER_INDICATION_MSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DOPPLER_INDICATION_MASK 0x00004000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SU_EXTENDED_OFFSET 0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SU_EXTENDED_LSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SU_EXTENDED_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SU_EXTENDED_MASK 0x00008000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_MIN_PACKET_EXTENSION_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_MIN_PACKET_EXTENSION_MSB 17
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x00030000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_NSS_OFFSET 0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_NSS_LSB  18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_NSS_MSB  20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_NSS_MASK 0x001c0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CONTENT_OFFSET 0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CONTENT_LSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CONTENT_MSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CONTENT_MASK 0x00200000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_LTF_SIZE_OFFSET 0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_LTF_SIZE_LSB 22
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_LTF_SIZE_MSB 23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_LTF_SIZE_MASK 0x00c00000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CHAIN_CSD_EN_OFFSET 0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CHAIN_CSD_EN_LSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CHAIN_CSD_EN_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CHAIN_CSD_EN_MASK 0x01000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CHAIN_CSD_EN_LSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CHAIN_CSD_EN_MSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x02000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DL_UL_FLAG_OFFSET 0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DL_UL_FLAG_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DL_UL_FLAG_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DL_UL_FLAG_MASK 0x04000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_4A_OFFSET  0x0000003c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_4A_LSB     27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_4A_MSB     31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_4A_MASK    0xf8000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x00000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_START_INDEX_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_START_INDEX_MSB 3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_START_INDEX_MASK 0x0000000f

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_SIZE_OFFSET 0x00000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_SIZE_LSB 4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_SIZE_MSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_SIZE_MASK 0x000000f0

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_EHT_DUPLICATE_MODE_OFFSET 0x00000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_EHT_DUPLICATE_MODE_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_EHT_DUPLICATE_MODE_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_EHT_DUPLICATE_MODE_MASK 0x00000300

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_DCM_OFFSET  0x00000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_DCM_LSB     10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_DCM_MSB     10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_DCM_MASK    0x00000400

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_0_MCS_OFFSET 0x00000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_0_MCS_LSB   11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_0_MCS_MSB   13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_0_MCS_MASK  0x00003800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NUM_HE_SIGB_SYM_OFFSET 0x00000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NUM_HE_SIGB_SYM_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NUM_HE_SIGB_SYM_MSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NUM_HE_SIGB_SYM_MASK 0x0007c000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x00000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_SOURCE_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_SOURCE_MSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x00080000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_5A_OFFSET  0x00000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_5A_LSB     20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_5A_MSB     25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_5A_MASK    0x03f00000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x00000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0xfc000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x00000044
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x000003ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x00000044
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x00000400

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x00000044
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x00000800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x00000044
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x00001000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x00000044
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x0000e000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_OFFSET 0x00000044
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_MSB 27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_MASK 0x0fff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x00000044
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11BE_PARAMS_PLACEHOLDER_LSB 28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11BE_PARAMS_PLACEHOLDER_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11BE_PARAMS_PLACEHOLDER_MASK 0xf0000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_0A_OFFSET  0x00000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_0A_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_0A_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_0A_MASK    0x00000001

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x00000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_ANTENNA_SECTOR_CTRL_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_ANTENNA_SECTOR_CTRL_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_ANTENNA_SECTOR_CTRL_MASK 0x01fffffe

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_PKT_TYPE_OFFSET     0x00000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_PKT_TYPE_LSB        25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_PKT_TYPE_MSB        28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_PKT_TYPE_MASK       0x1e000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SMOOTHING_OFFSET    0x00000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SMOOTHING_LSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SMOOTHING_MSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SMOOTHING_MASK      0x20000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_LDPC_OFFSET         0x00000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_LDPC_LSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_LDPC_MSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_LDPC_MASK           0x40000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STBC_OFFSET         0x00000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STBC_LSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STBC_MSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STBC_MASK           0x80000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_OFFSET   0x0000004c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_LSB      0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_MSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_MASK     0x000000ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_MIN_TX_PWR_OFFSET 0x0000004c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_MIN_TX_PWR_LSB  8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_MIN_TX_PWR_MSB  15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_MIN_TX_PWR_MASK 0x0000ff00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_NSS_OFFSET      0x0000004c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_NSS_LSB         16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_NSS_MSB         18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_NSS_MASK        0x00070000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_CHAIN_MASK_OFFSET 0x0000004c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_CHAIN_MASK_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_CHAIN_MASK_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_CHAIN_MASK_MASK 0x07f80000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_BW_OFFSET       0x0000004c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_BW_LSB          27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_BW_MSB          29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_BW_MASK         0x38000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STF_LTF_3DB_BOOST_OFFSET 0x0000004c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STF_LTF_3DB_BOOST_LSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STF_LTF_3DB_BOOST_MSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STF_LTF_3DB_BOOST_MASK 0x40000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_FORCE_EXTRA_SYMBOL_OFFSET 0x0000004c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_FORCE_EXTRA_SYMBOL_LSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_FORCE_EXTRA_SYMBOL_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_FORCE_EXTRA_SYMBOL_MASK 0x80000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_RATE_MCS_OFFSET 0x00000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_RATE_MCS_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_RATE_MCS_MSB    3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_RATE_MCS_MASK   0x0000000f

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NSS_OFFSET          0x00000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NSS_LSB             4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NSS_MSB             6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NSS_MASK            0x00000070

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DPD_ENABLE_OFFSET   0x00000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DPD_ENABLE_LSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DPD_ENABLE_MSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DPD_ENABLE_MASK     0x00000080

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_OFFSET       0x00000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_LSB          8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_MSB          15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_MASK         0x0000ff00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MIN_TX_PWR_OFFSET   0x00000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MIN_TX_PWR_LSB      16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MIN_TX_PWR_MSB      23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MIN_TX_PWR_MASK     0x00ff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_CHAIN_MASK_OFFSET 0x00000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_CHAIN_MASK_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_CHAIN_MASK_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_CHAIN_MASK_MASK  0xff000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3A_OFFSET  0x00000054
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3A_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3A_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3A_MASK    0x000000ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SGI_OFFSET          0x00000054
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SGI_LSB             8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SGI_MSB             9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SGI_MASK            0x00000300

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RATE_MCS_OFFSET     0x00000054
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RATE_MCS_LSB        10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RATE_MCS_MSB        13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RATE_MCS_MASK       0x00003c00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3B_OFFSET  0x00000054
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3B_LSB     14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3B_MSB     15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3B_MASK    0x0000c000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_1_OFFSET     0x00000054
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_1_LSB        16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_1_MSB        23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_1_MASK       0x00ff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_1_OFFSET 0x00000054
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_1_LSB    24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_1_MSB    31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_1_MASK   0xff000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_AGGREGATION_OFFSET  0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_AGGREGATION_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_AGGREGATION_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_AGGREGATION_MASK    0x00000001

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_BSS_COLOR_ID_OFFSET 0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_BSS_COLOR_ID_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_BSS_COLOR_ID_MSB 6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_BSS_COLOR_ID_MASK 0x0000007e

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SPATIAL_REUSE_OFFSET 0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SPATIAL_REUSE_LSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SPATIAL_REUSE_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SPATIAL_REUSE_MASK 0x00000780

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CP_LTF_SIZE_OFFSET 0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CP_LTF_SIZE_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CP_LTF_SIZE_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CP_LTF_SIZE_MASK 0x00001800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DCM_OFFSET  0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DCM_LSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DCM_MSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DCM_MASK    0x00002000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DOPPLER_INDICATION_OFFSET 0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DOPPLER_INDICATION_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DOPPLER_INDICATION_MSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DOPPLER_INDICATION_MASK 0x00004000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SU_EXTENDED_OFFSET 0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SU_EXTENDED_LSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SU_EXTENDED_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SU_EXTENDED_MASK 0x00008000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_MIN_PACKET_EXTENSION_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_MIN_PACKET_EXTENSION_MSB 17
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x00030000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_NSS_OFFSET 0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_NSS_LSB  18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_NSS_MSB  20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_NSS_MASK 0x001c0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CONTENT_OFFSET 0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CONTENT_LSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CONTENT_MSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CONTENT_MASK 0x00200000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_LTF_SIZE_OFFSET 0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_LTF_SIZE_LSB 22
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_LTF_SIZE_MSB 23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_LTF_SIZE_MASK 0x00c00000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CHAIN_CSD_EN_OFFSET 0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CHAIN_CSD_EN_LSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CHAIN_CSD_EN_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CHAIN_CSD_EN_MASK 0x01000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CHAIN_CSD_EN_LSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CHAIN_CSD_EN_MSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x02000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DL_UL_FLAG_OFFSET 0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DL_UL_FLAG_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DL_UL_FLAG_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DL_UL_FLAG_MASK 0x04000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_4A_OFFSET  0x00000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_4A_LSB     27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_4A_MSB     31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_4A_MASK    0xf8000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x0000005c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_START_INDEX_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_START_INDEX_MSB 3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_START_INDEX_MASK 0x0000000f

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_SIZE_OFFSET 0x0000005c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_SIZE_LSB 4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_SIZE_MSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_SIZE_MASK 0x000000f0

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_EHT_DUPLICATE_MODE_OFFSET 0x0000005c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_EHT_DUPLICATE_MODE_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_EHT_DUPLICATE_MODE_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_EHT_DUPLICATE_MODE_MASK 0x00000300

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_DCM_OFFSET  0x0000005c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_DCM_LSB     10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_DCM_MSB     10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_DCM_MASK    0x00000400

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_0_MCS_OFFSET 0x0000005c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_0_MCS_LSB   11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_0_MCS_MSB   13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_0_MCS_MASK  0x00003800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NUM_HE_SIGB_SYM_OFFSET 0x0000005c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NUM_HE_SIGB_SYM_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NUM_HE_SIGB_SYM_MSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NUM_HE_SIGB_SYM_MASK 0x0007c000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x0000005c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_SOURCE_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_SOURCE_MSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x00080000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_5A_OFFSET  0x0000005c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_5A_LSB     20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_5A_MSB     25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_5A_MASK    0x03f00000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x0000005c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0xfc000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x00000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x000003ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x00000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x00000400

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x00000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x00000800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x00000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x00001000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x00000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x0000e000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_OFFSET 0x00000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_MSB 27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_MASK 0x0fff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x00000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11BE_PARAMS_PLACEHOLDER_LSB 28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11BE_PARAMS_PLACEHOLDER_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11BE_PARAMS_PLACEHOLDER_MASK 0xf0000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_0A_OFFSET 0x00000064
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_0A_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_0A_MSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_0A_MASK   0x00000001

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x00000064
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_ANTENNA_SECTOR_CTRL_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_ANTENNA_SECTOR_CTRL_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_ANTENNA_SECTOR_CTRL_MASK 0x01fffffe

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_PKT_TYPE_OFFSET    0x00000064
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_PKT_TYPE_LSB       25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_PKT_TYPE_MSB       28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_PKT_TYPE_MASK      0x1e000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SMOOTHING_OFFSET   0x00000064
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SMOOTHING_LSB      29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SMOOTHING_MSB      29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SMOOTHING_MASK     0x20000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_LDPC_OFFSET        0x00000064
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_LDPC_LSB           30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_LDPC_MSB           30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_LDPC_MASK          0x40000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STBC_OFFSET        0x00000064
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STBC_LSB           31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STBC_MSB           31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STBC_MASK          0x80000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_OFFSET  0x00000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_MASK    0x000000ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_MIN_TX_PWR_OFFSET 0x00000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_MIN_TX_PWR_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_MIN_TX_PWR_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_MIN_TX_PWR_MASK 0x0000ff00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_NSS_OFFSET     0x00000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_NSS_LSB        16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_NSS_MSB        18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_NSS_MASK       0x00070000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_CHAIN_MASK_OFFSET 0x00000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_CHAIN_MASK_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_CHAIN_MASK_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_CHAIN_MASK_MASK 0x07f80000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_BW_OFFSET      0x00000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_BW_LSB         27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_BW_MSB         29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_BW_MASK        0x38000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STF_LTF_3DB_BOOST_OFFSET 0x00000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STF_LTF_3DB_BOOST_LSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STF_LTF_3DB_BOOST_MSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STF_LTF_3DB_BOOST_MASK 0x40000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_FORCE_EXTRA_SYMBOL_OFFSET 0x00000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_FORCE_EXTRA_SYMBOL_LSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_FORCE_EXTRA_SYMBOL_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_FORCE_EXTRA_SYMBOL_MASK 0x80000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_RATE_MCS_OFFSET 0x0000006c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_RATE_MCS_LSB   0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_RATE_MCS_MSB   3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_RATE_MCS_MASK  0x0000000f

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NSS_OFFSET         0x0000006c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NSS_LSB            4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NSS_MSB            6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NSS_MASK           0x00000070

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DPD_ENABLE_OFFSET  0x0000006c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DPD_ENABLE_LSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DPD_ENABLE_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DPD_ENABLE_MASK    0x00000080

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_OFFSET      0x0000006c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_LSB         8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_MSB         15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_MASK        0x0000ff00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MIN_TX_PWR_OFFSET  0x0000006c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MIN_TX_PWR_LSB     16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MIN_TX_PWR_MSB     23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MIN_TX_PWR_MASK    0x00ff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_CHAIN_MASK_OFFSET 0x0000006c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_CHAIN_MASK_LSB  24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_CHAIN_MASK_MSB  31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_CHAIN_MASK_MASK 0xff000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3A_OFFSET 0x00000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3A_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3A_MSB    7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3A_MASK   0x000000ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SGI_OFFSET         0x00000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SGI_LSB            8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SGI_MSB            9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SGI_MASK           0x00000300

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RATE_MCS_OFFSET    0x00000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RATE_MCS_LSB       10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RATE_MCS_MSB       13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RATE_MCS_MASK      0x00003c00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3B_OFFSET 0x00000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3B_LSB    14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3B_MSB    15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3B_MASK   0x0000c000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_1_OFFSET    0x00000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_1_LSB       16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_1_MSB       23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_1_MASK      0x00ff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_1_OFFSET 0x00000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_1_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_1_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_1_MASK  0xff000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_AGGREGATION_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_AGGREGATION_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_AGGREGATION_MSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_AGGREGATION_MASK   0x00000001

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_BSS_COLOR_ID_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_BSS_COLOR_ID_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_BSS_COLOR_ID_MSB 6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_BSS_COLOR_ID_MASK 0x0000007e

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SPATIAL_REUSE_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SPATIAL_REUSE_LSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SPATIAL_REUSE_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SPATIAL_REUSE_MASK 0x00000780

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CP_LTF_SIZE_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CP_LTF_SIZE_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CP_LTF_SIZE_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CP_LTF_SIZE_MASK 0x00001800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DCM_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DCM_LSB    13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DCM_MSB    13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DCM_MASK   0x00002000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DOPPLER_INDICATION_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DOPPLER_INDICATION_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DOPPLER_INDICATION_MSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DOPPLER_INDICATION_MASK 0x00004000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SU_EXTENDED_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SU_EXTENDED_LSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SU_EXTENDED_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SU_EXTENDED_MASK 0x00008000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_MIN_PACKET_EXTENSION_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_MIN_PACKET_EXTENSION_MSB 17
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x00030000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_NSS_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_NSS_LSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_NSS_MSB 20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_NSS_MASK 0x001c0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CONTENT_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CONTENT_LSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CONTENT_MSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CONTENT_MASK 0x00200000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_LTF_SIZE_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_LTF_SIZE_LSB 22
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_LTF_SIZE_MSB 23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_LTF_SIZE_MASK 0x00c00000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CHAIN_CSD_EN_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CHAIN_CSD_EN_LSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CHAIN_CSD_EN_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CHAIN_CSD_EN_MASK 0x01000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CHAIN_CSD_EN_LSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CHAIN_CSD_EN_MSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x02000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DL_UL_FLAG_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DL_UL_FLAG_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DL_UL_FLAG_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DL_UL_FLAG_MASK 0x04000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_4A_OFFSET 0x00000074
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_4A_LSB    27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_4A_MSB    31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_4A_MASK   0xf8000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x00000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_START_INDEX_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_START_INDEX_MSB 3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_START_INDEX_MASK 0x0000000f

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_SIZE_OFFSET 0x00000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_SIZE_LSB 4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_SIZE_MSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_SIZE_MASK 0x000000f0

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_EHT_DUPLICATE_MODE_OFFSET 0x00000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_EHT_DUPLICATE_MODE_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_EHT_DUPLICATE_MODE_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_EHT_DUPLICATE_MODE_MASK 0x00000300

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_DCM_OFFSET 0x00000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_DCM_LSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_DCM_MSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_DCM_MASK   0x00000400

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_0_MCS_OFFSET 0x00000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_0_MCS_LSB  11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_0_MCS_MSB  13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_0_MCS_MASK 0x00003800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NUM_HE_SIGB_SYM_OFFSET 0x00000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NUM_HE_SIGB_SYM_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NUM_HE_SIGB_SYM_MSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NUM_HE_SIGB_SYM_MASK 0x0007c000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x00000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_SOURCE_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_SOURCE_MSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x00080000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_5A_OFFSET 0x00000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_5A_LSB    20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_5A_MSB    25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_5A_MASK   0x03f00000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x00000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0xfc000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x0000007c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x000003ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x0000007c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x00000400

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000007c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x00000800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000007c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x00001000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x0000007c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x0000e000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_OFFSET 0x0000007c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_MSB 27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_MASK 0x0fff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x0000007c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11BE_PARAMS_PLACEHOLDER_LSB 28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11BE_PARAMS_PLACEHOLDER_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11BE_PARAMS_PLACEHOLDER_MASK 0xf0000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_0A_OFFSET 0x00000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_0A_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_0A_MSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_0A_MASK   0x00000001

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x00000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_ANTENNA_SECTOR_CTRL_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_ANTENNA_SECTOR_CTRL_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_ANTENNA_SECTOR_CTRL_MASK 0x01fffffe

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_PKT_TYPE_OFFSET    0x00000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_PKT_TYPE_LSB       25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_PKT_TYPE_MSB       28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_PKT_TYPE_MASK      0x1e000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SMOOTHING_OFFSET   0x00000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SMOOTHING_LSB      29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SMOOTHING_MSB      29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SMOOTHING_MASK     0x20000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_LDPC_OFFSET        0x00000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_LDPC_LSB           30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_LDPC_MSB           30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_LDPC_MASK          0x40000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STBC_OFFSET        0x00000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STBC_LSB           31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STBC_MSB           31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STBC_MASK          0x80000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_OFFSET  0x00000084
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_MASK    0x000000ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_MIN_TX_PWR_OFFSET 0x00000084
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_MIN_TX_PWR_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_MIN_TX_PWR_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_MIN_TX_PWR_MASK 0x0000ff00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_NSS_OFFSET     0x00000084
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_NSS_LSB        16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_NSS_MSB        18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_NSS_MASK       0x00070000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_CHAIN_MASK_OFFSET 0x00000084
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_CHAIN_MASK_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_CHAIN_MASK_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_CHAIN_MASK_MASK 0x07f80000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_BW_OFFSET      0x00000084
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_BW_LSB         27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_BW_MSB         29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_BW_MASK        0x38000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STF_LTF_3DB_BOOST_OFFSET 0x00000084
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STF_LTF_3DB_BOOST_LSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STF_LTF_3DB_BOOST_MSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STF_LTF_3DB_BOOST_MASK 0x40000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_FORCE_EXTRA_SYMBOL_OFFSET 0x00000084
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_FORCE_EXTRA_SYMBOL_LSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_FORCE_EXTRA_SYMBOL_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_FORCE_EXTRA_SYMBOL_MASK 0x80000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_RATE_MCS_OFFSET 0x00000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_RATE_MCS_LSB   0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_RATE_MCS_MSB   3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_RATE_MCS_MASK  0x0000000f

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NSS_OFFSET         0x00000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NSS_LSB            4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NSS_MSB            6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NSS_MASK           0x00000070

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DPD_ENABLE_OFFSET  0x00000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DPD_ENABLE_LSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DPD_ENABLE_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DPD_ENABLE_MASK    0x00000080

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_OFFSET      0x00000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_LSB         8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_MSB         15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_MASK        0x0000ff00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MIN_TX_PWR_OFFSET  0x00000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MIN_TX_PWR_LSB     16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MIN_TX_PWR_MSB     23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MIN_TX_PWR_MASK    0x00ff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_CHAIN_MASK_OFFSET 0x00000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_CHAIN_MASK_LSB  24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_CHAIN_MASK_MSB  31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_CHAIN_MASK_MASK 0xff000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3A_OFFSET 0x0000008c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3A_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3A_MSB    7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3A_MASK   0x000000ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SGI_OFFSET         0x0000008c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SGI_LSB            8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SGI_MSB            9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SGI_MASK           0x00000300

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RATE_MCS_OFFSET    0x0000008c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RATE_MCS_LSB       10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RATE_MCS_MSB       13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RATE_MCS_MASK      0x00003c00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3B_OFFSET 0x0000008c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3B_LSB    14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3B_MSB    15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3B_MASK   0x0000c000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_1_OFFSET    0x0000008c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_1_LSB       16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_1_MSB       23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_1_MASK      0x00ff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_1_OFFSET 0x0000008c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_1_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_1_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_1_MASK  0xff000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_AGGREGATION_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_AGGREGATION_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_AGGREGATION_MSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_AGGREGATION_MASK   0x00000001

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_BSS_COLOR_ID_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_BSS_COLOR_ID_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_BSS_COLOR_ID_MSB 6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_BSS_COLOR_ID_MASK 0x0000007e

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SPATIAL_REUSE_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SPATIAL_REUSE_LSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SPATIAL_REUSE_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SPATIAL_REUSE_MASK 0x00000780

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CP_LTF_SIZE_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CP_LTF_SIZE_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CP_LTF_SIZE_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CP_LTF_SIZE_MASK 0x00001800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DCM_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DCM_LSB    13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DCM_MSB    13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DCM_MASK   0x00002000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DOPPLER_INDICATION_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DOPPLER_INDICATION_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DOPPLER_INDICATION_MSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DOPPLER_INDICATION_MASK 0x00004000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SU_EXTENDED_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SU_EXTENDED_LSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SU_EXTENDED_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SU_EXTENDED_MASK 0x00008000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_MIN_PACKET_EXTENSION_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_MIN_PACKET_EXTENSION_MSB 17
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x00030000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_NSS_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_NSS_LSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_NSS_MSB 20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_NSS_MASK 0x001c0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CONTENT_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CONTENT_LSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CONTENT_MSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CONTENT_MASK 0x00200000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_LTF_SIZE_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_LTF_SIZE_LSB 22
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_LTF_SIZE_MSB 23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_LTF_SIZE_MASK 0x00c00000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CHAIN_CSD_EN_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CHAIN_CSD_EN_LSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CHAIN_CSD_EN_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CHAIN_CSD_EN_MASK 0x01000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CHAIN_CSD_EN_LSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CHAIN_CSD_EN_MSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x02000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DL_UL_FLAG_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DL_UL_FLAG_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DL_UL_FLAG_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DL_UL_FLAG_MASK 0x04000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_4A_OFFSET 0x00000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_4A_LSB    27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_4A_MSB    31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_4A_MASK   0xf8000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x00000094
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_START_INDEX_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_START_INDEX_MSB 3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_START_INDEX_MASK 0x0000000f

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_SIZE_OFFSET 0x00000094
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_SIZE_LSB 4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_SIZE_MSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_SIZE_MASK 0x000000f0

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_EHT_DUPLICATE_MODE_OFFSET 0x00000094
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_EHT_DUPLICATE_MODE_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_EHT_DUPLICATE_MODE_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_EHT_DUPLICATE_MODE_MASK 0x00000300

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_DCM_OFFSET 0x00000094
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_DCM_LSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_DCM_MSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_DCM_MASK   0x00000400

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_0_MCS_OFFSET 0x00000094
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_0_MCS_LSB  11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_0_MCS_MSB  13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_0_MCS_MASK 0x00003800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NUM_HE_SIGB_SYM_OFFSET 0x00000094
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NUM_HE_SIGB_SYM_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NUM_HE_SIGB_SYM_MSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NUM_HE_SIGB_SYM_MASK 0x0007c000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x00000094
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_SOURCE_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_SOURCE_MSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x00080000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_5A_OFFSET 0x00000094
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_5A_LSB    20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_5A_MSB    25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_5A_MASK   0x03f00000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x00000094
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0xfc000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x00000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x000003ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x00000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x00000400

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x00000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x00000800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x00000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x00001000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x00000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x0000e000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_OFFSET 0x00000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_MSB 27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_MASK 0x0fff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x00000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11BE_PARAMS_PLACEHOLDER_LSB 28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11BE_PARAMS_PLACEHOLDER_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11BE_PARAMS_PLACEHOLDER_MASK 0xf0000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_0A_OFFSET 0x0000009c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_0A_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_0A_MSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_0A_MASK   0x00000001

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000009c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_ANTENNA_SECTOR_CTRL_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_ANTENNA_SECTOR_CTRL_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_ANTENNA_SECTOR_CTRL_MASK 0x01fffffe

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_PKT_TYPE_OFFSET    0x0000009c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_PKT_TYPE_LSB       25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_PKT_TYPE_MSB       28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_PKT_TYPE_MASK      0x1e000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SMOOTHING_OFFSET   0x0000009c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SMOOTHING_LSB      29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SMOOTHING_MSB      29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SMOOTHING_MASK     0x20000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_LDPC_OFFSET        0x0000009c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_LDPC_LSB           30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_LDPC_MSB           30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_LDPC_MASK          0x40000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STBC_OFFSET        0x0000009c
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STBC_LSB           31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STBC_MSB           31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STBC_MASK          0x80000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_OFFSET  0x000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_MASK    0x000000ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_MIN_TX_PWR_OFFSET 0x000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_MIN_TX_PWR_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_MIN_TX_PWR_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_MIN_TX_PWR_MASK 0x0000ff00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_NSS_OFFSET     0x000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_NSS_LSB        16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_NSS_MSB        18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_NSS_MASK       0x00070000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_CHAIN_MASK_OFFSET 0x000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_CHAIN_MASK_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_CHAIN_MASK_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_CHAIN_MASK_MASK 0x07f80000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_BW_OFFSET      0x000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_BW_LSB         27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_BW_MSB         29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_BW_MASK        0x38000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STF_LTF_3DB_BOOST_OFFSET 0x000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STF_LTF_3DB_BOOST_LSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STF_LTF_3DB_BOOST_MSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STF_LTF_3DB_BOOST_MASK 0x40000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_FORCE_EXTRA_SYMBOL_OFFSET 0x000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_FORCE_EXTRA_SYMBOL_LSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_FORCE_EXTRA_SYMBOL_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_FORCE_EXTRA_SYMBOL_MASK 0x80000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_RATE_MCS_OFFSET 0x000000a4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_RATE_MCS_LSB   0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_RATE_MCS_MSB   3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_RATE_MCS_MASK  0x0000000f

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NSS_OFFSET         0x000000a4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NSS_LSB            4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NSS_MSB            6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NSS_MASK           0x00000070

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DPD_ENABLE_OFFSET  0x000000a4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DPD_ENABLE_LSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DPD_ENABLE_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DPD_ENABLE_MASK    0x00000080

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_OFFSET      0x000000a4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_LSB         8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_MSB         15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_MASK        0x0000ff00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MIN_TX_PWR_OFFSET  0x000000a4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MIN_TX_PWR_LSB     16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MIN_TX_PWR_MSB     23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MIN_TX_PWR_MASK    0x00ff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_CHAIN_MASK_OFFSET 0x000000a4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_CHAIN_MASK_LSB  24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_CHAIN_MASK_MSB  31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_CHAIN_MASK_MASK 0xff000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3A_OFFSET 0x000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3A_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3A_MSB    7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3A_MASK   0x000000ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SGI_OFFSET         0x000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SGI_LSB            8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SGI_MSB            9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SGI_MASK           0x00000300

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RATE_MCS_OFFSET    0x000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RATE_MCS_LSB       10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RATE_MCS_MSB       13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RATE_MCS_MASK      0x00003c00

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3B_OFFSET 0x000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3B_LSB    14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3B_MSB    15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3B_MASK   0x0000c000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_1_OFFSET    0x000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_1_LSB       16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_1_MSB       23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_1_MASK      0x00ff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_1_OFFSET 0x000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_1_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_1_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_1_MASK  0xff000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_AGGREGATION_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_AGGREGATION_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_AGGREGATION_MSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_AGGREGATION_MASK   0x00000001

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_BSS_COLOR_ID_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_BSS_COLOR_ID_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_BSS_COLOR_ID_MSB 6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_BSS_COLOR_ID_MASK 0x0000007e

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SPATIAL_REUSE_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SPATIAL_REUSE_LSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SPATIAL_REUSE_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SPATIAL_REUSE_MASK 0x00000780

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CP_LTF_SIZE_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CP_LTF_SIZE_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CP_LTF_SIZE_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CP_LTF_SIZE_MASK 0x00001800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DCM_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DCM_LSB    13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DCM_MSB    13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DCM_MASK   0x00002000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DOPPLER_INDICATION_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DOPPLER_INDICATION_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DOPPLER_INDICATION_MSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DOPPLER_INDICATION_MASK 0x00004000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SU_EXTENDED_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SU_EXTENDED_LSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SU_EXTENDED_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SU_EXTENDED_MASK 0x00008000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_MIN_PACKET_EXTENSION_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_MIN_PACKET_EXTENSION_MSB 17
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x00030000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_NSS_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_NSS_LSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_NSS_MSB 20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_NSS_MASK 0x001c0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CONTENT_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CONTENT_LSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CONTENT_MSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CONTENT_MASK 0x00200000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_LTF_SIZE_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_LTF_SIZE_LSB 22
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_LTF_SIZE_MSB 23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_LTF_SIZE_MASK 0x00c00000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CHAIN_CSD_EN_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CHAIN_CSD_EN_LSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CHAIN_CSD_EN_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CHAIN_CSD_EN_MASK 0x01000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CHAIN_CSD_EN_LSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CHAIN_CSD_EN_MSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x02000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DL_UL_FLAG_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DL_UL_FLAG_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DL_UL_FLAG_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DL_UL_FLAG_MASK 0x04000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_4A_OFFSET 0x000000ac
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_4A_LSB    27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_4A_MSB    31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_4A_MASK   0xf8000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_START_INDEX_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_START_INDEX_MSB 3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_START_INDEX_MASK 0x0000000f

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_SIZE_OFFSET 0x000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_SIZE_LSB 4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_SIZE_MSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_SIZE_MASK 0x000000f0

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_EHT_DUPLICATE_MODE_OFFSET 0x000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_EHT_DUPLICATE_MODE_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_EHT_DUPLICATE_MODE_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_EHT_DUPLICATE_MODE_MASK 0x00000300

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_DCM_OFFSET 0x000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_DCM_LSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_DCM_MSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_DCM_MASK   0x00000400

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_0_MCS_OFFSET 0x000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_0_MCS_LSB  11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_0_MCS_MSB  13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_0_MCS_MASK 0x00003800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NUM_HE_SIGB_SYM_OFFSET 0x000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NUM_HE_SIGB_SYM_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NUM_HE_SIGB_SYM_MSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NUM_HE_SIGB_SYM_MASK 0x0007c000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_SOURCE_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_SOURCE_MSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x00080000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_5A_OFFSET 0x000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_5A_LSB    20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_5A_MSB    25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_5A_MASK   0x03f00000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0xfc000000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x000000b4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x000003ff

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x000000b4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x00000400

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x000000b4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x00000800

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x000000b4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x00001000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x000000b4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x0000e000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_OFFSET 0x000000b4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_MSB 27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_MASK 0x0fff0000

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x000000b4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11BE_PARAMS_PLACEHOLDER_LSB 28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11BE_PARAMS_PLACEHOLDER_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11BE_PARAMS_PLACEHOLDER_MASK 0xf0000000

#define PCU_PPDU_SETUP_INIT_R2R_HW_RESPONSE_TX_DURATION_OFFSET                      0x000000b8
#define PCU_PPDU_SETUP_INIT_R2R_HW_RESPONSE_TX_DURATION_LSB                         0
#define PCU_PPDU_SETUP_INIT_R2R_HW_RESPONSE_TX_DURATION_MSB                         15
#define PCU_PPDU_SETUP_INIT_R2R_HW_RESPONSE_TX_DURATION_MASK                        0x0000ffff

#define PCU_PPDU_SETUP_INIT_R2R_RX_DURATION_FIELD_OFFSET                            0x000000b8
#define PCU_PPDU_SETUP_INIT_R2R_RX_DURATION_FIELD_LSB                               16
#define PCU_PPDU_SETUP_INIT_R2R_RX_DURATION_FIELD_MSB                               31
#define PCU_PPDU_SETUP_INIT_R2R_RX_DURATION_FIELD_MASK                              0xffff0000

#define PCU_PPDU_SETUP_INIT_R2R_GROUP_ID_OFFSET                                     0x000000bc
#define PCU_PPDU_SETUP_INIT_R2R_GROUP_ID_LSB                                        0
#define PCU_PPDU_SETUP_INIT_R2R_GROUP_ID_MSB                                        5
#define PCU_PPDU_SETUP_INIT_R2R_GROUP_ID_MASK                                       0x0000003f

#define PCU_PPDU_SETUP_INIT_R2R_RESPONSE_FRAME_TYPE_OFFSET                          0x000000bc
#define PCU_PPDU_SETUP_INIT_R2R_RESPONSE_FRAME_TYPE_LSB                             6
#define PCU_PPDU_SETUP_INIT_R2R_RESPONSE_FRAME_TYPE_MSB                             9
#define PCU_PPDU_SETUP_INIT_R2R_RESPONSE_FRAME_TYPE_MASK                            0x000003c0

#define PCU_PPDU_SETUP_INIT_R2R_STA_PARTIAL_AID_OFFSET                              0x000000bc
#define PCU_PPDU_SETUP_INIT_R2R_STA_PARTIAL_AID_LSB                                 10
#define PCU_PPDU_SETUP_INIT_R2R_STA_PARTIAL_AID_MSB                                 20
#define PCU_PPDU_SETUP_INIT_R2R_STA_PARTIAL_AID_MASK                                0x001ffc00

#define PCU_PPDU_SETUP_INIT_USE_ADDRESS_FIELDS_FOR_PROTECTION_OFFSET                0x000000bc
#define PCU_PPDU_SETUP_INIT_USE_ADDRESS_FIELDS_FOR_PROTECTION_LSB                   21
#define PCU_PPDU_SETUP_INIT_USE_ADDRESS_FIELDS_FOR_PROTECTION_MSB                   21
#define PCU_PPDU_SETUP_INIT_USE_ADDRESS_FIELDS_FOR_PROTECTION_MASK                  0x00200000

#define PCU_PPDU_SETUP_INIT_R2R_SET_REQUIRED_RESPONSE_TIME_OFFSET                   0x000000bc
#define PCU_PPDU_SETUP_INIT_R2R_SET_REQUIRED_RESPONSE_TIME_LSB                      22
#define PCU_PPDU_SETUP_INIT_R2R_SET_REQUIRED_RESPONSE_TIME_MSB                      22
#define PCU_PPDU_SETUP_INIT_R2R_SET_REQUIRED_RESPONSE_TIME_MASK                     0x00400000

#define PCU_PPDU_SETUP_INIT_RESERVED_29A_OFFSET                                     0x000000bc
#define PCU_PPDU_SETUP_INIT_RESERVED_29A_LSB                                        23
#define PCU_PPDU_SETUP_INIT_RESERVED_29A_MSB                                        25
#define PCU_PPDU_SETUP_INIT_RESERVED_29A_MASK                                       0x03800000

#define PCU_PPDU_SETUP_INIT_R2R_BW20_ACTIVE_CHANNEL_OFFSET                          0x000000bc
#define PCU_PPDU_SETUP_INIT_R2R_BW20_ACTIVE_CHANNEL_LSB                             26
#define PCU_PPDU_SETUP_INIT_R2R_BW20_ACTIVE_CHANNEL_MSB                             28
#define PCU_PPDU_SETUP_INIT_R2R_BW20_ACTIVE_CHANNEL_MASK                            0x1c000000

#define PCU_PPDU_SETUP_INIT_R2R_BW40_ACTIVE_CHANNEL_OFFSET                          0x000000bc
#define PCU_PPDU_SETUP_INIT_R2R_BW40_ACTIVE_CHANNEL_LSB                             29
#define PCU_PPDU_SETUP_INIT_R2R_BW40_ACTIVE_CHANNEL_MSB                             31
#define PCU_PPDU_SETUP_INIT_R2R_BW40_ACTIVE_CHANNEL_MASK                            0xe0000000

#define PCU_PPDU_SETUP_INIT_R2R_BW80_ACTIVE_CHANNEL_OFFSET                          0x000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW80_ACTIVE_CHANNEL_LSB                             0
#define PCU_PPDU_SETUP_INIT_R2R_BW80_ACTIVE_CHANNEL_MSB                             2
#define PCU_PPDU_SETUP_INIT_R2R_BW80_ACTIVE_CHANNEL_MASK                            0x00000007

#define PCU_PPDU_SETUP_INIT_R2R_BW160_ACTIVE_CHANNEL_OFFSET                         0x000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW160_ACTIVE_CHANNEL_LSB                            3
#define PCU_PPDU_SETUP_INIT_R2R_BW160_ACTIVE_CHANNEL_MSB                            5
#define PCU_PPDU_SETUP_INIT_R2R_BW160_ACTIVE_CHANNEL_MASK                           0x00000038

#define PCU_PPDU_SETUP_INIT_R2R_BW240_ACTIVE_CHANNEL_OFFSET                         0x000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW240_ACTIVE_CHANNEL_LSB                            6
#define PCU_PPDU_SETUP_INIT_R2R_BW240_ACTIVE_CHANNEL_MSB                            8
#define PCU_PPDU_SETUP_INIT_R2R_BW240_ACTIVE_CHANNEL_MASK                           0x000001c0

#define PCU_PPDU_SETUP_INIT_R2R_BW320_ACTIVE_CHANNEL_OFFSET                         0x000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW320_ACTIVE_CHANNEL_LSB                            9
#define PCU_PPDU_SETUP_INIT_R2R_BW320_ACTIVE_CHANNEL_MSB                            11
#define PCU_PPDU_SETUP_INIT_R2R_BW320_ACTIVE_CHANNEL_MASK                           0x00000e00

#define PCU_PPDU_SETUP_INIT_R2R_BW20_OFFSET                                         0x000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW20_LSB                                            12
#define PCU_PPDU_SETUP_INIT_R2R_BW20_MSB                                            14
#define PCU_PPDU_SETUP_INIT_R2R_BW20_MASK                                           0x00007000

#define PCU_PPDU_SETUP_INIT_R2R_BW40_OFFSET                                         0x000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW40_LSB                                            15
#define PCU_PPDU_SETUP_INIT_R2R_BW40_MSB                                            17
#define PCU_PPDU_SETUP_INIT_R2R_BW40_MASK                                           0x00038000

#define PCU_PPDU_SETUP_INIT_R2R_BW80_OFFSET                                         0x000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW80_LSB                                            18
#define PCU_PPDU_SETUP_INIT_R2R_BW80_MSB                                            20
#define PCU_PPDU_SETUP_INIT_R2R_BW80_MASK                                           0x001c0000

#define PCU_PPDU_SETUP_INIT_R2R_BW160_OFFSET                                        0x000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW160_LSB                                           21
#define PCU_PPDU_SETUP_INIT_R2R_BW160_MSB                                           23
#define PCU_PPDU_SETUP_INIT_R2R_BW160_MASK                                          0x00e00000

#define PCU_PPDU_SETUP_INIT_R2R_BW240_OFFSET                                        0x000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW240_LSB                                           24
#define PCU_PPDU_SETUP_INIT_R2R_BW240_MSB                                           26
#define PCU_PPDU_SETUP_INIT_R2R_BW240_MASK                                          0x07000000

#define PCU_PPDU_SETUP_INIT_R2R_BW320_OFFSET                                        0x000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW320_LSB                                           27
#define PCU_PPDU_SETUP_INIT_R2R_BW320_MSB                                           29
#define PCU_PPDU_SETUP_INIT_R2R_BW320_MASK                                          0x38000000

#define PCU_PPDU_SETUP_INIT_RESERVED_30A_OFFSET                                     0x000000c0
#define PCU_PPDU_SETUP_INIT_RESERVED_30A_LSB                                        30
#define PCU_PPDU_SETUP_INIT_RESERVED_30A_MSB                                        31
#define PCU_PPDU_SETUP_INIT_RESERVED_30A_MASK                                       0xc0000000

#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_31_0_OFFSET                 0x000000c4
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_31_0_LSB                    0
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_31_0_MSB                    31
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_31_0_MASK                   0xffffffff

#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_36_32_OFFSET                0x000000c8
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_36_32_LSB                   0
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_36_32_MSB                   4
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_36_32_MASK                  0x0000001f

#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_CBF_COUNT_OFFSET                   0x000000c8
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_CBF_COUNT_LSB                      5
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_CBF_COUNT_MSB                      10
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_CBF_COUNT_MASK                     0x000007e0

#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_STA_COUNT_OFFSET                   0x000000c8
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_STA_COUNT_LSB                      11
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_STA_COUNT_MSB                      16
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_STA_COUNT_MASK                     0x0001f800

#define PCU_PPDU_SETUP_INIT_TRANSMIT_INCLUDES_MULTIDESTINATION_OFFSET               0x000000c8
#define PCU_PPDU_SETUP_INIT_TRANSMIT_INCLUDES_MULTIDESTINATION_LSB                  17
#define PCU_PPDU_SETUP_INIT_TRANSMIT_INCLUDES_MULTIDESTINATION_MSB                  17
#define PCU_PPDU_SETUP_INIT_TRANSMIT_INCLUDES_MULTIDESTINATION_MASK                 0x00020000

#define PCU_PPDU_SETUP_INIT_INSERT_PREV_TX_START_TIMING_INFO_OFFSET                 0x000000c8
#define PCU_PPDU_SETUP_INIT_INSERT_PREV_TX_START_TIMING_INFO_LSB                    18
#define PCU_PPDU_SETUP_INIT_INSERT_PREV_TX_START_TIMING_INFO_MSB                    18
#define PCU_PPDU_SETUP_INIT_INSERT_PREV_TX_START_TIMING_INFO_MASK                   0x00040000

#define PCU_PPDU_SETUP_INIT_INSERT_CURRENT_TX_START_TIMING_INFO_OFFSET              0x000000c8
#define PCU_PPDU_SETUP_INIT_INSERT_CURRENT_TX_START_TIMING_INFO_LSB                 19
#define PCU_PPDU_SETUP_INIT_INSERT_CURRENT_TX_START_TIMING_INFO_MSB                 19
#define PCU_PPDU_SETUP_INIT_INSERT_CURRENT_TX_START_TIMING_INFO_MASK                0x00080000

#define PCU_PPDU_SETUP_INIT_TX_START_TRANSMIT_TIME_BYTE_OFFSET_OFFSET               0x000000c8
#define PCU_PPDU_SETUP_INIT_TX_START_TRANSMIT_TIME_BYTE_OFFSET_LSB                  20
#define PCU_PPDU_SETUP_INIT_TX_START_TRANSMIT_TIME_BYTE_OFFSET_MSB                  31
#define PCU_PPDU_SETUP_INIT_TX_START_TRANSMIT_TIME_BYTE_OFFSET_MASK                 0xfff00000

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_31_0_OFFSET                        0x000000cc
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_31_0_LSB                           0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_31_0_MSB                           31
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_31_0_MASK                          0xffffffff

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_47_32_OFFSET                       0x000000d0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_47_32_LSB                          0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_47_32_MSB                          15
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_47_32_MASK                         0x0000ffff

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_15_0_OFFSET                        0x000000d0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_15_0_LSB                           16
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_15_0_MSB                           31
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_15_0_MASK                          0xffff0000

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_47_16_OFFSET                       0x000000d4
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_47_16_LSB                          0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_47_16_MSB                          31
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_47_16_MASK                         0xffffffff

#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_THRESHOLD_OFFSET                    0x000000d8
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_THRESHOLD_LSB                       0
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_THRESHOLD_MSB                       23
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_THRESHOLD_MASK                      0x00ffffff

#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_TYPE_OFFSET                         0x000000d8
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_TYPE_LSB                            24
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_TYPE_MSB                            24
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_TYPE_MASK                           0x01000000

#define PCU_PPDU_SETUP_INIT_RESERVED_54A_OFFSET                                     0x000000d8
#define PCU_PPDU_SETUP_INIT_RESERVED_54A_LSB                                        25
#define PCU_PPDU_SETUP_INIT_RESERVED_54A_MSB                                        31
#define PCU_PPDU_SETUP_INIT_RESERVED_54A_MASK                                       0xfe000000

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_31_0_OFFSET                        0x000000dc
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_31_0_LSB                           0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_31_0_MSB                           31
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_31_0_MASK                          0xffffffff

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_47_32_OFFSET                       0x000000e0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_47_32_LSB                          0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_47_32_MSB                          15
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_47_32_MASK                         0x0000ffff

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_15_0_OFFSET                        0x000000e0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_15_0_LSB                           16
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_15_0_MSB                           31
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_15_0_MASK                          0xffff0000

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_47_16_OFFSET                       0x000000e4
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_47_16_LSB                          0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_47_16_MSB                          31
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_47_16_MASK                         0xffffffff

#endif
