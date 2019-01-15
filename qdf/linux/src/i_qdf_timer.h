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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
#include <linux/sched/task_stack.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0)
#define setup_deferrable_timer(timer, fn, data)                                \
	__setup_timer((timer), (fn), (data), TIMER_DEFERRABLE)
#endif

struct __qdf_timer_t {
	struct timer_list os_timer;
};

typedef void (*__legacy_timer_callback_t)(unsigned long arg);

static inline QDF_STATUS __qdf_timer_init(struct __qdf_timer_t *timer,
					  qdf_timer_func_t func, void *arg,
					  QDF_TIMER_TYPE type)
{
	struct timer_list *os_timer = &timer->os_timer;
	bool is_on_stack = object_is_on_stack(os_timer);
	__legacy_timer_callback_t callback = (__legacy_timer_callback_t)func;
	unsigned long ctx = (unsigned long)arg;

	if (type == QDF_TIMER_TYPE_SW) {
		if (is_on_stack)
			setup_deferrable_timer_on_stack(os_timer, callback,
							ctx);
		else
			setup_deferrable_timer(os_timer, callback, ctx);
	} else {
		if (is_on_stack)
			setup_timer_on_stack(os_timer, callback, ctx);
		else
			setup_timer(os_timer, callback, ctx);
	}

	return QDF_STATUS_SUCCESS;
}

static inline void __qdf_timer_start(struct __qdf_timer_t *timer, uint32_t msec)
{
	struct timer_list *os_timer = &timer->os_timer;

	os_timer->expires = jiffies + msecs_to_jiffies(msec);
	add_timer(os_timer);
}

static inline void __qdf_timer_mod(struct __qdf_timer_t *timer, uint32_t msec)
{
	mod_timer(&timer->os_timer, jiffies + msecs_to_jiffies(msec));
}

static inline bool __qdf_timer_stop(struct __qdf_timer_t *timer)
{
	return !!del_timer(&timer->os_timer);
}

static inline void __qdf_timer_free(struct __qdf_timer_t *timer)
{
	struct timer_list *os_timer = &timer->os_timer;

	del_timer_sync(os_timer);

	if (object_is_on_stack(os_timer))
		destroy_timer_on_stack(os_timer);
}

static inline bool __qdf_timer_sync_cancel(struct __qdf_timer_t *timer)
{
	return del_timer_sync(&timer->os_timer);
}

#endif /* _I_QDF_TIMER_H */
