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

static void tgt_vdev_mgr_response_timeout_handler(
					struct wlan_objmgr_vdev *vdev)
{
}

static struct vdev_response_timer *
tgt_vdev_mgr_get_response_timer_info(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		QDF_ASSERT(0);
		return NULL;
	}

	return &vdev_mlme->vdev_rt;
}

static QDF_STATUS tgt_vdev_mgr_start_response_handle(
					struct wlan_objmgr_psoc *psoc,
					struct vdev_start_response *rsp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_objmgr_vdev *vdev;
	struct vdev_response_timer *vdev_rsp;

	if (!rsp || !psoc) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, rsp->vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		QDF_ASSERT(0);
		goto tgt_vdev_mgr_start_response_handle_end;
	}

	vdev_rsp = &vdev_mlme->vdev_rt;
	if (!vdev_rsp)
		goto tgt_vdev_mgr_start_response_handle_end;

	if (!qdf_atomic_test_and_clear_bit(START_RESPONSE_BIT,
					   &vdev_rsp->rsp_status)) {
		mlme_info("Unexpected response");
		goto tgt_vdev_mgr_start_response_handle_end;
	}

	qdf_timer_stop(&vdev_rsp->rsp_timer);

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_ext_start_rsp)
		status = vdev_mlme->ops->mlme_vdev_ext_start_rsp(
								vdev_mlme,
								rsp);
tgt_vdev_mgr_start_response_handle_end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);
	return status;
}

static QDF_STATUS tgt_vdev_mgr_stop_response_handle(
					struct wlan_objmgr_psoc *psoc,
					struct vdev_stop_response *rsp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_objmgr_vdev *vdev;
	struct vdev_response_timer *vdev_rsp;

	if (!rsp || !psoc) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, rsp->vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		QDF_ASSERT(0);
		goto tgt_vdev_mgr_stop_response_handle_end;
	}

	vdev_rsp = &vdev_mlme->vdev_rt;
	if (!vdev_rsp)
		goto tgt_vdev_mgr_stop_response_handle_end;

	if (!qdf_atomic_test_and_clear_bit(STOP_RESPONSE_BIT,
					   &vdev_rsp->rsp_status)) {
		mlme_info("Unexpected response");
		goto tgt_vdev_mgr_stop_response_handle_end;
	}

	qdf_timer_stop(&vdev_rsp->rsp_timer);

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_ext_stop_rsp)
		status = vdev_mlme->ops->mlme_vdev_ext_stop_rsp(
								vdev_mlme,
								rsp);
tgt_vdev_mgr_stop_response_handle_end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);
	return status;
}

static QDF_STATUS tgt_vdev_mgr_delete_response_handle(
					struct wlan_objmgr_psoc *psoc,
					struct vdev_delete_response *rsp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_objmgr_vdev *vdev;
	struct vdev_response_timer *vdev_rsp;

	if (!rsp || !psoc) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, rsp->vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		QDF_ASSERT(0);
		goto tgt_vdev_mgr_delete_response_handle_end;
	}

	vdev_rsp = &vdev_mlme->vdev_rt;
	if (!vdev_rsp)
		goto tgt_vdev_mgr_delete_response_handle_end;

	if (!qdf_atomic_test_and_clear_bit(DELETE_RESPONSE_BIT,
					   &vdev_rsp->rsp_status)) {
		mlme_info("Unexpected response");
		goto tgt_vdev_mgr_delete_response_handle_end;
	}

	qdf_timer_stop(&vdev_rsp->rsp_timer);

	if ((vdev_mlme->ops) &&
	    vdev_mlme->ops->mlme_vdev_ext_delete_rsp)
		status = vdev_mlme->ops->mlme_vdev_ext_delete_rsp(
								vdev_mlme,
								rsp);

tgt_vdev_mgr_delete_response_handle_end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);
	return status;
}

static QDF_STATUS
tgt_vdev_mgr_offload_bcn_tx_status_event_handle(uint32_t vdev_id,
						uint32_t tx_status)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	return status;
}

static QDF_STATUS
tgt_vdev_mgr_tbttoffset_update_handle(uint32_t num_vdevs, bool is_ext)
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
		tgt_vdev_mgr_offload_bcn_tx_status_event_handle;
	mlme_rx_ops->vdev_mgr_tbttoffset_update_handle =
		tgt_vdev_mgr_tbttoffset_update_handle;
	mlme_rx_ops->vdev_mgr_start_response =
		tgt_vdev_mgr_start_response_handle;
	mlme_rx_ops->vdev_mgr_stop_response =
		tgt_vdev_mgr_stop_response_handle;
	mlme_rx_ops->vdev_mgr_delete_response =
		tgt_vdev_mgr_delete_response_handle;
	mlme_rx_ops->vdev_mgr_get_response_timer_info =
		tgt_vdev_mgr_get_response_timer_info;
	mlme_rx_ops->vdev_mgr_response_timeout_cb =
		tgt_vdev_mgr_response_timeout_handler;
}
