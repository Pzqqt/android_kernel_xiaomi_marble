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
#include <linux/version.h>
#include <linux/netdevice.h>
#include <net/cfg80211.h>
#include <wlan_objmgr_cmn.h>

/**
 * wlan_cfg80211_register_cp_stats_cmd_handler() - api to register cfg80211
 * command handler for cp_stats
 * @pdev:    Pointer to pdev
 * @idx:     Index in function table
 * @handler: Pointer to function handler
 *
 * Return: void
 */
void wlan_cfg80211_register_cp_stats_cmd_handler
			(struct wlan_objmgr_pdev *pdev,
			 int idx, void *handler);

/**
 * wlan_cfg80211_unregister_cp_stats_cmd_handler() - api to unregister cfg80211
 * command handler for cp_stats
 * @pdev:    Pointer to pdev
 * @idx:     Index in function table
 * @handler: Pointer to function handler
 *
 * Return: void
 */
void wlan_cfg80211_unregister_cp_stats_cmd_handler
			(struct wlan_objmgr_pdev *pdev,
			 int idx, void *handler);

/**
 * wlan_cfg80211_cp_stats_get_atf_stats() - api to get atf stats
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Return: Return: 0 on success, negative errno on failure
 */
int wlan_cfg80211_cp_stats_get_atf_stats
			(struct wiphy *wiphy,
			 struct wlan_objmgr_pdev *pdev,
			 const void *data,
			 int data_len);

/**
 * wlan_cfg80211_cp_stats_get_vow_ext_stats() - api to get vow stats
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Return: Return: 0 on success, negative errno on failure
 */
int wlan_cfg80211_cp_stats_get_vow_ext_stats
			(struct wiphy *wiphy,
			 struct wlan_objmgr_pdev *pdev,
			 const void *data,
			 int data_len);

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CFG80211_IC_CP_STATS_H__ */
