/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC : lim_mlo.c
 *
 * WLAN Host Device Driver file for 802.11be (Extremely High Throughput)
 * support.
 *
 */

#include "lim_mlo.h"
#include "sch_api.h"
#include "lim_types.h"
#include "wlan_mlo_mgr_ap.h"

/**
 * lim_send_mlo_ie_update - mlo ie is changed, populate new beacon template
 * @session: pe session
 *
 * Return: void
 */
static void lim_send_mlo_ie_update(struct mac_context *mac_ctx,
				   struct pe_session *session)
{
	if (QDF_IS_STATUS_ERROR(
		sch_set_fixed_beacon_fields(mac_ctx, session))) {
		pe_err("Unable to update mlo IE in beacon");
		return;
	}

	lim_send_beacon_ind(mac_ctx, session, REASON_MLO_IE_UPDATE);
}

QDF_STATUS lim_partner_link_info_change(struct wlan_objmgr_vdev *vdev)
{
	struct pe_session *session;
	struct mac_context *mac;

	mac = cds_get_context(QDF_MODULE_ID_PE);
	if (!mac) {
		pe_err("mac ctx is null");
		return QDF_STATUS_E_INVAL;
	}
	if (!vdev) {
		pe_err("vdev is null");
		return QDF_STATUS_E_INVAL;
	}
	session = pe_find_session_by_vdev_id(
			mac, vdev->vdev_objmgr.vdev_id);
	if (!session) {
		pe_err("session is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (session->mlo_link_info.bcn_tmpl_exist)
		lim_send_mlo_ie_update(mac, session);

	return QDF_STATUS_SUCCESS;
}

uint8_t lim_get_max_simultaneous_link_num(struct pe_session *session)
{
	struct wlan_objmgr_vdev *wlan_vdev_list[WLAN_UMAC_MLO_MAX_VDEVS];
	uint16_t vdev_count = 0;

	mlo_ap_get_vdev_list(session->vdev, &vdev_count,
			     wlan_vdev_list);

	return vdev_count;
}

void lim_mlo_release_vdev_ref(struct wlan_objmgr_vdev *vdev)
{
	mlo_release_vdev_ref(vdev);
}

struct pe_session *pe_find_partner_session_by_link_id(
			struct pe_session *session, uint8_t link_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct mac_context *mac;

	mac = cds_get_context(QDF_MODULE_ID_PE);
	if (!mac) {
		pe_err("mac ctx is null");
		return NULL;
	}

	if (!session) {
		pe_err("session is null");
		return NULL;
	}

	vdev = mlo_get_partner_vdev_by_link_id(session->vdev, link_id);

	if (!vdev) {
		pe_err("vdev is null");
		return NULL;
	}

	return pe_find_session_by_vdev_id(
			mac, vdev->vdev_objmgr.vdev_id);
}

void lim_get_mlo_vdev_list(struct pe_session *session, uint16_t *vdev_count,
			   struct wlan_objmgr_vdev **wlan_vdev_list)
{
	mlo_ap_get_vdev_list(session->vdev, vdev_count,
			     wlan_vdev_list);
}
