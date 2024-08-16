/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#define NUM_OF_DWORDS_TCL_GSE_CMD 8

struct tcl_gse_cmd {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t control_buffer_addr_31_0                                : 32;
             uint32_t control_buffer_addr_39_32                               :  8,
                      gse_ctrl                                                :  4,
                      gse_sel                                                 :  1,
                      status_destination_ring_id                              :  1,
                      swap                                                    :  1,
                      index_search_en                                         :  1,
                      cache_set_num                                           :  4,
                      reserved_1a                                             : 12;
             uint32_t tcl_cmd_type                                            :  1,
                      reserved_2a                                             : 31;
             uint32_t cmd_meta_data_31_0                                      : 32;
             uint32_t cmd_meta_data_63_32                                     : 32;
             uint32_t reserved_5a                                             : 32;
             uint32_t reserved_6a                                             : 32;
             uint32_t reserved_7a                                             : 20,
                      ring_id                                                 :  8,
                      looping_count                                           :  4;
#else
             uint32_t control_buffer_addr_31_0                                : 32;
             uint32_t reserved_1a                                             : 12,
                      cache_set_num                                           :  4,
                      index_search_en                                         :  1,
                      swap                                                    :  1,
                      status_destination_ring_id                              :  1,
                      gse_sel                                                 :  1,
                      gse_ctrl                                                :  4,
                      control_buffer_addr_39_32                               :  8;
             uint32_t reserved_2a                                             : 31,
                      tcl_cmd_type                                            :  1;
             uint32_t cmd_meta_data_31_0                                      : 32;
             uint32_t cmd_meta_data_63_32                                     : 32;
             uint32_t reserved_5a                                             : 32;
             uint32_t reserved_6a                                             : 32;
             uint32_t looping_count                                           :  4,
                      ring_id                                                 :  8,
                      reserved_7a                                             : 20;
#endif
};

#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_31_0_OFFSET                                 0x00000000
#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_31_0_LSB                                    0
#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_31_0_MSB                                    31
#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_31_0_MASK                                   0xffffffff

#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_39_32_OFFSET                                0x00000004
#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_39_32_LSB                                   0
#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_39_32_MSB                                   7
#define TCL_GSE_CMD_CONTROL_BUFFER_ADDR_39_32_MASK                                  0x000000ff

#define TCL_GSE_CMD_GSE_CTRL_OFFSET                                                 0x00000004
#define TCL_GSE_CMD_GSE_CTRL_LSB                                                    8
#define TCL_GSE_CMD_GSE_CTRL_MSB                                                    11
#define TCL_GSE_CMD_GSE_CTRL_MASK                                                   0x00000f00

#define TCL_GSE_CMD_GSE_SEL_OFFSET                                                  0x00000004
#define TCL_GSE_CMD_GSE_SEL_LSB                                                     12
#define TCL_GSE_CMD_GSE_SEL_MSB                                                     12
#define TCL_GSE_CMD_GSE_SEL_MASK                                                    0x00001000

#define TCL_GSE_CMD_STATUS_DESTINATION_RING_ID_OFFSET                               0x00000004
#define TCL_GSE_CMD_STATUS_DESTINATION_RING_ID_LSB                                  13
#define TCL_GSE_CMD_STATUS_DESTINATION_RING_ID_MSB                                  13
#define TCL_GSE_CMD_STATUS_DESTINATION_RING_ID_MASK                                 0x00002000

#define TCL_GSE_CMD_SWAP_OFFSET                                                     0x00000004
#define TCL_GSE_CMD_SWAP_LSB                                                        14
#define TCL_GSE_CMD_SWAP_MSB                                                        14
#define TCL_GSE_CMD_SWAP_MASK                                                       0x00004000

#define TCL_GSE_CMD_INDEX_SEARCH_EN_OFFSET                                          0x00000004
#define TCL_GSE_CMD_INDEX_SEARCH_EN_LSB                                             15
#define TCL_GSE_CMD_INDEX_SEARCH_EN_MSB                                             15
#define TCL_GSE_CMD_INDEX_SEARCH_EN_MASK                                            0x00008000

#define TCL_GSE_CMD_CACHE_SET_NUM_OFFSET                                            0x00000004
#define TCL_GSE_CMD_CACHE_SET_NUM_LSB                                               16
#define TCL_GSE_CMD_CACHE_SET_NUM_MSB                                               19
#define TCL_GSE_CMD_CACHE_SET_NUM_MASK                                              0x000f0000

#define TCL_GSE_CMD_RESERVED_1A_OFFSET                                              0x00000004
#define TCL_GSE_CMD_RESERVED_1A_LSB                                                 20
#define TCL_GSE_CMD_RESERVED_1A_MSB                                                 31
#define TCL_GSE_CMD_RESERVED_1A_MASK                                                0xfff00000

#define TCL_GSE_CMD_TCL_CMD_TYPE_OFFSET                                             0x00000008
#define TCL_GSE_CMD_TCL_CMD_TYPE_LSB                                                0
#define TCL_GSE_CMD_TCL_CMD_TYPE_MSB                                                0
#define TCL_GSE_CMD_TCL_CMD_TYPE_MASK                                               0x00000001

#define TCL_GSE_CMD_RESERVED_2A_OFFSET                                              0x00000008
#define TCL_GSE_CMD_RESERVED_2A_LSB                                                 1
#define TCL_GSE_CMD_RESERVED_2A_MSB                                                 31
#define TCL_GSE_CMD_RESERVED_2A_MASK                                                0xfffffffe

#define TCL_GSE_CMD_CMD_META_DATA_31_0_OFFSET                                       0x0000000c
#define TCL_GSE_CMD_CMD_META_DATA_31_0_LSB                                          0
#define TCL_GSE_CMD_CMD_META_DATA_31_0_MSB                                          31
#define TCL_GSE_CMD_CMD_META_DATA_31_0_MASK                                         0xffffffff

#define TCL_GSE_CMD_CMD_META_DATA_63_32_OFFSET                                      0x00000010
#define TCL_GSE_CMD_CMD_META_DATA_63_32_LSB                                         0
#define TCL_GSE_CMD_CMD_META_DATA_63_32_MSB                                         31
#define TCL_GSE_CMD_CMD_META_DATA_63_32_MASK                                        0xffffffff

#define TCL_GSE_CMD_RESERVED_5A_OFFSET                                              0x00000014
#define TCL_GSE_CMD_RESERVED_5A_LSB                                                 0
#define TCL_GSE_CMD_RESERVED_5A_MSB                                                 31
#define TCL_GSE_CMD_RESERVED_5A_MASK                                                0xffffffff

#define TCL_GSE_CMD_RESERVED_6A_OFFSET                                              0x00000018
#define TCL_GSE_CMD_RESERVED_6A_LSB                                                 0
#define TCL_GSE_CMD_RESERVED_6A_MSB                                                 31
#define TCL_GSE_CMD_RESERVED_6A_MASK                                                0xffffffff

#define TCL_GSE_CMD_RESERVED_7A_OFFSET                                              0x0000001c
#define TCL_GSE_CMD_RESERVED_7A_LSB                                                 0
#define TCL_GSE_CMD_RESERVED_7A_MSB                                                 19
#define TCL_GSE_CMD_RESERVED_7A_MASK                                                0x000fffff

#define TCL_GSE_CMD_RING_ID_OFFSET                                                  0x0000001c
#define TCL_GSE_CMD_RING_ID_LSB                                                     20
#define TCL_GSE_CMD_RING_ID_MSB                                                     27
#define TCL_GSE_CMD_RING_ID_MASK                                                    0x0ff00000

#define TCL_GSE_CMD_LOOPING_COUNT_OFFSET                                            0x0000001c
#define TCL_GSE_CMD_LOOPING_COUNT_LSB                                               28
#define TCL_GSE_CMD_LOOPING_COUNT_MSB                                               31
#define TCL_GSE_CMD_LOOPING_COUNT_MASK                                              0xf0000000

#endif
