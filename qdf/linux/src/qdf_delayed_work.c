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

#include "qdf_delayed_work.h"
#include "qdf_status.h"
#include "qdf_trace.h"
#include "qdf_types.h"

static void __qdf_delayed_work_handler(struct work_struct *work)
{
	struct qdf_delayed_work *dwork =
		container_of(work, struct qdf_delayed_work, dwork.work);

	dwork->callback(dwork->context);
}

QDF_STATUS qdf_delayed_work_create(struct qdf_delayed_work *dwork,
				   qdf_delayed_work_cb callback,
				   void *context)
{
	QDF_BUG(dwork);
	QDF_BUG(callback);
	if (!dwork || !callback)
		return QDF_STATUS_E_INVAL;

	INIT_DELAYED_WORK(&dwork->dwork, __qdf_delayed_work_handler);
	dwork->callback = callback;
	dwork->context = context;

	return QDF_STATUS_SUCCESS;
}

void qdf_delayed_work_destroy(struct qdf_delayed_work *dwork)
{
	qdf_delayed_work_stop_sync(dwork);
}

bool qdf_delayed_work_start(struct qdf_delayed_work *dwork, uint32_t msec)
{
	return schedule_delayed_work(&dwork->dwork, msecs_to_jiffies(msec));
}

bool qdf_delayed_work_stop_sync(struct qdf_delayed_work *dwork)
{
	return cancel_delayed_work_sync(&dwork->dwork);
}

