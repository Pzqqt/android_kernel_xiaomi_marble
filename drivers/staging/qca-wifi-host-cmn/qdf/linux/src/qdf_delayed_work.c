/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
#include "qdf_module.h"

#ifdef WLAN_DELAYED_WORK_DEBUG
#include "qdf_tracker.h"

#define qdf_dwork_tracker_bits 2 /* 4 buckets */
static qdf_tracker_declare(qdf_dwork_tracker, qdf_dwork_tracker_bits,
			   "delayed work leaks", "delayed work create",
			   "delayed work destroy");

void qdf_delayed_work_feature_init(void)
{
	qdf_tracker_init(&qdf_dwork_tracker);
}

void qdf_delayed_work_feature_deinit(void)
{
	qdf_tracker_deinit(&qdf_dwork_tracker);
}

void qdf_delayed_work_check_for_leaks(void)
{
	qdf_tracker_check_for_leaks(&qdf_dwork_tracker);
}

static inline QDF_STATUS qdf_dwork_dbg_track(struct qdf_delayed_work *dwork,
					     const char *func, uint32_t line)
{
	return qdf_tracker_track(&qdf_dwork_tracker, dwork, func, line);
}

static inline void qdf_dwork_dbg_untrack(struct qdf_delayed_work *dwork,
					 const char *func, uint32_t line)
{
	qdf_tracker_untrack(&qdf_dwork_tracker, dwork, func, line);
}
#else
static inline QDF_STATUS qdf_dwork_dbg_track(struct qdf_delayed_work *dwork,
					     const char *func, uint32_t line)
{
	return QDF_STATUS_SUCCESS;
}

static inline void qdf_dwork_dbg_untrack(struct qdf_delayed_work *dwork,
					 const char *func, uint32_t line)
{ }
#endif /* WLAN_DELAYED_WORK_DEBUG */

static void __qdf_delayed_work_handler(struct work_struct *work)
{
	struct qdf_delayed_work *dwork =
		container_of(work, struct qdf_delayed_work, dwork.work);

	dwork->callback(dwork->context);
}

QDF_STATUS __qdf_delayed_work_create(struct qdf_delayed_work *dwork,
				     qdf_delayed_work_cb callback,
				     void *context,
				     const char *func, uint32_t line)
{
	QDF_STATUS status;

	QDF_BUG(dwork);
	QDF_BUG(callback);
	if (!dwork || !callback)
		return QDF_STATUS_E_INVAL;

	status = qdf_dwork_dbg_track(dwork, func, line);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	INIT_DELAYED_WORK(&dwork->dwork, __qdf_delayed_work_handler);
	dwork->callback = callback;
	dwork->context = context;

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(__qdf_delayed_work_create);

void __qdf_delayed_work_destroy(struct qdf_delayed_work *dwork,
				const char *func, uint32_t line)
{
	qdf_delayed_work_stop_sync(dwork);
	qdf_dwork_dbg_untrack(dwork, func, line);
}

qdf_export_symbol(__qdf_delayed_work_destroy);

bool __qdf_delayed_work_start(struct qdf_delayed_work *dwork, uint32_t msec)
{
	return schedule_delayed_work(&dwork->dwork, msecs_to_jiffies(msec));
}

qdf_export_symbol(__qdf_delayed_work_start);

bool __qdf_delayed_work_stop_sync(struct qdf_delayed_work *dwork)
{
	return cancel_delayed_work_sync(&dwork->dwork);
}

qdf_export_symbol(__qdf_delayed_work_stop_sync);

bool __qdf_delayed_work_stop(struct qdf_delayed_work *dwork)
{
	return cancel_delayed_work(&dwork->dwork);
}

qdf_export_symbol(__qdf_delayed_work_stop);
