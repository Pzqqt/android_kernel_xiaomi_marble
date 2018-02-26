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
 * @dcs_cck_phyerr: channel noise floor
 * @dcs_total_chan_util: total channel utilization
 * @dcs_tx_chan_util: tx channel utilization
 * @dcs_rx_chan_util: rx channel utilization
 * @dcs_self_bss_util: self BSS util
 * @dcs_other_bss_util: other BSS util
 * @dcs_wasted_chan_util: wasted chan util
 * @dcs_unused_chan_util: unused chan util
 * @dcs_ss_under_util: spatial stream under util
 * @dcs_sec_20_util: secondary 20MHz util
 * @dcs_sec_40_util: secondary 40Mhz util
 * @dcs_sec_80_util: secondary 80MHz util
 * @dcs_ofdm_phyerr: tx ofdm errors
 * @dcs_cck_phyerr: tx cck errors
 */
struct pdev_dcs_chan_stats {
	uint32_t dcs_chan_nf;
	uint32_t dcs_total_chan_util;
	uint32_t dcs_tx_chan_util;
	uint32_t dcs_rx_chan_util;
	uint32_t dcs_self_bss_util;
	uint32_t dcs_other_bss_util;
	uint32_t dcs_wasted_chan_util;
	uint32_t dcs_unused_chan_util;
	uint32_t dcs_ss_under_util;
	uint32_t dcs_sec_20_util;
	uint32_t dcs_sec_40_util;
	uint32_t dcs_sec_80_util;
	uint32_t dcs_ofdm_phyerr;
	uint32_t dcs_cck_phyerr;
};

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_IC_DCS_CHAN_STATS_H__ */
