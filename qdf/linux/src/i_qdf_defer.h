/*
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: i_qdf_defer.h
 * This file provides OS dependent deferred API's.
 */

#ifndef _I_QDF_DEFER_H
#define _I_QDF_DEFER_H

#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <qdf_types.h>
#include <qdf_status.h>
#include <qdf_trace.h>

typedef struct workqueue_struct __qdf_workqueue_t;

/**
 * __qdf_work_t - wrapper around the real task func
 * @work: Instance of work
 * @fn: function pointer to the handler
 * @arg: pointer to argument
 */
typedef struct {
	struct work_struct work;
	qdf_defer_fn_t fn;
	void *arg;
} __qdf_work_t;

/**
 * __qdf_bh_t - wrapper around the real task func
 * @bh: Instance of the bottom half
 * @fn: function pointer to the handler
 * @arg: pointer to argument
 */
typedef struct {
	struct tasklet_struct bh;
	qdf_defer_fn_t fn;
	void *arg;
} __qdf_bh_t;

/**
 * __qdf_defer_func() - Linux-specific defer work handler
 * @work: Pointer to defer work
 *
 * This function services all Linux-specific deferred work
 * and dispatches them to the correct handler using the
 * abstracted functional interface.
 *
 * Return: none
 */
void __qdf_defer_func(struct work_struct *work);

/**
 * __qdf_bh_func() - bottom half handler
 * @arg: Pointer to bottom half abstraction
 *
 * This function services all Linux-specific bottom halfs
 * and dispatches them to the correct handler using the
 * abstracted functional interface.
 *
 * Return: none
 */
void __qdf_bh_func(unsigned long arg);

/**
 * __qdf_init_work - Initialize a work/task queue, This runs in non-interrupt
 * context, so can be preempted by H/W & S/W intr
 * @work: pointer to work
 * @func: deferred function to run at bottom half non-interrupt context.
 * @arg: argument for the deferred function
 * Return: none
 */
static inline QDF_STATUS
__qdf_init_work(__qdf_work_t *work, qdf_defer_fn_t func, void *arg)
{
	work->fn = func;
	work->arg = arg;
	INIT_WORK(&work->work, __qdf_defer_func);
	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_queue_work - Queue the work/task
 * @wqueue: pointer to workqueue
 * @work: pointer to work
 * Return: false if work was already on a queue, true otherwise
 */
static inline bool
__qdf_queue_work(__qdf_workqueue_t *wqueue, __qdf_work_t *work)
{
	return queue_work(wqueue, &work->work);
}

/**
 * __qdf_sched_work - Schedule a deferred task on non-interrupt context
 * @work: pointer to work
 * Retrun: none
 */
static inline QDF_STATUS __qdf_sched_work(__qdf_work_t *work)
{
	schedule_work(&work->work);
	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_cancel_work() - Cancel a work
 * @work: pointer to work
 * Return: true if work was pending, false otherwise
 */
static inline bool __qdf_cancel_work(__qdf_work_t *work)
{
	return cancel_work_sync(&work->work);
}

/**
 * __qdf_flush_work - Flush a deferred task on non-interrupt context
 * @work: pointer to work
 * Return: none
 */
static inline uint32_t __qdf_flush_work(__qdf_work_t *work)
{
	flush_work(&work->work);
	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_create_workqueue - create a workqueue, This runs in non-interrupt
 * context, so can be preempted by H/W & S/W intr
 * @name: string
 * Return: pointer of type qdf_workqueue_t
 */
static inline __qdf_workqueue_t *__qdf_create_workqueue(char *name)
{
	return create_workqueue(name);
}

/**
 * __qdf_create_singlethread_workqueue() - create a single threaded workqueue
 * @name: string
 *
 * This API creates a dedicated work queue with a single worker thread to avoid
 * wasting unnecessary resources when works which needs to be submitted in this
 * queue are not very critical and frequent.
 *
 * Return: pointer of type qdf_workqueue_t
 */
static inline __qdf_workqueue_t *__qdf_create_singlethread_workqueue(char *name)
{
	return create_singlethread_workqueue(name);
}

/**
 * __qdf_alloc_high_prior_ordered_workqueue - alloc high-prior ordered workqueue
 * @name: string
 *
 * Return: pointer of type qdf_workqueue_t
 */
static inline
__qdf_workqueue_t *__qdf_alloc_high_prior_ordered_workqueue(char *name)
{
	return  alloc_ordered_workqueue(name, WQ_HIGHPRI);
}

/**
 * __qdf_alloc_unbound_workqueue - alloc an unbound workqueue
 * @name: string
 *
 * Return: pointer of type qdf_workqueue_t
 */
static inline __qdf_workqueue_t *__qdf_alloc_unbound_workqueue(char *name)
{
	return alloc_workqueue(name, WQ_UNBOUND, 0);
}

/**
 * __qdf_flush_workqueue - flush the workqueue
 * @wqueue: pointer to workqueue
 * Return: none
 */
static inline void __qdf_flush_workqueue(__qdf_workqueue_t *wqueue)
{
	flush_workqueue(wqueue);
}

/**
 * __qdf_destroy_workqueue - Destroy the workqueue
 * @wqueue: pointer to workqueue
 * Return: none
 */
static inline void __qdf_destroy_workqueue(__qdf_workqueue_t *wqueue)
{
	destroy_workqueue(wqueue);
}

/**
 * __qdf_init_bh - creates the Bottom half deferred handler
 * @bh: pointer to bottom
 * @func: deferred function to run at bottom half interrupt context.
 * @arg: argument for the deferred function
 *
 * Return: none
 */
static inline void __qdf_init_bh(__qdf_bh_t *bh, qdf_defer_fn_t func, void *arg)
{
	bh->fn = func;
	bh->arg = arg;
	tasklet_init(&bh->bh, __qdf_bh_func, (unsigned long)bh);
}

/**
 * __qdf_sched_bh - schedule a bottom half (DPC)
 * @bh: pointer to bottom
 *
 * Return: none
 */
static inline void __qdf_sched_bh(__qdf_bh_t *bh)
{
	tasklet_schedule(&bh->bh);
}

/**
 * __qdf_disable_work - disable the deferred task (synchronous)
 * @work: pointer to work
 * Return: unsigned int
 */
static inline QDF_STATUS __qdf_disable_work(__qdf_work_t *work)
{
	if (cancel_work_sync(&work->work))
		return QDF_STATUS_E_ALREADY;

	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_disable_bh - destroy the bh (synchronous)
 * @bh: pointer to bottom
 *
 * Return: none
 */
static inline void __qdf_disable_bh(__qdf_bh_t *bh)
{
	tasklet_kill(&bh->bh);
}

#endif /*_I_QDF_DEFER_H*/
