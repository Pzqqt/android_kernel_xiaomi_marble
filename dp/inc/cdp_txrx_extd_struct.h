/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
#ifndef _CDP_TXRX_EXTD_STRUCT_H_
#define _CDP_TXRX_EXTD_STRUCT_H_

/* Maximum number of receive chains */
#define CDP_MAX_RX_CHAINS 8

#ifdef WLAN_RX_PKT_CAPTURE_ENH
/**
 * struct cdp_rx_indication_mpdu_info - Rx MPDU info
 * @ppdu_id: PPDU Id
 * @duration: PPDU duration
 * @first_data_seq_ctrl: Sequence control field of first data frame
 * @ltf_size: ltf_size
 * @stbc: When set, STBC rate was used
 * @he_re: he_re (range extension)
 * @bw: Bandwidth
 *       <enum 0 bw_20_MHz>
 *       <enum 1 bw_40_MHz>
 *       <enum 2 bw_80_MHz>
 *       <enum 3 bw_160_MHz>
 * @nss: NSS 1,2, ...8
 * @mcs: MCS index
 * @preamble: preamble
 * @gi: <enum 0     0_8_us_sgi > Legacy normal GI
 *       <enum 1     0_4_us_sgi > Legacy short GI
 *       <enum 2     1_6_us_sgi > HE related GI
 *       <enum 3     3_2_us_sgi > HE
 * @dcm: dcm
 * @ldpc: ldpc
 * @fcs_err: FCS error
 * @ppdu_type: SU/MU_MIMO/MU_OFDMA/MU_MIMO_OFDMA/UL_TRIG/BURST_BCN/UL_BSR_RESP/
 * UL_BSR_TRIG/UNKNOWN
 * @rssi_comb: Combined RSSI value (units = dB above noise floor)
 * @nf: noise floor
 * @timestamp: TSF at the reception of PPDU
 * @length: PPDU length
 * @per_chain_rssi: RSSI per chain
 * @channel: Channel informartion
 */
struct cdp_rx_indication_mpdu_info {
	uint32_t ppdu_id;
	uint16_t duration;
	uint16_t first_data_seq_ctrl;
	uint64_t ltf_size:2,
		 stbc:1,
		 he_re:1,
		 bw:4,
		 ofdma_info_valid:1,
		 ofdma_ru_start_index:7,
		 ofdma_ru_width:7,
		 nss:4,
		 mcs:4,
		 preamble:4,
		 gi:4,
		 dcm:1,
		 ldpc:1,
		 fcs_err:1,
		 ppdu_type:5;
	uint32_t rssi_comb;
	uint32_t nf;
	uint64_t timestamp;
	uint32_t length;
	uint8_t per_chain_rssi[MAX_CHAIN];
	uint8_t channel;
};

/**
 * struct cdp_rx_indication_mpdu- Rx MPDU plus MPDU info
 * @mpdu_info: defined in cdp_rx_indication_mpdu_info
 * @data: skb chain of a MPDU. The first of 128 Byte of MPDU
 *        chained with first of 128 Byte of MSDUs.
 */
struct cdp_rx_indication_mpdu {
	struct cdp_rx_indication_mpdu_info mpdu_info;
	qdf_nbuf_t nbuf;
};
#endif /* WLAN_RX_PKT_CAPTURE_ENH */
#endif /* _CDP_TXRX_EXTD_STRUCT_H_ */
