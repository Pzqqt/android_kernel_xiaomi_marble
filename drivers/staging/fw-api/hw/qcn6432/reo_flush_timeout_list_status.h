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

#ifndef _REO_FLUSH_TIMEOUT_LIST_STATUS_H_
#define _REO_FLUSH_TIMEOUT_LIST_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_status_header.h"
#define NUM_OF_DWORDS_REO_FLUSH_TIMEOUT_LIST_STATUS 26

#define NUM_OF_QWORDS_REO_FLUSH_TIMEOUT_LIST_STATUS 13


struct reo_flush_timeout_list_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_reo_status_header                                 status_header;
             uint32_t error_detected                                          :  1, // [0:0]
                      timout_list_empty                                       :  1, // [1:1]
                      reserved_2a                                             : 30; // [31:2]
             uint32_t release_desc_count                                      : 16, // [15:0]
                      forward_buf_count                                       : 16; // [31:16]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 32; // [31:0]
             uint32_t reserved_8a                                             : 32; // [31:0]
             uint32_t reserved_9a                                             : 32; // [31:0]
             uint32_t reserved_10a                                            : 32; // [31:0]
             uint32_t reserved_11a                                            : 32; // [31:0]
             uint32_t reserved_12a                                            : 32; // [31:0]
             uint32_t reserved_13a                                            : 32; // [31:0]
             uint32_t reserved_14a                                            : 32; // [31:0]
             uint32_t reserved_15a                                            : 32; // [31:0]
             uint32_t reserved_16a                                            : 32; // [31:0]
             uint32_t reserved_17a                                            : 32; // [31:0]
             uint32_t reserved_18a                                            : 32; // [31:0]
             uint32_t reserved_19a                                            : 32; // [31:0]
             uint32_t reserved_20a                                            : 32; // [31:0]
             uint32_t reserved_21a                                            : 32; // [31:0]
             uint32_t reserved_22a                                            : 32; // [31:0]
             uint32_t reserved_23a                                            : 32; // [31:0]
             uint32_t reserved_24a                                            : 32; // [31:0]
             uint32_t reserved_25a                                            : 28, // [27:0]
                      looping_count                                           :  4; // [31:28]
#else
             struct   uniform_reo_status_header                                 status_header;
             uint32_t reserved_2a                                             : 30, // [31:2]
                      timout_list_empty                                       :  1, // [1:1]
                      error_detected                                          :  1; // [0:0]
             uint32_t forward_buf_count                                       : 16, // [31:16]
                      release_desc_count                                      : 16; // [15:0]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 32; // [31:0]
             uint32_t reserved_8a                                             : 32; // [31:0]
             uint32_t reserved_9a                                             : 32; // [31:0]
             uint32_t reserved_10a                                            : 32; // [31:0]
             uint32_t reserved_11a                                            : 32; // [31:0]
             uint32_t reserved_12a                                            : 32; // [31:0]
             uint32_t reserved_13a                                            : 32; // [31:0]
             uint32_t reserved_14a                                            : 32; // [31:0]
             uint32_t reserved_15a                                            : 32; // [31:0]
             uint32_t reserved_16a                                            : 32; // [31:0]
             uint32_t reserved_17a                                            : 32; // [31:0]
             uint32_t reserved_18a                                            : 32; // [31:0]
             uint32_t reserved_19a                                            : 32; // [31:0]
             uint32_t reserved_20a                                            : 32; // [31:0]
             uint32_t reserved_21a                                            : 32; // [31:0]
             uint32_t reserved_22a                                            : 32; // [31:0]
             uint32_t reserved_23a                                            : 32; // [31:0]
             uint32_t reserved_24a                                            : 32; // [31:0]
             uint32_t looping_count                                           :  4, // [31:28]
                      reserved_25a                                            : 28; // [27:0]
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

#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_OFFSET        0x0000000000000000
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_LSB           0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_MSB           15
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_MASK          0x000000000000ffff


/* Description		CMD_EXECUTION_TIME

			Consumer: DEBUG
			Producer: REO 
			
			The amount of time REO took to excecute the command. Note
			 that this time does not include the duration of the command
			 waiting in the command ring, before the execution started.
			
			
			In us.
			
			<legal all>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_OFFSET       0x0000000000000000
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_LSB          16
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_MSB          25
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_MASK         0x0000000003ff0000


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

#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_OFFSET 0x0000000000000000
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_LSB    26
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_MSB    27
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_MASK   0x000000000c000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_RESERVED_0A_OFFSET              0x0000000000000000
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_RESERVED_0A_LSB                 28
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_RESERVED_0A_MSB                 31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_RESERVED_0A_MASK                0x00000000f0000000


/* Description		TIMESTAMP

			Timestamp at the moment that this status report is written.
			
			
			<legal all>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_TIMESTAMP_OFFSET                0x0000000000000000
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_TIMESTAMP_LSB                   32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_TIMESTAMP_MSB                   63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_STATUS_HEADER_TIMESTAMP_MASK                  0xffffffff00000000


/* Description		ERROR_DETECTED

			0: No error has been detected while executing this command
			
			1: command not properly executed and returned with an error
			
			
			NOTE: Current no error is defined, but field is put in place
			 to avoid data structure changes in future...
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_ERROR_DETECTED_OFFSET                         0x0000000000000008
#define REO_FLUSH_TIMEOUT_LIST_STATUS_ERROR_DETECTED_LSB                            0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_ERROR_DETECTED_MSB                            0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_ERROR_DETECTED_MASK                           0x0000000000000001


/* Description		TIMOUT_LIST_EMPTY

			When set, REO has depleted the timeout list and all entries
			 are gone.
			<legal all>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_TIMOUT_LIST_EMPTY_OFFSET                      0x0000000000000008
#define REO_FLUSH_TIMEOUT_LIST_STATUS_TIMOUT_LIST_EMPTY_LSB                         1
#define REO_FLUSH_TIMEOUT_LIST_STATUS_TIMOUT_LIST_EMPTY_MSB                         1
#define REO_FLUSH_TIMEOUT_LIST_STATUS_TIMOUT_LIST_EMPTY_MASK                        0x0000000000000002


/* Description		RESERVED_2A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_2A_OFFSET                            0x0000000000000008
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_2A_LSB                               2
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_2A_MSB                               31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_2A_MASK                              0x00000000fffffffc


/* Description		RELEASE_DESC_COUNT

			Consumer: REO
			Producer: SW
			
			The number of link descriptors released
			<legal all>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RELEASE_DESC_COUNT_OFFSET                     0x0000000000000008
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RELEASE_DESC_COUNT_LSB                        32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RELEASE_DESC_COUNT_MSB                        47
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RELEASE_DESC_COUNT_MASK                       0x0000ffff00000000


/* Description		FORWARD_BUF_COUNT

			Consumer: REO
			Producer: SW
			
			The number of buffers forwarded to the REO destination rings
			
			<legal all>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_FORWARD_BUF_COUNT_OFFSET                      0x0000000000000008
#define REO_FLUSH_TIMEOUT_LIST_STATUS_FORWARD_BUF_COUNT_LSB                         48
#define REO_FLUSH_TIMEOUT_LIST_STATUS_FORWARD_BUF_COUNT_MSB                         63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_FORWARD_BUF_COUNT_MASK                        0xffff000000000000


/* Description		RESERVED_4A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_4A_OFFSET                            0x0000000000000010
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_4A_LSB                               0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_4A_MSB                               31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_4A_MASK                              0x00000000ffffffff


/* Description		RESERVED_5A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_5A_OFFSET                            0x0000000000000010
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_5A_LSB                               32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_5A_MSB                               63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_5A_MASK                              0xffffffff00000000


/* Description		RESERVED_6A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_6A_OFFSET                            0x0000000000000018
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_6A_LSB                               0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_6A_MSB                               31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_6A_MASK                              0x00000000ffffffff


/* Description		RESERVED_7A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_7A_OFFSET                            0x0000000000000018
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_7A_LSB                               32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_7A_MSB                               63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_7A_MASK                              0xffffffff00000000


/* Description		RESERVED_8A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_8A_OFFSET                            0x0000000000000020
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_8A_LSB                               0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_8A_MSB                               31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_8A_MASK                              0x00000000ffffffff


/* Description		RESERVED_9A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_9A_OFFSET                            0x0000000000000020
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_9A_LSB                               32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_9A_MSB                               63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_9A_MASK                              0xffffffff00000000


/* Description		RESERVED_10A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_10A_OFFSET                           0x0000000000000028
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_10A_LSB                              0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_10A_MSB                              31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_10A_MASK                             0x00000000ffffffff


/* Description		RESERVED_11A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_11A_OFFSET                           0x0000000000000028
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_11A_LSB                              32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_11A_MSB                              63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_11A_MASK                             0xffffffff00000000


/* Description		RESERVED_12A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_12A_OFFSET                           0x0000000000000030
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_12A_LSB                              0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_12A_MSB                              31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_12A_MASK                             0x00000000ffffffff


/* Description		RESERVED_13A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_13A_OFFSET                           0x0000000000000030
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_13A_LSB                              32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_13A_MSB                              63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_13A_MASK                             0xffffffff00000000


/* Description		RESERVED_14A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_14A_OFFSET                           0x0000000000000038
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_14A_LSB                              0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_14A_MSB                              31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_14A_MASK                             0x00000000ffffffff


/* Description		RESERVED_15A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_15A_OFFSET                           0x0000000000000038
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_15A_LSB                              32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_15A_MSB                              63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_15A_MASK                             0xffffffff00000000


/* Description		RESERVED_16A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_16A_OFFSET                           0x0000000000000040
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_16A_LSB                              0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_16A_MSB                              31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_16A_MASK                             0x00000000ffffffff


/* Description		RESERVED_17A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_17A_OFFSET                           0x0000000000000040
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_17A_LSB                              32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_17A_MSB                              63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_17A_MASK                             0xffffffff00000000


/* Description		RESERVED_18A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_18A_OFFSET                           0x0000000000000048
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_18A_LSB                              0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_18A_MSB                              31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_18A_MASK                             0x00000000ffffffff


/* Description		RESERVED_19A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_19A_OFFSET                           0x0000000000000048
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_19A_LSB                              32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_19A_MSB                              63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_19A_MASK                             0xffffffff00000000


/* Description		RESERVED_20A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_20A_OFFSET                           0x0000000000000050
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_20A_LSB                              0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_20A_MSB                              31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_20A_MASK                             0x00000000ffffffff


/* Description		RESERVED_21A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_21A_OFFSET                           0x0000000000000050
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_21A_LSB                              32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_21A_MSB                              63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_21A_MASK                             0xffffffff00000000


/* Description		RESERVED_22A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_22A_OFFSET                           0x0000000000000058
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_22A_LSB                              0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_22A_MSB                              31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_22A_MASK                             0x00000000ffffffff


/* Description		RESERVED_23A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_23A_OFFSET                           0x0000000000000058
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_23A_LSB                              32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_23A_MSB                              63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_23A_MASK                             0xffffffff00000000


/* Description		RESERVED_24A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_24A_OFFSET                           0x0000000000000060
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_24A_LSB                              0
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_24A_MSB                              31
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_24A_MASK                             0x00000000ffffffff


/* Description		RESERVED_25A

			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_25A_OFFSET                           0x0000000000000060
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_25A_LSB                              32
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_25A_MSB                              59
#define REO_FLUSH_TIMEOUT_LIST_STATUS_RESERVED_25A_MASK                             0x0fffffff00000000


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

#define REO_FLUSH_TIMEOUT_LIST_STATUS_LOOPING_COUNT_OFFSET                          0x0000000000000060
#define REO_FLUSH_TIMEOUT_LIST_STATUS_LOOPING_COUNT_LSB                             60
#define REO_FLUSH_TIMEOUT_LIST_STATUS_LOOPING_COUNT_MSB                             63
#define REO_FLUSH_TIMEOUT_LIST_STATUS_LOOPING_COUNT_MASK                            0xf000000000000000



#endif   // REO_FLUSH_TIMEOUT_LIST_STATUS
