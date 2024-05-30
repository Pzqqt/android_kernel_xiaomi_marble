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


#ifndef _TX_CBF_INFO_H_
#define _TX_CBF_INFO_H_

#define NUM_OF_DWORDS_TX_CBF_INFO 15

struct tx_cbf_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t sw_peer_id                                              : 16,
                      pre_cbf_duration                                        : 16;
             uint32_t brpoll_info_valid                                       :  1,
                      trigger_brpoll_info_valid                               :  1,
                      npda_info_11ac_valid                                    :  1,
                      npda_info_11ax_valid                                    :  1,
                      dot11ax_su_extended                                     :  1,
                      bandwidth                                               :  3,
                      brpoll_info                                             :  8,
                      cbf_response_table_base_index                           :  8,
                      peer_index                                              :  3,
                      pkt_type                                                :  4,
                      txop_duration_all_ones                                  :  1;
             uint32_t trigger_brpoll_common_info_15_0                         : 16,
                      trigger_brpoll_common_info_31_16                        : 16;
             uint32_t trigger_brpoll_user_info_15_0                           : 16,
                      trigger_brpoll_user_info_31_16                          : 16;
             uint32_t addr1_31_0                                              : 32;
             uint32_t addr1_47_32                                             : 16,
                      addr2_15_0                                              : 16;
             uint32_t addr2_47_16                                             : 32;
             uint32_t addr3_31_0                                              : 32;
             uint32_t addr3_47_32                                             : 16,
                      sta_partial_aid                                         : 11,
                      reserved_8a                                             :  4,
                      cbf_resp_pwr_mgmt                                       :  1;
             uint32_t group_id                                                :  6,
                      rssi_comb                                               :  8,
                      reserved_9a                                             :  2,
                      vht_ndpa_sta_info                                       : 16;
             uint32_t he_eht_sta_info_15_0                                    : 16,
                      he_eht_sta_info_31_16                                   : 16;
             uint32_t dot11ax_received_format_indication                      :  1,
                      dot11ax_received_dl_ul_flag                             :  1,
                      dot11ax_received_bss_color_id                           :  6,
                      dot11ax_received_spatial_reuse                          :  4,
                      dot11ax_received_cp_size                                :  2,
                      dot11ax_received_ltf_size                               :  2,
                      dot11ax_received_coding                                 :  1,
                      dot11ax_received_dcm                                    :  1,
                      dot11ax_received_doppler_indication                     :  1,
                      dot11ax_received_ext_ru_size                            :  4,
                      dot11ax_dl_ul_flag                                      :  1,
                      reserved_11a                                            :  8;
             uint32_t sw_response_frame_length                                : 16,
                      sw_response_tlv_from_crypto                             :  1,
                      wait_sifs_config_valid                                  :  1,
                      wait_sifs                                               :  2,
                      __reserved_g_0005                                                 :  1,
                      secure                                                  :  1,
                      tb___reserved_g_0005_response_required                            :  2,
                      emlsr_main_tlv_if                                       :  1,
                      reserved_12a                                            :  1,
                      u_sig_puncture_pattern_encoding                         :  6;
             uint32_t dot11be_puncture_bitmap                                 : 16,
                      dot11be_response                                        :  1,
                      punctured_response                                      :  1,
                      npda_info_11be_valid                                    :  1,
                      eht_duplicate_mode                                      :  2,
                      reserved_13a                                            : 11;
             uint32_t eht_sta_info_39_32                                      :  8,
                      reserved_14a                                            : 24;
#else
             uint32_t pre_cbf_duration                                        : 16,
                      sw_peer_id                                              : 16;
             uint32_t txop_duration_all_ones                                  :  1,
                      pkt_type                                                :  4,
                      peer_index                                              :  3,
                      cbf_response_table_base_index                           :  8,
                      brpoll_info                                             :  8,
                      bandwidth                                               :  3,
                      dot11ax_su_extended                                     :  1,
                      npda_info_11ax_valid                                    :  1,
                      npda_info_11ac_valid                                    :  1,
                      trigger_brpoll_info_valid                               :  1,
                      brpoll_info_valid                                       :  1;
             uint32_t trigger_brpoll_common_info_31_16                        : 16,
                      trigger_brpoll_common_info_15_0                         : 16;
             uint32_t trigger_brpoll_user_info_31_16                          : 16,
                      trigger_brpoll_user_info_15_0                           : 16;
             uint32_t addr1_31_0                                              : 32;
             uint32_t addr2_15_0                                              : 16,
                      addr1_47_32                                             : 16;
             uint32_t addr2_47_16                                             : 32;
             uint32_t addr3_31_0                                              : 32;
             uint32_t cbf_resp_pwr_mgmt                                       :  1,
                      reserved_8a                                             :  4,
                      sta_partial_aid                                         : 11,
                      addr3_47_32                                             : 16;
             uint32_t vht_ndpa_sta_info                                       : 16,
                      reserved_9a                                             :  2,
                      rssi_comb                                               :  8,
                      group_id                                                :  6;
             uint32_t he_eht_sta_info_31_16                                   : 16,
                      he_eht_sta_info_15_0                                    : 16;
             uint32_t reserved_11a                                            :  8,
                      dot11ax_dl_ul_flag                                      :  1,
                      dot11ax_received_ext_ru_size                            :  4,
                      dot11ax_received_doppler_indication                     :  1,
                      dot11ax_received_dcm                                    :  1,
                      dot11ax_received_coding                                 :  1,
                      dot11ax_received_ltf_size                               :  2,
                      dot11ax_received_cp_size                                :  2,
                      dot11ax_received_spatial_reuse                          :  4,
                      dot11ax_received_bss_color_id                           :  6,
                      dot11ax_received_dl_ul_flag                             :  1,
                      dot11ax_received_format_indication                      :  1;
             uint32_t u_sig_puncture_pattern_encoding                         :  6,
                      reserved_12a                                            :  1,
                      emlsr_main_tlv_if                                       :  1,
                      tb___reserved_g_0005_response_required                            :  2,
                      secure                                                  :  1,
                      __reserved_g_0005                                                 :  1,
                      wait_sifs                                               :  2,
                      wait_sifs_config_valid                                  :  1,
                      sw_response_tlv_from_crypto                             :  1,
                      sw_response_frame_length                                : 16;
             uint32_t reserved_13a                                            : 11,
                      eht_duplicate_mode                                      :  2,
                      npda_info_11be_valid                                    :  1,
                      punctured_response                                      :  1,
                      dot11be_response                                        :  1,
                      dot11be_puncture_bitmap                                 : 16;
             uint32_t reserved_14a                                            : 24,
                      eht_sta_info_39_32                                      :  8;
#endif
};

#define TX_CBF_INFO_SW_PEER_ID_OFFSET                                               0x00000000
#define TX_CBF_INFO_SW_PEER_ID_LSB                                                  0
#define TX_CBF_INFO_SW_PEER_ID_MSB                                                  15
#define TX_CBF_INFO_SW_PEER_ID_MASK                                                 0x0000ffff

#define TX_CBF_INFO_PRE_CBF_DURATION_OFFSET                                         0x00000000
#define TX_CBF_INFO_PRE_CBF_DURATION_LSB                                            16
#define TX_CBF_INFO_PRE_CBF_DURATION_MSB                                            31
#define TX_CBF_INFO_PRE_CBF_DURATION_MASK                                           0xffff0000

#define TX_CBF_INFO_BRPOLL_INFO_VALID_OFFSET                                        0x00000004
#define TX_CBF_INFO_BRPOLL_INFO_VALID_LSB                                           0
#define TX_CBF_INFO_BRPOLL_INFO_VALID_MSB                                           0
#define TX_CBF_INFO_BRPOLL_INFO_VALID_MASK                                          0x00000001

#define TX_CBF_INFO_TRIGGER_BRPOLL_INFO_VALID_OFFSET                                0x00000004
#define TX_CBF_INFO_TRIGGER_BRPOLL_INFO_VALID_LSB                                   1
#define TX_CBF_INFO_TRIGGER_BRPOLL_INFO_VALID_MSB                                   1
#define TX_CBF_INFO_TRIGGER_BRPOLL_INFO_VALID_MASK                                  0x00000002

#define TX_CBF_INFO_NPDA_INFO_11AC_VALID_OFFSET                                     0x00000004
#define TX_CBF_INFO_NPDA_INFO_11AC_VALID_LSB                                        2
#define TX_CBF_INFO_NPDA_INFO_11AC_VALID_MSB                                        2
#define TX_CBF_INFO_NPDA_INFO_11AC_VALID_MASK                                       0x00000004

#define TX_CBF_INFO_NPDA_INFO_11AX_VALID_OFFSET                                     0x00000004
#define TX_CBF_INFO_NPDA_INFO_11AX_VALID_LSB                                        3
#define TX_CBF_INFO_NPDA_INFO_11AX_VALID_MSB                                        3
#define TX_CBF_INFO_NPDA_INFO_11AX_VALID_MASK                                       0x00000008

#define TX_CBF_INFO_DOT11AX_SU_EXTENDED_OFFSET                                      0x00000004
#define TX_CBF_INFO_DOT11AX_SU_EXTENDED_LSB                                         4
#define TX_CBF_INFO_DOT11AX_SU_EXTENDED_MSB                                         4
#define TX_CBF_INFO_DOT11AX_SU_EXTENDED_MASK                                        0x00000010

#define TX_CBF_INFO_BANDWIDTH_OFFSET                                                0x00000004
#define TX_CBF_INFO_BANDWIDTH_LSB                                                   5
#define TX_CBF_INFO_BANDWIDTH_MSB                                                   7
#define TX_CBF_INFO_BANDWIDTH_MASK                                                  0x000000e0

#define TX_CBF_INFO_BRPOLL_INFO_OFFSET                                              0x00000004
#define TX_CBF_INFO_BRPOLL_INFO_LSB                                                 8
#define TX_CBF_INFO_BRPOLL_INFO_MSB                                                 15
#define TX_CBF_INFO_BRPOLL_INFO_MASK                                                0x0000ff00

#define TX_CBF_INFO_CBF_RESPONSE_TABLE_BASE_INDEX_OFFSET                            0x00000004
#define TX_CBF_INFO_CBF_RESPONSE_TABLE_BASE_INDEX_LSB                               16
#define TX_CBF_INFO_CBF_RESPONSE_TABLE_BASE_INDEX_MSB                               23
#define TX_CBF_INFO_CBF_RESPONSE_TABLE_BASE_INDEX_MASK                              0x00ff0000

#define TX_CBF_INFO_PEER_INDEX_OFFSET                                               0x00000004
#define TX_CBF_INFO_PEER_INDEX_LSB                                                  24
#define TX_CBF_INFO_PEER_INDEX_MSB                                                  26
#define TX_CBF_INFO_PEER_INDEX_MASK                                                 0x07000000

#define TX_CBF_INFO_PKT_TYPE_OFFSET                                                 0x00000004
#define TX_CBF_INFO_PKT_TYPE_LSB                                                    27
#define TX_CBF_INFO_PKT_TYPE_MSB                                                    30
#define TX_CBF_INFO_PKT_TYPE_MASK                                                   0x78000000

#define TX_CBF_INFO_TXOP_DURATION_ALL_ONES_OFFSET                                   0x00000004
#define TX_CBF_INFO_TXOP_DURATION_ALL_ONES_LSB                                      31
#define TX_CBF_INFO_TXOP_DURATION_ALL_ONES_MSB                                      31
#define TX_CBF_INFO_TXOP_DURATION_ALL_ONES_MASK                                     0x80000000

#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_15_0_OFFSET                          0x00000008
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_15_0_LSB                             0
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_15_0_MSB                             15
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_15_0_MASK                            0x0000ffff

#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_31_16_OFFSET                         0x00000008
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_31_16_LSB                            16
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_31_16_MSB                            31
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_31_16_MASK                           0xffff0000

#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_15_0_OFFSET                            0x0000000c
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_15_0_LSB                               0
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_15_0_MSB                               15
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_15_0_MASK                              0x0000ffff

#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_31_16_OFFSET                           0x0000000c
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_31_16_LSB                              16
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_31_16_MSB                              31
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_31_16_MASK                             0xffff0000

#define TX_CBF_INFO_ADDR1_31_0_OFFSET                                               0x00000010
#define TX_CBF_INFO_ADDR1_31_0_LSB                                                  0
#define TX_CBF_INFO_ADDR1_31_0_MSB                                                  31
#define TX_CBF_INFO_ADDR1_31_0_MASK                                                 0xffffffff

#define TX_CBF_INFO_ADDR1_47_32_OFFSET                                              0x00000014
#define TX_CBF_INFO_ADDR1_47_32_LSB                                                 0
#define TX_CBF_INFO_ADDR1_47_32_MSB                                                 15
#define TX_CBF_INFO_ADDR1_47_32_MASK                                                0x0000ffff

#define TX_CBF_INFO_ADDR2_15_0_OFFSET                                               0x00000014
#define TX_CBF_INFO_ADDR2_15_0_LSB                                                  16
#define TX_CBF_INFO_ADDR2_15_0_MSB                                                  31
#define TX_CBF_INFO_ADDR2_15_0_MASK                                                 0xffff0000

#define TX_CBF_INFO_ADDR2_47_16_OFFSET                                              0x00000018
#define TX_CBF_INFO_ADDR2_47_16_LSB                                                 0
#define TX_CBF_INFO_ADDR2_47_16_MSB                                                 31
#define TX_CBF_INFO_ADDR2_47_16_MASK                                                0xffffffff

#define TX_CBF_INFO_ADDR3_31_0_OFFSET                                               0x0000001c
#define TX_CBF_INFO_ADDR3_31_0_LSB                                                  0
#define TX_CBF_INFO_ADDR3_31_0_MSB                                                  31
#define TX_CBF_INFO_ADDR3_31_0_MASK                                                 0xffffffff

#define TX_CBF_INFO_ADDR3_47_32_OFFSET                                              0x00000020
#define TX_CBF_INFO_ADDR3_47_32_LSB                                                 0
#define TX_CBF_INFO_ADDR3_47_32_MSB                                                 15
#define TX_CBF_INFO_ADDR3_47_32_MASK                                                0x0000ffff

#define TX_CBF_INFO_STA_PARTIAL_AID_OFFSET                                          0x00000020
#define TX_CBF_INFO_STA_PARTIAL_AID_LSB                                             16
#define TX_CBF_INFO_STA_PARTIAL_AID_MSB                                             26
#define TX_CBF_INFO_STA_PARTIAL_AID_MASK                                            0x07ff0000

#define TX_CBF_INFO_RESERVED_8A_OFFSET                                              0x00000020
#define TX_CBF_INFO_RESERVED_8A_LSB                                                 27
#define TX_CBF_INFO_RESERVED_8A_MSB                                                 30
#define TX_CBF_INFO_RESERVED_8A_MASK                                                0x78000000

#define TX_CBF_INFO_CBF_RESP_PWR_MGMT_OFFSET                                        0x00000020
#define TX_CBF_INFO_CBF_RESP_PWR_MGMT_LSB                                           31
#define TX_CBF_INFO_CBF_RESP_PWR_MGMT_MSB                                           31
#define TX_CBF_INFO_CBF_RESP_PWR_MGMT_MASK                                          0x80000000

#define TX_CBF_INFO_GROUP_ID_OFFSET                                                 0x00000024
#define TX_CBF_INFO_GROUP_ID_LSB                                                    0
#define TX_CBF_INFO_GROUP_ID_MSB                                                    5
#define TX_CBF_INFO_GROUP_ID_MASK                                                   0x0000003f

#define TX_CBF_INFO_RSSI_COMB_OFFSET                                                0x00000024
#define TX_CBF_INFO_RSSI_COMB_LSB                                                   6
#define TX_CBF_INFO_RSSI_COMB_MSB                                                   13
#define TX_CBF_INFO_RSSI_COMB_MASK                                                  0x00003fc0

#define TX_CBF_INFO_RESERVED_9A_OFFSET                                              0x00000024
#define TX_CBF_INFO_RESERVED_9A_LSB                                                 14
#define TX_CBF_INFO_RESERVED_9A_MSB                                                 15
#define TX_CBF_INFO_RESERVED_9A_MASK                                                0x0000c000

#define TX_CBF_INFO_VHT_NDPA_STA_INFO_OFFSET                                        0x00000024
#define TX_CBF_INFO_VHT_NDPA_STA_INFO_LSB                                           16
#define TX_CBF_INFO_VHT_NDPA_STA_INFO_MSB                                           31
#define TX_CBF_INFO_VHT_NDPA_STA_INFO_MASK                                          0xffff0000

#define TX_CBF_INFO_HE_EHT_STA_INFO_15_0_OFFSET                                     0x00000028
#define TX_CBF_INFO_HE_EHT_STA_INFO_15_0_LSB                                        0
#define TX_CBF_INFO_HE_EHT_STA_INFO_15_0_MSB                                        15
#define TX_CBF_INFO_HE_EHT_STA_INFO_15_0_MASK                                       0x0000ffff

#define TX_CBF_INFO_HE_EHT_STA_INFO_31_16_OFFSET                                    0x00000028
#define TX_CBF_INFO_HE_EHT_STA_INFO_31_16_LSB                                       16
#define TX_CBF_INFO_HE_EHT_STA_INFO_31_16_MSB                                       31
#define TX_CBF_INFO_HE_EHT_STA_INFO_31_16_MASK                                      0xffff0000

#define TX_CBF_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_OFFSET                       0x0000002c
#define TX_CBF_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_LSB                          0
#define TX_CBF_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_MSB                          0
#define TX_CBF_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_MASK                         0x00000001

#define TX_CBF_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_OFFSET                              0x0000002c
#define TX_CBF_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_LSB                                 1
#define TX_CBF_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_MSB                                 1
#define TX_CBF_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_MASK                                0x00000002

#define TX_CBF_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_OFFSET                            0x0000002c
#define TX_CBF_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_LSB                               2
#define TX_CBF_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_MSB                               7
#define TX_CBF_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_MASK                              0x000000fc

#define TX_CBF_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_OFFSET                           0x0000002c
#define TX_CBF_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_LSB                              8
#define TX_CBF_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_MSB                              11
#define TX_CBF_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_MASK                             0x00000f00

#define TX_CBF_INFO_DOT11AX_RECEIVED_CP_SIZE_OFFSET                                 0x0000002c
#define TX_CBF_INFO_DOT11AX_RECEIVED_CP_SIZE_LSB                                    12
#define TX_CBF_INFO_DOT11AX_RECEIVED_CP_SIZE_MSB                                    13
#define TX_CBF_INFO_DOT11AX_RECEIVED_CP_SIZE_MASK                                   0x00003000

#define TX_CBF_INFO_DOT11AX_RECEIVED_LTF_SIZE_OFFSET                                0x0000002c
#define TX_CBF_INFO_DOT11AX_RECEIVED_LTF_SIZE_LSB                                   14
#define TX_CBF_INFO_DOT11AX_RECEIVED_LTF_SIZE_MSB                                   15
#define TX_CBF_INFO_DOT11AX_RECEIVED_LTF_SIZE_MASK                                  0x0000c000

#define TX_CBF_INFO_DOT11AX_RECEIVED_CODING_OFFSET                                  0x0000002c
#define TX_CBF_INFO_DOT11AX_RECEIVED_CODING_LSB                                     16
#define TX_CBF_INFO_DOT11AX_RECEIVED_CODING_MSB                                     16
#define TX_CBF_INFO_DOT11AX_RECEIVED_CODING_MASK                                    0x00010000

#define TX_CBF_INFO_DOT11AX_RECEIVED_DCM_OFFSET                                     0x0000002c
#define TX_CBF_INFO_DOT11AX_RECEIVED_DCM_LSB                                        17
#define TX_CBF_INFO_DOT11AX_RECEIVED_DCM_MSB                                        17
#define TX_CBF_INFO_DOT11AX_RECEIVED_DCM_MASK                                       0x00020000

#define TX_CBF_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_OFFSET                      0x0000002c
#define TX_CBF_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_LSB                         18
#define TX_CBF_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_MSB                         18
#define TX_CBF_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_MASK                        0x00040000

#define TX_CBF_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_OFFSET                             0x0000002c
#define TX_CBF_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_LSB                                19
#define TX_CBF_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_MSB                                22
#define TX_CBF_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_MASK                               0x00780000

#define TX_CBF_INFO_DOT11AX_DL_UL_FLAG_OFFSET                                       0x0000002c
#define TX_CBF_INFO_DOT11AX_DL_UL_FLAG_LSB                                          23
#define TX_CBF_INFO_DOT11AX_DL_UL_FLAG_MSB                                          23
#define TX_CBF_INFO_DOT11AX_DL_UL_FLAG_MASK                                         0x00800000

#define TX_CBF_INFO_RESERVED_11A_OFFSET                                             0x0000002c
#define TX_CBF_INFO_RESERVED_11A_LSB                                                24
#define TX_CBF_INFO_RESERVED_11A_MSB                                                31
#define TX_CBF_INFO_RESERVED_11A_MASK                                               0xff000000

#define TX_CBF_INFO_SW_RESPONSE_FRAME_LENGTH_OFFSET                                 0x00000030
#define TX_CBF_INFO_SW_RESPONSE_FRAME_LENGTH_LSB                                    0
#define TX_CBF_INFO_SW_RESPONSE_FRAME_LENGTH_MSB                                    15
#define TX_CBF_INFO_SW_RESPONSE_FRAME_LENGTH_MASK                                   0x0000ffff

#define TX_CBF_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_OFFSET                              0x00000030
#define TX_CBF_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_LSB                                 16
#define TX_CBF_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_MSB                                 16
#define TX_CBF_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_MASK                                0x00010000

#define TX_CBF_INFO_WAIT_SIFS_CONFIG_VALID_OFFSET                                   0x00000030
#define TX_CBF_INFO_WAIT_SIFS_CONFIG_VALID_LSB                                      17
#define TX_CBF_INFO_WAIT_SIFS_CONFIG_VALID_MSB                                      17
#define TX_CBF_INFO_WAIT_SIFS_CONFIG_VALID_MASK                                     0x00020000

#define TX_CBF_INFO_WAIT_SIFS_OFFSET                                                0x00000030
#define TX_CBF_INFO_WAIT_SIFS_LSB                                                   18
#define TX_CBF_INFO_WAIT_SIFS_MSB                                                   19
#define TX_CBF_INFO_WAIT_SIFS_MASK                                                  0x000c0000

#define TX_CBF_INFO_SECURE_OFFSET                                                   0x00000030
#define TX_CBF_INFO_SECURE_LSB                                                      21
#define TX_CBF_INFO_SECURE_MSB                                                      21
#define TX_CBF_INFO_SECURE_MASK                                                     0x00200000

#define TX_CBF_INFO_TB_RANGING_RESPONSE_REQUIRED_OFFSET                             0x00000030
#define TX_CBF_INFO_TB_RANGING_RESPONSE_REQUIRED_LSB                                22
#define TX_CBF_INFO_TB_RANGING_RESPONSE_REQUIRED_MSB                                23
#define TX_CBF_INFO_TB_RANGING_RESPONSE_REQUIRED_MASK                               0x00c00000

#define TX_CBF_INFO_EMLSR_MAIN_TLV_IF_OFFSET                                        0x00000030
#define TX_CBF_INFO_EMLSR_MAIN_TLV_IF_LSB                                           24
#define TX_CBF_INFO_EMLSR_MAIN_TLV_IF_MSB                                           24
#define TX_CBF_INFO_EMLSR_MAIN_TLV_IF_MASK                                          0x01000000

#define TX_CBF_INFO_RESERVED_12A_OFFSET                                             0x00000030
#define TX_CBF_INFO_RESERVED_12A_LSB                                                25
#define TX_CBF_INFO_RESERVED_12A_MSB                                                25
#define TX_CBF_INFO_RESERVED_12A_MASK                                               0x02000000

#define TX_CBF_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET                          0x00000030
#define TX_CBF_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB                             26
#define TX_CBF_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB                             31
#define TX_CBF_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK                            0xfc000000

#define TX_CBF_INFO_DOT11BE_PUNCTURE_BITMAP_OFFSET                                  0x00000034
#define TX_CBF_INFO_DOT11BE_PUNCTURE_BITMAP_LSB                                     0
#define TX_CBF_INFO_DOT11BE_PUNCTURE_BITMAP_MSB                                     15
#define TX_CBF_INFO_DOT11BE_PUNCTURE_BITMAP_MASK                                    0x0000ffff

#define TX_CBF_INFO_DOT11BE_RESPONSE_OFFSET                                         0x00000034
#define TX_CBF_INFO_DOT11BE_RESPONSE_LSB                                            16
#define TX_CBF_INFO_DOT11BE_RESPONSE_MSB                                            16
#define TX_CBF_INFO_DOT11BE_RESPONSE_MASK                                           0x00010000

#define TX_CBF_INFO_PUNCTURED_RESPONSE_OFFSET                                       0x00000034
#define TX_CBF_INFO_PUNCTURED_RESPONSE_LSB                                          17
#define TX_CBF_INFO_PUNCTURED_RESPONSE_MSB                                          17
#define TX_CBF_INFO_PUNCTURED_RESPONSE_MASK                                         0x00020000

#define TX_CBF_INFO_NPDA_INFO_11BE_VALID_OFFSET                                     0x00000034
#define TX_CBF_INFO_NPDA_INFO_11BE_VALID_LSB                                        18
#define TX_CBF_INFO_NPDA_INFO_11BE_VALID_MSB                                        18
#define TX_CBF_INFO_NPDA_INFO_11BE_VALID_MASK                                       0x00040000

#define TX_CBF_INFO_EHT_DUPLICATE_MODE_OFFSET                                       0x00000034
#define TX_CBF_INFO_EHT_DUPLICATE_MODE_LSB                                          19
#define TX_CBF_INFO_EHT_DUPLICATE_MODE_MSB                                          20
#define TX_CBF_INFO_EHT_DUPLICATE_MODE_MASK                                         0x00180000

#define TX_CBF_INFO_RESERVED_13A_OFFSET                                             0x00000034
#define TX_CBF_INFO_RESERVED_13A_LSB                                                21
#define TX_CBF_INFO_RESERVED_13A_MSB                                                31
#define TX_CBF_INFO_RESERVED_13A_MASK                                               0xffe00000

#define TX_CBF_INFO_EHT_STA_INFO_39_32_OFFSET                                       0x00000038
#define TX_CBF_INFO_EHT_STA_INFO_39_32_LSB                                          0
#define TX_CBF_INFO_EHT_STA_INFO_39_32_MSB                                          7
#define TX_CBF_INFO_EHT_STA_INFO_39_32_MASK                                         0x000000ff

#define TX_CBF_INFO_RESERVED_14A_OFFSET                                             0x00000038
#define TX_CBF_INFO_RESERVED_14A_LSB                                                8
#define TX_CBF_INFO_RESERVED_14A_MSB                                                31
#define TX_CBF_INFO_RESERVED_14A_MASK                                               0xffffff00

#endif
