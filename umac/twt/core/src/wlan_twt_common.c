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
 *  DOC: wlan_twt_common.c
 */
#include "wlan_twt_common.h"
#include "wlan_twt_priv.h"
#include <wlan_twt_public_structs.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_twt_tgt_if_tx_api.h>

QDF_STATUS
wlan_twt_tgt_caps_get_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	*val = twt_psoc->twt_caps.twt_requestor;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_tgt_caps_get_responder(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	*val = twt_psoc->twt_caps.twt_responder;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_tgt_caps_get_legacy_bcast_support(struct wlan_objmgr_psoc *psoc,
					   bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	*val = twt_psoc->twt_caps.legacy_bcast_twt_support;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_tgt_caps_get_bcast_req_support(struct wlan_objmgr_psoc *psoc,
					bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	*val = twt_psoc->twt_caps.twt_bcast_req_support;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_tgt_caps_get_bcast_res_support(struct wlan_objmgr_psoc *psoc,
					bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	*val = twt_psoc->twt_caps.twt_bcast_res_support;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_tgt_caps_get_nudge_enabled(struct wlan_objmgr_psoc *psoc,
				    bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	*val = twt_psoc->twt_caps.twt_nudge_enabled;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_tgt_caps_get_all_twt_enabled(struct wlan_objmgr_psoc *psoc,
				      bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	*val = twt_psoc->twt_caps.all_twt_enabled;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_tgt_caps_get_stats_enabled(struct wlan_objmgr_psoc *psoc,
					bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	*val = twt_psoc->twt_caps.twt_stats_enabled;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_twt_check_all_twt_support(struct wlan_objmgr_psoc *psoc,
					  uint32_t dialog_id)
{
	bool is_all_twt_enabled = false;
	QDF_STATUS status;

	/* Cap check is check NOT required if id is for a single session */
	if (dialog_id != TWT_ALL_SESSIONS_DIALOG_ID)
		return QDF_STATUS_SUCCESS;

	status = wlan_twt_tgt_caps_get_all_twt_enabled(psoc,
						       &is_all_twt_enabled);
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_INVAL;

	if (!is_all_twt_enabled)
		return QDF_STATUS_E_NOSUPPORT;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_tgt_caps_get_ack_supported(struct wlan_objmgr_psoc *psoc,
				    bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;

	if (!psoc) {
		twt_err("null psoc");
		*val = false;
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		*val = false;
		return QDF_STATUS_E_FAILURE;
	}

	*val = twt_psoc->twt_caps.twt_ack_supported;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_requestor_disable(struct wlan_objmgr_psoc *psoc,
			   struct twt_disable_param *req,
			   void *context)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_responder_disable(struct wlan_objmgr_psoc *psoc,
			   struct twt_disable_param *req,
			   void *context)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_requestor_enable(struct wlan_objmgr_psoc *psoc,
			  struct twt_enable_param *req,
			  void *context)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_responder_enable(struct wlan_objmgr_psoc *psoc,
			  struct twt_enable_param *req,
			  void *context)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_enable_event_handler(struct wlan_objmgr_psoc *psoc,
			      struct twt_enable_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_disable_event_handler(struct wlan_objmgr_psoc *psoc,
			       struct twt_disable_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

