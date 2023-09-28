/*
 * Copyright (c) 2014, 2016, 2018-2020 The Linux Foundation. All rights reserved.
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

#define hif_alert(args ...) QDF_TRACE_FATAL(QDF_MODULE_ID_HIF, args)
#define hif_err(args ...) QDF_TRACE_ERROR(QDF_MODULE_ID_HIF, args)
#define hif_warn(args ...) QDF_TRACE_WARN(QDF_MODULE_ID_HIF, args)
#define hif_info(args ...) QDF_TRACE_INFO(QDF_MODULE_ID_HIF, args)
#define hif_debug(args ...) QDF_TRACE_DEBUG(QDF_MODULE_ID_HIF, args)
#define hif_info_high(args ...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_HIF, ## args)

#define hif_nofl_alert(args ...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_HIF, args)
#define hif_nofl_err(args ...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_HIF, args)
#define hif_nofl_warn(args ...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_HIF, args)
#define hif_nofl_info(args ...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_HIF, args)
#define hif_nofl_debug(args ...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_HIF, args)

#define HIF_ENTER() QDF_TRACE_ENTER(QDF_MODULE_ID_HIF, "enter")

#define HIF_EXIT() QDF_TRACE_EXIT(QDF_MODULE_ID_HIF, "exit")

#endif /* __HIF_DEBUG_H__ */
