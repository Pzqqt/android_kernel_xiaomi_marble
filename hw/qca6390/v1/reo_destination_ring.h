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

#define REO_DESTINATION_RING_0_BUFFER_ADDR_INFO_BUF_OR_LINK_DESC_ADDR_INFO_OFFSET 0x00000000
#define REO_DESTINATION_RING_0_BUFFER_ADDR_INFO_BUF_OR_LINK_DESC_ADDR_INFO_LSB 28
#define REO_DESTINATION_RING_0_BUFFER_ADDR_INFO_BUF_OR_LINK_DESC_ADDR_INFO_MASK 0xffffffff
#define REO_DESTINATION_RING_1_BUFFER_ADDR_INFO_BUF_OR_LINK_DESC_ADDR_INFO_OFFSET 0x00000004
#define REO_DESTINATION_RING_1_BUFFER_ADDR_INFO_BUF_OR_LINK_DESC_ADDR_INFO_LSB 28
#define REO_DESTINATION_RING_1_BUFFER_ADDR_INFO_BUF_OR_LINK_DESC_ADDR_INFO_MASK 0xffffffff
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET 0x00000008
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_LSB 28
#define REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_MASK 0xffffffff
#define REO_DESTINATION_RING_3_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET 0x0000000c
#define REO_DESTINATION_RING_3_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_LSB 28
#define REO_DESTINATION_RING_3_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_MASK 0xffffffff
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET 0x00000010
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_LSB 28
#define REO_DESTINATION_RING_4_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_MASK 0xffffffff
#define REO_DESTINATION_RING_5_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET 0x00000014
#define REO_DESTINATION_RING_5_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_LSB 28
#define REO_DESTINATION_RING_5_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_MASK 0xffffffff

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
