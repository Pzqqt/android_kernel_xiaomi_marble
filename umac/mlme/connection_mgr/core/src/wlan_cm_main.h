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

/* Max candidate to be tried to connect */
#define CM_MAX_CANDIDATE_TO_BE_TRIED 5
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
 * enum wlan_cm_sm_evt - connection manager related events
 * @WLAN_CM_SM_EV_CONNECT_REQ:            Connect request event
 * @WLAN_CM_SM_EV_SCAN_FOR_SSID:          Event for scan for SSID
 * @WLAN_CM_SM_EV_SCAN_FOR_SSID_SUCCESS:  Scan for SSID success event
 * @WLAN_CM_SM_EV_SCAN_FOR_SSID_FAILURE:  Scan for SSID fail event
 * @WLAN_CM_SM_EV_CONNECT_START_REQ:      Connect start request after BSSID list
 *                                        is prepared
 * @WLAN_CM_SM_EV_CONNECT_START:          Connect request for a particular BSSID
 * @WLAN_CM_SM_EV_CONNECT_SUCCESS:        Connect success
 * @WLAN_CM_SM_EV_CONNECT_NEXT_CANDIDATE: Select next candidate for connection
 * @WLAN_CM_SM_EV_CONNECT_FAILURE:        Connect failed event
 * @WLAN_CM_SM_EV_DISCONNECT_REQ:         Disconnect request event
 * @WLAN_CM_SM_EV_DISCONNECT_START_REQ:   Start disconnect sequence
 * @WLAN_CM_SM_EV_DISCONNECT_START:       Disconnect process start event
 * @WLAN_CM_SM_EV_DISCONNECT_DONE:        Disconnect done event
 * @WLAN_CM_SM_EV_ROAM_START:             Roam start event
 * @WLAN_CM_SM_EV_ROAM_SYNC:              Roam sync event
 * @WLAN_CM_SM_EV_ROAM_INVOKE_FAIL:       Roam invoke fail event
 * @WLAN_CM_SM_EV_ROAM_HO_FAIL:           Hand off failed event
 * @WLAN_CM_SM_EV_PREAUTH_DONE:           Preauth is completed
 * @WLAN_CM_SM_EV_GET_NEXT_PREAUTH_AP:    Get next candidate as preauth failed
 * @WLAN_CM_SM_EV_PREAUTH_FAIL:           Preauth failed for all candidate
 * @WLAN_CM_SM_EV_START_REASSOC:          Start reassoc after preauth done
 * @WLAN_CM_SM_EV_REASSOC_DONE:           Reassoc completed
 * @WLAN_CM_SM_EV_REASSOC_FAILURE:        Reassoc failed
 * @WLAN_CM_SM_EV_ROAM_COMPLETE:          Roaming completed
 * @WLAN_CM_SM_EV_CONNECT_TIMEOUT:        Connect timeout event
 * @WLAN_CM_SM_EV_CONNECT_SER_FAIL:       Connect request failed to serialize
 * @WLAN_CM_SM_EV_HW_MODE_CHANGE_FAIL:    HW mode change failed event
 * @WLAN_CM_SM_EV_MAX:                    Max event
 */
enum wlan_cm_sm_evt {
	WLAN_CM_SM_EV_CONNECT_REQ = 0,
	WLAN_CM_SM_EV_SCAN_FOR_SSID = 1,
	WLAN_CM_SM_EV_SCAN_FOR_SSID_SUCCESS = 2,
	WLAN_CM_SM_EV_SCAN_FOR_SSID_FAILURE = 3,
	WLAN_CM_SM_EV_CONNECT_START_REQ = 4,
	WLAN_CM_SM_EV_CONNECT_START = 5,
	WLAN_CM_SM_EV_CONNECT_SUCCESS = 6,
	WLAN_CM_SM_EV_CONNECT_NEXT_CANDIDATE = 7,
	WLAN_CM_SM_EV_CONNECT_FAILURE = 8,
	WLAN_CM_SM_EV_DISCONNECT_REQ = 9,
	WLAN_CM_SM_EV_DISCONNECT_START_REQ = 10,
	WLAN_CM_SM_EV_DISCONNECT_START = 11,
	WLAN_CM_SM_EV_DISCONNECT_DONE = 12,
	WLAN_CM_SM_EV_ROAM_START = 13,
	WLAN_CM_SM_EV_ROAM_SYNC = 14,
	WLAN_CM_SM_EV_ROAM_INVOKE_FAIL = 15,
	WLAN_CM_SM_EV_ROAM_HO_FAIL = 16,
	WLAN_CM_SM_EV_PREAUTH_DONE = 17,
	WLAN_CM_SM_EV_GET_NEXT_PREAUTH_AP = 18,
	WLAN_CM_SM_EV_PREAUTH_FAIL = 19,
	WLAN_CM_SM_EV_START_REASSOC = 20,
	WLAN_CM_SM_EV_REASSOC_DONE = 21,
	WLAN_CM_SM_EV_REASSOC_FAILURE = 22,
	WLAN_CM_SM_EV_ROAM_COMPLETE = 23,
	WLAN_CM_SM_EV_CONNECT_TIMEOUT = 24,
	WLAN_CM_SM_EV_CONNECT_SER_FAIL = 25,
	WLAN_CM_SM_EV_HW_MODE_CHANGE_FAIL = 26,
	WLAN_CM_SM_EV_MAX,
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
 * @scan_timer: timer for scan for ssid to get completed
 * @hw_mode_timer: timer for hw mode chane to get completed
 * @req: connect req from osif
 * @candidate_list: candidate list
 * @cur_candidate: current candidate
 */
struct cm_connect_req {
	uint64_t cm_id;
	uint64_t scan_id;
	qdf_timer_t scan_timer;
	qdf_timer_t hw_mode_timer;
	struct wlan_cm_connect_req req;
	qdf_list_t candidate_list;
	struct scan_cache_node *cur_candidate;
};

/**
 * struct cm_disconnect_req - disconnect req
 * @cm_id: Connect manager id
 * @req: disconnect connect req from osif
 */
struct cm_disconnect_req {
	uint64_t cm_id;
	struct wlan_cm_disconnect_req req;
};

/**
 * struct cm_req - connect manager req
 * @node: connection manager req node
 * @source: req source
 * @cm_id: cm id
 * @connect_req: connect req
 * @disconnect_req: disconnect req
 */
struct cm_req {
	qdf_list_node_t node;
	enum wlan_cm_source source;
	uint64_t cm_id;
	union {
		struct cm_connect_req connect_req;
		struct cm_disconnect_req discon_req;
	};
};

/**
 * struct connect_req_ies - connect req ies stored in vdev
 * @additional_assoc_ie: assoc req additional IE to be appended to assoc req
 * @auth_ft_ies: auth ft ies received during preauth phase
 * @reassoc_ft_ies: reassoc ft ies received during reassoc phase
 * @cck_ie: cck ie for cck connection
 * @forced_rsn_ie: set if force_rsne_override is set from the connect IE. This
 * is used to store the invalid RSN IE from suplicant to enable testbed STA for
 * cert cases.
 */
struct connect_req_ies {
	struct element_info additional_assoc_ie;
	struct element_info auth_ft_ies;
	struct element_info reassoc_ft_ies;
	struct element_info cck_ie;
	struct element_info forced_rsn_ie;
};

#ifdef WLAN_FEATURE_FILS_SK
#define CM_FILS_MAX_HLP_DATA_LEN 2048
#define MAX_KEK_LENGTH 64
#define MAX_TK_LENGTH 32
#define MAX_GTK_LENGTH 255

/**
 * struct fils_connect_rsp_params - fils related connect rsp params
 * @fils_pmk: fils pmk
 * @fils_pmk_len: fils pmk length
 * @fils_pmkid: fils pmkid
 * @kek: kek
 * @kek_len: kek length
 * @tk: tk
 * @tk_len: tk length
 * @gtk: gtk
 * @gtk_len: gtk length
 * @dst_mac: dst mac
 * @src_mac: src mac
 * @hlp_data: hlp data
 * @hlp_data_len: hlp data length
 */
struct fils_connect_rsp_params {
	uint8_t *fils_pmk;
	uint8_t fils_pmk_len;
	uint8_t fils_pmkid[PMKID_LEN];
	uint8_t kek[MAX_KEK_LENGTH];
	uint8_t kek_len;
	uint8_t tk[MAX_TK_LENGTH];
	uint8_t tk_len;
	uint8_t gtk[MAX_GTK_LENGTH];
	uint8_t gtk_len;
	struct qdf_mac_addr dst_mac;
	struct qdf_mac_addr src_mac;
	uint8_t hlp_data[CM_FILS_MAX_HLP_DATA_LEN];
	uint16_t hlp_data_len;
};
#endif

/**
 * struct connect_rsp_ies - connect rsp ies stored in vdev filled during connect
 * @bcn_probe_rsp: beacon or probe rsp of connected AP
 * @assoc_req: assoc req send during conenct
 * @assoc_rsq: assoc rsp received during connection
 * @ric_resp_ie: ric ie from assoc resp received during connection
 * @fills_ie: fills connection ie received during connection
 */
struct connect_rsp_ies {
	struct element_info bcn_probe_rsp;
	struct element_info assoc_req;
	struct element_info assoc_rsp;
	struct element_info ric_resp_ie;
#ifdef WLAN_FEATURE_FILS_SK
	struct fils_connect_rsp_params fils_ie;
#endif
};

/**
 * struct disconnect_ies - disconnect ies stored in vdev filled during
 * disconnect
 * @peer_discon_ie: disconnect ie sent by peer to be sent to OSIF
 * @self_discon_ie: disconnect ie to be sent in disassoc/deauth filled by OSIF
 */
struct disconnect_ies {
	struct element_info peer_discon_ie;
	struct element_info self_discon_ie;
};

/**
 * struct cnx_mgr - connect manager req
 * @vdev: vdev back pointer
 * @sm: state machine
 * @req_list: connect/disconnect req list
 * @disconnect_count: disconnect count
 * @connect_count: connect count
 * @force_rsne_override: if QCA_WLAN_VENDOR_ATTR_CONFIG_RSN_IE is set by
 * framework
 * @req_ie: request ies for connect
 * @rsp_ie: connect resp ie
 * @discon_ie: disconnect IE
 * @fils_info: fils info for the connect req
 */
struct cnx_mgr {
	struct wlan_objmgr_vdev *vdev;
	struct cm_state_sm sm;
	qdf_list_t req_list;
	uint8_t disconnect_count;
	uint8_t connect_count;
	bool force_rsne_override;
	struct connect_req_ies req_ie;
	struct connect_rsp_ies rsp_ie;
	struct disconnect_ies discon_ie;
#ifdef WLAN_FEATURE_FILS_SK
	struct wlan_fils_con_info fils_info;
#endif
};

/**
 * mlme_cm_init() - Invoke connection manager init
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API allocates CM and init
 *
 * Return: SUCCESS on successful allocation
 *         FAILURE, if registration fails
 */
QDF_STATUS mlme_cm_init(struct vdev_mlme_obj *vdev_mlme);

/**
 * mlme_cm_deinit() - Invoke connection manager deinit
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API destroys CM
 *
 * Return: SUCCESS on successful deletion
 *         FAILURE, if deletion fails
 */
QDF_STATUS mlme_cm_deinit(struct vdev_mlme_obj *vdev_mlme);
#else

static inline QDF_STATUS mlme_cm_init(struct vdev_mlme_obj *vdev_mlme)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS mlme_cm_deinit(struct vdev_mlme_obj *vdev_mlme)
{
	return QDF_STATUS_SUCCESS;
}

#endif /* FEATURE_CM_ENABLE */

#endif /* __WLAN_CM_MAIN_H__ */
