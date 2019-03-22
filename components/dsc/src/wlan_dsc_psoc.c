/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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

#include "qdf_list.h"
#include "qdf_status.h"
#include "qdf_talloc.h"
#include "qdf_types.h"
#include "__wlan_dsc.h"
#include "wlan_dsc.h"

#define __dsc_driver_lock(psoc) __dsc_lock((psoc)->driver)
#define __dsc_driver_unlock(psoc) __dsc_unlock((psoc)->driver)

static QDF_STATUS
__dsc_psoc_create(struct dsc_driver *driver, struct dsc_psoc **out_psoc)
{
	struct dsc_psoc *psoc;

	if (!dsc_assert(driver))
		return QDF_STATUS_E_INVAL;

	if (!dsc_assert(out_psoc))
		return QDF_STATUS_E_INVAL;

	*out_psoc = NULL;

	psoc = qdf_talloc_type(driver, psoc);
	if (!psoc)
		return QDF_STATUS_E_NOMEM;

	/* init */
	psoc->driver = driver;
	qdf_list_create(&psoc->vdevs, 0);
	__dsc_trans_init(&psoc->trans);
	__dsc_ops_init(&psoc->ops);

	/* attach */
	__dsc_driver_lock(psoc);
	qdf_list_insert_back(&driver->psocs, &psoc->node);
	__dsc_driver_unlock(psoc);

	*out_psoc = psoc;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
dsc_psoc_create(struct dsc_driver *driver, struct dsc_psoc **out_psoc)
{
	QDF_STATUS status;

	dsc_enter();
	status =  __dsc_psoc_create(driver, out_psoc);
	dsc_exit();

	return status;
}

static void __dsc_psoc_destroy(struct dsc_psoc **out_psoc)
{
	struct dsc_psoc *psoc;

	if (!dsc_assert(out_psoc))
		return;

	psoc = *out_psoc;
	if (!dsc_assert(psoc))
		return;

	*out_psoc = NULL;

	/* assert no children */
	dsc_assert(qdf_list_empty(&psoc->vdevs));

	/* flush pending transitions */
	while (__dsc_trans_abort(&psoc->trans))
		;

	/* detach */
	__dsc_driver_lock(psoc);
	qdf_list_remove_node(&psoc->driver->psocs, &psoc->node);
	__dsc_driver_unlock(psoc);

	/* de-init */
	__dsc_ops_deinit(&psoc->ops);
	__dsc_trans_deinit(&psoc->trans);
	qdf_list_destroy(&psoc->vdevs);
	psoc->driver = NULL;

	qdf_tfree(psoc);
}

void dsc_psoc_destroy(struct dsc_psoc **out_psoc)
{
	dsc_enter();
	__dsc_psoc_destroy(out_psoc);
	dsc_exit();
}

static bool __dsc_psoc_trans_active_down_tree(struct dsc_psoc *psoc)
{
	struct dsc_vdev *vdev;

	dsc_for_each_psoc_vdev(psoc, vdev) {
		if (__dsc_trans_active(&vdev->trans))
			return true;
	}

	return false;
}

#define __dsc_psoc_can_op(psoc) __dsc_psoc_can_trans(psoc)

static bool __dsc_psoc_can_trans(struct dsc_psoc *psoc)
{
	return !__dsc_trans_active_or_queued(&psoc->driver->trans) &&
		!__dsc_trans_active_or_queued(&psoc->trans) &&
		!__dsc_psoc_trans_active_down_tree(psoc);
}

static bool __dsc_psoc_can_trigger(struct dsc_psoc *psoc)
{
	return !__dsc_trans_active_or_queued(&psoc->driver->trans) &&
		!__dsc_trans_active(&psoc->trans) &&
		!__dsc_psoc_trans_active_down_tree(psoc);
}

static QDF_STATUS
__dsc_psoc_trans_start_nolock(struct dsc_psoc *psoc, const char *desc)
{
	if (!__dsc_psoc_can_trans(psoc))
		return QDF_STATUS_E_AGAIN;

	return __dsc_trans_start(&psoc->trans, desc);
}

static QDF_STATUS
__dsc_psoc_trans_start(struct dsc_psoc *psoc, const char *desc)
{
	QDF_STATUS status;

	if (!dsc_assert(psoc))
		return QDF_STATUS_E_INVAL;

	if (!dsc_assert(desc))
		return QDF_STATUS_E_INVAL;

	__dsc_driver_lock(psoc);
	status = __dsc_psoc_trans_start_nolock(psoc, desc);
	__dsc_driver_unlock(psoc);

	return status;
}

QDF_STATUS dsc_psoc_trans_start(struct dsc_psoc *psoc, const char *desc)
{
	QDF_STATUS status;

	dsc_enter_str(desc);
	status = __dsc_psoc_trans_start(psoc, desc);
	dsc_exit_status(status);

	return status;
}

static QDF_STATUS
__dsc_psoc_trans_start_wait(struct dsc_psoc *psoc, const char *desc)
{
	QDF_STATUS status;
	struct dsc_tran tran = { 0 };

	if (!dsc_assert(psoc))
		return QDF_STATUS_E_INVAL;

	if (!dsc_assert(desc))
		return QDF_STATUS_E_INVAL;

	__dsc_driver_lock(psoc);

	/* try to start without waiting */
	status = __dsc_psoc_trans_start_nolock(psoc, desc);
	if (QDF_IS_STATUS_SUCCESS(status))
		goto unlock;

	status = __dsc_trans_queue(&psoc->trans, &tran, desc);
	if (QDF_IS_STATUS_ERROR(status))
		goto unlock;

	__dsc_driver_unlock(psoc);

	return __dsc_tran_wait(&tran);

unlock:
	__dsc_driver_unlock(psoc);

	return status;
}

QDF_STATUS dsc_psoc_trans_start_wait(struct dsc_psoc *psoc, const char *desc)
{
	QDF_STATUS status;

	dsc_enter_str(desc);
	status = __dsc_psoc_trans_start_wait(psoc, desc);
	dsc_exit_status(status);

	return status;
}

static void __dsc_psoc_trigger_trans(struct dsc_psoc *psoc)
{
	struct dsc_vdev *vdev;

	if (__dsc_driver_trans_trigger_checked(psoc->driver))
		return;

	if (__dsc_trans_trigger(&psoc->trans))
		return;

	dsc_for_each_psoc_vdev(psoc, vdev)
		__dsc_trans_trigger(&vdev->trans);
}

static void __dsc_psoc_trans_stop(struct dsc_psoc *psoc)
{
	if (!dsc_assert(psoc))
		return;

	__dsc_driver_lock(psoc);

	__dsc_trans_stop(&psoc->trans);
	__dsc_psoc_trigger_trans(psoc);

	__dsc_driver_unlock(psoc);
}

void dsc_psoc_trans_stop(struct dsc_psoc *psoc)
{
	dsc_enter();
	__dsc_psoc_trans_stop(psoc);
	dsc_exit();
}

static void __dsc_psoc_assert_trans_protected(struct dsc_psoc *psoc)
{
	if (!dsc_assert(psoc))
		return;

	__dsc_driver_lock(psoc);
	dsc_assert(__dsc_trans_active(&psoc->trans) ||
		   __dsc_trans_active(&psoc->driver->trans));
	__dsc_driver_unlock(psoc);
}

void dsc_psoc_assert_trans_protected(struct dsc_psoc *psoc)
{
	dsc_enter();
	__dsc_psoc_assert_trans_protected(psoc);
	dsc_exit();
}

bool __dsc_psoc_trans_trigger_checked(struct dsc_psoc *psoc)
{
	if (qdf_list_empty(&psoc->trans.queue))
		return false;

	/* handled, but don't trigger; we need to wait for more children */
	if (!__dsc_psoc_can_trigger(psoc))
		return true;

	return __dsc_trans_trigger(&psoc->trans);
}

static QDF_STATUS __dsc_psoc_op_start(struct dsc_psoc *psoc, const char *func)
{
	QDF_STATUS status;

	if (!dsc_assert(psoc))
		return QDF_STATUS_E_INVAL;

	if (!dsc_assert(func))
		return QDF_STATUS_E_INVAL;

	__dsc_driver_lock(psoc);

	if (!__dsc_psoc_can_op(psoc)) {
		status = QDF_STATUS_E_AGAIN;
		goto unlock;
	}

	status = __dsc_ops_insert(&psoc->ops, func);

unlock:
	__dsc_driver_unlock(psoc);

	return status;
}

QDF_STATUS _dsc_psoc_op_start(struct dsc_psoc *psoc, const char *func)
{
	QDF_STATUS status;

	dsc_enter_str(func);
	status = __dsc_psoc_op_start(psoc, func);
	dsc_exit_status(status);

	return status;
}

static void __dsc_psoc_op_stop(struct dsc_psoc *psoc, const char *func)
{
	if (!dsc_assert(psoc))
		return;

	if (!dsc_assert(func))
		return;

	__dsc_driver_lock(psoc);
	if (__dsc_ops_remove(&psoc->ops, func))
		qdf_event_set(&psoc->ops.event);
	__dsc_driver_unlock(psoc);
}

void _dsc_psoc_op_stop(struct dsc_psoc *psoc, const char *func)
{
	dsc_enter_str(func);
	__dsc_psoc_op_stop(psoc, func);
	dsc_exit();
}

static void __dsc_psoc_wait_for_ops(struct dsc_psoc *psoc)
{
	struct dsc_vdev *vdev;
	bool wait;

	if (!dsc_assert(psoc))
		return;

	__dsc_driver_lock(psoc);

	/* flushing without preventing new ops is almost certainly a bug */
	dsc_assert(!__dsc_psoc_can_op(psoc));

	wait = psoc->ops.count > 0;
	if (wait)
		qdf_event_reset(&psoc->ops.event);

	__dsc_driver_unlock(psoc);

	if (wait)
		qdf_wait_single_event(&psoc->ops.event, 0);

	/* wait for down-tree ops to complete as well */
	dsc_for_each_psoc_vdev(psoc, vdev)
		dsc_vdev_wait_for_ops(vdev);
}

void dsc_psoc_wait_for_ops(struct dsc_psoc *psoc)
{
	dsc_enter();
	__dsc_psoc_wait_for_ops(psoc);
	dsc_exit();
}

