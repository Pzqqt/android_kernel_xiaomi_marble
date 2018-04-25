/*
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
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
 * @file qdf_lock.h
 * This file abstracts locking operations.
 */

#ifndef _QDF_LOCK_H
#define _QDF_LOCK_H

#include <qdf_types.h>
#include <qdf_mem.h>
#include <qdf_time.h>
#include <i_qdf_trace.h>

#ifndef QDF_LOCK_STATS
#define QDF_LOCK_STATS 0
#endif
#ifndef QDF_LOCK_STATS_DESTROY_PRINT
#define QDF_LOCK_STATS_DESTROY_PRINT 0
#endif
#ifndef QDF_LOCK_STATS_BUG_ON
#define QDF_LOCK_STATS_BUG_ON 0
#endif
#ifndef QDF_LOCK_STATS_LIST
#define QDF_LOCK_STATS_LIST 0
#endif

/* Max hold time in micro seconds, 0 to disable detection*/
#define QDF_MAX_HOLD_TIME_ALOWED_SPINLOCK_IRQ         10000
#define QDF_MAX_HOLD_TIME_ALOWED_SPINLOCK_BH        1000000
#define QDF_MAX_HOLD_TIME_ALOWED_SPINLOCK                 0

#if !QDF_LOCK_STATS
struct lock_stats {};
#define BEFORE_LOCK(x...) do {} while (0)
#define AFTER_LOCK(x...) do {} while (0)
#define BEFORE_TRYLOCK(x...) do {} while (0)
#define AFTER_TRYLOCK(x...) do {} while (0)
#define BEFORE_UNLOCK(x...) do {} while (0)
#define qdf_lock_stats_create(x...) do {} while (0)
#define qdf_lock_stats_destroy(x...) do {} while (0)
#define qdf_lock_stats_init(x...) do {} while (0)
#define qdf_lock_stats_deinit(x...) do {} while (0)
#else
void qdf_lock_stats_init(void);
void qdf_lock_stats_deinit(void);
struct qdf_lock_cookie;
struct lock_stats {
	const char *initialization_fn;
	const char *acquired_by;
	int line;
	int acquired;
	int contended;
	uint64_t contention_time;
	uint64_t non_contention_time;
	uint64_t held_time;
	uint64_t last_acquired;
	uint64_t max_contention_wait;
	uint64_t max_held_time;
	int num_large_contentions;
	int num_large_holds;
	struct qdf_lock_cookie *cookie;
};
#define LARGE_CONTENTION QDF_LOG_TIMESTAMP_CYCLES_PER_10_US

#define BEFORE_LOCK(lock, was_locked) \
do { \
	uint64_t BEFORE_LOCK_time; \
	uint64_t AFTER_LOCK_time;  \
	bool BEFORE_LOCK_is_locked = was_locked; \
	BEFORE_LOCK_time = qdf_get_log_timestamp(); \
	do {} while (0)


#define AFTER_LOCK(lock, func) \
	lock->stats.acquired_by = func; \
	AFTER_LOCK_time = qdf_get_log_timestamp(); \
	lock->stats.acquired++; \
	lock->stats.last_acquired = AFTER_LOCK_time; \
	if (BEFORE_LOCK_is_locked) { \
		lock->stats.contended++; \
		lock->stats.contention_time += \
			(AFTER_LOCK_time - BEFORE_LOCK_time); \
	} else { \
		lock->stats.non_contention_time += \
			(AFTER_LOCK_time - BEFORE_LOCK_time); \
	} \
\
	if (AFTER_LOCK_time - BEFORE_LOCK_time > LARGE_CONTENTION) \
		lock->stats.num_large_contentions++; \
\
	if (AFTER_LOCK_time - BEFORE_LOCK_time > \
	    lock->stats.max_contention_wait) \
		lock->stats.max_contention_wait = \
			AFTER_LOCK_time - BEFORE_LOCK_time; \
} while (0)

#define BEFORE_TRYLOCK(lock) \
do { \
	uint64_t BEFORE_LOCK_time; \
	uint64_t AFTER_LOCK_time;  \
	BEFORE_LOCK_time = qdf_get_log_timestamp(); \
	do {} while (0)

#define AFTER_TRYLOCK(lock, trylock_return, func) \
	AFTER_LOCK_time = qdf_get_log_timestamp(); \
	if (trylock_return) { \
		lock->stats.acquired++; \
		lock->stats.last_acquired = AFTER_LOCK_time; \
		lock->stats.non_contention_time += \
			(AFTER_LOCK_time - BEFORE_LOCK_time); \
		lock->stats.acquired_by = func; \
	} \
} while (0)

/* max_hold_time in US */
#define BEFORE_UNLOCK(lock, max_hold_time) \
do {\
	uint64_t held_time = qdf_get_log_timestamp() - \
		lock->stats.last_acquired; \
	lock->stats.held_time += held_time; \
\
	if (held_time > lock->stats.max_held_time) \
		lock->stats.max_held_time = held_time; \
\
	if (held_time > LARGE_CONTENTION) \
		lock->stats.num_large_holds++; \
	if (QDF_LOCK_STATS_BUG_ON && max_hold_time && \
	    held_time > qdf_usecs_to_log_timestamp(max_hold_time)) { \
		qdf_print("BEFORE_UNLOCK: lock held too long (%lluus)\n", \
		       qdf_log_timestamp_to_usecs(held_time)); \
		QDF_BUG(0); \
	} \
	lock->stats.acquired_by = NULL; \
} while (0)

void qdf_lock_stats_cookie_destroy(struct lock_stats *stats);
void qdf_lock_stats_cookie_create(struct lock_stats *stats,
				  const char *func, int line);

static inline void qdf_lock_stats_destroy(struct lock_stats *stats)
{
	if (QDF_LOCK_STATS_DESTROY_PRINT) {
		qdf_print("%s: lock: %s %d \t"
			"acquired:\t%d\tcontended:\t%d\t"
			"contention_time\t%llu\tmax_contention_wait:\t%llu\t"
			"non_contention_time\t%llu\t"
			"held_time\t%llu\tmax_held:\t%llu\t\n"
			, __func__, stats->initialization_fn, stats->line,
			stats->acquired, stats->contended,
			qdf_log_timestamp_to_usecs(stats->contention_time),
			qdf_log_timestamp_to_usecs(stats->max_contention_wait),
			qdf_log_timestamp_to_usecs(stats->non_contention_time),
			qdf_log_timestamp_to_usecs(stats->held_time),
			qdf_log_timestamp_to_usecs(stats->max_held_time));
	}

	if (QDF_LOCK_STATS_LIST)
		qdf_lock_stats_cookie_destroy(stats);
}

#ifndef MEMORY_DEBUG
#define qdf_mem_malloc_debug(x, y, z) qdf_mem_malloc(x)
#endif

/* qdf_lock_stats_create() - initialize the lock stats structure
 *
 */
static inline void qdf_lock_stats_create(struct lock_stats *stats,
					 const char *func, int line)
{
	qdf_mem_zero(stats, sizeof(*stats));
	stats->initialization_fn = func;
	stats->line = line;

	if (QDF_LOCK_STATS_LIST)
		qdf_lock_stats_cookie_create(stats, func, line);
}
#endif

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

struct qdf_spinlock {
	__qdf_spinlock_t lock;
	struct lock_stats stats;
};

/**
 * @brief Platform spinlock object
 */
typedef struct qdf_spinlock qdf_spinlock_t;


/**
 * @brief Platform mutex object
 */
typedef __qdf_semaphore_t qdf_semaphore_t;
typedef __qdf_mutex_t qdf_mutex_t;

/* function Declaration */
QDF_STATUS qdf_mutex_create(qdf_mutex_t *m, const char *func, int line);
#define qdf_mutex_create(m) qdf_mutex_create(m, __func__, __LINE__)

QDF_STATUS qdf_mutex_acquire(qdf_mutex_t *m);

QDF_STATUS qdf_mutex_release(qdf_mutex_t *m);

QDF_STATUS qdf_mutex_destroy(qdf_mutex_t *lock);

/**
 * qdf_spinlock_create - Initialize a spinlock
 * @lock: spinlock object pointer
 * Return: none
 */
static inline void qdf_spinlock_create(qdf_spinlock_t *lock, const char *func,
				       int line)
{
	__qdf_spinlock_create(&lock->lock);

	/* spinlock stats create relies on the spinlock working allread */
	qdf_lock_stats_create(&lock->stats, func, line);
}

#define qdf_spinlock_create(x) qdf_spinlock_create(x, __func__, __LINE__)

/**
 * qdf_spinlock_destroy - Delete a spinlock
 * @lock: spinlock object pointer
 * Return: none
 */
static inline void qdf_spinlock_destroy(qdf_spinlock_t *lock)
{
	qdf_lock_stats_destroy(&lock->stats);
	__qdf_spinlock_destroy(&lock->lock);
}

/**
 * qdf_spin_is_locked() - check if the spinlock is locked
 * @lock: spinlock object
 *
 * Return: nonzero if lock is held.
 */
static inline int qdf_spin_is_locked(qdf_spinlock_t *lock)
{
	return __qdf_spin_is_locked(&lock->lock);
}

/**
 * qdf_spin_trylock_bh() - spin trylock bottomhalf
 * @lock: spinlock object
 *
 * Return: nonzero if lock is acquired
 */
static inline int qdf_spin_trylock_bh(qdf_spinlock_t *lock, const char *func)
{
	int trylock_return;

	BEFORE_TRYLOCK(lock);
	trylock_return = __qdf_spin_trylock_bh(&lock->lock);
	AFTER_TRYLOCK(lock, trylock_return, func);

	return trylock_return;
}
#define qdf_spin_trylock_bh(lock) qdf_spin_trylock_bh(lock, __func__)

int qdf_spin_trylock_bh_outline(qdf_spinlock_t *lock);

/**
 * qdf_spin_lock_bh() - locks the spinlock mutex in soft irq context
 * @lock: spinlock object pointer
 * Return: none
 */
static inline void qdf_spin_lock_bh(qdf_spinlock_t *lock, const char *func)
{
	BEFORE_LOCK(lock, qdf_spin_is_locked(lock));
	__qdf_spin_lock_bh(&lock->lock);
	AFTER_LOCK(lock, func);
}

#define qdf_spin_lock_bh(lock) qdf_spin_lock_bh(lock, __func__)

void qdf_spin_lock_bh_outline(qdf_spinlock_t *lock);

/**
 * qdf_spin_unlock_bh() - unlocks the spinlock mutex in soft irq context
 * @lock: spinlock object pointer
 * Return: none
 */
static inline void qdf_spin_unlock_bh(qdf_spinlock_t *lock)
{
	BEFORE_UNLOCK(lock, QDF_MAX_HOLD_TIME_ALOWED_SPINLOCK_BH);
	__qdf_spin_unlock_bh(&lock->lock);
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
	return __qdf_spinlock_irq_exec(hdl, &lock->lock, func, arg);
}

/**
 * qdf_spin_lock() - Acquire a Spinlock(SMP) & disable Preemption (Preemptive)
 * @lock: Lock object
 *
 * Return: none
 */
static inline void qdf_spin_lock(qdf_spinlock_t *lock, const char *func)
{
	BEFORE_LOCK(lock, qdf_spin_is_locked(lock));
	__qdf_spin_lock(&lock->lock);
	AFTER_LOCK(lock, func);
}
#define qdf_spin_lock(lock) qdf_spin_lock(lock, __func__)

/**
 * qdf_spin_unlock() - Unlock the spinlock and enables the Preemption
 * @lock: Lock object
 *
 * Return: none
 */
static inline void qdf_spin_unlock(qdf_spinlock_t *lock)
{
	BEFORE_UNLOCK(lock, QDF_MAX_HOLD_TIME_ALOWED_SPINLOCK);
	__qdf_spin_unlock(&lock->lock);
}

/**
 * qdf_spin_lock_irq() - Acquire a Spinlock(SMP) & save the irq state
 * @lock: Lock object
 * @flags: flags
 *
 * Return: none
 */
static inline void qdf_spin_lock_irq(qdf_spinlock_t *lock, unsigned long flags,
				     const char *func)
{
	BEFORE_LOCK(lock, qdf_spin_is_locked(lock));
	__qdf_spin_lock_irq(&lock->lock.spinlock, flags);
	AFTER_LOCK(lock, func);
}
#define qdf_spin_lock_irq(lock, flags) qdf_spin_lock_irq(lock, flags, __func__)

/**
 * qdf_spin_lock_irqsave() - Acquire a Spinlock (SMP) & disable Preemption
 * (Preemptive) and disable IRQs
 * @lock: Lock object
 *
 * Return: none
 */
static inline void qdf_spin_lock_irqsave(qdf_spinlock_t *lock, const char *func)
{
	BEFORE_LOCK(lock, qdf_spin_is_locked(lock));
	__qdf_spin_lock_irqsave(&lock->lock);
	AFTER_LOCK(lock, func);
}
#define qdf_spin_lock_irqsave(lock) qdf_spin_lock_irqsave(lock, __func__)

/**
 * qdf_spin_unlock_irqrestore() - Unlock the spinlock and enables the
 * Preemption and enable IRQ
 * @lock: Lock object
 *
 * Return: none
 */
static inline void qdf_spin_unlock_irqrestore(qdf_spinlock_t *lock)
{
	BEFORE_UNLOCK(lock, QDF_MAX_HOLD_TIME_ALOWED_SPINLOCK_IRQ);
	__qdf_spin_unlock_irqrestore(&lock->lock);
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
	BEFORE_UNLOCK(lock, QDF_MAX_HOLD_TIME_ALOWED_SPINLOCK_IRQ);
	__qdf_spin_unlock_irq(&lock->lock.spinlock, flags);
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

const char *qdf_wake_lock_name(qdf_wake_lock_t *lock);
QDF_STATUS qdf_wake_lock_timeout_acquire(qdf_wake_lock_t *lock,
					 uint32_t msec);

QDF_STATUS qdf_wake_lock_release(qdf_wake_lock_t *lock, uint32_t reason);

QDF_STATUS qdf_wake_lock_destroy(qdf_wake_lock_t *lock);

QDF_STATUS qdf_runtime_pm_get(void);
QDF_STATUS qdf_runtime_pm_put(void);
QDF_STATUS qdf_runtime_pm_prevent_suspend(qdf_runtime_lock_t *lock);
QDF_STATUS qdf_runtime_pm_allow_suspend(qdf_runtime_lock_t *lock);

QDF_STATUS __qdf_runtime_lock_init(qdf_runtime_lock_t *lock, const char *name);

#define qdf_runtime_lock_init(lock) __qdf_runtime_lock_init(lock, #lock)

void qdf_runtime_lock_deinit(qdf_runtime_lock_t *lock);

QDF_STATUS qdf_spinlock_acquire(qdf_spinlock_t *lock);

QDF_STATUS qdf_spinlock_release(qdf_spinlock_t *lock);
#endif /* _QDF_LOCK_H */
