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

static const struct nla_policy
beacon_reporting_params[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE] = {.type = NLA_U8},
	[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_ACTIVE_REPORTING] = {.type =
								     NLA_FLAG},
	[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_PERIOD] = {.type = NLA_U8},
};

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

	hdd_enter_dev(dev);

	errno = hdd_validate_adapter(adapter);
	if (errno)
		return errno;

	if (adapter->device_mode != QDF_STA_MODE) {
		hdd_err("Command not allowed as device not in STA mode");
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

	bcn_report =
		nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_BEACON_REPORTING_OP_TYPE]);
	hdd_debug("Bcn Report: OP type:%d", bcn_report);

	if (bcn_report == QCA_WLAN_VENDOR_BEACON_REPORTING_OP_START) {
		/* Vendor event is intended for Start*/
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
