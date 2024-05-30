/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#ifndef _REO_GET_QUEUE_STATS_STATUS_H_
#define _REO_GET_QUEUE_STATS_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_status_header.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0-1	struct uniform_reo_status_header status_header;
//	2	ssn[11:0], current_index[19:12], reserved_2[31:20]
//	3	pn_31_0[31:0]
//	4	pn_63_32[31:0]
//	5	pn_95_64[31:0]
//	6	pn_127_96[31:0]
//	7	last_rx_enqueue_timestamp[31:0]
//	8	last_rx_dequeue_timestamp[31:0]
//	9	rx_bitmap_31_0[31:0]
//	10	rx_bitmap_63_32[31:0]
//	11	rx_bitmap_95_64[31:0]
//	12	rx_bitmap_127_96[31:0]
//	13	rx_bitmap_159_128[31:0]
//	14	rx_bitmap_191_160[31:0]
//	15	rx_bitmap_223_192[31:0]
//	16	rx_bitmap_255_224[31:0]
//	17	current_mpdu_count[6:0], current_msdu_count[31:7]
//	18	reserved_18[3:0], timeout_count[9:4], forward_due_to_bar_count[15:10], duplicate_count[31:16]
//	19	frames_in_order_count[23:0], bar_received_count[31:24]
//	20	mpdu_frames_processed_count[31:0]
//	21	msdu_frames_processed_count[31:0]
//	22	total_processed_byte_count[31:0]
//	23	late_receive_mpdu_count[11:0], window_jump_2k[15:12], hole_count[31:16]
//	24	reserved_24a[27:0], looping_count[31:28]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_REO_GET_QUEUE_STATS_STATUS 25

struct reo_get_queue_stats_status {
    struct            uniform_reo_status_header                       status_header;
             uint32_t ssn                             : 12, //[11:0]
                      current_index                   :  8, //[19:12]
                      reserved_2                      : 12; //[31:20]
             uint32_t pn_31_0                         : 32; //[31:0]
             uint32_t pn_63_32                        : 32; //[31:0]
             uint32_t pn_95_64                        : 32; //[31:0]
             uint32_t pn_127_96                       : 32; //[31:0]
             uint32_t last_rx_enqueue_timestamp       : 32; //[31:0]
             uint32_t last_rx_dequeue_timestamp       : 32; //[31:0]
             uint32_t rx_bitmap_31_0                  : 32; //[31:0]
             uint32_t rx_bitmap_63_32                 : 32; //[31:0]
             uint32_t rx_bitmap_95_64                 : 32; //[31:0]
             uint32_t rx_bitmap_127_96                : 32; //[31:0]
             uint32_t rx_bitmap_159_128               : 32; //[31:0]
             uint32_t rx_bitmap_191_160               : 32; //[31:0]
             uint32_t rx_bitmap_223_192               : 32; //[31:0]
             uint32_t rx_bitmap_255_224               : 32; //[31:0]
             uint32_t current_mpdu_count              :  7, //[6:0]
                      current_msdu_count              : 25; //[31:7]
             uint32_t reserved_18                     :  4, //[3:0]
                      timeout_count                   :  6, //[9:4]
                      forward_due_to_bar_count        :  6, //[15:10]
                      duplicate_count                 : 16; //[31:16]
             uint32_t frames_in_order_count           : 24, //[23:0]
                      bar_received_count              :  8; //[31:24]
             uint32_t mpdu_frames_processed_count     : 32; //[31:0]
             uint32_t msdu_frames_processed_count     : 32; //[31:0]
             uint32_t total_processed_byte_count      : 32; //[31:0]
             uint32_t late_receive_mpdu_count         : 12, //[11:0]
                      window_jump_2k                  :  4, //[15:12]
                      hole_count                      : 16; //[31:16]
             uint32_t reserved_24a                    : 28, //[27:0]
                      looping_count                   :  4; //[31:28]
};

/*

struct uniform_reo_status_header status_header
			
			Consumer: SW
			
			Producer: REO
			
			
			
			Details that can link this status with the original
			command. It also contains info on how long REO took to
			execute this command.

ssn
			
			Starting Sequence number of the session, this changes
			whenever window moves. (can be filled by SW then maintained
			by REO)
			
			<legal all>

current_index
			
			Points to last forwarded packet
			
			<legal all>

reserved_2
			
			<legal 0>

pn_31_0
			
			
			<legal all>

pn_63_32
			
			Bits [63:32] of the PN number.  
			
			<legal all> 

pn_95_64
			
			Bits [95:64] of the PN number.  
			
			<legal all>

pn_127_96
			
			Bits [127:96] of the PN number.  
			
			<legal all>

last_rx_enqueue_timestamp
			
			Timestamp of arrival of the last MPDU for this queue
			
			<legal all>

last_rx_dequeue_timestamp
			
			Timestamp of forwarding an MPDU
			
			
			
			If the queue is empty when a frame gets received, this
			time shall be initialized to the 'enqueue' timestamp
			
			
			
			Used for aging
			
			<legal all>

rx_bitmap_31_0
			
			When a bit is set, the corresponding frame is currently
			held in the re-order queue.
			
			The bitmap  is Fully managed by HW. 
			
			SW shall init this to 0, and then never ever change it
			
			<legal all>

rx_bitmap_63_32
			
			See Rx_bitmap_31_0 description
			
			<legal all>

rx_bitmap_95_64
			
			See Rx_bitmap_31_0 description
			
			<legal all>

rx_bitmap_127_96
			
			See Rx_bitmap_31_0 description
			
			<legal all>

rx_bitmap_159_128
			
			See Rx_bitmap_31_0 description
			
			<legal all>

rx_bitmap_191_160
			
			See Rx_bitmap_31_0 description
			
			<legal all>

rx_bitmap_223_192
			
			See Rx_bitmap_31_0 description
			
			<legal all>

rx_bitmap_255_224
			
			See Rx_bitmap_31_0 description
			
			<legal all>

current_mpdu_count
			
			The number of MPDUs in the queue.
			
			
			
			<legal all>

current_msdu_count
			
			The number of MSDUs in the queue.
			
			<legal all>

reserved_18
			
			<legal 0>

timeout_count
			
			The number of times that REO started forwarding frames
			even though there is a hole in the bitmap. Forwarding reason
			is Timeout
			
			
			
			The counter saturates and freezes at 0x3F
			
			
			
			<legal all>

forward_due_to_bar_count
			
			The number of times that REO started forwarding frames
			even though there is a hole in the bitmap. Forwarding reason
			is reception of BAR frame.
			
			
			
			The counter saturates and freezes at 0x3F
			
			
			
			<legal all>

duplicate_count
			
			The number of duplicate frames that have been detected
			
			<legal all>

frames_in_order_count
			
			The number of frames that have been received in order
			(without a hole that prevented them from being forwarded
			immediately)
			
			
			
			This corresponds to the Reorder opcodes:
			
			'FWDCUR' and 'FWD BUF'
			
			
			
			<legal all>

bar_received_count
			
			The number of times a BAR frame is received.
			
			
			
			This corresponds to the Reorder opcodes with 'DROP'
			
			
			
			The counter saturates and freezes at 0xFF
			
			<legal all>

mpdu_frames_processed_count
			
			The total number of MPDU frames that have been processed
			by REO. This includes the duplicates.
			
			
			
			<legal all>

msdu_frames_processed_count
			
			The total number of MSDU frames that have been processed
			by REO. This includes the duplicates.
			
			
			
			<legal all>

total_processed_byte_count
			
			An approximation of the number of bytes received for
			this queue. 
			
			
			
			In 64 byte units
			
			<legal all>

late_receive_mpdu_count
			
			The number of MPDUs received after the window had
			already moved on. The 'late' sequence window is defined as
			(Window SSN - 256) - (Window SSN - 1)
			
			
			
			This corresponds with Out of order detection in
			duplicate detect FSM
			
			
			
			The counter saturates and freezes at 0xFFF
			
			
			
			<legal all>

window_jump_2k
			
			The number of times the window moved more then 2K
			
			
			
			The counter saturates and freezes at 0xF
			
			
			
			(Note: field name can not start with number: previous
			2k_window_jump)
			
			
			
			<legal all>

hole_count
			
			The number of times a hole was created in the receive
			bitmap.
			
			
			
			This corresponds to the Reorder opcodes with 'QCUR'
			
			
			
			<legal all>

reserved_24a
			
			<legal 0>

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


 /* EXTERNAL REFERENCE : struct uniform_reo_status_header status_header */ 


/* Description		REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_REO_STATUS_NUMBER
			
			Consumer: SW , DEBUG
			
			Producer: REO
			
			
			
			The value in this field is equal to value of the
			'REO_CMD_Number' field the REO command 
			
			
			
			This field helps to correlate the statuses with the REO
			commands.
			
			
			
			<legal all> 
*/
#define REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_REO_STATUS_NUMBER_OFFSET 0x00000000
#define REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_REO_STATUS_NUMBER_LSB 0
#define REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_REO_STATUS_NUMBER_MASK 0x0000ffff

/* Description		REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_CMD_EXECUTION_TIME
			
			Consumer: DEBUG
			
			Producer: REO 
			
			
			
			The amount of time REO took to excecute the command.
			Note that this time does not include the duration of the
			command waiting in the command ring, before the execution
			started.
			
			
			
			In us.
			
			
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_CMD_EXECUTION_TIME_OFFSET 0x00000000
#define REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_CMD_EXECUTION_TIME_LSB 16
#define REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_CMD_EXECUTION_TIME_MASK 0x03ff0000

/* Description		REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_REO_CMD_EXECUTION_STATUS
			
			Consumer: DEBUG
			
			Producer: REO 
			
			
			
			Execution status of the command.
			
			
			
			<enum 0 reo_successful_execution> Command has
			successfully be executed
			
			<enum 1 reo_blocked_execution> Command could not be
			executed as the queue or cache was blocked
			
			<enum 2 reo_failed_execution> Command has encountered
			problems when executing, like the queue descriptor not being
			valid. None of the status fields in the entire STATUS TLV
			are valid.
			
			<enum 3 reo_resource_blocked> Command is NOT  executed
			because one or more descriptors were blocked. This is SW
			programming mistake.
			
			None of the status fields in the entire STATUS TLV are
			valid.
			
			
			
			<legal  0-3>
*/
#define REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_OFFSET 0x00000000
#define REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_LSB 26
#define REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_MASK 0x0c000000

/* Description		REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_RESERVED_0A
			
			<legal 0>
*/
#define REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_RESERVED_0A_OFFSET 0x00000000
#define REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_RESERVED_0A_LSB   28
#define REO_GET_QUEUE_STATS_STATUS_0_STATUS_HEADER_RESERVED_0A_MASK  0xf0000000

/* Description		REO_GET_QUEUE_STATS_STATUS_1_STATUS_HEADER_TIMESTAMP
			
			Timestamp at the moment that this status report is
			written.
			
			
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_1_STATUS_HEADER_TIMESTAMP_OFFSET  0x00000004
#define REO_GET_QUEUE_STATS_STATUS_1_STATUS_HEADER_TIMESTAMP_LSB     0
#define REO_GET_QUEUE_STATS_STATUS_1_STATUS_HEADER_TIMESTAMP_MASK    0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_2_SSN
			
			Starting Sequence number of the session, this changes
			whenever window moves. (can be filled by SW then maintained
			by REO)
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_2_SSN_OFFSET                      0x00000008
#define REO_GET_QUEUE_STATS_STATUS_2_SSN_LSB                         0
#define REO_GET_QUEUE_STATS_STATUS_2_SSN_MASK                        0x00000fff

/* Description		REO_GET_QUEUE_STATS_STATUS_2_CURRENT_INDEX
			
			Points to last forwarded packet
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_2_CURRENT_INDEX_OFFSET            0x00000008
#define REO_GET_QUEUE_STATS_STATUS_2_CURRENT_INDEX_LSB               12
#define REO_GET_QUEUE_STATS_STATUS_2_CURRENT_INDEX_MASK              0x000ff000

/* Description		REO_GET_QUEUE_STATS_STATUS_2_RESERVED_2
			
			<legal 0>
*/
#define REO_GET_QUEUE_STATS_STATUS_2_RESERVED_2_OFFSET               0x00000008
#define REO_GET_QUEUE_STATS_STATUS_2_RESERVED_2_LSB                  20
#define REO_GET_QUEUE_STATS_STATUS_2_RESERVED_2_MASK                 0xfff00000

/* Description		REO_GET_QUEUE_STATS_STATUS_3_PN_31_0
			
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_3_PN_31_0_OFFSET                  0x0000000c
#define REO_GET_QUEUE_STATS_STATUS_3_PN_31_0_LSB                     0
#define REO_GET_QUEUE_STATS_STATUS_3_PN_31_0_MASK                    0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_4_PN_63_32
			
			Bits [63:32] of the PN number.  
			
			<legal all> 
*/
#define REO_GET_QUEUE_STATS_STATUS_4_PN_63_32_OFFSET                 0x00000010
#define REO_GET_QUEUE_STATS_STATUS_4_PN_63_32_LSB                    0
#define REO_GET_QUEUE_STATS_STATUS_4_PN_63_32_MASK                   0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_5_PN_95_64
			
			Bits [95:64] of the PN number.  
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_5_PN_95_64_OFFSET                 0x00000014
#define REO_GET_QUEUE_STATS_STATUS_5_PN_95_64_LSB                    0
#define REO_GET_QUEUE_STATS_STATUS_5_PN_95_64_MASK                   0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_6_PN_127_96
			
			Bits [127:96] of the PN number.  
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_6_PN_127_96_OFFSET                0x00000018
#define REO_GET_QUEUE_STATS_STATUS_6_PN_127_96_LSB                   0
#define REO_GET_QUEUE_STATS_STATUS_6_PN_127_96_MASK                  0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_7_LAST_RX_ENQUEUE_TIMESTAMP
			
			Timestamp of arrival of the last MPDU for this queue
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_7_LAST_RX_ENQUEUE_TIMESTAMP_OFFSET 0x0000001c
#define REO_GET_QUEUE_STATS_STATUS_7_LAST_RX_ENQUEUE_TIMESTAMP_LSB   0
#define REO_GET_QUEUE_STATS_STATUS_7_LAST_RX_ENQUEUE_TIMESTAMP_MASK  0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_8_LAST_RX_DEQUEUE_TIMESTAMP
			
			Timestamp of forwarding an MPDU
			
			
			
			If the queue is empty when a frame gets received, this
			time shall be initialized to the 'enqueue' timestamp
			
			
			
			Used for aging
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_8_LAST_RX_DEQUEUE_TIMESTAMP_OFFSET 0x00000020
#define REO_GET_QUEUE_STATS_STATUS_8_LAST_RX_DEQUEUE_TIMESTAMP_LSB   0
#define REO_GET_QUEUE_STATS_STATUS_8_LAST_RX_DEQUEUE_TIMESTAMP_MASK  0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_9_RX_BITMAP_31_0
			
			When a bit is set, the corresponding frame is currently
			held in the re-order queue.
			
			The bitmap  is Fully managed by HW. 
			
			SW shall init this to 0, and then never ever change it
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_9_RX_BITMAP_31_0_OFFSET           0x00000024
#define REO_GET_QUEUE_STATS_STATUS_9_RX_BITMAP_31_0_LSB              0
#define REO_GET_QUEUE_STATS_STATUS_9_RX_BITMAP_31_0_MASK             0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_10_RX_BITMAP_63_32
			
			See Rx_bitmap_31_0 description
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_10_RX_BITMAP_63_32_OFFSET         0x00000028
#define REO_GET_QUEUE_STATS_STATUS_10_RX_BITMAP_63_32_LSB            0
#define REO_GET_QUEUE_STATS_STATUS_10_RX_BITMAP_63_32_MASK           0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_11_RX_BITMAP_95_64
			
			See Rx_bitmap_31_0 description
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_11_RX_BITMAP_95_64_OFFSET         0x0000002c
#define REO_GET_QUEUE_STATS_STATUS_11_RX_BITMAP_95_64_LSB            0
#define REO_GET_QUEUE_STATS_STATUS_11_RX_BITMAP_95_64_MASK           0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_12_RX_BITMAP_127_96
			
			See Rx_bitmap_31_0 description
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_12_RX_BITMAP_127_96_OFFSET        0x00000030
#define REO_GET_QUEUE_STATS_STATUS_12_RX_BITMAP_127_96_LSB           0
#define REO_GET_QUEUE_STATS_STATUS_12_RX_BITMAP_127_96_MASK          0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_13_RX_BITMAP_159_128
			
			See Rx_bitmap_31_0 description
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_13_RX_BITMAP_159_128_OFFSET       0x00000034
#define REO_GET_QUEUE_STATS_STATUS_13_RX_BITMAP_159_128_LSB          0
#define REO_GET_QUEUE_STATS_STATUS_13_RX_BITMAP_159_128_MASK         0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_14_RX_BITMAP_191_160
			
			See Rx_bitmap_31_0 description
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_14_RX_BITMAP_191_160_OFFSET       0x00000038
#define REO_GET_QUEUE_STATS_STATUS_14_RX_BITMAP_191_160_LSB          0
#define REO_GET_QUEUE_STATS_STATUS_14_RX_BITMAP_191_160_MASK         0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_15_RX_BITMAP_223_192
			
			See Rx_bitmap_31_0 description
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_15_RX_BITMAP_223_192_OFFSET       0x0000003c
#define REO_GET_QUEUE_STATS_STATUS_15_RX_BITMAP_223_192_LSB          0
#define REO_GET_QUEUE_STATS_STATUS_15_RX_BITMAP_223_192_MASK         0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_16_RX_BITMAP_255_224
			
			See Rx_bitmap_31_0 description
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_16_RX_BITMAP_255_224_OFFSET       0x00000040
#define REO_GET_QUEUE_STATS_STATUS_16_RX_BITMAP_255_224_LSB          0
#define REO_GET_QUEUE_STATS_STATUS_16_RX_BITMAP_255_224_MASK         0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_17_CURRENT_MPDU_COUNT
			
			The number of MPDUs in the queue.
			
			
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_17_CURRENT_MPDU_COUNT_OFFSET      0x00000044
#define REO_GET_QUEUE_STATS_STATUS_17_CURRENT_MPDU_COUNT_LSB         0
#define REO_GET_QUEUE_STATS_STATUS_17_CURRENT_MPDU_COUNT_MASK        0x0000007f

/* Description		REO_GET_QUEUE_STATS_STATUS_17_CURRENT_MSDU_COUNT
			
			The number of MSDUs in the queue.
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_17_CURRENT_MSDU_COUNT_OFFSET      0x00000044
#define REO_GET_QUEUE_STATS_STATUS_17_CURRENT_MSDU_COUNT_LSB         7
#define REO_GET_QUEUE_STATS_STATUS_17_CURRENT_MSDU_COUNT_MASK        0xffffff80

/* Description		REO_GET_QUEUE_STATS_STATUS_18_RESERVED_18
			
			<legal 0>
*/
#define REO_GET_QUEUE_STATS_STATUS_18_RESERVED_18_OFFSET             0x00000048
#define REO_GET_QUEUE_STATS_STATUS_18_RESERVED_18_LSB                0
#define REO_GET_QUEUE_STATS_STATUS_18_RESERVED_18_MASK               0x0000000f

/* Description		REO_GET_QUEUE_STATS_STATUS_18_TIMEOUT_COUNT
			
			The number of times that REO started forwarding frames
			even though there is a hole in the bitmap. Forwarding reason
			is Timeout
			
			
			
			The counter saturates and freezes at 0x3F
			
			
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_18_TIMEOUT_COUNT_OFFSET           0x00000048
#define REO_GET_QUEUE_STATS_STATUS_18_TIMEOUT_COUNT_LSB              4
#define REO_GET_QUEUE_STATS_STATUS_18_TIMEOUT_COUNT_MASK             0x000003f0

/* Description		REO_GET_QUEUE_STATS_STATUS_18_FORWARD_DUE_TO_BAR_COUNT
			
			The number of times that REO started forwarding frames
			even though there is a hole in the bitmap. Forwarding reason
			is reception of BAR frame.
			
			
			
			The counter saturates and freezes at 0x3F
			
			
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_18_FORWARD_DUE_TO_BAR_COUNT_OFFSET 0x00000048
#define REO_GET_QUEUE_STATS_STATUS_18_FORWARD_DUE_TO_BAR_COUNT_LSB   10
#define REO_GET_QUEUE_STATS_STATUS_18_FORWARD_DUE_TO_BAR_COUNT_MASK  0x0000fc00

/* Description		REO_GET_QUEUE_STATS_STATUS_18_DUPLICATE_COUNT
			
			The number of duplicate frames that have been detected
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_18_DUPLICATE_COUNT_OFFSET         0x00000048
#define REO_GET_QUEUE_STATS_STATUS_18_DUPLICATE_COUNT_LSB            16
#define REO_GET_QUEUE_STATS_STATUS_18_DUPLICATE_COUNT_MASK           0xffff0000

/* Description		REO_GET_QUEUE_STATS_STATUS_19_FRAMES_IN_ORDER_COUNT
			
			The number of frames that have been received in order
			(without a hole that prevented them from being forwarded
			immediately)
			
			
			
			This corresponds to the Reorder opcodes:
			
			'FWDCUR' and 'FWD BUF'
			
			
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_19_FRAMES_IN_ORDER_COUNT_OFFSET   0x0000004c
#define REO_GET_QUEUE_STATS_STATUS_19_FRAMES_IN_ORDER_COUNT_LSB      0
#define REO_GET_QUEUE_STATS_STATUS_19_FRAMES_IN_ORDER_COUNT_MASK     0x00ffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_19_BAR_RECEIVED_COUNT
			
			The number of times a BAR frame is received.
			
			
			
			This corresponds to the Reorder opcodes with 'DROP'
			
			
			
			The counter saturates and freezes at 0xFF
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_19_BAR_RECEIVED_COUNT_OFFSET      0x0000004c
#define REO_GET_QUEUE_STATS_STATUS_19_BAR_RECEIVED_COUNT_LSB         24
#define REO_GET_QUEUE_STATS_STATUS_19_BAR_RECEIVED_COUNT_MASK        0xff000000

/* Description		REO_GET_QUEUE_STATS_STATUS_20_MPDU_FRAMES_PROCESSED_COUNT
			
			The total number of MPDU frames that have been processed
			by REO. This includes the duplicates.
			
			
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_20_MPDU_FRAMES_PROCESSED_COUNT_OFFSET 0x00000050
#define REO_GET_QUEUE_STATS_STATUS_20_MPDU_FRAMES_PROCESSED_COUNT_LSB 0
#define REO_GET_QUEUE_STATS_STATUS_20_MPDU_FRAMES_PROCESSED_COUNT_MASK 0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_21_MSDU_FRAMES_PROCESSED_COUNT
			
			The total number of MSDU frames that have been processed
			by REO. This includes the duplicates.
			
			
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_21_MSDU_FRAMES_PROCESSED_COUNT_OFFSET 0x00000054
#define REO_GET_QUEUE_STATS_STATUS_21_MSDU_FRAMES_PROCESSED_COUNT_LSB 0
#define REO_GET_QUEUE_STATS_STATUS_21_MSDU_FRAMES_PROCESSED_COUNT_MASK 0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_22_TOTAL_PROCESSED_BYTE_COUNT
			
			An approximation of the number of bytes received for
			this queue. 
			
			
			
			In 64 byte units
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_22_TOTAL_PROCESSED_BYTE_COUNT_OFFSET 0x00000058
#define REO_GET_QUEUE_STATS_STATUS_22_TOTAL_PROCESSED_BYTE_COUNT_LSB 0
#define REO_GET_QUEUE_STATS_STATUS_22_TOTAL_PROCESSED_BYTE_COUNT_MASK 0xffffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_23_LATE_RECEIVE_MPDU_COUNT
			
			The number of MPDUs received after the window had
			already moved on. The 'late' sequence window is defined as
			(Window SSN - 256) - (Window SSN - 1)
			
			
			
			This corresponds with Out of order detection in
			duplicate detect FSM
			
			
			
			The counter saturates and freezes at 0xFFF
			
			
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_23_LATE_RECEIVE_MPDU_COUNT_OFFSET 0x0000005c
#define REO_GET_QUEUE_STATS_STATUS_23_LATE_RECEIVE_MPDU_COUNT_LSB    0
#define REO_GET_QUEUE_STATS_STATUS_23_LATE_RECEIVE_MPDU_COUNT_MASK   0x00000fff

/* Description		REO_GET_QUEUE_STATS_STATUS_23_WINDOW_JUMP_2K
			
			The number of times the window moved more then 2K
			
			
			
			The counter saturates and freezes at 0xF
			
			
			
			(Note: field name can not start with number: previous
			2k_window_jump)
			
			
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_23_WINDOW_JUMP_2K_OFFSET          0x0000005c
#define REO_GET_QUEUE_STATS_STATUS_23_WINDOW_JUMP_2K_LSB             12
#define REO_GET_QUEUE_STATS_STATUS_23_WINDOW_JUMP_2K_MASK            0x0000f000

/* Description		REO_GET_QUEUE_STATS_STATUS_23_HOLE_COUNT
			
			The number of times a hole was created in the receive
			bitmap.
			
			
			
			This corresponds to the Reorder opcodes with 'QCUR'
			
			
			
			<legal all>
*/
#define REO_GET_QUEUE_STATS_STATUS_23_HOLE_COUNT_OFFSET              0x0000005c
#define REO_GET_QUEUE_STATS_STATUS_23_HOLE_COUNT_LSB                 16
#define REO_GET_QUEUE_STATS_STATUS_23_HOLE_COUNT_MASK                0xffff0000

/* Description		REO_GET_QUEUE_STATS_STATUS_24_RESERVED_24A
			
			<legal 0>
*/
#define REO_GET_QUEUE_STATS_STATUS_24_RESERVED_24A_OFFSET            0x00000060
#define REO_GET_QUEUE_STATS_STATUS_24_RESERVED_24A_LSB               0
#define REO_GET_QUEUE_STATS_STATUS_24_RESERVED_24A_MASK              0x0fffffff

/* Description		REO_GET_QUEUE_STATS_STATUS_24_LOOPING_COUNT
			
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
#define REO_GET_QUEUE_STATS_STATUS_24_LOOPING_COUNT_OFFSET           0x00000060
#define REO_GET_QUEUE_STATS_STATUS_24_LOOPING_COUNT_LSB              28
#define REO_GET_QUEUE_STATS_STATUS_24_LOOPING_COUNT_MASK             0xf0000000


#endif // _REO_GET_QUEUE_STATS_STATUS_H_
