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

#ifndef _PCU_PPDU_SETUP_INIT_H_
#define _PCU_PPDU_SETUP_INIT_H_
#if !defined(__ASSEMBLER__)
#endif

#include "pdg_response_rate_setting.h"
#define NUM_OF_DWORDS_PCU_PPDU_SETUP_INIT 58

#define NUM_OF_QWORDS_PCU_PPDU_SETUP_INIT 29


struct pcu_ppdu_setup_init {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t medium_prot_type                                        :  3, // [2:0]
                      response_type                                           :  5, // [7:3]
                      response_info_part2_required                            :  1, // [8:8]
                      response_to_response                                    :  3, // [11:9]
                      mba_user_order                                          :  2, // [13:12]
                      expected_mba_size                                       : 11, // [24:14]
                      required_ul_mu_resp_user_count                          :  6, // [30:25]
                      transmitted_bssid_check_en                              :  1; // [31:31]
             uint32_t mprot_required_bw1                                      :  1, // [0:0]
                      mprot_required_bw20                                     :  1, // [1:1]
                      mprot_required_bw40                                     :  1, // [2:2]
                      mprot_required_bw80                                     :  1, // [3:3]
                      mprot_required_bw160                                    :  1, // [4:4]
                      mprot_required_bw240                                    :  1, // [5:5]
                      mprot_required_bw320                                    :  1, // [6:6]
                      ppdu_allowed_bw1                                        :  1, // [7:7]
                      ppdu_allowed_bw20                                       :  1, // [8:8]
                      ppdu_allowed_bw40                                       :  1, // [9:9]
                      ppdu_allowed_bw80                                       :  1, // [10:10]
                      ppdu_allowed_bw160                                      :  1, // [11:11]
                      ppdu_allowed_bw240                                      :  1, // [12:12]
                      ppdu_allowed_bw320                                      :  1, // [13:13]
                      set_fc_pwr_mgt                                          :  1, // [14:14]
                      use_cts_duration_for_data_tx                            :  1, // [15:15]
                      update_timestamp_64                                     :  1, // [16:16]
                      update_timestamp_32_lower                               :  1, // [17:17]
                      update_timestamp_32_upper                               :  1, // [18:18]
                      reserved_1a                                             : 13; // [31:19]
             uint32_t insert_timestamp_offset_0                               : 16, // [15:0]
                      insert_timestamp_offset_1                               : 16; // [31:16]
             uint32_t max_bw40_try_count                                      :  4, // [3:0]
                      max_bw80_try_count                                      :  4, // [7:4]
                      max_bw160_try_count                                     :  4, // [11:8]
                      max_bw240_try_count                                     :  4, // [15:12]
                      max_bw320_try_count                                     :  4, // [19:16]
                      insert_wur_timestamp_offset                             :  6, // [25:20]
                      update_wur_timestamp                                    :  1, // [26:26]
                      wur_embedded_bssid_present                              :  1, // [27:27]
                      insert_wur_fcs                                          :  1, // [28:28]
                      reserved_3b                                             :  3; // [31:29]
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw20;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw40;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw80;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw160;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw240;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw320;
             uint32_t r2r_hw_response_tx_duration                             : 16, // [15:0]
                      r2r_rx_duration_field                                   : 16; // [31:16]
             uint32_t r2r_group_id                                            :  6, // [5:0]
                      r2r_response_frame_type                                 :  4, // [9:6]
                      r2r_sta_partial_aid                                     : 11, // [20:10]
                      use_address_fields_for_protection                       :  1, // [21:21]
                      r2r_set_required_response_time                          :  1, // [22:22]
                      reserved_29a                                            :  3, // [25:23]
                      r2r_bw20_active_channel                                 :  3, // [28:26]
                      r2r_bw40_active_channel                                 :  3; // [31:29]
             uint32_t r2r_bw80_active_channel                                 :  3, // [2:0]
                      r2r_bw160_active_channel                                :  3, // [5:3]
                      r2r_bw240_active_channel                                :  3, // [8:6]
                      r2r_bw320_active_channel                                :  3, // [11:9]
                      r2r_bw20                                                :  3, // [14:12]
                      r2r_bw40                                                :  3, // [17:15]
                      r2r_bw80                                                :  3, // [20:18]
                      r2r_bw160                                               :  3, // [23:21]
                      r2r_bw240                                               :  3, // [26:24]
                      r2r_bw320                                               :  3, // [29:27]
                      reserved_30a                                            :  2; // [31:30]
             uint32_t mu_response_expected_bitmap_31_0                        : 32; // [31:0]
             uint32_t mu_response_expected_bitmap_36_32                       :  5, // [4:0]
                      mu_expected_response_cbf_count                          :  6, // [10:5]
                      mu_expected_response_sta_count                          :  6, // [16:11]
                      transmit_includes_multidestination                      :  1, // [17:17]
                      insert_prev_tx_start_timing_info                        :  1, // [18:18]
                      insert_current_tx_start_timing_info                     :  1, // [19:19]
                      tx_start_transmit_time_byte_offset                      : 12; // [31:20]
             uint32_t protection_frame_ad1_31_0                               : 32; // [31:0]
             uint32_t protection_frame_ad1_47_32                              : 16, // [15:0]
                      protection_frame_ad2_15_0                               : 16; // [31:16]
             uint32_t protection_frame_ad2_47_16                              : 32; // [31:0]
             uint32_t dynamic_medium_prot_threshold                           : 24, // [23:0]
                      dynamic_medium_prot_type                                :  1, // [24:24]
                      reserved_54a                                            :  7; // [31:25]
             uint32_t protection_frame_ad3_31_0                               : 32; // [31:0]
             uint32_t protection_frame_ad3_47_32                              : 16, // [15:0]
                      protection_frame_ad4_15_0                               : 16; // [31:16]
             uint32_t protection_frame_ad4_47_16                              : 32; // [31:0]
#else
             uint32_t transmitted_bssid_check_en                              :  1, // [31:31]
                      required_ul_mu_resp_user_count                          :  6, // [30:25]
                      expected_mba_size                                       : 11, // [24:14]
                      mba_user_order                                          :  2, // [13:12]
                      response_to_response                                    :  3, // [11:9]
                      response_info_part2_required                            :  1, // [8:8]
                      response_type                                           :  5, // [7:3]
                      medium_prot_type                                        :  3; // [2:0]
             uint32_t reserved_1a                                             : 13, // [31:19]
                      update_timestamp_32_upper                               :  1, // [18:18]
                      update_timestamp_32_lower                               :  1, // [17:17]
                      update_timestamp_64                                     :  1, // [16:16]
                      use_cts_duration_for_data_tx                            :  1, // [15:15]
                      set_fc_pwr_mgt                                          :  1, // [14:14]
                      ppdu_allowed_bw320                                      :  1, // [13:13]
                      ppdu_allowed_bw240                                      :  1, // [12:12]
                      ppdu_allowed_bw160                                      :  1, // [11:11]
                      ppdu_allowed_bw80                                       :  1, // [10:10]
                      ppdu_allowed_bw40                                       :  1, // [9:9]
                      ppdu_allowed_bw20                                       :  1, // [8:8]
                      ppdu_allowed_bw1                                        :  1, // [7:7]
                      mprot_required_bw320                                    :  1, // [6:6]
                      mprot_required_bw240                                    :  1, // [5:5]
                      mprot_required_bw160                                    :  1, // [4:4]
                      mprot_required_bw80                                     :  1, // [3:3]
                      mprot_required_bw40                                     :  1, // [2:2]
                      mprot_required_bw20                                     :  1, // [1:1]
                      mprot_required_bw1                                      :  1; // [0:0]
             uint32_t insert_timestamp_offset_1                               : 16, // [31:16]
                      insert_timestamp_offset_0                               : 16; // [15:0]
             uint32_t reserved_3b                                             :  3, // [31:29]
                      insert_wur_fcs                                          :  1, // [28:28]
                      wur_embedded_bssid_present                              :  1, // [27:27]
                      update_wur_timestamp                                    :  1, // [26:26]
                      insert_wur_timestamp_offset                             :  6, // [25:20]
                      max_bw320_try_count                                     :  4, // [19:16]
                      max_bw240_try_count                                     :  4, // [15:12]
                      max_bw160_try_count                                     :  4, // [11:8]
                      max_bw80_try_count                                      :  4, // [7:4]
                      max_bw40_try_count                                      :  4; // [3:0]
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw20;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw40;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw80;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw160;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw240;
             struct   pdg_response_rate_setting                                 response_to_response_rate_info_bw320;
             uint32_t r2r_rx_duration_field                                   : 16, // [31:16]
                      r2r_hw_response_tx_duration                             : 16; // [15:0]
             uint32_t r2r_bw40_active_channel                                 :  3, // [31:29]
                      r2r_bw20_active_channel                                 :  3, // [28:26]
                      reserved_29a                                            :  3, // [25:23]
                      r2r_set_required_response_time                          :  1, // [22:22]
                      use_address_fields_for_protection                       :  1, // [21:21]
                      r2r_sta_partial_aid                                     : 11, // [20:10]
                      r2r_response_frame_type                                 :  4, // [9:6]
                      r2r_group_id                                            :  6; // [5:0]
             uint32_t reserved_30a                                            :  2, // [31:30]
                      r2r_bw320                                               :  3, // [29:27]
                      r2r_bw240                                               :  3, // [26:24]
                      r2r_bw160                                               :  3, // [23:21]
                      r2r_bw80                                                :  3, // [20:18]
                      r2r_bw40                                                :  3, // [17:15]
                      r2r_bw20                                                :  3, // [14:12]
                      r2r_bw320_active_channel                                :  3, // [11:9]
                      r2r_bw240_active_channel                                :  3, // [8:6]
                      r2r_bw160_active_channel                                :  3, // [5:3]
                      r2r_bw80_active_channel                                 :  3; // [2:0]
             uint32_t mu_response_expected_bitmap_31_0                        : 32; // [31:0]
             uint32_t tx_start_transmit_time_byte_offset                      : 12, // [31:20]
                      insert_current_tx_start_timing_info                     :  1, // [19:19]
                      insert_prev_tx_start_timing_info                        :  1, // [18:18]
                      transmit_includes_multidestination                      :  1, // [17:17]
                      mu_expected_response_sta_count                          :  6, // [16:11]
                      mu_expected_response_cbf_count                          :  6, // [10:5]
                      mu_response_expected_bitmap_36_32                       :  5; // [4:0]
             uint32_t protection_frame_ad1_31_0                               : 32; // [31:0]
             uint32_t protection_frame_ad2_15_0                               : 16, // [31:16]
                      protection_frame_ad1_47_32                              : 16; // [15:0]
             uint32_t protection_frame_ad2_47_16                              : 32; // [31:0]
             uint32_t reserved_54a                                            :  7, // [31:25]
                      dynamic_medium_prot_type                                :  1, // [24:24]
                      dynamic_medium_prot_threshold                           : 24; // [23:0]
             uint32_t protection_frame_ad3_31_0                               : 32; // [31:0]
             uint32_t protection_frame_ad4_15_0                               : 16, // [31:16]
                      protection_frame_ad3_47_32                              : 16; // [15:0]
             uint32_t protection_frame_ad4_47_16                              : 32; // [31:0]
#endif
};


/* Description		MEDIUM_PROT_TYPE

			Self Gen Medium Protection type used
			<enum 0 No_protection>
			<enum 1 RTS_legacy>
			<enum 2 RTS_11ac_static_bw>
			<enum 3 RTS_11ac_dynamic_bw>
			<enum 4 CTS2Self>
			<enum 5 QoS_Null_no_ack_3addr>
			<enum 6 QoS_Null_no_ack_4addr>
			
			<legal 0-6>
*/

#define PCU_PPDU_SETUP_INIT_MEDIUM_PROT_TYPE_OFFSET                                 0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MEDIUM_PROT_TYPE_LSB                                    0
#define PCU_PPDU_SETUP_INIT_MEDIUM_PROT_TYPE_MSB                                    2
#define PCU_PPDU_SETUP_INIT_MEDIUM_PROT_TYPE_MASK                                   0x0000000000000007


/* Description		RESPONSE_TYPE

			PPDU transmission Response type expected
			
			Used by PDG to calculate the anticipated response duration
			 time.
			
			Used by TXPCU to prepare for expecting to receive a response.
			
			
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

#define PCU_PPDU_SETUP_INIT_RESPONSE_TYPE_OFFSET                                    0x0000000000000000
#define PCU_PPDU_SETUP_INIT_RESPONSE_TYPE_LSB                                       3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TYPE_MSB                                       7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TYPE_MASK                                      0x00000000000000f8


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

#define PCU_PPDU_SETUP_INIT_RESPONSE_INFO_PART2_REQUIRED_OFFSET                     0x0000000000000000
#define PCU_PPDU_SETUP_INIT_RESPONSE_INFO_PART2_REQUIRED_LSB                        8
#define PCU_PPDU_SETUP_INIT_RESPONSE_INFO_PART2_REQUIRED_MSB                        8
#define PCU_PPDU_SETUP_INIT_RESPONSE_INFO_PART2_REQUIRED_MASK                       0x0000000000000100


/* Description		RESPONSE_TO_RESPONSE

			Field indicates if after receiving an expected PPDU response
			 (as indicated by the Response_type), TXPCU is expected 
			to generate a reponse to that response
			
			Example: OFDMA trigger frame is sent, with expected response
			 being UL OFDMA data, which result in a response to the 
			response of MBA
			
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

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_OFFSET                             0x0000000000000000
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_LSB                                9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_MSB                                11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_MASK                               0x0000000000000e00


/* Description		MBA_USER_ORDER

			Field only valid in case of 'response_to_response' set to
			 MU_BA.
			
			<enum 0 mu_ba_fixed_user_order> TXPCU shall ask RXPCU for
			 BA info for all TX users, in order from user 0 to user 
			N
			<enum 1 mu_ba_optimized_user_order> TXPCU shall ask RXPCU
			 for BA info for all TX users, but let RXPCU determine in
			 which order the BA bitmaps for each user shall be returned. 
			Note that RXPCU might return some 'invalid' bitmaps in case
			 there was no data received from all the users. 
			<enum 2 mu_ba_fully_optimized> TXPCU shall ask RXPCU for
			 BA info for the number RX users that RXPCU indicated in
			 the 'Max_rx_user_count' in the RX_PPDU_START TLV. TXPCU
			 shall let RXPCU determine in which order the BA bitmaps
			 for each user shall be returned. Note that RXPCU might 
			still return some 'invalid' bitmaps in case there were only
			 frames with FCS errors for some of the users
			<enum 3 mu_ba_fully_optimized_multi_tid> TXPCU shall ask
			 RXPCU for BA info for the number bitmaps that RXPCU indicated
			 in the (SUM of) response_ack_count, response_ba64_count, 
			response_ba256_count fields in RX_RESPONSE_REQUIRED. TXPCU
			 shall let RXPCU determine in which order the BA bitmaps
			 for each user (and sometimes multiple bitmaps for a the
			 same user in case of multi TID) shall be returned. It is
			 not expected that RXPCU will return invalid bitmaps for
			 this scenario as RXPCU earlier indicates that this number
			 of bitmaps was actually available in RXPCU...
			
			<legal 0-3>
*/

#define PCU_PPDU_SETUP_INIT_MBA_USER_ORDER_OFFSET                                   0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MBA_USER_ORDER_LSB                                      12
#define PCU_PPDU_SETUP_INIT_MBA_USER_ORDER_MSB                                      13
#define PCU_PPDU_SETUP_INIT_MBA_USER_ORDER_MASK                                     0x0000000000003000


/* Description		EXPECTED_MBA_SIZE

			Field only valid for:
			Mba_user_order == mu_ba_fixed_user_order, mu_ba_optimized_user_order
			
			
			The expected number of bytes in response (Multi destination) 
			BA that TXPCU shall request to PDG.
			NOTE that SW should have pre-calculated and thus looked-up
			 the window sizes for each of the STAs.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_EXPECTED_MBA_SIZE_OFFSET                                0x0000000000000000
#define PCU_PPDU_SETUP_INIT_EXPECTED_MBA_SIZE_LSB                                   14
#define PCU_PPDU_SETUP_INIT_EXPECTED_MBA_SIZE_MSB                                   24
#define PCU_PPDU_SETUP_INIT_EXPECTED_MBA_SIZE_MASK                                  0x0000000001ffc000


/* Description		REQUIRED_UL_MU_RESP_USER_COUNT

			Field only valid for: Response_to_response
			== MU_BA 
			or 
			RESPONSE_TO_RESPONSE_CMD
			
			Field MU_RX_successful_user_count as reported in the RECEIVED_RESPONSE_INFO
			 TLV shall be >= to this field, in order to consider the
			 reception successful.
			
			Note that the value in this field shall always be equal 
			or smaller to the number of bits set in field MU_Response_expected_bitmap_....
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_REQUIRED_UL_MU_RESP_USER_COUNT_OFFSET                   0x0000000000000000
#define PCU_PPDU_SETUP_INIT_REQUIRED_UL_MU_RESP_USER_COUNT_LSB                      25
#define PCU_PPDU_SETUP_INIT_REQUIRED_UL_MU_RESP_USER_COUNT_MSB                      30
#define PCU_PPDU_SETUP_INIT_REQUIRED_UL_MU_RESP_USER_COUNT_MASK                     0x000000007e000000


/* Description		TRANSMITTED_BSSID_CHECK_EN

			When set to 1, RXPCU shall assume group addressed frame 
			with Tx_AD2 equal to TBSSID was sent. RxPCU should properly
			 handle receive frame(s) from STA(s) which A1 is TBSSID 
			or any VAPs.When NOT set, RXPCU shall compare received frame's
			 A1 with Tx_AD2 only.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_TRANSMITTED_BSSID_CHECK_EN_OFFSET                       0x0000000000000000
#define PCU_PPDU_SETUP_INIT_TRANSMITTED_BSSID_CHECK_EN_LSB                          31
#define PCU_PPDU_SETUP_INIT_TRANSMITTED_BSSID_CHECK_EN_MSB                          31
#define PCU_PPDU_SETUP_INIT_TRANSMITTED_BSSID_CHECK_EN_MASK                         0x0000000080000000


/* Description		MPROT_REQUIRED_BW1

			Field only valid when ppdu_allowed_bw1 is set.
			
			When set, Medium protection transmission is required for
			 a 1 MHz bandwidth PPDU transmission. In case of MU transmissions, 
			all the medium protection settings are coming from user0. <legal
			 all>
*/

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW1_OFFSET                               0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW1_LSB                                  32
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW1_MSB                                  32
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW1_MASK                                 0x0000000100000000


/* Description		MPROT_REQUIRED_BW20

			Field only valid when ppdu_allowed_bw20_bw2  is set.
			
			NOTE: This field is also known as Mprot_required_pattern_0
			 in case punctured transmission is enabled.
			
			When set, Medium protection transmission is required for
			 a 20 MHz or 2Mhz 11ah bandwidth PPDU transmission
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW20_OFFSET                              0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW20_LSB                                 33
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW20_MSB                                 33
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW20_MASK                                0x0000000200000000


/* Description		MPROT_REQUIRED_BW40

			Field only valid when ppdu_allowed_bw40_bw4 is set.
			
			NOTE: This field is also known as Mprot_required_pattern_1
			 in case punctured transmission is enabled.
			
			When set, Medium protection transmission is required for
			 a 40 MHz or 4Mhz 11ah bandwidth PPDU transmission 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW40_OFFSET                              0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW40_LSB                                 34
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW40_MSB                                 34
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW40_MASK                                0x0000000400000000


/* Description		MPROT_REQUIRED_BW80

			Field only valid when ppdu_allowed_bw80_bw8  is set.
			
			
			NOTE: This field is also known as Mprot_required_pattern_2
			 in case punctured transmission is enabled.
			
			When set, Medium protection transmission is required for
			 a 80 MHz or 8MHz 11ah bandwidth PPDU transmission 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW80_OFFSET                              0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW80_LSB                                 35
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW80_MSB                                 35
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW80_MASK                                0x0000000800000000


/* Description		MPROT_REQUIRED_BW160

			Field only valid when ppdu_allowed_bw160_bw16 is set.
			
			NOTE: This field is also known as Mprot_required_pattern_3
			 in case punctured transmission is enabled.
			
			When set, Medium protection transmission is required for
			 a 160 MHz or 16MHz 11ah bandwidth PPDU transmission. 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW160_OFFSET                             0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW160_LSB                                36
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW160_MSB                                36
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW160_MASK                               0x0000001000000000


/* Description		MPROT_REQUIRED_BW240

			Field only valid when ppdu_allowed_bw240 is set.
			
			NOTE: This field is also known as Mprot_required_pattern_4
			 in case punctured transmission is enabled.
			
			When set, Medium protection transmission is required for
			 a 240 MHz bandwidth PPDU transmission. 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW240_OFFSET                             0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW240_LSB                                37
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW240_MSB                                37
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW240_MASK                               0x0000002000000000


/* Description		MPROT_REQUIRED_BW320

			Field only valid when ppdu_allowed_bw320 is set.
			
			NOTE: This field is also known as Mprot_required_pattern_5
			 in case punctured transmission is enabled.
			
			When set, Medium protection transmission is required for
			 a 320 MHz bandwidth PPDU transmission. 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW320_OFFSET                             0x0000000000000000
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW320_LSB                                38
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW320_MSB                                38
#define PCU_PPDU_SETUP_INIT_MPROT_REQUIRED_BW320_MASK                               0x0000004000000000


/* Description		PPDU_ALLOWED_BW1

			When set, allow PPDU transmission with 1 MHz 11ah bandwidth. 
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW1_OFFSET                                 0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW1_LSB                                    39
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW1_MSB                                    39
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW1_MASK                                   0x0000008000000000


/* Description		PPDU_ALLOWED_BW20

			Field Not valid in  case punctured transmission is enabled. 
			This fields meaning is than taken over by field TX_PUNCTURE_SETUP.
			
			puncture_pattern_count
			
			When set, allow PPDU transmission with 20 MHz or 2MHz 11ah
			 bandwidth 
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW20_OFFSET                                0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW20_LSB                                   40
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW20_MSB                                   40
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW20_MASK                                  0x0000010000000000


/* Description		PPDU_ALLOWED_BW40

			Field Not valid in  case punctured transmission is enabled. 
			This fields meaning is than taken over by field TX_PUNCTURE_SETUP.
			
			puncture_pattern_count
			
			When set, allow PPDU transmission with 40 MHz or 4MHz 11ah
			 bandwidth 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW40_OFFSET                                0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW40_LSB                                   41
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW40_MSB                                   41
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW40_MASK                                  0x0000020000000000


/* Description		PPDU_ALLOWED_BW80

			Field Not valid in  case punctured transmission is enabled. 
			This fields meaning is than taken over by field TX_PUNCTURE_SETUP.
			
			puncture_pattern_count
			
			When set, allow PPDU transmission with 80 MHz or 8MHz 11ah
			 bandwidth 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW80_OFFSET                                0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW80_LSB                                   42
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW80_MSB                                   42
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW80_MASK                                  0x0000040000000000


/* Description		PPDU_ALLOWED_BW160

			Field Not valid in  case punctured transmission is enabled. 
			This fields meaning is than taken over by field TX_PUNCTURE_SETUP.
			
			puncture_pattern_count
			
			When set, allow PPDU transmission with 160 MHz or 16MHz 
			11ah bandwidth 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW160_OFFSET                               0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW160_LSB                                  43
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW160_MSB                                  43
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW160_MASK                                 0x0000080000000000


/* Description		PPDU_ALLOWED_BW240

			Field Not valid in  case punctured transmission is enabled. 
			This fields meaning is than taken over by field TX_PUNCTURE_SETUP.
			
			puncture_pattern_count
			
			When set, allow PPDU transmission with 240 MHz bandwidth
			 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW240_OFFSET                               0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW240_LSB                                  44
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW240_MSB                                  44
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW240_MASK                                 0x0000100000000000


/* Description		PPDU_ALLOWED_BW320

			Field Not valid in  case punctured transmission is enabled. 
			This fields meaning is than taken over by field TX_PUNCTURE_SETUP.
			
			puncture_pattern_count
			
			When set, allow PPDU transmission with 320 MHz bandwidth
			 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW320_OFFSET                               0x0000000000000000
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW320_LSB                                  45
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW320_MSB                                  45
#define PCU_PPDU_SETUP_INIT_PPDU_ALLOWED_BW320_MASK                                 0x0000200000000000


/* Description		SET_FC_PWR_MGT

			Field valid for SU transmissions only
			
			When set, the TXPCU will set the power management bit in
			 the Frame Control field for the transmitted frames.
			
			Note: this is there for backup purposes only. TXOLE is the
			 module now that should be setting the pm bit to the proper
			 value.
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_SET_FC_PWR_MGT_OFFSET                                   0x0000000000000000
#define PCU_PPDU_SETUP_INIT_SET_FC_PWR_MGT_LSB                                      46
#define PCU_PPDU_SETUP_INIT_SET_FC_PWR_MGT_MSB                                      46
#define PCU_PPDU_SETUP_INIT_SET_FC_PWR_MGT_MASK                                     0x0000400000000000


/* Description		USE_CTS_DURATION_FOR_DATA_TX

			When set, take the value of the duration field from the 
			CTS frame, and use this as the reference point for how long
			 the 'data' ppdu transmission can be.
			This is an E2E feature.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_USE_CTS_DURATION_FOR_DATA_TX_OFFSET                     0x0000000000000000
#define PCU_PPDU_SETUP_INIT_USE_CTS_DURATION_FOR_DATA_TX_LSB                        47
#define PCU_PPDU_SETUP_INIT_USE_CTS_DURATION_FOR_DATA_TX_MSB                        47
#define PCU_PPDU_SETUP_INIT_USE_CTS_DURATION_FOR_DATA_TX_MASK                       0x0000800000000000


/* Description		UPDATE_TIMESTAMP_64

			When set, TXPCU shall update the timestamp value at the 
			indicated location.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_64_OFFSET                              0x0000000000000000
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_64_LSB                                 48
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_64_MSB                                 48
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_64_MASK                                0x0001000000000000


/* Description		UPDATE_TIMESTAMP_32_LOWER

			Update the 32 bit timestamp at the offset specified by the
			 insert_timestamp_offset_32.  This will be used for AWDL
			 action frames.  The value of the TSF will be added to the
			 timestamp field in the packet buffer in memory.  The tx_delay
			 should also be included in the timestamp field<legal all>
			
*/

#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_LOWER_OFFSET                        0x0000000000000000
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_LOWER_LSB                           49
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_LOWER_MSB                           49
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_LOWER_MASK                          0x0002000000000000


/* Description		UPDATE_TIMESTAMP_32_UPPER

			Update the 64 bit TSF at the offset specified by the insert_timestamp_offset_64. 
			 This will be used for beacons and probe response frames. 
			 The value of the TSF will be added to the TSF field in 
			the packet buffer in memory.  The tx_delay should also be
			 included in the TSF field
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_UPPER_OFFSET                        0x0000000000000000
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_UPPER_LSB                           50
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_UPPER_MSB                           50
#define PCU_PPDU_SETUP_INIT_UPDATE_TIMESTAMP_32_UPPER_MASK                          0x0004000000000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define PCU_PPDU_SETUP_INIT_RESERVED_1A_OFFSET                                      0x0000000000000000
#define PCU_PPDU_SETUP_INIT_RESERVED_1A_LSB                                         51
#define PCU_PPDU_SETUP_INIT_RESERVED_1A_MSB                                         63
#define PCU_PPDU_SETUP_INIT_RESERVED_1A_MASK                                        0xfff8000000000000


/* Description		INSERT_TIMESTAMP_OFFSET_0

			Byte offset  to the first byte of the lower 32 bit timestamp
			 to be inserted.  This is applicable to both beacon and 
			probe response TSF and the AWDL timestamp<legal all>
*/

#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_0_OFFSET                        0x0000000000000008
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_0_LSB                           0
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_0_MSB                           15
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_0_MASK                          0x000000000000ffff


/* Description		INSERT_TIMESTAMP_OFFSET_1

			Byte offset  to the first byte of the upper 32 bit timestamp
			 to be inserted.  This is applicable to both beacon and 
			probe response TSF and the AWDL timestamp<legal all>
*/

#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_1_OFFSET                        0x0000000000000008
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_1_LSB                           16
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_1_MSB                           31
#define PCU_PPDU_SETUP_INIT_INSERT_TIMESTAMP_OFFSET_1_MASK                          0x00000000ffff0000


/* Description		MAX_BW40_TRY_COUNT

			Field only valid when ppdu_allowed_bw40_bw4 or Mprot_required_bw40_bw4
			 is set.
			
			NOTE: This field is also known as Max_try_count_pattern_1
			 in case punctured transmission is enabled.
			
			The maximum number of times that TXPCU will try to do a 
			transmission at this or a higher BW, before deciding to 
			go to a lower BW.
			If this count (as indicated by field Optimal_bw_retry_count
			 in TX_FES_SETUP) has not been reached yet, and this BW 
			is not available, TXPCU will generate a flush with flush
			 reason set to 'TXPCU_FLREQ_RETRY_FOR_OPTIMAL_BW.'
			
			When value is 0, it means that if this BW is not available, 
			TXPCU should immediately try a lower BW.
			
			Note that this value shall always be equal or greater then: 
			Max_bw80_try_count
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MAX_BW40_TRY_COUNT_OFFSET                               0x0000000000000008
#define PCU_PPDU_SETUP_INIT_MAX_BW40_TRY_COUNT_LSB                                  32
#define PCU_PPDU_SETUP_INIT_MAX_BW40_TRY_COUNT_MSB                                  35
#define PCU_PPDU_SETUP_INIT_MAX_BW40_TRY_COUNT_MASK                                 0x0000000f00000000


/* Description		MAX_BW80_TRY_COUNT

			Field only valid when ppdu_allowed_bw80_bw4 or Mprot_required_bw80_bw4
			 is set.
			
			NOTE: This field is also known as Max_try_count_pattern_2
			 in case punctured transmission is enabled.
			
			The maximum number of times that TXPCU will try to do a 
			transmission at this or a higher BW, before deciding to 
			go to a lower BW.
			If this count (as indicated by field Optimal_bw_retry_count
			 in TX_FES_SETUP) has not been reached yet, and this BW 
			is not available, TXPCU will generate a flush with flush
			 reason set to 'TXPCU_FLREQ_RETRY_FOR_OPTIMAL_BW.'
			
			When value is 0, it means that if this BW is not available, 
			TXPCU should immediately try a lower BW.
			
			Note that this value shall always be equal or greater then: 
			Max_bw160_try_count
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MAX_BW80_TRY_COUNT_OFFSET                               0x0000000000000008
#define PCU_PPDU_SETUP_INIT_MAX_BW80_TRY_COUNT_LSB                                  36
#define PCU_PPDU_SETUP_INIT_MAX_BW80_TRY_COUNT_MSB                                  39
#define PCU_PPDU_SETUP_INIT_MAX_BW80_TRY_COUNT_MASK                                 0x000000f000000000


/* Description		MAX_BW160_TRY_COUNT

			Field only valid when ppdu_allowed_bw160_bw16 or Mprot_required_bw160_bw16
			 is set.
			
			NOTE: This field is also known as Max_try_count_pattern_3
			 in case punctured transmission is enabled.
			
			The maximum number of times that TXPCU will try to do a 
			transmission at this, before deciding to go to a lower BW.
			
			If this count (as indicated by field Optimal_bw_retry_count
			 in TX_FES_SETUP) has not been reached yet, and this BW 
			is not available, TXPCU will generate a flush with flush
			 reason set to 'TXPCU_FLREQ_RETRY_FOR_OPTIMAL_BW.'
			
			When value is 0, it means that if this BW is not available, 
			TXPCU should immediately try a lower BW.
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MAX_BW160_TRY_COUNT_OFFSET                              0x0000000000000008
#define PCU_PPDU_SETUP_INIT_MAX_BW160_TRY_COUNT_LSB                                 40
#define PCU_PPDU_SETUP_INIT_MAX_BW160_TRY_COUNT_MSB                                 43
#define PCU_PPDU_SETUP_INIT_MAX_BW160_TRY_COUNT_MASK                                0x00000f0000000000


/* Description		MAX_BW240_TRY_COUNT

			Field only valid when ppdu_allowed_bw240 or Mprot_required_bw240
			 is set.
			
			NOTE: This field is also known as Max_try_count_pattern_4
			 in case punctured transmission is enabled.
			
			The maximum number of times that TXPCU will try to do a 
			transmission at this, before deciding to go to a lower BW.
			
			If this count (as indicated by field Optimal_bw_retry_count
			 in TX_FES_SETUP) has not been reached yet, and this BW 
			is not available, TXPCU will generate a flush with flush
			 reason set to 'TXPCU_FLREQ_RETRY_FOR_OPTIMAL_BW.'
			
			When value is 0, it means that if this BW is not available, 
			TXPCU should immediately try a lower BW.
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MAX_BW240_TRY_COUNT_OFFSET                              0x0000000000000008
#define PCU_PPDU_SETUP_INIT_MAX_BW240_TRY_COUNT_LSB                                 44
#define PCU_PPDU_SETUP_INIT_MAX_BW240_TRY_COUNT_MSB                                 47
#define PCU_PPDU_SETUP_INIT_MAX_BW240_TRY_COUNT_MASK                                0x0000f00000000000


/* Description		MAX_BW320_TRY_COUNT

			Field only valid when ppdu_allowed_bw320 or Mprot_required_bw320
			 is set.
			
			NOTE: This field is also known as Max_try_count_pattern_5
			 in case punctured transmission is enabled.
			
			The maximum number of times that TXPCU will try to do a 
			transmission at this, before deciding to go to a lower BW.
			
			If this count (as indicated by field Optimal_bw_retry_count
			 in TX_FES_SETUP) has not been reached yet, and this BW 
			is not available, TXPCU will generate a flush with flush
			 reason set to 'TXPCU_FLREQ_RETRY_FOR_OPTIMAL_BW.'
			
			When value is 0, it means that if this BW is not available, 
			TXPCU should immediately try a lower BW.
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MAX_BW320_TRY_COUNT_OFFSET                              0x0000000000000008
#define PCU_PPDU_SETUP_INIT_MAX_BW320_TRY_COUNT_LSB                                 48
#define PCU_PPDU_SETUP_INIT_MAX_BW320_TRY_COUNT_MSB                                 51
#define PCU_PPDU_SETUP_INIT_MAX_BW320_TRY_COUNT_MASK                                0x000f000000000000


/* Description		INSERT_WUR_TIMESTAMP_OFFSET

			Field only to be used in case PCU_PPDU_SETUP_START.pkt_type
			 indicates a .11ba packet
			
			Used by TXPCU to determine the offset within a WUR packet, 
			e.g. a WUR beacon into which to insert the timestamp.
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_INSERT_WUR_TIMESTAMP_OFFSET_OFFSET                      0x0000000000000008
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_TIMESTAMP_OFFSET_LSB                         52
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_TIMESTAMP_OFFSET_MSB                         57
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_TIMESTAMP_OFFSET_MASK                        0x03f0000000000000


/* Description		UPDATE_WUR_TIMESTAMP

			Field only to be used in case PCU_PPDU_SETUP_START.pkt_type
			 indicates a .11ba packet
			
			TXPCU will insert the timestamp into a WUR packet if this
			 bit is set.
			
			<legal all> 
*/

#define PCU_PPDU_SETUP_INIT_UPDATE_WUR_TIMESTAMP_OFFSET                             0x0000000000000008
#define PCU_PPDU_SETUP_INIT_UPDATE_WUR_TIMESTAMP_LSB                                58
#define PCU_PPDU_SETUP_INIT_UPDATE_WUR_TIMESTAMP_MSB                                58
#define PCU_PPDU_SETUP_INIT_UPDATE_WUR_TIMESTAMP_MASK                               0x0400000000000000


/* Description		WUR_EMBEDDED_BSSID_PRESENT

			Field only to be used in case PCU_PPDU_SETUP_START.pkt_type
			 indicates a .11ba packet
			
			If this bit is set, TXPCU will assume the packet includes
			 an extra 16 bits which contain the embedded BSSID to be
			 used in the WUR FCS calculation. TXPCU will replace the
			 16 bits with the 16-bit FCS field.
			If this bit is clear, TXPCU will append the 16-bit FCS calculated
			 without any embedded BSSID.
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_WUR_EMBEDDED_BSSID_PRESENT_OFFSET                       0x0000000000000008
#define PCU_PPDU_SETUP_INIT_WUR_EMBEDDED_BSSID_PRESENT_LSB                          59
#define PCU_PPDU_SETUP_INIT_WUR_EMBEDDED_BSSID_PRESENT_MSB                          59
#define PCU_PPDU_SETUP_INIT_WUR_EMBEDDED_BSSID_PRESENT_MASK                         0x0800000000000000


/* Description		INSERT_WUR_FCS

			Field only to be used in case PCU_PPDU_SETUP_START.pkt_type
			 indicates a .11ba packet
			
			TXPCU will replace/append the FCS bytes for a WUR packet
			 if this bit is set. The replace/append choice is based 
			on WUR_embedded_BSSID_present.
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_INSERT_WUR_FCS_OFFSET                                   0x0000000000000008
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_FCS_LSB                                      60
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_FCS_MSB                                      60
#define PCU_PPDU_SETUP_INIT_INSERT_WUR_FCS_MASK                                     0x1000000000000000


/* Description		RESERVED_3B

			<legal 0>
*/

#define PCU_PPDU_SETUP_INIT_RESERVED_3B_OFFSET                                      0x0000000000000008
#define PCU_PPDU_SETUP_INIT_RESERVED_3B_LSB                                         61
#define PCU_PPDU_SETUP_INIT_RESERVED_3B_MSB                                         63
#define PCU_PPDU_SETUP_INIT_RESERVED_3B_MASK                                        0xe000000000000000


/* Description		RESPONSE_TO_RESPONSE_RATE_INFO_BW20

			Field only valid in case of Response_to_response set to 
			SU_BA or MU_BA
			
			NOTE: This field is also known as response_to_response_rate_info_pattern_0
			 in case punctured transmission is enabled.
			
			Used by TXPCU to determine what the transmit rates are for
			 the response to response transmission in case original 
			transmission was 20 MHz.
			
			Note: 
			see field R2R_bw20_active_channel for the BW of this transmission
			
*/


/* Description		RESERVED_0A

			 
			<legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_0A_OFFSET  0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_0A_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_0A_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_0A_MASK    0x0000000000000001


/* Description		TX_ANTENNA_SECTOR_CTRL

			Sectored transmit antenna
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_ANTENNA_SECTOR_CTRL_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_ANTENNA_SECTOR_CTRL_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_ANTENNA_SECTOR_CTRL_MASK 0x0000000001fffffe


/* Description		PKT_TYPE

			Packet type:
			<enum 0 dot11a>802.11a PPDU type
			<enum 1 dot11b>802.11b PPDU type
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			<enum 3 dot11ac>802.11ac PPDU type
			<enum 4 dot11ax>802.11ax PPDU type
			<enum 5 dot11ba>802.11ba (WUR) PPDU type
			<enum 6 dot11be>802.11be PPDU type
			<enum 7 dot11az>802.11az (ranging) PPDU type
			<enum 8 dot11n_gf>802.11n Green Field PPDU type (unsupported
			 & aborted)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_PKT_TYPE_OFFSET     0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_PKT_TYPE_LSB        25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_PKT_TYPE_MSB        28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_PKT_TYPE_MASK       0x000000001e000000


/* Description		SMOOTHING

			This field is used by PDG to populate the SMOOTHING filed
			 in the SIG Preamble of the PPDU
			<legal 0-1>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SMOOTHING_OFFSET    0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SMOOTHING_LSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SMOOTHING_MSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SMOOTHING_MASK      0x0000000020000000


/* Description		LDPC

			When set, use LDPC transmission rates
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_LDPC_OFFSET         0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_LDPC_LSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_LDPC_MSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_LDPC_MASK           0x0000000040000000


/* Description		STBC

			When set, use STBC transmission rates
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STBC_OFFSET         0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STBC_LSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STBC_MSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STBC_MASK           0x0000000080000000


/* Description		ALT_TX_PWR

			Coex related AlternativeTransmit parameter 
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_OFFSET   0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_LSB      32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_MSB      39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_MASK     0x000000ff00000000


/* Description		ALT_MIN_TX_PWR

			Coex related Alternative Transmit parameter 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_MIN_TX_PWR_OFFSET 0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_MIN_TX_PWR_LSB  40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_MIN_TX_PWR_MSB  47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_MIN_TX_PWR_MASK 0x0000ff0000000000


/* Description		ALT_NSS

			Coex related Alternative Transmit parameter
			
			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_NSS_OFFSET      0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_NSS_LSB         48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_NSS_MSB         50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_NSS_MASK        0x0007000000000000


/* Description		ALT_TX_CHAIN_MASK

			Coex related Alternative Transmit parameter
			
			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_CHAIN_MASK_OFFSET 0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_CHAIN_MASK_LSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_CHAIN_MASK_MSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_CHAIN_MASK_MASK 0x07f8000000000000


/* Description		ALT_BW

			Coex related Alternative Transmit parameter
			
			The BW of the upcoming transmission.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_BW_OFFSET       0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_BW_LSB          59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_BW_MSB          61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_BW_MASK         0x3800000000000000


/* Description		STF_LTF_3DB_BOOST

			Boost the STF and LTF power by 3dB in 11a/n/ac packets. 
			This includes both the legacy preambles and the HT/VHT preambles.0: 
			disable power boost1: enable power boost
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STF_LTF_3DB_BOOST_OFFSET 0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STF_LTF_3DB_BOOST_LSB 62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STF_LTF_3DB_BOOST_MSB 62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_STF_LTF_3DB_BOOST_MASK 0x4000000000000000


/* Description		FORCE_EXTRA_SYMBOL

			Set to 1 to force an extra OFDM symbol (or symbols) even
			 if the PPDU encoding process does not result in an extra
			 OFDM symbol (or symbols)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_FORCE_EXTRA_SYMBOL_OFFSET 0x0000000000000010
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_FORCE_EXTRA_SYMBOL_LSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_FORCE_EXTRA_SYMBOL_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_FORCE_EXTRA_SYMBOL_MASK 0x8000000000000000


/* Description		ALT_RATE_MCS

			Coex related Alternative Transmit parameter
			
			For details, refer to  MCS_TYPE 
			Note: This is "rate" in case of 11a/11b
			description
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_RATE_MCS_OFFSET 0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_RATE_MCS_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_RATE_MCS_MSB    3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_RATE_MCS_MASK   0x000000000000000f


/* Description		NSS

			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NSS_OFFSET          0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NSS_LSB             4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NSS_MSB             6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NSS_MASK            0x0000000000000070


/* Description		DPD_ENABLE

			DPD enable control
			
			This is needed on a per packet basis
			<enum 0     dpd_off> DPD profile not applied to current 
			packet
			<enum 1     dpd_on> DPD profile applied to current packet
			 if available
			<legal 0-1>
			
			This field is not applicable in11ah mode of operation and
			 is ignored by the HW 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DPD_ENABLE_OFFSET   0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DPD_ENABLE_LSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DPD_ENABLE_MSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DPD_ENABLE_MASK     0x0000000000000080


/* Description		TX_PWR

			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_OFFSET       0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_LSB          8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_MSB          15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_MASK         0x000000000000ff00


/* Description		MIN_TX_PWR

			Coex related field: 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MIN_TX_PWR_OFFSET   0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MIN_TX_PWR_LSB      16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MIN_TX_PWR_MSB      23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MIN_TX_PWR_MASK     0x0000000000ff0000


/* Description		TX_CHAIN_MASK

			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_CHAIN_MASK_OFFSET 0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_CHAIN_MASK_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_CHAIN_MASK_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_CHAIN_MASK_MASK  0x00000000ff000000


/* Description		RESERVED_3A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3A_OFFSET  0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3A_LSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3A_MSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3A_MASK    0x000000ff00000000


/* Description		SGI

			Field only valid when pkt type is HT or VHT.For 11ax see
			 field Dot11ax_CP_LTF_size
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > Not used for pre 11ax pkt_types. 
			
			<enum 3     3_2_us_sgi > Not used for pre 11ax pkt_types
			
			
			<legal 0 - 3>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SGI_OFFSET          0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SGI_LSB             40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SGI_MSB             41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_SGI_MASK            0x0000030000000000


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RATE_MCS_OFFSET     0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RATE_MCS_LSB        42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RATE_MCS_MSB        45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RATE_MCS_MASK       0x00003c0000000000


/* Description		RESERVED_3B

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3B_OFFSET  0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3B_LSB     46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3B_MSB     47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_3B_MASK    0x0000c00000000000


/* Description		TX_PWR_1

			Default (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_1_OFFSET     0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_1_LSB        48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_1_MSB        55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_TX_PWR_1_MASK       0x00ff000000000000


/* Description		ALT_TX_PWR_1

			Alternate (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_1_OFFSET 0x0000000000000018
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_1_LSB    56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_1_MSB    63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_ALT_TX_PWR_1_MASK   0xff00000000000000


/* Description		AGGREGATION

			Field only valid in case of pkt_type == 11n
			
			<enum 0     mpdu> Indicates MPDU format. TXPCU will select
			 this setting if the CBF response only contains a single
			 segment
			<enum 1     a_mpdu> Indicates A-MPDU format. TXPCU will 
			select this setting if the CBF response will contain two
			 or more segments
			<legal 0-1>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_AGGREGATION_OFFSET  0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_AGGREGATION_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_AGGREGATION_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_AGGREGATION_MASK    0x0000000000000001


/* Description		DOT11AX_BSS_COLOR_ID

			BSS color of the nextwork to which this STA belongs. 
			When generated by TXPCU, this field is set equal to: Dot11ax_received_Bss_color_id
			
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_BSS_COLOR_ID_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_BSS_COLOR_ID_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_BSS_COLOR_ID_MSB 6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_BSS_COLOR_ID_MASK 0x000000000000007e


/* Description		DOT11AX_SPATIAL_REUSE

			This field is only valid for pkt_type == 11ax
			
			Spatial re-use
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SPATIAL_REUSE_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SPATIAL_REUSE_LSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SPATIAL_REUSE_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SPATIAL_REUSE_MASK 0x0000000000000780


/* Description		DOT11AX_CP_LTF_SIZE

			field is only valid for pkt_type == 11ax
			
			Indicates the CP and HE-LTF type 
			
			<enum 0 OneX_LTF_0_8CP> 1xLTF + 0.8 us CP
			<enum 1 TwoX_LTF_0_8CP> 2x LTF + 0.8 µs CP
			<enum 2 TwoX_LTF_1_6CP> 2x LTF + 1.6 µs CP
			<enum 3 FourX_LTF_0_8CP_3_2CP>
			When DCM == 0 OR STBC == 0: 4x LTF + 3.2 µs CP 
			When DCM == 1 AND STBC == 1: 4x LTF + 0.8 µs CP. Note: 
			In this scenario, Neither DCM nor STBC is applied to HE 
			data field.
			
			If ( DCM == 1 ) and ( MCS > 0 ) and (STBC == 0) 
			0      = 1xLTF + 0.4 usec
			1      = 2xLTF + 0.4 usec
			2~3 = Reserved
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CP_LTF_SIZE_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CP_LTF_SIZE_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CP_LTF_SIZE_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CP_LTF_SIZE_MASK 0x0000000000001800


/* Description		DOT11AX_DCM

			field is only valid for pkt_type == 11ax
			
			Indicates whether dual sub-carrier modulation is applied
			
			0: No DCM
			1:DCM
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DCM_OFFSET  0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DCM_LSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DCM_MSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DCM_MASK    0x0000000000002000


/* Description		DOT11AX_DOPPLER_INDICATION

			field is only valid for pkt_type == 11ax
			
			0: No Doppler support
			1: Doppler support
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DOPPLER_INDICATION_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DOPPLER_INDICATION_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DOPPLER_INDICATION_MSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DOPPLER_INDICATION_MASK 0x0000000000004000


/* Description		DOT11AX_SU_EXTENDED

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			When set, the 11ax or 11be frame is of the extended range
			 format
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SU_EXTENDED_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SU_EXTENDED_LSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SU_EXTENDED_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_SU_EXTENDED_MASK 0x0000000000008000


/* Description		DOT11AX_MIN_PACKET_EXTENSION

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			The min packet extension duration for this user.
			0: no extension
			1: 8us
			2: 16 us
			3: 20 us (only for .11be)
			<legal 0-3>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_MIN_PACKET_EXTENSION_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_MIN_PACKET_EXTENSION_MSB 17
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x0000000000030000


/* Description		DOT11AX_PE_NSS

			Number of active spatial streams during packet extension.
			
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_NSS_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_NSS_LSB  18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_NSS_MSB  20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_NSS_MASK 0x00000000001c0000


/* Description		DOT11AX_PE_CONTENT

			Content of packet extension. Valid for all 11ax packets 
			having packet extension
			
			0-he_ltf, 1-last_data_symbol
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CONTENT_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CONTENT_LSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CONTENT_MSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CONTENT_MASK 0x0000000000200000


/* Description		DOT11AX_PE_LTF_SIZE

			LTF size to be used during packet extention. . This field
			 is valid for both FTM and non-FTM packets.
			0-1x
			1-2x (unsupported un HWK-1)
			2-4x (unsupported un HWK-1) 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_LTF_SIZE_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_LTF_SIZE_LSB 22
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_LTF_SIZE_MSB 23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_LTF_SIZE_MASK 0x0000000000c00000


/* Description		DOT11AX_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the preamble 
			and data portion of the packet. This field is valid for 
			all transmit packets
			0: disable per-chain csd
			1: enable per-chain csd 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CHAIN_CSD_EN_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CHAIN_CSD_EN_LSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CHAIN_CSD_EN_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_CHAIN_CSD_EN_MASK 0x0000000001000000


/* Description		DOT11AX_PE_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the packet extension
			 portion of the packet. This field is valid for all 11ax
			 packets.
			0: disable per-chain csd
			1: enable per-chain csd
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CHAIN_CSD_EN_LSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CHAIN_CSD_EN_MSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x0000000002000000


/* Description		DOT11AX_DL_UL_FLAG

			field is only valid for pkt_type == 11ax 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DL_UL_FLAG_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DL_UL_FLAG_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DL_UL_FLAG_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_DL_UL_FLAG_MASK 0x0000000004000000


/* Description		RESERVED_4A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_4A_OFFSET  0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_4A_LSB     27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_4A_MSB     31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_4A_MASK    0x00000000f8000000


/* Description		DOT11AX_EXT_RU_START_INDEX

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1
			
			RU Number to which User is assigned
			
			The RU numbering bitwidth  is only enough to cover the 20MHz
			 BW that extended range allows
			<legal 0-8>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_START_INDEX_LSB 32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_START_INDEX_MSB 35
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_START_INDEX_MASK 0x0000000f00000000


/* Description		DOT11AX_EXT_RU_SIZE

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1 or pkt_type == 11be and EHT_duplicate_mode == 1
			
			The size of the RU for this user.
			
			In case of EHT duplicate transmissions, this field indicates
			 the width of the actual content before duplication, e.g. 
			a 40 MHz PPDU duplicated to 160 MHz will have the bandwidth
			 fields indicating 160 MHz and this field set to e-num 4
			 (RU_484).
			
			<enum 0 RU_26> 
			<enum 1 RU_52>
			<enum 2 RU_106>
			<enum 3 RU_242>
			<enum 4 RU_484>
			<enum 5 RU_996>
			<enum 6 RU_1992>
			<enum 7 RU_FULLBW> Set when the RU occupies the full packet
			 bandwidth
			<enum 8 RU_FULLBW_240> Set when the RU occupies the full
			 packet bandwidth
			<enum 9 RU_FULLBW_320> Set when the RU occupies the full
			 packet bandwidth
			<enum 10 RU_MULTI_LARGE> DO NOT USE
			<enum 11 RU_78> DO NOT USE
			<enum 12 RU_132> DO NOT USE
			<legal 0-12>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_SIZE_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_SIZE_LSB 36
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_SIZE_MSB 39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11AX_EXT_RU_SIZE_MASK 0x000000f000000000


/* Description		EHT_DUPLICATE_MODE

			Field only valid for pkt_type == 11be
			
			Indicates EHT duplicate modulation
			
			<enum 0 eht_no_duplicate>
			<enum 1 eht_2x_duplicate>
			<enum 2 eht_4x_duplicate>
			
			<legal 0-2>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_EHT_DUPLICATE_MODE_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_EHT_DUPLICATE_MODE_LSB 40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_EHT_DUPLICATE_MODE_MSB 41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_EHT_DUPLICATE_MODE_MASK 0x0000030000000000


/* Description		HE_SIGB_DCM

			Indicates whether dual sub-carrier modulation is applied
			 to EHT-SIG
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_DCM_OFFSET  0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_DCM_LSB     42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_DCM_MSB     42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_DCM_MASK    0x0000040000000000


/* Description		HE_SIGB_0_MCS

			Indicates the MCS of EHT-SIG
			
			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_0_MCS_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_0_MCS_LSB   43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_0_MCS_MSB   45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_HE_SIGB_0_MCS_MASK  0x0000380000000000


/* Description		NUM_HE_SIGB_SYM

			Indicates the number of EHT-SIG symbols
			
			This field is 0-based with 0 indicating that 1 eht_sig symbol
			 needs to be transmitted.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NUM_HE_SIGB_SYM_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NUM_HE_SIGB_SYM_LSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NUM_HE_SIGB_SYM_MSB 50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_NUM_HE_SIGB_SYM_MASK 0x0007c00000000000


/* Description		REQUIRED_RESPONSE_TIME_SOURCE

			<enum 0 reqd_resp_time_src_is_RXPCU> Typically from received
			 HT Control for sync MLO response
			<enum 1 reqd_resp_time_src_is_FW>
			Typically from 'PCU_PPDU_SETUP_INIT' for sync MLO response
			 to response
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_SOURCE_LSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_SOURCE_MSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x0008000000000000


/* Description		RESERVED_5A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_5A_OFFSET  0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_5A_LSB     52
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_5A_MSB     57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_RESERVED_5A_MASK    0x03f0000000000000


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			6-bit value copied from 'RX_RESPONSE_REQUIRED_INFO' and 'TX_CBF_INFO' 
			to pass on to PDG
			<legal 0-29>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x0000000000000020
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0xfc00000000000000


/* Description		MLO_STA_ID_DETAILS_RX

			16-bi value copied from 'RX_RESPONSE_REQUIRED_INFO' to pass
			 on to PDG
			
			Bits 10 and 11 are not valid, bits [9:0] reflect 'NSTR_MLO_STA_ID' 
			from address search.
			
			See definition of mlo_sta_id_details.
*/


/* Description		NSTR_MLO_STA_ID

			ID of peer participating in non-STR MLO
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x00000000000003ff


/* Description		BLOCK_SELF_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for self-link.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000000000000400


/* Description		BLOCK_PARTNER_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for partner links.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000000000000800


/* Description		NSTR_MLO_STA_ID_VALID

			All the fields in this TLV are valid only if this bit is
			 set.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000000000001000


/* Description		RESERVED_0A

			<legal 0>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x000000000000e000


/* Description		REQUIRED_RESPONSE_TIME

			When non-zero, indicates that PDG shall pad the response
			 transmission to the indicated duration (in us)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_MSB 27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_REQUIRED_RESPONSE_TIME_MASK 0x000000000fff0000


/* Description		DOT11BE_PARAMS_PLACEHOLDER

			4 bytes for use as placeholders for 'Dot11be_*' parameters
			
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11BE_PARAMS_PLACEHOLDER_LSB 28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11BE_PARAMS_PLACEHOLDER_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW20_DOT11BE_PARAMS_PLACEHOLDER_MASK 0x00000000f0000000


/* Description		RESPONSE_TO_RESPONSE_RATE_INFO_BW40

			Field only valid in case of Response_to_response set to 
			SU_BA or MU_BA
			
			NOTE: This field is also known as response_to_response_rate_info_pattern_1
			 in case punctured transmission is enabled.
			
			Used by TXPCU to determine what the transmit rates are for
			 the response to response transmission in case original 
			transmission was 40 MHz.
			
			Note: 
			see field R2R_bw40_active_channel for the BW of this transmission
			
*/


/* Description		RESERVED_0A

			 
			<legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_0A_OFFSET  0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_0A_LSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_0A_MSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_0A_MASK    0x0000000100000000


/* Description		TX_ANTENNA_SECTOR_CTRL

			Sectored transmit antenna
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_ANTENNA_SECTOR_CTRL_LSB 33
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_ANTENNA_SECTOR_CTRL_MSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_ANTENNA_SECTOR_CTRL_MASK 0x01fffffe00000000


/* Description		PKT_TYPE

			Packet type:
			<enum 0 dot11a>802.11a PPDU type
			<enum 1 dot11b>802.11b PPDU type
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			<enum 3 dot11ac>802.11ac PPDU type
			<enum 4 dot11ax>802.11ax PPDU type
			<enum 5 dot11ba>802.11ba (WUR) PPDU type
			<enum 6 dot11be>802.11be PPDU type
			<enum 7 dot11az>802.11az (ranging) PPDU type
			<enum 8 dot11n_gf>802.11n Green Field PPDU type (unsupported
			 & aborted)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_PKT_TYPE_OFFSET     0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_PKT_TYPE_LSB        57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_PKT_TYPE_MSB        60
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_PKT_TYPE_MASK       0x1e00000000000000


/* Description		SMOOTHING

			This field is used by PDG to populate the SMOOTHING filed
			 in the SIG Preamble of the PPDU
			<legal 0-1>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SMOOTHING_OFFSET    0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SMOOTHING_LSB       61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SMOOTHING_MSB       61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SMOOTHING_MASK      0x2000000000000000


/* Description		LDPC

			When set, use LDPC transmission rates
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_LDPC_OFFSET         0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_LDPC_LSB            62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_LDPC_MSB            62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_LDPC_MASK           0x4000000000000000


/* Description		STBC

			When set, use STBC transmission rates
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STBC_OFFSET         0x0000000000000028
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STBC_LSB            63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STBC_MSB            63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STBC_MASK           0x8000000000000000


/* Description		ALT_TX_PWR

			Coex related AlternativeTransmit parameter 
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_OFFSET   0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_LSB      0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_MSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_MASK     0x00000000000000ff


/* Description		ALT_MIN_TX_PWR

			Coex related Alternative Transmit parameter 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_MIN_TX_PWR_OFFSET 0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_MIN_TX_PWR_LSB  8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_MIN_TX_PWR_MSB  15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_MIN_TX_PWR_MASK 0x000000000000ff00


/* Description		ALT_NSS

			Coex related Alternative Transmit parameter
			
			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_NSS_OFFSET      0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_NSS_LSB         16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_NSS_MSB         18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_NSS_MASK        0x0000000000070000


/* Description		ALT_TX_CHAIN_MASK

			Coex related Alternative Transmit parameter
			
			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_CHAIN_MASK_OFFSET 0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_CHAIN_MASK_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_CHAIN_MASK_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_CHAIN_MASK_MASK 0x0000000007f80000


/* Description		ALT_BW

			Coex related Alternative Transmit parameter
			
			The BW of the upcoming transmission.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_BW_OFFSET       0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_BW_LSB          27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_BW_MSB          29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_BW_MASK         0x0000000038000000


/* Description		STF_LTF_3DB_BOOST

			Boost the STF and LTF power by 3dB in 11a/n/ac packets. 
			This includes both the legacy preambles and the HT/VHT preambles.0: 
			disable power boost1: enable power boost
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STF_LTF_3DB_BOOST_OFFSET 0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STF_LTF_3DB_BOOST_LSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STF_LTF_3DB_BOOST_MSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_STF_LTF_3DB_BOOST_MASK 0x0000000040000000


/* Description		FORCE_EXTRA_SYMBOL

			Set to 1 to force an extra OFDM symbol (or symbols) even
			 if the PPDU encoding process does not result in an extra
			 OFDM symbol (or symbols)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_FORCE_EXTRA_SYMBOL_OFFSET 0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_FORCE_EXTRA_SYMBOL_LSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_FORCE_EXTRA_SYMBOL_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_FORCE_EXTRA_SYMBOL_MASK 0x0000000080000000


/* Description		ALT_RATE_MCS

			Coex related Alternative Transmit parameter
			
			For details, refer to  MCS_TYPE 
			Note: This is "rate" in case of 11a/11b
			description
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_RATE_MCS_OFFSET 0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_RATE_MCS_LSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_RATE_MCS_MSB    35
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_RATE_MCS_MASK   0x0000000f00000000


/* Description		NSS

			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NSS_OFFSET          0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NSS_LSB             36
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NSS_MSB             38
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NSS_MASK            0x0000007000000000


/* Description		DPD_ENABLE

			DPD enable control
			
			This is needed on a per packet basis
			<enum 0     dpd_off> DPD profile not applied to current 
			packet
			<enum 1     dpd_on> DPD profile applied to current packet
			 if available
			<legal 0-1>
			
			This field is not applicable in11ah mode of operation and
			 is ignored by the HW 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DPD_ENABLE_OFFSET   0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DPD_ENABLE_LSB      39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DPD_ENABLE_MSB      39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DPD_ENABLE_MASK     0x0000008000000000


/* Description		TX_PWR

			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_OFFSET       0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_LSB          40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_MSB          47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_MASK         0x0000ff0000000000


/* Description		MIN_TX_PWR

			Coex related field: 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MIN_TX_PWR_OFFSET   0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MIN_TX_PWR_LSB      48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MIN_TX_PWR_MSB      55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MIN_TX_PWR_MASK     0x00ff000000000000


/* Description		TX_CHAIN_MASK

			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_CHAIN_MASK_OFFSET 0x0000000000000030
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_CHAIN_MASK_LSB   56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_CHAIN_MASK_MSB   63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_CHAIN_MASK_MASK  0xff00000000000000


/* Description		RESERVED_3A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3A_OFFSET  0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3A_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3A_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3A_MASK    0x00000000000000ff


/* Description		SGI

			Field only valid when pkt type is HT or VHT.For 11ax see
			 field Dot11ax_CP_LTF_size
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > Not used for pre 11ax pkt_types. 
			
			<enum 3     3_2_us_sgi > Not used for pre 11ax pkt_types
			
			
			<legal 0 - 3>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SGI_OFFSET          0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SGI_LSB             8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SGI_MSB             9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_SGI_MASK            0x0000000000000300


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RATE_MCS_OFFSET     0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RATE_MCS_LSB        10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RATE_MCS_MSB        13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RATE_MCS_MASK       0x0000000000003c00


/* Description		RESERVED_3B

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3B_OFFSET  0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3B_LSB     14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3B_MSB     15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_3B_MASK    0x000000000000c000


/* Description		TX_PWR_1

			Default (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_1_OFFSET     0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_1_LSB        16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_1_MSB        23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_TX_PWR_1_MASK       0x0000000000ff0000


/* Description		ALT_TX_PWR_1

			Alternate (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_1_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_1_LSB    24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_1_MSB    31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_ALT_TX_PWR_1_MASK   0x00000000ff000000


/* Description		AGGREGATION

			Field only valid in case of pkt_type == 11n
			
			<enum 0     mpdu> Indicates MPDU format. TXPCU will select
			 this setting if the CBF response only contains a single
			 segment
			<enum 1     a_mpdu> Indicates A-MPDU format. TXPCU will 
			select this setting if the CBF response will contain two
			 or more segments
			<legal 0-1>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_AGGREGATION_OFFSET  0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_AGGREGATION_LSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_AGGREGATION_MSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_AGGREGATION_MASK    0x0000000100000000


/* Description		DOT11AX_BSS_COLOR_ID

			BSS color of the nextwork to which this STA belongs. 
			When generated by TXPCU, this field is set equal to: Dot11ax_received_Bss_color_id
			
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_BSS_COLOR_ID_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_BSS_COLOR_ID_LSB 33
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_BSS_COLOR_ID_MSB 38
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_BSS_COLOR_ID_MASK 0x0000007e00000000


/* Description		DOT11AX_SPATIAL_REUSE

			This field is only valid for pkt_type == 11ax
			
			Spatial re-use
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SPATIAL_REUSE_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SPATIAL_REUSE_LSB 39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SPATIAL_REUSE_MSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SPATIAL_REUSE_MASK 0x0000078000000000


/* Description		DOT11AX_CP_LTF_SIZE

			field is only valid for pkt_type == 11ax
			
			Indicates the CP and HE-LTF type 
			
			<enum 0 OneX_LTF_0_8CP> 1xLTF + 0.8 us CP
			<enum 1 TwoX_LTF_0_8CP> 2x LTF + 0.8 µs CP
			<enum 2 TwoX_LTF_1_6CP> 2x LTF + 1.6 µs CP
			<enum 3 FourX_LTF_0_8CP_3_2CP>
			When DCM == 0 OR STBC == 0: 4x LTF + 3.2 µs CP 
			When DCM == 1 AND STBC == 1: 4x LTF + 0.8 µs CP. Note: 
			In this scenario, Neither DCM nor STBC is applied to HE 
			data field.
			
			If ( DCM == 1 ) and ( MCS > 0 ) and (STBC == 0) 
			0      = 1xLTF + 0.4 usec
			1      = 2xLTF + 0.4 usec
			2~3 = Reserved
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CP_LTF_SIZE_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CP_LTF_SIZE_LSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CP_LTF_SIZE_MSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CP_LTF_SIZE_MASK 0x0000180000000000


/* Description		DOT11AX_DCM

			field is only valid for pkt_type == 11ax
			
			Indicates whether dual sub-carrier modulation is applied
			
			0: No DCM
			1:DCM
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DCM_OFFSET  0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DCM_LSB     45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DCM_MSB     45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DCM_MASK    0x0000200000000000


/* Description		DOT11AX_DOPPLER_INDICATION

			field is only valid for pkt_type == 11ax
			
			0: No Doppler support
			1: Doppler support
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DOPPLER_INDICATION_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DOPPLER_INDICATION_LSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DOPPLER_INDICATION_MSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DOPPLER_INDICATION_MASK 0x0000400000000000


/* Description		DOT11AX_SU_EXTENDED

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			When set, the 11ax or 11be frame is of the extended range
			 format
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SU_EXTENDED_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SU_EXTENDED_LSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SU_EXTENDED_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_SU_EXTENDED_MASK 0x0000800000000000


/* Description		DOT11AX_MIN_PACKET_EXTENSION

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			The min packet extension duration for this user.
			0: no extension
			1: 8us
			2: 16 us
			3: 20 us (only for .11be)
			<legal 0-3>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_MIN_PACKET_EXTENSION_LSB 48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_MIN_PACKET_EXTENSION_MSB 49
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x0003000000000000


/* Description		DOT11AX_PE_NSS

			Number of active spatial streams during packet extension.
			
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_NSS_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_NSS_LSB  50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_NSS_MSB  52
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_NSS_MASK 0x001c000000000000


/* Description		DOT11AX_PE_CONTENT

			Content of packet extension. Valid for all 11ax packets 
			having packet extension
			
			0-he_ltf, 1-last_data_symbol
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CONTENT_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CONTENT_LSB 53
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CONTENT_MSB 53
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CONTENT_MASK 0x0020000000000000


/* Description		DOT11AX_PE_LTF_SIZE

			LTF size to be used during packet extention. . This field
			 is valid for both FTM and non-FTM packets.
			0-1x
			1-2x (unsupported un HWK-1)
			2-4x (unsupported un HWK-1) 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_LTF_SIZE_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_LTF_SIZE_LSB 54
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_LTF_SIZE_MSB 55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_LTF_SIZE_MASK 0x00c0000000000000


/* Description		DOT11AX_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the preamble 
			and data portion of the packet. This field is valid for 
			all transmit packets
			0: disable per-chain csd
			1: enable per-chain csd 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CHAIN_CSD_EN_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CHAIN_CSD_EN_LSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CHAIN_CSD_EN_MSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_CHAIN_CSD_EN_MASK 0x0100000000000000


/* Description		DOT11AX_PE_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the packet extension
			 portion of the packet. This field is valid for all 11ax
			 packets.
			0: disable per-chain csd
			1: enable per-chain csd
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CHAIN_CSD_EN_LSB 57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CHAIN_CSD_EN_MSB 57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x0200000000000000


/* Description		DOT11AX_DL_UL_FLAG

			field is only valid for pkt_type == 11ax 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DL_UL_FLAG_OFFSET 0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DL_UL_FLAG_LSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DL_UL_FLAG_MSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_DL_UL_FLAG_MASK 0x0400000000000000


/* Description		RESERVED_4A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_4A_OFFSET  0x0000000000000038
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_4A_LSB     59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_4A_MSB     63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_4A_MASK    0xf800000000000000


/* Description		DOT11AX_EXT_RU_START_INDEX

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1
			
			RU Number to which User is assigned
			
			The RU numbering bitwidth  is only enough to cover the 20MHz
			 BW that extended range allows
			<legal 0-8>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_START_INDEX_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_START_INDEX_MSB 3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_START_INDEX_MASK 0x000000000000000f


/* Description		DOT11AX_EXT_RU_SIZE

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1 or pkt_type == 11be and EHT_duplicate_mode == 1
			
			The size of the RU for this user.
			
			In case of EHT duplicate transmissions, this field indicates
			 the width of the actual content before duplication, e.g. 
			a 40 MHz PPDU duplicated to 160 MHz will have the bandwidth
			 fields indicating 160 MHz and this field set to e-num 4
			 (RU_484).
			
			<enum 0 RU_26> 
			<enum 1 RU_52>
			<enum 2 RU_106>
			<enum 3 RU_242>
			<enum 4 RU_484>
			<enum 5 RU_996>
			<enum 6 RU_1992>
			<enum 7 RU_FULLBW> Set when the RU occupies the full packet
			 bandwidth
			<enum 8 RU_FULLBW_240> Set when the RU occupies the full
			 packet bandwidth
			<enum 9 RU_FULLBW_320> Set when the RU occupies the full
			 packet bandwidth
			<enum 10 RU_MULTI_LARGE> DO NOT USE
			<enum 11 RU_78> DO NOT USE
			<enum 12 RU_132> DO NOT USE
			<legal 0-12>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_SIZE_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_SIZE_LSB 4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_SIZE_MSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11AX_EXT_RU_SIZE_MASK 0x00000000000000f0


/* Description		EHT_DUPLICATE_MODE

			Field only valid for pkt_type == 11be
			
			Indicates EHT duplicate modulation
			
			<enum 0 eht_no_duplicate>
			<enum 1 eht_2x_duplicate>
			<enum 2 eht_4x_duplicate>
			
			<legal 0-2>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_EHT_DUPLICATE_MODE_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_EHT_DUPLICATE_MODE_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_EHT_DUPLICATE_MODE_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_EHT_DUPLICATE_MODE_MASK 0x0000000000000300


/* Description		HE_SIGB_DCM

			Indicates whether dual sub-carrier modulation is applied
			 to EHT-SIG
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_DCM_OFFSET  0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_DCM_LSB     10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_DCM_MSB     10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_DCM_MASK    0x0000000000000400


/* Description		HE_SIGB_0_MCS

			Indicates the MCS of EHT-SIG
			
			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_0_MCS_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_0_MCS_LSB   11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_0_MCS_MSB   13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_HE_SIGB_0_MCS_MASK  0x0000000000003800


/* Description		NUM_HE_SIGB_SYM

			Indicates the number of EHT-SIG symbols
			
			This field is 0-based with 0 indicating that 1 eht_sig symbol
			 needs to be transmitted.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NUM_HE_SIGB_SYM_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NUM_HE_SIGB_SYM_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NUM_HE_SIGB_SYM_MSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_NUM_HE_SIGB_SYM_MASK 0x000000000007c000


/* Description		REQUIRED_RESPONSE_TIME_SOURCE

			<enum 0 reqd_resp_time_src_is_RXPCU> Typically from received
			 HT Control for sync MLO response
			<enum 1 reqd_resp_time_src_is_FW>
			Typically from 'PCU_PPDU_SETUP_INIT' for sync MLO response
			 to response
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_SOURCE_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_SOURCE_MSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x0000000000080000


/* Description		RESERVED_5A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_5A_OFFSET  0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_5A_LSB     20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_5A_MSB     25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_RESERVED_5A_MASK    0x0000000003f00000


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			6-bit value copied from 'RX_RESPONSE_REQUIRED_INFO' and 'TX_CBF_INFO' 
			to pass on to PDG
			<legal 0-29>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0x00000000fc000000


/* Description		MLO_STA_ID_DETAILS_RX

			16-bi value copied from 'RX_RESPONSE_REQUIRED_INFO' to pass
			 on to PDG
			
			Bits 10 and 11 are not valid, bits [9:0] reflect 'NSTR_MLO_STA_ID' 
			from address search.
			
			See definition of mlo_sta_id_details.
*/


/* Description		NSTR_MLO_STA_ID

			ID of peer participating in non-STR MLO
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x000003ff00000000


/* Description		BLOCK_SELF_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for self-link.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000040000000000


/* Description		BLOCK_PARTNER_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for partner links.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000080000000000


/* Description		NSTR_MLO_STA_ID_VALID

			All the fields in this TLV are valid only if this bit is
			 set.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000100000000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x0000e00000000000


/* Description		REQUIRED_RESPONSE_TIME

			When non-zero, indicates that PDG shall pad the response
			 transmission to the indicated duration (in us)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_LSB 48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_MSB 59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_REQUIRED_RESPONSE_TIME_MASK 0x0fff000000000000


/* Description		DOT11BE_PARAMS_PLACEHOLDER

			4 bytes for use as placeholders for 'Dot11be_*' parameters
			
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x0000000000000040
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11BE_PARAMS_PLACEHOLDER_LSB 60
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11BE_PARAMS_PLACEHOLDER_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW40_DOT11BE_PARAMS_PLACEHOLDER_MASK 0xf000000000000000


/* Description		RESPONSE_TO_RESPONSE_RATE_INFO_BW80

			Field only valid in case of Response_to_response set to 
			SU_BA or MU_BA
			
			NOTE: This field is also known as response_to_response_rate_info_pattern_2
			 in case punctured transmission is enabled.
			
			Used by TXPCU to determine what the transmit rates are for
			 the response to response transmission in case original 
			transmission was 80 MHz.
			
			Note: 
			see field R2R_bw80_active_channel for the BW of this transmission
			
*/


/* Description		RESERVED_0A

			 
			<legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_0A_OFFSET  0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_0A_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_0A_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_0A_MASK    0x0000000000000001


/* Description		TX_ANTENNA_SECTOR_CTRL

			Sectored transmit antenna
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_ANTENNA_SECTOR_CTRL_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_ANTENNA_SECTOR_CTRL_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_ANTENNA_SECTOR_CTRL_MASK 0x0000000001fffffe


/* Description		PKT_TYPE

			Packet type:
			<enum 0 dot11a>802.11a PPDU type
			<enum 1 dot11b>802.11b PPDU type
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			<enum 3 dot11ac>802.11ac PPDU type
			<enum 4 dot11ax>802.11ax PPDU type
			<enum 5 dot11ba>802.11ba (WUR) PPDU type
			<enum 6 dot11be>802.11be PPDU type
			<enum 7 dot11az>802.11az (ranging) PPDU type
			<enum 8 dot11n_gf>802.11n Green Field PPDU type (unsupported
			 & aborted)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_PKT_TYPE_OFFSET     0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_PKT_TYPE_LSB        25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_PKT_TYPE_MSB        28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_PKT_TYPE_MASK       0x000000001e000000


/* Description		SMOOTHING

			This field is used by PDG to populate the SMOOTHING filed
			 in the SIG Preamble of the PPDU
			<legal 0-1>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SMOOTHING_OFFSET    0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SMOOTHING_LSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SMOOTHING_MSB       29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SMOOTHING_MASK      0x0000000020000000


/* Description		LDPC

			When set, use LDPC transmission rates
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_LDPC_OFFSET         0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_LDPC_LSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_LDPC_MSB            30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_LDPC_MASK           0x0000000040000000


/* Description		STBC

			When set, use STBC transmission rates
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STBC_OFFSET         0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STBC_LSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STBC_MSB            31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STBC_MASK           0x0000000080000000


/* Description		ALT_TX_PWR

			Coex related AlternativeTransmit parameter 
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_OFFSET   0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_LSB      32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_MSB      39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_MASK     0x000000ff00000000


/* Description		ALT_MIN_TX_PWR

			Coex related Alternative Transmit parameter 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_MIN_TX_PWR_OFFSET 0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_MIN_TX_PWR_LSB  40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_MIN_TX_PWR_MSB  47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_MIN_TX_PWR_MASK 0x0000ff0000000000


/* Description		ALT_NSS

			Coex related Alternative Transmit parameter
			
			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_NSS_OFFSET      0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_NSS_LSB         48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_NSS_MSB         50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_NSS_MASK        0x0007000000000000


/* Description		ALT_TX_CHAIN_MASK

			Coex related Alternative Transmit parameter
			
			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_CHAIN_MASK_OFFSET 0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_CHAIN_MASK_LSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_CHAIN_MASK_MSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_CHAIN_MASK_MASK 0x07f8000000000000


/* Description		ALT_BW

			Coex related Alternative Transmit parameter
			
			The BW of the upcoming transmission.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_BW_OFFSET       0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_BW_LSB          59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_BW_MSB          61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_BW_MASK         0x3800000000000000


/* Description		STF_LTF_3DB_BOOST

			Boost the STF and LTF power by 3dB in 11a/n/ac packets. 
			This includes both the legacy preambles and the HT/VHT preambles.0: 
			disable power boost1: enable power boost
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STF_LTF_3DB_BOOST_OFFSET 0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STF_LTF_3DB_BOOST_LSB 62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STF_LTF_3DB_BOOST_MSB 62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_STF_LTF_3DB_BOOST_MASK 0x4000000000000000


/* Description		FORCE_EXTRA_SYMBOL

			Set to 1 to force an extra OFDM symbol (or symbols) even
			 if the PPDU encoding process does not result in an extra
			 OFDM symbol (or symbols)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_FORCE_EXTRA_SYMBOL_OFFSET 0x0000000000000048
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_FORCE_EXTRA_SYMBOL_LSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_FORCE_EXTRA_SYMBOL_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_FORCE_EXTRA_SYMBOL_MASK 0x8000000000000000


/* Description		ALT_RATE_MCS

			Coex related Alternative Transmit parameter
			
			For details, refer to  MCS_TYPE 
			Note: This is "rate" in case of 11a/11b
			description
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_RATE_MCS_OFFSET 0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_RATE_MCS_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_RATE_MCS_MSB    3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_RATE_MCS_MASK   0x000000000000000f


/* Description		NSS

			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NSS_OFFSET          0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NSS_LSB             4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NSS_MSB             6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NSS_MASK            0x0000000000000070


/* Description		DPD_ENABLE

			DPD enable control
			
			This is needed on a per packet basis
			<enum 0     dpd_off> DPD profile not applied to current 
			packet
			<enum 1     dpd_on> DPD profile applied to current packet
			 if available
			<legal 0-1>
			
			This field is not applicable in11ah mode of operation and
			 is ignored by the HW 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DPD_ENABLE_OFFSET   0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DPD_ENABLE_LSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DPD_ENABLE_MSB      7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DPD_ENABLE_MASK     0x0000000000000080


/* Description		TX_PWR

			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_OFFSET       0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_LSB          8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_MSB          15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_MASK         0x000000000000ff00


/* Description		MIN_TX_PWR

			Coex related field: 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MIN_TX_PWR_OFFSET   0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MIN_TX_PWR_LSB      16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MIN_TX_PWR_MSB      23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MIN_TX_PWR_MASK     0x0000000000ff0000


/* Description		TX_CHAIN_MASK

			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_CHAIN_MASK_OFFSET 0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_CHAIN_MASK_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_CHAIN_MASK_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_CHAIN_MASK_MASK  0x00000000ff000000


/* Description		RESERVED_3A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3A_OFFSET  0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3A_LSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3A_MSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3A_MASK    0x000000ff00000000


/* Description		SGI

			Field only valid when pkt type is HT or VHT.For 11ax see
			 field Dot11ax_CP_LTF_size
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > Not used for pre 11ax pkt_types. 
			
			<enum 3     3_2_us_sgi > Not used for pre 11ax pkt_types
			
			
			<legal 0 - 3>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SGI_OFFSET          0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SGI_LSB             40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SGI_MSB             41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_SGI_MASK            0x0000030000000000


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RATE_MCS_OFFSET     0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RATE_MCS_LSB        42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RATE_MCS_MSB        45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RATE_MCS_MASK       0x00003c0000000000


/* Description		RESERVED_3B

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3B_OFFSET  0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3B_LSB     46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3B_MSB     47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_3B_MASK    0x0000c00000000000


/* Description		TX_PWR_1

			Default (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_1_OFFSET     0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_1_LSB        48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_1_MSB        55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_TX_PWR_1_MASK       0x00ff000000000000


/* Description		ALT_TX_PWR_1

			Alternate (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_1_OFFSET 0x0000000000000050
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_1_LSB    56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_1_MSB    63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_ALT_TX_PWR_1_MASK   0xff00000000000000


/* Description		AGGREGATION

			Field only valid in case of pkt_type == 11n
			
			<enum 0     mpdu> Indicates MPDU format. TXPCU will select
			 this setting if the CBF response only contains a single
			 segment
			<enum 1     a_mpdu> Indicates A-MPDU format. TXPCU will 
			select this setting if the CBF response will contain two
			 or more segments
			<legal 0-1>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_AGGREGATION_OFFSET  0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_AGGREGATION_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_AGGREGATION_MSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_AGGREGATION_MASK    0x0000000000000001


/* Description		DOT11AX_BSS_COLOR_ID

			BSS color of the nextwork to which this STA belongs. 
			When generated by TXPCU, this field is set equal to: Dot11ax_received_Bss_color_id
			
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_BSS_COLOR_ID_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_BSS_COLOR_ID_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_BSS_COLOR_ID_MSB 6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_BSS_COLOR_ID_MASK 0x000000000000007e


/* Description		DOT11AX_SPATIAL_REUSE

			This field is only valid for pkt_type == 11ax
			
			Spatial re-use
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SPATIAL_REUSE_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SPATIAL_REUSE_LSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SPATIAL_REUSE_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SPATIAL_REUSE_MASK 0x0000000000000780


/* Description		DOT11AX_CP_LTF_SIZE

			field is only valid for pkt_type == 11ax
			
			Indicates the CP and HE-LTF type 
			
			<enum 0 OneX_LTF_0_8CP> 1xLTF + 0.8 us CP
			<enum 1 TwoX_LTF_0_8CP> 2x LTF + 0.8 µs CP
			<enum 2 TwoX_LTF_1_6CP> 2x LTF + 1.6 µs CP
			<enum 3 FourX_LTF_0_8CP_3_2CP>
			When DCM == 0 OR STBC == 0: 4x LTF + 3.2 µs CP 
			When DCM == 1 AND STBC == 1: 4x LTF + 0.8 µs CP. Note: 
			In this scenario, Neither DCM nor STBC is applied to HE 
			data field.
			
			If ( DCM == 1 ) and ( MCS > 0 ) and (STBC == 0) 
			0      = 1xLTF + 0.4 usec
			1      = 2xLTF + 0.4 usec
			2~3 = Reserved
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CP_LTF_SIZE_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CP_LTF_SIZE_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CP_LTF_SIZE_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CP_LTF_SIZE_MASK 0x0000000000001800


/* Description		DOT11AX_DCM

			field is only valid for pkt_type == 11ax
			
			Indicates whether dual sub-carrier modulation is applied
			
			0: No DCM
			1:DCM
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DCM_OFFSET  0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DCM_LSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DCM_MSB     13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DCM_MASK    0x0000000000002000


/* Description		DOT11AX_DOPPLER_INDICATION

			field is only valid for pkt_type == 11ax
			
			0: No Doppler support
			1: Doppler support
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DOPPLER_INDICATION_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DOPPLER_INDICATION_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DOPPLER_INDICATION_MSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DOPPLER_INDICATION_MASK 0x0000000000004000


/* Description		DOT11AX_SU_EXTENDED

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			When set, the 11ax or 11be frame is of the extended range
			 format
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SU_EXTENDED_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SU_EXTENDED_LSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SU_EXTENDED_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_SU_EXTENDED_MASK 0x0000000000008000


/* Description		DOT11AX_MIN_PACKET_EXTENSION

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			The min packet extension duration for this user.
			0: no extension
			1: 8us
			2: 16 us
			3: 20 us (only for .11be)
			<legal 0-3>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_MIN_PACKET_EXTENSION_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_MIN_PACKET_EXTENSION_MSB 17
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x0000000000030000


/* Description		DOT11AX_PE_NSS

			Number of active spatial streams during packet extension.
			
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_NSS_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_NSS_LSB  18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_NSS_MSB  20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_NSS_MASK 0x00000000001c0000


/* Description		DOT11AX_PE_CONTENT

			Content of packet extension. Valid for all 11ax packets 
			having packet extension
			
			0-he_ltf, 1-last_data_symbol
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CONTENT_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CONTENT_LSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CONTENT_MSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CONTENT_MASK 0x0000000000200000


/* Description		DOT11AX_PE_LTF_SIZE

			LTF size to be used during packet extention. . This field
			 is valid for both FTM and non-FTM packets.
			0-1x
			1-2x (unsupported un HWK-1)
			2-4x (unsupported un HWK-1) 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_LTF_SIZE_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_LTF_SIZE_LSB 22
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_LTF_SIZE_MSB 23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_LTF_SIZE_MASK 0x0000000000c00000


/* Description		DOT11AX_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the preamble 
			and data portion of the packet. This field is valid for 
			all transmit packets
			0: disable per-chain csd
			1: enable per-chain csd 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CHAIN_CSD_EN_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CHAIN_CSD_EN_LSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CHAIN_CSD_EN_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_CHAIN_CSD_EN_MASK 0x0000000001000000


/* Description		DOT11AX_PE_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the packet extension
			 portion of the packet. This field is valid for all 11ax
			 packets.
			0: disable per-chain csd
			1: enable per-chain csd
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CHAIN_CSD_EN_LSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CHAIN_CSD_EN_MSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x0000000002000000


/* Description		DOT11AX_DL_UL_FLAG

			field is only valid for pkt_type == 11ax 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DL_UL_FLAG_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DL_UL_FLAG_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DL_UL_FLAG_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_DL_UL_FLAG_MASK 0x0000000004000000


/* Description		RESERVED_4A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_4A_OFFSET  0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_4A_LSB     27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_4A_MSB     31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_4A_MASK    0x00000000f8000000


/* Description		DOT11AX_EXT_RU_START_INDEX

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1
			
			RU Number to which User is assigned
			
			The RU numbering bitwidth  is only enough to cover the 20MHz
			 BW that extended range allows
			<legal 0-8>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_START_INDEX_LSB 32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_START_INDEX_MSB 35
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_START_INDEX_MASK 0x0000000f00000000


/* Description		DOT11AX_EXT_RU_SIZE

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1 or pkt_type == 11be and EHT_duplicate_mode == 1
			
			The size of the RU for this user.
			
			In case of EHT duplicate transmissions, this field indicates
			 the width of the actual content before duplication, e.g. 
			a 40 MHz PPDU duplicated to 160 MHz will have the bandwidth
			 fields indicating 160 MHz and this field set to e-num 4
			 (RU_484).
			
			<enum 0 RU_26> 
			<enum 1 RU_52>
			<enum 2 RU_106>
			<enum 3 RU_242>
			<enum 4 RU_484>
			<enum 5 RU_996>
			<enum 6 RU_1992>
			<enum 7 RU_FULLBW> Set when the RU occupies the full packet
			 bandwidth
			<enum 8 RU_FULLBW_240> Set when the RU occupies the full
			 packet bandwidth
			<enum 9 RU_FULLBW_320> Set when the RU occupies the full
			 packet bandwidth
			<enum 10 RU_MULTI_LARGE> DO NOT USE
			<enum 11 RU_78> DO NOT USE
			<enum 12 RU_132> DO NOT USE
			<legal 0-12>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_SIZE_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_SIZE_LSB 36
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_SIZE_MSB 39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11AX_EXT_RU_SIZE_MASK 0x000000f000000000


/* Description		EHT_DUPLICATE_MODE

			Field only valid for pkt_type == 11be
			
			Indicates EHT duplicate modulation
			
			<enum 0 eht_no_duplicate>
			<enum 1 eht_2x_duplicate>
			<enum 2 eht_4x_duplicate>
			
			<legal 0-2>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_EHT_DUPLICATE_MODE_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_EHT_DUPLICATE_MODE_LSB 40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_EHT_DUPLICATE_MODE_MSB 41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_EHT_DUPLICATE_MODE_MASK 0x0000030000000000


/* Description		HE_SIGB_DCM

			Indicates whether dual sub-carrier modulation is applied
			 to EHT-SIG
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_DCM_OFFSET  0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_DCM_LSB     42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_DCM_MSB     42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_DCM_MASK    0x0000040000000000


/* Description		HE_SIGB_0_MCS

			Indicates the MCS of EHT-SIG
			
			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_0_MCS_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_0_MCS_LSB   43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_0_MCS_MSB   45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_HE_SIGB_0_MCS_MASK  0x0000380000000000


/* Description		NUM_HE_SIGB_SYM

			Indicates the number of EHT-SIG symbols
			
			This field is 0-based with 0 indicating that 1 eht_sig symbol
			 needs to be transmitted.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NUM_HE_SIGB_SYM_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NUM_HE_SIGB_SYM_LSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NUM_HE_SIGB_SYM_MSB 50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_NUM_HE_SIGB_SYM_MASK 0x0007c00000000000


/* Description		REQUIRED_RESPONSE_TIME_SOURCE

			<enum 0 reqd_resp_time_src_is_RXPCU> Typically from received
			 HT Control for sync MLO response
			<enum 1 reqd_resp_time_src_is_FW>
			Typically from 'PCU_PPDU_SETUP_INIT' for sync MLO response
			 to response
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_SOURCE_LSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_SOURCE_MSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x0008000000000000


/* Description		RESERVED_5A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_5A_OFFSET  0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_5A_LSB     52
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_5A_MSB     57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_RESERVED_5A_MASK    0x03f0000000000000


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			6-bit value copied from 'RX_RESPONSE_REQUIRED_INFO' and 'TX_CBF_INFO' 
			to pass on to PDG
			<legal 0-29>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x0000000000000058
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0xfc00000000000000


/* Description		MLO_STA_ID_DETAILS_RX

			16-bi value copied from 'RX_RESPONSE_REQUIRED_INFO' to pass
			 on to PDG
			
			Bits 10 and 11 are not valid, bits [9:0] reflect 'NSTR_MLO_STA_ID' 
			from address search.
			
			See definition of mlo_sta_id_details.
*/


/* Description		NSTR_MLO_STA_ID

			ID of peer participating in non-STR MLO
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x00000000000003ff


/* Description		BLOCK_SELF_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for self-link.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000000000000400


/* Description		BLOCK_PARTNER_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for partner links.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000000000000800


/* Description		NSTR_MLO_STA_ID_VALID

			All the fields in this TLV are valid only if this bit is
			 set.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000000000001000


/* Description		RESERVED_0A

			<legal 0>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x000000000000e000


/* Description		REQUIRED_RESPONSE_TIME

			When non-zero, indicates that PDG shall pad the response
			 transmission to the indicated duration (in us)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_MSB 27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_REQUIRED_RESPONSE_TIME_MASK 0x000000000fff0000


/* Description		DOT11BE_PARAMS_PLACEHOLDER

			4 bytes for use as placeholders for 'Dot11be_*' parameters
			
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11BE_PARAMS_PLACEHOLDER_LSB 28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11BE_PARAMS_PLACEHOLDER_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW80_DOT11BE_PARAMS_PLACEHOLDER_MASK 0x00000000f0000000


/* Description		RESPONSE_TO_RESPONSE_RATE_INFO_BW160

			Field only valid in case of Response_to_response set to 
			SU_BA or MU_BA
			
			NOTE: This field is also known as response_to_response_rate_info_pattern_3
			 in case punctured transmission is enabled.
			
			Used by TXPCU to determine what the transmit rates are for
			 the response to response transmission in case original 
			transmission was 160 MHz.
			
			Note: 
			see field R2R_bw160_active_channel for the BW of this transmission
			
*/


/* Description		RESERVED_0A

			 
			<legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_0A_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_0A_LSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_0A_MSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_0A_MASK   0x0000000100000000


/* Description		TX_ANTENNA_SECTOR_CTRL

			Sectored transmit antenna
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_ANTENNA_SECTOR_CTRL_LSB 33
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_ANTENNA_SECTOR_CTRL_MSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_ANTENNA_SECTOR_CTRL_MASK 0x01fffffe00000000


/* Description		PKT_TYPE

			Packet type:
			<enum 0 dot11a>802.11a PPDU type
			<enum 1 dot11b>802.11b PPDU type
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			<enum 3 dot11ac>802.11ac PPDU type
			<enum 4 dot11ax>802.11ax PPDU type
			<enum 5 dot11ba>802.11ba (WUR) PPDU type
			<enum 6 dot11be>802.11be PPDU type
			<enum 7 dot11az>802.11az (ranging) PPDU type
			<enum 8 dot11n_gf>802.11n Green Field PPDU type (unsupported
			 & aborted)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_PKT_TYPE_OFFSET    0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_PKT_TYPE_LSB       57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_PKT_TYPE_MSB       60
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_PKT_TYPE_MASK      0x1e00000000000000


/* Description		SMOOTHING

			This field is used by PDG to populate the SMOOTHING filed
			 in the SIG Preamble of the PPDU
			<legal 0-1>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SMOOTHING_OFFSET   0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SMOOTHING_LSB      61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SMOOTHING_MSB      61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SMOOTHING_MASK     0x2000000000000000


/* Description		LDPC

			When set, use LDPC transmission rates
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_LDPC_OFFSET        0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_LDPC_LSB           62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_LDPC_MSB           62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_LDPC_MASK          0x4000000000000000


/* Description		STBC

			When set, use STBC transmission rates
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STBC_OFFSET        0x0000000000000060
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STBC_LSB           63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STBC_MSB           63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STBC_MASK          0x8000000000000000


/* Description		ALT_TX_PWR

			Coex related AlternativeTransmit parameter 
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_OFFSET  0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_MASK    0x00000000000000ff


/* Description		ALT_MIN_TX_PWR

			Coex related Alternative Transmit parameter 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_MIN_TX_PWR_OFFSET 0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_MIN_TX_PWR_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_MIN_TX_PWR_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_MIN_TX_PWR_MASK 0x000000000000ff00


/* Description		ALT_NSS

			Coex related Alternative Transmit parameter
			
			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_NSS_OFFSET     0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_NSS_LSB        16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_NSS_MSB        18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_NSS_MASK       0x0000000000070000


/* Description		ALT_TX_CHAIN_MASK

			Coex related Alternative Transmit parameter
			
			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_CHAIN_MASK_OFFSET 0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_CHAIN_MASK_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_CHAIN_MASK_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_CHAIN_MASK_MASK 0x0000000007f80000


/* Description		ALT_BW

			Coex related Alternative Transmit parameter
			
			The BW of the upcoming transmission.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_BW_OFFSET      0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_BW_LSB         27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_BW_MSB         29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_BW_MASK        0x0000000038000000


/* Description		STF_LTF_3DB_BOOST

			Boost the STF and LTF power by 3dB in 11a/n/ac packets. 
			This includes both the legacy preambles and the HT/VHT preambles.0: 
			disable power boost1: enable power boost
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STF_LTF_3DB_BOOST_OFFSET 0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STF_LTF_3DB_BOOST_LSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STF_LTF_3DB_BOOST_MSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_STF_LTF_3DB_BOOST_MASK 0x0000000040000000


/* Description		FORCE_EXTRA_SYMBOL

			Set to 1 to force an extra OFDM symbol (or symbols) even
			 if the PPDU encoding process does not result in an extra
			 OFDM symbol (or symbols)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_FORCE_EXTRA_SYMBOL_OFFSET 0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_FORCE_EXTRA_SYMBOL_LSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_FORCE_EXTRA_SYMBOL_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_FORCE_EXTRA_SYMBOL_MASK 0x0000000080000000


/* Description		ALT_RATE_MCS

			Coex related Alternative Transmit parameter
			
			For details, refer to  MCS_TYPE 
			Note: This is "rate" in case of 11a/11b
			description
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_RATE_MCS_OFFSET 0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_RATE_MCS_LSB   32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_RATE_MCS_MSB   35
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_RATE_MCS_MASK  0x0000000f00000000


/* Description		NSS

			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NSS_OFFSET         0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NSS_LSB            36
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NSS_MSB            38
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NSS_MASK           0x0000007000000000


/* Description		DPD_ENABLE

			DPD enable control
			
			This is needed on a per packet basis
			<enum 0     dpd_off> DPD profile not applied to current 
			packet
			<enum 1     dpd_on> DPD profile applied to current packet
			 if available
			<legal 0-1>
			
			This field is not applicable in11ah mode of operation and
			 is ignored by the HW 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DPD_ENABLE_OFFSET  0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DPD_ENABLE_LSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DPD_ENABLE_MSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DPD_ENABLE_MASK    0x0000008000000000


/* Description		TX_PWR

			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_OFFSET      0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_LSB         40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_MSB         47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_MASK        0x0000ff0000000000


/* Description		MIN_TX_PWR

			Coex related field: 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MIN_TX_PWR_OFFSET  0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MIN_TX_PWR_LSB     48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MIN_TX_PWR_MSB     55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MIN_TX_PWR_MASK    0x00ff000000000000


/* Description		TX_CHAIN_MASK

			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_CHAIN_MASK_OFFSET 0x0000000000000068
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_CHAIN_MASK_LSB  56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_CHAIN_MASK_MSB  63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_CHAIN_MASK_MASK 0xff00000000000000


/* Description		RESERVED_3A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3A_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3A_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3A_MSB    7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3A_MASK   0x00000000000000ff


/* Description		SGI

			Field only valid when pkt type is HT or VHT.For 11ax see
			 field Dot11ax_CP_LTF_size
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > Not used for pre 11ax pkt_types. 
			
			<enum 3     3_2_us_sgi > Not used for pre 11ax pkt_types
			
			
			<legal 0 - 3>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SGI_OFFSET         0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SGI_LSB            8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SGI_MSB            9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_SGI_MASK           0x0000000000000300


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RATE_MCS_OFFSET    0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RATE_MCS_LSB       10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RATE_MCS_MSB       13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RATE_MCS_MASK      0x0000000000003c00


/* Description		RESERVED_3B

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3B_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3B_LSB    14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3B_MSB    15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_3B_MASK   0x000000000000c000


/* Description		TX_PWR_1

			Default (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_1_OFFSET    0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_1_LSB       16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_1_MSB       23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_TX_PWR_1_MASK      0x0000000000ff0000


/* Description		ALT_TX_PWR_1

			Alternate (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_1_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_1_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_1_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_ALT_TX_PWR_1_MASK  0x00000000ff000000


/* Description		AGGREGATION

			Field only valid in case of pkt_type == 11n
			
			<enum 0     mpdu> Indicates MPDU format. TXPCU will select
			 this setting if the CBF response only contains a single
			 segment
			<enum 1     a_mpdu> Indicates A-MPDU format. TXPCU will 
			select this setting if the CBF response will contain two
			 or more segments
			<legal 0-1>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_AGGREGATION_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_AGGREGATION_LSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_AGGREGATION_MSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_AGGREGATION_MASK   0x0000000100000000


/* Description		DOT11AX_BSS_COLOR_ID

			BSS color of the nextwork to which this STA belongs. 
			When generated by TXPCU, this field is set equal to: Dot11ax_received_Bss_color_id
			
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_BSS_COLOR_ID_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_BSS_COLOR_ID_LSB 33
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_BSS_COLOR_ID_MSB 38
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_BSS_COLOR_ID_MASK 0x0000007e00000000


/* Description		DOT11AX_SPATIAL_REUSE

			This field is only valid for pkt_type == 11ax
			
			Spatial re-use
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SPATIAL_REUSE_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SPATIAL_REUSE_LSB 39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SPATIAL_REUSE_MSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SPATIAL_REUSE_MASK 0x0000078000000000


/* Description		DOT11AX_CP_LTF_SIZE

			field is only valid for pkt_type == 11ax
			
			Indicates the CP and HE-LTF type 
			
			<enum 0 OneX_LTF_0_8CP> 1xLTF + 0.8 us CP
			<enum 1 TwoX_LTF_0_8CP> 2x LTF + 0.8 µs CP
			<enum 2 TwoX_LTF_1_6CP> 2x LTF + 1.6 µs CP
			<enum 3 FourX_LTF_0_8CP_3_2CP>
			When DCM == 0 OR STBC == 0: 4x LTF + 3.2 µs CP 
			When DCM == 1 AND STBC == 1: 4x LTF + 0.8 µs CP. Note: 
			In this scenario, Neither DCM nor STBC is applied to HE 
			data field.
			
			If ( DCM == 1 ) and ( MCS > 0 ) and (STBC == 0) 
			0      = 1xLTF + 0.4 usec
			1      = 2xLTF + 0.4 usec
			2~3 = Reserved
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CP_LTF_SIZE_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CP_LTF_SIZE_LSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CP_LTF_SIZE_MSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CP_LTF_SIZE_MASK 0x0000180000000000


/* Description		DOT11AX_DCM

			field is only valid for pkt_type == 11ax
			
			Indicates whether dual sub-carrier modulation is applied
			
			0: No DCM
			1:DCM
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DCM_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DCM_LSB    45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DCM_MSB    45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DCM_MASK   0x0000200000000000


/* Description		DOT11AX_DOPPLER_INDICATION

			field is only valid for pkt_type == 11ax
			
			0: No Doppler support
			1: Doppler support
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DOPPLER_INDICATION_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DOPPLER_INDICATION_LSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DOPPLER_INDICATION_MSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DOPPLER_INDICATION_MASK 0x0000400000000000


/* Description		DOT11AX_SU_EXTENDED

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			When set, the 11ax or 11be frame is of the extended range
			 format
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SU_EXTENDED_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SU_EXTENDED_LSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SU_EXTENDED_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_SU_EXTENDED_MASK 0x0000800000000000


/* Description		DOT11AX_MIN_PACKET_EXTENSION

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			The min packet extension duration for this user.
			0: no extension
			1: 8us
			2: 16 us
			3: 20 us (only for .11be)
			<legal 0-3>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_MIN_PACKET_EXTENSION_LSB 48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_MIN_PACKET_EXTENSION_MSB 49
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x0003000000000000


/* Description		DOT11AX_PE_NSS

			Number of active spatial streams during packet extension.
			
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_NSS_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_NSS_LSB 50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_NSS_MSB 52
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_NSS_MASK 0x001c000000000000


/* Description		DOT11AX_PE_CONTENT

			Content of packet extension. Valid for all 11ax packets 
			having packet extension
			
			0-he_ltf, 1-last_data_symbol
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CONTENT_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CONTENT_LSB 53
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CONTENT_MSB 53
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CONTENT_MASK 0x0020000000000000


/* Description		DOT11AX_PE_LTF_SIZE

			LTF size to be used during packet extention. . This field
			 is valid for both FTM and non-FTM packets.
			0-1x
			1-2x (unsupported un HWK-1)
			2-4x (unsupported un HWK-1) 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_LTF_SIZE_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_LTF_SIZE_LSB 54
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_LTF_SIZE_MSB 55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_LTF_SIZE_MASK 0x00c0000000000000


/* Description		DOT11AX_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the preamble 
			and data portion of the packet. This field is valid for 
			all transmit packets
			0: disable per-chain csd
			1: enable per-chain csd 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CHAIN_CSD_EN_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CHAIN_CSD_EN_LSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CHAIN_CSD_EN_MSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_CHAIN_CSD_EN_MASK 0x0100000000000000


/* Description		DOT11AX_PE_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the packet extension
			 portion of the packet. This field is valid for all 11ax
			 packets.
			0: disable per-chain csd
			1: enable per-chain csd
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CHAIN_CSD_EN_LSB 57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CHAIN_CSD_EN_MSB 57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x0200000000000000


/* Description		DOT11AX_DL_UL_FLAG

			field is only valid for pkt_type == 11ax 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DL_UL_FLAG_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DL_UL_FLAG_LSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DL_UL_FLAG_MSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_DL_UL_FLAG_MASK 0x0400000000000000


/* Description		RESERVED_4A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_4A_OFFSET 0x0000000000000070
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_4A_LSB    59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_4A_MSB    63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_4A_MASK   0xf800000000000000


/* Description		DOT11AX_EXT_RU_START_INDEX

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1
			
			RU Number to which User is assigned
			
			The RU numbering bitwidth  is only enough to cover the 20MHz
			 BW that extended range allows
			<legal 0-8>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_START_INDEX_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_START_INDEX_MSB 3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_START_INDEX_MASK 0x000000000000000f


/* Description		DOT11AX_EXT_RU_SIZE

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1 or pkt_type == 11be and EHT_duplicate_mode == 1
			
			The size of the RU for this user.
			
			In case of EHT duplicate transmissions, this field indicates
			 the width of the actual content before duplication, e.g. 
			a 40 MHz PPDU duplicated to 160 MHz will have the bandwidth
			 fields indicating 160 MHz and this field set to e-num 4
			 (RU_484).
			
			<enum 0 RU_26> 
			<enum 1 RU_52>
			<enum 2 RU_106>
			<enum 3 RU_242>
			<enum 4 RU_484>
			<enum 5 RU_996>
			<enum 6 RU_1992>
			<enum 7 RU_FULLBW> Set when the RU occupies the full packet
			 bandwidth
			<enum 8 RU_FULLBW_240> Set when the RU occupies the full
			 packet bandwidth
			<enum 9 RU_FULLBW_320> Set when the RU occupies the full
			 packet bandwidth
			<enum 10 RU_MULTI_LARGE> DO NOT USE
			<enum 11 RU_78> DO NOT USE
			<enum 12 RU_132> DO NOT USE
			<legal 0-12>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_SIZE_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_SIZE_LSB 4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_SIZE_MSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11AX_EXT_RU_SIZE_MASK 0x00000000000000f0


/* Description		EHT_DUPLICATE_MODE

			Field only valid for pkt_type == 11be
			
			Indicates EHT duplicate modulation
			
			<enum 0 eht_no_duplicate>
			<enum 1 eht_2x_duplicate>
			<enum 2 eht_4x_duplicate>
			
			<legal 0-2>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_EHT_DUPLICATE_MODE_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_EHT_DUPLICATE_MODE_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_EHT_DUPLICATE_MODE_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_EHT_DUPLICATE_MODE_MASK 0x0000000000000300


/* Description		HE_SIGB_DCM

			Indicates whether dual sub-carrier modulation is applied
			 to EHT-SIG
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_DCM_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_DCM_LSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_DCM_MSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_DCM_MASK   0x0000000000000400


/* Description		HE_SIGB_0_MCS

			Indicates the MCS of EHT-SIG
			
			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_0_MCS_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_0_MCS_LSB  11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_0_MCS_MSB  13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_HE_SIGB_0_MCS_MASK 0x0000000000003800


/* Description		NUM_HE_SIGB_SYM

			Indicates the number of EHT-SIG symbols
			
			This field is 0-based with 0 indicating that 1 eht_sig symbol
			 needs to be transmitted.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NUM_HE_SIGB_SYM_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NUM_HE_SIGB_SYM_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NUM_HE_SIGB_SYM_MSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_NUM_HE_SIGB_SYM_MASK 0x000000000007c000


/* Description		REQUIRED_RESPONSE_TIME_SOURCE

			<enum 0 reqd_resp_time_src_is_RXPCU> Typically from received
			 HT Control for sync MLO response
			<enum 1 reqd_resp_time_src_is_FW>
			Typically from 'PCU_PPDU_SETUP_INIT' for sync MLO response
			 to response
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_SOURCE_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_SOURCE_MSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x0000000000080000


/* Description		RESERVED_5A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_5A_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_5A_LSB    20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_5A_MSB    25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_RESERVED_5A_MASK   0x0000000003f00000


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			6-bit value copied from 'RX_RESPONSE_REQUIRED_INFO' and 'TX_CBF_INFO' 
			to pass on to PDG
			<legal 0-29>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0x00000000fc000000


/* Description		MLO_STA_ID_DETAILS_RX

			16-bi value copied from 'RX_RESPONSE_REQUIRED_INFO' to pass
			 on to PDG
			
			Bits 10 and 11 are not valid, bits [9:0] reflect 'NSTR_MLO_STA_ID' 
			from address search.
			
			See definition of mlo_sta_id_details.
*/


/* Description		NSTR_MLO_STA_ID

			ID of peer participating in non-STR MLO
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x000003ff00000000


/* Description		BLOCK_SELF_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for self-link.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000040000000000


/* Description		BLOCK_PARTNER_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for partner links.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000080000000000


/* Description		NSTR_MLO_STA_ID_VALID

			All the fields in this TLV are valid only if this bit is
			 set.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000100000000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x0000e00000000000


/* Description		REQUIRED_RESPONSE_TIME

			When non-zero, indicates that PDG shall pad the response
			 transmission to the indicated duration (in us)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_LSB 48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_MSB 59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_REQUIRED_RESPONSE_TIME_MASK 0x0fff000000000000


/* Description		DOT11BE_PARAMS_PLACEHOLDER

			4 bytes for use as placeholders for 'Dot11be_*' parameters
			
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x0000000000000078
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11BE_PARAMS_PLACEHOLDER_LSB 60
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11BE_PARAMS_PLACEHOLDER_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW160_DOT11BE_PARAMS_PLACEHOLDER_MASK 0xf000000000000000


/* Description		RESPONSE_TO_RESPONSE_RATE_INFO_BW240

			Field only valid in case of Response_to_response set to 
			SU_BA or MU_BA
			
			NOTE: This field is also known as response_to_response_rate_info_pattern_4
			 in case punctured transmission is enabled.
			
			Used by TXPCU to determine what the transmit rates are for
			 the response to response transmission in case original 
			transmission was 240 MHz.
			
			Note: 
			see field R2R_bw240_active_channel for the BW of this transmission
			
*/


/* Description		RESERVED_0A

			 
			<legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_0A_OFFSET 0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_0A_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_0A_MSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_0A_MASK   0x0000000000000001


/* Description		TX_ANTENNA_SECTOR_CTRL

			Sectored transmit antenna
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_ANTENNA_SECTOR_CTRL_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_ANTENNA_SECTOR_CTRL_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_ANTENNA_SECTOR_CTRL_MASK 0x0000000001fffffe


/* Description		PKT_TYPE

			Packet type:
			<enum 0 dot11a>802.11a PPDU type
			<enum 1 dot11b>802.11b PPDU type
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			<enum 3 dot11ac>802.11ac PPDU type
			<enum 4 dot11ax>802.11ax PPDU type
			<enum 5 dot11ba>802.11ba (WUR) PPDU type
			<enum 6 dot11be>802.11be PPDU type
			<enum 7 dot11az>802.11az (ranging) PPDU type
			<enum 8 dot11n_gf>802.11n Green Field PPDU type (unsupported
			 & aborted)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_PKT_TYPE_OFFSET    0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_PKT_TYPE_LSB       25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_PKT_TYPE_MSB       28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_PKT_TYPE_MASK      0x000000001e000000


/* Description		SMOOTHING

			This field is used by PDG to populate the SMOOTHING filed
			 in the SIG Preamble of the PPDU
			<legal 0-1>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SMOOTHING_OFFSET   0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SMOOTHING_LSB      29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SMOOTHING_MSB      29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SMOOTHING_MASK     0x0000000020000000


/* Description		LDPC

			When set, use LDPC transmission rates
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_LDPC_OFFSET        0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_LDPC_LSB           30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_LDPC_MSB           30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_LDPC_MASK          0x0000000040000000


/* Description		STBC

			When set, use STBC transmission rates
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STBC_OFFSET        0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STBC_LSB           31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STBC_MSB           31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STBC_MASK          0x0000000080000000


/* Description		ALT_TX_PWR

			Coex related AlternativeTransmit parameter 
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_OFFSET  0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_LSB     32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_MSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_MASK    0x000000ff00000000


/* Description		ALT_MIN_TX_PWR

			Coex related Alternative Transmit parameter 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_MIN_TX_PWR_OFFSET 0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_MIN_TX_PWR_LSB 40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_MIN_TX_PWR_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_MIN_TX_PWR_MASK 0x0000ff0000000000


/* Description		ALT_NSS

			Coex related Alternative Transmit parameter
			
			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_NSS_OFFSET     0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_NSS_LSB        48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_NSS_MSB        50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_NSS_MASK       0x0007000000000000


/* Description		ALT_TX_CHAIN_MASK

			Coex related Alternative Transmit parameter
			
			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_CHAIN_MASK_OFFSET 0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_CHAIN_MASK_LSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_CHAIN_MASK_MSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_CHAIN_MASK_MASK 0x07f8000000000000


/* Description		ALT_BW

			Coex related Alternative Transmit parameter
			
			The BW of the upcoming transmission.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_BW_OFFSET      0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_BW_LSB         59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_BW_MSB         61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_BW_MASK        0x3800000000000000


/* Description		STF_LTF_3DB_BOOST

			Boost the STF and LTF power by 3dB in 11a/n/ac packets. 
			This includes both the legacy preambles and the HT/VHT preambles.0: 
			disable power boost1: enable power boost
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STF_LTF_3DB_BOOST_OFFSET 0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STF_LTF_3DB_BOOST_LSB 62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STF_LTF_3DB_BOOST_MSB 62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_STF_LTF_3DB_BOOST_MASK 0x4000000000000000


/* Description		FORCE_EXTRA_SYMBOL

			Set to 1 to force an extra OFDM symbol (or symbols) even
			 if the PPDU encoding process does not result in an extra
			 OFDM symbol (or symbols)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_FORCE_EXTRA_SYMBOL_OFFSET 0x0000000000000080
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_FORCE_EXTRA_SYMBOL_LSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_FORCE_EXTRA_SYMBOL_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_FORCE_EXTRA_SYMBOL_MASK 0x8000000000000000


/* Description		ALT_RATE_MCS

			Coex related Alternative Transmit parameter
			
			For details, refer to  MCS_TYPE 
			Note: This is "rate" in case of 11a/11b
			description
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_RATE_MCS_OFFSET 0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_RATE_MCS_LSB   0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_RATE_MCS_MSB   3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_RATE_MCS_MASK  0x000000000000000f


/* Description		NSS

			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NSS_OFFSET         0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NSS_LSB            4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NSS_MSB            6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NSS_MASK           0x0000000000000070


/* Description		DPD_ENABLE

			DPD enable control
			
			This is needed on a per packet basis
			<enum 0     dpd_off> DPD profile not applied to current 
			packet
			<enum 1     dpd_on> DPD profile applied to current packet
			 if available
			<legal 0-1>
			
			This field is not applicable in11ah mode of operation and
			 is ignored by the HW 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DPD_ENABLE_OFFSET  0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DPD_ENABLE_LSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DPD_ENABLE_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DPD_ENABLE_MASK    0x0000000000000080


/* Description		TX_PWR

			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_OFFSET      0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_LSB         8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_MSB         15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_MASK        0x000000000000ff00


/* Description		MIN_TX_PWR

			Coex related field: 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MIN_TX_PWR_OFFSET  0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MIN_TX_PWR_LSB     16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MIN_TX_PWR_MSB     23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MIN_TX_PWR_MASK    0x0000000000ff0000


/* Description		TX_CHAIN_MASK

			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_CHAIN_MASK_OFFSET 0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_CHAIN_MASK_LSB  24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_CHAIN_MASK_MSB  31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_CHAIN_MASK_MASK 0x00000000ff000000


/* Description		RESERVED_3A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3A_OFFSET 0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3A_LSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3A_MSB    39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3A_MASK   0x000000ff00000000


/* Description		SGI

			Field only valid when pkt type is HT or VHT.For 11ax see
			 field Dot11ax_CP_LTF_size
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > Not used for pre 11ax pkt_types. 
			
			<enum 3     3_2_us_sgi > Not used for pre 11ax pkt_types
			
			
			<legal 0 - 3>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SGI_OFFSET         0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SGI_LSB            40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SGI_MSB            41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_SGI_MASK           0x0000030000000000


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RATE_MCS_OFFSET    0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RATE_MCS_LSB       42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RATE_MCS_MSB       45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RATE_MCS_MASK      0x00003c0000000000


/* Description		RESERVED_3B

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3B_OFFSET 0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3B_LSB    46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3B_MSB    47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_3B_MASK   0x0000c00000000000


/* Description		TX_PWR_1

			Default (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_1_OFFSET    0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_1_LSB       48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_1_MSB       55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_TX_PWR_1_MASK      0x00ff000000000000


/* Description		ALT_TX_PWR_1

			Alternate (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_1_OFFSET 0x0000000000000088
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_1_LSB   56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_1_MSB   63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_ALT_TX_PWR_1_MASK  0xff00000000000000


/* Description		AGGREGATION

			Field only valid in case of pkt_type == 11n
			
			<enum 0     mpdu> Indicates MPDU format. TXPCU will select
			 this setting if the CBF response only contains a single
			 segment
			<enum 1     a_mpdu> Indicates A-MPDU format. TXPCU will 
			select this setting if the CBF response will contain two
			 or more segments
			<legal 0-1>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_AGGREGATION_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_AGGREGATION_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_AGGREGATION_MSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_AGGREGATION_MASK   0x0000000000000001


/* Description		DOT11AX_BSS_COLOR_ID

			BSS color of the nextwork to which this STA belongs. 
			When generated by TXPCU, this field is set equal to: Dot11ax_received_Bss_color_id
			
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_BSS_COLOR_ID_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_BSS_COLOR_ID_LSB 1
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_BSS_COLOR_ID_MSB 6
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_BSS_COLOR_ID_MASK 0x000000000000007e


/* Description		DOT11AX_SPATIAL_REUSE

			This field is only valid for pkt_type == 11ax
			
			Spatial re-use
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SPATIAL_REUSE_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SPATIAL_REUSE_LSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SPATIAL_REUSE_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SPATIAL_REUSE_MASK 0x0000000000000780


/* Description		DOT11AX_CP_LTF_SIZE

			field is only valid for pkt_type == 11ax
			
			Indicates the CP and HE-LTF type 
			
			<enum 0 OneX_LTF_0_8CP> 1xLTF + 0.8 us CP
			<enum 1 TwoX_LTF_0_8CP> 2x LTF + 0.8 µs CP
			<enum 2 TwoX_LTF_1_6CP> 2x LTF + 1.6 µs CP
			<enum 3 FourX_LTF_0_8CP_3_2CP>
			When DCM == 0 OR STBC == 0: 4x LTF + 3.2 µs CP 
			When DCM == 1 AND STBC == 1: 4x LTF + 0.8 µs CP. Note: 
			In this scenario, Neither DCM nor STBC is applied to HE 
			data field.
			
			If ( DCM == 1 ) and ( MCS > 0 ) and (STBC == 0) 
			0      = 1xLTF + 0.4 usec
			1      = 2xLTF + 0.4 usec
			2~3 = Reserved
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CP_LTF_SIZE_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CP_LTF_SIZE_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CP_LTF_SIZE_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CP_LTF_SIZE_MASK 0x0000000000001800


/* Description		DOT11AX_DCM

			field is only valid for pkt_type == 11ax
			
			Indicates whether dual sub-carrier modulation is applied
			
			0: No DCM
			1:DCM
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DCM_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DCM_LSB    13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DCM_MSB    13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DCM_MASK   0x0000000000002000


/* Description		DOT11AX_DOPPLER_INDICATION

			field is only valid for pkt_type == 11ax
			
			0: No Doppler support
			1: Doppler support
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DOPPLER_INDICATION_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DOPPLER_INDICATION_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DOPPLER_INDICATION_MSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DOPPLER_INDICATION_MASK 0x0000000000004000


/* Description		DOT11AX_SU_EXTENDED

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			When set, the 11ax or 11be frame is of the extended range
			 format
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SU_EXTENDED_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SU_EXTENDED_LSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SU_EXTENDED_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_SU_EXTENDED_MASK 0x0000000000008000


/* Description		DOT11AX_MIN_PACKET_EXTENSION

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			The min packet extension duration for this user.
			0: no extension
			1: 8us
			2: 16 us
			3: 20 us (only for .11be)
			<legal 0-3>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_MIN_PACKET_EXTENSION_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_MIN_PACKET_EXTENSION_MSB 17
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x0000000000030000


/* Description		DOT11AX_PE_NSS

			Number of active spatial streams during packet extension.
			
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_NSS_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_NSS_LSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_NSS_MSB 20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_NSS_MASK 0x00000000001c0000


/* Description		DOT11AX_PE_CONTENT

			Content of packet extension. Valid for all 11ax packets 
			having packet extension
			
			0-he_ltf, 1-last_data_symbol
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CONTENT_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CONTENT_LSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CONTENT_MSB 21
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CONTENT_MASK 0x0000000000200000


/* Description		DOT11AX_PE_LTF_SIZE

			LTF size to be used during packet extention. . This field
			 is valid for both FTM and non-FTM packets.
			0-1x
			1-2x (unsupported un HWK-1)
			2-4x (unsupported un HWK-1) 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_LTF_SIZE_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_LTF_SIZE_LSB 22
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_LTF_SIZE_MSB 23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_LTF_SIZE_MASK 0x0000000000c00000


/* Description		DOT11AX_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the preamble 
			and data portion of the packet. This field is valid for 
			all transmit packets
			0: disable per-chain csd
			1: enable per-chain csd 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CHAIN_CSD_EN_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CHAIN_CSD_EN_LSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CHAIN_CSD_EN_MSB 24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_CHAIN_CSD_EN_MASK 0x0000000001000000


/* Description		DOT11AX_PE_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the packet extension
			 portion of the packet. This field is valid for all 11ax
			 packets.
			0: disable per-chain csd
			1: enable per-chain csd
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CHAIN_CSD_EN_LSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CHAIN_CSD_EN_MSB 25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x0000000002000000


/* Description		DOT11AX_DL_UL_FLAG

			field is only valid for pkt_type == 11ax 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DL_UL_FLAG_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DL_UL_FLAG_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DL_UL_FLAG_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_DL_UL_FLAG_MASK 0x0000000004000000


/* Description		RESERVED_4A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_4A_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_4A_LSB    27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_4A_MSB    31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_4A_MASK   0x00000000f8000000


/* Description		DOT11AX_EXT_RU_START_INDEX

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1
			
			RU Number to which User is assigned
			
			The RU numbering bitwidth  is only enough to cover the 20MHz
			 BW that extended range allows
			<legal 0-8>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_START_INDEX_LSB 32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_START_INDEX_MSB 35
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_START_INDEX_MASK 0x0000000f00000000


/* Description		DOT11AX_EXT_RU_SIZE

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1 or pkt_type == 11be and EHT_duplicate_mode == 1
			
			The size of the RU for this user.
			
			In case of EHT duplicate transmissions, this field indicates
			 the width of the actual content before duplication, e.g. 
			a 40 MHz PPDU duplicated to 160 MHz will have the bandwidth
			 fields indicating 160 MHz and this field set to e-num 4
			 (RU_484).
			
			<enum 0 RU_26> 
			<enum 1 RU_52>
			<enum 2 RU_106>
			<enum 3 RU_242>
			<enum 4 RU_484>
			<enum 5 RU_996>
			<enum 6 RU_1992>
			<enum 7 RU_FULLBW> Set when the RU occupies the full packet
			 bandwidth
			<enum 8 RU_FULLBW_240> Set when the RU occupies the full
			 packet bandwidth
			<enum 9 RU_FULLBW_320> Set when the RU occupies the full
			 packet bandwidth
			<enum 10 RU_MULTI_LARGE> DO NOT USE
			<enum 11 RU_78> DO NOT USE
			<enum 12 RU_132> DO NOT USE
			<legal 0-12>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_SIZE_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_SIZE_LSB 36
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_SIZE_MSB 39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11AX_EXT_RU_SIZE_MASK 0x000000f000000000


/* Description		EHT_DUPLICATE_MODE

			Field only valid for pkt_type == 11be
			
			Indicates EHT duplicate modulation
			
			<enum 0 eht_no_duplicate>
			<enum 1 eht_2x_duplicate>
			<enum 2 eht_4x_duplicate>
			
			<legal 0-2>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_EHT_DUPLICATE_MODE_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_EHT_DUPLICATE_MODE_LSB 40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_EHT_DUPLICATE_MODE_MSB 41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_EHT_DUPLICATE_MODE_MASK 0x0000030000000000


/* Description		HE_SIGB_DCM

			Indicates whether dual sub-carrier modulation is applied
			 to EHT-SIG
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_DCM_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_DCM_LSB    42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_DCM_MSB    42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_DCM_MASK   0x0000040000000000


/* Description		HE_SIGB_0_MCS

			Indicates the MCS of EHT-SIG
			
			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_0_MCS_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_0_MCS_LSB  43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_0_MCS_MSB  45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_HE_SIGB_0_MCS_MASK 0x0000380000000000


/* Description		NUM_HE_SIGB_SYM

			Indicates the number of EHT-SIG symbols
			
			This field is 0-based with 0 indicating that 1 eht_sig symbol
			 needs to be transmitted.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NUM_HE_SIGB_SYM_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NUM_HE_SIGB_SYM_LSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NUM_HE_SIGB_SYM_MSB 50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_NUM_HE_SIGB_SYM_MASK 0x0007c00000000000


/* Description		REQUIRED_RESPONSE_TIME_SOURCE

			<enum 0 reqd_resp_time_src_is_RXPCU> Typically from received
			 HT Control for sync MLO response
			<enum 1 reqd_resp_time_src_is_FW>
			Typically from 'PCU_PPDU_SETUP_INIT' for sync MLO response
			 to response
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_SOURCE_LSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_SOURCE_MSB 51
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x0008000000000000


/* Description		RESERVED_5A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_5A_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_5A_LSB    52
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_5A_MSB    57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_RESERVED_5A_MASK   0x03f0000000000000


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			6-bit value copied from 'RX_RESPONSE_REQUIRED_INFO' and 'TX_CBF_INFO' 
			to pass on to PDG
			<legal 0-29>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x0000000000000090
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0xfc00000000000000


/* Description		MLO_STA_ID_DETAILS_RX

			16-bi value copied from 'RX_RESPONSE_REQUIRED_INFO' to pass
			 on to PDG
			
			Bits 10 and 11 are not valid, bits [9:0] reflect 'NSTR_MLO_STA_ID' 
			from address search.
			
			See definition of mlo_sta_id_details.
*/


/* Description		NSTR_MLO_STA_ID

			ID of peer participating in non-STR MLO
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x00000000000003ff


/* Description		BLOCK_SELF_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for self-link.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000000000000400


/* Description		BLOCK_PARTNER_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for partner links.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000000000000800


/* Description		NSTR_MLO_STA_ID_VALID

			All the fields in this TLV are valid only if this bit is
			 set.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 12
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000000000001000


/* Description		RESERVED_0A

			<legal 0>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x000000000000e000


/* Description		REQUIRED_RESPONSE_TIME

			When non-zero, indicates that PDG shall pad the response
			 transmission to the indicated duration (in us)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_LSB 16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_MSB 27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_REQUIRED_RESPONSE_TIME_MASK 0x000000000fff0000


/* Description		DOT11BE_PARAMS_PLACEHOLDER

			4 bytes for use as placeholders for 'Dot11be_*' parameters
			
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11BE_PARAMS_PLACEHOLDER_LSB 28
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11BE_PARAMS_PLACEHOLDER_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW240_DOT11BE_PARAMS_PLACEHOLDER_MASK 0x00000000f0000000


/* Description		RESPONSE_TO_RESPONSE_RATE_INFO_BW320

			Field only valid in case of Response_to_response set to 
			SU_BA or MU_BA
			
			NOTE: This field is also known as response_to_response_rate_info_pattern_5
			 in case punctured transmission is enabled.
			
			Used by TXPCU to determine what the transmit rates are for
			 the response to response transmission in case original 
			transmission was 320 MHz.
			
			Note: 
			see field R2R_bw320_active_channel for the BW of this transmission
			
*/


/* Description		RESERVED_0A

			 
			<legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_0A_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_0A_LSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_0A_MSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_0A_MASK   0x0000000100000000


/* Description		TX_ANTENNA_SECTOR_CTRL

			Sectored transmit antenna
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_ANTENNA_SECTOR_CTRL_OFFSET 0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_ANTENNA_SECTOR_CTRL_LSB 33
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_ANTENNA_SECTOR_CTRL_MSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_ANTENNA_SECTOR_CTRL_MASK 0x01fffffe00000000


/* Description		PKT_TYPE

			Packet type:
			<enum 0 dot11a>802.11a PPDU type
			<enum 1 dot11b>802.11b PPDU type
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			<enum 3 dot11ac>802.11ac PPDU type
			<enum 4 dot11ax>802.11ax PPDU type
			<enum 5 dot11ba>802.11ba (WUR) PPDU type
			<enum 6 dot11be>802.11be PPDU type
			<enum 7 dot11az>802.11az (ranging) PPDU type
			<enum 8 dot11n_gf>802.11n Green Field PPDU type (unsupported
			 & aborted)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_PKT_TYPE_OFFSET    0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_PKT_TYPE_LSB       57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_PKT_TYPE_MSB       60
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_PKT_TYPE_MASK      0x1e00000000000000


/* Description		SMOOTHING

			This field is used by PDG to populate the SMOOTHING filed
			 in the SIG Preamble of the PPDU
			<legal 0-1>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SMOOTHING_OFFSET   0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SMOOTHING_LSB      61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SMOOTHING_MSB      61
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SMOOTHING_MASK     0x2000000000000000


/* Description		LDPC

			When set, use LDPC transmission rates
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_LDPC_OFFSET        0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_LDPC_LSB           62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_LDPC_MSB           62
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_LDPC_MASK          0x4000000000000000


/* Description		STBC

			When set, use STBC transmission rates
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STBC_OFFSET        0x0000000000000098
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STBC_LSB           63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STBC_MSB           63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STBC_MASK          0x8000000000000000


/* Description		ALT_TX_PWR

			Coex related AlternativeTransmit parameter 
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_OFFSET  0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_LSB     0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_MSB     7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_MASK    0x00000000000000ff


/* Description		ALT_MIN_TX_PWR

			Coex related Alternative Transmit parameter 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_MIN_TX_PWR_OFFSET 0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_MIN_TX_PWR_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_MIN_TX_PWR_MSB 15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_MIN_TX_PWR_MASK 0x000000000000ff00


/* Description		ALT_NSS

			Coex related Alternative Transmit parameter
			
			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_NSS_OFFSET     0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_NSS_LSB        16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_NSS_MSB        18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_NSS_MASK       0x0000000000070000


/* Description		ALT_TX_CHAIN_MASK

			Coex related Alternative Transmit parameter
			
			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_CHAIN_MASK_OFFSET 0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_CHAIN_MASK_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_CHAIN_MASK_MSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_CHAIN_MASK_MASK 0x0000000007f80000


/* Description		ALT_BW

			Coex related Alternative Transmit parameter
			
			The BW of the upcoming transmission.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_BW_OFFSET      0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_BW_LSB         27
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_BW_MSB         29
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_BW_MASK        0x0000000038000000


/* Description		STF_LTF_3DB_BOOST

			Boost the STF and LTF power by 3dB in 11a/n/ac packets. 
			This includes both the legacy preambles and the HT/VHT preambles.0: 
			disable power boost1: enable power boost
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STF_LTF_3DB_BOOST_OFFSET 0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STF_LTF_3DB_BOOST_LSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STF_LTF_3DB_BOOST_MSB 30
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_STF_LTF_3DB_BOOST_MASK 0x0000000040000000


/* Description		FORCE_EXTRA_SYMBOL

			Set to 1 to force an extra OFDM symbol (or symbols) even
			 if the PPDU encoding process does not result in an extra
			 OFDM symbol (or symbols)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_FORCE_EXTRA_SYMBOL_OFFSET 0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_FORCE_EXTRA_SYMBOL_LSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_FORCE_EXTRA_SYMBOL_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_FORCE_EXTRA_SYMBOL_MASK 0x0000000080000000


/* Description		ALT_RATE_MCS

			Coex related Alternative Transmit parameter
			
			For details, refer to  MCS_TYPE 
			Note: This is "rate" in case of 11a/11b
			description
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_RATE_MCS_OFFSET 0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_RATE_MCS_LSB   32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_RATE_MCS_MSB   35
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_RATE_MCS_MASK  0x0000000f00000000


/* Description		NSS

			Number of spatial streams.
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NSS_OFFSET         0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NSS_LSB            36
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NSS_MSB            38
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NSS_MASK           0x0000007000000000


/* Description		DPD_ENABLE

			DPD enable control
			
			This is needed on a per packet basis
			<enum 0     dpd_off> DPD profile not applied to current 
			packet
			<enum 1     dpd_on> DPD profile applied to current packet
			 if available
			<legal 0-1>
			
			This field is not applicable in11ah mode of operation and
			 is ignored by the HW 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DPD_ENABLE_OFFSET  0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DPD_ENABLE_LSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DPD_ENABLE_MSB     39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DPD_ENABLE_MASK    0x0000008000000000


/* Description		TX_PWR

			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_OFFSET      0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_LSB         40
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_MSB         47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_MASK        0x0000ff0000000000


/* Description		MIN_TX_PWR

			Coex related field: 
			
			Minimum allowed Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MIN_TX_PWR_OFFSET  0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MIN_TX_PWR_LSB     48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MIN_TX_PWR_MSB     55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MIN_TX_PWR_MASK    0x00ff000000000000


/* Description		TX_CHAIN_MASK

			Chain mask to support up to 8 antennas.  
			<legal 1-255>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_CHAIN_MASK_OFFSET 0x00000000000000a0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_CHAIN_MASK_LSB  56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_CHAIN_MASK_MSB  63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_CHAIN_MASK_MASK 0xff00000000000000


/* Description		RESERVED_3A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3A_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3A_LSB    0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3A_MSB    7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3A_MASK   0x00000000000000ff


/* Description		SGI

			Field only valid when pkt type is HT or VHT.For 11ax see
			 field Dot11ax_CP_LTF_size
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > Not used for pre 11ax pkt_types. 
			
			<enum 3     3_2_us_sgi > Not used for pre 11ax pkt_types
			
			
			<legal 0 - 3>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SGI_OFFSET         0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SGI_LSB            8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SGI_MSB            9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_SGI_MASK           0x0000000000000300


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RATE_MCS_OFFSET    0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RATE_MCS_LSB       10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RATE_MCS_MSB       13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RATE_MCS_MASK      0x0000000000003c00


/* Description		RESERVED_3B

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3B_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3B_LSB    14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3B_MSB    15
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_3B_MASK   0x000000000000c000


/* Description		TX_PWR_1

			Default (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_1_OFFSET    0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_1_LSB       16
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_1_MSB       23
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_TX_PWR_1_MASK      0x0000000000ff0000


/* Description		ALT_TX_PWR_1

			Alternate (desired) transmit parameter for the second chain
			
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			
			Note that there is no Min value for this
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_1_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_1_LSB   24
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_1_MSB   31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_ALT_TX_PWR_1_MASK  0x00000000ff000000


/* Description		AGGREGATION

			Field only valid in case of pkt_type == 11n
			
			<enum 0     mpdu> Indicates MPDU format. TXPCU will select
			 this setting if the CBF response only contains a single
			 segment
			<enum 1     a_mpdu> Indicates A-MPDU format. TXPCU will 
			select this setting if the CBF response will contain two
			 or more segments
			<legal 0-1>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_AGGREGATION_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_AGGREGATION_LSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_AGGREGATION_MSB    32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_AGGREGATION_MASK   0x0000000100000000


/* Description		DOT11AX_BSS_COLOR_ID

			BSS color of the nextwork to which this STA belongs. 
			When generated by TXPCU, this field is set equal to: Dot11ax_received_Bss_color_id
			
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_BSS_COLOR_ID_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_BSS_COLOR_ID_LSB 33
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_BSS_COLOR_ID_MSB 38
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_BSS_COLOR_ID_MASK 0x0000007e00000000


/* Description		DOT11AX_SPATIAL_REUSE

			This field is only valid for pkt_type == 11ax
			
			Spatial re-use
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SPATIAL_REUSE_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SPATIAL_REUSE_LSB 39
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SPATIAL_REUSE_MSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SPATIAL_REUSE_MASK 0x0000078000000000


/* Description		DOT11AX_CP_LTF_SIZE

			field is only valid for pkt_type == 11ax
			
			Indicates the CP and HE-LTF type 
			
			<enum 0 OneX_LTF_0_8CP> 1xLTF + 0.8 us CP
			<enum 1 TwoX_LTF_0_8CP> 2x LTF + 0.8 µs CP
			<enum 2 TwoX_LTF_1_6CP> 2x LTF + 1.6 µs CP
			<enum 3 FourX_LTF_0_8CP_3_2CP>
			When DCM == 0 OR STBC == 0: 4x LTF + 3.2 µs CP 
			When DCM == 1 AND STBC == 1: 4x LTF + 0.8 µs CP. Note: 
			In this scenario, Neither DCM nor STBC is applied to HE 
			data field.
			
			If ( DCM == 1 ) and ( MCS > 0 ) and (STBC == 0) 
			0      = 1xLTF + 0.4 usec
			1      = 2xLTF + 0.4 usec
			2~3 = Reserved
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CP_LTF_SIZE_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CP_LTF_SIZE_LSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CP_LTF_SIZE_MSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CP_LTF_SIZE_MASK 0x0000180000000000


/* Description		DOT11AX_DCM

			field is only valid for pkt_type == 11ax
			
			Indicates whether dual sub-carrier modulation is applied
			
			0: No DCM
			1:DCM
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DCM_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DCM_LSB    45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DCM_MSB    45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DCM_MASK   0x0000200000000000


/* Description		DOT11AX_DOPPLER_INDICATION

			field is only valid for pkt_type == 11ax
			
			0: No Doppler support
			1: Doppler support
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DOPPLER_INDICATION_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DOPPLER_INDICATION_LSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DOPPLER_INDICATION_MSB 46
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DOPPLER_INDICATION_MASK 0x0000400000000000


/* Description		DOT11AX_SU_EXTENDED

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			When set, the 11ax or 11be frame is of the extended range
			 format
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SU_EXTENDED_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SU_EXTENDED_LSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SU_EXTENDED_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_SU_EXTENDED_MASK 0x0000800000000000


/* Description		DOT11AX_MIN_PACKET_EXTENSION

			field is only valid for pkt_type == 11ax OR pkt_type == 
			11be
			
			The min packet extension duration for this user.
			0: no extension
			1: 8us
			2: 16 us
			3: 20 us (only for .11be)
			<legal 0-3>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_MIN_PACKET_EXTENSION_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_MIN_PACKET_EXTENSION_LSB 48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_MIN_PACKET_EXTENSION_MSB 49
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_MIN_PACKET_EXTENSION_MASK 0x0003000000000000


/* Description		DOT11AX_PE_NSS

			Number of active spatial streams during packet extension.
			
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_NSS_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_NSS_LSB 50
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_NSS_MSB 52
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_NSS_MASK 0x001c000000000000


/* Description		DOT11AX_PE_CONTENT

			Content of packet extension. Valid for all 11ax packets 
			having packet extension
			
			0-he_ltf, 1-last_data_symbol
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CONTENT_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CONTENT_LSB 53
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CONTENT_MSB 53
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CONTENT_MASK 0x0020000000000000


/* Description		DOT11AX_PE_LTF_SIZE

			LTF size to be used during packet extention. . This field
			 is valid for both FTM and non-FTM packets.
			0-1x
			1-2x (unsupported un HWK-1)
			2-4x (unsupported un HWK-1) 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_LTF_SIZE_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_LTF_SIZE_LSB 54
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_LTF_SIZE_MSB 55
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_LTF_SIZE_MASK 0x00c0000000000000


/* Description		DOT11AX_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the preamble 
			and data portion of the packet. This field is valid for 
			all transmit packets
			0: disable per-chain csd
			1: enable per-chain csd 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CHAIN_CSD_EN_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CHAIN_CSD_EN_LSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CHAIN_CSD_EN_MSB 56
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_CHAIN_CSD_EN_MASK 0x0100000000000000


/* Description		DOT11AX_PE_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the packet extension
			 portion of the packet. This field is valid for all 11ax
			 packets.
			0: disable per-chain csd
			1: enable per-chain csd
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CHAIN_CSD_EN_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CHAIN_CSD_EN_LSB 57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CHAIN_CSD_EN_MSB 57
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_PE_CHAIN_CSD_EN_MASK 0x0200000000000000


/* Description		DOT11AX_DL_UL_FLAG

			field is only valid for pkt_type == 11ax 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DL_UL_FLAG_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DL_UL_FLAG_LSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DL_UL_FLAG_MSB 58
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_DL_UL_FLAG_MASK 0x0400000000000000


/* Description		RESERVED_4A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_4A_OFFSET 0x00000000000000a8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_4A_LSB    59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_4A_MSB    63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_4A_MASK   0xf800000000000000


/* Description		DOT11AX_EXT_RU_START_INDEX

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1
			
			RU Number to which User is assigned
			
			The RU numbering bitwidth  is only enough to cover the 20MHz
			 BW that extended range allows
			<legal 0-8>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_START_INDEX_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_START_INDEX_LSB 0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_START_INDEX_MSB 3
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_START_INDEX_MASK 0x000000000000000f


/* Description		DOT11AX_EXT_RU_SIZE

			field is only valid for pkt_type == 11ax and  Dot11ax_su_extended
			 == 1 or pkt_type == 11be and EHT_duplicate_mode == 1
			
			The size of the RU for this user.
			
			In case of EHT duplicate transmissions, this field indicates
			 the width of the actual content before duplication, e.g. 
			a 40 MHz PPDU duplicated to 160 MHz will have the bandwidth
			 fields indicating 160 MHz and this field set to e-num 4
			 (RU_484).
			
			<enum 0 RU_26> 
			<enum 1 RU_52>
			<enum 2 RU_106>
			<enum 3 RU_242>
			<enum 4 RU_484>
			<enum 5 RU_996>
			<enum 6 RU_1992>
			<enum 7 RU_FULLBW> Set when the RU occupies the full packet
			 bandwidth
			<enum 8 RU_FULLBW_240> Set when the RU occupies the full
			 packet bandwidth
			<enum 9 RU_FULLBW_320> Set when the RU occupies the full
			 packet bandwidth
			<enum 10 RU_MULTI_LARGE> DO NOT USE
			<enum 11 RU_78> DO NOT USE
			<enum 12 RU_132> DO NOT USE
			<legal 0-12>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_SIZE_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_SIZE_LSB 4
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_SIZE_MSB 7
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11AX_EXT_RU_SIZE_MASK 0x00000000000000f0


/* Description		EHT_DUPLICATE_MODE

			Field only valid for pkt_type == 11be
			
			Indicates EHT duplicate modulation
			
			<enum 0 eht_no_duplicate>
			<enum 1 eht_2x_duplicate>
			<enum 2 eht_4x_duplicate>
			
			<legal 0-2>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_EHT_DUPLICATE_MODE_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_EHT_DUPLICATE_MODE_LSB 8
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_EHT_DUPLICATE_MODE_MSB 9
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_EHT_DUPLICATE_MODE_MASK 0x0000000000000300


/* Description		HE_SIGB_DCM

			Indicates whether dual sub-carrier modulation is applied
			 to EHT-SIG
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_DCM_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_DCM_LSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_DCM_MSB    10
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_DCM_MASK   0x0000000000000400


/* Description		HE_SIGB_0_MCS

			Indicates the MCS of EHT-SIG
			
			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_0_MCS_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_0_MCS_LSB  11
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_0_MCS_MSB  13
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_HE_SIGB_0_MCS_MASK 0x0000000000003800


/* Description		NUM_HE_SIGB_SYM

			Indicates the number of EHT-SIG symbols
			
			This field is 0-based with 0 indicating that 1 eht_sig symbol
			 needs to be transmitted.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NUM_HE_SIGB_SYM_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NUM_HE_SIGB_SYM_LSB 14
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NUM_HE_SIGB_SYM_MSB 18
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_NUM_HE_SIGB_SYM_MASK 0x000000000007c000


/* Description		REQUIRED_RESPONSE_TIME_SOURCE

			<enum 0 reqd_resp_time_src_is_RXPCU> Typically from received
			 HT Control for sync MLO response
			<enum 1 reqd_resp_time_src_is_FW>
			Typically from 'PCU_PPDU_SETUP_INIT' for sync MLO response
			 to response
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_SOURCE_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_SOURCE_LSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_SOURCE_MSB 19
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_SOURCE_MASK 0x0000000000080000


/* Description		RESERVED_5A

			 <legal 0> 
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_5A_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_5A_LSB    20
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_5A_MSB    25
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_RESERVED_5A_MASK   0x0000000003f00000


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			6-bit value copied from 'RX_RESPONSE_REQUIRED_INFO' and 'TX_CBF_INFO' 
			to pass on to PDG
			<legal 0-29>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB 26
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB 31
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK 0x00000000fc000000


/* Description		MLO_STA_ID_DETAILS_RX

			16-bi value copied from 'RX_RESPONSE_REQUIRED_INFO' to pass
			 on to PDG
			
			Bits 10 and 11 are not valid, bits [9:0] reflect 'NSTR_MLO_STA_ID' 
			from address search.
			
			See definition of mlo_sta_id_details.
*/


/* Description		NSTR_MLO_STA_ID

			ID of peer participating in non-STR MLO
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB 32
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB 41
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK 0x000003ff00000000


/* Description		BLOCK_SELF_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for self-link.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB 42
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK 0x0000040000000000


/* Description		BLOCK_PARTNER_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for partner links.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB 43
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK 0x0000080000000000


/* Description		NSTR_MLO_STA_ID_VALID

			All the fields in this TLV are valid only if this bit is
			 set.
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB 44
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK 0x0000100000000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB 45
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB 47
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK 0x0000e00000000000


/* Description		REQUIRED_RESPONSE_TIME

			When non-zero, indicates that PDG shall pad the response
			 transmission to the indicated duration (in us)
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_LSB 48
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_MSB 59
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_REQUIRED_RESPONSE_TIME_MASK 0x0fff000000000000


/* Description		DOT11BE_PARAMS_PLACEHOLDER

			4 bytes for use as placeholders for 'Dot11be_*' parameters
			
*/

#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11BE_PARAMS_PLACEHOLDER_OFFSET 0x00000000000000b0
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11BE_PARAMS_PLACEHOLDER_LSB 60
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11BE_PARAMS_PLACEHOLDER_MSB 63
#define PCU_PPDU_SETUP_INIT_RESPONSE_TO_RESPONSE_RATE_INFO_BW320_DOT11BE_PARAMS_PLACEHOLDER_MASK 0xf000000000000000


/* Description		R2R_HW_RESPONSE_TX_DURATION

			Field only valid in case of Response_to_response set to 
			SU_BA or MU_BA
			
			The amount of time the transmission of the HW response to
			 response will take (in us)
			
			Used for coex as well as e.g. for sync MLO to align R2R 
			times on the medium across multiple channels
			
			This field also represents the 'alt_hw_response_tx_duration'. 
			Note that this implies that no different duration can be
			 programmed for the default and alt setting. SW should program
			 the worst case value in the RXPCU table in case they are
			 different.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_R2R_HW_RESPONSE_TX_DURATION_OFFSET                      0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_HW_RESPONSE_TX_DURATION_LSB                         0
#define PCU_PPDU_SETUP_INIT_R2R_HW_RESPONSE_TX_DURATION_MSB                         15
#define PCU_PPDU_SETUP_INIT_R2R_HW_RESPONSE_TX_DURATION_MASK                        0x000000000000ffff


/* Description		R2R_RX_DURATION_FIELD

			Field only valid in case of Response_to_response set to 
			SU_BA or MU_BA
			
			The duration field assumed to have been received in the 
			response frame and what will be used in the duration field
			 calculation for the response_to_response_Frame
			
			PDG uses this field to calculate what the duration field
			 value should be in the response frame.
			This is returned to the TXPCU
			
			Note that if PDG has protection in place to wrap around... 
			I the actual transmit time is larger then the value programmed
			 here, PDG HW will set the duration field in the response
			 to response frame to zero.
			
			This field is used in 11ah mode as well
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_R2R_RX_DURATION_FIELD_OFFSET                            0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_RX_DURATION_FIELD_LSB                               16
#define PCU_PPDU_SETUP_INIT_R2R_RX_DURATION_FIELD_MSB                               31
#define PCU_PPDU_SETUP_INIT_R2R_RX_DURATION_FIELD_MASK                              0x00000000ffff0000


/* Description		R2R_GROUP_ID

			Field only valid in case of Response_to_response set to 
			SU_BA or MU_BA
			
			Specifies the Group ID to be used in the response to  response
			 frame.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_R2R_GROUP_ID_OFFSET                                     0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_GROUP_ID_LSB                                        32
#define PCU_PPDU_SETUP_INIT_R2R_GROUP_ID_MSB                                        37
#define PCU_PPDU_SETUP_INIT_R2R_GROUP_ID_MASK                                       0x0000003f00000000


/* Description		R2R_RESPONSE_FRAME_TYPE

			Field only valid in case of Response_to_response set to 
			SU_BA or MU_BA
			
			Response_frame_type to be indicated in the PDG_RESPONSE 
			TLV for the response to response frame.
			
			Coex related field
			<enum 0 Non_11ah_ACK >  
			<enum 1 Non_11ah_BA >  also used for M-BA
			<enum 2 Non_11ah_CTS > 
			<enum 3 AH_NDP_CTS> 
			<enum 4 AH_NDP_ACK>
			<enum 5 AH_NDP_BA>
			<enum 6 AH_NDP_MOD_ACK>
			<enum 7 AH_Normal_ACK>
			<enum 8 AH_Normal_BA>
			<enum 9  RTT_ACK>
			<enum 10 CBF_RESPONSE>
			<enum 11 MBA> This can be a multi STA BA or multi TID BA
			
			<enum 12 Ranging_NDP>
			<enum 13 LMR_RESPONSE> NDP followed by LMR response for 
			Rx ranging NDPA followed by NDP
			
			<legal 0-12>
*/

#define PCU_PPDU_SETUP_INIT_R2R_RESPONSE_FRAME_TYPE_OFFSET                          0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_RESPONSE_FRAME_TYPE_LSB                             38
#define PCU_PPDU_SETUP_INIT_R2R_RESPONSE_FRAME_TYPE_MSB                             41
#define PCU_PPDU_SETUP_INIT_R2R_RESPONSE_FRAME_TYPE_MASK                            0x000003c000000000


/* Description		R2R_STA_PARTIAL_AID

			Field only valid in case of Response_to_response set to 
			SU_BA or MU_BA
			
			Specifies the partial AID of the response to response frame
			 in case it is transmitted at VHT rates.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_R2R_STA_PARTIAL_AID_OFFSET                              0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_STA_PARTIAL_AID_LSB                                 42
#define PCU_PPDU_SETUP_INIT_R2R_STA_PARTIAL_AID_MSB                                 52
#define PCU_PPDU_SETUP_INIT_R2R_STA_PARTIAL_AID_MASK                                0x001ffc0000000000


/* Description		USE_ADDRESS_FIELDS_FOR_PROTECTION

			When set, the protection_frame_ad1/ad2 fields are to be 
			used for RTS/CTS2S frames
			
			When set and not disabled through a TXPCU register bit, 
			the protection_frame_ad2* fields are also copied to the 
			tx_ad2* fields of the 'EXPECTED_RESPONSE' TLV (i.e. the 
			expected response Rx AD1) to RXPCU for all frames.
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_USE_ADDRESS_FIELDS_FOR_PROTECTION_OFFSET                0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_USE_ADDRESS_FIELDS_FOR_PROTECTION_LSB                   53
#define PCU_PPDU_SETUP_INIT_USE_ADDRESS_FIELDS_FOR_PROTECTION_MSB                   53
#define PCU_PPDU_SETUP_INIT_USE_ADDRESS_FIELDS_FOR_PROTECTION_MASK                  0x0020000000000000


/* Description		R2R_SET_REQUIRED_RESPONSE_TIME

			Field only valid in case of response to response
			
			When set, TXPCU shall copy the R2R_Hw_response_tx_duration
			 field and pass it on to PDG in field required_response_time
			 in 'PDG_RESPONSE.'
			
			This allows SW to force an R2R time e.g. in case of sync
			 MLO, making sure that the R2R times on the medium for multiple
			 links are aligned.
			 
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_R2R_SET_REQUIRED_RESPONSE_TIME_OFFSET                   0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_SET_REQUIRED_RESPONSE_TIME_LSB                      54
#define PCU_PPDU_SETUP_INIT_R2R_SET_REQUIRED_RESPONSE_TIME_MSB                      54
#define PCU_PPDU_SETUP_INIT_R2R_SET_REQUIRED_RESPONSE_TIME_MASK                     0x0040000000000000


/* Description		RESERVED_29A

			<legal 0>
*/

#define PCU_PPDU_SETUP_INIT_RESERVED_29A_OFFSET                                     0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_RESERVED_29A_LSB                                        55
#define PCU_PPDU_SETUP_INIT_RESERVED_29A_MSB                                        57
#define PCU_PPDU_SETUP_INIT_RESERVED_29A_MASK                                       0x0380000000000000


/* Description		R2R_BW20_ACTIVE_CHANNEL

			Field only valid for 20 BW
			
			NOTE: This field is also known as R2R_active_channel_pattern_0
			 in case punctured transmission is enabled.
			
			This field indicates the active frequency band when the 
			initial trigger frame transmission was in 20 MHz
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_R2R_BW20_ACTIVE_CHANNEL_OFFSET                          0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_BW20_ACTIVE_CHANNEL_LSB                             58
#define PCU_PPDU_SETUP_INIT_R2R_BW20_ACTIVE_CHANNEL_MSB                             60
#define PCU_PPDU_SETUP_INIT_R2R_BW20_ACTIVE_CHANNEL_MASK                            0x1c00000000000000


/* Description		R2R_BW40_ACTIVE_CHANNEL

			Field only valid for 40 BW
			
			NOTE: This field is also known as R2R_active_channel_pattern_1
			 in case punctured transmission is enabled.
			
			This field indicates the active frequency band when the 
			initial trigger frame transmission was in 40 MHz
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_R2R_BW40_ACTIVE_CHANNEL_OFFSET                          0x00000000000000b8
#define PCU_PPDU_SETUP_INIT_R2R_BW40_ACTIVE_CHANNEL_LSB                             61
#define PCU_PPDU_SETUP_INIT_R2R_BW40_ACTIVE_CHANNEL_MSB                             63
#define PCU_PPDU_SETUP_INIT_R2R_BW40_ACTIVE_CHANNEL_MASK                            0xe000000000000000


/* Description		R2R_BW80_ACTIVE_CHANNEL

			Field only valid for 80 BW
			
			NOTE: This field is also known as R2R_active_channel_pattern_2
			 in case punctured transmission is enabled.
			
			This field indicates the active frequency band when the 
			initial trigger frame transmission was in 80 MHz
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_R2R_BW80_ACTIVE_CHANNEL_OFFSET                          0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW80_ACTIVE_CHANNEL_LSB                             0
#define PCU_PPDU_SETUP_INIT_R2R_BW80_ACTIVE_CHANNEL_MSB                             2
#define PCU_PPDU_SETUP_INIT_R2R_BW80_ACTIVE_CHANNEL_MASK                            0x0000000000000007


/* Description		R2R_BW160_ACTIVE_CHANNEL

			Field only valid for 160 BW
			
			NOTE: This field is also known as R2R_active_channel_pattern_3
			 in case punctured transmission is enabled.
			
			This field indicates the active frequency band when the 
			initial trigger frame transmission was in 160 MHz
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_R2R_BW160_ACTIVE_CHANNEL_OFFSET                         0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW160_ACTIVE_CHANNEL_LSB                            3
#define PCU_PPDU_SETUP_INIT_R2R_BW160_ACTIVE_CHANNEL_MSB                            5
#define PCU_PPDU_SETUP_INIT_R2R_BW160_ACTIVE_CHANNEL_MASK                           0x0000000000000038


/* Description		R2R_BW240_ACTIVE_CHANNEL

			Field only valid for 240 BW
			
			NOTE: This field is also known as R2R_active_channel_pattern_4
			 in case punctured transmission is enabled.
			
			This field indicates the active frequency band when the 
			initial trigger frame transmission was in 240 MHz
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_R2R_BW240_ACTIVE_CHANNEL_OFFSET                         0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW240_ACTIVE_CHANNEL_LSB                            6
#define PCU_PPDU_SETUP_INIT_R2R_BW240_ACTIVE_CHANNEL_MSB                            8
#define PCU_PPDU_SETUP_INIT_R2R_BW240_ACTIVE_CHANNEL_MASK                           0x00000000000001c0


/* Description		R2R_BW320_ACTIVE_CHANNEL

			Field only valid for 320 BW
			
			NOTE: This field is also known as R2R_active_channel_pattern_5
			 in case punctured transmission is enabled.
			
			This field indicates the active frequency band when the 
			initial trigger frame transmission was in 320 MHz
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_R2R_BW320_ACTIVE_CHANNEL_OFFSET                         0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW320_ACTIVE_CHANNEL_LSB                            9
#define PCU_PPDU_SETUP_INIT_R2R_BW320_ACTIVE_CHANNEL_MSB                            11
#define PCU_PPDU_SETUP_INIT_R2R_BW320_ACTIVE_CHANNEL_MASK                           0x0000000000000e00


/* Description		R2R_BW20

			The BW for the response to response frame when the initial
			 trigger frame transmission was in 20 MHz 
			
			NOTE: This field is also known as R2R_pattern_0 in case 
			punctured transmission is enabled.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PCU_PPDU_SETUP_INIT_R2R_BW20_OFFSET                                         0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW20_LSB                                            12
#define PCU_PPDU_SETUP_INIT_R2R_BW20_MSB                                            14
#define PCU_PPDU_SETUP_INIT_R2R_BW20_MASK                                           0x0000000000007000


/* Description		R2R_BW40

			The BW for the response to response frame when the initial
			 trigger frame transmission was in 40 MHz 
			
			NOTE: This field is also known as R2R_pattern_1 in case 
			punctured transmission is enabled.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PCU_PPDU_SETUP_INIT_R2R_BW40_OFFSET                                         0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW40_LSB                                            15
#define PCU_PPDU_SETUP_INIT_R2R_BW40_MSB                                            17
#define PCU_PPDU_SETUP_INIT_R2R_BW40_MASK                                           0x0000000000038000


/* Description		R2R_BW80

			The BW for the response to response frame when the initial
			 trigger frame transmission was in 80 MHz 
			
			NOTE: This field is also known as R2R_pattern_2 in case 
			punctured transmission is enabled.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PCU_PPDU_SETUP_INIT_R2R_BW80_OFFSET                                         0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW80_LSB                                            18
#define PCU_PPDU_SETUP_INIT_R2R_BW80_MSB                                            20
#define PCU_PPDU_SETUP_INIT_R2R_BW80_MASK                                           0x00000000001c0000


/* Description		R2R_BW160

			The BW for the response to response frame when the initial
			 trigger frame transmission was in 160 MHz 
			
			NOTE: This field is also known as R2R_pattern_3 in case 
			punctured transmission is enabled.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PCU_PPDU_SETUP_INIT_R2R_BW160_OFFSET                                        0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW160_LSB                                           21
#define PCU_PPDU_SETUP_INIT_R2R_BW160_MSB                                           23
#define PCU_PPDU_SETUP_INIT_R2R_BW160_MASK                                          0x0000000000e00000


/* Description		R2R_BW240

			The BW for the response to response frame when the initial
			 trigger frame transmission was in 240 MHz 
			
			NOTE: This field is also known as R2R_pattern_4 in case 
			punctured transmission is enabled.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PCU_PPDU_SETUP_INIT_R2R_BW240_OFFSET                                        0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW240_LSB                                           24
#define PCU_PPDU_SETUP_INIT_R2R_BW240_MSB                                           26
#define PCU_PPDU_SETUP_INIT_R2R_BW240_MASK                                          0x0000000007000000


/* Description		R2R_BW320

			The BW for the response to response frame when the initial
			 trigger frame transmission was in 320 MHz 
			
			NOTE: This field is also known as R2R_pattern_5 in case 
			punctured transmission is enabled.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PCU_PPDU_SETUP_INIT_R2R_BW320_OFFSET                                        0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_R2R_BW320_LSB                                           27
#define PCU_PPDU_SETUP_INIT_R2R_BW320_MSB                                           29
#define PCU_PPDU_SETUP_INIT_R2R_BW320_MASK                                          0x0000000038000000


/* Description		RESERVED_30A

			<legal 0>
*/

#define PCU_PPDU_SETUP_INIT_RESERVED_30A_OFFSET                                     0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_RESERVED_30A_LSB                                        30
#define PCU_PPDU_SETUP_INIT_RESERVED_30A_MSB                                        31
#define PCU_PPDU_SETUP_INIT_RESERVED_30A_MASK                                       0x00000000c0000000


/* Description		MU_RESPONSE_EXPECTED_BITMAP_31_0

			Field only valid in case of MU transmission and a response
			 from other or more then just user0 is expected.
			
			Note that this implies that for all legacy SU exchanges, 
			or legacy MU-MIMO where only user 0 can get a response, 
			this field does not need to be programmed by SW. All existing
			 programming remains backwards compatible.
			
			Bit 0 represents user 0
			Bit 1 represents user 1
			...
			When set, a response from this user is expected, and TXPCU
			 shall generate the 'tx_fes_status_user_response' TLV for
			 this user
			
			Note that the number of bits set in bitmap fields 0 - 36
			 (including next field), shall always be equal or greater
			 then the number indicated in field: Required_UL_MU_resp_user_count
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_31_0_OFFSET                 0x00000000000000c0
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_31_0_LSB                    32
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_31_0_MSB                    63
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_31_0_MASK                   0xffffffff00000000


/* Description		MU_RESPONSE_EXPECTED_BITMAP_36_32

			Field only valid in case of MU transmission and a response
			 from other or more then just user0 is expected.
			
			Note that this implies that for all legacy SU exchanges, 
			or legacy MU-MIMO where only user 0 can get a response, 
			this field does not need to be programmed by SW. All existing
			 programming remains backwards compatible.
			
			Bit 0 represents user 32
			Bit 1 represents user 33
			...
			When set, a response from this user is expected, and TXPCU
			 shall generate the 'tx_fes_status_user_response' TLV for
			 this user
			
			Note that the number of bits set in bitmap fields 0 - 36
			 (including previous field), shall always be equal or greater
			 then the number indicated in field: Required_UL_MU_resp_user_count
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_36_32_OFFSET                0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_36_32_LSB                   0
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_36_32_MSB                   4
#define PCU_PPDU_SETUP_INIT_MU_RESPONSE_EXPECTED_BITMAP_36_32_MASK                  0x000000000000001f


/* Description		MU_EXPECTED_RESPONSE_CBF_COUNT

			Field only valid when Response_type == MU_CBF_expected
			
			The number of STAs that are expected to send a CBF back
			
			Note that the actual amount could be smaller....
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_CBF_COUNT_OFFSET                   0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_CBF_COUNT_LSB                      5
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_CBF_COUNT_MSB                      10
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_CBF_COUNT_MASK                     0x00000000000007e0


/* Description		MU_EXPECTED_RESPONSE_STA_COUNT

			SW shall program this field if the number of STAs that are
			 expected to send something (ACK, DATA, BA, CBF, etc...) 
			back is 2 or larger..
			
			The number of STAs that are expected to send a response 
			back.
			
			Note that the actual amount could be smaller....
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_STA_COUNT_OFFSET                   0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_STA_COUNT_LSB                      11
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_STA_COUNT_MSB                      16
#define PCU_PPDU_SETUP_INIT_MU_EXPECTED_RESPONSE_STA_COUNT_MASK                     0x000000000001f800


/* Description		TRANSMIT_INCLUDES_MULTIDESTINATION

			Used by TXPCU
			
			When set, the MD (Multi Destination) feature is used for
			 this transmission. Either for real multi destination STA
			 transmissions or Multi TID transmissions.
			
			Used by TXPCU to know when it can start pre-fetching data
			 in order to do BW constrained frame drops.
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_TRANSMIT_INCLUDES_MULTIDESTINATION_OFFSET               0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_TRANSMIT_INCLUDES_MULTIDESTINATION_LSB                  17
#define PCU_PPDU_SETUP_INIT_TRANSMIT_INCLUDES_MULTIDESTINATION_MSB                  17
#define PCU_PPDU_SETUP_INIT_TRANSMIT_INCLUDES_MULTIDESTINATION_MASK                 0x0000000000020000


/* Description		INSERT_PREV_TX_START_TIMING_INFO

			When set, TXPCU will insert the value in TXPCU register "prev_phy_tx_start_transmit_time" 
			in the transmit frame at the byte location indicated by 
			field tx_start_transmit_time_byte_offset
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_INSERT_PREV_TX_START_TIMING_INFO_OFFSET                 0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_INSERT_PREV_TX_START_TIMING_INFO_LSB                    18
#define PCU_PPDU_SETUP_INIT_INSERT_PREV_TX_START_TIMING_INFO_MSB                    18
#define PCU_PPDU_SETUP_INIT_INSERT_PREV_TX_START_TIMING_INFO_MASK                   0x0000000000040000


/* Description		INSERT_CURRENT_TX_START_TIMING_INFO

			When set, TXPCU will insert the value in TXPCU register "current_phy_tx_start_transmit_time" 
			in the transmit frame at the byte location indicated by 
			field tx_start_transmit_time_byte_offset
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_INSERT_CURRENT_TX_START_TIMING_INFO_OFFSET              0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_INSERT_CURRENT_TX_START_TIMING_INFO_LSB                 19
#define PCU_PPDU_SETUP_INIT_INSERT_CURRENT_TX_START_TIMING_INFO_MSB                 19
#define PCU_PPDU_SETUP_INIT_INSERT_CURRENT_TX_START_TIMING_INFO_MASK                0x0000000000080000


/* Description		TX_START_TRANSMIT_TIME_BYTE_OFFSET

			Field only valid when insert_prev_tx_start_timing_info or
			 insert_current_tx_start_timing_info is set.
			Start byte offset where the 'start_time' needs to be overwritten
			 in the frame
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_TX_START_TRANSMIT_TIME_BYTE_OFFSET_OFFSET               0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_TX_START_TRANSMIT_TIME_BYTE_OFFSET_LSB                  20
#define PCU_PPDU_SETUP_INIT_TX_START_TRANSMIT_TIME_BYTE_OFFSET_MSB                  31
#define PCU_PPDU_SETUP_INIT_TX_START_TRANSMIT_TIME_BYTE_OFFSET_MASK                 0x00000000fff00000


/* Description		PROTECTION_FRAME_AD1_31_0

			Field only valid when use_address_fields_for_protection 
			is set
			
			The Least Significant 4 bytes of the Protection Frame MAC
			 Address AD1
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_31_0_OFFSET                        0x00000000000000c8
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_31_0_LSB                           32
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_31_0_MSB                           63
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_31_0_MASK                          0xffffffff00000000


/* Description		PROTECTION_FRAME_AD1_47_32

			Field only valid when use_address_fields_for_protection 
			is set
			
			The 2 most significant bytes of the Protection Frame MAC
			 Address AD1
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_47_32_OFFSET                       0x00000000000000d0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_47_32_LSB                          0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_47_32_MSB                          15
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD1_47_32_MASK                         0x000000000000ffff


/* Description		PROTECTION_FRAME_AD2_15_0

			Field only valid when use_address_fields_for_protection 
			is set
			
			The Least Significant 2 bytes of the MAC Address AD2
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_15_0_OFFSET                        0x00000000000000d0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_15_0_LSB                           16
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_15_0_MSB                           31
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_15_0_MASK                          0x00000000ffff0000


/* Description		PROTECTION_FRAME_AD2_47_16

			Field only valid when use_address_fields_for_protection 
			is set
			
			The 4 most significant bytes of the MAC Address AD2
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_47_16_OFFSET                       0x00000000000000d0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_47_16_LSB                          32
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_47_16_MSB                          63
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD2_47_16_MASK                         0xffffffff00000000


/* Description		DYNAMIC_MEDIUM_PROT_THRESHOLD

			Threshold to enable the dynamic medium protection feature
			 in terms of PPDU duration in us or PSDU length in bytes
			
			
			This is set to zero to disable the dynamic medium protection
			 feature.
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_THRESHOLD_OFFSET                    0x00000000000000d8
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_THRESHOLD_LSB                       0
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_THRESHOLD_MSB                       23
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_THRESHOLD_MASK                      0x0000000000ffffff


/* Description		DYNAMIC_MEDIUM_PROT_TYPE

			<enum 0 dyn_medium_prot_byte> dynamic_medium_prot_threshold
			 indicates PSDU length in bytes.
			<enum 1 dyn_medium_prot_us>
			dynamic_medium_prot_threshold indicates PPDU duration in
			 us.
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_TYPE_OFFSET                         0x00000000000000d8
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_TYPE_LSB                            24
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_TYPE_MSB                            24
#define PCU_PPDU_SETUP_INIT_DYNAMIC_MEDIUM_PROT_TYPE_MASK                           0x0000000001000000


/* Description		RESERVED_54A

			<legal 0>
*/

#define PCU_PPDU_SETUP_INIT_RESERVED_54A_OFFSET                                     0x00000000000000d8
#define PCU_PPDU_SETUP_INIT_RESERVED_54A_LSB                                        25
#define PCU_PPDU_SETUP_INIT_RESERVED_54A_MSB                                        31
#define PCU_PPDU_SETUP_INIT_RESERVED_54A_MASK                                       0x00000000fe000000


/* Description		PROTECTION_FRAME_AD3_31_0

			Field only valid when use_address_fields_for_protection 
			is set
			
			The least significant 4 bytes of the Protection Frame MAC
			 Address AD3
			
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_31_0_OFFSET                        0x00000000000000d8
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_31_0_LSB                           32
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_31_0_MSB                           63
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_31_0_MASK                          0xffffffff00000000


/* Description		PROTECTION_FRAME_AD3_47_32

			Field only valid when use_address_fields_for_protection 
			is set
			
			The 2 most significant bytes of the Protection Frame MAC
			 Address AD3
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_47_32_OFFSET                       0x00000000000000e0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_47_32_LSB                          0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_47_32_MSB                          15
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD3_47_32_MASK                         0x000000000000ffff


/* Description		PROTECTION_FRAME_AD4_15_0

			Field only valid when use_address_fields_for_protection 
			is set
			
			The least significant 2 bytes of the Protection Frame MAC
			 Address AD4
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_15_0_OFFSET                        0x00000000000000e0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_15_0_LSB                           16
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_15_0_MSB                           31
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_15_0_MASK                          0x00000000ffff0000


/* Description		PROTECTION_FRAME_AD4_47_16

			Field only valid when use_address_fields_for_protection 
			is set
			
			The 4 most significant bytes of the Protection Frame MAC
			 Address AD4
			<legal all>
*/

#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_47_16_OFFSET                       0x00000000000000e0
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_47_16_LSB                          32
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_47_16_MSB                          63
#define PCU_PPDU_SETUP_INIT_PROTECTION_FRAME_AD4_47_16_MASK                         0xffffffff00000000



#endif   // PCU_PPDU_SETUP_INIT
