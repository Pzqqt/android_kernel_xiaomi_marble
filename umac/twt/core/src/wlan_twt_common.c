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
wlan_twt_get_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_get_responder(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_get_legacy_bcast_twt_support(struct wlan_objmgr_psoc *psoc,
				      bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_get_twt_bcast_req_support(struct wlan_objmgr_psoc *psoc,
				   bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_get_twt_bcast_res_support(struct wlan_objmgr_psoc *psoc,
				   bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_get_twt_nudge_enabled(struct wlan_objmgr_psoc *psoc,
			       bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_get_all_twt_enabled(struct wlan_objmgr_psoc *psoc,
			     bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_get_twt_stats_enabled(struct wlan_objmgr_psoc *psoc,
			       bool *val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_get_twt_ack_supported(struct wlan_objmgr_psoc *psoc,
			       bool *val)
{
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

