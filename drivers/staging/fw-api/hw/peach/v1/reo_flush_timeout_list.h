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


#ifndef _REO_FLUSH_TIMEOUT_LIST_H_
#define _REO_FLUSH_TIMEOUT_LIST_H_

#include "uniform_reo_cmd_header.h"
#define NUM_OF_DWORDS_REO_FLUSH_TIMEOUT_LIST 9

struct reo_flush_timeout_list {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t ac_timout_list                                          :  2,
                      reserved_1                                              : 30;
             uint32_t minimum_release_desc_count                              : 16,
                      minimum_forward_buf_count                               : 16;
             uint32_t reserved_3a                                             : 32;
             uint32_t reserved_4a                                             : 32;
             uint32_t reserved_5a                                             : 32;
             uint32_t reserved_6a                                             : 32;
             uint32_t reserved_7a                                             : 32;
             uint32_t reserved_8a                                             : 32;
#else
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t reserved_1                                              : 30,
                      ac_timout_list                                          :  2;
             uint32_t minimum_forward_buf_count                               : 16,
                      minimum_release_desc_count                              : 16;
             uint32_t reserved_3a                                             : 32;
             uint32_t reserved_4a                                             : 32;
             uint32_t reserved_5a                                             : 32;
             uint32_t reserved_6a                                             : 32;
             uint32_t reserved_7a                                             : 32;
             uint32_t reserved_8a                                             : 32;
#endif
};

#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_CMD_NUMBER_OFFSET                     0x00000000
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_CMD_NUMBER_LSB                        0
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_CMD_NUMBER_MSB                        15
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_CMD_NUMBER_MASK                       0x0000ffff

#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET                0x00000000
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_STATUS_REQUIRED_LSB                   16
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_STATUS_REQUIRED_MSB                   16
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_REO_STATUS_REQUIRED_MASK                  0x00010000

#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_RESERVED_0A_OFFSET                        0x00000000
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_RESERVED_0A_LSB                           17
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_RESERVED_0A_MSB                           31
#define REO_FLUSH_TIMEOUT_LIST_CMD_HEADER_RESERVED_0A_MASK                          0xfffe0000

#define REO_FLUSH_TIMEOUT_LIST_AC_TIMOUT_LIST_OFFSET                                0x00000004
#define REO_FLUSH_TIMEOUT_LIST_AC_TIMOUT_LIST_LSB                                   0
#define REO_FLUSH_TIMEOUT_LIST_AC_TIMOUT_LIST_MSB                                   1
#define REO_FLUSH_TIMEOUT_LIST_AC_TIMOUT_LIST_MASK                                  0x00000003

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_1_OFFSET                                    0x00000004
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_1_LSB                                       2
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_1_MSB                                       31
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_1_MASK                                      0xfffffffc

#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_RELEASE_DESC_COUNT_OFFSET                    0x00000008
#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_RELEASE_DESC_COUNT_LSB                       0
#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_RELEASE_DESC_COUNT_MSB                       15
#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_RELEASE_DESC_COUNT_MASK                      0x0000ffff

#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_FORWARD_BUF_COUNT_OFFSET                     0x00000008
#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_FORWARD_BUF_COUNT_LSB                        16
#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_FORWARD_BUF_COUNT_MSB                        31
#define REO_FLUSH_TIMEOUT_LIST_MINIMUM_FORWARD_BUF_COUNT_MASK                       0xffff0000

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_3A_OFFSET                                   0x0000000c
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_3A_LSB                                      0
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_3A_MSB                                      31
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_3A_MASK                                     0xffffffff

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_4A_OFFSET                                   0x00000010
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_4A_LSB                                      0
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_4A_MSB                                      31
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_4A_MASK                                     0xffffffff

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_5A_OFFSET                                   0x00000014
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_5A_LSB                                      0
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_5A_MSB                                      31
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_5A_MASK                                     0xffffffff

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_6A_OFFSET                                   0x00000018
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_6A_LSB                                      0
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_6A_MSB                                      31
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_6A_MASK                                     0xffffffff

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_7A_OFFSET                                   0x0000001c
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_7A_LSB                                      0
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_7A_MSB                                      31
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_7A_MASK                                     0xffffffff

#define REO_FLUSH_TIMEOUT_LIST_RESERVED_8A_OFFSET                                   0x00000020
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_8A_LSB                                      0
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_8A_MSB                                      31
#define REO_FLUSH_TIMEOUT_LIST_RESERVED_8A_MASK                                     0xffffffff

#endif
