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


#ifndef _OFDMA_TRIGGER_DETAILS_H_
#define _OFDMA_TRIGGER_DETAILS_H_

#include "mlo_sta_id_details.h"
#define NUM_OF_DWORDS_OFDMA_TRIGGER_DETAILS 22

struct ofdma_trigger_details {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ax_trigger_source                                       :  1,
                      rx_trigger_frame_user_source                            :  2,
                      received_bandwidth                                      :  3,
                      txop_duration_all_ones                                  :  1,
                      eht_trigger_response                                    :  1,
                      pre_rssi_comb                                           :  8,
                      rssi_comb                                               :  8,
                      rxpcu_pcie_l0_req_duration                              :  8;
             uint32_t he_trigger_ul_ppdu_length                               :  5,
                      he_trigger_ru_allocation                                :  8,
                      he_trigger_dl_tx_power                                  :  5,
                      he_trigger_ul_target_rssi                               :  5,
                      he_trigger_ul_mcs                                       :  2,
                      he_trigger_reserved                                     :  1,
                      bss_color                                               :  6;
             uint32_t trigger_type                                            :  4,
                      lsig_response_length                                    : 12,
                      cascade_indication                                      :  1,
                      carrier_sense                                           :  1,
                      bandwidth                                               :  2,
                      cp_ltf_size                                             :  2,
                      mu_mimo_ltf_mode                                        :  1,
                      number_of_ltfs                                          :  3,
                      stbc                                                    :  1,
                      ldpc_extra_symbol                                       :  1,
                      ap_tx_power_lsb_part                                    :  4;
             uint32_t ap_tx_power_msb_part                                    :  2,
                      packet_extension_a_factor                               :  2,
                      packet_extension_pe_disambiguity                        :  1,
                      spatial_reuse                                           : 16,
                      doppler                                                 :  1,
                      he_siga_reserved                                        :  9,
                      reserved_3b                                             :  1;
             uint32_t aid12                                                   : 12,
                      ru_allocation                                           :  9,
                      mcs                                                     :  4,
                      dcm                                                     :  1,
                      start_spatial_stream                                    :  3,
                      number_of_spatial_stream                                :  3;
             uint32_t target_rssi                                             :  7,
                      coding_type                                             :  1,
                      mpdu_mu_spacing_factor                                  :  2,
                      tid_aggregation_limit                                   :  3,
                      reserved_5b                                             :  1,
                      prefered_ac                                             :  2,
                      bar_control_ack_policy                                  :  1,
                      bar_control_multi_tid                                   :  1,
                      bar_control_compressed_bitmap                           :  1,
                      bar_control_reserved                                    :  9,
                      bar_control_tid_info                                    :  4;
             uint32_t nr0_per_tid_info_reserved                               : 12,
                      nr0_per_tid_info_tid_value                              :  4,
                      nr0_start_seq_ctrl_frag_number                          :  4,
                      nr0_start_seq_ctrl_start_seq_number                     : 12;
             uint32_t nr1_per_tid_info_reserved                               : 12,
                      nr1_per_tid_info_tid_value                              :  4,
                      nr1_start_seq_ctrl_frag_number                          :  4,
                      nr1_start_seq_ctrl_start_seq_number                     : 12;
             uint32_t nr2_per_tid_info_reserved                               : 12,
                      nr2_per_tid_info_tid_value                              :  4,
                      nr2_start_seq_ctrl_frag_number                          :  4,
                      nr2_start_seq_ctrl_start_seq_number                     : 12;
             uint32_t nr3_per_tid_info_reserved                               : 12,
                      nr3_per_tid_info_tid_value                              :  4,
                      nr3_start_seq_ctrl_frag_number                          :  4,
                      nr3_start_seq_ctrl_start_seq_number                     : 12;
             uint32_t nr4_per_tid_info_reserved                               : 12,
                      nr4_per_tid_info_tid_value                              :  4,
                      nr4_start_seq_ctrl_frag_number                          :  4,
                      nr4_start_seq_ctrl_start_seq_number                     : 12;
             uint32_t nr5_per_tid_info_reserved                               : 12,
                      nr5_per_tid_info_tid_value                              :  4,
                      nr5_start_seq_ctrl_frag_number                          :  4,
                      nr5_start_seq_ctrl_start_seq_number                     : 12;
             uint32_t nr6_per_tid_info_reserved                               : 12,
                      nr6_per_tid_info_tid_value                              :  4,
                      nr6_start_seq_ctrl_frag_number                          :  4,
                      nr6_start_seq_ctrl_start_seq_number                     : 12;
             uint32_t nr7_per_tid_info_reserved                               : 12,
                      nr7_per_tid_info_tid_value                              :  4,
                      nr7_start_seq_ctrl_frag_number                          :  4,
                      nr7_start_seq_ctrl_start_seq_number                     : 12;
             uint32_t fb_segment_retransmission_bitmap                        :  8,
                      reserved_14a                                            :  2,
                      u_sig_puncture_pattern_encoding                         :  6,
                      dot11be_puncture_bitmap                                 : 16;
             uint32_t rx_chain_mask                                           :  8,
                      rx_duration_field                                       : 16,
                      scrambler_seed                                          :  7,
                      rx_chain_mask_type                                      :  1;
             struct   mlo_sta_id_details                                        mlo_sta_id_details_rx;
             uint16_t normalized_pre_rssi_comb                                :  8,
                      normalized_rssi_comb                                    :  8;
             uint32_t sw_peer_id                                              : 16,
                      response_tx_duration                                    : 16;
             uint32_t __reserved_g_0005_trigger_subtype                                 :  4,
                      tbr_trigger_common_info_79_68                           : 12,
                      tbr_trigger_sound_reserved_20_12                        :  9,
                      i2r_rep                                                 :  3,
                      tbr_trigger_sound_reserved_25_24                        :  2,
                      reserved_18a                                            :  1,
                      qos_null_only_response_tx                               :  1;
             uint32_t tbr_trigger_sound_sac                                   : 16,
                      reserved_19a                                            :  8,
                      u_sig_reserved2                                         :  5,
                      reserved_19b                                            :  3;
             uint32_t eht_special_aid12                                       : 12,
                      phy_version                                             :  3,
                      bandwidth_ext                                           :  2,
                      eht_spatial_reuse                                       :  8,
                      u_sig_reserved1                                         :  7;
             uint32_t eht_trigger_special_user_info_71_40                     : 32;
#else
             uint32_t rxpcu_pcie_l0_req_duration                              :  8,
                      rssi_comb                                               :  8,
                      pre_rssi_comb                                           :  8,
                      eht_trigger_response                                    :  1,
                      txop_duration_all_ones                                  :  1,
                      received_bandwidth                                      :  3,
                      rx_trigger_frame_user_source                            :  2,
                      ax_trigger_source                                       :  1;
             uint32_t bss_color                                               :  6,
                      he_trigger_reserved                                     :  1,
                      he_trigger_ul_mcs                                       :  2,
                      he_trigger_ul_target_rssi                               :  5,
                      he_trigger_dl_tx_power                                  :  5,
                      he_trigger_ru_allocation                                :  8,
                      he_trigger_ul_ppdu_length                               :  5;
             uint32_t ap_tx_power_lsb_part                                    :  4,
                      ldpc_extra_symbol                                       :  1,
                      stbc                                                    :  1,
                      number_of_ltfs                                          :  3,
                      mu_mimo_ltf_mode                                        :  1,
                      cp_ltf_size                                             :  2,
                      bandwidth                                               :  2,
                      carrier_sense                                           :  1,
                      cascade_indication                                      :  1,
                      lsig_response_length                                    : 12,
                      trigger_type                                            :  4;
             uint32_t reserved_3b                                             :  1,
                      he_siga_reserved                                        :  9,
                      doppler                                                 :  1,
                      spatial_reuse                                           : 16,
                      packet_extension_pe_disambiguity                        :  1,
                      packet_extension_a_factor                               :  2,
                      ap_tx_power_msb_part                                    :  2;
             uint32_t number_of_spatial_stream                                :  3,
                      start_spatial_stream                                    :  3,
                      dcm                                                     :  1,
                      mcs                                                     :  4,
                      ru_allocation                                           :  9,
                      aid12                                                   : 12;
             uint32_t bar_control_tid_info                                    :  4,
                      bar_control_reserved                                    :  9,
                      bar_control_compressed_bitmap                           :  1,
                      bar_control_multi_tid                                   :  1,
                      bar_control_ack_policy                                  :  1,
                      prefered_ac                                             :  2,
                      reserved_5b                                             :  1,
                      tid_aggregation_limit                                   :  3,
                      mpdu_mu_spacing_factor                                  :  2,
                      coding_type                                             :  1,
                      target_rssi                                             :  7;
             uint32_t nr0_start_seq_ctrl_start_seq_number                     : 12,
                      nr0_start_seq_ctrl_frag_number                          :  4,
                      nr0_per_tid_info_tid_value                              :  4,
                      nr0_per_tid_info_reserved                               : 12;
             uint32_t nr1_start_seq_ctrl_start_seq_number                     : 12,
                      nr1_start_seq_ctrl_frag_number                          :  4,
                      nr1_per_tid_info_tid_value                              :  4,
                      nr1_per_tid_info_reserved                               : 12;
             uint32_t nr2_start_seq_ctrl_start_seq_number                     : 12,
                      nr2_start_seq_ctrl_frag_number                          :  4,
                      nr2_per_tid_info_tid_value                              :  4,
                      nr2_per_tid_info_reserved                               : 12;
             uint32_t nr3_start_seq_ctrl_start_seq_number                     : 12,
                      nr3_start_seq_ctrl_frag_number                          :  4,
                      nr3_per_tid_info_tid_value                              :  4,
                      nr3_per_tid_info_reserved                               : 12;
             uint32_t nr4_start_seq_ctrl_start_seq_number                     : 12,
                      nr4_start_seq_ctrl_frag_number                          :  4,
                      nr4_per_tid_info_tid_value                              :  4,
                      nr4_per_tid_info_reserved                               : 12;
             uint32_t nr5_start_seq_ctrl_start_seq_number                     : 12,
                      nr5_start_seq_ctrl_frag_number                          :  4,
                      nr5_per_tid_info_tid_value                              :  4,
                      nr5_per_tid_info_reserved                               : 12;
             uint32_t nr6_start_seq_ctrl_start_seq_number                     : 12,
                      nr6_start_seq_ctrl_frag_number                          :  4,
                      nr6_per_tid_info_tid_value                              :  4,
                      nr6_per_tid_info_reserved                               : 12;
             uint32_t nr7_start_seq_ctrl_start_seq_number                     : 12,
                      nr7_start_seq_ctrl_frag_number                          :  4,
                      nr7_per_tid_info_tid_value                              :  4,
                      nr7_per_tid_info_reserved                               : 12;
             uint32_t dot11be_puncture_bitmap                                 : 16,
                      u_sig_puncture_pattern_encoding                         :  6,
                      reserved_14a                                            :  2,
                      fb_segment_retransmission_bitmap                        :  8;
             uint32_t rx_chain_mask_type                                      :  1,
                      scrambler_seed                                          :  7,
                      rx_duration_field                                       : 16,
                      rx_chain_mask                                           :  8;
             uint32_t normalized_rssi_comb                                    :  8,
                      normalized_pre_rssi_comb                                :  8;
             struct   mlo_sta_id_details                                        mlo_sta_id_details_rx;
             uint32_t response_tx_duration                                    : 16,
                      sw_peer_id                                              : 16;
             uint32_t qos_null_only_response_tx                               :  1,
                      reserved_18a                                            :  1,
                      tbr_trigger_sound_reserved_25_24                        :  2,
                      i2r_rep                                                 :  3,
                      tbr_trigger_sound_reserved_20_12                        :  9,
                      tbr_trigger_common_info_79_68                           : 12,
                      __reserved_g_0005_trigger_subtype                                 :  4;
             uint32_t reserved_19b                                            :  3,
                      u_sig_reserved2                                         :  5,
                      reserved_19a                                            :  8,
                      tbr_trigger_sound_sac                                   : 16;
             uint32_t u_sig_reserved1                                         :  7,
                      eht_spatial_reuse                                       :  8,
                      bandwidth_ext                                           :  2,
                      phy_version                                             :  3,
                      eht_special_aid12                                       : 12;
             uint32_t eht_trigger_special_user_info_71_40                     : 32;
#endif
};

#define OFDMA_TRIGGER_DETAILS_AX_TRIGGER_SOURCE_OFFSET                              0x00000000
#define OFDMA_TRIGGER_DETAILS_AX_TRIGGER_SOURCE_LSB                                 0
#define OFDMA_TRIGGER_DETAILS_AX_TRIGGER_SOURCE_MSB                                 0
#define OFDMA_TRIGGER_DETAILS_AX_TRIGGER_SOURCE_MASK                                0x00000001

#define OFDMA_TRIGGER_DETAILS_RX_TRIGGER_FRAME_USER_SOURCE_OFFSET                   0x00000000
#define OFDMA_TRIGGER_DETAILS_RX_TRIGGER_FRAME_USER_SOURCE_LSB                      1
#define OFDMA_TRIGGER_DETAILS_RX_TRIGGER_FRAME_USER_SOURCE_MSB                      2
#define OFDMA_TRIGGER_DETAILS_RX_TRIGGER_FRAME_USER_SOURCE_MASK                     0x00000006

#define OFDMA_TRIGGER_DETAILS_RECEIVED_BANDWIDTH_OFFSET                             0x00000000
#define OFDMA_TRIGGER_DETAILS_RECEIVED_BANDWIDTH_LSB                                3
#define OFDMA_TRIGGER_DETAILS_RECEIVED_BANDWIDTH_MSB                                5
#define OFDMA_TRIGGER_DETAILS_RECEIVED_BANDWIDTH_MASK                               0x00000038

#define OFDMA_TRIGGER_DETAILS_TXOP_DURATION_ALL_ONES_OFFSET                         0x00000000
#define OFDMA_TRIGGER_DETAILS_TXOP_DURATION_ALL_ONES_LSB                            6
#define OFDMA_TRIGGER_DETAILS_TXOP_DURATION_ALL_ONES_MSB                            6
#define OFDMA_TRIGGER_DETAILS_TXOP_DURATION_ALL_ONES_MASK                           0x00000040

#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_RESPONSE_OFFSET                           0x00000000
#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_RESPONSE_LSB                              7
#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_RESPONSE_MSB                              7
#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_RESPONSE_MASK                             0x00000080

#define OFDMA_TRIGGER_DETAILS_PRE_RSSI_COMB_OFFSET                                  0x00000000
#define OFDMA_TRIGGER_DETAILS_PRE_RSSI_COMB_LSB                                     8
#define OFDMA_TRIGGER_DETAILS_PRE_RSSI_COMB_MSB                                     15
#define OFDMA_TRIGGER_DETAILS_PRE_RSSI_COMB_MASK                                    0x0000ff00

#define OFDMA_TRIGGER_DETAILS_RSSI_COMB_OFFSET                                      0x00000000
#define OFDMA_TRIGGER_DETAILS_RSSI_COMB_LSB                                         16
#define OFDMA_TRIGGER_DETAILS_RSSI_COMB_MSB                                         23
#define OFDMA_TRIGGER_DETAILS_RSSI_COMB_MASK                                        0x00ff0000

#define OFDMA_TRIGGER_DETAILS_RXPCU_PCIE_L0_REQ_DURATION_OFFSET                     0x00000000
#define OFDMA_TRIGGER_DETAILS_RXPCU_PCIE_L0_REQ_DURATION_LSB                        24
#define OFDMA_TRIGGER_DETAILS_RXPCU_PCIE_L0_REQ_DURATION_MSB                        31
#define OFDMA_TRIGGER_DETAILS_RXPCU_PCIE_L0_REQ_DURATION_MASK                       0xff000000

#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_PPDU_LENGTH_OFFSET                      0x00000004
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_PPDU_LENGTH_LSB                         0
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_PPDU_LENGTH_MSB                         4
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_PPDU_LENGTH_MASK                        0x0000001f

#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RU_ALLOCATION_OFFSET                       0x00000004
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RU_ALLOCATION_LSB                          5
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RU_ALLOCATION_MSB                          12
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RU_ALLOCATION_MASK                         0x00001fe0

#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_DL_TX_POWER_OFFSET                         0x00000004
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_DL_TX_POWER_LSB                            13
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_DL_TX_POWER_MSB                            17
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_DL_TX_POWER_MASK                           0x0003e000

#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_TARGET_RSSI_OFFSET                      0x00000004
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_TARGET_RSSI_LSB                         18
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_TARGET_RSSI_MSB                         22
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_TARGET_RSSI_MASK                        0x007c0000

#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_MCS_OFFSET                              0x00000004
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_MCS_LSB                                 23
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_MCS_MSB                                 24
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_UL_MCS_MASK                                0x01800000

#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RESERVED_OFFSET                            0x00000004
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RESERVED_LSB                               25
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RESERVED_MSB                               25
#define OFDMA_TRIGGER_DETAILS_HE_TRIGGER_RESERVED_MASK                              0x02000000

#define OFDMA_TRIGGER_DETAILS_BSS_COLOR_OFFSET                                      0x00000004
#define OFDMA_TRIGGER_DETAILS_BSS_COLOR_LSB                                         26
#define OFDMA_TRIGGER_DETAILS_BSS_COLOR_MSB                                         31
#define OFDMA_TRIGGER_DETAILS_BSS_COLOR_MASK                                        0xfc000000

#define OFDMA_TRIGGER_DETAILS_TRIGGER_TYPE_OFFSET                                   0x00000008
#define OFDMA_TRIGGER_DETAILS_TRIGGER_TYPE_LSB                                      0
#define OFDMA_TRIGGER_DETAILS_TRIGGER_TYPE_MSB                                      3
#define OFDMA_TRIGGER_DETAILS_TRIGGER_TYPE_MASK                                     0x0000000f

#define OFDMA_TRIGGER_DETAILS_LSIG_RESPONSE_LENGTH_OFFSET                           0x00000008
#define OFDMA_TRIGGER_DETAILS_LSIG_RESPONSE_LENGTH_LSB                              4
#define OFDMA_TRIGGER_DETAILS_LSIG_RESPONSE_LENGTH_MSB                              15
#define OFDMA_TRIGGER_DETAILS_LSIG_RESPONSE_LENGTH_MASK                             0x0000fff0

#define OFDMA_TRIGGER_DETAILS_CASCADE_INDICATION_OFFSET                             0x00000008
#define OFDMA_TRIGGER_DETAILS_CASCADE_INDICATION_LSB                                16
#define OFDMA_TRIGGER_DETAILS_CASCADE_INDICATION_MSB                                16
#define OFDMA_TRIGGER_DETAILS_CASCADE_INDICATION_MASK                               0x00010000

#define OFDMA_TRIGGER_DETAILS_CARRIER_SENSE_OFFSET                                  0x00000008
#define OFDMA_TRIGGER_DETAILS_CARRIER_SENSE_LSB                                     17
#define OFDMA_TRIGGER_DETAILS_CARRIER_SENSE_MSB                                     17
#define OFDMA_TRIGGER_DETAILS_CARRIER_SENSE_MASK                                    0x00020000

#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_OFFSET                                      0x00000008
#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_LSB                                         18
#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_MSB                                         19
#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_MASK                                        0x000c0000

#define OFDMA_TRIGGER_DETAILS_CP_LTF_SIZE_OFFSET                                    0x00000008
#define OFDMA_TRIGGER_DETAILS_CP_LTF_SIZE_LSB                                       20
#define OFDMA_TRIGGER_DETAILS_CP_LTF_SIZE_MSB                                       21
#define OFDMA_TRIGGER_DETAILS_CP_LTF_SIZE_MASK                                      0x00300000

#define OFDMA_TRIGGER_DETAILS_MU_MIMO_LTF_MODE_OFFSET                               0x00000008
#define OFDMA_TRIGGER_DETAILS_MU_MIMO_LTF_MODE_LSB                                  22
#define OFDMA_TRIGGER_DETAILS_MU_MIMO_LTF_MODE_MSB                                  22
#define OFDMA_TRIGGER_DETAILS_MU_MIMO_LTF_MODE_MASK                                 0x00400000

#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_LTFS_OFFSET                                 0x00000008
#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_LTFS_LSB                                    23
#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_LTFS_MSB                                    25
#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_LTFS_MASK                                   0x03800000

#define OFDMA_TRIGGER_DETAILS_STBC_OFFSET                                           0x00000008
#define OFDMA_TRIGGER_DETAILS_STBC_LSB                                              26
#define OFDMA_TRIGGER_DETAILS_STBC_MSB                                              26
#define OFDMA_TRIGGER_DETAILS_STBC_MASK                                             0x04000000

#define OFDMA_TRIGGER_DETAILS_LDPC_EXTRA_SYMBOL_OFFSET                              0x00000008
#define OFDMA_TRIGGER_DETAILS_LDPC_EXTRA_SYMBOL_LSB                                 27
#define OFDMA_TRIGGER_DETAILS_LDPC_EXTRA_SYMBOL_MSB                                 27
#define OFDMA_TRIGGER_DETAILS_LDPC_EXTRA_SYMBOL_MASK                                0x08000000

#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_LSB_PART_OFFSET                           0x00000008
#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_LSB_PART_LSB                              28
#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_LSB_PART_MSB                              31
#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_LSB_PART_MASK                             0xf0000000

#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_MSB_PART_OFFSET                           0x0000000c
#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_MSB_PART_LSB                              0
#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_MSB_PART_MSB                              1
#define OFDMA_TRIGGER_DETAILS_AP_TX_POWER_MSB_PART_MASK                             0x00000003

#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_A_FACTOR_OFFSET                      0x0000000c
#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_A_FACTOR_LSB                         2
#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_A_FACTOR_MSB                         3
#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_A_FACTOR_MASK                        0x0000000c

#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_PE_DISAMBIGUITY_OFFSET               0x0000000c
#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_PE_DISAMBIGUITY_LSB                  4
#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_PE_DISAMBIGUITY_MSB                  4
#define OFDMA_TRIGGER_DETAILS_PACKET_EXTENSION_PE_DISAMBIGUITY_MASK                 0x00000010

#define OFDMA_TRIGGER_DETAILS_SPATIAL_REUSE_OFFSET                                  0x0000000c
#define OFDMA_TRIGGER_DETAILS_SPATIAL_REUSE_LSB                                     5
#define OFDMA_TRIGGER_DETAILS_SPATIAL_REUSE_MSB                                     20
#define OFDMA_TRIGGER_DETAILS_SPATIAL_REUSE_MASK                                    0x001fffe0

#define OFDMA_TRIGGER_DETAILS_DOPPLER_OFFSET                                        0x0000000c
#define OFDMA_TRIGGER_DETAILS_DOPPLER_LSB                                           21
#define OFDMA_TRIGGER_DETAILS_DOPPLER_MSB                                           21
#define OFDMA_TRIGGER_DETAILS_DOPPLER_MASK                                          0x00200000

#define OFDMA_TRIGGER_DETAILS_HE_SIGA_RESERVED_OFFSET                               0x0000000c
#define OFDMA_TRIGGER_DETAILS_HE_SIGA_RESERVED_LSB                                  22
#define OFDMA_TRIGGER_DETAILS_HE_SIGA_RESERVED_MSB                                  30
#define OFDMA_TRIGGER_DETAILS_HE_SIGA_RESERVED_MASK                                 0x7fc00000

#define OFDMA_TRIGGER_DETAILS_RESERVED_3B_OFFSET                                    0x0000000c
#define OFDMA_TRIGGER_DETAILS_RESERVED_3B_LSB                                       31
#define OFDMA_TRIGGER_DETAILS_RESERVED_3B_MSB                                       31
#define OFDMA_TRIGGER_DETAILS_RESERVED_3B_MASK                                      0x80000000

#define OFDMA_TRIGGER_DETAILS_AID12_OFFSET                                          0x00000010
#define OFDMA_TRIGGER_DETAILS_AID12_LSB                                             0
#define OFDMA_TRIGGER_DETAILS_AID12_MSB                                             11
#define OFDMA_TRIGGER_DETAILS_AID12_MASK                                            0x00000fff

#define OFDMA_TRIGGER_DETAILS_RU_ALLOCATION_OFFSET                                  0x00000010
#define OFDMA_TRIGGER_DETAILS_RU_ALLOCATION_LSB                                     12
#define OFDMA_TRIGGER_DETAILS_RU_ALLOCATION_MSB                                     20
#define OFDMA_TRIGGER_DETAILS_RU_ALLOCATION_MASK                                    0x001ff000

#define OFDMA_TRIGGER_DETAILS_MCS_OFFSET                                            0x00000010
#define OFDMA_TRIGGER_DETAILS_MCS_LSB                                               21
#define OFDMA_TRIGGER_DETAILS_MCS_MSB                                               24
#define OFDMA_TRIGGER_DETAILS_MCS_MASK                                              0x01e00000

#define OFDMA_TRIGGER_DETAILS_DCM_OFFSET                                            0x00000010
#define OFDMA_TRIGGER_DETAILS_DCM_LSB                                               25
#define OFDMA_TRIGGER_DETAILS_DCM_MSB                                               25
#define OFDMA_TRIGGER_DETAILS_DCM_MASK                                              0x02000000

#define OFDMA_TRIGGER_DETAILS_START_SPATIAL_STREAM_OFFSET                           0x00000010
#define OFDMA_TRIGGER_DETAILS_START_SPATIAL_STREAM_LSB                              26
#define OFDMA_TRIGGER_DETAILS_START_SPATIAL_STREAM_MSB                              28
#define OFDMA_TRIGGER_DETAILS_START_SPATIAL_STREAM_MASK                             0x1c000000

#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_SPATIAL_STREAM_OFFSET                       0x00000010
#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_SPATIAL_STREAM_LSB                          29
#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_SPATIAL_STREAM_MSB                          31
#define OFDMA_TRIGGER_DETAILS_NUMBER_OF_SPATIAL_STREAM_MASK                         0xe0000000

#define OFDMA_TRIGGER_DETAILS_TARGET_RSSI_OFFSET                                    0x00000014
#define OFDMA_TRIGGER_DETAILS_TARGET_RSSI_LSB                                       0
#define OFDMA_TRIGGER_DETAILS_TARGET_RSSI_MSB                                       6
#define OFDMA_TRIGGER_DETAILS_TARGET_RSSI_MASK                                      0x0000007f

#define OFDMA_TRIGGER_DETAILS_CODING_TYPE_OFFSET                                    0x00000014
#define OFDMA_TRIGGER_DETAILS_CODING_TYPE_LSB                                       7
#define OFDMA_TRIGGER_DETAILS_CODING_TYPE_MSB                                       7
#define OFDMA_TRIGGER_DETAILS_CODING_TYPE_MASK                                      0x00000080

#define OFDMA_TRIGGER_DETAILS_MPDU_MU_SPACING_FACTOR_OFFSET                         0x00000014
#define OFDMA_TRIGGER_DETAILS_MPDU_MU_SPACING_FACTOR_LSB                            8
#define OFDMA_TRIGGER_DETAILS_MPDU_MU_SPACING_FACTOR_MSB                            9
#define OFDMA_TRIGGER_DETAILS_MPDU_MU_SPACING_FACTOR_MASK                           0x00000300

#define OFDMA_TRIGGER_DETAILS_TID_AGGREGATION_LIMIT_OFFSET                          0x00000014
#define OFDMA_TRIGGER_DETAILS_TID_AGGREGATION_LIMIT_LSB                             10
#define OFDMA_TRIGGER_DETAILS_TID_AGGREGATION_LIMIT_MSB                             12
#define OFDMA_TRIGGER_DETAILS_TID_AGGREGATION_LIMIT_MASK                            0x00001c00

#define OFDMA_TRIGGER_DETAILS_RESERVED_5B_OFFSET                                    0x00000014
#define OFDMA_TRIGGER_DETAILS_RESERVED_5B_LSB                                       13
#define OFDMA_TRIGGER_DETAILS_RESERVED_5B_MSB                                       13
#define OFDMA_TRIGGER_DETAILS_RESERVED_5B_MASK                                      0x00002000

#define OFDMA_TRIGGER_DETAILS_PREFERED_AC_OFFSET                                    0x00000014
#define OFDMA_TRIGGER_DETAILS_PREFERED_AC_LSB                                       14
#define OFDMA_TRIGGER_DETAILS_PREFERED_AC_MSB                                       15
#define OFDMA_TRIGGER_DETAILS_PREFERED_AC_MASK                                      0x0000c000

#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_ACK_POLICY_OFFSET                         0x00000014
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_ACK_POLICY_LSB                            16
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_ACK_POLICY_MSB                            16
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_ACK_POLICY_MASK                           0x00010000

#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_MULTI_TID_OFFSET                          0x00000014
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_MULTI_TID_LSB                             17
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_MULTI_TID_MSB                             17
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_MULTI_TID_MASK                            0x00020000

#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_COMPRESSED_BITMAP_OFFSET                  0x00000014
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_COMPRESSED_BITMAP_LSB                     18
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_COMPRESSED_BITMAP_MSB                     18
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_COMPRESSED_BITMAP_MASK                    0x00040000

#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_RESERVED_OFFSET                           0x00000014
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_RESERVED_LSB                              19
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_RESERVED_MSB                              27
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_RESERVED_MASK                             0x0ff80000

#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_TID_INFO_OFFSET                           0x00000014
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_TID_INFO_LSB                              28
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_TID_INFO_MSB                              31
#define OFDMA_TRIGGER_DETAILS_BAR_CONTROL_TID_INFO_MASK                             0xf0000000

#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_RESERVED_OFFSET                      0x00000018
#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_RESERVED_LSB                         0
#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_RESERVED_MSB                         11
#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_RESERVED_MASK                        0x00000fff

#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_TID_VALUE_OFFSET                     0x00000018
#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_TID_VALUE_LSB                        12
#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_TID_VALUE_MSB                        15
#define OFDMA_TRIGGER_DETAILS_NR0_PER_TID_INFO_TID_VALUE_MASK                       0x0000f000

#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x00000018
#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_FRAG_NUMBER_LSB                    16
#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_FRAG_NUMBER_MSB                    19
#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x000f0000

#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x00000018
#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               20
#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               31
#define OFDMA_TRIGGER_DETAILS_NR0_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0xfff00000

#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_RESERVED_OFFSET                      0x0000001c
#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_RESERVED_LSB                         0
#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_RESERVED_MSB                         11
#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_RESERVED_MASK                        0x00000fff

#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_TID_VALUE_OFFSET                     0x0000001c
#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_TID_VALUE_LSB                        12
#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_TID_VALUE_MSB                        15
#define OFDMA_TRIGGER_DETAILS_NR1_PER_TID_INFO_TID_VALUE_MASK                       0x0000f000

#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x0000001c
#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_FRAG_NUMBER_LSB                    16
#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_FRAG_NUMBER_MSB                    19
#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x000f0000

#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x0000001c
#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               20
#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               31
#define OFDMA_TRIGGER_DETAILS_NR1_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0xfff00000

#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_RESERVED_OFFSET                      0x00000020
#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_RESERVED_LSB                         0
#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_RESERVED_MSB                         11
#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_RESERVED_MASK                        0x00000fff

#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_TID_VALUE_OFFSET                     0x00000020
#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_TID_VALUE_LSB                        12
#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_TID_VALUE_MSB                        15
#define OFDMA_TRIGGER_DETAILS_NR2_PER_TID_INFO_TID_VALUE_MASK                       0x0000f000

#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x00000020
#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_FRAG_NUMBER_LSB                    16
#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_FRAG_NUMBER_MSB                    19
#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x000f0000

#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x00000020
#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               20
#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               31
#define OFDMA_TRIGGER_DETAILS_NR2_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0xfff00000

#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_RESERVED_OFFSET                      0x00000024
#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_RESERVED_LSB                         0
#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_RESERVED_MSB                         11
#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_RESERVED_MASK                        0x00000fff

#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_TID_VALUE_OFFSET                     0x00000024
#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_TID_VALUE_LSB                        12
#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_TID_VALUE_MSB                        15
#define OFDMA_TRIGGER_DETAILS_NR3_PER_TID_INFO_TID_VALUE_MASK                       0x0000f000

#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x00000024
#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_FRAG_NUMBER_LSB                    16
#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_FRAG_NUMBER_MSB                    19
#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x000f0000

#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x00000024
#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               20
#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               31
#define OFDMA_TRIGGER_DETAILS_NR3_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0xfff00000

#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_RESERVED_OFFSET                      0x00000028
#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_RESERVED_LSB                         0
#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_RESERVED_MSB                         11
#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_RESERVED_MASK                        0x00000fff

#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_TID_VALUE_OFFSET                     0x00000028
#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_TID_VALUE_LSB                        12
#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_TID_VALUE_MSB                        15
#define OFDMA_TRIGGER_DETAILS_NR4_PER_TID_INFO_TID_VALUE_MASK                       0x0000f000

#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x00000028
#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_FRAG_NUMBER_LSB                    16
#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_FRAG_NUMBER_MSB                    19
#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x000f0000

#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x00000028
#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               20
#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               31
#define OFDMA_TRIGGER_DETAILS_NR4_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0xfff00000

#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_RESERVED_OFFSET                      0x0000002c
#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_RESERVED_LSB                         0
#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_RESERVED_MSB                         11
#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_RESERVED_MASK                        0x00000fff

#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_TID_VALUE_OFFSET                     0x0000002c
#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_TID_VALUE_LSB                        12
#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_TID_VALUE_MSB                        15
#define OFDMA_TRIGGER_DETAILS_NR5_PER_TID_INFO_TID_VALUE_MASK                       0x0000f000

#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x0000002c
#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_FRAG_NUMBER_LSB                    16
#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_FRAG_NUMBER_MSB                    19
#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x000f0000

#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x0000002c
#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               20
#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               31
#define OFDMA_TRIGGER_DETAILS_NR5_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0xfff00000

#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_RESERVED_OFFSET                      0x00000030
#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_RESERVED_LSB                         0
#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_RESERVED_MSB                         11
#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_RESERVED_MASK                        0x00000fff

#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_TID_VALUE_OFFSET                     0x00000030
#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_TID_VALUE_LSB                        12
#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_TID_VALUE_MSB                        15
#define OFDMA_TRIGGER_DETAILS_NR6_PER_TID_INFO_TID_VALUE_MASK                       0x0000f000

#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x00000030
#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_FRAG_NUMBER_LSB                    16
#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_FRAG_NUMBER_MSB                    19
#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x000f0000

#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x00000030
#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               20
#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               31
#define OFDMA_TRIGGER_DETAILS_NR6_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0xfff00000

#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_RESERVED_OFFSET                      0x00000034
#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_RESERVED_LSB                         0
#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_RESERVED_MSB                         11
#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_RESERVED_MASK                        0x00000fff

#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_TID_VALUE_OFFSET                     0x00000034
#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_TID_VALUE_LSB                        12
#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_TID_VALUE_MSB                        15
#define OFDMA_TRIGGER_DETAILS_NR7_PER_TID_INFO_TID_VALUE_MASK                       0x0000f000

#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_FRAG_NUMBER_OFFSET                 0x00000034
#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_FRAG_NUMBER_LSB                    16
#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_FRAG_NUMBER_MSB                    19
#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_FRAG_NUMBER_MASK                   0x000f0000

#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_START_SEQ_NUMBER_OFFSET            0x00000034
#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_START_SEQ_NUMBER_LSB               20
#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_START_SEQ_NUMBER_MSB               31
#define OFDMA_TRIGGER_DETAILS_NR7_START_SEQ_CTRL_START_SEQ_NUMBER_MASK              0xfff00000

#define OFDMA_TRIGGER_DETAILS_FB_SEGMENT_RETRANSMISSION_BITMAP_OFFSET               0x00000038
#define OFDMA_TRIGGER_DETAILS_FB_SEGMENT_RETRANSMISSION_BITMAP_LSB                  0
#define OFDMA_TRIGGER_DETAILS_FB_SEGMENT_RETRANSMISSION_BITMAP_MSB                  7
#define OFDMA_TRIGGER_DETAILS_FB_SEGMENT_RETRANSMISSION_BITMAP_MASK                 0x000000ff

#define OFDMA_TRIGGER_DETAILS_RESERVED_14A_OFFSET                                   0x00000038
#define OFDMA_TRIGGER_DETAILS_RESERVED_14A_LSB                                      8
#define OFDMA_TRIGGER_DETAILS_RESERVED_14A_MSB                                      9
#define OFDMA_TRIGGER_DETAILS_RESERVED_14A_MASK                                     0x00000300

#define OFDMA_TRIGGER_DETAILS_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET                0x00000038
#define OFDMA_TRIGGER_DETAILS_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB                   10
#define OFDMA_TRIGGER_DETAILS_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB                   15
#define OFDMA_TRIGGER_DETAILS_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK                  0x0000fc00

#define OFDMA_TRIGGER_DETAILS_DOT11BE_PUNCTURE_BITMAP_OFFSET                        0x00000038
#define OFDMA_TRIGGER_DETAILS_DOT11BE_PUNCTURE_BITMAP_LSB                           16
#define OFDMA_TRIGGER_DETAILS_DOT11BE_PUNCTURE_BITMAP_MSB                           31
#define OFDMA_TRIGGER_DETAILS_DOT11BE_PUNCTURE_BITMAP_MASK                          0xffff0000

#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_OFFSET                                  0x0000003c
#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_LSB                                     0
#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_MSB                                     7
#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_MASK                                    0x000000ff

#define OFDMA_TRIGGER_DETAILS_RX_DURATION_FIELD_OFFSET                              0x0000003c
#define OFDMA_TRIGGER_DETAILS_RX_DURATION_FIELD_LSB                                 8
#define OFDMA_TRIGGER_DETAILS_RX_DURATION_FIELD_MSB                                 23
#define OFDMA_TRIGGER_DETAILS_RX_DURATION_FIELD_MASK                                0x00ffff00

#define OFDMA_TRIGGER_DETAILS_SCRAMBLER_SEED_OFFSET                                 0x0000003c
#define OFDMA_TRIGGER_DETAILS_SCRAMBLER_SEED_LSB                                    24
#define OFDMA_TRIGGER_DETAILS_SCRAMBLER_SEED_MSB                                    30
#define OFDMA_TRIGGER_DETAILS_SCRAMBLER_SEED_MASK                                   0x7f000000

#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_TYPE_OFFSET                             0x0000003c
#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_TYPE_LSB                                31
#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_TYPE_MSB                                31
#define OFDMA_TRIGGER_DETAILS_RX_CHAIN_MASK_TYPE_MASK                               0x80000000

#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET          0x00000040
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB             0
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB             9
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK            0x000003ff

#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET       0x00000040
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB          10
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB          10
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK         0x00000400

#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET    0x00000040
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB       11
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB       11
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK      0x00000800

#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET    0x00000040
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB       12
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB       12
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK      0x00001000

#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET              0x00000040
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB                 13
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB                 15
#define OFDMA_TRIGGER_DETAILS_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK                0x0000e000

#define OFDMA_TRIGGER_DETAILS_NORMALIZED_PRE_RSSI_COMB_OFFSET                       0x00000040
#define OFDMA_TRIGGER_DETAILS_NORMALIZED_PRE_RSSI_COMB_LSB                          16
#define OFDMA_TRIGGER_DETAILS_NORMALIZED_PRE_RSSI_COMB_MSB                          23
#define OFDMA_TRIGGER_DETAILS_NORMALIZED_PRE_RSSI_COMB_MASK                         0x00ff0000

#define OFDMA_TRIGGER_DETAILS_NORMALIZED_RSSI_COMB_OFFSET                           0x00000040
#define OFDMA_TRIGGER_DETAILS_NORMALIZED_RSSI_COMB_LSB                              24
#define OFDMA_TRIGGER_DETAILS_NORMALIZED_RSSI_COMB_MSB                              31
#define OFDMA_TRIGGER_DETAILS_NORMALIZED_RSSI_COMB_MASK                             0xff000000

#define OFDMA_TRIGGER_DETAILS_SW_PEER_ID_OFFSET                                     0x00000044
#define OFDMA_TRIGGER_DETAILS_SW_PEER_ID_LSB                                        0
#define OFDMA_TRIGGER_DETAILS_SW_PEER_ID_MSB                                        15
#define OFDMA_TRIGGER_DETAILS_SW_PEER_ID_MASK                                       0x0000ffff

#define OFDMA_TRIGGER_DETAILS_RESPONSE_TX_DURATION_OFFSET                           0x00000044
#define OFDMA_TRIGGER_DETAILS_RESPONSE_TX_DURATION_LSB                              16
#define OFDMA_TRIGGER_DETAILS_RESPONSE_TX_DURATION_MSB                              31
#define OFDMA_TRIGGER_DETAILS_RESPONSE_TX_DURATION_MASK                             0xffff0000

#define OFDMA_TRIGGER_DETAILS_RANGING_TRIGGER_SUBTYPE_OFFSET                        0x00000048
#define OFDMA_TRIGGER_DETAILS_RANGING_TRIGGER_SUBTYPE_LSB                           0
#define OFDMA_TRIGGER_DETAILS_RANGING_TRIGGER_SUBTYPE_MSB                           3
#define OFDMA_TRIGGER_DETAILS_RANGING_TRIGGER_SUBTYPE_MASK                          0x0000000f

#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_COMMON_INFO_79_68_OFFSET                  0x00000048
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_COMMON_INFO_79_68_LSB                     4
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_COMMON_INFO_79_68_MSB                     15
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_COMMON_INFO_79_68_MASK                    0x0000fff0

#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_20_12_OFFSET               0x00000048
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_20_12_LSB                  16
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_20_12_MSB                  24
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_20_12_MASK                 0x01ff0000

#define OFDMA_TRIGGER_DETAILS_I2R_REP_OFFSET                                        0x00000048
#define OFDMA_TRIGGER_DETAILS_I2R_REP_LSB                                           25
#define OFDMA_TRIGGER_DETAILS_I2R_REP_MSB                                           27
#define OFDMA_TRIGGER_DETAILS_I2R_REP_MASK                                          0x0e000000

#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_25_24_OFFSET               0x00000048
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_25_24_LSB                  28
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_25_24_MSB                  29
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_RESERVED_25_24_MASK                 0x30000000

#define OFDMA_TRIGGER_DETAILS_RESERVED_18A_OFFSET                                   0x00000048
#define OFDMA_TRIGGER_DETAILS_RESERVED_18A_LSB                                      30
#define OFDMA_TRIGGER_DETAILS_RESERVED_18A_MSB                                      30
#define OFDMA_TRIGGER_DETAILS_RESERVED_18A_MASK                                     0x40000000

#define OFDMA_TRIGGER_DETAILS_QOS_NULL_ONLY_RESPONSE_TX_OFFSET                      0x00000048
#define OFDMA_TRIGGER_DETAILS_QOS_NULL_ONLY_RESPONSE_TX_LSB                         31
#define OFDMA_TRIGGER_DETAILS_QOS_NULL_ONLY_RESPONSE_TX_MSB                         31
#define OFDMA_TRIGGER_DETAILS_QOS_NULL_ONLY_RESPONSE_TX_MASK                        0x80000000

#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_SAC_OFFSET                          0x0000004c
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_SAC_LSB                             0
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_SAC_MSB                             15
#define OFDMA_TRIGGER_DETAILS_TBR_TRIGGER_SOUND_SAC_MASK                            0x0000ffff

#define OFDMA_TRIGGER_DETAILS_RESERVED_19A_OFFSET                                   0x0000004c
#define OFDMA_TRIGGER_DETAILS_RESERVED_19A_LSB                                      16
#define OFDMA_TRIGGER_DETAILS_RESERVED_19A_MSB                                      23
#define OFDMA_TRIGGER_DETAILS_RESERVED_19A_MASK                                     0x00ff0000

#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED2_OFFSET                                0x0000004c
#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED2_LSB                                   24
#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED2_MSB                                   28
#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED2_MASK                                  0x1f000000

#define OFDMA_TRIGGER_DETAILS_RESERVED_19B_OFFSET                                   0x0000004c
#define OFDMA_TRIGGER_DETAILS_RESERVED_19B_LSB                                      29
#define OFDMA_TRIGGER_DETAILS_RESERVED_19B_MSB                                      31
#define OFDMA_TRIGGER_DETAILS_RESERVED_19B_MASK                                     0xe0000000

#define OFDMA_TRIGGER_DETAILS_EHT_SPECIAL_AID12_OFFSET                              0x00000050
#define OFDMA_TRIGGER_DETAILS_EHT_SPECIAL_AID12_LSB                                 0
#define OFDMA_TRIGGER_DETAILS_EHT_SPECIAL_AID12_MSB                                 11
#define OFDMA_TRIGGER_DETAILS_EHT_SPECIAL_AID12_MASK                                0x00000fff

#define OFDMA_TRIGGER_DETAILS_PHY_VERSION_OFFSET                                    0x00000050
#define OFDMA_TRIGGER_DETAILS_PHY_VERSION_LSB                                       12
#define OFDMA_TRIGGER_DETAILS_PHY_VERSION_MSB                                       14
#define OFDMA_TRIGGER_DETAILS_PHY_VERSION_MASK                                      0x00007000

#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_EXT_OFFSET                                  0x00000050
#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_EXT_LSB                                     15
#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_EXT_MSB                                     16
#define OFDMA_TRIGGER_DETAILS_BANDWIDTH_EXT_MASK                                    0x00018000

#define OFDMA_TRIGGER_DETAILS_EHT_SPATIAL_REUSE_OFFSET                              0x00000050
#define OFDMA_TRIGGER_DETAILS_EHT_SPATIAL_REUSE_LSB                                 17
#define OFDMA_TRIGGER_DETAILS_EHT_SPATIAL_REUSE_MSB                                 24
#define OFDMA_TRIGGER_DETAILS_EHT_SPATIAL_REUSE_MASK                                0x01fe0000

#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED1_OFFSET                                0x00000050
#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED1_LSB                                   25
#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED1_MSB                                   31
#define OFDMA_TRIGGER_DETAILS_U_SIG_RESERVED1_MASK                                  0xfe000000

#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_SPECIAL_USER_INFO_71_40_OFFSET            0x00000054
#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_SPECIAL_USER_INFO_71_40_LSB               0
#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_SPECIAL_USER_INFO_71_40_MSB               31
#define OFDMA_TRIGGER_DETAILS_EHT_TRIGGER_SPECIAL_USER_INFO_71_40_MASK              0xffffffff

#endif
