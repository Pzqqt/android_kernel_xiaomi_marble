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


#ifndef _REO_GET_QUEUE_STATS_STATUS_H_
#define _REO_GET_QUEUE_STATS_STATUS_H_

#include "uniform_reo_status_header.h"
#define NUM_OF_DWORDS_REO_GET_QUEUE_STATS_STATUS 27

struct reo_get_queue_stats_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t tlv32_ring_padding                                      : 32;
             struct   uniform_reo_status_header                                 status_header;
             uint32_t ssn                                                     : 12,
                      current_index                                           : 10,
                      reserved_2                                              : 10;
             uint32_t pn_31_0                                                 : 32;
             uint32_t pn_63_32                                                : 32;
             uint32_t pn_95_64                                                : 32;
             uint32_t pn_127_96                                               : 32;
             uint32_t last_rx_enqueue_timestamp                               : 32;
             uint32_t last_rx_dequeue_timestamp                               : 32;
             uint32_t rx_bitmap_31_0                                          : 32;
             uint32_t rx_bitmap_63_32                                         : 32;
             uint32_t rx_bitmap_95_64                                         : 32;
             uint32_t rx_bitmap_127_96                                        : 32;
             uint32_t rx_bitmap_159_128                                       : 32;
             uint32_t rx_bitmap_191_160                                       : 32;
             uint32_t rx_bitmap_223_192                                       : 32;
             uint32_t rx_bitmap_255_224                                       : 32;
             uint32_t rx_bitmap_287_256                                       : 32;
             uint32_t current_mpdu_count                                      :  7,
                      current_msdu_count                                      : 25;
             uint32_t window_jump_2k                                          :  4,
                      timeout_count                                           :  6,
                      forward_due_to_bar_count                                :  6,
                      duplicate_count                                         : 16;
             uint32_t frames_in_order_count                                   : 24,
                      bar_received_count                                      :  8;
             uint32_t mpdu_frames_processed_count                             : 32;
             uint32_t msdu_frames_processed_count                             : 32;
             uint32_t total_processed_byte_count                              : 32;
             uint32_t late_receive_mpdu_count                                 : 12,
                      hole_count                                              : 16,
                      get_queue_1k_stats_status_to_follow                     :  1,
                      reserved_24a                                            :  3;
             uint32_t aging_drop_mpdu_count                                   : 16,
                      aging_drop_interval                                     :  8,
                      reserved_25a                                            :  4,
                      looping_count                                           :  4;
#else
             uint32_t tlv32_ring_padding                                      : 32;
             struct   uniform_reo_status_header                                 status_header;
             uint32_t reserved_2                                              : 10,
                      current_index                                           : 10,
                      ssn                                                     : 12;
             uint32_t pn_31_0                                                 : 32;
             uint32_t pn_63_32                                                : 32;
             uint32_t pn_95_64                                                : 32;
             uint32_t pn_127_96                                               : 32;
             uint32_t last_rx_enqueue_timestamp                               : 32;
             uint32_t last_rx_dequeue_timestamp                               : 32;
             uint32_t rx_bitmap_31_0                                          : 32;
             uint32_t rx_bitmap_63_32                                         : 32;
             uint32_t rx_bitmap_95_64                                         : 32;
             uint32_t rx_bitmap_127_96                                        : 32;
             uint32_t rx_bitmap_159_128                                       : 32;
             uint32_t rx_bitmap_191_160                                       : 32;
             uint32_t rx_bitmap_223_192                                       : 32;
             uint32_t rx_bitmap_255_224                                       : 32;
             uint32_t rx_bitmap_287_256                                       : 32;
             uint32_t current_msdu_count                                      : 25,
                      current_mpdu_count                                      :  7;
             uint32_t duplicate_count                                         : 16,
                      forward_due_to_bar_count                                :  6,
                      timeout_count                                           :  6,
                      window_jump_2k                                          :  4;
             uint32_t bar_received_count                                      :  8,
                      frames_in_order_count                                   : 24;
             uint32_t mpdu_frames_processed_count                             : 32;
             uint32_t msdu_frames_processed_count                             : 32;
             uint32_t total_processed_byte_count                              : 32;
             uint32_t reserved_24a                                            :  3,
                      get_queue_1k_stats_status_to_follow                     :  1,
                      hole_count                                              : 16,
                      late_receive_mpdu_count                                 : 12;
             uint32_t looping_count                                           :  4,
                      reserved_25a                                            :  4,
                      aging_drop_interval                                     :  8,
                      aging_drop_mpdu_count                                   : 16;
#endif
};

#define REO_GET_QUEUE_STATS_STATUS_TLV32_RING_PADDING_OFFSET                        0x00000000
#define REO_GET_QUEUE_STATS_STATUS_TLV32_RING_PADDING_LSB                           0
#define REO_GET_QUEUE_STATS_STATUS_TLV32_RING_PADDING_MSB                           31
#define REO_GET_QUEUE_STATS_STATUS_TLV32_RING_PADDING_MASK                          0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_OFFSET           0x00000004
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_LSB              0
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_MSB              15
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_MASK             0x0000ffff

#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_OFFSET          0x00000004
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_LSB             16
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_MSB             25
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_MASK            0x03ff0000

#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_OFFSET    0x00000004
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_LSB       26
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_MSB       27
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_MASK      0x0c000000

#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_RESERVED_0A_OFFSET                 0x00000004
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_RESERVED_0A_LSB                    28
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_RESERVED_0A_MSB                    31
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_RESERVED_0A_MASK                   0xf0000000

#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_TIMESTAMP_OFFSET                   0x00000008
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_TIMESTAMP_LSB                      0
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_TIMESTAMP_MSB                      31
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_TIMESTAMP_MASK                     0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_SSN_OFFSET                                       0x0000000c
#define REO_GET_QUEUE_STATS_STATUS_SSN_LSB                                          0
#define REO_GET_QUEUE_STATS_STATUS_SSN_MSB                                          11
#define REO_GET_QUEUE_STATS_STATUS_SSN_MASK                                         0x00000fff

#define REO_GET_QUEUE_STATS_STATUS_CURRENT_INDEX_OFFSET                             0x0000000c
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_INDEX_LSB                                12
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_INDEX_MSB                                21
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_INDEX_MASK                               0x003ff000

#define REO_GET_QUEUE_STATS_STATUS_RESERVED_2_OFFSET                                0x0000000c
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_2_LSB                                   22
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_2_MSB                                   31
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_2_MASK                                  0xffc00000

#define REO_GET_QUEUE_STATS_STATUS_PN_31_0_OFFSET                                   0x00000010
#define REO_GET_QUEUE_STATS_STATUS_PN_31_0_LSB                                      0
#define REO_GET_QUEUE_STATS_STATUS_PN_31_0_MSB                                      31
#define REO_GET_QUEUE_STATS_STATUS_PN_31_0_MASK                                     0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_PN_63_32_OFFSET                                  0x00000014
#define REO_GET_QUEUE_STATS_STATUS_PN_63_32_LSB                                     0
#define REO_GET_QUEUE_STATS_STATUS_PN_63_32_MSB                                     31
#define REO_GET_QUEUE_STATS_STATUS_PN_63_32_MASK                                    0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_PN_95_64_OFFSET                                  0x00000018
#define REO_GET_QUEUE_STATS_STATUS_PN_95_64_LSB                                     0
#define REO_GET_QUEUE_STATS_STATUS_PN_95_64_MSB                                     31
#define REO_GET_QUEUE_STATS_STATUS_PN_95_64_MASK                                    0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_PN_127_96_OFFSET                                 0x0000001c
#define REO_GET_QUEUE_STATS_STATUS_PN_127_96_LSB                                    0
#define REO_GET_QUEUE_STATS_STATUS_PN_127_96_MSB                                    31
#define REO_GET_QUEUE_STATS_STATUS_PN_127_96_MASK                                   0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_ENQUEUE_TIMESTAMP_OFFSET                 0x00000020
#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_ENQUEUE_TIMESTAMP_LSB                    0
#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_ENQUEUE_TIMESTAMP_MSB                    31
#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_ENQUEUE_TIMESTAMP_MASK                   0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_DEQUEUE_TIMESTAMP_OFFSET                 0x00000024
#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_DEQUEUE_TIMESTAMP_LSB                    0
#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_DEQUEUE_TIMESTAMP_MSB                    31
#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_DEQUEUE_TIMESTAMP_MASK                   0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_31_0_OFFSET                            0x00000028
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_31_0_LSB                               0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_31_0_MSB                               31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_31_0_MASK                              0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_63_32_OFFSET                           0x0000002c
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_63_32_LSB                              0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_63_32_MSB                              31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_63_32_MASK                             0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_95_64_OFFSET                           0x00000030
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_95_64_LSB                              0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_95_64_MSB                              31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_95_64_MASK                             0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_127_96_OFFSET                          0x00000034
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_127_96_LSB                             0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_127_96_MSB                             31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_127_96_MASK                            0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_159_128_OFFSET                         0x00000038
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_159_128_LSB                            0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_159_128_MSB                            31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_159_128_MASK                           0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_191_160_OFFSET                         0x0000003c
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_191_160_LSB                            0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_191_160_MSB                            31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_191_160_MASK                           0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_223_192_OFFSET                         0x00000040
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_223_192_LSB                            0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_223_192_MSB                            31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_223_192_MASK                           0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_255_224_OFFSET                         0x00000044
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_255_224_LSB                            0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_255_224_MSB                            31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_255_224_MASK                           0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_287_256_OFFSET                         0x00000048
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_287_256_LSB                            0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_287_256_MSB                            31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_287_256_MASK                           0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MPDU_COUNT_OFFSET                        0x0000004c
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MPDU_COUNT_LSB                           0
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MPDU_COUNT_MSB                           6
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MPDU_COUNT_MASK                          0x0000007f

#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MSDU_COUNT_OFFSET                        0x0000004c
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MSDU_COUNT_LSB                           7
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MSDU_COUNT_MSB                           31
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MSDU_COUNT_MASK                          0xffffff80

#define REO_GET_QUEUE_STATS_STATUS_WINDOW_JUMP_2K_OFFSET                            0x00000050
#define REO_GET_QUEUE_STATS_STATUS_WINDOW_JUMP_2K_LSB                               0
#define REO_GET_QUEUE_STATS_STATUS_WINDOW_JUMP_2K_MSB                               3
#define REO_GET_QUEUE_STATS_STATUS_WINDOW_JUMP_2K_MASK                              0x0000000f

#define REO_GET_QUEUE_STATS_STATUS_TIMEOUT_COUNT_OFFSET                             0x00000050
#define REO_GET_QUEUE_STATS_STATUS_TIMEOUT_COUNT_LSB                                4
#define REO_GET_QUEUE_STATS_STATUS_TIMEOUT_COUNT_MSB                                9
#define REO_GET_QUEUE_STATS_STATUS_TIMEOUT_COUNT_MASK                               0x000003f0

#define REO_GET_QUEUE_STATS_STATUS_FORWARD_DUE_TO_BAR_COUNT_OFFSET                  0x00000050
#define REO_GET_QUEUE_STATS_STATUS_FORWARD_DUE_TO_BAR_COUNT_LSB                     10
#define REO_GET_QUEUE_STATS_STATUS_FORWARD_DUE_TO_BAR_COUNT_MSB                     15
#define REO_GET_QUEUE_STATS_STATUS_FORWARD_DUE_TO_BAR_COUNT_MASK                    0x0000fc00

#define REO_GET_QUEUE_STATS_STATUS_DUPLICATE_COUNT_OFFSET                           0x00000050
#define REO_GET_QUEUE_STATS_STATUS_DUPLICATE_COUNT_LSB                              16
#define REO_GET_QUEUE_STATS_STATUS_DUPLICATE_COUNT_MSB                              31
#define REO_GET_QUEUE_STATS_STATUS_DUPLICATE_COUNT_MASK                             0xffff0000

#define REO_GET_QUEUE_STATS_STATUS_FRAMES_IN_ORDER_COUNT_OFFSET                     0x00000054
#define REO_GET_QUEUE_STATS_STATUS_FRAMES_IN_ORDER_COUNT_LSB                        0
#define REO_GET_QUEUE_STATS_STATUS_FRAMES_IN_ORDER_COUNT_MSB                        23
#define REO_GET_QUEUE_STATS_STATUS_FRAMES_IN_ORDER_COUNT_MASK                       0x00ffffff

#define REO_GET_QUEUE_STATS_STATUS_BAR_RECEIVED_COUNT_OFFSET                        0x00000054
#define REO_GET_QUEUE_STATS_STATUS_BAR_RECEIVED_COUNT_LSB                           24
#define REO_GET_QUEUE_STATS_STATUS_BAR_RECEIVED_COUNT_MSB                           31
#define REO_GET_QUEUE_STATS_STATUS_BAR_RECEIVED_COUNT_MASK                          0xff000000

#define REO_GET_QUEUE_STATS_STATUS_MPDU_FRAMES_PROCESSED_COUNT_OFFSET               0x00000058
#define REO_GET_QUEUE_STATS_STATUS_MPDU_FRAMES_PROCESSED_COUNT_LSB                  0
#define REO_GET_QUEUE_STATS_STATUS_MPDU_FRAMES_PROCESSED_COUNT_MSB                  31
#define REO_GET_QUEUE_STATS_STATUS_MPDU_FRAMES_PROCESSED_COUNT_MASK                 0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_MSDU_FRAMES_PROCESSED_COUNT_OFFSET               0x0000005c
#define REO_GET_QUEUE_STATS_STATUS_MSDU_FRAMES_PROCESSED_COUNT_LSB                  0
#define REO_GET_QUEUE_STATS_STATUS_MSDU_FRAMES_PROCESSED_COUNT_MSB                  31
#define REO_GET_QUEUE_STATS_STATUS_MSDU_FRAMES_PROCESSED_COUNT_MASK                 0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_TOTAL_PROCESSED_BYTE_COUNT_OFFSET                0x00000060
#define REO_GET_QUEUE_STATS_STATUS_TOTAL_PROCESSED_BYTE_COUNT_LSB                   0
#define REO_GET_QUEUE_STATS_STATUS_TOTAL_PROCESSED_BYTE_COUNT_MSB                   31
#define REO_GET_QUEUE_STATS_STATUS_TOTAL_PROCESSED_BYTE_COUNT_MASK                  0xffffffff

#define REO_GET_QUEUE_STATS_STATUS_LATE_RECEIVE_MPDU_COUNT_OFFSET                   0x00000064
#define REO_GET_QUEUE_STATS_STATUS_LATE_RECEIVE_MPDU_COUNT_LSB                      0
#define REO_GET_QUEUE_STATS_STATUS_LATE_RECEIVE_MPDU_COUNT_MSB                      11
#define REO_GET_QUEUE_STATS_STATUS_LATE_RECEIVE_MPDU_COUNT_MASK                     0x00000fff

#define REO_GET_QUEUE_STATS_STATUS_HOLE_COUNT_OFFSET                                0x00000064
#define REO_GET_QUEUE_STATS_STATUS_HOLE_COUNT_LSB                                   12
#define REO_GET_QUEUE_STATS_STATUS_HOLE_COUNT_MSB                                   27
#define REO_GET_QUEUE_STATS_STATUS_HOLE_COUNT_MASK                                  0x0ffff000

#define REO_GET_QUEUE_STATS_STATUS_GET_QUEUE_1K_STATS_STATUS_TO_FOLLOW_OFFSET       0x00000064
#define REO_GET_QUEUE_STATS_STATUS_GET_QUEUE_1K_STATS_STATUS_TO_FOLLOW_LSB          28
#define REO_GET_QUEUE_STATS_STATUS_GET_QUEUE_1K_STATS_STATUS_TO_FOLLOW_MSB          28
#define REO_GET_QUEUE_STATS_STATUS_GET_QUEUE_1K_STATS_STATUS_TO_FOLLOW_MASK         0x10000000

#define REO_GET_QUEUE_STATS_STATUS_RESERVED_24A_OFFSET                              0x00000064
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_24A_LSB                                 29
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_24A_MSB                                 31
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_24A_MASK                                0xe0000000

#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_MPDU_COUNT_OFFSET                     0x00000068
#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_MPDU_COUNT_LSB                        0
#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_MPDU_COUNT_MSB                        15
#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_MPDU_COUNT_MASK                       0x0000ffff

#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_INTERVAL_OFFSET                       0x00000068
#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_INTERVAL_LSB                          16
#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_INTERVAL_MSB                          23
#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_INTERVAL_MASK                         0x00ff0000

#define REO_GET_QUEUE_STATS_STATUS_RESERVED_25A_OFFSET                              0x00000068
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_25A_LSB                                 24
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_25A_MSB                                 27
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_25A_MASK                                0x0f000000

#define REO_GET_QUEUE_STATS_STATUS_LOOPING_COUNT_OFFSET                             0x00000068
#define REO_GET_QUEUE_STATS_STATUS_LOOPING_COUNT_LSB                                28
#define REO_GET_QUEUE_STATS_STATUS_LOOPING_COUNT_MSB                                31
#define REO_GET_QUEUE_STATS_STATUS_LOOPING_COUNT_MASK                               0xf0000000

#endif
