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
 * DOC: target_if_dfs.c
 * This file contains dfs target interface
 */

#include <target_if.h>
#include <qdf_types.h>
#include <qdf_status.h>
#include <target_if_dfs.h>
#include <wlan_module_ids.h>
#include <wmi_unified_api.h>
#include <wlan_lmac_if_def.h>
#include <wmi_unified_priv.h>
#include <wlan_scan_tgt_api.h>
#include <wmi_unified_param.h>
#include <wmi_unified_dfs_api.h>
#include "wlan_dfs_tgt_api.h"
#include "target_type.h"
#include <init_deinit_ucfg.h>
#include <wlan_reg_ucfg_api.h>

static inline struct wlan_lmac_if_dfs_rx_ops *
target_if_dfs_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.rx_ops.dfs_rx_ops;
}

/**
 * target_if_is_dfs_3() - Is dfs3 support or not
 * @target_type: target type being used.
 *
 * Return: true if dfs3 is supported, false otherwise.
 */
static bool target_if_is_dfs_3(uint32_t target_type)
{
	bool is_dfs_3;

	switch (target_type) {
	case TARGET_TYPE_AR6320:
		is_dfs_3 = false;
		break;
	case TARGET_TYPE_ADRASTEA:
		is_dfs_3 = true;
		break;
	default:
		is_dfs_3 = true;
	}

	return is_dfs_3;
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

#ifdef QCA_MCL_DFS_SUPPORT
/**
 * target_if_radar_event_handler() - handle radar event when
 * phyerr filter offload is enabled.
 * @scn: Handle to HIF context
 * @data: radar event buffer
 * @datalen: radar event buffer length
 *
 * Return: 0 on success; error code otherwise
*/
static int target_if_radar_event_handler(
	ol_scn_t scn, uint8_t *data, uint32_t datalen)
{
	struct radar_event_info wlan_radar_event;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_lmac_if_dfs_rx_ops *dfs_rx_ops;

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

	if (!dfs_rx_ops || !dfs_rx_ops->dfs_process_phyerr_filter_offload) {
		target_if_err("Invalid dfs_rx_ops: %pK", dfs_rx_ops);
		return -EINVAL;
	}
	if (QDF_IS_STATUS_ERROR(wmi_extract_wlan_radar_event_info(
			GET_WMI_HDL_FROM_PSOC(psoc), data,
			&wlan_radar_event, datalen))) {
		target_if_err("failed to extract wlan radar event");
		return -EFAULT;
	}
	pdev = wlan_objmgr_get_pdev_by_id(psoc, wlan_radar_event.pdev_id,
					WLAN_DFS_ID);
	if (!pdev) {
		target_if_err("null pdev");
		return -EINVAL;
	}
	dfs_rx_ops->dfs_process_phyerr_filter_offload(pdev,
					&wlan_radar_event);
	wlan_objmgr_pdev_release_ref(pdev, WLAN_DFS_ID);

	return 0;
}

/**
 * target_if_reg_phyerr_events() - register dfs phyerr radar event.
 * @psoc: pointer to psoc.
 * @pdev: pointer to pdev.
 *
 * Return: QDF_STATUS.
 */
static QDF_STATUS target_if_reg_phyerr_events_dfs2(
				struct wlan_objmgr_psoc *psoc)
{
	int ret = -1;
	struct wlan_lmac_if_dfs_rx_ops *dfs_rx_ops;
	bool is_phyerr_filter_offload;

	dfs_rx_ops = target_if_dfs_get_rx_ops(psoc);

	if (dfs_rx_ops && dfs_rx_ops->dfs_is_phyerr_filter_offload)
		if (QDF_IS_STATUS_SUCCESS(
			dfs_rx_ops->dfs_is_phyerr_filter_offload(psoc,
						&is_phyerr_filter_offload)))
			if (is_phyerr_filter_offload)
				ret = wmi_unified_register_event(
					GET_WMI_HDL_FROM_PSOC(psoc),
					wmi_dfs_radar_event_id,
					target_if_radar_event_handler);

	if (ret) {
		target_if_err("failed to register wmi_dfs_radar_event_id");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS target_if_reg_phyerr_events_dfs2(
				struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static QDF_STATUS target_if_dfs_register_event_handler(
		struct wlan_objmgr_psoc *psoc,
		bool dfs_offload)
{
	struct target_psoc_info *tgt_psoc_info;

	if (!psoc) {
		target_if_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	if (!dfs_offload) {
		tgt_psoc_info = wlan_psoc_get_tgt_if_handle(psoc);
		if (!tgt_psoc_info) {
			target_if_err("null tgt_psoc_info");
			return QDF_STATUS_E_FAILURE;
		}
		if (target_if_is_dfs_3(
				target_psoc_get_target_type(tgt_psoc_info)))
			return target_if_dfs_reg_phyerr_events(psoc);
		else
			return target_if_reg_phyerr_events_dfs2(psoc);
	} else {
		return target_if_dfs_reg_offload_events(psoc);
	}
}

static QDF_STATUS target_process_bang_radar_cmd(
		struct wlan_objmgr_pdev *pdev,
		struct dfs_emulate_bang_radar_test_cmd *dfs_unit_test)
{
	QDF_STATUS status;
	struct wmi_unit_test_cmd wmi_utest;
	int i;
	wmi_unified_t wmi_handle;

	if (!pdev) {
		target_if_err("null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle = GET_WMI_HDL_FROM_PDEV(pdev);
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
	wmi_utest.args[IDX_PDEV_ID] = wmi_handle->ops->
		convert_pdev_id_host_to_target(pdev->pdev_objmgr.wlan_pdev_id);

	status = wmi_unified_unit_test_cmd(wmi_handle, &wmi_utest);
	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("dfs: unit_test_cmd send failed %d", status);

	return status;
}

static QDF_STATUS target_if_dfs_is_pdev_5ghz(struct wlan_objmgr_pdev *pdev,
		bool *is_5ghz)
{
	struct wlan_objmgr_psoc *psoc;
	uint8_t pdev_id;
	struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap_ptr;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		target_if_err("dfs: null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	reg_cap_ptr = ucfg_reg_get_hal_reg_cap(psoc);
	if (!reg_cap_ptr) {
		target_if_err("dfs: reg cap null");
		return QDF_STATUS_E_FAILURE;
	}

	if (reg_cap_ptr[pdev_id].wireless_modes &
			WMI_HOST_REGDMN_MODE_11A)
		*is_5ghz = true;
	else
		*is_5ghz = false;

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_MCL_DFS_SUPPORT
/**
 * target_if_dfs_set_phyerr_filter_offload() - config phyerr filter offload.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_phyerr_filter_offload: Phyerr filter offload value.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS target_if_dfs_set_phyerr_filter_offload(
					struct wlan_objmgr_pdev *pdev,
					bool dfs_phyerr_filter_offload)
{
	QDF_STATUS status;
	void *wmi_handle;

	if (!pdev) {
		target_if_err("null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!wmi_handle) {
		target_if_err("null wmi_handle");
		return QDF_STATUS_E_FAILURE;
	}

	status = wmi_unified_dfs_phyerr_filter_offload_en_cmd(wmi_handle,
					dfs_phyerr_filter_offload);
	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("phyerr filter offload %d set fail: %d",
			      dfs_phyerr_filter_offload, status);

	return status;
}
#else
static QDF_STATUS target_if_dfs_set_phyerr_filter_offload(
					struct wlan_objmgr_pdev *pdev,
					bool dfs_phyerr_filter_offload)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * target_if_dfs_get_caps - get dfs caps.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_caps: Pointer to dfs_caps structure.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS target_if_dfs_get_caps(struct wlan_objmgr_pdev *pdev,
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

static QDF_STATUS target_send_dfs_offload_enable_cmd(
		struct wlan_objmgr_pdev *pdev, bool enable)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t pdev_id;
	void *wmi_hdl;

	if (!pdev) {
		target_if_err("null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_hdl = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!wmi_hdl) {
		target_if_err("null wmi_hdl");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	if (enable)
		status = wmi_unified_dfs_phyerr_offload_en_cmd(wmi_hdl,
							       pdev_id);
	else
		status = wmi_unified_dfs_phyerr_offload_dis_cmd(wmi_hdl,
								pdev_id);

	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("dfs: dfs offload cmd failed, enable:%d, pdev:%d",
			      enable, pdev_id);
	else
		target_if_debug("dfs: sent dfs offload cmd, enable:%d, pdev:%d",
				enable, pdev_id);

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

	dfs_tx_ops->dfs_process_emulate_bang_radar_cmd =
				&target_process_bang_radar_cmd;
	dfs_tx_ops->dfs_is_pdev_5ghz = &target_if_dfs_is_pdev_5ghz;
	dfs_tx_ops->dfs_send_offload_enable_cmd =
		&target_send_dfs_offload_enable_cmd;

	dfs_tx_ops->dfs_set_phyerr_filter_offload =
				&target_if_dfs_set_phyerr_filter_offload;

	dfs_tx_ops->dfs_get_caps = &target_if_dfs_get_caps;

	return QDF_STATUS_SUCCESS;
}
