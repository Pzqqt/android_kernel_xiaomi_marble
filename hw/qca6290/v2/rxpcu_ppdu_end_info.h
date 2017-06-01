/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	wb_timestamp_lower_32[31:0]
//	1	wb_timestamp_upper_32[31:0]
//	2	rx_antenna[23:0], tx_ht_vht_ack[24], unsupported_mu_nc[25], otp_txbf_disable[26], previous_tlv_corrupted[27], phyrx_abort_request_info_valid[28], macrx_abort_request_info_valid[29], reserved[31:30]
//	3	coex_bt_tx_from_start_of_rx[0], coex_bt_tx_after_start_of_rx[1], coex_wan_tx_from_start_of_rx[2], coex_wan_tx_after_start_of_rx[3], coex_wlan_tx_from_start_of_rx[4], coex_wlan_tx_after_start_of_rx[5], mpdu_delimiter_errors_seen[6], ftm_tm[8:7], dialog_token[16:9], follow_up_dialog_token[24:17], bb_captured_channel[25], reserved_3[31:26]
//	4	before_mpdu_count_passing_fcs[7:0], before_mpdu_count_failing_fcs[15:8], after_mpdu_count_passing_fcs[23:16], after_mpdu_count_failing_fcs[31:24]
//	5	phy_timestamp_tx_lower_32[31:0]
//	6	phy_timestamp_tx_upper_32[31:0]
//	7	bb_length[15:0], bb_data[16], reserved_7[31:17]
//	8	rx_ppdu_duration[23:0], reserved_8[31:24]
//	9	ast_index[15:0], ast_index_valid[16], reserved_9[31:17]
//	10	struct phyrx_abort_request_info phyrx_abort_request_info_details;
//	11	struct macrx_abort_request_info macrx_abort_request_info_details;
//	12	rx_ppdu_end_marker[31:0]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RXPCU_PPDU_END_INFO 13

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
                      reserved_3                      :  6; //[31:26]
             uint32_t before_mpdu_count_passing_fcs   :  8, //[7:0]
                      before_mpdu_count_failing_fcs   :  8, //[15:8]
                      after_mpdu_count_passing_fcs    :  8, //[23:16]
                      after_mpdu_count_failing_fcs    :  8; //[31:24]
             uint32_t phy_timestamp_tx_lower_32       : 32; //[31:0]
             uint32_t phy_timestamp_tx_upper_32       : 32; //[31:0]
             uint32_t bb_length                       : 16, //[15:0]
                      bb_data                         :  1, //[16]
                      reserved_7                      : 15; //[31:17]
             uint32_t rx_ppdu_duration                : 24, //[23:0]
                      reserved_8                      :  8; //[31:24]
             uint32_t ast_index                       : 16, //[15:0]
                      ast_index_valid                 :  1, //[16]
                      reserved_9                      : 15; //[31:17]
    struct            phyrx_abort_request_info                       phyrx_abort_request_info_details;
    struct            macrx_abort_request_info                       macrx_abort_request_info_details;
             uint16_t reserved_after_struct16         : 16; //[31:16]
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
			
			When set, the PHY sent an PHYRX_ABORT_REQUEST TLV to
			RXPCU. The abort fields embedded in this TLV contain valid
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
			
			Set by RXPCU when the following conditions are met:
			
			
			
			Directed (=> unicast) TM or FTM frame has been received
			with passing FCS
			
			PHYRX_PKT_END. Location_info_valid is set
			
			
			
			<legal all>

reserved_3
			
			<legal 0>

before_mpdu_count_passing_fcs
			
			Number of MPDUs received in this PPDU that passed the
			FCS check before the Coex TX started

before_mpdu_count_failing_fcs
			
			Number of MPDUs received in this PPDU that failed the
			FCS check before the Coex TX started

after_mpdu_count_passing_fcs
			
			Number of MPDUs received in this PPDU that passed the
			FCS check after the moment the Coex TX started
			
			
			
			(Note: The partially received MPDU when the COEX tx
			start event came in falls in the after category)

after_mpdu_count_failing_fcs
			
			Number of MPDUs received in this PPDU that failed the
			FCS check after the moment the Coex TX started
			
			
			
			(Note: The partially received MPDU when the COEX tx
			start event came in falls in the after category)

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

reserved_7
			
			Reserved: HW should fill with 0, FW should ignore.

rx_ppdu_duration
			
			The length of this PPDU reception in us

reserved_8
			
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

reserved_9
			
			<legal 0>

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
			
			When set, the PHY sent an PHYRX_ABORT_REQUEST TLV to
			RXPCU. The abort fields embedded in this TLV contain valid
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
			
			Set by RXPCU when the following conditions are met:
			
			
			
			Directed (=> unicast) TM or FTM frame has been received
			with passing FCS
			
			PHYRX_PKT_END. Location_info_valid is set
			
			
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_CHANNEL_OFFSET             0x0000000c
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_CHANNEL_LSB                25
#define RXPCU_PPDU_END_INFO_3_BB_CAPTURED_CHANNEL_MASK               0x02000000

/* Description		RXPCU_PPDU_END_INFO_3_RESERVED_3
			
			<legal 0>
*/
#define RXPCU_PPDU_END_INFO_3_RESERVED_3_OFFSET                      0x0000000c
#define RXPCU_PPDU_END_INFO_3_RESERVED_3_LSB                         26
#define RXPCU_PPDU_END_INFO_3_RESERVED_3_MASK                        0xfc000000

/* Description		RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_PASSING_FCS
			
			Number of MPDUs received in this PPDU that passed the
			FCS check before the Coex TX started
*/
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_PASSING_FCS_OFFSET   0x00000010
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_PASSING_FCS_LSB      0
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_PASSING_FCS_MASK     0x000000ff

/* Description		RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_FAILING_FCS
			
			Number of MPDUs received in this PPDU that failed the
			FCS check before the Coex TX started
*/
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_FAILING_FCS_OFFSET   0x00000010
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_FAILING_FCS_LSB      8
#define RXPCU_PPDU_END_INFO_4_BEFORE_MPDU_COUNT_FAILING_FCS_MASK     0x0000ff00

/* Description		RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_PASSING_FCS
			
			Number of MPDUs received in this PPDU that passed the
			FCS check after the moment the Coex TX started
			
			
			
			(Note: The partially received MPDU when the COEX tx
			start event came in falls in the after category)
*/
#define RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_PASSING_FCS_OFFSET    0x00000010
#define RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_PASSING_FCS_LSB       16
#define RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_PASSING_FCS_MASK      0x00ff0000

/* Description		RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_FAILING_FCS
			
			Number of MPDUs received in this PPDU that failed the
			FCS check after the moment the Coex TX started
			
			
			
			(Note: The partially received MPDU when the COEX tx
			start event came in falls in the after category)
*/
#define RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_FAILING_FCS_OFFSET    0x00000010
#define RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_FAILING_FCS_LSB       24
#define RXPCU_PPDU_END_INFO_4_AFTER_MPDU_COUNT_FAILING_FCS_MASK      0xff000000

/* Description		RXPCU_PPDU_END_INFO_5_PHY_TIMESTAMP_TX_LOWER_32
			
			The PHY timestamp in the AMPI of the most recent rising
			edge (TODO: of what ???) after the TX_PHY_DESC.  This field
			indicates the lower 32 bits of the timestamp
*/
#define RXPCU_PPDU_END_INFO_5_PHY_TIMESTAMP_TX_LOWER_32_OFFSET       0x00000014
#define RXPCU_PPDU_END_INFO_5_PHY_TIMESTAMP_TX_LOWER_32_LSB          0
#define RXPCU_PPDU_END_INFO_5_PHY_TIMESTAMP_TX_LOWER_32_MASK         0xffffffff

/* Description		RXPCU_PPDU_END_INFO_6_PHY_TIMESTAMP_TX_UPPER_32
			
			The PHY timestamp in the AMPI of the most recent rising
			edge (TODO: of what ???) after the TX_PHY_DESC.  This field
			indicates the upper 32 bits of the timestamp
*/
#define RXPCU_PPDU_END_INFO_6_PHY_TIMESTAMP_TX_UPPER_32_OFFSET       0x00000018
#define RXPCU_PPDU_END_INFO_6_PHY_TIMESTAMP_TX_UPPER_32_LSB          0
#define RXPCU_PPDU_END_INFO_6_PHY_TIMESTAMP_TX_UPPER_32_MASK         0xffffffff

/* Description		RXPCU_PPDU_END_INFO_7_BB_LENGTH
			
			Indicates the number of bytes of baseband information
			for PPDUs where the BB descriptor preamble type is 0x80 to
			0xFF which indicates that this is not a normal PPDU but
			rather contains baseband debug information.
			
			TODO: Is this still needed ??? 
*/
#define RXPCU_PPDU_END_INFO_7_BB_LENGTH_OFFSET                       0x0000001c
#define RXPCU_PPDU_END_INFO_7_BB_LENGTH_LSB                          0
#define RXPCU_PPDU_END_INFO_7_BB_LENGTH_MASK                         0x0000ffff

/* Description		RXPCU_PPDU_END_INFO_7_BB_DATA
			
			Indicates that BB data associated with this PPDU will
			exist in the receive buffer.  The exact contents of this BB
			data can be found by decoding the BB TLV in the buffer
			associated with the BB data.  See vector_fragment in the
			Helium_mac_phy_interface.docx
*/
#define RXPCU_PPDU_END_INFO_7_BB_DATA_OFFSET                         0x0000001c
#define RXPCU_PPDU_END_INFO_7_BB_DATA_LSB                            16
#define RXPCU_PPDU_END_INFO_7_BB_DATA_MASK                           0x00010000

/* Description		RXPCU_PPDU_END_INFO_7_RESERVED_7
			
			Reserved: HW should fill with 0, FW should ignore.
*/
#define RXPCU_PPDU_END_INFO_7_RESERVED_7_OFFSET                      0x0000001c
#define RXPCU_PPDU_END_INFO_7_RESERVED_7_LSB                         17
#define RXPCU_PPDU_END_INFO_7_RESERVED_7_MASK                        0xfffe0000

/* Description		RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION
			
			The length of this PPDU reception in us
*/
#define RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_OFFSET                0x00000020
#define RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_LSB                   0
#define RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_MASK                  0x00ffffff

/* Description		RXPCU_PPDU_END_INFO_8_RESERVED_8
			
			<legal 0>
*/
#define RXPCU_PPDU_END_INFO_8_RESERVED_8_OFFSET                      0x00000020
#define RXPCU_PPDU_END_INFO_8_RESERVED_8_LSB                         24
#define RXPCU_PPDU_END_INFO_8_RESERVED_8_MASK                        0xff000000

/* Description		RXPCU_PPDU_END_INFO_9_AST_INDEX
			
			The AST index of the receive Ack/BA.  This information
			is provided from the TXPCU to the RXPCU for receive Ack/BA
			for implicit beamforming.
			
			<legal all>
*/
#define RXPCU_PPDU_END_INFO_9_AST_INDEX_OFFSET                       0x00000024
#define RXPCU_PPDU_END_INFO_9_AST_INDEX_LSB                          0
#define RXPCU_PPDU_END_INFO_9_AST_INDEX_MASK                         0x0000ffff

/* Description		RXPCU_PPDU_END_INFO_9_AST_INDEX_VALID
			
			Indicates that ast_index is valid.  Should only be set
			for receive Ack/BA where single stream implicit sounding is
			captured.
*/
#define RXPCU_PPDU_END_INFO_9_AST_INDEX_VALID_OFFSET                 0x00000024
#define RXPCU_PPDU_END_INFO_9_AST_INDEX_VALID_LSB                    16
#define RXPCU_PPDU_END_INFO_9_AST_INDEX_VALID_MASK                   0x00010000

/* Description		RXPCU_PPDU_END_INFO_9_RESERVED_9
			
			<legal 0>
*/
#define RXPCU_PPDU_END_INFO_9_RESERVED_9_OFFSET                      0x00000024
#define RXPCU_PPDU_END_INFO_9_RESERVED_9_LSB                         17
#define RXPCU_PPDU_END_INFO_9_RESERVED_9_MASK                        0xfffe0000
#define RXPCU_PPDU_END_INFO_10_PHYRX_ABORT_REQUEST_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_OFFSET 0x00000028
#define RXPCU_PPDU_END_INFO_10_PHYRX_ABORT_REQUEST_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_LSB 17
#define RXPCU_PPDU_END_INFO_10_PHYRX_ABORT_REQUEST_INFO_PHYRX_ABORT_REQUEST_INFO_DETAILS_MASK 0xffffffff
#define RXPCU_PPDU_END_INFO_11_MACRX_ABORT_REQUEST_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_OFFSET 0x0000002c
#define RXPCU_PPDU_END_INFO_11_MACRX_ABORT_REQUEST_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_LSB 17
#define RXPCU_PPDU_END_INFO_11_MACRX_ABORT_REQUEST_INFO_MACRX_ABORT_REQUEST_INFO_DETAILS_MASK 0xffffffff

/* Description		RXPCU_PPDU_END_INFO_12_RX_PPDU_END_MARKER
			
			Field used by SW to double check that their structure
			alignment is in sync with what HW has done.
			
			<legal 0xAABBCCDD>
*/
#define RXPCU_PPDU_END_INFO_12_RX_PPDU_END_MARKER_OFFSET             0x00000030
#define RXPCU_PPDU_END_INFO_12_RX_PPDU_END_MARKER_LSB                0
#define RXPCU_PPDU_END_INFO_12_RX_PPDU_END_MARKER_MASK               0xffffffff


#endif // _RXPCU_PPDU_END_INFO_H_
