
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

 
 
 
 
 
 
 


#ifndef _TX_CBF_INFO_H_
#define _TX_CBF_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_CBF_INFO 16

#define NUM_OF_QWORDS_TX_CBF_INFO 8


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
                      ranging                                                 :  1,  
                      secure                                                  :  1,  
                      tb_ranging_response_required                            :  2,  
                      reserved_12a                                            :  2,  
                      u_sig_puncture_pattern_encoding                         :  6;  
             uint32_t dot11be_puncture_bitmap                                 : 16,  
                      dot11be_response                                        :  1,  
                      punctured_response                                      :  1,  
                      npda_info_11be_valid                                    :  1,  
                      eht_duplicate_mode                                      :  2,  
                      reserved_13a                                            : 11;  
             uint32_t eht_sta_info_39_32                                      :  8,  
                      reserved_14a                                            : 24;  
             uint32_t tlv64_padding                                           : 32;  
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
                      reserved_12a                                            :  2,  
                      tb_ranging_response_required                            :  2,  
                      secure                                                  :  1,  
                      ranging                                                 :  1,  
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
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 

#define TX_CBF_INFO_SW_PEER_ID_OFFSET                                               0x0000000000000000
#define TX_CBF_INFO_SW_PEER_ID_LSB                                                  0
#define TX_CBF_INFO_SW_PEER_ID_MSB                                                  15
#define TX_CBF_INFO_SW_PEER_ID_MASK                                                 0x000000000000ffff


 

#define TX_CBF_INFO_PRE_CBF_DURATION_OFFSET                                         0x0000000000000000
#define TX_CBF_INFO_PRE_CBF_DURATION_LSB                                            16
#define TX_CBF_INFO_PRE_CBF_DURATION_MSB                                            31
#define TX_CBF_INFO_PRE_CBF_DURATION_MASK                                           0x00000000ffff0000


 

#define TX_CBF_INFO_BRPOLL_INFO_VALID_OFFSET                                        0x0000000000000000
#define TX_CBF_INFO_BRPOLL_INFO_VALID_LSB                                           32
#define TX_CBF_INFO_BRPOLL_INFO_VALID_MSB                                           32
#define TX_CBF_INFO_BRPOLL_INFO_VALID_MASK                                          0x0000000100000000


 

#define TX_CBF_INFO_TRIGGER_BRPOLL_INFO_VALID_OFFSET                                0x0000000000000000
#define TX_CBF_INFO_TRIGGER_BRPOLL_INFO_VALID_LSB                                   33
#define TX_CBF_INFO_TRIGGER_BRPOLL_INFO_VALID_MSB                                   33
#define TX_CBF_INFO_TRIGGER_BRPOLL_INFO_VALID_MASK                                  0x0000000200000000


 

#define TX_CBF_INFO_NPDA_INFO_11AC_VALID_OFFSET                                     0x0000000000000000
#define TX_CBF_INFO_NPDA_INFO_11AC_VALID_LSB                                        34
#define TX_CBF_INFO_NPDA_INFO_11AC_VALID_MSB                                        34
#define TX_CBF_INFO_NPDA_INFO_11AC_VALID_MASK                                       0x0000000400000000


 

#define TX_CBF_INFO_NPDA_INFO_11AX_VALID_OFFSET                                     0x0000000000000000
#define TX_CBF_INFO_NPDA_INFO_11AX_VALID_LSB                                        35
#define TX_CBF_INFO_NPDA_INFO_11AX_VALID_MSB                                        35
#define TX_CBF_INFO_NPDA_INFO_11AX_VALID_MASK                                       0x0000000800000000


 

#define TX_CBF_INFO_DOT11AX_SU_EXTENDED_OFFSET                                      0x0000000000000000
#define TX_CBF_INFO_DOT11AX_SU_EXTENDED_LSB                                         36
#define TX_CBF_INFO_DOT11AX_SU_EXTENDED_MSB                                         36
#define TX_CBF_INFO_DOT11AX_SU_EXTENDED_MASK                                        0x0000001000000000


 

#define TX_CBF_INFO_BANDWIDTH_OFFSET                                                0x0000000000000000
#define TX_CBF_INFO_BANDWIDTH_LSB                                                   37
#define TX_CBF_INFO_BANDWIDTH_MSB                                                   39
#define TX_CBF_INFO_BANDWIDTH_MASK                                                  0x000000e000000000


 

#define TX_CBF_INFO_BRPOLL_INFO_OFFSET                                              0x0000000000000000
#define TX_CBF_INFO_BRPOLL_INFO_LSB                                                 40
#define TX_CBF_INFO_BRPOLL_INFO_MSB                                                 47
#define TX_CBF_INFO_BRPOLL_INFO_MASK                                                0x0000ff0000000000


 

#define TX_CBF_INFO_CBF_RESPONSE_TABLE_BASE_INDEX_OFFSET                            0x0000000000000000
#define TX_CBF_INFO_CBF_RESPONSE_TABLE_BASE_INDEX_LSB                               48
#define TX_CBF_INFO_CBF_RESPONSE_TABLE_BASE_INDEX_MSB                               55
#define TX_CBF_INFO_CBF_RESPONSE_TABLE_BASE_INDEX_MASK                              0x00ff000000000000


 

#define TX_CBF_INFO_PEER_INDEX_OFFSET                                               0x0000000000000000
#define TX_CBF_INFO_PEER_INDEX_LSB                                                  56
#define TX_CBF_INFO_PEER_INDEX_MSB                                                  58
#define TX_CBF_INFO_PEER_INDEX_MASK                                                 0x0700000000000000


 

#define TX_CBF_INFO_PKT_TYPE_OFFSET                                                 0x0000000000000000
#define TX_CBF_INFO_PKT_TYPE_LSB                                                    59
#define TX_CBF_INFO_PKT_TYPE_MSB                                                    62
#define TX_CBF_INFO_PKT_TYPE_MASK                                                   0x7800000000000000


 

#define TX_CBF_INFO_TXOP_DURATION_ALL_ONES_OFFSET                                   0x0000000000000000
#define TX_CBF_INFO_TXOP_DURATION_ALL_ONES_LSB                                      63
#define TX_CBF_INFO_TXOP_DURATION_ALL_ONES_MSB                                      63
#define TX_CBF_INFO_TXOP_DURATION_ALL_ONES_MASK                                     0x8000000000000000


 

#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_15_0_OFFSET                          0x0000000000000008
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_15_0_LSB                             0
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_15_0_MSB                             15
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_15_0_MASK                            0x000000000000ffff


 

#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_31_16_OFFSET                         0x0000000000000008
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_31_16_LSB                            16
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_31_16_MSB                            31
#define TX_CBF_INFO_TRIGGER_BRPOLL_COMMON_INFO_31_16_MASK                           0x00000000ffff0000


 

#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_15_0_OFFSET                            0x0000000000000008
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_15_0_LSB                               32
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_15_0_MSB                               47
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_15_0_MASK                              0x0000ffff00000000


 

#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_31_16_OFFSET                           0x0000000000000008
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_31_16_LSB                              48
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_31_16_MSB                              63
#define TX_CBF_INFO_TRIGGER_BRPOLL_USER_INFO_31_16_MASK                             0xffff000000000000


 

#define TX_CBF_INFO_ADDR1_31_0_OFFSET                                               0x0000000000000010
#define TX_CBF_INFO_ADDR1_31_0_LSB                                                  0
#define TX_CBF_INFO_ADDR1_31_0_MSB                                                  31
#define TX_CBF_INFO_ADDR1_31_0_MASK                                                 0x00000000ffffffff


 

#define TX_CBF_INFO_ADDR1_47_32_OFFSET                                              0x0000000000000010
#define TX_CBF_INFO_ADDR1_47_32_LSB                                                 32
#define TX_CBF_INFO_ADDR1_47_32_MSB                                                 47
#define TX_CBF_INFO_ADDR1_47_32_MASK                                                0x0000ffff00000000


 

#define TX_CBF_INFO_ADDR2_15_0_OFFSET                                               0x0000000000000010
#define TX_CBF_INFO_ADDR2_15_0_LSB                                                  48
#define TX_CBF_INFO_ADDR2_15_0_MSB                                                  63
#define TX_CBF_INFO_ADDR2_15_0_MASK                                                 0xffff000000000000


 

#define TX_CBF_INFO_ADDR2_47_16_OFFSET                                              0x0000000000000018
#define TX_CBF_INFO_ADDR2_47_16_LSB                                                 0
#define TX_CBF_INFO_ADDR2_47_16_MSB                                                 31
#define TX_CBF_INFO_ADDR2_47_16_MASK                                                0x00000000ffffffff


 

#define TX_CBF_INFO_ADDR3_31_0_OFFSET                                               0x0000000000000018
#define TX_CBF_INFO_ADDR3_31_0_LSB                                                  32
#define TX_CBF_INFO_ADDR3_31_0_MSB                                                  63
#define TX_CBF_INFO_ADDR3_31_0_MASK                                                 0xffffffff00000000


 

#define TX_CBF_INFO_ADDR3_47_32_OFFSET                                              0x0000000000000020
#define TX_CBF_INFO_ADDR3_47_32_LSB                                                 0
#define TX_CBF_INFO_ADDR3_47_32_MSB                                                 15
#define TX_CBF_INFO_ADDR3_47_32_MASK                                                0x000000000000ffff


 

#define TX_CBF_INFO_STA_PARTIAL_AID_OFFSET                                          0x0000000000000020
#define TX_CBF_INFO_STA_PARTIAL_AID_LSB                                             16
#define TX_CBF_INFO_STA_PARTIAL_AID_MSB                                             26
#define TX_CBF_INFO_STA_PARTIAL_AID_MASK                                            0x0000000007ff0000


 

#define TX_CBF_INFO_RESERVED_8A_OFFSET                                              0x0000000000000020
#define TX_CBF_INFO_RESERVED_8A_LSB                                                 27
#define TX_CBF_INFO_RESERVED_8A_MSB                                                 30
#define TX_CBF_INFO_RESERVED_8A_MASK                                                0x0000000078000000


 

#define TX_CBF_INFO_CBF_RESP_PWR_MGMT_OFFSET                                        0x0000000000000020
#define TX_CBF_INFO_CBF_RESP_PWR_MGMT_LSB                                           31
#define TX_CBF_INFO_CBF_RESP_PWR_MGMT_MSB                                           31
#define TX_CBF_INFO_CBF_RESP_PWR_MGMT_MASK                                          0x0000000080000000


 

#define TX_CBF_INFO_GROUP_ID_OFFSET                                                 0x0000000000000020
#define TX_CBF_INFO_GROUP_ID_LSB                                                    32
#define TX_CBF_INFO_GROUP_ID_MSB                                                    37
#define TX_CBF_INFO_GROUP_ID_MASK                                                   0x0000003f00000000


 

#define TX_CBF_INFO_RSSI_COMB_OFFSET                                                0x0000000000000020
#define TX_CBF_INFO_RSSI_COMB_LSB                                                   38
#define TX_CBF_INFO_RSSI_COMB_MSB                                                   45
#define TX_CBF_INFO_RSSI_COMB_MASK                                                  0x00003fc000000000


 

#define TX_CBF_INFO_RESERVED_9A_OFFSET                                              0x0000000000000020
#define TX_CBF_INFO_RESERVED_9A_LSB                                                 46
#define TX_CBF_INFO_RESERVED_9A_MSB                                                 47
#define TX_CBF_INFO_RESERVED_9A_MASK                                                0x0000c00000000000


 

#define TX_CBF_INFO_VHT_NDPA_STA_INFO_OFFSET                                        0x0000000000000020
#define TX_CBF_INFO_VHT_NDPA_STA_INFO_LSB                                           48
#define TX_CBF_INFO_VHT_NDPA_STA_INFO_MSB                                           63
#define TX_CBF_INFO_VHT_NDPA_STA_INFO_MASK                                          0xffff000000000000


 

#define TX_CBF_INFO_HE_EHT_STA_INFO_15_0_OFFSET                                     0x0000000000000028
#define TX_CBF_INFO_HE_EHT_STA_INFO_15_0_LSB                                        0
#define TX_CBF_INFO_HE_EHT_STA_INFO_15_0_MSB                                        15
#define TX_CBF_INFO_HE_EHT_STA_INFO_15_0_MASK                                       0x000000000000ffff


 

#define TX_CBF_INFO_HE_EHT_STA_INFO_31_16_OFFSET                                    0x0000000000000028
#define TX_CBF_INFO_HE_EHT_STA_INFO_31_16_LSB                                       16
#define TX_CBF_INFO_HE_EHT_STA_INFO_31_16_MSB                                       31
#define TX_CBF_INFO_HE_EHT_STA_INFO_31_16_MASK                                      0x00000000ffff0000


 

#define TX_CBF_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_OFFSET                       0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_LSB                          32
#define TX_CBF_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_MSB                          32
#define TX_CBF_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_MASK                         0x0000000100000000


 

#define TX_CBF_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_OFFSET                              0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_LSB                                 33
#define TX_CBF_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_MSB                                 33
#define TX_CBF_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_MASK                                0x0000000200000000


 

#define TX_CBF_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_OFFSET                            0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_LSB                               34
#define TX_CBF_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_MSB                               39
#define TX_CBF_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_MASK                              0x000000fc00000000


 

#define TX_CBF_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_OFFSET                           0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_LSB                              40
#define TX_CBF_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_MSB                              43
#define TX_CBF_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_MASK                             0x00000f0000000000


 

#define TX_CBF_INFO_DOT11AX_RECEIVED_CP_SIZE_OFFSET                                 0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_CP_SIZE_LSB                                    44
#define TX_CBF_INFO_DOT11AX_RECEIVED_CP_SIZE_MSB                                    45
#define TX_CBF_INFO_DOT11AX_RECEIVED_CP_SIZE_MASK                                   0x0000300000000000


 

#define TX_CBF_INFO_DOT11AX_RECEIVED_LTF_SIZE_OFFSET                                0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_LTF_SIZE_LSB                                   46
#define TX_CBF_INFO_DOT11AX_RECEIVED_LTF_SIZE_MSB                                   47
#define TX_CBF_INFO_DOT11AX_RECEIVED_LTF_SIZE_MASK                                  0x0000c00000000000


 

#define TX_CBF_INFO_DOT11AX_RECEIVED_CODING_OFFSET                                  0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_CODING_LSB                                     48
#define TX_CBF_INFO_DOT11AX_RECEIVED_CODING_MSB                                     48
#define TX_CBF_INFO_DOT11AX_RECEIVED_CODING_MASK                                    0x0001000000000000


 

#define TX_CBF_INFO_DOT11AX_RECEIVED_DCM_OFFSET                                     0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_DCM_LSB                                        49
#define TX_CBF_INFO_DOT11AX_RECEIVED_DCM_MSB                                        49
#define TX_CBF_INFO_DOT11AX_RECEIVED_DCM_MASK                                       0x0002000000000000


 

#define TX_CBF_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_OFFSET                      0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_LSB                         50
#define TX_CBF_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_MSB                         50
#define TX_CBF_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_MASK                        0x0004000000000000


 

#define TX_CBF_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_OFFSET                             0x0000000000000028
#define TX_CBF_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_LSB                                51
#define TX_CBF_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_MSB                                54
#define TX_CBF_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_MASK                               0x0078000000000000


 

#define TX_CBF_INFO_DOT11AX_DL_UL_FLAG_OFFSET                                       0x0000000000000028
#define TX_CBF_INFO_DOT11AX_DL_UL_FLAG_LSB                                          55
#define TX_CBF_INFO_DOT11AX_DL_UL_FLAG_MSB                                          55
#define TX_CBF_INFO_DOT11AX_DL_UL_FLAG_MASK                                         0x0080000000000000


 

#define TX_CBF_INFO_RESERVED_11A_OFFSET                                             0x0000000000000028
#define TX_CBF_INFO_RESERVED_11A_LSB                                                56
#define TX_CBF_INFO_RESERVED_11A_MSB                                                63
#define TX_CBF_INFO_RESERVED_11A_MASK                                               0xff00000000000000


 

#define TX_CBF_INFO_SW_RESPONSE_FRAME_LENGTH_OFFSET                                 0x0000000000000030
#define TX_CBF_INFO_SW_RESPONSE_FRAME_LENGTH_LSB                                    0
#define TX_CBF_INFO_SW_RESPONSE_FRAME_LENGTH_MSB                                    15
#define TX_CBF_INFO_SW_RESPONSE_FRAME_LENGTH_MASK                                   0x000000000000ffff


 

#define TX_CBF_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_OFFSET                              0x0000000000000030
#define TX_CBF_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_LSB                                 16
#define TX_CBF_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_MSB                                 16
#define TX_CBF_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_MASK                                0x0000000000010000


 

#define TX_CBF_INFO_WAIT_SIFS_CONFIG_VALID_OFFSET                                   0x0000000000000030
#define TX_CBF_INFO_WAIT_SIFS_CONFIG_VALID_LSB                                      17
#define TX_CBF_INFO_WAIT_SIFS_CONFIG_VALID_MSB                                      17
#define TX_CBF_INFO_WAIT_SIFS_CONFIG_VALID_MASK                                     0x0000000000020000


 

#define TX_CBF_INFO_WAIT_SIFS_OFFSET                                                0x0000000000000030
#define TX_CBF_INFO_WAIT_SIFS_LSB                                                   18
#define TX_CBF_INFO_WAIT_SIFS_MSB                                                   19
#define TX_CBF_INFO_WAIT_SIFS_MASK                                                  0x00000000000c0000


 

#define TX_CBF_INFO_RANGING_OFFSET                                                  0x0000000000000030
#define TX_CBF_INFO_RANGING_LSB                                                     20
#define TX_CBF_INFO_RANGING_MSB                                                     20
#define TX_CBF_INFO_RANGING_MASK                                                    0x0000000000100000


 

#define TX_CBF_INFO_SECURE_OFFSET                                                   0x0000000000000030
#define TX_CBF_INFO_SECURE_LSB                                                      21
#define TX_CBF_INFO_SECURE_MSB                                                      21
#define TX_CBF_INFO_SECURE_MASK                                                     0x0000000000200000


 

#define TX_CBF_INFO_TB_RANGING_RESPONSE_REQUIRED_OFFSET                             0x0000000000000030
#define TX_CBF_INFO_TB_RANGING_RESPONSE_REQUIRED_LSB                                22
#define TX_CBF_INFO_TB_RANGING_RESPONSE_REQUIRED_MSB                                23
#define TX_CBF_INFO_TB_RANGING_RESPONSE_REQUIRED_MASK                               0x0000000000c00000


 

#define TX_CBF_INFO_RESERVED_12A_OFFSET                                             0x0000000000000030
#define TX_CBF_INFO_RESERVED_12A_LSB                                                24
#define TX_CBF_INFO_RESERVED_12A_MSB                                                25
#define TX_CBF_INFO_RESERVED_12A_MASK                                               0x0000000003000000


 

#define TX_CBF_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET                          0x0000000000000030
#define TX_CBF_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB                             26
#define TX_CBF_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB                             31
#define TX_CBF_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK                            0x00000000fc000000


 

#define TX_CBF_INFO_DOT11BE_PUNCTURE_BITMAP_OFFSET                                  0x0000000000000030
#define TX_CBF_INFO_DOT11BE_PUNCTURE_BITMAP_LSB                                     32
#define TX_CBF_INFO_DOT11BE_PUNCTURE_BITMAP_MSB                                     47
#define TX_CBF_INFO_DOT11BE_PUNCTURE_BITMAP_MASK                                    0x0000ffff00000000


 

#define TX_CBF_INFO_DOT11BE_RESPONSE_OFFSET                                         0x0000000000000030
#define TX_CBF_INFO_DOT11BE_RESPONSE_LSB                                            48
#define TX_CBF_INFO_DOT11BE_RESPONSE_MSB                                            48
#define TX_CBF_INFO_DOT11BE_RESPONSE_MASK                                           0x0001000000000000


 

#define TX_CBF_INFO_PUNCTURED_RESPONSE_OFFSET                                       0x0000000000000030
#define TX_CBF_INFO_PUNCTURED_RESPONSE_LSB                                          49
#define TX_CBF_INFO_PUNCTURED_RESPONSE_MSB                                          49
#define TX_CBF_INFO_PUNCTURED_RESPONSE_MASK                                         0x0002000000000000


 

#define TX_CBF_INFO_NPDA_INFO_11BE_VALID_OFFSET                                     0x0000000000000030
#define TX_CBF_INFO_NPDA_INFO_11BE_VALID_LSB                                        50
#define TX_CBF_INFO_NPDA_INFO_11BE_VALID_MSB                                        50
#define TX_CBF_INFO_NPDA_INFO_11BE_VALID_MASK                                       0x0004000000000000


 

#define TX_CBF_INFO_EHT_DUPLICATE_MODE_OFFSET                                       0x0000000000000030
#define TX_CBF_INFO_EHT_DUPLICATE_MODE_LSB                                          51
#define TX_CBF_INFO_EHT_DUPLICATE_MODE_MSB                                          52
#define TX_CBF_INFO_EHT_DUPLICATE_MODE_MASK                                         0x0018000000000000


 

#define TX_CBF_INFO_RESERVED_13A_OFFSET                                             0x0000000000000030
#define TX_CBF_INFO_RESERVED_13A_LSB                                                53
#define TX_CBF_INFO_RESERVED_13A_MSB                                                63
#define TX_CBF_INFO_RESERVED_13A_MASK                                               0xffe0000000000000


 

#define TX_CBF_INFO_EHT_STA_INFO_39_32_OFFSET                                       0x0000000000000038
#define TX_CBF_INFO_EHT_STA_INFO_39_32_LSB                                          0
#define TX_CBF_INFO_EHT_STA_INFO_39_32_MSB                                          7
#define TX_CBF_INFO_EHT_STA_INFO_39_32_MASK                                         0x00000000000000ff


 

#define TX_CBF_INFO_RESERVED_14A_OFFSET                                             0x0000000000000038
#define TX_CBF_INFO_RESERVED_14A_LSB                                                8
#define TX_CBF_INFO_RESERVED_14A_MSB                                                31
#define TX_CBF_INFO_RESERVED_14A_MASK                                               0x00000000ffffff00


 

#define TX_CBF_INFO_TLV64_PADDING_OFFSET                                            0x0000000000000038
#define TX_CBF_INFO_TLV64_PADDING_LSB                                               32
#define TX_CBF_INFO_TLV64_PADDING_MSB                                               63
#define TX_CBF_INFO_TLV64_PADDING_MASK                                              0xffffffff00000000



#endif    
