/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: Internal APIs for the configuration component.
 */

#ifndef __I_CFG_H
#define __I_CFG_H

#include "cfg_define.h"
#include "qdf_trace.h"
#include "qdf_types.h"
#include "wlan_objmgr_psoc_obj.h"

#define cfg_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_CONFIG, params)
#define cfg_info(params...) QDF_TRACE_INFO(QDF_MODULE_ID_CONFIG, params)
#define cfg_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_CONFIG, params)
#define cfg_enter() QDF_TRACE_ENTER(QDF_MODULE_ID_CONFIG, "enter")
#define cfg_exit() QDF_TRACE_EXIT(QDF_MODULE_ID_CONFIG, "exit")

#define cfg_err_rl(params...) QDF_TRACE_ERROR_RL(QDF_MODULE_ID_CONFIG, params)
#define cfg_warn_rl(params...) QDF_TRACE_WARN_RL(QDF_MODULE_ID_CONFIG, params)
#define cfg_info_rl(params...) QDF_TRACE_INFO_RL(QDF_MODULE_ID_CONFIG, params)
#define cfg_debug_rl(params...) QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_CONFIG, params)

#define cfg_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_CONFIG, params)
#define cfg_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_CONFIG, params)
#define cfg_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_CONFIG, params)
#define cfg_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_CONFIG, params)

/* define global config values structure */

#undef __CFG_INI_STRING
#define __CFG_INI_STRING(id, mtype, ctype, name, min, max, fallback, desc, \
			 def...) \
	const char id##_internal[(max) + 1];
#undef __CFG_INI
#define __CFG_INI(id, mtype, ctype, name, min, max, fallback, desc, def...) \
	const ctype id##_internal;

struct cfg_values {
	/* e.g. const int32_t __CFG_SCAN_DWELL_TIME_internal; */
	CFG_ALL
};

#undef __CFG_INI_STRING
#define __CFG_INI_STRING(args...) __CFG_INI(args)
#undef __CFG_INI
#define __CFG_INI(args...) (args)

struct cfg_values *cfg_psoc_get_values(struct wlan_objmgr_psoc *psoc);

#define __cfg_get(psoc, id) (cfg_psoc_get_values( \
			(struct wlan_objmgr_psoc *)psoc)->id##_internal)

#endif /* __I_CFG_H */

