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


#ifndef _RU_ALLOCATION_160_INFO_H_
#define _RU_ALLOCATION_160_INFO_H_

#define NUM_OF_DWORDS_RU_ALLOCATION_160_INFO 4

struct ru_allocation_160_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ru_allocation_band0_0                                   :  9,
                      ru_allocation_band0_1                                   :  9,
                      reserved_0a                                             :  6,
                      ru_allocations_01_subband80_mask                        :  4,
                      ru_allocations_23_subband80_mask                        :  4;
             uint32_t ru_allocation_band0_2                                   :  9,
                      ru_allocation_band0_3                                   :  9,
                      reserved_1a                                             : 14;
             uint32_t ru_allocation_band1_0                                   :  9,
                      ru_allocation_band1_1                                   :  9,
                      reserved_2a                                             : 14;
             uint32_t ru_allocation_band1_2                                   :  9,
                      ru_allocation_band1_3                                   :  9,
                      reserved_3a                                             : 14;
#else
             uint32_t ru_allocations_23_subband80_mask                        :  4,
                      ru_allocations_01_subband80_mask                        :  4,
                      reserved_0a                                             :  6,
                      ru_allocation_band0_1                                   :  9,
                      ru_allocation_band0_0                                   :  9;
             uint32_t reserved_1a                                             : 14,
                      ru_allocation_band0_3                                   :  9,
                      ru_allocation_band0_2                                   :  9;
             uint32_t reserved_2a                                             : 14,
                      ru_allocation_band1_1                                   :  9,
                      ru_allocation_band1_0                                   :  9;
             uint32_t reserved_3a                                             : 14,
                      ru_allocation_band1_3                                   :  9,
                      ru_allocation_band1_2                                   :  9;
#endif
};

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_0_OFFSET                         0x00000000
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_0_LSB                            0
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_0_MSB                            8
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_0_MASK                           0x000001ff

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_1_OFFSET                         0x00000000
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_1_LSB                            9
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_1_MSB                            17
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_1_MASK                           0x0003fe00

#define RU_ALLOCATION_160_INFO_RESERVED_0A_OFFSET                                   0x00000000
#define RU_ALLOCATION_160_INFO_RESERVED_0A_LSB                                      18
#define RU_ALLOCATION_160_INFO_RESERVED_0A_MSB                                      23
#define RU_ALLOCATION_160_INFO_RESERVED_0A_MASK                                     0x00fc0000

#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_01_SUBBAND80_MASK_OFFSET              0x00000000
#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_01_SUBBAND80_MASK_LSB                 24
#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_01_SUBBAND80_MASK_MSB                 27
#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_01_SUBBAND80_MASK_MASK                0x0f000000

#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_23_SUBBAND80_MASK_OFFSET              0x00000000
#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_23_SUBBAND80_MASK_LSB                 28
#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_23_SUBBAND80_MASK_MSB                 31
#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_23_SUBBAND80_MASK_MASK                0xf0000000

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_2_OFFSET                         0x00000004
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_2_LSB                            0
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_2_MSB                            8
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_2_MASK                           0x000001ff

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_3_OFFSET                         0x00000004
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_3_LSB                            9
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_3_MSB                            17
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_3_MASK                           0x0003fe00

#define RU_ALLOCATION_160_INFO_RESERVED_1A_OFFSET                                   0x00000004
#define RU_ALLOCATION_160_INFO_RESERVED_1A_LSB                                      18
#define RU_ALLOCATION_160_INFO_RESERVED_1A_MSB                                      31
#define RU_ALLOCATION_160_INFO_RESERVED_1A_MASK                                     0xfffc0000

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_0_OFFSET                         0x00000008
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_0_LSB                            0
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_0_MSB                            8
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_0_MASK                           0x000001ff

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_1_OFFSET                         0x00000008
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_1_LSB                            9
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_1_MSB                            17
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_1_MASK                           0x0003fe00

#define RU_ALLOCATION_160_INFO_RESERVED_2A_OFFSET                                   0x00000008
#define RU_ALLOCATION_160_INFO_RESERVED_2A_LSB                                      18
#define RU_ALLOCATION_160_INFO_RESERVED_2A_MSB                                      31
#define RU_ALLOCATION_160_INFO_RESERVED_2A_MASK                                     0xfffc0000

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_2_OFFSET                         0x0000000c
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_2_LSB                            0
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_2_MSB                            8
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_2_MASK                           0x000001ff

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_3_OFFSET                         0x0000000c
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_3_LSB                            9
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_3_MSB                            17
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_3_MASK                           0x0003fe00

#define RU_ALLOCATION_160_INFO_RESERVED_3A_OFFSET                                   0x0000000c
#define RU_ALLOCATION_160_INFO_RESERVED_3A_LSB                                      18
#define RU_ALLOCATION_160_INFO_RESERVED_3A_MSB                                      31
#define RU_ALLOCATION_160_INFO_RESERVED_3A_MASK                                     0xfffc0000

#endif
