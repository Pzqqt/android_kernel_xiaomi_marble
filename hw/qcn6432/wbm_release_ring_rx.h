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

#ifndef _WBM_RELEASE_RING_RX_H_
#define _WBM_RELEASE_RING_RX_H_
#if !defined(__ASSEMBLER__)
#endif

#include "rx_msdu_desc_info.h"
#include "rx_mpdu_desc_info.h"
#include "buffer_addr_info.h"
#define NUM_OF_DWORDS_WBM_RELEASE_RING_RX 8


struct wbm_release_ring_rx {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   buffer_addr_info                                          released_buff_or_desc_addr_info;
             uint32_t release_source_module                                   :  3, // [2:0]
                      bm_action                                               :  3, // [5:3]
                      buffer_or_desc_type                                     :  3, // [8:6]
                      first_msdu_index                                        :  4, // [12:9]
                      reserved_2a                                             :  2, // [14:13]
                      cache_id                                                :  1, // [15:15]
                      cookie_conversion_status                                :  1, // [16:16]
                      rxdma_push_reason                                       :  2, // [18:17]
                      rxdma_error_code                                        :  5, // [23:19]
                      reo_push_reason                                         :  2, // [25:24]
                      reo_error_code                                          :  5, // [30:26]
                      wbm_internal_error                                      :  1; // [31:31]
             struct   rx_mpdu_desc_info                                         rx_mpdu_desc_info_details;
             struct   rx_msdu_desc_info                                         rx_msdu_desc_info_details;
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 20, // [19:0]
                      ring_id                                                 :  8, // [27:20]
                      looping_count                                           :  4; // [31:28]
#else
             struct   buffer_addr_info                                          released_buff_or_desc_addr_info;
             uint32_t wbm_internal_error                                      :  1, // [31:31]
                      reo_error_code                                          :  5, // [30:26]
                      reo_push_reason                                         :  2, // [25:24]
                      rxdma_error_code                                        :  5, // [23:19]
                      rxdma_push_reason                                       :  2, // [18:17]
                      cookie_conversion_status                                :  1, // [16:16]
                      cache_id                                                :  1, // [15:15]
                      reserved_2a                                             :  2, // [14:13]
                      first_msdu_index                                        :  4, // [12:9]
                      buffer_or_desc_type                                     :  3, // [8:6]
                      bm_action                                               :  3, // [5:3]
                      release_source_module                                   :  3; // [2:0]
             struct   rx_mpdu_desc_info                                         rx_mpdu_desc_info_details;
             struct   rx_msdu_desc_info                                         rx_msdu_desc_info_details;
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t looping_count                                           :  4, // [31:28]
                      ring_id                                                 :  8, // [27:20]
                      reserved_7a                                             : 20; // [19:0]
#endif
};


/* Description		RELEASED_BUFF_OR_DESC_ADDR_INFO

			Consumer: WBM/SW/FW
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			Details of the physical address of the buffer or link descriptor
			 that is being released. Note that within this descriptor, 
			WBM will look at the 'owner' of the released buffer/descriptor
			 and forward it to SW/FW is WBM is not the owner.
*/


/* Description		BUFFER_ADDR_31_0

			Address (lower 32 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET 0x00000000
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB    0
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MSB    31
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK   0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET 0x00000004
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB   0
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MSB   7
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK  0x000000ff


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

#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET 0x00000004
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB 8
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB 11
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK 0x00000f00


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

#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET 0x00000004
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB    12
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MSB    31
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK   0xfffff000


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

#define WBM_RELEASE_RING_RX_RELEASE_SOURCE_MODULE_OFFSET                            0x00000008
#define WBM_RELEASE_RING_RX_RELEASE_SOURCE_MODULE_LSB                               0
#define WBM_RELEASE_RING_RX_RELEASE_SOURCE_MODULE_MSB                               2
#define WBM_RELEASE_RING_RX_RELEASE_SOURCE_MODULE_MASK                              0x00000007


/* Description		BM_ACTION

			Consumer: WBM/SW/FW
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			Field only valid when the field return_buffer_manager in
			 the Released_buff_or_desc_addr_info indicates:
			WBM_IDLE_BUF_LIST or
			WBM_IDLE_DESC_LIST
			
			An MSDU extension descriptor shall never be marked as WBM
			 being the 'owner', and thus WBM will forward it to FW/SW
			
			
			<enum 0 Put_in_idle_list> Put the buffer or descriptor back
			 in the idle list. In case of MSDU or MDPU link descriptor, 
			BM does not need to check to release any individual MSDU
			 buffers
			
			<enum 1 release_msdu_list > This BM action can only be used
			 in combination with buffer_or_desc_type being msdu_link_descriptor. 
			Field first_msdu_index points out which MSDU pointer in 
			the MSDU link descriptor is the first of an MPDU that is
			 released.
			BM shall release all the MSDU buffers linked to this first
			 MSDU buffer pointer. All related MSDU buffer pointer entries
			 shall be set to value 0, which represents the 'NULL" pointer. 
			When all MSDU buffer pointers in the MSDU link descriptor
			 are 'NULL', the MSDU link descriptor itself shall also 
			be released.
			
			<enum 2 Put_in_idle_list_expanded> CURRENTLY NOT IMPLEMENTED....
			
			Put the buffer or descriptor back in the idle list. Only
			 valid in combination with buffer_or_desc_type indicating
			 MDPU_link_descriptor.
			BM shall release the MPDU link descriptor as well as all
			 MSDUs that are linked to the MPDUs in this descriptor. 
			
			
			TODO: Any restrictions?
			<legal 0-2>
*/

#define WBM_RELEASE_RING_RX_BM_ACTION_OFFSET                                        0x00000008
#define WBM_RELEASE_RING_RX_BM_ACTION_LSB                                           3
#define WBM_RELEASE_RING_RX_BM_ACTION_MSB                                           5
#define WBM_RELEASE_RING_RX_BM_ACTION_MASK                                          0x00000038


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

#define WBM_RELEASE_RING_RX_BUFFER_OR_DESC_TYPE_OFFSET                              0x00000008
#define WBM_RELEASE_RING_RX_BUFFER_OR_DESC_TYPE_LSB                                 6
#define WBM_RELEASE_RING_RX_BUFFER_OR_DESC_TYPE_MSB                                 8
#define WBM_RELEASE_RING_RX_BUFFER_OR_DESC_TYPE_MASK                                0x000001c0


/* Description		FIRST_MSDU_INDEX

			Consumer: WBM/SW/FW
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			Field only valid for the bm_action release_msdu_list.
			
			The index of the first MSDU in an MSDU link descriptor all
			 belonging to the same MPDU.
			
			TODO: Any restrictions?
			<legal 0-6>
*/

#define WBM_RELEASE_RING_RX_FIRST_MSDU_INDEX_OFFSET                                 0x00000008
#define WBM_RELEASE_RING_RX_FIRST_MSDU_INDEX_LSB                                    9
#define WBM_RELEASE_RING_RX_FIRST_MSDU_INDEX_MSB                                    12
#define WBM_RELEASE_RING_RX_FIRST_MSDU_INDEX_MASK                                   0x00001e00


/* Description		RESERVED_2A

			<legal 0>
*/

#define WBM_RELEASE_RING_RX_RESERVED_2A_OFFSET                                      0x00000008
#define WBM_RELEASE_RING_RX_RESERVED_2A_LSB                                         13
#define WBM_RELEASE_RING_RX_RESERVED_2A_MSB                                         14
#define WBM_RELEASE_RING_RX_RESERVED_2A_MASK                                        0x00006000


/* Description		CACHE_ID

			Indicates the WBM cache the MSDU was released from
			<legal all>
*/

#define WBM_RELEASE_RING_RX_CACHE_ID_OFFSET                                         0x00000008
#define WBM_RELEASE_RING_RX_CACHE_ID_LSB                                            15
#define WBM_RELEASE_RING_RX_CACHE_ID_MSB                                            15
#define WBM_RELEASE_RING_RX_CACHE_ID_MASK                                           0x00008000


/* Description		COOKIE_CONVERSION_STATUS

			0: 'Sw_buffer_cookie' not converted to 'Buffer_virt_addr'
			
			1: 'Sw_buffer_cookie' coverted to 'Buffer_virt_addr'
			<legal 0>
*/

#define WBM_RELEASE_RING_RX_COOKIE_CONVERSION_STATUS_OFFSET                         0x00000008
#define WBM_RELEASE_RING_RX_COOKIE_CONVERSION_STATUS_LSB                            16
#define WBM_RELEASE_RING_RX_COOKIE_CONVERSION_STATUS_MSB                            16
#define WBM_RELEASE_RING_RX_COOKIE_CONVERSION_STATUS_MASK                           0x00010000


/* Description		RXDMA_PUSH_REASON

			Field only valid when Release_source_module is set to release_source_RXDMA
			
			
			Indicates why rxdma pushed the frame to this ring
			
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

#define WBM_RELEASE_RING_RX_RXDMA_PUSH_REASON_OFFSET                                0x00000008
#define WBM_RELEASE_RING_RX_RXDMA_PUSH_REASON_LSB                                   17
#define WBM_RELEASE_RING_RX_RXDMA_PUSH_REASON_MSB                                   18
#define WBM_RELEASE_RING_RX_RXDMA_PUSH_REASON_MASK                                  0x00060000



#define WBM_RELEASE_RING_RX_RXDMA_ERROR_CODE_OFFSET                                 0x00000008
#define WBM_RELEASE_RING_RX_RXDMA_ERROR_CODE_LSB                                    19
#define WBM_RELEASE_RING_RX_RXDMA_ERROR_CODE_MSB                                    23
#define WBM_RELEASE_RING_RX_RXDMA_ERROR_CODE_MASK                                   0x00f80000


/* Description		REO_PUSH_REASON

			Field only valid when Release_source_module is set to release_source_REO
			
			
			Indicates why REO pushed the frame to this release ring
			
			<enum 0 reo_error_detected> Reo detected an error an pushed
			 this frame to this queue
			<enum 1 reo_routing_instruction> Reo pushed the frame to
			 this queue per received routing instructions. No error 
			within REO was detected
			
			<legal 0 - 1>
*/

#define WBM_RELEASE_RING_RX_REO_PUSH_REASON_OFFSET                                  0x00000008
#define WBM_RELEASE_RING_RX_REO_PUSH_REASON_LSB                                     24
#define WBM_RELEASE_RING_RX_REO_PUSH_REASON_MSB                                     25
#define WBM_RELEASE_RING_RX_REO_PUSH_REASON_MASK                                    0x03000000


/* Description		REO_ERROR_CODE

			Field only valid when 'Reo_push_reason' set to 'reo_error_detected'.
			
			
			<enum 0 reo_queue_desc_addr_zero> Reo queue descriptor provided
			 in the REO_ENTRANCE ring is set to 0
			<enum 1 reo_queue_desc_not_valid> Reo queue descriptor valid
			 bit is NOT set
			<enum 2 ampdu_in_non_ba> AMPDU frame received without BA
			 session having been setup.
			<enum 3 non_ba_duplicate> Non-BA session, SN equal to SSN, 
			Retry bit set: duplicate frame
			<enum 4 ba_duplicate> BA session, duplicate frame
			<enum 5 regular_frame_2k_jump> A normal (management/data
			 frame) received with 2K jump in SN
			<enum 6 bar_frame_2k_jump> A bar received with 2K jump in
			 SSN
			<enum 7 regular_frame_OOR> A normal (management/data frame) 
			received with SN falling within the OOR window
			<enum 8 bar_frame_OOR> A bar received with SSN falling within
			 the OOR window
			<enum 9 bar_frame_no_ba_session> A bar received without 
			a BA session
			<enum 10 bar_frame_sn_equals_ssn> A bar received with SSN
			 equal to SN
			<enum 11 pn_check_failed> PN Check Failed packet.
			<enum 12 2k_error_handling_flag_set> Frame is forwarded 
			as a result of the 'Seq_2k_error_detected_flag' been set
			 in the REO Queue descriptor
			<enum 13 pn_error_handling_flag_set> Frame is forwarded 
			as a result of the 'pn_error_detected_flag' been set in 
			the REO Queue descriptor
			<enum 14 queue_descriptor_blocked_set> Frame is forwarded
			 as a result of the queue descriptor(address) being blocked
			 as SW/FW seems to be currently in the process of making
			 updates to this descriptor...
			
			<legal 0-14>
*/

#define WBM_RELEASE_RING_RX_REO_ERROR_CODE_OFFSET                                   0x00000008
#define WBM_RELEASE_RING_RX_REO_ERROR_CODE_LSB                                      26
#define WBM_RELEASE_RING_RX_REO_ERROR_CODE_MSB                                      30
#define WBM_RELEASE_RING_RX_REO_ERROR_CODE_MASK                                     0x7c000000


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

#define WBM_RELEASE_RING_RX_WBM_INTERNAL_ERROR_OFFSET                               0x00000008
#define WBM_RELEASE_RING_RX_WBM_INTERNAL_ERROR_LSB                                  31
#define WBM_RELEASE_RING_RX_WBM_INTERNAL_ERROR_MSB                                  31
#define WBM_RELEASE_RING_RX_WBM_INTERNAL_ERROR_MASK                                 0x80000000


/* Description		RX_MPDU_DESC_INFO_DETAILS

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			General information related to the MPDU whose link descriptors
			 are being released from Rx DMA or REO
			
			When enabled in REO, REO will overwrite this structure to
			 have only the 'Msdu_count' field and 56 bits of the previous
			 PN from 'RX_REO_QUEUE'
*/


/* Description		MSDU_COUNT

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			The number of MSDUs within the MPDU 
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET             0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_LSB                0
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MSB                7
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MASK               0x000000ff


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

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_OFFSET          0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_LSB             8
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MSB             8
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MASK            0x00000100


/* Description		MPDU_RETRY_BIT

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			The retry bit setting from the MPDU header of the received
			 frame
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_OFFSET         0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_LSB            9
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MSB            9
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MASK           0x00000200


/* Description		AMPDU_FLAG

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			When set, the MPDU was received as part of an A-MPDU.
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_OFFSET             0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_LSB                10
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MSB                10
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MASK               0x00000400


/* Description		BAR_FRAME

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			When set, the received frame is a BAR frame. After processing, 
			this frame shall be pushed to SW or deleted.
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_OFFSET              0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_LSB                 11
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MSB                 11
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MASK                0x00000800


/* Description		PN_FIELDS_CONTAIN_VALID_INFO

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			Copied here by RXDMA from RX_MPDU_END
			When not set, REO will Not perform a PN sequence number 
			check
*/

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_OFFSET 0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_LSB 12
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MSB 12
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MASK 0x00001000


/* Description		RAW_MPDU

			Field only valid when first_msdu_in_mpdu_flag is set.
			
			When set, the contents in the MSDU buffer contains a 'RAW' 
			MPDU. This 'RAW' MPDU might be spread out over multiple 
			MSDU buffers.
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_OFFSET               0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_LSB                  13
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MSB                  13
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MASK                 0x00002000


/* Description		MORE_FRAGMENT_FLAG

			The More Fragment bit setting from the MPDU header of the
			 received frame
			
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_OFFSET     0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_LSB        14
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MSB        14
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MASK       0x00004000


/* Description		SRC_INFO

			Source (virtual) device/interface info. associated with 
			this peer
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_OFFSET               0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_LSB                  15
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_MSB                  26
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_MASK                 0x07ff8000


/* Description		MPDU_QOS_CONTROL_VALID

			When set, the MPDU has a QoS control field.
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_OFFSET 0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_LSB    27
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_MSB    27
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_MASK   0x08000000


/* Description		TID

			Field only valid when mpdu_qos_control_valid is set
			
			The TID field in the QoS control field
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_TID_OFFSET                    0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_TID_LSB                       28
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_TID_MSB                       31
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_TID_MASK                      0xf0000000


/* Description		PEER_META_DATA

			Meta data that SW has programmed in the Peer table entry
			 of the transmitting STA.
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_OFFSET         0x00000010
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_LSB            0
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MSB            31
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MASK           0xffffffff


/* Description		RX_MSDU_DESC_INFO_DETAILS

			Consumer: TQM/SW
			Producer: SW/SCH(from TXPCU, PDG) /WBM (from RXDMA)
			
			In case of RXDMA or REO releasing Rx MSDU link descriptors,' 
			WBM fills this field with Rx_msdu_desc_info_details when
			 releasing the MSDUs to SW.
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

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_LSB   0
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MSB   0
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MASK  0x00000001


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

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_LSB    1
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MSB    1
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MASK   0x00000002


/* Description		MSDU_CONTINUATION

			When set, this MSDU buffer was not able to hold the entire
			 MSDU. The next buffer will therefor contain additional 
			information related to this MSDU.
			
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_OFFSET      0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_LSB         2
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MSB         2
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MASK        0x00000004


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

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_OFFSET            0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_LSB               3
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MSB               16
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MASK              0x0001fff8


/* Description		MSDU_DROP

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			When set, REO shall drop this MSDU and not forward it to
			 any other ring...
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_OFFSET              0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_LSB                 17
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MSB                 17
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MASK                0x00020000


/* Description		SA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid SA entry for this MSDU
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_OFFSET            0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_LSB               18
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MSB               18
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MASK              0x00040000


/* Description		DA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid DA entry for this MSDU
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_OFFSET            0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_LSB               19
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MSB               19
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MASK              0x00080000


/* Description		DA_IS_MCBC

			Field Only valid if "da_is_valid" is set
			
			Indicates the DA address was a Multicast of Broadcast address
			 for this MSDU
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_OFFSET             0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_LSB                20
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MSB                20
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MASK               0x00100000


/* Description		L3_HEADER_PADDING_MSB

			Passed on from 'RX_MSDU_END' TLV (only the MSB is reported
			 as the LSB is always zero)
			Number of bytes padded to make sure that the L3 header will
			 always start of a Dword boundary
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_OFFSET  0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_LSB     21
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MSB     21
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MASK    0x00200000


/* Description		TCP_UDP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the TCP/UDP header.
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_OFFSET    0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_LSB       22
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MSB       22
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MASK      0x00400000


/* Description		IP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the IP header.
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_OFFSET         0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_LSB            23
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MSB            23
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MASK           0x00800000


/* Description		FR_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'from DS' bit is set in the frame control.
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FR_DS_OFFSET                  0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FR_DS_LSB                     24
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MSB                     24
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MASK                    0x01000000


/* Description		TO_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'to DS' bit is set in the frame control.
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TO_DS_OFFSET                  0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TO_DS_LSB                     25
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MSB                     25
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MASK                    0x02000000


/* Description		INTRA_BSS

			This packet needs intra-BSS routing by SW as the 'vdev_id' 
			for the destination is the same as the 'vdev_id' (from 'RX_MPDU_PCU_START') 
			that this MSDU was got in.
			
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_OFFSET              0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_LSB                 26
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MSB                 26
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MASK                0x04000000


/* Description		DEST_CHIP_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which chip's TCL the packet should be
			 queued.
			
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_OFFSET           0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_LSB              27
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MSB              28
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MASK             0x18000000


/* Description		DECAP_FORMAT

			Indicates the format after decapsulation:
			
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_OFFSET           0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_LSB              29
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MSB              30
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MASK             0x60000000


/* Description		DEST_CHIP_PMAC_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which link/'vdev' the packet should 
			be queued in TCL.
			
			<legal all>
*/

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_OFFSET      0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_LSB         31
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MSB         31
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_PMAC_ID_MASK        0x80000000


/* Description		RESERVED_6A

			<legal 0>
*/

#define WBM_RELEASE_RING_RX_RESERVED_6A_OFFSET                                      0x00000018
#define WBM_RELEASE_RING_RX_RESERVED_6A_LSB                                         0
#define WBM_RELEASE_RING_RX_RESERVED_6A_MSB                                         31
#define WBM_RELEASE_RING_RX_RESERVED_6A_MASK                                        0xffffffff


/* Description		RESERVED_7A

			For debugging, RXDMA and REO may fill the Rx MPDU sequence
			 number in bits [11:0] and WBM may copy over when it releases
			 Rx MSDUs.
			
			<legal 0-4095>
*/

#define WBM_RELEASE_RING_RX_RESERVED_7A_OFFSET                                      0x0000001c
#define WBM_RELEASE_RING_RX_RESERVED_7A_LSB                                         0
#define WBM_RELEASE_RING_RX_RESERVED_7A_MSB                                         19
#define WBM_RELEASE_RING_RX_RESERVED_7A_MASK                                        0x000fffff


/* Description		RING_ID

			Consumer: TQM/REO/RXDMA/SW
			Producer: SRNG (of RXDMA)
			
			For debugging. 
			This field is filled in by the SRNG module.
			It help to identify the ring that is being looked <legal
			 all>
*/

#define WBM_RELEASE_RING_RX_RING_ID_OFFSET                                          0x0000001c
#define WBM_RELEASE_RING_RX_RING_ID_LSB                                             20
#define WBM_RELEASE_RING_RX_RING_ID_MSB                                             27
#define WBM_RELEASE_RING_RX_RING_ID_MASK                                            0x0ff00000


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

#define WBM_RELEASE_RING_RX_LOOPING_COUNT_OFFSET                                    0x0000001c
#define WBM_RELEASE_RING_RX_LOOPING_COUNT_LSB                                       28
#define WBM_RELEASE_RING_RX_LOOPING_COUNT_MSB                                       31
#define WBM_RELEASE_RING_RX_LOOPING_COUNT_MASK                                      0xf0000000



#endif   // WBM_RELEASE_RING_RX
