/*
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
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
#define QDF_TRACE_BUFFER_SIZE (512)

#ifdef CONFIG_MCL
#define QDF_DEFAULT_TRACE_LEVEL \
	((1 << QDF_TRACE_LEVEL_FATAL) | (1 << QDF_TRACE_LEVEL_ERROR))
#else
#define QDF_DEFAULT_TRACE_LEVEL (1 << QDF_TRACE_LEVEL_INFO)
#endif

#define QDF_CATEGORY_INFO_U16(val) (((val >> 16) & 0x0000FFFF))
#define QDF_TRACE_LEVEL_INFO_L16(val) (val & 0x0000FFFF)

typedef int (qdf_abstract_print)(void *priv, const char *fmt, ...);

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


/* DP Trace Implementation */
#ifdef CONFIG_DP_TRACE
#define DPTRACE(p) p
#else
#define DPTRACE(p)
#endif

#ifdef CONFIG_MCL
void qdf_trace(uint8_t module, uint8_t code, uint16_t session, uint32_t data);
#endif

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

#define QDF_TRACE_DEFAULT_PDEV_ID 0xff
#define MAX_QDF_TRACE_RECORDS 4000
#define INVALID_QDF_TRACE_ADDR 0xffffffff
#define DEFAULT_QDF_TRACE_DUMP_COUNT 0

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
 * @qtime: qtimer ticks
 * @time: user timestamp
 * @module: module name
 * @code: hold record of code
 * @session: hold record of session
 * @data: hold data
 * @pid: hold pid of the process
 */
typedef struct qdf_trace_record_s {
	uint64_t qtime;
	char time[18];
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
 * QDF_DP_TRACE_EVENT_RECORD - record events
 * @QDF_DP_TRACE_DEFAULT_VERBOSITY - below this are part of default verbosity
 * @QDF_DP_TRACE_HDD_TX_TIMEOUT - HDD tx timeout
 * @QDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT- SOFTAP HDD tx timeout
 * @QDF_DP_TRACE_HDD_TX_PACKET_PTR_RECORD - HDD layer ptr record
 * @QDF_DP_TRACE_LI_DP_TX_PACKET_PTR_RECORD - Lithium DP layer ptr record
 * @QDF_DP_TRACE_CE_PACKET_PTR_RECORD - CE layer ptr record
 * @QDF_DP_TRACE_CE_FAST_PACKET_PTR_RECORD- CE fastpath ptr record
 * @QDF_DP_TRACE_FREE_PACKET_PTR_RECORD - tx completion ptr record
 * @QDF_DP_TRACE_RX_HTT_PACKET_PTR_RECORD - HTT RX record
 * @QDF_DP_TRACE_RX_OFFLOAD_HTT_PACKET_PTR_RECORD- HTT RX offload record
 * @QDF_DP_TRACE_RX_HDD_PACKET_PTR_RECORD - HDD RX record
 * @QDF_DP_TRACE_RX_LI_DP_PACKET_PTR_RECORD - Lithium DP RX record
 * @QDF_DP_TRACE_LOW_VERBOSITY - below this are part of low verbosity
 * @QDF_DP_TRACE_TXRX_QUEUE_PACKET_PTR_RECORD -tx queue ptr record
 * @QDF_DP_TRACE_TXRX_PACKET_PTR_RECORD - txrx packet ptr record
 * @QDF_DP_TRACE_TXRX_FAST_PACKET_PTR_RECORD - txrx fast path record
 * @QDF_DP_TRACE_HTT_PACKET_PTR_RECORD - htt packet ptr record
 * @QDF_DP_TRACE_HTC_PACKET_PTR_RECORD - htc packet ptr record
 * @QDF_DP_TRACE_HIF_PACKET_PTR_RECORD - hif packet ptr record
 * @QDF_DP_TRACE_RX_TXRX_PACKET_PTR_RECORD - txrx packet ptr record
 * @QDF_DP_TRACE_MED_VERBOSITY - below this are part of med verbosity
 * @QDF_DP_TRACE_HDD_TX_PACKET_RECORD - record 32 bytes of tx pkt at HDD
 * @QDF_DP_TRACE_HDD_RX_PACKET_RECORD - record 32 bytes of rx pkt at HDD
 * @QDF_DP_TRACE_LI_DP_TX_PACKET_RECORD - record 32 bytes of tx pkt at LI_DP
 * @QDF_DP_TRACE_LI_DP_RX_PACKET_RECORD - record 32 bytes of rx pkt at LI_DP
 * @QDF_DP_TRACE_LI_DP_NULL_RX_PACKET_RECORD
 *		- record 32 bytes of rx null_queue pkt at LI_DP
 * @QDF_DP_TRACE_HIGH_VERBOSITY - below this are part of high verbosity
 */
enum  QDF_DP_TRACE_ID {
	QDF_DP_TRACE_INVALID,
	QDF_DP_TRACE_DROP_PACKET_RECORD,
	QDF_DP_TRACE_EAPOL_PACKET_RECORD,
	QDF_DP_TRACE_DHCP_PACKET_RECORD,
	QDF_DP_TRACE_ARP_PACKET_RECORD,
	QDF_DP_TRACE_MGMT_PACKET_RECORD,
	QDF_DP_TRACE_EVENT_RECORD,
	QDF_DP_TRACE_DEFAULT_VERBOSITY,
	QDF_DP_TRACE_HDD_TX_TIMEOUT,
	QDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT,
	QDF_DP_TRACE_HDD_TX_PACKET_PTR_RECORD,
	QDF_DP_TRACE_LI_DP_TX_PACKET_PTR_RECORD,
	QDF_DP_TRACE_CE_PACKET_PTR_RECORD,
	QDF_DP_TRACE_CE_FAST_PACKET_PTR_RECORD,
	QDF_DP_TRACE_FREE_PACKET_PTR_RECORD,
	QDF_DP_TRACE_RX_HTT_PACKET_PTR_RECORD,
	QDF_DP_TRACE_RX_OFFLOAD_HTT_PACKET_PTR_RECORD,
	QDF_DP_TRACE_RX_HDD_PACKET_PTR_RECORD,
	QDF_DP_TRACE_RX_LI_DP_PACKET_PTR_RECORD,
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
	QDF_DP_TRACE_HDD_RX_PACKET_RECORD,
	QDF_DP_TRACE_LI_DP_TX_PACKET_RECORD,
	QDF_DP_TRACE_LI_DP_RX_PACKET_RECORD,
	QDF_DP_TRACE_LI_DP_NULL_RX_PACKET_RECORD,
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
 * struct qdf_dp_trace_event_buf - event buffer
 * @vdev_id : vdev id
 * @type: packet type
 * @subtype: packet subtype
 */
struct qdf_dp_trace_event_buf {
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
	char time[20];
	uint8_t code;
	uint8_t data[QDF_DP_TRACE_RECORD_SIZE];
	uint8_t size;
	uint32_t pid;
	uint8_t pdev_id;
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
typedef void (*tp_qdf_state_info_cb) (char **buf, uint16_t *size);
void qdf_register_debugcb_init(void);
void qdf_register_debug_callback(QDF_MODULE_ID module_id,
					tp_qdf_state_info_cb qdf_state_infocb);
QDF_STATUS qdf_state_info_dump_all(char *buf, uint16_t size,
			uint16_t *driver_dump_size);
void qdf_trace_register(QDF_MODULE_ID, tp_qdf_trace_cb);
QDF_STATUS qdf_trace_spin_lock_init(void);
void qdf_trace_init(void);
void qdf_trace_enable(uint32_t, uint8_t enable);
void qdf_trace_dump_all(void *, uint8_t, uint8_t, uint32_t, uint32_t);


#ifdef CONFIG_DP_TRACE
void qdf_dp_set_proto_bitmap(uint32_t val);
void qdf_dp_trace_set_verbosity(uint32_t val);
void qdf_dp_set_no_of_record(uint32_t val);
void qdf_dp_trace_log_pkt(uint8_t session_id, struct sk_buff *skb,
				enum qdf_proto_dir dir, uint8_t pdev_id);
void qdf_dp_trace_init(void);
void qdf_dp_trace_spin_lock_init(void);
void qdf_dp_trace_set_value(uint8_t proto_bitmap, uint8_t no_of_records,
			 uint8_t verbosity);
void qdf_dp_trace_set_track(qdf_nbuf_t nbuf, enum qdf_proto_dir dir);
void qdf_dp_trace(qdf_nbuf_t nbuf, enum QDF_DP_TRACE_ID code, uint8_t pdev_id,
			uint8_t *data, uint8_t size, enum qdf_proto_dir dir);
void qdf_dp_trace_dump_all(uint32_t count, uint8_t pdev_id);
typedef void (*tp_qdf_dp_trace_cb)(struct qdf_dp_trace_record_s*,
				uint16_t, uint8_t);
void qdf_dp_display_record(struct qdf_dp_trace_record_s *record,
					uint16_t index, uint8_t pdev_id);
void qdf_dp_trace_ptr(qdf_nbuf_t nbuf, enum QDF_DP_TRACE_ID code,
	uint8_t pdev_id, uint8_t *data, uint8_t size, uint16_t msdu_id,
	uint16_t status);

void qdf_dp_display_ptr_record(struct qdf_dp_trace_record_s *pRecord,
				uint16_t recIndex, uint8_t pdev_id);
uint8_t qdf_dp_get_proto_bitmap(void);
uint8_t qdf_dp_get_verbosity(void);
uint8_t qdf_dp_get_no_of_record(void);
void
qdf_dp_trace_proto_pkt(enum QDF_DP_TRACE_ID code, uint8_t vdev_id,
	uint8_t *sa, uint8_t *da, enum qdf_proto_type type,
	enum qdf_proto_subtype subtype, enum qdf_proto_dir dir,
	uint8_t pdev_id);
void qdf_dp_display_proto_pkt(struct qdf_dp_trace_record_s *record,
				uint16_t index, uint8_t pdev_id);
void qdf_dp_trace_enable_live_mode(void);
void qdf_dp_trace_clear_buffer(void);
void qdf_dp_trace_mgmt_pkt(enum QDF_DP_TRACE_ID code, uint8_t vdev_id,
	uint8_t pdev_id, enum qdf_proto_type type,
	enum qdf_proto_subtype subtype);
void qdf_dp_display_mgmt_pkt(struct qdf_dp_trace_record_s *record,
			      uint16_t index, uint8_t pdev_id);
void qdf_dp_display_event_record(struct qdf_dp_trace_record_s *record,
			      uint16_t index, uint8_t pdev_id);
void qdf_dp_trace_record_event(enum QDF_DP_TRACE_ID code, uint8_t vdev_id,
	uint8_t pdev_id, enum qdf_proto_type type,
	enum qdf_proto_subtype subtype);
#else
static inline
void qdf_dp_trace_log_pkt(uint8_t session_id, struct sk_buff *skb,
				enum qdf_proto_dir dir, uint8_t pdev_id)
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
void qdf_dp_trace_dump_all(uint32_t count, uint8_t pdev_id)
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



void qdf_trace_display(void);

void qdf_trace_set_value(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
			 uint8_t on);

void qdf_trace_set_module_trace_level(QDF_MODULE_ID module, uint32_t level);

void __printf(3, 4) qdf_snprintf(char *str_buffer, unsigned int size,
		  char *str_format, ...);

#define QDF_SNPRINTF qdf_snprintf

#ifdef TSOSEG_DEBUG
static inline
int qdf_tso_seg_dbg_record(struct qdf_tso_seg_elem_t *tsoseg,
			   uint16_t caller)
{
	int rc = -1;

	if (tsoseg != NULL) {
		tsoseg->dbg.cur++;  tsoseg->dbg.cur &= 0x0f;
		tsoseg->dbg.history[tsoseg->dbg.cur] = caller;
		rc = tsoseg->dbg.cur;
	}
	return rc;
};
static inline void qdf_tso_seg_dbg_bug(char *msg)
{
	qdf_print(msg);
	QDF_BUG(0);
};

static inline void
qdf_tso_seg_dbg_setowner(struct qdf_tso_seg_elem_t *tsoseg, void *owner)
{
	tsoseg->dbg.txdesc = owner;
};

static inline void
qdf_tso_seg_dbg_zero(struct qdf_tso_seg_elem_t *tsoseg)
{
	memset(tsoseg, 0, offsetof(struct qdf_tso_seg_elem_t, dbg));
	return;
};

#else
static inline
int qdf_tso_seg_dbg_record(struct qdf_tso_seg_elem_t *tsoseg,
			   uint16_t caller)
{
	return 0;
};
static inline void qdf_tso_seg_dbg_bug(char *msg)
{
};
static inline void
qdf_tso_seg_dbg_setowner(struct qdf_tso_seg_elem_t *tsoseg, void *owner)
{
};
static inline int
qdf_tso_seg_dbg_zero(struct qdf_tso_seg_elem_t *tsoseg)
{
	memset(tsoseg, 0, sizeof(struct qdf_tso_seg_elem_t));
	return 0;
};

#endif /* TSOSEG_DEBUG */

void qdf_trace_hex_dump(QDF_MODULE_ID module, QDF_TRACE_LEVEL level,
			void *data, int buf_len);

#define ERROR_CODE                      -1
#define QDF_MAX_NAME_SIZE               32
#define MAX_PRINT_CONFIG_SUPPORTED      32

#define MAX_SUPPORTED_CATEGORY QDF_MODULE_ID_MAX

/**
 * qdf_set_pidx() - Sets the global qdf_pidx.
 * @pidx : Index of print control object assigned to the module
 *
 */
void qdf_set_pidx(int pidx);

/**
 * qdf_get_pidx() - Returns the global qdf_pidx.
 *
 * Return : Current qdf print index.
 */
int qdf_get_pidx(void);
/*
 * Shared print control index
 * for converged debug framework
 */

#define QDF_PRINT_IDX_SHARED -1

/**
 * QDF_PRINT_INFO() - Generic wrapper API for logging
 * @idx : Index of print control object
 * @module : Module identifier. A member of QDF_MODULE_ID enumeration that
 *           identifies the module issuing the trace message
 * @level : Trace level. A member of QDF_TRACE_LEVEL enumeration indicating
 *          the severity of the condition causing the trace message to be
 *          issued.
 * @str_format : Format string that contains the message to be logged.
 *
 *
 * This wrapper will be used for any generic logging messages. Wrapper will
 * compile a call to converged QDF trace message API.
 *
 * Return : Nothing
 *
 */
void QDF_PRINT_INFO(unsigned int idx, QDF_MODULE_ID module,
		    QDF_TRACE_LEVEL level,
		    char *str_format, ...);

/**
 * struct category_info  : Category information structure
 * @category_verbose_mask: Embeds information about category's verbose level
 */
struct category_info {
	uint16_t category_verbose_mask;
};

/**
 * struct category_name_info  : Category name information structure
 * @category_name_str: Embeds information about category name
 */
struct category_name_info {
	unsigned char category_name_str[QDF_MAX_NAME_SIZE];
};

/**
 * qdf_trace_msg_cmn()- Converged logging API
 * @idx: Index of print control object assigned to the module
 * @category: Category identifier. A member of the QDF_MODULE_ID enumeration
 *            that identifies the category issuing the trace message.
 * @verbose: Verbose level. A member of the QDF_TRACE_LEVEL enumeration
 *           indicating the severity of the condition causing the trace
 *           message to be issued. More severe conditions are more likely
 *           to be logged.
 * @str_format: Format string. The message to be logged. This format string
 *              contains printf-like replacement parameters, which follow this
 *              parameter in the variable argument list.
 * @val: Variable argument list part of the log message
 *
 * Return: nothing
 *
 */
void qdf_trace_msg_cmn(unsigned int idx,
			QDF_MODULE_ID category,
			QDF_TRACE_LEVEL verbose,
			const char *str_format,
			va_list val);

/**
 * struct qdf_print_ctrl: QDF Print Control structure
 *                        Statically allocated objects of print control
 *                        structure are declared that will support maximum of
 *                        32 print control objects. Any module that needs to
 *                        register to the print control framework needs to
 *                        obtain a print control object using
 *                        qdf_print_ctrl_register API. It will have to pass
 *                        pointer to category info structure, name and
 *                        custom print function to be used if required.
 * @name                : Optional name for the control object
 * @cat_info            : Array of category_info struct
 * @custom_print        : Custom print handler
 * @custom_ctxt         : Custom print context
 * @dbglvlmac_on        : Flag to enable/disable MAC level filtering
 * @in_use              : Boolean to indicate if control object is in use
 */
struct qdf_print_ctrl {
	char name[QDF_MAX_NAME_SIZE];
	struct category_info cat_info[MAX_SUPPORTED_CATEGORY];
	void (*custom_print)(void *ctxt, const char *fmt, va_list args);
	void *custom_ctxt;
#ifdef DBG_LVL_MAC_FILTERING
	unsigned char dbglvlmac_on;
#endif
	bool in_use;
};

/**
 * qdf_print_ctrl_register() - Allocate QDF print control object, assign
 *                             pointer to category info or print control
 *                             structure and return the index to the callee
 * @cinfo                 : Pointer to array of category info structure
 * @custom_print_handler  : Pointer to custom print handler
 * @custom_ctx            : Pointer to custom context
 * @pctrl_name            : Pointer to print control object name
 *
 * Return                 : Index of qdf_print_ctrl structure
 *
 */
int qdf_print_ctrl_register(const struct category_info *cinfo,
			    void *custom_print_handler,
			    void *custom_ctx,
			    const char *pctrl_name);

/**
 * qdf_shared_print_ctrl_init() - Initialize the shared print ctrl obj with
 *                                all categories set to the default level
 *
 * Return                 : void
 *
 */
void qdf_shared_print_ctrl_init(void);

/**
 * qdf_print_setup() - Setup default values to all the print control objects
 *
 * Register new print control object for the callee
 *
 * Return :             QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE
 *                      on failure
 */
QDF_STATUS qdf_print_setup(void);

/**
 * qdf_print_ctrl_cleanup() - Clean up a print control object
 *
 * Cleanup the print control object for the callee
 *
 * @pctrl : Index of print control object
 *
 * Return : QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS qdf_print_ctrl_cleanup(unsigned int idx);

/**
 * qdf_print_ctrl_shared_cleanup() - Clean up of the shared object
 *
 * Cleanup the shared print-ctrl-object
 *
 * Return : void
 */
void qdf_shared_print_ctrl_cleanup(void);

/**
 * qdf_print_set_category_verbose() - Enable/Disable category for a
 *                                    print control object with
 *                                    user provided verbose level
 *
 * @idx : Index of the print control object assigned to callee
 * @category : Category information
 * @verbose: Verbose information
 * @is_set: Flag indicating if verbose level needs to be enabled or disabled
 *
 * Return : QDF_STATUS_SUCCESS for success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS qdf_print_set_category_verbose(unsigned int idx,
					  QDF_MODULE_ID category,
					  QDF_TRACE_LEVEL verbose,
					  bool is_set);

/**
 * qdf_print_is_category_enabled() - Get category information for the
 *                                   print control object
 *
 * @idx : Index of print control object
 * @category : Category information
 *
 * Return : Verbose enabled(true) or disabled(false) or invalid input (false)
 */
bool qdf_print_is_category_enabled(unsigned int idx,
				   QDF_MODULE_ID category);

/**
 * qdf_print_is_verbose_enabled() - Get verbose information of a category for
 *                                  the print control object
 *
 * @idx : Index of print control object
 * @category : Category information
 * @verbose : Verbose information
 *
 * Return : Verbose enabled(true) or disabled(false) or invalid input (false)
 */
bool qdf_print_is_verbose_enabled(unsigned int idx,
				  QDF_MODULE_ID category,
				  QDF_TRACE_LEVEL verbose);

/**
 * qdf_print_clean_node_flag() - Clean up node flag for print control object
 *
 * @idx : Index of print control object
 *
 * Return : None
 */
void qdf_print_clean_node_flag(unsigned int idx);

#ifdef DBG_LVL_MAC_FILTERING

/**
 * qdf_print_set_node_flag() - Set flag to enable MAC level filtering
 *
 * @idx : Index of print control object
 * @enable : Enable/Disable bit sent by callee
 *
 * Return : QDF_STATUS_SUCCESS on Success and QDF_STATUS_E_FAILURE on Failure
 */
QDF_STATUS qdf_print_set_node_flag(unsigned int idx,
				   uint8_t enable);

/**
 * qdf_print_get_node_flag() - Get flag that controls MAC level filtering
 *
 * @idx : Index of print control object
 *
 * Return : Flag that indicates enable(1) or disable(0) or invalid(-1)
 */
bool qdf_print_get_node_flag(unsigned int idx);

#endif

/**
 * qdf_logging_init() - Initialize msg logging functionality
 *
 *
 * Return : void
 */
void qdf_logging_init(void);

/**
 * qdf_logging_exit() - Cleanup msg logging functionality
 *
 *
 * Return : void
 */
void qdf_logging_exit(void);

#define QDF_SYMBOL_LEN __QDF_SYMBOL_LEN

/**
 * qdf_sprint_symbol() - prints the name of a symbol into a string buffer
 * @buffer: the string buffer to print into
 * @addr: address of the symbol to lookup and print
 *
 * Return: number of characters printed
 */
int qdf_sprint_symbol(char *buffer, void *addr);

#endif /* __QDF_TRACE_H */
