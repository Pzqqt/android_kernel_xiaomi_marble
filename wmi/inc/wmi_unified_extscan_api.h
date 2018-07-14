/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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

QDF_STATUS wmi_unified_reset_passpoint_network_list_cmd(void *wmi_hdl,
					struct wifi_passpoint_req_param *req);

QDF_STATUS wmi_unified_set_passpoint_network_list_cmd(void *wmi_hdl,
					struct wifi_passpoint_req_param *req);

QDF_STATUS wmi_unified_set_epno_network_list_cmd(void *wmi_hdl,
		struct wifi_enhanced_pno_params *req);

QDF_STATUS wmi_unified_extscan_get_capabilities_cmd(void *wmi_hdl,
			  struct extscan_capabilities_params *pgetcapab);

QDF_STATUS wmi_unified_extscan_get_cached_results_cmd(void *wmi_hdl,
			  struct extscan_cached_result_params *pcached_results);

QDF_STATUS wmi_unified_extscan_stop_change_monitor_cmd(void *wmi_hdl,
			  struct extscan_capabilities_reset_params *reset_req);

QDF_STATUS wmi_unified_extscan_start_change_monitor_cmd(void *wmi_hdl,
				   struct extscan_set_sig_changereq_params *
					   psigchange);

QDF_STATUS wmi_unified_extscan_stop_hotlist_monitor_cmd(void *wmi_hdl,
		  struct extscan_bssid_hotlist_reset_params *photlist_reset);

/**
 * wmi_unified_extscan_start_hotlist_monitor_cmd() - start hotlist monitor
 * @wmi_hdl: wmi handle
 * @params: hotlist params
 *
 * This function configures hotlist monitor to start in fw.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_extscan_start_hotlist_monitor_cmd(void *wmi_hdl,
			struct extscan_bssid_hotlist_set_params *params);

QDF_STATUS wmi_unified_stop_extscan_cmd(void *wmi_hdl,
			  struct extscan_stop_req_params *pstopcmd);

QDF_STATUS wmi_unified_start_extscan_cmd(void *wmi_hdl,
			  struct wifi_scan_cmd_req_params *pstart);

#endif /* _WMI_UNIFIED_EXTSCAN_API_H_ */
