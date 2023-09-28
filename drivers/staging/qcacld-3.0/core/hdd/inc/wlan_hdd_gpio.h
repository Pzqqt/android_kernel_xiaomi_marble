/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_gpio.h
 *
 * This Header file provide declaration for cfg80211 command handler API
 */

#ifndef __WLAN_HDD_GPIO_H__
#define __WLAN_HDD_GPIO_H__

#include <qdf_types.h>
#include <net/cfg80211.h>
#include <qca_vendor.h>

#ifdef WLAN_FEATURE_GPIO_CFG
#include <wlan_cfg80211_gpio.h>

#define FEATURE_GPIO_CFG_VENDOR_COMMANDS \
{ \
	.info.vendor_id = QCA_NL80211_VENDOR_ID, \
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GPIO_CONFIG_COMMAND, \
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV | \
		 WIPHY_VENDOR_CMD_NEED_NETDEV | \
		 WIPHY_VENDOR_CMD_NEED_RUNNING, \
	.doit = wlan_hdd_cfg80211_set_gpio_config, \
	vendor_command_policy(wlan_cfg80211_gpio_config_policy, \
			      QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_MAX) \
},

/**
 * wlan_hdd_cfg80211_set_gpio_config() - set GPIO config
 * @wiphy: wiphy structure pointer
 * @wdev: Wireless device structure pointer
 * @data: Pointer to the data to be passed via vendor interface
 * @data_len: Length of the data to be passed
 *
 * Return: Return the Success or Failure code
 */
int wlan_hdd_cfg80211_set_gpio_config(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      const void *data, int data_len);
#else
#define FEATURE_GPIO_CFG_VENDOR_COMMANDS
#endif /* WLAN_FEATURE_GPIO_CFG */
#endif /* __WLAN_CFG80211_GPIO_CFG_H__ */
