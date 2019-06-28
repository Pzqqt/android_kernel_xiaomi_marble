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
 * DOC: Implement API's specific to DFS component.
 */

#include <qdf_status.h>
#include <qdf_module.h>
#include <wmi_unified_api.h>
#include <wmi_unified_priv.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_dfs_utils_api.h>
#include <wmi_unified_dfs_api.h>
#include <init_deinit_lmac.h>

QDF_STATUS wmi_extract_dfs_cac_complete_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		uint32_t *vdev_id,
		uint32_t len)
{
	if (wmi_handle && wmi_handle->ops->extract_dfs_cac_complete_event)
		return wmi_handle->ops->extract_dfs_cac_complete_event(
				wmi_handle, evt_buf, vdev_id, len);

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(wmi_extract_dfs_cac_complete_event);

QDF_STATUS
wmi_extract_dfs_ocac_complete_event(wmi_unified_t wmi_handle,
				    uint8_t *evt_buf,
				    struct vdev_adfs_complete_status *param)
{
	if (wmi_handle && wmi_handle->ops->extract_dfs_ocac_complete_event)
		return wmi_handle->ops->extract_dfs_ocac_complete_event(
				wmi_handle, evt_buf, param);

	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(wmi_extract_dfs_ocac_complete_event);

QDF_STATUS wmi_extract_dfs_radar_detection_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct radar_found_info *radar_found,
		uint32_t len)
{
	if (wmi_handle && wmi_handle->ops->extract_dfs_radar_detection_event)
		return wmi_handle->ops->extract_dfs_radar_detection_event(
				wmi_handle, evt_buf, radar_found, len);

	return QDF_STATUS_E_FAILURE;
}

#ifdef QCA_MCL_DFS_SUPPORT
QDF_STATUS wmi_extract_wlan_radar_event_info(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct radar_event_info *wlan_radar_event,
		uint32_t len)
{
	if (wmi_handle->ops->extract_wlan_radar_event_info)
		return wmi_handle->ops->extract_wlan_radar_event_info(
				wmi_handle, evt_buf, wlan_radar_event, len);

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(wmi_extract_dfs_radar_detection_event);
#endif

#if defined(WLAN_DFS_FULL_OFFLOAD) && defined(QCA_DFS_NOL_OFFLOAD)
QDF_STATUS wmi_send_usenol_pdev_param(wmi_unified_t wmi_handle, bool usenol,
				      struct wlan_objmgr_pdev *pdev)
{
	struct pdev_params pparam;
	int pdev_idx;

	pdev_idx = lmac_get_pdev_idx(pdev);
	if (pdev_idx < 0)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_zero(&pparam, sizeof(pparam));
	pparam.param_id = wmi_pdev_param_use_nol;
	pparam.param_value = usenol;

	return wmi_unified_pdev_param_send(wmi_handle, &pparam, pdev_idx);
}

QDF_STATUS
wmi_send_subchan_marking_pdev_param(wmi_unified_t wmi_handle,
				    bool subchanmark,
				    struct wlan_objmgr_pdev *pdev)
{
	struct pdev_params pparam;
	int pdev_idx;

	pdev_idx = lmac_get_pdev_idx(pdev);
	if (pdev_idx < 0)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_zero(&pparam, sizeof(pparam));
	pparam.param_id = wmi_pdev_param_sub_channel_marking;
	pparam.param_value = subchanmark;

	return wmi_unified_pdev_param_send(wmi_handle, &pparam, pdev_idx);
}

#endif
