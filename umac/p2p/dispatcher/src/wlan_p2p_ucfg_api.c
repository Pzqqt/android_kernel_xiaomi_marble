/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

/**
 * DOC: This file contains p2p north bound interface definitions
 */

#include <wmi_unified_api.h>
#include <wlan_objmgr_psoc_obj.h>
#include <scheduler_api.h>
#include "wlan_p2p_ucfg_api.h"
#include "wlan_p2p_public_struct.h"
#include "../../core/src/wlan_p2p_main.h"
#include "../../core/src/wlan_p2p_roc.h"
#include "../../core/src/wlan_p2p_off_chan_tx.h"

static inline struct wlan_lmac_if_p2p_tx_ops *
ucfg_p2p_psoc_get_tx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &(psoc->soc_cb.tx_ops.p2p);
}

/**
 * is_p2p_ps_allowed() - If P2P power save is allowed or not
 * @vdev: vdev object
 * @id: umac component id
 *
 * This function returns TRUE if P2P power-save is allowed
 * else returns FALSE.
 *
 * Return: bool
 */
static bool is_p2p_ps_allowed(struct wlan_objmgr_vdev *vdev,
				enum wlan_umac_comp_id id)
{
	struct p2p_vdev_priv_obj *p2p_vdev_obj;
	uint8_t is_p2pgo = 0;

	if (!vdev) {
		p2p_err("vdev:%p", vdev);
		return true;
	}
	p2p_vdev_obj = wlan_objmgr_vdev_get_comp_private_obj(vdev,
						WLAN_UMAC_COMP_P2P);

	if (wlan_vdev_mlme_get_opmode(vdev) == QDF_P2P_GO_MODE)
		is_p2pgo = 1;

	if (!p2p_vdev_obj || !is_p2pgo) {
		p2p_err("p2p_vdev_obj:%p is_p2pgo:%u",
			p2p_vdev_obj, is_p2pgo);
		return false;
	}
	if (p2p_vdev_obj->non_p2p_peer_count &&
	    p2p_vdev_obj->noa_status == false) {
		p2p_debug("non_p2p_peer_count: %u, noa_status: %d",
			p2p_vdev_obj->non_p2p_peer_count,
			p2p_vdev_obj->noa_status);
		return false;
	}

	return true;
}

QDF_STATUS ucfg_p2p_init(void)
{
	return p2p_component_init();
}

QDF_STATUS ucfg_p2p_deinit(void)
{
	return p2p_component_deinit();
}

QDF_STATUS ucfg_p2p_psoc_open(struct wlan_objmgr_psoc *soc)
{
	return p2p_psoc_object_open(soc);
}

QDF_STATUS ucfg_p2p_psoc_close(struct wlan_objmgr_psoc *soc)
{
	return p2p_psoc_object_close(soc);
}

QDF_STATUS ucfg_p2p_psoc_start(struct wlan_objmgr_psoc *soc,
	struct p2p_start_param *req)
{
	return p2p_psoc_start(soc, req);
}

QDF_STATUS ucfg_p2p_psoc_stop(struct wlan_objmgr_psoc *soc)
{
	return p2p_psoc_stop(soc);
}

QDF_STATUS ucfg_p2p_roc_req(struct wlan_objmgr_psoc *soc,
	struct p2p_roc_req *roc_req, uint64_t *cookie)
{
	struct scheduler_msg msg = {0};
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct p2p_roc_context *roc_ctx;

	p2p_debug("soc:%p, vdev_id:%d, chan:%d, phy_mode:%d, duration:%d",
		soc, roc_req->vdev_id, roc_req->chan,
		roc_req->phy_mode, roc_req->duration);

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("P2P soc object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	roc_ctx = qdf_mem_malloc(sizeof(*roc_ctx));
	if (!roc_ctx) {
		p2p_err("failed to allocate p2p roc context");
		return QDF_STATUS_E_NOMEM;
	}

	*cookie = (uintptr_t)roc_ctx;
	roc_ctx->p2p_soc_obj = p2p_soc_obj;
	roc_ctx->vdev_id = roc_req->vdev_id;
	roc_ctx->chan = roc_req->chan;
	roc_ctx->phy_mode = roc_req->phy_mode;
	roc_ctx->duration = roc_req->duration;
	roc_ctx->roc_state = ROC_STATE_IDLE;
	roc_ctx->roc_type = USER_REQUESTED;
	msg.type = P2P_ROC_REQ;
	msg.bodyptr = roc_ctx;
	msg.callback = p2p_process_cmd;
	scheduler_post_msg(QDF_MODULE_ID_OS_IF, &msg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_roc_cancel_req(struct wlan_objmgr_psoc *soc,
	uint64_t cookie)
{
	struct scheduler_msg msg = {0};
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct cancel_roc_context *cancel_roc;

	p2p_debug("soc:%p, cookie:0x%llx", soc, cookie);

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("p2p soc context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	cancel_roc = qdf_mem_malloc(sizeof(*cancel_roc));
	if (!cancel_roc) {
		p2p_err("failed to allocate cancel p2p roc");
		return QDF_STATUS_E_NOMEM;
	}

	cancel_roc->p2p_soc_obj = p2p_soc_obj;
	cancel_roc->cookie = cookie;
	msg.type = P2P_CANCEL_ROC_REQ;
	msg.bodyptr = cancel_roc;
	msg.callback = p2p_process_cmd;
	scheduler_post_msg(QDF_MODULE_ID_OS_IF, &msg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_cleanup_roc(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct p2p_soc_priv_obj *p2p_soc_obj;
	uint32_t vdev_id;

	p2p_debug("vdev:%p", vdev);

	if (!vdev) {
		p2p_err("null vdev");
		return QDF_STATUS_E_INVAL;
	}

	wlan_vdev_obj_lock(vdev);
	vdev_id = (uint32_t)wlan_vdev_get_id(vdev);
	psoc = wlan_vdev_get_psoc(vdev);
	wlan_vdev_obj_unlock(vdev);
	if (!psoc) {
		p2p_err("null psoc");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("p2p soc context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return p2p_cleanup_roc_by_vdev(p2p_soc_obj, vdev_id);
}

QDF_STATUS ucfg_p2p_mgmt_tx(struct wlan_objmgr_psoc *soc,
	struct p2p_mgmt_tx *mgmt_frm, uint64_t *cookie)
{
	struct scheduler_msg msg = {0};
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct  tx_action_context *tx_action;

	p2p_debug("soc:%p, vdev_id:%d, chan:%d, wait:%d, buf_len:%d, cck:%d, no ack:%d, off chan:%d",
		soc, mgmt_frm->vdev_id, mgmt_frm->chan,
		mgmt_frm->wait, mgmt_frm->len, mgmt_frm->no_cck,
		mgmt_frm->dont_wait_for_ack, mgmt_frm->off_chan);

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("P2P soc context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	tx_action = qdf_mem_malloc(sizeof(*tx_action));
	if (!tx_action) {
		p2p_err("Failed to allocate tx action context");
		return QDF_STATUS_E_NOMEM;
	}

	/* return cookie just for ota ack frames */
	if (mgmt_frm->dont_wait_for_ack)
		*cookie = 0;
	else
		*cookie = (uintptr_t)tx_action;

	tx_action->p2p_soc_obj = p2p_soc_obj;
	tx_action->vdev_id = mgmt_frm->vdev_id;
	tx_action->chan = mgmt_frm->chan;
	tx_action->duration = mgmt_frm->wait;
	tx_action->buf_len = mgmt_frm->len;
	tx_action->no_cck = mgmt_frm->no_cck;
	tx_action->no_ack = mgmt_frm->dont_wait_for_ack;
	tx_action->off_chan = mgmt_frm->off_chan;
	tx_action->buf = qdf_mem_malloc(tx_action->buf_len);
	if (!(tx_action->buf)) {
		p2p_err("Failed to allocate buffer for action frame");
		qdf_mem_free(tx_action);
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_copy(tx_action->buf, mgmt_frm->buf, tx_action->buf_len);
	msg.type = P2P_MGMT_TX;
	msg.bodyptr = tx_action;
	msg.callback = p2p_process_cmd;
	scheduler_post_msg(QDF_MODULE_ID_OS_IF, &msg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_mgmt_tx_cancel(struct wlan_objmgr_psoc *soc,
	uint64_t cookie)
{
	struct scheduler_msg msg = {0};
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct cancel_roc_context *cancel_tx;

	p2p_debug("soc:%p, cookie:0x%llx", soc, cookie);

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("p2p soc context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	cancel_tx = qdf_mem_malloc(sizeof(*cancel_tx));
	if (!cancel_tx) {
		p2p_err("Failed to allocate cancel p2p roc");
		return QDF_STATUS_E_NOMEM;
	}

	cancel_tx->p2p_soc_obj = p2p_soc_obj;
	cancel_tx->cookie = cookie;
	msg.type = P2P_MGMT_TX_CANCEL;
	msg.bodyptr = cancel_tx;
	msg.callback = p2p_process_cmd;
	scheduler_post_msg(QDF_MODULE_ID_OS_IF, &msg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_p2p_set_ps(struct wlan_objmgr_psoc *soc,
	struct p2p_ps_config *ps_config)
{
	struct wlan_lmac_if_p2p_tx_ops *p2p_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint16_t obj_id;
	struct wlan_objmgr_vdev *vdev;

	p2p_debug("soc:%p, vdev_id:%d, opp_ps:%d, ct_window:%d, count:%d, duration:%d, duration:%d, ps_selection:%d",
		soc, ps_config->vdev_id, ps_config->opp_ps,
		ps_config->ct_window, ps_config->count,
		ps_config->duration, ps_config->single_noa_duration,
		ps_config->ps_selection);

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	for (obj_id = 0; obj_id < WLAN_UMAC_PSOC_MAX_VDEVS; obj_id++) {

		vdev = wlan_objmgr_get_vdev_by_id_from_psoc(soc, obj_id,
							WLAN_P2P_ID);
		if (vdev) {
			if (is_p2p_ps_allowed(vdev,
				WLAN_UMAC_COMP_P2P) == false) {
				p2p_debug("p2p set ps, NoA is disabled as legacy STA is connected to GO.");
				return QDF_STATUS_E_BUSY;
			}
			wlan_objmgr_vdev_release_ref(vdev, WLAN_P2P_ID);
		}
	}

	p2p_ops = ucfg_p2p_psoc_get_tx_ops(soc);
	if (p2p_ops->set_ps) {
		status = p2p_ops->set_ps(soc, ps_config);
		p2p_debug("p2p set ps, status:%d", status);
	}

	return status;
}

QDF_STATUS ucfg_p2p_lo_start(struct wlan_objmgr_psoc *soc,
	struct p2p_lo_start *p2p_lo_start)
{
	struct wlan_lmac_if_p2p_tx_ops *p2p_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	p2p_debug("soc:%p, vdev_id:%d, ctl_flags:%d, freq:%d, period:%d, interval:%d, count:%d, dev_types_len:%d, probe_resp_len:%d, device_types:%p, probe_resp_tmplt:%p",
		soc, p2p_lo_start->vdev_id, p2p_lo_start->ctl_flags,
		p2p_lo_start->freq, p2p_lo_start->period,
		p2p_lo_start->interval, p2p_lo_start->count,
		p2p_lo_start->dev_types_len, p2p_lo_start->probe_resp_len,
		p2p_lo_start->device_types, p2p_lo_start->probe_resp_tmplt);

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_ops = ucfg_p2p_psoc_get_tx_ops(soc);
	if (p2p_ops->lo_start) {
		status = p2p_ops->lo_start(soc, p2p_lo_start);
		p2p_debug("p2p lo start, status:%d", status);
	}

	return status;
}

QDF_STATUS ucfg_p2p_lo_stop(struct wlan_objmgr_psoc *soc,
	uint32_t vdev_id)
{
	struct wlan_lmac_if_p2p_tx_ops *p2p_ops;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	p2p_debug("soc:%p, vdev_id:%d", soc, vdev_id);

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_ops = ucfg_p2p_psoc_get_tx_ops(soc);
	if (p2p_ops->lo_stop) {
		status = p2p_ops->lo_stop(soc, vdev_id);
		p2p_debug("p2p lo stop, status:%d", status);
	}

	return status;
}

QDF_STATUS  ucfg_p2p_set_noa(struct wlan_objmgr_psoc *soc,
	uint32_t vdev_id, bool disable_noa)
{
	struct wlan_lmac_if_p2p_tx_ops *p2p_ops;
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	p2p_ops = ucfg_p2p_psoc_get_tx_ops(soc);
	if (p2p_ops->set_noa) {
		status = p2p_ops->set_noa(soc, vdev_id, disable_noa);
		p2p_debug("p2p set noa, status:%d", status);
	}

	return status;
}
