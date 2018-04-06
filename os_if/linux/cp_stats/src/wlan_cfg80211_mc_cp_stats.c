/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: wlan_cfg80211_mc_cp_stats.c
 *
 * This file provide definitions to cp stats supported cfg80211 cmd handlers
 */

#include <wlan_cfg80211.h>
#include <wlan_cp_stats_mc_defs.h>
#include <wlan_cp_stats_mc_ucfg_api.h>
#include <wlan_cfg80211_mc_cp_stats.h>

/**
 * wlan_cfg80211_mc_cp_stats_send_wake_lock_stats() - API to send wakelock stats
 * @wiphy: wiphy pointer
 * @stats: stats data to be sent
 *
 * Return: 0 on success, error number otherwise.
 */
static int wlan_cfg80211_mc_cp_stats_send_wake_lock_stats(struct wiphy *wiphy,
						struct wake_lock_stats *stats)
{
	struct sk_buff *skb;
	uint32_t nl_buf_len;
	uint32_t icmpv6_cnt;
	uint32_t ipv6_rx_multicast_addr_cnt;
	uint32_t total_rx_data_wake, rx_multicast_cnt;

	nl_buf_len = NLMSG_HDRLEN;
	nl_buf_len += QCA_WLAN_VENDOR_GET_WAKE_STATS_MAX *
				(NLMSG_HDRLEN + sizeof(uint32_t));

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, nl_buf_len);

	if (!skb) {
		cfg80211_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
		return -ENOMEM;
	}

	cfg80211_debug("wow_ucast_wake_up_count %d",
		       stats->ucast_wake_up_count);
	cfg80211_debug("wow_bcast_wake_up_count %d",
		       stats->bcast_wake_up_count);
	cfg80211_debug("wow_ipv4_mcast_wake_up_count %d",
		       stats->ipv4_mcast_wake_up_count);
	cfg80211_debug("wow_ipv6_mcast_wake_up_count %d",
		       stats->ipv6_mcast_wake_up_count);
	cfg80211_debug("wow_ipv6_mcast_ra_stats %d",
		       stats->ipv6_mcast_ra_stats);
	cfg80211_debug("wow_ipv6_mcast_ns_stats %d",
		       stats->ipv6_mcast_ns_stats);
	cfg80211_debug("wow_ipv6_mcast_na_stats %d",
		       stats->ipv6_mcast_na_stats);
	cfg80211_debug("wow_icmpv4_count %d",
		       stats->icmpv4_count);
	cfg80211_debug("wow_icmpv6_count %d",
		       stats->icmpv6_count);
	cfg80211_debug("wow_rssi_breach_wake_up_count %d",
		       stats->rssi_breach_wake_up_count);
	cfg80211_debug("wow_low_rssi_wake_up_count %d",
		       stats->low_rssi_wake_up_count);
	cfg80211_debug("wow_gscan_wake_up_count %d",
		       stats->gscan_wake_up_count);
	cfg80211_debug("wow_pno_complete_wake_up_count %d",
		       stats->pno_complete_wake_up_count);
	cfg80211_debug("wow_pno_match_wake_up_count %d",
		       stats->pno_match_wake_up_count);

	ipv6_rx_multicast_addr_cnt = stats->ipv6_mcast_wake_up_count;
	icmpv6_cnt = stats->icmpv6_count;
	rx_multicast_cnt = stats->ipv4_mcast_wake_up_count +
						ipv6_rx_multicast_addr_cnt;
	total_rx_data_wake = stats->ucast_wake_up_count +
			stats->bcast_wake_up_count + rx_multicast_cnt;

	if (nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_TOTAL_CMD_EVENT_WAKE, 0) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_CMD_EVENT_WAKE_CNT_PTR, 0) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_CMD_EVENT_WAKE_CNT_SZ, 0) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_TOTAL_DRIVER_FW_LOCAL_WAKE,
			0) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_DRIVER_FW_LOCAL_WAKE_CNT_PTR,
			0) ||
	    nla_put_u32(skb, QCA_WLAN_VENDOR_ATTR_DRIVER_FW_LOCAL_WAKE_CNT_SZ,
			0) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_TOTAL_RX_DATA_WAKE,
			total_rx_data_wake) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_RX_UNICAST_CNT,
			stats->ucast_wake_up_count) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_RX_MULTICAST_CNT,
			rx_multicast_cnt) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_RX_BROADCAST_CNT,
			stats->bcast_wake_up_count) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_ICMP_PKT,
			stats->icmpv4_count) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_ICMP6_PKT,
			icmpv6_cnt) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_ICMP6_RA,
			stats->ipv6_mcast_ra_stats) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_ICMP6_NA,
			stats->ipv6_mcast_na_stats) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_ICMP6_NS,
			stats->ipv6_mcast_ns_stats) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_ICMP4_RX_MULTICAST_CNT,
			stats->ipv4_mcast_wake_up_count) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_ICMP6_RX_MULTICAST_CNT,
			ipv6_rx_multicast_addr_cnt) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_RSSI_BREACH_CNT,
			stats->rssi_breach_wake_up_count) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_LOW_RSSI_CNT,
			stats->low_rssi_wake_up_count) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_GSCAN_CNT,
			stats->gscan_wake_up_count) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_PNO_COMPLETE_CNT,
			stats->pno_complete_wake_up_count) ||
	    nla_put_u32(skb,
			QCA_WLAN_VENDOR_ATTR_PNO_MATCH_CNT,
			stats->pno_match_wake_up_count)) {
		cfg80211_err("nla put fail");
		goto nla_put_failure;
	}

	cfg80211_vendor_cmd_reply(skb);
	return 0;

nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}

int wlan_cfg80211_mc_cp_stats_get_wakelock_stats(struct wlan_objmgr_psoc *psoc,
						 struct wiphy *wiphy)
{
	/* refer __wlan_hdd_cfg80211_get_wakelock_stats */
	QDF_STATUS status;
	struct wake_lock_stats stats = {0};

	status = ucfg_mc_cp_stats_get_psoc_wake_lock_stats(psoc, &stats);
	if (QDF_IS_STATUS_ERROR(status))
		return qdf_status_to_os_return(status);

	return wlan_cfg80211_mc_cp_stats_send_wake_lock_stats(wiphy, &stats);
}
