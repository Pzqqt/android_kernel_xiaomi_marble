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
#include "include/wlan_mlme_cmn.h"
#include "wlan_twt_common.h"
#include "wlan_twt_priv.h"
#include <wlan_twt_public_structs.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_twt_tgt_if_tx_api.h>
#include "twt/core/src/wlan_twt_cfg.h"

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

	twt_psoc->disable_context.twt_role = TWT_ROLE_REQUESTOR;
	twt_psoc->disable_context.context = context;

	req->twt_role = TWT_ROLE_REQUESTOR;

	twt_debug("TWT req disable: pdev_id:%d role:%d ext:%d reason_code:%d",
		  req->pdev_id, req->twt_role, req->ext_conf_present,
		  req->dis_reason_code);

	return tgt_twt_disable_req_send(psoc, req);
}

QDF_STATUS
wlan_twt_responder_disable(struct wlan_objmgr_psoc *psoc,
			   struct twt_disable_param *req,
			   void *context)
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

	twt_psoc->disable_context.twt_role = TWT_ROLE_RESPONDER;
	twt_psoc->disable_context.context = context;

	req->twt_role = TWT_ROLE_RESPONDER;

	twt_debug("TWT res disable: pdev_id:%d role:%d ext:%d reason_code:%d",
		  req->pdev_id, req->twt_role, req->ext_conf_present,
		  req->dis_reason_code);

	return tgt_twt_disable_req_send(psoc, req);
}

QDF_STATUS
wlan_twt_requestor_enable(struct wlan_objmgr_psoc *psoc,
			  struct twt_enable_param *req,
			  void *context)
{
	struct twt_psoc_priv_obj *twt_psoc;
	bool requestor_en = false, twt_bcast_requestor = false;

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

	wlan_twt_cfg_get_requestor(psoc, &requestor_en);
	if (!requestor_en) {
		twt_warn("twt requestor ini is not enabled");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc->enable_context.twt_role = TWT_ROLE_REQUESTOR;
	twt_psoc->enable_context.context = context;

	wlan_twt_cfg_get_congestion_timeout(psoc, &req->sta_cong_timer_ms);
	wlan_twt_cfg_get_bcast_requestor(psoc, &twt_bcast_requestor);
	req->b_twt_enable = twt_bcast_requestor;
	req->twt_role = TWT_ROLE_REQUESTOR;
	if (twt_bcast_requestor)
		req->twt_oper = TWT_OPERATION_BROADCAST;
	else
		req->twt_oper = TWT_OPERATION_INDIVIDUAL;

	twt_debug("TWT req enable: pdev_id:%d cong:%d bcast:%d",
		  req->pdev_id, req->sta_cong_timer_ms, req->b_twt_enable);
	twt_debug("TWT req enable: role:%d ext:%d oper:%d",
		  req->twt_role, req->ext_conf_present, req->twt_oper);

	return tgt_twt_enable_req_send(psoc, req);
}

QDF_STATUS
wlan_twt_responder_enable(struct wlan_objmgr_psoc *psoc,
			  struct twt_enable_param *req,
			  void *context)
{
	struct twt_psoc_priv_obj *twt_psoc;
	bool responder_en = false, twt_bcast_responder = false;

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

	wlan_twt_cfg_get_responder(psoc, &responder_en);
	if (!responder_en) {
		twt_warn("twt responder ini is not enabled");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc->enable_context.twt_role = TWT_ROLE_RESPONDER;
	twt_psoc->enable_context.context = context;

	wlan_twt_cfg_get_bcast_responder(psoc, &twt_bcast_responder);
	req->b_twt_enable = twt_bcast_responder;
	req->twt_role = TWT_ROLE_RESPONDER;
	if (twt_bcast_responder)
		req->twt_oper = TWT_OPERATION_BROADCAST;
	else
		req->twt_oper = TWT_OPERATION_INDIVIDUAL;

	twt_debug("TWT res enable: pdev_id:%d bcast:%d",
		  req->pdev_id, req->b_twt_enable);
	twt_debug("TWT res enable: role:%d ext:%d oper:%d",
		  req->twt_role, req->ext_conf_present, req->twt_oper);

	return tgt_twt_enable_req_send(psoc, req);
}

QDF_STATUS
wlan_twt_set_peer_capabilities(struct wlan_objmgr_psoc *psoc,
			       struct qdf_mac_addr *peer_mac,
			       uint8_t peer_cap)
{
	struct twt_peer_priv_obj *peer_priv;
	struct wlan_objmgr_peer *peer;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found "QDF_MAC_ADDR_FMT,
			QDF_MAC_ADDR_REF(peer_mac));
		return QDF_STATUS_E_FAILURE;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							  WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		twt_err("peer twt component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mutex_acquire(&peer_priv->twt_peer_lock);
	peer_priv->peer_capability = peer_cap;
	qdf_mutex_release(&peer_priv->twt_peer_lock);

	twt_debug("set peer cap: 0x%x", peer_cap);
	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_get_peer_capabilities(struct wlan_objmgr_psoc *psoc,
			       struct qdf_mac_addr *peer_mac,
			       uint8_t *peer_cap)
{
	struct twt_peer_priv_obj *peer_priv;
	struct wlan_objmgr_peer *peer;

	peer = wlan_objmgr_get_peer_by_mac(psoc, peer_mac->bytes,
					   WLAN_TWT_ID);
	if (!peer) {
		twt_err("Peer object not found "QDF_MAC_ADDR_FMT,
			QDF_MAC_ADDR_REF(peer_mac));
		*peer_cap = 0;
		return QDF_STATUS_E_FAILURE;
	}

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							  WLAN_UMAC_COMP_TWT);
	if (!peer_priv) {
		wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
		*peer_cap = 0;
		twt_err("peer twt component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mutex_acquire(&peer_priv->twt_peer_lock);
	*peer_cap = peer_priv->peer_capability;
	qdf_mutex_release(&peer_priv->twt_peer_lock);

	twt_debug("get peer cap: 0x%x", *peer_cap);
	wlan_objmgr_peer_release_ref(peer, WLAN_TWT_ID);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_enable_event_handler(struct wlan_objmgr_psoc *psoc,
			      struct twt_enable_complete_event_param *event)
{
	struct twt_psoc_priv_obj *twt_psoc;
	struct twt_en_dis_context *twt_context;

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

	twt_context = &twt_psoc->enable_context;

	twt_debug("pdev_id:%d status:%d twt_role:%d",
		  event->pdev_id, event->status, twt_context->twt_role);
	switch (event->status) {
	case HOST_TWT_ENABLE_STATUS_OK:
	case HOST_TWT_ENABLE_STATUS_ALREADY_ENABLED:
		if (twt_context->twt_role == TWT_ROLE_REQUESTOR)
			wlan_twt_cfg_set_requestor_flag(psoc, true);
		else if (twt_context->twt_role == TWT_ROLE_RESPONDER)
			wlan_twt_cfg_set_responder_flag(psoc, true);
		else
			twt_err("Invalid role:%d", twt_context->twt_role);

		break;

	default:
		twt_err("twt enable status:%d", event->status);
		break;
	}

	return mlme_twt_osif_enable_complete_ind(psoc, event,
						 twt_context->context);
}

QDF_STATUS
wlan_twt_disable_event_handler(struct wlan_objmgr_psoc *psoc,
			       struct twt_disable_complete_event_param *event)
{
	struct twt_psoc_priv_obj *twt_psoc;
	struct twt_en_dis_context *twt_context;

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

	twt_context = &twt_psoc->disable_context;

	twt_debug("pdev_id:%d status:%d twt_role:%d",
		  event->pdev_id, event->status, twt_context->twt_role);
	switch (event->status) {
	case HOST_TWT_DISABLE_STATUS_OK:
		if (twt_context->twt_role == TWT_ROLE_REQUESTOR)
			wlan_twt_cfg_set_requestor_flag(psoc, false);
		else if (twt_context->twt_role == TWT_ROLE_RESPONDER)
			wlan_twt_cfg_set_responder_flag(psoc, false);
		else
			twt_err("Invalid role:%d", twt_context->twt_role);

		break;

	default:
		twt_err("twt disable status:%d", event->status);
		break;
	}

	return mlme_twt_osif_disable_complete_ind(psoc, event,
						  twt_context->context);
}

