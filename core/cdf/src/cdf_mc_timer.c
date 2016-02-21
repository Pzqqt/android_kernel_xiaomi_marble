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
 *  DOC: cdf_mc_timer
 *
 *  Connectivity driver framework timer APIs serialized to MC thread
 */

/* Include Files */
#include <cdf_mc_timer.h>
#include <qdf_lock.h>
#include <cds_api.h>
#include "wlan_qct_sys.h"
#include "cds_sched.h"

/* Preprocessor definitions and constants */

#define LINUX_TIMER_COOKIE 0x12341234
#define LINUX_INVALID_TIMER_COOKIE 0xfeedface
#define TMR_INVALID_ID (0)

/* Type declarations */

/* Static Variable Definitions */
static unsigned int persistent_timer_count;
static qdf_mutex_t persistent_timer_count_lock;

/* Function declarations and documenation */

/**
 * try_allowing_sleep() - clean up timer states after it has been deactivated
 * @type: Timer type
 *
 * Clean up timer states after it has been deactivated check and try to allow
 * sleep after a timer has been stopped or expired.
 *
 * Return: none
 */
static void try_allowing_sleep(QDF_TIMER_TYPE type)
{
	if (QDF_TIMER_TYPE_WAKE_APPS == type) {
		/* qdf_mutex_acquire(&persistent_timer_count_lock); */
		persistent_timer_count--;
		if (0 == persistent_timer_count) {
			/* since the number of persistent timers has
			   decreased from 1 to 0, the timer should allow
			   sleep sleep_assert_okts( sleepClientHandle ); */
		}
		/* qdf_mutex_release(&persistent_timer_count_lock); */
	}
}

/**
 * cdf_linux_timer_callback() - internal cdf entry point which is
 *				called when the timer interval expires
 * @data: pointer to the timer control block which describes the
 *	timer that expired
 *
 * This function in turn calls the CDF client callback and changes the
 * state of the timer from running (ACTIVE) to expired (INIT).
 *
 * Note: function signature is defined by the Linux kernel.  The fact
 * that the argument is "unsigned long" instead of "void *" is
 * unfortunately imposed upon us.  But we can safely pass a pointer via
 * this parameter for LP32 and LP64 architectures.
 *
 *  Return: nothing
 */

static void cdf_linux_timer_callback(unsigned long data)
{
	cdf_mc_timer_t *timer = (cdf_mc_timer_t *) data;
	cds_msg_t msg;
	QDF_STATUS vStatus;
	unsigned long flags;

	cdf_mc_timer_callback_t callback = NULL;
	void *userData = NULL;
	int threadId;
	QDF_TIMER_TYPE type = QDF_TIMER_TYPE_SW;

	CDF_ASSERT(timer);

	if (timer == NULL) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s Null pointer passed in!", __func__);
		return;
	}

	threadId = timer->platformInfo.threadID;
	spin_lock_irqsave(&timer->platformInfo.spinlock, flags);

	switch (timer->state) {
	case CDF_TIMER_STATE_STARTING:
		/* we are in this state because someone just started the timer,
		 * MC timer got started and expired, but the time content have
		 * not been updated this is a rare race condition!
		 */
		timer->state = CDF_TIMER_STATE_STOPPED;
		vStatus = QDF_STATUS_E_ALREADY;
		break;

	case CDF_TIMER_STATE_STOPPED:
		vStatus = QDF_STATUS_E_ALREADY;
		break;

	case CDF_TIMER_STATE_UNUSED:
		vStatus = QDF_STATUS_E_EXISTS;
		break;

	case CDF_TIMER_STATE_RUNNING:
		/* need to go to stop state here because the call-back function
		 * may restart timer (to emulate periodic timer)
		 */
		timer->state = CDF_TIMER_STATE_STOPPED;
		/* copy the relevant timer information to local variables;
		 * once we exist from this critical section, the timer content
		 * may be modified by other tasks
		 */
		callback = timer->callback;
		userData = timer->userData;
		threadId = timer->platformInfo.threadID;
		type = timer->type;
		vStatus = QDF_STATUS_SUCCESS;
		break;

	default:
		CDF_ASSERT(0);
		vStatus = QDF_STATUS_E_FAULT;
		break;
	}

	spin_unlock_irqrestore(&timer->platformInfo.spinlock, flags);

	if (QDF_STATUS_SUCCESS != vStatus) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "TIMER callback called in a wrong state=%d",
			  timer->state);
		return;
	}

	try_allowing_sleep(type);

	if (callback == NULL) {
		CDF_ASSERT(0);
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: No TIMER callback, Could not enqueue timer to any queue",
			  __func__);
		return;
	}
	CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_INFO,
		  "TIMER callback: running on MC thread");

	/* serialize to the MC thread */
	sys_build_message_header(SYS_MSG_ID_MC_TIMER, &msg);
	msg.callback = callback;
	msg.bodyptr = userData;
	msg.bodyval = 0;

	if (cds_mq_post_message(CDS_MQ_ID_SYS, &msg) == QDF_STATUS_SUCCESS)
		return;

	CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
		  "%s: Could not enqueue timer to any queue", __func__);
	CDF_ASSERT(0);
}

/**
 * cdf_mc_timer_get_current_state() - get the current state of the timer
 * @pTimer: Pointer to timer object
 *
 * Return:
 *	CDF_TIMER_STATE - cdf timer state
 */
CDF_TIMER_STATE cdf_mc_timer_get_current_state(cdf_mc_timer_t *pTimer)
{
	if (NULL == pTimer) {
		CDF_ASSERT(0);
		return CDF_TIMER_STATE_UNUSED;
	}

	switch (pTimer->state) {
	case CDF_TIMER_STATE_STOPPED:
	case CDF_TIMER_STATE_STARTING:
	case CDF_TIMER_STATE_RUNNING:
	case CDF_TIMER_STATE_UNUSED:
		return pTimer->state;
	default:
		CDF_ASSERT(0);
		return CDF_TIMER_STATE_UNUSED;
	}
}

/**
 * cdf_timer_module_init() - initializes a CDF timer module.
 *
 * This API initializes the CDF timer module. This needs to be called
 * exactly once prior to using any CDF timers.
 *
 * Return: none
 */
void cdf_timer_module_init(void)
{
	CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_INFO,
		  "Initializing the CDF timer module");
	qdf_mutex_create(&persistent_timer_count_lock);
}

#ifdef TIMER_MANAGER

qdf_list_t cdf_timer_list;
qdf_spinlock_t cdf_timer_list_lock;

static void cdf_timer_clean(void);

/**
 * cdf_mc_timer_manager_init() - initialize CDF debug timer manager
 *
 * This API initializes CDF timer debug functionality.
 *
 * Return: none
 */
void cdf_mc_timer_manager_init(void)
{
	qdf_list_create(&cdf_timer_list, 1000);
	qdf_spinlock_create(&cdf_timer_list_lock);
	return;
}

/**
 * cdf_timer_clean() - clean up CDF timer debug functionality
 *
 * This API cleans up CDF timer debug functionality and prints which CDF timers
 * are leaked. This is called during driver unload.
 *
 * Return: none
 */
static void cdf_timer_clean(void)
{
	uint32_t listSize;

	listSize = qdf_list_size(&cdf_timer_list);

	if (listSize) {
		qdf_list_node_t *pNode;
		QDF_STATUS qdf_status;

		cdf_mc_timer_node_t *ptimerNode;
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: List is not Empty. listSize %d ",
			  __func__, (int)listSize);

		do {
			qdf_spin_lock_irqsave(&cdf_timer_list_lock);
			qdf_status =
				qdf_list_remove_front(&cdf_timer_list, &pNode);
			qdf_spin_unlock_irqrestore(&cdf_timer_list_lock);
			if (QDF_STATUS_SUCCESS == qdf_status) {
				ptimerNode = (cdf_mc_timer_node_t *) pNode;
				CDF_TRACE(QDF_MODULE_ID_QDF,
					  CDF_TRACE_LEVEL_FATAL,
					  "Timer Leak@ File %s, @Line %d",
					  ptimerNode->fileName,
					  (int)ptimerNode->lineNum);
				cdf_mem_free(ptimerNode);
			}
		} while (qdf_status == QDF_STATUS_SUCCESS);
	}
}

/**
 * cdf_mc_timer_exit() - exit CDF timer debug functionality
 *
 * This API exists CDF timer debug functionality
 *
 * Return: none
 */
void cdf_mc_timer_exit(void)
{
	cdf_timer_clean();
	qdf_list_destroy(&cdf_timer_list);
}
#endif

/**
 * cdf_mc_timer_init() - initialize a CDF timer
 * @pTimer:     Pointer to timer object
 * @timerType:  Type of timer
 * @callback:   Callback to be called after timer expiry
 * @serData:    User data which will be passed to callback function
 *
 * This API initializes a CDF Timer object.
 *
 * cdf_mc_timer_init() initializes a CDF Timer object.  A timer must be
 * initialized by calling cdf_mc_timer_initialize() before it may be used in
 * any other timer functions.
 *
 * Attempting to initialize timer that is already initialized results in
 * a failure. A destroyed timer object can be re-initialized with a call to
 * cdf_mc_timer_init().  The results of otherwise referencing the object
 * after it has been destroyed are undefined.
 *
 *  Calls to CDF timer functions to manipulate the timer such
 *  as cdf_mc_timer_set() will fail if the timer is not initialized or has
 *  been destroyed.  Therefore, don't use the timer after it has been
 *  destroyed until it has been re-initialized.
 *
 *  All callback will be executed within the CDS main thread unless it is
 *  initialized from the Tx thread flow, in which case it will be executed
 *  within the tx thread flow.
 *
 * Return:
 *	QDF_STATUS_SUCCESS - Timer is initialized successfully
 *	CDF failure status - Timer initialization failed
 */
#ifdef TIMER_MANAGER
QDF_STATUS cdf_mc_timer_init_debug(cdf_mc_timer_t *timer,
				   QDF_TIMER_TYPE timerType,
				   cdf_mc_timer_callback_t callback,
				   void *userData, char *fileName,
				   uint32_t lineNum)
{
	QDF_STATUS qdf_status;

	/* check for invalid pointer */
	if ((timer == NULL) || (callback == NULL)) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Null params being passed", __func__);
		CDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	timer->ptimerNode = cdf_mem_malloc(sizeof(cdf_mc_timer_node_t));

	if (timer->ptimerNode == NULL) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Not able to allocate memory for timeNode",
			  __func__);
		CDF_ASSERT(0);
		return QDF_STATUS_E_NOMEM;
	}

	cdf_mem_set(timer->ptimerNode, sizeof(cdf_mc_timer_node_t), 0);

	timer->ptimerNode->fileName = fileName;
	timer->ptimerNode->lineNum = lineNum;
	timer->ptimerNode->cdf_timer = timer;

	qdf_spin_lock_irqsave(&cdf_timer_list_lock);
	qdf_status = qdf_list_insert_front(&cdf_timer_list,
					   &timer->ptimerNode->pNode);
	qdf_spin_unlock_irqrestore(&cdf_timer_list_lock);
	if (QDF_STATUS_SUCCESS != qdf_status) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Unable to insert node into List qdf_status %d",
			  __func__, qdf_status);
	}

	/* set the various members of the timer structure
	 * with arguments passed or with default values
	 */
	spin_lock_init(&timer->platformInfo.spinlock);
	if (QDF_TIMER_TYPE_SW == timerType)
		init_timer_deferrable(&(timer->platformInfo.Timer));
	else
		init_timer(&(timer->platformInfo.Timer));
	timer->platformInfo.Timer.function = cdf_linux_timer_callback;
	timer->platformInfo.Timer.data = (unsigned long)timer;
	timer->callback = callback;
	timer->userData = userData;
	timer->type = timerType;
	timer->platformInfo.cookie = LINUX_TIMER_COOKIE;
	timer->platformInfo.threadID = 0;
	timer->state = CDF_TIMER_STATE_STOPPED;

	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS cdf_mc_timer_init(cdf_mc_timer_t *timer, QDF_TIMER_TYPE timerType,
			     cdf_mc_timer_callback_t callback,
			     void *userData)
{
	/* check for invalid pointer */
	if ((timer == NULL) || (callback == NULL)) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Null params being passed", __func__);
		CDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	/* set the various members of the timer structure
	 * with arguments passed or with default values
	 */
	spin_lock_init(&timer->platformInfo.spinlock);
	if (QDF_TIMER_TYPE_SW == timerType)
		init_timer_deferrable(&(timer->platformInfo.Timer));
	else
		init_timer(&(timer->platformInfo.Timer));
	timer->platformInfo.Timer.function = cdf_linux_timer_callback;
	timer->platformInfo.Timer.data = (unsigned long)timer;
	timer->callback = callback;
	timer->userData = userData;
	timer->type = timerType;
	timer->platformInfo.cookie = LINUX_TIMER_COOKIE;
	timer->platformInfo.threadID = 0;
	timer->state = CDF_TIMER_STATE_STOPPED;

	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * cdf_mc_timer_destroy() - destroy CDF timer
 * @timer: Pointer to timer object
 *
 * cdf_mc_timer_destroy() function shall destroy the timer object.
 * After a successful return from \a cdf_mc_timer_destroy() the timer
 * object becomes, in effect, uninitialized.
 *
 * A destroyed timer object can be re-initialized by calling
 * cdf_mc_timer_init().  The results of otherwise referencing the object
 * after it has been destroyed are undefined.
 *
 * Calls to CDF timer functions to manipulate the timer, such
 * as cdf_mc_timer_set() will fail if the lock is destroyed.  Therefore,
 * don't use the timer after it has been destroyed until it has
 * been re-initialized.
 *
 * Return:
 *	QDF_STATUS_SUCCESS - Timer is initialized successfully
 *	CDF failure status - Timer initialization failed
 */
#ifdef TIMER_MANAGER
QDF_STATUS cdf_mc_timer_destroy(cdf_mc_timer_t *timer)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	unsigned long flags;

	/* check for invalid pointer */
	if (NULL == timer) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Null timer pointer being passed", __func__);
		CDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	/* Check if timer refers to an uninitialized object */
	if (LINUX_TIMER_COOKIE != timer->platformInfo.cookie) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Cannot destroy uninitialized timer", __func__);
		CDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	qdf_spin_lock_irqsave(&cdf_timer_list_lock);
	status = qdf_list_remove_node(&cdf_timer_list,
				       &timer->ptimerNode->pNode);
	qdf_spin_unlock_irqrestore(&cdf_timer_list_lock);
	if (status != QDF_STATUS_SUCCESS) {
		CDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}
	cdf_mem_free(timer->ptimerNode);

	spin_lock_irqsave(&timer->platformInfo.spinlock, flags);

	switch (timer->state) {

	case CDF_TIMER_STATE_STARTING:
		status = QDF_STATUS_E_BUSY;
		break;

	case CDF_TIMER_STATE_RUNNING:
		/* Stop the timer first */
		del_timer(&(timer->platformInfo.Timer));
		status = QDF_STATUS_SUCCESS;
		break;
	case CDF_TIMER_STATE_STOPPED:
		status = QDF_STATUS_SUCCESS;
		break;

	case CDF_TIMER_STATE_UNUSED:
		status = QDF_STATUS_E_ALREADY;
		break;

	default:
		status = QDF_STATUS_E_FAULT;
		break;
	}

	if (QDF_STATUS_SUCCESS == status) {
		timer->platformInfo.cookie = LINUX_INVALID_TIMER_COOKIE;
		timer->state = CDF_TIMER_STATE_UNUSED;
		spin_unlock_irqrestore(&timer->platformInfo.spinlock, flags);
		return status;
	}

	spin_unlock_irqrestore(&timer->platformInfo.spinlock, flags);

	CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
		  "%s: Cannot destroy timer in state = %d", __func__,
		  timer->state);
	CDF_ASSERT(0);

	return status;
}

#else

/**
 * cdf_mc_timer_destroy() - destroy CDF timer
 * @timer: Pointer to timer object
 *
 * cdf_mc_timer_destroy() function shall destroy the timer object.
 * After a successful return from \a cdf_mc_timer_destroy() the timer
 * object becomes, in effect, uninitialized.
 *
 * A destroyed timer object can be re-initialized by calling
 * cdf_mc_timer_init().  The results of otherwise referencing the object
 * after it has been destroyed are undefined.
 *
 * Calls to CDF timer functions to manipulate the timer, such
 * as cdf_mc_timer_set() will fail if the lock is destroyed.  Therefore,
 * don't use the timer after it has been destroyed until it has
 * been re-initialized.
 *
 * Return:
 *      QDF_STATUS_SUCCESS - Timer is initialized successfully
 *      CDF failure status - Timer initialization failed
 */
QDF_STATUS cdf_mc_timer_destroy(cdf_mc_timer_t *timer)
{
	QDF_STATUS vStatus = QDF_STATUS_SUCCESS;
	unsigned long flags;

	/* check for invalid pointer */
	if (NULL == timer) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Null timer pointer being passed", __func__);
		CDF_ASSERT(0);
		return QDF_STATUS_E_FAULT;
	}

	/* check if timer refers to an uninitialized object */
	if (LINUX_TIMER_COOKIE != timer->platformInfo.cookie) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Cannot destroy uninitialized timer", __func__);
		CDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}
	spin_lock_irqsave(&timer->platformInfo.spinlock, flags);

	switch (timer->state) {

	case CDF_TIMER_STATE_STARTING:
		vStatus = QDF_STATUS_E_BUSY;
		break;

	case CDF_TIMER_STATE_RUNNING:
		/* Stop the timer first */
		del_timer(&(timer->platformInfo.Timer));
		vStatus = QDF_STATUS_SUCCESS;
		break;

	case CDF_TIMER_STATE_STOPPED:
		vStatus = QDF_STATUS_SUCCESS;
		break;

	case CDF_TIMER_STATE_UNUSED:
		vStatus = QDF_STATUS_E_ALREADY;
		break;

	default:
		vStatus = QDF_STATUS_E_FAULT;
		break;
	}

	if (QDF_STATUS_SUCCESS == vStatus) {
		timer->platformInfo.cookie = LINUX_INVALID_TIMER_COOKIE;
		timer->state = CDF_TIMER_STATE_UNUSED;
		spin_unlock_irqrestore(&timer->platformInfo.spinlock, flags);
		return vStatus;
	}

	spin_unlock_irqrestore(&timer->platformInfo.spinlock, flags);

	CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
		  "%s: Cannot destroy timer in state = %d", __func__,
		  timer->state);
	CDF_ASSERT(0);

	return vStatus;
}
#endif

/**
 * cdf_mc_timer_start() - start a CDF Timer object
 * @timer:      Pointer to timer object
 * @expirationTime:     Time to expire
 *
 * cdf_mc_timer_start() function starts a timer to expire after the
 * specified interval, thus running the timer callback function when
 * the interval expires.
 *
 * A timer only runs once (a one-shot timer).  To re-start the
 * timer, cdf_mc_timer_start() has to be called after the timer runs
 * or has been cancelled.
 *
 * Return:
 *	QDF_STATUS_SUCCESS - Timer is initialized successfully
 *	CDF failure status - Timer initialization failed
 */
QDF_STATUS cdf_mc_timer_start(cdf_mc_timer_t *timer, uint32_t expirationTime)
{
	unsigned long flags;

	CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_INFO_HIGH,
		  "Timer Addr inside cds_enable : 0x%p ", timer);

	/* check for invalid pointer */
	if (NULL == timer) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s Null timer pointer being passed", __func__);
		CDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	/* check if timer refers to an uninitialized object */
	if (LINUX_TIMER_COOKIE != timer->platformInfo.cookie) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Cannot start uninitialized timer", __func__);
		CDF_ASSERT(0);

		return QDF_STATUS_E_INVAL;
	}

	/* check if timer has expiration time less than 10 ms */
	if (expirationTime < 10) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Cannot start a timer with expiration less than 10 ms",
			  __func__);
		CDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	/* make sure the remainer of the logic isn't interrupted */
	spin_lock_irqsave(&timer->platformInfo.spinlock, flags);

	/* ensure if the timer can be started */
	if (CDF_TIMER_STATE_STOPPED != timer->state) {
		spin_unlock_irqrestore(&timer->platformInfo.spinlock, flags);
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Cannot start timer in state = %d ", __func__,
			  timer->state);
		return QDF_STATUS_E_ALREADY;
	}

	/* start the timer */
	mod_timer(&(timer->platformInfo.Timer),
		  jiffies + msecs_to_jiffies(expirationTime));

	timer->state = CDF_TIMER_STATE_RUNNING;

	/* get the thread ID on which the timer is being started */
	timer->platformInfo.threadID = current->pid;

	if (QDF_TIMER_TYPE_WAKE_APPS == timer->type) {
		persistent_timer_count++;
		if (1 == persistent_timer_count) {
			/* since we now have one persistent timer,
			 * we need to disallow sleep
			 * sleep_negate_okts(sleepClientHandle);
			 */
		}
	}

	spin_unlock_irqrestore(&timer->platformInfo.spinlock, flags);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdf_mc_timer_stop() - stop a CDF Timer
 * @timer:      Pointer to timer object
 * cdf_mc_timer_stop() function stops a timer that has been started but
 * has not expired, essentially cancelling the 'start' request.
 *
 * After a timer is stopped, it goes back to the state it was in after it
 * was created and can be started again via a call to cdf_mc_timer_start().
 *
 * Return:
 *	QDF_STATUS_SUCCESS - Timer is initialized successfully
 *	CDF failure status - Timer initialization failed
 */
QDF_STATUS cdf_mc_timer_stop(cdf_mc_timer_t *timer)
{
	unsigned long flags;

	CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: Timer Addr inside cds_disable : 0x%p", __func__, timer);

	/* check for invalid pointer */
	if (NULL == timer) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s Null timer pointer being passed", __func__);
		CDF_ASSERT(0);
		return QDF_STATUS_E_INVAL;
	}

	/* check if timer refers to an uninitialized object */
	if (LINUX_TIMER_COOKIE != timer->platformInfo.cookie) {
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_ERROR,
			  "%s: Cannot stop uninitialized timer", __func__);
		CDF_ASSERT(0);

		return QDF_STATUS_E_INVAL;
	}

	/* ensure the timer state is correct */
	spin_lock_irqsave(&timer->platformInfo.spinlock, flags);

	if (CDF_TIMER_STATE_RUNNING != timer->state) {
		spin_unlock_irqrestore(&timer->platformInfo.spinlock, flags);
		CDF_TRACE(QDF_MODULE_ID_QDF, CDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Cannot stop timer in state = %d",
			  __func__, timer->state);
		return QDF_STATUS_SUCCESS;
	}

	timer->state = CDF_TIMER_STATE_STOPPED;

	del_timer(&(timer->platformInfo.Timer));

	spin_unlock_irqrestore(&timer->platformInfo.spinlock, flags);

	try_allowing_sleep(timer->type);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdf_mc_timer_get_system_ticks() - get the system time in 10ms ticks

 * cdf_mc_timer_get_system_ticks() function returns the current number
 * of timer ticks in 10msec intervals. This function is suitable timestamping
 * and calculating time intervals by calculating the difference between two
 * timestamps.
 *
 * Return:
 *	The current system tick count (in 10msec intervals).  This
 *	function cannot fail.
 */
unsigned long cdf_mc_timer_get_system_ticks(void)
{
	return jiffies_to_msecs(jiffies) / 10;
}

/**
 * cdf_mc_timer_get_system_time() - Get the system time in milliseconds
 *
 * cdf_mc_timer_get_system_time() function returns the number of milliseconds
 * that have elapsed since the system was started
 *
 * Return:
 *	The current system time in milliseconds
 */
unsigned long cdf_mc_timer_get_system_time(void)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
