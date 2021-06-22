/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 * DOC:  wlan_hdd_avoid_freq_ext.h
 *
 * WLAN Host Device Driver extended avoid frequency interface implementation.
 */

#if !defined(__WLAN_HDD_AVOID_FREQ_EXT_H)
#define __WLAN_HDD_AVOID_FREQ_EXT_H

#include <linux/netdevice.h>
#include <net/netlink.h>
#include <net/cfg80211.h>
#include <qca_vendor.h>
#include "wlan_hdd_main.h"

#ifdef FEATURE_WLAN_CH_AVOID_EXT
extern const struct nla_policy
avoid_freq_ext_policy[QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_MAX + 1];

#define FEATURE_AVOID_FREQ_EXT_VENDOR_COMMANDS                          \
{                                                                       \
	.info.vendor_id = QCA_NL80211_VENDOR_ID,                        \
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY_EXT,   \
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |                           \
		WIPHY_VENDOR_CMD_NEED_NETDEV |                          \
		WIPHY_VENDOR_CMD_NEED_RUNNING,                          \
	.doit = wlan_hdd_cfg80211_avoid_freq_ext,                       \
	vendor_command_policy(avoid_freq_ext_policy,                    \
			      QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_MAX) \
},

int wlan_hdd_cfg80211_avoid_freq_ext(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     const void *data, int data_len);

#else
#define FEATURE_AVOID_FREQ_EXT_VENDOR_COMMANDS
static inline void wlan_hdd_cfg80211_avoid_freq_ext(void) {}
#endif
#endif

