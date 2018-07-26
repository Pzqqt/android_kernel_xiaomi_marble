/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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

#ifndef __WLAN_HDD_SAR_LIMITS_H
#define __WLAN_HDD_SAR_LIMITS_H

/**
 * DOC: wlan_hdd_sar_limits_h
 *
 * WLAN Host Device Driver SAR limits API specification
 */

#ifdef FEATURE_SAR_LIMITS
/**
 * wlan_hdd_cfg80211_get_sar_power_limits() - Get SAR power limits
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Wrapper function of __wlan_hdd_cfg80211_get_sar_power_limits()
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_get_sar_power_limits(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data,
					   int data_len);

#define FEATURE_SAR_LIMITS_VENDOR_COMMANDS				\
{									\
	.info.vendor_id = QCA_NL80211_VENDOR_ID,			\
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS,	\
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |				\
		 WIPHY_VENDOR_CMD_NEED_RUNNING,				\
	.doit = wlan_hdd_cfg80211_get_sar_power_limits			\
},
#else /* FEATURE_SAR_LIMITS */
#define FEATURE_SAR_LIMITS_VENDOR_COMMANDS
#endif /* FEATURE_SAR_LIMITS */

#endif /* __WLAN_HDD_SAR_LIMITS_H */

