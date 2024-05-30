
/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */



#ifndef _RX_ATTENTION_H_
#define _RX_ATTENTION_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_ATTENTION 3

struct rx_attention {
             uint32_t rxpcu_mpdu_filter_in_category   :  2,
                      sw_frame_group_id               :  7,
                      reserved_0                      :  7,
                      phy_ppdu_id                     : 16;
             uint32_t first_mpdu                      :  1,
                      reserved_1a                     :  1,
                      mcast_bcast                     :  1,
                      ast_index_not_found             :  1,
                      ast_index_timeout               :  1,
                      power_mgmt                      :  1,
                      non_qos                         :  1,
                      null_data                       :  1,
                      mgmt_type                       :  1,
                      ctrl_type                       :  1,
                      more_data                       :  1,
                      eosp                            :  1,
                      a_msdu_error                    :  1,
                      fragment_flag                   :  1,
                      order                           :  1,
                      cce_match                       :  1,
                      overflow_err                    :  1,
                      msdu_length_err                 :  1,
                      tcp_udp_chksum_fail             :  1,
                      ip_chksum_fail                  :  1,
                      sa_idx_invalid                  :  1,
                      da_idx_invalid                  :  1,
                      reserved_1b                     :  1,
                      rx_in_tx_decrypt_byp            :  1,
                      encrypt_required                :  1,
                      directed                        :  1,
                      buffer_fragment                 :  1,
                      mpdu_length_err                 :  1,
                      tkip_mic_err                    :  1,
                      decrypt_err                     :  1,
                      unencrypted_frame_err           :  1,
                      fcs_err                         :  1;
             uint32_t flow_idx_timeout                :  1,
                      flow_idx_invalid                :  1,
                      wifi_parser_error               :  1,
                      amsdu_parser_error              :  1,
                      sa_idx_timeout                  :  1,
                      da_idx_timeout                  :  1,
                      msdu_limit_error                :  1,
                      da_is_valid                     :  1,
                      da_is_mcbc                      :  1,
                      sa_is_valid                     :  1,
                      decrypt_status_code             :  3,
                      rx_bitmap_not_updated           :  1,
                      reserved_2                      : 17,
                      msdu_done                       :  1;
};

#define RX_ATTENTION_0_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET          0x00000000
#define RX_ATTENTION_0_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB             0
#define RX_ATTENTION_0_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK            0x00000003

#define RX_ATTENTION_0_SW_FRAME_GROUP_ID_OFFSET                      0x00000000
#define RX_ATTENTION_0_SW_FRAME_GROUP_ID_LSB                         2
#define RX_ATTENTION_0_SW_FRAME_GROUP_ID_MASK                        0x000001fc

#define RX_ATTENTION_0_RESERVED_0_OFFSET                             0x00000000
#define RX_ATTENTION_0_RESERVED_0_LSB                                9
#define RX_ATTENTION_0_RESERVED_0_MASK                               0x0000fe00

#define RX_ATTENTION_0_PHY_PPDU_ID_OFFSET                            0x00000000
#define RX_ATTENTION_0_PHY_PPDU_ID_LSB                               16
#define RX_ATTENTION_0_PHY_PPDU_ID_MASK                              0xffff0000

#define RX_ATTENTION_1_FIRST_MPDU_OFFSET                             0x00000004
#define RX_ATTENTION_1_FIRST_MPDU_LSB                                0
#define RX_ATTENTION_1_FIRST_MPDU_MASK                               0x00000001

#define RX_ATTENTION_1_RESERVED_1A_OFFSET                            0x00000004
#define RX_ATTENTION_1_RESERVED_1A_LSB                               1
#define RX_ATTENTION_1_RESERVED_1A_MASK                              0x00000002

#define RX_ATTENTION_1_MCAST_BCAST_OFFSET                            0x00000004
#define RX_ATTENTION_1_MCAST_BCAST_LSB                               2
#define RX_ATTENTION_1_MCAST_BCAST_MASK                              0x00000004

#define RX_ATTENTION_1_AST_INDEX_NOT_FOUND_OFFSET                    0x00000004
#define RX_ATTENTION_1_AST_INDEX_NOT_FOUND_LSB                       3
#define RX_ATTENTION_1_AST_INDEX_NOT_FOUND_MASK                      0x00000008

#define RX_ATTENTION_1_AST_INDEX_TIMEOUT_OFFSET                      0x00000004
#define RX_ATTENTION_1_AST_INDEX_TIMEOUT_LSB                         4
#define RX_ATTENTION_1_AST_INDEX_TIMEOUT_MASK                        0x00000010

#define RX_ATTENTION_1_POWER_MGMT_OFFSET                             0x00000004
#define RX_ATTENTION_1_POWER_MGMT_LSB                                5
#define RX_ATTENTION_1_POWER_MGMT_MASK                               0x00000020

#define RX_ATTENTION_1_NON_QOS_OFFSET                                0x00000004
#define RX_ATTENTION_1_NON_QOS_LSB                                   6
#define RX_ATTENTION_1_NON_QOS_MASK                                  0x00000040

#define RX_ATTENTION_1_NULL_DATA_OFFSET                              0x00000004
#define RX_ATTENTION_1_NULL_DATA_LSB                                 7
#define RX_ATTENTION_1_NULL_DATA_MASK                                0x00000080

#define RX_ATTENTION_1_MGMT_TYPE_OFFSET                              0x00000004
#define RX_ATTENTION_1_MGMT_TYPE_LSB                                 8
#define RX_ATTENTION_1_MGMT_TYPE_MASK                                0x00000100

#define RX_ATTENTION_1_CTRL_TYPE_OFFSET                              0x00000004
#define RX_ATTENTION_1_CTRL_TYPE_LSB                                 9
#define RX_ATTENTION_1_CTRL_TYPE_MASK                                0x00000200

#define RX_ATTENTION_1_MORE_DATA_OFFSET                              0x00000004
#define RX_ATTENTION_1_MORE_DATA_LSB                                 10
#define RX_ATTENTION_1_MORE_DATA_MASK                                0x00000400

#define RX_ATTENTION_1_EOSP_OFFSET                                   0x00000004
#define RX_ATTENTION_1_EOSP_LSB                                      11
#define RX_ATTENTION_1_EOSP_MASK                                     0x00000800

#define RX_ATTENTION_1_A_MSDU_ERROR_OFFSET                           0x00000004
#define RX_ATTENTION_1_A_MSDU_ERROR_LSB                              12
#define RX_ATTENTION_1_A_MSDU_ERROR_MASK                             0x00001000

#define RX_ATTENTION_1_FRAGMENT_FLAG_OFFSET                          0x00000004
#define RX_ATTENTION_1_FRAGMENT_FLAG_LSB                             13
#define RX_ATTENTION_1_FRAGMENT_FLAG_MASK                            0x00002000

#define RX_ATTENTION_1_ORDER_OFFSET                                  0x00000004
#define RX_ATTENTION_1_ORDER_LSB                                     14
#define RX_ATTENTION_1_ORDER_MASK                                    0x00004000

#define RX_ATTENTION_1_CCE_MATCH_OFFSET                              0x00000004
#define RX_ATTENTION_1_CCE_MATCH_LSB                                 15
#define RX_ATTENTION_1_CCE_MATCH_MASK                                0x00008000

#define RX_ATTENTION_1_OVERFLOW_ERR_OFFSET                           0x00000004
#define RX_ATTENTION_1_OVERFLOW_ERR_LSB                              16
#define RX_ATTENTION_1_OVERFLOW_ERR_MASK                             0x00010000

#define RX_ATTENTION_1_MSDU_LENGTH_ERR_OFFSET                        0x00000004
#define RX_ATTENTION_1_MSDU_LENGTH_ERR_LSB                           17
#define RX_ATTENTION_1_MSDU_LENGTH_ERR_MASK                          0x00020000

#define RX_ATTENTION_1_TCP_UDP_CHKSUM_FAIL_OFFSET                    0x00000004
#define RX_ATTENTION_1_TCP_UDP_CHKSUM_FAIL_LSB                       18
#define RX_ATTENTION_1_TCP_UDP_CHKSUM_FAIL_MASK                      0x00040000

#define RX_ATTENTION_1_IP_CHKSUM_FAIL_OFFSET                         0x00000004
#define RX_ATTENTION_1_IP_CHKSUM_FAIL_LSB                            19
#define RX_ATTENTION_1_IP_CHKSUM_FAIL_MASK                           0x00080000

#define RX_ATTENTION_1_SA_IDX_INVALID_OFFSET                         0x00000004
#define RX_ATTENTION_1_SA_IDX_INVALID_LSB                            20
#define RX_ATTENTION_1_SA_IDX_INVALID_MASK                           0x00100000

#define RX_ATTENTION_1_DA_IDX_INVALID_OFFSET                         0x00000004
#define RX_ATTENTION_1_DA_IDX_INVALID_LSB                            21
#define RX_ATTENTION_1_DA_IDX_INVALID_MASK                           0x00200000

#define RX_ATTENTION_1_RESERVED_1B_OFFSET                            0x00000004
#define RX_ATTENTION_1_RESERVED_1B_LSB                               22
#define RX_ATTENTION_1_RESERVED_1B_MASK                              0x00400000

#define RX_ATTENTION_1_RX_IN_TX_DECRYPT_BYP_OFFSET                   0x00000004
#define RX_ATTENTION_1_RX_IN_TX_DECRYPT_BYP_LSB                      23
#define RX_ATTENTION_1_RX_IN_TX_DECRYPT_BYP_MASK                     0x00800000

#define RX_ATTENTION_1_ENCRYPT_REQUIRED_OFFSET                       0x00000004
#define RX_ATTENTION_1_ENCRYPT_REQUIRED_LSB                          24
#define RX_ATTENTION_1_ENCRYPT_REQUIRED_MASK                         0x01000000

#define RX_ATTENTION_1_DIRECTED_OFFSET                               0x00000004
#define RX_ATTENTION_1_DIRECTED_LSB                                  25
#define RX_ATTENTION_1_DIRECTED_MASK                                 0x02000000

#define RX_ATTENTION_1_BUFFER_FRAGMENT_OFFSET                        0x00000004
#define RX_ATTENTION_1_BUFFER_FRAGMENT_LSB                           26
#define RX_ATTENTION_1_BUFFER_FRAGMENT_MASK                          0x04000000

#define RX_ATTENTION_1_MPDU_LENGTH_ERR_OFFSET                        0x00000004
#define RX_ATTENTION_1_MPDU_LENGTH_ERR_LSB                           27
#define RX_ATTENTION_1_MPDU_LENGTH_ERR_MASK                          0x08000000

#define RX_ATTENTION_1_TKIP_MIC_ERR_OFFSET                           0x00000004
#define RX_ATTENTION_1_TKIP_MIC_ERR_LSB                              28
#define RX_ATTENTION_1_TKIP_MIC_ERR_MASK                             0x10000000

#define RX_ATTENTION_1_DECRYPT_ERR_OFFSET                            0x00000004
#define RX_ATTENTION_1_DECRYPT_ERR_LSB                               29
#define RX_ATTENTION_1_DECRYPT_ERR_MASK                              0x20000000

#define RX_ATTENTION_1_UNENCRYPTED_FRAME_ERR_OFFSET                  0x00000004
#define RX_ATTENTION_1_UNENCRYPTED_FRAME_ERR_LSB                     30
#define RX_ATTENTION_1_UNENCRYPTED_FRAME_ERR_MASK                    0x40000000

#define RX_ATTENTION_1_FCS_ERR_OFFSET                                0x00000004
#define RX_ATTENTION_1_FCS_ERR_LSB                                   31
#define RX_ATTENTION_1_FCS_ERR_MASK                                  0x80000000

#define RX_ATTENTION_2_FLOW_IDX_TIMEOUT_OFFSET                       0x00000008
#define RX_ATTENTION_2_FLOW_IDX_TIMEOUT_LSB                          0
#define RX_ATTENTION_2_FLOW_IDX_TIMEOUT_MASK                         0x00000001

#define RX_ATTENTION_2_FLOW_IDX_INVALID_OFFSET                       0x00000008
#define RX_ATTENTION_2_FLOW_IDX_INVALID_LSB                          1
#define RX_ATTENTION_2_FLOW_IDX_INVALID_MASK                         0x00000002

#define RX_ATTENTION_2_WIFI_PARSER_ERROR_OFFSET                      0x00000008
#define RX_ATTENTION_2_WIFI_PARSER_ERROR_LSB                         2
#define RX_ATTENTION_2_WIFI_PARSER_ERROR_MASK                        0x00000004

#define RX_ATTENTION_2_AMSDU_PARSER_ERROR_OFFSET                     0x00000008
#define RX_ATTENTION_2_AMSDU_PARSER_ERROR_LSB                        3
#define RX_ATTENTION_2_AMSDU_PARSER_ERROR_MASK                       0x00000008

#define RX_ATTENTION_2_SA_IDX_TIMEOUT_OFFSET                         0x00000008
#define RX_ATTENTION_2_SA_IDX_TIMEOUT_LSB                            4
#define RX_ATTENTION_2_SA_IDX_TIMEOUT_MASK                           0x00000010

#define RX_ATTENTION_2_DA_IDX_TIMEOUT_OFFSET                         0x00000008
#define RX_ATTENTION_2_DA_IDX_TIMEOUT_LSB                            5
#define RX_ATTENTION_2_DA_IDX_TIMEOUT_MASK                           0x00000020

#define RX_ATTENTION_2_MSDU_LIMIT_ERROR_OFFSET                       0x00000008
#define RX_ATTENTION_2_MSDU_LIMIT_ERROR_LSB                          6
#define RX_ATTENTION_2_MSDU_LIMIT_ERROR_MASK                         0x00000040

#define RX_ATTENTION_2_DA_IS_VALID_OFFSET                            0x00000008
#define RX_ATTENTION_2_DA_IS_VALID_LSB                               7
#define RX_ATTENTION_2_DA_IS_VALID_MASK                              0x00000080

#define RX_ATTENTION_2_DA_IS_MCBC_OFFSET                             0x00000008
#define RX_ATTENTION_2_DA_IS_MCBC_LSB                                8
#define RX_ATTENTION_2_DA_IS_MCBC_MASK                               0x00000100

#define RX_ATTENTION_2_SA_IS_VALID_OFFSET                            0x00000008
#define RX_ATTENTION_2_SA_IS_VALID_LSB                               9
#define RX_ATTENTION_2_SA_IS_VALID_MASK                              0x00000200

#define RX_ATTENTION_2_DECRYPT_STATUS_CODE_OFFSET                    0x00000008
#define RX_ATTENTION_2_DECRYPT_STATUS_CODE_LSB                       10
#define RX_ATTENTION_2_DECRYPT_STATUS_CODE_MASK                      0x00001c00

#define RX_ATTENTION_2_RX_BITMAP_NOT_UPDATED_OFFSET                  0x00000008
#define RX_ATTENTION_2_RX_BITMAP_NOT_UPDATED_LSB                     13
#define RX_ATTENTION_2_RX_BITMAP_NOT_UPDATED_MASK                    0x00002000

#define RX_ATTENTION_2_RESERVED_2_OFFSET                             0x00000008
#define RX_ATTENTION_2_RESERVED_2_LSB                                14
#define RX_ATTENTION_2_RESERVED_2_MASK                               0x7fffc000

#define RX_ATTENTION_2_MSDU_DONE_OFFSET                              0x00000008
#define RX_ATTENTION_2_MSDU_DONE_LSB                                 31
#define RX_ATTENTION_2_MSDU_DONE_MASK                                0x80000000

#endif
