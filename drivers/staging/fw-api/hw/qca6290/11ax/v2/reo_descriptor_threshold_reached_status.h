/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#ifndef _REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_H_
#define _REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_status_header.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0-1	struct uniform_reo_status_header status_header;
//	2	threshold_index[1:0], reserved_2[31:2]
//	3	link_descriptor_counter0[23:0], reserved_3[31:24]
//	4	link_descriptor_counter1[23:0], reserved_4[31:24]
//	5	link_descriptor_counter2[23:0], reserved_5[31:24]
//	6	link_descriptor_counter_sum[25:0], reserved_6[31:26]
//	7	reserved_7[31:0]
//	8	reserved_8[31:0]
//	9	reserved_9a[31:0]
//	10	reserved_10a[31:0]
//	11	reserved_11a[31:0]
//	12	reserved_12a[31:0]
//	13	reserved_13a[31:0]
//	14	reserved_14a[31:0]
//	15	reserved_15a[31:0]
//	16	reserved_16a[31:0]
//	17	reserved_17a[31:0]
//	18	reserved_18a[31:0]
//	19	reserved_19a[31:0]
//	20	reserved_20a[31:0]
//	21	reserved_21a[31:0]
//	22	reserved_22a[31:0]
//	23	reserved_23a[31:0]
//	24	reserved_24a[27:0], looping_count[31:28]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS 25

struct reo_descriptor_threshold_reached_status {
    struct            uniform_reo_status_header                       status_header;
             uint32_t threshold_index                 :  2, //[1:0]
                      reserved_2                      : 30; //[31:2]
             uint32_t link_descriptor_counter0        : 24, //[23:0]
                      reserved_3                      :  8; //[31:24]
             uint32_t link_descriptor_counter1        : 24, //[23:0]
                      reserved_4                      :  8; //[31:24]
             uint32_t link_descriptor_counter2        : 24, //[23:0]
                      reserved_5                      :  8; //[31:24]
             uint32_t link_descriptor_counter_sum     : 26, //[25:0]
                      reserved_6                      :  6; //[31:26]
             uint32_t reserved_7                      : 32; //[31:0]
             uint32_t reserved_8                      : 32; //[31:0]
             uint32_t reserved_9a                     : 32; //[31:0]
             uint32_t reserved_10a                    : 32; //[31:0]
             uint32_t reserved_11a                    : 32; //[31:0]
             uint32_t reserved_12a                    : 32; //[31:0]
             uint32_t reserved_13a                    : 32; //[31:0]
             uint32_t reserved_14a                    : 32; //[31:0]
             uint32_t reserved_15a                    : 32; //[31:0]
             uint32_t reserved_16a                    : 32; //[31:0]
             uint32_t reserved_17a                    : 32; //[31:0]
             uint32_t reserved_18a                    : 32; //[31:0]
             uint32_t reserved_19a                    : 32; //[31:0]
             uint32_t reserved_20a                    : 32; //[31:0]
             uint32_t reserved_21a                    : 32; //[31:0]
             uint32_t reserved_22a                    : 32; //[31:0]
             uint32_t reserved_23a                    : 32; //[31:0]
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

threshold_index
			
			The index of the threshold register whose value got
			reached
			
			
			
			<enum 0     reo_desc_counter0_threshold>
			
			<enum 1     reo_desc_counter1_threshold>
			
			<enum 2     reo_desc_counter2_threshold>
			
			<enum 3     reo_desc_counter_sum_threshold>
			
			
			
			<legal all>

reserved_2
			
			<legal 0>

link_descriptor_counter0
			
			Value of this counter at generation of this message
			
			<legal all>

reserved_3
			
			<legal 0>

link_descriptor_counter1
			
			Value of this counter at generation of this message
			
			<legal all>

reserved_4
			
			<legal 0>

link_descriptor_counter2
			
			Value of this counter at generation of this message
			
			<legal all>

reserved_5
			
			<legal 0>

link_descriptor_counter_sum
			
			Value of this counter at generation of this message
			
			<legal all>

reserved_6
			
			<legal 0>

reserved_7
			
			<legal 0>

reserved_8
			
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

reserved_15a
			
			<legal 0>

reserved_16a
			
			<legal 0>

reserved_17a
			
			<legal 0>

reserved_18a
			
			<legal 0>

reserved_19a
			
			<legal 0>

reserved_20a
			
			<legal 0>

reserved_21a
			
			<legal 0>

reserved_22a
			
			<legal 0>

reserved_23a
			
			<legal 0>

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

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_0_UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_OFFSET 0x00000000
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_0_UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_LSB 28
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_0_UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_MASK 0xffffffff
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_1_UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_OFFSET 0x00000004
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_1_UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_LSB 28
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_1_UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_2_THRESHOLD_INDEX
			
			The index of the threshold register whose value got
			reached
			
			
			
			<enum 0     reo_desc_counter0_threshold>
			
			<enum 1     reo_desc_counter1_threshold>
			
			<enum 2     reo_desc_counter2_threshold>
			
			<enum 3     reo_desc_counter_sum_threshold>
			
			
			
			<legal all>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_2_THRESHOLD_INDEX_OFFSET 0x00000008
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_2_THRESHOLD_INDEX_LSB 0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_2_THRESHOLD_INDEX_MASK 0x00000003

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_2_RESERVED_2
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_2_RESERVED_2_OFFSET  0x00000008
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_2_RESERVED_2_LSB     2
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_2_RESERVED_2_MASK    0xfffffffc

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_3_LINK_DESCRIPTOR_COUNTER0
			
			Value of this counter at generation of this message
			
			<legal all>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_3_LINK_DESCRIPTOR_COUNTER0_OFFSET 0x0000000c
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_3_LINK_DESCRIPTOR_COUNTER0_LSB 0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_3_LINK_DESCRIPTOR_COUNTER0_MASK 0x00ffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_3_RESERVED_3
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_3_RESERVED_3_OFFSET  0x0000000c
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_3_RESERVED_3_LSB     24
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_3_RESERVED_3_MASK    0xff000000

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_4_LINK_DESCRIPTOR_COUNTER1
			
			Value of this counter at generation of this message
			
			<legal all>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_4_LINK_DESCRIPTOR_COUNTER1_OFFSET 0x00000010
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_4_LINK_DESCRIPTOR_COUNTER1_LSB 0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_4_LINK_DESCRIPTOR_COUNTER1_MASK 0x00ffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_4_RESERVED_4
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_4_RESERVED_4_OFFSET  0x00000010
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_4_RESERVED_4_LSB     24
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_4_RESERVED_4_MASK    0xff000000

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_5_LINK_DESCRIPTOR_COUNTER2
			
			Value of this counter at generation of this message
			
			<legal all>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_5_LINK_DESCRIPTOR_COUNTER2_OFFSET 0x00000014
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_5_LINK_DESCRIPTOR_COUNTER2_LSB 0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_5_LINK_DESCRIPTOR_COUNTER2_MASK 0x00ffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_5_RESERVED_5
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_5_RESERVED_5_OFFSET  0x00000014
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_5_RESERVED_5_LSB     24
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_5_RESERVED_5_MASK    0xff000000

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_6_LINK_DESCRIPTOR_COUNTER_SUM
			
			Value of this counter at generation of this message
			
			<legal all>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_6_LINK_DESCRIPTOR_COUNTER_SUM_OFFSET 0x00000018
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_6_LINK_DESCRIPTOR_COUNTER_SUM_LSB 0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_6_LINK_DESCRIPTOR_COUNTER_SUM_MASK 0x03ffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_6_RESERVED_6
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_6_RESERVED_6_OFFSET  0x00000018
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_6_RESERVED_6_LSB     26
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_6_RESERVED_6_MASK    0xfc000000

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_7_RESERVED_7
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_7_RESERVED_7_OFFSET  0x0000001c
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_7_RESERVED_7_LSB     0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_7_RESERVED_7_MASK    0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_8_RESERVED_8
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_8_RESERVED_8_OFFSET  0x00000020
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_8_RESERVED_8_LSB     0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_8_RESERVED_8_MASK    0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_9_RESERVED_9A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_9_RESERVED_9A_OFFSET 0x00000024
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_9_RESERVED_9A_LSB    0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_9_RESERVED_9A_MASK   0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_10_RESERVED_10A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_10_RESERVED_10A_OFFSET 0x00000028
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_10_RESERVED_10A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_10_RESERVED_10A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_11_RESERVED_11A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_11_RESERVED_11A_OFFSET 0x0000002c
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_11_RESERVED_11A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_11_RESERVED_11A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_12_RESERVED_12A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_12_RESERVED_12A_OFFSET 0x00000030
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_12_RESERVED_12A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_12_RESERVED_12A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_13_RESERVED_13A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_13_RESERVED_13A_OFFSET 0x00000034
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_13_RESERVED_13A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_13_RESERVED_13A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_14_RESERVED_14A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_14_RESERVED_14A_OFFSET 0x00000038
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_14_RESERVED_14A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_14_RESERVED_14A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_15_RESERVED_15A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_15_RESERVED_15A_OFFSET 0x0000003c
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_15_RESERVED_15A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_15_RESERVED_15A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_16_RESERVED_16A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_16_RESERVED_16A_OFFSET 0x00000040
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_16_RESERVED_16A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_16_RESERVED_16A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_17_RESERVED_17A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_17_RESERVED_17A_OFFSET 0x00000044
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_17_RESERVED_17A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_17_RESERVED_17A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_18_RESERVED_18A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_18_RESERVED_18A_OFFSET 0x00000048
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_18_RESERVED_18A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_18_RESERVED_18A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_19_RESERVED_19A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_19_RESERVED_19A_OFFSET 0x0000004c
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_19_RESERVED_19A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_19_RESERVED_19A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_20_RESERVED_20A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_20_RESERVED_20A_OFFSET 0x00000050
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_20_RESERVED_20A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_20_RESERVED_20A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_21_RESERVED_21A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_21_RESERVED_21A_OFFSET 0x00000054
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_21_RESERVED_21A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_21_RESERVED_21A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_22_RESERVED_22A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_22_RESERVED_22A_OFFSET 0x00000058
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_22_RESERVED_22A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_22_RESERVED_22A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_23_RESERVED_23A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_23_RESERVED_23A_OFFSET 0x0000005c
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_23_RESERVED_23A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_23_RESERVED_23A_MASK 0xffffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_24_RESERVED_24A
			
			<legal 0>
*/
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_24_RESERVED_24A_OFFSET 0x00000060
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_24_RESERVED_24A_LSB  0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_24_RESERVED_24A_MASK 0x0fffffff

/* Description		REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_24_LOOPING_COUNT
			
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
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_24_LOOPING_COUNT_OFFSET 0x00000060
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_24_LOOPING_COUNT_LSB 28
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_24_LOOPING_COUNT_MASK 0xf0000000


#endif // _REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_H_
