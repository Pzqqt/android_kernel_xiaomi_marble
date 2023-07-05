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

#ifndef _RX_MPDU_DETAILS_H_
#define _RX_MPDU_DETAILS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "rx_mpdu_desc_info.h"
#include "buffer_addr_info.h"
#define NUM_OF_DWORDS_RX_MPDU_DETAILS 4


struct rx_mpdu_details {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   buffer_addr_info                                          msdu_link_desc_addr_info;
             struct   rx_mpdu_desc_info                                         rx_mpdu_desc_info_details;
#else
             struct   buffer_addr_info                                          msdu_link_desc_addr_info;
             struct   rx_mpdu_desc_info                                         rx_mpdu_desc_info_details;
#endif
};


/* Description		MSDU_LINK_DESC_ADDR_INFO

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			Details of the physical address of the MSDU link descriptor
			 that contains pointers to MSDUs related to this MPDU
*/


/* Description		BUFFER_ADDR_31_0

			Address (lower 32 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET            0x00000000
#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB               0
#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MSB               31
#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK              0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET           0x00000004
#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB              0
#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MSB              7
#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK             0x000000ff


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

#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET       0x00000004
#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB          8
#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB          11
#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK         0x00000f00


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

#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET            0x00000004
#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB               12
#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MSB               31
#define RX_MPDU_DETAILS_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK              0xfffff000


/* Description		RX_MPDU_DESC_INFO_DETAILS

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			General information related to the MPDU that should be passed
			 on from REO entrance ring to the REO destination ring
*/


/* Description		MSDU_COUNT

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			The number of MSDUs within the MPDU 
			<legal all>
*/

#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET                 0x00000008
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_LSB                    0
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MSB                    7
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MASK                   0x000000ff


/* Description		FRAGMENT_FLAG

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			When set, this MPDU is a fragment and REO should forward
			 this fragment MPDU to the REO destination ring without 
			any reorder checks, pn checks or bitmap update. This implies
			 that REO is forwarding the pointer to the MSDU link descriptor. 
			The destination ring is coming from a programmable register
			 setting in REO
			
			<legal all>
*/

#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_OFFSET              0x00000008
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_LSB                 8
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MSB                 8
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MASK                0x00000100


/* Description		MPDU_RETRY_BIT

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			The retry bit setting from the MPDU header of the received
			 frame
			<legal all>
*/

#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_OFFSET             0x00000008
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_LSB                9
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MSB                9
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MASK               0x00000200


/* Description		AMPDU_FLAG

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			When set, the MPDU was received as part of an A-MPDU.
			<legal all>
*/

#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_OFFSET                 0x00000008
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_LSB                    10
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MSB                    10
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MASK                   0x00000400


/* Description		BAR_FRAME

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			When set, the received frame is a BAR frame. After processing, 
			this frame shall be pushed to SW or deleted.
			<legal all>
*/

#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_OFFSET                  0x00000008
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_LSB                     11
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MSB                     11
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MASK                    0x00000800


/* Description		PN_FIELDS_CONTAIN_VALID_INFO

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			Copied here by RXDMA from RX_MPDU_END
			When not set, REO will Not perform a PN sequence number 
			check
*/

#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_OFFSET 0x00000008
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_LSB  12
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MSB  12
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MASK 0x00001000


/* Description		RAW_MPDU

			Field only valid when first_msdu_in_mpdu_flag is set.
			
			When set, the contents in the MSDU buffer contains a 'RAW' 
			MPDU. This 'RAW' MPDU might be spread out over multiple 
			MSDU buffers.
			<legal all>
*/

#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_OFFSET                   0x00000008
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_LSB                      13
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MSB                      13
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MASK                     0x00002000


/* Description		MORE_FRAGMENT_FLAG

			The More Fragment bit setting from the MPDU header of the
			 received frame
			
			<legal all>
*/

#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_OFFSET         0x00000008
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_LSB            14
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MSB            14
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MASK           0x00004000


/* Description		SRC_INFO

			Source (virtual) device/interface info. associated with 
			this peer
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_OFFSET                   0x00000008
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_LSB                      15
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_MSB                      26
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_MASK                     0x07ff8000


/* Description		MPDU_QOS_CONTROL_VALID

			When set, the MPDU has a QoS control field.
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_OFFSET     0x00000008
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_LSB        27
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_MSB        27
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_MASK       0x08000000


/* Description		TID

			Field only valid when mpdu_qos_control_valid is set
			
			The TID field in the QoS control field
			<legal all>
*/

#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_TID_OFFSET                        0x00000008
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_TID_LSB                           28
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_TID_MSB                           31
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_TID_MASK                          0xf0000000


/* Description		PEER_META_DATA

			Meta data that SW has programmed in the Peer table entry
			 of the transmitting STA.
			<legal all>
*/

#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_OFFSET             0x0000000c
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_LSB                0
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MSB                31
#define RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MASK               0xffffffff



#endif   // RX_MPDU_DETAILS
