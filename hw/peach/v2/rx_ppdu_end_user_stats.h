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


#ifndef _RX_PPDU_END_USER_STATS_H_
#define _RX_PPDU_END_USER_STATS_H_

#include "rx_rxpcu_classification_overview.h"
#define NUM_OF_DWORDS_RX_PPDU_END_USER_STATS 30

struct rx_ppdu_end_user_stats {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   rx_rxpcu_classification_overview                          rxpcu_classification_details;
             uint32_t sta_full_aid                                            : 13,
                      mcs                                                     :  4,
                      nss                                                     :  3,
                      expected_response_ack_or_ba                             :  1,
                      reserved_1a                                             : 11;
             uint32_t sw_peer_id                                              : 16,
                      mpdu_cnt_fcs_err                                        : 11,
                      sw2rxdma0_buf_source_used                               :  1,
                      fw2rxdma_pmac0_buf_source_used                          :  1,
                      sw2rxdma1_buf_source_used                               :  1,
                      sw2rxdma_exception_buf_source_used                      :  1,
                      fw2rxdma_pmac1_buf_source_used                          :  1;
             uint32_t mpdu_cnt_fcs_ok                                         : 11,
                      frame_control_info_valid                                :  1,
                      qos_control_info_valid                                  :  1,
                      ht_control_info_valid                                   :  1,
                      data_sequence_control_info_valid                        :  1,
                      ht_control_info_null_valid                              :  1,
                      rxdma2fw_pmac1_ring_used                                :  1,
                      rxdma2reo_ring_used                                     :  1,
                      rxdma2fw_pmac0_ring_used                                :  1,
                      rxdma2sw_ring_used                                      :  1,
                      rxdma_release_ring_used                                 :  1,
                      ht_control_field_pkt_type                               :  4,
                      rxdma2reo_remote0_ring_used                             :  1,
                      rxdma2reo_remote1_ring_used                             :  1,
                      reserved_3b                                             :  5;
             uint32_t ast_index                                               : 16,
                      frame_control_field                                     : 16;
             uint32_t first_data_seq_ctrl                                     : 16,
                      qos_control_field                                       : 16;
             uint32_t ht_control_field                                        : 32;
             uint32_t fcs_ok_bitmap_31_0                                      : 32;
             uint32_t fcs_ok_bitmap_63_32                                     : 32;
             uint32_t udp_msdu_count                                          : 16,
                      tcp_msdu_count                                          : 16;
             uint32_t other_msdu_count                                        : 16,
                      tcp_ack_msdu_count                                      : 16;
             uint32_t sw_response_reference_ptr                               : 32;
             uint32_t received_qos_data_tid_bitmap                            : 16,
                      received_qos_data_tid_eosp_bitmap                       : 16;
             uint32_t qosctrl_15_8_tid0                                       :  8,
                      qosctrl_15_8_tid1                                       :  8,
                      qosctrl_15_8_tid2                                       :  8,
                      qosctrl_15_8_tid3                                       :  8;
             uint32_t qosctrl_15_8_tid4                                       :  8,
                      qosctrl_15_8_tid5                                       :  8,
                      qosctrl_15_8_tid6                                       :  8,
                      qosctrl_15_8_tid7                                       :  8;
             uint32_t qosctrl_15_8_tid8                                       :  8,
                      qosctrl_15_8_tid9                                       :  8,
                      qosctrl_15_8_tid10                                      :  8,
                      qosctrl_15_8_tid11                                      :  8;
             uint32_t qosctrl_15_8_tid12                                      :  8,
                      qosctrl_15_8_tid13                                      :  8,
                      qosctrl_15_8_tid14                                      :  8,
                      qosctrl_15_8_tid15                                      :  8;
             uint32_t mpdu_ok_byte_count                                      : 25,
                      ampdu_delim_ok_count_6_0                                :  7;
             uint32_t ampdu_delim_err_count                                   : 25,
                      ampdu_delim_ok_count_13_7                               :  7;
             uint32_t mpdu_err_byte_count                                     : 25,
                      ampdu_delim_ok_count_20_14                              :  7;
             uint32_t non_consecutive_delimiter_err                           : 16,
                      retried_msdu_count                                      : 16;
             uint32_t ht_control_null_field                                   : 32;
             uint32_t sw_response_reference_ptr_ext                           : 32;
             uint32_t corrupted_due_to_fifo_delay                             :  1,
                      frame_control_info_null_valid                           :  1,
                      frame_control_field_null                                : 16,
                      retried_mpdu_count                                      : 11,
                      reserved_23a                                            :  3;
             uint32_t rxpcu_mpdu_filter_in_category                           :  2,
                      sw_frame_group_id                                       :  7,
                      reserved_24a                                            :  4,
                      frame_control_info_mgmt_ctrl_valid                      :  1,
                      mac_addr_ad2_valid                                      :  1,
                      mcast_bcast                                             :  1,
                      frame_control_field_mgmt_ctrl                           : 16;
             uint32_t user_ppdu_len                                           : 24,
                      reserved_25a                                            :  8;
             uint32_t mac_addr_ad2_31_0                                       : 32;
             uint32_t mac_addr_ad2_47_32                                      : 16,
                      amsdu_msdu_count                                        : 16;
             uint32_t non_amsdu_msdu_count                                    : 16,
                      ucast_msdu_count                                        : 16;
             uint32_t bcast_msdu_count                                        : 16,
                      mcast_bcast_msdu_count                                  : 16;
#else
             struct   rx_rxpcu_classification_overview                          rxpcu_classification_details;
             uint32_t reserved_1a                                             : 11,
                      expected_response_ack_or_ba                             :  1,
                      nss                                                     :  3,
                      mcs                                                     :  4,
                      sta_full_aid                                            : 13;
             uint32_t fw2rxdma_pmac1_buf_source_used                          :  1,
                      sw2rxdma_exception_buf_source_used                      :  1,
                      sw2rxdma1_buf_source_used                               :  1,
                      fw2rxdma_pmac0_buf_source_used                          :  1,
                      sw2rxdma0_buf_source_used                               :  1,
                      mpdu_cnt_fcs_err                                        : 11,
                      sw_peer_id                                              : 16;
             uint32_t reserved_3b                                             :  5,
                      rxdma2reo_remote1_ring_used                             :  1,
                      rxdma2reo_remote0_ring_used                             :  1,
                      ht_control_field_pkt_type                               :  4,
                      rxdma_release_ring_used                                 :  1,
                      rxdma2sw_ring_used                                      :  1,
                      rxdma2fw_pmac0_ring_used                                :  1,
                      rxdma2reo_ring_used                                     :  1,
                      rxdma2fw_pmac1_ring_used                                :  1,
                      ht_control_info_null_valid                              :  1,
                      data_sequence_control_info_valid                        :  1,
                      ht_control_info_valid                                   :  1,
                      qos_control_info_valid                                  :  1,
                      frame_control_info_valid                                :  1,
                      mpdu_cnt_fcs_ok                                         : 11;
             uint32_t frame_control_field                                     : 16,
                      ast_index                                               : 16;
             uint32_t qos_control_field                                       : 16,
                      first_data_seq_ctrl                                     : 16;
             uint32_t ht_control_field                                        : 32;
             uint32_t fcs_ok_bitmap_31_0                                      : 32;
             uint32_t fcs_ok_bitmap_63_32                                     : 32;
             uint32_t tcp_msdu_count                                          : 16,
                      udp_msdu_count                                          : 16;
             uint32_t tcp_ack_msdu_count                                      : 16,
                      other_msdu_count                                        : 16;
             uint32_t sw_response_reference_ptr                               : 32;
             uint32_t received_qos_data_tid_eosp_bitmap                       : 16,
                      received_qos_data_tid_bitmap                            : 16;
             uint32_t qosctrl_15_8_tid3                                       :  8,
                      qosctrl_15_8_tid2                                       :  8,
                      qosctrl_15_8_tid1                                       :  8,
                      qosctrl_15_8_tid0                                       :  8;
             uint32_t qosctrl_15_8_tid7                                       :  8,
                      qosctrl_15_8_tid6                                       :  8,
                      qosctrl_15_8_tid5                                       :  8,
                      qosctrl_15_8_tid4                                       :  8;
             uint32_t qosctrl_15_8_tid11                                      :  8,
                      qosctrl_15_8_tid10                                      :  8,
                      qosctrl_15_8_tid9                                       :  8,
                      qosctrl_15_8_tid8                                       :  8;
             uint32_t qosctrl_15_8_tid15                                      :  8,
                      qosctrl_15_8_tid14                                      :  8,
                      qosctrl_15_8_tid13                                      :  8,
                      qosctrl_15_8_tid12                                      :  8;
             uint32_t ampdu_delim_ok_count_6_0                                :  7,
                      mpdu_ok_byte_count                                      : 25;
             uint32_t ampdu_delim_ok_count_13_7                               :  7,
                      ampdu_delim_err_count                                   : 25;
             uint32_t ampdu_delim_ok_count_20_14                              :  7,
                      mpdu_err_byte_count                                     : 25;
             uint32_t retried_msdu_count                                      : 16,
                      non_consecutive_delimiter_err                           : 16;
             uint32_t ht_control_null_field                                   : 32;
             uint32_t sw_response_reference_ptr_ext                           : 32;
             uint32_t reserved_23a                                            :  3,
                      retried_mpdu_count                                      : 11,
                      frame_control_field_null                                : 16,
                      frame_control_info_null_valid                           :  1,
                      corrupted_due_to_fifo_delay                             :  1;
             uint32_t frame_control_field_mgmt_ctrl                           : 16,
                      mcast_bcast                                             :  1,
                      mac_addr_ad2_valid                                      :  1,
                      frame_control_info_mgmt_ctrl_valid                      :  1,
                      reserved_24a                                            :  4,
                      sw_frame_group_id                                       :  7,
                      rxpcu_mpdu_filter_in_category                           :  2;
             uint32_t reserved_25a                                            :  8,
                      user_ppdu_len                                           : 24;
             uint32_t mac_addr_ad2_31_0                                       : 32;
             uint32_t amsdu_msdu_count                                        : 16,
                      mac_addr_ad2_47_32                                      : 16;
             uint32_t ucast_msdu_count                                        : 16,
                      non_amsdu_msdu_count                                    : 16;
             uint32_t mcast_bcast_msdu_count                                  : 16,
                      bcast_msdu_count                                        : 16;
#endif
};

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_OFFSET 0x00000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_LSB   0
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_MSB   0
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_MASK  0x00000001

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_FCS_OK_OFFSET 0x00000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_FCS_OK_LSB 1
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_FCS_OK_MSB 1
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_FCS_OK_MASK 0x00000002

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_OFFSET 0x00000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_LSB 2
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_MSB 2
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_MASK 0x00000004

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_FCS_OK_OFFSET 0x00000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_FCS_OK_LSB 3
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_FCS_OK_MSB 3
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_FCS_OK_MASK 0x00000008

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_OFFSET 0x00000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_LSB 4
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_MSB 4
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_MASK 0x00000010

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_FCS_OK_OFFSET 0x00000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_FCS_OK_LSB 5
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_FCS_OK_MSB 5
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_FCS_OK_MASK 0x00000020

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHYRX_ABORT_RECEIVED_OFFSET 0x00000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHYRX_ABORT_RECEIVED_LSB 6
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHYRX_ABORT_RECEIVED_MSB 6
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHYRX_ABORT_RECEIVED_MASK 0x00000040

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_OFFSET 0x00000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_LSB 7
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_MSB 7
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_MASK 0x00000080

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_OFFSET 0x00000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_LSB 8
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_MSB 8
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_MASK 0x00000100

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_RESERVED_0_OFFSET       0x00000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_RESERVED_0_LSB          9
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_RESERVED_0_MSB          15
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_RESERVED_0_MASK         0x0000fe00

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHY_PPDU_ID_OFFSET      0x00000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHY_PPDU_ID_LSB         16
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHY_PPDU_ID_MSB         31
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHY_PPDU_ID_MASK        0xffff0000

#define RX_PPDU_END_USER_STATS_STA_FULL_AID_OFFSET                                  0x00000004
#define RX_PPDU_END_USER_STATS_STA_FULL_AID_LSB                                     0
#define RX_PPDU_END_USER_STATS_STA_FULL_AID_MSB                                     12
#define RX_PPDU_END_USER_STATS_STA_FULL_AID_MASK                                    0x00001fff

#define RX_PPDU_END_USER_STATS_MCS_OFFSET                                           0x00000004
#define RX_PPDU_END_USER_STATS_MCS_LSB                                              13
#define RX_PPDU_END_USER_STATS_MCS_MSB                                              16
#define RX_PPDU_END_USER_STATS_MCS_MASK                                             0x0001e000

#define RX_PPDU_END_USER_STATS_NSS_OFFSET                                           0x00000004
#define RX_PPDU_END_USER_STATS_NSS_LSB                                              17
#define RX_PPDU_END_USER_STATS_NSS_MSB                                              19
#define RX_PPDU_END_USER_STATS_NSS_MASK                                             0x000e0000

#define RX_PPDU_END_USER_STATS_EXPECTED_RESPONSE_ACK_OR_BA_OFFSET                   0x00000004
#define RX_PPDU_END_USER_STATS_EXPECTED_RESPONSE_ACK_OR_BA_LSB                      20
#define RX_PPDU_END_USER_STATS_EXPECTED_RESPONSE_ACK_OR_BA_MSB                      20
#define RX_PPDU_END_USER_STATS_EXPECTED_RESPONSE_ACK_OR_BA_MASK                     0x00100000

#define RX_PPDU_END_USER_STATS_RESERVED_1A_OFFSET                                   0x00000004
#define RX_PPDU_END_USER_STATS_RESERVED_1A_LSB                                      21
#define RX_PPDU_END_USER_STATS_RESERVED_1A_MSB                                      31
#define RX_PPDU_END_USER_STATS_RESERVED_1A_MASK                                     0xffe00000

#define RX_PPDU_END_USER_STATS_SW_PEER_ID_OFFSET                                    0x00000008
#define RX_PPDU_END_USER_STATS_SW_PEER_ID_LSB                                       0
#define RX_PPDU_END_USER_STATS_SW_PEER_ID_MSB                                       15
#define RX_PPDU_END_USER_STATS_SW_PEER_ID_MASK                                      0x0000ffff

#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_ERR_OFFSET                              0x00000008
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_ERR_LSB                                 16
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_ERR_MSB                                 26
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_ERR_MASK                                0x07ff0000

#define RX_PPDU_END_USER_STATS_SW2RXDMA0_BUF_SOURCE_USED_OFFSET                     0x00000008
#define RX_PPDU_END_USER_STATS_SW2RXDMA0_BUF_SOURCE_USED_LSB                        27
#define RX_PPDU_END_USER_STATS_SW2RXDMA0_BUF_SOURCE_USED_MSB                        27
#define RX_PPDU_END_USER_STATS_SW2RXDMA0_BUF_SOURCE_USED_MASK                       0x08000000

#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC0_BUF_SOURCE_USED_OFFSET                0x00000008
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC0_BUF_SOURCE_USED_LSB                   28
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC0_BUF_SOURCE_USED_MSB                   28
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC0_BUF_SOURCE_USED_MASK                  0x10000000

#define RX_PPDU_END_USER_STATS_SW2RXDMA1_BUF_SOURCE_USED_OFFSET                     0x00000008
#define RX_PPDU_END_USER_STATS_SW2RXDMA1_BUF_SOURCE_USED_LSB                        29
#define RX_PPDU_END_USER_STATS_SW2RXDMA1_BUF_SOURCE_USED_MSB                        29
#define RX_PPDU_END_USER_STATS_SW2RXDMA1_BUF_SOURCE_USED_MASK                       0x20000000

#define RX_PPDU_END_USER_STATS_SW2RXDMA_EXCEPTION_BUF_SOURCE_USED_OFFSET            0x00000008
#define RX_PPDU_END_USER_STATS_SW2RXDMA_EXCEPTION_BUF_SOURCE_USED_LSB               30
#define RX_PPDU_END_USER_STATS_SW2RXDMA_EXCEPTION_BUF_SOURCE_USED_MSB               30
#define RX_PPDU_END_USER_STATS_SW2RXDMA_EXCEPTION_BUF_SOURCE_USED_MASK              0x40000000

#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC1_BUF_SOURCE_USED_OFFSET                0x00000008
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC1_BUF_SOURCE_USED_LSB                   31
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC1_BUF_SOURCE_USED_MSB                   31
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC1_BUF_SOURCE_USED_MASK                  0x80000000

#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_OK_OFFSET                               0x0000000c
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_OK_LSB                                  0
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_OK_MSB                                  10
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_OK_MASK                                 0x000007ff

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_VALID_OFFSET                      0x0000000c
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_VALID_LSB                         11
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_VALID_MSB                         11
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_VALID_MASK                        0x00000800

#define RX_PPDU_END_USER_STATS_QOS_CONTROL_INFO_VALID_OFFSET                        0x0000000c
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_INFO_VALID_LSB                           12
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_INFO_VALID_MSB                           12
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_INFO_VALID_MASK                          0x00001000

#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_VALID_OFFSET                         0x0000000c
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_VALID_LSB                            13
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_VALID_MSB                            13
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_VALID_MASK                           0x00002000

#define RX_PPDU_END_USER_STATS_DATA_SEQUENCE_CONTROL_INFO_VALID_OFFSET              0x0000000c
#define RX_PPDU_END_USER_STATS_DATA_SEQUENCE_CONTROL_INFO_VALID_LSB                 14
#define RX_PPDU_END_USER_STATS_DATA_SEQUENCE_CONTROL_INFO_VALID_MSB                 14
#define RX_PPDU_END_USER_STATS_DATA_SEQUENCE_CONTROL_INFO_VALID_MASK                0x00004000

#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_NULL_VALID_OFFSET                    0x0000000c
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_NULL_VALID_LSB                       15
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_NULL_VALID_MSB                       15
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_NULL_VALID_MASK                      0x00008000

#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC1_RING_USED_OFFSET                      0x0000000c
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC1_RING_USED_LSB                         16
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC1_RING_USED_MSB                         16
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC1_RING_USED_MASK                        0x00010000

#define RX_PPDU_END_USER_STATS_RXDMA2REO_RING_USED_OFFSET                           0x0000000c
#define RX_PPDU_END_USER_STATS_RXDMA2REO_RING_USED_LSB                              17
#define RX_PPDU_END_USER_STATS_RXDMA2REO_RING_USED_MSB                              17
#define RX_PPDU_END_USER_STATS_RXDMA2REO_RING_USED_MASK                             0x00020000

#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC0_RING_USED_OFFSET                      0x0000000c
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC0_RING_USED_LSB                         18
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC0_RING_USED_MSB                         18
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC0_RING_USED_MASK                        0x00040000

#define RX_PPDU_END_USER_STATS_RXDMA2SW_RING_USED_OFFSET                            0x0000000c
#define RX_PPDU_END_USER_STATS_RXDMA2SW_RING_USED_LSB                               19
#define RX_PPDU_END_USER_STATS_RXDMA2SW_RING_USED_MSB                               19
#define RX_PPDU_END_USER_STATS_RXDMA2SW_RING_USED_MASK                              0x00080000

#define RX_PPDU_END_USER_STATS_RXDMA_RELEASE_RING_USED_OFFSET                       0x0000000c
#define RX_PPDU_END_USER_STATS_RXDMA_RELEASE_RING_USED_LSB                          20
#define RX_PPDU_END_USER_STATS_RXDMA_RELEASE_RING_USED_MSB                          20
#define RX_PPDU_END_USER_STATS_RXDMA_RELEASE_RING_USED_MASK                         0x00100000

#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_PKT_TYPE_OFFSET                     0x0000000c
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_PKT_TYPE_LSB                        21
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_PKT_TYPE_MSB                        24
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_PKT_TYPE_MASK                       0x01e00000

#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE0_RING_USED_OFFSET                   0x0000000c
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE0_RING_USED_LSB                      25
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE0_RING_USED_MSB                      25
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE0_RING_USED_MASK                     0x02000000

#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE1_RING_USED_OFFSET                   0x0000000c
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE1_RING_USED_LSB                      26
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE1_RING_USED_MSB                      26
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE1_RING_USED_MASK                     0x04000000

#define RX_PPDU_END_USER_STATS_RESERVED_3B_OFFSET                                   0x0000000c
#define RX_PPDU_END_USER_STATS_RESERVED_3B_LSB                                      27
#define RX_PPDU_END_USER_STATS_RESERVED_3B_MSB                                      31
#define RX_PPDU_END_USER_STATS_RESERVED_3B_MASK                                     0xf8000000

#define RX_PPDU_END_USER_STATS_AST_INDEX_OFFSET                                     0x00000010
#define RX_PPDU_END_USER_STATS_AST_INDEX_LSB                                        0
#define RX_PPDU_END_USER_STATS_AST_INDEX_MSB                                        15
#define RX_PPDU_END_USER_STATS_AST_INDEX_MASK                                       0x0000ffff

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_OFFSET                           0x00000010
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_LSB                              16
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MSB                              31
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MASK                             0xffff0000

#define RX_PPDU_END_USER_STATS_FIRST_DATA_SEQ_CTRL_OFFSET                           0x00000014
#define RX_PPDU_END_USER_STATS_FIRST_DATA_SEQ_CTRL_LSB                              0
#define RX_PPDU_END_USER_STATS_FIRST_DATA_SEQ_CTRL_MSB                              15
#define RX_PPDU_END_USER_STATS_FIRST_DATA_SEQ_CTRL_MASK                             0x0000ffff

#define RX_PPDU_END_USER_STATS_QOS_CONTROL_FIELD_OFFSET                             0x00000014
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_FIELD_LSB                                16
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_FIELD_MSB                                31
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_FIELD_MASK                               0xffff0000

#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_OFFSET                              0x00000018
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_LSB                                 0
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_MSB                                 31
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_MASK                                0xffffffff

#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_31_0_OFFSET                            0x0000001c
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_31_0_LSB                               0
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_31_0_MSB                               31
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_31_0_MASK                              0xffffffff

#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_63_32_OFFSET                           0x00000020
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_63_32_LSB                              0
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_63_32_MSB                              31
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_63_32_MASK                             0xffffffff

#define RX_PPDU_END_USER_STATS_UDP_MSDU_COUNT_OFFSET                                0x00000024
#define RX_PPDU_END_USER_STATS_UDP_MSDU_COUNT_LSB                                   0
#define RX_PPDU_END_USER_STATS_UDP_MSDU_COUNT_MSB                                   15
#define RX_PPDU_END_USER_STATS_UDP_MSDU_COUNT_MASK                                  0x0000ffff

#define RX_PPDU_END_USER_STATS_TCP_MSDU_COUNT_OFFSET                                0x00000024
#define RX_PPDU_END_USER_STATS_TCP_MSDU_COUNT_LSB                                   16
#define RX_PPDU_END_USER_STATS_TCP_MSDU_COUNT_MSB                                   31
#define RX_PPDU_END_USER_STATS_TCP_MSDU_COUNT_MASK                                  0xffff0000

#define RX_PPDU_END_USER_STATS_OTHER_MSDU_COUNT_OFFSET                              0x00000028
#define RX_PPDU_END_USER_STATS_OTHER_MSDU_COUNT_LSB                                 0
#define RX_PPDU_END_USER_STATS_OTHER_MSDU_COUNT_MSB                                 15
#define RX_PPDU_END_USER_STATS_OTHER_MSDU_COUNT_MASK                                0x0000ffff

#define RX_PPDU_END_USER_STATS_TCP_ACK_MSDU_COUNT_OFFSET                            0x00000028
#define RX_PPDU_END_USER_STATS_TCP_ACK_MSDU_COUNT_LSB                               16
#define RX_PPDU_END_USER_STATS_TCP_ACK_MSDU_COUNT_MSB                               31
#define RX_PPDU_END_USER_STATS_TCP_ACK_MSDU_COUNT_MASK                              0xffff0000

#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_OFFSET                     0x0000002c
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_LSB                        0
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_MSB                        31
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_MASK                       0xffffffff

#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_BITMAP_OFFSET                  0x00000030
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_BITMAP_LSB                     0
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_BITMAP_MSB                     15
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_BITMAP_MASK                    0x0000ffff

#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_OFFSET             0x00000030
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_LSB                16
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_MSB                31
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_MASK               0xffff0000

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID0_OFFSET                             0x00000034
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID0_LSB                                0
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID0_MSB                                7
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID0_MASK                               0x000000ff

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID1_OFFSET                             0x00000034
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID1_LSB                                8
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID1_MSB                                15
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID1_MASK                               0x0000ff00

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID2_OFFSET                             0x00000034
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID2_LSB                                16
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID2_MSB                                23
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID2_MASK                               0x00ff0000

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID3_OFFSET                             0x00000034
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID3_LSB                                24
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID3_MSB                                31
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID3_MASK                               0xff000000

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID4_OFFSET                             0x00000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID4_LSB                                0
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID4_MSB                                7
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID4_MASK                               0x000000ff

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID5_OFFSET                             0x00000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID5_LSB                                8
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID5_MSB                                15
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID5_MASK                               0x0000ff00

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID6_OFFSET                             0x00000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID6_LSB                                16
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID6_MSB                                23
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID6_MASK                               0x00ff0000

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID7_OFFSET                             0x00000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID7_LSB                                24
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID7_MSB                                31
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID7_MASK                               0xff000000

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID8_OFFSET                             0x0000003c
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID8_LSB                                0
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID8_MSB                                7
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID8_MASK                               0x000000ff

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID9_OFFSET                             0x0000003c
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID9_LSB                                8
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID9_MSB                                15
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID9_MASK                               0x0000ff00

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID10_OFFSET                            0x0000003c
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID10_LSB                               16
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID10_MSB                               23
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID10_MASK                              0x00ff0000

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID11_OFFSET                            0x0000003c
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID11_LSB                               24
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID11_MSB                               31
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID11_MASK                              0xff000000

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID12_OFFSET                            0x00000040
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID12_LSB                               0
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID12_MSB                               7
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID12_MASK                              0x000000ff

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID13_OFFSET                            0x00000040
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID13_LSB                               8
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID13_MSB                               15
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID13_MASK                              0x0000ff00

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID14_OFFSET                            0x00000040
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID14_LSB                               16
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID14_MSB                               23
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID14_MASK                              0x00ff0000

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID15_OFFSET                            0x00000040
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID15_LSB                               24
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID15_MSB                               31
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID15_MASK                              0xff000000

#define RX_PPDU_END_USER_STATS_MPDU_OK_BYTE_COUNT_OFFSET                            0x00000044
#define RX_PPDU_END_USER_STATS_MPDU_OK_BYTE_COUNT_LSB                               0
#define RX_PPDU_END_USER_STATS_MPDU_OK_BYTE_COUNT_MSB                               24
#define RX_PPDU_END_USER_STATS_MPDU_OK_BYTE_COUNT_MASK                              0x01ffffff

#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_6_0_OFFSET                      0x00000044
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_6_0_LSB                         25
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_6_0_MSB                         31
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_6_0_MASK                        0xfe000000

#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_ERR_COUNT_OFFSET                         0x00000048
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_ERR_COUNT_LSB                            0
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_ERR_COUNT_MSB                            24
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_ERR_COUNT_MASK                           0x01ffffff

#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_13_7_OFFSET                     0x00000048
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_13_7_LSB                        25
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_13_7_MSB                        31
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_13_7_MASK                       0xfe000000

#define RX_PPDU_END_USER_STATS_MPDU_ERR_BYTE_COUNT_OFFSET                           0x0000004c
#define RX_PPDU_END_USER_STATS_MPDU_ERR_BYTE_COUNT_LSB                              0
#define RX_PPDU_END_USER_STATS_MPDU_ERR_BYTE_COUNT_MSB                              24
#define RX_PPDU_END_USER_STATS_MPDU_ERR_BYTE_COUNT_MASK                             0x01ffffff

#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_20_14_OFFSET                    0x0000004c
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_20_14_LSB                       25
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_20_14_MSB                       31
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_20_14_MASK                      0xfe000000

#define RX_PPDU_END_USER_STATS_NON_CONSECUTIVE_DELIMITER_ERR_OFFSET                 0x00000050
#define RX_PPDU_END_USER_STATS_NON_CONSECUTIVE_DELIMITER_ERR_LSB                    0
#define RX_PPDU_END_USER_STATS_NON_CONSECUTIVE_DELIMITER_ERR_MSB                    15
#define RX_PPDU_END_USER_STATS_NON_CONSECUTIVE_DELIMITER_ERR_MASK                   0x0000ffff

#define RX_PPDU_END_USER_STATS_RETRIED_MSDU_COUNT_OFFSET                            0x00000050
#define RX_PPDU_END_USER_STATS_RETRIED_MSDU_COUNT_LSB                               16
#define RX_PPDU_END_USER_STATS_RETRIED_MSDU_COUNT_MSB                               31
#define RX_PPDU_END_USER_STATS_RETRIED_MSDU_COUNT_MASK                              0xffff0000

#define RX_PPDU_END_USER_STATS_HT_CONTROL_NULL_FIELD_OFFSET                         0x00000054
#define RX_PPDU_END_USER_STATS_HT_CONTROL_NULL_FIELD_LSB                            0
#define RX_PPDU_END_USER_STATS_HT_CONTROL_NULL_FIELD_MSB                            31
#define RX_PPDU_END_USER_STATS_HT_CONTROL_NULL_FIELD_MASK                           0xffffffff

#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_OFFSET                 0x00000058
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_LSB                    0
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_MSB                    31
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_MASK                   0xffffffff

#define RX_PPDU_END_USER_STATS_CORRUPTED_DUE_TO_FIFO_DELAY_OFFSET                   0x0000005c
#define RX_PPDU_END_USER_STATS_CORRUPTED_DUE_TO_FIFO_DELAY_LSB                      0
#define RX_PPDU_END_USER_STATS_CORRUPTED_DUE_TO_FIFO_DELAY_MSB                      0
#define RX_PPDU_END_USER_STATS_CORRUPTED_DUE_TO_FIFO_DELAY_MASK                     0x00000001

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_NULL_VALID_OFFSET                 0x0000005c
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_NULL_VALID_LSB                    1
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_NULL_VALID_MSB                    1
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_NULL_VALID_MASK                   0x00000002

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_NULL_OFFSET                      0x0000005c
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_NULL_LSB                         2
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_NULL_MSB                         17
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_NULL_MASK                        0x0003fffc

#define RX_PPDU_END_USER_STATS_RETRIED_MPDU_COUNT_OFFSET                            0x0000005c
#define RX_PPDU_END_USER_STATS_RETRIED_MPDU_COUNT_LSB                               18
#define RX_PPDU_END_USER_STATS_RETRIED_MPDU_COUNT_MSB                               28
#define RX_PPDU_END_USER_STATS_RETRIED_MPDU_COUNT_MASK                              0x1ffc0000

#define RX_PPDU_END_USER_STATS_RESERVED_23A_OFFSET                                  0x0000005c
#define RX_PPDU_END_USER_STATS_RESERVED_23A_LSB                                     29
#define RX_PPDU_END_USER_STATS_RESERVED_23A_MSB                                     31
#define RX_PPDU_END_USER_STATS_RESERVED_23A_MASK                                    0xe0000000

#define RX_PPDU_END_USER_STATS_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET                 0x00000060
#define RX_PPDU_END_USER_STATS_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB                    0
#define RX_PPDU_END_USER_STATS_RXPCU_MPDU_FILTER_IN_CATEGORY_MSB                    1
#define RX_PPDU_END_USER_STATS_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK                   0x00000003

#define RX_PPDU_END_USER_STATS_SW_FRAME_GROUP_ID_OFFSET                             0x00000060
#define RX_PPDU_END_USER_STATS_SW_FRAME_GROUP_ID_LSB                                2
#define RX_PPDU_END_USER_STATS_SW_FRAME_GROUP_ID_MSB                                8
#define RX_PPDU_END_USER_STATS_SW_FRAME_GROUP_ID_MASK                               0x000001fc

#define RX_PPDU_END_USER_STATS_RESERVED_24A_OFFSET                                  0x00000060
#define RX_PPDU_END_USER_STATS_RESERVED_24A_LSB                                     9
#define RX_PPDU_END_USER_STATS_RESERVED_24A_MSB                                     12
#define RX_PPDU_END_USER_STATS_RESERVED_24A_MASK                                    0x00001e00

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_MGMT_CTRL_VALID_OFFSET            0x00000060
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_MGMT_CTRL_VALID_LSB               13
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_MGMT_CTRL_VALID_MSB               13
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_MGMT_CTRL_VALID_MASK              0x00002000

#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_VALID_OFFSET                            0x00000060
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_VALID_LSB                               14
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_VALID_MSB                               14
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_VALID_MASK                              0x00004000

#define RX_PPDU_END_USER_STATS_MCAST_BCAST_OFFSET                                   0x00000060
#define RX_PPDU_END_USER_STATS_MCAST_BCAST_LSB                                      15
#define RX_PPDU_END_USER_STATS_MCAST_BCAST_MSB                                      15
#define RX_PPDU_END_USER_STATS_MCAST_BCAST_MASK                                     0x00008000

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MGMT_CTRL_OFFSET                 0x00000060
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MGMT_CTRL_LSB                    16
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MGMT_CTRL_MSB                    31
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MGMT_CTRL_MASK                   0xffff0000

#define RX_PPDU_END_USER_STATS_USER_PPDU_LEN_OFFSET                                 0x00000064
#define RX_PPDU_END_USER_STATS_USER_PPDU_LEN_LSB                                    0
#define RX_PPDU_END_USER_STATS_USER_PPDU_LEN_MSB                                    23
#define RX_PPDU_END_USER_STATS_USER_PPDU_LEN_MASK                                   0x00ffffff

#define RX_PPDU_END_USER_STATS_RESERVED_25A_OFFSET                                  0x00000064
#define RX_PPDU_END_USER_STATS_RESERVED_25A_LSB                                     24
#define RX_PPDU_END_USER_STATS_RESERVED_25A_MSB                                     31
#define RX_PPDU_END_USER_STATS_RESERVED_25A_MASK                                    0xff000000

#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_31_0_OFFSET                             0x00000068
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_31_0_LSB                                0
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_31_0_MSB                                31
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_31_0_MASK                               0xffffffff

#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_47_32_OFFSET                            0x0000006c
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_47_32_LSB                               0
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_47_32_MSB                               15
#define RX_PPDU_END_USER_STATS_MAC_ADDR_AD2_47_32_MASK                              0x0000ffff

#define RX_PPDU_END_USER_STATS_AMSDU_MSDU_COUNT_OFFSET                              0x0000006c
#define RX_PPDU_END_USER_STATS_AMSDU_MSDU_COUNT_LSB                                 16
#define RX_PPDU_END_USER_STATS_AMSDU_MSDU_COUNT_MSB                                 31
#define RX_PPDU_END_USER_STATS_AMSDU_MSDU_COUNT_MASK                                0xffff0000

#define RX_PPDU_END_USER_STATS_NON_AMSDU_MSDU_COUNT_OFFSET                          0x00000070
#define RX_PPDU_END_USER_STATS_NON_AMSDU_MSDU_COUNT_LSB                             0
#define RX_PPDU_END_USER_STATS_NON_AMSDU_MSDU_COUNT_MSB                             15
#define RX_PPDU_END_USER_STATS_NON_AMSDU_MSDU_COUNT_MASK                            0x0000ffff

#define RX_PPDU_END_USER_STATS_UCAST_MSDU_COUNT_OFFSET                              0x00000070
#define RX_PPDU_END_USER_STATS_UCAST_MSDU_COUNT_LSB                                 16
#define RX_PPDU_END_USER_STATS_UCAST_MSDU_COUNT_MSB                                 31
#define RX_PPDU_END_USER_STATS_UCAST_MSDU_COUNT_MASK                                0xffff0000

#define RX_PPDU_END_USER_STATS_BCAST_MSDU_COUNT_OFFSET                              0x00000074
#define RX_PPDU_END_USER_STATS_BCAST_MSDU_COUNT_LSB                                 0
#define RX_PPDU_END_USER_STATS_BCAST_MSDU_COUNT_MSB                                 15
#define RX_PPDU_END_USER_STATS_BCAST_MSDU_COUNT_MASK                                0x0000ffff

#define RX_PPDU_END_USER_STATS_MCAST_BCAST_MSDU_COUNT_OFFSET                        0x00000074
#define RX_PPDU_END_USER_STATS_MCAST_BCAST_MSDU_COUNT_LSB                           16
#define RX_PPDU_END_USER_STATS_MCAST_BCAST_MSDU_COUNT_MSB                           31
#define RX_PPDU_END_USER_STATS_MCAST_BCAST_MSDU_COUNT_MASK                          0xffff0000

#endif
