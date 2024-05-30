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


#ifndef _RX_RESPONSE_REQUIRED_INFO_H_
#define _RX_RESPONSE_REQUIRED_INFO_H_

#include "mlo_sta_id_details.h"
#define NUM_OF_DWORDS_RX_RESPONSE_REQUIRED_INFO 15

struct rx_response_required_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t phy_ppdu_id                                             : 16,
                      su_or_uplink_mu_reception                               :  1,
                      trigger_frame_received                                  :  1,
                      __reserved_g_0012                                                  :  2,
                      tb___reserved_g_0005_response_required                            :  2,
                      mac_security                                            :  1,
                      filter_pass_monitor_ovrd                                :  1,
                      ast_search_incomplete                                   :  1,
                      r2r_end_status_to_follow                                :  1,
                      __reserved_g_0016_listen_cca_check_at_phy_desc                        :  1,
                      __reserved_g_0016_listen_indication                                   :  1,
                      three_or_more_type_subtypes                             :  1,
                      wait_sifs_config_valid                                  :  1,
                      wait_sifs                                               :  2;
             uint32_t general_frame_control                                   : 16,
                      second_frame_control                                    : 16;
             uint32_t duration                                                : 16,
                      pkt_type                                                :  4,
                      dot11ax_su_extended                                     :  1,
                      rate_mcs                                                :  4,
                      sgi                                                     :  2,
                      stbc                                                    :  1,
                      ldpc                                                    :  1,
                      ampdu                                                   :  1,
                      vht_ack                                                 :  1,
                      rts_ta_grp_bit                                          :  1;
             uint32_t ctrl_frame_soliciting_resp                              :  1,
                      ast_fail_for_dot11ax_su_ext                             :  1,
                      service_dynamic                                         :  1,
                      m_pkt                                                   :  1,
                      sta_partial_aid                                         : 12,
                      group_id                                                :  6,
                      ctrl_resp_pwr_mgmt                                      :  1,
                      response_indication                                     :  2,
                      ndp_indication                                          :  1,
                      ndp_frame_type                                          :  3,
                      second_frame_control_valid                              :  1,
                      ack_ba_resp_more_data                                   :  1,
                      reserved_3a                                             :  1;
             uint32_t ack_id                                                  : 16,
                      ack_id_ext                                              : 10,
                      agc_cbw                                                 :  3,
                      service_cbw                                             :  3;
             uint32_t response_sta_count                                      :  7,
                      reserved                                                :  4,
                      ht_vht_sig_cbw                                          :  3,
                      cts_cbw                                                 :  3,
                      response_ack_count                                      :  7,
                      response_assoc_ack_count                                :  7,
                      txop_duration_all_ones                                  :  1;
             uint32_t response_ba32_count                                     :  7,
                      response_ba64_count                                     :  7,
                      response_ba128_count                                    :  7,
                      response_ba256_count                                    :  7,
                      multi_tid                                               :  1,
                      sw_response_tlv_from_crypto                             :  1,
                      dot11ax_dl_ul_flag                                      :  1,
                      emlsr_main_tlv_if                                       :  1;
             uint32_t sw_response_frame_length                                : 16,
                      response_ba512_count                                    :  7,
                      response_ba1024_count                                   :  7,
                      reserved_7a                                             :  2;
             uint32_t addr1_31_0                                              : 32;
             uint32_t addr1_47_32                                             : 16,
                      addr2_15_0                                              : 16;
             uint32_t addr2_47_16                                             : 32;
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
                      ftm_fields_valid                                        :  1,
                      ftm_pe_nss                                              :  3,
                      ftm_pe_ltf_size                                         :  2,
                      ftm_pe_content                                          :  1,
                      ftm_chain_csd_en                                        :  1,
                      ftm_pe_chain_csd_en                                     :  1;
             uint32_t dot11ax_response_rate_source                            :  8,
                      dot11ax_ext_response_rate_source                        :  8,
                      sw_peer_id                                              : 16;
             uint32_t dot11be_puncture_bitmap                                 : 16,
                      dot11be_response                                        :  1,
                      punctured_response                                      :  1,
                      eht_duplicate_mode                                      :  2,
                      force_extra_symbol                                      :  1,
                      reserved_13a                                            :  5,
                      u_sig_puncture_pattern_encoding                         :  6;
             struct   mlo_sta_id_details                                        mlo_sta_id_details_rx;
             uint16_t he_a_control_response_time                              : 12,
                      reserved_after_struct16                                 :  4;
#else
             uint32_t wait_sifs                                               :  2,
                      wait_sifs_config_valid                                  :  1,
                      three_or_more_type_subtypes                             :  1,
                      __reserved_g_0016_listen_indication                                   :  1,
                      __reserved_g_0016_listen_cca_check_at_phy_desc                        :  1,
                      r2r_end_status_to_follow                                :  1,
                      ast_search_incomplete                                   :  1,
                      filter_pass_monitor_ovrd                                :  1,
                      mac_security                                            :  1,
                      tb___reserved_g_0005_response_required                            :  2,
                      __reserved_g_0012                                                  :  2,
                      trigger_frame_received                                  :  1,
                      su_or_uplink_mu_reception                               :  1,
                      phy_ppdu_id                                             : 16;
             uint32_t second_frame_control                                    : 16,
                      general_frame_control                                   : 16;
             uint32_t rts_ta_grp_bit                                          :  1,
                      vht_ack                                                 :  1,
                      ampdu                                                   :  1,
                      ldpc                                                    :  1,
                      stbc                                                    :  1,
                      sgi                                                     :  2,
                      rate_mcs                                                :  4,
                      dot11ax_su_extended                                     :  1,
                      pkt_type                                                :  4,
                      duration                                                : 16;
             uint32_t reserved_3a                                             :  1,
                      ack_ba_resp_more_data                                   :  1,
                      second_frame_control_valid                              :  1,
                      ndp_frame_type                                          :  3,
                      ndp_indication                                          :  1,
                      response_indication                                     :  2,
                      ctrl_resp_pwr_mgmt                                      :  1,
                      group_id                                                :  6,
                      sta_partial_aid                                         : 12,
                      m_pkt                                                   :  1,
                      service_dynamic                                         :  1,
                      ast_fail_for_dot11ax_su_ext                             :  1,
                      ctrl_frame_soliciting_resp                              :  1;
             uint32_t service_cbw                                             :  3,
                      agc_cbw                                                 :  3,
                      ack_id_ext                                              : 10,
                      ack_id                                                  : 16;
             uint32_t txop_duration_all_ones                                  :  1,
                      response_assoc_ack_count                                :  7,
                      response_ack_count                                      :  7,
                      cts_cbw                                                 :  3,
                      ht_vht_sig_cbw                                          :  3,
                      reserved                                                :  4,
                      response_sta_count                                      :  7;
             uint32_t emlsr_main_tlv_if                                       :  1,
                      dot11ax_dl_ul_flag                                      :  1,
                      sw_response_tlv_from_crypto                             :  1,
                      multi_tid                                               :  1,
                      response_ba256_count                                    :  7,
                      response_ba128_count                                    :  7,
                      response_ba64_count                                     :  7,
                      response_ba32_count                                     :  7;
             uint32_t reserved_7a                                             :  2,
                      response_ba1024_count                                   :  7,
                      response_ba512_count                                    :  7,
                      sw_response_frame_length                                : 16;
             uint32_t addr1_31_0                                              : 32;
             uint32_t addr2_15_0                                              : 16,
                      addr1_47_32                                             : 16;
             uint32_t addr2_47_16                                             : 32;
             uint32_t ftm_pe_chain_csd_en                                     :  1,
                      ftm_chain_csd_en                                        :  1,
                      ftm_pe_content                                          :  1,
                      ftm_pe_ltf_size                                         :  2,
                      ftm_pe_nss                                              :  3,
                      ftm_fields_valid                                        :  1,
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
             uint32_t sw_peer_id                                              : 16,
                      dot11ax_ext_response_rate_source                        :  8,
                      dot11ax_response_rate_source                            :  8;
             uint32_t u_sig_puncture_pattern_encoding                         :  6,
                      reserved_13a                                            :  5,
                      force_extra_symbol                                      :  1,
                      eht_duplicate_mode                                      :  2,
                      punctured_response                                      :  1,
                      dot11be_response                                        :  1,
                      dot11be_puncture_bitmap                                 : 16;
             uint32_t reserved_after_struct16                                 :  4,
                      he_a_control_response_time                              : 12;
             struct   mlo_sta_id_details                                        mlo_sta_id_details_rx;
#endif
};

#define RX_RESPONSE_REQUIRED_INFO_PHY_PPDU_ID_OFFSET                                0x00000000
#define RX_RESPONSE_REQUIRED_INFO_PHY_PPDU_ID_LSB                                   0
#define RX_RESPONSE_REQUIRED_INFO_PHY_PPDU_ID_MSB                                   15
#define RX_RESPONSE_REQUIRED_INFO_PHY_PPDU_ID_MASK                                  0x0000ffff

#define RX_RESPONSE_REQUIRED_INFO_SU_OR_UPLINK_MU_RECEPTION_OFFSET                  0x00000000
#define RX_RESPONSE_REQUIRED_INFO_SU_OR_UPLINK_MU_RECEPTION_LSB                     16
#define RX_RESPONSE_REQUIRED_INFO_SU_OR_UPLINK_MU_RECEPTION_MSB                     16
#define RX_RESPONSE_REQUIRED_INFO_SU_OR_UPLINK_MU_RECEPTION_MASK                    0x00010000

#define RX_RESPONSE_REQUIRED_INFO_TRIGGER_FRAME_RECEIVED_OFFSET                     0x00000000
#define RX_RESPONSE_REQUIRED_INFO_TRIGGER_FRAME_RECEIVED_LSB                        17
#define RX_RESPONSE_REQUIRED_INFO_TRIGGER_FRAME_RECEIVED_MSB                        17
#define RX_RESPONSE_REQUIRED_INFO_TRIGGER_FRAME_RECEIVED_MASK                       0x00020000

#define RX_RESPONSE_REQUIRED_INFO_TB_RANGING_RESPONSE_REQUIRED_OFFSET               0x00000000
#define RX_RESPONSE_REQUIRED_INFO_TB_RANGING_RESPONSE_REQUIRED_LSB                  20
#define RX_RESPONSE_REQUIRED_INFO_TB_RANGING_RESPONSE_REQUIRED_MSB                  21
#define RX_RESPONSE_REQUIRED_INFO_TB_RANGING_RESPONSE_REQUIRED_MASK                 0x00300000

#define RX_RESPONSE_REQUIRED_INFO_MAC_SECURITY_OFFSET                               0x00000000
#define RX_RESPONSE_REQUIRED_INFO_MAC_SECURITY_LSB                                  22
#define RX_RESPONSE_REQUIRED_INFO_MAC_SECURITY_MSB                                  22
#define RX_RESPONSE_REQUIRED_INFO_MAC_SECURITY_MASK                                 0x00400000

#define RX_RESPONSE_REQUIRED_INFO_FILTER_PASS_MONITOR_OVRD_OFFSET                   0x00000000
#define RX_RESPONSE_REQUIRED_INFO_FILTER_PASS_MONITOR_OVRD_LSB                      23
#define RX_RESPONSE_REQUIRED_INFO_FILTER_PASS_MONITOR_OVRD_MSB                      23
#define RX_RESPONSE_REQUIRED_INFO_FILTER_PASS_MONITOR_OVRD_MASK                     0x00800000

#define RX_RESPONSE_REQUIRED_INFO_AST_SEARCH_INCOMPLETE_OFFSET                      0x00000000
#define RX_RESPONSE_REQUIRED_INFO_AST_SEARCH_INCOMPLETE_LSB                         24
#define RX_RESPONSE_REQUIRED_INFO_AST_SEARCH_INCOMPLETE_MSB                         24
#define RX_RESPONSE_REQUIRED_INFO_AST_SEARCH_INCOMPLETE_MASK                        0x01000000

#define RX_RESPONSE_REQUIRED_INFO_R2R_END_STATUS_TO_FOLLOW_OFFSET                   0x00000000
#define RX_RESPONSE_REQUIRED_INFO_R2R_END_STATUS_TO_FOLLOW_LSB                      25
#define RX_RESPONSE_REQUIRED_INFO_R2R_END_STATUS_TO_FOLLOW_MSB                      25
#define RX_RESPONSE_REQUIRED_INFO_R2R_END_STATUS_TO_FOLLOW_MASK                     0x02000000

#define RX_RESPONSE_REQUIRED_INFO_THREE_OR_MORE_TYPE_SUBTYPES_OFFSET                0x00000000
#define RX_RESPONSE_REQUIRED_INFO_THREE_OR_MORE_TYPE_SUBTYPES_LSB                   28
#define RX_RESPONSE_REQUIRED_INFO_THREE_OR_MORE_TYPE_SUBTYPES_MSB                   28
#define RX_RESPONSE_REQUIRED_INFO_THREE_OR_MORE_TYPE_SUBTYPES_MASK                  0x10000000

#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_CONFIG_VALID_OFFSET                     0x00000000
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_CONFIG_VALID_LSB                        29
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_CONFIG_VALID_MSB                        29
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_CONFIG_VALID_MASK                       0x20000000

#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_OFFSET                                  0x00000000
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_LSB                                     30
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_MSB                                     31
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_MASK                                    0xc0000000

#define RX_RESPONSE_REQUIRED_INFO_GENERAL_FRAME_CONTROL_OFFSET                      0x00000004
#define RX_RESPONSE_REQUIRED_INFO_GENERAL_FRAME_CONTROL_LSB                         0
#define RX_RESPONSE_REQUIRED_INFO_GENERAL_FRAME_CONTROL_MSB                         15
#define RX_RESPONSE_REQUIRED_INFO_GENERAL_FRAME_CONTROL_MASK                        0x0000ffff

#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_OFFSET                       0x00000004
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_LSB                          16
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_MSB                          31
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_MASK                         0xffff0000

#define RX_RESPONSE_REQUIRED_INFO_DURATION_OFFSET                                   0x00000008
#define RX_RESPONSE_REQUIRED_INFO_DURATION_LSB                                      0
#define RX_RESPONSE_REQUIRED_INFO_DURATION_MSB                                      15
#define RX_RESPONSE_REQUIRED_INFO_DURATION_MASK                                     0x0000ffff

#define RX_RESPONSE_REQUIRED_INFO_PKT_TYPE_OFFSET                                   0x00000008
#define RX_RESPONSE_REQUIRED_INFO_PKT_TYPE_LSB                                      16
#define RX_RESPONSE_REQUIRED_INFO_PKT_TYPE_MSB                                      19
#define RX_RESPONSE_REQUIRED_INFO_PKT_TYPE_MASK                                     0x000f0000

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_SU_EXTENDED_OFFSET                        0x00000008
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_SU_EXTENDED_LSB                           20
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_SU_EXTENDED_MSB                           20
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_SU_EXTENDED_MASK                          0x00100000

#define RX_RESPONSE_REQUIRED_INFO_RATE_MCS_OFFSET                                   0x00000008
#define RX_RESPONSE_REQUIRED_INFO_RATE_MCS_LSB                                      21
#define RX_RESPONSE_REQUIRED_INFO_RATE_MCS_MSB                                      24
#define RX_RESPONSE_REQUIRED_INFO_RATE_MCS_MASK                                     0x01e00000

#define RX_RESPONSE_REQUIRED_INFO_SGI_OFFSET                                        0x00000008
#define RX_RESPONSE_REQUIRED_INFO_SGI_LSB                                           25
#define RX_RESPONSE_REQUIRED_INFO_SGI_MSB                                           26
#define RX_RESPONSE_REQUIRED_INFO_SGI_MASK                                          0x06000000

#define RX_RESPONSE_REQUIRED_INFO_STBC_OFFSET                                       0x00000008
#define RX_RESPONSE_REQUIRED_INFO_STBC_LSB                                          27
#define RX_RESPONSE_REQUIRED_INFO_STBC_MSB                                          27
#define RX_RESPONSE_REQUIRED_INFO_STBC_MASK                                         0x08000000

#define RX_RESPONSE_REQUIRED_INFO_LDPC_OFFSET                                       0x00000008
#define RX_RESPONSE_REQUIRED_INFO_LDPC_LSB                                          28
#define RX_RESPONSE_REQUIRED_INFO_LDPC_MSB                                          28
#define RX_RESPONSE_REQUIRED_INFO_LDPC_MASK                                         0x10000000

#define RX_RESPONSE_REQUIRED_INFO_AMPDU_OFFSET                                      0x00000008
#define RX_RESPONSE_REQUIRED_INFO_AMPDU_LSB                                         29
#define RX_RESPONSE_REQUIRED_INFO_AMPDU_MSB                                         29
#define RX_RESPONSE_REQUIRED_INFO_AMPDU_MASK                                        0x20000000

#define RX_RESPONSE_REQUIRED_INFO_VHT_ACK_OFFSET                                    0x00000008
#define RX_RESPONSE_REQUIRED_INFO_VHT_ACK_LSB                                       30
#define RX_RESPONSE_REQUIRED_INFO_VHT_ACK_MSB                                       30
#define RX_RESPONSE_REQUIRED_INFO_VHT_ACK_MASK                                      0x40000000

#define RX_RESPONSE_REQUIRED_INFO_RTS_TA_GRP_BIT_OFFSET                             0x00000008
#define RX_RESPONSE_REQUIRED_INFO_RTS_TA_GRP_BIT_LSB                                31
#define RX_RESPONSE_REQUIRED_INFO_RTS_TA_GRP_BIT_MSB                                31
#define RX_RESPONSE_REQUIRED_INFO_RTS_TA_GRP_BIT_MASK                               0x80000000

#define RX_RESPONSE_REQUIRED_INFO_CTRL_FRAME_SOLICITING_RESP_OFFSET                 0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_CTRL_FRAME_SOLICITING_RESP_LSB                    0
#define RX_RESPONSE_REQUIRED_INFO_CTRL_FRAME_SOLICITING_RESP_MSB                    0
#define RX_RESPONSE_REQUIRED_INFO_CTRL_FRAME_SOLICITING_RESP_MASK                   0x00000001

#define RX_RESPONSE_REQUIRED_INFO_AST_FAIL_FOR_DOT11AX_SU_EXT_OFFSET                0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_AST_FAIL_FOR_DOT11AX_SU_EXT_LSB                   1
#define RX_RESPONSE_REQUIRED_INFO_AST_FAIL_FOR_DOT11AX_SU_EXT_MSB                   1
#define RX_RESPONSE_REQUIRED_INFO_AST_FAIL_FOR_DOT11AX_SU_EXT_MASK                  0x00000002

#define RX_RESPONSE_REQUIRED_INFO_SERVICE_DYNAMIC_OFFSET                            0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_DYNAMIC_LSB                               2
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_DYNAMIC_MSB                               2
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_DYNAMIC_MASK                              0x00000004

#define RX_RESPONSE_REQUIRED_INFO_M_PKT_OFFSET                                      0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_M_PKT_LSB                                         3
#define RX_RESPONSE_REQUIRED_INFO_M_PKT_MSB                                         3
#define RX_RESPONSE_REQUIRED_INFO_M_PKT_MASK                                        0x00000008

#define RX_RESPONSE_REQUIRED_INFO_STA_PARTIAL_AID_OFFSET                            0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_STA_PARTIAL_AID_LSB                               4
#define RX_RESPONSE_REQUIRED_INFO_STA_PARTIAL_AID_MSB                               15
#define RX_RESPONSE_REQUIRED_INFO_STA_PARTIAL_AID_MASK                              0x0000fff0

#define RX_RESPONSE_REQUIRED_INFO_GROUP_ID_OFFSET                                   0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_GROUP_ID_LSB                                      16
#define RX_RESPONSE_REQUIRED_INFO_GROUP_ID_MSB                                      21
#define RX_RESPONSE_REQUIRED_INFO_GROUP_ID_MASK                                     0x003f0000

#define RX_RESPONSE_REQUIRED_INFO_CTRL_RESP_PWR_MGMT_OFFSET                         0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_CTRL_RESP_PWR_MGMT_LSB                            22
#define RX_RESPONSE_REQUIRED_INFO_CTRL_RESP_PWR_MGMT_MSB                            22
#define RX_RESPONSE_REQUIRED_INFO_CTRL_RESP_PWR_MGMT_MASK                           0x00400000

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_INDICATION_OFFSET                        0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_INDICATION_LSB                           23
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_INDICATION_MSB                           24
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_INDICATION_MASK                          0x01800000

#define RX_RESPONSE_REQUIRED_INFO_NDP_INDICATION_OFFSET                             0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_NDP_INDICATION_LSB                                25
#define RX_RESPONSE_REQUIRED_INFO_NDP_INDICATION_MSB                                25
#define RX_RESPONSE_REQUIRED_INFO_NDP_INDICATION_MASK                               0x02000000

#define RX_RESPONSE_REQUIRED_INFO_NDP_FRAME_TYPE_OFFSET                             0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_NDP_FRAME_TYPE_LSB                                26
#define RX_RESPONSE_REQUIRED_INFO_NDP_FRAME_TYPE_MSB                                28
#define RX_RESPONSE_REQUIRED_INFO_NDP_FRAME_TYPE_MASK                               0x1c000000

#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_VALID_OFFSET                 0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_VALID_LSB                    29
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_VALID_MSB                    29
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_VALID_MASK                   0x20000000

#define RX_RESPONSE_REQUIRED_INFO_ACK_BA_RESP_MORE_DATA_OFFSET                      0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_ACK_BA_RESP_MORE_DATA_LSB                         30
#define RX_RESPONSE_REQUIRED_INFO_ACK_BA_RESP_MORE_DATA_MSB                         30
#define RX_RESPONSE_REQUIRED_INFO_ACK_BA_RESP_MORE_DATA_MASK                        0x40000000

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_3A_OFFSET                                0x0000000c
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_3A_LSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_3A_MSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_3A_MASK                                  0x80000000

#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_OFFSET                                     0x00000010
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_LSB                                        0
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_MSB                                        15
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_MASK                                       0x0000ffff

#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_EXT_OFFSET                                 0x00000010
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_EXT_LSB                                    16
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_EXT_MSB                                    25
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_EXT_MASK                                   0x03ff0000

#define RX_RESPONSE_REQUIRED_INFO_AGC_CBW_OFFSET                                    0x00000010
#define RX_RESPONSE_REQUIRED_INFO_AGC_CBW_LSB                                       26
#define RX_RESPONSE_REQUIRED_INFO_AGC_CBW_MSB                                       28
#define RX_RESPONSE_REQUIRED_INFO_AGC_CBW_MASK                                      0x1c000000

#define RX_RESPONSE_REQUIRED_INFO_SERVICE_CBW_OFFSET                                0x00000010
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_CBW_LSB                                   29
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_CBW_MSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_CBW_MASK                                  0xe0000000

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_STA_COUNT_OFFSET                         0x00000014
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_STA_COUNT_LSB                            0
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_STA_COUNT_MSB                            6
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_STA_COUNT_MASK                           0x0000007f

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_OFFSET                                   0x00000014
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_LSB                                      7
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_MSB                                      10
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_MASK                                     0x00000780

#define RX_RESPONSE_REQUIRED_INFO_HT_VHT_SIG_CBW_OFFSET                             0x00000014
#define RX_RESPONSE_REQUIRED_INFO_HT_VHT_SIG_CBW_LSB                                11
#define RX_RESPONSE_REQUIRED_INFO_HT_VHT_SIG_CBW_MSB                                13
#define RX_RESPONSE_REQUIRED_INFO_HT_VHT_SIG_CBW_MASK                               0x00003800

#define RX_RESPONSE_REQUIRED_INFO_CTS_CBW_OFFSET                                    0x00000014
#define RX_RESPONSE_REQUIRED_INFO_CTS_CBW_LSB                                       14
#define RX_RESPONSE_REQUIRED_INFO_CTS_CBW_MSB                                       16
#define RX_RESPONSE_REQUIRED_INFO_CTS_CBW_MASK                                      0x0001c000

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ACK_COUNT_OFFSET                         0x00000014
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ACK_COUNT_LSB                            17
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ACK_COUNT_MSB                            23
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ACK_COUNT_MASK                           0x00fe0000

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ASSOC_ACK_COUNT_OFFSET                   0x00000014
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ASSOC_ACK_COUNT_LSB                      24
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ASSOC_ACK_COUNT_MSB                      30
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ASSOC_ACK_COUNT_MASK                     0x7f000000

#define RX_RESPONSE_REQUIRED_INFO_TXOP_DURATION_ALL_ONES_OFFSET                     0x00000014
#define RX_RESPONSE_REQUIRED_INFO_TXOP_DURATION_ALL_ONES_LSB                        31
#define RX_RESPONSE_REQUIRED_INFO_TXOP_DURATION_ALL_ONES_MSB                        31
#define RX_RESPONSE_REQUIRED_INFO_TXOP_DURATION_ALL_ONES_MASK                       0x80000000

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA32_COUNT_OFFSET                        0x00000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA32_COUNT_LSB                           0
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA32_COUNT_MSB                           6
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA32_COUNT_MASK                          0x0000007f

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA64_COUNT_OFFSET                        0x00000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA64_COUNT_LSB                           7
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA64_COUNT_MSB                           13
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA64_COUNT_MASK                          0x00003f80

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA128_COUNT_OFFSET                       0x00000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA128_COUNT_LSB                          14
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA128_COUNT_MSB                          20
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA128_COUNT_MASK                         0x001fc000

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA256_COUNT_OFFSET                       0x00000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA256_COUNT_LSB                          21
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA256_COUNT_MSB                          27
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA256_COUNT_MASK                         0x0fe00000

#define RX_RESPONSE_REQUIRED_INFO_MULTI_TID_OFFSET                                  0x00000018
#define RX_RESPONSE_REQUIRED_INFO_MULTI_TID_LSB                                     28
#define RX_RESPONSE_REQUIRED_INFO_MULTI_TID_MSB                                     28
#define RX_RESPONSE_REQUIRED_INFO_MULTI_TID_MASK                                    0x10000000

#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_OFFSET                0x00000018
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_LSB                   29
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_MSB                   29
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_MASK                  0x20000000

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_DL_UL_FLAG_OFFSET                         0x00000018
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_DL_UL_FLAG_LSB                            30
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_DL_UL_FLAG_MSB                            30
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_DL_UL_FLAG_MASK                           0x40000000

#define RX_RESPONSE_REQUIRED_INFO_EMLSR_MAIN_TLV_IF_OFFSET                          0x00000018
#define RX_RESPONSE_REQUIRED_INFO_EMLSR_MAIN_TLV_IF_LSB                             31
#define RX_RESPONSE_REQUIRED_INFO_EMLSR_MAIN_TLV_IF_MSB                             31
#define RX_RESPONSE_REQUIRED_INFO_EMLSR_MAIN_TLV_IF_MASK                            0x80000000

#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_FRAME_LENGTH_OFFSET                   0x0000001c
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_FRAME_LENGTH_LSB                      0
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_FRAME_LENGTH_MSB                      15
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_FRAME_LENGTH_MASK                     0x0000ffff

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA512_COUNT_OFFSET                       0x0000001c
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA512_COUNT_LSB                          16
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA512_COUNT_MSB                          22
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA512_COUNT_MASK                         0x007f0000

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA1024_COUNT_OFFSET                      0x0000001c
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA1024_COUNT_LSB                         23
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA1024_COUNT_MSB                         29
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA1024_COUNT_MASK                        0x3f800000

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_7A_OFFSET                                0x0000001c
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_7A_LSB                                   30
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_7A_MSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_7A_MASK                                  0xc0000000

#define RX_RESPONSE_REQUIRED_INFO_ADDR1_31_0_OFFSET                                 0x00000020
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_31_0_LSB                                    0
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_31_0_MSB                                    31
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_31_0_MASK                                   0xffffffff

#define RX_RESPONSE_REQUIRED_INFO_ADDR1_47_32_OFFSET                                0x00000024
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_47_32_LSB                                   0
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_47_32_MSB                                   15
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_47_32_MASK                                  0x0000ffff

#define RX_RESPONSE_REQUIRED_INFO_ADDR2_15_0_OFFSET                                 0x00000024
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_15_0_LSB                                    16
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_15_0_MSB                                    31
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_15_0_MASK                                   0xffff0000

#define RX_RESPONSE_REQUIRED_INFO_ADDR2_47_16_OFFSET                                0x00000028
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_47_16_LSB                                   0
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_47_16_MSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_47_16_MASK                                  0xffffffff

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_OFFSET         0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_LSB            0
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_MSB            0
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_MASK           0x00000001

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_OFFSET                0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_LSB                   1
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_MSB                   1
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_MASK                  0x00000002

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_OFFSET              0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_LSB                 2
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_MSB                 7
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_MASK                0x000000fc

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_OFFSET             0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_LSB                8
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_MSB                11
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_MASK               0x00000f00

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CP_SIZE_OFFSET                   0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CP_SIZE_LSB                      12
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CP_SIZE_MSB                      13
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CP_SIZE_MASK                     0x00003000

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_LTF_SIZE_OFFSET                  0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_LTF_SIZE_LSB                     14
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_LTF_SIZE_MSB                     15
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_LTF_SIZE_MASK                    0x0000c000

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CODING_OFFSET                    0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CODING_LSB                       16
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CODING_MSB                       16
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CODING_MASK                      0x00010000

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DCM_OFFSET                       0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DCM_LSB                          17
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DCM_MSB                          17
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DCM_MASK                         0x00020000

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_OFFSET        0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_LSB           18
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_MSB           18
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_MASK          0x00040000

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_OFFSET               0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_LSB                  19
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_MSB                  22
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_MASK                 0x00780000

#define RX_RESPONSE_REQUIRED_INFO_FTM_FIELDS_VALID_OFFSET                           0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_FTM_FIELDS_VALID_LSB                              23
#define RX_RESPONSE_REQUIRED_INFO_FTM_FIELDS_VALID_MSB                              23
#define RX_RESPONSE_REQUIRED_INFO_FTM_FIELDS_VALID_MASK                             0x00800000

#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_NSS_OFFSET                                 0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_NSS_LSB                                    24
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_NSS_MSB                                    26
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_NSS_MASK                                   0x07000000

#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_LTF_SIZE_OFFSET                            0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_LTF_SIZE_LSB                               27
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_LTF_SIZE_MSB                               28
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_LTF_SIZE_MASK                              0x18000000

#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CONTENT_OFFSET                             0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CONTENT_LSB                                29
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CONTENT_MSB                                29
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CONTENT_MASK                               0x20000000

#define RX_RESPONSE_REQUIRED_INFO_FTM_CHAIN_CSD_EN_OFFSET                           0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_FTM_CHAIN_CSD_EN_LSB                              30
#define RX_RESPONSE_REQUIRED_INFO_FTM_CHAIN_CSD_EN_MSB                              30
#define RX_RESPONSE_REQUIRED_INFO_FTM_CHAIN_CSD_EN_MASK                             0x40000000

#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CHAIN_CSD_EN_OFFSET                        0x0000002c
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CHAIN_CSD_EN_LSB                           31
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CHAIN_CSD_EN_MSB                           31
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CHAIN_CSD_EN_MASK                          0x80000000

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RESPONSE_RATE_SOURCE_OFFSET               0x00000030
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RESPONSE_RATE_SOURCE_LSB                  0
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RESPONSE_RATE_SOURCE_MSB                  7
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RESPONSE_RATE_SOURCE_MASK                 0x000000ff

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_EXT_RESPONSE_RATE_SOURCE_OFFSET           0x00000030
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_EXT_RESPONSE_RATE_SOURCE_LSB              8
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_EXT_RESPONSE_RATE_SOURCE_MSB              15
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_EXT_RESPONSE_RATE_SOURCE_MASK             0x0000ff00

#define RX_RESPONSE_REQUIRED_INFO_SW_PEER_ID_OFFSET                                 0x00000030
#define RX_RESPONSE_REQUIRED_INFO_SW_PEER_ID_LSB                                    16
#define RX_RESPONSE_REQUIRED_INFO_SW_PEER_ID_MSB                                    31
#define RX_RESPONSE_REQUIRED_INFO_SW_PEER_ID_MASK                                   0xffff0000

#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_PUNCTURE_BITMAP_OFFSET                    0x00000034
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_PUNCTURE_BITMAP_LSB                       0
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_PUNCTURE_BITMAP_MSB                       15
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_PUNCTURE_BITMAP_MASK                      0x0000ffff

#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_RESPONSE_OFFSET                           0x00000034
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_RESPONSE_LSB                              16
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_RESPONSE_MSB                              16
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_RESPONSE_MASK                             0x00010000

#define RX_RESPONSE_REQUIRED_INFO_PUNCTURED_RESPONSE_OFFSET                         0x00000034
#define RX_RESPONSE_REQUIRED_INFO_PUNCTURED_RESPONSE_LSB                            17
#define RX_RESPONSE_REQUIRED_INFO_PUNCTURED_RESPONSE_MSB                            17
#define RX_RESPONSE_REQUIRED_INFO_PUNCTURED_RESPONSE_MASK                           0x00020000

#define RX_RESPONSE_REQUIRED_INFO_EHT_DUPLICATE_MODE_OFFSET                         0x00000034
#define RX_RESPONSE_REQUIRED_INFO_EHT_DUPLICATE_MODE_LSB                            18
#define RX_RESPONSE_REQUIRED_INFO_EHT_DUPLICATE_MODE_MSB                            19
#define RX_RESPONSE_REQUIRED_INFO_EHT_DUPLICATE_MODE_MASK                           0x000c0000

#define RX_RESPONSE_REQUIRED_INFO_FORCE_EXTRA_SYMBOL_OFFSET                         0x00000034
#define RX_RESPONSE_REQUIRED_INFO_FORCE_EXTRA_SYMBOL_LSB                            20
#define RX_RESPONSE_REQUIRED_INFO_FORCE_EXTRA_SYMBOL_MSB                            20
#define RX_RESPONSE_REQUIRED_INFO_FORCE_EXTRA_SYMBOL_MASK                           0x00100000

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_13A_OFFSET                               0x00000034
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_13A_LSB                                  21
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_13A_MSB                                  25
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_13A_MASK                                 0x03e00000

#define RX_RESPONSE_REQUIRED_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET            0x00000034
#define RX_RESPONSE_REQUIRED_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB               26
#define RX_RESPONSE_REQUIRED_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB               31
#define RX_RESPONSE_REQUIRED_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK              0xfc000000

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET      0x00000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB         0
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB         9
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK        0x000003ff

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET   0x00000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB      10
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB      10
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK     0x00000400

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x00000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB   11
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB   11
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK  0x00000800

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x00000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB   12
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB   12
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK  0x00001000

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET          0x00000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB             13
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB             15
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK            0x0000e000

#define RX_RESPONSE_REQUIRED_INFO_HE_A_CONTROL_RESPONSE_TIME_OFFSET                 0x00000038
#define RX_RESPONSE_REQUIRED_INFO_HE_A_CONTROL_RESPONSE_TIME_LSB                    16
#define RX_RESPONSE_REQUIRED_INFO_HE_A_CONTROL_RESPONSE_TIME_MSB                    27
#define RX_RESPONSE_REQUIRED_INFO_HE_A_CONTROL_RESPONSE_TIME_MASK                   0x0fff0000

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_AFTER_STRUCT16_OFFSET                    0x00000038
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_AFTER_STRUCT16_LSB                       28
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_AFTER_STRUCT16_MSB                       31
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_AFTER_STRUCT16_MASK                      0xf0000000

#endif
