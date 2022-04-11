
/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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










#ifndef _PDG_RESPONSE_RATE_SETTING_H_
#define _PDG_RESPONSE_RATE_SETTING_H_
#if !defined(__ASSEMBLER__)
#endif

#include "mlo_sta_id_details.h"
#define NUM_OF_DWORDS_PDG_RESPONSE_RATE_SETTING 7


struct pdg_response_rate_setting {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t reserved_0a                                             :  1, // [0:0]
                      tx_antenna_sector_ctrl                                  : 24, // [24:1]
                      pkt_type                                                :  4, // [28:25]
                      smoothing                                               :  1, // [29:29]
                      ldpc                                                    :  1, // [30:30]
                      stbc                                                    :  1; // [31:31]
             uint32_t alt_tx_pwr                                              :  8, // [7:0]
                      alt_min_tx_pwr                                          :  8, // [15:8]
                      alt_nss                                                 :  3, // [18:16]
                      alt_tx_chain_mask                                       :  8, // [26:19]
                      alt_bw                                                  :  3, // [29:27]
                      stf_ltf_3db_boost                                       :  1, // [30:30]
                      force_extra_symbol                                      :  1; // [31:31]
             uint32_t alt_rate_mcs                                            :  4, // [3:0]
                      nss                                                     :  3, // [6:4]
                      dpd_enable                                              :  1, // [7:7]
                      tx_pwr                                                  :  8, // [15:8]
                      min_tx_pwr                                              :  8, // [23:16]
                      tx_chain_mask                                           :  8; // [31:24]
             uint32_t reserved_3a                                             :  8, // [7:0]
                      sgi                                                     :  2, // [9:8]
                      rate_mcs                                                :  4, // [13:10]
                      reserved_3b                                             :  2, // [15:14]
                      tx_pwr_1                                                :  8, // [23:16]
                      alt_tx_pwr_1                                            :  8; // [31:24]
             uint32_t aggregation                                             :  1, // [0:0]
                      dot11ax_bss_color_id                                    :  6, // [6:1]
                      dot11ax_spatial_reuse                                   :  4, // [10:7]
                      dot11ax_cp_ltf_size                                     :  2, // [12:11]
                      dot11ax_dcm                                             :  1, // [13:13]
                      dot11ax_doppler_indication                              :  1, // [14:14]
                      dot11ax_su_extended                                     :  1, // [15:15]
                      dot11ax_min_packet_extension                            :  2, // [17:16]
                      dot11ax_pe_nss                                          :  3, // [20:18]
                      dot11ax_pe_content                                      :  1, // [21:21]
                      dot11ax_pe_ltf_size                                     :  2, // [23:22]
                      dot11ax_chain_csd_en                                    :  1, // [24:24]
                      dot11ax_pe_chain_csd_en                                 :  1, // [25:25]
                      dot11ax_dl_ul_flag                                      :  1, // [26:26]
                      reserved_4a                                             :  5; // [31:27]
             uint32_t dot11ax_ext_ru_start_index                              :  4, // [3:0]
                      dot11ax_ext_ru_size                                     :  4, // [7:4]
                      eht_duplicate_mode                                      :  2, // [9:8]
                      he_sigb_dcm                                             :  1, // [10:10]
                      he_sigb_0_mcs                                           :  3, // [13:11]
                      num_he_sigb_sym                                         :  5, // [18:14]
                      required_response_time_source                           :  1, // [19:19]
                      reserved_5a                                             :  6, // [25:20]
                      u_sig_puncture_pattern_encoding                         :  6; // [31:26]
             struct   mlo_sta_id_details                                        mlo_sta_id_details_rx;
             uint16_t required_response_time                                  : 12, // [27:16]
                      dot11be_params_placeholder                              :  4; // [31:28]
#else
             uint32_t stbc                                                    :  1, // [31:31]
                      ldpc                                                    :  1, // [30:30]
                      smoothing                                               :  1, // [29:29]
                      pkt_type                                                :  4, // [28:25]
                      tx_antenna_sector_ctrl                                  : 24, // [24:1]
                      reserved_0a                                             :  1; // [0:0]
             uint32_t force_extra_symbol                                      :  1, // [31:31]
                      stf_ltf_3db_boost                                       :  1, // [30:30]
                      alt_bw                                                  :  3, // [29:27]
                      alt_tx_chain_mask                                       :  8, // [26:19]
                      alt_nss                                                 :  3, // [18:16]
                      alt_min_tx_pwr                                          :  8, // [15:8]
                      alt_tx_pwr                                              :  8; // [7:0]
             uint32_t tx_chain_mask                                           :  8, // [31:24]
                      min_tx_pwr                                              :  8, // [23:16]
                      tx_pwr                                                  :  8, // [15:8]
                      dpd_enable                                              :  1, // [7:7]
                      nss                                                     :  3, // [6:4]
                      alt_rate_mcs                                            :  4; // [3:0]
             uint32_t alt_tx_pwr_1                                            :  8, // [31:24]
                      tx_pwr_1                                                :  8, // [23:16]
                      reserved_3b                                             :  2, // [15:14]
                      rate_mcs                                                :  4, // [13:10]
                      sgi                                                     :  2, // [9:8]
                      reserved_3a                                             :  8; // [7:0]
             uint32_t reserved_4a                                             :  5, // [31:27]
                      dot11ax_dl_ul_flag                                      :  1, // [26:26]
                      dot11ax_pe_chain_csd_en                                 :  1, // [25:25]
                      dot11ax_chain_csd_en                                    :  1, // [24:24]
                      dot11ax_pe_ltf_size                                     :  2, // [23:22]
                      dot11ax_pe_content                                      :  1, // [21:21]
                      dot11ax_pe_nss                                          :  3, // [20:18]
                      dot11ax_min_packet_extension                            :  2, // [17:16]
                      dot11ax_su_extended                                     :  1, // [15:15]
                      dot11ax_doppler_indication                              :  1, // [14:14]
                      dot11ax_dcm                                             :  1, // [13:13]
                      dot11ax_cp_ltf_size                                     :  2, // [12:11]
                      dot11ax_spatial_reuse                                   :  4, // [10:7]
                      dot11ax_bss_color_id                                    :  6, // [6:1]
                      aggregation                                             :  1; // [0:0]
             uint32_t u_sig_puncture_pattern_encoding                         :  6, // [31:26]
                      reserved_5a                                             :  6, // [25:20]
                      required_response_time_source                           :  1, // [19:19]
                      num_he_sigb_sym                                         :  5, // [18:14]
                      he_sigb_0_mcs                                           :  3, // [13:11]
                      he_sigb_dcm                                             :  1, // [10:10]
                      eht_duplicate_mode                                      :  2, // [9:8]
                      dot11ax_ext_ru_size                                     :  4, // [7:4]
                      dot11ax_ext_ru_start_index                              :  4; // [3:0]
             uint32_t dot11be_params_placeholder                              :  4, // [31:28]
                      required_response_time                                  : 12; // [27:16]
             struct   mlo_sta_id_details                                        mlo_sta_id_details_rx;
#endif
};




#define PDG_RESPONSE_RATE_SETTING_RESERVED_0A_OFFSET                                0x00000000
#define PDG_RESPONSE_RATE_SETTING_RESERVED_0A_LSB                                   0
#define PDG_RESPONSE_RATE_SETTING_RESERVED_0A_MSB                                   0
#define PDG_RESPONSE_RATE_SETTING_RESERVED_0A_MASK                                  0x00000001




#define PDG_RESPONSE_RATE_SETTING_TX_ANTENNA_SECTOR_CTRL_OFFSET                     0x00000000
#define PDG_RESPONSE_RATE_SETTING_TX_ANTENNA_SECTOR_CTRL_LSB                        1
#define PDG_RESPONSE_RATE_SETTING_TX_ANTENNA_SECTOR_CTRL_MSB                        24
#define PDG_RESPONSE_RATE_SETTING_TX_ANTENNA_SECTOR_CTRL_MASK                       0x01fffffe




#define PDG_RESPONSE_RATE_SETTING_PKT_TYPE_OFFSET                                   0x00000000
#define PDG_RESPONSE_RATE_SETTING_PKT_TYPE_LSB                                      25
#define PDG_RESPONSE_RATE_SETTING_PKT_TYPE_MSB                                      28
#define PDG_RESPONSE_RATE_SETTING_PKT_TYPE_MASK                                     0x1e000000




#define PDG_RESPONSE_RATE_SETTING_SMOOTHING_OFFSET                                  0x00000000
#define PDG_RESPONSE_RATE_SETTING_SMOOTHING_LSB                                     29
#define PDG_RESPONSE_RATE_SETTING_SMOOTHING_MSB                                     29
#define PDG_RESPONSE_RATE_SETTING_SMOOTHING_MASK                                    0x20000000




#define PDG_RESPONSE_RATE_SETTING_LDPC_OFFSET                                       0x00000000
#define PDG_RESPONSE_RATE_SETTING_LDPC_LSB                                          30
#define PDG_RESPONSE_RATE_SETTING_LDPC_MSB                                          30
#define PDG_RESPONSE_RATE_SETTING_LDPC_MASK                                         0x40000000




#define PDG_RESPONSE_RATE_SETTING_STBC_OFFSET                                       0x00000000
#define PDG_RESPONSE_RATE_SETTING_STBC_LSB                                          31
#define PDG_RESPONSE_RATE_SETTING_STBC_MSB                                          31
#define PDG_RESPONSE_RATE_SETTING_STBC_MASK                                         0x80000000




#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_OFFSET                                 0x00000004
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_LSB                                    0
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_MSB                                    7
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_MASK                                   0x000000ff




#define PDG_RESPONSE_RATE_SETTING_ALT_MIN_TX_PWR_OFFSET                             0x00000004
#define PDG_RESPONSE_RATE_SETTING_ALT_MIN_TX_PWR_LSB                                8
#define PDG_RESPONSE_RATE_SETTING_ALT_MIN_TX_PWR_MSB                                15
#define PDG_RESPONSE_RATE_SETTING_ALT_MIN_TX_PWR_MASK                               0x0000ff00




#define PDG_RESPONSE_RATE_SETTING_ALT_NSS_OFFSET                                    0x00000004
#define PDG_RESPONSE_RATE_SETTING_ALT_NSS_LSB                                       16
#define PDG_RESPONSE_RATE_SETTING_ALT_NSS_MSB                                       18
#define PDG_RESPONSE_RATE_SETTING_ALT_NSS_MASK                                      0x00070000




#define PDG_RESPONSE_RATE_SETTING_ALT_TX_CHAIN_MASK_OFFSET                          0x00000004
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_CHAIN_MASK_LSB                             19
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_CHAIN_MASK_MSB                             26
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_CHAIN_MASK_MASK                            0x07f80000




#define PDG_RESPONSE_RATE_SETTING_ALT_BW_OFFSET                                     0x00000004
#define PDG_RESPONSE_RATE_SETTING_ALT_BW_LSB                                        27
#define PDG_RESPONSE_RATE_SETTING_ALT_BW_MSB                                        29
#define PDG_RESPONSE_RATE_SETTING_ALT_BW_MASK                                       0x38000000




#define PDG_RESPONSE_RATE_SETTING_STF_LTF_3DB_BOOST_OFFSET                          0x00000004
#define PDG_RESPONSE_RATE_SETTING_STF_LTF_3DB_BOOST_LSB                             30
#define PDG_RESPONSE_RATE_SETTING_STF_LTF_3DB_BOOST_MSB                             30
#define PDG_RESPONSE_RATE_SETTING_STF_LTF_3DB_BOOST_MASK                            0x40000000




#define PDG_RESPONSE_RATE_SETTING_FORCE_EXTRA_SYMBOL_OFFSET                         0x00000004
#define PDG_RESPONSE_RATE_SETTING_FORCE_EXTRA_SYMBOL_LSB                            31
#define PDG_RESPONSE_RATE_SETTING_FORCE_EXTRA_SYMBOL_MSB                            31
#define PDG_RESPONSE_RATE_SETTING_FORCE_EXTRA_SYMBOL_MASK                           0x80000000




#define PDG_RESPONSE_RATE_SETTING_ALT_RATE_MCS_OFFSET                               0x00000008
#define PDG_RESPONSE_RATE_SETTING_ALT_RATE_MCS_LSB                                  0
#define PDG_RESPONSE_RATE_SETTING_ALT_RATE_MCS_MSB                                  3
#define PDG_RESPONSE_RATE_SETTING_ALT_RATE_MCS_MASK                                 0x0000000f




#define PDG_RESPONSE_RATE_SETTING_NSS_OFFSET                                        0x00000008
#define PDG_RESPONSE_RATE_SETTING_NSS_LSB                                           4
#define PDG_RESPONSE_RATE_SETTING_NSS_MSB                                           6
#define PDG_RESPONSE_RATE_SETTING_NSS_MASK                                          0x00000070




#define PDG_RESPONSE_RATE_SETTING_DPD_ENABLE_OFFSET                                 0x00000008
#define PDG_RESPONSE_RATE_SETTING_DPD_ENABLE_LSB                                    7
#define PDG_RESPONSE_RATE_SETTING_DPD_ENABLE_MSB                                    7
#define PDG_RESPONSE_RATE_SETTING_DPD_ENABLE_MASK                                   0x00000080




#define PDG_RESPONSE_RATE_SETTING_TX_PWR_OFFSET                                     0x00000008
#define PDG_RESPONSE_RATE_SETTING_TX_PWR_LSB                                        8
#define PDG_RESPONSE_RATE_SETTING_TX_PWR_MSB                                        15
#define PDG_RESPONSE_RATE_SETTING_TX_PWR_MASK                                       0x0000ff00




#define PDG_RESPONSE_RATE_SETTING_MIN_TX_PWR_OFFSET                                 0x00000008
#define PDG_RESPONSE_RATE_SETTING_MIN_TX_PWR_LSB                                    16
#define PDG_RESPONSE_RATE_SETTING_MIN_TX_PWR_MSB                                    23
#define PDG_RESPONSE_RATE_SETTING_MIN_TX_PWR_MASK                                   0x00ff0000




#define PDG_RESPONSE_RATE_SETTING_TX_CHAIN_MASK_OFFSET                              0x00000008
#define PDG_RESPONSE_RATE_SETTING_TX_CHAIN_MASK_LSB                                 24
#define PDG_RESPONSE_RATE_SETTING_TX_CHAIN_MASK_MSB                                 31
#define PDG_RESPONSE_RATE_SETTING_TX_CHAIN_MASK_MASK                                0xff000000




#define PDG_RESPONSE_RATE_SETTING_RESERVED_3A_OFFSET                                0x0000000c
#define PDG_RESPONSE_RATE_SETTING_RESERVED_3A_LSB                                   0
#define PDG_RESPONSE_RATE_SETTING_RESERVED_3A_MSB                                   7
#define PDG_RESPONSE_RATE_SETTING_RESERVED_3A_MASK                                  0x000000ff




#define PDG_RESPONSE_RATE_SETTING_SGI_OFFSET                                        0x0000000c
#define PDG_RESPONSE_RATE_SETTING_SGI_LSB                                           8
#define PDG_RESPONSE_RATE_SETTING_SGI_MSB                                           9
#define PDG_RESPONSE_RATE_SETTING_SGI_MASK                                          0x00000300




#define PDG_RESPONSE_RATE_SETTING_RATE_MCS_OFFSET                                   0x0000000c
#define PDG_RESPONSE_RATE_SETTING_RATE_MCS_LSB                                      10
#define PDG_RESPONSE_RATE_SETTING_RATE_MCS_MSB                                      13
#define PDG_RESPONSE_RATE_SETTING_RATE_MCS_MASK                                     0x00003c00




#define PDG_RESPONSE_RATE_SETTING_RESERVED_3B_OFFSET                                0x0000000c
#define PDG_RESPONSE_RATE_SETTING_RESERVED_3B_LSB                                   14
#define PDG_RESPONSE_RATE_SETTING_RESERVED_3B_MSB                                   15
#define PDG_RESPONSE_RATE_SETTING_RESERVED_3B_MASK                                  0x0000c000




#define PDG_RESPONSE_RATE_SETTING_TX_PWR_1_OFFSET                                   0x0000000c
#define PDG_RESPONSE_RATE_SETTING_TX_PWR_1_LSB                                      16
#define PDG_RESPONSE_RATE_SETTING_TX_PWR_1_MSB                                      23
#define PDG_RESPONSE_RATE_SETTING_TX_PWR_1_MASK                                     0x00ff0000




#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_1_OFFSET                               0x0000000c
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_1_LSB                                  24
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_1_MSB                                  31
#define PDG_RESPONSE_RATE_SETTING_ALT_TX_PWR_1_MASK                                 0xff000000




#define PDG_RESPONSE_RATE_SETTING_AGGREGATION_OFFSET                                0x00000010
#define PDG_RESPONSE_RATE_SETTING_AGGREGATION_LSB                                   0
#define PDG_RESPONSE_RATE_SETTING_AGGREGATION_MSB                                   0
#define PDG_RESPONSE_RATE_SETTING_AGGREGATION_MASK                                  0x00000001




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_BSS_COLOR_ID_OFFSET                       0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_BSS_COLOR_ID_LSB                          1
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_BSS_COLOR_ID_MSB                          6
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_BSS_COLOR_ID_MASK                         0x0000007e




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SPATIAL_REUSE_OFFSET                      0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SPATIAL_REUSE_LSB                         7
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SPATIAL_REUSE_MSB                         10
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SPATIAL_REUSE_MASK                        0x00000780




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CP_LTF_SIZE_OFFSET                        0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CP_LTF_SIZE_LSB                           11
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CP_LTF_SIZE_MSB                           12
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CP_LTF_SIZE_MASK                          0x00001800




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DCM_OFFSET                                0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DCM_LSB                                   13
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DCM_MSB                                   13
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DCM_MASK                                  0x00002000




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DOPPLER_INDICATION_OFFSET                 0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DOPPLER_INDICATION_LSB                    14
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DOPPLER_INDICATION_MSB                    14
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DOPPLER_INDICATION_MASK                   0x00004000




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SU_EXTENDED_OFFSET                        0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SU_EXTENDED_LSB                           15
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SU_EXTENDED_MSB                           15
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_SU_EXTENDED_MASK                          0x00008000




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_MIN_PACKET_EXTENSION_OFFSET               0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_MIN_PACKET_EXTENSION_LSB                  16
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_MIN_PACKET_EXTENSION_MSB                  17
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_MIN_PACKET_EXTENSION_MASK                 0x00030000




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_NSS_OFFSET                             0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_NSS_LSB                                18
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_NSS_MSB                                20
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_NSS_MASK                               0x001c0000




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CONTENT_OFFSET                         0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CONTENT_LSB                            21
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CONTENT_MSB                            21
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CONTENT_MASK                           0x00200000




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_LTF_SIZE_OFFSET                        0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_LTF_SIZE_LSB                           22
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_LTF_SIZE_MSB                           23
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_LTF_SIZE_MASK                          0x00c00000




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CHAIN_CSD_EN_OFFSET                       0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CHAIN_CSD_EN_LSB                          24
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CHAIN_CSD_EN_MSB                          24
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_CHAIN_CSD_EN_MASK                         0x01000000




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CHAIN_CSD_EN_OFFSET                    0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CHAIN_CSD_EN_LSB                       25
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CHAIN_CSD_EN_MSB                       25
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_PE_CHAIN_CSD_EN_MASK                      0x02000000




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DL_UL_FLAG_OFFSET                         0x00000010
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DL_UL_FLAG_LSB                            26
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DL_UL_FLAG_MSB                            26
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_DL_UL_FLAG_MASK                           0x04000000




#define PDG_RESPONSE_RATE_SETTING_RESERVED_4A_OFFSET                                0x00000010
#define PDG_RESPONSE_RATE_SETTING_RESERVED_4A_LSB                                   27
#define PDG_RESPONSE_RATE_SETTING_RESERVED_4A_MSB                                   31
#define PDG_RESPONSE_RATE_SETTING_RESERVED_4A_MASK                                  0xf8000000




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_START_INDEX_OFFSET                 0x00000014
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_START_INDEX_LSB                    0
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_START_INDEX_MSB                    3
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_START_INDEX_MASK                   0x0000000f




#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_SIZE_OFFSET                        0x00000014
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_SIZE_LSB                           4
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_SIZE_MSB                           7
#define PDG_RESPONSE_RATE_SETTING_DOT11AX_EXT_RU_SIZE_MASK                          0x000000f0




#define PDG_RESPONSE_RATE_SETTING_EHT_DUPLICATE_MODE_OFFSET                         0x00000014
#define PDG_RESPONSE_RATE_SETTING_EHT_DUPLICATE_MODE_LSB                            8
#define PDG_RESPONSE_RATE_SETTING_EHT_DUPLICATE_MODE_MSB                            9
#define PDG_RESPONSE_RATE_SETTING_EHT_DUPLICATE_MODE_MASK                           0x00000300




#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_DCM_OFFSET                                0x00000014
#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_DCM_LSB                                   10
#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_DCM_MSB                                   10
#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_DCM_MASK                                  0x00000400




#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_0_MCS_OFFSET                              0x00000014
#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_0_MCS_LSB                                 11
#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_0_MCS_MSB                                 13
#define PDG_RESPONSE_RATE_SETTING_HE_SIGB_0_MCS_MASK                                0x00003800




#define PDG_RESPONSE_RATE_SETTING_NUM_HE_SIGB_SYM_OFFSET                            0x00000014
#define PDG_RESPONSE_RATE_SETTING_NUM_HE_SIGB_SYM_LSB                               14
#define PDG_RESPONSE_RATE_SETTING_NUM_HE_SIGB_SYM_MSB                               18
#define PDG_RESPONSE_RATE_SETTING_NUM_HE_SIGB_SYM_MASK                              0x0007c000




#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET              0x00000014
#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_SOURCE_LSB                 19
#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_SOURCE_MSB                 19
#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_SOURCE_MASK                0x00080000




#define PDG_RESPONSE_RATE_SETTING_RESERVED_5A_OFFSET                                0x00000014
#define PDG_RESPONSE_RATE_SETTING_RESERVED_5A_LSB                                   20
#define PDG_RESPONSE_RATE_SETTING_RESERVED_5A_MSB                                   25
#define PDG_RESPONSE_RATE_SETTING_RESERVED_5A_MASK                                  0x03f00000




#define PDG_RESPONSE_RATE_SETTING_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET            0x00000014
#define PDG_RESPONSE_RATE_SETTING_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB               26
#define PDG_RESPONSE_RATE_SETTING_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB               31
#define PDG_RESPONSE_RATE_SETTING_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK              0xfc000000




#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET      0x00000018
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB         0
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB         9
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK        0x000003ff




#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET   0x00000018
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB      10
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB      10
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK     0x00000400




#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x00000018
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB   11
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB   11
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK  0x00000800




#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x00000018
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB   12
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB   12
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK  0x00001000




#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET          0x00000018
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB             13
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB             15
#define PDG_RESPONSE_RATE_SETTING_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK            0x0000e000




#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_OFFSET                     0x00000018
#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_LSB                        16
#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_MSB                        27
#define PDG_RESPONSE_RATE_SETTING_REQUIRED_RESPONSE_TIME_MASK                       0x0fff0000




#define PDG_RESPONSE_RATE_SETTING_DOT11BE_PARAMS_PLACEHOLDER_OFFSET                 0x00000018
#define PDG_RESPONSE_RATE_SETTING_DOT11BE_PARAMS_PLACEHOLDER_LSB                    28
#define PDG_RESPONSE_RATE_SETTING_DOT11BE_PARAMS_PLACEHOLDER_MSB                    31
#define PDG_RESPONSE_RATE_SETTING_DOT11BE_PARAMS_PLACEHOLDER_MASK                   0xf0000000




#endif
