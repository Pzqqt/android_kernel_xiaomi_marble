/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: contains interface definitions for OS_IF layer
 */

#include "nan_ucfg_api.h"
#include "nan_public_structs.h"
#include "wlan_nan_api.h"
#include "../../core/src/nan_main_i.h"
#include "scheduler_api.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"

struct wlan_objmgr_psoc;
struct wlan_objmgr_vdev;

inline QDF_STATUS ucfg_nan_set_ndi_state(struct wlan_objmgr_vdev *vdev,
					 uint32_t state)
{
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}
	qdf_spin_lock_bh(&priv_obj->lock);
	priv_obj->state = state;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return QDF_STATUS_SUCCESS;
}

inline enum nan_datapath_state ucfg_nan_get_ndi_state(
					struct wlan_objmgr_vdev *vdev)
{
	enum nan_datapath_state val;
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return NAN_DATA_INVALID_STATE;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	val = priv_obj->state;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return val;
}

inline QDF_STATUS ucfg_nan_set_active_peers(struct wlan_objmgr_vdev *vdev,
				     uint32_t val)
{
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	priv_obj->active_ndp_peers = val;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return QDF_STATUS_SUCCESS;
}

inline uint32_t ucfg_nan_get_active_peers(struct wlan_objmgr_vdev *vdev)
{
	uint32_t val;
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return 0;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	val = priv_obj->active_ndp_peers;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return val;
}

inline QDF_STATUS ucfg_nan_set_active_ndp_sessions(
		struct wlan_objmgr_vdev *vdev, uint32_t val, uint8_t idx)
{
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (idx >= MAX_PEERS) {
		nan_err("peer_idx(%d), MAX(%d)",
			idx, MAX_PEERS);
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	priv_obj->active_ndp_sessions[idx] = val;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return QDF_STATUS_SUCCESS;
}

inline uint32_t ucfg_nan_get_active_ndp_sessions(struct wlan_objmgr_vdev *vdev,
						 uint8_t idx)
{
	uint32_t val;
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return 0;
	}

	if (idx >= MAX_PEERS) {
		nan_err("peer_idx(%d), MAX(%d)",
			idx, MAX_PEERS);
		return 0;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	val = priv_obj->active_ndp_sessions[idx];
	qdf_spin_unlock_bh(&priv_obj->lock);

	return val;
}

inline QDF_STATUS ucfg_nan_set_ndp_create_transaction_id(
				struct wlan_objmgr_vdev *vdev, uint16_t val)
{
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	priv_obj->ndp_create_transaction_id = val;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return QDF_STATUS_SUCCESS;
}

inline uint16_t ucfg_nan_get_ndp_create_transaction_id(
						struct wlan_objmgr_vdev *vdev)
{
	uint16_t val;
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return 0;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	val = priv_obj->ndp_create_transaction_id;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return val;
}

inline QDF_STATUS ucfg_nan_set_ndp_delete_transaction_id(
				struct wlan_objmgr_vdev *vdev, uint16_t val)
{
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	priv_obj->ndp_delete_transaction_id = val;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return QDF_STATUS_SUCCESS;
}

inline uint16_t ucfg_nan_get_ndp_delete_transaction_id(
					struct wlan_objmgr_vdev *vdev)
{
	uint16_t val;
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return 0;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	val = priv_obj->ndp_delete_transaction_id;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return val;
}

inline QDF_STATUS ucfg_nan_set_ndi_delete_rsp_reason(
				struct wlan_objmgr_vdev *vdev, uint32_t val)
{
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	priv_obj->ndi_delete_rsp_reason = val;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return QDF_STATUS_SUCCESS;
}

inline uint32_t ucfg_nan_get_ndi_delete_rsp_reason(
					struct wlan_objmgr_vdev *vdev)
{
	uint32_t val;
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return 0;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	val = priv_obj->ndi_delete_rsp_reason;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return val;
}

inline QDF_STATUS ucfg_nan_set_ndi_delete_rsp_status(
				struct wlan_objmgr_vdev *vdev, uint32_t val)
{
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	priv_obj->ndi_delete_rsp_status = val;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return QDF_STATUS_SUCCESS;
}

inline uint32_t ucfg_nan_get_ndi_delete_rsp_status(
						struct wlan_objmgr_vdev *vdev)
{
	uint32_t val;
	struct nan_vdev_priv_obj *priv_obj = nan_get_vdev_priv_obj(vdev);

	if (!priv_obj) {
		nan_err("priv_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock_bh(&priv_obj->lock);
	val = priv_obj->ndi_delete_rsp_status;
	qdf_spin_unlock_bh(&priv_obj->lock);

	return val;
}

inline QDF_STATUS ucfg_nan_get_callbacks(struct wlan_objmgr_psoc *psoc,
					 struct nan_callbacks *cb_obj)
{
	struct nan_psoc_priv_obj *psoc_obj = nan_get_psoc_priv_obj(psoc);

	if (!psoc_obj) {
		nan_err("nan psoc priv object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}
	qdf_spin_lock_bh(&psoc_obj->lock);
	qdf_mem_copy(cb_obj, &psoc_obj->cb_obj, sizeof(*cb_obj));
	qdf_spin_unlock_bh(&psoc_obj->lock);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS ucfg_nan_sch_msg_flush_cb(struct scheduler_msg *msg)
{
	struct wlan_objmgr_vdev *vdev = NULL;

	if (!msg || !msg->bodyptr)
		return QDF_STATUS_E_NULL_VALUE;

	switch (msg->type) {
	case NDP_INITIATOR_REQ:
		vdev = ((struct nan_datapath_initiator_req *)
			msg->bodyptr)->vdev;
		break;
	case NDP_RESPONDER_REQ:
		vdev = ((struct nan_datapath_responder_req *)
			msg->bodyptr)->vdev;
		break;
	case NDP_END_REQ:
		vdev = ((struct nan_datapath_end_req *)msg->bodyptr)->vdev;
		break;
	default:
		nan_err("Invalid NAN msg type during sch flush");
		return QDF_STATUS_E_INVAL;
	}

	if (vdev) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_NAN_ID);
		qdf_mem_free(msg->bodyptr);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_nan_req_processor(struct wlan_objmgr_vdev *vdev,
				  void *in_req, uint32_t req_type)
{
	uint32_t len;
	QDF_STATUS status;
	struct scheduler_msg msg = {0};

	if (!in_req) {
		nan_alert("req is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	switch (req_type) {
	case NDP_INITIATOR_REQ:
		len = sizeof(struct nan_datapath_initiator_req);
		break;
	case NDP_RESPONDER_REQ:
		len = sizeof(struct nan_datapath_responder_req);
		break;
	case NDP_END_REQ:
		len = sizeof(struct nan_datapath_end_req);
		break;
	default:
		nan_err("in correct message req type: %d", req_type);
		return QDF_STATUS_E_INVAL;
	}

	msg.bodyptr = qdf_mem_malloc(len);
	if (!msg.bodyptr) {
		nan_err("malloc failed");
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_copy(msg.bodyptr, in_req, len);
	msg.type = req_type;
	msg.callback = nan_scheduled_msg_handler;
	msg.flush_callback = ucfg_nan_sch_msg_flush_cb;
	status = scheduler_post_message(QDF_MODULE_ID_HDD,
					QDF_MODULE_ID_NAN,
					QDF_MODULE_ID_OS_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		nan_err("failed to post msg to NAN component, status: %d",
			status);
		qdf_mem_free(msg.bodyptr);
	}

	return status;
}

void ucfg_nan_datapath_event_handler(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_vdev *vdev,
				     uint32_t type, void *msg)
{
	struct nan_psoc_priv_obj *psoc_obj = nan_get_psoc_priv_obj(psoc);

	if (!psoc_obj) {
		nan_err("nan psoc priv object is NULL");
		return;
	}

	psoc_obj->cb_obj.os_if_ndp_event_handler(psoc, vdev, type, msg);
}

int ucfg_nan_register_hdd_callbacks(struct wlan_objmgr_psoc *psoc,
				    struct nan_callbacks *cb_obj)
{
	struct nan_psoc_priv_obj *psoc_obj = nan_get_psoc_priv_obj(psoc);

	if (!psoc_obj) {
		nan_err("nan psoc priv object is NULL");
		return -EINVAL;
	}

	psoc_obj->cb_obj.ndi_open = cb_obj->ndi_open;
	psoc_obj->cb_obj.ndi_start = cb_obj->ndi_start;
	psoc_obj->cb_obj.ndi_delete = cb_obj->ndi_delete;
	psoc_obj->cb_obj.ndi_close = cb_obj->ndi_close;
	psoc_obj->cb_obj.drv_ndi_create_rsp_handler =
				cb_obj->drv_ndi_create_rsp_handler;
	psoc_obj->cb_obj.drv_ndi_delete_rsp_handler =
				cb_obj->drv_ndi_delete_rsp_handler;

	psoc_obj->cb_obj.get_peer_idx = cb_obj->get_peer_idx;
	psoc_obj->cb_obj.new_peer_ind = cb_obj->new_peer_ind;
	psoc_obj->cb_obj.peer_departed_ind = cb_obj->peer_departed_ind;
	psoc_obj->cb_obj.os_if_ndp_event_handler =
				cb_obj->os_if_ndp_event_handler;
	psoc_obj->cb_obj.os_if_nan_event_handler =
				cb_obj->os_if_nan_event_handler;

	return 0;
}

int ucfg_nan_register_lim_callbacks(struct wlan_objmgr_psoc *psoc,
				    struct nan_callbacks *cb_obj)
{
	struct nan_psoc_priv_obj *psoc_obj = nan_get_psoc_priv_obj(psoc);

	if (!psoc_obj) {
		nan_err("nan psoc priv object is NULL");
		return -EINVAL;
	}

	psoc_obj->cb_obj.add_ndi_peer = cb_obj->add_ndi_peer;
	psoc_obj->cb_obj.ndp_delete_peers = cb_obj->ndp_delete_peers;
	psoc_obj->cb_obj.delete_peers_by_addr = cb_obj->delete_peers_by_addr;

	return 0;
}

void ucfg_nan_set_tgt_caps(struct wlan_objmgr_psoc *psoc,
			   struct nan_tgt_caps *nan_caps)
{
	struct nan_psoc_priv_obj *psoc_priv = nan_get_psoc_priv_obj(psoc);

	if (!psoc_priv) {
		nan_err("nan psoc priv object is NULL");
		return;
	}

	psoc_priv->nan_caps = *nan_caps;
}

bool ucfg_is_nan_dbs_supported(struct wlan_objmgr_psoc *psoc)
{
	struct nan_psoc_priv_obj *psoc_priv;

	psoc_priv = nan_get_psoc_priv_obj(psoc);
	if (!psoc_priv) {
		nan_err("nan psoc priv object is NULL");
		return false;
	}

	return (psoc_priv->nan_caps.nan_dbs_supported == 1);
}

QDF_STATUS ucfg_nan_discovery_req(void *in_req, uint32_t req_type)
{
	struct wlan_objmgr_psoc *psoc;
	struct scheduler_msg msg = {0};
	uint32_t len;
	QDF_STATUS status;

	if (!in_req) {
		nan_alert("NAN Discovery req is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	switch (req_type) {
	case NAN_ENABLE_REQ: {
			struct nan_enable_req *req = in_req;

			psoc = req->psoc;
			/*
			 * Take a psoc reference while it is being used by the
			 * NAN requests.
			 */
			status =  wlan_objmgr_psoc_try_get_ref(psoc,
							       WLAN_NAN_ID);
			if (QDF_IS_STATUS_ERROR(status)) {
				nan_err("Couldn't obtain psoc ref");
				return status;
			}

			status = nan_discovery_pre_enable(psoc,
							  req->social_chan_2g);
			if (QDF_IS_STATUS_SUCCESS(status)) {
				len = sizeof(struct nan_enable_req) +
					req->params.request_data_len;
			} else {
				wlan_objmgr_psoc_release_ref(psoc,
							     WLAN_NAN_ID);
				return status;
			}
			break;
		}
	case NAN_DISABLE_REQ: {
			struct nan_disable_req *req = in_req;

			psoc = req->psoc;
			status =  wlan_objmgr_psoc_try_get_ref(psoc,
							       WLAN_NAN_ID);
			if (QDF_IS_STATUS_ERROR(status)) {
				nan_err("Couldn't obtain psoc ref");
				return status;
			}

			status =
			  nan_set_discovery_state(req->psoc,
						  NAN_DISC_DISABLE_IN_PROGRESS);
			if (QDF_IS_STATUS_SUCCESS(status)) {
				len = sizeof(struct nan_disable_req) +
					req->params.request_data_len;
			} else {
				wlan_objmgr_psoc_release_ref(psoc,
							     WLAN_NAN_ID);
				return status;
			}
			break;
		}
	case NAN_GENERIC_REQ: {
			struct nan_generic_req *req = in_req;

			psoc = req->psoc;
			status =  wlan_objmgr_psoc_try_get_ref(psoc,
							       WLAN_NAN_ID);
			if (QDF_IS_STATUS_ERROR(status)) {
				nan_err("Couldn't obtain psoc ref");
				return status;
			}
			len = sizeof(struct nan_generic_req) +
				req->params.request_data_len;
			break;
		}
	default:
		nan_err("in correct message req type: %d", req_type);
		return QDF_STATUS_E_INVAL;
	}

	msg.bodyptr = qdf_mem_malloc(len);
	if (!msg.bodyptr) {
		wlan_objmgr_psoc_release_ref(psoc, WLAN_NAN_ID);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(msg.bodyptr, in_req, len);
	msg.type = req_type;
	msg.callback = nan_discovery_scheduled_handler;
	msg.flush_callback = nan_discovery_flush_callback;
	status = scheduler_post_message(QDF_MODULE_ID_NAN,
					QDF_MODULE_ID_NAN,
					QDF_MODULE_ID_OS_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		nan_err("failed to post msg to NAN component, status: %d",
			status);
		nan_discovery_flush_callback(&msg);
	}

	return status;
}
