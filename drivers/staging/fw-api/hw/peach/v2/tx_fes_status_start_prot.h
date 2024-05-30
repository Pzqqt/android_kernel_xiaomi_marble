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


#ifndef _TX_FES_STATUS_START_PROT_H_
#define _TX_FES_STATUS_START_PROT_H_

#define NUM_OF_DWORDS_TX_FES_STATUS_START_PROT 4

struct tx_fes_status_start_prot {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t prot_timestamp_lower_32                                 : 32;
             uint32_t prot_timestamp_upper_32                                 : 32;
             uint32_t subband_mask                                            : 16,
                      reserved_2b                                             :  4,
                      prot_coex_based_tx_bw                                   :  3,
                      prot_coex_based_ant_mask                                :  8,
                      prot_coex_result_alt_based                              :  1;
             uint32_t prot_coex_tx_pwr_shared_ant                             :  8,
                      prot_coex_tx_pwr_ant                                    :  8,
                      prot_concurrent_bt_tx                                   :  1,
                      prot_concurrent_wlan_tx                                 :  1,
                      prot_concurrent_wan_tx                                  :  1,
                      prot_concurrent_wan_rx                                  :  1,
                      prot_coex_pwr_reduction_bt                              :  1,
                      prot_coex_pwr_reduction_wlan                            :  1,
                      prot_coex_pwr_reduction_wan                             :  1,
                      prot_request_packet_bw                                  :  3,
                      response_type                                           :  5,
                      reserved_3a                                             :  1;
#else
             uint32_t prot_timestamp_lower_32                                 : 32;
             uint32_t prot_timestamp_upper_32                                 : 32;
             uint32_t prot_coex_result_alt_based                              :  1,
                      prot_coex_based_ant_mask                                :  8,
                      prot_coex_based_tx_bw                                   :  3,
                      reserved_2b                                             :  4,
                      subband_mask                                            : 16;
             uint32_t reserved_3a                                             :  1,
                      response_type                                           :  5,
                      prot_request_packet_bw                                  :  3,
                      prot_coex_pwr_reduction_wan                             :  1,
                      prot_coex_pwr_reduction_wlan                            :  1,
                      prot_coex_pwr_reduction_bt                              :  1,
                      prot_concurrent_wan_rx                                  :  1,
                      prot_concurrent_wan_tx                                  :  1,
                      prot_concurrent_wlan_tx                                 :  1,
                      prot_concurrent_bt_tx                                   :  1,
                      prot_coex_tx_pwr_ant                                    :  8,
                      prot_coex_tx_pwr_shared_ant                             :  8;
#endif
};

#define TX_FES_STATUS_START_PROT_PROT_TIMESTAMP_LOWER_32_OFFSET                     0x00000000
#define TX_FES_STATUS_START_PROT_PROT_TIMESTAMP_LOWER_32_LSB                        0
#define TX_FES_STATUS_START_PROT_PROT_TIMESTAMP_LOWER_32_MSB                        31
#define TX_FES_STATUS_START_PROT_PROT_TIMESTAMP_LOWER_32_MASK                       0xffffffff

#define TX_FES_STATUS_START_PROT_PROT_TIMESTAMP_UPPER_32_OFFSET                     0x00000004
#define TX_FES_STATUS_START_PROT_PROT_TIMESTAMP_UPPER_32_LSB                        0
#define TX_FES_STATUS_START_PROT_PROT_TIMESTAMP_UPPER_32_MSB                        31
#define TX_FES_STATUS_START_PROT_PROT_TIMESTAMP_UPPER_32_MASK                       0xffffffff

#define TX_FES_STATUS_START_PROT_SUBBAND_MASK_OFFSET                                0x00000008
#define TX_FES_STATUS_START_PROT_SUBBAND_MASK_LSB                                   0
#define TX_FES_STATUS_START_PROT_SUBBAND_MASK_MSB                                   15
#define TX_FES_STATUS_START_PROT_SUBBAND_MASK_MASK                                  0x0000ffff

#define TX_FES_STATUS_START_PROT_RESERVED_2B_OFFSET                                 0x00000008
#define TX_FES_STATUS_START_PROT_RESERVED_2B_LSB                                    16
#define TX_FES_STATUS_START_PROT_RESERVED_2B_MSB                                    19
#define TX_FES_STATUS_START_PROT_RESERVED_2B_MASK                                   0x000f0000

#define TX_FES_STATUS_START_PROT_PROT_COEX_BASED_TX_BW_OFFSET                       0x00000008
#define TX_FES_STATUS_START_PROT_PROT_COEX_BASED_TX_BW_LSB                          20
#define TX_FES_STATUS_START_PROT_PROT_COEX_BASED_TX_BW_MSB                          22
#define TX_FES_STATUS_START_PROT_PROT_COEX_BASED_TX_BW_MASK                         0x00700000

#define TX_FES_STATUS_START_PROT_PROT_COEX_BASED_ANT_MASK_OFFSET                    0x00000008
#define TX_FES_STATUS_START_PROT_PROT_COEX_BASED_ANT_MASK_LSB                       23
#define TX_FES_STATUS_START_PROT_PROT_COEX_BASED_ANT_MASK_MSB                       30
#define TX_FES_STATUS_START_PROT_PROT_COEX_BASED_ANT_MASK_MASK                      0x7f800000

#define TX_FES_STATUS_START_PROT_PROT_COEX_RESULT_ALT_BASED_OFFSET                  0x00000008
#define TX_FES_STATUS_START_PROT_PROT_COEX_RESULT_ALT_BASED_LSB                     31
#define TX_FES_STATUS_START_PROT_PROT_COEX_RESULT_ALT_BASED_MSB                     31
#define TX_FES_STATUS_START_PROT_PROT_COEX_RESULT_ALT_BASED_MASK                    0x80000000

#define TX_FES_STATUS_START_PROT_PROT_COEX_TX_PWR_SHARED_ANT_OFFSET                 0x0000000c
#define TX_FES_STATUS_START_PROT_PROT_COEX_TX_PWR_SHARED_ANT_LSB                    0
#define TX_FES_STATUS_START_PROT_PROT_COEX_TX_PWR_SHARED_ANT_MSB                    7
#define TX_FES_STATUS_START_PROT_PROT_COEX_TX_PWR_SHARED_ANT_MASK                   0x000000ff

#define TX_FES_STATUS_START_PROT_PROT_COEX_TX_PWR_ANT_OFFSET                        0x0000000c
#define TX_FES_STATUS_START_PROT_PROT_COEX_TX_PWR_ANT_LSB                           8
#define TX_FES_STATUS_START_PROT_PROT_COEX_TX_PWR_ANT_MSB                           15
#define TX_FES_STATUS_START_PROT_PROT_COEX_TX_PWR_ANT_MASK                          0x0000ff00

#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_BT_TX_OFFSET                       0x0000000c
#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_BT_TX_LSB                          16
#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_BT_TX_MSB                          16
#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_BT_TX_MASK                         0x00010000

#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_WLAN_TX_OFFSET                     0x0000000c
#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_WLAN_TX_LSB                        17
#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_WLAN_TX_MSB                        17
#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_WLAN_TX_MASK                       0x00020000

#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_WAN_TX_OFFSET                      0x0000000c
#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_WAN_TX_LSB                         18
#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_WAN_TX_MSB                         18
#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_WAN_TX_MASK                        0x00040000

#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_WAN_RX_OFFSET                      0x0000000c
#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_WAN_RX_LSB                         19
#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_WAN_RX_MSB                         19
#define TX_FES_STATUS_START_PROT_PROT_CONCURRENT_WAN_RX_MASK                        0x00080000

#define TX_FES_STATUS_START_PROT_PROT_COEX_PWR_REDUCTION_BT_OFFSET                  0x0000000c
#define TX_FES_STATUS_START_PROT_PROT_COEX_PWR_REDUCTION_BT_LSB                     20
#define TX_FES_STATUS_START_PROT_PROT_COEX_PWR_REDUCTION_BT_MSB                     20
#define TX_FES_STATUS_START_PROT_PROT_COEX_PWR_REDUCTION_BT_MASK                    0x00100000

#define TX_FES_STATUS_START_PROT_PROT_COEX_PWR_REDUCTION_WLAN_OFFSET                0x0000000c
#define TX_FES_STATUS_START_PROT_PROT_COEX_PWR_REDUCTION_WLAN_LSB                   21
#define TX_FES_STATUS_START_PROT_PROT_COEX_PWR_REDUCTION_WLAN_MSB                   21
#define TX_FES_STATUS_START_PROT_PROT_COEX_PWR_REDUCTION_WLAN_MASK                  0x00200000

#define TX_FES_STATUS_START_PROT_PROT_COEX_PWR_REDUCTION_WAN_OFFSET                 0x0000000c
#define TX_FES_STATUS_START_PROT_PROT_COEX_PWR_REDUCTION_WAN_LSB                    22
#define TX_FES_STATUS_START_PROT_PROT_COEX_PWR_REDUCTION_WAN_MSB                    22
#define TX_FES_STATUS_START_PROT_PROT_COEX_PWR_REDUCTION_WAN_MASK                   0x00400000

#define TX_FES_STATUS_START_PROT_PROT_REQUEST_PACKET_BW_OFFSET                      0x0000000c
#define TX_FES_STATUS_START_PROT_PROT_REQUEST_PACKET_BW_LSB                         23
#define TX_FES_STATUS_START_PROT_PROT_REQUEST_PACKET_BW_MSB                         25
#define TX_FES_STATUS_START_PROT_PROT_REQUEST_PACKET_BW_MASK                        0x03800000

#define TX_FES_STATUS_START_PROT_RESPONSE_TYPE_OFFSET                               0x0000000c
#define TX_FES_STATUS_START_PROT_RESPONSE_TYPE_LSB                                  26
#define TX_FES_STATUS_START_PROT_RESPONSE_TYPE_MSB                                  30
#define TX_FES_STATUS_START_PROT_RESPONSE_TYPE_MASK                                 0x7c000000

#define TX_FES_STATUS_START_PROT_RESERVED_3A_OFFSET                                 0x0000000c
#define TX_FES_STATUS_START_PROT_RESERVED_3A_LSB                                    31
#define TX_FES_STATUS_START_PROT_RESERVED_3A_MSB                                    31
#define TX_FES_STATUS_START_PROT_RESERVED_3A_MASK                                   0x80000000

#endif
