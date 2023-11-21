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
 *  DOC: wlan_twt_ucfg_api.c
 *  This file contains twt ucfg APIs
 */
#include <wlan_twt_ucfg_api.h>
#include "wlan_twt_common.h"

QDF_STATUS ucfg_twt_get_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return wlan_twt_tgt_caps_get_requestor(psoc, val);
}

QDF_STATUS ucfg_twt_get_responder(struct wlan_objmgr_psoc *psoc, bool *val)
{
	return wlan_twt_tgt_caps_get_responder(psoc, val);
}

QDF_STATUS ucfg_twt_get_legacy_bcast_twt_support(struct wlan_objmgr_psoc *psoc,
						bool *val)
{
	return wlan_twt_tgt_caps_get_legacy_bcast_support(psoc, val);
}

QDF_STATUS ucfg_twt_get_twt_bcast_req_support(struct wlan_objmgr_psoc *psoc,
						bool *val)
{
	return wlan_twt_tgt_caps_get_bcast_req_support(psoc, val);
}

QDF_STATUS ucfg_twt_get_twt_bcast_res_support(struct wlan_objmgr_psoc *psoc,
						bool *val)
{
	return wlan_twt_tgt_caps_get_bcast_res_support(psoc, val);
}

QDF_STATUS ucfg_twt_get_twt_nudge_enabled(struct wlan_objmgr_psoc *psoc,
					bool *val)
{
	return wlan_twt_tgt_caps_get_nudge_enabled(psoc, val);
}

QDF_STATUS ucfg_twt_get_all_twt_enabled(struct wlan_objmgr_psoc *psoc,
					bool *val)
{
	return wlan_twt_tgt_caps_get_all_twt_enabled(psoc, val);
}

QDF_STATUS ucfg_twt_check_all_twt_support(struct wlan_objmgr_psoc *psoc,
					  uint32_t dialog_id)
{
	return wlan_twt_check_all_twt_support(psoc, dialog_id);
}

QDF_STATUS ucfg_twt_get_twt_stats_enabled(struct wlan_objmgr_psoc *psoc,
					bool *val)
{
	return wlan_twt_tgt_caps_get_stats_enabled(psoc, val);
}

QDF_STATUS ucfg_twt_get_twt_ack_supported(struct wlan_objmgr_psoc *psoc,
					bool *val)
{
	return wlan_twt_tgt_caps_get_ack_supported(psoc, val);
}

QDF_STATUS ucfg_twt_requestor_disable(struct wlan_objmgr_psoc *psoc,
				      struct twt_disable_param *req,
				      void *context)
{
	return wlan_twt_requestor_disable(psoc, req, context);
}

QDF_STATUS ucfg_twt_responder_disable(struct wlan_objmgr_psoc *psoc,
				      struct twt_disable_param *req,
				      void *context)
{
	return wlan_twt_responder_disable(psoc, req, context);
}

QDF_STATUS ucfg_twt_requestor_enable(struct wlan_objmgr_psoc *psoc,
				     struct twt_enable_param *req,
				     void *context)
{
	return wlan_twt_requestor_enable(psoc, req, context);
}

QDF_STATUS ucfg_twt_responder_enable(struct wlan_objmgr_psoc *psoc,
				     struct twt_enable_param *req,
				     void *context)
{
	return wlan_twt_responder_enable(psoc, req, context);
}

QDF_STATUS
ucfg_twt_set_peer_capabilities(struct wlan_objmgr_psoc *psoc,
			       struct qdf_mac_addr *peer_mac,
			       uint8_t peer_cap)
{
	return wlan_twt_set_peer_capabilities(psoc, peer_mac, peer_cap);
}

QDF_STATUS
ucfg_twt_get_peer_capabilities(struct wlan_objmgr_psoc *psoc,
			       struct qdf_mac_addr *peer_mac,
			       uint8_t *peer_cap)
{
	return wlan_twt_get_peer_capabilities(psoc, peer_mac, peer_cap);
}
