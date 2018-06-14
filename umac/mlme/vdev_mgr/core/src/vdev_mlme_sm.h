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
 * DOC: Declares VDEV MLME SM APIs and structures
 */

#ifndef _VDEV_MLME_SM_H_
#define _VDEV_MLME_SM_H_

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
static inline void mlme_vdev_sm_spinlock_create(
						struct vdev_mlme_obj *vdev_mlme)
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
#endif

#endif

