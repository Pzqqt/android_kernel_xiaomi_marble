/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_hdd_coap.c
 *
 * The implementation of CoAP offload configuration
 *
 */

#include "wlan_hdd_main.h"
#include "wlan_hdd_coap.h"
#include "osif_sync.h"
#include "wlan_hdd_object_manager.h"
#include "wlan_cfg80211_coap.h"

/**
 * __wlan_hdd_cfg80211_coap_offload() - configure CoAP offloading
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: pointer to netlink TLV buffer
 * @data_len: the length of @data in bytes
 *
 * Return: An error code or 0 on success.
 */
static int
__wlan_hdd_cfg80211_coap_offload(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 const void *data, int data_len)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(wdev->netdev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int errno;
	struct wlan_objmgr_vdev *vdev;

	hdd_enter_dev(wdev->netdev);

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno != 0)
		return errno;

	if (adapter->device_mode != QDF_STA_MODE)
		return -ENOTSUPP;

	vdev = hdd_objmgr_get_vdev_by_user(adapter, WLAN_COAP_ID);
	if (!vdev)
		return -EINVAL;

	errno = wlan_cfg80211_coap_offload(wiphy, vdev, data, data_len);
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_COAP_ID);
	return errno;
}

int wlan_hdd_cfg80211_coap_offload(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   const void *data, int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_coap_offload(wiphy, wdev, data, data_len);
	osif_vdev_sync_op_stop(vdev_sync);
	return errno;
}
