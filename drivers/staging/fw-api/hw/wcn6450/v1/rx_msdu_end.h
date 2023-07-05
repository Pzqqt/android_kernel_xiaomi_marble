
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



#ifndef _RX_MSDU_END_H_
#define _RX_MSDU_END_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_MSDU_END 17

struct rx_msdu_end {
             uint32_t rxpcu_mpdu_filter_in_category   :  2,
                      sw_frame_group_id               :  7,
                      reserved_0                      :  7,
                      phy_ppdu_id                     : 16;
             uint32_t ip_hdr_chksum                   : 16,
                      reported_mpdu_length            : 14,
                      reserved_1a                     :  2;
             uint32_t key_id_octet                    :  8,
                      cce_super_rule                  :  6,
                      cce_classify_not_done_truncate  :  1,
                      cce_classify_not_done_cce_dis   :  1,
                      cumulative_l3_checksum          : 16;
             uint32_t rule_indication_31_0            : 32;
             uint32_t rule_indication_63_32           : 32;
             uint32_t da_offset                       :  6,
                      sa_offset                       :  6,
                      da_offset_valid                 :  1,
                      sa_offset_valid                 :  1,
                      reserved_5a                     :  2,
                      l3_type                         : 16;
             uint32_t ipv6_options_crc                : 32;
             uint32_t tcp_seq_number                  : 32;
             uint32_t tcp_ack_number                  : 32;
             uint32_t tcp_flag                        :  9,
                      lro_eligible                    :  1,
                      reserved_9a                     :  6,
                      window_size                     : 16;
             uint32_t tcp_udp_chksum                  : 16,
                      sa_idx_timeout                  :  1,
                      da_idx_timeout                  :  1,
                      msdu_limit_error                :  1,
                      flow_idx_timeout                :  1,
                      flow_idx_invalid                :  1,
                      wifi_parser_error               :  1,
                      amsdu_parser_error              :  1,
                      sa_is_valid                     :  1,
                      da_is_valid                     :  1,
                      da_is_mcbc                      :  1,
                      l3_header_padding               :  2,
                      first_msdu                      :  1,
                      last_msdu                       :  1,
                      tcp_udp_chksum_fail             :  1,
                      ip_chksum_fail                  :  1;
             uint32_t sa_idx                          : 16,
                      da_idx_or_sw_peer_id            : 16;
             uint32_t msdu_drop                       :  1,
                      reo_destination_indication      :  5,
                      flow_idx                        : 20,
                      reserved_12a                    :  6;
             uint32_t fse_metadata                    : 32;
             uint32_t cce_metadata                    : 16,
                      sa_sw_peer_id                   : 16;
             uint32_t aggregation_count               :  8,
                      flow_aggregation_continuation   :  1,
                      fisa_timeout                    :  1,
                      reserved_15a                    : 22;
             uint32_t cumulative_l4_checksum          : 16,
                      cumulative_ip_length            : 16;
};

#define RX_MSDU_END_0_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET           0x00000000
#define RX_MSDU_END_0_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB              0
#define RX_MSDU_END_0_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK             0x00000003

#define RX_MSDU_END_0_SW_FRAME_GROUP_ID_OFFSET                       0x00000000
#define RX_MSDU_END_0_SW_FRAME_GROUP_ID_LSB                          2
#define RX_MSDU_END_0_SW_FRAME_GROUP_ID_MASK                         0x000001fc

#define RX_MSDU_END_0_RESERVED_0_OFFSET                              0x00000000
#define RX_MSDU_END_0_RESERVED_0_LSB                                 9
#define RX_MSDU_END_0_RESERVED_0_MASK                                0x0000fe00

#define RX_MSDU_END_0_PHY_PPDU_ID_OFFSET                             0x00000000
#define RX_MSDU_END_0_PHY_PPDU_ID_LSB                                16
#define RX_MSDU_END_0_PHY_PPDU_ID_MASK                               0xffff0000

#define RX_MSDU_END_1_IP_HDR_CHKSUM_OFFSET                           0x00000004
#define RX_MSDU_END_1_IP_HDR_CHKSUM_LSB                              0
#define RX_MSDU_END_1_IP_HDR_CHKSUM_MASK                             0x0000ffff

#define RX_MSDU_END_1_REPORTED_MPDU_LENGTH_OFFSET                    0x00000004
#define RX_MSDU_END_1_REPORTED_MPDU_LENGTH_LSB                       16
#define RX_MSDU_END_1_REPORTED_MPDU_LENGTH_MASK                      0x3fff0000

#define RX_MSDU_END_1_RESERVED_1A_OFFSET                             0x00000004
#define RX_MSDU_END_1_RESERVED_1A_LSB                                30
#define RX_MSDU_END_1_RESERVED_1A_MASK                               0xc0000000

#define RX_MSDU_END_2_KEY_ID_OCTET_OFFSET                            0x00000008
#define RX_MSDU_END_2_KEY_ID_OCTET_LSB                               0
#define RX_MSDU_END_2_KEY_ID_OCTET_MASK                              0x000000ff

#define RX_MSDU_END_2_CCE_SUPER_RULE_OFFSET                          0x00000008
#define RX_MSDU_END_2_CCE_SUPER_RULE_LSB                             8
#define RX_MSDU_END_2_CCE_SUPER_RULE_MASK                            0x00003f00

#define RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_TRUNCATE_OFFSET          0x00000008
#define RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_TRUNCATE_LSB             14
#define RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_TRUNCATE_MASK            0x00004000

#define RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_CCE_DIS_OFFSET           0x00000008
#define RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_CCE_DIS_LSB              15
#define RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_CCE_DIS_MASK             0x00008000

#define RX_MSDU_END_2_CUMULATIVE_L3_CHECKSUM_OFFSET                  0x00000008
#define RX_MSDU_END_2_CUMULATIVE_L3_CHECKSUM_LSB                     16
#define RX_MSDU_END_2_CUMULATIVE_L3_CHECKSUM_MASK                    0xffff0000

#define RX_MSDU_END_3_RULE_INDICATION_31_0_OFFSET                    0x0000000c
#define RX_MSDU_END_3_RULE_INDICATION_31_0_LSB                       0
#define RX_MSDU_END_3_RULE_INDICATION_31_0_MASK                      0xffffffff

#define RX_MSDU_END_4_RULE_INDICATION_63_32_OFFSET                   0x00000010
#define RX_MSDU_END_4_RULE_INDICATION_63_32_LSB                      0
#define RX_MSDU_END_4_RULE_INDICATION_63_32_MASK                     0xffffffff

#define RX_MSDU_END_5_DA_OFFSET_OFFSET                               0x00000014
#define RX_MSDU_END_5_DA_OFFSET_LSB                                  0
#define RX_MSDU_END_5_DA_OFFSET_MASK                                 0x0000003f

#define RX_MSDU_END_5_SA_OFFSET_OFFSET                               0x00000014
#define RX_MSDU_END_5_SA_OFFSET_LSB                                  6
#define RX_MSDU_END_5_SA_OFFSET_MASK                                 0x00000fc0

#define RX_MSDU_END_5_DA_OFFSET_VALID_OFFSET                         0x00000014
#define RX_MSDU_END_5_DA_OFFSET_VALID_LSB                            12
#define RX_MSDU_END_5_DA_OFFSET_VALID_MASK                           0x00001000

#define RX_MSDU_END_5_SA_OFFSET_VALID_OFFSET                         0x00000014
#define RX_MSDU_END_5_SA_OFFSET_VALID_LSB                            13
#define RX_MSDU_END_5_SA_OFFSET_VALID_MASK                           0x00002000

#define RX_MSDU_END_5_RESERVED_5A_OFFSET                             0x00000014
#define RX_MSDU_END_5_RESERVED_5A_LSB                                14
#define RX_MSDU_END_5_RESERVED_5A_MASK                               0x0000c000

#define RX_MSDU_END_5_L3_TYPE_OFFSET                                 0x00000014
#define RX_MSDU_END_5_L3_TYPE_LSB                                    16
#define RX_MSDU_END_5_L3_TYPE_MASK                                   0xffff0000

#define RX_MSDU_END_6_IPV6_OPTIONS_CRC_OFFSET                        0x00000018
#define RX_MSDU_END_6_IPV6_OPTIONS_CRC_LSB                           0
#define RX_MSDU_END_6_IPV6_OPTIONS_CRC_MASK                          0xffffffff

#define RX_MSDU_END_7_TCP_SEQ_NUMBER_OFFSET                          0x0000001c
#define RX_MSDU_END_7_TCP_SEQ_NUMBER_LSB                             0
#define RX_MSDU_END_7_TCP_SEQ_NUMBER_MASK                            0xffffffff

#define RX_MSDU_END_8_TCP_ACK_NUMBER_OFFSET                          0x00000020
#define RX_MSDU_END_8_TCP_ACK_NUMBER_LSB                             0
#define RX_MSDU_END_8_TCP_ACK_NUMBER_MASK                            0xffffffff

#define RX_MSDU_END_9_TCP_FLAG_OFFSET                                0x00000024
#define RX_MSDU_END_9_TCP_FLAG_LSB                                   0
#define RX_MSDU_END_9_TCP_FLAG_MASK                                  0x000001ff

#define RX_MSDU_END_9_LRO_ELIGIBLE_OFFSET                            0x00000024
#define RX_MSDU_END_9_LRO_ELIGIBLE_LSB                               9
#define RX_MSDU_END_9_LRO_ELIGIBLE_MASK                              0x00000200

#define RX_MSDU_END_9_RESERVED_9A_OFFSET                             0x00000024
#define RX_MSDU_END_9_RESERVED_9A_LSB                                10
#define RX_MSDU_END_9_RESERVED_9A_MASK                               0x0000fc00

#define RX_MSDU_END_9_WINDOW_SIZE_OFFSET                             0x00000024
#define RX_MSDU_END_9_WINDOW_SIZE_LSB                                16
#define RX_MSDU_END_9_WINDOW_SIZE_MASK                               0xffff0000

#define RX_MSDU_END_10_TCP_UDP_CHKSUM_OFFSET                         0x00000028
#define RX_MSDU_END_10_TCP_UDP_CHKSUM_LSB                            0
#define RX_MSDU_END_10_TCP_UDP_CHKSUM_MASK                           0x0000ffff

#define RX_MSDU_END_10_SA_IDX_TIMEOUT_OFFSET                         0x00000028
#define RX_MSDU_END_10_SA_IDX_TIMEOUT_LSB                            16
#define RX_MSDU_END_10_SA_IDX_TIMEOUT_MASK                           0x00010000

#define RX_MSDU_END_10_DA_IDX_TIMEOUT_OFFSET                         0x00000028
#define RX_MSDU_END_10_DA_IDX_TIMEOUT_LSB                            17
#define RX_MSDU_END_10_DA_IDX_TIMEOUT_MASK                           0x00020000

#define RX_MSDU_END_10_MSDU_LIMIT_ERROR_OFFSET                       0x00000028
#define RX_MSDU_END_10_MSDU_LIMIT_ERROR_LSB                          18
#define RX_MSDU_END_10_MSDU_LIMIT_ERROR_MASK                         0x00040000

#define RX_MSDU_END_10_FLOW_IDX_TIMEOUT_OFFSET                       0x00000028
#define RX_MSDU_END_10_FLOW_IDX_TIMEOUT_LSB                          19
#define RX_MSDU_END_10_FLOW_IDX_TIMEOUT_MASK                         0x00080000

#define RX_MSDU_END_10_FLOW_IDX_INVALID_OFFSET                       0x00000028
#define RX_MSDU_END_10_FLOW_IDX_INVALID_LSB                          20
#define RX_MSDU_END_10_FLOW_IDX_INVALID_MASK                         0x00100000

#define RX_MSDU_END_10_WIFI_PARSER_ERROR_OFFSET                      0x00000028
#define RX_MSDU_END_10_WIFI_PARSER_ERROR_LSB                         21
#define RX_MSDU_END_10_WIFI_PARSER_ERROR_MASK                        0x00200000

#define RX_MSDU_END_10_AMSDU_PARSER_ERROR_OFFSET                     0x00000028
#define RX_MSDU_END_10_AMSDU_PARSER_ERROR_LSB                        22
#define RX_MSDU_END_10_AMSDU_PARSER_ERROR_MASK                       0x00400000

#define RX_MSDU_END_10_SA_IS_VALID_OFFSET                            0x00000028
#define RX_MSDU_END_10_SA_IS_VALID_LSB                               23
#define RX_MSDU_END_10_SA_IS_VALID_MASK                              0x00800000

#define RX_MSDU_END_10_DA_IS_VALID_OFFSET                            0x00000028
#define RX_MSDU_END_10_DA_IS_VALID_LSB                               24
#define RX_MSDU_END_10_DA_IS_VALID_MASK                              0x01000000

#define RX_MSDU_END_10_DA_IS_MCBC_OFFSET                             0x00000028
#define RX_MSDU_END_10_DA_IS_MCBC_LSB                                25
#define RX_MSDU_END_10_DA_IS_MCBC_MASK                               0x02000000

#define RX_MSDU_END_10_L3_HEADER_PADDING_OFFSET                      0x00000028
#define RX_MSDU_END_10_L3_HEADER_PADDING_LSB                         26
#define RX_MSDU_END_10_L3_HEADER_PADDING_MASK                        0x0c000000

#define RX_MSDU_END_10_FIRST_MSDU_OFFSET                             0x00000028
#define RX_MSDU_END_10_FIRST_MSDU_LSB                                28
#define RX_MSDU_END_10_FIRST_MSDU_MASK                               0x10000000

#define RX_MSDU_END_10_LAST_MSDU_OFFSET                              0x00000028
#define RX_MSDU_END_10_LAST_MSDU_LSB                                 29
#define RX_MSDU_END_10_LAST_MSDU_MASK                                0x20000000

#define RX_MSDU_END_10_TCP_UDP_CHKSUM_FAIL_OFFSET                    0x00000028
#define RX_MSDU_END_10_TCP_UDP_CHKSUM_FAIL_LSB                       30
#define RX_MSDU_END_10_TCP_UDP_CHKSUM_FAIL_MASK                      0x40000000

#define RX_MSDU_END_10_IP_CHKSUM_FAIL_OFFSET                         0x00000028
#define RX_MSDU_END_10_IP_CHKSUM_FAIL_LSB                            31
#define RX_MSDU_END_10_IP_CHKSUM_FAIL_MASK                           0x80000000

#define RX_MSDU_END_11_SA_IDX_OFFSET                                 0x0000002c
#define RX_MSDU_END_11_SA_IDX_LSB                                    0
#define RX_MSDU_END_11_SA_IDX_MASK                                   0x0000ffff

#define RX_MSDU_END_11_DA_IDX_OR_SW_PEER_ID_OFFSET                   0x0000002c
#define RX_MSDU_END_11_DA_IDX_OR_SW_PEER_ID_LSB                      16
#define RX_MSDU_END_11_DA_IDX_OR_SW_PEER_ID_MASK                     0xffff0000

#define RX_MSDU_END_12_MSDU_DROP_OFFSET                              0x00000030
#define RX_MSDU_END_12_MSDU_DROP_LSB                                 0
#define RX_MSDU_END_12_MSDU_DROP_MASK                                0x00000001

#define RX_MSDU_END_12_REO_DESTINATION_INDICATION_OFFSET             0x00000030
#define RX_MSDU_END_12_REO_DESTINATION_INDICATION_LSB                1
#define RX_MSDU_END_12_REO_DESTINATION_INDICATION_MASK               0x0000003e

#define RX_MSDU_END_12_FLOW_IDX_OFFSET                               0x00000030
#define RX_MSDU_END_12_FLOW_IDX_LSB                                  6
#define RX_MSDU_END_12_FLOW_IDX_MASK                                 0x03ffffc0

#define RX_MSDU_END_12_RESERVED_12A_OFFSET                           0x00000030
#define RX_MSDU_END_12_RESERVED_12A_LSB                              26
#define RX_MSDU_END_12_RESERVED_12A_MASK                             0xfc000000

#define RX_MSDU_END_13_FSE_METADATA_OFFSET                           0x00000034
#define RX_MSDU_END_13_FSE_METADATA_LSB                              0
#define RX_MSDU_END_13_FSE_METADATA_MASK                             0xffffffff

#define RX_MSDU_END_14_CCE_METADATA_OFFSET                           0x00000038
#define RX_MSDU_END_14_CCE_METADATA_LSB                              0
#define RX_MSDU_END_14_CCE_METADATA_MASK                             0x0000ffff

#define RX_MSDU_END_14_SA_SW_PEER_ID_OFFSET                          0x00000038
#define RX_MSDU_END_14_SA_SW_PEER_ID_LSB                             16
#define RX_MSDU_END_14_SA_SW_PEER_ID_MASK                            0xffff0000

#define RX_MSDU_END_15_AGGREGATION_COUNT_OFFSET                      0x0000003c
#define RX_MSDU_END_15_AGGREGATION_COUNT_LSB                         0
#define RX_MSDU_END_15_AGGREGATION_COUNT_MASK                        0x000000ff

#define RX_MSDU_END_15_FLOW_AGGREGATION_CONTINUATION_OFFSET          0x0000003c
#define RX_MSDU_END_15_FLOW_AGGREGATION_CONTINUATION_LSB             8
#define RX_MSDU_END_15_FLOW_AGGREGATION_CONTINUATION_MASK            0x00000100

#define RX_MSDU_END_15_FISA_TIMEOUT_OFFSET                           0x0000003c
#define RX_MSDU_END_15_FISA_TIMEOUT_LSB                              9
#define RX_MSDU_END_15_FISA_TIMEOUT_MASK                             0x00000200

#define RX_MSDU_END_15_RESERVED_15A_OFFSET                           0x0000003c
#define RX_MSDU_END_15_RESERVED_15A_LSB                              10
#define RX_MSDU_END_15_RESERVED_15A_MASK                             0xfffffc00

#define RX_MSDU_END_16_CUMULATIVE_L4_CHECKSUM_OFFSET                 0x00000040
#define RX_MSDU_END_16_CUMULATIVE_L4_CHECKSUM_LSB                    0
#define RX_MSDU_END_16_CUMULATIVE_L4_CHECKSUM_MASK                   0x0000ffff

#define RX_MSDU_END_16_CUMULATIVE_IP_LENGTH_OFFSET                   0x00000040
#define RX_MSDU_END_16_CUMULATIVE_IP_LENGTH_LSB                      16
#define RX_MSDU_END_16_CUMULATIVE_IP_LENGTH_MASK                     0xffff0000

#endif
