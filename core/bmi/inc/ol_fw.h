/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

#ifndef _OL_FW_H_
#define _OL_FW_H_

#ifdef QCA_WIFI_FTM
#include "cdf_types.h"
#endif
#include "hif.h"

#define AR6004_VERSION_REV1_3        0x31c8088a

#define AR9888_REV2_VERSION          0x4100016c
#define AR6320_REV1_VERSION          0x5000000
#define AR6320_REV1_1_VERSION        0x5000001
#define AR6320_REV1_VERSION_1        AR6320_REV1_1_VERSION
#define AR6320_REV1_3_VERSION        0x5000003
#define AR6320_REV2_VERSION          AR6320_REV1_1_VERSION
#define AR6320_REV2_1_VERSION        0x5010000
#define AR6320_REV3_VERSION          0x5020000
#define AR6320_REV3_2_VERSION        0x5030000
#define AR6320_REV4_VERSION          AR6320_REV2_1_VERSION
#define AR6320_DEV_VERSION           0x1000000

#ifdef HIF_PCI
void ol_target_failure(void *instance, QDF_STATUS status);
#else
static inline void ol_target_failure(void *instance, QDF_STATUS status)
{
	return;
}
#endif
#endif /* _OL_FW_H_ */
