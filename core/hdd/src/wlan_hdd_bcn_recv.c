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
 * DOC:  wlan_hdd_bcn_recv.c
 * Feature for receiving beacons of connected AP and sending select
 * params to upper layer via vendor event
 */

#include <wlan_hdd_includes.h>
#include <net/cfg80211.h>
#include "wlan_osif_priv.h"
#include "qdf_trace.h"
#include "wlan_hdd_main.h"
#include "osif_sync.h"
#include "wlan_hdd_bcn_recv.h"

#define BOOTTIME QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_BOOTTIME_WHEN_RECEIVED

static const struct nla_policy
beacon_reporting_params[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_ACTIVE_REPORTING] = {.type =
								     NLA_FLAG},
	[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_PERIOD] = {.type = NLA_U8},
};

/**
 * get_beacon_report_data_len() - Calculate length for beacon
 * report to allocate skb buffer
 * @report: beacon report structure
 *
 * Return: skb buffer length
 */
static
int get_beacon_report_data_len(struct wlan_beacon_report *report)
{
	uint32_t data_len = NLMSG_HDRLEN;

	/* QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE */
	data_len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_SSID */
	data_len += nla_total_size(report->ssid.length);

	/* QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_BSSID */
	data_len += nla_total_size(ETH_ALEN);

	/* QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_FREQ */
	data_len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_BI */
	data_len += nla_total_size(sizeof(u16));

	/* QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_TSF */
	data_len += nla_total_size(sizeof(uint64_t));

	/* QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_BOOTTIME_WHEN_RECEIVED */
	data_len += nla_total_size(sizeof(uint64_t));

	return data_len;
}

/**
 * get_pause_ind_data_len() - Calculate skb buffer length
 * @report: Required beacon report
 *
 * Calculate length for pause indication to allocate skb buffer
 *
 * Return: skb buffer length
 */
static int get_pause_ind_data_len(void)
{
	uint32_t data_len = NLMSG_HDRLEN;

	/* QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE */
	data_len += nla_total_size(sizeof(u32));

	/* QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_PAUSE_REASON */
	data_len += nla_total_size(sizeof(u32));

	return data_len;
}

/**
 * hdd_beacon_recv_pause_indication()- Send vendor event to user space
 * to inform SCAN started indication
 * @hdd_handle: hdd handler
 * @vdev_id: vdev id
 * @type: scan event type
 *
 * Return: None
 */
static void hdd_beacon_recv_pause_indication(hdd_handle_t hdd_handle,
					     uint8_t vdev_id,
					     enum scan_event_type type)
{
	struct hdd_context *hdd_ctx = hdd_handle_to_context(hdd_handle);
	struct hdd_adapter *adapter;
	struct sk_buff *vendor_event;
	uint32_t data_len;
	int flags;
	uint32_t abort_reason;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (hdd_validate_adapter(adapter))
		return;

	data_len = get_pause_ind_data_len();
	flags = cds_get_gfp_flags();

	vendor_event =
		cfg80211_vendor_event_alloc(
			hdd_ctx->wiphy, NULL,
			data_len,
			QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING_INDEX,
			flags);
	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	switch (type) {
	case SCAN_EVENT_TYPE_STARTED:
		abort_reason =
		  QCA_WLAN_VENDOR_BEACON_REPORTING_PAUSE_REASON_SCAN_STARTED;
		break;
	default:
		abort_reason =
		     QCA_WLAN_VENDOR_BEACON_REPORTING_PAUSE_REASON_UNSPECIFIED;
	}
	/* Send vendor event to user space to inform ABORT */
	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE,
			QCA_WLAN_VENDOR_BEACON_REPORTING_OP_PAUSE) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_PAUSE_REASON,
			abort_reason)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		kfree_skb(vendor_event);
		return;
	}

	cfg80211_vendor_event(vendor_event, flags);
}

/**
 * hdd_send_bcn_recv_info() - Send beacon info to userspace for
 * connected AP
 * @hdd_handle: hdd_handle to get hdd_adapter
 * @beacon_report: Required beacon report
 *
 * Send beacon info to userspace for connected AP through a vendor event:
 * QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING.
 */
static void hdd_send_bcn_recv_info(hdd_handle_t hdd_handle,
				   struct wlan_beacon_report *beacon_report)
{
	struct sk_buff *vendor_event;
	struct hdd_context *hdd_ctx = hdd_handle_to_context(hdd_handle);
	uint32_t data_len;
	int flags = cds_get_gfp_flags();

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	data_len = get_beacon_report_data_len(beacon_report);

	vendor_event =
		cfg80211_vendor_event_alloc(
			hdd_ctx->wiphy, NULL,
			data_len,
			QCA_NL80211_VENDOR_SUBCMD_BEACON_REPORTING_INDEX,
			flags);
	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}

	if (nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE,
			QCA_WLAN_VENDOR_BEACON_REPORTING_OP_BEACON_INFO) ||
	    nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_SSID,
		    beacon_report->ssid.length, beacon_report->ssid.ssid) ||
	    nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_BSSID,
		    ETH_ALEN, beacon_report->bssid.bytes) ||
	    nla_put_u32(vendor_event,
			QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_FREQ,
			beacon_report->frequency) ||
	    nla_put_u16(vendor_event,
			QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_BI,
			beacon_report->beacon_interval) ||
	    wlan_cfg80211_nla_put_u64(vendor_event,
				      QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_TSF,
				      beacon_report->time_stamp) ||
	    wlan_cfg80211_nla_put_u64(vendor_event, BOOTTIME,
				      beacon_report->boot_time)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR put fail");
		kfree_skb(vendor_event);
		return;
	}

	cfg80211_vendor_event(vendor_event, flags);
}

/**
 * __wlan_hdd_cfg80211_bcn_rcv_start() - enable/disable beacon reporting
 * indication
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * This function is used to enable/disable asynchronous beacon
 * reporting feature using vendor commands.
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_bcn_rcv_start(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_MAX + 1];
	uint32_t bcn_report;
	int errno;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	bool active_report;

	hdd_enter_dev(dev);

	errno = hdd_validate_adapter(adapter);
	if (errno)
		return errno;

	if (adapter->device_mode != QDF_STA_MODE) {
		hdd_err("Command not allowed as device not in STA mode");
		return -EINVAL;
	}

	if (!hdd_conn_is_connected(WLAN_HDD_GET_STATION_CTX_PTR(adapter))) {
		hdd_err("STA not in connected state");
		return -EINVAL;
	}

	errno =
	   wlan_cfg80211_nla_parse(tb,
				   QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_MAX,
				   data, data_len, beacon_reporting_params);
	if (errno) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	/* Parse and fetch OP Type */
	if (!tb[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE]) {
		hdd_err("attr beacon report OP type failed");
		return -EINVAL;
	}
	active_report =
		!!tb[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_ACTIVE_REPORTING];
	hdd_debug("attr active_report %d", active_report);

	bcn_report =
		nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE]);
	hdd_debug("Bcn Report: OP type:%d", bcn_report);

	if (bcn_report == QCA_WLAN_VENDOR_BEACON_REPORTING_OP_START) {
		if (active_report) {
			qdf_status = sme_register_bcn_report_pe_cb(
							hdd_ctx->mac_handle,
							hdd_send_bcn_recv_info);
			if (QDF_IS_STATUS_ERROR(qdf_status)) {
				hdd_err("bcn recv info cb reg failed = %d",
					qdf_status);
				errno = qdf_status_to_os_return(qdf_status);
				return errno;
			}
		}
		qdf_status =
			sme_register_bcn_recv_pause_ind_cb(hdd_ctx->mac_handle,
					hdd_beacon_recv_pause_indication);
		if (QDF_IS_STATUS_ERROR(qdf_status)) {
			hdd_err("bcn_recv_abort_ind cb reg failed = %d",
				qdf_status);
			errno = qdf_status_to_os_return(qdf_status);
			return errno;
		}

		qdf_status =
			sme_handle_bcn_recv_start(hdd_ctx->mac_handle,
						  adapter->vdev_id);
		if (QDF_IS_STATUS_ERROR(qdf_status))
			hdd_err("beacon receive start failed with status=%d",
				qdf_status);
	}

	errno = qdf_status_to_os_return(qdf_status);
	return errno;
}

int wlan_hdd_cfg80211_bcn_rcv_start(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    const void *data, int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_bcn_rcv_start(wiphy, wdev,
						  data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
