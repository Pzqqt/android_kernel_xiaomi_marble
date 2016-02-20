/*
 * Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
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

#if !defined(__I_CDF_TRACE_H)
#define __I_CDF_TRACE_H

#if !defined(__printf)
#define __printf(a, b)
#endif

/**
 * DOC: i_cdf_trace.h
 *
 * Linux-specific definitions for CDF trace
 *
 */

/* Include Files */

/**
 * cdf_trace_msg()- logging API
 * @module: Module identifier. A member of the QDF_MODULE_ID enumeration that
 *	    identifies the module issuing the trace message.
 * @level: Trace level. A member of the CDF_TRACE_LEVEL enumeration indicating
 *	   the severity of the condition causing the trace message to be issued.
 *	   More severe conditions are more likely to be logged.
 * @strFormat: Format string. The message to be logged. This format string
 *	       contains printf-like replacement parameters, which follow this
 *	       parameter in the variable argument list.
 *
 * Users wishing to add tracing information to their code should use
 * CDF_TRACE.  CDF_TRACE() will compile into a call to cdf_trace_msg() when
 * tracing is enabled.
 *
 * Return: nothing
 *
 */
void __printf(3, 4) cdf_trace_msg(QDF_MODULE_ID module, CDF_TRACE_LEVEL level,
				  char *strFormat, ...);

void cdf_trace_hex_dump(QDF_MODULE_ID module, CDF_TRACE_LEVEL level,
			void *data, int buf_len);

void cdf_trace_display(void);

void cdf_trace_set_value(QDF_MODULE_ID module, CDF_TRACE_LEVEL level,
			 uint8_t on);

void cdf_trace_set_module_trace_level(QDF_MODULE_ID module, uint32_t level);

/* CDF_TRACE is the macro invoked to add trace messages to code.  See the
 * documenation for cdf_trace_msg() for the parameters etc. for this function.
 *
 * NOTE:  Code CDF_TRACE() macros into the source code.  Do not code directly
 * to the cdf_trace_msg() function.
 *
 * NOTE 2:  cdf tracing is totally turned off if WLAN_DEBUG is *not* defined.
 * This allows us to build 'performance' builds where we can measure performance
 * without being bogged down by all the tracing in the code
 */

#if defined(WLAN_DEBUG)
#define CDF_TRACE cdf_trace_msg
#define CDF_TRACE_HEX_DUMP cdf_trace_hex_dump
#else
#define CDF_TRACE(arg ...)
#define CDF_TRACE_HEX_DUMP(arg ...)
#endif

void __printf(3, 4) cdf_snprintf(char *strBuffer, unsigned int size,
				 char *strFormat, ...);
#define CDF_SNPRINTF cdf_snprintf

#ifdef CDF_ENABLE_TRACING

#define CDF_ASSERT(_condition) \
	do { \
		if (!(_condition)) { \
			pr_err("CDF ASSERT in %s Line %d\n", \
			       __func__, __LINE__); \
			WARN_ON(1); \
		} \
	} while (0)

#else

/* This code will be used for compilation if tracing is to be compiled out */
/* of the code so these functions/macros are 'do nothing' */
static inline void cdf_trace_msg(QDF_MODULE_ID module, ...)
{
}

#define CDF_ASSERT(_condition)

#endif

#ifdef PANIC_ON_BUG

#define CDF_BUG(_condition) \
	do { \
		if (!(_condition)) { \
			pr_err("CDF BUG in %s Line %d\n", \
			       __func__, __LINE__); \
			BUG_ON(1); \
		} \
	} while (0)

#else

#define CDF_BUG(_condition) \
	do { \
		if (!(_condition)) { \
			pr_err("CDF BUG in %s Line %d\n", \
			       __func__, __LINE__); \
			WARN_ON(1); \
		} \
	} while (0)

#endif

#endif
