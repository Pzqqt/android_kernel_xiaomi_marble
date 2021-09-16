/*
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_hdd_txq_flush.h
 *
 * WLAN Host Device Driver peer Tx queue flush config interface definitions
 *
 */
#ifndef WLAN_HDD_PEER_TXQ_FLUSH_H
#define WLAN_HDD_PEER_TXQ_FLUSH_H
#include "qdf_types.h"
#include "qdf_status.h"
#include "qca_vendor.h"
#include <net/cfg80211.h>

#ifdef WLAN_FEATURE_PEER_TXQ_FLUSH_CONF
extern const struct nla_policy
peer_txq_flush_policy[QCA_WLAN_VENDOR_ATTR_FLUSH_PENDING_MAX + 1];

/**
 * wlan_hdd_cfg80211_peer_txq_flush_config() - Set peer tx queues flush config
 * @wiphy: Wireless info object
 * @wdev: Wireless dev object
 * @attr: Command attributes
 * @attr_len: Length of attributes
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_peer_txq_flush_config(struct wiphy *wiphy,
					    struct wireless_dev *wdev,
					    const void *attr,
					    int attr_len);

#define	FEATURE_PEER_FLUSH_VENDOR_COMMANDS \
{ \
	.info.vendor_id = QCA_NL80211_VENDOR_ID, \
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_PEER_FLUSH_PENDING, \
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV | \
		 WIPHY_VENDOR_CMD_NEED_NETDEV | \
		 WIPHY_VENDOR_CMD_NEED_RUNNING, \
	.doit = wlan_hdd_cfg80211_peer_txq_flush_config, \
	vendor_command_policy(peer_txq_flush_policy, \
	QCA_WLAN_VENDOR_ATTR_FLUSH_PENDING_MAX) \
},
#else /* WLAN_FEATURE_PEER_TXQ_FLUSH_CONF */
#define	FEATURE_PEER_FLUSH_VENDOR_COMMANDS
#endif /* WLAN_FEATURE_PEER_TXQ_FLUSH_CONF */
#endif /* WLAN_HDD_PEER_TXQ_FLUSH_H */
