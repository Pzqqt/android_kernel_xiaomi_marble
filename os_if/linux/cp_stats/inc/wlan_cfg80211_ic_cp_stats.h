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
 * DOC: wlan_cfg80211_ic_cp_stats.h
 *
 * This Header file provide declaration for cfg80211 command handler API
 * registered cp stats and specific with ic
 */

#ifndef __WLAN_CFG80211_IC_CP_STATS_H__
#define __WLAN_CFG80211_IC_CP_STATS_H__

#ifdef QCA_SUPPORT_CP_STATS
#include <wlan_objmgr_cmn.h>
#include <wlan_cp_stats_ic_defs.h>
#ifdef WLAN_ATF_ENABLE
#include <wlan_cp_stats_ic_atf_defs.h>
#endif
#include <wlan_cp_stats_ic_dcs_defs.h>

/**
 * wlan_cfg80211_get_peer_cp_stats() - API to get peer stats object
 * @peer_obj: peer object as input
 * @peer_cp_stats: peer stats object to populate
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_get_peer_cp_stats(struct wlan_objmgr_peer *peer_obj,
				    struct peer_ic_cp_stats *peer_cp_stats);

/**
 * wlan_cfg80211_get_vdev_cp_stats() - API to get vdev stats object
 * @vdev_obj: vdev object as input
 * @vdev_cp_stats: vdev stats object to populate
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_get_vdev_cp_stats(struct wlan_objmgr_vdev *vdev_obj,
				    struct vdev_ic_cp_stats *vdev_cp_stats);

/**
 * wlan_cfg80211_get_pdev_cp_stats() - API to get pdev cp stats object
 * @pdev_obj: pdev object as input
 * @pdev_cp_stats: pdev cp stats object to populate
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_get_pdev_cp_stats(struct wlan_objmgr_pdev *pdev_obj,
				    struct pdev_ic_cp_stats *pdev_cp_stats);

#ifdef WLAN_ATF_ENABLE
/**
 * wlan_cfg80211_get_peer_atf_cp_stats() - API to get ATF peer stats object
 * @peer_obj: peer object as input
 * @atf_cp_stats: atf peer cp stats object to populate
 *
 * Return: 0 on success, negative value on failure
 */
int
wlan_cfg80211_get_atf_peer_cp_stats(struct wlan_objmgr_peer *peer_obj,
				    struct atf_peer_cp_stats *atf_cp_stats);

/**
 * wlan_cfg80211_get_peer_atf_cp_stats_from_mac() - API to get ATF peer
 * stats object from peer mac address
 * @vdev_obj: vdev object as input
 * @mac: peer mac address as input
 * @atf_cp_stats: atf peer cp stats object to populate
 *
 * API used from ucfg layer to get ATF peer cp stats object when only peer
 * mac address is available
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_get_atf_peer_cp_stats_from_mac(
		struct wlan_objmgr_vdev *vdev_obj,
		uint8_t *mac,
		struct atf_peer_cp_stats *atf_cp_stats);
#endif /* WLAN_ATF_ENABLE */

/**
 * wlan_cfg80211_get_dcs_pdev_cp_stats() - API to get DCS chan stats
 * @pdev_obj: pdev object as input
 * @dcs_chan_stats: DCS pdev stats object to populate
 *
 * Return: 0 on success, negative value on failure
 */
int wlan_cfg80211_get_dcs_pdev_cp_stats(
		struct wlan_objmgr_pdev *pdev_obj,
		struct pdev_dcs_chan_stats *dcs_chan_stats);

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CFG80211_IC_CP_STATS_H__ */
