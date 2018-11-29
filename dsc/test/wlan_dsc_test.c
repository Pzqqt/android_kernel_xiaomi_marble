/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

#include "__wlan_dsc.h"
#include "qdf_event.h"
#include "qdf_threads.h"
#include "qdf_trace.h"
#include "qdf_types.h"
#include "wlan_dsc.h"
#include "wlan_dsc_test.h"

#define dsc_driver_trans_start(driver) dsc_driver_trans_start(driver, __func__)
#define dsc_psoc_trans_start(psoc) dsc_psoc_trans_start(psoc, __func__)
#define dsc_vdev_trans_start(vdev) dsc_vdev_trans_start(vdev, __func__)

#define dsc_driver_trans_start_wait(driver) \
	dsc_driver_trans_start_wait(driver, "")
#define dsc_psoc_trans_start_wait(psoc) \
	dsc_psoc_trans_start_wait(psoc, __func__)
#define dsc_vdev_trans_start_wait(vdev) \
	dsc_vdev_trans_start_wait(vdev, __func__)

static struct dsc_psoc *nth_psoc(struct dsc_driver *driver, int n)
{
	struct dsc_psoc *psoc;

	QDF_BUG(n > 0);
	if (n <= 0)
		return NULL;

	dsc_for_each_driver_psoc(driver, psoc) {
		n--;
		if (n)
			continue;

		return psoc;
	}

	QDF_DEBUG_PANIC();

	return NULL;
}

static struct dsc_vdev *nth_vdev(struct dsc_psoc *psoc, int n)
{
	struct dsc_vdev *vdev;

	QDF_BUG(n > 0);
	if (n <= 0)
		return NULL;

	dsc_for_each_psoc_vdev(psoc, vdev) {
		n--;
		if (n)
			continue;

		return vdev;
	}

	QDF_DEBUG_PANIC();

	return NULL;
}

static void __dsc_tree_destroy(struct dsc_driver *driver)
{
	struct dsc_psoc *psoc;
	struct dsc_psoc *next_psoc;

	QDF_BUG(driver);

	qdf_list_for_each_del(&driver->psocs, psoc, next_psoc, node) {
		struct dsc_vdev *vdev;
		struct dsc_vdev *next_vdev;

		qdf_list_for_each_del(&psoc->vdevs, vdev, next_vdev, node)
			dsc_vdev_destroy(&vdev);

		dsc_psoc_destroy(&psoc);
	}

	dsc_driver_destroy(&driver);
}

static QDF_STATUS __dsc_tree_create(struct dsc_driver **out_driver,
				    uint8_t psocs_per_driver,
				    uint8_t vdevs_per_psoc)
{
	QDF_STATUS status;
	struct dsc_driver *driver;
	int i, j;

	status = dsc_driver_create(&driver);
	if (QDF_IS_STATUS_ERROR(status)) {
		dsc_err("Failed to create driver; status:%u", status);
		return status;
	}

	for (i = 0; i < psocs_per_driver; i++) {
		struct dsc_psoc *psoc;

		status = dsc_psoc_create(driver, &psoc);
		if (QDF_IS_STATUS_ERROR(status)) {
			dsc_err("Failed to create psoc; status:%u", status);
			goto free_tree;
		}

		for (j = 0; j < vdevs_per_psoc; j++) {
			struct dsc_vdev *vdev;

			status = dsc_vdev_create(psoc, &vdev);
			if (QDF_IS_STATUS_ERROR(status)) {
				dsc_err("Failed to create vdev; status:%u",
					status);
				goto free_tree;
			}
		}
	}

	*out_driver = driver;

	return QDF_STATUS_SUCCESS;

free_tree:
	__dsc_tree_destroy(driver);

	return status;
}

static uint32_t dsc_test_create_destroy(void)
{
	uint32_t errors = 0;
	QDF_STATUS status;
	struct dsc_driver *driver;

	dsc_enter();

	status = __dsc_tree_create(&driver, 2, 2);
	if (QDF_IS_STATUS_ERROR(status)) {
		errors++;
		goto exit;
	}

	__dsc_tree_destroy(driver);

exit:
	dsc_exit();

	return errors;
}

#define action_expect(obj, action, status, errors) \
do { \
	void *__obj = obj; \
	QDF_STATUS __expected = status; \
	QDF_STATUS __result; \
\
	__result = dsc_##obj##_##action##_start(__obj); \
	if (__result != __expected) { \
		dsc_err("FAIL: " #obj " " #action \
			"; expected " #status " (%u), found %u", \
			__expected, __result); \
		(errors)++; \
	} \
	if (QDF_IS_STATUS_SUCCESS(__result) && QDF_IS_STATUS_ERROR(__expected))\
		dsc_##obj##_##action##_stop(__obj); \
} while (false)

static uint32_t dsc_test_driver_trans_blocks(void)
{
	uint32_t errors = 0;
	QDF_STATUS status;
	struct dsc_driver *driver;
	struct dsc_psoc *psoc;
	struct dsc_vdev *vdev;

	dsc_enter();

	/* setup */

	status = __dsc_tree_create(&driver, 2, 2);
	if (QDF_IS_STATUS_ERROR(status)) {
		errors++;
		goto exit;
	}

	action_expect(driver, trans, QDF_STATUS_SUCCESS, errors);

	/* test */

	action_expect(driver, trans, QDF_STATUS_E_AGAIN, errors);
	action_expect(driver, op, QDF_STATUS_E_AGAIN, errors);

	dsc_for_each_driver_psoc(driver, psoc) {
		action_expect(psoc, trans, QDF_STATUS_E_AGAIN, errors);
		action_expect(psoc, op, QDF_STATUS_E_AGAIN, errors);

		dsc_for_each_psoc_vdev(psoc, vdev) {
			action_expect(vdev, trans, QDF_STATUS_E_AGAIN, errors);
			action_expect(vdev, op, QDF_STATUS_E_AGAIN, errors);
		}
	}

	/* teardown */

	dsc_driver_trans_stop(driver);

	__dsc_tree_destroy(driver);

exit:
	dsc_exit();

	return errors;
}

static uint32_t dsc_test_psoc_trans_blocks(void)
{
	uint32_t errors = 0;
	QDF_STATUS status;
	struct dsc_driver *driver;
	struct dsc_psoc *psoc;
	struct dsc_vdev *vdev;

	dsc_enter();

	/* setup */

	status = __dsc_tree_create(&driver, 2, 2);
	if (QDF_IS_STATUS_ERROR(status)) {
		errors++;
		goto exit;
	}

	/* test */

	psoc = nth_psoc(driver, 1);
	action_expect(psoc, trans, QDF_STATUS_SUCCESS, errors);

	action_expect(driver, trans, QDF_STATUS_E_AGAIN, errors);
	action_expect(driver, op, QDF_STATUS_SUCCESS, errors);
	dsc_driver_op_stop(driver);

	action_expect(psoc, trans, QDF_STATUS_E_AGAIN, errors);
	action_expect(psoc, op, QDF_STATUS_E_AGAIN, errors);

	dsc_for_each_psoc_vdev(psoc, vdev) {
		action_expect(vdev, trans, QDF_STATUS_E_AGAIN, errors);
		action_expect(vdev, op, QDF_STATUS_E_AGAIN, errors);
	}

	psoc = nth_psoc(driver, 2);
	action_expect(psoc, trans, QDF_STATUS_SUCCESS, errors);

	action_expect(driver, trans, QDF_STATUS_E_AGAIN, errors);
	action_expect(driver, op, QDF_STATUS_SUCCESS, errors);
	dsc_driver_op_stop(driver);

	action_expect(psoc, trans, QDF_STATUS_E_AGAIN, errors);
	action_expect(psoc, op, QDF_STATUS_E_AGAIN, errors);

	dsc_for_each_psoc_vdev(psoc, vdev) {
		action_expect(vdev, trans, QDF_STATUS_E_AGAIN, errors);
		action_expect(vdev, op, QDF_STATUS_E_AGAIN, errors);
	}

	/* teardown */

	dsc_for_each_driver_psoc(driver, psoc)
		dsc_psoc_trans_stop(psoc);

	__dsc_tree_destroy(driver);

exit:
	dsc_exit();

	return errors;
}

static uint32_t dsc_test_vdev_trans_blocks(void)
{
	uint32_t errors = 0;
	QDF_STATUS status;
	struct dsc_driver *driver;
	struct dsc_psoc *psoc;
	struct dsc_vdev *vdev;

	dsc_enter();

	/* setup */

	status = __dsc_tree_create(&driver, 2, 2);
	if (QDF_IS_STATUS_ERROR(status)) {
		errors++;
		goto exit;
	}

	dsc_for_each_driver_psoc(driver, psoc) {
		dsc_for_each_psoc_vdev(psoc, vdev)
			action_expect(vdev, trans, QDF_STATUS_SUCCESS, errors);
	}

	/* test */

	action_expect(driver, trans, QDF_STATUS_E_AGAIN, errors);
	action_expect(driver, op, QDF_STATUS_SUCCESS, errors);
	dsc_driver_op_stop(driver);

	dsc_for_each_driver_psoc(driver, psoc) {
		action_expect(psoc, trans, QDF_STATUS_E_AGAIN, errors);
		action_expect(psoc, op, QDF_STATUS_SUCCESS, errors);
		dsc_psoc_op_stop(psoc);

		dsc_for_each_psoc_vdev(psoc, vdev) {
			action_expect(vdev, trans, QDF_STATUS_E_AGAIN, errors);
			action_expect(vdev, op, QDF_STATUS_E_AGAIN, errors);
		}
	}

	/* teardown */

	dsc_for_each_driver_psoc(driver, psoc) {
		dsc_for_each_psoc_vdev(psoc, vdev)
			dsc_vdev_trans_stop(vdev);
	}

	__dsc_tree_destroy(driver);

exit:
	dsc_exit();

	return errors;
}

#define THREAD_TIMEOUT 1000 /* ms */
#define dsc_event_wait(event) qdf_wait_single_event(event, THREAD_TIMEOUT)

struct thread_ctx {
	struct dsc_driver *driver;
	qdf_event_t start_vdev_trans;
	qdf_event_t start_vdev_wait;
	qdf_event_t start_psoc_wait;
	qdf_event_t start_driver_wait;
	qdf_event_t stop_ops;
	uint32_t step;
};

static QDF_STATUS dsc_thread_ops(void *context)
{
	struct thread_ctx *ctx = context;
	struct dsc_driver *driver = ctx->driver;
	struct dsc_psoc *psoc = nth_psoc(driver, 1);
	struct dsc_vdev *vdev = nth_vdev(psoc, 1);

	dsc_enter();

	dsc_assert_success(dsc_driver_op_start(driver));
	dsc_assert_success(dsc_psoc_op_start(psoc));
	dsc_assert_success(dsc_vdev_op_start(vdev));

	dsc_assert(++ctx->step == 1);
	qdf_event_set(&ctx->start_vdev_trans);

	dsc_assert_success(dsc_event_wait(&ctx->stop_ops));
	dsc_assert(++ctx->step == 7);

	dsc_driver_op_stop(driver);
	schedule();
	dsc_psoc_op_stop(psoc);
	schedule();
	dsc_vdev_op_stop(vdev);

	dsc_exit();

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dsc_thread_vdev_trans(void *context)
{
	struct thread_ctx *ctx = context;
	struct dsc_driver *driver = ctx->driver;
	struct dsc_psoc *psoc = nth_psoc(driver, 1);
	struct dsc_vdev *vdev;

	dsc_enter();

	dsc_assert_success(dsc_event_wait(&ctx->start_vdev_trans));
	dsc_assert(++ctx->step == 2);

	dsc_for_each_psoc_vdev(psoc, vdev)
		dsc_assert_success(dsc_vdev_trans_start(vdev));

	dsc_assert(++ctx->step == 3);
	qdf_event_set(&ctx->start_vdev_wait);

	dsc_vdev_wait_for_ops(nth_vdev(psoc, 1));

	dsc_assert(++ctx->step == 8);
	dsc_vdev_trans_stop(nth_vdev(psoc, 1));
	schedule();
	dsc_assert(++ctx->step == 9);

	dsc_vdev_trans_stop(nth_vdev(psoc, 2));

	dsc_exit();

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dsc_thread_vdev_wait(void *context)
{
	struct thread_ctx *ctx = context;
	struct dsc_vdev *vdev = nth_vdev(nth_psoc(ctx->driver, 1), 1);

	dsc_enter();

	dsc_assert_success(dsc_event_wait(&ctx->start_vdev_wait));
	dsc_assert(++ctx->step == 4);

	qdf_event_set(&ctx->start_psoc_wait);
	dsc_assert_success(dsc_vdev_trans_start_wait(vdev));
	dsc_assert(++ctx->step == 14);

	schedule();

	dsc_assert(++ctx->step == 15);
	dsc_vdev_trans_stop(vdev);

	dsc_exit();

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dsc_thread_psoc_wait(void *context)
{
	struct thread_ctx *ctx = context;
	struct dsc_psoc *psoc = nth_psoc(ctx->driver, 1);

	dsc_enter();

	dsc_assert_success(dsc_event_wait(&ctx->start_psoc_wait));
	dsc_assert(++ctx->step == 5);

	qdf_event_set(&ctx->start_driver_wait);
	dsc_assert_success(dsc_psoc_trans_start_wait(psoc));
	dsc_assert(++ctx->step == 12);

	schedule();

	dsc_assert(++ctx->step == 13);
	dsc_psoc_trans_stop(psoc);

	dsc_exit();

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dsc_thread_driver_wait(void *context)
{
	struct thread_ctx *ctx = context;
	struct dsc_driver *driver = ctx->driver;

	dsc_enter();

	dsc_assert_success(dsc_event_wait(&ctx->start_driver_wait));
	dsc_assert(++ctx->step == 6);

	qdf_event_set(&ctx->stop_ops);
	dsc_assert_success(dsc_driver_trans_start_wait(driver));
	dsc_assert(++ctx->step == 10);

	schedule();

	dsc_assert(++ctx->step == 11);
	dsc_driver_trans_stop(driver);

	dsc_exit();

	return QDF_STATUS_SUCCESS;
}

static uint32_t dsc_test_trans_wait(void)
{
	uint32_t errors = 0;
	QDF_STATUS status;
	qdf_thread_t *ops_thread;
	qdf_thread_t *vdev_trans_thread;
	qdf_thread_t *vdev_wait_thread;
	qdf_thread_t *psoc_wait_thread;
	qdf_thread_t *driver_wait_thread;
	struct thread_ctx ctx = { 0 };

	dsc_enter();

	status = __dsc_tree_create(&ctx.driver, 1, 2);
	if (QDF_IS_STATUS_ERROR(status)) {
		errors++;
		goto exit;
	}

	dsc_assert_success(qdf_event_create(&ctx.start_vdev_trans));
	dsc_assert_success(qdf_event_create(&ctx.start_vdev_wait));
	dsc_assert_success(qdf_event_create(&ctx.start_psoc_wait));
	dsc_assert_success(qdf_event_create(&ctx.start_driver_wait));
	dsc_assert_success(qdf_event_create(&ctx.stop_ops));

	dsc_debug("starting threads");

	ops_thread = qdf_thread_run(dsc_thread_ops, &ctx);
	vdev_trans_thread = qdf_thread_run(dsc_thread_vdev_trans, &ctx);
	vdev_wait_thread = qdf_thread_run(dsc_thread_vdev_wait, &ctx);
	psoc_wait_thread = qdf_thread_run(dsc_thread_psoc_wait, &ctx);
	driver_wait_thread = qdf_thread_run(dsc_thread_driver_wait, &ctx);

	qdf_thread_join(ops_thread);
	qdf_thread_join(vdev_trans_thread);
	qdf_thread_join(vdev_wait_thread);
	qdf_thread_join(psoc_wait_thread);
	qdf_thread_join(driver_wait_thread);

	dsc_debug("threads joined");

	qdf_event_destroy(&ctx.stop_ops);
	qdf_event_destroy(&ctx.start_driver_wait);
	qdf_event_destroy(&ctx.start_psoc_wait);
	qdf_event_destroy(&ctx.start_vdev_wait);
	qdf_event_destroy(&ctx.start_vdev_trans);

	__dsc_tree_destroy(ctx.driver);

exit:
	dsc_exit();

	return errors;
}

uint32_t dsc_unit_test(void)
{
	uint32_t errors = 0;

	errors += dsc_test_create_destroy();
	errors += dsc_test_driver_trans_blocks();
	errors += dsc_test_psoc_trans_blocks();
	errors += dsc_test_vdev_trans_blocks();
	errors += dsc_test_trans_wait();

	return errors;
}

