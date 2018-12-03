/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cfg80211_mc_cp_stats.c
 *
 * This file provide definitions to cp stats supported cfg80211 cmd handlers
 */

#include <wlan_cfg80211.h>
#include <wlan_cp_stats_mc_defs.h>
#include <wlan_cp_stats_mc_ucfg_api.h>
#include <wlan_cfg80211_mc_cp_stats.h>
#include "wlan_osif_request_manager.h"
#include "wlan_objmgr_peer_obj.h"

/* max time in ms, caller may wait for stats request get serviced */
#define CP_STATS_WAIT_TIME_STAT 800

/**
 * wlan_cfg80211_mc_cp_stats_dealloc() - callback to free priv
 * allocations for stats
 * @priv: Pointer to priv data statucture
 *
 * Return: None
 */
static void wlan_cfg80211_mc_cp_stats_dealloc(void *priv)
{
	struct stats_event *stats = priv;

	if (!stats)
		return;

	qdf_mem_free(stats->pdev_stats);
	qdf_mem_free(stats->peer_stats);
	qdf_mem_free(stats->cca_stats);
	qdf_mem_free(stats->vdev_summary_stats);
	qdf_mem_free(stats->vdev_chain_rssi);
}

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

struct tx_power_priv {
	int dbm;
};

/**
 * get_tx_power_cb() - "Get tx power" callback function
 * @tx_power: tx_power
 * @cookie: a cookie for the request context
 *
 * Return: None
 */
static void get_tx_power_cb(int tx_power, void *cookie)
{
	struct osif_request *request;
	struct tx_power_priv *priv;

	request = osif_request_get(cookie);
	if (!request) {
		cfg80211_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);
	priv->dbm = tx_power;
	osif_request_complete(request);
	osif_request_put(request);
}

int wlan_cfg80211_mc_cp_stats_get_tx_power(struct wlan_objmgr_vdev *vdev,
					   int *dbm)
{
	int ret = 0;
	void *cookie;
	QDF_STATUS status;
	struct request_info info = {0};
	struct wlan_objmgr_peer *peer;
	struct tx_power_priv *priv = NULL;
	struct osif_request *request = NULL;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = CP_STATS_WAIT_TIME_STAT,
	};

	request = osif_request_alloc(&params);
	if (!request) {
		cfg80211_err("Request allocation failure, return cached value");
		goto fetch_tx_power;
	}

	cookie = osif_request_cookie(request);
	info.cookie = cookie;
	info.u.get_tx_power_cb = get_tx_power_cb;
	info.vdev_id = wlan_vdev_get_id(vdev);
	info.pdev_id = wlan_objmgr_pdev_get_pdev_id(wlan_vdev_get_pdev(vdev));
	peer = wlan_vdev_get_bsspeer(vdev);
	if (!peer) {
		ret = -EINVAL;
		goto peer_is_null;
	}
	qdf_mem_copy(info.peer_mac_addr, peer->macaddr, WLAN_MACADDR_LEN);

	status = ucfg_mc_cp_stats_send_stats_request(vdev,
						     TYPE_CONNECTION_TX_POWER,
						     &info);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("wlan_mc_cp_stats_request_tx_power status: %d",
			     status);
		ret = qdf_status_to_os_return(status);
	} else {
		ret = osif_request_wait_for_response(request);
		if (ret)
			cfg80211_err("wait failed or timed out ret: %d", ret);
		else
			priv = osif_request_priv(request);
	}

fetch_tx_power:
	if (priv) {
		*dbm = priv->dbm;
	} else {
		status = ucfg_mc_cp_stats_get_tx_power(vdev, dbm);
		if (QDF_IS_STATUS_ERROR(status)) {
			cfg80211_err("ucfg_mc_cp_stats_get_tx_power status: %d",
				     status);
			ret = qdf_status_to_os_return(status);
		}
	}

peer_is_null:
	/*
	 * either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.
	 * regardless we are done with the request.
	 */
	if (request)
		osif_request_put(request);

	return ret;
}

/**
 * get_peer_rssi_cb() - get_peer_rssi_cb callback function
 * @ev: peer stats buffer
 * @cookie: a cookie for the request context
 *
 * Return: None
 */
static void get_peer_rssi_cb(struct stats_event *ev, void *cookie)
{
	struct stats_event *priv;
	struct osif_request *request;
	uint32_t rssi_size;

	request = osif_request_get(cookie);
	if (!request) {
		cfg80211_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);
	rssi_size = sizeof(*ev->peer_stats) * ev->num_peer_stats;
	if (rssi_size == 0) {
		cfg80211_err("Invalid rssi stats");
		goto get_peer_rssi_cb_fail;
	}

	priv->peer_stats = qdf_mem_malloc(rssi_size);
	if (!priv->peer_stats) {
		cfg80211_err("allocation failed");
		goto get_peer_rssi_cb_fail;
	}

	priv->num_peer_stats = ev->num_peer_stats;
	qdf_mem_copy(priv->peer_stats, ev->peer_stats, rssi_size);

get_peer_rssi_cb_fail:
	osif_request_complete(request);
	osif_request_put(request);
}

struct stats_event *
wlan_cfg80211_mc_cp_stats_get_peer_rssi(struct wlan_objmgr_vdev *vdev,
					uint8_t *mac_addr,
					int *errno)
{
	void *cookie;
	QDF_STATUS status;
	struct stats_event *priv, *out;
	struct request_info info = {0};
	struct osif_request *request = NULL;
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = CP_STATS_WAIT_TIME_STAT,
		.dealloc = wlan_cfg80211_mc_cp_stats_dealloc,
	};

	out = qdf_mem_malloc(sizeof(*out));
	if (!out) {
		cfg80211_err("allocation failed");
		*errno = -ENOMEM;
		return NULL;
	}

	request = osif_request_alloc(&params);
	if (!request) {
		cfg80211_err("Request allocation failure, return cached value");
		*errno = -ENOMEM;
		qdf_mem_free(out);
		return NULL;
	}

	cookie = osif_request_cookie(request);
	priv = osif_request_priv(request);
	info.cookie = cookie;
	info.u.get_peer_rssi_cb = get_peer_rssi_cb;
	info.vdev_id = wlan_vdev_get_id(vdev);
	info.pdev_id = wlan_objmgr_pdev_get_pdev_id(wlan_vdev_get_pdev(vdev));
	qdf_mem_copy(info.peer_mac_addr, mac_addr, WLAN_MACADDR_LEN);
	status = ucfg_mc_cp_stats_send_stats_request(vdev, TYPE_PEER_STATS,
						     &info);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("stats req failed: %d", status);
		*errno = qdf_status_to_os_return(status);
		goto get_peer_rssi_fail;
	}

	*errno = osif_request_wait_for_response(request);
	if (*errno) {
		cfg80211_err("wait failed or timed out ret: %d", *errno);
		goto get_peer_rssi_fail;
	}

	if (!priv->peer_stats || priv->num_peer_stats == 0) {
		cfg80211_err("Invalid peer stats, count %d, data %pK",
			     priv->num_peer_stats, priv->peer_stats);
		*errno = -EINVAL;
		goto get_peer_rssi_fail;
	}
	out->num_peer_stats = priv->num_peer_stats;
	out->peer_stats = priv->peer_stats;
	priv->peer_stats = NULL;
	osif_request_put(request);

	return out;

get_peer_rssi_fail:
	osif_request_put(request);
	wlan_cfg80211_mc_cp_stats_free_stats_event(out);

	return NULL;
}

/**
 * get_station_stats_cb() - get_station_stats_cb callback function
 * @ev: station stats buffer
 * @cookie: a cookie for the request context
 *
 * Return: None
 */
static void get_station_stats_cb(struct stats_event *ev, void *cookie)
{
	struct stats_event *priv;
	struct osif_request *request;
	uint32_t summary_size, rssi_size;

	request = osif_request_get(cookie);
	if (!request) {
		cfg80211_err("Obsolete request");
		return;
	}

	priv = osif_request_priv(request);
	summary_size = sizeof(*ev->vdev_summary_stats) * ev->num_summary_stats;
	rssi_size = sizeof(*ev->vdev_chain_rssi) * ev->num_chain_rssi_stats;
	if (summary_size == 0 || rssi_size == 0) {
		cfg80211_err("Invalid stats, summary size %d rssi size %d",
			     summary_size, rssi_size);
		goto station_stats_cb_fail;
	}

	priv->vdev_summary_stats = qdf_mem_malloc(summary_size);
	if (!priv->vdev_summary_stats) {
		cfg80211_err("memory allocation failed");
		goto station_stats_cb_fail;
	}

	priv->vdev_chain_rssi = qdf_mem_malloc(rssi_size);
	if (!priv->vdev_chain_rssi) {
		cfg80211_err("memory allocation failed");
		goto station_stats_cb_fail;
	}

	priv->num_summary_stats = ev->num_summary_stats;
	priv->num_chain_rssi_stats = ev->num_chain_rssi_stats;
	priv->tx_rate = ev->tx_rate;
	priv->rx_rate = ev->rx_rate;
	priv->tx_rate_flags = ev->tx_rate_flags;
	qdf_mem_copy(priv->vdev_chain_rssi, ev->vdev_chain_rssi, rssi_size);
	qdf_mem_copy(priv->vdev_summary_stats, ev->vdev_summary_stats,
		     summary_size);

station_stats_cb_fail:
	osif_request_complete(request);
	osif_request_put(request);
}

struct stats_event *
wlan_cfg80211_mc_cp_stats_get_station_stats(struct wlan_objmgr_vdev *vdev,
					    int *errno)
{
	void *cookie;
	QDF_STATUS status;
	struct stats_event *priv, *out;
	struct wlan_objmgr_peer *peer;
	struct osif_request *request;
	struct request_info info = {0};
	static const struct osif_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = 2 * CP_STATS_WAIT_TIME_STAT,
		.dealloc = wlan_cfg80211_mc_cp_stats_dealloc,
	};

	out = qdf_mem_malloc(sizeof(*out));
	if (!out) {
		cfg80211_err("allocation failed");
		*errno = -ENOMEM;
		return NULL;
	}

	request = osif_request_alloc(&params);
	if (!request) {
		cfg80211_err("Request allocation failure, return cached value");
		qdf_mem_free(out);
		*errno = -ENOMEM;
		return NULL;
	}

	cookie = osif_request_cookie(request);
	priv = osif_request_priv(request);
	info.cookie = cookie;
	info.u.get_station_stats_cb = get_station_stats_cb;
	info.vdev_id = wlan_vdev_get_id(vdev);
	info.pdev_id = wlan_objmgr_pdev_get_pdev_id(wlan_vdev_get_pdev(vdev));
	peer = wlan_vdev_get_bsspeer(vdev);
	if (!peer) {
		cfg80211_err("peer is null");
		*errno = -EINVAL;
		goto get_station_stats_fail;
	}
	qdf_mem_copy(info.peer_mac_addr, peer->macaddr, WLAN_MACADDR_LEN);

	status = ucfg_mc_cp_stats_send_stats_request(vdev, TYPE_STATION_STATS,
						     &info);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("Failed to send stats request status: %d", status);
		*errno = qdf_status_to_os_return(status);
		goto get_station_stats_fail;
	}

	*errno = osif_request_wait_for_response(request);
	if (*errno) {
		cfg80211_err("wait failed or timed out ret: %d", *errno);
		goto get_station_stats_fail;
	}

	if (!priv->vdev_summary_stats || !priv->vdev_chain_rssi ||
	    priv->num_summary_stats == 0 || priv->num_chain_rssi_stats == 0) {
		cfg80211_err("Invalid stats, summary %d:%pK, rssi %d:%pK",
			     priv->num_summary_stats, priv->vdev_summary_stats,
			     priv->num_chain_rssi_stats, priv->vdev_chain_rssi);
		*errno = -EINVAL;
		goto get_station_stats_fail;
	}

	out->tx_rate = priv->tx_rate;
	out->rx_rate = priv->rx_rate;
	out->tx_rate_flags = priv->tx_rate_flags;
	out->num_summary_stats = priv->num_summary_stats;
	out->num_chain_rssi_stats = priv->num_chain_rssi_stats;
	out->vdev_summary_stats = priv->vdev_summary_stats;
	priv->vdev_summary_stats = NULL;
	out->vdev_chain_rssi = priv->vdev_chain_rssi;
	priv->vdev_chain_rssi = NULL;
	osif_request_put(request);

	return out;

get_station_stats_fail:
	osif_request_put(request);
	wlan_cfg80211_mc_cp_stats_free_stats_event(out);

	return NULL;
}

void wlan_cfg80211_mc_cp_stats_free_stats_event(struct stats_event *stats)
{
	if (!stats)
		return;

	qdf_mem_free(stats->pdev_stats);
	qdf_mem_free(stats->peer_stats);
	qdf_mem_free(stats->cca_stats);
	qdf_mem_free(stats->vdev_summary_stats);
	qdf_mem_free(stats->vdev_chain_rssi);
	qdf_mem_free(stats);
}
