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

/**
 * DOC: HIF internal unit-test related APIs for triggering WoW suspend/resume
 * while the application processor is still up.
 */

#ifndef _HIF_UNIT_TEST_SUSPEND_I_H_
#define _HIF_UNIT_TEST_SUSPEND_I_H_

#include "qdf_status.h"
#include "hif_main.h"
#include "hif_unit_test_suspend.h"

#ifdef WLAN_SUSPEND_RESUME_TEST

struct hif_ut_suspend_context {
	unsigned long state;
	hif_ut_resume_callback resume_callback;
	struct work_struct resume_work;
};

/**
 * hif_ut_suspend_init() - Initialize the unit-test suspend context
 * @scn: the hif context to initialize
 *
 * Return: None
 */
void hif_ut_suspend_init(struct hif_softc *scn);

/**
 * hif_is_ut_suspended() - Tests if the given hif context is unit-test suspended
 * @scn: The HIF context to check
 *
 * Return: true, if unit-test suspended, otherwise false
 */
bool hif_is_ut_suspended(struct hif_softc *scn);

/**
 * hif_ut_fw_resume() - Initiate a firmware triggered unit-test resume
 * @scn: The HIF context to operate on
 *
 * This schedules the callback previously registered via a call to
 * hif_ut_apps_suspend for execution.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hif_ut_fw_resume(struct hif_softc *scn);

/**
 * hif_irq_trigger_ut_resume() - Test for given hif ctx unit-test resume needed
 * @scn: The HIF context to check
 *
 * Return: true, if unit-test resume procedure is needed, otherwise false
 */
bool hif_irq_trigger_ut_resume(struct hif_softc *scn);

#else /* WLAN_SUSPEND_RESUME_TEST */

struct hif_ut_suspend_context {};

static inline void hif_ut_suspend_init(struct hif_softc *scn) {}

static inline bool hif_is_ut_suspended(struct hif_softc *scn)
{
	return false;
}

static inline QDF_STATUS hif_ut_fw_resume(struct hif_softc *scn)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUSPEND_RESUME_TEST */

#endif /* _HIF_UNIT_TEST_SUSPEND_I_H_ */
