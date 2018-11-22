/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
 *
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
 * DOC: This file init/deint functions for DFS module.
 */

#include "wlan_dfs_ucfg_api.h"
#include "wlan_dfs_tgt_api.h"
#include <wlan_objmgr_vdev_obj.h>
#include "wlan_dfs_utils_api.h"
#ifndef QCA_MCL_DFS_SUPPORT
#include "ieee80211_mlme_dfs_interface.h"
#endif
#include "wlan_objmgr_global_obj.h"
#include "wlan_dfs_init_deinit_api.h"
#include "wlan_dfs_lmac_api.h"
#include "../../core/src/dfs.h"
#include "a_types.h"
#include "wlan_serialization_api.h"
#include <qdf_trace.h>
#include "wlan_scan_ucfg_api.h"
#include "wlan_dfs_mlme_api.h"
#include "../../core/src/dfs_zero_cac.h"

struct dfs_to_mlme global_dfs_to_mlme;

struct wlan_dfs *wlan_pdev_get_dfs_obj(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;
	dfs = wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_DFS);

	return dfs;
}

#ifndef QCA_MCL_DFS_SUPPORT
#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
static inline void
register_dfs_precac_auto_chan_callbacks(struct dfs_to_mlme *mlme_callback)
{
	if (!mlme_callback)
		return;

	mlme_callback->mlme_precac_chan_change_csa =
		mlme_dfs_precac_chan_change_csa;
}
#else
static inline void
register_dfs_precac_auto_chan_callbacks(struct dfs_to_mlme *mlme_callback)
{
}
#endif

void register_dfs_callbacks(void)
{
	struct dfs_to_mlme *tmp_dfs_to_mlme = &global_dfs_to_mlme;

	tmp_dfs_to_mlme->pdev_component_obj_attach =
		wlan_objmgr_pdev_component_obj_attach;
	tmp_dfs_to_mlme->pdev_component_obj_detach =
		wlan_objmgr_pdev_component_obj_detach;

	tmp_dfs_to_mlme->dfs_start_rcsa = mlme_dfs_start_rcsa;
	tmp_dfs_to_mlme->mlme_mark_dfs = mlme_dfs_mark_dfs;
	tmp_dfs_to_mlme->mlme_start_csa = mlme_dfs_start_csa;
	tmp_dfs_to_mlme->mlme_proc_cac = mlme_dfs_proc_cac;
	tmp_dfs_to_mlme->mlme_deliver_event_up_after_cac =
		mlme_dfs_deliver_event_up_after_cac;
	tmp_dfs_to_mlme->mlme_get_dfs_ch_nchans = mlme_dfs_get_dfs_ch_nchans;
	tmp_dfs_to_mlme->mlme_get_extchan = mlme_dfs_get_extchan;
	tmp_dfs_to_mlme->mlme_set_no_chans_available =
		mlme_dfs_set_no_chans_available;
	tmp_dfs_to_mlme->mlme_ieee2mhz = mlme_dfs_ieee2mhz;
	tmp_dfs_to_mlme->mlme_find_dot11_channel = mlme_dfs_find_dot11_channel;
	tmp_dfs_to_mlme->mlme_get_dfs_ch_channels =
		mlme_dfs_get_dfs_ch_channels;
	tmp_dfs_to_mlme->mlme_dfs_ch_flags_ext = mlme_dfs_dfs_ch_flags_ext;
	tmp_dfs_to_mlme->mlme_channel_change_by_precac =
		mlme_dfs_channel_change_by_precac;
	tmp_dfs_to_mlme->mlme_nol_timeout_notification =
		mlme_dfs_nol_timeout_notification;
	tmp_dfs_to_mlme->mlme_clist_update = mlme_dfs_clist_update;
	tmp_dfs_to_mlme->mlme_get_cac_timeout = mlme_dfs_get_cac_timeout;
	tmp_dfs_to_mlme->mlme_rebuild_chan_list_with_non_dfs_channels =
		mlme_dfs_rebuild_chan_list_with_non_dfs_channels;
	tmp_dfs_to_mlme->mlme_restart_vaps_with_non_dfs_chan =
		mlme_dfs_restart_vaps_with_non_dfs_chan;
	tmp_dfs_to_mlme->mlme_is_opmode_sta =
		mlme_dfs_is_opmode_sta;
	tmp_dfs_to_mlme->mlme_check_allowed_prim_chanlist =
		mlme_dfs_check_allowed_prim_chanlist;
	tmp_dfs_to_mlme->mlme_update_scan_channel_list =
		mlme_dfs_update_scan_channel_list;
	tmp_dfs_to_mlme->mlme_bringdown_vaps =
		mlme_dfs_bringdown_vaps;
	tmp_dfs_to_mlme->mlme_dfs_deliver_event =
		mlme_dfs_deliver_event;

	/*
	 * Register precac auto channel switch feature related callbacks
	 */
	register_dfs_precac_auto_chan_callbacks(tmp_dfs_to_mlme);
}
#else
void register_dfs_callbacks(void)
{
	struct dfs_to_mlme *tmp_dfs_to_mlme = &global_dfs_to_mlme;

	tmp_dfs_to_mlme->pdev_component_obj_attach =
		wlan_objmgr_pdev_component_obj_attach;
	tmp_dfs_to_mlme->pdev_component_obj_detach =
		wlan_objmgr_pdev_component_obj_detach;
}
#endif

/**
 * dfs_psoc_obj_create_notification() - dfs psoc create notification handler
 * @psoc: psoc object
 * @arg_list: Argument list
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dfs_psoc_obj_create_notification(struct wlan_objmgr_psoc *psoc,
					     void *arg_list)
{
	QDF_STATUS status;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = qdf_mem_malloc(sizeof(*dfs_soc_obj));
	if (!dfs_soc_obj)
		return QDF_STATUS_E_NOMEM;

	dfs_soc_obj->psoc = psoc;

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_DFS,
						       (void *)dfs_soc_obj,
						       QDF_STATUS_SUCCESS);

	if (QDF_IS_STATUS_ERROR(status)) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to attach psoc dfs component");
		qdf_mem_free(dfs_soc_obj);
		return status;
	}
	/* Initialize precac timer here*/
	dfs_zero_cac_timer_init(dfs_soc_obj);

	dfs_debug(NULL, WLAN_DEBUG_DFS1,
		"DFS obj attach to psoc successfully");

	return status;
}

/**
 * dfs_psoc_obj_destroy_notification() - dfs psoc destroy notification handler
 * @psoc: psoc object
 * @arg_list: Argument list
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dfs_psoc_obj_destroy_notification(struct wlan_objmgr_psoc *psoc,
					      void *arg_list)
{
	QDF_STATUS status;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						WLAN_UMAC_COMP_DFS);
	if (!dfs_soc_obj) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to get dfs obj in psoc");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_zero_cac_timer_detach(dfs_soc_obj);

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						       WLAN_UMAC_COMP_DFS,
						       dfs_soc_obj);

	if (QDF_IS_STATUS_ERROR(status))
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to detach psoc dfs component");

	qdf_mem_free(dfs_soc_obj);

	return status;
}

QDF_STATUS dfs_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(WLAN_UMAC_COMP_DFS,
			dfs_psoc_obj_create_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to register psoc create handler for dfs");
		goto err_psoc_create;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(WLAN_UMAC_COMP_DFS,
			dfs_psoc_obj_destroy_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to register psoc delete handler for dfs");
		goto err_psoc_delete;
	}

	register_dfs_callbacks();

	status = wlan_objmgr_register_pdev_create_handler(WLAN_UMAC_COMP_DFS,
			wlan_dfs_pdev_obj_create_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to register pdev create handler for dfs");
		goto err_pdev_create;
	}

	status = wlan_objmgr_register_pdev_destroy_handler(WLAN_UMAC_COMP_DFS,
			wlan_dfs_pdev_obj_destroy_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status)) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to register pdev delete handler for dfs");
		goto err_pdev_delete;
	}

	status = qdf_print_set_category_verbose(qdf_get_pidx(),
			QDF_MODULE_ID_DFS, QDF_TRACE_LEVEL_INFO, true);

	if (QDF_IS_STATUS_ERROR(status)) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to set verbose for category");
		goto err_category_verbose;
	}

	return QDF_STATUS_SUCCESS;

err_category_verbose:
	wlan_objmgr_unregister_pdev_destroy_handler(WLAN_UMAC_COMP_DFS,
			wlan_dfs_pdev_obj_destroy_notification,
			NULL);
err_pdev_delete:
	wlan_objmgr_unregister_pdev_create_handler(WLAN_UMAC_COMP_DFS,
			wlan_dfs_pdev_obj_create_notification,
			NULL);
err_pdev_create:
	wlan_objmgr_unregister_psoc_destroy_handler(WLAN_UMAC_COMP_DFS,
			dfs_psoc_obj_destroy_notification,
			NULL);
err_psoc_delete:
	wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_DFS,
			dfs_psoc_obj_create_notification,
			NULL);
err_psoc_create:
	return status;
}

QDF_STATUS dfs_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_DFS,
			dfs_psoc_obj_create_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to deregister dfs psoc obj create");

	status = wlan_objmgr_unregister_psoc_destroy_handler(WLAN_UMAC_COMP_DFS,
			dfs_psoc_obj_destroy_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to deregister dfs psoc obj destroy");

	status = wlan_objmgr_unregister_pdev_create_handler(WLAN_UMAC_COMP_DFS,
			wlan_dfs_pdev_obj_create_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to deregister dfs pdev obj create");

	status = wlan_objmgr_unregister_pdev_destroy_handler(WLAN_UMAC_COMP_DFS,
			wlan_dfs_pdev_obj_destroy_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Failed to deregister dfs pdev obj destroy");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_dfs_pdev_obj_create_notification(struct wlan_objmgr_pdev *pdev,
		void *arg)
{
	struct wlan_dfs *dfs = NULL;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint8_t pdev_id;
	QDF_STATUS status;
	bool is_5ghz = false;

	if (!pdev) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);
	if (!(dfs_tx_ops && dfs_tx_ops->dfs_is_pdev_5ghz)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs_tx_ops is null");
		return QDF_STATUS_E_FAILURE;
	}

	status = dfs_tx_ops->dfs_is_pdev_5ghz(pdev, &is_5ghz);
	if (QDF_IS_STATUS_ERROR(status)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "Failed to get is_5ghz value");
		return QDF_STATUS_E_FAILURE;
	}

	if (!is_5ghz) {
		pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"Do not allocate DFS object for 2G, pdev_id = %d",
				pdev_id);
		return QDF_STATUS_SUCCESS;
	}

	if (dfs_create_object(&dfs) == 1) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "failed to create object");
		return QDF_STATUS_E_FAILURE;
	}

	status = global_dfs_to_mlme.pdev_component_obj_attach(pdev,
		WLAN_UMAC_COMP_DFS, (void *)dfs, QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "obj attach failed");
		dfs_destroy_object(dfs);
		return QDF_STATUS_E_FAILURE;
	}

	dfs->dfs_pdev_obj = pdev;

	if (!dfs_tx_ops->dfs_is_tgt_offload) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"dfs_is_tgt_offload is null");
		dfs_destroy_object(dfs);
		return QDF_STATUS_E_FAILURE;
	}

	dfs->dfs_is_offload_enabled = dfs_tx_ops->dfs_is_tgt_offload(psoc);
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs_offload %d",
		 dfs->dfs_is_offload_enabled);
	dfs_agile_soc_obj_init(dfs, psoc);

	if (dfs_attach(dfs) == 1) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs_attch failed");
		dfs_destroy_object(dfs);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_dfs_pdev_obj_destroy_notification(struct wlan_objmgr_pdev *pdev,
		void *arg)
{
	struct wlan_dfs *dfs = NULL;

	if (!pdev) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "PDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	dfs = wlan_pdev_get_dfs_obj(pdev);

	/* DFS is NULL during unload. should we call this function before */
	if (dfs) {
		global_dfs_to_mlme.pdev_component_obj_detach(pdev,
				WLAN_UMAC_COMP_DFS,
				(void *)dfs);

		dfs_detach(dfs);
		dfs->dfs_pdev_obj = NULL;
		dfs_destroy_object(dfs);
	}

	return QDF_STATUS_SUCCESS;
}

static void dfs_scan_serialization_comp_info_cb(
		struct wlan_objmgr_vdev *vdev,
		union wlan_serialization_rules_info *comp_info)
{
	struct wlan_dfs *dfs = NULL;
	struct wlan_objmgr_pdev *pdev;

	if (!comp_info) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "comp_info is NULL");
		return;
	}

	if (!vdev) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "vdev is NULL");
		return;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "pdev is NULL");
		return;
	}

	comp_info->scan_info.is_cac_in_progress = false;

	if (!tgt_dfs_is_pdev_5ghz(pdev))
		return;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	if (dfs_is_ap_cac_timer_running(dfs))
		comp_info->scan_info.is_cac_in_progress = true;
}

QDF_STATUS wifi_dfs_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	status = tgt_dfs_reg_ev_handler(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tgt_dfs_reg_ev_handler failed");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_serialization_register_comp_info_cb(psoc,
			WLAN_UMAC_COMP_DFS,
			WLAN_SER_CMD_SCAN,
			dfs_scan_serialization_comp_info_cb);
	if (status != QDF_STATUS_SUCCESS) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "Serialize scan cmd register failed");
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wifi_dfs_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	status = wlan_serialization_deregister_comp_info_cb(psoc,
			WLAN_UMAC_COMP_DFS,
			WLAN_SER_CMD_SCAN);
	if (status != QDF_STATUS_SUCCESS) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "Serialize scan cmd deregister failed");
		return status;
	}

	return QDF_STATUS_SUCCESS;
}
