/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
  * DOC: Define the debug utils for MLME
  */
#ifndef _WLAN_MLME_DBG_H_
#define _WLAN_MLME_DBG_H_

#include <qdf_types.h>
#include <qdf_trace.h>

#define mlme_alert(format, args...) \
		QDF_TRACE_FATAL(QDF_MODULE_ID_CMN_MLME, format, ## args)

#define mlme_err(format, args...) \
		QDF_TRACE_ERROR(QDF_MODULE_ID_CMN_MLME, format, ## args)

#define mlme_warn(format, args...) \
		QDF_TRACE_WARN(QDF_MODULE_ID_CMN_MLME, format, ## args)

#define mlme_info(format, args...) \
		QDF_TRACE_INFO(QDF_MODULE_ID_CMN_MLME, format, ## args)

#define mlme_debug(format, args...) \
		QDF_TRACE_DEBUG(QDF_MODULE_ID_CMN_MLME, format, ## args)

#define mlme_nofl_alert(format, args...) \
		QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_CMN_MLME, format, ## args)

#define mlme_nofl_err(format, args...) \
		QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_CMN_MLME, format, ## args)

#define mlme_nofl_warn(format, args...) \
		QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_CMN_MLME, format, ## args)

#define mlme_nofl_info(format, args...) \
		QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_CMN_MLME, format, ## args)

#define mlme_nofl_debug(format, args...) \
		QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_CMN_MLME, format, ## args)

#define mlme_rl_debug(format, args...) \
		QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_CMN_MLME, format, ## args)

#endif    /* _WLAN_MLME_DBG_H_ */

