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


#ifndef _TXPCU_BUFFER_BASICS_H_
#define _TXPCU_BUFFER_BASICS_H_

#define NUM_OF_DWORDS_TXPCU_BUFFER_BASICS 1

struct txpcu_buffer_basics {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t available_memory                                        :  8,
                      partial_tx_data_tlv_count                               :  8,
                      tx_data_tlv_count                                       : 16;
#else
             uint32_t tx_data_tlv_count                                       : 16,
                      partial_tx_data_tlv_count                               :  8,
                      available_memory                                        :  8;
#endif
};

#define TXPCU_BUFFER_BASICS_AVAILABLE_MEMORY_OFFSET                                 0x00000000
#define TXPCU_BUFFER_BASICS_AVAILABLE_MEMORY_LSB                                    0
#define TXPCU_BUFFER_BASICS_AVAILABLE_MEMORY_MSB                                    7
#define TXPCU_BUFFER_BASICS_AVAILABLE_MEMORY_MASK                                   0x000000ff

#define TXPCU_BUFFER_BASICS_PARTIAL_TX_DATA_TLV_COUNT_OFFSET                        0x00000000
#define TXPCU_BUFFER_BASICS_PARTIAL_TX_DATA_TLV_COUNT_LSB                           8
#define TXPCU_BUFFER_BASICS_PARTIAL_TX_DATA_TLV_COUNT_MSB                           15
#define TXPCU_BUFFER_BASICS_PARTIAL_TX_DATA_TLV_COUNT_MASK                          0x0000ff00

#define TXPCU_BUFFER_BASICS_TX_DATA_TLV_COUNT_OFFSET                                0x00000000
#define TXPCU_BUFFER_BASICS_TX_DATA_TLV_COUNT_LSB                                   16
#define TXPCU_BUFFER_BASICS_TX_DATA_TLV_COUNT_MSB                                   31
#define TXPCU_BUFFER_BASICS_TX_DATA_TLV_COUNT_MASK                                  0xffff0000

#endif
