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

/**
 * DOC: cdf_softirq_timer
 * This file abstracts OS timers running in soft IRQ context.
 */

#ifndef _CDF_SOFTIRQ_TIMER_H
#define _CDF_SOFTIRQ_TIMER_H

#include <cdf_types.h>
#include <i_cdf_softirq_timer.h>

/* Platform timer object */
typedef __cdf_softirq_timer_t cdf_softirq_timer_t;

/**
 * cdf_softirq_timer_init() - initialize a softirq timer
 * @hdl: OS handle
 * @timer: Timer object pointer
 * @func: Timer function
 * @arg: Arguement of timer function
 * @type: deferrable or non deferrable timer type
 *
 * Timer type CDF_TIMER_TYPE_SW means its a deferrable sw timer which will
 * not cause CPU wake upon expiry
 * Timer type CDF_TIMER_TYPE_WAKE_APPS means its a non-deferrable timer which
 * will cause CPU wake up on expiry
 *
 * Return: none
 */
static inline void
cdf_softirq_timer_init(cdf_handle_t hdl,
			cdf_softirq_timer_t *timer,
			cdf_softirq_timer_func_t func, void *arg,
			CDF_TIMER_TYPE type)
{
	__cdf_softirq_timer_init(hdl, timer, func, arg, type);
}

/**
 * cdf_softirq_timer_start() - start a one-shot softirq timer
 * @timer: Timer object pointer
 * @msec: Expiration period in milliseconds
 *
 * Return: none
 */
static inline void
cdf_softirq_timer_start(cdf_softirq_timer_t *timer, int msec)
{
	__cdf_softirq_timer_start(timer, msec);
}

/**
 * cdf_softirq_timer_mod() - modify existing timer to new timeout value
 * @timer: Timer object pointer
 * @msec: Expiration period in milliseconds
 *
 * Return: none
 */
static inline void cdf_softirq_timer_mod(cdf_softirq_timer_t *timer, int msec)
{
	__cdf_softirq_timer_mod(timer, msec);
}

/**
 * cdf_softirq_timer_cancel() - cancel cdf softirq timer
 * @timer: Timer object pointer
 * @retval: Timer was cancelled and deactived
 * @retval: Timer was cancelled but already got fired.
 *
 * The function will return after any running timer completes.
 *
 * Return: none
 */
static inline bool cdf_softirq_timer_cancel(cdf_softirq_timer_t *timer)
{
	return __cdf_softirq_timer_cancel(timer);
}

/**
 * cdf_softirq_timer_free() - free cdf softirq timer
 * @timer: Timer object pointer
 *
 * The function will return after any running timer completes.
 * Return: none
 */
static inline void cdf_softirq_timer_free(cdf_softirq_timer_t *timer)
{
	__cdf_softirq_timer_free(timer);
}

#endif
