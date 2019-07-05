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
#include "wmi_unified_atf_param.h"
#include "qdf_module.h"

QDF_STATUS wmi_unified_set_bwf_cmd_send(wmi_unified_t wmi_handle,
					struct set_bwf_params *param)
{
	if (wmi_handle->ops->send_set_bwf_cmd)
		return wmi_handle->ops->send_set_bwf_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_ATF_ENABLE
QDF_STATUS
wmi_unified_set_atf_cmd_send(wmi_unified_t wmi_handle,
			     struct set_atf_params *param)
{
	if (wmi_handle->ops->send_set_atf_cmd)
		return wmi_handle->ops->send_set_atf_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_send_atf_peer_request_cmd(wmi_unified_t wmi_handle,
			      struct atf_peer_request_params *param)
{
	if (wmi_handle->ops->send_atf_peer_request_cmd)
		return wmi_handle->ops->send_atf_peer_request_cmd(wmi_handle,
								  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_send_set_atf_grouping_cmd(wmi_unified_t wmi_handle,
			      struct atf_grouping_params *param)
{
	if (wmi_handle->ops->send_set_atf_grouping_cmd)
		return wmi_handle->ops->send_set_atf_grouping_cmd(wmi_handle,
								  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_send_set_atf_group_ac_cmd(wmi_unified_t wmi_handle,
			      struct atf_group_ac_params *param)
{
	if (wmi_handle->ops->send_set_atf_group_ac_cmd)
		return wmi_handle->ops->send_set_atf_group_ac_cmd(wmi_handle,
								  param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_atf_peer_stats_ev(wmi_unified_t wmi_handle, void *evt_buf,
			      wmi_host_atf_peer_stats_event *ev)
{
	if (wmi_handle->ops->extract_atf_peer_stats_ev)
		return wmi_handle->ops->extract_atf_peer_stats_ev(
					wmi_handle, evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_extract_atf_token_info_ev(wmi_unified_t wmi_handle,
			      void *evt_buf, uint8_t idx,
			      wmi_host_atf_peer_stats_info *ev)
{
	if (wmi_handle->ops->extract_atf_token_info_ev)
		return wmi_handle->ops->extract_atf_token_info_ev(
					wmi_handle, evt_buf, idx, ev);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_ATF_ENABLE */
