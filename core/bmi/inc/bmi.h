/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

/* ================================================================ */
/* BMI declarations and prototypes */
/* */
/* ================================================================= */

#ifndef _BMI_H_
#define _BMI_H_
#include "bmi_msg.h"
#include "cdf_trace.h"
#include "ol_if_athvar.h"
#include "hif.h"

struct ol_context;
CDF_STATUS ol_cds_init(cdf_device_t cdf_dev, void *hif_ctx);
void ol_cds_free(void);

#ifdef HIF_PCI
void bmi_cleanup(struct ol_softc *scn);
CDF_STATUS bmi_done(struct ol_context *ol_ctx);
CDF_STATUS bmi_download_firmware(struct ol_context *ol_ctx);
#else
static inline void bmi_cleanup(struct ol_softc *scn)
{
	return;
}

static inline CDF_STATUS bmi_done(struct ol_context *ol_ctx)
{
	return CDF_STATUS_SUCCESS;
}

static inline CDF_STATUS bmi_download_firmware(struct ol_context *ol_ctx)
{
	return CDF_STATUS_SUCCESS;
}
#endif
#endif /* _BMI_H_ */
