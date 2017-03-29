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
#include <wlan_osif_priv.h>


static void hdd_init_pdev_os_priv(hdd_context_t *hdd_ctx,
	struct pdev_osif_priv *os_priv)
{
	/* Initialize the OS private structure*/
	os_priv->wiphy = hdd_ctx->wiphy;
	wlan_cfg80211_scan_priv_init(hdd_ctx->hdd_pdev);
}

static void hdd_deinit_pdev_os_priv(hdd_context_t *hdd_ctx)
{
	wlan_cfg80211_scan_priv_deinit(hdd_ctx->hdd_pdev);
}

static void hdd_init_vdev_os_priv(hdd_adapter_t *adapter,
	struct vdev_osif_priv *os_priv)
{
	/* Initialize the vdev OS private structure*/
	os_priv->wdev = adapter->dev->ieee80211_ptr;
#ifdef CONVERGED_TDLS_ENABLE
	wlan_cfg80211_tdls_priv_init(os_priv);
#endif
}

static void hdd_init_psoc_qdf_ctx(struct wlan_objmgr_psoc *psoc)
{
	qdf_device_t qdf_ctx;

	qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	if (!qdf_ctx) {
		hdd_err("qdf ctx is null, can't set to soc object");
		return;
	}

	wlan_psoc_set_qdf_dev(psoc, qdf_ctx);
}

int hdd_objmgr_create_and_store_psoc(hdd_context_t *hdd_ctx, uint8_t psoc_id)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_objmgr_psoc_obj_create(psoc_id, WLAN_DEV_OL);
	if (!psoc)
		return -ENOMEM;

	hdd_init_psoc_qdf_ctx(psoc);
	hdd_ctx->hdd_psoc = psoc;

	return 0;
}

int hdd_objmgr_release_and_destroy_psoc(hdd_context_t *hdd_ctx)
{
	struct wlan_objmgr_psoc *psoc = hdd_ctx->hdd_psoc;

	hdd_ctx->hdd_psoc = NULL;
	if (!psoc)
		return -EINVAL;

	wlan_objmgr_print_ref_all_objects_per_psoc(psoc);

	return qdf_status_to_os_return(wlan_objmgr_psoc_obj_delete(psoc));
}

int hdd_objmgr_create_and_store_pdev(hdd_context_t *hdd_ctx)
{
	struct wlan_objmgr_psoc *psoc = hdd_ctx->hdd_psoc;
	struct wlan_objmgr_pdev *pdev;
	struct pdev_osif_priv *priv;

	if (!psoc) {
		hdd_err("Psoc NULL");
		return -EINVAL;
	}

	priv = qdf_mem_malloc(sizeof(*priv));
	if (priv == NULL) {
		hdd_err("pdev os obj create failed");
		return -ENOMEM;
	}
	pdev = wlan_objmgr_pdev_obj_create(psoc, priv);
	if (!pdev) {
		hdd_err("pdev obj create failed");
		return -ENOMEM;
	}
	hdd_ctx->hdd_pdev = pdev;
	hdd_init_pdev_os_priv(hdd_ctx, priv);
	wlan_pdev_obj_lock(pdev);
	wlan_pdev_set_tgt_if_handle(pdev, psoc->tgt_if_handle);
	wlan_pdev_obj_unlock(pdev);
	return 0;
}

int hdd_objmgr_release_and_destroy_pdev(hdd_context_t *hdd_ctx)
{
	struct wlan_objmgr_pdev *pdev = hdd_ctx->hdd_pdev;
	struct pdev_osif_priv *osif_priv;

	hdd_deinit_pdev_os_priv(hdd_ctx);
	hdd_ctx->hdd_pdev = NULL;
	if (!pdev)
		return -EINVAL;

	osif_priv = wlan_pdev_get_ospriv(pdev);
	wlan_pdev_reset_ospriv(pdev);
	qdf_mem_free(osif_priv);

	return qdf_status_to_os_return(wlan_objmgr_pdev_obj_delete(pdev));
}

int hdd_objmgr_create_and_store_vdev(struct wlan_objmgr_pdev *pdev,
				     hdd_adapter_t *adapter)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_peer *peer;
	struct vdev_osif_priv *osif_priv;
	struct wlan_vdev_create_params vdev_params;

	vdev_params.opmode = adapter->device_mode;
	qdf_mem_copy(vdev_params.macaddr, adapter->macAddressCurrent.bytes,
						QDF_NET_MAC_ADDR_MAX_LEN);
	if (!pdev) {
		hdd_err("pdev NULL");
		return -EINVAL;
	}

	osif_priv = qdf_mem_malloc(sizeof(*osif_priv));
	if (!osif_priv) {
		hdd_err("vdev os obj create failed");
		return -ENOMEM;
	}

	hdd_init_vdev_os_priv(adapter, osif_priv);
	vdev_params.osifp = osif_priv;

	vdev = wlan_objmgr_vdev_obj_create(pdev, &vdev_params);
	if (!vdev) {
		hdd_err("vdev obj create fails");
		return -ENOMEM;
	}

	peer = wlan_objmgr_peer_obj_create(vdev, WLAN_PEER_SELF,
					vdev_params.macaddr);
	if (!peer) {
		hdd_err("obj manager self peer create fails for adapter %d",
			adapter->device_mode);
		wlan_objmgr_vdev_obj_delete(vdev);
		return -ENOMEM;
	}

	/*
	 * To enable legacy use cases, we need to delay physical vdev destroy
	 * until after the sme session has been closed. We accomplish this by
	 * getting an additional reference here.
	 */
	wlan_objmgr_vdev_get_ref(vdev, WLAN_HDD_ID_OBJ_MGR);

	adapter->hdd_vdev = vdev;
	adapter->sessionId = wlan_vdev_get_id(vdev);

	return 0;
}

int hdd_objmgr_destroy_vdev(hdd_adapter_t *adapter)
{
	struct wlan_objmgr_vdev *vdev;
	struct vdev_osif_priv *osif_priv;

	vdev = adapter->hdd_vdev;
	if (!vdev)
		return -EINVAL;

	osif_priv = wlan_vdev_get_ospriv(vdev);
	wlan_vdev_reset_ospriv(vdev);
#ifdef CONVERGED_TDLS_ENABLE
	wlan_cfg80211_tdls_priv_deinit(osif_priv);
#endif
	qdf_mem_free(osif_priv);

	if (hdd_objmgr_remove_peer_object(vdev,
					  wlan_vdev_mlme_get_macaddr(vdev))) {
		hdd_err("Self peer delete failed");
		return -EINVAL;
	}

	return qdf_status_to_os_return(wlan_objmgr_vdev_obj_delete(vdev));
}

int hdd_objmgr_release_vdev(hdd_adapter_t *adapter)
{
	/* allow physical vdev destroy by releasing the hdd reference */
	wlan_objmgr_vdev_release_ref(adapter->hdd_vdev, WLAN_HDD_ID_OBJ_MGR);

	adapter->hdd_vdev = NULL;
	adapter->sessionId = HDD_SESSION_ID_INVALID;

	return 0;
}

int hdd_objmgr_release_and_destroy_vdev(hdd_adapter_t *adapter)
{
	int errno;

	errno = hdd_objmgr_destroy_vdev(adapter);
	if (errno)
		return errno;

	return hdd_objmgr_release_vdev(adapter);
}

int hdd_objmgr_add_peer_object(struct wlan_objmgr_vdev *vdev,
			       enum tQDF_ADAPTER_MODE adapter_mode,
			       uint8_t *mac_addr)
{
	enum wlan_peer_type peer_type;
	struct wlan_objmgr_peer *peer;

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
		return -EINVAL;
	}

	if (!vdev) {
		hdd_err("vdev NULL");
		QDF_ASSERT(0);
		return -EFAULT;
	}

	peer = wlan_objmgr_peer_obj_create(vdev, peer_type, mac_addr);
	if (!peer)
		return -ENOMEM;

	hdd_info("Peer object "MAC_ADDRESS_STR" add success!",
		 MAC_ADDR_ARRAY(mac_addr));

	return 0;
}

int hdd_objmgr_remove_peer_object(struct wlan_objmgr_vdev *vdev,
				  uint8_t *mac_addr)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_peer *peer;

	if (!vdev) {
		hdd_err("vdev NULL");
		QDF_ASSERT(0);
		return -EINVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		hdd_err("Psoc NUll");
		QDF_ASSERT(0);
		return -EINVAL;
	}

	peer = wlan_objmgr_get_peer(psoc, mac_addr, WLAN_HDD_ID_OBJ_MGR);
	if (peer) {
		/* Unref to decrement ref happened in find_peer */
		wlan_objmgr_peer_release_ref(peer, WLAN_HDD_ID_OBJ_MGR);

		wlan_objmgr_peer_obj_delete(peer);

		hdd_info("Peer obj "MAC_ADDRESS_STR" deleted",
				MAC_ADDR_ARRAY(mac_addr));
		return 0;
	}

	hdd_err("Peer obj "MAC_ADDRESS_STR" not found",
				MAC_ADDR_ARRAY(mac_addr));

	return -EINVAL;
}

int hdd_objmgr_set_peer_mlme_state(struct wlan_objmgr_vdev *vdev,
				   enum wlan_peer_state peer_state)
{
	struct wlan_objmgr_peer *peer;

	wlan_vdev_obj_lock(vdev);
	peer = wlan_vdev_get_bsspeer(vdev);
	wlan_vdev_obj_unlock(vdev);

	if (!peer) {
		hdd_err("peer is null");

		return -EINVAL;
	}

	wlan_peer_obj_lock(peer);
	wlan_peer_mlme_set_state(peer, WLAN_ASSOC_STATE);
	wlan_peer_obj_unlock(peer);

	return 0;
}

