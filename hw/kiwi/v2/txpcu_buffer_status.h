
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



#ifndef _TXPCU_BUFFER_STATUS_H_
#define _TXPCU_BUFFER_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "txpcu_buffer_basics.h"
#define NUM_OF_DWORDS_TXPCU_BUFFER_STATUS 2

#define NUM_OF_QWORDS_TXPCU_BUFFER_STATUS 1

struct txpcu_buffer_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   txpcu_buffer_basics                                       txpcu_basix_buffer_info;
             uint32_t reserved                                                : 15,
                      msdu_end                                                :  1,
                      tx_data_sync_value                                      : 16;
#else
             struct   txpcu_buffer_basics                                       txpcu_basix_buffer_info;
             uint32_t tx_data_sync_value                                      : 16,
                      msdu_end                                                :  1,
                      reserved                                                : 15;
#endif
};

#define TXPCU_BUFFER_STATUS_TXPCU_BASIX_BUFFER_INFO_AVAILABLE_MEMORY_OFFSET         0x0000000000000000
#define TXPCU_BUFFER_STATUS_TXPCU_BASIX_BUFFER_INFO_AVAILABLE_MEMORY_LSB            0
#define TXPCU_BUFFER_STATUS_TXPCU_BASIX_BUFFER_INFO_AVAILABLE_MEMORY_MSB            7
#define TXPCU_BUFFER_STATUS_TXPCU_BASIX_BUFFER_INFO_AVAILABLE_MEMORY_MASK           0x00000000000000ff

#define TXPCU_BUFFER_STATUS_TXPCU_BASIX_BUFFER_INFO_PARTIAL_TX_DATA_TLV_COUNT_OFFSET 0x0000000000000000
#define TXPCU_BUFFER_STATUS_TXPCU_BASIX_BUFFER_INFO_PARTIAL_TX_DATA_TLV_COUNT_LSB   8
#define TXPCU_BUFFER_STATUS_TXPCU_BASIX_BUFFER_INFO_PARTIAL_TX_DATA_TLV_COUNT_MSB   15
#define TXPCU_BUFFER_STATUS_TXPCU_BASIX_BUFFER_INFO_PARTIAL_TX_DATA_TLV_COUNT_MASK  0x000000000000ff00

#define TXPCU_BUFFER_STATUS_TXPCU_BASIX_BUFFER_INFO_TX_DATA_TLV_COUNT_OFFSET        0x0000000000000000
#define TXPCU_BUFFER_STATUS_TXPCU_BASIX_BUFFER_INFO_TX_DATA_TLV_COUNT_LSB           16
#define TXPCU_BUFFER_STATUS_TXPCU_BASIX_BUFFER_INFO_TX_DATA_TLV_COUNT_MSB           31
#define TXPCU_BUFFER_STATUS_TXPCU_BASIX_BUFFER_INFO_TX_DATA_TLV_COUNT_MASK          0x00000000ffff0000

#define TXPCU_BUFFER_STATUS_RESERVED_OFFSET                                         0x0000000000000000
#define TXPCU_BUFFER_STATUS_RESERVED_LSB                                            32
#define TXPCU_BUFFER_STATUS_RESERVED_MSB                                            46
#define TXPCU_BUFFER_STATUS_RESERVED_MASK                                           0x00007fff00000000

#define TXPCU_BUFFER_STATUS_MSDU_END_OFFSET                                         0x0000000000000000
#define TXPCU_BUFFER_STATUS_MSDU_END_LSB                                            47
#define TXPCU_BUFFER_STATUS_MSDU_END_MSB                                            47
#define TXPCU_BUFFER_STATUS_MSDU_END_MASK                                           0x0000800000000000

#define TXPCU_BUFFER_STATUS_TX_DATA_SYNC_VALUE_OFFSET                               0x0000000000000000
#define TXPCU_BUFFER_STATUS_TX_DATA_SYNC_VALUE_LSB                                  48
#define TXPCU_BUFFER_STATUS_TX_DATA_SYNC_VALUE_MSB                                  63
#define TXPCU_BUFFER_STATUS_TX_DATA_SYNC_VALUE_MASK                                 0xffff000000000000

#endif
