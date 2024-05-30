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


#ifndef _CE_STAT_DESC_H_
#define _CE_STAT_DESC_H_

#define NUM_OF_DWORDS_CE_STAT_DESC 4

struct ce_stat_desc {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ce_res_5                                                :  8,
                      toeplitz_en                                             :  1,
                      src_swap                                                :  1,
                      dest_swap                                               :  1,
                      gather                                                  :  1,
                      barrier_read                                            :  1,
                      ce_res_6                                                :  3,
                      length                                                  : 16;
             uint32_t toeplitz_hash_0                                         : 32;
             uint32_t toeplitz_hash_1                                         : 32;
             uint32_t fw_metadata                                             : 16,
                      ce_res_7                                                :  4,
                      ring_id                                                 :  8,
                      looping_count                                           :  4;
#else
             uint32_t length                                                  : 16,
                      ce_res_6                                                :  3,
                      barrier_read                                            :  1,
                      gather                                                  :  1,
                      dest_swap                                               :  1,
                      src_swap                                                :  1,
                      toeplitz_en                                             :  1,
                      ce_res_5                                                :  8;
             uint32_t toeplitz_hash_0                                         : 32;
             uint32_t toeplitz_hash_1                                         : 32;
             uint32_t looping_count                                           :  4,
                      ring_id                                                 :  8,
                      ce_res_7                                                :  4,
                      fw_metadata                                             : 16;
#endif
};

#define CE_STAT_DESC_CE_RES_5_OFFSET                                                0x00000000
#define CE_STAT_DESC_CE_RES_5_LSB                                                   0
#define CE_STAT_DESC_CE_RES_5_MSB                                                   7
#define CE_STAT_DESC_CE_RES_5_MASK                                                  0x000000ff

#define CE_STAT_DESC_TOEPLITZ_EN_OFFSET                                             0x00000000
#define CE_STAT_DESC_TOEPLITZ_EN_LSB                                                8
#define CE_STAT_DESC_TOEPLITZ_EN_MSB                                                8
#define CE_STAT_DESC_TOEPLITZ_EN_MASK                                               0x00000100

#define CE_STAT_DESC_SRC_SWAP_OFFSET                                                0x00000000
#define CE_STAT_DESC_SRC_SWAP_LSB                                                   9
#define CE_STAT_DESC_SRC_SWAP_MSB                                                   9
#define CE_STAT_DESC_SRC_SWAP_MASK                                                  0x00000200

#define CE_STAT_DESC_DEST_SWAP_OFFSET                                               0x00000000
#define CE_STAT_DESC_DEST_SWAP_LSB                                                  10
#define CE_STAT_DESC_DEST_SWAP_MSB                                                  10
#define CE_STAT_DESC_DEST_SWAP_MASK                                                 0x00000400

#define CE_STAT_DESC_GATHER_OFFSET                                                  0x00000000
#define CE_STAT_DESC_GATHER_LSB                                                     11
#define CE_STAT_DESC_GATHER_MSB                                                     11
#define CE_STAT_DESC_GATHER_MASK                                                    0x00000800

#define CE_STAT_DESC_BARRIER_READ_OFFSET                                            0x00000000
#define CE_STAT_DESC_BARRIER_READ_LSB                                               12
#define CE_STAT_DESC_BARRIER_READ_MSB                                               12
#define CE_STAT_DESC_BARRIER_READ_MASK                                              0x00001000

#define CE_STAT_DESC_CE_RES_6_OFFSET                                                0x00000000
#define CE_STAT_DESC_CE_RES_6_LSB                                                   13
#define CE_STAT_DESC_CE_RES_6_MSB                                                   15
#define CE_STAT_DESC_CE_RES_6_MASK                                                  0x0000e000

#define CE_STAT_DESC_LENGTH_OFFSET                                                  0x00000000
#define CE_STAT_DESC_LENGTH_LSB                                                     16
#define CE_STAT_DESC_LENGTH_MSB                                                     31
#define CE_STAT_DESC_LENGTH_MASK                                                    0xffff0000

#define CE_STAT_DESC_TOEPLITZ_HASH_0_OFFSET                                         0x00000004
#define CE_STAT_DESC_TOEPLITZ_HASH_0_LSB                                            0
#define CE_STAT_DESC_TOEPLITZ_HASH_0_MSB                                            31
#define CE_STAT_DESC_TOEPLITZ_HASH_0_MASK                                           0xffffffff

#define CE_STAT_DESC_TOEPLITZ_HASH_1_OFFSET                                         0x00000008
#define CE_STAT_DESC_TOEPLITZ_HASH_1_LSB                                            0
#define CE_STAT_DESC_TOEPLITZ_HASH_1_MSB                                            31
#define CE_STAT_DESC_TOEPLITZ_HASH_1_MASK                                           0xffffffff

#define CE_STAT_DESC_FW_METADATA_OFFSET                                             0x0000000c
#define CE_STAT_DESC_FW_METADATA_LSB                                                0
#define CE_STAT_DESC_FW_METADATA_MSB                                                15
#define CE_STAT_DESC_FW_METADATA_MASK                                               0x0000ffff

#define CE_STAT_DESC_CE_RES_7_OFFSET                                                0x0000000c
#define CE_STAT_DESC_CE_RES_7_LSB                                                   16
#define CE_STAT_DESC_CE_RES_7_MSB                                                   19
#define CE_STAT_DESC_CE_RES_7_MASK                                                  0x000f0000

#define CE_STAT_DESC_RING_ID_OFFSET                                                 0x0000000c
#define CE_STAT_DESC_RING_ID_LSB                                                    20
#define CE_STAT_DESC_RING_ID_MSB                                                    27
#define CE_STAT_DESC_RING_ID_MASK                                                   0x0ff00000

#define CE_STAT_DESC_LOOPING_COUNT_OFFSET                                           0x0000000c
#define CE_STAT_DESC_LOOPING_COUNT_LSB                                              28
#define CE_STAT_DESC_LOOPING_COUNT_MSB                                              31
#define CE_STAT_DESC_LOOPING_COUNT_MASK                                             0xf0000000

#endif
