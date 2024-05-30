
/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#define NUM_OF_DWORDS_RX_ATTENTION 4

#define NUM_OF_QWORDS_RX_ATTENTION 2


struct rx_attention {
	     uint32_t rxpcu_mpdu_filter_in_category                           :  2,
		      sw_frame_group_id                                       :  7,
		      reserved_0                                              :  7,
		      phy_ppdu_id                                             : 16;
	     uint32_t first_mpdu                                              :  1,
		      reserved_1a                                             :  1,
		      mcast_bcast                                             :  1,
		      ast_index_not_found                                     :  1,
		      ast_index_timeout                                       :  1,
		      power_mgmt                                              :  1,
		      non_qos                                                 :  1,
		      null_data                                               :  1,
		      mgmt_type                                               :  1,
		      ctrl_type                                               :  1,
		      more_data                                               :  1,
		      eosp                                                    :  1,
		      a_msdu_error                                            :  1,
		      fragment_flag                                           :  1,
		      order                                                   :  1,
		      cce_match                                               :  1,
		      overflow_err                                            :  1,
		      msdu_length_err                                         :  1,
		      tcp_udp_chksum_fail                                     :  1,
		      ip_chksum_fail                                          :  1,
		      sa_idx_invalid                                          :  1,
		      da_idx_invalid                                          :  1,
		      reserved_1b                                             :  1,
		      rx_in_tx_decrypt_byp                                    :  1,
		      encrypt_required                                        :  1,
		      directed                                                :  1,
		      buffer_fragment                                         :  1,
		      mpdu_length_err                                         :  1,
		      tkip_mic_err                                            :  1,
		      decrypt_err                                             :  1,
		      unencrypted_frame_err                                   :  1,
		      fcs_err                                                 :  1;
	     uint32_t flow_idx_timeout                                        :  1,
		      flow_idx_invalid                                        :  1,
		      wifi_parser_error                                       :  1,
		      amsdu_parser_error                                      :  1,
		      sa_idx_timeout                                          :  1,
		      da_idx_timeout                                          :  1,
		      msdu_limit_error                                        :  1,
		      da_is_valid                                             :  1,
		      da_is_mcbc                                              :  1,
		      sa_is_valid                                             :  1,
		      decrypt_status_code                                     :  3,
		      rx_bitmap_not_updated                                   :  1,
		      reserved_2                                              : 17,
		      msdu_done                                               :  1;
	     uint32_t tlv64_padding                                           : 32;
};




#define RX_ATTENTION_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET                           0x0000000000000000
#define RX_ATTENTION_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB                              0
#define RX_ATTENTION_RXPCU_MPDU_FILTER_IN_CATEGORY_MSB                              1
#define RX_ATTENTION_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK                             0x0000000000000003




#define RX_ATTENTION_SW_FRAME_GROUP_ID_OFFSET                                       0x0000000000000000
#define RX_ATTENTION_SW_FRAME_GROUP_ID_LSB                                          2
#define RX_ATTENTION_SW_FRAME_GROUP_ID_MSB                                          8
#define RX_ATTENTION_SW_FRAME_GROUP_ID_MASK                                         0x00000000000001fc




#define RX_ATTENTION_RESERVED_0_OFFSET                                              0x0000000000000000
#define RX_ATTENTION_RESERVED_0_LSB                                                 9
#define RX_ATTENTION_RESERVED_0_MSB                                                 15
#define RX_ATTENTION_RESERVED_0_MASK                                                0x000000000000fe00




#define RX_ATTENTION_PHY_PPDU_ID_OFFSET                                             0x0000000000000000
#define RX_ATTENTION_PHY_PPDU_ID_LSB                                                16
#define RX_ATTENTION_PHY_PPDU_ID_MSB                                                31
#define RX_ATTENTION_PHY_PPDU_ID_MASK                                               0x00000000ffff0000




#define RX_ATTENTION_FIRST_MPDU_OFFSET                                              0x0000000000000000
#define RX_ATTENTION_FIRST_MPDU_LSB                                                 32
#define RX_ATTENTION_FIRST_MPDU_MSB                                                 32
#define RX_ATTENTION_FIRST_MPDU_MASK                                                0x0000000100000000




#define RX_ATTENTION_RESERVED_1A_OFFSET                                             0x0000000000000000
#define RX_ATTENTION_RESERVED_1A_LSB                                                33
#define RX_ATTENTION_RESERVED_1A_MSB                                                33
#define RX_ATTENTION_RESERVED_1A_MASK                                               0x0000000200000000




#define RX_ATTENTION_MCAST_BCAST_OFFSET                                             0x0000000000000000
#define RX_ATTENTION_MCAST_BCAST_LSB                                                34
#define RX_ATTENTION_MCAST_BCAST_MSB                                                34
#define RX_ATTENTION_MCAST_BCAST_MASK                                               0x0000000400000000




#define RX_ATTENTION_AST_INDEX_NOT_FOUND_OFFSET                                     0x0000000000000000
#define RX_ATTENTION_AST_INDEX_NOT_FOUND_LSB                                        35
#define RX_ATTENTION_AST_INDEX_NOT_FOUND_MSB                                        35
#define RX_ATTENTION_AST_INDEX_NOT_FOUND_MASK                                       0x0000000800000000




#define RX_ATTENTION_AST_INDEX_TIMEOUT_OFFSET                                       0x0000000000000000
#define RX_ATTENTION_AST_INDEX_TIMEOUT_LSB                                          36
#define RX_ATTENTION_AST_INDEX_TIMEOUT_MSB                                          36
#define RX_ATTENTION_AST_INDEX_TIMEOUT_MASK                                         0x0000001000000000




#define RX_ATTENTION_POWER_MGMT_OFFSET                                              0x0000000000000000
#define RX_ATTENTION_POWER_MGMT_LSB                                                 37
#define RX_ATTENTION_POWER_MGMT_MSB                                                 37
#define RX_ATTENTION_POWER_MGMT_MASK                                                0x0000002000000000




#define RX_ATTENTION_NON_QOS_OFFSET                                                 0x0000000000000000
#define RX_ATTENTION_NON_QOS_LSB                                                    38
#define RX_ATTENTION_NON_QOS_MSB                                                    38
#define RX_ATTENTION_NON_QOS_MASK                                                   0x0000004000000000




#define RX_ATTENTION_NULL_DATA_OFFSET                                               0x0000000000000000
#define RX_ATTENTION_NULL_DATA_LSB                                                  39
#define RX_ATTENTION_NULL_DATA_MSB                                                  39
#define RX_ATTENTION_NULL_DATA_MASK                                                 0x0000008000000000




#define RX_ATTENTION_MGMT_TYPE_OFFSET                                               0x0000000000000000
#define RX_ATTENTION_MGMT_TYPE_LSB                                                  40
#define RX_ATTENTION_MGMT_TYPE_MSB                                                  40
#define RX_ATTENTION_MGMT_TYPE_MASK                                                 0x0000010000000000




#define RX_ATTENTION_CTRL_TYPE_OFFSET                                               0x0000000000000000
#define RX_ATTENTION_CTRL_TYPE_LSB                                                  41
#define RX_ATTENTION_CTRL_TYPE_MSB                                                  41
#define RX_ATTENTION_CTRL_TYPE_MASK                                                 0x0000020000000000




#define RX_ATTENTION_MORE_DATA_OFFSET                                               0x0000000000000000
#define RX_ATTENTION_MORE_DATA_LSB                                                  42
#define RX_ATTENTION_MORE_DATA_MSB                                                  42
#define RX_ATTENTION_MORE_DATA_MASK                                                 0x0000040000000000




#define RX_ATTENTION_EOSP_OFFSET                                                    0x0000000000000000
#define RX_ATTENTION_EOSP_LSB                                                       43
#define RX_ATTENTION_EOSP_MSB                                                       43
#define RX_ATTENTION_EOSP_MASK                                                      0x0000080000000000




#define RX_ATTENTION_A_MSDU_ERROR_OFFSET                                            0x0000000000000000
#define RX_ATTENTION_A_MSDU_ERROR_LSB                                               44
#define RX_ATTENTION_A_MSDU_ERROR_MSB                                               44
#define RX_ATTENTION_A_MSDU_ERROR_MASK                                              0x0000100000000000




#define RX_ATTENTION_FRAGMENT_FLAG_OFFSET                                           0x0000000000000000
#define RX_ATTENTION_FRAGMENT_FLAG_LSB                                              45
#define RX_ATTENTION_FRAGMENT_FLAG_MSB                                              45
#define RX_ATTENTION_FRAGMENT_FLAG_MASK                                             0x0000200000000000




#define RX_ATTENTION_ORDER_OFFSET                                                   0x0000000000000000
#define RX_ATTENTION_ORDER_LSB                                                      46
#define RX_ATTENTION_ORDER_MSB                                                      46
#define RX_ATTENTION_ORDER_MASK                                                     0x0000400000000000




#define RX_ATTENTION_CCE_MATCH_OFFSET                                               0x0000000000000000
#define RX_ATTENTION_CCE_MATCH_LSB                                                  47
#define RX_ATTENTION_CCE_MATCH_MSB                                                  47
#define RX_ATTENTION_CCE_MATCH_MASK                                                 0x0000800000000000




#define RX_ATTENTION_OVERFLOW_ERR_OFFSET                                            0x0000000000000000
#define RX_ATTENTION_OVERFLOW_ERR_LSB                                               48
#define RX_ATTENTION_OVERFLOW_ERR_MSB                                               48
#define RX_ATTENTION_OVERFLOW_ERR_MASK                                              0x0001000000000000




#define RX_ATTENTION_MSDU_LENGTH_ERR_OFFSET                                         0x0000000000000000
#define RX_ATTENTION_MSDU_LENGTH_ERR_LSB                                            49
#define RX_ATTENTION_MSDU_LENGTH_ERR_MSB                                            49
#define RX_ATTENTION_MSDU_LENGTH_ERR_MASK                                           0x0002000000000000




#define RX_ATTENTION_TCP_UDP_CHKSUM_FAIL_OFFSET                                     0x0000000000000000
#define RX_ATTENTION_TCP_UDP_CHKSUM_FAIL_LSB                                        50
#define RX_ATTENTION_TCP_UDP_CHKSUM_FAIL_MSB                                        50
#define RX_ATTENTION_TCP_UDP_CHKSUM_FAIL_MASK                                       0x0004000000000000




#define RX_ATTENTION_IP_CHKSUM_FAIL_OFFSET                                          0x0000000000000000
#define RX_ATTENTION_IP_CHKSUM_FAIL_LSB                                             51
#define RX_ATTENTION_IP_CHKSUM_FAIL_MSB                                             51
#define RX_ATTENTION_IP_CHKSUM_FAIL_MASK                                            0x0008000000000000




#define RX_ATTENTION_SA_IDX_INVALID_OFFSET                                          0x0000000000000000
#define RX_ATTENTION_SA_IDX_INVALID_LSB                                             52
#define RX_ATTENTION_SA_IDX_INVALID_MSB                                             52
#define RX_ATTENTION_SA_IDX_INVALID_MASK                                            0x0010000000000000




#define RX_ATTENTION_DA_IDX_INVALID_OFFSET                                          0x0000000000000000
#define RX_ATTENTION_DA_IDX_INVALID_LSB                                             53
#define RX_ATTENTION_DA_IDX_INVALID_MSB                                             53
#define RX_ATTENTION_DA_IDX_INVALID_MASK                                            0x0020000000000000




#define RX_ATTENTION_RESERVED_1B_OFFSET                                             0x0000000000000000
#define RX_ATTENTION_RESERVED_1B_LSB                                                54
#define RX_ATTENTION_RESERVED_1B_MSB                                                54
#define RX_ATTENTION_RESERVED_1B_MASK                                               0x0040000000000000




#define RX_ATTENTION_RX_IN_TX_DECRYPT_BYP_OFFSET                                    0x0000000000000000
#define RX_ATTENTION_RX_IN_TX_DECRYPT_BYP_LSB                                       55
#define RX_ATTENTION_RX_IN_TX_DECRYPT_BYP_MSB                                       55
#define RX_ATTENTION_RX_IN_TX_DECRYPT_BYP_MASK                                      0x0080000000000000




#define RX_ATTENTION_ENCRYPT_REQUIRED_OFFSET                                        0x0000000000000000
#define RX_ATTENTION_ENCRYPT_REQUIRED_LSB                                           56
#define RX_ATTENTION_ENCRYPT_REQUIRED_MSB                                           56
#define RX_ATTENTION_ENCRYPT_REQUIRED_MASK                                          0x0100000000000000




#define RX_ATTENTION_DIRECTED_OFFSET                                                0x0000000000000000
#define RX_ATTENTION_DIRECTED_LSB                                                   57
#define RX_ATTENTION_DIRECTED_MSB                                                   57
#define RX_ATTENTION_DIRECTED_MASK                                                  0x0200000000000000




#define RX_ATTENTION_BUFFER_FRAGMENT_OFFSET                                         0x0000000000000000
#define RX_ATTENTION_BUFFER_FRAGMENT_LSB                                            58
#define RX_ATTENTION_BUFFER_FRAGMENT_MSB                                            58
#define RX_ATTENTION_BUFFER_FRAGMENT_MASK                                           0x0400000000000000




#define RX_ATTENTION_MPDU_LENGTH_ERR_OFFSET                                         0x0000000000000000
#define RX_ATTENTION_MPDU_LENGTH_ERR_LSB                                            59
#define RX_ATTENTION_MPDU_LENGTH_ERR_MSB                                            59
#define RX_ATTENTION_MPDU_LENGTH_ERR_MASK                                           0x0800000000000000




#define RX_ATTENTION_TKIP_MIC_ERR_OFFSET                                            0x0000000000000000
#define RX_ATTENTION_TKIP_MIC_ERR_LSB                                               60
#define RX_ATTENTION_TKIP_MIC_ERR_MSB                                               60
#define RX_ATTENTION_TKIP_MIC_ERR_MASK                                              0x1000000000000000




#define RX_ATTENTION_DECRYPT_ERR_OFFSET                                             0x0000000000000000
#define RX_ATTENTION_DECRYPT_ERR_LSB                                                61
#define RX_ATTENTION_DECRYPT_ERR_MSB                                                61
#define RX_ATTENTION_DECRYPT_ERR_MASK                                               0x2000000000000000




#define RX_ATTENTION_UNENCRYPTED_FRAME_ERR_OFFSET                                   0x0000000000000000
#define RX_ATTENTION_UNENCRYPTED_FRAME_ERR_LSB                                      62
#define RX_ATTENTION_UNENCRYPTED_FRAME_ERR_MSB                                      62
#define RX_ATTENTION_UNENCRYPTED_FRAME_ERR_MASK                                     0x4000000000000000




#define RX_ATTENTION_FCS_ERR_OFFSET                                                 0x0000000000000000
#define RX_ATTENTION_FCS_ERR_LSB                                                    63
#define RX_ATTENTION_FCS_ERR_MSB                                                    63
#define RX_ATTENTION_FCS_ERR_MASK                                                   0x8000000000000000




#define RX_ATTENTION_FLOW_IDX_TIMEOUT_OFFSET                                        0x0000000000000008
#define RX_ATTENTION_FLOW_IDX_TIMEOUT_LSB                                           0
#define RX_ATTENTION_FLOW_IDX_TIMEOUT_MSB                                           0
#define RX_ATTENTION_FLOW_IDX_TIMEOUT_MASK                                          0x0000000000000001




#define RX_ATTENTION_FLOW_IDX_INVALID_OFFSET                                        0x0000000000000008
#define RX_ATTENTION_FLOW_IDX_INVALID_LSB                                           1
#define RX_ATTENTION_FLOW_IDX_INVALID_MSB                                           1
#define RX_ATTENTION_FLOW_IDX_INVALID_MASK                                          0x0000000000000002




#define RX_ATTENTION_WIFI_PARSER_ERROR_OFFSET                                       0x0000000000000008
#define RX_ATTENTION_WIFI_PARSER_ERROR_LSB                                          2
#define RX_ATTENTION_WIFI_PARSER_ERROR_MSB                                          2
#define RX_ATTENTION_WIFI_PARSER_ERROR_MASK                                         0x0000000000000004




#define RX_ATTENTION_AMSDU_PARSER_ERROR_OFFSET                                      0x0000000000000008
#define RX_ATTENTION_AMSDU_PARSER_ERROR_LSB                                         3
#define RX_ATTENTION_AMSDU_PARSER_ERROR_MSB                                         3
#define RX_ATTENTION_AMSDU_PARSER_ERROR_MASK                                        0x0000000000000008




#define RX_ATTENTION_SA_IDX_TIMEOUT_OFFSET                                          0x0000000000000008
#define RX_ATTENTION_SA_IDX_TIMEOUT_LSB                                             4
#define RX_ATTENTION_SA_IDX_TIMEOUT_MSB                                             4
#define RX_ATTENTION_SA_IDX_TIMEOUT_MASK                                            0x0000000000000010




#define RX_ATTENTION_DA_IDX_TIMEOUT_OFFSET                                          0x0000000000000008
#define RX_ATTENTION_DA_IDX_TIMEOUT_LSB                                             5
#define RX_ATTENTION_DA_IDX_TIMEOUT_MSB                                             5
#define RX_ATTENTION_DA_IDX_TIMEOUT_MASK                                            0x0000000000000020




#define RX_ATTENTION_MSDU_LIMIT_ERROR_OFFSET                                        0x0000000000000008
#define RX_ATTENTION_MSDU_LIMIT_ERROR_LSB                                           6
#define RX_ATTENTION_MSDU_LIMIT_ERROR_MSB                                           6
#define RX_ATTENTION_MSDU_LIMIT_ERROR_MASK                                          0x0000000000000040




#define RX_ATTENTION_DA_IS_VALID_OFFSET                                             0x0000000000000008
#define RX_ATTENTION_DA_IS_VALID_LSB                                                7
#define RX_ATTENTION_DA_IS_VALID_MSB                                                7
#define RX_ATTENTION_DA_IS_VALID_MASK                                               0x0000000000000080




#define RX_ATTENTION_DA_IS_MCBC_OFFSET                                              0x0000000000000008
#define RX_ATTENTION_DA_IS_MCBC_LSB                                                 8
#define RX_ATTENTION_DA_IS_MCBC_MSB                                                 8
#define RX_ATTENTION_DA_IS_MCBC_MASK                                                0x0000000000000100




#define RX_ATTENTION_SA_IS_VALID_OFFSET                                             0x0000000000000008
#define RX_ATTENTION_SA_IS_VALID_LSB                                                9
#define RX_ATTENTION_SA_IS_VALID_MSB                                                9
#define RX_ATTENTION_SA_IS_VALID_MASK                                               0x0000000000000200




#define RX_ATTENTION_DECRYPT_STATUS_CODE_OFFSET                                     0x0000000000000008
#define RX_ATTENTION_DECRYPT_STATUS_CODE_LSB                                        10
#define RX_ATTENTION_DECRYPT_STATUS_CODE_MSB                                        12
#define RX_ATTENTION_DECRYPT_STATUS_CODE_MASK                                       0x0000000000001c00




#define RX_ATTENTION_RX_BITMAP_NOT_UPDATED_OFFSET                                   0x0000000000000008
#define RX_ATTENTION_RX_BITMAP_NOT_UPDATED_LSB                                      13
#define RX_ATTENTION_RX_BITMAP_NOT_UPDATED_MSB                                      13
#define RX_ATTENTION_RX_BITMAP_NOT_UPDATED_MASK                                     0x0000000000002000




#define RX_ATTENTION_RESERVED_2_OFFSET                                              0x0000000000000008
#define RX_ATTENTION_RESERVED_2_LSB                                                 14
#define RX_ATTENTION_RESERVED_2_MSB                                                 30
#define RX_ATTENTION_RESERVED_2_MASK                                                0x000000007fffc000




#define RX_ATTENTION_MSDU_DONE_OFFSET                                               0x0000000000000008
#define RX_ATTENTION_MSDU_DONE_LSB                                                  31
#define RX_ATTENTION_MSDU_DONE_MSB                                                  31
#define RX_ATTENTION_MSDU_DONE_MASK                                                 0x0000000080000000




#define RX_ATTENTION_TLV64_PADDING_OFFSET                                           0x0000000000000008
#define RX_ATTENTION_TLV64_PADDING_LSB                                              32
#define RX_ATTENTION_TLV64_PADDING_MSB                                              63
#define RX_ATTENTION_TLV64_PADDING_MASK                                             0xffffffff00000000



#endif
