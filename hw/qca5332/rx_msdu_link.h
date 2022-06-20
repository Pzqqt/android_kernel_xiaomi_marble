
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

 
 
 
 
 
 
 


#ifndef _RX_MSDU_LINK_H_
#define _RX_MSDU_LINK_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_descriptor_header.h"
#include "buffer_addr_info.h"
#include "rx_msdu_details.h"
#define NUM_OF_DWORDS_RX_MSDU_LINK 32


struct rx_msdu_link {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_descriptor_header                                 descriptor_header;
             struct   buffer_addr_info                                          next_msdu_link_desc_addr_info;
             uint32_t receive_queue_number                                    : 16, // [15:0]
                      first_rx_msdu_link_struct                               :  1, // [16:16]
                      reserved_3a                                             : 15; // [31:17]
             uint32_t pn_31_0                                                 : 32; // [31:0]
             uint32_t pn_63_32                                                : 32; // [31:0]
             uint32_t pn_95_64                                                : 32; // [31:0]
             uint32_t pn_127_96                                               : 32; // [31:0]
             struct   rx_msdu_details                                           msdu_0;
             struct   rx_msdu_details                                           msdu_1;
             struct   rx_msdu_details                                           msdu_2;
             struct   rx_msdu_details                                           msdu_3;
             struct   rx_msdu_details                                           msdu_4;
             struct   rx_msdu_details                                           msdu_5;
#else
             struct   uniform_descriptor_header                                 descriptor_header;
             struct   buffer_addr_info                                          next_msdu_link_desc_addr_info;
             uint32_t reserved_3a                                             : 15, // [31:17]
                      first_rx_msdu_link_struct                               :  1, // [16:16]
                      receive_queue_number                                    : 16; // [15:0]
             uint32_t pn_31_0                                                 : 32; // [31:0]
             uint32_t pn_63_32                                                : 32; // [31:0]
             uint32_t pn_95_64                                                : 32; // [31:0]
             uint32_t pn_127_96                                               : 32; // [31:0]
             struct   rx_msdu_details                                           msdu_0;
             struct   rx_msdu_details                                           msdu_1;
             struct   rx_msdu_details                                           msdu_2;
             struct   rx_msdu_details                                           msdu_3;
             struct   rx_msdu_details                                           msdu_4;
             struct   rx_msdu_details                                           msdu_5;
#endif
};


/* Description		DESCRIPTOR_HEADER

			Details about which module owns this struct.
			Note that sub field "Buffer_type" shall be set to "Receive_MSDU_Link_descriptor"
			
*/


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

#define RX_MSDU_LINK_DESCRIPTOR_HEADER_OWNER_OFFSET                                 0x00000000
#define RX_MSDU_LINK_DESCRIPTOR_HEADER_OWNER_LSB                                    0
#define RX_MSDU_LINK_DESCRIPTOR_HEADER_OWNER_MSB                                    3
#define RX_MSDU_LINK_DESCRIPTOR_HEADER_OWNER_MASK                                   0x0000000f


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

#define RX_MSDU_LINK_DESCRIPTOR_HEADER_BUFFER_TYPE_OFFSET                           0x00000000
#define RX_MSDU_LINK_DESCRIPTOR_HEADER_BUFFER_TYPE_LSB                              4
#define RX_MSDU_LINK_DESCRIPTOR_HEADER_BUFFER_TYPE_MSB                              7
#define RX_MSDU_LINK_DESCRIPTOR_HEADER_BUFFER_TYPE_MASK                             0x000000f0


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_MSDU_LINK_DESCRIPTOR_HEADER_RESERVED_0A_OFFSET                           0x00000000
#define RX_MSDU_LINK_DESCRIPTOR_HEADER_RESERVED_0A_LSB                              8
#define RX_MSDU_LINK_DESCRIPTOR_HEADER_RESERVED_0A_MSB                              31
#define RX_MSDU_LINK_DESCRIPTOR_HEADER_RESERVED_0A_MASK                             0xffffff00


/* Description		NEXT_MSDU_LINK_DESC_ADDR_INFO

			Details of the physical address of the next MSDU link descriptor
			 that contains info about additional MSDUs that are part
			 of this MPDU.
*/


/* Description		BUFFER_ADDR_31_0

			Address (lower 32 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET          0x00000004
#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB             0
#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MSB             31
#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK            0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET         0x00000008
#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB            0
#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MSB            7
#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK           0x000000ff


/* Description		RETURN_BUFFER_MANAGER

			Consumer: WBM
			Producer: SW/FW
			
			In case of 'NULL' pointer, this field is set to 0
			
			Indicates to which buffer manager the buffer OR MSDU_EXTENSION
			 descriptor OR link descriptor that is being pointed to 
			shall be returned after the frame has been processed. It
			 is used by WBM for routing purposes.
			
			<enum 0 WBM_IDLE_BUF_LIST> This buffer shall be returned
			 to the WMB buffer idle list
			<enum 1 WBM_CHIP0_IDLE_DESC_LIST> This buffer shall be returned
			 to the WBM idle link descriptor idle list, where the chip
			 0 WBM is chosen in case of a multi-chip config
			<enum 2 WBM_CHIP1_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 1 WBM idle link descriptor idle list
			<enum 3 WBM_CHIP2_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 2 WBM idle link descriptor idle list
			<enum 12 WBM_CHIP3_IDLE_DESC_LIST> This buffer shall be 
			returned to chip 3 WBM idle link descriptor idle list
			<enum 4 FW_BM> This buffer shall be returned to the FW
			<enum 5 SW0_BM> This buffer shall be returned to the SW, 
			ring 0
			<enum 6 SW1_BM> This buffer shall be returned to the SW, 
			ring 1
			<enum 7 SW2_BM> This buffer shall be returned to the SW, 
			ring 2
			<enum 8 SW3_BM> This buffer shall be returned to the SW, 
			ring 3
			<enum 9 SW4_BM> This buffer shall be returned to the SW, 
			ring 4
			<enum 10 SW5_BM> This buffer shall be returned to the SW, 
			ring 5
			<enum 11 SW6_BM> This buffer shall be returned to the SW, 
			ring 6
			
			<legal 0-12>
*/

#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET     0x00000008
#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB        8
#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB        11
#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK       0x00000f00


/* Description		SW_BUFFER_COOKIE

			Cookie field exclusively used by SW. 
			
			In case of 'NULL' pointer, this field is set to 0
			
			HW ignores the contents, accept that it passes the programmed
			 value on to other descriptors together with the physical
			 address 
			
			Field can be used by SW to for example associate the buffers
			 physical address with the virtual address
			The bit definitions as used by SW are within SW HLD specification
			
			
			NOTE1:
			The three most significant bits can have a special meaning
			 in case this struct is embedded in a TX_MPDU_DETAILS STRUCT, 
			and field transmit_bw_restriction is set
			
			In case of NON punctured transmission:
			Sw_buffer_cookie[19:17] = 3'b000: 20 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b001: 40 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b010: 80 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b011: 160 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b101: 240 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b100: 320 MHz TX only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			In case of punctured transmission:
			Sw_buffer_cookie[19:16] = 4'b0000: pattern 0 only
			Sw_buffer_cookie[19:16] = 4'b0001: pattern 1 only
			Sw_buffer_cookie[19:16] = 4'b0010: pattern 2 only
			Sw_buffer_cookie[19:16] = 4'b0011: pattern 3 only
			Sw_buffer_cookie[19:16] = 4'b0100: pattern 4 only
			Sw_buffer_cookie[19:16] = 4'b0101: pattern 5 only
			Sw_buffer_cookie[19:16] = 4'b0110: pattern 6 only
			Sw_buffer_cookie[19:16] = 4'b0111: pattern 7 only
			Sw_buffer_cookie[19:16] = 4'b1000: pattern 8 only
			Sw_buffer_cookie[19:16] = 4'b1001: pattern 9 only
			Sw_buffer_cookie[19:16] = 4'b1010: pattern 10 only
			Sw_buffer_cookie[19:16] = 4'b1011: pattern 11 only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			Note: a punctured transmission is indicated by the presence
			 of TLV TX_PUNCTURE_SETUP embedded in the scheduler TLV
			
			<legal all>
*/

#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET          0x00000008
#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB             12
#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MSB             31
#define RX_MSDU_LINK_NEXT_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK            0xfffff000


/* Description		RECEIVE_QUEUE_NUMBER

			Indicates the Receive queue to which this MPDU descriptor
			 belongs
			Used for tracking, finding bugs and debugging.
			<legal all>
*/

#define RX_MSDU_LINK_RECEIVE_QUEUE_NUMBER_OFFSET                                    0x0000000c
#define RX_MSDU_LINK_RECEIVE_QUEUE_NUMBER_LSB                                       0
#define RX_MSDU_LINK_RECEIVE_QUEUE_NUMBER_MSB                                       15
#define RX_MSDU_LINK_RECEIVE_QUEUE_NUMBER_MASK                                      0x0000ffff


/* Description		FIRST_RX_MSDU_LINK_STRUCT

			When set, this RX_MSDU_link descriptor is the first one 
			in the MSDU link list. Field MSDU_0 points to the very first
			 MSDU buffer descriptor in the MPDU
			<legal all>
*/

#define RX_MSDU_LINK_FIRST_RX_MSDU_LINK_STRUCT_OFFSET                               0x0000000c
#define RX_MSDU_LINK_FIRST_RX_MSDU_LINK_STRUCT_LSB                                  16
#define RX_MSDU_LINK_FIRST_RX_MSDU_LINK_STRUCT_MSB                                  16
#define RX_MSDU_LINK_FIRST_RX_MSDU_LINK_STRUCT_MASK                                 0x00010000


/* Description		RESERVED_3A

			<legal 0>
*/

#define RX_MSDU_LINK_RESERVED_3A_OFFSET                                             0x0000000c
#define RX_MSDU_LINK_RESERVED_3A_LSB                                                17
#define RX_MSDU_LINK_RESERVED_3A_MSB                                                31
#define RX_MSDU_LINK_RESERVED_3A_MASK                                               0xfffe0000


/* Description		PN_31_0

			Field only valid when First_RX_MSDU_link_struct  is set.
			
			
			31-0 bits of the 256-bit packet number bitmap.  
			<legal all>
*/

#define RX_MSDU_LINK_PN_31_0_OFFSET                                                 0x00000010
#define RX_MSDU_LINK_PN_31_0_LSB                                                    0
#define RX_MSDU_LINK_PN_31_0_MSB                                                    31
#define RX_MSDU_LINK_PN_31_0_MASK                                                   0xffffffff


/* Description		PN_63_32

			Field only valid when First_RX_MSDU_link_struct  is set.
			
			
			63-32 bits of the 256-bit packet number bitmap.  
			<legal all>
*/

#define RX_MSDU_LINK_PN_63_32_OFFSET                                                0x00000014
#define RX_MSDU_LINK_PN_63_32_LSB                                                   0
#define RX_MSDU_LINK_PN_63_32_MSB                                                   31
#define RX_MSDU_LINK_PN_63_32_MASK                                                  0xffffffff


/* Description		PN_95_64

			Field only valid when First_RX_MSDU_link_struct  is set.
			
			
			95-64 bits of the 256-bit packet number bitmap. 
			<legal all>
*/

#define RX_MSDU_LINK_PN_95_64_OFFSET                                                0x00000018
#define RX_MSDU_LINK_PN_95_64_LSB                                                   0
#define RX_MSDU_LINK_PN_95_64_MSB                                                   31
#define RX_MSDU_LINK_PN_95_64_MASK                                                  0xffffffff


/* Description		PN_127_96

			Field only valid when First_RX_MSDU_link_struct  is set.
			
			
			127-96 bits of the 256-bit packet number bitmap. 
			<legal all>
*/

#define RX_MSDU_LINK_PN_127_96_OFFSET                                               0x0000001c
#define RX_MSDU_LINK_PN_127_96_LSB                                                  0
#define RX_MSDU_LINK_PN_127_96_MSB                                                  31
#define RX_MSDU_LINK_PN_127_96_MASK                                                 0xffffffff


/* Description		MSDU_0

			When First_RX_MSDU_link_struct  is set, this MSDU is the
			 first in the MPDU
			
			When First_RX_MSDU_link_struct  is NOT set, this MSDU follows
			 the last MSDU in the previous RX_MSDU_link data structure
			
*/


/* Description		BUFFER_ADDR_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			Details of the physical address of the buffer containing
			 an MSDU (or entire MPDU)
*/


/* Description		BUFFER_ADDR_31_0

			Address (lower 32 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_OFFSET        0x00000020
#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_LSB           0
#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MSB           31
#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MASK          0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_OFFSET       0x00000024
#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_LSB          0
#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MSB          7
#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MASK         0x000000ff


/* Description		RETURN_BUFFER_MANAGER

			Consumer: WBM
			Producer: SW/FW
			
			In case of 'NULL' pointer, this field is set to 0
			
			Indicates to which buffer manager the buffer OR MSDU_EXTENSION
			 descriptor OR link descriptor that is being pointed to 
			shall be returned after the frame has been processed. It
			 is used by WBM for routing purposes.
			
			<enum 0 WBM_IDLE_BUF_LIST> This buffer shall be returned
			 to the WMB buffer idle list
			<enum 1 WBM_CHIP0_IDLE_DESC_LIST> This buffer shall be returned
			 to the WBM idle link descriptor idle list, where the chip
			 0 WBM is chosen in case of a multi-chip config
			<enum 2 WBM_CHIP1_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 1 WBM idle link descriptor idle list
			<enum 3 WBM_CHIP2_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 2 WBM idle link descriptor idle list
			<enum 12 WBM_CHIP3_IDLE_DESC_LIST> This buffer shall be 
			returned to chip 3 WBM idle link descriptor idle list
			<enum 4 FW_BM> This buffer shall be returned to the FW
			<enum 5 SW0_BM> This buffer shall be returned to the SW, 
			ring 0
			<enum 6 SW1_BM> This buffer shall be returned to the SW, 
			ring 1
			<enum 7 SW2_BM> This buffer shall be returned to the SW, 
			ring 2
			<enum 8 SW3_BM> This buffer shall be returned to the SW, 
			ring 3
			<enum 9 SW4_BM> This buffer shall be returned to the SW, 
			ring 4
			<enum 10 SW5_BM> This buffer shall be returned to the SW, 
			ring 5
			<enum 11 SW6_BM> This buffer shall be returned to the SW, 
			ring 6
			
			<legal 0-12>
*/

#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_OFFSET   0x00000024
#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_LSB      8
#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MSB      11
#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MASK     0x00000f00


/* Description		SW_BUFFER_COOKIE

			Cookie field exclusively used by SW. 
			
			In case of 'NULL' pointer, this field is set to 0
			
			HW ignores the contents, accept that it passes the programmed
			 value on to other descriptors together with the physical
			 address 
			
			Field can be used by SW to for example associate the buffers
			 physical address with the virtual address
			The bit definitions as used by SW are within SW HLD specification
			
			
			NOTE1:
			The three most significant bits can have a special meaning
			 in case this struct is embedded in a TX_MPDU_DETAILS STRUCT, 
			and field transmit_bw_restriction is set
			
			In case of NON punctured transmission:
			Sw_buffer_cookie[19:17] = 3'b000: 20 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b001: 40 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b010: 80 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b011: 160 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b101: 240 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b100: 320 MHz TX only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			In case of punctured transmission:
			Sw_buffer_cookie[19:16] = 4'b0000: pattern 0 only
			Sw_buffer_cookie[19:16] = 4'b0001: pattern 1 only
			Sw_buffer_cookie[19:16] = 4'b0010: pattern 2 only
			Sw_buffer_cookie[19:16] = 4'b0011: pattern 3 only
			Sw_buffer_cookie[19:16] = 4'b0100: pattern 4 only
			Sw_buffer_cookie[19:16] = 4'b0101: pattern 5 only
			Sw_buffer_cookie[19:16] = 4'b0110: pattern 6 only
			Sw_buffer_cookie[19:16] = 4'b0111: pattern 7 only
			Sw_buffer_cookie[19:16] = 4'b1000: pattern 8 only
			Sw_buffer_cookie[19:16] = 4'b1001: pattern 9 only
			Sw_buffer_cookie[19:16] = 4'b1010: pattern 10 only
			Sw_buffer_cookie[19:16] = 4'b1011: pattern 11 only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			Note: a punctured transmission is indicated by the presence
			 of TLV TX_PUNCTURE_SETUP embedded in the scheduler TLV
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_OFFSET        0x00000024
#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_LSB           12
#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MSB           31
#define RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MASK          0xfffff000


/* Description		RX_MSDU_DESC_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			General information related to the MSDU that should be passed
			 on from RXDMA all the way to to the REO destination ring.
			
*/


/* Description		FIRST_MSDU_IN_MPDU_FLAG

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU 
			
			<enum 0 Not_first_msdu> This is not the first MSDU in the
			 MPDU. 
			<enum 1 first_msdu> This MSDU is the first one in the MPDU.
			
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_LSB   0
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MSB   0
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MASK  0x00000001


/* Description		LAST_MSDU_IN_MPDU_FLAG

			Consumer: WBM/REO/SW/FW
			Producer: RXDMA
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			
			<enum 0 Not_last_msdu> There are more MSDUs linked to this
			 MSDU that belongs to this MPDU 
			<enum 1 Last_msdu> this MSDU is the last one in the MPDU. 
			This setting is only allowed in combination with 'Msdu_continuation' 
			set to 0. This implies that when an msdu is spread out over
			 multiple buffers and thus msdu_continuation is set, only
			 for the very last buffer of the msdu, can the 'last_msdu_in_mpdu_flag' 
			be set.
			
			When both first_msdu_in_mpdu_flag and last_msdu_in_mpdu_flag
			 are set, the MPDU that this MSDU belongs to only contains
			 a single MSDU.
			
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_LSB    1
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MSB    1
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MASK   0x00000002


/* Description		MSDU_CONTINUATION

			When set, this MSDU buffer was not able to hold the entire
			 MSDU. The next buffer will therefor contain additional 
			information related to this MSDU.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_OFFSET      0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_LSB         2
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MSB         2
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MASK        0x00000004


/* Description		MSDU_LENGTH

			Parsed from RX_MSDU_START TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the First
			 buffer used by MSDU.
			 
			Full MSDU length in bytes after decapsulation. 
			
			This field is still valid for MPDU frames without A-MSDU. 
			 It still represents MSDU length after decapsulation 
			
			Or in case of RAW MPDUs, it indicates the length of the 
			entire MPDU (without FCS field)
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_OFFSET            0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_LSB               3
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MSB               16
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MASK              0x0001fff8


/* Description		MSDU_DROP

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			When set, REO shall drop this MSDU and not forward it to
			 any other ring...
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_OFFSET              0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_LSB                 17
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MSB                 17
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MASK                0x00020000


/* Description		SA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid SA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_OFFSET            0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_LSB               18
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MSB               18
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MASK              0x00040000


/* Description		DA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid DA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_OFFSET            0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_LSB               19
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MSB               19
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MASK              0x00080000


/* Description		DA_IS_MCBC

			Field Only valid if "da_is_valid" is set
			
			Indicates the DA address was a Multicast of Broadcast address
			 for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_OFFSET             0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_LSB                20
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MSB                20
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MASK               0x00100000


/* Description		L3_HEADER_PADDING_MSB

			Passed on from 'RX_MSDU_END' TLV (only the MSB is reported
			 as the LSB is always zero)
			Number of bytes padded to make sure that the L3 header will
			 always start of a Dword boundary
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_OFFSET  0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_LSB     21
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MSB     21
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MASK    0x00200000


/* Description		TCP_UDP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the TCP/UDP header.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_OFFSET    0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_LSB       22
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MSB       22
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MASK      0x00400000


/* Description		IP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the IP header.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_OFFSET         0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_LSB            23
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MSB            23
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MASK           0x00800000


/* Description		FR_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'from DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_FR_DS_OFFSET                  0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_FR_DS_LSB                     24
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MSB                     24
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MASK                    0x01000000


/* Description		TO_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'to DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_TO_DS_OFFSET                  0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_TO_DS_LSB                     25
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MSB                     25
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MASK                    0x02000000


/* Description		INTRA_BSS

			This packet needs intra-BSS routing by SW as the 'vdev_id' 
			for the destination is the same as the 'vdev_id' (from 'RX_MPDU_PCU_START') 
			that this MSDU was got in.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_OFFSET              0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_LSB                 26
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MSB                 26
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MASK                0x04000000


/* Description		DEST_CHIP_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which chip's TCL the packet should be
			 queued.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_OFFSET           0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_LSB              27
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MSB              28
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MASK             0x18000000


/* Description		DECAP_FORMAT

			Indicates the format after decapsulation:
			
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_OFFSET           0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_LSB              29
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MSB              30
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MASK             0x60000000


/* Description		DEST_CHIP_PMAC_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which link/'vdev' the packet should 
			be queued in TCL.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_OFFSET      0x00000028
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_LSB         31
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MSB         31
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MASK        0x80000000


/* Description		RX_MSDU_EXT_DESC_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			Extended information related to the MSDU that is passed 
			on from RXDMA to REO but not part of the REO destination
			 ring. Some fields are passed on to PPE.
*/


/* Description		REO_DESTINATION_INDICATION

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			The ID of the REO exit ring where the MSDU frame shall push
			 after (MPDU level) reordering has finished.
			
			<enum 0 reo_destination_sw0> Reo will push the frame into
			 the REO2SW0 ring
			<enum 1 reo_destination_sw1> Reo will push the frame into
			 the REO2SW1 ring
			<enum 2 reo_destination_sw2> Reo will push the frame into
			 the REO2SW2 ring
			<enum 3 reo_destination_sw3> Reo will push the frame into
			 the REO2SW3 ring
			<enum 4 reo_destination_sw4> Reo will push the frame into
			 the REO2SW4 ring
			<enum 5 reo_destination_release> Reo will push the frame
			 into the REO_release ring
			<enum 6 reo_destination_fw> Reo will push the frame into
			 the REO2FW ring
			<enum 7 reo_destination_sw5> Reo will push the frame into
			 the REO2SW5 ring (REO remaps this in chips without REO2SW5
			 ring, e.g. Pine) 
			<enum 8 reo_destination_sw6> Reo will push the frame into
			 the REO2SW6 ring (REO remaps this in chips without REO2SW6
			 ring, e.g. Pine)
			 <enum 9 reo_destination_sw7> Reo will push the frame into
			 the REO2SW7 ring (REO remaps this in chips without REO2SW7
			 ring)
			<enum 10 reo_destination_sw8> Reo will push the frame into
			 the REO2SW8 ring (REO remaps this in chips without REO2SW8
			 ring)
			<enum 11 reo_destination_11> REO remaps this 
			<enum 12 reo_destination_12> REO remaps this <enum 13 reo_destination_13> 
			REO remaps this 
			<enum 14 reo_destination_14> REO remaps this 
			<enum 15 reo_destination_15> REO remaps this 
			<enum 16 reo_destination_16> REO remaps this 
			<enum 17 reo_destination_17> REO remaps this 
			<enum 18 reo_destination_18> REO remaps this 
			<enum 19 reo_destination_19> REO remaps this 
			<enum 20 reo_destination_20> REO remaps this 
			<enum 21 reo_destination_21> REO remaps this 
			<enum 22 reo_destination_22> REO remaps this 
			<enum 23 reo_destination_23> REO remaps this 
			<enum 24 reo_destination_24> REO remaps this 
			<enum 25 reo_destination_25> REO remaps this 
			<enum 26 reo_destination_26> REO remaps this 
			<enum 27 reo_destination_27> REO remaps this 
			<enum 28 reo_destination_28> REO remaps this 
			<enum 29 reo_destination_29> REO remaps this 
			<enum 30 reo_destination_30> REO remaps this 
			<enum 31 reo_destination_31> REO remaps this 
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_OFFSET 0x0000002c
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_LSB 0
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MSB 4
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MASK 0x0000001f


/* Description		SERVICE_CODE

			Opaque service code between PPE and Wi-Fi
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_OFFSET       0x0000002c
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_LSB          5
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_MSB          13
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_MASK         0x00003fe0


/* Description		PRIORITY_VALID

			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_OFFSET     0x0000002c
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_LSB        14
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_MSB        14
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_MASK       0x00004000


/* Description		DATA_OFFSET

			The offset to Rx packet data within the buffer (including
			 Rx DMA offset programming and L3 header padding inserted
			 by Rx OLE).
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_OFFSET        0x0000002c
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_LSB           15
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_MSB           26
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_MASK          0x07ff8000


/* Description		SRC_LINK_ID

			Consumer: SW
			Producer: RXDMA
			
			Set to the link ID of the PMAC that received the frame
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_OFFSET        0x0000002c
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_LSB           27
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_MSB           29
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_MASK          0x38000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_OFFSET        0x0000002c
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_LSB           30
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_MSB           31
#define RX_MSDU_LINK_MSDU_0_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_MASK          0xc0000000


/* Description		MSDU_1

			Details of next MSDU in this (MSDU flow) linked list
*/


/* Description		BUFFER_ADDR_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			Details of the physical address of the buffer containing
			 an MSDU (or entire MPDU)
*/


/* Description		BUFFER_ADDR_31_0

			Address (lower 32 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_OFFSET        0x00000030
#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_LSB           0
#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MSB           31
#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MASK          0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_OFFSET       0x00000034
#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_LSB          0
#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MSB          7
#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MASK         0x000000ff


/* Description		RETURN_BUFFER_MANAGER

			Consumer: WBM
			Producer: SW/FW
			
			In case of 'NULL' pointer, this field is set to 0
			
			Indicates to which buffer manager the buffer OR MSDU_EXTENSION
			 descriptor OR link descriptor that is being pointed to 
			shall be returned after the frame has been processed. It
			 is used by WBM for routing purposes.
			
			<enum 0 WBM_IDLE_BUF_LIST> This buffer shall be returned
			 to the WMB buffer idle list
			<enum 1 WBM_CHIP0_IDLE_DESC_LIST> This buffer shall be returned
			 to the WBM idle link descriptor idle list, where the chip
			 0 WBM is chosen in case of a multi-chip config
			<enum 2 WBM_CHIP1_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 1 WBM idle link descriptor idle list
			<enum 3 WBM_CHIP2_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 2 WBM idle link descriptor idle list
			<enum 12 WBM_CHIP3_IDLE_DESC_LIST> This buffer shall be 
			returned to chip 3 WBM idle link descriptor idle list
			<enum 4 FW_BM> This buffer shall be returned to the FW
			<enum 5 SW0_BM> This buffer shall be returned to the SW, 
			ring 0
			<enum 6 SW1_BM> This buffer shall be returned to the SW, 
			ring 1
			<enum 7 SW2_BM> This buffer shall be returned to the SW, 
			ring 2
			<enum 8 SW3_BM> This buffer shall be returned to the SW, 
			ring 3
			<enum 9 SW4_BM> This buffer shall be returned to the SW, 
			ring 4
			<enum 10 SW5_BM> This buffer shall be returned to the SW, 
			ring 5
			<enum 11 SW6_BM> This buffer shall be returned to the SW, 
			ring 6
			
			<legal 0-12>
*/

#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_OFFSET   0x00000034
#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_LSB      8
#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MSB      11
#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MASK     0x00000f00


/* Description		SW_BUFFER_COOKIE

			Cookie field exclusively used by SW. 
			
			In case of 'NULL' pointer, this field is set to 0
			
			HW ignores the contents, accept that it passes the programmed
			 value on to other descriptors together with the physical
			 address 
			
			Field can be used by SW to for example associate the buffers
			 physical address with the virtual address
			The bit definitions as used by SW are within SW HLD specification
			
			
			NOTE1:
			The three most significant bits can have a special meaning
			 in case this struct is embedded in a TX_MPDU_DETAILS STRUCT, 
			and field transmit_bw_restriction is set
			
			In case of NON punctured transmission:
			Sw_buffer_cookie[19:17] = 3'b000: 20 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b001: 40 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b010: 80 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b011: 160 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b101: 240 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b100: 320 MHz TX only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			In case of punctured transmission:
			Sw_buffer_cookie[19:16] = 4'b0000: pattern 0 only
			Sw_buffer_cookie[19:16] = 4'b0001: pattern 1 only
			Sw_buffer_cookie[19:16] = 4'b0010: pattern 2 only
			Sw_buffer_cookie[19:16] = 4'b0011: pattern 3 only
			Sw_buffer_cookie[19:16] = 4'b0100: pattern 4 only
			Sw_buffer_cookie[19:16] = 4'b0101: pattern 5 only
			Sw_buffer_cookie[19:16] = 4'b0110: pattern 6 only
			Sw_buffer_cookie[19:16] = 4'b0111: pattern 7 only
			Sw_buffer_cookie[19:16] = 4'b1000: pattern 8 only
			Sw_buffer_cookie[19:16] = 4'b1001: pattern 9 only
			Sw_buffer_cookie[19:16] = 4'b1010: pattern 10 only
			Sw_buffer_cookie[19:16] = 4'b1011: pattern 11 only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			Note: a punctured transmission is indicated by the presence
			 of TLV TX_PUNCTURE_SETUP embedded in the scheduler TLV
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_OFFSET        0x00000034
#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_LSB           12
#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MSB           31
#define RX_MSDU_LINK_MSDU_1_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MASK          0xfffff000


/* Description		RX_MSDU_DESC_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			General information related to the MSDU that should be passed
			 on from RXDMA all the way to to the REO destination ring.
			
*/


/* Description		FIRST_MSDU_IN_MPDU_FLAG

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU 
			
			<enum 0 Not_first_msdu> This is not the first MSDU in the
			 MPDU. 
			<enum 1 first_msdu> This MSDU is the first one in the MPDU.
			
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_LSB   0
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MSB   0
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MASK  0x00000001


/* Description		LAST_MSDU_IN_MPDU_FLAG

			Consumer: WBM/REO/SW/FW
			Producer: RXDMA
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			
			<enum 0 Not_last_msdu> There are more MSDUs linked to this
			 MSDU that belongs to this MPDU 
			<enum 1 Last_msdu> this MSDU is the last one in the MPDU. 
			This setting is only allowed in combination with 'Msdu_continuation' 
			set to 0. This implies that when an msdu is spread out over
			 multiple buffers and thus msdu_continuation is set, only
			 for the very last buffer of the msdu, can the 'last_msdu_in_mpdu_flag' 
			be set.
			
			When both first_msdu_in_mpdu_flag and last_msdu_in_mpdu_flag
			 are set, the MPDU that this MSDU belongs to only contains
			 a single MSDU.
			
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_LSB    1
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MSB    1
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MASK   0x00000002


/* Description		MSDU_CONTINUATION

			When set, this MSDU buffer was not able to hold the entire
			 MSDU. The next buffer will therefor contain additional 
			information related to this MSDU.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_OFFSET      0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_LSB         2
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MSB         2
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MASK        0x00000004


/* Description		MSDU_LENGTH

			Parsed from RX_MSDU_START TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the First
			 buffer used by MSDU.
			 
			Full MSDU length in bytes after decapsulation. 
			
			This field is still valid for MPDU frames without A-MSDU. 
			 It still represents MSDU length after decapsulation 
			
			Or in case of RAW MPDUs, it indicates the length of the 
			entire MPDU (without FCS field)
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_OFFSET            0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_LSB               3
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MSB               16
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MASK              0x0001fff8


/* Description		MSDU_DROP

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			When set, REO shall drop this MSDU and not forward it to
			 any other ring...
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_OFFSET              0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_LSB                 17
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MSB                 17
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MASK                0x00020000


/* Description		SA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid SA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_OFFSET            0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_LSB               18
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MSB               18
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MASK              0x00040000


/* Description		DA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid DA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_OFFSET            0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_LSB               19
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MSB               19
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MASK              0x00080000


/* Description		DA_IS_MCBC

			Field Only valid if "da_is_valid" is set
			
			Indicates the DA address was a Multicast of Broadcast address
			 for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_OFFSET             0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_LSB                20
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MSB                20
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MASK               0x00100000


/* Description		L3_HEADER_PADDING_MSB

			Passed on from 'RX_MSDU_END' TLV (only the MSB is reported
			 as the LSB is always zero)
			Number of bytes padded to make sure that the L3 header will
			 always start of a Dword boundary
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_OFFSET  0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_LSB     21
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MSB     21
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MASK    0x00200000


/* Description		TCP_UDP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the TCP/UDP header.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_OFFSET    0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_LSB       22
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MSB       22
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MASK      0x00400000


/* Description		IP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the IP header.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_OFFSET         0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_LSB            23
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MSB            23
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MASK           0x00800000


/* Description		FR_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'from DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_FR_DS_OFFSET                  0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_FR_DS_LSB                     24
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MSB                     24
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MASK                    0x01000000


/* Description		TO_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'to DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_TO_DS_OFFSET                  0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_TO_DS_LSB                     25
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MSB                     25
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MASK                    0x02000000


/* Description		INTRA_BSS

			This packet needs intra-BSS routing by SW as the 'vdev_id' 
			for the destination is the same as the 'vdev_id' (from 'RX_MPDU_PCU_START') 
			that this MSDU was got in.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_OFFSET              0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_LSB                 26
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MSB                 26
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MASK                0x04000000


/* Description		DEST_CHIP_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which chip's TCL the packet should be
			 queued.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_OFFSET           0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_LSB              27
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MSB              28
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MASK             0x18000000


/* Description		DECAP_FORMAT

			Indicates the format after decapsulation:
			
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_OFFSET           0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_LSB              29
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MSB              30
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MASK             0x60000000


/* Description		DEST_CHIP_PMAC_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which link/'vdev' the packet should 
			be queued in TCL.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_OFFSET      0x00000038
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_LSB         31
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MSB         31
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MASK        0x80000000


/* Description		RX_MSDU_EXT_DESC_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			Extended information related to the MSDU that is passed 
			on from RXDMA to REO but not part of the REO destination
			 ring. Some fields are passed on to PPE.
*/


/* Description		REO_DESTINATION_INDICATION

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			The ID of the REO exit ring where the MSDU frame shall push
			 after (MPDU level) reordering has finished.
			
			<enum 0 reo_destination_sw0> Reo will push the frame into
			 the REO2SW0 ring
			<enum 1 reo_destination_sw1> Reo will push the frame into
			 the REO2SW1 ring
			<enum 2 reo_destination_sw2> Reo will push the frame into
			 the REO2SW2 ring
			<enum 3 reo_destination_sw3> Reo will push the frame into
			 the REO2SW3 ring
			<enum 4 reo_destination_sw4> Reo will push the frame into
			 the REO2SW4 ring
			<enum 5 reo_destination_release> Reo will push the frame
			 into the REO_release ring
			<enum 6 reo_destination_fw> Reo will push the frame into
			 the REO2FW ring
			<enum 7 reo_destination_sw5> Reo will push the frame into
			 the REO2SW5 ring (REO remaps this in chips without REO2SW5
			 ring, e.g. Pine) 
			<enum 8 reo_destination_sw6> Reo will push the frame into
			 the REO2SW6 ring (REO remaps this in chips without REO2SW6
			 ring, e.g. Pine)
			 <enum 9 reo_destination_sw7> Reo will push the frame into
			 the REO2SW7 ring (REO remaps this in chips without REO2SW7
			 ring)
			<enum 10 reo_destination_sw8> Reo will push the frame into
			 the REO2SW8 ring (REO remaps this in chips without REO2SW8
			 ring)
			<enum 11 reo_destination_11> REO remaps this 
			<enum 12 reo_destination_12> REO remaps this <enum 13 reo_destination_13> 
			REO remaps this 
			<enum 14 reo_destination_14> REO remaps this 
			<enum 15 reo_destination_15> REO remaps this 
			<enum 16 reo_destination_16> REO remaps this 
			<enum 17 reo_destination_17> REO remaps this 
			<enum 18 reo_destination_18> REO remaps this 
			<enum 19 reo_destination_19> REO remaps this 
			<enum 20 reo_destination_20> REO remaps this 
			<enum 21 reo_destination_21> REO remaps this 
			<enum 22 reo_destination_22> REO remaps this 
			<enum 23 reo_destination_23> REO remaps this 
			<enum 24 reo_destination_24> REO remaps this 
			<enum 25 reo_destination_25> REO remaps this 
			<enum 26 reo_destination_26> REO remaps this 
			<enum 27 reo_destination_27> REO remaps this 
			<enum 28 reo_destination_28> REO remaps this 
			<enum 29 reo_destination_29> REO remaps this 
			<enum 30 reo_destination_30> REO remaps this 
			<enum 31 reo_destination_31> REO remaps this 
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_OFFSET 0x0000003c
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_LSB 0
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MSB 4
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MASK 0x0000001f


/* Description		SERVICE_CODE

			Opaque service code between PPE and Wi-Fi
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_OFFSET       0x0000003c
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_LSB          5
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_MSB          13
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_MASK         0x00003fe0


/* Description		PRIORITY_VALID

			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_OFFSET     0x0000003c
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_LSB        14
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_MSB        14
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_MASK       0x00004000


/* Description		DATA_OFFSET

			The offset to Rx packet data within the buffer (including
			 Rx DMA offset programming and L3 header padding inserted
			 by Rx OLE).
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_OFFSET        0x0000003c
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_LSB           15
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_MSB           26
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_MASK          0x07ff8000


/* Description		SRC_LINK_ID

			Consumer: SW
			Producer: RXDMA
			
			Set to the link ID of the PMAC that received the frame
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_OFFSET        0x0000003c
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_LSB           27
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_MSB           29
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_MASK          0x38000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_OFFSET        0x0000003c
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_LSB           30
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_MSB           31
#define RX_MSDU_LINK_MSDU_1_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_MASK          0xc0000000


/* Description		MSDU_2

			Details of next MSDU in this (MSDU flow) linked list
*/


/* Description		BUFFER_ADDR_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			Details of the physical address of the buffer containing
			 an MSDU (or entire MPDU)
*/


/* Description		BUFFER_ADDR_31_0

			Address (lower 32 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_OFFSET        0x00000040
#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_LSB           0
#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MSB           31
#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MASK          0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_OFFSET       0x00000044
#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_LSB          0
#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MSB          7
#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MASK         0x000000ff


/* Description		RETURN_BUFFER_MANAGER

			Consumer: WBM
			Producer: SW/FW
			
			In case of 'NULL' pointer, this field is set to 0
			
			Indicates to which buffer manager the buffer OR MSDU_EXTENSION
			 descriptor OR link descriptor that is being pointed to 
			shall be returned after the frame has been processed. It
			 is used by WBM for routing purposes.
			
			<enum 0 WBM_IDLE_BUF_LIST> This buffer shall be returned
			 to the WMB buffer idle list
			<enum 1 WBM_CHIP0_IDLE_DESC_LIST> This buffer shall be returned
			 to the WBM idle link descriptor idle list, where the chip
			 0 WBM is chosen in case of a multi-chip config
			<enum 2 WBM_CHIP1_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 1 WBM idle link descriptor idle list
			<enum 3 WBM_CHIP2_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 2 WBM idle link descriptor idle list
			<enum 12 WBM_CHIP3_IDLE_DESC_LIST> This buffer shall be 
			returned to chip 3 WBM idle link descriptor idle list
			<enum 4 FW_BM> This buffer shall be returned to the FW
			<enum 5 SW0_BM> This buffer shall be returned to the SW, 
			ring 0
			<enum 6 SW1_BM> This buffer shall be returned to the SW, 
			ring 1
			<enum 7 SW2_BM> This buffer shall be returned to the SW, 
			ring 2
			<enum 8 SW3_BM> This buffer shall be returned to the SW, 
			ring 3
			<enum 9 SW4_BM> This buffer shall be returned to the SW, 
			ring 4
			<enum 10 SW5_BM> This buffer shall be returned to the SW, 
			ring 5
			<enum 11 SW6_BM> This buffer shall be returned to the SW, 
			ring 6
			
			<legal 0-12>
*/

#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_OFFSET   0x00000044
#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_LSB      8
#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MSB      11
#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MASK     0x00000f00


/* Description		SW_BUFFER_COOKIE

			Cookie field exclusively used by SW. 
			
			In case of 'NULL' pointer, this field is set to 0
			
			HW ignores the contents, accept that it passes the programmed
			 value on to other descriptors together with the physical
			 address 
			
			Field can be used by SW to for example associate the buffers
			 physical address with the virtual address
			The bit definitions as used by SW are within SW HLD specification
			
			
			NOTE1:
			The three most significant bits can have a special meaning
			 in case this struct is embedded in a TX_MPDU_DETAILS STRUCT, 
			and field transmit_bw_restriction is set
			
			In case of NON punctured transmission:
			Sw_buffer_cookie[19:17] = 3'b000: 20 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b001: 40 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b010: 80 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b011: 160 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b101: 240 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b100: 320 MHz TX only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			In case of punctured transmission:
			Sw_buffer_cookie[19:16] = 4'b0000: pattern 0 only
			Sw_buffer_cookie[19:16] = 4'b0001: pattern 1 only
			Sw_buffer_cookie[19:16] = 4'b0010: pattern 2 only
			Sw_buffer_cookie[19:16] = 4'b0011: pattern 3 only
			Sw_buffer_cookie[19:16] = 4'b0100: pattern 4 only
			Sw_buffer_cookie[19:16] = 4'b0101: pattern 5 only
			Sw_buffer_cookie[19:16] = 4'b0110: pattern 6 only
			Sw_buffer_cookie[19:16] = 4'b0111: pattern 7 only
			Sw_buffer_cookie[19:16] = 4'b1000: pattern 8 only
			Sw_buffer_cookie[19:16] = 4'b1001: pattern 9 only
			Sw_buffer_cookie[19:16] = 4'b1010: pattern 10 only
			Sw_buffer_cookie[19:16] = 4'b1011: pattern 11 only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			Note: a punctured transmission is indicated by the presence
			 of TLV TX_PUNCTURE_SETUP embedded in the scheduler TLV
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_OFFSET        0x00000044
#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_LSB           12
#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MSB           31
#define RX_MSDU_LINK_MSDU_2_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MASK          0xfffff000


/* Description		RX_MSDU_DESC_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			General information related to the MSDU that should be passed
			 on from RXDMA all the way to to the REO destination ring.
			
*/


/* Description		FIRST_MSDU_IN_MPDU_FLAG

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU 
			
			<enum 0 Not_first_msdu> This is not the first MSDU in the
			 MPDU. 
			<enum 1 first_msdu> This MSDU is the first one in the MPDU.
			
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_LSB   0
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MSB   0
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MASK  0x00000001


/* Description		LAST_MSDU_IN_MPDU_FLAG

			Consumer: WBM/REO/SW/FW
			Producer: RXDMA
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			
			<enum 0 Not_last_msdu> There are more MSDUs linked to this
			 MSDU that belongs to this MPDU 
			<enum 1 Last_msdu> this MSDU is the last one in the MPDU. 
			This setting is only allowed in combination with 'Msdu_continuation' 
			set to 0. This implies that when an msdu is spread out over
			 multiple buffers and thus msdu_continuation is set, only
			 for the very last buffer of the msdu, can the 'last_msdu_in_mpdu_flag' 
			be set.
			
			When both first_msdu_in_mpdu_flag and last_msdu_in_mpdu_flag
			 are set, the MPDU that this MSDU belongs to only contains
			 a single MSDU.
			
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_LSB    1
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MSB    1
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MASK   0x00000002


/* Description		MSDU_CONTINUATION

			When set, this MSDU buffer was not able to hold the entire
			 MSDU. The next buffer will therefor contain additional 
			information related to this MSDU.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_OFFSET      0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_LSB         2
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MSB         2
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MASK        0x00000004


/* Description		MSDU_LENGTH

			Parsed from RX_MSDU_START TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the First
			 buffer used by MSDU.
			 
			Full MSDU length in bytes after decapsulation. 
			
			This field is still valid for MPDU frames without A-MSDU. 
			 It still represents MSDU length after decapsulation 
			
			Or in case of RAW MPDUs, it indicates the length of the 
			entire MPDU (without FCS field)
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_OFFSET            0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_LSB               3
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MSB               16
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MASK              0x0001fff8


/* Description		MSDU_DROP

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			When set, REO shall drop this MSDU and not forward it to
			 any other ring...
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_OFFSET              0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_LSB                 17
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MSB                 17
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MASK                0x00020000


/* Description		SA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid SA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_OFFSET            0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_LSB               18
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MSB               18
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MASK              0x00040000


/* Description		DA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid DA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_OFFSET            0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_LSB               19
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MSB               19
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MASK              0x00080000


/* Description		DA_IS_MCBC

			Field Only valid if "da_is_valid" is set
			
			Indicates the DA address was a Multicast of Broadcast address
			 for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_OFFSET             0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_LSB                20
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MSB                20
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MASK               0x00100000


/* Description		L3_HEADER_PADDING_MSB

			Passed on from 'RX_MSDU_END' TLV (only the MSB is reported
			 as the LSB is always zero)
			Number of bytes padded to make sure that the L3 header will
			 always start of a Dword boundary
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_OFFSET  0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_LSB     21
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MSB     21
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MASK    0x00200000


/* Description		TCP_UDP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the TCP/UDP header.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_OFFSET    0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_LSB       22
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MSB       22
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MASK      0x00400000


/* Description		IP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the IP header.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_OFFSET         0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_LSB            23
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MSB            23
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MASK           0x00800000


/* Description		FR_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'from DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_FR_DS_OFFSET                  0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_FR_DS_LSB                     24
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MSB                     24
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MASK                    0x01000000


/* Description		TO_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'to DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_TO_DS_OFFSET                  0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_TO_DS_LSB                     25
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MSB                     25
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MASK                    0x02000000


/* Description		INTRA_BSS

			This packet needs intra-BSS routing by SW as the 'vdev_id' 
			for the destination is the same as the 'vdev_id' (from 'RX_MPDU_PCU_START') 
			that this MSDU was got in.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_OFFSET              0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_LSB                 26
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MSB                 26
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MASK                0x04000000


/* Description		DEST_CHIP_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which chip's TCL the packet should be
			 queued.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_OFFSET           0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_LSB              27
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MSB              28
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MASK             0x18000000


/* Description		DECAP_FORMAT

			Indicates the format after decapsulation:
			
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_OFFSET           0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_LSB              29
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MSB              30
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MASK             0x60000000


/* Description		DEST_CHIP_PMAC_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which link/'vdev' the packet should 
			be queued in TCL.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_OFFSET      0x00000048
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_LSB         31
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MSB         31
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MASK        0x80000000


/* Description		RX_MSDU_EXT_DESC_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			Extended information related to the MSDU that is passed 
			on from RXDMA to REO but not part of the REO destination
			 ring. Some fields are passed on to PPE.
*/


/* Description		REO_DESTINATION_INDICATION

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			The ID of the REO exit ring where the MSDU frame shall push
			 after (MPDU level) reordering has finished.
			
			<enum 0 reo_destination_sw0> Reo will push the frame into
			 the REO2SW0 ring
			<enum 1 reo_destination_sw1> Reo will push the frame into
			 the REO2SW1 ring
			<enum 2 reo_destination_sw2> Reo will push the frame into
			 the REO2SW2 ring
			<enum 3 reo_destination_sw3> Reo will push the frame into
			 the REO2SW3 ring
			<enum 4 reo_destination_sw4> Reo will push the frame into
			 the REO2SW4 ring
			<enum 5 reo_destination_release> Reo will push the frame
			 into the REO_release ring
			<enum 6 reo_destination_fw> Reo will push the frame into
			 the REO2FW ring
			<enum 7 reo_destination_sw5> Reo will push the frame into
			 the REO2SW5 ring (REO remaps this in chips without REO2SW5
			 ring, e.g. Pine) 
			<enum 8 reo_destination_sw6> Reo will push the frame into
			 the REO2SW6 ring (REO remaps this in chips without REO2SW6
			 ring, e.g. Pine)
			 <enum 9 reo_destination_sw7> Reo will push the frame into
			 the REO2SW7 ring (REO remaps this in chips without REO2SW7
			 ring)
			<enum 10 reo_destination_sw8> Reo will push the frame into
			 the REO2SW8 ring (REO remaps this in chips without REO2SW8
			 ring)
			<enum 11 reo_destination_11> REO remaps this 
			<enum 12 reo_destination_12> REO remaps this <enum 13 reo_destination_13> 
			REO remaps this 
			<enum 14 reo_destination_14> REO remaps this 
			<enum 15 reo_destination_15> REO remaps this 
			<enum 16 reo_destination_16> REO remaps this 
			<enum 17 reo_destination_17> REO remaps this 
			<enum 18 reo_destination_18> REO remaps this 
			<enum 19 reo_destination_19> REO remaps this 
			<enum 20 reo_destination_20> REO remaps this 
			<enum 21 reo_destination_21> REO remaps this 
			<enum 22 reo_destination_22> REO remaps this 
			<enum 23 reo_destination_23> REO remaps this 
			<enum 24 reo_destination_24> REO remaps this 
			<enum 25 reo_destination_25> REO remaps this 
			<enum 26 reo_destination_26> REO remaps this 
			<enum 27 reo_destination_27> REO remaps this 
			<enum 28 reo_destination_28> REO remaps this 
			<enum 29 reo_destination_29> REO remaps this 
			<enum 30 reo_destination_30> REO remaps this 
			<enum 31 reo_destination_31> REO remaps this 
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_OFFSET 0x0000004c
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_LSB 0
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MSB 4
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MASK 0x0000001f


/* Description		SERVICE_CODE

			Opaque service code between PPE and Wi-Fi
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_OFFSET       0x0000004c
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_LSB          5
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_MSB          13
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_MASK         0x00003fe0


/* Description		PRIORITY_VALID

			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_OFFSET     0x0000004c
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_LSB        14
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_MSB        14
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_MASK       0x00004000


/* Description		DATA_OFFSET

			The offset to Rx packet data within the buffer (including
			 Rx DMA offset programming and L3 header padding inserted
			 by Rx OLE).
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_OFFSET        0x0000004c
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_LSB           15
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_MSB           26
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_MASK          0x07ff8000


/* Description		SRC_LINK_ID

			Consumer: SW
			Producer: RXDMA
			
			Set to the link ID of the PMAC that received the frame
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_OFFSET        0x0000004c
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_LSB           27
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_MSB           29
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_MASK          0x38000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_OFFSET        0x0000004c
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_LSB           30
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_MSB           31
#define RX_MSDU_LINK_MSDU_2_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_MASK          0xc0000000


/* Description		MSDU_3

			Details of next MSDU in this (MSDU flow) linked list
*/


/* Description		BUFFER_ADDR_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			Details of the physical address of the buffer containing
			 an MSDU (or entire MPDU)
*/


/* Description		BUFFER_ADDR_31_0

			Address (lower 32 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_OFFSET        0x00000050
#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_LSB           0
#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MSB           31
#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MASK          0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_OFFSET       0x00000054
#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_LSB          0
#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MSB          7
#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MASK         0x000000ff


/* Description		RETURN_BUFFER_MANAGER

			Consumer: WBM
			Producer: SW/FW
			
			In case of 'NULL' pointer, this field is set to 0
			
			Indicates to which buffer manager the buffer OR MSDU_EXTENSION
			 descriptor OR link descriptor that is being pointed to 
			shall be returned after the frame has been processed. It
			 is used by WBM for routing purposes.
			
			<enum 0 WBM_IDLE_BUF_LIST> This buffer shall be returned
			 to the WMB buffer idle list
			<enum 1 WBM_CHIP0_IDLE_DESC_LIST> This buffer shall be returned
			 to the WBM idle link descriptor idle list, where the chip
			 0 WBM is chosen in case of a multi-chip config
			<enum 2 WBM_CHIP1_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 1 WBM idle link descriptor idle list
			<enum 3 WBM_CHIP2_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 2 WBM idle link descriptor idle list
			<enum 12 WBM_CHIP3_IDLE_DESC_LIST> This buffer shall be 
			returned to chip 3 WBM idle link descriptor idle list
			<enum 4 FW_BM> This buffer shall be returned to the FW
			<enum 5 SW0_BM> This buffer shall be returned to the SW, 
			ring 0
			<enum 6 SW1_BM> This buffer shall be returned to the SW, 
			ring 1
			<enum 7 SW2_BM> This buffer shall be returned to the SW, 
			ring 2
			<enum 8 SW3_BM> This buffer shall be returned to the SW, 
			ring 3
			<enum 9 SW4_BM> This buffer shall be returned to the SW, 
			ring 4
			<enum 10 SW5_BM> This buffer shall be returned to the SW, 
			ring 5
			<enum 11 SW6_BM> This buffer shall be returned to the SW, 
			ring 6
			
			<legal 0-12>
*/

#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_OFFSET   0x00000054
#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_LSB      8
#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MSB      11
#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MASK     0x00000f00


/* Description		SW_BUFFER_COOKIE

			Cookie field exclusively used by SW. 
			
			In case of 'NULL' pointer, this field is set to 0
			
			HW ignores the contents, accept that it passes the programmed
			 value on to other descriptors together with the physical
			 address 
			
			Field can be used by SW to for example associate the buffers
			 physical address with the virtual address
			The bit definitions as used by SW are within SW HLD specification
			
			
			NOTE1:
			The three most significant bits can have a special meaning
			 in case this struct is embedded in a TX_MPDU_DETAILS STRUCT, 
			and field transmit_bw_restriction is set
			
			In case of NON punctured transmission:
			Sw_buffer_cookie[19:17] = 3'b000: 20 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b001: 40 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b010: 80 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b011: 160 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b101: 240 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b100: 320 MHz TX only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			In case of punctured transmission:
			Sw_buffer_cookie[19:16] = 4'b0000: pattern 0 only
			Sw_buffer_cookie[19:16] = 4'b0001: pattern 1 only
			Sw_buffer_cookie[19:16] = 4'b0010: pattern 2 only
			Sw_buffer_cookie[19:16] = 4'b0011: pattern 3 only
			Sw_buffer_cookie[19:16] = 4'b0100: pattern 4 only
			Sw_buffer_cookie[19:16] = 4'b0101: pattern 5 only
			Sw_buffer_cookie[19:16] = 4'b0110: pattern 6 only
			Sw_buffer_cookie[19:16] = 4'b0111: pattern 7 only
			Sw_buffer_cookie[19:16] = 4'b1000: pattern 8 only
			Sw_buffer_cookie[19:16] = 4'b1001: pattern 9 only
			Sw_buffer_cookie[19:16] = 4'b1010: pattern 10 only
			Sw_buffer_cookie[19:16] = 4'b1011: pattern 11 only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			Note: a punctured transmission is indicated by the presence
			 of TLV TX_PUNCTURE_SETUP embedded in the scheduler TLV
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_OFFSET        0x00000054
#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_LSB           12
#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MSB           31
#define RX_MSDU_LINK_MSDU_3_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MASK          0xfffff000


/* Description		RX_MSDU_DESC_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			General information related to the MSDU that should be passed
			 on from RXDMA all the way to to the REO destination ring.
			
*/


/* Description		FIRST_MSDU_IN_MPDU_FLAG

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU 
			
			<enum 0 Not_first_msdu> This is not the first MSDU in the
			 MPDU. 
			<enum 1 first_msdu> This MSDU is the first one in the MPDU.
			
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_LSB   0
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MSB   0
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MASK  0x00000001


/* Description		LAST_MSDU_IN_MPDU_FLAG

			Consumer: WBM/REO/SW/FW
			Producer: RXDMA
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			
			<enum 0 Not_last_msdu> There are more MSDUs linked to this
			 MSDU that belongs to this MPDU 
			<enum 1 Last_msdu> this MSDU is the last one in the MPDU. 
			This setting is only allowed in combination with 'Msdu_continuation' 
			set to 0. This implies that when an msdu is spread out over
			 multiple buffers and thus msdu_continuation is set, only
			 for the very last buffer of the msdu, can the 'last_msdu_in_mpdu_flag' 
			be set.
			
			When both first_msdu_in_mpdu_flag and last_msdu_in_mpdu_flag
			 are set, the MPDU that this MSDU belongs to only contains
			 a single MSDU.
			
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_LSB    1
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MSB    1
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MASK   0x00000002


/* Description		MSDU_CONTINUATION

			When set, this MSDU buffer was not able to hold the entire
			 MSDU. The next buffer will therefor contain additional 
			information related to this MSDU.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_OFFSET      0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_LSB         2
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MSB         2
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MASK        0x00000004


/* Description		MSDU_LENGTH

			Parsed from RX_MSDU_START TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the First
			 buffer used by MSDU.
			 
			Full MSDU length in bytes after decapsulation. 
			
			This field is still valid for MPDU frames without A-MSDU. 
			 It still represents MSDU length after decapsulation 
			
			Or in case of RAW MPDUs, it indicates the length of the 
			entire MPDU (without FCS field)
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_OFFSET            0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_LSB               3
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MSB               16
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MASK              0x0001fff8


/* Description		MSDU_DROP

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			When set, REO shall drop this MSDU and not forward it to
			 any other ring...
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_OFFSET              0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_LSB                 17
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MSB                 17
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MASK                0x00020000


/* Description		SA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid SA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_OFFSET            0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_LSB               18
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MSB               18
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MASK              0x00040000


/* Description		DA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid DA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_OFFSET            0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_LSB               19
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MSB               19
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MASK              0x00080000


/* Description		DA_IS_MCBC

			Field Only valid if "da_is_valid" is set
			
			Indicates the DA address was a Multicast of Broadcast address
			 for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_OFFSET             0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_LSB                20
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MSB                20
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MASK               0x00100000


/* Description		L3_HEADER_PADDING_MSB

			Passed on from 'RX_MSDU_END' TLV (only the MSB is reported
			 as the LSB is always zero)
			Number of bytes padded to make sure that the L3 header will
			 always start of a Dword boundary
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_OFFSET  0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_LSB     21
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MSB     21
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MASK    0x00200000


/* Description		TCP_UDP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the TCP/UDP header.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_OFFSET    0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_LSB       22
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MSB       22
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MASK      0x00400000


/* Description		IP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the IP header.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_OFFSET         0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_LSB            23
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MSB            23
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MASK           0x00800000


/* Description		FR_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'from DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_FR_DS_OFFSET                  0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_FR_DS_LSB                     24
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MSB                     24
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MASK                    0x01000000


/* Description		TO_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'to DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_TO_DS_OFFSET                  0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_TO_DS_LSB                     25
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MSB                     25
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MASK                    0x02000000


/* Description		INTRA_BSS

			This packet needs intra-BSS routing by SW as the 'vdev_id' 
			for the destination is the same as the 'vdev_id' (from 'RX_MPDU_PCU_START') 
			that this MSDU was got in.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_OFFSET              0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_LSB                 26
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MSB                 26
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MASK                0x04000000


/* Description		DEST_CHIP_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which chip's TCL the packet should be
			 queued.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_OFFSET           0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_LSB              27
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MSB              28
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MASK             0x18000000


/* Description		DECAP_FORMAT

			Indicates the format after decapsulation:
			
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_OFFSET           0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_LSB              29
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MSB              30
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MASK             0x60000000


/* Description		DEST_CHIP_PMAC_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which link/'vdev' the packet should 
			be queued in TCL.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_OFFSET      0x00000058
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_LSB         31
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MSB         31
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MASK        0x80000000


/* Description		RX_MSDU_EXT_DESC_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			Extended information related to the MSDU that is passed 
			on from RXDMA to REO but not part of the REO destination
			 ring. Some fields are passed on to PPE.
*/


/* Description		REO_DESTINATION_INDICATION

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			The ID of the REO exit ring where the MSDU frame shall push
			 after (MPDU level) reordering has finished.
			
			<enum 0 reo_destination_sw0> Reo will push the frame into
			 the REO2SW0 ring
			<enum 1 reo_destination_sw1> Reo will push the frame into
			 the REO2SW1 ring
			<enum 2 reo_destination_sw2> Reo will push the frame into
			 the REO2SW2 ring
			<enum 3 reo_destination_sw3> Reo will push the frame into
			 the REO2SW3 ring
			<enum 4 reo_destination_sw4> Reo will push the frame into
			 the REO2SW4 ring
			<enum 5 reo_destination_release> Reo will push the frame
			 into the REO_release ring
			<enum 6 reo_destination_fw> Reo will push the frame into
			 the REO2FW ring
			<enum 7 reo_destination_sw5> Reo will push the frame into
			 the REO2SW5 ring (REO remaps this in chips without REO2SW5
			 ring, e.g. Pine) 
			<enum 8 reo_destination_sw6> Reo will push the frame into
			 the REO2SW6 ring (REO remaps this in chips without REO2SW6
			 ring, e.g. Pine)
			 <enum 9 reo_destination_sw7> Reo will push the frame into
			 the REO2SW7 ring (REO remaps this in chips without REO2SW7
			 ring)
			<enum 10 reo_destination_sw8> Reo will push the frame into
			 the REO2SW8 ring (REO remaps this in chips without REO2SW8
			 ring)
			<enum 11 reo_destination_11> REO remaps this 
			<enum 12 reo_destination_12> REO remaps this <enum 13 reo_destination_13> 
			REO remaps this 
			<enum 14 reo_destination_14> REO remaps this 
			<enum 15 reo_destination_15> REO remaps this 
			<enum 16 reo_destination_16> REO remaps this 
			<enum 17 reo_destination_17> REO remaps this 
			<enum 18 reo_destination_18> REO remaps this 
			<enum 19 reo_destination_19> REO remaps this 
			<enum 20 reo_destination_20> REO remaps this 
			<enum 21 reo_destination_21> REO remaps this 
			<enum 22 reo_destination_22> REO remaps this 
			<enum 23 reo_destination_23> REO remaps this 
			<enum 24 reo_destination_24> REO remaps this 
			<enum 25 reo_destination_25> REO remaps this 
			<enum 26 reo_destination_26> REO remaps this 
			<enum 27 reo_destination_27> REO remaps this 
			<enum 28 reo_destination_28> REO remaps this 
			<enum 29 reo_destination_29> REO remaps this 
			<enum 30 reo_destination_30> REO remaps this 
			<enum 31 reo_destination_31> REO remaps this 
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_OFFSET 0x0000005c
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_LSB 0
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MSB 4
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MASK 0x0000001f


/* Description		SERVICE_CODE

			Opaque service code between PPE and Wi-Fi
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_OFFSET       0x0000005c
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_LSB          5
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_MSB          13
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_MASK         0x00003fe0


/* Description		PRIORITY_VALID

			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_OFFSET     0x0000005c
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_LSB        14
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_MSB        14
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_MASK       0x00004000


/* Description		DATA_OFFSET

			The offset to Rx packet data within the buffer (including
			 Rx DMA offset programming and L3 header padding inserted
			 by Rx OLE).
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_OFFSET        0x0000005c
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_LSB           15
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_MSB           26
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_MASK          0x07ff8000


/* Description		SRC_LINK_ID

			Consumer: SW
			Producer: RXDMA
			
			Set to the link ID of the PMAC that received the frame
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_OFFSET        0x0000005c
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_LSB           27
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_MSB           29
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_MASK          0x38000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_OFFSET        0x0000005c
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_LSB           30
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_MSB           31
#define RX_MSDU_LINK_MSDU_3_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_MASK          0xc0000000


/* Description		MSDU_4

			Details of next MSDU in this (MSDU flow) linked list
*/


/* Description		BUFFER_ADDR_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			Details of the physical address of the buffer containing
			 an MSDU (or entire MPDU)
*/


/* Description		BUFFER_ADDR_31_0

			Address (lower 32 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_OFFSET        0x00000060
#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_LSB           0
#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MSB           31
#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MASK          0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_OFFSET       0x00000064
#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_LSB          0
#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MSB          7
#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MASK         0x000000ff


/* Description		RETURN_BUFFER_MANAGER

			Consumer: WBM
			Producer: SW/FW
			
			In case of 'NULL' pointer, this field is set to 0
			
			Indicates to which buffer manager the buffer OR MSDU_EXTENSION
			 descriptor OR link descriptor that is being pointed to 
			shall be returned after the frame has been processed. It
			 is used by WBM for routing purposes.
			
			<enum 0 WBM_IDLE_BUF_LIST> This buffer shall be returned
			 to the WMB buffer idle list
			<enum 1 WBM_CHIP0_IDLE_DESC_LIST> This buffer shall be returned
			 to the WBM idle link descriptor idle list, where the chip
			 0 WBM is chosen in case of a multi-chip config
			<enum 2 WBM_CHIP1_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 1 WBM idle link descriptor idle list
			<enum 3 WBM_CHIP2_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 2 WBM idle link descriptor idle list
			<enum 12 WBM_CHIP3_IDLE_DESC_LIST> This buffer shall be 
			returned to chip 3 WBM idle link descriptor idle list
			<enum 4 FW_BM> This buffer shall be returned to the FW
			<enum 5 SW0_BM> This buffer shall be returned to the SW, 
			ring 0
			<enum 6 SW1_BM> This buffer shall be returned to the SW, 
			ring 1
			<enum 7 SW2_BM> This buffer shall be returned to the SW, 
			ring 2
			<enum 8 SW3_BM> This buffer shall be returned to the SW, 
			ring 3
			<enum 9 SW4_BM> This buffer shall be returned to the SW, 
			ring 4
			<enum 10 SW5_BM> This buffer shall be returned to the SW, 
			ring 5
			<enum 11 SW6_BM> This buffer shall be returned to the SW, 
			ring 6
			
			<legal 0-12>
*/

#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_OFFSET   0x00000064
#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_LSB      8
#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MSB      11
#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MASK     0x00000f00


/* Description		SW_BUFFER_COOKIE

			Cookie field exclusively used by SW. 
			
			In case of 'NULL' pointer, this field is set to 0
			
			HW ignores the contents, accept that it passes the programmed
			 value on to other descriptors together with the physical
			 address 
			
			Field can be used by SW to for example associate the buffers
			 physical address with the virtual address
			The bit definitions as used by SW are within SW HLD specification
			
			
			NOTE1:
			The three most significant bits can have a special meaning
			 in case this struct is embedded in a TX_MPDU_DETAILS STRUCT, 
			and field transmit_bw_restriction is set
			
			In case of NON punctured transmission:
			Sw_buffer_cookie[19:17] = 3'b000: 20 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b001: 40 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b010: 80 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b011: 160 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b101: 240 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b100: 320 MHz TX only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			In case of punctured transmission:
			Sw_buffer_cookie[19:16] = 4'b0000: pattern 0 only
			Sw_buffer_cookie[19:16] = 4'b0001: pattern 1 only
			Sw_buffer_cookie[19:16] = 4'b0010: pattern 2 only
			Sw_buffer_cookie[19:16] = 4'b0011: pattern 3 only
			Sw_buffer_cookie[19:16] = 4'b0100: pattern 4 only
			Sw_buffer_cookie[19:16] = 4'b0101: pattern 5 only
			Sw_buffer_cookie[19:16] = 4'b0110: pattern 6 only
			Sw_buffer_cookie[19:16] = 4'b0111: pattern 7 only
			Sw_buffer_cookie[19:16] = 4'b1000: pattern 8 only
			Sw_buffer_cookie[19:16] = 4'b1001: pattern 9 only
			Sw_buffer_cookie[19:16] = 4'b1010: pattern 10 only
			Sw_buffer_cookie[19:16] = 4'b1011: pattern 11 only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			Note: a punctured transmission is indicated by the presence
			 of TLV TX_PUNCTURE_SETUP embedded in the scheduler TLV
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_OFFSET        0x00000064
#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_LSB           12
#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MSB           31
#define RX_MSDU_LINK_MSDU_4_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MASK          0xfffff000


/* Description		RX_MSDU_DESC_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			General information related to the MSDU that should be passed
			 on from RXDMA all the way to to the REO destination ring.
			
*/


/* Description		FIRST_MSDU_IN_MPDU_FLAG

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU 
			
			<enum 0 Not_first_msdu> This is not the first MSDU in the
			 MPDU. 
			<enum 1 first_msdu> This MSDU is the first one in the MPDU.
			
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_LSB   0
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MSB   0
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MASK  0x00000001


/* Description		LAST_MSDU_IN_MPDU_FLAG

			Consumer: WBM/REO/SW/FW
			Producer: RXDMA
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			
			<enum 0 Not_last_msdu> There are more MSDUs linked to this
			 MSDU that belongs to this MPDU 
			<enum 1 Last_msdu> this MSDU is the last one in the MPDU. 
			This setting is only allowed in combination with 'Msdu_continuation' 
			set to 0. This implies that when an msdu is spread out over
			 multiple buffers and thus msdu_continuation is set, only
			 for the very last buffer of the msdu, can the 'last_msdu_in_mpdu_flag' 
			be set.
			
			When both first_msdu_in_mpdu_flag and last_msdu_in_mpdu_flag
			 are set, the MPDU that this MSDU belongs to only contains
			 a single MSDU.
			
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_LSB    1
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MSB    1
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MASK   0x00000002


/* Description		MSDU_CONTINUATION

			When set, this MSDU buffer was not able to hold the entire
			 MSDU. The next buffer will therefor contain additional 
			information related to this MSDU.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_OFFSET      0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_LSB         2
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MSB         2
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MASK        0x00000004


/* Description		MSDU_LENGTH

			Parsed from RX_MSDU_START TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the First
			 buffer used by MSDU.
			 
			Full MSDU length in bytes after decapsulation. 
			
			This field is still valid for MPDU frames without A-MSDU. 
			 It still represents MSDU length after decapsulation 
			
			Or in case of RAW MPDUs, it indicates the length of the 
			entire MPDU (without FCS field)
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_OFFSET            0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_LSB               3
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MSB               16
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MASK              0x0001fff8


/* Description		MSDU_DROP

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			When set, REO shall drop this MSDU and not forward it to
			 any other ring...
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_OFFSET              0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_LSB                 17
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MSB                 17
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MASK                0x00020000


/* Description		SA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid SA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_OFFSET            0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_LSB               18
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MSB               18
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MASK              0x00040000


/* Description		DA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid DA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_OFFSET            0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_LSB               19
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MSB               19
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MASK              0x00080000


/* Description		DA_IS_MCBC

			Field Only valid if "da_is_valid" is set
			
			Indicates the DA address was a Multicast of Broadcast address
			 for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_OFFSET             0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_LSB                20
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MSB                20
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MASK               0x00100000


/* Description		L3_HEADER_PADDING_MSB

			Passed on from 'RX_MSDU_END' TLV (only the MSB is reported
			 as the LSB is always zero)
			Number of bytes padded to make sure that the L3 header will
			 always start of a Dword boundary
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_OFFSET  0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_LSB     21
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MSB     21
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MASK    0x00200000


/* Description		TCP_UDP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the TCP/UDP header.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_OFFSET    0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_LSB       22
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MSB       22
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MASK      0x00400000


/* Description		IP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the IP header.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_OFFSET         0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_LSB            23
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MSB            23
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MASK           0x00800000


/* Description		FR_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'from DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_FR_DS_OFFSET                  0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_FR_DS_LSB                     24
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MSB                     24
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MASK                    0x01000000


/* Description		TO_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'to DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_TO_DS_OFFSET                  0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_TO_DS_LSB                     25
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MSB                     25
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MASK                    0x02000000


/* Description		INTRA_BSS

			This packet needs intra-BSS routing by SW as the 'vdev_id' 
			for the destination is the same as the 'vdev_id' (from 'RX_MPDU_PCU_START') 
			that this MSDU was got in.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_OFFSET              0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_LSB                 26
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MSB                 26
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MASK                0x04000000


/* Description		DEST_CHIP_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which chip's TCL the packet should be
			 queued.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_OFFSET           0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_LSB              27
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MSB              28
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MASK             0x18000000


/* Description		DECAP_FORMAT

			Indicates the format after decapsulation:
			
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_OFFSET           0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_LSB              29
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MSB              30
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MASK             0x60000000


/* Description		DEST_CHIP_PMAC_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which link/'vdev' the packet should 
			be queued in TCL.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_OFFSET      0x00000068
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_LSB         31
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MSB         31
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MASK        0x80000000


/* Description		RX_MSDU_EXT_DESC_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			Extended information related to the MSDU that is passed 
			on from RXDMA to REO but not part of the REO destination
			 ring. Some fields are passed on to PPE.
*/


/* Description		REO_DESTINATION_INDICATION

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			The ID of the REO exit ring where the MSDU frame shall push
			 after (MPDU level) reordering has finished.
			
			<enum 0 reo_destination_sw0> Reo will push the frame into
			 the REO2SW0 ring
			<enum 1 reo_destination_sw1> Reo will push the frame into
			 the REO2SW1 ring
			<enum 2 reo_destination_sw2> Reo will push the frame into
			 the REO2SW2 ring
			<enum 3 reo_destination_sw3> Reo will push the frame into
			 the REO2SW3 ring
			<enum 4 reo_destination_sw4> Reo will push the frame into
			 the REO2SW4 ring
			<enum 5 reo_destination_release> Reo will push the frame
			 into the REO_release ring
			<enum 6 reo_destination_fw> Reo will push the frame into
			 the REO2FW ring
			<enum 7 reo_destination_sw5> Reo will push the frame into
			 the REO2SW5 ring (REO remaps this in chips without REO2SW5
			 ring, e.g. Pine) 
			<enum 8 reo_destination_sw6> Reo will push the frame into
			 the REO2SW6 ring (REO remaps this in chips without REO2SW6
			 ring, e.g. Pine)
			 <enum 9 reo_destination_sw7> Reo will push the frame into
			 the REO2SW7 ring (REO remaps this in chips without REO2SW7
			 ring)
			<enum 10 reo_destination_sw8> Reo will push the frame into
			 the REO2SW8 ring (REO remaps this in chips without REO2SW8
			 ring)
			<enum 11 reo_destination_11> REO remaps this 
			<enum 12 reo_destination_12> REO remaps this <enum 13 reo_destination_13> 
			REO remaps this 
			<enum 14 reo_destination_14> REO remaps this 
			<enum 15 reo_destination_15> REO remaps this 
			<enum 16 reo_destination_16> REO remaps this 
			<enum 17 reo_destination_17> REO remaps this 
			<enum 18 reo_destination_18> REO remaps this 
			<enum 19 reo_destination_19> REO remaps this 
			<enum 20 reo_destination_20> REO remaps this 
			<enum 21 reo_destination_21> REO remaps this 
			<enum 22 reo_destination_22> REO remaps this 
			<enum 23 reo_destination_23> REO remaps this 
			<enum 24 reo_destination_24> REO remaps this 
			<enum 25 reo_destination_25> REO remaps this 
			<enum 26 reo_destination_26> REO remaps this 
			<enum 27 reo_destination_27> REO remaps this 
			<enum 28 reo_destination_28> REO remaps this 
			<enum 29 reo_destination_29> REO remaps this 
			<enum 30 reo_destination_30> REO remaps this 
			<enum 31 reo_destination_31> REO remaps this 
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_OFFSET 0x0000006c
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_LSB 0
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MSB 4
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MASK 0x0000001f


/* Description		SERVICE_CODE

			Opaque service code between PPE and Wi-Fi
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_OFFSET       0x0000006c
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_LSB          5
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_MSB          13
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_MASK         0x00003fe0


/* Description		PRIORITY_VALID

			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_OFFSET     0x0000006c
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_LSB        14
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_MSB        14
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_MASK       0x00004000


/* Description		DATA_OFFSET

			The offset to Rx packet data within the buffer (including
			 Rx DMA offset programming and L3 header padding inserted
			 by Rx OLE).
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_OFFSET        0x0000006c
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_LSB           15
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_MSB           26
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_MASK          0x07ff8000


/* Description		SRC_LINK_ID

			Consumer: SW
			Producer: RXDMA
			
			Set to the link ID of the PMAC that received the frame
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_OFFSET        0x0000006c
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_LSB           27
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_MSB           29
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_MASK          0x38000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_OFFSET        0x0000006c
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_LSB           30
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_MSB           31
#define RX_MSDU_LINK_MSDU_4_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_MASK          0xc0000000


/* Description		MSDU_5

			Details of next MSDU in this (MSDU flow) linked list
*/


/* Description		BUFFER_ADDR_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			Details of the physical address of the buffer containing
			 an MSDU (or entire MPDU)
*/


/* Description		BUFFER_ADDR_31_0

			Address (lower 32 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_OFFSET        0x00000070
#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_LSB           0
#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MSB           31
#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MASK          0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_OFFSET       0x00000074
#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_LSB          0
#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MSB          7
#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MASK         0x000000ff


/* Description		RETURN_BUFFER_MANAGER

			Consumer: WBM
			Producer: SW/FW
			
			In case of 'NULL' pointer, this field is set to 0
			
			Indicates to which buffer manager the buffer OR MSDU_EXTENSION
			 descriptor OR link descriptor that is being pointed to 
			shall be returned after the frame has been processed. It
			 is used by WBM for routing purposes.
			
			<enum 0 WBM_IDLE_BUF_LIST> This buffer shall be returned
			 to the WMB buffer idle list
			<enum 1 WBM_CHIP0_IDLE_DESC_LIST> This buffer shall be returned
			 to the WBM idle link descriptor idle list, where the chip
			 0 WBM is chosen in case of a multi-chip config
			<enum 2 WBM_CHIP1_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 1 WBM idle link descriptor idle list
			<enum 3 WBM_CHIP2_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 2 WBM idle link descriptor idle list
			<enum 12 WBM_CHIP3_IDLE_DESC_LIST> This buffer shall be 
			returned to chip 3 WBM idle link descriptor idle list
			<enum 4 FW_BM> This buffer shall be returned to the FW
			<enum 5 SW0_BM> This buffer shall be returned to the SW, 
			ring 0
			<enum 6 SW1_BM> This buffer shall be returned to the SW, 
			ring 1
			<enum 7 SW2_BM> This buffer shall be returned to the SW, 
			ring 2
			<enum 8 SW3_BM> This buffer shall be returned to the SW, 
			ring 3
			<enum 9 SW4_BM> This buffer shall be returned to the SW, 
			ring 4
			<enum 10 SW5_BM> This buffer shall be returned to the SW, 
			ring 5
			<enum 11 SW6_BM> This buffer shall be returned to the SW, 
			ring 6
			
			<legal 0-12>
*/

#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_OFFSET   0x00000074
#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_LSB      8
#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MSB      11
#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MASK     0x00000f00


/* Description		SW_BUFFER_COOKIE

			Cookie field exclusively used by SW. 
			
			In case of 'NULL' pointer, this field is set to 0
			
			HW ignores the contents, accept that it passes the programmed
			 value on to other descriptors together with the physical
			 address 
			
			Field can be used by SW to for example associate the buffers
			 physical address with the virtual address
			The bit definitions as used by SW are within SW HLD specification
			
			
			NOTE1:
			The three most significant bits can have a special meaning
			 in case this struct is embedded in a TX_MPDU_DETAILS STRUCT, 
			and field transmit_bw_restriction is set
			
			In case of NON punctured transmission:
			Sw_buffer_cookie[19:17] = 3'b000: 20 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b001: 40 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b010: 80 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b011: 160 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b101: 240 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b100: 320 MHz TX only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			In case of punctured transmission:
			Sw_buffer_cookie[19:16] = 4'b0000: pattern 0 only
			Sw_buffer_cookie[19:16] = 4'b0001: pattern 1 only
			Sw_buffer_cookie[19:16] = 4'b0010: pattern 2 only
			Sw_buffer_cookie[19:16] = 4'b0011: pattern 3 only
			Sw_buffer_cookie[19:16] = 4'b0100: pattern 4 only
			Sw_buffer_cookie[19:16] = 4'b0101: pattern 5 only
			Sw_buffer_cookie[19:16] = 4'b0110: pattern 6 only
			Sw_buffer_cookie[19:16] = 4'b0111: pattern 7 only
			Sw_buffer_cookie[19:16] = 4'b1000: pattern 8 only
			Sw_buffer_cookie[19:16] = 4'b1001: pattern 9 only
			Sw_buffer_cookie[19:16] = 4'b1010: pattern 10 only
			Sw_buffer_cookie[19:16] = 4'b1011: pattern 11 only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			Note: a punctured transmission is indicated by the presence
			 of TLV TX_PUNCTURE_SETUP embedded in the scheduler TLV
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_OFFSET        0x00000074
#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_LSB           12
#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MSB           31
#define RX_MSDU_LINK_MSDU_5_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MASK          0xfffff000


/* Description		RX_MSDU_DESC_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			General information related to the MSDU that should be passed
			 on from RXDMA all the way to to the REO destination ring.
			
*/


/* Description		FIRST_MSDU_IN_MPDU_FLAG

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU 
			
			<enum 0 Not_first_msdu> This is not the first MSDU in the
			 MPDU. 
			<enum 1 first_msdu> This MSDU is the first one in the MPDU.
			
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_LSB   0
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MSB   0
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MASK  0x00000001


/* Description		LAST_MSDU_IN_MPDU_FLAG

			Consumer: WBM/REO/SW/FW
			Producer: RXDMA
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			
			<enum 0 Not_last_msdu> There are more MSDUs linked to this
			 MSDU that belongs to this MPDU 
			<enum 1 Last_msdu> this MSDU is the last one in the MPDU. 
			This setting is only allowed in combination with 'Msdu_continuation' 
			set to 0. This implies that when an msdu is spread out over
			 multiple buffers and thus msdu_continuation is set, only
			 for the very last buffer of the msdu, can the 'last_msdu_in_mpdu_flag' 
			be set.
			
			When both first_msdu_in_mpdu_flag and last_msdu_in_mpdu_flag
			 are set, the MPDU that this MSDU belongs to only contains
			 a single MSDU.
			
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_LSB    1
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MSB    1
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MASK   0x00000002


/* Description		MSDU_CONTINUATION

			When set, this MSDU buffer was not able to hold the entire
			 MSDU. The next buffer will therefor contain additional 
			information related to this MSDU.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_OFFSET      0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_LSB         2
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MSB         2
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MASK        0x00000004


/* Description		MSDU_LENGTH

			Parsed from RX_MSDU_START TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the First
			 buffer used by MSDU.
			 
			Full MSDU length in bytes after decapsulation. 
			
			This field is still valid for MPDU frames without A-MSDU. 
			 It still represents MSDU length after decapsulation 
			
			Or in case of RAW MPDUs, it indicates the length of the 
			entire MPDU (without FCS field)
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_OFFSET            0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_LSB               3
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MSB               16
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MASK              0x0001fff8


/* Description		MSDU_DROP

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			When set, REO shall drop this MSDU and not forward it to
			 any other ring...
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_OFFSET              0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_LSB                 17
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MSB                 17
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MASK                0x00020000


/* Description		SA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid SA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_OFFSET            0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_LSB               18
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MSB               18
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MASK              0x00040000


/* Description		DA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid DA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_OFFSET            0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_LSB               19
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MSB               19
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MASK              0x00080000


/* Description		DA_IS_MCBC

			Field Only valid if "da_is_valid" is set
			
			Indicates the DA address was a Multicast of Broadcast address
			 for this MSDU
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_OFFSET             0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_LSB                20
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MSB                20
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MASK               0x00100000


/* Description		L3_HEADER_PADDING_MSB

			Passed on from 'RX_MSDU_END' TLV (only the MSB is reported
			 as the LSB is always zero)
			Number of bytes padded to make sure that the L3 header will
			 always start of a Dword boundary
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_OFFSET  0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_LSB     21
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MSB     21
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MASK    0x00200000


/* Description		TCP_UDP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the TCP/UDP header.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_OFFSET    0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_LSB       22
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MSB       22
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MASK      0x00400000


/* Description		IP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the IP header.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_OFFSET         0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_LSB            23
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MSB            23
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MASK           0x00800000


/* Description		FR_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'from DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_FR_DS_OFFSET                  0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_FR_DS_LSB                     24
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MSB                     24
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MASK                    0x01000000


/* Description		TO_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'to DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_TO_DS_OFFSET                  0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_TO_DS_LSB                     25
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MSB                     25
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MASK                    0x02000000


/* Description		INTRA_BSS

			This packet needs intra-BSS routing by SW as the 'vdev_id' 
			for the destination is the same as the 'vdev_id' (from 'RX_MPDU_PCU_START') 
			that this MSDU was got in.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_OFFSET              0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_LSB                 26
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MSB                 26
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MASK                0x04000000


/* Description		DEST_CHIP_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which chip's TCL the packet should be
			 queued.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_OFFSET           0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_LSB              27
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MSB              28
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MASK             0x18000000


/* Description		DECAP_FORMAT

			Indicates the format after decapsulation:
			
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_OFFSET           0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_LSB              29
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MSB              30
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MASK             0x60000000


/* Description		DEST_CHIP_PMAC_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which link/'vdev' the packet should 
			be queued in TCL.
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_OFFSET      0x00000078
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_LSB         31
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MSB         31
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MASK        0x80000000


/* Description		RX_MSDU_EXT_DESC_INFO_DETAILS

			Consumer: REO/SW
			Producer: RXDMA
			
			Extended information related to the MSDU that is passed 
			on from RXDMA to REO but not part of the REO destination
			 ring. Some fields are passed on to PPE.
*/


/* Description		REO_DESTINATION_INDICATION

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			The ID of the REO exit ring where the MSDU frame shall push
			 after (MPDU level) reordering has finished.
			
			<enum 0 reo_destination_sw0> Reo will push the frame into
			 the REO2SW0 ring
			<enum 1 reo_destination_sw1> Reo will push the frame into
			 the REO2SW1 ring
			<enum 2 reo_destination_sw2> Reo will push the frame into
			 the REO2SW2 ring
			<enum 3 reo_destination_sw3> Reo will push the frame into
			 the REO2SW3 ring
			<enum 4 reo_destination_sw4> Reo will push the frame into
			 the REO2SW4 ring
			<enum 5 reo_destination_release> Reo will push the frame
			 into the REO_release ring
			<enum 6 reo_destination_fw> Reo will push the frame into
			 the REO2FW ring
			<enum 7 reo_destination_sw5> Reo will push the frame into
			 the REO2SW5 ring (REO remaps this in chips without REO2SW5
			 ring, e.g. Pine) 
			<enum 8 reo_destination_sw6> Reo will push the frame into
			 the REO2SW6 ring (REO remaps this in chips without REO2SW6
			 ring, e.g. Pine)
			 <enum 9 reo_destination_sw7> Reo will push the frame into
			 the REO2SW7 ring (REO remaps this in chips without REO2SW7
			 ring)
			<enum 10 reo_destination_sw8> Reo will push the frame into
			 the REO2SW8 ring (REO remaps this in chips without REO2SW8
			 ring)
			<enum 11 reo_destination_11> REO remaps this 
			<enum 12 reo_destination_12> REO remaps this <enum 13 reo_destination_13> 
			REO remaps this 
			<enum 14 reo_destination_14> REO remaps this 
			<enum 15 reo_destination_15> REO remaps this 
			<enum 16 reo_destination_16> REO remaps this 
			<enum 17 reo_destination_17> REO remaps this 
			<enum 18 reo_destination_18> REO remaps this 
			<enum 19 reo_destination_19> REO remaps this 
			<enum 20 reo_destination_20> REO remaps this 
			<enum 21 reo_destination_21> REO remaps this 
			<enum 22 reo_destination_22> REO remaps this 
			<enum 23 reo_destination_23> REO remaps this 
			<enum 24 reo_destination_24> REO remaps this 
			<enum 25 reo_destination_25> REO remaps this 
			<enum 26 reo_destination_26> REO remaps this 
			<enum 27 reo_destination_27> REO remaps this 
			<enum 28 reo_destination_28> REO remaps this 
			<enum 29 reo_destination_29> REO remaps this 
			<enum 30 reo_destination_30> REO remaps this 
			<enum 31 reo_destination_31> REO remaps this 
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_OFFSET 0x0000007c
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_LSB 0
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MSB 4
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MASK 0x0000001f


/* Description		SERVICE_CODE

			Opaque service code between PPE and Wi-Fi
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_OFFSET       0x0000007c
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_LSB          5
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_MSB          13
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_SERVICE_CODE_MASK         0x00003fe0


/* Description		PRIORITY_VALID

			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_OFFSET     0x0000007c
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_LSB        14
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_MSB        14
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_PRIORITY_VALID_MASK       0x00004000


/* Description		DATA_OFFSET

			The offset to Rx packet data within the buffer (including
			 Rx DMA offset programming and L3 header padding inserted
			 by Rx OLE).
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_OFFSET        0x0000007c
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_LSB           15
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_MSB           26
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_DATA_OFFSET_MASK          0x07ff8000


/* Description		SRC_LINK_ID

			Consumer: SW
			Producer: RXDMA
			
			Set to the link ID of the PMAC that received the frame
			<legal all>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_OFFSET        0x0000007c
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_LSB           27
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_MSB           29
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_SRC_LINK_ID_MASK          0x38000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_OFFSET        0x0000007c
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_LSB           30
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_MSB           31
#define RX_MSDU_LINK_MSDU_5_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_MASK          0xc0000000



#endif   // RX_MSDU_LINK
