/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: feature_bcn_recv
 * Feature for receiving beacons of connected AP and sending select
 * params to upper layer via vendor event
 */

#ifdef WLAN_BCN_RECV_FEATURE

struct wireless_dev;
struct wiphy;

/**
 * wlan_hdd_cfg80211_bcn_rcv_start() - Process bcn recv start ind
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Wrapper function of __wlan_hdd_cfg80211_bcn_rcv_start()
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_bcn_rcv_start(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    const void *data, int data_len);

#define BCN_RECV_FEATURE_VENDOR_COMMANDS				\
{									\
	.info.vendor_id = QCA_NL80211_VENDOR_ID,			\
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING,	\
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |				\
		WIPHY_VENDOR_CMD_NEED_RUNNING,				\
	.doit = wlan_hdd_cfg80211_bcn_rcv_start				\
},

#define BCN_RECV_FEATURE_VENDOR_EVENTS			\
[QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING_INDEX] = {		\
	.vendor_id = QCA_NL80211_VENDOR_ID,			\
	.subcmd = QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING	\
},
#else
#define BCN_RECV_FEATURE_VENDOR_COMMANDS
#define BCN_RECV_FEATURE_VENDOR_EVENTS
#endif

