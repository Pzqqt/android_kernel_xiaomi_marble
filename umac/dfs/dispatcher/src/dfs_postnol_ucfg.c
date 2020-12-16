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

