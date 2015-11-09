/*
 * Copyright (c) 2014 The Linux Foundation. All rights reserved.
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

#ifndef __HIF_DEBUG_H__
#define __HIF_DEBUG_H__
#include "cdf_trace.h"

#define HIF_ERROR(args ...) \
	CDF_TRACE(CDF_MODULE_ID_HIF, CDF_TRACE_LEVEL_ERROR, ## args)
#define HIF_WARN(args ...) \
	CDF_TRACE(CDF_MODULE_ID_HIF, CDF_TRACE_LEVEL_WARN, ## args)
#define HIF_INFO(args ...) \
	CDF_TRACE(CDF_MODULE_ID_HIF, CDF_TRACE_LEVEL_INFO, ## args)
#define HIF_INFO_HI(args ...) \
	CDF_TRACE(CDF_MODULE_ID_HIF, CDF_TRACE_LEVEL_INFO_HIGH, ## args)
#define HIF_INFO_MED(args ...) \
	CDF_TRACE(CDF_MODULE_ID_HIF, CDF_TRACE_LEVEL_INFO_MED, ## args)
#define HIF_INFO_LO(args ...) \
	CDF_TRACE(CDF_MODULE_ID_HIF, CDF_TRACE_LEVEL_INFO_LOW, ## args)
#define HIF_TRACE(args ...) \
	CDF_TRACE(CDF_MODULE_ID_HIF, CDF_TRACE_LEVEL_ERROR, ## args)
#define HIF_DBG(args ...) \
	CDF_TRACE(CDF_MODULE_ID_HIF, CDF_TRACE_LEVEL_DEBUG, ## args)
#endif /* __HIF_DEBUG_H__ */
