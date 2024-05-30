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

#ifndef _REO_GET_QUEUE_STATS_STATUS_H_
#define _REO_GET_QUEUE_STATS_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_status_header.h"
#define NUM_OF_DWORDS_REO_GET_QUEUE_STATS_STATUS 26

#define NUM_OF_QWORDS_REO_GET_QUEUE_STATS_STATUS 13


struct reo_get_queue_stats_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_reo_status_header                                 status_header;
             uint32_t ssn                                                     : 12, // [11:0]
                      current_index                                           : 10, // [21:12]
                      reserved_2                                              : 10; // [31:22]
             uint32_t pn_31_0                                                 : 32; // [31:0]
             uint32_t pn_63_32                                                : 32; // [31:0]
             uint32_t pn_95_64                                                : 32; // [31:0]
             uint32_t pn_127_96                                               : 32; // [31:0]
             uint32_t last_rx_enqueue_timestamp                               : 32; // [31:0]
             uint32_t last_rx_dequeue_timestamp                               : 32; // [31:0]
             uint32_t rx_bitmap_31_0                                          : 32; // [31:0]
             uint32_t rx_bitmap_63_32                                         : 32; // [31:0]
             uint32_t rx_bitmap_95_64                                         : 32; // [31:0]
             uint32_t rx_bitmap_127_96                                        : 32; // [31:0]
             uint32_t rx_bitmap_159_128                                       : 32; // [31:0]
             uint32_t rx_bitmap_191_160                                       : 32; // [31:0]
             uint32_t rx_bitmap_223_192                                       : 32; // [31:0]
             uint32_t rx_bitmap_255_224                                       : 32; // [31:0]
             uint32_t rx_bitmap_287_256                                       : 32; // [31:0]
             uint32_t current_mpdu_count                                      :  7, // [6:0]
                      current_msdu_count                                      : 25; // [31:7]
             uint32_t window_jump_2k                                          :  4, // [3:0]
                      timeout_count                                           :  6, // [9:4]
                      forward_due_to_bar_count                                :  6, // [15:10]
                      duplicate_count                                         : 16; // [31:16]
             uint32_t frames_in_order_count                                   : 24, // [23:0]
                      bar_received_count                                      :  8; // [31:24]
             uint32_t mpdu_frames_processed_count                             : 32; // [31:0]
             uint32_t msdu_frames_processed_count                             : 32; // [31:0]
             uint32_t total_processed_byte_count                              : 32; // [31:0]
             uint32_t late_receive_mpdu_count                                 : 12, // [11:0]
                      hole_count                                              : 16, // [27:12]
                      get_queue_1k_stats_status_to_follow                     :  1, // [28:28]
                      reserved_24a                                            :  3; // [31:29]
             uint32_t aging_drop_mpdu_count                                   : 16, // [15:0]
                      aging_drop_interval                                     :  8, // [23:16]
                      reserved_25a                                            :  4, // [27:24]
                      looping_count                                           :  4; // [31:28]
#else
             struct   uniform_reo_status_header                                 status_header;
             uint32_t reserved_2                                              : 10, // [31:22]
                      current_index                                           : 10, // [21:12]
                      ssn                                                     : 12; // [11:0]
             uint32_t pn_31_0                                                 : 32; // [31:0]
             uint32_t pn_63_32                                                : 32; // [31:0]
             uint32_t pn_95_64                                                : 32; // [31:0]
             uint32_t pn_127_96                                               : 32; // [31:0]
             uint32_t last_rx_enqueue_timestamp                               : 32; // [31:0]
             uint32_t last_rx_dequeue_timestamp                               : 32; // [31:0]
             uint32_t rx_bitmap_31_0                                          : 32; // [31:0]
             uint32_t rx_bitmap_63_32                                         : 32; // [31:0]
             uint32_t rx_bitmap_95_64                                         : 32; // [31:0]
             uint32_t rx_bitmap_127_96                                        : 32; // [31:0]
             uint32_t rx_bitmap_159_128                                       : 32; // [31:0]
             uint32_t rx_bitmap_191_160                                       : 32; // [31:0]
             uint32_t rx_bitmap_223_192                                       : 32; // [31:0]
             uint32_t rx_bitmap_255_224                                       : 32; // [31:0]
             uint32_t rx_bitmap_287_256                                       : 32; // [31:0]
             uint32_t current_msdu_count                                      : 25, // [31:7]
                      current_mpdu_count                                      :  7; // [6:0]
             uint32_t duplicate_count                                         : 16, // [31:16]
                      forward_due_to_bar_count                                :  6, // [15:10]
                      timeout_count                                           :  6, // [9:4]
                      window_jump_2k                                          :  4; // [3:0]
             uint32_t bar_received_count                                      :  8, // [31:24]
                      frames_in_order_count                                   : 24; // [23:0]
             uint32_t mpdu_frames_processed_count                             : 32; // [31:0]
             uint32_t msdu_frames_processed_count                             : 32; // [31:0]
             uint32_t total_processed_byte_count                              : 32; // [31:0]
             uint32_t reserved_24a                                            :  3, // [31:29]
                      get_queue_1k_stats_status_to_follow                     :  1, // [28:28]
                      hole_count                                              : 16, // [27:12]
                      late_receive_mpdu_count                                 : 12; // [11:0]
             uint32_t looping_count                                           :  4, // [31:28]
                      reserved_25a                                            :  4, // [27:24]
                      aging_drop_interval                                     :  8, // [23:16]
                      aging_drop_mpdu_count                                   : 16; // [15:0]
#endif
};


/* Description		STATUS_HEADER

			Consumer: SW
			Producer: REO
			
			Details that can link this status with the original command. 
			It also contains info on how long REO took to execute this
			 command.
*/


/* Description		REO_STATUS_NUMBER

			Consumer: SW , DEBUG
			Producer: REO
			
			The value in this field is equal to value of the 'REO_CMD_Number' 
			field the REO command 
			
			This field helps to correlate the statuses with the REO 
			commands.
			
			<legal all> 
*/

#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_OFFSET           0x0000000000000000
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_LSB              0
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_MSB              15
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_MASK             0x000000000000ffff


/* Description		CMD_EXECUTION_TIME

			Consumer: DEBUG
			Producer: REO 
			
			The amount of time REO took to excecute the command. Note
			 that this time does not include the duration of the command
			 waiting in the command ring, before the execution started.
			
			
			In us.
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_OFFSET          0x0000000000000000
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_LSB             16
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_MSB             25
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_MASK            0x0000000003ff0000


/* Description		REO_CMD_EXECUTION_STATUS

			Consumer: DEBUG
			Producer: REO 
			
			Execution status of the command.
			
			<enum 0 reo_successful_execution> Command has successfully
			 be executed
			<enum 1 reo_blocked_execution> Command could not be executed
			 as the queue or cache was blocked
			<enum 2 reo_failed_execution> Command has encountered problems
			 when executing, like the queue descriptor not being valid. 
			None of the status fields in the entire STATUS TLV are valid.
			
			<enum 3 reo_resource_blocked> Command is NOT  executed because
			 one or more descriptors were blocked. This is SW programming
			 mistake.
			None of the status fields in the entire STATUS TLV are valid.
			
			
			<legal  0-3>
*/

#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_OFFSET    0x0000000000000000
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_LSB       26
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_MSB       27
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_MASK      0x000000000c000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_RESERVED_0A_OFFSET                 0x0000000000000000
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_RESERVED_0A_LSB                    28
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_RESERVED_0A_MSB                    31
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_RESERVED_0A_MASK                   0x00000000f0000000


/* Description		TIMESTAMP

			Timestamp at the moment that this status report is written.
			
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_TIMESTAMP_OFFSET                   0x0000000000000000
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_TIMESTAMP_LSB                      32
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_TIMESTAMP_MSB                      63
#define REO_GET_QUEUE_STATS_STATUS_STATUS_HEADER_TIMESTAMP_MASK                     0xffffffff00000000


/* Description		SSN

			Starting Sequence number of the session, this changes whenever
			 window moves. (can be filled by SW then maintained by REO)
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_SSN_OFFSET                                       0x0000000000000008
#define REO_GET_QUEUE_STATS_STATUS_SSN_LSB                                          0
#define REO_GET_QUEUE_STATS_STATUS_SSN_MSB                                          11
#define REO_GET_QUEUE_STATS_STATUS_SSN_MASK                                         0x0000000000000fff


/* Description		CURRENT_INDEX

			Points to last forwarded packet
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_CURRENT_INDEX_OFFSET                             0x0000000000000008
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_INDEX_LSB                                12
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_INDEX_MSB                                21
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_INDEX_MASK                               0x00000000003ff000


/* Description		RESERVED_2

			<legal 0>
*/

#define REO_GET_QUEUE_STATS_STATUS_RESERVED_2_OFFSET                                0x0000000000000008
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_2_LSB                                   22
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_2_MSB                                   31
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_2_MASK                                  0x00000000ffc00000


/* Description		PN_31_0

			Bits [31:0] of the PN number extracted from the IV field
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_PN_31_0_OFFSET                                   0x0000000000000008
#define REO_GET_QUEUE_STATS_STATUS_PN_31_0_LSB                                      32
#define REO_GET_QUEUE_STATS_STATUS_PN_31_0_MSB                                      63
#define REO_GET_QUEUE_STATS_STATUS_PN_31_0_MASK                                     0xffffffff00000000


/* Description		PN_63_32

			Bits [63:32] of the PN number.  
			<legal all> 
*/

#define REO_GET_QUEUE_STATS_STATUS_PN_63_32_OFFSET                                  0x0000000000000010
#define REO_GET_QUEUE_STATS_STATUS_PN_63_32_LSB                                     0
#define REO_GET_QUEUE_STATS_STATUS_PN_63_32_MSB                                     31
#define REO_GET_QUEUE_STATS_STATUS_PN_63_32_MASK                                    0x00000000ffffffff


/* Description		PN_95_64

			Bits [95:64] of the PN number.  
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_PN_95_64_OFFSET                                  0x0000000000000010
#define REO_GET_QUEUE_STATS_STATUS_PN_95_64_LSB                                     32
#define REO_GET_QUEUE_STATS_STATUS_PN_95_64_MSB                                     63
#define REO_GET_QUEUE_STATS_STATUS_PN_95_64_MASK                                    0xffffffff00000000


/* Description		PN_127_96

			Bits [127:96] of the PN number.  
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_PN_127_96_OFFSET                                 0x0000000000000018
#define REO_GET_QUEUE_STATS_STATUS_PN_127_96_LSB                                    0
#define REO_GET_QUEUE_STATS_STATUS_PN_127_96_MSB                                    31
#define REO_GET_QUEUE_STATS_STATUS_PN_127_96_MASK                                   0x00000000ffffffff


/* Description		LAST_RX_ENQUEUE_TIMESTAMP

			Timestamp of arrival of the last MPDU for this queue
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_ENQUEUE_TIMESTAMP_OFFSET                 0x0000000000000018
#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_ENQUEUE_TIMESTAMP_LSB                    32
#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_ENQUEUE_TIMESTAMP_MSB                    63
#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_ENQUEUE_TIMESTAMP_MASK                   0xffffffff00000000


/* Description		LAST_RX_DEQUEUE_TIMESTAMP

			Timestamp of forwarding an MPDU
			
			If the queue is empty when a frame gets received, this time
			 shall be initialized to the 'enqueue' timestamp
			
			Used for aging
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_DEQUEUE_TIMESTAMP_OFFSET                 0x0000000000000020
#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_DEQUEUE_TIMESTAMP_LSB                    0
#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_DEQUEUE_TIMESTAMP_MSB                    31
#define REO_GET_QUEUE_STATS_STATUS_LAST_RX_DEQUEUE_TIMESTAMP_MASK                   0x00000000ffffffff


/* Description		RX_BITMAP_31_0

			When a bit is set, the corresponding frame is currently 
			held in the re-order queue.
			The bitmap  is Fully managed by HW. 
			SW shall init this to 0, and then never ever change it
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_31_0_OFFSET                            0x0000000000000020
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_31_0_LSB                               32
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_31_0_MSB                               63
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_31_0_MASK                              0xffffffff00000000


/* Description		RX_BITMAP_63_32

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_63_32_OFFSET                           0x0000000000000028
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_63_32_LSB                              0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_63_32_MSB                              31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_63_32_MASK                             0x00000000ffffffff


/* Description		RX_BITMAP_95_64

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_95_64_OFFSET                           0x0000000000000028
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_95_64_LSB                              32
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_95_64_MSB                              63
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_95_64_MASK                             0xffffffff00000000


/* Description		RX_BITMAP_127_96

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_127_96_OFFSET                          0x0000000000000030
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_127_96_LSB                             0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_127_96_MSB                             31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_127_96_MASK                            0x00000000ffffffff


/* Description		RX_BITMAP_159_128

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_159_128_OFFSET                         0x0000000000000030
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_159_128_LSB                            32
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_159_128_MSB                            63
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_159_128_MASK                           0xffffffff00000000


/* Description		RX_BITMAP_191_160

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_191_160_OFFSET                         0x0000000000000038
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_191_160_LSB                            0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_191_160_MSB                            31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_191_160_MASK                           0x00000000ffffffff


/* Description		RX_BITMAP_223_192

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_223_192_OFFSET                         0x0000000000000038
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_223_192_LSB                            32
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_223_192_MSB                            63
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_223_192_MASK                           0xffffffff00000000


/* Description		RX_BITMAP_255_224

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_255_224_OFFSET                         0x0000000000000040
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_255_224_LSB                            0
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_255_224_MSB                            31
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_255_224_MASK                           0x00000000ffffffff


/* Description		RX_BITMAP_287_256

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_287_256_OFFSET                         0x0000000000000040
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_287_256_LSB                            32
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_287_256_MSB                            63
#define REO_GET_QUEUE_STATS_STATUS_RX_BITMAP_287_256_MASK                           0xffffffff00000000


/* Description		CURRENT_MPDU_COUNT

			The number of MPDUs in the queue.
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MPDU_COUNT_OFFSET                        0x0000000000000048
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MPDU_COUNT_LSB                           0
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MPDU_COUNT_MSB                           6
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MPDU_COUNT_MASK                          0x000000000000007f


/* Description		CURRENT_MSDU_COUNT

			The number of MSDUs in the queue.
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MSDU_COUNT_OFFSET                        0x0000000000000048
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MSDU_COUNT_LSB                           7
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MSDU_COUNT_MSB                           31
#define REO_GET_QUEUE_STATS_STATUS_CURRENT_MSDU_COUNT_MASK                          0x00000000ffffff80


/* Description		WINDOW_JUMP_2K

			The number of times the window moved more then 2K
			
			The counter saturates and freezes at 0xF
			
			(Note: field name can not start with number: previous 2k_window_jump)
			
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_WINDOW_JUMP_2K_OFFSET                            0x0000000000000048
#define REO_GET_QUEUE_STATS_STATUS_WINDOW_JUMP_2K_LSB                               32
#define REO_GET_QUEUE_STATS_STATUS_WINDOW_JUMP_2K_MSB                               35
#define REO_GET_QUEUE_STATS_STATUS_WINDOW_JUMP_2K_MASK                              0x0000000f00000000


/* Description		TIMEOUT_COUNT

			The number of times that REO started forwarding frames even
			 though there is a hole in the bitmap. Forwarding reason
			 is Timeout
			
			The counter saturates and freezes at 0x3F
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_TIMEOUT_COUNT_OFFSET                             0x0000000000000048
#define REO_GET_QUEUE_STATS_STATUS_TIMEOUT_COUNT_LSB                                36
#define REO_GET_QUEUE_STATS_STATUS_TIMEOUT_COUNT_MSB                                41
#define REO_GET_QUEUE_STATS_STATUS_TIMEOUT_COUNT_MASK                               0x000003f000000000


/* Description		FORWARD_DUE_TO_BAR_COUNT

			The number of times that REO started forwarding frames even
			 though there is a hole in the bitmap. Forwarding reason
			 is reception of BAR frame.
			
			The counter saturates and freezes at 0x3F
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_FORWARD_DUE_TO_BAR_COUNT_OFFSET                  0x0000000000000048
#define REO_GET_QUEUE_STATS_STATUS_FORWARD_DUE_TO_BAR_COUNT_LSB                     42
#define REO_GET_QUEUE_STATS_STATUS_FORWARD_DUE_TO_BAR_COUNT_MSB                     47
#define REO_GET_QUEUE_STATS_STATUS_FORWARD_DUE_TO_BAR_COUNT_MASK                    0x0000fc0000000000


/* Description		DUPLICATE_COUNT

			The number of duplicate frames that have been detected
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_DUPLICATE_COUNT_OFFSET                           0x0000000000000048
#define REO_GET_QUEUE_STATS_STATUS_DUPLICATE_COUNT_LSB                              48
#define REO_GET_QUEUE_STATS_STATUS_DUPLICATE_COUNT_MSB                              63
#define REO_GET_QUEUE_STATS_STATUS_DUPLICATE_COUNT_MASK                             0xffff000000000000


/* Description		FRAMES_IN_ORDER_COUNT

			The number of frames that have been received in order (without
			 a hole that prevented them from being forwarded immediately)
			
			
			This corresponds to the Reorder opcodes:
			'FWDCUR' and 'FWD BUF'
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_FRAMES_IN_ORDER_COUNT_OFFSET                     0x0000000000000050
#define REO_GET_QUEUE_STATS_STATUS_FRAMES_IN_ORDER_COUNT_LSB                        0
#define REO_GET_QUEUE_STATS_STATUS_FRAMES_IN_ORDER_COUNT_MSB                        23
#define REO_GET_QUEUE_STATS_STATUS_FRAMES_IN_ORDER_COUNT_MASK                       0x0000000000ffffff


/* Description		BAR_RECEIVED_COUNT

			The number of times a BAR frame is received.
			
			This corresponds to the Reorder opcodes with 'DROP'
			
			The counter saturates and freezes at 0xFF
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_BAR_RECEIVED_COUNT_OFFSET                        0x0000000000000050
#define REO_GET_QUEUE_STATS_STATUS_BAR_RECEIVED_COUNT_LSB                           24
#define REO_GET_QUEUE_STATS_STATUS_BAR_RECEIVED_COUNT_MSB                           31
#define REO_GET_QUEUE_STATS_STATUS_BAR_RECEIVED_COUNT_MASK                          0x00000000ff000000


/* Description		MPDU_FRAMES_PROCESSED_COUNT

			The total number of MPDU frames that have been processed
			 by REO. This includes the duplicates.
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_MPDU_FRAMES_PROCESSED_COUNT_OFFSET               0x0000000000000050
#define REO_GET_QUEUE_STATS_STATUS_MPDU_FRAMES_PROCESSED_COUNT_LSB                  32
#define REO_GET_QUEUE_STATS_STATUS_MPDU_FRAMES_PROCESSED_COUNT_MSB                  63
#define REO_GET_QUEUE_STATS_STATUS_MPDU_FRAMES_PROCESSED_COUNT_MASK                 0xffffffff00000000


/* Description		MSDU_FRAMES_PROCESSED_COUNT

			The total number of MSDU frames that have been processed
			 by REO. This includes the duplicates.
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_MSDU_FRAMES_PROCESSED_COUNT_OFFSET               0x0000000000000058
#define REO_GET_QUEUE_STATS_STATUS_MSDU_FRAMES_PROCESSED_COUNT_LSB                  0
#define REO_GET_QUEUE_STATS_STATUS_MSDU_FRAMES_PROCESSED_COUNT_MSB                  31
#define REO_GET_QUEUE_STATS_STATUS_MSDU_FRAMES_PROCESSED_COUNT_MASK                 0x00000000ffffffff


/* Description		TOTAL_PROCESSED_BYTE_COUNT

			An approximation of the number of bytes received for this
			 queue. 
			
			In 64 byte units
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_TOTAL_PROCESSED_BYTE_COUNT_OFFSET                0x0000000000000058
#define REO_GET_QUEUE_STATS_STATUS_TOTAL_PROCESSED_BYTE_COUNT_LSB                   32
#define REO_GET_QUEUE_STATS_STATUS_TOTAL_PROCESSED_BYTE_COUNT_MSB                   63
#define REO_GET_QUEUE_STATS_STATUS_TOTAL_PROCESSED_BYTE_COUNT_MASK                  0xffffffff00000000


/* Description		LATE_RECEIVE_MPDU_COUNT

			The number of MPDUs received after the window had already
			 moved on. The 'late' sequence window is defined as (Window
			 SSN - 256) - (Window SSN - 1)
			
			This corresponds with Out of order detection in duplicate
			 detect FSM
			
			The counter saturates and freezes at 0xFFF
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_LATE_RECEIVE_MPDU_COUNT_OFFSET                   0x0000000000000060
#define REO_GET_QUEUE_STATS_STATUS_LATE_RECEIVE_MPDU_COUNT_LSB                      0
#define REO_GET_QUEUE_STATS_STATUS_LATE_RECEIVE_MPDU_COUNT_MSB                      11
#define REO_GET_QUEUE_STATS_STATUS_LATE_RECEIVE_MPDU_COUNT_MASK                     0x0000000000000fff


/* Description		HOLE_COUNT

			The number of times a hole was created in the receive bitmap.
			
			
			This corresponds to the Reorder opcodes with 'QCUR'
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_HOLE_COUNT_OFFSET                                0x0000000000000060
#define REO_GET_QUEUE_STATS_STATUS_HOLE_COUNT_LSB                                   12
#define REO_GET_QUEUE_STATS_STATUS_HOLE_COUNT_MSB                                   27
#define REO_GET_QUEUE_STATS_STATUS_HOLE_COUNT_MASK                                  0x000000000ffff000


/* Description		GET_QUEUE_1K_STATS_STATUS_TO_FOLLOW

			Indicates that the queue supports a BA window size above
			 256, so a 'REO_GET_QUEUE_STATS_1K_STATUS' status TLV will
			 immediately follow.
			
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_GET_QUEUE_1K_STATS_STATUS_TO_FOLLOW_OFFSET       0x0000000000000060
#define REO_GET_QUEUE_STATS_STATUS_GET_QUEUE_1K_STATS_STATUS_TO_FOLLOW_LSB          28
#define REO_GET_QUEUE_STATS_STATUS_GET_QUEUE_1K_STATS_STATUS_TO_FOLLOW_MSB          28
#define REO_GET_QUEUE_STATS_STATUS_GET_QUEUE_1K_STATS_STATUS_TO_FOLLOW_MASK         0x0000000010000000


/* Description		RESERVED_24A

			<legal 0>
*/

#define REO_GET_QUEUE_STATS_STATUS_RESERVED_24A_OFFSET                              0x0000000000000060
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_24A_LSB                                 29
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_24A_MSB                                 31
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_24A_MASK                                0x00000000e0000000


/* Description		AGING_DROP_MPDU_COUNT

			The number of holes in the bitmap that moved due to aging
			 counter expiry
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_MPDU_COUNT_OFFSET                     0x0000000000000060
#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_MPDU_COUNT_LSB                        32
#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_MPDU_COUNT_MSB                        47
#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_MPDU_COUNT_MASK                       0x0000ffff00000000


/* Description		AGING_DROP_INTERVAL

			The number of times holes got removed from the bitmap due
			 to aging counter expiry
			<legal all>
*/

#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_INTERVAL_OFFSET                       0x0000000000000060
#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_INTERVAL_LSB                          48
#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_INTERVAL_MSB                          55
#define REO_GET_QUEUE_STATS_STATUS_AGING_DROP_INTERVAL_MASK                         0x00ff000000000000


/* Description		RESERVED_25A

			<legal 0>
*/

#define REO_GET_QUEUE_STATS_STATUS_RESERVED_25A_OFFSET                              0x0000000000000060
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_25A_LSB                                 56
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_25A_MSB                                 59
#define REO_GET_QUEUE_STATS_STATUS_RESERVED_25A_MASK                                0x0f00000000000000


/* Description		LOOPING_COUNT

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

#define REO_GET_QUEUE_STATS_STATUS_LOOPING_COUNT_OFFSET                             0x0000000000000060
#define REO_GET_QUEUE_STATS_STATUS_LOOPING_COUNT_LSB                                60
#define REO_GET_QUEUE_STATS_STATUS_LOOPING_COUNT_MSB                                63
#define REO_GET_QUEUE_STATS_STATUS_LOOPING_COUNT_MASK                               0xf000000000000000



#endif   // REO_GET_QUEUE_STATS_STATUS
