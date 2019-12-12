/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_trace.h
 *
 * Linux-specific definitions for QDF trace
 *
 */

#if !defined(__I_QDF_TRACE_H)
#define __I_QDF_TRACE_H

/* older kernels have a bug in kallsyms, so ensure module.h is included */
#include <linux/module.h>
#include <linux/kallsyms.h>
#ifdef CONFIG_QCA_MINIDUMP
#include <linux/minidump_tlv.h>
#endif

#if !defined(__printf)
#define __printf(a, b)
#endif

/* QDF_TRACE is the macro invoked to add trace messages to code.  See the
 * documenation for qdf_trace_msg() for the parameters etc. for this function.
 *
 * NOTE:  Code QDF_TRACE() macros into the source code.  Do not code directly
 * to the qdf_trace_msg() function.
 *
 * NOTE 2:  qdf tracing is totally turned off if WLAN_DEBUG is *not* defined.
 * This allows us to build 'performance' builds where we can measure performance
 * without being bogged down by all the tracing in the code
 */
#if defined(QDF_TRACE_PRINT_ENABLE)
#define qdf_trace(log_level, args...) \
		do {	\
			extern int qdf_dbg_mask; \
			if (qdf_dbg_mask >= log_level) { \
				printk(args); \
				printk("\n"); \
			} \
		} while (0)
#endif

#if defined(WLAN_DEBUG) || defined(DEBUG) || defined(QDF_TRACE_PRINT_ENABLE)
#define QDF_TRACE qdf_trace_msg
#define QDF_VTRACE qdf_vtrace_msg
#define QDF_TRACE_HEX_DUMP qdf_trace_hex_dump
#else
#define QDF_TRACE(arg ...) __qdf_trace_dummy(arg)
#define QDF_VTRACE(arg ...) __qdf_vtrace_dummy(arg)
#define QDF_TRACE_HEX_DUMP(arg ...) __qdf_trace_hexdump_dummy(arg)
#endif

#if defined(WLAN_DEBUG) || defined(DEBUG) || defined(QDF_TRACE_PRINT_ENABLE)
#define QDF_MAX_LOGS_PER_SEC 2
/**
 * __QDF_TRACE_RATE_LIMITED() - rate limited version of QDF_TRACE
 * @params: parameters to pass through to QDF_TRACE
 *
 * This API prevents logging a message more than QDF_MAX_LOGS_PER_SEC times per
 * second. This means any subsequent calls to this API from the same location
 * within 1/QDF_MAX_LOGS_PER_SEC seconds will be dropped.
 *
 * Return: None
 */
#define __QDF_TRACE_RATE_LIMITED(params...)\
	do {\
		static ulong __last_ticks;\
		ulong __ticks = jiffies;\
		if (time_after(__ticks,\
			       __last_ticks + HZ / QDF_MAX_LOGS_PER_SEC)) {\
			QDF_TRACE(params);\
			__last_ticks = __ticks;\
		} \
	} while (0)
#else
#define __QDF_TRACE_RATE_LIMITED(arg ...)
#endif

#define __QDF_TRACE_NO_FL(log_level, module_id, format, args...) \
	QDF_TRACE(module_id, log_level, format, ## args)

#define __QDF_TRACE_FL(log_level, module_id, format, args...) \
	QDF_TRACE(module_id, log_level, FL(format), ## args)

#define __QDF_TRACE_RL(log_level, module_id, format, args...) \
	__QDF_TRACE_RATE_LIMITED(module_id, log_level, FL(format), ## args)

#define __QDF_TRACE_RL_NO_FL(log_level, module_id, format, args...) \
	__QDF_TRACE_RATE_LIMITED(module_id, log_level, format, ## args)

static inline void __qdf_trace_noop(QDF_MODULE_ID module,
				    const char *format, ...) { }
static inline void __qdf_trace_dummy(QDF_MODULE_ID module,
				     QDF_TRACE_LEVEL level,
				     const char *format, ...) { }
static inline void __qdf_vtrace_dummy(QDF_MODULE_ID module,
				      QDF_TRACE_LEVEL level,
				      const char *str_format, va_list val) { }
static inline void __qdf_trace_hexdump_dummy(QDF_MODULE_ID module,
					     QDF_TRACE_LEVEL level,
					     void *data, int buf_len) { }


#ifdef WLAN_LOG_FATAL
#define QDF_TRACE_FATAL(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_FATAL, ## params)
#define QDF_TRACE_FATAL_NO_FL(params...) \
	__QDF_TRACE_NO_FL(QDF_TRACE_LEVEL_FATAL, ## params)
#define QDF_TRACE_FATAL_RL(params...) \
	__QDF_TRACE_RL(QDF_TRACE_LEVEL_FATAL, ## params)
#define QDF_TRACE_FATAL_RL_NO_FL(params...) \
	__QDF_TRACE_RL_NO_FL(QDF_TRACE_LEVEL_FATAL, ## params)
#define QDF_VTRACE_FATAL(module_id, fmt, args) \
	QDF_VTRACE(module_id, QDF_TRACE_LEVEL_FATAL, fmt, args)
#else
#define QDF_TRACE_FATAL(params...) __qdf_trace_noop(params)
#define QDF_TRACE_FATAL_NO_FL(params...) __qdf_trace_noop(params)
#define QDF_TRACE_FATAL_RL(params...) __qdf_trace_noop(params)
#define QDF_TRACE_FATAL_RL_NO_FL(params...) __qdf_trace_noop(params)
#define QDF_VTRACE_FATAL(params...) __qdf_trace_noop(params)
#endif

#ifdef WLAN_LOG_ERROR
#define QDF_TRACE_ERROR(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_ERROR, ## params)
#define QDF_TRACE_ERROR_NO_FL(params...) \
	__QDF_TRACE_NO_FL(QDF_TRACE_LEVEL_ERROR, ## params)
#define QDF_TRACE_ERROR_RL(params...) \
	__QDF_TRACE_RL(QDF_TRACE_LEVEL_ERROR, ## params)
#define QDF_TRACE_ERROR_RL_NO_FL(params...) \
	__QDF_TRACE_RL_NO_FL(QDF_TRACE_LEVEL_ERROR, ## params)
#define QDF_VTRACE_ERROR(module_id, fmt, args) \
	QDF_VTRACE(module_id, QDF_TRACE_LEVEL_ERROR, fmt, args)
#else
#define QDF_TRACE_ERROR(params...) __qdf_trace_noop(params)
#define QDF_TRACE_ERROR_NO_FL(params...) __qdf_trace_noop(params)
#define QDF_TRACE_ERROR_RL(params...) __qdf_trace_noop(params)
#define QDF_TRACE_ERROR_RL_NO_FL(params...) __qdf_trace_noop(params)
#define QDF_VTRACE_ERROR(params...) __qdf_trace_noop(params)
#endif

#ifdef WLAN_LOG_WARN
#define QDF_TRACE_WARN(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_WARN, ## params)
#define QDF_TRACE_WARN_NO_FL(params...) \
	__QDF_TRACE_NO_FL(QDF_TRACE_LEVEL_WARN, ## params)
#define QDF_TRACE_WARN_RL(params...) \
	__QDF_TRACE_RL(QDF_TRACE_LEVEL_WARN, ## params)
#define QDF_TRACE_WARN_RL_NO_FL(params...) \
	__QDF_TRACE_RL_NO_FL(QDF_TRACE_LEVEL_WARN, ## params)
#define QDF_VTRACE_WARN(module_id, fmt, args) \
	QDF_VTRACE(module_id, QDF_TRACE_LEVEL_WARN, fmt, args)
#else
#define QDF_TRACE_WARN(params...) __qdf_trace_noop(params)
#define QDF_TRACE_WARN_NO_FL(params...) __qdf_trace_noop(params)
#define QDF_TRACE_WARN_RL(params...) __qdf_trace_noop(params)
#define QDF_TRACE_WARN_RL_NO_FL(params...) __qdf_trace_noop(params)
#define QDF_VTRACE_WARN(params...) __qdf_trace_noop(params)
#endif

#ifdef WLAN_LOG_INFO
#define QDF_TRACE_INFO(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO, ## params)
#define QDF_TRACE_INFO_NO_FL(params...) \
	__QDF_TRACE_NO_FL(QDF_TRACE_LEVEL_INFO, ## params)
#define QDF_TRACE_INFO_RL(params...) \
	__QDF_TRACE_RL(QDF_TRACE_LEVEL_INFO, ## params)
#define QDF_TRACE_INFO_RL_NO_FL(params...) \
	__QDF_TRACE_RL_NO_FL(QDF_TRACE_LEVEL_INFO, ## params)
#define QDF_VTRACE_INFO(module_id, fmt, args) \
	QDF_VTRACE(module_id, QDF_TRACE_LEVEL_INFO, fmt, args)
#else
#define QDF_TRACE_INFO(params...) __qdf_trace_noop(params)
#define QDF_TRACE_INFO_NO_FL(params...) __qdf_trace_noop(params)
#define QDF_TRACE_INFO_RL(params...) __qdf_trace_noop(params)
#define QDF_TRACE_INFO_RL_NO_FL(params...) __qdf_trace_noop(params)
#define QDF_VTRACE_INFO(params...) __qdf_trace_noop(params)
#endif

#ifdef WLAN_LOG_DEBUG
#define QDF_TRACE_DEBUG(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_DEBUG, ## params)
#define QDF_TRACE_DEBUG_NO_FL(params...) \
	__QDF_TRACE_NO_FL(QDF_TRACE_LEVEL_DEBUG, ## params)
#define QDF_TRACE_DEBUG_RL(params...) \
	__QDF_TRACE_RL(QDF_TRACE_LEVEL_DEBUG, ## params)
#define QDF_TRACE_DEBUG_RL_NO_FL(params...) \
	__QDF_TRACE_RL_NO_FL(QDF_TRACE_LEVEL_DEBUG, ## params)
#define QDF_VTRACE_DEBUG(module_id, fmt, args) \
	QDF_VTRACE(module_id, QDF_TRACE_LEVEL_DEBUG, fmt, args)
#else
#define QDF_TRACE_DEBUG(params...) __qdf_trace_noop(params)
#define QDF_TRACE_DEBUG_NO_FL(params...) __qdf_trace_noop(params)
#define QDF_TRACE_DEBUG_RL(params...) __qdf_trace_noop(params)
#define QDF_TRACE_DEBUG_RL_NO_FL(params...) __qdf_trace_noop(params)
#define QDF_VTRACE_DEBUG(params...) __qdf_trace_noop(params)
#endif

#ifdef WLAN_LOG_ENTER
#define QDF_TRACE_ENTER(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_DEBUG, ## params)
#else
#define QDF_TRACE_ENTER(params...) __qdf_trace_noop(params)
#endif

#ifdef WLAN_LOG_EXIT
#define QDF_TRACE_EXIT(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_DEBUG, ## params)
#else
#define QDF_TRACE_EXIT(params...) __qdf_trace_noop(params)
#endif

#define QDF_ENABLE_TRACING
#define qdf_scnprintf scnprintf

#ifdef QDF_ENABLE_TRACING

#ifdef WLAN_WARN_ON_ASSERT
#define QDF_ASSERT(_condition) \
	do { \
		if (!(_condition)) { \
			pr_err("QDF ASSERT in %s Line %d\n", \
			       __func__, __LINE__); \
			WARN_ON(1); \
		} \
	} while (0)
#else
#define QDF_ASSERT(_condition) \
	do { \
		if (!(_condition)) { \
			/* no-op */ \
		} \
	} while (0)
#endif /* WLAN_WARN_ON_ASSERT */
/**
 * qdf_trace_msg()- logging API
 * @module: Module identifier. A member of the QDF_MODULE_ID enumeration that
 *	    identifies the module issuing the trace message.
 * @level: Trace level. A member of the QDF_TRACE_LEVEL enumeration indicating
 *	   the severity of the condition causing the trace message to be issued.
 *	   More severe conditions are more likely to be logged.
 * @str_format: Format string. The message to be logged. This format string
 *	       contains printf-like replacement parameters, which follow this
 *	       parameter in the variable argument list.
 *
 * Users wishing to add tracing information to their code should use
 * QDF_TRACE.  QDF_TRACE() will compile into a call to qdf_trace_msg() when
 * tracing is enabled.
 *
 * Return: nothing
 *
 * implemented in qdf_trace.c
 */
void __printf(3, 4) qdf_trace_msg(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
				  const char *str_format, ...);

/**
 * qdf_vtrace_msg() - the va_list version of qdf_trace_msg
 * @module: the calling module's Id
 * @level: the logging level to log using
 * @str_format: the log format string
 * @val: the va_list containing the values to format according to str_format
 *
 * Return: None
 */
void qdf_vtrace_msg(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
		    const char *str_format, va_list val);

#else

/* This code will be used for compilation if tracing is to be compiled out */
/* of the code so these functions/macros are 'do nothing' */
static inline void qdf_trace_msg(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
				 const char *str_format, ...)
{
}

#define QDF_ASSERT(_condition)

#endif

#ifdef QDF_TRACE_PRINT_ENABLE
static inline void qdf_vprint(const char *fmt, va_list args)
{
	QDF_VTRACE_INFO(QDF_MODULE_ID_ANY, fmt, args);
}
#else /* QDF_TRACE_PRINT_ENABLE */
static inline void qdf_vprint(const char *fmt, va_list args)
{
	QDF_VTRACE_ERROR(QDF_MODULE_ID_QDF, fmt, args);
}
#endif

#ifdef PANIC_ON_BUG
#ifdef CONFIG_SLUB_DEBUG
/**
 * __qdf_bug() - Calls BUG() when the PANIC_ON_BUG compilation option is enabled
 *
 * Note: Calling BUG() can cause a compiler to assume any following code is
 * unreachable. Because these BUG's may or may not be enabled by the build
 * configuration, this can cause developers some pain. Consider:
 *
 *	bool bit;
 *
 *	if (ptr)
 *		bit = ptr->returns_bool();
 *	else
 *		__qdf_bug();
 *
 *	// do stuff with @bit
 *
 *	return bit;
 *
 * In this case, @bit is potentially uninitialized when we return! However, the
 * compiler can correctly assume this case is impossible when PANIC_ON_BUG is
 * enabled. Because developers typically enable this feature, the "maybe
 * uninitialized" warning will not be emitted, and the bug remains uncaught
 * until someone tries to make a build without PANIC_ON_BUG.
 *
 * A simple workaround for this, is to put the definition of __qdf_bug in
 * another compilation unit, which prevents the compiler from assuming
 * subsequent code is unreachable. For CONFIG_SLUB_DEBUG, do this to catch more
 * bugs. Otherwise, use the typical inlined approach.
 *
 * Return: None
 */
void __qdf_bug(void);
#else /* CONFIG_SLUB_DEBUG */
static inline void __qdf_bug(void)
{
	BUG();
}
#endif /* CONFIG_SLUB_DEBUG */

/**
 * QDF_DEBUG_PANIC() - In debug builds, panic, otherwise do nothing
 * @reason_fmt: a format string containing the reason for the panic
 * @args: zero or more printf compatible logging arguments
 *
 * Return: None
 */
#define QDF_DEBUG_PANIC(reason_fmt, args...) \
	QDF_DEBUG_PANIC_FL(__func__, __LINE__, reason_fmt, ## args)

/**
 * QDF_DEBUG_PANIC_FL() - In debug builds, panic, otherwise do nothing
 * @func: origin function name to be logged
 * @line: origin line number to be logged
 * @fmt: printf compatible format string to be logged
 * @args: zero or more printf compatible logging arguments
 *
 * Return: None
 */
#define QDF_DEBUG_PANIC_FL(func, line, fmt, args...) \
	do { \
		pr_err("WLAN Panic @ %s:%d: " fmt "\n", func, line, ##args); \
		__qdf_bug(); \
	} while (false)

#define QDF_BUG(_condition) \
	do { \
		if (!(_condition)) { \
			pr_err("QDF BUG in %s Line %d: Failed assertion '" \
			       #_condition "'\n", __func__, __LINE__); \
			__qdf_bug(); \
		} \
	} while (0)

#else /* PANIC_ON_BUG */

#define QDF_DEBUG_PANIC(reason...) \
	do { \
		/* no-op */ \
	} while (false)

#define QDF_DEBUG_PANIC_FL(func, line, fmt, args...) \
	do { \
		/* no-op */ \
	} while (false)

#define QDF_BUG(_condition) \
	do { \
		if (!(_condition)) { \
			/* no-op */ \
		} \
	} while (0)

#endif /* PANIC_ON_BUG */

#ifdef KSYM_SYMBOL_LEN
#define __QDF_SYMBOL_LEN KSYM_SYMBOL_LEN
#else
#define __QDF_SYMBOL_LEN 1
#endif

#ifdef CONFIG_QCA_MINIDUMP
static inline void
__qdf_minidump_log(void *start_addr, size_t size, const char *name)
{
	if (fill_minidump_segments((uintptr_t)start_addr, size,
	    QCA_WDT_LOG_DUMP_TYPE_WLAN_MOD, (char *)name) < 0)
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
			"%s: failed to log %pK (%s)\n",
			__func__, start_addr, name);
}

static inline void
__qdf_minidump_remove(void *addr)
{
	remove_minidump_segments((uintptr_t)addr);
}
#else
static inline void
__qdf_minidump_log(void *start_addr, size_t size, const char *name) {}
static inline void
__qdf_minidump_remove(void *addr) {}
#endif
#endif /* __I_QDF_TRACE_H */
