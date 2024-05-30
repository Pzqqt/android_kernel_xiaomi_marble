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

#ifndef _EXPECTED_RESPONSE_H_
#define _EXPECTED_RESPONSE_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_EXPECTED_RESPONSE 6

#define NUM_OF_QWORDS_EXPECTED_RESPONSE 3


struct expected_response {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t tx_ad2_31_0                                             : 32; // [31:0]
             uint32_t tx_ad2_47_32                                            : 16, // [15:0]
                      expected_response_type                                  :  5, // [20:16]
                      response_to_response                                    :  3, // [23:21]
                      su_ba_user_number                                       :  1, // [24:24]
                      response_info_part2_required                            :  1, // [25:25]
                      transmitted_bssid_check_en                              :  1, // [26:26]
                      reserved_1                                              :  5; // [31:27]
             uint32_t ndp_sta_partial_aid_2_8_0                               : 11, // [10:0]
                      reserved_2                                              : 10, // [20:11]
                      ndp_sta_partial_aid1_8_0                                : 11; // [31:21]
             uint32_t ast_index                                               : 16, // [15:0]
                      capture_ack_ba_sounding                                 :  1, // [16:16]
                      capture_sounding_1str_20mhz                             :  1, // [17:17]
                      capture_sounding_1str_40mhz                             :  1, // [18:18]
                      capture_sounding_1str_80mhz                             :  1, // [19:19]
                      capture_sounding_1str_160mhz                            :  1, // [20:20]
                      capture_sounding_1str_240mhz                            :  1, // [21:21]
                      capture_sounding_1str_320mhz                            :  1, // [22:22]
                      reserved_3a                                             :  9; // [31:23]
             uint32_t fcs                                                     :  9, // [8:0]
                      reserved_4a                                             :  1, // [9:9]
                      crc                                                     :  4, // [13:10]
                      scrambler_seed                                          :  7, // [20:14]
                      reserved_4b                                             : 11; // [31:21]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t tx_ad2_31_0                                             : 32; // [31:0]
             uint32_t reserved_1                                              :  5, // [31:27]
                      transmitted_bssid_check_en                              :  1, // [26:26]
                      response_info_part2_required                            :  1, // [25:25]
                      su_ba_user_number                                       :  1, // [24:24]
                      response_to_response                                    :  3, // [23:21]
                      expected_response_type                                  :  5, // [20:16]
                      tx_ad2_47_32                                            : 16; // [15:0]
             uint32_t ndp_sta_partial_aid1_8_0                                : 11, // [31:21]
                      reserved_2                                              : 10, // [20:11]
                      ndp_sta_partial_aid_2_8_0                               : 11; // [10:0]
             uint32_t reserved_3a                                             :  9, // [31:23]
                      capture_sounding_1str_320mhz                            :  1, // [22:22]
                      capture_sounding_1str_240mhz                            :  1, // [21:21]
                      capture_sounding_1str_160mhz                            :  1, // [20:20]
                      capture_sounding_1str_80mhz                             :  1, // [19:19]
                      capture_sounding_1str_40mhz                             :  1, // [18:18]
                      capture_sounding_1str_20mhz                             :  1, // [17:17]
                      capture_ack_ba_sounding                                 :  1, // [16:16]
                      ast_index                                               : 16; // [15:0]
             uint32_t reserved_4b                                             : 11, // [31:21]
                      scrambler_seed                                          :  7, // [20:14]
                      crc                                                     :  4, // [13:10]
                      reserved_4a                                             :  1, // [9:9]
                      fcs                                                     :  9; // [8:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		TX_AD2_31_0

			Lower 32 bits of the transmitter address (AD2) of the last
			 packet which was transmitted, which is used by RXPCU in
			 Proxy STA mode.
*/

#define EXPECTED_RESPONSE_TX_AD2_31_0_OFFSET                                        0x0000000000000000
#define EXPECTED_RESPONSE_TX_AD2_31_0_LSB                                           0
#define EXPECTED_RESPONSE_TX_AD2_31_0_MSB                                           31
#define EXPECTED_RESPONSE_TX_AD2_31_0_MASK                                          0x00000000ffffffff


/* Description		TX_AD2_47_32

			Upper 16 bits of the transmitter address (AD2) of the last
			 packet which was transmitted, which is used by RXPCU in
			 Proxy STA mode.
*/

#define EXPECTED_RESPONSE_TX_AD2_47_32_OFFSET                                       0x0000000000000000
#define EXPECTED_RESPONSE_TX_AD2_47_32_LSB                                          32
#define EXPECTED_RESPONSE_TX_AD2_47_32_MSB                                          47
#define EXPECTED_RESPONSE_TX_AD2_47_32_MASK                                         0x0000ffff00000000


/* Description		EXPECTED_RESPONSE_TYPE

			Provides insight for RXPCU of what type of response is expected
			 in the medium. 
			
			Mainly used for debugging purposes.
			
			No matter what RXPCU receives, it shall always report it
			 to TXPCU.
			
			Only special scenario where RXPCU will have to generate 
			a RECEIVED_RESPONSE_INFO TLV , even when no actual MPDU 
			with passing FCS was received is when the response_type 
			is set to: frameless_phyrx_response_accepted
			
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
			
			TXPCU expects UL MU OFDMA or UL MU MIMO reception.
			As PDG does not know how RUs are assigned for the uplink
			 portion, PDG can not calculate the uplink duration. Therefor
			 SW shall program the 'Extend_duration_value_bw...' field
			
			
			RXPCU will report any frame received, irrespective of it
			 having been UL MU or SU It is TXPCUs responsibility to 
			distinguish between the UL MU or SU
			
			TXPCU can check in TLV RECEIVED_RESPONSE_INFO MU_Response_BA_bitmap
			 if indeed BA was received
			
			TXPCU expects UL MU OFDMA or UL MU MIMO reception.
			As PDG does not know how RUs are assigned for the uplink
			 portion, PDG can not calculate the uplink duration. Therefor
			 SW shall program the 'Extend_duration_value_bw...' field
			
			
			RXPCU will report any frame received, irrespective of it
			 having been UL MU or SU It is TXPCUs responsibility to 
			distinguish between the UL MU or SU
			
			TXPCU can check in TLV RECEIVED_RESPONSE_INFO, field MU_Response_data_bitmap
			 and MU_Response_BA_bitmap if indeed BA and data was received
			
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

#define EXPECTED_RESPONSE_EXPECTED_RESPONSE_TYPE_OFFSET                             0x0000000000000000
#define EXPECTED_RESPONSE_EXPECTED_RESPONSE_TYPE_LSB                                48
#define EXPECTED_RESPONSE_EXPECTED_RESPONSE_TYPE_MSB                                52
#define EXPECTED_RESPONSE_EXPECTED_RESPONSE_TYPE_MASK                               0x001f000000000000


/* Description		RESPONSE_TO_RESPONSE

			Field indicates if after receiving the PPDU response (indicated
			 in the field above), TXPCU is expected to generate a reponse
			 to the response
			
			In case a response to response is expected, RXPCU shall 
			first acknowledge the proper reception of the received frames, 
			so that TXPCU can first wrapup that portion of the FES.
			
			<enum 0 None> No response after response allowed.
			<enum 1 SU_BA> The response after response that TXPCU is
			 allowed to generate is a single BA. Even if RXPCU is indicating
			 that multiple users are received, TXPCU shall only send
			 a BA for 1 STA. Response_to_response rates can be found
			 in fields 'response_to_response_rate_info_bw...'
			<enum 2 MU_BA> The response after response that TXPCU is
			 allowed to generate is only Multi Destination Multi User
			 BA. Response_to_response rates can be found in fields 'response_to_response_rate_info_bw...'
			
			
			<enum 3 RESPONSE_TO_RESPONSE_CMD> A response to response
			 is expected to be generated. In other words, RXPCU will
			 likely indicate to TXPCU at the end of upcoming reception
			 that a response is needed. TXPCU is however to ignore this
			 indication from RXPCU, and assume for a moment that no 
			response to response is needed, as all the details on how
			 to handle this is provided in the next scheduling command, 
			which is marked as a 'response_to_response' type.
			
			<legal    0-3>
*/

#define EXPECTED_RESPONSE_RESPONSE_TO_RESPONSE_OFFSET                               0x0000000000000000
#define EXPECTED_RESPONSE_RESPONSE_TO_RESPONSE_LSB                                  53
#define EXPECTED_RESPONSE_RESPONSE_TO_RESPONSE_MSB                                  55
#define EXPECTED_RESPONSE_RESPONSE_TO_RESPONSE_MASK                                 0x00e0000000000000


/* Description		SU_BA_USER_NUMBER

			Field only valid when Response_to_response is SU_BA
			
			Indicates the user number of which the BA will be send after
			 receiving the uplink OFDMA.
*/

#define EXPECTED_RESPONSE_SU_BA_USER_NUMBER_OFFSET                                  0x0000000000000000
#define EXPECTED_RESPONSE_SU_BA_USER_NUMBER_LSB                                     56
#define EXPECTED_RESPONSE_SU_BA_USER_NUMBER_MSB                                     56
#define EXPECTED_RESPONSE_SU_BA_USER_NUMBER_MASK                                    0x0100000000000000


/* Description		RESPONSE_INFO_PART2_REQUIRED

			Field only valid when Response_type  is NOT set to No_response_expected
			 
			
			When set to 1, RXPCU shall generate the  RECEIVED_RESPONSE_INFO_PART2
			 TLV after having received the response frame. TXPCU shall
			 wait for this TLV before sending the TX_FES_STATUS_END 
			TLV.
			
			When NOT set, RXPCU shall NOT generate the above mentioned
			 TLV. TXPCU shall not wait for this TLV and after having
			 received  RECEIVED_RESPONSE_INFO  TLV, it can immediately
			 generate the TX_FES_STATUS_END TLV.
			
			<legal all>
*/

#define EXPECTED_RESPONSE_RESPONSE_INFO_PART2_REQUIRED_OFFSET                       0x0000000000000000
#define EXPECTED_RESPONSE_RESPONSE_INFO_PART2_REQUIRED_LSB                          57
#define EXPECTED_RESPONSE_RESPONSE_INFO_PART2_REQUIRED_MSB                          57
#define EXPECTED_RESPONSE_RESPONSE_INFO_PART2_REQUIRED_MASK                         0x0200000000000000


/* Description		TRANSMITTED_BSSID_CHECK_EN

			When set to 1, RXPCU shall assume group addressed frame 
			with Tx_AD2 equal to TBSSID was sent. RxPCU should properly
			 handle receive frame(s) from STA(s) which A1 is TBSSID 
			or any VAPs.When NOT set, RXPCU shall compare received frame's
			 A1 with Tx_AD2 only.
			<legal all>
*/

#define EXPECTED_RESPONSE_TRANSMITTED_BSSID_CHECK_EN_OFFSET                         0x0000000000000000
#define EXPECTED_RESPONSE_TRANSMITTED_BSSID_CHECK_EN_LSB                            58
#define EXPECTED_RESPONSE_TRANSMITTED_BSSID_CHECK_EN_MSB                            58
#define EXPECTED_RESPONSE_TRANSMITTED_BSSID_CHECK_EN_MASK                           0x0400000000000000


/* Description		RESERVED_1

			<legal 0>
*/

#define EXPECTED_RESPONSE_RESERVED_1_OFFSET                                         0x0000000000000000
#define EXPECTED_RESPONSE_RESERVED_1_LSB                                            59
#define EXPECTED_RESPONSE_RESERVED_1_MSB                                            63
#define EXPECTED_RESPONSE_RESERVED_1_MASK                                           0xf800000000000000


/* Description		NDP_STA_PARTIAL_AID_2_8_0

			This field is applicable only in 11ah mode of operation. 
			This field carries the information needed for RxPCU to qualify
			 valid NDP-CTS
			
			When an RTS is being transmitted, this field  provides the
			 partial AID of STA/BSSID of the transmitter,so the received
			 RA/BSSID of the NDP CTS response frame can be compared 
			to validate it. This value is provided by SW for valiadating
			 the NDP CTS. 
			
			This filed also carries information for TA of the NDP Modified
			 ACK when an NDP PS-Poll is transmitted. 
*/

#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID_2_8_0_OFFSET                          0x0000000000000008
#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID_2_8_0_LSB                             0
#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID_2_8_0_MSB                             10
#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID_2_8_0_MASK                            0x00000000000007ff


/* Description		RESERVED_2

			Reserved: Generator should set to 0, consumer shall ignore
			 <legal 0>
*/

#define EXPECTED_RESPONSE_RESERVED_2_OFFSET                                         0x0000000000000008
#define EXPECTED_RESPONSE_RESERVED_2_LSB                                            11
#define EXPECTED_RESPONSE_RESERVED_2_MSB                                            20
#define EXPECTED_RESPONSE_RESERVED_2_MASK                                           0x00000000001ff800


/* Description		NDP_STA_PARTIAL_AID1_8_0

			This field is applicable only in 11ah mode of operation. 
			This field carries the information needed for RxPCU to qualify
			 valid NDP Modified ACK
			
			TxPCU provides the partial AID (RA) of the NDP PS-Poll frame. 
			
*/

#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID1_8_0_OFFSET                           0x0000000000000008
#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID1_8_0_LSB                              21
#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID1_8_0_MSB                              31
#define EXPECTED_RESPONSE_NDP_STA_PARTIAL_AID1_8_0_MASK                             0x00000000ffe00000


/* Description		AST_INDEX

			The AST index of the receive Ack/BA.  This information is
			 provided from the TXPCU to the RXPCU for receive Ack/BA.
			
*/

#define EXPECTED_RESPONSE_AST_INDEX_OFFSET                                          0x0000000000000008
#define EXPECTED_RESPONSE_AST_INDEX_LSB                                             32
#define EXPECTED_RESPONSE_AST_INDEX_MSB                                             47
#define EXPECTED_RESPONSE_AST_INDEX_MASK                                            0x0000ffff00000000


/* Description		CAPTURE_ACK_BA_SOUNDING

			If set enables capture of 1str and 2str sounding on Ack 
			or BA as long as the corresponding capture_sounding_1str_##mhz
			 bits is set.
			
			If clear the capture of sounding on Ack or BA is disabled
			 even if the corresponding capture_sounding_1str_##mhz is
			 set.
*/

#define EXPECTED_RESPONSE_CAPTURE_ACK_BA_SOUNDING_OFFSET                            0x0000000000000008
#define EXPECTED_RESPONSE_CAPTURE_ACK_BA_SOUNDING_LSB                               48
#define EXPECTED_RESPONSE_CAPTURE_ACK_BA_SOUNDING_MSB                               48
#define EXPECTED_RESPONSE_CAPTURE_ACK_BA_SOUNDING_MASK                              0x0001000000000000


/* Description		CAPTURE_SOUNDING_1STR_20MHZ

			Capture sounding for 1 stream 20 MHz receive packets
*/

#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_20MHZ_OFFSET                        0x0000000000000008
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_20MHZ_LSB                           49
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_20MHZ_MSB                           49
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_20MHZ_MASK                          0x0002000000000000


/* Description		CAPTURE_SOUNDING_1STR_40MHZ

			Capture sounding for 1 stream 40 MHz receive packets
*/

#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_40MHZ_OFFSET                        0x0000000000000008
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_40MHZ_LSB                           50
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_40MHZ_MSB                           50
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_40MHZ_MASK                          0x0004000000000000


/* Description		CAPTURE_SOUNDING_1STR_80MHZ

			Capture sounding for 1 stream 80 MHz receive packets
*/

#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_80MHZ_OFFSET                        0x0000000000000008
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_80MHZ_LSB                           51
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_80MHZ_MSB                           51
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_80MHZ_MASK                          0x0008000000000000


/* Description		CAPTURE_SOUNDING_1STR_160MHZ

			Capture sounding for 1 stream 160 MHz receive packets
*/

#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_160MHZ_OFFSET                       0x0000000000000008
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_160MHZ_LSB                          52
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_160MHZ_MSB                          52
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_160MHZ_MASK                         0x0010000000000000


/* Description		CAPTURE_SOUNDING_1STR_240MHZ

			Capture sounding for 1 stream 240 MHz receive packets
*/

#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_240MHZ_OFFSET                       0x0000000000000008
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_240MHZ_LSB                          53
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_240MHZ_MSB                          53
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_240MHZ_MASK                         0x0020000000000000


/* Description		CAPTURE_SOUNDING_1STR_320MHZ

			Capture sounding for 1 stream 320 MHz receive packets
*/

#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_320MHZ_OFFSET                       0x0000000000000008
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_320MHZ_LSB                          54
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_320MHZ_MSB                          54
#define EXPECTED_RESPONSE_CAPTURE_SOUNDING_1STR_320MHZ_MASK                         0x0040000000000000


/* Description		RESERVED_3A

			Reserved: Generator should set to 0, consumer shall ignore
			 <legal 0>
*/

#define EXPECTED_RESPONSE_RESERVED_3A_OFFSET                                        0x0000000000000008
#define EXPECTED_RESPONSE_RESERVED_3A_LSB                                           55
#define EXPECTED_RESPONSE_RESERVED_3A_MSB                                           63
#define EXPECTED_RESPONSE_RESERVED_3A_MASK                                          0xff80000000000000


/* Description		FCS

			Tx Frame's FCS[31:23]
			
			TODO: describe what this is used for ...
			
			For aggregates and NDP frames, this field is reserved and
			 TxPCU should populate this to Zero.
*/

#define EXPECTED_RESPONSE_FCS_OFFSET                                                0x0000000000000010
#define EXPECTED_RESPONSE_FCS_LSB                                                   0
#define EXPECTED_RESPONSE_FCS_MSB                                                   8
#define EXPECTED_RESPONSE_FCS_MASK                                                  0x00000000000001ff


/* Description		RESERVED_4A

			Reserved: Generator should set to 0, consumer shall ignore
			 <legal 0>
*/

#define EXPECTED_RESPONSE_RESERVED_4A_OFFSET                                        0x0000000000000010
#define EXPECTED_RESPONSE_RESERVED_4A_LSB                                           9
#define EXPECTED_RESPONSE_RESERVED_4A_MSB                                           9
#define EXPECTED_RESPONSE_RESERVED_4A_MASK                                          0x0000000000000200


/* Description		CRC

			TODO: describe what this is used for ...
			
			Tx SIG's CRC[3:0]
*/

#define EXPECTED_RESPONSE_CRC_OFFSET                                                0x0000000000000010
#define EXPECTED_RESPONSE_CRC_LSB                                                   10
#define EXPECTED_RESPONSE_CRC_MSB                                                   13
#define EXPECTED_RESPONSE_CRC_MASK                                                  0x0000000000003c00


/* Description		SCRAMBLER_SEED

			TODO: describe what this is used for ...
			
			Tx Frames SERVICE[6:0]
*/

#define EXPECTED_RESPONSE_SCRAMBLER_SEED_OFFSET                                     0x0000000000000010
#define EXPECTED_RESPONSE_SCRAMBLER_SEED_LSB                                        14
#define EXPECTED_RESPONSE_SCRAMBLER_SEED_MSB                                        20
#define EXPECTED_RESPONSE_SCRAMBLER_SEED_MASK                                       0x00000000001fc000


/* Description		RESERVED_4B

			Reserved: Generator should set to 0, consumer shall ignore
			 <legal 0>
*/

#define EXPECTED_RESPONSE_RESERVED_4B_OFFSET                                        0x0000000000000010
#define EXPECTED_RESPONSE_RESERVED_4B_LSB                                           21
#define EXPECTED_RESPONSE_RESERVED_4B_MSB                                           31
#define EXPECTED_RESPONSE_RESERVED_4B_MASK                                          0x00000000ffe00000


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define EXPECTED_RESPONSE_TLV64_PADDING_OFFSET                                      0x0000000000000010
#define EXPECTED_RESPONSE_TLV64_PADDING_LSB                                         32
#define EXPECTED_RESPONSE_TLV64_PADDING_MSB                                         63
#define EXPECTED_RESPONSE_TLV64_PADDING_MASK                                        0xffffffff00000000



#endif   // EXPECTED_RESPONSE
