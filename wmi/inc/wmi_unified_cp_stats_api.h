/*
 * Copyright (c) 2013-2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/**
 * DOC: Implement API's specific to cp stats component.
 */

#ifndef _WMI_UNIFIED_CP_STATS_API_H_
#define _WMI_UNIFIED_CP_STATS_API_H_

#ifdef QCA_SUPPORT_MC_CP_STATS
#include <wmi_unified_mc_cp_stats_api.h>
#endif

#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS
/**
 * wmi_unified_infra_cp_stats_request_send() - WMI request infra_cp_stats
 * function
 * @wmi_handle: wmi handle
 * @param: pointer to hold infra_cp_stats request parameters
 *
 * Return: QDF_STATUS_SUCCESS on Success, other QDF_STATUS error
 *         codes on failure
 */
QDF_STATUS
wmi_unified_infra_cp_stats_request_send(
				wmi_unified_t wmi_handle,
				struct infra_cp_stats_cmd_info *param);

/**
 * wmi_unified_extract_infra_cp_stats() - extract various infra cp statistics
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer
 * @evt_buf_len: length of event buffer
 * @params: pointer to store the extracted event info
 *
 * This function extracts the infra cp statistics from the event
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_extract_infra_cp_stats(wmi_unified_t wmi_handle,
				   void *evt_buf, uint32_t evt_buf_len,
				   struct infra_cp_stats_event *params);

QDF_STATUS wmi_stats_handler(void *buff, int32_t len,
			     struct infra_cp_stats_event *params);

QDF_STATUS
extract_infra_cp_stats_tlv(wmi_unified_t wmi_handle, void *evt_buf,
			   uint32_t evt_buf_len,
			   struct infra_cp_stats_event *params);
#endif /* WLAN_SUPPORT_INFRA_CTRL_PATH_STATS */

/**
 * wmi_unified_stats_request_send() - WMI request stats function
 * @wmi_handle: handle to WMI
 * @macaddr: MAC address
 * @param: pointer to hold stats request parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_stats_request_send(wmi_unified_t wmi_handle,
					  uint8_t macaddr[QDF_MAC_ADDR_SIZE],
					  struct stats_request_params *param);

#ifdef WLAN_FEATURE_BIG_DATA_STATS
/**
 * wmi_unified_big_data_stats_request_send() - WMI request big data stats
 * function
 * @wmi_handle: handle to WMI
 * @param: pointer to hold stats request parameter
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_big_data_stats_request_send(wmi_unified_t wmi_handle,
					struct stats_request_params *param);
#endif

/**
 * wmi_extract_stats_param() - extract all stats count from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @stats_param: Pointer to hold stats count
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_stats_param(wmi_unified_t wmi_handle, void *evt_buf,
			wmi_host_stats_event *stats_param);

/**
 * wmi_extract_pdev_stats() - extract pdev stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into pdev stats
 * @pdev_stats: Pointer to hold pdev stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_pdev_stats(wmi_unified_t wmi_handle, void *evt_buf,
		       uint32_t index, wmi_host_pdev_stats *pdev_stats);

/**
 * wmi_extract_vdev_stats() - extract vdev stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into vdev stats
 * @vdev_stats: Pointer to hold vdev stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_vdev_stats(wmi_unified_t wmi_handle, void *evt_buf,
		       uint32_t index, wmi_host_vdev_stats *vdev_stats);

/**
 * wmi_extract_peer_stats() - extract peer stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into peer stats
 * @peer_stats: Pointer to hold peer stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_peer_stats(wmi_unified_t wmi_handle, void *evt_buf,
		       uint32_t index, wmi_host_peer_stats *peer_stats);

/**
 * wmi_extract_peer_extd_stats() - extract extended peer stats from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @index: Index into extended peer stats
 * @peer_extd_stats: Pointer to hold extended peer stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_peer_extd_stats(wmi_unified_t wmi_handle, void *evt_buf,
			    uint32_t index,
			    wmi_host_peer_extd_stats *peer_extd_stats);
#endif /* _WMI_UNIFIED_CP_STATS_API_H_ */
