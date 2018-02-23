/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

#define __cfg_log(level, fmt, args...) \
	QDF_TRACE(QDF_MODULE_ID_CONFIG, level, FL(fmt), ##args)
#define cfg_err(fmt, args...) __cfg_log(QDF_TRACE_LEVEL_ERROR, fmt, ##args)
#define cfg_info(fmt, args...) __cfg_log(QDF_TRACE_LEVEL_INFO, fmt, ##args)
#define cfg_debug(fmt, args...) __cfg_log(QDF_TRACE_LEVEL_DEBUG, fmt, ##args)
#define cfg_enter() cfg_debug("enter")
#define cfg_exit() cfg_debug("exit")

/* define global config values structure */

#undef __CFG_STRING
#define __CFG_STRING(id, mtype, ctype, name, min, max, fallback, desc, def...) \
	const char id##_internal[max + 1];
#undef __CFG_ANY
#define __CFG_ANY(id, mtype, ctype, name, min, max, fallback, desc, def...) \
	const ctype id##_internal;

struct cfg_values {
	/* e.g. const int32_t __CFG_SCAN_DWELL_TIME_internal; */
	CFG_ALL
};

#undef __CFG_STRING
#define __CFG_STRING(args...) __CFG_ANY(args)
#undef __CFG_ANY
#define __CFG_ANY(args...) (args)

struct cfg_values *cfg_psoc_get_values(struct wlan_objmgr_psoc *psoc);

#define __cfg_get(psoc, id) (cfg_psoc_get_values(psoc)->id##_internal)

#endif /* __I_CFG_H */

