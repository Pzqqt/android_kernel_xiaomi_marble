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
#include "wlan_scan_api.h"

/* Scan for ssid timeout set to 10 seconds
 * Calculation for timeout:
 * 8 sec(time to complete scan on all channels) + 2 sec(buffer)
 */
#define SCAN_FOR_SSID_TIMEOUT       10000

static QDF_STATUS cm_fill_scan_req(struct cnx_mgr *cm_ctx,
				   struct cm_connect_req *cm_req,
				   struct scan_start_request *req)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	enum channel_state state;
	qdf_freq_t ch_freq;

	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "Failed to find pdev",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id));
		return status;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "Failed to find psoc",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id));
		return status;
	}

	cm_req->scan_id = wlan_scan_get_scan_id(psoc);
	status = wlan_scan_init_default_params(cm_ctx->vdev, req);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	req->scan_req.scan_type = SCAN_TYPE_SCAN_FOR_CONNECT;
	req->scan_req.scan_id = cm_req->scan_id;
	req->scan_req.scan_req_id = cm_ctx->scan_requester_id;
	req->scan_req.scan_f_passive = false;
	req->scan_req.scan_f_bcast_probe = false;

	if (cm_req->req.scan_ie.len) {
		req->scan_req.extraie.ptr =
			qdf_mem_malloc(cm_req->req.scan_ie.len);

		if (!req->scan_req.extraie.ptr) {
			status = QDF_STATUS_E_NOMEM;
			return status;
		}

		qdf_mem_copy(req->scan_req.extraie.ptr,
			     cm_req->req.scan_ie.ptr,
			     cm_req->req.scan_ie.len);
		req->scan_req.extraie.len = cm_req->req.scan_ie.len;
	}

	if (wlan_vdev_mlme_get_opmode(cm_ctx->vdev) == QDF_P2P_CLIENT_MODE)
		req->scan_req.scan_priority = SCAN_PRIORITY_HIGH;

	ch_freq = cm_req->req.chan_freq;
	if (ch_freq) {
		state = wlan_reg_get_channel_state_for_freq(pdev,
							    ch_freq);

		if (state == CHANNEL_STATE_DISABLE ||
		    state == CHANNEL_STATE_INVALID) {
			mlme_err(CM_PREFIX_FMT "Invalid channel frequency",
				 CM_PREFIX_REF(vdev_id, cm_req->cm_id));
			status = QDF_STATUS_E_INVAL;
			return status;
		}
		req->scan_req.chan_list.chan[0].freq = ch_freq;
		req->scan_req.chan_list.num_chan = 1;
	}

	if (cm_req->req.ssid.length > WLAN_SSID_MAX_LEN) {
		mlme_debug(CM_PREFIX_FMT "Wrong ssid length %d",
			   CM_PREFIX_REF(vdev_id, cm_req->cm_id),
			   cm_req->req.ssid.length);

		status = QDF_STATUS_E_INVAL;
		return status;
	}
	req->scan_req.num_ssids = 1;
	qdf_mem_copy(&req->scan_req.ssid[0].ssid,
		     &cm_req->req.ssid.ssid,
		     cm_req->req.ssid.length);

	req->scan_req.ssid[0].length = cm_req->req.ssid.length;
	mlme_debug(CM_PREFIX_FMT "Connect scan for %.*s",
		   CM_PREFIX_REF(vdev_id, cm_req->cm_id),
		   req->scan_req.ssid[0].length,
		   req->scan_req.ssid[0].ssid);

	req->scan_req.num_bssid = 1;
	if (qdf_is_macaddr_zero(&cm_req->req.bssid))
		qdf_set_macaddr_broadcast(&req->scan_req.bssid_list[0]);
	else
		qdf_copy_macaddr(&req->scan_req.bssid_list[0],
				 &cm_req->req.bssid);

	/* max_scan_time set to 10sec, at timeout scan is aborted */
	req->scan_req.max_scan_time = SCAN_FOR_SSID_TIMEOUT;

	return status;
}

QDF_STATUS cm_connect_scan_start(struct cnx_mgr *cm_ctx,
				 struct cm_connect_req *cm_req)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct scan_start_request *scan_req;

	scan_req = qdf_mem_malloc(sizeof(*scan_req));
	if (!scan_req) {
		status = QDF_STATUS_E_NOMEM;
		goto scan_err;
	}

	status = cm_fill_scan_req(cm_ctx, cm_req, scan_req);

	if (QDF_IS_STATUS_ERROR(status)) {
		if (scan_req->scan_req.extraie.ptr) {
			qdf_mem_free(scan_req->scan_req.extraie.ptr);
			scan_req->scan_req.extraie.len = 0;
			scan_req->scan_req.extraie.ptr = NULL;
		}
		qdf_mem_free(scan_req);
		goto scan_err;
	}

	/* scan_req will be freed by wlan_scan_start */
	status = wlan_scan_start(scan_req);

scan_err:
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "Failed to initiate scan with status: %d",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
				       cm_req->cm_id), status);

		status = cm_sm_deliver_event_sync(cm_ctx,
						  WLAN_CM_SM_EV_SCAN_FAILURE,
						  sizeof(*cm_req),
						  cm_req);
		/*
		 * Handle failure if posting fails, i.e. the SM state has
		 * changed or head cm_id doesn't match the active cm_id.
		 * scan start failure should be handled only in SS_SCAN. If
		 * new command has been received connect procedure should be
		 * aborted from here with connect req cleanup.
		 */
		if (QDF_IS_STATUS_ERROR(status))
			cm_connect_handle_event_post_fail(cm_ctx,
							  cm_req->cm_id);
	}

	return status;
}

QDF_STATUS cm_connect_scan_resp(struct cnx_mgr *cm_ctx, wlan_scan_id *scan_id,
				QDF_STATUS status)
{
	struct cm_req *cm_req = NULL;
	enum wlan_cm_connect_fail_reason reason = CM_GENERIC_FAILURE;

	if (!*scan_id)
		goto scan_failure;

	cm_req = cm_get_req_by_scan_id(cm_ctx, *scan_id);
	if (!cm_req)
		goto scan_failure;

	if (QDF_IS_STATUS_ERROR(status)) {
		reason = CM_NO_CANDIDATE_FOUND;
		goto scan_failure;
	}
	cm_connect_start(cm_ctx, &cm_req->connect_req);

	return QDF_STATUS_SUCCESS;
scan_failure:
	return cm_send_connect_start_fail(cm_ctx, &cm_req->connect_req,
					  reason);
}

void wlan_cm_scan_cb(struct wlan_objmgr_vdev *vdev,
		     struct scan_event *event, void *arg)
{
	struct cnx_mgr *cm_ctx = (struct cnx_mgr *)arg;
	wlan_cm_id cm_id = CM_ID_INVALID;
	bool success = false;
	QDF_STATUS status;

	if (!util_is_scan_completed(event, &success))
		return;

	status = cm_sm_deliver_event(vdev,
				     WLAN_CM_SM_EV_SCAN_SUCCESS,
				     sizeof(event->scan_id),
				     &event->scan_id);
	/*
	 * Handle failure if posting fails, i.e. the SM state has
	 * changed or head cm_id doesn't match the active cm_id.
	 * scan cb should be handled only in SS_SCAN. If
	 * new command has been received connect procedure should be
	 * aborted from here with connect req cleanup.
	 */
	if (QDF_IS_STATUS_ERROR(status)) {
		cm_id = cm_get_cm_id_by_scan_id(cm_ctx, event->scan_id);
		if (cm_id != CM_ID_INVALID)
			cm_connect_handle_event_post_fail(cm_ctx,
							  cm_id);
	}

}

