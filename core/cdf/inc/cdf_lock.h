/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#if !defined(__CDF_LOCK_H)
#define __CDF_LOCK_H

/**
 *
 * @file  cdf_lock.h
 *
 * @brief Connectivity driver framework (CDF) lock APIs
 *
 * Definitions for CDF locks
 *
 */

/* Include Files */
#include "cdf_status.h"
#include "qdf_status.h"
#include "i_cdf_lock.h"

/* Preprocessor definitions and constants */

/* Type declarations */
/**
 * @brief Platform spinlock object
 */
typedef __cdf_spinlock_t cdf_spinlock_t;
/**
 * @brief Platform mutex object
 */
typedef __cdf_semaphore_t cdf_semaphore_t;

/* Function declarations and documenation */

/**
 * cdf_semaphore_init() - initialize a semaphore
 * @m:  Semaphore to initialize
 *
 * Return: None
 */

static inline void cdf_semaphore_init(cdf_semaphore_t *m)
{
	__cdf_semaphore_init(m);
}

/**
 * cdf_semaphore_acquire() - take the semaphore
 * @m:  Semaphore to take
 *
 * Return: None
 */
static inline int cdf_semaphore_acquire(cdf_device_t osdev, cdf_semaphore_t *m)
{
	return __cdf_semaphore_acquire(osdev, m);
}

/**
 * cdf_semaphore_release () - give the semaphore
 * @m:  Semaphore to give
 *
 * Return: None
 */
static inline void
cdf_semaphore_release(cdf_device_t osdev, cdf_semaphore_t *m)
{
	__cdf_semaphore_release(osdev, m);
}

/**
 * cdf_mutex_init() - initialize a CDF lock
 * @lock:	 Pointer to the opaque lock object to initialize
 *
 * cdf_mutex_init() function initializes the specified lock. Upon
 * successful initialization, the state of the lock becomes initialized
 * and unlocked.
 *
 * A lock must be initialized by calling cdf_mutex_init() before it
 * may be used in any other lock functions.
 *
 * Attempting to initialize an already initialized lock results in
 * a failure.
 *
 * Return:
 *	CDF_STATUS_SUCCESS:	lock was successfully initialized
 *	CDF failure reason codes: lock is not initialized and can't be used
 */
CDF_STATUS cdf_mutex_init(cdf_mutex_t *lock);

/**
 * cdf_mutex_acquire () - acquire a CDF lock
 * @lock:	 Pointer to the opaque lock object to acquire
 *
 * A lock object is acquired by calling cdf_mutex_acquire().  If the lock
 * is already locked, the calling thread shall block until the lock becomes
 * available. This operation shall return with the lock object referenced by
 * lock in the locked state with the calling thread as its owner.
 *
 * Return:
 *	CDF_STATUS_SUCCESS:	lock was successfully initialized
 *	CDF failure reason codes: lock is not initialized and can't be used
 */
CDF_STATUS cdf_mutex_acquire(cdf_mutex_t *lock);

/**
 * cdf_mutex_release() - release a CDF lock
 * @lock:	 Pointer to the opaque lock object to be released
 *
 * cdf_mutex_release() function shall release the lock object
 * referenced by 'lock'.
 *
 * If a thread attempts to release a lock that it unlocked or is not
 * initialized, an error is returned.
 *
 * Return:
 *	CDF_STATUS_SUCCESS:	lock was successfully initialized
 *	CDF failure reason codes: lock is not initialized and can't be used
 */
CDF_STATUS cdf_mutex_release(cdf_mutex_t *lock);

/**
 * cdf_mutex_destroy() - destroy a CDF lock
 * @lock:	 Pointer to the opaque lock object to be destroyed
 *
 * cdf_mutex_destroy() function shall destroy the lock object
 * referenced by lock.  After a successful return from \a cdf_mutex_destroy()
 * the lock object becomes, in effect, uninitialized.
 *
 * A destroyed lock object can be reinitialized using cdf_mutex_init();
 * the results of otherwise referencing the object after it has been destroyed
 * are undefined.  Calls to CDF lock functions to manipulate the lock such
 * as cdf_mutex_acquire() will fail if the lock is destroyed.  Therefore,
 * don't use the lock after it has been destroyed until it has
 * been re-initialized.
 *
 * Return:
 *	CDF_STATUS_SUCCESS:	lock was successfully initialized
 *	CDF failure reason codes: lock is not initialized and can't be used
 */
CDF_STATUS cdf_mutex_destroy(cdf_mutex_t *lock);

/**
 * cdf_spinlock_init() - initialize a spinlock
 * @lock: Spinlock object pointer
 *
 * Return: None
 */
static inline void cdf_spinlock_init(cdf_spinlock_t *lock)
{
	__cdf_spinlock_init(lock);
}

/**
 * cdf_spinlock_destroy() - delete a spinlock
 * @lock: Spinlock object pointer
 *
 * Return: None
 */
static inline void cdf_spinlock_destroy(cdf_spinlock_t *lock)
{
	__cdf_spinlock_destroy(lock);
}

/**
 * cdf_spin_lock_bh() - locks the spinlock semaphore in soft irq context
 * @lock: Spinlock object pointer
 *
 * Return: None
 */
static inline void cdf_spin_lock_bh(cdf_spinlock_t *lock)
{
	__cdf_spin_lock_bh(lock);
}

/**
 * cdf_spin_lock_bh() - unlocks the spinlock semaphore in soft irq context
 * @lock: Spinlock object pointer
 *
 * Return: None
 */
static inline void cdf_spin_unlock_bh(cdf_spinlock_t *lock)
{
	__cdf_spin_unlock_bh(lock);
}

/**
 * cdf_wake_lock_init() - initializes a CDF wake lock
 * @lock: The wake lock to initialize
 * @name: Name of wake lock
 *
 * Return:
 *    CDF status success : if wake lock is initialized
 *    CDF status fialure : if wake lock was not initialized
 */
CDF_STATUS cdf_wake_lock_init(cdf_wake_lock_t *lock, const char *name);

/**
 * cdf_wake_lock_acquire() - acquires a wake lock
 * @lock:	The wake lock to acquire
 * @reason:	Reason for taking wakelock
 *
 * Return:
 *    CDF status success : if wake lock is acquired
 *    CDF status fialure : if wake lock was not acquired
 */
CDF_STATUS cdf_wake_lock_acquire(cdf_wake_lock_t *pLock, uint32_t reason);

/**
 * cdf_wake_lock_timeout_acquire() - acquires a wake lock with a timeout
 * @lock:	The wake lock to acquire
 * @reason:	Reason for taking wakelock
 *
 * Return:
 *   CDF status success : if wake lock is acquired
 *   CDF status fialure : if wake lock was not acquired
 */
CDF_STATUS cdf_wake_lock_timeout_acquire(cdf_wake_lock_t *pLock,
					 uint32_t msec, uint32_t reason);

/**
 * cdf_wake_lock_release() - releases a wake lock
 * @lock:	the wake lock to release
 * @@reason:	Reason for taking wakelock
 *
 * Return:
 *    CDF status success : if wake lock is acquired
 *    CDF status fialure : if wake lock was not acquired
 */
CDF_STATUS cdf_wake_lock_release(cdf_wake_lock_t *pLock, uint32_t reason);

/**
 * cdf_wake_lock_destroy() - destroys a wake lock
 * @lock:	The wake lock to destroy
 *
 * Return:
 * CDF status success :	if wake lock is acquired
 * CDF status fialure :	if wake lock was not acquired
 */
CDF_STATUS cdf_wake_lock_destroy(cdf_wake_lock_t *pLock);

struct hif_pm_runtime_lock;
typedef struct hif_pm_runtime_lock *cdf_runtime_lock_t;

CDF_STATUS cdf_runtime_pm_get(void);
CDF_STATUS cdf_runtime_pm_put(void);
CDF_STATUS cdf_runtime_pm_prevent_suspend(cdf_runtime_lock_t lock);
CDF_STATUS cdf_runtime_pm_allow_suspend(cdf_runtime_lock_t lock);
cdf_runtime_lock_t cdf_runtime_lock_init(const char *name);
void cdf_runtime_lock_deinit(cdf_runtime_lock_t lock);

/**
 * cdf_spinlock_acquire() - acquires a spin lock
 * @lock:	Spin lock to acquire
 *
 * Return:
 *    CDF status success : if wake lock is acquired
 *    CDF status fialure : if wake lock was not acquired
 */
CDF_STATUS cdf_spinlock_acquire(cdf_spinlock_t *pLock);

/**
 * cdf_spinlock_release() - release a spin lock
 * @lock:	Spin lock to release
 *
 * Return:
 * CDF status success :	if wake lock is acquired
 * CDF status fialure :	if wake lock was not acquired
 */
CDF_STATUS cdf_spinlock_release(cdf_spinlock_t *pLock);

#define cdf_spin_lock(_lock) __cdf_spin_lock(_lock)
#define cdf_spin_unlock(_lock) __cdf_spin_unlock(_lock)
#define cdf_spin_lock_irqsave(_lock) __cdf_spin_lock_irqsave(_lock)
#define cdf_spin_unlock_irqrestore(_lock) \
	__cdf_spin_unlock_irqrestore(_lock)
#define cdf_spin_lock_irq(_pLock, _flags)   __cdf_spin_lock_irq(_pLock, _flags)
#define cdf_spin_unlock_irq(_pLock, _flags) \
	__cdf_spin_unlock_irq(_pLock, _flags)

#define cdf_in_softirq() __cdf_in_softirq()

#endif /* __CDF_LOCK_H */
