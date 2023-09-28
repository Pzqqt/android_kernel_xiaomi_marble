/*
 * Copyright (c) 2012-2015, 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_cm_main.h
 *
 * This header file maintain structures required for connection mgr core
 */

#ifndef __WLAN_CM_MAIN_H__
#define __WLAN_CM_MAIN_H__

#include "include/wlan_vdev_mlme.h"
#include <qdf_event.h>
#include <wlan_cm_public_struct.h>

/* Max candidate/attempts to be tried to connect */
#define CM_MAX_CONNECT_ATTEMPTS 5
/*
 * Default connect timeout to consider 3 sec join timeout + 5 sec auth timeout +
 * 2 sec assoc timeout + 5 sec buffer for vdev related timeouts.
 */
#define CM_MAX_PER_CANDIDATE_CONNECT_TIMEOUT 15000

/*
 * Default max retry attempts to be tried for a candidate.
 * In SAE connection this value will be overwritten from the sae_connect_retries
 * INI
 */
#define CM_MAX_CANDIDATE_RETRIES 1
/* Max time to wait for scan for SSID */
#define CM_SCAN_MAX_TIME 5000
/* Max connect/disconnect/roam req that can be queued at a time */
#define CM_MAX_REQ 5

/**
 * enum wlan_cm_sm_state - Connection manager states
 * @WLAN_CM_S_INIT:                     Default state, IDLE state
 * @WLAN_CM_S_CONNECTING:               State when connect request comes
 * @WLAN_CM_S_CONNECTED:                State when connect is complete
 * @WLAN_CM_S_DISCONNECTING:            State when disconnect request comes
 * @WLAN_CM_S_ROAMING:                  State when roaming is in progress
 * @WLAN_CM_S_MAX:                      Max State
 * @WLAN_CM_SS_IDLE:                    Idle state (no substate)
 * @WLAN_CM_SS_JOIN_PENDING:            Connect request not serialized
 * @WLAN_CM_SS_SCAN:                    Scan for SSID state
 * @WLAN_CM_SS_JOIN_ACTIVE:             Conenct request activated
 * @WLAN_CM_SS_PREAUTH:                 Roam substate of preauth stage
 * @WLAN_CM_SS_REASSOC:                 Roam substate for reassoc state
 * @WLAN_CM_SS_ROAM_STARTED:            Roaming in progress (LFR 3.0)
 * @WLAN_CM_SS_ROAM_SYNC:               Roam sync indication from FW
 * @WLAN_CM_SS_MAX:                     Max Substate
 */
enum wlan_cm_sm_state {
	WLAN_CM_S_INIT = 0,
	WLAN_CM_S_CONNECTING = 1,
	WLAN_CM_S_CONNECTED = 2,
	WLAN_CM_S_DISCONNECTING = 3,
	WLAN_CM_S_ROAMING = 4,
	WLAN_CM_S_MAX = 5,
	WLAN_CM_SS_IDLE = 6,
	WLAN_CM_SS_JOIN_PENDING = 7,
	WLAN_CM_SS_SCAN = 8,
	WLAN_CM_SS_JOIN_ACTIVE = 9,
	WLAN_CM_SS_PREAUTH = 10,
	WLAN_CM_SS_REASSOC = 11,
	WLAN_CM_SS_ROAM_STARTED = 12,
	WLAN_CM_SS_ROAM_SYNC = 13,
	WLAN_CM_SS_MAX = 14,
};

/**
 * struct cm_state_sm - connection manager sm
 * @cm_sm_lock: sm lock
 * @sm_hdl: sm handlers
 * @cm_state: current state
 * @cm_substate: current substate
 */
struct cm_state_sm {
#ifdef WLAN_CM_USE_SPINLOCK
	qdf_spinlock_t cm_sm_lock;
#else
	qdf_mutex_t cm_sm_lock;
#endif
	struct wlan_sm *sm_hdl;
	enum wlan_cm_sm_state cm_state;
	enum wlan_cm_sm_state cm_substate;
};

/**
 * struct cm_connect_req - connect req stored in connect manager
 * @cm_id: Connect manager id
 * @scan_id: scan id for scan for ssid
 * @req: connect req from osif
 * @candidate_list: candidate list
 * @cur_candidate: current candidate
 * @cur_candidate_retries: attempts for current candidate
 * @connect_attempts: number of connect attempts tried
 * @connect_active_time: timestamp when connect became active
 */
struct cm_connect_req {
	wlan_cm_id cm_id;
	wlan_scan_id scan_id;
	struct wlan_cm_connect_req req;
	qdf_list_t *candidate_list;
	struct scan_cache_node *cur_candidate;
	uint8_t cur_candidate_retries;
	uint8_t connect_attempts;
	qdf_time_t connect_active_time;
};

/**
 * struct cm_roam_req - roam req stored in connect manager
 * @cm_id: Connect manager id
 * @req: roam req from osif
 * @candidate_list: candidate list
 * @cur_candidate: current candidate
 * @num_preauth_retry: retry times for the same candidate
 */
struct cm_roam_req {
	wlan_cm_id cm_id;
	struct wlan_cm_roam_req req;
	qdf_list_t *candidate_list;
	struct scan_cache_node *cur_candidate;
#ifdef WLAN_FEATURE_PREAUTH_ENABLE
	uint8_t num_preauth_retry;
#endif
};

/**
 * struct cm_disconnect_req - disconnect req
 * @cm_id: Connect manager id
 * @req: disconnect connect req from osif
 */
struct cm_disconnect_req {
	wlan_cm_id cm_id;
	struct wlan_cm_disconnect_req req;
};

/**
 * struct cm_req - connect manager req
 * @node: connection manager req node
 * @cm_id: cm id
 * @failed_req: set if req failed before serialization,
 * with a commands pending before it, ie this is the latest command which failed
 * but still some operation(req) is pending.
 * @connect_req: connect req
 * @discon_req: disconnect req
 * @roam_req: roam req
 */
struct cm_req {
	qdf_list_node_t node;
	wlan_cm_id cm_id;
	bool failed_req;
	union {
		struct cm_connect_req connect_req;
		struct cm_disconnect_req discon_req;
		struct cm_roam_req roam_req;
	};
};

/**
 * enum cm_req_del_type - Context in which a request is removed from
 * connection manager request list
 * @CM_REQ_DEL_ACTIVE: Remove request from active queue
 * @CM_REQ_DEL_PENDING: Remove request from pending queue
 * @CM_REQ_DEL_FLUSH: Request removed due to request list flush
 */
enum cm_req_del_type {
	CM_REQ_DEL_ACTIVE,
	CM_REQ_DEL_PENDING,
	CM_REQ_DEL_FLUSH,
	CM_REQ_DEL_MAX,
};

#ifdef SM_ENG_HIST_ENABLE

#define CM_REQ_HISTORY_SIZE 30

/**
 * struct cm_req_history_info - History element structure
 * @cm_id: Request id
 * @add_time: Timestamp when the request was added to the list
 * @del_time: Timestamp when the request was removed from list
 * @add_cm_state: Conn_SM state when req was added
 * @del_cm_state: Conn_SM state when req was deleted
 * @del_type: Context in which delete was triggered. i.e active removal,
 * pending removal or flush from queue.
 */
struct cm_req_history_info {
	wlan_cm_id cm_id;
	uint64_t add_time;
	uint64_t del_time;
	enum wlan_cm_sm_state add_cm_state;
	enum wlan_cm_sm_state del_cm_state;
	enum cm_req_del_type del_type;
};

/**
 * struct cm_req_history - Connection manager history
 * @cm_req_hist_lock: CM request history lock
 * @index: Index of next entry that will be updated
 * @data: Array of history element
 */
struct cm_req_history {
	qdf_spinlock_t cm_req_hist_lock;
	uint8_t index;
	struct cm_req_history_info data[CM_REQ_HISTORY_SIZE];
};
#endif

/**
 * struct cnx_mgr - connect manager req
 * @vdev: vdev back pointer
 * @sm: state machine
 * @active_cm_id: cm_id of the active command, if any active command present
 * @preauth_in_progress: is roaming in preauth state, set during preauth state,
 * this is used to get which command to flush from serialization during
 * host roaming.
 * @req_list: connect/disconnect req list
 * @cm_req_lock: lock to manupulate/read the cm req list
 * @disconnect_count: disconnect count
 * @connect_count: connect count
 * @force_rsne_override: if QCA_WLAN_VENDOR_ATTR_CONFIG_RSN_IE is set by
 * framework
 * @global_cmd_id: global cmd id for getting cm id for connect/disconnect req
 * @max_connect_attempts: Max attempts to be tried for a connect req
 * @connect_timeout: Connect timeout value in milliseconds
 * @scan_requester_id: scan requester id.
 * @disconnect_complete: disconnect completion wait event
 * @ext_cm_ptr: connection manager ext pointer
 * @history: Holds the connection manager history
 */
struct cnx_mgr {
	struct wlan_objmgr_vdev *vdev;
	struct cm_state_sm sm;
	wlan_cm_id active_cm_id;
	bool preauth_in_progress;
	qdf_list_t req_list;
#ifdef WLAN_CM_USE_SPINLOCK
	qdf_spinlock_t cm_req_lock;
#else
	qdf_mutex_t cm_req_lock;
#endif
	uint8_t disconnect_count;
	uint8_t connect_count;
	bool force_rsne_override;
	qdf_atomic_t global_cmd_id;
	uint8_t max_connect_attempts;
	uint32_t connect_timeout;
	wlan_scan_requester scan_requester_id;
	qdf_event_t disconnect_complete;
	cm_ext_t *ext_cm_ptr;
#ifdef SM_ENG_HIST_ENABLE
	struct cm_req_history req_history;
#endif
#ifndef CONN_MGR_ADV_FEATURE
	void (*cm_candidate_advance_filter)(struct wlan_objmgr_vdev *vdev,
					    struct scan_filter *filter);
	void (*cm_candidate_list_custom_sort)(struct wlan_objmgr_vdev *vdev,
					      qdf_list_t *list);
#endif
};

/**
 * struct vdev_op_search_arg - vdev op search arguments
 * @current_vdev_id: current vdev id
 * @sap_go_vdev_id: sap/go vdev id
 * @sta_cli_vdev_id: sta/p2p client vdev id
 */
struct vdev_op_search_arg {
	uint8_t current_vdev_id;
	uint8_t sap_go_vdev_id;
	uint8_t sta_cli_vdev_id;
};

/**
 * wlan_cm_init() - Invoke connection manager init
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API allocates CM and init
 *
 * Return: SUCCESS on successful allocation
 *         FAILURE, if registration fails
 */
QDF_STATUS wlan_cm_init(struct vdev_mlme_obj *vdev_mlme);

/**
 * wlan_cm_deinit() - Invoke connection manager deinit
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API destroys CM
 *
 * Return: SUCCESS on successful deletion
 *         FAILURE, if deletion fails
 */
QDF_STATUS wlan_cm_deinit(struct vdev_mlme_obj *vdev_mlme);
#endif /* __WLAN_CM_MAIN_H__ */
