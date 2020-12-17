/*
 * Copyright (c) 2012-2015,2020-2021 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cm_roam_sm.h
 *
 * This header file maintain structures required for connection mgr roam sm
 */

#ifndef __WLAN_CM_ROAM_SM_H__
#define __WLAN_CM_ROAM_SM_H__

#include "wlan_cm_roam.h"

/**
 * cm_state_roaming_entry() - Entry API for roaming state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to roaming state
 *
 * Return: void
 */
void cm_state_roaming_entry(void *ctx);

/**
 * cm_state_roaming_exit() - Exit API for roaming state for connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from roaming state
 *
 * Return: void
 */
void cm_state_roaming_exit(void *ctx);

/**
 * cm_state_roaming_event() - Roaming State event handler for
 * connection mgr
 * @ctx: connection manager ctx
 * @event: event to handle
 * @data_len: event data len
 * @data:event data
 *
 * API to handle events in roaming state
 *
 * Return: bool
 */
bool cm_state_roaming_event(void *ctx, uint16_t event, uint16_t data_len,
			    void *data);

#if defined(WLAN_FEATURE_HOST_ROAM)
/**
 * cm_subst_preauth_entry() - Entry API for preauth sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to preauth sub-state
 *
 * Return: void
 */
void cm_subst_preauth_entry(void *ctx);

/**
 * cm_subst_preauth_exit() - Exit API for preauth sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from preauth sub-state
 *
 * Return: void
 */
void cm_subst_preauth_exit(void *ctx);

/**
 * cm_subst_preauth_event() - Preauth sub-state event handler for
 * connection mgr
 * @ctx: connection manager ctx
 * @event: event to handle
 * @data_len: event data len
 * @data:event data
 *
 * API to handle events in preauth sub-state
 *
 * Return: bool
 */
bool cm_subst_preauth_event(void *ctx, uint16_t event,
			    uint16_t data_len, void *data);
#else

static inline void cm_subst_preauth_entry(void *ctx) {}

static inline void cm_subst_preauth_exit(void *ctx) {}

static inline bool cm_subst_preauth_event(void *ctx, uint16_t event,
					  uint16_t data_len, void *data)
{
	return true;
}
#endif /* WLAN_FEATURE_HOST_ROAM && WLAN_FEATURE_PREAUTH_ENABLE */

#ifdef WLAN_FEATURE_HOST_ROAM
/**
 * cm_subst_reassoc_entry() - Entry API for reassoc sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to reassoc sub-state
 *
 * Return: void
 */
void cm_subst_reassoc_entry(void *ctx);

/**
 * cm_subst_reassoc_exit() - Exit API for reassoc sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from reassoc sub-state
 *
 * Return: void
 */
void cm_subst_reassoc_exit(void *ctx);

/**
 * cm_subst_reassoc_event() - Reassoc sub-state event handler for
 * connection mgr
 * @ctx: connection manager ctx
 * @event: event to handle
 * @data_len: event data len
 * @data:event data
 *
 * API to handle events in preauth sub-state
 *
 * Return: bool
 */
bool cm_subst_reassoc_event(void *ctx, uint16_t event, uint16_t data_len,
			    void *data);
#else

static inline void cm_subst_reassoc_entry(void *ctx) {}
static inline void cm_subst_reassoc_exit(void *ctx) {}

static inline
bool cm_subst_reassoc_event(void *ctx, uint16_t event, uint16_t data_len,
			    void *data)
{
	return true;
}

#endif /* WLAN_FEATURE_HOST_ROAM */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD

/**
 * cm_subst_roam_start_entry() - Entry API for roam start sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to roam start sub-state
 *
 * Return: void
 */
void cm_subst_roam_start_entry(void *ctx);

/**
 * cm_subst_roam_start_exit() - Exit API for roam start sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from roam start sub-state
 *
 * Return: void
 */
void cm_subst_roam_start_exit(void *ctx);

/**
 * cm_subst_roam_start_event() - Roam start sub-state event handler for
 * connection mgr
 * @ctx: connection manager ctx
 * @event: event to handle
 * @data_len: event data len
 * @data:event data
 *
 * API to handle events in roam start sub-state
 *
 * Return: bool
 */
bool cm_subst_roam_start_event(void *ctx, uint16_t event, uint16_t data_len,
			       void *data);

/**
 * cm_subst_roam_sync_entry() - Entry API for roam sync sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to roam sync sub-state
 *
 * Return: void
 */
void cm_subst_roam_sync_entry(void *ctx);

/**
 * cm_subst_roam_sync_exit() - Exit API for roam sync sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from roam sync sub-state
 *
 * Return: void
 */
void cm_subst_roam_sync_exit(void *ctx);

/**
 * cm_subst_roam_sync_event() - Roam sync sub-state event handler for
 * connection mgr
 * @ctx: connection manager ctx
 * @event: event to handle
 * @data_len: event data len
 * @data:event data
 *
 * API to handle events in roam sync sub-state
 *
 * Return: bool
 */
bool cm_subst_roam_sync_event(void *ctx, uint16_t event, uint16_t data_len,
			      void *data);
#else

static inline void cm_subst_roam_start_entry(void *ctx) {}

static inline void cm_subst_roam_start_exit(void *ctx) {}

static inline
bool cm_subst_roam_start_event(void *ctx, uint16_t event, uint16_t data_len,
			       void *data)
{
	return true;
}

static inline void cm_subst_roam_sync_entry(void *ctx) {}

static inline void cm_subst_roam_sync_exit(void *ctx) {}

static inline
bool cm_subst_roam_sync_event(void *ctx, uint16_t event, uint16_t data_len,
			      void *data)
{
	return true;
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
#endif /* __WLAN_CM_ROAM_SM_H__ */
