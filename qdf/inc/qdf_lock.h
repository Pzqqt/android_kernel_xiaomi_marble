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

/**
 * @file qdf_lock.h
 * This file abstracts locking operations.
 */

#ifndef _QDF_LOCK_H
#define _QDF_LOCK_H

#include <qdf_types.h>
#include <i_qdf_lock.h>

#define WIFI_POWER_EVENT_DEFAULT_WAKELOCK_TIMEOUT 0
#define WIFI_POWER_EVENT_WAKELOCK_TAKEN 0
#define WIFI_POWER_EVENT_WAKELOCK_RELEASED 1

/**
 * qdf_semaphore_acquire_timeout() - Take the semaphore before timeout
 * @m: semaphore to take
 * @timeout: maximum time to try to take the semaphore
 * Return: int
 */
static inline int qdf_semaphore_acquire_timeout(struct semaphore *m,
						unsigned long timeout)
{
	return __qdf_semaphore_acquire_timeout(m, timeout);
}

/**
 * @brief Platform spinlock object
 */
typedef __qdf_spinlock_t qdf_spinlock_t;

/**
 * @brief Platform mutex object
 */
typedef __qdf_semaphore_t qdf_semaphore_t;
typedef __qdf_mutex_t qdf_mutex_t;

/* function Declaration */
QDF_STATUS qdf_mutex_create(qdf_mutex_t *m);

QDF_STATUS qdf_mutex_acquire(qdf_mutex_t *m);

QDF_STATUS qdf_mutex_release(qdf_mutex_t *m);

QDF_STATUS qdf_mutex_destroy(qdf_mutex_t *lock);

/**
 * qdf_spinlock_create - Initialize a spinlock
 * @lock: spinlock object pointer
 * Retrun: none
 */
static inline void qdf_spinlock_create(qdf_spinlock_t *lock)
{
	__qdf_spinlock_create(lock);
}

/**
 * qdf_spinlock_destroy - Delete a spinlock
 * @lock: spinlock object pointer
 * Return: none
 */
static inline void qdf_spinlock_destroy(qdf_spinlock_t *lock)
{
	__qdf_spinlock_destroy(lock);
}

/**
 * qdf_spin_trylock_bh() - spin trylock bottomhalf
 * @lock: spinlock object
 * Return: int
 */
static inline int qdf_spin_trylock_bh(qdf_spinlock_t *lock)
{
	return __qdf_spin_trylock_bh(lock);
}

int qdf_spin_trylock_bh_outline(qdf_spinlock_t *lock);

/**
 * qdf_spin_lock_bh() - locks the spinlock mutex in soft irq context
 * @lock: spinlock object pointer
 * Return: none
 */
static inline void qdf_spin_lock_bh(qdf_spinlock_t *lock)
{
	__qdf_spin_lock_bh(lock);
}

void qdf_spin_lock_bh_outline(qdf_spinlock_t *lock);

/**
 * qdf_spin_unlock_bh() - unlocks the spinlock mutex in soft irq context
 * @lock: spinlock object pointer
 * Return: none
 */
static inline void qdf_spin_unlock_bh(qdf_spinlock_t *lock)
{
	__qdf_spin_unlock_bh(lock);
}

void qdf_spin_unlock_bh_outline(qdf_spinlock_t *lock);

/**
 * qdf_spinlock_irq_exec - Execute the input function with spinlock held
 * and interrupt disabled.
 * @hdl: OS handle
 * @lock: spinlock to be held for the critical region
 * @func: critical region function that to be executed
 * @context: context of the critical region function
 * Return: Boolean status returned by the critical region function
 */
static inline bool qdf_spinlock_irq_exec(qdf_handle_t hdl,
					 qdf_spinlock_t *lock,
					 qdf_irqlocked_func_t func, void *arg)
{
	return __qdf_spinlock_irq_exec(hdl, lock, func, arg);
}

/**
 * qdf_spin_lock() - Acquire a Spinlock(SMP) & disable Preemption (Preemptive)
 * @lock: Lock object
 *
 * Return: none
 */
static inline void qdf_spin_lock(qdf_spinlock_t *lock)
{
	__qdf_spin_lock(lock);
}

/**
 * qdf_spin_unlock() - Unlock the spinlock and enables the Preemption
 * @lock: Lock object
 *
 * Return: none
 */
static inline void qdf_spin_unlock(qdf_spinlock_t *lock)
{
	__qdf_spin_unlock(lock);
}

/**
 * qdf_spin_lock_irq() - Acquire a Spinlock(SMP) & save the irq state
 * @lock: Lock object
 * @flags: flags
 *
 * Return: none
 */
static inline void qdf_spin_lock_irq(qdf_spinlock_t *lock, unsigned long flags)
{
	__qdf_spin_lock_irq(&lock->spinlock, flags);
}

/**
 * qdf_spin_lock_irqsave() - Acquire a Spinlock (SMP) & disable Preemption
 * (Preemptive) and disable IRQs
 * @lock: Lock object
 *
 * Return: none
 */
static inline void qdf_spin_lock_irqsave(qdf_spinlock_t *lock)
{
	__qdf_spin_lock_irqsave(lock);
}

/**
 * qdf_spin_unlock_irqrestore() - Unlock the spinlock and enables the
 * Preemption and enable IRQ
 * @lock: Lock object
 *
 * Return: none
 */
static inline void qdf_spin_unlock_irqrestore(qdf_spinlock_t *lock)
{
	__qdf_spin_unlock_irqrestore(lock);
}

/**
 * qdf_spin_unlock_irq() - Unlock a Spinlock(SMP) & save the restore state
 * @lock: Lock object
 * @flags: flags
 *
 * Return: none
 */
static inline void qdf_spin_unlock_irq(qdf_spinlock_t *lock,
				       unsigned long flags)
{
	__qdf_spin_unlock_irq(&lock->spinlock, flags);
}

/**
 * qdf_semaphore_init() - initialize a semaphore
 * @m: Semaphore to initialize
 * Return: None
 */
static inline void qdf_semaphore_init(qdf_semaphore_t *m)
{
	__qdf_semaphore_init(m);
}

/**
 * qdf_semaphore_acquire() - take the semaphore
 * @m: Semaphore to take
 * Return: int
 */
static inline int qdf_semaphore_acquire(qdf_semaphore_t *m)
{
	return __qdf_semaphore_acquire(m);
}

/**
 * qdf_semaphore_release() - give the semaphore
 * @m: Semaphore to give
 * Return: None
 */
static inline void qdf_semaphore_release(qdf_semaphore_t *m)
{
	__qdf_semaphore_release(m);
}

/**
 * qdf_semaphore_acquire_intr - Take the semaphore, interruptible version
 * @osdev: OS Device
 * @m: mutex to take
 * Return: int
 */
static inline int qdf_semaphore_acquire_intr(qdf_semaphore_t *m)
{
	return __qdf_semaphore_acquire_intr(m);
}

QDF_STATUS qdf_wake_lock_create(qdf_wake_lock_t *lock, const char *name);

QDF_STATUS qdf_wake_lock_acquire(qdf_wake_lock_t *lock, uint32_t reason);

QDF_STATUS qdf_wake_lock_timeout_acquire(qdf_wake_lock_t *lock,
					 uint32_t msec, uint32_t reason);

QDF_STATUS qdf_wake_lock_release(qdf_wake_lock_t *lock, uint32_t reason);

QDF_STATUS qdf_wake_lock_destroy(qdf_wake_lock_t *lock);

struct hif_pm_runtime_lock;
typedef struct hif_pm_runtime_lock *qdf_runtime_lock_t;

QDF_STATUS qdf_runtime_pm_get(void);
QDF_STATUS qdf_runtime_pm_put(void);
QDF_STATUS qdf_runtime_pm_prevent_suspend(qdf_runtime_lock_t lock);
QDF_STATUS qdf_runtime_pm_allow_suspend(qdf_runtime_lock_t lock);
qdf_runtime_lock_t qdf_runtime_lock_init(const char *name);
void qdf_runtime_lock_deinit(qdf_runtime_lock_t lock);

QDF_STATUS qdf_spinlock_acquire(qdf_spinlock_t *lock);

QDF_STATUS qdf_spinlock_release(qdf_spinlock_t *lock);

#endif /* _QDF_LOCK_H */
