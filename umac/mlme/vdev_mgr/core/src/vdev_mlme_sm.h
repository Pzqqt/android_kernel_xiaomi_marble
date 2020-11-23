/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
 * DOC: Declares VDEV MLME SM APIs and structures
 */

#ifndef _VDEV_MLME_SM_H_
#define _VDEV_MLME_SM_H_

/**
 * mlme_vdev_sm_deliver_event() - Delivers event to VDEV MLME SM
 * @vdev_mlme: MLME VDEV comp object
 * @event: MLME event
 * @event_data_len: data size
 * @event_data: event data
 *
 * API to dispatch event to VDEV MLME SM
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
QDF_STATUS mlme_vdev_sm_deliver_event(struct vdev_mlme_obj *vdev_mlme,
				      enum wlan_vdev_sm_evt event,
				      uint16_t event_data_len,
				      void *event_data);

/**
 * mlme_vdev_sm_print_state_event() - Prints the state/substate, event
 * @vdev_mlme: MLME VDEV comp object
 * @event: MLME event
 *
 * API to print current state/substate, events in readable format
 *
 * Return: void
 */
void mlme_vdev_sm_print_state_event(struct vdev_mlme_obj *vdev_mlme,
				    enum wlan_vdev_sm_evt event);

/**
 * mlme_vdev_sm_print_state() - Prints the state/substate
 * @vdev_mlme: MLME VDEV comp object
 *
 * API to print current state/substate
 *
 * Return: void
 */
void mlme_vdev_sm_print_state(struct vdev_mlme_obj *vdev_mlme);
#ifdef SM_ENG_HIST_ENABLE
/**
 * mlme_vdev_sm_history_print() - Prints SM history
 * @vdev_mlme: MLME VDEV comp object
 *
 * API to print SM history
 *
 * Return: void
 */
void mlme_vdev_sm_history_print(struct vdev_mlme_obj *vdev_mlme);
#endif

#endif

/**
 * mlme_vdev_sm_create - Invoke SME creation for VDEV
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API allocates VDEV MLME SM and initializes SM lock
 *
 * Return: SUCCESS on successful allocation
 *         FAILURE, if registration fails
 */
QDF_STATUS mlme_vdev_sm_create(struct vdev_mlme_obj *vdev_mlme);

/**
 * mlme_vdev_sm_destroy - Invoke SME destroy for VDEV
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API frees VDEV MLME SM and destroys the SM lock
 *
 * Return: SUCCESS on successful destroy
 *         FAILURE, if registration fails
 */
QDF_STATUS mlme_vdev_sm_destroy(struct vdev_mlme_obj *vdev_mlme);

/**
 * mlme_vdev_validate_basic_params - Validate basic params
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API validate MLME VDEV basic parameters
 *
 * Return: SUCCESS on successful validation
 *         FAILURE, if any parameter is not initialized
 */
static inline QDF_STATUS mlme_vdev_validate_basic_params(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_validate_basic_params)
		ret = vdev_mlme->ops->mlme_vdev_validate_basic_params(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_reset_proto_params - Reset VDEV protocol params
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API resets the protocol params fo vdev
 *
 * Return: SUCCESS on successful reset
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_reset_proto_params(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_reset_proto_params)
		ret = vdev_mlme->ops->mlme_vdev_reset_proto_params(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_start_send - Invokes VDEV start operation
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API invokes VDEV start operation
 *
 * Return: SUCCESS on successful completion of start operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_start_send(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_start_send)
		ret = vdev_mlme->ops->mlme_vdev_start_send(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_restart_send - Invokes VDEV restart operation
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API invokes VDEV restart operation
 *
 * Return: SUCCESS on successful completion of restart operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_restart_send(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_restart_send)
		ret = vdev_mlme->ops->mlme_vdev_restart_send(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_stop_start_send - Invoke block VDEV restart operation
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @restart: restart req/start req
 * @event_data_len: data size
 * @event_data: event data
 *
 * API invokes stops pending VDEV restart operation
 *
 * Return: SUCCESS alsways
 */
static inline QDF_STATUS mlme_vdev_stop_start_send(
				struct vdev_mlme_obj *vdev_mlme,
				uint8_t restart,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_stop_start_send)
		ret = vdev_mlme->ops->mlme_vdev_stop_start_send(
				vdev_mlme, restart, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_start_continue - VDEV start response handling
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API invokes VDEV start response actions
 *
 * Return: SUCCESS on successful completion of start response operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_start_continue(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_start_continue)
		ret = vdev_mlme->ops->mlme_vdev_start_continue(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_start_req_failed - Invoke Station VDEV connection, if it pause
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API invokes on START fail response
 *
 * Return: SUCCESS on successful invocation of callback
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_start_req_failed(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_start_req_failed)
		ret = vdev_mlme->ops->mlme_vdev_start_req_failed(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_sta_conn_start - Invoke Station VDEV connection, if it pause
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API invokes connection SM to start station connection
 *
 * Return: SUCCESS on successful invocation of connection sm
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_sta_conn_start(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_sta_conn_start)
		ret = vdev_mlme->ops->mlme_vdev_sta_conn_start(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_up_send - VDEV up operation
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API invokes VDEV up operations
 *
 * Return: SUCCESS on successful completion of up operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_up_send(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_up_send)
		ret = vdev_mlme->ops->mlme_vdev_up_send(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_notify_up_complete - VDEV up state transition notification
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API notifies MLME on moving to UP state
 *
 * Return: SUCCESS on successful completion of up notification
 *         FAILURE, if it fails due to any
 */
static inline
QDF_STATUS mlme_vdev_notify_up_complete(struct vdev_mlme_obj *vdev_mlme,
					uint16_t event_data_len,
					void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (vdev_mlme->ops && vdev_mlme->ops->mlme_vdev_notify_up_complete)
		ret = vdev_mlme->ops->mlme_vdev_notify_up_complete(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_notify_roam_start - VDEV Roaming notification
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_len: data size
 * @event_data: event data
 *
 * API notifies MLME on roaming
 *
 * Return: SUCCESS on successful completion of up notification
 *         FAILURE, if it fails due to any
 */
static inline
QDF_STATUS mlme_vdev_notify_roam_start(struct vdev_mlme_obj *vdev_mlme,
				       uint16_t event_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (vdev_mlme->ops && vdev_mlme->ops->mlme_vdev_notify_roam_start)
		ret = vdev_mlme->ops->mlme_vdev_notify_roam_start(vdev_mlme,
								  event_len,
								  event_data);

	return ret;
}

/**
 * mlme_vdev_update_beacon - Updates beacon
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @op: beacon update type
 * @event_data_len: data size
 * @event_data: event data
 *
 * API updates/allocates/frees the beacon
 *
 * Return: SUCCESS on successful update of beacon
 *         FAILURE, if it fails due to any
 */
static inline
QDF_STATUS mlme_vdev_update_beacon(struct vdev_mlme_obj *vdev_mlme,
				   enum beacon_update_op op,
				   uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (vdev_mlme->ops && vdev_mlme->ops->mlme_vdev_update_beacon)
		ret = vdev_mlme->ops->mlme_vdev_update_beacon(vdev_mlme, op,
						event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_disconnect_peers - Disconnect peers
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API trigger stations disconnection with AP VDEV or AP disconnection with STA
 * VDEV
 *
 * Return: SUCCESS on successful invocation of station disconnection
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_disconnect_peers(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_disconnect_peers)
		ret = vdev_mlme->ops->mlme_vdev_disconnect_peers(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_dfs_cac_timer_stop - Stop CAC timer
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API stops the CAC timer through DFS API
 *
 * Return: SUCCESS on successful CAC timer stop
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_dfs_cac_timer_stop(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_dfs_cac_timer_stop)
		ret = vdev_mlme->ops->mlme_vdev_dfs_cac_timer_stop(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_stop_send - Invokes VDEV stop operation
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API invokes VDEV stop operation
 *
 * Return: SUCCESS on successful completion of stop operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_stop_send(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_stop_send)
		ret = vdev_mlme->ops->mlme_vdev_stop_send(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_stop_continue - VDEV stop response handling
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API invokes VDEV stop response actions
 *
 * Return: SUCCESS on successful completion of stop response operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_stop_continue(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_stop_continue)
		ret = vdev_mlme->ops->mlme_vdev_stop_continue(vdev_mlme,
							      event_data_len,
							      event_data);

	return ret;
}

/**
 * mlme_vdev_down_send - VDEV down operation
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API invokes VDEV down operation
 *
 * Return: SUCCESS on successful completion of VDEV down operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_down_send(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_down_send)
		ret = vdev_mlme->ops->mlme_vdev_down_send(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_notify_down_complete - VDEV init state transition notification
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @event_data_len: data size
 * @event_data: event data
 *
 * API notifies MLME on moving to INIT state
 *
 * Return: SUCCESS on successful completion of down notification
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_notify_down_complete(
				struct vdev_mlme_obj *vdev_mlme,
				uint16_t event_data_len, void *event_data)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_notify_down_complete)
		ret = vdev_mlme->ops->mlme_vdev_notify_down_complete(
					vdev_mlme, event_data_len, event_data);

	return ret;
}

/**
 * mlme_vdev_notify_start_state_exit - VDEV SM start state exit notification
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API notifies on start state exit
 *
 * Return: SUCCESS on successful completion of notification
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_notify_start_state_exit(
				struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) &&
	    vdev_mlme->ops->mlme_vdev_notify_start_state_exit)
		ret = vdev_mlme->ops->mlme_vdev_notify_start_state_exit(
								vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_is_newchan_no_cac - Checks new channel requires CAC
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API checks whether Channel needs CAC period,
 * if yes, it moves to SUSPEND_RESTART to disconnect stations before
 * sending RESTART to FW, otherwise, it moves to RESTART_PROGRESS substate
 *
 * Return: SUCCESS to move to RESTART_PROGRESS substate
 *         FAILURE, move to SUSPEND_RESTART state
 */
static inline QDF_STATUS mlme_vdev_is_newchan_no_cac(
				struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_is_newchan_no_cac)
		ret = vdev_mlme->ops->mlme_vdev_is_newchan_no_cac(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_replace_csa_with_stop_start - Support to use stop-start instead of
 * multivdev restart in CSA with DFS chan
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * Return: NO_SUPPORT if the callback is not supported.
 *         SUCCESS if stop-start is enqueued, else FAILURE.
 */
static inline QDF_STATUS mlme_vdev_replace_csa_with_stop_start(
				struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_E_NOSUPPORT;

	if ((vdev_mlme->ops) &&
	    vdev_mlme->ops->mlme_vdev_replace_csa_with_stop_start)
		ret = vdev_mlme->ops->mlme_vdev_replace_csa_with_stop_start(
							vdev_mlme);

	return ret;
}

#ifdef VDEV_SM_LOCK_SUPPORT
/**
 * mlme_vdev_sm_spinlock_create - Create VDEV MLME spinlock
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * Creates VDEV MLME spinlock
 *
 * Return: void
 */
static inline void mlme_vdev_sm_spinlock_create(struct vdev_mlme_obj *vdev_mlme)
{
	qdf_spinlock_create(&vdev_mlme->sm_lock);
}

/**
 * mlme_vdev_sm_spinlock_destroy - Destroy VDEV MLME spinlock
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * Destroy VDEV MLME spinlock
 *
 * Return: void
 */
static inline void mlme_vdev_sm_spinlock_destroy(
						struct vdev_mlme_obj *vdev_mlme)
{
	qdf_spinlock_destroy(&vdev_mlme->sm_lock);
}

/**
 * mlme_vdev_sm_spin_lock - acquire spinlock
 * @vdev_mlme_obj:  vdev mlme comp object
 *
 * acquire vdev mlme spinlock
 *
 * return: void
 */
static inline void mlme_vdev_sm_spin_lock(struct vdev_mlme_obj *vdev_mlme)
{
	qdf_spin_lock_bh(&vdev_mlme->sm_lock);
}

/**
 * mlme_vdev_sm_spin_unlock - release spinlock
 * @vdev_mlme_obj:  vdev mlme comp object
 *
 * release vdev mlme spinlock
 *
 * return: void
 */
static inline void mlme_vdev_sm_spin_unlock(struct vdev_mlme_obj *vdev_mlme)
{
	qdf_spin_unlock_bh(&vdev_mlme->sm_lock);
}

/**
 * mlme_vdev_cmd_mutex_create - Create VDEV MLME cmd mutex
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * Creates VDEV MLME cmd mutex
 *
 * Return: void
 */
static inline void
mlme_vdev_cmd_mutex_create(struct vdev_mlme_obj *vdev_mlme)
{
	qdf_mutex_create(&vdev_mlme->vdev_cmd_lock);
}

/**
 * mlme_vdev_cmd_mutex_destroy - Destroy VDEV MLME cmd mutex
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * Destroy VDEV MLME cmd mutex
 *
 * Return: void
 */
static inline void
mlme_vdev_cmd_mutex_destroy(struct vdev_mlme_obj *vdev_mlme)
{
	qdf_mutex_destroy(&vdev_mlme->vdev_cmd_lock);
}

/**
 * mlme_vdev_cmd_mutex_acquire - acquire mutex
 * @vdev_mlme_obj:  vdev mlme comp object
 *
 * acquire vdev mlme cmd mutex
 *
 * return: void
 */
static inline void mlme_vdev_cmd_mutex_acquire(struct vdev_mlme_obj *vdev_mlme)
{
	qdf_mutex_acquire(&vdev_mlme->vdev_cmd_lock);
}

/**
 * mlme_vdev_cmd_mutex_release - release mutex
 * @vdev_mlme_obj:  vdev mlme comp object
 *
 * release vdev mlme cmd mutex
 *
 * return: void
 */
static inline void mlme_vdev_cmd_mutex_release(struct vdev_mlme_obj *vdev_mlme)
{
	qdf_mutex_release(&vdev_mlme->vdev_cmd_lock);
}

#else
static inline void mlme_vdev_sm_spinlock_create(struct vdev_mlme_obj *vdev_mlme)
{
	mlme_debug("VDEV SM lock is disabled!!!");
}

static inline void mlme_vdev_sm_spinlock_destroy(
						struct vdev_mlme_obj *vdev_mlme)
{
	mlme_debug("VDEV SM lock is disabled!!!");
}

static inline void mlme_vdev_sm_spin_lock(struct vdev_mlme_obj *vdev_mlme)
{
}

static inline void mlme_vdev_sm_spin_unlock(struct vdev_mlme_obj *vdev_mlme)
{
}

static inline void
mlme_vdev_cmd_mutex_create(struct vdev_mlme_obj *vdev_mlme)
{
	mlme_debug("VDEV CMD lock is disabled!!!");
}

static inline void
mlme_vdev_cmd_mutex_destroy(struct vdev_mlme_obj *vdev_mlme)
{
	mlme_debug("VDEV CMD lock is disabled!!!");
}

static inline void mlme_vdev_cmd_mutex_acquire(struct vdev_mlme_obj *vdev_mlme)
{
}

static inline void mlme_vdev_cmd_mutex_release(struct vdev_mlme_obj *vdev_mlme)
{
}
#endif
