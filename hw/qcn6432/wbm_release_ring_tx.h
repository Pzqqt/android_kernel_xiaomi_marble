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

#ifndef _WBM_RELEASE_RING_TX_H_
#define _WBM_RELEASE_RING_TX_H_
#if !defined(__ASSEMBLER__)
#endif

#include "tx_rate_stats_info.h"
#include "buffer_addr_info.h"
#define NUM_OF_DWORDS_WBM_RELEASE_RING_TX 8


struct wbm_release_ring_tx {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   buffer_addr_info                                          released_buff_or_desc_addr_info;
             uint32_t release_source_module                                   :  3, // [2:0]
                      bm_action                                               :  3, // [5:3]
                      buffer_or_desc_type                                     :  3, // [8:6]
                      first_msdu_index                                        :  4, // [12:9]
                      tqm_release_reason                                      :  4, // [16:13]
                      rbm_override_valid                                      :  1, // [17:17]
                      rbm_override                                            :  4, // [21:18]
                      reserved_2a                                             :  7, // [28:22]
                      cache_id                                                :  1, // [29:29]
                      cookie_conversion_status                                :  1, // [30:30]
                      wbm_internal_error                                      :  1; // [31:31]
             uint32_t tqm_status_number                                       : 24, // [23:0]
                      transmit_count                                          :  7, // [30:24]
                      sw_release_details_valid                                :  1; // [31:31]
             uint32_t ack_frame_rssi                                          :  8, // [7:0]
                      first_msdu                                              :  1, // [8:8]
                      last_msdu                                               :  1, // [9:9]
                      fw_tx_notify_frame                                      :  3, // [12:10]
                      buffer_timestamp                                        : 19; // [31:13]
             struct   tx_rate_stats_info                                        tx_rate_stats;
             uint32_t sw_peer_id                                              : 16, // [15:0]
                      tid                                                     :  4, // [19:16]
                      tqm_status_number_31_24                                 :  8, // [27:20]
                      looping_count                                           :  4; // [31:28]
#else
             struct   buffer_addr_info                                          released_buff_or_desc_addr_info;
             uint32_t wbm_internal_error                                      :  1, // [31:31]
                      cookie_conversion_status                                :  1, // [30:30]
                      cache_id                                                :  1, // [29:29]
                      reserved_2a                                             :  7, // [28:22]
                      rbm_override                                            :  4, // [21:18]
                      rbm_override_valid                                      :  1, // [17:17]
                      tqm_release_reason                                      :  4, // [16:13]
                      first_msdu_index                                        :  4, // [12:9]
                      buffer_or_desc_type                                     :  3, // [8:6]
                      bm_action                                               :  3, // [5:3]
                      release_source_module                                   :  3; // [2:0]
             uint32_t sw_release_details_valid                                :  1, // [31:31]
                      transmit_count                                          :  7, // [30:24]
                      tqm_status_number                                       : 24; // [23:0]
             uint32_t buffer_timestamp                                        : 19, // [31:13]
                      fw_tx_notify_frame                                      :  3, // [12:10]
                      last_msdu                                               :  1, // [9:9]
                      first_msdu                                              :  1, // [8:8]
                      ack_frame_rssi                                          :  8; // [7:0]
             struct   tx_rate_stats_info                                        tx_rate_stats;
             uint32_t looping_count                                           :  4, // [31:28]
                      tqm_status_number_31_24                                 :  8, // [27:20]
                      tid                                                     :  4, // [19:16]
                      sw_peer_id                                              : 16; // [15:0]
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

#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET 0x00000000
#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB    0
#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MSB    31
#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK   0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET 0x00000004
#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB   0
#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MSB   7
#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK  0x000000ff


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

#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET 0x00000004
#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB 8
#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB 11
#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK 0x00000f00


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

#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET 0x00000004
#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB    12
#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MSB    31
#define WBM_RELEASE_RING_TX_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK   0xfffff000


/* Description		RELEASE_SOURCE_MODULE

			Indicates which module initiated the release of this buffer
			 or descriptor
			
			<enum 1 release_source_RXDMA> DO NOT USE
			<enum 2 release_source_REO> DO NOT USE
			<enum 5 release_source_FW_RX> DO NOT USE
			<enum 4 release_source_SW_RX> DO NOT USE
			<enum 0 release_source_TQM> TQM released this buffer or 
			descriptor
			<enum 3 release_source_FW_TX> FW released this buffer or
			 descriptor
			<enum 6 release_source_SW_TX> SW released this buffer or
			 descriptor
			<legal 0-6>
*/

#define WBM_RELEASE_RING_TX_RELEASE_SOURCE_MODULE_OFFSET                            0x00000008
#define WBM_RELEASE_RING_TX_RELEASE_SOURCE_MODULE_LSB                               0
#define WBM_RELEASE_RING_TX_RELEASE_SOURCE_MODULE_MSB                               2
#define WBM_RELEASE_RING_TX_RELEASE_SOURCE_MODULE_MASK                              0x00000007


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
			
			
			<legal 0-2>
*/

#define WBM_RELEASE_RING_TX_BM_ACTION_OFFSET                                        0x00000008
#define WBM_RELEASE_RING_TX_BM_ACTION_LSB                                           3
#define WBM_RELEASE_RING_TX_BM_ACTION_MSB                                           5
#define WBM_RELEASE_RING_TX_BM_ACTION_MASK                                          0x00000038


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
			
			<legal 0-4>
*/

#define WBM_RELEASE_RING_TX_BUFFER_OR_DESC_TYPE_OFFSET                              0x00000008
#define WBM_RELEASE_RING_TX_BUFFER_OR_DESC_TYPE_LSB                                 6
#define WBM_RELEASE_RING_TX_BUFFER_OR_DESC_TYPE_MSB                                 8
#define WBM_RELEASE_RING_TX_BUFFER_OR_DESC_TYPE_MASK                                0x000001c0


/* Description		FIRST_MSDU_INDEX

			Consumer: WBM/SW/FW
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			Field only valid for the bm_action release_msdu_list.
			
			The index of the first MSDU in an MSDU link descriptor all
			 belonging to the same MPDU.
			
			<legal 0-6>
*/

#define WBM_RELEASE_RING_TX_FIRST_MSDU_INDEX_OFFSET                                 0x00000008
#define WBM_RELEASE_RING_TX_FIRST_MSDU_INDEX_LSB                                    9
#define WBM_RELEASE_RING_TX_FIRST_MSDU_INDEX_MSB                                    12
#define WBM_RELEASE_RING_TX_FIRST_MSDU_INDEX_MASK                                   0x00001e00


/* Description		TQM_RELEASE_REASON

			Consumer: WBM/SW/FW
			Producer: TQM
			
			Field only valid when Release_source_module is set to release_source_TQM
			
			
			(rr = Release Reason)
			<enum 0 tqm_rr_frame_acked> frame is removed because an 
			ACK of BA for it was received 
			<enum 1 tqm_rr_rem_cmd_rem> frame is removed because a remove
			 command of type "Remove_mpdus" initiated by SW
			<enum 2 tqm_rr_rem_cmd_tx> frame is removed because a remove
			 command of type "Remove_transmitted_mpdus" initiated by
			 SW
			<enum 3 tqm_rr_rem_cmd_notx> frame is removed because a 
			remove command of type "Remove_untransmitted_mpdus" initiated
			 by SW
			<enum 4 tqm_rr_rem_cmd_aged> frame is removed because a 
			remove command of type "Remove_aged_mpdus" or "Remove_aged_msdus" 
			initiated by SW
			<enum 5 tqm_fw_reason1> frame is removed because a remove
			 command where fw indicated that remove reason is fw_reason1
			
			<enum 6 tqm_fw_reason2> frame is removed because a remove
			 command where fw indicated that remove reason is fw_reason1
			
			<enum 7 tqm_fw_reason3> frame is removed because a remove
			 command where fw indicated that remove reason is fw_reason1
			
			<enum 8 tqm_rr_rem_cmd_disable_queue> frame is removed because
			 a remove command of type "remove_mpdus_and_disable_queue" 
			or "remove_msdus_and_disable_flow" initiated by SW
			<enum 9 tqm_rr_rem_cmd_till_nonmatching> frame is removed
			 because remove command of type "remove_till_nonmatching_mpdu" 
			initiated by SW
			<enum 10 tqm_rr_drop_threshold> frame is dropped at TQM 
			entrance due to one of slow/medium/hard drop threshold criteria
			
			<enum 11 tqm_rr_link_desc_unavailable> frame is dropped 
			at TQM entrance due to the WBM2TQM_LINK_RING having fewer
			 descriptors than a threshold programmed in TQM
			<enum 12 tqm_rr_drop_or_invalid_msdu> frame is dropped at
			 TQM entrance due to 'TQM_Drop_frame' being set or "null" 
			MSDU flow pointer or MSDU flow pointer 'Flow_valid' being
			 zero or MSDU length being zero
			<enum 13 tqm_rr_multicast_drop> frame is dropped at TQM 
			entrance due to 'TQM_Drop_frame' being set with 'TCL_drop_reason' 
			set to TCL_multicast_drop_for_vdev.
			<enum 14 tqm_rr_vdev_mismatch_drop> frame is dropped at 
			TQM entrance due to 'TQM_Drop_frame' being set with 'TCL_drop_reason' 
			set to TCL_vdev_id_mismatch_drop.
			
			<legal 0-14>
*/

#define WBM_RELEASE_RING_TX_TQM_RELEASE_REASON_OFFSET                               0x00000008
#define WBM_RELEASE_RING_TX_TQM_RELEASE_REASON_LSB                                  13
#define WBM_RELEASE_RING_TX_TQM_RELEASE_REASON_MSB                                  16
#define WBM_RELEASE_RING_TX_TQM_RELEASE_REASON_MASK                                 0x0001e000


/* Description		RBM_OVERRIDE_VALID

			This is set to 0 for Tx cases not involving reinjection, 
			and set to 1 for TQM release cases requiring FW reinjection
			
			When set to 1, WBM releases the MSDU buffers to FW and overrides
			 the tx_rate_stats field with words 2 and 3 of the 'TX_MSDU_DETAILS' 
			structure, for FW reinjection of these MSDUs
			
			<legal 0-1>
*/

#define WBM_RELEASE_RING_TX_RBM_OVERRIDE_VALID_OFFSET                               0x00000008
#define WBM_RELEASE_RING_TX_RBM_OVERRIDE_VALID_LSB                                  17
#define WBM_RELEASE_RING_TX_RBM_OVERRIDE_VALID_MSB                                  17
#define WBM_RELEASE_RING_TX_RBM_OVERRIDE_VALID_MASK                                 0x00020000


/* Description		RBM_OVERRIDE

			Field only valid when rbm_override_valid = 1
			
			WBM releases the MSDU buffers to FW and overrides the tx_rate_stats
			 field with words 2 and 3 of the 'TX_MSDU_DETAILS' structure, 
			for FW reinjection of these MSDUs.
			
*/

#define WBM_RELEASE_RING_TX_RBM_OVERRIDE_OFFSET                                     0x00000008
#define WBM_RELEASE_RING_TX_RBM_OVERRIDE_LSB                                        18
#define WBM_RELEASE_RING_TX_RBM_OVERRIDE_MSB                                        21
#define WBM_RELEASE_RING_TX_RBM_OVERRIDE_MASK                                       0x003c0000


/* Description		RESERVED_2A

			<legal 0>
*/

#define WBM_RELEASE_RING_TX_RESERVED_2A_OFFSET                                      0x00000008
#define WBM_RELEASE_RING_TX_RESERVED_2A_LSB                                         22
#define WBM_RELEASE_RING_TX_RESERVED_2A_MSB                                         28
#define WBM_RELEASE_RING_TX_RESERVED_2A_MASK                                        0x1fc00000


/* Description		CACHE_ID

			To improve WBM performance, out-of-order completions may
			 be allowed to process multiple MPDUs in parallel.
			
			The MSDUs released from each cache would be in order so 'First_msdu' 
			and this field together can be used by SW to reorder the
			 completions back to the original order by keeping all MSDUs
			 of an MPDU from one cache together before switching to 
			the next MPDU (from either cache).
			<legal all>
*/

#define WBM_RELEASE_RING_TX_CACHE_ID_OFFSET                                         0x00000008
#define WBM_RELEASE_RING_TX_CACHE_ID_LSB                                            29
#define WBM_RELEASE_RING_TX_CACHE_ID_MSB                                            29
#define WBM_RELEASE_RING_TX_CACHE_ID_MASK                                           0x20000000


/* Description		COOKIE_CONVERSION_STATUS

			0: 'Sw_buffer_cookie' not converted to 'Buffer_virt_addr'
			
			1: 'Sw_buffer_cookie' coverted to 'Buffer_virt_addr'
			<legal 0>
*/

#define WBM_RELEASE_RING_TX_COOKIE_CONVERSION_STATUS_OFFSET                         0x00000008
#define WBM_RELEASE_RING_TX_COOKIE_CONVERSION_STATUS_LSB                            30
#define WBM_RELEASE_RING_TX_COOKIE_CONVERSION_STATUS_MSB                            30
#define WBM_RELEASE_RING_TX_COOKIE_CONVERSION_STATUS_MASK                           0x40000000


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

#define WBM_RELEASE_RING_TX_WBM_INTERNAL_ERROR_OFFSET                               0x00000008
#define WBM_RELEASE_RING_TX_WBM_INTERNAL_ERROR_LSB                                  31
#define WBM_RELEASE_RING_TX_WBM_INTERNAL_ERROR_MSB                                  31
#define WBM_RELEASE_RING_TX_WBM_INTERNAL_ERROR_MASK                                 0x80000000


/* Description		TQM_STATUS_NUMBER

			Field only valid when Release_source_module is set to release_source_TQM
			
			
			The value in this field is equal to value of the 'TQM_CMD_Number' 
			field from the TQM command or the 'TQM_add_cmd_Number' field
			 from the TQM entrance ring descriptor LSB 24-bits.
			
			This field helps to correlate the statuses with the TQM 
			commands.
			
			NOTE that SW could program this number to be equal to the
			 PPDU_ID number in case direct correlation with the PPDU
			 ID is desired
			
			<legal all> 
*/

#define WBM_RELEASE_RING_TX_TQM_STATUS_NUMBER_OFFSET                                0x0000000c
#define WBM_RELEASE_RING_TX_TQM_STATUS_NUMBER_LSB                                   0
#define WBM_RELEASE_RING_TX_TQM_STATUS_NUMBER_MSB                                   23
#define WBM_RELEASE_RING_TX_TQM_STATUS_NUMBER_MASK                                  0x00ffffff


/* Description		TRANSMIT_COUNT

			Field only valid when Release_source_module is set to release_source_TQM
			
			
			The number of times this frame has been transmitted
*/

#define WBM_RELEASE_RING_TX_TRANSMIT_COUNT_OFFSET                                   0x0000000c
#define WBM_RELEASE_RING_TX_TRANSMIT_COUNT_LSB                                      24
#define WBM_RELEASE_RING_TX_TRANSMIT_COUNT_MSB                                      30
#define WBM_RELEASE_RING_TX_TRANSMIT_COUNT_MASK                                     0x7f000000


/* Description		SW_RELEASE_DETAILS_VALID

			Consumer: SW
			Producer: WBM
			
			When set, some WBM specific release info for SW is valid.
			
			This is set when WMB got a 'release_msdu_list' command from
			 TQM and the return buffer manager is not WMB. WBM will 
			then de-aggregate all the MSDUs and pass them one at a time
			 on to the 'buffer owner'
			
			<legal all>
*/

#define WBM_RELEASE_RING_TX_SW_RELEASE_DETAILS_VALID_OFFSET                         0x0000000c
#define WBM_RELEASE_RING_TX_SW_RELEASE_DETAILS_VALID_LSB                            31
#define WBM_RELEASE_RING_TX_SW_RELEASE_DETAILS_VALID_MSB                            31
#define WBM_RELEASE_RING_TX_SW_RELEASE_DETAILS_VALID_MASK                           0x80000000


/* Description		ACK_FRAME_RSSI

			This field is only valid when the source is TQM.
			
			If this frame is removed as the result of the reception 
			of an ACK or BA, this field indicates the RSSI of the received
			 ACK or BA frame. 
			
			When the frame is removed as result of a direct remove command
			 from the SW,  this field is set to 0x0 (which is never 
			a valid value when real RSSI is available)
			
			<legal all>
*/

#define WBM_RELEASE_RING_TX_ACK_FRAME_RSSI_OFFSET                                   0x00000010
#define WBM_RELEASE_RING_TX_ACK_FRAME_RSSI_LSB                                      0
#define WBM_RELEASE_RING_TX_ACK_FRAME_RSSI_MSB                                      7
#define WBM_RELEASE_RING_TX_ACK_FRAME_RSSI_MASK                                     0x000000ff


/* Description		FIRST_MSDU

			Field only valid when SW_release_details_valid is set.
			
			Consumer: SW
			Producer: WBM
			
			When set, this MSDU is the first MSDU pointed to in the 'release_msdu_list' 
			command.
			
			First_msdu ≠ last_msdu indicates the MSDU was part of 
			an A-MSDU.
			<legal all>
*/

#define WBM_RELEASE_RING_TX_FIRST_MSDU_OFFSET                                       0x00000010
#define WBM_RELEASE_RING_TX_FIRST_MSDU_LSB                                          8
#define WBM_RELEASE_RING_TX_FIRST_MSDU_MSB                                          8
#define WBM_RELEASE_RING_TX_FIRST_MSDU_MASK                                         0x00000100


/* Description		LAST_MSDU

			Field only valid when SW_release_details_valid is set.
			
			Consumer: SW
			Producer: WBM
			
			When set, this MSDU is the last MSDU pointed to in the 'release_msdu_list' 
			command.
			
			First_msdu ≠ last_msdu indicates the MSDU was part of 
			an A-MSDU.
			<legal all>
*/

#define WBM_RELEASE_RING_TX_LAST_MSDU_OFFSET                                        0x00000010
#define WBM_RELEASE_RING_TX_LAST_MSDU_LSB                                           9
#define WBM_RELEASE_RING_TX_LAST_MSDU_MSB                                           9
#define WBM_RELEASE_RING_TX_LAST_MSDU_MASK                                          0x00000200


/* Description		FW_TX_NOTIFY_FRAME

			Field only valid when SW_release_details_valid is set.
			
			Consumer: SW
			Producer: WBM
			
			This is the FW_tx_notify_frame field from the TX_MSDU_DETAILS
			 for this frame from the MSDU link descriptor
			<legal all>
*/

#define WBM_RELEASE_RING_TX_FW_TX_NOTIFY_FRAME_OFFSET                               0x00000010
#define WBM_RELEASE_RING_TX_FW_TX_NOTIFY_FRAME_LSB                                  10
#define WBM_RELEASE_RING_TX_FW_TX_NOTIFY_FRAME_MSB                                  12
#define WBM_RELEASE_RING_TX_FW_TX_NOTIFY_FRAME_MASK                                 0x00001c00


/* Description		BUFFER_TIMESTAMP

			Field only valid when SW_release_details_valid is set.
			
			Consumer: SW
			Producer: WBM
			
			This is the Buffer_timestamp field from the TX_MSDU_DETAILS
			 for this frame from the MSDU link descriptor.
			
			Timestamp in units determined by the UMCMN 'TX_TIMESTAMP_RESOLUTION_SELECT' 
			register
			
			<legal all>
*/

#define WBM_RELEASE_RING_TX_BUFFER_TIMESTAMP_OFFSET                                 0x00000010
#define WBM_RELEASE_RING_TX_BUFFER_TIMESTAMP_LSB                                    13
#define WBM_RELEASE_RING_TX_BUFFER_TIMESTAMP_MSB                                    31
#define WBM_RELEASE_RING_TX_BUFFER_TIMESTAMP_MASK                                   0xffffe000


/* Description		TX_RATE_STATS

			Consumer: TQM/SW
			Producer: SW/SCH(from TXPCU, PDG) /WBM (from RXDMA)
			
			Details for command execution tracking purposes.
*/


/* Description		TX_RATE_STATS_INFO_VALID

			When set all other fields in this STRUCT contain valid info.
			
			
			When clear, none of the other fields contain valid info.
			
			<legal all>
*/

#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TX_RATE_STATS_INFO_VALID_OFFSET           0x00000014
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TX_RATE_STATS_INFO_VALID_LSB              0
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TX_RATE_STATS_INFO_VALID_MSB              0
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TX_RATE_STATS_INFO_VALID_MASK             0x00000001


/* Description		TRANSMIT_BW

			Field only valid when Tx_rate_stats_info_valid is set
			
			Indicates the BW of the upcoming transmission that shall
			 likely start in about 3 -4 us on the medium
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_BW_OFFSET                        0x00000014
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_BW_LSB                           1
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_BW_MSB                           3
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_BW_MASK                          0x0000000e


/* Description		TRANSMIT_PKT_TYPE

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			Not valid when in SW transmit mode
			
			The packet type
			<enum 0 dot11a>802.11a PPDU type
			<enum 1 dot11b>802.11b PPDU type
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			<enum 3 dot11ac>802.11ac PPDU type
			<enum 4 dot11ax>802.11ax PPDU type
			<enum 5 dot11ba>802.11ba (WUR) PPDU type
			<enum 6 dot11be>802.11be PPDU type
			<enum 7 dot11az>802.11az (ranging) PPDU type
			<enum 8 dot11n_gf>802.11n Green Field PPDU type (unsupported
			 & aborted)
*/

#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_PKT_TYPE_OFFSET                  0x00000014
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_PKT_TYPE_LSB                     4
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_PKT_TYPE_MSB                     7
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_PKT_TYPE_MASK                    0x000000f0


/* Description		TRANSMIT_STBC

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			Not valid when in SW transmit mode
			
			When set, STBC transmission rate was used.
*/

#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_STBC_OFFSET                      0x00000014
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_STBC_LSB                         8
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_STBC_MSB                         8
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_STBC_MASK                        0x00000100


/* Description		TRANSMIT_LDPC

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			Not valid when in SW transmit mode
			
			When set, use LDPC transmission rates
*/

#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_LDPC_OFFSET                      0x00000014
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_LDPC_LSB                         9
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_LDPC_MSB                         9
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_LDPC_MASK                        0x00000200


/* Description		TRANSMIT_SGI

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			Not valid when in SW transmit mode
			
			Specify the right GI for HE-Ranging NDPs (11az)/Short NDP.
			
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > HE related GI
			<enum 3     3_2_us_sgi > HE related GI
			<legal 0 - 3>
*/

#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_SGI_OFFSET                       0x00000014
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_SGI_LSB                          10
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_SGI_MSB                          11
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_SGI_MASK                         0x00000c00


/* Description		TRANSMIT_MCS

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			Not valid when in SW transmit mode
			
			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_MCS_OFFSET                       0x00000014
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_MCS_LSB                          12
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_MCS_MSB                          15
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_MCS_MASK                         0x0000f000


/* Description		OFDMA_TRANSMISSION

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			
			Set when the transmission was an OFDMA transmission (DL 
			or UL).
			<legal all>
*/

#define WBM_RELEASE_RING_TX_TX_RATE_STATS_OFDMA_TRANSMISSION_OFFSET                 0x00000014
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_OFDMA_TRANSMISSION_LSB                    16
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_OFDMA_TRANSMISSION_MSB                    16
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_OFDMA_TRANSMISSION_MASK                   0x00010000


/* Description		TONES_IN_RU

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG.
			Not valid when in SW transmit mode
			
			The number of tones in the RU used.
			<legal all>
*/

#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TONES_IN_RU_OFFSET                        0x00000014
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TONES_IN_RU_LSB                           17
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TONES_IN_RU_MSB                           28
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TONES_IN_RU_MASK                          0x1ffe0000


/* Description		TRANSMIT_NSS

			Field only valid when Tx_rate_stats_info_valid is set
			
			Field filled in by PDG
			Not valid when in SW transmit mode
			
			The number of spatial streams used in the transmission
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_NSS_OFFSET                       0x00000014
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_NSS_LSB                          29
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_NSS_MSB                          31
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_TRANSMIT_NSS_MASK                         0xe0000000


/* Description		PPDU_TRANSMISSION_TSF

			Field only valid when Tx_rate_stats_info_valid is set
			
			Based on a HWSCH configuration register setting, this field
			 either contains:
			
			Lower 32 bits of the TSF, snapshot of this value when transmission
			 of the PPDU containing the frame finished.
			OR
			Lower 32 bits of the TSF, snapshot of this value when transmission
			 of the PPDU containing the frame started
			
			<legal all>
*/

#define WBM_RELEASE_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_OFFSET              0x00000018
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_LSB                 0
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_MSB                 31
#define WBM_RELEASE_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_MASK                0xffffffff


/* Description		SW_PEER_ID

			Field only valid when Release_source_module is set to release_source_TQM
			
			
			1) Release of msdu buffer due to drop_frame = 1. Flow is
			 not fetched and hence sw_peer_id and tid = 0
			buffer_or_desc_type = e_num 0 MSDU_rel_buffertqm_release_reason
			 = e_num 1 tqm_rr_rem_cmd_rem
			
			
			2) Release of msdu buffer due to Flow is not fetched and
			 hence sw_peer_id and tid = 0
			buffer_or_desc_type = e_num 0 MSDU_rel_buffertqm_release_reason
			 = e_num 1 tqm_rr_rem_cmd_rem
			
			
			3) Release of msdu link due to remove_mpdu or acked_mpdu
			 command.
			buffer_or_desc_type = e_num1 msdu_link_descriptortqm_release_reason
			 can be:e_num 1 tqm_rr_rem_cmd_reme_num 2 tqm_rr_rem_cmd_tx
			
			e_num 3 tqm_rr_rem_cmd_notxe_num 4 tqm_rr_rem_cmd_aged (this
			 e_num is used for REMOVE_MPDU as well as REMOVE_MSDU).
			
			Sw_peer_id from the TX_MSDU_FLOW descriptor or TX_MPDU_QUEUE
			 descriptor
			<legal all>
*/

#define WBM_RELEASE_RING_TX_SW_PEER_ID_OFFSET                                       0x0000001c
#define WBM_RELEASE_RING_TX_SW_PEER_ID_LSB                                          0
#define WBM_RELEASE_RING_TX_SW_PEER_ID_MSB                                          15
#define WBM_RELEASE_RING_TX_SW_PEER_ID_MASK                                         0x0000ffff


/* Description		TID

			Field only valid when Release_source_module is set to release_source_TQM
			
			
			1) Release of msdu buffer due to drop_frame = 1. Flow is
			 not fetched and hence sw_peer_id and tid = 0
			buffer_or_desc_type = e_num 0 MSDU_rel_buffertqm_release_reason
			 = e_num 1 tqm_rr_rem_cmd_rem
			
			
			2) Release of msdu buffer due to Flow is not fetched and
			 hence sw_peer_id and tid = 0
			buffer_or_desc_type = e_num 0 MSDU_rel_buffertqm_release_reason
			 = e_num 1 tqm_rr_rem_cmd_rem
			
			
			3) Release of msdu link due to remove_mpdu or acked_mpdu
			 command.
			buffer_or_desc_type = e_num1 msdu_link_descriptortqm_release_reason
			 can be:e_num 1 tqm_rr_rem_cmd_reme_num 2 tqm_rr_rem_cmd_tx
			
			e_num 3 tqm_rr_rem_cmd_notxe_num 4 tqm_rr_rem_cmd_aged (this
			 e_num is used for REMOVE_MPDU as well as REMOVE_MSDU).
			
			
			This field represents the TID from the TX_MSDU_FLOW descriptor
			 or TX_MPDU_QUEUE descriptor
			
			 <legal all>
*/

#define WBM_RELEASE_RING_TX_TID_OFFSET                                              0x0000001c
#define WBM_RELEASE_RING_TX_TID_LSB                                                 16
#define WBM_RELEASE_RING_TX_TID_MSB                                                 19
#define WBM_RELEASE_RING_TX_TID_MASK                                                0x000f0000


/* Description		TQM_STATUS_NUMBER_31_24

			Field only valid when Release_source_module is set to release_source_TQM
			
			
			The value in this field is equal to value of the 'TQM_CMD_Number' 
			field from the TQM command or the 'TQM_add_cmd_Number' field
			 from the TQM entrance ring descriptor MSB 8-bits.
			
			This field helps to correlate the statuses with the TQM 
			commands.
			
			 <legal all>
*/

#define WBM_RELEASE_RING_TX_TQM_STATUS_NUMBER_31_24_OFFSET                          0x0000001c
#define WBM_RELEASE_RING_TX_TQM_STATUS_NUMBER_31_24_LSB                             20
#define WBM_RELEASE_RING_TX_TQM_STATUS_NUMBER_31_24_MSB                             27
#define WBM_RELEASE_RING_TX_TQM_STATUS_NUMBER_31_24_MASK                            0x0ff00000


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

#define WBM_RELEASE_RING_TX_LOOPING_COUNT_OFFSET                                    0x0000001c
#define WBM_RELEASE_RING_TX_LOOPING_COUNT_LSB                                       28
#define WBM_RELEASE_RING_TX_LOOPING_COUNT_MSB                                       31
#define WBM_RELEASE_RING_TX_LOOPING_COUNT_MASK                                      0xf0000000



#endif   // WBM_RELEASE_RING_TX
