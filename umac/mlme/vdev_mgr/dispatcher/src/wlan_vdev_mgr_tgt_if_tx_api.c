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
 * DOC: wlan_vdev_mgr_tgt_if_tx_api.c
 *
 * This file provides definitions for mlme tgt_if APIs, which will
 * further call target_if/mlme component using LMAC MLME txops
 */
#include <wlan_vdev_mgr_tgt_if_tx_api.h>
#include <target_if_vdev_mgr_tx_ops.h>
#include "include/wlan_vdev_mlme.h"
#include <wlan_mlme_dbg.h>
#include <cdp_txrx_cmn_struct.h>
#include <cdp_txrx_cmn.h>
#include <wlan_lmac_if_api.h>
#include <wlan_utility.h>
#include <cdp_txrx_ctrl.h>
#include <wlan_vdev_mlme_api.h>
#include <wlan_dfs_utils_api.h>
#include <wlan_vdev_mgr_utils_api.h>
#include <wlan_vdev_mgr_ucfg_api.h>
#include <wlan_vdev_mlme_main.h>

static inline struct wlan_lmac_if_mlme_tx_ops
*wlan_vdev_mlme_get_lmac_txops(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);

	return target_if_vdev_mgr_get_tx_ops(psoc);
}

QDF_STATUS tgt_vdev_mgr_create_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_create_params *param)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	ol_txrx_soc_handle soc_txrx_handle;
	enum wlan_op_mode cdp_txrx_opmode;
	enum wlan_op_subtype cdp_txrx_subtype;
	uint32_t vdev_id;
	uint8_t *vdev_addr;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_create_send) {
		mlme_err("VDEV_%d No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mlme_err("psoc object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = txops->vdev_create_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("VDEV_%d PSOC_%d Tx Ops Error : %d", vdev_id,
			 wlan_psoc_get_id(psoc), status);
		return status;
	}

	cdp_txrx_opmode = wlan_util_vdev_get_cdp_txrx_opmode(vdev);
	cdp_txrx_subtype = wlan_util_vdev_get_cdp_txrx_subtype(vdev);
	vdev_addr = wlan_vdev_mlme_get_macaddr(vdev);
	pdev = wlan_vdev_get_pdev(vdev);
	soc_txrx_handle = wlan_psoc_get_dp_handle(psoc);
	if (!soc_txrx_handle)
		return QDF_STATUS_E_FAILURE;

	return cdp_vdev_attach(soc_txrx_handle,
			       wlan_objmgr_pdev_get_pdev_id(pdev),
			       vdev_addr, vdev_id,
			       cdp_txrx_opmode,
			       cdp_txrx_subtype);
}

QDF_STATUS tgt_vdev_mgr_create_complete(struct vdev_mlme_obj *vdev_mlme)
{
	struct wlan_objmgr_vdev *vdev;
	struct vdev_set_params param = {0};
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct vdev_mlme_inactivity_params *inactivity;
	uint8_t vdev_id;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	vdev = vdev_mlme->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_set_param_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	inactivity = &vdev_mlme->mgmt.inactivity_params;

	param.vdev_id = vdev_id;

	param.param_value =
		inactivity->keepalive_min_idle_inactive_time_secs;
	param.param_id = WLAN_MLME_CFG_MIN_IDLE_INACTIVE_TIME;
	status = txops->vdev_set_param_send(vdev, &param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Failed to set min idle inactive time!",
			 vdev_id);

	param.param_value =
		inactivity->keepalive_max_idle_inactive_time_secs;
	param.param_id = WLAN_MLME_CFG_MAX_IDLE_INACTIVE_TIME;
	status = txops->vdev_set_param_send(vdev, &param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Failed to set max idle inactive time!",
			 vdev_id);

	param.param_value =
		inactivity->keepalive_max_unresponsive_time_secs;
	param.param_id = WLAN_MLME_CFG_MAX_UNRESPONSIVE_INACTIVE_TIME;
	status = txops->vdev_set_param_send(vdev, &param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Failed to set max unresponse inactive time!",
			 vdev_id);

	return status;
}

QDF_STATUS tgt_vdev_mgr_start_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_start_params *param)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_start_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = txops->vdev_start_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return status;
}

QDF_STATUS tgt_vdev_mgr_delete_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_delete_params *param)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;
	ol_txrx_soc_handle soc_txrx_handle;
	uint8_t vdev_id;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_delete_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	soc_txrx_handle = wlan_psoc_get_dp_handle(psoc);
	if (soc_txrx_handle)
		cdp_vdev_detach(soc_txrx_handle, wlan_vdev_get_id(vdev),
				NULL, NULL);

	status = txops->vdev_delete_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return status;
}

QDF_STATUS tgt_vdev_mgr_peer_flush_tids_send(
				struct vdev_mlme_obj *mlme_obj,
				struct peer_flush_params *param)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->peer_flush_tids_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = txops->peer_flush_tids_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_vdev_mgr_stop_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_stop_params *param)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_stop_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = txops->vdev_stop_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return status;
}

QDF_STATUS tgt_vdev_mgr_beacon_stop(struct vdev_mlme_obj *mlme_obj)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_vdev_mgr_beacon_free(struct vdev_mlme_obj *mlme_obj)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_vdev_mgr_up_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_up_params *param)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	ol_txrx_soc_handle soc_txrx_handle;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_up_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	/* cdp set rx and tx decap type */
	psoc = wlan_vdev_get_psoc(vdev);
	soc_txrx_handle = wlan_psoc_get_dp_handle(psoc);
	if (!soc_txrx_handle || vdev_id == WLAN_INVALID_VDEV_ID)
		return QDF_STATUS_E_INVAL;

	status = txops->vdev_up_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return status;
}

QDF_STATUS tgt_vdev_mgr_down_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_down_params *param)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	enum QDF_OPMODE opmode;
	uint8_t vdev_id;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_down_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("PDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (wlan_util_is_vdev_active(pdev, WLAN_VDEV_TARGET_IF_ID) ==
						QDF_STATUS_SUCCESS) {

		if (opmode == QDF_SAP_MODE)
			utils_dfs_cancel_precac_timer(pdev);
	}

	status = txops->vdev_down_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return status;
}

QDF_STATUS tgt_vdev_mgr_set_neighbour_rx_cmd_send(
				struct vdev_mlme_obj *mlme_obj,
				struct set_neighbour_rx_params *param)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_vdev_mgr_nac_rssi_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_scan_nac_rssi_params *param)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_vdev_mgr_sifs_trigger_send(
				struct vdev_mlme_obj *mlme_obj,
				struct sifs_trigger_param *param)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_sifs_trigger_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = txops->vdev_sifs_trigger_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return status;
}

QDF_STATUS tgt_vdev_mgr_set_custom_aggr_size_send(
				struct vdev_mlme_obj *mlme_obj,
				struct set_custom_aggr_size_params *param)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_set_custom_aggr_size_cmd_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = txops->vdev_set_custom_aggr_size_cmd_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return status;
}

QDF_STATUS tgt_vdev_mgr_config_ratemask_cmd_send(
				struct vdev_mlme_obj *mlme_obj,
				struct config_ratemask_params *param)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_config_ratemask_cmd_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = txops->vdev_config_ratemask_cmd_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return status;
}

QDF_STATUS tgt_vdev_mgr_beacon_cmd_send(
				struct vdev_mlme_obj *mlme_obj,
				struct beacon_params *param)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_vdev_mgr_beacon_tmpl_send(
				struct vdev_mlme_obj *mlme_obj,
				struct beacon_tmpl_params *param)
{
	return QDF_STATUS_SUCCESS;
}

#if defined(WLAN_SUPPORT_FILS) || defined(CONFIG_BAND_6GHZ)
QDF_STATUS tgt_vdev_mgr_fils_enable_send(
				struct vdev_mlme_obj *mlme_obj,
				struct config_fils_params *param)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_fils_enable_send) {
		mlme_err("VDEV_%d: No Tx Ops fils Enable", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = txops->vdev_fils_enable_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops fils Enable Error : %d",
			 vdev_id, status);

	return status;
}
#endif

QDF_STATUS tgt_vdev_mgr_multiple_vdev_restart_send(
				struct wlan_objmgr_pdev *pdev,
				struct multiple_vdev_restart_params *param)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev,
						    param->vdev_ids[0],
						    WLAN_VDEV_TARGET_IF_ID);
	if (vdev) {
		txops = wlan_vdev_mlme_get_lmac_txops(vdev);
		if (!txops || !txops->multiple_vdev_restart_req_cmd) {
			mlme_err("VDEV_%d: No Tx Ops", wlan_vdev_get_id(vdev));
			wlan_objmgr_vdev_release_ref(vdev,
						     WLAN_VDEV_TARGET_IF_ID);
			return QDF_STATUS_E_INVAL;
		}

		status = txops->multiple_vdev_restart_req_cmd(pdev, param);
		if (QDF_IS_STATUS_ERROR(status))
			mlme_err("Tx Ops Error: %d", status);

		wlan_objmgr_vdev_release_ref(vdev, WLAN_VDEV_TARGET_IF_ID);
	}

	return status;
}

QDF_STATUS tgt_vdev_mgr_set_tx_rx_decap_type(struct vdev_mlme_obj *mlme_obj,
					     enum wlan_mlme_cfg_id param_id,
					     uint32_t value)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;

	if (!mlme_obj) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_set_tx_rx_decap_type) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = txops->vdev_set_tx_rx_decap_type(vdev, param_id, value);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return status;
}

QDF_STATUS tgt_vdev_mgr_set_param_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_set_params *param)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_set_param_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = txops->vdev_set_param_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return status;
}

QDF_STATUS tgt_vdev_mgr_sta_ps_param_send(
				struct vdev_mlme_obj *mlme_obj,
				struct sta_ps_params *param)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->vdev_sta_ps_param_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = txops->vdev_sta_ps_param_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return status;
}

QDF_STATUS tgt_vdev_mgr_peer_delete_all_send(
				struct vdev_mlme_obj *mlme_obj,
				struct peer_delete_all_params *param)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *txops;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_id;

	if (!param) {
		mlme_err("Invalid input");
		return QDF_STATUS_E_INVAL;
	}

	vdev = mlme_obj->vdev;
	vdev_id = wlan_vdev_get_id(vdev);
	txops = wlan_vdev_mlme_get_lmac_txops(vdev);
	if (!txops || !txops->peer_delete_all_send) {
		mlme_err("VDEV_%d: No Tx Ops", vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = txops->peer_delete_all_send(vdev, param);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("VDEV_%d: Tx Ops Error : %d", vdev_id, status);

	return QDF_STATUS_SUCCESS;
}
