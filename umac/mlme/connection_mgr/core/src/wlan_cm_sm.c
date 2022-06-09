/*
 * Copyright (c) 2012-2015,2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: Implements general SM framework for connection manager
 */

#include "wlan_cm_main_api.h"
#include "wlan_cm_sm.h"
#include "wlan_cm_roam_sm.h"

void cm_set_state(struct cnx_mgr *cm_ctx, enum wlan_cm_sm_state state)
{
	if (state < WLAN_CM_S_MAX)
		cm_ctx->sm.cm_state = state;
	else
		mlme_err("vdev %d mlme state (%d) is invalid",
			 wlan_vdev_get_id(cm_ctx->vdev), state);
}

void cm_set_substate(struct cnx_mgr *cm_ctx, enum wlan_cm_sm_state substate)
{
	if ((substate > WLAN_CM_S_MAX) && (substate < WLAN_CM_SS_MAX))
		cm_ctx->sm.cm_substate = substate;
	else
		mlme_err("vdev %d mlme sub state (%d) is invalid",
			 wlan_vdev_get_id(cm_ctx->vdev), substate);
}

void cm_sm_state_update(struct cnx_mgr *cm_ctx,
			enum wlan_cm_sm_state state,
			enum wlan_cm_sm_state substate)
{
	if (!cm_ctx)
		return;

	cm_set_state(cm_ctx, state);
	cm_set_substate(cm_ctx, substate);
}

/**
 * cm_state_init_entry() - Entry API for init state for connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to init state
 *
 * Return: void
 */
static void cm_state_init_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	cm_sm_state_update(cm_ctx, WLAN_CM_S_INIT, WLAN_CM_SS_IDLE);
}

/**
 * cm_state_init_exit() - Exit API for init state for connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from init state
 *
 * Return: void
 */
static void cm_state_init_exit(void *ctx)
{
}

/**
 * cm_state_init_event() - Init State event handler for connection mgr
 * @ctx: connection manager ctx
 *
 * API to handle events in INIT state
 *
 * Return: bool
 */
static bool cm_state_init_event(void *ctx, uint16_t event,
				uint16_t data_len, void *data)
{
	struct cnx_mgr *cm_ctx = ctx;
	bool event_handled = true;
	QDF_STATUS status;

	switch (event) {
	case WLAN_CM_SM_EV_CONNECT_REQ:
		status = cm_add_connect_req_to_list(cm_ctx, data);
		if (QDF_IS_STATUS_ERROR(status)) {
			/* if fail to add req return failure */
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_CONNECTING);
		cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_CONNECT_START,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_CONNECT_FAILURE:
		cm_connect_complete(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_DONE:
		cm_disconnect_complete(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_REQ:
		cm_handle_discon_req_in_non_connected_state(cm_ctx, data,
							    WLAN_CM_S_INIT);
		/*
		 * Return not handled as this req need to be dropped and return
		 * failure to the requester
		 */
		event_handled = false;
		break;
	case WLAN_CM_SM_EV_ROAM_SYNC:
		/**
		 * If it's a legacy to MLO roaming, bringup the link vdev to
		 * process ROAM_SYNC indication on the link vdev.
		 */
		if (wlan_vdev_mlme_is_mlo_link_vdev(cm_ctx->vdev)) {
			cm_sm_transition_to(cm_ctx, WLAN_CM_S_CONNECTED);
			cm_sm_deliver_event_sync(cm_ctx,
						 WLAN_CM_SM_EV_ROAM_SYNC,
						 data_len, data);
		} else {
			event_handled = false;
		}
		break;
	default:
		event_handled = false;
		break;
	}

	return event_handled;
}

/**
 * cm_state_connecting_entry() - Entry API for connecting state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to connecting state
 *
 * Return: void
 */
static void cm_state_connecting_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	cm_sm_state_update(cm_ctx, WLAN_CM_S_CONNECTING, WLAN_CM_SS_IDLE);
}

/**
 * cm_state_connecting_exit() - Exit API for connecting state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from connecting state
 *
 * Return: void
 */
static void cm_state_connecting_exit(void *ctx)
{
}

/**
 * cm_state_connecting_event() - Connecting State event handler for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to handle events in CONNECTING state
 *
 * Return: bool
 */
static bool cm_state_connecting_event(void *ctx, uint16_t event,
				      uint16_t data_len, void *data)
{
	struct cnx_mgr *cm_ctx = ctx;
	bool event_handled = true;

	switch (event) {
	case WLAN_CM_SM_EV_CONNECT_START:
		cm_sm_transition_to(cm_ctx, WLAN_CM_SS_JOIN_PENDING);
		cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
		break;
	default:
		event_handled = false;
		break;
	}

	return event_handled;
}

/**
 * cm_state_connected_entry() - Entry API for connected state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to connected state
 *
 * Return: void
 */
static void cm_state_connected_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	cm_sm_state_update(cm_ctx, WLAN_CM_S_CONNECTED, WLAN_CM_SS_IDLE);
}

/**
 * cm_state_connected_exit() - Exit API for connected state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from connected state
 *
 * Return: void
 */
static void cm_state_connected_exit(void *ctx)
{
}

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static
bool cm_handle_fw_roam_connected_event(struct cnx_mgr *cm_ctx, uint16_t event,
				       uint16_t data_len, void *data)
{
	bool event_handled = true;
	QDF_STATUS status;
	struct cm_req *roam_cm_req;

	switch (event) {
	case WLAN_CM_SM_EV_ROAM_INVOKE:
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_ROAMING);
		cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
		break;
	case WLAN_CM_SM_EV_ROAM_ABORT:
	case WLAN_CM_SM_EV_ROAM_INVOKE_FAIL:
	case WLAN_CM_SM_EV_ROAM_HO_FAIL:
		cm_remove_cmd(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_ROAM_START:
		status = cm_prepare_roam_cmd(cm_ctx, &roam_cm_req,
					     CM_ROAMING_FW);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_ROAMING);
		cm_sm_deliver_event_sync(cm_ctx, event,
					 sizeof(*roam_cm_req), roam_cm_req);
		break;
	case WLAN_CM_SM_EV_ROAM_SYNC:
		status = cm_prepare_roam_cmd(cm_ctx, &roam_cm_req,
					     CM_ROAMING_FW);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		status = cm_add_fw_roam_cmd_to_list_n_ser(cm_ctx, roam_cm_req);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_ROAMING);
		cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
		break;
	case WLAN_CM_SM_EV_ROAM_DONE:
		cm_fw_roam_complete(cm_ctx, data);
		break;
	default:
		event_handled = false;
		break;
	}

	return event_handled;
}
#else /* WLAN_FEATURE_ROAM_OFFLOAD */
static inline
bool cm_handle_fw_roam_connected_event(struct cnx_mgr *cm_ctx, uint16_t event,
				       uint16_t data_len, void *data)
{
	return false;
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

static bool
cm_handle_roam_connected_event(struct cnx_mgr *cm_ctx, uint16_t event,
			       uint16_t data_len, void *data)
{
	bool event_handled = true;

	/* Handle roam event only if roam is enabled */
	if (!cm_is_roam_enabled(wlan_vdev_get_psoc(cm_ctx->vdev)))
		return false;

	switch (event) {
	case WLAN_CM_SM_EV_ROAM_REQ:
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_ROAMING);
		cm_sm_deliver_event_sync(cm_ctx,
					 WLAN_CM_SM_EV_ROAM_REQ,
					 data_len, data);
		break;
	default:
		event_handled =
			cm_handle_fw_roam_connected_event(cm_ctx, event,
							  data_len, data);
		break;
	}

	return event_handled;
}
#else /* WLAN_FEATURE_HOST_ROAM || WLAN_FEATURE_ROAM_OFFLOAD */
static inline
bool cm_handle_roam_connected_event(struct cnx_mgr *cm_ctx, uint16_t event,
				    uint16_t data_len, void *data)
{
	return false;
}
#endif /* WLAN_FEATURE_HOST_ROAM || WLAN_FEATURE_ROAM_OFFLOAD */

/**
 * cm_state_connected_event() - Connected State event handler for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to handle events in CONNECTED state
 *
 * Return: bool
 */
static bool cm_state_connected_event(void *ctx, uint16_t event,
				     uint16_t data_len, void *data)
{
	struct cnx_mgr *cm_ctx = ctx;
	bool event_handled = true;
	QDF_STATUS status;
	struct cm_req *roam_cm_req;

	switch (event) {
	case WLAN_CM_SM_EV_CONNECT_REQ:
		status = cm_check_and_prepare_roam_req(cm_ctx, data,
						       &roam_cm_req);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			cm_sm_deliver_event_sync(cm_ctx,
						 WLAN_CM_SM_EV_ROAM_REQ,
						 sizeof(*roam_cm_req),
						 roam_cm_req);
			break;
		}
		status = cm_handle_connect_req_in_non_init_state(cm_ctx, data,
							WLAN_CM_S_CONNECTED);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_CONNECTING);
		cm_sm_deliver_event_sync(cm_ctx,
					 WLAN_CM_SM_EV_CONNECT_START,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_ACTIVE:
		cm_disconnect_active(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_CONNECT_SUCCESS:
		cm_connect_complete(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_REQ:
		status = cm_add_disconnect_req_to_list(cm_ctx, data);
		if (QDF_IS_STATUS_ERROR(status)) {
			/* if fail to add req return failure */
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_DISCONNECTING);
		cm_sm_deliver_event_sync(cm_ctx, WLAN_CM_SM_EV_DISCONNECT_START,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_REASSOC_DONE:
		cm_reassoc_complete(cm_ctx, data);
		break;
	default:
		event_handled =
			cm_handle_roam_connected_event(cm_ctx, event,
						       data_len, data);
		break;
	}
	return event_handled;
}

/**
 * cm_state_disconnecting_entry() - Entry API for disconnecting state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to disconnecting state
 *
 * Return: void
 */
static void cm_state_disconnecting_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	cm_sm_state_update(cm_ctx, WLAN_CM_S_DISCONNECTING, WLAN_CM_SS_IDLE);
}

/**
 * cm_state_disconnecting_exit() - Exit API for disconnecting state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from disconnecting state
 *
 * Return: void
 */
static void cm_state_disconnecting_exit(void *ctx)
{
}

/**
 * cm_state_connected_event() - Disconnecting State event handler for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to handle events in Disconnecting state
 *
 * Return: bool
 */
static bool cm_state_disconnecting_event(void *ctx, uint16_t event,
					 uint16_t data_len, void *data)
{
	struct cnx_mgr *cm_ctx = ctx;
	bool event_handled = true;
	QDF_STATUS status;

	switch (event) {
	case WLAN_CM_SM_EV_CONNECT_REQ:
		status = cm_handle_connect_req_in_non_init_state(cm_ctx, data,
						WLAN_CM_S_DISCONNECTING);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_CONNECTING);
		cm_sm_deliver_event_sync(cm_ctx,
					 WLAN_CM_SM_EV_CONNECT_START,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_START:
		cm_disconnect_start(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_ACTIVE:
		cm_disconnect_active(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_DONE:
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_INIT);
		cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_REQ:
		status = cm_handle_discon_req_in_non_connected_state(cm_ctx,
						data, WLAN_CM_S_DISCONNECTING);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_deliver_event_sync(cm_ctx,
					 WLAN_CM_SM_EV_DISCONNECT_START,
					 data_len, data);
		break;
	default:
		event_handled = false;
		break;
	}

	return event_handled;
}

/**
 * cm_subst_join_pending_entry() - Entry API for join pending sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to join pending sub-state
 *
 * Return: void
 */
static void cm_subst_join_pending_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	if (cm_get_state(cm_ctx) != WLAN_CM_S_CONNECTING)
		QDF_BUG(0);

	cm_set_substate(cm_ctx, WLAN_CM_SS_JOIN_PENDING);
}

/**
 * cm_subst_join_pending_exit() - Exit API for join pending sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from join pending sub-state
 *
 * Return: void
 */
static void cm_subst_join_pending_exit(void *ctx)
{
}

/**
 * cm_subst_join_pending_event() - Join pending sub-state event handler for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to handle events in Join pending sub-state
 *
 * Return: bool
 */
static bool cm_subst_join_pending_event(void *ctx, uint16_t event,
					uint16_t data_len, void *data)
{
	struct cnx_mgr *cm_ctx = ctx;
	bool event_handled = true;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_cm_connect_resp *resp;
	struct cm_req *cm_req;

	switch (event) {
	case WLAN_CM_SM_EV_CONNECT_REQ:
		status =
			cm_handle_connect_req_in_non_init_state(cm_ctx, data,
						WLAN_CM_SS_JOIN_PENDING);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_deliver_event_sync(cm_ctx,
					 WLAN_CM_SM_EV_CONNECT_START,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_CONNECT_START:
		cm_connect_start(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_CONNECT_ACTIVE:
		/* check if cm id is valid for the current req */
		if (!cm_check_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_SS_JOIN_ACTIVE);
		cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
		break;
	case WLAN_CM_SM_EV_HW_MODE_SUCCESS:
	case WLAN_CM_SM_EV_HW_MODE_FAILURE:
		/* check if cm id is valid for the current req */
		if (!cm_check_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_handle_hw_mode_change(cm_ctx, data, event);
		break;
	case WLAN_CM_SM_EV_SCAN:
		cm_sm_transition_to(cm_ctx, WLAN_CM_SS_SCAN);
		cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
		break;
	case WLAN_CM_SM_EV_SCAN_FAILURE:
		status = QDF_STATUS_E_FAILURE;
		/* Fall through after setting status failure */
		fallthrough;
	case WLAN_CM_SM_EV_SCAN_SUCCESS:
		cm_connect_scan_resp(cm_ctx, data, status);
		break;
	case WLAN_CM_SM_EV_CONNECT_FAILURE:
		/* check if connect resp cm id is valid for the current req */
		if (!cm_connect_resp_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		/*
		 * On connect req failure (before serialization), if there is a
		 * pending disconnect req then move to disconnecting state and
		 * wait for disconnect to complete before moving to INIT state.
		 * Else directly transition to INIT state.
		 *
		 * On disconnect completion or a new connect/disconnect req in
		 * disconnnecting state, the failed connect req will be flushed.
		 * This will ensure SM moves to INIT state after completion of
		 * all operation.
		 */
		if (cm_ctx->disconnect_count) {
			resp = data;

			mlme_debug(CM_PREFIX_FMT "disconnect_count %d",
				   CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
						 resp->cm_id),
				   cm_ctx->disconnect_count);
			cm_req = cm_get_req_by_cm_id(cm_ctx, resp->cm_id);
			if (cm_req)
				cm_req->failed_req = true;
			cm_sm_transition_to(cm_ctx, WLAN_CM_S_DISCONNECTING);
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_INIT);
		cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_ACTIVE:
		cm_disconnect_active(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_DONE:
		cm_disconnect_complete(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_REQ:
		status = cm_handle_discon_req_in_non_connected_state(cm_ctx,
						data, WLAN_CM_SS_JOIN_PENDING);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_DISCONNECTING);
		cm_sm_deliver_event_sync(cm_ctx,
					 WLAN_CM_SM_EV_DISCONNECT_START,
					 data_len, data);
		break;
	default:
		event_handled = false;
		break;
	}

	return event_handled;
}

/**
 * cm_subst_scan_entry() - Entry API for scan sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to scan sub-state
 *
 * Return: void
 */
static void cm_subst_scan_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	if (cm_get_state(cm_ctx) != WLAN_CM_S_CONNECTING)
		QDF_BUG(0);

	cm_set_substate(cm_ctx, WLAN_CM_SS_SCAN);
}

/**
 * cm_subst_scan_exit() - Exit API for scan sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from scan sub-state
 *
 * Return: void
 */
static void cm_subst_scan_exit(void *ctx)
{
}

/**
 * cm_subst_scan_event() - Scan sub-state event handler for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to handle events in scan sub-state
 *
 * Return: bool
 */
static bool cm_subst_scan_event(void *ctx, uint16_t event,
				uint16_t data_len, void *data)
{
	struct cnx_mgr *cm_ctx = ctx;
	bool event_handled = true;
	QDF_STATUS status;

	switch (event) {
	case WLAN_CM_SM_EV_CONNECT_REQ:
		status = cm_handle_connect_req_in_non_init_state(cm_ctx, data,
							WLAN_CM_SS_SCAN);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_SS_JOIN_PENDING);
		cm_sm_deliver_event_sync(cm_ctx,
					 WLAN_CM_SM_EV_CONNECT_START,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_SCAN:
		cm_connect_scan_start(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_SCAN_SUCCESS:
	case WLAN_CM_SM_EV_SCAN_FAILURE:
		/* check if scan id is valid for the current req */
		if (!cm_check_scanid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_SS_JOIN_PENDING);
		cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_ACTIVE:
		cm_disconnect_active(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_DONE:
		cm_disconnect_complete(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_REQ:
		status = cm_handle_discon_req_in_non_connected_state(cm_ctx,
						data, WLAN_CM_SS_SCAN);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_DISCONNECTING);
		cm_sm_deliver_event_sync(cm_ctx,
					 WLAN_CM_SM_EV_DISCONNECT_START,
					 data_len, data);
		break;
	default:
		event_handled = false;
		break;
	}

	return event_handled;
}

/**
 * cm_subst_join_active_entry() - Entry API for join active sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on moving to join active sub-state
 *
 * Return: void
 */
static void cm_subst_join_active_entry(void *ctx)
{
	struct cnx_mgr *cm_ctx = ctx;

	if (cm_get_state(cm_ctx) != WLAN_CM_S_CONNECTING)
		QDF_BUG(0);

	cm_set_substate(cm_ctx, WLAN_CM_SS_JOIN_ACTIVE);
}

/**
 * cm_subst_join_active_exit() - Exit API for join active sub-state for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to perform operations on exiting from join active sub-state
 *
 * Return: void
 */
static void cm_subst_join_active_exit(void *ctx)
{
}

/**
 * cm_subst_join_active_event() - Join active sub-state event handler for
 * connection mgr
 * @ctx: connection manager ctx
 *
 * API to handle events in join active sub-state
 *
 * Return: bool
 */
static bool cm_subst_join_active_event(void *ctx, uint16_t event,
				       uint16_t data_len, void *data)
{
	struct cnx_mgr *cm_ctx = ctx;
	bool event_handled = true;
	QDF_STATUS status;

	switch (event) {
	case WLAN_CM_SM_EV_CONNECT_REQ:
		status = cm_handle_connect_req_in_non_init_state(cm_ctx, data,
							WLAN_CM_SS_JOIN_ACTIVE);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_SS_JOIN_PENDING);
		cm_sm_deliver_event_sync(cm_ctx,
					 WLAN_CM_SM_EV_CONNECT_START,
					 data_len, data);
		break;
	case WLAN_CM_SM_EV_CONNECT_ACTIVE:
		cm_connect_active(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_CONNECT_SUCCESS:
		/* check if connect resp cm id is valid for the current req */
		if (!cm_connect_resp_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_CONNECTED);
		cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
		break;
	case WLAN_CM_SM_EV_CONNECT_GET_NEXT_CANDIDATE:
		/* check if connect resp cm id is valid for the current req */
		if (!cm_connect_resp_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_try_next_candidate(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_CONNECT_FAILURE:
		/* check if connect resp cm id is valid for the current req */
		if (!cm_connect_resp_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_INIT);
		cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
		break;
	case WLAN_CM_SM_EV_BSS_SELECT_IND_SUCCESS:
		/* check if cm id is valid for the current req */
		if (!cm_check_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_peer_create_on_bss_select_ind_resp(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_BSS_CREATE_PEER_SUCCESS:
		/* check if cm id is valid for the current req */
		if (!cm_check_cmid_match_list_head(cm_ctx, data)) {
			event_handled = false;
			break;
		}
		cm_resume_connect_after_peer_create(cm_ctx, data);
		break;
	case WLAN_CM_SM_EV_DISCONNECT_REQ:
		status = cm_handle_discon_req_in_non_connected_state(cm_ctx,
						data, WLAN_CM_SS_JOIN_ACTIVE);
		if (QDF_IS_STATUS_ERROR(status)) {
			event_handled = false;
			break;
		}
		cm_sm_transition_to(cm_ctx, WLAN_CM_S_DISCONNECTING);
		cm_sm_deliver_event_sync(cm_ctx,
					 WLAN_CM_SM_EV_DISCONNECT_START,
					 data_len, data);
		break;
	default:
		event_handled = false;
		break;
	}

	return event_handled;
}

struct wlan_sm_state_info cm_sm_info[] = {
	{
		(uint8_t)WLAN_CM_S_INIT,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		true,
		"INIT",
		cm_state_init_entry,
		cm_state_init_exit,
		cm_state_init_event
	},
	{
		(uint8_t)WLAN_CM_S_CONNECTING,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		true,
		"CONNECTING",
		cm_state_connecting_entry,
		cm_state_connecting_exit,
		cm_state_connecting_event
	},
	{
		(uint8_t)WLAN_CM_S_CONNECTED,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		true,
		"CONNECTED",
		cm_state_connected_entry,
		cm_state_connected_exit,
		cm_state_connected_event
	},
	{
		(uint8_t)WLAN_CM_S_DISCONNECTING,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		true,
		"DISCONNECTING",
		cm_state_disconnecting_entry,
		cm_state_disconnecting_exit,
		cm_state_disconnecting_event
	},
	{
		(uint8_t)WLAN_CM_S_ROAMING,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		true,
		"ROAMING",
		cm_state_roaming_entry,
		cm_state_roaming_exit,
		cm_state_roaming_event
	},
	{
		(uint8_t)WLAN_CM_S_MAX,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"INVALID",
		NULL,
		NULL,
		NULL
	},
	{
		(uint8_t)WLAN_CM_SS_IDLE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"IDLE",
		NULL,
		NULL,
		NULL
	},
	{
		(uint8_t)WLAN_CM_SS_JOIN_PENDING,
		(uint8_t)WLAN_CM_S_CONNECTING,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"JOIN_PENDING",
		cm_subst_join_pending_entry,
		cm_subst_join_pending_exit,
		cm_subst_join_pending_event
	},
	{
		(uint8_t)WLAN_CM_SS_SCAN,
		(uint8_t)WLAN_CM_S_CONNECTING,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"SCAN",
		cm_subst_scan_entry,
		cm_subst_scan_exit,
		cm_subst_scan_event
	},
	{
		(uint8_t)WLAN_CM_SS_JOIN_ACTIVE,
		(uint8_t)WLAN_CM_S_CONNECTING,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"JOIN_ACTIVE",
		cm_subst_join_active_entry,
		cm_subst_join_active_exit,
		cm_subst_join_active_event
	},
	{
		(uint8_t)WLAN_CM_SS_PREAUTH,
		(uint8_t)WLAN_CM_S_ROAMING,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"PREAUTH",
		cm_subst_preauth_entry,
		cm_subst_preauth_exit,
		cm_subst_preauth_event
	},
	{
		(uint8_t)WLAN_CM_SS_REASSOC,
		(uint8_t)WLAN_CM_S_ROAMING,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"REASSOC",
		cm_subst_reassoc_entry,
		cm_subst_reassoc_exit,
		cm_subst_reassoc_event
	},
	{
		(uint8_t)WLAN_CM_SS_ROAM_STARTED,
		(uint8_t)WLAN_CM_S_ROAMING,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"ROAM_START",
		cm_subst_roam_start_entry,
		cm_subst_roam_start_exit,
		cm_subst_roam_start_event
	},
	{
		(uint8_t)WLAN_CM_SS_ROAM_SYNC,
		(uint8_t)WLAN_CM_S_ROAMING,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"ROAM_SYNC",
		cm_subst_roam_sync_entry,
		cm_subst_roam_sync_exit,
		cm_subst_roam_sync_event
	},
	{
		(uint8_t)WLAN_CM_SS_MAX,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"INVALID",
		NULL,
		NULL,
		NULL
	},
};

static const char *cm_sm_event_names[] = {
	"EV_CONNECT_REQ",
	"EV_SCAN",
	"EV_SCAN_SUCCESS",
	"EV_SCAN_FAILURE",
	"EV_HW_MODE_SUCCESS",
	"EV_HW_MODE_FAILURE",
	"EV_CONNECT_START",
	"EV_CONNECT_ACTIVE",
	"EV_CONNECT_SUCCESS",
	"EV_BSS_SELECT_IND_SUCCESS",
	"EV_BSS_CREATE_PEER_SUCCESS",
	"EV_CONNECT_GET_NXT_CANDIDATE",
	"EV_CONNECT_FAILURE",
	"EV_DISCONNECT_REQ",
	"EV_DISCONNECT_START",
	"EV_DISCONNECT_ACTIVE",
	"EV_DISCONNECT_DONE",
	"EV_ROAM_START",
	"EV_ROAM_SYNC",
	"EV_ROAM_INVOKE_FAIL",
	"EV_ROAM_HO_FAIL",
	"EV_PREAUTH_DONE",
	"EV_GET_NEXT_PREAUTH_AP",
	"EV_PREAUTH_FAIL",
	"EV_START_REASSOC",
	"EV_REASSOC_ACTIVE",
	"EV_REASSOC_DONE",
	"EV_REASSOC_FAILURE",
	"EV_ROAM_COMPLETE",
	"EV_ROAM_REQ",
	"EV_ROAM_INVOKE",
	"EV_ROAM_ABORT",
	"EV_ROAM_DONE",
	"EV_PREAUTH_ACTIVE",
	"EV_PREAUTH_RESP",
	"EV_REASSOC_TIMER",
	"EV_HO_ROAM_DISCONNECT_DONE",
};

enum wlan_cm_sm_state cm_get_state(struct cnx_mgr *cm_ctx)
{
	enum QDF_OPMODE op_mode;

	if (!cm_ctx || !cm_ctx->vdev)
		return WLAN_CM_S_MAX;

	op_mode = wlan_vdev_mlme_get_opmode(cm_ctx->vdev);

	if (op_mode != QDF_STA_MODE && op_mode != QDF_P2P_CLIENT_MODE)
		return WLAN_CM_S_MAX;

	return cm_ctx->sm.cm_state;
}

enum wlan_cm_sm_state cm_get_sub_state(struct cnx_mgr *cm_ctx)
{
	enum QDF_OPMODE op_mode;

	if (!cm_ctx || !cm_ctx->vdev)
		return WLAN_CM_SS_MAX;

	op_mode = wlan_vdev_mlme_get_opmode(cm_ctx->vdev);

	if (op_mode != QDF_STA_MODE && op_mode != QDF_P2P_CLIENT_MODE)
		return WLAN_CM_SS_MAX;

	return cm_ctx->sm.cm_substate;
}

static void cm_sm_print_state_event(struct cnx_mgr *cm_ctx,
				    enum wlan_cm_sm_evt event)
{
	enum wlan_cm_sm_state state;
	enum wlan_cm_sm_state substate;

	state = cm_get_state(cm_ctx);
	substate = cm_get_sub_state(cm_ctx);

	mlme_nofl_debug("[%s]%s - %s, %s", cm_ctx->sm.sm_hdl->name,
			cm_sm_info[state].name, cm_sm_info[substate].name,
			cm_sm_event_names[event]);
}

static void cm_sm_print_state(struct cnx_mgr *cm_ctx)
{
	enum wlan_cm_sm_state state;
	enum wlan_cm_sm_state substate;

	state = cm_get_state(cm_ctx);
	substate = cm_get_sub_state(cm_ctx);

	mlme_nofl_debug("[%s]%s - %s", cm_ctx->sm.sm_hdl->name,
			cm_sm_info[state].name, cm_sm_info[substate].name);
}

QDF_STATUS cm_sm_deliver_event(struct wlan_objmgr_vdev *vdev,
			       enum wlan_cm_sm_evt event,
			       uint16_t data_len, void *data)
{
	QDF_STATUS status;
	enum wlan_cm_sm_state state_entry, state_exit;
	enum wlan_cm_sm_state substate_entry, substate_exit;
	enum QDF_OPMODE op_mode = wlan_vdev_mlme_get_opmode(vdev);
	struct cnx_mgr *cm_ctx;

	if (op_mode != QDF_STA_MODE && op_mode != QDF_P2P_CLIENT_MODE) {
		mlme_err("vdev %d Invalid mode %d",
			 wlan_vdev_get_id(vdev), op_mode);
		return QDF_STATUS_E_NOSUPPORT;
	}

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return QDF_STATUS_E_FAILURE;

	cm_lock_acquire(cm_ctx);

	/* store entry state and sub state for prints */
	state_entry = cm_get_state(cm_ctx);
	substate_entry = cm_get_sub_state(cm_ctx);
	cm_sm_print_state_event(cm_ctx, event);

	status = cm_sm_deliver_event_sync(cm_ctx, event, data_len, data);
	/* Take exit state, exit substate for prints */
	state_exit = cm_get_state(cm_ctx);
	substate_exit = cm_get_sub_state(cm_ctx);
	/* If no state and substate change, don't print */
	if (!((state_entry == state_exit) && (substate_entry == substate_exit)))
		cm_sm_print_state(cm_ctx);
	cm_lock_release(cm_ctx);

	return status;
}

QDF_STATUS cm_sm_create(struct cnx_mgr *cm_ctx)
{
	struct wlan_sm *sm;
	uint8_t name[WLAN_SM_ENGINE_MAX_NAME];

	qdf_scnprintf(name, sizeof(name), "CM-VDEV-%d",
		      wlan_vdev_get_id(cm_ctx->vdev));
	sm = wlan_sm_create(name, cm_ctx,
			    WLAN_CM_S_INIT,
			    cm_sm_info,
			    QDF_ARRAY_SIZE(cm_sm_info),
			    cm_sm_event_names,
			    QDF_ARRAY_SIZE(cm_sm_event_names));
	if (!sm) {
		mlme_err("vdev %d CM State Machine allocation failed",
			 wlan_vdev_get_id(cm_ctx->vdev));
		return QDF_STATUS_E_NOMEM;
	}
	cm_ctx->sm.sm_hdl = sm;

	cm_lock_create(cm_ctx);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cm_sm_destroy(struct cnx_mgr *cm_ctx)
{
	cm_lock_destroy(cm_ctx);
	wlan_sm_delete(cm_ctx->sm.sm_hdl);

	return QDF_STATUS_SUCCESS;
}

#ifdef SM_ENG_HIST_ENABLE
void cm_sm_history_print(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx) {
		mlme_err("cm_ctx is NULL");
		return;
	}

	return wlan_sm_print_history(cm_ctx->sm.sm_hdl);
}
#endif
