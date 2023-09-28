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
 * DOC: wlan_cfg80211_vdev_mlme.h
 *
 * This header file provide API declarations for osif layer
 * to get and set vdev mgr mlme structure parameters
 */

#ifndef __WLAN_CFG80211_VDEV_MLME_H__
#define __WLAN_CFG80211_VDEV_MLME_H__
#include <wlan_objmgr_vdev_obj.h>
#include <qdf_nbuf.h>
#include "include/wlan_vdev_mlme.h"

/**
 * wlan_cfg80211_vdev_mlme_get_param_ssid() – cfg80211 MLME API to
 * get ssid
 * @vdev: pointer to vdev object
 * @ssid: pointer to store the ssid
 * @ssid_len: pointer to store the ssid length value
 *
 * Return: 0 on success, negative value on failure
 */
int
wlan_cfg80211_vdev_mlme_get_param_ssid(struct wlan_objmgr_vdev *vdev,
				       uint8_t *ssid,
				       uint8_t *ssid_len);

/**
 * wlan_cfg80211_vdev_mlme_get_trans_bssid() – cfg80211 MLME API to
 * get trans bssid
 * @vdev: pointer to vdev object
 * @addr: pointer to store the addr of the transmission bssid
 *
 * Return: 0 on success, negative value on failure
 */
int
wlan_cfg80211_vdev_mlme_get_trans_bssid(struct wlan_objmgr_vdev *vdev,
					uint8_t *addr);

/**
 * wlan_cfg80211_vdev_mlme_set_param() – cfg80211 MLME API to fill common
 * parameters of vdev_mlme object
 * @vdev: pointer to vdev object
 * @param_id: param id for which the value should be set
 * @mlme_cfg: structure of a union to set the parameter
 *
 * Return: void
 */
void
wlan_cfg80211_vdev_mlme_set_param(struct wlan_objmgr_vdev *vdev,
				  enum wlan_mlme_cfg_id param_id,
				  struct wlan_vdev_mgr_cfg mlme_cfg);

/**
 * wlan_cfg80211_vdev_mlme_get_param() – cfg80211 MLME API to get common
 * parameters of vdev_mlme object
 * @vdev: pointer to vdev object
 * @param_id: param id for which the value should be set
 * @value: pointer to store the value of the req vdev_mlme object
 *
 * Return: void
 */
void
wlan_cfg80211_vdev_mlme_get_param(struct wlan_objmgr_vdev *vdev,
				  enum wlan_mlme_cfg_id param_id,
				  uint32_t *value);
#endif /* __WLAN_CFG80211_VDEV_MLME_H__ */
