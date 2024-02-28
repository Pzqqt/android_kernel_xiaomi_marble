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


#ifndef _TCL_DATA_CMD_H_
#define _TCL_DATA_CMD_H_

#include "buffer_addr_info.h"
#define NUM_OF_DWORDS_TCL_DATA_CMD 8

struct tcl_data_cmd {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   buffer_addr_info                                          buf_addr_info;
             uint32_t tcl_cmd_type                                            :  1,
                      buf_or_ext_desc_type                                    :  1,
                      bank_id                                                 :  6,
                      tx_notify_frame                                         :  3,
                      header_length_read_sel                                  :  1,
                      buffer_timestamp                                        : 19,
                      buffer_timestamp_valid                                  :  1;
             uint32_t reserved_3a                                             : 16,
                      tcl_cmd_number                                          : 16;
             uint32_t data_length                                             : 16,
                      ipv4_checksum_en                                        :  1,
                      udp_over_ipv4_checksum_en                               :  1,
                      udp_over_ipv6_checksum_en                               :  1,
                      tcp_over_ipv4_checksum_en                               :  1,
                      tcp_over_ipv6_checksum_en                               :  1,
                      to_fw                                                   :  1,
                      reserved_4a                                             :  1,
                      packet_offset                                           :  9;
             uint32_t hlos_tid_overwrite                                      :  1,
                      flow_override_enable                                    :  1,
                      who_classify_info_sel                                   :  2,
                      hlos_tid                                                :  4,
                      flow_override                                           :  1,
                      pmac_id                                                 :  2,
                      msdu_color                                              :  2,
                      reserved_5a                                             : 11,
                      vdev_id                                                 :  8;
             uint32_t search_index                                            : 20,
                      cache_set_num                                           :  4,
                      index_lookup_override                                   :  1,
                      reserved_6a                                             :  7;
             uint32_t reserved_7a                                             : 20,
                      ring_id                                                 :  8,
                      looping_count                                           :  4;
#else
             struct   buffer_addr_info                                          buf_addr_info;
             uint32_t buffer_timestamp_valid                                  :  1,
                      buffer_timestamp                                        : 19,
                      header_length_read_sel                                  :  1,
                      tx_notify_frame                                         :  3,
                      bank_id                                                 :  6,
                      buf_or_ext_desc_type                                    :  1,
                      tcl_cmd_type                                            :  1;
             uint32_t tcl_cmd_number                                          : 16,
                      reserved_3a                                             : 16;
             uint32_t packet_offset                                           :  9,
                      reserved_4a                                             :  1,
                      to_fw                                                   :  1,
                      tcp_over_ipv6_checksum_en                               :  1,
                      tcp_over_ipv4_checksum_en                               :  1,
                      udp_over_ipv6_checksum_en                               :  1,
                      udp_over_ipv4_checksum_en                               :  1,
                      ipv4_checksum_en                                        :  1,
                      data_length                                             : 16;
             uint32_t vdev_id                                                 :  8,
                      reserved_5a                                             : 11,
                      msdu_color                                              :  2,
                      pmac_id                                                 :  2,
                      flow_override                                           :  1,
                      hlos_tid                                                :  4,
                      who_classify_info_sel                                   :  2,
                      flow_override_enable                                    :  1,
                      hlos_tid_overwrite                                      :  1;
             uint32_t reserved_6a                                             :  7,
                      index_lookup_override                                   :  1,
                      cache_set_num                                           :  4,
                      search_index                                            : 20;
             uint32_t looping_count                                           :  4,
                      ring_id                                                 :  8,
                      reserved_7a                                             : 20;
#endif
};

#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET                          0x00000000
#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_31_0_LSB                             0
#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_31_0_MSB                             31
#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_31_0_MASK                            0xffffffff

#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET                         0x00000004
#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_39_32_LSB                            0
#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_39_32_MSB                            7
#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_39_32_MASK                           0x000000ff

#define TCL_DATA_CMD_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET                     0x00000004
#define TCL_DATA_CMD_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB                        8
#define TCL_DATA_CMD_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB                        11
#define TCL_DATA_CMD_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK                       0x00000f00

#define TCL_DATA_CMD_BUF_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET                          0x00000004
#define TCL_DATA_CMD_BUF_ADDR_INFO_SW_BUFFER_COOKIE_LSB                             12
#define TCL_DATA_CMD_BUF_ADDR_INFO_SW_BUFFER_COOKIE_MSB                             31
#define TCL_DATA_CMD_BUF_ADDR_INFO_SW_BUFFER_COOKIE_MASK                            0xfffff000

#define TCL_DATA_CMD_TCL_CMD_TYPE_OFFSET                                            0x00000008
#define TCL_DATA_CMD_TCL_CMD_TYPE_LSB                                               0
#define TCL_DATA_CMD_TCL_CMD_TYPE_MSB                                               0
#define TCL_DATA_CMD_TCL_CMD_TYPE_MASK                                              0x00000001

#define TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_OFFSET                                    0x00000008
#define TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_LSB                                       1
#define TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_MSB                                       1
#define TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_MASK                                      0x00000002

#define TCL_DATA_CMD_BANK_ID_OFFSET                                                 0x00000008
#define TCL_DATA_CMD_BANK_ID_LSB                                                    2
#define TCL_DATA_CMD_BANK_ID_MSB                                                    7
#define TCL_DATA_CMD_BANK_ID_MASK                                                   0x000000fc

#define TCL_DATA_CMD_TX_NOTIFY_FRAME_OFFSET                                         0x00000008
#define TCL_DATA_CMD_TX_NOTIFY_FRAME_LSB                                            8
#define TCL_DATA_CMD_TX_NOTIFY_FRAME_MSB                                            10
#define TCL_DATA_CMD_TX_NOTIFY_FRAME_MASK                                           0x00000700

#define TCL_DATA_CMD_HEADER_LENGTH_READ_SEL_OFFSET                                  0x00000008
#define TCL_DATA_CMD_HEADER_LENGTH_READ_SEL_LSB                                     11
#define TCL_DATA_CMD_HEADER_LENGTH_READ_SEL_MSB                                     11
#define TCL_DATA_CMD_HEADER_LENGTH_READ_SEL_MASK                                    0x00000800

#define TCL_DATA_CMD_BUFFER_TIMESTAMP_OFFSET                                        0x00000008
#define TCL_DATA_CMD_BUFFER_TIMESTAMP_LSB                                           12
#define TCL_DATA_CMD_BUFFER_TIMESTAMP_MSB                                           30
#define TCL_DATA_CMD_BUFFER_TIMESTAMP_MASK                                          0x7ffff000

#define TCL_DATA_CMD_BUFFER_TIMESTAMP_VALID_OFFSET                                  0x00000008
#define TCL_DATA_CMD_BUFFER_TIMESTAMP_VALID_LSB                                     31
#define TCL_DATA_CMD_BUFFER_TIMESTAMP_VALID_MSB                                     31
#define TCL_DATA_CMD_BUFFER_TIMESTAMP_VALID_MASK                                    0x80000000

#define TCL_DATA_CMD_RESERVED_3A_OFFSET                                             0x0000000c
#define TCL_DATA_CMD_RESERVED_3A_LSB                                                0
#define TCL_DATA_CMD_RESERVED_3A_MSB                                                15
#define TCL_DATA_CMD_RESERVED_3A_MASK                                               0x0000ffff

#define TCL_DATA_CMD_TCL_CMD_NUMBER_OFFSET                                          0x0000000c
#define TCL_DATA_CMD_TCL_CMD_NUMBER_LSB                                             16
#define TCL_DATA_CMD_TCL_CMD_NUMBER_MSB                                             31
#define TCL_DATA_CMD_TCL_CMD_NUMBER_MASK                                            0xffff0000

#define TCL_DATA_CMD_DATA_LENGTH_OFFSET                                             0x00000010
#define TCL_DATA_CMD_DATA_LENGTH_LSB                                                0
#define TCL_DATA_CMD_DATA_LENGTH_MSB                                                15
#define TCL_DATA_CMD_DATA_LENGTH_MASK                                               0x0000ffff

#define TCL_DATA_CMD_IPV4_CHECKSUM_EN_OFFSET                                        0x00000010
#define TCL_DATA_CMD_IPV4_CHECKSUM_EN_LSB                                           16
#define TCL_DATA_CMD_IPV4_CHECKSUM_EN_MSB                                           16
#define TCL_DATA_CMD_IPV4_CHECKSUM_EN_MASK                                          0x00010000

#define TCL_DATA_CMD_UDP_OVER_IPV4_CHECKSUM_EN_OFFSET                               0x00000010
#define TCL_DATA_CMD_UDP_OVER_IPV4_CHECKSUM_EN_LSB                                  17
#define TCL_DATA_CMD_UDP_OVER_IPV4_CHECKSUM_EN_MSB                                  17
#define TCL_DATA_CMD_UDP_OVER_IPV4_CHECKSUM_EN_MASK                                 0x00020000

#define TCL_DATA_CMD_UDP_OVER_IPV6_CHECKSUM_EN_OFFSET                               0x00000010
#define TCL_DATA_CMD_UDP_OVER_IPV6_CHECKSUM_EN_LSB                                  18
#define TCL_DATA_CMD_UDP_OVER_IPV6_CHECKSUM_EN_MSB                                  18
#define TCL_DATA_CMD_UDP_OVER_IPV6_CHECKSUM_EN_MASK                                 0x00040000

#define TCL_DATA_CMD_TCP_OVER_IPV4_CHECKSUM_EN_OFFSET                               0x00000010
#define TCL_DATA_CMD_TCP_OVER_IPV4_CHECKSUM_EN_LSB                                  19
#define TCL_DATA_CMD_TCP_OVER_IPV4_CHECKSUM_EN_MSB                                  19
#define TCL_DATA_CMD_TCP_OVER_IPV4_CHECKSUM_EN_MASK                                 0x00080000

#define TCL_DATA_CMD_TCP_OVER_IPV6_CHECKSUM_EN_OFFSET                               0x00000010
#define TCL_DATA_CMD_TCP_OVER_IPV6_CHECKSUM_EN_LSB                                  20
#define TCL_DATA_CMD_TCP_OVER_IPV6_CHECKSUM_EN_MSB                                  20
#define TCL_DATA_CMD_TCP_OVER_IPV6_CHECKSUM_EN_MASK                                 0x00100000

#define TCL_DATA_CMD_TO_FW_OFFSET                                                   0x00000010
#define TCL_DATA_CMD_TO_FW_LSB                                                      21
#define TCL_DATA_CMD_TO_FW_MSB                                                      21
#define TCL_DATA_CMD_TO_FW_MASK                                                     0x00200000

#define TCL_DATA_CMD_RESERVED_4A_OFFSET                                             0x00000010
#define TCL_DATA_CMD_RESERVED_4A_LSB                                                22
#define TCL_DATA_CMD_RESERVED_4A_MSB                                                22
#define TCL_DATA_CMD_RESERVED_4A_MASK                                               0x00400000

#define TCL_DATA_CMD_PACKET_OFFSET_OFFSET                                           0x00000010
#define TCL_DATA_CMD_PACKET_OFFSET_LSB                                              23
#define TCL_DATA_CMD_PACKET_OFFSET_MSB                                              31
#define TCL_DATA_CMD_PACKET_OFFSET_MASK                                             0xff800000

#define TCL_DATA_CMD_HLOS_TID_OVERWRITE_OFFSET                                      0x00000014
#define TCL_DATA_CMD_HLOS_TID_OVERWRITE_LSB                                         0
#define TCL_DATA_CMD_HLOS_TID_OVERWRITE_MSB                                         0
#define TCL_DATA_CMD_HLOS_TID_OVERWRITE_MASK                                        0x00000001

#define TCL_DATA_CMD_FLOW_OVERRIDE_ENABLE_OFFSET                                    0x00000014
#define TCL_DATA_CMD_FLOW_OVERRIDE_ENABLE_LSB                                       1
#define TCL_DATA_CMD_FLOW_OVERRIDE_ENABLE_MSB                                       1
#define TCL_DATA_CMD_FLOW_OVERRIDE_ENABLE_MASK                                      0x00000002

#define TCL_DATA_CMD_WHO_CLASSIFY_INFO_SEL_OFFSET                                   0x00000014
#define TCL_DATA_CMD_WHO_CLASSIFY_INFO_SEL_LSB                                      2
#define TCL_DATA_CMD_WHO_CLASSIFY_INFO_SEL_MSB                                      3
#define TCL_DATA_CMD_WHO_CLASSIFY_INFO_SEL_MASK                                     0x0000000c

#define TCL_DATA_CMD_HLOS_TID_OFFSET                                                0x00000014
#define TCL_DATA_CMD_HLOS_TID_LSB                                                   4
#define TCL_DATA_CMD_HLOS_TID_MSB                                                   7
#define TCL_DATA_CMD_HLOS_TID_MASK                                                  0x000000f0

#define TCL_DATA_CMD_FLOW_OVERRIDE_OFFSET                                           0x00000014
#define TCL_DATA_CMD_FLOW_OVERRIDE_LSB                                              8
#define TCL_DATA_CMD_FLOW_OVERRIDE_MSB                                              8
#define TCL_DATA_CMD_FLOW_OVERRIDE_MASK                                             0x00000100

#define TCL_DATA_CMD_PMAC_ID_OFFSET                                                 0x00000014
#define TCL_DATA_CMD_PMAC_ID_LSB                                                    9
#define TCL_DATA_CMD_PMAC_ID_MSB                                                    10
#define TCL_DATA_CMD_PMAC_ID_MASK                                                   0x00000600

#define TCL_DATA_CMD_MSDU_COLOR_OFFSET                                              0x00000014
#define TCL_DATA_CMD_MSDU_COLOR_LSB                                                 11
#define TCL_DATA_CMD_MSDU_COLOR_MSB                                                 12
#define TCL_DATA_CMD_MSDU_COLOR_MASK                                                0x00001800

#define TCL_DATA_CMD_RESERVED_5A_OFFSET                                             0x00000014
#define TCL_DATA_CMD_RESERVED_5A_LSB                                                13
#define TCL_DATA_CMD_RESERVED_5A_MSB                                                23
#define TCL_DATA_CMD_RESERVED_5A_MASK                                               0x00ffe000

#define TCL_DATA_CMD_VDEV_ID_OFFSET                                                 0x00000014
#define TCL_DATA_CMD_VDEV_ID_LSB                                                    24
#define TCL_DATA_CMD_VDEV_ID_MSB                                                    31
#define TCL_DATA_CMD_VDEV_ID_MASK                                                   0xff000000

#define TCL_DATA_CMD_SEARCH_INDEX_OFFSET                                            0x00000018
#define TCL_DATA_CMD_SEARCH_INDEX_LSB                                               0
#define TCL_DATA_CMD_SEARCH_INDEX_MSB                                               19
#define TCL_DATA_CMD_SEARCH_INDEX_MASK                                              0x000fffff

#define TCL_DATA_CMD_CACHE_SET_NUM_OFFSET                                           0x00000018
#define TCL_DATA_CMD_CACHE_SET_NUM_LSB                                              20
#define TCL_DATA_CMD_CACHE_SET_NUM_MSB                                              23
#define TCL_DATA_CMD_CACHE_SET_NUM_MASK                                             0x00f00000

#define TCL_DATA_CMD_INDEX_LOOKUP_OVERRIDE_OFFSET                                   0x00000018
#define TCL_DATA_CMD_INDEX_LOOKUP_OVERRIDE_LSB                                      24
#define TCL_DATA_CMD_INDEX_LOOKUP_OVERRIDE_MSB                                      24
#define TCL_DATA_CMD_INDEX_LOOKUP_OVERRIDE_MASK                                     0x01000000

#define TCL_DATA_CMD_RESERVED_6A_OFFSET                                             0x00000018
#define TCL_DATA_CMD_RESERVED_6A_LSB                                                25
#define TCL_DATA_CMD_RESERVED_6A_MSB                                                31
#define TCL_DATA_CMD_RESERVED_6A_MASK                                               0xfe000000

#define TCL_DATA_CMD_RESERVED_7A_OFFSET                                             0x0000001c
#define TCL_DATA_CMD_RESERVED_7A_LSB                                                0
#define TCL_DATA_CMD_RESERVED_7A_MSB                                                19
#define TCL_DATA_CMD_RESERVED_7A_MASK                                               0x000fffff

#define TCL_DATA_CMD_RING_ID_OFFSET                                                 0x0000001c
#define TCL_DATA_CMD_RING_ID_LSB                                                    20
#define TCL_DATA_CMD_RING_ID_MSB                                                    27
#define TCL_DATA_CMD_RING_ID_MASK                                                   0x0ff00000

#define TCL_DATA_CMD_LOOPING_COUNT_OFFSET                                           0x0000001c
#define TCL_DATA_CMD_LOOPING_COUNT_LSB                                              28
#define TCL_DATA_CMD_LOOPING_COUNT_MSB                                              31
#define TCL_DATA_CMD_LOOPING_COUNT_MASK                                             0xf0000000

#endif
