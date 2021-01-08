/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#ifndef _HTT_EVENT_LOGGING__
#define _HTT_EVENT_LOGGING__

#include <cdp_txrx_handle.h>

#ifndef HTT_EVENT_DEBUG_MAX_ENTRY
#define HTT_EVENT_DEBUG_MAX_ENTRY (1024)
#endif

#define HTT_EVENT_DEBUG_ENTRY_MAX_LENGTH (32)
#define HTT_COMMAND_DEBUG_ENTRY_MAX_LENGTH (32)
#define HTT_WBM_EVENT_DEBUG_ENTRY_MAX_LENGTH (32)
#define NUM_HTT_DEBUG_INFOS 8

#define HTT_LOGD(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HTT, QDF_TRACE_LEVEL_DEBUG, ## args)
#define HTT_LOGI(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HTT, QDF_TRACE_LEVEL_INFO, ## args)
#define HTT_LOGW(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HTT, QDF_TRACE_LEVEL_WARN, ## args)
#define HTT_LOGE(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HTT, QDF_TRACE_LEVEL_ERROR, ## args)
#define HTT_LOGP(args ...) \
	QDF_TRACE(QDF_MODULE_ID_HTT, QDF_TRACE_LEVEL_FATAL, ## args)


/**
 * struct htt_command_debug - HTT command log buffer data type
 * @ command - Store HTT MSG id
 * @ data - vdevid,peerid
 * @ time - Time of HTT command handling
 * @ cpu_id - Cpu number which handles cmd/event
 */
struct htt_command_debug {
	uint8_t command;
	uint8_t data[HTT_COMMAND_DEBUG_ENTRY_MAX_LENGTH];
	uint64_t time;
	uint8_t cpu_id;
};

/**
 * struct htt_event_debug - HTT event log buffer data type
 * @ event - Store HTT event id
 * @ data - vdevid,peerid
 * @ time - Time of HTT event handling
 * @ cpu_id - Cpu number which handles cmd/event
 */
struct htt_event_debug {
	uint8_t event;
	uint8_t data[HTT_EVENT_DEBUG_ENTRY_MAX_LENGTH];
	uint64_t time;
	uint8_t cpu_id;
};

/**
 * struct htt_wbm_event_debug - HTT wbm event log buffer data type
 * @ tx_status  - Status
 * @ data - msg data(peer_id,tid..)
 * @ time - Time of HTT event handling
 * @ cpu_id - Cpu number which handles cmd/event
 */
struct htt_wbm_event_debug {
	uint8_t tx_status;
	uint8_t data[HTT_WBM_EVENT_DEBUG_ENTRY_MAX_LENGTH];
	uint64_t time;
	uint8_t cpu_id;
};

/**
 * struct htt_log_buf_t - HTT log buffer information type
 * @buf - Refernce to HTT log buffer
 * @ length - total count of command/event logged
 * @ buf_tail_idx - Tail index of buffer
 * @ size - the size of the buffer in number of entries
 * @record_lock - Lock HTT recording separate for all cmd/event/wbm
 */
struct htt_log_buf_t {
	void *buf;
	uint32_t length;
	uint32_t buf_tail_idx;
	uint32_t size;

	qdf_spinlock_t record_lock;
};

/**
 * struct htt_debug_log_info - Meta data to hold information of
 *                             all buffers used for HTT logging
 * @htt_command_log_buf_info - Buffer info for HTT Command log
 * @htt_event_log_buf_info - Buffer info for HTT Event log
 * @htt_wbm_event_log_buf_info - Buffer info for HTT wbm event log
 * @htt_record_lock - Lock HTT recording (while reading through debugfs)
 * @htt_logging_enable - Enable/Disable state for HTT logging
 * @htt_cmd_disable_list - Enable/Disable logging for some
 *                           specific cmd(BitMap)
 * @htt_event_disable_list - Enable/Disable logging for some
 *                           specific Event(BitMap)
 * @htt_wbm_event_disable_list - Enable/Disable logging for
 *                           some specific wbm event(BitMap)
 * @stop_seq - Used for read ack.
 *      stop read sequence once done.
 *      qdf_debugfs show calls .show function ptr iteratively
 *      and hence we need to explicitly stop after one call.
 *      1 bit is used for one fops show call.
 *      Default all is set to 0.
 * @htt_log_debugfs_dir - refernce to debugfs directory
 */
struct htt_debug_log_info {
	struct htt_log_buf_t htt_command_log_buf_info;
	struct htt_log_buf_t htt_event_log_buf_info;
	struct htt_log_buf_t htt_wbm_event_log_buf_info;

	qdf_spinlock_t htt_record_lock;
	bool htt_logging_enable;
	uint64_t htt_cmd_disable_list;
	uint64_t htt_event_disable_list;
	uint64_t htt_wbm_event_disable_list;
	uint16_t stop_seq;
	qdf_dentry_t htt_log_debugfs_dir;
};

/**
 * @ debugfs_de - Dentry for each debugfs exported entry
 * @log_info - Data for all commad/event
 */
struct htt_logger {
	qdf_dentry_t debugfs_de[NUM_HTT_DEBUG_INFOS];
	struct htt_debug_log_info log_info;
};

/**
 * htt_interface_logging_init() - Initialize HTT event tracer
 *
 * @ htt_logger_handle - Pointer for HTT tracer
 */
void htt_interface_logging_init(struct htt_logger **htt_logger_handle,
				struct cdp_ctrl_objmgr_psoc *ctrl_psoc);

/**
 * htt_interface_logging_deinit() - Deinitialize HTT event tracer
 *
 * @ htt_logger_handle - Pointer for HTT tracer
 */
void htt_interface_logging_deinit(struct htt_logger *htt_logger_handle);

/**
 * htt_command_recored() - Record HTT command
 *
 * @ h - Pointer for HTT tracer
 * @ msg_type - Message type to be logged
 * @ msg_data - Data to be logged
 */
int htt_command_record(struct htt_logger *h, uint8_t msg_type,
		       uint8_t *msg_data);

/**
 * htt_event_recored() - Record HTT event
 *
 * @ h - Pointer for HTT tracer
 * @ msg_type - Message type to be logged
 * @ msg_data - Data to be logged
 */
int htt_event_record(struct htt_logger *h, uint8_t msg_type,
		     uint8_t *msg_data);

/**
 * htt_wbm_event_recored() - Record HTT wbm event
 *
 * @ h - Pointer for HTT tracer
 * @ tx_status - Status flag
 * @ msg_data - Data to be logged
 */
int htt_wbm_event_record(struct htt_logger *h, uint8_t tx_status,
			 uint8_t *msg_data);

#endif /* _HTT_EVENT_LOGGING__ */
