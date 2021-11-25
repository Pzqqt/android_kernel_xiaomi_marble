/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * DOC: contains MLO manager ap related functionality
 */
#include "wlan_mlo_mgr_cmn.h"
#include "wlan_mlo_mgr_main.h"
#ifdef WLAN_MLO_MULTI_CHIP
#include "wlan_lmac_if_def.h"
#endif
#include "wlan_serialization_api.h"
#include <target_if_mlo_mgr.h>

void mlo_get_link_information(struct qdf_mac_addr *mld_addr,
			      struct mlo_link_info *info)
{
/* Pass the partner link information*/
}

void is_mlo_all_links_up(struct wlan_mlo_dev_context *mldev)
{
/* Loop through all the vdev's part of the ML device*/
/* STA: Loop through all the associated vdev status. */
}

struct wlan_objmgr_vdev *mlo_get_vdev_by_link_id(
			struct wlan_objmgr_vdev *vdev,
			uint8_t link_id)
{
	struct wlan_mlo_dev_context *dev_ctx;
	int i;
	struct wlan_objmgr_vdev *partner_vdev = NULL;

	if (!vdev || !vdev->mlo_dev_ctx) {
		mlo_err("Invalid input");
		return partner_vdev;
	}

	dev_ctx = vdev->mlo_dev_ctx;

	mlo_dev_lock_acquire(dev_ctx);
	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (dev_ctx->wlan_vdev_list[i] &&
		    wlan_vdev_mlme_is_mlo_vdev(dev_ctx->wlan_vdev_list[i]) &&
		    dev_ctx->wlan_vdev_list[i]->vdev_mlme.mlo_link_id ==
		    link_id) {
			if (wlan_objmgr_vdev_try_get_ref(
						dev_ctx->wlan_vdev_list[i],
						WLAN_MLO_MGR_ID) ==
							QDF_STATUS_SUCCESS)
				partner_vdev = dev_ctx->wlan_vdev_list[i];

			break;
		}
	}
	mlo_dev_lock_release(dev_ctx);

	return partner_vdev;
}

void mlo_release_vdev_ref(struct wlan_objmgr_vdev *vdev)
{
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLO_MGR_ID);
}

QDF_STATUS mlo_reg_mlme_ext_cb(struct mlo_mgr_context *ctx,
			       struct mlo_mlme_ext_ops *ops)
{
	if (!ctx)
		return QDF_STATUS_E_FAILURE;

	ctx->mlme_ops = ops;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlo_unreg_mlme_ext_cb(struct mlo_mgr_context *ctx)
{
	if (!ctx)
		return QDF_STATUS_E_FAILURE;

	ctx->mlme_ops = NULL;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlo_mlme_clone_sta_security(struct wlan_objmgr_vdev *vdev,
				       struct wlan_cm_connect_req *req)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	struct vdev_mlme_obj *vdev_mlme;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_validate_conn_req)
		return QDF_STATUS_E_FAILURE;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme)
		return QDF_STATUS_E_FAILURE;

	if (mlo_ctx->mlme_ops->mlo_mlme_ext_clone_security_param) {
		status =
			mlo_ctx->mlme_ops->mlo_mlme_ext_clone_security_param(
				vdev_mlme, req);
	}

	return status;
}

QDF_STATUS mlo_mlme_validate_conn_req(struct wlan_objmgr_vdev *vdev,
				      void *ext_data)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	struct vdev_mlme_obj *vdev_mlme;
	QDF_STATUS status;

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_validate_conn_req)
		return QDF_STATUS_E_FAILURE;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme)
		return QDF_STATUS_E_FAILURE;

	status =
		mlo_ctx->mlme_ops->mlo_mlme_ext_validate_conn_req(vdev_mlme,
								  ext_data);
	return status;
}

QDF_STATUS mlo_mlme_create_link_vdev(struct wlan_objmgr_vdev *vdev,
				     void *ext_data)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	struct vdev_mlme_obj *vdev_mlme;
	QDF_STATUS status;

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_create_link_vdev)
		return QDF_STATUS_E_FAILURE;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme)
		return QDF_STATUS_E_FAILURE;

	status =
		mlo_ctx->mlme_ops->mlo_mlme_ext_create_link_vdev(vdev_mlme,
								 ext_data);
	return status;
}

void mlo_mlme_peer_create(struct wlan_objmgr_vdev *vdev,
			  struct wlan_mlo_peer_context *ml_peer,
			  struct qdf_mac_addr *addr,
			  qdf_nbuf_t frm_buf)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_peer_create)
		return;

	mlo_ctx->mlme_ops->mlo_mlme_ext_peer_create(vdev, ml_peer,
						    addr, frm_buf);
}

void mlo_mlme_peer_assoc(struct wlan_objmgr_peer *peer)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_peer_assoc)
		return;

	mlo_ctx->mlme_ops->mlo_mlme_ext_peer_assoc(peer);
}

void mlo_mlme_peer_assoc_fail(struct wlan_objmgr_peer *peer)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_peer_assoc_fail)
		return;

	mlo_ctx->mlme_ops->mlo_mlme_ext_peer_assoc_fail(peer);
}

void mlo_mlme_peer_delete(struct wlan_objmgr_peer *peer)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_peer_delete)
		return;

	mlo_ctx->mlme_ops->mlo_mlme_ext_peer_delete(peer);
}

void mlo_mlme_peer_assoc_resp(struct wlan_objmgr_peer *peer)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_assoc_resp)
		return;

	mlo_ctx->mlme_ops->mlo_mlme_ext_assoc_resp(peer);
}

qdf_nbuf_t mlo_mlme_get_link_assoc_req(struct wlan_objmgr_peer *peer,
				       uint8_t link_ix)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_get_link_assoc_req)
		return NULL;

	return mlo_ctx->mlme_ops->mlo_mlme_get_link_assoc_req(peer, link_ix);
}

void mlo_mlme_peer_deauth(struct wlan_objmgr_peer *peer)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_deauth)
		return;

	mlo_ctx->mlme_ops->mlo_mlme_ext_deauth(peer);
}

uint8_t mlo_get_link_vdev_ix(struct wlan_mlo_dev_context *ml_dev,
			     struct wlan_objmgr_vdev *vdev)
{
	uint8_t i;

	mlo_dev_lock_acquire(ml_dev);
	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (vdev == ml_dev->wlan_vdev_list[i]) {
			mlo_dev_lock_release(ml_dev);
			return i;
		}
	}
	mlo_dev_lock_release(ml_dev);

	return (uint8_t)-1;
}

#ifdef WLAN_MLO_MULTI_CHIP
uint16_t wlan_mlo_get_pdev_hw_link_id(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_tx_ops *tx_ops;
	uint16_t hw_link_id = INVALID_HW_LINK_ID;

	psoc = wlan_pdev_get_psoc(pdev);
	if (psoc) {
		tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
		if (tx_ops && tx_ops->mops.get_hw_link_id)
			hw_link_id = tx_ops->mops.get_hw_link_id(pdev);
	}

	return hw_link_id;
}

static void wlan_pdev_hw_link_iterator(struct wlan_objmgr_psoc *psoc,
				       void *obj, void *arg)
{
	struct hw_link_id_iterator *itr = (struct hw_link_id_iterator *)arg;
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)obj;
	uint16_t hw_link_id;

	if (itr->pdev)
		return;

	hw_link_id = wlan_mlo_get_pdev_hw_link_id(pdev);
	if (hw_link_id == itr->hw_link_id) {
		if (wlan_objmgr_pdev_try_get_ref(pdev, itr->dbgid) ==
							QDF_STATUS_SUCCESS)
			itr->pdev = pdev;
	}
}

static void wlan_mlo_find_hw_link_id(struct wlan_objmgr_psoc *psoc,
				     void *arg,
				     uint8_t index)
{
	struct hw_link_id_iterator *itr = (struct hw_link_id_iterator *)arg;

	wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
				     wlan_pdev_hw_link_iterator,
				     arg, false, itr->dbgid);
}

struct wlan_objmgr_pdev *
wlan_mlo_get_pdev_by_hw_link_id(uint16_t hw_link_id,
				wlan_objmgr_ref_dbgid refdbgid)
{
	struct hw_link_id_iterator itr;

	itr.hw_link_id = hw_link_id;
	itr.pdev = NULL;
	itr.dbgid = refdbgid;

	wlan_objmgr_iterate_psoc_list(wlan_mlo_find_hw_link_id,
				      &itr, refdbgid);

	return itr.pdev;
}
#endif /*WLAN_MLO_MULTI_CHIP*/

void mlo_get_ml_vdev_list(struct wlan_objmgr_vdev *vdev,
			  uint16_t *vdev_count,
			  struct wlan_objmgr_vdev **wlan_vdev_list)
{
	struct wlan_mlo_dev_context *dev_ctx;
	int i;
	QDF_STATUS status;

	*vdev_count = 0;

	if (!vdev || !vdev->mlo_dev_ctx) {
		mlo_err("Invalid input");
		return;
	}

	dev_ctx = vdev->mlo_dev_ctx;

	mlo_dev_lock_acquire(dev_ctx);
	*vdev_count = 0;
	for (i = 0; i < QDF_ARRAY_SIZE(dev_ctx->wlan_vdev_list); i++) {
		if (dev_ctx->wlan_vdev_list[i] &&
		    wlan_vdev_mlme_is_mlo_vdev(dev_ctx->wlan_vdev_list[i])) {
			status = wlan_objmgr_vdev_try_get_ref(
						dev_ctx->wlan_vdev_list[i],
						WLAN_MLO_MGR_ID);
			if (QDF_IS_STATUS_ERROR(status))
				break;
			wlan_vdev_list[*vdev_count] =
				dev_ctx->wlan_vdev_list[i];
			(*vdev_count) += 1;
		}
	}
	mlo_dev_lock_release(dev_ctx);
}

/**
 * mlo_link_set_active() - send MLO link set active command
 * @psoc: PSOC object
 * @param: MLO link set active params
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mlo_link_set_active(struct wlan_objmgr_psoc *psoc,
		    struct mlo_link_set_active_param *param)
{
	struct wlan_lmac_if_mlo_tx_ops *mlo_tx_ops;

	if (!psoc) {
		mlo_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	mlo_tx_ops = target_if_mlo_get_tx_ops(psoc);
	if (!mlo_tx_ops) {
		mlo_err("tx_ops is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!mlo_tx_ops->link_set_active) {
		mlo_err("link_set_active function is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return mlo_tx_ops->link_set_active(psoc, param);
}

/**
 * mlo_release_ser_link_set_active_cmd() - relases serialization command for
 *  forcing MLO link active/inactive
 * @vdev: Object manager vdev
 *
 * Return: None
 */
static void
mlo_release_ser_link_set_active_cmd(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_serialization_queued_cmd_info cmd = {0};

	cmd.cmd_type = WLAN_SER_CMD_SET_MLO_LINK;
	cmd.requestor = WLAN_UMAC_COMP_MLO_MGR;
	cmd.cmd_id = 0;
	cmd.vdev = vdev;

	mlo_debug("release serialization command");
	wlan_serialization_remove_cmd(&cmd);
}

/**
 * mlo_link_set_active_resp_vdev_handler() - vdev handler for mlo link set
 * active response event.
 * @psoc: psoc object
 * @obj: vdev object
 * @arg: mlo link set active response
 *
 * Return: None
 */
static void
mlo_link_set_active_resp_vdev_handler(struct wlan_objmgr_psoc *psoc,
				      void *obj, void *arg)
{
	struct mlo_link_set_active_req *req;
	struct wlan_objmgr_vdev *vdev = obj;
	struct mlo_link_set_active_resp *event = arg;

	req = wlan_serialization_get_active_cmd(wlan_vdev_get_psoc(vdev),
						wlan_vdev_get_id(vdev),
						WLAN_SER_CMD_SET_MLO_LINK);
	if (!req)
		return;

	if (req->ctx.set_mlo_link_cb)
		req->ctx.set_mlo_link_cb(vdev, req->ctx.cb_arg, event);

	mlo_release_ser_link_set_active_cmd(vdev);
}

QDF_STATUS
mlo_process_link_set_active_resp(struct wlan_objmgr_psoc *psoc,
				 struct mlo_link_set_active_resp *event)
{
	wlan_objmgr_iterate_obj_list(psoc, WLAN_VDEV_OP,
				     mlo_link_set_active_resp_vdev_handler,
				     event, true, WLAN_MLO_MGR_ID);
	return QDF_STATUS_SUCCESS;
}

/**
 * mlo_ser_set_link_cb() - Serialization callback function
 * @cmd: Serialization command info
 * @reason: Serialization reason for callback execution
 *
 * Return: Status of callback execution
 */
static QDF_STATUS
mlo_ser_set_link_cb(struct wlan_serialization_command *cmd,
		    enum wlan_serialization_cb_reason reason)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;
	struct mlo_link_set_active_req *req;
	struct mlo_mgr_context *mlo_ctx;

	if (!cmd || !cmd->vdev)
		return QDF_STATUS_E_FAILURE;

	mlo_ctx = wlan_objmgr_get_mlo_ctx();
	if (!mlo_ctx)
		return QDF_STATUS_E_FAILURE;

	psoc = wlan_vdev_get_psoc(cmd->vdev);
	if (!psoc) {
		mlo_err("psoc is NULL, reason: %d", reason);
		return QDF_STATUS_E_NULL_VALUE;
	}

	req = cmd->umac_cmd;
	if (!req)
		return QDF_STATUS_E_INVAL;

	vdev = cmd->vdev;
	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		status = mlo_link_set_active(psoc, &req->param);
		break;
	case WLAN_SER_CB_CANCEL_CMD:
	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		mlo_err("vdev %d command not execute: %d",
			wlan_vdev_get_id(vdev), reason);
		if (req->ctx.set_mlo_link_cb)
			req->ctx.set_mlo_link_cb(vdev, req->ctx.cb_arg, NULL);
		break;
	case WLAN_SER_CB_RELEASE_MEM_CMD:
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLO_MGR_ID);
		qdf_mem_free(req);
		break;
	default:
		QDF_ASSERT(0);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
}

#define MLO_SER_CMD_TIMEOUT_MS 5000
QDF_STATUS mlo_ser_set_link_req(struct mlo_link_set_active_req *req)
{
	struct wlan_serialization_command cmd = {0, };
	enum wlan_serialization_status ser_cmd_status;
	QDF_STATUS status;
	void *umac_cmd;
	struct wlan_objmgr_vdev *vdev;

	if (!req)
		return QDF_STATUS_E_INVAL;

	vdev = req->ctx.vdev;
	status = wlan_objmgr_vdev_try_get_ref(vdev, WLAN_MLO_MGR_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlo_err("vdev %d unable to get reference",
			wlan_vdev_get_id(vdev));
		return status;
	}

	umac_cmd = qdf_mem_malloc(sizeof(*req));
	if (!umac_cmd) {
		status = QDF_STATUS_E_NOMEM;
		goto out;
	}
	qdf_mem_copy(umac_cmd, req, sizeof(*req));

	cmd.cmd_type = WLAN_SER_CMD_SET_MLO_LINK;
	cmd.cmd_id = 0;
	cmd.cmd_cb = mlo_ser_set_link_cb;
	cmd.source = WLAN_UMAC_COMP_MLO_MGR;
	cmd.is_high_priority = false;
	cmd.cmd_timeout_duration = MLO_SER_CMD_TIMEOUT_MS;
	cmd.vdev = vdev;
	cmd.is_blocking = true;
	cmd.umac_cmd = umac_cmd;

	ser_cmd_status = wlan_serialization_request(&cmd);
	switch (ser_cmd_status) {
	case WLAN_SER_CMD_PENDING:
		/* command moved to pending list.Do nothing */
		break;
	case WLAN_SER_CMD_ACTIVE:
		/* command moved to active list. Do nothing */
		break;
	default:
		mlo_err("vdev %d ser cmd status %d",
			wlan_vdev_get_id(vdev), ser_cmd_status);
		status = QDF_STATUS_E_FAILURE;
	}

out:
	if (QDF_IS_STATUS_SUCCESS(status))
		return status;

	if (umac_cmd)
		qdf_mem_free(umac_cmd);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLO_MGR_ID);

	return status;
}

void mlo_mlme_handle_sta_csa_param(struct wlan_objmgr_vdev *vdev,
				   struct csa_offload_params *csa_param)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_handle_sta_csa_param)
		return;

	mlo_ctx->mlme_ops->mlo_mlme_ext_handle_sta_csa_param(vdev, csa_param);
}
