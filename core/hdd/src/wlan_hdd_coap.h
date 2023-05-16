/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_hdd_coap.h
 *
 * Add Vendor subcommand QCA_NL80211_VENDOR_SUBCMD_COAP_OFFLOAD
 */

#ifndef __WLAN_HDD_COAP_H
#define __WLAN_HDD_COAP_H

#ifdef WLAN_FEATURE_COAP
#include <net/cfg80211.h>

/**
 * wlan_hdd_cfg80211_coap_offload() - configure CoAP offloading
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: pointer to netlink TLV buffer
 * @data_len: the length of @data in bytes
 *
 * Return: An error code or 0 on success.
 */
int wlan_hdd_cfg80211_coap_offload(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   const void *data, int data_len);

extern const struct nla_policy
	coap_offload_policy[QCA_WLAN_VENDOR_ATTR_COAP_OFFLOAD_MAX + 1];

#define FEATURE_COAP_OFFLOAD_COMMANDS					\
{									\
	.info.vendor_id = QCA_NL80211_VENDOR_ID,			\
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_COAP_OFFLOAD,		\
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |				\
		WIPHY_VENDOR_CMD_NEED_NETDEV |				\
		WIPHY_VENDOR_CMD_NEED_RUNNING,				\
	.doit = wlan_hdd_cfg80211_coap_offload,				\
	vendor_command_policy(coap_offload_policy,			\
			      QCA_WLAN_VENDOR_ATTR_COAP_OFFLOAD_MAX)	\
},
#else /* WLAN_FEATURE_COAP */
#define FEATURE_COAP_OFFLOAD_COMMANDS
#endif /* WLAN_FEATURE_COAP */

#endif /* __WLAN_HDD_COAP_H */
