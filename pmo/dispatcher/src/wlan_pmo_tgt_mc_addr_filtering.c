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
#include "wlan_pmo_mc_addr_filtering_public_struct.h"
#include "wlan_pmo_obj_mgmt_public_struct.h"

QDF_STATUS pmo_tgt_set_mc_filter_req(struct wlan_objmgr_vdev *vdev,
		struct qdf_mac_addr multicast_addr)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_pmo_tx_ops pmo_tx_ops;

	PMO_ENTER();
	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		pmo_err("Failed to find psoc from from vdev:%p",
			vdev);
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	pmo_tx_ops = GET_PMO_TX_OPS_FROM_PSOC(psoc);
	if (!pmo_tx_ops.send_set_mc_filter_req) {
		pmo_err("send_add_clear_mcbc_filter_request is null");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = pmo_tx_ops.send_set_mc_filter_req(
			vdev, multicast_addr);
	if (status != QDF_STATUS_SUCCESS)
		pmo_err("Failed to add/clear mc filter");
out:
	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_tgt_clear_mc_filter_req(struct wlan_objmgr_vdev *vdev,
		struct qdf_mac_addr multicast_addr)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_pmo_tx_ops pmo_tx_ops;

	PMO_ENTER();
	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		pmo_err("Failed to find psoc from from vdev:%p",
			vdev);
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	pmo_tx_ops = GET_PMO_TX_OPS_FROM_PSOC(psoc);
	if (!pmo_tx_ops.send_clear_mc_filter_req) {
		pmo_err("send_add_clear_mcbc_filter_request is null");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = pmo_tx_ops.send_clear_mc_filter_req(
			vdev, multicast_addr);
	if (status != QDF_STATUS_SUCCESS)
		pmo_err("Failed to add/clear mc filter");
out:
	PMO_EXIT();

	return status;
}


