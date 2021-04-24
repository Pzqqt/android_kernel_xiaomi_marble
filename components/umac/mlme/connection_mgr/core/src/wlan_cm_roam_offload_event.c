/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: wlan_cm_roam_offload_event.c
 *
 * Implementation for the FW based roaming events api interfaces.
 */
#include "qdf_status.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_cm_roam_i.h"
#include <wlan_cm_public_struct.h>
#include "wlan_scan_public_structs.h"
#include "wlan_cm_roam_public_struct.h"
#include "wlan_serialization_api.h"
#include "wlan_cm_roam_api.h"
#include <wlan_cfg80211_scan.h>
#ifdef FEATURE_CM_ENABLE
#include "connection_mgr/core/src/wlan_cm_roam.h"
#include "connection_mgr/core/src/wlan_cm_sm.h"
#include "connection_mgr/core/src/wlan_cm_main_api.h"
#define FW_ROAM_SYNC_TIMEOUT 7000

static QDF_STATUS
cm_fw_roam_ser_cb(struct wlan_serialization_command *cmd,
		  enum wlan_serialization_cb_reason reason)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_vdev *vdev;
	struct cnx_mgr *cm_ctx;

	if (!cmd) {
		mlme_err("cmd is NULL, reason: %d", reason);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	vdev = cmd->vdev;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_NULL_VALUE;

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		cm_ctx->active_cm_id = cmd->cmd_id;
		break;
	case WLAN_SER_CB_CANCEL_CMD:
		/* command removed from pending list. */
		break;
	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		mlme_err(CM_PREFIX_FMT "Active command timeout",
			 CM_PREFIX_REF(wlan_vdev_get_id(vdev), cmd->cmd_id));

		cm_abort_fw_roam(cm_ctx, cmd->cmd_id);
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

QDF_STATUS cm_abort_fw_roam(struct cnx_mgr *cm_ctx,
			    wlan_cm_id cm_id)
{
	QDF_STATUS status;

	status = cm_sm_deliver_event(cm_ctx->vdev,
				     WLAN_CM_SM_EV_ROAM_ABORT,
				     sizeof(wlan_cm_id), &cm_id);

	if (QDF_IS_STATUS_ERROR(status))
		cm_remove_cmd(cm_ctx, &cm_id);

	return status;
}

QDF_STATUS
cm_add_fw_roam_dummy_ser_cb(struct wlan_objmgr_pdev *pdev,
			    struct cnx_mgr *cm_ctx,
			    struct cm_req *cm_req)
{
	struct wlan_serialization_command cmd = {0, };
	enum wlan_serialization_status ser_cmd_status;
	QDF_STATUS status;
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);

	status = wlan_objmgr_vdev_try_get_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "unable to get reference",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id));
		return status;
	}

	cmd.cmd_type = WLAN_SER_CMD_VDEV_ROAM;
	cmd.cmd_id = cm_req->cm_id;
	cmd.cmd_cb = cm_fw_roam_ser_cb;
	cmd.source = WLAN_UMAC_COMP_MLME;
	cmd.is_high_priority = true;
	cmd.cmd_timeout_duration = FW_ROAM_SYNC_TIMEOUT;
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
		mlme_err(CM_PREFIX_FMT "ser cmd status %d",
			 CM_PREFIX_REF(vdev_id, cm_req->cm_id), ser_cmd_status);
		wlan_objmgr_vdev_release_ref(cm_ctx->vdev, WLAN_MLME_CM_ID);

		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_prepare_roam_cmd(struct cnx_mgr *cm_ctx,
			       struct cm_req **roam_req,
			       enum wlan_cm_source source)
{
	struct cm_req *req;

	*roam_req = qdf_mem_malloc(sizeof(**roam_req));
	if (!*roam_req)
		return QDF_STATUS_E_NOMEM;

	req = *roam_req;
	req->roam_req.req.source = source;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_add_fw_roam_cmd_to_list_n_ser(struct cnx_mgr *cm_ctx,
					    struct cm_req *cm_req)
{
	struct wlan_objmgr_pdev *pdev;
	QDF_STATUS status;

	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);

	if (!pdev) {
		mlme_err("Failed to find pdev for vdev id %d",
			 wlan_vdev_get_id(cm_ctx->vdev));
		return QDF_STATUS_E_FAILURE;
	}

	status = cm_add_roam_req_to_list(cm_ctx, cm_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		cm_abort_fw_roam(cm_ctx, CM_ID_INVALID);
		cm_free_roam_req_mem(&cm_req->roam_req);
		qdf_mem_free(cm_req);
		return status;
	}

	status = cm_add_fw_roam_dummy_ser_cb(pdev, cm_ctx, cm_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		cm_abort_fw_roam(cm_ctx, cm_req->roam_req.cm_id);
		return status;
	}

	return status;
}

QDF_STATUS cm_fw_roam_start_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_SB_ID);

	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = cm_sm_deliver_event(vdev, WLAN_CM_SM_EV_ROAM_START,
				     0, NULL);

	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("EV ROAM START not handled");

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);

	return status;
}

QDF_STATUS cm_fw_roam_start(struct cnx_mgr *cm_ctx)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;
	wlan_scan_id scan_id;
	bool abort_host_scan_cap = false;
	wlan_cm_id cm_id;
	struct cm_roam_req *roam_req = NULL;

	roam_req = cm_get_first_roam_command(cm_ctx->vdev);
	if (!roam_req) {
		mlme_err("Failed to find roam req from list");
		cm_id = CM_ID_INVALID;
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	cm_id = roam_req->cm_id;
	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "Failed to find pdev",
			 CM_PREFIX_REF(roam_req->req.vdev_id,
				       roam_req->cm_id));
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "Failed to find psoc",
			 CM_PREFIX_REF(roam_req->req.vdev_id,
				       roam_req->cm_id));
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	status = wlan_cm_roam_state_change(pdev,
					   roam_req->req.vdev_id,
					   WLAN_ROAMING_IN_PROG,
					   REASON_ROAM_CANDIDATE_FOUND);

	if (QDF_IS_STATUS_ERROR(status))
		goto error;

	mlme_cm_osif_roam_start_ind(cm_ctx->vdev);
	/*
	 * For emergency deauth roaming, firmware sends ROAM start
	 * instead of ROAM scan start notification as data path queues
	 * will be stopped only during roam start notification.
	 * This is because, for deauth/disassoc triggered roam, the
	 * AP has sent deauth, and packets shouldn't be sent to AP
	 * after that. Since firmware is sending roam start directly
	 * host sends scan abort during roam scan, but in other
	 * triggers, the host receives roam start after candidate
	 * selection and roam scan is complete. So when host sends
	 * roam abort for emergency deauth roam trigger, the firmware
	 * roam scan is also aborted. This results in roaming failure.
	 * So send scan_id as CANCEL_HOST_SCAN_ID to scan module to
	 * abort only host triggered scans.
	 */
	abort_host_scan_cap =
		wlan_mlme_get_host_scan_abort_support(psoc);
	if (abort_host_scan_cap)
		scan_id = CANCEL_HOST_SCAN_ID;
	else
		scan_id = INVAL_SCAN_ID;

	wlan_abort_scan(pdev, INVAL_PDEV_ID,
			roam_req->req.vdev_id,
			scan_id, false);
error:
	if (QDF_IS_STATUS_ERROR(status))
		cm_abort_fw_roam(cm_ctx, cm_id);

	return status;
}

QDF_STATUS cm_fw_roam_abort_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	struct cnx_mgr *cm_ctx;
	QDF_STATUS status;
	struct cm_roam_req *roam_req = NULL;
	wlan_cm_id cm_id;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	cm_ctx = cm_get_cm_ctx(vdev);
	roam_req = cm_get_first_roam_command(vdev);
	if (!roam_req) {
		mlme_err("Failed to find roam req from list");
		cm_id = CM_ID_INVALID;
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	cm_id = roam_req->cm_id;
	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("Failed to find pdev for vdev id %d",
			 roam_req->req.vdev_id);
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	mlme_cm_osif_roam_abort_ind(cm_ctx->vdev);
	status = wlan_cm_roam_state_change(pdev,
					   roam_req->req.vdev_id,
					   WLAN_ROAM_RSO_ENABLED,
					   REASON_ROAM_ABORT);

	if (QDF_IS_STATUS_ERROR(status))
		goto end;
end:
	cm_abort_fw_roam(cm_ctx, cm_id);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);

	return status;
}
#endif /*FEATURE_CM_ENABLE */
