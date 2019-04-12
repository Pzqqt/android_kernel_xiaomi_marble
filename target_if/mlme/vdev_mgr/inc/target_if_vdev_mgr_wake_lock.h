/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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
 * DOC: target_if_vdev_mgr_wake_lock.h
 *
 * This file provides declaration for wakelock APIs
 */

#ifndef __TARGET_IF_VDEV_MGR_WAKE_LOCK_H__
#define __TARGET_IF_VDEV_MGR_WAKE_LOCK_H__

enum wakelock_mode {
	START_WAKELOCK,
	STOP_WAKELOCK,
	DELETE_WAKELOCK
};

#ifdef FEATURE_VDEV_RSP_WAKELOCK

/**
 * target_if_wake_lock_init() - API to initialize
				wakelocks:start,
				stop and delete.
 * @vdev: pointer to vdev
 *
 * This also initialize the runtime lock
 *
 * Return: None
 */
void target_if_wake_lock_init(struct wlan_objmgr_vdev *vdev);

/**
 * target_if_wake_lock_deinit() - API to destroy
			wakelocks:start, stop and delete.
 * @vdev: pointer to vdev
 *
 * This also destroy the runtime lock
 *
 * Return: None
 */
void target_if_wake_lock_deinit(struct wlan_objmgr_vdev *vdev);

/**
 * target_if_start_wake_lock_timeout_acquire() - acquire the
					vdev start wakelock
 * @vdev: pointer to vdev
 *
 * This also acquires the target_if runtime pm lock.
 *
 * Return: Success/Failure
 */
QDF_STATUS target_if_wake_lock_timeout_acquire(struct wlan_objmgr_vdev *vdev,
					       enum wakelock_mode mode);
/**
 * target_if_start_wake_lock_timeout_release() - release the
						start wakelock
 * @vdev: pointer to vdev
 *
 * This also release the target_if runtime pm lock.
 *
 * Return: Success/Failure
 */
QDF_STATUS target_if_wake_lock_timeout_release(struct wlan_objmgr_vdev *vdev,
					       enum wakelock_mode mode);
#else
static inline void target_if_wake_lock_init(struct wlan_objmgr_vdev *vdev)
{
}

static inline void target_if_wake_lock_deinit(struct wlan_objmgr_vdev *vdev)
{
}

static inline QDF_STATUS target_if_wake_lock_timeout_acquire(
					struct wlan_objmgr_vdev *vdev,
					enum wakelock_mode mode)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS target_if_wake_lock_timeout_release(
				struct wlan_objmgr_vdev *vdev,
				enum wakelock_mode mode)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif
