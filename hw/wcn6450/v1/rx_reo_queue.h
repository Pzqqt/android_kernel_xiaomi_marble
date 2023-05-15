
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



#ifndef _RX_REO_QUEUE_H_
#define _RX_REO_QUEUE_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_descriptor_header.h"

#define NUM_OF_DWORDS_RX_REO_QUEUE 32

struct rx_reo_queue {
    struct            uniform_descriptor_header                       descriptor_header;
             uint32_t receive_queue_number            : 16,
                      reserved_1b                     : 16;
             uint32_t vld                             :  1,
                      associated_link_descriptor_counter:  2,
                      disable_duplicate_detection     :  1,
                      soft_reorder_enable             :  1,
                      ac                              :  2,
                      bar                             :  1,
                      rty                             :  1,
                      chk_2k_mode                     :  1,
                      oor_mode                        :  1,
                      ba_window_size                  :  8,
                      pn_check_needed                 :  1,
                      pn_shall_be_even                :  1,
                      pn_shall_be_uneven              :  1,
                      pn_handling_enable              :  1,
                      pn_size                         :  2,
                      ignore_ampdu_flag               :  1,
                      reserved_2b                     :  6;
             uint32_t svld                            :  1,
                      ssn                             : 12,
                      current_index                   :  8,
                      seq_2k_error_detected_flag      :  1,
                      pn_error_detected_flag          :  1,
                      reserved_3a                     :  8,
                      pn_valid                        :  1;
             uint32_t pn_31_0                         : 32;
             uint32_t pn_63_32                        : 32;
             uint32_t pn_95_64                        : 32;
             uint32_t pn_127_96                       : 32;
             uint32_t last_rx_enqueue_timestamp       : 32;
             uint32_t last_rx_dequeue_timestamp       : 32;
             uint32_t ptr_to_next_aging_queue_31_0    : 32;
             uint32_t ptr_to_next_aging_queue_39_32   :  8,
                      reserved_11a                    : 24;
             uint32_t ptr_to_previous_aging_queue_31_0: 32;
             uint32_t ptr_to_previous_aging_queue_39_32:  8,
                      reserved_13a                    : 24;
             uint32_t rx_bitmap_31_0                  : 32;
             uint32_t rx_bitmap_63_32                 : 32;
             uint32_t rx_bitmap_95_64                 : 32;
             uint32_t rx_bitmap_127_96                : 32;
             uint32_t rx_bitmap_159_128               : 32;
             uint32_t rx_bitmap_191_160               : 32;
             uint32_t rx_bitmap_223_192               : 32;
             uint32_t rx_bitmap_255_224               : 32;
             uint32_t current_mpdu_count              :  7,
                      current_msdu_count              : 25;
             uint32_t reserved_23                     :  4,
                      timeout_count                   :  6,
                      forward_due_to_bar_count        :  6,
                      duplicate_count                 : 16;
             uint32_t frames_in_order_count           : 24,
                      bar_received_count              :  8;
             uint32_t mpdu_frames_processed_count     : 32;
             uint32_t msdu_frames_processed_count     : 32;
             uint32_t total_processed_byte_count      : 32;
             uint32_t late_receive_mpdu_count         : 12,
                      window_jump_2k                  :  4,
                      hole_count                      : 16;
             uint32_t reserved_29                     : 32;
             uint32_t reserved_30                     : 32;
             uint32_t reserved_31                     : 32;
};

#define RX_REO_QUEUE_0_DESCRIPTOR_HEADER_OWNER_OFFSET                0x00000000
#define RX_REO_QUEUE_0_DESCRIPTOR_HEADER_OWNER_LSB                   0
#define RX_REO_QUEUE_0_DESCRIPTOR_HEADER_OWNER_MASK                  0x0000000f

#define RX_REO_QUEUE_0_DESCRIPTOR_HEADER_BUFFER_TYPE_OFFSET          0x00000000
#define RX_REO_QUEUE_0_DESCRIPTOR_HEADER_BUFFER_TYPE_LSB             4
#define RX_REO_QUEUE_0_DESCRIPTOR_HEADER_BUFFER_TYPE_MASK            0x000000f0

#define RX_REO_QUEUE_0_DESCRIPTOR_HEADER_RESERVED_0A_OFFSET          0x00000000
#define RX_REO_QUEUE_0_DESCRIPTOR_HEADER_RESERVED_0A_LSB             8
#define RX_REO_QUEUE_0_DESCRIPTOR_HEADER_RESERVED_0A_MASK            0xffffff00

#define RX_REO_QUEUE_1_RECEIVE_QUEUE_NUMBER_OFFSET                   0x00000004
#define RX_REO_QUEUE_1_RECEIVE_QUEUE_NUMBER_LSB                      0
#define RX_REO_QUEUE_1_RECEIVE_QUEUE_NUMBER_MASK                     0x0000ffff

#define RX_REO_QUEUE_1_RESERVED_1B_OFFSET                            0x00000004
#define RX_REO_QUEUE_1_RESERVED_1B_LSB                               16
#define RX_REO_QUEUE_1_RESERVED_1B_MASK                              0xffff0000

#define RX_REO_QUEUE_2_VLD_OFFSET                                    0x00000008
#define RX_REO_QUEUE_2_VLD_LSB                                       0
#define RX_REO_QUEUE_2_VLD_MASK                                      0x00000001

#define RX_REO_QUEUE_2_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_OFFSET     0x00000008
#define RX_REO_QUEUE_2_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_LSB        1
#define RX_REO_QUEUE_2_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MASK       0x00000006

#define RX_REO_QUEUE_2_DISABLE_DUPLICATE_DETECTION_OFFSET            0x00000008
#define RX_REO_QUEUE_2_DISABLE_DUPLICATE_DETECTION_LSB               3
#define RX_REO_QUEUE_2_DISABLE_DUPLICATE_DETECTION_MASK              0x00000008

#define RX_REO_QUEUE_2_SOFT_REORDER_ENABLE_OFFSET                    0x00000008
#define RX_REO_QUEUE_2_SOFT_REORDER_ENABLE_LSB                       4
#define RX_REO_QUEUE_2_SOFT_REORDER_ENABLE_MASK                      0x00000010

#define RX_REO_QUEUE_2_AC_OFFSET                                     0x00000008
#define RX_REO_QUEUE_2_AC_LSB                                        5
#define RX_REO_QUEUE_2_AC_MASK                                       0x00000060

#define RX_REO_QUEUE_2_BAR_OFFSET                                    0x00000008
#define RX_REO_QUEUE_2_BAR_LSB                                       7
#define RX_REO_QUEUE_2_BAR_MASK                                      0x00000080

#define RX_REO_QUEUE_2_RTY_OFFSET                                    0x00000008
#define RX_REO_QUEUE_2_RTY_LSB                                       8
#define RX_REO_QUEUE_2_RTY_MASK                                      0x00000100

#define RX_REO_QUEUE_2_CHK_2K_MODE_OFFSET                            0x00000008
#define RX_REO_QUEUE_2_CHK_2K_MODE_LSB                               9
#define RX_REO_QUEUE_2_CHK_2K_MODE_MASK                              0x00000200

#define RX_REO_QUEUE_2_OOR_MODE_OFFSET                               0x00000008
#define RX_REO_QUEUE_2_OOR_MODE_LSB                                  10
#define RX_REO_QUEUE_2_OOR_MODE_MASK                                 0x00000400

#define RX_REO_QUEUE_2_BA_WINDOW_SIZE_OFFSET                         0x00000008
#define RX_REO_QUEUE_2_BA_WINDOW_SIZE_LSB                            11
#define RX_REO_QUEUE_2_BA_WINDOW_SIZE_MASK                           0x0007f800

#define RX_REO_QUEUE_2_PN_CHECK_NEEDED_OFFSET                        0x00000008
#define RX_REO_QUEUE_2_PN_CHECK_NEEDED_LSB                           19
#define RX_REO_QUEUE_2_PN_CHECK_NEEDED_MASK                          0x00080000

#define RX_REO_QUEUE_2_PN_SHALL_BE_EVEN_OFFSET                       0x00000008
#define RX_REO_QUEUE_2_PN_SHALL_BE_EVEN_LSB                          20
#define RX_REO_QUEUE_2_PN_SHALL_BE_EVEN_MASK                         0x00100000

#define RX_REO_QUEUE_2_PN_SHALL_BE_UNEVEN_OFFSET                     0x00000008
#define RX_REO_QUEUE_2_PN_SHALL_BE_UNEVEN_LSB                        21
#define RX_REO_QUEUE_2_PN_SHALL_BE_UNEVEN_MASK                       0x00200000

#define RX_REO_QUEUE_2_PN_HANDLING_ENABLE_OFFSET                     0x00000008
#define RX_REO_QUEUE_2_PN_HANDLING_ENABLE_LSB                        22
#define RX_REO_QUEUE_2_PN_HANDLING_ENABLE_MASK                       0x00400000

#define RX_REO_QUEUE_2_PN_SIZE_OFFSET                                0x00000008
#define RX_REO_QUEUE_2_PN_SIZE_LSB                                   23
#define RX_REO_QUEUE_2_PN_SIZE_MASK                                  0x01800000

#define RX_REO_QUEUE_2_IGNORE_AMPDU_FLAG_OFFSET                      0x00000008
#define RX_REO_QUEUE_2_IGNORE_AMPDU_FLAG_LSB                         25
#define RX_REO_QUEUE_2_IGNORE_AMPDU_FLAG_MASK                        0x02000000

#define RX_REO_QUEUE_2_RESERVED_2B_OFFSET                            0x00000008
#define RX_REO_QUEUE_2_RESERVED_2B_LSB                               26
#define RX_REO_QUEUE_2_RESERVED_2B_MASK                              0xfc000000

#define RX_REO_QUEUE_3_SVLD_OFFSET                                   0x0000000c
#define RX_REO_QUEUE_3_SVLD_LSB                                      0
#define RX_REO_QUEUE_3_SVLD_MASK                                     0x00000001

#define RX_REO_QUEUE_3_SSN_OFFSET                                    0x0000000c
#define RX_REO_QUEUE_3_SSN_LSB                                       1
#define RX_REO_QUEUE_3_SSN_MASK                                      0x00001ffe

#define RX_REO_QUEUE_3_CURRENT_INDEX_OFFSET                          0x0000000c
#define RX_REO_QUEUE_3_CURRENT_INDEX_LSB                             13
#define RX_REO_QUEUE_3_CURRENT_INDEX_MASK                            0x001fe000

#define RX_REO_QUEUE_3_SEQ_2K_ERROR_DETECTED_FLAG_OFFSET             0x0000000c
#define RX_REO_QUEUE_3_SEQ_2K_ERROR_DETECTED_FLAG_LSB                21
#define RX_REO_QUEUE_3_SEQ_2K_ERROR_DETECTED_FLAG_MASK               0x00200000

#define RX_REO_QUEUE_3_PN_ERROR_DETECTED_FLAG_OFFSET                 0x0000000c
#define RX_REO_QUEUE_3_PN_ERROR_DETECTED_FLAG_LSB                    22
#define RX_REO_QUEUE_3_PN_ERROR_DETECTED_FLAG_MASK                   0x00400000

#define RX_REO_QUEUE_3_RESERVED_3A_OFFSET                            0x0000000c
#define RX_REO_QUEUE_3_RESERVED_3A_LSB                               23
#define RX_REO_QUEUE_3_RESERVED_3A_MASK                              0x7f800000

#define RX_REO_QUEUE_3_PN_VALID_OFFSET                               0x0000000c
#define RX_REO_QUEUE_3_PN_VALID_LSB                                  31
#define RX_REO_QUEUE_3_PN_VALID_MASK                                 0x80000000

#define RX_REO_QUEUE_4_PN_31_0_OFFSET                                0x00000010
#define RX_REO_QUEUE_4_PN_31_0_LSB                                   0
#define RX_REO_QUEUE_4_PN_31_0_MASK                                  0xffffffff

#define RX_REO_QUEUE_5_PN_63_32_OFFSET                               0x00000014
#define RX_REO_QUEUE_5_PN_63_32_LSB                                  0
#define RX_REO_QUEUE_5_PN_63_32_MASK                                 0xffffffff

#define RX_REO_QUEUE_6_PN_95_64_OFFSET                               0x00000018
#define RX_REO_QUEUE_6_PN_95_64_LSB                                  0
#define RX_REO_QUEUE_6_PN_95_64_MASK                                 0xffffffff

#define RX_REO_QUEUE_7_PN_127_96_OFFSET                              0x0000001c
#define RX_REO_QUEUE_7_PN_127_96_LSB                                 0
#define RX_REO_QUEUE_7_PN_127_96_MASK                                0xffffffff

#define RX_REO_QUEUE_8_LAST_RX_ENQUEUE_TIMESTAMP_OFFSET              0x00000020
#define RX_REO_QUEUE_8_LAST_RX_ENQUEUE_TIMESTAMP_LSB                 0
#define RX_REO_QUEUE_8_LAST_RX_ENQUEUE_TIMESTAMP_MASK                0xffffffff

#define RX_REO_QUEUE_9_LAST_RX_DEQUEUE_TIMESTAMP_OFFSET              0x00000024
#define RX_REO_QUEUE_9_LAST_RX_DEQUEUE_TIMESTAMP_LSB                 0
#define RX_REO_QUEUE_9_LAST_RX_DEQUEUE_TIMESTAMP_MASK                0xffffffff

#define RX_REO_QUEUE_10_PTR_TO_NEXT_AGING_QUEUE_31_0_OFFSET          0x00000028
#define RX_REO_QUEUE_10_PTR_TO_NEXT_AGING_QUEUE_31_0_LSB             0
#define RX_REO_QUEUE_10_PTR_TO_NEXT_AGING_QUEUE_31_0_MASK            0xffffffff

#define RX_REO_QUEUE_11_PTR_TO_NEXT_AGING_QUEUE_39_32_OFFSET         0x0000002c
#define RX_REO_QUEUE_11_PTR_TO_NEXT_AGING_QUEUE_39_32_LSB            0
#define RX_REO_QUEUE_11_PTR_TO_NEXT_AGING_QUEUE_39_32_MASK           0x000000ff

#define RX_REO_QUEUE_11_RESERVED_11A_OFFSET                          0x0000002c
#define RX_REO_QUEUE_11_RESERVED_11A_LSB                             8
#define RX_REO_QUEUE_11_RESERVED_11A_MASK                            0xffffff00

#define RX_REO_QUEUE_12_PTR_TO_PREVIOUS_AGING_QUEUE_31_0_OFFSET      0x00000030
#define RX_REO_QUEUE_12_PTR_TO_PREVIOUS_AGING_QUEUE_31_0_LSB         0
#define RX_REO_QUEUE_12_PTR_TO_PREVIOUS_AGING_QUEUE_31_0_MASK        0xffffffff

#define RX_REO_QUEUE_13_PTR_TO_PREVIOUS_AGING_QUEUE_39_32_OFFSET     0x00000034
#define RX_REO_QUEUE_13_PTR_TO_PREVIOUS_AGING_QUEUE_39_32_LSB        0
#define RX_REO_QUEUE_13_PTR_TO_PREVIOUS_AGING_QUEUE_39_32_MASK       0x000000ff

#define RX_REO_QUEUE_13_RESERVED_13A_OFFSET                          0x00000034
#define RX_REO_QUEUE_13_RESERVED_13A_LSB                             8
#define RX_REO_QUEUE_13_RESERVED_13A_MASK                            0xffffff00

#define RX_REO_QUEUE_14_RX_BITMAP_31_0_OFFSET                        0x00000038
#define RX_REO_QUEUE_14_RX_BITMAP_31_0_LSB                           0
#define RX_REO_QUEUE_14_RX_BITMAP_31_0_MASK                          0xffffffff

#define RX_REO_QUEUE_15_RX_BITMAP_63_32_OFFSET                       0x0000003c
#define RX_REO_QUEUE_15_RX_BITMAP_63_32_LSB                          0
#define RX_REO_QUEUE_15_RX_BITMAP_63_32_MASK                         0xffffffff

#define RX_REO_QUEUE_16_RX_BITMAP_95_64_OFFSET                       0x00000040
#define RX_REO_QUEUE_16_RX_BITMAP_95_64_LSB                          0
#define RX_REO_QUEUE_16_RX_BITMAP_95_64_MASK                         0xffffffff

#define RX_REO_QUEUE_17_RX_BITMAP_127_96_OFFSET                      0x00000044
#define RX_REO_QUEUE_17_RX_BITMAP_127_96_LSB                         0
#define RX_REO_QUEUE_17_RX_BITMAP_127_96_MASK                        0xffffffff

#define RX_REO_QUEUE_18_RX_BITMAP_159_128_OFFSET                     0x00000048
#define RX_REO_QUEUE_18_RX_BITMAP_159_128_LSB                        0
#define RX_REO_QUEUE_18_RX_BITMAP_159_128_MASK                       0xffffffff

#define RX_REO_QUEUE_19_RX_BITMAP_191_160_OFFSET                     0x0000004c
#define RX_REO_QUEUE_19_RX_BITMAP_191_160_LSB                        0
#define RX_REO_QUEUE_19_RX_BITMAP_191_160_MASK                       0xffffffff

#define RX_REO_QUEUE_20_RX_BITMAP_223_192_OFFSET                     0x00000050
#define RX_REO_QUEUE_20_RX_BITMAP_223_192_LSB                        0
#define RX_REO_QUEUE_20_RX_BITMAP_223_192_MASK                       0xffffffff

#define RX_REO_QUEUE_21_RX_BITMAP_255_224_OFFSET                     0x00000054
#define RX_REO_QUEUE_21_RX_BITMAP_255_224_LSB                        0
#define RX_REO_QUEUE_21_RX_BITMAP_255_224_MASK                       0xffffffff

#define RX_REO_QUEUE_22_CURRENT_MPDU_COUNT_OFFSET                    0x00000058
#define RX_REO_QUEUE_22_CURRENT_MPDU_COUNT_LSB                       0
#define RX_REO_QUEUE_22_CURRENT_MPDU_COUNT_MASK                      0x0000007f

#define RX_REO_QUEUE_22_CURRENT_MSDU_COUNT_OFFSET                    0x00000058
#define RX_REO_QUEUE_22_CURRENT_MSDU_COUNT_LSB                       7
#define RX_REO_QUEUE_22_CURRENT_MSDU_COUNT_MASK                      0xffffff80

#define RX_REO_QUEUE_23_RESERVED_23_OFFSET                           0x0000005c
#define RX_REO_QUEUE_23_RESERVED_23_LSB                              0
#define RX_REO_QUEUE_23_RESERVED_23_MASK                             0x0000000f

#define RX_REO_QUEUE_23_TIMEOUT_COUNT_OFFSET                         0x0000005c
#define RX_REO_QUEUE_23_TIMEOUT_COUNT_LSB                            4
#define RX_REO_QUEUE_23_TIMEOUT_COUNT_MASK                           0x000003f0

#define RX_REO_QUEUE_23_FORWARD_DUE_TO_BAR_COUNT_OFFSET              0x0000005c
#define RX_REO_QUEUE_23_FORWARD_DUE_TO_BAR_COUNT_LSB                 10
#define RX_REO_QUEUE_23_FORWARD_DUE_TO_BAR_COUNT_MASK                0x0000fc00

#define RX_REO_QUEUE_23_DUPLICATE_COUNT_OFFSET                       0x0000005c
#define RX_REO_QUEUE_23_DUPLICATE_COUNT_LSB                          16
#define RX_REO_QUEUE_23_DUPLICATE_COUNT_MASK                         0xffff0000

#define RX_REO_QUEUE_24_FRAMES_IN_ORDER_COUNT_OFFSET                 0x00000060
#define RX_REO_QUEUE_24_FRAMES_IN_ORDER_COUNT_LSB                    0
#define RX_REO_QUEUE_24_FRAMES_IN_ORDER_COUNT_MASK                   0x00ffffff

#define RX_REO_QUEUE_24_BAR_RECEIVED_COUNT_OFFSET                    0x00000060
#define RX_REO_QUEUE_24_BAR_RECEIVED_COUNT_LSB                       24
#define RX_REO_QUEUE_24_BAR_RECEIVED_COUNT_MASK                      0xff000000

#define RX_REO_QUEUE_25_MPDU_FRAMES_PROCESSED_COUNT_OFFSET           0x00000064
#define RX_REO_QUEUE_25_MPDU_FRAMES_PROCESSED_COUNT_LSB              0
#define RX_REO_QUEUE_25_MPDU_FRAMES_PROCESSED_COUNT_MASK             0xffffffff

#define RX_REO_QUEUE_26_MSDU_FRAMES_PROCESSED_COUNT_OFFSET           0x00000068
#define RX_REO_QUEUE_26_MSDU_FRAMES_PROCESSED_COUNT_LSB              0
#define RX_REO_QUEUE_26_MSDU_FRAMES_PROCESSED_COUNT_MASK             0xffffffff

#define RX_REO_QUEUE_27_TOTAL_PROCESSED_BYTE_COUNT_OFFSET            0x0000006c
#define RX_REO_QUEUE_27_TOTAL_PROCESSED_BYTE_COUNT_LSB               0
#define RX_REO_QUEUE_27_TOTAL_PROCESSED_BYTE_COUNT_MASK              0xffffffff

#define RX_REO_QUEUE_28_LATE_RECEIVE_MPDU_COUNT_OFFSET               0x00000070
#define RX_REO_QUEUE_28_LATE_RECEIVE_MPDU_COUNT_LSB                  0
#define RX_REO_QUEUE_28_LATE_RECEIVE_MPDU_COUNT_MASK                 0x00000fff

#define RX_REO_QUEUE_28_WINDOW_JUMP_2K_OFFSET                        0x00000070
#define RX_REO_QUEUE_28_WINDOW_JUMP_2K_LSB                           12
#define RX_REO_QUEUE_28_WINDOW_JUMP_2K_MASK                          0x0000f000

#define RX_REO_QUEUE_28_HOLE_COUNT_OFFSET                            0x00000070
#define RX_REO_QUEUE_28_HOLE_COUNT_LSB                               16
#define RX_REO_QUEUE_28_HOLE_COUNT_MASK                              0xffff0000

#define RX_REO_QUEUE_29_RESERVED_29_OFFSET                           0x00000074
#define RX_REO_QUEUE_29_RESERVED_29_LSB                              0
#define RX_REO_QUEUE_29_RESERVED_29_MASK                             0xffffffff

#define RX_REO_QUEUE_30_RESERVED_30_OFFSET                           0x00000078
#define RX_REO_QUEUE_30_RESERVED_30_LSB                              0
#define RX_REO_QUEUE_30_RESERVED_30_MASK                             0xffffffff

#define RX_REO_QUEUE_31_RESERVED_31_OFFSET                           0x0000007c
#define RX_REO_QUEUE_31_RESERVED_31_LSB                              0
#define RX_REO_QUEUE_31_RESERVED_31_MASK                             0xffffffff

#endif
