
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



#ifndef _REO_UNBLOCK_CACHE_H_
#define _REO_UNBLOCK_CACHE_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_cmd_header.h"

#define NUM_OF_DWORDS_REO_UNBLOCK_CACHE 9

struct reo_unblock_cache {
    struct            uniform_reo_cmd_header                       cmd_header;
             uint32_t unblock_type                    :  1,
                      cache_block_resource_index      :  2,
                      reserved_1a                     : 29;
             uint32_t reserved_2a                     : 32;
             uint32_t reserved_3a                     : 32;
             uint32_t reserved_4a                     : 32;
             uint32_t reserved_5a                     : 32;
             uint32_t reserved_6a                     : 32;
             uint32_t reserved_7a                     : 32;
             uint32_t reserved_8a                     : 32;
};

#define REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_CMD_NUMBER_OFFSET         0x00000000
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_CMD_NUMBER_LSB            0
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_CMD_NUMBER_MASK           0x0000ffff

#define REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET    0x00000000
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_STATUS_REQUIRED_LSB       16
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_STATUS_REQUIRED_MASK      0x00010000

#define REO_UNBLOCK_CACHE_0_CMD_HEADER_RESERVED_0A_OFFSET            0x00000000
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_RESERVED_0A_LSB               17
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_RESERVED_0A_MASK              0xfffe0000

#define REO_UNBLOCK_CACHE_1_UNBLOCK_TYPE_OFFSET                      0x00000004
#define REO_UNBLOCK_CACHE_1_UNBLOCK_TYPE_LSB                         0
#define REO_UNBLOCK_CACHE_1_UNBLOCK_TYPE_MASK                        0x00000001

#define REO_UNBLOCK_CACHE_1_CACHE_BLOCK_RESOURCE_INDEX_OFFSET        0x00000004
#define REO_UNBLOCK_CACHE_1_CACHE_BLOCK_RESOURCE_INDEX_LSB           1
#define REO_UNBLOCK_CACHE_1_CACHE_BLOCK_RESOURCE_INDEX_MASK          0x00000006

#define REO_UNBLOCK_CACHE_1_RESERVED_1A_OFFSET                       0x00000004
#define REO_UNBLOCK_CACHE_1_RESERVED_1A_LSB                          3
#define REO_UNBLOCK_CACHE_1_RESERVED_1A_MASK                         0xfffffff8

#define REO_UNBLOCK_CACHE_2_RESERVED_2A_OFFSET                       0x00000008
#define REO_UNBLOCK_CACHE_2_RESERVED_2A_LSB                          0
#define REO_UNBLOCK_CACHE_2_RESERVED_2A_MASK                         0xffffffff

#define REO_UNBLOCK_CACHE_3_RESERVED_3A_OFFSET                       0x0000000c
#define REO_UNBLOCK_CACHE_3_RESERVED_3A_LSB                          0
#define REO_UNBLOCK_CACHE_3_RESERVED_3A_MASK                         0xffffffff

#define REO_UNBLOCK_CACHE_4_RESERVED_4A_OFFSET                       0x00000010
#define REO_UNBLOCK_CACHE_4_RESERVED_4A_LSB                          0
#define REO_UNBLOCK_CACHE_4_RESERVED_4A_MASK                         0xffffffff

#define REO_UNBLOCK_CACHE_5_RESERVED_5A_OFFSET                       0x00000014
#define REO_UNBLOCK_CACHE_5_RESERVED_5A_LSB                          0
#define REO_UNBLOCK_CACHE_5_RESERVED_5A_MASK                         0xffffffff

#define REO_UNBLOCK_CACHE_6_RESERVED_6A_OFFSET                       0x00000018
#define REO_UNBLOCK_CACHE_6_RESERVED_6A_LSB                          0
#define REO_UNBLOCK_CACHE_6_RESERVED_6A_MASK                         0xffffffff

#define REO_UNBLOCK_CACHE_7_RESERVED_7A_OFFSET                       0x0000001c
#define REO_UNBLOCK_CACHE_7_RESERVED_7A_LSB                          0
#define REO_UNBLOCK_CACHE_7_RESERVED_7A_MASK                         0xffffffff

#define REO_UNBLOCK_CACHE_8_RESERVED_8A_OFFSET                       0x00000020
#define REO_UNBLOCK_CACHE_8_RESERVED_8A_LSB                          0
#define REO_UNBLOCK_CACHE_8_RESERVED_8A_MASK                         0xffffffff

#endif
