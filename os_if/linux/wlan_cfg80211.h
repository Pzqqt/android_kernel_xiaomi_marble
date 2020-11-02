/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
#include <net/netlink.h>
#include <net/cfg80211.h>
#include <qca_vendor.h>
#include <qdf_nbuf.h>
#include "qal_devcfg.h"

#define osif_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_OS_IF, params)
#define osif_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_OS_IF, params)
#define osif_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_OS_IF, params)
#define osif_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_OS_IF, params)
#define osif_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_OS_IF, params)
#define osif_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_OS_IF, params)
#define osif_rl_debug(params...) \
	QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_OS_IF, params)
#define osif_err_rl(params...) \
	QDF_TRACE_ERROR_RL(QDF_MODULE_ID_OS_IF, params)

#define osif_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_OS_IF, params)
#define osif_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_OS_IF, params)
#define osif_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_OS_IF, params)
#define osif_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_OS_IF, params)
#define osif_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_OS_IF, params)

#define osif_enter_dev(dev) \
	QDF_TRACE_ENTER(QDF_MODULE_ID_OS_IF, "enter(%s)", (dev)->name)

/**
 * enum qca_nl80211_vendor_subcmds_index - vendor sub commands index
 *
 * @QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY_INDEX: Avoid frequency
 * @QCA_NL80211_VENDOR_SUBCMD_NAN_INDEX: Nan
 * @QCA_NL80211_VENDOR_SUBCMD_STATS_EXT_INDEX: Ext stats
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START_INDEX: Ext scan start
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP_INDEX: Ext scan stop
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES_INDEX: Ext scan get
 *	capability
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS_INDEX: Ext scan get
 *	cached results
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE_INDEX: Ext scan
 *	results available
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT_INDEX: Ext scan full
 *	scan result
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT_INDEX: Ext scan event
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND_INDEX: Ext scan hot list
 *	AP found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST_INDEX: Ext scan set
 *	bssid hotlist
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST_INDEX: Ext scan reset
 *	bssid hotlist
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE_INDEX: Ext scan
 *	significant change
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE_INDEX: Ext scan
 *	set significant change
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE_INDEX: Ext scan
 *	reset significant change
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET_INDEX: Set stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET_INDEX: Get stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR_INDEX: Clear stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_RADIO_STATS_INDEX: Radio stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_IFACE_STATS_INDEX: Iface stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_PEER_INFO_STATS_INDEX: Peer info stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT_INDEX: MAC layer counters
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE_CHANGE_INDEX: Ext tdls state change
 * @QCA_NL80211_VENDOR_SUBCMD_DO_ACS_INDEX: ACS command
 * @QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH_INDEX: Pass Roam and Auth info
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_LOST_INDEX: hotlist ap lost
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND_INDEX:
 *	pno network found index
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND_INDEX:
 *	passpoint match found index
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SSID_HOTLIST_INDEX:
 *	set ssid hotlist index
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SSID_HOTLIST_INDEX:
 *	reset ssid hotlist index
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_FOUND_INDEX:
 *	hotlist ssid found index
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_LOST_INDEX:
 *	hotlist ssid lost index
 * @QCA_NL80211_VENDOR_SUBCMD_DCC_STATS_EVENT_INDEX
 *	dcc stats event index
 * @QCA_NL80211_VENDOR_SUBCMD_SCAN_INDEX: vendor scan index
 * @QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE_INDEX:
 *	vendor scan complete event  index
 * @QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG_INDEX:
 *	update gateway parameters index
 * @QCA_NL80211_VENDOR_SUBCMD_INTEROP_ISSUES_AP_INDEX:
 *	update aps info which has interop issues events index
 * @QCA_NL80211_VENDOR_SUBCMD_TSF_INDEX: TSF response events index
 * @QCA_NL80211_VENDOR_SUBCMD_P2P_LO_EVENT_INDEX:
 *      P2P listen offload index
 * @QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH_INDEX: SAP
 *      conditional channel switch index
 * @QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET_INDEX: NUD DEBUG Stats index
 * @QCA_NL80211_VENDOR_SUBCMD_HANG_REASON_INDEX: hang event reason index
 * @QCA_NL80211_VENDOR_SUBCMD_WLAN_MAC_INFO_INDEX: MAC mode info index
 * @QCA_NL80211_VENDOR_SUBCMD_NAN_EXT_INDEX: NAN Extended index
 * @QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING_INDEX: Beacon reporting index
 * @QCA_NL80211_VENDOR_SUBCMD_REQUEST_SAR_LIMITS_INDEX: Request SAR limit index
 * @QCA_NL80211_VENDOR_SUBCMD_UPDATE_STA_INFO_INDEX: Update STA info index
 * @QCA_NL80211_VENDOR_SUBCMD_UPDATE_SSID_INDEX: Update SSID index
 * @QCA_NL80211_VENDOR_SUBCMD_WIFI_FW_STATS_INDEX: Wifi FW stats index
 * @QCA_NL80211_VENDOR_SUBCMD_MBSSID_TX_VDEV_STATUS_INDEX:
 *	MBSSID TX VDEV status index
 * @QCA_NL80211_VENDOR_SUBCMD_THERMAL_INDEX: Report thermal event index
 */

enum qca_nl80211_vendor_subcmds_index {
	QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY_INDEX = 0,

	QCA_NL80211_VENDOR_SUBCMD_NAN_INDEX,

#ifdef WLAN_FEATURE_STATS_EXT
	QCA_NL80211_VENDOR_SUBCMD_STATS_EXT_INDEX,
#endif /* WLAN_FEATURE_STATS_EXT */

#ifdef FEATURE_WLAN_EXTSCAN
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE_INDEX,
#endif /* FEATURE_WLAN_EXTSCAN */

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LL_RADIO_STATS_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LL_IFACE_STATS_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LL_PEER_INFO_STATS_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT_INDEX,
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

	QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE_CHANGE_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_DO_ACS_INDEX,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH_INDEX,
#endif
	/* DFS */
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_STARTED_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_FINISHED_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_ABORTED_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_NOP_FINISHED_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_RADAR_DETECTED_INDEX,
#ifdef FEATURE_WLAN_EXTSCAN
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_LOST_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SSID_HOTLIST_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SSID_HOTLIST_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_FOUND_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_LOST_INDEX,
#endif /* FEATURE_WLAN_EXTSCAN */
	QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_MONITOR_RSSI_INDEX,
#ifdef WLAN_FEATURE_MEMDUMP
	QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_MEMORY_DUMP_INDEX,
#endif /* WLAN_FEATURE_MEMDUMP */
	/* OCB events */
	QCA_NL80211_VENDOR_SUBCMD_DCC_STATS_EVENT_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_SCAN_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_INTEROP_ISSUES_AP_INDEX,
#ifdef WLAN_FEATURE_TSF
	QCA_NL80211_VENDOR_SUBCMD_TSF_INDEX,
#endif
	QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_P2P_LO_EVENT_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_UPDATE_EXTERNAL_ACS_CONFIG,
	QCA_NL80211_VENDOR_SUBCMD_PWR_SAVE_FAIL_DETECTED_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_HANG_REASON_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_WLAN_MAC_INFO_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_NAN_EXT_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_THROUGHPUT_CHANGE_EVENT_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LINK_PROPERTIES_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_PEER_STATS_CACHE_FLUSH_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_ROAM_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_OEM_DATA_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_REQUEST_SAR_LIMITS_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_UPDATE_STA_INFO_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_MEDIUM_ASSESS_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_UPDATE_SSID_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_WIFI_FW_STATS_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_MBSSID_TX_VDEV_STATUS_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_THERMAL_INDEX,
};

#if !defined(SUPPORT_WDEV_CFG80211_VENDOR_EVENT_ALLOC) && \
	(LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)) && \
	!defined(WITH_BACKPORTS)

static inline struct sk_buff *
backported_cfg80211_vendor_event_alloc(struct wiphy *wiphy,
				       struct wireless_dev *wdev,
				       int approxlen,
				       int event_idx, gfp_t gfp)
{
	struct sk_buff *skb;

	skb = cfg80211_vendor_event_alloc(wiphy, approxlen, event_idx, gfp);

	if (skb && wdev) {
		struct nlattr *attr;
		u32 ifindex = wdev->netdev->ifindex;

		nla_nest_cancel(skb, ((void **)skb->cb)[2]);
		if (nla_put_u32(skb, NL80211_ATTR_IFINDEX, ifindex))
			goto nla_fail;

		attr = nla_nest_start(skb, NL80211_ATTR_VENDOR_DATA);
		((void **)skb->cb)[2] = attr;
	}

	return skb;

nla_fail:
	kfree_skb(skb);

	return NULL;
}

#define cfg80211_vendor_event_alloc backported_cfg80211_vendor_event_alloc
#endif

/* For kernel version >= 5.2, driver needs to provide policy */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
#define vendor_command_policy(__policy, __maxattr) \
	.policy = __policy,                        \
	.maxattr = __maxattr
#define VENDOR_NLA_POLICY_NESTED(__policy) \
	NLA_POLICY_NESTED(__policy)
#else
#define vendor_command_policy(__policy, __maxattr)
#define VENDOR_NLA_POLICY_NESTED(__policy) {.type = NLA_NESTED}
#endif /*End of (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0) */

/* For kernel version <= 4.20, driver needs to provide policy */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0))
#define VENDOR_NLA_POLICY_MAC_ADDR NLA_POLICY_ETH_ADDR
#else
#define VENDOR_NLA_POLICY_MAC_ADDR \
	{.type = NLA_UNSPEC, .len = QDF_MAC_ADDR_SIZE}
#define NLA_EXACT_LEN NLA_UNSPEC
#endif /*End of (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 20, 0) */

#if defined(NBUF_MEMORY_DEBUG) && defined(NETLINK_BUF_TRACK)
#define wlan_cfg80211_vendor_free_skb(skb) \
	qdf_nbuf_free(skb)

#define wlan_cfg80211_vendor_event(skb, gfp) \
{ \
	qdf_nbuf_count_dec(skb); \
	qdf_net_buf_debug_release_skb(skb); \
	cfg80211_vendor_event(skb, gfp); \
}

#define wlan_cfg80211_vendor_cmd_reply(skb) \
{ \
	qdf_nbuf_count_dec(skb); \
	qdf_net_buf_debug_release_skb(skb); \
	cfg80211_vendor_cmd_reply(skb); \
}

static inline QDF_STATUS wlan_cfg80211_qal_devcfg_send_response(qdf_nbuf_t skb)
{
	qdf_nbuf_count_dec(skb);
	qdf_net_buf_debug_release_skb(skb);
	return qal_devcfg_send_response(skb);
}

static inline struct sk_buff *
__cfg80211_vendor_cmd_alloc_reply_skb(struct wiphy *wiphy, int len,
				      const char *func, uint32_t line)
{
	struct sk_buff *skb;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len);
	if (skb) {
		qdf_nbuf_count_inc(skb);
		qdf_net_buf_debug_acquire_skb(skb, func, line);
	}
	return skb;
}
#define wlan_cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len) \
	__cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len, __func__, __LINE__)

static inline struct sk_buff *
__cfg80211_vendor_event_alloc(struct wiphy *wiphy,
			      struct wireless_dev *wdev,
			      int approxlen,
			      int event_idx,
			      gfp_t gfp,
			      const char *func,
			      uint32_t line)
{
	struct sk_buff *skb;

	skb = cfg80211_vendor_event_alloc(wiphy, wdev,
					  approxlen,
					  event_idx,
					  gfp);
	if (skb) {
		qdf_nbuf_count_inc(skb);
		qdf_net_buf_debug_acquire_skb(skb, func, line);
	}
	return skb;
}
#define wlan_cfg80211_vendor_event_alloc(wiphy, wdev, len, idx, gfp) \
	__cfg80211_vendor_event_alloc(wiphy, wdev, len, \
				      idx, gfp, \
				      __func__, __LINE__)
#else /* NBUF_MEMORY_DEBUG && NETLINK_BUF_TRACK */
#define wlan_cfg80211_vendor_free_skb(skb) \
	kfree_skb(skb)

#define wlan_cfg80211_vendor_event(skb, gfp) \
	cfg80211_vendor_event(skb, gfp)

#define wlan_cfg80211_vendor_cmd_reply(skb) \
	cfg80211_vendor_cmd_reply(skb)

#define wlan_cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len) \
	cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len)

#define wlan_cfg80211_vendor_event_alloc(wiphy, wdev, len, idx, gfp) \
	cfg80211_vendor_event_alloc(wiphy, wdev, len, idx, gfp)

static inline QDF_STATUS wlan_cfg80211_qal_devcfg_send_response( qdf_nbuf_t skb)
{
	return qal_devcfg_send_response(skb);
}
#endif /* NBUF_MEMORY_DEBUG && NETLINK_BUF_TRACK */

#undef nla_parse
#undef nla_parse_nested
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
static inline int wlan_cfg80211_nla_parse(struct nlattr **tb,
					  int maxtype,
					  const struct nlattr *head,
					  int len,
					  const struct nla_policy *policy)
{
	return nla_parse(tb, maxtype, head, len, policy);
}

static inline int
wlan_cfg80211_nla_parse_nested(struct nlattr *tb[],
			       int maxtype,
			       const struct nlattr *nla,
			       const struct nla_policy *policy)
{
	return nla_parse_nested(tb, maxtype, nla, policy);
}
#else
static inline int wlan_cfg80211_nla_parse(struct nlattr **tb,
					  int maxtype,
					  const struct nlattr *head,
					  int len,
					  const struct nla_policy *policy)
{
	return nla_parse(tb, maxtype, head, len, policy, NULL);
}

static inline int
wlan_cfg80211_nla_parse_nested(struct nlattr *tb[],
			       int maxtype,
			       const struct nlattr *nla,
			       const struct nla_policy *policy)
{
	return nla_parse_nested(tb, maxtype, nla, policy, NULL);
}
#endif
#define nla_parse(...) (obsolete, use wlan_cfg80211_nla_parse)
#define nla_parse_nested(...) (obsolete, use wlan_cfg80211_nla_parse_nested)

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0))
static inline int
wlan_cfg80211_nla_put_u64(struct sk_buff *skb, int attrtype, u64 value)
{
	return nla_put_u64(skb, attrtype, value);
}
#else
static inline int
wlan_cfg80211_nla_put_u64(struct sk_buff *skb, int attrtype, u64 value)
{
	return nla_put_u64_64bit(skb, attrtype, value, NL80211_ATTR_PAD);
}
#endif
#endif
