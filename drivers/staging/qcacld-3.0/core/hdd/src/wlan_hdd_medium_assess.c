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
 * DOC : wlan_hdd_medium_assess.c
 *
 * WLAN Host Device Driver medium assess related implementation
 *
 */

#include "wlan_hdd_medium_assess.h"
#include <osif_sync.h>
#include <wlan_hdd_main.h>
#include <wlan_hdd_object_manager.h>
#include <wlan_dcs_ucfg_api.h>
#include <wlan_cp_stats_mc_ucfg_api.h>
#include <sme_api.h>
#include <wma_api.h>
#include "wlan_cmn.h"

/* define short names for get station info attributes */
#define MEDIUM_ASSESS_TYPE \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_TYPE
#define PERIOD \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_PERIOD
#define TOTAL_CYCLE_COUNT \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_TOTAL_CYCLE_COUNT
#define IDLE_COUNT \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_IDLE_COUNT
#define IBSS_RX_COUNT \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_IBSS_RX_COUNT
#define OBSS_RX_COUNT \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_OBSS_RX_COUNT
#define MAX_IBSS_RSSI \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_MAX_IBSS_RSSI
#define MIN_IBSS_RSSI \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_MIN_IBSS_RSSI
#define CONGESTION_REPORT_ENABLE \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_REPORT_ENABLE
#define CONGESTION_REPORT_THRESHOLD \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_REPORT_THRESHOLD
#define CONGESTION_REPORT_INTERVAL \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_REPORT_INTERVAL
#define CONGESTION_PERCENTAGE \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_PERCENTAGE
#define MEDIUM_ASSESS_MAX \
	QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_MAX

#define DEFAULT_CCA_PERIOD 10	/* seconds */
#define CCA_GET_RSSI_INTERVAL 1000	/* 1 second */

#define REPORT_DISABLE 0
#define REPORT_ENABLE 1

#define MAX_CONGESTION_THRESHOLD 100
#define CYCLE_THRESHOLD 6400000 /* 40000us * 160M */

#define MEDIUM_ASSESS_TIMER_INTERVAL 1000 /* 1000ms */
static qdf_mc_timer_t hdd_medium_assess_timer;
static uint8_t timer_enable, ssr_flag;
struct hdd_medium_assess_info medium_assess_info[WLAN_UMAC_MAX_RP_PID];
unsigned long stime;

const struct nla_policy
hdd_medium_assess_policy[MEDIUM_ASSESS_MAX + 1] = {
	[MEDIUM_ASSESS_TYPE] = {.type = NLA_U8},
	[PERIOD] = {.type = NLA_U32},
	[CONGESTION_REPORT_ENABLE] = {.type = NLA_U8},
	[CONGESTION_REPORT_THRESHOLD] = {.type = NLA_U8},
	[CONGESTION_REPORT_INTERVAL] = {.type = NLA_U8},
};

/*
 * get_cca_report_len() - Calculate length for CCA report
 * to allocate skb buffer
 *
 * Return: skb buffer length
 */
static int get_cca_report_len(void)
{
	uint32_t data_len = NLMSG_HDRLEN;

	/* QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_TYPE */
	data_len += nla_total_size(sizeof(uint8_t));

	/* QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_TOTAL_CYCLE_COUNT */
	data_len += nla_total_size(sizeof(uint32_t));

	/* QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_IDLE_COUNT */
	data_len += nla_total_size(sizeof(uint32_t));

	/* QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_IBSS_RX_COUNT */
	data_len += nla_total_size(sizeof(uint32_t));

	/* QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_OBSS_RX_COUNT */
	data_len += nla_total_size(sizeof(uint32_t));

	/* QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_MAX_IBSS_RSSI */
	data_len += nla_total_size(sizeof(uint32_t));

	/* QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_MIN_IBSS_RSSI */
	data_len += nla_total_size(sizeof(uint32_t));

	return data_len;
}

/**
 * hdd_cca_notification_cb() - cca notification callback function
 * @vdev_id: vdev id
 * @stats: dcs im stats
 * @status: status of cca statistics
 *
 * Return: None
 */
static void hdd_cca_notification_cb(uint8_t vdev_id,
				    struct wlan_host_dcs_im_user_stats *stats,
				    int status)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;
	struct sk_buff *event;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("Failed to find adapter of vdev %d", vdev_id);
		return;
	}

	event = cfg80211_vendor_event_alloc(
				hdd_ctx->wiphy, &adapter->wdev,
				get_cca_report_len(),
				QCA_NL80211_VENDOR_SUBCMD_MEDIUM_ASSESS_INDEX,
				GFP_KERNEL);
	if (!event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u8(event, MEDIUM_ASSESS_TYPE,
		       QCA_WLAN_MEDIUM_ASSESS_CCA) ||
	    nla_put_u32(event, TOTAL_CYCLE_COUNT, stats->cycle_count) ||
	    nla_put_u32(event, IDLE_COUNT,
			stats->cycle_count - stats->rxclr_count) ||
	    nla_put_u32(event, IBSS_RX_COUNT, stats->my_bss_rx_cycle_count) ||
	    nla_put_u32(event, OBSS_RX_COUNT,
			stats->rx_frame_count - stats->my_bss_rx_cycle_count) ||
	    nla_put_u32(event, MAX_IBSS_RSSI, stats->max_rssi) ||
	    nla_put_u32(event, MIN_IBSS_RSSI, stats->min_rssi)) {
		hdd_err("nla put failed");
		kfree_skb(event);
		return;
	}

	cfg80211_vendor_event(event, GFP_KERNEL);
}

/**
 * hdd_medium_assess_cca() - clear channel assessment
 * @hdd_ctx: pointer to HDD context
 * @adapter: pointer to adapter
 * @tb: list of attributes
 *
 * Return: success(0) or reason code for failure
 */
static int hdd_medium_assess_cca(struct hdd_context *hdd_ctx,
				 struct hdd_adapter *adapter,
				 struct nlattr **tb)
{
	struct wlan_objmgr_vdev *vdev;
	uint32_t cca_period = DEFAULT_CCA_PERIOD;
	uint8_t mac_id, dcs_enable;
	QDF_STATUS status;
	int errno = 0;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_DCS_ID);
	if (!vdev)
		return -EINVAL;

	status = policy_mgr_get_mac_id_by_session_id(hdd_ctx->psoc,
						     adapter->vdev_id,
						     &mac_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err_rl("Failed to get mac_id");
		errno = -EINVAL;
		goto out;
	}

	dcs_enable = ucfg_get_dcs_enable(hdd_ctx->psoc, mac_id);
	if (!(dcs_enable & WLAN_HOST_DCS_WLANIM)) {
		hdd_err_rl("DCS_WLANIM is not enabled");
		errno = -EINVAL;
		goto out;
	}

	if (qdf_atomic_read(&adapter->session.ap.acs_in_progress)) {
		hdd_err_rl("ACS is in progress");
		errno = -EBUSY;
		goto out;
	}

	if (tb[PERIOD])
		cca_period = nla_get_u32(tb[PERIOD]);
	if (cca_period == 0)
		cca_period = DEFAULT_CCA_PERIOD;

	ucfg_dcs_reset_user_stats(hdd_ctx->psoc, mac_id);
	ucfg_dcs_register_user_cb(hdd_ctx->psoc, mac_id, adapter->vdev_id,
				  hdd_cca_notification_cb);
	/* dcs is already enabled and dcs event is reported every second
	 * set the user request counter to collect user stats
	 */
	ucfg_dcs_set_user_request(hdd_ctx->psoc, mac_id, cca_period);

out:
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_DCS_ID);
	return errno;
}

/*
 * get_congestion_report_len() - Calculate length for congestion report
 * to allocate skb buffer
 *
 * Return: skb buffer length
 */
static int get_congestion_report_len(void)
{
	uint32_t data_len = NLMSG_HDRLEN;

	/* QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_TYPE */
	data_len += nla_total_size(sizeof(uint8_t));

	/* QCA_WLAN_VENDOR_ATTR_MEDIUM_ASSESS_CONGESTION_PERCENTAGE */
	data_len += nla_total_size(sizeof(uint8_t));

	return data_len;
}

/**
 * hdd_congestion_reset_data() - reset/invalid the previous data
 * @vdev_id: vdev id
 *
 * Return: None
 */
static void hdd_congestion_reset_data(uint8_t pdev_id)
{
	struct hdd_medium_assess_info *mdata;
	uint8_t i;

	mdata = &medium_assess_info[pdev_id];
	for (i = 0; i < MEDIUM_ASSESS_NUM; i++)
		mdata->data[i].part1_valid = 0;
}

/**
 * hdd_congestion_notification_cb() - congestion notification callback function
 * @vdev_id: vdev id
 * @congestion: congestion percentage
 *
 * Return: None
 */
static void hdd_congestion_notification_cb(uint8_t vdev_id,
					   struct medium_assess_data *data)
{
	struct hdd_medium_assess_info *mdata;
	uint8_t i;
	int32_t index;

	/* the cb should not be delay more than 40 ms or drop it */
	if (qdf_system_time_after(jiffies, stime)) {
		hdd_debug("medium assess interference data drop");
		return;
	}

	for (i = 0; i < WLAN_UMAC_MAX_RP_PID; i++) {
		mdata = &medium_assess_info[i];

		index = mdata->index - 1;
		if (index < 0)
			index = MEDIUM_ASSESS_NUM - 1;

		if (data[i].part1_valid && mdata->data[index].part1_valid) {
			if (CYCLE_THRESHOLD > (data[i].cycle_count -
			    mdata->data[index].cycle_count))
				continue;
		}

		if (data[i].part1_valid) {
			mdata->data[mdata->index].part1_valid = 1;
			mdata->data[mdata->index].cycle_count =
						data[i].cycle_count;
			mdata->data[mdata->index].rx_clear_count =
						data[i].rx_clear_count;
			mdata->data[mdata->index].tx_frame_count =
						data[i].tx_frame_count;
		}

		if (mdata->data[mdata->index].part1_valid) {
			mdata->index++;
			if (mdata->index >= MEDIUM_ASSESS_NUM)
				mdata->index = 0;
			mdata->data[mdata->index].part1_valid = 0;
		}
	}
}

/**
 * hdd_congestion_notification_report() - congestion report function
 * @vdev_id: vdev id
 * @congestion: congestion percentage
 *
 * Return: None
 */
static void hdd_congestion_notification_report(uint8_t vdev_id,
					       uint8_t congestion)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;
	struct sk_buff *event;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("Failed to find adapter of vdev %d", vdev_id);
		return;
	}

	event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy, &(adapter->wdev),
				  get_congestion_report_len(),
				  QCA_NL80211_VENDOR_SUBCMD_MEDIUM_ASSESS_INDEX,
				  GFP_KERNEL);
	if (!event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u8(event, MEDIUM_ASSESS_TYPE,
		       QCA_WLAN_MEDIUM_ASSESS_CONGESTION_REPORT) ||
	    nla_put_u8(event, CONGESTION_PERCENTAGE, congestion)) {
		hdd_err("nla put failed");
		kfree_skb(event);
		return;
	}

	cfg80211_vendor_event(event, GFP_KERNEL);
}

void hdd_medium_assess_ssr_enable_flag(void)
{
	uint8_t i;

	ssr_flag = 1;
	for (i = 0; i < WLAN_UMAC_MAX_RP_PID; i++)
		hdd_congestion_reset_data(i);
}

void hdd_medium_assess_stop_timer(uint8_t pdev_id, struct hdd_context *hdd_ctx)
{
	struct request_info info = {0};
	bool pending = false;
	uint8_t i, interval = 0;

	if (ssr_flag)
		return;

	medium_assess_info[pdev_id].config.threshold = MAX_CONGESTION_THRESHOLD;
	medium_assess_info[pdev_id].config.interval = 0;
	medium_assess_info[pdev_id].index = 0;
	medium_assess_info[pdev_id].count = 0;
	hdd_congestion_reset_data(pdev_id);

	for (i = 0; i < WLAN_UMAC_MAX_RP_PID; i++)
		interval += medium_assess_info[i].config.interval;

	if (!interval) {
		ucfg_mc_cp_stats_reset_pending_req(hdd_ctx->psoc,
						   TYPE_CONGESTION_STATS,
						   &info, &pending);
		qdf_mc_timer_stop(&hdd_medium_assess_timer);
		hdd_debug("medium assess atimer stop");
	}
}

/**
 * hdd_medium_assess_congestion_report() - congestion report
 * @hdd_ctx: pointer to HDD context
 * @adapter: pointer to adapter
 * @tb: list of attributes
 *
 * Return: success(0) or reason code for failure
 */
static int hdd_medium_assess_congestion_report(struct hdd_context *hdd_ctx,
					       struct hdd_adapter *adapter,
					       struct nlattr **tb)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;
	uint8_t enable, threshold, interval = 0;
	uint8_t pdev_id, vdev_id;
	int errno = 0;

	if (!tb[CONGESTION_REPORT_ENABLE]) {
		hdd_err_rl("Congestion report enable is not present");
		return -EINVAL;
	}

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_CP_STATS_ID);
	if (!vdev)
		return -EINVAL;

	vdev_id = adapter->vdev_id;
	status = policy_mgr_get_mac_id_by_session_id(hdd_ctx->psoc, vdev_id,
						     &pdev_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("get mac id failed");
		goto out;
	}

	medium_assess_info[pdev_id].vdev_id = vdev_id;
	medium_assess_info[pdev_id].pdev_id = pdev_id;

	enable = nla_get_u8(tb[CONGESTION_REPORT_ENABLE]);
	switch (enable) {
	case REPORT_DISABLE:
		hdd_debug("medium assess disable: pdev_id %d, vdev_id: %d",
			  pdev_id, vdev_id);
		hdd_medium_assess_stop_timer(pdev_id, hdd_ctx);
		break;
	case REPORT_ENABLE:
		if (!tb[CONGESTION_REPORT_THRESHOLD]) {
			hdd_err_rl("Congestion threshold is not present");
			errno = -EINVAL;
			goto out;
		}
		threshold = nla_get_u8(tb[CONGESTION_REPORT_THRESHOLD]);
		if (threshold > MAX_CONGESTION_THRESHOLD) {
			hdd_err_rl("Invalid threshold %d", threshold);
			errno = -EINVAL;
			goto out;
		}
		if (tb[CONGESTION_REPORT_INTERVAL]) {
			interval = nla_get_u8(tb[CONGESTION_REPORT_INTERVAL]);
			if (interval >= MEDIUM_ASSESS_NUM)
				interval = MEDIUM_ASSESS_NUM - 1;
		} else {
			interval = 1;
		}

		medium_assess_info[pdev_id].config.threshold = threshold;
		medium_assess_info[pdev_id].config.interval = interval;
		medium_assess_info[pdev_id].index = 0;
		medium_assess_info[pdev_id].count = 0;
		hdd_congestion_reset_data(pdev_id);
		hdd_debug("medium assess enable: pdev_id %d, vdev_id: %d",
			  pdev_id, vdev_id);

		if (qdf_mc_timer_get_current_state(&hdd_medium_assess_timer) ==
						     QDF_TIMER_STATE_STOPPED) {
			hdd_debug("medium assess atimer start");
			qdf_mc_timer_start(&hdd_medium_assess_timer,
					   MEDIUM_ASSESS_TIMER_INTERVAL);
		}
		break;
	default:
		hdd_err_rl("Invalid enable: %d", enable);
		errno = -EINVAL;
		break;
	}

out:
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_CP_STATS_ID);
	return errno;
}

/**
 * __hdd_cfg80211_medium_assess() - medium assess
 * @wiphy: pointer to wireless phy
 * @wdev: wireless device
 * @data: data
 * @data_len: data length
 *
 * Return: success(0) or reason code for failure
 */
static int __hdd_cfg80211_medium_assess(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	enum QDF_GLOBAL_MODE driver_mode = hdd_get_conparam();
	struct nlattr *tb[MEDIUM_ASSESS_MAX + 1];
	uint8_t type;
	int errno;

	hdd_enter_dev(dev);

	if (driver_mode == QDF_GLOBAL_FTM_MODE ||
	    driver_mode == QDF_GLOBAL_MONITOR_MODE) {
		hdd_err_rl("Command not allowed in FTM / Monitor mode");
		return -EPERM;
	}

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	errno = wlan_cfg80211_nla_parse(tb, MEDIUM_ASSESS_MAX, data, data_len,
					hdd_medium_assess_policy);
	if (errno) {
		hdd_err_rl("Invalid ATTR");
		return errno;
	}

	if (!tb[MEDIUM_ASSESS_TYPE]) {
		hdd_err_rl("Medium assess type is not present");
		return -EINVAL;
	}
	type = nla_get_u8(tb[MEDIUM_ASSESS_TYPE]);

	switch (type) {
	case QCA_WLAN_MEDIUM_ASSESS_CCA:
		errno = hdd_medium_assess_cca(hdd_ctx, adapter, tb);
		break;
	case QCA_WLAN_MEDIUM_ASSESS_CONGESTION_REPORT:
		errno = hdd_medium_assess_congestion_report(hdd_ctx, adapter,
							     tb);
		break;
	default:
		hdd_err_rl("Invalid medium assess type: %d", type);
		return -EINVAL;
	}

	hdd_exit();

	return errno;
}

int hdd_cfg80211_medium_assess(struct wiphy *wiphy,
			       struct wireless_dev *wdev,
			       const void *data,
			       int data_len)
{
	struct osif_vdev_sync *vdev_sync;
	int errno;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __hdd_cfg80211_medium_assess(wiphy, wdev, data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}

/**
 * hdd_congestion_notification_calculation() - medium assess congestion
 * calculation.
 * @info: structure hdd_medium_assess_info
 *
 * Return: None
 */
static void
hdd_congestion_notification_calculation(struct hdd_medium_assess_info *info)
{
	struct medium_assess_data *h_data, *t_data;
	int32_t h_index, t_index;
	uint32_t rx_clear_count_delta, tx_frame_count_delta;
	uint32_t cycle_count_delta;
	uint32_t congestion = 0;
	uint64_t diff;

	h_index = info->index - 1;
	if (h_index < 0)
		h_index = MEDIUM_ASSESS_NUM - 1;

	if (h_index >= info->config.interval)
		t_index = h_index - info->config.interval;
	else
		t_index = MEDIUM_ASSESS_NUM - info->config.interval - h_index;

	if (h_index < 0 || h_index >= MEDIUM_ASSESS_NUM ||
	    t_index < 0 || t_index >= MEDIUM_ASSESS_NUM) {
		hdd_err("medium assess index is not valid.");
		return;
	}

	h_data = &info->data[h_index];
	t_data = &info->data[t_index];

	if (!(h_data->part1_valid || t_data->part1_valid)) {
		hdd_err("medium assess data is not valid.");
		return;
	}

	if (h_data->rx_clear_count >= t_data->rx_clear_count) {
		rx_clear_count_delta = h_data->rx_clear_count -
						t_data->rx_clear_count;
	} else {
		rx_clear_count_delta = U32_MAX - t_data->rx_clear_count;
		rx_clear_count_delta += h_data->rx_clear_count;
	}

	if (h_data->tx_frame_count >= t_data->tx_frame_count) {
		tx_frame_count_delta = h_data->tx_frame_count -
						t_data->tx_frame_count;
	} else {
		tx_frame_count_delta = U32_MAX - t_data->tx_frame_count;
		tx_frame_count_delta += h_data->tx_frame_count;
	}

	if (h_data->cycle_count >= t_data->cycle_count) {
		cycle_count_delta = h_data->cycle_count - t_data->cycle_count;
	} else {
		cycle_count_delta = U32_MAX - t_data->cycle_count;
		cycle_count_delta += h_data->cycle_count;
	}

	diff = (rx_clear_count_delta - tx_frame_count_delta) * 100;
	if (cycle_count_delta)
		congestion = qdf_do_div(diff, cycle_count_delta);

	if (congestion > 100)
		congestion = 100;

	hdd_debug("pdev: %d, rx_clear %u, tx_frame %u cycle %u congestion: %u",
		  info->pdev_id, rx_clear_count_delta, tx_frame_count_delta,
		  cycle_count_delta, congestion);
	if (congestion >= info->config.threshold)
		hdd_congestion_notification_report(info->vdev_id, congestion);
}

/**
 * hdd_congestion_notification_report_multi() - medium assess report
 * multi interface.
 * @pdev_id: pdev id
 *
 * Return: None
 */
static void hdd_congestion_notification_report_multi(uint8_t pdev_id)
{
	struct hdd_medium_assess_info *info;

	info = &medium_assess_info[pdev_id];
	info->count++;
	if (info->count % info->config.interval == 0)
		hdd_congestion_notification_calculation(info);
}

/**
 * hdd_medium_assess_expire_handler() - timer callback
 * @arg: argument
 *
 * Return: None
 */
static void hdd_medium_assess_expire_handler(void *arg)
{
	struct wlan_objmgr_vdev *vdev;
	struct request_info info = {0};
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	struct hdd_adapter *adapter;
	uint8_t vdev_id = INVALID_VDEV_ID, pdev_id;
	uint8_t index, i;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	for (i = 0; i < WLAN_UMAC_MAX_RP_PID; i++)
		if (medium_assess_info[i].config.interval != 0) {
			vdev_id = medium_assess_info[i].vdev_id;
			pdev_id = medium_assess_info[i].pdev_id;
			hdd_congestion_notification_report_multi(pdev_id);

			/* ensure events are reveived at the 'same' time */
			index = medium_assess_info[i].index;
			medium_assess_info[i].data[index].part1_valid = 0;
		}

	if (vdev_id == INVALID_VDEV_ID)
		return;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter) {
		hdd_err("Failed to find adapter of vdev %d", vdev_id);
		return;
	}

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_CP_STATS_ID);
	if (!vdev)
		return;

	info.vdev_id = vdev_id;
	info.pdev_id = 0;
	info.u.congestion_notif_cb = hdd_congestion_notification_cb;
	stime = jiffies + msecs_to_jiffies(40);
	ucfg_mc_cp_stats_send_stats_request(vdev,
					    TYPE_CONGESTION_STATS,
					    &info);
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_CP_STATS_ID);
	qdf_mc_timer_start(&hdd_medium_assess_timer,
			   MEDIUM_ASSESS_TIMER_INTERVAL);
}

void hdd_medium_assess_init(void)
{
	QDF_STATUS status;

	if (!timer_enable) {
		hdd_debug("medium assess init timer");
		status = qdf_mc_timer_init(&hdd_medium_assess_timer,
					   QDF_TIMER_TYPE_SW,
					   hdd_medium_assess_expire_handler,
					   NULL);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_debug("medium assess init timer failed");
			return;
		}

		if (ssr_flag) {
			ssr_flag = 0;
			qdf_mc_timer_start(&hdd_medium_assess_timer,
					   MEDIUM_ASSESS_TIMER_INTERVAL);
		}
	}
	timer_enable += 1;
}

void hdd_medium_assess_deinit(void)
{
	timer_enable -= 1;
	if (!timer_enable) {
		hdd_debug("medium assess deinit timer");
		if (qdf_mc_timer_get_current_state(&hdd_medium_assess_timer) ==
						     QDF_TIMER_STATE_RUNNING)
			qdf_mc_timer_stop(&hdd_medium_assess_timer);

		qdf_mc_timer_destroy(&hdd_medium_assess_timer);
	}
}
