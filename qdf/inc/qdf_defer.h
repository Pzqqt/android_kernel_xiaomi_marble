/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_defer.h
 * This file abstracts deferred execution API's.
 */

#ifndef __QDF_DEFER_H
#define __QDF_DEFER_H

#include <qdf_types.h>
#include <i_qdf_defer.h>

/**
 * TODO This implements work queues (worker threads, kernel threads etc.).
 * Note that there is no cancel on a scheduled work. You cannot free a work
 * item if its queued. You cannot know if a work item is queued or not unless
 * its running, hence you know its not queued.
 *
 * so if, say, a module is asked to unload itself, how exactly will it make
 * sure that the work's not queued, for OS'es that dont provide such a
 * mechanism??
 */

/*
 * Representation of a work queue.
 */
typedef __qdf_work_t     qdf_work_t;
typedef __qdf_delayed_work_t qdf_delayed_work_t;
typedef __qdf_workqueue_t     qdf_workqueue_t;

/*
 * Representation of a bottom half.
 */
typedef __qdf_bh_t       qdf_bh_t;

/**
 * qdf_create_bh - creates the bottom half deferred handler
 * @hdl: os handle
 * @bh: pointer to bottom
 * @func: deferred function to run at bottom half interrupt context.
 * @arg: argument for the deferred function
 * Return: none
 */
static inline void qdf_create_bh(qdf_handle_t  hdl, qdf_bh_t  *bh,
				 qdf_defer_fn_t  func, void  *arg)
{
	__qdf_init_bh(hdl, bh, func, arg);
}

/**
 * qdf_sched - schedule a bottom half (DPC)
 * @hdl: OS handle
 * @bh: pointer to bottom
 * Return: none
 */
static inline void qdf_sched_bh(qdf_handle_t hdl, qdf_bh_t *bh)
{
	__qdf_sched_bh(hdl, bh);
}

/**
 * qdf_destroy_bh - destroy the bh (synchronous)
 * @hdl: OS handle
 * @bh: pointer to bottom
 * Return: none
 */
static inline void qdf_destroy_bh(qdf_handle_t hdl, qdf_bh_t *bh)
{
	__qdf_disable_bh(hdl, bh);
}

/*********************Non-Interrupt Context deferred Execution***************/

/**
 * qdf_create_work - create a work/task queue, This runs in non-interrupt
 * context, so can be preempted by H/W & S/W intr
 * @hdl: OS handle
 * @work: pointer to work
 * @func: deferred function to run at bottom half non-interrupt context.
 * @arg: argument for the deferred function
 * Return: none
 */
static inline void qdf_create_work(qdf_handle_t hdl, qdf_work_t  *work,
				   qdf_defer_fn_t  func, void  *arg)
{
	__qdf_init_work(hdl, work, func, arg);
}

/**
 * qdf_create_delayed_work - create a delayed work/task, This runs in
 * non-interrupt context, so can be preempted by H/W & S/W intr
 * @hdl: OS handle
 * @work: pointer to work
 * @func: deferred function to run at bottom half non-interrupt context.
 * @arg: argument for the deferred function
 * Return: none
 */
static inline void qdf_create_delayed_work(qdf_handle_t hdl,
					   qdf_delayed_work_t  *work,
					   qdf_defer_fn_t  func, void  *arg)
{
	__qdf_init_delayed_work(hdl, work, func, arg);
}

/**
 * qdf_create_workqueue - create a workqueue, This runs in non-interrupt
 * context, so can be preempted by H/W & S/W intr
 * @name: string
 * Return: pointer of type qdf_workqueue_t
 */
static inline qdf_workqueue_t *qdf_create_workqueue(char *name)
{
	return  __qdf_create_workqueue(name);
}

/**
 * qdf_queue_work - Queue the work/task
 * @hdl: OS handle
 * @wqueue: pointer to workqueue
 * @work: pointer to work
 * Return: none
 */
static inline void
qdf_queue_work(qdf_handle_t hdl, qdf_workqueue_t *wqueue, qdf_work_t *work)
{
	return  __qdf_queue_work(hdl, wqueue, work);
}

/**
 * qdf_queue_delayed_work - Queue the delayed work/task
 * @hdl: OS handle
 * @wqueue: pointer to workqueue
 * @work: pointer to work
 * @delay: delay interval
 * Return: none
 */
static inline void qdf_queue_delayed_work(qdf_handle_t hdl,
					  qdf_workqueue_t *wqueue,
					  qdf_delayed_work_t *work,
					  uint32_t delay)
{
	return  __qdf_queue_delayed_work(hdl, wqueue, work, delay);
}

/**
 * qdf_flush_workqueue - flush the workqueue
 * @hdl: OS handle
 * @wqueue: pointer to workqueue
 * Return: none
 */
static inline void qdf_flush_workqueue(qdf_handle_t hdl,
				       qdf_workqueue_t *wqueue)
{
	return  __qdf_flush_workqueue(hdl, wqueue);
}

/**
 * qdf_destroy_workqueue - Destroy the workqueue
 * @hdl: OS handle
 * @wqueue: pointer to workqueue
 * Return: none
 */
static inline void qdf_destroy_workqueue(qdf_handle_t hdl,
					 qdf_workqueue_t *wqueue)
{
	return  __qdf_destroy_workqueue(hdl, wqueue);
}

/**
 * qdf_sched_work - Schedule a deferred task on non-interrupt context
 * @hdl: OS handle
 * @work: pointer to work
 * Retrun: none
 */
static inline void qdf_sched_work(qdf_handle_t hdl, qdf_work_t *work)
{
	__qdf_sched_work(hdl, work);
}

/**
 * qdf_sched_delayed_work() - Schedule a delayed task
 * @hdl: OS handle
 * @work: pointer to delayed work
 * @delay: delay interval
 * Return: none
 */
static inline void qdf_sched_delayed_work(qdf_handle_t hdl,
					  qdf_delayed_work_t *work,
					  uint32_t delay)
{
	__qdf_sched_delayed_work(hdl, work, delay);
}

/**
 * qdf_cancel_work() - Cancel a work
 * @hdl: OS handle
 * @work: pointer to work
 *
 * Cancel work and wait for its execution to finish.
 * This function can be used even if the work re-queues
 * itself or migrates to another workqueue. On return
 * from this function, work is guaranteed to be not
 * pending or executing on any CPU. The caller must
 * ensure that the workqueue on which work was last
 * queued can't be destroyed before this function returns.
 *
 * Return: true if work was pending, false otherwise
 */
static inline bool qdf_cancel_work(qdf_handle_t hdl,
				   qdf_work_t *work)
{
	return __qdf_cancel_work(hdl, work);
}

/**
 * qdf_cancel_delayed_work() - Cancel a delayed work
 * @hdl: OS handle
 * @work: pointer to delayed work
 *
 * This is qdf_cancel_work for delayed works.
 *
 * Return: true if work was pending, false otherwise
 */
static inline bool qdf_cancel_delayed_work(qdf_handle_t hdl,
					   qdf_delayed_work_t *work)
{
	return __qdf_cancel_delayed_work(hdl, work);
}

/**
 * qdf_flush_work - Flush a deferred task on non-interrupt context
 * @hdl: OS handle
 * @work: pointer to work
 *
 * Wait until work has finished execution. work is guaranteed to be
 * idle on return if it hasn't been requeued since flush started.
 *
 * Return: none
 */
static inline void qdf_flush_work(qdf_handle_t hdl, qdf_work_t *work)
{
	__qdf_flush_work(hdl, work);
}

/**
 * qdf_flush_delayed_work() - Flush a delayed work
 * @hdl: OS handle
 * @work: pointer to delayed work
 *
 * This is qdf_flush_work for delayed works.
 *
 * Return: none
 */
static inline void qdf_flush_delayed_work(qdf_handle_t hdl,
					  qdf_delayed_work_t *work)
{
	__qdf_flush_delayed_work(hdl, work);
}

/**
 * qdf_disable_work - disable the deferred task (synchronous)
 * @hdl: OS handle
 * @work: pointer to work
 * Return: unsigned int
 */
static inline uint32_t qdf_disable_work(qdf_handle_t hdl, qdf_work_t *work)
{
	return __qdf_disable_work(hdl, work);
}

/**
 * qdf_destroy_work - destroy the deferred task (synchronous)
 * @hdl: OS handle
 * @work: pointer to work
 * Return: none
 */
static inline void qdf_destroy_work(qdf_handle_t hdl, qdf_work_t *work)
{
	__qdf_disable_work(hdl, work);
}

#endif /*_QDF_DEFER_H*/
