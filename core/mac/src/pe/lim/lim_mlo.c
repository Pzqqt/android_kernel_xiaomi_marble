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
#include <wlan_mlo_mgr_peer.h>
#include <lim_assoc_utils.h>

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

void lim_mlo_notify_peer_disconn(struct pe_session *pe_session,
				 tpDphHashNode sta_ds)
{
	struct wlan_objmgr_peer *peer;
	struct mac_context *mac_ctx;

	if (!pe_session) {
		pe_err("pe session is null");
		return;
	}
	if (!sta_ds) {
		pe_err("sta ds is null");
		return;
	}
	mac_ctx = pe_session->mac_ctx;
	if (!mac_ctx) {
		pe_err("mac context is null");
		return;
	}

	peer = wlan_objmgr_get_peer_by_mac(mac_ctx->psoc,
					   sta_ds->staAddr,
					   WLAN_LEGACY_MAC_ID);
	if (!peer) {
		pe_err("peer is null");
		return;
	}

	if (lim_is_mlo_conn(pe_session, sta_ds))
		wlan_mlo_partner_peer_disconnect_notify(peer);

	wlan_objmgr_peer_release_ref(peer, WLAN_LEGACY_MAC_ID);
}

void lim_mlo_cleanup_partner_peer(struct wlan_objmgr_peer *peer)
{
	struct mac_context *mac_ctx;
	uint16_t aid;
	tpDphHashNode sta_ds;
	struct pe_session *pe_session;
	tpSirAssocReq tmp_assoc_req;
	struct wlan_objmgr_vdev *vdev;

	mac_ctx = cds_get_context(QDF_MODULE_ID_PE);
	if (!mac_ctx) {
		pe_err("mac ctx is null");
		return;
	}

	vdev = wlan_peer_get_vdev(peer);
	if (!vdev) {
		pe_err("vdev is null");
		return;
	}

	pe_session = pe_find_session_by_vdev_id(
			mac_ctx, vdev->vdev_objmgr.vdev_id);
	if (!pe_session) {
		pe_err("pe session is null");
		return;
	}

	sta_ds = dph_lookup_hash_entry(mac_ctx, peer->macaddr, &aid,
				       &pe_session->dph.dphHashTable);

	if (!sta_ds) {
		pe_err("sta ds is null");
		return;
	}

	sta_ds->mlmStaContext.cleanupTrigger = eLIM_MLO_PARTNER_PEER;

	lim_cleanup_rx_path(mac_ctx, sta_ds, pe_session, true);

	if (pe_session->parsedAssocReq) {
		tmp_assoc_req = pe_session->parsedAssocReq[sta_ds->assocId];
		if (tmp_assoc_req) {
			lim_free_assoc_req_frm_buf(tmp_assoc_req);
			qdf_mem_free(tmp_assoc_req);
			tmp_assoc_req = NULL;
		}

		pe_session->parsedAssocReq[sta_ds->assocId] = NULL;
	}
}

void lim_mlo_set_mld_mac_peer(tpDphHashNode sta_ds,
			      uint8_t peer_mld_addr[QDF_MAC_ADDR_SIZE])
{
	WLAN_ADDR_COPY(sta_ds->mld_addr, peer_mld_addr);
}

bool lim_is_mlo_conn(struct pe_session *session, tpDphHashNode sta_ds)
{
	if (!sta_ds) {
		pe_err("sta ds is null");
		return false;
	}

	if (!session) {
		pe_err("session is null");
		return false;
	}

	return sta_ds->mlmStaContext.eht_capable &&
	       IS_DOT11_MODE_EHT(session->dot11mode);
}

void lim_set_mlo_recv_assoc(tpDphHashNode sta_ds, bool mlo_recv_assoc_frm)
{
	if (!sta_ds) {
		pe_err("sta ds is null");
		return;
	}

	sta_ds->recv_assoc_frm = mlo_recv_assoc_frm;
}

bool lim_is_mlo_recv_assoc(tpDphHashNode sta_ds)
{
	if (!sta_ds) {
		pe_err("sta ds is null");
		return false;
	}

	return sta_ds->recv_assoc_frm;
}
