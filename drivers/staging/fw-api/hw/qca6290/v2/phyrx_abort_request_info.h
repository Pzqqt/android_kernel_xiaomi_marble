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

#ifndef _PHYRX_ABORT_REQUEST_INFO_H_
#define _PHYRX_ABORT_REQUEST_INFO_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	phyrx_abort_reason[7:0], reserved_0[31:8]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_PHYRX_ABORT_REQUEST_INFO 1

struct phyrx_abort_request_info {
             uint32_t phyrx_abort_reason              :  8, //[7:0]
                      reserved_0                      : 24; //[31:8]
};

/*

phyrx_abort_reason
			
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
			
			
			
			<legal 0 - 42>

reserved_0
			
			<legal 0>
*/


/* Description		PHYRX_ABORT_REQUEST_INFO_0_PHYRX_ABORT_REASON
			
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
			
			
			
			<legal 0 - 42>
*/
#define PHYRX_ABORT_REQUEST_INFO_0_PHYRX_ABORT_REASON_OFFSET         0x00000000
#define PHYRX_ABORT_REQUEST_INFO_0_PHYRX_ABORT_REASON_LSB            0
#define PHYRX_ABORT_REQUEST_INFO_0_PHYRX_ABORT_REASON_MASK           0x000000ff

/* Description		PHYRX_ABORT_REQUEST_INFO_0_RESERVED_0
			
			<legal 0>
*/
#define PHYRX_ABORT_REQUEST_INFO_0_RESERVED_0_OFFSET                 0x00000000
#define PHYRX_ABORT_REQUEST_INFO_0_RESERVED_0_LSB                    8
#define PHYRX_ABORT_REQUEST_INFO_0_RESERVED_0_MASK                   0xffffff00


#endif // _PHYRX_ABORT_REQUEST_INFO_H_
