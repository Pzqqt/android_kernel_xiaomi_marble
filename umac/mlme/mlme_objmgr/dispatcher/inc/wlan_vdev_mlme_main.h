/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: Define VDEV MLME init/deinit APIs
 */

#ifndef _WLAN_VDEV_MLME_MAIN_H_
#define _WLAN_VDEV_MLME_MAIN_H_

#ifdef CMN_VDEV_MLME_SM_ENABLE
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
 * @WLAN_VDEV_SM_EV_SUPSEND_RESTART:     Invokes suspend restart operation
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
 */
enum wlan_vdev_sm_evt {
	WLAN_VDEV_SM_EV_START = 0,
	WLAN_VDEV_SM_EV_START_REQ,
	WLAN_VDEV_SM_EV_RESTART_REQ,
	WLAN_VDEV_SM_EV_START_RESP,
	WLAN_VDEV_SM_EV_RESTART_RESP,
	WLAN_VDEV_SM_EV_START_REQ_FAIL,
	WLAN_VDEV_SM_EV_RESTART_REQ_FAIL,
	WLAN_VDEV_SM_EV_START_SUCCESS,
	WLAN_VDEV_SM_EV_CONN_PROGRESS,
	WLAN_VDEV_SM_EV_STA_CONN_START,
	WLAN_VDEV_SM_EV_DFS_CAC_WAIT = 10,
	WLAN_VDEV_SM_EV_DFS_CAC_COMPLETED,
	WLAN_VDEV_SM_EV_DOWN,
	WLAN_VDEV_SM_EV_CONNECTION_FAIL,
	WLAN_VDEV_SM_EV_STOP_RESP,
	WLAN_VDEV_SM_EV_STOP_FAIL,
	WLAN_VDEV_SM_EV_DOWN_FAIL,
	WLAN_VDEV_SM_EV_DISCONNECT_COMPLETE,
	WLAN_VDEV_SM_EV_SUPSEND_RESTART,
	WLAN_VDEV_SM_EV_HOST_RESTART,
	WLAN_VDEV_SM_EV_UP_HOST_RESTART = 20,
	WLAN_VDEV_SM_EV_FW_VDEV_RESTART,
	WLAN_VDEV_SM_EV_UP_FAIL,
	WLAN_VDEV_SM_EV_RADAR_DETECTED,
	WLAN_VDEV_SM_EV_CSA_RESTART,
	WLAN_VDEV_SM_EV_CSA_COMPLETE,
	WLAN_VDEV_SM_EV_MLME_DOWN_REQ,
	WLAN_VDEV_SM_EV_DOWN_COMPLETE,
};

/**
 * wlan_vdev_mlme_init - Initializes MLME component
 *
 * Registers callbacks with object manager for create/destroy
 *
 * Return: SUCCESS on successful registration
 *         FAILURE, if registration fails
 */
QDF_STATUS wlan_vdev_mlme_init(void);

/**
 * wlan_vdev_mlme_deinit - Uninitializes MLME component
 *
 * Unregisters callbacks with object manager for create/destroy
 *
 * Return: SUCCESS on successful registration
 *         FAILURE, if registration fails
 */
QDF_STATUS wlan_vdev_mlme_deinit(void);

#else

/**
 * enum wlan_vdev_state - VDEV state
 * @WLAN_VDEV_S_INIT:    Default state, IDLE state
 * @WLAN_VDEV_S_SCAN:    SCAN state
 * @WLAN_VDEV_S_JOIN:    Join state
 * @WLAN_VDEV_S_DFS_WAIT:CAC period
 * @WLAN_VDEV_S_RUN:     RUN state
 * @WLAN_VDEV_S_STOP:    STOP state
 * @WLAN_VDEV_S_RESET:   RESET state, STOP+INIT+JOIN
 * @WLAN_VDEV_S_MAX:     MAX state
 */
enum wlan_vdev_state {
	WLAN_VDEV_S_INIT     = 0,
	WLAN_VDEV_S_SCAN     = 1,
	WLAN_VDEV_S_JOIN     = 2,
	WLAN_VDEV_S_DFS_WAIT = 3,
	WLAN_VDEV_S_RUN      = 4,
	WLAN_VDEV_S_STOP     = 5,
	WLAN_VDEV_S_RESET    = 6,
	WLAN_VDEV_S_MAX,
};

/**
 * wlan_vdev_mlme_init - Initializes MLME component
 *
 * Registers callbacks with object manager for create/destroy
 *
 * Return: SUCCESS on successful registration
 *         FAILURE, if registration fails
 */
static inline QDF_STATUS wlan_vdev_mlme_init(void)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_vdev_mlme_deinit - Uninitializes MLME component
 *
 * Unregisters callbacks with object manager for create/destroy
 *
 * Return: SUCCESS on successful registration
 *         FAILURE, if registration fails
 */
static inline QDF_STATUS wlan_vdev_mlme_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif
