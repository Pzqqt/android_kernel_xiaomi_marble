/*
 * Copyright (c) 2012-2015, 2020, The Linux Foundation. All rights reserved.
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
 * DOC: Implements disconnect specific apis of connection manager
 */
#include "wlan_cm_main_api.h"
#include "wlan_cm_sm.h"

QDF_STATUS cm_disconnect_start(struct cnx_mgr *cm_ctx,
			       struct cm_disconnect_req *req)
{
	/*
	 * TODO: Interface event, stop scan, disconnect TDLS, P2P roc cleanup
	 * queue serialization.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_disconnect_active(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	/*
	 * TODO: call vdev sm to start disconnect.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_disconnect_complete(struct cnx_mgr *cm_ctx,
				  struct wlan_cm_discon_rsp *resp)
{
	/*
	 * TODO: inform osif, inform interface manager
	 * update fils/wep key and inform legacy, update bcn filter and scan
	 * entry mlme info, blm action and remove from serialization at the end.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_disconnect_start_req(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_disconnect_req *req)
{
	struct cnx_mgr *cm_ctx = NULL;
	struct cm_disconnect_req *cm_req = NULL;

	/*
	 * TODO: Prepare cm_disconnect_req cm_req, get cm id and inform it to
	 * OSIF. store disconnect req to the cm ctx req_list
	 */

	return cm_sm_deliver_event(cm_ctx, WLAN_CM_SM_EV_DISCONNECT_REQ,
				   sizeof(*cm_req), cm_req);
}
