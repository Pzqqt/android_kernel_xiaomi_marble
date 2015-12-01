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

/**
 *  DOC:  cdf_trace
 *
 *  Connectivity driver framework (CDF) trace APIs
 *
 *  Trace, logging, and debugging definitions and APIs
 *
 */

/* Include Files */
#include <cdf_trace.h>
#include <ani_global.h>
#include <wlan_logging_sock_svc.h>
#include "cdf_time.h"
/* Preprocessor definitions and constants */

#define CDF_TRACE_BUFFER_SIZE (512)

enum cdf_timestamp_unit cdf_log_timestamp_type = CDF_LOG_TIMESTAMP_UNIT;

/* macro to map cdf trace levels into the bitmask */
#define CDF_TRACE_LEVEL_TO_MODULE_BITMASK(_level) ((1 << (_level)))

typedef struct {
	/* Trace level for a module, as a bitmask.  The bits in this mask
	 * are ordered by CDF_TRACE_LEVEL.  For example, each bit represents
	 * one of the bits in CDF_TRACE_LEVEL that may be turned on to have
	 * traces at that level logged, i.e. if CDF_TRACE_LEVEL_ERROR is
	 * == 2, then if bit 2 (low order) is turned ON, then ERROR traces
	 * will be printed to the trace log.
	 * Note that all bits turned OFF means no traces
	 */
	uint16_t moduleTraceLevel;

	/* 3 character string name for the module */
	unsigned char moduleNameStr[4]; /* 3 chars plus the NULL */
} moduleTraceInfo;

#define CDF_DEFAULT_TRACE_LEVEL	\
	((1 << CDF_TRACE_LEVEL_FATAL) | (1 << CDF_TRACE_LEVEL_ERROR))

/* Array of static data that contains all of the per module trace
 * information.  This includes the trace level for the module and
 * the 3 character 'name' of the module for marking the trace logs
 */
moduleTraceInfo g_cdf_trace_info[CDF_MODULE_ID_MAX] = {
	[CDF_MODULE_ID_TLSHIM] = {CDF_DEFAULT_TRACE_LEVEL, "DP"},
	[CDF_MODULE_ID_WMI] = {CDF_DEFAULT_TRACE_LEVEL, "WMI"},
	[CDF_MODULE_ID_HDD] = {CDF_DEFAULT_TRACE_LEVEL, "HDD"},
	[CDF_MODULE_ID_SME] = {CDF_DEFAULT_TRACE_LEVEL, "SME"},
	[CDF_MODULE_ID_PE] = {CDF_DEFAULT_TRACE_LEVEL, "PE "},
	[CDF_MODULE_ID_WMA] = {CDF_DEFAULT_TRACE_LEVEL, "WMA"},
	[CDF_MODULE_ID_SYS] = {CDF_DEFAULT_TRACE_LEVEL, "SYS"},
	[CDF_MODULE_ID_CDF] = {CDF_DEFAULT_TRACE_LEVEL, "CDF"},
	[CDF_MODULE_ID_SAP] = {CDF_DEFAULT_TRACE_LEVEL, "SAP"},
	[CDF_MODULE_ID_HDD_SOFTAP] = {CDF_DEFAULT_TRACE_LEVEL, "HSP"},
	[CDF_MODULE_ID_HDD_DATA] = {CDF_DEFAULT_TRACE_LEVEL, "HDP"},
	[CDF_MODULE_ID_HDD_SAP_DATA] = {CDF_DEFAULT_TRACE_LEVEL, "SDP"},
	[CDF_MODULE_ID_BMI] = {CDF_DEFAULT_TRACE_LEVEL, "BMI"},
	[CDF_MODULE_ID_HIF] = {CDF_DEFAULT_TRACE_LEVEL, "HIF"},
	[CDF_MODULE_ID_TXRX] = {CDF_DEFAULT_TRACE_LEVEL, "TRX"},
	[CDF_MODULE_ID_HTT] = {CDF_DEFAULT_TRACE_LEVEL, "HTT"},
};

/* Static and Global variables */
static spinlock_t ltrace_lock;

static cdf_trace_record_t g_cdf_trace_tbl[MAX_CDF_TRACE_RECORDS];
/* global cdf trace data */
static t_cdf_trace_data g_cdf_trace_data;
/*
 * all the call back functions for dumping MTRACE messages from ring buffer
 * are stored in cdf_trace_cb_table,these callbacks are initialized during init
 * only so, we will make a copy of these call back functions and maintain in to
 * cdf_trace_restore_cb_table. Incase if we make modifications to
 * cdf_trace_cb_table, we can certainly retrieve all the call back functions
 * back from Restore Table
 */
static tp_cdf_trace_cb cdf_trace_cb_table[CDF_MODULE_ID_MAX];
static tp_cdf_trace_cb cdf_trace_restore_cb_table[CDF_MODULE_ID_MAX];

/* Static and Global variables */
static spinlock_t l_dp_trace_lock;

static struct cdf_dp_trace_record_s
			g_cdf_dp_trace_tbl[MAX_CDF_DP_TRACE_RECORDS];

/*
 * all the options to configure/control DP trace are
 * defined in this structure
 */
static struct s_cdf_dp_trace_data g_cdf_dp_trace_data;
/*
 * all the call back functions for dumping DPTRACE messages from ring buffer
 * are stored in cdf_dp_trace_cb_table, callbacks are initialized during init
 */
static tp_cdf_dp_trace_cb cdf_dp_trace_cb_table[CDF_DP_TRACE_MAX];

/**
 * cdf_trace_set_level() - Set the trace level for a particular module
 * @level : trace level
 *
 * Trace level is a member of the CDF_TRACE_LEVEL enumeration indicating
 * the severity of the condition causing the trace message to be issued.
 * More severe conditions are more likely to be logged.
 *
 * This is an external API that allows trace levels to be set for each module.
 *
 * Return:  nothing
 */
void cdf_trace_set_level(CDF_MODULE_ID module, CDF_TRACE_LEVEL level)
{
	/* make sure the caller is passing in a valid LEVEL */
	if (level >= CDF_TRACE_LEVEL_MAX) {
		pr_err("%s: Invalid trace level %d passed in!\n", __func__,
		       level);
		return;
	}

	/* Treat 'none' differently.  NONE means we have to run off all
	 * the bits in the bit mask so none of the traces appear. Anything
	 * other than 'none' means we need to turn ON a bit in the bitmask
	 */
	if (CDF_TRACE_LEVEL_NONE == level)
		g_cdf_trace_info[module].moduleTraceLevel =
			CDF_TRACE_LEVEL_NONE;
	else
		/* set the desired bit in the bit mask for the module trace
		 * level */
		g_cdf_trace_info[module].moduleTraceLevel |=
			CDF_TRACE_LEVEL_TO_MODULE_BITMASK(level);
}

/**
 * cdf_trace_set_module_trace_level() - Set module trace level
 * @module: Module id
 * @level: Trace level for a module, as a bitmask as per 'moduleTraceInfo'
 *
 * Sets the module trace level where the trace level is given as a bit mask
 *
 * Return: None
 */
void cdf_trace_set_module_trace_level(CDF_MODULE_ID module, uint32_t level)
{
	if (module < 0 || module >= CDF_MODULE_ID_MAX) {
		pr_err("%s: Invalid module id %d passed\n", __func__, module);
		return;
	}
	g_cdf_trace_info[module].moduleTraceLevel = level;
}

void cdf_trace_set_value(CDF_MODULE_ID module, CDF_TRACE_LEVEL level,
			 uint8_t on)
{
	/* make sure the caller is passing in a valid LEVEL */
	if (level < 0 || level >= CDF_TRACE_LEVEL_MAX) {
		pr_err("%s: Invalid trace level %d passed in!\n", __func__,
		       level);
		return;
	}

	/* make sure the caller is passing in a valid module */
	if (module < 0 || module >= CDF_MODULE_ID_MAX) {
		pr_err("%s: Invalid module id %d passed in!\n", __func__,
		       module);
		return;
	}

	/* Treat 'none' differently.  NONE means we have to turn off all
	   the bits in the bit mask so none of the traces appear */
	if (CDF_TRACE_LEVEL_NONE == level) {
		g_cdf_trace_info[module].moduleTraceLevel =
			CDF_TRACE_LEVEL_NONE;
	}
	/* Treat 'All' differently.  All means we have to turn on all
	   the bits in the bit mask so all of the traces appear */
	else if (CDF_TRACE_LEVEL_ALL == level) {
		g_cdf_trace_info[module].moduleTraceLevel = 0xFFFF;
	} else {
		if (on)
			/* set the desired bit in the bit mask for the module
			   trace level */
			g_cdf_trace_info[module].moduleTraceLevel |=
				CDF_TRACE_LEVEL_TO_MODULE_BITMASK(level);
		else
			/* clear the desired bit in the bit mask for the module
			   trace level */
			g_cdf_trace_info[module].moduleTraceLevel &=
				~(CDF_TRACE_LEVEL_TO_MODULE_BITMASK(level));
	}
}

/**
 * cdf_trace_get_level() - get the trace level
 * @level : trace level
 *
 * This is an external API that returns a bool value to signify if a
 * particular trace level is set for the specified module.
 * A member of the CDF_TRACE_LEVEL enumeration indicating the severity
 * of the condition causing the trace message to be issued.
 *
 * Note that individual trace levels are the only valid values
 * for this API.  CDF_TRACE_LEVEL_NONE and CDF_TRACE_LEVEL_ALL
 * are not valid input and will return false
 *
 * Return:
 *      false - the specified trace level for the specified module is OFF
 *      true - the specified trace level for the specified module is ON
 */
bool cdf_trace_get_level(CDF_MODULE_ID module, CDF_TRACE_LEVEL level)
{
	bool traceOn = false;

	if ((CDF_TRACE_LEVEL_NONE == level) ||
	    (CDF_TRACE_LEVEL_ALL == level) || (level >= CDF_TRACE_LEVEL_MAX)) {
		traceOn = false;
	} else {
		traceOn = (level & g_cdf_trace_info[module].moduleTraceLevel)
			  ? true : false;
	}

	return traceOn;
}

void cdf_snprintf(char *strBuffer, unsigned int size, char *strFormat, ...)
{
	va_list val;

	va_start(val, strFormat);
	snprintf(strBuffer, size, strFormat, val);
	va_end(val);
}

#ifdef CDF_ENABLE_TRACING

/**
 * cdf_trace_msg() - externally called trace function
 * @module : Module identifier a member of the CDF_MODULE_ID
 *	enumeration that identifies the module issuing the trace message.
 * @level : Trace level a member of the CDF_TRACE_LEVEL enumeration
 *	indicating the severity of the condition causing the trace message
 *	to be issued.   More severe conditions are more likely to be logged.
 * @strFormat : Format string  in which the message to be logged.  This format
 *	string contains printf-like replacement parameters, which follow
 *	this parameter in the variable argument list.
 *
 *  Checks the level of severity and accordingly prints the trace messages
 *
 *  Return:  nothing
 *
 */
void cdf_trace_msg(CDF_MODULE_ID module, CDF_TRACE_LEVEL level,
		   char *strFormat, ...)
{
	char strBuffer[CDF_TRACE_BUFFER_SIZE];
	int n;

	/* Print the trace message when the desired level bit is set in
	   the module tracel level mask */
	if (g_cdf_trace_info[module].moduleTraceLevel &
	    CDF_TRACE_LEVEL_TO_MODULE_BITMASK(level)) {
		/* the trace level strings in an array.  these are ordered in
		 * the same order as the trace levels are defined in the enum
		 * (see CDF_TRACE_LEVEL) so we can index into this array with
		 * the level and get the right string. The cdf trace levels
		 * are... none, Fatal, Error, Warning, Info, InfoHigh, InfoMed,
		 * InfoLow, Debug
		 */
		static const char *TRACE_LEVEL_STR[] = { "  ", "F ", "E ", "W ",
						"I ", "IH", "IM", "IL", "D" };
		va_list val;
		va_start(val, strFormat);

		/* print the prefix string into the string buffer... */
		n = snprintf(strBuffer, CDF_TRACE_BUFFER_SIZE,
			     "wlan: [%d:%2s:%3s] ",
			     in_interrupt() ? 0 : current->pid,
			     (char *)TRACE_LEVEL_STR[level],
			     (char *)g_cdf_trace_info[module].moduleNameStr);

		/* print the formatted log message after the prefix string */
		if ((n >= 0) && (n < CDF_TRACE_BUFFER_SIZE)) {
			vsnprintf(strBuffer + n, CDF_TRACE_BUFFER_SIZE - n,
				  strFormat, val);
#if defined(WLAN_LOGGING_SOCK_SVC_ENABLE)
			wlan_log_to_user(level, (char *)strBuffer,
					 strlen(strBuffer));
#else
			pr_err("%s\n", strBuffer);
#endif
		}
		va_end(val);
	}
}

void cdf_trace_display(void)
{
	CDF_MODULE_ID moduleId;

	pr_err
		("     1)FATAL  2)ERROR  3)WARN  4)INFO  5)INFO_H  6)INFO_M  7)INFO_L 8)DEBUG\n");
	for (moduleId = 0; moduleId < CDF_MODULE_ID_MAX; ++moduleId) {
		pr_err
			("%2d)%s    %s        %s       %s       %s        %s         %s         %s        %s\n",
			(int)moduleId, g_cdf_trace_info[moduleId].moduleNameStr,
			(g_cdf_trace_info[moduleId].
			 moduleTraceLevel & (1 << CDF_TRACE_LEVEL_FATAL)) ? "X" :
			" ",
			(g_cdf_trace_info[moduleId].
			 moduleTraceLevel & (1 << CDF_TRACE_LEVEL_ERROR)) ? "X" :
			" ",
			(g_cdf_trace_info[moduleId].
			 moduleTraceLevel & (1 << CDF_TRACE_LEVEL_WARN)) ? "X" :
			" ",
			(g_cdf_trace_info[moduleId].
			 moduleTraceLevel & (1 << CDF_TRACE_LEVEL_INFO)) ? "X" :
			" ",
			(g_cdf_trace_info[moduleId].
			 moduleTraceLevel & (1 << CDF_TRACE_LEVEL_INFO_HIGH)) ? "X"
			: " ",
			(g_cdf_trace_info[moduleId].
			 moduleTraceLevel & (1 << CDF_TRACE_LEVEL_INFO_MED)) ? "X"
			: " ",
			(g_cdf_trace_info[moduleId].
			 moduleTraceLevel & (1 << CDF_TRACE_LEVEL_INFO_LOW)) ? "X"
			: " ",
			(g_cdf_trace_info[moduleId].
			 moduleTraceLevel & (1 << CDF_TRACE_LEVEL_DEBUG)) ? "X" :
			" ");
	}
}

#define ROW_SIZE 16
/* Buffer size = data bytes(2 hex chars plus space) + NULL */
#define BUFFER_SIZE ((ROW_SIZE * 3) + 1)

/**
 * cdf_trace_hex_dump() - externally called hex dump function
 * @module : Module identifier a member of the CDF_MODULE_ID enumeration that
 *	     identifies the module issuing the trace message.
 * @level : Trace level a member of the CDF_TRACE_LEVEL enumeration indicating
 *	    the severity of the condition causing the trace message to be
 *	    issued. More severe conditions are more likely to be logged.
 * @data : The base address of the buffer to be logged.
 * @buf_len : The size of the buffer to be logged.
 *
 *  Checks the level of severity and accordingly prints the trace messages
 *
 *  Return :  nothing
 */
void cdf_trace_hex_dump(CDF_MODULE_ID module, CDF_TRACE_LEVEL level,
			void *data, int buf_len)
{
	const u8 *ptr = data;
	int i, linelen, remaining = buf_len;
	unsigned char linebuf[BUFFER_SIZE];

	if (!(g_cdf_trace_info[module].moduleTraceLevel &
		CDF_TRACE_LEVEL_TO_MODULE_BITMASK(level)))
		return;

	for (i = 0; i < buf_len; i += ROW_SIZE) {
		linelen = min(remaining, ROW_SIZE);
		remaining -= ROW_SIZE;

		hex_dump_to_buffer(ptr + i, linelen, ROW_SIZE, 1,
				linebuf, sizeof(linebuf), false);

		cdf_trace_msg(module, level, "%.8x: %s", i, linebuf);
	}
}

#endif

/**
 * cdf_trace_enable() - Enable MTRACE for specific modules
 * @bitmask_of_moduleId : Bitmask according to enum of the modules.
 *  32 [dec]  = 0010 0000 [bin] <enum of HDD is 5>
 *  64 [dec]  = 0100 0000 [bin] <enum of SME is 6>
 *  128 [dec] = 1000 0000 [bin] <enum of PE is 7>
 * @enable : can be true or false true implies enabling MTRACE false implies
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
 *
 * Return : nothing
 */
void cdf_trace_enable(uint32_t bitmask_of_moduleId, uint8_t enable)
{
	int i;
	if (bitmask_of_moduleId) {
		for (i = 0; i < CDF_MODULE_ID_MAX; i++) {
			if (((bitmask_of_moduleId >> i) & 1)) {
				if (enable) {
					if (NULL !=
					    cdf_trace_restore_cb_table[i]) {
						cdf_trace_cb_table[i] =
						cdf_trace_restore_cb_table[i];
					}
				} else {
					cdf_trace_restore_cb_table[i] =
						cdf_trace_cb_table[i];
					cdf_trace_cb_table[i] = NULL;
				}
			}
		}
	} else {
		if (enable) {
			for (i = 0; i < CDF_MODULE_ID_MAX; i++) {
				if (NULL != cdf_trace_restore_cb_table[i]) {
					cdf_trace_cb_table[i] =
						cdf_trace_restore_cb_table[i];
				}
			}
		} else {
			for (i = 0; i < CDF_MODULE_ID_MAX; i++) {
				cdf_trace_restore_cb_table[i] =
					cdf_trace_cb_table[i];
				cdf_trace_cb_table[i] = NULL;
			}
		}
	}
}

/**
 * cdf_trace_init() - initializes cdf trace structures and variables
 *
 * Called immediately after cds_preopen, so that we can start recording HDD
 * events ASAP.
 *
 * Return : nothing
 */
void cdf_trace_init(void)
{
	uint8_t i;
	g_cdf_trace_data.head = INVALID_CDF_TRACE_ADDR;
	g_cdf_trace_data.tail = INVALID_CDF_TRACE_ADDR;
	g_cdf_trace_data.num = 0;
	g_cdf_trace_data.enable = true;
	g_cdf_trace_data.dumpCount = DEFAULT_CDF_TRACE_DUMP_COUNT;
	g_cdf_trace_data.numSinceLastDump = 0;

	for (i = 0; i < CDF_MODULE_ID_MAX; i++) {
		cdf_trace_cb_table[i] = NULL;
		cdf_trace_restore_cb_table[i] = NULL;
	}
}

/**
 * cdf_trace() - puts the messages in to ring-buffer
 * @module : Enum of module, basically module id.
 * @param : Code to be recorded
 * @session : Session ID of the log
 * @data : Actual message contents
 *
 * This function will be called from each module who wants record the messages
 * in circular queue. Before calling this functions make sure you have
 * registered your module with cdf through cdf_trace_register function.
 *
 *
 * Return : nothing
 */
void cdf_trace(uint8_t module, uint8_t code, uint16_t session, uint32_t data)
{
	tp_cdf_trace_record rec = NULL;
	unsigned long flags;

	if (!g_cdf_trace_data.enable)
		return;

	/* if module is not registered, don't record for that module */
	if (NULL == cdf_trace_cb_table[module])
		return;

	/* Aquire the lock so that only one thread at a time can fill the ring
	 * buffer
	 */
	spin_lock_irqsave(&ltrace_lock, flags);

	g_cdf_trace_data.num++;

	if (g_cdf_trace_data.num > MAX_CDF_TRACE_RECORDS)
		g_cdf_trace_data.num = MAX_CDF_TRACE_RECORDS;

	if (INVALID_CDF_TRACE_ADDR == g_cdf_trace_data.head) {
		/* first record */
		g_cdf_trace_data.head = 0;
		g_cdf_trace_data.tail = 0;
	} else {
		/* queue is not empty */
		uint32_t tail = g_cdf_trace_data.tail + 1;

		if (MAX_CDF_TRACE_RECORDS == tail)
			tail = 0;

		if (g_cdf_trace_data.head == tail) {
			/* full */
			if (MAX_CDF_TRACE_RECORDS == ++g_cdf_trace_data.head)
				g_cdf_trace_data.head = 0;
		}
		g_cdf_trace_data.tail = tail;
	}

	rec = &g_cdf_trace_tbl[g_cdf_trace_data.tail];
	rec->code = code;
	rec->session = session;
	rec->data = data;
	rec->time = cdf_get_log_timestamp();
	rec->module = module;
	rec->pid = (in_interrupt() ? 0 : current->pid);
	g_cdf_trace_data.numSinceLastDump++;
	spin_unlock_irqrestore(&ltrace_lock, flags);
}

/**
 * cdf_trace_spin_lock_init() - initializes the lock variable before use
 *
 * This function will be called from cds_alloc_global_context, we will have lock
 * available to use ASAP
 *
 * Return : nothing
 */
CDF_STATUS cdf_trace_spin_lock_init(void)
{
	spin_lock_init(&ltrace_lock);

	return CDF_STATUS_SUCCESS;
}

/**
 * cdf_trace_register() - registers the call back functions
 * @moduleID - enum value of module
 * @cdf_trace_callback - call back functions to display the messages in
 *  particular format.
 *
 * Registers the call back functions to display the messages in particular
 * format mentioned in these call back functions. This functions should be
 * called by interested module in their init part as we will be ready to
 * register as soon as modules are up.
 *
 * Return : nothing
 */
void cdf_trace_register(CDF_MODULE_ID moduleID,
			tp_cdf_trace_cb cdf_trace_callback)
{
	cdf_trace_cb_table[moduleID] = cdf_trace_callback;
}

/**
 * cdf_trace_dump_all() - Dump data from ring buffer via call back functions
 *			  registered with CDF
 * @pMac : Context of particular module
 * @code : Reason code
 * @session : Session id of log
 * @count : Number of lines to dump starting from tail to head
 *
 * This function will be called up on issueing ioctl call as mentioned following
 * [iwpriv wlan0 dumplog 0 0 <n> <bitmask_of_module>]
 *
 *  <n> - number lines to dump starting from tail to head.
 *
 *  <bitmask_of_module> - if anybody wants to know how many messages were
 *  recorded for particular module/s mentioned by setbit in bitmask from last
 *  <n> messages. It is optional, if you don't provide then it will dump
 *  everything from buffer.
 *
 * Return : nothing
 */
void cdf_trace_dump_all(void *pMac, uint8_t code, uint8_t session,
			uint32_t count, uint32_t bitmask_of_module)
{
	cdf_trace_record_t pRecord;
	int32_t i, tail;

	if (!g_cdf_trace_data.enable) {
		CDF_TRACE(CDF_MODULE_ID_SYS,
			  CDF_TRACE_LEVEL_ERROR, "Tracing Disabled");
		return;
	}

	CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_INFO,
		  "Total Records: %d, Head: %d, Tail: %d",
		  g_cdf_trace_data.num, g_cdf_trace_data.head,
		  g_cdf_trace_data.tail);

	/* aquire the lock so that only one thread at a time can read
	 * the ring buffer
	 */
	spin_lock(&ltrace_lock);

	if (g_cdf_trace_data.head != INVALID_CDF_TRACE_ADDR) {
		i = g_cdf_trace_data.head;
		tail = g_cdf_trace_data.tail;

		if (count) {
			if (count > g_cdf_trace_data.num)
				count = g_cdf_trace_data.num;
			if (tail >= (count - 1))
				i = tail - count + 1;
			else if (count != MAX_CDF_TRACE_RECORDS)
				i = MAX_CDF_TRACE_RECORDS - ((count - 1) -
							     tail);
		}

		pRecord = g_cdf_trace_tbl[i];
		/* right now we are not using numSinceLastDump member but
		 * in future we might re-visit and use this member to track
		 * how many latest messages got added while we were dumping
		 * from ring buffer
		 */
		g_cdf_trace_data.numSinceLastDump = 0;
		spin_unlock(&ltrace_lock);
		for (;; ) {
			if ((code == 0 || (code == pRecord.code)) &&
			    (cdf_trace_cb_table[pRecord.module] != NULL)) {
				if (0 == bitmask_of_module) {
					cdf_trace_cb_table[pRecord.
							   module] (pMac,
								    &pRecord,
								    (uint16_t)
								    i);
				} else {
					if (bitmask_of_module &
					    (1 << pRecord.module)) {
						cdf_trace_cb_table[pRecord.
								   module]
							(pMac, &pRecord,
							(uint16_t) i);
					}
				}
			}

			if (i == tail)
				break;
			i += 1;

			spin_lock(&ltrace_lock);
			if (MAX_CDF_TRACE_RECORDS == i) {
				i = 0;
				pRecord = g_cdf_trace_tbl[0];
			} else {
				pRecord = g_cdf_trace_tbl[i];
			}
			spin_unlock(&ltrace_lock);
		}
	} else {
		spin_unlock(&ltrace_lock);
	}
}

/**
 * cdf_dp_trace_init() - enables the DP trace
 * Called during driver load and it enables DP trace
 *
 * Return: None
 */
void cdf_dp_trace_init(void)
{
	uint8_t i;

	cdf_dp_trace_spin_lock_init();
	g_cdf_dp_trace_data.head = INVALID_CDF_DP_TRACE_ADDR;
	g_cdf_dp_trace_data.tail = INVALID_CDF_DP_TRACE_ADDR;
	g_cdf_dp_trace_data.num = 0;
	g_cdf_dp_trace_data.proto_bitmap = 0;
	g_cdf_dp_trace_data.no_of_record = 0;
	g_cdf_dp_trace_data.verbosity    = CDF_DP_TRACE_VERBOSITY_DEFAULT;
	g_cdf_dp_trace_data.enable = true;

	for (i = 0; i < CDF_DP_TRACE_MAX; i++)
		cdf_dp_trace_cb_table[i] = cdf_dp_display_record;
}

/**
 * cdf_dp_trace_set_value() - Configure the value to control DP trace
 * @proto_bitmap  : defines the protocol to be tracked
 * @no_of_records : defines the nth packet which is traced
 * @verbosity     : defines the verbosity level
 *
 * Return: None
 */
void cdf_dp_trace_set_value(uint8_t proto_bitmap, uint8_t no_of_record,
			 uint8_t verbosity)
{
	g_cdf_dp_trace_data.proto_bitmap = proto_bitmap;
	g_cdf_dp_trace_data.no_of_record = no_of_record;
	g_cdf_dp_trace_data.verbosity    = verbosity;
	return;
}

/**
 * cdf_dp_trace_enable_track() - enable the tracing for netbuf
 * @code : defines the event
 *
 * Return: true or false depends on whether tracing enabled
 */
static bool cdf_dp_trace_enable_track(enum CDF_DP_TRACE_ID code)
{
	if (g_cdf_dp_trace_data.verbosity == CDF_DP_TRACE_VERBOSITY_HIGH)
		return true;
	if (g_cdf_dp_trace_data.verbosity == CDF_DP_TRACE_VERBOSITY_MEDIUM
		&& (code <= CDF_DP_TRACE_HIF_PACKET_PTR_RECORD))
		return true;
	if (g_cdf_dp_trace_data.verbosity == CDF_DP_TRACE_VERBOSITY_LOW
		&& (code <= CDF_DP_TRACE_CE_PACKET_RECORD))
		return true;
	if (g_cdf_dp_trace_data.verbosity == CDF_DP_TRACE_VERBOSITY_DEFAULT
		&& (code == CDF_DP_TRACE_DROP_PACKET_RECORD))
		return true;
	return false;
}

/**
 * cdf_dp_trace_set_track() - Marks whether the packet needs to be traced
 * @nbuf  : defines the netbuf
 *
 * Return: None
 */
void cdf_dp_trace_set_track(cdf_nbuf_t nbuf)
{
	spin_lock_bh(&l_dp_trace_lock);
	g_cdf_dp_trace_data.count++;
	if (g_cdf_dp_trace_data.proto_bitmap != 0) {
		if (cds_pkt_get_proto_type(nbuf,
			g_cdf_dp_trace_data.proto_bitmap, 0)) {
			CDF_NBUF_SET_DP_TRACE(nbuf, 1);
		}
	}
	if ((g_cdf_dp_trace_data.no_of_record != 0) &&
		(g_cdf_dp_trace_data.count %
			g_cdf_dp_trace_data.no_of_record == 0)) {
		CDF_NBUF_SET_DP_TRACE(nbuf, 1);
	}
	spin_unlock_bh(&l_dp_trace_lock);
	return;
}

/**
 * dump_hex_trace() - Display the data in buffer
 * @buf:     buffer which contains data to be displayed
 * @buf_len: defines the size of the data to be displayed
 *
 * Return: None
 */
static void dump_hex_trace(uint8_t *buf, uint8_t buf_len)
{
	uint8_t i = 0;
	/* Dump the bytes in the last line */
	cdf_print("DATA: ");
	for (i = 0; i < buf_len; i++)
		cdf_print("%02x ", buf[i]);
	cdf_print("\n");
}

/**
 * cdf_dp_display_trace() - Displays a record in DP trace
 * @pRecord  : pointer to a record in DP trace
 * @recIndex : record index
 *
 * Return: None
 */
void cdf_dp_display_record(struct cdf_dp_trace_record_s *pRecord ,
				uint16_t recIndex)
{
	cdf_print("INDEX: %04d TIME: %012llu CODE: %02d\n", recIndex,
						pRecord->time, pRecord->code);
	switch (pRecord->code) {
	case  CDF_DP_TRACE_HDD_TX_TIMEOUT:
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
						"HDD TX Timeout\n");
		break;
	case  CDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT:
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
						"HDD SoftAP TX Timeout\n");
		break;
	case  CDF_DP_TRACE_VDEV_PAUSE:
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
						"VDEV Pause\n");
		break;
	case  CDF_DP_TRACE_VDEV_UNPAUSE:
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
						"VDEV UnPause\n");
		break;
	default:
		dump_hex_trace(pRecord->data, pRecord->size);
	}
	return;
}

/**
 * cdf_dp_trace() - Stores the data in buffer
 * @nbuf  : defines the netbuf
 * @code : defines the event
 * @data : defines the data to be stored
 * @size : defines the size of the data record
 *
 * Return: None
 */
void cdf_dp_trace(cdf_nbuf_t nbuf, enum CDF_DP_TRACE_ID code,
			uint8_t *data, uint8_t size)
{
	struct cdf_dp_trace_record_s *rec = NULL;

	/* Return when Dp trace is not enabled */
	if (!g_cdf_dp_trace_data.enable)
		return;

	/* If nbuf is NULL, check for VDEV PAUSE, UNPAUSE, TIMEOUT */
	if (!nbuf) {
		switch (code) {
		case CDF_DP_TRACE_HDD_TX_TIMEOUT:
		case CDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT:
		case CDF_DP_TRACE_VDEV_PAUSE:
		case CDF_DP_TRACE_VDEV_UNPAUSE:
			if (cdf_dp_trace_enable_track(code))
				goto  register_record;
			else
				return;

		default:
			return;
		}
	}

	/* Return when the packet is not a data packet */
	if (NBUF_GET_PACKET_TRACK(nbuf) != NBUF_TX_PKT_DATA_TRACK)
		return;

	/* Return when nbuf is not marked for dp tracing or
	 * verbosity does not allow
	 */
	if (cdf_dp_trace_enable_track(code) == false ||
			!CDF_NBUF_GET_DP_TRACE(nbuf))
		return;

	/* Acquire the lock so that only one thread at a time can fill the ring
	 * buffer
	 */

register_record:

	spin_lock_bh(&l_dp_trace_lock);

	g_cdf_dp_trace_data.num++;

	if (g_cdf_dp_trace_data.num > MAX_CDF_DP_TRACE_RECORDS)
		g_cdf_dp_trace_data.num = MAX_CDF_DP_TRACE_RECORDS;

	if (INVALID_CDF_DP_TRACE_ADDR == g_cdf_dp_trace_data.head) {
		/* first record */
		g_cdf_dp_trace_data.head = 0;
		g_cdf_dp_trace_data.tail = 0;
	} else {
		/* queue is not empty */
		g_cdf_dp_trace_data.tail++;

		if (MAX_CDF_DP_TRACE_RECORDS == g_cdf_dp_trace_data.tail)
			g_cdf_dp_trace_data.tail = 0;

		if (g_cdf_dp_trace_data.head == g_cdf_dp_trace_data.tail) {
			/* full */
			if (MAX_CDF_DP_TRACE_RECORDS ==
				++g_cdf_dp_trace_data.head)
				g_cdf_dp_trace_data.head = 0;
		}
	}

	rec = &g_cdf_dp_trace_tbl[g_cdf_dp_trace_data.tail];
	rec->code = code;
	rec->size = 0;
	if (data != NULL && size > 0) {
		if (size > CDF_DP_TRACE_RECORD_SIZE)
			size = CDF_DP_TRACE_RECORD_SIZE;

		rec->size = size;
		switch (code) {
		case CDF_DP_TRACE_HDD_PACKET_PTR_RECORD:
		case CDF_DP_TRACE_CE_PACKET_PTR_RECORD:
		case CDF_DP_TRACE_TXRX_QUEUE_PACKET_PTR_RECORD:
		case CDF_DP_TRACE_TXRX_PACKET_PTR_RECORD:
		case CDF_DP_TRACE_HTT_PACKET_PTR_RECORD:
		case CDF_DP_TRACE_HTC_PACKET_PTR_RECORD:
		case CDF_DP_TRACE_HIF_PACKET_PTR_RECORD:
			cdf_mem_copy(rec->data, (uint8_t *)(&data), size);
			break;

		case CDF_DP_TRACE_DROP_PACKET_RECORD:
		case CDF_DP_TRACE_HDD_PACKET_RECORD:
		case CDF_DP_TRACE_CE_PACKET_RECORD:
			cdf_mem_copy(rec->data, data, size);
			break;
		default:
			break;
		}
	}
	rec->time = cdf_get_log_timestamp();
	rec->pid = (in_interrupt() ? 0 : current->pid);
	spin_unlock_bh(&l_dp_trace_lock);
}

/**
 * cdf_dp_trace_spin_lock_init() - initializes the lock variable before use
 * This function will be called from cds_alloc_global_context, we will have lock
 * available to use ASAP
 *
 * Return : nothing
 */
void cdf_dp_trace_spin_lock_init(void)
{
	spin_lock_init(&l_dp_trace_lock);

	return;
}

/**
 * cdf_dp_trace_dump_all() - Dump data from ring buffer via call back functions
 *			  registered with CDF
 * @code : Reason code
 * @count : Number of lines to dump starting from tail to head
 *
 * Return : nothing
 */
void cdf_dp_trace_dump_all(uint32_t count)
{
	struct cdf_dp_trace_record_s pRecord;
	int32_t i, tail;

	if (!g_cdf_dp_trace_data.enable) {
		CDF_TRACE(CDF_MODULE_ID_SYS,
			  CDF_TRACE_LEVEL_ERROR, "Tracing Disabled");
		return;
	}

	CDF_TRACE(CDF_MODULE_ID_SYS, CDF_TRACE_LEVEL_ERROR,
		  "Total Records: %d, Head: %d, Tail: %d",
		  g_cdf_dp_trace_data.num, g_cdf_dp_trace_data.head,
		  g_cdf_dp_trace_data.tail);

	/* aquire the lock so that only one thread at a time can read
	 * the ring buffer
	 */
	spin_lock_bh(&l_dp_trace_lock);

	if (g_cdf_dp_trace_data.head != INVALID_CDF_DP_TRACE_ADDR) {
		i = g_cdf_dp_trace_data.head;
		tail = g_cdf_dp_trace_data.tail;

		if (count) {
			if (count > g_cdf_dp_trace_data.num)
				count = g_cdf_dp_trace_data.num;
			if (tail >= (count - 1))
				i = tail - count + 1;
			else if (count != MAX_CDF_DP_TRACE_RECORDS)
				i = MAX_CDF_DP_TRACE_RECORDS - ((count - 1) -
							     tail);
		}

		pRecord = g_cdf_dp_trace_tbl[i];
		spin_unlock_bh(&l_dp_trace_lock);
		for (;; ) {

			cdf_dp_trace_cb_table[pRecord.
					   code] (&pRecord, (uint16_t)i);
			if (i == tail)
				break;
			i += 1;

			spin_lock_bh(&l_dp_trace_lock);
			if (MAX_CDF_DP_TRACE_RECORDS == i)
				i = 0;

			pRecord = g_cdf_dp_trace_tbl[i];
			spin_unlock_bh(&l_dp_trace_lock);
		}
	} else {
		spin_unlock_bh(&l_dp_trace_lock);
	}
}
