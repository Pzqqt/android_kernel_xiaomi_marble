/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
 * DOC: Implement API's specific to TWT component.
 */

#include "wmi_unified_priv.h"
#include "wmi_unified_twt_api.h"


QDF_STATUS
wmi_unified_twt_enable_cmd(wmi_unified_t wmi_handle,
			   struct wmi_twt_enable_param *params)
{
	if (wmi_handle->ops->send_twt_enable_cmd)
		return wmi_handle->ops->send_twt_enable_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_twt_disable_cmd(wmi_unified_t wmi_handle,
			    struct wmi_twt_disable_param *params)
{
	if (wmi_handle->ops->send_twt_disable_cmd)
		return wmi_handle->ops->send_twt_disable_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_twt_add_dialog_cmd(wmi_unified_t wmi_handle,
			       struct wmi_twt_add_dialog_param *params)
{
	if (wmi_handle->ops->send_twt_add_dialog_cmd)
		return wmi_handle->ops->send_twt_add_dialog_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_twt_del_dialog_cmd(wmi_unified_t wmi_handle,
			       struct wmi_twt_del_dialog_param *params)
{
	if (wmi_handle->ops->send_twt_del_dialog_cmd)
		return wmi_handle->ops->send_twt_del_dialog_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_twt_pause_dialog_cmd(wmi_unified_t wmi_handle,
				 struct wmi_twt_pause_dialog_cmd_param *params)
{
	if (wmi_handle->ops->send_twt_pause_dialog_cmd)
		return wmi_handle->ops->send_twt_pause_dialog_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_twt_resume_dialog_cmd(
			wmi_unified_t wmi_handle,
			struct wmi_twt_resume_dialog_cmd_param *params)
{
	if (wmi_handle->ops->send_twt_resume_dialog_cmd)
		return wmi_handle->ops->send_twt_resume_dialog_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_SUPPORT_BCAST_TWT
QDF_STATUS wmi_unified_twt_btwt_invite_sta_cmd(
			wmi_unified_t wmi_handle,
			struct wmi_twt_btwt_invite_sta_cmd_param *params)
{
	if (wmi_handle->ops->send_twt_btwt_invite_sta_cmd)
		return wmi_handle->ops->send_twt_btwt_invite_sta_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_twt_btwt_remove_sta_cmd(
			wmi_unified_t wmi_handle,
			struct wmi_twt_btwt_remove_sta_cmd_param *params)
{
	if (wmi_handle->ops->send_twt_btwt_remove_sta_cmd)
		return wmi_handle->ops->send_twt_btwt_remove_sta_cmd(
				wmi_handle, params);

	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wmi_extract_twt_enable_comp_event(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct wmi_twt_enable_complete_event_param *params)
{
	if (wmi_handle->ops->extract_twt_enable_comp_event)
		return wmi_handle->ops->extract_twt_enable_comp_event(
				wmi_handle, evt_buf, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_twt_disable_comp_event(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct wmi_twt_disable_complete_event *params)
{
	if (wmi_handle->ops->extract_twt_disable_comp_event)
		return wmi_handle->ops->extract_twt_disable_comp_event(
				wmi_handle, evt_buf, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_twt_add_dialog_comp_event(
			wmi_unified_t wmi_handle,
			uint8_t *evt_buf,
			struct wmi_twt_add_dialog_complete_event_param *params)
{
	if (wmi_handle->ops->extract_twt_add_dialog_comp_event)
		return wmi_handle->ops->extract_twt_add_dialog_comp_event(
				wmi_handle, evt_buf, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_twt_del_dialog_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_del_dialog_complete_event_param *params)
{
	if (wmi_handle->ops->extract_twt_del_dialog_comp_event)
		return wmi_handle->ops->extract_twt_del_dialog_comp_event(
				wmi_handle, evt_buf, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_twt_pause_dialog_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_pause_dialog_complete_event_param *params)
{
	if (wmi_handle->ops->extract_twt_pause_dialog_comp_event)
		return wmi_handle->ops->extract_twt_pause_dialog_comp_event(
				wmi_handle, evt_buf, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_twt_resume_dialog_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_resume_dialog_complete_event_param *params)
{
	if (wmi_handle->ops->extract_twt_resume_dialog_comp_event)
		return wmi_handle->ops->extract_twt_resume_dialog_comp_event(
				wmi_handle, evt_buf, params);

	return QDF_STATUS_E_FAILURE;
}

#ifdef WLAN_SUPPORT_BCAST_TWT
QDF_STATUS wmi_extract_twt_btwt_invite_sta_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_btwt_invite_sta_complete_event_param *params)
{
	if (wmi_handle->ops->extract_twt_btwt_invite_sta_comp_event)
		return wmi_handle->ops->extract_twt_btwt_invite_sta_comp_event(
				wmi_handle, evt_buf, params);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_extract_twt_btwt_remove_sta_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_btwt_remove_sta_complete_event_param *params)
{
	if (wmi_handle->ops->extract_twt_btwt_remove_sta_comp_event)
		return wmi_handle->ops->extract_twt_btwt_remove_sta_comp_event(
				wmi_handle, evt_buf, params);

	return QDF_STATUS_E_FAILURE;
}
#endif
