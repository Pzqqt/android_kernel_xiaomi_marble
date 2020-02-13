/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#endif
