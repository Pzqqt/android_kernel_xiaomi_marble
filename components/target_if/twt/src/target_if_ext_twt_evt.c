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
	return 0;
}

static int
target_if_twt_teardown_complete_event_handler(ol_scn_t scn, uint8_t *event,
					      uint32_t len)
{
	return 0;
}

static int
target_if_twt_pause_complete_event_handler(ol_scn_t scn, uint8_t *event,
					   uint32_t len)
{
	return 0;
}

static int
target_if_twt_resume_complete_event_handler(ol_scn_t scn, uint8_t *event,
					    uint32_t len)
{
	return 0;
}

static int
target_if_twt_nudge_complete_event_handler(ol_scn_t scn, uint8_t *event,
					   uint32_t len)
{
	return 0;
}

static int
target_if_twt_notify_event_handler(ol_scn_t scn, uint8_t *event,
				   uint32_t len)
{
	return 0;
}

static int
target_if_twt_ack_complete_event_handler(ol_scn_t scn, uint8_t *event,
					 uint32_t len)
{
	return 0;
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

