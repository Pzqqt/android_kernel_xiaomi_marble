/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#include "wmi_unified_priv.h"

QDF_STATUS wmi_unified_reset_passpoint_network_list_cmd(
	wmi_unified_t wmi_handle,
	struct wifi_passpoint_req_param *req)
{
	if (wmi_handle->ops->send_reset_passpoint_network_list_cmd)
		return wmi_handle->ops->send_reset_passpoint_network_list_cmd(wmi_handle,
			    req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_passpoint_network_list_cmd(
		wmi_unified_t wmi_handle,
		struct wifi_passpoint_req_param *req)
{
	if (wmi_handle->ops->send_set_passpoint_network_list_cmd)
		return wmi_handle->ops->send_set_passpoint_network_list_cmd(wmi_handle,
			    req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_epno_network_list_cmd(
		wmi_unified_t wmi_handle,
		struct wifi_enhanced_pno_params *req)
{
	if (wmi_handle->ops->send_set_epno_network_list_cmd)
		return wmi_handle->ops->send_set_epno_network_list_cmd(wmi_handle,
			    req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extscan_get_capabilities_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_capabilities_params *pgetcapab)
{
	if (wmi_handle->ops->send_extscan_get_capabilities_cmd)
		return wmi_handle->ops->send_extscan_get_capabilities_cmd(wmi_handle,
			    pgetcapab);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extscan_get_cached_results_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_cached_result_params *pcached_results)
{
	if (wmi_handle->ops->send_extscan_get_cached_results_cmd)
		return wmi_handle->ops->send_extscan_get_cached_results_cmd(wmi_handle,
			    pcached_results);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extscan_stop_change_monitor_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_capabilities_reset_params *reset_req)
{
	if (wmi_handle->ops->send_extscan_stop_change_monitor_cmd)
		return wmi_handle->ops->send_extscan_stop_change_monitor_cmd(wmi_handle,
			    reset_req);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extscan_start_change_monitor_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_set_sig_changereq_params *psigchange)
{
	if (wmi_handle->ops->send_extscan_start_change_monitor_cmd)
		return wmi_handle->ops->send_extscan_start_change_monitor_cmd(wmi_handle,
			    psigchange);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extscan_stop_hotlist_monitor_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_bssid_hotlist_reset_params *photlist_reset)
{
	if (wmi_handle->ops->send_extscan_stop_hotlist_monitor_cmd)
		return wmi_handle->ops->send_extscan_stop_hotlist_monitor_cmd(wmi_handle,
			    photlist_reset);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_extscan_start_hotlist_monitor_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_bssid_hotlist_set_params *params)
{
	if (wmi_handle->ops->send_extscan_start_hotlist_monitor_cmd)
		return wmi_handle->ops->send_extscan_start_hotlist_monitor_cmd(wmi_handle,
									       params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_stop_extscan_cmd(
		wmi_unified_t wmi_handle,
		struct extscan_stop_req_params *pstopcmd)
{
	if (wmi_handle->ops->send_stop_extscan_cmd)
		return wmi_handle->ops->send_stop_extscan_cmd(wmi_handle,
			    pstopcmd);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_start_extscan_cmd(
		wmi_unified_t wmi_handle,
		struct wifi_scan_cmd_req_params *pstart)
{
	if (wmi_handle->ops->send_start_extscan_cmd)
		return wmi_handle->ops->send_start_extscan_cmd(wmi_handle,
			    pstart);

	return QDF_STATUS_E_FAILURE;
}
