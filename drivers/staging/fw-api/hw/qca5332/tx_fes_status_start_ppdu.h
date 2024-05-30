
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

 
 
 
 
 
 
 


#ifndef _TX_FES_STATUS_START_PPDU_H_
#define _TX_FES_STATUS_START_PPDU_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_FES_STATUS_START_PPDU 4

#define NUM_OF_QWORDS_TX_FES_STATUS_START_PPDU 2


struct tx_fes_status_start_ppdu {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ppdu_timestamp_lower_32                                 : 32; // [31:0]
             uint32_t ppdu_timestamp_upper_32                                 : 32; // [31:0]
             uint32_t subband_mask                                            : 16, // [15:0]
                      ndp_frame                                               :  2, // [17:16]
                      reserved_2b                                             :  2, // [19:18]
                      coex_based_tx_bw                                        :  3, // [22:20]
                      coex_based_ant_mask                                     :  8, // [30:23]
                      reserved_2c                                             :  1; // [31:31]
             uint32_t coex_based_tx_pwr_shared_ant                            :  8, // [7:0]
                      coex_based_tx_pwr_ant                                   :  8, // [15:8]
                      concurrent_bt_tx                                        :  1, // [16:16]
                      concurrent_wlan_tx                                      :  1, // [17:17]
                      concurrent_wan_tx                                       :  1, // [18:18]
                      concurrent_wan_rx                                       :  1, // [19:19]
                      coex_pwr_reduction_bt                                   :  1, // [20:20]
                      coex_pwr_reduction_wlan                                 :  1, // [21:21]
                      coex_pwr_reduction_wan                                  :  1, // [22:22]
                      coex_result_alt_based                                   :  1, // [23:23]
                      request_packet_bw                                       :  3, // [26:24]
                      response_type                                           :  5; // [31:27]
#else
             uint32_t ppdu_timestamp_lower_32                                 : 32; // [31:0]
             uint32_t ppdu_timestamp_upper_32                                 : 32; // [31:0]
             uint32_t reserved_2c                                             :  1, // [31:31]
                      coex_based_ant_mask                                     :  8, // [30:23]
                      coex_based_tx_bw                                        :  3, // [22:20]
                      reserved_2b                                             :  2, // [19:18]
                      ndp_frame                                               :  2, // [17:16]
                      subband_mask                                            : 16; // [15:0]
             uint32_t response_type                                           :  5, // [31:27]
                      request_packet_bw                                       :  3, // [26:24]
                      coex_result_alt_based                                   :  1, // [23:23]
                      coex_pwr_reduction_wan                                  :  1, // [22:22]
                      coex_pwr_reduction_wlan                                 :  1, // [21:21]
                      coex_pwr_reduction_bt                                   :  1, // [20:20]
                      concurrent_wan_rx                                       :  1, // [19:19]
                      concurrent_wan_tx                                       :  1, // [18:18]
                      concurrent_wlan_tx                                      :  1, // [17:17]
                      concurrent_bt_tx                                        :  1, // [16:16]
                      coex_based_tx_pwr_ant                                   :  8, // [15:8]
                      coex_based_tx_pwr_shared_ant                            :  8; // [7:0]
#endif
};


/* Description		PPDU_TIMESTAMP_LOWER_32

			Global timer value at start of Protection transmission
*/

#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_LOWER_32_OFFSET                     0x0000000000000000
#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_LOWER_32_LSB                        0
#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_LOWER_32_MSB                        31
#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_LOWER_32_MASK                       0x00000000ffffffff


/* Description		PPDU_TIMESTAMP_UPPER_32

			Global timer value at start of Protection transmission
*/

#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_UPPER_32_OFFSET                     0x0000000000000000
#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_UPPER_32_LSB                        32
#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_UPPER_32_MSB                        63
#define TX_FES_STATUS_START_PPDU_PPDU_TIMESTAMP_UPPER_32_MASK                       0xffffffff00000000


/* Description		SUBBAND_MASK

			This mask indicates which 20 Mhz channels are actively used
			 in the BW or puncture pattern selected for transmit.
			
			Bit 0: primary 20 Mhz
			Bit 1: secondary 20 MHz
			Etc.
			
			Hamilton v1 used bits [8:4] for the 'Response_type' expected
			 and bits [10:9] for the 'ndp_frame' field from 'TX_FES_SETUP.'
			
			<legal all>
*/

#define TX_FES_STATUS_START_PPDU_SUBBAND_MASK_OFFSET                                0x0000000000000008
#define TX_FES_STATUS_START_PPDU_SUBBAND_MASK_LSB                                   0
#define TX_FES_STATUS_START_PPDU_SUBBAND_MASK_MSB                                   15
#define TX_FES_STATUS_START_PPDU_SUBBAND_MASK_MASK                                  0x000000000000ffff


/* Description		NDP_FRAME

			Bit copied from the TX_FES_SETUP TLV
			
			<enum 0 no_ndp>No NDP transmission
			<enum 1 beamforming_ndp>Beamforming NDP
			<enum 2 he_ranging_ndp>11az NDP (HE Ranging NDP)
			<enum 3 he_feedback_ndp>Short TB (HE Feedback NDP)
*/

#define TX_FES_STATUS_START_PPDU_NDP_FRAME_OFFSET                                   0x0000000000000008
#define TX_FES_STATUS_START_PPDU_NDP_FRAME_LSB                                      16
#define TX_FES_STATUS_START_PPDU_NDP_FRAME_MSB                                      17
#define TX_FES_STATUS_START_PPDU_NDP_FRAME_MASK                                     0x0000000000030000


/* Description		RESERVED_2B

			<legal 0>
*/

#define TX_FES_STATUS_START_PPDU_RESERVED_2B_OFFSET                                 0x0000000000000008
#define TX_FES_STATUS_START_PPDU_RESERVED_2B_LSB                                    18
#define TX_FES_STATUS_START_PPDU_RESERVED_2B_MSB                                    19
#define TX_FES_STATUS_START_PPDU_RESERVED_2B_MASK                                   0x00000000000c0000


/* Description		COEX_BASED_TX_BW

			Field valid for regular PPDU frame transmission
			
			This is the transmit bandwidth value
			that is granted by Coex.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_BW_OFFSET                            0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_BW_LSB                               20
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_BW_MSB                               22
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_BW_MASK                              0x0000000000700000


/* Description		COEX_BASED_ANT_MASK

			Field valid for regular PPDU or Response frame transmission
			
			
			The antennas allowed to be used for this transmission.
			(Coex is allowed to reduce the number of antennas to be 
			used, but not the number of SS)
			<legal all>
*/

#define TX_FES_STATUS_START_PPDU_COEX_BASED_ANT_MASK_OFFSET                         0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_BASED_ANT_MASK_LSB                            23
#define TX_FES_STATUS_START_PPDU_COEX_BASED_ANT_MASK_MSB                            30
#define TX_FES_STATUS_START_PPDU_COEX_BASED_ANT_MASK_MASK                           0x000000007f800000


/* Description		RESERVED_2C

			<legal 0>
*/

#define TX_FES_STATUS_START_PPDU_RESERVED_2C_OFFSET                                 0x0000000000000008
#define TX_FES_STATUS_START_PPDU_RESERVED_2C_LSB                                    31
#define TX_FES_STATUS_START_PPDU_RESERVED_2C_MSB                                    31
#define TX_FES_STATUS_START_PPDU_RESERVED_2C_MASK                                   0x0000000080000000


/* Description		COEX_BASED_TX_PWR_SHARED_ANT

			Field valid for regular PPDU or Response frame transmission
			
			
			Granted tx power for the shared antenna.
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_SHARED_ANT_OFFSET                0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_SHARED_ANT_LSB                   32
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_SHARED_ANT_MSB                   39
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_SHARED_ANT_MASK                  0x000000ff00000000


/* Description		COEX_BASED_TX_PWR_ANT

			Field valid for regular PPDU or Response frame transmission
			
			
			Granted tx power for the unshared antenna
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_ANT_OFFSET                       0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_ANT_LSB                          40
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_ANT_MSB                          47
#define TX_FES_STATUS_START_PPDU_COEX_BASED_TX_PWR_ANT_MASK                         0x0000ff0000000000


/* Description		CONCURRENT_BT_TX

			Indicate the current TX is concurrent with a BT transmission. 
			This bit is to be copied over into the FES status info. 
			
			<legal all>
*/

#define TX_FES_STATUS_START_PPDU_CONCURRENT_BT_TX_OFFSET                            0x0000000000000008
#define TX_FES_STATUS_START_PPDU_CONCURRENT_BT_TX_LSB                               48
#define TX_FES_STATUS_START_PPDU_CONCURRENT_BT_TX_MSB                               48
#define TX_FES_STATUS_START_PPDU_CONCURRENT_BT_TX_MASK                              0x0001000000000000


/* Description		CONCURRENT_WLAN_TX

			Field valid for regular PPDU or Response frame transmission
			
			
			Indicate the current TX is concurrent with other WLAN transmission. 
			This bit is to be copied over into FES status info. 
			<legal all>
*/

#define TX_FES_STATUS_START_PPDU_CONCURRENT_WLAN_TX_OFFSET                          0x0000000000000008
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WLAN_TX_LSB                             49
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WLAN_TX_MSB                             49
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WLAN_TX_MASK                            0x0002000000000000


/* Description		CONCURRENT_WAN_TX

			Field valid for regular PPDU or Response frame transmission
			
			
			Indicate the current TX is concurrent with WAN transmission. 
			This bit is to be copied over into FES status info. 
			<legal all>
*/

#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_TX_OFFSET                           0x0000000000000008
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_TX_LSB                              50
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_TX_MSB                              50
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_TX_MASK                             0x0004000000000000


/* Description		CONCURRENT_WAN_RX

			Field valid for regular PPDU or Response frame transmission
			
			
			Indicate the current TX is concurrent with WAN reception. 
			This bit is to be copied over into FES status info. 
			<legal all>
*/

#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_RX_OFFSET                           0x0000000000000008
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_RX_LSB                              51
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_RX_MSB                              51
#define TX_FES_STATUS_START_PPDU_CONCURRENT_WAN_RX_MASK                             0x0008000000000000


/* Description		COEX_PWR_REDUCTION_BT

			Field valid for regular or response frame transmission.
			When set, transmit power is reduced due to BT coex reason
			
			<legal all>
*/

#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_BT_OFFSET                       0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_BT_LSB                          52
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_BT_MSB                          52
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_BT_MASK                         0x0010000000000000


/* Description		COEX_PWR_REDUCTION_WLAN

			Field valid for regular or response frame transmission.
			When set, transmit power is reduced due to wlan coex reason
			
			<legal all>
*/

#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WLAN_OFFSET                     0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WLAN_LSB                        53
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WLAN_MSB                        53
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WLAN_MASK                       0x0020000000000000


/* Description		COEX_PWR_REDUCTION_WAN

			Field valid for regular or response frame transmission.
			When set, transmit power is reduced due to wan coex reason
			
			<legal all>
*/

#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WAN_OFFSET                      0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WAN_LSB                         54
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WAN_MSB                         54
#define TX_FES_STATUS_START_PPDU_COEX_PWR_REDUCTION_WAN_MASK                        0x0040000000000000


/* Description		COEX_RESULT_ALT_BASED

			Field valid for regular PPDU or Response frame transmission
			
			
			When set, the resulting Coex transmit parameters are based
			 alternate transmit settings in the TX_RATE_SETTING STRUCT
			 of the original selected BW
			
			When not set, the resulting Coex parameters are based on
			 the default transmit settings in the TX_RATE_SETTING STRUCT
			
			
			<legal all>
*/

#define TX_FES_STATUS_START_PPDU_COEX_RESULT_ALT_BASED_OFFSET                       0x0000000000000008
#define TX_FES_STATUS_START_PPDU_COEX_RESULT_ALT_BASED_LSB                          55
#define TX_FES_STATUS_START_PPDU_COEX_RESULT_ALT_BASED_MSB                          55
#define TX_FES_STATUS_START_PPDU_COEX_RESULT_ALT_BASED_MASK                         0x0080000000000000


/* Description		REQUEST_PACKET_BW

			The requested transmit BW to PDG
			Note that Coex can have changed the actual allowed transmit
			 bandwidth.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define TX_FES_STATUS_START_PPDU_REQUEST_PACKET_BW_OFFSET                           0x0000000000000008
#define TX_FES_STATUS_START_PPDU_REQUEST_PACKET_BW_LSB                              56
#define TX_FES_STATUS_START_PPDU_REQUEST_PACKET_BW_MSB                              58
#define TX_FES_STATUS_START_PPDU_REQUEST_PACKET_BW_MASK                             0x0700000000000000


/* Description		RESPONSE_TYPE

			PPDU transmission Response type expected
			
			<enum 0 no_response_expected>After transmission of this 
			frame, no response in SIFS time is expected
			
			When TXPCU sees this setting, it shall not generated the
			 EXPECTED_RESPONSE TLV.
			
			RXPCU should never see this setting
			<enum 1 ack_expected>An ACK frame is expected as response
			
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 2 ba_64_bitmap_expected>BA with 64 bitmap is expected.
			
			
			PDG DOES NOT use the size info to calculated response duration. 
			The length of the response will have to be programmed by
			 SW in the per-BW 'Expected_ppdu_resp_length' field.
			
			For TXPCU only the fact that it is a BA is important. Actual
			 received BA size is not important
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 3 ba_256_expected>BA with 256 bitmap is expected.
			
			PDG DOES NOT use the size info to calculated response duration. 
			The length of the response will have to be programmed by
			 SW in the per-BW 'Expected_ppdu_resp_length' field.
			
			For TXPCU only the fact that it is a BA is important. Actual
			 received BA size is not important
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 4 actionnoack_expected>SW sets this after sending 
			NDP or BR-Poll. 
			
			As PDG has no idea on how long the reception is going to
			 be, the reception time of the response will have to be 
			programmed by SW in the 'Extend_duration_value_bw...' field
			
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 5 ack_ba_expected>PDG uses the size info and assumes
			 single BA format with ACK and 64 bitmap embedded. 
			If SW expects more bitmaps in case of multi-TID, is shall
			 program the 'Extend_duration_value_bw...' field for additional
			 duration time.
			For TXPCU only the fact that an ACK and/or BA is received
			 is important. Reception of only ACK or BA is also considered
			 a success.
			SW also typically sets this when sending VHT single MPDU. 
			Some chip vendors might send BA rather than ACK in response
			 to VHT single MPDU but still we want to accept BA as well. 
			
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 6 cts_expected>SW sets this after queuing RTS frame
			 as standalone packet and sending it.
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 7 ack_data_expected>SW sets this after sending PS-Poll. 
			
			
			For TXPCU either ACK and/or data reception is considered
			 success.
			PDG basis it's response duration calculation on an ACK. 
			For the data portion, SW shall program the 'Extend_duration_value_bw...' 
			field
			<enum 8 ndp_ack_expected>Reserved for 11ah usage. 
			<enum 9 ndp_modified_ack>Reserved for 11ah usage 
			<enum 10 ndp_ba_expected>Reserved for 11ah usage. 
			<enum 11 ndp_cts_expected>Reserved for 11ah usage
			<enum 12 ndp_ack_or_ndp_modified_ack_expected>Reserved for
			 11ah usage
			<enum 13 ul_mu_ba_expected>NOT SUPPORTED IN NAPIER AX AND
			 HASTINGS
			
			TXPCU expects UL MU OFDMA or UL MU MIMO reception.
			As PDG does not know how RUs are assigned for the uplink
			 portion, PDG can not calculate the uplink duration. Therefor
			 SW shall program the 'Extend_duration_value_bw...' field
			
			
			RXPCU will report any frame received, irrespective of it
			 having been UL MU or SU It is TXPCUs responsibility to 
			distinguish between the UL MU or SU
			
			TXPCU can check in TLV RECEIVED_RESPONSE_INFO MU_Response_BA_bitmap
			 if indeed BA was received
			<enum 14 ul_mu_ba_and_data_expected>NOT SUPPORTED IN NAPIER
			 AX AND HASTINGS
			
			TXPCU expects UL MU OFDMA or UL MU MIMO reception.
			As PDG does not know how RUs are assigned for the uplink
			 portion, PDG can not calculate the uplink duration. Therefor
			 SW shall program the 'Extend_duration_value_bw...' field
			
			
			RXPCU will report any frame received, irrespective of it
			 having been UL MU or SU It is TXPCUs responsibility to 
			distinguish between the UL MU or SU
			
			TXPCU can check in TLV RECEIVED_RESPONSE_INFO, field MU_Response_data_bitmap
			 and MU_Response_BA_bitmap if indeed BA and data was received
			
			<enum 15 ul_mu_cbf_expected>NOT SUPPORTED IN NAPIER AX AND
			 HASTINGS
			
			When selected, CBF frames are expected to be received in
			 MU reception (uplink OFDMA or uplink MIMO)
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received 
			TXPCU can check in TLV RECEIVED_RESPONSE_INFO, field MU_Response_cbf_bitmap
			 if indeed CBF frames were received.
			<enum 16 ul_mu_frames_expected>When selected, MPDU frames
			 are expected in the MU reception (uplink OFDMA or uplink
			 MIMO)
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received 
			
			TXPCU can check in TLV RECEIVED_RESPONSE_INFO, field MU_Response_bitmap
			 if indeed frames were received.
			<enum 17 any_response_to_this_device>Any response expected
			 to be send to this device in SIFS time is acceptable. 
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received 
			
			For TXPCU, UL MU or SU is both acceptable.
			
			Can be used for complex OFDMA scenarios. PDG can not calculate
			 the uplink duration. Therefor SW shall program the 'Extend_duration_value_bw...' 
			field
			<enum 18 any_response_accepted>Any frame in the medium to
			 this or any other device, is acceptable as response. 
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received 
			
			For TXPCU, UL MU or SU is both acceptable.
			
			Can be used for complex OFDMA scenarios. PDG can not calculate
			 the uplink duration. Therefor SW shall program the 'Extend_duration_value_bw...' 
			field
			<enum 19 frameless_phyrx_response_accepted>Any MU frameless
			 reception generated by the PHY is acceptable. 
			
			PHY indicates this type of reception explicitly in TLV PHYRX_RSSI_LEGACY, 
			field Reception_type == reception_is_frameless
			
			RXPCU will report any frame received, irrespective of it
			 having been UL MU or SU.
			
			This can be used for complex MU-MIMO or OFDMA scenarios, 
			like receiving MU-CTS.
			
			PDG can not calculate the uplink duration. Therefor SW shall
			 program the 'Extend_duration_value_bw...' field
			<enum 20 ranging_ndp_and_lmr_expected>SW sets this after
			 sending ranging NDPA followed by NDP as an ISTA and NDP
			 and LMR (Action No Ack) are expected as back-to-back reception
			 in SIFS.
			
			As PDG has no idea on how long the reception is going to
			 be, the reception time of the response will have to be 
			programmed by SW in the 'Extend_duration_value_bw...' field
			
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 21 ba_512_expected>BA with 512 bitmap is expected.
			
			
			PDG DOES NOT use the size info to calculated response duration. 
			The length of the response will have to be programmed by
			 SW in the per-BW 'Expected_ppdu_resp_length' field.
			
			For TXPCU only the fact that it is a BA is important. Actual
			 received BA size is not important
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 22 ba_1024_expected>BA with 1024 bitmap is expected.
			
			
			PDG DOES NOT use the size info to calculated response duration. 
			The length of the response will have to be programmed by
			 SW in the per-BW 'Expected_ppdu_resp_length' field.
			
			For TXPCU only the fact that it is a BA is important. Actual
			 received BA size is not important
			
			RXPCU is just expecting any response. It is TXPCU who checks
			 that the right response was received.
			<enum 23 ul_mu_ranging_cts2s_expected>When selected, CTS2S
			 frames are expected to be received in MU reception (uplink
			 OFDMA)
			
			RXPCU shall check each response for CTS2S and report to 
			TXPCU.
			
			TXPCU can check in the TLV 'RECEIVED_RESPONSE_INFO,' fields
			 'MU_Response_bitmap' and 'TB_Ranging_Resp' if indeed CTS2S
			 frames were received.
			<enum 24 ul_mu_ranging_ndp_expected>When selected, UL NDP
			 frames are expected to be received in MU reception (uplink
			 spatial multiplexing)
			
			RXPCU shall check each response for NDP and report to TXPCU.
			
			
			TXPCU can check in the TLV 'RECEIVED_RESPONSE_INFO,' fields
			 'MU_Response_bitmap' and 'TB_Ranging_Resp' if indeed NDP
			 frames were received.
			<enum 25 ul_mu_ranging_lmr_expected>When selected, LMR frames
			 are expected to be received in MU reception (uplink OFDMA
			 or uplink MIMO)
			
			RXPCU shall check each response for LMR and report to TXPCU.
			
			
			TXPCU can check in the TLV 'RECEIVED_RESPONSE_INFO,' fields
			 'MU_Response_bitmap' and 'TB_Ranging_Resp' if indeed LMR
			 frames were received.
*/

#define TX_FES_STATUS_START_PPDU_RESPONSE_TYPE_OFFSET                               0x0000000000000008
#define TX_FES_STATUS_START_PPDU_RESPONSE_TYPE_LSB                                  59
#define TX_FES_STATUS_START_PPDU_RESPONSE_TYPE_MSB                                  63
#define TX_FES_STATUS_START_PPDU_RESPONSE_TYPE_MASK                                 0xf800000000000000



#endif   // TX_FES_STATUS_START_PPDU
