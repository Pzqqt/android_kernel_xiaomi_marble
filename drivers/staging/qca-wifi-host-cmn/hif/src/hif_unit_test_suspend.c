/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

#include "qdf_status.h"
#include "hif_main.h"
#include "hif_unit_test_suspend.h"
#include "hif_unit_test_suspend_i.h"

enum hif_ut_suspend_state_bits {
	UT_SUSPENDED_BIT = 0
};

/**
 * hif_ut_fw_resume_work() - Work handler for firmware-triggered resume
 * @work: The work struct being passed from the linux kernel
 *
 * Return: None
 */
static void hif_ut_fw_resume_work(struct work_struct *work)
{
	struct hif_ut_suspend_context *ctx =
		container_of(work, struct hif_ut_suspend_context, resume_work);

	QDF_BUG(ctx);
	if (!ctx)
		return;

	QDF_BUG(ctx->resume_callback);
	if (!ctx->resume_callback)
		return;

	ctx->resume_callback();
	ctx->resume_callback = NULL;
}

void hif_ut_suspend_init(struct hif_softc *scn)
{
	INIT_WORK(&scn->ut_suspend_ctx.resume_work, hif_ut_fw_resume_work);
}

bool hif_is_ut_suspended(struct hif_softc *scn)
{
	QDF_BUG(scn);
	if (!scn)
		return false;

	return test_bit(UT_SUSPENDED_BIT, &scn->ut_suspend_ctx.state);
}

QDF_STATUS hif_ut_apps_suspend(struct hif_opaque_softc *opaque_scn,
			       hif_ut_resume_callback callback)
{
	struct hif_softc *scn = HIF_GET_SOFTC(opaque_scn);

	QDF_BUG(scn);
	if (!scn)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(callback);
	if (!callback)
		return QDF_STATUS_E_INVAL;

	if (test_and_set_bit(UT_SUSPENDED_BIT, &scn->ut_suspend_ctx.state))
		return QDF_STATUS_E_INVAL;

	scn->ut_suspend_ctx.resume_callback = callback;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hif_ut_apps_resume(struct hif_opaque_softc *opaque_scn)
{
	struct hif_softc *scn = HIF_GET_SOFTC(opaque_scn);

	QDF_BUG(scn);
	if (!scn)
		return QDF_STATUS_E_INVAL;

	if (!test_and_clear_bit(UT_SUSPENDED_BIT, &scn->ut_suspend_ctx.state))
		return QDF_STATUS_E_INVAL;

	scn->ut_suspend_ctx.resume_callback = NULL;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hif_ut_fw_resume(struct hif_softc *scn)
{
	QDF_BUG(scn);
	if (!scn)
		return QDF_STATUS_E_INVAL;

	if (!test_and_clear_bit(UT_SUSPENDED_BIT, &scn->ut_suspend_ctx.state))
		return QDF_STATUS_E_INVAL;

	schedule_work(&scn->ut_suspend_ctx.resume_work);

	return QDF_STATUS_SUCCESS;
}

bool hif_irq_trigger_ut_resume(struct hif_softc *scn)
{
	if (!hif_is_ut_suspended(scn))
		return false;

	return true;
}
