/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "wlan_mlo_mgr_main.h"
#include "qdf_types.h"
#include "wlan_cmn.h"
#include "wlan_mlo_mgr_peer.h"
#include <wlan_mlo_mgr_ap.h>
#include <wlan_utility.h>
#include <wlan_reg_services_api.h>

static void
mld_get_best_primary_umac_w_rssi(struct wlan_mlo_peer_context *ml_peer,
				 struct wlan_objmgr_vdev *link_vdevs[])
{
	struct wlan_objmgr_peer *assoc_peer;

	assoc_peer = wlan_mlo_peer_get_assoc_peer(ml_peer);
	ml_peer->primary_umac_psoc_id = wlan_peer_get_psoc_id(assoc_peer);
}

void mlo_peer_assign_primary_umac(
		struct wlan_mlo_peer_context *ml_peer,
		struct wlan_mlo_link_peer_entry *peer_entry)
{
	/* If MLD is within single SOC, then assoc link becomes
	 * primary umac
	 */
	if (ml_peer->primary_umac_psoc_id == ML_PRIMARY_UMAC_ID_INVAL) {
		if (mlo_peer_is_assoc_peer(ml_peer, peer_entry->link_peer)) {
			peer_entry->is_primary = true;
			ml_peer->primary_umac_psoc_id =
				wlan_peer_get_psoc_id(peer_entry->link_peer);
		} else {
			peer_entry->is_primary = false;
		}
	} else {
		if (wlan_peer_get_psoc_id(peer_entry->link_peer) ==
				ml_peer->primary_umac_psoc_id)
			peer_entry->is_primary = true;
		else
			peer_entry->is_primary = false;
	}
}

QDF_STATUS mlo_peer_allocate_primary_umac(
		struct wlan_mlo_dev_context *ml_dev,
		struct wlan_mlo_peer_context *ml_peer,
		struct wlan_objmgr_vdev *link_vdevs[])
{
	struct wlan_mlo_link_peer_entry *peer_entry;
	struct wlan_objmgr_peer *assoc_peer = NULL;
	int32_t rssi;
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	uint8_t first_link_id = 0;
	bool primary_umac_set = false;
	uint8_t i, psoc_id;

	peer_entry = &ml_peer->peer_list[0];
	assoc_peer = peer_entry->link_peer;
	if (!assoc_peer)
		return QDF_STATUS_E_FAILURE;

	/* For Station mode, assign assoc peer as primary umac */
	if (wlan_peer_get_peer_type(assoc_peer) == WLAN_PEER_AP) {
		mlo_peer_assign_primary_umac(ml_peer, peer_entry);
		mlo_info("MLD ID %d ML Peer " QDF_MAC_ADDR_FMT " primary umac soc %d ",
			 ml_dev->mld_id,
			 QDF_MAC_ADDR_REF(ml_peer->peer_mld_addr.bytes),
			 ml_peer->primary_umac_psoc_id);

		return QDF_STATUS_SUCCESS;
	}

	/* If MLD is single chip MLO then assoc link becomes primary UMAC */
	/*
	 * if (ml_dev->single_chip_mlo) {
	 *	mlo_peer_assign_primary_umac(ml_peer, peer_entry);
	 *	return QDF_STATUS_SUCCESS;
	 * }
	 */

	if (mlo_ctx->mlo_is_force_primary_umac) {
		for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
			if (!link_vdevs[i])
				continue;

			psoc_id = wlan_vdev_get_psoc_id(link_vdevs[i]);
			if (!first_link_id)
				first_link_id = psoc_id;

			if (psoc_id == mlo_ctx->mlo_forced_primary_umac_id) {
				ml_peer->primary_umac_psoc_id = psoc_id;
				primary_umac_set = true;
				break;
			}
		}

		if (!primary_umac_set)
			ml_peer->primary_umac_psoc_id = first_link_id;

		mlo_info("MLD ID %d ML Peer " QDF_MAC_ADDR_FMT " primary umac soc %d ",
			 ml_dev->mld_id,
			 QDF_MAC_ADDR_REF(ml_peer->peer_mld_addr.bytes),
			 ml_peer->primary_umac_psoc_id);

		return QDF_STATUS_SUCCESS;
	}

	rssi = wlan_peer_get_rssi(assoc_peer);

	mld_get_best_primary_umac_w_rssi(ml_peer, link_vdevs);

	mlo_peer_assign_primary_umac(ml_peer, peer_entry);

	mlo_info("MLD ID %d ML Peer " QDF_MAC_ADDR_FMT " avg RSSI %d primary umac soc %d ",
		 ml_dev->mld_id,
		 QDF_MAC_ADDR_REF(ml_peer->peer_mld_addr.bytes),
		 ml_peer->avg_link_rssi, ml_peer->primary_umac_psoc_id);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlo_peer_free_primary_umac(
		struct wlan_mlo_dev_context *ml_dev,
		struct wlan_mlo_peer_context *ml_peer)
{
	return QDF_STATUS_SUCCESS;
}
