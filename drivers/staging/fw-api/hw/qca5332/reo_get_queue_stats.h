
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

 
 
 
 
 
 
 


#ifndef _REO_GET_QUEUE_STATS_H_
#define _REO_GET_QUEUE_STATS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_cmd_header.h"
#define NUM_OF_DWORDS_REO_GET_QUEUE_STATS 10

#define NUM_OF_QWORDS_REO_GET_QUEUE_STATS 5


struct reo_get_queue_stats {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32; // [31:0]
             uint32_t rx_reo_queue_desc_addr_39_32                            :  8, // [7:0]
                      clear_stats                                             :  1, // [8:8]
                      reserved_2a                                             : 23; // [31:9]
             uint32_t reserved_3a                                             : 32; // [31:0]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 32; // [31:0]
             uint32_t reserved_8a                                             : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32; // [31:0]
             uint32_t reserved_2a                                             : 23, // [31:9]
                      clear_stats                                             :  1, // [8:8]
                      rx_reo_queue_desc_addr_39_32                            :  8; // [7:0]
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

#define REO_GET_QUEUE_STATS_CMD_HEADER_REO_CMD_NUMBER_OFFSET                        0x0000000000000000
#define REO_GET_QUEUE_STATS_CMD_HEADER_REO_CMD_NUMBER_LSB                           0
#define REO_GET_QUEUE_STATS_CMD_HEADER_REO_CMD_NUMBER_MSB                           15
#define REO_GET_QUEUE_STATS_CMD_HEADER_REO_CMD_NUMBER_MASK                          0x000000000000ffff


/* Description		REO_STATUS_REQUIRED

			Consumer: REO
			Producer: SW 
			
			<enum 0 NoStatus> REO does not need to generate a status
			 TLV for the execution of this command
			<enum 1 StatusRequired> REO shall generate a status TLV 
			for the execution of this command
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET                   0x0000000000000000
#define REO_GET_QUEUE_STATS_CMD_HEADER_REO_STATUS_REQUIRED_LSB                      16
#define REO_GET_QUEUE_STATS_CMD_HEADER_REO_STATUS_REQUIRED_MSB                      16
#define REO_GET_QUEUE_STATS_CMD_HEADER_REO_STATUS_REQUIRED_MASK                     0x0000000000010000


/* Description		RESERVED_0A

			<legal 0>
*/

#define REO_GET_QUEUE_STATS_CMD_HEADER_RESERVED_0A_OFFSET                           0x0000000000000000
#define REO_GET_QUEUE_STATS_CMD_HEADER_RESERVED_0A_LSB                              17
#define REO_GET_QUEUE_STATS_CMD_HEADER_RESERVED_0A_MSB                              31
#define REO_GET_QUEUE_STATS_CMD_HEADER_RESERVED_0A_MASK                             0x00000000fffe0000


/* Description		RX_REO_QUEUE_DESC_ADDR_31_0

			Consumer: REO
			Producer: SW
			
			Address (lower 32 bits) of the REO queue descriptor
			<legal all>
*/

#define REO_GET_QUEUE_STATS_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET                      0x0000000000000000
#define REO_GET_QUEUE_STATS_RX_REO_QUEUE_DESC_ADDR_31_0_LSB                         32
#define REO_GET_QUEUE_STATS_RX_REO_QUEUE_DESC_ADDR_31_0_MSB                         63
#define REO_GET_QUEUE_STATS_RX_REO_QUEUE_DESC_ADDR_31_0_MASK                        0xffffffff00000000


/* Description		RX_REO_QUEUE_DESC_ADDR_39_32

			Consumer: REO
			Producer: SW
			
			Address (upper 8 bits) of the REO queue descriptor
			<legal all>
*/

#define REO_GET_QUEUE_STATS_RX_REO_QUEUE_DESC_ADDR_39_32_OFFSET                     0x0000000000000008
#define REO_GET_QUEUE_STATS_RX_REO_QUEUE_DESC_ADDR_39_32_LSB                        0
#define REO_GET_QUEUE_STATS_RX_REO_QUEUE_DESC_ADDR_39_32_MSB                        7
#define REO_GET_QUEUE_STATS_RX_REO_QUEUE_DESC_ADDR_39_32_MASK                       0x00000000000000ff


/* Description		CLEAR_STATS

			Clear stat settings....
			
			<enum 0 no_clear> Do NOT clear the stats after generating
			 the status
			<enum 1 clear_the_stats> Clear the stats after generating
			 the status. 
			
			The stats actually cleared are:
			Timeout_count
			Forward_due_to_bar_count
			Duplicate_count
			Frames_in_order_count
			BAR_received_count
			MPDU_Frames_processed_count
			MSDU_Frames_processed_count
			Total_processed_byte_count
			Late_receive_MPDU_count
			window_jump_2k
			Hole_count
			<legal 0-1>
*/

#define REO_GET_QUEUE_STATS_CLEAR_STATS_OFFSET                                      0x0000000000000008
#define REO_GET_QUEUE_STATS_CLEAR_STATS_LSB                                         8
#define REO_GET_QUEUE_STATS_CLEAR_STATS_MSB                                         8
#define REO_GET_QUEUE_STATS_CLEAR_STATS_MASK                                        0x0000000000000100


/* Description		RESERVED_2A

			<legal 0>
*/

#define REO_GET_QUEUE_STATS_RESERVED_2A_OFFSET                                      0x0000000000000008
#define REO_GET_QUEUE_STATS_RESERVED_2A_LSB                                         9
#define REO_GET_QUEUE_STATS_RESERVED_2A_MSB                                         31
#define REO_GET_QUEUE_STATS_RESERVED_2A_MASK                                        0x00000000fffffe00


/* Description		RESERVED_3A

			<legal 0>
*/

#define REO_GET_QUEUE_STATS_RESERVED_3A_OFFSET                                      0x0000000000000008
#define REO_GET_QUEUE_STATS_RESERVED_3A_LSB                                         32
#define REO_GET_QUEUE_STATS_RESERVED_3A_MSB                                         63
#define REO_GET_QUEUE_STATS_RESERVED_3A_MASK                                        0xffffffff00000000


/* Description		RESERVED_4A

			<legal 0>
*/

#define REO_GET_QUEUE_STATS_RESERVED_4A_OFFSET                                      0x0000000000000010
#define REO_GET_QUEUE_STATS_RESERVED_4A_LSB                                         0
#define REO_GET_QUEUE_STATS_RESERVED_4A_MSB                                         31
#define REO_GET_QUEUE_STATS_RESERVED_4A_MASK                                        0x00000000ffffffff


/* Description		RESERVED_5A

			<legal 0>
*/

#define REO_GET_QUEUE_STATS_RESERVED_5A_OFFSET                                      0x0000000000000010
#define REO_GET_QUEUE_STATS_RESERVED_5A_LSB                                         32
#define REO_GET_QUEUE_STATS_RESERVED_5A_MSB                                         63
#define REO_GET_QUEUE_STATS_RESERVED_5A_MASK                                        0xffffffff00000000


/* Description		RESERVED_6A

			<legal 0>
*/

#define REO_GET_QUEUE_STATS_RESERVED_6A_OFFSET                                      0x0000000000000018
#define REO_GET_QUEUE_STATS_RESERVED_6A_LSB                                         0
#define REO_GET_QUEUE_STATS_RESERVED_6A_MSB                                         31
#define REO_GET_QUEUE_STATS_RESERVED_6A_MASK                                        0x00000000ffffffff


/* Description		RESERVED_7A

			<legal 0>
*/

#define REO_GET_QUEUE_STATS_RESERVED_7A_OFFSET                                      0x0000000000000018
#define REO_GET_QUEUE_STATS_RESERVED_7A_LSB                                         32
#define REO_GET_QUEUE_STATS_RESERVED_7A_MSB                                         63
#define REO_GET_QUEUE_STATS_RESERVED_7A_MASK                                        0xffffffff00000000


/* Description		RESERVED_8A

			<legal 0>
*/

#define REO_GET_QUEUE_STATS_RESERVED_8A_OFFSET                                      0x0000000000000020
#define REO_GET_QUEUE_STATS_RESERVED_8A_LSB                                         0
#define REO_GET_QUEUE_STATS_RESERVED_8A_MSB                                         31
#define REO_GET_QUEUE_STATS_RESERVED_8A_MASK                                        0x00000000ffffffff


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define REO_GET_QUEUE_STATS_TLV64_PADDING_OFFSET                                    0x0000000000000020
#define REO_GET_QUEUE_STATS_TLV64_PADDING_LSB                                       32
#define REO_GET_QUEUE_STATS_TLV64_PADDING_MSB                                       63
#define REO_GET_QUEUE_STATS_TLV64_PADDING_MASK                                      0xffffffff00000000



#endif   // REO_GET_QUEUE_STATS
