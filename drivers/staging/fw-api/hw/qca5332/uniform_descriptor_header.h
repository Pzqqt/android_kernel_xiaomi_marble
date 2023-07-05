
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


#ifndef _UNIFORM_DESCRIPTOR_HEADER_H_
#define _UNIFORM_DESCRIPTOR_HEADER_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_UNIFORM_DESCRIPTOR_HEADER 1


struct uniform_descriptor_header {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t owner                                                   :  4, // [3:0]
                      buffer_type                                             :  4, // [7:4]
                      tx_mpdu_queue_number                                    : 20, // [27:8]
                      reserved_0a                                             :  4; // [31:28]
#else
             uint32_t reserved_0a                                             :  4, // [31:28]
                      tx_mpdu_queue_number                                    : 20, // [27:8]
                      buffer_type                                             :  4, // [7:4]
                      owner                                                   :  4; // [3:0]
#endif
};


/* Description		OWNER

			Consumer: In DEBUG mode: WBM, TQM, TXDMA, RXDMA, REO
			Producer: In DEBUG mode: WBM, TQM, TXDMA, RXDMA, REO 
			
			The owner of this data structure:
			<enum 0 WBM_owned> Buffer Manager currently owns this data
			 structure.
			<enum 1 SW_OR_FW_owned> Software of FW currently owns this
			 data structure.
			<enum 2 TQM_owned> Transmit Queue Manager currently owns
			 this data structure.
			<enum 3 RXDMA_owned> Receive DMA currently owns this data
			 structure.
			<enum 4 REO_owned> Reorder currently owns this data structure.
			
			<enum 5 SWITCH_owned> SWITCH currently owns this data structure.
			
			
			<legal 0-5> 
*/

#define UNIFORM_DESCRIPTOR_HEADER_OWNER_OFFSET                                      0x00000000
#define UNIFORM_DESCRIPTOR_HEADER_OWNER_LSB                                         0
#define UNIFORM_DESCRIPTOR_HEADER_OWNER_MSB                                         3
#define UNIFORM_DESCRIPTOR_HEADER_OWNER_MASK                                        0x0000000f


/* Description		BUFFER_TYPE

			Consumer: In DEBUG mode: WBM, TQM, TXDMA, RXDMA, REO
			Producer: In DEBUG mode: WBM, TQM, TXDMA, RXDMA, REO 
			
			Field describing what contents format is of this descriptor
			
			
			<enum 0 Transmit_MSDU_Link_descriptor> 
			<enum 1 Transmit_MPDU_Link_descriptor> 
			<enum 2 Transmit_MPDU_Queue_head_descriptor>
			<enum 3 Transmit_MPDU_Queue_ext_descriptor>
			<enum 4 Transmit_flow_descriptor>
			<enum 5 Transmit_buffer> NOT TO BE USED: 
			
			<enum 6 Receive_MSDU_Link_descriptor>
			<enum 7 Receive_MPDU_Link_descriptor>
			<enum 8 Receive_REO_queue_descriptor>
			<enum 9 Receive_REO_queue_1k_descriptor>
			<enum 10 Receive_REO_queue_ext_descriptor>
			
			<enum 11 Receive_buffer>
			
			<enum 12 Idle_link_list_entry>
			
			<legal 0-12> 
*/

#define UNIFORM_DESCRIPTOR_HEADER_BUFFER_TYPE_OFFSET                                0x00000000
#define UNIFORM_DESCRIPTOR_HEADER_BUFFER_TYPE_LSB                                   4
#define UNIFORM_DESCRIPTOR_HEADER_BUFFER_TYPE_MSB                                   7
#define UNIFORM_DESCRIPTOR_HEADER_BUFFER_TYPE_MASK                                  0x000000f0


/* Description		TX_MPDU_QUEUE_NUMBER

			Consumer: TQM/Debug
			Producer: SW (in 'TX_MPDU_QUEUE_HEAD')/TQM (elsewhere)
			
			Field only valid if Buffer_type is any of Transmit_MPDU_*_descriptor
			
			
			Indicates the MPDU queue ID to which this MPDU descriptor
			 belongs
			Used for tracking and debugging
			
			Hamilton and Waikiki used bits [19:0] of word 1 of 'TX_MPDU_LINK,' 
			word 16 of 'TX_MPDU_QUEUE_HEAD' and word 1 of 'TX_MPDU_QUEUE_EXT' 
			for this.
			 <legal all>
*/

#define UNIFORM_DESCRIPTOR_HEADER_TX_MPDU_QUEUE_NUMBER_OFFSET                       0x00000000
#define UNIFORM_DESCRIPTOR_HEADER_TX_MPDU_QUEUE_NUMBER_LSB                          8
#define UNIFORM_DESCRIPTOR_HEADER_TX_MPDU_QUEUE_NUMBER_MSB                          27
#define UNIFORM_DESCRIPTOR_HEADER_TX_MPDU_QUEUE_NUMBER_MASK                         0x0fffff00


/* Description		RESERVED_0A

			<legal 0>
*/

#define UNIFORM_DESCRIPTOR_HEADER_RESERVED_0A_OFFSET                                0x00000000
#define UNIFORM_DESCRIPTOR_HEADER_RESERVED_0A_LSB                                   28
#define UNIFORM_DESCRIPTOR_HEADER_RESERVED_0A_MSB                                   31
#define UNIFORM_DESCRIPTOR_HEADER_RESERVED_0A_MASK                                  0xf0000000



#endif   // UNIFORM_DESCRIPTOR_HEADER
