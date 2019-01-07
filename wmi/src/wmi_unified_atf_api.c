/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

/**
 *  wmi_unified_set_bwf_cmd_send() - WMI set bwf function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to set bwf param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_bwf_cmd_send(void *wmi_hdl,
				struct set_bwf_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) wmi_hdl;

	if (wmi_handle->ops->send_set_bwf_cmd)
		return wmi_handle->ops->send_set_bwf_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_ATF_ENABLE
/**
 *  wmi_unified_set_atf_cmd_send() - WMI set atf function
 *  @param wmi_handle      : handle to WMI.
 *  @param param    : pointer to set atf param
 *
 *  @return QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_atf_cmd_send(void *wmi_hdl,
			     struct set_atf_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_set_atf_cmd)
		return wmi_handle->ops->send_set_atf_cmd(wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_send_atf_peer_request_cmd() - send atf peer request command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to atf peer request param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_atf_peer_request_cmd(void *wmi_hdl,
			      struct atf_peer_request_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_atf_peer_request_cmd)
		return wmi_handle->ops->send_atf_peer_request_cmd(wmi_handle,
								  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_send_set_atf_grouping_cmd() - send set atf grouping command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set atf grouping param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_set_atf_grouping_cmd(void *wmi_hdl,
			      struct atf_grouping_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_set_atf_grouping_cmd)
		return wmi_handle->ops->send_set_atf_grouping_cmd(wmi_handle,
								  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_send_set_atf_group_ac_cmd() - send set atf AC command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set atf AC group param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_set_atf_group_ac_cmd(void *wmi_hdl,
			      struct atf_group_ac_params *param)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)wmi_hdl;

	if (wmi_handle->ops->send_set_atf_group_ac_cmd)
		return wmi_handle->ops->send_set_atf_group_ac_cmd(wmi_handle,
								  param);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_atf_peer_stats_ev() - extract atf peer stats
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param ev: Pointer to hold atf peer stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_atf_peer_stats_ev(void *wmi_hdl, void *evt_buf,
			      wmi_host_atf_peer_stats_event *ev)
{
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (wmi->ops->extract_atf_peer_stats_ev)
		return wmi->ops->extract_atf_peer_stats_ev(wmi, evt_buf, ev);

	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_extract_atf_token_info_ev() - extract atf token info
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param idx: Index indicating the peer number
 * @param ev: Pointer to hold atf token info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_atf_token_info_ev(void *wmi_hdl, void *evt_buf, uint8_t idx,
			      wmi_host_atf_peer_stats_info *ev)
{
	wmi_unified_t wmi = (wmi_unified_t)wmi_hdl;

	if (wmi->ops->extract_atf_token_info_ev)
		return wmi->ops->extract_atf_token_info_ev(wmi, evt_buf,
							   idx, ev);

	return QDF_STATUS_E_FAILURE;
}
#endif /* WLAN_ATF_ENABLE */
