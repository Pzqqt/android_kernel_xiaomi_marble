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


#ifndef _RX_FRAME_BITMAP_ACK_H_
#define _RX_FRAME_BITMAP_ACK_H_

#define NUM_OF_DWORDS_RX_FRAME_BITMAP_ACK 13

struct rx_frame_bitmap_ack {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t no_bitmap_available                                     :  1,
                      explicit_ack                                            :  1,
                      explict_ack_type                                        :  3,
                      ba_bitmap_size                                          :  2,
                      reserved_0a                                             :  3,
                      ba_tid                                                  :  4,
                      sta_full_aid                                            : 13,
                      reserved_0b                                             :  5;
             uint32_t addr1_31_0                                              : 32;
             uint32_t addr1_47_32                                             : 16,
                      addr2_15_0                                              : 16;
             uint32_t addr2_47_16                                             : 32;
             uint32_t ba_ts_ctrl                                              : 16,
                      ba_ts_seq                                               : 16;
             uint32_t ba_ts_bitmap_31_0                                       : 32;
             uint32_t ba_ts_bitmap_63_32                                      : 32;
             uint32_t ba_ts_bitmap_95_64                                      : 32;
             uint32_t ba_ts_bitmap_127_96                                     : 32;
             uint32_t ba_ts_bitmap_159_128                                    : 32;
             uint32_t ba_ts_bitmap_191_160                                    : 32;
             uint32_t ba_ts_bitmap_223_192                                    : 32;
             uint32_t ba_ts_bitmap_255_224                                    : 32;
#else
             uint32_t reserved_0b                                             :  5,
                      sta_full_aid                                            : 13,
                      ba_tid                                                  :  4,
                      reserved_0a                                             :  3,
                      ba_bitmap_size                                          :  2,
                      explict_ack_type                                        :  3,
                      explicit_ack                                            :  1,
                      no_bitmap_available                                     :  1;
             uint32_t addr1_31_0                                              : 32;
             uint32_t addr2_15_0                                              : 16,
                      addr1_47_32                                             : 16;
             uint32_t addr2_47_16                                             : 32;
             uint32_t ba_ts_seq                                               : 16,
                      ba_ts_ctrl                                              : 16;
             uint32_t ba_ts_bitmap_31_0                                       : 32;
             uint32_t ba_ts_bitmap_63_32                                      : 32;
             uint32_t ba_ts_bitmap_95_64                                      : 32;
             uint32_t ba_ts_bitmap_127_96                                     : 32;
             uint32_t ba_ts_bitmap_159_128                                    : 32;
             uint32_t ba_ts_bitmap_191_160                                    : 32;
             uint32_t ba_ts_bitmap_223_192                                    : 32;
             uint32_t ba_ts_bitmap_255_224                                    : 32;
#endif
};

#define RX_FRAME_BITMAP_ACK_NO_BITMAP_AVAILABLE_OFFSET                              0x00000000
#define RX_FRAME_BITMAP_ACK_NO_BITMAP_AVAILABLE_LSB                                 0
#define RX_FRAME_BITMAP_ACK_NO_BITMAP_AVAILABLE_MSB                                 0
#define RX_FRAME_BITMAP_ACK_NO_BITMAP_AVAILABLE_MASK                                0x00000001

#define RX_FRAME_BITMAP_ACK_EXPLICIT_ACK_OFFSET                                     0x00000000
#define RX_FRAME_BITMAP_ACK_EXPLICIT_ACK_LSB                                        1
#define RX_FRAME_BITMAP_ACK_EXPLICIT_ACK_MSB                                        1
#define RX_FRAME_BITMAP_ACK_EXPLICIT_ACK_MASK                                       0x00000002

#define RX_FRAME_BITMAP_ACK_EXPLICT_ACK_TYPE_OFFSET                                 0x00000000
#define RX_FRAME_BITMAP_ACK_EXPLICT_ACK_TYPE_LSB                                    2
#define RX_FRAME_BITMAP_ACK_EXPLICT_ACK_TYPE_MSB                                    4
#define RX_FRAME_BITMAP_ACK_EXPLICT_ACK_TYPE_MASK                                   0x0000001c

#define RX_FRAME_BITMAP_ACK_BA_BITMAP_SIZE_OFFSET                                   0x00000000
#define RX_FRAME_BITMAP_ACK_BA_BITMAP_SIZE_LSB                                      5
#define RX_FRAME_BITMAP_ACK_BA_BITMAP_SIZE_MSB                                      6
#define RX_FRAME_BITMAP_ACK_BA_BITMAP_SIZE_MASK                                     0x00000060

#define RX_FRAME_BITMAP_ACK_RESERVED_0A_OFFSET                                      0x00000000
#define RX_FRAME_BITMAP_ACK_RESERVED_0A_LSB                                         7
#define RX_FRAME_BITMAP_ACK_RESERVED_0A_MSB                                         9
#define RX_FRAME_BITMAP_ACK_RESERVED_0A_MASK                                        0x00000380

#define RX_FRAME_BITMAP_ACK_BA_TID_OFFSET                                           0x00000000
#define RX_FRAME_BITMAP_ACK_BA_TID_LSB                                              10
#define RX_FRAME_BITMAP_ACK_BA_TID_MSB                                              13
#define RX_FRAME_BITMAP_ACK_BA_TID_MASK                                             0x00003c00

#define RX_FRAME_BITMAP_ACK_STA_FULL_AID_OFFSET                                     0x00000000
#define RX_FRAME_BITMAP_ACK_STA_FULL_AID_LSB                                        14
#define RX_FRAME_BITMAP_ACK_STA_FULL_AID_MSB                                        26
#define RX_FRAME_BITMAP_ACK_STA_FULL_AID_MASK                                       0x07ffc000

#define RX_FRAME_BITMAP_ACK_RESERVED_0B_OFFSET                                      0x00000000
#define RX_FRAME_BITMAP_ACK_RESERVED_0B_LSB                                         27
#define RX_FRAME_BITMAP_ACK_RESERVED_0B_MSB                                         31
#define RX_FRAME_BITMAP_ACK_RESERVED_0B_MASK                                        0xf8000000

#define RX_FRAME_BITMAP_ACK_ADDR1_31_0_OFFSET                                       0x00000004
#define RX_FRAME_BITMAP_ACK_ADDR1_31_0_LSB                                          0
#define RX_FRAME_BITMAP_ACK_ADDR1_31_0_MSB                                          31
#define RX_FRAME_BITMAP_ACK_ADDR1_31_0_MASK                                         0xffffffff

#define RX_FRAME_BITMAP_ACK_ADDR1_47_32_OFFSET                                      0x00000008
#define RX_FRAME_BITMAP_ACK_ADDR1_47_32_LSB                                         0
#define RX_FRAME_BITMAP_ACK_ADDR1_47_32_MSB                                         15
#define RX_FRAME_BITMAP_ACK_ADDR1_47_32_MASK                                        0x0000ffff

#define RX_FRAME_BITMAP_ACK_ADDR2_15_0_OFFSET                                       0x00000008
#define RX_FRAME_BITMAP_ACK_ADDR2_15_0_LSB                                          16
#define RX_FRAME_BITMAP_ACK_ADDR2_15_0_MSB                                          31
#define RX_FRAME_BITMAP_ACK_ADDR2_15_0_MASK                                         0xffff0000

#define RX_FRAME_BITMAP_ACK_ADDR2_47_16_OFFSET                                      0x0000000c
#define RX_FRAME_BITMAP_ACK_ADDR2_47_16_LSB                                         0
#define RX_FRAME_BITMAP_ACK_ADDR2_47_16_MSB                                         31
#define RX_FRAME_BITMAP_ACK_ADDR2_47_16_MASK                                        0xffffffff

#define RX_FRAME_BITMAP_ACK_BA_TS_CTRL_OFFSET                                       0x00000010
#define RX_FRAME_BITMAP_ACK_BA_TS_CTRL_LSB                                          0
#define RX_FRAME_BITMAP_ACK_BA_TS_CTRL_MSB                                          15
#define RX_FRAME_BITMAP_ACK_BA_TS_CTRL_MASK                                         0x0000ffff

#define RX_FRAME_BITMAP_ACK_BA_TS_SEQ_OFFSET                                        0x00000010
#define RX_FRAME_BITMAP_ACK_BA_TS_SEQ_LSB                                           16
#define RX_FRAME_BITMAP_ACK_BA_TS_SEQ_MSB                                           31
#define RX_FRAME_BITMAP_ACK_BA_TS_SEQ_MASK                                          0xffff0000

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_31_0_OFFSET                                0x00000014
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_31_0_LSB                                   0
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_31_0_MSB                                   31
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_31_0_MASK                                  0xffffffff

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_63_32_OFFSET                               0x00000018
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_63_32_LSB                                  0
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_63_32_MSB                                  31
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_63_32_MASK                                 0xffffffff

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_95_64_OFFSET                               0x0000001c
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_95_64_LSB                                  0
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_95_64_MSB                                  31
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_95_64_MASK                                 0xffffffff

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_127_96_OFFSET                              0x00000020
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_127_96_LSB                                 0
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_127_96_MSB                                 31
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_127_96_MASK                                0xffffffff

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_159_128_OFFSET                             0x00000024
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_159_128_LSB                                0
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_159_128_MSB                                31
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_159_128_MASK                               0xffffffff

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_191_160_OFFSET                             0x00000028
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_191_160_LSB                                0
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_191_160_MSB                                31
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_191_160_MASK                               0xffffffff

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_223_192_OFFSET                             0x0000002c
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_223_192_LSB                                0
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_223_192_MSB                                31
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_223_192_MASK                               0xffffffff

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_255_224_OFFSET                             0x00000030
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_255_224_LSB                                0
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_255_224_MSB                                31
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_255_224_MASK                               0xffffffff

#endif
