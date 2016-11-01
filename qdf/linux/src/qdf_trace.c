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
 *  DOC:  qdf_trace
 *  QCA driver framework (QDF) trace APIs
 *  Trace, logging, and debugging definitions and APIs
 */

/* Include Files */
#include <qdf_trace.h>
#include <wlan_logging_sock_svc.h>
#include "qdf_time.h"
/* Preprocessor definitions and constants */

#define QDF_TRACE_BUFFER_SIZE (512)

enum qdf_timestamp_unit qdf_log_timestamp_type = QDF_LOG_TIMESTAMP_UNIT;

/* macro to map qdf trace levels into the bitmask */
#define QDF_TRACE_LEVEL_TO_MODULE_BITMASK(_level) ((1 << (_level)))

/**
 * typedef struct module_trace_info - Trace level for a module, as a bitmask.
 * The bits in this mask are ordered by QDF_TRACE_LEVEL.  For example,
 * each bit represents one of the bits in QDF_TRACE_LEVEL that may be turned
 * on to have traces at that level logged, i.e. if QDF_TRACE_LEVEL_ERROR is
 * == 2, then if bit 2 (low order) is turned ON, then ERROR traces will be
 * printed to the trace log. Note that all bits turned OFF means no traces
 * @module_trace_level: trace level
 * @module_name_str: 3 character string name for the module
 */
typedef struct {
	uint16_t module_trace_level;
	unsigned char module_name_str[4];
} module_trace_info;

#define QDF_DEFAULT_TRACE_LEVEL	\
	((1 << QDF_TRACE_LEVEL_FATAL) | (1 << QDF_TRACE_LEVEL_ERROR))

/* Array of static data that contains all of the per module trace
 * information.  This includes the trace level for the module and
 * the 3 character 'name' of the module for marking the trace logs
 */
module_trace_info g_qdf_trace_info[QDF_MODULE_ID_MAX] = {
	[QDF_MODULE_ID_TLSHIM] = {QDF_DEFAULT_TRACE_LEVEL, "DP"},
	[QDF_MODULE_ID_WMI] = {QDF_DEFAULT_TRACE_LEVEL, "WMI"},
	[QDF_MODULE_ID_HDD] = {QDF_DEFAULT_TRACE_LEVEL, "HDD"},
	[QDF_MODULE_ID_SME] = {QDF_DEFAULT_TRACE_LEVEL, "SME"},
	[QDF_MODULE_ID_PE] = {QDF_DEFAULT_TRACE_LEVEL, "PE "},
	[QDF_MODULE_ID_WMA] = {QDF_DEFAULT_TRACE_LEVEL, "WMA"},
	[QDF_MODULE_ID_SYS] = {QDF_DEFAULT_TRACE_LEVEL, "SYS"},
	[QDF_MODULE_ID_QDF] = {QDF_DEFAULT_TRACE_LEVEL, "QDF"},
	[QDF_MODULE_ID_SAP] = {QDF_DEFAULT_TRACE_LEVEL, "SAP"},
	[QDF_MODULE_ID_HDD_SOFTAP] = {QDF_DEFAULT_TRACE_LEVEL, "HSP"},
	[QDF_MODULE_ID_HDD_DATA] = {QDF_DEFAULT_TRACE_LEVEL, "HDP"},
	[QDF_MODULE_ID_HDD_SAP_DATA] = {QDF_DEFAULT_TRACE_LEVEL, "SDP"},
	[QDF_MODULE_ID_BMI] = {QDF_DEFAULT_TRACE_LEVEL, "BMI"},
	[QDF_MODULE_ID_HIF] = {QDF_DEFAULT_TRACE_LEVEL, "HIF"},
	[QDF_MODULE_ID_TXRX] = {QDF_DEFAULT_TRACE_LEVEL, "TRX"},
	[QDF_MODULE_ID_HTT] = {QDF_DEFAULT_TRACE_LEVEL, "HTT"},
};

/* Static and Global variables */
static spinlock_t ltrace_lock;

static qdf_trace_record_t g_qdf_trace_tbl[MAX_QDF_TRACE_RECORDS];
/* global qdf trace data */
static t_qdf_trace_data g_qdf_trace_data;
/*
 * all the call back functions for dumping MTRACE messages from ring buffer
 * are stored in qdf_trace_cb_table,these callbacks are initialized during init
 * only so, we will make a copy of these call back functions and maintain in to
 * qdf_trace_restore_cb_table. Incase if we make modifications to
 * qdf_trace_cb_table, we can certainly retrieve all the call back functions
 * back from Restore Table
 */
static tp_qdf_trace_cb qdf_trace_cb_table[QDF_MODULE_ID_MAX];
static tp_qdf_trace_cb qdf_trace_restore_cb_table[QDF_MODULE_ID_MAX];
static tp_qdf_state_info_cb qdf_state_info_table[QDF_MODULE_ID_MAX];

#ifdef FEATURE_DP_TRACE
/* Static and Global variables */
static spinlock_t l_dp_trace_lock;

static struct qdf_dp_trace_record_s
			g_qdf_dp_trace_tbl[MAX_QDF_DP_TRACE_RECORDS];

/*
 * all the options to configure/control DP trace are
 * defined in this structure
 */
static struct s_qdf_dp_trace_data g_qdf_dp_trace_data;
/*
 * all the call back functions for dumping DPTRACE messages from ring buffer
 * are stored in qdf_dp_trace_cb_table, callbacks are initialized during init
 */
static tp_qdf_dp_trace_cb qdf_dp_trace_cb_table[QDF_DP_TRACE_MAX+1];
#endif
/**
 * qdf_trace_set_level() - Set the trace level for a particular module
 * @module: Module id
 * @level : trace level
 *
 * Trace level is a member of the QDF_TRACE_LEVEL enumeration indicating
 * the severity of the condition causing the trace message to be issued.
 * More severe conditions are more likely to be logged.
 *
 * This is an external API that allows trace levels to be set for each module.
 *
 * Return:  None
 */
void qdf_trace_set_level(QDF_MODULE_ID module, QDF_TRACE_LEVEL level)
{
	/* make sure the caller is passing in a valid LEVEL */
	if (level >= QDF_TRACE_LEVEL_MAX) {
		pr_err("%s: Invalid trace level %d passed in!\n", __func__,
		       level);
		return;
	}

	/* Treat 'none' differently.  NONE means we have to run off all
	 * the bits in the bit mask so none of the traces appear. Anything
	 * other than 'none' means we need to turn ON a bit in the bitmask
	 */
	if (QDF_TRACE_LEVEL_NONE == level)
		g_qdf_trace_info[module].module_trace_level =
			QDF_TRACE_LEVEL_NONE;
	else
		/* set the desired bit in the bit mask for the module trace
		 * level */
		g_qdf_trace_info[module].module_trace_level |=
			QDF_TRACE_LEVEL_TO_MODULE_BITMASK(level);
}
EXPORT_SYMBOL(qdf_trace_set_level);

/**
 * qdf_trace_set_module_trace_level() - Set module trace level
 * @module: Module id
 * @level: Trace level for a module, as a bitmask as per 'module_trace_info'
 *
 * Sets the module trace level where the trace level is given as a bit mask
 *
 * Return: None
 */
void qdf_trace_set_module_trace_level(QDF_MODULE_ID module, uint32_t level)
{
	if (module < 0 || module >= QDF_MODULE_ID_MAX) {
		pr_err("%s: Invalid module id %d passed\n", __func__, module);
		return;
	}
	g_qdf_trace_info[module].module_trace_level = level;
}
EXPORT_SYMBOL(qdf_trace_set_module_trace_level);

/**
 * qdf_trace_set_value() - Set module trace value
 * @module: Module id
 * @level: Trace level for a module, as a bitmask as per 'module_trace_info'
 * @on: set/clear the desired bit in the bit mask
 *
 * Return: None
 */
void qdf_trace_set_value(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
			 uint8_t on)
{
	/* make sure the caller is passing in a valid LEVEL */
	if (level < 0 || level >= QDF_TRACE_LEVEL_MAX) {
		pr_err("%s: Invalid trace level %d passed in!\n", __func__,
		       level);
		return;
	}

	/* make sure the caller is passing in a valid module */
	if (module < 0 || module >= QDF_MODULE_ID_MAX) {
		pr_err("%s: Invalid module id %d passed in!\n", __func__,
		       module);
		return;
	}

	/* Treat 'none' differently.  NONE means we have to turn off all
	   the bits in the bit mask so none of the traces appear */
	if (QDF_TRACE_LEVEL_NONE == level) {
		g_qdf_trace_info[module].module_trace_level =
			QDF_TRACE_LEVEL_NONE;
	}
	/* Treat 'All' differently.  All means we have to turn on all
	   the bits in the bit mask so all of the traces appear */
	else if (QDF_TRACE_LEVEL_ALL == level) {
		g_qdf_trace_info[module].module_trace_level = 0xFFFF;
	} else {
		if (on)
			/* set the desired bit in the bit mask for the module
			   trace level */
			g_qdf_trace_info[module].module_trace_level |=
				QDF_TRACE_LEVEL_TO_MODULE_BITMASK(level);
		else
			/* clear the desired bit in the bit mask for the module
			   trace level */
			g_qdf_trace_info[module].module_trace_level &=
				~(QDF_TRACE_LEVEL_TO_MODULE_BITMASK(level));
	}
}
EXPORT_SYMBOL(qdf_trace_set_value);

/**
 * qdf_trace_get_level() - get the trace level
 * @module: module Id
 * @level: trace level
 *
 * This is an external API that returns a bool value to signify if a
 * particular trace level is set for the specified module.
 * A member of the QDF_TRACE_LEVEL enumeration indicating the severity
 * of the condition causing the trace message to be issued.
 *
 * Note that individual trace levels are the only valid values
 * for this API.  QDF_TRACE_LEVEL_NONE and QDF_TRACE_LEVEL_ALL
 * are not valid input and will return false
 *
 * Return:
 * false - the specified trace level for the specified module is OFF
 * true - the specified trace level for the specified module is ON
 */
bool qdf_trace_get_level(QDF_MODULE_ID module, QDF_TRACE_LEVEL level)
{
	bool trace_on = false;

	if ((QDF_TRACE_LEVEL_NONE == level) ||
	    (QDF_TRACE_LEVEL_ALL == level) || (level >= QDF_TRACE_LEVEL_MAX)) {
		trace_on = false;
	} else {
		trace_on = (level & g_qdf_trace_info[module].module_trace_level)
			  ? true : false;
	}

	return trace_on;
}
EXPORT_SYMBOL(qdf_trace_get_level);

/**
 * qdf_snprintf() - wrapper function to snprintf
 * @str_buffer: string Buffer
 * @size: defines the size of the data record
 * @str_format: Format string in which the message to be logged. This format
 * string contains printf-like replacement parameters, which follow
 * this parameter in the variable argument list.
 *
 * Return: None
 */
void qdf_snprintf(char *str_buffer, unsigned int size, char *str_format, ...)
{
	va_list val;

	va_start(val, str_format);
	snprintf(str_buffer, size, str_format, val);
	va_end(val);
}
EXPORT_SYMBOL(qdf_snprintf);

#ifdef QDF_ENABLE_TRACING

/**
 * qdf_trace_msg() - externally called trace function
 * @module: Module identifier a member of the QDF_MODULE_ID
 * enumeration that identifies the module issuing the trace message.
 * @level: Trace level a member of the QDF_TRACE_LEVEL enumeration
 * indicating the severity of the condition causing the trace message
 * to be issued. More severe conditions are more likely to be logged.
 * @str_format: Format string in which the message to be logged. This format
 * string contains printf-like replacement parameters, which follow
 * this parameter in the variable argument list.
 *
 * Checks the level of severity and accordingly prints the trace messages
 *
 * Return: None
 */
void qdf_trace_msg(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
		   char *str_format, ...)
{
	char str_buffer[QDF_TRACE_BUFFER_SIZE];
	int n;

	/* Print the trace message when the desired level bit is set in
	   the module tracel level mask */
	if (g_qdf_trace_info[module].module_trace_level &
	    QDF_TRACE_LEVEL_TO_MODULE_BITMASK(level)) {
		/* the trace level strings in an array.  these are ordered in
		 * the same order as the trace levels are defined in the enum
		 * (see QDF_TRACE_LEVEL) so we can index into this array with
		 * the level and get the right string. The qdf trace levels
		 * are... none, Fatal, Error, Warning, Info, info_high, info_med,
		 * info_low, Debug
		 */
		static const char *TRACE_LEVEL_STR[] = { "  ", "F ", "E ", "W ",
						"I ", "IH", "IM", "IL", "D" };
		va_list val;
		va_start(val, str_format);

		/* print the prefix string into the string buffer... */
		n = snprintf(str_buffer, QDF_TRACE_BUFFER_SIZE,
			     "wlan: [%d:%2s:%3s] ",
			     in_interrupt() ? 0 : current->pid,
			     (char *)TRACE_LEVEL_STR[level],
			     (char *)g_qdf_trace_info[module].module_name_str);

		/* print the formatted log message after the prefix string */
		if ((n >= 0) && (n < QDF_TRACE_BUFFER_SIZE)) {
			vsnprintf(str_buffer + n, QDF_TRACE_BUFFER_SIZE - n,
				  str_format, val);
#if defined(WLAN_LOGGING_SOCK_SVC_ENABLE)
			wlan_log_to_user(level, (char *)str_buffer,
					 strlen(str_buffer));
#else
			pr_err("%s\n", str_buffer);
#endif
		}
		va_end(val);
	}
}
EXPORT_SYMBOL(qdf_trace_msg);

/**
 * qdf_trace_display() - Display trace
 *
 * Return:  None
 */
void qdf_trace_display(void)
{
	QDF_MODULE_ID module_id;

	pr_err
		("     1)FATAL  2)ERROR  3)WARN  4)INFO  5)INFO_H  6)INFO_M  7)INFO_L 8)DEBUG\n");
	for (module_id = 0; module_id < QDF_MODULE_ID_MAX; ++module_id) {
		pr_err
			("%2d)%s    %s        %s       %s       %s        %s         %s         %s        %s\n",
			(int)module_id, g_qdf_trace_info[module_id].module_name_str,
			(g_qdf_trace_info[module_id].
			 module_trace_level & (1 << QDF_TRACE_LEVEL_FATAL)) ? "X" :
			" ",
			(g_qdf_trace_info[module_id].
			 module_trace_level & (1 << QDF_TRACE_LEVEL_ERROR)) ? "X" :
			" ",
			(g_qdf_trace_info[module_id].
			 module_trace_level & (1 << QDF_TRACE_LEVEL_WARN)) ? "X" :
			" ",
			(g_qdf_trace_info[module_id].
			 module_trace_level & (1 << QDF_TRACE_LEVEL_INFO)) ? "X" :
			" ",
			(g_qdf_trace_info[module_id].
			 module_trace_level & (1 << QDF_TRACE_LEVEL_INFO_HIGH)) ? "X"
			: " ",
			(g_qdf_trace_info[module_id].
			 module_trace_level & (1 << QDF_TRACE_LEVEL_INFO_MED)) ? "X"
			: " ",
			(g_qdf_trace_info[module_id].
			 module_trace_level & (1 << QDF_TRACE_LEVEL_INFO_LOW)) ? "X"
			: " ",
			(g_qdf_trace_info[module_id].
			 module_trace_level & (1 << QDF_TRACE_LEVEL_DEBUG)) ? "X" :
			" ");
	}
}
EXPORT_SYMBOL(qdf_trace_display);

#define ROW_SIZE 16
/* Buffer size = data bytes(2 hex chars plus space) + NULL */
#define BUFFER_SIZE ((ROW_SIZE * 3) + 1)

/**
 * qdf_trace_hex_dump() - externally called hex dump function
 * @module: Module identifier a member of the QDF_MODULE_ID enumeration that
 * identifies the module issuing the trace message.
 * @level: Trace level a member of the QDF_TRACE_LEVEL enumeration indicating
 * the severity of the condition causing the trace message to be
 * issued. More severe conditions are more likely to be logged.
 * @data: The base address of the buffer to be logged.
 * @buf_len: The size of the buffer to be logged.
 *
 * Checks the level of severity and accordingly prints the trace messages
 *
 * Return:  None
 */
void qdf_trace_hex_dump(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
			void *data, int buf_len)
{
	const u8 *ptr = data;
	int i, linelen, remaining = buf_len;
	unsigned char linebuf[BUFFER_SIZE];

	if (!(g_qdf_trace_info[module].module_trace_level &
		QDF_TRACE_LEVEL_TO_MODULE_BITMASK(level)))
		return;

	for (i = 0; i < buf_len; i += ROW_SIZE) {
		linelen = min(remaining, ROW_SIZE);
		remaining -= ROW_SIZE;

		hex_dump_to_buffer(ptr + i, linelen, ROW_SIZE, 1,
				linebuf, sizeof(linebuf), false);

		qdf_trace_msg(module, level, "%.8x: %s", i, linebuf);
	}
}
EXPORT_SYMBOL(qdf_trace_hex_dump);

#endif

/**
 * qdf_trace_enable() - Enable MTRACE for specific modules
 * @bitmask_of_module_id: Bitmask according to enum of the modules.
 *  32[dec] = 0010 0000 [bin] <enum of HDD is 5>
 *  64[dec] = 0100 0000 [bin] <enum of SME is 6>
 *  128[dec] = 1000 0000 [bin] <enum of PE is 7>
 * @enable: can be true or false true implies enabling MTRACE false implies
 *		disabling MTRACE.
 *
 * Enable MTRACE for specific modules whose bits are set in bitmask and enable
 * is true. if enable is false it disables MTRACE for that module. set the
 * bitmask according to enum value of the modules.
 * This functions will be called when you issue ioctl as mentioned following
 * [iwpriv wlan0 setdumplog <value> <enable>].
 * <value> - Decimal number, i.e. 64 decimal value shows only SME module,
 * 128 decimal value shows only PE module, 192 decimal value shows PE and SME.
 *
 * Return: None
 */
void qdf_trace_enable(uint32_t bitmask_of_module_id, uint8_t enable)
{
	int i;
	if (bitmask_of_module_id) {
		for (i = 0; i < QDF_MODULE_ID_MAX; i++) {
			if (((bitmask_of_module_id >> i) & 1)) {
				if (enable) {
					if (NULL !=
					    qdf_trace_restore_cb_table[i]) {
						qdf_trace_cb_table[i] =
						qdf_trace_restore_cb_table[i];
					}
				} else {
					qdf_trace_restore_cb_table[i] =
						qdf_trace_cb_table[i];
					qdf_trace_cb_table[i] = NULL;
				}
			}
		}
	} else {
		if (enable) {
			for (i = 0; i < QDF_MODULE_ID_MAX; i++) {
				if (NULL != qdf_trace_restore_cb_table[i]) {
					qdf_trace_cb_table[i] =
						qdf_trace_restore_cb_table[i];
				}
			}
		} else {
			for (i = 0; i < QDF_MODULE_ID_MAX; i++) {
				qdf_trace_restore_cb_table[i] =
					qdf_trace_cb_table[i];
				qdf_trace_cb_table[i] = NULL;
			}
		}
	}
}
EXPORT_SYMBOL(qdf_trace_enable);

/**
 * qdf_trace_init() - initializes qdf trace structures and variables
 *
 * Called immediately after cds_preopen, so that we can start recording HDD
 * events ASAP.
 *
 * Return: None
 */
void qdf_trace_init(void)
{
	uint8_t i;
	g_qdf_trace_data.head = INVALID_QDF_TRACE_ADDR;
	g_qdf_trace_data.tail = INVALID_QDF_TRACE_ADDR;
	g_qdf_trace_data.num = 0;
	g_qdf_trace_data.enable = true;
	g_qdf_trace_data.dump_count = DEFAULT_QDF_TRACE_DUMP_COUNT;
	g_qdf_trace_data.num_since_last_dump = 0;

	for (i = 0; i < QDF_MODULE_ID_MAX; i++) {
		qdf_trace_cb_table[i] = NULL;
		qdf_trace_restore_cb_table[i] = NULL;
	}
}
EXPORT_SYMBOL(qdf_trace_init);

/**
 * qdf_trace() - puts the messages in to ring-buffer
 * @module: Enum of module, basically module id.
 * @param: Code to be recorded
 * @session: Session ID of the log
 * @data: Actual message contents
 *
 * This function will be called from each module who wants record the messages
 * in circular queue. Before calling this functions make sure you have
 * registered your module with qdf through qdf_trace_register function.
 *
 * Return: None
 */
void qdf_trace(uint8_t module, uint8_t code, uint16_t session, uint32_t data)
{
	tp_qdf_trace_record rec = NULL;
	unsigned long flags;

	if (!g_qdf_trace_data.enable)
		return;

	/* if module is not registered, don't record for that module */
	if (NULL == qdf_trace_cb_table[module])
		return;

	/* Aquire the lock so that only one thread at a time can fill the ring
	 * buffer
	 */
	spin_lock_irqsave(&ltrace_lock, flags);

	g_qdf_trace_data.num++;

	if (g_qdf_trace_data.num > MAX_QDF_TRACE_RECORDS)
		g_qdf_trace_data.num = MAX_QDF_TRACE_RECORDS;

	if (INVALID_QDF_TRACE_ADDR == g_qdf_trace_data.head) {
		/* first record */
		g_qdf_trace_data.head = 0;
		g_qdf_trace_data.tail = 0;
	} else {
		/* queue is not empty */
		uint32_t tail = g_qdf_trace_data.tail + 1;

		if (MAX_QDF_TRACE_RECORDS == tail)
			tail = 0;

		if (g_qdf_trace_data.head == tail) {
			/* full */
			if (MAX_QDF_TRACE_RECORDS == ++g_qdf_trace_data.head)
				g_qdf_trace_data.head = 0;
		}
		g_qdf_trace_data.tail = tail;
	}

	rec = &g_qdf_trace_tbl[g_qdf_trace_data.tail];
	rec->code = code;
	rec->session = session;
	rec->data = data;
	rec->time = qdf_get_log_timestamp();
	rec->module = module;
	rec->pid = (in_interrupt() ? 0 : current->pid);
	g_qdf_trace_data.num_since_last_dump++;
	spin_unlock_irqrestore(&ltrace_lock, flags);
}
EXPORT_SYMBOL(qdf_trace);

/**
 * qdf_trace_spin_lock_init() - initializes the lock variable before use
 *
 * This function will be called from cds_alloc_global_context, we will have lock
 * available to use ASAP
 *
 * Return: None
 */
QDF_STATUS qdf_trace_spin_lock_init(void)
{
	spin_lock_init(&ltrace_lock);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_trace_spin_lock_init);

/**
 * qdf_trace_register() - registers the call back functions
 * @module_iD: enum value of module
 * @qdf_trace_callback: call back functions to display the messages in
 * particular format.
 *
 * Registers the call back functions to display the messages in particular
 * format mentioned in these call back functions. This functions should be
 * called by interested module in their init part as we will be ready to
 * register as soon as modules are up.
 *
 * Return: None
 */
void qdf_trace_register(QDF_MODULE_ID module_iD,
			tp_qdf_trace_cb qdf_trace_callback)
{
	qdf_trace_cb_table[module_iD] = qdf_trace_callback;
}
EXPORT_SYMBOL(qdf_trace_register);

/**
 * qdf_trace_dump_all() - Dump data from ring buffer via call back functions
 * registered with QDF
 * @p_mac: Context of particular module
 * @code: Reason code
 * @session: Session id of log
 * @count: Number of lines to dump starting from tail to head
 *
 * This function will be called up on issueing ioctl call as mentioned following
 * [iwpriv wlan0 dumplog 0 0 <n> <bitmask_of_module>]
 *
 * <n> - number lines to dump starting from tail to head.
 *
 * <bitmask_of_module> - if anybody wants to know how many messages were
 * recorded for particular module/s mentioned by setbit in bitmask from last
 * <n> messages. It is optional, if you don't provide then it will dump
 * everything from buffer.
 *
 * Return: None
 */
void qdf_trace_dump_all(void *p_mac, uint8_t code, uint8_t session,
			uint32_t count, uint32_t bitmask_of_module)
{
	qdf_trace_record_t p_record;
	int32_t i, tail;

	if (!g_qdf_trace_data.enable) {
		QDF_TRACE(QDF_MODULE_ID_SYS,
			  QDF_TRACE_LEVEL_ERROR, "Tracing Disabled");
		return;
	}

	QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_INFO,
		  "Total Records: %d, Head: %d, Tail: %d",
		  g_qdf_trace_data.num, g_qdf_trace_data.head,
		  g_qdf_trace_data.tail);

	/* aquire the lock so that only one thread at a time can read
	 * the ring buffer
	 */
	spin_lock(&ltrace_lock);

	if (g_qdf_trace_data.head != INVALID_QDF_TRACE_ADDR) {
		i = g_qdf_trace_data.head;
		tail = g_qdf_trace_data.tail;

		if (count) {
			if (count > g_qdf_trace_data.num)
				count = g_qdf_trace_data.num;
			if (tail >= (count - 1))
				i = tail - count + 1;
			else if (count != MAX_QDF_TRACE_RECORDS)
				i = MAX_QDF_TRACE_RECORDS - ((count - 1) -
							     tail);
		}

		p_record = g_qdf_trace_tbl[i];
		/* right now we are not using num_since_last_dump member but
		 * in future we might re-visit and use this member to track
		 * how many latest messages got added while we were dumping
		 * from ring buffer
		 */
		g_qdf_trace_data.num_since_last_dump = 0;
		spin_unlock(&ltrace_lock);
		for (;; ) {
			if ((code == 0 || (code == p_record.code)) &&
			    (qdf_trace_cb_table[p_record.module] != NULL)) {
				if (0 == bitmask_of_module) {
					qdf_trace_cb_table[p_record.
							   module] (p_mac,
								    &p_record,
								    (uint16_t)
								    i);
				} else {
					if (bitmask_of_module &
					    (1 << p_record.module)) {
						qdf_trace_cb_table[p_record.
								   module]
							(p_mac, &p_record,
							(uint16_t) i);
					}
				}
			}

			if (i == tail)
				break;
			i += 1;

			spin_lock(&ltrace_lock);
			if (MAX_QDF_TRACE_RECORDS == i) {
				i = 0;
				p_record = g_qdf_trace_tbl[0];
			} else {
				p_record = g_qdf_trace_tbl[i];
			}
			spin_unlock(&ltrace_lock);
		}
	} else {
		spin_unlock(&ltrace_lock);
	}
}
EXPORT_SYMBOL(qdf_trace_dump_all);

/**
 * qdf_register_debugcb_init() - initializes debug callbacks
 * to NULL
 *
 * Return: None
 */
void qdf_register_debugcb_init(void)
{
	uint8_t i;

	for (i = 0; i < QDF_MODULE_ID_MAX; i++)
		qdf_state_info_table[i] = NULL;
}
EXPORT_SYMBOL(qdf_register_debugcb_init);

/**
 * qdf_register_debug_callback() - stores callback handlers to print
 * state information
 * @module_id: module id of layer
 * @qdf_state_infocb: callback to be registered
 *
 * This function is used to store callback handlers to print
 * state information
 *
 * Return: None
 */
void qdf_register_debug_callback(QDF_MODULE_ID module_id,
					tp_qdf_state_info_cb qdf_state_infocb)
{
	qdf_state_info_table[module_id] = qdf_state_infocb;
}
EXPORT_SYMBOL(qdf_register_debug_callback);

/**
 * qdf_state_info_dump_all() - it invokes callback of layer which registered
 * its callback to print its state information.
 * @buf:  buffer pointer to be passed
 * @size:  size of buffer to be filled
 * @driver_dump_size: actual size of buffer used
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS qdf_state_info_dump_all(char *buf, uint16_t size,
			uint16_t *driver_dump_size)
{
	uint8_t module, ret = QDF_STATUS_SUCCESS;
	uint16_t buf_len = size;
	char *buf_ptr = buf;

	for (module = 0; module < QDF_MODULE_ID_MAX; module++) {
		if (NULL != qdf_state_info_table[module]) {
			qdf_state_info_table[module](&buf_ptr, &buf_len);
			if (!buf_len) {
				ret = QDF_STATUS_E_NOMEM;
				break;
			}
		}
	}

	*driver_dump_size = size - buf_len;
	return ret;
}
EXPORT_SYMBOL(qdf_state_info_dump_all);

#ifdef FEATURE_DP_TRACE
static void qdf_dp_unused(struct qdf_dp_trace_record_s *record,
			  uint16_t index)
{
	qdf_print("%s: QDF_DP_TRACE_MAX event should not be generated",
		  __func__);
}

/**
 * qdf_dp_trace_init() - enables the DP trace
 * Called during driver load and it enables DP trace
 *
 * Return: None
 */
void qdf_dp_trace_init(void)
{
	uint8_t i;

	qdf_dp_trace_spin_lock_init();
	g_qdf_dp_trace_data.head = INVALID_QDF_DP_TRACE_ADDR;
	g_qdf_dp_trace_data.tail = INVALID_QDF_DP_TRACE_ADDR;
	g_qdf_dp_trace_data.num = 0;
	g_qdf_dp_trace_data.proto_bitmap = QDF_NBUF_PKT_TRAC_TYPE_EAPOL |
					   QDF_NBUF_PKT_TRAC_TYPE_DHCP |
					   QDF_NBUF_PKT_TRAC_TYPE_MGMT_ACTION |
					   QDF_NBUF_PKT_TRAC_TYPE_ARP;
	g_qdf_dp_trace_data.no_of_record = 0;
	g_qdf_dp_trace_data.verbosity    = QDF_DP_TRACE_VERBOSITY_HIGH;
	g_qdf_dp_trace_data.enable = true;

	for (i = 0; i < ARRAY_SIZE(qdf_dp_trace_cb_table); i++)
		qdf_dp_trace_cb_table[i] = qdf_dp_display_record;

	qdf_dp_trace_cb_table[QDF_DP_TRACE_TXRX_PACKET_PTR_RECORD] =
	qdf_dp_trace_cb_table[QDF_DP_TRACE_TXRX_FAST_PACKET_PTR_RECORD] =
	qdf_dp_trace_cb_table[QDF_DP_TRACE_FREE_PACKET_PTR_RECORD] =
						qdf_dp_display_ptr_record;
	qdf_dp_trace_cb_table[QDF_DP_TRACE_EAPOL_PACKET_RECORD] =
	qdf_dp_trace_cb_table[QDF_DP_TRACE_DHCP_PACKET_RECORD] =
	qdf_dp_trace_cb_table[QDF_DP_TRACE_ARP_PACKET_RECORD] =
						qdf_dp_display_proto_pkt;
	qdf_dp_trace_cb_table[QDF_DP_TRACE_MGMT_PACKET_RECORD] =
					qdf_dp_display_mgmt_pkt;
	qdf_dp_trace_cb_table[QDF_DP_TRACE_EVENT_RECORD] =
					qdf_dp_display_event_record;

	qdf_dp_trace_cb_table[QDF_DP_TRACE_MAX] = qdf_dp_unused;
}
EXPORT_SYMBOL(qdf_dp_trace_init);

/**
 * qdf_dp_trace_set_value() - Configure the value to control DP trace
 * @proto_bitmap: defines the protocol to be tracked
 * @no_of_records: defines the nth packet which is traced
 * @verbosity: defines the verbosity level
 *
 * Return: None
 */
void qdf_dp_trace_set_value(uint8_t proto_bitmap, uint8_t no_of_record,
			    uint8_t verbosity)
{
	g_qdf_dp_trace_data.proto_bitmap = proto_bitmap;
	g_qdf_dp_trace_data.no_of_record = no_of_record;
	g_qdf_dp_trace_data.verbosity    = verbosity;
	return;
}
EXPORT_SYMBOL(qdf_dp_trace_set_value);

/**
 * qdf_dp_trace_enable_track() - enable the tracing for netbuf
 * @code: defines the event
 *
 * In High verbosity all codes are logged.
 * For Med/Low and Default case code which has
 * less value than corresponding verbosity codes
 * are logged.
 *
 * Return: true or false depends on whether tracing enabled
 */
static bool qdf_dp_trace_enable_track(enum QDF_DP_TRACE_ID code)
{
	switch (g_qdf_dp_trace_data.verbosity) {
	case QDF_DP_TRACE_VERBOSITY_HIGH:
		return true;
	case QDF_DP_TRACE_VERBOSITY_MEDIUM:
		if (code <= QDF_DP_TRACE_MED_VERBOSITY)
			return true;
		return false;
	case QDF_DP_TRACE_VERBOSITY_LOW:
		if (code <= QDF_DP_TRACE_LOW_VERBOSITY)
			return true;
		return false;
	case QDF_DP_TRACE_VERBOSITY_DEFAULT:
		if (code <= QDF_DP_TRACE_DEFAULT_VERBOSITY)
			return true;
		return false;
	default:
		return false;
	}
}
EXPORT_SYMBOL(qdf_dp_trace_enable_track);

/**
 * qdf_dp_get_proto_bitmap() - get dp trace proto bitmap
 *
 * Return: proto bitmap
 */
uint8_t qdf_dp_get_proto_bitmap(void)
{
	if (g_qdf_dp_trace_data.enable)
		return g_qdf_dp_trace_data.proto_bitmap;
	else
		return 0;
}

/**
 * qdf_dp_trace_set_track() - Marks whether the packet needs to be traced
 * @nbuf: defines the netbuf
 * @dir: direction
 *
 * Return: None
 */
void qdf_dp_trace_set_track(qdf_nbuf_t nbuf, enum qdf_proto_dir dir)
{
	uint32_t count = 0;

	spin_lock_bh(&l_dp_trace_lock);
	if (QDF_TX == dir)
		count = ++g_qdf_dp_trace_data.tx_count;
	else if (QDF_RX == dir)
		count = ++g_qdf_dp_trace_data.rx_count;

	if ((g_qdf_dp_trace_data.no_of_record != 0) &&
		(count % g_qdf_dp_trace_data.no_of_record == 0)) {
		if (QDF_TX == dir)
			QDF_NBUF_CB_TX_DP_TRACE(nbuf) = 1;
		else if (QDF_RX == dir)
			QDF_NBUF_CB_RX_DP_TRACE(nbuf) = 1;
	}
	spin_unlock_bh(&l_dp_trace_lock);
	return;
}
EXPORT_SYMBOL(qdf_dp_trace_set_track);

/**
 * dump_hex_trace() - Display the data in buffer
 * @buf:     buffer which contains data to be displayed
 * @buf_len: defines the size of the data to be displayed
 *
 * Return: None
 */
static void dump_hex_trace(char *str, uint8_t *buf, uint8_t buf_len)
{
	unsigned char linebuf[BUFFER_SIZE];
	const u8 *ptr = buf;
	int i, linelen, remaining = buf_len;

	/* Dump the bytes in the last line */
	for (i = 0; i < buf_len; i += ROW_SIZE) {
		linelen = min(remaining, ROW_SIZE);
		remaining -= ROW_SIZE;

		hex_dump_to_buffer(ptr + i, linelen, ROW_SIZE, 1,
				linebuf, sizeof(linebuf), false);

		qdf_trace_msg(QDF_MODULE_ID_QDF,
		   QDF_TRACE_LEVEL_ERROR, "DPT: %s: %s", str, linebuf);
	}
}
EXPORT_SYMBOL(dump_hex_trace);

/**
 * qdf_dp_code_to_string() - convert dptrace code to string
 * @code: dptrace code
 *
 * Return: string version of code
 */
static
const char *qdf_dp_code_to_string(enum QDF_DP_TRACE_ID code)
{
	switch (code) {
	case QDF_DP_TRACE_DROP_PACKET_RECORD:
		return "DROP:";
	case QDF_DP_TRACE_EAPOL_PACKET_RECORD:
		return "EAPOL:";
	case QDF_DP_TRACE_DHCP_PACKET_RECORD:
		return "DHCP:";
	case QDF_DP_TRACE_ARP_PACKET_RECORD:
		return "ARP:";
	case QDF_DP_TRACE_MGMT_PACKET_RECORD:
		return "MGMT:";
	case QDF_DP_TRACE_EVENT_RECORD:
		return "EVENT:";
	case QDF_DP_TRACE_HDD_TX_PACKET_PTR_RECORD:
		return "HDD: TX: PTR:";
	case QDF_DP_TRACE_HDD_TX_PACKET_RECORD:
		return "HDD: TX: DATA:";
	case QDF_DP_TRACE_CE_PACKET_PTR_RECORD:
		return "CE: TX: PTR:";
	case QDF_DP_TRACE_CE_FAST_PACKET_PTR_RECORD:
		return "CE: TX: FAST: PTR:";
	case QDF_DP_TRACE_FREE_PACKET_PTR_RECORD:
		return "FREE: TX: PTR:";
	case QDF_DP_TRACE_RX_HTT_PACKET_PTR_RECORD:
		return "HTT: RX: PTR:";
	case QDF_DP_TRACE_RX_OFFLOAD_HTT_PACKET_PTR_RECORD:
		return "HTT: RX: OF: PTR:";
	case QDF_DP_TRACE_RX_HDD_PACKET_PTR_RECORD:
		return "HDD: RX: PTR:";
	case QDF_DP_TRACE_TXRX_QUEUE_PACKET_PTR_RECORD:
		return "TXRX: TX: Q: PTR:";
	case QDF_DP_TRACE_TXRX_PACKET_PTR_RECORD:
		return "TXRX: TX: PTR:";
	case QDF_DP_TRACE_TXRX_FAST_PACKET_PTR_RECORD:
		return "TXRX: TX: FAST: PTR:";
	case QDF_DP_TRACE_HTT_PACKET_PTR_RECORD:
		return "HTT: TX: PTR:";
	case QDF_DP_TRACE_HTC_PACKET_PTR_RECORD:
		return "HTC: TX: PTR:";
	case QDF_DP_TRACE_HIF_PACKET_PTR_RECORD:
		return "HIF: TX: PTR:";
	case QDF_DP_TRACE_RX_TXRX_PACKET_PTR_RECORD:
		return "TXRX: RX: PTR:";
	case QDF_DP_TRACE_HDD_TX_TIMEOUT:
		return "HDD: STA: TO:";
	case QDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT:
		return "HDD: SAP: TO:";
	default:
		return "Invalid";
	}
}

/**
 * qdf_dp_dir_to_str() - convert direction to string
 * @dir: direction
 *
 * Return: string version of direction
 */
static const char *qdf_dp_dir_to_str(enum qdf_proto_dir dir)
{
	switch (dir) {
	case QDF_TX:
		return " --> ";
	case QDF_RX:
		return " <-- ";
	default:
		return "invalid";
	}
}

/**
 * qdf_dp_type_to_str() - convert packet type to string
 * @type: type
 *
 * Return: string version of packet type
 */
static const char *qdf_dp_type_to_str(enum qdf_proto_type type)
{
	switch (type) {
	case QDF_PROTO_TYPE_DHCP:
		return "DHCP";
	case QDF_PROTO_TYPE_EAPOL:
		return "EAPOL";
	case QDF_PROTO_TYPE_ARP:
		return "ARP";
	case QDF_PROTO_TYPE_MGMT:
		return "MGMT";
	case QDF_PROTO_TYPE_EVENT:
		return "EVENT";
	default:
		return "invalid";
	}
}

/**
 * qdf_dp_subtype_to_str() - convert packet subtype to string
 * @type: type
 *
 * Return: string version of packet subtype
 */
static const char *qdf_dp_subtype_to_str(enum qdf_proto_subtype subtype)
{
	switch (subtype) {
	case QDF_PROTO_EAPOL_M1:
		return "M1";
	case QDF_PROTO_EAPOL_M2:
		return "M2";
	case QDF_PROTO_EAPOL_M3:
		return "M3";
	case QDF_PROTO_EAPOL_M4:
		return "M4";
	case QDF_PROTO_DHCP_DISCOVER:
		return "DISCOVER";
	case QDF_PROTO_DHCP_REQUEST:
		return "REQUEST";
	case QDF_PROTO_DHCP_OFFER:
		return "OFFER";
	case QDF_PROTO_DHCP_ACK:
		return "ACK";
	case QDF_PROTO_DHCP_NACK:
		return "NACK";
	case QDF_PROTO_DHCP_RELEASE:
		return "RELEASE";
	case QDF_PROTO_DHCP_INFORM:
		return "INFORM";
	case QDF_PROTO_DHCP_DECLINE:
		return "DECLINE";
	case QDF_PROTO_ARP_REQ:
		return "REQUEST";
	case QDF_PROTO_ARP_RES:
		return "RESPONSE";
	case QDF_PROTO_MGMT_ASSOC:
		return "ASSOC";
	case QDF_PROTO_MGMT_DISASSOC:
		return "DISASSOC";
	case QDF_PROTO_MGMT_AUTH:
		return "AUTH";
	case QDF_PROTO_MGMT_DEAUTH:
		return "DEAUTH";
	case QDF_ROAM_SYNCH:
		return "ROAM SYNCH";
	case QDF_ROAM_COMPLETE:
		return "ROAM COMPLETE";
	case QDF_ROAM_EVENTID:
		return "ROAM EVENTID";
	default:
		return "invalid";
	}
}

/**
 * qdf_dp_enable_check() - check if dptrace is enable or not
 * @nbuf: nbuf
 * @code: dptrace code
 *
 * Return: true/false
 */
static bool qdf_dp_enable_check(qdf_nbuf_t nbuf, enum QDF_DP_TRACE_ID code,
				enum qdf_proto_dir dir)
{
	/* Return when Dp trace is not enabled */
	if (!g_qdf_dp_trace_data.enable)
		return false;

	if (qdf_dp_trace_enable_track(code) == false)
		return false;

	if ((nbuf) && ((QDF_NBUF_CB_TX_PACKET_TRACK(nbuf) !=
		 QDF_NBUF_TX_PKT_DATA_TRACK) ||
		 ((dir == QDF_TX) && (QDF_NBUF_CB_TX_DP_TRACE(nbuf) == 0)) ||
		 ((dir == QDF_RX) && (QDF_NBUF_CB_RX_DP_TRACE(nbuf) == 0))))
		return false;

	return true;
}

/**
 * qdf_dp_add_record() - add dp trace record
 * @code: dptrace code
 * @data: data pointer
 * @size: size of buffer
 * @print: true to print it in kmsg
 *
 * Return: none
 */
static void qdf_dp_add_record(enum QDF_DP_TRACE_ID code,
			      uint8_t *data, uint8_t size, bool print)
{
	struct qdf_dp_trace_record_s *rec = NULL;
	int index;
	spin_lock_bh(&l_dp_trace_lock);

	g_qdf_dp_trace_data.num++;

	if (g_qdf_dp_trace_data.num > MAX_QDF_DP_TRACE_RECORDS)
		g_qdf_dp_trace_data.num = MAX_QDF_DP_TRACE_RECORDS;

	if (INVALID_QDF_DP_TRACE_ADDR == g_qdf_dp_trace_data.head) {
		/* first record */
		g_qdf_dp_trace_data.head = 0;
		g_qdf_dp_trace_data.tail = 0;
	} else {
		/* queue is not empty */
		g_qdf_dp_trace_data.tail++;

		if (MAX_QDF_DP_TRACE_RECORDS == g_qdf_dp_trace_data.tail)
			g_qdf_dp_trace_data.tail = 0;

		if (g_qdf_dp_trace_data.head == g_qdf_dp_trace_data.tail) {
			/* full */
			if (MAX_QDF_DP_TRACE_RECORDS ==
				++g_qdf_dp_trace_data.head)
				g_qdf_dp_trace_data.head = 0;
		}
	}

	rec = &g_qdf_dp_trace_tbl[g_qdf_dp_trace_data.tail];
	index = g_qdf_dp_trace_data.tail;
	rec->code = code;
	rec->size = 0;
	if (data != NULL && size > 0) {
		if (size > QDF_DP_TRACE_RECORD_SIZE)
			size = QDF_DP_TRACE_RECORD_SIZE;

		rec->size = size;
		qdf_mem_copy(rec->data, data, size);
	}
	rec->time = qdf_get_log_timestamp();
	rec->pid = (in_interrupt() ? 0 : current->pid);
	spin_unlock_bh(&l_dp_trace_lock);

	if (g_qdf_dp_trace_data.live_mode || (print == true))
		qdf_dp_trace_cb_table[rec->code] (rec, index);
}


/**
 * qdf_log_eapol_pkt() - log EAPOL packet
 * @session_id: vdev_id
 * @skb: skb pointer
 * @dir: direction
 *
 * Return: true/false
 */
static bool qdf_log_eapol_pkt(uint8_t session_id, struct sk_buff *skb,
			      enum qdf_proto_dir dir)
{
	enum qdf_proto_subtype subtype;

	if ((qdf_dp_get_proto_bitmap() & QDF_NBUF_PKT_TRAC_TYPE_EAPOL) &&
		((dir == QDF_TX && QDF_NBUF_CB_PACKET_TYPE_EAPOL ==
			QDF_NBUF_CB_GET_PACKET_TYPE(skb)) ||
		 (dir == QDF_RX && qdf_nbuf_is_ipv4_eapol_pkt(skb) == true))) {

		subtype = qdf_nbuf_get_eapol_subtype(skb);
		DPTRACE(qdf_dp_trace_proto_pkt(QDF_DP_TRACE_EAPOL_PACKET_RECORD,
			session_id, (skb->data + QDF_NBUF_SRC_MAC_OFFSET),
			(skb->data + QDF_NBUF_DEST_MAC_OFFSET),
			QDF_PROTO_TYPE_EAPOL, subtype, dir));
		if (QDF_TX == dir)
			QDF_NBUF_CB_TX_DP_TRACE(skb) = 1;
		else if (QDF_RX == dir)
			QDF_NBUF_CB_RX_DP_TRACE(skb) = 1;

		QDF_NBUF_CB_DP_TRACE_PRINT(skb) = true;
		return true;
	}
	return false;
}

/**
 * qdf_log_dhcp_pkt() - log DHCP packet
 * @session_id: vdev_id
 * @skb: skb pointer
 * @dir: direction
 *
 * Return: true/false
 */
static bool qdf_log_dhcp_pkt(uint8_t session_id, struct sk_buff *skb,
			     enum qdf_proto_dir dir)
{
	enum qdf_proto_subtype subtype = QDF_PROTO_INVALID;

	if ((qdf_dp_get_proto_bitmap() & QDF_NBUF_PKT_TRAC_TYPE_DHCP) &&
		((dir == QDF_TX && QDF_NBUF_CB_PACKET_TYPE_DHCP ==
				QDF_NBUF_CB_GET_PACKET_TYPE(skb)) ||
		 (dir == QDF_RX && qdf_nbuf_is_ipv4_dhcp_pkt(skb) == true))) {

		subtype = qdf_nbuf_get_dhcp_subtype(skb);
		DPTRACE(qdf_dp_trace_proto_pkt(QDF_DP_TRACE_DHCP_PACKET_RECORD,
			session_id, (skb->data + QDF_NBUF_SRC_MAC_OFFSET),
			(skb->data + QDF_NBUF_DEST_MAC_OFFSET),
			QDF_PROTO_TYPE_DHCP, subtype, dir));
		if (QDF_TX == dir)
			QDF_NBUF_CB_TX_DP_TRACE(skb) = 1;
		else if (QDF_RX == dir)
			QDF_NBUF_CB_RX_DP_TRACE(skb) = 1;

		QDF_NBUF_CB_DP_TRACE_PRINT(skb) = true;
		return true;
	}
	return false;
}

/**
 * qdf_log_arp_pkt() - log ARP packet
 * @session_id: vdev_id
 * @skb: skb pointer
 * @dir: direction
 *
 * Return: true/false
 */
static bool qdf_log_arp_pkt(uint8_t session_id, struct sk_buff *skb,
			    enum qdf_proto_dir dir)
{
	enum qdf_proto_subtype proto_subtype;

	if ((qdf_dp_get_proto_bitmap() & QDF_NBUF_PKT_TRAC_TYPE_ARP) &&
		((dir == QDF_TX && QDF_NBUF_CB_PACKET_TYPE_ARP ==
			QDF_NBUF_CB_GET_PACKET_TYPE(skb)) ||
		 (dir == QDF_RX && qdf_nbuf_is_ipv4_arp_pkt(skb) == true))) {

		proto_subtype = qdf_nbuf_get_arp_subtype(skb);
		DPTRACE(qdf_dp_trace_proto_pkt(QDF_DP_TRACE_ARP_PACKET_RECORD,
			session_id, (skb->data + QDF_NBUF_SRC_MAC_OFFSET),
			(skb->data + QDF_NBUF_DEST_MAC_OFFSET),
			QDF_PROTO_TYPE_ARP, proto_subtype, dir));
		if (QDF_TX == dir)
			QDF_NBUF_CB_TX_DP_TRACE(skb) = 1;
		else if (QDF_RX == dir)
			QDF_NBUF_CB_RX_DP_TRACE(skb) = 1;

		QDF_NBUF_CB_DP_TRACE_PRINT(skb) = true;
		return true;
	}
	return false;
}

/**
 * qdf_dp_trace_log_pkt() - log packet type enabled through iwpriv
 * @session_id: vdev_id
 * @skb: skb pointer
 * @dir: direction
 *
 * Return: none
 */
void qdf_dp_trace_log_pkt(uint8_t session_id, struct sk_buff *skb,
			  enum qdf_proto_dir dir)
{
	if (qdf_dp_get_proto_bitmap()) {
		if (qdf_log_arp_pkt(session_id,
			skb, dir) == false) {
			if (qdf_log_dhcp_pkt(session_id,
				skb, dir) == false) {
				if (qdf_log_eapol_pkt(session_id,
					skb, dir) == false) {
					return;
				}
			}
		}
	}
}
EXPORT_SYMBOL(qdf_dp_trace_log_pkt);

/**
 * qdf_dp_display_mgmt_pkt() - display proto packet
 * @record: dptrace record
 * @index: index
 *
 * Return: none
 */
void qdf_dp_display_mgmt_pkt(struct qdf_dp_trace_record_s *record,
			      uint16_t index)
{
	struct qdf_dp_trace_mgmt_buf *buf =
		(struct qdf_dp_trace_mgmt_buf *)record->data;

	qdf_print("DPT: %04d: %012llu: %s vdev_id %d", index,
		record->time, qdf_dp_code_to_string(record->code),
		buf->vdev_id);
	qdf_print("DPT: Type %s Subtype %s", qdf_dp_type_to_str(buf->type),
		qdf_dp_subtype_to_str(buf->subtype));
}
EXPORT_SYMBOL(qdf_dp_display_mgmt_pkt);

/**
 * qdf_dp_trace_mgmt_pkt() - record mgmt packet
 * @code: dptrace code
 * @vdev_id: vdev id
 * @type: proto type
 * @subtype: proto subtype
 *
 * Return: none
 */
void qdf_dp_trace_mgmt_pkt(enum QDF_DP_TRACE_ID code, uint8_t vdev_id,
		enum qdf_proto_type type, enum qdf_proto_subtype subtype)
{
	struct qdf_dp_trace_mgmt_buf buf;
	int buf_size = sizeof(struct qdf_dp_trace_mgmt_buf);

	if (qdf_dp_enable_check(NULL, code, QDF_NA) == false)
		return;

	if (buf_size > QDF_DP_TRACE_RECORD_SIZE)
		QDF_BUG(0);

	buf.type = type;
	buf.subtype = subtype;
	buf.vdev_id = vdev_id;
	qdf_dp_add_record(code, (uint8_t *)&buf, buf_size, true);
}
EXPORT_SYMBOL(qdf_dp_trace_mgmt_pkt);

/**
 * qdf_dp_display_event_record() - display event records
 * @record: dptrace record
 * @index: index
 *
 * Return: none
 */
void qdf_dp_display_event_record(struct qdf_dp_trace_record_s *record,
			      uint16_t index)
{
	struct qdf_dp_trace_event_buf *buf =
		(struct qdf_dp_trace_event_buf *)record->data;

	qdf_print("DPT: %04d: %012llu: %s vdev_id %d", index,
		record->time, qdf_dp_code_to_string(record->code),
		buf->vdev_id);
	qdf_print("DPT: Type %s Subtype %s", qdf_dp_type_to_str(buf->type),
		qdf_dp_subtype_to_str(buf->subtype));
}
EXPORT_SYMBOL(qdf_dp_display_event_record);

/**
 * qdf_dp_trace_record_event() - record events
 * @code: dptrace code
 * @vdev_id: vdev id
 * @type: proto type
 * @subtype: proto subtype
 *
 * Return: none
 */
void qdf_dp_trace_record_event(enum QDF_DP_TRACE_ID code, uint8_t vdev_id,
		enum qdf_proto_type type, enum qdf_proto_subtype subtype)
{
	struct qdf_dp_trace_event_buf buf;
	int buf_size = sizeof(struct qdf_dp_trace_event_buf);

	if (qdf_dp_enable_check(NULL, code, QDF_NA) == false)
		return;

	if (buf_size > QDF_DP_TRACE_RECORD_SIZE)
		QDF_BUG(0);

	buf.type = type;
	buf.subtype = subtype;
	buf.vdev_id = vdev_id;
	qdf_dp_add_record(code, (uint8_t *)&buf, buf_size, true);
}
EXPORT_SYMBOL(qdf_dp_trace_record_event);

/**
 * qdf_dp_display_proto_pkt() - display proto packet
 * @record: dptrace record
 * @index: index
 *
 * Return: none
 */
void qdf_dp_display_proto_pkt(struct qdf_dp_trace_record_s *record,
			      uint16_t index)
{
	struct qdf_dp_trace_proto_buf *buf =
		(struct qdf_dp_trace_proto_buf *)record->data;

	qdf_print("DPT: %04d: %012llu: %s vdev_id %d", index,
		record->time, qdf_dp_code_to_string(record->code),
		buf->vdev_id);
	qdf_print("DPT: SA: " QDF_MAC_ADDRESS_STR " %s DA: "
		  QDF_MAC_ADDRESS_STR " Type %s Subtype %s",
		QDF_MAC_ADDR_ARRAY(buf->sa.bytes), qdf_dp_dir_to_str(buf->dir),
		QDF_MAC_ADDR_ARRAY(buf->da.bytes),
		qdf_dp_type_to_str(buf->type),
		qdf_dp_subtype_to_str(buf->subtype));
}
EXPORT_SYMBOL(qdf_dp_display_proto_pkt);

/**
 * qdf_dp_trace_proto_pkt() - record proto packet
 * @code: dptrace code
 * @vdev_id: vdev id
 * @sa: source mac address
 * @da: destination mac address
 * @type: proto type
 * @subtype: proto subtype
 * @dir: direction
 *
 * Return: none
 */
void qdf_dp_trace_proto_pkt(enum QDF_DP_TRACE_ID code, uint8_t vdev_id,
		uint8_t *sa, uint8_t *da, enum qdf_proto_type type,
		enum qdf_proto_subtype subtype, enum qdf_proto_dir dir)
{
	struct qdf_dp_trace_proto_buf buf;
	int buf_size = sizeof(struct qdf_dp_trace_ptr_buf);

	if (qdf_dp_enable_check(NULL, code, dir) == false)
		return;

	if (buf_size > QDF_DP_TRACE_RECORD_SIZE)
		QDF_BUG(0);

	memcpy(&buf.sa, sa, QDF_NET_ETH_LEN);
	memcpy(&buf.da, da, QDF_NET_ETH_LEN);
	buf.dir = dir;
	buf.type = type;
	buf.subtype = subtype;
	buf.vdev_id = vdev_id;
	qdf_dp_add_record(code, (uint8_t *)&buf, buf_size, true);
}
EXPORT_SYMBOL(qdf_dp_trace_proto_pkt);

/**
 * qdf_dp_display_ptr_record() - display record
 * @record: dptrace record
 * @index: index
 *
 * Return: none
 */
void qdf_dp_display_ptr_record(struct qdf_dp_trace_record_s *record,
				uint16_t index)
{
	struct qdf_dp_trace_ptr_buf *buf =
		(struct qdf_dp_trace_ptr_buf *)record->data;

	if (record->code == QDF_DP_TRACE_FREE_PACKET_PTR_RECORD)
		qdf_print("DPT: %04d: %012llu: %s msdu_id: %d, status: %d",
			 index, record->time,
			 qdf_dp_code_to_string(record->code), buf->msdu_id,
			 buf->status);
	else
		qdf_print("DPT: %04d: %012llu: %s msdu_id: %d, vdev_id: %d",
			 index,
			 record->time, qdf_dp_code_to_string(record->code),
			 buf->msdu_id, buf->status);
	dump_hex_trace("cookie", (uint8_t *)&buf->cookie, sizeof(buf->cookie));
}
EXPORT_SYMBOL(qdf_dp_display_ptr_record);

/**
 * qdf_dp_trace_ptr() - record dptrace
 * @code: dptrace code
 * @data: data
 * @size: size of data
 * @msdu_id: msdu_id
 * @status: return status
 *
 * Return: none
 */
void qdf_dp_trace_ptr(qdf_nbuf_t nbuf, enum QDF_DP_TRACE_ID code,
		uint8_t *data, uint8_t size, uint16_t msdu_id, uint16_t status)
{
	struct qdf_dp_trace_ptr_buf buf;
	int buf_size = sizeof(struct qdf_dp_trace_ptr_buf);

	if (qdf_dp_enable_check(nbuf, code, QDF_TX) == false)
		return;

	if (buf_size > QDF_DP_TRACE_RECORD_SIZE)
		QDF_BUG(0);

	qdf_mem_copy(&buf.cookie, data, size);
	buf.msdu_id = msdu_id;
	buf.status = status;
	qdf_dp_add_record(code, (uint8_t *)&buf, buf_size,
				QDF_NBUF_CB_DP_TRACE_PRINT(nbuf));
}
EXPORT_SYMBOL(qdf_dp_trace_ptr);

/**
 * qdf_dp_display_trace() - Displays a record in DP trace
 * @pRecord  : pointer to a record in DP trace
 * @recIndex : record index
 *
 * Return: None
 */
void qdf_dp_display_record(struct qdf_dp_trace_record_s *pRecord,
				uint16_t recIndex)
{
	qdf_print("DPT: %04d: %012llu: %s", recIndex,
		pRecord->time, qdf_dp_code_to_string(pRecord->code));
	switch (pRecord->code) {
	case  QDF_DP_TRACE_HDD_TX_TIMEOUT:
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
						"HDD TX Timeout\n");
		break;
	case  QDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT:
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
						"HDD SoftAP TX Timeout\n");
		break;
	case QDF_DP_TRACE_HDD_TX_PACKET_RECORD:
		dump_hex_trace("DATA", pRecord->data, pRecord->size);
		break;
	default:
		dump_hex_trace("cookie", pRecord->data, pRecord->size);
	}
}
EXPORT_SYMBOL(qdf_dp_display_record);


/**
 * qdf_dp_trace() - Stores the data in buffer
 * @nbuf  : defines the netbuf
 * @code : defines the event
 * @data : defines the data to be stored
 * @size : defines the size of the data record
 *
 * Return: None
 */
void qdf_dp_trace(qdf_nbuf_t nbuf, enum QDF_DP_TRACE_ID code,
			uint8_t *data, uint8_t size, enum qdf_proto_dir dir)
{

	if (qdf_dp_enable_check(nbuf, code, dir) == false)
		return;

	qdf_dp_add_record(code, data, size,
		(nbuf != NULL) ? QDF_NBUF_CB_DP_TRACE_PRINT(nbuf) : false);
}
EXPORT_SYMBOL(qdf_dp_trace);

/**
 * qdf_dp_trace_spin_lock_init() - initializes the lock variable before use
 * This function will be called from cds_alloc_global_context, we will have lock
 * available to use ASAP
 *
 * Return: None
 */
void qdf_dp_trace_spin_lock_init(void)
{
	spin_lock_init(&l_dp_trace_lock);
}
EXPORT_SYMBOL(qdf_dp_trace_spin_lock_init);

/**
 * qdf_dp_trace_enable_live_mode() - enable live mode for dptrace
 *
 * Return: none
 */
void qdf_dp_trace_enable_live_mode(void)
{
	g_qdf_dp_trace_data.live_mode = 1;

}
EXPORT_SYMBOL(qdf_dp_trace_enable_live_mode);


/**
 * qdf_dp_trace_clear_buffer() - clear dp trace buffer
 *
 * Return: none
 */
void qdf_dp_trace_clear_buffer(void)
{
	g_qdf_dp_trace_data.head = INVALID_QDF_DP_TRACE_ADDR;
	g_qdf_dp_trace_data.tail = INVALID_QDF_DP_TRACE_ADDR;
	g_qdf_dp_trace_data.num = 0;
	g_qdf_dp_trace_data.proto_bitmap = QDF_NBUF_PKT_TRAC_TYPE_EAPOL |
	    QDF_NBUF_PKT_TRAC_TYPE_DHCP | QDF_NBUF_PKT_TRAC_TYPE_MGMT_ACTION;
	g_qdf_dp_trace_data.no_of_record = 0;
	g_qdf_dp_trace_data.verbosity    = QDF_DP_TRACE_VERBOSITY_LOW;
	g_qdf_dp_trace_data.enable = true;

	memset(g_qdf_dp_trace_tbl, 0,
	   MAX_QDF_DP_TRACE_RECORDS * sizeof(struct qdf_dp_trace_record_s));
}
EXPORT_SYMBOL(qdf_dp_trace_clear_buffer);

/**
 * qdf_dp_trace_dump_all() - Dump data from ring buffer via call back functions
 * registered with QDF
 * @code: Reason code
 * @count: Number of lines to dump starting from tail to head
 *
 * Return: None
 */
void qdf_dp_trace_dump_all(uint32_t count)
{
	struct qdf_dp_trace_record_s p_record;
	int32_t i, tail;

	if (!g_qdf_dp_trace_data.enable) {
		QDF_TRACE(QDF_MODULE_ID_SYS,
			  QDF_TRACE_LEVEL_ERROR, "Tracing Disabled");
		return;
	}

	QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_ERROR,
		  "Total Records: %d, Head: %d, Tail: %d",
		  g_qdf_dp_trace_data.num, g_qdf_dp_trace_data.head,
		  g_qdf_dp_trace_data.tail);

	/* aquire the lock so that only one thread at a time can read
	 * the ring buffer
	 */
	spin_lock_bh(&l_dp_trace_lock);

	if (g_qdf_dp_trace_data.head != INVALID_QDF_DP_TRACE_ADDR) {
		i = g_qdf_dp_trace_data.head;
		tail = g_qdf_dp_trace_data.tail;

		if (count) {
			if (count > g_qdf_dp_trace_data.num)
				count = g_qdf_dp_trace_data.num;
			if (tail >= (count - 1))
				i = tail - count + 1;
			else if (count != MAX_QDF_DP_TRACE_RECORDS)
				i = MAX_QDF_DP_TRACE_RECORDS - ((count - 1) -
							     tail);
		}

		p_record = g_qdf_dp_trace_tbl[i];
		spin_unlock_bh(&l_dp_trace_lock);
		for (;; ) {

			qdf_dp_trace_cb_table[p_record.
					   code] (&p_record, (uint16_t)i);
			if (i == tail)
				break;
			i += 1;

			spin_lock_bh(&l_dp_trace_lock);
			if (MAX_QDF_DP_TRACE_RECORDS == i)
				i = 0;

			p_record = g_qdf_dp_trace_tbl[i];
			spin_unlock_bh(&l_dp_trace_lock);
		}
	} else {
		spin_unlock_bh(&l_dp_trace_lock);
	}
}
EXPORT_SYMBOL(qdf_dp_trace_dump_all);
#endif
