/*
* Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: Implements public API for pmo to interact with target/WMI
 */

#include "wlan_pmo_tgt_api.h"
#include "wlan_pmo_non_arp_hwbcast_public_struct.h"
#include "wlan_pmo_obj_mgmt_public_struct.h"

QDF_STATUS pmo_tgt_enable_non_arp_bcast_filter_req(
		struct wlan_objmgr_vdev *vdev,
		uint8_t vdev_id)
{
	struct pmo_bcast_filter_params *bcast_req = NULL;
	struct pmo_vdev_priv_obj *vdev_ctx;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;
	struct wlan_lmac_if_pmo_tx_ops pmo_tx_ops;
	struct qdf_mac_addr peer_bssid;

	PMO_ENTER();
	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx) {
		pmo_err("vdev_ctx is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		pmo_err("psoc unavailable for vdev %p", vdev);
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	bcast_req = qdf_mem_malloc(sizeof(*bcast_req));
	if (!bcast_req) {
		pmo_err("unable to allocate arp_offload_req");
		status = QDF_STATUS_E_NOMEM;
		goto out;
	}

	status = pmo_get_vdev_bss_peer_mac_addr(vdev,
			&peer_bssid);
	if (status != QDF_STATUS_SUCCESS)
		goto out;

	qdf_mem_copy(&bcast_req->bssid.bytes, &peer_bssid.bytes,
			QDF_MAC_ADDR_SIZE);
	pmo_info("vdev self mac addr: %pM bss peer mac addr: %pM",
		wlan_vdev_mlme_get_macaddr(vdev),
		peer_bssid.bytes);

	bcast_req->enable = true;

	pmo_debug("Non ARP Broadcast filter vdev_id: %d enable: %d",
		vdev_id,
		bcast_req->enable);

	pmo_tx_ops = GET_PMO_TX_OPS_FROM_PSOC(psoc);
	if (!pmo_tx_ops.send_non_arp_bcast_filter_req) {
		pmo_err("send_non_arp_bcast_filter_req is null");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = pmo_tx_ops.send_non_arp_bcast_filter_req(
			vdev, bcast_req);

out:
	if (bcast_req)
		qdf_mem_free(bcast_req);
	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_tgt_disable_non_arp_bcast_filter_req(
		struct wlan_objmgr_vdev *vdev,
		uint8_t vdev_id)
{
	struct pmo_bcast_filter_params *bcast_req = NULL;
	struct pmo_vdev_priv_obj *vdev_ctx;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;
	struct wlan_lmac_if_pmo_tx_ops pmo_tx_ops;
	struct qdf_mac_addr peer_bssid;

	PMO_ENTER();
	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx) {
		pmo_err("vdev_ctx is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		pmo_err("psoc unavailable for vdev %p", vdev);
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	bcast_req = qdf_mem_malloc(sizeof(*bcast_req));
	if (!bcast_req) {
		pmo_err("unable to allocate arp_offload_req");
		status = QDF_STATUS_E_NOMEM;
		goto out;
	}

	status = pmo_get_vdev_bss_peer_mac_addr(vdev,
			&peer_bssid);
	if (status != QDF_STATUS_SUCCESS)
		goto out;

	qdf_mem_copy(&bcast_req->bssid.bytes, &peer_bssid.bytes,
			QDF_MAC_ADDR_SIZE);
	pmo_info("vdev self mac addr: %pM bss peer mac addr: %pM",
		wlan_vdev_mlme_get_macaddr(vdev),
		peer_bssid.bytes);

	bcast_req->enable = false;

	pmo_debug("Non ARP Broadcast filter vdev_id: %d enable: %d",
		vdev_id,
		bcast_req->enable);

	pmo_tx_ops = GET_PMO_TX_OPS_FROM_PSOC(psoc);
	if (!pmo_tx_ops.send_non_arp_bcast_filter_req) {
		pmo_err("send_non_arp_bcast_filter_req is null");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = pmo_tx_ops.send_non_arp_bcast_filter_req(
			vdev, bcast_req);

out:
	if (bcast_req)
		qdf_mem_free(bcast_req);
	PMO_EXIT();

	return status;
}
