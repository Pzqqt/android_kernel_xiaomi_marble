/*
 * Copyright (c) 2012-2015,2020-2021 The Linux Foundation. All rights reserved.
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
 * DOC: osif_cm_roam_rsp.c
 *
 * This file maintains definitaions of roam response apis.
 */

#include <linux/version.h>
#include <linux/nl80211.h>
#include <net/cfg80211.h>
#include <wlan_osif_priv.h>
#include "osif_cm_rsp.h"
#include <osif_cm_util.h>
#include <wlan_cfg80211.h>
#include <wlan_cfg80211_scan.h>

#ifdef CONN_MGR_ADV_FEATURE
#ifdef WLAN_FEATURE_FILS_SK
static inline void osif_update_fils_hlp_data(struct net_device *dev,
					     struct wlan_objmgr_vdev *vdev,
					     struct wlan_cm_connect_resp *rsp)
{
	if (rsp->connect_ies.fils_ie && rsp->connect_ies.fils_ie->hlp_data_len)
		osif_cm_set_hlp_data(dev, vdev, rsp);
}
#else
static inline void osif_update_fils_hlp_data(struct net_device *dev,
					     struct wlan_objmgr_vdev *vdev,
					     struct wlan_cm_connect_resp *rsp)
{
}
#endif

/**
 * osif_roamed_ind() - send roamed indication to cfg80211
 * @dev: network device
 * @bss: cfg80211 roamed bss pointer
 * @req_ie: IEs used in reassociation request
 * @req_ie_len: Length of the @req_ie
 * @resp_ie: IEs received in successful reassociation response
 * @resp_ie_len: Length of @resp_ie
 *
 * Return: none
 */
#if defined CFG80211_ROAMED_API_UNIFIED || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
static void osif_roamed_ind(struct net_device *dev, struct cfg80211_bss *bss,
			    const uint8_t *req_ie,
			    size_t req_ie_len, const uint8_t *resp_ie,
			    size_t resp_ie_len)
{
	struct cfg80211_roam_info info = {0};

	info.bss = bss;
	info.req_ie = req_ie;
	info.req_ie_len = req_ie_len;
	info.resp_ie = resp_ie;
	info.resp_ie_len = resp_ie_len;
	cfg80211_roamed(dev, &info, GFP_KERNEL);
}
#else
static inline void osif_roamed_ind(struct net_device *dev,
				   struct cfg80211_bss *bss,
				   const uint8_t *req_ie, size_t req_ie_len,
				   const uint8_t *resp_ie,
				   size_t resp_ie_len)
{
	cfg80211_roamed_bss(dev, bss, req_ie, req_ie_len, resp_ie, resp_ie_len,
			    GFP_KERNEL);
}
#endif

void osif_indicate_reassoc_results(struct wlan_objmgr_vdev *vdev,
				   struct vdev_osif_priv *osif_priv,
				   struct wlan_cm_connect_resp *rsp)
{
	struct net_device *dev = osif_priv->wdev->netdev;
	size_t req_len = 0;
	const uint8_t *req_ie = NULL;
	size_t rsp_len = 0;
	const uint8_t *rsp_ie = NULL;
	struct cfg80211_bss *bss;
	struct ieee80211_channel *chan;

	if (QDF_IS_STATUS_ERROR(rsp->connect_status))
		return;

	chan = ieee80211_get_channel(osif_priv->wdev->wiphy,
				     rsp->freq);
	bss = wlan_cfg80211_get_bss(osif_priv->wdev->wiphy, chan,
				    rsp->bssid.bytes, rsp->ssid.ssid,
				    rsp->ssid.length);
	if (!bss)
		osif_warn("not able to find bss");
	osif_cm_get_assoc_req_ie_data(&rsp->connect_ies.assoc_req,
				      &req_len, &req_ie);
	osif_cm_get_assoc_rsp_ie_data(&rsp->connect_ies.assoc_rsp,
				      &rsp_len, &rsp_ie);
	osif_roamed_ind(dev, bss, req_ie, req_len, rsp_ie, rsp_len);

	osif_cm_save_gtk(vdev, rsp);

	/* Add osif_send_roam_auth_event (wlan_hdd_send_roam_auth_event) */

	osif_update_fils_hlp_data(dev, vdev, rsp);
}
#endif
