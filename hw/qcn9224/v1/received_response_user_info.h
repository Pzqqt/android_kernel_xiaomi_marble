
/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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










#ifndef _RECEIVED_RESPONSE_USER_INFO_H_
#define _RECEIVED_RESPONSE_USER_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RECEIVED_RESPONSE_USER_INFO 8


struct received_response_user_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t mpdu_fcs_pass_count                                     : 12, // [11:0]
                      mpdu_fcs_fail_count                                     : 12, // [23:12]
                      qosnull_frame_count                                     :  4, // [27:24]
                      reserved_0a                                             :  3, // [30:28]
                      user_info_valid                                         :  1; // [31:31]
             uint32_t null_delimiter_count                                    : 22, // [21:0]
                      reserved_1a                                             :  9, // [30:22]
                      ht_control_valid                                        :  1; // [31:31]
             uint32_t ht_control                                              : 32; // [31:0]
             uint32_t qos_control_valid                                       : 16, // [15:0]
                      eosp                                                    : 16; // [31:16]
             uint32_t qos_control_15_8_tid_0                                  :  8, // [7:0]
                      qos_control_15_8_tid_1                                  :  8, // [15:8]
                      qos_control_15_8_tid_2                                  :  8, // [23:16]
                      qos_control_15_8_tid_3                                  :  8; // [31:24]
             uint32_t qos_control_15_8_tid_4                                  :  8, // [7:0]
                      qos_control_15_8_tid_5                                  :  8, // [15:8]
                      qos_control_15_8_tid_6                                  :  8, // [23:16]
                      qos_control_15_8_tid_7                                  :  8; // [31:24]
             uint32_t qos_control_15_8_tid_8                                  :  8, // [7:0]
                      qos_control_15_8_tid_9                                  :  8, // [15:8]
                      qos_control_15_8_tid_10                                 :  8, // [23:16]
                      qos_control_15_8_tid_11                                 :  8; // [31:24]
             uint32_t qos_control_15_8_tid_12                                 :  8, // [7:0]
                      qos_control_15_8_tid_13                                 :  8, // [15:8]
                      qos_control_15_8_tid_14                                 :  8, // [23:16]
                      qos_control_15_8_tid_15                                 :  8; // [31:24]
#else
             uint32_t user_info_valid                                         :  1, // [31:31]
                      reserved_0a                                             :  3, // [30:28]
                      qosnull_frame_count                                     :  4, // [27:24]
                      mpdu_fcs_fail_count                                     : 12, // [23:12]
                      mpdu_fcs_pass_count                                     : 12; // [11:0]
             uint32_t ht_control_valid                                        :  1, // [31:31]
                      reserved_1a                                             :  9, // [30:22]
                      null_delimiter_count                                    : 22; // [21:0]
             uint32_t ht_control                                              : 32; // [31:0]
             uint32_t eosp                                                    : 16, // [31:16]
                      qos_control_valid                                       : 16; // [15:0]
             uint32_t qos_control_15_8_tid_3                                  :  8, // [31:24]
                      qos_control_15_8_tid_2                                  :  8, // [23:16]
                      qos_control_15_8_tid_1                                  :  8, // [15:8]
                      qos_control_15_8_tid_0                                  :  8; // [7:0]
             uint32_t qos_control_15_8_tid_7                                  :  8, // [31:24]
                      qos_control_15_8_tid_6                                  :  8, // [23:16]
                      qos_control_15_8_tid_5                                  :  8, // [15:8]
                      qos_control_15_8_tid_4                                  :  8; // [7:0]
             uint32_t qos_control_15_8_tid_11                                 :  8, // [31:24]
                      qos_control_15_8_tid_10                                 :  8, // [23:16]
                      qos_control_15_8_tid_9                                  :  8, // [15:8]
                      qos_control_15_8_tid_8                                  :  8; // [7:0]
             uint32_t qos_control_15_8_tid_15                                 :  8, // [31:24]
                      qos_control_15_8_tid_14                                 :  8, // [23:16]
                      qos_control_15_8_tid_13                                 :  8, // [15:8]
                      qos_control_15_8_tid_12                                 :  8; // [7:0]
#endif
};




#define RECEIVED_RESPONSE_USER_INFO_MPDU_FCS_PASS_COUNT_OFFSET                      0x00000000
#define RECEIVED_RESPONSE_USER_INFO_MPDU_FCS_PASS_COUNT_LSB                         0
#define RECEIVED_RESPONSE_USER_INFO_MPDU_FCS_PASS_COUNT_MSB                         11
#define RECEIVED_RESPONSE_USER_INFO_MPDU_FCS_PASS_COUNT_MASK                        0x00000fff




#define RECEIVED_RESPONSE_USER_INFO_MPDU_FCS_FAIL_COUNT_OFFSET                      0x00000000
#define RECEIVED_RESPONSE_USER_INFO_MPDU_FCS_FAIL_COUNT_LSB                         12
#define RECEIVED_RESPONSE_USER_INFO_MPDU_FCS_FAIL_COUNT_MSB                         23
#define RECEIVED_RESPONSE_USER_INFO_MPDU_FCS_FAIL_COUNT_MASK                        0x00fff000




#define RECEIVED_RESPONSE_USER_INFO_QOSNULL_FRAME_COUNT_OFFSET                      0x00000000
#define RECEIVED_RESPONSE_USER_INFO_QOSNULL_FRAME_COUNT_LSB                         24
#define RECEIVED_RESPONSE_USER_INFO_QOSNULL_FRAME_COUNT_MSB                         27
#define RECEIVED_RESPONSE_USER_INFO_QOSNULL_FRAME_COUNT_MASK                        0x0f000000




#define RECEIVED_RESPONSE_USER_INFO_RESERVED_0A_OFFSET                              0x00000000
#define RECEIVED_RESPONSE_USER_INFO_RESERVED_0A_LSB                                 28
#define RECEIVED_RESPONSE_USER_INFO_RESERVED_0A_MSB                                 30
#define RECEIVED_RESPONSE_USER_INFO_RESERVED_0A_MASK                                0x70000000




#define RECEIVED_RESPONSE_USER_INFO_USER_INFO_VALID_OFFSET                          0x00000000
#define RECEIVED_RESPONSE_USER_INFO_USER_INFO_VALID_LSB                             31
#define RECEIVED_RESPONSE_USER_INFO_USER_INFO_VALID_MSB                             31
#define RECEIVED_RESPONSE_USER_INFO_USER_INFO_VALID_MASK                            0x80000000




#define RECEIVED_RESPONSE_USER_INFO_NULL_DELIMITER_COUNT_OFFSET                     0x00000004
#define RECEIVED_RESPONSE_USER_INFO_NULL_DELIMITER_COUNT_LSB                        0
#define RECEIVED_RESPONSE_USER_INFO_NULL_DELIMITER_COUNT_MSB                        21
#define RECEIVED_RESPONSE_USER_INFO_NULL_DELIMITER_COUNT_MASK                       0x003fffff




#define RECEIVED_RESPONSE_USER_INFO_RESERVED_1A_OFFSET                              0x00000004
#define RECEIVED_RESPONSE_USER_INFO_RESERVED_1A_LSB                                 22
#define RECEIVED_RESPONSE_USER_INFO_RESERVED_1A_MSB                                 30
#define RECEIVED_RESPONSE_USER_INFO_RESERVED_1A_MASK                                0x7fc00000




#define RECEIVED_RESPONSE_USER_INFO_HT_CONTROL_VALID_OFFSET                         0x00000004
#define RECEIVED_RESPONSE_USER_INFO_HT_CONTROL_VALID_LSB                            31
#define RECEIVED_RESPONSE_USER_INFO_HT_CONTROL_VALID_MSB                            31
#define RECEIVED_RESPONSE_USER_INFO_HT_CONTROL_VALID_MASK                           0x80000000




#define RECEIVED_RESPONSE_USER_INFO_HT_CONTROL_OFFSET                               0x00000008
#define RECEIVED_RESPONSE_USER_INFO_HT_CONTROL_LSB                                  0
#define RECEIVED_RESPONSE_USER_INFO_HT_CONTROL_MSB                                  31
#define RECEIVED_RESPONSE_USER_INFO_HT_CONTROL_MASK                                 0xffffffff




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_VALID_OFFSET                        0x0000000c
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_VALID_LSB                           0
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_VALID_MSB                           15
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_VALID_MASK                          0x0000ffff




#define RECEIVED_RESPONSE_USER_INFO_EOSP_OFFSET                                     0x0000000c
#define RECEIVED_RESPONSE_USER_INFO_EOSP_LSB                                        16
#define RECEIVED_RESPONSE_USER_INFO_EOSP_MSB                                        31
#define RECEIVED_RESPONSE_USER_INFO_EOSP_MASK                                       0xffff0000




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_0_OFFSET                   0x00000010
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_0_LSB                      0
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_0_MSB                      7
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_0_MASK                     0x000000ff




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_1_OFFSET                   0x00000010
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_1_LSB                      8
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_1_MSB                      15
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_1_MASK                     0x0000ff00




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_2_OFFSET                   0x00000010
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_2_LSB                      16
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_2_MSB                      23
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_2_MASK                     0x00ff0000




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_3_OFFSET                   0x00000010
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_3_LSB                      24
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_3_MSB                      31
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_3_MASK                     0xff000000




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_4_OFFSET                   0x00000014
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_4_LSB                      0
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_4_MSB                      7
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_4_MASK                     0x000000ff




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_5_OFFSET                   0x00000014
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_5_LSB                      8
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_5_MSB                      15
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_5_MASK                     0x0000ff00




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_6_OFFSET                   0x00000014
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_6_LSB                      16
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_6_MSB                      23
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_6_MASK                     0x00ff0000




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_7_OFFSET                   0x00000014
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_7_LSB                      24
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_7_MSB                      31
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_7_MASK                     0xff000000




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_8_OFFSET                   0x00000018
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_8_LSB                      0
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_8_MSB                      7
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_8_MASK                     0x000000ff




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_9_OFFSET                   0x00000018
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_9_LSB                      8
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_9_MSB                      15
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_9_MASK                     0x0000ff00




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_10_OFFSET                  0x00000018
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_10_LSB                     16
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_10_MSB                     23
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_10_MASK                    0x00ff0000




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_11_OFFSET                  0x00000018
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_11_LSB                     24
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_11_MSB                     31
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_11_MASK                    0xff000000




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_12_OFFSET                  0x0000001c
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_12_LSB                     0
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_12_MSB                     7
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_12_MASK                    0x000000ff




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_13_OFFSET                  0x0000001c
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_13_LSB                     8
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_13_MSB                     15
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_13_MASK                    0x0000ff00




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_14_OFFSET                  0x0000001c
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_14_LSB                     16
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_14_MSB                     23
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_14_MASK                    0x00ff0000




#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_15_OFFSET                  0x0000001c
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_15_LSB                     24
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_15_MSB                     31
#define RECEIVED_RESPONSE_USER_INFO_QOS_CONTROL_15_8_TID_15_MASK                    0xff000000




#endif
