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
 * DOC: Implements connect specific APIs of connection manager
 */

#include "wlan_cm_main_api.h"

QDF_STATUS cm_connect_start(struct cnx_mgr *cm_ctx, struct cm_connect_req *req)
{
	/*
	 * TODO: Interface event, get candidate list, do hw mode change and
	 * serialize. Also move to scan state from here if required.
	 * If candidate found and all are blacklisted or serialization fails
	 * post WLAN_CM_SM_EV_CONNECT_REQ_FAIL.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_connect_active(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	/*
	 * TODO: get first valid candidate, create bss peer.
	 * fill vdev crypto for the peer.
	 * call vdev sm to start connect for the candidate.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_try_next_candidate(struct cnx_mgr *cm_ctx,
				 struct wlan_cm_connect_rsp *resp)
{
	/*
	 * TODO: get next valid candidate, if no candidate left, post
	 * WLAN_CM_SM_EV_CONNECT_FAILURE to SM, inform osif about failure for
	 * the candidate if its not last one. and initiate the connect for
	 * next candidate.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_connect_cmd_timeout(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
{
	/*
	 * TODO: get the connect req from connect list and post
	 * WLAN_CM_SM_EV_CONNECT_FAILURE.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_connect_complete(struct cnx_mgr *cm_ctx,
			       struct wlan_cm_connect_rsp *resp)
{
	/*
	 * TODO: inform osif about success/failure, inform interface manager
	 * update fils/wep key and inform legacy, update bcn filter and scan
	 * entry mlme info, blm action and remove from serialization at the end.
	 */
	return QDF_STATUS_SUCCESS;
}
