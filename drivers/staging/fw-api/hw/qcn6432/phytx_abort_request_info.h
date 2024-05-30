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

#ifndef _PHYTX_ABORT_REQUEST_INFO_H_
#define _PHYTX_ABORT_REQUEST_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_WORDS_PHYTX_ABORT_REQUEST_INFO 1


struct phytx_abort_request_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint16_t phytx_abort_reason                                      :  8, // [7:0]
                      user_number                                             :  6, // [13:8]
                      reserved                                                :  2; // [15:14]
#else
             uint16_t reserved                                                :  2, // [15:14]
                      user_number                                             :  6, // [13:8]
                      phytx_abort_reason                                      :  8; // [7:0]
#endif
};


/* Description		PHYTX_ABORT_REASON

			Reason for early termination of TX packet by the PHY 
			
			<enum 0 no_phytx_error_reported>This value is the default
			 value the MAC will fill in the status TLV (when not PHY
			 abort was received).
			
			Note that when PHY generates the PHYTX_ABORT_REQUEST, this
			 value shall never be used.
			<enum 1 error_txtd_ifft_underrun>PHY ran out of transmit
			 data due to transmit underrun - this field is user-specific
			 (see user_number field)
			<enum 2 error_tx_invalid_tlv>
			<enum 3 error_tx_unexpected_tlv>
			<enum 4 error_tx_pkt_end_error>
			<enum 5 error_tx_bw_is_gt_dyn_bw>
			<enum 6 error_txtd_pkt_start_error>
			<enum 7 error_txfd_pre_phy_tlv_ooo>
			<enum 8 error_txtd_mu_data_underrun>
			<enum 9 error_tx_legacy_rate_illegal>
			<enum 10 error_tx_fifo_error>
			<enum 11 error_tx_ack_wd_error>
			<enum 12 error_tx_tpc_miss>
			<enum 13 error_mac_tx_abort>
			<enum 14 error_tx_pcss_phy_desc_wdg_timeout>
			<enum 15 error_unsupported_cbf>
			<enum 16 error_cv_static_bandwidth_mismatch>
			<enum 17 error_cv_dynamic_bandwidth_mismatch>
			<enum 18 error_cv_unsupported_nss_total>
			<enum 19 error_nss_bf_params_mismatch>
			<enum 20 error_txbf_fail>
			<enum 21 error_txbf_snd_fail>This used to be called 'error_illegal_nss.'
			
			<enum 22 error_otp_txbf>
			<enum 23 error_tx_inv_chainmask>
			<enum 24 error_cv_index_assign_overload>This error indicates
			 that CV prefetch command indicated a CV index that is not
			 available.
			<enum 25 error_cv_index_delete>This error indicates that
			 CV delete command indicated a CV index that did not contain
			 any valid info
			<enum 26 error_tx_he_rate_illegal>Error found with the HE
			 transmission parameters
			<enum 27 error_tx_pcss_wdg_timeout>
			<enum 28 error_tx_tlv_tag_mismatch>
			<enum 29 error_tx_cck_fifo_flush>
			<enum 30 error_tx_no_mac_pkt_end>
			<enum 31 error_tx_abort_for_mac_war>
			<enum 32 error_tx_stuck>
			<enum 33 error_tx_invalid_uplink_tlv>
			<enum 34 error_txfd_txcck_illegal_tx_rate_error>
			<enum 35 error_txfd_txcck_underrun_error>
			<enum 36 error_txfd_mpi_req_grant_error>
			<enum 37 error_txfd_control_tlv_fifo_ovfl_error>
			<enum 38 error_txfd_tlv_fifo_overflow_error>
			<enum 39 error_txfd_data_fifo_underflow_error>
			<enum 40 error_txfd_data_fifo_overflow_error>
			<enum 41 error_txfd_service_fifo_overflow_error>
			<enum 42 error_txfd_he_sigb_fifo_overflow_error>
			<enum 43 error_txfd_spurious_data_fifo_error>
			<enum 44 error_txfd_he_siga_fifo_ovfl_error>
			<enum 45 error_txfd_unknown_tlv_error>
			<enum 46 error_txfd_mac_response_ordering_error>
			<enum 47 error_txfd_unexpected_mac_pkt_end_error>
			<enum 48 error_txfd_tlv_fifo_rd_hang_error>All FIFO read
			 hang errors use this value.
			<enum 49 error_txfd_tlv_fifo_no_rd_error>All FIFO no read
			 errors use this value.
			<enum 50 error_txfd_ordering_fifo_no_rd_error>
			<enum 51 error_txfd_illegal_cf_tlv_error>
			<enum 52 error_txfd_user_ru_hang_error>
			<enum 53 error_txfd_stream_ru_hang_error>
			<enum 54 error_txfd_num_pad_bits_error>
			<enum 55 error_txfd_phy_abort_ack_wd_to_error>
			<enum 56 error_txfd_pre_pkt_isr_not_done_before_phy_desc_error>
			
			<enum 57 error_txfd_bf_weights_not_ready_error>
			<enum 58 error_txfd_req_timer_breach_error>
			<enum 59 error_txfd_wd_to_error>
			<enum 60 error_txfd_legacy_bf_weights_not_ready_error>
			<enum 61 error_txfd_axi_slave_to_error>
			<enum 62 error_txfd_hw_acc_error>
			<enum 63 error_txfd_txb_req_fifo_underrun_error>
			<enum 64 error_txfd_unknown_ru_alloc_error>
			<enum 65 error_txfd_more_user_desc_per_user_tlvs_error>
			<enum 66 error_txfd_ldpc_param_calc_to_error>
			<enum 69 error_txfd_cbf_start_before_expect_cbf_clear_error>
			
			<enum 70 error_txfd_out_of_range_cbf_user_id_error>
			<enum 71 error_txfd_less_cbf_data_error>
			<enum 72 error_txfd_more_cbf_data_error>
			<enum 73 error_txfd_cbf_done_not_received_error>
			<enum 74 error_txfd_mpi_cbf_valid_to_error>
			<enum 75 error_txfd_cbf_start_missing_error>
			<enum 76 error_txfd_mimo_ctrl_error>
			<enum 77 error_txfd_cbf_buffer_ovfl_error>
			<enum 78 error_txfd_dma0_hang_error>
			<enum 79 error_txfd_dma1_hang_error>
			<enum 80 error_txfd_b2b_cbf_start_error>
			<enum 81 error_txfd_b2b_cbf_done_error>
			<enum 82 error_txfd_unsaved_cv_error>
			<enum 83 error_txfd_wt_mem_wr_conflict_error>
			<enum 84 error_txfd_wt_mem_rd_conflict_error>
			<enum 85 error_txfd_qre_intf_to_error>
			<enum 86 error_txfd_qre_txbf_stomp_rx_error>
			<enum 87 error_txfd_qre_rx_stomp_txbf_error>
			<enum 88 error_txfd_precoding_start_before_bf_param_clr_error>
			
			<enum 89 error_txfd_tone_map_lut_rd_conflict_error>
			<enum 90 error_txfd_precoding_fifo_ovfl_error>
			<enum 91 error_txfd_precoding_fifo_udfl_error>
			<enum 92 error_txfd_txbf_axi_slave_to_error>
			<enum 93 error_txfd_less_prefetch_tlvs_error>
			<enum 94 error_txfd_more_prefetch_tlvs_error>
			<enum 95 error_txfd_prefetch_fifo_ovfl_error>
			<enum 96 error_txfd_prefetch_fifo_udfl_error>
			<enum 97 error_txfd_precoding_error>
			<enum 98 error_txfd_cv_ctrl_state_to_error>
			<enum 99 error_txfd_txbfp_qre_tone_udfl_error>
			<enum 100 error_txfd_less_bf_param_per_user_tlvs_error>
			<enum 101 error_txfd_more_bf_param_per_user_tlvs_error>
			<enum 102 error_txfd_bf_param_common_unexpected_error>
			<enum 103 error_txfd_less_expect_cbf_per_user_tlvs_error>
			
			<enum 104 error_txfd_more_expect_cbf_per_user_tlvs_error>
			
			<enum 105 error_txfd_precoding_stg1_stg2_wait_to_error>
			<enum 106 error_txfd_expect_cbf_per_user_before_common_error>
			
			<enum 107 error_txfd_prefetch_per_user_before_common_error>
			
			<enum 108 error_txfd_bf_param_per_user_before_common_error>
			
			<enum 109 error_txfd_ndp_cbf_bw_mismatch_error>
			<enum 110 error_txtd_tx_pre_desc_error>
			<enum 111 error_txtd_tx_desc_error>
			<enum 112 error_txtd_start_error>
			<enum 113 error_txtd_sym_error>
			<enum 114 error_txtd_multi_sym_error>
			<enum 115 error_txtd_pre_data_error>
			<enum 116 error_txtd_pkt_data_error>
			<enum 117 error_txtd_pkt_end_error>
			<enum 118 error_txtd_tx_frame_unexp>
			<enum 119 error_txtd_start_unexp>
			<enum 120 error_txtd_fft_error_1>
			<enum 121 error_txtd_fft_error_2>
			<enum 122 error_txtd_uld_sym_cp_len_zero>
			<enum 123 error_txtd_start_done>
			<enum 124 error_txtd_start_nonidle>
			<enum 125 error_txtd_tx_abort_nonidle>
			<enum 126 error_txtd_tx_abort_done>
			<enum 127 error_txtd_tx_abort_idle>
			<enum 128 error_txtd_cck_sample_overflow>
			<enum 129 error_txtd_cck_timeout>
			<enum 130 error_txtd_ofdm_sym_mismatch>
			<enum 131 error_txtd_tx_vld_unalign_error>
			<enum 132 error_txtd_fft_cdc_fifo>This is the merged Rx/Tx
			 CDC FIFO empty/full error code
			<enum 133 error_mac_tb_ppdu_abort>All 'error_txtd_chn' codes
			 use this value as well.
			<enum 136 error_abort_req_from_macrx_enum_05>This code is
			 used to abort the Tx when MAC Rx issues an abort request
			 with code 05 "macrx_abort_too_much_bad_data."
			<enum 137 error_tx_extra_sym_mismatch>
			<enum 138 error_tx_vht_length_not_multiple_of_3>
			<enum 139 error_tx_11b_rate_illegal>
			<enum 140 error_tx_ht_rate_illegal>
			<enum 141 error_tx_vht_rate_illegal>
			<enum 142 error_mac_rf_only_abort>
			<enum 255 error_tx_invalid_error_code>
*/

#define PHYTX_ABORT_REQUEST_INFO_PHYTX_ABORT_REASON_OFFSET                          0x00000000
#define PHYTX_ABORT_REQUEST_INFO_PHYTX_ABORT_REASON_LSB                             0
#define PHYTX_ABORT_REQUEST_INFO_PHYTX_ABORT_REASON_MSB                             7
#define PHYTX_ABORT_REQUEST_INFO_PHYTX_ABORT_REASON_MASK                            0x000000ff


/* Description		USER_NUMBER

			For some errors, the user for which this error was detected
			 can be indicated in this field.
			<legal 0-36>
*/

#define PHYTX_ABORT_REQUEST_INFO_USER_NUMBER_OFFSET                                 0x00000000
#define PHYTX_ABORT_REQUEST_INFO_USER_NUMBER_LSB                                    8
#define PHYTX_ABORT_REQUEST_INFO_USER_NUMBER_MSB                                    13
#define PHYTX_ABORT_REQUEST_INFO_USER_NUMBER_MASK                                   0x00003f00


/* Description		RESERVED

			<legal 0>
*/

#define PHYTX_ABORT_REQUEST_INFO_RESERVED_OFFSET                                    0x00000000
#define PHYTX_ABORT_REQUEST_INFO_RESERVED_LSB                                       14
#define PHYTX_ABORT_REQUEST_INFO_RESERVED_MSB                                       15
#define PHYTX_ABORT_REQUEST_INFO_RESERVED_MASK                                      0x0000c000



#endif   // PHYTX_ABORT_REQUEST_INFO
