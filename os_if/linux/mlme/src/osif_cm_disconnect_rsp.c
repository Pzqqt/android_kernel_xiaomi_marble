/*
 * Copyright (c) 2012-2015, 2020 The Linux Foundation. All rights reserved.
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
 * DOC: osif_cm_disconnect_rsp.c
 *
 * This file maintains definitaions of disconnect response
 * fucntions.
 */

#include <wlan_cfg80211.h>
#include <linux/wireless.h>
#include "osif_cm_rsp.h"
#include "wlan_osif_priv.h"
#include "osif_cm_util.h"

/**
 * osif_validate_disconnect_and_reset_src_id() - Validate disconnection
 * and resets source and id
 * @osif_priv: Pointer to vdev osif priv
 * @rsp: Disconnect response from connectin manager
 *
 * This function validates disconnect response and if the disconnect
 * response is valid, resets the source and id of the command
 *
 * Context: Any context. Takes and releases cmd id spinlock.
 * Return: QDF_STATUS
 */

static QDF_STATUS
osif_validate_disconnect_and_reset_src_id(struct vdev_osif_priv *osif_priv,
					  struct wlan_cm_discon_rsp *rsp)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* Always drop internal disconnect */
	qdf_spinlock_acquire(&osif_priv->cm_info.cmd_id_lock);
	if (rsp->req.req.source == CM_INTERNAL_DISCONNECT) {
		osif_debug("ignore internal disconnect");
		status = QDF_STATUS_E_INVAL;
		goto rel_lock;
	}

	/*
	 * Send to kernel only if last osif cmd type is disconnect and
	 * cookie match else drop. If cookie match reset the cookie
	 * and source
	 */
	if (rsp->req.cm_id != osif_priv->cm_info.last_id ||
	    rsp->req.req.source != osif_priv->cm_info.last_source) {
		osif_debug("Ignore as cm_id(%d)/src(%d) didn't match stored cm_id(%d)/src(%d)",
			   rsp->req.cm_id, rsp->req.req.source,
			   osif_priv->cm_info.last_id,
			   osif_priv->cm_info.last_source);
		status = QDF_STATUS_E_INVAL;
		goto rel_lock;
	}

	osif_cm_reset_id_and_src_no_lock(osif_priv);
rel_lock:
	qdf_spinlock_release(&osif_priv->cm_info.cmd_id_lock);

	return status;
}

#if defined(CFG80211_DISCONNECTED_V2) || \
(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0))
static void
osif_cm_indicate_disconnect(struct net_device *dev,
			    enum ieee80211_reasoncode reason,
			    bool locally_generated, const u8 *ie,
			    size_t ie_len, gfp_t gfp)
{
	cfg80211_disconnected(dev, reason, ie, ie_len, locally_generated, gfp);
}
#else
static void
osif_cm_indicate_disconnect(struct net_device *dev,
			    enum ieee80211_reasoncode reason,
			    bool locally_generated, const u8 *ie,
			    size_t ie_len, gfp_t gfp)
{
	cfg80211_disconnected(dev, reason, ie, ie_len, gfp);
}
#endif

static enum ieee80211_reasoncode
osif_cm_get_disconnect_reason(struct vdev_osif_priv *osif_priv, uint16_t reason)
{
	enum ieee80211_reasoncode ieee80211_reason = WLAN_REASON_UNSPECIFIED;

	osif_priv->cm_info.last_disconnect_reason =
					osif_cm_mac_to_qca_reason(reason);
	if (reason < REASON_PROP_START)
		ieee80211_reason = reason;
	/*
	 * Applications expect reason code as 0 for beacon miss failure
	 * due to backward compatibility. So send ieee80211_reason as 0.
	 */
	if (reason == REASON_BEACON_MISSED)
		ieee80211_reason = 0;

	return ieee80211_reason;
}

QDF_STATUS osif_disconnect_handler(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_discon_rsp *rsp)
{
	enum ieee80211_reasoncode ieee80211_reason;
	struct vdev_osif_priv *osif_priv = wlan_vdev_get_ospriv(vdev);
	bool locally_generated = true;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	ieee80211_reason =
		osif_cm_get_disconnect_reason(osif_priv,
					      rsp->req.req.reason_code);

	osif_nofl_info("%s(vdevid-%d): " QDF_MAC_ADDR_FMT " %sdisconnect " QDF_MAC_ADDR_FMT " cm_id %d source %d reason:%u %s vendor:%u %s",
		       osif_priv->wdev->netdev->name,
		       rsp->req.req.vdev_id,
		       QDF_MAC_ADDR_REF(wlan_vdev_mlme_get_macaddr(vdev)),
		       locally_generated ? "locally-generated " : "",
		       QDF_MAC_ADDR_REF(rsp->req.req.bssid.bytes),
		       rsp->req.cm_id, rsp->req.req.source, ieee80211_reason,
		       ucfg_cm_reason_code_to_str(rsp->req.req.reason_code),
		       osif_priv->cm_info.last_disconnect_reason,
		       osif_cm_qca_reason_to_str(osif_priv->cm_info.last_disconnect_reason));

	/* Unlink bss if disconnect is from peer or south bound */
	if (rsp->req.req.source == CM_PEER_DISCONNECT ||
	    rsp->req.req.source == CM_SB_DISCONNECT)
		osif_cm_unlink_bss(vdev, osif_priv, &rsp->req.req.bssid,
				   NULL, 0);

	status = osif_validate_disconnect_and_reset_src_id(osif_priv, rsp);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_cm_disconnect_comp_ind(vdev, rsp, OSIF_NOT_HANDLED);
		return status;
	}

	if (rsp->req.req.source == CM_PEER_DISCONNECT)
		locally_generated = false;

	osif_cm_disconnect_comp_ind(vdev, rsp, OSIF_PRE_USERSPACE_UPDATE);
	osif_cm_indicate_disconnect(osif_priv->wdev->netdev, ieee80211_reason,
				    locally_generated, rsp->ap_discon_ie.ptr,
				    rsp->ap_discon_ie.len, GFP_KERNEL);

	osif_cm_disconnect_comp_ind(vdev, rsp, OSIF_POST_USERSPACE_UPDATE);
	qdf_event_set(&osif_priv->cm_info.disconnect_complete);

	return status;
}
