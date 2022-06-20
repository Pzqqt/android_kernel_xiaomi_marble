
/* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
#if !defined(__ASSEMBLER__)
#endif

#include "phyrx_abort_request_info.h"
#include "macrx_abort_request_info.h"
#include "rxpcu_ppdu_end_layout_info.h"
#define NUM_OF_DWORDS_RXPCU_PPDU_END_INFO 28

#define NUM_OF_QWORDS_RXPCU_PPDU_END_INFO 14


struct rxpcu_ppdu_end_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t wb_timestamp_lower_32                                   : 32; // [31:0]
             uint32_t wb_timestamp_upper_32                                   : 32; // [31:0]
             uint32_t rx_antenna                                              : 24, // [23:0]
                      tx_ht_vht_ack                                           :  1, // [24:24]
                      unsupported_mu_nc                                       :  1, // [25:25]
                      otp_txbf_disable                                        :  1, // [26:26]
                      previous_tlv_corrupted                                  :  1, // [27:27]
                      phyrx_abort_request_info_valid                          :  1, // [28:28]
                      macrx_abort_request_info_valid                          :  1, // [29:29]
                      reserved                                                :  2; // [31:30]
             uint32_t coex_bt_tx_from_start_of_rx                             :  1, // [0:0]
                      coex_bt_tx_after_start_of_rx                            :  1, // [1:1]
                      coex_wan_tx_from_start_of_rx                            :  1, // [2:2]
                      coex_wan_tx_after_start_of_rx                           :  1, // [3:3]
                      coex_wlan_tx_from_start_of_rx                           :  1, // [4:4]
                      coex_wlan_tx_after_start_of_rx                          :  1, // [5:5]
                      mpdu_delimiter_errors_seen                              :  1, // [6:6]
                      ftm_tm                                                  :  2, // [8:7]
                      dialog_token                                            :  8, // [16:9]
                      follow_up_dialog_token                                  :  8, // [24:17]
                      bb_captured_channel                                     :  1, // [25:25]
                      bb_captured_reason                                      :  3, // [28:26]
                      bb_captured_timeout                                     :  1, // [29:29]
                      reserved_3                                              :  2; // [31:30]
             uint32_t before_mpdu_count_passing_fcs                           : 10, // [9:0]
                      before_mpdu_count_failing_fcs                           : 10, // [19:10]
                      after_mpdu_count_passing_fcs                            : 10, // [29:20]
                      reserved_4                                              :  2; // [31:30]
             uint32_t after_mpdu_count_failing_fcs                            : 10, // [9:0]
                      reserved_5                                              : 22; // [31:10]
             uint32_t phy_timestamp_tx_lower_32                               : 32; // [31:0]
             uint32_t phy_timestamp_tx_upper_32                               : 32; // [31:0]
             uint32_t bb_length                                               : 16, // [15:0]
                      bb_data                                                 :  1, // [16:16]
                      reserved_8                                              :  3, // [19:17]
                      first_bt_broadcast_status_details                       : 12; // [31:20]
             uint32_t rx_ppdu_duration                                        : 24, // [23:0]
                      reserved_9                                              :  8; // [31:24]
             uint32_t ast_index                                               : 16, // [15:0]
                      ast_index_valid                                         :  1, // [16:16]
                      reserved_10                                             :  3, // [19:17]
                      second_bt_broadcast_status_details                      : 12; // [31:20]
             struct   phyrx_abort_request_info                                  phyrx_abort_request_info_details;
             struct   macrx_abort_request_info                                  macrx_abort_request_info_details;
             uint16_t pre_bt_broadcast_status_details                         : 12, // [27:16]
                      reserved_12a                                            :  4; // [31:28]
             uint32_t non_qos_sn_info_valid                                   :  1, // [0:0]
                      reserved_13a                                            :  5, // [5:1]
                      non_qos_sn_highest                                      : 12, // [17:6]
                      non_qos_sn_highest_retry_setting                        :  1, // [18:18]
                      non_qos_sn_lowest                                       : 12, // [30:19]
                      non_qos_sn_lowest_retry_setting                         :  1; // [31:31]
             uint32_t qos_sn_1_info_valid                                     :  1, // [0:0]
                      reserved_14a                                            :  1, // [1:1]
                      qos_sn_1_tid                                            :  4, // [5:2]
                      qos_sn_1_highest                                        : 12, // [17:6]
                      qos_sn_1_highest_retry_setting                          :  1, // [18:18]
                      qos_sn_1_lowest                                         : 12, // [30:19]
                      qos_sn_1_lowest_retry_setting                           :  1; // [31:31]
             uint32_t qos_sn_2_info_valid                                     :  1, // [0:0]
                      reserved_15a                                            :  1, // [1:1]
                      qos_sn_2_tid                                            :  4, // [5:2]
                      qos_sn_2_highest                                        : 12, // [17:6]
                      qos_sn_2_highest_retry_setting                          :  1, // [18:18]
                      qos_sn_2_lowest                                         : 12, // [30:19]
                      qos_sn_2_lowest_retry_setting                           :  1; // [31:31]
             struct   rxpcu_ppdu_end_layout_info                                rxpcu_ppdu_end_layout_details;
             uint32_t corrupted_due_to_fifo_delay                             :  1, // [0:0]
                      qos_sn_1_more_frag_state                                :  1, // [1:1]
                      qos_sn_1_frag_num_state                                 :  4, // [5:2]
                      qos_sn_2_more_frag_state                                :  1, // [6:6]
                      qos_sn_2_frag_num_state                                 :  4, // [10:7]
                      reserved_26a                                            : 21; // [31:11]
             uint32_t rx_ppdu_end_marker                                      : 32; // [31:0]
#else
             uint32_t wb_timestamp_lower_32                                   : 32; // [31:0]
             uint32_t wb_timestamp_upper_32                                   : 32; // [31:0]
             uint32_t reserved                                                :  2, // [31:30]
                      macrx_abort_request_info_valid                          :  1, // [29:29]
                      phyrx_abort_request_info_valid                          :  1, // [28:28]
                      previous_tlv_corrupted                                  :  1, // [27:27]
                      otp_txbf_disable                                        :  1, // [26:26]
                      unsupported_mu_nc                                       :  1, // [25:25]
                      tx_ht_vht_ack                                           :  1, // [24:24]
                      rx_antenna                                              : 24; // [23:0]
             uint32_t reserved_3                                              :  2, // [31:30]
                      bb_captured_timeout                                     :  1, // [29:29]
                      bb_captured_reason                                      :  3, // [28:26]
                      bb_captured_channel                                     :  1, // [25:25]
                      follow_up_dialog_token                                  :  8, // [24:17]
                      dialog_token                                            :  8, // [16:9]
                      ftm_tm                                                  :  2, // [8:7]
                      mpdu_delimiter_errors_seen                              :  1, // [6:6]
                      coex_wlan_tx_after_start_of_rx                          :  1, // [5:5]
                      coex_wlan_tx_from_start_of_rx                           :  1, // [4:4]
                      coex_wan_tx_after_start_of_rx                           :  1, // [3:3]
                      coex_wan_tx_from_start_of_rx                            :  1, // [2:2]
                      coex_bt_tx_after_start_of_rx                            :  1, // [1:1]
                      coex_bt_tx_from_start_of_rx                             :  1; // [0:0]
             uint32_t reserved_4                                              :  2, // [31:30]
                      after_mpdu_count_passing_fcs                            : 10, // [29:20]
                      before_mpdu_count_failing_fcs                           : 10, // [19:10]
                      before_mpdu_count_passing_fcs                           : 10; // [9:0]
             uint32_t reserved_5                                              : 22, // [31:10]
                      after_mpdu_count_failing_fcs                            : 10; // [9:0]
             uint32_t phy_timestamp_tx_lower_32                               : 32; // [31:0]
             uint32_t phy_timestamp_tx_upper_32                               : 32; // [31:0]
             uint32_t first_bt_broadcast_status_details                       : 12, // [31:20]
                      reserved_8                                              :  3, // [19:17]
                      bb_data                                                 :  1, // [16:16]
                      bb_length                                               : 16; // [15:0]
             uint32_t reserved_9                                              :  8, // [31:24]
                      rx_ppdu_duration                                        : 24; // [23:0]
             uint32_t second_bt_broadcast_status_details                      : 12, // [31:20]
                      reserved_10                                             :  3, // [19:17]
                      ast_index_valid                                         :  1, // [16:16]
                      ast_index                                               : 16; // [15:0]
             struct   phyrx_abort_request_info                                  phyrx_abort_request_info_details;
             uint32_t reserved_12a                                            :  4, // [31:28]
                      pre_bt_broadcast_status_details                         : 12; // [27:16]
             struct   macrx_abort_request_info                                  macrx_abort_request_info_details;
             uint32_t non_qos_sn_lowest_retry_setting                         :  1, // [31:31]
                      non_qos_sn_lowest                                       : 12, // [30:19]
                      non_qos_sn_highest_retry_setting                        :  1, // [18:18]
                      non_qos_sn_highest                                      : 12, // [17:6]
                      reserved_13a                                            :  5, // [5:1]
                      non_qos_sn_info_valid                                   :  1; // [0:0]
             uint32_t qos_sn_1_lowest_retry_setting                           :  1, // [31:31]
                      qos_sn_1_lowest                                         : 12, // [30:19]
                      qos_sn_1_highest_retry_setting                          :  1, // [18:18]
                      qos_sn_1_highest                                        : 12, // [17:6]
                      qos_sn_1_tid                                            :  4, // [5:2]
                      reserved_14a                                            :  1, // [1:1]
                      qos_sn_1_info_valid                                     :  1; // [0:0]
             uint32_t qos_sn_2_lowest_retry_setting                           :  1, // [31:31]
                      qos_sn_2_lowest                                         : 12, // [30:19]
                      qos_sn_2_highest_retry_setting                          :  1, // [18:18]
                      qos_sn_2_highest                                        : 12, // [17:6]
                      qos_sn_2_tid                                            :  4, // [5:2]
                      reserved_15a                                            :  1, // [1:1]
                      qos_sn_2_info_valid                                     :  1; // [0:0]
             struct   rxpcu_ppdu_end_layout_info                                rxpcu_ppdu_end_layout_details;
             uint32_t reserved_26a                                            : 21, // [31:11]
                      qos_sn_2_frag_num_state                                 :  4, // [10:7]
                      qos_sn_2_more_frag_state                                :  1, // [6:6]
                      qos_sn_1_frag_num_state                                 :  4, // [5:2]
                      qos_sn_1_more_frag_state                                :  1, // [1:1]
                      corrupted_due_to_fifo_delay                             :  1; // [0:0]
             uint32_t rx_ppdu_end_marker                                      : 32; // [31:0]
#endif
};


/* Description		WB_TIMESTAMP_LOWER_32

			WLAN/BT timestamp is a 1 usec resolution timestamp which
			 does not get updated based on receive beacon like TSF. 
			 The same rules for capturing tsf_timestamp are used to 
			capture the wb_timestamp. This field represents the lower
			 32 bits of the 64-bit timestamp
*/

#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_LOWER_32_OFFSET                            0x0000000000000000
#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_LOWER_32_LSB                               0
#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_LOWER_32_MSB                               31
#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_LOWER_32_MASK                              0x00000000ffffffff


/* Description		WB_TIMESTAMP_UPPER_32

			WLAN/BT timestamp is a 1 usec resolution timestamp which
			 does not get updated based on receive beacon like TSF. 
			 The same rules for capturing tsf_timestamp are used to 
			capture the wb_timestamp. This field represents the upper
			 32 bits of the 64-bit timestamp
*/

#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_UPPER_32_OFFSET                            0x0000000000000000
#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_UPPER_32_LSB                               32
#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_UPPER_32_MSB                               63
#define RXPCU_PPDU_END_INFO_WB_TIMESTAMP_UPPER_32_MASK                              0xffffffff00000000


/* Description		RX_ANTENNA

			Receive antenna value ???
*/

#define RXPCU_PPDU_END_INFO_RX_ANTENNA_OFFSET                                       0x0000000000000008
#define RXPCU_PPDU_END_INFO_RX_ANTENNA_LSB                                          0
#define RXPCU_PPDU_END_INFO_RX_ANTENNA_MSB                                          23
#define RXPCU_PPDU_END_INFO_RX_ANTENNA_MASK                                         0x0000000000ffffff


/* Description		TX_HT_VHT_ACK

			Indicates that a HT or VHT Ack/BA frame was transmitted 
			in response to this receive packet.
*/

#define RXPCU_PPDU_END_INFO_TX_HT_VHT_ACK_OFFSET                                    0x0000000000000008
#define RXPCU_PPDU_END_INFO_TX_HT_VHT_ACK_LSB                                       24
#define RXPCU_PPDU_END_INFO_TX_HT_VHT_ACK_MSB                                       24
#define RXPCU_PPDU_END_INFO_TX_HT_VHT_ACK_MASK                                      0x0000000001000000


/* Description		UNSUPPORTED_MU_NC

			Set if MU Nc > 2 in received NDPA.
			If this bit is set, even though AID and BSSID are matched, 
			MAC doesn't send tx_expect_ndp to PHY, because MU Nc > 2
			 is not supported in Helium. 
*/

#define RXPCU_PPDU_END_INFO_UNSUPPORTED_MU_NC_OFFSET                                0x0000000000000008
#define RXPCU_PPDU_END_INFO_UNSUPPORTED_MU_NC_LSB                                   25
#define RXPCU_PPDU_END_INFO_UNSUPPORTED_MU_NC_MSB                                   25
#define RXPCU_PPDU_END_INFO_UNSUPPORTED_MU_NC_MASK                                  0x0000000002000000


/* Description		OTP_TXBF_DISABLE

			Set if either OTP_SUBFEE_DISABLE or OTP_TXBF_DISABLE is 
			set and if RXPU receives directed NDPA frame. Then, RXPCU
			 should not send TX_EXPECT_NDP TLV to SW but set this bit
			 to inform SW. 
*/

#define RXPCU_PPDU_END_INFO_OTP_TXBF_DISABLE_OFFSET                                 0x0000000000000008
#define RXPCU_PPDU_END_INFO_OTP_TXBF_DISABLE_LSB                                    26
#define RXPCU_PPDU_END_INFO_OTP_TXBF_DISABLE_MSB                                    26
#define RXPCU_PPDU_END_INFO_OTP_TXBF_DISABLE_MASK                                   0x0000000004000000


/* Description		PREVIOUS_TLV_CORRUPTED

			When set, the TLV preceding this RXPCU_END_INFO TLV within
			 the RX_PPDU_END TLV, is corrupted. Not the entire TLV was
			 received.... Likely due to an abort scenario... If abort
			 is to blame, see the abort data datastructure for details.
			
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_PREVIOUS_TLV_CORRUPTED_OFFSET                           0x0000000000000008
#define RXPCU_PPDU_END_INFO_PREVIOUS_TLV_CORRUPTED_LSB                              27
#define RXPCU_PPDU_END_INFO_PREVIOUS_TLV_CORRUPTED_MSB                              27
#define RXPCU_PPDU_END_INFO_PREVIOUS_TLV_CORRUPTED_MASK                             0x0000000008000000


/* Description		PHYRX_ABORT_REQUEST_INFO_VALID

			When set, the PHY sent an PHYRX_ABORT_REQUEST TLV to RXPCU. 
			The abort fields embedded in this TLV contain valid info.
			
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_VALID_OFFSET                   0x0000000000000008
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_VALID_LSB                      28
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_VALID_MSB                      28
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_VALID_MASK                     0x0000000010000000


/* Description		MACRX_ABORT_REQUEST_INFO_VALID

			When set, the MAC sent an MACRX_ABORT_REQUEST TLV to PHYRX. 
			The abort fields embedded in this TLV contain valid info.
			
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_VALID_OFFSET                   0x0000000000000008
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_VALID_LSB                      29
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_VALID_MSB                      29
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_VALID_MASK                     0x0000000020000000


/* Description		RESERVED

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RESERVED_OFFSET                                         0x0000000000000008
#define RXPCU_PPDU_END_INFO_RESERVED_LSB                                            30
#define RXPCU_PPDU_END_INFO_RESERVED_MSB                                            31
#define RXPCU_PPDU_END_INFO_RESERVED_MASK                                           0x00000000c0000000


/* Description		COEX_BT_TX_FROM_START_OF_RX

			Set when BT TX was ongoing when WLAN RX started
*/

#define RXPCU_PPDU_END_INFO_COEX_BT_TX_FROM_START_OF_RX_OFFSET                      0x0000000000000008
#define RXPCU_PPDU_END_INFO_COEX_BT_TX_FROM_START_OF_RX_LSB                         32
#define RXPCU_PPDU_END_INFO_COEX_BT_TX_FROM_START_OF_RX_MSB                         32
#define RXPCU_PPDU_END_INFO_COEX_BT_TX_FROM_START_OF_RX_MASK                        0x0000000100000000


/* Description		COEX_BT_TX_AFTER_START_OF_RX

			Set when BT TX started while WLAN RX was already ongoing
			
*/

#define RXPCU_PPDU_END_INFO_COEX_BT_TX_AFTER_START_OF_RX_OFFSET                     0x0000000000000008
#define RXPCU_PPDU_END_INFO_COEX_BT_TX_AFTER_START_OF_RX_LSB                        33
#define RXPCU_PPDU_END_INFO_COEX_BT_TX_AFTER_START_OF_RX_MSB                        33
#define RXPCU_PPDU_END_INFO_COEX_BT_TX_AFTER_START_OF_RX_MASK                       0x0000000200000000


/* Description		COEX_WAN_TX_FROM_START_OF_RX

			Set when WAN TX was ongoing when WLAN RX started
*/

#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_FROM_START_OF_RX_OFFSET                     0x0000000000000008
#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_FROM_START_OF_RX_LSB                        34
#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_FROM_START_OF_RX_MSB                        34
#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_FROM_START_OF_RX_MASK                       0x0000000400000000


/* Description		COEX_WAN_TX_AFTER_START_OF_RX

			Set when WAN TX started while WLAN RX was already ongoing
			
*/

#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_AFTER_START_OF_RX_OFFSET                    0x0000000000000008
#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_AFTER_START_OF_RX_LSB                       35
#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_AFTER_START_OF_RX_MSB                       35
#define RXPCU_PPDU_END_INFO_COEX_WAN_TX_AFTER_START_OF_RX_MASK                      0x0000000800000000


/* Description		COEX_WLAN_TX_FROM_START_OF_RX

			Set when other WLAN TX was ongoing when WLAN RX started
*/

#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_FROM_START_OF_RX_OFFSET                    0x0000000000000008
#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_FROM_START_OF_RX_LSB                       36
#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_FROM_START_OF_RX_MSB                       36
#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_FROM_START_OF_RX_MASK                      0x0000001000000000


/* Description		COEX_WLAN_TX_AFTER_START_OF_RX

			Set when other WLAN TX started while WLAN RX was already
			 ongoing
*/

#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_AFTER_START_OF_RX_OFFSET                   0x0000000000000008
#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_AFTER_START_OF_RX_LSB                      37
#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_AFTER_START_OF_RX_MSB                      37
#define RXPCU_PPDU_END_INFO_COEX_WLAN_TX_AFTER_START_OF_RX_MASK                     0x0000002000000000


/* Description		MPDU_DELIMITER_ERRORS_SEEN

			When set, MPDU delimiter errors have been detected during
			 this PPDU reception
*/

#define RXPCU_PPDU_END_INFO_MPDU_DELIMITER_ERRORS_SEEN_OFFSET                       0x0000000000000008
#define RXPCU_PPDU_END_INFO_MPDU_DELIMITER_ERRORS_SEEN_LSB                          38
#define RXPCU_PPDU_END_INFO_MPDU_DELIMITER_ERRORS_SEEN_MSB                          38
#define RXPCU_PPDU_END_INFO_MPDU_DELIMITER_ERRORS_SEEN_MASK                         0x0000004000000000


/* Description		FTM_TM

			Indicate the timestamp is for the FTM or TM frame 
			
			0: non TM or FTM frame
			1: FTM frame
			2: TM frame
			3: reserved
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_FTM_TM_OFFSET                                           0x0000000000000008
#define RXPCU_PPDU_END_INFO_FTM_TM_LSB                                              39
#define RXPCU_PPDU_END_INFO_FTM_TM_MSB                                              40
#define RXPCU_PPDU_END_INFO_FTM_TM_MASK                                             0x0000018000000000


/* Description		DIALOG_TOKEN

			The dialog token in the FTM or TM frame. Only valid when
			 the FTM is set. Clear to 254 for a non-FTM frame
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_DIALOG_TOKEN_OFFSET                                     0x0000000000000008
#define RXPCU_PPDU_END_INFO_DIALOG_TOKEN_LSB                                        41
#define RXPCU_PPDU_END_INFO_DIALOG_TOKEN_MSB                                        48
#define RXPCU_PPDU_END_INFO_DIALOG_TOKEN_MASK                                       0x0001fe0000000000


/* Description		FOLLOW_UP_DIALOG_TOKEN

			The follow up dialog token in the FTM or TM frame. Only 
			valid when the FTM is set. Clear to 0 for a non-FTM frame, 
			The follow up dialog token in the FTM frame. Only valid 
			when the FTM is set. Clear to 255 for a non-FTM frame<legal
			 all>
*/

#define RXPCU_PPDU_END_INFO_FOLLOW_UP_DIALOG_TOKEN_OFFSET                           0x0000000000000008
#define RXPCU_PPDU_END_INFO_FOLLOW_UP_DIALOG_TOKEN_LSB                              49
#define RXPCU_PPDU_END_INFO_FOLLOW_UP_DIALOG_TOKEN_MSB                              56
#define RXPCU_PPDU_END_INFO_FOLLOW_UP_DIALOG_TOKEN_MASK                             0x01fe000000000000


/* Description		BB_CAPTURED_CHANNEL

			Set by RXPCU when MACRX_FREEZE_CAPTURE_CHANNEL TLV is sent
			 to PHY, FW check it to correlate current PPDU TLVs with
			 uploaded channel information.
			
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_BB_CAPTURED_CHANNEL_OFFSET                              0x0000000000000008
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_CHANNEL_LSB                                 57
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_CHANNEL_MSB                                 57
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_CHANNEL_MASK                                0x0200000000000000


/* Description		BB_CAPTURED_REASON

			Copy "capture_reason" of MACRX_FREEZE_CAPTURE_CHANNEL TLV
			 to here for FW usage. Valid when bb_captured_channel or
			 bb_captured_timeout is set.
			
			This field indicates why the MAC asked to capture the channel
			
			<enum 0 freeze_reason_TM>
			<enum 1 freeze_reason_FTM>
			<enum 2 freeze_reason_ACK_resp_to_TM_FTM>
			<enum 3 freeze_reason_TA_RA_TYPE_FILTER>
			<enum 4 freeze_reason_NDPA_NDP>
			<enum 5 freeze_reason_ALL_PACKET>
			
			<legal 0-5>
*/

#define RXPCU_PPDU_END_INFO_BB_CAPTURED_REASON_OFFSET                               0x0000000000000008
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_REASON_LSB                                  58
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_REASON_MSB                                  60
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_REASON_MASK                                 0x1c00000000000000


/* Description		BB_CAPTURED_TIMEOUT

			Set by RxPCU to indicate channel capture condition is meet, 
			but MACRX_FREEZE_CAPTURE_CHANNEL is not sent to PHY due 
			to AST long delay, which means the rx_frame_falling edge
			 to FREEZE TLV ready time exceed the threshold time defined
			 by RXPCU register FREEZE_TLV_DELAY_CNT_THRESH. 
			Bb_captured_reason is still valid in this case.
			
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_BB_CAPTURED_TIMEOUT_OFFSET                              0x0000000000000008
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_TIMEOUT_LSB                                 61
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_TIMEOUT_MSB                                 61
#define RXPCU_PPDU_END_INFO_BB_CAPTURED_TIMEOUT_MASK                                0x2000000000000000


/* Description		RESERVED_3

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RESERVED_3_OFFSET                                       0x0000000000000008
#define RXPCU_PPDU_END_INFO_RESERVED_3_LSB                                          62
#define RXPCU_PPDU_END_INFO_RESERVED_3_MSB                                          63
#define RXPCU_PPDU_END_INFO_RESERVED_3_MASK                                         0xc000000000000000


/* Description		BEFORE_MPDU_COUNT_PASSING_FCS

			Number of MPDUs received in this PPDU that passed the FCS
			 check before the Coex TX started
			
			The counter saturates at 0x3FF.
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_PASSING_FCS_OFFSET                    0x0000000000000010
#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_PASSING_FCS_LSB                       0
#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_PASSING_FCS_MSB                       9
#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_PASSING_FCS_MASK                      0x00000000000003ff


/* Description		BEFORE_MPDU_COUNT_FAILING_FCS

			Number of MPDUs received in this PPDU that failed the FCS
			 check before the Coex TX started
			
			The counter saturates at 0x3FF.
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_FAILING_FCS_OFFSET                    0x0000000000000010
#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_FAILING_FCS_LSB                       10
#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_FAILING_FCS_MSB                       19
#define RXPCU_PPDU_END_INFO_BEFORE_MPDU_COUNT_FAILING_FCS_MASK                      0x00000000000ffc00


/* Description		AFTER_MPDU_COUNT_PASSING_FCS

			Number of MPDUs received in this PPDU that passed the FCS
			 check after the moment the Coex TX started
			
			(Note: The partially received MPDU when the COEX tx start
			 event came in falls in the "after" category)
			
			The counter saturates at 0x3FF.
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_PASSING_FCS_OFFSET                     0x0000000000000010
#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_PASSING_FCS_LSB                        20
#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_PASSING_FCS_MSB                        29
#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_PASSING_FCS_MASK                       0x000000003ff00000


/* Description		RESERVED_4

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RESERVED_4_OFFSET                                       0x0000000000000010
#define RXPCU_PPDU_END_INFO_RESERVED_4_LSB                                          30
#define RXPCU_PPDU_END_INFO_RESERVED_4_MSB                                          31
#define RXPCU_PPDU_END_INFO_RESERVED_4_MASK                                         0x00000000c0000000


/* Description		AFTER_MPDU_COUNT_FAILING_FCS

			Number of MPDUs received in this PPDU that failed the FCS
			 check after the moment the Coex TX started
			
			(Note: The partially received MPDU when the COEX tx start
			 event came in falls in the "after" category)
			
			The counter saturates at 0x3FF.
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_FAILING_FCS_OFFSET                     0x0000000000000010
#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_FAILING_FCS_LSB                        32
#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_FAILING_FCS_MSB                        41
#define RXPCU_PPDU_END_INFO_AFTER_MPDU_COUNT_FAILING_FCS_MASK                       0x000003ff00000000


/* Description		RESERVED_5

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RESERVED_5_OFFSET                                       0x0000000000000010
#define RXPCU_PPDU_END_INFO_RESERVED_5_LSB                                          42
#define RXPCU_PPDU_END_INFO_RESERVED_5_MSB                                          63
#define RXPCU_PPDU_END_INFO_RESERVED_5_MASK                                         0xfffffc0000000000


/* Description		PHY_TIMESTAMP_TX_LOWER_32

			The PHY timestamp in the AMPI of the most recent rising 
			edge (TODO: of what ???) after the TX_PHY_DESC.  This field
			 indicates the lower 32 bits of the timestamp
*/

#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_LOWER_32_OFFSET                        0x0000000000000018
#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_LOWER_32_LSB                           0
#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_LOWER_32_MSB                           31
#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_LOWER_32_MASK                          0x00000000ffffffff


/* Description		PHY_TIMESTAMP_TX_UPPER_32

			The PHY timestamp in the AMPI of the most recent rising 
			edge (TODO: of what ???) after the TX_PHY_DESC.  This field
			 indicates the upper 32 bits of the timestamp
*/

#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_UPPER_32_OFFSET                        0x0000000000000018
#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_UPPER_32_LSB                           32
#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_UPPER_32_MSB                           63
#define RXPCU_PPDU_END_INFO_PHY_TIMESTAMP_TX_UPPER_32_MASK                          0xffffffff00000000


/* Description		BB_LENGTH

			Indicates the number of bytes of baseband information for
			 PPDUs where the BB descriptor preamble type is 0x80 to 
			0xFF which indicates that this is not a normal PPDU but 
			rather contains baseband debug information.
			TODO: Is this still needed ??? 
*/

#define RXPCU_PPDU_END_INFO_BB_LENGTH_OFFSET                                        0x0000000000000020
#define RXPCU_PPDU_END_INFO_BB_LENGTH_LSB                                           0
#define RXPCU_PPDU_END_INFO_BB_LENGTH_MSB                                           15
#define RXPCU_PPDU_END_INFO_BB_LENGTH_MASK                                          0x000000000000ffff


/* Description		BB_DATA

			Indicates that BB data associated with this PPDU will exist
			 in the receive buffer.  The exact contents of this BB data
			 can be found by decoding the BB TLV in the buffer associated
			 with the BB data.  See vector_fragment in the Helium_mac_phy_interface.docx
			
*/

#define RXPCU_PPDU_END_INFO_BB_DATA_OFFSET                                          0x0000000000000020
#define RXPCU_PPDU_END_INFO_BB_DATA_LSB                                             16
#define RXPCU_PPDU_END_INFO_BB_DATA_MSB                                             16
#define RXPCU_PPDU_END_INFO_BB_DATA_MASK                                            0x0000000000010000


/* Description		RESERVED_8

			Reserved: HW should fill with 0, FW should ignore.
*/

#define RXPCU_PPDU_END_INFO_RESERVED_8_OFFSET                                       0x0000000000000020
#define RXPCU_PPDU_END_INFO_RESERVED_8_LSB                                          17
#define RXPCU_PPDU_END_INFO_RESERVED_8_MSB                                          19
#define RXPCU_PPDU_END_INFO_RESERVED_8_MASK                                         0x00000000000e0000


/* Description		FIRST_BT_BROADCAST_STATUS_DETAILS

			Same contents as field "bt_broadcast_status_details" for
			 the first received COEX_STATUS_BROADCAST tlv during this
			 PPDU reception.
			
			If no COEX_STATUS_BROADCAST tlv is received during this 
			PPDU reception, this field will be set to 0
			
			
			For detailed info see doc: TBD
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_FIRST_BT_BROADCAST_STATUS_DETAILS_OFFSET                0x0000000000000020
#define RXPCU_PPDU_END_INFO_FIRST_BT_BROADCAST_STATUS_DETAILS_LSB                   20
#define RXPCU_PPDU_END_INFO_FIRST_BT_BROADCAST_STATUS_DETAILS_MSB                   31
#define RXPCU_PPDU_END_INFO_FIRST_BT_BROADCAST_STATUS_DETAILS_MASK                  0x00000000fff00000


/* Description		RX_PPDU_DURATION

			The length of this PPDU reception in us
*/

#define RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_OFFSET                                 0x0000000000000020
#define RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_LSB                                    32
#define RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_MSB                                    55
#define RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_MASK                                   0x00ffffff00000000


/* Description		RESERVED_9

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RESERVED_9_OFFSET                                       0x0000000000000020
#define RXPCU_PPDU_END_INFO_RESERVED_9_LSB                                          56
#define RXPCU_PPDU_END_INFO_RESERVED_9_MSB                                          63
#define RXPCU_PPDU_END_INFO_RESERVED_9_MASK                                         0xff00000000000000


/* Description		AST_INDEX

			The AST index of the receive Ack/BA.  This information is
			 provided from the TXPCU to the RXPCU for receive Ack/BA
			 for implicit beamforming.
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_AST_INDEX_OFFSET                                        0x0000000000000028
#define RXPCU_PPDU_END_INFO_AST_INDEX_LSB                                           0
#define RXPCU_PPDU_END_INFO_AST_INDEX_MSB                                           15
#define RXPCU_PPDU_END_INFO_AST_INDEX_MASK                                          0x000000000000ffff


/* Description		AST_INDEX_VALID

			Indicates that ast_index is valid.  Should only be set for
			 receive Ack/BA where single stream implicit sounding is
			 captured.
*/

#define RXPCU_PPDU_END_INFO_AST_INDEX_VALID_OFFSET                                  0x0000000000000028
#define RXPCU_PPDU_END_INFO_AST_INDEX_VALID_LSB                                     16
#define RXPCU_PPDU_END_INFO_AST_INDEX_VALID_MSB                                     16
#define RXPCU_PPDU_END_INFO_AST_INDEX_VALID_MASK                                    0x0000000000010000


/* Description		RESERVED_10

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RESERVED_10_OFFSET                                      0x0000000000000028
#define RXPCU_PPDU_END_INFO_RESERVED_10_LSB                                         17
#define RXPCU_PPDU_END_INFO_RESERVED_10_MSB                                         19
#define RXPCU_PPDU_END_INFO_RESERVED_10_MASK                                        0x00000000000e0000


/* Description		SECOND_BT_BROADCAST_STATUS_DETAILS

			Same contents as field "bt_broadcast_status_details" for
			 the second received COEX_STATUS_BROADCAST tlv during this
			 PPDU reception.
			
			If no second COEX_STATUS_BROADCAST tlv is received during
			 this PPDU reception, this field will be set to 0
			
			
			For detailed info see doc: TBD
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_SECOND_BT_BROADCAST_STATUS_DETAILS_OFFSET               0x0000000000000028
#define RXPCU_PPDU_END_INFO_SECOND_BT_BROADCAST_STATUS_DETAILS_LSB                  20
#define RXPCU_PPDU_END_INFO_SECOND_BT_BROADCAST_STATUS_DETAILS_MSB                  31
#define RXPCU_PPDU_END_INFO_SECOND_BT_BROADCAST_STATUS_DETAILS_MASK                 0x00000000fff00000


/* Description		PHYRX_ABORT_REQUEST_INFO_DETAILS

			Field only valid when Phyrx_abort_request_info_valid is 
			set
			The reason why PHY generated an abort request
*/


/* Description		PHYRX_ABORT_REASON

			<enum 0 phyrx_err_phy_off> Reception aborted due to receiving
			 a PHY_OFF TLV
			<enum 1 phyrx_err_synth_off> 
			<enum 2 phyrx_err_ofdma_timing> 
			<enum 3 phyrx_err_ofdma_signal_parity> 
			<enum 4 phyrx_err_ofdma_rate_illegal> 
			<enum 5 phyrx_err_ofdma_length_illegal> 
			<enum 6 phyrx_err_ofdma_restart> 
			<enum 7 phyrx_err_ofdma_service> 
			<enum 8 phyrx_err_ppdu_ofdma_power_drop> 
			
			<enum 9 phyrx_err_cck_blokker> 
			<enum 10 phyrx_err_cck_timing> 
			<enum 11 phyrx_err_cck_header_crc> 
			<enum 12 phyrx_err_cck_rate_illegal> 
			<enum 13 phyrx_err_cck_length_illegal> 
			<enum 14 phyrx_err_cck_restart> 
			<enum 15 phyrx_err_cck_service> 
			<enum 16 phyrx_err_cck_power_drop> 
			
			<enum 17 phyrx_err_ht_crc_err> 
			<enum 18 phyrx_err_ht_length_illegal> 
			<enum 19 phyrx_err_ht_rate_illegal> 
			<enum 20 phyrx_err_ht_zlf> 
			<enum 21 phyrx_err_false_radar_ext> 
			<enum 22 phyrx_err_green_field>
			<enum 60 phyrx_err_ht_nsym_lt_zero>
			
			<enum 23 phyrx_err_bw_gt_dyn_bw> 
			<enum 24 phyrx_err_leg_ht_mismatch> 
			<enum 25 phyrx_err_vht_crc_error> 
			<enum 26 phyrx_err_vht_siga_unsupported> 
			<enum 27 phyrx_err_vht_lsig_len_invalid> 
			<enum 28 phyrx_err_vht_ndp_or_zlf> 
			<enum 29 phyrx_err_vht_nsym_lt_zero> 
			<enum 30 phyrx_err_vht_rx_extra_symbol_mismatch> 
			<enum 31 phyrx_err_vht_rx_skip_group_id0> 
			<enum 32 phyrx_err_vht_rx_skip_group_id1to62> 
			<enum 33 phyrx_err_vht_rx_skip_group_id63> 
			<enum 34 phyrx_err_ofdm_ldpc_decoder_disabled> 
			<enum 35 phyrx_err_defer_nap>
			
			<enum 61 phyrx_err_vht_lsig_rate_mismatch>
			<enum 62 phyrx_err_vht_paid_gid_mismatch>
			<enum 63 phyrx_err_vht_unsupported_bw>
			<enum 64 phyrx_err_vht_gi_disam_mismatch>
			
			<enum 36 phyrx_err_fdomain_timeout> 
			<enum 37 phyrx_err_lsig_rel_check> 
			<enum 38 phyrx_err_bt_collision> 
			<enum 39 phyrx_err_unsupported_mu_feedback> 
			<enum 40 phyrx_err_ppdu_tx_interrupt_rx> 
			<enum 41 phyrx_err_unsupported_cbf> 
			
			<enum 42 phyrx_err_other>  Should not really be used. If
			 needed, ask for documentation update 
			
			<enum 43 phyrx_err_he_siga_unsupported > <enum 44 phyrx_err_he_crc_error
			 > <enum 45 phyrx_err_he_sigb_unsupported > <enum 46 phyrx_err_he_mu_mode_unsupported
			 > <enum 47 phyrx_err_he_ndp_or_zlf > <enum 48 phyrx_err_he_nsym_lt_zero
			 > <enum 49 phyrx_err_he_ru_params_unsupported > <enum 50
			 phyrx_err_he_num_users_unsupported ><enum 51 phyrx_err_he_sounding_params_unsupported
			 >
			<enum 54 phyrx_err_he_sigb_crc_error>
			<enum 55 phyrx_err_he_ext_su_unsupported>
			<enum 56 phyrx_err_he_trig_unsupported>
			<enum 57 phyrx_err_he_lsig_len_invalid>
			<enum 58 phyrx_err_he_lsig_rate_mismatch>
			<enum 59 phyrx_err_ofdma_signal_reliability>
			
			<enum 77 phyrx_err_wur_detection>
			
			<enum 72 phyrx_err_u_sig_crc_error>
			<enum 73 phyrx_err_u_sig_unsupported_mode>
			<enum 74 phyrx_err_u_sig_rsvd_err>
			<enum 75 phyrx_err_u_sig_mcs_error>
			<enum 76 phyrx_err_u_sig_bw_error>
			<enum 79 phyrx_err_u_sig_320_channel_mismatch>
			<enum 71 phyrx_err_eht_sig_crc_error>
			<enum 78 phyrx_err_eht_sig_unsupported_mode>
			
			<enum 80 phyrx_err_ehtplus_er_detection>
			
			<enum 52 phyrx_err_MU_UL_no_power_detected> 
			<enum 53 phyrx_err_MU_UL_not_for_me>
			
			<enum 65 phyrx_err_rx_wdg_timeout>
			<enum 66 phyrx_err_sizing_evt_unexpected>
			<enum 67 phyrx_err_spectralscan>
			<enum 68 phyrx_err_radar_misdetected_as_ofdm>
			<enum 69 phyrx_err_rx_stuck>
			<enum 70 phyrx_err_invalid_11b_state>
			
			<legal 0 - 80>
*/

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_OFFSET 0x0000000000000028
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_LSB 32
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_MSB 39
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_MASK 0x000000ff00000000


/* Description		PHY_ENTERS_NAP_STATE

			When set, PHY enters PHY NAP state after sending this abort
			
			
			Note that nap and defer state are mutually exclusive.
			
			Field put pro-actively in place....usage still to be agreed
			 upon.
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_OFFSET 0x0000000000000028
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_LSB 40
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_MSB 40
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_MASK 0x0000010000000000


/* Description		PHY_ENTERS_DEFER_STATE

			When set, PHY enters PHY defer state after sending this 
			abort
			
			Note that nap and defer state are mutually exclusive.
			
			Field put pro-actively in place....usage still to be agreed
			 upon.
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_OFFSET 0x0000000000000028
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_LSB 41
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_MSB 41
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_MASK 0x0000020000000000


/* Description		RESERVED_0

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_OFFSET      0x0000000000000028
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_LSB         42
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_MSB         47
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_MASK        0x0000fc0000000000


/* Description		RECEIVE_DURATION

			The remaining receive duration of this PPDU in the medium
			 (in us). When PHY does not know this duration when this
			 TLV is generated, the field will be set to 0.
			The timing reference point is the reception by the MAC of
			 this TLV. The value shall be accurate to within 2us.
			
			In case Phy_enters_nap_state and/or Phy_enters_defer_state
			 is set, there is a possibility that MAC PMM can also decide
			 to go into a low(er) power state. 
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_OFFSET 0x0000000000000028
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_LSB   48
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_MSB   63
#define RXPCU_PPDU_END_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_MASK  0xffff000000000000


/* Description		MACRX_ABORT_REQUEST_INFO_DETAILS

			Field only valid when macrx_abort_request_info_valid is 
			set
			The reason why MACRX generated an abort request
*/


/* Description		MACRX_ABORT_REASON

			<enum 0 macrx_abort_sw_initiated>
			<enum 1 macrx_abort_obss_reception> Upon receiving this 
			abort reason, PHY should stop reception of the current frame
			 and go back into a search mode
			<enum 2 macrx_abort_other>
			<enum 3 macrx_abort_sw_initiated_channel_switch> MAC FW 
			issued an abort for channel switch reasons
			<enum 4 macrx_abort_sw_initiated_power_save> MAC FW issued
			 an abort power save reasons
			<enum 5 macrx_abort_too_much_bad_data> RXPCU is terminating
			 the current ongoing reception, as the data that MAC is 
			receiving seems to be all garbage... The PER is too high, 
			or in case of MU UL, Likely the trigger frame never got 
			properly received by any of the targeted MU UL devices. 
			After the abort, PHYRX can resume a normal search mode.
			<enum 6 macrx_abort_ul_mu_early_abort> RXPCU is terminating
			 the current ongoing UL MU reception, because at the end
			 of the "early_termination_window," the required number 
			of users with at least one valid MPDU delimiter was not 
			reached. Likely the trigger frame never got properly received
			 by the required number of targeted devices. After the abort, 
			PHYRX can resume a normal search mode.
			
			<legal 0-6>
*/

#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_OFFSET 0x0000000000000030
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_LSB 0
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_MSB 7
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_MASK 0x00000000000000ff


/* Description		RESERVED_0

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_OFFSET      0x0000000000000030
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_LSB         8
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_MSB         15
#define RXPCU_PPDU_END_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_MASK        0x000000000000ff00


/* Description		PRE_BT_BROADCAST_STATUS_DETAILS

			Same contents as field "bt_broadcast_status_details" of 
			the last received COEX_STATUS_BROADCAST tlv before this 
			PPDU reception.
			After power up, this field is all initialized to 0
			
			For detailed info see doc: TBD
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_PRE_BT_BROADCAST_STATUS_DETAILS_OFFSET                  0x0000000000000030
#define RXPCU_PPDU_END_INFO_PRE_BT_BROADCAST_STATUS_DETAILS_LSB                     16
#define RXPCU_PPDU_END_INFO_PRE_BT_BROADCAST_STATUS_DETAILS_MSB                     27
#define RXPCU_PPDU_END_INFO_PRE_BT_BROADCAST_STATUS_DETAILS_MASK                    0x000000000fff0000


/* Description		RESERVED_12A

			Bits: [27:16] 
			Same contents as field "bt_broadcast_status_details" of 
			the last received COEX_STATUS_BROADCAST tlv before this 
			PPDU reception.
			After power up, this field is all initialized to 0
			
			Bits: [31:28]: always 0
			
			
			For detailed info see doc: TBD
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_RESERVED_12A_OFFSET                                     0x0000000000000030
#define RXPCU_PPDU_END_INFO_RESERVED_12A_LSB                                        28
#define RXPCU_PPDU_END_INFO_RESERVED_12A_MSB                                        31
#define RXPCU_PPDU_END_INFO_RESERVED_12A_MASK                                       0x00000000f0000000


/* Description		NON_QOS_SN_INFO_VALID

			When set, the non_QoS_SN_... fields contain valid info.
			
			This field will ONLY be set upon the very first reception
			 of a non QoS frame.
			
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_NON_QOS_SN_INFO_VALID_OFFSET                            0x0000000000000030
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_INFO_VALID_LSB                               32
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_INFO_VALID_MSB                               32
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_INFO_VALID_MASK                              0x0000000100000000


/* Description		RESERVED_13A

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RESERVED_13A_OFFSET                                     0x0000000000000030
#define RXPCU_PPDU_END_INFO_RESERVED_13A_LSB                                        33
#define RXPCU_PPDU_END_INFO_RESERVED_13A_MSB                                        37
#define RXPCU_PPDU_END_INFO_RESERVED_13A_MASK                                       0x0000003e00000000


/* Description		NON_QOS_SN_HIGHEST

			Field only valid when non_QoS_SN_info_valid is set
			
			Lowest and highest are defined based on a 2K window.
			When only 1 non-QoS frame is received, the 'highest' and
			 'lowest' fields will have the same values.
			
			The highest MPDU sequence number for a non-QoS frame received
			 in this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_OFFSET                               0x0000000000000030
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_LSB                                  38
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_MSB                                  49
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_MASK                                 0x0003ffc000000000


/* Description		NON_QOS_SN_HIGHEST_RETRY_SETTING

			Field only valid when non_QoS_SN_info_valid is set
			
			The 'retry' bit setting of the highest MPDU sequence number
			 non-QOS frame received in this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_RETRY_SETTING_OFFSET                 0x0000000000000030
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_RETRY_SETTING_LSB                    50
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_RETRY_SETTING_MSB                    50
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_HIGHEST_RETRY_SETTING_MASK                   0x0004000000000000


/* Description		NON_QOS_SN_LOWEST

			Field only valid when non_QoS_SN_info_valid is set
			
			Lowest and highest are defined based on a 2K window.
			When only 1 non-QoS frame is received, the 'highest' and
			 'lowest' fields will have the same values.
			
			The lowest MPDU sequence number for a non-QoS frame received
			 in this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_OFFSET                                0x0000000000000030
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_LSB                                   51
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_MSB                                   62
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_MASK                                  0x7ff8000000000000


/* Description		NON_QOS_SN_LOWEST_RETRY_SETTING

			Field only valid when non_QoS_SN_info_valid is set
			
			The 'retry' bit setting of the lowest MPDU sequence number
			 non-QoS frame received in this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_RETRY_SETTING_OFFSET                  0x0000000000000030
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_RETRY_SETTING_LSB                     63
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_RETRY_SETTING_MSB                     63
#define RXPCU_PPDU_END_INFO_NON_QOS_SN_LOWEST_RETRY_SETTING_MASK                    0x8000000000000000


/* Description		QOS_SN_1_INFO_VALID

			When set, the QoS_SN_1_... fields contain valid info.
			
			This field will ONLY be set upon the very first reception
			 of a QoS frame.
			
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_1_INFO_VALID_OFFSET                              0x0000000000000038
#define RXPCU_PPDU_END_INFO_QOS_SN_1_INFO_VALID_LSB                                 0
#define RXPCU_PPDU_END_INFO_QOS_SN_1_INFO_VALID_MSB                                 0
#define RXPCU_PPDU_END_INFO_QOS_SN_1_INFO_VALID_MASK                                0x0000000000000001


/* Description		RESERVED_14A

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RESERVED_14A_OFFSET                                     0x0000000000000038
#define RXPCU_PPDU_END_INFO_RESERVED_14A_LSB                                        1
#define RXPCU_PPDU_END_INFO_RESERVED_14A_MSB                                        1
#define RXPCU_PPDU_END_INFO_RESERVED_14A_MASK                                       0x0000000000000002


/* Description		QOS_SN_1_TID

			Field only valid when QoS_SN_1_info_valid is set.
			
			The TID of the frames related to the QoS_SN_1_... fields
			
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_1_TID_OFFSET                                     0x0000000000000038
#define RXPCU_PPDU_END_INFO_QOS_SN_1_TID_LSB                                        2
#define RXPCU_PPDU_END_INFO_QOS_SN_1_TID_MSB                                        5
#define RXPCU_PPDU_END_INFO_QOS_SN_1_TID_MASK                                       0x000000000000003c


/* Description		QOS_SN_1_HIGHEST

			Field only valid when QoS_SN_1_info_valid is set.
			
			Lowest and highest are defined based on a 2K window.
			When only 1 QoS frame of the relevant TID is received, the
			 'highest' and 'lowest' fields will have the same values.
			
			
			The highest MPDU sequence number for a QoS frame with TID
			 QoS_SN_1_TID received in this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_OFFSET                                 0x0000000000000038
#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_LSB                                    6
#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_MSB                                    17
#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_MASK                                   0x000000000003ffc0


/* Description		QOS_SN_1_HIGHEST_RETRY_SETTING

			Field only valid when QoS_SN_1_info_valid is set.
			
			The 'retry' bit setting of the highest MPDU sequence number
			 QoS frame with TID QoS_SN_1_TID received in this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_RETRY_SETTING_OFFSET                   0x0000000000000038
#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_RETRY_SETTING_LSB                      18
#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_RETRY_SETTING_MSB                      18
#define RXPCU_PPDU_END_INFO_QOS_SN_1_HIGHEST_RETRY_SETTING_MASK                     0x0000000000040000


/* Description		QOS_SN_1_LOWEST

			Field only valid when QoS_SN_1_info_valid is set.
			
			Lowest and highest are defined based on a 2K window.
			When only 1 QoS frame of the relevant TID is received, the
			 'highest' and 'lowest' fields will have the same values.
			
			
			The lowest MPDU sequence number for a QoS frame with TID
			 QoS_SN_1_TID received in this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_OFFSET                                  0x0000000000000038
#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_LSB                                     19
#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_MSB                                     30
#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_MASK                                    0x000000007ff80000


/* Description		QOS_SN_1_LOWEST_RETRY_SETTING

			Field only valid when QoS_SN_1_info_valid is set.
			
			The 'retry' bit setting of the lowest MPDU sequence number
			 QoS frame with TID QoS_SN_1_TID received in this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_RETRY_SETTING_OFFSET                    0x0000000000000038
#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_RETRY_SETTING_LSB                       31
#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_RETRY_SETTING_MSB                       31
#define RXPCU_PPDU_END_INFO_QOS_SN_1_LOWEST_RETRY_SETTING_MASK                      0x0000000080000000


/* Description		QOS_SN_2_INFO_VALID

			When set, the QoS_SN_2_... fields contain valid info.
			
			This field can ONLY be set in case of a multi-TID PPDU reception. 
			This field is set upon the very first reception of a QoS
			 frame belonging to the second TID in the PPDU.
			
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_2_INFO_VALID_OFFSET                              0x0000000000000038
#define RXPCU_PPDU_END_INFO_QOS_SN_2_INFO_VALID_LSB                                 32
#define RXPCU_PPDU_END_INFO_QOS_SN_2_INFO_VALID_MSB                                 32
#define RXPCU_PPDU_END_INFO_QOS_SN_2_INFO_VALID_MASK                                0x0000000100000000


/* Description		RESERVED_15A

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RESERVED_15A_OFFSET                                     0x0000000000000038
#define RXPCU_PPDU_END_INFO_RESERVED_15A_LSB                                        33
#define RXPCU_PPDU_END_INFO_RESERVED_15A_MSB                                        33
#define RXPCU_PPDU_END_INFO_RESERVED_15A_MASK                                       0x0000000200000000


/* Description		QOS_SN_2_TID

			Field only valid when QoS_SN_2_info_valid is set.
			
			The TID of the frames related to the QoS_SN_2_... fields
			
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_2_TID_OFFSET                                     0x0000000000000038
#define RXPCU_PPDU_END_INFO_QOS_SN_2_TID_LSB                                        34
#define RXPCU_PPDU_END_INFO_QOS_SN_2_TID_MSB                                        37
#define RXPCU_PPDU_END_INFO_QOS_SN_2_TID_MASK                                       0x0000003c00000000


/* Description		QOS_SN_2_HIGHEST

			Field only valid when QoS_SN_2_info_valid is set.
			
			Lowest and highest are defined based on a 2K window.
			When only 1 QoS frame of the relevant TID is received, the
			 highest and lowest fields will have the same values.
			
			The highest MPDU sequence number for a QoS frame with TID
			 QoS_SN_2_TID received in this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_OFFSET                                 0x0000000000000038
#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_LSB                                    38
#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_MSB                                    49
#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_MASK                                   0x0003ffc000000000


/* Description		QOS_SN_2_HIGHEST_RETRY_SETTING

			Field only valid when QoS_SN_2_info_valid is set.
			
			The 'retry' bit setting of the highest MPDU sequence number
			 QoS frame with TID QoS_SN_2_TID received in this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_RETRY_SETTING_OFFSET                   0x0000000000000038
#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_RETRY_SETTING_LSB                      50
#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_RETRY_SETTING_MSB                      50
#define RXPCU_PPDU_END_INFO_QOS_SN_2_HIGHEST_RETRY_SETTING_MASK                     0x0004000000000000


/* Description		QOS_SN_2_LOWEST

			Field only valid when QoS_SN_2_info_valid is set.
			
			Lowest and highest are defined based on a 2K window.
			When only 1 QoS frame of the relevant TID is received, the
			 highest and lowest fields will have the same values.
			
			The lowest MPDU sequence number for a QoS frame with TID
			 QoS_SN_2_TID received in this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_OFFSET                                  0x0000000000000038
#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_LSB                                     51
#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_MSB                                     62
#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_MASK                                    0x7ff8000000000000


/* Description		QOS_SN_2_LOWEST_RETRY_SETTING

			Field only valid when QoS_SN_2_info_valid is set.
			
			The 'retry' bit setting of the lowest MPDU sequence number
			 QoS frame with TID QoS_SN_2_TID received in this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_RETRY_SETTING_OFFSET                    0x0000000000000038
#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_RETRY_SETTING_LSB                       63
#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_RETRY_SETTING_MSB                       63
#define RXPCU_PPDU_END_INFO_QOS_SN_2_LOWEST_RETRY_SETTING_MASK                      0x8000000000000000


/* Description		RXPCU_PPDU_END_LAYOUT_DETAILS

			Structure containing the relative offsets of preamble TLVs
			 within 'RX_PPDU_END' documenting the layout within 'RX_PPDU_END'
			
*/


/* Description		RSSI_LEGACY_OFFSET

			Offset in units of 4 bytes of 'PHYRX_RSSI_LEGACY' within
			 'RX_PPDU_END'<legal 1, 2>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_LEGACY_OFFSET_OFFSET 0x0000000000000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_LEGACY_OFFSET_LSB    0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_LEGACY_OFFSET_MSB    1
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_LEGACY_OFFSET_MASK   0x0000000000000003


/* Description		L_SIG_A_OFFSET

			Offset in units of 4 bytes of 'PHYRX_L_SIG_A' within 'RX_PPDU_END' 
			Set to zero if the TLV is not included<legal 0, 44, 46>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_A_OFFSET_OFFSET     0x0000000000000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_A_OFFSET_LSB        2
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_A_OFFSET_MSB        7
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_A_OFFSET_MASK       0x00000000000000fc


/* Description		L_SIG_B_OFFSET

			Offset in units of 4 bytes of 'PHYRX_L_SIG_A' within 'RX_PPDU_END' 
			Set to zero if the TLV is not included<legal 0, 44, 46>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_B_OFFSET_OFFSET     0x0000000000000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_B_OFFSET_LSB        8
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_B_OFFSET_MSB        13
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_L_SIG_B_OFFSET_MASK       0x0000000000003f00


/* Description		HT_SIG_OFFSET

			Offset of 'PHYRX_HT_SIG' within 'RX_PPDU_END' Set to zero
			 if the TLV is not included<legal 0, 46, 50>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HT_SIG_OFFSET_OFFSET      0x0000000000000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HT_SIG_OFFSET_LSB         14
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HT_SIG_OFFSET_MSB         19
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HT_SIG_OFFSET_MASK        0x00000000000fc000


/* Description		VHT_SIG_A_OFFSET

			Offset in units of 4 bytes of 'PHYRX_VHT_SIG_A' within 'RX_PPDU_END' 
			Set to zero if the TLV is not included<legal 0, 46, 50>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_A_OFFSET_OFFSET   0x0000000000000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_A_OFFSET_LSB      20
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_A_OFFSET_MSB      25
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_A_OFFSET_MASK     0x0000000003f00000


/* Description		REPEAT_L_SIG_A_OFFSET

			Offset in units of 4 bytes of the repeat 'PHYRX_L_SIG_A' (in
			 HE and EHT cases) within 'RX_PPDU_END'
			 
			Set to zero if the TLV is not included
			<legal 0, 46, 50>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_REPEAT_L_SIG_A_OFFSET_OFFSET 0x0000000000000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_REPEAT_L_SIG_A_OFFSET_LSB 26
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_REPEAT_L_SIG_A_OFFSET_MSB 31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_REPEAT_L_SIG_A_OFFSET_MASK 0x00000000fc000000


/* Description		HE_SIG_A_SU_OFFSET

			Offset in units of 4 bytes of 'PHYRX_HE_SIG_A_SU' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 48, 54>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_SU_OFFSET_OFFSET 0x0000000000000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_SU_OFFSET_LSB    32
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_SU_OFFSET_MSB    37
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_SU_OFFSET_MASK   0x0000003f00000000


/* Description		HE_SIG_A_MU_DL_OFFSET

			Offset in units of 4 bytes of 'PHYRX_HE_SIG_A_MU_DL' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 48, 54>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_DL_OFFSET_OFFSET 0x0000000000000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_DL_OFFSET_LSB 38
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_DL_OFFSET_MSB 43
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_DL_OFFSET_MASK 0x00000fc000000000


/* Description		HE_SIG_A_MU_UL_OFFSET

			Offset in units of 4 bytes of 'PHYRX_HE_SIG_A_MU_UL' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 48, 54>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_UL_OFFSET_OFFSET 0x0000000000000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_UL_OFFSET_LSB 44
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_UL_OFFSET_MSB 49
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_A_MU_UL_OFFSET_MASK 0x0003f00000000000


/* Description		GENERIC_U_SIG_OFFSET

			Offset in units of 4 bytes of 'PHYRX_GENERIC_U_SIG' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 48, 54>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERIC_U_SIG_OFFSET_OFFSET 0x0000000000000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERIC_U_SIG_OFFSET_LSB  50
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERIC_U_SIG_OFFSET_MSB  55
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERIC_U_SIG_OFFSET_MASK 0x00fc000000000000


/* Description		RSSI_HT_OFFSET

			Offset in units of 4 bytes of 'PHYRX_RSSI_HT' within 'RX_PPDU_END' 
			Set to zero if the TLV is not included<legal 0, 49-127>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_HT_OFFSET_OFFSET     0x0000000000000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_HT_OFFSET_LSB        56
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_HT_OFFSET_MSB        62
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RSSI_HT_OFFSET_MASK       0x7f00000000000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_1A_OFFSET        0x0000000000000040
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_1A_LSB           63
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_1A_MSB           63
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_1A_MASK          0x8000000000000000


/* Description		VHT_SIG_B_SU20_OFFSET

			Offset in units of 4 bytes of 'PHYRX_VHT_SIG_B_SU20' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 67, 74>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU20_OFFSET_OFFSET 0x0000000000000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU20_OFFSET_LSB 0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU20_OFFSET_MSB 6
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU20_OFFSET_MASK 0x000000000000007f


/* Description		VHT_SIG_B_SU40_OFFSET

			Offset in units of 4 bytes of 'PHYRX_VHT_SIG_B_SU40' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 67, 74>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU40_OFFSET_OFFSET 0x0000000000000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU40_OFFSET_LSB 7
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU40_OFFSET_MSB 13
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU40_OFFSET_MASK 0x0000000000003f80


/* Description		VHT_SIG_B_SU80_OFFSET

			Offset in units of 4 bytes of 'PHYRX_VHT_SIG_B_SU80' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 67, 74>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU80_OFFSET_OFFSET 0x0000000000000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU80_OFFSET_LSB 14
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU80_OFFSET_MSB 20
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU80_OFFSET_MASK 0x00000000001fc000


/* Description		VHT_SIG_B_SU160_OFFSET

			Offset in units of 4 bytes of 'PHYRX_VHT_SIG_B_SU160' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 67, 74>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU160_OFFSET_OFFSET 0x0000000000000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU160_OFFSET_LSB 21
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU160_OFFSET_MSB 27
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_SU160_OFFSET_MASK 0x000000000fe00000


/* Description		RESERVED_2A

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_2A_OFFSET        0x0000000000000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_2A_LSB           28
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_2A_MSB           31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_2A_MASK          0x00000000f0000000


/* Description		VHT_SIG_B_MU20_OFFSET

			Offset in units of 4 bytes of 'PHYRX_VHT_SIG_B_MU20' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 67, 74>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU20_OFFSET_OFFSET 0x0000000000000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU20_OFFSET_LSB 32
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU20_OFFSET_MSB 38
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU20_OFFSET_MASK 0x0000007f00000000


/* Description		VHT_SIG_B_MU40_OFFSET

			Offset in units of 4 bytes of 'PHYRX_VHT_SIG_B_MU40' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 67, 74>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU40_OFFSET_OFFSET 0x0000000000000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU40_OFFSET_LSB 39
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU40_OFFSET_MSB 45
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU40_OFFSET_MASK 0x00003f8000000000


/* Description		VHT_SIG_B_MU80_OFFSET

			Offset in units of 4 bytes of 'PHYRX_VHT_SIG_B_MU80' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 67, 74>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU80_OFFSET_OFFSET 0x0000000000000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU80_OFFSET_LSB 46
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU80_OFFSET_MSB 52
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU80_OFFSET_MASK 0x001fc00000000000


/* Description		VHT_SIG_B_MU160_OFFSET

			Offset in units of 4 bytes of 'PHYRX_VHT_SIG_B_MU160' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 67, 74>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU160_OFFSET_OFFSET 0x0000000000000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU160_OFFSET_LSB 53
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU160_OFFSET_MSB 59
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_VHT_SIG_B_MU160_OFFSET_MASK 0x0fe0000000000000


/* Description		RESERVED_3A

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_3A_OFFSET        0x0000000000000048
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_3A_LSB           60
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_3A_MSB           63
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_3A_MASK          0xf000000000000000


/* Description		HE_SIG_B1_MU_OFFSET

			Offset in units of 4 bytes of 'PHYRX_HE_SIG_B1_MU' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 51, 58>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B1_MU_OFFSET_OFFSET 0x0000000000000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B1_MU_OFFSET_LSB   0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B1_MU_OFFSET_MSB   6
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B1_MU_OFFSET_MASK  0x000000000000007f


/* Description		HE_SIG_B2_MU_OFFSET

			Offset in units of 4 bytes of 'PHYRX_HE_SIG_B2_MU' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 51, 58>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_MU_OFFSET_OFFSET 0x0000000000000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_MU_OFFSET_LSB   7
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_MU_OFFSET_MSB   13
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_MU_OFFSET_MASK  0x0000000000003f80


/* Description		HE_SIG_B2_OFDMA_OFFSET

			Offset in units of 4 bytes of 'PHYRX_HE_SIG_B2_OFDMA' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 53, 62>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_OFDMA_OFFSET_OFFSET 0x0000000000000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_OFDMA_OFFSET_LSB 14
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_OFDMA_OFFSET_MSB 20
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_HE_SIG_B2_OFDMA_OFFSET_MASK 0x00000000001fc000


/* Description		FIRST_GENERIC_EHT_SIG_OFFSET

			Offset in units of 4 bytes of the first 'PHYRX_GENERIC_EHT_SIG' 
			within 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 51, 58>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_GENERIC_EHT_SIG_OFFSET_OFFSET 0x0000000000000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_GENERIC_EHT_SIG_OFFSET_LSB 21
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_GENERIC_EHT_SIG_OFFSET_MSB 27
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_GENERIC_EHT_SIG_OFFSET_MASK 0x000000000fe00000


/* Description		MULTIPLE_GENERIC_EHT_SIG_INCLUDED

			Set to one if more than one 'PHYRX_GENERIC_EHT_SIG' TLVs
			 are included in 'RX_PPDU_END,' set to zero otherwise
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_GENERIC_EHT_SIG_INCLUDED_OFFSET 0x0000000000000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_GENERIC_EHT_SIG_INCLUDED_LSB 28
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_GENERIC_EHT_SIG_INCLUDED_MSB 28
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_GENERIC_EHT_SIG_INCLUDED_MASK 0x0000000010000000


/* Description		RESERVED_4A

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_4A_OFFSET        0x0000000000000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_4A_LSB           29
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_4A_MSB           31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_4A_MASK          0x00000000e0000000


/* Description		COMMON_USER_INFO_OFFSET

			Offset in units of 4 bytes of 'PHYRX_COMMON_USER_INFO' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 0, 46, 50, 67, 70-127>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_COMMON_USER_INFO_OFFSET_OFFSET 0x0000000000000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_COMMON_USER_INFO_OFFSET_LSB 32
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_COMMON_USER_INFO_OFFSET_MSB 38
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_COMMON_USER_INFO_OFFSET_MASK 0x0000007f00000000


/* Description		FIRST_DEBUG_INFO_OFFSET

			Offset in units of 4 bytes of the first 'PHYRX_DEBUG_INFO' 
			within 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_DEBUG_INFO_OFFSET_OFFSET 0x0000000000000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_DEBUG_INFO_OFFSET_LSB 39
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_DEBUG_INFO_OFFSET_MSB 46
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_DEBUG_INFO_OFFSET_MASK 0x00007f8000000000


/* Description		MULTIPLE_DEBUG_INFO_INCLUDED

			Set to one if more than one 'PHYRX_DEBUG_INFO' TLVs are 
			included in 'RX_PPDU_END,' set to zero otherwise<legal all>
			
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_DEBUG_INFO_INCLUDED_OFFSET 0x0000000000000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_DEBUG_INFO_INCLUDED_LSB 47
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_DEBUG_INFO_INCLUDED_MSB 47
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_DEBUG_INFO_INCLUDED_MASK 0x0000800000000000


/* Description		FIRST_OTHER_RECEIVE_INFO_OFFSET

			Offset in units of 4 bytes of the first 'PHYRX_OTHER_RECEIVE_INFO' 
			within 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_OTHER_RECEIVE_INFO_OFFSET_OFFSET 0x0000000000000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_OTHER_RECEIVE_INFO_OFFSET_LSB 48
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_OTHER_RECEIVE_INFO_OFFSET_MSB 55
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_FIRST_OTHER_RECEIVE_INFO_OFFSET_MASK 0x00ff000000000000


/* Description		MULTIPLE_OTHER_RECEIVE_INFO_INCLUDED

			Set to one if more than one 'PHYRX_OTHER_RECEIVE_INFO' TLVs
			 are included in 'RX_PPDU_END,' set to zero otherwise<legal
			 all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_OTHER_RECEIVE_INFO_INCLUDED_OFFSET 0x0000000000000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_OTHER_RECEIVE_INFO_INCLUDED_LSB 56
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_OTHER_RECEIVE_INFO_INCLUDED_MSB 56
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_MULTIPLE_OTHER_RECEIVE_INFO_INCLUDED_MASK 0x0100000000000000


/* Description		RESERVED_5A

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_5A_OFFSET        0x0000000000000050
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_5A_LSB           57
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_5A_MSB           63
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_5A_MASK          0xfe00000000000000


/* Description		DATA_DONE_OFFSET

			Offset in units of 4 bytes of 'PHYRX_DATA_DONE' within 'RX_PPDU_END' 
			Set to zero if the TLV is not included<legal all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_DATA_DONE_OFFSET_OFFSET   0x0000000000000058
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_DATA_DONE_OFFSET_LSB      0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_DATA_DONE_OFFSET_MSB      7
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_DATA_DONE_OFFSET_MASK     0x00000000000000ff


/* Description		GENERATED_CBF_DETAILS_OFFSET

			Offset in units of 4 bytes of 'PHYRX_GENERATED_CBF_DETAILS' 
			within 'RX_PPDU_END'Set to zero if the TLV is not included<legal
			 0, 70-127>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERATED_CBF_DETAILS_OFFSET_OFFSET 0x0000000000000058
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERATED_CBF_DETAILS_OFFSET_LSB 8
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERATED_CBF_DETAILS_OFFSET_MSB 15
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_GENERATED_CBF_DETAILS_OFFSET_MASK 0x000000000000ff00


/* Description		PKT_END_PART1_OFFSET

			Offset in units of 4 bytes of 'PHYRX_PKT_END_PART1' within
			 'RX_PPDU_END' Set to zero if the TLV is not included<legal
			 all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_PART1_OFFSET_OFFSET 0x0000000000000058
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_PART1_OFFSET_LSB  16
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_PART1_OFFSET_MSB  23
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_PART1_OFFSET_MASK 0x0000000000ff0000


/* Description		LOCATION_OFFSET

			Offset in units of 4 bytes of 'PHYRX_LOCATION' within 'RX_PPDU_END' 
			Set to zero if the TLV is not included<legal all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_LOCATION_OFFSET_OFFSET    0x0000000000000058
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_LOCATION_OFFSET_LSB       24
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_LOCATION_OFFSET_MSB       31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_LOCATION_OFFSET_MASK      0x00000000ff000000


/* Description		AZ_INTEGRITY_DATA_OFFSET

			Offset in units of 4 bytes of 'PHYRX_11AZ_INTEGRITY_DATA' 
			within 'RX_PPDU_END'
			 
			Set to zero if the TLV is not included
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_AZ_INTEGRITY_DATA_OFFSET_OFFSET 0x0000000000000058
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_AZ_INTEGRITY_DATA_OFFSET_LSB 32
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_AZ_INTEGRITY_DATA_OFFSET_MSB 39
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_AZ_INTEGRITY_DATA_OFFSET_MASK 0x000000ff00000000


/* Description		PKT_END_OFFSET

			Offset in units of 4 bytes of 'PHYRX_PKT_END' within 'RX_PPDU_END' 
			Set to zero if the TLV is not included<legal all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_OFFSET_OFFSET     0x0000000000000058
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_OFFSET_LSB        40
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_OFFSET_MSB        47
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_PKT_END_OFFSET_MASK       0x0000ff0000000000


/* Description		ABORT_REQUEST_ACK_OFFSET

			Offset in units of 4 bytes of either 'PHYRX_ABORT_REQUEST' 
			or 'PHYRX_ABORT_ACK' within 'RX_PPDU_END'
			
			Set to zero if the TLV is not included
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_ABORT_REQUEST_ACK_OFFSET_OFFSET 0x0000000000000058
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_ABORT_REQUEST_ACK_OFFSET_LSB 48
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_ABORT_REQUEST_ACK_OFFSET_MSB 55
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_ABORT_REQUEST_ACK_OFFSET_MASK 0x00ff000000000000


/* Description		RESERVED_7A

			Spare space in case the widths of the above offsets grow<legal
			 all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_7A_OFFSET        0x0000000000000058
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_7A_LSB           56
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_7A_MSB           63
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_7A_MASK          0xff00000000000000


/* Description		RESERVED_8A

			Spare space in case the widths of the above offsets grow
			
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_8A_OFFSET        0x0000000000000060
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_8A_LSB           0
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_8A_MSB           31
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_8A_MASK          0x00000000ffffffff


/* Description		RESERVED_9A

			Spare space in case the widths of the above offsets grow
			
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_9A_OFFSET        0x0000000000000060
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_9A_LSB           32
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_9A_MSB           63
#define RXPCU_PPDU_END_INFO_RXPCU_PPDU_END_LAYOUT_DETAILS_RESERVED_9A_MASK          0xffffffff00000000


/* Description		CORRUPTED_DUE_TO_FIFO_DELAY

			Set if Rx PCU avoided a hang due to SFM delays by writing
			 a corrupted 'RX_PPDU_END_USER_STATS' and/or 'RX_PPDU_END.'
			
*/

#define RXPCU_PPDU_END_INFO_CORRUPTED_DUE_TO_FIFO_DELAY_OFFSET                      0x0000000000000068
#define RXPCU_PPDU_END_INFO_CORRUPTED_DUE_TO_FIFO_DELAY_LSB                         0
#define RXPCU_PPDU_END_INFO_CORRUPTED_DUE_TO_FIFO_DELAY_MSB                         0
#define RXPCU_PPDU_END_INFO_CORRUPTED_DUE_TO_FIFO_DELAY_MASK                        0x0000000000000001


/* Description		QOS_SN_1_MORE_FRAG_STATE

			Field only valid when QoS_SN_1_info_valid is set.
			
			The 'more fragments' state of the QoS frames with TID QoS_SN_1_TID
			 at the end of this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_1_MORE_FRAG_STATE_OFFSET                         0x0000000000000068
#define RXPCU_PPDU_END_INFO_QOS_SN_1_MORE_FRAG_STATE_LSB                            1
#define RXPCU_PPDU_END_INFO_QOS_SN_1_MORE_FRAG_STATE_MSB                            1
#define RXPCU_PPDU_END_INFO_QOS_SN_1_MORE_FRAG_STATE_MASK                           0x0000000000000002


/* Description		QOS_SN_1_FRAG_NUM_STATE

			Field only valid when QoS_SN_1_info_valid is set.
			
			The 'fragment number' state of the QoS frames with TID QoS_SN_1_TID
			 at the end of this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_1_FRAG_NUM_STATE_OFFSET                          0x0000000000000068
#define RXPCU_PPDU_END_INFO_QOS_SN_1_FRAG_NUM_STATE_LSB                             2
#define RXPCU_PPDU_END_INFO_QOS_SN_1_FRAG_NUM_STATE_MSB                             5
#define RXPCU_PPDU_END_INFO_QOS_SN_1_FRAG_NUM_STATE_MASK                            0x000000000000003c


/* Description		QOS_SN_2_MORE_FRAG_STATE

			Field only valid when QoS_SN_2_info_valid is set.
			
			The 'more fragments' state of the QoS frames with TID QoS_SN_2_TID
			 at the end of this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_2_MORE_FRAG_STATE_OFFSET                         0x0000000000000068
#define RXPCU_PPDU_END_INFO_QOS_SN_2_MORE_FRAG_STATE_LSB                            6
#define RXPCU_PPDU_END_INFO_QOS_SN_2_MORE_FRAG_STATE_MSB                            6
#define RXPCU_PPDU_END_INFO_QOS_SN_2_MORE_FRAG_STATE_MASK                           0x0000000000000040


/* Description		QOS_SN_2_FRAG_NUM_STATE

			Field only valid when QoS_SN_2_info_valid is set.
			
			The 'fragment number' state of the QoS frames with TID QoS_SN_2_TID
			 at the end of this PPDU
			<legal all>
*/

#define RXPCU_PPDU_END_INFO_QOS_SN_2_FRAG_NUM_STATE_OFFSET                          0x0000000000000068
#define RXPCU_PPDU_END_INFO_QOS_SN_2_FRAG_NUM_STATE_LSB                             7
#define RXPCU_PPDU_END_INFO_QOS_SN_2_FRAG_NUM_STATE_MSB                             10
#define RXPCU_PPDU_END_INFO_QOS_SN_2_FRAG_NUM_STATE_MASK                            0x0000000000000780


/* Description		RESERVED_26A

			<legal 0>
*/

#define RXPCU_PPDU_END_INFO_RESERVED_26A_OFFSET                                     0x0000000000000068
#define RXPCU_PPDU_END_INFO_RESERVED_26A_LSB                                        11
#define RXPCU_PPDU_END_INFO_RESERVED_26A_MSB                                        31
#define RXPCU_PPDU_END_INFO_RESERVED_26A_MASK                                       0x00000000fffff800


/* Description		RX_PPDU_END_MARKER

			Field used by SW to double check that their structure alignment
			 is in sync with what HW has done.
			<legal 0xAABBCCDD>
*/

#define RXPCU_PPDU_END_INFO_RX_PPDU_END_MARKER_OFFSET                               0x0000000000000068
#define RXPCU_PPDU_END_INFO_RX_PPDU_END_MARKER_LSB                                  32
#define RXPCU_PPDU_END_INFO_RX_PPDU_END_MARKER_MSB                                  63
#define RXPCU_PPDU_END_INFO_RX_PPDU_END_MARKER_MASK                                 0xffffffff00000000



#endif   // RXPCU_PPDU_END_INFO
