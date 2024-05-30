/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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

/**
 * DOC: contains dcs structure definations
 */

#ifndef _WLAN_DCS_PUBLIC_STRUCTS_H_
#define _WLAN_DCS_PUBLIC_STRUCTS_H_

/**
 * enum wlan_host_dcs_type - types of DCS interference events
 * @WLAN_HOST_DCS_NONE: invalid type
 * @WLAN_HOST_DCS_CWIM: continuous wave interference
 * @WLAN_HOST_DCS_WLANIM: wlan interference stats
 * @WLAN_HOST_DCS_AWGNIM: additive white Gaussian noise (awgn) interference
 */
enum wlan_host_dcs_type {
	WLAN_HOST_DCS_NONE   = 0,      /* 0x0 */
	WLAN_HOST_DCS_CWIM   = BIT(0), /* 0x1 */
	WLAN_HOST_DCS_WLANIM = BIT(1), /* 0x2 */
	WLAN_HOST_DCS_AWGNIM = BIT(2), /* 0x4 */
};

/**
 * struct wlan_host_dcs_interference_param - dcs interference parameters
 * @interference_type: type of DCS interference
 * @uint32_t pdev_id: pdev id
 */
struct wlan_host_dcs_interference_param {
	uint32_t interference_type;
	uint32_t pdev_id;
};

/**
 * struct wlan_host_dcs_mib_stats - WLAN IM stats from target to host
 * @listen_time:
 * @reg_tx_frame_cnt:
 * @reg_rx_frame_cnt:
 * @reg_rxclr_cnt:
 * @reg_cycle_cnt: delta cycle count
 * @reg_rxclr_ext_cnt:
 * @reg_ofdm_phyerr_cnt:
 * @reg_cck_phyerr_cnt: CCK err count since last reset, read from register
 */
struct wlan_host_dcs_mib_stats {
	int32_t listen_time;
	uint32_t reg_tx_frame_cnt;
	uint32_t reg_rx_frame_cnt;
	uint32_t reg_rxclr_cnt;
	uint32_t reg_cycle_cnt;
	uint32_t reg_rxclr_ext_cnt;
	uint32_t reg_ofdm_phyerr_cnt;
	uint32_t reg_cck_phyerr_cnt;
};

/**
 * struct wlan_host_dcs_im_tgt_stats - DCS IM target stats
 * @reg_tsf32: current running TSF from the TSF-1
 * @last_ack_rssi: known last frame rssi, in case of multiple stations, if
 *      and at different ranges, this would not gaurantee that
 *      this is the least rssi.
 * @tx_waste_time: sum of all the failed durations in the last
 *      one second interval.
 * @rx_time: count how many times the hal_rxerr_phy is marked, in this
 *      time period
 * @phyerr_cnt:
 * @mib_stats: collected mib stats as explained in mib structure
 * @chan_nf: Channel noise floor (units are in dBm)
 * @my_bss_rx_cycle_count: BSS rx cycle count
 * @reg_rxclr_ext40_cnt: extension channel 40Mhz rxclear count
 * @reg_rxclr_ext80_cnt: extension channel 80Mhz rxclear count
 */
struct wlan_host_dcs_im_tgt_stats {
	uint32_t reg_tsf32;
	uint32_t last_ack_rssi;
	uint32_t tx_waste_time;
	uint32_t rx_time;
	uint32_t phyerr_cnt;
	struct wlan_host_dcs_mib_stats mib_stats;
	uint32_t chan_nf;
	uint32_t my_bss_rx_cycle_count;
	uint32_t reg_rxclr_ext40_cnt;
	uint32_t reg_rxclr_ext80_cnt;
};

/**
 * struct wlan_host_dcs_ch_util_stats - DCS IM chan utilization stats
 * @rx_cu: rx channel utilization
 * @tx_cu: tx channel utilization
 * @rx_obss_cu: obss rx channel utilization
 * @total_cu: total channel utilization
 * @chan_nf: Channel noise floor (units are in dBm)
 */
struct wlan_host_dcs_ch_util_stats {
	uint32_t rx_cu;
	uint32_t tx_cu;
	uint32_t obss_rx_cu;
	uint32_t total_cu;
	uint32_t chan_nf;
};

/**
 * struct wlan_host_dcs_im_user_stats - DCS IM stats requested by userspace
 * @max_rssi: max rssi of the bss traffic
 * @min_rssi: min rssi of the bss traffic
 * @cycle_count: cycle count
 * @rxclr_count: rx clear count
 * @rx_frame_count: rx frame count
 * @my_bss_rx_cycle_count: BSS rx cycle count
 */
struct wlan_host_dcs_im_user_stats {
	uint32_t max_rssi;
	uint32_t min_rssi;
	uint32_t cycle_count;
	uint32_t rxclr_count;
	uint32_t rx_frame_count;
	uint32_t my_bss_rx_cycle_count;
};

/**
 * struct wlan_host_dcs_awgn_info - DCS AWGN info
 * @channel_width: Channel width, enum phy_ch_width
 * @center_freq: Center frequency of primary channel
 * @center_freq0: Center frequency of segment 1
 * @center_freq1: Center frequency of segment 2
 * @chan_bw_intf_bitmap: Per-20MHz interference bitmap, each bit
 *  indicates 20MHz in which interference is seen, e.g.
 *  bit0 - primary 20MHz, bit1 - secondary 20MHz,
 *  bit2 - secondary 40MHz Lower, bit3 - secondary 40MHz Upper
 */
struct wlan_host_dcs_awgn_info {
	enum phy_ch_width channel_width;
	qdf_freq_t center_freq;
	qdf_freq_t center_freq0;
	qdf_freq_t center_freq1;
	uint32_t chan_bw_intf_bitmap;
};

/**
 * struct wlan_host_dcs_event - define dcs event
 * @wlan_stat: wlan interference target statistics
 * @dcs_param: dcs event param
 * @awgn_info: awgn info
 */
struct wlan_host_dcs_event {
	struct wlan_host_dcs_im_tgt_stats wlan_stat;
	struct wlan_host_dcs_interference_param dcs_param;
	struct wlan_host_dcs_awgn_info awgn_info;
};
#endif
