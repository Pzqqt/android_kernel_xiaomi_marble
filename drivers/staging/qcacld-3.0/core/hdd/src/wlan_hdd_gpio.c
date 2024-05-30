/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: defines driver functions interfacing with linux kernel
 */
#include <qdf_list.h>
#include <qdf_status.h>
#include <linux/wireless.h>
#include <linux/netdevice.h>
#include <wlan_cfg80211.h>
#include <wlan_osif_priv.h>
#include <osif_psoc_sync.h>
#include <qdf_mem.h>
#include <wlan_utility.h>
#include "wlan_hdd_main.h"
#include "cfg_ucfg_api.h"
#include <wlan_hdd_gpio.h>

/**
 * wlan_cfg80211_set_gpio_config - Set the gpio configuration
 * @wiphy: pointer to wiphy
 * @wdev: pointer to wireless_wdev
 * @data: pointer to data
 * @data_len: data length
 *
 * __wlan_cfg80211_set_gpio_config will forward the GPIO setting to FW by
 * WMI_GPIO_CONFIG/OUTPUT_CMDID
 *
 * Return: 0 on success; errno on failure
 */
static int
__wlan_hdd_cfg80211_set_gpio_config(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    const void *data,
				    int data_len)
{
	int ret;
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter;

	hdd_enter();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Command not allowed in FTM mode");
		return -EPERM;
	}

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	ret = wlan_cfg80211_start_gpio_config(wiphy,
					      hdd_ctx->psoc,
					      data, data_len);
	hdd_exit();

	return ret;
}

/**
 * wlan_hdd_cfg80211_set_gpio_config() - Set GPIO Configuration
 * @wiphy: wiphy structure pointer
 * @wdev: Wireless device structure pointer
 * @data: Pointer to the data received
 * @data_len: Length of @data
 *
 * Return: 0 on success; errno on failure
 */
int wlan_hdd_cfg80211_set_gpio_config(struct wiphy *wiphy,
				      struct wireless_dev *wdev,
				      const void *data, int data_len)
{
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_gpio_config(wiphy,
						    wdev,
						    data,
						    data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}
