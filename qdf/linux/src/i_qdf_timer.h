/*
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_timer
 * This file provides OS dependent timer API's.
 */

#ifndef _I_QDF_TIMER_H
#define _I_QDF_TIMER_H

#include <linux/version.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <qdf_types.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0)
#define setup_deferrable_timer(timer, fn, data)                                \
	__setup_timer((timer), (fn), (data), TIMER_DEFERRABLE)
#endif

/* timer data type */
typedef struct timer_list __qdf_timer_t;

typedef void (*qdf_dummy_timer_func_t)(unsigned long arg);

/**
 * __qdf_timer_init() - initialize a softirq timer
 * @hdl: OS handle
 * @timer: Pointer to timer object
 * @func: Function pointer
 * @arg: Arguement
 * @type: deferrable or non deferrable timer type
 *
 * Timer type QDF_TIMER_TYPE_SW means its a deferrable sw timer which will
 * not cause CPU wake upon expiry
 * Timer type QDF_TIMER_TYPE_WAKE_APPS means its a non-deferrable timer which
 * will cause CPU wake up on expiry
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS __qdf_timer_init(qdf_handle_t hdl,
					  struct timer_list *timer,
					  qdf_timer_func_t func, void *arg,
					  QDF_TIMER_TYPE type)
{
	if (type == QDF_TIMER_TYPE_SW) {
		if (object_is_on_stack(timer))
			setup_deferrable_timer_on_stack(
			    timer, (qdf_dummy_timer_func_t)func,
			    (unsigned long)arg);
		else
			setup_deferrable_timer(timer,
					       (qdf_dummy_timer_func_t)func,
					       (unsigned long)arg);
	} else {
		if (object_is_on_stack(timer))
			setup_timer_on_stack(timer,
					     (qdf_dummy_timer_func_t)func,
					     (unsigned long)arg);
		else
			setup_timer(timer, (qdf_dummy_timer_func_t)func,
				    (unsigned long)arg);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_timer_start() - start a qdf softirq timer
 * @timer: Pointer to timer object
 * @delay: Delay in milli seconds
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS __qdf_timer_start(struct timer_list *timer,
					   uint32_t delay)
{
	timer->expires = jiffies + msecs_to_jiffies(delay);
	add_timer(timer);

	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_timer_mod() - modify a timer
 * @timer: Pointer to timer object
 * @delay: Delay in milli seconds
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS __qdf_timer_mod(struct timer_list *timer,
					 uint32_t delay)
{
	mod_timer(timer, jiffies + msecs_to_jiffies(delay));

	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_timer_stop() - cancel a timer
 * @timer: Pointer to timer object
 *
 * Return: true if timer was cancelled and deactived,
 * false if timer was cancelled but already got fired.
 */
static inline bool __qdf_timer_stop(struct timer_list *timer)
{
	if (likely(del_timer(timer)))
		return 1;
	else
		return 0;
}

/**
 * __qdf_timer_free() - free a qdf timer
 * @timer: Pointer to timer object
 *
 * Return: None
 */
static inline void __qdf_timer_free(struct timer_list *timer)
{
	del_timer_sync(timer);

	if (object_is_on_stack(timer))
		destroy_timer_on_stack(timer);
}

/**
 * __qdf_sostirq_timer_sync_cancel() - Synchronously canel a timer
 * @timer: Pointer to timer object
 *
 * Synchronization Rules:
 * 1. caller must make sure timer function will not use
 *    qdf_set_timer to add iteself again.
 * 2. caller must not hold any lock that timer function
 *    is likely to hold as well.
 * 3. It can't be called from interrupt context.
 *
 * Return: true if timer was cancelled and deactived,
 * false if timer was cancelled but already got fired.
 */
static inline bool __qdf_timer_sync_cancel(struct timer_list *timer)
{
	return del_timer_sync(timer);
}

#endif /*_QDF_TIMER_PVT_H*/
