
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

 
 
 
 
 
 
 


#ifndef _RX_RESPONSE_REQUIRED_INFO_H_
#define _RX_RESPONSE_REQUIRED_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#include "mlo_sta_id_details.h"
#define NUM_OF_DWORDS_RX_RESPONSE_REQUIRED_INFO 16

#define NUM_OF_QWORDS_RX_RESPONSE_REQUIRED_INFO 8


struct rx_response_required_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t phy_ppdu_id                                             : 16,  
                      su_or_uplink_mu_reception                               :  1,  
                      trigger_frame_received                                  :  1,  
                      ftm_tm                                                  :  2,  
                      tb_ranging_response_required                            :  2,  
                      mac_security                                            :  1,  
                      filter_pass_monitor_ovrd                                :  1,  
                      ast_search_incomplete                                   :  1,  
                      r2r_end_status_to_follow                                :  1,  
                      reserved_0a                                             :  2,  
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
                      reserved_3a                                             :  2;  
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
                      reserved_6a                                             :  1;  
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
             uint32_t tlv64_padding                                           : 32;  
#else
             uint32_t wait_sifs                                               :  2,  
                      wait_sifs_config_valid                                  :  1,  
                      three_or_more_type_subtypes                             :  1,  
                      reserved_0a                                             :  2,  
                      r2r_end_status_to_follow                                :  1,  
                      ast_search_incomplete                                   :  1,  
                      filter_pass_monitor_ovrd                                :  1,  
                      mac_security                                            :  1,  
                      tb_ranging_response_required                            :  2,  
                      ftm_tm                                                  :  2,  
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
             uint32_t reserved_3a                                             :  2,  
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
             uint32_t reserved_6a                                             :  1,  
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
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 

#define RX_RESPONSE_REQUIRED_INFO_PHY_PPDU_ID_OFFSET                                0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_PHY_PPDU_ID_LSB                                   0
#define RX_RESPONSE_REQUIRED_INFO_PHY_PPDU_ID_MSB                                   15
#define RX_RESPONSE_REQUIRED_INFO_PHY_PPDU_ID_MASK                                  0x000000000000ffff


 

#define RX_RESPONSE_REQUIRED_INFO_SU_OR_UPLINK_MU_RECEPTION_OFFSET                  0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_SU_OR_UPLINK_MU_RECEPTION_LSB                     16
#define RX_RESPONSE_REQUIRED_INFO_SU_OR_UPLINK_MU_RECEPTION_MSB                     16
#define RX_RESPONSE_REQUIRED_INFO_SU_OR_UPLINK_MU_RECEPTION_MASK                    0x0000000000010000


 

#define RX_RESPONSE_REQUIRED_INFO_TRIGGER_FRAME_RECEIVED_OFFSET                     0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_TRIGGER_FRAME_RECEIVED_LSB                        17
#define RX_RESPONSE_REQUIRED_INFO_TRIGGER_FRAME_RECEIVED_MSB                        17
#define RX_RESPONSE_REQUIRED_INFO_TRIGGER_FRAME_RECEIVED_MASK                       0x0000000000020000


 

#define RX_RESPONSE_REQUIRED_INFO_FTM_TM_OFFSET                                     0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_FTM_TM_LSB                                        18
#define RX_RESPONSE_REQUIRED_INFO_FTM_TM_MSB                                        19
#define RX_RESPONSE_REQUIRED_INFO_FTM_TM_MASK                                       0x00000000000c0000


 

#define RX_RESPONSE_REQUIRED_INFO_TB_RANGING_RESPONSE_REQUIRED_OFFSET               0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_TB_RANGING_RESPONSE_REQUIRED_LSB                  20
#define RX_RESPONSE_REQUIRED_INFO_TB_RANGING_RESPONSE_REQUIRED_MSB                  21
#define RX_RESPONSE_REQUIRED_INFO_TB_RANGING_RESPONSE_REQUIRED_MASK                 0x0000000000300000


 

#define RX_RESPONSE_REQUIRED_INFO_MAC_SECURITY_OFFSET                               0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_MAC_SECURITY_LSB                                  22
#define RX_RESPONSE_REQUIRED_INFO_MAC_SECURITY_MSB                                  22
#define RX_RESPONSE_REQUIRED_INFO_MAC_SECURITY_MASK                                 0x0000000000400000


 

#define RX_RESPONSE_REQUIRED_INFO_FILTER_PASS_MONITOR_OVRD_OFFSET                   0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_FILTER_PASS_MONITOR_OVRD_LSB                      23
#define RX_RESPONSE_REQUIRED_INFO_FILTER_PASS_MONITOR_OVRD_MSB                      23
#define RX_RESPONSE_REQUIRED_INFO_FILTER_PASS_MONITOR_OVRD_MASK                     0x0000000000800000


 

#define RX_RESPONSE_REQUIRED_INFO_AST_SEARCH_INCOMPLETE_OFFSET                      0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_AST_SEARCH_INCOMPLETE_LSB                         24
#define RX_RESPONSE_REQUIRED_INFO_AST_SEARCH_INCOMPLETE_MSB                         24
#define RX_RESPONSE_REQUIRED_INFO_AST_SEARCH_INCOMPLETE_MASK                        0x0000000001000000


 

#define RX_RESPONSE_REQUIRED_INFO_R2R_END_STATUS_TO_FOLLOW_OFFSET                   0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_R2R_END_STATUS_TO_FOLLOW_LSB                      25
#define RX_RESPONSE_REQUIRED_INFO_R2R_END_STATUS_TO_FOLLOW_MSB                      25
#define RX_RESPONSE_REQUIRED_INFO_R2R_END_STATUS_TO_FOLLOW_MASK                     0x0000000002000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_0A_OFFSET                                0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_0A_LSB                                   26
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_0A_MSB                                   27
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_0A_MASK                                  0x000000000c000000


 

#define RX_RESPONSE_REQUIRED_INFO_THREE_OR_MORE_TYPE_SUBTYPES_OFFSET                0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_THREE_OR_MORE_TYPE_SUBTYPES_LSB                   28
#define RX_RESPONSE_REQUIRED_INFO_THREE_OR_MORE_TYPE_SUBTYPES_MSB                   28
#define RX_RESPONSE_REQUIRED_INFO_THREE_OR_MORE_TYPE_SUBTYPES_MASK                  0x0000000010000000


 

#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_CONFIG_VALID_OFFSET                     0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_CONFIG_VALID_LSB                        29
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_CONFIG_VALID_MSB                        29
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_CONFIG_VALID_MASK                       0x0000000020000000


 

#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_OFFSET                                  0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_LSB                                     30
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_MSB                                     31
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_MASK                                    0x00000000c0000000


 

#define RX_RESPONSE_REQUIRED_INFO_GENERAL_FRAME_CONTROL_OFFSET                      0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_GENERAL_FRAME_CONTROL_LSB                         32
#define RX_RESPONSE_REQUIRED_INFO_GENERAL_FRAME_CONTROL_MSB                         47
#define RX_RESPONSE_REQUIRED_INFO_GENERAL_FRAME_CONTROL_MASK                        0x0000ffff00000000


 

#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_OFFSET                       0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_LSB                          48
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_MSB                          63
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_MASK                         0xffff000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_DURATION_OFFSET                                   0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_DURATION_LSB                                      0
#define RX_RESPONSE_REQUIRED_INFO_DURATION_MSB                                      15
#define RX_RESPONSE_REQUIRED_INFO_DURATION_MASK                                     0x000000000000ffff


 

#define RX_RESPONSE_REQUIRED_INFO_PKT_TYPE_OFFSET                                   0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_PKT_TYPE_LSB                                      16
#define RX_RESPONSE_REQUIRED_INFO_PKT_TYPE_MSB                                      19
#define RX_RESPONSE_REQUIRED_INFO_PKT_TYPE_MASK                                     0x00000000000f0000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_SU_EXTENDED_OFFSET                        0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_SU_EXTENDED_LSB                           20
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_SU_EXTENDED_MSB                           20
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_SU_EXTENDED_MASK                          0x0000000000100000


 

#define RX_RESPONSE_REQUIRED_INFO_RATE_MCS_OFFSET                                   0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_RATE_MCS_LSB                                      21
#define RX_RESPONSE_REQUIRED_INFO_RATE_MCS_MSB                                      24
#define RX_RESPONSE_REQUIRED_INFO_RATE_MCS_MASK                                     0x0000000001e00000


 

#define RX_RESPONSE_REQUIRED_INFO_SGI_OFFSET                                        0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_SGI_LSB                                           25
#define RX_RESPONSE_REQUIRED_INFO_SGI_MSB                                           26
#define RX_RESPONSE_REQUIRED_INFO_SGI_MASK                                          0x0000000006000000


 

#define RX_RESPONSE_REQUIRED_INFO_STBC_OFFSET                                       0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_STBC_LSB                                          27
#define RX_RESPONSE_REQUIRED_INFO_STBC_MSB                                          27
#define RX_RESPONSE_REQUIRED_INFO_STBC_MASK                                         0x0000000008000000


 

#define RX_RESPONSE_REQUIRED_INFO_LDPC_OFFSET                                       0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_LDPC_LSB                                          28
#define RX_RESPONSE_REQUIRED_INFO_LDPC_MSB                                          28
#define RX_RESPONSE_REQUIRED_INFO_LDPC_MASK                                         0x0000000010000000


 

#define RX_RESPONSE_REQUIRED_INFO_AMPDU_OFFSET                                      0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_AMPDU_LSB                                         29
#define RX_RESPONSE_REQUIRED_INFO_AMPDU_MSB                                         29
#define RX_RESPONSE_REQUIRED_INFO_AMPDU_MASK                                        0x0000000020000000


 

#define RX_RESPONSE_REQUIRED_INFO_VHT_ACK_OFFSET                                    0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_VHT_ACK_LSB                                       30
#define RX_RESPONSE_REQUIRED_INFO_VHT_ACK_MSB                                       30
#define RX_RESPONSE_REQUIRED_INFO_VHT_ACK_MASK                                      0x0000000040000000


 

#define RX_RESPONSE_REQUIRED_INFO_RTS_TA_GRP_BIT_OFFSET                             0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_RTS_TA_GRP_BIT_LSB                                31
#define RX_RESPONSE_REQUIRED_INFO_RTS_TA_GRP_BIT_MSB                                31
#define RX_RESPONSE_REQUIRED_INFO_RTS_TA_GRP_BIT_MASK                               0x0000000080000000


 

#define RX_RESPONSE_REQUIRED_INFO_CTRL_FRAME_SOLICITING_RESP_OFFSET                 0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_CTRL_FRAME_SOLICITING_RESP_LSB                    32
#define RX_RESPONSE_REQUIRED_INFO_CTRL_FRAME_SOLICITING_RESP_MSB                    32
#define RX_RESPONSE_REQUIRED_INFO_CTRL_FRAME_SOLICITING_RESP_MASK                   0x0000000100000000


 

#define RX_RESPONSE_REQUIRED_INFO_AST_FAIL_FOR_DOT11AX_SU_EXT_OFFSET                0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_AST_FAIL_FOR_DOT11AX_SU_EXT_LSB                   33
#define RX_RESPONSE_REQUIRED_INFO_AST_FAIL_FOR_DOT11AX_SU_EXT_MSB                   33
#define RX_RESPONSE_REQUIRED_INFO_AST_FAIL_FOR_DOT11AX_SU_EXT_MASK                  0x0000000200000000


 

#define RX_RESPONSE_REQUIRED_INFO_SERVICE_DYNAMIC_OFFSET                            0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_DYNAMIC_LSB                               34
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_DYNAMIC_MSB                               34
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_DYNAMIC_MASK                              0x0000000400000000


 

#define RX_RESPONSE_REQUIRED_INFO_M_PKT_OFFSET                                      0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_M_PKT_LSB                                         35
#define RX_RESPONSE_REQUIRED_INFO_M_PKT_MSB                                         35
#define RX_RESPONSE_REQUIRED_INFO_M_PKT_MASK                                        0x0000000800000000


 

#define RX_RESPONSE_REQUIRED_INFO_STA_PARTIAL_AID_OFFSET                            0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_STA_PARTIAL_AID_LSB                               36
#define RX_RESPONSE_REQUIRED_INFO_STA_PARTIAL_AID_MSB                               47
#define RX_RESPONSE_REQUIRED_INFO_STA_PARTIAL_AID_MASK                              0x0000fff000000000


 

#define RX_RESPONSE_REQUIRED_INFO_GROUP_ID_OFFSET                                   0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_GROUP_ID_LSB                                      48
#define RX_RESPONSE_REQUIRED_INFO_GROUP_ID_MSB                                      53
#define RX_RESPONSE_REQUIRED_INFO_GROUP_ID_MASK                                     0x003f000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_CTRL_RESP_PWR_MGMT_OFFSET                         0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_CTRL_RESP_PWR_MGMT_LSB                            54
#define RX_RESPONSE_REQUIRED_INFO_CTRL_RESP_PWR_MGMT_MSB                            54
#define RX_RESPONSE_REQUIRED_INFO_CTRL_RESP_PWR_MGMT_MASK                           0x0040000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_INDICATION_OFFSET                        0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_INDICATION_LSB                           55
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_INDICATION_MSB                           56
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_INDICATION_MASK                          0x0180000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_NDP_INDICATION_OFFSET                             0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_NDP_INDICATION_LSB                                57
#define RX_RESPONSE_REQUIRED_INFO_NDP_INDICATION_MSB                                57
#define RX_RESPONSE_REQUIRED_INFO_NDP_INDICATION_MASK                               0x0200000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_NDP_FRAME_TYPE_OFFSET                             0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_NDP_FRAME_TYPE_LSB                                58
#define RX_RESPONSE_REQUIRED_INFO_NDP_FRAME_TYPE_MSB                                60
#define RX_RESPONSE_REQUIRED_INFO_NDP_FRAME_TYPE_MASK                               0x1c00000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_VALID_OFFSET                 0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_VALID_LSB                    61
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_VALID_MSB                    61
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_VALID_MASK                   0x2000000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_3A_OFFSET                                0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_3A_LSB                                   62
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_3A_MSB                                   63
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_3A_MASK                                  0xc000000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_OFFSET                                     0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_LSB                                        0
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_MSB                                        15
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_MASK                                       0x000000000000ffff


 

#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_EXT_OFFSET                                 0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_EXT_LSB                                    16
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_EXT_MSB                                    25
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_EXT_MASK                                   0x0000000003ff0000


 

#define RX_RESPONSE_REQUIRED_INFO_AGC_CBW_OFFSET                                    0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_AGC_CBW_LSB                                       26
#define RX_RESPONSE_REQUIRED_INFO_AGC_CBW_MSB                                       28
#define RX_RESPONSE_REQUIRED_INFO_AGC_CBW_MASK                                      0x000000001c000000


 

#define RX_RESPONSE_REQUIRED_INFO_SERVICE_CBW_OFFSET                                0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_CBW_LSB                                   29
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_CBW_MSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_CBW_MASK                                  0x00000000e0000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_STA_COUNT_OFFSET                         0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_STA_COUNT_LSB                            32
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_STA_COUNT_MSB                            38
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_STA_COUNT_MASK                           0x0000007f00000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_OFFSET                                   0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_LSB                                      39
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_MSB                                      42
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_MASK                                     0x0000078000000000


 

#define RX_RESPONSE_REQUIRED_INFO_HT_VHT_SIG_CBW_OFFSET                             0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_HT_VHT_SIG_CBW_LSB                                43
#define RX_RESPONSE_REQUIRED_INFO_HT_VHT_SIG_CBW_MSB                                45
#define RX_RESPONSE_REQUIRED_INFO_HT_VHT_SIG_CBW_MASK                               0x0000380000000000


 

#define RX_RESPONSE_REQUIRED_INFO_CTS_CBW_OFFSET                                    0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_CTS_CBW_LSB                                       46
#define RX_RESPONSE_REQUIRED_INFO_CTS_CBW_MSB                                       48
#define RX_RESPONSE_REQUIRED_INFO_CTS_CBW_MASK                                      0x0001c00000000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ACK_COUNT_OFFSET                         0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ACK_COUNT_LSB                            49
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ACK_COUNT_MSB                            55
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ACK_COUNT_MASK                           0x00fe000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ASSOC_ACK_COUNT_OFFSET                   0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ASSOC_ACK_COUNT_LSB                      56
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ASSOC_ACK_COUNT_MSB                      62
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ASSOC_ACK_COUNT_MASK                     0x7f00000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_TXOP_DURATION_ALL_ONES_OFFSET                     0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_TXOP_DURATION_ALL_ONES_LSB                        63
#define RX_RESPONSE_REQUIRED_INFO_TXOP_DURATION_ALL_ONES_MSB                        63
#define RX_RESPONSE_REQUIRED_INFO_TXOP_DURATION_ALL_ONES_MASK                       0x8000000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA32_COUNT_OFFSET                        0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA32_COUNT_LSB                           0
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA32_COUNT_MSB                           6
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA32_COUNT_MASK                          0x000000000000007f


 

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA64_COUNT_OFFSET                        0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA64_COUNT_LSB                           7
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA64_COUNT_MSB                           13
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA64_COUNT_MASK                          0x0000000000003f80


 

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA128_COUNT_OFFSET                       0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA128_COUNT_LSB                          14
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA128_COUNT_MSB                          20
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA128_COUNT_MASK                         0x00000000001fc000


 

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA256_COUNT_OFFSET                       0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA256_COUNT_LSB                          21
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA256_COUNT_MSB                          27
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA256_COUNT_MASK                         0x000000000fe00000


 

#define RX_RESPONSE_REQUIRED_INFO_MULTI_TID_OFFSET                                  0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_MULTI_TID_LSB                                     28
#define RX_RESPONSE_REQUIRED_INFO_MULTI_TID_MSB                                     28
#define RX_RESPONSE_REQUIRED_INFO_MULTI_TID_MASK                                    0x0000000010000000


 

#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_OFFSET                0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_LSB                   29
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_MSB                   29
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_MASK                  0x0000000020000000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_DL_UL_FLAG_OFFSET                         0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_DL_UL_FLAG_LSB                            30
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_DL_UL_FLAG_MSB                            30
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_DL_UL_FLAG_MASK                           0x0000000040000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_6A_OFFSET                                0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_6A_LSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_6A_MSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_6A_MASK                                  0x0000000080000000


 

#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_FRAME_LENGTH_OFFSET                   0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_FRAME_LENGTH_LSB                      32
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_FRAME_LENGTH_MSB                      47
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_FRAME_LENGTH_MASK                     0x0000ffff00000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA512_COUNT_OFFSET                       0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA512_COUNT_LSB                          48
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA512_COUNT_MSB                          54
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA512_COUNT_MASK                         0x007f000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA1024_COUNT_OFFSET                      0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA1024_COUNT_LSB                         55
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA1024_COUNT_MSB                         61
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA1024_COUNT_MASK                        0x3f80000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_7A_OFFSET                                0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_7A_LSB                                   62
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_7A_MSB                                   63
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_7A_MASK                                  0xc000000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_ADDR1_31_0_OFFSET                                 0x0000000000000020
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_31_0_LSB                                    0
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_31_0_MSB                                    31
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_31_0_MASK                                   0x00000000ffffffff


 

#define RX_RESPONSE_REQUIRED_INFO_ADDR1_47_32_OFFSET                                0x0000000000000020
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_47_32_LSB                                   32
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_47_32_MSB                                   47
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_47_32_MASK                                  0x0000ffff00000000


 

#define RX_RESPONSE_REQUIRED_INFO_ADDR2_15_0_OFFSET                                 0x0000000000000020
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_15_0_LSB                                    48
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_15_0_MSB                                    63
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_15_0_MASK                                   0xffff000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_ADDR2_47_16_OFFSET                                0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_47_16_LSB                                   0
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_47_16_MSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_47_16_MASK                                  0x00000000ffffffff


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_OFFSET         0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_LSB            32
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_MSB            32
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_MASK           0x0000000100000000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_OFFSET                0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_LSB                   33
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_MSB                   33
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_MASK                  0x0000000200000000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_OFFSET              0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_LSB                 34
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_MSB                 39
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_MASK                0x000000fc00000000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_OFFSET             0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_LSB                40
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_MSB                43
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_MASK               0x00000f0000000000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CP_SIZE_OFFSET                   0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CP_SIZE_LSB                      44
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CP_SIZE_MSB                      45
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CP_SIZE_MASK                     0x0000300000000000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_LTF_SIZE_OFFSET                  0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_LTF_SIZE_LSB                     46
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_LTF_SIZE_MSB                     47
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_LTF_SIZE_MASK                    0x0000c00000000000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CODING_OFFSET                    0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CODING_LSB                       48
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CODING_MSB                       48
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CODING_MASK                      0x0001000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DCM_OFFSET                       0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DCM_LSB                          49
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DCM_MSB                          49
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DCM_MASK                         0x0002000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_OFFSET        0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_LSB           50
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_MSB           50
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_MASK          0x0004000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_OFFSET               0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_LSB                  51
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_MSB                  54
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_MASK                 0x0078000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_FTM_FIELDS_VALID_OFFSET                           0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_FTM_FIELDS_VALID_LSB                              55
#define RX_RESPONSE_REQUIRED_INFO_FTM_FIELDS_VALID_MSB                              55
#define RX_RESPONSE_REQUIRED_INFO_FTM_FIELDS_VALID_MASK                             0x0080000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_NSS_OFFSET                                 0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_NSS_LSB                                    56
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_NSS_MSB                                    58
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_NSS_MASK                                   0x0700000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_LTF_SIZE_OFFSET                            0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_LTF_SIZE_LSB                               59
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_LTF_SIZE_MSB                               60
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_LTF_SIZE_MASK                              0x1800000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CONTENT_OFFSET                             0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CONTENT_LSB                                61
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CONTENT_MSB                                61
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CONTENT_MASK                               0x2000000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_FTM_CHAIN_CSD_EN_OFFSET                           0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_FTM_CHAIN_CSD_EN_LSB                              62
#define RX_RESPONSE_REQUIRED_INFO_FTM_CHAIN_CSD_EN_MSB                              62
#define RX_RESPONSE_REQUIRED_INFO_FTM_CHAIN_CSD_EN_MASK                             0x4000000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CHAIN_CSD_EN_OFFSET                        0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CHAIN_CSD_EN_LSB                           63
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CHAIN_CSD_EN_MSB                           63
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CHAIN_CSD_EN_MASK                          0x8000000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RESPONSE_RATE_SOURCE_OFFSET               0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RESPONSE_RATE_SOURCE_LSB                  0
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RESPONSE_RATE_SOURCE_MSB                  7
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RESPONSE_RATE_SOURCE_MASK                 0x00000000000000ff


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_EXT_RESPONSE_RATE_SOURCE_OFFSET           0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_EXT_RESPONSE_RATE_SOURCE_LSB              8
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_EXT_RESPONSE_RATE_SOURCE_MSB              15
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_EXT_RESPONSE_RATE_SOURCE_MASK             0x000000000000ff00


 

#define RX_RESPONSE_REQUIRED_INFO_SW_PEER_ID_OFFSET                                 0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_SW_PEER_ID_LSB                                    16
#define RX_RESPONSE_REQUIRED_INFO_SW_PEER_ID_MSB                                    31
#define RX_RESPONSE_REQUIRED_INFO_SW_PEER_ID_MASK                                   0x00000000ffff0000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_PUNCTURE_BITMAP_OFFSET                    0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_PUNCTURE_BITMAP_LSB                       32
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_PUNCTURE_BITMAP_MSB                       47
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_PUNCTURE_BITMAP_MASK                      0x0000ffff00000000


 

#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_RESPONSE_OFFSET                           0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_RESPONSE_LSB                              48
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_RESPONSE_MSB                              48
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_RESPONSE_MASK                             0x0001000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_PUNCTURED_RESPONSE_OFFSET                         0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_PUNCTURED_RESPONSE_LSB                            49
#define RX_RESPONSE_REQUIRED_INFO_PUNCTURED_RESPONSE_MSB                            49
#define RX_RESPONSE_REQUIRED_INFO_PUNCTURED_RESPONSE_MASK                           0x0002000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_EHT_DUPLICATE_MODE_OFFSET                         0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_EHT_DUPLICATE_MODE_LSB                            50
#define RX_RESPONSE_REQUIRED_INFO_EHT_DUPLICATE_MODE_MSB                            51
#define RX_RESPONSE_REQUIRED_INFO_EHT_DUPLICATE_MODE_MASK                           0x000c000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_FORCE_EXTRA_SYMBOL_OFFSET                         0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_FORCE_EXTRA_SYMBOL_LSB                            52
#define RX_RESPONSE_REQUIRED_INFO_FORCE_EXTRA_SYMBOL_MSB                            52
#define RX_RESPONSE_REQUIRED_INFO_FORCE_EXTRA_SYMBOL_MASK                           0x0010000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_13A_OFFSET                               0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_13A_LSB                                  53
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_13A_MSB                                  57
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_13A_MASK                                 0x03e0000000000000


 

#define RX_RESPONSE_REQUIRED_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET            0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB               58
#define RX_RESPONSE_REQUIRED_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB               63
#define RX_RESPONSE_REQUIRED_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK              0xfc00000000000000


 


 

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET      0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB         0
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB         9
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK        0x00000000000003ff


 

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET   0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB      10
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB      10
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK     0x0000000000000400


 

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB   11
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB   11
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK  0x0000000000000800


 

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB   12
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB   12
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK  0x0000000000001000


 

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET          0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB             13
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB             15
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK            0x000000000000e000


 

#define RX_RESPONSE_REQUIRED_INFO_HE_A_CONTROL_RESPONSE_TIME_OFFSET                 0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_HE_A_CONTROL_RESPONSE_TIME_LSB                    16
#define RX_RESPONSE_REQUIRED_INFO_HE_A_CONTROL_RESPONSE_TIME_MSB                    27
#define RX_RESPONSE_REQUIRED_INFO_HE_A_CONTROL_RESPONSE_TIME_MASK                   0x000000000fff0000


 

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_AFTER_STRUCT16_OFFSET                    0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_AFTER_STRUCT16_LSB                       28
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_AFTER_STRUCT16_MSB                       31
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_AFTER_STRUCT16_MASK                      0x00000000f0000000


 

#define RX_RESPONSE_REQUIRED_INFO_TLV64_PADDING_OFFSET                              0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_TLV64_PADDING_LSB                                 32
#define RX_RESPONSE_REQUIRED_INFO_TLV64_PADDING_MSB                                 63
#define RX_RESPONSE_REQUIRED_INFO_TLV64_PADDING_MASK                                0xffffffff00000000



#endif    
