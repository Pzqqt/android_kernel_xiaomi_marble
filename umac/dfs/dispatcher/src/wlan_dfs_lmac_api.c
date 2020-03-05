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
 * DOC: Functions to call lmac/offload functions from DFS component.
 */

#include "wlan_dfs_lmac_api.h"
#include "../../core/src/dfs_internal.h"
#include <wlan_reg_services_api.h>
#include <wlan_lmac_if_def.h>

void lmac_get_caps(struct wlan_objmgr_pdev *pdev,
		struct wlan_dfs_caps *dfs_caps)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tx_ops is null");
		return;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;

	if (dfs_tx_ops->dfs_get_caps)
		dfs_tx_ops->dfs_get_caps(pdev, dfs_caps);
}

uint64_t lmac_get_tsf64(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint64_t tsf64 = 0;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tx_ops is null");
		return tsf64;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;

	if (dfs_tx_ops->dfs_gettsf64)
		dfs_tx_ops->dfs_gettsf64(pdev, &tsf64);

	return tsf64;
}

void lmac_dfs_disable(struct wlan_objmgr_pdev *pdev, int no_cac)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tx_ops is null");
		return;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;

	if (dfs_tx_ops->dfs_disable)
		dfs_tx_ops->dfs_disable(pdev, no_cac);
}

void lmac_dfs_enable(struct wlan_objmgr_pdev *pdev,
		int *is_fastclk,
		struct wlan_dfs_phyerr_param *param,
		int dfsdomain)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tx_ops is null");
		return;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;

	if (dfs_tx_ops->dfs_enable)
		dfs_tx_ops->dfs_enable(pdev,
				is_fastclk,
				param,
				dfsdomain);
}

void lmac_dfs_get_thresholds(struct wlan_objmgr_pdev *pdev,
		struct wlan_dfs_phyerr_param *param)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tx_ops is null");
		return;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;

	if (dfs_tx_ops->dfs_get_thresholds)
		dfs_tx_ops->dfs_get_thresholds(pdev, param);
}

uint16_t lmac_get_ah_devid(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint16_t devid = 0;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tx_ops is null");
		return devid;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;

	if (dfs_tx_ops->dfs_get_ah_devid)
		dfs_tx_ops->dfs_get_ah_devid(pdev, &devid);

	return devid;
}

uint32_t lmac_get_ext_busy(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint32_t ext_chan_busy = 0;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tx_ops is null");
		return ext_chan_busy;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;

	if (dfs_tx_ops->dfs_get_ext_busy)
		dfs_tx_ops->dfs_get_ext_busy(pdev, &ext_chan_busy);

	return ext_chan_busy;
}

void lmac_set_use_cac_prssi(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tx_ops is null");
		return;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;

	if (dfs_tx_ops->dfs_set_use_cac_prssi)
		dfs_tx_ops->dfs_set_use_cac_prssi(pdev);
}

uint32_t lmac_get_target_type(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint32_t target_type = 0;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tx_ops is null");
		return target_type;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;

	if (dfs_tx_ops->dfs_get_target_type)
		dfs_tx_ops->dfs_get_target_type(pdev, &target_type);

	return target_type;
}

uint32_t lmac_get_phymode_info(struct wlan_objmgr_pdev *pdev,
		uint32_t chan_mode)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint32_t mode_info = 0;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tx_ops is null");
		return mode_info;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;

	/* since dfs never comes into 2G, hardcode is_2gvht_en flag to false */
	if (dfs_tx_ops->dfs_get_phymode_info)
		dfs_tx_ops->dfs_get_phymode_info(pdev, chan_mode, &mode_info,
						 false);

	return mode_info;
}

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
bool lmac_is_host_dfs_check_support_enabled(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	bool enabled = false;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "tx_ops is null");
		return enabled;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;

	if (dfs_tx_ops->dfs_host_dfs_check_support)
		dfs_tx_ops->dfs_host_dfs_check_support(pdev, &enabled);

	return enabled;
}
#endif

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
