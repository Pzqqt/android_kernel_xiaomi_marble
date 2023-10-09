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


#ifndef _RECEIVE_PKT_START_INFO_H_
#define _RECEIVE_PKT_START_INFO_H_

#define NUM_OF_DWORDS_RECEIVE_PKT_START_INFO 4

struct receive_pkt_start_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t reception_type                                          :  4,
                      rx_chain_mask_type                                      :  1,
                      receive_bandwidth                                       :  3,
                      rx_chain_mask                                           :  8,
                      phy_ppdu_id                                             : 16;
             uint32_t ppdu_start_timestamp_31_0                               : 32;
             uint32_t ppdu_start_timestamp_63_32                              : 32;
             uint32_t preamble_time_to_rxframe                                :  8,
                      standalone_sniffer_mode                                 :  1,
                      reserved_3a                                             : 23;
#else
             uint32_t phy_ppdu_id                                             : 16,
                      rx_chain_mask                                           :  8,
                      receive_bandwidth                                       :  3,
                      rx_chain_mask_type                                      :  1,
                      reception_type                                          :  4;
             uint32_t ppdu_start_timestamp_31_0                               : 32;
             uint32_t ppdu_start_timestamp_63_32                              : 32;
             uint32_t reserved_3a                                             : 23,
                      standalone_sniffer_mode                                 :  1,
                      preamble_time_to_rxframe                                :  8;
#endif
};

#define RECEIVE_PKT_START_INFO_RECEPTION_TYPE_OFFSET                                0x00000000
#define RECEIVE_PKT_START_INFO_RECEPTION_TYPE_LSB                                   0
#define RECEIVE_PKT_START_INFO_RECEPTION_TYPE_MSB                                   3
#define RECEIVE_PKT_START_INFO_RECEPTION_TYPE_MASK                                  0x0000000f

#define RECEIVE_PKT_START_INFO_RX_CHAIN_MASK_TYPE_OFFSET                            0x00000000
#define RECEIVE_PKT_START_INFO_RX_CHAIN_MASK_TYPE_LSB                               4
#define RECEIVE_PKT_START_INFO_RX_CHAIN_MASK_TYPE_MSB                               4
#define RECEIVE_PKT_START_INFO_RX_CHAIN_MASK_TYPE_MASK                              0x00000010

#define RECEIVE_PKT_START_INFO_RECEIVE_BANDWIDTH_OFFSET                             0x00000000
#define RECEIVE_PKT_START_INFO_RECEIVE_BANDWIDTH_LSB                                5
#define RECEIVE_PKT_START_INFO_RECEIVE_BANDWIDTH_MSB                                7
#define RECEIVE_PKT_START_INFO_RECEIVE_BANDWIDTH_MASK                               0x000000e0

#define RECEIVE_PKT_START_INFO_RX_CHAIN_MASK_OFFSET                                 0x00000000
#define RECEIVE_PKT_START_INFO_RX_CHAIN_MASK_LSB                                    8
#define RECEIVE_PKT_START_INFO_RX_CHAIN_MASK_MSB                                    15
#define RECEIVE_PKT_START_INFO_RX_CHAIN_MASK_MASK                                   0x0000ff00

#define RECEIVE_PKT_START_INFO_PHY_PPDU_ID_OFFSET                                   0x00000000
#define RECEIVE_PKT_START_INFO_PHY_PPDU_ID_LSB                                      16
#define RECEIVE_PKT_START_INFO_PHY_PPDU_ID_MSB                                      31
#define RECEIVE_PKT_START_INFO_PHY_PPDU_ID_MASK                                     0xffff0000

#define RECEIVE_PKT_START_INFO_PPDU_START_TIMESTAMP_31_0_OFFSET                     0x00000004
#define RECEIVE_PKT_START_INFO_PPDU_START_TIMESTAMP_31_0_LSB                        0
#define RECEIVE_PKT_START_INFO_PPDU_START_TIMESTAMP_31_0_MSB                        31
#define RECEIVE_PKT_START_INFO_PPDU_START_TIMESTAMP_31_0_MASK                       0xffffffff

#define RECEIVE_PKT_START_INFO_PPDU_START_TIMESTAMP_63_32_OFFSET                    0x00000008
#define RECEIVE_PKT_START_INFO_PPDU_START_TIMESTAMP_63_32_LSB                       0
#define RECEIVE_PKT_START_INFO_PPDU_START_TIMESTAMP_63_32_MSB                       31
#define RECEIVE_PKT_START_INFO_PPDU_START_TIMESTAMP_63_32_MASK                      0xffffffff

#define RECEIVE_PKT_START_INFO_PREAMBLE_TIME_TO_RXFRAME_OFFSET                      0x0000000c
#define RECEIVE_PKT_START_INFO_PREAMBLE_TIME_TO_RXFRAME_LSB                         0
#define RECEIVE_PKT_START_INFO_PREAMBLE_TIME_TO_RXFRAME_MSB                         7
#define RECEIVE_PKT_START_INFO_PREAMBLE_TIME_TO_RXFRAME_MASK                        0x000000ff

#define RECEIVE_PKT_START_INFO_STANDALONE_SNIFFER_MODE_OFFSET                       0x0000000c
#define RECEIVE_PKT_START_INFO_STANDALONE_SNIFFER_MODE_LSB                          8
#define RECEIVE_PKT_START_INFO_STANDALONE_SNIFFER_MODE_MSB                          8
#define RECEIVE_PKT_START_INFO_STANDALONE_SNIFFER_MODE_MASK                         0x00000100

#define RECEIVE_PKT_START_INFO_RESERVED_3A_OFFSET                                   0x0000000c
#define RECEIVE_PKT_START_INFO_RESERVED_3A_LSB                                      9
#define RECEIVE_PKT_START_INFO_RESERVED_3A_MSB                                      31
#define RECEIVE_PKT_START_INFO_RESERVED_3A_MASK                                     0xfffffe00

#endif
