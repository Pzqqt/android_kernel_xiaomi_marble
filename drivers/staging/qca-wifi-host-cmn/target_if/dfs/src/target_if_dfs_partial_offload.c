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
#include "target_if_dfs.h"

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

#if defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS target_if_dfs_send_avg_params_to_fw(
		struct wlan_objmgr_pdev *pdev,
		struct dfs_radar_found_params *params)
{
	QDF_STATUS status;
	wmi_unified_t wmi_handle;

	if (!pdev) {
		target_if_err("null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle = get_wmi_unified_hdl_from_pdev(pdev);
	if (!wmi_handle) {
		target_if_err("null wmi_handle");
		return QDF_STATUS_E_FAILURE;
	}

	status = wmi_unified_dfs_send_avg_params_cmd(wmi_handle,
						     params);
	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("dfs radar found average parameters send failed: %d",
			      status);

	return status;
}

int target_if_dfs_status_check_event_handler(ol_scn_t scn,
					     uint8_t *data,
					     uint32_t datalen)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_lmac_if_dfs_rx_ops *dfs_rx_ops;
	u_int32_t dfs_status_check;
	wmi_unified_t wmi_hdl;

	if (!scn || !data) {
		target_if_err("scn: %pK, data: %pK", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("null psoc");
		return -EINVAL;
	}

	/* Since Partial Offload chipsets have only one pdev per psoc, the first
	 * pdev from the pdev list is used.
	 */
	pdev = wlan_objmgr_get_pdev_by_id(psoc, 0, WLAN_DFS_ID);
	if (!pdev) {
		target_if_err("null pdev");
		return -EINVAL;
	}

	dfs_rx_ops = target_if_dfs_get_rx_ops(psoc);
	if (!dfs_rx_ops) {
		target_if_err("null dfs_rx_ops");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_DFS_ID);
		return -EINVAL;
	}

	if (!dfs_rx_ops->dfs_action_on_status) {
		target_if_err("dfs_rx_ops->dfs_action_on_status is NULL");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_DFS_ID);
		return -EINVAL;
	}

	wmi_hdl = get_wmi_unified_hdl_from_pdev(pdev);
	if (!wmi_hdl) {
		target_if_err("wmi_hdl is NULL");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_DFS_ID);
		return -EINVAL;
	}

	if (wmi_extract_dfs_status_from_fw(wmi_hdl, data, &dfs_status_check) !=
			QDF_STATUS_SUCCESS) {
		target_if_err("failed to extract status response from FW");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_DFS_ID);
		return -EINVAL;
	}

	if (dfs_rx_ops->dfs_action_on_status(pdev, &dfs_status_check) !=
		QDF_STATUS_SUCCESS) {
		target_if_err("dfs action on host dfs status from FW failed");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_DFS_ID);
		return -EINVAL;
	}

	wlan_objmgr_pdev_release_ref(pdev, WLAN_DFS_ID);

	return 0;
}
#endif
