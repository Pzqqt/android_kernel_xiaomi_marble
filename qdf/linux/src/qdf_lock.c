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

#include <linux/module.h>
#include <qdf_lock.h>
#include <qdf_trace.h>

#include <qdf_types.h>
#ifdef CONFIG_MCL
#include <i_host_diag_core_event.h>
#include <hif.h>
#include <cds_api.h>
#endif
#include <i_qdf_lock.h>

/* Function declarations and documenation */
typedef __qdf_mutex_t qdf_mutex_t;

/**
 * qdf_mutex_create() - Initialize a mutex
 * @m: mutex to initialize
 *
 * Returns: QDF_STATUS
 * =0 success
 * else fail status
 */
QDF_STATUS qdf_mutex_create(qdf_mutex_t *lock)
{
	/* check for invalid pointer */
	if (lock == NULL) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed in", __func__);
		return QDF_STATUS_E_FAULT;
	}
	/* check for 'already initialized' lock */
	if (LINUX_LOCK_COOKIE == lock->cookie) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: already initialized lock", __func__);
		return QDF_STATUS_E_BUSY;
	}

	if (in_interrupt()) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s cannot be called from interrupt context!!!",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}

	/* initialize new lock */
	mutex_init(&lock->m_lock);
	lock->cookie = LINUX_LOCK_COOKIE;
	lock->state = LOCK_RELEASED;
	lock->process_id = 0;
	lock->refcount = 0;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_mutex_create);

/**
 * qdf_mutex_acquire() - acquire a QDF lock
 * @lock: Pointer to the opaque lock object to acquire
 *
 * A lock object is acquired by calling qdf_mutex_acquire().  If the lock
 * is already locked, the calling thread shall block until the lock becomes
 * available. This operation shall return with the lock object referenced by
 * lock in the locked state with the calling thread as its owner.
 *
 * Return:
 * QDF_STATUS_SUCCESS: lock was successfully initialized
 * QDF failure reason codes: lock is not initialized and can't be used
 */
QDF_STATUS qdf_mutex_acquire(qdf_mutex_t *lock)
{
	int rc;
	/* check for invalid pointer */
	if (lock == NULL) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed in", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}
	/* check if lock refers to an initialized object */
	if (LINUX_LOCK_COOKIE != lock->cookie) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: uninitialized lock", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	if (in_interrupt()) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s cannot be called from interrupt context!!!",
			  __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}
	if ((lock->process_id == current->pid) &&
		(lock->state == LOCK_ACQUIRED)) {
		lock->refcount++;
#ifdef QDF_NESTED_LOCK_DEBUG
			pe_err("%s: %x %d %d", __func__, lock, current->pid,
			  lock->refcount);
#endif
		return QDF_STATUS_SUCCESS;
	}
	/* acquire a Lock */
	mutex_lock(&lock->m_lock);
	rc = mutex_is_locked(&lock->m_lock);
	if (rc == 0) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: unable to lock mutex (rc = %d)", __func__, rc);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
#ifdef QDF_NESTED_LOCK_DEBUG
		pe_err("%s: %x %d", __func__, lock, current->pid);
#endif
	if (LOCK_DESTROYED != lock->state) {
		lock->process_id = current->pid;
		lock->refcount++;
		lock->state = LOCK_ACQUIRED;
		return QDF_STATUS_SUCCESS;
	} else {
		/* lock is already destroyed */
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Lock is already destroyed", __func__);
		mutex_unlock(&lock->m_lock);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
}
EXPORT_SYMBOL(qdf_mutex_acquire);

/**
 * qdf_mutex_release() - release a QDF lock
 * @lock: Pointer to the opaque lock object to be released
 *
 * qdf_mutex_release() function shall release the lock object
 * referenced by 'lock'.
 *
 * If a thread attempts to release a lock that it unlocked or is not
 * initialized, an error is returned.
 *
 * Return:
 * QDF_STATUS_SUCCESS: lock was successfully initialized
 * QDF failure reason codes: lock is not initialized and can't be used
 */
QDF_STATUS qdf_mutex_release(qdf_mutex_t *lock)
{
	/* check for invalid pointer */
	if (lock == NULL) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed in", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	/* check if lock refers to an uninitialized object */
	if (LINUX_LOCK_COOKIE != lock->cookie) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: uninitialized lock", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	if (in_interrupt()) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s cannot be called from interrupt context!!!",
			  __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	/* current_thread = get_current_thread_id();
	 * Check thread ID of caller against thread ID
	 * of the thread which acquire the lock
	 */
	if (lock->process_id != current->pid) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: current task pid does not match original task pid!!",
			  __func__);
#ifdef QDF_NESTED_LOCK_DEBUG
		pe_err("%s: Lock held by=%d being released by=%d",
			  __func__, lock->process_id, current->pid);
#endif
		QDF_ASSERT(0);
		return QDF_STATUS_E_PERM;
	}
	if ((lock->process_id == current->pid) &&
		(lock->state == LOCK_ACQUIRED)) {
		if (lock->refcount > 0)
			lock->refcount--;
	}
#ifdef QDF_NESTED_LOCK_DEBUG
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR, "%s: %x %d %d", __func__, lock, lock->process_id,
		  lock->refcount);
#endif
	if (lock->refcount)
		return QDF_STATUS_SUCCESS;

	lock->process_id = 0;
	lock->refcount = 0;
	lock->state = LOCK_RELEASED;
	/* release a Lock */
	mutex_unlock(&lock->m_lock);
#ifdef QDF_NESTED_LOCK_DEBUG
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR, "%s: Freeing lock %x %d %d", lock, lock->process_id,
		  lock->refcount);
#endif
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_mutex_release);

/**
 * qdf_wake_lock_name() - This function returns the name of the wakelock
 * @lock: Pointer to the wakelock
 *
 * This function returns the name of the wakelock
 *
 * Return: Pointer to the name if it is valid or a default string
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
const char *qdf_wake_lock_name(qdf_wake_lock_t *lock)
{
	if (lock->name)
		return lock->name;
	return "UNNAMED_WAKELOCK";
}
#else
const char *qdf_wake_lock_name(qdf_wake_lock_t *lock)
{
	return "NO_WAKELOCK_SUPPORT";
}
#endif
EXPORT_SYMBOL(qdf_wake_lock_name);

/**
 * qdf_wake_lock_create() - initializes a wake lock
 * @lock: The wake lock to initialize
 * @name: Name of wake lock
 *
 * Return:
 * QDF status success: if wake lock is initialized
 * QDF status failure: if wake lock was not initialized
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
QDF_STATUS qdf_wake_lock_create(qdf_wake_lock_t *lock, const char *name)
{
	wakeup_source_init(lock, name);
	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS qdf_wake_lock_create(qdf_wake_lock_t *lock, const char *name)
{
	return QDF_STATUS_SUCCESS;
}
#endif
EXPORT_SYMBOL(qdf_wake_lock_create);

/**
 * qdf_wake_lock_acquire() - acquires a wake lock
 * @lock: The wake lock to acquire
 * @reason: Reason for wakelock
 *
 * Return:
 * QDF status success: if wake lock is acquired
 * QDF status failure: if wake lock was not acquired
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
QDF_STATUS qdf_wake_lock_acquire(qdf_wake_lock_t *lock, uint32_t reason)
{
#ifdef CONFIG_MCL
	host_diag_log_wlock(reason, qdf_wake_lock_name(lock),
			WIFI_POWER_EVENT_DEFAULT_WAKELOCK_TIMEOUT,
			WIFI_POWER_EVENT_WAKELOCK_TAKEN);
#endif
	__pm_stay_awake(lock);
	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS qdf_wake_lock_acquire(qdf_wake_lock_t *lock, uint32_t reason)
{
	return QDF_STATUS_SUCCESS;
}
#endif
EXPORT_SYMBOL(qdf_wake_lock_acquire);

/**
 * qdf_wake_lock_timeout_acquire() - acquires a wake lock with a timeout
 * @lock: The wake lock to acquire
 * @reason: Reason for wakelock
 *
 * Return:
 * QDF status success: if wake lock is acquired
 * QDF status failure: if wake lock was not acquired
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
QDF_STATUS qdf_wake_lock_timeout_acquire(qdf_wake_lock_t *lock, uint32_t msec)
{
	/* Wakelock for Rx is frequent.
	 * It is reported only during active debug
	 */
	__pm_wakeup_event(lock, msec);
	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS qdf_wake_lock_timeout_acquire(qdf_wake_lock_t *lock, uint32_t msec)
{
	return QDF_STATUS_SUCCESS;
}
#endif
EXPORT_SYMBOL(qdf_wake_lock_timeout_acquire);

/**
 * qdf_wake_lock_release() - releases a wake lock
 * @lock: the wake lock to release
 * @reason: Reason for wakelock
 *
 * Return:
 * QDF status success: if wake lock is acquired
 * QDF status failure: if wake lock was not acquired
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
QDF_STATUS qdf_wake_lock_release(qdf_wake_lock_t *lock, uint32_t reason)
{
#ifdef CONFIG_MCL
	host_diag_log_wlock(reason, qdf_wake_lock_name(lock),
			WIFI_POWER_EVENT_DEFAULT_WAKELOCK_TIMEOUT,
			WIFI_POWER_EVENT_WAKELOCK_RELEASED);
#endif
	__pm_relax(lock);
	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS qdf_wake_lock_release(qdf_wake_lock_t *lock, uint32_t reason)
{
	return QDF_STATUS_SUCCESS;
}
#endif
EXPORT_SYMBOL(qdf_wake_lock_release);

/**
 * qdf_wake_lock_destroy() - destroys a wake lock
 * @lock: The wake lock to destroy
 *
 * Return:
 * QDF status success: if wake lock is acquired
 * QDF status failure: if wake lock was not acquired
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
QDF_STATUS qdf_wake_lock_destroy(qdf_wake_lock_t *lock)
{
	wakeup_source_trash(lock);
	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS qdf_wake_lock_destroy(qdf_wake_lock_t *lock)
{
	return QDF_STATUS_SUCCESS;
}
#endif
EXPORT_SYMBOL(qdf_wake_lock_destroy);

#ifdef CONFIG_MCL
/**
 * qdf_runtime_pm_get() - do a get opperation on the device
 *
 * A get opperation will prevent a runtime suspend untill a
 * corresponding put is done.  This api should be used when sending
 * data.
 *
 * CONTRARY TO THE REGULAR RUNTIME PM, WHEN THE BUS IS SUSPENDED,
 * THIS API WILL ONLY REQUEST THE RESUME AND NOT TO A GET!!!
 *
 * return: success if the bus is up and a get has been issued
 *   otherwise an error code.
 */
QDF_STATUS qdf_runtime_pm_get(void)
{
	void *ol_sc;
	int ret;

	ol_sc = cds_get_context(QDF_MODULE_ID_HIF);

	if (ol_sc == NULL) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: HIF context is null!", __func__);
		return QDF_STATUS_E_INVAL;
	}

	ret = hif_pm_runtime_get(ol_sc);

	if (ret)
		return QDF_STATUS_E_FAILURE;
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_runtime_pm_get);

/**
 * qdf_runtime_pm_put() - do a put opperation on the device
 *
 * A put opperation will allow a runtime suspend after a corresponding
 * get was done.  This api should be used when sending data.
 *
 * This api will return a failure if the hif module hasn't been
 * initialized
 *
 * return: QDF_STATUS_SUCCESS if the put is performed
 */
QDF_STATUS qdf_runtime_pm_put(void)
{
	void *ol_sc;
	int ret;

	ol_sc = cds_get_context(QDF_MODULE_ID_HIF);

	if (ol_sc == NULL) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: HIF context is null!", __func__);
		return QDF_STATUS_E_INVAL;
	}

	ret = hif_pm_runtime_put(ol_sc);

	if (ret)
		return QDF_STATUS_E_FAILURE;
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_runtime_pm_put);

/**
 * qdf_runtime_pm_prevent_suspend() - prevent a runtime bus suspend
 * @lock: an opaque context for tracking
 *
 * The lock can only be acquired once per lock context and is tracked.
 *
 * return: QDF_STATUS_SUCCESS or failure code.
 */
QDF_STATUS qdf_runtime_pm_prevent_suspend(qdf_runtime_lock_t lock)
{
	void *ol_sc;
	int ret;

	ol_sc = cds_get_context(QDF_MODULE_ID_HIF);

	if (ol_sc == NULL) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: HIF context is null!", __func__);
		return QDF_STATUS_E_INVAL;
	}

	ret = hif_pm_runtime_prevent_suspend(ol_sc, lock);

	if (ret)
		return QDF_STATUS_E_FAILURE;
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_runtime_pm_prevent_suspend);

/**
 * qdf_runtime_pm_prevent_suspend() - prevent a runtime bus suspend
 * @lock: an opaque context for tracking
 *
 * The lock can only be acquired once per lock context and is tracked.
 *
 * return: QDF_STATUS_SUCCESS or failure code.
 */
QDF_STATUS qdf_runtime_pm_allow_suspend(qdf_runtime_lock_t lock)
{
	void *ol_sc;
	int ret;
	ol_sc = cds_get_context(QDF_MODULE_ID_HIF);

	if (ol_sc == NULL) {
		QDF_ASSERT(0);
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
		"%s: HIF context is null!", __func__);
		return QDF_STATUS_E_INVAL;
	}

	ret = hif_pm_runtime_allow_suspend(ol_sc, lock);
	if (ret)
		return QDF_STATUS_E_FAILURE;
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_runtime_pm_allow_suspend);

/**
 * qdf_runtime_lock_init() - initialize runtime lock
 * @name: name of the runtime lock
 *
 * Initialize a runtime pm lock.  This lock can be used
 * to prevent the runtime pm system from putting the bus
 * to sleep.
 *
 * Return: runtime_pm_lock_t
 */
qdf_runtime_lock_t qdf_runtime_lock_init(const char *name)
{
	return hif_runtime_lock_init(name);
}
EXPORT_SYMBOL(qdf_runtime_lock_init);

/**
 * qdf_runtime_lock_deinit() - deinitialize runtime pm lock
 * @lock: the lock to deinitialize
 *
 * Ensures the lock is released. Frees the runtime lock.
 *
 * Return: void
 */
void qdf_runtime_lock_deinit(qdf_runtime_lock_t lock)
{
	void *hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	hif_runtime_lock_deinit(hif_ctx, lock);
}
EXPORT_SYMBOL(qdf_runtime_lock_deinit);

#endif /* CONFIG_MCL */

/**
 * qdf_spinlock_acquire() - acquires a spin lock
 * @lock: Spin lock to acquire
 *
 * Return:
 * QDF status success: if wake lock is acquired
 */
QDF_STATUS qdf_spinlock_acquire(qdf_spinlock_t *lock)
{
	spin_lock(&lock->spinlock);
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_spinlock_acquire);


/**
 * qdf_spinlock_release() - release a spin lock
 * @lock: Spin lock to release
 *
 * Return:
 * QDF status success : if wake lock is acquired
 */
QDF_STATUS qdf_spinlock_release(qdf_spinlock_t *lock)
{
	spin_unlock(&lock->spinlock);
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_spinlock_release);

/**
 * qdf_mutex_destroy() - destroy a QDF lock
 * @lock: Pointer to the opaque lock object to be destroyed
 *
 * function shall destroy the lock object referenced by lock. After a
 * successful return from qdf_mutex_destroy()
 * the lock object becomes, in effect, uninitialized.
 *
 * A destroyed lock object can be reinitialized using qdf_mutex_create();
 * the results of otherwise referencing the object after it has been destroyed
 * are undefined.  Calls to QDF lock functions to manipulate the lock such
 * as qdf_mutex_acquire() will fail if the lock is destroyed.  Therefore,
 * don't use the lock after it has been destroyed until it has
 * been re-initialized.
 *
 * Return:
 * QDF_STATUS_SUCCESS: lock was successfully initialized
 * QDF failure reason codes: lock is not initialized and can't be used
 */
QDF_STATUS qdf_mutex_destroy(qdf_mutex_t *lock)
{
	/* check for invalid pointer */
	if (NULL == lock) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed in", __func__);
		return QDF_STATUS_E_FAULT;
	}

	if (LINUX_LOCK_COOKIE != lock->cookie) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: uninitialized lock", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (in_interrupt()) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s cannot be called from interrupt context!!!",
			  __func__);
		return QDF_STATUS_E_FAULT;
	}

	/* check if lock is released */
	if (!mutex_trylock(&lock->m_lock)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: lock is not released", __func__);
		return QDF_STATUS_E_BUSY;
	}
	lock->cookie = 0;
	lock->state = LOCK_DESTROYED;
	lock->process_id = 0;
	lock->refcount = 0;

	mutex_unlock(&lock->m_lock);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_mutex_destroy);

/**
 * qdf_spin_trylock_bh_outline() - spin trylock bottomhalf
 * @lock: spinlock object
 * Retrun: int
 */
int qdf_spin_trylock_bh_outline(qdf_spinlock_t *lock)
{
	return qdf_spin_trylock_bh(lock);
}
EXPORT_SYMBOL(qdf_spin_trylock_bh_outline);

/**
 * qdf_spin_lock_bh_outline() - locks the spinlock in soft irq context
 * @lock: spinlock object pointer
 * Return: none
 */
void qdf_spin_lock_bh_outline(qdf_spinlock_t *lock)
{
	qdf_spin_lock_bh(lock);
}
EXPORT_SYMBOL(qdf_spin_lock_bh_outline);

/**
 * qdf_spin_unlock_bh_outline() - unlocks spinlock in soft irq context
 * @lock: spinlock object pointer
 * Return: none
 */
void qdf_spin_unlock_bh_outline(qdf_spinlock_t *lock)
{
	qdf_spin_unlock_bh(lock);
}
EXPORT_SYMBOL(qdf_spin_unlock_bh_outline);
