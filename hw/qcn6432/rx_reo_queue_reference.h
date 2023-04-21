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

#ifndef _RX_REO_QUEUE_REFERENCE_H_
#define _RX_REO_QUEUE_REFERENCE_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_REO_QUEUE_REFERENCE 2


struct rx_reo_queue_reference {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32; // [31:0]
             uint32_t rx_reo_queue_desc_addr_39_32                            :  8, // [7:0]
                      reserved_1                                              :  8, // [15:8]
                      receive_queue_number                                    : 16; // [31:16]
#else
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32; // [31:0]
             uint32_t receive_queue_number                                    : 16, // [31:16]
                      reserved_1                                              :  8, // [15:8]
                      rx_reo_queue_desc_addr_39_32                            :  8; // [7:0]
#endif
};


/* Description		RX_REO_QUEUE_DESC_ADDR_31_0

			Consumer: RXDMA
			Producer: RXOLE
			
			Address (lower 32 bits) of the REO queue descriptor. 
			<legal all>
*/

#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET                   0x00000000
#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_31_0_LSB                      0
#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_31_0_MSB                      31
#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_31_0_MASK                     0xffffffff


/* Description		RX_REO_QUEUE_DESC_ADDR_39_32

			Consumer: RXDMA
			Producer: RXOLE
			
			Address (upper 8 bits) of the REO queue descriptor. 
			<legal all>
*/

#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_39_32_OFFSET                  0x00000004
#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_39_32_LSB                     0
#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_39_32_MSB                     7
#define RX_REO_QUEUE_REFERENCE_RX_REO_QUEUE_DESC_ADDR_39_32_MASK                    0x000000ff


/* Description		RESERVED_1

			<legal 0>
*/

#define RX_REO_QUEUE_REFERENCE_RESERVED_1_OFFSET                                    0x00000004
#define RX_REO_QUEUE_REFERENCE_RESERVED_1_LSB                                       8
#define RX_REO_QUEUE_REFERENCE_RESERVED_1_MSB                                       15
#define RX_REO_QUEUE_REFERENCE_RESERVED_1_MASK                                      0x0000ff00


/* Description		RECEIVE_QUEUE_NUMBER

			Indicates the MPDU queue ID to which this MPDU link descriptor
			 belongs
			Used for tracking and debugging
			<legal all>
*/

#define RX_REO_QUEUE_REFERENCE_RECEIVE_QUEUE_NUMBER_OFFSET                          0x00000004
#define RX_REO_QUEUE_REFERENCE_RECEIVE_QUEUE_NUMBER_LSB                             16
#define RX_REO_QUEUE_REFERENCE_RECEIVE_QUEUE_NUMBER_MSB                             31
#define RX_REO_QUEUE_REFERENCE_RECEIVE_QUEUE_NUMBER_MASK                            0xffff0000



#endif   // RX_REO_QUEUE_REFERENCE
