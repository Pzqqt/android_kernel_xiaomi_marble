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
 * DOC: qdf_hrtimer
 * This file abstracts high resolution timers running in hardware context.
 */

#ifndef _QDF_HRTIMER_H
#define _QDF_HRTIMER_H

#include <qdf_types.h>
#include <i_qdf_hrtimer.h>
#include <qdf_time.h>

/* Context independent hrtimer object */
typedef __qdf_hrtimer_data_t qdf_hrtimer_data_t;

/* Platform independent timer callback function */
typedef enum qdf_hrtimer_restart_status(*qdf_hrtimer_func_t)
					(qdf_hrtimer_data_t *timer);

/**
 * qdf_hrtimer_start() - Starts hrtimer in given context
 * @timer: pointer to the qdf_hrtimer_data_t object
 * @interval: interval to forward as qdf_ktime_t object
 * @mode: mode of qdf_hrtimer_data_t
 *
 * Starts hrtimer in given context
 *
 * Return: void
 */
static inline
void qdf_hrtimer_start(qdf_hrtimer_data_t *timer, qdf_ktime_t interval,
		       enum qdf_hrtimer_mode mode)
{
	__qdf_hrtimer_start(timer, interval, mode);
}

/**
 * qdf_hrtimer_cancel() - Cancels hrtimer in given context
 * @timer: pointer to the qdf_hrtimer_data_t object
 *
 * Cancels hrtimer in given context
 *
 * Return: int
 */
static inline
int qdf_hrtimer_cancel(qdf_hrtimer_data_t *timer)
{
	return __qdf_hrtimer_cancel(timer);
}

/**
 * qdf_hrtimer_init() - init hrtimer based on context
 * @timer: pointer to the qdf_hrtimer_data_t object
 * @callback: callback function to be fired
 * @qdf_clock_id: clock type
 * @qdf_hrtimer_mode: mode of qdf_hrtimer_data_t
 * @qdf_context_mode: interrupt context mode
 *
 * starts hrtimer in a context passed as per qdf_context_mode
 *
 * Return: void
 */
static inline void qdf_hrtimer_init(qdf_hrtimer_data_t *timer,
				    qdf_hrtimer_func_t callback,
				    enum qdf_clock_id clock,
				    enum qdf_hrtimer_mode mode,
				    enum qdf_context_mode ctx)
{
	__qdf_hrtimer_init(timer, callback, clock, mode, ctx);
}

/**
 * qdf_hrtimer_kill() - kills hrtimer in given context
 * @timer: pointer to the hrtimer object
 *
 * kills hrtimer in given context
 *
 * Return: void
 */
static inline
void qdf_hrtimer_kill(__qdf_hrtimer_data_t *timer)
{
	 __qdf_hrtimer_kill(timer);
}

/**
 * qdf_hrtimer_get_remaining() - check remaining time in the timer
 * @timer: pointer to the qdf_hrtimer_data_t object
 *
 * check whether the timer is on one of the queues
 *
 * Return: remaining time as qdf_ktime_t object
 */
static inline qdf_ktime_t qdf_hrtimer_get_remaining(qdf_hrtimer_data_t *timer)
{
	return __qdf_hrtimer_get_remaining(timer);
}

/**
 * qdf_hrtimer_is_queued() - check whether the timer is on one of the queues
 * @timer: pointer to the qdf_hrtimer_data_t object
 *
 * check whether the timer is on one of the queues
 *
 * Return: false when the timer was not in queue
 *         true when the timer was in queue
 */
static inline bool qdf_hrtimer_is_queued(qdf_hrtimer_data_t *timer)
{
	return __qdf_hrtimer_is_queued(timer);
}

/**
 * qdf_hrtimer_callback_running() - check if callback is running
 * @timer: pointer to the qdf_hrtimer_data_t object
 *
 * check whether the timer is running the callback function
 *
 * Return: false when callback is not running
 *         true when callback is running
 */
static inline bool qdf_hrtimer_callback_running(qdf_hrtimer_data_t *timer)
{
	return __qdf_hrtimer_callback_running(timer);
}

/**
 * qdf_hrtimer_active() - check if timer is active
 * @timer: pointer to the qdf_hrtimer_data_t object
 *
 * Check if timer is active. A timer is active, when it is enqueued into
 * the rbtree or the callback function is running.
 *
 * Return: false if timer is not active
 *         true if timer is active
 */
static inline bool qdf_hrtimer_active(qdf_hrtimer_data_t *timer)
{
	return __qdf_hrtimer_active(timer);
}

/**
 * qdf_hrtimer_cb_get_time() - get remaining time in callback
 * @timer: pointer to the qdf_hrtimer_data_t object
 *
 * Get remaining time in the hrtimer callback
 *
 * Return: time remaining as qdf_ktime_t object
 */
static inline qdf_ktime_t qdf_hrtimer_cb_get_time(qdf_hrtimer_data_t *timer)
{
	return __qdf_hrtimer_cb_get_time(timer);
}

/**
 * qdf_hrtimer_forward() - forward the hrtimer
 * @timer: pointer to the qdf_hrtimer_data_t object
 * @now: current time as qdf_ktime_t object
 * @interval: interval to forward as qdf_ktime_t object
 *
 * Forward the timer expiry so it will expire in the future
 *
 * Return: the number of overruns
 */
static inline uint64_t qdf_hrtimer_forward(qdf_hrtimer_data_t *timer,
					   qdf_ktime_t now,
					   qdf_ktime_t interval)
{
	return __qdf_hrtimer_forward(timer, now, interval);
}

#endif /* _QDF_HRTIMER_H */
