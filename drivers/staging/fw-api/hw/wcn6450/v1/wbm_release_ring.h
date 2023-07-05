
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



#ifndef _WBM_RELEASE_RING_H_
#define _WBM_RELEASE_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#include "buffer_addr_info.h"
#include "tx_rate_stats_info.h"

#define NUM_OF_DWORDS_WBM_RELEASE_RING 8

struct wbm_release_ring {
    struct            buffer_addr_info                       released_buff_or_desc_addr_info;
             uint32_t release_source_module           :  3,
                      bm_action                       :  3,
                      buffer_or_desc_type             :  3,
                      first_msdu_index                :  4,
                      tqm_release_reason              :  4,
                      rxdma_push_reason               :  2,
                      rxdma_error_code                :  5,
                      reo_push_reason                 :  2,
                      reo_error_code                  :  5,
                      wbm_internal_error              :  1;
             uint32_t tqm_status_number               : 24,
                      transmit_count                  :  7,
                      msdu_continuation               :  1;
             uint32_t ack_frame_rssi                  :  8,
                      sw_release_details_valid        :  1,
                      first_msdu                      :  1,
                      last_msdu                       :  1,
                      msdu_part_of_amsdu              :  1,
                      fw_tx_notify_frame              :  1,
                      buffer_timestamp                : 19;
    struct            tx_rate_stats_info                       tx_rate_stats;
             uint32_t sw_peer_id                      : 16,
                      tid                             :  4,
                      ring_id                         :  8,
                      looping_count                   :  4;
};

#define WBM_RELEASE_RING_0_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET 0x00000000
#define WBM_RELEASE_RING_0_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB 0
#define WBM_RELEASE_RING_0_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK 0xffffffff

#define WBM_RELEASE_RING_1_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET 0x00000004
#define WBM_RELEASE_RING_1_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB 0
#define WBM_RELEASE_RING_1_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK 0x000000ff

#define WBM_RELEASE_RING_1_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET 0x00000004
#define WBM_RELEASE_RING_1_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB 8
#define WBM_RELEASE_RING_1_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK 0x00000700

#define WBM_RELEASE_RING_1_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET 0x00000004
#define WBM_RELEASE_RING_1_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB 11
#define WBM_RELEASE_RING_1_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK 0xfffff800

#define WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_OFFSET              0x00000008
#define WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_LSB                 0
#define WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_MASK                0x00000007

#define WBM_RELEASE_RING_2_BM_ACTION_OFFSET                          0x00000008
#define WBM_RELEASE_RING_2_BM_ACTION_LSB                             3
#define WBM_RELEASE_RING_2_BM_ACTION_MASK                            0x00000038

#define WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE_OFFSET                0x00000008
#define WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE_LSB                   6
#define WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE_MASK                  0x000001c0

#define WBM_RELEASE_RING_2_FIRST_MSDU_INDEX_OFFSET                   0x00000008
#define WBM_RELEASE_RING_2_FIRST_MSDU_INDEX_LSB                      9
#define WBM_RELEASE_RING_2_FIRST_MSDU_INDEX_MASK                     0x00001e00

#define WBM_RELEASE_RING_2_TQM_RELEASE_REASON_OFFSET                 0x00000008
#define WBM_RELEASE_RING_2_TQM_RELEASE_REASON_LSB                    13
#define WBM_RELEASE_RING_2_TQM_RELEASE_REASON_MASK                   0x0001e000

#define WBM_RELEASE_RING_2_RXDMA_PUSH_REASON_OFFSET                  0x00000008
#define WBM_RELEASE_RING_2_RXDMA_PUSH_REASON_LSB                     17
#define WBM_RELEASE_RING_2_RXDMA_PUSH_REASON_MASK                    0x00060000

#define WBM_RELEASE_RING_2_RXDMA_ERROR_CODE_OFFSET                   0x00000008
#define WBM_RELEASE_RING_2_RXDMA_ERROR_CODE_LSB                      19
#define WBM_RELEASE_RING_2_RXDMA_ERROR_CODE_MASK                     0x00f80000

#define WBM_RELEASE_RING_2_REO_PUSH_REASON_OFFSET                    0x00000008
#define WBM_RELEASE_RING_2_REO_PUSH_REASON_LSB                       24
#define WBM_RELEASE_RING_2_REO_PUSH_REASON_MASK                      0x03000000

#define WBM_RELEASE_RING_2_REO_ERROR_CODE_OFFSET                     0x00000008
#define WBM_RELEASE_RING_2_REO_ERROR_CODE_LSB                        26
#define WBM_RELEASE_RING_2_REO_ERROR_CODE_MASK                       0x7c000000

#define WBM_RELEASE_RING_2_WBM_INTERNAL_ERROR_OFFSET                 0x00000008
#define WBM_RELEASE_RING_2_WBM_INTERNAL_ERROR_LSB                    31
#define WBM_RELEASE_RING_2_WBM_INTERNAL_ERROR_MASK                   0x80000000

#define WBM_RELEASE_RING_3_TQM_STATUS_NUMBER_OFFSET                  0x0000000c
#define WBM_RELEASE_RING_3_TQM_STATUS_NUMBER_LSB                     0
#define WBM_RELEASE_RING_3_TQM_STATUS_NUMBER_MASK                    0x00ffffff

#define WBM_RELEASE_RING_3_TRANSMIT_COUNT_OFFSET                     0x0000000c
#define WBM_RELEASE_RING_3_TRANSMIT_COUNT_LSB                        24
#define WBM_RELEASE_RING_3_TRANSMIT_COUNT_MASK                       0x7f000000

#define WBM_RELEASE_RING_3_MSDU_CONTINUATION_OFFSET                  0x0000000c
#define WBM_RELEASE_RING_3_MSDU_CONTINUATION_LSB                     31
#define WBM_RELEASE_RING_3_MSDU_CONTINUATION_MASK                    0x80000000

#define WBM_RELEASE_RING_4_ACK_FRAME_RSSI_OFFSET                     0x00000010
#define WBM_RELEASE_RING_4_ACK_FRAME_RSSI_LSB                        0
#define WBM_RELEASE_RING_4_ACK_FRAME_RSSI_MASK                       0x000000ff

#define WBM_RELEASE_RING_4_SW_RELEASE_DETAILS_VALID_OFFSET           0x00000010
#define WBM_RELEASE_RING_4_SW_RELEASE_DETAILS_VALID_LSB              8
#define WBM_RELEASE_RING_4_SW_RELEASE_DETAILS_VALID_MASK             0x00000100

#define WBM_RELEASE_RING_4_FIRST_MSDU_OFFSET                         0x00000010
#define WBM_RELEASE_RING_4_FIRST_MSDU_LSB                            9
#define WBM_RELEASE_RING_4_FIRST_MSDU_MASK                           0x00000200

#define WBM_RELEASE_RING_4_LAST_MSDU_OFFSET                          0x00000010
#define WBM_RELEASE_RING_4_LAST_MSDU_LSB                             10
#define WBM_RELEASE_RING_4_LAST_MSDU_MASK                            0x00000400

#define WBM_RELEASE_RING_4_MSDU_PART_OF_AMSDU_OFFSET                 0x00000010
#define WBM_RELEASE_RING_4_MSDU_PART_OF_AMSDU_LSB                    11
#define WBM_RELEASE_RING_4_MSDU_PART_OF_AMSDU_MASK                   0x00000800

#define WBM_RELEASE_RING_4_FW_TX_NOTIFY_FRAME_OFFSET                 0x00000010
#define WBM_RELEASE_RING_4_FW_TX_NOTIFY_FRAME_LSB                    12
#define WBM_RELEASE_RING_4_FW_TX_NOTIFY_FRAME_MASK                   0x00001000

#define WBM_RELEASE_RING_4_BUFFER_TIMESTAMP_OFFSET                   0x00000010
#define WBM_RELEASE_RING_4_BUFFER_TIMESTAMP_LSB                      13
#define WBM_RELEASE_RING_4_BUFFER_TIMESTAMP_MASK                     0xffffe000

#define WBM_RELEASE_RING_5_TX_RATE_STATS_TX_RATE_STATS_INFO_VALID_OFFSET 0x00000014
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TX_RATE_STATS_INFO_VALID_LSB 0
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TX_RATE_STATS_INFO_VALID_MASK 0x00000001

#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_BW_OFFSET          0x00000014
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_BW_LSB             1
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_BW_MASK            0x00000006

#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_PKT_TYPE_OFFSET    0x00000014
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_PKT_TYPE_LSB       3
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_PKT_TYPE_MASK      0x00000078

#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_STBC_OFFSET        0x00000014
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_STBC_LSB           7
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_STBC_MASK          0x00000080

#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_LDPC_OFFSET        0x00000014
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_LDPC_LSB           8
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_LDPC_MASK          0x00000100

#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_SGI_OFFSET         0x00000014
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_SGI_LSB            9
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_SGI_MASK           0x00000600

#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_MCS_OFFSET         0x00000014
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_MCS_LSB            11
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TRANSMIT_MCS_MASK           0x00007800

#define WBM_RELEASE_RING_5_TX_RATE_STATS_OFDMA_TRANSMISSION_OFFSET   0x00000014
#define WBM_RELEASE_RING_5_TX_RATE_STATS_OFDMA_TRANSMISSION_LSB      15
#define WBM_RELEASE_RING_5_TX_RATE_STATS_OFDMA_TRANSMISSION_MASK     0x00008000

#define WBM_RELEASE_RING_5_TX_RATE_STATS_TONES_IN_RU_OFFSET          0x00000014
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TONES_IN_RU_LSB             16
#define WBM_RELEASE_RING_5_TX_RATE_STATS_TONES_IN_RU_MASK            0x0fff0000

#define WBM_RELEASE_RING_5_TX_RATE_STATS_RESERVED_0A_OFFSET          0x00000014
#define WBM_RELEASE_RING_5_TX_RATE_STATS_RESERVED_0A_LSB             28
#define WBM_RELEASE_RING_5_TX_RATE_STATS_RESERVED_0A_MASK            0xf0000000

#define WBM_RELEASE_RING_6_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_OFFSET 0x00000018
#define WBM_RELEASE_RING_6_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_LSB   0
#define WBM_RELEASE_RING_6_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_MASK  0xffffffff

#define WBM_RELEASE_RING_7_SW_PEER_ID_OFFSET                         0x0000001c
#define WBM_RELEASE_RING_7_SW_PEER_ID_LSB                            0
#define WBM_RELEASE_RING_7_SW_PEER_ID_MASK                           0x0000ffff

#define WBM_RELEASE_RING_7_TID_OFFSET                                0x0000001c
#define WBM_RELEASE_RING_7_TID_LSB                                   16
#define WBM_RELEASE_RING_7_TID_MASK                                  0x000f0000

#define WBM_RELEASE_RING_7_RING_ID_OFFSET                            0x0000001c
#define WBM_RELEASE_RING_7_RING_ID_LSB                               20
#define WBM_RELEASE_RING_7_RING_ID_MASK                              0x0ff00000

#define WBM_RELEASE_RING_7_LOOPING_COUNT_OFFSET                      0x0000001c
#define WBM_RELEASE_RING_7_LOOPING_COUNT_LSB                         28
#define WBM_RELEASE_RING_7_LOOPING_COUNT_MASK                        0xf0000000

#endif
