/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	wb_timestamp_lower_32[31:0]
//	1	wb_timestamp_upper_32[31:0]
//	2	rx_antenna[23:0], tx_ht_vht_ack[24], unsupported_mu_nc[25], otp_txbf_disable[26], previous_tlv_corrupted[27], phyrx_abort_request_info_valid[28], macrx_abort_request_info_valid[29], reserved[31:30]
//	3	coex_bt_tx_from_start_of_rx[0], coex_bt_tx_after_start_of_rx[1], coex_wan_tx_from_start_of_rx[2], coex_wan_tx_after_start_of_rx[3], coex_wlan_tx_from_start_of_rx[4], coex_wlan_tx_after_start_of_rx[5], mpdu_delimiter_errors_seen[6], ftm_tm[8:7], dialog_token[16:9], follow_up_dialog_token[24:17], bb_captured_channel[25], bb_captured_reason[28:26], bb_captured_timeout[29], reserved_3[31:30]
//	4	before_mpdu_count_passing_fcs[9:0], before_mpdu_count_failing_fcs[19:10], after_mpdu_count_passing_fcs[29:20], reserved_4[31:30]
//	5	after_mpdu_count_failing_fcs[9:0], reserved_5[31:10]
//	6	phy_timestamp_tx_lower_32[31:0]
//	7	phy_timestamp_tx_upper_32[31:0]
//	8	bb_length[15:0], bb_data[16], reserved_8[19:17], first_bt_broadcast_status_details[31:20]
//	9	rx_ppdu_duration[23:0], reserved_9[31:24]
//	10	ast_index[15:0], ast_index_valid[16], reserved_10[19:17], second_bt_broadcast_status_details[31:20]
//	11	struct phyrx_abort_request_info phyrx_abort_request_info_details;
//	12	struct macrx_abort_request_info macrx_abort_request_info_details;
//	13	rx_ppdu_end_marker[31:0]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RXPCU_PPDU_END_INFO 14

struct rxpcu_ppdu_end_info {
             uint32_t wb_timestamp_lower_32           : 32; //[31:0]
             uint32_t wb_timestamp_upper_32           : 32; //[31:0]
             uint32_t rx_antenna                      : 24, //[23:0]
                      tx_ht_vht_ack                   :  1, //[24]
                      unsupported_mu_nc               :  1, //[25]
                      otp_txbf_disable                :  1, //[26]
                      previous_tlv_corrupted          :  1, //[27]
                      phyrx_abort_request_info_valid  :  1, //[28]
                      macrx_abort_request_info_valid  :  1, //[29]
                      reserved                        :  2; //[31:30]
             uint32_t coex_bt_tx_from_start_of_rx     :  1, //[0]
                      coex_bt_tx_after_start_of_rx    :  1, //[1]
                      coex_wan_tx_from_start_of_rx    :  1, //[2]
                      coex_wan_tx_after_start_of_rx   :  1, //[3]
                      coex_wlan_tx_from_start_of_rx   :  1, //[4]
                      coex_wlan_tx_after_start_of_rx  :  1, //[5]
                      mpdu_delimiter_errors_seen      :  1, //[6]
                      ftm_tm                          :  2, //[8:7]
                      dialog_token                    :  8, //[16:9]
                      follow_up_dialog_token          :  8, //[24:17]
                      bb_captured_channel             :  1, //[25]
                      bb_captured_reason              :  3, //[28:26]
                      bb_captured_timeout             :  1, //[29]
                      reserved_3                      :  2; //[31:30]
             uint32_t before_mpdu_count_passing_fcs   : 10, //[9:0]
                      before_mpdu_count_failing_fcs   : 10, //[19:10]
                      after_mpdu_count_passing_fcs    : 10, //[29:20]
                      reserved_4                      :  2; //[31:30]
             uint32_t after_mpdu_count_failing_fcs    : 10, //[9:0]
                      reserved_5                      : 22; //[31:10]
             uint32_t phy_timestamp_tx_lower_32       : 32; //[31:0]
             uint32_t phy_timestamp_tx_upper_32       : 32; //[31:0]
             uint32_t bb_length                       : 16, //[15:0]
                      bb_data                         :  1, //[16]
                      reserved_8                      :  3, //[19:17]
                      first_bt_broadcast_status_details: 12; //[31:20]
             uint32_t rx_ppdu_duration                : 24, //[23:0]
                      reserved_9                      :  8; //[31:24]
             uint32_t ast_index                       : 16, //[15:0]
                      ast_index_valid                 :  1, //[16]
                      reserved_10                     :  3, //[19:17]
                      second_bt_broadcast_status_details: 12; //[31:20]
    struct            phyrx_abort_request_info                       phyrx_abort_request_info_details;
    struct            macrx_abort_request_info                       macrx_abort_request_info_details;
             uint16_t pre_bt_broadcast_status_details : 12, //[27:16]
                      reserved_12a                    :  4; //[31:28]
             uint32_t rx_ppdu_end_marker              : 32; //[31:0]
};

/*

wb_timestamp_lower_32
			
			WLAN/BT timestamp is a 1 usec resolution timestamp which
			does not get updated based on receive beacon like TSF.  The
			same rules for capturing tsf_timestamp are used to capture
			the wb_timestamp. This field represents the lower 32 bits of
			the 64-bit timestamp

wb_timestamp_upper_32
			
			WLAN/BT timestamp is a 1 usec resolution timestamp which
			does not get updated based on receive beacon like TSF.  The
			same rules for capturing tsf_timestamp are used to capture
			the wb_timestamp. This field represents the upper 32 bits of
			the 64-bit timestamp

rx_antenna
			
			Receive antenna value ???

tx_ht_vht_ack
			
			Indicates that a HT or VHT Ack/BA frame was transmitted
			in response to this receive packet.

unsupported_mu_nc
			
			Set if MU Nc > 2 in received NDPA.
			
			If this bit is set, even though AID and BSSID are
			matched, MAC doesn't send tx_expect_ndp to PHY, because MU
			Nc > 2 is not supported in Helium. 

otp_txbf_disable
			
			Set if either OTP_SUBFEE_DISABLE or OTP_TXBF_DISABLE is
			set and if RXPU receives directed NDPA frame. Then, RXPCU
			should not send TX_EXPECT_NDP TLV to SW but set this bit to
			inform SW. 

previous_tlv_corrupted
			
			When set, the TLV preceding this RXPCU_END_INFO TLV
			within the RX_PPDU_END TLV, is corrupted. Not the entire TLV
			was received.... Likely due to an abort scenario... If abort
			is to blame, see the abort data datastructure for details.
			
			<legal all>

phyrx_abort_request_info_valid
			
			When set, the PHY sent an PHYRX_ABORT_REQUEST TLV to
			RXPCU. The abort fields embedded in this TLV contain valid
			info.
			
			<legal all>

macrx_abort_request_info_valid
			
			When set, the MAC sent an MACRX_ABORT_REQUEST TLV to
			PHYRX. The abort fields embedded in this TLV contain valid
			info.
			
			<legal all>

reserved
			
			<legal 0>

coex_bt_tx_from_start_of_rx
			
			Set when BT TX was ongoing when WLAN RX started

coex_bt_tx_after_start_of_rx
			

coex_wan_tx_from_start_of_rx
			
			Set when WAN TX was ongoing when WLAN RX started

coex_wan_tx_after_start_of_rx
			
			Set when WAN TX started while WLAN RX was already
			ongoing

coex_wlan_tx_from_start_of_rx
			
			Set when other WLAN TX was ongoing when WLAN RX started

coex_wlan_tx_after_start_of_rx
			
			Set when other WLAN TX started while WLAN RX was already
			ongoing

mpdu_delimiter_errors_seen
			
			When set, MPDU delimiter errors have been detected
			during this PPDU reception

ftm_tm
			
			Indicate the timestamp is for the FTM or TM frame 
			
			
			
			0: non TM or FTM frame
			
			1: FTM frame
			
			2: TM frame
			
			3: reserved
			
			<legal all>

dialog_token
			
			The dialog token in the FTM or TM frame. Only valid when
			the FTM is set. Clear to 254 for a non-FTM frame
			
			<legal all>

follow_up_dialog_token
			
			The follow up dialog token in the FTM or TM frame. Only
			valid when the FTM is set. Clear to 0 for a non-FTM frame,
			The follow up dialog token in the FTM frame. Only valid when
			the FTM is set. Clear to 255 for a non-FTM frame<legal all>

bb_captured_channel
			
			Set by RXPCU when MACRX_FREEZE_CAPTURE_CHANNEL TLV is
			sent to PHY, FW check it to correlate current PPDU TLVs with
			uploaded channel information.
			
			
			
			<legal all>

bb_captured_reason
			
			Copy capture_reason of MACRX_FREEZE_CAPTURE_CHANNEL TLV
			to here for FW usage. Valid when bb_captured_channel or
			bb_captured_timeout is set.
			
			
			
			This field indicates why the MAC asked to capture the
			channel
			
			<enum 0 freeze_reason_TM>
			
			<enum 1 freeze_reason_FTM>
			
			<enum 2 freeze_reason_ACK_resp_to_TM_FTM>
			
			<enum 3 freeze_reason_TA_RA_TYPE_FILTER>
			
			<enum 4 freeze_reason_NDPA_NDP>
			
			<enum 5 freeze_reason_ALL_PACKET>
			
			
			
			<legal 0-5>

bb_captured_timeout
			
			Set by RxPCU to indicate channel capture condition is
			meet, but MACRX_FREEZE_CAPTURE_CHANNEL is not sent to PHY
			due to AST long delay, which means the rx_frame_falling edge
			to FREEZE TLV ready time exceed the threshold time defined
			by RXPCU register FREEZE_TLV_DELAY_CNT_THRESH. 
			
			Bb_captured_reason is still valid in this case.
			
			
			
			<legal all>

reserved_3
			
			<legal 0>

before_mpdu_count_passing_fcs
			
			Number of MPDUs received in this PPDU that passed the
			FCS check before the Coex TX started
			
			
			
			The counter saturates at 0x3FF.
			
			<legal all>

before_mpdu_count_failing_fcs
			
			Number of MPDUs received in this PPDU that failed the
			FCS check before the Coex TX started
			
			
			
			The counter saturates at 0x3FF.
			
			<legal all>

after_mpdu_count_passing_fcs
			
			Number of MPDUs received in this PPDU that passed the
			FCS check after the moment the Coex TX started
			
			
			
			(Note: The partially received MPDU when the COEX tx
			start event came in falls in the after category)
			
			
			
			The counter saturates at 0x3FF.
			
			<legal all>

reserved_4
			
			<legal 0>

after_mpdu_count_failing_fcs
			
			Number of MPDUs received in this PPDU that failed the
			FCS check after the moment the Coex TX started
			
			
			
			(Note: The partially received MPDU when the COEX tx
			start event came in falls in the after category)
			
			
			
			The counter saturates at 0x3FF.
			
			<legal all>

reserved_5
			
			<legal 0>

phy_timestamp_tx_lower_32
			
			The PHY timestamp in the AMPI of the most recent rising
			edge (TODO: of what ???) after the TX_PHY_DESC.  This field
			indicates the lower 32 bits of the timestamp

phy_timestamp_tx_upper_32
			
			The PHY timestamp in the AMPI of the most recent rising
			edge (TODO: of what ???) after the TX_PHY_DESC.  This field
			indicates the upper 32 bits of the timestamp

bb_length
			
			Indicates the number of bytes of baseband information
			for PPDUs where the BB descriptor preamble type is 0x80 to
			0xFF which indicates that this is not a normal PPDU but
			rather contains baseband debug information.
			
			TODO: Is this still needed ??? 

bb_data
			
			Indicates that BB data associated with this PPDU will
			exist in the receive buffer.  The exact contents of this BB
			data can be found by decoding the BB TLV in the buffer
			associated with the BB data.  See vector_fragment in the
			Helium_mac_phy_interface.docx

reserved_8
			
			Reserved: HW should fill with 0, FW should ignore.

first_bt_broadcast_status_details
			
			Same contents as field bt_broadcast_status_details for
			the first received COEX_STATUS_BROADCAST tlv during this
			PPDU reception.
			
			
			
			If no COEX_STATUS_BROADCAST tlv is received during this
			PPDU reception, this field will be set to 0
			
			
			
			
			
			For detailed info see doc: TBD
			
			<legal all>

rx_ppdu_duration
			
			The length of this PPDU reception in us

reserved_9
			
			<legal 0>

ast_index
			
			The AST index of the receive Ack/BA.  This information
			is provided from the TXPCU to the RXPCU for receive Ack/BA
			for implicit beamforming.
			
			<legal all>

ast_index_valid
			
			Indicates that ast_index is valid.  Should only be set
			for receive Ack/BA where single stream implicit sounding is
			captured.

reserved_10
			
			<legal 0>

second_bt_broadcast_status_details
			
			Same contents as field bt_broadcast_status_details for
			the second received COEX_STATUS_BROADCAST tlv during this
			PPDU reception.
			
			
			
			If no second COEX_STATUS_BROADCAST tlv is received
			during this PPDU reception, this field will be set to 0
			
			
			
			
			
			For detailed info see doc: TBD
			
			<legal all>

struct phyrx_abort_request_info phyrx_abort_request_info_details
			
			Field only valid when Phyrx_abort_request_info_valid is
			set
			
			The reason why PHY generated an abort request

struct macrx_abort_request_info macrx_abort_request_info_details
			
			Field only valid when macrx_abort_request_info_valid is
			set
			
			The reason why MACRX generated an abort request

rx_ppdu_end_marker
			
			Field used by SW to double check that their structure
			alignment is in sync with what HW has done.
			
			<legal 0xAABBCCDD>
*/


/* Description		RXPCU_PPDU_END_INFO_0_WB_TIMESTAMP_LOWER_32
			
			WLAN/BT timestamp is a 1 usec resolution timestamp which
			does not get updated based on receive beacon like TSF.  The
			same rules for capturing tsf_timestamp are used to capture
			the wb_timestamp. This field represents the lower 32 bits of
			the 64-bit timestamp
*/
#define RXPCU_PPDU_END_INFO_0_WB_TIMESTAMP_LOWER_32_OFFSET           0x00000000
#define RXPCU_PPDU_END_INFO_0_WB_TIMESTAMP_LOWER_32_LSB              0
#define RXPCU_PPDU_END_INFO_0_WB_TIMESTAMP_LOWER_32_MASK             0xffffffff

/* Description		RXPCU_PPDU_END_INFO_1_WB_TIMESTAMP_UPPER_32
			
			WLAN/BT timestamp is a 1 usec resolution timestamp which
			does not get updated based on receive beacon like TSF.  The
			same rules for capturing tsf_timestamp are used to capture
			the wb_timestamp. This field represents the upper 32 bits of
			the 64-bit timestamp
*/
#define RXPCU_PPDU_END_INFO_1_WB_TIMESTAMP_UPPER_32_OFFSET           0x00000004
#define RXPCU_PPDU_END_INFO_1_WB_TIMESTAMP_UPPER_32_LSB              0
#define RXPCU_PPDU_END_INFO_1_WB_TIMESTAMP_UPPER_32_MASK             0xffffffff

/* Description		RXPCU_PPDU_END_INFO_2_RX_ANTENNA
			
			Receive antenna value ???
*/
#define RXPCU_PPDU_END_INFO_2_RX_ANTENNA_OFFSET                      0x00000008
#define RXPCU_PPDU_END_INFO_2_RX_ANTENNA_LSB                         0
#define RXPCU_PPDU_END_INFO_2_RX_ANTENNA_MASK                        0x00ffffff

/* Description		RXPCU_PPDU_END_INFO_2_TX_HT_VHT_ACK
			
			Indicates that a HT or VHT Ack/BA frame was transmitted
			in response to this receive packet.
*/
#define RXPCU_PPDU_END_INFO_2_TX_HT_VHT_ACK_OFFSET                   0x00000008
#define RXPCU_PPDU_END_INFO_2_TX_HT_VHT_ACK_LSB                      24
#define RXPCU_PPDU_END_INFO_2_TX_HT_VHT_ACK_MASK                     0x01000000

/* Description		RXPCU_PPDU_END_INFO_2_UNSUPPORTED_MU_NC
			
			Set if MU Nc > 2 in received NDPA.
			
			If this bit is set, even though AID and BSSID are
			matched, MAC doesn't send tx_expect_ndp to PHY, because MU
			Nc > 2 is not supported in Helium. 
*/
#define RXPCU_PPDU_END_INFO_2_UNSUPPORTED_MU_NC_OFFSET               0x00000008
#define RXPCU_PPDU_END_INFO_2_UNSUPPORTED_MU_NC_LSB                  25
#define RXPCU_PPDU_END_INFO_2_UNSUPPORTED_MU_NC_MASK                 0x02000000

/* Description		RXPCU_PPDU_END_INFO_2_OTP_TXBF_DISABLE
			
			Set if either OTP_SUBFEE_DISABLE or OTP_TXBF_DISABLE is
			set and if RXPU receives directed NDPA frame. Then, RXPCU
			should not send TX_EXPECT_NDP TLV to SW but set this bit to
			inform SW. 
*/
#define RXPCU_PPDU_END_INFO_2_OTP_TXBF_DISABLE_OFFSET                0x00000008
#define RXPCU_PPDU_END_INFO_2_OTP_TXBF_DISABLE_LSB                   26
#define RXPCU_PPDU_END_INFO_2_OTP_TXBF_DISABLE_MASK                  0x04000000

/* Description		RXPCU_PPDU_END_INFO_2_PREVIOUS_TLV_CORRUPTED
			
			When set, the TLV preceding this RXPCU_END_INFO TLV
			within the RX_PPDU_END TLV, is corrupted. Not the entire TLV
			was received.... Likely due to an abort scenario... If abort
			is to blame, see the abort data datastructure for details.
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_2_PREVIOUS_TLV_CORRUPTED_OFFSET          0x00000008
#define RXPCU_PPDU_END_INFO_2_PREVIOUS_TLV_CORRUPTED_LSB             27
#define RXPCU_PPDU_END_INFO_2_PREVIOUS_TLV_CORRUPTED_MASK            0x08000000

/* Description		RXPCU_PPDU_END_INFO_2_PHYRX_ABORT_REQUEST_INFO_VALID
			
			When set, the PHY sent an PHYRX_ABORT_REQUEST TLV to
			RXPCU. The abort fields embedded in this TLV contain valid
			info.
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_2_PHYRX_ABORT_REQUEST_INFO_VALID_OFFSET  0x00000008
#define RXPCU_PPDU_END_INFO_2_PHYRX_ABORT_REQUEST_INFO_VALID_LSB     28
#define RXPCU_PPDU_END_INFO_2_PHYRX_ABORT_REQUEST_INFO_VALID_MASK    0x10000000

/* Description		RXPCU_PPDU_END_INFO_2_MACRX_ABORT_REQUEST_INFO_VALID
			
			When set, the MAC sent an MACRX_ABORT_REQUEST TLV to
			PHYRX. The abort fields embedded in this TLV contain valid
			info.
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_2_MACRX_ABORT_REQUEST_INFO_VALID_OFFSET  0x00000008
#define RXPCU_PPDU_END_INFO_2_MACRX_ABORT_REQUEST_INFO_VALID_LSB     29
#define RXPCU_PPDU_END_INFO_2_MACRX_ABORT_REQUEST_INFO_VALID_MASK    0x20000000

/* Description		RXPCU_PPDU_END_INFO_2_RESERVED
			
			<legal 0>
*/
#define RXPCU_PPDU_END_INFO_2_RESERVED_OFFSET                        0x00000008
#define RXPCU_PPDU_END_INFO_2_RESERVED_LSB                           30
#define RXPCU_PPDU_END_INFO_2_RESERVED_MASK                          0xc0000000

/* Description		RXPCU_PPDU_END_INFO_3_COEX_BT_TX_FROM_START_OF_RX
			
			Set when BT TX was ongoing when WLAN RX started
*/
#define RXPCU_PPDU_END_INFO_3_COEX_BT_TX_FROM_START_OF_RX_OFFSET     0x0000000c
#define RXPCU_PPDU_END_INFO_3_COEX_BT_TX_FROM_START_OF_RX_LSB        0
#define RXPCU_PPDU_END_INFO_3_COEX_BT_TX_FROM_START_OF_RX_MASK       0x00000001

/* Description		RXPCU_PPDU_END_INFO_3_COEX_BT_TX_AFTER_START_OF_RX
			
*/
#define RXPCU_PPDU_END_INFO_3_COEX_BT_TX_AFTER_START_OF_RX_OFFSET    0x0000000c
#define RXPCU_PPDU_END_INFO_3_COEX_BT_TX_AFTER_START_OF_RX_LSB       1
#define RXPCU_PPDU_END_INFO_3_COEX_BT_TX_AFTER_START_OF_RX_MASK      0x00000002

/* Description		RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_FROM_START_OF_RX
			
			Set when WAN TX was ongoing when WLAN RX started
*/
#define RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_FROM_START_OF_RX_OFFSET    0x0000000c
#define RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_FROM_START_OF_RX_LSB       2
#define RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_FROM_START_OF_RX_MASK      0x00000004

/* Description		RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_AFTER_START_OF_RX
			
			Set when WAN TX started while WLAN RX was already
			ongoing
*/
#define RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_AFTER_START_OF_RX_OFFSET   0x0000000c
#define RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_AFTER_START_OF_RX_LSB      3
#define RXPCU_PPDU_END_INFO_3_COEX_WAN_TX_AFTER_START_OF_RX_MASK     0x00000008

/* Description		RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_FROM_START_OF_RX
			
			Set when other WLAN TX was ongoing when WLAN RX started
*/
#define RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_FROM_START_OF_RX_OFFSET   0x0000000c
#define RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_FROM_START_OF_RX_LSB      4
#define RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_FROM_START_OF_RX_MASK     0x00000010

/* Description		RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_AFTER_START_OF_RX
			
			Set when other WLAN TX started while WLAN RX was already
			ongoing
*/
#define RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_AFTER_START_OF_RX_OFFSET  0x0000000c
#define RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_AFTER_START_OF_RX_LSB     5
#define RXPCU_PPDU_END_INFO_3_COEX_WLAN_TX_AFTER_START_OF_RX_MASK    0x00000020

/* Description		RXPCU_PPDU_END_INFO_3_MPDU_DELIMITER_ERRORS_SEEN
			
			When set, MPDU delimiter errors have been detected
			during this PPDU reception
*/
#define RXPCU_PPDU_END_INFO_3_MPDU_DELIMITER_ERRORS_SEEN_OFFSET      0x0000000c
#define RXPCU_PPDU_END_INFO_3_MPDU_DELIMITER_ERRORS_SEEN_LSB         6
#define RXPCU_PPDU_END_INFO_3_MPDU_DELIMITER_ERRORS_SEEN_MASK        0x00000040

/* Description		RXPCU_PPDU_END_INFO_3_FTM_TM
			
			Indicate the timestamp is for the FTM or TM frame 
			
			
			
			0: non TM or FTM frame
			
			1: FTM frame
			
			2: TM frame
			
			3: reserved
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_3_FTM_TM_OFFSET                          0x0000000c
#define RXPCU_PPDU_END_INFO_3_FTM_TM_LSB                             7
#define RXPCU_PPDU_END_INFO_3_FTM_TM_MASK                            0x00000180

/* Description		RXPCU_PPDU_END_INFO_3_DIALOG_TOKEN
			
			The dialog token in the FTM or TM frame. Only valid when
			the FTM is set. Clear to 254 for a non-FTM frame
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_3_DIALOG_TOKEN_OFFSET                    0x0000000c
#define RXPCU_PPDU_END_INFO_3_DIALOG_TOKEN_LSB                       9
#define RXPCU_PPDU_END_INFO_3_DIALOG_TOKEN_MASK                      0x0001fe00

/* Description		RXPCU_PPDU_END_INFO_3_FOLLOW_UP_DIALOG_TOKEN
			
			The follow up dialog token in the FTM or TM frame. Only
			valid when the FTM is set. Clear to 0 for a non-FTM frame,
			The follow up dialog token in the FTM frame. Only valid when
			the FTM is set. Clear to 255 for a non-FTM frame<legal all>
*/
#define RXPCU_PPDU_END_INFO_3_FOLLOW_UP_DIALOG_TOKEN_OFFSET          0x0000000c
#define RXPCU_PPDU_END_INFO_3_FOLLOW_UP_DIALOG_TOKEN_LSB             17
#define RXPCU_PPDU_END_INFO_3_FOLLOW_UP_DIALOG_TOKEN_MASK            0x01fe0000

/* Description		RXPCU_PPDU_END_INFO_3_BB_CAPTURED_CHANNEL
			
			Set by RXPCU when MACRX_FREEZE_CAPTURE_CHANNEL TLV is
			sent to PHY, FW check it to correlate current PPDU TLVs with
			uploaded channel information.
			
			
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_CHANNEL_OFFSET             0x0000000c
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_CHANNEL_LSB                25
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_CHANNEL_MASK               0x02000000

/* Description		RXPCU_PPDU_END_INFO_3_BB_CAPTURED_REASON
			
			Copy capture_reason of MACRX_FREEZE_CAPTURE_CHANNEL TLV
			to here for FW usage. Valid when bb_captured_channel or
			bb_captured_timeout is set.
			
			
			
			This field indicates why the MAC asked to capture the
			channel
			
			<enum 0 freeze_reason_TM>
			
			<enum 1 freeze_reason_FTM>
			
			<enum 2 freeze_reason_ACK_resp_to_TM_FTM>
			
			<enum 3 freeze_reason_TA_RA_TYPE_FILTER>
			
			<enum 4 freeze_reason_NDPA_NDP>
			
			<enum 5 freeze_reason_ALL_PACKET>
			
			
			
			<legal 0-5>
*/
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_REASON_OFFSET              0x0000000c
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_REASON_LSB                 26
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_REASON_MASK                0x1c000000

/* Description		RXPCU_PPDU_END_INFO_3_BB_CAPTURED_TIMEOUT
			
			Set by RxPCU to indicate channel capture condition is
			meet, but MACRX_FREEZE_CAPTURE_CHANNEL is not sent to PHY
			due to AST long delay, which means the rx_frame_falling edge
			to FREEZE TLV ready time exceed the threshold time defined
			by RXPCU register FREEZE_TLV_DELAY_CNT_THRESH. 
			
			Bb_captured_reason is still valid in this case.
			
			
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_TIMEOUT_OFFSET             0x0000000c
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_TIMEOUT_LSB                29
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_TIMEOUT_MASK               0x20000000

/* Description		RXPCU_PPDU_END_INFO_3_RESERVED_3
			
			<legal 0>
*/
#define RXPCU_PPDU_END_INFO_3_RESERVED_3_OFFSET                      0x0000000c
#define RXPCU_PPDU_END_INFO_3_RESERVED_3_LSB                         30
#define RXPCU_PPDU_END_INFO_3_RESERVED_3_MASK                        0xc0000000

/* Description		RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_PASSING_FCS
			
			Number of MPDUs received in this PPDU that passed the
			FCS check before the Coex TX started
			
			
			
			The counter saturates at 0x3FF.
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_PASSING_FCS_OFFSET   0x00000010
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_PASSING_FCS_LSB      0
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_PASSING_FCS_MASK     0x000003ff

/* Description		RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_FAILING_FCS
			
			Number of MPDUs received in this PPDU that failed the
			FCS check before the Coex TX started
			
			
			
			The counter saturates at 0x3FF.
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_FAILING_FCS_OFFSET   0x00000010
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_FAILING_FCS_LSB      10
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_FAILING_FCS_MASK     0x000ffc00

/* Description		RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_PASSING_FCS
			
			Number of MPDUs received in this PPDU that passed the
			FCS check after the moment the Coex TX started
			
			
			
			(Note: The partially received MPDU when the COEX tx
			start event came in falls in the after category)
			
			
			
			The counter saturates at 0x3FF.
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_PASSING_FCS_OFFSET    0x00000010
#define RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_PASSING_FCS_LSB       20
#define RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_PASSING_FCS_MASK      0x3ff00000

/* Description		RXPCU_PPDU_END_INFO_4_RESERVED_4
			
			<legal 0>
*/
#define RXPCU_PPDU_END_INFO_4_RESERVED_4_OFFSET                      0x00000010
#define RXPCU_PPDU_END_INFO_4_RESERVED_4_LSB                         30
#define RXPCU_PPDU_END_INFO_4_RESERVED_4_MASK                        0xc0000000

/* Description		RXPCU_PPDU_END_INFO_5_AFTER_MPDU_COUNT_FAILING_FCS
			
			Number of MPDUs received in this PPDU that failed the
			FCS check after the moment the Coex TX started
			
			
			
			(Note: The partially received MPDU when the COEX tx
			start event came in falls in the after category)
			
			
			
			The counter saturates at 0x3FF.
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_5_AFTER_MPDU_COUNT_FAILING_FCS_OFFSET    0x00000014
#define RXPCU_PPDU_END_INFO_5_AFTER_MPDU_COUNT_FAILING_FCS_LSB       0
#define RXPCU_PPDU_END_INFO_5_AFTER_MPDU_COUNT_FAILING_FCS_MASK      0x000003ff

/* Description		RXPCU_PPDU_END_INFO_5_RESERVED_5
			
			<legal 0>
*/
#define RXPCU_PPDU_END_INFO_5_RESERVED_5_OFFSET                      0x00000014
#define RXPCU_PPDU_END_INFO_5_RESERVED_5_LSB                         10
#define RXPCU_PPDU_END_INFO_5_RESERVED_5_MASK                        0xfffffc00

/* Description		RXPCU_PPDU_END_INFO_6_PHY_TIMESTAMP_TX_LOWER_32
			
			The PHY timestamp in the AMPI of the most recent rising
			edge (TODO: of what ???) after the TX_PHY_DESC.  This field
			indicates the lower 32 bits of the timestamp
*/
#define RXPCU_PPDU_END_INFO_6_PHY_TIMESTAMP_TX_LOWER_32_OFFSET       0x00000018
#define RXPCU_PPDU_END_INFO_6_PHY_TIMESTAMP_TX_LOWER_32_LSB          0
#define RXPCU_PPDU_END_INFO_6_PHY_TIMESTAMP_TX_LOWER_32_MASK         0xffffffff

/* Description		RXPCU_PPDU_END_INFO_7_PHY_TIMESTAMP_TX_UPPER_32
			
			The PHY timestamp in the AMPI of the most recent rising
			edge (TODO: of what ???) after the TX_PHY_DESC.  This field
			indicates the upper 32 bits of the timestamp
*/
#define RXPCU_PPDU_END_INFO_7_PHY_TIMESTAMP_TX_UPPER_32_OFFSET       0x0000001c
#define RXPCU_PPDU_END_INFO_7_PHY_TIMESTAMP_TX_UPPER_32_LSB          0
#define RXPCU_PPDU_END_INFO_7_PHY_TIMESTAMP_TX_UPPER_32_MASK         0xffffffff

/* Description		RXPCU_PPDU_END_INFO_8_BB_LENGTH
			
			Indicates the number of bytes of baseband information
			for PPDUs where the BB descriptor preamble type is 0x80 to
			0xFF which indicates that this is not a normal PPDU but
			rather contains baseband debug information.
			
			TODO: Is this still needed ??? 
*/
#define RXPCU_PPDU_END_INFO_8_BB_LENGTH_OFFSET                       0x00000020
#define RXPCU_PPDU_END_INFO_8_BB_LENGTH_LSB                          0
#define RXPCU_PPDU_END_INFO_8_BB_LENGTH_MASK                         0x0000ffff

/* Description		RXPCU_PPDU_END_INFO_8_BB_DATA
			
			Indicates that BB data associated with this PPDU will
			exist in the receive buffer.  The exact contents of this BB
			data can be found by decoding the BB TLV in the buffer
			associated with the BB data.  See vector_fragment in the
			Helium_mac_phy_interface.docx
*/
#define RXPCU_PPDU_END_INFO_8_BB_DATA_OFFSET                         0x00000020
#define RXPCU_PPDU_END_INFO_8_BB_DATA_LSB                            16
#define RXPCU_PPDU_END_INFO_8_BB_DATA_MASK                           0x00010000

/* Description		RXPCU_PPDU_END_INFO_8_RESERVED_8
			
			Reserved: HW should fill with 0, FW should ignore.
*/
#define RXPCU_PPDU_END_INFO_8_RESERVED_8_OFFSET                      0x00000020
#define RXPCU_PPDU_END_INFO_8_RESERVED_8_LSB                         17
#define RXPCU_PPDU_END_INFO_8_RESERVED_8_MASK                        0x000e0000

/* Description		RXPCU_PPDU_END_INFO_8_FIRST_BT_BROADCAST_STATUS_DETAILS
			
			Same contents as field bt_broadcast_status_details for
			the first received COEX_STATUS_BROADCAST tlv during this
			PPDU reception.
			
			
			
			If no COEX_STATUS_BROADCAST tlv is received during this
			PPDU reception, this field will be set to 0
			
			
			
			
			
			For detailed info see doc: TBD
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_8_FIRST_BT_BROADCAST_STATUS_DETAILS_OFFSET 0x00000020
#define RXPCU_PPDU_END_INFO_8_FIRST_BT_BROADCAST_STATUS_DETAILS_LSB  20
#define RXPCU_PPDU_END_INFO_8_FIRST_BT_BROADCAST_STATUS_DETAILS_MASK 0xfff00000

/* Description		RXPCU_PPDU_END_INFO_9_RX_PPDU_DURATION
			
			The length of this PPDU reception in us
*/
#define RXPCU_PPDU_END_INFO_9_RX_PPDU_DURATION_OFFSET                0x00000024
#define RXPCU_PPDU_END_INFO_9_RX_PPDU_DURATION_LSB                   0
#define RXPCU_PPDU_END_INFO_9_RX_PPDU_DURATION_MASK                  0x00ffffff

/* Description		RXPCU_PPDU_END_INFO_9_RESERVED_9
			
			<legal 0>
*/
#define RXPCU_PPDU_END_INFO_9_RESERVED_9_OFFSET                      0x00000024
#define RXPCU_PPDU_END_INFO_9_RESERVED_9_LSB                         24
#define RXPCU_PPDU_END_INFO_9_RESERVED_9_MASK                        0xff000000

/* Description		RXPCU_PPDU_END_INFO_10_AST_INDEX
			
			The AST index of the receive Ack/BA.  This information
			is provided from the TXPCU to the RXPCU for receive Ack/BA
			for implicit beamforming.
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_10_AST_INDEX_OFFSET                      0x00000028
#define RXPCU_PPDU_END_INFO_10_AST_INDEX_LSB                         0
#define RXPCU_PPDU_END_INFO_10_AST_INDEX_MASK                        0x0000ffff

/* Description		RXPCU_PPDU_END_INFO_10_AST_INDEX_VALID
			
			Indicates that ast_index is valid.  Should only be set
			for receive Ack/BA where single stream implicit sounding is
			captured.
*/
#define RXPCU_PPDU_END_INFO_10_AST_INDEX_VALID_OFFSET                0x00000028
#define RXPCU_PPDU_END_INFO_10_AST_INDEX_VALID_LSB                   16
#define RXPCU_PPDU_END_INFO_10_AST_INDEX_VALID_MASK                  0x00010000

/* Description		RXPCU_PPDU_END_INFO_10_RESERVED_10
			
			<legal 0>
*/
#define RXPCU_PPDU_END_INFO_10_RESERVED_10_OFFSET                    0x00000028
#define RXPCU_PPDU_END_INFO_10_RESERVED_10_LSB                       17
#define RXPCU_PPDU_END_INFO_10_RESERVED_10_MASK                      0x000e0000

/* Description		RXPCU_PPDU_END_INFO_10_SECOND_BT_BROADCAST_STATUS_DETAILS
			
			Same contents as field bt_broadcast_status_details for
			the second received COEX_STATUS_BROADCAST tlv during this
			PPDU reception.
			
			
			
			If no second COEX_STATUS_BROADCAST tlv is received
			during this PPDU reception, this field will be set to 0
			
			
			
			
			
			For detailed info see doc: TBD
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_10_SECOND_BT_BROADCAST_STATUS_DETAILS_OFFSET 0x00000028
#define RXPCU_PPDU_END_INFO_10_SECOND_BT_BROADCAST_STATUS_DETAILS_LSB 20
#define RXPCU_PPDU_END_INFO_10_SECOND_BT_BROADCAST_STATUS_DETAILS_MASK 0xfff00000

 /* EXTERNAL REFERENCE : struct phyrx_abort_request_info phyrx_abort_request_info_details */ 


/* Description		RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON
			
			<enum 0 phyrx_err_phy_off> Reception aborted due to
			receiving a PHY_OFF TLV
			
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
			
			<enum 36 phyrx_err_fdomain_timeout> 
			
			<enum 37 phyrx_err_lsig_rel_check> 
			
			<enum 38 phyrx_err_bt_collision> 
			
			<enum 39 phyrx_err_unsupported_mu_feedback> 
			
			<enum 40 phyrx_err_ppdu_tx_interrupt_rx> 
			
			<enum 41 phyrx_err_unsupported_cbf> 
			
			
			
			<enum 42 phyrx_err_other>  Should not really be used. If
			needed, ask for documentation update 
			
			
			
			<enum 43 phyrx_err_he_siga_unsupported > <enum 44
			phyrx_err_he_crc_error > <enum 45
			phyrx_err_he_sigb_unsupported > <enum 46
			phyrx_err_he_mu_mode_unsupported > <enum 47
			phyrx_err_he_ndp_or_zlf > <enum 48 phyrx_err_he_nsym_lt_zero
			> <enum 49 phyrx_err_he_ru_params_unsupported > <enum 50
			phyrx_err_he_num_users_unsupported ><enum 51
			phyrx_err_he_sounding_params_unsupported >
			
			
			
			<enum 52 phyrx_err_MU_UL_no_power_detected> 
			
			<enum 53 phyrx_err_MU_UL_not_for_me>
			
			
			
			<legal 0 - 53>
*/
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_LSB 0
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHYRX_ABORT_REASON_MASK 0x000000ff

/* Description		RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE
			
			When set, PHY enters PHY NAP state after sending this
			abort
			
			
			
			Note that nap and defer state are mutually exclusive.
			
			
			
			Field put pro-actively in place....usage still to be
			agreed upon.
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_LSB 8
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_NAP_STATE_MASK 0x00000100

/* Description		RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE
			
			When set, PHY enters PHY defer state after sending this
			abort
			
			
			
			Note that nap and defer state are mutually exclusive.
			
			
			
			Field put pro-actively in place....usage still to be
			agreed upon.
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_LSB 9
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_PHY_ENTERS_DEFER_STATE_MASK 0x00000200

/* Description		RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0
			
			<legal 0>
*/
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_LSB 10
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_MASK 0x0000fc00

/* Description		RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION
			
			The remaining receive duration of this PPDU in the
			medium (in us). When PHY does not know this duration when
			this TLV is generated, the field will be set to 0.
			
			The timing reference point is the reception by the MAC
			of this TLV. The value shall be accurate to within 2us.
			
			
			
			In case Phy_enters_nap_state and/or
			Phy_enters_defer_state is set, there is a possibility that
			MAC PMM can also decide to go into a low(er) power state. 
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_LSB 16
#define RXPCU_PPDU_END_INFO_11_PHYRX_ABORT_REQUEST_INFO_DETAILS_RECEIVE_DURATION_MASK 0xffff0000

 /* EXTERNAL REFERENCE : struct macrx_abort_request_info macrx_abort_request_info_details */ 


/* Description		RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON
			
			<enum 0 macrx_abort_sw_initiated>
			
			<enum 1 macrx_abort_obss_reception> Upon receiving this
			abort reason, PHY should stop reception of the current frame
			and go back into a search mode
			
			<enum 2 macrx_abort_other>
			
			<enum 3 macrx_abort_sw_initiated_channel_switch > MAC FW
			issued an abort for channel switch reasons
			
			<enum 4 macrx_abort_sw_initiated_power_save > MAC FW
			issued an abort power save reasons
			
			<enum 5 macrx_abort_too_much_bad_data > RXPCU is
			terminating the current ongoing reception, as the data that
			MAC is receiving seems to be all garbage... The PER is too
			high, or in case of MU UL, Likely the trigger frame never
			got properly received by any of the targeted MU UL devices.
			After the abort, PHYRX can resume a normal search mode.
			
			
			
			<legal 0-5>
*/
#define RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_OFFSET 0x00000030
#define RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_LSB 0
#define RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_MACRX_ABORT_REASON_MASK 0x000000ff

/* Description		RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0
			
			<legal 0>
*/
#define RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_OFFSET 0x00000030
#define RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_LSB 8
#define RXPCU_PPDU_END_INFO_12_MACRX_ABORT_REQUEST_INFO_DETAILS_RESERVED_0_MASK 0x0000ff00

/* Description		RXPCU_PPDU_END_INFO_13_RX_PPDU_END_MARKER
			
			Field used by SW to double check that their structure
			alignment is in sync with what HW has done.
			
			<legal 0xAABBCCDD>
*/
#define RXPCU_PPDU_END_INFO_13_RX_PPDU_END_MARKER_OFFSET             0x00000034
#define RXPCU_PPDU_END_INFO_13_RX_PPDU_END_MARKER_LSB                0
#define RXPCU_PPDU_END_INFO_13_RX_PPDU_END_MARKER_MASK               0xffffffff


#endif // _RXPCU_PPDU_END_INFO_H_
