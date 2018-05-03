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
 * DOC: wlan_cp_stats_ic_defs.h
 *
 * This header file maintain structure definitions for cp stats specific to ic
 */

#ifndef __WLAN_CP_STATS_IC_DEFS_H__
#define __WLAN_CP_STATS_IC_DEFS_H__

#ifdef QCA_SUPPORT_CP_STATS

/**
 * struct pdev_80211_stats - control plane stats at pdev
 *
 * the same statistics were earlier maintained with a reference to
 * ieee80211_mac_stats in vap structure, now the same structure will be
 * used as interface structure with user space application
 * make sure to align this structure with ieee80211_mac_stats
 *
 * @cs_tx_hw_retries:
 * @cs_tx_hw_failures:
 */
struct pdev_80211_stats {
	uint64_t cs_tx_hw_retries;
	uint64_t cs_tx_hw_failures;
};

/**
 * struct pdev_ic_cp_stats - control plane stats specific to WIN at pdev
 * @stats: 80211 stats
 */
struct pdev_ic_cp_stats {
	struct pdev_80211_stats stats;
};

/**
 * struct vdev_80211_stats - control plane 80211 stats at vdev
 *
 * the same statistics were earlier maintained with a reference to
 * ieee80211_mac_stats in vap structure, now the same structure will be
 * used as interface structure with user space application
 * make sure to align this structure with ieee80211_mac_stats
 *
 * @cs_rx_mgmt: rx mgmt
 */
struct vdev_80211_stats {
	uint64_t cs_rx_mgmt;
};

/**
 * struct vdev_80211_mac_stats - control plane 80211 mac stats at vdev
 *
 * the same statistics were earlier maintained with a reference to
 * ieee80211_mac_stats in vap structure, now the same structure will be
 * used as interface structure with user space application
 * make sure to align this structure with ieee80211_mac_stats
 *
 * @cs_rx_badkeyid: rx bad keyid
 * @cs_rx_decryptok: rx decrypt success
 * @cs_rx_wepfail: rx wep failures
 * @cs_rx_tkipreplay: rx tkip replays
 * @cs_rx_tkipformat: rx tkip format
 * @cs_rx_tkipicv: rx tkip icv
 * @cs_rx_ccmpreplay: rx ccmp replay
 * @cs_rx_ccmpformat: rx ccmp format
 * @cs_rx_ccmpmic: rx ccmp mic failures
 * @cs_rx_wpireplay: rx wpi replay
 * @cs_rx_wpimic: rx wpi mic failures
 * @cs_rx_countermeasure: rx counter measures count
 * @cs_retries: rx retries
 * @cs_tx_mgmt: tx mgmt
 * @cs_rx_mgmt: rx mgmt
 */
struct vdev_80211_mac_stats {
	uint64_t cs_rx_badkeyid;
	uint64_t cs_rx_decryptok;
	uint64_t cs_rx_wepfail;
	uint64_t cs_rx_tkipreplay;
	uint64_t cs_rx_tkipformat;
	uint64_t cs_rx_tkipicv;
	uint64_t cs_rx_ccmpreplay;
	uint64_t cs_rx_ccmpformat;
	uint64_t cs_rx_ccmpmic;
	uint64_t cs_rx_wpireplay;
	uint64_t cs_rx_wpimic;
	uint64_t cs_rx_countermeasure;
	uint64_t cs_retries;
	uint64_t cs_tx_mgmt;
	uint64_t cs_rx_mgmt;
};

/**
 * struct vdev_ic_cp_stats - control plane stats specific to WIN at vdev
 * @stats: 80211 stats
 * @ucast_stats: unicast stats
 * @mcast_stats: multicast or broadcast stats
 */
struct vdev_ic_cp_stats {
	struct vdev_80211_stats stats;
	struct vdev_80211_mac_stats ucast_stats;
	struct vdev_80211_mac_stats mcast_stats;
};

/**
 * struct peer_ic_cp_stats - control plane stats specific to WIN at peer
 * the same statistics were earlier maintained with a reference to
 * ieee80211_nodestats in ni structure, now the same structure will be
 * as interface structure with user space application
 * make sure to align this structure with ieee80211_nodestats always
 *
 *  @cs_rx_mgmt_rssi: rx mgmt rssi
 *  @cs_rx_mgmt: rx mgmt
 *  @cs_rx_noprivacy: rx no privacy
 *  @cs_rx_wepfail: rx wep failures
 *  @cs_rx_ccmpmic: rx ccmp mic failures
 *  @cs_rx_wpimic: rx wpi mic failures
 *  @cs_rx_tkipicv: rx tkip icv
 *  @cs_tx_mgmt: tx mgmt
 *  @cs_is_tx_not_ok: tx failures
 *  @cs_ps_discard: ps discard
 *  @cs_rx_mgmt_rate: rx mgmt rate
 *  @cs_tx_bytes_rate: tx rate
 *  @cs_tx_data_rate: tx data rate
 *  @cs_rx_bytes_rate: rx rate
 *  @cs_rx_data_rate: rx data rate
 *  @cs_tx_bytes_success_last: tx success count in last 1 sec
 *  @cs_tx_data_success_last: tx data success count in last 1 sec
 *  @cs_rx_bytes_last: rx rate
 *  @cs_rx_data_last: rx data rate
 *  @cs_psq_drops: psq drops
 *  @cs_tx_dropblock: tx dropblock
 *  @cs_tx_assoc: tx assoc success
 *  @cs_tx_assoc_fail: tx assoc failure
 */
struct peer_ic_cp_stats {
	int8_t   cs_rx_mgmt_rssi;
	uint32_t cs_rx_mgmt;
	uint32_t cs_rx_noprivacy;
	uint32_t cs_rx_wepfail;
	uint32_t cs_rx_ccmpmic;
	uint32_t cs_rx_wpimic;
	uint32_t cs_rx_tkipicv;
	uint32_t cs_tx_mgmt;
	uint32_t cs_is_tx_not_ok;
	uint32_t cs_ps_discard;
	uint32_t cs_rx_mgmt_rate;
#ifdef WLAN_ATH_SUPPORT_EXT_STAT
	uint32_t cs_tx_bytes_rate;
	uint32_t cs_tx_data_rate;
	uint32_t cs_rx_bytes_rate;
	uint32_t cs_rx_data_rate;
	uint32_t cs_tx_bytes_success_last;
	uint32_t cs_tx_data_success_last;
	uint32_t cs_rx_bytes_last;
	uint32_t cs_rx_data_last;
#endif
	uint32_t cs_psq_drops;
#ifdef ATH_SUPPORT_IQUE
	uint32_t cs_tx_dropblock;
#endif
	uint32_t cs_tx_assoc;
	uint32_t cs_tx_assoc_fail;
};

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_IC_DEFS_H__ */
