/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _WBM_RELEASE_RING_H_
#define _WBM_RELEASE_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#include "buffer_addr_info.h"
#include "tx_rate_stats_info.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0-1	struct buffer_addr_info released_buff_or_desc_addr_info;
//	2	release_source_module[2:0], bm_action[5:3], buffer_or_desc_type[8:6], first_msdu_index[12:9], tqm_release_reason[15:13], rxdma_push_reason[17:16], rxdma_error_code[22:18], reo_push_reason[24:23], reo_error_code[29:25], wbm_internal_error[30], reserved_2[31]
//	3	tqm_status_number[23:0], transmit_count[30:24], reserved_3a[31]
//	4	ack_frame_rssi[7:0], sw_release_details_valid[8], first_msdu[9], last_msdu[10], msdu_part_of_amsdu[11], fw_tx_notify_frame[12], buffer_timestamp[31:13]
//	5-6	struct tx_rate_stats_info tx_rate_stats;
//	7	sw_peer_id[15:0], tid[19:16], ring_id[27:20], looping_count[31:28]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_WBM_RELEASE_RING 8

struct wbm_release_ring {
    struct            buffer_addr_info                       released_buff_or_desc_addr_info;
             uint32_t release_source_module           :  3, //[2:0]
                      bm_action                       :  3, //[5:3]
                      buffer_or_desc_type             :  3, //[8:6]
                      first_msdu_index                :  4, //[12:9]
                      tqm_release_reason              :  3, //[15:13]
                      rxdma_push_reason               :  2, //[17:16]
                      rxdma_error_code                :  5, //[22:18]
                      reo_push_reason                 :  2, //[24:23]
                      reo_error_code                  :  5, //[29:25]
                      wbm_internal_error              :  1, //[30]
                      reserved_2                      :  1; //[31]
             uint32_t tqm_status_number               : 24, //[23:0]
                      transmit_count                  :  7, //[30:24]
                      reserved_3a                     :  1; //[31]
             uint32_t ack_frame_rssi                  :  8, //[7:0]
                      sw_release_details_valid        :  1, //[8]
                      first_msdu                      :  1, //[9]
                      last_msdu                       :  1, //[10]
                      msdu_part_of_amsdu              :  1, //[11]
                      fw_tx_notify_frame              :  1, //[12]
                      buffer_timestamp                : 19; //[31:13]
    struct            tx_rate_stats_info                       tx_rate_stats;
             uint32_t sw_peer_id                      : 16, //[15:0]
                      tid                             :  4, //[19:16]
                      ring_id                         :  8, //[27:20]
                      looping_count                   :  4; //[31:28]
};

/*

struct buffer_addr_info released_buff_or_desc_addr_info
			
			Consumer: WBM/SW/FW
			
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			
			
			Details of the physical address of the buffer or link
			descriptor that is being released. Note that within this
			descriptor, WBM will look at the 'owner' of the released
			buffer/descriptor and forward it to SW/FW is WBM is not the
			owner.

release_source_module
			
			Indicates which module initiated the release of this
			buffer or descriptor
			
			
			
			<enum 0 release_source_TQM> TQM released this buffer or
			descriptor
			
			<enum 1 release_source_RXDMA> RXDMA released this buffer
			or descriptor
			
			<enum 2 release_source_REO> REO released this buffer or
			descriptor
			
			<enum 3 release_source_FW> FW released this buffer or
			descriptor
			
			<enum 4 release_source_SW> SW released this buffer or
			descriptor
			
			<legal 0-4>

bm_action
			
			Consumer: WBM/SW/FW
			
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			
			
			Field only valid when the field return_buffer_manager in
			the Released_buff_or_desc_addr_info indicates:
			
			WBM_IDLE_BUF_LIST or
			
			WBM_IDLE_DESC_LIST
			
			
			
			An MSDU extension descriptor shall never be marked as
			
			
			
			<enum 0 Put_in_idle_list> Put the buffer or descriptor
			back in the idle list. In case of MSDU or MDPU link
			descriptor, BM does not need to check to release any
			individual MSDU buffers
			
			
			
			<enum 1 release_msdu_list > This BM action can only be
			used in combination with buffer_or_desc_type being
			msdu_link_descriptor. Field first_msdu_index points out
			which MSDU pointer in the MSDU link descriptor is the first
			of an MPDU that is released.
			
			BM shall release all the MSDU buffers linked to this
			first MSDU buffer pointer. All related MSDU buffer pointer
			entries shall be set to value 0, which represents the 'NULL
			pointer. When all MSDU buffer pointers in the MSDU link
			descriptor are 'NULL', the MSDU link descriptor itself shall
			also be released.
			
			
			
			<enum 2 Put_in_idle_list_expanded> CURRENTLY NOT
			IMPLEMENTED....
			
			Put the buffer or descriptor back in the idle list. Only
			valid in combination with buffer_or_desc_type indicating
			MDPU_link_descriptor.
			
			BM shall release the MPDU link descriptor as well as all
			MSDUs that are linked to the MPDUs in this descriptor. 
			
			
			
			<legal 0-2>

buffer_or_desc_type
			
			Consumer: WBM/SW/FW
			
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			
			
			Field only valid when WBM is marked as the
			return_buffer_manager in the Released_Buffer_address_info
			
			
			
			Indicates that type of buffer or descriptor is being
			released
			
			
			
			<enum 0 MSDU_rel_buffer> The address points to an MSDU
			buffer 
			
			<enum 1 msdu_link_descriptor> The address points to an
			TX MSDU link descriptor
			
			<enum 2 mpdu_link_descriptor> The address points to an
			MPDU link descriptor
			
			<enum 3 msdu_ext_descriptor > The address points to an
			MSDU extension descriptor.
			
			In case BM finds this one in a release ring, it passes
			it on to FW...
			
			<enum 4 queue_ext_descriptor> The address points to an
			TQM queue extension descriptor. WBM should treat this is the
			same way as a link descriptor. That is, put the 128 byte
			buffer back in the link buffer idle list.
			
			
			
			<legal 0-4>

first_msdu_index
			
			Consumer: WBM/SW/FW
			
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			
			
			Field only valid for the bm_action release_msdu_list.
			
			
			
			The index of the first MSDU in an MSDU link descriptor
			all belonging to the same MPDU.
			
			
			
			<legal 0-6>

tqm_release_reason
			
			Consumer: WBM/SW/FW
			
			Producer: TQM
			
			
			
			Field only valid when Release_source_module is set to
			release_source_TQM
			
			
			
			(rr = Release Reason)
			
			<enum 0 tqm_rr_frame_acked> frame is removed because an
			ACK of BA for it was received 
			
			<enum 1 tqm_rr_rem_cmd_rem> frame is removed because a
			remove command of type Remove_mpdus initiated by SW
			
			<enum 2 tqm_rr_rem_cmd_tx> frame is removed because a
			remove command of type Remove_transmitted_mpdus initiated by
			SW
			
			<enum 3 tqm_rr_rem_cmd_notx> frame is removed because a
			remove command of type Remove_untransmitted_mpdus initiated
			by SW
			
			<enum 4 tqm_rr_rem_cmd_aged> frame is removed because a
			remove command of type Remove_aged_mpdus or
			Remove_aged_msdus initiated by SW
			
			<enum 5 tqm_fw_reason1> frame is removed because a
			remove command where fw indicated that remove reason is
			fw_reason1
			
			<enum 6 tqm_fw_reason2> frame is removed because a
			remove command where fw indicated that remove reason is
			fw_reason1
			
			<enum 7 tqm_fw_reason3> frame is removed because a
			remove command where fw indicated that remove reason is
			fw_reason1
			
			
			
			<legal 0-7>

rxdma_push_reason
			
			Field only valid when Release_source_module is set to
			release_source_RXDMA
			
			
			
			Indicates why rxdma pushed the frame to this ring
			
			
			
			<enum 0 rxdma_error_detected> RXDMA detected an error an
			pushed this frame to this queue
			
			<enum 1 rxdma_routing_instruction> RXDMA pushed the
			frame to this queue per received routing instructions. No
			error within RXDMA was detected
			
			<enum 2 rxdma_rx_flush> RXDMA received an RX_FLUSH. As a
			result the MSDU link descriptor might not have the
			last_msdu_in_mpdu_flag set, but instead WBM might just see a
			NULL pointer in the MSDU link descriptor. This is to be
			considered a normal condition for this scenario.
			
			
			
			<legal 0 - 2>

rxdma_error_code
			
			Field only valid when 'rxdma_push_reason' set to
			'rxdma_error_detected'.
			
			
			
			<enum 0 rxdma_overflow_err>MPDU frame is not complete
			due to a FIFO overflow error in RXPCU.
			
			<enum 1 rxdma_mpdu_length_err>MPDU frame is not complete
			due to receiving incomplete MPDU from the PHY
			
			
			<enum 3 rxdma_decrypt_err>CRYPTO reported a decryption
			error or CRYPTO received an encrypted frame, but did not get
			a valid corresponding key id in the peer entry.
			
			<enum 4 rxdma_tkip_mic_err>CRYPTO reported a TKIP MIC
			error
			
			<enum 5 rxdma_unecrypted_err>CRYPTO reported an
			unencrypted frame error when encrypted was expected
			
			<enum 6 rxdma_msdu_len_err>RX OLE reported an MSDU
			length error
			
			<enum 7 rxdma_msdu_limit_err>RX OLE reported that max
			number of MSDUs allowed in an MPDU got exceeded
			
			<enum 8 rxdma_wifi_parse_err>RX OLE reported a parsing
			error
			
			<enum 9 rxdma_amsdu_parse_err>RX OLE reported an A-MSDU
			parsing error
			
			<enum 10 rxdma_sa_timeout_err>RX OLE reported a timeout
			during SA search
			
			<enum 11 rxdma_da_timeout_err>RX OLE reported a timeout
			during DA search
			
			<enum 12 rxdma_flow_timeout_err>RX OLE reported a
			timeout during flow search
			
			<enum 13 rxdma_flush_request>RXDMA received a flush
			request
			
			<enum 14 rxdma_amsdu_fragment_err>Rx PCU reported A-MSDU
			present as well as a fragmented MPDU. A-MSDU defragmentation
			is not supported in Lithium SW so this is treated as an
			error.

reo_push_reason
			
			Field only valid when Release_source_module is set to
			release_source_REO
			
			
			
			Indicates why REO pushed the frame to this release ring
			
			
			
			<enum 0 reo_error_detected> Reo detected an error an
			pushed this frame to this queue
			
			<enum 1 reo_routing_instruction> Reo pushed the frame to
			this queue per received routing instructions. No error
			within REO was detected
			
			
			
			<legal 0 - 1>

reo_error_code
			
			Field only valid when 'Reo_push_reason' set to
			'reo_error_detected'.
			
			
			
			<enum 0 reo_queue_desc_addr_zero> Reo queue descriptor
			provided in the REO_ENTRANCE ring is set to 0
			
			<enum 1 reo_queue_desc_not_valid> Reo queue descriptor
			valid bit is NOT set
			
			<enum 2 ampdu_in_non_ba> AMPDU frame received without BA
			session having been setup.
			
			<enum 3 non_ba_duplicate> Non-BA session, SN equal to
			SSN, Retry bit set: duplicate frame
			
			<enum 4 ba_duplicate> BA session, duplicate frame
			
			<enum 5 regular_frame_2k_jump> A normal (management/data
			frame) received with 2K jump in SN
			
			<enum 6 bar_frame_2k_jump> A bar received with 2K jump
			in SSN
			
			<enum 7 regular_frame_OOR> A normal (management/data
			frame) received with SN falling within the OOR window
			
			<enum 8 bar_frame_OOR> A bar received with SSN falling
			within the OOR window
			
			<enum 9 bar_frame_no_ba_session> A bar received without
			a BA session
			
			<enum 10 bar_frame_sn_equals_ssn> A bar received with
			SSN equal to SN
			
			<enum 11 pn_check_failed> PN Check Failed packet.
			
			<enum 12 2k_error_handling_flag_set> Frame is forwarded
			as a result of the 'Seq_2k_error_detected_flag' been set in
			the REO Queue descriptor
			
			<enum 13 pn_error_handling_flag_set> Frame is forwarded
			as a result of the 'pn_error_detected_flag' been set in the
			REO Queue descriptor
			
			<enum 14 queue_descriptor_blocked_set> Frame is
			forwarded as a result of the queue descriptor(address) being
			blocked as SW/FW seems to be currently in the process of
			making updates to this descriptor...
			
			
			
			<legal 0-14>

wbm_internal_error
			
			Can only be set by WBM.
			
			
			
			Is set when WBM got a buffer pointer but the action was
			to push it to the idle link descriptor ring or do link
			related activity
			
			OR
			
			Is set when WBM got a link buffer pointer but the action
			was to push it to the buffer  descriptor ring 
			
			
			
			<legal all>

reserved_2
			
			<legal 0>

tqm_status_number
			
			Field only valid when Release_source_module is set to
			release_source_TQM
			
			
			
			The value in this field is equal to value of the
			'TQM_CMD_Number' field the TQM command or the
			'TQM_add_cmd_Number' field from the TQM entrance ring
			descriptor
			
			
			
			This field helps to correlate the statuses with the TQM
			commands.
			
			
			
			NOTE that SW could program this number to be equal to
			the PPDU_ID number in case direct correlation with the PPDU
			ID is desired
			
			
			
			<legal all> 

transmit_count
			
			Field only valid when Release_source_module is set to
			release_source_TQM
			
			
			
			The number of times this frame has been transmitted

reserved_3a
			
			<legal 0>

ack_frame_rssi
			
			This field is only valid when the source is TQM.
			
			
			
			If this frame is removed as the result of the reception
			of an ACK or BA, this field indicates the RSSI of the
			received ACK or BA frame. 
			
			
			
			When the frame is removed as result of a direct remove
			command from the SW,  this field is set to 0x0 (which is
			never a valid value when real RSSI is available)
			
			
			
			<legal all>

sw_release_details_valid
			
			Consumer: SW
			
			Producer: WBM
			
			
			
			When set, some WBM specific release info for SW is
			valid.
			
			This is set when WMB got a 'release_msdu_list' command
			from TQM and the return buffer manager is not WMB. WBM will
			then de-aggregate all the MSDUs and pass them one at a time
			on to the 'buffer owner'
			
			
			
			<legal all>

first_msdu
			
			Field only valid when SW_release_details_valid is set.
			
			
			
			Consumer: SW
			
			Producer: WBM
			
			
			
			When set, this MSDU is the first MSDU pointed to in the
			'release_msdu_list' command.
			
			
			
			<legal all>

last_msdu
			
			Field only valid when SW_release_details_valid is set.
			
			
			
			Consumer: SW
			
			Producer: WBM
			
			
			
			When set, this MSDU is the last MSDU pointed to in the
			'release_msdu_list' command.
			
			
			
			<legal all>

msdu_part_of_amsdu
			
			Field only valid when SW_release_details_valid is set.
			
			
			
			Consumer: SW
			
			Producer: WBM
			
			
			
			When set, this MSDU was part of an A-MSDU in MPDU
			
			<legal all>

fw_tx_notify_frame
			
			Field only valid when SW_release_details_valid is set.
			
			
			
			Consumer: SW
			
			Producer: WBM
			
			
			
			This is the FW_tx_notify_frame field from the
			
			<legal all>

buffer_timestamp
			
			Field only valid when SW_release_details_valid is set.
			
			
			
			Consumer: SW
			
			Producer: WBM
			
			
			
			This is the Buffer_timestamp field from the
			
			<legal all>

struct tx_rate_stats_info tx_rate_stats
			
			Consumer: TQM
			
			Producer: SW/SCH(from TXPCU, PDG)
			
			
			
			Details for command execution tracking purposes. 

sw_peer_id
			
			Field only valid when Release_source_module is set to
			release_source_TQM
			
			
			
			1) Release of msdu buffer due to drop_frame = 1. Flow is
			not fetched and hence sw_peer_id and tid = 0
			
			buffer_or_desc_type = e_num 0
			MSDU_rel_buffertqm_release_reason = e_num 1
			tqm_rr_rem_cmd_rem
			
			
			
			
			
			2) Release of msdu buffer due to Flow is not fetched and
			hence sw_peer_id and tid = 0
			
			buffer_or_desc_type = e_num 0
			MSDU_rel_buffertqm_release_reason = e_num 1
			tqm_rr_rem_cmd_rem
			
			
			
			
			
			3) Release of msdu link due to remove_mpdu or acked_mpdu
			command.
			
			buffer_or_desc_type = e_num1
			msdu_link_descriptortqm_release_reason can be:e_num 1
			tqm_rr_rem_cmd_reme_num 2 tqm_rr_rem_cmd_tx
			
			e_num 3 tqm_rr_rem_cmd_notxe_num 4 tqm_rr_rem_cmd_aged
			
			
			
			Sw_peer_id from the TX_MSDU_FLOW descriptor or
			TX_MPDU_QUEUE descriptor
			
			<legal all>

tid
			
			Field only valid when Release_source_module is set to
			release_source_TQM
			
			
			
			1) Release of msdu buffer due to drop_frame = 1. Flow is
			not fetched and hence sw_peer_id and tid = 0
			
			buffer_or_desc_type = e_num 0
			MSDU_rel_buffertqm_release_reason = e_num 1
			tqm_rr_rem_cmd_rem
			
			
			
			
			
			2) Release of msdu buffer due to Flow is not fetched and
			hence sw_peer_id and tid = 0
			
			buffer_or_desc_type = e_num 0
			MSDU_rel_buffertqm_release_reason = e_num 1
			tqm_rr_rem_cmd_rem
			
			
			
			
			
			3) Release of msdu link due to remove_mpdu or acked_mpdu
			command.
			
			buffer_or_desc_type = e_num1
			msdu_link_descriptortqm_release_reason can be:e_num 1
			tqm_rr_rem_cmd_reme_num 2 tqm_rr_rem_cmd_tx
			
			e_num 3 tqm_rr_rem_cmd_notxe_num 4 tqm_rr_rem_cmd_aged
			
			
			
			
			
			This field represents the TID from the TX_MSDU_FLOW
			descriptor or TX_MPDU_QUEUE descriptor
			
			
			
			 <legal all>

ring_id
			
			Consumer: TQM/REO/RXDMA/SW
			
			Producer: SRNG (of RXDMA)
			
			
			
			For debugging. 
			
			This field is filled in by the SRNG module.
			
			It help to identify the ring that is being looked <legal
			all>

looping_count
			
			Consumer: WBM/SW/FW
			
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			
			
			A count value that indicates the number of times the
			producer of entries into the Buffer Manager Ring has looped
			around the ring.
			
			At initialization time, this value is set to 0. On the
			first loop, this value is set to 1. After the max value is
			reached allowed by the number of bits for this field, the
			count value continues with 0 again.
			
			
			
			In case SW is the consumer of the ring entries, it can
			use this field to figure out up to where the producer of
			entries has created new entries. This eliminates the need to
			check where the head pointer' of the ring is located once
			the SW starts processing an interrupt indicating that new
			entries have been put into this ring...
			
			
			
			Also note that SW if it wants only needs to look at the
			LSB bit of this count value.
			
			<legal all>
*/

#define WBM_RELEASE_RING_0_BUFFER_ADDR_INFO_RELEASED_BUFF_OR_DESC_ADDR_INFO_OFFSET 0x00000000
#define WBM_RELEASE_RING_0_BUFFER_ADDR_INFO_RELEASED_BUFF_OR_DESC_ADDR_INFO_LSB 28
#define WBM_RELEASE_RING_0_BUFFER_ADDR_INFO_RELEASED_BUFF_OR_DESC_ADDR_INFO_MASK 0xffffffff
#define WBM_RELEASE_RING_1_BUFFER_ADDR_INFO_RELEASED_BUFF_OR_DESC_ADDR_INFO_OFFSET 0x00000004
#define WBM_RELEASE_RING_1_BUFFER_ADDR_INFO_RELEASED_BUFF_OR_DESC_ADDR_INFO_LSB 28
#define WBM_RELEASE_RING_1_BUFFER_ADDR_INFO_RELEASED_BUFF_OR_DESC_ADDR_INFO_MASK 0xffffffff

/* Description		WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE
			
			Indicates which module initiated the release of this
			buffer or descriptor
			
			
			
			<enum 0 release_source_TQM> TQM released this buffer or
			descriptor
			
			<enum 1 release_source_RXDMA> RXDMA released this buffer
			or descriptor
			
			<enum 2 release_source_REO> REO released this buffer or
			descriptor
			
			<enum 3 release_source_FW> FW released this buffer or
			descriptor
			
			<enum 4 release_source_SW> SW released this buffer or
			descriptor
			
			<legal 0-4>
*/
#define WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_OFFSET              0x00000008
#define WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_LSB                 0
#define WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_MASK                0x00000007

/* Description		WBM_RELEASE_RING_2_BM_ACTION
			
			Consumer: WBM/SW/FW
			
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			
			
			Field only valid when the field return_buffer_manager in
			the Released_buff_or_desc_addr_info indicates:
			
			WBM_IDLE_BUF_LIST or
			
			WBM_IDLE_DESC_LIST
			
			
			
			An MSDU extension descriptor shall never be marked as
			
			
			
			<enum 0 Put_in_idle_list> Put the buffer or descriptor
			back in the idle list. In case of MSDU or MDPU link
			descriptor, BM does not need to check to release any
			individual MSDU buffers
			
			
			
			<enum 1 release_msdu_list > This BM action can only be
			used in combination with buffer_or_desc_type being
			msdu_link_descriptor. Field first_msdu_index points out
			which MSDU pointer in the MSDU link descriptor is the first
			of an MPDU that is released.
			
			BM shall release all the MSDU buffers linked to this
			first MSDU buffer pointer. All related MSDU buffer pointer
			entries shall be set to value 0, which represents the 'NULL
			pointer. When all MSDU buffer pointers in the MSDU link
			descriptor are 'NULL', the MSDU link descriptor itself shall
			also be released.
			
			
			
			<enum 2 Put_in_idle_list_expanded> CURRENTLY NOT
			IMPLEMENTED....
			
			Put the buffer or descriptor back in the idle list. Only
			valid in combination with buffer_or_desc_type indicating
			MDPU_link_descriptor.
			
			BM shall release the MPDU link descriptor as well as all
			MSDUs that are linked to the MPDUs in this descriptor. 
			
			
			
			<legal 0-2>
*/
#define WBM_RELEASE_RING_2_BM_ACTION_OFFSET                          0x00000008
#define WBM_RELEASE_RING_2_BM_ACTION_LSB                             3
#define WBM_RELEASE_RING_2_BM_ACTION_MASK                            0x00000038

/* Description		WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE
			
			Consumer: WBM/SW/FW
			
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			
			
			Field only valid when WBM is marked as the
			return_buffer_manager in the Released_Buffer_address_info
			
			
			
			Indicates that type of buffer or descriptor is being
			released
			
			
			
			<enum 0 MSDU_rel_buffer> The address points to an MSDU
			buffer 
			
			<enum 1 msdu_link_descriptor> The address points to an
			TX MSDU link descriptor
			
			<enum 2 mpdu_link_descriptor> The address points to an
			MPDU link descriptor
			
			<enum 3 msdu_ext_descriptor > The address points to an
			MSDU extension descriptor.
			
			In case BM finds this one in a release ring, it passes
			it on to FW...
			
			<enum 4 queue_ext_descriptor> The address points to an
			TQM queue extension descriptor. WBM should treat this is the
			same way as a link descriptor. That is, put the 128 byte
			buffer back in the link buffer idle list.
			
			
			
			<legal 0-4>
*/
#define WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE_OFFSET                0x00000008
#define WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE_LSB                   6
#define WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE_MASK                  0x000001c0

/* Description		WBM_RELEASE_RING_2_FIRST_MSDU_INDEX
			
			Consumer: WBM/SW/FW
			
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			
			
			Field only valid for the bm_action release_msdu_list.
			
			
			
			The index of the first MSDU in an MSDU link descriptor
			all belonging to the same MPDU.
			
			
			
			<legal 0-6>
*/
#define WBM_RELEASE_RING_2_FIRST_MSDU_INDEX_OFFSET                   0x00000008
#define WBM_RELEASE_RING_2_FIRST_MSDU_INDEX_LSB                      9
#define WBM_RELEASE_RING_2_FIRST_MSDU_INDEX_MASK                     0x00001e00

/* Description		WBM_RELEASE_RING_2_TQM_RELEASE_REASON
			
			Consumer: WBM/SW/FW
			
			Producer: TQM
			
			
			
			Field only valid when Release_source_module is set to
			release_source_TQM
			
			
			
			(rr = Release Reason)
			
			<enum 0 tqm_rr_frame_acked> frame is removed because an
			ACK of BA for it was received 
			
			<enum 1 tqm_rr_rem_cmd_rem> frame is removed because a
			remove command of type Remove_mpdus initiated by SW
			
			<enum 2 tqm_rr_rem_cmd_tx> frame is removed because a
			remove command of type Remove_transmitted_mpdus initiated by
			SW
			
			<enum 3 tqm_rr_rem_cmd_notx> frame is removed because a
			remove command of type Remove_untransmitted_mpdus initiated
			by SW
			
			<enum 4 tqm_rr_rem_cmd_aged> frame is removed because a
			remove command of type Remove_aged_mpdus or
			Remove_aged_msdus initiated by SW
			
			<enum 5 tqm_fw_reason1> frame is removed because a
			remove command where fw indicated that remove reason is
			fw_reason1
			
			<enum 6 tqm_fw_reason2> frame is removed because a
			remove command where fw indicated that remove reason is
			fw_reason1
			
			<enum 7 tqm_fw_reason3> frame is removed because a
			remove command where fw indicated that remove reason is
			fw_reason1
			
			
			
			<legal 0-7>
*/
#define WBM_RELEASE_RING_2_TQM_RELEASE_REASON_OFFSET                 0x00000008
#define WBM_RELEASE_RING_2_TQM_RELEASE_REASON_LSB                    13
#define WBM_RELEASE_RING_2_TQM_RELEASE_REASON_MASK                   0x0000e000

/* Description		WBM_RELEASE_RING_2_RXDMA_PUSH_REASON
			
			Field only valid when Release_source_module is set to
			release_source_RXDMA
			
			
			
			Indicates why rxdma pushed the frame to this ring
			
			
			
			<enum 0 rxdma_error_detected> RXDMA detected an error an
			pushed this frame to this queue
			
			<enum 1 rxdma_routing_instruction> RXDMA pushed the
			frame to this queue per received routing instructions. No
			error within RXDMA was detected
			
			<enum 2 rxdma_rx_flush> RXDMA received an RX_FLUSH. As a
			result the MSDU link descriptor might not have the
			last_msdu_in_mpdu_flag set, but instead WBM might just see a
			NULL pointer in the MSDU link descriptor. This is to be
			considered a normal condition for this scenario.
			
			
			
			<legal 0 - 2>
*/
#define WBM_RELEASE_RING_2_RXDMA_PUSH_REASON_OFFSET                  0x00000008
#define WBM_RELEASE_RING_2_RXDMA_PUSH_REASON_LSB                     16
#define WBM_RELEASE_RING_2_RXDMA_PUSH_REASON_MASK                    0x00030000

/* Description		WBM_RELEASE_RING_2_RXDMA_ERROR_CODE
			
			Field only valid when 'rxdma_push_reason' set to
			'rxdma_error_detected'.
			
			
			
			<enum 0 rxdma_overflow_err>MPDU frame is not complete
			due to a FIFO overflow error in RXPCU.
			
			<enum 1 rxdma_mpdu_length_err>MPDU frame is not complete
			due to receiving incomplete MPDU from the PHY
			
			
			<enum 3 rxdma_decrypt_err>CRYPTO reported a decryption
			error or CRYPTO received an encrypted frame, but did not get
			a valid corresponding key id in the peer entry.
			
			<enum 4 rxdma_tkip_mic_err>CRYPTO reported a TKIP MIC
			error
			
			<enum 5 rxdma_unecrypted_err>CRYPTO reported an
			unencrypted frame error when encrypted was expected
			
			<enum 6 rxdma_msdu_len_err>RX OLE reported an MSDU
			length error
			
			<enum 7 rxdma_msdu_limit_err>RX OLE reported that max
			number of MSDUs allowed in an MPDU got exceeded
			
			<enum 8 rxdma_wifi_parse_err>RX OLE reported a parsing
			error
			
			<enum 9 rxdma_amsdu_parse_err>RX OLE reported an A-MSDU
			parsing error
			
			<enum 10 rxdma_sa_timeout_err>RX OLE reported a timeout
			during SA search
			
			<enum 11 rxdma_da_timeout_err>RX OLE reported a timeout
			during DA search
			
			<enum 12 rxdma_flow_timeout_err>RX OLE reported a
			timeout during flow search
			
			<enum 13 rxdma_flush_request>RXDMA received a flush
			request
			
			<enum 14 rxdma_amsdu_fragment_err>Rx PCU reported A-MSDU
			present as well as a fragmented MPDU. A-MSDU defragmentation
			is not supported in Lithium SW so this is treated as an
			error.
*/
#define WBM_RELEASE_RING_2_RXDMA_ERROR_CODE_OFFSET                   0x00000008
#define WBM_RELEASE_RING_2_RXDMA_ERROR_CODE_LSB                      18
#define WBM_RELEASE_RING_2_RXDMA_ERROR_CODE_MASK                     0x007c0000

/* Description		WBM_RELEASE_RING_2_REO_PUSH_REASON
			
			Field only valid when Release_source_module is set to
			release_source_REO
			
			
			
			Indicates why REO pushed the frame to this release ring
			
			
			
			<enum 0 reo_error_detected> Reo detected an error an
			pushed this frame to this queue
			
			<enum 1 reo_routing_instruction> Reo pushed the frame to
			this queue per received routing instructions. No error
			within REO was detected
			
			
			
			<legal 0 - 1>
*/
#define WBM_RELEASE_RING_2_REO_PUSH_REASON_OFFSET                    0x00000008
#define WBM_RELEASE_RING_2_REO_PUSH_REASON_LSB                       23
#define WBM_RELEASE_RING_2_REO_PUSH_REASON_MASK                      0x01800000

/* Description		WBM_RELEASE_RING_2_REO_ERROR_CODE
			
			Field only valid when 'Reo_push_reason' set to
			'reo_error_detected'.
			
			
			
			<enum 0 reo_queue_desc_addr_zero> Reo queue descriptor
			provided in the REO_ENTRANCE ring is set to 0
			
			<enum 1 reo_queue_desc_not_valid> Reo queue descriptor
			valid bit is NOT set
			
			<enum 2 ampdu_in_non_ba> AMPDU frame received without BA
			session having been setup.
			
			<enum 3 non_ba_duplicate> Non-BA session, SN equal to
			SSN, Retry bit set: duplicate frame
			
			<enum 4 ba_duplicate> BA session, duplicate frame
			
			<enum 5 regular_frame_2k_jump> A normal (management/data
			frame) received with 2K jump in SN
			
			<enum 6 bar_frame_2k_jump> A bar received with 2K jump
			in SSN
			
			<enum 7 regular_frame_OOR> A normal (management/data
			frame) received with SN falling within the OOR window
			
			<enum 8 bar_frame_OOR> A bar received with SSN falling
			within the OOR window
			
			<enum 9 bar_frame_no_ba_session> A bar received without
			a BA session
			
			<enum 10 bar_frame_sn_equals_ssn> A bar received with
			SSN equal to SN
			
			<enum 11 pn_check_failed> PN Check Failed packet.
			
			<enum 12 2k_error_handling_flag_set> Frame is forwarded
			as a result of the 'Seq_2k_error_detected_flag' been set in
			the REO Queue descriptor
			
			<enum 13 pn_error_handling_flag_set> Frame is forwarded
			as a result of the 'pn_error_detected_flag' been set in the
			REO Queue descriptor
			
			<enum 14 queue_descriptor_blocked_set> Frame is
			forwarded as a result of the queue descriptor(address) being
			blocked as SW/FW seems to be currently in the process of
			making updates to this descriptor...
			
			
			
			<legal 0-14>
*/
#define WBM_RELEASE_RING_2_REO_ERROR_CODE_OFFSET                     0x00000008
#define WBM_RELEASE_RING_2_REO_ERROR_CODE_LSB                        25
#define WBM_RELEASE_RING_2_REO_ERROR_CODE_MASK                       0x3e000000

/* Description		WBM_RELEASE_RING_2_WBM_INTERNAL_ERROR
			
			Can only be set by WBM.
			
			
			
			Is set when WBM got a buffer pointer but the action was
			to push it to the idle link descriptor ring or do link
			related activity
			
			OR
			
			Is set when WBM got a link buffer pointer but the action
			was to push it to the buffer  descriptor ring 
			
			
			
			<legal all>
*/
#define WBM_RELEASE_RING_2_WBM_INTERNAL_ERROR_OFFSET                 0x00000008
#define WBM_RELEASE_RING_2_WBM_INTERNAL_ERROR_LSB                    30
#define WBM_RELEASE_RING_2_WBM_INTERNAL_ERROR_MASK                   0x40000000

/* Description		WBM_RELEASE_RING_2_RESERVED_2
			
			<legal 0>
*/
#define WBM_RELEASE_RING_2_RESERVED_2_OFFSET                         0x00000008
#define WBM_RELEASE_RING_2_RESERVED_2_LSB                            31
#define WBM_RELEASE_RING_2_RESERVED_2_MASK                           0x80000000

/* Description		WBM_RELEASE_RING_3_TQM_STATUS_NUMBER
			
			Field only valid when Release_source_module is set to
			release_source_TQM
			
			
			
			The value in this field is equal to value of the
			'TQM_CMD_Number' field the TQM command or the
			'TQM_add_cmd_Number' field from the TQM entrance ring
			descriptor
			
			
			
			This field helps to correlate the statuses with the TQM
			commands.
			
			
			
			NOTE that SW could program this number to be equal to
			the PPDU_ID number in case direct correlation with the PPDU
			ID is desired
			
			
			
			<legal all> 
*/
#define WBM_RELEASE_RING_3_TQM_STATUS_NUMBER_OFFSET                  0x0000000c
#define WBM_RELEASE_RING_3_TQM_STATUS_NUMBER_LSB                     0
#define WBM_RELEASE_RING_3_TQM_STATUS_NUMBER_MASK                    0x00ffffff

/* Description		WBM_RELEASE_RING_3_TRANSMIT_COUNT
			
			Field only valid when Release_source_module is set to
			release_source_TQM
			
			
			
			The number of times this frame has been transmitted
*/
#define WBM_RELEASE_RING_3_TRANSMIT_COUNT_OFFSET                     0x0000000c
#define WBM_RELEASE_RING_3_TRANSMIT_COUNT_LSB                        24
#define WBM_RELEASE_RING_3_TRANSMIT_COUNT_MASK                       0x7f000000

/* Description		WBM_RELEASE_RING_3_RESERVED_3A
			
			<legal 0>
*/
#define WBM_RELEASE_RING_3_RESERVED_3A_OFFSET                        0x0000000c
#define WBM_RELEASE_RING_3_RESERVED_3A_LSB                           31
#define WBM_RELEASE_RING_3_RESERVED_3A_MASK                          0x80000000

/* Description		WBM_RELEASE_RING_4_ACK_FRAME_RSSI
			
			This field is only valid when the source is TQM.
			
			
			
			If this frame is removed as the result of the reception
			of an ACK or BA, this field indicates the RSSI of the
			received ACK or BA frame. 
			
			
			
			When the frame is removed as result of a direct remove
			command from the SW,  this field is set to 0x0 (which is
			never a valid value when real RSSI is available)
			
			
			
			<legal all>
*/
#define WBM_RELEASE_RING_4_ACK_FRAME_RSSI_OFFSET                     0x00000010
#define WBM_RELEASE_RING_4_ACK_FRAME_RSSI_LSB                        0
#define WBM_RELEASE_RING_4_ACK_FRAME_RSSI_MASK                       0x000000ff

/* Description		WBM_RELEASE_RING_4_SW_RELEASE_DETAILS_VALID
			
			Consumer: SW
			
			Producer: WBM
			
			
			
			When set, some WBM specific release info for SW is
			valid.
			
			This is set when WMB got a 'release_msdu_list' command
			from TQM and the return buffer manager is not WMB. WBM will
			then de-aggregate all the MSDUs and pass them one at a time
			on to the 'buffer owner'
			
			
			
			<legal all>
*/
#define WBM_RELEASE_RING_4_SW_RELEASE_DETAILS_VALID_OFFSET           0x00000010
#define WBM_RELEASE_RING_4_SW_RELEASE_DETAILS_VALID_LSB              8
#define WBM_RELEASE_RING_4_SW_RELEASE_DETAILS_VALID_MASK             0x00000100

/* Description		WBM_RELEASE_RING_4_FIRST_MSDU
			
			Field only valid when SW_release_details_valid is set.
			
			
			
			Consumer: SW
			
			Producer: WBM
			
			
			
			When set, this MSDU is the first MSDU pointed to in the
			'release_msdu_list' command.
			
			
			
			<legal all>
*/
#define WBM_RELEASE_RING_4_FIRST_MSDU_OFFSET                         0x00000010
#define WBM_RELEASE_RING_4_FIRST_MSDU_LSB                            9
#define WBM_RELEASE_RING_4_FIRST_MSDU_MASK                           0x00000200

/* Description		WBM_RELEASE_RING_4_LAST_MSDU
			
			Field only valid when SW_release_details_valid is set.
			
			
			
			Consumer: SW
			
			Producer: WBM
			
			
			
			When set, this MSDU is the last MSDU pointed to in the
			'release_msdu_list' command.
			
			
			
			<legal all>
*/
#define WBM_RELEASE_RING_4_LAST_MSDU_OFFSET                          0x00000010
#define WBM_RELEASE_RING_4_LAST_MSDU_LSB                             10
#define WBM_RELEASE_RING_4_LAST_MSDU_MASK                            0x00000400

/* Description		WBM_RELEASE_RING_4_MSDU_PART_OF_AMSDU
			
			Field only valid when SW_release_details_valid is set.
			
			
			
			Consumer: SW
			
			Producer: WBM
			
			
			
			When set, this MSDU was part of an A-MSDU in MPDU
			
			<legal all>
*/
#define WBM_RELEASE_RING_4_MSDU_PART_OF_AMSDU_OFFSET                 0x00000010
#define WBM_RELEASE_RING_4_MSDU_PART_OF_AMSDU_LSB                    11
#define WBM_RELEASE_RING_4_MSDU_PART_OF_AMSDU_MASK                   0x00000800

/* Description		WBM_RELEASE_RING_4_FW_TX_NOTIFY_FRAME
			
			Field only valid when SW_release_details_valid is set.
			
			
			
			Consumer: SW
			
			Producer: WBM
			
			
			
			This is the FW_tx_notify_frame field from the
			
			<legal all>
*/
#define WBM_RELEASE_RING_4_FW_TX_NOTIFY_FRAME_OFFSET                 0x00000010
#define WBM_RELEASE_RING_4_FW_TX_NOTIFY_FRAME_LSB                    12
#define WBM_RELEASE_RING_4_FW_TX_NOTIFY_FRAME_MASK                   0x00001000

/* Description		WBM_RELEASE_RING_4_BUFFER_TIMESTAMP
			
			Field only valid when SW_release_details_valid is set.
			
			
			
			Consumer: SW
			
			Producer: WBM
			
			
			
			This is the Buffer_timestamp field from the
			
			<legal all>
*/
#define WBM_RELEASE_RING_4_BUFFER_TIMESTAMP_OFFSET                   0x00000010
#define WBM_RELEASE_RING_4_BUFFER_TIMESTAMP_LSB                      13
#define WBM_RELEASE_RING_4_BUFFER_TIMESTAMP_MASK                     0xffffe000
#define WBM_RELEASE_RING_5_TX_RATE_STATS_INFO_TX_RATE_STATS_OFFSET   0x00000014
#define WBM_RELEASE_RING_5_TX_RATE_STATS_INFO_TX_RATE_STATS_LSB      13
#define WBM_RELEASE_RING_5_TX_RATE_STATS_INFO_TX_RATE_STATS_MASK     0xffffffff
#define WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_OFFSET   0x00000018
#define WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_LSB      13
#define WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_MASK     0xffffffff

/* Description		WBM_RELEASE_RING_7_SW_PEER_ID
			
			Field only valid when Release_source_module is set to
			release_source_TQM
			
			
			
			1) Release of msdu buffer due to drop_frame = 1. Flow is
			not fetched and hence sw_peer_id and tid = 0
			
			buffer_or_desc_type = e_num 0
			MSDU_rel_buffertqm_release_reason = e_num 1
			tqm_rr_rem_cmd_rem
			
			
			
			
			
			2) Release of msdu buffer due to Flow is not fetched and
			hence sw_peer_id and tid = 0
			
			buffer_or_desc_type = e_num 0
			MSDU_rel_buffertqm_release_reason = e_num 1
			tqm_rr_rem_cmd_rem
			
			
			
			
			
			3) Release of msdu link due to remove_mpdu or acked_mpdu
			command.
			
			buffer_or_desc_type = e_num1
			msdu_link_descriptortqm_release_reason can be:e_num 1
			tqm_rr_rem_cmd_reme_num 2 tqm_rr_rem_cmd_tx
			
			e_num 3 tqm_rr_rem_cmd_notxe_num 4 tqm_rr_rem_cmd_aged
			
			
			
			Sw_peer_id from the TX_MSDU_FLOW descriptor or
			TX_MPDU_QUEUE descriptor
			
			<legal all>
*/
#define WBM_RELEASE_RING_7_SW_PEER_ID_OFFSET                         0x0000001c
#define WBM_RELEASE_RING_7_SW_PEER_ID_LSB                            0
#define WBM_RELEASE_RING_7_SW_PEER_ID_MASK                           0x0000ffff

/* Description		WBM_RELEASE_RING_7_TID
			
			Field only valid when Release_source_module is set to
			release_source_TQM
			
			
			
			1) Release of msdu buffer due to drop_frame = 1. Flow is
			not fetched and hence sw_peer_id and tid = 0
			
			buffer_or_desc_type = e_num 0
			MSDU_rel_buffertqm_release_reason = e_num 1
			tqm_rr_rem_cmd_rem
			
			
			
			
			
			2) Release of msdu buffer due to Flow is not fetched and
			hence sw_peer_id and tid = 0
			
			buffer_or_desc_type = e_num 0
			MSDU_rel_buffertqm_release_reason = e_num 1
			tqm_rr_rem_cmd_rem
			
			
			
			
			
			3) Release of msdu link due to remove_mpdu or acked_mpdu
			command.
			
			buffer_or_desc_type = e_num1
			msdu_link_descriptortqm_release_reason can be:e_num 1
			tqm_rr_rem_cmd_reme_num 2 tqm_rr_rem_cmd_tx
			
			e_num 3 tqm_rr_rem_cmd_notxe_num 4 tqm_rr_rem_cmd_aged
			
			
			
			
			
			This field represents the TID from the TX_MSDU_FLOW
			descriptor or TX_MPDU_QUEUE descriptor
			
			
			
			 <legal all>
*/
#define WBM_RELEASE_RING_7_TID_OFFSET                                0x0000001c
#define WBM_RELEASE_RING_7_TID_LSB                                   16
#define WBM_RELEASE_RING_7_TID_MASK                                  0x000f0000

/* Description		WBM_RELEASE_RING_7_RING_ID
			
			Consumer: TQM/REO/RXDMA/SW
			
			Producer: SRNG (of RXDMA)
			
			
			
			For debugging. 
			
			This field is filled in by the SRNG module.
			
			It help to identify the ring that is being looked <legal
			all>
*/
#define WBM_RELEASE_RING_7_RING_ID_OFFSET                            0x0000001c
#define WBM_RELEASE_RING_7_RING_ID_LSB                               20
#define WBM_RELEASE_RING_7_RING_ID_MASK                              0x0ff00000

/* Description		WBM_RELEASE_RING_7_LOOPING_COUNT
			
			Consumer: WBM/SW/FW
			
			Producer: SW/TQM/RXDMA/REO/SWITCH
			
			
			
			A count value that indicates the number of times the
			producer of entries into the Buffer Manager Ring has looped
			around the ring.
			
			At initialization time, this value is set to 0. On the
			first loop, this value is set to 1. After the max value is
			reached allowed by the number of bits for this field, the
			count value continues with 0 again.
			
			
			
			In case SW is the consumer of the ring entries, it can
			use this field to figure out up to where the producer of
			entries has created new entries. This eliminates the need to
			check where the head pointer' of the ring is located once
			the SW starts processing an interrupt indicating that new
			entries have been put into this ring...
			
			
			
			Also note that SW if it wants only needs to look at the
			LSB bit of this count value.
			
			<legal all>
*/
#define WBM_RELEASE_RING_7_LOOPING_COUNT_OFFSET                      0x0000001c
#define WBM_RELEASE_RING_7_LOOPING_COUNT_LSB                         28
#define WBM_RELEASE_RING_7_LOOPING_COUNT_MASK                        0xf0000000


#endif // _WBM_RELEASE_RING_H_
