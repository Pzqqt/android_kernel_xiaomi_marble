/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: target_if_dfs_partial_offload.c
 * This file contains dfs target interface for partial offload
 */

#include <target_if.h>
#include "target_type.h"
#include "target_if_dfs_partial_offload.h"

QDF_STATUS target_if_dfs_reg_phyerr_events(struct wlan_objmgr_psoc *psoc)
{
	/* TODO: dfs non-offload case */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_dfs_get_caps(struct wlan_objmgr_pdev *pdev,
		struct wlan_dfs_caps *dfs_caps)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct target_psoc_info *tgt_psoc_info;

	if (!dfs_caps) {
		target_if_err("null dfs_caps");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_caps->wlan_dfs_combined_rssi_ok = 0;
	dfs_caps->wlan_dfs_ext_chan_ok = 0;
	dfs_caps->wlan_dfs_use_enhancement = 0;
	dfs_caps->wlan_strong_signal_diversiry = 0;
	dfs_caps->wlan_fastdiv_val = 0;
	dfs_caps->wlan_chip_is_bb_tlv = 1;
	dfs_caps->wlan_chip_is_over_sampled = 0;
	dfs_caps->wlan_chip_is_ht160 = 0;
	dfs_caps->wlan_chip_is_false_detect = 0;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		target_if_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	tgt_psoc_info = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_psoc_info) {
		target_if_err("null tgt_psoc_info");
		return QDF_STATUS_E_FAILURE;
	}

	switch (target_psoc_get_target_type(tgt_psoc_info)) {
	case TARGET_TYPE_AR900B:
		break;

	case TARGET_TYPE_IPQ4019:
		dfs_caps->wlan_chip_is_false_detect = 0;
		break;

	case TARGET_TYPE_AR9888:
		dfs_caps->wlan_chip_is_over_sampled = 1;
		break;

	case TARGET_TYPE_QCA9984:
	case TARGET_TYPE_QCA9888:
		dfs_caps->wlan_chip_is_ht160 = 1;
		break;
	default:
		break;
	}

	return QDF_STATUS_SUCCESS;
}
