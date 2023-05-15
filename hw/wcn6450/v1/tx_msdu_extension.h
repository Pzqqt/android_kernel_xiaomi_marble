
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



#ifndef _TX_MSDU_EXTENSION_H_
#define _TX_MSDU_EXTENSION_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_MSDU_EXTENSION 18

struct tx_msdu_extension {
             uint32_t tso_enable                      :  1,
                      reserved_0a                     :  6,
                      tcp_flag                        :  9,
                      tcp_flag_mask                   :  9,
                      reserved_0b                     :  7;
             uint32_t l2_length                       : 16,
                      ip_length                       : 16;
             uint32_t tcp_seq_number                  : 32;
             uint32_t ip_identification               : 16,
                      udp_length                      : 16;
             uint32_t checksum_offset                 : 14,
                      partial_checksum_en             :  1,
                      reserved_4a                     :  1,
                      payload_start_offset            : 14,
                      reserved_4b                     :  2;
             uint32_t payload_end_offset              : 14,
                      reserved_5a                     :  2,
                      wds                             :  1,
                      reserved_5b                     : 15;
             uint32_t buf0_ptr_31_0                   : 32;
             uint32_t buf0_ptr_39_32                  :  8,
                      reserved_7a                     :  8,
                      buf0_len                        : 16;
             uint32_t buf1_ptr_31_0                   : 32;
             uint32_t buf1_ptr_39_32                  :  8,
                      reserved_9a                     :  8,
                      buf1_len                        : 16;
             uint32_t buf2_ptr_31_0                   : 32;
             uint32_t buf2_ptr_39_32                  :  8,
                      reserved_11a                    :  8,
                      buf2_len                        : 16;
             uint32_t buf3_ptr_31_0                   : 32;
             uint32_t buf3_ptr_39_32                  :  8,
                      reserved_13a                    :  8,
                      buf3_len                        : 16;
             uint32_t buf4_ptr_31_0                   : 32;
             uint32_t buf4_ptr_39_32                  :  8,
                      reserved_15a                    :  8,
                      buf4_len                        : 16;
             uint32_t buf5_ptr_31_0                   : 32;
             uint32_t buf5_ptr_39_32                  :  8,
                      reserved_17a                    :  8,
                      buf5_len                        : 16;
};

#define TX_MSDU_EXTENSION_0_TSO_ENABLE_OFFSET                        0x00000000
#define TX_MSDU_EXTENSION_0_TSO_ENABLE_LSB                           0
#define TX_MSDU_EXTENSION_0_TSO_ENABLE_MASK                          0x00000001

#define TX_MSDU_EXTENSION_0_RESERVED_0A_OFFSET                       0x00000000
#define TX_MSDU_EXTENSION_0_RESERVED_0A_LSB                          1
#define TX_MSDU_EXTENSION_0_RESERVED_0A_MASK                         0x0000007e

#define TX_MSDU_EXTENSION_0_TCP_FLAG_OFFSET                          0x00000000
#define TX_MSDU_EXTENSION_0_TCP_FLAG_LSB                             7
#define TX_MSDU_EXTENSION_0_TCP_FLAG_MASK                            0x0000ff80

#define TX_MSDU_EXTENSION_0_TCP_FLAG_MASK_OFFSET                     0x00000000
#define TX_MSDU_EXTENSION_0_TCP_FLAG_MASK_LSB                        16
#define TX_MSDU_EXTENSION_0_TCP_FLAG_MASK_MASK                       0x01ff0000

#define TX_MSDU_EXTENSION_0_RESERVED_0B_OFFSET                       0x00000000
#define TX_MSDU_EXTENSION_0_RESERVED_0B_LSB                          25
#define TX_MSDU_EXTENSION_0_RESERVED_0B_MASK                         0xfe000000

#define TX_MSDU_EXTENSION_1_L2_LENGTH_OFFSET                         0x00000004
#define TX_MSDU_EXTENSION_1_L2_LENGTH_LSB                            0
#define TX_MSDU_EXTENSION_1_L2_LENGTH_MASK                           0x0000ffff

#define TX_MSDU_EXTENSION_1_IP_LENGTH_OFFSET                         0x00000004
#define TX_MSDU_EXTENSION_1_IP_LENGTH_LSB                            16
#define TX_MSDU_EXTENSION_1_IP_LENGTH_MASK                           0xffff0000

#define TX_MSDU_EXTENSION_2_TCP_SEQ_NUMBER_OFFSET                    0x00000008
#define TX_MSDU_EXTENSION_2_TCP_SEQ_NUMBER_LSB                       0
#define TX_MSDU_EXTENSION_2_TCP_SEQ_NUMBER_MASK                      0xffffffff

#define TX_MSDU_EXTENSION_3_IP_IDENTIFICATION_OFFSET                 0x0000000c
#define TX_MSDU_EXTENSION_3_IP_IDENTIFICATION_LSB                    0
#define TX_MSDU_EXTENSION_3_IP_IDENTIFICATION_MASK                   0x0000ffff

#define TX_MSDU_EXTENSION_3_UDP_LENGTH_OFFSET                        0x0000000c
#define TX_MSDU_EXTENSION_3_UDP_LENGTH_LSB                           16
#define TX_MSDU_EXTENSION_3_UDP_LENGTH_MASK                          0xffff0000

#define TX_MSDU_EXTENSION_4_CHECKSUM_OFFSET_OFFSET                   0x00000010
#define TX_MSDU_EXTENSION_4_CHECKSUM_OFFSET_LSB                      0
#define TX_MSDU_EXTENSION_4_CHECKSUM_OFFSET_MASK                     0x00003fff

#define TX_MSDU_EXTENSION_4_PARTIAL_CHECKSUM_EN_OFFSET               0x00000010
#define TX_MSDU_EXTENSION_4_PARTIAL_CHECKSUM_EN_LSB                  14
#define TX_MSDU_EXTENSION_4_PARTIAL_CHECKSUM_EN_MASK                 0x00004000

#define TX_MSDU_EXTENSION_4_RESERVED_4A_OFFSET                       0x00000010
#define TX_MSDU_EXTENSION_4_RESERVED_4A_LSB                          15
#define TX_MSDU_EXTENSION_4_RESERVED_4A_MASK                         0x00008000

#define TX_MSDU_EXTENSION_4_PAYLOAD_START_OFFSET_OFFSET              0x00000010
#define TX_MSDU_EXTENSION_4_PAYLOAD_START_OFFSET_LSB                 16
#define TX_MSDU_EXTENSION_4_PAYLOAD_START_OFFSET_MASK                0x3fff0000

#define TX_MSDU_EXTENSION_4_RESERVED_4B_OFFSET                       0x00000010
#define TX_MSDU_EXTENSION_4_RESERVED_4B_LSB                          30
#define TX_MSDU_EXTENSION_4_RESERVED_4B_MASK                         0xc0000000

#define TX_MSDU_EXTENSION_5_PAYLOAD_END_OFFSET_OFFSET                0x00000014
#define TX_MSDU_EXTENSION_5_PAYLOAD_END_OFFSET_LSB                   0
#define TX_MSDU_EXTENSION_5_PAYLOAD_END_OFFSET_MASK                  0x00003fff

#define TX_MSDU_EXTENSION_5_RESERVED_5A_OFFSET                       0x00000014
#define TX_MSDU_EXTENSION_5_RESERVED_5A_LSB                          14
#define TX_MSDU_EXTENSION_5_RESERVED_5A_MASK                         0x0000c000

#define TX_MSDU_EXTENSION_5_WDS_OFFSET                               0x00000014
#define TX_MSDU_EXTENSION_5_WDS_LSB                                  16
#define TX_MSDU_EXTENSION_5_WDS_MASK                                 0x00010000

#define TX_MSDU_EXTENSION_5_RESERVED_5B_OFFSET                       0x00000014
#define TX_MSDU_EXTENSION_5_RESERVED_5B_LSB                          17
#define TX_MSDU_EXTENSION_5_RESERVED_5B_MASK                         0xfffe0000

#define TX_MSDU_EXTENSION_6_BUF0_PTR_31_0_OFFSET                     0x00000018
#define TX_MSDU_EXTENSION_6_BUF0_PTR_31_0_LSB                        0
#define TX_MSDU_EXTENSION_6_BUF0_PTR_31_0_MASK                       0xffffffff

#define TX_MSDU_EXTENSION_7_BUF0_PTR_39_32_OFFSET                    0x0000001c
#define TX_MSDU_EXTENSION_7_BUF0_PTR_39_32_LSB                       0
#define TX_MSDU_EXTENSION_7_BUF0_PTR_39_32_MASK                      0x000000ff

#define TX_MSDU_EXTENSION_7_RESERVED_7A_OFFSET                       0x0000001c
#define TX_MSDU_EXTENSION_7_RESERVED_7A_LSB                          8
#define TX_MSDU_EXTENSION_7_RESERVED_7A_MASK                         0x0000ff00

#define TX_MSDU_EXTENSION_7_BUF0_LEN_OFFSET                          0x0000001c
#define TX_MSDU_EXTENSION_7_BUF0_LEN_LSB                             16
#define TX_MSDU_EXTENSION_7_BUF0_LEN_MASK                            0xffff0000

#define TX_MSDU_EXTENSION_8_BUF1_PTR_31_0_OFFSET                     0x00000020
#define TX_MSDU_EXTENSION_8_BUF1_PTR_31_0_LSB                        0
#define TX_MSDU_EXTENSION_8_BUF1_PTR_31_0_MASK                       0xffffffff

#define TX_MSDU_EXTENSION_9_BUF1_PTR_39_32_OFFSET                    0x00000024
#define TX_MSDU_EXTENSION_9_BUF1_PTR_39_32_LSB                       0
#define TX_MSDU_EXTENSION_9_BUF1_PTR_39_32_MASK                      0x000000ff

#define TX_MSDU_EXTENSION_9_RESERVED_9A_OFFSET                       0x00000024
#define TX_MSDU_EXTENSION_9_RESERVED_9A_LSB                          8
#define TX_MSDU_EXTENSION_9_RESERVED_9A_MASK                         0x0000ff00

#define TX_MSDU_EXTENSION_9_BUF1_LEN_OFFSET                          0x00000024
#define TX_MSDU_EXTENSION_9_BUF1_LEN_LSB                             16
#define TX_MSDU_EXTENSION_9_BUF1_LEN_MASK                            0xffff0000

#define TX_MSDU_EXTENSION_10_BUF2_PTR_31_0_OFFSET                    0x00000028
#define TX_MSDU_EXTENSION_10_BUF2_PTR_31_0_LSB                       0
#define TX_MSDU_EXTENSION_10_BUF2_PTR_31_0_MASK                      0xffffffff

#define TX_MSDU_EXTENSION_11_BUF2_PTR_39_32_OFFSET                   0x0000002c
#define TX_MSDU_EXTENSION_11_BUF2_PTR_39_32_LSB                      0
#define TX_MSDU_EXTENSION_11_BUF2_PTR_39_32_MASK                     0x000000ff

#define TX_MSDU_EXTENSION_11_RESERVED_11A_OFFSET                     0x0000002c
#define TX_MSDU_EXTENSION_11_RESERVED_11A_LSB                        8
#define TX_MSDU_EXTENSION_11_RESERVED_11A_MASK                       0x0000ff00

#define TX_MSDU_EXTENSION_11_BUF2_LEN_OFFSET                         0x0000002c
#define TX_MSDU_EXTENSION_11_BUF2_LEN_LSB                            16
#define TX_MSDU_EXTENSION_11_BUF2_LEN_MASK                           0xffff0000

#define TX_MSDU_EXTENSION_12_BUF3_PTR_31_0_OFFSET                    0x00000030
#define TX_MSDU_EXTENSION_12_BUF3_PTR_31_0_LSB                       0
#define TX_MSDU_EXTENSION_12_BUF3_PTR_31_0_MASK                      0xffffffff

#define TX_MSDU_EXTENSION_13_BUF3_PTR_39_32_OFFSET                   0x00000034
#define TX_MSDU_EXTENSION_13_BUF3_PTR_39_32_LSB                      0
#define TX_MSDU_EXTENSION_13_BUF3_PTR_39_32_MASK                     0x000000ff

#define TX_MSDU_EXTENSION_13_RESERVED_13A_OFFSET                     0x00000034
#define TX_MSDU_EXTENSION_13_RESERVED_13A_LSB                        8
#define TX_MSDU_EXTENSION_13_RESERVED_13A_MASK                       0x0000ff00

#define TX_MSDU_EXTENSION_13_BUF3_LEN_OFFSET                         0x00000034
#define TX_MSDU_EXTENSION_13_BUF3_LEN_LSB                            16
#define TX_MSDU_EXTENSION_13_BUF3_LEN_MASK                           0xffff0000

#define TX_MSDU_EXTENSION_14_BUF4_PTR_31_0_OFFSET                    0x00000038
#define TX_MSDU_EXTENSION_14_BUF4_PTR_31_0_LSB                       0
#define TX_MSDU_EXTENSION_14_BUF4_PTR_31_0_MASK                      0xffffffff

#define TX_MSDU_EXTENSION_15_BUF4_PTR_39_32_OFFSET                   0x0000003c
#define TX_MSDU_EXTENSION_15_BUF4_PTR_39_32_LSB                      0
#define TX_MSDU_EXTENSION_15_BUF4_PTR_39_32_MASK                     0x000000ff

#define TX_MSDU_EXTENSION_15_RESERVED_15A_OFFSET                     0x0000003c
#define TX_MSDU_EXTENSION_15_RESERVED_15A_LSB                        8
#define TX_MSDU_EXTENSION_15_RESERVED_15A_MASK                       0x0000ff00

#define TX_MSDU_EXTENSION_15_BUF4_LEN_OFFSET                         0x0000003c
#define TX_MSDU_EXTENSION_15_BUF4_LEN_LSB                            16
#define TX_MSDU_EXTENSION_15_BUF4_LEN_MASK                           0xffff0000

#define TX_MSDU_EXTENSION_16_BUF5_PTR_31_0_OFFSET                    0x00000040
#define TX_MSDU_EXTENSION_16_BUF5_PTR_31_0_LSB                       0
#define TX_MSDU_EXTENSION_16_BUF5_PTR_31_0_MASK                      0xffffffff

#define TX_MSDU_EXTENSION_17_BUF5_PTR_39_32_OFFSET                   0x00000044
#define TX_MSDU_EXTENSION_17_BUF5_PTR_39_32_LSB                      0
#define TX_MSDU_EXTENSION_17_BUF5_PTR_39_32_MASK                     0x000000ff

#define TX_MSDU_EXTENSION_17_RESERVED_17A_OFFSET                     0x00000044
#define TX_MSDU_EXTENSION_17_RESERVED_17A_LSB                        8
#define TX_MSDU_EXTENSION_17_RESERVED_17A_MASK                       0x0000ff00

#define TX_MSDU_EXTENSION_17_BUF5_LEN_OFFSET                         0x00000044
#define TX_MSDU_EXTENSION_17_BUF5_LEN_LSB                            16
#define TX_MSDU_EXTENSION_17_BUF5_LEN_MASK                           0xffff0000

#endif
