
/* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

 
 
 
 
 
 
 


#ifndef _TCL_STATUS_RING_H_
#define _TCL_STATUS_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TCL_STATUS_RING 8


struct tcl_status_ring {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t gse_ctrl                                                :  4, // [3:0]
                      ase_fse_sel                                             :  1, // [4:4]
                      cache_op_res                                            :  2, // [6:5]
                      index_search_en                                         :  1, // [7:7]
                      msdu_cnt_n                                              : 24; // [31:8]
             uint32_t msdu_byte_cnt_n                                         : 32; // [31:0]
             uint32_t msdu_timestmp_n                                         : 32; // [31:0]
             uint32_t cmd_meta_data_31_0                                      : 32; // [31:0]
             uint32_t cmd_meta_data_63_32                                     : 32; // [31:0]
             uint32_t hash_indx_val                                           : 20, // [19:0]
                      cache_set_num                                           :  4, // [23:20]
                      reserved_5a                                             :  8; // [31:24]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 20, // [19:0]
                      ring_id                                                 :  8, // [27:20]
                      looping_count                                           :  4; // [31:28]
#else
             uint32_t msdu_cnt_n                                              : 24, // [31:8]
                      index_search_en                                         :  1, // [7:7]
                      cache_op_res                                            :  2, // [6:5]
                      ase_fse_sel                                             :  1, // [4:4]
                      gse_ctrl                                                :  4; // [3:0]
             uint32_t msdu_byte_cnt_n                                         : 32; // [31:0]
             uint32_t msdu_timestmp_n                                         : 32; // [31:0]
             uint32_t cmd_meta_data_31_0                                      : 32; // [31:0]
             uint32_t cmd_meta_data_63_32                                     : 32; // [31:0]
             uint32_t reserved_5a                                             :  8, // [31:24]
                      cache_set_num                                           :  4, // [23:20]
                      hash_indx_val                                           : 20; // [19:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t looping_count                                           :  4, // [31:28]
                      ring_id                                                 :  8, // [27:20]
                      reserved_7a                                             : 20; // [19:0]
#endif
};


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

#define TCL_STATUS_RING_GSE_CTRL_OFFSET                                             0x00000000
#define TCL_STATUS_RING_GSE_CTRL_LSB                                                0
#define TCL_STATUS_RING_GSE_CTRL_MSB                                                3
#define TCL_STATUS_RING_GSE_CTRL_MASK                                               0x0000000f


/* Description		ASE_FSE_SEL

			Search Engine for which operation is done.
			1'b0: Address Search Engine Result
			1'b1: Flow Search Engine result
*/

#define TCL_STATUS_RING_ASE_FSE_SEL_OFFSET                                          0x00000000
#define TCL_STATUS_RING_ASE_FSE_SEL_LSB                                             4
#define TCL_STATUS_RING_ASE_FSE_SEL_MSB                                             4
#define TCL_STATUS_RING_ASE_FSE_SEL_MASK                                            0x00000010


/* Description		CACHE_OP_RES

			Cache operation result. Following are results of cache operation.
			
			<enum 0 op_done>  Operation successful
			<enum 1 not_fnd> Entry not found in Table
			<enum 2 timeout_er>  Timeout Error
			<legal 0-2>
*/

#define TCL_STATUS_RING_CACHE_OP_RES_OFFSET                                         0x00000000
#define TCL_STATUS_RING_CACHE_OP_RES_LSB                                            5
#define TCL_STATUS_RING_CACHE_OP_RES_MSB                                            6
#define TCL_STATUS_RING_CACHE_OP_RES_MASK                                           0x00000060


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

#define TCL_STATUS_RING_INDEX_SEARCH_EN_OFFSET                                      0x00000000
#define TCL_STATUS_RING_INDEX_SEARCH_EN_LSB                                         7
#define TCL_STATUS_RING_INDEX_SEARCH_EN_MSB                                         7
#define TCL_STATUS_RING_INDEX_SEARCH_EN_MASK                                        0x00000080


/* Description		MSDU_CNT_N

			MSDU count of Entry. Valid when GSE_CTRL is 4'b0111 and 
			4'b1000
*/

#define TCL_STATUS_RING_MSDU_CNT_N_OFFSET                                           0x00000000
#define TCL_STATUS_RING_MSDU_CNT_N_LSB                                              8
#define TCL_STATUS_RING_MSDU_CNT_N_MSB                                              31
#define TCL_STATUS_RING_MSDU_CNT_N_MASK                                             0xffffff00


/* Description		MSDU_BYTE_CNT_N

			MSDU byte count for entry 1. Valid when GSE_CTRL is 4'b0111
			 and 4'b1000
*/

#define TCL_STATUS_RING_MSDU_BYTE_CNT_N_OFFSET                                      0x00000004
#define TCL_STATUS_RING_MSDU_BYTE_CNT_N_LSB                                         0
#define TCL_STATUS_RING_MSDU_BYTE_CNT_N_MSB                                         31
#define TCL_STATUS_RING_MSDU_BYTE_CNT_N_MASK                                        0xffffffff


/* Description		MSDU_TIMESTMP_N

			MSDU timestamp for entry 1. Valid when GSE_CTRL is 4'b0111
			 and 4'b1000
*/

#define TCL_STATUS_RING_MSDU_TIMESTMP_N_OFFSET                                      0x00000008
#define TCL_STATUS_RING_MSDU_TIMESTMP_N_LSB                                         0
#define TCL_STATUS_RING_MSDU_TIMESTMP_N_MSB                                         31
#define TCL_STATUS_RING_MSDU_TIMESTMP_N_MASK                                        0xffffffff


/* Description		CMD_META_DATA_31_0

			Meta data from input ring
			<legal all>
*/

#define TCL_STATUS_RING_CMD_META_DATA_31_0_OFFSET                                   0x0000000c
#define TCL_STATUS_RING_CMD_META_DATA_31_0_LSB                                      0
#define TCL_STATUS_RING_CMD_META_DATA_31_0_MSB                                      31
#define TCL_STATUS_RING_CMD_META_DATA_31_0_MASK                                     0xffffffff


/* Description		CMD_META_DATA_63_32

			Meta data from input ring
			<legal all>
*/

#define TCL_STATUS_RING_CMD_META_DATA_63_32_OFFSET                                  0x00000010
#define TCL_STATUS_RING_CMD_META_DATA_63_32_LSB                                     0
#define TCL_STATUS_RING_CMD_META_DATA_63_32_MSB                                     31
#define TCL_STATUS_RING_CMD_META_DATA_63_32_MASK                                    0xffffffff


/* Description		HASH_INDX_VAL

			Index of entry in the table in case of search pass  (or)
			
			Hash value of the entry in table in case of search failed
			 or search disable.
			<legal all>
*/

#define TCL_STATUS_RING_HASH_INDX_VAL_OFFSET                                        0x00000014
#define TCL_STATUS_RING_HASH_INDX_VAL_LSB                                           0
#define TCL_STATUS_RING_HASH_INDX_VAL_MSB                                           19
#define TCL_STATUS_RING_HASH_INDX_VAL_MASK                                          0x000fffff


/* Description		CACHE_SET_NUM

			Cache set number copied from TCL_GSE_CMD
*/

#define TCL_STATUS_RING_CACHE_SET_NUM_OFFSET                                        0x00000014
#define TCL_STATUS_RING_CACHE_SET_NUM_LSB                                           20
#define TCL_STATUS_RING_CACHE_SET_NUM_MSB                                           23
#define TCL_STATUS_RING_CACHE_SET_NUM_MASK                                          0x00f00000


/* Description		RESERVED_5A

			<legal 0>
*/

#define TCL_STATUS_RING_RESERVED_5A_OFFSET                                          0x00000014
#define TCL_STATUS_RING_RESERVED_5A_LSB                                             24
#define TCL_STATUS_RING_RESERVED_5A_MSB                                             31
#define TCL_STATUS_RING_RESERVED_5A_MASK                                            0xff000000


/* Description		RESERVED_6A

			<legal 0>
*/

#define TCL_STATUS_RING_RESERVED_6A_OFFSET                                          0x00000018
#define TCL_STATUS_RING_RESERVED_6A_LSB                                             0
#define TCL_STATUS_RING_RESERVED_6A_MSB                                             31
#define TCL_STATUS_RING_RESERVED_6A_MASK                                            0xffffffff


/* Description		RESERVED_7A

			<legal 0>
*/

#define TCL_STATUS_RING_RESERVED_7A_OFFSET                                          0x0000001c
#define TCL_STATUS_RING_RESERVED_7A_LSB                                             0
#define TCL_STATUS_RING_RESERVED_7A_MSB                                             19
#define TCL_STATUS_RING_RESERVED_7A_MASK                                            0x000fffff


/* Description		RING_ID

			The buffer pointer ring ID.
			
			Helps with debugging when dumping ring contents.
			<legal all>
*/

#define TCL_STATUS_RING_RING_ID_OFFSET                                              0x0000001c
#define TCL_STATUS_RING_RING_ID_LSB                                                 20
#define TCL_STATUS_RING_RING_ID_MSB                                                 27
#define TCL_STATUS_RING_RING_ID_MASK                                                0x0ff00000


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

#define TCL_STATUS_RING_LOOPING_COUNT_OFFSET                                        0x0000001c
#define TCL_STATUS_RING_LOOPING_COUNT_LSB                                           28
#define TCL_STATUS_RING_LOOPING_COUNT_MSB                                           31
#define TCL_STATUS_RING_LOOPING_COUNT_MASK                                          0xf0000000



#endif   // TCL_STATUS_RING
