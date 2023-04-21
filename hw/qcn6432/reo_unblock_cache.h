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

#ifndef _REO_UNBLOCK_CACHE_H_
#define _REO_UNBLOCK_CACHE_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_cmd_header.h"
#define NUM_OF_DWORDS_REO_UNBLOCK_CACHE 10

#define NUM_OF_QWORDS_REO_UNBLOCK_CACHE 5


struct reo_unblock_cache {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t unblock_type                                            :  1, // [0:0]
                      cache_block_resource_index                              :  2, // [2:1]
                      reserved_1a                                             : 29; // [31:3]
             uint32_t reserved_2a                                             : 32; // [31:0]
             uint32_t reserved_3a                                             : 32; // [31:0]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 32; // [31:0]
             uint32_t reserved_8a                                             : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t reserved_1a                                             : 29, // [31:3]
                      cache_block_resource_index                              :  2, // [2:1]
                      unblock_type                                            :  1; // [0:0]
             uint32_t reserved_2a                                             : 32; // [31:0]
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

#define REO_UNBLOCK_CACHE_CMD_HEADER_REO_CMD_NUMBER_OFFSET                          0x0000000000000000
#define REO_UNBLOCK_CACHE_CMD_HEADER_REO_CMD_NUMBER_LSB                             0
#define REO_UNBLOCK_CACHE_CMD_HEADER_REO_CMD_NUMBER_MSB                             15
#define REO_UNBLOCK_CACHE_CMD_HEADER_REO_CMD_NUMBER_MASK                            0x000000000000ffff


/* Description		REO_STATUS_REQUIRED

			Consumer: REO
			Producer: SW 
			
			<enum 0 NoStatus> REO does not need to generate a status
			 TLV for the execution of this command
			<enum 1 StatusRequired> REO shall generate a status TLV 
			for the execution of this command
			
			<legal all>
*/

#define REO_UNBLOCK_CACHE_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET                     0x0000000000000000
#define REO_UNBLOCK_CACHE_CMD_HEADER_REO_STATUS_REQUIRED_LSB                        16
#define REO_UNBLOCK_CACHE_CMD_HEADER_REO_STATUS_REQUIRED_MSB                        16
#define REO_UNBLOCK_CACHE_CMD_HEADER_REO_STATUS_REQUIRED_MASK                       0x0000000000010000


/* Description		RESERVED_0A

			<legal 0>
*/

#define REO_UNBLOCK_CACHE_CMD_HEADER_RESERVED_0A_OFFSET                             0x0000000000000000
#define REO_UNBLOCK_CACHE_CMD_HEADER_RESERVED_0A_LSB                                17
#define REO_UNBLOCK_CACHE_CMD_HEADER_RESERVED_0A_MSB                                31
#define REO_UNBLOCK_CACHE_CMD_HEADER_RESERVED_0A_MASK                               0x00000000fffe0000


/* Description		UNBLOCK_TYPE

			Unblock type
			
			<enum 0 unblock_resource_index> Unblock a block resource, 
			whose index is given in field 'cache_block_resource_index'.
			
			If the indicated blocking resource is not in use (=> not
			 blocking an address at the moment), the command status 
			will indicate an error.
			
			<enum 1 unblock_cache> The entire cache usage is unblocked. 
			
			If the entire cache is not in a blocked mode at the moment
			 this command is received, the command status will indicate
			 an error.
			Note that unlocking the "entire cache" has no changes to
			 the current settings of the blocking resource settings
			
			<legal all>
*/

#define REO_UNBLOCK_CACHE_UNBLOCK_TYPE_OFFSET                                       0x0000000000000000
#define REO_UNBLOCK_CACHE_UNBLOCK_TYPE_LSB                                          32
#define REO_UNBLOCK_CACHE_UNBLOCK_TYPE_MSB                                          32
#define REO_UNBLOCK_CACHE_UNBLOCK_TYPE_MASK                                         0x0000000100000000


/* Description		CACHE_BLOCK_RESOURCE_INDEX

			Field not valid when field Unblock_type is set to unblock_cache.
			
			
			Indicates which of the four blocking resources in REO should
			 be released from blocking a (descriptor) address.
			<legal all>
*/

#define REO_UNBLOCK_CACHE_CACHE_BLOCK_RESOURCE_INDEX_OFFSET                         0x0000000000000000
#define REO_UNBLOCK_CACHE_CACHE_BLOCK_RESOURCE_INDEX_LSB                            33
#define REO_UNBLOCK_CACHE_CACHE_BLOCK_RESOURCE_INDEX_MSB                            34
#define REO_UNBLOCK_CACHE_CACHE_BLOCK_RESOURCE_INDEX_MASK                           0x0000000600000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define REO_UNBLOCK_CACHE_RESERVED_1A_OFFSET                                        0x0000000000000000
#define REO_UNBLOCK_CACHE_RESERVED_1A_LSB                                           35
#define REO_UNBLOCK_CACHE_RESERVED_1A_MSB                                           63
#define REO_UNBLOCK_CACHE_RESERVED_1A_MASK                                          0xfffffff800000000


/* Description		RESERVED_2A

			<legal 0>
*/

#define REO_UNBLOCK_CACHE_RESERVED_2A_OFFSET                                        0x0000000000000008
#define REO_UNBLOCK_CACHE_RESERVED_2A_LSB                                           0
#define REO_UNBLOCK_CACHE_RESERVED_2A_MSB                                           31
#define REO_UNBLOCK_CACHE_RESERVED_2A_MASK                                          0x00000000ffffffff


/* Description		RESERVED_3A

			<legal 0>
*/

#define REO_UNBLOCK_CACHE_RESERVED_3A_OFFSET                                        0x0000000000000008
#define REO_UNBLOCK_CACHE_RESERVED_3A_LSB                                           32
#define REO_UNBLOCK_CACHE_RESERVED_3A_MSB                                           63
#define REO_UNBLOCK_CACHE_RESERVED_3A_MASK                                          0xffffffff00000000


/* Description		RESERVED_4A

			<legal 0>
*/

#define REO_UNBLOCK_CACHE_RESERVED_4A_OFFSET                                        0x0000000000000010
#define REO_UNBLOCK_CACHE_RESERVED_4A_LSB                                           0
#define REO_UNBLOCK_CACHE_RESERVED_4A_MSB                                           31
#define REO_UNBLOCK_CACHE_RESERVED_4A_MASK                                          0x00000000ffffffff


/* Description		RESERVED_5A

			<legal 0>
*/

#define REO_UNBLOCK_CACHE_RESERVED_5A_OFFSET                                        0x0000000000000010
#define REO_UNBLOCK_CACHE_RESERVED_5A_LSB                                           32
#define REO_UNBLOCK_CACHE_RESERVED_5A_MSB                                           63
#define REO_UNBLOCK_CACHE_RESERVED_5A_MASK                                          0xffffffff00000000


/* Description		RESERVED_6A

			<legal 0>
*/

#define REO_UNBLOCK_CACHE_RESERVED_6A_OFFSET                                        0x0000000000000018
#define REO_UNBLOCK_CACHE_RESERVED_6A_LSB                                           0
#define REO_UNBLOCK_CACHE_RESERVED_6A_MSB                                           31
#define REO_UNBLOCK_CACHE_RESERVED_6A_MASK                                          0x00000000ffffffff


/* Description		RESERVED_7A

			<legal 0>
*/

#define REO_UNBLOCK_CACHE_RESERVED_7A_OFFSET                                        0x0000000000000018
#define REO_UNBLOCK_CACHE_RESERVED_7A_LSB                                           32
#define REO_UNBLOCK_CACHE_RESERVED_7A_MSB                                           63
#define REO_UNBLOCK_CACHE_RESERVED_7A_MASK                                          0xffffffff00000000


/* Description		RESERVED_8A

			<legal 0>
*/

#define REO_UNBLOCK_CACHE_RESERVED_8A_OFFSET                                        0x0000000000000020
#define REO_UNBLOCK_CACHE_RESERVED_8A_LSB                                           0
#define REO_UNBLOCK_CACHE_RESERVED_8A_MSB                                           31
#define REO_UNBLOCK_CACHE_RESERVED_8A_MASK                                          0x00000000ffffffff


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define REO_UNBLOCK_CACHE_TLV64_PADDING_OFFSET                                      0x0000000000000020
#define REO_UNBLOCK_CACHE_TLV64_PADDING_LSB                                         32
#define REO_UNBLOCK_CACHE_TLV64_PADDING_MSB                                         63
#define REO_UNBLOCK_CACHE_TLV64_PADDING_MASK                                        0xffffffff00000000



#endif   // REO_UNBLOCK_CACHE
