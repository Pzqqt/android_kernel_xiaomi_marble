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

static QDF_STATUS
osif_validate_reassoc_and_reset_src_id(struct vdev_osif_priv *osif_priv,
				       wlan_cm_id cm_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	qdf_spinlock_acquire(&osif_priv->cm_info.cmd_id_lock);
	if (cm_id != osif_priv->cm_info.last_id) {
		osif_debug("Ignore as cm_id(%d) didn't match stored cm_id(%d)",
			   cm_id, osif_priv->cm_info.last_id);
		status = QDF_STATUS_E_INVAL;
		goto rel_lock;
	}
	osif_cm_reset_id_and_src_no_lock(osif_priv);

rel_lock:
	qdf_spinlock_release(&osif_priv->cm_info.cmd_id_lock);

	return status;
}

#ifdef CONN_MGR_ADV_FEATURE
static void osif_indicate_reassoc_results(struct wlan_objmgr_vdev *vdev,
					  struct vdev_osif_priv *osif_priv,
					  struct wlan_cm_roam_resp *rsp)
{
	/*
	 * To notify kernel on connection completion
	 */
}
#else
static void osif_indicate_reassoc_results(struct wlan_objmgr_vdev *vdev,
					  struct vdev_osif_priv *osif_priv,
					  struct wlan_cm_roam_resp *rsp)
{}
#endif

QDF_STATUS osif_reassoc_handler(struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_roam_resp *rsp)
{
	struct vdev_osif_priv *osif_priv  = wlan_vdev_get_ospriv(vdev);
	QDF_STATUS status;
	enum wlan_cm_source source;

	osif_nofl_info("%s(vdevid-%d): " QDF_MAC_ADDR_FMT " Roam with " QDF_MAC_ADDR_FMT " SSID \"%.*s\" is %s cm_id %d cm_reason %d status_code %d",
		       osif_priv->wdev->netdev->name, rsp->vdev_id,
		       QDF_MAC_ADDR_REF(wlan_vdev_mlme_get_macaddr(vdev)),
		       QDF_MAC_ADDR_REF(rsp->bssid.bytes),
		       rsp->ssid.length, rsp->ssid.ssid,
		       rsp->reassoc_status ? "FAILURE" : "SUCCESS", rsp->cm_id,
		       rsp->reason, rsp->status_code);
	source = osif_priv->cm_info.last_source;
	status = osif_validate_reassoc_and_reset_src_id(osif_priv, rsp->cm_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_cm_reassoc_comp_ind(vdev, rsp, OSIF_NOT_HANDLED);
		return status;
	}

	osif_cm_reassoc_comp_ind(vdev, rsp, OSIF_PRE_USERSPACE_UPDATE);
	osif_indicate_reassoc_results(vdev, osif_priv, rsp);
	osif_cm_reassoc_comp_ind(vdev, rsp, OSIF_POST_USERSPACE_UPDATE);

	return QDF_STATUS_SUCCESS;
}
