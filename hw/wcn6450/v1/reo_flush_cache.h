
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



#ifndef _REO_FLUSH_CACHE_H_
#define _REO_FLUSH_CACHE_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_cmd_header.h"

#define NUM_OF_DWORDS_REO_FLUSH_CACHE 9

struct reo_flush_cache {
    struct            uniform_reo_cmd_header                       cmd_header;
             uint32_t flush_addr_31_0                 : 32;
             uint32_t flush_addr_39_32                :  8,
                      forward_all_mpdus_in_queue      :  1,
                      release_cache_block_index       :  1,
                      cache_block_resource_index      :  2,
                      flush_without_invalidate        :  1,
                      block_cache_usage_after_flush   :  1,
                      flush_entire_cache              :  1,
                      reserved_2b                     : 17;
             uint32_t reserved_3a                     : 32;
             uint32_t reserved_4a                     : 32;
             uint32_t reserved_5a                     : 32;
             uint32_t reserved_6a                     : 32;
             uint32_t reserved_7a                     : 32;
             uint32_t reserved_8a                     : 32;
};

#define REO_FLUSH_CACHE_0_CMD_HEADER_REO_CMD_NUMBER_OFFSET           0x00000000
#define REO_FLUSH_CACHE_0_CMD_HEADER_REO_CMD_NUMBER_LSB              0
#define REO_FLUSH_CACHE_0_CMD_HEADER_REO_CMD_NUMBER_MASK             0x0000ffff

#define REO_FLUSH_CACHE_0_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET      0x00000000
#define REO_FLUSH_CACHE_0_CMD_HEADER_REO_STATUS_REQUIRED_LSB         16
#define REO_FLUSH_CACHE_0_CMD_HEADER_REO_STATUS_REQUIRED_MASK        0x00010000

#define REO_FLUSH_CACHE_0_CMD_HEADER_RESERVED_0A_OFFSET              0x00000000
#define REO_FLUSH_CACHE_0_CMD_HEADER_RESERVED_0A_LSB                 17
#define REO_FLUSH_CACHE_0_CMD_HEADER_RESERVED_0A_MASK                0xfffe0000

#define REO_FLUSH_CACHE_1_FLUSH_ADDR_31_0_OFFSET                     0x00000004
#define REO_FLUSH_CACHE_1_FLUSH_ADDR_31_0_LSB                        0
#define REO_FLUSH_CACHE_1_FLUSH_ADDR_31_0_MASK                       0xffffffff

#define REO_FLUSH_CACHE_2_FLUSH_ADDR_39_32_OFFSET                    0x00000008
#define REO_FLUSH_CACHE_2_FLUSH_ADDR_39_32_LSB                       0
#define REO_FLUSH_CACHE_2_FLUSH_ADDR_39_32_MASK                      0x000000ff

#define REO_FLUSH_CACHE_2_FORWARD_ALL_MPDUS_IN_QUEUE_OFFSET          0x00000008
#define REO_FLUSH_CACHE_2_FORWARD_ALL_MPDUS_IN_QUEUE_LSB             8
#define REO_FLUSH_CACHE_2_FORWARD_ALL_MPDUS_IN_QUEUE_MASK            0x00000100

#define REO_FLUSH_CACHE_2_RELEASE_CACHE_BLOCK_INDEX_OFFSET           0x00000008
#define REO_FLUSH_CACHE_2_RELEASE_CACHE_BLOCK_INDEX_LSB              9
#define REO_FLUSH_CACHE_2_RELEASE_CACHE_BLOCK_INDEX_MASK             0x00000200

#define REO_FLUSH_CACHE_2_CACHE_BLOCK_RESOURCE_INDEX_OFFSET          0x00000008
#define REO_FLUSH_CACHE_2_CACHE_BLOCK_RESOURCE_INDEX_LSB             10
#define REO_FLUSH_CACHE_2_CACHE_BLOCK_RESOURCE_INDEX_MASK            0x00000c00

#define REO_FLUSH_CACHE_2_FLUSH_WITHOUT_INVALIDATE_OFFSET            0x00000008
#define REO_FLUSH_CACHE_2_FLUSH_WITHOUT_INVALIDATE_LSB               12
#define REO_FLUSH_CACHE_2_FLUSH_WITHOUT_INVALIDATE_MASK              0x00001000

#define REO_FLUSH_CACHE_2_BLOCK_CACHE_USAGE_AFTER_FLUSH_OFFSET       0x00000008
#define REO_FLUSH_CACHE_2_BLOCK_CACHE_USAGE_AFTER_FLUSH_LSB          13
#define REO_FLUSH_CACHE_2_BLOCK_CACHE_USAGE_AFTER_FLUSH_MASK         0x00002000

#define REO_FLUSH_CACHE_2_FLUSH_ENTIRE_CACHE_OFFSET                  0x00000008
#define REO_FLUSH_CACHE_2_FLUSH_ENTIRE_CACHE_LSB                     14
#define REO_FLUSH_CACHE_2_FLUSH_ENTIRE_CACHE_MASK                    0x00004000

#define REO_FLUSH_CACHE_2_RESERVED_2B_OFFSET                         0x00000008
#define REO_FLUSH_CACHE_2_RESERVED_2B_LSB                            15
#define REO_FLUSH_CACHE_2_RESERVED_2B_MASK                           0xffff8000

#define REO_FLUSH_CACHE_3_RESERVED_3A_OFFSET                         0x0000000c
#define REO_FLUSH_CACHE_3_RESERVED_3A_LSB                            0
#define REO_FLUSH_CACHE_3_RESERVED_3A_MASK                           0xffffffff

#define REO_FLUSH_CACHE_4_RESERVED_4A_OFFSET                         0x00000010
#define REO_FLUSH_CACHE_4_RESERVED_4A_LSB                            0
#define REO_FLUSH_CACHE_4_RESERVED_4A_MASK                           0xffffffff

#define REO_FLUSH_CACHE_5_RESERVED_5A_OFFSET                         0x00000014
#define REO_FLUSH_CACHE_5_RESERVED_5A_LSB                            0
#define REO_FLUSH_CACHE_5_RESERVED_5A_MASK                           0xffffffff

#define REO_FLUSH_CACHE_6_RESERVED_6A_OFFSET                         0x00000018
#define REO_FLUSH_CACHE_6_RESERVED_6A_LSB                            0
#define REO_FLUSH_CACHE_6_RESERVED_6A_MASK                           0xffffffff

#define REO_FLUSH_CACHE_7_RESERVED_7A_OFFSET                         0x0000001c
#define REO_FLUSH_CACHE_7_RESERVED_7A_LSB                            0
#define REO_FLUSH_CACHE_7_RESERVED_7A_MASK                           0xffffffff

#define REO_FLUSH_CACHE_8_RESERVED_8A_OFFSET                         0x00000020
#define REO_FLUSH_CACHE_8_RESERVED_8A_LSB                            0
#define REO_FLUSH_CACHE_8_RESERVED_8A_MASK                           0xffffffff

#endif
