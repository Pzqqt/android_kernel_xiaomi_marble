/*
 * Copyright (c) 2014, 2016 The Linux Foundation. All rights reserved.
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
 * DOC: cdf_defer.h
 * This file abstracts deferred execution contexts.
 */

#ifndef __CDF_DEFER_H
#define __CDF_DEFER_H

#include <cdf_types.h>
#include <i_cdf_defer.h>

/**
 * This implements work queues (worker threads, kernel threads etc.).
 * Note that there is no cancel on a scheduled work. You cannot free a work
 * item if its queued. You cannot know if a work item is queued or not unless
 * its running, whence you know its not queued.
 *
 * so if, say, a module is asked to unload itself, how exactly will it make
 * sure that the work's not queued, for OS'es that dont provide such a
 * mechanism??
 */

/* cdf_work_t - representation of a work queue */
typedef __cdf_work_t cdf_work_t;

/* cdf_work_t - representation of a bottom half */
typedef __cdf_bh_t cdf_bh_t;

/**
 * cdf_create_bh() - this creates the Bottom half deferred handler
 * @hdl:   OS handle
 * @bh:    Bottom instance
 * @func:  Func deferred function to run at bottom half interrupt
 *         context
 * Return: None
 */
static inline void
cdf_create_bh(cdf_handle_t hdl, cdf_bh_t *bh, cdf_defer_fn_t func, void *arg)
{
	__cdf_init_bh(hdl, bh, func, arg);
}

/**
 * cdf_sched_bh() - schedule a bottom half (DPC)
 * @hdl:	OS handle
 * @bh:		Bottom instance
 *
 * Return: None
 */
static inline void cdf_sched_bh(cdf_handle_t hdl, cdf_bh_t *bh)
{
	__cdf_sched_bh(hdl, bh);
}

/**
 * cdf_destroy_bh() - destroy a bottom half (DPC)
 * @hdl:	OS handle
 * @bh:		Bottom instance
 *
 * Return: None
 */
static inline void cdf_destroy_bh(cdf_handle_t hdl, cdf_bh_t *bh)
{
	__cdf_disable_bh(hdl, bh);
}

/*********************Non-Interrupt Context deferred Execution***************/

/**
 * cdf_create_work() - create a work/task queue, This runs in non-interrupt
 *		       context, so can be preempted by H/W & S/W intr
 * @work:	Work instance
 * @func:	Deferred function to run at bottom half non-interrupt
 *		context
 * @arg:	Argument for the deferred function
 *
 * Return: None
 */
static inline void
cdf_create_work(cdf_work_t *work,
		cdf_defer_fn_t func, void *arg)
{
	__cdf_init_work(work, func, arg);
}

/**
 * cdf_sched_work() - schedule a deferred task on non-interrupt context
 * @work:	Work instance
 *
 * Return: None
 */
static inline void cdf_schedule_work(cdf_work_t *work)
{
	__cdf_schedule_work(work);
}
#endif /*__CDF_DEFER_H*/
