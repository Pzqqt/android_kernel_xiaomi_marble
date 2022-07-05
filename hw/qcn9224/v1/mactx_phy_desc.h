
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

 
 
 
 
 
 
 


#ifndef _MACTX_PHY_DESC_H_
#define _MACTX_PHY_DESC_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_MACTX_PHY_DESC 4

#define NUM_OF_QWORDS_MACTX_PHY_DESC 2


struct mactx_phy_desc {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t reserved_0a                                             : 16,  
                      bf_type                                                 :  2,  
                      wait_sifs                                               :  2,  
                      dot11b_preamble_type                                    :  1,  
                      pkt_type                                                :  4,  
                      su_or_mu                                                :  2,  
                      mu_type                                                 :  1,  
                      bandwidth                                               :  3,  
                      channel_capture                                         :  1;  
             uint32_t mcs                                                     :  4,  
                      global_ofdma_mimo_enable                                :  1,  
                      reserved_1a                                             :  1,  
                      stbc                                                    :  1,  
                      dot11ax_su_extended                                     :  1,  
                      dot11ax_trigger_frame_embedded                          :  1,  
                      tx_pwr_shared                                           :  8,  
                      tx_pwr_unshared                                         :  8,  
                      measure_power                                           :  1,  
                      tpc_glut_self_cal                                       :  1,  
                      back_to_back_transmission_expected                      :  1,  
                      heavy_clip_nss                                          :  3,  
                      txbf_per_packet_no_csd_no_walsh                         :  1;  
             uint32_t ndp                                                     :  2,  
                      ul_flag                                                 :  1,  
                      triggered                                               :  1,  
                      ap_pkt_bw                                               :  3,  
                      ru_position_start                                       :  8,  
                      pcu_ppdu_setup_start_reason                             :  3,  
                      tlv_source                                              :  1,  
                      reserved_2a                                             :  2,  
                      nss                                                     :  3,  
                      stream_offset                                           :  3,  
                      reserved_2b                                             :  2,  
                      clpc_enable                                             :  1,  
                      mu_ndp                                                  :  1,  
                      response_expected                                       :  1;  
             uint32_t rx_chain_mask                                           :  8,  
                      rx_chain_mask_valid                                     :  1,  
                      ant_sel_valid                                           :  1,  
                      ant_sel                                                 :  1,  
                      cp_setting                                              :  2,  
                      he_ppdu_subtype                                         :  2,  
                      active_channel                                          :  3,  
                      generate_phyrx_tx_start_timing                          :  1,  
                      ltf_size                                                :  2,  
                      ru_size_updated_v2                                      :  4,  
                      reserved_3c                                             :  1,  
                      u_sig_puncture_pattern_encoding                         :  6;  
#else
             uint32_t channel_capture                                         :  1,  
                      bandwidth                                               :  3,  
                      mu_type                                                 :  1,  
                      su_or_mu                                                :  2,  
                      pkt_type                                                :  4,  
                      dot11b_preamble_type                                    :  1,  
                      wait_sifs                                               :  2,  
                      bf_type                                                 :  2,  
                      reserved_0a                                             : 16;  
             uint32_t txbf_per_packet_no_csd_no_walsh                         :  1,  
                      heavy_clip_nss                                          :  3,  
                      back_to_back_transmission_expected                      :  1,  
                      tpc_glut_self_cal                                       :  1,  
                      measure_power                                           :  1,  
                      tx_pwr_unshared                                         :  8,  
                      tx_pwr_shared                                           :  8,  
                      dot11ax_trigger_frame_embedded                          :  1,  
                      dot11ax_su_extended                                     :  1,  
                      stbc                                                    :  1,  
                      reserved_1a                                             :  1,  
                      global_ofdma_mimo_enable                                :  1,  
                      mcs                                                     :  4;  
             uint32_t response_expected                                       :  1,  
                      mu_ndp                                                  :  1,  
                      clpc_enable                                             :  1,  
                      reserved_2b                                             :  2,  
                      stream_offset                                           :  3,  
                      nss                                                     :  3,  
                      reserved_2a                                             :  2,  
                      tlv_source                                              :  1,  
                      pcu_ppdu_setup_start_reason                             :  3,  
                      ru_position_start                                       :  8,  
                      ap_pkt_bw                                               :  3,  
                      triggered                                               :  1,  
                      ul_flag                                                 :  1,  
                      ndp                                                     :  2;  
             uint32_t u_sig_puncture_pattern_encoding                         :  6,  
                      reserved_3c                                             :  1,  
                      ru_size_updated_v2                                      :  4,  
                      ltf_size                                                :  2,  
                      generate_phyrx_tx_start_timing                          :  1,  
                      active_channel                                          :  3,  
                      he_ppdu_subtype                                         :  2,  
                      cp_setting                                              :  2,  
                      ant_sel                                                 :  1,  
                      ant_sel_valid                                           :  1,  
                      rx_chain_mask_valid                                     :  1,  
                      rx_chain_mask                                           :  8;  
#endif
};


 

#define MACTX_PHY_DESC_RESERVED_0A_OFFSET                                           0x0000000000000000
#define MACTX_PHY_DESC_RESERVED_0A_LSB                                              0
#define MACTX_PHY_DESC_RESERVED_0A_MSB                                              15
#define MACTX_PHY_DESC_RESERVED_0A_MASK                                             0x000000000000ffff


 

#define MACTX_PHY_DESC_BF_TYPE_OFFSET                                               0x0000000000000000
#define MACTX_PHY_DESC_BF_TYPE_LSB                                                  16
#define MACTX_PHY_DESC_BF_TYPE_MSB                                                  17
#define MACTX_PHY_DESC_BF_TYPE_MASK                                                 0x0000000000030000


 

#define MACTX_PHY_DESC_WAIT_SIFS_OFFSET                                             0x0000000000000000
#define MACTX_PHY_DESC_WAIT_SIFS_LSB                                                18
#define MACTX_PHY_DESC_WAIT_SIFS_MSB                                                19
#define MACTX_PHY_DESC_WAIT_SIFS_MASK                                               0x00000000000c0000


 

#define MACTX_PHY_DESC_DOT11B_PREAMBLE_TYPE_OFFSET                                  0x0000000000000000
#define MACTX_PHY_DESC_DOT11B_PREAMBLE_TYPE_LSB                                     20
#define MACTX_PHY_DESC_DOT11B_PREAMBLE_TYPE_MSB                                     20
#define MACTX_PHY_DESC_DOT11B_PREAMBLE_TYPE_MASK                                    0x0000000000100000


 

#define MACTX_PHY_DESC_PKT_TYPE_OFFSET                                              0x0000000000000000
#define MACTX_PHY_DESC_PKT_TYPE_LSB                                                 21
#define MACTX_PHY_DESC_PKT_TYPE_MSB                                                 24
#define MACTX_PHY_DESC_PKT_TYPE_MASK                                                0x0000000001e00000


 

#define MACTX_PHY_DESC_SU_OR_MU_OFFSET                                              0x0000000000000000
#define MACTX_PHY_DESC_SU_OR_MU_LSB                                                 25
#define MACTX_PHY_DESC_SU_OR_MU_MSB                                                 26
#define MACTX_PHY_DESC_SU_OR_MU_MASK                                                0x0000000006000000


 

#define MACTX_PHY_DESC_MU_TYPE_OFFSET                                               0x0000000000000000
#define MACTX_PHY_DESC_MU_TYPE_LSB                                                  27
#define MACTX_PHY_DESC_MU_TYPE_MSB                                                  27
#define MACTX_PHY_DESC_MU_TYPE_MASK                                                 0x0000000008000000


 

#define MACTX_PHY_DESC_BANDWIDTH_OFFSET                                             0x0000000000000000
#define MACTX_PHY_DESC_BANDWIDTH_LSB                                                28
#define MACTX_PHY_DESC_BANDWIDTH_MSB                                                30
#define MACTX_PHY_DESC_BANDWIDTH_MASK                                               0x0000000070000000


 

#define MACTX_PHY_DESC_CHANNEL_CAPTURE_OFFSET                                       0x0000000000000000
#define MACTX_PHY_DESC_CHANNEL_CAPTURE_LSB                                          31
#define MACTX_PHY_DESC_CHANNEL_CAPTURE_MSB                                          31
#define MACTX_PHY_DESC_CHANNEL_CAPTURE_MASK                                         0x0000000080000000


 

#define MACTX_PHY_DESC_MCS_OFFSET                                                   0x0000000000000000
#define MACTX_PHY_DESC_MCS_LSB                                                      32
#define MACTX_PHY_DESC_MCS_MSB                                                      35
#define MACTX_PHY_DESC_MCS_MASK                                                     0x0000000f00000000


 

#define MACTX_PHY_DESC_GLOBAL_OFDMA_MIMO_ENABLE_OFFSET                              0x0000000000000000
#define MACTX_PHY_DESC_GLOBAL_OFDMA_MIMO_ENABLE_LSB                                 36
#define MACTX_PHY_DESC_GLOBAL_OFDMA_MIMO_ENABLE_MSB                                 36
#define MACTX_PHY_DESC_GLOBAL_OFDMA_MIMO_ENABLE_MASK                                0x0000001000000000


 

#define MACTX_PHY_DESC_RESERVED_1A_OFFSET                                           0x0000000000000000
#define MACTX_PHY_DESC_RESERVED_1A_LSB                                              37
#define MACTX_PHY_DESC_RESERVED_1A_MSB                                              37
#define MACTX_PHY_DESC_RESERVED_1A_MASK                                             0x0000002000000000


 

#define MACTX_PHY_DESC_STBC_OFFSET                                                  0x0000000000000000
#define MACTX_PHY_DESC_STBC_LSB                                                     38
#define MACTX_PHY_DESC_STBC_MSB                                                     38
#define MACTX_PHY_DESC_STBC_MASK                                                    0x0000004000000000


 

#define MACTX_PHY_DESC_DOT11AX_SU_EXTENDED_OFFSET                                   0x0000000000000000
#define MACTX_PHY_DESC_DOT11AX_SU_EXTENDED_LSB                                      39
#define MACTX_PHY_DESC_DOT11AX_SU_EXTENDED_MSB                                      39
#define MACTX_PHY_DESC_DOT11AX_SU_EXTENDED_MASK                                     0x0000008000000000


 

#define MACTX_PHY_DESC_DOT11AX_TRIGGER_FRAME_EMBEDDED_OFFSET                        0x0000000000000000
#define MACTX_PHY_DESC_DOT11AX_TRIGGER_FRAME_EMBEDDED_LSB                           40
#define MACTX_PHY_DESC_DOT11AX_TRIGGER_FRAME_EMBEDDED_MSB                           40
#define MACTX_PHY_DESC_DOT11AX_TRIGGER_FRAME_EMBEDDED_MASK                          0x0000010000000000


 

#define MACTX_PHY_DESC_TX_PWR_SHARED_OFFSET                                         0x0000000000000000
#define MACTX_PHY_DESC_TX_PWR_SHARED_LSB                                            41
#define MACTX_PHY_DESC_TX_PWR_SHARED_MSB                                            48
#define MACTX_PHY_DESC_TX_PWR_SHARED_MASK                                           0x0001fe0000000000


 

#define MACTX_PHY_DESC_TX_PWR_UNSHARED_OFFSET                                       0x0000000000000000
#define MACTX_PHY_DESC_TX_PWR_UNSHARED_LSB                                          49
#define MACTX_PHY_DESC_TX_PWR_UNSHARED_MSB                                          56
#define MACTX_PHY_DESC_TX_PWR_UNSHARED_MASK                                         0x01fe000000000000


 

#define MACTX_PHY_DESC_MEASURE_POWER_OFFSET                                         0x0000000000000000
#define MACTX_PHY_DESC_MEASURE_POWER_LSB                                            57
#define MACTX_PHY_DESC_MEASURE_POWER_MSB                                            57
#define MACTX_PHY_DESC_MEASURE_POWER_MASK                                           0x0200000000000000


 

#define MACTX_PHY_DESC_TPC_GLUT_SELF_CAL_OFFSET                                     0x0000000000000000
#define MACTX_PHY_DESC_TPC_GLUT_SELF_CAL_LSB                                        58
#define MACTX_PHY_DESC_TPC_GLUT_SELF_CAL_MSB                                        58
#define MACTX_PHY_DESC_TPC_GLUT_SELF_CAL_MASK                                       0x0400000000000000


 

#define MACTX_PHY_DESC_BACK_TO_BACK_TRANSMISSION_EXPECTED_OFFSET                    0x0000000000000000
#define MACTX_PHY_DESC_BACK_TO_BACK_TRANSMISSION_EXPECTED_LSB                       59
#define MACTX_PHY_DESC_BACK_TO_BACK_TRANSMISSION_EXPECTED_MSB                       59
#define MACTX_PHY_DESC_BACK_TO_BACK_TRANSMISSION_EXPECTED_MASK                      0x0800000000000000


 

#define MACTX_PHY_DESC_HEAVY_CLIP_NSS_OFFSET                                        0x0000000000000000
#define MACTX_PHY_DESC_HEAVY_CLIP_NSS_LSB                                           60
#define MACTX_PHY_DESC_HEAVY_CLIP_NSS_MSB                                           62
#define MACTX_PHY_DESC_HEAVY_CLIP_NSS_MASK                                          0x7000000000000000


 

#define MACTX_PHY_DESC_TXBF_PER_PACKET_NO_CSD_NO_WALSH_OFFSET                       0x0000000000000000
#define MACTX_PHY_DESC_TXBF_PER_PACKET_NO_CSD_NO_WALSH_LSB                          63
#define MACTX_PHY_DESC_TXBF_PER_PACKET_NO_CSD_NO_WALSH_MSB                          63
#define MACTX_PHY_DESC_TXBF_PER_PACKET_NO_CSD_NO_WALSH_MASK                         0x8000000000000000


 

#define MACTX_PHY_DESC_NDP_OFFSET                                                   0x0000000000000008
#define MACTX_PHY_DESC_NDP_LSB                                                      0
#define MACTX_PHY_DESC_NDP_MSB                                                      1
#define MACTX_PHY_DESC_NDP_MASK                                                     0x0000000000000003


 

#define MACTX_PHY_DESC_UL_FLAG_OFFSET                                               0x0000000000000008
#define MACTX_PHY_DESC_UL_FLAG_LSB                                                  2
#define MACTX_PHY_DESC_UL_FLAG_MSB                                                  2
#define MACTX_PHY_DESC_UL_FLAG_MASK                                                 0x0000000000000004


 

#define MACTX_PHY_DESC_TRIGGERED_OFFSET                                             0x0000000000000008
#define MACTX_PHY_DESC_TRIGGERED_LSB                                                3
#define MACTX_PHY_DESC_TRIGGERED_MSB                                                3
#define MACTX_PHY_DESC_TRIGGERED_MASK                                               0x0000000000000008


 

#define MACTX_PHY_DESC_AP_PKT_BW_OFFSET                                             0x0000000000000008
#define MACTX_PHY_DESC_AP_PKT_BW_LSB                                                4
#define MACTX_PHY_DESC_AP_PKT_BW_MSB                                                6
#define MACTX_PHY_DESC_AP_PKT_BW_MASK                                               0x0000000000000070


 

#define MACTX_PHY_DESC_RU_POSITION_START_OFFSET                                     0x0000000000000008
#define MACTX_PHY_DESC_RU_POSITION_START_LSB                                        7
#define MACTX_PHY_DESC_RU_POSITION_START_MSB                                        14
#define MACTX_PHY_DESC_RU_POSITION_START_MASK                                       0x0000000000007f80


 

#define MACTX_PHY_DESC_PCU_PPDU_SETUP_START_REASON_OFFSET                           0x0000000000000008
#define MACTX_PHY_DESC_PCU_PPDU_SETUP_START_REASON_LSB                              15
#define MACTX_PHY_DESC_PCU_PPDU_SETUP_START_REASON_MSB                              17
#define MACTX_PHY_DESC_PCU_PPDU_SETUP_START_REASON_MASK                             0x0000000000038000


 

#define MACTX_PHY_DESC_TLV_SOURCE_OFFSET                                            0x0000000000000008
#define MACTX_PHY_DESC_TLV_SOURCE_LSB                                               18
#define MACTX_PHY_DESC_TLV_SOURCE_MSB                                               18
#define MACTX_PHY_DESC_TLV_SOURCE_MASK                                              0x0000000000040000


 

#define MACTX_PHY_DESC_RESERVED_2A_OFFSET                                           0x0000000000000008
#define MACTX_PHY_DESC_RESERVED_2A_LSB                                              19
#define MACTX_PHY_DESC_RESERVED_2A_MSB                                              20
#define MACTX_PHY_DESC_RESERVED_2A_MASK                                             0x0000000000180000


 

#define MACTX_PHY_DESC_NSS_OFFSET                                                   0x0000000000000008
#define MACTX_PHY_DESC_NSS_LSB                                                      21
#define MACTX_PHY_DESC_NSS_MSB                                                      23
#define MACTX_PHY_DESC_NSS_MASK                                                     0x0000000000e00000


 

#define MACTX_PHY_DESC_STREAM_OFFSET_OFFSET                                         0x0000000000000008
#define MACTX_PHY_DESC_STREAM_OFFSET_LSB                                            24
#define MACTX_PHY_DESC_STREAM_OFFSET_MSB                                            26
#define MACTX_PHY_DESC_STREAM_OFFSET_MASK                                           0x0000000007000000


 

#define MACTX_PHY_DESC_RESERVED_2B_OFFSET                                           0x0000000000000008
#define MACTX_PHY_DESC_RESERVED_2B_LSB                                              27
#define MACTX_PHY_DESC_RESERVED_2B_MSB                                              28
#define MACTX_PHY_DESC_RESERVED_2B_MASK                                             0x0000000018000000


 

#define MACTX_PHY_DESC_CLPC_ENABLE_OFFSET                                           0x0000000000000008
#define MACTX_PHY_DESC_CLPC_ENABLE_LSB                                              29
#define MACTX_PHY_DESC_CLPC_ENABLE_MSB                                              29
#define MACTX_PHY_DESC_CLPC_ENABLE_MASK                                             0x0000000020000000


 

#define MACTX_PHY_DESC_MU_NDP_OFFSET                                                0x0000000000000008
#define MACTX_PHY_DESC_MU_NDP_LSB                                                   30
#define MACTX_PHY_DESC_MU_NDP_MSB                                                   30
#define MACTX_PHY_DESC_MU_NDP_MASK                                                  0x0000000040000000


 

#define MACTX_PHY_DESC_RESPONSE_EXPECTED_OFFSET                                     0x0000000000000008
#define MACTX_PHY_DESC_RESPONSE_EXPECTED_LSB                                        31
#define MACTX_PHY_DESC_RESPONSE_EXPECTED_MSB                                        31
#define MACTX_PHY_DESC_RESPONSE_EXPECTED_MASK                                       0x0000000080000000


 

#define MACTX_PHY_DESC_RX_CHAIN_MASK_OFFSET                                         0x0000000000000008
#define MACTX_PHY_DESC_RX_CHAIN_MASK_LSB                                            32
#define MACTX_PHY_DESC_RX_CHAIN_MASK_MSB                                            39
#define MACTX_PHY_DESC_RX_CHAIN_MASK_MASK                                           0x000000ff00000000


 

#define MACTX_PHY_DESC_RX_CHAIN_MASK_VALID_OFFSET                                   0x0000000000000008
#define MACTX_PHY_DESC_RX_CHAIN_MASK_VALID_LSB                                      40
#define MACTX_PHY_DESC_RX_CHAIN_MASK_VALID_MSB                                      40
#define MACTX_PHY_DESC_RX_CHAIN_MASK_VALID_MASK                                     0x0000010000000000


 

#define MACTX_PHY_DESC_ANT_SEL_VALID_OFFSET                                         0x0000000000000008
#define MACTX_PHY_DESC_ANT_SEL_VALID_LSB                                            41
#define MACTX_PHY_DESC_ANT_SEL_VALID_MSB                                            41
#define MACTX_PHY_DESC_ANT_SEL_VALID_MASK                                           0x0000020000000000


 

#define MACTX_PHY_DESC_ANT_SEL_OFFSET                                               0x0000000000000008
#define MACTX_PHY_DESC_ANT_SEL_LSB                                                  42
#define MACTX_PHY_DESC_ANT_SEL_MSB                                                  42
#define MACTX_PHY_DESC_ANT_SEL_MASK                                                 0x0000040000000000


 

#define MACTX_PHY_DESC_CP_SETTING_OFFSET                                            0x0000000000000008
#define MACTX_PHY_DESC_CP_SETTING_LSB                                               43
#define MACTX_PHY_DESC_CP_SETTING_MSB                                               44
#define MACTX_PHY_DESC_CP_SETTING_MASK                                              0x0000180000000000


 

#define MACTX_PHY_DESC_HE_PPDU_SUBTYPE_OFFSET                                       0x0000000000000008
#define MACTX_PHY_DESC_HE_PPDU_SUBTYPE_LSB                                          45
#define MACTX_PHY_DESC_HE_PPDU_SUBTYPE_MSB                                          46
#define MACTX_PHY_DESC_HE_PPDU_SUBTYPE_MASK                                         0x0000600000000000


 

#define MACTX_PHY_DESC_ACTIVE_CHANNEL_OFFSET                                        0x0000000000000008
#define MACTX_PHY_DESC_ACTIVE_CHANNEL_LSB                                           47
#define MACTX_PHY_DESC_ACTIVE_CHANNEL_MSB                                           49
#define MACTX_PHY_DESC_ACTIVE_CHANNEL_MASK                                          0x0003800000000000


 

#define MACTX_PHY_DESC_GENERATE_PHYRX_TX_START_TIMING_OFFSET                        0x0000000000000008
#define MACTX_PHY_DESC_GENERATE_PHYRX_TX_START_TIMING_LSB                           50
#define MACTX_PHY_DESC_GENERATE_PHYRX_TX_START_TIMING_MSB                           50
#define MACTX_PHY_DESC_GENERATE_PHYRX_TX_START_TIMING_MASK                          0x0004000000000000


 

#define MACTX_PHY_DESC_LTF_SIZE_OFFSET                                              0x0000000000000008
#define MACTX_PHY_DESC_LTF_SIZE_LSB                                                 51
#define MACTX_PHY_DESC_LTF_SIZE_MSB                                                 52
#define MACTX_PHY_DESC_LTF_SIZE_MASK                                                0x0018000000000000


 

#define MACTX_PHY_DESC_RU_SIZE_UPDATED_V2_OFFSET                                    0x0000000000000008
#define MACTX_PHY_DESC_RU_SIZE_UPDATED_V2_LSB                                       53
#define MACTX_PHY_DESC_RU_SIZE_UPDATED_V2_MSB                                       56
#define MACTX_PHY_DESC_RU_SIZE_UPDATED_V2_MASK                                      0x01e0000000000000


 

#define MACTX_PHY_DESC_RESERVED_3C_OFFSET                                           0x0000000000000008
#define MACTX_PHY_DESC_RESERVED_3C_LSB                                              57
#define MACTX_PHY_DESC_RESERVED_3C_MSB                                              57
#define MACTX_PHY_DESC_RESERVED_3C_MASK                                             0x0200000000000000


 

#define MACTX_PHY_DESC_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET                       0x0000000000000008
#define MACTX_PHY_DESC_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB                          58
#define MACTX_PHY_DESC_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB                          63
#define MACTX_PHY_DESC_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK                         0xfc00000000000000



#endif    
