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


#ifndef _REO_FLUSH_QUEUE_H_
#define _REO_FLUSH_QUEUE_H_

#include "uniform_reo_cmd_header.h"
#define NUM_OF_DWORDS_REO_FLUSH_QUEUE 9

struct reo_flush_queue {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t flush_desc_addr_31_0                                    : 32;
             uint32_t flush_desc_addr_39_32                                   :  8,
                      block_desc_addr_usage_after_flush                       :  1,
                      block_resource_index                                    :  2,
                      reserved_2a                                             : 21;
             uint32_t reserved_3a                                             : 32;
             uint32_t reserved_4a                                             : 32;
             uint32_t reserved_5a                                             : 32;
             uint32_t reserved_6a                                             : 32;
             uint32_t reserved_7a                                             : 32;
             uint32_t reserved_8a                                             : 32;
#else
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t flush_desc_addr_31_0                                    : 32;
             uint32_t reserved_2a                                             : 21,
                      block_resource_index                                    :  2,
                      block_desc_addr_usage_after_flush                       :  1,
                      flush_desc_addr_39_32                                   :  8;
             uint32_t reserved_3a                                             : 32;
             uint32_t reserved_4a                                             : 32;
             uint32_t reserved_5a                                             : 32;
             uint32_t reserved_6a                                             : 32;
             uint32_t reserved_7a                                             : 32;
             uint32_t reserved_8a                                             : 32;
#endif
};

#define REO_FLUSH_QUEUE_CMD_HEADER_REO_CMD_NUMBER_OFFSET                            0x00000000
#define REO_FLUSH_QUEUE_CMD_HEADER_REO_CMD_NUMBER_LSB                               0
#define REO_FLUSH_QUEUE_CMD_HEADER_REO_CMD_NUMBER_MSB                               15
#define REO_FLUSH_QUEUE_CMD_HEADER_REO_CMD_NUMBER_MASK                              0x0000ffff

#define REO_FLUSH_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET                       0x00000000
#define REO_FLUSH_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_LSB                          16
#define REO_FLUSH_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_MSB                          16
#define REO_FLUSH_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_MASK                         0x00010000

#define REO_FLUSH_QUEUE_CMD_HEADER_RESERVED_0A_OFFSET                               0x00000000
#define REO_FLUSH_QUEUE_CMD_HEADER_RESERVED_0A_LSB                                  17
#define REO_FLUSH_QUEUE_CMD_HEADER_RESERVED_0A_MSB                                  31
#define REO_FLUSH_QUEUE_CMD_HEADER_RESERVED_0A_MASK                                 0xfffe0000

#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_31_0_OFFSET                                 0x00000004
#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_31_0_LSB                                    0
#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_31_0_MSB                                    31
#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_31_0_MASK                                   0xffffffff

#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_39_32_OFFSET                                0x00000008
#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_39_32_LSB                                   0
#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_39_32_MSB                                   7
#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_39_32_MASK                                  0x000000ff

#define REO_FLUSH_QUEUE_BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH_OFFSET                    0x00000008
#define REO_FLUSH_QUEUE_BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH_LSB                       8
#define REO_FLUSH_QUEUE_BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH_MSB                       8
#define REO_FLUSH_QUEUE_BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH_MASK                      0x00000100

#define REO_FLUSH_QUEUE_BLOCK_RESOURCE_INDEX_OFFSET                                 0x00000008
#define REO_FLUSH_QUEUE_BLOCK_RESOURCE_INDEX_LSB                                    9
#define REO_FLUSH_QUEUE_BLOCK_RESOURCE_INDEX_MSB                                    10
#define REO_FLUSH_QUEUE_BLOCK_RESOURCE_INDEX_MASK                                   0x00000600

#define REO_FLUSH_QUEUE_RESERVED_2A_OFFSET                                          0x00000008
#define REO_FLUSH_QUEUE_RESERVED_2A_LSB                                             11
#define REO_FLUSH_QUEUE_RESERVED_2A_MSB                                             31
#define REO_FLUSH_QUEUE_RESERVED_2A_MASK                                            0xfffff800

#define REO_FLUSH_QUEUE_RESERVED_3A_OFFSET                                          0x0000000c
#define REO_FLUSH_QUEUE_RESERVED_3A_LSB                                             0
#define REO_FLUSH_QUEUE_RESERVED_3A_MSB                                             31
#define REO_FLUSH_QUEUE_RESERVED_3A_MASK                                            0xffffffff

#define REO_FLUSH_QUEUE_RESERVED_4A_OFFSET                                          0x00000010
#define REO_FLUSH_QUEUE_RESERVED_4A_LSB                                             0
#define REO_FLUSH_QUEUE_RESERVED_4A_MSB                                             31
#define REO_FLUSH_QUEUE_RESERVED_4A_MASK                                            0xffffffff

#define REO_FLUSH_QUEUE_RESERVED_5A_OFFSET                                          0x00000014
#define REO_FLUSH_QUEUE_RESERVED_5A_LSB                                             0
#define REO_FLUSH_QUEUE_RESERVED_5A_MSB                                             31
#define REO_FLUSH_QUEUE_RESERVED_5A_MASK                                            0xffffffff

#define REO_FLUSH_QUEUE_RESERVED_6A_OFFSET                                          0x00000018
#define REO_FLUSH_QUEUE_RESERVED_6A_LSB                                             0
#define REO_FLUSH_QUEUE_RESERVED_6A_MSB                                             31
#define REO_FLUSH_QUEUE_RESERVED_6A_MASK                                            0xffffffff

#define REO_FLUSH_QUEUE_RESERVED_7A_OFFSET                                          0x0000001c
#define REO_FLUSH_QUEUE_RESERVED_7A_LSB                                             0
#define REO_FLUSH_QUEUE_RESERVED_7A_MSB                                             31
#define REO_FLUSH_QUEUE_RESERVED_7A_MASK                                            0xffffffff

#define REO_FLUSH_QUEUE_RESERVED_8A_OFFSET                                          0x00000020
#define REO_FLUSH_QUEUE_RESERVED_8A_LSB                                             0
#define REO_FLUSH_QUEUE_RESERVED_8A_MSB                                             31
#define REO_FLUSH_QUEUE_RESERVED_8A_MASK                                            0xffffffff

#endif
