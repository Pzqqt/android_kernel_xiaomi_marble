/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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


#ifndef _RX_MPDU_DESC_INFO_H_
#define _RX_MPDU_DESC_INFO_H_

#define NUM_OF_DWORDS_RX_MPDU_DESC_INFO 2

struct rx_mpdu_desc_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t msdu_count                                              :  8,
                      fragment_flag                                           :  1,
                      mpdu_retry_bit                                          :  1,
                      ampdu_flag                                              :  1,
                      bar_frame                                               :  1,
                      pn_fields_contain_valid_info                            :  1,
                      raw_mpdu                                                :  1,
                      more_fragment_flag                                      :  1,
                      src_info                                                : 12,
                      mpdu_qos_control_valid                                  :  1,
                      tid                                                     :  4;
             uint32_t peer_meta_data                                          : 32;
#else
             uint32_t tid                                                     :  4,
                      mpdu_qos_control_valid                                  :  1,
                      src_info                                                : 12,
                      more_fragment_flag                                      :  1,
                      raw_mpdu                                                :  1,
                      pn_fields_contain_valid_info                            :  1,
                      bar_frame                                               :  1,
                      ampdu_flag                                              :  1,
                      mpdu_retry_bit                                          :  1,
                      fragment_flag                                           :  1,
                      msdu_count                                              :  8;
             uint32_t peer_meta_data                                          : 32;
#endif
};

#define RX_MPDU_DESC_INFO_MSDU_COUNT_OFFSET                                         0x00000000
#define RX_MPDU_DESC_INFO_MSDU_COUNT_LSB                                            0
#define RX_MPDU_DESC_INFO_MSDU_COUNT_MSB                                            7
#define RX_MPDU_DESC_INFO_MSDU_COUNT_MASK                                           0x000000ff

#define RX_MPDU_DESC_INFO_FRAGMENT_FLAG_OFFSET                                      0x00000000
#define RX_MPDU_DESC_INFO_FRAGMENT_FLAG_LSB                                         8
#define RX_MPDU_DESC_INFO_FRAGMENT_FLAG_MSB                                         8
#define RX_MPDU_DESC_INFO_FRAGMENT_FLAG_MASK                                        0x00000100

#define RX_MPDU_DESC_INFO_MPDU_RETRY_BIT_OFFSET                                     0x00000000
#define RX_MPDU_DESC_INFO_MPDU_RETRY_BIT_LSB                                        9
#define RX_MPDU_DESC_INFO_MPDU_RETRY_BIT_MSB                                        9
#define RX_MPDU_DESC_INFO_MPDU_RETRY_BIT_MASK                                       0x00000200

#define RX_MPDU_DESC_INFO_AMPDU_FLAG_OFFSET                                         0x00000000
#define RX_MPDU_DESC_INFO_AMPDU_FLAG_LSB                                            10
#define RX_MPDU_DESC_INFO_AMPDU_FLAG_MSB                                            10
#define RX_MPDU_DESC_INFO_AMPDU_FLAG_MASK                                           0x00000400

#define RX_MPDU_DESC_INFO_BAR_FRAME_OFFSET                                          0x00000000
#define RX_MPDU_DESC_INFO_BAR_FRAME_LSB                                             11
#define RX_MPDU_DESC_INFO_BAR_FRAME_MSB                                             11
#define RX_MPDU_DESC_INFO_BAR_FRAME_MASK                                            0x00000800

#define RX_MPDU_DESC_INFO_PN_FIELDS_CONTAIN_VALID_INFO_OFFSET                       0x00000000
#define RX_MPDU_DESC_INFO_PN_FIELDS_CONTAIN_VALID_INFO_LSB                          12
#define RX_MPDU_DESC_INFO_PN_FIELDS_CONTAIN_VALID_INFO_MSB                          12
#define RX_MPDU_DESC_INFO_PN_FIELDS_CONTAIN_VALID_INFO_MASK                         0x00001000

#define RX_MPDU_DESC_INFO_RAW_MPDU_OFFSET                                           0x00000000
#define RX_MPDU_DESC_INFO_RAW_MPDU_LSB                                              13
#define RX_MPDU_DESC_INFO_RAW_MPDU_MSB                                              13
#define RX_MPDU_DESC_INFO_RAW_MPDU_MASK                                             0x00002000

#define RX_MPDU_DESC_INFO_MORE_FRAGMENT_FLAG_OFFSET                                 0x00000000
#define RX_MPDU_DESC_INFO_MORE_FRAGMENT_FLAG_LSB                                    14
#define RX_MPDU_DESC_INFO_MORE_FRAGMENT_FLAG_MSB                                    14
#define RX_MPDU_DESC_INFO_MORE_FRAGMENT_FLAG_MASK                                   0x00004000

#define RX_MPDU_DESC_INFO_SRC_INFO_OFFSET                                           0x00000000
#define RX_MPDU_DESC_INFO_SRC_INFO_LSB                                              15
#define RX_MPDU_DESC_INFO_SRC_INFO_MSB                                              26
#define RX_MPDU_DESC_INFO_SRC_INFO_MASK                                             0x07ff8000

#define RX_MPDU_DESC_INFO_MPDU_QOS_CONTROL_VALID_OFFSET                             0x00000000
#define RX_MPDU_DESC_INFO_MPDU_QOS_CONTROL_VALID_LSB                                27
#define RX_MPDU_DESC_INFO_MPDU_QOS_CONTROL_VALID_MSB                                27
#define RX_MPDU_DESC_INFO_MPDU_QOS_CONTROL_VALID_MASK                               0x08000000

#define RX_MPDU_DESC_INFO_TID_OFFSET                                                0x00000000
#define RX_MPDU_DESC_INFO_TID_LSB                                                   28
#define RX_MPDU_DESC_INFO_TID_MSB                                                   31
#define RX_MPDU_DESC_INFO_TID_MASK                                                  0xf0000000

#define RX_MPDU_DESC_INFO_PEER_META_DATA_OFFSET                                     0x00000004
#define RX_MPDU_DESC_INFO_PEER_META_DATA_LSB                                        0
#define RX_MPDU_DESC_INFO_PEER_META_DATA_MSB                                        31
#define RX_MPDU_DESC_INFO_PEER_META_DATA_MASK                                       0xffffffff

#endif
