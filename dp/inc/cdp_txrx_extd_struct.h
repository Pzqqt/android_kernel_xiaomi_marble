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
 * @rate: legacy packet rate
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
		 ppdu_type:5,
		 rate:8;
	uint32_t rssi_comb;
	uint32_t nf;
	uint64_t timestamp;
	uint32_t length;
	uint8_t per_chain_rssi[CDP_MAX_RX_CHAINS];
	uint8_t channel;
};

#ifdef __KERNEL__
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
#endif
#endif /* WLAN_RX_PKT_CAPTURE_ENH */
struct ol_ath_dbg_rx_rssi {
	uint8_t     rx_rssi_pri20;
	uint8_t     rx_rssi_sec20;
	uint8_t     rx_rssi_sec40;
	uint8_t     rx_rssi_sec80;
};

struct ol_ath_radiostats {
	uint64_t    tx_beacon;
	uint32_t    tx_buf_count;
	int32_t     tx_mgmt;
	int32_t     rx_mgmt;
	uint32_t    rx_num_mgmt;
	uint32_t    rx_num_ctl;
	uint32_t    tx_rssi;
	uint32_t    rx_rssi_comb;
	struct      ol_ath_dbg_rx_rssi rx_rssi_chain0;
	struct      ol_ath_dbg_rx_rssi rx_rssi_chain1;
	struct      ol_ath_dbg_rx_rssi rx_rssi_chain2;
	struct      ol_ath_dbg_rx_rssi rx_rssi_chain3;
	uint32_t    rx_overrun;
	uint32_t    rx_phyerr;
	uint32_t    ackrcvbad;
	uint32_t    rtsbad;
	uint32_t    rtsgood;
	uint32_t    fcsbad;
	uint32_t    nobeacons;
	uint32_t    mib_int_count;
	uint32_t    rx_looplimit_start;
	uint32_t    rx_looplimit_end;
	uint8_t     ap_stats_tx_cal_enable;
	uint8_t     self_bss_util;
	uint8_t     obss_util;
	uint8_t     ap_rx_util;
	uint8_t     free_medium;
	uint8_t     ap_tx_util;
	uint8_t     obss_rx_util;
	uint8_t     non_wifi_util;
	uint32_t    tgt_asserts;
	int16_t     chan_nf;
	int16_t     chan_nf_sec80;
	uint64_t    wmi_tx_mgmt;
	uint64_t    wmi_tx_mgmt_completions;
	uint32_t    wmi_tx_mgmt_completion_err;
	uint32_t    peer_delete_req;
	uint32_t    peer_delete_resp;
	uint32_t    rx_mgmt_rssi_drop;
	uint32_t    tx_frame_count;
	uint32_t    rx_frame_count;
	uint32_t    rx_clear_count;
	uint32_t    cycle_count;
	uint32_t    phy_err_count;
	uint32_t    chan_tx_pwr;
	uint32_t    be_nobuf;
	uint32_t    tx_packets;
	uint32_t    rx_packets;
	uint32_t    tx_num_data;
	uint32_t    rx_num_data;
	uint32_t    tx_mcs[10];
	uint32_t    rx_mcs[10];
	uint64_t    rx_bytes;
	uint64_t    tx_bytes;
	uint32_t    tx_compaggr;
	uint32_t    rx_aggr;
	uint32_t    tx_bawadv;
	uint32_t    tx_compunaggr;
	uint32_t    rx_badcrypt;
	uint32_t    rx_badmic;
	uint32_t    rx_crcerr;
	uint32_t    rx_last_msdu_unset_cnt;
	uint32_t    rx_data_bytes;
	uint32_t    tx_retries;
};

/* Enumeration of PDEV Configuration parameter */
enum _ol_hal_param_t {
	OL_HAL_CONFIG_DMA_BEACON_RESPONSE_TIME = 0
};
#endif /* _CDP_TXRX_EXTD_STRUCT_H_ */
