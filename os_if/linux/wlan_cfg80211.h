/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
 * DOC: declares driver functions interfacing with linux kernel
 */


#ifndef _WLAN_CFG80211_H_
#define _WLAN_CFG80211_H_

#include <linux/version.h>
#include <linux/netdevice.h>
#include <net/cfg80211.h>
#include <qca_vendor.h>

#define COMMON_VENDOR_COMMANDS						\
{ 									\
	.info.vendor_id = OUI_QCA,					\
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION,\
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |				\
		 WIPHY_VENDOR_CMD_NEED_NETDEV,				\
	.doit = NULL							\
},									\
{									\
	.info.vendor_id = OUI_QCA,					\
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION,\
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |				\
		WIPHY_VENDOR_CMD_NEED_NETDEV,				\
	.doit = NULL							\
},

#endif
