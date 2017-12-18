/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI) which are specific to DFS module.
 */

#ifndef _WMI_UNIFIED_DFS_API_H_
#define _WMI_UNIFIED_DFS_API_H_

#include <wlan_dfs_utils_api.h>

/**
 * wmi_extract_dfs_cac_complete_event() - function to handle cac complete event
 * @handle: wma handle
 * @event_buf: event buffer
 * @vdev_id: vdev id
 * @len: length of buffer
 *
 * Return: 0 for success or error code
 */
QDF_STATUS wmi_extract_dfs_cac_complete_event(void *wmi_hdl,
		uint8_t *evt_buf,
		uint32_t *vdev_id,
		uint32_t len);

/**
 * wmi_extract_dfs_radar_detection_event() - function to handle radar event
 * @handle: wma handle
 * @event_buf: event buffer
 * @radar_found: radar found event info
 * @vdev_id: vdev id
 * @len: length of buffer
 *
 * Return: 0 for success or error code
 */
QDF_STATUS wmi_extract_dfs_radar_detection_event(void *wmi_hdl,
		uint8_t *evt_buf,
		struct radar_found_info *radar_found,
		uint32_t len);

#ifdef QCA_MCL_DFS_SUPPORT
/**
 * wmi_extract_wlan_radar_event_info() - function to handle radar pulse event.
 * @wmi_hdl: wmi handle
 * @evt_buf: event buffer
 * @wlan_radar_event: pointer to radar event info structure
 * @len: length of buffer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_extract_wlan_radar_event_info(void *wmi_hdl,
		uint8_t *evt_buf,
		struct radar_event_info *wlan_radar_event,
		uint32_t len);
#endif
#endif /* _WMI_UNIFIED_DFS_API_H_ */
