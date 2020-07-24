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
 * DOC: Implements general SM framework for connection manager roaming sm
 */

#include "wlan_cm_main.h"
#include "wlan_cm_roam_sm.h"
#include "wlan_cm_sm.h"

void cm_state_roaming_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	cm_sm_state_update(cm_ctx, WLAN_CM_S_ROAMING, WLAN_CM_SS_IDLE);
}

void cm_state_roaming_exit(void *ctx)
{
}

bool cm_state_roaming_event(void *ctx, uint16_t event, uint16_t data_len,
			    void *data)
{
//	struct cnx_mgr *cm_ctx = ctx;
	bool status;

	switch (event) {
	default:
		status = false;
		break;
	}

	return status;
}

#ifdef WLAN_FEATURE_HOST_ROAM
#ifdef WLAN_FEATURE_PREAUTH_ENABLE
void cm_subst_preauth_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	if (cm_get_state(cm_ctx) != WLAN_CM_S_ROAMING)
		QDF_BUG(0);

	cm_set_substate(cm_ctx, WLAN_CM_SS_PREAUTH);
}

void cm_subst_preauth_exit(void *ctx)
{
}

bool cm_subst_preauth_event(void *ctx, uint16_t event, uint16_t data_len,
			    void *data)
{
//	struct cnx_mgr *cm_ctx = ctx;
	bool status;

	switch (event) {
	default:
		status = false;
		break;
	}

	return status;
}

#endif /* WLAN_FEATURE_PREAUTH_ENABLE */

void cm_subst_reassoc_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	if (cm_get_state(cm_ctx) != WLAN_CM_S_ROAMING)
		QDF_BUG(0);

	cm_set_substate(cm_ctx, WLAN_CM_SS_REASSOC);
}

void cm_subst_reassoc_exit(void *ctx)
{
}

bool cm_subst_reassoc_event(void *ctx, uint16_t event, uint16_t data_len,
			    void *data)
{
//	struct cnx_mgr *cm_ctx = ctx;
	bool status;

	switch (event) {
	default:
		status = false;
		break;
	}

	return status;
}

#endif /* WLAN_FEATURE_HOST_ROAM */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
void cm_subst_roam_start_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	if (cm_get_state(cm_ctx) != WLAN_CM_S_ROAMING)
		QDF_BUG(0);

	cm_set_substate(cm_ctx, WLAN_CM_SS_ROAM_STARTED);
}

void cm_subst_roam_start_exit(void *ctx)
{
}

bool cm_subst_roam_start_event(void *ctx, uint16_t event, uint16_t data_len,
			       void *data)
{
//	struct cnx_mgr *cm_ctx = ctx;
	bool status;

	switch (event) {
	default:
		status = false;
		break;
	}

	return status;
}

void cm_subst_roam_sync_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	if (cm_get_state(cm_ctx) != WLAN_CM_S_ROAMING)
		QDF_BUG(0);

	cm_set_substate(cm_ctx, WLAN_CM_SS_ROAM_SYNC);
}

void cm_subst_roam_sync_exit(void *ctx)
{
}

bool cm_subst_roam_sync_event(void *ctx, uint16_t event, uint16_t data_len,
			      void *data)
{
//	struct cnx_mgr *cm_ctx = ctx;
	bool status;

	switch (event) {
	default:
		status = false;
		break;
	}

	return status;
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
