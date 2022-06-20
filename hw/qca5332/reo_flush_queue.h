
/* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_cmd_header.h"
#define NUM_OF_DWORDS_REO_FLUSH_QUEUE 10

#define NUM_OF_QWORDS_REO_FLUSH_QUEUE 5


struct reo_flush_queue {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t flush_desc_addr_31_0                                    : 32; // [31:0]
             uint32_t flush_desc_addr_39_32                                   :  8, // [7:0]
                      block_desc_addr_usage_after_flush                       :  1, // [8:8]
                      block_resource_index                                    :  2, // [10:9]
                      reserved_2a                                             : 21; // [31:11]
             uint32_t reserved_3a                                             : 32; // [31:0]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 32; // [31:0]
             uint32_t reserved_8a                                             : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t flush_desc_addr_31_0                                    : 32; // [31:0]
             uint32_t reserved_2a                                             : 21, // [31:11]
                      block_resource_index                                    :  2, // [10:9]
                      block_desc_addr_usage_after_flush                       :  1, // [8:8]
                      flush_desc_addr_39_32                                   :  8; // [7:0]
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

#define REO_FLUSH_QUEUE_CMD_HEADER_REO_CMD_NUMBER_OFFSET                            0x0000000000000000
#define REO_FLUSH_QUEUE_CMD_HEADER_REO_CMD_NUMBER_LSB                               0
#define REO_FLUSH_QUEUE_CMD_HEADER_REO_CMD_NUMBER_MSB                               15
#define REO_FLUSH_QUEUE_CMD_HEADER_REO_CMD_NUMBER_MASK                              0x000000000000ffff


/* Description		REO_STATUS_REQUIRED

			Consumer: REO
			Producer: SW 
			
			<enum 0 NoStatus> REO does not need to generate a status
			 TLV for the execution of this command
			<enum 1 StatusRequired> REO shall generate a status TLV 
			for the execution of this command
			
			<legal all>
*/

#define REO_FLUSH_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET                       0x0000000000000000
#define REO_FLUSH_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_LSB                          16
#define REO_FLUSH_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_MSB                          16
#define REO_FLUSH_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_MASK                         0x0000000000010000


/* Description		RESERVED_0A

			<legal 0>
*/

#define REO_FLUSH_QUEUE_CMD_HEADER_RESERVED_0A_OFFSET                               0x0000000000000000
#define REO_FLUSH_QUEUE_CMD_HEADER_RESERVED_0A_LSB                                  17
#define REO_FLUSH_QUEUE_CMD_HEADER_RESERVED_0A_MSB                                  31
#define REO_FLUSH_QUEUE_CMD_HEADER_RESERVED_0A_MASK                                 0x00000000fffe0000


/* Description		FLUSH_DESC_ADDR_31_0

			Consumer: REO
			Producer: SW
			
			Address (lower 32 bits) of the descriptor to flush
			<legal all>
*/

#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_31_0_OFFSET                                 0x0000000000000000
#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_31_0_LSB                                    32
#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_31_0_MSB                                    63
#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_31_0_MASK                                   0xffffffff00000000


/* Description		FLUSH_DESC_ADDR_39_32

			Consumer: REO
			Producer: SW
			
			Address (upper 8 bits) of the descriptor to flush
			<legal all>
*/

#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_39_32_OFFSET                                0x0000000000000008
#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_39_32_LSB                                   0
#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_39_32_MSB                                   7
#define REO_FLUSH_QUEUE_FLUSH_DESC_ADDR_39_32_MASK                                  0x00000000000000ff


/* Description		BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH

			When set, REO shall not re-fetch this address till SW explicitly
			 unblocked this address
			
			If the blocking resource was already used, this command 
			shall fail and an error is reported
			
			<legal all>
*/

#define REO_FLUSH_QUEUE_BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH_OFFSET                    0x0000000000000008
#define REO_FLUSH_QUEUE_BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH_LSB                       8
#define REO_FLUSH_QUEUE_BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH_MSB                       8
#define REO_FLUSH_QUEUE_BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH_MASK                      0x0000000000000100


/* Description		BLOCK_RESOURCE_INDEX

			Field only valid when 'Block_desc_addr_usage_after_flush
			 ' is set.
			
			Indicates which of the four blocking resources in REO will
			 be assigned for managing the blocking of this address.
			<legal all>
*/

#define REO_FLUSH_QUEUE_BLOCK_RESOURCE_INDEX_OFFSET                                 0x0000000000000008
#define REO_FLUSH_QUEUE_BLOCK_RESOURCE_INDEX_LSB                                    9
#define REO_FLUSH_QUEUE_BLOCK_RESOURCE_INDEX_MSB                                    10
#define REO_FLUSH_QUEUE_BLOCK_RESOURCE_INDEX_MASK                                   0x0000000000000600


/* Description		RESERVED_2A

			<legal 0>
*/

#define REO_FLUSH_QUEUE_RESERVED_2A_OFFSET                                          0x0000000000000008
#define REO_FLUSH_QUEUE_RESERVED_2A_LSB                                             11
#define REO_FLUSH_QUEUE_RESERVED_2A_MSB                                             31
#define REO_FLUSH_QUEUE_RESERVED_2A_MASK                                            0x00000000fffff800


/* Description		RESERVED_3A

			<legal 0>
*/

#define REO_FLUSH_QUEUE_RESERVED_3A_OFFSET                                          0x0000000000000008
#define REO_FLUSH_QUEUE_RESERVED_3A_LSB                                             32
#define REO_FLUSH_QUEUE_RESERVED_3A_MSB                                             63
#define REO_FLUSH_QUEUE_RESERVED_3A_MASK                                            0xffffffff00000000


/* Description		RESERVED_4A

			<legal 0>
*/

#define REO_FLUSH_QUEUE_RESERVED_4A_OFFSET                                          0x0000000000000010
#define REO_FLUSH_QUEUE_RESERVED_4A_LSB                                             0
#define REO_FLUSH_QUEUE_RESERVED_4A_MSB                                             31
#define REO_FLUSH_QUEUE_RESERVED_4A_MASK                                            0x00000000ffffffff


/* Description		RESERVED_5A

			<legal 0>
*/

#define REO_FLUSH_QUEUE_RESERVED_5A_OFFSET                                          0x0000000000000010
#define REO_FLUSH_QUEUE_RESERVED_5A_LSB                                             32
#define REO_FLUSH_QUEUE_RESERVED_5A_MSB                                             63
#define REO_FLUSH_QUEUE_RESERVED_5A_MASK                                            0xffffffff00000000


/* Description		RESERVED_6A

			<legal 0>
*/

#define REO_FLUSH_QUEUE_RESERVED_6A_OFFSET                                          0x0000000000000018
#define REO_FLUSH_QUEUE_RESERVED_6A_LSB                                             0
#define REO_FLUSH_QUEUE_RESERVED_6A_MSB                                             31
#define REO_FLUSH_QUEUE_RESERVED_6A_MASK                                            0x00000000ffffffff


/* Description		RESERVED_7A

			<legal 0>
*/

#define REO_FLUSH_QUEUE_RESERVED_7A_OFFSET                                          0x0000000000000018
#define REO_FLUSH_QUEUE_RESERVED_7A_LSB                                             32
#define REO_FLUSH_QUEUE_RESERVED_7A_MSB                                             63
#define REO_FLUSH_QUEUE_RESERVED_7A_MASK                                            0xffffffff00000000


/* Description		RESERVED_8A

			<legal 0>
*/

#define REO_FLUSH_QUEUE_RESERVED_8A_OFFSET                                          0x0000000000000020
#define REO_FLUSH_QUEUE_RESERVED_8A_LSB                                             0
#define REO_FLUSH_QUEUE_RESERVED_8A_MSB                                             31
#define REO_FLUSH_QUEUE_RESERVED_8A_MASK                                            0x00000000ffffffff


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define REO_FLUSH_QUEUE_TLV64_PADDING_OFFSET                                        0x0000000000000020
#define REO_FLUSH_QUEUE_TLV64_PADDING_LSB                                           32
#define REO_FLUSH_QUEUE_TLV64_PADDING_MSB                                           63
#define REO_FLUSH_QUEUE_TLV64_PADDING_MASK                                          0xffffffff00000000



#endif   // REO_FLUSH_QUEUE
