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
 * mlme_cm_sm_create() - Invoke SM creation for connection manager
 * @cm_ctx:  connection manager ctx
 *
 * API allocates CM MLME SM and initializes SM lock
 *
 * Return: SUCCESS on successful allocation
 *         FAILURE, if registration fails
 */
QDF_STATUS mlme_cm_sm_create(struct cnx_mgr *cm_ctx);

/**
 * mlme_cm_sm_destroy() - Invoke SM deletion for connection manager
 * @cm_ctx:  connection manager ctx
 *
 * API destroys CM MLME SM and SM lock
 *
 * Return: SUCCESS on successful deletion
 *         FAILURE, if deletion fails
 */
QDF_STATUS mlme_cm_sm_destroy(struct cnx_mgr *cm_ctx);

/**
 * mlme_cm_sm_history_print() - Prints SM history
 * @cm_ctx:  connection manager ctx
 *
 * API to print CM SM history
 *
 * Return: void
 */
#ifdef SM_ENG_HIST_ENABLE
static inline void mlme_cm_sm_history_print(struct cnx_mgr *cm_ctx)
{
	return wlan_sm_print_history(cm_ctx->sm.sm_hdl);
}
#else
static inline void mlme_cm_sm_history_print(struct cnx_mgr *cm_ctx)
{
}
#endif

#ifdef WLAN_CM_USE_SPINLOCK
/**
 * mlme_cm_lock_create - Create CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * Creates CM SM mutex/spinlock
 *
 * Return: void
 */
static inline void
mlme_cm_lock_create(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_create(&cm_ctx->sm.cm_sm_lock);
}

/**
 * mlme_cm_lock_destroy - Destroy CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * Destroy CM SM mutex/spinlock
 *
 * Return: void
 */
static inline void
mlme_cm_lock_destroy(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_destroy(&cm_ctx->sm.cm_sm_lock);
}

/**
 * mlme_cm_lock_acquire - acquire CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * acquire CM SM mutex/spinlock
 *
 * return: void
 */
static inline void mlme_cm_lock_acquire(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_acquire(&cm_ctx->sm.cm_sm_lock);
}

/**
 * mlme_cm_lock_release - release CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * release CM SM mutex/spinlock
 *
 * return: void
 */
static inline void mlme_cm_lock_release(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_release(&cm_ctx->sm.cm_sm_lock);
}
#else
static inline void
mlme_cm_lock_create(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_create(&cm_ctx->sm.cm_sm_lock);
}

static inline void
mlme_cm_lock_destroy(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_destroy(&cm_ctx->sm.cm_sm_lock);
}

static inline void mlme_cm_lock_acquire(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_acquire(&cm_ctx->sm.cm_sm_lock);
}

static inline void mlme_cm_lock_release(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_release(&cm_ctx->sm.cm_sm_lock);
}
#endif /* WLAN_CM_USE_SPINLOCK */

/**
 * mlme_cm_sm_transition_to() - invokes state transition
 * @cm_ctx:  connection manager ctx
 * @state: new cm state
 *
 * API to invoke SM API to move to new state
 *
 * Return: void
 */
static inline void mlme_cm_sm_transition_to(struct cnx_mgr *cm_ctx,
					    enum wlan_cm_sm_state state)
{
	wlan_sm_transition_to(cm_ctx->sm.sm_hdl, state);
}

/**
 * mlme_cm_get_state() - get mlme state
 * @cm_ctx: connection manager SM ctx
 *
 * API to get cm state
 *
 * Return: state of cm
 */
enum wlan_cm_sm_state mlme_cm_get_state(struct cnx_mgr *cm_ctx);

/**
 * mlme_cm_get_sub_state() - get mlme substate
 * @cm_ctx: connection manager SM ctx
 *
 * API to get cm substate
 *
 * Return: substate of cm
 */
enum wlan_cm_sm_state mlme_cm_get_sub_state(struct cnx_mgr *cm_ctx);

/**
 * mlme_cm_set_state() - set cm mlme state
 * @cm_ctx: connection manager SM ctx
 * @state: cm state
 *
 * API to set cm state
 *
 * Return: void
 */
void mlme_cm_set_state(struct cnx_mgr *cm_ctx, enum wlan_cm_sm_state state);
/**
 * mlme_cm_set_substate() - set cm mlme sub state
 * @cm_ctx: connection manager SM ctx
 * @substate: cm sub state
 *
 * API to set cm sub state
 *
 * Return: void
 */
void mlme_cm_set_substate(struct cnx_mgr *cm_ctx,
			  enum wlan_cm_sm_state substate);

/**
 * mlme_cm_sm_state_update() - set cm mlme state and sub state
 * @cm_ctx: connection manager SM ctx
 * @state: cm state
 * @substate: cm sub state
 *
 * API to invoke util APIs to set state and MLME sub state
 *
 * Return: void
 */
void mlme_cm_sm_state_update(struct cnx_mgr *cm_ctx,
			     enum wlan_cm_sm_state state,
			     enum wlan_cm_sm_state substate);

/**
 * mlme_cm_sm_deliver_evt() - Delivers event to CM SM
 * @vdev: Object manager VDEV object
 * @event: CM event
 * @event_data_len: data size
 * @event_data: event data
 *
 * API to dispatch event to VDEV MLME SM with lock acquired
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
QDF_STATUS mlme_cm_sm_deliver_evt(struct wlan_objmgr_vdev *vdev,
				  enum wlan_cm_sm_evt event,
				  uint16_t event_data_len,
				  void *event_data);

#endif /* FEATURE_CM_ENABLE */
#endif /* __WLAN_CM_SM_H__ */
