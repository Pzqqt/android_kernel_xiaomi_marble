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
 * DOC: Implement API's specific to DFS component.
 */

#include <qdf_status.h>
#include <qdf_module.h>
#include <wmi_unified_api.h>
#include <wmi_unified_priv.h>
#include <wlan_dfs_utils_api.h>
#include <wmi_unified_dfs_api.h>

QDF_STATUS wmi_extract_dfs_cac_complete_event(void *wmi_hdl,
		uint8_t *evt_buf,
		uint32_t *vdev_id,
		uint32_t len)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle && wmi_handle->ops->extract_dfs_cac_complete_event)
		return wmi_handle->ops->extract_dfs_cac_complete_event(
				wmi_handle, evt_buf, vdev_id, len);

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(wmi_extract_dfs_cac_complete_event);

QDF_STATUS wmi_extract_dfs_radar_detection_event(void *wmi_hdl,
		uint8_t *evt_buf,
		struct radar_found_info *radar_found,
		uint32_t len)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle && wmi_handle->ops->extract_dfs_radar_detection_event)
		return wmi_handle->ops->extract_dfs_radar_detection_event(
				wmi_handle, evt_buf, radar_found, len);

	return QDF_STATUS_E_FAILURE;
}

#ifdef QCA_MCL_DFS_SUPPORT
QDF_STATUS wmi_extract_wlan_radar_event_info(void *wmi_hdl,
		uint8_t *evt_buf,
		struct radar_event_info *wlan_radar_event,
		uint32_t len)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)wmi_hdl;

	if (wmi_handle->ops->extract_wlan_radar_event_info)
		return wmi_handle->ops->extract_wlan_radar_event_info(
				wmi_handle, evt_buf, wlan_radar_event, len);

	return QDF_STATUS_E_FAILURE;
}
#endif
qdf_export_symbol(wmi_extract_dfs_radar_detection_event);
