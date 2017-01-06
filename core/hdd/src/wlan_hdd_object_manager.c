/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 *  DOC: HDD object manager API source file to create/destroy PSOC,
 *  PDEV, VDEV and PEER objects.
 */

#include <wlan_hdd_object_manager.h>

QDF_STATUS hdd_create_and_store_psoc(hdd_context_t *hdd_ctx, uint8_t psoc_id)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_objmgr_psoc_obj_create(psoc_id, WLAN_DEV_OL);
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	hdd_ctx->hdd_psoc = psoc;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_release_and_destroy_psoc(hdd_context_t *hdd_ctx)
{
	struct wlan_objmgr_psoc *psoc = hdd_ctx->hdd_psoc;

	hdd_ctx->hdd_psoc = NULL;
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	wlan_objmgr_psoc_obj_delete(psoc);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_create_and_store_pdev(hdd_context_t *hdd_ctx)
{
	struct wlan_objmgr_psoc *psoc = hdd_ctx->hdd_psoc;
	struct wlan_objmgr_pdev *pdev;

	if (!psoc) {
		hdd_err("Psoc NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev = wlan_objmgr_pdev_obj_create(psoc, NULL);
	if (!pdev) {
		hdd_err("pdev obj create failed");
		return QDF_STATUS_E_FAILURE;
	}
	hdd_ctx->hdd_pdev = pdev;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_release_and_destroy_pdev(hdd_context_t *hdd_ctx)
{
	struct wlan_objmgr_pdev *pdev = hdd_ctx->hdd_pdev;

	hdd_ctx->hdd_pdev = NULL;
	if (!pdev)
		return QDF_STATUS_E_FAILURE;

	wlan_objmgr_pdev_obj_delete(pdev);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_create_and_store_vdev(struct wlan_objmgr_pdev *pdev,
				hdd_adapter_t *adapter)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_peer *peer;
	struct wlan_vdev_create_params vdev_params;

	vdev_params.opmode = adapter->device_mode;
	qdf_mem_copy(vdev_params.macaddr, adapter->macAddressCurrent.bytes,
						QDF_NET_MAC_ADDR_MAX_LEN);
	if (!pdev) {
		hdd_err("pdev NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = wlan_objmgr_vdev_obj_create(pdev, &vdev_params);
	if (!vdev) {
		hdd_err("vdev obj create fails");
		return QDF_STATUS_E_FAILURE;
	}

	if (adapter->sessionId != wlan_vdev_get_id(vdev)) {
		hdd_err("session id and vdev id mismatch");
		wlan_objmgr_vdev_obj_delete(vdev);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	peer = wlan_objmgr_peer_obj_create(vdev, WLAN_PEER_SELF,
					vdev_params.macaddr);
	if (!peer) {
		hdd_err("obj manager self peer create fails for adapter %d",
					adapter->device_mode);
		wlan_objmgr_vdev_obj_delete(vdev);
		return QDF_STATUS_E_FAILURE;
	}
	adapter->hdd_vdev = vdev;

	wlan_objmgr_peer_ref_peer(peer);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_release_and_destroy_vdev(hdd_adapter_t *adapter)
{
	struct wlan_objmgr_vdev *vdev = adapter->hdd_vdev;

	adapter->hdd_vdev = NULL;
	if (!vdev)
		return QDF_STATUS_E_FAILURE;

	if (hdd_remove_peer_object(vdev,
			wlan_vdev_mlme_get_macaddr(vdev))) {
		hdd_err("Self peer delete fails");
		return QDF_STATUS_E_FAILURE;
	}

	wlan_objmgr_vdev_obj_delete(vdev);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_add_peer_object(struct wlan_objmgr_vdev *vdev,
					enum tQDF_ADAPTER_MODE adapter_mode,
					uint8_t *mac_addr)
{
	enum wlan_peer_type peer_type;

	if ((adapter_mode == QDF_STA_MODE) ||
		(adapter_mode == QDF_P2P_CLIENT_MODE)) {
		peer_type = WLAN_PEER_AP;
	} else if ((adapter_mode == QDF_SAP_MODE) ||
		(adapter_mode == QDF_P2P_GO_MODE)) {
		peer_type = WLAN_PEER_STA;
	} else if (adapter_mode == QDF_IBSS_MODE) {
		peer_type = WLAN_PEER_IBSS;
	} else {
		hdd_err("Unsupported device mode %d", adapter_mode);
		return QDF_STATUS_E_FAILURE;
	}

	if (!vdev) {
		hdd_err("vdev NULL");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	if (!wlan_objmgr_peer_obj_create(vdev, peer_type, mac_addr))
		return QDF_STATUS_E_FAILURE;

	hdd_info("Peer object "MAC_ADDRESS_STR" add success!",
					MAC_ADDR_ARRAY(mac_addr));

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hdd_remove_peer_object(struct wlan_objmgr_vdev *vdev,
						uint8_t *mac_addr)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_peer *peer;

	if (!vdev) {
		hdd_err("vdev NULL");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		hdd_err("Psoc NUll");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	peer = wlan_objmgr_find_peer(psoc, mac_addr);
	if (peer) {
		if (wlan_objmgr_peer_obj_delete(peer))
			return QDF_STATUS_E_FAILURE;

		hdd_info("Peer obj "MAC_ADDRESS_STR" deleted",
				MAC_ADDR_ARRAY(mac_addr));
		return QDF_STATUS_SUCCESS;
	}

	hdd_err("Peer obj "MAC_ADDRESS_STR" not found",
				MAC_ADDR_ARRAY(mac_addr));

	return QDF_STATUS_E_FAILURE;
}
