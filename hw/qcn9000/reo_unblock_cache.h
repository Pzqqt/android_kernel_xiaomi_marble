/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	struct uniform_reo_cmd_header cmd_header;
//	1	unblock_type[0], cache_block_resource_index[2:1], reserved_1a[31:3]
//	2	reserved_2a[31:0]
//	3	reserved_3a[31:0]
//	4	reserved_4a[31:0]
//	5	reserved_5a[31:0]
//	6	reserved_6a[31:0]
//	7	reserved_7a[31:0]
//	8	reserved_8a[31:0]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_REO_UNBLOCK_CACHE 9

struct reo_unblock_cache {
    struct            uniform_reo_cmd_header                       cmd_header;
             uint32_t unblock_type                    :  1, //[0]
                      cache_block_resource_index      :  2, //[2:1]
                      reserved_1a                     : 29; //[31:3]
             uint32_t reserved_2a                     : 32; //[31:0]
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

unblock_type
			
			Unblock type
			
			
			
			<enum 0 unblock_resource_index> Unblock a block
			resource, whose index is given in field
			'cache_block_resource_index'.
			
			If the indicated blocking resource is not in use (=> not
			blocking an address at the moment), the command status will
			indicate an error.
			
			
			
			<enum 1 unblock_cache> The entire cache usage is
			unblocked. 
			
			If the entire cache is not in a blocked mode at the
			moment this command is received, the command status will
			indicate an error.
			
			Note that unlocking the entire cache has no changes to
			the current settings of the blocking resource settings
			
			
			
			<legal all>

cache_block_resource_index
			
			Field not valid when field Unblock_type is set to
			unblock_cache.
			
			
			
			Indicates which of the four blocking resources in REO
			should be released from blocking a (descriptor) address.
			
			<legal all>

reserved_1a
			
			<legal 0>

reserved_2a
			
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


 /* EXTERNAL REFERENCE : struct uniform_reo_cmd_header cmd_header */ 


/* Description		REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_CMD_NUMBER
			
			Consumer: REO/SW/DEBUG
			
			Producer: SW 
			
			
			
			This number can be used by SW to track, identify and
			link the created commands with the command statusses
			
			
			
			
			
			<legal all> 
*/
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_CMD_NUMBER_OFFSET         0x00000000
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_CMD_NUMBER_LSB            0
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_CMD_NUMBER_MASK           0x0000ffff

/* Description		REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_STATUS_REQUIRED
			
			Consumer: REO
			
			Producer: SW 
			
			
			
			<enum 0 NoStatus> REO does not need to generate a status
			TLV for the execution of this command
			
			<enum 1 StatusRequired> REO shall generate a status TLV
			for the execution of this command
			
			
			
			<legal all>
*/
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET    0x00000000
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_STATUS_REQUIRED_LSB       16
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_REO_STATUS_REQUIRED_MASK      0x00010000

/* Description		REO_UNBLOCK_CACHE_0_CMD_HEADER_RESERVED_0A
			
			<legal 0>
*/
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_RESERVED_0A_OFFSET            0x00000000
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_RESERVED_0A_LSB               17
#define REO_UNBLOCK_CACHE_0_CMD_HEADER_RESERVED_0A_MASK              0xfffe0000

/* Description		REO_UNBLOCK_CACHE_1_UNBLOCK_TYPE
			
			Unblock type
			
			
			
			<enum 0 unblock_resource_index> Unblock a block
			resource, whose index is given in field
			'cache_block_resource_index'.
			
			If the indicated blocking resource is not in use (=> not
			blocking an address at the moment), the command status will
			indicate an error.
			
			
			
			<enum 1 unblock_cache> The entire cache usage is
			unblocked. 
			
			If the entire cache is not in a blocked mode at the
			moment this command is received, the command status will
			indicate an error.
			
			Note that unlocking the entire cache has no changes to
			the current settings of the blocking resource settings
			
			
			
			<legal all>
*/
#define REO_UNBLOCK_CACHE_1_UNBLOCK_TYPE_OFFSET                      0x00000004
#define REO_UNBLOCK_CACHE_1_UNBLOCK_TYPE_LSB                         0
#define REO_UNBLOCK_CACHE_1_UNBLOCK_TYPE_MASK                        0x00000001

/* Description		REO_UNBLOCK_CACHE_1_CACHE_BLOCK_RESOURCE_INDEX
			
			Field not valid when field Unblock_type is set to
			unblock_cache.
			
			
			
			Indicates which of the four blocking resources in REO
			should be released from blocking a (descriptor) address.
			
			<legal all>
*/
#define REO_UNBLOCK_CACHE_1_CACHE_BLOCK_RESOURCE_INDEX_OFFSET        0x00000004
#define REO_UNBLOCK_CACHE_1_CACHE_BLOCK_RESOURCE_INDEX_LSB           1
#define REO_UNBLOCK_CACHE_1_CACHE_BLOCK_RESOURCE_INDEX_MASK          0x00000006

/* Description		REO_UNBLOCK_CACHE_1_RESERVED_1A
			
			<legal 0>
*/
#define REO_UNBLOCK_CACHE_1_RESERVED_1A_OFFSET                       0x00000004
#define REO_UNBLOCK_CACHE_1_RESERVED_1A_LSB                          3
#define REO_UNBLOCK_CACHE_1_RESERVED_1A_MASK                         0xfffffff8

/* Description		REO_UNBLOCK_CACHE_2_RESERVED_2A
			
			<legal 0>
*/
#define REO_UNBLOCK_CACHE_2_RESERVED_2A_OFFSET                       0x00000008
#define REO_UNBLOCK_CACHE_2_RESERVED_2A_LSB                          0
#define REO_UNBLOCK_CACHE_2_RESERVED_2A_MASK                         0xffffffff

/* Description		REO_UNBLOCK_CACHE_3_RESERVED_3A
			
			<legal 0>
*/
#define REO_UNBLOCK_CACHE_3_RESERVED_3A_OFFSET                       0x0000000c
#define REO_UNBLOCK_CACHE_3_RESERVED_3A_LSB                          0
#define REO_UNBLOCK_CACHE_3_RESERVED_3A_MASK                         0xffffffff

/* Description		REO_UNBLOCK_CACHE_4_RESERVED_4A
			
			<legal 0>
*/
#define REO_UNBLOCK_CACHE_4_RESERVED_4A_OFFSET                       0x00000010
#define REO_UNBLOCK_CACHE_4_RESERVED_4A_LSB                          0
#define REO_UNBLOCK_CACHE_4_RESERVED_4A_MASK                         0xffffffff

/* Description		REO_UNBLOCK_CACHE_5_RESERVED_5A
			
			<legal 0>
*/
#define REO_UNBLOCK_CACHE_5_RESERVED_5A_OFFSET                       0x00000014
#define REO_UNBLOCK_CACHE_5_RESERVED_5A_LSB                          0
#define REO_UNBLOCK_CACHE_5_RESERVED_5A_MASK                         0xffffffff

/* Description		REO_UNBLOCK_CACHE_6_RESERVED_6A
			
			<legal 0>
*/
#define REO_UNBLOCK_CACHE_6_RESERVED_6A_OFFSET                       0x00000018
#define REO_UNBLOCK_CACHE_6_RESERVED_6A_LSB                          0
#define REO_UNBLOCK_CACHE_6_RESERVED_6A_MASK                         0xffffffff

/* Description		REO_UNBLOCK_CACHE_7_RESERVED_7A
			
			<legal 0>
*/
#define REO_UNBLOCK_CACHE_7_RESERVED_7A_OFFSET                       0x0000001c
#define REO_UNBLOCK_CACHE_7_RESERVED_7A_LSB                          0
#define REO_UNBLOCK_CACHE_7_RESERVED_7A_MASK                         0xffffffff

/* Description		REO_UNBLOCK_CACHE_8_RESERVED_8A
			
			<legal 0>
*/
#define REO_UNBLOCK_CACHE_8_RESERVED_8A_OFFSET                       0x00000020
#define REO_UNBLOCK_CACHE_8_RESERVED_8A_LSB                          0
#define REO_UNBLOCK_CACHE_8_RESERVED_8A_MASK                         0xffffffff


#endif // _REO_UNBLOCK_CACHE_H_
