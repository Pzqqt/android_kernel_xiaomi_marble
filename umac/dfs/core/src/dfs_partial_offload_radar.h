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
 * DOC: dfs_partial_offload_radar.h
 * This file contains partial offload specific dfs interfaces
 */

#ifndef _DFS_PARTIAL_OFFLOAD_RADAR_H_
#define _DFS_PARTIAL_OFFLOAD_RADAR_H_

/**
 * dfs_get_po_radars() - Initialize the RADAR table for PO.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD)
void dfs_get_po_radars(struct wlan_dfs *dfs);
#else
static void dfs_get_po_radars(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_send_avg_params_to_fw - send avg radar parameters to FW.
 * @dfs: Pointer to wlan_dfs structure.
 * @params: Pointer to dfs_radar_found_params.
 *
 * Return: None
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
void dfs_send_avg_params_to_fw(struct wlan_dfs *dfs,
			       struct dfs_radar_found_params *params);
#else
static inline
void dfs_send_avg_params_to_fw(struct wlan_dfs *dfs,
			       struct dfs_radar_found_params *params)
{
}
#endif

/**
 * dfs_host_wait_timer_init() - Initialize dfs host status wait timer.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
void dfs_host_wait_timer_init(struct wlan_dfs *dfs);
#else
static inline void dfs_host_wait_timer_init(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_set_override_status_timeout() - Change the dfs host status timeout.
 * @dfs: Pointer to wlan_dfs structure.
 * @status_timeout: timeout value.
 *
 * Return: QDF_STATUS
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS dfs_set_override_status_timeout(struct wlan_dfs *dfs,
				       int status_timeout);
#else
static inline QDF_STATUS dfs_set_override_status_timeout(struct wlan_dfs *dfs,
						     int status_timeout)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * dfs_get_override_status_timeout() - Get the dfs host status timeout value.
 * @dfs: Pointer to wlan_dfs structure.
 * @status_timeout: Pointer to  timeout value.
 *
 * Return: QDF_STATUS
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS dfs_get_override_status_timeout(struct wlan_dfs *dfs,
					   int *status_timeout);
#else
static inline
QDF_STATUS dfs_get_override_status_timeout(struct wlan_dfs *dfs,
					   int *status_timeout)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * dfs_radarfound_action_fcc() - The dfs action on radar detection by host for
 * FCC domain.
 * @dfs: Pointer to wlan_dfs structure.
 * @seg_id: segment id.
 * @false_radar_found: Indicates if false radar is found.
 *
 * Return: None
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
void dfs_radarfound_action_fcc(struct wlan_dfs *dfs, uint8_t seg_id,
			       int false_radar_found);
#else
static inline void dfs_radarfound_action_fcc(struct wlan_dfs *dfs,
					     uint8_t seg_id,
					     int false_radar_found)
{
}
#endif
#endif /*  _DFS_PARTIAL_OFFLOAD_RADAR_H_ */
