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
 * DOC: Functions to call lmac/offload functions from DFS component.
 */

#include "wlan_dfs_lmac_api.h"
#include "../../core/src/dfs_internal.h"
#include <wlan_reg_services_api.h>

void lmac_get_caps(struct wlan_objmgr_pdev *pdev,
		bool *ext_chan,
		bool *combined_rssi,
		bool *use_enhancement,
		bool *strong_signal_diversiry,
		bool *chip_is_bb_tlv,
		bool *chip_is_over_sampled,
		bool *chip_is_ht160,
		bool *chip_is_false_detect,
		uint32_t *fastdiv_val)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);

	dfs_tx_ops = &psoc->soc_cb.tx_ops.dfs_tx_ops;

	if (dfs_tx_ops->dfs_get_caps)
		dfs_tx_ops->dfs_get_caps(pdev,
				ext_chan,
				combined_rssi,
				use_enhancement,
				strong_signal_diversiry,
				chip_is_bb_tlv,
				chip_is_over_sampled,
				chip_is_ht160,
				chip_is_false_detect,
				fastdiv_val);
}

uint64_t lmac_get_tsf64(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint64_t tsf64 = 0;

	psoc = wlan_pdev_get_psoc(pdev);

	dfs_tx_ops = &psoc->soc_cb.tx_ops.dfs_tx_ops;

	if (dfs_tx_ops->dfs_gettsf64)
		dfs_tx_ops->dfs_gettsf64(pdev, &tsf64);

	return tsf64;
}

void lmac_dfs_disable(struct wlan_objmgr_pdev *pdev, int no_cac)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);

	dfs_tx_ops = &psoc->soc_cb.tx_ops.dfs_tx_ops;

	if (dfs_tx_ops->dfs_disable)
		dfs_tx_ops->dfs_disable(pdev, no_cac);
}

void lmac_dfs_enable(struct wlan_objmgr_pdev *pdev,
		int *is_fastclk,
		int32_t pe_firpwr,
		int32_t pe_rrssi,
		int32_t pe_height,
		int32_t pe_prssi,
		int32_t pe_inband,
		uint32_t pe_relpwr,
		uint32_t pe_relstep,
		uint32_t pe_maxlen,
		int dfsdomain)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);

	dfs_tx_ops = &psoc->soc_cb.tx_ops.dfs_tx_ops;

	if (dfs_tx_ops->dfs_enable)
		dfs_tx_ops->dfs_enable(pdev,
				is_fastclk,
				pe_firpwr,
				pe_rrssi,
				pe_height,
				pe_prssi,
				pe_inband,
				pe_relpwr,
				pe_relstep,
				pe_maxlen,
				dfsdomain);
}

void lmac_dfs_get_thresholds(struct wlan_objmgr_pdev *pdev,
	int32_t *pe_firpwr,
	int32_t *pe_rrssi,
	int32_t *pe_height,
	int32_t *pe_prssi,
	int32_t *pe_inband,
	uint32_t *pe_relpwr,
	uint32_t *pe_relstep,
	uint32_t *pe_maxlen)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);

	dfs_tx_ops = &psoc->soc_cb.tx_ops.dfs_tx_ops;

	if (dfs_tx_ops->dfs_get_thresholds)
		dfs_tx_ops->dfs_get_thresholds(pdev,
				pe_firpwr,
				pe_rrssi,
				pe_height,
				pe_prssi,
				pe_inband,
				pe_relpwr,
				pe_relstep,
				pe_maxlen);
}

bool lmac_is_mode_offload(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	bool is_offload = false;

	psoc = wlan_pdev_get_psoc(pdev);

	dfs_tx_ops = &psoc->soc_cb.tx_ops.dfs_tx_ops;

	if (dfs_tx_ops->dfs_is_mode_offload)
		dfs_tx_ops->dfs_is_mode_offload(pdev, &is_offload);

	return is_offload;
}

uint16_t lmac_get_ah_devid(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint16_t devid = 0;

	psoc = wlan_pdev_get_psoc(pdev);

	dfs_tx_ops = &psoc->soc_cb.tx_ops.dfs_tx_ops;

	if (dfs_tx_ops->dfs_get_ah_devid)
		dfs_tx_ops->dfs_get_ah_devid(pdev, &devid);

	return devid;
}

uint32_t lmac_get_ext_busy(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint32_t ext_chan_busy = 0;

	psoc = wlan_pdev_get_psoc(pdev);

	dfs_tx_ops = &psoc->soc_cb.tx_ops.dfs_tx_ops;

	if (dfs_tx_ops->dfs_get_ext_busy)
		dfs_tx_ops->dfs_get_ext_busy(pdev, &ext_chan_busy);

	return ext_chan_busy;
}

void lmac_set_use_cac_prssi(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);

	dfs_tx_ops = &psoc->soc_cb.tx_ops.dfs_tx_ops;

	if (dfs_tx_ops->dfs_set_use_cac_prssi)
		dfs_tx_ops->dfs_set_use_cac_prssi(pdev);
}

uint32_t lmac_get_target_type(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint32_t target_type = 0;

	psoc = wlan_pdev_get_psoc(pdev);

	dfs_tx_ops = &psoc->soc_cb.tx_ops.dfs_tx_ops;

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

	psoc = wlan_pdev_get_psoc(pdev);

	dfs_tx_ops = &psoc->soc_cb.tx_ops.dfs_tx_ops;

	if (dfs_tx_ops->dfs_get_phymode_info)
		dfs_tx_ops->dfs_get_phymode_info(pdev, chan_mode, &mode_info);

	return mode_info;
}
