/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	struct uniform_reo_cmd_header cmd_header;
//	1	flush_addr_31_0[31:0]
//	2	flush_addr_39_32[7:0], forward_all_mpdus_in_queue[8], release_cache_block_index[9], cache_block_resource_index[11:10], flush_without_invalidate[12], block_cache_usage_after_flush[13], flush_entire_cache[14], reserved_2b[31:15]
//	3	reserved_3a[31:0]
//	4	reserved_4a[31:0]
//	5	reserved_5a[31:0]
//	6	reserved_6a[31:0]
//	7	reserved_7a[31:0]
//	8	reserved_8a[31:0]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_REO_FLUSH_CACHE 9

struct reo_flush_cache {
    struct            uniform_reo_cmd_header                       cmd_header;
             uint32_t flush_addr_31_0                 : 32; //[31:0]
             uint32_t flush_addr_39_32                :  8, //[7:0]
                      forward_all_mpdus_in_queue      :  1, //[8]
                      release_cache_block_index       :  1, //[9]
                      cache_block_resource_index      :  2, //[11:10]
                      flush_without_invalidate        :  1, //[12]
                      block_cache_usage_after_flush   :  1, //[13]
                      flush_entire_cache              :  1, //[14]
                      reserved_2b                     : 17; //[31:15]
             uint32_t reserved_3a                     : 32; //[31:0]
             uint32_t reserved_4a                     : 32; //[31:0]
             uint32_t reserved_5a                     : 32; //[31:0]
             uint32_t reserved_6a                     : 32; //[31:0]
             uint32_t reserved_7a                     : 32; //[31:0]
             uint32_t reserved_8a                     : 32; //[31:0]
};

/*

struct uniform_reo_cmd_header cmd_header
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Details for command execution tracking purposes.

flush_addr_31_0
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Address (lower 32 bits) of the descriptor to flush
			
			<legal all>

flush_addr_39_32
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Address (upper 8 bits) of the descriptor to flush
			
			<legal all>

forward_all_mpdus_in_queue
			
			Is only allowed to be set when the flush address
			corresponds with a REO descriptor.
			
			
			
			When set, REO shall first forward all the MPDUs held in
			the indicated re-order queue, before flushing the descriptor
			from the cache.
			
			<legal all>

release_cache_block_index
			
			Field not valid when Flush_entire_cache is set.
			
			
			
			If SW has previously used a blocking resource that it
			now wants to re-use for this command, this bit shall be set.
			It prevents SW from having to send a separate
			REO_UNBLOCK_CACHE command.
			
			
			
			When set, HW will first release the blocking resource
			(indicated in field 'Cache_block_resouce_index') before this
			command gets executed.
			
			If that resource was already unblocked, this will be
			considered an error. This command will not be executed, and
			an error shall be returned.
			
			<legal all>

cache_block_resource_index
			
			Field not valid when Flush_entire_cache is set.
			
			
			
			Indicates which of the four blocking resources in REO
			will be assigned for managing the blocking of this
			(descriptor) address 
			
			<legal all>

flush_without_invalidate
			
			Field not valid when Flush_entire_cache is set.
			
			
			
			When set, REO shall flush the cache line contents from
			the cache, but there is NO need to invalidate the cache line
			entry... The contents in the cache can be maintained. This
			feature can be used by SW (and DV) to get a current snapshot
			of the contents in the cache
			
			
			
			<legal all>

block_cache_usage_after_flush
			
			Field not valid when Flush_entire_cache is set.
			
			
			
			When set, REO shall block any cache accesses to this
			address till explicitly unblocked. 
			
			
			
			Whenever SW sets this bit, SW shall also set bit
			'Forward_all_mpdus_in_queue' to ensure all packets are
			flushed out in order to make sure this queue desc is not in
			one of the aging link lists. In case SW does not want to
			flush the MPDUs in the queue, see the recipe description
			below this TLV definition.
			
			
			
			The 'blocking' index to be used for this is indicated in
			field 'cache_block_resource_index'. If SW had previously
			used this blocking resource and was not freed up yet, SW
			shall first unblock that index (by setting bit
			Release_cache_block_index) or use an unblock command.
			
			
			
			If the resource indicated here was already blocked (and
			did not get unblocked in this command), it is considered an
			error scenario...
			
			No flush shall happen. The status for this command shall
			indicate error.
			
			
			
			<legal all>

flush_entire_cache
			
			When set, the entire cache shall be flushed. The entire
			cache will also remain blocked, till the
			'REO_UNBLOCK_COMMAND' is received with bit unblock type set
			to unblock_cache. All other fields in this command are to be
			ignored.
			
			
			
			Note that flushing the entire cache has no changes to
			the current settings of the blocking resource settings
			
			
			
			<legal all>

reserved_2b
			
			<legal 0>

reserved_3a
			
			<legal 0>

reserved_4a
			
			<legal 0>

reserved_5a
			
			<legal 0>

reserved_6a
			
			<legal 0>

reserved_7a
			
			<legal 0>

reserved_8a
			
			<legal 0>
*/

#define REO_FLUSH_CACHE_0_UNIFORM_REO_CMD_HEADER_CMD_HEADER_OFFSET   0x00000000
#define REO_FLUSH_CACHE_0_UNIFORM_REO_CMD_HEADER_CMD_HEADER_LSB      0
#define REO_FLUSH_CACHE_0_UNIFORM_REO_CMD_HEADER_CMD_HEADER_MASK     0xffffffff

/* Description		REO_FLUSH_CACHE_1_FLUSH_ADDR_31_0
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Address (lower 32 bits) of the descriptor to flush
			
			<legal all>
*/
#define REO_FLUSH_CACHE_1_FLUSH_ADDR_31_0_OFFSET                     0x00000004
#define REO_FLUSH_CACHE_1_FLUSH_ADDR_31_0_LSB                        0
#define REO_FLUSH_CACHE_1_FLUSH_ADDR_31_0_MASK                       0xffffffff

/* Description		REO_FLUSH_CACHE_2_FLUSH_ADDR_39_32
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Address (upper 8 bits) of the descriptor to flush
			
			<legal all>
*/
#define REO_FLUSH_CACHE_2_FLUSH_ADDR_39_32_OFFSET                    0x00000008
#define REO_FLUSH_CACHE_2_FLUSH_ADDR_39_32_LSB                       0
#define REO_FLUSH_CACHE_2_FLUSH_ADDR_39_32_MASK                      0x000000ff

/* Description		REO_FLUSH_CACHE_2_FORWARD_ALL_MPDUS_IN_QUEUE
			
			Is only allowed to be set when the flush address
			corresponds with a REO descriptor.
			
			
			
			When set, REO shall first forward all the MPDUs held in
			the indicated re-order queue, before flushing the descriptor
			from the cache.
			
			<legal all>
*/
#define REO_FLUSH_CACHE_2_FORWARD_ALL_MPDUS_IN_QUEUE_OFFSET          0x00000008
#define REO_FLUSH_CACHE_2_FORWARD_ALL_MPDUS_IN_QUEUE_LSB             8
#define REO_FLUSH_CACHE_2_FORWARD_ALL_MPDUS_IN_QUEUE_MASK            0x00000100

/* Description		REO_FLUSH_CACHE_2_RELEASE_CACHE_BLOCK_INDEX
			
			Field not valid when Flush_entire_cache is set.
			
			
			
			If SW has previously used a blocking resource that it
			now wants to re-use for this command, this bit shall be set.
			It prevents SW from having to send a separate
			REO_UNBLOCK_CACHE command.
			
			
			
			When set, HW will first release the blocking resource
			(indicated in field 'Cache_block_resouce_index') before this
			command gets executed.
			
			If that resource was already unblocked, this will be
			considered an error. This command will not be executed, and
			an error shall be returned.
			
			<legal all>
*/
#define REO_FLUSH_CACHE_2_RELEASE_CACHE_BLOCK_INDEX_OFFSET           0x00000008
#define REO_FLUSH_CACHE_2_RELEASE_CACHE_BLOCK_INDEX_LSB              9
#define REO_FLUSH_CACHE_2_RELEASE_CACHE_BLOCK_INDEX_MASK             0x00000200

/* Description		REO_FLUSH_CACHE_2_CACHE_BLOCK_RESOURCE_INDEX
			
			Field not valid when Flush_entire_cache is set.
			
			
			
			Indicates which of the four blocking resources in REO
			will be assigned for managing the blocking of this
			(descriptor) address 
			
			<legal all>
*/
#define REO_FLUSH_CACHE_2_CACHE_BLOCK_RESOURCE_INDEX_OFFSET          0x00000008
#define REO_FLUSH_CACHE_2_CACHE_BLOCK_RESOURCE_INDEX_LSB             10
#define REO_FLUSH_CACHE_2_CACHE_BLOCK_RESOURCE_INDEX_MASK            0x00000c00

/* Description		REO_FLUSH_CACHE_2_FLUSH_WITHOUT_INVALIDATE
			
			Field not valid when Flush_entire_cache is set.
			
			
			
			When set, REO shall flush the cache line contents from
			the cache, but there is NO need to invalidate the cache line
			entry... The contents in the cache can be maintained. This
			feature can be used by SW (and DV) to get a current snapshot
			of the contents in the cache
			
			
			
			<legal all>
*/
#define REO_FLUSH_CACHE_2_FLUSH_WITHOUT_INVALIDATE_OFFSET            0x00000008
#define REO_FLUSH_CACHE_2_FLUSH_WITHOUT_INVALIDATE_LSB               12
#define REO_FLUSH_CACHE_2_FLUSH_WITHOUT_INVALIDATE_MASK              0x00001000

/* Description		REO_FLUSH_CACHE_2_BLOCK_CACHE_USAGE_AFTER_FLUSH
			
			Field not valid when Flush_entire_cache is set.
			
			
			
			When set, REO shall block any cache accesses to this
			address till explicitly unblocked. 
			
			
			
			Whenever SW sets this bit, SW shall also set bit
			'Forward_all_mpdus_in_queue' to ensure all packets are
			flushed out in order to make sure this queue desc is not in
			one of the aging link lists. In case SW does not want to
			flush the MPDUs in the queue, see the recipe description
			below this TLV definition.
			
			
			
			The 'blocking' index to be used for this is indicated in
			field 'cache_block_resource_index'. If SW had previously
			used this blocking resource and was not freed up yet, SW
			shall first unblock that index (by setting bit
			Release_cache_block_index) or use an unblock command.
			
			
			
			If the resource indicated here was already blocked (and
			did not get unblocked in this command), it is considered an
			error scenario...
			
			No flush shall happen. The status for this command shall
			indicate error.
			
			
			
			<legal all>
*/
#define REO_FLUSH_CACHE_2_BLOCK_CACHE_USAGE_AFTER_FLUSH_OFFSET       0x00000008
#define REO_FLUSH_CACHE_2_BLOCK_CACHE_USAGE_AFTER_FLUSH_LSB          13
#define REO_FLUSH_CACHE_2_BLOCK_CACHE_USAGE_AFTER_FLUSH_MASK         0x00002000

/* Description		REO_FLUSH_CACHE_2_FLUSH_ENTIRE_CACHE
			
			When set, the entire cache shall be flushed. The entire
			cache will also remain blocked, till the
			'REO_UNBLOCK_COMMAND' is received with bit unblock type set
			to unblock_cache. All other fields in this command are to be
			ignored.
			
			
			
			Note that flushing the entire cache has no changes to
			the current settings of the blocking resource settings
			
			
			
			<legal all>
*/
#define REO_FLUSH_CACHE_2_FLUSH_ENTIRE_CACHE_OFFSET                  0x00000008
#define REO_FLUSH_CACHE_2_FLUSH_ENTIRE_CACHE_LSB                     14
#define REO_FLUSH_CACHE_2_FLUSH_ENTIRE_CACHE_MASK                    0x00004000

/* Description		REO_FLUSH_CACHE_2_RESERVED_2B
			
			<legal 0>
*/
#define REO_FLUSH_CACHE_2_RESERVED_2B_OFFSET                         0x00000008
#define REO_FLUSH_CACHE_2_RESERVED_2B_LSB                            15
#define REO_FLUSH_CACHE_2_RESERVED_2B_MASK                           0xffff8000

/* Description		REO_FLUSH_CACHE_3_RESERVED_3A
			
			<legal 0>
*/
#define REO_FLUSH_CACHE_3_RESERVED_3A_OFFSET                         0x0000000c
#define REO_FLUSH_CACHE_3_RESERVED_3A_LSB                            0
#define REO_FLUSH_CACHE_3_RESERVED_3A_MASK                           0xffffffff

/* Description		REO_FLUSH_CACHE_4_RESERVED_4A
			
			<legal 0>
*/
#define REO_FLUSH_CACHE_4_RESERVED_4A_OFFSET                         0x00000010
#define REO_FLUSH_CACHE_4_RESERVED_4A_LSB                            0
#define REO_FLUSH_CACHE_4_RESERVED_4A_MASK                           0xffffffff

/* Description		REO_FLUSH_CACHE_5_RESERVED_5A
			
			<legal 0>
*/
#define REO_FLUSH_CACHE_5_RESERVED_5A_OFFSET                         0x00000014
#define REO_FLUSH_CACHE_5_RESERVED_5A_LSB                            0
#define REO_FLUSH_CACHE_5_RESERVED_5A_MASK                           0xffffffff

/* Description		REO_FLUSH_CACHE_6_RESERVED_6A
			
			<legal 0>
*/
#define REO_FLUSH_CACHE_6_RESERVED_6A_OFFSET                         0x00000018
#define REO_FLUSH_CACHE_6_RESERVED_6A_LSB                            0
#define REO_FLUSH_CACHE_6_RESERVED_6A_MASK                           0xffffffff

/* Description		REO_FLUSH_CACHE_7_RESERVED_7A
			
			<legal 0>
*/
#define REO_FLUSH_CACHE_7_RESERVED_7A_OFFSET                         0x0000001c
#define REO_FLUSH_CACHE_7_RESERVED_7A_LSB                            0
#define REO_FLUSH_CACHE_7_RESERVED_7A_MASK                           0xffffffff

/* Description		REO_FLUSH_CACHE_8_RESERVED_8A
			
			<legal 0>
*/
#define REO_FLUSH_CACHE_8_RESERVED_8A_OFFSET                         0x00000020
#define REO_FLUSH_CACHE_8_RESERVED_8A_LSB                            0
#define REO_FLUSH_CACHE_8_RESERVED_8A_MASK                           0xffffffff


#endif // _REO_FLUSH_CACHE_H_
