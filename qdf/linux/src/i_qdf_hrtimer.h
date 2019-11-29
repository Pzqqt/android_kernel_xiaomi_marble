/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_hrtimer
 * This file provides OS dependent timer API's.
 */

#ifndef _I_QDF_HRTIMER_H
#define _I_QDF_HRTIMER_H

#include <linux/version.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <qdf_types.h>

/* hrtimer data type */
typedef struct {
	union {
		struct hrtimer hrtimer;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0))
		struct tasklet_hrtimer tasklet_hrtimer;
#endif
	} u;
	enum qdf_context_mode ctx;
} __qdf_hrtimer_data_t;

/**
 * __qdf_hrtimer_get_mode() - Get hrtimer_mode with qdf mode
 * @mode: mode of hrtimer
 *
 * Get hrtimer_mode with qdf hrtimer mode
 *
 * Return: void
 */
static inline
enum hrtimer_mode __qdf_hrtimer_get_mode(enum qdf_hrtimer_mode mode)
{
	return (enum hrtimer_mode)mode;
}

/**
 * __qdf_hrtimer_start() - Starts hrtimer in given context
 * @timer: pointer to the hrtimer object
 * @interval: interval to forward as qdf_ktime_t object
 * @mode: mode of hrtimer
 *
 * Starts hrtimer in given context
 *
 * Return: void
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
static inline
void __qdf_hrtimer_start(__qdf_hrtimer_data_t *timer, ktime_t interval,
			 enum qdf_hrtimer_mode mode)
{
	enum hrtimer_mode hrt_mode = __qdf_hrtimer_get_mode(mode);

	hrtimer_start(&timer->u.hrtimer, interval, hrt_mode);
}
#else
static inline
void __qdf_hrtimer_start(__qdf_hrtimer_data_t *timer, ktime_t interval,
			 enum qdf_hrtimer_mode mode)
{
	enum hrtimer_mode hrt_mode = __qdf_hrtimer_get_mode(mode);

	if (timer->ctx == QDF_CONTEXT_HARDWARE)
		hrtimer_start(&timer->u.hrtimer, interval, hrt_mode);
	else if (timer->ctx == QDF_CONTEXT_TASKLET)
		tasklet_hrtimer_start(&timer->u.tasklet_hrtimer,
				      interval, hrt_mode);
}
#endif

/**
 * __qdf_hrtimer_cancel() - cancels hrtimer in given context
 * @timer: pointer to the hrtimer object
 *
 * cancels hrtimer in given context
 *
 * Return: int
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
static inline
int __qdf_hrtimer_cancel(__qdf_hrtimer_data_t *timer)
{
	if (timer->ctx == QDF_CONTEXT_HARDWARE)
		return hrtimer_cancel(&timer->u.hrtimer);

	return 0;
}
#else
static inline
int __qdf_hrtimer_cancel(__qdf_hrtimer_data_t *timer)
{
	if (timer->ctx == QDF_CONTEXT_HARDWARE)
		return hrtimer_cancel(&timer->u.hrtimer);
	else if (timer->ctx == QDF_CONTEXT_TASKLET)
		return hrtimer_cancel(&timer->u.tasklet_hrtimer.timer);

	return 0;
}
#endif

/**
 * __qdf_hrtimer_init() - init hrtimer in a given context
 * @timer: pointer to the hrtimer object
 * @cback: callback function to be fired
 * @clock: clock id
 * @hrtimer_mode: mode of hrtimer
 *
 * starts hrtimer in a context passed as per the context
 *
 * Return: void
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
static inline void  __qdf_hrtimer_init(__qdf_hrtimer_data_t *timer,
				       void *cback,
				       enum qdf_clock_id clock,
				       enum qdf_hrtimer_mode mode,
				       enum qdf_context_mode ctx)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;
	enum hrtimer_mode hrt_mode = __qdf_hrtimer_get_mode(mode);

	timer->ctx = ctx;

	if (timer->ctx == QDF_CONTEXT_HARDWARE) {
		hrtimer_init(hrtimer, clock, hrt_mode);
		hrtimer->function = cback;
	} else if (timer->ctx == QDF_CONTEXT_TASKLET) {
		QDF_BUG(0);
	}
}
#else
static inline void  __qdf_hrtimer_init(__qdf_hrtimer_data_t *timer,
				       void *cback,
				       enum qdf_clock_id clock,
				       enum qdf_hrtimer_mode mode,
				       enum qdf_context_mode ctx)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;
	struct tasklet_hrtimer *tasklet_hrtimer = &timer->u.tasklet_hrtimer;
	enum hrtimer_mode hrt_mode = __qdf_hrtimer_get_mode(mode);

	timer->ctx = ctx;

	if (timer->ctx == QDF_CONTEXT_HARDWARE) {
		hrtimer_init(hrtimer, clock, hrt_mode);
		hrtimer->function = cback;
	} else if (timer->ctx == QDF_CONTEXT_TASKLET) {
		tasklet_hrtimer_init(tasklet_hrtimer, cback, clock, hrt_mode);
	}
}
#endif

/**
 * __qdf_hrtimer_kill() - kills hrtimer in given context
 * @timer: pointer to the hrtimer object
 *
 * kills hrtimer in given context
 *
 * Return: void
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
static inline
void __qdf_hrtimer_kill(__qdf_hrtimer_data_t *timer)
{
	hrtimer_cancel(&timer->u.hrtimer);
}
#else
static inline
void __qdf_hrtimer_kill(__qdf_hrtimer_data_t *timer)
{
	if (timer->ctx == QDF_CONTEXT_HARDWARE)
		hrtimer_cancel(&timer->u.hrtimer);
	else if (timer->ctx == QDF_CONTEXT_TASKLET)
		tasklet_hrtimer_cancel(&timer->u.tasklet_hrtimer);
}
#endif

/**
 * __qdf_hrtimer_get_remaining() - check remaining time in the timer
 * @timer: pointer to the hrtimer object
 *
 * check whether the timer is on one of the queues
 *
 * Return: remaining time as ktime object
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
static inline ktime_t __qdf_hrtimer_get_remaining(__qdf_hrtimer_data_t *timer)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;

	return hrtimer_get_remaining(hrtimer);
}
#else
static inline ktime_t __qdf_hrtimer_get_remaining(__qdf_hrtimer_data_t *timer)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;
	struct tasklet_hrtimer *tasklet_hrtimer = &timer->u.tasklet_hrtimer;

	if (timer->ctx == QDF_CONTEXT_HARDWARE)
		return hrtimer_get_remaining(hrtimer);
	else
		return hrtimer_get_remaining(&tasklet_hrtimer->timer);
}
#endif

/**
 * __qdf_hrtimer_is_queued() - check whether the timer is on one of the queues
 * @timer: pointer to the hrtimer object
 *
 * check whether the timer is on one of the queues
 *
 * Return: false when the timer was not in queue
 *         true when the timer was in queue
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
static inline bool __qdf_hrtimer_is_queued(__qdf_hrtimer_data_t *timer)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;

	return hrtimer_is_queued(hrtimer);
}
#else
static inline bool __qdf_hrtimer_is_queued(__qdf_hrtimer_data_t *timer)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;
	struct tasklet_hrtimer *tasklet_hrtimer = &timer->u.tasklet_hrtimer;

	if (timer->ctx == QDF_CONTEXT_HARDWARE)
		return hrtimer_is_queued(hrtimer);
	else
		return hrtimer_is_queued(&tasklet_hrtimer->timer);
}
#endif

/**
 * __qdf_hrtimer_callback_running() - check if callback is running
 * @timer: pointer to the hrtimer object
 *
 * check whether the timer is running the callback function
 *
 * Return: false when callback is not running
 *         true when callback is running
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
static inline bool __qdf_hrtimer_callback_running(__qdf_hrtimer_data_t *timer)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;

	return hrtimer_callback_running(hrtimer);
}
#else
static inline bool __qdf_hrtimer_callback_running(__qdf_hrtimer_data_t *timer)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;
	struct tasklet_hrtimer *tasklet_hrtimer = &timer->u.tasklet_hrtimer;

	if (timer->ctx == QDF_CONTEXT_HARDWARE)
		return hrtimer_callback_running(hrtimer);
	else
		return hrtimer_callback_running(&tasklet_hrtimer->timer);
}
#endif

/**
 * __qdf_hrtimer_active() - check if timer is active
 * @timer: pointer to the hrtimer object
 *
 * Check if timer is active. A timer is active, when it is enqueued into
 * the rbtree or the callback function is running.
 *
 * Return: false if timer is not active
 *         true if timer is active
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
static inline bool __qdf_hrtimer_active(__qdf_hrtimer_data_t *timer)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;

	return hrtimer_active(hrtimer);
}
#else
static inline bool __qdf_hrtimer_active(__qdf_hrtimer_data_t *timer)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;
	struct tasklet_hrtimer *tasklet_hrtimer = &timer->u.tasklet_hrtimer;

	if (timer->ctx == QDF_CONTEXT_HARDWARE)
		return hrtimer_active(hrtimer);
	else
		return hrtimer_active(&tasklet_hrtimer->timer);
}
#endif

/**
 * __qdf_hrtimer_cb_get_time() - get remaining time in callback
 * @timer: pointer to the hrtimer object
 *
 * Get remaining time in the hrtimer callback
 *
 * Return: time remaining as ktime object
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
static inline ktime_t __qdf_hrtimer_cb_get_time(__qdf_hrtimer_data_t *timer)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;

	return hrtimer_cb_get_time(hrtimer);
}
#else
static inline ktime_t __qdf_hrtimer_cb_get_time(__qdf_hrtimer_data_t *timer)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;
	struct tasklet_hrtimer *tasklet_hrtimer = &timer->u.tasklet_hrtimer;

	if (timer->ctx == QDF_CONTEXT_HARDWARE)
		return hrtimer_cb_get_time(hrtimer);
	else
		return hrtimer_cb_get_time(&tasklet_hrtimer->timer);
}
#endif

/**
 * __qdf_hrtimer_forward() - forward the hrtimer
 * @timer: pointer to the hrtimer object
 * @now: current ktime
 * @interval: interval to forward as ktime object
 *
 * Forward the timer expiry so it will expire in the future
 *
 * Return:the number of overruns
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
static inline uint64_t __qdf_hrtimer_forward(__qdf_hrtimer_data_t *timer,
					     ktime_t now,
					     ktime_t interval)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;

	return hrtimer_forward(hrtimer, now, interval);
}
#else
static inline uint64_t __qdf_hrtimer_forward(__qdf_hrtimer_data_t *timer,
					     ktime_t now,
					     ktime_t interval)
{
	struct hrtimer *hrtimer = &timer->u.hrtimer;
	struct tasklet_hrtimer *tasklet_hrtimer = &timer->u.tasklet_hrtimer;

	if (timer->ctx == QDF_CONTEXT_HARDWARE)
		return hrtimer_forward(hrtimer, now, interval);
	else
		return hrtimer_forward(&tasklet_hrtimer->timer, now, interval);
}
#endif

#endif /* _I_QDF_HRTIMER_H */
