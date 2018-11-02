/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * DOC: contains declaration of common utility APIs and private structs to be
 * used in NAN modules
 */

#ifdef WLAN_FEATURE_NAN_CONVERGENCE
#ifndef _WLAN_NAN_MAIN_I_H_
#define _WLAN_NAN_MAIN_I_H_

#include "qdf_types.h"
#include "qdf_status.h"
#include "nan_public_structs.h"
#include "wlan_objmgr_cmn.h"

struct wlan_objmgr_vdev;
struct wlan_objmgr_psoc;
struct scheduler_msg;

#define nan_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_NAN, params)
#define nan_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_NAN, params)
#define nan_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_NAN, params)
#define nan_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_NAN, params)
#define nan_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_NAN, params)
#define nan_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_NAN, params)

#define nan_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_NAN, params)
#define nan_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_NAN, params)
#define nan_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_NAN, params)
#define nan_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_NAN, params)
#define nan_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_NAN, params)

#ifndef MAX_PEERS
#define MAX_PEERS 32
#endif

/**
 * struct nan_cfg_params - NAN INI config params
 * @enable: NAN feature enable
 * @dp_enable: NAN Datapath feature enable
 * @ndi_ch: NAN Datapath channel
 * @ndi_mac_randomize: Randomize NAN datapath interface MAC
 */
struct nan_cfg_params {
#ifdef WLAN_FEATURE_NAN
	bool enable;
#endif
#ifdef WLAN_FEATURE_NAN_DATAPATH
	bool dp_enable;
	uint32_t ndi_ch;
	bool ndi_mac_randomize;
#endif
};

/**
 * struct nan_psoc_priv_obj - nan private psoc obj
 * @lock: lock to be acquired before reading or writing to object
 * @cb_obj: struct contaning callback pointers
 * @cfg_param: NAN Config parameters in INI
 * @nan_caps: NAN Target capabilities
 * @tx_ops: NAN Tx operations
 * @rx_ops: NAN Rx operations
 */
struct nan_psoc_priv_obj {
	qdf_spinlock_t lock;
	struct nan_callbacks cb_obj;
	struct nan_cfg_params cfg_param;
	struct nan_tgt_caps nan_caps;
	struct wlan_nan_tx_ops tx_ops;
	struct wlan_nan_rx_ops rx_ops;
};

/**
 * struct nan_vdev_priv_obj - nan private vdev obj
 * @lock: lock to be acquired before reading or writing to object
 * @state: Current state of NDP
 * @active_ndp_sessions: active ndp sessions per adapter
 * @active_ndp_peers: number of active ndp peers
 * @ndp_create_transaction_id: transaction id for create req
 * @ndp_delete_transaction_id: transaction id for delete req
 * @ndi_delete_rsp_reason: reason code for ndi_delete rsp
 * @ndi_delete_rsp_status: status for ndi_delete rsp
 */
struct nan_vdev_priv_obj {
	qdf_spinlock_t lock;
	enum nan_datapath_state state;
	/* idx in following array should follow conn_info.peerMacAddress */
	uint32_t active_ndp_sessions[MAX_PEERS];
	uint32_t active_ndp_peers;
	uint16_t ndp_create_transaction_id;
	uint16_t ndp_delete_transaction_id;
	uint32_t ndi_delete_rsp_reason;
	uint32_t ndi_delete_rsp_status;
};

/**
 * nan_release_cmd: frees resources for NAN command.
 * @in_req: pointer to msg buffer to be freed
 * @req_type: type of request
 *
 * Return: None
 */
void nan_release_cmd(void *in_req, uint32_t req_type);

/**
 * nan_scheduled_msg_handler: callback pointer to be called when scheduler
 * starts executing enqueued NAN command.
 * @msg: pointer to msg
 *
 * Return: status of operation
 */
QDF_STATUS nan_scheduled_msg_handler(struct scheduler_msg *msg);

/*
 * nan_event_handler: function to process events from firmware
 * @msg: message received from lmac
 *
 * Return: status of operation
 */
QDF_STATUS nan_event_handler(struct scheduler_msg *msg);

#endif /* _WLAN_NAN_MAIN_I_H_ */
#endif /* WLAN_FEATURE_NAN_CONVERGENCE */
