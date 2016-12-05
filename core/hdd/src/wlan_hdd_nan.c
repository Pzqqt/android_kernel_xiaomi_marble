/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_hdd_nan.c
 *
 * WLAN Host Device Driver NAN API implementation
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <net/cfg80211.h>
#include <ani_global.h>
#include "sme_api.h"
#include "nan_api.h"
#include "wlan_hdd_main.h"
#include "wlan_hdd_nan.h"

/**
 * __wlan_hdd_cfg80211_nan_request() - cfg80211 NAN request handler
 * @wiphy: driver's wiphy struct
 * @wdev: wireless device to which the request is targeted
 * @data: actual request data (netlink-encapsulated)
 * @data_len: length of @data
 *
 * This is called when userspace needs to send a nan request to
 * firmware. The wlan host driver simply de-encapsulates the
 * request from the netlink payload and then forwards it to
 * firmware via SME.
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_nan_request(struct wiphy *wiphy,
					   struct wireless_dev *wdev,
					   const void *data,
					   int data_len)
{
	tNanRequestReq nan_req;
	QDF_STATUS status;
	int ret_val;
	hdd_context_t *hdd_ctx = wiphy_priv(wiphy);

	ENTER_DEV(wdev->netdev);

	ret_val = wlan_hdd_validate_context(hdd_ctx);
	if (ret_val)
		return ret_val;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (!hdd_ctx->config->enable_nan_support) {
		hdd_err("NaN support is not enabled in INI");
		return -EPERM;
	}

	nan_req.request_data_len = data_len;
	nan_req.request_data = data;

	status = sme_nan_request(&nan_req);
	if (QDF_STATUS_SUCCESS != status) {
		ret_val = -EINVAL;
	}
	return ret_val;
}

/**
 * wlan_hdd_cfg80211_nan_request() - handle NAN request
 * @wiphy:   pointer to wireless wiphy structure.
 * @wdev:    pointer to wireless_dev structure.
 * @data:    Pointer to the data to be passed via vendor interface
 * @data_len:Length of the data to be passed
 *
 * This function is called by userspace to send a NAN request to
 * firmware.  This is an SSR-protected wrapper function.
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_nan_request(struct wiphy *wiphy,
				  struct wireless_dev *wdev,
				  const void *data,
				  int data_len)

{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_nan_request(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_cfg80211_nan_callback() - cfg80211 NAN event handler
 * @ctx: global HDD context
 * @msg: NAN event message
 *
 * This is a callback function and it gets called when we need to report
 * a nan event to userspace.  The wlan host driver simply encapsulates the
 * event into a netlink payload and then forwards it to userspace via a
 * cfg80211 vendor event.
 *
 * Return: nothing
 */
void wlan_hdd_cfg80211_nan_callback(void *ctx, tSirNanEvent *msg)
{
	hdd_context_t *hdd_ctx = ctx;
	struct sk_buff *vendor_event;
	int status;

	if (NULL == msg) {
		hdd_err("msg received here is null");
		return;
	}

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		return;

	vendor_event =
		cfg80211_vendor_event_alloc(hdd_ctx->wiphy,
					    NULL,
					    msg->event_data_len + NLMSG_HDRLEN,
					    QCA_NL80211_VENDOR_SUBCMD_NAN_INDEX,
					    GFP_KERNEL);

	if (!vendor_event) {
		hdd_err("cfg80211_vendor_event_alloc failed");
		return;
	}
	if (nla_put(vendor_event, QCA_WLAN_VENDOR_ATTR_NAN,
		    msg->event_data_len, msg->event_data)) {
		hdd_err("QCA_WLAN_VENDOR_ATTR_NAN put fail");
		kfree_skb(vendor_event);
		return;
	}
	cfg80211_vendor_event(vendor_event, GFP_KERNEL);
}

/**
 * wlan_hdd_nan_is_supported() - HDD NAN support query function
 *
 * This function is called to determine if NAN is supported by the
 * driver and by the firmware.
 *
 * Return: true if NAN is supported by the driver and firmware
 */
bool wlan_hdd_nan_is_supported(void)
{
	return sme_is_feature_supported_by_fw(NAN);
}
