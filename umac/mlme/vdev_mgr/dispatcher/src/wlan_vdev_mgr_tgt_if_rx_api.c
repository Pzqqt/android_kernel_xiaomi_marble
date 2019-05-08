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
 * DOC: wlan_vdev_mgr_tgt_if_rx_api.c
 *
 * This file provide definition for APIs registered for LMAC MLME Rx Ops
 */
#include <qdf_types.h>
#include <wlan_vdev_mgr_tgt_if_rx_defs.h>
#include <wlan_vdev_mgr_tgt_if_rx_api.h>
#include <include/wlan_vdev_mlme.h>
#include <wlan_mlme_dbg.h>
#include <wlan_vdev_mlme_api.h>
#include <target_if_vdev_mgr_tx_ops.h>

static struct vdev_response_timer *
tgt_vdev_mgr_get_response_timer_info(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("VDEV_%d: VDEV_MLME is NULL", wlan_vdev_get_id(vdev));
		return NULL;
	}

	return &vdev_mlme->vdev_rt;
}

static QDF_STATUS tgt_vdev_mgr_start_response_handler(
					struct wlan_objmgr_psoc *psoc,
					struct vdev_start_response *rsp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_objmgr_vdev *vdev;
	struct vdev_response_timer *vdev_rsp;
	struct wlan_lmac_if_mlme_tx_ops *tx_ops;

	if (!rsp || !psoc) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, rsp->vdev_id,
						    WLAN_VDEV_TARGET_IF_ID);
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("VDEV_%d: VDEV_MLME is NULL", rsp->vdev_id);
		goto tgt_vdev_mgr_start_response_handler_end;
	}

	vdev_rsp = &vdev_mlme->vdev_rt;
	if (!vdev_rsp) {
		mlme_err("VDEV_%d: Invalid response", rsp->vdev_id);
		goto tgt_vdev_mgr_start_response_handler_end;
	}

	tx_ops = target_if_vdev_mgr_get_tx_ops(psoc);
	if (rsp->resp_type == RESTART_RESPONSE)
		status = tx_ops->vdev_mgr_rsp_timer_stop(vdev, vdev_rsp,
							 RESTART_RESPONSE_BIT);
	else
		status = tx_ops->vdev_mgr_rsp_timer_stop(vdev, vdev_rsp,
							 START_RESPONSE_BIT);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("VDEV_%d: Unexpected response", rsp->vdev_id);
		goto tgt_vdev_mgr_start_response_handler_end;
	}

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_ext_start_rsp)
		status = vdev_mlme->ops->mlme_vdev_ext_start_rsp(
								vdev_mlme,
								rsp);
tgt_vdev_mgr_start_response_handler_end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_VDEV_TARGET_IF_ID);
	return status;
}

static QDF_STATUS tgt_vdev_mgr_stop_response_handler(
					struct wlan_objmgr_psoc *psoc,
					struct vdev_stop_response *rsp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_objmgr_vdev *vdev;
	struct vdev_response_timer *vdev_rsp;
	struct wlan_lmac_if_mlme_tx_ops *tx_ops;

	if (!rsp || !psoc) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, rsp->vdev_id,
						    WLAN_VDEV_TARGET_IF_ID);
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("VDEV_%d: VDEV_MLME is NULL", rsp->vdev_id);
		goto tgt_vdev_mgr_stop_response_handler_end;
	}

	vdev_rsp = &vdev_mlme->vdev_rt;
	if (!vdev_rsp) {
		mlme_err("VDEV_%d: Invalid response", rsp->vdev_id);
		goto tgt_vdev_mgr_stop_response_handler_end;
	}

	tx_ops = target_if_vdev_mgr_get_tx_ops(psoc);
	status = tx_ops->vdev_mgr_rsp_timer_stop(vdev, vdev_rsp,
						 STOP_RESPONSE_BIT);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("VDEV_%d: Unexpected response", rsp->vdev_id);
		goto tgt_vdev_mgr_stop_response_handler_end;
	}

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_ext_stop_rsp)
		status = vdev_mlme->ops->mlme_vdev_ext_stop_rsp(
								vdev_mlme,
								rsp);
tgt_vdev_mgr_stop_response_handler_end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_VDEV_TARGET_IF_ID);
	return status;
}

QDF_STATUS tgt_vdev_mgr_delete_response_handler(
					struct wlan_objmgr_psoc *psoc,
					struct vdev_delete_response *rsp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_objmgr_vdev *vdev;
	struct vdev_response_timer *vdev_rsp;
	struct wlan_lmac_if_mlme_tx_ops *tx_ops;

	if (!rsp || !psoc) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						    rsp->vdev_id,
						    WLAN_VDEV_TARGET_IF_ID);
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("VDEV_%d: VDEV_MLME is NULL", rsp->vdev_id);
		goto tgt_vdev_mgr_delete_response_handler_end;
	}

	vdev_rsp = &vdev_mlme->vdev_rt;
	if (!vdev_rsp) {
		mlme_err("VDEV_%d: Invalid response", rsp->vdev_id);
		goto tgt_vdev_mgr_delete_response_handler_end;
	}

	tx_ops = target_if_vdev_mgr_get_tx_ops(psoc);
	status = tx_ops->vdev_mgr_rsp_timer_stop(vdev, vdev_rsp,
						 DELETE_RESPONSE_BIT);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("VDEV_%d: Unexpected response", rsp->vdev_id);
		goto tgt_vdev_mgr_delete_response_handler_end;
	}

	if ((vdev_mlme->ops) &&
	    vdev_mlme->ops->mlme_vdev_ext_delete_rsp)
		status = vdev_mlme->ops->mlme_vdev_ext_delete_rsp(
								vdev_mlme,
								rsp);

tgt_vdev_mgr_delete_response_handler_end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_VDEV_TARGET_IF_ID);
	return status;
}

static QDF_STATUS
tgt_vdev_mgr_offload_bcn_tx_status_event_handler(uint32_t vdev_id,
						 uint32_t tx_status)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	return status;
}

static QDF_STATUS
tgt_vdev_mgr_tbttoffset_update_handler(uint32_t num_vdevs, bool is_ext)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	return status;
}

static QDF_STATUS
tgt_vdev_mgr_ext_tbttoffset_update_handle(uint32_t num_vdevs, bool is_ext)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	return status;
}

void tgt_vdev_mgr_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	struct wlan_lmac_if_mlme_rx_ops *mlme_rx_ops = &rx_ops->mops;

	mlme_rx_ops->vdev_mgr_offload_bcn_tx_status_event_handle =
		tgt_vdev_mgr_offload_bcn_tx_status_event_handler;
	mlme_rx_ops->vdev_mgr_tbttoffset_update_handle =
		tgt_vdev_mgr_tbttoffset_update_handler;
	mlme_rx_ops->vdev_mgr_start_response =
		tgt_vdev_mgr_start_response_handler;
	mlme_rx_ops->vdev_mgr_stop_response =
		tgt_vdev_mgr_stop_response_handler;
	mlme_rx_ops->vdev_mgr_delete_response =
		tgt_vdev_mgr_delete_response_handler;
	mlme_rx_ops->vdev_mgr_get_response_timer_info =
		tgt_vdev_mgr_get_response_timer_info;
}
