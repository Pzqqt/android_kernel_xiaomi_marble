/*
 * Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
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

#if !defined(__I_CDF_LOCK_H)
#define __I_CDF_LOCK_H

/**
 * DOC: i_cdf_lock.h
 *
 * Linux-specific definitions for CDF Locks
 *
 */

/* Include Files */
#include <cdf_types.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#if defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
#include <linux/wakelock.h>
#endif

/* Preprocessor definitions and constants */

/* define for flag */
#define ADF_OS_LINUX_UNLOCK_BH  1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * typedef struct - cdf_mutex_t
 * @m_lock: Mutex lock
 * @cookie: Lock cookie
 * @processID: Process ID to track lock
 * @state: Lock status
 * @refcount: Reference count for recursive lock
 */
typedef struct cdf_lock_s {
	struct mutex m_lock;
	uint32_t cookie;
	int processID;
	uint32_t state;
	uint8_t refcount;
} cdf_mutex_t;

/**
 * typedef struct - cdf_spinlock_t
 * @spinlock: Spin lock
 * @flags: Lock flag
 * @_flags: Internal lock flag
 */
typedef struct __cdf_spinlock {
	spinlock_t spinlock;
	unsigned int flags;
	unsigned long _flags;
} cdf_spinlock_t;

typedef cdf_spinlock_t __cdf_spinlock_t;
typedef struct semaphore __cdf_semaphore_t;

#if defined CONFIG_CNSS
typedef struct wakeup_source cdf_wake_lock_t;
#elif defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
typedef struct wake_lock cdf_wake_lock_t;
#else
typedef int cdf_wake_lock_t;
#endif

/* Function declarations and documenation */

/**
 * __cdf_semaphore_init() - initialize the semaphore
 * @m: Semaphore object
 *
 * Return: QDF_STATUS_SUCCESS
 */
static inline QDF_STATUS __cdf_semaphore_init(struct semaphore *m)
{
	sema_init(m, 1);
	return QDF_STATUS_SUCCESS;
}

/**
 * __cdf_semaphore_acquire() - acquire semaphore
 * @m: Semaphore object
 *
 * Return: 0
 */
static inline int
__cdf_semaphore_acquire(cdf_device_t osdev, struct semaphore *m)
{
	down(m);
	return 0;
}

/**
 * __cdf_semaphore_release() - release semaphore
 * @m: Semaphore object
 *
 * Return: result of UP operation in integer
 */
static inline void
__cdf_semaphore_release(cdf_device_t osdev, struct semaphore *m)
{
	up(m);
}

/**
 * __cdf_spinlock_init() - initialize spin lock
 * @lock: Spin lock object
 *
 * Return: QDF_STATUS_SUCCESS
 */
static inline QDF_STATUS __cdf_spinlock_init(__cdf_spinlock_t *lock)
{
	spin_lock_init(&lock->spinlock);
	lock->flags = 0;

	return QDF_STATUS_SUCCESS;
}

#define __cdf_spinlock_destroy(lock)
/**
 * __cdf_spin_lock() - Acquire a Spinlock(SMP) & disable Preemption (Preemptive)
 * @lock: Lock object
 *
 * Return: none
 */
static inline void
__cdf_spin_lock(__cdf_spinlock_t *lock)
{
	spin_lock(&lock->spinlock);
}

/**
 * __cdf_spin_unlock() - Unlock the spinlock and enables the Preemption
 * @lock: Lock object
 *
 * Return: none
 */
static inline void
__cdf_spin_unlock(__cdf_spinlock_t *lock)
{
	spin_unlock(&lock->spinlock);
}

/**
 * __cdf_spin_lock_irqsave() - Acquire a Spinlock (SMP) & disable Preemption
 *				(Preemptive) and disable IRQs
 * @lock: Lock object
 *
 * Return: none
 */
static inline void
__cdf_spin_lock_irqsave(__cdf_spinlock_t *lock)
{
	spin_lock_irqsave(&lock->spinlock, lock->_flags);
}
/**
 * __cdf_spin_unlock_irqrestore() - Unlock the spinlock and enables the
 *					Preemption and enable IRQ
 * @lock: Lock object
 *
 * Return: none
 */
static inline void
__cdf_spin_unlock_irqrestore(__cdf_spinlock_t *lock)
{
	spin_unlock_irqrestore(&lock->spinlock, lock->_flags);
}

/*
 * Synchronous versions - only for OS' that have interrupt disable
 */
#define __cdf_spin_lock_irq(_pLock, _flags)    spin_lock_irqsave(_pLock, _flags)
#define __cdf_spin_unlock_irq(_pLock, _flags)  spin_unlock_irqrestore(_pLock, _flags)

/**
 * __cdf_spin_lock_bh() - Acquire the spinlock and disable bottom halves
 * @lock: Lock object
 *
 * Return: none
 */
static inline void
__cdf_spin_lock_bh(__cdf_spinlock_t *lock)
{
	if (likely(irqs_disabled() || in_softirq())) {
		spin_lock(&lock->spinlock);
	} else {
		spin_lock_bh(&lock->spinlock);
		lock->flags |= ADF_OS_LINUX_UNLOCK_BH;
	}

}

/**
 * __cdf_spin_unlock_bh() - Release the spinlock and enable bottom halves
 * @lock: Lock object
 *
 * Return: none
 */
static inline void
__cdf_spin_unlock_bh(__cdf_spinlock_t *lock)
{
	if (unlikely(lock->flags & ADF_OS_LINUX_UNLOCK_BH)) {
		lock->flags &= ~ADF_OS_LINUX_UNLOCK_BH;
		spin_unlock_bh(&lock->spinlock);
	} else
		spin_unlock(&lock->spinlock);
}

/**
 * __cdf_in_softirq() - in soft irq context
 *
 * Return: true if in softirs context else false
 */
static inline bool __cdf_in_softirq(void)
{
	return in_softirq();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __I_CDF_LOCK_H */
