/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_mc_timer
 * QCA driver framework timer APIs serialized to MC thread
 */

#if !defined(__QDF_MC_TIMER_H)
#define __QDF_MC_TIMER_H

/* Include Files */
#include <qdf_types.h>
#include <qdf_status.h>
#include <qdf_lock.h>
#include <i_qdf_mc_timer.h>

#ifdef TIMER_MANAGER
#include <qdf_list.h>
#endif

/* Preprocessor definitions and constants */
#define QDF_TIMER_STATE_COOKIE (0x12)
#define QDF_MC_TIMER_TO_MS_UNIT (1000)
#define QDF_MC_TIMER_TO_SEC_UNIT (1000000)

/* Type declarations */
/* qdf Timer callback function prototype (well, actually a prototype for
 * a pointer to this callback function)
 */
typedef void (*qdf_mc_timer_callback_t)(void *user_data);

typedef enum {
	QDF_TIMER_STATE_UNUSED = QDF_TIMER_STATE_COOKIE,
	QDF_TIMER_STATE_STOPPED,
	QDF_TIMER_STATE_STARTING,
	QDF_TIMER_STATE_RUNNING,
} QDF_TIMER_STATE;

#ifdef TIMER_MANAGER
struct qdf_mc_timer_s;
typedef struct qdf_mc_timer_node_s {
	qdf_list_node_t node;
	char *file_name;
	uint32_t line_num;
	struct qdf_mc_timer_s *qdf_timer;
} qdf_mc_timer_node_t;
#endif

typedef struct qdf_mc_timer_s {
#ifdef TIMER_MANAGER
	qdf_mc_timer_node_t *timer_node;
#endif
	qdf_mc_timer_platform_t platform_info;
	qdf_mc_timer_callback_t callback;
	void *user_data;
	qdf_mutex_t lock;
	QDF_TIMER_TYPE type;
	QDF_TIMER_STATE state;
} qdf_mc_timer_t;


void qdf_try_allowing_sleep(QDF_TIMER_TYPE type);

/* Function declarations and documenation */
#ifdef TIMER_MANAGER
void qdf_mc_timer_manager_init(void);
void qdf_mc_timer_manager_exit(void);
void qdf_mc_timer_check_for_leaks(void);
#else
/**
 * qdf_mc_timer_manager_init() - initialize QDF debug timer manager
 * This API initializes QDF timer debug functionality.
 *
 * Return: none
 */
static inline void qdf_mc_timer_manager_init(void)
{
}

/**
 * qdf_mc_timer_manager_exit() - exit QDF timer debug functionality
 * This API exists QDF timer debug functionality
 *
 * Return: none
 */
static inline void qdf_mc_timer_manager_exit(void)
{
}

/**
 * qdf_mc_timer_check_for_leaks() - Assert there are no active mc timers
 *
 * If there are active timers, this API prints them and panics.
 *
 * Return: None
 */
static inline void qdf_mc_timer_check_for_leaks(void) { }
#endif
/**
 * qdf_mc_timer_get_current_state() - get the current state of the timer
 * @timer:  Pointer to timer object
 *
 * Return:
 * QDF_TIMER_STATE - qdf timer state
 */

QDF_TIMER_STATE qdf_mc_timer_get_current_state(qdf_mc_timer_t *timer);

/**
 * qdf_mc_timer_init() - initialize a QDF timer
 * @timer: Pointer to timer object
 * @timer_type: Type of timer
 * @callback: Callback to be called after timer expiry
 * @ser_data: User data which will be passed to callback function
 *
 * This API initializes a QDF Timer object.
 *
 * qdf_mc_timer_init() initializes a QDF Timer object.  A timer must be
 * initialized by calling qdf_mc_timer_initialize() before it may be used in
 * any other timer functions.
 *
 * Attempting to initialize timer that is already initialized results in
 * a failure. A destroyed timer object can be re-initialized with a call to
 * qdf_mc_timer_init().  The results of otherwise referencing the object
 * after it has been destroyed are undefined.
 *
 *  Calls to QDF timer functions to manipulate the timer such
 *  as qdf_mc_timer_set() will fail if the timer is not initialized or has
 *  been destroyed.  Therefore, don't use the timer after it has been
 *  destroyed until it has been re-initialized.
 *
 *  All callback will be executed within the CDS main thread unless it is
 *  initialized from the Tx thread flow, in which case it will be executed
 *  within the tx thread flow.
 *
 * Return:
 * QDF_STATUS_SUCCESS - Timer is initialized successfully
 * QDF failure status - Timer initialization failed
 */
#ifdef TIMER_MANAGER
#define qdf_mc_timer_init(timer, timer_type, callback, userdata) \
	qdf_mc_timer_init_debug(timer, timer_type, callback, userdata, \
				__FILE__, __LINE__)

QDF_STATUS qdf_mc_timer_init_debug(qdf_mc_timer_t *timer,
				   QDF_TIMER_TYPE timer_type,
				   qdf_mc_timer_callback_t callback,
				   void *user_data, char *file_name,
				   uint32_t line_num);
#else
QDF_STATUS qdf_mc_timer_init(qdf_mc_timer_t *timer, QDF_TIMER_TYPE timer_type,
			     qdf_mc_timer_callback_t callback,
			     void *user_data);
#endif

/**
 * qdf_mc_timer_destroy() - destroy QDF timer
 * @timer: Pointer to timer object
 *
 * qdf_mc_timer_destroy() function shall destroy the timer object.
 * After a successful return from \a qdf_mc_timer_destroy() the timer
 * object becomes, in effect, uninitialized.
 *
 * A destroyed timer object can be re-initialized by calling
 * qdf_mc_timer_init().  The results of otherwise referencing the object
 * after it has been destroyed are undefined.
 *
 * Calls to QDF timer functions to manipulate the timer, such
 * as qdf_mc_timer_set() will fail if the lock is destroyed.  Therefore,
 * don't use the timer after it has been destroyed until it has
 * been re-initialized.
 *
 * Return:
 * QDF_STATUS_SUCCESS - Timer is initialized successfully
 * QDF failure status - Timer initialization failed
 */
QDF_STATUS qdf_mc_timer_destroy(qdf_mc_timer_t *timer);

/**
 * qdf_mc_timer_start() - start a QDF Timer object
 * @timer: Pointer to timer object
 * @expiration_time: Time to expire
 *
 * qdf_mc_timer_start() function starts a timer to expire after the
 * specified interval, thus running the timer callback function when
 * the interval expires.
 *
 * A timer only runs once (a one-shot timer).  To re-start the
 * timer, qdf_mc_timer_start() has to be called after the timer runs
 * or has been cancelled.
 *
 * Return:
 * QDF_STATUS_SUCCESS - Timer is initialized successfully
 * QDF failure status - Timer initialization failed
 */
QDF_STATUS qdf_mc_timer_start(qdf_mc_timer_t *timer, uint32_t expiration_time);

/**
 * qdf_mc_timer_stop() - stop a QDF Timer
 * @timer: Pointer to timer object
 * qdf_mc_timer_stop() function stops a timer that has been started but
 * has not expired, essentially cancelling the 'start' request.
 *
 * After a timer is stopped, it goes back to the state it was in after it
 * was created and can be started again via a call to qdf_mc_timer_start().
 *
 * Return:
 * QDF_STATUS_SUCCESS - Timer is initialized successfully
 * QDF failure status - Timer initialization failed
 */
QDF_STATUS qdf_mc_timer_stop(qdf_mc_timer_t *timer);

/**
 * qdf_mc_timer_get_system_ticks() - get the system time in 10ms ticks
 *
 * qdf_mc_timer_get_system_ticks() function returns the current number
 * of timer ticks in 10msec intervals.  This function is suitable timestamping
 * and calculating time intervals by calculating the difference between two
 * timestamps.
 *
 * Return:
 * The current system tick count (in 10msec intervals).  This
 * function cannot fail.
 */
unsigned long qdf_mc_timer_get_system_ticks(void);

/**
 * qdf_mc_timer_get_system_time() - Get the system time in milliseconds
 *
 * qdf_mc_timer_get_system_time() function returns the number of milliseconds
 * that have elapsed since the system was started
 *
 * Return:
 * The current system time in milliseconds
 */
unsigned long qdf_mc_timer_get_system_time(void);

/**
 * qdf_get_monotonic_boottime_ns() - Get kernel boottime in ns
 *
 * Return: kernel boottime in nano sec (includes time spent in suspend)
 */
s64 qdf_get_monotonic_boottime_ns(void);

/**
 * qdf_timer_module_init() - initializes a QDF timer module.
 *
 * This API initializes the QDF timer module. This needs to be called
 * exactly once prior to using any QDF timers.
 *
 * Return: none
 */
void qdf_timer_module_init(void);

/**
 * qdf_get_time_of_the_day_ms() - get time of the day in millisec
 *
 * Return: time of the day in ms
 */
qdf_time_t qdf_get_time_of_the_day_ms(void);

/**
 * qdf_timer_module_deinit() - Deinitializes a QDF timer module.
 *
 * This API deinitializes the QDF timer module.
 * Return: none
 */
void qdf_timer_module_deinit(void);

/**
 * qdf_get_time_of_the_day_in_hr_min_sec_usec() - Get system time
 * @tbuf: Pointer to time stamp buffer
 * @len: Time buffer size
 *
 * This function updates the 'tbuf' with system time in hr:min:sec:msec format
 *
 * Return: None
 */
void qdf_get_time_of_the_day_in_hr_min_sec_usec(char *tbuf, int len);
void qdf_register_mc_timer_callback(void (*callback) (unsigned long data));

/**
 * qdf_timer_set_multiplier() - set the global QDF timer scalar value
 * @multiplier: the scalar value to apply
 *
 * Return: None
 */
void qdf_timer_set_multiplier(uint32_t multiplier);

/**
 * qdf_timer_get_multiplier() - get the global QDF timer scalar value
 *
 * Return: the global QDF timer scalar value
 */
uint32_t qdf_timer_get_multiplier(void);

#endif /* __QDF_MC_TIMER_H */
