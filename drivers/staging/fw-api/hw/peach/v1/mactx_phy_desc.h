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


#ifndef _MACTX_PHY_DESC_H_
#define _MACTX_PHY_DESC_H_

#define NUM_OF_DWORDS_MACTX_PHY_DESC 4

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

#define MACTX_PHY_DESC_RESERVED_0A_OFFSET                                           0x00000000
#define MACTX_PHY_DESC_RESERVED_0A_LSB                                              0
#define MACTX_PHY_DESC_RESERVED_0A_MSB                                              15
#define MACTX_PHY_DESC_RESERVED_0A_MASK                                             0x0000ffff

#define MACTX_PHY_DESC_BF_TYPE_OFFSET                                               0x00000000
#define MACTX_PHY_DESC_BF_TYPE_LSB                                                  16
#define MACTX_PHY_DESC_BF_TYPE_MSB                                                  17
#define MACTX_PHY_DESC_BF_TYPE_MASK                                                 0x00030000

#define MACTX_PHY_DESC_WAIT_SIFS_OFFSET                                             0x00000000
#define MACTX_PHY_DESC_WAIT_SIFS_LSB                                                18
#define MACTX_PHY_DESC_WAIT_SIFS_MSB                                                19
#define MACTX_PHY_DESC_WAIT_SIFS_MASK                                               0x000c0000

#define MACTX_PHY_DESC_DOT11B_PREAMBLE_TYPE_OFFSET                                  0x00000000
#define MACTX_PHY_DESC_DOT11B_PREAMBLE_TYPE_LSB                                     20
#define MACTX_PHY_DESC_DOT11B_PREAMBLE_TYPE_MSB                                     20
#define MACTX_PHY_DESC_DOT11B_PREAMBLE_TYPE_MASK                                    0x00100000

#define MACTX_PHY_DESC_PKT_TYPE_OFFSET                                              0x00000000
#define MACTX_PHY_DESC_PKT_TYPE_LSB                                                 21
#define MACTX_PHY_DESC_PKT_TYPE_MSB                                                 24
#define MACTX_PHY_DESC_PKT_TYPE_MASK                                                0x01e00000

#define MACTX_PHY_DESC_SU_OR_MU_OFFSET                                              0x00000000
#define MACTX_PHY_DESC_SU_OR_MU_LSB                                                 25
#define MACTX_PHY_DESC_SU_OR_MU_MSB                                                 26
#define MACTX_PHY_DESC_SU_OR_MU_MASK                                                0x06000000

#define MACTX_PHY_DESC_MU_TYPE_OFFSET                                               0x00000000
#define MACTX_PHY_DESC_MU_TYPE_LSB                                                  27
#define MACTX_PHY_DESC_MU_TYPE_MSB                                                  27
#define MACTX_PHY_DESC_MU_TYPE_MASK                                                 0x08000000

#define MACTX_PHY_DESC_BANDWIDTH_OFFSET                                             0x00000000
#define MACTX_PHY_DESC_BANDWIDTH_LSB                                                28
#define MACTX_PHY_DESC_BANDWIDTH_MSB                                                30
#define MACTX_PHY_DESC_BANDWIDTH_MASK                                               0x70000000

#define MACTX_PHY_DESC_CHANNEL_CAPTURE_OFFSET                                       0x00000000
#define MACTX_PHY_DESC_CHANNEL_CAPTURE_LSB                                          31
#define MACTX_PHY_DESC_CHANNEL_CAPTURE_MSB                                          31
#define MACTX_PHY_DESC_CHANNEL_CAPTURE_MASK                                         0x80000000

#define MACTX_PHY_DESC_MCS_OFFSET                                                   0x00000004
#define MACTX_PHY_DESC_MCS_LSB                                                      0
#define MACTX_PHY_DESC_MCS_MSB                                                      3
#define MACTX_PHY_DESC_MCS_MASK                                                     0x0000000f

#define MACTX_PHY_DESC_GLOBAL_OFDMA_MIMO_ENABLE_OFFSET                              0x00000004
#define MACTX_PHY_DESC_GLOBAL_OFDMA_MIMO_ENABLE_LSB                                 4
#define MACTX_PHY_DESC_GLOBAL_OFDMA_MIMO_ENABLE_MSB                                 4
#define MACTX_PHY_DESC_GLOBAL_OFDMA_MIMO_ENABLE_MASK                                0x00000010

#define MACTX_PHY_DESC_RESERVED_1A_OFFSET                                           0x00000004
#define MACTX_PHY_DESC_RESERVED_1A_LSB                                              5
#define MACTX_PHY_DESC_RESERVED_1A_MSB                                              5
#define MACTX_PHY_DESC_RESERVED_1A_MASK                                             0x00000020

#define MACTX_PHY_DESC_STBC_OFFSET                                                  0x00000004
#define MACTX_PHY_DESC_STBC_LSB                                                     6
#define MACTX_PHY_DESC_STBC_MSB                                                     6
#define MACTX_PHY_DESC_STBC_MASK                                                    0x00000040

#define MACTX_PHY_DESC_DOT11AX_SU_EXTENDED_OFFSET                                   0x00000004
#define MACTX_PHY_DESC_DOT11AX_SU_EXTENDED_LSB                                      7
#define MACTX_PHY_DESC_DOT11AX_SU_EXTENDED_MSB                                      7
#define MACTX_PHY_DESC_DOT11AX_SU_EXTENDED_MASK                                     0x00000080

#define MACTX_PHY_DESC_DOT11AX_TRIGGER_FRAME_EMBEDDED_OFFSET                        0x00000004
#define MACTX_PHY_DESC_DOT11AX_TRIGGER_FRAME_EMBEDDED_LSB                           8
#define MACTX_PHY_DESC_DOT11AX_TRIGGER_FRAME_EMBEDDED_MSB                           8
#define MACTX_PHY_DESC_DOT11AX_TRIGGER_FRAME_EMBEDDED_MASK                          0x00000100

#define MACTX_PHY_DESC_TX_PWR_SHARED_OFFSET                                         0x00000004
#define MACTX_PHY_DESC_TX_PWR_SHARED_LSB                                            9
#define MACTX_PHY_DESC_TX_PWR_SHARED_MSB                                            16
#define MACTX_PHY_DESC_TX_PWR_SHARED_MASK                                           0x0001fe00

#define MACTX_PHY_DESC_TX_PWR_UNSHARED_OFFSET                                       0x00000004
#define MACTX_PHY_DESC_TX_PWR_UNSHARED_LSB                                          17
#define MACTX_PHY_DESC_TX_PWR_UNSHARED_MSB                                          24
#define MACTX_PHY_DESC_TX_PWR_UNSHARED_MASK                                         0x01fe0000

#define MACTX_PHY_DESC_MEASURE_POWER_OFFSET                                         0x00000004
#define MACTX_PHY_DESC_MEASURE_POWER_LSB                                            25
#define MACTX_PHY_DESC_MEASURE_POWER_MSB                                            25
#define MACTX_PHY_DESC_MEASURE_POWER_MASK                                           0x02000000

#define MACTX_PHY_DESC_TPC_GLUT_SELF_CAL_OFFSET                                     0x00000004
#define MACTX_PHY_DESC_TPC_GLUT_SELF_CAL_LSB                                        26
#define MACTX_PHY_DESC_TPC_GLUT_SELF_CAL_MSB                                        26
#define MACTX_PHY_DESC_TPC_GLUT_SELF_CAL_MASK                                       0x04000000

#define MACTX_PHY_DESC_BACK_TO_BACK_TRANSMISSION_EXPECTED_OFFSET                    0x00000004
#define MACTX_PHY_DESC_BACK_TO_BACK_TRANSMISSION_EXPECTED_LSB                       27
#define MACTX_PHY_DESC_BACK_TO_BACK_TRANSMISSION_EXPECTED_MSB                       27
#define MACTX_PHY_DESC_BACK_TO_BACK_TRANSMISSION_EXPECTED_MASK                      0x08000000

#define MACTX_PHY_DESC_HEAVY_CLIP_NSS_OFFSET                                        0x00000004
#define MACTX_PHY_DESC_HEAVY_CLIP_NSS_LSB                                           28
#define MACTX_PHY_DESC_HEAVY_CLIP_NSS_MSB                                           30
#define MACTX_PHY_DESC_HEAVY_CLIP_NSS_MASK                                          0x70000000

#define MACTX_PHY_DESC_TXBF_PER_PACKET_NO_CSD_NO_WALSH_OFFSET                       0x00000004
#define MACTX_PHY_DESC_TXBF_PER_PACKET_NO_CSD_NO_WALSH_LSB                          31
#define MACTX_PHY_DESC_TXBF_PER_PACKET_NO_CSD_NO_WALSH_MSB                          31
#define MACTX_PHY_DESC_TXBF_PER_PACKET_NO_CSD_NO_WALSH_MASK                         0x80000000

#define MACTX_PHY_DESC_NDP_OFFSET                                                   0x00000008
#define MACTX_PHY_DESC_NDP_LSB                                                      0
#define MACTX_PHY_DESC_NDP_MSB                                                      1
#define MACTX_PHY_DESC_NDP_MASK                                                     0x00000003

#define MACTX_PHY_DESC_UL_FLAG_OFFSET                                               0x00000008
#define MACTX_PHY_DESC_UL_FLAG_LSB                                                  2
#define MACTX_PHY_DESC_UL_FLAG_MSB                                                  2
#define MACTX_PHY_DESC_UL_FLAG_MASK                                                 0x00000004

#define MACTX_PHY_DESC_TRIGGERED_OFFSET                                             0x00000008
#define MACTX_PHY_DESC_TRIGGERED_LSB                                                3
#define MACTX_PHY_DESC_TRIGGERED_MSB                                                3
#define MACTX_PHY_DESC_TRIGGERED_MASK                                               0x00000008

#define MACTX_PHY_DESC_AP_PKT_BW_OFFSET                                             0x00000008
#define MACTX_PHY_DESC_AP_PKT_BW_LSB                                                4
#define MACTX_PHY_DESC_AP_PKT_BW_MSB                                                6
#define MACTX_PHY_DESC_AP_PKT_BW_MASK                                               0x00000070

#define MACTX_PHY_DESC_RU_POSITION_START_OFFSET                                     0x00000008
#define MACTX_PHY_DESC_RU_POSITION_START_LSB                                        7
#define MACTX_PHY_DESC_RU_POSITION_START_MSB                                        14
#define MACTX_PHY_DESC_RU_POSITION_START_MASK                                       0x00007f80

#define MACTX_PHY_DESC_PCU_PPDU_SETUP_START_REASON_OFFSET                           0x00000008
#define MACTX_PHY_DESC_PCU_PPDU_SETUP_START_REASON_LSB                              15
#define MACTX_PHY_DESC_PCU_PPDU_SETUP_START_REASON_MSB                              17
#define MACTX_PHY_DESC_PCU_PPDU_SETUP_START_REASON_MASK                             0x00038000

#define MACTX_PHY_DESC_TLV_SOURCE_OFFSET                                            0x00000008
#define MACTX_PHY_DESC_TLV_SOURCE_LSB                                               18
#define MACTX_PHY_DESC_TLV_SOURCE_MSB                                               18
#define MACTX_PHY_DESC_TLV_SOURCE_MASK                                              0x00040000

#define MACTX_PHY_DESC_RESERVED_2A_OFFSET                                           0x00000008
#define MACTX_PHY_DESC_RESERVED_2A_LSB                                              19
#define MACTX_PHY_DESC_RESERVED_2A_MSB                                              20
#define MACTX_PHY_DESC_RESERVED_2A_MASK                                             0x00180000

#define MACTX_PHY_DESC_NSS_OFFSET                                                   0x00000008
#define MACTX_PHY_DESC_NSS_LSB                                                      21
#define MACTX_PHY_DESC_NSS_MSB                                                      23
#define MACTX_PHY_DESC_NSS_MASK                                                     0x00e00000

#define MACTX_PHY_DESC_STREAM_OFFSET_OFFSET                                         0x00000008
#define MACTX_PHY_DESC_STREAM_OFFSET_LSB                                            24
#define MACTX_PHY_DESC_STREAM_OFFSET_MSB                                            26
#define MACTX_PHY_DESC_STREAM_OFFSET_MASK                                           0x07000000

#define MACTX_PHY_DESC_RESERVED_2B_OFFSET                                           0x00000008
#define MACTX_PHY_DESC_RESERVED_2B_LSB                                              27
#define MACTX_PHY_DESC_RESERVED_2B_MSB                                              28
#define MACTX_PHY_DESC_RESERVED_2B_MASK                                             0x18000000

#define MACTX_PHY_DESC_CLPC_ENABLE_OFFSET                                           0x00000008
#define MACTX_PHY_DESC_CLPC_ENABLE_LSB                                              29
#define MACTX_PHY_DESC_CLPC_ENABLE_MSB                                              29
#define MACTX_PHY_DESC_CLPC_ENABLE_MASK                                             0x20000000

#define MACTX_PHY_DESC_MU_NDP_OFFSET                                                0x00000008
#define MACTX_PHY_DESC_MU_NDP_LSB                                                   30
#define MACTX_PHY_DESC_MU_NDP_MSB                                                   30
#define MACTX_PHY_DESC_MU_NDP_MASK                                                  0x40000000

#define MACTX_PHY_DESC_RESPONSE_EXPECTED_OFFSET                                     0x00000008
#define MACTX_PHY_DESC_RESPONSE_EXPECTED_LSB                                        31
#define MACTX_PHY_DESC_RESPONSE_EXPECTED_MSB                                        31
#define MACTX_PHY_DESC_RESPONSE_EXPECTED_MASK                                       0x80000000

#define MACTX_PHY_DESC_RX_CHAIN_MASK_OFFSET                                         0x0000000c
#define MACTX_PHY_DESC_RX_CHAIN_MASK_LSB                                            0
#define MACTX_PHY_DESC_RX_CHAIN_MASK_MSB                                            7
#define MACTX_PHY_DESC_RX_CHAIN_MASK_MASK                                           0x000000ff

#define MACTX_PHY_DESC_RX_CHAIN_MASK_VALID_OFFSET                                   0x0000000c
#define MACTX_PHY_DESC_RX_CHAIN_MASK_VALID_LSB                                      8
#define MACTX_PHY_DESC_RX_CHAIN_MASK_VALID_MSB                                      8
#define MACTX_PHY_DESC_RX_CHAIN_MASK_VALID_MASK                                     0x00000100

#define MACTX_PHY_DESC_ANT_SEL_VALID_OFFSET                                         0x0000000c
#define MACTX_PHY_DESC_ANT_SEL_VALID_LSB                                            9
#define MACTX_PHY_DESC_ANT_SEL_VALID_MSB                                            9
#define MACTX_PHY_DESC_ANT_SEL_VALID_MASK                                           0x00000200

#define MACTX_PHY_DESC_ANT_SEL_OFFSET                                               0x0000000c
#define MACTX_PHY_DESC_ANT_SEL_LSB                                                  10
#define MACTX_PHY_DESC_ANT_SEL_MSB                                                  10
#define MACTX_PHY_DESC_ANT_SEL_MASK                                                 0x00000400

#define MACTX_PHY_DESC_CP_SETTING_OFFSET                                            0x0000000c
#define MACTX_PHY_DESC_CP_SETTING_LSB                                               11
#define MACTX_PHY_DESC_CP_SETTING_MSB                                               12
#define MACTX_PHY_DESC_CP_SETTING_MASK                                              0x00001800

#define MACTX_PHY_DESC_HE_PPDU_SUBTYPE_OFFSET                                       0x0000000c
#define MACTX_PHY_DESC_HE_PPDU_SUBTYPE_LSB                                          13
#define MACTX_PHY_DESC_HE_PPDU_SUBTYPE_MSB                                          14
#define MACTX_PHY_DESC_HE_PPDU_SUBTYPE_MASK                                         0x00006000

#define MACTX_PHY_DESC_ACTIVE_CHANNEL_OFFSET                                        0x0000000c
#define MACTX_PHY_DESC_ACTIVE_CHANNEL_LSB                                           15
#define MACTX_PHY_DESC_ACTIVE_CHANNEL_MSB                                           17
#define MACTX_PHY_DESC_ACTIVE_CHANNEL_MASK                                          0x00038000

#define MACTX_PHY_DESC_GENERATE_PHYRX_TX_START_TIMING_OFFSET                        0x0000000c
#define MACTX_PHY_DESC_GENERATE_PHYRX_TX_START_TIMING_LSB                           18
#define MACTX_PHY_DESC_GENERATE_PHYRX_TX_START_TIMING_MSB                           18
#define MACTX_PHY_DESC_GENERATE_PHYRX_TX_START_TIMING_MASK                          0x00040000

#define MACTX_PHY_DESC_LTF_SIZE_OFFSET                                              0x0000000c
#define MACTX_PHY_DESC_LTF_SIZE_LSB                                                 19
#define MACTX_PHY_DESC_LTF_SIZE_MSB                                                 20
#define MACTX_PHY_DESC_LTF_SIZE_MASK                                                0x00180000

#define MACTX_PHY_DESC_RU_SIZE_UPDATED_V2_OFFSET                                    0x0000000c
#define MACTX_PHY_DESC_RU_SIZE_UPDATED_V2_LSB                                       21
#define MACTX_PHY_DESC_RU_SIZE_UPDATED_V2_MSB                                       24
#define MACTX_PHY_DESC_RU_SIZE_UPDATED_V2_MASK                                      0x01e00000

#define MACTX_PHY_DESC_RESERVED_3C_OFFSET                                           0x0000000c
#define MACTX_PHY_DESC_RESERVED_3C_LSB                                              25
#define MACTX_PHY_DESC_RESERVED_3C_MSB                                              25
#define MACTX_PHY_DESC_RESERVED_3C_MASK                                             0x02000000

#define MACTX_PHY_DESC_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET                       0x0000000c
#define MACTX_PHY_DESC_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB                          26
#define MACTX_PHY_DESC_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB                          31
#define MACTX_PHY_DESC_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK                         0xfc000000

#endif
