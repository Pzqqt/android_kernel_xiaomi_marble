/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_timer
 * This file abstracts OS timers running in soft IRQ context.
 */

#ifndef _QDF_TIMER_H
#define _QDF_TIMER_H

#include <qdf_types.h>
#include <i_qdf_timer.h>

/* Platform timer object */
typedef __qdf_timer_t qdf_timer_t;

/**
 * qdf_timer_init() - initialize a timer
 * @hdl: OS handle
 * @timer: Timer object pointer
 * @func: Timer function
 * @arg: Arguement of timer function
 * @type: deferrable or non deferrable timer type
 *
 * Timer type QDF_TIMER_TYPE_SW means its a deferrable sw timer which will
 * not cause CPU wake upon expiry
 * Timer type QDF_TIMER_TYPE_WAKE_APPS means its a non-deferrable timer which
 * will cause CPU wake up on expiry
 *
 * Return: none
 */
static inline void qdf_timer_init(qdf_handle_t hdl, qdf_timer_t *timer,
				  qdf_timer_func_t func, void *arg,
				  QDF_TIMER_TYPE type)
{
	__qdf_timer_init(hdl, timer, func, arg, type);
}

/**
 * qdf_timer_start() - start a one-shot timer
 * @timer: Timer object pointer
 * @msec: Expiration period in milliseconds
 *
 * Return: none
 */
static inline void
qdf_timer_start(qdf_timer_t *timer, int msec)
{
	__qdf_timer_start(timer, msec);
}

/**
 * qdf_timer_mod() - modify existing timer to new timeout value
 * @timer: Timer object pointer
 * @msec: Expiration period in milliseconds
 *
 * Return: none
 */
static inline void qdf_timer_mod(qdf_timer_t *timer, int msec)
{
	__qdf_timer_mod(timer, msec);
}

/**
 * qdf_timer_stop() - cancel qdf timer
 * @timer: Timer object pointer
 *
 * return: bool TRUE Timer was cancelled and deactived
 * FALSE Timer was cancelled but already got fired.
 *
 * The function will return after any running timer completes.
 */
static inline bool qdf_timer_stop(qdf_timer_t *timer)
{
	return __qdf_timer_stop(timer);
}


/**
 * qdf_timer_sync_cancel - Cancel a timer synchronously
 * The function will return after any running timer completes.
 * @timer: timer object pointer
 *
 * return: bool TRUE timer was cancelled and deactived
 * FALSE timer was not cancelled
 */
static inline bool qdf_timer_sync_cancel(qdf_timer_t *timer)
{
	return __qdf_timer_sync_cancel(timer);
}


/**
 * qdf_timer_free() - free qdf timer
 * @timer: Timer object pointer
 *
 * The function will return after any running timer completes.
 * Return: none
 */
static inline void qdf_timer_free(qdf_timer_t *timer)
{
	__qdf_timer_free(timer);
}

#endif /* _QDF_TIMER_H */
