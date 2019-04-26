/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: target_if_dfs_full_offload.c
 * This file contains dfs target interface for full offload
 */

#include <target_if.h>
#include <target_if_dfs.h>
#include <wmi_unified_dfs_api.h>
#include <init_deinit_lmac.h>
#include <wlan_module_ids.h>
#include <target_if_dfs_full_offload.h>
#include <wlan_dfs_tgt_api.h>
#include <wlan_objmgr_pdev_obj.h>

#if defined(QCA_SUPPORT_AGILE_DFS)
#include <wlan_mlme_dispatcher.h>
#define QUICK_OCAC_MODE 0
#endif
/**
 * target_if_dfs_cac_complete_event_handler() - CAC complete indication.
 * @scn: scn handle.
 * @data: Pointer to data buffer.
 * @datalen: data length.
 *
 * Return: 0 on successful indication.
 */
static int target_if_dfs_cac_complete_event_handler(
		ol_scn_t scn, uint8_t *data, uint32_t datalen)
{
	struct wlan_lmac_if_dfs_rx_ops *dfs_rx_ops;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	int ret = 0;
	uint32_t vdev_id = 0;
	struct wmi_unified *wmi_handle;

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

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("Invalid WMI handle");
		return -EINVAL;
	}

	if (wmi_extract_dfs_cac_complete_event(wmi_handle, data, &vdev_id,
					       datalen) != QDF_STATUS_SUCCESS) {
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

#if defined(QCA_SUPPORT_AGILE_DFS)
/**
 * target_if_dfs_ocac_complete_event_handler() - Off Channel CAC complete
 *						 indication.
 * @scn: scn handle.
 * @data: Pointer to data buffer.
 * @datalen: data length.
 *
 * Return: 0 on successful indication.
 */
static int target_if_dfs_ocac_complete_event_handler(
		ol_scn_t scn, uint8_t *data, uint32_t datalen)
{
	struct wlan_lmac_if_dfs_rx_ops *dfs_rx_ops;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct vdev_adfs_complete_status ocac_status;
	int ret = 0;
	struct wmi_unified *wmi_handle;

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
	if (!dfs_rx_ops || !dfs_rx_ops->dfs_dfs_ocac_complete_ind) {
		target_if_err("Invalid dfs_rx_ops: %pK", dfs_rx_ops);
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("Invalid WMI handle");
		return -EINVAL;
	}

	if (wmi_extract_dfs_ocac_complete_event(wmi_handle,
						data,
						&ocac_status)
						!= QDF_STATUS_SUCCESS) {
		target_if_err("failed to extract off channel cac complete event");
		return -EFAULT;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						    ocac_status.vdev_id,
						    WLAN_DFS_ID);
	if (!vdev) {
		target_if_err("null vdev");
		return -EINVAL;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		target_if_err("null pdev");
		ret = -EINVAL;
		goto free_vdevref;
	}

	if (!ret && (QDF_STATUS_SUCCESS !=
	    dfs_rx_ops->dfs_dfs_ocac_complete_ind(pdev, &ocac_status))) {
		target_if_err("dfs_dfs_ocac_complete_ind failed");
		ret = -EINVAL;
	}

free_vdevref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_DFS_ID);

	return ret;
}
#endif

/**
 * target_if_dfs_radar_detection_event_handler() - Indicate RADAR detection and
 * process RADAR detection.
 * @scn: scn handle.
 * @data: pointer to data buffer.
 * @datalen: data length.
 *
 * Return: 0 on successful indication.
 */
static int target_if_dfs_radar_detection_event_handler(
		ol_scn_t scn, uint8_t *data, uint32_t datalen)
{
	struct radar_found_info radar;
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_lmac_if_dfs_rx_ops *dfs_rx_ops;
	int ret = 0;
	struct wmi_unified *wmi_handle;

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

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("Invalid WMI handle");
		return -EINVAL;
	}

	if (wmi_extract_dfs_radar_detection_event(wmi_handle, data, &radar,
						  datalen)
	    != QDF_STATUS_SUCCESS) {
		target_if_err("failed to extract cac complete event");
		return -EFAULT;
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, radar.pdev_id, WLAN_DFS_ID);
	if (!pdev) {
		target_if_err("null pdev");
		return -EINVAL;
	}

	if (dfs_rx_ops->dfs_process_radar_ind(pdev,
				&radar) != QDF_STATUS_SUCCESS) {
		target_if_err("dfs_process_radar_ind failed pdev_id=%d",
			      radar.pdev_id);
		ret = -EINVAL;
	}

	wlan_objmgr_pdev_release_ref(pdev, WLAN_DFS_ID);

	return ret;
}

/**
 * target_if_dfs_reg_ocac_event() - registers dfs off channel event
 * for full offload.
 * @psoc: Pointer to psoc object.
 *
 * Return: 0 on successful registration.
 */
#if defined(QCA_SUPPORT_AGILE_DFS)
static int target_if_dfs_reg_ocac_event(struct wlan_objmgr_psoc *psoc)
{
	return wmi_unified_register_event(
			get_wmi_unified_hdl_from_psoc(psoc),
			wmi_vdev_ocac_complete_event_id,
			target_if_dfs_ocac_complete_event_handler);
}
#else
static int target_if_dfs_reg_ocac_event(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}
#endif

#if defined(WLAN_DFS_FULL_OFFLOAD)
QDF_STATUS target_if_dfs_reg_offload_events(
		struct wlan_objmgr_psoc *psoc)
{
	int ret1, ret2, ret3;

	ret1 = wmi_unified_register_event(
			get_wmi_unified_hdl_from_psoc(psoc),
			wmi_dfs_radar_detection_event_id,
			target_if_dfs_radar_detection_event_handler);
	target_if_debug("wmi_dfs_radar_detection_event_id ret=%d", ret1);

	ret2 = wmi_unified_register_event(
			get_wmi_unified_hdl_from_psoc(psoc),
			wmi_dfs_cac_complete_id,
			target_if_dfs_cac_complete_event_handler);
	target_if_debug("wmi_dfs_cac_complete_id ret=%d", ret2);

	ret3 = target_if_dfs_reg_ocac_event(psoc);
	target_if_debug("wmi_vdev_ocac_complete_event_id ret=%d", ret3);

	if (ret1 || ret2 || ret3)
		return QDF_STATUS_E_FAILURE;
	else
		return QDF_STATUS_SUCCESS;
}
#endif

#if defined(QCA_SUPPORT_AGILE_DFS)
QDF_STATUS target_send_ocac_abort_cmd(struct wlan_objmgr_pdev *pdev)
{
	wmi_unified_t wmi_handle;
	struct vdev_adfs_abort_params param;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status;

	if (!pdev) {
		target_if_err("null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = wlan_objmgr_pdev_get_first_vdev(pdev, WLAN_DFS_ID);

	if (!vdev) {
		target_if_err("null vdev");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle = get_wmi_unified_hdl_from_pdev(pdev);
	if (!wmi_handle) {
		target_if_err("null wmi_handle");
		status = QDF_STATUS_E_FAILURE;
		goto free_vdevref;
	}

	qdf_mem_set(&param, sizeof(param), 0);
	param.vdev_id = wlan_vdev_get_id(vdev);
	utils_dfs_cancel_precac_timer(pdev);

	status = wmi_unified_send_vdev_adfs_ocac_abort_cmd(wmi_handle, &param);
	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("dfs: unit_test_cmd send failed %d", status);

free_vdevref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_DFS_ID);

	return status;
}

QDF_STATUS target_send_agile_ch_cfg_cmd(struct wlan_objmgr_pdev *pdev,
					uint8_t *ch_freq)
{
	wmi_unified_t wmi_handle;
	struct vdev_adfs_ch_cfg_params param;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status;

	if (!pdev) {
		target_if_err("null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = wlan_objmgr_pdev_get_first_vdev(pdev, WLAN_DFS_ID);

	if (!vdev) {
		target_if_err("null vdev");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle = get_wmi_unified_hdl_from_pdev(pdev);
	if (!wmi_handle) {
		target_if_err("null wmi_handle");
		status = QDF_STATUS_E_FAILURE;
		goto free_vdevref;
	}

	qdf_mem_set(&param, sizeof(param), 0);
	param.vdev_id = wlan_vdev_get_id(vdev);
	param.ocac_mode = QUICK_OCAC_MODE;
	param.min_duration_ms = 60000;
	param.max_duration_ms = 0;
	param.chan_freq = *ch_freq;
	param.chan_width = wlan_vdev_get_ch_width(vdev);
	param.center_freq = *ch_freq;

	status = wmi_unified_send_vdev_adfs_ch_cfg_cmd(wmi_handle, &param);
	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("dfs: unit_test_cmd send failed %d", status);

free_vdevref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_DFS_ID);

	return status;
}
#endif

#if (defined(CONFIG_MCL) || defined(QCA_WIFI_QCA8074) || \
	defined(QCA_WIFI_QCA6018))
QDF_STATUS target_process_bang_radar_cmd(
		struct wlan_objmgr_pdev *pdev,
		struct dfs_emulate_bang_radar_test_cmd *dfs_unit_test)
{
	QDF_STATUS status;
	struct wmi_unit_test_cmd wmi_utest;
	int i;
	wmi_unified_t wmi_handle;
	uint32_t target_pdev_id = 0;

	if (!pdev) {
		target_if_err("null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle = get_wmi_unified_hdl_from_pdev(pdev);
	if (!wmi_handle) {
		target_if_err("null wmi_handle");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_utest.vdev_id = dfs_unit_test->vdev_id;
	wmi_utest.module_id = WLAN_MODULE_PHYERR_DFS;
	wmi_utest.num_args = dfs_unit_test->num_args;

	for (i = 0; i < dfs_unit_test->num_args; i++)
		wmi_utest.args[i] = dfs_unit_test->args[i];
	/*
	 * Host to Target  conversion for pdev id required
	 * before we send a wmi unit test command
	 */
	if (wmi_convert_pdev_id_host_to_target(
				wmi_handle, pdev->pdev_objmgr.wlan_pdev_id,
				&target_pdev_id) != QDF_STATUS_SUCCESS) {
		target_if_err("failed to convert host pdev id to target");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_utest.args[IDX_PDEV_ID] = target_pdev_id;

	status = wmi_unified_unit_test_cmd(wmi_handle, &wmi_utest);
	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("dfs: unit_test_cmd send failed %d", status);
	return status;
}
#endif

#if defined(WLAN_DFS_FULL_OFFLOAD) && defined(QCA_DFS_NOL_OFFLOAD)
QDF_STATUS target_send_usenol_pdev_param(struct wlan_objmgr_pdev *pdev,
					 bool usenol)
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
	status = wmi_send_usenol_pdev_param(wmi_handle, usenol, pdev);

	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("dfs: usenol_pdev_param send failed %d", status);
	return status;
}

QDF_STATUS
target_send_subchan_marking_pdev_param(struct wlan_objmgr_pdev *pdev,
				       bool subchanmark)
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
	status = wmi_send_subchan_marking_pdev_param(wmi_handle,
						     subchanmark, pdev);

	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("dfs: subchan_marking_pdev_param send failed %d",
			      status);

	return status;
}
#endif
