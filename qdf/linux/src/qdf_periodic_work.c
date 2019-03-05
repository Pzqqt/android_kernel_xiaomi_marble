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
#include "qdf_status.h"
#include "qdf_trace.h"
#include "qdf_types.h"

static void __qdf_periodic_work_handler(struct work_struct *work)
{
	struct qdf_periodic_work *pwork =
		container_of(work, struct qdf_periodic_work, dwork.work);
	uint32_t msec;

	pwork->callback(pwork->context);

	/* this is intentionally racy; see qdf_periodic_work_stop_sync() */
	msec = pwork->msec;
	if (msec)
		schedule_delayed_work(&pwork->dwork, msecs_to_jiffies(msec));
}

QDF_STATUS qdf_periodic_work_create(struct qdf_periodic_work *pwork,
				    qdf_periodic_work_cb callback,
				    void *context)
{
	QDF_BUG(pwork);
	QDF_BUG(callback);
	if (!pwork || !callback)
		return QDF_STATUS_E_INVAL;

	INIT_DELAYED_WORK(&pwork->dwork, __qdf_periodic_work_handler);
	pwork->callback = callback;
	pwork->context = context;
	pwork->msec = 0;

	return QDF_STATUS_SUCCESS;
}

void qdf_periodic_work_destroy(struct qdf_periodic_work *pwork)
{
	qdf_periodic_work_stop_sync(pwork);
}

bool qdf_periodic_work_start(struct qdf_periodic_work *pwork, uint32_t msec)
{
	QDF_BUG(msec);
	if (!msec)
		return false;

	pwork->msec = msec;

	return schedule_delayed_work(&pwork->dwork, msecs_to_jiffies(msec));
}

bool qdf_periodic_work_stop_async(struct qdf_periodic_work *pwork)
{
	bool pending = pwork->msec != 0;

	pwork->msec = 0;
	cancel_delayed_work(&pwork->dwork);

	return pending;
}

bool qdf_periodic_work_stop_sync(struct qdf_periodic_work *pwork)
{
	bool pending = pwork->msec != 0;

	/* To avoid using a lock, signal that the work shouldn't be restarted,
	 * and cancel_sync in a loop. There is a very small race window, and
	 * thus the work may ocassionally need to be cancelled more than once.
	 */
	pwork->msec = 0;
	while (cancel_delayed_work_sync(&pwork->dwork))
		; /* no-op*/

	return pending;
}

