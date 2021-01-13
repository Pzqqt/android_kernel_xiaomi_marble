/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
 * DOC: This file consits of APIs related to PostNOL feature.
 */

#include "../../core/src/dfs_misc.h"
#include "dfs_zero_cac.h"
#include "wlan_dfs_mlme_api.h"
#include "ieee80211_mlme_dfs_interface.h"
#include "wlan_dfs_init_deinit_api.h"
#include "_ieee80211.h"
#include "ieee80211_var.h"
#include "ieee80211_mlme_dfs_dispatcher.h"
#include "ieee80211_channel.h"
#include "wlan_mlme_if.h"
#include "dfs_postnol_ucfg.h"

#ifdef QCA_SUPPORT_DFS_CHAN_POSTNOL
QDF_STATUS ucfg_dfs_set_postnol_freq(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t postnol_freq)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "null dfs");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_set_postnol_freq(dfs, postnol_freq);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_set_postnol_freq);

QDF_STATUS ucfg_dfs_set_postnol_mode(struct wlan_objmgr_pdev *pdev,
				     uint8_t postnol_mode)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "null dfs");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_set_postnol_mode(dfs, postnol_mode);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_set_postnol_mode);

QDF_STATUS ucfg_dfs_set_postnol_cfreq2(struct wlan_objmgr_pdev *pdev,
				       qdf_freq_t postnol_cfreq2)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "null dfs");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_set_postnol_cfreq2(dfs, postnol_cfreq2);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_set_postnol_cfreq2);

QDF_STATUS ucfg_dfs_get_postnol_freq(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t *postnol_freq)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "null dfs");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_get_postnol_freq(dfs, postnol_freq);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_get_postnol_freq);

QDF_STATUS ucfg_dfs_get_postnol_mode(struct wlan_objmgr_pdev *pdev,
				     uint8_t *postnol_mode)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "null dfs");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_get_postnol_mode(dfs, postnol_mode);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_get_postnol_mode);

QDF_STATUS ucfg_dfs_get_postnol_cfreq2(struct wlan_objmgr_pdev *pdev,
				       qdf_freq_t *postnol_cfreq2)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "null dfs");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_get_postnol_cfreq2(dfs, postnol_cfreq2);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ucfg_dfs_get_postnol_cfreq2);
#endif

#ifdef QCA_HW_MODE_SWITCH
bool lmac_dfs_is_hw_mode_switch_in_progress(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	bool is_hw_mode_switch_in_progress = false;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tx_ops is null");
		return is_hw_mode_switch_in_progress;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;

	if (dfs_tx_ops->dfs_check_mode_switch_state)
		dfs_tx_ops->dfs_check_mode_switch_state(
			pdev,
			&is_hw_mode_switch_in_progress);

	return is_hw_mode_switch_in_progress;
}

void dfs_mlme_acquire_radar_mode_switch_lock(struct wlan_objmgr_pdev *pdev)
{
	if (!global_dfs_to_mlme.mlme_acquire_radar_mode_switch_lock)
		return;

	global_dfs_to_mlme.mlme_acquire_radar_mode_switch_lock(pdev);
}

void dfs_mlme_release_radar_mode_switch_lock(struct wlan_objmgr_pdev *pdev)
{
	if (!global_dfs_to_mlme.mlme_release_radar_mode_switch_lock)
		return;

	global_dfs_to_mlme.mlme_release_radar_mode_switch_lock(pdev);
}

void tgt_dfs_init_tmp_psoc_nol(struct wlan_objmgr_pdev *pdev,
			       uint8_t num_radios)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return;
	}

	dfs_init_tmp_psoc_nol(dfs, num_radios);
}

qdf_export_symbol(tgt_dfs_init_tmp_psoc_nol);

void tgt_dfs_deinit_tmp_psoc_nol(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return;
	}

	dfs_deinit_tmp_psoc_nol(dfs);
}

qdf_export_symbol(tgt_dfs_deinit_tmp_psoc_nol);

void tgt_dfs_save_dfs_nol_in_psoc(struct wlan_objmgr_pdev *pdev,
				  uint8_t pdev_id)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return;
	}

	dfs_save_dfs_nol_in_psoc(dfs, pdev_id);
}

qdf_export_symbol(tgt_dfs_save_dfs_nol_in_psoc);

void tgt_dfs_reinit_nol_from_psoc_copy(struct wlan_objmgr_pdev *pdev,
				       uint8_t pdev_id,
				       uint16_t low_5ghz_freq,
				       uint16_t high_5ghz_freq)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return;
	}

	dfs_reinit_nol_from_psoc_copy(dfs,
				      pdev_id,
				      low_5ghz_freq,
				      high_5ghz_freq);
}

qdf_export_symbol(tgt_dfs_reinit_nol_from_psoc_copy);

void tgt_dfs_reinit_precac_lists(struct wlan_objmgr_pdev *src_pdev,
				 struct wlan_objmgr_pdev *dest_pdev,
				 uint16_t low_5g_freq,
				 uint16_t high_5g_freq)
{
	struct wlan_dfs *src_dfs, *dest_dfs;

	src_dfs = wlan_pdev_get_dfs_obj(src_pdev);
	if (!src_dfs) {
		dfs_err(src_dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return;
	}
	dest_dfs = wlan_pdev_get_dfs_obj(dest_pdev);
	if (!dest_dfs) {
		dfs_err(dest_dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return;
	}

	dfs_reinit_precac_lists(src_dfs, dest_dfs, low_5g_freq, high_5g_freq);
}

void tgt_dfs_complete_deferred_tasks(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return;
	}

	dfs_complete_deferred_tasks(dfs);
}
#endif

#ifdef QCA_SUPPORT_STA_DFS
void tgt_dfs_enable_stadfs(struct wlan_objmgr_pdev *pdev, bool val)
{
	struct wlan_dfs *dfs;
	uint32_t dfs_reg = 0;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return;
	}

	if (val == dfs->dfs_is_stadfs_enabled) {
	    dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
		    "STA DFS enable already set to %d", val);
	    return;
	}

	wlan_reg_get_dfs_region(pdev, &dfs_reg);

	if ((dfs_reg != DFS_ETSI_DOMAIN) && val) {
	    dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
		    "Cannot enable STA DFS on non ETSI Domain %d", dfs_reg);
	    dfs->dfs_is_stadfs_enabled = 0;
	    return;
	}

	dfs->dfs_is_stadfs_enabled = val;
}

bool tgt_dfs_is_stadfs_enabled(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_dfs *dfs;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return false;
	}

	return dfs->dfs_is_stadfs_enabled;
}
#endif

#ifdef QCA_SUPPORT_STA_DFS
void utils_dfs_get_nol_history_chan_list(struct wlan_objmgr_pdev *pdev,
					 void *clist, uint32_t *num_chan)
{
	int i, j = 0;
	struct regulatory_channel *cur_chan_list;
	struct wlan_dfs *dfs;
	struct dfs_channel *chan_list = (struct dfs_channel *)clist;

	*num_chan = 0;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	if (!dfs)
		return;

	cur_chan_list = qdf_mem_malloc(NUM_CHANNELS * sizeof(*cur_chan_list));
	if (!cur_chan_list)
		return;

	if (wlan_reg_get_current_chan_list(
			pdev, cur_chan_list) != QDF_STATUS_SUCCESS) {
		dfs_alert(dfs, WLAN_DEBUG_DFS_ALWAYS,
			  "failed to get cur_chan list");
		qdf_mem_free(cur_chan_list);
		return;
	}

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (cur_chan_list[i].nol_history) {
			chan_list[j].dfs_ch_freq = cur_chan_list[i].center_freq;
			j++;
		}
	}

	*num_chan = j;
	qdf_mem_free(cur_chan_list);
}
#else
void utils_dfs_get_nol_history_chan_list(struct wlan_objmgr_pdev *pdev,
					 void *clist, uint32_t *num_chan)
{
}
#endif
