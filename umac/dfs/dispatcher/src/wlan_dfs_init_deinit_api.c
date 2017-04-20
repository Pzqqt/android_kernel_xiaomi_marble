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
#ifndef QCA_MCL_DFS_SUPPORT
#include "ieee80211_mlme_dfs_interface.h"
#endif
#include "wlan_objmgr_global_obj.h"
#include "wlan_dfs_init_deinit_api.h"
#include "../../core/src/dfs.h"

struct dfs_to_mlme global_dfs_to_mlme;

struct wlan_dfs *wlan_pdev_get_dfs_obj(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;
	wlan_pdev_obj_lock(pdev);
	dfs = wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_DFS);
	wlan_pdev_obj_unlock(pdev);

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
	tmp_dfs_to_mlme->mlme_get_ic_nchans = mlme_dfs_get_ic_nchans;
	tmp_dfs_to_mlme->mlme_get_ic_no_weather_radar_chan =
		mlme_dfs_get_ic_no_weather_radar_chan;
	tmp_dfs_to_mlme->mlme_find_alternate_mode_channel =
		mlme_dfs_find_alternate_mode_channel;
	tmp_dfs_to_mlme->mlme_find_any_valid_channel =
		mlme_dfs_find_any_valid_channel;
	tmp_dfs_to_mlme->mlme_get_extchan = mlme_dfs_get_extchan;
	tmp_dfs_to_mlme->mlme_set_no_chans_available =
		mlme_dfs_set_no_chans_available;
	tmp_dfs_to_mlme->mlme_ieee2mhz = mlme_dfs_ieee2mhz;
	tmp_dfs_to_mlme->mlme_find_dot11_channel = mlme_dfs_find_dot11_channel;
	tmp_dfs_to_mlme->mlme_get_ic_channels = mlme_dfs_get_ic_channels;
	tmp_dfs_to_mlme->mlme_ic_flags_ext = mlme_dfs_ic_flags_ext;
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

	if (pdev == NULL) {
		DFS_PRINTK("%s:PDEV is NULL\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (dfs_create_object(&dfs) == 1) {
		DFS_PRINTK("%s : Failed to create DFS object\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	global_dfs_to_mlme.pdev_component_obj_attach(pdev,
			WLAN_UMAC_COMP_DFS,
			(void *)dfs,
			QDF_STATUS_SUCCESS);
	dfs->dfs_pdev_obj = pdev;

	/* wlan_ar_ops are assigned to sc_wlan_ops in wlan_dev_attach.
	 * This function is called during module init.
	 * and wlan_dev_attach is called during wlan_attach.
	 * If we call dfs_attach here, seen crash in wlan_net80211_attach_dfs.
	 */
	if (dfs_attach(dfs) == 1) {
		DFS_PRINTK("%s : dfs_attch failed\n", __func__);
		dfs_destroy_object(dfs);
		return QDF_STATUS_E_FAILURE;
	}

	dfs_get_radars(dfs);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_dfs_pdev_obj_destroy_notification(struct wlan_objmgr_pdev *pdev,
		void *arg)
{
	struct wlan_dfs *dfs;

	if (pdev == NULL) {
		DFS_PRINTK("%s:PDEV is NULL\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	dfs = wlan_pdev_get_dfs_obj(pdev);

	/* DFS is NULL during unload. should we call this function before */
	if (dfs != NULL) {
		global_dfs_to_mlme.pdev_component_obj_detach(pdev,
				WLAN_UMAC_COMP_DFS,
				(void *)dfs);

		nif_dfs_detach(dfs);
		sif_dfs_detach(dfs);
		dfs->dfs_pdev_obj = NULL;
		dfs_destroy_object(dfs);
	}

	return QDF_STATUS_SUCCESS;
}

