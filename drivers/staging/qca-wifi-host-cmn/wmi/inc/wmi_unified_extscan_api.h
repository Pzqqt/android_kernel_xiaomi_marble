/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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

#ifndef _WMI_UNIFIED_EXTSCAN_API_H_
#define _WMI_UNIFIED_EXTSCAN_API_H_

/**
 * wmi_unified_reset_passpoint_network_list_cmd() - reset passpoint network list
 * @wmi_handle: wmi handle
 * @req: passpoint network request structure
 *
 * This function sends down WMI command with network id set to wildcard id.
 * firmware shall clear all the config entries
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_reset_passpoint_network_list_cmd(
		wmi_unified_t wmi_handle,
		struct wifi_passpoint_req_param *req);

/**
 * wmi_unified_set_passpoint_network_list_cmd() - set passpoint network list
 * @wmi_handle: wmi handle
 * @req: passpoint network request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and send down the passpoint configs down to the firmware
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */

QDF_STATUS wmi_unified_set_passpoint_network_list_cmd(
		wmi_unified_t wmi_handle,
		struct wifi_passpoint_req_param *req);

/** wmi_unified_set_epno_network_list_cmd() - set epno network list
 * @wmi_handle: wmi handle
 * @req: epno config params request structure
 *
 * This function reads the incoming epno config request structure
 * and constructs the WMI message to the firmware.
 *
 * Returns: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failures,
 *          error number otherwise
 */
QDF_STATUS wmi_unified_set_epno_network_list_cmd(
		wmi_unified_t wmi_handle,
		struct wifi_enhanced_pno_params *req);

/**
 * wmi_unified_extscan_get_capabilities_cmd() - extscan get capabilities
 * @wmi_handle: wmi handle
 * @pgetcapab: get capabilities params
 *
 * This function send request to fw to get extscan capabilities.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extscan_get_capabilities_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_capabilities_params *pgetcapab);

/**
 * wmi_unified_extscan_get_cached_results_cmd() - extscan get cached results
 * @wmi_handle: wmi handle
 * @pcached_results: cached results parameters
 *
 * This function send request to fw to get cached results.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extscan_get_cached_results_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_cached_result_params *pcached_results);

/**
 * wmi_unified_extscan_stop_change_monitor_cmd() - send stop change monitor cmd
 * @wmi_handle: wmi handle
 * @reset_req: Reset change request params
 *
 * This function sends stop change monitor request to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extscan_stop_change_monitor_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_capabilities_reset_params *reset_req);

/**
 * wmi_unified_extscan_start_change_monitor_cmd() - start change monitor cmd
 * @wmi_handle: wmi handle
 * @psigchange: change monitor request params
 *
 * This function sends start change monitor request to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extscan_start_change_monitor_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_set_sig_changereq_params *psigchange);

/**
 * wmi_unified_extscan_stop_hotlist_monitor_cmd() - stop hotlist monitor
 * @wmi_handle: wmi handle
 * @photlist_reset: hotlist reset params
 *
 * This function configures hotlist monitor to stop in fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extscan_stop_hotlist_monitor_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_bssid_hotlist_reset_params *photlist_reset);

/**
 * wmi_unified_extscan_start_hotlist_monitor_cmd() - start hotlist monitor
 * @wmi_handle: wmi handle
 * @params: hotlist params
 *
 * This function configures hotlist monitor to start in fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extscan_start_hotlist_monitor_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_bssid_hotlist_set_params *params);

/**
 * wmi_unified_stop_extscan_cmd() - stop extscan command to fw.
 * @wmi_handle: wmi handle
 * @pstopcmd: stop scan command request params
 *
 * This function sends stop extscan request to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_stop_extscan_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_stop_req_params *pstopcmd);

/**
 * wmi_unified_start_extscan_cmd() - start extscan command to fw.
 * @wmi_handle: wmi handle
 * @pstart: scan command request params
 *
 * This function sends start extscan request to fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure.
 */
QDF_STATUS wmi_unified_start_extscan_cmd(
		wmi_unified_t wmi_handle,
		struct wifi_scan_cmd_req_params *pstart);

#endif /* _WMI_UNIFIED_EXTSCAN_API_H_ */
