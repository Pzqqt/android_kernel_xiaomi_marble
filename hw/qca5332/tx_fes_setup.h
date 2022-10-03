
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

 
 
 
 
 
 
 


#ifndef _TX_FES_SETUP_H_
#define _TX_FES_SETUP_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_FES_SETUP 10

#define NUM_OF_QWORDS_TX_FES_SETUP 5


struct tx_fes_setup {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t schedule_id                                             : 32; // [31:0]
             uint32_t fes_in_11ax_trigger_response_config                     :  1, // [0:0]
                      bo_based_tid_aggregation_limit                          :  4, // [4:1]
                      ranging                                                 :  1, // [5:5]
                      expect_i2r_lmr                                          :  1, // [6:6]
                      transmit_start_reason                                   :  3, // [9:7]
                      use_alt_power_sr                                        :  1, // [10:10]
                      static_2_pwr_mode_status                                :  1, // [11:11]
                      obss_srg_opport_transmit_status                         :  1, // [12:12]
                      srp_based_transmit_status                               :  1, // [13:13]
                      obss_pd_based_transmit_status                           :  1, // [14:14]
                      puncture_from_all_allowed_modes                         :  1, // [15:15]
                      schedule_cmd_ring_id                                    :  5, // [20:16]
                      fes_control_mode                                        :  2, // [22:21]
                      number_of_users                                         :  6, // [28:23]
                      mu_type                                                 :  1, // [29:29]
                      ofdma_triggered_response                                :  1, // [30:30]
                      response_to_response_cmd                                :  1; // [31:31]
             uint32_t schedule_try                                            :  4, // [3:0]
                      ndp_frame                                               :  2, // [5:4]
                      txbf                                                    :  1, // [6:6]
                      allow_txop_exceed_in_1st_pkt                            :  1, // [7:7]
                      ignore_bw_available                                     :  1, // [8:8]
                      ignore_tbtt                                             :  1, // [9:9]
                      static_bandwidth                                        :  3, // [12:10]
                      set_txop_duration_all_ones                              :  1, // [13:13]
                      transmission_contains_mu_rts                            :  1, // [14:14]
                      bw_restricted_frames_embedded                           :  1, // [15:15]
                      ast_index                                               : 16; // [31:16]
             uint32_t cv_id                                                   :  8, // [7:0]
                      trigger_resp_txpdu_ppdu_boundary                        :  2, // [9:8]
                      rxpcu_setup_complete_present                            :  1, // [10:10]
                      rbo_must_have_data_user_limit                           :  4, // [14:11]
                      mu_ndp                                                  :  1, // [15:15]
                      bf_type                                                 :  2, // [17:16]
                      cbf_nc_index_mask                                       :  1, // [18:18]
                      cbf_nc_index                                            :  3, // [21:19]
                      cbf_nr_index_mask                                       :  1, // [22:22]
                      cbf_nr_index                                            :  3, // [25:23]
                      secure_ranging_ista                                     :  1, // [26:26]
                      ndpa                                                    :  1, // [27:27]
                      wait_sifs                                               :  2, // [29:28]
                      cbf_feedback_type_mask                                  :  1, // [30:30]
                      cbf_feedback_type                                       :  1; // [31:31]
             uint32_t cbf_sounding_token                                      :  6, // [5:0]
                      cbf_sounding_token_mask                                 :  1, // [6:6]
                      cbf_bw_mask                                             :  1, // [7:7]
                      cbf_bw                                                  :  3, // [10:8]
                      use_static_bw                                           :  1, // [11:11]
                      coex_nack_count                                         :  5, // [16:12]
                      sch_tx_burst_ongoing                                    :  1, // [17:17]
                      gen_tqm_update_mpdu_count_tlv                           :  1, // [18:18]
                      transmit_vif                                            :  4, // [22:19]
                      optimal_bw_retry_count                                  :  4, // [26:23]
                      fes_continuation_ratio_threshold                        :  5; // [31:27]
             uint32_t transmit_cca_bitmap                                     : 32; // [31:0]
             uint32_t tb_ranging                                              :  1, // [0:0]
                      ranging_trigger_subtype                                 :  4, // [4:1]
                      min_cts2self_count                                      :  4, // [8:5]
                      max_cts2self_count                                      :  4, // [12:9]
                      wifi_radar_enable                                       :  1, // [13:13]
                      reserved_6a                                             : 18; // [31:14]
             uint32_t monitor_override_sta_31_0                               : 32; // [31:0]
             uint32_t monitor_override_sta_36_32                              :  5, // [4:0]
                      reserved_8a                                             : 27; // [31:5]
             uint32_t fw2sw_info                                              : 32; // [31:0]
#else
             uint32_t schedule_id                                             : 32; // [31:0]
             uint32_t response_to_response_cmd                                :  1, // [31:31]
                      ofdma_triggered_response                                :  1, // [30:30]
                      mu_type                                                 :  1, // [29:29]
                      number_of_users                                         :  6, // [28:23]
                      fes_control_mode                                        :  2, // [22:21]
                      schedule_cmd_ring_id                                    :  5, // [20:16]
                      puncture_from_all_allowed_modes                         :  1, // [15:15]
                      obss_pd_based_transmit_status                           :  1, // [14:14]
                      srp_based_transmit_status                               :  1, // [13:13]
                      obss_srg_opport_transmit_status                         :  1, // [12:12]
                      static_2_pwr_mode_status                                :  1, // [11:11]
                      use_alt_power_sr                                        :  1, // [10:10]
                      transmit_start_reason                                   :  3, // [9:7]
                      expect_i2r_lmr                                          :  1, // [6:6]
                      ranging                                                 :  1, // [5:5]
                      bo_based_tid_aggregation_limit                          :  4, // [4:1]
                      fes_in_11ax_trigger_response_config                     :  1; // [0:0]
             uint32_t ast_index                                               : 16, // [31:16]
                      bw_restricted_frames_embedded                           :  1, // [15:15]
                      transmission_contains_mu_rts                            :  1, // [14:14]
                      set_txop_duration_all_ones                              :  1, // [13:13]
                      static_bandwidth                                        :  3, // [12:10]
                      ignore_tbtt                                             :  1, // [9:9]
                      ignore_bw_available                                     :  1, // [8:8]
                      allow_txop_exceed_in_1st_pkt                            :  1, // [7:7]
                      txbf                                                    :  1, // [6:6]
                      ndp_frame                                               :  2, // [5:4]
                      schedule_try                                            :  4; // [3:0]
             uint32_t cbf_feedback_type                                       :  1, // [31:31]
                      cbf_feedback_type_mask                                  :  1, // [30:30]
                      wait_sifs                                               :  2, // [29:28]
                      ndpa                                                    :  1, // [27:27]
                      secure_ranging_ista                                     :  1, // [26:26]
                      cbf_nr_index                                            :  3, // [25:23]
                      cbf_nr_index_mask                                       :  1, // [22:22]
                      cbf_nc_index                                            :  3, // [21:19]
                      cbf_nc_index_mask                                       :  1, // [18:18]
                      bf_type                                                 :  2, // [17:16]
                      mu_ndp                                                  :  1, // [15:15]
                      rbo_must_have_data_user_limit                           :  4, // [14:11]
                      rxpcu_setup_complete_present                            :  1, // [10:10]
                      trigger_resp_txpdu_ppdu_boundary                        :  2, // [9:8]
                      cv_id                                                   :  8; // [7:0]
             uint32_t fes_continuation_ratio_threshold                        :  5, // [31:27]
                      optimal_bw_retry_count                                  :  4, // [26:23]
                      transmit_vif                                            :  4, // [22:19]
                      gen_tqm_update_mpdu_count_tlv                           :  1, // [18:18]
                      sch_tx_burst_ongoing                                    :  1, // [17:17]
                      coex_nack_count                                         :  5, // [16:12]
                      use_static_bw                                           :  1, // [11:11]
                      cbf_bw                                                  :  3, // [10:8]
                      cbf_bw_mask                                             :  1, // [7:7]
                      cbf_sounding_token_mask                                 :  1, // [6:6]
                      cbf_sounding_token                                      :  6; // [5:0]
             uint32_t transmit_cca_bitmap                                     : 32; // [31:0]
             uint32_t reserved_6a                                             : 18, // [31:14]
                      wifi_radar_enable                                       :  1, // [13:13]
                      max_cts2self_count                                      :  4, // [12:9]
                      min_cts2self_count                                      :  4, // [8:5]
                      ranging_trigger_subtype                                 :  4, // [4:1]
                      tb_ranging                                              :  1; // [0:0]
             uint32_t monitor_override_sta_31_0                               : 32; // [31:0]
             uint32_t reserved_8a                                             : 27, // [31:5]
                      monitor_override_sta_36_32                              :  5; // [4:0]
             uint32_t fw2sw_info                                              : 32; // [31:0]
#endif
};


/* Description		SCHEDULE_ID

			Consumer: PDG/TXDMA/TXOLE/TXCRYPTO/TXPCU
			Producer: SCH
			
			This field is overwritten by the scheduler module and it's
			 value is coming from the"schedule_id" field in the  Scheduler
			 command.
			
			Configured by scheduler in HW transmit mode
			A field that HW copies over into the scheduling status report, 
			so that SW can determine to which scheduler command the 
			status report belongs.
			This schedule ID is also reported in the PPDU status.   
			    
			<legal all>
*/

#define TX_FES_SETUP_SCHEDULE_ID_OFFSET                                             0x0000000000000000
#define TX_FES_SETUP_SCHEDULE_ID_LSB                                                0
#define TX_FES_SETUP_SCHEDULE_ID_MSB                                                31
#define TX_FES_SETUP_SCHEDULE_ID_MASK                                               0x00000000ffffffff


/* Description		FES_IN_11AX_TRIGGER_RESPONSE_CONFIG

			Consumer: PDG/TXPCU
			Producer: SW
			When set, this scheduler command has some additional settings
			 that PDG and TXPCU need to take into account, depending
			 on if the transmission has been iniated as a backoff expiration
			 or as the result of an 11ax trigger reception.
			
			0: not in special trigger response config
			1: command is special trigger response config.
			
			When set to 1, there are some programming limitations: There
			 can only be 1 group, up to 8 users, SW shall have specified
			 the AC for each user, and AC order per user is from BE 
			to VO
			(see PDG_USER_SETUP, fields Triggered_mpdu_AC_category)
			
			<legal all>
*/

#define TX_FES_SETUP_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_OFFSET                     0x0000000000000000
#define TX_FES_SETUP_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_LSB                        32
#define TX_FES_SETUP_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_MSB                        32
#define TX_FES_SETUP_FES_IN_11AX_TRIGGER_RESPONSE_CONFIG_MASK                       0x0000000100000000


/* Description		BO_BASED_TID_AGGREGATION_LIMIT

			Consumer: PDG
			Producer: SW
			
			Field only valid when Ofdma_triggered_response is NOT set
			 (=> implies transmission started due to backoff expiration)
			
			
			Field only valid for SU and "MU_SU" transmissions.
			
			The requirements for what to transmit depend on what the
			 reason is that this transmission started. If it is 11ax
			 trigger based, the trigger frame will specify all the constrains
			 like max TID count, prefered AC, etc.
			However if this command starts executing due to backoff 
			expiration, the requirements could be different from those
			 that might have come from the trigger frame.
			This field specifies what the constaints are when the transmission
			 is Backoff initiated.
			
			If zero, this feature is disabled.
			If non-zero, this indicates the number of users within a
			 group that can be aggregated by a STA in a multi-TID A-MPDU. 
			This can also be used to block the series of QoS-null MPDUs
			 when an RBO+Trig queue transmits using RBO.
			
			Based on this number, PDG will mask of user numbers >= this
			 count
			<legal all>
*/

#define TX_FES_SETUP_BO_BASED_TID_AGGREGATION_LIMIT_OFFSET                          0x0000000000000000
#define TX_FES_SETUP_BO_BASED_TID_AGGREGATION_LIMIT_LSB                             33
#define TX_FES_SETUP_BO_BASED_TID_AGGREGATION_LIMIT_MSB                             36
#define TX_FES_SETUP_BO_BASED_TID_AGGREGATION_LIMIT_MASK                            0x0000001e00000000


/* Description		RANGING

			Consumer: TXPCU
			Producer: SW
			
			Set to 1 in case the frame queued is:
			a .11az ranging NDPA,
			a .11az ranging NDP, or
			an ISTA2RSTA LMR.
			Set to 0 for all other cases.
*/

#define TX_FES_SETUP_RANGING_OFFSET                                                 0x0000000000000000
#define TX_FES_SETUP_RANGING_LSB                                                    37
#define TX_FES_SETUP_RANGING_MSB                                                    37
#define TX_FES_SETUP_RANGING_MASK                                                   0x0000002000000000


/* Description		EXPECT_I2R_LMR

			Consumer: TXPCU
			Producer: SW
			
			Set to 1 in case the frame queued is  a .11az randing NDPA/NDP
			 and if the ISTA2RSTA LMR frame is also queued after SIFS.
			
			
			Set to 0 otherwise.
*/

#define TX_FES_SETUP_EXPECT_I2R_LMR_OFFSET                                          0x0000000000000000
#define TX_FES_SETUP_EXPECT_I2R_LMR_LSB                                             38
#define TX_FES_SETUP_EXPECT_I2R_LMR_MSB                                             38
#define TX_FES_SETUP_EXPECT_I2R_LMR_MASK                                            0x0000004000000000


/* Description		TRANSMIT_START_REASON

			Indicates what the SCH start reason reason was for initiating
			 this transmission.
			
			<enum 0 BO_based_transmit_start> The transmission of this
			 PPDU got initiated by the scheduler due to Backoff expiration
			
			<enum 1 Trigger_based_transmit_start> The transmission of
			 this PPDU got initiated by the scheduler due to reception
			 (by the SCH) of the TLV RECEIVED_TRIGGER_INFO that RXPCU
			 generated. Note that this can be an OFDMA trigger frame
			 based transmission as well as some legacy trigger (PS-POLL, 
			Qboost, U-APSD, etc.)  based transmission
			<enum 2 Sifs_continuation_in_ongoing_burst> This transmission
			 of this PPDU got initiated as part of SIFS continuation. 
			An earlier PPDU was transmitted due to RBO expiration. Next
			 command is also expected to be transmitted in SIFS burst.
			
			<enum 3 Sifs_continuation_last_command> This transmission
			 of this PPDU got initiated as part of SIFS continuation
			 and this is the last command in the burst. An earlier PPDU
			 was transmitted due to RBO expiration.
			<enum 4 NTBR_response_start> DO NOT USE
			<legal 0-4>
*/

#define TX_FES_SETUP_TRANSMIT_START_REASON_OFFSET                                   0x0000000000000000
#define TX_FES_SETUP_TRANSMIT_START_REASON_LSB                                      39
#define TX_FES_SETUP_TRANSMIT_START_REASON_MSB                                      41
#define TX_FES_SETUP_TRANSMIT_START_REASON_MASK                                     0x0000038000000000


/* Description		USE_ALT_POWER_SR

			0: Primary/default power1: Alternate power
			<legal all>
*/

#define TX_FES_SETUP_USE_ALT_POWER_SR_OFFSET                                        0x0000000000000000
#define TX_FES_SETUP_USE_ALT_POWER_SR_LSB                                           42
#define TX_FES_SETUP_USE_ALT_POWER_SR_MSB                                           42
#define TX_FES_SETUP_USE_ALT_POWER_SR_MASK                                          0x0000040000000000


/* Description		STATIC_2_PWR_MODE_STATUS

			0: Static 2 power mode disabled1: Static 2 power mode enabled
			
			<legal all>
*/

#define TX_FES_SETUP_STATIC_2_PWR_MODE_STATUS_OFFSET                                0x0000000000000000
#define TX_FES_SETUP_STATIC_2_PWR_MODE_STATUS_LSB                                   43
#define TX_FES_SETUP_STATIC_2_PWR_MODE_STATUS_MSB                                   43
#define TX_FES_SETUP_STATIC_2_PWR_MODE_STATUS_MASK                                  0x0000080000000000


/* Description		OBSS_SRG_OPPORT_TRANSMIT_STATUS

			0: Transmit based on SRG OBSS_PD opportunity initiated1: 
			Transmit based on non-SRG OBSS_PD opportunity initiated
			<legal all>
*/

#define TX_FES_SETUP_OBSS_SRG_OPPORT_TRANSMIT_STATUS_OFFSET                         0x0000000000000000
#define TX_FES_SETUP_OBSS_SRG_OPPORT_TRANSMIT_STATUS_LSB                            44
#define TX_FES_SETUP_OBSS_SRG_OPPORT_TRANSMIT_STATUS_MSB                            44
#define TX_FES_SETUP_OBSS_SRG_OPPORT_TRANSMIT_STATUS_MASK                           0x0000100000000000


/* Description		SRP_BASED_TRANSMIT_STATUS

			0: non-SRP based transmit initiated1: SRP based transmit
			 initiated
			<legal all>
*/

#define TX_FES_SETUP_SRP_BASED_TRANSMIT_STATUS_OFFSET                               0x0000000000000000
#define TX_FES_SETUP_SRP_BASED_TRANSMIT_STATUS_LSB                                  45
#define TX_FES_SETUP_SRP_BASED_TRANSMIT_STATUS_MSB                                  45
#define TX_FES_SETUP_SRP_BASED_TRANSMIT_STATUS_MASK                                 0x0000200000000000


/* Description		OBSS_PD_BASED_TRANSMIT_STATUS

			0: non-OBSS_PD based transmit initiated1: obss_pd based 
			transmit initiated
			<legal all>
*/

#define TX_FES_SETUP_OBSS_PD_BASED_TRANSMIT_STATUS_OFFSET                           0x0000000000000000
#define TX_FES_SETUP_OBSS_PD_BASED_TRANSMIT_STATUS_LSB                              46
#define TX_FES_SETUP_OBSS_PD_BASED_TRANSMIT_STATUS_MSB                              46
#define TX_FES_SETUP_OBSS_PD_BASED_TRANSMIT_STATUS_MASK                             0x0000400000000000


/* Description		PUNCTURE_FROM_ALL_ALLOWED_MODES

			Enables new scheme 2 puncturing in Beryllium:
			TXPCU registers determine which puncture patterns (up to
			 37) are enabled for the transmission.
			'TX_PUNCTURE_SETUP' is unused.
			<legal all>
*/

#define TX_FES_SETUP_PUNCTURE_FROM_ALL_ALLOWED_MODES_OFFSET                         0x0000000000000000
#define TX_FES_SETUP_PUNCTURE_FROM_ALL_ALLOWED_MODES_LSB                            47
#define TX_FES_SETUP_PUNCTURE_FROM_ALL_ALLOWED_MODES_MSB                            47
#define TX_FES_SETUP_PUNCTURE_FROM_ALL_ALLOWED_MODES_MASK                           0x0000800000000000


/* Description		SCHEDULE_CMD_RING_ID

			Consumer: PDG/TXDMA/TXOLE/TXCRYPTO/TXPCU
			Producer: SCH
			
			This field is overwritten by the scheduler module and its
			 value is based on the scheduler ring where the command 
			is initiated.
			
			The schedule command ring  that originated this transmission
			
			<enum 0 sch_cmd_ring_number0>
			<enum 1 sch_cmd_ring_number1>
			<enum 2 sch_cmd_ring_number2>
			<enum 3 sch_cmd_ring_number3>
			<enum 4 sch_cmd_ring_number4>
			<enum 5 sch_cmd_ring_number5>
			<enum 6 sch_cmd_ring_number6>
			<enum 7 sch_cmd_ring_number7>
			<enum 8 sch_cmd_ring_number8>
			<enum 9 sch_cmd_ring_number9>
			<enum 10 sch_cmd_ring_number10>
			<enum 11 sch_cmd_ring_number11>
			<enum 12 sch_cmd_ring_number12>
			<enum 13 sch_cmd_ring_number13>
			<enum 14 sch_cmd_ring_number14>
			<enum 15 sch_cmd_ring_number15>
			<enum 16 sch_cmd_ring_number16>
			<enum 17 sch_cmd_ring_number17>
			<enum 18 sch_cmd_ring_number18>
			<enum 19 sch_cmd_ring_number19>
			<enum 20 sch_cmd_ring_number20>
			
			<legal 0-20>
*/

#define TX_FES_SETUP_SCHEDULE_CMD_RING_ID_OFFSET                                    0x0000000000000000
#define TX_FES_SETUP_SCHEDULE_CMD_RING_ID_LSB                                       48
#define TX_FES_SETUP_SCHEDULE_CMD_RING_ID_MSB                                       52
#define TX_FES_SETUP_SCHEDULE_CMD_RING_ID_MASK                                      0x001f000000000000


/* Description		FES_CONTROL_MODE

			Consumer: PDG/TXDMA/TXOLE/TXCRYPTO/TXPCU
			Producer: SCH
			
			This field is overwritten by the scheduler module and it's
			 value is coming from the "FES_control_mode" field in the
			  Scheduler command.
			
			<enum 0  SW_transmit_mode>  No HW generated TLVs
			<enum 1 PDG_transmit_mode> PDG  is activated to generate
			 TLVs
			
			Note: Final Bandwidth selection is always performed by TX
			 PCU.
			<legal 0-1> 
*/

#define TX_FES_SETUP_FES_CONTROL_MODE_OFFSET                                        0x0000000000000000
#define TX_FES_SETUP_FES_CONTROL_MODE_LSB                                           53
#define TX_FES_SETUP_FES_CONTROL_MODE_MSB                                           54
#define TX_FES_SETUP_FES_CONTROL_MODE_MASK                                          0x0060000000000000


/* Description		NUMBER_OF_USERS

			Consumer: PDG/TXDMA/TXOLE/TXCRYPTO/TXPCU
			Producer: SCH
			
			The number of users in this transmission. Can be MU-MIMO
			 or OFDMA in case the number is > 1
			<legal 1-63> 
*/

#define TX_FES_SETUP_NUMBER_OF_USERS_OFFSET                                         0x0000000000000000
#define TX_FES_SETUP_NUMBER_OF_USERS_LSB                                            55
#define TX_FES_SETUP_NUMBER_OF_USERS_MSB                                            60
#define TX_FES_SETUP_NUMBER_OF_USERS_MASK                                           0x1f80000000000000


/* Description		MU_TYPE

			In case the Number_of_users > 1, the transmission could 
			be MU or OFDMA.
			This field indicates which one it is.
			
			0: MU-MIMO
			1: OFDMA
			
			
			In case the number_of_user == 1, and PDG_FES_SETUP.mu_su_transmission
			 is set, this field indicates:0: SU transmitted in MU MIMO
			 format in compressed mode;1: SU transmitted in MU-OFDMA
			 format in uncompressed mode
			
			Note: Within OFDMA classification, it could be that within
			 one or more RUs there will be MIMO transmission...This 
			is still considered as an 'OFDMA' class of MU transmission.
			
			
			<legal all>
*/

#define TX_FES_SETUP_MU_TYPE_OFFSET                                                 0x0000000000000000
#define TX_FES_SETUP_MU_TYPE_LSB                                                    61
#define TX_FES_SETUP_MU_TYPE_MSB                                                    61
#define TX_FES_SETUP_MU_TYPE_MASK                                                   0x2000000000000000


/* Description		OFDMA_TRIGGERED_RESPONSE

			Consumer: TXPCU/PDG
			Producer: SCH/SW
			
			SW should always set this bit to 0
			SCH will always overwrite this field and set it to the appropriate
			 value for the upcoming transmission.
			
			When set (by SCH), this FES is initiated as a result of 
			receiving an OFDMA transmit trigger. PDG already has received
			 all transmit info from RXPCU. PDG can ignore most of the
			 transmit initialization info.
			
			<legal all>
*/

#define TX_FES_SETUP_OFDMA_TRIGGERED_RESPONSE_OFFSET                                0x0000000000000000
#define TX_FES_SETUP_OFDMA_TRIGGERED_RESPONSE_LSB                                   62
#define TX_FES_SETUP_OFDMA_TRIGGERED_RESPONSE_MSB                                   62
#define TX_FES_SETUP_OFDMA_TRIGGERED_RESPONSE_MASK                                  0x4000000000000000


/* Description		RESPONSE_TO_RESPONSE_CMD

			When set, this scheduler command contains the transmission
			 control for the response_to_response transmission
			<legal all>
*/

#define TX_FES_SETUP_RESPONSE_TO_RESPONSE_CMD_OFFSET                                0x0000000000000000
#define TX_FES_SETUP_RESPONSE_TO_RESPONSE_CMD_LSB                                   63
#define TX_FES_SETUP_RESPONSE_TO_RESPONSE_CMD_MSB                                   63
#define TX_FES_SETUP_RESPONSE_TO_RESPONSE_CMD_MASK                                  0x8000000000000000


/* Description		SCHEDULE_TRY

			Consumer: TXPCU
			Producer: SCH
			
			This field is overwritten by the scheduler module and it's
			 value is coming from an internal counter in the scheduler
			 that keeps track of how many times a scheduling command
			 has been tried. 
			
			This count indicates how many times the FES did not successfully
			 complete as the ACK/BA frame did not get received.
			<legal all>
*/

#define TX_FES_SETUP_SCHEDULE_TRY_OFFSET                                            0x0000000000000008
#define TX_FES_SETUP_SCHEDULE_TRY_LSB                                               0
#define TX_FES_SETUP_SCHEDULE_TRY_MSB                                               3
#define TX_FES_SETUP_SCHEDULE_TRY_MASK                                              0x000000000000000f


/* Description		NDP_FRAME

			Consumer: PDG/TXPCU
			Producer: SCH
			
			When set, the scheduling command contains an NDP frame. 
			This can only be done using the SW transmit mode.
			
			<enum 0 no_ndp>No NDP transmission
			<enum 1 beamforming_ndp>Beamforming NDP
			<enum 2 he_ranging_ndp>11az NDP (HE Ranging NDP)
			<enum 3 he_feedback_ndp>Short TB (HE Feedback NDP)
*/

#define TX_FES_SETUP_NDP_FRAME_OFFSET                                               0x0000000000000008
#define TX_FES_SETUP_NDP_FRAME_LSB                                                  4
#define TX_FES_SETUP_NDP_FRAME_MSB                                                  5
#define TX_FES_SETUP_NDP_FRAME_MASK                                                 0x0000000000000030


/* Description		TXBF

			Consumer: PDG/TXPCU
			Producer: SCH
			
			If set, this bit indicates that this is a TX beamformed 
			SU transaction or MU transaction
			
			
			In case of a beamformed transmission, note that in the PCU_PPDU_SETUP_INIT
			 TLV, SW can narrow down for which of the BW the beamforming
			 shall take place. For example, SW can decide that BW is
			 only desired for 40MHz BW, but not for 20...
			If for any of the allowed BW, beamforming is desired, this
			 field should be set, and the 'bf_type' shall be properly
			 programmed. 
			
			TXPCU controls with bit 'beamforming' in the MACTX_PRE_PHY_DESC
			 if the final actual transmission shall be beamformed.
*/

#define TX_FES_SETUP_TXBF_OFFSET                                                    0x0000000000000008
#define TX_FES_SETUP_TXBF_LSB                                                       6
#define TX_FES_SETUP_TXBF_MSB                                                       6
#define TX_FES_SETUP_TXBF_MASK                                                      0x0000000000000040


/* Description		ALLOW_TXOP_EXCEED_IN_1ST_PKT

			Consumer: PDG
			Producer: SCH
			
			Field only valid for SU transmissions.
			
			When set, a single MPDU transmission after RBO is allowed
			 to exceed TXOP. In this setting, this field has priority
			 over the setting of the duration_field_boundary. Reason
			 for this is that if Coex issues on the receiver STA start
			 preventing the transmission of frames on this device, it
			 can lead to a death spiral. With some luck, this frame 
			although maybe too long, might still be received.
			
			When 0, single MPDU after RBO is not allowed to exceed TXOP. 
			
			<legal all> 
*/

#define TX_FES_SETUP_ALLOW_TXOP_EXCEED_IN_1ST_PKT_OFFSET                            0x0000000000000008
#define TX_FES_SETUP_ALLOW_TXOP_EXCEED_IN_1ST_PKT_LSB                               7
#define TX_FES_SETUP_ALLOW_TXOP_EXCEED_IN_1ST_PKT_MSB                               7
#define TX_FES_SETUP_ALLOW_TXOP_EXCEED_IN_1ST_PKT_MASK                              0x0000000000000080


/* Description		IGNORE_BW_AVAILABLE

			Consumer: TXPCU
			Producer: SCH
			
			If set, TXPCU ignores 'BW available signals' from the scheduler
			 and transmit using the single BW that SW has programmed
			 the transmission to go out in. This bit should be set for
			 SIFS response frame to PS-Poll/uAPSD/QBoost and note that
			 for this mode, SW is only allowed to program a single transmit
			 BW.
			Also note that this bit can not be set in combination with
			 preamble puncturing.
			<legal all>
*/

#define TX_FES_SETUP_IGNORE_BW_AVAILABLE_OFFSET                                     0x0000000000000008
#define TX_FES_SETUP_IGNORE_BW_AVAILABLE_LSB                                        8
#define TX_FES_SETUP_IGNORE_BW_AVAILABLE_MSB                                        8
#define TX_FES_SETUP_IGNORE_BW_AVAILABLE_MASK                                       0x0000000000000100


/* Description		IGNORE_TBTT

			Consumer: PDG
			Producer: SCH
			
			If set, PDG ignores remaining TBTTs in PPDU time calculation.
			
			<legal all>
*/

#define TX_FES_SETUP_IGNORE_TBTT_OFFSET                                             0x0000000000000008
#define TX_FES_SETUP_IGNORE_TBTT_LSB                                                9
#define TX_FES_SETUP_IGNORE_TBTT_MSB                                                9
#define TX_FES_SETUP_IGNORE_TBTT_MASK                                               0x0000000000000200


/* Description		STATIC_BANDWIDTH

			Consumer: PDG/TXPCU
			Producer: SCH
			
			Field is reserved when use_static_bw is clear.
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define TX_FES_SETUP_STATIC_BANDWIDTH_OFFSET                                        0x0000000000000008
#define TX_FES_SETUP_STATIC_BANDWIDTH_LSB                                           10
#define TX_FES_SETUP_STATIC_BANDWIDTH_MSB                                           12
#define TX_FES_SETUP_STATIC_BANDWIDTH_MASK                                          0x0000000000001c00


/* Description		SET_TXOP_DURATION_ALL_ONES

			Consumer: PDG
			Producer: SCH
			
			When set, SW embedded a PS_POLL frame in this transmission
			 or the frame in this transmission is for a BSS with BSS
			 Color disabled, e.g. due to BSS color collision.
			PDG sets the TXOP_DURATION of the transmit PPDU to all 1s.
			
			<legal all>
*/

#define TX_FES_SETUP_SET_TXOP_DURATION_ALL_ONES_OFFSET                              0x0000000000000008
#define TX_FES_SETUP_SET_TXOP_DURATION_ALL_ONES_LSB                                 13
#define TX_FES_SETUP_SET_TXOP_DURATION_ALL_ONES_MSB                                 13
#define TX_FES_SETUP_SET_TXOP_DURATION_ALL_ONES_MASK                                0x0000000000002000


/* Description		TRANSMISSION_CONTAINS_MU_RTS

			Consumer: PDG
			Producer: SCH
			
			When set, SW embedded a MU-RTS trigger frame in this transmission.
			
			TXPCU will have to do something special for this with the
			 CTS response timeout (whose value comes from a MU-CTS timeout
			 register)
			
			<legal all>
*/

#define TX_FES_SETUP_TRANSMISSION_CONTAINS_MU_RTS_OFFSET                            0x0000000000000008
#define TX_FES_SETUP_TRANSMISSION_CONTAINS_MU_RTS_LSB                               14
#define TX_FES_SETUP_TRANSMISSION_CONTAINS_MU_RTS_MSB                               14
#define TX_FES_SETUP_TRANSMISSION_CONTAINS_MU_RTS_MASK                              0x0000000000004000


/* Description		BW_RESTRICTED_FRAMES_EMBEDDED

			Consumer: TXPCU
			Producer: SW
			
			This bit should be set by SW when the transmission includes
			 bandwidth restricted frames. As a result of this bit being
			 set, TXPCU will hold of indicating that buffer space is
			 available to TXDMA till the BW decision is done. This allows
			 TXPCU to drop the BW restricted frames at SFM input.
			
			<legal all>
*/

#define TX_FES_SETUP_BW_RESTRICTED_FRAMES_EMBEDDED_OFFSET                           0x0000000000000008
#define TX_FES_SETUP_BW_RESTRICTED_FRAMES_EMBEDDED_LSB                              15
#define TX_FES_SETUP_BW_RESTRICTED_FRAMES_EMBEDDED_MSB                              15
#define TX_FES_SETUP_BW_RESTRICTED_FRAMES_EMBEDDED_MASK                             0x0000000000008000


/* Description		AST_INDEX

			Consumer: RXPCU
			Producer: SCH
			
			Used for implicit BF sounding capture on receive Ack/BA. 
			 The RXPCU needs to tag the receive sounding with ast_index
			 so FW will know which STA is associated with Ack/BA sounding. 
			
			
			<legal all>
*/

#define TX_FES_SETUP_AST_INDEX_OFFSET                                               0x0000000000000008
#define TX_FES_SETUP_AST_INDEX_LSB                                                  16
#define TX_FES_SETUP_AST_INDEX_MSB                                                  31
#define TX_FES_SETUP_AST_INDEX_MASK                                                 0x00000000ffff0000


/* Description		CV_ID

			Consumer: TXPCU
			Producer: SCH
			
			This field is only valid when expect_cbf is set.
			
			A unique ID corresponding to the CV data expected from the
			 CBF frame. 
			
			TXPCU copies this field over to the TX_FES_STATUS TLV
			<legal all>
*/

#define TX_FES_SETUP_CV_ID_OFFSET                                                   0x0000000000000008
#define TX_FES_SETUP_CV_ID_LSB                                                      32
#define TX_FES_SETUP_CV_ID_MSB                                                      39
#define TX_FES_SETUP_CV_ID_MASK                                                     0x000000ff00000000


/* Description		TRIGGER_RESP_TXPDU_PPDU_BOUNDARY

			This field indicates to TXPCU how far into the 11ax trigger
			 response transmission, TXPCU should still accept Trigger
			 response related configuration info from the SCHEDULER (and
			 PDG) to be processed. 
			
			The field indicates a percentage of the total  byte count
			 to be given to the PHY, up to which point TXPCU will still
			 accept all the setup related TLVS to arrive. After that, 
			TXPCU will ignore any remaining setup TLVs to come in and
			 not initiate any MPDU based transfers to the PHY anymore. 
			This is to help avoid corner cases.
			If any setup TLVs did arrive after this point, TXPCU will
			 keep on continuing giving NULL data to the PHY, but once
			 PHYTX_PKT_END is received, TXPCU shall issue a FLUSH request
			 to the SCH, with flush code: TXPCU_TRIG_RESPONSE_INFO_TOO_LATE
			
			TXPCU should not abort the transmission halfway, as that
			 can cause problems for the MU UL receiver...
			
			<enum 0 txpcu_trig_response_boundary_75> TXPCU will not 
			initiate SCH based MPDU transfers after 75% of the PPDU 
			octed count has already been given to the PHY.
			
			<enum 1 txpcu_trig_response_boundary_50> TXPCU will not 
			initiate SCH based MPDU transfers after 50% of the PPDU 
			octed count has already been given to the PHY.
			
			<enum 2 txpcu_trig_response_boundary_25> TXPCU will not 
			initiate SCH based MPDU transfers after 75% of the PPDU 
			octed count has already been given to the PHY.
			
			Note that if TXPCU receives a TX_FES_SETUP with "11ax trigger
			 response transmission" set, and it had already finished
			 sending a response , it should generate a flush with code: 
			TXPCU_TRIG_RESPONSE_MODE_CORRUPTION
			
			<legal 0-2>
*/

#define TX_FES_SETUP_TRIGGER_RESP_TXPDU_PPDU_BOUNDARY_OFFSET                        0x0000000000000008
#define TX_FES_SETUP_TRIGGER_RESP_TXPDU_PPDU_BOUNDARY_LSB                           40
#define TX_FES_SETUP_TRIGGER_RESP_TXPDU_PPDU_BOUNDARY_MSB                           41
#define TX_FES_SETUP_TRIGGER_RESP_TXPDU_PPDU_BOUNDARY_MASK                          0x0000030000000000


/* Description		RXPCU_SETUP_COMPLETE_PRESENT

			To notify current TXFES use new mode and delay "RXPCU_*_SETUP" 
			for HWSCH/TXPCU/RXPCU module
			<legal all>
*/

#define TX_FES_SETUP_RXPCU_SETUP_COMPLETE_PRESENT_OFFSET                            0x0000000000000008
#define TX_FES_SETUP_RXPCU_SETUP_COMPLETE_PRESENT_LSB                               42
#define TX_FES_SETUP_RXPCU_SETUP_COMPLETE_PRESENT_MSB                               42
#define TX_FES_SETUP_RXPCU_SETUP_COMPLETE_PRESENT_MASK                              0x0000040000000000


/* Description		RBO_MUST_HAVE_DATA_USER_LIMIT

			Consumer: PDG
			Producer: SW
			
			Field only valid when Ofdma_triggered_response is NOT set
			 (=> implies transmission started due to backoff expiration)
			
			
			Field only valid for SU and "MU_SU" transmissions.
			
			The requirements for what to transmit depend on what the
			 reason is that this transmission started. If it is 11ax
			 trigger based, the trigger frame will specify all the constrains
			 like max TID count, prefered AC, etc.
			However if this command starts executing due to backoff 
			expiration, the requirements could be different from those
			 that might have come from the trigger frame.
			This field specifies what the constaints are when the transmission
			 is Backoff initiated. 
			
			When set to 0, this feature is disabled
			When set to 1, user 0 must have data otherwise PDG should
			 flush the transmission
			When set to 2, user 0 AND/OR user 1 must have data otherwise
			 PDG should flush the transmission
			When set to 3, user 0 AND/OR user 1 AND/OR user 2 must have
			 data otherwise PDG should flush the transmission
			...
			<legal all>
*/

#define TX_FES_SETUP_RBO_MUST_HAVE_DATA_USER_LIMIT_OFFSET                           0x0000000000000008
#define TX_FES_SETUP_RBO_MUST_HAVE_DATA_USER_LIMIT_LSB                              43
#define TX_FES_SETUP_RBO_MUST_HAVE_DATA_USER_LIMIT_MSB                              46
#define TX_FES_SETUP_RBO_MUST_HAVE_DATA_USER_LIMIT_MASK                             0x0000780000000000


/* Description		MU_NDP

			Field only valid when ndp_frame is set.
			
			If set indicates that this packet is an NDP used for MU 
			channel estimation.  This bit will be used by the TPC to
			 signal that the analog gain settings can be updated. The
			 analog gain settings will not change for subsequent MU 
			data packets.
			<legal all>
*/

#define TX_FES_SETUP_MU_NDP_OFFSET                                                  0x0000000000000008
#define TX_FES_SETUP_MU_NDP_LSB                                                     47
#define TX_FES_SETUP_MU_NDP_MSB                                                     47
#define TX_FES_SETUP_MU_NDP_MASK                                                    0x0000800000000000


/* Description		BF_TYPE

			Consumer: PDG/TXPCU
			Producer: SCH
			
			Field is ONLY valid when 'txbf' is set...
			
			Defines the type of beamforming that is required using this
			 transmission. 
			Note that in the PCU_PPDU_SETUP_INIT TLV, SW can narrow 
			down for which BW the beamforming shall take place. For 
			example, SW can decide that BW is only desired for 40MHz
			 BW, but not for 20...
			If for any of the allowed BW, beamforming is desired, this
			 field should indicate which type of BF.
			
			<enum 0    NO_BF>
			<enum 1    LEGACY_BF>
			<enum 2    SU_BF>
			<enum 3    MU_BF>
			 <legal all>
*/

#define TX_FES_SETUP_BF_TYPE_OFFSET                                                 0x0000000000000008
#define TX_FES_SETUP_BF_TYPE_LSB                                                    48
#define TX_FES_SETUP_BF_TYPE_MSB                                                    49
#define TX_FES_SETUP_BF_TYPE_MASK                                                   0x0003000000000000


/* Description		CBF_NC_INDEX_MASK

			Consumer: TXPCU
			Producer: SCH
			
			When set, TXPCU shall confirm that the received cbf_nc_index
			 is equal to the expected one, indicated by field: cbf_nc_index
			
			
			This field is only allowed to be set in case of a single
			 SU CBF reception.
			
			<legal all>
*/

#define TX_FES_SETUP_CBF_NC_INDEX_MASK_OFFSET                                       0x0000000000000008
#define TX_FES_SETUP_CBF_NC_INDEX_MASK_LSB                                          50
#define TX_FES_SETUP_CBF_NC_INDEX_MASK_MSB                                          50
#define TX_FES_SETUP_CBF_NC_INDEX_MASK_MASK                                         0x0004000000000000


/* Description		CBF_NC_INDEX

			Consumer: TXPCU
			Producer: SCH
			
			Field only valid when cbf_nc_index_mask is set
			
			Expected Nc_index of received CBF frame after sending NDP
			 or BR-Poll. 
			
			<enum 0 nc_1>
			<enum 1 nc_2>
			<enum 2 nc_3>
			<enum 3 nc_4>
			<enum 4 nc_5>
			<enum 5 nc_6>
			<enum 6 nc_7>
			<enum 7 nc_8>
			<legal 0-7>
*/

#define TX_FES_SETUP_CBF_NC_INDEX_OFFSET                                            0x0000000000000008
#define TX_FES_SETUP_CBF_NC_INDEX_LSB                                               51
#define TX_FES_SETUP_CBF_NC_INDEX_MSB                                               53
#define TX_FES_SETUP_CBF_NC_INDEX_MASK                                              0x0038000000000000


/* Description		CBF_NR_INDEX_MASK

			Consumer: TXPCU
			Producer: SCH
			
			When set, TXPCU shall confirm that the received cbf_nr_index
			 is equal to the expected one, indicated in the field: cbf_nr_index
			
			
			This field is only allowed to be set in case of a single
			 SU CBF reception.
			<legal all>
*/

#define TX_FES_SETUP_CBF_NR_INDEX_MASK_OFFSET                                       0x0000000000000008
#define TX_FES_SETUP_CBF_NR_INDEX_MASK_LSB                                          54
#define TX_FES_SETUP_CBF_NR_INDEX_MASK_MSB                                          54
#define TX_FES_SETUP_CBF_NR_INDEX_MASK_MASK                                         0x0040000000000000


/* Description		CBF_NR_INDEX

			Expected Nr_index of received CBF frame after sending NDP
			 or BR-Poll. This field is compared only if cbf_nr_index_mask
			 is set to 1. 
			<enum 0 nr_1>
			<enum 1 nr_2>
			<enum 2 nr_3>
			<enum 3 nr_4>
			<enum 4 nr_5>
			<enum 5 nr_6>
			<enum 6 nr_7>
			<enum 7 nr_8>
			<legal 0-7>
*/

#define TX_FES_SETUP_CBF_NR_INDEX_OFFSET                                            0x0000000000000008
#define TX_FES_SETUP_CBF_NR_INDEX_LSB                                               55
#define TX_FES_SETUP_CBF_NR_INDEX_MSB                                               57
#define TX_FES_SETUP_CBF_NR_INDEX_MASK                                              0x0380000000000000


/* Description		SECURE_RANGING_ISTA

			Consumer: Crypto
			Producer: SW
			
			If set to 1, Crypto will use the 'TX_PEER_ENTRY' for encryption
			 but not for the 'TX_DATA' from TXOLE interface but will
			 wait for 'LMR_{MPDU_START, DATA, MPDU_END}' TLVs from TXPCU
			 to encrypt the ISTA2RSTA LMR.
			
			If set to 0, Crypto will encrypt 'TX_DATA' as for any non-.11az-ranging
			 frame.
*/

#define TX_FES_SETUP_SECURE_RANGING_ISTA_OFFSET                                     0x0000000000000008
#define TX_FES_SETUP_SECURE_RANGING_ISTA_LSB                                        58
#define TX_FES_SETUP_SECURE_RANGING_ISTA_MSB                                        58
#define TX_FES_SETUP_SECURE_RANGING_ISTA_MASK                                       0x0400000000000000


/* Description		NDPA

			When set, this packet is an NDP announcement.
*/

#define TX_FES_SETUP_NDPA_OFFSET                                                    0x0000000000000008
#define TX_FES_SETUP_NDPA_LSB                                                       59
#define TX_FES_SETUP_NDPA_MSB                                                       59
#define TX_FES_SETUP_NDPA_MASK                                                      0x0800000000000000


/* Description		WAIT_SIFS

			Consumer: TXPCU
			Producer: SCH
			
			This field is passed over to the tx_phy_desc by the PDG 
			module. If set, the AMPI will hold this tx_phy_desc TLV 
			from the TX PCU until SIFS has elapsed and then forward 
			the tx_phy_desc to the PHY.  The PHY should ignore this 
			bit.  This bit is used to make sure that transmit SIFS response
			 to a receive frame is cycle accurate and consistent to 
			enable accurate RTT measurement.   
			
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

#define TX_FES_SETUP_WAIT_SIFS_OFFSET                                               0x0000000000000008
#define TX_FES_SETUP_WAIT_SIFS_LSB                                                  60
#define TX_FES_SETUP_WAIT_SIFS_MSB                                                  61
#define TX_FES_SETUP_WAIT_SIFS_MASK                                                 0x3000000000000000


/* Description		CBF_FEEDBACK_TYPE_MASK

			Consumer: TXPCU
			Producer: SCH
			
			When set, TXPCU shall confirm that the cbf_feedback_type
			 is equal to the expected one, indicated in the field: cbf_feedback_type
			
			
			This field is only allowed to be set in case of a single
			 SU CBF reception.
			<legal all>
*/

#define TX_FES_SETUP_CBF_FEEDBACK_TYPE_MASK_OFFSET                                  0x0000000000000008
#define TX_FES_SETUP_CBF_FEEDBACK_TYPE_MASK_LSB                                     62
#define TX_FES_SETUP_CBF_FEEDBACK_TYPE_MASK_MSB                                     62
#define TX_FES_SETUP_CBF_FEEDBACK_TYPE_MASK_MASK                                    0x4000000000000000


/* Description		CBF_FEEDBACK_TYPE

			Consumer: TXPCU
			Producer: SCH
			
			Expected feedback type of received CBF frame after sending
			 NDP or BR-Poll. This field is compared only if cbf_feedback_type_mask
			 is set to 1. 
			<enum 0     SU>
			<enum 1     MU>
			<legal all>
*/

#define TX_FES_SETUP_CBF_FEEDBACK_TYPE_OFFSET                                       0x0000000000000008
#define TX_FES_SETUP_CBF_FEEDBACK_TYPE_LSB                                          63
#define TX_FES_SETUP_CBF_FEEDBACK_TYPE_MSB                                          63
#define TX_FES_SETUP_CBF_FEEDBACK_TYPE_MASK                                         0x8000000000000000


/* Description		CBF_SOUNDING_TOKEN

			Consumer: TXPCU
			Producer: SCH
			
			Expected sounding token of received CBF frame after sending
			 NDP or BR-Poll. This field is compared only if cbf_sounding_token_mask
			 is set to 1. 
			<legal all>
*/

#define TX_FES_SETUP_CBF_SOUNDING_TOKEN_OFFSET                                      0x0000000000000010
#define TX_FES_SETUP_CBF_SOUNDING_TOKEN_LSB                                         0
#define TX_FES_SETUP_CBF_SOUNDING_TOKEN_MSB                                         5
#define TX_FES_SETUP_CBF_SOUNDING_TOKEN_MASK                                        0x000000000000003f


/* Description		CBF_SOUNDING_TOKEN_MASK

			Consumer: TXPCU
			Producer: SCH
			
			When set, TXPCU shall confirm that the cbf_sounding_token
			 is equal to the expected one, indicated in the field: cbf_sounding_token
			
			
			This field is only allowed to be set in case of a single
			 SU CBF reception.
			<legal all>
*/

#define TX_FES_SETUP_CBF_SOUNDING_TOKEN_MASK_OFFSET                                 0x0000000000000010
#define TX_FES_SETUP_CBF_SOUNDING_TOKEN_MASK_LSB                                    6
#define TX_FES_SETUP_CBF_SOUNDING_TOKEN_MASK_MSB                                    6
#define TX_FES_SETUP_CBF_SOUNDING_TOKEN_MASK_MASK                                   0x0000000000000040


/* Description		CBF_BW_MASK

			Consumer: TXPCU
			Producer: SCH
			
			When set, TXPCU shall confirm that the cbf_bw_mask is equal
			 to the expected one, indicated in the field: cbf_bw
			
			This field is only allowed to be set in case of a single
			 SU CBF reception.
			<legal all>
*/

#define TX_FES_SETUP_CBF_BW_MASK_OFFSET                                             0x0000000000000010
#define TX_FES_SETUP_CBF_BW_MASK_LSB                                                7
#define TX_FES_SETUP_CBF_BW_MASK_MSB                                                7
#define TX_FES_SETUP_CBF_BW_MASK_MASK                                               0x0000000000000080


/* Description		CBF_BW

			Consumer: TXPCU
			Producer: SCH
			
			Expected channel width of received CBF frame after sending
			 NDP or BR-Poll. This field is compared only if cbf_bw_mask
			 is set to 1.
			 
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define TX_FES_SETUP_CBF_BW_OFFSET                                                  0x0000000000000010
#define TX_FES_SETUP_CBF_BW_LSB                                                     8
#define TX_FES_SETUP_CBF_BW_MSB                                                     10
#define TX_FES_SETUP_CBF_BW_MASK                                                    0x0000000000000700


/* Description		USE_STATIC_BW

			Consumer: TXPCU
			Producer: SCH
			
			Part of TX_BF_PARAMS: This field is used to indicate to 
			the SVD that the b/w that will be defined in the TX_PHY_DESC
			 for the upcoming TXBF packet will be the same as the static
			 bandwidth, i.e. the bandwidth that was in operation during
			 sounding for the clients in question
			<legal all>
*/

#define TX_FES_SETUP_USE_STATIC_BW_OFFSET                                           0x0000000000000010
#define TX_FES_SETUP_USE_STATIC_BW_LSB                                              11
#define TX_FES_SETUP_USE_STATIC_BW_MSB                                              11
#define TX_FES_SETUP_USE_STATIC_BW_MASK                                             0x0000000000000800


/* Description		COEX_NACK_COUNT

			Consumer: TXPCU
			Producer: SCH
			
			The number of times PDG informed the SCHeduler module that
			 for this scheduling command, the WLAN transmission can 
			not be initialized due to getting a NACK response from the
			 Coex engine, or PDG not being able to fit a transmission
			 within the timing constraints given by Coex.
			
			Note that SCH will (re)set this count to 0 at the start 
			of reading a new SCH command.
			This count is maintained on a per ring basis by the SCHeduler
			
			
			<legal all>
*/

#define TX_FES_SETUP_COEX_NACK_COUNT_OFFSET                                         0x0000000000000010
#define TX_FES_SETUP_COEX_NACK_COUNT_LSB                                            12
#define TX_FES_SETUP_COEX_NACK_COUNT_MSB                                            16
#define TX_FES_SETUP_COEX_NACK_COUNT_MASK                                           0x000000000001f000


/* Description		SCH_TX_BURST_ONGOING

			Consumer: PDG/TXPCU
			Producer: SCH
			
			This field is overwritten by the scheduler module and it's
			 value is coming from the" sifs_burst_continuation" field
			 in the  Scheduler command.
			
			0: No action
			1: The next scheduling command needs to start at SIFS time
			 after finishing the frame transmissions in this command. 
			This allows for SIFS based bursting
			<legal all>
*/

#define TX_FES_SETUP_SCH_TX_BURST_ONGOING_OFFSET                                    0x0000000000000010
#define TX_FES_SETUP_SCH_TX_BURST_ONGOING_LSB                                       17
#define TX_FES_SETUP_SCH_TX_BURST_ONGOING_MSB                                       17
#define TX_FES_SETUP_SCH_TX_BURST_ONGOING_MASK                                      0x0000000000020000


/* Description		GEN_TQM_UPDATE_MPDU_COUNT_TLV

			Consumer: TXPCU
			Producer: SW
			
			NOTE: When PDG is configured to do transmissions in SW mode, 
			this bit shall NEVER be set.
			
			When set, TXPCU shall generate the TQM_UPDATE_TX_MPDU_COUNT
			 TLV immediately after PPDU transmission has finished (and
			 before any response frame might have been received)
			
			When set, SW shall also generate the RXPCU_USER_SETUP TLVs
			 as this is where TXPCU will get the MPDU_queue addresses.
			
			<legal all>
*/

#define TX_FES_SETUP_GEN_TQM_UPDATE_MPDU_COUNT_TLV_OFFSET                           0x0000000000000010
#define TX_FES_SETUP_GEN_TQM_UPDATE_MPDU_COUNT_TLV_LSB                              18
#define TX_FES_SETUP_GEN_TQM_UPDATE_MPDU_COUNT_TLV_MSB                              18
#define TX_FES_SETUP_GEN_TQM_UPDATE_MPDU_COUNT_TLV_MASK                             0x0000000000040000


/* Description		TRANSMIT_VIF

			Consumer: TXOLE
			Producer: SW
			
			The VIF for this transmission. Used in MCC mode to control/overwrite
			 the PM bit settings. Based on this VIF value, TXOLE gets
			 the pm bit control instructions from the pm_state_overwrite_per_vif
			 register
			
			<legal all>
*/

#define TX_FES_SETUP_TRANSMIT_VIF_OFFSET                                            0x0000000000000010
#define TX_FES_SETUP_TRANSMIT_VIF_LSB                                               19
#define TX_FES_SETUP_TRANSMIT_VIF_MSB                                               22
#define TX_FES_SETUP_TRANSMIT_VIF_MASK                                              0x0000000000780000


/* Description		OPTIMAL_BW_RETRY_COUNT

			Consumer: TXPCU
			Producer: SCH
			
			This field is overwritten by the scheduler module and it's
			 value is coming from an internal counter in the scheduler
			 that keeps track of how many times this scheduling command
			 has been flushed by TXPCU as a result of most desired BW
			 not being available (=> flush code: TXPCU_FLREQ_RETRY_FOR_OPTIMAL_BW)
			
			
			For the first transmission, this count is always set to 
			0. 
			<legal all>
*/

#define TX_FES_SETUP_OPTIMAL_BW_RETRY_COUNT_OFFSET                                  0x0000000000000010
#define TX_FES_SETUP_OPTIMAL_BW_RETRY_COUNT_LSB                                     23
#define TX_FES_SETUP_OPTIMAL_BW_RETRY_COUNT_MSB                                     26
#define TX_FES_SETUP_OPTIMAL_BW_RETRY_COUNT_MASK                                    0x0000000007800000


/* Description		FES_CONTINUATION_RATIO_THRESHOLD

			Field evaluated by TXPCU only.
			
			This Feature is not supported in Napier and Hastings.
			
			Field can be used in both SU and MU transmissions, but might
			 be most useful in MU transmissions.
			
			TXPCU keeps track of how many MPDU data words are transmited
			 as well as how many Null delimiters are transmitted. In
			 case of an MU and/or multi TID transmission, these two 
			counters are the aggregates over all the users. 
			
			At the end of the FES, TXPCU determines the ratio between
			 the actual MPDU data words and Null delimiters. If this
			 ratio is LESS then the ratio indicated here, TXPCU should
			 indicate "Transmit_data_null_ratio_not_met" in the TX_FES_STATUS_END
			 
			
			<enum 0 No_Data_Null_ratio_requirement> TXPCU does not need
			 to do any evaluation on the ratio between actual data transmitted
			 and NULL delimiters inserted.
			<enum 1 Data_Null_ratio_16_1> At the end of the FES, TXPCU
			 shall confirm that the DATA:NULL delimiter ratio was at
			 least 16:1. If not met, TXPCU should terminate FES.
			<enum 2 Data_Null_ratio_8_1> At the end of the FES, TXPCU
			 shall confirm that the DATA:NULL delimiter ratio was at
			 least 8:1. If not met, TXPCU should terminate FES.
			<enum 3 Data_Null_ratio_4_1> At the end of the FES, TXPCU
			 shall confirm that the DATA:NULL delimiter ratio was at
			 least 4:1. If not met, TXPCU should terminate FES.
			<enum 4 Data_Null_ratio_2_1> At the end of the FES, TXPCU
			 shall confirm that the DATA:NULL delimiter ratio was at
			 least 2:1. If not met, TXPCU should terminate FES.
			<enum 5 Data_Null_ratio_1_1> At the end of the FES, TXPCU
			 shall confirm that the DATA:NULL delimiter ratio was at
			 least 1:1. If not met, TXPCU should terminate FES.
			<enum 6 Data_Null_ratio_1_2> At the end of the FES, TXPCU
			 shall confirm that the DATA:NULL delimiter ratio was at
			 least 1:2. If not met, TXPCU should terminate FES.
			<enum 7 Data_Null_ratio_1_4> At the end of the FES, TXPCU
			 shall confirm that the DATA:NULL delimiter ratio was at
			 least 1:4. If not met, TXPCU should terminate FES.
			<enum 8 Data_Null_ratio_1_8> At the end of the FES, TXPCU
			 shall confirm that the DATA:NULL delimiter ratio was at
			 least 1:8. If not met, TXPCU should terminate FES.
			<enum 9 Data_Null_ratio_1_16> At the end of the FES, TXPCU
			 shall confirm that the DATA:NULL delimiter ratio was at
			 least 1:16. If not met, TXPCU should terminate FES.
			
			<legal 0-9>
*/

#define TX_FES_SETUP_FES_CONTINUATION_RATIO_THRESHOLD_OFFSET                        0x0000000000000010
#define TX_FES_SETUP_FES_CONTINUATION_RATIO_THRESHOLD_LSB                           27
#define TX_FES_SETUP_FES_CONTINUATION_RATIO_THRESHOLD_MSB                           31
#define TX_FES_SETUP_FES_CONTINUATION_RATIO_THRESHOLD_MASK                          0x00000000f8000000


/* Description		TRANSMIT_CCA_BITMAP

			The CCA signals that shall be evaluated by TXPCU to determine
			 the BW/puncture pattern available for transmission.
			
			0: CCA signal not needed. Ignore the CCA setting
			1: CCA signals shall be evaluated
			
			Bit [1:0]     => cca20_0 related signals
			Bit [3:2]   => cca20_1 related signals
			...
			Bit [31:30] => cca20_15 related signals
			
			Within the 2 bits, the order is always:
			Bit0: ED
			Bit1: GI
			
			NOTE: HW Sch takes care of MUXing ED1/ED2 with ED0 and MUXing
			 GI1 with GI0. Hence this field should be set to 0x55555555
			 for chips not supporting GI-correlation and 0xFFFFFFFF 
			for chips that support, usually.
			<legal all>
*/

#define TX_FES_SETUP_TRANSMIT_CCA_BITMAP_OFFSET                                     0x0000000000000010
#define TX_FES_SETUP_TRANSMIT_CCA_BITMAP_LSB                                        32
#define TX_FES_SETUP_TRANSMIT_CCA_BITMAP_MSB                                        63
#define TX_FES_SETUP_TRANSMIT_CCA_BITMAP_MASK                                       0xffffffff00000000


/* Description		TB_RANGING

			Indicates that this frame is generated for a TB ranging 
			sequence
			<legal all>
*/

#define TX_FES_SETUP_TB_RANGING_OFFSET                                              0x0000000000000018
#define TX_FES_SETUP_TB_RANGING_LSB                                                 0
#define TX_FES_SETUP_TB_RANGING_MSB                                                 0
#define TX_FES_SETUP_TB_RANGING_MASK                                                0x0000000000000001


/* Description		RANGING_TRIGGER_SUBTYPE

			Field only valid if TB_Ranging is set
			
			Indicates the Trigger subtype for the current ranging TF
			
			
			<enum 0 TF_Poll>
			<enum 1 TF_Sound>
			<enum 2 TF_Secure_Sound>
			<enum 3 TF_Report>
			
			<legal 0-3>
*/

#define TX_FES_SETUP_RANGING_TRIGGER_SUBTYPE_OFFSET                                 0x0000000000000018
#define TX_FES_SETUP_RANGING_TRIGGER_SUBTYPE_LSB                                    1
#define TX_FES_SETUP_RANGING_TRIGGER_SUBTYPE_MSB                                    4
#define TX_FES_SETUP_RANGING_TRIGGER_SUBTYPE_MASK                                   0x000000000000001e


/* Description		MIN_CTS2SELF_COUNT

			Field only valid when max_cts2self_count is non-zero
			
			This is the minimum number of CTS2SELF frames that PDG should
			 transmit before the actual data transmission.
*/

#define TX_FES_SETUP_MIN_CTS2SELF_COUNT_OFFSET                                      0x0000000000000018
#define TX_FES_SETUP_MIN_CTS2SELF_COUNT_LSB                                         5
#define TX_FES_SETUP_MIN_CTS2SELF_COUNT_MSB                                         8
#define TX_FES_SETUP_MIN_CTS2SELF_COUNT_MASK                                        0x00000000000001e0


/* Description		MAX_CTS2SELF_COUNT

			Field only valid when non-zero
			
			This is the maximum number of CTS2SELF frames that PDG is
			 allowed to transmit before the actual data transmission. 
			PDG will only use these additional frames if MPDU info from
			 TQM or CV-correlation info from microcode is delayed.
*/

#define TX_FES_SETUP_MAX_CTS2SELF_COUNT_OFFSET                                      0x0000000000000018
#define TX_FES_SETUP_MAX_CTS2SELF_COUNT_LSB                                         9
#define TX_FES_SETUP_MAX_CTS2SELF_COUNT_MSB                                         12
#define TX_FES_SETUP_MAX_CTS2SELF_COUNT_MASK                                        0x0000000000001e00


/* Description		WIFI_RADAR_ENABLE

			When set to 1, the packet is intended to be used by PHY 
			for WiFi radar (by sensing the reflected WiFi signal).
			<legal all>
*/

#define TX_FES_SETUP_WIFI_RADAR_ENABLE_OFFSET                                       0x0000000000000018
#define TX_FES_SETUP_WIFI_RADAR_ENABLE_LSB                                          13
#define TX_FES_SETUP_WIFI_RADAR_ENABLE_MSB                                          13
#define TX_FES_SETUP_WIFI_RADAR_ENABLE_MASK                                         0x0000000000002000


/* Description		RESERVED_6A

			Bit 14: cqi_feedback:
			Consumer: TXPCU
			Producer: SCH
			
			MSB of the expected feedback type of received CBF frame 
			after sending NDP or BR-Poll in case of HE/EHT sounding. 
			See field cbf_feedback_type above for the LSB. This field
			 is compared only if cbf_feedback_type_mask is set to 1. 
			
			0: compressed beamforming feedback
			1: CQI feedback
			
			<legal 0-1>
*/

#define TX_FES_SETUP_RESERVED_6A_OFFSET                                             0x0000000000000018
#define TX_FES_SETUP_RESERVED_6A_LSB                                                14
#define TX_FES_SETUP_RESERVED_6A_MSB                                                31
#define TX_FES_SETUP_RESERVED_6A_MASK                                               0x00000000ffffc000


/* Description		MONITOR_OVERRIDE_STA_31_0

			Used by TXMON
			
			LSB 32 bits of a 37-bit user bitmap with 1s denoting the
			 'tlv_usr' values that correspond to'Monitor override client's
			
			
			When enabled in TXMON, it will discard the user-TLVs of 
			the users not selected by the bitmap. FW should program 
			this setting in line with the 'Monitor_override_sta' setting
			 in the 'ADDR_SEARCH_ENTRY' corresponding to each of the
			 clients.
			
			<legal all>
*/

#define TX_FES_SETUP_MONITOR_OVERRIDE_STA_31_0_OFFSET                               0x0000000000000018
#define TX_FES_SETUP_MONITOR_OVERRIDE_STA_31_0_LSB                                  32
#define TX_FES_SETUP_MONITOR_OVERRIDE_STA_31_0_MSB                                  63
#define TX_FES_SETUP_MONITOR_OVERRIDE_STA_31_0_MASK                                 0xffffffff00000000


/* Description		MONITOR_OVERRIDE_STA_36_32

			Used by TXMON
			
			MSB 5 bits of a 37-bit user bitmap with 1s denoting the 'tlv_usr' 
			values that correspond to 'Monitor override client's
			
			See 'Monitor_override_sta_31_0.'
			
			Hamilton v1 did not include this (and any subsequent) word.
			
			<legal all>
*/

#define TX_FES_SETUP_MONITOR_OVERRIDE_STA_36_32_OFFSET                              0x0000000000000020
#define TX_FES_SETUP_MONITOR_OVERRIDE_STA_36_32_LSB                                 0
#define TX_FES_SETUP_MONITOR_OVERRIDE_STA_36_32_MSB                                 4
#define TX_FES_SETUP_MONITOR_OVERRIDE_STA_36_32_MASK                                0x000000000000001f


/* Description		RESERVED_8A

			<legal 0>
*/

#define TX_FES_SETUP_RESERVED_8A_OFFSET                                             0x0000000000000020
#define TX_FES_SETUP_RESERVED_8A_LSB                                                5
#define TX_FES_SETUP_RESERVED_8A_MSB                                                31
#define TX_FES_SETUP_RESERVED_8A_MASK                                               0x00000000ffffffe0


/* Description		FW2SW_INFO

			This field is provided by FW, to be logged via TXMON to 
			host SW. It is transparent to HW.
			
			<legal all>
*/

#define TX_FES_SETUP_FW2SW_INFO_OFFSET                                              0x0000000000000020
#define TX_FES_SETUP_FW2SW_INFO_LSB                                                 32
#define TX_FES_SETUP_FW2SW_INFO_MSB                                                 63
#define TX_FES_SETUP_FW2SW_INFO_MASK                                                0xffffffff00000000



#endif   // TX_FES_SETUP
