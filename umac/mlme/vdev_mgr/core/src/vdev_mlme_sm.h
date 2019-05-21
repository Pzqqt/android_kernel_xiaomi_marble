/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
	mlme_info("VDEV SM lock is disabled!!!");
}

static inline void mlme_vdev_sm_spinlock_destroy(
						struct vdev_mlme_obj *vdev_mlme)
{
	mlme_info("VDEV SM lock is disabled!!!");
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
	mlme_info("VDEV CMD lock is disabled!!!");
}

static inline void
mlme_vdev_cmd_mutex_destroy(struct vdev_mlme_obj *vdev_mlme)
{
	mlme_info("VDEV CMD lock is disabled!!!");
}

static inline void mlme_vdev_cmd_mutex_acquire(struct vdev_mlme_obj *vdev_mlme)
{
}

static inline void mlme_vdev_cmd_mutex_release(struct vdev_mlme_obj *vdev_mlme)
{
}
#endif
