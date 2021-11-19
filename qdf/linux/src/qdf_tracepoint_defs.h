/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: qdf_tracepoint_defs.h
 * This file provides OS abstraction for function tracing.
 */

#if  !defined(_QDF_TRACEPOINT_DEFS_H) || defined(TRACE_HEADER_MULTI_READ)
#define _QDF_TRACEPOINT_DEFS_H

#include <linux/tracepoint.h>

#undef TRACE_SYSTEM
#define TRACE_SYSTEM wlan

#ifndef WLAN_TRACEPOINTS
#undef TRACE_EVENT
#define TRACE_EVENT(name, proto, args, tstruct, assign, print) \
	static inline void trace_##name(proto) {}              \
	static inline bool trace_##name##_enabled(void)        \
	{                                                      \
		return false;                                  \
	}

#undef DECLARE_EVENT_CLASS
#define DECLARE_EVENT_CLASS(name, proto, args, tstruct, assign, print)

#undef DEFINE_EVENT
#define DEFINE_EVENT(evt_class, name, proto, args)      \
	static inline void trace_##name(proto) {}       \
	static inline bool trace_##name##_enabled(void) \
	{                                               \
		return false;                           \
	}
#endif /* WLAN_TRACEPOINTS */
#endif /* _QDF_TRACEPOINT_DEFS_H */

/* Below should be outside the protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE qdf_tracepoint_defs
#include <trace/define_trace.h>
