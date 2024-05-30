/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: Implement API's specific to interop issues ap component.
 */

#ifndef _WMI_UNIFIED_INTEROP_ISSUES_AP_API_H_
#define _WMI_UNIFIED_INTEROP_ISSUES_AP_API_H_

#include <wmi_unified_param.h>
#include <wlan_interop_issues_ap_public_structs.h>

/**
 * wmi_extract_interop_issues_ap_ev_param() - extract info from event
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer
 * @param: pointer to interop issues ap event structure
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_interop_issues_ap_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
				   struct wlan_interop_issues_ap_event *param);
/**
 * wmi_unified_set_rap_ps_cmd() - set interop issues ap for ps in fw
 * @wmi_handle: wmi handle
 * @rap: interop issues ap info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_rap_ps_cmd(wmi_unified_t wmi_handle,
				      struct wlan_interop_issues_ap_info *rap);
#endif
