/*
 * Copyright (c) 2012-2015,2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "wlan_cm_main_api.h"
#include "wlan_cm_roam.h"
#ifdef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
#include "wlan_mlo_mgr_roam.h"
#endif

void cm_state_roaming_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	cm_sm_state_update(cm_ctx, WLAN_CM_S_ROAMING, WLAN_CM_SS_IDLE);
}

void cm_state_roaming_exit(void *ctx)
{
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static
bool cm_handle_fw_roaming_event(struct cnx_mgr *cm_ctx, uint16_t event,
				uint16_t data_len, void *data)
{
	bool event_handled = true;
	QDF_STATUS status;

	switch (event) {
	case WLAN_CM_SM_EV_ROAM_INVOKE:
		status = cm_add_fw_roam_cmd_to_list_n_ser(cm_ctx, data);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_SS_ROAM_STARTED);
		cm_sm_deliver_event_sync(cm_ctx,
					 WLAN_CM_SM_EV_ROAM_INVOKE,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_ROAM_START:
		status = cm_add_fw_roam_cmd_to_list_n_ser(cm_ctx, data);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_SS_ROAM_STARTED);
		cm_sm_deliver_event_sync(cm_ctx,
					 WLAN_CM_SM_EV_ROAM_START,
					 0, NULL);
		break;
	case WLAN_CM_SM_EV_ROAM_ABORT:
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_CONNECTED);
		cm_sm_deliver_event_sync(cm_ctx, event,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_ROAM_SYNC:
		cm_sm_transition_to(cm_ctx, WLAN_CM_SS_ROAM_SYNC);
		cm_sm_deliver_event_sync(cm_ctx, event,
					 data_len, data);
		break;
	default:
		event_handled = false;
		break;
	}

	return event_handled;
}
#else
static inline
bool cm_handle_fw_roaming_event(struct cnx_mgr *cm_ctx, uint16_t event,
				uint16_t data_len, void *data)
{
	return false;
}
#endif

bool cm_state_roaming_event(void *ctx, uint16_t event,
			    uint16_t data_len, void *data)
{
	struct cnx_mgr *cm_ctx = ctx;
	bool event_handled = true;
	struct wlan_objmgr_psoc *psoc;

	switch (event) {
	case WLAN_CM_SM_EV_ROAM_REQ:
		psoc = wlan_vdev_get_psoc(cm_ctx->vdev);
		if (!psoc) {
			event_handled = false;
			break;
		}
		if (cm_roam_offload_enabled(psoc)) {
			cm_sm_deliver_event_sync(cm_ctx,
						 WLAN_CM_SM_EV_ROAM_INVOKE,
						 data_len, data);
		} else {
			cm_add_roam_req_to_list(cm_ctx, data);
			cm_sm_transition_to(cm_ctx, WLAN_CM_SS_PREAUTH);
			cm_sm_deliver_event_sync(cm_ctx,
						 WLAN_CM_SM_EV_ROAM_START,
						 data_len, data);
		}
		break;
	default:
		event_handled = cm_handle_fw_roaming_event(cm_ctx, event,
							   data_len, data);
		break;
	}

	return event_handled;
}

static bool cm_handle_connect_disconnect_in_roam(struct cnx_mgr *cm_ctx,
						 uint16_t event,
						 uint16_t data_len, void *data)
{
	QDF_STATUS status;

	switch (event) {
	case WLAN_CM_SM_EV_CONNECT_REQ:
		status = cm_handle_connect_req_in_non_init_state(cm_ctx, data,
							WLAN_CM_S_ROAMING);
		if (QDF_IS_STATUS_ERROR(status))
			return false;
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_CONNECTING);
		cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_CONNECT_START,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_REQ:
		status = cm_handle_discon_req_in_non_connected_state(cm_ctx,
						data, WLAN_CM_S_ROAMING);
		if (QDF_IS_STATUS_ERROR(status))
			return false;
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_DISCONNECTING);
		cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_DISCONNECT_START,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_ACTIVE:
		cm_disconnect_active(cm_ctx, data);
		break;
	default:
		return false;
		break;
	}

	return true;
}

#ifdef WLAN_FEATURE_HOST_ROAM
void cm_subst_preauth_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	if (cm_get_state(cm_ctx) != WLAN_CM_S_ROAMING)
		QDF_BUG(0);

	cm_set_substate(cm_ctx, WLAN_CM_SS_PREAUTH);
	/* set preauth to true when we enter preauth state */
	cm_ctx->preauth_in_progress = true;
}

void cm_subst_preauth_exit(void *ctx)
{
}

#ifdef WLAN_FEATURE_PREAUTH_ENABLE
static bool
cm_handle_preauth_event(struct cnx_mgr *cm_ctx, uint16_t event,
			uint16_t data_len, void *data)
{
	bool event_handled = true;

	switch (event) {
	case WLAN_CM_SM_EV_PREAUTH_ACTIVE:
		if (!cm_check_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_preauth_active(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_PREAUTH_RESP:
		cm_preauth_done_resp(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_PREAUTH_DONE:
		cm_sm_transition_to(cm_ctx, WLAN_CM_SS_REASSOC);
		cm_preauth_success(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_PREAUTH_FAIL:
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_CONNECTED);
		cm_preauth_fail(cm_ctx, data);
		break;
	default:
		event_handled = false;
	}

	return event_handled;
}
#else
static inline bool
cm_handle_preauth_event(struct cnx_mgr *cm_ctx, uint16_t event,
			uint16_t data_len, void *data)
{
	return false;
}
#endif

bool cm_subst_preauth_event(void *ctx, uint16_t event,
			    uint16_t data_len, void *data)
{
	struct cnx_mgr *cm_ctx = ctx;
	bool event_handled = true;

	switch (event) {
	case WLAN_CM_SM_EV_CONNECT_REQ:
	case WLAN_CM_SM_EV_DISCONNECT_REQ:
	case WLAN_CM_SM_EV_DISCONNECT_ACTIVE:
		event_handled =
			cm_handle_connect_disconnect_in_roam(cm_ctx, event,
							     data_len, data);
		break;
	case WLAN_CM_SM_EV_ROAM_START:
		cm_host_roam_start_req(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_START_REASSOC:
		cm_sm_transition_to(cm_ctx, WLAN_CM_SS_REASSOC);
		cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
		break;
	case WLAN_CM_SM_EV_REASSOC_FAILURE:
		cm_reassoc_complete(cm_ctx, data);
		break;
	default:
		event_handled = cm_handle_preauth_event(cm_ctx, event,
							data_len, data);
		break;
	}

	return event_handled;
}

void cm_subst_reassoc_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	if (cm_get_state(cm_ctx) != WLAN_CM_S_ROAMING)
		QDF_BUG(0);

	cm_set_substate(cm_ctx, WLAN_CM_SS_REASSOC);
	/* set preauth to false as soon as we move to reassoc state */
	cm_ctx->preauth_in_progress = false;
}

void cm_subst_reassoc_exit(void *ctx)
{
}

#ifdef WLAN_FEATURE_PREAUTH_ENABLE
static bool
cm_handle_reassoc_event(struct cnx_mgr *cm_ctx, uint16_t event,
			uint16_t data_len, void *data)
{
	bool event_handled = true;
	QDF_STATUS status;

	switch (event) {
	case WLAN_CM_SM_EV_REASSOC_TIMER:
		status = cm_handle_reassoc_timer(cm_ctx, data);
		if (QDF_IS_STATUS_ERROR(status))
			event_handled = false;
		break;
	default:
		event_handled = false;
	}

	return event_handled;
}
#else
static inline bool
cm_handle_reassoc_event(struct cnx_mgr *cm_ctx, uint16_t event,
			uint16_t data_len, void *data)
{
	return false;
}
#endif

bool cm_subst_reassoc_event(void *ctx, uint16_t event,
			    uint16_t data_len, void *data)
{
	struct cnx_mgr *cm_ctx = ctx;
	bool event_handled = true;

	switch (event) {
	case WLAN_CM_SM_EV_CONNECT_REQ:
	case WLAN_CM_SM_EV_DISCONNECT_REQ:
	case WLAN_CM_SM_EV_DISCONNECT_ACTIVE:
		event_handled =
			cm_handle_connect_disconnect_in_roam(cm_ctx, event,
							     data_len, data);
		break;
	case WLAN_CM_SM_EV_START_REASSOC:
		cm_reassoc_start(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_REASSOC_ACTIVE:
		if (!cm_check_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_reassoc_active(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_HO_ROAM_DISCONNECT_DONE:
		cm_reassoc_disconnect_complete(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_BSS_CREATE_PEER_SUCCESS:
		if (!cm_check_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_resume_reassoc_after_peer_create(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_REASSOC_DONE:
		if (!cm_roam_resp_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_CONNECTED);
		cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
		break;
	case WLAN_CM_SM_EV_REASSOC_FAILURE:
		cm_reassoc_complete(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_HW_MODE_SUCCESS:
	case WLAN_CM_SM_EV_HW_MODE_FAILURE:
		/* check if cm id is valid for the current req */
		if (!cm_check_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_handle_reassoc_hw_mode_change(cm_ctx, data, event);
		break;
	default:
		event_handled = cm_handle_reassoc_event(cm_ctx, event,
							data_len, data);
		break;
	}

	return event_handled;
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

bool cm_subst_roam_start_event(void *ctx, uint16_t event,
			       uint16_t data_len, void *data)
{
	bool event_handled = true;
	struct cnx_mgr *cm_ctx = ctx;

	switch (event) {
	case WLAN_CM_SM_EV_CONNECT_REQ:
	case WLAN_CM_SM_EV_DISCONNECT_REQ:
	case WLAN_CM_SM_EV_DISCONNECT_ACTIVE:
		event_handled =
			cm_handle_connect_disconnect_in_roam(cm_ctx, event,
							     data_len, data);
		break;
	case WLAN_CM_SM_EV_ROAM_START:
		cm_fw_roam_start(ctx);
		break;
	case WLAN_CM_SM_EV_ROAM_INVOKE:
		cm_send_roam_invoke_req(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_ROAM_ABORT:
	case WLAN_CM_SM_EV_ROAM_INVOKE_FAIL:
	case WLAN_CM_SM_EV_ROAM_HO_FAIL:
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_CONNECTED);
		cm_sm_deliver_event_sync(cm_ctx, event,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_ROAM_SYNC:
		cm_sm_transition_to(cm_ctx, WLAN_CM_SS_ROAM_SYNC);
		cm_sm_deliver_event_sync(cm_ctx, event,
					 data_len, data);
		break;
	default:
		event_handled = false;
		break;
	}

	return event_handled;
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

bool cm_subst_roam_sync_event(void *ctx, uint16_t event,
			      uint16_t data_len, void *data)
{
	bool event_handled = true;
	struct cnx_mgr *cm_ctx = ctx;

	switch (event) {
	case WLAN_CM_SM_EV_CONNECT_REQ:
	case WLAN_CM_SM_EV_DISCONNECT_REQ:
	case WLAN_CM_SM_EV_DISCONNECT_ACTIVE:
		event_handled =
			cm_handle_connect_disconnect_in_roam(cm_ctx, event,
							     data_len, data);
		break;
	case WLAN_CM_SM_EV_ROAM_SYNC:
#ifdef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
		mlo_cm_roam_sync_cb(cm_ctx->vdev, data, data_len);
#endif
		cm_fw_send_vdev_roam_event(cm_ctx, data_len, data);
		break;
	case WLAN_CM_SM_EV_ROAM_DONE:
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_CONNECTED);
		cm_sm_deliver_event_sync(cm_ctx, event,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_ROAM_ABORT:
	case WLAN_CM_SM_EV_ROAM_HO_FAIL:
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_CONNECTED);
		cm_sm_deliver_event_sync(cm_ctx, event,
					 data_len, data);
		break;
	default:
		event_handled = false;
		break;
	}

	return event_handled;
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
