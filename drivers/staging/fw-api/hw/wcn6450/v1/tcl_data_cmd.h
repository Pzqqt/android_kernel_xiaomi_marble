
/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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



#ifndef _TCL_DATA_CMD_H_
#define _TCL_DATA_CMD_H_
#if !defined(__ASSEMBLER__)
#endif

#include "buffer_addr_info.h"

#define NUM_OF_DWORDS_TCL_DATA_CMD 7

struct tcl_data_cmd {
    struct            buffer_addr_info                       buf_addr_info;
             uint32_t buf_or_ext_desc_type            :  1,
                      epd                             :  1,
                      encap_type                      :  2,
                      encrypt_type                    :  4,
                      src_buffer_swap                 :  1,
                      link_meta_swap                  :  1,
                      tqm_no_drop                     :  1,
                      reserved_2a                     :  1,
                      search_type                     :  2,
                      addrx_en                        :  1,
                      addry_en                        :  1,
                      tcl_cmd_number                  : 16;
             uint32_t data_length                     : 16,
                      ipv4_checksum_en                :  1,
                      udp_over_ipv4_checksum_en       :  1,
                      udp_over_ipv6_checksum_en       :  1,
                      tcp_over_ipv4_checksum_en       :  1,
                      tcp_over_ipv6_checksum_en       :  1,
                      to_fw                           :  1,
                      reserved_3a                     :  1,
                      packet_offset                   :  9;
             uint32_t buffer_timestamp                : 19,
                      buffer_timestamp_valid          :  1,
                      reserved_4a                     :  1,
                      hlos_tid_overwrite              :  1,
                      hlos_tid                        :  4,
                      lmac_id                         :  2,
                      udp_flow_override               :  2,
                      reserved_4b                     :  2;
             uint32_t dscp_tid_table_num              :  6,
                      search_index                    : 20,
                      cache_set_num                   :  4,
                      mesh_enable                     :  2;
             uint32_t reserved_6a                     : 20,
                      ring_id                         :  8,
                      looping_count                   :  4;
};

#define TCL_DATA_CMD_0_BUF_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET         0x00000000
#define TCL_DATA_CMD_0_BUF_ADDR_INFO_BUFFER_ADDR_31_0_LSB            0
#define TCL_DATA_CMD_0_BUF_ADDR_INFO_BUFFER_ADDR_31_0_MASK           0xffffffff

#define TCL_DATA_CMD_1_BUF_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET        0x00000004
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_BUFFER_ADDR_39_32_LSB           0
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_BUFFER_ADDR_39_32_MASK          0x000000ff

#define TCL_DATA_CMD_1_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET    0x00000004
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB       8
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK      0x00000700

#define TCL_DATA_CMD_1_BUF_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET         0x00000004
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_SW_BUFFER_COOKIE_LSB            11
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_SW_BUFFER_COOKIE_MASK           0xfffff800

#define TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_OFFSET                   0x00000008
#define TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_LSB                      0
#define TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_MASK                     0x00000001

#define TCL_DATA_CMD_2_EPD_OFFSET                                    0x00000008
#define TCL_DATA_CMD_2_EPD_LSB                                       1
#define TCL_DATA_CMD_2_EPD_MASK                                      0x00000002

#define TCL_DATA_CMD_2_ENCAP_TYPE_OFFSET                             0x00000008
#define TCL_DATA_CMD_2_ENCAP_TYPE_LSB                                2
#define TCL_DATA_CMD_2_ENCAP_TYPE_MASK                               0x0000000c

#define TCL_DATA_CMD_2_ENCRYPT_TYPE_OFFSET                           0x00000008
#define TCL_DATA_CMD_2_ENCRYPT_TYPE_LSB                              4
#define TCL_DATA_CMD_2_ENCRYPT_TYPE_MASK                             0x000000f0

#define TCL_DATA_CMD_2_SRC_BUFFER_SWAP_OFFSET                        0x00000008
#define TCL_DATA_CMD_2_SRC_BUFFER_SWAP_LSB                           8
#define TCL_DATA_CMD_2_SRC_BUFFER_SWAP_MASK                          0x00000100

#define TCL_DATA_CMD_2_LINK_META_SWAP_OFFSET                         0x00000008
#define TCL_DATA_CMD_2_LINK_META_SWAP_LSB                            9
#define TCL_DATA_CMD_2_LINK_META_SWAP_MASK                           0x00000200

#define TCL_DATA_CMD_2_TQM_NO_DROP_OFFSET                            0x00000008
#define TCL_DATA_CMD_2_TQM_NO_DROP_LSB                               10
#define TCL_DATA_CMD_2_TQM_NO_DROP_MASK                              0x00000400

#define TCL_DATA_CMD_2_RESERVED_2A_OFFSET                            0x00000008
#define TCL_DATA_CMD_2_RESERVED_2A_LSB                               11
#define TCL_DATA_CMD_2_RESERVED_2A_MASK                              0x00000800

#define TCL_DATA_CMD_2_SEARCH_TYPE_OFFSET                            0x00000008
#define TCL_DATA_CMD_2_SEARCH_TYPE_LSB                               12
#define TCL_DATA_CMD_2_SEARCH_TYPE_MASK                              0x00003000

#define TCL_DATA_CMD_2_ADDRX_EN_OFFSET                               0x00000008
#define TCL_DATA_CMD_2_ADDRX_EN_LSB                                  14
#define TCL_DATA_CMD_2_ADDRX_EN_MASK                                 0x00004000

#define TCL_DATA_CMD_2_ADDRY_EN_OFFSET                               0x00000008
#define TCL_DATA_CMD_2_ADDRY_EN_LSB                                  15
#define TCL_DATA_CMD_2_ADDRY_EN_MASK                                 0x00008000

#define TCL_DATA_CMD_2_TCL_CMD_NUMBER_OFFSET                         0x00000008
#define TCL_DATA_CMD_2_TCL_CMD_NUMBER_LSB                            16
#define TCL_DATA_CMD_2_TCL_CMD_NUMBER_MASK                           0xffff0000

#define TCL_DATA_CMD_3_DATA_LENGTH_OFFSET                            0x0000000c
#define TCL_DATA_CMD_3_DATA_LENGTH_LSB                               0
#define TCL_DATA_CMD_3_DATA_LENGTH_MASK                              0x0000ffff

#define TCL_DATA_CMD_3_IPV4_CHECKSUM_EN_OFFSET                       0x0000000c
#define TCL_DATA_CMD_3_IPV4_CHECKSUM_EN_LSB                          16
#define TCL_DATA_CMD_3_IPV4_CHECKSUM_EN_MASK                         0x00010000

#define TCL_DATA_CMD_3_UDP_OVER_IPV4_CHECKSUM_EN_OFFSET              0x0000000c
#define TCL_DATA_CMD_3_UDP_OVER_IPV4_CHECKSUM_EN_LSB                 17
#define TCL_DATA_CMD_3_UDP_OVER_IPV4_CHECKSUM_EN_MASK                0x00020000

#define TCL_DATA_CMD_3_UDP_OVER_IPV6_CHECKSUM_EN_OFFSET              0x0000000c
#define TCL_DATA_CMD_3_UDP_OVER_IPV6_CHECKSUM_EN_LSB                 18
#define TCL_DATA_CMD_3_UDP_OVER_IPV6_CHECKSUM_EN_MASK                0x00040000

#define TCL_DATA_CMD_3_TCP_OVER_IPV4_CHECKSUM_EN_OFFSET              0x0000000c
#define TCL_DATA_CMD_3_TCP_OVER_IPV4_CHECKSUM_EN_LSB                 19
#define TCL_DATA_CMD_3_TCP_OVER_IPV4_CHECKSUM_EN_MASK                0x00080000

#define TCL_DATA_CMD_3_TCP_OVER_IPV6_CHECKSUM_EN_OFFSET              0x0000000c
#define TCL_DATA_CMD_3_TCP_OVER_IPV6_CHECKSUM_EN_LSB                 20
#define TCL_DATA_CMD_3_TCP_OVER_IPV6_CHECKSUM_EN_MASK                0x00100000

#define TCL_DATA_CMD_3_TO_FW_OFFSET                                  0x0000000c
#define TCL_DATA_CMD_3_TO_FW_LSB                                     21
#define TCL_DATA_CMD_3_TO_FW_MASK                                    0x00200000

#define TCL_DATA_CMD_3_RESERVED_3A_OFFSET                            0x0000000c
#define TCL_DATA_CMD_3_RESERVED_3A_LSB                               22
#define TCL_DATA_CMD_3_RESERVED_3A_MASK                              0x00400000

#define TCL_DATA_CMD_3_PACKET_OFFSET_OFFSET                          0x0000000c
#define TCL_DATA_CMD_3_PACKET_OFFSET_LSB                             23
#define TCL_DATA_CMD_3_PACKET_OFFSET_MASK                            0xff800000

#define TCL_DATA_CMD_4_BUFFER_TIMESTAMP_OFFSET                       0x00000010
#define TCL_DATA_CMD_4_BUFFER_TIMESTAMP_LSB                          0
#define TCL_DATA_CMD_4_BUFFER_TIMESTAMP_MASK                         0x0007ffff

#define TCL_DATA_CMD_4_BUFFER_TIMESTAMP_VALID_OFFSET                 0x00000010
#define TCL_DATA_CMD_4_BUFFER_TIMESTAMP_VALID_LSB                    19
#define TCL_DATA_CMD_4_BUFFER_TIMESTAMP_VALID_MASK                   0x00080000

#define TCL_DATA_CMD_4_RESERVED_4A_OFFSET                            0x00000010
#define TCL_DATA_CMD_4_RESERVED_4A_LSB                               20
#define TCL_DATA_CMD_4_RESERVED_4A_MASK                              0x00100000

#define TCL_DATA_CMD_4_HLOS_TID_OVERWRITE_OFFSET                     0x00000010
#define TCL_DATA_CMD_4_HLOS_TID_OVERWRITE_LSB                        21
#define TCL_DATA_CMD_4_HLOS_TID_OVERWRITE_MASK                       0x00200000

#define TCL_DATA_CMD_4_HLOS_TID_OFFSET                               0x00000010
#define TCL_DATA_CMD_4_HLOS_TID_LSB                                  22
#define TCL_DATA_CMD_4_HLOS_TID_MASK                                 0x03c00000

#define TCL_DATA_CMD_4_LMAC_ID_OFFSET                                0x00000010
#define TCL_DATA_CMD_4_LMAC_ID_LSB                                   26
#define TCL_DATA_CMD_4_LMAC_ID_MASK                                  0x0c000000

#define TCL_DATA_CMD_4_UDP_FLOW_OVERRIDE_OFFSET                      0x00000010
#define TCL_DATA_CMD_4_UDP_FLOW_OVERRIDE_LSB                         28
#define TCL_DATA_CMD_4_UDP_FLOW_OVERRIDE_MASK                        0x30000000

#define TCL_DATA_CMD_4_RESERVED_4B_OFFSET                            0x00000010
#define TCL_DATA_CMD_4_RESERVED_4B_LSB                               30
#define TCL_DATA_CMD_4_RESERVED_4B_MASK                              0xc0000000

#define TCL_DATA_CMD_5_DSCP_TID_TABLE_NUM_OFFSET                     0x00000014
#define TCL_DATA_CMD_5_DSCP_TID_TABLE_NUM_LSB                        0
#define TCL_DATA_CMD_5_DSCP_TID_TABLE_NUM_MASK                       0x0000003f

#define TCL_DATA_CMD_5_SEARCH_INDEX_OFFSET                           0x00000014
#define TCL_DATA_CMD_5_SEARCH_INDEX_LSB                              6
#define TCL_DATA_CMD_5_SEARCH_INDEX_MASK                             0x03ffffc0

#define TCL_DATA_CMD_5_CACHE_SET_NUM_OFFSET                          0x00000014
#define TCL_DATA_CMD_5_CACHE_SET_NUM_LSB                             26
#define TCL_DATA_CMD_5_CACHE_SET_NUM_MASK                            0x3c000000

#define TCL_DATA_CMD_5_MESH_ENABLE_OFFSET                            0x00000014
#define TCL_DATA_CMD_5_MESH_ENABLE_LSB                               30
#define TCL_DATA_CMD_5_MESH_ENABLE_MASK                              0xc0000000

#define TCL_DATA_CMD_6_RESERVED_6A_OFFSET                            0x00000018
#define TCL_DATA_CMD_6_RESERVED_6A_LSB                               0
#define TCL_DATA_CMD_6_RESERVED_6A_MASK                              0x000fffff

#define TCL_DATA_CMD_6_RING_ID_OFFSET                                0x00000018
#define TCL_DATA_CMD_6_RING_ID_LSB                                   20
#define TCL_DATA_CMD_6_RING_ID_MASK                                  0x0ff00000

#define TCL_DATA_CMD_6_LOOPING_COUNT_OFFSET                          0x00000018
#define TCL_DATA_CMD_6_LOOPING_COUNT_LSB                             28
#define TCL_DATA_CMD_6_LOOPING_COUNT_MASK                            0xf0000000

#endif
