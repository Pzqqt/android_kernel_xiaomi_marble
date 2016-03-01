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
 * DOC: cdf_lock.c
 *
 * OVERVIEW: This source file contains definitions for CDF lock APIs
 *	     The four APIs mentioned in this file are used for
 *	     initializing, acquiring, releasing and destroying a lock.
 *	     the lock are implemented using critical sections
 */

/* Include Files */

#include "cdf_lock.h"
#include "cdf_memory.h"
#include "cdf_trace.h"
#include <cdf_types.h>
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#endif
#include "i_host_diag_core_event.h"
#include "cds_api.h"
#include "ani_global.h"
#include "hif.h"

/* Preprocessor Definitions and Constants */
#define LINUX_LOCK_COOKIE 0x12345678

#define WIFI_POWER_EVENT_DEFAULT_WAKELOCK_TIMEOUT 0
#define WIFI_POWER_EVENT_WAKELOCK_TAKEN 0
#define WIFI_POWER_EVENT_WAKELOCK_RELEASED 1

/* Type Declarations */

enum {
	LOCK_RELEASED = 0x11223344,
	LOCK_ACQUIRED,
	LOCK_DESTROYED
};

/* Global Data Definitions */

/* Function Definitions and Documentation */

/**
 * cdf_mutex_init() - initialize a CDF lock
 * @lock:        Pointer to the opaque lock object to initialize
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
 *      CDF_STATUS_SUCCESS:     lock was successfully initialized
 *      CDF failure reason codes: lock is not initialized and can't be used
 */
CDF_STATUS cdf_mutex_init(cdf_mutex_t *lock)
{
	/* check for invalid pointer */
	if (lock == NULL) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed in", __func__);
		return CDF_STATUS_E_FAULT;
	}
	/* check for 'already initialized' lock */
	if (LINUX_LOCK_COOKIE == lock->cookie) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: already initialized lock", __func__);
		return CDF_STATUS_E_BUSY;
	}

	if (in_interrupt()) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s cannot be called from interrupt context!!!",
			  __func__);
		return CDF_STATUS_E_FAULT;
	}

	/* initialize new lock */
	mutex_init(&lock->m_lock);
	lock->cookie = LINUX_LOCK_COOKIE;
	lock->state = LOCK_RELEASED;
	lock->processID = 0;
	lock->refcount = 0;

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_mutex_acquire() - acquire a CDF lock
 * @lock:        Pointer to the opaque lock object to acquire
 *
 * A lock object is acquired by calling cdf_mutex_acquire().  If the lock
 * is already locked, the calling thread shall block until the lock becomes
 * available. This operation shall return with the lock object referenced by
 * lock in the locked state with the calling thread as its owner.
 *
 * Return:
 *      CDF_STATUS_SUCCESS:     lock was successfully initialized
 *      CDF failure reason codes: lock is not initialized and can't be used
 */
CDF_STATUS cdf_mutex_acquire(cdf_mutex_t *lock)
{
	int rc;
	/* check for invalid pointer */
	if (lock == NULL) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed in", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAULT;
	}
	/* check if lock refers to an initialized object */
	if (LINUX_LOCK_COOKIE != lock->cookie) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: uninitialized lock", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_INVAL;
	}

	if (in_interrupt()) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s cannot be called from interrupt context!!!",
			  __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAULT;
	}
	if ((lock->processID == current->pid) &&
		(lock->state == LOCK_ACQUIRED)) {
		lock->refcount++;
#ifdef CDF_NESTED_LOCK_DEBUG
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_INFO,
			  "%s: %x %d %d", __func__, lock, current->pid,
			  lock->refcount);
#endif
		return CDF_STATUS_SUCCESS;
	}
	/* acquire a Lock */
	mutex_lock(&lock->m_lock);
	rc = mutex_is_locked(&lock->m_lock);
	if (rc == 0) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: unable to lock mutex (rc = %d)", __func__, rc);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAILURE;
	}
#ifdef CDF_NESTED_LOCK_DEBUG
	CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_INFO,
		  "%s: %x %d", __func__, lock, current->pid);
#endif
	if (LOCK_DESTROYED != lock->state) {
		lock->processID = current->pid;
		lock->refcount++;
		lock->state = LOCK_ACQUIRED;
		return CDF_STATUS_SUCCESS;
	} else {
		/* lock is already destroyed */
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Lock is already destroyed", __func__);
		mutex_unlock(&lock->m_lock);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAILURE;
	}
}

/**
 * cdf_mutex_release() - release a CDF lock
 * @lock:        Pointer to the opaque lock object to be released
 *
 * cdf_mutex_release() function shall release the lock object
 * referenced by 'lock'.
 *
 * If a thread attempts to release a lock that it unlocked or is not
 * initialized, an error is returned.
 *
 * Return:
 *      CDF_STATUS_SUCCESS:     lock was successfully initialized
 *      CDF failure reason codes: lock is not initialized and can't be used
 */
CDF_STATUS cdf_mutex_release(cdf_mutex_t *lock)
{
	/* check for invalid pointer */
	if (lock == NULL) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed in", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAULT;
	}

	/* check if lock refers to an uninitialized object */
	if (LINUX_LOCK_COOKIE != lock->cookie) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: uninitialized lock", __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_INVAL;
	}

	if (in_interrupt()) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s cannot be called from interrupt context!!!",
			  __func__);
		CDF_ASSERT(0);
		return CDF_STATUS_E_FAULT;
	}

	/* CurrentThread = GetCurrentThreadId();
	 * Check thread ID of caller against thread ID
	 * of the thread which acquire the lock
	 */
	if (lock->processID != current->pid) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: current task pid does not match original task pid!!",
			  __func__);
#ifdef CDF_NESTED_LOCK_DEBUG
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_INFO,
			  "%s: Lock held by=%d being released by=%d",
			  __func__, lock->processID, current->pid);
#endif
		CDF_ASSERT(0);
		return CDF_STATUS_E_PERM;
	}
	if ((lock->processID == current->pid) &&
		(lock->state == LOCK_ACQUIRED)) {
		if (lock->refcount > 0)
			lock->refcount--;
	}
#ifdef CDF_NESTED_LOCK_DEBUG
	CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_INFO,
		  "%s: %x %d %d", __func__, lock, lock->processID,
		  lock->refcount);
#endif
	if (lock->refcount)
		return CDF_STATUS_SUCCESS;

	lock->processID = 0;
	lock->refcount = 0;
	lock->state = LOCK_RELEASED;
	/* release a Lock */
	mutex_unlock(&lock->m_lock);
#ifdef CDF_NESTED_LOCK_DEBUG
	CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_INFO,
		  "%s: Freeing lock %x %d %d", lock, lock->processID,
		  lock->refcount);
#endif
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_mutex_destroy() - destroy a CDF lock
 * @lock:        Pointer to the opaque lock object to be destroyed
 *
 * cdf_mutex_destroy() function shall destroy the lock object
 * referenced by lock.  After a successful return from cdf_mutex_destroy()
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
 *      CDF_STATUS_SUCCESS:     lock was successfully initialized
 *      CDF failure reason codes: lock is not initialized and can't be used
 */
CDF_STATUS cdf_mutex_destroy(cdf_mutex_t *lock)
{
	/* check for invalid pointer */
	if (NULL == lock) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: NULL pointer passed in", __func__);
		return CDF_STATUS_E_FAULT;
	}

	if (LINUX_LOCK_COOKIE != lock->cookie) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: uninitialized lock", __func__);
		return CDF_STATUS_E_INVAL;
	}

	if (in_interrupt()) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s cannot be called from interrupt context!!!",
			  __func__);
		return CDF_STATUS_E_FAULT;
	}

	/* check if lock is released */
	if (!mutex_trylock(&lock->m_lock)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: lock is not released", __func__);
		return CDF_STATUS_E_BUSY;
	}
	lock->cookie = 0;
	lock->state = LOCK_DESTROYED;
	lock->processID = 0;
	lock->refcount = 0;

	mutex_unlock(&lock->m_lock);

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_spinlock_acquire() - acquires a spin lock
 * @pLock:       Spin lock to acquire
 *
 * Return:
 *    CDF status success : if wake lock is acquired
 *    CDF status failure : if wake lock was not acquired
 */
CDF_STATUS cdf_spinlock_acquire(cdf_spinlock_t *pLock)
{
	spin_lock(&pLock->spinlock);
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_spinlock_release() - release a spin lock
 * @pLock:       Spin lock to release
 *
 * Return:
 * CDF status success : if wake lock is acquired
 * CDF status failure : if wake lock was not acquired
 */
CDF_STATUS cdf_spinlock_release(cdf_spinlock_t *pLock)
{
	spin_unlock(&pLock->spinlock);
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_wake_lock_name() - This function returns the name of the wakelock
 * @pLock: Pointer to the wakelock
 *
 * This function returns the name of the wakelock
 *
 * Return: Pointer to the name if it is valid or a default string
 *
 */
static const char *cdf_wake_lock_name(cdf_wake_lock_t *pLock)
{
#if defined CONFIG_CNSS
	if (pLock->name)
		return pLock->name;
#elif defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
	if (pLock->ws.name)
		return pLock->ws.name;
#endif
	return "UNNAMED_WAKELOCK";
}

/**
 * cdf_wake_lock_init() - initializes a CDF wake lock
 * @pLock: The wake lock to initialize
 * @name: Name of wake lock
 *
 * Return:
 *    CDF status success : if wake lock is initialized
 *    CDF status failure : if wake lock was not initialized
 */
CDF_STATUS cdf_wake_lock_init(cdf_wake_lock_t *pLock, const char *name)
{
#if defined CONFIG_CNSS
	cnss_pm_wake_lock_init(pLock, name);
#elif defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
	wake_lock_init(pLock, WAKE_LOCK_SUSPEND, name);
#endif
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_wake_lock_acquire() - acquires a wake lock
 * @pLock:       The wake lock to acquire
 * @reason:      Reason for wakelock
 *
 * Return:
 *    CDF status success : if wake lock is acquired
 *    CDF status failure : if wake lock was not acquired
 */
CDF_STATUS cdf_wake_lock_acquire(cdf_wake_lock_t *pLock, uint32_t reason)
{
	host_diag_log_wlock(reason, cdf_wake_lock_name(pLock),
			WIFI_POWER_EVENT_DEFAULT_WAKELOCK_TIMEOUT,
			WIFI_POWER_EVENT_WAKELOCK_TAKEN);
#if defined CONFIG_CNSS
	cnss_pm_wake_lock(pLock);
#elif defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
	wake_lock(pLock);
#endif
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_wake_lock_timeout_acquire() - acquires a wake lock with a timeout
 * @pLock:       The wake lock to acquire
 * @reason:      Reason for wakelock
 *
 * Return:
 *   CDF status success : if wake lock is acquired
 *   CDF status failure : if wake lock was not acquired
 */
CDF_STATUS cdf_wake_lock_timeout_acquire(cdf_wake_lock_t *pLock, uint32_t msec,
					 uint32_t reason)
{
	/* Wakelock for Rx is frequent.
	 * It is reported only during active debug
	 */
	if (((cds_get_ring_log_level(RING_ID_WAKELOCK) >= WLAN_LOG_LEVEL_ACTIVE)
			&& (WIFI_POWER_EVENT_WAKELOCK_HOLD_RX == reason)) ||
			(WIFI_POWER_EVENT_WAKELOCK_HOLD_RX != reason)) {
		host_diag_log_wlock(reason, cdf_wake_lock_name(pLock), msec,
				WIFI_POWER_EVENT_WAKELOCK_TAKEN);
	}
#if defined CONFIG_CNSS
	cnss_pm_wake_lock_timeout(pLock, msec);
#elif defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
	wake_lock_timeout(pLock, msecs_to_jiffies(msec));
#endif
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_wake_lock_release() - releases a wake lock
 * @pLock:       the wake lock to release
 * @reason:      Reason for wakelock
 *
 * Return:
 *    CDF status success : if wake lock is acquired
 *    CDF status failure : if wake lock was not acquired
 */
CDF_STATUS cdf_wake_lock_release(cdf_wake_lock_t *pLock, uint32_t reason)
{
	host_diag_log_wlock(reason, cdf_wake_lock_name(pLock),
			WIFI_POWER_EVENT_DEFAULT_WAKELOCK_TIMEOUT,
			WIFI_POWER_EVENT_WAKELOCK_RELEASED);
#if defined CONFIG_CNSS
	cnss_pm_wake_lock_release(pLock);
#elif defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
	wake_unlock(pLock);
#endif
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_wake_lock_destroy() - destroys a wake lock
 * @pLock:       The wake lock to destroy
 *
 * Return:
 * CDF status success : if wake lock is acquired
 * CDF status failure : if wake lock was not acquired
 */
CDF_STATUS cdf_wake_lock_destroy(cdf_wake_lock_t *pLock)
{
#if defined CONFIG_CNSS
	cnss_pm_wake_lock_destroy(pLock);
#elif defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
	wake_lock_destroy(pLock);
#endif
	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_runtime_pm_get() - do a get opperation on the device
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
CDF_STATUS cdf_runtime_pm_get(void)
{
	void *ol_sc;
	int ret;

	ol_sc = cds_get_context(CDF_MODULE_ID_HIF);

	if (ol_sc == NULL) {
		CDF_ASSERT(0);
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
				"%s: HIF context is null!", __func__);
		return CDF_STATUS_E_INVAL;
	}

	ret = hif_pm_runtime_get(ol_sc);

	if (ret)
		return CDF_STATUS_E_FAILURE;

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_runtime_pm_put() - do a put opperation on the device
 *
 * A put opperation will allow a runtime suspend after a corresponding
 * get was done.  This api should be used when sending data.
 *
 * This api will return a failure if the hif module hasn't been initialized
 *
 * return: CDF_STATUS_SUCCESS if the put is performed
 */
CDF_STATUS cdf_runtime_pm_put(void)
{
	void *ol_sc;
	int ret;

	ol_sc = cds_get_context(CDF_MODULE_ID_HIF);

	if (ol_sc == NULL) {
		CDF_ASSERT(0);
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
				"%s: HIF context is null!", __func__);
		return CDF_STATUS_E_INVAL;
	}

	ret = hif_pm_runtime_put(ol_sc);

	if (ret)
		return CDF_STATUS_E_FAILURE;

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_runtime_pm_prevent_suspend() - prevent a runtime bus suspend
 * @lock: an opaque context for tracking
 *
 * The lock can only be acquired once per lock context and is tracked.
 *
 * return: CDF_STATUS_SUCCESS or failure code.
 */
CDF_STATUS cdf_runtime_pm_prevent_suspend(cdf_runtime_lock_t lock)
{
	void *ol_sc;
	int ret;

	ol_sc = cds_get_context(CDF_MODULE_ID_HIF);

	if (ol_sc == NULL) {
		CDF_ASSERT(0);
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
				"%s: HIF context is null!", __func__);
		return CDF_STATUS_E_INVAL;
	}

	ret = hif_pm_runtime_prevent_suspend(ol_sc, lock);

	if (ret)
		return CDF_STATUS_E_FAILURE;

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_runtime_pm_prevent_suspend() - prevent a runtime bus suspend
 * @lock: an opaque context for tracking
 *
 * The lock can only be acquired once per lock context and is tracked.
 *
 * return: CDF_STATUS_SUCCESS or failure code.
 */
CDF_STATUS cdf_runtime_pm_allow_suspend(cdf_runtime_lock_t lock)
{
	void *ol_sc;
	int ret;

	ol_sc = cds_get_context(CDF_MODULE_ID_HIF);

	if (ol_sc == NULL) {
		CDF_ASSERT(0);
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
				"%s: HIF context is null!", __func__);
		return CDF_STATUS_E_INVAL;
	}

	ret = hif_pm_runtime_allow_suspend(ol_sc, lock);

	if (ret)
		return CDF_STATUS_E_FAILURE;

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_runtime_lock_init() - initialize runtime lock
 * @name: name of the runtime lock
 *
 * Initialize a runtime pm lock.  This lock can be used
 * to prevent the runtime pm system from putting the bus
 * to sleep.
 *
 * Return: runtime_pm_lock_t
 */
cdf_runtime_lock_t cdf_runtime_lock_init(const char *name)
{
	return hif_runtime_lock_init(name);
}

/**
 * cdf_runtime_lock_deinit() - deinitialize runtime pm lock
 * @lock: the lock to deinitialize
 *
 * Ensures the lock is released. Frees the runtime lock.
 *
 * Return: void
 */
void cdf_runtime_lock_deinit(cdf_runtime_lock_t lock)
{
	hif_runtime_lock_deinit(lock);
}
