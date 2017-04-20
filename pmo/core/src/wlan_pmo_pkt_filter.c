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
 * DOC: Implements Packet filter feature API's
 */

#include "wlan_pmo_pkt_filter.h"
#include "wlan_pmo_tgt_api.h"
#include "wlan_pmo_main.h"
#include "wlan_pmo_obj_mgmt_public_struct.h"

QDF_STATUS pmo_core_set_pkt_filter(struct wlan_objmgr_psoc *psoc,
			struct pmo_rcv_pkt_fltr_cfg *pmo_set_pkt_fltr_req,
			uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status;

	PMO_ENTER();

	if (!psoc) {
		pmo_err("psoc is null");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id, WLAN_PMO_ID);
	if (!vdev) {
		pmo_err("vdev is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = pmo_tgt_set_pkt_filter(vdev, pmo_set_pkt_fltr_req, vdev_id);
	if (status != QDF_STATUS_SUCCESS)
		goto dec_ref;

dec_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_PMO_ID);
out:
	PMO_EXIT();

	return status;

}

QDF_STATUS pmo_core_clear_pkt_filter(struct wlan_objmgr_psoc *psoc,
		struct pmo_rcv_pkt_fltr_clear_param *pmo_clr_pkt_fltr_param,
		uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status;

	PMO_ENTER();

	if (!psoc) {
		pmo_err("psoc is null");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id, WLAN_PMO_ID);
	if (!vdev) {
		pmo_err("vdev is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = pmo_tgt_clear_pkt_filter(vdev, pmo_clr_pkt_fltr_param,
								vdev_id);
	if (status != QDF_STATUS_SUCCESS)
		goto dec_ref;

dec_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_PMO_ID);
out:
	PMO_EXIT();

	return status;

}
