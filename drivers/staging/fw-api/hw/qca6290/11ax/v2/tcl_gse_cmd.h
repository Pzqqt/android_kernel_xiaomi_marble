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

#ifndef _TCL_GSE_CMD_H_
#define _TCL_GSE_CMD_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	control_buffer_addr_31_0[31:0]
//	1	control_buffer_addr_39_32[7:0], gse_ctrl[11:8], gse_sel[12], status_destination_ring_id[13], swap[14], index_search_en[15], cache_set_num[19:16], reserved_1a[31:20]
//	2	cmd_meta_data_31_0[31:0]
//	3	cmd_meta_data_63_32[31:0]
//	4	reserved_4a[31:0]
//	5	reserved_5a[31:0]
//	6	reserved_6a[19:0], ring_id[27:20], looping_count[31:28]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_TCL_GSE_CMD 7

struct tcl_gse_cmd {
             uint32_t control_buffer_addr_31_0        : 32; //[31:0]
             uint32_t control_buffer_addr_39_32       :  8, //[7:0]
                      gse_ctrl                        :  4, //[11:8]
                      gse_sel                         :  1, //[12]
                      status_destination_ring_id      :  1, //[13]
                      swap                            :  1, //[14]
                      index_search_en                 :  1, //[15]
                      cache_set_num                   :  4, //[19:16]
                      reserved_1a                     : 12; //[31:20]
             uint32_t cmd_meta_data_31_0              : 32; //[31:0]
             uint32_t cmd_meta_data_63_32             : 32; //[31:0]
             uint32_t reserved_4a                     : 32; //[31:0]
             uint32_t reserved_5a                     : 32; //[31:0]
             uint32_t reserved_6a                     : 20, //[19:0]
                      ring_id                         :  8, //[27:20]
                      looping_count                   :  4; //[31:28]
};

/*

control_buffer_addr_31_0
			
			Address (lower 32 bits) of a control buffer containing
			additional info needed for this command execution.
			
			<legal all>

control_buffer_addr_39_32
			
			Address (upper 8 bits) of a control buffer containing
			additional info needed for this command execution.
			
			<legal all>

gse_ctrl
			
			GSE control operations. This includes cache operations
			and table entry statistics read/clear operation.
			
			<enum 0 rd_stat> Report or Read statistics
			
			<enum 1 srch_dis> Search disable. Report only Hash
			
			<enum 2 Wr_bk_single> Write Back single entry
			
			<enum 3 wr_bk_all> Write Back entire cache entry
			
			<enum 4 inval_single> Invalidate single cache entry
			
			<enum 5 inval_all> Invalidate entire cache
			
			<enum 6 wr_bk_inval_single> Write back and Invalidate 
			single entry in cache
			
			<enum 7 wr_bk_inval_all> write back and invalidate
			entire cache
			
			<enum 8 clr_stat_single> Clear statistics for single
			entry
			
			<legal 0-8>
			
			Rest of the values reserved. 
			
			For all single entry control operations (write back,
			Invalidate or both)Statistics will be reported

gse_sel
			
			Bit to select the ASE or FSE to do the operation mention
			by GSE_ctrl bit
			
			0: FSE select
			
			1: ASE select

status_destination_ring_id
			
			The TCL status ring to which the GSE status needs to be
			send.
			
			
			
			<enum 0 tcl_status_0_ring>
			
			<enum 1 tcl_status_1_ring>
			
			
			
			<legal all>

swap
			
			Bit to enable byte swapping of contents of buffer
			
			<enum 0 Byte_swap_disable > 
			
			<enum 1 byte_swap_enable >
			
			<legal all>

index_search_en
			
			When this bit is set to 1 control_buffer_addr[19:0] will
			be considered as index of the AST or Flow table and GSE
			commands will be executed accordingly on the entry pointed
			by the index. 
			
			This feature is disabled by setting this bit to 0.
			
			<enum 0 index_based_cmd_disable>
			
			<enum 1 index_based_cmd_enable>
			
			
			
			<legal all>

cache_set_num
			
			Cache set number that should be used to cache the index
			based search results, for address and flow search. This
			value should be equal to value of cache_set_num for the
			index that is issued in TCL_DATA_CMD during search index
			based ASE or FSE. This field is valid for index based GSE
			commands
			
			<legal all>

reserved_1a
			
			<legal 0>

cmd_meta_data_31_0
			
			Meta data to be returned in the status descriptor
			
			<legal all>

cmd_meta_data_63_32
			
			Meta data to be returned in the status descriptor
			
			<legal all>

reserved_4a
			
			<legal 0>

reserved_5a
			
			<legal 0>

reserved_6a
			
			<legal 0>

ring_id
			
			Helps with debugging when dumping ring contents.
			
			<legal all>

looping_count
			
			A count value that indicates the number of times the
			producer of entries into the Ring has looped around the
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


/* Description		TCL_GSE_CMD_0_CONTROL_BUFFER_ADDR_31_0
			
			Address (lower 32 bits) of a control buffer containing
			additional info needed for this command execution.
			
			<legal all>
*/
#define TCL_GSE_CMD_0_CONTROL_BUFFER_ADDR_31_0_OFFSET                0x00000000
#define TCL_GSE_CMD_0_CONTROL_BUFFER_ADDR_31_0_LSB                   0
#define TCL_GSE_CMD_0_CONTROL_BUFFER_ADDR_31_0_MASK                  0xffffffff

/* Description		TCL_GSE_CMD_1_CONTROL_BUFFER_ADDR_39_32
			
			Address (upper 8 bits) of a control buffer containing
			additional info needed for this command execution.
			
			<legal all>
*/
#define TCL_GSE_CMD_1_CONTROL_BUFFER_ADDR_39_32_OFFSET               0x00000004
#define TCL_GSE_CMD_1_CONTROL_BUFFER_ADDR_39_32_LSB                  0
#define TCL_GSE_CMD_1_CONTROL_BUFFER_ADDR_39_32_MASK                 0x000000ff

/* Description		TCL_GSE_CMD_1_GSE_CTRL
			
			GSE control operations. This includes cache operations
			and table entry statistics read/clear operation.
			
			<enum 0 rd_stat> Report or Read statistics
			
			<enum 1 srch_dis> Search disable. Report only Hash
			
			<enum 2 Wr_bk_single> Write Back single entry
			
			<enum 3 wr_bk_all> Write Back entire cache entry
			
			<enum 4 inval_single> Invalidate single cache entry
			
			<enum 5 inval_all> Invalidate entire cache
			
			<enum 6 wr_bk_inval_single> Write back and Invalidate 
			single entry in cache
			
			<enum 7 wr_bk_inval_all> write back and invalidate
			entire cache
			
			<enum 8 clr_stat_single> Clear statistics for single
			entry
			
			<legal 0-8>
			
			Rest of the values reserved. 
			
			For all single entry control operations (write back,
			Invalidate or both)Statistics will be reported
*/
#define TCL_GSE_CMD_1_GSE_CTRL_OFFSET                                0x00000004
#define TCL_GSE_CMD_1_GSE_CTRL_LSB                                   8
#define TCL_GSE_CMD_1_GSE_CTRL_MASK                                  0x00000f00

/* Description		TCL_GSE_CMD_1_GSE_SEL
			
			Bit to select the ASE or FSE to do the operation mention
			by GSE_ctrl bit
			
			0: FSE select
			
			1: ASE select
*/
#define TCL_GSE_CMD_1_GSE_SEL_OFFSET                                 0x00000004
#define TCL_GSE_CMD_1_GSE_SEL_LSB                                    12
#define TCL_GSE_CMD_1_GSE_SEL_MASK                                   0x00001000

/* Description		TCL_GSE_CMD_1_STATUS_DESTINATION_RING_ID
			
			The TCL status ring to which the GSE status needs to be
			send.
			
			
			
			<enum 0 tcl_status_0_ring>
			
			<enum 1 tcl_status_1_ring>
			
			
			
			<legal all>
*/
#define TCL_GSE_CMD_1_STATUS_DESTINATION_RING_ID_OFFSET              0x00000004
#define TCL_GSE_CMD_1_STATUS_DESTINATION_RING_ID_LSB                 13
#define TCL_GSE_CMD_1_STATUS_DESTINATION_RING_ID_MASK                0x00002000

/* Description		TCL_GSE_CMD_1_SWAP
			
			Bit to enable byte swapping of contents of buffer
			
			<enum 0 Byte_swap_disable > 
			
			<enum 1 byte_swap_enable >
			
			<legal all>
*/
#define TCL_GSE_CMD_1_SWAP_OFFSET                                    0x00000004
#define TCL_GSE_CMD_1_SWAP_LSB                                       14
#define TCL_GSE_CMD_1_SWAP_MASK                                      0x00004000

/* Description		TCL_GSE_CMD_1_INDEX_SEARCH_EN
			
			When this bit is set to 1 control_buffer_addr[19:0] will
			be considered as index of the AST or Flow table and GSE
			commands will be executed accordingly on the entry pointed
			by the index. 
			
			This feature is disabled by setting this bit to 0.
			
			<enum 0 index_based_cmd_disable>
			
			<enum 1 index_based_cmd_enable>
			
			
			
			<legal all>
*/
#define TCL_GSE_CMD_1_INDEX_SEARCH_EN_OFFSET                         0x00000004
#define TCL_GSE_CMD_1_INDEX_SEARCH_EN_LSB                            15
#define TCL_GSE_CMD_1_INDEX_SEARCH_EN_MASK                           0x00008000

/* Description		TCL_GSE_CMD_1_CACHE_SET_NUM
			
			Cache set number that should be used to cache the index
			based search results, for address and flow search. This
			value should be equal to value of cache_set_num for the
			index that is issued in TCL_DATA_CMD during search index
			based ASE or FSE. This field is valid for index based GSE
			commands
			
			<legal all>
*/
#define TCL_GSE_CMD_1_CACHE_SET_NUM_OFFSET                           0x00000004
#define TCL_GSE_CMD_1_CACHE_SET_NUM_LSB                              16
#define TCL_GSE_CMD_1_CACHE_SET_NUM_MASK                             0x000f0000

/* Description		TCL_GSE_CMD_1_RESERVED_1A
			
			<legal 0>
*/
#define TCL_GSE_CMD_1_RESERVED_1A_OFFSET                             0x00000004
#define TCL_GSE_CMD_1_RESERVED_1A_LSB                                20
#define TCL_GSE_CMD_1_RESERVED_1A_MASK                               0xfff00000

/* Description		TCL_GSE_CMD_2_CMD_META_DATA_31_0
			
			Meta data to be returned in the status descriptor
			
			<legal all>
*/
#define TCL_GSE_CMD_2_CMD_META_DATA_31_0_OFFSET                      0x00000008
#define TCL_GSE_CMD_2_CMD_META_DATA_31_0_LSB                         0
#define TCL_GSE_CMD_2_CMD_META_DATA_31_0_MASK                        0xffffffff

/* Description		TCL_GSE_CMD_3_CMD_META_DATA_63_32
			
			Meta data to be returned in the status descriptor
			
			<legal all>
*/
#define TCL_GSE_CMD_3_CMD_META_DATA_63_32_OFFSET                     0x0000000c
#define TCL_GSE_CMD_3_CMD_META_DATA_63_32_LSB                        0
#define TCL_GSE_CMD_3_CMD_META_DATA_63_32_MASK                       0xffffffff

/* Description		TCL_GSE_CMD_4_RESERVED_4A
			
			<legal 0>
*/
#define TCL_GSE_CMD_4_RESERVED_4A_OFFSET                             0x00000010
#define TCL_GSE_CMD_4_RESERVED_4A_LSB                                0
#define TCL_GSE_CMD_4_RESERVED_4A_MASK                               0xffffffff

/* Description		TCL_GSE_CMD_5_RESERVED_5A
			
			<legal 0>
*/
#define TCL_GSE_CMD_5_RESERVED_5A_OFFSET                             0x00000014
#define TCL_GSE_CMD_5_RESERVED_5A_LSB                                0
#define TCL_GSE_CMD_5_RESERVED_5A_MASK                               0xffffffff

/* Description		TCL_GSE_CMD_6_RESERVED_6A
			
			<legal 0>
*/
#define TCL_GSE_CMD_6_RESERVED_6A_OFFSET                             0x00000018
#define TCL_GSE_CMD_6_RESERVED_6A_LSB                                0
#define TCL_GSE_CMD_6_RESERVED_6A_MASK                               0x000fffff

/* Description		TCL_GSE_CMD_6_RING_ID
			
			Helps with debugging when dumping ring contents.
			
			<legal all>
*/
#define TCL_GSE_CMD_6_RING_ID_OFFSET                                 0x00000018
#define TCL_GSE_CMD_6_RING_ID_LSB                                    20
#define TCL_GSE_CMD_6_RING_ID_MASK                                   0x0ff00000

/* Description		TCL_GSE_CMD_6_LOOPING_COUNT
			
			A count value that indicates the number of times the
			producer of entries into the Ring has looped around the
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
#define TCL_GSE_CMD_6_LOOPING_COUNT_OFFSET                           0x00000018
#define TCL_GSE_CMD_6_LOOPING_COUNT_LSB                              28
#define TCL_GSE_CMD_6_LOOPING_COUNT_MASK                             0xf0000000


#endif // _TCL_GSE_CMD_H_
