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
 * DOC: wlan_cm_main.h
 *
 * This header file maintain structures required for connection mgr core
 */

#ifndef __WLAN_CM_MAIN_H__
#define __WLAN_CM_MAIN_H__

#include "include/wlan_vdev_mlme.h"

#ifdef FEATURE_CM_ENABLE
#include <wlan_cm_public_struct.h>

/* Max candidate/attempts to be tried to connect */
#define CM_MAX_CONNECT_ATTEMPTS 5
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
 * @rsn_ie: rsn_ie in connect req
 * @candidate_list: candidate list
 * @cur_candidate: current candidate
 * @cur_candidate_retries: attempts for current candidate
 * @connect_attempts: number of connect attempts tried
 */
struct cm_connect_req {
	wlan_cm_id cm_id;
	wlan_scan_id scan_id;
	struct wlan_cm_connect_req req;
	struct element_info rsn_ie;
	qdf_list_t *candidate_list;
	struct scan_cache_node *cur_candidate;
	uint8_t cur_candidate_retries;
	uint8_t connect_attempts;
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
 * @disconnect_req: disconnect req
 */
struct cm_req {
	qdf_list_node_t node;
	wlan_cm_id cm_id;
	bool failed_req;
	union {
		struct cm_connect_req connect_req;
		struct cm_disconnect_req discon_req;
	};
};

/**
 * struct connect_ies - connect related ies stored in vdev, set by osif/user
 * @auth_ft_ies: auth ft ies received during preauth phase
 * @reassoc_ft_ies: reassoc ft ies received during reassoc phase
 * @cck_ie: cck ie for cck connection
 * @discon_ie: disconnect ie to be sent in disassoc/deauth req
 */
struct connect_ies {
	struct element_info auth_ft_ies;
	struct element_info reassoc_ft_ies;
#ifdef FEATURE_WLAN_ESE
	struct element_info cck_ie;
#endif
	struct element_info discon_ie;
};

/**
 * struct cnx_mgr - connect manager req
 * @vdev: vdev back pointer
 * @sm: state machine
 * @active_cm_id: cm_id of the active command, if any active command present
 * @req_list: connect/disconnect req list
 * @cm_req_lock: lock to manupulate/read the cm req list
 * @disconnect_count: disconnect count
 * @connect_count: connect count
 * @force_rsne_override: if QCA_WLAN_VENDOR_ATTR_CONFIG_RSN_IE is set by
 * framework
 * @req_ie: request ies for connect/disconnect set by osif/user separately from
 * connect req
 * @global_cmd_id: global cmd id for getting cm id for connect/disconnect req
 * @max_connect_attempts: Max attempts to be tried for a connect req
 * @scan_requester_id: scan requester id.
 */
struct cnx_mgr {
	struct wlan_objmgr_vdev *vdev;
	struct cm_state_sm sm;
	wlan_cm_id active_cm_id;
	qdf_list_t req_list;
#ifdef WLAN_CM_USE_SPINLOCK
	qdf_spinlock_t cm_req_lock;
#else
	qdf_mutex_t cm_req_lock;
#endif
	uint8_t disconnect_count;
	uint8_t connect_count;
	bool force_rsne_override;
	struct connect_ies req_ie;
	qdf_atomic_t global_cmd_id;
	uint8_t max_connect_attempts;
	wlan_scan_requester scan_requester_id;
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
#else

static inline QDF_STATUS wlan_cm_init(struct vdev_mlme_obj *vdev_mlme)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wlan_cm_deinit(struct vdev_mlme_obj *vdev_mlme)
{
	return QDF_STATUS_SUCCESS;
}

#endif /* FEATURE_CM_ENABLE */

#endif /* __WLAN_CM_MAIN_H__ */
