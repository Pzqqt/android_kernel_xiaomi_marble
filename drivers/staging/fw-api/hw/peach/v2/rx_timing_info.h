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


#ifndef _RX_TIMING_INFO_H_
#define _RX_TIMING_INFO_H_

#define NUM_OF_DWORDS_RX_TIMING_INFO 5

struct rx_timing_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t phy_timestamp_1_lower_32                                : 32;
             uint32_t phy_timestamp_1_upper_32                                : 32;
             uint32_t phy_timestamp_2_lower_32                                : 32;
             uint32_t phy_timestamp_2_upper_32                                : 32;
             uint32_t residual_phase_offset                                   : 12,
                      reserved                                                : 20;
#else
             uint32_t phy_timestamp_1_lower_32                                : 32;
             uint32_t phy_timestamp_1_upper_32                                : 32;
             uint32_t phy_timestamp_2_lower_32                                : 32;
             uint32_t phy_timestamp_2_upper_32                                : 32;
             uint32_t reserved                                                : 20,
                      residual_phase_offset                                   : 12;
#endif
};

#define RX_TIMING_INFO_PHY_TIMESTAMP_1_LOWER_32_OFFSET                              0x00000000
#define RX_TIMING_INFO_PHY_TIMESTAMP_1_LOWER_32_LSB                                 0
#define RX_TIMING_INFO_PHY_TIMESTAMP_1_LOWER_32_MSB                                 31
#define RX_TIMING_INFO_PHY_TIMESTAMP_1_LOWER_32_MASK                                0xffffffff

#define RX_TIMING_INFO_PHY_TIMESTAMP_1_UPPER_32_OFFSET                              0x00000004
#define RX_TIMING_INFO_PHY_TIMESTAMP_1_UPPER_32_LSB                                 0
#define RX_TIMING_INFO_PHY_TIMESTAMP_1_UPPER_32_MSB                                 31
#define RX_TIMING_INFO_PHY_TIMESTAMP_1_UPPER_32_MASK                                0xffffffff

#define RX_TIMING_INFO_PHY_TIMESTAMP_2_LOWER_32_OFFSET                              0x00000008
#define RX_TIMING_INFO_PHY_TIMESTAMP_2_LOWER_32_LSB                                 0
#define RX_TIMING_INFO_PHY_TIMESTAMP_2_LOWER_32_MSB                                 31
#define RX_TIMING_INFO_PHY_TIMESTAMP_2_LOWER_32_MASK                                0xffffffff

#define RX_TIMING_INFO_PHY_TIMESTAMP_2_UPPER_32_OFFSET                              0x0000000c
#define RX_TIMING_INFO_PHY_TIMESTAMP_2_UPPER_32_LSB                                 0
#define RX_TIMING_INFO_PHY_TIMESTAMP_2_UPPER_32_MSB                                 31
#define RX_TIMING_INFO_PHY_TIMESTAMP_2_UPPER_32_MASK                                0xffffffff

#define RX_TIMING_INFO_RESIDUAL_PHASE_OFFSET_OFFSET                                 0x00000010
#define RX_TIMING_INFO_RESIDUAL_PHASE_OFFSET_LSB                                    0
#define RX_TIMING_INFO_RESIDUAL_PHASE_OFFSET_MSB                                    11
#define RX_TIMING_INFO_RESIDUAL_PHASE_OFFSET_MASK                                   0x00000fff

#define RX_TIMING_INFO_RESERVED_OFFSET                                              0x00000010
#define RX_TIMING_INFO_RESERVED_LSB                                                 12
#define RX_TIMING_INFO_RESERVED_MSB                                                 31
#define RX_TIMING_INFO_RESERVED_MASK                                                0xfffff000

#endif
