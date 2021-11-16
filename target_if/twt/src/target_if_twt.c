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
 *  DOC: target_if_twt.c
 *  This file contains twt component's target related function definitions
 */
#include <target_if_twt.h>
#include <target_if_twt_cmd.h>
#include <target_if_twt_evt.h>
#include <target_if_ext_twt.h>
#include "twt/core/src/wlan_twt_common.h"
#include "twt/core/src/wlan_twt_priv.h"

QDF_STATUS
target_if_twt_register_events(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_twt_deregister_events(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_twt_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	return QDF_STATUS_SUCCESS;
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

