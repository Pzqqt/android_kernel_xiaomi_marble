/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
#include "wlan_dfs_utils_api.h"
#ifndef QCA_MCL_DFS_SUPPORT
#include "ieee80211_mlme_dfs_interface.h"
#endif
#include "wlan_objmgr_global_obj.h"
#include "wlan_dfs_init_deinit_api.h"
#include "../../core/src/dfs.h"
#include "a_types.h"
#include "wlan_serialization_api.h"
#include <qdf_trace.h>

struct dfs_to_mlme global_dfs_to_mlme;

struct wlan_dfs *wlan_pdev_get_dfs_obj(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;
	dfs = wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_DFS);

	return dfs;
}

#ifndef QCA_MCL_DFS_SUPPORT
void register_dfs_callbacks(void)
{
	struct dfs_to_mlme *tmp_dfs_to_mlme = &global_dfs_to_mlme;

	tmp_dfs_to_mlme->pdev_component_obj_attach =
		wlan_objmgr_pdev_component_obj_attach;
	tmp_dfs_to_mlme->pdev_component_obj_detach =
		wlan_objmgr_pdev_component_obj_detach;
	tmp_dfs_to_mlme->pdev_get_comp_private_obj =
		wlan_pdev_get_dfs_obj;

	tmp_dfs_to_mlme->dfs_channel_mark_radar = mlme_dfs_channel_mark_radar;
	tmp_dfs_to_mlme->dfs_start_rcsa = mlme_dfs_start_rcsa;
	tmp_dfs_to_mlme->mlme_mark_dfs = mlme_dfs_mark_dfs;
	tmp_dfs_to_mlme->mlme_start_csa = mlme_dfs_start_csa;
	tmp_dfs_to_mlme->mlme_proc_cac = mlme_dfs_proc_cac;
	tmp_dfs_to_mlme->mlme_deliver_event_up_afrer_cac =
		mlme_dfs_deliver_event_up_afrer_cac;
	tmp_dfs_to_mlme->mlme_get_dfs_ch_nchans = mlme_dfs_get_dfs_ch_nchans;
	tmp_dfs_to_mlme->mlme_get_dfs_ch_no_weather_radar_chan =
		mlme_dfs_get_dfs_ch_no_weather_radar_chan;
	tmp_dfs_to_mlme->mlme_find_alternate_mode_channel =
		mlme_dfs_find_alternate_mode_channel;
	tmp_dfs_to_mlme->mlme_find_any_valid_channel =
		mlme_dfs_find_any_valid_channel;
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
}
#else
void register_dfs_callbacks(void)
{
	struct dfs_to_mlme *tmp_dfs_to_mlme = &global_dfs_to_mlme;

	tmp_dfs_to_mlme->pdev_component_obj_attach =
		wlan_objmgr_pdev_component_obj_attach;
	tmp_dfs_to_mlme->pdev_component_obj_detach =
		wlan_objmgr_pdev_component_obj_detach;
	tmp_dfs_to_mlme->pdev_get_comp_private_obj =
		wlan_pdev_get_dfs_obj;
}
#endif

QDF_STATUS dfs_init(void)
{
	QDF_STATUS res;

	register_dfs_callbacks();

	if (wlan_objmgr_register_pdev_create_handler(WLAN_UMAC_COMP_DFS,
				wlan_dfs_pdev_obj_create_notification,
				NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_register_pdev_destroy_handler(WLAN_UMAC_COMP_DFS,
				wlan_dfs_pdev_obj_destroy_notification,
				NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	res = qdf_print_set_category_verbose(qdf_get_pidx(),
			QDF_MODULE_ID_DFS, QDF_TRACE_LEVEL_INFO, true);
	if (res) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "Failed to set verbose for category");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dfs_deinit(void)
{
	if (wlan_objmgr_unregister_pdev_create_handler(WLAN_UMAC_COMP_DFS,
				wlan_dfs_pdev_obj_create_notification,
				NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_objmgr_unregister_pdev_destroy_handler(WLAN_UMAC_COMP_DFS,
				wlan_dfs_pdev_obj_destroy_notification,
				NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_dfs_pdev_obj_create_notification(struct wlan_objmgr_pdev *pdev,
		void *arg)
{
	struct wlan_dfs *dfs = NULL;
	struct wlan_objmgr_psoc *psoc;

	if (!pdev) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	if (dfs_create_object(&dfs) == 1) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "failed to create object");
		return QDF_STATUS_E_FAILURE;
	}

	global_dfs_to_mlme.pdev_component_obj_attach(pdev,
		WLAN_UMAC_COMP_DFS, (void *)dfs, QDF_STATUS_SUCCESS);
	dfs->dfs_pdev_obj = pdev;
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null psoc");
		return QDF_STATUS_E_FAILURE;
	}
	dfs->dfs_is_offload_enabled =
		DFS_OFFLOAD_IS_ENABLED(psoc->service_param.service_bitmap);
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"dfs_offload %d", dfs->dfs_is_offload_enabled);
	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (dfs_attach(dfs) == 1) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs_attch failed");
		dfs_destroy_object(dfs);
		return QDF_STATUS_E_FAILURE;
	}
	dfs_init_nol(pdev);
	dfs_print_nol(dfs);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_dfs_pdev_obj_destroy_notification(struct wlan_objmgr_pdev *pdev,
		void *arg)
{
	struct wlan_dfs *dfs;

	if (!pdev) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "PDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	dfs = wlan_pdev_get_dfs_obj(pdev);

	/* DFS is NULL during unload. should we call this function before */
	if (dfs != NULL) {
		global_dfs_to_mlme.pdev_component_obj_detach(pdev,
				WLAN_UMAC_COMP_DFS,
				(void *)dfs);

		dfs_reset(dfs);
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
	struct wlan_dfs *dfs;
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

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	if (dfs_is_ap_cac_timer_running(dfs))
		comp_info->scan_info.is_cac_in_progress = true;
	else
		comp_info->scan_info.is_cac_in_progress = false;
}

QDF_STATUS wifi_dfs_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	bool dfs_offload =
		DFS_OFFLOAD_IS_ENABLED(psoc->service_param.service_bitmap);

	dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS, "dfs_offload %d", dfs_offload);

	status = tgt_dfs_reg_ev_handler(psoc, dfs_offload);
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
