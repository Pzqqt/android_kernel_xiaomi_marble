
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

#ifndef _WMI_UNIFIED_TWT_API_H_
#define _WMI_UNIFIED_TWT_API_H_

#include "wmi_unified_twt_param.h"


/**
 * wmi_unified_twt_enable_cmd() - Send WMI command to Enable TWT
 * @wmi_handle: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_twt_enable_cmd(wmi_unified_t wmi_handle,
			   struct wmi_twt_enable_param *params);

/**
 * wmi_unified_twt_disable_cmd() - Send WMI command to disable TWT
 * @wmi_handle: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_twt_disable_cmd(wmi_unified_t wmi_handle,
			    struct wmi_twt_disable_param *params);

/**
 * wmi_unified_twt_add_dialog_cmd() - Send WMI command to add TWT dialog
 * @wmi_handle: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_twt_add_dialog_cmd(wmi_unified_t wmi_handle,
			       struct wmi_twt_add_dialog_param *params);

/**
 * wmi_unified_twt_del_dialog_cmd() - Send WMI command to delete TWT dialog
 * @wmi_handle: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_twt_del_dialog_cmd(wmi_unified_t wmi_handle,
			       struct wmi_twt_del_dialog_param *params);

/**
 * wmi_unified_twt_pause_dialog_cmd() - Send WMI command to pause TWT dialog
 * @wmi_handle: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_twt_pause_dialog_cmd(wmi_unified_t wmi_handle,
				 struct wmi_twt_pause_dialog_cmd_param *params);

/**
 * wmi_unified_twt_resume_dialog_cmd() - Send WMI command to resume TWT dialog
 * @wmi_handle: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_twt_resume_dialog_cmd(
			wmi_unified_t wmi_handle,
			struct wmi_twt_resume_dialog_cmd_param *params);

#ifdef WLAN_SUPPORT_BCAST_TWT
/**
 * wmi_unified_twt_btwt_invite_sta_cmd() - Send WMI command for bTWT sta
 *                               invitation
 * @wmi_handle: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_twt_btwt_invite_sta_cmd(
			wmi_unified_t wmi_handle,
			struct wmi_twt_btwt_invite_sta_cmd_param *params);

/**
 * wmi_unified_twt_btwt_remove_sta_cmd() - Send WMI command for bTWT sta kickoff
 * @wmi_handle: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_twt_btwt_remove_sta_cmd(
			wmi_unified_t wmi_handle,
			struct wmi_twt_btwt_remove_sta_cmd_param *params);
#endif

/**
 * wmi_extract_twt_enable_comp_event() - Extract WMI event params for TWT enable
 *                               completion event
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_enable_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_enable_complete_event_param *params);

/**
 * wmi_extract_twt_disable_comp_event() - Extract WMI event params for TWT
 *                               disable completion event
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_disable_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_disable_complete_event *params);

/**
 * wmi_extract_twt_add_dialog_comp_event() - Extract WMI event params for TWT
 *                               add dialog completion event
 * @wmi_hdl: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_add_dialog_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_add_dialog_complete_event_param *params);

/**
 * wmi_extract_twt_add_dialog_comp_additional_params() - Extracts additional
 * twt parameters, as part of add dialog completion event
 * @wmi_hdl: wmi handle
 * @evt_buf: Pointer event buffer
 * @evt_buf_len: length of the add dialog event buffer
 * @idx: index of num_twt_params to extract
 * @additional_params: additional parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_add_dialog_comp_additional_params(
		wmi_unified_t wmi_handle, uint8_t *evt_buf,
		uint32_t evt_buf_len, uint32_t idx,
		struct wmi_twt_add_dialog_additional_params *additional_params);

/**
 * wmi_extract_twt_del_dialog_comp_event() - Extract WMI event params for TWT
 *                               delete dialog completion event
 * @wmi_hdl: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_del_dialog_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_del_dialog_complete_event_param *params);

/**
 * wmi_extract_twt_pause_dialog_comp_event() - Extract WMI event params for TWT
 *                               pause dialog completion event
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_pause_dialog_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_pause_dialog_complete_event_param *params);

/**
 * wmi_extract_twt_resume_dialog_comp_event() - Extract WMI event params for TWT
 *                               resume dialog completion event
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_resume_dialog_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_resume_dialog_complete_event_param *params);

#ifdef WLAN_SUPPORT_BCAST_TWT
/**
 * wmi_extract_twt_btwt_invite_sta_comp_event() - Extract WMI event params for
 *                          BTWT sta invitation completion event
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_btwt_invite_sta_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_btwt_invite_sta_complete_event_param *params);

/**
 * wmi_extract_twt_btwt_remove_sta_comp_event() - Extract WMI event params for
 *                          BTWT sta kickoff completion event
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_btwt_remove_sta_comp_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_btwt_remove_sta_complete_event_param *params);
#endif

/**
 * wmi_extract_twt_session_stats_event() - Extract WMI event params for TWT
 *                               session stats event
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_session_stats_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_session_stats_event_param *params);

/**
 * wmi_extract_twt_session_stats_data() - Extract one TWT session from TWT
 *                               session stats event
 * @wmi_handle: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 * @session: Session struct to save one TWT session
 * @idx: TWT session index
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_session_stats_data(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct wmi_twt_session_stats_event_param *params,
		struct wmi_host_twt_session_stats_info *session,
		uint32_t idx);

#ifdef WLAN_SUPPORT_TWT
void wmi_twt_attach_tlv(struct wmi_unified *wmi_handle);
#else
static void wmi_twt_attach_tlv(struct wmi_unified *wmi_handle)
{
	return;
}
#endif

#endif /* _WMI_UNIFIED_TWT_API_H_ */
