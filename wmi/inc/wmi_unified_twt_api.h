
/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * @wmi_hdl: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_twt_enable_cmd(void *wmi_hdl,
			struct wmi_twt_enable_param *params);

/**
 * wmi_unified_twt_disable_cmd() - Send WMI command to disable TWT
 * @wmi_hdl: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_twt_disable_cmd(void *wmi_hdl,
			struct wmi_twt_disable_param *params);

/**
 * wmi_unified_twt_add_dialog_cmd() - Send WMI command to add TWT dialog
 * @wmi_hdl: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_twt_add_dialog_cmd(void *wmi_hdl,
			struct wmi_twt_add_dialog_param *params);

/**
 * wmi_unified_twt_del_dialog_cmd() - Send WMI command to delete TWT dialog
 * @wmi_hdl: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_twt_del_dialog_cmd(void *wmi_hdl,
			struct wmi_twt_del_dialog_param *params);

/**
 * wmi_unified_twt_pause_dialog_cmd() - Send WMI command to pause TWT dialog
 * @wmi_hdl: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_twt_pause_dialog_cmd(void *wmi_hdl,
			struct wmi_twt_pause_dialog_cmd_param *params);

/**
 * wmi_unified_twt_resume_dialog_cmd() - Send WMI command to resume TWT dialog
 * @wmi_hdl: wmi handle
 * @params: Parameters to be configured
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_twt_resume_dialog_cmd(void *wmi_hdl,
			struct wmi_twt_resume_dialog_cmd_param *params);

/**
 * wmi_extract_twt_enable_comp_event() - Extract WMI event params for TWT enable
 *                               completion event
 * @wmi_hdl: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_enable_comp_event(void *wmi_hdl,
		uint8_t *evt_buf,
		struct wmi_twt_enable_complete_event_param *params);

/**
 * wmi_extract_twt_disable_comp_event() - Extract WMI event params for TWT
 *                               disable completion event
 * @wmi_hdl: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_disable_comp_event(void *wmi_hdl,
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
QDF_STATUS wmi_extract_twt_add_dialog_comp_event(void *wmi_hdl,
		uint8_t *evt_buf,
		struct wmi_twt_add_dialog_complete_event_param *params);

/**
 * wmi_extract_twt_del_dialog_comp_event() - Extract WMI event params for TWT
 *                               delete dialog completion event
 * @wmi_hdl: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_del_dialog_comp_event(void *wmi_hdl,
		uint8_t *evt_buf,
		struct wmi_twt_del_dialog_complete_event_param *params);

/**
 * wmi_extract_twt_pause_dialog_comp_event() - Extract WMI event params for TWT
 *                               pause dialog completion event
 * @wmi_hdl: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_pause_dialog_comp_event(void *wmi_hdl,
		uint8_t *evt_buf,
		struct wmi_twt_pause_dialog_complete_event_param *params);

/**
 * wmi_extract_twt_resume_dialog_comp_event() - Extract WMI event params for TWT
 *                               resume dialog completion event
 * @wmi_hdl: wmi handle
 * @evt_buf: Pointer event buffer
 * @params: Parameters to extract
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_twt_resume_dialog_comp_event(void *wmi_hdl,
		uint8_t *evt_buf,
		struct wmi_twt_resume_dialog_complete_event_param *params);

#ifdef WLAN_SUPPORT_TWT
void wmi_twt_attach_tlv(struct wmi_unified *wmi_handle);
#else
static void wmi_twt_attach_tlv(struct wmi_unified *wmi_handle)
{
	return;
}
#endif

#endif /* _WMI_UNIFIED_TWT_API_H_ */
