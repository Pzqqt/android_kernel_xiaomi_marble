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


#ifndef _COEX_TX_STATUS_H_
#define _COEX_TX_STATUS_H_

#define NUM_OF_DWORDS_COEX_TX_STATUS 3

struct coex_tx_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t reserved_0a                                             :  7,
                      tx_bw                                                   :  3,
                      tx_status_reason                                        :  3,
                      tx_wait_ack                                             :  1,
                      fes_tx_is_gen_frame                                     :  1,
                      sch_tx_burst_ongoing                                    :  1,
                      current_tx_duration                                     : 16;
             uint32_t next_rx_active_time                                     : 16,
                      remaining_fes_time                                      : 16;
             uint32_t tx_antenna_mask                                         :  8,
                      shared_ant_tx_pwr                                       :  8,
                      other_ant_tx_pwr                                        :  8,
                      reserved_2                                              :  8;
#else
             uint32_t current_tx_duration                                     : 16,
                      sch_tx_burst_ongoing                                    :  1,
                      fes_tx_is_gen_frame                                     :  1,
                      tx_wait_ack                                             :  1,
                      tx_status_reason                                        :  3,
                      tx_bw                                                   :  3,
                      reserved_0a                                             :  7;
             uint32_t remaining_fes_time                                      : 16,
                      next_rx_active_time                                     : 16;
             uint32_t reserved_2                                              :  8,
                      other_ant_tx_pwr                                        :  8,
                      shared_ant_tx_pwr                                       :  8,
                      tx_antenna_mask                                         :  8;
#endif
};

#define COEX_TX_STATUS_RESERVED_0A_OFFSET                                           0x00000000
#define COEX_TX_STATUS_RESERVED_0A_LSB                                              0
#define COEX_TX_STATUS_RESERVED_0A_MSB                                              6
#define COEX_TX_STATUS_RESERVED_0A_MASK                                             0x0000007f

#define COEX_TX_STATUS_TX_BW_OFFSET                                                 0x00000000
#define COEX_TX_STATUS_TX_BW_LSB                                                    7
#define COEX_TX_STATUS_TX_BW_MSB                                                    9
#define COEX_TX_STATUS_TX_BW_MASK                                                   0x00000380

#define COEX_TX_STATUS_TX_STATUS_REASON_OFFSET                                      0x00000000
#define COEX_TX_STATUS_TX_STATUS_REASON_LSB                                         10
#define COEX_TX_STATUS_TX_STATUS_REASON_MSB                                         12
#define COEX_TX_STATUS_TX_STATUS_REASON_MASK                                        0x00001c00

#define COEX_TX_STATUS_TX_WAIT_ACK_OFFSET                                           0x00000000
#define COEX_TX_STATUS_TX_WAIT_ACK_LSB                                              13
#define COEX_TX_STATUS_TX_WAIT_ACK_MSB                                              13
#define COEX_TX_STATUS_TX_WAIT_ACK_MASK                                             0x00002000

#define COEX_TX_STATUS_FES_TX_IS_GEN_FRAME_OFFSET                                   0x00000000
#define COEX_TX_STATUS_FES_TX_IS_GEN_FRAME_LSB                                      14
#define COEX_TX_STATUS_FES_TX_IS_GEN_FRAME_MSB                                      14
#define COEX_TX_STATUS_FES_TX_IS_GEN_FRAME_MASK                                     0x00004000

#define COEX_TX_STATUS_SCH_TX_BURST_ONGOING_OFFSET                                  0x00000000
#define COEX_TX_STATUS_SCH_TX_BURST_ONGOING_LSB                                     15
#define COEX_TX_STATUS_SCH_TX_BURST_ONGOING_MSB                                     15
#define COEX_TX_STATUS_SCH_TX_BURST_ONGOING_MASK                                    0x00008000

#define COEX_TX_STATUS_CURRENT_TX_DURATION_OFFSET                                   0x00000000
#define COEX_TX_STATUS_CURRENT_TX_DURATION_LSB                                      16
#define COEX_TX_STATUS_CURRENT_TX_DURATION_MSB                                      31
#define COEX_TX_STATUS_CURRENT_TX_DURATION_MASK                                     0xffff0000

#define COEX_TX_STATUS_NEXT_RX_ACTIVE_TIME_OFFSET                                   0x00000004
#define COEX_TX_STATUS_NEXT_RX_ACTIVE_TIME_LSB                                      0
#define COEX_TX_STATUS_NEXT_RX_ACTIVE_TIME_MSB                                      15
#define COEX_TX_STATUS_NEXT_RX_ACTIVE_TIME_MASK                                     0x0000ffff

#define COEX_TX_STATUS_REMAINING_FES_TIME_OFFSET                                    0x00000004
#define COEX_TX_STATUS_REMAINING_FES_TIME_LSB                                       16
#define COEX_TX_STATUS_REMAINING_FES_TIME_MSB                                       31
#define COEX_TX_STATUS_REMAINING_FES_TIME_MASK                                      0xffff0000

#define COEX_TX_STATUS_TX_ANTENNA_MASK_OFFSET                                       0x00000008
#define COEX_TX_STATUS_TX_ANTENNA_MASK_LSB                                          0
#define COEX_TX_STATUS_TX_ANTENNA_MASK_MSB                                          7
#define COEX_TX_STATUS_TX_ANTENNA_MASK_MASK                                         0x000000ff

#define COEX_TX_STATUS_SHARED_ANT_TX_PWR_OFFSET                                     0x00000008
#define COEX_TX_STATUS_SHARED_ANT_TX_PWR_LSB                                        8
#define COEX_TX_STATUS_SHARED_ANT_TX_PWR_MSB                                        15
#define COEX_TX_STATUS_SHARED_ANT_TX_PWR_MASK                                       0x0000ff00

#define COEX_TX_STATUS_OTHER_ANT_TX_PWR_OFFSET                                      0x00000008
#define COEX_TX_STATUS_OTHER_ANT_TX_PWR_LSB                                         16
#define COEX_TX_STATUS_OTHER_ANT_TX_PWR_MSB                                         23
#define COEX_TX_STATUS_OTHER_ANT_TX_PWR_MASK                                        0x00ff0000

#define COEX_TX_STATUS_RESERVED_2_OFFSET                                            0x00000008
#define COEX_TX_STATUS_RESERVED_2_LSB                                               24
#define COEX_TX_STATUS_RESERVED_2_MSB                                               31
#define COEX_TX_STATUS_RESERVED_2_MASK                                              0xff000000

#endif
