/*
 * Copyright (c) 2011-2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cfg80211_mc_cp_stats.h
 *
 * This Header file provide declaration for cfg80211 command handler API
 * registered cp stats and specific with ic
 */

#ifndef __WLAN_CFG80211_MC_CP_STATS_H__
#define __WLAN_CFG80211_MC_CP_STATS_H__

#ifdef QCA_SUPPORT_CP_STATS

/* forward declaration */
struct wiphy;
struct wlan_objmgr_psoc;

/**
 * wlan_cfg80211_mc_cp_stats_get_wakelock_stats() - API to request wake lock
 * stats. Stats are returned to user space via vender event
 * @psoc:    Pointer to psoc
 * @wiphy:   wiphy pointer
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_mc_cp_stats_get_wakelock_stats(struct wlan_objmgr_psoc *psoc,
						 struct wiphy *wiphy);

/**
 * wlan_cfg80211_mc_cp_stats_get_tx_power() - API to fetch tx power
 * @vdev:    Pointer to vdev
 * @dbm:     Pointer to TX power in dbm
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_mc_cp_stats_get_tx_power(struct wlan_objmgr_vdev *vdev,
					   int *dbm);
#ifdef WLAN_FEATURE_MIB_STATS
/**
 * wlan_cfg80211_mc_cp_stats_get_mib_stats() - API to get mib stats
 * statistics from firmware
 * @vdev:    Pointer to vdev
 * @errno:   error type in case of failure
 *
 * Callers of this API must call wlan_cfg80211_mc_cp_stats_free_stats_event
 * API.
 * Return: stats buffer on success, Null on failure
 */
struct stats_event *
wlan_cfg80211_mc_cp_stats_get_mib_stats(struct wlan_objmgr_vdev *vdev,
					int *errno);
#endif

/**
 * wlan_cfg80211_mc_cp_stats_get_station_stats() - API to get station
 * statistics to firmware
 * @vdev:    Pointer to vdev
 * @errno:   error type in case of failure
 *
 * Call of this API must call wlan_cfg80211_mc_cp_stats_free_stats_event
 * API when done with information provided by info.
 * Return: stats buffer on success, Null on failure
 */
struct stats_event *
wlan_cfg80211_mc_cp_stats_get_station_stats(struct wlan_objmgr_vdev *vdev,
					    int *errno);

/**
 * wlan_cfg80211_mc_cp_stats_free_stats_event() - API to release station
 * statistics buffer
 * @vdev:    Pointer to vdev
 * @info:    pointer to object to populate with station stats
 *
 * Return: None
 */
void wlan_cfg80211_mc_cp_stats_free_stats_event(struct stats_event *info);

/**
 * wlan_cfg80211_mc_cp_stats_get_peer_rssi() - API to fetch peer rssi
 * @vdev:    Pointer to vdev
 * @macaddress: mac address
 * @errno:   error type in case of failure
 *
 * Call of this API must call wlan_cfg80211_mc_cp_stats_free_stats_event
 * API when done with information provided by rssi_info.
 * Return: stats buffer on success, Null on failure
 */
struct stats_event *
wlan_cfg80211_mc_cp_stats_get_peer_rssi(struct wlan_objmgr_vdev *vdev,
					uint8_t *macaddress, int *errno);

/**
 * wlan_cfg80211_mc_cp_stats_get_peer_stats() - API to get peer
 * statistics from firmware
 * @vdev:    Pointer to vdev
 * @mac_addr: mac address
 * @errno:   error type in case of failure
 *
 * Call of this API must call wlan_cfg80211_mc_cp_stats_free_stats_event
 * API when done with information provided by info.
 * Return: stats buffer on success, Null on failure
 */
struct stats_event *
wlan_cfg80211_mc_cp_stats_get_peer_stats(struct wlan_objmgr_vdev *vdev,
					 const uint8_t *mac_addr,
					 int *errno);
#else
static inline int wlan_cfg80211_mc_cp_stats_get_tx_power(
				struct wlan_objmgr_vdev *vdev,
				int *dbm)
{
	return 0;
}

static inline int wlan_cfg80211_mc_cp_stats_get_wakelock_stats(
				struct wlan_objmgr_psoc *psoc,
				struct wiphy *wiphy)
{
	return 0;
}

static inline struct stats_event *
wlan_cfg80211_mc_cp_stats_get_peer_rssi(struct wlan_objmgr_vdev *vdev,
					uint8_t *macaddress, int *errno)
{
	return NULL;
}

static inline void wlan_cfg80211_mc_cp_stats_free_stats_event(
			struct stats_event *info)
{}

static inline struct stats_event *
wlan_cfg80211_mc_cp_stats_get_station_stats(struct wlan_objmgr_vdev *vdev,
					    int *errno)
{
	return NULL;
}

static inline struct stats_event *
wlan_cfg80211_mc_cp_stats_get_peer_stats(struct wlan_objmgr_vdev *vdev,
					 const uint8_t *mac_addr,
					 int *errno)
{
	return NULL;
}
#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CFG80211_MC_CP_STATS_H__ */
