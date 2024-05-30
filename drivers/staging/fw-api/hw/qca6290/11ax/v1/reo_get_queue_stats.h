/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#ifndef _REO_GET_QUEUE_STATS_H_
#define _REO_GET_QUEUE_STATS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_cmd_header.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	struct uniform_reo_cmd_header cmd_header;
//	1	rx_reo_queue_desc_addr_31_0[31:0]
//	2	rx_reo_queue_desc_addr_39_32[7:0], clear_stats[8], reserved_2a[31:9]
//	3	reserved_3a[31:0]
//	4	reserved_4a[31:0]
//	5	reserved_5a[31:0]
//	6	reserved_6a[31:0]
//	7	reserved_7a[31:0]
//	8	reserved_8a[31:0]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_REO_GET_QUEUE_STATS 9

struct reo_get_queue_stats {
    struct            uniform_reo_cmd_header                       cmd_header;
             uint32_t rx_reo_queue_desc_addr_31_0     : 32; //[31:0]
             uint32_t rx_reo_queue_desc_addr_39_32    :  8, //[7:0]
                      clear_stats                     :  1, //[8]
                      reserved_2a                     : 23; //[31:9]
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

rx_reo_queue_desc_addr_31_0
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Address (lower 32 bits) of the REO queue descriptor
			
			<legal all>

rx_reo_queue_desc_addr_39_32
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Address (upper 8 bits) of the REO queue descriptor
			
			<legal all>

clear_stats
			
			Clear stat settings....
			
			
			
			<enum 0 no_clear> Do NOT clear the stats after
			generating the status
			
			<enum 1 clear_the_stats> Clear the stats after
			generating the status. 
			
			
			
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

#define REO_GET_QUEUE_STATS_0_UNIFORM_REO_CMD_HEADER_CMD_HEADER_OFFSET 0x00000000
#define REO_GET_QUEUE_STATS_0_UNIFORM_REO_CMD_HEADER_CMD_HEADER_LSB  0
#define REO_GET_QUEUE_STATS_0_UNIFORM_REO_CMD_HEADER_CMD_HEADER_MASK 0xffffffff

/* Description		REO_GET_QUEUE_STATS_1_RX_REO_QUEUE_DESC_ADDR_31_0
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Address (lower 32 bits) of the REO queue descriptor
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_1_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET     0x00000004
#define REO_GET_QUEUE_STATS_1_RX_REO_QUEUE_DESC_ADDR_31_0_LSB        0
#define REO_GET_QUEUE_STATS_1_RX_REO_QUEUE_DESC_ADDR_31_0_MASK       0xffffffff

/* Description		REO_GET_QUEUE_STATS_2_RX_REO_QUEUE_DESC_ADDR_39_32
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Address (upper 8 bits) of the REO queue descriptor
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_2_RX_REO_QUEUE_DESC_ADDR_39_32_OFFSET    0x00000008
#define REO_GET_QUEUE_STATS_2_RX_REO_QUEUE_DESC_ADDR_39_32_LSB       0
#define REO_GET_QUEUE_STATS_2_RX_REO_QUEUE_DESC_ADDR_39_32_MASK      0x000000ff

/* Description		REO_GET_QUEUE_STATS_2_CLEAR_STATS
			
			Clear stat settings....
			
			
			
			<enum 0 no_clear> Do NOT clear the stats after
			generating the status
			
			<enum 1 clear_the_stats> Clear the stats after
			generating the status. 
			
			
			
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
#define REO_GET_QUEUE_STATS_2_CLEAR_STATS_OFFSET                     0x00000008
#define REO_GET_QUEUE_STATS_2_CLEAR_STATS_LSB                        8
#define REO_GET_QUEUE_STATS_2_CLEAR_STATS_MASK                       0x00000100

/* Description		REO_GET_QUEUE_STATS_2_RESERVED_2A
			
			<legal 0>
*/
#define REO_GET_QUEUE_STATS_2_RESERVED_2A_OFFSET                     0x00000008
#define REO_GET_QUEUE_STATS_2_RESERVED_2A_LSB                        9
#define REO_GET_QUEUE_STATS_2_RESERVED_2A_MASK                       0xfffffe00

/* Description		REO_GET_QUEUE_STATS_3_RESERVED_3A
			
			<legal 0>
*/
#define REO_GET_QUEUE_STATS_3_RESERVED_3A_OFFSET                     0x0000000c
#define REO_GET_QUEUE_STATS_3_RESERVED_3A_LSB                        0
#define REO_GET_QUEUE_STATS_3_RESERVED_3A_MASK                       0xffffffff

/* Description		REO_GET_QUEUE_STATS_4_RESERVED_4A
			
			<legal 0>
*/
#define REO_GET_QUEUE_STATS_4_RESERVED_4A_OFFSET                     0x00000010
#define REO_GET_QUEUE_STATS_4_RESERVED_4A_LSB                        0
#define REO_GET_QUEUE_STATS_4_RESERVED_4A_MASK                       0xffffffff

/* Description		REO_GET_QUEUE_STATS_5_RESERVED_5A
			
			<legal 0>
*/
#define REO_GET_QUEUE_STATS_5_RESERVED_5A_OFFSET                     0x00000014
#define REO_GET_QUEUE_STATS_5_RESERVED_5A_LSB                        0
#define REO_GET_QUEUE_STATS_5_RESERVED_5A_MASK                       0xffffffff

/* Description		REO_GET_QUEUE_STATS_6_RESERVED_6A
			
			<legal 0>
*/
#define REO_GET_QUEUE_STATS_6_RESERVED_6A_OFFSET                     0x00000018
#define REO_GET_QUEUE_STATS_6_RESERVED_6A_LSB                        0
#define REO_GET_QUEUE_STATS_6_RESERVED_6A_MASK                       0xffffffff

/* Description		REO_GET_QUEUE_STATS_7_RESERVED_7A
			
			<legal 0>
*/
#define REO_GET_QUEUE_STATS_7_RESERVED_7A_OFFSET                     0x0000001c
#define REO_GET_QUEUE_STATS_7_RESERVED_7A_LSB                        0
#define REO_GET_QUEUE_STATS_7_RESERVED_7A_MASK                       0xffffffff

/* Description		REO_GET_QUEUE_STATS_8_RESERVED_8A
			
			<legal 0>
*/
#define REO_GET_QUEUE_STATS_8_RESERVED_8A_OFFSET                     0x00000020
#define REO_GET_QUEUE_STATS_8_RESERVED_8A_LSB                        0
#define REO_GET_QUEUE_STATS_8_RESERVED_8A_MASK                       0xffffffff


#endif // _REO_GET_QUEUE_STATS_H_
