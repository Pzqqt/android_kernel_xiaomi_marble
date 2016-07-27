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
 * DOC: i_qdf_defer.h
 * This file provides OS dependent deferred API's.
 */

#ifndef _I_QDF_DEFER_H
#define _I_QDF_DEFER_H

#include <linux/version.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <qdf_types.h>
#include <qdf_status.h>
#include <qdf_trace.h>

typedef struct tasklet_struct __qdf_bh_t;
typedef struct workqueue_struct __qdf_workqueue_t;

#if LINUX_VERSION_CODE  <= KERNEL_VERSION(2, 6, 19)
typedef struct work_struct      __qdf_work_t;
typedef struct work_struct      __qdf_delayed_work_t;
#else

/**
 * __qdf_work_t - wrapper around the real task func
 * @work: Instance of work
 * @fn: function pointer to the handler
 * @arg: pointer to argument
 */
typedef struct {
	struct work_struct   work;
	qdf_defer_fn_t    fn;
	void                 *arg;
} __qdf_work_t;

/**
 * __qdf_delayed_work_t - wrapper around the real work func
 * @dwork: Instance of delayed work
 * @fn: function pointer to the handler
 * @arg: pointer to argument
 */
typedef struct {
	struct delayed_work  dwork;
	qdf_defer_fn_t    fn;
	void                 *arg;
} __qdf_delayed_work_t;

extern void __qdf_defer_func(struct work_struct *work);
extern void __qdf_defer_delayed_func(struct work_struct *work);
#endif

typedef void (*__qdf_bh_fn_t)(unsigned long arg);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 19)
/**
 * __qdf_init_work - Initialize a work/task queue, This runs in non-interrupt
 * context, so can be preempted by H/W & S/W intr
 * @hdl: OS handle
 * @work: pointer to work
 * @func: deferred function to run at bottom half non-interrupt context.
 * @arg: argument for the deferred function
 * Return: none
 */
static inline QDF_STATUS __qdf_init_work(qdf_handle_t hdl,
					 __qdf_work_t *work,
					 qdf_defer_fn_t func, void *arg)
{
	/*Initilize func and argument in work struct */
	INIT_WORK(&work->work, __qdf_defer_func);
	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_init_delayed_work - create a work/task, This runs in non-interrupt
 * context, so can be preempted by H/W & S/W intr
 * @hdl: OS handle
 * @work: pointer to work
 * @func: deferred function to run at bottom half non-interrupt context.
 * @arg: argument for the deferred function
 * Return: none
 */
static inline uint32_t __qdf_init_delayed_work(qdf_handle_t hdl,
					       __qdf_delayed_work_t *work,
					       qdf_defer_fn_t func, void *arg)
{
	INIT_WORK(work, func, arg);
	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_queue_work - Queue the work/task
 * @hdl: OS handle
 * @wqueue: pointer to workqueue
 * @work: pointer to work
 * Return: none
 */
static inline void __qdf_queue_work(qdf_handle_t hdl,
				    __qdf_workqueue_t *wqueue,
				    __qdf_work_t *work)
{
	queue_work(wqueue, work);
}

/**
 * __qdf_queue_delayed_work - Queue the delayed work/task
 * @hdl: OS handle
 * @wqueue: pointer to workqueue
 * @work: pointer to work
 * @delay: delay interval
 * Return: none
 */
static inline void __qdf_queue_delayed_work(qdf_handle_t hdl,
					    __qdf_workqueue_t *wqueue,
					    __qdf_delayed_work_t *work,
					    uint32_t delay)
{
	queue_delayed_work(wqueue, work, delay);
}

/**
 * __qdf_sched_work - Schedule a deferred task on non-interrupt context
 * @hdl: OS handle
 * @work: pointer to work
 * Retrun: none
 */
static inline QDF_STATUS __qdf_sched_work(qdf_handle_t hdl, __qdf_work_t *work)
{
	schedule_work(work);
	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_sched_delayed_work() - Schedule a delayed work
 * @hdl: OS handle
 * @work: pointer to delayed work
 * @delay: delay interval
 * Return: none
 */
static inline QDF_STATUS __qdf_sched_delayed_work(qdf_handle_t hdl,
						  __qdf_delayed_work_t *work,
						  uint32_t delay)
{
	schedule_delayed_work(work, delay);
	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_cancel_work() - Cancel a work
 * @hdl: OS handle
 * @work: pointer to work
 * Return: true if work was pending, false otherwise
 */
static inline bool __qdf_cancel_work(qdf_handle_t hdl,
					   __qdf_work_t *work)
{
	return cancel_work_sync(work);
}

/**
 * __qdf_cancel_delayed_work() - Cancel a delayed work
 * @hdl: OS handle
 * @work: pointer to delayed work
 * Return: true if work was pending, false otherwise
 */
static inline bool __qdf_cancel_delayed_work(qdf_handle_t hdl,
					     __qdf_delayed_work_t *work)
{
	return cancel_delayed_work_sync(work);
}

/**
 * __qdf_flush_work - Flush a deferred task on non-interrupt context
 * @hdl: OS handle
 * @work: pointer to work
 * Return: none
 */
static inline uint32_t __qdf_flush_work(qdf_handle_t hdl, __qdf_work_t *work)
{
	flush_work(work);
	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_flush_delayed_work() - Flush a delayed work
 * @hdl: OS handle
 * @work: pointer to delayed work
 * Return: none
 */
static inline uint32_t __qdf_flush_delayed_work(qdf_handle_t hdl,
						__qdf_delayed_work_t *work)
{
	flush_delayed_work(work);
	return QDF_STATUS_SUCCESS;
}

#else
static inline QDF_STATUS __qdf_init_work(qdf_handle_t hdl,
					 __qdf_work_t *work,
					 qdf_defer_fn_t func, void *arg)
{
	work->fn = func;
	work->arg = arg;
	INIT_WORK(&work->work, __qdf_defer_func);
	return QDF_STATUS_SUCCESS;
}

static inline uint32_t __qdf_init_delayed_work(qdf_handle_t hdl,
					       __qdf_delayed_work_t *work,
					       qdf_defer_fn_t func, void *arg)
{
	/*Initilize func and argument in work struct */
	work->fn = func;
	work->arg = arg;
	INIT_DELAYED_WORK(&work->dwork, __qdf_defer_delayed_func);
	return QDF_STATUS_SUCCESS;
}

static inline void __qdf_queue_work(qdf_handle_t hdl,
				    __qdf_workqueue_t *wqueue,
				    __qdf_work_t *work)
{
	queue_work(wqueue, &work->work);
}

static inline void __qdf_queue_delayed_work(qdf_handle_t hdl,
					    __qdf_workqueue_t *wqueue,
					    __qdf_delayed_work_t *work,
					    uint32_t delay)
{
	queue_delayed_work(wqueue, &work->dwork, delay);
}

static inline QDF_STATUS __qdf_sched_work(qdf_handle_t hdl, __qdf_work_t *work)
{
	schedule_work(&work->work);
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS __qdf_sched_delayed_work(qdf_handle_t hdl,
						  __qdf_delayed_work_t *work,
						  uint32_t delay)
{
	schedule_delayed_work(&work->dwork, delay);
	return QDF_STATUS_SUCCESS;
}

static inline bool __qdf_cancel_work(qdf_handle_t hdl,
					   __qdf_work_t *work)
{
	return cancel_work_sync(&work->work);
}

static inline bool __qdf_cancel_delayed_work(qdf_handle_t hdl,
					     __qdf_delayed_work_t *work)
{
	return cancel_delayed_work_sync(&work->dwork);
}

static inline uint32_t __qdf_flush_work(qdf_handle_t hdl, __qdf_work_t *work)
{
	flush_work(&work->work);
	return QDF_STATUS_SUCCESS;
}
static inline uint32_t __qdf_flush_delayed_work(qdf_handle_t hdl,
						__qdf_delayed_work_t *work)
{
	flush_delayed_work(&work->dwork);
	return QDF_STATUS_SUCCESS;
}

#endif

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
 * __qdf_flush_workqueue - flush the workqueue
 * @hdl: OS handle
 * @wqueue: pointer to workqueue
 * Return: none
 */
static inline void __qdf_flush_workqueue(qdf_handle_t hdl,
	__qdf_workqueue_t *wqueue)
{
	flush_workqueue(wqueue);
}

/**
 * __qdf_destroy_workqueue - Destroy the workqueue
 * @hdl: OS handle
 * @wqueue: pointer to workqueue
 * Return: none
 */
static inline void __qdf_destroy_workqueue(qdf_handle_t hdl,
	 __qdf_workqueue_t *wqueue)
{
	destroy_workqueue(wqueue);
}

/**
 * __qdf_init_bh - creates the Bottom half deferred handler
 * @hdl: OS handle
 * @bh: pointer to bottom
 * @func: deferred function to run at bottom half interrupt context.
 * @arg: argument for the deferred function
 * Return: none
 */
static inline QDF_STATUS __qdf_init_bh(qdf_handle_t hdl,
			struct tasklet_struct *bh,
			qdf_defer_fn_t func, void *arg)
{
	tasklet_init(bh, (__qdf_bh_fn_t) func, (unsigned long)arg);
	return QDF_STATUS_SUCCESS;
}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 19)
#else
#endif

/**
 * __qdf_sched_bh - schedule a bottom half (DPC)
 * @hdl: OS handle
 * @bh: pointer to bottom
 * Return: none
 */
static inline QDF_STATUS
__qdf_sched_bh(qdf_handle_t hdl, struct tasklet_struct *bh)
{
	tasklet_schedule(bh);
	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_disable_work - disable the deferred task (synchronous)
 * @hdl: OS handle
 * @work: pointer to work
 * Return: unsigned int
 */
static inline QDF_STATUS
__qdf_disable_work(qdf_handle_t hdl, __qdf_work_t *work)
{
	if (cancel_work_sync(&work->work))
		return QDF_STATUS_E_ALREADY;

	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_disable_bh - destroy the bh (synchronous)
 * @hdl: OS handle
 * @bh: pointer to bottom
 * Return: none
 */
static inline QDF_STATUS
__qdf_disable_bh(qdf_handle_t hdl, struct tasklet_struct *bh)
{
	tasklet_kill(bh);
	return QDF_STATUS_SUCCESS;
}

#endif /*_I_QDF_DEFER_H*/
