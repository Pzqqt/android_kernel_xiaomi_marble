
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



#ifndef _RXPCU_PPDU_END_INFO_H_
#define _RXPCU_PPDU_END_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#include "phyrx_abort_request_info.h"
#include "macrx_abort_request_info.h"

#define NUM_OF_DWORDS_RXPCU_PPDU_END_INFO 14

struct rxpcu_ppdu_end_info {
             uint32_t wb_timestamp_lower_32           : 32;
             uint32_t wb_timestamp_upper_32           : 32;
             uint32_t rx_antenna                      : 24,
                      tx_ht_vht_ack                   :  1,
                      unsupported_mu_nc               :  1,
                      otp_txbf_disable                :  1,
                      previous_tlv_corrupted          :  1,
                      phyrx_abort_request_info_valid  :  1,
                      macrx_abort_request_info_valid  :  1,
                      reserved                        :  2;
             uint32_t coex_bt_tx_from_start_of_rx     :  1,
                      coex_bt_tx_after_start_of_rx    :  1,
                      coex_wan_tx_from_start_of_rx    :  1,
                      coex_wan_tx_after_start_of_rx   :  1,
                      coex_wlan_tx_from_start_of_rx   :  1,
                      coex_wlan_tx_after_start_of_rx  :  1,
                      mpdu_delimiter_errors_seen      :  1,
                      __reserved_g_0012                          :  2,
                      dialog_token                    :  8,
                      follow_up_dialog_token          :  8,
                      bb_captured_channel             :  1,
                      bb_captured_reason              :  3,
                      bb_captured_timeout             :  1,
                      reserved_3                      :  2;
             uint32_t before_mpdu_count_passing_fcs   : 10,
                      before_mpdu_count_failing_fcs   : 10,
                      after_mpdu_count_passing_fcs    : 10,
                      reserved_4                      :  2;
             uint32_t after_mpdu_count_failing_fcs    : 10,
                      reserved_5                      : 22;
             uint32_t phy_timestamp_tx_lower_32       : 32;
             uint32_t phy_timestamp_tx_upper_32       : 32;
             uint32_t bb_length                       : 16,
                      bb_data                         :  1,
                      reserved_8                      :  3,
                      first_bt_broadcast_status_details: 12;
             uint32_t rx_ppdu_duration                : 24,
                      reserved_9                      :  8;
             uint32_t ast_index                       : 16,
                      ast_index_valid                 :  1,
                      reserved_10                     :  3,
                      second_bt_broadcast_status_details: 12;
    struct            phyrx_abort_request_info                       phyrx_abort_request_info_details;
    struct            macrx_abort_request_info                       macrx_abort_request_info_details;
             uint16_t pre_bt_broadcast_status_details : 12,
                      reserved_12a                    :  4;
             uint32_t rx_ppdu_end_marker              : 32;
};

#define RXPCU_PPDU_END_INFO_0_WB_TIMESTAMP_LOWER_32_OFFSET           0x00000000
#define RXPCU_PPDU_END_INFO_0_WB_TIMESTAMP_LOWER_32_LSB              0
#define RXPCU_PPDU_END_INFO_0_WB_TIMESTAMP_LOWER_32_MASK             0xffffffff

#define RXPCU_PPDU_END_INFO_1_WB_TIMESTAMP_UPPER_32_OFFSET           0x00000004
#define RXPCU_PPDU_END_INFO_1_WB_TIMESTAMP_UPPER_32_LSB              0
#define RXPCU_PPDU_END_INFO_1_WB_TIMESTAMP_UPPER_32_MASK             0xffffffff

#define RXPCU_PPDU_END_INFO_2_RX_ANTENNA_OFFSET                      0x00000008
#define RXPCU_PPDU_END_INFO_2_RX_ANTENNA_LSB                         0
#define RXPCU_PPDU_END_INFO_2_RX_ANTENNA_MASK                        0x00ffffff

#define RXPCU_PPDU_END_INFO_2_TX_HT_VHT_ACK_OFFSET                   0x00000008
#define RXPCU_PPDU_END_INFO_2_TX_HT_VHT_ACK_LSB                      24
#define RXPCU_PPDU_END_INFO_2_TX_HT_VHT_ACK_MASK                     0x01000000

#define RXPCU_PPDU_END_INFO_2_UNSUPPORTED_MU_NC_OFFSET               0x00000008
#define RXPCU_PPDU_END_INFO_2_UNSUPPORTED_MU_NC_LSB                  25
#define RXPCU_PPDU_END_INFO_2_UNSUPPORTED_MU_NC_MASK                 0x02000000

#define RXPCU_PPDU_END_INFO_2_OTP_TXBF_DISABLE_OFFSET                0x00000008
#define RXPCU_PPDU_END_INFO_2_OTP_TXBF_DISABLE_LSB                   26
#define RXPCU_PPDU_END_INFO_2_OTP_TXBF_DISABLE_MASK                  0x04000000

#define RXPCU_PPDU_END_INFO_2_PREVIOUS_TLV_CORRUPTED_OFFSET          0x00000008
#define RXPCU_PPDU_END_INFO_2_PREVIOUS_TLV_CORRUPTED_LSB             27
#define RXPCU_PPDU_END_INFO_2_PREVIOUS_TLV_CORRUPTED_MASK            0x08000000

#define RXPCU_PPDU_END_INFO_2_PHYRX_ABORT_REQUEST_INFO_VALID_OFFSET  0x00000008
#define RXPCU_PPDU_END_INFO_2_PHYRX_ABORT_REQUEST_INFO_VALID_LSB     28
#define RXPCU_PPDU_END_INFO_2_PHYRX_ABORT_REQUEST_INFO_VALID_MASK    0x10000000

#define RXPCU_PPDU_END_INFO_2_MACRX_ABORT_REQUEST_INFO_VALID_OFFSET  0x00000008
#define RXPCU_PPDU_END_INFO_2_MACRX_ABORT_REQUEST_INFO_VALID_LSB     29
#define RXPCU_PPDU_END_INFO_2_MACRX_ABORT_REQUEST_INFO_VALID_MASK    0x20000000

#define RXPCU_PPDU_END_INFO_2_RESERVED_OFFSET                        0x00000008
#define RXPCU_PPDU_END_INFO_2_RESERVED_LSB                           30
#define RXPCU_PPDU_END_INFO_2_RESERVED_MASK                          0xc0000000

#define RXPCU_PPDU_END_INFO_3_COEX_BT_TX_FROM_START_OF_RX_OFFSET     0x0000000c
#define RXPCU_PPDU_END_INFO_3_COEX_BT_TX_FROM_START_OF_RX_LSB        0
#define RXPCU_PPDU_END_INFO_3_COEX_BT_TX_FROM_START_OF_RX_MASK       0x00000001

#define RXPCU_PPDU_END_INFO_3_COEX_BT_TX_AFTER_START_OF_RX_OFFSET    0x0000000c
#define RXPCU_PPDU_END_INFO_3_COEX_BT_TX_AFTER_START_OF_RX_LSB       1
#define RXPCU_PPDU_END_INFO_3_COEX_BT_TX_AFTER_START_OF_RX_MASK      0x00000002

#define RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_FROM_START_OF_RX_OFFSET    0x0000000c
#define RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_FROM_START_OF_RX_LSB       2
#define RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_FROM_START_OF_RX_MASK      0x00000004

#define RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_AFTER_START_OF_RX_OFFSET   0x0000000c
#define RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_AFTER_START_OF_RX_LSB      3
#define RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_AFTER_START_OF_RX_MASK     0x00000008

#define RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_FROM_START_OF_RX_OFFSET   0x0000000c
#define RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_FROM_START_OF_RX_LSB      4
#define RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_FROM_START_OF_RX_MASK     0x00000010

#define RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_AFTER_START_OF_RX_OFFSET  0x0000000c
#define RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_AFTER_START_OF_RX_LSB     5
#define RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_AFTER_START_OF_RX_MASK    0x00000020

#define RXPCU_PPDU_END_INFO_3_MPDU_DELIMITER_ERRORS_SEEN_OFFSET      0x0000000c
#define RXPCU_PPDU_END_INFO_3_MPDU_DELIMITER_ERRORS_SEEN_LSB         6
#define RXPCU_PPDU_END_INFO_3_MPDU_DELIMITER_ERRORS_SEEN_MASK        0x00000040

#define RXPCU_PPDU_END_INFO_3_DIALOG_TOKEN_OFFSET                    0x0000000c
#define RXPCU_PPDU_END_INFO_3_DIALOG_TOKEN_LSB                       9
#define RXPCU_PPDU_END_INFO_3_DIALOG_TOKEN_MASK                      0x0001fe00

#define RXPCU_PPDU_END_INFO_3_FOLLOW_UP_DIALOG_TOKEN_OFFSET          0x0000000c
#define RXPCU_PPDU_END_INFO_3_FOLLOW_UP_DIALOG_TOKEN_LSB             17
#define RXPCU_PPDU_END_INFO_3_FOLLOW_UP_DIALOG_TOKEN_MASK            0x01fe0000

#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_CHANNEL_OFFSET             0x0000000c
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_CHANNEL_LSB                25
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_CHANNEL_MASK               0x02000000

#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_REASON_OFFSET              0x0000000c
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_REASON_LSB                 26
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_REASON_MASK                0x1c000000

#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_TIMEOUT_OFFSET             0x0000000c
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_TIMEOUT_LSB                29
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_TIMEOUT_MASK               0x20000000

#define RXPCU_PPDU_END_INFO_3_RESERVED_3_OFFSET                      0x0000000c
#define RXPCU_PPDU_END_INFO_3_RESERVED_3_LSB                         30
#define RXPCU_PPDU_END_INFO_3_RESERVED_3_MASK                        0xc0000000

#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_PASSING_FCS_OFFSET   0x00000010
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_PASSING_FCS_LSB      0
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_PASSING_FCS_MASK     0x000003ff

#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_FAILING_FCS_OFFSET   0x00000010
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_FAILING_FCS_LSB      10
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_FAILING_FCS_MASK     0x000ffc00

#define RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_PASSING_FCS_OFFSET    0x00000010
#define RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_PASSING_FCS_LSB       20
#define RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_PASSING_FCS_MASK      0x3ff00000

#define RXPCU_PPDU_END_INFO_4_RESERVED_4_OFFSET                      0x00000010
#define RXPCU_PPDU_END_INFO_4_RESERVED_4_LSB                         30
#define RXPCU_PPDU_END_INFO_4_RESERVED_4_MASK                        0xc0000000

#define RXPCU_PPDU_END_INFO_5_AFTER_MPDU_COUNT_FAILING_FCS_OFFSET    0x00000014
#define RXPCU_PPDU_END_INFO_5_AFTER_MPDU_COUNT_FAILING_FCS_LSB       0
#define RXPCU_PPDU_END_INFO_5_AFTER_MPDU_COUNT_FAILING_FCS_MASK      0x000003ff

#define RXPCU_PPDU_END_INFO_5_RESERVED_5_OFFSET                      0x00000014
#define RXPCU_PPDU_END_INFO_5_RESERVED_5_LSB                         10
#define RXPCU_PPDU_END_INFO_5_RESERVED_5_MASK                        0xfffffc00

#define RXPCU_PPDU_END_INFO_6_PHY_TIMESTAMP_TX_LOWER_32_OFFSET       0x00000018
#define RXPCU_PPDU_END_INFO_6_PHY_TIMESTAMP_TX_LOWER_32_LSB          0
#define RXPCU_PPDU_END_INFO_6_PHY_TIMESTAMP_TX_LOWER_32_MASK         0xffffffff

#define RXPCU_PPDU_END_INFO_7_PHY_TIMESTAMP_TX_UPPER_32_OFFSET       0x0000001c
#define RXPCU_PPDU_END_INFO_7_PHY_TIMESTAMP_TX_UPPER_32_LSB          0
#define RXPCU_PPDU_END_INFO_7_PHY_TIMESTAMP_TX_UPPER_32_MASK         0xffffffff

#define RXPCU_PPDU_END_INFO_8_BB_LENGTH_OFFSET                       0x00000020
#define RXPCU_PPDU_END_INFO_8_BB_LENGTH_LSB                          0
#define RXPCU_PPDU_END_INFO_8_BB_LENGTH_MASK                         0x0000ffff

#define RXPCU_PPDU_END_INFO_8_BB_DATA_OFFSET                         0x00000020
#define RXPCU_PPDU_END_INFO_8_BB_DATA_LSB                            16
#define RXPCU_PPDU_END_INFO_8_BB_DATA_MASK                           0x00010000

#define RXPCU_PPDU_END_INFO_8_RESERVED_8_OFFSET                      0x00000020
#define RXPCU_PPDU_END_INFO_8_RESERVED_8_LSB                         17
#define RXPCU_PPDU_END_INFO_8_RESERVED_8_MASK                        0x000e0000

#define RXPCU_PPDU_END_INFO_8_FIRST_BT_BROADCAST_STATUS_DETAILS_OFFSET 0x00000020
#define RXPCU_PPDU_END_INFO_8_FIRST_BT_BROADCAST_STATUS_DETAILS_LSB  20
#define RXPCU_PPDU_END_INFO_8_FIRST_BT_BROADCAST_STATUS_DETAILS_MASK 0xfff00000

#define RXPCU_PPDU_END_INFO_9_RX_PPDU_DURATION_OFFSET                0x00000024
#define RXPCU_PPDU_END_INFO_9_RX_PPDU_DURATION_LSB                   0
#define RXPCU_PPDU_END_INFO_9_RX_PPDU_DURATION_MASK                  0x00ffffff

#define RXPCU_PPDU_END_INFO_9_RESERVED_9_OFFSET                      0x00000024
#define RXPCU_PPDU_END_INFO_9_RESERVED_9_LSB                         24
#define RXPCU_PPDU_END_INFO_9_RESERVED_9_MASK                        0xff000000

#define RXPCU_PPDU_END_INFO_10_AST_INDEX_OFFSET                      0x00000028
#define RXPCU_PPDU_END_INFO_10_AST_INDEX_LSB                         0
#define RXPCU_PPDU_END_INFO_10_AST_INDEX_MASK                        0x0000ffff

#define RXPCU_PPDU_END_INFO_10_AST_INDEX_VALID_OFFSET                0x00000028
#define RXPCU_PPDU_END_INFO_10_AST_INDEX_VALID_LSB                   16
#define RXPCU_PPDU_END_INFO_10_AST_INDEX_VALID_MASK                  0x00010000

#define RXPCU_PPDU_END_INFO_10_RESERVED_10_OFFSET                    0x00000028
#define RXPCU_PPDU_END_INFO_10_RESERVED_10_LSB                       17
#define RXPCU_PPDU_END_INFO_10_RESERVED_10_MASK                      0x000e0000

#define RXPCU_PPDU_END_INFO_10_SECOND_BT_BROADCAST_STATUS_DETAILS_OFFSET 0x00000028
#define RXPCU_PPDU_END_INFO_10_SECOND_BT_BROADCAST_STATUS_DETAILS_LSB 20
#define RXPCU_PPDU_END_INFO_10_SECOND_BT_BROADCAST_STATUS_DETAILS_MASK 0xfff00000

#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_LSB 0
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_MASK 0x000000ff

#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_LSB 8
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_MASK 0x00000100

#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_LSB 9
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_MASK 0x00000200

#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_LSB 10
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_MASK 0x0000fc00

#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_LSB 16
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_MASK 0xffff0000

#define RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_OFFSET 0x00000030
#define RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_LSB 0
#define RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_MASK 0x000000ff

#define RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_OFFSET 0x00000030
#define RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_LSB 8
#define RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_MASK 0x0000ff00

#define RXPCU_PPDU_END_INFO_13_RX_PPDU_END_MARKER_OFFSET             0x00000034
#define RXPCU_PPDU_END_INFO_13_RX_PPDU_END_MARKER_LSB                0
#define RXPCU_PPDU_END_INFO_13_RX_PPDU_END_MARKER_MASK               0xffffffff

#endif
