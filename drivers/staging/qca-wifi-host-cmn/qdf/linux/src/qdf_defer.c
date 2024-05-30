/*
 * Copyright (c) 2014-2019,2021 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_defer.c
 * This file provides OS dependent deferred API's.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>

#include "i_qdf_defer.h"
#include <qdf_module.h>
#include <qdf_defer.h>

void __qdf_defer_func(struct work_struct *work)
{
	__qdf_work_t *ctx = container_of(work, __qdf_work_t, work);

	if (!ctx->fn) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "No callback registered !!");
		return;
	}
	ctx->fn(ctx->arg);
}
qdf_export_symbol(__qdf_defer_func);

void __qdf_bh_func(unsigned long arg)
{
	__qdf_bh_t *ctx = (__qdf_bh_t *)arg;

	if (!ctx->fn) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "No callback registered !!");
		return;
	}
	ctx->fn(ctx->arg);
}
qdf_export_symbol(__qdf_bh_func);

#ifdef ENHANCED_OS_ABSTRACTION
void
qdf_create_bh(qdf_bh_t  *bh, qdf_defer_fn_t  func, void  *arg)
{
	__qdf_init_bh(bh, func, arg);
}

void qdf_sched_bh(qdf_bh_t *bh)
{
	__qdf_sched_bh(bh);
}

void qdf_destroy_bh(qdf_bh_t *bh)
{
	__qdf_disable_bh(bh);
}

void qdf_destroy_work(qdf_handle_t hdl, qdf_work_t *work)
{
	__qdf_disable_work(work);
}

qdf_export_symbol(qdf_destroy_work);

void qdf_flush_work(qdf_work_t *work)
{
	__qdf_flush_work(work);
}

qdf_export_symbol(qdf_flush_work);

uint32_t qdf_disable_work(qdf_work_t *work)
{
	return __qdf_disable_work(work);
}

qdf_export_symbol(qdf_disable_work);

bool qdf_cancel_work(qdf_work_t *work)
{
	return __qdf_cancel_work(work);
}

qdf_export_symbol(qdf_cancel_work);

qdf_workqueue_t *qdf_create_workqueue(char *name)
{
	return  __qdf_create_workqueue(name);
}

qdf_export_symbol(qdf_create_workqueue);

qdf_workqueue_t *qdf_create_singlethread_workqueue(char *name)
{
	return  __qdf_create_singlethread_workqueue(name);
}

qdf_export_symbol(qdf_create_singlethread_workqueue);

void qdf_destroy_workqueue(qdf_handle_t hdl,
			   qdf_workqueue_t *wqueue)
{
	return  __qdf_destroy_workqueue(wqueue);
}

qdf_export_symbol(qdf_destroy_workqueue);

qdf_workqueue_t *qdf_alloc_unbound_workqueue(char *name)
{
	return  __qdf_alloc_unbound_workqueue(name);
}

qdf_export_symbol(qdf_alloc_unbound_workqueue);

QDF_STATUS qdf_create_work(qdf_handle_t hdl, qdf_work_t  *work,
			   qdf_defer_fn_t  func, void  *arg)
{
	return __qdf_init_work(work, func, arg);
}

qdf_export_symbol(qdf_create_work);

void qdf_sched_work(qdf_handle_t hdl, qdf_work_t *work)
{
	__qdf_sched_work(work);
}

qdf_export_symbol(qdf_sched_work);

bool
qdf_queue_work(qdf_handle_t hdl, qdf_workqueue_t *wqueue, qdf_work_t *work)
{
	return  __qdf_queue_work(wqueue, work);
}

qdf_export_symbol(qdf_queue_work);

void qdf_flush_workqueue(qdf_handle_t hdl, qdf_workqueue_t *wqueue)
{
	return  __qdf_flush_workqueue(wqueue);
}

qdf_export_symbol(qdf_flush_workqueue);
#endif
