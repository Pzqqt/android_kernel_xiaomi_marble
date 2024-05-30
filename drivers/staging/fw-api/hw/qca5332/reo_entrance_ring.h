
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

 
 
 
 
 
 
 


#ifndef _REO_ENTRANCE_RING_H_
#define _REO_ENTRANCE_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#include "rx_mpdu_details.h"
#define NUM_OF_DWORDS_REO_ENTRANCE_RING 8


struct reo_entrance_ring {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   rx_mpdu_details                                           reo_level_mpdu_frame_info;
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32; // [31:0]
             uint32_t rx_reo_queue_desc_addr_39_32                            :  8, // [7:0]
                      rounded_mpdu_byte_count                                 : 14, // [21:8]
                      reo_destination_indication                              :  5, // [26:22]
                      frameless_bar                                           :  1, // [27:27]
                      reserved_5a                                             :  4; // [31:28]
             uint32_t rxdma_push_reason                                       :  2, // [1:0]
                      rxdma_error_code                                        :  5, // [6:2]
                      mpdu_fragment_number                                    :  4, // [10:7]
                      sw_exception                                            :  1, // [11:11]
                      sw_exception_mpdu_delink                                :  1, // [12:12]
                      sw_exception_destination_ring_valid                     :  1, // [13:13]
                      sw_exception_destination_ring                           :  5, // [18:14]
                      mpdu_sequence_number                                    : 12, // [30:19]
                      reserved_6a                                             :  1; // [31:31]
             uint32_t phy_ppdu_id                                             : 16, // [15:0]
                      src_link_id                                             :  3, // [18:16]
                      reserved_7a                                             :  1, // [19:19]
                      ring_id                                                 :  8, // [27:20]
                      looping_count                                           :  4; // [31:28]
#else
             struct   rx_mpdu_details                                           reo_level_mpdu_frame_info;
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32; // [31:0]
             uint32_t reserved_5a                                             :  4, // [31:28]
                      frameless_bar                                           :  1, // [27:27]
                      reo_destination_indication                              :  5, // [26:22]
                      rounded_mpdu_byte_count                                 : 14, // [21:8]
                      rx_reo_queue_desc_addr_39_32                            :  8; // [7:0]
             uint32_t reserved_6a                                             :  1, // [31:31]
                      mpdu_sequence_number                                    : 12, // [30:19]
                      sw_exception_destination_ring                           :  5, // [18:14]
                      sw_exception_destination_ring_valid                     :  1, // [13:13]
                      sw_exception_mpdu_delink                                :  1, // [12:12]
                      sw_exception                                            :  1, // [11:11]
                      mpdu_fragment_number                                    :  4, // [10:7]
                      rxdma_error_code                                        :  5, // [6:2]
                      rxdma_push_reason                                       :  2; // [1:0]
             uint32_t looping_count                                           :  4, // [31:28]
                      ring_id                                                 :  8, // [27:20]
                      reserved_7a                                             :  1, // [19:19]
                      src_link_id                                             :  3, // [18:16]
                      phy_ppdu_id                                             : 16; // [15:0]
#endif
};


/* Description		REO_LEVEL_MPDU_FRAME_INFO

			Consumer: REO
			Producer: RXDMA
			
			Details related to the MPDU being pushed into the REO
*/


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

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET 0x00000000
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB 0
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MSB 31
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK 0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET 0x00000004
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB 0
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MSB 7
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK 0x000000ff


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

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET 0x00000004
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB 8
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB 11
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK 0x00000f00


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

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET 0x00000004
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB 12
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MSB 31
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_MSDU_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK 0xfffff000


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

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_LSB 0
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MSB 7
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MASK 0x000000ff


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

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_LSB 8
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MSB 8
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MASK 0x00000100


/* Description		MPDU_RETRY_BIT

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			The retry bit setting from the MPDU header of the received
			 frame
			<legal all>
*/

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_LSB 9
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MSB 9
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MASK 0x00000200


/* Description		AMPDU_FLAG

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			When set, the MPDU was received as part of an A-MPDU.
			<legal all>
*/

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_LSB 10
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MSB 10
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MASK 0x00000400


/* Description		BAR_FRAME

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			When set, the received frame is a BAR frame. After processing, 
			this frame shall be pushed to SW or deleted.
			<legal all>
*/

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_LSB 11
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MSB 11
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MASK 0x00000800


/* Description		PN_FIELDS_CONTAIN_VALID_INFO

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			Copied here by RXDMA from RX_MPDU_END
			When not set, REO will Not perform a PN sequence number 
			check
*/

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_LSB 12
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MSB 12
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MASK 0x00001000


/* Description		RAW_MPDU

			Field only valid when first_msdu_in_mpdu_flag is set.
			
			When set, the contents in the MSDU buffer contains a 'RAW' 
			MPDU. This 'RAW' MPDU might be spread out over multiple 
			MSDU buffers.
			<legal all>
*/

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_LSB 13
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MSB 13
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MASK 0x00002000


/* Description		MORE_FRAGMENT_FLAG

			The More Fragment bit setting from the MPDU header of the
			 received frame
			
			<legal all>
*/

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_LSB 14
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MSB 14
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MASK 0x00004000


/* Description		SRC_INFO

			Source (virtual) device/interface info. associated with 
			this peer
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			Hamilton v1 used this for 'vdev_id' instead.
			<legal all>
*/

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_LSB 15
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_MSB 26
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_MASK 0x07ff8000


/* Description		MPDU_QOS_CONTROL_VALID

			When set, the MPDU has a QoS control field.
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_LSB 27
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_MSB 27
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_MASK 0x08000000


/* Description		TID

			Field only valid when mpdu_qos_control_valid is set
			
			The TID field in the QoS control field
			<legal all>
*/

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_TID_OFFSET 0x00000008
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_TID_LSB 28
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_TID_MSB 31
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_TID_MASK 0xf0000000


/* Description		PEER_META_DATA

			Meta data that SW has programmed in the Peer table entry
			 of the transmitting STA.
			<legal all>
*/

#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_OFFSET 0x0000000c
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_LSB 0
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MSB 31
#define REO_ENTRANCE_RING_REO_LEVEL_MPDU_FRAME_INFO_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MASK 0xffffffff


/* Description		RX_REO_QUEUE_DESC_ADDR_31_0

			Consumer: REO
			Producer: RXDMA
			
			Address (lower 32 bits) of the REO queue descriptor
			
			Alternatively, as per FR63739, REO internally looks up the
			 queue descriptor address from 'Sw_peer_id' and 'Tid.' In
			 this mode, RXDMA fills 'Sw_peer_id' from 'RX_MPDU_START' 
			in the LSB 16 bits. 'Tid' is available in 'RX_MPDU_DETAILS.'
			
			<legal all>
*/

#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET                        0x00000010
#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_31_0_LSB                           0
#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_31_0_MSB                           31
#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_31_0_MASK                          0xffffffff


/* Description		RX_REO_QUEUE_DESC_ADDR_39_32

			Consumer: REO
			Producer: RXDMA
			
			Address (upper 8 bits) of the REO queue descriptor
			Alternatively, as per FR63739, REO internally looks up the
			 queue descriptor address from 'Sw_peer_id' and 'Tid.' In
			 this mode, this field is unused.
			<legal all>
*/

#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_39_32_OFFSET                       0x00000014
#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_39_32_LSB                          0
#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_39_32_MSB                          7
#define REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_39_32_MASK                         0x000000ff


/* Description		ROUNDED_MPDU_BYTE_COUNT

			An approximation of the number of bytes received in this
			 MPDU. 
			Used to keeps stats on the amount of data flowing through
			 a queue.
			<legal all>
*/

#define REO_ENTRANCE_RING_ROUNDED_MPDU_BYTE_COUNT_OFFSET                            0x00000014
#define REO_ENTRANCE_RING_ROUNDED_MPDU_BYTE_COUNT_LSB                               8
#define REO_ENTRANCE_RING_ROUNDED_MPDU_BYTE_COUNT_MSB                               21
#define REO_ENTRANCE_RING_ROUNDED_MPDU_BYTE_COUNT_MASK                              0x003fff00


/* Description		REO_DESTINATION_INDICATION

			RXDMA copy the MPDU's first MSDU's destination indication
			 field here. This is used for REO to be able to re-route
			 the packet to a different SW destination ring if the packet
			 is detected as error in REO.
			
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

#define REO_ENTRANCE_RING_REO_DESTINATION_INDICATION_OFFSET                         0x00000014
#define REO_ENTRANCE_RING_REO_DESTINATION_INDICATION_LSB                            22
#define REO_ENTRANCE_RING_REO_DESTINATION_INDICATION_MSB                            26
#define REO_ENTRANCE_RING_REO_DESTINATION_INDICATION_MASK                           0x07c00000


/* Description		FRAMELESS_BAR

			When set, this REO entrance ring struct contains BAR info
			 from a multi TID BAR frame. The original multi TID BAR 
			frame itself contained all the REO info for the first TID, 
			but all the subsequent TID info and their linkage to the
			 REO descriptors is passed down as 'frameless' BAR info.
			
			
			The only fields valid in this descriptor when this bit is
			 set are:
			Rx_reo_queue_desc_addr_31_0
			RX_reo_queue_desc_addr_39_32
			
			And within the
			Reo_level_mpdu_frame_info:    
			   Within Rx_mpdu_desc_info_details:
			Mpdu_Sequence_number
			BAR_frame
			Peer_meta_data
			All other fields shall be set to 0
			
			<legal all>
*/

#define REO_ENTRANCE_RING_FRAMELESS_BAR_OFFSET                                      0x00000014
#define REO_ENTRANCE_RING_FRAMELESS_BAR_LSB                                         27
#define REO_ENTRANCE_RING_FRAMELESS_BAR_MSB                                         27
#define REO_ENTRANCE_RING_FRAMELESS_BAR_MASK                                        0x08000000


/* Description		RESERVED_5A

			<legal 0>
*/

#define REO_ENTRANCE_RING_RESERVED_5A_OFFSET                                        0x00000014
#define REO_ENTRANCE_RING_RESERVED_5A_LSB                                           28
#define REO_ENTRANCE_RING_RESERVED_5A_MSB                                           31
#define REO_ENTRANCE_RING_RESERVED_5A_MASK                                          0xf0000000


/* Description		RXDMA_PUSH_REASON

			Indicates why rxdma pushed the frame to this ring
			
			This field is ignored by REO. 
			
			<enum 0 rxdma_error_detected> RXDMA detected an error an
			 pushed this frame to this queue
			<enum 1 rxdma_routing_instruction> RXDMA pushed the frame
			 to this queue per received routing instructions. No error
			 within RXDMA was detected
			<enum 2 rxdma_rx_flush> RXDMA received an RX_FLUSH. As a
			 result the MSDU link descriptor might not have the "last_msdu_in_mpdu_flag" 
			set, but instead WBM might just see a NULL pointer in the
			 MSDU link descriptor. This is to be considered a normal
			 condition for this scenario.
			
			<legal 0 - 2>
*/

#define REO_ENTRANCE_RING_RXDMA_PUSH_REASON_OFFSET                                  0x00000018
#define REO_ENTRANCE_RING_RXDMA_PUSH_REASON_LSB                                     0
#define REO_ENTRANCE_RING_RXDMA_PUSH_REASON_MSB                                     1
#define REO_ENTRANCE_RING_RXDMA_PUSH_REASON_MASK                                    0x00000003


/* Description		RXDMA_ERROR_CODE

			Field only valid when 'rxdma_push_reason' set to 'rxdma_error_detected'.
			
			
			This field is ignored by REO.
			
			<enum 0 rxdma_overflow_err>MPDU frame is not complete due
			 to a FIFO overflow error in RXPCU.
			<enum 1 rxdma_mpdu_length_err>MPDU frame is not complete
			 due to receiving incomplete MPDU from the PHY
			<enum 2 rxdma_fcs_err>FCS check on the MPDU frame failed
			
			<enum 3 rxdma_decrypt_err>CRYPTO reported a decryption error
			 or CRYPTO received an encrypted frame, but did not get 
			a valid corresponding key id in the peer entry.
			<enum 4 rxdma_tkip_mic_err>CRYPTO reported a TKIP MIC error
			
			<enum 5 rxdma_unecrypted_err>CRYPTO reported an unencrypted
			 frame error when encrypted was expected
			<enum 6 rxdma_msdu_len_err>RX OLE reported an MSDU length
			 error
			<enum 7 rxdma_msdu_limit_err>RX OLE reported that max number
			 of MSDUs allowed in an MPDU got exceeded
			<enum 8 rxdma_wifi_parse_err>RX OLE reported a parsing error
			
			<enum 9 rxdma_amsdu_parse_err>RX OLE reported an A-MSDU 
			parsing error
			<enum 10 rxdma_sa_timeout_err>RX OLE reported a timeout 
			during SA search
			<enum 11 rxdma_da_timeout_err>RX OLE reported a timeout 
			during DA search
			<enum 12 rxdma_flow_timeout_err>RX OLE reported a timeout
			 during flow search
			<enum 13 rxdma_flush_request>RXDMA received a flush request
			
			<enum 14 rxdma_amsdu_fragment_err>Rx PCU reported A-MSDU
			 present as well as a fragmented MPDU. A-MSDU defragmentation
			 is not supported in Lithium SW so this is treated as an
			 error.
			<enum 15 rxdma_multicast_echo_err>RX OLE reported a multicast
			 echo
			<enum 16 rxdma_amsdu_addr_mismatch_err>RX OLE reported an
			 A-MSDU with either 'from DS = 0' with an SA mismatching
			 TA or 'to DS = 0' with a DA mismatching RA.
			<enum 17 rxdma_unauthorized_wds_err>RX PCU reported that
			 Rx peer entry did not indicate 'authorized_to_send_WDS' 
			and also indicated 'from DS = to DS = 1.'
			<enum 18 rxdma_groupcast_amsdu_or_wds_err>RX PCU reported
			 a broadcast or multicast RA as well as either A-MSDU present
			 or 'from DS = to DS = 1.'
*/

#define REO_ENTRANCE_RING_RXDMA_ERROR_CODE_OFFSET                                   0x00000018
#define REO_ENTRANCE_RING_RXDMA_ERROR_CODE_LSB                                      2
#define REO_ENTRANCE_RING_RXDMA_ERROR_CODE_MSB                                      6
#define REO_ENTRANCE_RING_RXDMA_ERROR_CODE_MASK                                     0x0000007c


/* Description		MPDU_FRAGMENT_NUMBER

			Field only valid when Reo_level_mpdu_frame_info. Rx_mpdu_desc_info_details.Fragment_flag
			 is set.
			
			The fragment number from the 802.11 header.
			
			Note that the sequence number is embedded in the field: 
			Reo_level_mpdu_frame_info. Rx_mpdu_desc_info_details. Mpdu_sequence_number
			
			
			<legal all>
*/

#define REO_ENTRANCE_RING_MPDU_FRAGMENT_NUMBER_OFFSET                               0x00000018
#define REO_ENTRANCE_RING_MPDU_FRAGMENT_NUMBER_LSB                                  7
#define REO_ENTRANCE_RING_MPDU_FRAGMENT_NUMBER_MSB                                  10
#define REO_ENTRANCE_RING_MPDU_FRAGMENT_NUMBER_MASK                                 0x00000780


/* Description		SW_EXCEPTION

			When not set, REO is performing all its default MPDU processing
			 operations,
			When set, this REO entrance descriptor is generated by FW, 
			and should be processed as an exception. This implies: 
			NO re-order function is needed.
			MPDU delinking is determined by the setting of field SW_excection_mpdu_delink
			
			Destination ring selection is based on the setting of the
			 field SW_exception_destination_ring_valid
			In the destination ring descriptor set bit: SW_exception_entry
			
			Feature supported only in HastingsPrime
			<legal all>
*/

#define REO_ENTRANCE_RING_SW_EXCEPTION_OFFSET                                       0x00000018
#define REO_ENTRANCE_RING_SW_EXCEPTION_LSB                                          11
#define REO_ENTRANCE_RING_SW_EXCEPTION_MSB                                          11
#define REO_ENTRANCE_RING_SW_EXCEPTION_MASK                                         0x00000800


/* Description		SW_EXCEPTION_MPDU_DELINK

			Field only valid when SW_exception is set.
			1'b0: REO should NOT delink the MPDU, and thus pass this
			 MPDU on to the destination ring as is. This implies that
			 in the REO_DESTINATION_RING struct field Buf_or_link_desc_addr_info
			 should point to an MSDU link descriptor
			1'b1: REO should perform the normal MPDU delink into MSDU
			 operations.
			Feature supported only in HastingsPrime
			<legal all>
*/

#define REO_ENTRANCE_RING_SW_EXCEPTION_MPDU_DELINK_OFFSET                           0x00000018
#define REO_ENTRANCE_RING_SW_EXCEPTION_MPDU_DELINK_LSB                              12
#define REO_ENTRANCE_RING_SW_EXCEPTION_MPDU_DELINK_MSB                              12
#define REO_ENTRANCE_RING_SW_EXCEPTION_MPDU_DELINK_MASK                             0x00001000


/* Description		SW_EXCEPTION_DESTINATION_RING_VALID

			Field only valid when SW_exception is set.
			1'b0: REO shall push the MPDU (or delinked MPDU based on
			 the setting of SW_exception_mpdu_delink) to the destination
			 ring according to field reo_destination_indication.
			1'b1: REO shall push the MPDU (or delinked MPDU based on
			 the setting of SW_exception_mpdu_delink) to the destination
			 ring according to field SW_exception_destination_ring.
			Feature supported only in HastingsPrime
			<legal all>
*/

#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_VALID_OFFSET                0x00000018
#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_VALID_LSB                   13
#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_VALID_MSB                   13
#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_VALID_MASK                  0x00002000


/* Description		SW_EXCEPTION_DESTINATION_RING

			Field only valid when fields SW_exception and SW_exception_destination_ring_valid
			 are set.
			The ID of the ring where REO shall push this frame.
			<enum 0 reo_destination_sw0> Reo will push the frame into
			 the REO2SW0 ring
			<enum 1 reo_destination_sw1> Reo will push the frame into
			 the REO2SW1 ring
			<enum 2 reo_destination_sw2> Reo will push the frame into
			 the REO2SW1 ring
			<enum 3 reo_destination_sw3> Reo will push the frame into
			 the REO2SW1 ring
			<enum 4 reo_destination_sw4> Reo will push the frame into
			 the REO2SW1 ring
			<enum 5 reo_destination_release> Reo will push the frame
			 into the REO_release ring
			<enum 6 reo_destination_fw> Reo will push the frame into
			 the REO2FW ring
			<enum 7 reo_destination_sw5> REO remaps this
			<enum 8 reo_destination_sw6> REO remaps this 
			<enum 9 reo_destination_sw7> REO remaps this
			<enum 10 reo_destination_sw8> REO remaps this 
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
			
			Feature supported only in HastingsPrime
			<legal all>
*/

#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_OFFSET                      0x00000018
#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_LSB                         14
#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_MSB                         18
#define REO_ENTRANCE_RING_SW_EXCEPTION_DESTINATION_RING_MASK                        0x0007c000


/* Description		MPDU_SEQUENCE_NUMBER

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			The field can have two different meanings based on the setting
			 of sub-field Reo_level_mpdu_frame_info. Rx_mpdu_desc_info_details.BAR_frame:
			
			
			'BAR_frame' is NOT set:
			The MPDU sequence number of the received frame.
			
			'BAR_frame' is set.
			The MPDU Start sequence number from the BAR frame
			<legal all>
*/

#define REO_ENTRANCE_RING_MPDU_SEQUENCE_NUMBER_OFFSET                               0x00000018
#define REO_ENTRANCE_RING_MPDU_SEQUENCE_NUMBER_LSB                                  19
#define REO_ENTRANCE_RING_MPDU_SEQUENCE_NUMBER_MSB                                  30
#define REO_ENTRANCE_RING_MPDU_SEQUENCE_NUMBER_MASK                                 0x7ff80000


/* Description		RESERVED_6A

			Field only valid when Reo_level_mpdu_frame_info. Rx_mpdu_desc_info_details. 
			Mpdu_qos_control_valid is set
			
			This indicates whether the 'Ack policy' field within the
			 QoS control field of the MPDU indicates 'no-Ack.'
			<legal all>
*/

#define REO_ENTRANCE_RING_RESERVED_6A_OFFSET                                        0x00000018
#define REO_ENTRANCE_RING_RESERVED_6A_LSB                                           31
#define REO_ENTRANCE_RING_RESERVED_6A_MSB                                           31
#define REO_ENTRANCE_RING_RESERVED_6A_MASK                                          0x80000000


/* Description		PHY_PPDU_ID

			A PPDU counter value that PHY increments for every PPDU 
			received
			The counter value wraps around. Pine RXDMA can be configured
			 to copy this from the RX_PPDU_START TLV for every output
			 descriptor.
			
			This field is ignored by REO.
			
			Feature supported only in Pine
			<legal all>
*/

#define REO_ENTRANCE_RING_PHY_PPDU_ID_OFFSET                                        0x0000001c
#define REO_ENTRANCE_RING_PHY_PPDU_ID_LSB                                           0
#define REO_ENTRANCE_RING_PHY_PPDU_ID_MSB                                           15
#define REO_ENTRANCE_RING_PHY_PPDU_ID_MASK                                          0x0000ffff


/* Description		SRC_LINK_ID

			Consumer: SW
			Producer: RXDMA
			
			Set to the link ID of the PMAC that received the frame
			<legal all>
*/

#define REO_ENTRANCE_RING_SRC_LINK_ID_OFFSET                                        0x0000001c
#define REO_ENTRANCE_RING_SRC_LINK_ID_LSB                                           16
#define REO_ENTRANCE_RING_SRC_LINK_ID_MSB                                           18
#define REO_ENTRANCE_RING_SRC_LINK_ID_MASK                                          0x00070000


/* Description		RESERVED_7A

			Hamilton v1 filled the link ID of the PMAC that received
			 the frame here.
			<legal 0>
*/

#define REO_ENTRANCE_RING_RESERVED_7A_OFFSET                                        0x0000001c
#define REO_ENTRANCE_RING_RESERVED_7A_LSB                                           19
#define REO_ENTRANCE_RING_RESERVED_7A_MSB                                           19
#define REO_ENTRANCE_RING_RESERVED_7A_MASK                                          0x00080000


/* Description		RING_ID

			Consumer: SW/REO/DEBUG
			Producer: SRNG (of RXDMA)
			
			For debugging. 
			This field is filled in by the SRNG module.
			It help to identify the ring that is being looked <legal
			 all>
*/

#define REO_ENTRANCE_RING_RING_ID_OFFSET                                            0x0000001c
#define REO_ENTRANCE_RING_RING_ID_LSB                                               20
#define REO_ENTRANCE_RING_RING_ID_MSB                                               27
#define REO_ENTRANCE_RING_RING_ID_MASK                                              0x0ff00000


/* Description		LOOPING_COUNT

			Consumer: SW/REO/DEBUG
			Producer: SRNG (of RXDMA)
			
			For debugging. 
			This field is filled in by the SRNG module.
			
			A count value that indicates the number of times the producer
			 of entries into this Ring has looped around the ring.
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

#define REO_ENTRANCE_RING_LOOPING_COUNT_OFFSET                                      0x0000001c
#define REO_ENTRANCE_RING_LOOPING_COUNT_LSB                                         28
#define REO_ENTRANCE_RING_LOOPING_COUNT_MSB                                         31
#define REO_ENTRANCE_RING_LOOPING_COUNT_MASK                                        0xf0000000



#endif   // REO_ENTRANCE_RING
