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

#ifndef _I_CDF_DEFER_H
#define _I_CDF_DEFER_H

#include <linux/version.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#endif
#include <cdf_types.h>
#include <cdf_status.h>
#include <qdf_status.h>
#include <cdf_trace.h>

typedef struct tasklet_struct __cdf_bh_t;

typedef void (*__cdf_bh_fn_t)(unsigned long arg);

/* wrapper around the real task func */
typedef struct {
	struct work_struct work;
	cdf_defer_fn_t fn;
	void *arg;
} __cdf_work_t;

extern void __cdf_defer_func(struct work_struct *work);

static inline CDF_STATUS
__cdf_init_work(__cdf_work_t *work, cdf_defer_fn_t func, void *arg)
{
	/*Initilize func and argument in work struct */
	work->fn = func;
	work->arg = arg;
#ifdef CONFIG_CNSS
	cnss_init_work(&work->work, __cdf_defer_func);
#else
	INIT_WORK(&work->work, __cdf_defer_func);
#endif
	return CDF_STATUS_SUCCESS;
}

static inline CDF_STATUS __cdf_schedule_work(__cdf_work_t *work)
{
	schedule_work(&work->work);
	return CDF_STATUS_SUCCESS;
}

static inline CDF_STATUS __cdf_init_bh(cdf_handle_t hdl,
				       struct tasklet_struct *bh,
				       cdf_defer_fn_t func, void *arg)
{
	tasklet_init(bh, (__cdf_bh_fn_t) func, (unsigned long)arg);

	return CDF_STATUS_SUCCESS;
}

static inline CDF_STATUS
__cdf_sched_bh(cdf_handle_t hdl, struct tasklet_struct *bh)
{
	tasklet_schedule(bh);

	return CDF_STATUS_SUCCESS;
}

static inline CDF_STATUS
__cdf_disable_bh(cdf_handle_t hdl, struct tasklet_struct *bh)
{
	tasklet_kill(bh);

	return CDF_STATUS_SUCCESS;
}

#endif /*_I_CDF_DEFER_H*/
