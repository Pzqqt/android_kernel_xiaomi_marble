/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
#include <qdf_module.h>
#include <wlan_vdev_mgr_tgt_if_rx_defs.h>
#include <wlan_vdev_mgr_tgt_if_rx_api.h>
#include <include/wlan_vdev_mlme.h>
#include <wlan_mlme_dbg.h>
#include <wlan_vdev_mlme_api.h>
#include <target_if_vdev_mgr_tx_ops.h>
#include <wlan_psoc_mlme_main.h>
#include <include/wlan_psoc_mlme.h>
#include <include/wlan_mlme_cmn.h>
#include <wlan_vdev_mgr_utils_api.h>

struct vdev_response_timer *
tgt_vdev_mgr_get_response_timer_info(struct wlan_objmgr_psoc *psoc,
				     uint8_t vdev_id)
{
	struct psoc_mlme_obj *psoc_mlme;

	if (vdev_id >= WLAN_UMAC_PSOC_MAX_VDEVS) {
		mlme_err("Incorrect vdev_id: %d", vdev_id);
		return NULL;
	}

	psoc_mlme = mlme_psoc_get_priv(psoc);
	if (!psoc_mlme) {
		mlme_err("VDEV_%d PSOC_%d PSOC_MLME is NULL", vdev_id,
			 wlan_psoc_get_id(psoc));
		return NULL;
	}

	return &psoc_mlme->psoc_vdev_rt[vdev_id];
}

qdf_export_symbol(tgt_vdev_mgr_get_response_timer_info);

static QDF_STATUS tgt_vdev_mgr_start_response_handler(
					struct wlan_objmgr_psoc *psoc,
					struct vdev_start_response *rsp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_objmgr_vdev *vdev;

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
		mlme_err("VDEV_%d PSOC_%d VDEV_MLME is NULL", rsp->vdev_id,
			 wlan_psoc_get_id(psoc));
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
		mlme_err("VDEV_%d: PSOC_%d VDEV_MLME is NULL", rsp->vdev_id,
			 wlan_psoc_get_id(psoc));
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

static QDF_STATUS tgt_vdev_mgr_delete_response_handler(
					struct wlan_objmgr_psoc *psoc,
					struct vdev_delete_response *rsp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	status = mlme_vdev_ops_ext_hdl_delete_rsp(psoc, rsp);
	return status;
}

static QDF_STATUS tgt_vdev_mgr_peer_delete_all_response_handler(
					struct wlan_objmgr_psoc *psoc,
					struct peer_delete_all_response *rsp)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_objmgr_vdev *vdev;

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
		mlme_err("VDEV_%d: PSOC_%d VDEV_MLME is NULL", rsp->vdev_id,
			 wlan_psoc_get_id(psoc));
		goto tgt_vdev_mgr_peer_delete_all_response_handler_end;
	}

	if ((vdev_mlme->ops) &&
	    vdev_mlme->ops->mlme_vdev_ext_peer_delete_all_rsp)
		status = vdev_mlme->ops->mlme_vdev_ext_peer_delete_all_rsp(
								vdev_mlme,
								rsp);

tgt_vdev_mgr_peer_delete_all_response_handler_end:
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

QDF_STATUS
tgt_vdev_mgr_ext_tbttoffset_update_handle(uint32_t num_vdevs, bool is_ext)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	return status;
}

static QDF_STATUS tgt_vdev_mgr_multi_vdev_restart_resp_handler(
					struct wlan_objmgr_psoc *psoc,
					struct multi_vdev_restart_resp *resp)
{
	return mlme_vdev_ops_ext_hdl_multivdev_restart_resp(psoc, resp);
}

#ifdef FEATURE_VDEV_OPS_WAKELOCK
static struct psoc_mlme_wakelock *
tgt_psoc_get_wakelock_info(struct wlan_objmgr_psoc *psoc)
{
	struct psoc_mlme_obj *psoc_mlme;

	psoc_mlme = mlme_psoc_get_priv(psoc);
	if (!psoc_mlme) {
		mlme_err("PSOC_MLME is NULL");
		return NULL;
	}

	return &psoc_mlme->psoc_mlme_wakelock;
}

static inline void
tgt_psoc_reg_wakelock_info_rx_op(struct wlan_lmac_if_mlme_rx_ops
				     *mlme_rx_ops)
{
	mlme_rx_ops->psoc_get_wakelock_info = tgt_psoc_get_wakelock_info;
}
#else
static inline void
tgt_psoc_reg_wakelock_info_rx_op(struct wlan_lmac_if_mlme_rx_ops
				     *mlme_rx_ops)
{
}
#endif

#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
static inline void tgt_vdev_mgr_reg_set_mac_address_response(
				struct wlan_lmac_if_mlme_rx_ops *mlme_rx_ops)
{
	mlme_rx_ops->vdev_mgr_set_mac_addr_response =
				mlme_vdev_mgr_notify_set_mac_addr_response;
}
#else
static inline void tgt_vdev_mgr_reg_set_mac_address_response(
				struct wlan_lmac_if_mlme_rx_ops *mlme_rx_ops)
{
}
#endif

static void tgt_vdev_mgr_set_max_channel_switch_time(
		struct wlan_objmgr_psoc *psoc, uint32_t *vdev_ids,
		uint32_t num_vdevs)
{
	struct wlan_objmgr_vdev *vdev = NULL;
	struct vdev_mlme_obj *vdev_mlme = NULL;
	unsigned long current_time = qdf_mc_timer_get_system_time();
	uint32_t max_chan_switch_time = 0;
	int i = 0;
	QDF_STATUS status;

	/* Compute and populate the max channel switch time and time of the last
	 * beacon sent on the CSA triggered channel for all the vdevs.
	 */
	for (i = 0; i < num_vdevs; i++) {
		vdev = wlan_objmgr_get_vdev_by_id_from_psoc
		    (psoc, vdev_ids[i], WLAN_VDEV_TARGET_IF_ID);
		if (!vdev) {
			mlme_err("VDEV is NULL");
			continue;
		}

		vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
		if (!vdev_mlme) {
			mlme_err("VDEV_%d: PSOC_%d VDEV_MLME is NULL",
				 vdev_ids[i],
				 wlan_psoc_get_id(psoc));
			wlan_objmgr_vdev_release_ref(vdev,
						     WLAN_VDEV_TARGET_IF_ID);
			continue;
		}

		status = wlan_util_vdev_mgr_compute_max_channel_switch_time
			(vdev, &max_chan_switch_time);
		if (QDF_IS_STATUS_ERROR(status)) {
			mlme_err("Failed to get the max channel switch time value");
			wlan_objmgr_vdev_release_ref(vdev,
						     WLAN_VDEV_TARGET_IF_ID);
			continue;
		}

		vdev_mlme->mgmt.ap.last_bcn_ts_ms = current_time;
		vdev_mlme->mgmt.ap.max_chan_switch_time = max_chan_switch_time;

		wlan_objmgr_vdev_release_ref(vdev, WLAN_VDEV_TARGET_IF_ID);
	}
}

#ifdef WLAN_FEATURE_11BE_MLO
static QDF_STATUS
tgt_vdev_mgr_quiet_offload_handler(struct wlan_objmgr_psoc *psoc,
				   struct vdev_sta_quiet_event *quiet_event)
{
	return wlan_util_vdev_mgr_quiet_offload(psoc, quiet_event);
}

static inline void tgt_vdev_mgr_reg_quiet_offload(
				struct wlan_lmac_if_mlme_rx_ops *mlme_rx_ops)
{
	mlme_rx_ops->vdev_mgr_quiet_offload =
				tgt_vdev_mgr_quiet_offload_handler;
}
#else
static inline void tgt_vdev_mgr_reg_quiet_offload(
				struct wlan_lmac_if_mlme_rx_ops *mlme_rx_ops)
{
}
#endif

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
	mlme_rx_ops->vdev_mgr_peer_delete_all_response =
		tgt_vdev_mgr_peer_delete_all_response_handler;
	mlme_rx_ops->psoc_get_vdev_response_timer_info =
		tgt_vdev_mgr_get_response_timer_info;
	mlme_rx_ops->vdev_mgr_multi_vdev_restart_resp =
		tgt_vdev_mgr_multi_vdev_restart_resp_handler;
	mlme_rx_ops->vdev_mgr_set_max_channel_switch_time =
		tgt_vdev_mgr_set_max_channel_switch_time;
	tgt_psoc_reg_wakelock_info_rx_op(&rx_ops->mops);
	tgt_vdev_mgr_reg_set_mac_address_response(mlme_rx_ops);
	tgt_vdev_mgr_reg_quiet_offload(mlme_rx_ops);
}
