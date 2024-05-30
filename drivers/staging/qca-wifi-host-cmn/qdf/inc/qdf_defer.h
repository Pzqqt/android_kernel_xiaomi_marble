/*
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
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
typedef __qdf_workqueue_t     qdf_workqueue_t;

/*
 * Representation of a bottom half.
 */
typedef __qdf_bh_t       qdf_bh_t;

#ifdef ENHANCED_OS_ABSTRACTION
/**
 * qdf_create_bh - creates the bottom half deferred handler
 * @bh: pointer to bottom
 * @func: deferred function to run at bottom half interrupt context.
 * @arg: argument for the deferred function
 * Return: none
 */
void
qdf_create_bh(qdf_bh_t  *bh, qdf_defer_fn_t  func, void  *arg);

/**
 * qdf_sched - schedule a bottom half (DPC)
 * @bh: pointer to bottom
 * Return: none
 */
void qdf_sched_bh(qdf_bh_t *bh);

/**
 * qdf_destroy_bh - destroy the bh (synchronous)
 * @bh: pointer to bottom
 * Return: none
 */
void qdf_destroy_bh(qdf_bh_t *bh);

/**
 * qdf_create_workqueue - create a workqueue, This runs in non-interrupt
 * context, so can be preempted by H/W & S/W intr
 * @name: string
 *
 * Return: pointer of type qdf_workqueue_t
 */
qdf_workqueue_t *qdf_create_workqueue(char *name);

/**
 * qdf_create_singlethread_workqueue() - create a single threaded workqueue
 * @name: string
 *
 * This API creates a dedicated work queue with a single worker thread to avoid
 * wasting unnecessary resources when works which needs to be submitted in this
 * queue are not very critical and frequent.
 *
 * Return: pointer of type qdf_workqueue_t
 */
qdf_workqueue_t *qdf_create_singlethread_workqueue(char *name);

/**
 * qdf_alloc_unbound_workqueue - allocate an unbound workqueue
 * @name: string
 *
 * Return: pointer of type qdf_workqueue_t
 */
qdf_workqueue_t *qdf_alloc_unbound_workqueue(char *name);

/**
 * qdf_destroy_workqueue - Destroy the workqueue
 * @hdl: OS handle
 * @wqueue: pointer to workqueue
 *
 * Return: none
 */
void qdf_destroy_workqueue(qdf_handle_t hdl, qdf_workqueue_t *wqueue);

/**
 * qdf_cancel_work() - Cancel a work
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
bool qdf_cancel_work(qdf_work_t *work);

/**
 * qdf_disable_work - disable the deferred task (synchronous)
 * @work: pointer to work
 *
 * Return: unsigned int
 */
uint32_t qdf_disable_work(qdf_work_t *work);

/**
 * qdf_flush_work - Flush a deferred task on non-interrupt context
 * @work: pointer to work
 *
 * Wait until work has finished execution. work is guaranteed to be
 * idle on return if it hasn't been requeued since flush started.
 *
 * Return: none
 */
void qdf_flush_work(qdf_work_t *work);

/**
 * qdf_create_work - create a work/task queue, This runs in non-interrupt
 * context, so can be preempted by H/W & S/W intr
 * @hdl: OS handle
 * @work: pointer to work
 * @func: deferred function to run at bottom half non-interrupt context.
 * @arg: argument for the deferred function
 *
 * Return: QDF status
 */
QDF_STATUS qdf_create_work(qdf_handle_t hdl, qdf_work_t  *work,
			   qdf_defer_fn_t  func, void  *arg);

/**
 * qdf_sched_work - Schedule a deferred task on non-interrupt context
 * @hdl: OS handle
 * @work: pointer to work
 *
 * Retrun: none
 */
void qdf_sched_work(qdf_handle_t hdl, qdf_work_t *work);

/**
 * qdf_queue_work - Queue the work/task
 * @hdl: OS handle
 * @wqueue: pointer to workqueue
 * @work: pointer to work
 *
 * Return: false if work was already on a queue, true otherwise
 */
bool
qdf_queue_work(qdf_handle_t hdl, qdf_workqueue_t *wqueue, qdf_work_t *work);

/**
 * qdf_flush_workqueue - flush the workqueue
 * @hdl: OS handle
 * @wqueue: pointer to workqueue
 *
 * Return: none
 */
void qdf_flush_workqueue(qdf_handle_t hdl, qdf_workqueue_t *wqueue);

/**
 * qdf_destroy_work - destroy the deferred task (synchronous)
 * @hdl: OS handle
 * @work: pointer to work
 *
 * Return: none
 */
void qdf_destroy_work(qdf_handle_t hdl, qdf_work_t *work);
#else
/**
 * qdf_create_bh - creates the bottom half deferred handler
 * @bh: pointer to bottom
 * @func: deferred function to run at bottom half interrupt context.
 * @arg: argument for the deferred function
 * Return: none
 */
static inline void
qdf_create_bh(qdf_bh_t  *bh, qdf_defer_fn_t  func, void  *arg)
{
	__qdf_init_bh(bh, func, arg);
}

/**
 * qdf_sched - schedule a bottom half (DPC)
 * @bh: pointer to bottom
 * Return: none
 */
static inline void qdf_sched_bh(qdf_bh_t *bh)
{
	__qdf_sched_bh(bh);
}

/**
 * qdf_destroy_bh - destroy the bh (synchronous)
 * @bh: pointer to bottom
 * Return: none
 */
static inline void qdf_destroy_bh(qdf_bh_t *bh)
{
	__qdf_disable_bh(bh);
}

/*********************Non-Interrupt Context deferred Execution***************/

/**
 * qdf_create_work - create a work/task queue, This runs in non-interrupt
 * context, so can be preempted by H/W & S/W intr
 * @hdl: OS handle
 * @work: pointer to work
 * @func: deferred function to run at bottom half non-interrupt context.
 * @arg: argument for the deferred function
 *
 * Return: QDF status
 */
static inline QDF_STATUS qdf_create_work(qdf_handle_t hdl, qdf_work_t  *work,
				   qdf_defer_fn_t  func, void  *arg)
{
	return __qdf_init_work(work, func, arg);
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
 * qdf_create_singlethread_workqueue() - create a single threaded workqueue
 * @name: string
 *
 * This API creates a dedicated work queue with a single worker thread to avoid
 * wasting unnecessary resources when works which needs to be submitted in this
 * queue are not very critical and frequent.
 *
 * Return: pointer of type qdf_workqueue_t
 */
static inline qdf_workqueue_t *qdf_create_singlethread_workqueue(char *name)
{
	return  __qdf_create_singlethread_workqueue(name);
}

/**
 * qdf_alloc_high_prior_ordered_workqueue - alloc high-prior ordered workqueue
 * @name: string
 *
 * Return: pointer of type qdf_workqueue_t
 */
static inline
qdf_workqueue_t *qdf_alloc_high_prior_ordered_workqueue(char *name)
{
	return __qdf_alloc_high_prior_ordered_workqueue(name);
}

/**
 * qdf_alloc_unbound_workqueue - allocate an unbound workqueue
 * @name: string
 *
 * Return: pointer of type qdf_workqueue_t
 */
static inline qdf_workqueue_t *qdf_alloc_unbound_workqueue(char *name)
{
	return  __qdf_alloc_unbound_workqueue(name);
}

/**
 * qdf_queue_work - Queue the work/task
 * @hdl: OS handle
 * @wqueue: pointer to workqueue
 * @work: pointer to work
 * Return: false if work was already on a queue, true otherwise
 */
static inline bool
qdf_queue_work(qdf_handle_t hdl, qdf_workqueue_t *wqueue, qdf_work_t *work)
{
	return __qdf_queue_work(wqueue, work);
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
	return  __qdf_flush_workqueue(wqueue);
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
	return  __qdf_destroy_workqueue(wqueue);
}

/**
 * qdf_sched_work - Schedule a deferred task on non-interrupt context
 * @hdl: OS handle
 * @work: pointer to work
 * Retrun: none
 */
static inline void qdf_sched_work(qdf_handle_t hdl, qdf_work_t *work)
{
	__qdf_sched_work(work);
}

/**
 * qdf_cancel_work() - Cancel a work
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
static inline bool qdf_cancel_work(qdf_work_t *work)
{
	return __qdf_cancel_work(work);
}

/**
 * qdf_flush_work - Flush a deferred task on non-interrupt context
 * @work: pointer to work
 *
 * Wait until work has finished execution. work is guaranteed to be
 * idle on return if it hasn't been requeued since flush started.
 *
 * Return: none
 */
static inline void qdf_flush_work(qdf_work_t *work)
{
	__qdf_flush_work(work);
}

/**
 * qdf_disable_work - disable the deferred task (synchronous)
 * @work: pointer to work
 * Return: unsigned int
 */
static inline uint32_t qdf_disable_work(qdf_work_t *work)
{
	return __qdf_disable_work(work);
}

/**
 * qdf_destroy_work - destroy the deferred task (synchronous)
 * @hdl: OS handle
 * @work: pointer to work
 * Return: none
 */
static inline void qdf_destroy_work(qdf_handle_t hdl, qdf_work_t *work)
{
	__qdf_disable_work(work);
}
#endif

#endif /*_QDF_DEFER_H*/
