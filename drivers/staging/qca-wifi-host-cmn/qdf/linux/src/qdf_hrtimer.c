/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
 * QCA driver framework (QDF) high resolution timers timer APIs
 */

#include "i_qdf_hrtimer.h"
#include "qdf_hrtimer.h"
#include <qdf_module.h>

void qdf_hrtimer_start(qdf_hrtimer_data_t *timer, qdf_ktime_t interval,
		       enum qdf_hrtimer_mode mode)
{
	__qdf_hrtimer_start(timer, interval, mode);
}

qdf_export_symbol(qdf_hrtimer_start);

int qdf_hrtimer_cancel(qdf_hrtimer_data_t *timer)
{
	return __qdf_hrtimer_cancel(timer);
}

qdf_export_symbol(qdf_hrtimer_cancel);

void qdf_hrtimer_init(qdf_hrtimer_data_t *timer,
		      qdf_hrtimer_func_t callback,
		      enum qdf_clock_id clock,
		      enum qdf_hrtimer_mode mode,
		      enum qdf_context_mode ctx)
{
	__qdf_hrtimer_init(timer, callback, clock, mode, ctx);
}

qdf_export_symbol(qdf_hrtimer_init);

void qdf_hrtimer_kill(qdf_hrtimer_data_t *timer)
{
	__qdf_hrtimer_kill(timer);
}

qdf_export_symbol(qdf_hrtimer_kill);

qdf_ktime_t qdf_hrtimer_get_remaining(qdf_hrtimer_data_t *timer)
{
	return __qdf_hrtimer_get_remaining(timer);
}

qdf_export_symbol(qdf_hrtimer_get_remaining);

bool qdf_hrtimer_is_queued(qdf_hrtimer_data_t *timer)
{
	return __qdf_hrtimer_is_queued(timer);
}

qdf_export_symbol(qdf_hrtimer_is_queued);

bool qdf_hrtimer_callback_running(qdf_hrtimer_data_t *timer)
{
	return __qdf_hrtimer_callback_running(timer);
}

qdf_export_symbol(qdf_hrtimer_callback_running);

bool qdf_hrtimer_active(qdf_hrtimer_data_t *timer)
{
	return __qdf_hrtimer_active(timer);
}

qdf_export_symbol(qdf_hrtimer_active);

qdf_ktime_t qdf_hrtimer_cb_get_time(qdf_hrtimer_data_t *timer)
{
	return __qdf_hrtimer_cb_get_time(timer);
}

qdf_export_symbol(qdf_hrtimer_cb_get_time);

uint64_t qdf_hrtimer_forward(qdf_hrtimer_data_t *timer,
			     qdf_ktime_t now,
			     qdf_ktime_t interval)
{
	return __qdf_hrtimer_forward(timer, now, interval);
}

qdf_export_symbol(qdf_hrtimer_forward);
