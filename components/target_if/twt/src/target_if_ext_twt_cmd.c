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
 *  DOC: target_if_ext_twt_cmd.c
 *  This file contains twt component's target related function definitions
 */
#include <wlan_twt_public_structs.h>
#include <wlan_lmac_if_def.h>
#include <target_if.h>
#include <target_if_ext_twt.h>
#include <wmi_unified_twt_api.h>

QDF_STATUS
target_if_twt_setup_req(struct wlan_objmgr_psoc *psoc,
			struct twt_add_dialog_param *req)
{
	struct wmi_unified *wmi_handle;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return wmi_unified_twt_add_dialog_cmd(wmi_handle, req);
}

QDF_STATUS
target_if_twt_teardown_req(struct wlan_objmgr_psoc *psoc,
			   struct twt_del_dialog_param *req)
{
	struct wmi_unified *wmi_handle;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return wmi_unified_twt_del_dialog_cmd(wmi_handle, req);
}

QDF_STATUS
target_if_twt_pause_req(struct wlan_objmgr_psoc *psoc,
			struct twt_pause_dialog_cmd_param *req)
{
	struct wmi_unified *wmi_handle;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return wmi_unified_twt_pause_dialog_cmd(wmi_handle, req);
}

QDF_STATUS
target_if_twt_resume_req(struct wlan_objmgr_psoc *psoc,
			 struct twt_resume_dialog_cmd_param *req)
{
	struct wmi_unified *wmi_handle;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return wmi_unified_twt_resume_dialog_cmd(wmi_handle, req);
}

QDF_STATUS
target_if_twt_nudge_req(struct wlan_objmgr_psoc *psoc,
			struct twt_nudge_dialog_cmd_param *req)
{
	struct wmi_unified *wmi_handle;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return wmi_unified_twt_nudge_dialog_cmd(wmi_handle, req);
}

QDF_STATUS
target_if_twt_register_ext_tx_ops(struct wlan_lmac_if_twt_tx_ops *twt_tx_ops)
{
	twt_tx_ops->setup_req = target_if_twt_setup_req;
	twt_tx_ops->teardown_req = target_if_twt_teardown_req;
	twt_tx_ops->pause_req = target_if_twt_pause_req;
	twt_tx_ops->resume_req = target_if_twt_resume_req;
	twt_tx_ops->nudge_req = target_if_twt_nudge_req;

	return QDF_STATUS_SUCCESS;
}

