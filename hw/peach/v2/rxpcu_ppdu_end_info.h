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


#ifndef _RXPCU_PPDU_END_INFO_H_
#define _RXPCU_PPDU_END_INFO_H_

#include "phyrx_abort_request_info.h"
#include "macrx_abort_request_info.h"
#include "rxpcu_ppdu_end_layout_info.h"
#define NUM_OF_DWORDS_RXPCU_PPDU_END_INFO 31

struct rxpcu_ppdu_end_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t wb_timestamp_lower_32                                   : 32;
             uint32_t wb_timestamp_upper_32                                   : 32;
             uint32_t rx_antenna                                              : 24,
                      tx_ht_vht_ack                                           :  1,
                      unsupported_mu_nc                                       :  1,
                      otp_txbf_disable                                        :  1,
                      previous_tlv_corrupted                                  :  1,
                      phyrx_abort_request_info_valid                          :  1,
                      macrx_abort_request_info_valid                          :  1,
                      reserved                                                :  2;
             uint32_t coex_bt_tx_from_start_of_rx                             :  1,
                      coex_bt_tx_after_start_of_rx                            :  1,
                      coex_wan_tx_from_start_of_rx                            :  1,
                      coex_wan_tx_after_start_of_rx                           :  1,
                      coex_wlan_tx_from_start_of_rx                           :  1,
                      coex_wlan_tx_after_start_of_rx                          :  1,
                      mpdu_delimiter_errors_seen                              :  1,
                      __reserved_g_0012                                                  :  2,
                      dialog_token                                            :  8,
                      follow_up_dialog_token                                  :  8,
                      bb_captured_channel                                     :  1,
                      bb_captured_reason                                      :  3,
                      bb_captured_timeout                                     :  1,
                      coex_uwb_tx_after_start_of_rx                           :  1,
                      coex_uwb_tx_from_start_of_rx                            :  1;
             uint32_t before_mpdu_count_passing_fcs                           : 10,
                      before_mpdu_count_failing_fcs                           : 10,
                      after_mpdu_count_passing_fcs                            : 10,
                      reserved_4                                              :  2;
             uint32_t after_mpdu_count_failing_fcs                            : 10,
                      reserved_5                                              : 22;
             uint32_t phy_timestamp_tx_lower_32                               : 32;
             uint32_t phy_timestamp_tx_upper_32                               : 32;
             uint32_t bb_length                                               : 16,
                      bb_data                                                 :  1,
                      reserved_8                                              :  3,
                      first_bt_broadcast_status_details                       : 12;
             uint32_t rx_ppdu_duration                                        : 24,
                      reserved_9                                              :  8;
             uint32_t ast_index                                               : 16,
                      ast_index_valid                                         :  1,
                      reserved_10                                             :  3,
                      second_bt_broadcast_status_details                      : 12;
             struct   phyrx_abort_request_info                                  phyrx_abort_request_info_details;
             struct   macrx_abort_request_info                                  macrx_abort_request_info_details;
             uint16_t pre_bt_broadcast_status_details                         : 12,
                      reserved_12a                                            :  4;
             uint32_t non_qos_sn_info_valid                                   :  1,
                      rts_or_trig_protected_ppdu                              :  1,
                      rts_or_trig_prot_type                                   :  2,
                      reserved_13a                                            :  2,
                      non_qos_sn_highest                                      : 12,
                      non_qos_sn_highest_retry_setting                        :  1,
                      non_qos_sn_lowest                                       : 12,
                      non_qos_sn_lowest_retry_setting                         :  1;
             uint32_t qos_sn_1_info_valid                                     :  1,
                      reserved_14a                                            :  1,
                      qos_sn_1_tid                                            :  4,
                      qos_sn_1_highest                                        : 12,
                      qos_sn_1_highest_retry_setting                          :  1,
                      qos_sn_1_lowest                                         : 12,
                      qos_sn_1_lowest_retry_setting                           :  1;
             uint32_t qos_sn_2_info_valid                                     :  1,
                      reserved_15a                                            :  1,
                      qos_sn_2_tid                                            :  4,
                      qos_sn_2_highest                                        : 12,
                      qos_sn_2_highest_retry_setting                          :  1,
                      qos_sn_2_lowest                                         : 12,
                      qos_sn_2_lowest_retry_setting                           :  1;
             struct   rxpcu_ppdu_end_layout_info                                rxpcu_ppdu_end_layout_details;
             uint32_t corrupted_due_to_fifo_delay                             :  1,
                      qos_sn_1_more_frag_state                                :  1,
                      qos_sn_1_frag_num_state                                 :  4,
                      qos_sn_2_more_frag_state                                :  1,
                      qos_sn_2_frag_num_state                                 :  4,
                      rts_or_trig_prot_non_11a                                :  1,
                      rts_or_trig_prot_rate_mcs                               :  4,
                      rts_or_trig_prot_peer_addr_15_0                         : 16;
             uint32_t rts_or_trig_prot_peer_addr_47_16                        : 32;
             uint32_t rts_or_trig_rx_count                                    : 32;
             uint32_t cts_or_null_tx_count                                    : 32;
             uint32_t rx_ppdu_end_marker                                      : 32;
#else
             uint32_t wb_timestamp_lower_32                                   : 32;
             uint32_t wb_timestamp_upper_32                                   : 32;
             uint32_t reserved                                                :  2,
                      macrx_abort_request_info_valid                          :  1,
                      phyrx_abort_request_info_valid                          :  1,
                      previous_tlv_corrupted                                  :  1,
                      otp_txbf_disable                                        :  1,
                      unsupported_mu_nc                                       :  1,
                      tx_ht_vht_ack                                           :  1,
                      rx_antenna                                              : 24;
             uint32_t coex_uwb_tx_from_start_of_rx                            :  1,
                      coex_uwb_tx_after_start_of_rx                           :  1,
                      bb_captured_timeout                                     :  1,
                      bb_captured_reason                                      :  3,
                      bb_captured_channel                                     :  1,
                      follow_up_dialog_token                                  :  8,
                      dialog_token                                            :  8,
                      __reserved_g_0012                                                  :  2,
                      mpdu_delimiter_errors_seen                              :  1,
                      coex_wlan_tx_after_start_of_rx                          :  1,
                      coex_wlan_tx_from_start_of_rx                           :  1,
                      coex_wan_tx_after_start_of_rx                           :  1,
                      coex_wan_tx_from_start_of_rx                            :  1,
                      coex_bt_tx_after_start_of_rx                            :  1,
                      coex_bt_tx_from_start_of_rx                             :  1;
             uint32_t reserved_4                                              :  2,
                      after_mpdu_count_passing_fcs                            : 10,
                      before_mpdu_count_failing_fcs                           : 10,
                      before_mpdu_count_passing_fcs                           : 10;
             uint32_t reserved_5                                              : 22,
                      after_mpdu_count_failing_fcs                            : 10;
             uint32_t phy_timestamp_tx_lower_32                               : 32;
             uint32_t phy_timestamp_tx_upper_32                               : 32;
             uint32_t first_bt_broadcast_status_details                       : 12,
                      reserved_8                                              :  3,
                      bb_data                                                 :  1,
                      bb_length                                               : 16;
             uint32_t reserved_9                                              :  8,
                      rx_ppdu_duration                                        : 24;
             uint32_t second_bt_broadcast_status_details                      : 12,
                      reserved_10                                             :  3,
                      ast_index_valid                                         :  1,
                      ast_index                                               : 16;
             struct   phyrx_abort_request_info                                  phyrx_abort_request_info_details;
             uint32_t reserved_12a                                            :  4,
                      pre_bt_broadcast_status_details                         : 12;
             struct   macrx_abort_request_info                                  macrx_abort_request_info_details;
             uint32_t non_qos_sn_lowest_retry_setting                         :  1,
                      non_qos_sn_lowest                                       : 12,
                      non_qos_sn_highest_retry_setting                        :  1,
                      non_qos_sn_highest                                      : 12,
                      reserved_13a                                            :  2,
                      rts_or_trig_prot_type                                   :  2,
                      rts_or_trig_protected_ppdu                              :  1,
                      non_qos_sn_info_valid                                   :  1;
             uint32_t qos_sn_1_lowest_retry_setting                           :  1,
                      qos_sn_1_lowest                                         : 12,
                      qos_sn_1_highest_retry_setting                          :  1,
                      qos_sn_1_highest                                        : 12,
                      qos_sn_1_tid                                            :  4,
                      reserved_14a                                            :  1,
                      qos_sn_1_info_valid                                     :  1;
             uint32_t qos_sn_2_lowest_retry_setting                           :  1,
                      qos_sn_2_lowest                                         : 12,
                      qos_sn_2_highest_retry_setting                          :  1,
                      qos_sn_2_highest                                        : 12,
                      qos_sn_2_tid                                            :  4,
                      reserved_15a                                            :  1,
                      qos_sn_2_info_valid                                     :  1;
             struct   rxpcu_ppdu_end_layout_info                                rxpcu_ppdu_end_layout_details;
             uint32_t rts_or_trig_prot_peer_addr_15_0                         : 16,
                      rts_or_trig_prot_rate_mcs                               :  4,
                      rts_or_trig_prot_non_11a                                :  1,
                      qos_sn_2_frag_num_state                                 :  4,
                      qos_sn_2_more_frag_state                                :  1,
                      qos_sn_1_frag_num_state                                 :  4,
                      qos_sn_1_more_frag_state                                :  1,
                      corrupted_due_to_fifo_delay                             :  1;
             uint32_t rts_or_trig_prot_peer_addr_47_16                        : 32;
             uint32_t rts_or_trig_rx_count                                    : 32;
             uint32_t cts_or_null_tx_count                                    : 32;
             uint32_t rx_ppdu_end_marker                                      : 32;
#endif
};

#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_LOWER_32_OFFSET                            0x00000000
#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_LOWER_32_LSB                               0
#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_LOWER_32_MSB                               31
#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_LOWER_32_MASK                              0xffffffff

#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_UPPER_32_OFFSET                            0x00000004
#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_UPPER_32_LSB                               0
#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_UPPER_32_MSB                               31
#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_UPPER_32_MASK                              0xffffffff

#define RXPCU_PPDU_END_INFO_RX_ANTENNA_OFFSET                                       0x00000008
#define RXPCU_PPDU_END_INFO_RX_ANTENNA_LSB                                          0
#define RXPCU_PPDU_END_INFO_RX_ANTENNA_MSB                                          23
#define RXPCU_PPDU_END_INFO_RX_ANTENNA_MASK                                         0x00ffffff

#define RXPCU_PPDU_END_INFO_TX_HT_VHT_ACK_OFFSET                                    0x00000008
#define RXPCU_PPDU_END_INFO_TX_HT_VHT_ACK_LSB                                       24
#define RXPCU_PPDU_END_INFO_TX_HT_VHT_ACK_MSB                                       24
#define RXPCU_PPDU_END_INFO_TX_HT_VHT_ACK_MASK                                      0x01000000

#define RXPCU_PPDU_END_INFO_UNSUPPORTED_MU_NC_OFFSET                                0x00000008
#define RXPCU_PPDU_END_INFO_UNSUPPORTED_MU_NC_LSB                                   25
#define RXPCU_PPDU_END_INFO_UNSUPPORTED_MU_NC_MSB                                   25
#define RXPCU_PPDU_END_INFO_UNSUPPORTED_MU_NC_MASK                                  0x02000000

#define RXPCU_PPDU_END_INFO_OTP_TXBF_DISABLE_OFFSET                                 0x00000008
#define RXPCU_PPDU_END_INFO_OTP_TXBF_DISABLE_LSB                                    26
#define RXPCU_PPDU_END_INFO_OTP_TXBF_DISABLE_MSB                                    26
#define RXPCU_PPDU_END_INFO_OTP_TXBF_DISABLE_MASK                                   0x04000000

#define RXPCU_PPDU_END_INFO_PREVIOUS_TLV_CORRUPTED_OFFSET                           0x00000008
#define RXPCU_PPDU_END_INFO_PREVIOUS_TLV_CORRUPTED_LSB                              27
#define RXPCU_PPDU_END_INFO_PREVIOUS_TLV_CORRUPTED_MSB                              27
#define RXPCU_PPDU_END_INFO_PREVIOUS_TLV_CORRUPTED_MASK                             0x08000000

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_VALID_OFFSET                   0x00000008
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_VALID_LSB                      28
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_VALID_MSB                      28
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_VALID_MASK                     0x10000000

#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_VALID_OFFSET                   0x00000008
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_VALID_LSB                      29
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_VALID_MSB                      29
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_VALID_MASK                     0x20000000

#define RXPCU_PPDU_END_INFO_RESERVED_OFFSET                                         0x00000008
#define RXPCU_PPDU_END_INFO_RESERVED_LSB                                            30
#define RXPCU_PPDU_END_INFO_RESERVED_MSB                                            31
#define RXPCU_PPDU_END_INFO_RESERVED_MASK                                           0xc0000000

#define RXPCU_PPDU_END_INFO_COEX_BT_TX_FROM_START_OF_RX_OFFSET                      0x0000000c
#define RXPCU_PPDU_END_INFO_COEX_BT_TX_FROM_START_OF_RX_LSB                         0
#define RXPCU_PPDU_END_INFO_COEX_BT_TX_FROM_START_OF_RX_MSB                         0
#define RXPCU_PPDU_END_INFO_COEX_BT_TX_FROM_START_OF_RX_MASK                        0x00000001

#define RXPCU_PPDU_END_INFO_COEX_BT_TX_AFTER_START_OF_RX_OFFSET                     0x0000000c
#define RXPCU_PPDU_END_INFO_COEX_BT_TX_AFTER_START_OF_RX_LSB                        1
#define RXPCU_PPDU_END_INFO_COEX_BT_TX_AFTER_START_OF_RX_MSB                        1
#define RXPCU_PPDU_END_INFO_COEX_BT_TX_AFTER_START_OF_RX_MASK                       0x00000002

#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_FROM_START_OF_RX_OFFSET                     0x0000000c
#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_FROM_START_OF_RX_LSB                        2
#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_FROM_START_OF_RX_MSB                        2
#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_FROM_START_OF_RX_MASK                       0x00000004

#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_AFTER_START_OF_RX_OFFSET                    0x0000000c
#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_AFTER_START_OF_RX_LSB                       3
#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_AFTER_START_OF_RX_MSB                       3
#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_AFTER_START_OF_RX_MASK                      0x00000008

#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_FROM_START_OF_RX_OFFSET                    0x0000000c
#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_FROM_START_OF_RX_LSB                       4
#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_FROM_START_OF_RX_MSB                       4
#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_FROM_START_OF_RX_MASK                      0x00000010

#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_AFTER_START_OF_RX_OFFSET                   0x0000000c
#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_AFTER_START_OF_RX_LSB                      5
#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_AFTER_START_OF_RX_MSB                      5
#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_AFTER_START_OF_RX_MASK                     0x00000020

#define RXPCU_PPDU_END_INFO_MPDU_DELIMITER_ERRORS_SEEN_OFFSET                       0x0000000c
#define RXPCU_PPDU_END_INFO_MPDU_DELIMITER_ERRORS_SEEN_LSB                          6
#define RXPCU_PPDU_END_INFO_MPDU_DELIMITER_ERRORS_SEEN_MSB                          6
#define RXPCU_PPDU_END_INFO_MPDU_DELIMITER_ERRORS_SEEN_MASK                         0x00000040

#define RXPCU_PPDU_END_INFO_DIALOG_TOKEN_OFFSET                                     0x0000000c
#define RXPCU_PPDU_END_INFO_DIALOG_TOKEN_LSB                                        9
#define RXPCU_PPDU_END_INFO_DIALOG_TOKEN_MSB                                        16
#define RXPCU_PPDU_END_INFO_DIALOG_TOKEN_MASK                                       0x0001fe00

#define RXPCU_PPDU_END_INFO_FOLLOW_UP_DIALOG_TOKEN_OFFSET                           0x0000000c
#define RXPCU_PPDU_END_INFO_FOLLOW_UP_DIALOG_TOKEN_LSB                              17
#define RXPCU_PPDU_END_INFO_FOLLOW_UP_DIALOG_TOKEN_MSB                              24
#define RXPCU_PPDU_END_INFO_FOLLOW_UP_DIALOG_TOKEN_MASK                             0x01fe0000

#define RXPCU_PPDU_END_INFO_BB_CAPTURED_CHANNEL_OFFSET                              0x0000000c
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_CHANNEL_LSB                                 25
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_CHANNEL_MSB                                 25
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_CHANNEL_MASK                                0x02000000

#define RXPCU_PPDU_END_INFO_BB_CAPTURED_REASON_OFFSET                               0x0000000c
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_REASON_LSB                                  26
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_REASON_MSB                                  28
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_REASON_MASK                                 0x1c000000

#define RXPCU_PPDU_END_INFO_BB_CAPTURED_TIMEOUT_OFFSET                              0x0000000c
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_TIMEOUT_LSB                                 29
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_TIMEOUT_MSB                                 29
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_TIMEOUT_MASK                                0x20000000

#define RXPCU_PPDU_END_INFO_COEX_UWB_TX_AFTER_START_OF_RX_OFFSET                    0x0000000c
#define RXPCU_PPDU_END_INFO_COEX_UWB_TX_AFTER_START_OF_RX_LSB                       30
#define RXPCU_PPDU_END_INFO_COEX_UWB_TX_AFTER_START_OF_RX_MSB                       30
#define RXPCU_PPDU_END_INFO_COEX_UWB_TX_AFTER_START_OF_RX_MASK                      0x40000000

#define RXPCU_PPDU_END_INFO_COEX_UWB_TX_FROM_START_OF_RX_OFFSET                     0x0000000c
#define RXPCU_PPDU_END_INFO_COEX_UWB_TX_FROM_START_OF_RX_LSB                        31
#define RXPCU_PPDU_END_INFO_COEX_UWB_TX_FROM_START_OF_RX_MSB                        31
#define RXPCU_PPDU_END_INFO_COEX_UWB_TX_FROM_START_OF_RX_MASK                       0x80000000

#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_PASSING_FCS_OFFSET                    0x00000010
#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_PASSING_FCS_LSB                       0
#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_PASSING_FCS_MSB                       9
#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_PASSING_FCS_MASK                      0x000003ff

#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_FAILING_FCS_OFFSET                    0x00000010
#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_FAILING_FCS_LSB                       10
#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_FAILING_FCS_MSB                       19
#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_FAILING_FCS_MASK                      0x000ffc00

#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_PASSING_FCS_OFFSET                     0x00000010
#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_PASSING_FCS_LSB                        20
#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_PASSING_FCS_MSB                        29
#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_PASSING_FCS_MASK                       0x3ff00000

#define RXPCU_PPDU_END_INFO_RESERVED_4_OFFSET                                       0x00000010
#define RXPCU_PPDU_END_INFO_RESERVED_4_LSB                                          30
#define RXPCU_PPDU_END_INFO_RESERVED_4_MSB                                          31
#define RXPCU_PPDU_END_INFO_RESERVED_4_MASK                                         0xc0000000

#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_FAILING_FCS_OFFSET                     0x00000014
#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_FAILING_FCS_LSB                        0
#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_FAILING_FCS_MSB                        9
#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_FAILING_FCS_MASK                       0x000003ff

#define RXPCU_PPDU_END_INFO_RESERVED_5_OFFSET                                       0x00000014
#define RXPCU_PPDU_END_INFO_RESERVED_5_LSB                                          10
#define RXPCU_PPDU_END_INFO_RESERVED_5_MSB                                          31
#define RXPCU_PPDU_END_INFO_RESERVED_5_MASK                                         0xfffffc00

#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_LOWER_32_OFFSET                        0x00000018
#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_LOWER_32_LSB                           0
#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_LOWER_32_MSB                           31
#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_LOWER_32_MASK                          0xffffffff

#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_UPPER_32_OFFSET                        0x0000001c
#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_UPPER_32_LSB                           0
#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_UPPER_32_MSB                           31
#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_UPPER_32_MASK                          0xffffffff

#define RXPCU_PPDU_END_INFO_BB_LENGTH_OFFSET                                        0x00000020
#define RXPCU_PPDU_END_INFO_BB_LENGTH_LSB                                           0
#define RXPCU_PPDU_END_INFO_BB_LENGTH_MSB                                           15
#define RXPCU_PPDU_END_INFO_BB_LENGTH_MASK                                          0x0000ffff

#define RXPCU_PPDU_END_INFO_BB_DATA_OFFSET                                          0x00000020
#define RXPCU_PPDU_END_INFO_BB_DATA_LSB                                             16
#define RXPCU_PPDU_END_INFO_BB_DATA_MSB                                             16
#define RXPCU_PPDU_END_INFO_BB_DATA_MASK                                            0x00010000

#define RXPCU_PPDU_END_INFO_RESERVED_8_OFFSET                                       0x00000020
#define RXPCU_PPDU_END_INFO_RESERVED_8_LSB                                          17
#define RXPCU_PPDU_END_INFO_RESERVED_8_MSB                                          19
#define RXPCU_PPDU_END_INFO_RESERVED_8_MASK                                         0x000e0000

#define RXPCU_PPDU_END_INFO_FIRST_BT_BROADCAST_STATUS_DETAILS_OFFSET                0x00000020
#define RXPCU_PPDU_END_INFO_FIRST_BT_BROADCAST_STATUS_DETAILS_LSB                   20
#define RXPCU_PPDU_END_INFO_FIRST_BT_BROADCAST_STATUS_DETAILS_MSB                   31
#define RXPCU_PPDU_END_INFO_FIRST_BT_BROADCAST_STATUS_DETAILS_MASK                  0xfff00000

#define RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_OFFSET                                 0x00000024
#define RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_LSB                                    0
#define RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_MSB                                    23
#define RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_MASK                                   0x00ffffff

#define RXPCU_PPDU_END_INFO_RESERVED_9_OFFSET                                       0x00000024
#define RXPCU_PPDU_END_INFO_RESERVED_9_LSB                                          24
#define RXPCU_PPDU_END_INFO_RESERVED_9_MSB                                          31
#define RXPCU_PPDU_END_INFO_RESERVED_9_MASK                                         0xff000000

#define RXPCU_PPDU_END_INFO_AST_INDEX_OFFSET                                        0x00000028
#define RXPCU_PPDU_END_INFO_AST_INDEX_LSB                                           0
#define RXPCU_PPDU_END_INFO_AST_INDEX_MSB                                           15
#define RXPCU_PPDU_END_INFO_AST_INDEX_MASK                                          0x0000ffff

#define RXPCU_PPDU_END_INFO_AST_INDEX_VALID_OFFSET                                  0x00000028
#define RXPCU_PPDU_END_INFO_AST_INDEX_VALID_LSB                                     16
#define RXPCU_PPDU_END_INFO_AST_INDEX_VALID_MSB                                     16
#define RXPCU_PPDU_END_INFO_AST_INDEX_VALID_MASK                                    0x00010000

#define RXPCU_PPDU_END_INFO_RESERVED_10_OFFSET                                      0x00000028
#define RXPCU_PPDU_END_INFO_RESERVED_10_LSB                                         17
#define RXPCU_PPDU_END_INFO_RESERVED_10_MSB                                         19
#define RXPCU_PPDU_END_INFO_RESERVED_10_MASK                                        0x000e0000

#define RXPCU_PPDU_END_INFO_SECOND_BT_BROADCAST_STATUS_DETAILS_OFFSET               0x00000028
#define RXPCU_PPDU_END_INFO_SECOND_BT_BROADCAST_STATUS_DETAILS_LSB                  20
#define RXPCU_PPDU_END_INFO_SECOND_BT_BROADCAST_STATUS_DETAILS_MSB                  31
#define RXPCU_PPDU_END_INFO_SECOND_BT_BROADCAST_STATUS_DETAILS_MASK                 0xfff00000

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_LSB 0
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_MSB 7
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_MASK 0x000000ff

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_LSB 8
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_MSB 8
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_MASK 0x00000100

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_LSB 9
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_MSB 9
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_MASK 0x00000200

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_GAIN_CHANGE_BY_MAIN_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_GAIN_CHANGE_BY_MAIN_LSB 10
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_GAIN_CHANGE_BY_MAIN_MSB 10
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_GAIN_CHANGE_BY_MAIN_MASK 0x00000400

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_GAIN_CHANGE_BY_BT_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_GAIN_CHANGE_BY_BT_LSB  11
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_GAIN_CHANGE_BY_BT_MSB  11
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_GAIN_CHANGE_BY_BT_MASK 0x00000800

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_MAIN_TX_INDICATION_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_MAIN_TX_INDICATION_LSB 12
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_MAIN_TX_INDICATION_MSB 12
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_MAIN_TX_INDICATION_MASK 0x00001000

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_BT_TX_INDICATION_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_BT_TX_INDICATION_LSB   13
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_BT_TX_INDICATION_MSB   13
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_BT_TX_INDICATION_MASK  0x00002000

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_CONCURRENT_MODE_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_CONCURRENT_MODE_LSB    14
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_CONCURRENT_MODE_MSB    14
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_CONCURRENT_MODE_MASK   0x00004000

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_OFFSET      0x0000002c
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_LSB         15
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_MSB         15
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_MASK        0x00008000

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_LSB   16
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_MSB   31
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_MASK  0xffff0000

#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_OFFSET 0x00000030
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_LSB 0
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_MSB 7
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_MASK 0x000000ff

#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_OFFSET      0x00000030
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_LSB         8
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_MSB         15
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_MASK        0x0000ff00

#define RXPCU_PPDU_END_INFO_PRE_BT_BROADCAST_STATUS_DETAILS_OFFSET                  0x00000030
#define RXPCU_PPDU_END_INFO_PRE_BT_BROADCAST_STATUS_DETAILS_LSB                     16
#define RXPCU_PPDU_END_INFO_PRE_BT_BROADCAST_STATUS_DETAILS_MSB                     27
#define RXPCU_PPDU_END_INFO_PRE_BT_BROADCAST_STATUS_DETAILS_MASK                    0x0fff0000

#define RXPCU_PPDU_END_INFO_RESERVED_12A_OFFSET                                     0x00000030
#define RXPCU_PPDU_END_INFO_RESERVED_12A_LSB                                        28
#define RXPCU_PPDU_END_INFO_RESERVED_12A_MSB                                        31
#define RXPCU_PPDU_END_INFO_RESERVED_12A_MASK                                       0xf0000000

#define RXPCU_PPDU_END_INFO_NON_QOS_SN_INFO_VALID_OFFSET                            0x00000034
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_INFO_VALID_LSB                               0
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_INFO_VALID_MSB                               0
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_INFO_VALID_MASK                              0x00000001

#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROTECTED_PPDU_OFFSET                       0x00000034
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROTECTED_PPDU_LSB                          1
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROTECTED_PPDU_MSB                          1
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROTECTED_PPDU_MASK                         0x00000002

#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_TYPE_OFFSET                            0x00000034
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_TYPE_LSB                               2
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_TYPE_MSB                               3
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_TYPE_MASK                              0x0000000c

#define RXPCU_PPDU_END_INFO_RESERVED_13A_OFFSET                                     0x00000034
#define RXPCU_PPDU_END_INFO_RESERVED_13A_LSB                                        4
#define RXPCU_PPDU_END_INFO_RESERVED_13A_MSB                                        5
#define RXPCU_PPDU_END_INFO_RESERVED_13A_MASK                                       0x00000030

#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_OFFSET                               0x00000034
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_LSB                                  6
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_MSB                                  17
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_MASK                                 0x0003ffc0

#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_RETRY_SETTING_OFFSET                 0x00000034
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_RETRY_SETTING_LSB                    18
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_RETRY_SETTING_MSB                    18
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_RETRY_SETTING_MASK                   0x00040000

#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_OFFSET                                0x00000034
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_LSB                                   19
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_MSB                                   30
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_MASK                                  0x7ff80000

#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_RETRY_SETTING_OFFSET                  0x00000034
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_RETRY_SETTING_LSB                     31
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_RETRY_SETTING_MSB                     31
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_RETRY_SETTING_MASK                    0x80000000

#define RXPCU_PPDU_END_INFO_QOS_SN_1_INFO_VALID_OFFSET                              0x00000038
#define RXPCU_PPDU_END_INFO_QOS_SN_1_INFO_VALID_LSB                                 0
#define RXPCU_PPDU_END_INFO_QOS_SN_1_INFO_VALID_MSB                                 0
#define RXPCU_PPDU_END_INFO_QOS_SN_1_INFO_VALID_MASK                                0x00000001

#define RXPCU_PPDU_END_INFO_RESERVED_14A_OFFSET                                     0x00000038
#define RXPCU_PPDU_END_INFO_RESERVED_14A_LSB                                        1
#define RXPCU_PPDU_END_INFO_RESERVED_14A_MSB                                        1
#define RXPCU_PPDU_END_INFO_RESERVED_14A_MASK                                       0x00000002

#define RXPCU_PPDU_END_INFO_QOS_SN_1_TID_OFFSET                                     0x00000038
#define RXPCU_PPDU_END_INFO_QOS_SN_1_TID_LSB                                        2
#define RXPCU_PPDU_END_INFO_QOS_SN_1_TID_MSB                                        5
#define RXPCU_PPDU_END_INFO_QOS_SN_1_TID_MASK                                       0x0000003c

#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_OFFSET                                 0x00000038
#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_LSB                                    6
#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_MSB                                    17
#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_MASK                                   0x0003ffc0

#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_RETRY_SETTING_OFFSET                   0x00000038
#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_RETRY_SETTING_LSB                      18
#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_RETRY_SETTING_MSB                      18
#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_RETRY_SETTING_MASK                     0x00040000

#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_OFFSET                                  0x00000038
#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_LSB                                     19
#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_MSB                                     30
#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_MASK                                    0x7ff80000

#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_RETRY_SETTING_OFFSET                    0x00000038
#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_RETRY_SETTING_LSB                       31
#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_RETRY_SETTING_MSB                       31
#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_RETRY_SETTING_MASK                      0x80000000

#define RXPCU_PPDU_END_INFO_QOS_SN_2_INFO_VALID_OFFSET                              0x0000003c
#define RXPCU_PPDU_END_INFO_QOS_SN_2_INFO_VALID_LSB                                 0
#define RXPCU_PPDU_END_INFO_QOS_SN_2_INFO_VALID_MSB                                 0
#define RXPCU_PPDU_END_INFO_QOS_SN_2_INFO_VALID_MASK                                0x00000001

#define RXPCU_PPDU_END_INFO_RESERVED_15A_OFFSET                                     0x0000003c
#define RXPCU_PPDU_END_INFO_RESERVED_15A_LSB                                        1
#define RXPCU_PPDU_END_INFO_RESERVED_15A_MSB                                        1
#define RXPCU_PPDU_END_INFO_RESERVED_15A_MASK                                       0x00000002

#define RXPCU_PPDU_END_INFO_QOS_SN_2_TID_OFFSET                                     0x0000003c
#define RXPCU_PPDU_END_INFO_QOS_SN_2_TID_LSB                                        2
#define RXPCU_PPDU_END_INFO_QOS_SN_2_TID_MSB                                        5
#define RXPCU_PPDU_END_INFO_QOS_SN_2_TID_MASK                                       0x0000003c

#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_OFFSET                                 0x0000003c
#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_LSB                                    6
#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_MSB                                    17
#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_MASK                                   0x0003ffc0

#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_RETRY_SETTING_OFFSET                   0x0000003c
#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_RETRY_SETTING_LSB                      18
#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_RETRY_SETTING_MSB                      18
#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_RETRY_SETTING_MASK                     0x00040000

#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_OFFSET                                  0x0000003c
#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_LSB                                     19
#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_MSB                                     30
#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_MASK                                    0x7ff80000

#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_RETRY_SETTING_OFFSET                    0x0000003c
#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_RETRY_SETTING_LSB                       31
#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_RETRY_SETTING_MSB                       31
#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_RETRY_SETTING_MASK                      0x80000000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_LEGACY_OFFSET_OFFSET 0x00000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_LEGACY_OFFSET_LSB    0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_LEGACY_OFFSET_MSB    1
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_LEGACY_OFFSET_MASK   0x00000003

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_A_OFFSET_OFFSET     0x00000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_A_OFFSET_LSB        2
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_A_OFFSET_MSB        7
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_A_OFFSET_MASK       0x000000fc

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_B_OFFSET_OFFSET     0x00000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_B_OFFSET_LSB        8
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_B_OFFSET_MSB        13
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_B_OFFSET_MASK       0x00003f00

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HT_SIG_OFFSET_OFFSET      0x00000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HT_SIG_OFFSET_LSB         14
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HT_SIG_OFFSET_MSB         19
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HT_SIG_OFFSET_MASK        0x000fc000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_A_OFFSET_OFFSET   0x00000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_A_OFFSET_LSB      20
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_A_OFFSET_MSB      25
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_A_OFFSET_MASK     0x03f00000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_REPEAT_L_SIG_A_OFFSET_OFFSET 0x00000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_REPEAT_L_SIG_A_OFFSET_LSB 26
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_REPEAT_L_SIG_A_OFFSET_MSB 31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_REPEAT_L_SIG_A_OFFSET_MASK 0xfc000000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_SU_OFFSET_OFFSET 0x00000044
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_SU_OFFSET_LSB    0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_SU_OFFSET_MSB    5
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_SU_OFFSET_MASK   0x0000003f

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_DL_OFFSET_OFFSET 0x00000044
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_DL_OFFSET_LSB 6
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_DL_OFFSET_MSB 11
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_DL_OFFSET_MASK 0x00000fc0

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_UL_OFFSET_OFFSET 0x00000044
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_UL_OFFSET_LSB 12
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_UL_OFFSET_MSB 17
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_UL_OFFSET_MASK 0x0003f000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERIC_U_SIG_OFFSET_OFFSET 0x00000044
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERIC_U_SIG_OFFSET_LSB  18
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERIC_U_SIG_OFFSET_MSB  23
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERIC_U_SIG_OFFSET_MASK 0x00fc0000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_HT_OFFSET_OFFSET     0x00000044
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_HT_OFFSET_LSB        24
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_HT_OFFSET_MSB        30
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_HT_OFFSET_MASK       0x7f000000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_1A_OFFSET        0x00000044
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_1A_LSB           31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_1A_MSB           31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_1A_MASK          0x80000000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU20_OFFSET_OFFSET 0x00000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU20_OFFSET_LSB 0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU20_OFFSET_MSB 6
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU20_OFFSET_MASK 0x0000007f

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU40_OFFSET_OFFSET 0x00000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU40_OFFSET_LSB 7
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU40_OFFSET_MSB 13
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU40_OFFSET_MASK 0x00003f80

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU80_OFFSET_OFFSET 0x00000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU80_OFFSET_LSB 14
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU80_OFFSET_MSB 20
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU80_OFFSET_MASK 0x001fc000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU160_OFFSET_OFFSET 0x00000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU160_OFFSET_LSB 21
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU160_OFFSET_MSB 27
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU160_OFFSET_MASK 0x0fe00000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_2A_OFFSET        0x00000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_2A_LSB           28
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_2A_MSB           31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_2A_MASK          0xf0000000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU20_OFFSET_OFFSET 0x0000004c
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU20_OFFSET_LSB 0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU20_OFFSET_MSB 6
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU20_OFFSET_MASK 0x0000007f

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU40_OFFSET_OFFSET 0x0000004c
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU40_OFFSET_LSB 7
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU40_OFFSET_MSB 13
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU40_OFFSET_MASK 0x00003f80

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU80_OFFSET_OFFSET 0x0000004c
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU80_OFFSET_LSB 14
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU80_OFFSET_MSB 20
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU80_OFFSET_MASK 0x001fc000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU160_OFFSET_OFFSET 0x0000004c
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU160_OFFSET_LSB 21
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU160_OFFSET_MSB 27
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU160_OFFSET_MASK 0x0fe00000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_3A_OFFSET        0x0000004c
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_3A_LSB           28
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_3A_MSB           31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_3A_MASK          0xf0000000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B1_MU_OFFSET_OFFSET 0x00000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B1_MU_OFFSET_LSB   0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B1_MU_OFFSET_MSB   6
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B1_MU_OFFSET_MASK  0x0000007f

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_MU_OFFSET_OFFSET 0x00000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_MU_OFFSET_LSB   7
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_MU_OFFSET_MSB   13
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_MU_OFFSET_MASK  0x00003f80

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_OFDMA_OFFSET_OFFSET 0x00000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_OFDMA_OFFSET_LSB 14
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_OFDMA_OFFSET_MSB 20
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_OFDMA_OFFSET_MASK 0x001fc000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_GENERIC_EHT_SIG_OFFSET_OFFSET 0x00000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_GENERIC_EHT_SIG_OFFSET_LSB 21
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_GENERIC_EHT_SIG_OFFSET_MSB 27
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_GENERIC_EHT_SIG_OFFSET_MASK 0x0fe00000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_GENERIC_EHT_SIG_INCLUDED_OFFSET 0x00000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_GENERIC_EHT_SIG_INCLUDED_LSB 28
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_GENERIC_EHT_SIG_INCLUDED_MSB 28
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_GENERIC_EHT_SIG_INCLUDED_MASK 0x10000000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_4A_OFFSET        0x00000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_4A_LSB           29
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_4A_MSB           31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_4A_MASK          0xe0000000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_COMMON_USER_INFO_OFFSET_OFFSET 0x00000054
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_COMMON_USER_INFO_OFFSET_LSB 0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_COMMON_USER_INFO_OFFSET_MSB 6
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_COMMON_USER_INFO_OFFSET_MASK 0x0000007f

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_DEBUG_INFO_OFFSET_OFFSET 0x00000054
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_DEBUG_INFO_OFFSET_LSB 7
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_DEBUG_INFO_OFFSET_MSB 14
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_DEBUG_INFO_OFFSET_MASK 0x00007f80

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_DEBUG_INFO_INCLUDED_OFFSET 0x00000054
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_DEBUG_INFO_INCLUDED_LSB 15
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_DEBUG_INFO_INCLUDED_MSB 15
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_DEBUG_INFO_INCLUDED_MASK 0x00008000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_OTHER_RECEIVE_INFO_OFFSET_OFFSET 0x00000054
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_OTHER_RECEIVE_INFO_OFFSET_LSB 16
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_OTHER_RECEIVE_INFO_OFFSET_MSB 23
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_OTHER_RECEIVE_INFO_OFFSET_MASK 0x00ff0000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_OTHER_RECEIVE_INFO_INCLUDED_OFFSET 0x00000054
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_OTHER_RECEIVE_INFO_INCLUDED_LSB 24
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_OTHER_RECEIVE_INFO_INCLUDED_MSB 24
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_OTHER_RECEIVE_INFO_INCLUDED_MASK 0x01000000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_5A_OFFSET        0x00000054
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_5A_LSB           25
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_5A_MSB           31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_5A_MASK          0xfe000000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_DATA_DONE_OFFSET_OFFSET   0x00000058
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_DATA_DONE_OFFSET_LSB      0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_DATA_DONE_OFFSET_MSB      7
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_DATA_DONE_OFFSET_MASK     0x000000ff

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERATED_CBF_DETAILS_OFFSET_OFFSET 0x00000058
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERATED_CBF_DETAILS_OFFSET_LSB 8
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERATED_CBF_DETAILS_OFFSET_MSB 15
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERATED_CBF_DETAILS_OFFSET_MASK 0x0000ff00

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_PART1_OFFSET_OFFSET 0x00000058
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_PART1_OFFSET_LSB  16
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_PART1_OFFSET_MSB  23
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_PART1_OFFSET_MASK 0x00ff0000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_LOCATION_OFFSET_OFFSET    0x00000058
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_LOCATION_OFFSET_LSB       24
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_LOCATION_OFFSET_MSB       31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_LOCATION_OFFSET_MASK      0xff000000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_OFFSET_OFFSET     0x0000005c
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_OFFSET_LSB        8
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_OFFSET_MSB        15
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_OFFSET_MASK       0x0000ff00

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_ABORT_REQUEST_ACK_OFFSET_OFFSET 0x0000005c
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_ABORT_REQUEST_ACK_OFFSET_LSB 16
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_ABORT_REQUEST_ACK_OFFSET_MSB 23
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_ABORT_REQUEST_ACK_OFFSET_MASK 0x00ff0000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_7A_OFFSET        0x0000005c
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_7A_LSB           24
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_7A_MSB           31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_7A_MASK          0xff000000

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_8A_OFFSET        0x00000060
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_8A_LSB           0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_8A_MSB           31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_8A_MASK          0xffffffff

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_9A_OFFSET        0x00000064
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_9A_LSB           0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_9A_MSB           31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_9A_MASK          0xffffffff

#define RXPCU_PPDU_END_INFO_CORRUPTED_DUE_TO_FIFO_DELAY_OFFSET                      0x00000068
#define RXPCU_PPDU_END_INFO_CORRUPTED_DUE_TO_FIFO_DELAY_LSB                         0
#define RXPCU_PPDU_END_INFO_CORRUPTED_DUE_TO_FIFO_DELAY_MSB                         0
#define RXPCU_PPDU_END_INFO_CORRUPTED_DUE_TO_FIFO_DELAY_MASK                        0x00000001

#define RXPCU_PPDU_END_INFO_QOS_SN_1_MORE_FRAG_STATE_OFFSET                         0x00000068
#define RXPCU_PPDU_END_INFO_QOS_SN_1_MORE_FRAG_STATE_LSB                            1
#define RXPCU_PPDU_END_INFO_QOS_SN_1_MORE_FRAG_STATE_MSB                            1
#define RXPCU_PPDU_END_INFO_QOS_SN_1_MORE_FRAG_STATE_MASK                           0x00000002

#define RXPCU_PPDU_END_INFO_QOS_SN_1_FRAG_NUM_STATE_OFFSET                          0x00000068
#define RXPCU_PPDU_END_INFO_QOS_SN_1_FRAG_NUM_STATE_LSB                             2
#define RXPCU_PPDU_END_INFO_QOS_SN_1_FRAG_NUM_STATE_MSB                             5
#define RXPCU_PPDU_END_INFO_QOS_SN_1_FRAG_NUM_STATE_MASK                            0x0000003c

#define RXPCU_PPDU_END_INFO_QOS_SN_2_MORE_FRAG_STATE_OFFSET                         0x00000068
#define RXPCU_PPDU_END_INFO_QOS_SN_2_MORE_FRAG_STATE_LSB                            6
#define RXPCU_PPDU_END_INFO_QOS_SN_2_MORE_FRAG_STATE_MSB                            6
#define RXPCU_PPDU_END_INFO_QOS_SN_2_MORE_FRAG_STATE_MASK                           0x00000040

#define RXPCU_PPDU_END_INFO_QOS_SN_2_FRAG_NUM_STATE_OFFSET                          0x00000068
#define RXPCU_PPDU_END_INFO_QOS_SN_2_FRAG_NUM_STATE_LSB                             7
#define RXPCU_PPDU_END_INFO_QOS_SN_2_FRAG_NUM_STATE_MSB                             10
#define RXPCU_PPDU_END_INFO_QOS_SN_2_FRAG_NUM_STATE_MASK                            0x00000780

#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_NON_11A_OFFSET                         0x00000068
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_NON_11A_LSB                            11
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_NON_11A_MSB                            11
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_NON_11A_MASK                           0x00000800

#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_RATE_MCS_OFFSET                        0x00000068
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_RATE_MCS_LSB                           12
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_RATE_MCS_MSB                           15
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_RATE_MCS_MASK                          0x0000f000

#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_PEER_ADDR_15_0_OFFSET                  0x00000068
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_PEER_ADDR_15_0_LSB                     16
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_PEER_ADDR_15_0_MSB                     31
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_PEER_ADDR_15_0_MASK                    0xffff0000

#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_PEER_ADDR_47_16_OFFSET                 0x0000006c
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_PEER_ADDR_47_16_LSB                    0
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_PEER_ADDR_47_16_MSB                    31
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_PROT_PEER_ADDR_47_16_MASK                   0xffffffff

#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_RX_COUNT_OFFSET                             0x00000070
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_RX_COUNT_LSB                                0
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_RX_COUNT_MSB                                31
#define RXPCU_PPDU_END_INFO_RTS_OR_TRIG_RX_COUNT_MASK                               0xffffffff

#define RXPCU_PPDU_END_INFO_CTS_OR_NULL_TX_COUNT_OFFSET                             0x00000074
#define RXPCU_PPDU_END_INFO_CTS_OR_NULL_TX_COUNT_LSB                                0
#define RXPCU_PPDU_END_INFO_CTS_OR_NULL_TX_COUNT_MSB                                31
#define RXPCU_PPDU_END_INFO_CTS_OR_NULL_TX_COUNT_MASK                               0xffffffff

#define RXPCU_PPDU_END_INFO_RX_PPDU_END_MARKER_OFFSET                               0x00000078
#define RXPCU_PPDU_END_INFO_RX_PPDU_END_MARKER_LSB                                  0
#define RXPCU_PPDU_END_INFO_RX_PPDU_END_MARKER_MSB                                  31
#define RXPCU_PPDU_END_INFO_RX_PPDU_END_MARKER_MASK                                 0xffffffff

#endif
