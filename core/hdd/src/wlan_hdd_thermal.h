/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#ifndef __HDD_THERMAL_H
#define __HDD_THERMAL_H
/**
 * DOC: wlan_hdd_thermal.h
 * WLAN Host Device Driver thermal mitigation include file
 */

#include <linux/netdevice.h>
#include <net/cfg80211.h>

#ifdef FW_THERMAL_THROTTLE_SUPPORT

int
wlan_hdd_cfg80211_set_thermal_mitigation_policy(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data, int data_len);

/**
 * wlan_hdd_thermal_config_support() - thermal mitigation support
 *
 * Return: true if thermal mitigation support enabled otherwise false
 */
bool wlan_hdd_thermal_config_support(void);

#define FEATURE_THERMAL_VENDOR_COMMANDS				\
{								\
	.info.vendor_id = QCA_NL80211_VENDOR_ID,		\
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_THERMAL_CMD,	\
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV,			\
	.doit = wlan_hdd_cfg80211_set_thermal_mitigation_policy	\
},
#else
#define FEATURE_THERMAL_VENDOR_COMMANDS

static inline bool wlan_hdd_thermal_config_support(void)
{
	return false;
}

#endif /* FEATURE_THERMAL_VENDOR_COMMANDS */
#endif /* __HDD_THERMAL_H */
