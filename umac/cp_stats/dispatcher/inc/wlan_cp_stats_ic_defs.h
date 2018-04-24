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
 * @cs_rx_mgmt: rx mgmt
 */
struct vdev_80211_mac_stats {
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
 *  @cs_rx_mgmt: rx mgmt
 */
struct peer_ic_cp_stats {
	uint32_t cs_rx_mgmt;
};

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_IC_DEFS_H__ */
