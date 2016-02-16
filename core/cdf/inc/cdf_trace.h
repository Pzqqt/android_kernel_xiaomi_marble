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

#if !defined(__CDF_TRACE_H)
#define __CDF_TRACE_H

/**
 *  DOC:  cdf_trace
 *
 *  Connectivity driver framework trace APIs
 *
 *  Trace, logging, and debugging definitions and APIs
 *
 */

/* Include Files */
#include  <cdf_types.h>         /* For CDF_MODULE_ID... */
#include  <stdarg.h>            /* For va_list... */
#include  <cdf_status.h>
#include  <qdf_status.h>
#include  <cdf_nbuf.h>
#include  <cds_packet.h>
#include  <i_cdf_types.h>

/* Type declarations */

typedef enum {
	/* NONE means NO traces will be logged.  This value is in place
	 * for the cdf_trace_setlevel() to allow the user to turn off
	 * all traces
	 */
	CDF_TRACE_LEVEL_NONE = 0,

	/* Following trace levels are the ones that 'callers' of CDF_TRACE()
	 * can specify in for the CDF_TRACE_LEVEL parameter.  Traces are
	 * classified by severity. FATAL being more serious than INFO for
	 * example
	 */
	CDF_TRACE_LEVEL_FATAL,
	CDF_TRACE_LEVEL_ERROR,
	CDF_TRACE_LEVEL_WARN,
	CDF_TRACE_LEVEL_INFO,
	CDF_TRACE_LEVEL_INFO_HIGH,
	CDF_TRACE_LEVEL_INFO_MED,
	CDF_TRACE_LEVEL_INFO_LOW,
	CDF_TRACE_LEVEL_DEBUG,

	/* All means all trace levels will be active.  This value is in place
	 * for the cdf_trace_setlevel() to allow the user to turn ON all traces
	 */
	CDF_TRACE_LEVEL_ALL,

	/* Not a real level.  Used to identify the maximum number of
	 * CDF_TRACE_LEVELs defined
	 */
	CDF_TRACE_LEVEL_MAX
} CDF_TRACE_LEVEL;

/* By default Data Path module will have all log levels enabled, except debug
 * log level. Debug level will be left up to the framework or user space modules
 * to be enabled when issue is detected
 */
#define CDF_DATA_PATH_TRACE_LEVEL \
	((1 << CDF_TRACE_LEVEL_FATAL) | (1 << CDF_TRACE_LEVEL_ERROR) | \
	(1 << CDF_TRACE_LEVEL_WARN) | (1 << CDF_TRACE_LEVEL_INFO) | \
	(1 << CDF_TRACE_LEVEL_INFO_HIGH) | (1 << CDF_TRACE_LEVEL_INFO_MED) | \
	(1 << CDF_TRACE_LEVEL_INFO_LOW))

/* Preprocessor definitions and constants */
#define ASSERT_BUFFER_SIZE (512)

#define CDF_ENABLE_TRACING
#define MAX_CDF_TRACE_RECORDS 4000
#define INVALID_CDF_TRACE_ADDR 0xffffffff
#define DEFAULT_CDF_TRACE_DUMP_COUNT 0

#include  <i_cdf_trace.h>

#ifdef TRACE_RECORD

#define MTRACE(p) p
#define NO_SESSION 0xFF

#else
#define MTRACE(p) {  }

#endif

/* Structure definition */
typedef struct cdf_trace_record_s {
	uint64_t time;
	uint8_t module;
	uint8_t code;
	uint16_t session;
	uint32_t data;
	uint32_t pid;
} cdf_trace_record_t, *tp_cdf_trace_record;

typedef struct s_cdf_trace_data {
	/* MTRACE logs are stored in ring buffer where head represents the
	 * position of first record, tail represents the position of last record
	 * added till now and num is the count of total record added
	 */
	uint32_t head;
	uint32_t tail;
	uint32_t num;
	uint16_t numSinceLastDump;

	/* config for controlling the trace */
	uint8_t enable;
	/* Dump after number of records reach this number */
	uint16_t dumpCount;
} t_cdf_trace_data;

#define CASE_RETURN_STRING(str) case ((str)): return (uint8_t *)(# str);

/* DP Trace Implementation */
#define DPTRACE(p) p

#define MAX_CDF_DP_TRACE_RECORDS       4000
#define CDF_DP_TRACE_RECORD_SIZE       16
#define INVALID_CDF_DP_TRACE_ADDR      0xffffffff
#define CDF_DP_TRACE_VERBOSITY_HIGH    3
#define CDF_DP_TRACE_VERBOSITY_MEDIUM  2
#define CDF_DP_TRACE_VERBOSITY_LOW     1
#define CDF_DP_TRACE_VERBOSITY_DEFAULT 0

/**
 * enum CDF_DP_TRACE_ID - Generic ID to identify various events in data path
 * @CDF_DP_TRACE_INVALID: Invalid ID
 * @CDF_DP_TRACE_DROP_PACKET_RECORD: Dropped packet stored with this id
 * @CDF_DP_TRACE_HDD_PACKET_PTR_RECORD: nbuf->data ptr of HDD
 * @CDF_DP_TRACE_HDD_PACKET_RECORD: nbuf->data stored with this id
 * @CDF_DP_TRACE_CE_PACKET_PTR_RECORD: nbuf->data ptr of CE
 * @CDF_DP_TRACE_CE_PACKET_RECORD: nbuf->data stored with this id
 * @CDF_DP_TRACE_TXRX_QUEUE_PACKET_PTR_RECORD: nbuf->data ptr of txrx queue
 * @CDF_DP_TRACE_TXRX_PACKET_PTR_RECORD: nbuf->data ptr of txrx
 * @CDF_DP_TRACE_HTT_PACKET_PTR_RECORD: nbuf->data ptr of htt
 * @CDF_DP_TRACE_HTC_PACKET_PTR_RECORD: nbuf->data ptr of htc
 * @CDF_DP_TRACE_HIF_PACKET_PTR_RECORD: nbuf->data ptr of hif
 * @CDF_DP_TRACE_HDD_TX_TIMEOUT: hdd tx timeout event
 * @CDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT: hdd tx softap timeout event
 * @CDF_DP_TRACE_VDEV_PAUSE: vdev pause event
 * @CDF_DP_TRACE_VDEV_UNPAUSE: vdev unpause event
 *
 */
enum  CDF_DP_TRACE_ID {
	CDF_DP_TRACE_INVALID                           = 0,
	CDF_DP_TRACE_DROP_PACKET_RECORD                = 1,
	CDF_DP_TRACE_HDD_PACKET_PTR_RECORD             = 2,
	CDF_DP_TRACE_HDD_PACKET_RECORD                 = 3,
	CDF_DP_TRACE_CE_PACKET_PTR_RECORD              = 4,
	CDF_DP_TRACE_CE_PACKET_RECORD                  = 5,
	CDF_DP_TRACE_TXRX_QUEUE_PACKET_PTR_RECORD      = 6,
	CDF_DP_TRACE_TXRX_PACKET_PTR_RECORD            = 7,
	CDF_DP_TRACE_HTT_PACKET_PTR_RECORD             = 8,
	CDF_DP_TRACE_HTC_PACKET_PTR_RECORD             = 9,
	CDF_DP_TRACE_HIF_PACKET_PTR_RECORD             = 10,
	CDF_DP_TRACE_HDD_TX_TIMEOUT                    = 11,
	CDF_DP_TRACE_HDD_SOFTAP_TX_TIMEOUT             = 12,
	CDF_DP_TRACE_VDEV_PAUSE                        = 13,
	CDF_DP_TRACE_VDEV_UNPAUSE                      = 14,
	CDF_DP_TRACE_MAX

};

/**
 * struct cdf_dp_trace_record_s - Describes a record in DP trace
 * @time: time when it got stored
 * @code: Describes the particular event
 * @data: buffer to store data
 * @size: Length of the valid data stored in this record
 * @pid : process id which stored the data in this record
 */
struct cdf_dp_trace_record_s {
	uint64_t time;
	uint8_t code;
	uint8_t data[CDF_DP_TRACE_RECORD_SIZE];
	uint8_t size;
	uint32_t pid;
};

/**
 * struct cdf_dp_trace_data - Parameters to configure/control DP trace
 * @head: Position of first record
 * @tail: Position of last record
 * @num:  Current index
 * @proto_bitmap: defines which protocol to be traced
 * @no_of_record: defines every nth packet to be traced
 * @verbosity : defines verbosity level
 * @enable: enable/disable DP trace
 * @count: current packet number
 */
struct s_cdf_dp_trace_data {
	uint32_t head;
	uint32_t tail;
	uint32_t num;

	/* config for controlling the trace */
	uint8_t proto_bitmap;
	uint8_t no_of_record;
	uint8_t verbosity;
	bool enable;
	uint32_t count;
};
/* Function declarations and documenation */

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
void cdf_trace_set_level(CDF_MODULE_ID module, CDF_TRACE_LEVEL level);

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
 *  false - the specified trace level for the specified module is OFF
 *  true - the specified trace level for the specified module is ON
 */
bool cdf_trace_get_level(CDF_MODULE_ID module, CDF_TRACE_LEVEL level);

typedef void (*tp_cdf_trace_cb)(void *pMac, tp_cdf_trace_record, uint16_t);
void cdf_trace(uint8_t module, uint8_t code, uint16_t session, uint32_t data);
void cdf_trace_register(CDF_MODULE_ID, tp_cdf_trace_cb);
CDF_STATUS cdf_trace_spin_lock_init(void);
void cdf_trace_init(void);
void cdf_trace_enable(uint32_t, uint8_t enable);
void cdf_trace_dump_all(void *, uint8_t, uint8_t, uint32_t, uint32_t);

void cdf_dp_trace_spin_lock_init(void);
void cdf_dp_trace_init(void);
void cdf_dp_trace_set_value(uint8_t proto_bitmap, uint8_t no_of_records,
			 uint8_t verbosity);
void cdf_dp_trace_set_track(cdf_nbuf_t nbuf);
void cdf_dp_trace(cdf_nbuf_t nbuf, enum CDF_DP_TRACE_ID code,
			uint8_t *data, uint8_t size);
void cdf_dp_trace_dump_all(uint32_t count);
typedef void (*tp_cdf_dp_trace_cb)(struct cdf_dp_trace_record_s* , uint16_t);
void cdf_dp_display_record(struct cdf_dp_trace_record_s *record,
							uint16_t index);
#endif
