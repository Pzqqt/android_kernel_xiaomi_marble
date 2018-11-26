/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_cp_stats_ic_dcs_defs.h
 *
 * This header file provide structure definitions for DCS control plane stats
 */

#ifndef __WLAN_CP_STATS_IC_DCS_CHAN_STATS_H__
#define __WLAN_CP_STATS_IC_DCS_CHAN_STATS_H__

#ifdef QCA_SUPPORT_CP_STATS

/**
 * struct pdev_dcs_chan_stats - DCS statistics
 * @dcs_total_util: total channel utilization percentage
 * @dcs_ap_tx_util: tx channel utilization percentage
 * @dcs_ap_rx_util: rx channel utilization percentage
 * @dcs_self_bss_util: self BSS utilization percentage
 * @dcs_obss_util: other BSS utilization percentage
 * @dcs_obss_rx_util: total Rx channel utilization percentage
 * @dcs_free_medium: free medium percentage
 * @dcs_non_wifi_util: non wifi utilization percentage
 * @dcs_ss_under_util: spatial stream under util
 * @dcs_sec_20_util: secondary 20MHz util
 * @dcs_sec_40_util: secondary 40Mhz util
 * @dcs_sec_80_util: secondary 80MHz util
 */
struct pdev_dcs_chan_stats {
	uint8_t dcs_total_util;
	uint8_t dcs_ap_tx_util;
	uint8_t dcs_ap_rx_util;
	uint8_t dcs_self_bss_util;
	uint8_t dcs_obss_util;
	uint8_t dcs_obss_rx_util;
	uint8_t dcs_free_medium;
	uint8_t dcs_non_wifi_util;
	uint32_t dcs_ss_under_util;
	uint32_t dcs_sec_20_util;
	uint32_t dcs_sec_40_util;
	uint32_t dcs_sec_80_util;
};

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_IC_DCS_CHAN_STATS_H__ */
