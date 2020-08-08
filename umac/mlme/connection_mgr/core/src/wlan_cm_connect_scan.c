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
 * DOC: Implements connect scan (scan for ssid) specific apis of
 * connection manager
 */

#include "wlan_cm_main_api.h"

QDF_STATUS cm_connect_scan_start(struct cnx_mgr *cm_ctx,
				 struct cm_connect_req *req)
{
	/*
	 * initiate connect scan and scan timer.
	 * post WLAN_CM_SM_EV_CONNECT_SCAN_FAILURE on scan req failure.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_connect_scan_resp(struct cnx_mgr *cm_ctx, wlan_scan_id *scan_id,
				QDF_STATUS status)
{
	/*
	 * get connect req from head and check if scan_id match
	 * if not drop else proceed with success or failure handling
	 * (post WLAN_CM_SM_EV_CONNECT_FAILURE in case of status failure or if
	 * no candidate found again after scan)
	 */
	return QDF_STATUS_SUCCESS;
}
