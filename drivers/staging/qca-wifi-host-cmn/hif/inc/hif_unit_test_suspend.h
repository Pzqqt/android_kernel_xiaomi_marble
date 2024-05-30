/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: Public unit-test related APIs for triggering WoW suspend/resume while
 * the application processor is still up.
 */

#ifndef _HIF_UNIT_TEST_SUSPEND_H_
#define _HIF_UNIT_TEST_SUSPEND_H_

#ifdef WLAN_SUSPEND_RESUME_TEST

#include "qdf_status.h"
#include "hif.h"

typedef void (*hif_ut_resume_callback)(void);

/**
 * hif_ut_apps_suspend() - Setup unit-test related suspend state.
 * @opaque_scn: The HIF context to operate on
 * @callback: The function to call when unit-test resume is triggered
 *
 * Call after a normal WoW suspend has been completed.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hif_ut_apps_suspend(struct hif_opaque_softc *opaque_scn,
			       hif_ut_resume_callback callback);

/**
 * hif_ut_apps_resume() - Cleanup unit-test related suspend state.
 * @opaque_scn: The HIF context to operate on
 *
 * Call before doing a normal WoW resume if suspend was initiated via
 * unit-test suspend.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hif_ut_apps_resume(struct hif_opaque_softc *opaque_scn);

#endif /* WLAN_SUSPEND_RESUME_TEST */

#endif /* _HIF_UNIT_TEST_SUSPEND_H_ */
