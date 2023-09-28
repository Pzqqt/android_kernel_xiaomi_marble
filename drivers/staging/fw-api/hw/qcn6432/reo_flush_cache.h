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

#ifndef _REO_FLUSH_CACHE_H_
#define _REO_FLUSH_CACHE_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_cmd_header.h"
#define NUM_OF_DWORDS_REO_FLUSH_CACHE 10

#define NUM_OF_QWORDS_REO_FLUSH_CACHE 5


struct reo_flush_cache {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t flush_addr_31_0                                         : 32; // [31:0]
             uint32_t flush_addr_39_32                                        :  8, // [7:0]
                      forward_all_mpdus_in_queue                              :  1, // [8:8]
                      release_cache_block_index                               :  1, // [9:9]
                      cache_block_resource_index                              :  2, // [11:10]
                      flush_without_invalidate                                :  1, // [12:12]
                      block_cache_usage_after_flush                           :  1, // [13:13]
                      flush_entire_cache                                      :  1, // [14:14]
                      flush_queue_1k_desc                                     :  1, // [15:15]
                      reserved_2b                                             : 16; // [31:16]
             uint32_t reserved_3a                                             : 32; // [31:0]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 32; // [31:0]
             uint32_t reserved_8a                                             : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t flush_addr_31_0                                         : 32; // [31:0]
             uint32_t reserved_2b                                             : 16, // [31:16]
                      flush_queue_1k_desc                                     :  1, // [15:15]
                      flush_entire_cache                                      :  1, // [14:14]
                      block_cache_usage_after_flush                           :  1, // [13:13]
                      flush_without_invalidate                                :  1, // [12:12]
                      cache_block_resource_index                              :  2, // [11:10]
                      release_cache_block_index                               :  1, // [9:9]
                      forward_all_mpdus_in_queue                              :  1, // [8:8]
                      flush_addr_39_32                                        :  8; // [7:0]
             uint32_t reserved_3a                                             : 32; // [31:0]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 32; // [31:0]
             uint32_t reserved_8a                                             : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		CMD_HEADER

			Consumer: REO
			Producer: SW
			
			Details for command execution tracking purposes.
*/


/* Description		REO_CMD_NUMBER

			Consumer: REO/SW/DEBUG
			Producer: SW 
			
			This number can be used by SW to track, identify and link
			 the created commands with the command statusses
			
			
			<legal all> 
*/

#define REO_FLUSH_CACHE_CMD_HEADER_REO_CMD_NUMBER_OFFSET                            0x0000000000000000
#define REO_FLUSH_CACHE_CMD_HEADER_REO_CMD_NUMBER_LSB                               0
#define REO_FLUSH_CACHE_CMD_HEADER_REO_CMD_NUMBER_MSB                               15
#define REO_FLUSH_CACHE_CMD_HEADER_REO_CMD_NUMBER_MASK                              0x000000000000ffff


/* Description		REO_STATUS_REQUIRED

			Consumer: REO
			Producer: SW 
			
			<enum 0 NoStatus> REO does not need to generate a status
			 TLV for the execution of this command
			<enum 1 StatusRequired> REO shall generate a status TLV 
			for the execution of this command
			
			<legal all>
*/

#define REO_FLUSH_CACHE_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET                       0x0000000000000000
#define REO_FLUSH_CACHE_CMD_HEADER_REO_STATUS_REQUIRED_LSB                          16
#define REO_FLUSH_CACHE_CMD_HEADER_REO_STATUS_REQUIRED_MSB                          16
#define REO_FLUSH_CACHE_CMD_HEADER_REO_STATUS_REQUIRED_MASK                         0x0000000000010000


/* Description		RESERVED_0A

			<legal 0>
*/

#define REO_FLUSH_CACHE_CMD_HEADER_RESERVED_0A_OFFSET                               0x0000000000000000
#define REO_FLUSH_CACHE_CMD_HEADER_RESERVED_0A_LSB                                  17
#define REO_FLUSH_CACHE_CMD_HEADER_RESERVED_0A_MSB                                  31
#define REO_FLUSH_CACHE_CMD_HEADER_RESERVED_0A_MASK                                 0x00000000fffe0000


/* Description		FLUSH_ADDR_31_0

			Consumer: REO
			Producer: SW
			
			Address (lower 32 bits) of the descriptor to flush
			<legal all>
*/

#define REO_FLUSH_CACHE_FLUSH_ADDR_31_0_OFFSET                                      0x0000000000000000
#define REO_FLUSH_CACHE_FLUSH_ADDR_31_0_LSB                                         32
#define REO_FLUSH_CACHE_FLUSH_ADDR_31_0_MSB                                         63
#define REO_FLUSH_CACHE_FLUSH_ADDR_31_0_MASK                                        0xffffffff00000000


/* Description		FLUSH_ADDR_39_32

			Consumer: REO
			Producer: SW
			
			Address (upper 8 bits) of the descriptor to flush
			<legal all>
*/

#define REO_FLUSH_CACHE_FLUSH_ADDR_39_32_OFFSET                                     0x0000000000000008
#define REO_FLUSH_CACHE_FLUSH_ADDR_39_32_LSB                                        0
#define REO_FLUSH_CACHE_FLUSH_ADDR_39_32_MSB                                        7
#define REO_FLUSH_CACHE_FLUSH_ADDR_39_32_MASK                                       0x00000000000000ff


/* Description		FORWARD_ALL_MPDUS_IN_QUEUE

			Is only allowed to be set when the flush address corresponds
			 with a REO descriptor.
			
			When set, REO shall first forward all the MPDUs held in 
			the indicated re-order queue, before flushing the descriptor
			 from the cache.
			<legal all>
*/

#define REO_FLUSH_CACHE_FORWARD_ALL_MPDUS_IN_QUEUE_OFFSET                           0x0000000000000008
#define REO_FLUSH_CACHE_FORWARD_ALL_MPDUS_IN_QUEUE_LSB                              8
#define REO_FLUSH_CACHE_FORWARD_ALL_MPDUS_IN_QUEUE_MSB                              8
#define REO_FLUSH_CACHE_FORWARD_ALL_MPDUS_IN_QUEUE_MASK                             0x0000000000000100


/* Description		RELEASE_CACHE_BLOCK_INDEX

			Field not valid when Flush_entire_cache is set.
			
			If SW has previously used a blocking resource that it now
			 wants to re-use for this command, this bit shall be set. 
			It prevents SW from having to send a separate REO_UNBLOCK_CACHE
			 command.
			
			When set, HW will first release the blocking resource (indicated
			 in field 'Cache_block_resouce_index') before this command
			 gets executed.
			If that resource was already unblocked, this will be considered
			 an error. This command will not be executed, and an error
			 shall be returned.
			<legal all>
*/

#define REO_FLUSH_CACHE_RELEASE_CACHE_BLOCK_INDEX_OFFSET                            0x0000000000000008
#define REO_FLUSH_CACHE_RELEASE_CACHE_BLOCK_INDEX_LSB                               9
#define REO_FLUSH_CACHE_RELEASE_CACHE_BLOCK_INDEX_MSB                               9
#define REO_FLUSH_CACHE_RELEASE_CACHE_BLOCK_INDEX_MASK                              0x0000000000000200


/* Description		CACHE_BLOCK_RESOURCE_INDEX

			Field not valid when Flush_entire_cache is set.
			
			Indicates which of the four blocking resources in REO will
			 be assigned for managing the blocking of this (descriptor) 
			address 
			<legal all>
*/

#define REO_FLUSH_CACHE_CACHE_BLOCK_RESOURCE_INDEX_OFFSET                           0x0000000000000008
#define REO_FLUSH_CACHE_CACHE_BLOCK_RESOURCE_INDEX_LSB                              10
#define REO_FLUSH_CACHE_CACHE_BLOCK_RESOURCE_INDEX_MSB                              11
#define REO_FLUSH_CACHE_CACHE_BLOCK_RESOURCE_INDEX_MASK                             0x0000000000000c00


/* Description		FLUSH_WITHOUT_INVALIDATE

			Field not valid when Flush_entire_cache is set.
			
			When set, REO shall flush the cache line contents from the
			 cache, but there is NO need to invalidate the cache line
			 entry... The contents in the cache can be maintained. This
			 feature can be used by SW (and DV) to get a current snapshot
			 of the contents in the cache
			
			<legal all>
*/

#define REO_FLUSH_CACHE_FLUSH_WITHOUT_INVALIDATE_OFFSET                             0x0000000000000008
#define REO_FLUSH_CACHE_FLUSH_WITHOUT_INVALIDATE_LSB                                12
#define REO_FLUSH_CACHE_FLUSH_WITHOUT_INVALIDATE_MSB                                12
#define REO_FLUSH_CACHE_FLUSH_WITHOUT_INVALIDATE_MASK                               0x0000000000001000


/* Description		BLOCK_CACHE_USAGE_AFTER_FLUSH

			Field not valid when Flush_entire_cache is set.
			
			When set, REO shall block any cache accesses to this address
			 till explicitly unblocked. 
			
			Whenever SW sets this bit, SW shall also set bit 'Forward_all_mpdus_in_queue' 
			to ensure all packets are flushed out in order to make sure
			 this queue desc is not in one of the aging link lists. 
			In case SW does not want to flush the MPDUs in the queue, 
			see the recipe description below this TLV definition.
			
			The 'blocking' index to be used for this is indicated in
			 field 'cache_block_resource_index'. If SW had previously
			 used this blocking resource and was not freed up yet, SW
			 shall first unblock that index (by setting bit Release_cache_block_index) 
			or use an unblock command.
			
			If the resource indicated here was already blocked (and 
			did not get unblocked in this command), it is considered
			 an error scenario...
			No flush shall happen. The status for this command shall
			 indicate error.
			
			<legal all>
*/

#define REO_FLUSH_CACHE_BLOCK_CACHE_USAGE_AFTER_FLUSH_OFFSET                        0x0000000000000008
#define REO_FLUSH_CACHE_BLOCK_CACHE_USAGE_AFTER_FLUSH_LSB                           13
#define REO_FLUSH_CACHE_BLOCK_CACHE_USAGE_AFTER_FLUSH_MSB                           13
#define REO_FLUSH_CACHE_BLOCK_CACHE_USAGE_AFTER_FLUSH_MASK                          0x0000000000002000


/* Description		FLUSH_ENTIRE_CACHE

			When set, the entire cache shall be flushed. The entire 
			cache will also remain blocked, till the 'REO_UNBLOCK_COMMAND' 
			is received with bit unblock type set to unblock_cache. 
			All other fields in this command are to be ignored.
			
			Note that flushing the entire cache has no changes to the
			 current settings of the blocking resource settings
			
			<legal all>
*/

#define REO_FLUSH_CACHE_FLUSH_ENTIRE_CACHE_OFFSET                                   0x0000000000000008
#define REO_FLUSH_CACHE_FLUSH_ENTIRE_CACHE_LSB                                      14
#define REO_FLUSH_CACHE_FLUSH_ENTIRE_CACHE_MSB                                      14
#define REO_FLUSH_CACHE_FLUSH_ENTIRE_CACHE_MASK                                     0x0000000000004000


/* Description		FLUSH_QUEUE_1K_DESC

			When set, REO will flush the 'RX_REO_QUEUE_1K' descriptor
			 after flushing the 'RX_REO_QUEUE' descriptor.
			
			This bit shall only be set when the BA_window_size > 255
			 in 'RX_REO_QUEUE.'
			<legal all>
*/

#define REO_FLUSH_CACHE_FLUSH_QUEUE_1K_DESC_OFFSET                                  0x0000000000000008
#define REO_FLUSH_CACHE_FLUSH_QUEUE_1K_DESC_LSB                                     15
#define REO_FLUSH_CACHE_FLUSH_QUEUE_1K_DESC_MSB                                     15
#define REO_FLUSH_CACHE_FLUSH_QUEUE_1K_DESC_MASK                                    0x0000000000008000


/* Description		RESERVED_2B

			<legal 0>
*/

#define REO_FLUSH_CACHE_RESERVED_2B_OFFSET                                          0x0000000000000008
#define REO_FLUSH_CACHE_RESERVED_2B_LSB                                             16
#define REO_FLUSH_CACHE_RESERVED_2B_MSB                                             31
#define REO_FLUSH_CACHE_RESERVED_2B_MASK                                            0x00000000ffff0000


/* Description		RESERVED_3A

			<legal 0>
*/

#define REO_FLUSH_CACHE_RESERVED_3A_OFFSET                                          0x0000000000000008
#define REO_FLUSH_CACHE_RESERVED_3A_LSB                                             32
#define REO_FLUSH_CACHE_RESERVED_3A_MSB                                             63
#define REO_FLUSH_CACHE_RESERVED_3A_MASK                                            0xffffffff00000000


/* Description		RESERVED_4A

			<legal 0>
*/

#define REO_FLUSH_CACHE_RESERVED_4A_OFFSET                                          0x0000000000000010
#define REO_FLUSH_CACHE_RESERVED_4A_LSB                                             0
#define REO_FLUSH_CACHE_RESERVED_4A_MSB                                             31
#define REO_FLUSH_CACHE_RESERVED_4A_MASK                                            0x00000000ffffffff


/* Description		RESERVED_5A

			<legal 0>
*/

#define REO_FLUSH_CACHE_RESERVED_5A_OFFSET                                          0x0000000000000010
#define REO_FLUSH_CACHE_RESERVED_5A_LSB                                             32
#define REO_FLUSH_CACHE_RESERVED_5A_MSB                                             63
#define REO_FLUSH_CACHE_RESERVED_5A_MASK                                            0xffffffff00000000


/* Description		RESERVED_6A

			<legal 0>
*/

#define REO_FLUSH_CACHE_RESERVED_6A_OFFSET                                          0x0000000000000018
#define REO_FLUSH_CACHE_RESERVED_6A_LSB                                             0
#define REO_FLUSH_CACHE_RESERVED_6A_MSB                                             31
#define REO_FLUSH_CACHE_RESERVED_6A_MASK                                            0x00000000ffffffff


/* Description		RESERVED_7A

			<legal 0>
*/

#define REO_FLUSH_CACHE_RESERVED_7A_OFFSET                                          0x0000000000000018
#define REO_FLUSH_CACHE_RESERVED_7A_LSB                                             32
#define REO_FLUSH_CACHE_RESERVED_7A_MSB                                             63
#define REO_FLUSH_CACHE_RESERVED_7A_MASK                                            0xffffffff00000000


/* Description		RESERVED_8A

			<legal 0>
*/

#define REO_FLUSH_CACHE_RESERVED_8A_OFFSET                                          0x0000000000000020
#define REO_FLUSH_CACHE_RESERVED_8A_LSB                                             0
#define REO_FLUSH_CACHE_RESERVED_8A_MSB                                             31
#define REO_FLUSH_CACHE_RESERVED_8A_MASK                                            0x00000000ffffffff


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define REO_FLUSH_CACHE_TLV64_PADDING_OFFSET                                        0x0000000000000020
#define REO_FLUSH_CACHE_TLV64_PADDING_LSB                                           32
#define REO_FLUSH_CACHE_TLV64_PADDING_MSB                                           63
#define REO_FLUSH_CACHE_TLV64_PADDING_MASK                                          0xffffffff00000000



#endif   // REO_FLUSH_CACHE
