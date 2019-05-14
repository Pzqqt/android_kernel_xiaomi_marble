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

#include <wmi_unified_priv.h>
#include <wmi_unified_interop_issues_ap_api.h>

QDF_STATUS
wmi_extract_interop_issues_ap_ev_param(wmi_unified_t wmi_handle, void *evt_buf,
				    struct wlan_interop_issues_ap_event *param)
{
	if (wmi_handle->ops->extract_interop_issues_ap_ev_param)
		return wmi_handle->ops->extract_interop_issues_ap_ev_param(
						wmi_handle, evt_buf, param);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_rap_ps_cmd(wmi_unified_t wmi_handle,
				      struct wlan_interop_issues_ap_info *rap)
{
	if (wmi_handle->ops->send_set_rap_ps_cmd)
		return wmi_handle->ops->send_set_rap_ps_cmd(wmi_handle, rap);
	return QDF_STATUS_E_FAILURE;
}
