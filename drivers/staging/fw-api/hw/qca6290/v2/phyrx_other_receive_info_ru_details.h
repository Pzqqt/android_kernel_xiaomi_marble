/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#ifndef _PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_H_
#define _PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	ru_details_channel_0[31:0]
//	1	ru_details_channel_1[31:0]
//	2	spare[31:0]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS 3

struct phyrx_other_receive_info_ru_details {
             uint32_t ru_details_channel_0            : 32; //[31:0]
             uint32_t ru_details_channel_1            : 32; //[31:0]
             uint32_t spare                           : 32; //[31:0]
};

/*

ru_details_channel_0
			
			Ru_allocation from content channel 0
			
			[7:0] for 20/40 MHz
			
			[15:0] for 80 MHz
			
			[31:0] for 160 MHz
			
			<legal all>

ru_details_channel_1
			
			Ru_allocation from content channel 1
			
			[7:0] for 40 MHz
			
			[15:0] for 80 MHz
			
			[31:0] for 160 MHz
			
			<legal all>

spare
			
			Extra spare bits added to convey additional information
			
			<legal all>
*/


/* Description		PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_0_RU_DETAILS_CHANNEL_0
			
			Ru_allocation from content channel 0
			
			[7:0] for 20/40 MHz
			
			[15:0] for 80 MHz
			
			[31:0] for 160 MHz
			
			<legal all>
*/
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_0_RU_DETAILS_CHANNEL_0_OFFSET 0x00000000
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_0_RU_DETAILS_CHANNEL_0_LSB 0
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_0_RU_DETAILS_CHANNEL_0_MASK 0xffffffff

/* Description		PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_1_RU_DETAILS_CHANNEL_1
			
			Ru_allocation from content channel 1
			
			[7:0] for 40 MHz
			
			[15:0] for 80 MHz
			
			[31:0] for 160 MHz
			
			<legal all>
*/
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_1_RU_DETAILS_CHANNEL_1_OFFSET 0x00000004
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_1_RU_DETAILS_CHANNEL_1_LSB 0
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_1_RU_DETAILS_CHANNEL_1_MASK 0xffffffff

/* Description		PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_2_SPARE
			
			Extra spare bits added to convey additional information
			
			<legal all>
*/
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_2_SPARE_OFFSET           0x00000008
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_2_SPARE_LSB              0
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_2_SPARE_MASK             0xffffffff


#endif // _PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_H_
