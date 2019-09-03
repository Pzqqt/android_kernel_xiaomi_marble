/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: target_if_psoc_timer_tx_ops.c
 *
 * This file provide definition for APIs registered through lmac Tx Ops
 */

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_mlme_dbg.h>
#include <target_if_psoc_timer_tx_ops.h>
#include <wlan_vdev_mgr_tgt_if_rx_defs.h>
#include <target_if_vdev_mgr_tx_ops.h>
#include <target_if_vdev_mgr_rx_ops.h>

QDF_STATUS target_if_psoc_vdev_rsp_timer_inuse(struct wlan_objmgr_psoc *psoc,
					       uint8_t vdev_id)
{
	struct vdev_response_timer *vdev_rsp;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	if (vdev_id >= WLAN_UMAC_PSOC_MAX_VDEVS) {
		mlme_err("Invalid vdev id passed VDEV_%d", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops && !rx_ops->psoc_get_vdev_response_timer_info) {
		mlme_err("VDEV_%d PSOC_%d No Rx Ops", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc, vdev_id);
	if (!vdev_rsp) {
		mlme_err("vdev response is NULL for VDEV_%d PSOC_%d",
			 vdev_id, wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	if (qdf_atomic_read(&vdev_rsp->rsp_timer_inuse)) {
		mlme_err("vdev response timer still inuse VDEV_%d PSOC_%d",
			 vdev_id, wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_ALREADY;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_psoc_vdev_rsp_timer_init(struct wlan_objmgr_psoc *psoc,
					      uint8_t vdev_id)
{
	struct vdev_response_timer *vdev_rsp;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	if (vdev_id >= WLAN_UMAC_PSOC_MAX_VDEVS) {
		mlme_err("Invalid vdev id passed VDEV_%d PSOC_%d", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops && !rx_ops->psoc_get_vdev_response_timer_info) {
		mlme_err("VDEV_%d PSOC_%d No Rx Ops", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc, vdev_id);
	if (!vdev_rsp) {
		mlme_err("vdev response is NULL for VDEV_%d PSOC_%d",
			 vdev_id, wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_INVAL;
	}

	vdev_rsp->psoc = psoc;
	vdev_rsp->vdev_id = vdev_id;
	qdf_timer_init(NULL, &vdev_rsp->rsp_timer,
		       target_if_vdev_mgr_rsp_timer_mgmt_cb,
		       vdev_rsp, QDF_TIMER_TYPE_WAKE_APPS);
	qdf_atomic_init(&vdev_rsp->rsp_timer_inuse);

	return QDF_STATUS_SUCCESS;
}

void target_if_psoc_vdev_rsp_timer_deinit(struct wlan_objmgr_psoc *psoc,
					  uint8_t vdev_id)
{
	struct vdev_response_timer *vdev_rsp;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;

	if (vdev_id >= WLAN_UMAC_PSOC_MAX_VDEVS) {
		mlme_err("Invalid vdev id passed VDEV_%d PSOC_%d", vdev_id,
			 wlan_psoc_get_id(psoc));
		return;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops && !rx_ops->psoc_get_vdev_response_timer_info) {
		mlme_err("VDEV_%d PSOC_%d No Rx Ops", vdev_id,
			 wlan_psoc_get_id(psoc));
		return;
	}

	vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc, vdev_id);
	if (!vdev_rsp) {
		mlme_err("vdev response is NULL for VDEV_%d PSOC_%d",
			 vdev_id, wlan_psoc_get_id(psoc));
		return;
	}

	qdf_timer_free(&vdev_rsp->rsp_timer);
	qdf_atomic_set(&vdev_rsp->rsp_timer_inuse, 0);
	vdev_rsp->psoc = NULL;
}

void target_if_flush_psoc_vdev_timers(struct wlan_objmgr_psoc *psoc)
{
	struct vdev_response_timer *vdev_rsp;
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	int i;

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops && !rx_ops->psoc_get_vdev_response_timer_info) {
		mlme_err("PSOC_%d No Rx Ops", wlan_psoc_get_id(psoc));
		return;
	}

	for (i = 0; i < WLAN_UMAC_PSOC_MAX_VDEVS; i++) {
		vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc,
								     i);
		if (vdev_rsp && qdf_timer_sync_cancel(&vdev_rsp->rsp_timer))
			target_if_vdev_mgr_rsp_timer_cb(vdev_rsp);
	}
}

QDF_STATUS target_if_vdev_mgr_rsp_timer_mod(
					struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id,
					int mseconds)
{
	struct wlan_lmac_if_mlme_rx_ops *rx_ops;
	struct vdev_response_timer *vdev_rsp;

	if (!psoc) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_FAILURE;
	}

	rx_ops = target_if_vdev_mgr_get_rx_ops(psoc);
	if (!rx_ops && !rx_ops->psoc_get_vdev_response_timer_info) {
		mlme_err("VDEV_%d PSOC_%d No Rx Ops", vdev_id,
			 wlan_psoc_get_id(psoc));
		return QDF_STATUS_E_FAILURE;
	}

	vdev_rsp = rx_ops->psoc_get_vdev_response_timer_info(psoc, vdev_id);
	qdf_timer_mod(&vdev_rsp->rsp_timer, mseconds);
	return QDF_STATUS_SUCCESS;
}

