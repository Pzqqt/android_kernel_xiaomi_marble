/*
 * Copyright (c) 2014-2020 The Linux Foundation. All rights reserved.
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
 *  DOC:  qdf_trace
 *  QCA driver framework (QDF) trace APIs
 *  Trace, logging, and debugging definitions and APIs
 */

/* Include Files */
#include "qdf_str.h"
#include <qdf_trace.h>
#include <qdf_module.h>

/* macro to map qdf trace levels into the bitmask */
#define QDF_TRACE_LEVEL_TO_MODULE_BITMASK(_level) ((1 << (_level)))

#include <wlan_logging_sock_svc.h>
#include <qdf_module.h>
static int qdf_pidx = -1;
static bool qdf_log_dump_at_kernel_enable = true;
qdf_declare_param(qdf_log_dump_at_kernel_enable, bool);

/* This value of 0 will disable the timer by default. */
static uint32_t qdf_log_flush_timer_period;
qdf_declare_param(qdf_log_flush_timer_period, uint);

#include "qdf_time.h"
#include "qdf_mc_timer.h"
#include <host_diag_core_log.h>

/* Global qdf print id */

/* Preprocessor definitions and constants */

enum qdf_timestamp_unit qdf_log_timestamp_type = QDF_LOG_TIMESTAMP_UNIT;

#define DP_TRACE_META_DATA_STRLEN 50

#ifdef TRACE_RECORD
/* Static and Global variables */
static spinlock_t ltrace_lock;
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

#ifdef WLAN_LOGGING_BUFFERS_DYNAMICALLY
static qdf_trace_record_t *g_qdf_trace_tbl;
#else
static qdf_trace_record_t g_qdf_trace_tbl[MAX_QDF_TRACE_RECORDS];
#endif

#endif

#ifdef WLAN_FEATURE_MEMDUMP_ENABLE
static tp_qdf_state_info_cb qdf_state_info_table[QDF_MODULE_ID_MAX];
#endif

#ifdef CONFIG_DP_TRACE
/* Static and Global variables */
#ifdef WLAN_LOGGING_BUFFERS_DYNAMICALLY
static struct qdf_dp_trace_record_s *g_qdf_dp_trace_tbl;
#else
static struct qdf_dp_trace_record_s
			g_qdf_dp_trace_tbl[MAX_QDF_DP_TRACE_RECORDS];
#endif
static spinlock_t l_dp_trace_lock;

/*
 * all the options to configure/control DP trace are
 * defined in this structure
 */
static struct s_qdf_dp_trace_data g_qdf_dp_trace_data;
/*
 * all the call back functions for dumping DPTRACE messages from ring buffer
 * are stored in qdf_dp_trace_cb_table, callbacks are initialized during init
 */
static tp_qdf_dp_trace_cb qdf_dp_trace_cb_table[QDF_DP_TRACE_MAX + 1];
#endif

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
	snprintf(str_buffer, size, str_format);
}
qdf_export_symbol(qdf_snprintf);

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
		   const char *str_format, ...)
{
	va_list val;

	va_start(val, str_format);
	qdf_trace_msg_cmn(qdf_pidx, module, level, str_format, val);
	va_end(val);
}
qdf_export_symbol(qdf_trace_msg);

void qdf_vtrace_msg(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
		    const char *str_format, va_list val)
{
	qdf_trace_msg_cmn(qdf_pidx, module, level, str_format, val);
}
qdf_export_symbol(qdf_vtrace_msg);

#define ROW_SIZE 16
/* Buffer size = data bytes(2 hex chars plus space) + NULL */
#define BUFFER_SIZE ((QDF_DP_TRACE_RECORD_SIZE * 3) + 1)

static void __qdf_trace_hex_dump(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
				 void *data, int buf_len, bool print_ascii)
{
	const u8 *ptr = data;
	int i = 0;

	if (!qdf_print_is_verbose_enabled(qdf_pidx, module, level))
		return;

	while (buf_len > 0) {
		unsigned char linebuf[BUFFER_SIZE] = {0};
		int linelen = min(buf_len, ROW_SIZE);

		buf_len -= ROW_SIZE;

		hex_dump_to_buffer(ptr, linelen, ROW_SIZE, 1,
				   linebuf, sizeof(linebuf), print_ascii);

		qdf_trace_msg(module, level, "%.8x: %s", i, linebuf);
		ptr += ROW_SIZE;
		i += ROW_SIZE;
	}
}

void qdf_trace_hex_dump(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
			void *data, int buf_len)
{
	__qdf_trace_hex_dump(module, level, data, buf_len, false);
}

qdf_export_symbol(qdf_trace_hex_dump);

void qdf_trace_hex_ascii_dump(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
			      void *data, int buf_len)
{
	__qdf_trace_hex_dump(module, level, data, buf_len, true);
}

qdf_export_symbol(qdf_trace_hex_ascii_dump);

#endif

#ifdef TRACE_RECORD

#ifdef WLAN_LOGGING_BUFFERS_DYNAMICALLY
static inline QDF_STATUS allocate_g_qdf_trace_tbl_buffer(void)
{
	g_qdf_trace_tbl = vzalloc(MAX_QDF_TRACE_RECORDS *
				  sizeof(*g_qdf_trace_tbl));
	QDF_BUG(g_qdf_trace_tbl);
	return g_qdf_trace_tbl ? QDF_STATUS_SUCCESS : QDF_STATUS_E_NOMEM;
}

static inline void free_g_qdf_trace_tbl_buffer(void)
{
	vfree(g_qdf_trace_tbl);
	g_qdf_trace_tbl = NULL;
}
#else
static inline QDF_STATUS allocate_g_qdf_trace_tbl_buffer(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline void free_g_qdf_trace_tbl_buffer(void)
{ }
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
				if (qdf_trace_restore_cb_table[i]) {
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
qdf_export_symbol(qdf_trace_enable);

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

	if (allocate_g_qdf_trace_tbl_buffer() != QDF_STATUS_SUCCESS)
		return;
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
qdf_export_symbol(qdf_trace_init);

/**
 * qdf_trace_deinit() - frees memory allocated dynamically
 *
 * Called from cds_deinit, so that we can free the memory and resets
 * the variables
 *
 * Return: None
 */
void qdf_trace_deinit(void)
{
	g_qdf_trace_data.enable = false;
	g_qdf_trace_data.num = 0;
	g_qdf_trace_data.head = INVALID_QDF_TRACE_ADDR;
	g_qdf_trace_data.tail = INVALID_QDF_TRACE_ADDR;

	free_g_qdf_trace_tbl_buffer();
}

qdf_export_symbol(qdf_trace_deinit);

/**
 * qdf_trace() - puts the messages in to ring-buffer
 * @module: Enum of module, basically module id.
 * @code: Code to be recorded
 * @session: Session ID of the log
 * @data: Actual message contents
 *
 * This function will be called from each module who wants record the messages
 * in circular queue. Before calling this functions make sure you have
 * registered your module with qdf through qdf_trace_register function.
 *
 * Return: None
 */
void qdf_trace(uint8_t module, uint16_t code, uint16_t session, uint32_t data)
{
	tp_qdf_trace_record rec = NULL;
	unsigned long flags;
	char time[18];

	if (!g_qdf_trace_data.enable)
		return;

	/* if module is not registered, don't record for that module */
	if (!qdf_trace_cb_table[module])
		return;

	qdf_get_time_of_the_day_in_hr_min_sec_usec(time, sizeof(time));
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
	rec->qtime = qdf_get_log_timestamp();
	scnprintf(rec->time, sizeof(rec->time), "%s", time);
	rec->module = module;
	rec->pid = (in_interrupt() ? 0 : current->pid);
	g_qdf_trace_data.num_since_last_dump++;
	spin_unlock_irqrestore(&ltrace_lock, flags);
}
qdf_export_symbol(qdf_trace);

#ifdef ENABLE_MTRACE_LOG
void qdf_mtrace_log(QDF_MODULE_ID src_module, QDF_MODULE_ID dst_module,
		    uint16_t message_id, uint8_t vdev_id)
{
	uint32_t trace_log, payload;
	static uint16_t counter;

	trace_log = (src_module << 23) | (dst_module << 15) | message_id;
	payload = (vdev_id << 16) | counter++;

	QDF_TRACE(src_module, QDF_TRACE_LEVEL_TRACE, "%x %x",
		  trace_log, payload);
}

qdf_export_symbol(qdf_mtrace_log);
#endif

void qdf_mtrace(QDF_MODULE_ID src_module, QDF_MODULE_ID dst_module,
		uint16_t message_id, uint8_t vdev_id, uint32_t data)
{
	qdf_trace(src_module, message_id, vdev_id, data);
	qdf_mtrace_log(src_module, dst_module, message_id, vdev_id);
}

qdf_export_symbol(qdf_mtrace);

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
qdf_export_symbol(qdf_trace_spin_lock_init);

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
void qdf_trace_register(QDF_MODULE_ID module_id,
			tp_qdf_trace_cb qdf_trace_callback)
{
	qdf_trace_cb_table[module_id] = qdf_trace_callback;
}
qdf_export_symbol(qdf_trace_register);

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
		  "DPT: Total Records: %d, Head: %d, Tail: %d",
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
			    (qdf_trace_cb_table[p_record.module])) {
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
qdf_export_symbol(qdf_trace_dump_all);
#endif

#ifdef WLAN_FEATURE_MEMDUMP_ENABLE
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
qdf_export_symbol(qdf_register_debugcb_init);

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
qdf_export_symbol(qdf_register_debug_callback);

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
		if (qdf_state_info_table[module]) {
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
qdf_export_symbol(qdf_state_info_dump_all);
#endif

#ifdef CONFIG_DP_TRACE

#ifdef WLAN_LOGGING_BUFFERS_DYNAMICALLY
static inline QDF_STATUS allocate_g_qdf_dp_trace_tbl_buffer(void)
{
	g_qdf_dp_trace_tbl = vzalloc(MAX_QDF_DP_TRACE_RECORDS *
				     sizeof(*g_qdf_dp_trace_tbl));
	QDF_BUG(g_qdf_dp_trace_tbl);
	return g_qdf_dp_trace_tbl ? QDF_STATUS_SUCCESS : QDF_STATUS_E_NOMEM;
}

static inline void free_g_qdf_dp_trace_tbl_buffer(void)
{
	vfree(g_qdf_dp_trace_tbl);
	g_qdf_dp_trace_tbl = NULL;
}
#else
static inline QDF_STATUS allocate_g_qdf_dp_trace_tbl_buffer(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline void free_g_qdf_dp_trace_tbl_buffer(void)
{ }
#endif

#define QDF_DP_TRACE_PREPEND_STR_SIZE 100
/*
 * one dp trace record can't be greater than 300 bytes.
 * Max Size will be QDF_DP_TRACE_PREPEND_STR_SIZE(100) + BUFFER_SIZE(121).
 * Always make sure to change this QDF_DP_TRACE_MAX_RECORD_SIZE
 * value accordingly whenever above two mentioned MACRO value changes.
 */
#define QDF_DP_TRACE_MAX_RECORD_SIZE 300

static void qdf_dp_unused(struct qdf_dp_trace_record_s *record,
			  uint16_t index, uint8_t pdev_id, uint8_t info)
{
	qdf_print("%s: QDF_DP_TRACE_MAX event should not be generated",
		  __func__);
}

/**
 * qdf_dp_trace_init() - enables the DP trace
 * @live_mode_config: live mode configuration
 * @thresh: high throughput threshold for disabling live mode
 * @thresh_time_limit: max time to wait before deciding if thresh is crossed
 * @verbosity: dptrace verbosity level
 * @proto_bitmap: bitmap to enable/disable specific protocols
 *
 * Called during driver load to init dptrace
 *
 * A brief note on the 'thresh' param -
 * Total # of packets received in a bandwidth timer interval beyond which
 * DP Trace logging for data packets (including ICMP) will be disabled.
 * In memory logging will still continue for these packets. Other packets for
 * which proto.bitmap is set will continue to be recorded in logs and in memory.

 * Return: None
 */
void qdf_dp_trace_init(bool live_mode_config, uint8_t thresh,
				uint16_t time_limit, uint8_t verbosity,
				uint32_t proto_bitmap)
{
	uint8_t i;

	if (allocate_g_qdf_dp_trace_tbl_buffer() != QDF_STATUS_SUCCESS) {
		QDF_TRACE_ERROR(QDF_MODULE_ID_QDF,
				"Failed!!! DP Trace buffer allocation");
		return;
	}
	qdf_dp_trace_spin_lock_init();
	qdf_dp_trace_clear_buffer();
	g_qdf_dp_trace_data.enable = true;
	g_qdf_dp_trace_data.no_of_record = 1;

	g_qdf_dp_trace_data.live_mode_config = live_mode_config;
	g_qdf_dp_trace_data.live_mode = live_mode_config;
	g_qdf_dp_trace_data.high_tput_thresh = thresh;
	g_qdf_dp_trace_data.thresh_time_limit = time_limit;
	g_qdf_dp_trace_data.proto_bitmap = proto_bitmap;
	g_qdf_dp_trace_data.verbosity = verbosity;
	g_qdf_dp_trace_data.ini_conf_verbosity = verbosity;

	for (i = 0; i < ARRAY_SIZE(qdf_dp_trace_cb_table); i++)
		qdf_dp_trace_cb_table[i] = qdf_dp_display_record;

	qdf_dp_trace_cb_table[QDF_DP_TRACE_HDD_TX_PACKET_RECORD] =
		qdf_dp_trace_cb_table[QDF_DP_TRACE_HDD_RX_PACKET_RECORD] =
		qdf_dp_trace_cb_table[QDF_DP_TRACE_TX_PACKET_RECORD] =
		qdf_dp_trace_cb_table[QDF_DP_TRACE_RX_PACKET_RECORD] =
		qdf_dp_trace_cb_table[QDF_DP_TRACE_DROP_PACKET_RECORD] =
		qdf_dp_trace_cb_table[QDF_DP_TRACE_LI_DP_TX_PACKET_RECORD] =
		qdf_dp_trace_cb_table[QDF_DP_TRACE_LI_DP_RX_PACKET_RECORD] =
		qdf_dp_display_data_pkt_record;

	qdf_dp_trace_cb_table[QDF_DP_TRACE_TXRX_PACKET_PTR_RECORD] =
	qdf_dp_trace_cb_table[QDF_DP_TRACE_TXRX_FAST_PACKET_PTR_RECORD] =
	qdf_dp_trace_cb_table[QDF_DP_TRACE_FREE_PACKET_PTR_RECORD] =
	qdf_dp_trace_cb_table[QDF_DP_TRACE_LI_DP_FREE_PACKET_PTR_RECORD] =
						qdf_dp_display_ptr_record;
	qdf_dp_trace_cb_table[QDF_DP_TRACE_EAPOL_PACKET_RECORD] =
	qdf_dp_trace_cb_table[QDF_DP_TRACE_DHCP_PACKET_RECORD] =
	qdf_dp_trace_cb_table[QDF_DP_TRACE_ARP_PACKET_RECORD] =
	qdf_dp_trace_cb_table[QDF_DP_TRACE_ICMP_PACKET_RECORD] =
	qdf_dp_trace_cb_table[QDF_DP_TRACE_ICMPv6_PACKET_RECORD] =
						qdf_dp_display_proto_pkt;
	qdf_dp_trace_cb_table[QDF_DP_TRACE_MGMT_PACKET_RECORD] =
					qdf_dp_display_mgmt_pkt;
	qdf_dp_trace_cb_table[QDF_DP_TRACE_TX_CREDIT_RECORD] =
					qdf_dp_display_credit_record;
	qdf_dp_trace_cb_table[QDF_DP_TRACE_EVENT_RECORD] =
					qdf_dp_display_event_record;

	qdf_dp_trace_cb_table[QDF_DP_TRACE_MAX] = qdf_dp_unused;
}
qdf_export_symbol(qdf_dp_trace_init);

void qdf_dp_trace_deinit(void)
{
	if (!g_qdf_dp_trace_data.enable)
		return;
	spin_lock_bh(&l_dp_trace_lock);
	g_qdf_dp_trace_data.enable = false;
	g_qdf_dp_trace_data.no_of_record = 0;
	spin_unlock_bh(&l_dp_trace_lock);

	free_g_qdf_dp_trace_tbl_buffer();
}
/**
 * qdf_dp_trace_set_value() - Configure the value to control DP trace
 * @proto_bitmap: defines the protocol to be tracked
 * @no_of_records: defines the nth packet which is traced
 * @verbosity: defines the verbosity level
 *
 * Return: None
 */
void qdf_dp_trace_set_value(uint32_t proto_bitmap, uint8_t no_of_record,
			    uint8_t verbosity)
{
	g_qdf_dp_trace_data.proto_bitmap = proto_bitmap;
	g_qdf_dp_trace_data.no_of_record = no_of_record;
	g_qdf_dp_trace_data.verbosity    = verbosity;
	g_qdf_dp_trace_data.dynamic_verbosity_modify = true;
}
qdf_export_symbol(qdf_dp_trace_set_value);

/**
 * qdf_dp_trace_set_verbosity() - set verbosity value
 *
 * @val: Value to set
 *
 * Return: Null
 */
void qdf_dp_trace_set_verbosity(uint32_t val)
{
	g_qdf_dp_trace_data.verbosity = val;
}
qdf_export_symbol(qdf_dp_trace_set_verbosity);

/**
 * qdf_dp_get_verbosity) - get verbosity value
 *
 * Return: int
 */
uint8_t qdf_dp_get_verbosity(void)
{
	return g_qdf_dp_trace_data.verbosity;
}
qdf_export_symbol(qdf_dp_get_verbosity);

/**
 * qdf_dp_set_proto_bitmap() - set dp trace proto bitmap
 *
 * @val         : unsigned bitmap to set
 *
 * Return: proto bitmap
 */
void qdf_dp_set_proto_bitmap(uint32_t val)
{
	g_qdf_dp_trace_data.proto_bitmap = val;
}
qdf_export_symbol(qdf_dp_set_proto_bitmap);

void qdf_dp_set_proto_event_bitmap(uint32_t value)
{
	g_qdf_dp_trace_data.proto_event_bitmap = value;
}

qdf_export_symbol(qdf_dp_set_proto_event_bitmap);

static uint32_t qdf_dp_get_proto_event_bitmap(void)
{
	return g_qdf_dp_trace_data.proto_event_bitmap;
}

/**
 * qdf_dp_set_no_of_record() - set dp trace no_of_record
 *
 * @val         : unsigned no_of_record to set
 *
 * Return: null
 */
void qdf_dp_set_no_of_record(uint32_t val)
{
	g_qdf_dp_trace_data.no_of_record = val;
}
qdf_export_symbol(qdf_dp_set_no_of_record);

/**
 * qdf_dp_get_no_of_record() - get dp trace no_of_record
 *
 * Return: number of records
 */
uint8_t qdf_dp_get_no_of_record(void)
{
	return g_qdf_dp_trace_data.no_of_record;
}
qdf_export_symbol(qdf_dp_get_no_of_record);


/**
 * qdf_dp_trace_verbosity_check() - check whether verbosity level is enabled
 * @code: defines the event
 *
 * In High verbosity all codes are logged.
 * For Med/Low and Default case code which has
 * less value than corresponding verbosity codes
 * are logged.
 *
 * Return: true or false depends on whether tracing enabled
 */
static bool qdf_dp_trace_verbosity_check(enum QDF_DP_TRACE_ID code)
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
	case QDF_DP_TRACE_VERBOSITY_ULTRA_LOW:
		if (code <= QDF_DP_TRACE_ULTRA_LOW_VERBOSITY)
			return true;
		return false;
	case QDF_DP_TRACE_VERBOSITY_BASE:
		if (code <= QDF_DP_TRACE_BASE_VERBOSITY)
			return true;
		return false;
	default:
		return false;
	}
}

/**
 * qdf_dp_get_proto_bitmap() - get dp trace proto bitmap
 *
 * Return: proto bitmap
 */
uint32_t qdf_dp_get_proto_bitmap(void)
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

	if (!g_qdf_dp_trace_data.enable)
		return;

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
}
qdf_export_symbol(qdf_dp_trace_set_track);

/* Number of bytes to be grouped together while printing DP-Trace data */
#define QDF_DUMP_DP_GROUP_SIZE 6

/**
 * dump_dp_hex_trace() - Display the data in buffer
 * @prepend_str:     string to prepend the hexdump with.
 * @inbuf:     buffer which contains data to be displayed
 * @inbuf_len: defines the size of the data to be displayed
 *
 * Return: None
 */
static void
dump_dp_hex_trace(char *prepend_str, uint8_t *inbuf, uint8_t inbuf_len)
{
	unsigned char outbuf[BUFFER_SIZE];
	const uint8_t *inbuf_ptr = inbuf;
	char *outbuf_ptr = outbuf;
	int outbytes_written = 0;

	qdf_mem_zero(outbuf, sizeof(outbuf));
	do {
		outbytes_written += scnprintf(outbuf_ptr,
					BUFFER_SIZE - outbytes_written,
					"%02x", *inbuf_ptr);
		outbuf_ptr = outbuf + outbytes_written;

		if ((inbuf_ptr - inbuf) &&
		    (inbuf_ptr - inbuf + 1) % QDF_DUMP_DP_GROUP_SIZE == 0) {
			outbytes_written += scnprintf(outbuf_ptr,
						BUFFER_SIZE - outbytes_written,
						" ");
			outbuf_ptr = outbuf + outbytes_written;
		}
		inbuf_ptr++;
	} while (inbuf_ptr < (inbuf + inbuf_len));
	DPTRACE_PRINT("%s %s", prepend_str, outbuf);
}

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
	case QDF_DP_TRACE_ICMP_PACKET_RECORD:
		return "ICMP:";
	case QDF_DP_TRACE_ICMPv6_PACKET_RECORD:
		return "ICMPv6:";
	case QDF_DP_TRACE_MGMT_PACKET_RECORD:
		return "MGMT:";
	case QDF_DP_TRACE_TX_CREDIT_RECORD:
		return "CREDIT:";
	case QDF_DP_TRACE_EVENT_RECORD:
		return "EVENT:";
	case QDF_DP_TRACE_HDD_TX_PACKET_PTR_RECORD:
		return "HDD: TX: PTR:";
	case QDF_DP_TRACE_LI_DP_TX_PACKET_PTR_RECORD:
		return "LI_DP: TX: PTR:";
	case QDF_DP_TRACE_HDD_TX_PACKET_RECORD:
		return "HDD: TX: DATA:";
	case QDF_DP_TRACE_LI_DP_TX_PACKET_RECORD:
	case QDF_DP_TRACE_TX_PACKET_RECORD:
		return "TX:";
	case QDF_DP_TRACE_CE_PACKET_PTR_RECORD:
		return "CE: TX: PTR:";
	case QDF_DP_TRACE_CE_FAST_PACKET_PTR_RECORD:
		return "CE: TX: FAST: PTR:";
	case QDF_DP_TRACE_CE_FAST_PACKET_ERR_RECORD:
		return "CE: TX: FAST: ERR:";
	case QDF_DP_TRACE_LI_DP_FREE_PACKET_PTR_RECORD:
	case QDF_DP_TRACE_FREE_PACKET_PTR_RECORD:
		return "FREE: TX: PTR:";
	case QDF_DP_TRACE_RX_HTT_PACKET_PTR_RECORD:
		return "HTT: RX: PTR:";
	case QDF_DP_TRACE_RX_OFFLOAD_HTT_PACKET_PTR_RECORD:
		return "HTT: RX: OF: PTR:";
	case QDF_DP_TRACE_RX_HDD_PACKET_PTR_RECORD:
		return "HDD: RX: PTR:";
	case QDF_DP_TRACE_RX_LI_DP_PACKET_PTR_RECORD:
		return "LI_DP: RX: PTR:";
	case QDF_DP_TRACE_HDD_RX_PACKET_RECORD:
		return "HDD: RX: DATA:";
	case QDF_DP_TRACE_LI_DP_NULL_RX_PACKET_RECORD:
		return "LI_DP_NULL: RX: DATA:";
	case QDF_DP_TRACE_LI_DP_RX_PACKET_RECORD:
	case QDF_DP_TRACE_RX_PACKET_RECORD:
		return "RX:";
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

static const char *qdf_dp_credit_source_to_str(
		enum QDF_CREDIT_UPDATE_SOURCE source)
{
	switch (source) {
	case QDF_TX_SCHED:
		return "TX SCHED";
	case QDF_TX_COMP:
		return "TX COMP";
	case QDF_TX_CREDIT_UPDATE:
		return "CREDIT UP";
	case QDF_TX_HTT_MSG:
		return "HTT TX MSG";
	case QDF_HTT_ATTACH:
		return "HTT ATTACH";
	default:
		return "invalid";
	}
}

static const char *qdf_dp_operation_to_str(enum QDF_CREDIT_OPERATION op)
{
	switch (op) {
	case QDF_CREDIT_INC:
		return "+";
	case QDF_CREDIT_DEC:
		return "-";
	case QDF_CREDIT_ABS:
		return "ABS";
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
	case QDF_PROTO_TYPE_ICMP:
		return "ICMP";
	case QDF_PROTO_TYPE_ICMPv6:
		return "ICMPv6";
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
		return "DISC";
	case QDF_PROTO_DHCP_REQUEST:
		return "REQ";
	case QDF_PROTO_DHCP_OFFER:
		return "OFF";
	case QDF_PROTO_DHCP_ACK:
		return "ACK";
	case QDF_PROTO_DHCP_NACK:
		return "NACK";
	case QDF_PROTO_DHCP_RELEASE:
		return "REL";
	case QDF_PROTO_DHCP_INFORM:
		return "INFORM";
	case QDF_PROTO_DHCP_DECLINE:
		return "DECL";
	case QDF_PROTO_ARP_REQ:
	case QDF_PROTO_ICMP_REQ:
	case QDF_PROTO_ICMPV6_REQ:
		return "REQ";
	case QDF_PROTO_ARP_RES:
	case QDF_PROTO_ICMP_RES:
	case QDF_PROTO_ICMPV6_RES:
		return "RSP";
	case QDF_PROTO_ICMPV6_RS:
		return "RS";
	case QDF_PROTO_ICMPV6_RA:
		return "RA";
	case QDF_PROTO_ICMPV6_NS:
		return "NS";
	case QDF_PROTO_ICMPV6_NA:
		return "NA";
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
		return "ROAM COMP";
	case QDF_ROAM_EVENTID:
		return "ROAM EVENTID";
	default:
		return "invalid";
	}
}

/**
 * qdf_dp_enable_check() - check if dptrace, TX/RX tracing is enabled
 * @nbuf: nbuf
 * @code: dptrace code
 * @dir: TX or RX direction
 *
 * Return: true/false
 */
static bool qdf_dp_enable_check(qdf_nbuf_t nbuf, enum QDF_DP_TRACE_ID code,
				enum qdf_proto_dir dir)
{
	/* Return when Dp trace is not enabled */
	if (!g_qdf_dp_trace_data.enable)
		return false;

	if (qdf_dp_trace_verbosity_check(code) == false)
		return false;

	if (nbuf && (dir == QDF_TX && ((QDF_NBUF_CB_TX_DP_TRACE(nbuf) == 0) ||
				       (QDF_NBUF_CB_TX_PACKET_TRACK(nbuf) !=
					QDF_NBUF_TX_PKT_DATA_TRACK))))
		return false;

	if (nbuf && (dir == QDF_RX && (QDF_NBUF_CB_RX_DP_TRACE(nbuf) == 0)))
		return false;

	/*
	 * Special packets called with NULL nbuf and this API is expected to
	 * return true
	 */
	return true;
}

/**
 * qdf_dp_trace_fill_meta_str() - fill up a common meta string
 * @prepend_str: pointer to string
 * @size: size of prepend_str
 * @rec_index: index of record
 * @info: info related to the record
 * @record: pointer to the record
 *
 * Return: ret value from scnprintf
 */
static inline
int qdf_dp_trace_fill_meta_str(char *prepend_str, int size,
			       int rec_index, uint8_t info,
			       struct qdf_dp_trace_record_s *record)
{
	char buffer[20];
	int ret = 0;
	bool live = info & QDF_DP_TRACE_RECORD_INFO_LIVE ? true : false;
	bool throttled = info & QDF_DP_TRACE_RECORD_INFO_THROTTLED ?
								true : false;

	scnprintf(buffer, sizeof(buffer), "%llu", record->time);
	ret = scnprintf(prepend_str, size,
			"%s DPT: %04d:%02d%s %s",
			throttled ? "*" : "",
			rec_index,
			record->pdev_id,
			live ? "" : buffer,
			qdf_dp_code_to_string(record->code));

	return ret;
}

/**
 * qdf_dp_fill_record_data() - fill meta data and data into the record
 * @rec: pointer to record data
 * @data: pointer to data
 * @data_size: size of the data
 * @meta_data: pointer to metadata
 * @metadata_size: size of metadata
 *
 * Should be called from within a spin_lock for the qdf record.
 * Fills up rec->data with |metadata|data|
 *
 * Return: none
 */
static void qdf_dp_fill_record_data
	(struct qdf_dp_trace_record_s *rec,
	uint8_t *data, uint8_t data_size,
	uint8_t *meta_data, uint8_t metadata_size)
{
	int32_t available = QDF_DP_TRACE_RECORD_SIZE;
	uint8_t *rec_data = rec->data;
	uint8_t data_to_copy = 0;

	qdf_mem_zero(rec_data, QDF_DP_TRACE_RECORD_SIZE);

	/* copy meta data */
	if (meta_data) {
		if (metadata_size > available) {
			QDF_TRACE_WARN(QDF_MODULE_ID_QDF,
				       "%s: meta data does not fit into the record",
				       __func__);
			goto end;
		}
		qdf_mem_copy(rec_data, meta_data, metadata_size);
		available = available - metadata_size;
	} else {
		metadata_size = 0;
	}

	/* copy data */
	if (data && (data_size > 0) && (available > 0)) {
		data_to_copy = data_size;
		if (data_size > available)
			data_to_copy = available;
		qdf_mem_copy(&rec_data[metadata_size], data, data_to_copy);
	}
end:
	rec->size = data_to_copy;
}

/**
 * qdf_dp_add_record() - add dp trace record
 * @code: dptrace code
 * @pdev_id: pdev_id
 * @print: true to print it in kmsg
 * @data: data pointer
 * @data_size: size of data to be copied
 * @meta_data: meta data to be prepended to data
 * @metadata_size: sizeof meta data
 * @print: whether to print record
 *
 * Return: none
 */
static void qdf_dp_add_record(enum QDF_DP_TRACE_ID code, uint8_t pdev_id,
			      uint8_t *data, uint8_t data_size,
			      uint8_t *meta_data, uint8_t metadata_size,
			      bool print)

{
	struct qdf_dp_trace_record_s *rec = NULL;
	int index;
	bool print_this_record = false;
	u8 info = 0;

	if (code >= QDF_DP_TRACE_MAX) {
		QDF_TRACE_ERROR(QDF_MODULE_ID_QDF,
				"invalid record code %u, max code %u",
				code, QDF_DP_TRACE_MAX);
		return;
	}

	spin_lock_bh(&l_dp_trace_lock);

	if (print || g_qdf_dp_trace_data.force_live_mode) {
		print_this_record = true;
	} else if (g_qdf_dp_trace_data.live_mode == 1) {
		print_this_record = true;
		g_qdf_dp_trace_data.print_pkt_cnt++;
		if (g_qdf_dp_trace_data.print_pkt_cnt >
				g_qdf_dp_trace_data.high_tput_thresh) {
			g_qdf_dp_trace_data.live_mode = 0;
			g_qdf_dp_trace_data.verbosity =
					QDF_DP_TRACE_VERBOSITY_ULTRA_LOW;
			info |= QDF_DP_TRACE_RECORD_INFO_THROTTLED;
		}
	}

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
	rec->pdev_id = pdev_id;
	rec->size = 0;
	qdf_dp_fill_record_data(rec, data, data_size,
				meta_data, metadata_size);
	rec->time = qdf_get_log_timestamp();
	rec->pid = (in_interrupt() ? 0 : current->pid);

	if (rec->code >= QDF_DP_TRACE_MAX) {
		QDF_TRACE_ERROR(QDF_MODULE_ID_QDF,
				"invalid record code %u, max code %u",
				rec->code, QDF_DP_TRACE_MAX);
		return;
	}

	spin_unlock_bh(&l_dp_trace_lock);

	info |= QDF_DP_TRACE_RECORD_INFO_LIVE;
	if (print_this_record)
		qdf_dp_trace_cb_table[rec->code] (rec, index,
					QDF_TRACE_DEFAULT_PDEV_ID, info);
}

/**
 * qdf_get_rate_limit_by_type() - Get the rate limit by pkt type
 * @type: packet type
 *
 * Return: Rate limit value for a particular packet type
 */
static inline
uint8_t qdf_get_rate_limit_by_type(uint8_t type)
{
	switch (type) {
	case QDF_PROTO_TYPE_DHCP:
		return QDF_MAX_DHCP_PKTS_PER_SEC;
	case QDF_PROTO_TYPE_EAPOL:
		return QDF_MAX_EAPOL_PKTS_PER_SEC;
	case QDF_PROTO_TYPE_ARP:
		return QDF_MAX_ARP_PKTS_PER_SEC;
	case QDF_PROTO_TYPE_DNS:
		return QDF_MAX_DNS_PKTS_PER_SEC;
	default:
		return QDF_MAX_OTHER_PKTS_PER_SEC;
	}
}

/**
 * qdf_get_pkt_type_string() - Get the string based on pkt type
 * @type: packet type
 * @subtype: packet subtype
 *
 * Return: String based on pkt type
 */
static
uint8_t *qdf_get_pkt_type_string(uint8_t type, uint8_t subtype)
{
	switch (subtype) {
	case QDF_PROTO_EAPOL_M1:
		return "EAPOL-1";
	case QDF_PROTO_EAPOL_M2:
		return "EAPOL-2";
	case QDF_PROTO_EAPOL_M3:
		return "EAPOL-3";
	case QDF_PROTO_EAPOL_M4:
		return "EAPOL-4";
	case QDF_PROTO_DHCP_DISCOVER:
		return "DHCP-D";
	case QDF_PROTO_DHCP_REQUEST:
		return "DHCP-R";
	case QDF_PROTO_DHCP_OFFER:
		return "DHCP-O";
	case QDF_PROTO_DHCP_ACK:
		return "DHCP-A";
	case QDF_PROTO_DHCP_NACK:
		return "DHCP-NA";
	case QDF_PROTO_DHCP_RELEASE:
		return "DHCP-REL";
	case QDF_PROTO_DHCP_INFORM:
		return "DHCP-IN";
	case QDF_PROTO_DHCP_DECLINE:
		return "DHCP-DEC";
	case QDF_PROTO_ARP_REQ:
		return "ARP-RQ";
	case QDF_PROTO_ARP_RES:
		return "ARP-RS";
	case QDF_PROTO_DNS_QUERY:
		return "DNS_Q";
	case QDF_PROTO_DNS_RES:
		return "DNS_RS";
	default:
		switch (type) {
		case QDF_PROTO_TYPE_EAPOL:
			return "EAP";
		case QDF_PROTO_TYPE_DHCP:
			return "DHCP";
		case QDF_PROTO_TYPE_ARP:
			return "ARP";
		case QDF_PROTO_TYPE_DNS:
			return "DNS";
		default:
			return "UNKNOWN";
		}
	}
}

/**
 * qdf_get_pkt_status_string() - Get the string based on pkt status
 * @status: packet status
 *
 * Return: String based on pkt status
 */
static
uint8_t *qdf_get_pkt_status_string(uint8_t status)
{
	switch (status) {
	case QDF_TX_RX_STATUS_INVALID:
		return "inv";
	case QDF_TX_RX_STATUS_OK:
		return "succ";
	case QDF_TX_RX_STATUS_FW_DISCARD:
		return "disc";
	case QDF_TX_RX_STATUS_NO_ACK:
		return "nack";
	case QDF_TX_RX_STATUS_DROP:
		return "drop";
	default:
		return "unknown";
	}
}

/**
 * qdf_dp_log_proto_pkt_info() - Send diag log with pkt info
 * @sa: Source MAC address
 * @da: Destination MAC address
 * @type: packet type
 * @subtype: packet subtype
 * @dir: tx or rx
 * @msdu_id: MSDU id
 * @status: status code
 *
 * Return: none
 */
void qdf_dp_log_proto_pkt_info(uint8_t *sa, uint8_t *da, uint8_t type,
			       uint8_t subtype, uint8_t dir, uint16_t msdu_id,
			       uint8_t status)
{
	uint8_t pkt_rate_limit;
	static ulong last_ticks_tx[QDF_PROTO_SUBTYPE_MAX] = {0};
	static ulong last_ticks_rx[QDF_PROTO_SUBTYPE_MAX] = {0};
	ulong curr_ticks = jiffies;

	pkt_rate_limit = qdf_get_rate_limit_by_type(type);

	if ((dir == QDF_TX &&
	     !time_after(curr_ticks,
			 last_ticks_tx[subtype] + HZ / pkt_rate_limit)) ||
	    (dir == QDF_RX &&
	     !time_after(curr_ticks,
			 last_ticks_rx[subtype] + HZ / pkt_rate_limit)))
		return;

	if (dir == QDF_TX)
		last_ticks_tx[subtype] = curr_ticks;
	else
		last_ticks_rx[subtype] = curr_ticks;

	if (status == QDF_TX_RX_STATUS_INVALID)
		qdf_nofl_debug("%s %s: SA:"QDF_MAC_ADDR_FMT" DA:"QDF_MAC_ADDR_FMT,
			       qdf_get_pkt_type_string(type, subtype),
			       dir ? "RX":"TX", QDF_MAC_ADDR_REF(sa),
			       QDF_MAC_ADDR_REF(da));
	else
		qdf_nofl_debug("%s %s: SA:"QDF_MAC_ADDR_FMT" DA:"QDF_MAC_ADDR_FMT" msdu_id:%d status: %s",
			       qdf_get_pkt_type_string(type, subtype),
			       dir ? "RX":"TX", QDF_MAC_ADDR_REF(sa),
			       QDF_MAC_ADDR_REF(da), msdu_id,
			       qdf_get_pkt_status_string(status));
}

qdf_export_symbol(qdf_dp_log_proto_pkt_info);

/**
 * qdf_log_icmpv6_pkt() - log ICMPv6 packet
 * @vdev_id: ID of the vdev
 * @skb: skb pointer
 * @dir: direction
 * @pdev_id: ID of the pdev
 *
 * Return: true/false
 */
static bool qdf_log_icmpv6_pkt(uint8_t vdev_id, struct sk_buff *skb,
			       enum qdf_proto_dir dir, uint8_t pdev_id)
{
	enum qdf_proto_subtype subtype;

	if ((qdf_dp_get_proto_bitmap() & QDF_NBUF_PKT_TRAC_TYPE_ICMPv6) &&
		((dir == QDF_TX && QDF_NBUF_CB_PACKET_TYPE_ICMPv6 ==
			QDF_NBUF_CB_GET_PACKET_TYPE(skb)) ||
		 (dir == QDF_RX && qdf_nbuf_is_icmpv6_pkt(skb) == true))) {

		subtype = qdf_nbuf_get_icmpv6_subtype(skb);

		QDF_NBUF_CB_DP_TRACE_PRINT(skb) = false;
		if (dir == QDF_TX)
			QDF_NBUF_CB_TX_DP_TRACE(skb) = 1;
		else if (dir == QDF_RX)
			QDF_NBUF_CB_RX_DP_TRACE(skb) = 1;

		DPTRACE(qdf_dp_trace_proto_pkt(
			QDF_DP_TRACE_ICMPv6_PACKET_RECORD,
			vdev_id, (skb->data + QDF_NBUF_SRC_MAC_OFFSET),
			(skb->data + QDF_NBUF_DEST_MAC_OFFSET),
			QDF_PROTO_TYPE_ICMPv6, subtype, dir, pdev_id, false));

		switch (subtype) {
		case QDF_PROTO_ICMPV6_REQ:
			g_qdf_dp_trace_data.icmpv6_req++;
			break;
		case QDF_PROTO_ICMPV6_RES:
			g_qdf_dp_trace_data.icmpv6_resp++;
			break;
		case QDF_PROTO_ICMPV6_RS:
			g_qdf_dp_trace_data.icmpv6_rs++;
			break;
		case QDF_PROTO_ICMPV6_RA:
			g_qdf_dp_trace_data.icmpv6_ra++;
			break;
		case QDF_PROTO_ICMPV6_NS:
			g_qdf_dp_trace_data.icmpv6_ns++;
			break;
		case QDF_PROTO_ICMPV6_NA:
			g_qdf_dp_trace_data.icmpv6_na++;
			break;
		default:
			break;
		}
		return true;
	}

	return false;
}

/**
 * qdf_log_icmp_pkt() - log ICMP packet
 * @vdev_id: ID of the vdev
 * @skb: skb pointer
 * @dir: direction
 * @pdev_id: ID of the pdev
 *
 * Return: true/false
 */
static bool qdf_log_icmp_pkt(uint8_t vdev_id, struct sk_buff *skb,
			     enum qdf_proto_dir dir, uint8_t pdev_id)
{
	enum qdf_proto_subtype proto_subtype;

	if ((qdf_dp_get_proto_bitmap() & QDF_NBUF_PKT_TRAC_TYPE_ICMP) &&
	    (qdf_nbuf_is_icmp_pkt(skb) == true)) {

		QDF_NBUF_CB_DP_TRACE_PRINT(skb) = false;
		proto_subtype = qdf_nbuf_get_icmp_subtype(skb);

		if (QDF_TX == dir)
			QDF_NBUF_CB_TX_DP_TRACE(skb) = 1;
		else if (QDF_RX == dir)
			QDF_NBUF_CB_RX_DP_TRACE(skb) = 1;

		DPTRACE(qdf_dp_trace_proto_pkt(QDF_DP_TRACE_ICMP_PACKET_RECORD,
					       vdev_id,
					       skb->data +
					       QDF_NBUF_SRC_MAC_OFFSET,
					       skb->data +
					       QDF_NBUF_DEST_MAC_OFFSET,
					       QDF_PROTO_TYPE_ICMP,
					       proto_subtype, dir, pdev_id,
					       false));

		if (proto_subtype == QDF_PROTO_ICMP_REQ)
			g_qdf_dp_trace_data.icmp_req++;
		else
			g_qdf_dp_trace_data.icmp_resp++;

		return true;
	}
	return false;
}

/**
 * qdf_log_eapol_pkt() - log EAPOL packet
 * @vdev_id: ID of the vdev
 * @skb: skb pointer
 * @dir: direction
 * @pdev_id: ID of the pdev
 *
 * Return: true/false
 */
static bool qdf_log_eapol_pkt(uint8_t vdev_id, struct sk_buff *skb,
			      enum qdf_proto_dir dir, uint8_t pdev_id)
{
	enum qdf_proto_subtype subtype;
	uint32_t dp_eap_trace;
	uint32_t dp_eap_event;

	dp_eap_trace = qdf_dp_get_proto_bitmap() & QDF_NBUF_PKT_TRAC_TYPE_EAPOL;
	dp_eap_event = qdf_dp_get_proto_event_bitmap() &
				QDF_NBUF_PKT_TRAC_TYPE_EAPOL;

	if (!dp_eap_trace && !dp_eap_event)
		return false;

	if (!((dir == QDF_TX && QDF_NBUF_CB_PACKET_TYPE_EAPOL ==
	       QDF_NBUF_CB_GET_PACKET_TYPE(skb)) ||
	      (dir == QDF_RX && qdf_nbuf_is_ipv4_eapol_pkt(skb) == true)))
		return false;

	subtype = qdf_nbuf_get_eapol_subtype(skb);

	if (dp_eap_event && dir == QDF_RX)
		qdf_dp_log_proto_pkt_info(skb->data + QDF_NBUF_SRC_MAC_OFFSET,
					  skb->data + QDF_NBUF_DEST_MAC_OFFSET,
					  QDF_PROTO_TYPE_EAPOL, subtype, dir,
					  QDF_TRACE_DEFAULT_MSDU_ID,
					  QDF_TX_RX_STATUS_INVALID);

	if (dp_eap_trace) {
		QDF_NBUF_CB_DP_TRACE_PRINT(skb) = true;
		if (QDF_TX == dir)
			QDF_NBUF_CB_TX_DP_TRACE(skb) = 1;
		else if (QDF_RX == dir)
			QDF_NBUF_CB_RX_DP_TRACE(skb) = 1;

		DPTRACE(qdf_dp_trace_proto_pkt(QDF_DP_TRACE_EAPOL_PACKET_RECORD,
					       vdev_id,
					       skb->data +
					       QDF_NBUF_SRC_MAC_OFFSET,
					       skb->data +
					       QDF_NBUF_DEST_MAC_OFFSET,
					       QDF_PROTO_TYPE_EAPOL, subtype,
					       dir, pdev_id, true));

		switch (subtype) {
		case QDF_PROTO_EAPOL_M1:
			g_qdf_dp_trace_data.eapol_m1++;
			break;
		case QDF_PROTO_EAPOL_M2:
			g_qdf_dp_trace_data.eapol_m2++;
			break;
		case QDF_PROTO_EAPOL_M3:
			g_qdf_dp_trace_data.eapol_m3++;
			break;
		case QDF_PROTO_EAPOL_M4:
			g_qdf_dp_trace_data.eapol_m4++;
			break;
		default:
			g_qdf_dp_trace_data.eapol_others++;
			break;
		}
	}

	return true;
}

/**
 * qdf_log_dhcp_pkt() - log DHCP packet
 * @vdev_id: ID of the vdev
 * @skb: skb pointer
 * @dir: direction
 * @pdev_id: ID of the pdev
 *
 * Return: true/false
 */
static bool qdf_log_dhcp_pkt(uint8_t vdev_id, struct sk_buff *skb,
			     enum qdf_proto_dir dir, uint8_t pdev_id)
{
	enum qdf_proto_subtype subtype = QDF_PROTO_INVALID;
	uint32_t dp_dhcp_trace;
	uint32_t dp_dhcp_event;

	dp_dhcp_trace = qdf_dp_get_proto_bitmap() & QDF_NBUF_PKT_TRAC_TYPE_DHCP;
	dp_dhcp_event = qdf_dp_get_proto_event_bitmap() &
				QDF_NBUF_PKT_TRAC_TYPE_DHCP;

	if (!dp_dhcp_trace && !dp_dhcp_event)
		return false;

	if (!((dir == QDF_TX && QDF_NBUF_CB_PACKET_TYPE_DHCP ==
	       QDF_NBUF_CB_GET_PACKET_TYPE(skb)) ||
	      (dir == QDF_RX && qdf_nbuf_is_ipv4_dhcp_pkt(skb) == true)))
		return false;

	subtype = qdf_nbuf_get_dhcp_subtype(skb);

	if (dp_dhcp_event && dir == QDF_RX)
		qdf_dp_log_proto_pkt_info(skb->data + QDF_NBUF_SRC_MAC_OFFSET,
					  skb->data + QDF_NBUF_DEST_MAC_OFFSET,
					  QDF_PROTO_TYPE_DHCP, subtype, dir,
					  QDF_TRACE_DEFAULT_MSDU_ID,
					  QDF_TX_RX_STATUS_INVALID);

	if (dp_dhcp_trace) {
		QDF_NBUF_CB_DP_TRACE_PRINT(skb) = true;
		if (QDF_TX == dir)
			QDF_NBUF_CB_TX_DP_TRACE(skb) = 1;
		else if (QDF_RX == dir)
			QDF_NBUF_CB_RX_DP_TRACE(skb) = 1;

		DPTRACE(qdf_dp_trace_proto_pkt(QDF_DP_TRACE_DHCP_PACKET_RECORD,
					       vdev_id,
					       skb->data +
					       QDF_NBUF_SRC_MAC_OFFSET,
					       skb->data +
					       QDF_NBUF_DEST_MAC_OFFSET,
					       QDF_PROTO_TYPE_DHCP, subtype,
					       dir, pdev_id, true));

		switch (subtype) {
		case QDF_PROTO_DHCP_DISCOVER:
			g_qdf_dp_trace_data.dhcp_disc++;
			break;
		case QDF_PROTO_DHCP_OFFER:
			g_qdf_dp_trace_data.dhcp_off++;
			break;
		case QDF_PROTO_DHCP_REQUEST:
			g_qdf_dp_trace_data.dhcp_req++;
			break;
		case QDF_PROTO_DHCP_ACK:
			g_qdf_dp_trace_data.dhcp_ack++;
			break;
		case QDF_PROTO_DHCP_NACK:
			g_qdf_dp_trace_data.dhcp_nack++;
			break;
		default:
			g_qdf_dp_trace_data.eapol_others++;
			break;
		}
	}

	return true;
}

/**
 * qdf_log_arp_pkt() - log ARP packet
 * @vdev_id: ID of the vdev
 * @skb: skb pointer
 * @dir: direction
 * @pdev_id: ID of the pdev
 *
 * Return: true/false
 */
static bool qdf_log_arp_pkt(uint8_t vdev_id, struct sk_buff *skb,
			    enum qdf_proto_dir dir, uint8_t pdev_id)
{
	enum qdf_proto_subtype proto_subtype;

	if ((qdf_dp_get_proto_bitmap() & QDF_NBUF_PKT_TRAC_TYPE_ARP) &&
		((dir == QDF_TX && QDF_NBUF_CB_PACKET_TYPE_ARP ==
			QDF_NBUF_CB_GET_PACKET_TYPE(skb)) ||
		 (dir == QDF_RX && qdf_nbuf_is_ipv4_arp_pkt(skb) == true))) {

		proto_subtype = qdf_nbuf_get_arp_subtype(skb);
		QDF_NBUF_CB_DP_TRACE_PRINT(skb) = true;
		if (QDF_TX == dir)
			QDF_NBUF_CB_TX_DP_TRACE(skb) = 1;
		else if (QDF_RX == dir)
			QDF_NBUF_CB_RX_DP_TRACE(skb) = 1;

		DPTRACE(qdf_dp_trace_proto_pkt(QDF_DP_TRACE_ARP_PACKET_RECORD,
					       vdev_id,
					       skb->data +
					       QDF_NBUF_SRC_MAC_OFFSET,
					       skb->data +
					       QDF_NBUF_DEST_MAC_OFFSET,
					       QDF_PROTO_TYPE_ARP,
					       proto_subtype, dir, pdev_id,
					       true));

		if (QDF_PROTO_ARP_REQ == proto_subtype)
			g_qdf_dp_trace_data.arp_req++;
		else
			g_qdf_dp_trace_data.arp_resp++;

		return true;
	}
	return false;
}


bool qdf_dp_trace_log_pkt(uint8_t vdev_id, struct sk_buff *skb,
			  enum qdf_proto_dir dir, uint8_t pdev_id)
{
	if (!qdf_dp_get_proto_bitmap() && !qdf_dp_get_proto_event_bitmap())
		return false;
	if (qdf_log_arp_pkt(vdev_id, skb, dir, pdev_id))
		return true;
	if (qdf_log_dhcp_pkt(vdev_id, skb, dir, pdev_id))
		return true;
	if (qdf_log_eapol_pkt(vdev_id, skb, dir, pdev_id))
		return true;
	if (qdf_log_icmp_pkt(vdev_id, skb, dir, pdev_id))
		return true;
	if (qdf_log_icmpv6_pkt(vdev_id, skb, dir, pdev_id))
		return true;
	return false;
}
qdf_export_symbol(qdf_dp_trace_log_pkt);

void qdf_dp_display_mgmt_pkt(struct qdf_dp_trace_record_s *record,
			      uint16_t index, uint8_t pdev_id, uint8_t info)
{
	int loc;
	char prepend_str[QDF_DP_TRACE_PREPEND_STR_SIZE];
	struct qdf_dp_trace_mgmt_buf *buf =
		(struct qdf_dp_trace_mgmt_buf *)record->data;

	qdf_mem_zero(prepend_str, sizeof(prepend_str));
	loc = qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
					 index, info, record);

	DPTRACE_PRINT("%s [%d] [%s %s]",
		      prepend_str,
		      buf->vdev_id,
		      qdf_dp_type_to_str(buf->type),
		      qdf_dp_subtype_to_str(buf->subtype));
}
qdf_export_symbol(qdf_dp_display_mgmt_pkt);


void qdf_dp_trace_mgmt_pkt(enum QDF_DP_TRACE_ID code, uint8_t vdev_id,
		uint8_t pdev_id, enum qdf_proto_type type,
		enum qdf_proto_subtype subtype)
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
	qdf_dp_add_record(code, pdev_id, (uint8_t *)&buf, buf_size,
			  NULL, 0, true);
}
qdf_export_symbol(qdf_dp_trace_mgmt_pkt);

static void
qdf_dpt_display_credit_record_debugfs(qdf_debugfs_file_t file,
				      struct qdf_dp_trace_record_s *record,
				      uint32_t index)
{
	int loc;
	char prepend_str[QDF_DP_TRACE_PREPEND_STR_SIZE];
	struct qdf_dp_trace_credit_record *buf =
		(struct qdf_dp_trace_credit_record *)record->data;

	loc = qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
					 index, 0, record);
	if (buf->operation == QDF_OP_NA)
		qdf_debugfs_printf(file, "%s [%s] [T: %d G0: %d G1: %d]\n",
				   prepend_str,
				   qdf_dp_credit_source_to_str(buf->source),
				   buf->total_credits, buf->g0_credit,
				   buf->g1_credit);
	else
		qdf_debugfs_printf(file,
				   "%s [%s] [T: %d G0: %d G1: %d] [%s %d]\n",
				   prepend_str,
				   qdf_dp_credit_source_to_str(buf->source),
				   buf->total_credits, buf->g0_credit,
				   buf->g1_credit,
				   qdf_dp_operation_to_str(buf->operation),
				   buf->delta);
}

void qdf_dp_display_credit_record(struct qdf_dp_trace_record_s *record,
				  uint16_t index, uint8_t pdev_id, uint8_t info)
{
	int loc;
	char prepend_str[QDF_DP_TRACE_PREPEND_STR_SIZE];
	struct qdf_dp_trace_credit_record *buf =
		(struct qdf_dp_trace_credit_record *)record->data;

	loc = qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
					 index, info, record);
	if (buf->operation == QDF_OP_NA)
		DPTRACE_PRINT("%s [%s] [T: %d G0: %d G1: %d]",
			      prepend_str,
			      qdf_dp_credit_source_to_str(buf->source),
			      buf->total_credits, buf->g0_credit,
			      buf->g1_credit);
	else
		DPTRACE_PRINT("%s [%s] [T: %d G0: %d G1: %d] [%s %d]",
			      prepend_str,
			      qdf_dp_credit_source_to_str(buf->source),
			      buf->total_credits, buf->g0_credit,
			      buf->g1_credit,
			      qdf_dp_operation_to_str(buf->operation),
			      buf->delta);
}

void qdf_dp_trace_credit_record(enum QDF_CREDIT_UPDATE_SOURCE source,
				enum QDF_CREDIT_OPERATION operation,
				int delta, int total_credits,
				int g0_credit, int g1_credit)
{
	struct qdf_dp_trace_credit_record buf;
	int buf_size = sizeof(struct qdf_dp_trace_credit_record);
	enum QDF_DP_TRACE_ID code = QDF_DP_TRACE_TX_CREDIT_RECORD;

	if (qdf_dp_enable_check(NULL, code, QDF_NA) == false)
		return;

	if (!(qdf_dp_get_proto_bitmap() & QDF_HL_CREDIT_TRACKING))
		return;

	if (buf_size > QDF_DP_TRACE_RECORD_SIZE)
		QDF_BUG(0);

	buf.source = source;
	buf.operation = operation;
	buf.delta = delta;
	buf.total_credits = total_credits;
	buf.g0_credit = g0_credit;
	buf.g1_credit = g1_credit;

	qdf_dp_add_record(code, QDF_TRACE_DEFAULT_PDEV_ID, (uint8_t *)&buf,
			  buf_size, NULL, 0, false);
}
qdf_export_symbol(qdf_dp_trace_credit_record);

void qdf_dp_display_event_record(struct qdf_dp_trace_record_s *record,
			      uint16_t index, uint8_t pdev_id, uint8_t info)
{
	char prepend_str[QDF_DP_TRACE_PREPEND_STR_SIZE];
	struct qdf_dp_trace_event_buf *buf =
		(struct qdf_dp_trace_event_buf *)record->data;

	qdf_mem_zero(prepend_str, sizeof(prepend_str));
	qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
				   index, info, record);

	DPTRACE_PRINT("%s [%d] [%s %s]",
		      prepend_str,
		      buf->vdev_id,
		      qdf_dp_type_to_str(buf->type),
		      qdf_dp_subtype_to_str(buf->subtype));
}
qdf_export_symbol(qdf_dp_display_event_record);

/**
 * qdf_dp_trace_record_event() - record events
 * @code: dptrace code
 * @vdev_id: vdev id
 * @pdev_id: pdev_id
 * @type: proto type
 * @subtype: proto subtype
 *
 * Return: none
 */
void qdf_dp_trace_record_event(enum QDF_DP_TRACE_ID code, uint8_t vdev_id,
		uint8_t pdev_id, enum qdf_proto_type type,
		enum qdf_proto_subtype subtype)
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
	qdf_dp_add_record(code, pdev_id,
			  (uint8_t *)&buf, buf_size, NULL, 0, true);
}
qdf_export_symbol(qdf_dp_trace_record_event);


void qdf_dp_display_proto_pkt(struct qdf_dp_trace_record_s *record,
			      uint16_t index, uint8_t pdev_id, uint8_t info)
{
	int loc;
	char prepend_str[QDF_DP_TRACE_PREPEND_STR_SIZE];
	struct qdf_dp_trace_proto_buf *buf =
		(struct qdf_dp_trace_proto_buf *)record->data;

	qdf_mem_zero(prepend_str, sizeof(prepend_str));
	loc = qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
					 index, info, record);
	DPTRACE_PRINT("%s [%d] [%s] SA: "
		      QDF_MAC_ADDR_FMT " %s DA: "
		      QDF_MAC_ADDR_FMT,
		      prepend_str,
		      buf->vdev_id,
		      qdf_dp_subtype_to_str(buf->subtype),
		      QDF_MAC_ADDR_REF(buf->sa.bytes),
		      qdf_dp_dir_to_str(buf->dir),
		      QDF_MAC_ADDR_REF(buf->da.bytes));
}
qdf_export_symbol(qdf_dp_display_proto_pkt);

void qdf_dp_trace_proto_pkt(enum QDF_DP_TRACE_ID code, uint8_t vdev_id,
		uint8_t *sa, uint8_t *da, enum qdf_proto_type type,
		enum qdf_proto_subtype subtype, enum qdf_proto_dir dir,
		uint8_t pdev_id, bool print)
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
	qdf_dp_add_record(code, pdev_id,
			  (uint8_t *)&buf, buf_size, NULL, 0, print);
}
qdf_export_symbol(qdf_dp_trace_proto_pkt);

void qdf_dp_display_ptr_record(struct qdf_dp_trace_record_s *record,
				uint16_t index, uint8_t pdev_id, uint8_t info)
{
	int loc;
	char prepend_str[QDF_DP_TRACE_PREPEND_STR_SIZE];
	struct qdf_dp_trace_ptr_buf *buf =
		(struct qdf_dp_trace_ptr_buf *)record->data;
	bool is_free_pkt_ptr_record = false;

	if ((record->code == QDF_DP_TRACE_FREE_PACKET_PTR_RECORD) ||
	    (record->code == QDF_DP_TRACE_LI_DP_FREE_PACKET_PTR_RECORD))
		is_free_pkt_ptr_record = true;

	qdf_mem_zero(prepend_str, sizeof(prepend_str));
	loc = qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
					 index, info, record);

	if (loc < sizeof(prepend_str))
		scnprintf(&prepend_str[loc], sizeof(prepend_str) - loc,
			  "[msdu id %d %s %d]",
			  buf->msdu_id,
			  is_free_pkt_ptr_record ? "status" : "vdev_id",
			  buf->status);

	if (info & QDF_DP_TRACE_RECORD_INFO_LIVE) {
		/* In live mode donot dump the contents of the cookie */
		DPTRACE_PRINT("%s", prepend_str);
	} else {
		dump_dp_hex_trace(prepend_str, (uint8_t *)&buf->cookie,
			sizeof(buf->cookie));
	}
}
qdf_export_symbol(qdf_dp_display_ptr_record);

static
enum qdf_proto_type qdf_dp_get_pkt_proto_type(qdf_nbuf_t nbuf)
{
	uint8_t pkt_type;

	if (!nbuf)
		return QDF_PROTO_TYPE_MAX;

	if (qdf_nbuf_data_is_dns_query(nbuf) ||
	    qdf_nbuf_data_is_dns_response(nbuf))
		return QDF_PROTO_TYPE_DNS;

	pkt_type = QDF_NBUF_CB_GET_PACKET_TYPE(nbuf);

	switch (pkt_type) {
	case QDF_NBUF_CB_PACKET_TYPE_EAPOL:
		return QDF_PROTO_TYPE_EAPOL;
	case QDF_NBUF_CB_PACKET_TYPE_ARP:
		return QDF_PROTO_TYPE_ARP;
	case QDF_NBUF_CB_PACKET_TYPE_DHCP:
		return QDF_PROTO_TYPE_DHCP;
	default:
		return QDF_PROTO_TYPE_MAX;
	}
}

static
enum qdf_proto_subtype qdf_dp_get_pkt_subtype(qdf_nbuf_t nbuf,
					      enum qdf_proto_type pkt_type)
{
	switch (pkt_type) {
	case QDF_PROTO_TYPE_EAPOL:
		return qdf_nbuf_get_eapol_subtype(nbuf);
	case QDF_PROTO_TYPE_ARP:
		return qdf_nbuf_get_arp_subtype(nbuf);
	case QDF_PROTO_TYPE_DHCP:
		return qdf_nbuf_get_dhcp_subtype(nbuf);
	case QDF_PROTO_TYPE_DNS:
		return (qdf_nbuf_data_is_dns_query(nbuf)) ?
				QDF_PROTO_DNS_QUERY : QDF_PROTO_DNS_RES;
	default:
		return QDF_PROTO_INVALID;
	}
}

static
bool qdf_dp_proto_log_enable_check(enum qdf_proto_type pkt_type,
				   uint16_t status)
{
	if (pkt_type == QDF_PROTO_TYPE_MAX)
		return false;

	switch (pkt_type) {
	case QDF_PROTO_TYPE_EAPOL:
		return qdf_dp_get_proto_event_bitmap() &
				QDF_NBUF_PKT_TRAC_TYPE_EAPOL;
	case QDF_PROTO_TYPE_DHCP:
		return qdf_dp_get_proto_event_bitmap() &
				QDF_NBUF_PKT_TRAC_TYPE_DHCP;
	case QDF_PROTO_TYPE_ARP:
		if (status == QDF_TX_RX_STATUS_OK)
			return false;
		else
			return qdf_dp_get_proto_event_bitmap() &
					QDF_NBUF_PKT_TRAC_TYPE_ARP;
	case QDF_PROTO_TYPE_DNS:
		if (status == QDF_TX_RX_STATUS_OK)
			return false;
		else
			return qdf_dp_get_proto_event_bitmap() &
					QDF_NBUF_PKT_TRAC_TYPE_DNS;
	default:
		return false;
	}
}

void qdf_dp_track_noack_check(qdf_nbuf_t nbuf, enum qdf_proto_subtype *subtype)
{
	enum qdf_proto_type pkt_type = qdf_dp_get_pkt_proto_type(nbuf);
	uint16_t dp_track = 0;

	switch (pkt_type) {
	case QDF_PROTO_TYPE_EAPOL:
		dp_track = qdf_dp_get_proto_bitmap() &
				QDF_NBUF_PKT_TRAC_TYPE_EAPOL;
		break;
	case QDF_PROTO_TYPE_DHCP:
		dp_track = qdf_dp_get_proto_bitmap() &
				QDF_NBUF_PKT_TRAC_TYPE_DHCP;
		break;
	case QDF_PROTO_TYPE_ARP:
		dp_track = qdf_dp_get_proto_bitmap() &
					QDF_NBUF_PKT_TRAC_TYPE_ARP;
		break;
	case QDF_PROTO_TYPE_DNS:
		dp_track = qdf_dp_get_proto_bitmap() &
					QDF_NBUF_PKT_TRAC_TYPE_DNS;
		break;
	default:
		break;
	}

	if (!dp_track) {
		*subtype = QDF_PROTO_INVALID;
		return;
	}

	*subtype = qdf_dp_get_pkt_subtype(nbuf, pkt_type);
}
qdf_export_symbol(qdf_dp_track_noack_check);

enum qdf_dp_tx_rx_status qdf_dp_get_status_from_a_status(uint8_t status)
{
	if (status == QDF_A_STATUS_ERROR)
		return QDF_TX_RX_STATUS_INVALID;
	else if (status == QDF_A_STATUS_OK)
		return QDF_TX_RX_STATUS_OK;
	else
		return QDF_TX_RX_STATUS_MAX;
}
qdf_export_symbol(qdf_dp_get_status_from_a_status);

void qdf_dp_trace_ptr(qdf_nbuf_t nbuf, enum QDF_DP_TRACE_ID code,
		uint8_t pdev_id, uint8_t *data, uint8_t size,
		uint16_t msdu_id, uint16_t buf_arg_status,
		enum qdf_dp_tx_rx_status qdf_tx_status)
{
	struct qdf_dp_trace_ptr_buf buf;
	int buf_size = sizeof(struct qdf_dp_trace_ptr_buf);
	enum qdf_proto_type pkt_type;

	pkt_type = qdf_dp_get_pkt_proto_type(nbuf);
	if ((code == QDF_DP_TRACE_FREE_PACKET_PTR_RECORD ||
	     code == QDF_DP_TRACE_LI_DP_FREE_PACKET_PTR_RECORD) &&
	    qdf_dp_proto_log_enable_check(pkt_type, qdf_tx_status))
		qdf_dp_log_proto_pkt_info(nbuf->data + QDF_NBUF_SRC_MAC_OFFSET,
					 nbuf->data + QDF_NBUF_DEST_MAC_OFFSET,
					 pkt_type,
					 qdf_dp_get_pkt_subtype(nbuf, pkt_type),
					 QDF_TX, msdu_id, qdf_tx_status);

	if (qdf_dp_enable_check(nbuf, code, QDF_TX) == false)
		return;

	if (buf_size > QDF_DP_TRACE_RECORD_SIZE)
		QDF_BUG(0);

	qdf_mem_copy(&buf.cookie, data, size);
	buf.msdu_id = msdu_id;
	buf.status = buf_arg_status;
	qdf_dp_add_record(code, pdev_id, (uint8_t *)&buf, buf_size, NULL, 0,
			  QDF_NBUF_CB_DP_TRACE_PRINT(nbuf));
}
qdf_export_symbol(qdf_dp_trace_ptr);

void qdf_dp_trace_data_pkt(qdf_nbuf_t nbuf, uint8_t pdev_id,
			   enum QDF_DP_TRACE_ID code, uint16_t msdu_id,
			   enum qdf_proto_dir dir)
{
	struct qdf_dp_trace_data_buf buf;
	enum qdf_proto_type pkt_type;

	pkt_type = qdf_dp_get_pkt_proto_type(nbuf);
	if (code == QDF_DP_TRACE_DROP_PACKET_RECORD &&
	    qdf_dp_proto_log_enable_check(pkt_type, QDF_TX_RX_STATUS_DROP))
		qdf_dp_log_proto_pkt_info(nbuf->data + QDF_NBUF_SRC_MAC_OFFSET,
					 nbuf->data + QDF_NBUF_DEST_MAC_OFFSET,
					 pkt_type,
					 qdf_dp_get_pkt_subtype(nbuf, pkt_type),
					 QDF_TX, msdu_id,
					 QDF_TX_RX_STATUS_DROP);

	buf.msdu_id = msdu_id;
	if (!qdf_dp_enable_check(nbuf, code, dir))
		return;

	qdf_dp_add_record(code, pdev_id,
			  nbuf ? qdf_nbuf_data(nbuf) : NULL,
			  nbuf ? nbuf->len - nbuf->data_len : 0,
			  (uint8_t *)&buf, sizeof(struct qdf_dp_trace_data_buf),
			  (nbuf) ? QDF_NBUF_CB_DP_TRACE_PRINT(nbuf) : false);
}

qdf_export_symbol(qdf_dp_trace_data_pkt);

void qdf_dp_display_record(struct qdf_dp_trace_record_s *record,
			   uint16_t index, uint8_t pdev_id, uint8_t info)
{
	int loc;
	char prepend_str[QDF_DP_TRACE_PREPEND_STR_SIZE];

	if (!(pdev_id == QDF_TRACE_DEFAULT_PDEV_ID ||
		pdev_id == record->pdev_id))
		return;

	qdf_mem_zero(prepend_str, sizeof(prepend_str));
	loc = qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
					 index, info, record);

	switch (record->code) {
	case  QDF_DP_TRACE_HDD_TX_TIMEOUT:
		DPTRACE_PRINT(" %s: HDD TX Timeout", prepend_str);
		break;
	case  QDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT:
		DPTRACE_PRINT(" %s: HDD SoftAP TX Timeout", prepend_str);
		break;
	case  QDF_DP_TRACE_CE_FAST_PACKET_ERR_RECORD:
		DPTRACE_PRINT(" %s: CE Fast Packet Error", prepend_str);
		break;
	case QDF_DP_TRACE_LI_DP_NULL_RX_PACKET_RECORD:
	default:
		dump_dp_hex_trace(prepend_str, record->data, record->size);
		break;
	};
}
qdf_export_symbol(qdf_dp_display_record);

void
qdf_dp_display_data_pkt_record(struct qdf_dp_trace_record_s *record,
			       uint16_t rec_index, uint8_t pdev_id,
			       uint8_t info)
{
	int loc;
	char prepend_str[DP_TRACE_META_DATA_STRLEN + 10];
	struct qdf_dp_trace_data_buf *buf =
		(struct qdf_dp_trace_data_buf *)record->data;

	qdf_mem_zero(prepend_str, sizeof(prepend_str));

	loc = qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
					 rec_index, info, record);
	if (loc < sizeof(prepend_str))
		loc += snprintf(&prepend_str[loc], sizeof(prepend_str) - loc,
				"[%d]", buf->msdu_id);
	dump_dp_hex_trace(prepend_str,
			  &record->data[sizeof(struct qdf_dp_trace_data_buf)],
			  record->size);
}

/**
 * qdf_dp_trace() - Stores the data in buffer
 * @nbuf  : defines the netbuf
 * @code : defines the event
 * @pdev_id: pdev_id
 * @data : defines the data to be stored
 * @size : defines the size of the data record
 *
 * Return: None
 */
void qdf_dp_trace(qdf_nbuf_t nbuf, enum QDF_DP_TRACE_ID code, uint8_t pdev_id,
	uint8_t *data, uint8_t size, enum qdf_proto_dir dir)
{

	if (qdf_dp_enable_check(nbuf, code, dir) == false)
		return;

	qdf_dp_add_record(code, pdev_id, nbuf ? qdf_nbuf_data(nbuf) : NULL,
			  size, NULL, 0,
			  (nbuf) ? QDF_NBUF_CB_DP_TRACE_PRINT(nbuf) : false);
}
qdf_export_symbol(qdf_dp_trace);

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
qdf_export_symbol(qdf_dp_trace_spin_lock_init);

/**
 * qdf_dp_trace_disable_live_mode - disable live mode for dptrace
 *
 * Return: none
 */
void qdf_dp_trace_disable_live_mode(void)
{
	g_qdf_dp_trace_data.force_live_mode = 0;
}
qdf_export_symbol(qdf_dp_trace_disable_live_mode);

/**
 * qdf_dp_trace_enable_live_mode() - enable live mode for dptrace
 *
 * Return: none
 */
void qdf_dp_trace_enable_live_mode(void)
{
	g_qdf_dp_trace_data.force_live_mode = 1;
}
qdf_export_symbol(qdf_dp_trace_enable_live_mode);

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
	g_qdf_dp_trace_data.dump_counter = 0;
	g_qdf_dp_trace_data.num_records_to_dump = MAX_QDF_DP_TRACE_RECORDS;
	if (g_qdf_dp_trace_data.enable)
		memset(g_qdf_dp_trace_tbl, 0,
		       MAX_QDF_DP_TRACE_RECORDS *
		       sizeof(struct qdf_dp_trace_record_s));
}
qdf_export_symbol(qdf_dp_trace_clear_buffer);

void qdf_dp_trace_dump_stats(void)
{
		DPTRACE_PRINT("STATS |DPT: tx %u rx %u icmp(%u %u) arp(%u %u) icmpv6(%u %u %u %u %u %u) dhcp(%u %u %u %u %u %u) eapol(%u %u %u %u %u)",
			      g_qdf_dp_trace_data.tx_count,
			      g_qdf_dp_trace_data.rx_count,
			      g_qdf_dp_trace_data.icmp_req,
			      g_qdf_dp_trace_data.icmp_resp,
			      g_qdf_dp_trace_data.arp_req,
			      g_qdf_dp_trace_data.arp_resp,
			      g_qdf_dp_trace_data.icmpv6_req,
			      g_qdf_dp_trace_data.icmpv6_resp,
			      g_qdf_dp_trace_data.icmpv6_ns,
			      g_qdf_dp_trace_data.icmpv6_na,
			      g_qdf_dp_trace_data.icmpv6_rs,
			      g_qdf_dp_trace_data.icmpv6_ra,
			      g_qdf_dp_trace_data.dhcp_disc,
			      g_qdf_dp_trace_data.dhcp_off,
			      g_qdf_dp_trace_data.dhcp_req,
			      g_qdf_dp_trace_data.dhcp_ack,
			      g_qdf_dp_trace_data.dhcp_nack,
			      g_qdf_dp_trace_data.dhcp_others,
			      g_qdf_dp_trace_data.eapol_m1,
			      g_qdf_dp_trace_data.eapol_m2,
			      g_qdf_dp_trace_data.eapol_m3,
			      g_qdf_dp_trace_data.eapol_m4,
			      g_qdf_dp_trace_data.eapol_others);
}
qdf_export_symbol(qdf_dp_trace_dump_stats);

/**
 * qdf_dpt_dump_hex_trace_debugfs() - read data in file
 * @file: file to read
 * @str: string to prepend the hexdump with.
 * @buf: buffer which contains data to be written
 * @buf_len: defines the size of the data to be written
 *
 * Return: None
 */
static void qdf_dpt_dump_hex_trace_debugfs(qdf_debugfs_file_t file,
				char *str, uint8_t *buf, uint8_t buf_len)
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

		qdf_debugfs_printf(file, "%s %s\n", str, linebuf);
	}
}

/**
 * qdf_dpt_display_proto_pkt_debugfs() - display proto packet
 * @file: file to read
 * @record: dptrace record
 * @index: index
 *
 * Return: none
 */
static void qdf_dpt_display_proto_pkt_debugfs(qdf_debugfs_file_t file,
				struct qdf_dp_trace_record_s *record,
				uint32_t index)
{
	int loc;
	char prepend_str[QDF_DP_TRACE_PREPEND_STR_SIZE];
	struct qdf_dp_trace_proto_buf *buf =
		(struct qdf_dp_trace_proto_buf *)record->data;

	loc = qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
					 index, 0, record);
	qdf_debugfs_printf(file, "%s [%d] [%s] SA: "
			   QDF_MAC_ADDR_FMT " %s DA: "
			   QDF_MAC_ADDR_FMT,
			   prepend_str,
			   buf->vdev_id,
			   qdf_dp_subtype_to_str(buf->subtype),
			   QDF_MAC_ADDR_REF(buf->sa.bytes),
			   qdf_dp_dir_to_str(buf->dir),
			   QDF_MAC_ADDR_REF(buf->da.bytes));
	qdf_debugfs_printf(file, "\n");
}

/**
 * qdf_dpt_display_mgmt_pkt_debugfs() - display mgmt packet
 * @file: file to read
 * @record: dptrace record
 * @index: index
 *
 * Return: none
 */
static void qdf_dpt_display_mgmt_pkt_debugfs(qdf_debugfs_file_t file,
				struct qdf_dp_trace_record_s *record,
				uint32_t index)
{

	int loc;
	char prepend_str[QDF_DP_TRACE_PREPEND_STR_SIZE];
	struct qdf_dp_trace_mgmt_buf *buf =
		(struct qdf_dp_trace_mgmt_buf *)record->data;

	loc = qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
					 index, 0, record);

	qdf_debugfs_printf(file, "%s [%d] [%s %s]\n",
			   prepend_str,
			   buf->vdev_id,
			   qdf_dp_type_to_str(buf->type),
			   qdf_dp_subtype_to_str(buf->subtype));
}

/**
 * qdf_dpt_display_event_record_debugfs() - display event records
 * @file: file to read
 * @record: dptrace record
 * @index: index
 *
 * Return: none
 */
static void qdf_dpt_display_event_record_debugfs(qdf_debugfs_file_t file,
				struct qdf_dp_trace_record_s *record,
				uint32_t index)
{
	char prepend_str[QDF_DP_TRACE_PREPEND_STR_SIZE];
	struct qdf_dp_trace_event_buf *buf =
		(struct qdf_dp_trace_event_buf *)record->data;

	qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
				   index, 0, record);
	qdf_debugfs_printf(file, "%s [%d] [%s %s]\n",
			   prepend_str,
			   buf->vdev_id,
			   qdf_dp_type_to_str(buf->type),
			   qdf_dp_subtype_to_str(buf->subtype));
}

/**
 * qdf_dpt_display_ptr_record_debugfs() - display record ptr
 * @file: file to read
 * @record: dptrace record
 * @index: index
 *
 * Return: none
 */
static void qdf_dpt_display_ptr_record_debugfs(qdf_debugfs_file_t file,
				struct qdf_dp_trace_record_s *record,
				uint32_t index)
{
	char prepend_str[QDF_DP_TRACE_PREPEND_STR_SIZE];
	int loc;
	struct qdf_dp_trace_ptr_buf *buf =
		(struct qdf_dp_trace_ptr_buf *)record->data;
	loc = qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
					 index, 0, record);

	if (loc < sizeof(prepend_str))
		scnprintf(&prepend_str[loc], sizeof(prepend_str) - loc,
			  "[msdu id %d %s %d]",
			  buf->msdu_id,
			  (record->code ==
				QDF_DP_TRACE_FREE_PACKET_PTR_RECORD) ?
			  "status" : "vdev_id",
			  buf->status);

	qdf_dpt_dump_hex_trace_debugfs(file, prepend_str,
				       (uint8_t *)&buf->cookie,
				       sizeof(buf->cookie));
}

/**
 * qdf_dpt_display_ptr_record_debugfs() - display record
 * @file: file to read
 * @record: dptrace record
 * @index: index
 *
 * Return: none
 */
static void qdf_dpt_display_record_debugfs(qdf_debugfs_file_t file,
				struct qdf_dp_trace_record_s *record,
				uint32_t index)
{
	int loc;
	char prepend_str[QDF_DP_TRACE_PREPEND_STR_SIZE];
	struct qdf_dp_trace_data_buf *buf =
		(struct qdf_dp_trace_data_buf *)record->data;

	loc = qdf_dp_trace_fill_meta_str(prepend_str, sizeof(prepend_str),
					 index, 0, record);
	if (loc < sizeof(prepend_str))
		loc += snprintf(&prepend_str[loc], sizeof(prepend_str) - loc,
				"[%d]", buf->msdu_id);
	qdf_dpt_dump_hex_trace_debugfs(file, prepend_str,
				       record->data, record->size);
}

uint32_t qdf_dpt_get_curr_pos_debugfs(qdf_debugfs_file_t file,
				      enum qdf_dpt_debugfs_state state)
{
	uint32_t i = 0;
	uint32_t tail;
	uint32_t count = g_qdf_dp_trace_data.num;

	if (!g_qdf_dp_trace_data.enable) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Tracing Disabled", __func__);
		return QDF_STATUS_E_EMPTY;
	}

	if (!count) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_DEBUG,
		  "%s: no packets", __func__);
		return QDF_STATUS_E_EMPTY;
	}

	if (state == QDF_DPT_DEBUGFS_STATE_SHOW_IN_PROGRESS)
		return g_qdf_dp_trace_data.curr_pos;

	qdf_debugfs_printf(file,
		"DPT: config - bitmap 0x%x verb %u #rec %u rec_requested %u live_config %u thresh %u time_limit %u\n",
		g_qdf_dp_trace_data.proto_bitmap,
		g_qdf_dp_trace_data.verbosity,
		g_qdf_dp_trace_data.no_of_record,
		g_qdf_dp_trace_data.num_records_to_dump,
		g_qdf_dp_trace_data.live_mode_config,
		g_qdf_dp_trace_data.high_tput_thresh,
		g_qdf_dp_trace_data.thresh_time_limit);

	qdf_debugfs_printf(file,
		"STATS |DPT: icmp(%u %u) arp(%u %u) icmpv6(%u %u %u %u %u %u) dhcp(%u %u %u %u %u %u) eapol(%u %u %u %u %u)\n",
		g_qdf_dp_trace_data.icmp_req,
		g_qdf_dp_trace_data.icmp_resp,
		g_qdf_dp_trace_data.arp_req,
		g_qdf_dp_trace_data.arp_resp,
		g_qdf_dp_trace_data.icmpv6_req,
		g_qdf_dp_trace_data.icmpv6_resp,
		g_qdf_dp_trace_data.icmpv6_ns,
		g_qdf_dp_trace_data.icmpv6_na,
		g_qdf_dp_trace_data.icmpv6_rs,
		g_qdf_dp_trace_data.icmpv6_ra,
		g_qdf_dp_trace_data.dhcp_disc,
		g_qdf_dp_trace_data.dhcp_off,
		g_qdf_dp_trace_data.dhcp_req,
		g_qdf_dp_trace_data.dhcp_ack,
		g_qdf_dp_trace_data.dhcp_nack,
		g_qdf_dp_trace_data.dhcp_others,
		g_qdf_dp_trace_data.eapol_m1,
		g_qdf_dp_trace_data.eapol_m2,
		g_qdf_dp_trace_data.eapol_m3,
		g_qdf_dp_trace_data.eapol_m4,
		g_qdf_dp_trace_data.eapol_others);

	qdf_debugfs_printf(file,
		"DPT: Total Records: %d, Head: %d, Tail: %d\n",
		g_qdf_dp_trace_data.num, g_qdf_dp_trace_data.head,
		g_qdf_dp_trace_data.tail);

	spin_lock_bh(&l_dp_trace_lock);
	if (g_qdf_dp_trace_data.head != INVALID_QDF_DP_TRACE_ADDR) {
		i = g_qdf_dp_trace_data.head;
		tail = g_qdf_dp_trace_data.tail;

		if (count > g_qdf_dp_trace_data.num)
			count = g_qdf_dp_trace_data.num;

		if (tail >= (count - 1))
			i = tail - count + 1;
		else if (count != MAX_QDF_DP_TRACE_RECORDS)
			i = MAX_QDF_DP_TRACE_RECORDS - ((count - 1) -
						     tail);
		g_qdf_dp_trace_data.curr_pos = 0;
		g_qdf_dp_trace_data.saved_tail = tail;
	}
	spin_unlock_bh(&l_dp_trace_lock);

	return g_qdf_dp_trace_data.saved_tail;
}
qdf_export_symbol(qdf_dpt_get_curr_pos_debugfs);

QDF_STATUS qdf_dpt_dump_stats_debugfs(qdf_debugfs_file_t file,
				      uint32_t curr_pos)
{
	struct qdf_dp_trace_record_s p_record;
	uint32_t i = curr_pos;
	uint16_t num_records_to_dump = g_qdf_dp_trace_data.num_records_to_dump;

	if (!g_qdf_dp_trace_data.enable) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Tracing Disabled", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (num_records_to_dump > g_qdf_dp_trace_data.num)
		num_records_to_dump = g_qdf_dp_trace_data.num;

	/*
	 * Max dp trace record size should always be less than
	 * QDF_DP_TRACE_PREPEND_STR_SIZE(100) + BUFFER_SIZE(121).
	 */
	if (WARN_ON(QDF_DP_TRACE_MAX_RECORD_SIZE <
				QDF_DP_TRACE_PREPEND_STR_SIZE + BUFFER_SIZE))
		return QDF_STATUS_E_FAILURE;

	spin_lock_bh(&l_dp_trace_lock);
	p_record = g_qdf_dp_trace_tbl[i];
	spin_unlock_bh(&l_dp_trace_lock);

	for (;; ) {
		/*
		 * Initially we get file as 1 page size, and
		 * if remaining size in file is less than one record max size,
		 * then return so that it gets an extra page.
		 */
		if ((file->size - file->count) < QDF_DP_TRACE_MAX_RECORD_SIZE) {
			spin_lock_bh(&l_dp_trace_lock);
			g_qdf_dp_trace_data.curr_pos = i;
			spin_unlock_bh(&l_dp_trace_lock);
			return QDF_STATUS_E_FAILURE;
		}

		switch (p_record.code) {
		case QDF_DP_TRACE_TXRX_PACKET_PTR_RECORD:
		case QDF_DP_TRACE_TXRX_FAST_PACKET_PTR_RECORD:
		case QDF_DP_TRACE_FREE_PACKET_PTR_RECORD:
			qdf_dpt_display_ptr_record_debugfs(file, &p_record, i);
			break;

		case QDF_DP_TRACE_EAPOL_PACKET_RECORD:
		case QDF_DP_TRACE_DHCP_PACKET_RECORD:
		case QDF_DP_TRACE_ARP_PACKET_RECORD:
		case QDF_DP_TRACE_ICMP_PACKET_RECORD:
		case QDF_DP_TRACE_ICMPv6_PACKET_RECORD:
			qdf_dpt_display_proto_pkt_debugfs(file, &p_record, i);
			break;

		case QDF_DP_TRACE_TX_CREDIT_RECORD:
			qdf_dpt_display_credit_record_debugfs(file, &p_record,
							      i);
			break;

		case QDF_DP_TRACE_MGMT_PACKET_RECORD:
			qdf_dpt_display_mgmt_pkt_debugfs(file, &p_record, i);
			break;

		case QDF_DP_TRACE_EVENT_RECORD:
			qdf_dpt_display_event_record_debugfs(file, &p_record,
							     i);
			break;

		case QDF_DP_TRACE_HDD_TX_TIMEOUT:
			qdf_debugfs_printf(
					file, "DPT: %04d: %llu %s\n",
					i, p_record.time,
					qdf_dp_code_to_string(p_record.code));
			qdf_debugfs_printf(file, "HDD TX Timeout\n");
			break;

		case QDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT:
			qdf_debugfs_printf(
					file, "DPT: %04d: %llu %s\n",
					i, p_record.time,
					qdf_dp_code_to_string(p_record.code));
			qdf_debugfs_printf(file, "HDD SoftAP TX Timeout\n");
			break;

		case QDF_DP_TRACE_CE_FAST_PACKET_ERR_RECORD:
			qdf_debugfs_printf(
					file, "DPT: %04d: %llu %s\n",
					i, p_record.time,
					qdf_dp_code_to_string(p_record.code));
			qdf_debugfs_printf(file, "CE Fast Packet Error\n");
			break;

		case QDF_DP_TRACE_MAX:
			qdf_debugfs_printf(file,
				"%s: QDF_DP_TRACE_MAX event should not be generated\n",
				__func__);
			break;

		case QDF_DP_TRACE_HDD_TX_PACKET_RECORD:
		case QDF_DP_TRACE_HDD_RX_PACKET_RECORD:
		case QDF_DP_TRACE_TX_PACKET_RECORD:
		case QDF_DP_TRACE_RX_PACKET_RECORD:
		case QDF_DP_TRACE_LI_DP_TX_PACKET_RECORD:
		case QDF_DP_TRACE_LI_DP_RX_PACKET_RECORD:

		default:
			qdf_dpt_display_record_debugfs(file, &p_record, i);
			break;
		}

		if (++g_qdf_dp_trace_data.dump_counter == num_records_to_dump)
			break;

		spin_lock_bh(&l_dp_trace_lock);
		if (i == 0)
			i = MAX_QDF_DP_TRACE_RECORDS;

		i -= 1;
		p_record = g_qdf_dp_trace_tbl[i];
		spin_unlock_bh(&l_dp_trace_lock);
	}

	g_qdf_dp_trace_data.dump_counter = 0;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_dpt_dump_stats_debugfs);

/**
 * qdf_dpt_set_value_debugfs() - Configure the value to control DP trace
 * @proto_bitmap: defines the protocol to be tracked
 * @no_of_records: defines the nth packet which is traced
 * @verbosity: defines the verbosity level
 *
 * Return: None
 */
void qdf_dpt_set_value_debugfs(uint8_t proto_bitmap, uint8_t no_of_record,
			    uint8_t verbosity, uint16_t num_records_to_dump)
{
	if (g_qdf_dp_trace_data.enable) {
		g_qdf_dp_trace_data.proto_bitmap = proto_bitmap;
		g_qdf_dp_trace_data.no_of_record = no_of_record;
		g_qdf_dp_trace_data.verbosity    = verbosity;
		g_qdf_dp_trace_data.num_records_to_dump = num_records_to_dump;
	}
}
qdf_export_symbol(qdf_dpt_set_value_debugfs);


/**
 * qdf_dp_trace_dump_all() - Dump data from ring buffer via call back functions
 * registered with QDF
 * @count: Number of lines to dump starting from tail to head
 * @pdev_id: pdev_id
 *
 * Return: None
 */
void qdf_dp_trace_dump_all(uint32_t count, uint8_t pdev_id)
{
	struct qdf_dp_trace_record_s p_record;
	int32_t i, tail;

	if (!g_qdf_dp_trace_data.enable) {
		DPTRACE_PRINT("Tracing Disabled");
		return;
	}

	DPTRACE_PRINT(
		"DPT: config - bitmap 0x%x verb %u #rec %u live_config %u thresh %u time_limit %u",
		g_qdf_dp_trace_data.proto_bitmap,
		g_qdf_dp_trace_data.verbosity,
		g_qdf_dp_trace_data.no_of_record,
		g_qdf_dp_trace_data.live_mode_config,
		g_qdf_dp_trace_data.high_tput_thresh,
		g_qdf_dp_trace_data.thresh_time_limit);

	qdf_dp_trace_dump_stats();

	DPTRACE_PRINT("DPT: Total Records: %d, Head: %d, Tail: %d",
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
			qdf_dp_trace_cb_table[p_record.code](&p_record,
							(uint16_t)i, pdev_id, false);
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
qdf_export_symbol(qdf_dp_trace_dump_all);

/**
 * qdf_dp_trace_throttle_live_mode() - Throttle DP Trace live mode
 * @high_bw_request: whether this is a high BW req or not
 *
 * The function tries to prevent excessive logging into the live buffer by
 * having an upper limit on number of packets that can be logged per second.
 *
 * The intention is to allow occasional pings and data packets and really low
 * throughput levels while suppressing bursts and higher throughput levels so
 * that we donot hog the live buffer.
 *
 * If the number of packets printed in a particular second exceeds the thresh,
 * disable printing in the next second.
 *
 * Return: None
 */
void qdf_dp_trace_throttle_live_mode(bool high_bw_request)
{
	static int bw_interval_counter;

	if (g_qdf_dp_trace_data.enable == false ||
		g_qdf_dp_trace_data.live_mode_config == false)
		return;

	if (high_bw_request) {
		g_qdf_dp_trace_data.live_mode = 0;
		bw_interval_counter = 0;
		return;
	}

	bw_interval_counter++;

	if (0 == (bw_interval_counter %
			g_qdf_dp_trace_data.thresh_time_limit)) {

		spin_lock_bh(&l_dp_trace_lock);
			if (g_qdf_dp_trace_data.print_pkt_cnt <=
				g_qdf_dp_trace_data.high_tput_thresh)
				g_qdf_dp_trace_data.live_mode = 1;

		g_qdf_dp_trace_data.print_pkt_cnt = 0;
		spin_unlock_bh(&l_dp_trace_lock);
	}
}
qdf_export_symbol(qdf_dp_trace_throttle_live_mode);

void qdf_dp_trace_apply_tput_policy(bool is_data_traffic)
{
	if (g_qdf_dp_trace_data.dynamic_verbosity_modify) {
		goto check_live_mode;
		return;
	}

	if (is_data_traffic) {
		g_qdf_dp_trace_data.verbosity =
					QDF_DP_TRACE_VERBOSITY_ULTRA_LOW;
	} else {
		g_qdf_dp_trace_data.verbosity =
					g_qdf_dp_trace_data.ini_conf_verbosity;
	}
check_live_mode:
	qdf_dp_trace_throttle_live_mode(is_data_traffic);
}
#endif

struct qdf_print_ctrl print_ctrl_obj[MAX_PRINT_CONFIG_SUPPORTED];

struct category_name_info g_qdf_category_name[MAX_SUPPORTED_CATEGORY] = {
	[QDF_MODULE_ID_TDLS] = {"tdls"},
	[QDF_MODULE_ID_ACS] = {"ACS"},
	[QDF_MODULE_ID_SCAN_SM] = {"scan state machine"},
	[QDF_MODULE_ID_SCANENTRY] = {"scan entry"},
	[QDF_MODULE_ID_WDS] = {"WDS"},
	[QDF_MODULE_ID_ACTION] = {"action"},
	[QDF_MODULE_ID_ROAM] = {"STA roaming"},
	[QDF_MODULE_ID_INACT] = {"inactivity"},
	[QDF_MODULE_ID_DOTH] = {"11h"},
	[QDF_MODULE_ID_IQUE] = {"IQUE"},
	[QDF_MODULE_ID_WME] = {"WME"},
	[QDF_MODULE_ID_ACL] = {"ACL"},
	[QDF_MODULE_ID_WPA] = {"WPA/RSN"},
	[QDF_MODULE_ID_RADKEYS] = {"dump 802.1x keys"},
	[QDF_MODULE_ID_RADDUMP] = {"dump radius packet"},
	[QDF_MODULE_ID_RADIUS] = {"802.1x radius client"},
	[QDF_MODULE_ID_DOT1XSM] = {"802.1x state machine"},
	[QDF_MODULE_ID_DOT1X] = {"802.1x authenticator"},
	[QDF_MODULE_ID_POWER] = {"power save"},
	[QDF_MODULE_ID_STATE] = {"state"},
	[QDF_MODULE_ID_OUTPUT] = {"output"},
	[QDF_MODULE_ID_SCAN] = {"scan"},
	[QDF_MODULE_ID_AUTH] = {"authentication"},
	[QDF_MODULE_ID_ASSOC] = {"association"},
	[QDF_MODULE_ID_NODE] = {"node"},
	[QDF_MODULE_ID_ELEMID] = {"element ID"},
	[QDF_MODULE_ID_XRATE] = {"rate"},
	[QDF_MODULE_ID_INPUT] = {"input"},
	[QDF_MODULE_ID_CRYPTO] = {"crypto"},
	[QDF_MODULE_ID_DUMPPKTS] = {"dump packet"},
	[QDF_MODULE_ID_DEBUG] = {"debug"},
	[QDF_MODULE_ID_MLME] = {"mlme"},
	[QDF_MODULE_ID_RRM] = {"rrm"},
	[QDF_MODULE_ID_WNM] = {"wnm"},
	[QDF_MODULE_ID_P2P_PROT] = {"p2p_prot"},
	[QDF_MODULE_ID_PROXYARP] = {"proxyarp"},
	[QDF_MODULE_ID_L2TIF] = {"l2tif"},
	[QDF_MODULE_ID_WIFIPOS] = {"wifipos"},
	[QDF_MODULE_ID_WRAP] = {"wrap"},
	[QDF_MODULE_ID_DFS] = {"dfs"},
	[QDF_MODULE_ID_ATF] = {"atf"},
	[QDF_MODULE_ID_SPLITMAC] = {"splitmac"},
	[QDF_MODULE_ID_IOCTL] = {"ioctl"},
	[QDF_MODULE_ID_NAC] = {"nac"},
	[QDF_MODULE_ID_MESH] = {"mesh"},
	[QDF_MODULE_ID_MBO] = {"mbo"},
	[QDF_MODULE_ID_EXTIOCTL_CHANSWITCH] = {"extchanswitch"},
	[QDF_MODULE_ID_EXTIOCTL_CHANSSCAN] = {"extchanscan"},
	[QDF_MODULE_ID_TLSHIM] = {"tlshim"},
	[QDF_MODULE_ID_WMI] = {"WMI"},
	[QDF_MODULE_ID_HTT] = {"HTT"},
	[QDF_MODULE_ID_HDD] = {"HDD"},
	[QDF_MODULE_ID_SME] = {"SME"},
	[QDF_MODULE_ID_PE] = {"PE"},
	[QDF_MODULE_ID_WMA] = {"WMA"},
	[QDF_MODULE_ID_SYS] = {"SYS"},
	[QDF_MODULE_ID_QDF] = {"QDF"},
	[QDF_MODULE_ID_SAP] = {"SAP"},
	[QDF_MODULE_ID_HDD_SOFTAP] = {"HDD_SAP"},
	[QDF_MODULE_ID_HDD_DATA] = {"DATA"},
	[QDF_MODULE_ID_HDD_SAP_DATA] = {"SAP_DATA"},
	[QDF_MODULE_ID_HIF] = {"HIF"},
	[QDF_MODULE_ID_HTC] = {"HTC"},
	[QDF_MODULE_ID_TXRX] = {"TXRX"},
	[QDF_MODULE_ID_QDF_DEVICE] = {"QDF_DEV"},
	[QDF_MODULE_ID_CFG] = {"CFG"},
	[QDF_MODULE_ID_BMI] = {"BMI"},
	[QDF_MODULE_ID_EPPING] = {"EPPING"},
	[QDF_MODULE_ID_QVIT] = {"QVIT"},
	[QDF_MODULE_ID_DP] = {"DP"},
	[QDF_MODULE_ID_HAL] = {"HAL"},
	[QDF_MODULE_ID_SOC] = {"SOC"},
	[QDF_MODULE_ID_OS_IF] = {"OSIF"},
	[QDF_MODULE_ID_TARGET_IF] = {"TIF"},
	[QDF_MODULE_ID_SCHEDULER] = {"SCH"},
	[QDF_MODULE_ID_MGMT_TXRX] = {"MGMT_TXRX"},
	[QDF_MODULE_ID_PMO] = {"PMO"},
	[QDF_MODULE_ID_POLICY_MGR] = {"POLICY_MGR"},
	[QDF_MODULE_ID_SA_API] = {"SA_API"},
	[QDF_MODULE_ID_NAN] = {"NAN"},
	[QDF_MODULE_ID_SPECTRAL] = {"SPECTRAL"},
	[QDF_MODULE_ID_P2P] = {"P2P"},
	[QDF_MODULE_ID_OFFCHAN_TXRX] = {"OFFCHAN"},
	[QDF_MODULE_ID_REGULATORY] = {"REGULATORY"},
	[QDF_MODULE_ID_OBJ_MGR] = {"OBJMGR"},
	[QDF_MODULE_ID_SERIALIZATION] = {"SER"},
	[QDF_MODULE_ID_NSS] = {"NSS"},
	[QDF_MODULE_ID_ROAM_DEBUG] = {"roam debug"},
	[QDF_MODULE_ID_DIRECT_BUF_RX] = {"DIRECT_BUF_RX"},
	[QDF_MODULE_ID_DISA] = {"disa"},
	[QDF_MODULE_ID_GREEN_AP] = {"GREEN_AP"},
	[QDF_MODULE_ID_EXTAP] = {"EXTAP"},
	[QDF_MODULE_ID_FD] = {"FILS discovery"},
	[QDF_MODULE_ID_FTM] = {"FTM"},
	[QDF_MODULE_ID_OCB] = {"OCB"},
	[QDF_MODULE_ID_CONFIG] = {"CONFIG"},
	[QDF_MODULE_ID_IPA] = {"IPA"},
	[QDF_MODULE_ID_CP_STATS] = {"CP_STATS"},
	[QDF_MODULE_ID_DCS] = {"DCS"},
	[QDF_MODULE_ID_ACTION_OUI] = {"action_oui"},
	[QDF_MODULE_ID_TARGET] = {"TARGET"},
	[QDF_MODULE_ID_MBSSIE] = {"MBSSIE"},
	[QDF_MODULE_ID_FWOL] = {"fwol"},
	[QDF_MODULE_ID_SM_ENGINE] = {"SM_ENG"},
	[QDF_MODULE_ID_CMN_MLME] = {"CMN_MLME"},
	[QDF_MODULE_ID_BSSCOLOR] = {"BSSCOLOR"},
	[QDF_MODULE_ID_CFR] = {"CFR"},
	[QDF_MODULE_ID_TX_CAPTURE] = {"TX_CAPTURE_ENHANCE"},
	[QDF_MODULE_ID_INTEROP_ISSUES_AP] = {"INTEROP_ISSUES_AP"},
	[QDF_MODULE_ID_BLACKLIST_MGR] = {"blm"},
	[QDF_MODULE_ID_QLD] = {"QLD"},
	[QDF_MODULE_ID_DYNAMIC_MODE_CHG] = {"Dynamic Mode Change"},
	[QDF_MODULE_ID_COEX] = {"COEX"},
	[QDF_MODULE_ID_MON_FILTER] = {"Monitor Filter"},
	[QDF_MODULE_ID_ANY] = {"ANY"},
	[QDF_MODULE_ID_PKT_CAPTURE] = {"pkt_capture"},
	[QDF_MODULE_ID_RPTR] = {"RPTR"},
	[QDF_MODULE_ID_6GHZ] = {"6GHZ"},
	[QDF_MODULE_ID_IOT_SIM] = {"IOT_SIM"},
	[QDF_MODULE_ID_MSCS] = {"MSCS"},
	[QDF_MODULE_ID_GPIO] = {"GPIO_CFG"},
	[QDF_MODULE_ID_IFMGR] = {"IF_MGR"},
	[QDF_MODULE_ID_DIAG] = {"DIAG"},
};
qdf_export_symbol(g_qdf_category_name);

/**
 * qdf_trace_display() - Display trace
 *
 * Return:  None
 */
void qdf_trace_display(void)
{
	QDF_MODULE_ID module_id;

	pr_err("     1)FATAL  2)ERROR  3)WARN  4)INFO  5)INFO_H  6)INFO_M  7)INFO_L 8)DEBUG\n");
	for (module_id = 0; module_id < QDF_MODULE_ID_MAX; ++module_id) {
		pr_err("%2d)%s    %s        %s       %s       %s        %s         %s         %s        %s\n",
		       (int)module_id,
		       g_qdf_category_name[module_id].category_name_str,
		       qdf_print_is_verbose_enabled(qdf_pidx, module_id,
			       QDF_TRACE_LEVEL_FATAL) ? "X" : " ",
		       qdf_print_is_verbose_enabled(qdf_pidx, module_id,
			       QDF_TRACE_LEVEL_ERROR) ? "X" : " ",
		       qdf_print_is_verbose_enabled(qdf_pidx, module_id,
			       QDF_TRACE_LEVEL_WARN) ? "X" : " ",
		       qdf_print_is_verbose_enabled(qdf_pidx, module_id,
			       QDF_TRACE_LEVEL_INFO) ? "X" : " ",
		       qdf_print_is_verbose_enabled(qdf_pidx, module_id,
			       QDF_TRACE_LEVEL_INFO_HIGH) ? "X" : " ",
		       qdf_print_is_verbose_enabled(qdf_pidx, module_id,
			       QDF_TRACE_LEVEL_INFO_MED) ? "X" : " ",
		       qdf_print_is_verbose_enabled(qdf_pidx, module_id,
			       QDF_TRACE_LEVEL_INFO_LOW) ? "X" : " ",
		       qdf_print_is_verbose_enabled(qdf_pidx, module_id,
			       QDF_TRACE_LEVEL_DEBUG) ? "X" : " ");
	}
}
qdf_export_symbol(qdf_trace_display);

#ifdef WLAN_MAX_LOGS_PER_SEC
static qdf_time_t __log_window_end;
static qdf_atomic_t __log_window_count;
uint32_t qdf_rl_print_count = WLAN_MAX_LOGS_PER_SEC;
uint32_t qdf_rl_print_time = 1;
uint32_t qdf_rl_print_supressed;

/**
 * qdf_detected_excessive_logging() - Excessive logging detected
 *
 * Track logging count using a quasi-tumbling window.
 * If the max logging count for a given window is exceeded,
 * return true else fails.
 *
 * Return: true/false
 */
bool qdf_detected_excessive_logging(void)
{
	qdf_time_t now = qdf_system_ticks();
	bool excessive_prints = false;

	/*
	 * If 'now' is more recent than the end of the window, reset.
	 *
	 * Note: This is not thread safe, and can result in more than one reset.
	 * For our purposes, this is fine.
	 */
	if (!qdf_atomic_read(&__log_window_count)) {
		__log_window_end = now + (qdf_system_ticks_per_sec * qdf_rl_print_time);
	} else if (qdf_system_time_after(now, __log_window_end)) {
		__log_window_end = now + (qdf_system_ticks_per_sec * qdf_rl_print_time);
		qdf_atomic_set(&__log_window_count, 0);
	}

	if (qdf_atomic_inc_return(&__log_window_count) > qdf_rl_print_count)
		excessive_prints = true;

	return excessive_prints;
}

void qdf_rl_print_count_set(uint32_t rl_print_count)
{
	qdf_rl_print_count = rl_print_count;
}

qdf_export_symbol(qdf_rl_print_count_set);

void qdf_rl_print_time_set(uint32_t rl_print_time)
{
	qdf_rl_print_time = rl_print_time;
}

qdf_export_symbol(qdf_rl_print_time_set);

static inline void qdf_rl_print_supressed_log(void)
{
	if (qdf_rl_print_supressed) {
		pr_err("QDF Ratelimiting: %d prints supressed",
		       qdf_rl_print_supressed);
		qdf_rl_print_supressed = 0;
	}
}

static inline void qdf_rl_print_supressed_inc(void)
{
	qdf_rl_print_supressed++;
}
#else
#define qdf_rl_print_supressed_log()
#define qdf_rl_print_supressed_inc()
#endif /* WLAN_MAX_LOGS_PER_SEC */

#ifdef QDF_TRACE_PRINT_ENABLE
static inline void print_to_console(char *str_buffer)
{
	if (qdf_detected_excessive_logging()) {
		qdf_rl_print_supressed_inc();
		return;
	}
	qdf_rl_print_supressed_log();
	pr_err("%s\n", str_buffer);
}
#else

#define print_to_console(str)
#endif

#ifdef MULTI_IF_NAME
static const char *qdf_trace_wlan_modname(void)
{
	return MULTI_IF_NAME;
}
#else
static const char *qdf_trace_wlan_modname(void)
{
	return "wlan";
}
#endif

void qdf_trace_msg_cmn(unsigned int idx,
			QDF_MODULE_ID category,
			QDF_TRACE_LEVEL verbose,
			const char *str_format, va_list val)
{
	char str_buffer[QDF_TRACE_BUFFER_SIZE];
	int n;

	/* Check if index passed is valid */
	if (idx < 0 || idx >= MAX_PRINT_CONFIG_SUPPORTED) {
		pr_info("%s: Invalid index - %d\n", __func__, idx);
		return;
	}

	/* Check if print control object is in use */
	if (!print_ctrl_obj[idx].in_use) {
		pr_info("%s: Invalid print control object\n", __func__);
		return;
	}

	/* Check if category passed is valid */
	if (category < 0 || category >= MAX_SUPPORTED_CATEGORY) {
		vscnprintf(str_buffer, QDF_TRACE_BUFFER_SIZE, str_format, val);
		pr_info("%s: Invalid category: %d, log: %s\n",
			__func__, category, str_buffer);
		return;
	}

	/* Check if verbose mask is valid */
	if (verbose < 0 || verbose >= QDF_TRACE_LEVEL_MAX) {
		vscnprintf(str_buffer, QDF_TRACE_BUFFER_SIZE, str_format, val);
		pr_info("%s: Invalid verbose level %d, log: %s\n",
			__func__, verbose, str_buffer);
		return;
	}

	/*
	 * Print the trace message when the desired verbose level is set in
	 * the desired category for the print control object
	 */
	if (print_ctrl_obj[idx].cat_info[category].category_verbose_mask &
	    QDF_TRACE_LEVEL_TO_MODULE_BITMASK(verbose)) {
		static const char * const VERBOSE_STR[] = {
			[QDF_TRACE_LEVEL_NONE] = "",
			[QDF_TRACE_LEVEL_FATAL] = "F",
			[QDF_TRACE_LEVEL_ERROR] = "E",
			[QDF_TRACE_LEVEL_WARN] = "W",
			[QDF_TRACE_LEVEL_INFO] = "I",
			[QDF_TRACE_LEVEL_INFO_HIGH] = "IH",
			[QDF_TRACE_LEVEL_INFO_MED] = "IM",
			[QDF_TRACE_LEVEL_INFO_LOW] = "IL",
			[QDF_TRACE_LEVEL_DEBUG] = "D",
			[QDF_TRACE_LEVEL_TRACE] = "T",
			[QDF_TRACE_LEVEL_ALL] = "" };

		/* print the prefix string into the string buffer... */
		n = scnprintf(str_buffer, QDF_TRACE_BUFFER_SIZE,
			     "%s: [%d:%s:%s] ", qdf_trace_wlan_modname(),
			     in_interrupt() ? 0 : current->pid,
			     VERBOSE_STR[verbose],
			     g_qdf_category_name[category].category_name_str);

		/* print the formatted log message after the prefix string */
		vscnprintf(str_buffer + n, QDF_TRACE_BUFFER_SIZE - n,
			   str_format, val);
#if defined(WLAN_LOGGING_SOCK_SVC_ENABLE)
		wlan_log_to_user(verbose, (char *)str_buffer,
				 strlen(str_buffer));
		if (qdf_unlikely(qdf_log_dump_at_kernel_enable))
			print_to_console(str_buffer);
#else
		pr_err("%s\n", str_buffer);
#endif
	}
}
qdf_export_symbol(qdf_trace_msg_cmn);

QDF_STATUS qdf_print_setup(void)
{
	int i;

	/* Loop through all print ctrl objects */
	for (i = 0; i < MAX_PRINT_CONFIG_SUPPORTED; i++) {
		if (qdf_print_ctrl_cleanup(i))
			return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_print_setup);

QDF_STATUS qdf_print_ctrl_cleanup(unsigned int idx)
{
	int i = 0;

	if (idx < 0 || idx >= MAX_PRINT_CONFIG_SUPPORTED) {
		pr_info("%s: Invalid index - %d\n", __func__, idx);
		return QDF_STATUS_E_FAILURE;
	}

	/* Clean up the print control object corresponding to that index
	 * If success, callee to change print control index to -1
	 */

	for (i = 0; i < MAX_SUPPORTED_CATEGORY; i++) {
		print_ctrl_obj[idx].cat_info[i].category_verbose_mask =
							QDF_TRACE_LEVEL_NONE;
	}
	print_ctrl_obj[idx].custom_print = NULL;
	print_ctrl_obj[idx].custom_ctxt = NULL;
	qdf_print_clean_node_flag(idx);
	print_ctrl_obj[idx].in_use = false;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_print_ctrl_cleanup);

int qdf_print_ctrl_register(const struct category_info *cinfo,
			    void *custom_print_handler,
			    void *custom_ctx,
			    const char *pctrl_name)
{
	int idx = -1;
	int i = 0;

	for (i = 0; i < MAX_PRINT_CONFIG_SUPPORTED; i++) {
		if (!print_ctrl_obj[i].in_use) {
			idx = i;
			break;
		}
	}

	/* Callee to handle idx -1 appropriately */
	if (idx == -1) {
		pr_info("%s: Allocation failed! No print control object free\n",
			__func__);
		return idx;
	}

	print_ctrl_obj[idx].in_use = true;

	/*
	 * In case callee does not pass category info,
	 * custom print handler, custom context and print control name,
	 * we do not set any value here. Clean up for the print control
	 * getting allocated would have taken care of initializing
	 * default values.
	 *
	 * We need to only set in_use to 1 in such a case
	 */

	if (pctrl_name) {
		qdf_str_lcopy(print_ctrl_obj[idx].name, pctrl_name,
			      sizeof(print_ctrl_obj[idx].name));
	}

	if (custom_print_handler)
		print_ctrl_obj[idx].custom_print = custom_print_handler;

	if (custom_ctx)
		print_ctrl_obj[idx].custom_ctxt = custom_ctx;

	if (cinfo) {
		for (i = 0; i < MAX_SUPPORTED_CATEGORY; i++) {
			if (cinfo[i].category_verbose_mask ==
			    QDF_TRACE_LEVEL_ALL) {
				print_ctrl_obj[idx].cat_info[i]
				.category_verbose_mask = 0xFFFF;
			} else if ((cinfo[i].category_verbose_mask ==
				   QDF_TRACE_LEVEL_NONE) ||
				   (cinfo[i].category_verbose_mask ==
				   QDF_TRACE_LEVEL_TO_MODULE_BITMASK(
				   QDF_TRACE_LEVEL_NONE))) {
				print_ctrl_obj[idx].cat_info[i]
				.category_verbose_mask = 0;
			} else {
				print_ctrl_obj[idx].cat_info[i]
				.category_verbose_mask =
				cinfo[i].category_verbose_mask;
			}
		}
	}

	return idx;
}
qdf_export_symbol(qdf_print_ctrl_register);

#ifdef QDF_TRACE_PRINT_ENABLE
void qdf_shared_print_ctrl_cleanup(void)
{
	qdf_print_ctrl_cleanup(qdf_pidx);
}
qdf_export_symbol(qdf_shared_print_ctrl_cleanup);

/*
 * Set this to invalid value to differentiate with user-provided
 * value.
 */
int qdf_dbg_mask = QDF_TRACE_LEVEL_MAX;
qdf_export_symbol(qdf_dbg_mask);
qdf_declare_param(qdf_dbg_mask, int);

/*
 * QDF can be passed parameters which indicate the
 * debug level for each module.
 * an array of string values are passed, each string hold the following form
 *
 * <module name string>=<integer debug level value>
 *
 * The array qdf_dbg_arr will hold these module-string=value strings
 * The variable qdf_dbg_arr_cnt will have the count of how many such
 * string values were passed.
 */
static char *qdf_dbg_arr[QDF_MODULE_ID_MAX];
static int qdf_dbg_arr_cnt;
qdf_declare_param_array(qdf_dbg_arr, charp, &qdf_dbg_arr_cnt);

static uint16_t set_cumulative_verbose_mask(QDF_TRACE_LEVEL max_level)
{
	uint16_t category_verbose_mask = 0;
	QDF_TRACE_LEVEL level;

	for (level = QDF_TRACE_LEVEL_FATAL; level <= max_level; level++) {
		category_verbose_mask |=
			QDF_TRACE_LEVEL_TO_MODULE_BITMASK(level);
	}
	return category_verbose_mask;
}

static QDF_MODULE_ID find_qdf_module_from_string(char *str)
{
	QDF_MODULE_ID mod_id;

	for (mod_id = 0; mod_id < QDF_MODULE_ID_MAX; mod_id++) {
		if (strcasecmp(str,
				g_qdf_category_name[mod_id].category_name_str)
				== 0) {
			break;
		}
	}
	return mod_id;
}

static void process_qdf_dbg_arr_param(struct category_info *cinfo,
					int array_index)
{
	char *mod_val_str, *mod_str, *val_str;
	unsigned long dbg_level;
	QDF_MODULE_ID mod_id;

	mod_val_str = qdf_dbg_arr[array_index];
	mod_str = strsep(&mod_val_str, "=");
	val_str = mod_val_str;
	if (!val_str) {
		pr_info("qdf_dbg_arr: %s not in the <mod>=<val> form\n",
				mod_str);
		return;
	}

	mod_id = find_qdf_module_from_string(mod_str);
	if (mod_id >= QDF_MODULE_ID_MAX) {
		pr_info("ERROR!!Module name %s not in the list of modules\n",
				mod_str);
		return;
	}

	if (kstrtol(val_str, 10, &dbg_level) < 0) {
		pr_info("ERROR!!Invalid debug level for module: %s\n",
				mod_str);
		return;
	}

	if (dbg_level >= QDF_TRACE_LEVEL_MAX) {
		pr_info("ERROR!!Debug level for %s too high", mod_str);
		pr_info("max: %d given %lu\n", QDF_TRACE_LEVEL_MAX,
				dbg_level);
		return;
	}

	pr_info("User passed setting module %s(%d) to level %lu\n",
			mod_str,
			mod_id,
			dbg_level);
	cinfo[mod_id].category_verbose_mask =
		set_cumulative_verbose_mask((QDF_TRACE_LEVEL)dbg_level);
}

static void set_default_trace_levels(struct category_info *cinfo)
{
	int i;
	static QDF_TRACE_LEVEL module_trace_default_level[QDF_MODULE_ID_MAX] = {
		[QDF_MODULE_ID_TDLS] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_ACS] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_SCAN_SM] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_SCANENTRY] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_WDS] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_ACTION] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_ROAM] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_INACT] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_DOTH] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_IQUE] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_WME] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_ACL] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_WPA] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_RADKEYS] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_RADDUMP] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_RADIUS] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_DOT1XSM] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_DOT1X] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_POWER] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_STATE] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_OUTPUT] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_SCAN] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_AUTH] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_ASSOC] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_NODE] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_ELEMID] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_XRATE] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_INPUT] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_CRYPTO] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_DUMPPKTS] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_DEBUG] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_MLME] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_RRM] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_WNM] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_P2P_PROT] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_PROXYARP] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_L2TIF] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_WIFIPOS] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_WRAP] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_DFS] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_ATF] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_SPLITMAC] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_IOCTL] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_NAC] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_MESH] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_MBO] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_EXTIOCTL_CHANSWITCH] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_EXTIOCTL_CHANSSCAN] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_TLSHIM] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_WMI] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_HTT] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_HDD] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_SME] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_PE] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_WMA] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_SYS] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_QDF] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_SAP] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_HDD_SOFTAP] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_HDD_DATA] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_HDD_SAP_DATA] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_HIF] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_HTC] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_TXRX] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_QDF_DEVICE] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_CFG] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_BMI] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_EPPING] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_QVIT] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_DP] = QDF_TRACE_LEVEL_FATAL,
		[QDF_MODULE_ID_HAL] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_SOC] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_OS_IF] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_TARGET_IF] = QDF_TRACE_LEVEL_INFO,
		[QDF_MODULE_ID_SCHEDULER] = QDF_TRACE_LEVEL_FATAL,
		[QDF_MODULE_ID_MGMT_TXRX] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_SERIALIZATION] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_PMO] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_P2P] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_POLICY_MGR] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_CONFIG] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_REGULATORY] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_SA_API] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_NAN] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_OFFCHAN_TXRX] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_SON] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_SPECTRAL] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_OBJ_MGR] = QDF_TRACE_LEVEL_FATAL,
		[QDF_MODULE_ID_NSS] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_ROAM_DEBUG] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_CDP] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_DIRECT_BUF_RX] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_DISA] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_GREEN_AP] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_FTM] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_EXTAP] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_FD] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_OCB] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_IPA] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_ACTION_OUI] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_CP_STATS] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_DCS] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_MBSSIE] = QDF_TRACE_LEVEL_INFO,
		[QDF_MODULE_ID_FWOL] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_SM_ENGINE] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_CMN_MLME] = QDF_TRACE_LEVEL_INFO,
		[QDF_MODULE_ID_BSSCOLOR] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_CFR] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_TX_CAPTURE] = QDF_TRACE_LEVEL_FATAL,
		[QDF_MODULE_ID_INTEROP_ISSUES_AP] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_BLACKLIST_MGR] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_QLD] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_DYNAMIC_MODE_CHG] = QDF_TRACE_LEVEL_INFO,
		[QDF_MODULE_ID_COEX] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_MON_FILTER] = QDF_TRACE_LEVEL_INFO,
		[QDF_MODULE_ID_ANY] = QDF_TRACE_LEVEL_INFO,
		[QDF_MODULE_ID_PKT_CAPTURE] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_RPTR] = QDF_TRACE_LEVEL_INFO,
		[QDF_MODULE_ID_6GHZ] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_IOT_SIM] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_MSCS] = QDF_TRACE_LEVEL_INFO,
		[QDF_MODULE_ID_GPIO] = QDF_TRACE_LEVEL_NONE,
		[QDF_MODULE_ID_IFMGR] = QDF_TRACE_LEVEL_ERROR,
		[QDF_MODULE_ID_DIAG] = QDF_TRACE_LEVEL_ERROR,
	};

	for (i = 0; i < MAX_SUPPORTED_CATEGORY; i++) {
		cinfo[i].category_verbose_mask = set_cumulative_verbose_mask(
				module_trace_default_level[i]);
	}
}

void qdf_shared_print_ctrl_init(void)
{
	int i;
	struct category_info cinfo[MAX_SUPPORTED_CATEGORY];

	set_default_trace_levels(cinfo);

	/*
	 * User specified across-module single debug level
	 */
	if ((qdf_dbg_mask >= 0) && (qdf_dbg_mask < QDF_TRACE_LEVEL_MAX)) {
		pr_info("User specified module debug level of %d\n",
			qdf_dbg_mask);
		for (i = 0; i < MAX_SUPPORTED_CATEGORY; i++) {
			cinfo[i].category_verbose_mask =
			set_cumulative_verbose_mask(qdf_dbg_mask);
		}
	} else if (qdf_dbg_mask != QDF_TRACE_LEVEL_MAX) {
		pr_info("qdf_dbg_mask value is invalid\n");
		pr_info("Using the default module debug levels instead\n");
	}

	/*
	 * Module ID-Level specified as array during module load
	 */
	for (i = 0; i < qdf_dbg_arr_cnt; i++) {
		process_qdf_dbg_arr_param(cinfo, i);
	}
	qdf_pidx = qdf_print_ctrl_register(cinfo, NULL, NULL,
			"LOG_SHARED_OBJ");
}
qdf_export_symbol(qdf_shared_print_ctrl_init);
#endif

QDF_STATUS qdf_print_set_category_verbose(unsigned int idx,
						QDF_MODULE_ID category,
						QDF_TRACE_LEVEL verbose,
						bool is_set)
{
	/* Check if index passed is valid */
	if (idx < 0 || idx >= MAX_PRINT_CONFIG_SUPPORTED) {
		pr_err("%s: Invalid index - %d\n", __func__, idx);
		return QDF_STATUS_E_FAILURE;
	}

	/* Check if print control object is in use */
	if (!print_ctrl_obj[idx].in_use) {
		pr_err("%s: Invalid print control object\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* Check if category passed is valid */
	if (category < 0 || category >= MAX_SUPPORTED_CATEGORY) {
		pr_err("%s: Invalid category: %d\n", __func__, category);
		return QDF_STATUS_E_FAILURE;
	}

	/* Check if verbose mask is valid */
	if (verbose < 0 || verbose >= QDF_TRACE_LEVEL_MAX) {
		pr_err("%s: Invalid verbose level %d\n", __func__, verbose);
		return QDF_STATUS_E_FAILURE;
	}

	if (verbose == QDF_TRACE_LEVEL_ALL) {
		print_ctrl_obj[idx].cat_info[category].category_verbose_mask =
				0xFFFF;
		return QDF_STATUS_SUCCESS;
	}

	if (verbose == QDF_TRACE_LEVEL_NONE) {
		print_ctrl_obj[idx].cat_info[category].category_verbose_mask =
				QDF_TRACE_LEVEL_NONE;
		return QDF_STATUS_SUCCESS;
	}

	if (!is_set) {
		if (print_ctrl_obj[idx].cat_info[category].category_verbose_mask
		    & QDF_TRACE_LEVEL_TO_MODULE_BITMASK(verbose)) {
			print_ctrl_obj[idx].cat_info[category]
				.category_verbose_mask &=
				~QDF_TRACE_LEVEL_TO_MODULE_BITMASK(verbose);
		}
	} else {
		print_ctrl_obj[idx].cat_info[category].category_verbose_mask |=
				QDF_TRACE_LEVEL_TO_MODULE_BITMASK(verbose);
	}

	pr_debug("%s: Print control object %d, Category %d, Verbose level %d\n",
		__func__,
		idx,
		category,
		print_ctrl_obj[idx].cat_info[category].category_verbose_mask);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_print_set_category_verbose);

void qdf_log_dump_at_kernel_level(bool enable)
{
	if (qdf_log_dump_at_kernel_enable == enable) {
		QDF_TRACE_INFO(QDF_MODULE_ID_QDF,
			       "qdf_log_dump_at_kernel_enable is already %d\n",
			       enable);
	}
	qdf_log_dump_at_kernel_enable = enable;
}

qdf_export_symbol(qdf_log_dump_at_kernel_level);

bool qdf_print_is_category_enabled(unsigned int idx, QDF_MODULE_ID category)
{
	QDF_TRACE_LEVEL verbose_mask;

	/* Check if index passed is valid */
	if (idx < 0 || idx >= MAX_PRINT_CONFIG_SUPPORTED) {
		pr_info("%s: Invalid index - %d\n", __func__, idx);
		return false;
	}

	/* Check if print control object is in use */
	if (!print_ctrl_obj[idx].in_use) {
		pr_info("%s: Invalid print control object\n", __func__);
		return false;
	}

	/* Check if category passed is valid */
	if (category < 0 || category >= MAX_SUPPORTED_CATEGORY) {
		pr_info("%s: Invalid category: %d\n", __func__, category);
		return false;
	}

	verbose_mask =
		print_ctrl_obj[idx].cat_info[category].category_verbose_mask;

	if (verbose_mask == QDF_TRACE_LEVEL_NONE)
		return false;
	else
		return true;
}
qdf_export_symbol(qdf_print_is_category_enabled);

bool qdf_print_is_verbose_enabled(unsigned int idx, QDF_MODULE_ID category,
				  QDF_TRACE_LEVEL verbose)
{
	bool verbose_enabled = false;

	/* Check if index passed is valid */
	if (idx < 0 || idx >= MAX_PRINT_CONFIG_SUPPORTED) {
		pr_info("%s: Invalid index - %d\n", __func__, idx);
		return verbose_enabled;
	}

	/* Check if print control object is in use */
	if (!print_ctrl_obj[idx].in_use) {
		pr_info("%s: Invalid print control object\n", __func__);
		return verbose_enabled;
	}

	/* Check if category passed is valid */
	if (category < 0 || category >= MAX_SUPPORTED_CATEGORY) {
		pr_info("%s: Invalid category: %d\n", __func__, category);
		return verbose_enabled;
	}

	if ((verbose == QDF_TRACE_LEVEL_NONE) ||
	    (verbose >= QDF_TRACE_LEVEL_MAX)) {
		verbose_enabled = false;
	} else if (verbose == QDF_TRACE_LEVEL_ALL) {
		if (print_ctrl_obj[idx].cat_info[category]
					.category_verbose_mask == 0xFFFF)
			verbose_enabled = true;
	} else {
		verbose_enabled =
		(print_ctrl_obj[idx].cat_info[category].category_verbose_mask &
		 QDF_TRACE_LEVEL_TO_MODULE_BITMASK(verbose)) ? true : false;
	}

	return verbose_enabled;
}
qdf_export_symbol(qdf_print_is_verbose_enabled);

#ifdef DBG_LVL_MAC_FILTERING

QDF_STATUS qdf_print_set_node_flag(unsigned int idx, uint8_t enable)
{
	/* Check if index passed is valid */
	if (idx < 0 || idx >= MAX_PRINT_CONFIG_SUPPORTED) {
		pr_info("%s: Invalid index - %d\n", __func__, idx);
		return QDF_STATUS_E_FAILURE;
	}

	/* Check if print control object is in use */
	if (!print_ctrl_obj[idx].in_use) {
		pr_info("%s: Invalid print control object\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (enable > 1) {
		pr_info("%s: Incorrect input: Use 1 or 0 to enable or disable\n",
			__func__);
		return QDF_STATUS_E_FAILURE;
	}

	print_ctrl_obj[idx].dbglvlmac_on = enable;
	pr_info("%s: DbgLVLmac feature %s\n",
		__func__,
		((enable) ? "enabled" : "disabled"));

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_print_set_node_flag);

bool qdf_print_get_node_flag(unsigned int idx)
{
	bool node_flag = false;

	/* Check if index passed is valid */
	if (idx < 0 || idx >= MAX_PRINT_CONFIG_SUPPORTED) {
		pr_info("%s: Invalid index - %d\n", __func__, idx);
		return node_flag;
	}

	/* Check if print control object is in use */
	if (!print_ctrl_obj[idx].in_use) {
		pr_info("%s: Invalid print control object\n", __func__);
		return node_flag;
	}

	if (print_ctrl_obj[idx].dbglvlmac_on)
		node_flag = true;

	return node_flag;
}
qdf_export_symbol(qdf_print_get_node_flag);

void qdf_print_clean_node_flag(unsigned int idx)
{
	/* Disable dbglvlmac_on during cleanup */
	print_ctrl_obj[idx].dbglvlmac_on = 0;
}

#else

void qdf_print_clean_node_flag(unsigned int idx)
{
	/* No operation in case of no support for DBG_LVL_MAC_FILTERING */
	return;
}
#endif

void QDF_PRINT_INFO(unsigned int idx, QDF_MODULE_ID module,
		    QDF_TRACE_LEVEL level,
		    char *str_format, ...)
{
	va_list args;

	/* Generic wrapper API will compile qdf_vprint in order to
	 * log the message. Once QDF converged debug framework is in
	 * place, this will be changed to adapt to the framework, compiling
	 * call to converged tracing API
	 */
	va_start(args, str_format);
	qdf_vprint(str_format, args);
	va_end(args);
}
qdf_export_symbol(QDF_PRINT_INFO);

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
void qdf_logging_init(void)
{
	wlan_logging_sock_init_svc();
	nl_srv_init(NULL, WLAN_NLINK_PROTO_FAMILY);
	wlan_logging_notifier_init(qdf_log_dump_at_kernel_enable);
	wlan_logging_set_flush_timer(qdf_log_flush_timer_period);
}

void qdf_logging_exit(void)
{
	wlan_logging_notifier_deinit(qdf_log_dump_at_kernel_enable);
	nl_srv_exit();
	wlan_logging_sock_deinit_svc();
}

int qdf_logging_set_flush_timer(uint32_t milliseconds)
{
	if (wlan_logging_set_flush_timer(milliseconds) == 0)
		return QDF_STATUS_SUCCESS;
	else
		return QDF_STATUS_E_FAILURE;
}

void qdf_logging_flush_logs(void)
{
	wlan_flush_host_logs_for_fatal();
}

#else
void qdf_logging_init(void)
{
	nl_srv_init(NULL, WLAN_NLINK_PROTO_FAMILY);
}

void qdf_logging_exit(void)
{
	nl_srv_exit();
}

int qdf_logging_set_flush_timer(uint32_t milliseconds)
{
	return QDF_STATUS_E_FAILURE;
}

void qdf_logging_flush_logs(void)
{
}
#endif

qdf_export_symbol(qdf_logging_set_flush_timer);
qdf_export_symbol(qdf_logging_flush_logs);

#ifdef CONFIG_KALLSYMS
inline int qdf_sprint_symbol(char *buffer, void *addr)
{
	return sprint_symbol(buffer, (unsigned long)addr);
}
#else
int qdf_sprint_symbol(char *buffer, void *addr)
{
	if (!buffer)
		return 0;

	buffer[0] = '\0';
	return 1;
}
#endif
qdf_export_symbol(qdf_sprint_symbol);

void qdf_set_pidx(int pidx)
{
	qdf_pidx = pidx;
}
qdf_export_symbol(qdf_set_pidx);

int qdf_get_pidx(void)
{
	return qdf_pidx;
}
qdf_export_symbol(qdf_get_pidx);

#ifdef PANIC_ON_BUG
#ifdef CONFIG_SLUB_DEBUG
void __qdf_bug(void)
{
	BUG();
}
qdf_export_symbol(__qdf_bug);
#endif /* CONFIG_SLUB_DEBUG */
#endif /* PANIC_ON_BUG */

