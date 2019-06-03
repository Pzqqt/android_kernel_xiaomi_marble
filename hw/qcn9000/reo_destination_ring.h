/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

#ifndef _REO_DESTINATION_RING_H_
#define _REO_DESTINATION_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#include "buffer_addr_info.h"
#include "rx_mpdu_desc_info.h"
#include "rx_msdu_desc_info.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0-1	struct buffer_addr_info buf_or_link_desc_addr_info;
//	2-3	struct rx_mpdu_desc_info rx_mpdu_desc_info_details;
//	4-5	struct rx_msdu_desc_info rx_msdu_desc_info_details;
//	6	rx_reo_queue_desc_addr_31_0[31:0]
//	7	rx_reo_queue_desc_addr_39_32[7:0], reo_dest_buffer_type[8], reo_push_reason[10:9], reo_error_code[15:11], receive_queue_number[31:16]
//	8	soft_reorder_info_valid[0], reorder_opcode[4:1], reorder_slot_index[12:5], mpdu_fragment_number[16:13], reserved_8a[31:17]
//	9	reserved_9a[31:0]
//	10	reserved_10a[31:0]
//	11	reserved_11a[31:0]
//	12	reserved_12a[31:0]
//	13	reserved_13a[31:0]
//	14	reserved_14a[31:0]
//	15	reserved_15[19:0], ring_id[27:20], looping_count[31:28]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_REO_DESTINATION_RING 16

struct reo_destination_ring {
    struct            buffer_addr_info                       buf_or_link_desc_addr_info;
    struct            rx_mpdu_desc_info                       rx_mpdu_desc_info_details;
    struct            rx_msdu_desc_info                       rx_msdu_desc_info_details;
             uint32_t rx_reo_queue_desc_addr_31_0     : 32; //[31:0]
             uint32_t rx_reo_queue_desc_addr_39_32    :  8, //[7:0]
                      reo_dest_buffer_type            :  1, //[8]
                      reo_push_reason                 :  2, //[10:9]
                      reo_error_code                  :  5, //[15:11]
                      receive_queue_number            : 16; //[31:16]
             uint32_t soft_reorder_info_valid         :  1, //[0]
                      reorder_opcode                  :  4, //[4:1]
                      reorder_slot_index              :  8, //[12:5]
                      mpdu_fragment_number            :  4, //[16:13]
                      reserved_8a                     : 15; //[31:17]
             uint32_t reserved_9a                     : 32; //[31:0]
             uint32_t reserved_10a                    : 32; //[31:0]
             uint32_t reserved_11a                    : 32; //[31:0]
             uint32_t reserved_12a                    : 32; //[31:0]
             uint32_t reserved_13a                    : 32; //[31:0]
             uint32_t reserved_14a                    : 32; //[31:0]
             uint32_t reserved_15                     : 20, //[19:0]
                      ring_id                         :  8, //[27:20]
                      looping_count                   :  4; //[31:28]
};

/*

struct buffer_addr_info buf_or_link_desc_addr_info
			
			Consumer: REO/SW/FW
			
			Producer: RXDMA
			
			
			
			Details of the physical address of the a buffer or MSDU
			link descriptor

struct rx_mpdu_desc_info rx_mpdu_desc_info_details
			
			Consumer: REO/SW/FW
			
			Producer: RXDMA
			
			
			
			General information related to the MPDU that is passed
			on from REO entrance ring to the REO destination ring

struct rx_msdu_desc_info rx_msdu_desc_info_details
			
			General information related to the MSDU that is passed
			on from RXDMA all the way to to the REO destination ring.

rx_reo_queue_desc_addr_31_0
			
			Consumer: REO
			
			Producer: RXDMA
			
			
			
			Address (lower 32 bits) of the REO queue descriptor. 
			
			<legal all>

rx_reo_queue_desc_addr_39_32
			
			Consumer: REO
			
			Producer: RXDMA
			
			
			
			Address (upper 8 bits) of the REO queue descriptor. 
			
			<legal all>

reo_dest_buffer_type
			
			Indicates the type of address provided in the
			'Buf_or_link_desc_addr_info'
			
			
			
			<enum 0 MSDU_buf_address> The address of an MSDU buffer
			
			<enum 1 MSDU_link_desc_address> The address of the MSDU
			link descriptor. 
			
			
			
			<legal all>

reo_push_reason
			
			Indicates why REO pushed the frame to this exit ring
			
			
			
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

receive_queue_number
			
			This field indicates the REO MPDU reorder queue ID from
			which this frame originated. This field is populated from a
			field with the same name in the RX_REO_QUEUE descriptor.
			
			<legal all>

soft_reorder_info_valid
			
			When set, REO has been instructed to not perform the
			actual re-ordering of frames for this queue, but just to
			insert the reorder opcodes
			
			<legal all>

reorder_opcode
			
			Field is valid when 'Soft_reorder_info_valid' is set.
			This field is always valid for debug purpose as well.
			
			Details are in the MLD.
			
			
			
			<enum 0 invalid>
			
			<enum 1 fwdcur_fwdbuf>
			
			<enum 2 fwdbuf_fwdcur>
			
			<enum 3 qcur>
			
			<enum 4 fwdbuf_qcur>
			
			<enum 5 fwdbuf_drop>
			
			<enum 6 fwdall_drop>
			
			<enum 7 fwdall_qcur>
			
			<enum 8 reserved_reo_opcode_1>
			
			<enum 9 dropcur>  the error reason code is in
			reo_error_code field.
			
			<enum 10 reserved_reo_opcode_2>
			
			<enum 11 reserved_reo_opcode_3>
			
			<enum 12 reserved_reo_opcode_4>
			
			<enum 13 reserved_reo_opcode_5>
			
			<enum 14 reserved_reo_opcode_6>
			
			<enum 15 reserved_reo_opcode_7>
			
			
			
			<legal all>

reorder_slot_index
			
			Field only valid when 'Soft_reorder_info_valid' is set.
			
			
			
			TODO: add description
			
			
			
			<legal all>

mpdu_fragment_number
			
			Field only valid when Rx_mpdu_desc_info_details.
			Fragment_flag is set.
			
			
			
			The fragment number from the 802.11 header.
			
			
			
			Note that the sequence number is embedded in the field:
			Rx_mpdu_desc_info_details. Mpdu_sequence_number
			
			
			
			<legal all>

reserved_8a
			
			<legal 0>

reserved_9a
			
			<legal 0>

reserved_10a
			
			<legal 0>

reserved_11a
			
			<legal 0>

reserved_12a
			
			<legal 0>

reserved_13a
			
			<legal 0>

reserved_14a
			
			<legal 0>

reserved_15
			
			<legal 0>

ring_id
			
			The buffer pointer ring ID.
			
			0 refers to the IDLE ring
			
			1 - N refers to other rings
			
			
			
			Helps with debugging when dumping ring contents.
			
			<legal all>

looping_count
			
			A count value that indicates the number of times the
			producer of entries into this Ring has looped around the
			ring.
			
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


 /* EXTERNAL REFERENCE : struct buffer_addr_info buf_or_link_desc_addr_info */ 


/* Description		REO_DESTINATION_RING_0_BUF_OR_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0
			
			Address (lower 32 bits) of the MSDU buffer OR
			MSDU_EXTENSION descriptor OR Link Descriptor
			
			
			
			In case of 'NULL' pointer, this field is set to 0
			
			<legal all>
*/
#define REO_DESTINATION_RING_0_BUF_OR_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET 0x00000000
#define REO_DESTINATION_RING_0_BUF_OR_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB 0
#define REO_DESTINATION_RING_0_BUF_OR_LINK_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK 0xffffffff

/* Description		REO_DESTINATION_RING_1_BUF_OR_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32
			
			Address (upper 8 bits) of the MSDU buffer OR
			MSDU_EXTENSION descriptor OR Link Descriptor
			
			
			
			In case of 'NULL' pointer, this field is set to 0
			
			<legal all>
*/
#define REO_DESTINATION_RING_1_BUF_OR_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET 0x00000004
#define REO_DESTINATION_RING_1_BUF_OR_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB 0
#define REO_DESTINATION_RING_1_BUF_OR_LINK_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK 0x000000ff

/* Description		REO_DESTINATION_RING_1_BUF_OR_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER
			
			Consumer: WBM
			
			Producer: SW/FW
			
			
			
			In case of 'NULL' pointer, this field is set to 0
			
			
			
			Indicates to which buffer manager the buffer OR
			MSDU_EXTENSION descriptor OR link descriptor that is being
			pointed to shall be returned after the frame has been
			processed. It is used by WBM for routing purposes.
			
			
			
			<enum 0 WBM_IDLE_BUF_LIST> This buffer shall be returned
			to the WMB buffer idle list
			
			<enum 1 WBM_IDLE_DESC_LIST> This buffer shall be
			returned to the WMB idle link descriptor idle list
			
			<enum 2 FW_BM> This buffer shall be returned to the FW
			
			<enum 3 SW0_BM> This buffer shall be returned to the SW,
			ring 0
			
			<enum 4 SW1_BM> This buffer shall be returned to the SW,
			ring 1
			
			<enum 5 SW2_BM> This buffer shall be returned to the SW,
			ring 2
			
			<enum 6 SW3_BM> This buffer shall be returned to the SW,
			ring 3
			
			<enum 7 SW4_BM> This buffer shall be returned to the SW,
			ring 4
			
			
			
			<legal all>
*/
#define REO_DESTINATION_RING_1_BUF_OR_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET 0x00000004
#define REO_DESTINATION_RING_1_BUF_OR_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB 8
#define REO_DESTINATION_RING_1_BUF_OR_LINK_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK 0x00000700

/* Description		REO_DESTINATION_RING_1_BUF_OR_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE
			
			Cookie field exclusively used by SW. 
			
			
			
			In case of 'NULL' pointer, this field is set to 0
			
			
			
			HW ignores the contents, accept that it passes the
			programmed value on to other descriptors together with the
			physical address 
			
			
			
			Field can be used by SW to for example associate the
			buffers physical address with the virtual address
			
			The bit definitions as used by SW are within SW HLD
			specification
			
			
			
			NOTE:
			
			The three most significant bits can have a special
			meaning in case this struct is embedded in a TX_MPDU_DETAILS
			STRUCT, and field transmit_bw_restriction is set
			
			
			
			In case of NON punctured transmission:
			
			Sw_buffer_cookie[20:19] = 2'b00: 20 MHz TX only
			
			Sw_buffer_cookie[20:19] = 2'b01: 40 MHz TX only
			
			Sw_buffer_cookie[20:19] = 2'b10: 80 MHz TX only
			
			Sw_buffer_cookie[20:19] = 2'b11: 160 MHz TX only
			
			
			
			In case of punctured transmission:
			
			Sw_buffer_cookie[20:18] = 3'b000: pattern 0 only
			
			Sw_buffer_cookie[20:18] = 3'b001: pattern 1 only
			
			Sw_buffer_cookie[20:18] = 3'b010: pattern 2 only
			
			Sw_buffer_cookie[20:18] = 3'b011: pattern 3 only
			
			Sw_buffer_cookie[20:18] = 3'b100: pattern 4 only
			
			Sw_buffer_cookie[20:18] = 3'b101: pattern 5 only
			
			Sw_buffer_cookie[20:18] = 3'b110: pattern 6 only
			
			Sw_buffer_cookie[20:18] = 3'b111: pattern 7 only
			
			
			
			Note: a punctured transmission is indicated by the
			presence of TLV TX_PUNCTURE_SETUP embedded in the scheduler
			TLV
			
			
			
			<legal all>
*/
#define REO_DESTINATION_RING_1_BUF_OR_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET 0x00000004
#define REO_DESTINATION_RING_1_BUF_OR_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB 11
#define REO_DESTINATION_RING_1_BUF_OR_LINK_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK 0xfffff800

 /* EXTERNAL REFERENCE : struct rx_mpdu_desc_info rx_mpdu_desc_info_details */ 


/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT
			
			Consumer: REO/SW/FW
			
			Producer: RXDMA
			
			
			
			The number of MSDUs within the MPDU 
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_LSB 0
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MASK 0x000000ff

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MPDU_SEQUENCE_NUMBER
			
			Consumer: REO/SW/FW
			
			Producer: RXDMA
			
			
			
			The field can have two different meanings based on the
			setting of field 'BAR_frame':
			
			
			
			'BAR_frame' is NOT set:
			
			The MPDU sequence number of the received frame.
			
			
			
			'BAR_frame' is set.
			
			The MPDU Start sequence number from the BAR frame
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MPDU_SEQUENCE_NUMBER_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MPDU_SEQUENCE_NUMBER_LSB 8
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MPDU_SEQUENCE_NUMBER_MASK 0x000fff00

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG
			
			Consumer: REO/SW/FW
			
			Producer: RXDMA
			
			
			
			When set, this MPDU is a fragment and REO should forward
			this fragment MPDU to the REO destination ring without any
			reorder checks, pn checks or bitmap update. This implies
			that REO is forwarding the pointer to the MSDU link
			descriptor. The destination ring is coming from a
			programmable register setting in REO
			
			
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_LSB 20
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MASK 0x00100000

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT
			
			Consumer: REO/SW/FW
			
			Producer: RXDMA
			
			
			
			The retry bit setting from the MPDU header of the
			received frame
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_LSB 21
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MASK 0x00200000

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG
			
			Consumer: REO/SW/FW
			
			Producer: RXDMA
			
			
			
			When set, the MPDU was received as part of an A-MPDU.
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_LSB 22
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MASK 0x00400000

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME
			
			Consumer: REO/SW/FW
			
			Producer: RXDMA
			
			
			
			When set, the received frame is a BAR frame. After
			processing, this frame shall be pushed to SW or deleted.
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_LSB 23
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MASK 0x00800000

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO
			
			Consumer: REO/SW/FW
			
			Producer: RXDMA
			
			
			
			Copied here by RXDMA from RX_MPDU_END
			
			When not set, REO will Not perform a PN sequence number
			check
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_LSB 24
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MASK 0x01000000

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_SA_IS_VALID
			
			When set, OLE found a valid SA entry for all MSDUs in
			this MPDU
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_SA_IS_VALID_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_SA_IS_VALID_LSB 25
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_SA_IS_VALID_MASK 0x02000000

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_SA_IDX_TIMEOUT
			
			When set, at least 1 MSDU within the MPDU has an
			unsuccessful MAC source address search due to the expiration
			of the search timer.
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_SA_IDX_TIMEOUT_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_SA_IDX_TIMEOUT_LSB 26
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_SA_IDX_TIMEOUT_MASK 0x04000000

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_DA_IS_VALID
			
			When set, OLE found a valid DA entry for all MSDUs in
			this MPDU
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_DA_IS_VALID_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_DA_IS_VALID_LSB 27
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_DA_IS_VALID_MASK 0x08000000

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_DA_IS_MCBC
			
			Field Only valid if da_is_valid is set
			
			
			
			When set, at least one of the DA addresses is a
			Multicast or Broadcast address.
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_DA_IS_MCBC_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_DA_IS_MCBC_LSB 28
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_DA_IS_MCBC_MASK 0x10000000

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_DA_IDX_TIMEOUT
			
			When set, at least 1 MSDU within the MPDU has an
			unsuccessful MAC destination address search due to the
			expiration of the search timer.
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_DA_IDX_TIMEOUT_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_DA_IDX_TIMEOUT_LSB 29
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_DA_IDX_TIMEOUT_MASK 0x20000000

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU
			
			Field only valid when first_msdu_in_mpdu_flag is set.
			
			
			
			When set, the contents in the MSDU buffer contains a
			'RAW' MPDU. This 'RAW' MPDU might be spread out over
			multiple MSDU buffers.
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_LSB 30
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MASK 0x40000000

/* Description		REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG
			
			The More Fragment bit setting from the MPDU header of
			the received frame
			
			
			
			<legal all>
*/
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_LSB 31
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MASK 0x80000000

/* Description		REO_DESTINATION_RING_3_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA
			
			Meta data that SW has programmed in the Peer table entry
			of the transmitting STA.
			
			<legal all>
*/
#define REO_DESTINATION_RING_3_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_OFFSET 0x0000000c
#define REO_DESTINATION_RING_3_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_LSB 0
#define REO_DESTINATION_RING_3_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MASK 0xffffffff

 /* EXTERNAL REFERENCE : struct rx_msdu_desc_info rx_msdu_desc_info_details */ 


/* Description		REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU 
			
			
			
			<enum 0 Not_first_msdu> This is not the first MSDU in
			the MPDU. 
			
			<enum 1 first_msdu> This MSDU is the first one in the
			MPDU.
			
			
			
			<legal all>
*/
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_LSB 0
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MASK 0x00000001

/* Description		REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG
			
			Consumer: WBM/REO/SW/FW
			
			Producer: RXDMA
			
			
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			
			
			<enum 0 Not_last_msdu> There are more MSDUs linked to
			this MSDU that belongs to this MPDU 
			
			<enum 1 Last_msdu> this MSDU is the last one in the
			MPDU. This setting is only allowed in combination with
			'Msdu_continuation' set to 0. This implies that when an msdu
			is spread out over multiple buffers and thus
			msdu_continuation is set, only for the very last buffer of
			the msdu, can the 'last_msdu_in_mpdu_flag' be set.
			
			
			
			When both first_msdu_in_mpdu_flag and
			last_msdu_in_mpdu_flag are set, the MPDU that this MSDU
			belongs to only contains a single MSDU.
			
			
			
			
			
			<legal all>
*/
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_LSB 1
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MASK 0x00000002

/* Description		REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION
			
			When set, this MSDU buffer was not able to hold the
			entire MSDU. The next buffer will therefor contain
			additional information related to this MSDU.
			
			
			
			<legal all>
*/
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_LSB 2
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MASK 0x00000004

/* Description		REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH
			
			Parsed from RX_MSDU_START TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the First
			buffer used by MSDU.
			
			 
			
			Full MSDU length in bytes after decapsulation. 
			
			
			
			This field is still valid for MPDU frames without
			A-MSDU.  It still represents MSDU length after decapsulation
			
			
			
			Or in case of RAW MPDUs, it indicates the length of the
			entire MPDU (without FCS field)
			
			<legal all>
*/
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_LSB 3
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MASK 0x0001fff8

/* Description		REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			The ID of the REO exit ring where the MSDU frame shall
			push after (MPDU level) reordering has finished.
			
			
			
			<enum 0 reo_destination_tcl> Reo will push the frame
			into the REO2TCL ring
			
			<enum 1 reo_destination_sw1> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 2 reo_destination_sw2> Reo will push the frame
			into the REO2SW2 ring
			
			<enum 3 reo_destination_sw3> Reo will push the frame
			into the REO2SW3 ring
			
			<enum 4 reo_destination_sw4> Reo will push the frame
			into the REO2SW4 ring
			
			<enum 5 reo_destination_release> Reo will push the frame
			into the REO_release ring
			
			<enum 6 reo_destination_fw> Reo will push the frame into
			the REO2FW ring
			
			<enum 7 reo_destination_sw5> Reo will push the frame
			into the REO2SW5 ring 
			
			<enum 8 reo_destination_sw6> Reo will push the frame
			into the REO2SW6 ring 
			
			 <enum 9 reo_destination_9> REO remaps this <enum 10
			reo_destination_10> REO remaps this 
			
			<enum 11 reo_destination_11> REO remaps this 
			
			<enum 12 reo_destination_12> REO remaps this <enum 13
			reo_destination_13> REO remaps this 
			
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
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_LSB 17
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_REO_DESTINATION_INDICATION_MASK 0x003e0000

/* Description		REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			When set, REO shall drop this MSDU and not forward it to
			any other ring...
			
			<legal all>
*/
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_LSB 22
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MASK 0x00400000

/* Description		REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			Indicates that OLE found a valid SA entry for this MSDU
			
			<legal all>
*/
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_LSB 23
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MASK 0x00800000

/* Description		REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_SA_IDX_TIMEOUT
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			Indicates an unsuccessful MAC source address search due
			to the expiring of the search timer for this MSDU
			
			<legal all>
*/
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_SA_IDX_TIMEOUT_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_SA_IDX_TIMEOUT_LSB 24
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_SA_IDX_TIMEOUT_MASK 0x01000000

/* Description		REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			Indicates that OLE found a valid DA entry for this MSDU
			
			<legal all>
*/
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_LSB 25
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MASK 0x02000000

/* Description		REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC
			
			Field Only valid if da_is_valid is set
			
			
			
			Indicates the DA address was a Multicast of Broadcast
			address for this MSDU
			
			<legal all>
*/
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_LSB 26
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MASK 0x04000000

/* Description		REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_DA_IDX_TIMEOUT
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			Indicates an unsuccessful MAC destination address search
			due to the expiring of the search timer for this MSDU
			
			<legal all>
*/
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_DA_IDX_TIMEOUT_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_DA_IDX_TIMEOUT_LSB 27
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_DA_IDX_TIMEOUT_MASK 0x08000000

/* Description		REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_RESERVED_0A
			
			<legal 0>
*/
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_RESERVED_0A_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_RESERVED_0A_LSB 28
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_DETAILS_RESERVED_0A_MASK 0xf0000000

/* Description		REO_DESTINATION_RING_5_RX_MSDU_DESC_INFO_DETAILS_RESERVED_1A
			
			<legal 0>
*/
#define REO_DESTINATION_RING_5_RX_MSDU_DESC_INFO_DETAILS_RESERVED_1A_OFFSET 0x00000014
#define REO_DESTINATION_RING_5_RX_MSDU_DESC_INFO_DETAILS_RESERVED_1A_LSB 0
#define REO_DESTINATION_RING_5_RX_MSDU_DESC_INFO_DETAILS_RESERVED_1A_MASK 0xffffffff

/* Description		REO_DESTINATION_RING_6_RX_REO_QUEUE_DESC_ADDR_31_0
			
			Consumer: REO
			
			Producer: RXDMA
			
			
			
			Address (lower 32 bits) of the REO queue descriptor. 
			
			<legal all>
*/
#define REO_DESTINATION_RING_6_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET    0x00000018
#define REO_DESTINATION_RING_6_RX_REO_QUEUE_DESC_ADDR_31_0_LSB       0
#define REO_DESTINATION_RING_6_RX_REO_QUEUE_DESC_ADDR_31_0_MASK      0xffffffff

/* Description		REO_DESTINATION_RING_7_RX_REO_QUEUE_DESC_ADDR_39_32
			
			Consumer: REO
			
			Producer: RXDMA
			
			
			
			Address (upper 8 bits) of the REO queue descriptor. 
			
			<legal all>
*/
#define REO_DESTINATION_RING_7_RX_REO_QUEUE_DESC_ADDR_39_32_OFFSET   0x0000001c
#define REO_DESTINATION_RING_7_RX_REO_QUEUE_DESC_ADDR_39_32_LSB      0
#define REO_DESTINATION_RING_7_RX_REO_QUEUE_DESC_ADDR_39_32_MASK     0x000000ff

/* Description		REO_DESTINATION_RING_7_REO_DEST_BUFFER_TYPE
			
			Indicates the type of address provided in the
			'Buf_or_link_desc_addr_info'
			
			
			
			<enum 0 MSDU_buf_address> The address of an MSDU buffer
			
			<enum 1 MSDU_link_desc_address> The address of the MSDU
			link descriptor. 
			
			
			
			<legal all>
*/
#define REO_DESTINATION_RING_7_REO_DEST_BUFFER_TYPE_OFFSET           0x0000001c
#define REO_DESTINATION_RING_7_REO_DEST_BUFFER_TYPE_LSB              8
#define REO_DESTINATION_RING_7_REO_DEST_BUFFER_TYPE_MASK             0x00000100

/* Description		REO_DESTINATION_RING_7_REO_PUSH_REASON
			
			Indicates why REO pushed the frame to this exit ring
			
			
			
			<enum 0 reo_error_detected> Reo detected an error an
			pushed this frame to this queue
			
			<enum 1 reo_routing_instruction> Reo pushed the frame to
			this queue per received routing instructions. No error
			within REO was detected
			
			
			
			
			
			<legal 0 - 1>
*/
#define REO_DESTINATION_RING_7_REO_PUSH_REASON_OFFSET                0x0000001c
#define REO_DESTINATION_RING_7_REO_PUSH_REASON_LSB                   9
#define REO_DESTINATION_RING_7_REO_PUSH_REASON_MASK                  0x00000600

/* Description		REO_DESTINATION_RING_7_REO_ERROR_CODE
			
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
#define REO_DESTINATION_RING_7_REO_ERROR_CODE_OFFSET                 0x0000001c
#define REO_DESTINATION_RING_7_REO_ERROR_CODE_LSB                    11
#define REO_DESTINATION_RING_7_REO_ERROR_CODE_MASK                   0x0000f800

/* Description		REO_DESTINATION_RING_7_RECEIVE_QUEUE_NUMBER
			
			This field indicates the REO MPDU reorder queue ID from
			which this frame originated. This field is populated from a
			field with the same name in the RX_REO_QUEUE descriptor.
			
			<legal all>
*/
#define REO_DESTINATION_RING_7_RECEIVE_QUEUE_NUMBER_OFFSET           0x0000001c
#define REO_DESTINATION_RING_7_RECEIVE_QUEUE_NUMBER_LSB              16
#define REO_DESTINATION_RING_7_RECEIVE_QUEUE_NUMBER_MASK             0xffff0000

/* Description		REO_DESTINATION_RING_8_SOFT_REORDER_INFO_VALID
			
			When set, REO has been instructed to not perform the
			actual re-ordering of frames for this queue, but just to
			insert the reorder opcodes
			
			<legal all>
*/
#define REO_DESTINATION_RING_8_SOFT_REORDER_INFO_VALID_OFFSET        0x00000020
#define REO_DESTINATION_RING_8_SOFT_REORDER_INFO_VALID_LSB           0
#define REO_DESTINATION_RING_8_SOFT_REORDER_INFO_VALID_MASK          0x00000001

/* Description		REO_DESTINATION_RING_8_REORDER_OPCODE
			
			Field is valid when 'Soft_reorder_info_valid' is set.
			This field is always valid for debug purpose as well.
			
			Details are in the MLD.
			
			
			
			<enum 0 invalid>
			
			<enum 1 fwdcur_fwdbuf>
			
			<enum 2 fwdbuf_fwdcur>
			
			<enum 3 qcur>
			
			<enum 4 fwdbuf_qcur>
			
			<enum 5 fwdbuf_drop>
			
			<enum 6 fwdall_drop>
			
			<enum 7 fwdall_qcur>
			
			<enum 8 reserved_reo_opcode_1>
			
			<enum 9 dropcur>  the error reason code is in
			reo_error_code field.
			
			<enum 10 reserved_reo_opcode_2>
			
			<enum 11 reserved_reo_opcode_3>
			
			<enum 12 reserved_reo_opcode_4>
			
			<enum 13 reserved_reo_opcode_5>
			
			<enum 14 reserved_reo_opcode_6>
			
			<enum 15 reserved_reo_opcode_7>
			
			
			
			<legal all>
*/
#define REO_DESTINATION_RING_8_REORDER_OPCODE_OFFSET                 0x00000020
#define REO_DESTINATION_RING_8_REORDER_OPCODE_LSB                    1
#define REO_DESTINATION_RING_8_REORDER_OPCODE_MASK                   0x0000001e

/* Description		REO_DESTINATION_RING_8_REORDER_SLOT_INDEX
			
			Field only valid when 'Soft_reorder_info_valid' is set.
			
			
			
			TODO: add description
			
			
			
			<legal all>
*/
#define REO_DESTINATION_RING_8_REORDER_SLOT_INDEX_OFFSET             0x00000020
#define REO_DESTINATION_RING_8_REORDER_SLOT_INDEX_LSB                5
#define REO_DESTINATION_RING_8_REORDER_SLOT_INDEX_MASK               0x00001fe0

/* Description		REO_DESTINATION_RING_8_MPDU_FRAGMENT_NUMBER
			
			Field only valid when Rx_mpdu_desc_info_details.
			Fragment_flag is set.
			
			
			
			The fragment number from the 802.11 header.
			
			
			
			Note that the sequence number is embedded in the field:
			Rx_mpdu_desc_info_details. Mpdu_sequence_number
			
			
			
			<legal all>
*/
#define REO_DESTINATION_RING_8_MPDU_FRAGMENT_NUMBER_OFFSET           0x00000020
#define REO_DESTINATION_RING_8_MPDU_FRAGMENT_NUMBER_LSB              13
#define REO_DESTINATION_RING_8_MPDU_FRAGMENT_NUMBER_MASK             0x0001e000

/* Description		REO_DESTINATION_RING_8_RESERVED_8A
			
			<legal 0>
*/
#define REO_DESTINATION_RING_8_RESERVED_8A_OFFSET                    0x00000020
#define REO_DESTINATION_RING_8_RESERVED_8A_LSB                       17
#define REO_DESTINATION_RING_8_RESERVED_8A_MASK                      0xfffe0000

/* Description		REO_DESTINATION_RING_9_RESERVED_9A
			
			<legal 0>
*/
#define REO_DESTINATION_RING_9_RESERVED_9A_OFFSET                    0x00000024
#define REO_DESTINATION_RING_9_RESERVED_9A_LSB                       0
#define REO_DESTINATION_RING_9_RESERVED_9A_MASK                      0xffffffff

/* Description		REO_DESTINATION_RING_10_RESERVED_10A
			
			<legal 0>
*/
#define REO_DESTINATION_RING_10_RESERVED_10A_OFFSET                  0x00000028
#define REO_DESTINATION_RING_10_RESERVED_10A_LSB                     0
#define REO_DESTINATION_RING_10_RESERVED_10A_MASK                    0xffffffff

/* Description		REO_DESTINATION_RING_11_RESERVED_11A
			
			<legal 0>
*/
#define REO_DESTINATION_RING_11_RESERVED_11A_OFFSET                  0x0000002c
#define REO_DESTINATION_RING_11_RESERVED_11A_LSB                     0
#define REO_DESTINATION_RING_11_RESERVED_11A_MASK                    0xffffffff

/* Description		REO_DESTINATION_RING_12_RESERVED_12A
			
			<legal 0>
*/
#define REO_DESTINATION_RING_12_RESERVED_12A_OFFSET                  0x00000030
#define REO_DESTINATION_RING_12_RESERVED_12A_LSB                     0
#define REO_DESTINATION_RING_12_RESERVED_12A_MASK                    0xffffffff

/* Description		REO_DESTINATION_RING_13_RESERVED_13A
			
			<legal 0>
*/
#define REO_DESTINATION_RING_13_RESERVED_13A_OFFSET                  0x00000034
#define REO_DESTINATION_RING_13_RESERVED_13A_LSB                     0
#define REO_DESTINATION_RING_13_RESERVED_13A_MASK                    0xffffffff

/* Description		REO_DESTINATION_RING_14_RESERVED_14A
			
			<legal 0>
*/
#define REO_DESTINATION_RING_14_RESERVED_14A_OFFSET                  0x00000038
#define REO_DESTINATION_RING_14_RESERVED_14A_LSB                     0
#define REO_DESTINATION_RING_14_RESERVED_14A_MASK                    0xffffffff

/* Description		REO_DESTINATION_RING_15_RESERVED_15
			
			<legal 0>
*/
#define REO_DESTINATION_RING_15_RESERVED_15_OFFSET                   0x0000003c
#define REO_DESTINATION_RING_15_RESERVED_15_LSB                      0
#define REO_DESTINATION_RING_15_RESERVED_15_MASK                     0x000fffff

/* Description		REO_DESTINATION_RING_15_RING_ID
			
			The buffer pointer ring ID.
			
			0 refers to the IDLE ring
			
			1 - N refers to other rings
			
			
			
			Helps with debugging when dumping ring contents.
			
			<legal all>
*/
#define REO_DESTINATION_RING_15_RING_ID_OFFSET                       0x0000003c
#define REO_DESTINATION_RING_15_RING_ID_LSB                          20
#define REO_DESTINATION_RING_15_RING_ID_MASK                         0x0ff00000

/* Description		REO_DESTINATION_RING_15_LOOPING_COUNT
			
			A count value that indicates the number of times the
			producer of entries into this Ring has looped around the
			ring.
			
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
#define REO_DESTINATION_RING_15_LOOPING_COUNT_OFFSET                 0x0000003c
#define REO_DESTINATION_RING_15_LOOPING_COUNT_LSB                    28
#define REO_DESTINATION_RING_15_LOOPING_COUNT_MASK                   0xf0000000


#endif // _REO_DESTINATION_RING_H_
