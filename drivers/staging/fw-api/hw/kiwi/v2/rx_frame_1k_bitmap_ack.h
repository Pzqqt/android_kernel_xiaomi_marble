
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



#ifndef _RX_FRAME_1K_BITMAP_ACK_H_
#define _RX_FRAME_1K_BITMAP_ACK_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_FRAME_1K_BITMAP_ACK 38

#define NUM_OF_QWORDS_RX_FRAME_1K_BITMAP_ACK 19

struct rx_frame_1k_bitmap_ack {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t reserved_0a                                             :  5,
                      ba_bitmap_size                                          :  2,
                      reserved_0b                                             :  3,
                      ba_tid                                                  :  4,
                      sta_full_aid                                            : 13,
                      reserved_0c                                             :  5;
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
             uint32_t ba_ts_bitmap_287_256                                    : 32;
             uint32_t ba_ts_bitmap_319_288                                    : 32;
             uint32_t ba_ts_bitmap_351_320                                    : 32;
             uint32_t ba_ts_bitmap_383_352                                    : 32;
             uint32_t ba_ts_bitmap_415_384                                    : 32;
             uint32_t ba_ts_bitmap_447_416                                    : 32;
             uint32_t ba_ts_bitmap_479_448                                    : 32;
             uint32_t ba_ts_bitmap_511_480                                    : 32;
             uint32_t ba_ts_bitmap_543_512                                    : 32;
             uint32_t ba_ts_bitmap_575_544                                    : 32;
             uint32_t ba_ts_bitmap_607_576                                    : 32;
             uint32_t ba_ts_bitmap_639_608                                    : 32;
             uint32_t ba_ts_bitmap_671_640                                    : 32;
             uint32_t ba_ts_bitmap_703_672                                    : 32;
             uint32_t ba_ts_bitmap_735_704                                    : 32;
             uint32_t ba_ts_bitmap_767_736                                    : 32;
             uint32_t ba_ts_bitmap_799_768                                    : 32;
             uint32_t ba_ts_bitmap_831_800                                    : 32;
             uint32_t ba_ts_bitmap_863_832                                    : 32;
             uint32_t ba_ts_bitmap_895_864                                    : 32;
             uint32_t ba_ts_bitmap_927_896                                    : 32;
             uint32_t ba_ts_bitmap_959_928                                    : 32;
             uint32_t ba_ts_bitmap_991_960                                    : 32;
             uint32_t ba_ts_bitmap_1023_992                                   : 32;
             uint32_t tlv64_padding                                           : 32;
#else
             uint32_t reserved_0c                                             :  5,
                      sta_full_aid                                            : 13,
                      ba_tid                                                  :  4,
                      reserved_0b                                             :  3,
                      ba_bitmap_size                                          :  2,
                      reserved_0a                                             :  5;
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
             uint32_t ba_ts_bitmap_287_256                                    : 32;
             uint32_t ba_ts_bitmap_319_288                                    : 32;
             uint32_t ba_ts_bitmap_351_320                                    : 32;
             uint32_t ba_ts_bitmap_383_352                                    : 32;
             uint32_t ba_ts_bitmap_415_384                                    : 32;
             uint32_t ba_ts_bitmap_447_416                                    : 32;
             uint32_t ba_ts_bitmap_479_448                                    : 32;
             uint32_t ba_ts_bitmap_511_480                                    : 32;
             uint32_t ba_ts_bitmap_543_512                                    : 32;
             uint32_t ba_ts_bitmap_575_544                                    : 32;
             uint32_t ba_ts_bitmap_607_576                                    : 32;
             uint32_t ba_ts_bitmap_639_608                                    : 32;
             uint32_t ba_ts_bitmap_671_640                                    : 32;
             uint32_t ba_ts_bitmap_703_672                                    : 32;
             uint32_t ba_ts_bitmap_735_704                                    : 32;
             uint32_t ba_ts_bitmap_767_736                                    : 32;
             uint32_t ba_ts_bitmap_799_768                                    : 32;
             uint32_t ba_ts_bitmap_831_800                                    : 32;
             uint32_t ba_ts_bitmap_863_832                                    : 32;
             uint32_t ba_ts_bitmap_895_864                                    : 32;
             uint32_t ba_ts_bitmap_927_896                                    : 32;
             uint32_t ba_ts_bitmap_959_928                                    : 32;
             uint32_t ba_ts_bitmap_991_960                                    : 32;
             uint32_t ba_ts_bitmap_1023_992                                   : 32;
             uint32_t tlv64_padding                                           : 32;
#endif
};

#define RX_FRAME_1K_BITMAP_ACK_RESERVED_0A_OFFSET                                   0x0000000000000000
#define RX_FRAME_1K_BITMAP_ACK_RESERVED_0A_LSB                                      0
#define RX_FRAME_1K_BITMAP_ACK_RESERVED_0A_MSB                                      4
#define RX_FRAME_1K_BITMAP_ACK_RESERVED_0A_MASK                                     0x000000000000001f

#define RX_FRAME_1K_BITMAP_ACK_BA_BITMAP_SIZE_OFFSET                                0x0000000000000000
#define RX_FRAME_1K_BITMAP_ACK_BA_BITMAP_SIZE_LSB                                   5
#define RX_FRAME_1K_BITMAP_ACK_BA_BITMAP_SIZE_MSB                                   6
#define RX_FRAME_1K_BITMAP_ACK_BA_BITMAP_SIZE_MASK                                  0x0000000000000060

#define RX_FRAME_1K_BITMAP_ACK_RESERVED_0B_OFFSET                                   0x0000000000000000
#define RX_FRAME_1K_BITMAP_ACK_RESERVED_0B_LSB                                      7
#define RX_FRAME_1K_BITMAP_ACK_RESERVED_0B_MSB                                      9
#define RX_FRAME_1K_BITMAP_ACK_RESERVED_0B_MASK                                     0x0000000000000380

#define RX_FRAME_1K_BITMAP_ACK_BA_TID_OFFSET                                        0x0000000000000000
#define RX_FRAME_1K_BITMAP_ACK_BA_TID_LSB                                           10
#define RX_FRAME_1K_BITMAP_ACK_BA_TID_MSB                                           13
#define RX_FRAME_1K_BITMAP_ACK_BA_TID_MASK                                          0x0000000000003c00

#define RX_FRAME_1K_BITMAP_ACK_STA_FULL_AID_OFFSET                                  0x0000000000000000
#define RX_FRAME_1K_BITMAP_ACK_STA_FULL_AID_LSB                                     14
#define RX_FRAME_1K_BITMAP_ACK_STA_FULL_AID_MSB                                     26
#define RX_FRAME_1K_BITMAP_ACK_STA_FULL_AID_MASK                                    0x0000000007ffc000

#define RX_FRAME_1K_BITMAP_ACK_RESERVED_0C_OFFSET                                   0x0000000000000000
#define RX_FRAME_1K_BITMAP_ACK_RESERVED_0C_LSB                                      27
#define RX_FRAME_1K_BITMAP_ACK_RESERVED_0C_MSB                                      31
#define RX_FRAME_1K_BITMAP_ACK_RESERVED_0C_MASK                                     0x00000000f8000000

#define RX_FRAME_1K_BITMAP_ACK_ADDR1_31_0_OFFSET                                    0x0000000000000000
#define RX_FRAME_1K_BITMAP_ACK_ADDR1_31_0_LSB                                       32
#define RX_FRAME_1K_BITMAP_ACK_ADDR1_31_0_MSB                                       63
#define RX_FRAME_1K_BITMAP_ACK_ADDR1_31_0_MASK                                      0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_ADDR1_47_32_OFFSET                                   0x0000000000000008
#define RX_FRAME_1K_BITMAP_ACK_ADDR1_47_32_LSB                                      0
#define RX_FRAME_1K_BITMAP_ACK_ADDR1_47_32_MSB                                      15
#define RX_FRAME_1K_BITMAP_ACK_ADDR1_47_32_MASK                                     0x000000000000ffff

#define RX_FRAME_1K_BITMAP_ACK_ADDR2_15_0_OFFSET                                    0x0000000000000008
#define RX_FRAME_1K_BITMAP_ACK_ADDR2_15_0_LSB                                       16
#define RX_FRAME_1K_BITMAP_ACK_ADDR2_15_0_MSB                                       31
#define RX_FRAME_1K_BITMAP_ACK_ADDR2_15_0_MASK                                      0x00000000ffff0000

#define RX_FRAME_1K_BITMAP_ACK_ADDR2_47_16_OFFSET                                   0x0000000000000008
#define RX_FRAME_1K_BITMAP_ACK_ADDR2_47_16_LSB                                      32
#define RX_FRAME_1K_BITMAP_ACK_ADDR2_47_16_MSB                                      63
#define RX_FRAME_1K_BITMAP_ACK_ADDR2_47_16_MASK                                     0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_CTRL_OFFSET                                    0x0000000000000010
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_CTRL_LSB                                       0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_CTRL_MSB                                       15
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_CTRL_MASK                                      0x000000000000ffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_SEQ_OFFSET                                     0x0000000000000010
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_SEQ_LSB                                        16
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_SEQ_MSB                                        31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_SEQ_MASK                                       0x00000000ffff0000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_31_0_OFFSET                             0x0000000000000010
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_31_0_LSB                                32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_31_0_MSB                                63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_31_0_MASK                               0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_63_32_OFFSET                            0x0000000000000018
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_63_32_LSB                               0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_63_32_MSB                               31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_63_32_MASK                              0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_95_64_OFFSET                            0x0000000000000018
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_95_64_LSB                               32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_95_64_MSB                               63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_95_64_MASK                              0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_127_96_OFFSET                           0x0000000000000020
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_127_96_LSB                              0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_127_96_MSB                              31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_127_96_MASK                             0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_159_128_OFFSET                          0x0000000000000020
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_159_128_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_159_128_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_159_128_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_191_160_OFFSET                          0x0000000000000028
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_191_160_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_191_160_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_191_160_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_223_192_OFFSET                          0x0000000000000028
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_223_192_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_223_192_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_223_192_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_255_224_OFFSET                          0x0000000000000030
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_255_224_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_255_224_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_255_224_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_287_256_OFFSET                          0x0000000000000030
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_287_256_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_287_256_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_287_256_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_319_288_OFFSET                          0x0000000000000038
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_319_288_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_319_288_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_319_288_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_351_320_OFFSET                          0x0000000000000038
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_351_320_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_351_320_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_351_320_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_383_352_OFFSET                          0x0000000000000040
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_383_352_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_383_352_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_383_352_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_415_384_OFFSET                          0x0000000000000040
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_415_384_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_415_384_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_415_384_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_447_416_OFFSET                          0x0000000000000048
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_447_416_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_447_416_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_447_416_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_479_448_OFFSET                          0x0000000000000048
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_479_448_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_479_448_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_479_448_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_511_480_OFFSET                          0x0000000000000050
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_511_480_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_511_480_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_511_480_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_543_512_OFFSET                          0x0000000000000050
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_543_512_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_543_512_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_543_512_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_575_544_OFFSET                          0x0000000000000058
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_575_544_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_575_544_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_575_544_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_607_576_OFFSET                          0x0000000000000058
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_607_576_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_607_576_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_607_576_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_639_608_OFFSET                          0x0000000000000060
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_639_608_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_639_608_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_639_608_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_671_640_OFFSET                          0x0000000000000060
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_671_640_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_671_640_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_671_640_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_703_672_OFFSET                          0x0000000000000068
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_703_672_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_703_672_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_703_672_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_735_704_OFFSET                          0x0000000000000068
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_735_704_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_735_704_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_735_704_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_767_736_OFFSET                          0x0000000000000070
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_767_736_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_767_736_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_767_736_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_799_768_OFFSET                          0x0000000000000070
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_799_768_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_799_768_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_799_768_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_831_800_OFFSET                          0x0000000000000078
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_831_800_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_831_800_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_831_800_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_863_832_OFFSET                          0x0000000000000078
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_863_832_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_863_832_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_863_832_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_895_864_OFFSET                          0x0000000000000080
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_895_864_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_895_864_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_895_864_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_927_896_OFFSET                          0x0000000000000080
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_927_896_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_927_896_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_927_896_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_959_928_OFFSET                          0x0000000000000088
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_959_928_LSB                             0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_959_928_MSB                             31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_959_928_MASK                            0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_991_960_OFFSET                          0x0000000000000088
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_991_960_LSB                             32
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_991_960_MSB                             63
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_991_960_MASK                            0xffffffff00000000

#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_1023_992_OFFSET                         0x0000000000000090
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_1023_992_LSB                            0
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_1023_992_MSB                            31
#define RX_FRAME_1K_BITMAP_ACK_BA_TS_BITMAP_1023_992_MASK                           0x00000000ffffffff

#define RX_FRAME_1K_BITMAP_ACK_TLV64_PADDING_OFFSET                                 0x0000000000000090
#define RX_FRAME_1K_BITMAP_ACK_TLV64_PADDING_LSB                                    32
#define RX_FRAME_1K_BITMAP_ACK_TLV64_PADDING_MSB                                    63
#define RX_FRAME_1K_BITMAP_ACK_TLV64_PADDING_MASK                                   0xffffffff00000000

#endif
