
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

 
 
 
 
 
 
 


#ifndef _PHYRX_ABORT_REQUEST_INFO_H_
#define _PHYRX_ABORT_REQUEST_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_PHYRX_ABORT_REQUEST_INFO 1


struct phyrx_abort_request_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t phyrx_abort_reason                                      :  8, // [7:0]
                      phy_enters_nap_state                                    :  1, // [8:8]
                      phy_enters_defer_state                                  :  1, // [9:9]
                      reserved_0                                              :  6, // [15:10]
                      receive_duration                                        : 16; // [31:16]
#else
             uint32_t receive_duration                                        : 16, // [31:16]
                      reserved_0                                              :  6, // [15:10]
                      phy_enters_defer_state                                  :  1, // [9:9]
                      phy_enters_nap_state                                    :  1, // [8:8]
                      phyrx_abort_reason                                      :  8; // [7:0]
#endif
};


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

#define PHYRX_ABORT_REQUEST_INFO_PHYRX_ABORT_REASON_OFFSET                          0x00000000
#define PHYRX_ABORT_REQUEST_INFO_PHYRX_ABORT_REASON_LSB                             0
#define PHYRX_ABORT_REQUEST_INFO_PHYRX_ABORT_REASON_MSB                             7
#define PHYRX_ABORT_REQUEST_INFO_PHYRX_ABORT_REASON_MASK                            0x000000ff


/* Description		PHY_ENTERS_NAP_STATE

			When set, PHY enters PHY NAP state after sending this abort
			
			
			Note that nap and defer state are mutually exclusive.
			
			Field put pro-actively in place....usage still to be agreed
			 upon.
			<legal all>
*/

#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_NAP_STATE_OFFSET                        0x00000000
#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_NAP_STATE_LSB                           8
#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_NAP_STATE_MSB                           8
#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_NAP_STATE_MASK                          0x00000100


/* Description		PHY_ENTERS_DEFER_STATE

			When set, PHY enters PHY defer state after sending this 
			abort
			
			Note that nap and defer state are mutually exclusive.
			
			Field put pro-actively in place....usage still to be agreed
			 upon.
			<legal all>
*/

#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_DEFER_STATE_OFFSET                      0x00000000
#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_DEFER_STATE_LSB                         9
#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_DEFER_STATE_MSB                         9
#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_DEFER_STATE_MASK                        0x00000200


/* Description		RESERVED_0

			<legal 0>
*/

#define PHYRX_ABORT_REQUEST_INFO_RESERVED_0_OFFSET                                  0x00000000
#define PHYRX_ABORT_REQUEST_INFO_RESERVED_0_LSB                                     10
#define PHYRX_ABORT_REQUEST_INFO_RESERVED_0_MSB                                     15
#define PHYRX_ABORT_REQUEST_INFO_RESERVED_0_MASK                                    0x0000fc00


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

#define PHYRX_ABORT_REQUEST_INFO_RECEIVE_DURATION_OFFSET                            0x00000000
#define PHYRX_ABORT_REQUEST_INFO_RECEIVE_DURATION_LSB                               16
#define PHYRX_ABORT_REQUEST_INFO_RECEIVE_DURATION_MSB                               31
#define PHYRX_ABORT_REQUEST_INFO_RECEIVE_DURATION_MASK                              0xffff0000



#endif   // PHYRX_ABORT_REQUEST_INFO
