
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



#ifndef _RX_MSDU_DESC_INFO_H_
#define _RX_MSDU_DESC_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_MSDU_DESC_INFO 2

struct rx_msdu_desc_info {
             uint32_t first_msdu_in_mpdu_flag         :  1,
                      last_msdu_in_mpdu_flag          :  1,
                      msdu_continuation               :  1,
                      msdu_length                     : 14,
                      reo_destination_indication      :  5,
                      msdu_drop                       :  1,
                      sa_is_valid                     :  1,
                      sa_idx_timeout                  :  1,
                      da_is_valid                     :  1,
                      da_is_mcbc                      :  1,
                      da_idx_timeout                  :  1,
                      l3_header_padding_msb           :  1,
                      tcp_udp_chksum_fail             :  1,
                      ip_chksum_fail                  :  1,
                      raw_mpdu                        :  1;
             uint32_t sa_idx_or_sw_peer_id_14_0       : 15,
                      mpdu_ast_idx_or_sw_peer_id_14_0 : 15,
                      fr_ds                           :  1,
                      to_ds                           :  1;
};

#define RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG_OFFSET           0x00000000
#define RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG_LSB              0
#define RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG_MASK             0x00000001

#define RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG_OFFSET            0x00000000
#define RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG_LSB               1
#define RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG_MASK              0x00000002

#define RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_OFFSET                 0x00000000
#define RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_LSB                    2
#define RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_MASK                   0x00000004

#define RX_MSDU_DESC_INFO_0_MSDU_LENGTH_OFFSET                       0x00000000
#define RX_MSDU_DESC_INFO_0_MSDU_LENGTH_LSB                          3
#define RX_MSDU_DESC_INFO_0_MSDU_LENGTH_MASK                         0x0001fff8

#define RX_MSDU_DESC_INFO_0_REO_DESTINATION_INDICATION_OFFSET        0x00000000
#define RX_MSDU_DESC_INFO_0_REO_DESTINATION_INDICATION_LSB           17
#define RX_MSDU_DESC_INFO_0_REO_DESTINATION_INDICATION_MASK          0x003e0000

#define RX_MSDU_DESC_INFO_0_MSDU_DROP_OFFSET                         0x00000000
#define RX_MSDU_DESC_INFO_0_MSDU_DROP_LSB                            22
#define RX_MSDU_DESC_INFO_0_MSDU_DROP_MASK                           0x00400000

#define RX_MSDU_DESC_INFO_0_SA_IS_VALID_OFFSET                       0x00000000
#define RX_MSDU_DESC_INFO_0_SA_IS_VALID_LSB                          23
#define RX_MSDU_DESC_INFO_0_SA_IS_VALID_MASK                         0x00800000

#define RX_MSDU_DESC_INFO_0_SA_IDX_TIMEOUT_OFFSET                    0x00000000
#define RX_MSDU_DESC_INFO_0_SA_IDX_TIMEOUT_LSB                       24
#define RX_MSDU_DESC_INFO_0_SA_IDX_TIMEOUT_MASK                      0x01000000

#define RX_MSDU_DESC_INFO_0_DA_IS_VALID_OFFSET                       0x00000000
#define RX_MSDU_DESC_INFO_0_DA_IS_VALID_LSB                          25
#define RX_MSDU_DESC_INFO_0_DA_IS_VALID_MASK                         0x02000000

#define RX_MSDU_DESC_INFO_0_DA_IS_MCBC_OFFSET                        0x00000000
#define RX_MSDU_DESC_INFO_0_DA_IS_MCBC_LSB                           26
#define RX_MSDU_DESC_INFO_0_DA_IS_MCBC_MASK                          0x04000000

#define RX_MSDU_DESC_INFO_0_DA_IDX_TIMEOUT_OFFSET                    0x00000000
#define RX_MSDU_DESC_INFO_0_DA_IDX_TIMEOUT_LSB                       27
#define RX_MSDU_DESC_INFO_0_DA_IDX_TIMEOUT_MASK                      0x08000000

#define RX_MSDU_DESC_INFO_0_L3_HEADER_PADDING_MSB_OFFSET             0x00000000
#define RX_MSDU_DESC_INFO_0_L3_HEADER_PADDING_MSB_LSB                28
#define RX_MSDU_DESC_INFO_0_L3_HEADER_PADDING_MSB_MASK               0x10000000

#define RX_MSDU_DESC_INFO_0_TCP_UDP_CHKSUM_FAIL_OFFSET               0x00000000
#define RX_MSDU_DESC_INFO_0_TCP_UDP_CHKSUM_FAIL_LSB                  29
#define RX_MSDU_DESC_INFO_0_TCP_UDP_CHKSUM_FAIL_MASK                 0x20000000

#define RX_MSDU_DESC_INFO_0_IP_CHKSUM_FAIL_OFFSET                    0x00000000
#define RX_MSDU_DESC_INFO_0_IP_CHKSUM_FAIL_LSB                       30
#define RX_MSDU_DESC_INFO_0_IP_CHKSUM_FAIL_MASK                      0x40000000

#define RX_MSDU_DESC_INFO_0_RAW_MPDU_OFFSET                          0x00000000
#define RX_MSDU_DESC_INFO_0_RAW_MPDU_LSB                             31
#define RX_MSDU_DESC_INFO_0_RAW_MPDU_MASK                            0x80000000

#define RX_MSDU_DESC_INFO_1_SA_IDX_OR_SW_PEER_ID_14_0_OFFSET         0x00000004
#define RX_MSDU_DESC_INFO_1_SA_IDX_OR_SW_PEER_ID_14_0_LSB            0
#define RX_MSDU_DESC_INFO_1_SA_IDX_OR_SW_PEER_ID_14_0_MASK           0x00007fff

#define RX_MSDU_DESC_INFO_1_MPDU_AST_IDX_OR_SW_PEER_ID_14_0_OFFSET   0x00000004
#define RX_MSDU_DESC_INFO_1_MPDU_AST_IDX_OR_SW_PEER_ID_14_0_LSB      15
#define RX_MSDU_DESC_INFO_1_MPDU_AST_IDX_OR_SW_PEER_ID_14_0_MASK     0x3fff8000

#define RX_MSDU_DESC_INFO_1_FR_DS_OFFSET                             0x00000004
#define RX_MSDU_DESC_INFO_1_FR_DS_LSB                                30
#define RX_MSDU_DESC_INFO_1_FR_DS_MASK                               0x40000000

#define RX_MSDU_DESC_INFO_1_TO_DS_OFFSET                             0x00000004
#define RX_MSDU_DESC_INFO_1_TO_DS_LSB                                31
#define RX_MSDU_DESC_INFO_1_TO_DS_MASK                               0x80000000

#endif
