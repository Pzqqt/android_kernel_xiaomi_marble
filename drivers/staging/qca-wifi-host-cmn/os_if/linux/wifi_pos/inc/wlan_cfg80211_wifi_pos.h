/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_cfg80211_wifi_pos.h
 * declares wifi pos module driver functions interfacing with linux kernel
 */

#ifndef _WLAN_CFG80211_WIFI_POS_H_
#define _WLAN_CFG80211_WIFI_POS_H_

#include <linux/version.h>
#include <linux/netdevice.h>
#include <net/netlink.h>
#include <net/cfg80211.h>

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)) || \
		defined(CFG80211_PASN_SUPPORT)) && \
		defined(WIFI_POS_CONVERGED)
void
wlan_wifi_pos_cfg80211_set_wiphy_ext_feature(struct wiphy *wiphy,
					     struct wlan_objmgr_psoc *psoc);
#else
static inline void
wlan_wifi_pos_cfg80211_set_wiphy_ext_feature(struct wiphy *wiphy,
					     struct wlan_objmgr_psoc *psoc)
{}
#endif /* WIFI_POS_CONVERGED */
#endif /* _WLAN_CFG80211_WIFI_POS_H_ */
