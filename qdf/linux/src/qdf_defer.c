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
 * DOC: qdf_defer.c
 * This file provides OS dependent deferred API's.
 */

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/workqueue.h>

#include "i_qdf_defer.h"

/**
 * __qdf_defer_func() - defer work handler
 * @work: Pointer to defer work
 *
 * Return: none
 */
void __qdf_defer_func(struct work_struct *work)
{
	__qdf_work_t *ctx = container_of(work, __qdf_work_t, work);
	if (ctx->fn == NULL) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "No callback registered !!");
		return;
	}
	ctx->fn(ctx->arg);
}
EXPORT_SYMBOL(__qdf_defer_func);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 19)
/**
 * __qdf_defer_delayed_func() - defer work handler
 * @dwork: Pointer to defer work
 *
 * Return: none
 */
void
__qdf_defer_delayed_func(struct work_struct *dwork)
{
	return;
}
#else
void
__qdf_defer_delayed_func(struct work_struct *dwork)
{
	__qdf_delayed_work_t  *ctx = container_of(dwork, __qdf_delayed_work_t,
		 dwork.work);
	if (ctx->fn == NULL) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "BugCheck: Callback is not initilized while creating delayed work queue");
		return;
	}
	ctx->fn(ctx->arg);
}
#endif
EXPORT_SYMBOL(__qdf_defer_delayed_func);
