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
 * DOC: Defines main P2P functions & structures
 */

#ifndef _WLAN_P2P_MAIN_H_
#define _WLAN_P2P_MAIN_H_

#include <qdf_trace.h>
#include <qdf_types.h>
#include <qdf_event.h>
#include <qdf_list.h>
#include <qdf_lock.h>

#define MAX_QUEUE_LENGTH 20
#define P2P_NOA_ATTR_IND 0x1090
#define P2P_MODULE_NAME  "P2P"
#define P2P_INVALID_VDEV_ID 0xFFFFFFFF

#define p2p_log(level, args...) \
	QDF_TRACE(QDF_MODULE_ID_P2P, level, ## args)
#define p2p_logl(level, format, args...) \
	p2p_log(level, FL(format), ## args)

#define p2p_debug(format, args ...) \
	p2p_logl(QDF_TRACE_LEVEL_DEBUG, format, ## args)
#define p2p_info(format, args ...) \
	p2p_logl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define p2p_warn(format, args ...) \
	p2p_logl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define p2p_err(format, args ...) \
	p2p_logl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define p2p_alert(format, args ...) \
	p2p_logl(QDF_TRACE_LEVEL_FATAL, format, ## args)

struct scheduler_msg;
struct p2p_tx_cnf;
struct p2p_rx_mgmt_frame;
struct p2p_lo_event;
struct p2p_start_param;
struct p2p_noa_info;
struct tx_action_context;

/**
 * enum p2p_cmd_type - P2P request type
 * @P2P_ROC_REQ:            P2P roc request
 * @P2P_CANCEL_ROC_REQ:     Cancel P2P roc request
 * @P2P_MGMT_TX:            P2P tx action frame request
 * @P2P_MGMT_TX_CANCEL:     Cancel tx action frame request
 */
enum p2p_cmd_type {
	P2P_ROC_REQ = 0,
	P2P_CANCEL_ROC_REQ,
	P2P_MGMT_TX,
	P2P_MGMT_TX_CANCEL,
};

/**
 * enum p2p_event_type - P2P event type
 * @P2P_EVENT_SCAN_EVENT:        P2P scan event
 * @P2P_EVENT_MGMT_TX_ACK_CNF:   P2P mgmt tx confirm frame
 * @P2P_EVENT_RX_MGMT:           P2P rx mgmt frame
 * @P2P_EVENT_LO_STOPPED:        P2P listen offload stopped event
 * @P2P_EVENT_NOA:               P2P noa event
 */
enum p2p_event_type {
	P2P_EVENT_SCAN_EVENT = 0,
	P2P_EVENT_MGMT_TX_ACK_CNF,
	P2P_EVENT_RX_MGMT,
	P2P_EVENT_LO_STOPPED,
	P2P_EVENT_NOA,
};

/**
 * struct p2p_tx_conf_event - p2p tx confirm event
 * @p2p_soc_obj:        p2p soc private object
 * @tx_cnf:             p2p tx confirm structure
 * @tx_ctx:             tx context
 */
struct p2p_tx_conf_event {
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct p2p_tx_cnf *tx_cnf;
	struct tx_action_context *tx_ctx;
};

/**
 * struct p2p_rx_mgmt_event - p2p rx mgmt frame event
 * @p2p_soc_obj:        p2p soc private object
 * @rx_mgmt:            p2p rx mgmt frame structure
 */
struct p2p_rx_mgmt_event {
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct p2p_rx_mgmt_frame *rx_mgmt;
};

/**
 * struct p2p_lo_stop_event - p2p listen offload stop event
 * @p2p_soc_obj:        p2p soc private object
 * @lo_event:           p2p lo stop structure
 */
struct p2p_lo_stop_event {
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct p2p_lo_event *lo_event;
};

/**
 * struct p2p_noa_event - p2p noa event
 * @p2p_soc_obj:        p2p soc private object
 * @noa_info:           p2p noa information structure
 */
struct p2p_noa_event {
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct p2p_noa_info *noa_info;
};

#ifdef WLAN_FEATURE_P2P_DEBUG
/**
 * enum p2p_connection_status - p2p connection status
 * @P2P_NOT_ACTIVE:                P2P not active status
 * @P2P_GO_NEG_PROCESS:            P2P GO negotiation in process
 * @P2P_GO_NEG_COMPLETED:          P2P GO negotiation complete
 * @P2P_CLIENT_CONNECTING_STATE_1: P2P client connecting state 1
 * @P2P_GO_COMPLETED_STATE:        P2P GO complete state
 * @P2P_CLIENT_CONNECTED_STATE_1:  P2P client connected state 1
 * @P2P_CLIENT_DISCONNECTED_STATE: P2P client disconnected state
 * @P2P_CLIENT_CONNECTING_STATE_2: P2P client connecting state 2
 * @P2P_CLIENT_COMPLETED_STATE:    P2P client complete state
 */
enum p2p_connection_status {
	P2P_NOT_ACTIVE,
	P2P_GO_NEG_PROCESS,
	P2P_GO_NEG_COMPLETED,
	P2P_CLIENT_CONNECTING_STATE_1,
	P2P_GO_COMPLETED_STATE,
	P2P_CLIENT_CONNECTED_STATE_1,
	P2P_CLIENT_DISCONNECTED_STATE,
	P2P_CLIENT_CONNECTING_STATE_2,
	P2P_CLIENT_COMPLETED_STATE
};
#endif

/**
 * struct p2p_soc_priv_obj - Per SoC p2p private object
 * @soc:              Pointer to SoC context
 * @roc_q:            Queue for pending roc requests
 * @tx_q_roc:         Queue for tx frames waiting for RoC
 * @tx_q_ack:         Queue for tx frames waiting for ack
 * @scan_req_id:      Scan requestor id
 * @start_param:      Start parameters, include callbacks and user
 *                    data to HDD
 * @cancel_roc_done:  Cancel roc done event
 * @roc_runtime_lock: Runtime lock for roc request
 * @p2p_cb: Callbacks to protocol stack
 * @cur_roc_vdev_id:  Vdev id of current roc
 * @connection_status:Global P2P connection status
 */
struct p2p_soc_priv_obj {
	struct wlan_objmgr_psoc *soc;
	qdf_list_t roc_q;
	qdf_list_t tx_q_roc;
	qdf_list_t tx_q_ack;
	wlan_scan_requester scan_req_id;
	struct p2p_start_param *start_param;
	qdf_event_t cancel_roc_done;
	qdf_runtime_lock_t roc_runtime_lock;
	struct p2p_protocol_callbacks p2p_cb;
	uint32_t cur_roc_vdev_id;
#ifdef WLAN_FEATURE_P2P_DEBUG
	enum p2p_connection_status connection_status;
#endif
};

/**
 * struct p2p_vdev_priv_obj - Per vdev p2p private object
 * @vdev:               Pointer to vdev context
 * @noa_info:           NoA information
 * @noa_status:         NoA status i.e. Enabled / Disabled (TRUE/FALSE)
 * @non_p2p_peer_count: Number of legacy stations connected to this GO
 */
struct p2p_vdev_priv_obj {
	struct   wlan_objmgr_vdev *vdev;
	struct   p2p_noa_info *noa_info;
	bool     noa_status;
	uint16_t non_p2p_peer_count;
};

/**
 * struct p2p_noa_attr - p2p noa attribute
 * @rsvd1:             reserved bits 1
 * @opps_ps:           opps ps state of the AP
 * @ct_win:            ct window in TUs
 * @index:             identifies instance of NOA su element
 * @rsvd2:             reserved bits 2
 * @noa1_count:        interval count of noa1
 * @noa1_duration:     absent period duration of noa1
 * @noa1_interval:     absent period interval of noa1
 * @noa1_start_time:   32 bit tsf time of noa1
 * @rsvd3:             reserved bits 3
 * @noa2_count:        interval count of noa2
 * @noa2_duration:     absent period duration of noa2
 * @noa2_interval:     absent period interval of noa2
 * @noa2_start_time:   32 bit tsf time of noa2
 */
struct p2p_noa_attr {
	uint32_t rsvd1:16;
	uint32_t ct_win:7;
	uint32_t opps_ps:1;
	uint32_t index:8;
	uint32_t rsvd2:24;
	uint32_t noa1_count:8;
	uint32_t noa1_duration;
	uint32_t noa1_interval;
	uint32_t noa1_start_time;
	uint32_t rsvd3:24;
	uint32_t noa2_count:8;
	uint32_t noa2_duration;
	uint32_t noa2_interval;
	uint32_t noa2_start_time;
};

/**
 * p2p_component_init() - P2P component initialization
 *
 * This function registers psoc/vdev create/delete handler.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_component_init(void);

/**
 * p2p_component_deinit() - P2P component de-init
 *
 * This function deregisters psoc/vdev create/delete handler.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_component_deinit(void);

/**
 * p2p_psoc_object_open() - Open P2P component
 * @soc: soc context
 *
 * This function initialize p2p psoc object
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_psoc_object_open(struct wlan_objmgr_psoc *soc);

/**
 * p2p_psoc_object_close() - Close P2P component
 * @soc: soc context
 *
 * This function de-init p2p psoc object.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_psoc_object_close(struct wlan_objmgr_psoc *soc);

/**
 * p2p_psoc_start() - Start P2P component
 * @soc: soc context
 * @req: P2P start parameters
 *
 * This function sets up layer call back in p2p psoc object
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_psoc_start(struct wlan_objmgr_psoc *soc,
	struct p2p_start_param *req);

/**
 * p2p_psoc_stop() - Stop P2P component
 * @soc: soc context
 *
 * This function clears up layer call back in p2p psoc object.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_psoc_stop(struct wlan_objmgr_psoc *soc);

/**
 * p2p_process_cmd() - Process P2P messages in OS interface queue
 * @msg: message information
 *
 * This function is main handler for P2P messages in OS interface
 * queue, it gets called by message scheduler.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_process_cmd(struct scheduler_msg *msg);

/**
 * p2p_process_evt() - Process P2P messages in target interface queue
 * @msg: message information
 *
 * This function is main handler for P2P messages in target interface
 * queue, it gets called by message scheduler.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_process_evt(struct scheduler_msg *msg);

/**
 * p2p_process_lo_stop() - Process lo stop event
 * @lo_stop_event: listen offload stop event information
 *
 * This function handles listen offload stop event and deliver this
 * event to HDD layer by registered callback.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_process_lo_stop(
	struct p2p_lo_stop_event *lo_stop_event);

/**
 * p2p_process_noa() - Process noa event
 * @noa_event: noa event information
 *
 * This function handles noa event and save noa information in p2p
 * vdev object.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_process_noa(struct p2p_noa_event *noa_event);

#ifdef WLAN_FEATURE_P2P_DEBUG
/**
 * p2p_status_scan() - Update P2P connection status
 * @vdev: vdev context
 *
 * This function updates P2P connection status when scanning
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_status_scan(struct wlan_objmgr_vdev *vdev);

/**
 * p2p_status_connect() - Update P2P connection status
 * @vdev:        vdev context
 *
 * This function updates P2P connection status when connecting.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_status_connect(struct wlan_objmgr_vdev *vdev);

/**
 * p2p_status_disconnect() - Update P2P connection status
 * @vdev:        vdev context
 *
 * This function updates P2P connection status when disconnecting.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_status_disconnect(struct wlan_objmgr_vdev *vdev);

/**
 * p2p_status_start_bss() - Update P2P connection status
 * @vdev:        vdev context
 *
 * This function updates P2P connection status when starting BSS.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_status_start_bss(struct wlan_objmgr_vdev *vdev);

/**
 * p2p_status_stop_bss() - Update P2P connection status
 * @vdev:        vdev context
 *
 * This function updates P2P connection status when stopping BSS.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS p2p_status_stop_bss(struct wlan_objmgr_vdev *vdev);
#else
static inline QDF_STATUS p2p_status_scan(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS p2p_status_connect(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS p2p_status_disconnect(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS p2p_status_start_bss(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS p2p_status_stop_bss(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_P2P_DEBUG */
#endif /* _WLAN_P2P_MAIN_H_ */
