/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: Define VDEV MLME init/deinit APIs
 */

#ifndef _WLAN_VDEV_MLME_MAIN_H_
#define _WLAN_VDEV_MLME_MAIN_H_

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_lmac_if_def.h>

/**
 * wlan_mlme_get_lmac_tx_ops() - get tx ops
 * @psoc: pointer to psoc obj
 *
 * Return: pointer to tx ops
 */
static inline struct wlan_lmac_if_mlme_tx_ops *
wlan_mlme_get_lmac_tx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.tx_ops.mops;
}

/**
 * enum wlan_vdev_state - VDEV state
 * @WLAN_VDEV_S_INIT:                     Default state, IDLE state
 * @WLAN_VDEV_S_START:                    START  state
 * @WLAN_VDEV_S_DFS_CAC_WAIT:             CAC period
 * @WLAN_VDEV_S_UP:                       UP state
 * @WLAN_VDEV_S_SUSPEND:                  Suspend state
 * @WLAN_VDEV_S_STOP:                     STOP state
 * @WLAN_VDEV_S_MAX:                      MAX state
 * @WLAN_VDEV_SS_START_START_PROGRESS:    Start progress sub state
 * @WLAN_VDEV_SS_START_RESTART_PROGRESS:  Restart progress sub state
 * @WLAN_VDEV_SS_START_CONN_PROGRESS:     Start connection progress sub state
 * @WLAN_VDEV_SS_START_DISCONN_PROGRESS:  Start Disconnection progress sub state
 * @WLAN_VDEV_SS_SUSPEND_SUSPEND_DOWN:    Suspend down sub state
 * @WLAN_VDEV_SS_SUSPEND_SUSPEND_RESTART: Suspend restart sub state
 * @WLAN_VDEV_SS_SUSPEND_HOST_RESTART:    Suspend host restart sub state
 * @WLAN_VDEV_SS_SUSPEND_CSA_RESTART:     Suspend CSA restart sub state
 * @WLAN_VDEV_SS_STOP_STOP_PROGRESS:      Stop progress sub state
 * @WLAN_VDEV_SS_STOP_DOWN_PROGRESS:      Stop down progress sub state
 * @WLAN_VDEV_SS_IDLE:                    Idle sub state (used, only if a state
 *                                        does not have substate)
 * @WLAN_VDEV_SS_MAX:                     Max substate
 */
enum wlan_vdev_state {
	WLAN_VDEV_S_INIT = 0,
	WLAN_VDEV_S_START = 1,
	WLAN_VDEV_S_DFS_CAC_WAIT = 2,
	WLAN_VDEV_S_UP = 3,
	WLAN_VDEV_S_SUSPEND = 4,
	WLAN_VDEV_S_STOP = 5,
	WLAN_VDEV_S_MAX = 6,
	WLAN_VDEV_SS_START_START_PROGRESS = 7,
	WLAN_VDEV_SS_START_RESTART_PROGRESS = 8,
	WLAN_VDEV_SS_START_CONN_PROGRESS = 9,
	WLAN_VDEV_SS_START_DISCONN_PROGRESS = 10,
	WLAN_VDEV_SS_SUSPEND_SUSPEND_DOWN = 11,
	WLAN_VDEV_SS_SUSPEND_SUSPEND_RESTART = 12,
	WLAN_VDEV_SS_SUSPEND_HOST_RESTART = 13,
	WLAN_VDEV_SS_SUSPEND_CSA_RESTART = 14,
	WLAN_VDEV_SS_STOP_STOP_PROGRESS = 15,
	WLAN_VDEV_SS_STOP_DOWN_PROGRESS = 16,
	WLAN_VDEV_SS_IDLE = 17,
	WLAN_VDEV_SS_MAX = 18,
};

/**
 * enum wlan_vdev_sm_evt - VDEV SM event
 * @WLAN_VDEV_SM_EV_START:               Start VDEV UP operation
 * @WLAN_VDEV_SM_EV_START_REQ:           Invokes VDEV START handshake
 * @WLAN_VDEV_SM_EV_RESTART_REQ:         Invokes VDEV RESTART handshake
 * @WLAN_VDEV_SM_EV_START_RESP:          Notification on START resp
 * @WLAN_VDEV_SM_EV_RESTART_RESP:        Notification on RESTART resp
 * @WLAN_VDEV_SM_EV_START_REQ_FAIL:      Notification on START req failure
 * @WLAN_VDEV_SM_EV_RESTART_REQ_FAIL:    Notification on RESTART req failure
 * @WLAN_VDEV_SM_EV_START_SUCCESS:       Notification of Join Success
 * @WLAN_VDEV_SM_EV_CONN_PROGRESS:       Invoke Connection/up process
 * @WLAN_VDEV_SM_EV_STA_CONN_START:      Invoke Station Connection process
 * @WLAN_VDEV_SM_EV_DFS_CAC_WAIT:        Invoke DFS CAC WAIT timer
 * @WLAN_VDEV_SM_EV_DFS_CAC_COMPLETED:   Notifies on CAC completion
 * @WLAN_VDEV_SM_EV_DOWN:                Invokes VDEV DOWN operation
 * @WLAN_VDEV_SM_EV_CONNECTION_FAIL:     Notifications for UP/connection failure
 * @WLAN_VDEV_SM_EV_STOP_RESP:           Notifcation of stop response
 * @WLAN_VDEV_SM_EV_STOP_FAIL:           Notification of stop req failure
 * @WLAN_VDEV_SM_EV_DOWN_FAIL:           Notification of down failure
 * @WLAN_VDEV_SM_EV_DISCONNECT_COMPLETE: Notification of Peer cleanup complete
 * @WLAN_VDEV_SM_EV_SUSPEND_RESTART:     Invokes suspend restart operation
 * @WLAN_VDEV_SM_EV_HOST_RESTART:        Invokes host only restart operation
 * @WLAN_VDEV_SM_EV_UP_HOST_RESTART:     Moves to UP state without sending UP
 *                                       command to lower layers
 * @WLAN_VDEV_SM_EV_FW_VDEV_RESTART:     Invokes FW only restart
 * @WLAN_VDEV_SM_EV_UP_FAIL:             Notification of up command failure
 * @WLAN_VDEV_SM_EV_RADAR_DETECTED:      Notification of RADAR detected, Random
 *                                       channel should be selected before
 *                                       triggering this event
 * @WLAN_VDEV_SM_EV_CSA_RESTART:         Invokes CSA IE operation
 * @WLAN_VDEV_SM_EV_CSA_COMPLETE:        Notifiction of CSA process complete
 * @WLAN_VDEV_SM_EV_MLME_DOWN_REQ:       Invoke DOWN command operation
 * @WLAN_VDEV_SM_EV_DOWN_COMPLETE:       Notification of DOWN complete
 * @WLAN_VDEV_SM_EV_ROAM:                Notifiction on ROAMING
 * @WLAN_VDEV_SM_EV_STOP_REQ:            Invoke API to initiate STOP handshake
 * @WLAN_VDEV_SM_EV_CHAN_SWITCH_DISABLED:Test only, CSA completes without
 *					 change in channel
 */
enum wlan_vdev_sm_evt {
	WLAN_VDEV_SM_EV_START = 0,
	WLAN_VDEV_SM_EV_START_REQ = 1,
	WLAN_VDEV_SM_EV_RESTART_REQ = 2,
	WLAN_VDEV_SM_EV_START_RESP = 3,
	WLAN_VDEV_SM_EV_RESTART_RESP = 4,
	WLAN_VDEV_SM_EV_START_REQ_FAIL = 5,
	WLAN_VDEV_SM_EV_RESTART_REQ_FAIL = 6,
	WLAN_VDEV_SM_EV_START_SUCCESS = 7,
	WLAN_VDEV_SM_EV_CONN_PROGRESS = 8,
	WLAN_VDEV_SM_EV_STA_CONN_START = 9,
	WLAN_VDEV_SM_EV_DFS_CAC_WAIT = 10,
	WLAN_VDEV_SM_EV_DFS_CAC_COMPLETED = 11,
	WLAN_VDEV_SM_EV_DOWN = 12,
	WLAN_VDEV_SM_EV_CONNECTION_FAIL = 13,
	WLAN_VDEV_SM_EV_STOP_RESP = 14,
	WLAN_VDEV_SM_EV_STOP_FAIL = 15,
	WLAN_VDEV_SM_EV_DOWN_FAIL = 16,
	WLAN_VDEV_SM_EV_DISCONNECT_COMPLETE = 17,
	WLAN_VDEV_SM_EV_SUSPEND_RESTART = 18,
	WLAN_VDEV_SM_EV_HOST_RESTART = 19,
	WLAN_VDEV_SM_EV_UP_HOST_RESTART = 20,
	WLAN_VDEV_SM_EV_FW_VDEV_RESTART = 21,
	WLAN_VDEV_SM_EV_UP_FAIL = 22,
	WLAN_VDEV_SM_EV_RADAR_DETECTED = 23,
	WLAN_VDEV_SM_EV_CSA_RESTART = 24,
	WLAN_VDEV_SM_EV_CSA_COMPLETE = 25,
	WLAN_VDEV_SM_EV_MLME_DOWN_REQ = 26,
	WLAN_VDEV_SM_EV_DOWN_COMPLETE = 27,
	WLAN_VDEV_SM_EV_ROAM = 28,
	WLAN_VDEV_SM_EV_STOP_REQ = 29,
	WLAN_VDEV_SM_EV_CHAN_SWITCH_DISABLED = 30,
};

/**
 * wlan_vdev_mlme_init - Initializes VDEV MLME component
 *
 * Registers callbacks with object manager for create/destroy
 *
 * Return: SUCCESS on successful registration
 *         FAILURE, if registration fails
 */
QDF_STATUS wlan_vdev_mlme_init(void);

/**
 * wlan_vdev_mlme_deinit - Uninitializes VDEV MLME component
 *
 * Unregisters callbacks with object manager for create/destroy
 *
 * Return: SUCCESS on successful registration
 *         FAILURE, if registration fails
 */
QDF_STATUS wlan_vdev_mlme_deinit(void);

/**
 * wlan_mlme_psoc_enable - MLME initializations on PSOC enable
 *
 * Initializes MLME params on PSOC eable
 *
 * Return: SUCCESS on successful initialization
 *         FAILURE, if initialization fails
 */
QDF_STATUS wlan_mlme_psoc_enable(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_mlme_psoc_disable - MLME clean up on PSOC disable
 *
 * cleanup MLME params on PSOC eable
 *
 * Return: SUCCESS on successful cleanup
 *         FAILURE, if cleanup fails
 */
QDF_STATUS wlan_mlme_psoc_disable(struct wlan_objmgr_psoc *psoc);
#endif
