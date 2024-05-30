
/* Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
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
#if !defined(__ASSEMBLER__)
#endif

#include "rx_rxpcu_classification_overview.h"
#define NUM_OF_DWORDS_RX_PPDU_END_USER_STATS 24

#define NUM_OF_QWORDS_RX_PPDU_END_USER_STATS 12


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
#endif
};


 


 

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_LSB   0
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_MSB   0
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_MASK  0x0000000000000001


 

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_FCS_OK_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_FCS_OK_LSB 1
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_FCS_OK_MSB 1
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MPDUS_FCS_OK_MASK 0x0000000000000002


 

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_LSB 2
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_MSB 2
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_MASK 0x0000000000000004


 

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_FCS_OK_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_FCS_OK_LSB 3
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_FCS_OK_MSB 3
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_DIRECT_MPDUS_FCS_OK_MASK 0x0000000000000008


 

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_LSB 4
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_MSB 4
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_MASK 0x0000000000000010


 

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_FCS_OK_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_FCS_OK_LSB 5
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_FCS_OK_MSB 5
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_MONITOR_OTHER_MPDUS_FCS_OK_MASK 0x0000000000000020


 

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHYRX_ABORT_RECEIVED_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHYRX_ABORT_RECEIVED_LSB 6
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHYRX_ABORT_RECEIVED_MSB 6
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHYRX_ABORT_RECEIVED_MASK 0x0000000000000040


 

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_LSB 7
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_MSB 7
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_MASK 0x0000000000000080


 

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_OFFSET 0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_LSB 8
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_MSB 8
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_MASK 0x0000000000000100


 

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_RESERVED_0_OFFSET       0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_RESERVED_0_LSB          9
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_RESERVED_0_MSB          15
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_RESERVED_0_MASK         0x000000000000fe00


 

#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHY_PPDU_ID_OFFSET      0x0000000000000000
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHY_PPDU_ID_LSB         16
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHY_PPDU_ID_MSB         31
#define RX_PPDU_END_USER_STATS_RXPCU_CLASSIFICATION_DETAILS_PHY_PPDU_ID_MASK        0x00000000ffff0000


 

#define RX_PPDU_END_USER_STATS_STA_FULL_AID_OFFSET                                  0x0000000000000000
#define RX_PPDU_END_USER_STATS_STA_FULL_AID_LSB                                     32
#define RX_PPDU_END_USER_STATS_STA_FULL_AID_MSB                                     44
#define RX_PPDU_END_USER_STATS_STA_FULL_AID_MASK                                    0x00001fff00000000


 

#define RX_PPDU_END_USER_STATS_MCS_OFFSET                                           0x0000000000000000
#define RX_PPDU_END_USER_STATS_MCS_LSB                                              45
#define RX_PPDU_END_USER_STATS_MCS_MSB                                              48
#define RX_PPDU_END_USER_STATS_MCS_MASK                                             0x0001e00000000000


 

#define RX_PPDU_END_USER_STATS_NSS_OFFSET                                           0x0000000000000000
#define RX_PPDU_END_USER_STATS_NSS_LSB                                              49
#define RX_PPDU_END_USER_STATS_NSS_MSB                                              51
#define RX_PPDU_END_USER_STATS_NSS_MASK                                             0x000e000000000000


 

#define RX_PPDU_END_USER_STATS_EXPECTED_RESPONSE_ACK_OR_BA_OFFSET                   0x0000000000000000
#define RX_PPDU_END_USER_STATS_EXPECTED_RESPONSE_ACK_OR_BA_LSB                      52
#define RX_PPDU_END_USER_STATS_EXPECTED_RESPONSE_ACK_OR_BA_MSB                      52
#define RX_PPDU_END_USER_STATS_EXPECTED_RESPONSE_ACK_OR_BA_MASK                     0x0010000000000000


 

#define RX_PPDU_END_USER_STATS_RESERVED_1A_OFFSET                                   0x0000000000000000
#define RX_PPDU_END_USER_STATS_RESERVED_1A_LSB                                      53
#define RX_PPDU_END_USER_STATS_RESERVED_1A_MSB                                      63
#define RX_PPDU_END_USER_STATS_RESERVED_1A_MASK                                     0xffe0000000000000


 

#define RX_PPDU_END_USER_STATS_SW_PEER_ID_OFFSET                                    0x0000000000000008
#define RX_PPDU_END_USER_STATS_SW_PEER_ID_LSB                                       0
#define RX_PPDU_END_USER_STATS_SW_PEER_ID_MSB                                       15
#define RX_PPDU_END_USER_STATS_SW_PEER_ID_MASK                                      0x000000000000ffff


 

#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_ERR_OFFSET                              0x0000000000000008
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_ERR_LSB                                 16
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_ERR_MSB                                 26
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_ERR_MASK                                0x0000000007ff0000


 

#define RX_PPDU_END_USER_STATS_SW2RXDMA0_BUF_SOURCE_USED_OFFSET                     0x0000000000000008
#define RX_PPDU_END_USER_STATS_SW2RXDMA0_BUF_SOURCE_USED_LSB                        27
#define RX_PPDU_END_USER_STATS_SW2RXDMA0_BUF_SOURCE_USED_MSB                        27
#define RX_PPDU_END_USER_STATS_SW2RXDMA0_BUF_SOURCE_USED_MASK                       0x0000000008000000


 

#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC0_BUF_SOURCE_USED_OFFSET                0x0000000000000008
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC0_BUF_SOURCE_USED_LSB                   28
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC0_BUF_SOURCE_USED_MSB                   28
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC0_BUF_SOURCE_USED_MASK                  0x0000000010000000


 

#define RX_PPDU_END_USER_STATS_SW2RXDMA1_BUF_SOURCE_USED_OFFSET                     0x0000000000000008
#define RX_PPDU_END_USER_STATS_SW2RXDMA1_BUF_SOURCE_USED_LSB                        29
#define RX_PPDU_END_USER_STATS_SW2RXDMA1_BUF_SOURCE_USED_MSB                        29
#define RX_PPDU_END_USER_STATS_SW2RXDMA1_BUF_SOURCE_USED_MASK                       0x0000000020000000


 

#define RX_PPDU_END_USER_STATS_SW2RXDMA_EXCEPTION_BUF_SOURCE_USED_OFFSET            0x0000000000000008
#define RX_PPDU_END_USER_STATS_SW2RXDMA_EXCEPTION_BUF_SOURCE_USED_LSB               30
#define RX_PPDU_END_USER_STATS_SW2RXDMA_EXCEPTION_BUF_SOURCE_USED_MSB               30
#define RX_PPDU_END_USER_STATS_SW2RXDMA_EXCEPTION_BUF_SOURCE_USED_MASK              0x0000000040000000


 

#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC1_BUF_SOURCE_USED_OFFSET                0x0000000000000008
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC1_BUF_SOURCE_USED_LSB                   31
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC1_BUF_SOURCE_USED_MSB                   31
#define RX_PPDU_END_USER_STATS_FW2RXDMA_PMAC1_BUF_SOURCE_USED_MASK                  0x0000000080000000


 

#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_OK_OFFSET                               0x0000000000000008
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_OK_LSB                                  32
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_OK_MSB                                  42
#define RX_PPDU_END_USER_STATS_MPDU_CNT_FCS_OK_MASK                                 0x000007ff00000000


 

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_VALID_OFFSET                      0x0000000000000008
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_VALID_LSB                         43
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_VALID_MSB                         43
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_VALID_MASK                        0x0000080000000000


 

#define RX_PPDU_END_USER_STATS_QOS_CONTROL_INFO_VALID_OFFSET                        0x0000000000000008
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_INFO_VALID_LSB                           44
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_INFO_VALID_MSB                           44
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_INFO_VALID_MASK                          0x0000100000000000


 

#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_VALID_OFFSET                         0x0000000000000008
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_VALID_LSB                            45
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_VALID_MSB                            45
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_VALID_MASK                           0x0000200000000000


 

#define RX_PPDU_END_USER_STATS_DATA_SEQUENCE_CONTROL_INFO_VALID_OFFSET              0x0000000000000008
#define RX_PPDU_END_USER_STATS_DATA_SEQUENCE_CONTROL_INFO_VALID_LSB                 46
#define RX_PPDU_END_USER_STATS_DATA_SEQUENCE_CONTROL_INFO_VALID_MSB                 46
#define RX_PPDU_END_USER_STATS_DATA_SEQUENCE_CONTROL_INFO_VALID_MASK                0x0000400000000000


 

#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_NULL_VALID_OFFSET                    0x0000000000000008
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_NULL_VALID_LSB                       47
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_NULL_VALID_MSB                       47
#define RX_PPDU_END_USER_STATS_HT_CONTROL_INFO_NULL_VALID_MASK                      0x0000800000000000


 

#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC1_RING_USED_OFFSET                      0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC1_RING_USED_LSB                         48
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC1_RING_USED_MSB                         48
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC1_RING_USED_MASK                        0x0001000000000000


 

#define RX_PPDU_END_USER_STATS_RXDMA2REO_RING_USED_OFFSET                           0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA2REO_RING_USED_LSB                              49
#define RX_PPDU_END_USER_STATS_RXDMA2REO_RING_USED_MSB                              49
#define RX_PPDU_END_USER_STATS_RXDMA2REO_RING_USED_MASK                             0x0002000000000000


 

#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC0_RING_USED_OFFSET                      0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC0_RING_USED_LSB                         50
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC0_RING_USED_MSB                         50
#define RX_PPDU_END_USER_STATS_RXDMA2FW_PMAC0_RING_USED_MASK                        0x0004000000000000


 

#define RX_PPDU_END_USER_STATS_RXDMA2SW_RING_USED_OFFSET                            0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA2SW_RING_USED_LSB                               51
#define RX_PPDU_END_USER_STATS_RXDMA2SW_RING_USED_MSB                               51
#define RX_PPDU_END_USER_STATS_RXDMA2SW_RING_USED_MASK                              0x0008000000000000


 

#define RX_PPDU_END_USER_STATS_RXDMA_RELEASE_RING_USED_OFFSET                       0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA_RELEASE_RING_USED_LSB                          52
#define RX_PPDU_END_USER_STATS_RXDMA_RELEASE_RING_USED_MSB                          52
#define RX_PPDU_END_USER_STATS_RXDMA_RELEASE_RING_USED_MASK                         0x0010000000000000


 

#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_PKT_TYPE_OFFSET                     0x0000000000000008
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_PKT_TYPE_LSB                        53
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_PKT_TYPE_MSB                        56
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_PKT_TYPE_MASK                       0x01e0000000000000


 

#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE0_RING_USED_OFFSET                   0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE0_RING_USED_LSB                      57
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE0_RING_USED_MSB                      57
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE0_RING_USED_MASK                     0x0200000000000000


 

#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE1_RING_USED_OFFSET                   0x0000000000000008
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE1_RING_USED_LSB                      58
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE1_RING_USED_MSB                      58
#define RX_PPDU_END_USER_STATS_RXDMA2REO_REMOTE1_RING_USED_MASK                     0x0400000000000000


 

#define RX_PPDU_END_USER_STATS_RESERVED_3B_OFFSET                                   0x0000000000000008
#define RX_PPDU_END_USER_STATS_RESERVED_3B_LSB                                      59
#define RX_PPDU_END_USER_STATS_RESERVED_3B_MSB                                      63
#define RX_PPDU_END_USER_STATS_RESERVED_3B_MASK                                     0xf800000000000000


 

#define RX_PPDU_END_USER_STATS_AST_INDEX_OFFSET                                     0x0000000000000010
#define RX_PPDU_END_USER_STATS_AST_INDEX_LSB                                        0
#define RX_PPDU_END_USER_STATS_AST_INDEX_MSB                                        15
#define RX_PPDU_END_USER_STATS_AST_INDEX_MASK                                       0x000000000000ffff


 

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_OFFSET                           0x0000000000000010
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_LSB                              16
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MSB                              31
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_MASK                             0x00000000ffff0000


 

#define RX_PPDU_END_USER_STATS_FIRST_DATA_SEQ_CTRL_OFFSET                           0x0000000000000010
#define RX_PPDU_END_USER_STATS_FIRST_DATA_SEQ_CTRL_LSB                              32
#define RX_PPDU_END_USER_STATS_FIRST_DATA_SEQ_CTRL_MSB                              47
#define RX_PPDU_END_USER_STATS_FIRST_DATA_SEQ_CTRL_MASK                             0x0000ffff00000000


 

#define RX_PPDU_END_USER_STATS_QOS_CONTROL_FIELD_OFFSET                             0x0000000000000010
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_FIELD_LSB                                48
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_FIELD_MSB                                63
#define RX_PPDU_END_USER_STATS_QOS_CONTROL_FIELD_MASK                               0xffff000000000000


 

#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_OFFSET                              0x0000000000000018
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_LSB                                 0
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_MSB                                 31
#define RX_PPDU_END_USER_STATS_HT_CONTROL_FIELD_MASK                                0x00000000ffffffff


 

#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_31_0_OFFSET                            0x0000000000000018
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_31_0_LSB                               32
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_31_0_MSB                               63
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_31_0_MASK                              0xffffffff00000000


 

#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_63_32_OFFSET                           0x0000000000000020
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_63_32_LSB                              0
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_63_32_MSB                              31
#define RX_PPDU_END_USER_STATS_FCS_OK_BITMAP_63_32_MASK                             0x00000000ffffffff


 

#define RX_PPDU_END_USER_STATS_UDP_MSDU_COUNT_OFFSET                                0x0000000000000020
#define RX_PPDU_END_USER_STATS_UDP_MSDU_COUNT_LSB                                   32
#define RX_PPDU_END_USER_STATS_UDP_MSDU_COUNT_MSB                                   47
#define RX_PPDU_END_USER_STATS_UDP_MSDU_COUNT_MASK                                  0x0000ffff00000000


 

#define RX_PPDU_END_USER_STATS_TCP_MSDU_COUNT_OFFSET                                0x0000000000000020
#define RX_PPDU_END_USER_STATS_TCP_MSDU_COUNT_LSB                                   48
#define RX_PPDU_END_USER_STATS_TCP_MSDU_COUNT_MSB                                   63
#define RX_PPDU_END_USER_STATS_TCP_MSDU_COUNT_MASK                                  0xffff000000000000


 

#define RX_PPDU_END_USER_STATS_OTHER_MSDU_COUNT_OFFSET                              0x0000000000000028
#define RX_PPDU_END_USER_STATS_OTHER_MSDU_COUNT_LSB                                 0
#define RX_PPDU_END_USER_STATS_OTHER_MSDU_COUNT_MSB                                 15
#define RX_PPDU_END_USER_STATS_OTHER_MSDU_COUNT_MASK                                0x000000000000ffff


 

#define RX_PPDU_END_USER_STATS_TCP_ACK_MSDU_COUNT_OFFSET                            0x0000000000000028
#define RX_PPDU_END_USER_STATS_TCP_ACK_MSDU_COUNT_LSB                               16
#define RX_PPDU_END_USER_STATS_TCP_ACK_MSDU_COUNT_MSB                               31
#define RX_PPDU_END_USER_STATS_TCP_ACK_MSDU_COUNT_MASK                              0x00000000ffff0000


 

#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_OFFSET                     0x0000000000000028
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_LSB                        32
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_MSB                        63
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_MASK                       0xffffffff00000000


 

#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_BITMAP_OFFSET                  0x0000000000000030
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_BITMAP_LSB                     0
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_BITMAP_MSB                     15
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_BITMAP_MASK                    0x000000000000ffff


 

#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_OFFSET             0x0000000000000030
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_LSB                16
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_MSB                31
#define RX_PPDU_END_USER_STATS_RECEIVED_QOS_DATA_TID_EOSP_BITMAP_MASK               0x00000000ffff0000


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID0_OFFSET                             0x0000000000000030
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID0_LSB                                32
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID0_MSB                                39
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID0_MASK                               0x000000ff00000000


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID1_OFFSET                             0x0000000000000030
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID1_LSB                                40
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID1_MSB                                47
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID1_MASK                               0x0000ff0000000000


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID2_OFFSET                             0x0000000000000030
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID2_LSB                                48
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID2_MSB                                55
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID2_MASK                               0x00ff000000000000


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID3_OFFSET                             0x0000000000000030
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID3_LSB                                56
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID3_MSB                                63
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID3_MASK                               0xff00000000000000


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID4_OFFSET                             0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID4_LSB                                0
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID4_MSB                                7
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID4_MASK                               0x00000000000000ff


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID5_OFFSET                             0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID5_LSB                                8
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID5_MSB                                15
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID5_MASK                               0x000000000000ff00


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID6_OFFSET                             0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID6_LSB                                16
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID6_MSB                                23
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID6_MASK                               0x0000000000ff0000


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID7_OFFSET                             0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID7_LSB                                24
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID7_MSB                                31
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID7_MASK                               0x00000000ff000000


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID8_OFFSET                             0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID8_LSB                                32
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID8_MSB                                39
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID8_MASK                               0x000000ff00000000


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID9_OFFSET                             0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID9_LSB                                40
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID9_MSB                                47
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID9_MASK                               0x0000ff0000000000


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID10_OFFSET                            0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID10_LSB                               48
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID10_MSB                               55
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID10_MASK                              0x00ff000000000000


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID11_OFFSET                            0x0000000000000038
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID11_LSB                               56
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID11_MSB                               63
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID11_MASK                              0xff00000000000000


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID12_OFFSET                            0x0000000000000040
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID12_LSB                               0
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID12_MSB                               7
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID12_MASK                              0x00000000000000ff


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID13_OFFSET                            0x0000000000000040
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID13_LSB                               8
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID13_MSB                               15
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID13_MASK                              0x000000000000ff00


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID14_OFFSET                            0x0000000000000040
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID14_LSB                               16
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID14_MSB                               23
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID14_MASK                              0x0000000000ff0000


 

#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID15_OFFSET                            0x0000000000000040
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID15_LSB                               24
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID15_MSB                               31
#define RX_PPDU_END_USER_STATS_QOSCTRL_15_8_TID15_MASK                              0x00000000ff000000


 

#define RX_PPDU_END_USER_STATS_MPDU_OK_BYTE_COUNT_OFFSET                            0x0000000000000040
#define RX_PPDU_END_USER_STATS_MPDU_OK_BYTE_COUNT_LSB                               32
#define RX_PPDU_END_USER_STATS_MPDU_OK_BYTE_COUNT_MSB                               56
#define RX_PPDU_END_USER_STATS_MPDU_OK_BYTE_COUNT_MASK                              0x01ffffff00000000


 

#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_6_0_OFFSET                      0x0000000000000040
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_6_0_LSB                         57
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_6_0_MSB                         63
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_6_0_MASK                        0xfe00000000000000


 

#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_ERR_COUNT_OFFSET                         0x0000000000000048
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_ERR_COUNT_LSB                            0
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_ERR_COUNT_MSB                            24
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_ERR_COUNT_MASK                           0x0000000001ffffff


 

#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_13_7_OFFSET                     0x0000000000000048
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_13_7_LSB                        25
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_13_7_MSB                        31
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_13_7_MASK                       0x00000000fe000000


 

#define RX_PPDU_END_USER_STATS_MPDU_ERR_BYTE_COUNT_OFFSET                           0x0000000000000048
#define RX_PPDU_END_USER_STATS_MPDU_ERR_BYTE_COUNT_LSB                              32
#define RX_PPDU_END_USER_STATS_MPDU_ERR_BYTE_COUNT_MSB                              56
#define RX_PPDU_END_USER_STATS_MPDU_ERR_BYTE_COUNT_MASK                             0x01ffffff00000000


 

#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_20_14_OFFSET                    0x0000000000000048
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_20_14_LSB                       57
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_20_14_MSB                       63
#define RX_PPDU_END_USER_STATS_AMPDU_DELIM_OK_COUNT_20_14_MASK                      0xfe00000000000000


 

#define RX_PPDU_END_USER_STATS_NON_CONSECUTIVE_DELIMITER_ERR_OFFSET                 0x0000000000000050
#define RX_PPDU_END_USER_STATS_NON_CONSECUTIVE_DELIMITER_ERR_LSB                    0
#define RX_PPDU_END_USER_STATS_NON_CONSECUTIVE_DELIMITER_ERR_MSB                    15
#define RX_PPDU_END_USER_STATS_NON_CONSECUTIVE_DELIMITER_ERR_MASK                   0x000000000000ffff


 

#define RX_PPDU_END_USER_STATS_RETRIED_MSDU_COUNT_OFFSET                            0x0000000000000050
#define RX_PPDU_END_USER_STATS_RETRIED_MSDU_COUNT_LSB                               16
#define RX_PPDU_END_USER_STATS_RETRIED_MSDU_COUNT_MSB                               31
#define RX_PPDU_END_USER_STATS_RETRIED_MSDU_COUNT_MASK                              0x00000000ffff0000


 

#define RX_PPDU_END_USER_STATS_HT_CONTROL_NULL_FIELD_OFFSET                         0x0000000000000050
#define RX_PPDU_END_USER_STATS_HT_CONTROL_NULL_FIELD_LSB                            32
#define RX_PPDU_END_USER_STATS_HT_CONTROL_NULL_FIELD_MSB                            63
#define RX_PPDU_END_USER_STATS_HT_CONTROL_NULL_FIELD_MASK                           0xffffffff00000000


 

#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_OFFSET                 0x0000000000000058
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_LSB                    0
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_MSB                    31
#define RX_PPDU_END_USER_STATS_SW_RESPONSE_REFERENCE_PTR_EXT_MASK                   0x00000000ffffffff


 

#define RX_PPDU_END_USER_STATS_CORRUPTED_DUE_TO_FIFO_DELAY_OFFSET                   0x0000000000000058
#define RX_PPDU_END_USER_STATS_CORRUPTED_DUE_TO_FIFO_DELAY_LSB                      32
#define RX_PPDU_END_USER_STATS_CORRUPTED_DUE_TO_FIFO_DELAY_MSB                      32
#define RX_PPDU_END_USER_STATS_CORRUPTED_DUE_TO_FIFO_DELAY_MASK                     0x0000000100000000


 

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_NULL_VALID_OFFSET                 0x0000000000000058
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_NULL_VALID_LSB                    33
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_NULL_VALID_MSB                    33
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_INFO_NULL_VALID_MASK                   0x0000000200000000


 

#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_NULL_OFFSET                      0x0000000000000058
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_NULL_LSB                         34
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_NULL_MSB                         49
#define RX_PPDU_END_USER_STATS_FRAME_CONTROL_FIELD_NULL_MASK                        0x0003fffc00000000


 

#define RX_PPDU_END_USER_STATS_RETRIED_MPDU_COUNT_OFFSET                            0x0000000000000058
#define RX_PPDU_END_USER_STATS_RETRIED_MPDU_COUNT_LSB                               50
#define RX_PPDU_END_USER_STATS_RETRIED_MPDU_COUNT_MSB                               60
#define RX_PPDU_END_USER_STATS_RETRIED_MPDU_COUNT_MASK                              0x1ffc000000000000


 

#define RX_PPDU_END_USER_STATS_RESERVED_23A_OFFSET                                  0x0000000000000058
#define RX_PPDU_END_USER_STATS_RESERVED_23A_LSB                                     61
#define RX_PPDU_END_USER_STATS_RESERVED_23A_MSB                                     63
#define RX_PPDU_END_USER_STATS_RESERVED_23A_MASK                                    0xe000000000000000



#endif    
