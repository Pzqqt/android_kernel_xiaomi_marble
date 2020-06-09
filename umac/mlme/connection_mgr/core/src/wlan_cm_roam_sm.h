/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

/**
 * mlme_cm_state_roaming_entry() - Entry API for roaming state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to roaming state
 *
 * Return: void
 */
void mlme_cm_state_roaming_entry(void *ctx);

/**
 * mlme_cm_state_roaming_exit() - Exit API for roaming state for connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from roaming state
 *
 * Return: void
 */
void mlme_cm_state_roaming_exit(void *ctx);

/**
 * mlme_cm_state_roaming_event() - Roaming State event handler for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to handle events in roaming state
 *
 * Return: bool
 */
bool mlme_cm_state_roaming_event(void *ctx, uint16_t event,
				 uint16_t event_data_len,
				 void *event_data);

#ifdef WLAN_FEATURE_HOST_ROAM
#ifdef WLAN_FEATURE_PREAUTH_ENABLE
/**
 * mlme_cm_subst_preauth_entry() - Entry API for preauth sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to preauth sub-state
 *
 * Return: void
 */
void mlme_cm_subst_preauth_entry(void *ctx);

/**
 * mlme_cm_subst_preauth_exit() - Exit API for preauth sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from preauth sub-state
 *
 * Return: void
 */
void mlme_cm_subst_preauth_exit(void *ctx);

/**
 * mlme_cm_subst_preauth_event() - Preauth sub-state event handler for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to handle events in preauth sub-state
 *
 * Return: bool
 */
bool mlme_cm_subst_preauth_event(void *ctx, uint16_t event,
				 uint16_t event_data_len,
				 void *event_data);
#else /* WLAN_FEATURE_PREAUTH_ENABLE  && WLAN_FEATURE_HOST_ROAM */

static inline void mlme_cm_subst_preauth_entry(void *ctx) {}

static inline void mlme_cm_subst_preauth_exit(void *ctx) {}

static inline bool mlme_cm_subst_preauth_event(void *ctx, uint16_t event,
					       uint16_t event_data_len,
					       void *event_data)
{
	return true;
}
#endif /* WLAN_FEATURE_PREAUTH_ENABLE */

/**
 * mlme_cm_subst_reassoc_entry() - Entry API for reassoc sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to reassoc sub-state
 *
 * Return: void
 */
void mlme_cm_subst_reassoc_entry(void *ctx);

/**
 * mlme_cm_subst_reassoc_exit() - Exit API for reassoc sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from reassoc sub-state
 *
 * Return: void
 */
void mlme_cm_subst_reassoc_exit(void *ctx);

/**
 * mlme_cm_subst_reassoc_event() - Reassoc sub-state event handler for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to handle events in preauth sub-state
 *
 * Return: bool
 */
bool mlme_cm_subst_reassoc_event(void *ctx, uint16_t event,
				 uint16_t event_data_len,
				 void *event_data);
#endif /* WLAN_FEATURE_HOST_ROAM */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD

/**
 * mlme_cm_subst_roam_start_entry() - Entry API for roam start sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to roam start sub-state
 *
 * Return: void
 */
void mlme_cm_subst_roam_start_entry(void *ctx);

/**
 * mlme_cm_subst_roam_start_exit() - Exit API for roam start sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from roam start sub-state
 *
 * Return: void
 */
void mlme_cm_subst_roam_start_exit(void *ctx);

/**
 * mlme_cm_subst_roam_start_event() - Roam start sub-state event handler for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to handle events in roam start sub-state
 *
 * Return: bool
 */
bool mlme_cm_subst_roam_start_event(void *ctx, uint16_t event,
				    uint16_t event_data_len,
				    void *event_data);

/**
 * mlme_cm_subst_roam_sync_entry() - Entry API for roam sync sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to roam sync sub-state
 *
 * Return: void
 */
void mlme_cm_subst_roam_sync_entry(void *ctx);

/**
 * mlme_cm_subst_roam_sync_exit() - Exit API for roam sync sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from roam sync sub-state
 *
 * Return: void
 */
void mlme_cm_subst_roam_sync_exit(void *ctx);

/**
 * mlme_cm_subst_roam_sync_event() - Roam sync sub-state event handler for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to handle events in roam sync sub-state
 *
 * Return: bool
 */
bool mlme_cm_subst_roam_sync_event(void *ctx, uint16_t event,
				   uint16_t event_data_len,
				   void *event_data);
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

#endif /* __WLAN_CM_ROAM_SM_H__ */
