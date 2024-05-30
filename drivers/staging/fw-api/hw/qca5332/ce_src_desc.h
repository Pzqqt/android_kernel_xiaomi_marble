
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

 
 
 
 
 
 
 


#ifndef _CE_SRC_DESC_H_
#define _CE_SRC_DESC_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_CE_SRC_DESC 4


struct ce_src_desc {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t src_buffer_low                                          : 32; // [31:0]
             uint32_t src_buffer_high                                         :  8, // [7:0]
                      toeplitz_en                                             :  1, // [8:8]
                      src_swap                                                :  1, // [9:9]
                      dest_swap                                               :  1, // [10:10]
                      gather                                                  :  1, // [11:11]
                      ce_res_0                                                :  1, // [12:12]
                      barrier_read                                            :  1, // [13:13]
                      ce_res_1                                                :  2, // [15:14]
                      length                                                  : 16; // [31:16]
             uint32_t fw_metadata                                             : 16, // [15:0]
                      ce_res_2                                                : 16; // [31:16]
             uint32_t ce_res_3                                                : 20, // [19:0]
                      ring_id                                                 :  8, // [27:20]
                      looping_count                                           :  4; // [31:28]
#else
             uint32_t src_buffer_low                                          : 32; // [31:0]
             uint32_t length                                                  : 16, // [31:16]
                      ce_res_1                                                :  2, // [15:14]
                      barrier_read                                            :  1, // [13:13]
                      ce_res_0                                                :  1, // [12:12]
                      gather                                                  :  1, // [11:11]
                      dest_swap                                               :  1, // [10:10]
                      src_swap                                                :  1, // [9:9]
                      toeplitz_en                                             :  1, // [8:8]
                      src_buffer_high                                         :  8; // [7:0]
             uint32_t ce_res_2                                                : 16, // [31:16]
                      fw_metadata                                             : 16; // [15:0]
             uint32_t looping_count                                           :  4, // [31:28]
                      ring_id                                                 :  8, // [27:20]
                      ce_res_3                                                : 20; // [19:0]
#endif
};


/* Description		SRC_BUFFER_LOW

			LSB 32 bits of the 40 Bit Pointer to the source buffer
			<legal all>
*/

#define CE_SRC_DESC_SRC_BUFFER_LOW_OFFSET                                           0x00000000
#define CE_SRC_DESC_SRC_BUFFER_LOW_LSB                                              0
#define CE_SRC_DESC_SRC_BUFFER_LOW_MSB                                              31
#define CE_SRC_DESC_SRC_BUFFER_LOW_MASK                                             0xffffffff


/* Description		SRC_BUFFER_HIGH

			Bits [6:0] for 40 Bit Pointer to the source buffer
			Bit [7] can be programmed with VC bit. 
			Note: CE Descriptor has 40-bit address. Only 37 bits are
			 routed as address to NoC. Remaining bits are user bits. 
			Bit [7] of SRC_BUFFER_HIGH can be used for VC configuration. 
			0 indicate VC0 and 1 indicate VC1.
			<legal all>
*/

#define CE_SRC_DESC_SRC_BUFFER_HIGH_OFFSET                                          0x00000004
#define CE_SRC_DESC_SRC_BUFFER_HIGH_LSB                                             0
#define CE_SRC_DESC_SRC_BUFFER_HIGH_MSB                                             7
#define CE_SRC_DESC_SRC_BUFFER_HIGH_MASK                                            0x000000ff


/* Description		TOEPLITZ_EN

			Enable generation of 32-bit Toeplitz-LFSR hash for the data
			 transfer
			In case of gather field in first source ring entry of the
			 gather copy cycle in taken into account.
			<legal all>
*/

#define CE_SRC_DESC_TOEPLITZ_EN_OFFSET                                              0x00000004
#define CE_SRC_DESC_TOEPLITZ_EN_LSB                                                 8
#define CE_SRC_DESC_TOEPLITZ_EN_MSB                                                 8
#define CE_SRC_DESC_TOEPLITZ_EN_MASK                                                0x00000100


/* Description		SRC_SWAP

			Treats source memory organization as big-endian. For each
			 dword read (4 bytes), the byte 0 is swapped with byte 3
			 and byte 1 is swapped with byte 2.
			In case of gather field in first source ring entry of the
			 gather copy cycle in taken into account.
			<legal all>
*/

#define CE_SRC_DESC_SRC_SWAP_OFFSET                                                 0x00000004
#define CE_SRC_DESC_SRC_SWAP_LSB                                                    9
#define CE_SRC_DESC_SRC_SWAP_MSB                                                    9
#define CE_SRC_DESC_SRC_SWAP_MASK                                                   0x00000200


/* Description		DEST_SWAP

			Treats destination memory organization as big-endian. For
			 each dword write (4 bytes), the byte 0 is swapped with 
			byte 3 and byte 1 is swapped with byte 2.
			In case of gather field in first source ring entry of the
			 gather copy cycle in taken into account.
			<legal all>
*/

#define CE_SRC_DESC_DEST_SWAP_OFFSET                                                0x00000004
#define CE_SRC_DESC_DEST_SWAP_LSB                                                   10
#define CE_SRC_DESC_DEST_SWAP_MSB                                                   10
#define CE_SRC_DESC_DEST_SWAP_MASK                                                  0x00000400


/* Description		GATHER

			Enables gather of multiple copy engine source descriptors
			 to one destination.
			<legal all>
*/

#define CE_SRC_DESC_GATHER_OFFSET                                                   0x00000004
#define CE_SRC_DESC_GATHER_LSB                                                      11
#define CE_SRC_DESC_GATHER_MSB                                                      11
#define CE_SRC_DESC_GATHER_MASK                                                     0x00000800


/* Description		CE_RES_0

			Reserved
			<legal all>
*/

#define CE_SRC_DESC_CE_RES_0_OFFSET                                                 0x00000004
#define CE_SRC_DESC_CE_RES_0_LSB                                                    12
#define CE_SRC_DESC_CE_RES_0_MSB                                                    12
#define CE_SRC_DESC_CE_RES_0_MASK                                                   0x00001000


/* Description		BARRIER_READ

			Barrier Read enable
			<legal all>
*/

#define CE_SRC_DESC_BARRIER_READ_OFFSET                                             0x00000004
#define CE_SRC_DESC_BARRIER_READ_LSB                                                13
#define CE_SRC_DESC_BARRIER_READ_MSB                                                13
#define CE_SRC_DESC_BARRIER_READ_MASK                                               0x00002000


/* Description		CE_RES_1

			Reserved
			<legal all>
*/

#define CE_SRC_DESC_CE_RES_1_OFFSET                                                 0x00000004
#define CE_SRC_DESC_CE_RES_1_LSB                                                    14
#define CE_SRC_DESC_CE_RES_1_MSB                                                    15
#define CE_SRC_DESC_CE_RES_1_MASK                                                   0x0000c000


/* Description		LENGTH

			Length of the buffer in units of octets of the current descriptor
			
			<legal all>
*/

#define CE_SRC_DESC_LENGTH_OFFSET                                                   0x00000004
#define CE_SRC_DESC_LENGTH_LSB                                                      16
#define CE_SRC_DESC_LENGTH_MSB                                                      31
#define CE_SRC_DESC_LENGTH_MASK                                                     0xffff0000


/* Description		FW_METADATA

			Meta data used by FW
			In case of gather field in first source ring entry of the
			 gather copy cycle in taken into account.
			<legal all>
*/

#define CE_SRC_DESC_FW_METADATA_OFFSET                                              0x00000008
#define CE_SRC_DESC_FW_METADATA_LSB                                                 0
#define CE_SRC_DESC_FW_METADATA_MSB                                                 15
#define CE_SRC_DESC_FW_METADATA_MASK                                                0x0000ffff


/* Description		CE_RES_2

			Reserved
			<legal all>
*/

#define CE_SRC_DESC_CE_RES_2_OFFSET                                                 0x00000008
#define CE_SRC_DESC_CE_RES_2_LSB                                                    16
#define CE_SRC_DESC_CE_RES_2_MSB                                                    31
#define CE_SRC_DESC_CE_RES_2_MASK                                                   0xffff0000


/* Description		CE_RES_3

			Reserved 
			<legal all>
*/

#define CE_SRC_DESC_CE_RES_3_OFFSET                                                 0x0000000c
#define CE_SRC_DESC_CE_RES_3_LSB                                                    0
#define CE_SRC_DESC_CE_RES_3_MSB                                                    19
#define CE_SRC_DESC_CE_RES_3_MASK                                                   0x000fffff


/* Description		RING_ID

			The buffer pointer ring ID.
			0 refers to the IDLE ring
			1 - N refers to other rings
			
			Helps with debugging when dumping ring contents.
			<legal all>
*/

#define CE_SRC_DESC_RING_ID_OFFSET                                                  0x0000000c
#define CE_SRC_DESC_RING_ID_LSB                                                     20
#define CE_SRC_DESC_RING_ID_MSB                                                     27
#define CE_SRC_DESC_RING_ID_MASK                                                    0x0ff00000


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

#define CE_SRC_DESC_LOOPING_COUNT_OFFSET                                            0x0000000c
#define CE_SRC_DESC_LOOPING_COUNT_LSB                                               28
#define CE_SRC_DESC_LOOPING_COUNT_MSB                                               31
#define CE_SRC_DESC_LOOPING_COUNT_MASK                                              0xf0000000



#endif   // CE_SRC_DESC
