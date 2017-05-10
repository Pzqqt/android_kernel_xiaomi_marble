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
 * DOC: Implements arp offload feature API's
 */

#include "wlan_pmo_hw_bcast_fltr.h"
#include "wlan_pmo_tgt_api.h"
#include "wlan_pmo_main.h"
#include "wlan_pmo_obj_mgmt_public_struct.h"

static QDF_STATUS pmo_core_non_arp_bcast_filter_sanity(
			struct wlan_objmgr_vdev *vdev)
{
	struct pmo_vdev_priv_obj *vdev_ctx;

	if (!vdev) {
		pmo_err("vdev is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx) {
		pmo_err("vdev_ctx is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!vdev_ctx->pmo_psoc_ctx->psoc_cfg.hw_bcast_filter) {
		pmo_err("user disabled hw broadcast filter using ini");
		return QDF_STATUS_E_INVAL;
	}

	if (!pmo_core_is_vdev_connected(vdev))
		return QDF_STATUS_E_INVAL;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS pmo_core_do_enable_non_arp_bcast_filter(
		struct wlan_objmgr_vdev *vdev, uint8_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	PMO_ENTER();

	status = pmo_tgt_enable_non_arp_bcast_filter_req(vdev, vdev_id);

	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_core_do_disable_non_arp_bcast_filter(
			struct wlan_objmgr_vdev *vdev, uint8_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	PMO_ENTER();

	status = pmo_tgt_enable_non_arp_bcast_filter_req(vdev, vdev_id);

	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_core_enable_non_arp_bcast_filter_in_fwr(
				struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS status;
	uint8_t vdev_id;

	PMO_ENTER();
	if (!vdev) {
		pmo_err("vdev is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = wlan_objmgr_vdev_try_get_ref(vdev, WLAN_PMO_ID);
	if (status != QDF_STATUS_SUCCESS)
		goto out;

	status = pmo_core_non_arp_bcast_filter_sanity(vdev);
	if (status != QDF_STATUS_SUCCESS)
		goto def_ref;

	vdev_id = pmo_get_vdev_id(vdev);
	pmo_info("Enable non arp hw bcast filter in fwr vdev id: %d vdev: %p",
		vdev_id, vdev);

	status = pmo_core_do_enable_non_arp_bcast_filter(vdev, vdev_id);
def_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_PMO_ID);
out:
	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_core_disable_non_arp_bcast_filter_in_fwr(
				struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS status;
	uint8_t vdev_id;

	PMO_ENTER();
	if (!vdev) {
		pmo_err("vdev is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = wlan_objmgr_vdev_try_get_ref(vdev, WLAN_PMO_ID);
	if (status != QDF_STATUS_SUCCESS)
		goto out;

	status = pmo_core_non_arp_bcast_filter_sanity(vdev);
	if (status != QDF_STATUS_SUCCESS)
		goto def_ref;

	vdev_id = pmo_get_vdev_id(vdev);
	pmo_info("Disable non arp hw bcast filter in fwr vdev id: %d vdev: %p",
		vdev_id, vdev);

	status = pmo_core_do_disable_non_arp_bcast_filter(vdev, vdev_id);
def_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_PMO_ID);
out:
	PMO_EXIT();

	return status;
}
