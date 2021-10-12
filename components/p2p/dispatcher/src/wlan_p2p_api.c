/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains P2P public API's exposed.
 */

#include "wlan_p2p_api.h"
#include <wlan_objmgr_psoc_obj.h>
#include "wlan_p2p_public_struct.h"
#include "../../core/src/wlan_p2p_main.h"
#include "../../core/src/wlan_p2p_roc.h"
#include <cds_utils.h>
#include "wlan_scan_api.h"

bool wlan_p2p_check_oui_and_force_1x1(uint8_t *assoc_ie, uint32_t assoc_ie_len)
{
	if (!assoc_ie || !assoc_ie_len)
		return false;

	return p2p_check_oui_and_force_1x1(assoc_ie, assoc_ie_len);
}

QDF_STATUS wlan_p2p_cleanup_roc_by_vdev(struct wlan_objmgr_vdev *vdev)
{
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct wlan_objmgr_psoc *psoc;

	p2p_debug("vdev:%pK", vdev);

	if (!vdev) {
		p2p_debug("null vdev");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		p2p_err("null psoc");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("p2p soc context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return p2p_cleanup_roc_sync(p2p_soc_obj, vdev);
}

QDF_STATUS wlan_p2p_status_connect(struct wlan_objmgr_vdev *vdev)
{
	if (!vdev) {
		p2p_err("vdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	return p2p_status_connect(vdev);
}

static void wlan_p2p_abort_vdev_scan(struct wlan_objmgr_pdev *pdev,
				     void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	struct scan_cancel_request *req;
	struct p2p_soc_priv_obj *p2p_soc_obj;
	QDF_STATUS status;

	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_P2P_DEVICE_MODE)
		return;

	p2p_soc_obj =
		wlan_objmgr_psoc_get_comp_private_obj(wlan_vdev_get_psoc(vdev),
						      WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("P2P soc object is NULL");
		return;
	}

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return;

	req->vdev = vdev;
	req->cancel_req.requester = p2p_soc_obj->scan_req_id;
	req->cancel_req.scan_id = INVALID_SCAN_ID;
	req->cancel_req.vdev_id = wlan_vdev_get_id(vdev);
	req->cancel_req.req_type = WLAN_SCAN_CANCEL_VDEV_ALL;

	qdf_mtrace(QDF_MODULE_ID_P2P, QDF_MODULE_ID_SCAN,
		   req->cancel_req.req_type,
		   req->cancel_req.vdev_id,
		   req->cancel_req.scan_id);
	status = wlan_scan_cancel(req);

	p2p_debug("abort scan, scan req id:%d, scan id:%d, status:%d",
		  req->cancel_req.requester,
		  req->cancel_req.scan_id, status);
}

QDF_STATUS wlan_p2p_abort_scan(struct wlan_objmgr_pdev *pdev)
{
	return wlan_objmgr_pdev_iterate_obj_list(pdev,
						 WLAN_VDEV_OP,
						 wlan_p2p_abort_vdev_scan,
						 NULL, 0, WLAN_P2P_ID);
}
