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

#if !defined(__QDF_TRACE_H)
#define __QDF_TRACE_H

/**
 *  DOC: qdf_trace
 *  QCA driver framework trace APIs
 *  Trace, logging, and debugging definitions and APIs
 */

/* Include Files */
#include  <qdf_types.h>         /* For QDF_MODULE_ID... */
#include  <stdarg.h>            /* For va_list... */
#include  <qdf_status.h>
#include  <qdf_nbuf.h>
#include  <i_qdf_types.h>

/* Type declarations */

#define FL(x)    "%s: %d: " x, __func__, __LINE__

/**
 * typedef enum QDF_TRACE_LEVEL - Debug Trace level
 * @QDF_TRACE_LEVEL_NONE: no trace will be logged. This value is in place
 * for the qdf_trace_setlevel() to allow the user to turn off all traces
 * @QDF_TRACE_LEVEL_FATAL: enable trace for fatal Error
 * @QDF_TRACE_LEVEL_ERROR: enable trace for errors
 * @QDF_TRACE_LEVEL_WARN: enable trace for warnings
 * @QDF_TRACE_LEVEL_INFO: enable trace for information
 * @QDF_TRACE_LEVEL_INFO_HIGH: enable high level trace information
 * @QDF_TRACE_LEVEL_INFO_MED: enable middle level trace information
 * @QDF_TRACE_LEVEL_INFO_LOW: enable low level trace information
 * @QDF_TRACE_LEVEL_DEBUG: enable trace for debugging
 * @QDF_TRACE_LEVEL_ALL: enable all trace
 * @QDF_TRACE_LEVEL_MAX: enable max level trace
 */
typedef enum {
	QDF_TRACE_LEVEL_NONE = 0,
	QDF_TRACE_LEVEL_FATAL,
	QDF_TRACE_LEVEL_ERROR,
	QDF_TRACE_LEVEL_WARN,
	QDF_TRACE_LEVEL_INFO,
	QDF_TRACE_LEVEL_INFO_HIGH,
	QDF_TRACE_LEVEL_INFO_MED,
	QDF_TRACE_LEVEL_INFO_LOW,
	QDF_TRACE_LEVEL_DEBUG,
	QDF_TRACE_LEVEL_ALL,
	QDF_TRACE_LEVEL_MAX
} QDF_TRACE_LEVEL;

/*
 * Log levels
 */
#define QDF_DEBUG_FUNCTRACE     0x01
#define QDF_DEBUG_LEVEL0        0x02
#define QDF_DEBUG_LEVEL1        0x04
#define QDF_DEBUG_LEVEL2        0x08
#define QDF_DEBUG_LEVEL3        0x10
#define QDF_DEBUG_ERROR         0x20
#define QDF_DEBUG_CFG           0x40

#ifdef CONFIG_MCL
/* By default Data Path module will have all log levels enabled, except debug
 * log level. Debug level will be left up to the framework or user space modules
 * to be enabled when issue is detected
 */
#define QDF_DATA_PATH_TRACE_LEVEL \
	((1 << QDF_TRACE_LEVEL_FATAL) | (1 << QDF_TRACE_LEVEL_ERROR) | \
	(1 << QDF_TRACE_LEVEL_WARN) | (1 << QDF_TRACE_LEVEL_INFO) | \
	(1 << QDF_TRACE_LEVEL_INFO_HIGH) | (1 << QDF_TRACE_LEVEL_INFO_MED) | \
	(1 << QDF_TRACE_LEVEL_INFO_LOW))

/* Preprocessor definitions and constants */
#define ASSERT_BUFFER_SIZE (512)

#define MAX_QDF_TRACE_RECORDS 4000
#define INVALID_QDF_TRACE_ADDR 0xffffffff
#define DEFAULT_QDF_TRACE_DUMP_COUNT 0

#include  <i_qdf_trace.h>

#define DUMP_DP_TRACE       0
#define ENABLE_DP_TRACE_LIVE_MODE	1
#define CLEAR_DP_TRACE_BUFFER	2

#ifdef TRACE_RECORD

#define MTRACE(p) p
#define NO_SESSION 0xFF

#else
#define MTRACE(p) {  }

#endif

/**
 * typedef struct qdf_trace_record_s - keep trace record
 * @time: timestamp
 * @module: module name
 * @code: hold record of code
 * @session: hold record of session
 * @data: hold data
 * @pid: hold pid of the process
 */
typedef struct qdf_trace_record_s {
	uint64_t time;
	uint8_t module;
	uint8_t code;
	uint16_t session;
	uint32_t data;
	uint32_t pid;
} qdf_trace_record_t, *tp_qdf_trace_record;

/**
 * typedef struct s_qdf_trace_data - MTRACE logs are stored in ring buffer
 * @head: position of first record
 * @tail: position of last record
 * @num: count of total record
 * @num_since_last_dump: count from last dump
 * @enable: config for controlling the trace
 * @dump_count: Dump after number of records reach this number
 */
typedef struct s_qdf_trace_data {
	uint32_t head;
	uint32_t tail;
	uint32_t num;
	uint16_t num_since_last_dump;
	uint8_t enable;
	uint16_t dump_count;
} t_qdf_trace_data;

#define CASE_RETURN_STRING(str) case ((str)): return (uint8_t *)(# str);

/* DP Trace Implementation */
#ifdef FEATURE_DP_TRACE
#define DPTRACE(p) p
#else
#define DPTRACE(p)
#endif

#define MAX_QDF_DP_TRACE_RECORDS       4000
#define QDF_DP_TRACE_RECORD_SIZE       16
#define INVALID_QDF_DP_TRACE_ADDR      0xffffffff
#define QDF_DP_TRACE_VERBOSITY_HIGH    3
#define QDF_DP_TRACE_VERBOSITY_MEDIUM  2
#define QDF_DP_TRACE_VERBOSITY_LOW     1
#define QDF_DP_TRACE_VERBOSITY_DEFAULT 0

/**
 * enum QDF_DP_TRACE_ID - Generic ID to identify various events in data path
 * @QDF_DP_TRACE_INVALID - invalid
 * @QDF_DP_TRACE_DROP_PACKET_RECORD - record drop packet
 * @QDF_DP_TRACE_EAPOL_PACKET_RECORD - record EAPOL packet
 * @QDF_DP_TRACE_DHCP_PACKET_RECORD - record DHCP packet
 * @QDF_DP_TRACE_ARP_PACKET_RECORD - record ARP packet
 * @QDF_DP_TRACE_MGMT_PACKET_RECORD - record MGMT pacekt
 * @QDF_DP_TRACE_DEFAULT_VERBOSITY - below this are part of default verbosity
 * @QDF_DP_TRACE_HDD_TX_TIMEOUT - HDD tx timeout
 * @QDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT- SOFTAP HDD tx timeout
 * @QDF_DP_TRACE_HDD_TX_PACKET_PTR_RECORD - HDD layer ptr record
 * @QDF_DP_TRACE_CE_PACKET_PTR_RECORD - CE layer ptr record
 * @QDF_DP_TRACE_CE_FAST_PACKET_PTR_RECORD- CE fastpath ptr record
 * @QDF_DP_TRACE_FREE_PACKET_PTR_RECORD - tx completion ptr record
 * @QDF_DP_TRACE_RX_HTT_PACKET_PTR_RECORD - HTT RX record
 * @QDF_DP_TRACE_RX_OFFLOAD_HTT_PACKET_PTR_RECORD- HTT RX offload record
 * @QDF_DP_TRACE_RX_HDD_PACKET_PTR_RECORD - HDD RX record
 * @QDF_DP_TRACE_LOW_VERBOSITY - below this are part of low verbosity
 * @QDF_DP_TRACE_TXRX_QUEUE_PACKET_PTR_RECORD -tx queue ptr record
 * @QDF_DP_TRACE_TXRX_PACKET_PTR_RECORD - txrx packet ptr record
 * @QDF_DP_TRACE_TXRX_FAST_PACKET_PTR_RECORD - txrx fast path record
 * @QDF_DP_TRACE_HTT_PACKET_PTR_RECORD - htt packet ptr record
 * @QDF_DP_TRACE_HTC_PACKET_PTR_RECORD - htc packet ptr record
 * @QDF_DP_TRACE_HIF_PACKET_PTR_RECORD - hif packet ptr record
 * @QDF_DP_TRACE_RX_TXRX_PACKET_PTR_RECORD - txrx packet ptr record
 * @QDF_DP_TRACE_MED_VERBOSITY - below this are part of med verbosity
 * @QDF_DP_TRACE_HDD_TX_PACKET_RECORD - record 32 bytes at HDD
 * @QDF_DP_TRACE_HIGH_VERBOSITY - below this are part of high verbosity
 */
enum  QDF_DP_TRACE_ID {
	QDF_DP_TRACE_INVALID,
	QDF_DP_TRACE_DROP_PACKET_RECORD,
	QDF_DP_TRACE_EAPOL_PACKET_RECORD,
	QDF_DP_TRACE_DHCP_PACKET_RECORD,
	QDF_DP_TRACE_ARP_PACKET_RECORD,
	QDF_DP_TRACE_MGMT_PACKET_RECORD,
	QDF_DP_TRACE_DEFAULT_VERBOSITY,
	QDF_DP_TRACE_HDD_TX_TIMEOUT,
	QDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT,
	QDF_DP_TRACE_HDD_TX_PACKET_PTR_RECORD,
	QDF_DP_TRACE_CE_PACKET_PTR_RECORD,
	QDF_DP_TRACE_CE_FAST_PACKET_PTR_RECORD,
	QDF_DP_TRACE_FREE_PACKET_PTR_RECORD,
	QDF_DP_TRACE_RX_HTT_PACKET_PTR_RECORD,
	QDF_DP_TRACE_RX_OFFLOAD_HTT_PACKET_PTR_RECORD,
	QDF_DP_TRACE_RX_HDD_PACKET_PTR_RECORD,
	QDF_DP_TRACE_LOW_VERBOSITY,
	QDF_DP_TRACE_TXRX_QUEUE_PACKET_PTR_RECORD,
	QDF_DP_TRACE_TXRX_PACKET_PTR_RECORD,
	QDF_DP_TRACE_TXRX_FAST_PACKET_PTR_RECORD,
	QDF_DP_TRACE_HTT_PACKET_PTR_RECORD,
	QDF_DP_TRACE_HTC_PACKET_PTR_RECORD,
	QDF_DP_TRACE_HIF_PACKET_PTR_RECORD,
	QDF_DP_TRACE_RX_TXRX_PACKET_PTR_RECORD,
	QDF_DP_TRACE_MED_VERBOSITY,
	QDF_DP_TRACE_HDD_TX_PACKET_RECORD,
	QDF_DP_TRACE_HIGH_VERBOSITY,
	QDF_DP_TRACE_MAX
};

/**
 * qdf_proto_dir - direction
 * @QDF_TX: TX direction
 * @QDF_RX: RX direction
 * @QDF_NA: not applicable
 */
enum qdf_proto_dir {
	QDF_TX,
	QDF_RX,
	QDF_NA
};

/**
 * struct qdf_dp_trace_ptr_buf - pointer record buffer
 * @cookie: cookie value
 * @msdu_id: msdu_id
 * @status: completion status
 */
struct qdf_dp_trace_ptr_buf {
	uint64_t cookie;
	uint16_t msdu_id;
	uint16_t status;
};

/**
 * struct qdf_dp_trace_proto_buf - proto packet buffer
 * @sa: source address
 * @da: destination address
 * @vdev_id : vdev id
 * @type: packet type
 * @subtype: packet subtype
 * @dir: direction
 */
struct qdf_dp_trace_proto_buf {
	struct qdf_mac_addr sa;
	struct qdf_mac_addr da;
	uint8_t vdev_id;
	uint8_t type;
	uint8_t subtype;
	uint8_t dir;
};

/**
 * struct qdf_dp_trace_mgmt_buf - mgmt packet buffer
 * @vdev_id : vdev id
 * @type: packet type
 * @subtype: packet subtype
 */
struct qdf_dp_trace_mgmt_buf {
	uint8_t vdev_id;
	uint8_t type;
	uint8_t subtype;
};

/**
 * struct qdf_dp_trace_record_s - Describes a record in DP trace
 * @time: time when it got stored
 * @code: Describes the particular event
 * @data: buffer to store data
 * @size: Length of the valid data stored in this record
 * @pid : process id which stored the data in this record
 */
struct qdf_dp_trace_record_s {
	uint64_t time;
	uint8_t code;
	uint8_t data[QDF_DP_TRACE_RECORD_SIZE];
	uint8_t size;
	uint32_t pid;
};

/**
 * struct qdf_dp_trace_data - Parameters to configure/control DP trace
 * @head: Position of first record
 * @tail: Position of last record
 * @num:  Current index
 * @proto_bitmap: defines which protocol to be traced
 * @no_of_record: defines every nth packet to be traced
 * @verbosity : defines verbosity level
 * @enable: enable/disable DP trace
 * @count: current packet number
 */
struct s_qdf_dp_trace_data {
	uint32_t head;
	uint32_t tail;
	uint32_t num;
	uint8_t proto_bitmap;
	uint8_t no_of_record;
	uint8_t verbosity;
	bool enable;
	uint32_t tx_count;
	uint32_t rx_count;
	bool live_mode;
};


/* Function declarations and documenation */

/**
 * qdf_trace_set_level() - Set the trace level for a particular module
 * @level : trace level
 *
 * Trace level is a member of the QDF_TRACE_LEVEL enumeration indicating
 * the severity of the condition causing the trace message to be issued.
 * More severe conditions are more likely to be logged.
 *
 * This is an external API that allows trace levels to be set for each module.
 *
 * Return:  nothing
 */
void qdf_trace_set_level(QDF_MODULE_ID module, QDF_TRACE_LEVEL level);

/**
 * qdf_trace_get_level() - get the trace level
 * @level : trace level
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
 *  false - the specified trace level for the specified module is OFF
 *  true - the specified trace level for the specified module is ON
 */
bool qdf_trace_get_level(QDF_MODULE_ID module, QDF_TRACE_LEVEL level);

typedef void (*tp_qdf_trace_cb)(void *p_mac, tp_qdf_trace_record, uint16_t);
void qdf_trace(uint8_t module, uint8_t code, uint16_t session, uint32_t data);
void qdf_trace_register(QDF_MODULE_ID, tp_qdf_trace_cb);
QDF_STATUS qdf_trace_spin_lock_init(void);
void qdf_trace_init(void);
void qdf_trace_enable(uint32_t, uint8_t enable);
void qdf_trace_dump_all(void *, uint8_t, uint8_t, uint32_t, uint32_t);


#ifdef FEATURE_DP_TRACE
void qdf_dp_trace_log_pkt(uint8_t session_id, struct sk_buff *skb,
				enum qdf_proto_dir dir);
void qdf_dp_trace_init(void);
void qdf_dp_trace_spin_lock_init(void);
void qdf_dp_trace_set_value(uint8_t proto_bitmap, uint8_t no_of_records,
			 uint8_t verbosity);
void qdf_dp_trace_set_track(qdf_nbuf_t nbuf, enum qdf_proto_dir dir);
void qdf_dp_trace(qdf_nbuf_t nbuf, enum QDF_DP_TRACE_ID code,
			uint8_t *data, uint8_t size, enum qdf_proto_dir dir);
void qdf_dp_trace_dump_all(uint32_t count);
typedef void (*tp_qdf_dp_trace_cb)(struct qdf_dp_trace_record_s* , uint16_t);
void qdf_dp_display_record(struct qdf_dp_trace_record_s *record,
							uint16_t index);
void qdf_dp_trace_ptr(qdf_nbuf_t nbuf, enum QDF_DP_TRACE_ID code,
		uint8_t *data, uint8_t size, uint16_t msdu_id, uint16_t status);

void qdf_dp_display_ptr_record(struct qdf_dp_trace_record_s *pRecord,
				uint16_t recIndex);
uint8_t qdf_dp_get_proto_bitmap(void);
void
qdf_dp_trace_proto_pkt(enum QDF_DP_TRACE_ID code, uint8_t vdev_id,
		uint8_t *sa, uint8_t *da, enum qdf_proto_type type,
		enum qdf_proto_subtype subtype, enum qdf_proto_dir dir);
void qdf_dp_display_proto_pkt(struct qdf_dp_trace_record_s *record,
				uint16_t index);
void qdf_dp_trace_enable_live_mode(void);
void qdf_dp_trace_clear_buffer(void);
void qdf_dp_trace_mgmt_pkt(enum QDF_DP_TRACE_ID code, uint8_t vdev_id,
		enum qdf_proto_type type, enum qdf_proto_subtype subtype);
void qdf_dp_display_mgmt_pkt(struct qdf_dp_trace_record_s *record,
			      uint16_t index);
#else
static inline
void qdf_dp_trace_log_pkt(uint8_t session_id, struct sk_buff *skb,
				enum qdf_proto_dir dir)
{
}
static inline
void qdf_dp_trace_init(void)
{
}
static inline
void qdf_dp_trace_set_track(qdf_nbuf_t nbuf, enum qdf_proto_dir dir)
{
}
static inline
void qdf_dp_trace_set_value(uint8_t proto_bitmap, uint8_t no_of_records,
			 uint8_t verbosity)
{
}
static inline
void qdf_dp_trace_dump_all(uint32_t count)
{
}

static inline
void qdf_dp_trace_enable_live_mode(void)
{
}

static inline
void qdf_dp_trace_clear_buffer(void)
{
}

#endif


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
 */
void __printf(3, 4) qdf_trace_msg(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
		   char *str_format, ...);

void qdf_trace_hex_dump(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
			void *data, int buf_len);

void qdf_trace_display(void);

void qdf_trace_set_value(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
			 uint8_t on);

void qdf_trace_set_module_trace_level(QDF_MODULE_ID module, uint32_t level);

void __printf(3, 4) qdf_snprintf(char *str_buffer, unsigned int size,
		  char *str_format, ...);

#define QDF_SNPRINTF qdf_snprintf
#else

#define DPTRACE(x)
#define qdf_trace_hex_dump(x, y, z, q)

#endif /* CONFIG_MCL */

#endif /* __QDF_TRACE_H */
