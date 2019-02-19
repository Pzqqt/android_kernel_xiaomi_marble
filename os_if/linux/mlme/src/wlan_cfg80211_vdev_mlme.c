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
 * DOC: wlan_cfg80211_vdev_mlme.c
 *
 * This file provide API definitions for osif layer
 * to get and set vdev mgr mlme structure parameters
 */

#include <wlan_cfg80211_vdev_mlme.h>

int
wlan_cfg80211_vdev_mlme_get_param_ssid(struct wlan_objmgr_vdev *vdev,
				       uint8_t *ssid,
				       uint8_t *ssid_len)
{
	if (!vdev) {
		cfg80211_err("VDEV is NULL!!!!");
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	ucfg_wlan_vdev_mgr_get_param_ssid(vdev, ssid, ssid_len);
	return qdf_status_to_os_return(QDF_STATUS_SUCCESS);
}

int
wlan_cfg80211_vdev_mlme_get_trans_bssid(struct wlan_objmgr_vdev *vdev,
					uint8_t *addr)
{
	if (!vdev) {
		cfg80211_err("VDEV is NULL!!!!");
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	ucfg_wlan_vdev_mlme_get_trans_bssid(vdev, addr);
	return qdf_status_to_os_return(QDF_STATUS_SUCCESS);
}

void
wlan_cfg80211_vdev_mlme_set_param(struct wlan_objmgr_vdev *vdev,
				  enum wlan_mlme_cfg_id param_id,
				  struct wlan_vdev_mgr_cfg mlme_cfg)
{
	if (!vdev) {
		cfg80211_err("VDEV is NULL!!!!");
		return;
	}

	ucfg_wlan_vdev_mgr_set_param(vdev, param_id, mlme_cfg);
}

void
wlan_cfg80211_vdev_mlme_get_param(struct wlan_objmgr_vdev *vdev,
				  enum wlan_mlme_cfg_id param_id,
				  uint32_t *value)
{
	if (!vdev) {
		cfg80211_err("VDEV is NULL!!!!");
		return;
	}

	ucfg_wlan_vdev_mgr_get_param(vdev, param_id, value);
}
