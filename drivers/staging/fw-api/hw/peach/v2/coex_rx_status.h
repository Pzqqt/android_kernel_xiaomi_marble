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


#ifndef _COEX_RX_STATUS_H_
#define _COEX_RX_STATUS_H_

#define NUM_OF_DWORDS_COEX_RX_STATUS 2

struct coex_rx_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rx_mac_frame_status                                     :  2,
                      rx_with_tx_response                                     :  1,
                      rx_rate                                                 :  5,
                      rx_bw                                                   :  3,
                      single_mpdu                                             :  1,
                      filter_status                                           :  1,
                      ampdu                                                   :  1,
                      directed                                                :  1,
                      reserved_0                                              :  1,
                      rx_nss                                                  :  3,
                      rx_rssi                                                 :  8,
                      rx_type                                                 :  3,
                      retry_bit_setting                                       :  1,
                      more_data_bit_setting                                   :  1;
             uint32_t remain_rx_packet_time                                   : 16,
                      rx_remaining_fes_time                                   : 16;
#else
             uint32_t more_data_bit_setting                                   :  1,
                      retry_bit_setting                                       :  1,
                      rx_type                                                 :  3,
                      rx_rssi                                                 :  8,
                      rx_nss                                                  :  3,
                      reserved_0                                              :  1,
                      directed                                                :  1,
                      ampdu                                                   :  1,
                      filter_status                                           :  1,
                      single_mpdu                                             :  1,
                      rx_bw                                                   :  3,
                      rx_rate                                                 :  5,
                      rx_with_tx_response                                     :  1,
                      rx_mac_frame_status                                     :  2;
             uint32_t rx_remaining_fes_time                                   : 16,
                      remain_rx_packet_time                                   : 16;
#endif
};

#define COEX_RX_STATUS_RX_MAC_FRAME_STATUS_OFFSET                                   0x00000000
#define COEX_RX_STATUS_RX_MAC_FRAME_STATUS_LSB                                      0
#define COEX_RX_STATUS_RX_MAC_FRAME_STATUS_MSB                                      1
#define COEX_RX_STATUS_RX_MAC_FRAME_STATUS_MASK                                     0x00000003

#define COEX_RX_STATUS_RX_WITH_TX_RESPONSE_OFFSET                                   0x00000000
#define COEX_RX_STATUS_RX_WITH_TX_RESPONSE_LSB                                      2
#define COEX_RX_STATUS_RX_WITH_TX_RESPONSE_MSB                                      2
#define COEX_RX_STATUS_RX_WITH_TX_RESPONSE_MASK                                     0x00000004

#define COEX_RX_STATUS_RX_RATE_OFFSET                                               0x00000000
#define COEX_RX_STATUS_RX_RATE_LSB                                                  3
#define COEX_RX_STATUS_RX_RATE_MSB                                                  7
#define COEX_RX_STATUS_RX_RATE_MASK                                                 0x000000f8

#define COEX_RX_STATUS_RX_BW_OFFSET                                                 0x00000000
#define COEX_RX_STATUS_RX_BW_LSB                                                    8
#define COEX_RX_STATUS_RX_BW_MSB                                                    10
#define COEX_RX_STATUS_RX_BW_MASK                                                   0x00000700

#define COEX_RX_STATUS_SINGLE_MPDU_OFFSET                                           0x00000000
#define COEX_RX_STATUS_SINGLE_MPDU_LSB                                              11
#define COEX_RX_STATUS_SINGLE_MPDU_MSB                                              11
#define COEX_RX_STATUS_SINGLE_MPDU_MASK                                             0x00000800

#define COEX_RX_STATUS_FILTER_STATUS_OFFSET                                         0x00000000
#define COEX_RX_STATUS_FILTER_STATUS_LSB                                            12
#define COEX_RX_STATUS_FILTER_STATUS_MSB                                            12
#define COEX_RX_STATUS_FILTER_STATUS_MASK                                           0x00001000

#define COEX_RX_STATUS_AMPDU_OFFSET                                                 0x00000000
#define COEX_RX_STATUS_AMPDU_LSB                                                    13
#define COEX_RX_STATUS_AMPDU_MSB                                                    13
#define COEX_RX_STATUS_AMPDU_MASK                                                   0x00002000

#define COEX_RX_STATUS_DIRECTED_OFFSET                                              0x00000000
#define COEX_RX_STATUS_DIRECTED_LSB                                                 14
#define COEX_RX_STATUS_DIRECTED_MSB                                                 14
#define COEX_RX_STATUS_DIRECTED_MASK                                                0x00004000

#define COEX_RX_STATUS_RESERVED_0_OFFSET                                            0x00000000
#define COEX_RX_STATUS_RESERVED_0_LSB                                               15
#define COEX_RX_STATUS_RESERVED_0_MSB                                               15
#define COEX_RX_STATUS_RESERVED_0_MASK                                              0x00008000

#define COEX_RX_STATUS_RX_NSS_OFFSET                                                0x00000000
#define COEX_RX_STATUS_RX_NSS_LSB                                                   16
#define COEX_RX_STATUS_RX_NSS_MSB                                                   18
#define COEX_RX_STATUS_RX_NSS_MASK                                                  0x00070000

#define COEX_RX_STATUS_RX_RSSI_OFFSET                                               0x00000000
#define COEX_RX_STATUS_RX_RSSI_LSB                                                  19
#define COEX_RX_STATUS_RX_RSSI_MSB                                                  26
#define COEX_RX_STATUS_RX_RSSI_MASK                                                 0x07f80000

#define COEX_RX_STATUS_RX_TYPE_OFFSET                                               0x00000000
#define COEX_RX_STATUS_RX_TYPE_LSB                                                  27
#define COEX_RX_STATUS_RX_TYPE_MSB                                                  29
#define COEX_RX_STATUS_RX_TYPE_MASK                                                 0x38000000

#define COEX_RX_STATUS_RETRY_BIT_SETTING_OFFSET                                     0x00000000
#define COEX_RX_STATUS_RETRY_BIT_SETTING_LSB                                        30
#define COEX_RX_STATUS_RETRY_BIT_SETTING_MSB                                        30
#define COEX_RX_STATUS_RETRY_BIT_SETTING_MASK                                       0x40000000

#define COEX_RX_STATUS_MORE_DATA_BIT_SETTING_OFFSET                                 0x00000000
#define COEX_RX_STATUS_MORE_DATA_BIT_SETTING_LSB                                    31
#define COEX_RX_STATUS_MORE_DATA_BIT_SETTING_MSB                                    31
#define COEX_RX_STATUS_MORE_DATA_BIT_SETTING_MASK                                   0x80000000

#define COEX_RX_STATUS_REMAIN_RX_PACKET_TIME_OFFSET                                 0x00000004
#define COEX_RX_STATUS_REMAIN_RX_PACKET_TIME_LSB                                    0
#define COEX_RX_STATUS_REMAIN_RX_PACKET_TIME_MSB                                    15
#define COEX_RX_STATUS_REMAIN_RX_PACKET_TIME_MASK                                   0x0000ffff

#define COEX_RX_STATUS_RX_REMAINING_FES_TIME_OFFSET                                 0x00000004
#define COEX_RX_STATUS_RX_REMAINING_FES_TIME_LSB                                    16
#define COEX_RX_STATUS_RX_REMAINING_FES_TIME_MSB                                    31
#define COEX_RX_STATUS_RX_REMAINING_FES_TIME_MASK                                   0xffff0000

#endif
