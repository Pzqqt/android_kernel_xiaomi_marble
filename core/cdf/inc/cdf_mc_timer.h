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

#if !defined(__CDF_MC_TIMER_H)
#define __CDF_MC_TIMER_H

/**
 * DOC: cdf_mc_timer
 *
 * Connectivity driver framework timer APIs serialized to MC thread
 */

/* Include Files */
#include <qdf_types.h>
#include <qdf_status.h>
#include <qdf_lock.h>
#include <i_cdf_mc_timer.h>

#ifdef TIMER_MANAGER
#include <qdf_list.h>
#endif

/* Preprocessor definitions and constants */
#define CDF_TIMER_STATE_COOKIE (0x12)
#define CDF_MC_TIMER_TO_MS_UNIT   (1000)
#define CDF_MC_TIMER_TO_SEC_UNIT  (1000000)

/* Type declarations */
/* cdf Timer callback function prototype (well, actually a prototype for
   a pointer to this callback function) */
typedef void (*cdf_mc_timer_callback_t)(void *userData);

typedef enum {
	CDF_TIMER_STATE_UNUSED = CDF_TIMER_STATE_COOKIE,
	CDF_TIMER_STATE_STOPPED,
	CDF_TIMER_STATE_STARTING,
	CDF_TIMER_STATE_RUNNING,
} CDF_TIMER_STATE;

#ifdef TIMER_MANAGER
struct cdf_mc_timer_s;
typedef struct cdf_mc_timer_node_s {
	qdf_list_node_t pNode;
	char *fileName;
	unsigned int lineNum;
	struct cdf_mc_timer_s *cdf_timer;
} cdf_mc_timer_node_t;
#endif

typedef struct cdf_mc_timer_s {
#ifdef TIMER_MANAGER
	cdf_mc_timer_node_t *ptimerNode;
#endif

	cdf_mc_timer_platform_t platformInfo;
	cdf_mc_timer_callback_t callback;
	void *userData;
	qdf_mutex_t lock;
	QDF_TIMER_TYPE type;
	CDF_TIMER_STATE state;
} cdf_mc_timer_t;

/* Function declarations and documenation */
#ifdef TIMER_MANAGER
void cdf_mc_timer_manager_init(void);
void cdf_mc_timer_exit(void);
#else
/**
 * cdf_mc_timer_manager_init() - initialize CDF debug timer manager
 *
 * This API initializes CDF timer debug functionality.
 *
 * Return: none
 */
static inline void cdf_mc_timer_manager_init(void)
{
}

/**
 * cdf_mc_timer_exit() - exit CDF timer debug functionality
 *
 * This API exists CDF timer debug functionality
 *
 * Return: none
 */
static inline void cdf_mc_timer_exit(void)
{
}
#endif
/**
 * cdf_mc_timer_get_current_state() - get the current state of the timer
 * @pTimer:  Pointer to timer object
 *
 * Return:
 *	CDF_TIMER_STATE - cdf timer state
 */

CDF_TIMER_STATE cdf_mc_timer_get_current_state(cdf_mc_timer_t *pTimer);

/**
 * cdf_mc_timer_init() - initialize a CDF timer
 * @pTimer:	Pointer to timer object
 * @timerType:	Type of timer
 * @callback:	Callback to be called after timer expiry
 * @serData:	User data which will be passed to callback function
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
#define cdf_mc_timer_init(timer, timerType, callback, userdata)	\
	cdf_mc_timer_init_debug(timer, timerType, callback, userdata, \
		__FILE__, __LINE__)

QDF_STATUS cdf_mc_timer_init_debug(cdf_mc_timer_t *timer,
				   QDF_TIMER_TYPE timerType,
				   cdf_mc_timer_callback_t callback,
				   void *userData, char *fileName,
				   uint32_t lineNum);
#else
QDF_STATUS cdf_mc_timer_init(cdf_mc_timer_t *timer, QDF_TIMER_TYPE timerType,
			     cdf_mc_timer_callback_t callback,
			     void *userData);
#endif

/**
 * cdf_mc_timer_destroy() - destroy CDF timer
 * @timer:	Pointer to timer object
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
QDF_STATUS cdf_mc_timer_destroy(cdf_mc_timer_t *timer);

/**
 * cdf_mc_timer_start() - start a CDF Timer object
 * @timer:	Pointer to timer object
 * @expirationTime:	Time to expire
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
QDF_STATUS cdf_mc_timer_start(cdf_mc_timer_t *timer, uint32_t expirationTime);

/**
 * cdf_mc_timer_stop() - stop a CDF Timer
 * @timer:	Pointer to timer object
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
QDF_STATUS cdf_mc_timer_stop(cdf_mc_timer_t *timer);

/**
 * cdf_mc_timer_get_system_ticks() - get the system time in 10ms ticks

 * cdf_mc_timer_get_system_ticks() function returns the current number
 * of timer ticks in 10msec intervals.  This function is suitable timestamping
 * and calculating time intervals by calculating the difference between two
 * timestamps.
 *
 * Return:
 *	The current system tick count (in 10msec intervals).  This
 *	function cannot fail.
 */
unsigned long cdf_mc_timer_get_system_ticks(void);

/**
 * cdf_mc_timer_get_system_time() - Get the system time in milliseconds
 *
 * cdf_mc_timer_get_system_time() function returns the number of milliseconds
 * that have elapsed since the system was started
 *
 * Return:
 *	The current system time in milliseconds
 */
unsigned long cdf_mc_timer_get_system_time(void);

#endif /* #if !defined __CDF_MC_TIMER_H */
