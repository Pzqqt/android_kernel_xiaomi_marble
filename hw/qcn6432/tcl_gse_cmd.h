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

#ifndef _TCL_GSE_CMD_H_
#define _TCL_GSE_CMD_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TCL_GSE_CMD 8


struct tcl_gse_cmd {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t control_buffer_addr_31_0                                : 32; // [31:0]
             uint32_t control_buffer_addr_39_32                               :  8, // [7:0]
                      gse_ctrl                                                :  4, // [11:8]
                      gse_sel                                                 :  1, // [12:12]
                      status_destination_ring_id                              :  1, // [13:13]
                      swap                                                    :  1, // [14:14]
                      index_search_en                                         :  1, // [15:15]
                      cache_set_num                                           :  4, // [19:16]
                      reserved_1a                                             : 12; // [31:20]
             uint32_t tcl_cmd_type                                            :  1, // [0:0]
                      reserved_2a                                             : 31; // [31:1]
             uint32_t cmd_meta_data_31_0                                      : 32; // [31:0]
             uint32_t cmd_meta_data_63_32                                     : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 20, // [19:0]
                      ring_id                                                 :  8, // [27:20]
                      looping_count                                           :  4; // [31:28]
#else
             uint32_t control_buffer_addr_31_0                                : 32; // [31:0]
             uint32_t reserved_1a                                             : 12, // [31:20]
                      cache_set_num                                           :  4, // [19:16]
                      index_search_en                                         :  1, // [15:15]
                      swap                                                    :  1, // [14:14]
                      status_destination_ring_id                              :  1, // [13:13]
                      gse_sel                                                 :  1, // [12:12]
                      gse_ctrl                                                :  4, // [11:8]
                      control_buffer_addr_39_32                               :  8; // [7:0]
             uint32_t reserved_2a                                             : 31, // [31:1]
                      tcl_cmd_type                                            :  1; // [0:0]
             uint32_t cmd_meta_data_31_0                                      : 32; // [31:0]
             uint32_t cmd_meta_data_63_32                                     : 32; // [31:0]
             uint32_t reserved_5a                                             : 32; // [31:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t looping_count                                           :  4, // [31:28]
                      ring_id                                                 :  8, // [27:20]
                      reserved_7a                                             : 20; // [19:0]
#endif
};


/* Description		CONTROL_BUFFER_ADDR_31_0

			Address (lower 32 bits) of a control buffer containing additional
			 info needed for this command execution.
			<legal all>
*/

#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_31_0_OFFSET                                 0x00000000
#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_31_0_LSB                                    0
#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_31_0_MSB                                    31
#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_31_0_MASK                                   0xffffffff


/* Description		CONTROL_BUFFER_ADDR_39_32

			Address (upper 8 bits) of a control buffer containing additional
			 info needed for this command execution.
			<legal all>
*/

#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_39_32_OFFSET                                0x00000004
#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_39_32_LSB                                   0
#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_39_32_MSB                                   7
#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_39_32_MASK                                  0x000000ff


/* Description		GSE_CTRL

			GSE control operations. This includes cache operations and
			 table entry statistics read/clear operation.
			<enum 0 rd_stat> Report or Read statistics
			<enum 1 srch_dis> Search disable. Report only Hash
			<enum 2 Wr_bk_single> Write Back single entry
			<enum 3 wr_bk_all> Write Back entire cache entry
			<enum 4 inval_single> Invalidate single cache entry
			<enum 5 inval_all> Invalidate entire cache
			<enum 6 wr_bk_inval_single> Write back and Invalidate  single
			 entry in cache
			<enum 7 wr_bk_inval_all> write back and invalidate entire
			 cache
			<enum 8 clr_stat_single> Clear statistics for single entry
			
			<legal 0-8>
			Rest of the values reserved. 
			For all single entry control operations (write back, Invalidate
			 or both)Statistics will be reported
*/

#define TCL_GSE_CMD_GSE_CTRL_OFFSET                                                 0x00000004
#define TCL_GSE_CMD_GSE_CTRL_LSB                                                    8
#define TCL_GSE_CMD_GSE_CTRL_MSB                                                    11
#define TCL_GSE_CMD_GSE_CTRL_MASK                                                   0x00000f00


/* Description		GSE_SEL

			Bit to select the ASE or FSE to do the operation mention
			 by GSE_ctrl bit
			0: FSE select
			1: ASE select
*/

#define TCL_GSE_CMD_GSE_SEL_OFFSET                                                  0x00000004
#define TCL_GSE_CMD_GSE_SEL_LSB                                                     12
#define TCL_GSE_CMD_GSE_SEL_MSB                                                     12
#define TCL_GSE_CMD_GSE_SEL_MASK                                                    0x00001000


/* Description		STATUS_DESTINATION_RING_ID

			The TCL status ring to which the GSE status needs to be 
			send.
			
			<enum 0 tcl_status_0_ring>
			<enum 1 tcl_status_1_ring>
			
			<legal all>
*/

#define TCL_GSE_CMD_STATUS_DESTINATION_RING_ID_OFFSET                               0x00000004
#define TCL_GSE_CMD_STATUS_DESTINATION_RING_ID_LSB                                  13
#define TCL_GSE_CMD_STATUS_DESTINATION_RING_ID_MSB                                  13
#define TCL_GSE_CMD_STATUS_DESTINATION_RING_ID_MASK                                 0x00002000


/* Description		SWAP

			Bit to enable byte swapping of contents of buffer
			<enum 0 Byte_swap_disable > 
			<enum 1 byte_swap_enable >
			<legal all>
*/

#define TCL_GSE_CMD_SWAP_OFFSET                                                     0x00000004
#define TCL_GSE_CMD_SWAP_LSB                                                        14
#define TCL_GSE_CMD_SWAP_MSB                                                        14
#define TCL_GSE_CMD_SWAP_MASK                                                       0x00004000


/* Description		INDEX_SEARCH_EN

			When this bit is set to 1 control_buffer_addr[19:0] will
			 be considered as index of the AST or Flow table and GSE
			 commands will be executed accordingly on the entry pointed
			 by the index. 
			This feature is disabled by setting this bit to 0.
			<enum 0 index_based_cmd_disable>
			<enum 1 index_based_cmd_enable>
			
			<legal all>
*/

#define TCL_GSE_CMD_INDEX_SEARCH_EN_OFFSET                                          0x00000004
#define TCL_GSE_CMD_INDEX_SEARCH_EN_LSB                                             15
#define TCL_GSE_CMD_INDEX_SEARCH_EN_MSB                                             15
#define TCL_GSE_CMD_INDEX_SEARCH_EN_MASK                                            0x00008000


/* Description		CACHE_SET_NUM

			Cache set number that should be used to cache the index 
			based search results, for address and flow search. This 
			value should be equal to value of cache_set_num for the 
			index that is issued in TCL_DATA_CMD during search index
			 based ASE or FSE. This field is valid for index based GSE
			 commands
			<legal all>
*/

#define TCL_GSE_CMD_CACHE_SET_NUM_OFFSET                                            0x00000004
#define TCL_GSE_CMD_CACHE_SET_NUM_LSB                                               16
#define TCL_GSE_CMD_CACHE_SET_NUM_MSB                                               19
#define TCL_GSE_CMD_CACHE_SET_NUM_MASK                                              0x000f0000


/* Description		RESERVED_1A

			<legal 0>
*/

#define TCL_GSE_CMD_RESERVED_1A_OFFSET                                              0x00000004
#define TCL_GSE_CMD_RESERVED_1A_LSB                                                 20
#define TCL_GSE_CMD_RESERVED_1A_MSB                                                 31
#define TCL_GSE_CMD_RESERVED_1A_MASK                                                0xfff00000


/* Description		TCL_CMD_TYPE

			This field is used to select the type of TCL Command decriptor
			 that is queued by SW/FW. For 'TCL_GSE_CMD' this has to 
			be 1.
			<legal 1>
*/

#define TCL_GSE_CMD_TCL_CMD_TYPE_OFFSET                                             0x00000008
#define TCL_GSE_CMD_TCL_CMD_TYPE_LSB                                                0
#define TCL_GSE_CMD_TCL_CMD_TYPE_MSB                                                0
#define TCL_GSE_CMD_TCL_CMD_TYPE_MASK                                               0x00000001


/* Description		RESERVED_2A

			<legal 0>
*/

#define TCL_GSE_CMD_RESERVED_2A_OFFSET                                              0x00000008
#define TCL_GSE_CMD_RESERVED_2A_LSB                                                 1
#define TCL_GSE_CMD_RESERVED_2A_MSB                                                 31
#define TCL_GSE_CMD_RESERVED_2A_MASK                                                0xfffffffe


/* Description		CMD_META_DATA_31_0

			Meta data to be returned in the status descriptor
			<legal all>
*/

#define TCL_GSE_CMD_CMD_META_DATA_31_0_OFFSET                                       0x0000000c
#define TCL_GSE_CMD_CMD_META_DATA_31_0_LSB                                          0
#define TCL_GSE_CMD_CMD_META_DATA_31_0_MSB                                          31
#define TCL_GSE_CMD_CMD_META_DATA_31_0_MASK                                         0xffffffff


/* Description		CMD_META_DATA_63_32

			Meta data to be returned in the status descriptor
			<legal all>
*/

#define TCL_GSE_CMD_CMD_META_DATA_63_32_OFFSET                                      0x00000010
#define TCL_GSE_CMD_CMD_META_DATA_63_32_LSB                                         0
#define TCL_GSE_CMD_CMD_META_DATA_63_32_MSB                                         31
#define TCL_GSE_CMD_CMD_META_DATA_63_32_MASK                                        0xffffffff


/* Description		RESERVED_5A

			<legal 0>
*/

#define TCL_GSE_CMD_RESERVED_5A_OFFSET                                              0x00000014
#define TCL_GSE_CMD_RESERVED_5A_LSB                                                 0
#define TCL_GSE_CMD_RESERVED_5A_MSB                                                 31
#define TCL_GSE_CMD_RESERVED_5A_MASK                                                0xffffffff


/* Description		RESERVED_6A

			<legal 0>
*/

#define TCL_GSE_CMD_RESERVED_6A_OFFSET                                              0x00000018
#define TCL_GSE_CMD_RESERVED_6A_LSB                                                 0
#define TCL_GSE_CMD_RESERVED_6A_MSB                                                 31
#define TCL_GSE_CMD_RESERVED_6A_MASK                                                0xffffffff


/* Description		RESERVED_7A

			<legal 0>
*/

#define TCL_GSE_CMD_RESERVED_7A_OFFSET                                              0x0000001c
#define TCL_GSE_CMD_RESERVED_7A_LSB                                                 0
#define TCL_GSE_CMD_RESERVED_7A_MSB                                                 19
#define TCL_GSE_CMD_RESERVED_7A_MASK                                                0x000fffff


/* Description		RING_ID

			Helps with debugging when dumping ring contents.
			<legal all>
*/

#define TCL_GSE_CMD_RING_ID_OFFSET                                                  0x0000001c
#define TCL_GSE_CMD_RING_ID_LSB                                                     20
#define TCL_GSE_CMD_RING_ID_MSB                                                     27
#define TCL_GSE_CMD_RING_ID_MASK                                                    0x0ff00000


/* Description		LOOPING_COUNT

			A count value that indicates the number of times the producer
			 of entries into the Ring has looped around the ring.
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

#define TCL_GSE_CMD_LOOPING_COUNT_OFFSET                                            0x0000001c
#define TCL_GSE_CMD_LOOPING_COUNT_LSB                                               28
#define TCL_GSE_CMD_LOOPING_COUNT_MSB                                               31
#define TCL_GSE_CMD_LOOPING_COUNT_MASK                                              0xf0000000



#endif   // TCL_GSE_CMD
