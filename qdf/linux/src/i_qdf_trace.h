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

/**
 * DOC: i_qdf_trace.h
 *
 * Linux-specific definitions for QDF trace
 *
 */

#if !defined(__I_QDF_TRACE_H)
#define __I_QDF_TRACE_H

#if !defined(__printf)
#define __printf(a, b)
#endif

/* Include Files */
#include <cds_packet.h>

#define QDF_ENABLE_TRACING

#ifdef QDF_ENABLE_TRACING

#define QDF_ASSERT(_condition) \
	do { \
		if (!(_condition)) { \
			pr_err("QDF ASSERT in %s Line %d\n", \
			       __func__, __LINE__); \
			WARN_ON(1); \
		} \
	} while (0)

#else

/* This code will be used for compilation if tracing is to be compiled out */
/* of the code so these functions/macros are 'do nothing' */
static inline void qdf_trace_msg(QDF_MODULE_ID module, ...)
{
}

#define QDF_ASSERT(_condition)

#endif

#ifdef PANIC_ON_BUG

#define QDF_BUG(_condition) \
	do { \
		if (!(_condition)) { \
			pr_err("QDF BUG in %s Line %d\n", \
			       __func__, __LINE__); \
			BUG_ON(1); \
		} \
	} while (0)

#else

#define QDF_BUG(_condition) \
	do { \
		if (!(_condition)) { \
			pr_err("QDF BUG in %s Line %d\n", \
			       __func__, __LINE__); \
			WARN_ON(1); \
		} \
	} while (0)

#endif

#endif /* __I_QDF_TRACE_H */
