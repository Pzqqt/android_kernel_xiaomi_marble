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
 * DOC: wlan_cm_sm.h
 *
 * This header file maintain structures required for connection mgr sm infra
 */

#ifndef __WLAN_CM_SM_H__
#define __WLAN_CM_SM_H__

#ifdef FEATURE_CM_ENABLE
#include <wlan_sm_engine.h>

/**
 * enum wlan_cm_sm_evt - connection manager related events
 * @WLAN_CM_SM_EV_CONNECT_REQ:            Connect request event from requester
 * @WLAN_CM_SM_EV_SCAN:                   Event to start connect scan
 * @WLAN_CM_SM_EV_SCAN_SUCCESS:           Connect scan success event
 * @WLAN_CM_SM_EV_SCAN_FAILURE:           Connect scan fail event
 * @WLAN_CM_SM_EV_HW_MODE_SUCCESS:        Hw mode change is success
 * @WLAN_CM_SM_EV_HW_MODE_FAILURE:        Hw mode change is failure
 * @WLAN_CM_SM_EV_CONNECT_START:          Connect start process initiate
 * @WLAN_CM_SM_EV_CONNECT_ACTIVE:         Connect request is activated
 * @WLAN_CM_SM_EV_CONNECT_SUCCESS:        Connect success
 * @WLAN_CM_SM_EV_BSS_SELECT_IND_SUCCESS: Mlme resp for BSS select indication
 * @WLAN_CM_SM_EV_BSS_CREATE_PEER_SUCCESS: BSS peer create success
 * @WLAN_CM_SM_EV_CONNECT_GET_NEXT_CANDIDATE: Get next candidate for connection
 * @WLAN_CM_SM_EV_CONNECT_FAILURE:        Connect failed for all candidate
 * @WLAN_CM_SM_EV_DISCONNECT_REQ:         Disconnect request event from
 * requester
 * @WLAN_CM_SM_EV_DISCONNECT_START:       Start disconnect sequence
 * @WLAN_CM_SM_EV_DISCONNECT_ACTIVE:      Process disconnect after in active cmd
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
 * @WLAN_CM_SM_EV_MAX:                    Max event
 */
enum wlan_cm_sm_evt {
	WLAN_CM_SM_EV_CONNECT_REQ = 0,
	WLAN_CM_SM_EV_SCAN = 1,
	WLAN_CM_SM_EV_SCAN_SUCCESS = 2,
	WLAN_CM_SM_EV_SCAN_FAILURE = 3,
	WLAN_CM_SM_EV_HW_MODE_SUCCESS = 4,
	WLAN_CM_SM_EV_HW_MODE_FAILURE = 5,
	WLAN_CM_SM_EV_CONNECT_START = 6,
	WLAN_CM_SM_EV_CONNECT_ACTIVE = 7,
	WLAN_CM_SM_EV_CONNECT_SUCCESS = 8,
	WLAN_CM_SM_EV_BSS_SELECT_IND_SUCCESS = 9,
	WLAN_CM_SM_EV_BSS_CREATE_PEER_SUCCESS = 10,
	WLAN_CM_SM_EV_CONNECT_GET_NEXT_CANDIDATE = 11,
	WLAN_CM_SM_EV_CONNECT_FAILURE = 12,
	WLAN_CM_SM_EV_DISCONNECT_REQ = 13,
	WLAN_CM_SM_EV_DISCONNECT_START = 14,
	WLAN_CM_SM_EV_DISCONNECT_ACTIVE = 15,
	WLAN_CM_SM_EV_DISCONNECT_DONE = 16,
	WLAN_CM_SM_EV_ROAM_START = 17,
	WLAN_CM_SM_EV_ROAM_SYNC = 18,
	WLAN_CM_SM_EV_ROAM_INVOKE_FAIL = 19,
	WLAN_CM_SM_EV_ROAM_HO_FAIL = 20,
	WLAN_CM_SM_EV_PREAUTH_DONE = 21,
	WLAN_CM_SM_EV_GET_NEXT_PREAUTH_AP = 22,
	WLAN_CM_SM_EV_PREAUTH_FAIL = 23,
	WLAN_CM_SM_EV_START_REASSOC = 24,
	WLAN_CM_SM_EV_REASSOC_DONE = 25,
	WLAN_CM_SM_EV_REASSOC_FAILURE = 26,
	WLAN_CM_SM_EV_ROAM_COMPLETE = 27,
	WLAN_CM_SM_EV_MAX,
};

/**
 * cm_sm_create() - Invoke SM creation for connection manager
 * @cm_ctx:  connection manager ctx
 *
 * API allocates CM MLME SM and initializes SM lock
 *
 * Return: SUCCESS on successful allocation
 *         FAILURE, if registration fails
 */
QDF_STATUS cm_sm_create(struct cnx_mgr *cm_ctx);

/**
 * cm_sm_destroy() - Invoke SM deletion for connection manager
 * @cm_ctx:  connection manager ctx
 *
 * API destroys CM MLME SM and SM lock
 *
 * Return: SUCCESS on successful deletion
 *         FAILURE, if deletion fails
 */
QDF_STATUS cm_sm_destroy(struct cnx_mgr *cm_ctx);

/**
 * cm_sm_history_print() - Prints SM history
 * @cm_ctx:  connection manager ctx
 *
 * API to print CM SM history
 *
 * Return: void
 */
#ifdef SM_ENG_HIST_ENABLE
void cm_sm_history_print(struct wlan_objmgr_vdev *vdev);
#endif

#ifdef WLAN_CM_USE_SPINLOCK
/**
 * cm_lock_create - Create CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * Creates CM SM mutex/spinlock
 *
 * Return: void
 */
static inline void
cm_lock_create(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_create(&cm_ctx->sm.cm_sm_lock);
}

/**
 * cm_lock_destroy - Destroy CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * Destroy CM SM mutex/spinlock
 *
 * Return: void
 */
static inline void
cm_lock_destroy(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_destroy(&cm_ctx->sm.cm_sm_lock);
}

/**
 * cm_lock_acquire - acquire CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * acquire CM SM mutex/spinlock
 *
 * return: void
 */
static inline void cm_lock_acquire(struct cnx_mgr *cm_ctx)
{
	qdf_spin_lock_bh(&cm_ctx->sm.cm_sm_lock);
}

/**
 * cm_lock_release - release CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * release CM SM mutex/spinlock
 *
 * return: void
 */
static inline void cm_lock_release(struct cnx_mgr *cm_ctx)
{
	qdf_spin_unlock_bh(&cm_ctx->sm.cm_sm_lock);
}
#else
static inline void
cm_lock_create(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_create(&cm_ctx->sm.cm_sm_lock);
}

static inline void
cm_lock_destroy(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_destroy(&cm_ctx->sm.cm_sm_lock);
}

static inline void cm_lock_acquire(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_acquire(&cm_ctx->sm.cm_sm_lock);
}

static inline void cm_lock_release(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_release(&cm_ctx->sm.cm_sm_lock);
}
#endif /* WLAN_CM_USE_SPINLOCK */

/**
 * cm_sm_transition_to() - invokes state transition
 * @cm_ctx:  connection manager ctx
 * @state: new cm state
 *
 * API to invoke SM API to move to new state
 *
 * Return: void
 */
static inline void cm_sm_transition_to(struct cnx_mgr *cm_ctx,
				       enum wlan_cm_sm_state state)
{
	wlan_sm_transition_to(cm_ctx->sm.sm_hdl, state);
}

/**
 * cm_get_state() - get mlme state
 * @cm_ctx: connection manager SM ctx
 *
 * API to get cm state
 *
 * Return: state of cm
 */
enum wlan_cm_sm_state cm_get_state(struct cnx_mgr *cm_ctx);

/**
 * cm_get_sub_state() - get mlme substate
 * @cm_ctx: connection manager SM ctx
 *
 * API to get cm substate
 *
 * Return: substate of cm
 */
enum wlan_cm_sm_state cm_get_sub_state(struct cnx_mgr *cm_ctx);

/**
 * cm_set_state() - set cm mlme state
 * @cm_ctx: connection manager SM ctx
 * @state: cm state
 *
 * API to set cm state
 *
 * Return: void
 */
void cm_set_state(struct cnx_mgr *cm_ctx, enum wlan_cm_sm_state state);
/**
 * cm_set_substate() - set cm mlme sub state
 * @cm_ctx: connection manager SM ctx
 * @substate: cm sub state
 *
 * API to set cm sub state
 *
 * Return: void
 */
void cm_set_substate(struct cnx_mgr *cm_ctx,
		     enum wlan_cm_sm_state substate);

/**
 * cm_sm_state_update() - set cm mlme state and sub state
 * @cm_ctx: connection manager SM ctx
 * @state: cm state
 * @substate: cm sub state
 *
 * API to invoke util APIs to set state and MLME sub state
 *
 * Return: void
 */
void cm_sm_state_update(struct cnx_mgr *cm_ctx,
			enum wlan_cm_sm_state state,
			enum wlan_cm_sm_state substate);

/**
 * cm_sm_deliver_event_sync() - Delivers event to connection manager SM while
 * holding lock
 * @cm_ctx: cm ctx
 * @event: CM event
 * @data_len: data size
 * @data: event data
 *
 * API to dispatch event to VDEV MLME SM without lock, in case lock is already
 * held.
 *
 * Context: Can be called from any context, This should be called in case
 * SM lock is already taken. If lock is not taken use cm_sm_deliver_event API
 * instead.
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: If event not handled
 */
static inline
QDF_STATUS cm_sm_deliver_event_sync(struct cnx_mgr *cm_ctx,
				    enum wlan_cm_sm_evt event,
				    uint16_t data_len, void *data)
{
	return wlan_sm_dispatch(cm_ctx->sm.sm_hdl, event, data_len, data);
}

/**
 * cm_sm_deliver_event() - Delivers event to connection manager SM
 * @vdev: Object manager VDEV object
 * @event: CM event
 * @data_len: data size
 * @data: event data
 *
 * API to dispatch event to VDEV MLME SM with lock. To be used while paosting
 * events from API called from publick API. i.e. indication/response/request
 * from any other moudle or NB/SB.
 *
 * Context: Can be called from any context, This should be called in case
 * SM lock is not taken, the API will take the lock before posting to SM.
 * If lock is already taken use cm_sm_deliver_event_sync API instead.
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: If event not handled
 */
QDF_STATUS  cm_sm_deliver_event(struct wlan_objmgr_vdev *vdev,
				enum wlan_cm_sm_evt event,
				uint16_t data_len, void *data);

#endif /* FEATURE_CM_ENABLE */
#endif /* __WLAN_CM_SM_H__ */
