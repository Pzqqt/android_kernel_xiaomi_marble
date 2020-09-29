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
#include <wlan_serialization_api.h>

static QDF_STATUS
cm_ser_disconnect_cb(struct wlan_serialization_command *cmd,
		     enum wlan_serialization_cb_reason reason)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_vdev *vdev;

	if (!cmd) {
		mlme_err("cmd is NULL, reason: %d", reason);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	vdev = cmd->vdev;

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		status = cm_sm_deliver_event(vdev,
					     WLAN_CM_SM_EV_DISCONNECT_ACTIVE,
					     sizeof(wlan_cm_id), &cmd->cmd_id);
		/*
		 * Handle failure if posting fails, i.e. the SM state has
		 * changes. Disconnect shoul dbe handled in JOIN_PENDING,
		 * JOIN-SCAN state as well apart from DISCONNECTING.
		 * Also no need to check for head list as diconnect needs to be
		 * completed always once active.
		 */
		break;
	case WLAN_SER_CB_CANCEL_CMD:
		/* command removed from pending list. */
		break;
	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		mlme_err(CM_PREFIX_LOG "Active command timeout",
			 wlan_vdev_get_id(vdev), cmd->cmd_id);
		QDF_ASSERT(0);
		break;
	case WLAN_SER_CB_RELEASE_MEM_CMD:
		cm_reset_active_cm_id(vdev, cmd->cmd_id);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_CM_ID);
		break;
	default:
		QDF_ASSERT(0);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
}

#define DISCONNECT_TIMEOUT   STOP_RESPONSE_TIMER + DELETE_RESPONSE_TIMER + 1000

static QDF_STATUS cm_ser_disconnect_req(struct wlan_objmgr_pdev *pdev,
					struct cnx_mgr *cm_ctx,
					struct cm_disconnect_req *req)
{
	struct wlan_serialization_command cmd = {0, };
	enum wlan_serialization_status ser_cmd_status;
	QDF_STATUS status;

	status = wlan_objmgr_vdev_try_get_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_LOG "unable to get reference",
			 wlan_vdev_get_id(cm_ctx->vdev), req->cm_id);
		return status;
	}

	cmd.cmd_type = WLAN_SER_CMD_VDEV_CONNECT;
	cmd.cmd_id = req->cm_id;
	cmd.cmd_cb = cm_ser_disconnect_cb;
	cmd.source = WLAN_UMAC_COMP_MLME;
	cmd.is_high_priority = false;
	cmd.cmd_timeout_duration = DISCONNECT_TIMEOUT;
	cmd.vdev = cm_ctx->vdev;
	cmd.is_blocking = cm_ser_get_blocking_cmd();

	ser_cmd_status = wlan_serialization_request(&cmd);
	switch (ser_cmd_status) {
	case WLAN_SER_CMD_PENDING:
		/* command moved to pending list.Do nothing */
		break;
	case WLAN_SER_CMD_ACTIVE:
		/* command moved to active list. Do nothing */
		break;
	default:
		mlme_err(CM_PREFIX_LOG "ser cmd status %d",
			 wlan_vdev_get_id(cm_ctx->vdev), req->cm_id,
			 ser_cmd_status);
		wlan_objmgr_vdev_release_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);

		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_disconnect_start(struct cnx_mgr *cm_ctx,
			       struct cm_disconnect_req *req)
{
	struct wlan_objmgr_pdev *pdev;

	/* Interface event */
	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);
	if (!pdev)
		return QDF_STATUS_E_INVAL;

	/*
	 * Interface event, stop scan, disconnect TDLS, P2P roc cleanup
	 */

	/* Serialize disconnect req, Handle failure status */
	cm_ser_disconnect_req(pdev, cm_ctx, req);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_disconnect_active(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	struct cm_req *cm_req;

	cm_req = cm_get_req_by_cm_id(cm_ctx, *cm_id);
	if (!cm_req)
		return QDF_STATUS_E_INVAL;

	cm_ctx->active_cm_id = *cm_id;

	/*
	 * call vdev sm to start disconnect.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_disconnect_complete(struct cnx_mgr *cm_ctx,
				  struct wlan_cm_discon_rsp *resp)
{
	/*
	 * inform osif, inform interface manager
	 * update fils/wep key and inform legacy, update bcn filter and scan
	 * entry mlme info, blm action and remove from serialization at the end.
	 */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_add_disconnect_req_to_list(struct cnx_mgr *cm_ctx,
					 struct cm_disconnect_req *req)
{
	QDF_STATUS status;
	struct cm_req *cm_req;

	cm_req = qdf_container_of(req, struct cm_req, discon_req);
	req->cm_id = cm_get_cm_id(cm_ctx, req->req.source);
	cm_req->cm_id = req->cm_id;
	status = cm_add_req_to_list_and_indicate_osif(cm_ctx, cm_req,
						      req->req.source);

	return status;
}

QDF_STATUS cm_disconnect_start_req(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_disconnect_req *req)
{
	struct cnx_mgr *cm_ctx;
	struct cm_req *cm_req;
	struct cm_disconnect_req *disconnect_req;
	QDF_STATUS status;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_INVAL;

	/*
	 * This would be freed as part of removal from cm req list if adding
	 * to list is success after posting WLAN_CM_SM_EV_DISCONNECT_REQ.
	 */
	cm_req = qdf_mem_malloc(sizeof(*cm_req));

	if (!cm_req)
		return QDF_STATUS_E_NOMEM;

	disconnect_req = &cm_req->discon_req;
	disconnect_req->req = *req;

	status = cm_sm_deliver_event(vdev, WLAN_CM_SM_EV_DISCONNECT_REQ,
				     sizeof(*disconnect_req), disconnect_req);
	/* free the req if connect is not handled */
	if (QDF_IS_STATUS_ERROR(status))
		qdf_mem_free(cm_req);

	return status;
}
