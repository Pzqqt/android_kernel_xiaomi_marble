/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 *  DOC: target_if_twt.c
 *  This file contains twt component's target related function definitions
 */
#include <target_if_twt.h>
#include <target_if_twt_cmd.h>
#include <target_if_twt_evt.h>
#include <target_if_ext_twt.h>
#include "wlan_twt_common.h"
#include "wlan_twt_priv.h"

QDF_STATUS
target_if_twt_register_events(struct wlan_objmgr_psoc *psoc)
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

	status = wmi_unified_register_event_handler(wmi_handle,
					wmi_twt_enable_complete_event_id,
					target_if_twt_en_complete_event_handler,
					WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to register twt enable event cb");
		return status;
	}

	status = wmi_unified_register_event_handler(wmi_handle,
				wmi_twt_disable_complete_event_id,
				target_if_twt_disable_comp_event_handler,
				WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to register twt disable event cb");
		return status;
	}

	status = target_if_twt_register_ext_events(psoc);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to register twt ext events");
		return status;
	}

	return status;
}

QDF_STATUS
target_if_twt_deregister_events(struct wlan_objmgr_psoc *psoc)
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
					wmi_twt_enable_complete_event_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to deregister twt enable event cb");
		return status;
	}

	status = wmi_unified_unregister_event_handler(wmi_handle,
					 wmi_twt_disable_complete_event_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to deregister twt disable event cb");
		return status;
	}

	status = target_if_twt_deregister_ext_events(psoc);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to deregister twt ext events");
		return status;
	}

	return status;
}

QDF_STATUS
target_if_twt_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_twt_tx_ops *twt_tx_ops;
	QDF_STATUS status;

	if (!tx_ops) {
		target_if_err("txops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	twt_tx_ops = &tx_ops->twt_tx_ops;
	twt_tx_ops->enable_req = target_if_twt_enable_req;
	twt_tx_ops->disable_req = target_if_twt_disable_req;
	twt_tx_ops->register_events = target_if_twt_register_events;
	twt_tx_ops->deregister_events = target_if_twt_deregister_events;

	status = target_if_twt_register_ext_tx_ops(twt_tx_ops);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Failed to register twt ext tx ops");
		return status;
	}

	return status;
}

QDF_STATUS
target_if_twt_set_twt_ack_support(struct wlan_objmgr_psoc *psoc,
				  bool val)
{
	struct twt_psoc_priv_obj *twt_psoc;

	if (!psoc) {
		target_if_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		target_if_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc->twt_caps.twt_ack_supported = val;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_twt_fill_tgt_caps(struct wlan_objmgr_psoc *psoc,
			    wmi_unified_t wmi_handle)
{
	struct twt_psoc_priv_obj *twt_psoc;
	struct twt_tgt_caps *caps = NULL;

	if (!psoc || !wmi_handle) {
		target_if_err("null wmi_handle or psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		target_if_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	caps = &twt_psoc->twt_caps;

	if (wmi_service_enabled(wmi_handle, wmi_service_twt_requestor))
		caps->twt_requestor = true;
	else
		caps->twt_requestor = false;

	if (wmi_service_enabled(wmi_handle, wmi_service_twt_responder))
		caps->twt_responder = true;
	else
		caps->twt_responder = false;

	if (wmi_service_enabled(wmi_handle, wmi_service_bcast_twt_support))
		caps->legacy_bcast_twt_support = true;
	else
		caps->legacy_bcast_twt_support = false;

	if (wmi_service_enabled(wmi_handle, wmi_service_twt_bcast_req_support))
		caps->twt_bcast_req_support = true;
	else
		caps->twt_bcast_req_support = false;

	if (wmi_service_enabled(wmi_handle, wmi_service_twt_bcast_resp_support))
		caps->twt_bcast_res_support = true;
	else
		caps->twt_bcast_res_support = false;

	if (wmi_service_enabled(wmi_handle, wmi_service_twt_nudge))
		caps->twt_nudge_enabled = true;
	else
		caps->twt_nudge_enabled = false;

	if (wmi_service_enabled(wmi_handle, wmi_service_all_twt))
		caps->all_twt_enabled = true;
	else
		caps->all_twt_enabled = false;

	if (wmi_service_enabled(wmi_handle, wmi_service_twt_statistics))
		caps->twt_stats_enabled = true;
	else
		caps->twt_stats_enabled = false;

	target_if_debug("req:%d res:%d legacy_bcast_twt_support:%d",
		caps->twt_requestor,
		caps->twt_responder,
		caps->legacy_bcast_twt_support);
	target_if_debug("twt_bcast_req_support:%d twt_bcast_res_support:%d",
		caps->twt_bcast_req_support,
		caps->twt_bcast_res_support);
	target_if_debug("nudge_enabled:%d all_twt_enabled:%d stats_enabled:%d",
		caps->twt_nudge_enabled,
		caps->all_twt_enabled,
		caps->twt_stats_enabled);
	return QDF_STATUS_SUCCESS;
}

