/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
#include "wlan_osif_request_manager.h"
#include "wlan_policy_mgr_api.h"
#include "cfg_ucfg_api.h"
#include "cfg_nan.h"

struct wlan_objmgr_psoc;
struct wlan_objmgr_vdev;

#ifdef WLAN_FEATURE_NAN
/**
 * nan_cfg_init() - Initialize NAN config params
 * @psoc: Pointer to PSOC Object
 * @nan_obj: Pointer to NAN private object
 *
 * This function initialize NAN config params
 */
static void nan_cfg_init(struct wlan_objmgr_psoc *psoc,
			 struct nan_psoc_priv_obj *nan_obj)
{
	nan_obj->cfg_param.enable = cfg_get(psoc, CFG_NAN_ENABLE);
}

/**
 * nan_cfg_dp_init() - Initialize NAN Datapath config params
 * @psoc: Pointer to PSOC Object
 * @nan_obj: Pointer to NAN private object
 *
 * This function initialize NAN config params
 */
static void nan_cfg_dp_init(struct wlan_objmgr_psoc *psoc,
			    struct nan_psoc_priv_obj *nan_obj)
{
	nan_obj->cfg_param.dp_enable = cfg_get(psoc,
					       CFG_NAN_DATAPATH_ENABLE);
	nan_obj->cfg_param.ndi_mac_randomize =
				cfg_get(psoc, CFG_NAN_RANDOMIZE_NDI_MAC);
	nan_obj->cfg_param.ndp_inactivity_timeout =
				cfg_get(psoc, CFG_NAN_NDP_INACTIVITY_TIMEOUT);
}
#else
static void nan_cfg_init(struct wlan_objmgr_psoc *psoc,
			 struct nan_psoc_priv_obj *nan_obj)
{
}

static void nan_cfg_dp_init(struct wlan_objmgr_psoc *psoc,
			    struct nan_psoc_priv_obj *nan_obj)
{
}
#endif

QDF_STATUS ucfg_nan_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	struct nan_psoc_priv_obj *nan_obj = nan_get_psoc_priv_obj(psoc);

	if (!nan_obj) {
		nan_err("nan psoc priv object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	nan_cfg_init(psoc, nan_obj);
	nan_cfg_dp_init(psoc, nan_obj);

	return QDF_STATUS_SUCCESS;
}

void ucfg_nan_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	/* No cleanup required on psoc close for NAN */
}

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
	case NDP_END_ALL:
		vdev = ((struct nan_datapath_end_all_ndps *)msg->bodyptr)->vdev;
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
	case NDP_END_ALL:
		len = sizeof(struct nan_datapath_end_all_ndps);
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
	if (QDF_IS_STATUS_ERROR(status))
		qdf_mem_free(msg.bodyptr);

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

static void ucfg_nan_request_process_cb(void *cookie)
{
	struct osif_request *request;

	request = osif_request_get(cookie);
	if (request) {
		nan_debug("request (cookie:0x%pK) completed", cookie);
		osif_request_complete(request);
		osif_request_put(request);
	} else {
		nan_err("Obsolete request (cookie:0x%pK), do nothing", cookie);
	}
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

	psoc_obj->cb_obj.new_peer_ind = cb_obj->new_peer_ind;
	psoc_obj->cb_obj.peer_departed_ind = cb_obj->peer_departed_ind;
	psoc_obj->cb_obj.os_if_ndp_event_handler =
				cb_obj->os_if_ndp_event_handler;
	psoc_obj->cb_obj.os_if_nan_event_handler =
				cb_obj->os_if_nan_event_handler;
	psoc_obj->cb_obj.ucfg_nan_request_process_cb =
				ucfg_nan_request_process_cb;

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

int ucfg_nan_register_wma_callbacks(struct wlan_objmgr_psoc *psoc,
				    struct nan_callbacks *cb_obj)
{
	struct nan_psoc_priv_obj *psoc_obj = nan_get_psoc_priv_obj(psoc);

	if (!psoc_obj) {
		nan_err("nan psoc priv object is NULL");
		return -EINVAL;
	}

	psoc_obj->cb_obj.update_ndi_conn = cb_obj->update_ndi_conn;

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

bool ucfg_is_nan_disable_supported(struct wlan_objmgr_psoc *psoc)
{
	struct nan_psoc_priv_obj *psoc_priv;

	psoc_priv = nan_get_psoc_priv_obj(psoc);
	if (!psoc_priv) {
		nan_err("nan psoc priv object is NULL");
		return false;
	}

	return (psoc_priv->nan_caps.nan_disable_supported == 1);
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

bool ucfg_is_ndi_dbs_supported(struct wlan_objmgr_psoc *psoc)
{
	struct nan_psoc_priv_obj *psoc_priv;

	psoc_priv = nan_get_psoc_priv_obj(psoc);
	if (!psoc_priv) {
		nan_err("nan psoc priv object is NULL");
		return false;
	}

	return (psoc_priv->nan_caps.ndi_dbs_supported == 1);
}

bool ucfg_is_nan_enable_allowed(struct wlan_objmgr_psoc *psoc,
				uint32_t nan_ch_freq)
{
	return nan_is_enable_allowed(psoc, nan_ch_freq);
}

bool ucfg_is_nan_disc_active(struct wlan_objmgr_psoc *psoc)
{
	return nan_is_disc_active(psoc);
}

QDF_STATUS ucfg_nan_discovery_req(void *in_req, uint32_t req_type)
{
	struct wlan_objmgr_psoc *psoc;
	struct scheduler_msg msg = {0};
	uint32_t len;
	QDF_STATUS status;
	struct nan_psoc_priv_obj *psoc_priv;
	struct osif_request *request = NULL;
	static const struct osif_request_params params = {
		.priv_size = 0,
		.timeout_ms = 1000,
	};
	int err;

	if (!in_req) {
		nan_alert("NAN Discovery req is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	switch (req_type) {
	case NAN_ENABLE_REQ: {
			struct nan_enable_req *req = in_req;

			psoc = req->psoc;
			psoc_priv = nan_get_psoc_priv_obj(psoc);
			if (!psoc_priv) {
				nan_err("nan psoc priv object is NULL");
				return QDF_STATUS_E_INVAL;
			}

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
						  req->social_chan_2g_freq);
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
			psoc_priv = nan_get_psoc_priv_obj(psoc);
			if (!psoc_priv) {
				nan_err("nan psoc priv object is NULL");
				return QDF_STATUS_E_INVAL;
			}

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
			psoc_priv = nan_get_psoc_priv_obj(psoc);
			if (!psoc_priv) {
				nan_err("nan psoc priv object is NULL");
				return QDF_STATUS_E_INVAL;
			}

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

	if (req_type == NAN_GENERIC_REQ)
		goto post_msg;

	request = osif_request_alloc(&params);
	if (!request) {
		nan_err("Request allocation failure");
		nan_discovery_flush_callback(&msg);
		return QDF_STATUS_E_NOMEM;
	}

	psoc_priv->request_context = osif_request_cookie(request);
	if (req_type == NAN_DISABLE_REQ)
		psoc_priv->is_explicit_disable = true;

post_msg:
	nan_debug("posting request: %u", req_type);
	status = scheduler_post_message(QDF_MODULE_ID_NAN,
					QDF_MODULE_ID_NAN,
					QDF_MODULE_ID_OS_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		nan_err("failed to post msg to NAN component, status: %d",
			status);
		nan_discovery_flush_callback(&msg);
	}

	if (req_type != NAN_GENERIC_REQ) {
		err = osif_request_wait_for_response(request);
		if (err)
			nan_err("NAN request: %u timed out: %d",
				req_type, err);
		else
			nan_debug("NAN request: %u serviced successfully",
				  req_type);

		if (req_type == NAN_DISABLE_REQ)
			psoc_priv->is_explicit_disable = false;

		osif_request_put(request);
	}

	return status;
}

void ucfg_nan_disable_concurrency(struct wlan_objmgr_psoc *psoc)
{
	struct nan_disable_req nan_req = {0};
	enum nan_disc_state curr_nan_state;
	struct nan_psoc_priv_obj *psoc_priv;
	QDF_STATUS status;

	if (!psoc) {
		nan_err("psoc object is NULL, no action will be taken");
		return;
	}

	psoc_priv = nan_get_psoc_priv_obj(psoc);
	if (!psoc_priv) {
		nan_err("nan psoc priv object is NULL");
		return;
	}

	if (!ucfg_is_nan_disable_supported(psoc))
		return;

	qdf_spin_lock_bh(&psoc_priv->lock);
	curr_nan_state = nan_get_discovery_state(psoc);

	if (curr_nan_state == NAN_DISC_DISABLED ||
	    curr_nan_state == NAN_DISC_DISABLE_IN_PROGRESS) {
		qdf_spin_unlock_bh(&psoc_priv->lock);
		return;
	}
	qdf_spin_unlock_bh(&psoc_priv->lock);

	nan_req.psoc = psoc;
	nan_req.disable_2g_discovery = true;
	nan_req.disable_5g_discovery = true;

	status = ucfg_nan_discovery_req(&nan_req, NAN_DISABLE_REQ);
	if (QDF_IS_STATUS_ERROR(status)) {
		nan_err("Unable to disable NAN Discovery");
		return;
	}

	nan_debug("NAN Disabled successfully");
}

static QDF_STATUS
ucfg_nan_disable_ndi(struct wlan_objmgr_psoc *psoc, uint32_t ndi_vdev_id)
{
	enum nan_datapath_state curr_ndi_state;
	struct nan_datapath_host_event *event;
	struct nan_vdev_priv_obj *ndi_vdev_priv;
	struct nan_datapath_end_all_ndps req = {0};
	struct wlan_objmgr_vdev *ndi_vdev;
	struct osif_request *request;
	QDF_STATUS status;
	int err;
	static const struct osif_request_params params = {
		.priv_size = sizeof(struct nan_datapath_host_event),
		.timeout_ms = 1000,
	};

	if (!ucfg_is_ndi_dbs_supported(psoc))
		return QDF_STATUS_SUCCESS;

	ndi_vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, ndi_vdev_id,
							WLAN_NAN_ID);
	if (!ndi_vdev) {
		nan_err("Cannot obtain NDI vdev object!");
		return QDF_STATUS_E_INVAL;
	}

	ndi_vdev_priv = nan_get_vdev_priv_obj(ndi_vdev);
	if (!ndi_vdev_priv) {
		nan_err("ndi vdev priv object is NULL");
		wlan_objmgr_vdev_release_ref(ndi_vdev, WLAN_NAN_ID);
		return QDF_STATUS_E_INVAL;
	}

	qdf_spin_lock_bh(&ndi_vdev_priv->lock);
	curr_ndi_state = ndi_vdev_priv->state;
	/* Nothing to do if NDI is in DELETING or DATA_END state */
	if (curr_ndi_state == NAN_DATA_NDI_DELETING_STATE ||
	    curr_ndi_state == NAN_DATA_END_STATE) {
		qdf_spin_unlock_bh(&ndi_vdev_priv->lock);
		wlan_objmgr_vdev_release_ref(ndi_vdev, WLAN_NAN_ID);
		return QDF_STATUS_SUCCESS;
	}
	ndi_vdev_priv->state = NAN_DATA_END_STATE;
	qdf_spin_unlock_bh(&ndi_vdev_priv->lock);

	request = osif_request_alloc(&params);
	if (!request) {
		nan_err("Request allocation failure");
		status = QDF_STATUS_E_NOMEM;
		goto cleanup;
	}
	ndi_vdev_priv->disable_context = osif_request_cookie(request);

	req.vdev = ndi_vdev;
	status = ucfg_nan_req_processor(NULL, &req, NDP_END_ALL);

	if (QDF_IS_STATUS_ERROR(status)) {
		nan_err("Unable to disable NDP's on NDI");
		wlan_objmgr_vdev_release_ref(ndi_vdev, WLAN_NAN_ID);
		goto cleanup;
	}

	nan_debug("Disabling all NDP's on NDI vdev id - %d", ndi_vdev_id);

	err = osif_request_wait_for_response(request);
	if (err) {
		nan_err("Disabling NDP's timed out waiting for confirmation");
		status = QDF_STATUS_E_TIMEOUT;
		goto cleanup;
	}

	event = osif_request_priv(request);
	if (!event->ndp_termination_in_progress) {
		nan_err("Failed to terminate NDP's on NDI");
		status = QDF_STATUS_E_FAILURE;
	} else {
		/*
		 * Host can assume NDP delete is successful and
		 * remove policy mgr entry
		 */
		policy_mgr_decr_session_set_pcl(psoc, QDF_NDI_MODE,
						ndi_vdev_id);
	}

cleanup:
	/* Restore original NDI state in case of failure */
	qdf_spin_lock_bh(&ndi_vdev_priv->lock);
	if (QDF_IS_STATUS_SUCCESS(status))
		ndi_vdev_priv->state = NAN_DATA_DISCONNECTED_STATE;
	else
		ndi_vdev_priv->state = curr_ndi_state;
	qdf_spin_unlock_bh(&ndi_vdev_priv->lock);

	if (request)
		osif_request_put(request);

	return status;
}

QDF_STATUS
ucfg_nan_check_and_disable_unsupported_ndi(struct wlan_objmgr_psoc *psoc,
					   bool force)
{
	uint32_t ndi_count, first_ndi_vdev_id, i;
	QDF_STATUS status;

	if (!psoc) {
		nan_err("psoc object is NULL, no action will be taken");
		return QDF_STATUS_E_INVAL;
	}

	if (!ucfg_is_ndi_dbs_supported(psoc))
		return QDF_STATUS_SUCCESS;

	ndi_count = policy_mgr_mode_specific_connection_count(psoc, PM_NDI_MODE,
							      NULL);
	/* NDP force disable is done for unsupported concurrencies: NDI+SAP */
	if (force) {
		nan_warn("Force disable all NDPs");
		for (i = 0; i < ndi_count; i++) {
			first_ndi_vdev_id =
				policy_mgr_mode_specific_vdev_id(psoc,
								 PM_NDI_MODE);
			status = ucfg_nan_disable_ndi(psoc, first_ndi_vdev_id);
			if (QDF_IS_STATUS_ERROR(status))
				return status;
		}
		return QDF_STATUS_SUCCESS;
	}

	if (ndi_count < 2) {
		nan_debug("No more than one NDI is active, nothing to do...");
		return QDF_STATUS_SUCCESS;
	}

	/*
	 * At least 2 NDI active concurrencies exist. Disable all NDP's on the
	 * first NDI to support an incoming connection.
	 */
	first_ndi_vdev_id = policy_mgr_mode_specific_vdev_id(psoc, PM_NDI_MODE);
	status = ucfg_nan_disable_ndi(psoc, first_ndi_vdev_id);

	return status;
}

QDF_STATUS ucfg_ndi_remove_entry_from_policy_mgr(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct nan_psoc_priv_obj *psoc_priv_obj;
	struct nan_vdev_priv_obj *vdev_priv_obj = nan_get_vdev_priv_obj(vdev);
	enum nan_datapath_state state;
	uint32_t active_ndp_peers;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		nan_err("can't get psoc");
		return QDF_STATUS_E_FAILURE;
	}

	psoc_priv_obj = nan_get_psoc_priv_obj(psoc);
	if (!psoc_priv_obj) {
		nan_err("psoc_priv_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!vdev_priv_obj) {
		nan_err("priv_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock_bh(&vdev_priv_obj->lock);
	state = vdev_priv_obj->state;
	active_ndp_peers = vdev_priv_obj->active_ndp_peers;
	qdf_spin_unlock_bh(&vdev_priv_obj->lock);

	if (state == NAN_DATA_NDI_DELETED_STATE &&
	    psoc_priv_obj->nan_caps.ndi_dbs_supported &&
	    active_ndp_peers) {
		nan_info("Delete NDP peers: %u and remove NDI from policy mgr",
			 active_ndp_peers);
		policy_mgr_decr_session_set_pcl(psoc, QDF_NDI_MODE,
						wlan_vdev_get_id(vdev));
	}

	return QDF_STATUS_SUCCESS;
}

bool ucfg_nan_is_enable_disable_in_progress(struct wlan_objmgr_psoc *psoc)
{
	enum nan_disc_state nan_state;

	nan_state = nan_get_discovery_state(psoc);
	if (nan_state == NAN_DISC_ENABLE_IN_PROGRESS ||
	    nan_state == NAN_DISC_DISABLE_IN_PROGRESS) {
		nan_info("NAN enable/disable is in progress, state: %u",
			 nan_state);
		return true;
	}

	return false;
}

#ifdef NDP_SAP_CONCURRENCY_ENABLE
/**
 * is_sap_ndp_concurrency_allowed() - Is SAP+NDP allowed
 *
 * Return: True if the NDP_SAP_CONCURRENCY_ENABLE feature define
 *	   is enabled, false otherwise.
 */
static inline bool is_sap_ndp_concurrency_allowed(void)
{
	return true;
}
#else
static inline bool is_sap_ndp_concurrency_allowed(void)
{
	return false;
}
#endif

bool ucfg_nan_is_sta_ndp_concurrency_allowed(struct wlan_objmgr_psoc *psoc,
					     struct wlan_objmgr_vdev *vdev)
{
	uint8_t vdev_id_list[MAX_NUMBER_OF_CONC_CONNECTIONS];
	uint32_t freq_list[MAX_NUMBER_OF_CONC_CONNECTIONS];
	uint32_t ndi_cnt, sta_cnt, id;

	sta_cnt = policy_mgr_mode_specific_connection_count(psoc,
							    PM_STA_MODE, NULL);
	/* Allow if STA is not in connected state */
	if (!sta_cnt)
		return true;

	/* Reject if STA+STA is present */
	if (sta_cnt > 1) {
		nan_err("STA+STA+NDP concurrency is not allowed");
		return false;
	}

	/*
	 * SAP+NDP concurrency is already validated in hdd_is_ndp_allowed().
	 * If SAP+NDP concurrency is enabled, return true from here to avoid
	 * failure.
	 */
	if (is_sap_ndp_concurrency_allowed())
		return true;

	ndi_cnt = policy_mgr_get_mode_specific_conn_info(psoc,
							 freq_list,
							 vdev_id_list,
							 PM_NDI_MODE);

	/* Allow if no other NDP peers are present on the NDIs */
	if (!ndi_cnt)
		return true;

	/*
	 * Allow NDP creation if the current NDP request is on
	 * the NDI which already has an NDP by checking the vdev id of
	 * the NDIs
	 */
	for (id = 0; id < ndi_cnt; id++)
		if (wlan_vdev_get_id(vdev) == vdev_id_list[id])
			return true;

	return false;
}

bool ucfg_nan_is_vdev_creation_allowed(struct wlan_objmgr_psoc *psoc)
{
	struct nan_psoc_priv_obj *psoc_nan_obj;

	psoc_nan_obj = nan_get_psoc_priv_obj(psoc);
	if (!psoc_nan_obj) {
		nan_err("psoc_nan_obj is null");
		return false;
	}

	return psoc_nan_obj->nan_caps.nan_vdev_allowed;
}
