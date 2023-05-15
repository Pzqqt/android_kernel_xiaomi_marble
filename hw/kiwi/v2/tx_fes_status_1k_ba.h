
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



#ifndef _TX_FES_STATUS_1K_BA_H_
#define _TX_FES_STATUS_1K_BA_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_FES_STATUS_1K_BA 34

#define NUM_OF_QWORDS_TX_FES_STATUS_1K_BA 17

struct tx_fes_status_1k_ba {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ack_ba_status_type                                      :  1,
                      ba_type                                                 :  1,
                      ba_tid                                                  :  4,
                      unexpected_ack_or_ba                                    :  1,
                      response_timeout                                        :  1,
                      ack_frame_rssi                                          :  8,
                      ssn                                                     : 12,
                      reserved_0b                                             :  4;
             uint32_t sw_peer_id                                              : 16,
                      reserved_1a                                             : 16;
             uint32_t ba_bitmap_31_0                                          : 32;
             uint32_t ba_bitmap_63_32                                         : 32;
             uint32_t ba_bitmap_95_64                                         : 32;
             uint32_t ba_bitmap_127_96                                        : 32;
             uint32_t ba_bitmap_159_128                                       : 32;
             uint32_t ba_bitmap_191_160                                       : 32;
             uint32_t ba_bitmap_223_192                                       : 32;
             uint32_t ba_bitmap_255_224                                       : 32;
             uint32_t ba_bitmap_287_256                                       : 32;
             uint32_t ba_bitmap_319_288                                       : 32;
             uint32_t ba_bitmap_351_320                                       : 32;
             uint32_t ba_bitmap_383_352                                       : 32;
             uint32_t ba_bitmap_415_384                                       : 32;
             uint32_t ba_bitmap_447_416                                       : 32;
             uint32_t ba_bitmap_479_448                                       : 32;
             uint32_t ba_bitmap_511_480                                       : 32;
             uint32_t ba_bitmap_543_512                                       : 32;
             uint32_t ba_bitmap_575_544                                       : 32;
             uint32_t ba_bitmap_607_576                                       : 32;
             uint32_t ba_bitmap_639_608                                       : 32;
             uint32_t ba_bitmap_671_640                                       : 32;
             uint32_t ba_bitmap_703_672                                       : 32;
             uint32_t ba_bitmap_735_704                                       : 32;
             uint32_t ba_bitmap_767_736                                       : 32;
             uint32_t ba_bitmap_799_768                                       : 32;
             uint32_t ba_bitmap_831_800                                       : 32;
             uint32_t ba_bitmap_863_832                                       : 32;
             uint32_t ba_bitmap_895_864                                       : 32;
             uint32_t ba_bitmap_927_896                                       : 32;
             uint32_t ba_bitmap_959_928                                       : 32;
             uint32_t ba_bitmap_991_960                                       : 32;
             uint32_t ba_bitmap_1023_992                                      : 32;
#else
             uint32_t reserved_0b                                             :  4,
                      ssn                                                     : 12,
                      ack_frame_rssi                                          :  8,
                      response_timeout                                        :  1,
                      unexpected_ack_or_ba                                    :  1,
                      ba_tid                                                  :  4,
                      ba_type                                                 :  1,
                      ack_ba_status_type                                      :  1;
             uint32_t reserved_1a                                             : 16,
                      sw_peer_id                                              : 16;
             uint32_t ba_bitmap_31_0                                          : 32;
             uint32_t ba_bitmap_63_32                                         : 32;
             uint32_t ba_bitmap_95_64                                         : 32;
             uint32_t ba_bitmap_127_96                                        : 32;
             uint32_t ba_bitmap_159_128                                       : 32;
             uint32_t ba_bitmap_191_160                                       : 32;
             uint32_t ba_bitmap_223_192                                       : 32;
             uint32_t ba_bitmap_255_224                                       : 32;
             uint32_t ba_bitmap_287_256                                       : 32;
             uint32_t ba_bitmap_319_288                                       : 32;
             uint32_t ba_bitmap_351_320                                       : 32;
             uint32_t ba_bitmap_383_352                                       : 32;
             uint32_t ba_bitmap_415_384                                       : 32;
             uint32_t ba_bitmap_447_416                                       : 32;
             uint32_t ba_bitmap_479_448                                       : 32;
             uint32_t ba_bitmap_511_480                                       : 32;
             uint32_t ba_bitmap_543_512                                       : 32;
             uint32_t ba_bitmap_575_544                                       : 32;
             uint32_t ba_bitmap_607_576                                       : 32;
             uint32_t ba_bitmap_639_608                                       : 32;
             uint32_t ba_bitmap_671_640                                       : 32;
             uint32_t ba_bitmap_703_672                                       : 32;
             uint32_t ba_bitmap_735_704                                       : 32;
             uint32_t ba_bitmap_767_736                                       : 32;
             uint32_t ba_bitmap_799_768                                       : 32;
             uint32_t ba_bitmap_831_800                                       : 32;
             uint32_t ba_bitmap_863_832                                       : 32;
             uint32_t ba_bitmap_895_864                                       : 32;
             uint32_t ba_bitmap_927_896                                       : 32;
             uint32_t ba_bitmap_959_928                                       : 32;
             uint32_t ba_bitmap_991_960                                       : 32;
             uint32_t ba_bitmap_1023_992                                      : 32;
#endif
};

#define TX_FES_STATUS_1K_BA_ACK_BA_STATUS_TYPE_OFFSET                               0x0000000000000000
#define TX_FES_STATUS_1K_BA_ACK_BA_STATUS_TYPE_LSB                                  0
#define TX_FES_STATUS_1K_BA_ACK_BA_STATUS_TYPE_MSB                                  0
#define TX_FES_STATUS_1K_BA_ACK_BA_STATUS_TYPE_MASK                                 0x0000000000000001

#define TX_FES_STATUS_1K_BA_BA_TYPE_OFFSET                                          0x0000000000000000
#define TX_FES_STATUS_1K_BA_BA_TYPE_LSB                                             1
#define TX_FES_STATUS_1K_BA_BA_TYPE_MSB                                             1
#define TX_FES_STATUS_1K_BA_BA_TYPE_MASK                                            0x0000000000000002

#define TX_FES_STATUS_1K_BA_BA_TID_OFFSET                                           0x0000000000000000
#define TX_FES_STATUS_1K_BA_BA_TID_LSB                                              2
#define TX_FES_STATUS_1K_BA_BA_TID_MSB                                              5
#define TX_FES_STATUS_1K_BA_BA_TID_MASK                                             0x000000000000003c

#define TX_FES_STATUS_1K_BA_UNEXPECTED_ACK_OR_BA_OFFSET                             0x0000000000000000
#define TX_FES_STATUS_1K_BA_UNEXPECTED_ACK_OR_BA_LSB                                6
#define TX_FES_STATUS_1K_BA_UNEXPECTED_ACK_OR_BA_MSB                                6
#define TX_FES_STATUS_1K_BA_UNEXPECTED_ACK_OR_BA_MASK                               0x0000000000000040

#define TX_FES_STATUS_1K_BA_RESPONSE_TIMEOUT_OFFSET                                 0x0000000000000000
#define TX_FES_STATUS_1K_BA_RESPONSE_TIMEOUT_LSB                                    7
#define TX_FES_STATUS_1K_BA_RESPONSE_TIMEOUT_MSB                                    7
#define TX_FES_STATUS_1K_BA_RESPONSE_TIMEOUT_MASK                                   0x0000000000000080

#define TX_FES_STATUS_1K_BA_ACK_FRAME_RSSI_OFFSET                                   0x0000000000000000
#define TX_FES_STATUS_1K_BA_ACK_FRAME_RSSI_LSB                                      8
#define TX_FES_STATUS_1K_BA_ACK_FRAME_RSSI_MSB                                      15
#define TX_FES_STATUS_1K_BA_ACK_FRAME_RSSI_MASK                                     0x000000000000ff00

#define TX_FES_STATUS_1K_BA_SSN_OFFSET                                              0x0000000000000000
#define TX_FES_STATUS_1K_BA_SSN_LSB                                                 16
#define TX_FES_STATUS_1K_BA_SSN_MSB                                                 27
#define TX_FES_STATUS_1K_BA_SSN_MASK                                                0x000000000fff0000

#define TX_FES_STATUS_1K_BA_RESERVED_0B_OFFSET                                      0x0000000000000000
#define TX_FES_STATUS_1K_BA_RESERVED_0B_LSB                                         28
#define TX_FES_STATUS_1K_BA_RESERVED_0B_MSB                                         31
#define TX_FES_STATUS_1K_BA_RESERVED_0B_MASK                                        0x00000000f0000000

#define TX_FES_STATUS_1K_BA_SW_PEER_ID_OFFSET                                       0x0000000000000000
#define TX_FES_STATUS_1K_BA_SW_PEER_ID_LSB                                          32
#define TX_FES_STATUS_1K_BA_SW_PEER_ID_MSB                                          47
#define TX_FES_STATUS_1K_BA_SW_PEER_ID_MASK                                         0x0000ffff00000000

#define TX_FES_STATUS_1K_BA_RESERVED_1A_OFFSET                                      0x0000000000000000
#define TX_FES_STATUS_1K_BA_RESERVED_1A_LSB                                         48
#define TX_FES_STATUS_1K_BA_RESERVED_1A_MSB                                         63
#define TX_FES_STATUS_1K_BA_RESERVED_1A_MASK                                        0xffff000000000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_31_0_OFFSET                                   0x0000000000000008
#define TX_FES_STATUS_1K_BA_BA_BITMAP_31_0_LSB                                      0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_31_0_MSB                                      31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_31_0_MASK                                     0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_63_32_OFFSET                                  0x0000000000000008
#define TX_FES_STATUS_1K_BA_BA_BITMAP_63_32_LSB                                     32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_63_32_MSB                                     63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_63_32_MASK                                    0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_95_64_OFFSET                                  0x0000000000000010
#define TX_FES_STATUS_1K_BA_BA_BITMAP_95_64_LSB                                     0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_95_64_MSB                                     31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_95_64_MASK                                    0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_127_96_OFFSET                                 0x0000000000000010
#define TX_FES_STATUS_1K_BA_BA_BITMAP_127_96_LSB                                    32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_127_96_MSB                                    63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_127_96_MASK                                   0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_159_128_OFFSET                                0x0000000000000018
#define TX_FES_STATUS_1K_BA_BA_BITMAP_159_128_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_159_128_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_159_128_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_191_160_OFFSET                                0x0000000000000018
#define TX_FES_STATUS_1K_BA_BA_BITMAP_191_160_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_191_160_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_191_160_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_223_192_OFFSET                                0x0000000000000020
#define TX_FES_STATUS_1K_BA_BA_BITMAP_223_192_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_223_192_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_223_192_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_255_224_OFFSET                                0x0000000000000020
#define TX_FES_STATUS_1K_BA_BA_BITMAP_255_224_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_255_224_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_255_224_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_287_256_OFFSET                                0x0000000000000028
#define TX_FES_STATUS_1K_BA_BA_BITMAP_287_256_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_287_256_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_287_256_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_319_288_OFFSET                                0x0000000000000028
#define TX_FES_STATUS_1K_BA_BA_BITMAP_319_288_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_319_288_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_319_288_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_351_320_OFFSET                                0x0000000000000030
#define TX_FES_STATUS_1K_BA_BA_BITMAP_351_320_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_351_320_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_351_320_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_383_352_OFFSET                                0x0000000000000030
#define TX_FES_STATUS_1K_BA_BA_BITMAP_383_352_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_383_352_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_383_352_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_415_384_OFFSET                                0x0000000000000038
#define TX_FES_STATUS_1K_BA_BA_BITMAP_415_384_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_415_384_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_415_384_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_447_416_OFFSET                                0x0000000000000038
#define TX_FES_STATUS_1K_BA_BA_BITMAP_447_416_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_447_416_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_447_416_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_479_448_OFFSET                                0x0000000000000040
#define TX_FES_STATUS_1K_BA_BA_BITMAP_479_448_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_479_448_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_479_448_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_511_480_OFFSET                                0x0000000000000040
#define TX_FES_STATUS_1K_BA_BA_BITMAP_511_480_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_511_480_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_511_480_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_543_512_OFFSET                                0x0000000000000048
#define TX_FES_STATUS_1K_BA_BA_BITMAP_543_512_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_543_512_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_543_512_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_575_544_OFFSET                                0x0000000000000048
#define TX_FES_STATUS_1K_BA_BA_BITMAP_575_544_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_575_544_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_575_544_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_607_576_OFFSET                                0x0000000000000050
#define TX_FES_STATUS_1K_BA_BA_BITMAP_607_576_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_607_576_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_607_576_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_639_608_OFFSET                                0x0000000000000050
#define TX_FES_STATUS_1K_BA_BA_BITMAP_639_608_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_639_608_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_639_608_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_671_640_OFFSET                                0x0000000000000058
#define TX_FES_STATUS_1K_BA_BA_BITMAP_671_640_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_671_640_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_671_640_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_703_672_OFFSET                                0x0000000000000058
#define TX_FES_STATUS_1K_BA_BA_BITMAP_703_672_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_703_672_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_703_672_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_735_704_OFFSET                                0x0000000000000060
#define TX_FES_STATUS_1K_BA_BA_BITMAP_735_704_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_735_704_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_735_704_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_767_736_OFFSET                                0x0000000000000060
#define TX_FES_STATUS_1K_BA_BA_BITMAP_767_736_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_767_736_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_767_736_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_799_768_OFFSET                                0x0000000000000068
#define TX_FES_STATUS_1K_BA_BA_BITMAP_799_768_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_799_768_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_799_768_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_831_800_OFFSET                                0x0000000000000068
#define TX_FES_STATUS_1K_BA_BA_BITMAP_831_800_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_831_800_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_831_800_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_863_832_OFFSET                                0x0000000000000070
#define TX_FES_STATUS_1K_BA_BA_BITMAP_863_832_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_863_832_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_863_832_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_895_864_OFFSET                                0x0000000000000070
#define TX_FES_STATUS_1K_BA_BA_BITMAP_895_864_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_895_864_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_895_864_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_927_896_OFFSET                                0x0000000000000078
#define TX_FES_STATUS_1K_BA_BA_BITMAP_927_896_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_927_896_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_927_896_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_959_928_OFFSET                                0x0000000000000078
#define TX_FES_STATUS_1K_BA_BA_BITMAP_959_928_LSB                                   32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_959_928_MSB                                   63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_959_928_MASK                                  0xffffffff00000000

#define TX_FES_STATUS_1K_BA_BA_BITMAP_991_960_OFFSET                                0x0000000000000080
#define TX_FES_STATUS_1K_BA_BA_BITMAP_991_960_LSB                                   0
#define TX_FES_STATUS_1K_BA_BA_BITMAP_991_960_MSB                                   31
#define TX_FES_STATUS_1K_BA_BA_BITMAP_991_960_MASK                                  0x00000000ffffffff

#define TX_FES_STATUS_1K_BA_BA_BITMAP_1023_992_OFFSET                               0x0000000000000080
#define TX_FES_STATUS_1K_BA_BA_BITMAP_1023_992_LSB                                  32
#define TX_FES_STATUS_1K_BA_BA_BITMAP_1023_992_MSB                                  63
#define TX_FES_STATUS_1K_BA_BA_BITMAP_1023_992_MASK                                 0xffffffff00000000

#endif
