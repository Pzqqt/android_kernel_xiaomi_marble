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

#ifndef _RX_RESPONSE_REQUIRED_INFO_H_
#define _RX_RESPONSE_REQUIRED_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#include "mlo_sta_id_details.h"
#define NUM_OF_DWORDS_RX_RESPONSE_REQUIRED_INFO 16

#define NUM_OF_QWORDS_RX_RESPONSE_REQUIRED_INFO 8


struct rx_response_required_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t phy_ppdu_id                                             : 16, // [15:0]
                      su_or_uplink_mu_reception                               :  1, // [16:16]
                      trigger_frame_received                                  :  1, // [17:17]
                      ftm_tm                                                  :  2, // [19:18]
                      tb_ranging_response_required                            :  2, // [21:20]
                      mac_security                                            :  1, // [22:22]
                      filter_pass_monitor_ovrd                                :  1, // [23:23]
                      ast_search_incomplete                                   :  1, // [24:24]
                      r2r_end_status_to_follow                                :  1, // [25:25]
                      reserved_0a                                             :  2, // [27:26]
                      three_or_more_type_subtypes                             :  1, // [28:28]
                      wait_sifs_config_valid                                  :  1, // [29:29]
                      wait_sifs                                               :  2; // [31:30]
             uint32_t general_frame_control                                   : 16, // [15:0]
                      second_frame_control                                    : 16; // [31:16]
             uint32_t duration                                                : 16, // [15:0]
                      pkt_type                                                :  4, // [19:16]
                      dot11ax_su_extended                                     :  1, // [20:20]
                      rate_mcs                                                :  4, // [24:21]
                      sgi                                                     :  2, // [26:25]
                      stbc                                                    :  1, // [27:27]
                      ldpc                                                    :  1, // [28:28]
                      ampdu                                                   :  1, // [29:29]
                      vht_ack                                                 :  1, // [30:30]
                      rts_ta_grp_bit                                          :  1; // [31:31]
             uint32_t ctrl_frame_soliciting_resp                              :  1, // [0:0]
                      ast_fail_for_dot11ax_su_ext                             :  1, // [1:1]
                      service_dynamic                                         :  1, // [2:2]
                      m_pkt                                                   :  1, // [3:3]
                      sta_partial_aid                                         : 12, // [15:4]
                      group_id                                                :  6, // [21:16]
                      ctrl_resp_pwr_mgmt                                      :  1, // [22:22]
                      response_indication                                     :  2, // [24:23]
                      ndp_indication                                          :  1, // [25:25]
                      ndp_frame_type                                          :  3, // [28:26]
                      second_frame_control_valid                              :  1, // [29:29]
                      reserved_3a                                             :  2; // [31:30]
             uint32_t ack_id                                                  : 16, // [15:0]
                      ack_id_ext                                              : 10, // [25:16]
                      agc_cbw                                                 :  3, // [28:26]
                      service_cbw                                             :  3; // [31:29]
             uint32_t response_sta_count                                      :  7, // [6:0]
                      reserved                                                :  4, // [10:7]
                      ht_vht_sig_cbw                                          :  3, // [13:11]
                      cts_cbw                                                 :  3, // [16:14]
                      response_ack_count                                      :  7, // [23:17]
                      response_assoc_ack_count                                :  7, // [30:24]
                      txop_duration_all_ones                                  :  1; // [31:31]
             uint32_t response_ba32_count                                     :  7, // [6:0]
                      response_ba64_count                                     :  7, // [13:7]
                      response_ba128_count                                    :  7, // [20:14]
                      response_ba256_count                                    :  7, // [27:21]
                      multi_tid                                               :  1, // [28:28]
                      sw_response_tlv_from_crypto                             :  1, // [29:29]
                      dot11ax_dl_ul_flag                                      :  1, // [30:30]
                      reserved_6a                                             :  1; // [31:31]
             uint32_t sw_response_frame_length                                : 16, // [15:0]
                      response_ba512_count                                    :  7, // [22:16]
                      response_ba1024_count                                   :  7, // [29:23]
                      reserved_7a                                             :  2; // [31:30]
             uint32_t addr1_31_0                                              : 32; // [31:0]
             uint32_t addr1_47_32                                             : 16, // [15:0]
                      addr2_15_0                                              : 16; // [31:16]
             uint32_t addr2_47_16                                             : 32; // [31:0]
             uint32_t dot11ax_received_format_indication                      :  1, // [0:0]
                      dot11ax_received_dl_ul_flag                             :  1, // [1:1]
                      dot11ax_received_bss_color_id                           :  6, // [7:2]
                      dot11ax_received_spatial_reuse                          :  4, // [11:8]
                      dot11ax_received_cp_size                                :  2, // [13:12]
                      dot11ax_received_ltf_size                               :  2, // [15:14]
                      dot11ax_received_coding                                 :  1, // [16:16]
                      dot11ax_received_dcm                                    :  1, // [17:17]
                      dot11ax_received_doppler_indication                     :  1, // [18:18]
                      dot11ax_received_ext_ru_size                            :  4, // [22:19]
                      ftm_fields_valid                                        :  1, // [23:23]
                      ftm_pe_nss                                              :  3, // [26:24]
                      ftm_pe_ltf_size                                         :  2, // [28:27]
                      ftm_pe_content                                          :  1, // [29:29]
                      ftm_chain_csd_en                                        :  1, // [30:30]
                      ftm_pe_chain_csd_en                                     :  1; // [31:31]
             uint32_t dot11ax_response_rate_source                            :  8, // [7:0]
                      dot11ax_ext_response_rate_source                        :  8, // [15:8]
                      sw_peer_id                                              : 16; // [31:16]
             uint32_t dot11be_puncture_bitmap                                 : 16, // [15:0]
                      dot11be_response                                        :  1, // [16:16]
                      punctured_response                                      :  1, // [17:17]
                      eht_duplicate_mode                                      :  2, // [19:18]
                      force_extra_symbol                                      :  1, // [20:20]
                      reserved_13a                                            :  5, // [25:21]
                      u_sig_puncture_pattern_encoding                         :  6; // [31:26]
             struct   mlo_sta_id_details                                        mlo_sta_id_details_rx;
             uint16_t he_a_control_response_time                              : 12, // [27:16]
                      reserved_after_struct16                                 :  4; // [31:28]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t wait_sifs                                               :  2, // [31:30]
                      wait_sifs_config_valid                                  :  1, // [29:29]
                      three_or_more_type_subtypes                             :  1, // [28:28]
                      reserved_0a                                             :  2, // [27:26]
                      r2r_end_status_to_follow                                :  1, // [25:25]
                      ast_search_incomplete                                   :  1, // [24:24]
                      filter_pass_monitor_ovrd                                :  1, // [23:23]
                      mac_security                                            :  1, // [22:22]
                      tb_ranging_response_required                            :  2, // [21:20]
                      ftm_tm                                                  :  2, // [19:18]
                      trigger_frame_received                                  :  1, // [17:17]
                      su_or_uplink_mu_reception                               :  1, // [16:16]
                      phy_ppdu_id                                             : 16; // [15:0]
             uint32_t second_frame_control                                    : 16, // [31:16]
                      general_frame_control                                   : 16; // [15:0]
             uint32_t rts_ta_grp_bit                                          :  1, // [31:31]
                      vht_ack                                                 :  1, // [30:30]
                      ampdu                                                   :  1, // [29:29]
                      ldpc                                                    :  1, // [28:28]
                      stbc                                                    :  1, // [27:27]
                      sgi                                                     :  2, // [26:25]
                      rate_mcs                                                :  4, // [24:21]
                      dot11ax_su_extended                                     :  1, // [20:20]
                      pkt_type                                                :  4, // [19:16]
                      duration                                                : 16; // [15:0]
             uint32_t reserved_3a                                             :  2, // [31:30]
                      second_frame_control_valid                              :  1, // [29:29]
                      ndp_frame_type                                          :  3, // [28:26]
                      ndp_indication                                          :  1, // [25:25]
                      response_indication                                     :  2, // [24:23]
                      ctrl_resp_pwr_mgmt                                      :  1, // [22:22]
                      group_id                                                :  6, // [21:16]
                      sta_partial_aid                                         : 12, // [15:4]
                      m_pkt                                                   :  1, // [3:3]
                      service_dynamic                                         :  1, // [2:2]
                      ast_fail_for_dot11ax_su_ext                             :  1, // [1:1]
                      ctrl_frame_soliciting_resp                              :  1; // [0:0]
             uint32_t service_cbw                                             :  3, // [31:29]
                      agc_cbw                                                 :  3, // [28:26]
                      ack_id_ext                                              : 10, // [25:16]
                      ack_id                                                  : 16; // [15:0]
             uint32_t txop_duration_all_ones                                  :  1, // [31:31]
                      response_assoc_ack_count                                :  7, // [30:24]
                      response_ack_count                                      :  7, // [23:17]
                      cts_cbw                                                 :  3, // [16:14]
                      ht_vht_sig_cbw                                          :  3, // [13:11]
                      reserved                                                :  4, // [10:7]
                      response_sta_count                                      :  7; // [6:0]
             uint32_t reserved_6a                                             :  1, // [31:31]
                      dot11ax_dl_ul_flag                                      :  1, // [30:30]
                      sw_response_tlv_from_crypto                             :  1, // [29:29]
                      multi_tid                                               :  1, // [28:28]
                      response_ba256_count                                    :  7, // [27:21]
                      response_ba128_count                                    :  7, // [20:14]
                      response_ba64_count                                     :  7, // [13:7]
                      response_ba32_count                                     :  7; // [6:0]
             uint32_t reserved_7a                                             :  2, // [31:30]
                      response_ba1024_count                                   :  7, // [29:23]
                      response_ba512_count                                    :  7, // [22:16]
                      sw_response_frame_length                                : 16; // [15:0]
             uint32_t addr1_31_0                                              : 32; // [31:0]
             uint32_t addr2_15_0                                              : 16, // [31:16]
                      addr1_47_32                                             : 16; // [15:0]
             uint32_t addr2_47_16                                             : 32; // [31:0]
             uint32_t ftm_pe_chain_csd_en                                     :  1, // [31:31]
                      ftm_chain_csd_en                                        :  1, // [30:30]
                      ftm_pe_content                                          :  1, // [29:29]
                      ftm_pe_ltf_size                                         :  2, // [28:27]
                      ftm_pe_nss                                              :  3, // [26:24]
                      ftm_fields_valid                                        :  1, // [23:23]
                      dot11ax_received_ext_ru_size                            :  4, // [22:19]
                      dot11ax_received_doppler_indication                     :  1, // [18:18]
                      dot11ax_received_dcm                                    :  1, // [17:17]
                      dot11ax_received_coding                                 :  1, // [16:16]
                      dot11ax_received_ltf_size                               :  2, // [15:14]
                      dot11ax_received_cp_size                                :  2, // [13:12]
                      dot11ax_received_spatial_reuse                          :  4, // [11:8]
                      dot11ax_received_bss_color_id                           :  6, // [7:2]
                      dot11ax_received_dl_ul_flag                             :  1, // [1:1]
                      dot11ax_received_format_indication                      :  1; // [0:0]
             uint32_t sw_peer_id                                              : 16, // [31:16]
                      dot11ax_ext_response_rate_source                        :  8, // [15:8]
                      dot11ax_response_rate_source                            :  8; // [7:0]
             uint32_t u_sig_puncture_pattern_encoding                         :  6, // [31:26]
                      reserved_13a                                            :  5, // [25:21]
                      force_extra_symbol                                      :  1, // [20:20]
                      eht_duplicate_mode                                      :  2, // [19:18]
                      punctured_response                                      :  1, // [17:17]
                      dot11be_response                                        :  1, // [16:16]
                      dot11be_puncture_bitmap                                 : 16; // [15:0]
             uint32_t reserved_after_struct16                                 :  4, // [31:28]
                      he_a_control_response_time                              : 12; // [27:16]
             struct   mlo_sta_id_details                                        mlo_sta_id_details_rx;
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		PHY_PPDU_ID

			A ppdu counter value that PHY increments for every PPDU 
			received. The counter value wraps around  
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_PHY_PPDU_ID_OFFSET                                0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_PHY_PPDU_ID_LSB                                   0
#define RX_RESPONSE_REQUIRED_INFO_PHY_PPDU_ID_MSB                                   15
#define RX_RESPONSE_REQUIRED_INFO_PHY_PPDU_ID_MASK                                  0x000000000000ffff


/* Description		SU_OR_UPLINK_MU_RECEPTION

			<enum 0 Reception_is_SU>  This TLV is the result of an SU
			 reception. Note that this can be regular SU reception or
			 an SU reception as part of a downlink MU - MIMO/OFDMA transmission.
			
			
			<enum 1 Reception_is_MU> This TLV is the result of an MU_OFDMA
			 uplink reception or MU_MIMO uplink reception
			
			NOTE:When a STA receives a downlink MU-MIMO or DL MU_OFDMA, 
			this field shall still be set to Reception_is_SU. From the
			 STA perspective, it is only receiving from one other device.
			
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_SU_OR_UPLINK_MU_RECEPTION_OFFSET                  0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_SU_OR_UPLINK_MU_RECEPTION_LSB                     16
#define RX_RESPONSE_REQUIRED_INFO_SU_OR_UPLINK_MU_RECEPTION_MSB                     16
#define RX_RESPONSE_REQUIRED_INFO_SU_OR_UPLINK_MU_RECEPTION_MASK                    0x0000000000010000


/* Description		TRIGGER_FRAME_RECEIVED

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			When set, this TLV has been sent because a trigger frame
			 has been received.
			
			Note that in case there were other frames received as well
			 that required an immediate response, like data or management
			 frames, this will still be indicated here in this TLV with
			 the fields "Response_..._count".
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_TRIGGER_FRAME_RECEIVED_OFFSET                     0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_TRIGGER_FRAME_RECEIVED_LSB                        17
#define RX_RESPONSE_REQUIRED_INFO_TRIGGER_FRAME_RECEIVED_MSB                        17
#define RX_RESPONSE_REQUIRED_INFO_TRIGGER_FRAME_RECEIVED_MASK                       0x0000000000020000


/* Description		FTM_TM

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field Indicates if the response is related to receiving
			 a TM or FTM frame
			
			0: no TM and no FTM frame => there is NO measurement done
			
			1: FTM frame
			2: TM frame
			3: reserved
*/

#define RX_RESPONSE_REQUIRED_INFO_FTM_TM_OFFSET                                     0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_FTM_TM_LSB                                        18
#define RX_RESPONSE_REQUIRED_INFO_FTM_TM_MSB                                        19
#define RX_RESPONSE_REQUIRED_INFO_FTM_TM_MASK                                       0x00000000000c0000


/* Description		TB_RANGING_RESPONSE_REQUIRED

			Field only valid in case of TB Ranging
			<enum 0 No_TB_Ranging_Resp>
			<enum 1 CTS2S_Resp_to_TF_poll > TXPCU to generate CTS-to-self
			 in TB response
			<enum 2 LMR_Resp_to_TF_report> TXPCU to generate LMR in 
			TB response
			<enum 3 NDP_Resp_to_TF_sound> DO NOT USE.
			<legal 0-3>
*/

#define RX_RESPONSE_REQUIRED_INFO_TB_RANGING_RESPONSE_REQUIRED_OFFSET               0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_TB_RANGING_RESPONSE_REQUIRED_LSB                  20
#define RX_RESPONSE_REQUIRED_INFO_TB_RANGING_RESPONSE_REQUIRED_MSB                  21
#define RX_RESPONSE_REQUIRED_INFO_TB_RANGING_RESPONSE_REQUIRED_MASK                 0x0000000000300000


/* Description		MAC_SECURITY

			Field only valid if TB_Ranging_response_required = LMR_Resp_to_TF_report
			
			
			Indicates whether MAC security is enabled for LMR
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_MAC_SECURITY_OFFSET                               0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_MAC_SECURITY_LSB                                  22
#define RX_RESPONSE_REQUIRED_INFO_MAC_SECURITY_MSB                                  22
#define RX_RESPONSE_REQUIRED_INFO_MAC_SECURITY_MASK                                 0x0000000000400000


/* Description		FILTER_PASS_MONITOR_OVRD

			Consumer: TXMON/SW
			Producer: RXPCU
			
			This indicates that the Rx MPDU passed the 'normal' frame
			 filter programming of RXPCU and additionally the MAC address
			 search matched an 'ADDR_SEARCH_ENTRY' of a 'Monitor_override_sta.'
			
			
			When enabled in TXMON, it will discard the upstream response
			 TLVs for cases not matching the 'Filter_pass_Monitor_ovrd' 
			criterion.
			
			If RXPCU is generating this TLV before the address search
			 is complete, it shall fill this bit based on a register
			 configuration 'FILTER_PASS_OVRD_AST_NOT_DONE.'
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_FILTER_PASS_MONITOR_OVRD_OFFSET                   0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_FILTER_PASS_MONITOR_OVRD_LSB                      23
#define RX_RESPONSE_REQUIRED_INFO_FILTER_PASS_MONITOR_OVRD_MSB                      23
#define RX_RESPONSE_REQUIRED_INFO_FILTER_PASS_MONITOR_OVRD_MASK                     0x0000000000800000


/* Description		AST_SEARCH_INCOMPLETE

			Consumer: SW
			Producer: RXPCU
			
			If RXPCU is generating this TLV before the address search
			 is complete, it shall set this bit. This is to indicate
			 to SW (via TXMON) that the Filter_pass_Monitor_ovrd bit
			 is unreliable and SW may have to add their own filtering
			 logic.
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_AST_SEARCH_INCOMPLETE_OFFSET                      0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_AST_SEARCH_INCOMPLETE_LSB                         24
#define RX_RESPONSE_REQUIRED_INFO_AST_SEARCH_INCOMPLETE_MSB                         24
#define RX_RESPONSE_REQUIRED_INFO_AST_SEARCH_INCOMPLETE_MASK                        0x0000000001000000


/* Description		R2R_END_STATUS_TO_FOLLOW

			Consumer: TXMON
			Producer: TXPCU
			
			When set, TXPCU will generate an R2R frame (typically M-BA), 
			and the 'R2R_STATUS_END' TLV.
			
			TXMON uses this to identify the continuation of a Tx sequence
			 (typically including Trigger frames) with R2R Tx.
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_R2R_END_STATUS_TO_FOLLOW_OFFSET                   0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_R2R_END_STATUS_TO_FOLLOW_LSB                      25
#define RX_RESPONSE_REQUIRED_INFO_R2R_END_STATUS_TO_FOLLOW_MSB                      25
#define RX_RESPONSE_REQUIRED_INFO_R2R_END_STATUS_TO_FOLLOW_MASK                     0x0000000002000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_0A_OFFSET                                0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_0A_LSB                                   26
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_0A_MSB                                   27
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_0A_MASK                                  0x000000000c000000


/* Description		THREE_OR_MORE_TYPE_SUBTYPES

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			When set, there are 3 or more different frame type/subtypes
			 received that all required a response.
			Note that the HW will only report the very first two that
			 have been seen
*/

#define RX_RESPONSE_REQUIRED_INFO_THREE_OR_MORE_TYPE_SUBTYPES_OFFSET                0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_THREE_OR_MORE_TYPE_SUBTYPES_LSB                   28
#define RX_RESPONSE_REQUIRED_INFO_THREE_OR_MORE_TYPE_SUBTYPES_MSB                   28
#define RX_RESPONSE_REQUIRED_INFO_THREE_OR_MORE_TYPE_SUBTYPES_MASK                  0x0000000010000000


/* Description		WAIT_SIFS_CONFIG_VALID

			When set, TXPCU shall follow the wait_sifs configuration.
			
			
			Field added to be backwards compatible, and transition to
			 the new signalling.
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_CONFIG_VALID_OFFSET                     0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_CONFIG_VALID_LSB                        29
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_CONFIG_VALID_MSB                        29
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_CONFIG_VALID_MASK                       0x0000000020000000


/* Description		WAIT_SIFS

			Indicates to the TXPCU how precise the SIFS the response
			 timing shall be...
			
			The configuration for this is coming from SW programmable
			 register in RXPCU
			
			<enum 0 NO_SIFS_TIMING> Transmission shall start with the
			 normal delay in PHY after receiving this notification
			<enum 1 SIFS_TIMING_DESIRED> Transmission shall be made 
			at the SIFS boundary. If shall never start before SIFS boundary, 
			but if it a little later, it is not ideal and should be 
			flagged, but transmission shall not be aborted.
			<enum 2 SIFS_TIMING_MANDATED> Transmission shall be made
			 at exactly SIFS boundary. If this notification is received
			 by the PHY after SIFS boundary already passed, the PHY 
			shall abort the transmission
			<legal 0-2>
*/

#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_OFFSET                                  0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_LSB                                     30
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_MSB                                     31
#define RX_RESPONSE_REQUIRED_INFO_WAIT_SIFS_MASK                                    0x00000000c0000000


/* Description		GENERAL_FRAME_CONTROL

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			In case only a single frame is receive, this field will 
			always contain the frame control field of the received frame.
			
			
			In case multiple frames are received that require a response, 
			and one of those frames is not a data frame, this field 
			will always contain the frame control field of that received
			 frame.
			
			In case multiple frames are received that require a response, 
			but all have them have the same type/subtype, this field
			 will contain the very first one of them.
			
			Note: In case of a BAR frame reception, the 'response_ack_...' 
			fields will indicate for how many TIDs a BA is needed, as
			 well as their individual sizes.
			
			Used by TXPCU to determine the type of response that is 
			needed
			
			TODO: Look at table below for all the possible combination
			 of frames types reported here and in the next field: Second_frame_control
			
*/

#define RX_RESPONSE_REQUIRED_INFO_GENERAL_FRAME_CONTROL_OFFSET                      0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_GENERAL_FRAME_CONTROL_LSB                         32
#define RX_RESPONSE_REQUIRED_INFO_GENERAL_FRAME_CONTROL_MSB                         47
#define RX_RESPONSE_REQUIRED_INFO_GENERAL_FRAME_CONTROL_MASK                        0x0000ffff00000000


/* Description		SECOND_FRAME_CONTROL

			Field only valid when Second_frame_control_valid ==1
			
			In case multiple frames of different frame type/subtype 
			are received that require a response, this field will always
			 contain the frame control field remaining after the 'frame_control
			 ' field has been filled in.
			
			NOTE: in case more then 2 different frame type/subtypes 
			are received (which only happens if the transmitter did 
			something wrong), only the first two frame types are reported
			 in this and the General_frame_control field. All the other
			 ones are ignored, but bit 'three_or_more_type_subtypes' 
			shall be set.
			
			Note: In case of a BAR frame reception, the 'response_ack_...' 
			fields will indicate for how many TIDs a BA is needed, as
			 well as their individual sizes.
			
			Used by TXPCU to determine the type of response that is 
			needed
*/

#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_OFFSET                       0x0000000000000000
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_LSB                          48
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_MSB                          63
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_MASK                         0xffff000000000000


/* Description		DURATION

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			duration field of the received frame
*/

#define RX_RESPONSE_REQUIRED_INFO_DURATION_OFFSET                                   0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_DURATION_LSB                                      0
#define RX_RESPONSE_REQUIRED_INFO_DURATION_MSB                                      15
#define RX_RESPONSE_REQUIRED_INFO_DURATION_MASK                                     0x000000000000ffff


/* Description		PKT_TYPE

			Packet type:
			
			Note that for MU UL reception, this field can only be set
			 to dot11ax.
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

#define RX_RESPONSE_REQUIRED_INFO_PKT_TYPE_OFFSET                                   0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_PKT_TYPE_LSB                                      16
#define RX_RESPONSE_REQUIRED_INFO_PKT_TYPE_MSB                                      19
#define RX_RESPONSE_REQUIRED_INFO_PKT_TYPE_MASK                                     0x00000000000f0000


/* Description		DOT11AX_SU_EXTENDED

			This field is only valid for pkt_type == 11ax OR pkt_type
			 == 11be
			
			When set, the 11ax or 11be reception was an extended range
			 SU 
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_SU_EXTENDED_OFFSET                        0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_SU_EXTENDED_LSB                           20
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_SU_EXTENDED_MSB                           20
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_SU_EXTENDED_MASK                          0x0000000000100000


/* Description		RATE_MCS

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			
			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_RATE_MCS_OFFSET                                   0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_RATE_MCS_LSB                                      21
#define RX_RESPONSE_REQUIRED_INFO_RATE_MCS_MSB                                      24
#define RX_RESPONSE_REQUIRED_INFO_RATE_MCS_MASK                                     0x0000000001e00000


/* Description		SGI

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Field only valid when pkt type is HT, VHT or HE.
			
			Specify the right GI for HE-Ranging NDPs (11az).
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > HE related GI
			<enum 3     3_2_us_sgi > HE related GI
			
			<legal 0 - 3>
*/

#define RX_RESPONSE_REQUIRED_INFO_SGI_OFFSET                                        0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_SGI_LSB                                           25
#define RX_RESPONSE_REQUIRED_INFO_SGI_MSB                                           26
#define RX_RESPONSE_REQUIRED_INFO_SGI_MASK                                          0x0000000006000000


/* Description		STBC

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Indicate STBC
			
			In 11ah mode of Operation, this bit indicates the STBC bit
			 setting in the SIG Preamble.
*/

#define RX_RESPONSE_REQUIRED_INFO_STBC_OFFSET                                       0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_STBC_LSB                                          27
#define RX_RESPONSE_REQUIRED_INFO_STBC_MSB                                          27
#define RX_RESPONSE_REQUIRED_INFO_STBC_MASK                                         0x0000000008000000


/* Description		LDPC

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Indicate LDPC
			
			In 11ah mode of Operation, this bit indicates the LDPC bit
			 setting in the SIG Preamble.
*/

#define RX_RESPONSE_REQUIRED_INFO_LDPC_OFFSET                                       0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_LDPC_LSB                                          28
#define RX_RESPONSE_REQUIRED_INFO_LDPC_MSB                                          28
#define RX_RESPONSE_REQUIRED_INFO_LDPC_MASK                                         0x0000000010000000


/* Description		AMPDU

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Field indicates if the received frame was in ampdu format
			 or not. If set, it implies the reception was 11n, aggregation, 
			11ac or 11ax.
			
			Within TXPCU it is used to determine if the response will
			 have to be BA format or not. Note that there are some exceptions
			 where received frame was A-MPDU format, but the response
			 will still be just an ACK frame.
*/

#define RX_RESPONSE_REQUIRED_INFO_AMPDU_OFFSET                                      0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_AMPDU_LSB                                         29
#define RX_RESPONSE_REQUIRED_INFO_AMPDU_MSB                                         29
#define RX_RESPONSE_REQUIRED_INFO_AMPDU_MASK                                        0x0000000020000000


/* Description		VHT_ACK

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			set when ACK is required to be generated
*/

#define RX_RESPONSE_REQUIRED_INFO_VHT_ACK_OFFSET                                    0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_VHT_ACK_LSB                                       30
#define RX_RESPONSE_REQUIRED_INFO_VHT_ACK_MSB                                       30
#define RX_RESPONSE_REQUIRED_INFO_VHT_ACK_MASK                                      0x0000000040000000


/* Description		RTS_TA_GRP_BIT

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			frame is rts and TA G/I bit is set
*/

#define RX_RESPONSE_REQUIRED_INFO_RTS_TA_GRP_BIT_OFFSET                             0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_RTS_TA_GRP_BIT_LSB                                31
#define RX_RESPONSE_REQUIRED_INFO_RTS_TA_GRP_BIT_MSB                                31
#define RX_RESPONSE_REQUIRED_INFO_RTS_TA_GRP_BIT_MASK                               0x0000000080000000


/* Description		CTRL_FRAME_SOLICITING_RESP

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			frame is rts, bar or ps_poll and TA G/I bit is set
*/

#define RX_RESPONSE_REQUIRED_INFO_CTRL_FRAME_SOLICITING_RESP_OFFSET                 0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_CTRL_FRAME_SOLICITING_RESP_LSB                    32
#define RX_RESPONSE_REQUIRED_INFO_CTRL_FRAME_SOLICITING_RESP_MSB                    32
#define RX_RESPONSE_REQUIRED_INFO_CTRL_FRAME_SOLICITING_RESP_MASK                   0x0000000100000000


/* Description		AST_FAIL_FOR_DOT11AX_SU_EXT

			Field only valid in case of
			dot11ax_su_extended = 1
			
			When set, the just finished reception had address search
			 failure (e.g. unassociated STA).
			This field can be used to determine special response rates
			 for those types of STAs.
			This field shall be analyzed in combination with pkt_type
			 and dot11ax_su_extended settings.
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_AST_FAIL_FOR_DOT11AX_SU_EXT_OFFSET                0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_AST_FAIL_FOR_DOT11AX_SU_EXT_LSB                   33
#define RX_RESPONSE_REQUIRED_INFO_AST_FAIL_FOR_DOT11AX_SU_EXT_MSB                   33
#define RX_RESPONSE_REQUIRED_INFO_AST_FAIL_FOR_DOT11AX_SU_EXT_MASK                  0x0000000200000000


/* Description		SERVICE_DYNAMIC

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Dynamic field extracted from Service field
			
			Reserved for 11ah. Should be populated to zero by RxPCU
*/

#define RX_RESPONSE_REQUIRED_INFO_SERVICE_DYNAMIC_OFFSET                            0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_DYNAMIC_LSB                               34
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_DYNAMIC_MSB                               34
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_DYNAMIC_MASK                              0x0000000400000000


/* Description		M_PKT

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Indicates that RXPCU has detected a 802.11v M packet.  The
			 TXPCU should generate a TX_FREEZE_CAPTURE_CHANNEL message
			 to the PHY so that the PHY will hold the current channel
			 capture so FW can read the channel capture memory over 
			APB.
			Reserved for 11ah. Should be populated to zero by RxPCU
*/

#define RX_RESPONSE_REQUIRED_INFO_M_PKT_OFFSET                                      0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_M_PKT_LSB                                         35
#define RX_RESPONSE_REQUIRED_INFO_M_PKT_MSB                                         35
#define RX_RESPONSE_REQUIRED_INFO_M_PKT_MASK                                        0x0000000800000000


/* Description		STA_PARTIAL_AID

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Specifies the partial AID of response frames transmitted
			 at VHT rates.
			
			In 11ah mode of operation, this field is used to populate
			 the RA/partial BSSID filed in the NDP CTS response frame. 
			Please refer to the 802.11 spec for details on the NDP CTS
			 frame format.
			
			Reserved for 11ah. 
			Should be populated to zero by RxPCU 
*/

#define RX_RESPONSE_REQUIRED_INFO_STA_PARTIAL_AID_OFFSET                            0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_STA_PARTIAL_AID_LSB                               36
#define RX_RESPONSE_REQUIRED_INFO_STA_PARTIAL_AID_MSB                               47
#define RX_RESPONSE_REQUIRED_INFO_STA_PARTIAL_AID_MASK                              0x0000fff000000000


/* Description		GROUP_ID

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Reserved for 11ah. 
			Should be populated to zero by RxPCU
*/

#define RX_RESPONSE_REQUIRED_INFO_GROUP_ID_OFFSET                                   0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_GROUP_ID_LSB                                      48
#define RX_RESPONSE_REQUIRED_INFO_GROUP_ID_MSB                                      53
#define RX_RESPONSE_REQUIRED_INFO_GROUP_ID_MASK                                     0x003f000000000000


/* Description		CTRL_RESP_PWR_MGMT

			Field valid in case of both  SU_or_uplink_MU_reception = 
			Reception_is_SU
			AND
			SU_or_uplink_MU_reception = Reception_is_MU
			
			RX PCU passes this bit (coming from the peer entry) setting
			 on to TX PCU, where the setting of this bit is inserted
			 in the pwr_mgt bit in the control field of the SIFS response
			 control frames: ACK, CTS, BA
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_CTRL_RESP_PWR_MGMT_OFFSET                         0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_CTRL_RESP_PWR_MGMT_LSB                            54
#define RX_RESPONSE_REQUIRED_INFO_CTRL_RESP_PWR_MGMT_MSB                            54
#define RX_RESPONSE_REQUIRED_INFO_CTRL_RESP_PWR_MGMT_MASK                           0x0040000000000000


/* Description		RESPONSE_INDICATION

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			<enum 0     no_response>
			<enum 1     ndp_response>
			<enum 2     normal_response>
			<enum 3     long_response>
			<legal  0-3>
			
			This field indicates the Response Indication of the received
			 PPDU. RxPCU populates this field using the Response Indication
			 bits extracted from the SIG in the received PPDU. 
*/

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_INDICATION_OFFSET                        0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_INDICATION_LSB                           55
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_INDICATION_MSB                           56
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_INDICATION_MASK                          0x0180000000000000


/* Description		NDP_INDICATION

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is valid in 11ah mode of opearation only.  In
			 non-11ah mode, this bit is reserved and RxPCU populates
			 this bit to Zero.
			
			NDP Indication bit. 
			
			This field is set if the received SIG has the NDP Indication
			 bit set. 
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_NDP_INDICATION_OFFSET                             0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_NDP_INDICATION_LSB                                57
#define RX_RESPONSE_REQUIRED_INFO_NDP_INDICATION_MSB                                57
#define RX_RESPONSE_REQUIRED_INFO_NDP_INDICATION_MASK                               0x0200000000000000


/* Description		NDP_FRAME_TYPE

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Include the ndp_frame_type encoding.
			
			This field is valid in 11ah mode of opearation only.  In
			 non-11ah mode, this bit is reserved and RxPCU populates
			 this bit to Zero.
			
			The ndp_frame_type filed form the SIG is extracted and is
			 populated in this field by RxPCU. TxPCU can decode the 
			NDP frame type. 
*/

#define RX_RESPONSE_REQUIRED_INFO_NDP_FRAME_TYPE_OFFSET                             0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_NDP_FRAME_TYPE_LSB                                58
#define RX_RESPONSE_REQUIRED_INFO_NDP_FRAME_TYPE_MSB                                60
#define RX_RESPONSE_REQUIRED_INFO_NDP_FRAME_TYPE_MASK                               0x1c00000000000000


/* Description		SECOND_FRAME_CONTROL_VALID

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			When set, the second frame control field is valid.
*/

#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_VALID_OFFSET                 0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_VALID_LSB                    61
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_VALID_MSB                    61
#define RX_RESPONSE_REQUIRED_INFO_SECOND_FRAME_CONTROL_VALID_MASK                   0x2000000000000000


/* Description		RESERVED_3A

			<legal 0>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_3A_OFFSET                                0x0000000000000008
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_3A_LSB                                   62
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_3A_MSB                                   63
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_3A_MASK                                  0xc000000000000000


/* Description		ACK_ID

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Indicates the  ACD_ID to be used in NDP response frames (NDP
			 ACK and NDP Modified ACK). 
			
			For NDP ACK 
			ACK_ID (16bits)= {Scrambler Initialization[0:6], FCS[23:31} 
			  for 2MHz 
			ACK_ID (9bits)= { Scrambler Initialization[0:6], FCS[30:31]} 
			for 1MHz. Bits[15:9] should be filled with Zero by RxPCU
			
			
			For NDP Modified ACK
			ACK_ID (16bits)= {CRC[0:3],TA[0:8],RA[6:8]}   for 2MHz 
			ACK_ID (9bits)= { CRC[0:3], TA[4:8]} for 1MHz; Bits[15:9] 
			should be filled with Zero by RxPCU. 
*/

#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_OFFSET                                     0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_LSB                                        0
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_MSB                                        15
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_MASK                                       0x000000000000ffff


/* Description		ACK_ID_EXT

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This is populated by RxPCU when the Duration Indication 
			Bit is set to Zero in the Received NDP PS-Poll Frame.  
*/

#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_EXT_OFFSET                                 0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_EXT_LSB                                    16
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_EXT_MSB                                    25
#define RX_RESPONSE_REQUIRED_INFO_ACK_ID_EXT_MASK                                   0x0000000003ff0000


/* Description		AGC_CBW

			BW as detected by the AGC 
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define RX_RESPONSE_REQUIRED_INFO_AGC_CBW_OFFSET                                    0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_AGC_CBW_LSB                                       26
#define RX_RESPONSE_REQUIRED_INFO_AGC_CBW_MSB                                       28
#define RX_RESPONSE_REQUIRED_INFO_AGC_CBW_MASK                                      0x000000001c000000


/* Description		SERVICE_CBW

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field reflects the BW extracted from the Serivce Field
			 for 11ac mode of operation and from the FC portion of the
			 MAC header in 11ah mode of operation. This field is used
			 in the context of Dynamic/Static BW evaluation purposes
			 in TxPCU
			CBW field extracted from Service field by RXPCU and populates
			 this
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define RX_RESPONSE_REQUIRED_INFO_SERVICE_CBW_OFFSET                                0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_CBW_LSB                                   29
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_CBW_MSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_SERVICE_CBW_MASK                                  0x00000000e0000000


/* Description		RESPONSE_STA_COUNT

			The number of STAs to which the responses need to be sent.
			
			
			In case of multiple ACKs/BAs to be send, TXPCU uses this
			 field to determine what address formatting to use for the
			 response frame: This could be broadcast or unicast.
			
			<legal 0-36>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_STA_COUNT_OFFSET                         0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_STA_COUNT_LSB                            32
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_STA_COUNT_MSB                            38
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_STA_COUNT_MASK                           0x0000007f00000000


/* Description		RESERVED

			<legal 0> 
*/

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_OFFSET                                   0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_LSB                                      39
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_MSB                                      42
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_MASK                                     0x0000078000000000


/* Description		HT_VHT_SIG_CBW

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Bandwidth of the received frame from either the HT-SIG or
			 VHT-SIG-A or HE-SIG. For HT-SIG, this bandwidth can be 
			20 MHz or 40 MHz, For VHT or HE, this bandwidth can be 20, 
			40, 80, or 160 MHz:
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define RX_RESPONSE_REQUIRED_INFO_HT_VHT_SIG_CBW_OFFSET                             0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_HT_VHT_SIG_CBW_LSB                                43
#define RX_RESPONSE_REQUIRED_INFO_HT_VHT_SIG_CBW_MSB                                45
#define RX_RESPONSE_REQUIRED_INFO_HT_VHT_SIG_CBW_MASK                               0x0000380000000000


/* Description		CTS_CBW

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Calculated bandwidth for the CTS response frame
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define RX_RESPONSE_REQUIRED_INFO_CTS_CBW_OFFSET                                    0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_CTS_CBW_LSB                                       46
#define RX_RESPONSE_REQUIRED_INFO_CTS_CBW_MSB                                       48
#define RX_RESPONSE_REQUIRED_INFO_CTS_CBW_MASK                                      0x0001c00000000000


/* Description		RESPONSE_ACK_COUNT

			Field valid for both SU and MU reception
			
			ACK Count for management action frames, PS_POLL frames, 
			single data frame and the general "ACK ALL". For this last
			 one, a single "ACK" should be interpreted by the receiver
			 that all transmitted frames have been properly received.
			
			
			For SU:
			Max count can be 1
			Note that Response_ba64_count and/or Response_ba256_count
			 can be > 0, which implies that both an ACK and BA needs
			 to be send back.
			
			For MU:
			The number of users that need an 'ACK' response.
			<legal 0-36>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ACK_COUNT_OFFSET                         0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ACK_COUNT_LSB                            49
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ACK_COUNT_MSB                            55
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ACK_COUNT_MASK                           0x00fe000000000000


/* Description		RESPONSE_ASSOC_ACK_COUNT

			Field ONLY valid for Reception_is_MU. This count can only
			 be set to > 0, when there were wildcards send in the trigger
			 frame.
			
			ACK Count to be generated for Management frames from STAs
			 that are not yet associated to this device. These STAs 
			can only send this type of response when the trigger frame
			 included some wildcards.
			
			Note that in the MBA frame, this "ack" has a special format, 
			and includes more bytes then the normal "ack". For that 
			reason TXPCU needs to be able to differentiate between the
			 'normal acks' and these association request acks...
			<legal 0-36>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ASSOC_ACK_COUNT_OFFSET                   0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ASSOC_ACK_COUNT_LSB                      56
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ASSOC_ACK_COUNT_MSB                      62
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_ASSOC_ACK_COUNT_MASK                     0x7f00000000000000


/* Description		TXOP_DURATION_ALL_ONES

			When set, either the TXOP_DURATION of the received frame
			 was set to all 1s or there is a BSS color collision. The
			 TXOP_DURATION of the transmit response should be forced
			 to all 1s.
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_TXOP_DURATION_ALL_ONES_OFFSET                     0x0000000000000010
#define RX_RESPONSE_REQUIRED_INFO_TXOP_DURATION_ALL_ONES_LSB                        63
#define RX_RESPONSE_REQUIRED_INFO_TXOP_DURATION_ALL_ONES_MSB                        63
#define RX_RESPONSE_REQUIRED_INFO_TXOP_DURATION_ALL_ONES_MASK                       0x8000000000000000


/* Description		RESPONSE_BA32_COUNT

			Field valid for both Reception_is_SU and Reception_is_MU
			
			
			For SU:
			Total number of '32 bitmap BA' responses for this one user.
			
			If this value is > 1, in implies that multi TID response
			 is needed. Also, if the sum of all the Response_ba??? Counts
			 ) > 1, a multi TID response is needed.
			
			For MU:
			Total number of '32 bitmap BA' responses shared between 
			all the users.
			<legal 0-36>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA32_COUNT_OFFSET                        0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA32_COUNT_LSB                           0
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA32_COUNT_MSB                           6
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA32_COUNT_MASK                          0x000000000000007f


/* Description		RESPONSE_BA64_COUNT

			Field valid for both Reception_is_SU and Reception_is_MU
			
			
			For SU:
			Total number of '64 bitmap BA' responses for this one user.
			
			If this value is > 1, in implies that multi TID response
			 is needed. Also, if the sum of all the Response_ba??? Counts
			 ) > 1, a multi TID response is needed.
			
			For MU:
			Total number of '64 bitmap BA' responses shared between 
			all the users.
			<legal 0-36>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA64_COUNT_OFFSET                        0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA64_COUNT_LSB                           7
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA64_COUNT_MSB                           13
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA64_COUNT_MASK                          0x0000000000003f80


/* Description		RESPONSE_BA128_COUNT

			Field valid for both Reception_is_SU and Reception_is_MU
			
			
			For SU:
			Total number of '128 bitmap BA' responses for this one user.
			
			If this value is > 1, in implies that multi TID response
			 is needed. Also, if the sum of all the Response_ba??? Counts
			 ) > 1, a multi TID response is needed.
			
			For MU:
			Total number of '128 bitmap BA' responses shared between
			 all the users.
			<legal 0-36>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA128_COUNT_OFFSET                       0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA128_COUNT_LSB                          14
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA128_COUNT_MSB                          20
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA128_COUNT_MASK                         0x00000000001fc000


/* Description		RESPONSE_BA256_COUNT

			Field valid for both Reception_is_SU and Reception_is_MU
			
			
			For SU:
			Total number of '256 bitmap BA' responses for this one user.
			
			If this value is > 1, in implies that multi TID response
			 is needed. Also, if the sum of all the Response_ba??? Counts
			 ) > 1, a multi TID response is needed.
			
			For MU:
			Total number of '256 bitmap BA' responses shared between
			 all the users.
			<legal 0-36>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA256_COUNT_OFFSET                       0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA256_COUNT_LSB                          21
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA256_COUNT_MSB                          27
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA256_COUNT_MASK                         0x000000000fe00000


/* Description		MULTI_TID

			Field valid for both Reception_is_SU and Reception_is_MU
			
			
			When set, RXPCU has for at least one user multiple bitmaps
			 available (which corresponds to multiple TIDs)
			
			Note that the sum of Response_ack_count, 
			response_ba32_count, response_ba64_count, 
			response_ba128_count, response_ba256_count is larger then
			 the total number of users.
			
			Note: There is no restriction on TXPCU to retrieve all the
			 bitmaps using explicit_user_request mode or not.
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_MULTI_TID_OFFSET                                  0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_MULTI_TID_LSB                                     28
#define RX_RESPONSE_REQUIRED_INFO_MULTI_TID_MSB                                     28
#define RX_RESPONSE_REQUIRED_INFO_MULTI_TID_MASK                                    0x0000000010000000


/* Description		SW_RESPONSE_TLV_FROM_CRYPTO

			Field can only be set by MAC mitigation logic
			
			The idea is here that normally TXPCU generates the BA frame.
			
			But as a backup scenario, in case of a last moment BA format
			 change or some other issue, the BA frame could be fully
			 generated in the MAC micro CPU and pushed into TXPCU through
			 the Crypto - TXPCU TLV interface.
			This feature can be used for any response frame generation. 
			From TXPCU perspective, all interaction with PDG remains
			 exactly the same, accept that the frame length is now coming
			 from field SW_Response_frame_length and the response frame
			 is pushed into TXPCU over the CRYPTO - TXPCU TLV interface
			
			
			When set, this feature kick in
			When clear, this feature is not enabled
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_OFFSET                0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_LSB                   29
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_MSB                   29
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_TLV_FROM_CRYPTO_MASK                  0x0000000020000000


/* Description		DOT11AX_DL_UL_FLAG

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax
			
			DL_UL_flag to be used for response frame sent to this device.
			
			
			Differentiates between DL and UL transmission 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			
			Note: this setting can also come from response look-up table
			 in TXPCU...
			The selection is SW programmable
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_DL_UL_FLAG_OFFSET                         0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_DL_UL_FLAG_LSB                            30
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_DL_UL_FLAG_MSB                            30
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_DL_UL_FLAG_MASK                           0x0000000040000000


/* Description		RESERVED_6A

			<legal 0>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_6A_OFFSET                                0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_6A_LSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_6A_MSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_6A_MASK                                  0x0000000080000000


/* Description		SW_RESPONSE_FRAME_LENGTH

			Field only valid when SW_Response_tlv_from_crypto is set
			
			
			This is the size of the frame that SW will generate as the
			 response frame. In those scenarios where TXPCU needs to
			 indicate a frame_length in the PDG_RESPONSE TLV, this will
			 be the value that TXPCU needs to use.
			
			Note that this value shall always be such that when PDG 
			calculates the LSIG duration field, the calculated value
			 is less then the max time duration that the LSIG length
			 can hold.
			
			Note that the MAX range here for 
			11ax, MCS 11, BW 180, might not be reached. But as this 
			is just for 'normal HW generated response' frames, the range
			 is size here is more then enough. 
			Also not that this field is NOT used for trigger responses.
			
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_FRAME_LENGTH_OFFSET                   0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_FRAME_LENGTH_LSB                      32
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_FRAME_LENGTH_MSB                      47
#define RX_RESPONSE_REQUIRED_INFO_SW_RESPONSE_FRAME_LENGTH_MASK                     0x0000ffff00000000


/* Description		RESPONSE_BA512_COUNT

			Field valid for both Reception_is_SU and Reception_is_MU
			
			
			For SU:
			Total number of '512 bitmap BA' responses for this one user.
			
			If this value is > 1, in implies that multi TID response
			 is needed. Also, if the sum of all the Response_ba??? Counts
			 > 1, a multi TID response is needed.
			
			For MU:
			Total number of '512 bitmap BA' responses shared between
			 all the users.
			<legal 0-36>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA512_COUNT_OFFSET                       0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA512_COUNT_LSB                          48
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA512_COUNT_MSB                          54
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA512_COUNT_MASK                         0x007f000000000000


/* Description		RESPONSE_BA1024_COUNT

			Field valid for both Reception_is_SU and Reception_is_MU
			
			
			For SU:
			Total number of '1024 bitmap BA' responses for this one 
			user.
			If this value is > 1, in implies that multi TID response
			 is needed. Also, if the sum of all the Response_ba??? Counts
			 > 1, a multi TID response is needed.
			
			For MU:
			Total number of '1024 bitmap BA' responses shared between
			 all the users.
			<legal 0-36>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA1024_COUNT_OFFSET                      0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA1024_COUNT_LSB                         55
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA1024_COUNT_MSB                         61
#define RX_RESPONSE_REQUIRED_INFO_RESPONSE_BA1024_COUNT_MASK                        0x3f80000000000000


/* Description		RESERVED_7A

			<legal 0>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_7A_OFFSET                                0x0000000000000018
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_7A_LSB                                   62
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_7A_MSB                                   63
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_7A_MASK                                  0xc000000000000000


/* Description		ADDR1_31_0

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			lower 32 bits of addr1 of the received frame
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_ADDR1_31_0_OFFSET                                 0x0000000000000020
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_31_0_LSB                                    0
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_31_0_MSB                                    31
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_31_0_MASK                                   0x00000000ffffffff


/* Description		ADDR1_47_32

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			upper 16 bits of addr1 of the received frame
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_ADDR1_47_32_OFFSET                                0x0000000000000020
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_47_32_LSB                                   32
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_47_32_MSB                                   47
#define RX_RESPONSE_REQUIRED_INFO_ADDR1_47_32_MASK                                  0x0000ffff00000000


/* Description		ADDR2_15_0

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			lower 16 bits of addr2 of the received frame
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_ADDR2_15_0_OFFSET                                 0x0000000000000020
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_15_0_LSB                                    48
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_15_0_MSB                                    63
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_15_0_MASK                                   0xffff000000000000


/* Description		ADDR2_47_16

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			upper 32 bits of addr2 of the received frame
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_ADDR2_47_16_OFFSET                                0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_47_16_LSB                                   0
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_47_16_MSB                                   31
#define RX_RESPONSE_REQUIRED_INFO_ADDR2_47_16_MASK                                  0x00000000ffffffff


/* Description		DOT11AX_RECEIVED_FORMAT_INDICATION

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax
			
			Format_Indication from the received frame.
			
			<enum 0 HE_SIGA_FORMAT_HE_TRIG>
			<enum 1 HE_SIGA_FORMAT_SU_OR_EXT_SU>
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_OFFSET         0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_LSB            32
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_MSB            32
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_FORMAT_INDICATION_MASK           0x0000000100000000


/* Description		DOT11AX_RECEIVED_DL_UL_FLAG

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax
			
			DL_UL_flag from the received frame
			
			Differentiates between DL and UL transmission 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_OFFSET                0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_LSB                   33
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_MSB                   33
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DL_UL_FLAG_MASK                  0x0000000200000000


/* Description		DOT11AX_RECEIVED_BSS_COLOR_ID

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax
			
			BSS_color_id from the received frame
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_OFFSET              0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_LSB                 34
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_MSB                 39
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_BSS_COLOR_ID_MASK                0x000000fc00000000


/* Description		DOT11AX_RECEIVED_SPATIAL_REUSE

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax
			Spatial reuse from the received frame
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_OFFSET             0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_LSB                40
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_MSB                43
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_SPATIAL_REUSE_MASK               0x00000f0000000000


/* Description		DOT11AX_RECEIVED_CP_SIZE

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax
			
			CP size of the received frame
			
			Specify the right GI for HE-Ranging NDPs (11az).
			
			<enum 0     0_8_us_sgi > Legacy normal GI. Can also be used
			 for HE
			<enum 1     0_4_us_sgi > Legacy short GI. Can also be used
			 for HE
			<enum 2     1_6_us_sgi > HE related GI
			<enum 3     3_2_us_sgi > HE related GI
			<legal 0 - 3>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CP_SIZE_OFFSET                   0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CP_SIZE_LSB                      44
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CP_SIZE_MSB                      45
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CP_SIZE_MASK                     0x0000300000000000


/* Description		DOT11AX_RECEIVED_LTF_SIZE

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax
			
			LTF size of the received frame
			
			Specify the right LTF-size for HE-Ranging NDPs (11az).
			
			<enum 0     ltf_1x > 
			<enum 1     ltf_2x > 
			<enum 2     ltf_4x > 
			<legal 0 - 2>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_LTF_SIZE_OFFSET                  0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_LTF_SIZE_LSB                     46
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_LTF_SIZE_MSB                     47
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_LTF_SIZE_MASK                    0x0000c00000000000


/* Description		DOT11AX_RECEIVED_CODING

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax
			
			Coding from the received frame
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CODING_OFFSET                    0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CODING_LSB                       48
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CODING_MSB                       48
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_CODING_MASK                      0x0001000000000000


/* Description		DOT11AX_RECEIVED_DCM

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax
			
			DCM from the received frame
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DCM_OFFSET                       0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DCM_LSB                          49
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DCM_MSB                          49
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DCM_MASK                         0x0002000000000000


/* Description		DOT11AX_RECEIVED_DOPPLER_INDICATION

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax
			
			Doppler_indication from the received frame
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_OFFSET        0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_LSB           50
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_MSB           50
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_DOPPLER_INDICATION_MASK          0x0004000000000000


/* Description		DOT11AX_RECEIVED_EXT_RU_SIZE

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax OR pkt_type
			 == 11be AND dot11ax_su_extended is set
			The number of (basic) RUs in this extended range reception
			
			
			RXPCU gets this from the received HE_SIG_A
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_OFFSET               0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_LSB                  51
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_MSB                  54
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RECEIVED_EXT_RU_SIZE_MASK                 0x0078000000000000


/* Description		FTM_FIELDS_VALID

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Field only valid in case ftm_en is set.
			
			When set, the other ftm_ fields are valid and TXCPU shall
			 use these in the response frame instead of the response
			 table based fields with a similar name.
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_FTM_FIELDS_VALID_OFFSET                           0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_FTM_FIELDS_VALID_LSB                              55
#define RX_RESPONSE_REQUIRED_INFO_FTM_FIELDS_VALID_MSB                              55
#define RX_RESPONSE_REQUIRED_INFO_FTM_FIELDS_VALID_MASK                             0x0080000000000000


/* Description		FTM_PE_NSS

			Field only valid in case ftm_fields_valid is set.
			
			Number of active spatial streams during packet extension
			 for ftm related frame exchanges
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_NSS_OFFSET                                 0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_NSS_LSB                                    56
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_NSS_MSB                                    58
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_NSS_MASK                                   0x0700000000000000


/* Description		FTM_PE_LTF_SIZE

			Field only valid in case ftm_fields_valid is set.
			
			LTF size to be used during packet extention for ftm related
			 frame exchanges.
			
			0-1x
			1-2x (unsupported un HWK-1)
			2-4x (unsupported un HWK-1) 
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_LTF_SIZE_OFFSET                            0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_LTF_SIZE_LSB                               59
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_LTF_SIZE_MSB                               60
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_LTF_SIZE_MASK                              0x1800000000000000


/* Description		FTM_PE_CONTENT

			Field only valid in case ftm_fields_valid is set.
			
			The pe content for ftm related frame exchanges.
			
			Content of packet extension. 
			
			0-he_ltf, 1-last_data_symbol
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CONTENT_OFFSET                             0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CONTENT_LSB                                61
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CONTENT_MSB                                61
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CONTENT_MASK                               0x2000000000000000


/* Description		FTM_CHAIN_CSD_EN

			Field only valid in case ftm_fields_valid is set.
			
			This field denotes whether to apply CSD on the preamble 
			and data portion of the packet. This field is valid for 
			all transmit packets
			0: disable per-chain csd
			1: enable per-chain csd 
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_FTM_CHAIN_CSD_EN_OFFSET                           0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_FTM_CHAIN_CSD_EN_LSB                              62
#define RX_RESPONSE_REQUIRED_INFO_FTM_CHAIN_CSD_EN_MSB                              62
#define RX_RESPONSE_REQUIRED_INFO_FTM_CHAIN_CSD_EN_MASK                             0x4000000000000000


/* Description		FTM_PE_CHAIN_CSD_EN

			Field only valid in case ftm_fields_valid is set.
			
			This field denotes whether to apply CSD on the preamble 
			and data portion of the packet. This field is valid for 
			all transmit packets
			0: disable per-chain csd
			1: enable per-chain csd 
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CHAIN_CSD_EN_OFFSET                        0x0000000000000028
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CHAIN_CSD_EN_LSB                           63
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CHAIN_CSD_EN_MSB                           63
#define RX_RESPONSE_REQUIRED_INFO_FTM_PE_CHAIN_CSD_EN_MASK                          0x8000000000000000


/* Description		DOT11AX_RESPONSE_RATE_SOURCE

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Valid for response generation to an 11ax pkt_type received
			 frame, but NOT 11ax extended pkt_type of frame 
			
			When set to 0, use the register based lookup for determining
			 the 11ax response rates.
			
			When > 0, TXPCU shall use this response table index for 
			the 20 MHz response, and higher BW responses are in the 
			subsequent response table entries
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RESPONSE_RATE_SOURCE_OFFSET               0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RESPONSE_RATE_SOURCE_LSB                  0
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RESPONSE_RATE_SOURCE_MSB                  7
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_RESPONSE_RATE_SOURCE_MASK                 0x00000000000000ff


/* Description		DOT11AX_EXT_RESPONSE_RATE_SOURCE

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			This field is only valid for pkt_type == 11ax and dot11ax_su_extended
			 is set
			
			When set to 0, the response rates are based on the 11ax 
			extended response register based indexes in TXPCU.
			
			When > 0, TXPCU shall use this response table index for 
			the response to a 1RU reception. Higher RU count reception
			 responses can be found in the subsequent response table
			 entries: Next entry is for 2 RU receptions,  then 4 RU 
			receptions,  then >= 8 RU receptions...
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_EXT_RESPONSE_RATE_SOURCE_OFFSET           0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_EXT_RESPONSE_RATE_SOURCE_LSB              8
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_EXT_RESPONSE_RATE_SOURCE_MSB              15
#define RX_RESPONSE_REQUIRED_INFO_DOT11AX_EXT_RESPONSE_RATE_SOURCE_MASK             0x000000000000ff00


/* Description		SW_PEER_ID

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			An identifier indicating for which device this response 
			is needed.
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_SW_PEER_ID_OFFSET                                 0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_SW_PEER_ID_LSB                                    16
#define RX_RESPONSE_REQUIRED_INFO_SW_PEER_ID_MSB                                    31
#define RX_RESPONSE_REQUIRED_INFO_SW_PEER_ID_MASK                                   0x00000000ffff0000


/* Description		DOT11BE_PUNCTURE_BITMAP

			This field is only valid if Punctured_response is set
			
			The bitmap of 20 MHz sub-bands valid in this EHT reception
			
			
			RXPCU gets this from the received U-SIG and/or EHT-SIG via
			 PHY microcode.
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_PUNCTURE_BITMAP_OFFSET                    0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_PUNCTURE_BITMAP_LSB                       32
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_PUNCTURE_BITMAP_MSB                       47
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_PUNCTURE_BITMAP_MASK                      0x0000ffff00000000


/* Description		DOT11BE_RESPONSE

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU
			
			Indicates that the peer supports .11be response protocols, 
			e.g. .11be BW indication in scrambler seed, .11be dynamic
			 BW procedure, punctured response, etc.
*/

#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_RESPONSE_OFFSET                           0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_RESPONSE_LSB                              48
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_RESPONSE_MSB                              48
#define RX_RESPONSE_REQUIRED_INFO_DOT11BE_RESPONSE_MASK                             0x0001000000000000


/* Description		PUNCTURED_RESPONSE

			Field only valid if Dot11be_response is set
			
			Indicates that the response shall use preamble puncturing
			
*/

#define RX_RESPONSE_REQUIRED_INFO_PUNCTURED_RESPONSE_OFFSET                         0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_PUNCTURED_RESPONSE_LSB                            49
#define RX_RESPONSE_REQUIRED_INFO_PUNCTURED_RESPONSE_MSB                            49
#define RX_RESPONSE_REQUIRED_INFO_PUNCTURED_RESPONSE_MASK                           0x0002000000000000


/* Description		EHT_DUPLICATE_MODE

			Field only valid in case of SU_or_uplink_MU_reception = 
			Reception_is_SU for pkt_type == 11be
			
			Indicates EHT duplicate modulation
			
			<enum 0 eht_no_duplicate>
			<enum 1 eht_2x_duplicate>
			<enum 2 eht_4x_duplicate>
			
			<legal 0-2>
*/

#define RX_RESPONSE_REQUIRED_INFO_EHT_DUPLICATE_MODE_OFFSET                         0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_EHT_DUPLICATE_MODE_LSB                            50
#define RX_RESPONSE_REQUIRED_INFO_EHT_DUPLICATE_MODE_MSB                            51
#define RX_RESPONSE_REQUIRED_INFO_EHT_DUPLICATE_MODE_MASK                           0x000c000000000000


/* Description		FORCE_EXTRA_SYMBOL

			Set to 1 to force an extra OFDM symbol (or symbols) even
			 if the PPDU encoding process does not result in an extra
			 OFDM symbol (or symbols)
*/

#define RX_RESPONSE_REQUIRED_INFO_FORCE_EXTRA_SYMBOL_OFFSET                         0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_FORCE_EXTRA_SYMBOL_LSB                            52
#define RX_RESPONSE_REQUIRED_INFO_FORCE_EXTRA_SYMBOL_MSB                            52
#define RX_RESPONSE_REQUIRED_INFO_FORCE_EXTRA_SYMBOL_MASK                           0x0010000000000000


/* Description		RESERVED_13A

			<legal 0>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_13A_OFFSET                               0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_13A_LSB                                  53
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_13A_MSB                                  57
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_13A_MASK                                 0x03e0000000000000


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			This field is only valid if Punctured_response is set
			
			The 6-bit value used in U-SIG and/or EHT-SIG Common field
			 for the puncture pattern
			<legal 0-29>
*/

#define RX_RESPONSE_REQUIRED_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET            0x0000000000000030
#define RX_RESPONSE_REQUIRED_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB               58
#define RX_RESPONSE_REQUIRED_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB               63
#define RX_RESPONSE_REQUIRED_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK              0xfc00000000000000


/* Description		MLO_STA_ID_DETAILS_RX

			Bits 10 and 11 are not valid, bits [9:0] reflect 'NSTR_MLO_STA_ID' 
			from address search.
			
			
*/


/* Description		NSTR_MLO_STA_ID

			ID of peer participating in non-STR MLO
*/

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_OFFSET      0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_LSB         0
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MSB         9
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_MASK        0x00000000000003ff


/* Description		BLOCK_SELF_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for self-link.
*/

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_OFFSET   0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_LSB      10
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MSB      10
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_SELF_ML_SYNC_MASK     0x0000000000000400


/* Description		BLOCK_PARTNER_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for partner links.
*/

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_OFFSET 0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_LSB   11
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MSB   11
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_BLOCK_PARTNER_ML_SYNC_MASK  0x0000000000000800


/* Description		NSTR_MLO_STA_ID_VALID

			All the fields in this TLV are valid only if this bit is
			 set.
*/

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_OFFSET 0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_LSB   12
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MSB   12
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_NSTR_MLO_STA_ID_VALID_MASK  0x0000000000001000


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_OFFSET          0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_LSB             13
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MSB             15
#define RX_RESPONSE_REQUIRED_INFO_MLO_STA_ID_DETAILS_RX_RESERVED_0A_MASK            0x000000000000e000


/* Description		HE_A_CONTROL_RESPONSE_TIME

			When non-zero, indicates the value from an HE A-Control 
			in the received frame requiring a specific response time
			 (e.g. for sync MLO)
			
			<legal all>
*/

#define RX_RESPONSE_REQUIRED_INFO_HE_A_CONTROL_RESPONSE_TIME_OFFSET                 0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_HE_A_CONTROL_RESPONSE_TIME_LSB                    16
#define RX_RESPONSE_REQUIRED_INFO_HE_A_CONTROL_RESPONSE_TIME_MSB                    27
#define RX_RESPONSE_REQUIRED_INFO_HE_A_CONTROL_RESPONSE_TIME_MASK                   0x000000000fff0000


/* Description		RESERVED_AFTER_STRUCT16

			<legal 0>
*/

#define RX_RESPONSE_REQUIRED_INFO_RESERVED_AFTER_STRUCT16_OFFSET                    0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_AFTER_STRUCT16_LSB                       28
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_AFTER_STRUCT16_MSB                       31
#define RX_RESPONSE_REQUIRED_INFO_RESERVED_AFTER_STRUCT16_MASK                      0x00000000f0000000


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define RX_RESPONSE_REQUIRED_INFO_TLV64_PADDING_OFFSET                              0x0000000000000038
#define RX_RESPONSE_REQUIRED_INFO_TLV64_PADDING_LSB                                 32
#define RX_RESPONSE_REQUIRED_INFO_TLV64_PADDING_MSB                                 63
#define RX_RESPONSE_REQUIRED_INFO_TLV64_PADDING_MASK                                0xffffffff00000000



#endif   // RX_RESPONSE_REQUIRED_INFO
