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


#ifndef _CE_STAT_DESC_H_
#define _CE_STAT_DESC_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_CE_STAT_DESC 4


struct ce_stat_desc {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ce_res_5                                                :  8, // [7:0]
                      toeplitz_en                                             :  1, // [8:8]
                      src_swap                                                :  1, // [9:9]
                      dest_swap                                               :  1, // [10:10]
                      gather                                                  :  1, // [11:11]
                      barrier_read                                            :  1, // [12:12]
                      ce_res_6                                                :  3, // [15:13]
                      length                                                  : 16; // [31:16]
             uint32_t toeplitz_hash_0                                         : 32; // [31:0]
             uint32_t toeplitz_hash_1                                         : 32; // [31:0]
             uint32_t fw_metadata                                             : 16, // [15:0]
                      ce_res_7                                                :  4, // [19:16]
                      ring_id                                                 :  8, // [27:20]
                      looping_count                                           :  4; // [31:28]
#else
             uint32_t length                                                  : 16, // [31:16]
                      ce_res_6                                                :  3, // [15:13]
                      barrier_read                                            :  1, // [12:12]
                      gather                                                  :  1, // [11:11]
                      dest_swap                                               :  1, // [10:10]
                      src_swap                                                :  1, // [9:9]
                      toeplitz_en                                             :  1, // [8:8]
                      ce_res_5                                                :  8; // [7:0]
             uint32_t toeplitz_hash_0                                         : 32; // [31:0]
             uint32_t toeplitz_hash_1                                         : 32; // [31:0]
             uint32_t looping_count                                           :  4, // [31:28]
                      ring_id                                                 :  8, // [27:20]
                      ce_res_7                                                :  4, // [19:16]
                      fw_metadata                                             : 16; // [15:0]
#endif
};


/* Description		CE_RES_5

			Reserved
			<legal all>
*/

#define CE_STAT_DESC_CE_RES_5_OFFSET                                                0x00000000
#define CE_STAT_DESC_CE_RES_5_LSB                                                   0
#define CE_STAT_DESC_CE_RES_5_MSB                                                   7
#define CE_STAT_DESC_CE_RES_5_MASK                                                  0x000000ff


/* Description		TOEPLITZ_EN

			32-bit Toeplitz-LFSR hash for the data transfer, Enabled
			
			<legal all>
*/

#define CE_STAT_DESC_TOEPLITZ_EN_OFFSET                                             0x00000000
#define CE_STAT_DESC_TOEPLITZ_EN_LSB                                                8
#define CE_STAT_DESC_TOEPLITZ_EN_MSB                                                8
#define CE_STAT_DESC_TOEPLITZ_EN_MASK                                               0x00000100


/* Description		SRC_SWAP

			Source memory buffer swapped
			<legal all>
*/

#define CE_STAT_DESC_SRC_SWAP_OFFSET                                                0x00000000
#define CE_STAT_DESC_SRC_SWAP_LSB                                                   9
#define CE_STAT_DESC_SRC_SWAP_MSB                                                   9
#define CE_STAT_DESC_SRC_SWAP_MASK                                                  0x00000200


/* Description		DEST_SWAP

			Destination  memory buffer swapped
			<legal all>
*/

#define CE_STAT_DESC_DEST_SWAP_OFFSET                                               0x00000000
#define CE_STAT_DESC_DEST_SWAP_LSB                                                  10
#define CE_STAT_DESC_DEST_SWAP_MSB                                                  10
#define CE_STAT_DESC_DEST_SWAP_MASK                                                 0x00000400


/* Description		GATHER

			Gather of multiple copy engine source descriptors to one
			 destination enabled
			<legal all>
*/

#define CE_STAT_DESC_GATHER_OFFSET                                                  0x00000000
#define CE_STAT_DESC_GATHER_LSB                                                     11
#define CE_STAT_DESC_GATHER_MSB                                                     11
#define CE_STAT_DESC_GATHER_MASK                                                    0x00000800


/* Description		BARRIER_READ

			Barrier read enabled
			<legal all>
*/

#define CE_STAT_DESC_BARRIER_READ_OFFSET                                            0x00000000
#define CE_STAT_DESC_BARRIER_READ_LSB                                               12
#define CE_STAT_DESC_BARRIER_READ_MSB                                               12
#define CE_STAT_DESC_BARRIER_READ_MASK                                              0x00001000


/* Description		CE_RES_6

			Reserved
			<legal all>
*/

#define CE_STAT_DESC_CE_RES_6_OFFSET                                                0x00000000
#define CE_STAT_DESC_CE_RES_6_LSB                                                   13
#define CE_STAT_DESC_CE_RES_6_MSB                                                   15
#define CE_STAT_DESC_CE_RES_6_MASK                                                  0x0000e000


/* Description		LENGTH

			Sum of all the Lengths of the source descriptor in the gather
			 chain
			<legal all>
*/

#define CE_STAT_DESC_LENGTH_OFFSET                                                  0x00000000
#define CE_STAT_DESC_LENGTH_LSB                                                     16
#define CE_STAT_DESC_LENGTH_MSB                                                     31
#define CE_STAT_DESC_LENGTH_MASK                                                    0xffff0000


/* Description		TOEPLITZ_HASH_0

			32 LS bits of 64 bit Toeplitz LFSR hash result
			<legal all>
*/

#define CE_STAT_DESC_TOEPLITZ_HASH_0_OFFSET                                         0x00000004
#define CE_STAT_DESC_TOEPLITZ_HASH_0_LSB                                            0
#define CE_STAT_DESC_TOEPLITZ_HASH_0_MSB                                            31
#define CE_STAT_DESC_TOEPLITZ_HASH_0_MASK                                           0xffffffff


/* Description		TOEPLITZ_HASH_1

			32 MS bits of 64 bit Toeplitz LFSR hash result
			<legal all>
*/

#define CE_STAT_DESC_TOEPLITZ_HASH_1_OFFSET                                         0x00000008
#define CE_STAT_DESC_TOEPLITZ_HASH_1_LSB                                            0
#define CE_STAT_DESC_TOEPLITZ_HASH_1_MSB                                            31
#define CE_STAT_DESC_TOEPLITZ_HASH_1_MASK                                           0xffffffff


/* Description		FW_METADATA

			Meta data used by FW
			In case of gather field in first source ring entry of the
			 gather copy cycle in taken into account.
			<legal all>
*/

#define CE_STAT_DESC_FW_METADATA_OFFSET                                             0x0000000c
#define CE_STAT_DESC_FW_METADATA_LSB                                                0
#define CE_STAT_DESC_FW_METADATA_MSB                                                15
#define CE_STAT_DESC_FW_METADATA_MASK                                               0x0000ffff


/* Description		CE_RES_7

			Reserved 
			<legal all>
*/

#define CE_STAT_DESC_CE_RES_7_OFFSET                                                0x0000000c
#define CE_STAT_DESC_CE_RES_7_LSB                                                   16
#define CE_STAT_DESC_CE_RES_7_MSB                                                   19
#define CE_STAT_DESC_CE_RES_7_MASK                                                  0x000f0000


/* Description		RING_ID

			The buffer pointer ring ID.
			0 refers to the IDLE ring
			1 - N refers to other rings
			
			Helps with debugging when dumping ring contents.
			<legal all>
*/

#define CE_STAT_DESC_RING_ID_OFFSET                                                 0x0000000c
#define CE_STAT_DESC_RING_ID_LSB                                                    20
#define CE_STAT_DESC_RING_ID_MSB                                                    27
#define CE_STAT_DESC_RING_ID_MASK                                                   0x0ff00000


/* Description		LOOPING_COUNT

			A count value that indicates the number of times the producer
			 of entries into the Ring has looped around the ring.
			At initialization time, this value is set to 0. On the first
			 loop, this value is set to 1. After the max value is reached
			 allowed by the number of bits for this field, the count
			 value continues with 0 again.
			In case SW is the consumer of the ring entries, it can use
			 this field to figure out up to where the producer of entries
			 has created new entries. This eliminates the need to check
			 where the "head pointer' of the ring is located once the
			 SW starts processing an interrupt indicating that new entries
			 have been put into this ring...
			Also note that SW if it wants only needs to look at the 
			LSB bit of this count value.
			<legal all>
*/

#define CE_STAT_DESC_LOOPING_COUNT_OFFSET                                           0x0000000c
#define CE_STAT_DESC_LOOPING_COUNT_LSB                                              28
#define CE_STAT_DESC_LOOPING_COUNT_MSB                                              31
#define CE_STAT_DESC_LOOPING_COUNT_MASK                                             0xf0000000



#endif   // CE_STAT_DESC
