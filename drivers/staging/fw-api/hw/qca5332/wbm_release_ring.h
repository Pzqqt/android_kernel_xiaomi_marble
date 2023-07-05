
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

 
 
 
 
 
 
 


#ifndef _WBM_RELEASE_RING_H_
#define _WBM_RELEASE_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#include "buffer_addr_info.h"
#define NUM_OF_DWORDS_WBM_RELEASE_RING 8


struct wbm_release_ring {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   buffer_addr_info                                          released_buff_or_desc_addr_info;
             uint32_t release_source_module                                   :  3, // [2:0]
                      reserved_2a                                             :  3, // [5:3]
                      buffer_or_desc_type                                     :  3, // [8:6]
                      reserved_2b                                             : 22, // [30:9]
                      wbm_internal_error                                      :  1; // [31:31]
             uint32_t reserved_3a                                             : 32; // [31:0]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 28, // [27:0]
                      looping_count                                           :  4; // [31:28]
#else
             struct   buffer_addr_info                                          released_buff_or_desc_addr_info;
             uint32_t wbm_internal_error                                      :  1, // [31:31]
                      reserved_2b                                             : 22, // [30:9]
                      buffer_or_desc_type                                     :  3, // [8:6]
                      reserved_2a                                             :  3, // [5:3]
                      release_source_module                                   :  3; // [2:0]
             uint32_t reserved_3a                                             : 32; // [31:0]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t looping_count                                           :  4, // [31:28]
                      reserved_7a                                             : 28; // [27:0]
#endif
};


/* Description		RELEASED_BUFF_OR_DESC_ADDR_INFO

			DO NOT USE. This may be a 'BUFFER_ADDR_INFO' structure or
			 a 64-bit virtual address.
*/


/* Description		BUFFER_ADDR_31_0

			Address (lower 32 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET    0x00000000
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB       0
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MSB       31
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK      0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET   0x00000004
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB      0
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MSB      7
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK     0x000000ff


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

#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET 0x00000004
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB  8
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB  11
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK 0x00000f00


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

#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET    0x00000004
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB       12
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MSB       31
#define WBM_RELEASE_RING_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK      0xfffff000


/* Description		RELEASE_SOURCE_MODULE

			Indicates which module initiated the release of this buffer
			 or descriptor
			
			<enum 1 release_source_RXDMA> RXDMA released this buffer
			 or descriptor
			<enum 2 release_source_REO> REO released this buffer or 
			descriptor
			<enum 5 release_source_FW_RX> FW released this buffer or
			 descriptor
			<enum 4 release_source_SW_RX> SW released this buffer or
			 descriptor
			<enum 0 release_source_TQM> DO NOT USE
			<enum 3 release_source_FW_TX> DO NOT USE
			<enum 6 release_source_SW_TX> DO NOT USE
			<legal 0-6>
*/

#define WBM_RELEASE_RING_RELEASE_SOURCE_MODULE_OFFSET                               0x00000008
#define WBM_RELEASE_RING_RELEASE_SOURCE_MODULE_LSB                                  0
#define WBM_RELEASE_RING_RELEASE_SOURCE_MODULE_MSB                                  2
#define WBM_RELEASE_RING_RELEASE_SOURCE_MODULE_MASK                                 0x00000007


/* Description		RESERVED_2A

			This could be different fields depending on the structure.
			
			<legal all>
*/

#define WBM_RELEASE_RING_RESERVED_2A_OFFSET                                         0x00000008
#define WBM_RELEASE_RING_RESERVED_2A_LSB                                            3
#define WBM_RELEASE_RING_RESERVED_2A_MSB                                            5
#define WBM_RELEASE_RING_RESERVED_2A_MASK                                           0x00000038


/* Description		BUFFER_OR_DESC_TYPE

			Consumer: WBM/SW/FW
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			Field only valid when WBM is marked as the return_buffer_manager
			 in the Released_Buffer_address_info
			
			Indicates that type of buffer or descriptor is being released
			
			
			<enum 0 MSDU_rel_buffer> The address points to an MSDU buffer
			 
			<enum 1 msdu_link_descriptor> The address points to an TX
			 MSDU link descriptor
			<enum 2 mpdu_link_descriptor> The address points to an MPDU
			 link descriptor
			<enum 3 msdu_ext_descriptor > The address points to an MSDU
			 extension descriptor.
			In case BM finds this one in a release ring, it passes it
			 on to FW...
			<enum 4 queue_ext_descriptor> The address points to an TQM
			 queue extension descriptor. WBM should treat this is the
			 same way as a link descriptor. That is, put the 128 byte
			 buffer back in the link buffer idle list.
			
			TODO: Any restrictions?
			<legal 0-4>
*/

#define WBM_RELEASE_RING_BUFFER_OR_DESC_TYPE_OFFSET                                 0x00000008
#define WBM_RELEASE_RING_BUFFER_OR_DESC_TYPE_LSB                                    6
#define WBM_RELEASE_RING_BUFFER_OR_DESC_TYPE_MSB                                    8
#define WBM_RELEASE_RING_BUFFER_OR_DESC_TYPE_MASK                                   0x000001c0


/* Description		RESERVED_2B

			This could be different fields depending on the structure.
			
			<legal all>
*/

#define WBM_RELEASE_RING_RESERVED_2B_OFFSET                                         0x00000008
#define WBM_RELEASE_RING_RESERVED_2B_LSB                                            9
#define WBM_RELEASE_RING_RESERVED_2B_MSB                                            30
#define WBM_RELEASE_RING_RESERVED_2B_MASK                                           0x7ffffe00


/* Description		WBM_INTERNAL_ERROR

			Can only be set by WBM.
			
			Is set when WBM got a buffer pointer but the action was 
			to push it to the idle link descriptor ring or do link related
			 activity
			OR
			Is set when WBM got a link buffer pointer but the action
			 was to push it to the buffer  descriptor ring 
			
			<legal all>
*/

#define WBM_RELEASE_RING_WBM_INTERNAL_ERROR_OFFSET                                  0x00000008
#define WBM_RELEASE_RING_WBM_INTERNAL_ERROR_LSB                                     31
#define WBM_RELEASE_RING_WBM_INTERNAL_ERROR_MSB                                     31
#define WBM_RELEASE_RING_WBM_INTERNAL_ERROR_MASK                                    0x80000000


/* Description		RESERVED_3A

			This could be different fields depending on the structure.
			
			<legal all>
*/

#define WBM_RELEASE_RING_RESERVED_3A_OFFSET                                         0x0000000c
#define WBM_RELEASE_RING_RESERVED_3A_LSB                                            0
#define WBM_RELEASE_RING_RESERVED_3A_MSB                                            31
#define WBM_RELEASE_RING_RESERVED_3A_MASK                                           0xffffffff


/* Description		RESERVED_4A

			This could be different fields depending on the structure.
			
			<legal all>
*/

#define WBM_RELEASE_RING_RESERVED_4A_OFFSET                                         0x00000010
#define WBM_RELEASE_RING_RESERVED_4A_LSB                                            0
#define WBM_RELEASE_RING_RESERVED_4A_MSB                                            31
#define WBM_RELEASE_RING_RESERVED_4A_MASK                                           0xffffffff


/* Description		RESERVED_5A

			This could be different fields depending on the structure.
			
			<legal all>
*/

#define WBM_RELEASE_RING_RESERVED_5A_OFFSET                                         0x00000014
#define WBM_RELEASE_RING_RESERVED_5A_LSB                                            0
#define WBM_RELEASE_RING_RESERVED_5A_MSB                                            31
#define WBM_RELEASE_RING_RESERVED_5A_MASK                                           0xffffffff


/* Description		RESERVED_6A

			This could be different fields depending on the structure.
			
			<legal all>
*/

#define WBM_RELEASE_RING_RESERVED_6A_OFFSET                                         0x00000018
#define WBM_RELEASE_RING_RESERVED_6A_LSB                                            0
#define WBM_RELEASE_RING_RESERVED_6A_MSB                                            31
#define WBM_RELEASE_RING_RESERVED_6A_MASK                                           0xffffffff


/* Description		RESERVED_7A

			This could be different fields depending on the structure.
			
			<legal all>
*/

#define WBM_RELEASE_RING_RESERVED_7A_OFFSET                                         0x0000001c
#define WBM_RELEASE_RING_RESERVED_7A_LSB                                            0
#define WBM_RELEASE_RING_RESERVED_7A_MSB                                            27
#define WBM_RELEASE_RING_RESERVED_7A_MASK                                           0x0fffffff


/* Description		LOOPING_COUNT

			Consumer: WBM/SW/FW
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			If WBM_internal_error is set, this descriptor is sent to
			 the dedicated 'WBM_ERROR_RELEASE' ring and Looping_count
			 is used to indicate an error code.
			
			The values reported are documented further in the WBM MLD
			 doc.
			
			If WBM_internal_error is not set, the following holds.
			
			A count value that indicates the number of times the producer
			 of entries into the Buffer Manager Ring has looped around
			 the ring.
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

#define WBM_RELEASE_RING_LOOPING_COUNT_OFFSET                                       0x0000001c
#define WBM_RELEASE_RING_LOOPING_COUNT_LSB                                          28
#define WBM_RELEASE_RING_LOOPING_COUNT_MSB                                          31
#define WBM_RELEASE_RING_LOOPING_COUNT_MASK                                         0xf0000000



#endif   // WBM_RELEASE_RING
