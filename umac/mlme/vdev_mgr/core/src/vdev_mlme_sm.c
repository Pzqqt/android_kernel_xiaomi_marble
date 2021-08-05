/*
 * Copyright (c) 2018-2021 The Linux Foundation. All rights reserved.
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
 * DOC: Implements VDEV MLME SM
 */

#include <wlan_objmgr_vdev_obj.h>
#include <wlan_mlme_dbg.h>
#include <wlan_sm_engine.h>
#include "include/wlan_vdev_mlme.h"
#include "vdev_mlme_sm.h"
#include <wlan_utility.h>
#include <include/wlan_mlme_cmn.h>

/**
 * mlme_vdev_set_state() - set mlme state
 * @vdev: VDEV object
 * @state: MLME state
 *
 * API to set MLME state
 *
 * Return: void
 */
static void mlme_vdev_set_state(struct wlan_objmgr_vdev *vdev,
				enum wlan_vdev_state state)
{
	if (state < WLAN_VDEV_S_MAX) {
		vdev->vdev_mlme.mlme_state = state;
	} else {
		mlme_err("mlme state (%d) is invalid", state);
		QDF_BUG(0);
	}
}

/**
 * mlme_vdev_set_substate() - set mlme sub state
 * @vdev: VDEV object
 * @substate: MLME sub state
 *
 * API to set MLME sub state
 *
 * Return: void
 */
static void mlme_vdev_set_substate(struct wlan_objmgr_vdev *vdev,
				   enum wlan_vdev_state substate)
{
	if ((substate > WLAN_VDEV_S_MAX) && (substate < WLAN_VDEV_SS_MAX)) {
		vdev->vdev_mlme.mlme_substate = substate;
	} else {
		mlme_err(" mlme sub state (%d) is invalid", substate);
		QDF_BUG(0);
	}
}

/**
 * mlme_vdev_sm_state_update() - set mlme state and sub state
 * @vdev_mlme: MLME VDEV comp object
 * @state: MLME state
 * @substate: MLME sub state
 *
 * API to invoke util APIs to set state and MLME sub state
 *
 * Return: void
 */
static void mlme_vdev_sm_state_update(struct vdev_mlme_obj *vdev_mlme,
				      enum wlan_vdev_state state,
				      enum wlan_vdev_state substate)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;
	if (!vdev) {
		mlme_err(" VDEV is NULL");
		QDF_BUG(0);
	}

	mlme_vdev_set_state(vdev, state);
	mlme_vdev_set_substate(vdev, substate);
}

/**
 * mlme_vdev_sm_transition_to() - invokes state transition
 * @vdev_mlme: MLME VDEV comp object
 * @state: new MLME state
 *
 * API to invoke SM API to move to new state
 *
 * Return: void
 */
static void mlme_vdev_sm_transition_to(struct vdev_mlme_obj *vdev_mlme,
				       enum wlan_vdev_state state)
{
	wlan_sm_transition_to(vdev_mlme->sm_hdl, state);
}

/**
 * mlme_vdev_state_init_entry() - Entry API for Init state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to INIT state
 *
 * Return: void
 */
static void mlme_vdev_state_init_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;

	mlme_vdev_sm_state_update(vdev_mlme, WLAN_VDEV_S_INIT,
				  WLAN_VDEV_SS_IDLE);
}

/**
 * mlme_vdev_state_init_exit() - Exit API for Init state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of INIT state
 *
 * Return: void
 */
static void mlme_vdev_state_init_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_state_init_event() - Init State event handler
 * @ctx: VDEV MLME object
 *
 * API to handle events in INIT state
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_state_init_event(void *ctx, uint16_t event,
				       uint16_t event_data_len,
				       void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	bool status;
	enum QDF_OPMODE mode;

	mode = wlan_vdev_mlme_get_opmode(vdev_mlme->vdev);

	switch (event) {
	case WLAN_VDEV_SM_EV_START:
		/* call mlme callback API for sanity checks */
		if (mlme_vdev_validate_basic_params(vdev_mlme, event_data_len,
					event_data) == QDF_STATUS_SUCCESS) {
			mlme_vdev_sm_transition_to(vdev_mlme,
						   WLAN_VDEV_S_START);
			mlme_vdev_sm_deliver_event(vdev_mlme,
						   WLAN_VDEV_SM_EV_START_REQ,
						   event_data_len, event_data);
			status = true;
		} else {
			mlme_err(
			"failed to validate vdev init params to move to START state");
			/*
			 * In case of AP if false is returned, we consider as
			 * error scenario and print that the event is not
			 * handled. Hence return false only for STA.
			 */
			if (mode == QDF_STA_MODE)
				status = false;
			else
				status = true;
			mlme_vdev_notify_down_complete(vdev_mlme,
						       event_data_len,
						       event_data);
		}
		break;

	case WLAN_VDEV_SM_EV_DOWN_COMPLETE:
	case WLAN_VDEV_SM_EV_DOWN:
	case WLAN_VDEV_SM_EV_START_REQ_FAIL:
		/* already in down state, notify DOWN command is completed */
		/* NOTE: Keep this function call always at the end, to allow
		 * connection restart from this event
		 */
		mlme_vdev_notify_down_complete(vdev_mlme, event_data_len,
					       event_data);
		mlme_vdev_down_cmpl_notify_mlo_mgr(vdev_mlme);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_state_start_entry() - Entry API for Start state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to START state
 *
 * Return: void
 */
static void mlme_vdev_state_start_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;

	mlme_vdev_sm_state_update(vdev_mlme, WLAN_VDEV_S_START,
				  WLAN_VDEV_SS_IDLE);
}

/**
 * mlme_vdev_state_start_exit() - Exit API for Start state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of START state
 *
 * Return: void
 */
static void mlme_vdev_state_start_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_state_start_event() - Start State event handler
 * @ctx: VDEV MLME object
 *
 * API to handle events in START state
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_state_start_event(void *ctx, uint16_t event,
					uint16_t event_data_len,
					void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	bool status;

	switch (event) {
	case WLAN_VDEV_SM_EV_START_REQ:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_START_START_PROGRESS);
		mlme_vdev_sm_deliver_event(vdev_mlme, event, event_data_len,
					   event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_RESTART_REQ:
	case WLAN_VDEV_SM_EV_RADAR_DETECTED:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_START_RESTART_PROGRESS);
		mlme_vdev_sm_deliver_event(vdev_mlme, event, event_data_len,
					   event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_STA_CONN_START:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_START_CONN_PROGRESS);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_state_dfs_cac_wait_entry() - Entry API for DFS CAC WAIT state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to DFS CAC WAIT state
 *
 * Return: void
 */
static void mlme_vdev_state_dfs_cac_wait_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;

	mlme_vdev_sm_state_update(vdev_mlme, WLAN_VDEV_S_DFS_CAC_WAIT,
				  WLAN_VDEV_SS_IDLE);
}

/**
 * mlme_vdev_state_dfs_cac_wait_exit() - Exit API for DFS CAC WAIT state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of DFS CAC WAIT state
 *
 * Return: void
 */
static void mlme_vdev_state_dfs_cac_wait_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_state_dfs_cac_wait_event() - DFS CAC WAIT State event handler
 * @ctx: VDEV MLME object
 *
 * API to handle events in DFS CAC WAIT state
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_state_dfs_cac_wait_event(void *ctx, uint16_t event,
					       uint16_t event_data_len,
					       void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	enum QDF_OPMODE mode;
	struct wlan_objmgr_vdev *vdev;
	bool status;

	vdev = vdev_mlme->vdev;

	mode = wlan_vdev_mlme_get_opmode(vdev);

	switch (event) {
	case WLAN_VDEV_SM_EV_DFS_CAC_WAIT:
		/* Notify MLME about CAC wait state, MLME can perform
		 * unblocking of some commands
		 */
		mlme_vdev_dfs_cac_wait_notify(vdev_mlme);
		/* DFS timer should have started already, then only this event
		 * could have been triggered
		 */
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DOWN:
		/* stop the CAC timer, then notify state machine */
		mlme_vdev_dfs_cac_timer_stop(vdev_mlme, event_data_len,
					     event_data);
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_STOP);
		mlme_vdev_sm_deliver_event(vdev_mlme, WLAN_VDEV_SM_EV_STOP_REQ,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_RADAR_DETECTED:
		/* the random channel should have been selected, before issuing
		 * this event
		 */
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_START);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_RESTART_REQ,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DFS_CAC_COMPLETED:
		if (mode == QDF_STA_MODE) {
			mlme_vdev_sm_transition_to(vdev_mlme,
						   WLAN_VDEV_S_START);
			mlme_vdev_sm_deliver_event(vdev_mlme,
						WLAN_VDEV_SM_EV_STA_CONN_START,
						event_data_len, event_data);
		} else {
			mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_UP);
			mlme_vdev_sm_deliver_event(vdev_mlme,
						WLAN_VDEV_SM_EV_START_SUCCESS,
						event_data_len, event_data);
		}
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_state_up_entry() - Entry API for UP state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to UP state
 *
 * Return: void
 */
static void mlme_vdev_state_up_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;

	mlme_vdev_sm_state_update(vdev_mlme, WLAN_VDEV_S_UP,
				  WLAN_VDEV_SS_IDLE);
}

/**
 * mlme_vdev_state_up_exit() - Exit API for UP state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of UP state
 *
 * Return: void
 */
static void mlme_vdev_state_up_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_state_up_event() - UP State event handler
 * @ctx: VDEV MLME object
 *
 * API to handle events in UP state
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_state_up_event(void *ctx, uint16_t event,
				     uint16_t event_data_len, void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	enum QDF_OPMODE mode;
	struct wlan_objmgr_vdev *vdev;
	bool status;

	vdev = vdev_mlme->vdev;
	mode = wlan_vdev_mlme_get_opmode(vdev);

	switch (event) {
	case WLAN_VDEV_SM_EV_START_SUCCESS:
		if (wlan_vdev_mlme_is_mlo_ap(vdev))
			mlme_vdev_sm_transition_to(vdev_mlme,
						   WLAN_VDEV_SS_MLO_SYNC_WAIT);
		else
			mlme_vdev_sm_transition_to(vdev_mlme,
						   WLAN_VDEV_SS_UP_ACTIVE);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	/**
	 * Channel switch disabled case, then tansition to up state
	 * and deliver EV_UP_HOST_RESTART, hand it in up state and
	 * move to up active state
	 */
	case WLAN_VDEV_SM_EV_UP_HOST_RESTART:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_UP_ACTIVE);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_state_suspend_entry() - Entry API for Suspend state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to SUSPEND state
 *
 * Return: void
 */
static void mlme_vdev_state_suspend_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;

	mlme_vdev_sm_state_update(vdev_mlme, WLAN_VDEV_S_SUSPEND,
				  WLAN_VDEV_SS_IDLE);
}

/**
 * mlme_vdev_state_suspend_exit() - Exit API for Suspend state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of SUSPEND state
 *
 * Return: void
 */
static void mlme_vdev_state_suspend_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_state_suspend_event() - Suspend State event handler
 * @ctx: VDEV MLME object
 *
 * API to handle events in SUSPEND state
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_state_suspend_event(void *ctx, uint16_t event,
					  uint16_t event_data_len,
					  void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	bool status;

	switch (event) {
	case WLAN_VDEV_SM_EV_DOWN:
	case WLAN_VDEV_SM_EV_RESTART_REQ_FAIL:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_SUSPEND_SUSPEND_DOWN);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_SUSPEND_RESTART:
		mlme_vdev_sm_transition_to(vdev_mlme,
					  WLAN_VDEV_SS_SUSPEND_SUSPEND_RESTART);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_HOST_RESTART:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_SUSPEND_HOST_RESTART);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_CSA_RESTART:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_SUSPEND_CSA_RESTART);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_UP_FAIL:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_SUSPEND_SUSPEND_DOWN);
		mlme_vdev_sm_deliver_event(vdev_mlme, WLAN_VDEV_SM_EV_DOWN,
					   event_data_len, event_data);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_state_stop_entry() - Entry API for Stop state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to STOP state
 *
 * Return: void
 */
static void mlme_vdev_state_stop_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *) ctx;

	mlme_vdev_sm_state_update(vdev_mlme, WLAN_VDEV_S_STOP,
				  WLAN_VDEV_SS_IDLE);
}

/**
 * mlme_vdev_state_stop_exit() - Exit API for Stop state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of STOP state
 *
 * Return: void
 */
static void mlme_vdev_state_stop_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_state_stop_event() - Stop State event handler
 * @ctx: VDEV MLME object
 *
 * API to handle events in STOP state
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_state_stop_event(void *ctx, uint16_t event,
				       uint16_t event_data_len,
				       void *event_data)
{
	QDF_BUG(0);
	return false;
}

/**
 * mlme_vdev_subst_start_start_progress_entry() - Entry API for Start Progress
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to START-PROGRESS substate
 *
 * Return: void
 */
static void mlme_vdev_subst_start_start_progress_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_START)
		QDF_BUG(0);

	mlme_vdev_set_substate(vdev, WLAN_VDEV_SS_START_START_PROGRESS);
}

/**
 * mlme_vdev_subst_start_start_progress_exit() - Exit API for Start Progress
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of START-PROGRESS substate
 *
 * Return: void
 */
static void mlme_vdev_subst_start_start_progress_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_subst_start_start_progress_event() - Event handler API for Start
 *                                                Progress substate
 * @ctx: VDEV MLME object
 *
 * API to handle events in START-PROGRESS substate
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_subst_start_start_progress_event(void *ctx,
		uint16_t event, uint16_t event_data_len, void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;
	bool status;

	vdev = vdev_mlme->vdev;

	switch (event) {
	case WLAN_VDEV_SM_EV_START_REQ:
		/* send vdev start req command to FW */
		mlme_vdev_start_send(vdev_mlme,	event_data_len, event_data);
		status = true;
		break;
	/* While waiting for START response, move to RESTART_PROGRESS,
	 * wait for START response to send RESTART req */
	case WLAN_VDEV_SM_EV_RADAR_DETECTED:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_START_RESTART_PROGRESS);
		status = true;
		break;
	case WLAN_VDEV_SM_EV_START_RESP:
	case WLAN_VDEV_SM_EV_RESTART_RESP:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_START_CONN_PROGRESS);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_CONN_PROGRESS,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_START_REQ_FAIL:
		mlme_vdev_start_req_failed(vdev_mlme,
					   event_data_len, event_data);
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_INIT);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DOWN:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_START_DISCONN_PROGRESS);
		/* block start request, if it is pending */
		mlme_vdev_stop_start_send(vdev_mlme, START_REQ,
					  event_data_len, event_data);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_subst_start_restart_progress_entry() - Entry API for Restart
 *                                                  progress sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to RESTART-PROGRESS substate
 *
 * Return: void
 */
static void mlme_vdev_subst_start_restart_progress_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_START)
		QDF_BUG(0);

	mlme_vdev_set_substate(vdev, WLAN_VDEV_SS_START_RESTART_PROGRESS);
}

/**
 * mlme_vdev_subst_start_restart_progress_exit() - Exit API for Restart Progress
 *                                                 sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of RESTART-PROGRESS substate
 *
 * Return: void
 */
static void mlme_vdev_subst_start_restart_progress_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_subst_start_restart_progress_event() - Event handler API for
 *                                                  Restart Progress substate
 * @ctx: VDEV MLME object
 *
 * API to handle events in RESTART-PROGRESS substate
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_subst_start_restart_progress_event(void *ctx,
		uint16_t event, uint16_t event_data_len, void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;
	bool status;

	vdev = vdev_mlme->vdev;

	switch (event) {
	case WLAN_VDEV_SM_EV_RESTART_REQ:
	/* If Start resp is pending, send restart after start response */
	case WLAN_VDEV_SM_EV_START_RESP:
		/* send vdev restart req command to FW */
		mlme_vdev_restart_send(vdev_mlme, event_data_len, event_data);
		status = true;
		break;
	case WLAN_VDEV_SM_EV_RESTART_RESP:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_START_CONN_PROGRESS);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_CONN_PROGRESS,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_RESTART_REQ_FAIL:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_SUSPEND);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DOWN:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_START_DISCONN_PROGRESS);
		/* block restart request, if it is pending */
		mlme_vdev_stop_start_send(vdev_mlme, RESTART_REQ,
					  event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_RADAR_DETECTED:
		/* It is complicated to handle RADAR detected in this substate,
		 * as vdev updates des channels as bss channel on response,
		 * it would be easily handled, if it is deferred by DFS module
		 */
		QDF_BUG(0);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_subst_start_conn_progress_entry() - Entry API for Conn. Progress
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to CONN-PROGRESS substate
 *
 * Return: void
 */
static void mlme_vdev_subst_start_conn_progress_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_START)
		QDF_BUG(0);

	mlme_vdev_set_substate(vdev, WLAN_VDEV_SS_START_CONN_PROGRESS);
}

/**
 * mlme_vdev_subst_start_conn_progress_exit() - Exit API for Conn. Progress
 *                                              sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of CONN-PROGRESS substate
 *
 * Return: void
 */
static void mlme_vdev_subst_start_conn_progress_exit(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;

	mlme_vdev_notify_start_state_exit(vdev_mlme);
}

/**
 * mlme_vdev_subst_start_conn_progress_event() - Event handler API for Conn.
 *                                                Progress substate
 * @ctx: VDEV MLME object
 *
 * API to handle events in CONN-PROGRESS substate
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_subst_start_conn_progress_event(void *ctx,
						      uint16_t event,
						      uint16_t event_data_len,
						      void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	enum QDF_OPMODE mode;
	struct wlan_objmgr_vdev *vdev;
	bool status;

	vdev = vdev_mlme->vdev;

	mode = wlan_vdev_mlme_get_opmode(vdev);

	switch (event) {
	case WLAN_VDEV_SM_EV_CONN_PROGRESS:
		/* This API decides to move to DFS CAC WAIT or UP state,
		 * for station notify connection state machine */
		if (mlme_vdev_start_continue(vdev_mlme, event_data_len,
					     event_data) != QDF_STATUS_SUCCESS)
			mlme_vdev_sm_deliver_event(
					vdev_mlme,
					WLAN_VDEV_SM_EV_CONNECTION_FAIL,
					event_data_len, event_data);
		else
			mlme_vdev_start_rsp_notify_mlo_mgr(vdev_mlme);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DFS_CAC_WAIT:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_DFS_CAC_WAIT);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_START_SUCCESS:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_UP);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_STA_CONN_START:
		/* This event triggers station connection, if it is blocked for
		 * CAC WAIT
		 */
		if (mode != QDF_STA_MODE)
			QDF_BUG(0);

		mlme_vdev_sta_conn_start(vdev_mlme, event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_RADAR_DETECTED:
		if (mode != QDF_STA_MODE)
			QDF_BUG(0);

		status = true;
		break;

	case WLAN_VDEV_SM_EV_DOWN:
	case WLAN_VDEV_SM_EV_CONNECTION_FAIL:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_START_DISCONN_PROGRESS);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_subst_start_disconn_progress_entry() - Entry API for Disconn
 *                                                  progress sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to DISCONN-PROGRESS substate
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static void mlme_vdev_subst_start_disconn_progress_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_START)
		QDF_BUG(0);

	mlme_vdev_set_substate(vdev, WLAN_VDEV_SS_START_DISCONN_PROGRESS);
}

/**
 * mlme_vdev_subst_start_disconn_progress_exit() - Exit API for Disconn Progress
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of DISCONN-PROGRESS substate
 *
 * Return: void
 */
static void mlme_vdev_subst_start_disconn_progress_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_subst_start_disconn_progress_event() - Event handler API for Discon
 *                                                Progress substate
 * @ctx: VDEV MLME object
 *
 * API to handle events in DISCONN-PROGRESS substate
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_subst_start_disconn_progress_event(void *ctx,
		uint16_t event, uint16_t event_data_len, void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	bool status;

	switch (event) {
	case WLAN_VDEV_SM_EV_START_RESP:
	/* clean up, if any needs to be cleaned up */
	case WLAN_VDEV_SM_EV_CONNECTION_FAIL:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_STOP);
		mlme_vdev_sm_deliver_event(vdev_mlme, WLAN_VDEV_SM_EV_STOP_REQ,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_RESTART_RESP:
	case WLAN_VDEV_SM_EV_RESTART_REQ_FAIL:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_SUSPEND);
		mlme_vdev_sm_deliver_event(vdev_mlme, WLAN_VDEV_SM_EV_DOWN,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_START_REQ_FAIL:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_INIT);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DOWN:
		mlme_vdev_sta_disconn_start(vdev_mlme, event_data_len,
					    event_data);
		status = true;
		break;
	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_subst_suspend_suspend_down_entry() - Entry API for Suspend down
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to SUSPEND-DOWN substate
 *
 * Return: void
 */
static void mlme_vdev_subst_suspend_suspend_down_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_SUSPEND)
		QDF_BUG(0);

	mlme_vdev_set_substate(vdev, WLAN_VDEV_SS_SUSPEND_SUSPEND_DOWN);
}

/**
 * mlme_vdev_subst_suspend_suspend_down_exit() - Exit API for Suspend down
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of SUSPEND-DOWN substate
 *
 * Return: void
 */
static void mlme_vdev_subst_suspend_suspend_down_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_subst_suspend_suspend_down_event() - Event handler API for Suspend
 *                                                down substate
 * @ctx: VDEV MLME object
 *
 * API to handle events in SUSPEND-DOWN substate
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_subst_suspend_suspend_down_event(void *ctx,
		uint16_t event, uint16_t event_data_len, void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	bool status;

	switch (event) {
	case WLAN_VDEV_SM_EV_DOWN:
	case WLAN_VDEV_SM_EV_RESTART_REQ_FAIL:
		mlme_vdev_disconnect_peers(vdev_mlme,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DISCONNECT_COMPLETE:
		/* clean up, if any needs to be cleaned up */
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_STOP);
		mlme_vdev_sm_deliver_event(vdev_mlme, WLAN_VDEV_SM_EV_STOP_REQ,
					   event_data_len, event_data);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_subst_suspend_suspend_restart_entry() - Entry API for Suspend
 *                                                   restart substate
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to SUSPEND-RESTART substate
 *
 * Return: void
 */
static void mlme_vdev_subst_suspend_suspend_restart_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_SUSPEND)
		QDF_BUG(0);

	mlme_vdev_set_substate(vdev, WLAN_VDEV_SS_SUSPEND_SUSPEND_RESTART);
}

/**
 * mlme_vdev_subst_suspend_suspend_restart_exit() - Exit API for Suspend restart
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of SUSPEND-RESTART substate
 *
 * Return: void
 */
static void mlme_vdev_subst_suspend_suspend_restart_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_subst_suspend_suspend_restart_event() - Event handler API for
 *                                                   Suspend restart substate
 * @ctx: VDEV MLME object
 *
 * API to handle events in SUSPEND-RESTART substate
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_subst_suspend_suspend_restart_event(void *ctx,
		uint16_t event, uint16_t event_data_len, void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	bool status;

	switch (event) {
	case WLAN_VDEV_SM_EV_SUSPEND_RESTART:
		mlme_vdev_disconnect_peers(vdev_mlme,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DISCONNECT_COMPLETE:
		/* clean up, if any needs to be cleaned up */
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_START);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_RESTART_REQ,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DOWN:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_SUSPEND_SUSPEND_DOWN);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_RADAR_DETECTED:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_SUSPEND_CSA_RESTART);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_CSA_RESTART,
					   event_data_len, event_data);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_subst_suspend_host_restart_entry() - Entry API for Host restart
 *                                                substate
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to HOST-RESTART substate
 *
 * Return: void
 */
static void mlme_vdev_subst_suspend_host_restart_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_SUSPEND)
		QDF_BUG(0);

	mlme_vdev_set_substate(vdev, WLAN_VDEV_SS_SUSPEND_HOST_RESTART);
}

/**
 * mlme_vdev_subst_suspend_host_restart_exit() - Exit API for host restart
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of HOST-RESTART substate
 *
 * Return: void
 */
static void mlme_vdev_subst_suspend_host_restart_exit(void *ctx)
{
    /* NONE */
}

/**
 * mlme_vdev_subst_suspend_host_restart_entry() - Event handler API for Host
 *                                                restart substate
 * @ctx: VDEV MLME object
 *
 * API to handle events in HOST-RESTART substate
 *
 * Return: void
 */
static bool mlme_vdev_subst_suspend_host_restart_event(void *ctx,
		uint16_t event, uint16_t event_data_len, void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	bool status;

	switch (event) {
	case WLAN_VDEV_SM_EV_HOST_RESTART:
		mlme_vdev_disconnect_peers(vdev_mlme,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DISCONNECT_COMPLETE:
		/* VDEV up command need not be sent */
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_UP);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_UP_HOST_RESTART,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DOWN:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_SUSPEND_SUSPEND_DOWN);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_RADAR_DETECTED:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_SUSPEND_CSA_RESTART);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_CSA_RESTART,
					   event_data_len, event_data);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_subst_suspend_csa_restart_entry() - Entry API for CSA restart
 *                                               substate
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to CSA-RESTART substate
 *
 * Return: void
 */
static void mlme_vdev_subst_suspend_csa_restart_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_SUSPEND)
		QDF_BUG(0);

	mlme_vdev_set_substate(vdev, WLAN_VDEV_SS_SUSPEND_CSA_RESTART);
}

/**
 * mlme_vdev_subst_suspend_csa_restart_exit() - Exit API for CSA restart
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of CSA-RESTART substate
 *
 * Return: void
 */
static void mlme_vdev_subst_suspend_csa_restart_exit(void *ctx)
{
    /* NONE */
}

/**
 * mlme_vdev_subst_suspend_csa_restart_event() - Event handler API for CSA
 *                                               restart substate
 * @ctx: VDEV MLME object
 *
 * API to handle events in CSA-RESTART substate
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_subst_suspend_csa_restart_event(void *ctx,
		uint16_t event, uint16_t event_data_len, void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	bool status;

	switch (event) {
	case WLAN_VDEV_SM_EV_CHAN_SWITCH_DISABLED:
	/**
	 * This event is sent when CSA count becomes 0 without
	 * change in channel i.e. only Beacon Probe response template
	 * is updated (CSA / ECSA IE is removed).
	 */
		mlme_vdev_chan_switch_disable_notify_dfs(vdev_mlme);
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_UP);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_UP_HOST_RESTART,
					   event_data_len, event_data);
		status = true;
		break;
	case WLAN_VDEV_SM_EV_CSA_RESTART:
		mlme_vdev_update_beacon(vdev_mlme, BEACON_CSA,
					event_data_len, event_data);
		status = true;
		break;
	case WLAN_VDEV_SM_EV_CSA_COMPLETE:
		if ((mlme_vdev_is_newchan_no_cac(vdev_mlme) ==
		    QDF_STATUS_SUCCESS) ||
		    mlme_max_chan_switch_is_set(vdev_mlme->vdev)) {
			mlme_vdev_sm_transition_to(vdev_mlme,
						   WLAN_VDEV_S_START);
			mlme_vdev_sm_deliver_event(vdev_mlme,
						   WLAN_VDEV_SM_EV_RESTART_REQ,
						   event_data_len, event_data);
		} else  {
			mlme_vdev_sm_transition_to
				(vdev_mlme,
				 WLAN_VDEV_SS_SUSPEND_SUSPEND_RESTART);
			mlme_vdev_sm_deliver_event
				(vdev_mlme, WLAN_VDEV_SM_EV_SUSPEND_RESTART,
				 event_data_len, event_data);
		}
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DOWN:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_SUSPEND_SUSPEND_DOWN);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_RADAR_DETECTED:
		/* since channel change is already in progress,
		 * dfs ignore radar detected event
		 */
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_subst_stop_stop_progress_entry() - Entry API for Stop Progress
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to STOP-PROGRESS substate
 *
 * Return: void
 */
static void mlme_vdev_subst_stop_stop_progress_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *) ctx;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_STOP)
		QDF_BUG(0);

	mlme_vdev_set_substate(vdev, WLAN_VDEV_SS_STOP_STOP_PROGRESS);
}

/**
 * mlme_vdev_subst_stop_stop_progress_exit() - Exit API for Stop Progress
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of STOP-PROGRESS substate
 *
 * Return: void
 */
static void mlme_vdev_subst_stop_stop_progress_exit(void *ctx)
{
    /* NONE */
}

/**
 * mlme_vdev_subst_stop_stop_progress_event() - Event handler API for Stop
 *                                                Progress substate
 * @ctx: VDEV MLME object
 *
 * API to handle events in STOP-PROGRESS substate
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_subst_stop_stop_progress_event(void *ctx,
		uint16_t event, uint16_t event_data_len, void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	bool status;

	/* Debug framework is required to hold the events */

	switch (event) {
	case WLAN_VDEV_SM_EV_STOP_REQ:
		/* send vdev stop command to FW and delete BSS peer*/
		mlme_vdev_stop_send(vdev_mlme, event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_STOP_RESP:
		/* Processes stop response, and checks BSS peer delete wait
		 * is needed
		 */
		mlme_vdev_stop_continue(vdev_mlme, event_data_len, event_data);
		status = true;
		break;

	/* This event should be given by MLME on stop complete and BSS
	 * peer delete complete to move forward
	 */
	case WLAN_VDEV_SM_EV_MLME_DOWN_REQ:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_STOP_DOWN_PROGRESS);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_MLME_DOWN_REQ,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_STOP_FAIL:
		mlme_vdev_sm_transition_to(vdev_mlme,
					   WLAN_VDEV_SS_STOP_DOWN_PROGRESS);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_MLME_DOWN_REQ,
					   event_data_len, event_data);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_subst_stop_down_progress_entry() - Entry API for Down Progress
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to DOWN-PROGRESS substate
 *
 * Return: void
 */
static void mlme_vdev_subst_stop_down_progress_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_STOP)
		QDF_BUG(0);

	mlme_vdev_set_substate(vdev, WLAN_VDEV_SS_STOP_DOWN_PROGRESS);
}

/**
 * mlme_vdev_subst_stop_down_progress_exit() - Exit API for Down Progress
 *                                                sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of DOWN-PROGRESS substate
 *
 * Return: void
 */
static void mlme_vdev_subst_stop_down_progress_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_subst_stop_down_progress_event() - Event handler API for Down
 *                                                Progress substate
 * @ctx: VDEV MLME object
 *
 * API to handle events in DOWN-PROGRESS substate
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_subst_stop_down_progress_event(void *ctx,
		uint16_t event, uint16_t event_data_len, void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	bool status;

	switch (event) {
	case WLAN_VDEV_SM_EV_DOWN:
		status = true;
		break;

	case WLAN_VDEV_SM_EV_MLME_DOWN_REQ:
		/* send vdev down command to FW, if send is successful, sends
		 * DOWN_COMPLETE event
		 */
		mlme_vdev_down_send(vdev_mlme, event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DOWN_COMPLETE:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_INIT);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_DOWN_COMPLETE,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DOWN_FAIL:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_INIT);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_DOWN_COMPLETE,
					   event_data_len, event_data);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_subst_mlo_sync_wait_entry() - Entry API for mlo sync wait sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to MLO-SYNC-WAIT substate
 *
 * Return: void
 */
static void mlme_vdev_subst_mlo_sync_wait_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_UP)
		QDF_BUG(0);

	mlme_vdev_set_substate(vdev, WLAN_VDEV_SS_MLO_SYNC_WAIT);
}

/**
 * mlme_vdev_subst_mlo_sync_wait_exit() - Exit API for mlo sync wait sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of MLO-SYNC-WAIT substate
 *
 * Return: void
 */
static void mlme_vdev_subst_mlo_sync_wait_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_subst_mlo_sync_wait_event() - Event handler API for mlo sync wait
 *                                         substate
 * @ctx: VDEV MLME object
 *
 * API to handle events in MLO-SYNC-WAIT substate
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_subst_mlo_sync_wait_event(void *ctx, uint16_t event,
						uint16_t event_data_len,
						void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	bool status;

	switch (event) {
	case WLAN_VDEV_SM_EV_START_SUCCESS:
		mlme_vdev_up_notify_mlo_mgr(vdev_mlme);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_MLO_SYNC_COMPLETE:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_SS_UP_ACTIVE);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_DOWN:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_STOP);
		mlme_vdev_sm_deliver_event(vdev_mlme, WLAN_VDEV_SM_EV_STOP_REQ,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_RADAR_DETECTED:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_START);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_RESTART_REQ,
					   event_data_len, event_data);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}

/**
 * mlme_vdev_subst_up_active_entry() - Entry API for up active sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving to UP-ACTIVE substate
 *
 * Return: void
 */
static void mlme_vdev_subst_up_active_entry(void *ctx)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (wlan_vdev_mlme_get_state(vdev) != WLAN_VDEV_S_UP)
		QDF_BUG(0);

	mlme_vdev_set_substate(vdev, WLAN_VDEV_SS_UP_ACTIVE);
}

/**
 * mlme_vdev_subst_up_active_exit() - Exit API for up active sub state
 * @ctx: VDEV MLME object
 *
 * API to perform operations on moving out of UP-ACTIVE substate
 *
 * Return: void
 */
static void mlme_vdev_subst_up_active_exit(void *ctx)
{
	/* NONE */
}

/**
 * mlme_vdev_subst_up_active_event() - Event handler API for up active substate
 * @ctx: VDEV MLME object
 *
 * API to handle events in UP-ACTIVE substate
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
static bool mlme_vdev_subst_up_active_event(void *ctx, uint16_t event,
					    uint16_t event_data_len,
					    void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)ctx;
	enum QDF_OPMODE mode;
	struct wlan_objmgr_vdev *vdev;
	bool status;

	vdev = vdev_mlme->vdev;
	mode = wlan_vdev_mlme_get_opmode(vdev);

	switch (event) {
	case WLAN_VDEV_SM_EV_START_SUCCESS:
		if (wlan_vdev_mlme_is_mlo_ap(vdev))
			QDF_BUG(0);
		/* fallthrough */
	case WLAN_VDEV_SM_EV_MLO_SYNC_COMPLETE:
		mlme_vdev_update_beacon(vdev_mlme, BEACON_INIT,
					event_data_len, event_data);
		if (mlme_vdev_up_send(vdev_mlme, event_data_len,
				      event_data) != QDF_STATUS_SUCCESS)
			mlme_vdev_sm_deliver_event(vdev_mlme,
						   WLAN_VDEV_SM_EV_UP_FAIL,
						   event_data_len, event_data);
		else
			mlme_vdev_notify_up_complete(vdev_mlme, event_data_len,
						     event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_SUSPEND_RESTART:
	case WLAN_VDEV_SM_EV_HOST_RESTART:
	case WLAN_VDEV_SM_EV_CSA_RESTART:
		/* These events are not supported in STA mode */
		if (mode == QDF_STA_MODE)
			QDF_BUG(0);
		/* fallthrough */
	case WLAN_VDEV_SM_EV_DOWN:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_SUSPEND);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_RADAR_DETECTED:
		/* These events are not supported in STA mode */
		if (mode == QDF_STA_MODE)
			QDF_BUG(0);
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_SUSPEND);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_CSA_RESTART,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_UP_HOST_RESTART:
		/* Reinit beacon, send template to FW(use ping-pong buffer) */
		mlme_vdev_update_beacon(vdev_mlme, BEACON_UPDATE,
					event_data_len, event_data);
		/* fallthrough */
	case WLAN_VDEV_SM_EV_START:
		/* notify that UP command is completed */
		mlme_vdev_notify_up_complete(vdev_mlme,
					     event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_FW_VDEV_RESTART:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_START);
		mlme_vdev_sm_deliver_event(vdev_mlme,
					   WLAN_VDEV_SM_EV_RESTART_REQ,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_UP_FAIL:
		mlme_vdev_sm_transition_to(vdev_mlme, WLAN_VDEV_S_SUSPEND);
		mlme_vdev_sm_deliver_event(vdev_mlme, event,
					   event_data_len, event_data);
		status = true;
		break;

	case WLAN_VDEV_SM_EV_ROAM:
		mlme_vdev_notify_roam_start(vdev_mlme, event_data_len,
					    event_data);
		status = true;
		break;

	default:
		status = false;
		break;
	}

	return status;
}


static const char *vdev_sm_event_names[] = {
	"EV_START",
	"EV_START_REQ",
	"EV_RESTART_REQ",
	"EV_START_RESP",
	"EV_RESTART_RESP",
	"EV_START_REQ_FAIL",
	"EV_RESTART_REQ_FAIL",
	"EV_START_SUCCESS",
	"EV_CONN_PROGRESS",
	"EV_STA_CONN_START",
	"EV_DFS_CAC_WAIT",
	"EV_DFS_CAC_COMPLETED",
	"EV_DOWN",
	"EV_CONNECTION_FAIL",
	"EV_STOP_RESP",
	"EV_STOP_FAIL",
	"EV_DOWN_FAIL",
	"EV_DISCONNECT_COMPLETE",
	"EV_SUSPEND_RESTART",
	"EV_HOST_RESTART",
	"EV_UP_HOST_RESTART",
	"EV_FW_VDEV_RESTART",
	"EV_UP_FAIL",
	"EV_RADAR_DETECTED",
	"EV_CSA_RESTART",
	"EV_CSA_COMPLETE",
	"EV_MLME_DOWN_REQ",
	"EV_DOWN_COMPLETE",
	"EV_ROAM",
	"EV_STOP_REQ",
	"EV_CHAN_SWITCH_DISABLED",
	"EV_MLO_SYNC_COMPLETE",
};

struct wlan_sm_state_info sm_info[] = {
	{
		(uint8_t)WLAN_VDEV_S_INIT,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		true,
		"INIT",
		mlme_vdev_state_init_entry,
		mlme_vdev_state_init_exit,
		mlme_vdev_state_init_event
	},
	{
		(uint8_t)WLAN_VDEV_S_START,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		true,
		"START",
		mlme_vdev_state_start_entry,
		mlme_vdev_state_start_exit,
		mlme_vdev_state_start_event
	},
	{
		(uint8_t)WLAN_VDEV_S_DFS_CAC_WAIT,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		true,
		"DFS_CAC_WAIT",
		mlme_vdev_state_dfs_cac_wait_entry,
		mlme_vdev_state_dfs_cac_wait_exit,
		mlme_vdev_state_dfs_cac_wait_event
	},
	{
		(uint8_t)WLAN_VDEV_S_UP,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		true,
		"UP",
		mlme_vdev_state_up_entry,
		mlme_vdev_state_up_exit,
		mlme_vdev_state_up_event
	},
	{
		(uint8_t)WLAN_VDEV_S_SUSPEND,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		true,
		"SUSPEND",
		mlme_vdev_state_suspend_entry,
		mlme_vdev_state_suspend_exit,
		mlme_vdev_state_suspend_event
	},
	{
		(uint8_t)WLAN_VDEV_S_STOP,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_VDEV_SS_STOP_STOP_PROGRESS,
		true,
		"STOP",
		mlme_vdev_state_stop_entry,
		mlme_vdev_state_stop_exit,
		mlme_vdev_state_stop_event
	},
	{
		(uint8_t)WLAN_VDEV_S_MAX,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"INVALID",
		NULL,
		NULL,
		NULL
	},
	{
		(uint8_t)WLAN_VDEV_SS_START_START_PROGRESS,
		(uint8_t)WLAN_VDEV_S_START,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"ST-START_PROG",
		mlme_vdev_subst_start_start_progress_entry,
		mlme_vdev_subst_start_start_progress_exit,
		mlme_vdev_subst_start_start_progress_event
	},
	{
		(uint8_t)WLAN_VDEV_SS_START_RESTART_PROGRESS,
		(uint8_t)WLAN_VDEV_S_START,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"ST-RESTART_PROG",
		mlme_vdev_subst_start_restart_progress_entry,
		mlme_vdev_subst_start_restart_progress_exit,
		mlme_vdev_subst_start_restart_progress_event
	},
	{
		(uint8_t)WLAN_VDEV_SS_START_CONN_PROGRESS,
		(uint8_t)WLAN_VDEV_S_START,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"ST-CONN_PROG",
		mlme_vdev_subst_start_conn_progress_entry,
		mlme_vdev_subst_start_conn_progress_exit,
		mlme_vdev_subst_start_conn_progress_event
	},
	{
		(uint8_t)WLAN_VDEV_SS_START_DISCONN_PROGRESS,
		(uint8_t)WLAN_VDEV_S_START,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"ST-DISCONN_PROG",
		mlme_vdev_subst_start_disconn_progress_entry,
		mlme_vdev_subst_start_disconn_progress_exit,
		mlme_vdev_subst_start_disconn_progress_event
	},
	{
		(uint8_t)WLAN_VDEV_SS_SUSPEND_SUSPEND_DOWN,
		(uint8_t)WLAN_VDEV_S_SUSPEND,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"SP-SUSPEND_DOWN",
		mlme_vdev_subst_suspend_suspend_down_entry,
		mlme_vdev_subst_suspend_suspend_down_exit,
		mlme_vdev_subst_suspend_suspend_down_event
	},
	{
		(uint8_t)WLAN_VDEV_SS_SUSPEND_SUSPEND_RESTART,
		(uint8_t)WLAN_VDEV_S_SUSPEND,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"SP-SUSPEND_RESTART",
		mlme_vdev_subst_suspend_suspend_restart_entry,
		mlme_vdev_subst_suspend_suspend_restart_exit,
		mlme_vdev_subst_suspend_suspend_restart_event
	},
	{
		(uint8_t)WLAN_VDEV_SS_SUSPEND_HOST_RESTART,
		(uint8_t)WLAN_VDEV_S_SUSPEND,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"SP-HOST_RESTART",
		mlme_vdev_subst_suspend_host_restart_entry,
		mlme_vdev_subst_suspend_host_restart_exit,
		mlme_vdev_subst_suspend_host_restart_event
	},
	{
		(uint8_t)WLAN_VDEV_SS_SUSPEND_CSA_RESTART,
		(uint8_t)WLAN_VDEV_S_SUSPEND,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"SP-CSA_RESTART",
		mlme_vdev_subst_suspend_csa_restart_entry,
		mlme_vdev_subst_suspend_csa_restart_exit,
		mlme_vdev_subst_suspend_csa_restart_event
	},
	{
		(uint8_t)WLAN_VDEV_SS_STOP_STOP_PROGRESS,
		(uint8_t)WLAN_VDEV_S_STOP,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"STOP-STOP_PROG",
		mlme_vdev_subst_stop_stop_progress_entry,
		mlme_vdev_subst_stop_stop_progress_exit,
		mlme_vdev_subst_stop_stop_progress_event
	},
	{
		(uint8_t)WLAN_VDEV_SS_STOP_DOWN_PROGRESS,
		(uint8_t)WLAN_VDEV_S_STOP,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"STOP-DOWN_PROG",
		mlme_vdev_subst_stop_down_progress_entry,
		mlme_vdev_subst_stop_down_progress_exit,
		mlme_vdev_subst_stop_down_progress_event
	},
	{
		(uint8_t)WLAN_VDEV_SS_IDLE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"IDLE",
		NULL,
		NULL,
		NULL,
	},
	{
		(uint8_t)WLAN_VDEV_SS_MLO_SYNC_WAIT,
		(uint8_t)WLAN_VDEV_S_UP,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"UP-MLO-SYNC-WAIT",
		mlme_vdev_subst_mlo_sync_wait_entry,
		mlme_vdev_subst_mlo_sync_wait_exit,
		mlme_vdev_subst_mlo_sync_wait_event
	},
	{
		(uint8_t)WLAN_VDEV_SS_UP_ACTIVE,
		(uint8_t)WLAN_VDEV_S_UP,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"UP-UP-ACTIVE",
		mlme_vdev_subst_up_active_entry,
		mlme_vdev_subst_up_active_exit,
		mlme_vdev_subst_up_active_event
	},
	{
		(uint8_t)WLAN_VDEV_SS_MAX,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"INVALID",
		NULL,
		NULL,
		NULL,
	},
};

QDF_STATUS mlme_vdev_sm_deliver_event(struct vdev_mlme_obj *vdev_mlme,
				      enum wlan_vdev_sm_evt event,
				      uint16_t event_data_len, void *event_data)
{
	return wlan_sm_dispatch(vdev_mlme->sm_hdl, event,
				event_data_len, event_data);
}

void mlme_vdev_sm_print_state_event(struct vdev_mlme_obj *vdev_mlme,
				    enum wlan_vdev_sm_evt event)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);

	mlme_nofl_debug("[%s]%s - %s, %s", vdev_mlme->sm_hdl->name,
			sm_info[state].name, sm_info[substate].name,
			vdev_sm_event_names[event]);
}

void mlme_vdev_sm_print_state(struct vdev_mlme_obj *vdev_mlme)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);

	mlme_nofl_debug("[%s]%s - %s", vdev_mlme->sm_hdl->name,
			sm_info[state].name, sm_info[substate].name);
}

#ifdef SM_ENG_HIST_ENABLE
void mlme_vdev_sm_history_print(struct vdev_mlme_obj *vdev_mlme)
{
	return wlan_sm_print_history(vdev_mlme->sm_hdl);
}
#endif

QDF_STATUS mlme_vdev_sm_create(struct vdev_mlme_obj *vdev_mlme)
{
	struct wlan_sm *sm;
	uint8_t name[WLAN_SM_ENGINE_MAX_NAME];
	struct wlan_objmgr_vdev *vdev = vdev_mlme->vdev;

	qdf_scnprintf(name, sizeof(name), "VDEV%d-MLME",
		      wlan_vdev_get_id(vdev_mlme->vdev));
	sm = wlan_sm_create(name, vdev_mlme,
			    WLAN_VDEV_S_INIT,
			    sm_info,
			    QDF_ARRAY_SIZE(sm_info),
			    vdev_sm_event_names,
			    QDF_ARRAY_SIZE(vdev_sm_event_names));
	if (!sm) {
		mlme_err("VDEV MLME SM allocation failed");
		return QDF_STATUS_E_FAILURE;
	}
	vdev_mlme->sm_hdl = sm;
	wlan_minidump_log((void *)sm, sizeof(*sm),
			  wlan_vdev_get_psoc(vdev),
			  WLAN_MD_OBJMGR_VDEV_SM, "wlan_sm");

	mlme_vdev_sm_spinlock_create(vdev_mlme);

	mlme_vdev_cmd_mutex_create(vdev_mlme);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlme_vdev_sm_destroy(struct vdev_mlme_obj *vdev_mlme)
{
	struct wlan_objmgr_vdev *vdev = vdev_mlme->vdev;

	mlme_vdev_cmd_mutex_destroy(vdev_mlme);

	mlme_vdev_sm_spinlock_destroy(vdev_mlme);

	wlan_minidump_remove(vdev_mlme->sm_hdl,
			     sizeof(*vdev_mlme->sm_hdl),
			     wlan_vdev_get_psoc(vdev),
			     WLAN_MD_OBJMGR_VDEV_SM, "wlan_sm");

	wlan_sm_delete(vdev_mlme->sm_hdl);

	return QDF_STATUS_SUCCESS;
}
