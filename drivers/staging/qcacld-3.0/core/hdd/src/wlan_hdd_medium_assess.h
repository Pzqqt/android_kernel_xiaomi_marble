/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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
 * DOC : wlan_hdd_medium_assess.h
 *
 * WLAN Host Device Driver medium assess related implementation
 *
 */

#if !defined(__WLAN_HDD_MEDIUM_ASSESS_H)
#define __WLAN_HDD_MEDIUM_ASSESS_H

#include <linux/netdevice.h>
#include <net/netlink.h>
#include <net/cfg80211.h>
#include <qca_vendor.h>
#include "wlan_hdd_main.h"

#ifdef WLAN_FEATURE_MEDIUM_ASSESS
#include "wlan_cp_stats_mc_defs.h"

#define MEDIUM_ASSESS_NUM 31

/**
 * struct hdd_medium_assess_config: configuration from framework
 * @interval: the update period to framework. An integral multiple of 1 second,
 *	      less or equal to 30 seconds
 * @threshold: threshold for congestion percentage of pdev
 */
struct hdd_medium_assess_config {
	uint8_t interval;
	uint8_t threshold;
};

/**
 * struct hdd_medium_assess_info: the medium assess info for pdev
 * @pdev_id: pdev id
 * @vdev_id: vdev id
 * @config: config info from user
 * @index: the data's index
 * @data: the raw info from fw
 * @count: the times of timer triggered
 */
struct hdd_medium_assess_info {
	uint8_t pdev_id;
	uint8_t vdev_id;
	struct hdd_medium_assess_config config;

	int32_t index;
	struct medium_assess_data data[MEDIUM_ASSESS_NUM];

	uint32_t count;
};

/* QCA_NL80211_VENDOR_SUBCMD_MEDIUM_ASSESS */
extern const struct nla_policy
hdd_medium_assess_policy[QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_MAX + 1];

/**
 * hdd_cfg80211_medium_assess() - medium assess
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: success(0) or reason code for failure
 */
int hdd_cfg80211_medium_assess(struct wiphy *wiphy,
			       struct wireless_dev *wdev,
			       const void *data,
			       int data_len);

#define FEATURE_MEDIUM_ASSESS_VENDOR_COMMANDS                         \
{                                                                     \
	.info.vendor_id = QCA_NL80211_VENDOR_ID,                      \
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_MEDIUM_ASSESS,       \
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |                         \
		WIPHY_VENDOR_CMD_NEED_NETDEV |                        \
		WIPHY_VENDOR_CMD_NEED_RUNNING,                        \
	.doit = hdd_cfg80211_medium_assess,                           \
	vendor_command_policy(hdd_medium_assess_policy,               \
			      QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_MAX) \
},

#define FEATURE_MEDIUM_ASSESS_VENDOR_EVENTS                \
[QCA_NL80211_VENDOR_SUBCMD_MEDIUM_ASSESS_INDEX] = {        \
	.vendor_id = QCA_NL80211_VENDOR_ID,                \
	.subcmd = QCA_NL80211_VENDOR_SUBCMD_MEDIUM_ASSESS, \
},

/**
 * hdd_medium_assess_init() - medium assess init timer
 *
 * Return: none
 */
void hdd_medium_assess_init(void);

/**
 * hdd_cfg80211_medium_deinit() - medium assess deinit timer
 *
 * Return: none
 */
void hdd_medium_assess_deinit(void);

/**
 * hdd_medium_assess_stop_timer() - medium assess reset and stop timer
 * @pdev_id: pdev id
 * @hdd_ctx: hdd context
 *
 * Return: none
 */
void hdd_medium_assess_stop_timer(uint8_t pdev_id, struct hdd_context *hdd_ctx);

/**
 * hdd_medium_assess_ssr_enable_flag() - medium assess set ssr enable flag
 *
 * Return: none
 */
void hdd_medium_assess_ssr_enable_flag(void);
#else
#define FEATURE_MEDIUM_ASSESS_VENDOR_COMMANDS
#define FEATURE_MEDIUM_ASSESS_VENDOR_EVENTS
static inline void hdd_medium_assess_init(void) {}
static inline void hdd_medium_assess_deinit(void) {}
static inline void hdd_medium_assess_stop_timer(uint8_t pdev_id,
						struct hdd_context *hdd_ctx) {}
static inline void hdd_medium_assess_ssr_enable_flag(void) {}
#endif
#endif /* end #if !defined(__WLAN_HDD_MEDIUM_ASSESS_H) */
