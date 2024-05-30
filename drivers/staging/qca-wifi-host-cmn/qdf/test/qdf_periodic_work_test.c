/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#include "qdf_periodic_work.h"
#include "qdf_periodic_work_test.h"
#include "qdf_trace.h"

#define pwork_iterations 2
#define pwork_delay_ms 1

struct qdf_pwork_ut_ctx {
	struct qdf_periodic_work pwork;
	uint32_t count;
};

static void __qdf_pwork_inside_cb(void *context)
{
	struct qdf_pwork_ut_ctx *ut_ctx = context;

	/* stop before incrementing; the main thread is looking at @count */
	if (ut_ctx->count + 1 == pwork_iterations)
		qdf_periodic_work_stop_async(&ut_ctx->pwork);

	ut_ctx->count++;
}

static uint32_t qdf_pwork_stop_inside_cb(void)
{
	struct qdf_pwork_ut_ctx ut_ctx = { .count = 0 };
	QDF_STATUS status;

	status = qdf_periodic_work_create(&ut_ctx.pwork,
					  __qdf_pwork_inside_cb, &ut_ctx);
	QDF_BUG(QDF_IS_STATUS_SUCCESS(status));

	QDF_BUG(qdf_periodic_work_start(&ut_ctx.pwork, pwork_delay_ms));

	while (ut_ctx.count < pwork_iterations)
		schedule();

	QDF_BUG(!qdf_periodic_work_stop_sync(&ut_ctx.pwork));
	QDF_BUG(ut_ctx.count == pwork_iterations);

	qdf_periodic_work_destroy(&ut_ctx.pwork);

	return 0;
}

static void __qdf_pwork_outside_cb(void *context)
{
	struct qdf_pwork_ut_ctx *ut_ctx = context;

	ut_ctx->count++;
}

static uint32_t qdf_pwork_stop_outside_cb(void)
{
	struct qdf_pwork_ut_ctx ut_ctx = { .count = 0 };
	QDF_STATUS status;

	status = qdf_periodic_work_create(&ut_ctx.pwork,
					  __qdf_pwork_outside_cb, &ut_ctx);
	QDF_BUG(QDF_IS_STATUS_SUCCESS(status));

	QDF_BUG(qdf_periodic_work_start(&ut_ctx.pwork, pwork_delay_ms));

	while (ut_ctx.count < pwork_iterations)
		schedule();

	QDF_BUG(qdf_periodic_work_stop_sync(&ut_ctx.pwork));
	QDF_BUG(ut_ctx.count >= pwork_iterations);

	qdf_periodic_work_destroy(&ut_ctx.pwork);

	return 0;
}

uint32_t qdf_periodic_work_unit_test(void)
{
	uint32_t errors = 0;

	errors += qdf_pwork_stop_inside_cb();
	errors += qdf_pwork_stop_outside_cb();

	return errors;
}

