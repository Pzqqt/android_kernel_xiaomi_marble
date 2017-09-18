/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: target_if_dfs.c
 * This file contains dfs target interface
 */

#include <target_if.h>
#include <qdf_types.h>
#include <qdf_status.h>
#include <target_if_dfs.h>
#include <wmi_unified_api.h>
#include <wlan_lmac_if_def.h>
#include <wmi_unified_priv.h>
#include <wlan_scan_tgt_api.h>
#include <wmi_unified_param.h>
#include <wmi_unified_dfs_api.h>

static inline struct wlan_lmac_if_dfs_rx_ops *
target_if_dfs_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.rx_ops.dfs_rx_ops;
}

static int target_if_dfs_cac_complete_event_handler(
		ol_scn_t scn, uint8_t *data, uint32_t datalen)
{
	struct wlan_lmac_if_dfs_rx_ops *dfs_rx_ops;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	int ret = 0;
	uint32_t vdev_id = 0;

	if (!scn || !data) {
		target_if_err("scn: %pK, data: %pK", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("null psoc");
		return -EINVAL;
	}

	dfs_rx_ops = target_if_dfs_get_rx_ops(psoc);
	if (!dfs_rx_ops || !dfs_rx_ops->dfs_dfs_cac_complete_ind) {
		target_if_err("Invalid dfs_rx_ops: %pK", dfs_rx_ops);
		return -EINVAL;
	}

	if (wmi_extract_dfs_cac_complete_event(GET_WMI_HDL_FROM_PSOC(psoc),
			data, &vdev_id, datalen) != QDF_STATUS_SUCCESS) {
		target_if_err("failed to extract cac complete event");
		return -EFAULT;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id, WLAN_DFS_ID);
	if (!vdev) {
		target_if_err("null vdev");
		return -EINVAL;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		target_if_err("null pdev");
		ret = -EINVAL;
	}

	if (!ret && (QDF_STATUS_SUCCESS !=
	    dfs_rx_ops->dfs_dfs_cac_complete_ind(pdev, vdev_id))) {
		target_if_err("dfs_dfs_cac_complete_ind failed");
		ret = -EINVAL;
	}
	wlan_objmgr_vdev_release_ref(vdev, WLAN_DFS_ID);

	return ret;
}

static int target_if_dfs_radar_detection_event_handler(
		ol_scn_t scn, uint8_t *data, uint32_t datalen)
{
	struct radar_found_info radar;
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_lmac_if_dfs_rx_ops *dfs_rx_ops;
	int ret = 0;

	if (!scn || !data) {
		target_if_err("scn: %pK, data: %pK", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("null psoc");
		return -EINVAL;
	}

	dfs_rx_ops = target_if_dfs_get_rx_ops(psoc);
	if (!dfs_rx_ops || !dfs_rx_ops->dfs_process_radar_ind) {
		target_if_err("Invalid dfs_rx_ops: %pK", dfs_rx_ops);
		return -EINVAL;
	}

	if (wmi_extract_dfs_radar_detection_event(GET_WMI_HDL_FROM_PSOC(psoc),
			data, &radar, datalen) != QDF_STATUS_SUCCESS) {
		target_if_err("failed to extract cac complete event");
		return -EFAULT;
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, radar.pdev_id, WLAN_DFS_ID);
	if (!pdev) {
		target_if_err("null pdev");
		return -EINVAL;
	}

	if (QDF_STATUS_SUCCESS != dfs_rx_ops->dfs_process_radar_ind(pdev,
				&radar)) {
		target_if_err("dfs_process_radar_ind failed pdev_id=%d",
			      radar.pdev_id);
		ret = -EINVAL;
	}

	wlan_objmgr_pdev_release_ref(pdev, WLAN_DFS_ID);

	return ret;
}

static QDF_STATUS target_if_dfs_reg_offload_events(
		struct wlan_objmgr_psoc *psoc)
{
	int ret1, ret2;

	ret1 = wmi_unified_register_event(GET_WMI_HDL_FROM_PSOC(psoc),
			wmi_dfs_radar_detection_event_id,
			target_if_dfs_radar_detection_event_handler);
	target_if_debug("wmi_dfs_radar_detection_event_id ret=%d", ret1);

	ret2 = wmi_unified_register_event(GET_WMI_HDL_FROM_PSOC(psoc),
			wmi_dfs_cac_complete_id,
			target_if_dfs_cac_complete_event_handler);
	target_if_debug("wmi_dfs_cac_complete_id ret=%d", ret2);

	if (ret1 || ret2)
		return QDF_STATUS_E_FAILURE;
	else
		return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_dfs_reg_phyerr_events(struct wlan_objmgr_psoc *psoc)
{
	/* TODO: dfs non-offload case */
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS target_if_dfs_register_event_handler(
		struct wlan_objmgr_pdev *pdev,
		bool dfs_offload)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		target_if_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	if (!dfs_offload)
		return target_if_dfs_reg_phyerr_events(psoc);

	/* dfs offload case, send offload enable command first */
	status = wmi_unified_dfs_phyerr_offload_en_cmd(pdev->tgt_if_handle,
			WMI_HOST_PDEV_ID_SOC);
	if (QDF_IS_STATUS_SUCCESS(status))
		status = target_if_dfs_reg_offload_events(psoc);

	return status;
}

QDF_STATUS target_if_register_dfs_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;

	if (!tx_ops) {
		target_if_err("invalid tx_ops");
		return QDF_STATUS_E_FAILURE;
	}

	dfs_tx_ops = &tx_ops->dfs_tx_ops;
	dfs_tx_ops->dfs_reg_ev_handler = &target_if_dfs_register_event_handler;

	return QDF_STATUS_SUCCESS;
}
