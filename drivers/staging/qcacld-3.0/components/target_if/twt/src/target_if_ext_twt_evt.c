/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 *  DOC: target_if_ext_twt_evt.c
 *  This file contains twt component's target related function definitions
 */
#include <qdf_util.h>
#include <wlan_twt_public_structs.h>
#include <wlan_lmac_if_def.h>
#include <target_if.h>
#include <target_if_ext_twt.h>
#include <wlan_twt_api.h>
#include "twt/core/src/wlan_twt_main.h"

static int
target_if_twt_setup_complete_event_handler(ol_scn_t scn, uint8_t *event,
					   uint32_t len)
{
	QDF_STATUS qdf_status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct twt_add_dialog_complete_event *data;
	struct wlan_lmac_if_twt_rx_ops *twt_rx_ops;

	TARGET_IF_ENTER();

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	twt_rx_ops = wlan_twt_get_rx_ops(psoc);
	if (!twt_rx_ops || !twt_rx_ops->twt_setup_comp_cb) {
		target_if_err("No valid twt setup complete rx ops");
		return -EINVAL;
	}

	data = qdf_mem_malloc(sizeof(*data));
	if (!data)
		return -ENOMEM;

	qdf_status = wmi_extract_twt_add_dialog_comp_event(wmi_handle,
						event, &data->params);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		target_if_err("extract twt add dialog event failed (status=%d)",
			      qdf_status);
		goto done;
	}

	if (data->params.num_additional_twt_params) {
		qdf_status = wmi_extract_twt_add_dialog_comp_additional_params(
						wmi_handle, event, len, 0,
						&data->additional_params);
		if (QDF_IS_STATUS_ERROR(qdf_status))
			goto done;
	}

	qdf_status = twt_rx_ops->twt_setup_comp_cb(psoc, data);

done:
	qdf_mem_free(data);
	return qdf_status_to_os_return(qdf_status);
}

static int
target_if_twt_teardown_complete_event_handler(ol_scn_t scn, uint8_t *event,
					      uint32_t len)
{
	QDF_STATUS qdf_status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct twt_del_dialog_complete_event_param *data;
	struct wlan_lmac_if_twt_rx_ops *twt_rx_ops;

	TARGET_IF_ENTER();

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	twt_rx_ops = wlan_twt_get_rx_ops(psoc);
	if (!twt_rx_ops || !twt_rx_ops->twt_teardown_comp_cb) {
		target_if_err("No valid twt teardown complete rx ops");
		return -EINVAL;
	}

	data = qdf_mem_malloc(sizeof(*data));
	if (!data)
		return -ENOMEM;

	qdf_status = wmi_extract_twt_del_dialog_comp_event(wmi_handle,
							   event, data);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		target_if_err("extract twt del dialog event failed (status=%d)",
			      qdf_status);
		goto done;
	}

	qdf_status = twt_rx_ops->twt_teardown_comp_cb(psoc, data);

done:
	qdf_mem_free(data);
	return qdf_status_to_os_return(qdf_status);

}

/**
 * target_if_twt_pause_complete_event_handler - TWT pause complete handler
 * @scn: scn
 * @event: buffer with event
 * @len: buffer length
 *
 * Return: 0 on success, negative value on failure
 */
static int
target_if_twt_pause_complete_event_handler(ol_scn_t scn, uint8_t *event,
					   uint32_t len)
{
	QDF_STATUS qdf_status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct twt_pause_dialog_complete_event_param *param;
	struct wlan_lmac_if_twt_rx_ops *twt_rx_ops;

	TARGET_IF_ENTER();

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	twt_rx_ops = wlan_twt_get_rx_ops(psoc);
	if (!twt_rx_ops || !twt_rx_ops->twt_pause_comp_cb) {
		target_if_err("No valid twt pause complete rx ops");
		return -EINVAL;
	}

	param = qdf_mem_malloc(sizeof(*param));
	if (!param)
		return -ENOMEM;

	qdf_status = wmi_extract_twt_pause_dialog_comp_event(wmi_handle,
							     event, param);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		target_if_err("extract twt pause dialog event failed (status=%d)",
			     qdf_status);
		goto done;
	}

	qdf_status = twt_rx_ops->twt_pause_comp_cb(psoc, param);

done:
	qdf_mem_free(param);
	return qdf_status_to_os_return(qdf_status);
}

/**
 * target_if_twt_resume_dialog_complete_event_handler - TWT resume dlg
 * complete evt handler
 * @scn: scn
 * @event: buffer with event
 * @len: buffer length
 *
 * Return: 0 on success, negative value on failure
 */
static int
target_if_twt_resume_complete_event_handler(ol_scn_t scn, uint8_t *event,
					    uint32_t len)
{
	QDF_STATUS qdf_status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct twt_resume_dialog_complete_event_param *param;
	struct wlan_lmac_if_twt_rx_ops *twt_rx_ops;

	TARGET_IF_ENTER();

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	twt_rx_ops = wlan_twt_get_rx_ops(psoc);
	if (!twt_rx_ops || !twt_rx_ops->twt_resume_comp_cb) {
		target_if_err("No valid twt resume complete rx ops");
		return -EINVAL;
	}

	param = qdf_mem_malloc(sizeof(*param));
	if (!param)
		return -ENOMEM;

	qdf_status = wmi_extract_twt_resume_dialog_comp_event(wmi_handle,
							      event, param);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		target_if_err("extract twt resume event failed (status=%d)",
			      qdf_status);
		goto done;
	}

	qdf_status = twt_rx_ops->twt_resume_comp_cb(psoc, param);

done:
	qdf_mem_free(param);
	return qdf_status_to_os_return(qdf_status);
}

/**
 * target_if_twt_nudge_dialog_complete_event_handler - TWT nudge dlg
 * complete evt handler
 * @scn: scn
 * @event: buffer with event
 * @len: buffer length
 *
 * Return: 0 on success, negative value on failure
 */
static int
target_if_twt_nudge_complete_event_handler(ol_scn_t scn, uint8_t *event,
					   uint32_t len)
{
	QDF_STATUS qdf_status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct twt_nudge_dialog_complete_event_param *param;
	struct wlan_lmac_if_twt_rx_ops *twt_rx_ops;

	TARGET_IF_ENTER();

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	twt_rx_ops = wlan_twt_get_rx_ops(psoc);
	if (!twt_rx_ops || !twt_rx_ops->twt_nudge_comp_cb) {
		target_if_err("No valid twt nudge complete rx ops");
		return -EINVAL;
	}

	param = qdf_mem_malloc(sizeof(*param));
	if (!param)
		return -ENOMEM;

	qdf_status = wmi_extract_twt_nudge_dialog_comp_event(wmi_handle,
							     event, param);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		target_if_err("extract twt nudge event failed (status=%d)",
			      qdf_status);
		goto done;
	}

	qdf_status = twt_rx_ops->twt_nudge_comp_cb(psoc, param);

done:
	qdf_mem_free(param);
	return qdf_status_to_os_return(qdf_status);
}

static int
target_if_twt_notify_event_handler(ol_scn_t scn, uint8_t *event,
				   uint32_t len)
{
	QDF_STATUS qdf_status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct twt_notify_event_param *data;
	struct wlan_lmac_if_twt_rx_ops *twt_rx_ops;

	TARGET_IF_ENTER();

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	twt_rx_ops = wlan_twt_get_rx_ops(psoc);
	if (!twt_rx_ops || !twt_rx_ops->twt_notify_comp_cb) {
		target_if_err("No valid twt notify rx ops");
		return -EINVAL;
	}

	data = qdf_mem_malloc(sizeof(*data));
	if (!data)
		return -ENOMEM;

	qdf_status = wmi_extract_twt_notify_event(wmi_handle, event, data);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		target_if_err("extract twt notify event failed (status=%d)",
			      qdf_status);
		goto done;
	}

	qdf_status = twt_rx_ops->twt_notify_comp_cb(psoc, data);

done:
	qdf_mem_free(data);
	return qdf_status_to_os_return(qdf_status);

}

static int
target_if_twt_ack_complete_event_handler(ol_scn_t scn, uint8_t *event,
					 uint32_t len)
{
	QDF_STATUS qdf_status;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct twt_ack_complete_event_param *data;
	struct wlan_lmac_if_twt_rx_ops *twt_rx_ops;

	TARGET_IF_ENTER();

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	twt_rx_ops = wlan_twt_get_rx_ops(psoc);
	if (!twt_rx_ops || !twt_rx_ops->twt_ack_comp_cb) {
		target_if_err("No valid twt ack rx ops");
		return -EINVAL;
	}

	data = qdf_mem_malloc(sizeof(*data));
	if (!data)
		return -ENOMEM;

	qdf_status = wmi_extract_twt_ack_comp_event(wmi_handle, event, data);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		target_if_err("extract twt ack event failed (status=%d)",
			      qdf_status);
		goto done;
	}

	qdf_status = twt_rx_ops->twt_ack_comp_cb(psoc, data);

done:
	qdf_mem_free(data);
	return qdf_status_to_os_return(qdf_status);
}

QDF_STATUS
target_if_twt_register_ext_events(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		target_if_err("psoc obj is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null!");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_register_event_handler
			(wmi_handle,
			 wmi_twt_add_dialog_complete_event_id,
			 target_if_twt_setup_complete_event_handler,
			 WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to register twt add dialog event cb");
		return status;
	}

	status = wmi_unified_register_event_handler
			(wmi_handle,
			 wmi_twt_del_dialog_complete_event_id,
			 target_if_twt_teardown_complete_event_handler,
			 WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to register twt del dialog event cb");
		return status;
	}

	status = wmi_unified_register_event_handler
			(wmi_handle,
			 wmi_twt_pause_dialog_complete_event_id,
			 target_if_twt_pause_complete_event_handler,
			 WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to register twt pause dialog event cb");
		return status;
	}

	status = wmi_unified_register_event_handler
			(wmi_handle,
			 wmi_twt_resume_dialog_complete_event_id,
			 target_if_twt_resume_complete_event_handler,
			 WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to register twt resume dialog event cb");
		return status;
	}

	status = wmi_unified_register_event_handler
			(wmi_handle,
			 wmi_twt_nudge_dialog_complete_event_id,
			 target_if_twt_nudge_complete_event_handler,
			 WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to register twt nudge dialog event cb");
		return status;
	}

	status = wmi_unified_register_event_handler
				(wmi_handle,
				 wmi_twt_notify_event_id,
				 target_if_twt_notify_event_handler,
				 WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to register twt notify event cb");
		return status;
	}

	status = wmi_unified_register_event_handler
				(wmi_handle,
				 wmi_twt_ack_complete_event_id,
				 target_if_twt_ack_complete_event_handler,
				 WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to register twt ack event cb");
		return status;
	}

	return status;
}

QDF_STATUS
target_if_twt_deregister_ext_events(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		target_if_err("psoc is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = wmi_unified_unregister_event_handler(wmi_handle,
					wmi_twt_add_dialog_complete_event_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to deregister twt add dialog event cb");
		return status;
	}

	status = wmi_unified_unregister_event_handler(wmi_handle,
					wmi_twt_del_dialog_complete_event_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to deregister twt del dialog event cb");
		return status;
	}

	status = wmi_unified_unregister_event_handler(wmi_handle,
					wmi_twt_pause_dialog_complete_event_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to deregister twt pause dialog event cb");
		return status;
	}

	status = wmi_unified_unregister_event_handler(wmi_handle,
				wmi_twt_resume_dialog_complete_event_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to deregister twt resume dialog event");
		return status;
	}

	status = wmi_unified_unregister_event_handler(wmi_handle,
					wmi_twt_nudge_dialog_complete_event_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to deregister twt nudge dialog event cb");
		return status;
	}

	status = wmi_unified_unregister_event_handler(wmi_handle,
					wmi_twt_notify_event_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to deregister twt notify event cb");
		return status;
	}

	status = wmi_unified_unregister_event_handler(wmi_handle,
					wmi_twt_ack_complete_event_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to deregister twt ack complete event cb");
		return status;
	}

	return status;
}

