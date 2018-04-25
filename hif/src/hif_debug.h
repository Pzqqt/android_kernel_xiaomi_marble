/*
 * Copyright (c) 2014, 2016, 2018 The Linux Foundation. All rights reserved.
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
#include "qdf_trace.h"

#define hif_alert_rl(params...) QDF_TRACE_FATAL_RL(QDF_MODULE_ID_HIF, params)
#define hif_err_rl(params...) QDF_TRACE_ERROR_RL(QDF_MODULE_ID_HIF, params)
#define hif_warn_rl(params...) QDF_TRACE_WARN_RL(QDF_MODULE_ID_HIF, params)
#define hif_info_rl(params...) QDF_TRACE_INFO_RL(QDF_MODULE_ID_HIF, params)
#define hif_debug_rl(params...) QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_HIF, params)

#define HIF_ERROR(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_ERROR, ## args)
#define HIF_WARN(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_WARN, ## args)
#define HIF_INFO(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_INFO, ## args)
#define HIF_INFO_HI(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_INFO_HIGH, ## args)
#define HIF_INFO_MED(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_INFO_MED, ## args)
#define HIF_INFO_LO(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_INFO_LOW, ## args)
#define HIF_TRACE(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_INFO, ## args)
#define HIF_DBG(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_DEBUG, ## args)

#define HIF_ENTER(fmt, ...) QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_INFO, \
		"Enter: %s "fmt, __func__, ## __VA_ARGS__)

#define HIF_EXIT(fmt, ...) QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_INFO, \
		"Exit: %s "fmt, __func__, ## __VA_ARGS__)

#endif /* __HIF_DEBUG_H__ */
