/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

	event = cfg80211_vendor_event_alloc(hdd_ctx->wiphy, NULL,
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
	uint8_t pdev_id, dcs_enable;
	int status = 0;

	vdev = hdd_objmgr_get_vdev(adapter);
	if (!vdev)
		return -EINVAL;

	pdev_id = wlan_objmgr_pdev_get_pdev_id(wlan_vdev_get_pdev(vdev));

	dcs_enable = ucfg_get_dcs_enable(hdd_ctx->psoc, pdev_id);
	if (!(dcs_enable & CAP_DCS_WLANIM)) {
		hdd_err_rl("DCS_WLANIM is not enabled");
		status = -EINVAL;
		goto out;
	}

	if (qdf_atomic_read(&adapter->session.ap.acs_in_progress)) {
		hdd_err_rl("ACS is in progress");
		status = -EBUSY;
		goto out;
	}

	if (tb[PERIOD])
		cca_period = nla_get_u32(tb[PERIOD]);
	if (cca_period == 0)
		cca_period = DEFAULT_CCA_PERIOD;

	ucfg_dcs_reset_user_stats(hdd_ctx->psoc, pdev_id);
	ucfg_dcs_register_user_cb(hdd_ctx->psoc, pdev_id, adapter->vdev_id,
				  hdd_cca_notification_cb);
	/* dcs is already enabled and dcs event is reported every second
	 * set the user request counter to collect user stats
	 */
	ucfg_dcs_set_user_request(hdd_ctx->psoc, pdev_id, cca_period);

out:
	hdd_objmgr_put_vdev(vdev);
	return status;
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
 * hdd_congestion_notification_cb() - congestion notification callback function
 * @vdev_id: vdev id
 * @congestion: congestion percentage
 *
 * Return: None
 */
static void hdd_congestion_notification_cb(uint8_t vdev_id, uint8_t congestion)
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
	struct wlan_objmgr_vdev *vdev;
	uint8_t enable, threshold, interval = 1;
	struct request_info info = {0};
	bool pending = false;
	QDF_STATUS qdf_status;
	int status = 0;

	vdev = hdd_objmgr_get_vdev(adapter);
	if (!vdev)
		return -EINVAL;

	if (!tb[CONGESTION_REPORT_ENABLE]) {
		hdd_err_rl("Congestion report enable is not present");
		status = -EINVAL;
		goto out;
	}
	enable = nla_get_u8(tb[CONGESTION_REPORT_ENABLE]);

	switch (enable) {
	case REPORT_DISABLE:
		ucfg_mc_cp_stats_reset_pending_req(hdd_ctx->psoc,
						   TYPE_CONGESTION_STATS,
						   &info,
						   &pending);
		threshold = MAX_CONGESTION_THRESHOLD;
		interval = 0;
		break;
	case REPORT_ENABLE:
		if (!tb[CONGESTION_REPORT_THRESHOLD]) {
			hdd_err_rl("Congestion threshold is not present");
			status = -EINVAL;
			goto out;
		}
		threshold = nla_get_u8(tb[CONGESTION_REPORT_THRESHOLD]);
		if (threshold > MAX_CONGESTION_THRESHOLD) {
			hdd_err_rl("Invalid threshold %d", threshold);
			status = -EINVAL;
			goto out;
		}
		if (tb[CONGESTION_REPORT_INTERVAL])
			interval = nla_get_u8(tb[CONGESTION_REPORT_INTERVAL]);
		qdf_status = ucfg_mc_cp_stats_reset_congestion_counter(vdev);
		if (QDF_IS_STATUS_ERROR(qdf_status)) {
			hdd_err("Failed to set threshold");
			status = qdf_status_to_os_return(qdf_status);
			goto out;
		}
		break;
	default:
		hdd_err_rl("Invalid enable: %d", enable);
		status = -EINVAL;
		goto out;
	}

	qdf_status = ucfg_mc_cp_stats_set_congestion_threshold(vdev, threshold);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		hdd_err("Failed to set threshold");
		status = qdf_status_to_os_return(qdf_status);
		goto out;
	}

	status = sme_cli_set_command(adapter->vdev_id,
				     WMI_PDEV_PARAM_PDEV_STATS_UPDATE_PERIOD,
				     interval * 1000, PDEV_CMD);
	if (status) {
		hdd_err("Failed to set interval");
		goto out;
	}

	if (interval) {
		info.vdev_id = adapter->vdev_id;
		info.pdev_id =
			wlan_objmgr_pdev_get_pdev_id(wlan_vdev_get_pdev(vdev));
		info.u.congestion_notif_cb = hdd_congestion_notification_cb;
		status = ucfg_mc_cp_stats_send_stats_request(vdev,
							  TYPE_CONGESTION_STATS,
							  &info);
	}

out:
	hdd_objmgr_put_vdev(vdev);
	return status;
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
	int status;

	hdd_enter_dev(dev);

	if (driver_mode == QDF_GLOBAL_FTM_MODE ||
	    driver_mode == QDF_GLOBAL_MONITOR_MODE) {
		hdd_err_rl("Command not allowed in FTM / Monitor mode");
		return -EPERM;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status != 0)
		return status;

	status = wlan_cfg80211_nla_parse(tb, MEDIUM_ASSESS_MAX, data, data_len,
					 hdd_medium_assess_policy);
	if (status) {
		hdd_err_rl("Invalid ATTR");
		return status;
	}

	if (!tb[MEDIUM_ASSESS_TYPE]) {
		hdd_err_rl("Medium assess type is not present");
		return -EINVAL;
	}
	type = nla_get_u8(tb[MEDIUM_ASSESS_TYPE]);

	switch (type) {
	case QCA_WLAN_MEDIUM_ASSESS_CCA:
		status = hdd_medium_assess_cca(hdd_ctx, adapter, tb);
		break;
	case QCA_WLAN_MEDIUM_ASSESS_CONGESTION_REPORT:
		status = hdd_medium_assess_congestion_report(hdd_ctx, adapter,
							     tb);
		break;
	default:
		hdd_err_rl("Invalid medium assess type: %d", type);
		return -EINVAL;
	}

	hdd_exit();

	return status;
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
