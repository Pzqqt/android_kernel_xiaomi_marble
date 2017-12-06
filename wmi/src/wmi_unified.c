/*
 * Copyright (c) 2015-2018 The Linux Foundation. All rights reserved.
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

/*
 * Host WMI unified implementation
 */
#include "htc_api.h"
#include "htc_api.h"
#include "wmi_unified_priv.h"
#include "qdf_module.h"

#ifndef WMI_NON_TLV_SUPPORT
#include "wmi_tlv_helper.h"
#endif

#include <linux/debugfs.h>

/* This check for CONFIG_WIN temporary added due to redeclaration compilation
error in MCL. Error is caused due to inclusion of wmi.h in wmi_unified_api.h
which gets included here through ol_if_athvar.h. Eventually it is expected that
wmi.h will be removed from wmi_unified_api.h after cleanup, which will need
WMI_CMD_HDR to be defined here. */
#ifdef CONFIG_WIN
/* Copied from wmi.h */
#undef MS
#define MS(_v, _f) (((_v) & _f##_MASK) >> _f##_LSB)
#undef SM
#define SM(_v, _f) (((_v) << _f##_LSB) & _f##_MASK)
#undef WO
#define WO(_f)      ((_f##_OFFSET) >> 2)

#undef GET_FIELD
#define GET_FIELD(_addr, _f) MS(*((A_UINT32 *)(_addr) + WO(_f)), _f)
#undef SET_FIELD
#define SET_FIELD(_addr, _f, _val)  \
	    (*((A_UINT32 *)(_addr) + WO(_f)) = \
		(*((A_UINT32 *)(_addr) + WO(_f)) & ~_f##_MASK) | SM(_val, _f))

#define WMI_GET_FIELD(_msg_buf, _msg_type, _f) \
	    GET_FIELD(_msg_buf, _msg_type ## _ ## _f)

#define WMI_SET_FIELD(_msg_buf, _msg_type, _f, _val) \
	    SET_FIELD(_msg_buf, _msg_type ## _ ## _f, _val)

#define WMI_EP_APASS           0x0
#define WMI_EP_LPASS           0x1
#define WMI_EP_SENSOR          0x2

/*
 *  * Control Path
 *   */
typedef PREPACK struct {
	A_UINT32	commandId:24,
			reserved:2, /* used for WMI endpoint ID */
			plt_priv:6; /* platform private */
} POSTPACK WMI_CMD_HDR;        /* used for commands and events */

#define WMI_CMD_HDR_COMMANDID_LSB           0
#define WMI_CMD_HDR_COMMANDID_MASK          0x00ffffff
#define WMI_CMD_HDR_COMMANDID_OFFSET        0x00000000
#define WMI_CMD_HDR_WMI_ENDPOINTID_MASK        0x03000000
#define WMI_CMD_HDR_WMI_ENDPOINTID_OFFSET      24
#define WMI_CMD_HDR_PLT_PRIV_LSB               24
#define WMI_CMD_HDR_PLT_PRIV_MASK              0xff000000
#define WMI_CMD_HDR_PLT_PRIV_OFFSET            0x00000000
/* end of copy wmi.h */
#endif /* CONFIG_WIN */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))
/* TODO Cleanup this backported function */
static int wmi_bp_seq_printf(struct seq_file *m, const char *f, ...)
{
	va_list args;

	va_start(args, f);
	seq_vprintf(m, f, args);
	va_end(args);

	return 0;
}
#else
#define wmi_bp_seq_printf(m, fmt, ...) seq_printf((m), fmt, ##__VA_ARGS__)
#endif

#define WMI_MIN_HEAD_ROOM 64

#ifdef WMI_INTERFACE_EVENT_LOGGING
#ifndef MAX_WMI_INSTANCES
#ifdef CONFIG_MCL
#define MAX_WMI_INSTANCES 1
#else
#define MAX_WMI_INSTANCES 3
#endif
#define CUSTOM_MGMT_CMD_DATA_SIZE 4
#endif

#ifdef CONFIG_MCL
/* WMI commands */
uint32_t g_wmi_command_buf_idx = 0;
struct wmi_command_debug wmi_command_log_buffer[WMI_EVENT_DEBUG_MAX_ENTRY];

/* WMI commands TX completed */
uint32_t g_wmi_command_tx_cmp_buf_idx = 0;
struct wmi_command_debug
	wmi_command_tx_cmp_log_buffer[WMI_EVENT_DEBUG_MAX_ENTRY];

/* WMI events when processed */
uint32_t g_wmi_event_buf_idx = 0;
struct wmi_event_debug wmi_event_log_buffer[WMI_EVENT_DEBUG_MAX_ENTRY];

/* WMI events when queued */
uint32_t g_wmi_rx_event_buf_idx = 0;
struct wmi_event_debug wmi_rx_event_log_buffer[WMI_EVENT_DEBUG_MAX_ENTRY];
#endif

#define WMI_COMMAND_RECORD(h, a, b) {					\
	if (wmi_log_max_entry <=					\
		*(h->log_info.wmi_command_log_buf_info.p_buf_tail_idx))	\
		*(h->log_info.wmi_command_log_buf_info.p_buf_tail_idx) = 0;\
	((struct wmi_command_debug *)h->log_info.wmi_command_log_buf_info.buf)\
		[*(h->log_info.wmi_command_log_buf_info.p_buf_tail_idx)]\
						.command = a;		\
	qdf_mem_copy(((struct wmi_command_debug *)h->log_info.		\
				wmi_command_log_buf_info.buf)		\
		[*(h->log_info.wmi_command_log_buf_info.p_buf_tail_idx)].data,\
			b, wmi_record_max_length);			\
	((struct wmi_command_debug *)h->log_info.wmi_command_log_buf_info.buf)\
		[*(h->log_info.wmi_command_log_buf_info.p_buf_tail_idx)].\
		time = qdf_get_log_timestamp();			\
	(*(h->log_info.wmi_command_log_buf_info.p_buf_tail_idx))++;	\
	h->log_info.wmi_command_log_buf_info.length++;			\
}

#define WMI_COMMAND_TX_CMP_RECORD(h, a, b) {				\
	if (wmi_log_max_entry <=					\
		*(h->log_info.wmi_command_tx_cmp_log_buf_info.p_buf_tail_idx))\
		*(h->log_info.wmi_command_tx_cmp_log_buf_info.		\
				p_buf_tail_idx) = 0;			\
	((struct wmi_command_debug *)h->log_info.			\
		wmi_command_tx_cmp_log_buf_info.buf)			\
		[*(h->log_info.wmi_command_tx_cmp_log_buf_info.		\
				p_buf_tail_idx)].			\
							command	= a;	\
	qdf_mem_copy(((struct wmi_command_debug *)h->log_info.		\
				wmi_command_tx_cmp_log_buf_info.buf)	\
		[*(h->log_info.wmi_command_tx_cmp_log_buf_info.		\
			p_buf_tail_idx)].				\
		data, b, wmi_record_max_length);			\
	((struct wmi_command_debug *)h->log_info.			\
		wmi_command_tx_cmp_log_buf_info.buf)			\
		[*(h->log_info.wmi_command_tx_cmp_log_buf_info.		\
				p_buf_tail_idx)].			\
		time = qdf_get_log_timestamp();				\
	(*(h->log_info.wmi_command_tx_cmp_log_buf_info.p_buf_tail_idx))++;\
	h->log_info.wmi_command_tx_cmp_log_buf_info.length++;		\
}

#define WMI_EVENT_RECORD(h, a, b) {					\
	if (wmi_log_max_entry <=					\
		*(h->log_info.wmi_event_log_buf_info.p_buf_tail_idx))	\
		*(h->log_info.wmi_event_log_buf_info.p_buf_tail_idx) = 0;\
	((struct wmi_event_debug *)h->log_info.wmi_event_log_buf_info.buf)\
		[*(h->log_info.wmi_event_log_buf_info.p_buf_tail_idx)].	\
		event = a;						\
	qdf_mem_copy(((struct wmi_event_debug *)h->log_info.		\
				wmi_event_log_buf_info.buf)		\
		[*(h->log_info.wmi_event_log_buf_info.p_buf_tail_idx)].data, b,\
		wmi_record_max_length);					\
	((struct wmi_event_debug *)h->log_info.wmi_event_log_buf_info.buf)\
		[*(h->log_info.wmi_event_log_buf_info.p_buf_tail_idx)].time =\
		qdf_get_log_timestamp();				\
	(*(h->log_info.wmi_event_log_buf_info.p_buf_tail_idx))++;	\
	h->log_info.wmi_event_log_buf_info.length++;			\
}

#define WMI_RX_EVENT_RECORD(h, a, b) {					\
	if (wmi_log_max_entry <=					\
		*(h->log_info.wmi_rx_event_log_buf_info.p_buf_tail_idx))\
		*(h->log_info.wmi_rx_event_log_buf_info.p_buf_tail_idx) = 0;\
	((struct wmi_event_debug *)h->log_info.wmi_rx_event_log_buf_info.buf)\
		[*(h->log_info.wmi_rx_event_log_buf_info.p_buf_tail_idx)].\
		event = a;						\
	qdf_mem_copy(((struct wmi_event_debug *)h->log_info.		\
				wmi_rx_event_log_buf_info.buf)		\
		[*(h->log_info.wmi_rx_event_log_buf_info.p_buf_tail_idx)].\
			data, b, wmi_record_max_length);		\
	((struct wmi_event_debug *)h->log_info.wmi_rx_event_log_buf_info.buf)\
		[*(h->log_info.wmi_rx_event_log_buf_info.p_buf_tail_idx)].\
		time =	qdf_get_log_timestamp();			\
	(*(h->log_info.wmi_rx_event_log_buf_info.p_buf_tail_idx))++;	\
	h->log_info.wmi_rx_event_log_buf_info.length++;			\
}

#ifdef CONFIG_MCL
uint32_t g_wmi_mgmt_command_buf_idx = 0;
struct
wmi_command_debug wmi_mgmt_command_log_buffer[WMI_MGMT_EVENT_DEBUG_MAX_ENTRY];

/* wmi_mgmt commands TX completed */
uint32_t g_wmi_mgmt_command_tx_cmp_buf_idx = 0;
struct wmi_command_debug
wmi_mgmt_command_tx_cmp_log_buffer[WMI_MGMT_EVENT_DEBUG_MAX_ENTRY];

/* wmi_mgmt events when processed */
uint32_t g_wmi_mgmt_event_buf_idx = 0;
struct wmi_event_debug
wmi_mgmt_event_log_buffer[WMI_MGMT_EVENT_DEBUG_MAX_ENTRY];
#endif

#define WMI_MGMT_COMMAND_RECORD(h, a, b) {                              \
	if (wmi_mgmt_log_max_entry <=                                   \
		*(h->log_info.wmi_mgmt_command_log_buf_info.p_buf_tail_idx)) \
		*(h->log_info.wmi_mgmt_command_log_buf_info.		\
				p_buf_tail_idx) = 0;			\
	((struct wmi_command_debug *)h->log_info.                       \
		 wmi_mgmt_command_log_buf_info.buf)                     \
		[*(h->log_info.wmi_mgmt_command_log_buf_info.p_buf_tail_idx)].\
			command = a;                                    \
	qdf_mem_copy(((struct wmi_command_debug *)h->log_info.          \
				wmi_mgmt_command_log_buf_info.buf)      \
		[*(h->log_info.wmi_mgmt_command_log_buf_info.p_buf_tail_idx)].\
		data, b,                                                \
		wmi_record_max_length);                                	\
	((struct wmi_command_debug *)h->log_info.                       \
		 wmi_mgmt_command_log_buf_info.buf)                     \
		[*(h->log_info.wmi_mgmt_command_log_buf_info.p_buf_tail_idx)].\
			time =        qdf_get_log_timestamp();          \
	(*(h->log_info.wmi_mgmt_command_log_buf_info.p_buf_tail_idx))++;\
	h->log_info.wmi_mgmt_command_log_buf_info.length++;             \
}

#define WMI_MGMT_COMMAND_TX_CMP_RECORD(h, a, b) {			\
	if (wmi_mgmt_log_max_entry <=					\
		*(h->log_info.wmi_mgmt_command_tx_cmp_log_buf_info.	\
			p_buf_tail_idx))				\
		*(h->log_info.wmi_mgmt_command_tx_cmp_log_buf_info.	\
			p_buf_tail_idx) = 0;				\
	((struct wmi_command_debug *)h->log_info.			\
			wmi_mgmt_command_tx_cmp_log_buf_info.buf)	\
		[*(h->log_info.wmi_mgmt_command_tx_cmp_log_buf_info.	\
				p_buf_tail_idx)].command = a;		\
	qdf_mem_copy(((struct wmi_command_debug *)h->log_info.		\
				wmi_mgmt_command_tx_cmp_log_buf_info.buf)\
		[*(h->log_info.wmi_mgmt_command_tx_cmp_log_buf_info.	\
			p_buf_tail_idx)].data, b,			\
			wmi_record_max_length);				\
	((struct wmi_command_debug *)h->log_info.			\
			wmi_mgmt_command_tx_cmp_log_buf_info.buf)	\
		[*(h->log_info.wmi_mgmt_command_tx_cmp_log_buf_info.	\
				p_buf_tail_idx)].time =			\
		qdf_get_log_timestamp();				\
	(*(h->log_info.wmi_mgmt_command_tx_cmp_log_buf_info.		\
			p_buf_tail_idx))++;				\
	h->log_info.wmi_mgmt_command_tx_cmp_log_buf_info.length++;	\
}

#define WMI_MGMT_EVENT_RECORD(h, a, b) {				\
	if (wmi_mgmt_log_max_entry <=					\
		*(h->log_info.wmi_mgmt_event_log_buf_info.p_buf_tail_idx))\
		*(h->log_info.wmi_mgmt_event_log_buf_info.p_buf_tail_idx) = 0;\
	((struct wmi_event_debug *)h->log_info.wmi_mgmt_event_log_buf_info.buf)\
		[*(h->log_info.wmi_mgmt_event_log_buf_info.p_buf_tail_idx)]\
					.event = a;			\
	qdf_mem_copy(((struct wmi_event_debug *)h->log_info.		\
				wmi_mgmt_event_log_buf_info.buf)	\
		[*(h->log_info.wmi_mgmt_event_log_buf_info.p_buf_tail_idx)].\
			data, b, wmi_record_max_length);		\
	((struct wmi_event_debug *)h->log_info.wmi_mgmt_event_log_buf_info.buf)\
		[*(h->log_info.wmi_mgmt_event_log_buf_info.p_buf_tail_idx)].\
			time = qdf_get_log_timestamp();			\
	(*(h->log_info.wmi_mgmt_event_log_buf_info.p_buf_tail_idx))++;	\
	h->log_info.wmi_mgmt_event_log_buf_info.length++;		\
}

/* These are defined to made it as module param, which can be configured */
uint32_t wmi_log_max_entry = WMI_EVENT_DEBUG_MAX_ENTRY;
uint32_t wmi_mgmt_log_max_entry = WMI_MGMT_EVENT_DEBUG_MAX_ENTRY;
uint32_t wmi_record_max_length = WMI_EVENT_DEBUG_ENTRY_MAX_LENGTH;
uint32_t wmi_display_size = 100;

/**
 * wmi_log_init() - Initialize WMI event logging
 * @wmi_handle: WMI handle.
 *
 * Return: Initialization status
 */
#ifdef CONFIG_MCL
static QDF_STATUS wmi_log_init(struct wmi_unified *wmi_handle)
{
	struct wmi_log_buf_t *cmd_log_buf =
			&wmi_handle->log_info.wmi_command_log_buf_info;
	struct wmi_log_buf_t *cmd_tx_cmpl_log_buf =
			&wmi_handle->log_info.wmi_command_tx_cmp_log_buf_info;

	struct wmi_log_buf_t *event_log_buf =
			&wmi_handle->log_info.wmi_event_log_buf_info;
	struct wmi_log_buf_t *rx_event_log_buf =
			&wmi_handle->log_info.wmi_rx_event_log_buf_info;

	struct wmi_log_buf_t *mgmt_cmd_log_buf =
			&wmi_handle->log_info.wmi_mgmt_command_log_buf_info;
	struct wmi_log_buf_t *mgmt_cmd_tx_cmp_log_buf =
		&wmi_handle->log_info.wmi_mgmt_command_tx_cmp_log_buf_info;
	struct wmi_log_buf_t *mgmt_event_log_buf =
			&wmi_handle->log_info.wmi_mgmt_event_log_buf_info;

	/* WMI commands */
	cmd_log_buf->length = 0;
	cmd_log_buf->buf_tail_idx = 0;
	cmd_log_buf->buf = wmi_command_log_buffer;
	cmd_log_buf->p_buf_tail_idx = &g_wmi_command_buf_idx;
	cmd_log_buf->size = WMI_EVENT_DEBUG_MAX_ENTRY;

	/* WMI commands TX completed */
	cmd_tx_cmpl_log_buf->length = 0;
	cmd_tx_cmpl_log_buf->buf_tail_idx = 0;
	cmd_tx_cmpl_log_buf->buf = wmi_command_tx_cmp_log_buffer;
	cmd_tx_cmpl_log_buf->p_buf_tail_idx = &g_wmi_command_tx_cmp_buf_idx;
	cmd_tx_cmpl_log_buf->size = WMI_EVENT_DEBUG_MAX_ENTRY;

	/* WMI events when processed */
	event_log_buf->length = 0;
	event_log_buf->buf_tail_idx = 0;
	event_log_buf->buf = wmi_event_log_buffer;
	event_log_buf->p_buf_tail_idx = &g_wmi_event_buf_idx;
	event_log_buf->size = WMI_EVENT_DEBUG_MAX_ENTRY;

	/* WMI events when queued */
	rx_event_log_buf->length = 0;
	rx_event_log_buf->buf_tail_idx = 0;
	rx_event_log_buf->buf = wmi_rx_event_log_buffer;
	rx_event_log_buf->p_buf_tail_idx = &g_wmi_rx_event_buf_idx;
	rx_event_log_buf->size = WMI_EVENT_DEBUG_MAX_ENTRY;

	/* WMI Management commands */
	mgmt_cmd_log_buf->length = 0;
	mgmt_cmd_log_buf->buf_tail_idx = 0;
	mgmt_cmd_log_buf->buf = wmi_mgmt_command_log_buffer;
	mgmt_cmd_log_buf->p_buf_tail_idx = &g_wmi_mgmt_command_buf_idx;
	mgmt_cmd_log_buf->size = WMI_MGMT_EVENT_DEBUG_MAX_ENTRY;

	/* WMI Management commands Tx completed*/
	mgmt_cmd_tx_cmp_log_buf->length = 0;
	mgmt_cmd_tx_cmp_log_buf->buf_tail_idx = 0;
	mgmt_cmd_tx_cmp_log_buf->buf = wmi_mgmt_command_tx_cmp_log_buffer;
	mgmt_cmd_tx_cmp_log_buf->p_buf_tail_idx =
		&g_wmi_mgmt_command_tx_cmp_buf_idx;
	mgmt_cmd_tx_cmp_log_buf->size = WMI_MGMT_EVENT_DEBUG_MAX_ENTRY;

	/* WMI Management events when processed*/
	mgmt_event_log_buf->length = 0;
	mgmt_event_log_buf->buf_tail_idx = 0;
	mgmt_event_log_buf->buf = wmi_mgmt_event_log_buffer;
	mgmt_event_log_buf->p_buf_tail_idx = &g_wmi_mgmt_event_buf_idx;
	mgmt_event_log_buf->size = WMI_MGMT_EVENT_DEBUG_MAX_ENTRY;

	qdf_spinlock_create(&wmi_handle->log_info.wmi_record_lock);
	wmi_handle->log_info.wmi_logging_enable = 1;

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS wmi_log_init(struct wmi_unified *wmi_handle)
{
	struct wmi_log_buf_t *cmd_log_buf =
			&wmi_handle->log_info.wmi_command_log_buf_info;
	struct wmi_log_buf_t *cmd_tx_cmpl_log_buf =
			&wmi_handle->log_info.wmi_command_tx_cmp_log_buf_info;

	struct wmi_log_buf_t *event_log_buf =
			&wmi_handle->log_info.wmi_event_log_buf_info;
	struct wmi_log_buf_t *rx_event_log_buf =
			&wmi_handle->log_info.wmi_rx_event_log_buf_info;

	struct wmi_log_buf_t *mgmt_cmd_log_buf =
			&wmi_handle->log_info.wmi_mgmt_command_log_buf_info;
	struct wmi_log_buf_t *mgmt_cmd_tx_cmp_log_buf =
		&wmi_handle->log_info.wmi_mgmt_command_tx_cmp_log_buf_info;
	struct wmi_log_buf_t *mgmt_event_log_buf =
			&wmi_handle->log_info.wmi_mgmt_event_log_buf_info;

	wmi_handle->log_info.wmi_logging_enable = 0;

	/* WMI commands */
	cmd_log_buf->length = 0;
	cmd_log_buf->buf_tail_idx = 0;
	cmd_log_buf->buf = (struct wmi_command_debug *) qdf_mem_malloc(
		wmi_log_max_entry * sizeof(struct wmi_command_debug));
	cmd_log_buf->size = wmi_log_max_entry;

	if (!cmd_log_buf->buf) {
		qdf_print("no memory for WMI command log buffer..\n");
		return QDF_STATUS_E_NOMEM;
	}
	cmd_log_buf->p_buf_tail_idx = &cmd_log_buf->buf_tail_idx;

	/* WMI commands TX completed */
	cmd_tx_cmpl_log_buf->length = 0;
	cmd_tx_cmpl_log_buf->buf_tail_idx = 0;
	cmd_tx_cmpl_log_buf->buf = (struct wmi_command_debug *) qdf_mem_malloc(
		wmi_log_max_entry * sizeof(struct wmi_command_debug));
	cmd_tx_cmpl_log_buf->size = wmi_log_max_entry;

	if (!cmd_tx_cmpl_log_buf->buf) {
		qdf_print("no memory for WMI Command Tx Complete log buffer..\n");
		return QDF_STATUS_E_NOMEM;
	}
	cmd_tx_cmpl_log_buf->p_buf_tail_idx =
		&cmd_tx_cmpl_log_buf->buf_tail_idx;

	/* WMI events when processed */
	event_log_buf->length = 0;
	event_log_buf->buf_tail_idx = 0;
	event_log_buf->buf = (struct wmi_event_debug *) qdf_mem_malloc(
		wmi_log_max_entry * sizeof(struct wmi_event_debug));
	event_log_buf->size = wmi_log_max_entry;

	if (!event_log_buf->buf) {
		qdf_print("no memory for WMI Event log buffer..\n");
		return QDF_STATUS_E_NOMEM;
	}
	event_log_buf->p_buf_tail_idx = &event_log_buf->buf_tail_idx;

	/* WMI events when queued */
	rx_event_log_buf->length = 0;
	rx_event_log_buf->buf_tail_idx = 0;
	rx_event_log_buf->buf = (struct wmi_event_debug *) qdf_mem_malloc(
		wmi_log_max_entry * sizeof(struct wmi_event_debug));
	rx_event_log_buf->size = wmi_log_max_entry;

	if (!rx_event_log_buf->buf) {
		qdf_print("no memory for WMI Event Rx log buffer..\n");
		return QDF_STATUS_E_NOMEM;
	}
	rx_event_log_buf->p_buf_tail_idx = &rx_event_log_buf->buf_tail_idx;

	/* WMI Management commands */
	mgmt_cmd_log_buf->length = 0;
	mgmt_cmd_log_buf->buf_tail_idx = 0;
	mgmt_cmd_log_buf->buf = (struct wmi_command_debug *) qdf_mem_malloc(
		wmi_mgmt_log_max_entry * sizeof(struct wmi_command_debug));
	mgmt_cmd_log_buf->size = wmi_mgmt_log_max_entry;

	if (!mgmt_cmd_log_buf->buf) {
		qdf_print("no memory for WMI Management Command log buffer..\n");
		return QDF_STATUS_E_NOMEM;
	}
	mgmt_cmd_log_buf->p_buf_tail_idx = &mgmt_cmd_log_buf->buf_tail_idx;

	/* WMI Management commands Tx completed*/
	mgmt_cmd_tx_cmp_log_buf->length = 0;
	mgmt_cmd_tx_cmp_log_buf->buf_tail_idx = 0;
	mgmt_cmd_tx_cmp_log_buf->buf = (struct wmi_command_debug *)
		qdf_mem_malloc(
		wmi_mgmt_log_max_entry *
		sizeof(struct wmi_command_debug));
	mgmt_cmd_tx_cmp_log_buf->size = wmi_mgmt_log_max_entry;

	if (!mgmt_cmd_tx_cmp_log_buf->buf) {
		qdf_print("no memory for WMI Management Command Tx complete log buffer..\n");
		return QDF_STATUS_E_NOMEM;
	}
	mgmt_cmd_tx_cmp_log_buf->p_buf_tail_idx =
		&mgmt_cmd_tx_cmp_log_buf->buf_tail_idx;

	/* WMI Management events when processed*/
	mgmt_event_log_buf->length = 0;
	mgmt_event_log_buf->buf_tail_idx = 0;

	mgmt_event_log_buf->buf = (struct wmi_event_debug *) qdf_mem_malloc(
		wmi_mgmt_log_max_entry *
		sizeof(struct wmi_event_debug));
	mgmt_event_log_buf->size = wmi_mgmt_log_max_entry;

	if (!mgmt_event_log_buf->buf) {
		qdf_print("no memory for WMI Management Event log buffer..\n");
		return QDF_STATUS_E_NOMEM;
	}
	mgmt_event_log_buf->p_buf_tail_idx = &mgmt_event_log_buf->buf_tail_idx;

	qdf_spinlock_create(&wmi_handle->log_info.wmi_record_lock);
	wmi_handle->log_info.wmi_logging_enable = 1;

	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * wmi_log_buffer_free() - Free all dynamic allocated buffer memory for
 * event logging
 * @wmi_handle: WMI handle.
 *
 * Return: None
 */
#ifndef CONFIG_MCL
static inline void wmi_log_buffer_free(struct wmi_unified *wmi_handle)
{
	if (wmi_handle->log_info.wmi_command_log_buf_info.buf)
		qdf_mem_free(wmi_handle->log_info.wmi_command_log_buf_info.buf);
	if (wmi_handle->log_info.wmi_command_tx_cmp_log_buf_info.buf)
		qdf_mem_free(
		wmi_handle->log_info.wmi_command_tx_cmp_log_buf_info.buf);
	if (wmi_handle->log_info.wmi_event_log_buf_info.buf)
		qdf_mem_free(wmi_handle->log_info.wmi_event_log_buf_info.buf);
	if (wmi_handle->log_info.wmi_rx_event_log_buf_info.buf)
		qdf_mem_free(
			wmi_handle->log_info.wmi_rx_event_log_buf_info.buf);
	if (wmi_handle->log_info.wmi_mgmt_command_log_buf_info.buf)
		qdf_mem_free(
			wmi_handle->log_info.wmi_mgmt_command_log_buf_info.buf);
	if (wmi_handle->log_info.wmi_mgmt_command_tx_cmp_log_buf_info.buf)
		qdf_mem_free(
		wmi_handle->log_info.wmi_mgmt_command_tx_cmp_log_buf_info.buf);
	if (wmi_handle->log_info.wmi_mgmt_event_log_buf_info.buf)
		qdf_mem_free(
			wmi_handle->log_info.wmi_mgmt_event_log_buf_info.buf);
	wmi_handle->log_info.wmi_logging_enable = 0;
	qdf_spinlock_destroy(&wmi_handle->log_info.wmi_record_lock);
}
#else
static inline void wmi_log_buffer_free(struct wmi_unified *wmi_handle)
{
	/* Do Nothing */
}
#endif

/**
 * wmi_print_cmd_log_buffer() - an output agnostic wmi command log printer
 * @log_buffer: the command log buffer metadata of the buffer to print
 * @count: the maximum number of entries to print
 * @print: an abstract print method, e.g. a qdf_print() or seq_printf() wrapper
 * @print_priv: any data required by the print method, e.g. a file handle
 *
 * Return: None
 */
static void
wmi_print_cmd_log_buffer(struct wmi_log_buf_t *log_buffer, uint32_t count,
			 qdf_abstract_print *print, void *print_priv)
{
	static const int data_len =
		WMI_EVENT_DEBUG_ENTRY_MAX_LENGTH / sizeof(uint32_t);
	char str[128];
	uint32_t idx;

	if (count > log_buffer->size)
		count = log_buffer->size;
	if (count > log_buffer->length)
		count = log_buffer->length;

	/* subtract count from index, and wrap if necessary */
	idx = log_buffer->size + *log_buffer->p_buf_tail_idx - count;
	idx %= log_buffer->size;

	print(print_priv, "Time (seconds)      Cmd Id              Payload");
	while (count) {
		struct wmi_command_debug *cmd_log = (struct wmi_command_debug *)
			&((struct wmi_command_debug *)log_buffer->buf)[idx];
		uint64_t secs, usecs;
		int len = 0;
		int i;

		qdf_log_timestamp_to_secs(cmd_log->time, &secs, &usecs);
		len += scnprintf(str + len, sizeof(str) - len,
				 "% 8lld.%06lld    %6u (0x%06x)    ",
				 secs, usecs,
				 cmd_log->command, cmd_log->command);
		for (i = 0; i < data_len; ++i) {
			len += scnprintf(str + len, sizeof(str) - len,
					 "0x%08x ", cmd_log->data[i]);
		}

		print(print_priv, str);

		--count;
		++idx;
		if (idx >= log_buffer->size)
			idx = 0;
	}
}

/**
 * wmi_print_event_log_buffer() - an output agnostic wmi event log printer
 * @log_buffer: the event log buffer metadata of the buffer to print
 * @count: the maximum number of entries to print
 * @print: an abstract print method, e.g. a qdf_print() or seq_printf() wrapper
 * @print_priv: any data required by the print method, e.g. a file handle
 *
 * Return: None
 */
static void
wmi_print_event_log_buffer(struct wmi_log_buf_t *log_buffer, uint32_t count,
			   qdf_abstract_print *print, void *print_priv)
{
	static const int data_len =
		WMI_EVENT_DEBUG_ENTRY_MAX_LENGTH / sizeof(uint32_t);
	char str[128];
	uint32_t idx;

	if (count > log_buffer->size)
		count = log_buffer->size;
	if (count > log_buffer->length)
		count = log_buffer->length;

	/* subtract count from index, and wrap if necessary */
	idx = log_buffer->size + *log_buffer->p_buf_tail_idx - count;
	idx %= log_buffer->size;

	print(print_priv, "Time (seconds)      Event Id             Payload");
	while (count) {
		struct wmi_event_debug *event_log = (struct wmi_event_debug *)
			&((struct wmi_event_debug *)log_buffer->buf)[idx];
		uint64_t secs, usecs;
		int len = 0;
		int i;

		qdf_log_timestamp_to_secs(event_log->time, &secs, &usecs);
		len += scnprintf(str + len, sizeof(str) - len,
				 "% 8lld.%06lld    %6u (0x%06x)    ",
				 secs, usecs,
				 event_log->event, event_log->event);
		for (i = 0; i < data_len; ++i) {
			len += scnprintf(str + len, sizeof(str) - len,
					 "0x%08x ", event_log->data[i]);
		}

		print(print_priv, str);

		--count;
		++idx;
		if (idx >= log_buffer->size)
			idx = 0;
	}
}

inline void
wmi_print_cmd_log(wmi_unified_t wmi, uint32_t count,
		  qdf_abstract_print *print, void *print_priv)
{
	wmi_print_cmd_log_buffer(
		&wmi->log_info.wmi_command_log_buf_info,
		count, print, print_priv);
}

inline void
wmi_print_cmd_tx_cmp_log(wmi_unified_t wmi, uint32_t count,
			 qdf_abstract_print *print, void *print_priv)
{
	wmi_print_cmd_log_buffer(
		&wmi->log_info.wmi_command_tx_cmp_log_buf_info,
		count, print, print_priv);
}

inline void
wmi_print_mgmt_cmd_log(wmi_unified_t wmi, uint32_t count,
		       qdf_abstract_print *print, void *print_priv)
{
	wmi_print_cmd_log_buffer(
		&wmi->log_info.wmi_mgmt_command_log_buf_info,
		count, print, print_priv);
}

inline void
wmi_print_mgmt_cmd_tx_cmp_log(wmi_unified_t wmi, uint32_t count,
			      qdf_abstract_print *print, void *print_priv)
{
	wmi_print_cmd_log_buffer(
		&wmi->log_info.wmi_mgmt_command_tx_cmp_log_buf_info,
		count, print, print_priv);
}

inline void
wmi_print_event_log(wmi_unified_t wmi, uint32_t count,
		    qdf_abstract_print *print, void *print_priv)
{
	wmi_print_event_log_buffer(
		&wmi->log_info.wmi_event_log_buf_info,
		count, print, print_priv);
}

inline void
wmi_print_rx_event_log(wmi_unified_t wmi, uint32_t count,
		       qdf_abstract_print *print, void *print_priv)
{
	wmi_print_event_log_buffer(
		&wmi->log_info.wmi_rx_event_log_buf_info,
		count, print, print_priv);
}

inline void
wmi_print_mgmt_event_log(wmi_unified_t wmi, uint32_t count,
			 qdf_abstract_print *print, void *print_priv)
{
	wmi_print_event_log_buffer(
		&wmi->log_info.wmi_mgmt_event_log_buf_info,
		count, print, print_priv);
}

#ifdef CONFIG_MCL
const int8_t * const debugfs_dir[MAX_WMI_INSTANCES] = {"WMI0"};
#else
const int8_t * const debugfs_dir[MAX_WMI_INSTANCES] = {"WMI0", "WMI1", "WMI2"};
#endif

/* debugfs routines*/

/**
 * debug_wmi_##func_base##_show() - debugfs functions to display content of
 * command and event buffers. Macro uses max buffer length to display
 * buffer when it is wraparound.
 *
 * @m: debugfs handler to access wmi_handle
 * @v: Variable arguments (not used)
 *
 * Return: Length of characters printed
 */
#define GENERATE_COMMAND_DEBUG_SHOW_FUNCS(func_base, wmi_ring_size)	\
	static int debug_wmi_##func_base##_show(struct seq_file *m,	\
						void *v)		\
	{								\
		wmi_unified_t wmi_handle = (wmi_unified_t) m->private;	\
		struct wmi_log_buf_t *wmi_log =				\
			&wmi_handle->log_info.wmi_##func_base##_buf_info;\
		int pos, nread, outlen;					\
		int i;							\
		uint64_t secs, usecs;					\
									\
		qdf_spin_lock(&wmi_handle->log_info.wmi_record_lock);	\
		if (!wmi_log->length) {					\
			qdf_spin_unlock(&wmi_handle->log_info.wmi_record_lock);\
			return wmi_bp_seq_printf(m,			\
			"no elements to read from ring buffer!\n");	\
		}							\
									\
		if (wmi_log->length <= wmi_ring_size)			\
			nread = wmi_log->length;			\
		else							\
			nread = wmi_ring_size;				\
									\
		if (*(wmi_log->p_buf_tail_idx) == 0)			\
			/* tail can be 0 after wrap-around */		\
			pos = wmi_ring_size - 1;			\
		else							\
			pos = *(wmi_log->p_buf_tail_idx) - 1;		\
									\
		outlen = wmi_bp_seq_printf(m, "Length = %d\n", wmi_log->length);\
		qdf_spin_unlock(&wmi_handle->log_info.wmi_record_lock);	\
		while (nread--) {					\
			struct wmi_command_debug *wmi_record;		\
									\
			wmi_record = (struct wmi_command_debug *)	\
			&(((struct wmi_command_debug *)wmi_log->buf)[pos]);\
			outlen += wmi_bp_seq_printf(m, "CMD ID = %x\n",	\
				(wmi_record->command));			\
			qdf_log_timestamp_to_secs(wmi_record->time, &secs,\
				&usecs);				\
			outlen +=					\
			wmi_bp_seq_printf(m, "CMD TIME = [%llu.%06llu]\n",\
				secs, usecs);				\
			outlen += wmi_bp_seq_printf(m, "CMD = ");	\
			for (i = 0; i < (wmi_record_max_length/		\
					sizeof(uint32_t)); i++)		\
				outlen += wmi_bp_seq_printf(m, "%x ",	\
					wmi_record->data[i]);		\
			outlen += wmi_bp_seq_printf(m, "\n");		\
									\
			if (pos == 0)					\
				pos = wmi_ring_size - 1;		\
			else						\
				pos--;					\
		}							\
		return outlen;						\
	}								\

#define GENERATE_EVENT_DEBUG_SHOW_FUNCS(func_base, wmi_ring_size)	\
	static int debug_wmi_##func_base##_show(struct seq_file *m,	\
						void *v)		\
	{								\
		wmi_unified_t wmi_handle = (wmi_unified_t) m->private;	\
		struct wmi_log_buf_t *wmi_log =				\
			&wmi_handle->log_info.wmi_##func_base##_buf_info;\
		int pos, nread, outlen;					\
		int i;							\
		uint64_t secs, usecs;					\
									\
		qdf_spin_lock(&wmi_handle->log_info.wmi_record_lock);	\
		if (!wmi_log->length) {					\
			qdf_spin_unlock(&wmi_handle->log_info.wmi_record_lock);\
			return wmi_bp_seq_printf(m,			\
			"no elements to read from ring buffer!\n");	\
		}							\
									\
		if (wmi_log->length <= wmi_ring_size)			\
			nread = wmi_log->length;			\
		else							\
			nread = wmi_ring_size;				\
									\
		if (*(wmi_log->p_buf_tail_idx) == 0)			\
			/* tail can be 0 after wrap-around */		\
			pos = wmi_ring_size - 1;			\
		else							\
			pos = *(wmi_log->p_buf_tail_idx) - 1;		\
									\
		outlen = wmi_bp_seq_printf(m, "Length = %d\n", wmi_log->length);\
		qdf_spin_unlock(&wmi_handle->log_info.wmi_record_lock);	\
		while (nread--) {					\
			struct wmi_event_debug *wmi_record;		\
									\
			wmi_record = (struct wmi_event_debug *)		\
			&(((struct wmi_event_debug *)wmi_log->buf)[pos]);\
			qdf_log_timestamp_to_secs(wmi_record->time, &secs,\
				&usecs);				\
			outlen += wmi_bp_seq_printf(m, "Event ID = %x\n",\
				(wmi_record->event));			\
			outlen +=					\
			wmi_bp_seq_printf(m, "Event TIME = [%llu.%06llu]\n",\
				secs, usecs);				\
			outlen += wmi_bp_seq_printf(m, "CMD = ");	\
			for (i = 0; i < (wmi_record_max_length/		\
					sizeof(uint32_t)); i++)		\
				outlen += wmi_bp_seq_printf(m, "%x ",	\
					wmi_record->data[i]);		\
			outlen += wmi_bp_seq_printf(m, "\n");		\
									\
			if (pos == 0)					\
				pos = wmi_ring_size - 1;		\
			else						\
				pos--;					\
		}							\
		return outlen;						\
	}

GENERATE_COMMAND_DEBUG_SHOW_FUNCS(command_log, wmi_display_size);
GENERATE_COMMAND_DEBUG_SHOW_FUNCS(command_tx_cmp_log, wmi_display_size);
GENERATE_EVENT_DEBUG_SHOW_FUNCS(event_log, wmi_display_size);
GENERATE_EVENT_DEBUG_SHOW_FUNCS(rx_event_log, wmi_display_size);
GENERATE_COMMAND_DEBUG_SHOW_FUNCS(mgmt_command_log, wmi_display_size);
GENERATE_COMMAND_DEBUG_SHOW_FUNCS(mgmt_command_tx_cmp_log,
					wmi_display_size);
GENERATE_EVENT_DEBUG_SHOW_FUNCS(mgmt_event_log, wmi_display_size);

/**
 * debug_wmi_enable_show() - debugfs functions to display enable state of
 * wmi logging feature.
 *
 * @m: debugfs handler to access wmi_handle
 * @v: Variable arguments (not used)
 *
 * Return: always 1
 */
static int debug_wmi_enable_show(struct seq_file *m, void *v)
{
	wmi_unified_t wmi_handle = (wmi_unified_t) m->private;

	return wmi_bp_seq_printf(m, "%d\n",
			wmi_handle->log_info.wmi_logging_enable);
}

/**
 * debug_wmi_log_size_show() - debugfs functions to display configured size of
 * wmi logging command/event buffer and management command/event buffer.
 *
 * @m: debugfs handler to access wmi_handle
 * @v: Variable arguments (not used)
 *
 * Return: Length of characters printed
 */
static int debug_wmi_log_size_show(struct seq_file *m, void *v)
{

	wmi_bp_seq_printf(m, "WMI command/event log max size:%d\n",
				wmi_log_max_entry);
	return wmi_bp_seq_printf(m,
			"WMI management command/events log max size:%d\n",
			wmi_mgmt_log_max_entry);
}

/**
 * debug_wmi_##func_base##_write() - debugfs functions to clear
 * wmi logging command/event buffer and management command/event buffer.
 *
 * @file: file handler to access wmi_handle
 * @buf: received data buffer
 * @count: length of received buffer
 * @ppos: Not used
 *
 * Return: count
 */
#define GENERATE_DEBUG_WRITE_FUNCS(func_base, wmi_ring_size, wmi_record_type)\
	static ssize_t debug_wmi_##func_base##_write(struct file *file,	\
				const char __user *buf,			\
				size_t count, loff_t *ppos)		\
	{								\
		int k, ret;						\
		wmi_unified_t wmi_handle =				\
			((struct seq_file *)file->private_data)->private;\
		struct wmi_log_buf_t *wmi_log = &wmi_handle->log_info.	\
				wmi_##func_base##_buf_info;		\
		char locbuf[50];					\
									\
		if ((!buf) || (count > 50))				\
			return -EFAULT;					\
									\
		if (copy_from_user(locbuf, buf, count))			\
			return -EFAULT;					\
									\
		ret = sscanf(locbuf, "%d", &k);				\
		if ((ret != 1) || (k != 0)) {                           \
			qdf_print("Wrong input, echo 0 to clear the wmi	buffer\n");\
			return -EINVAL;					\
		}							\
									\
		qdf_spin_lock(&wmi_handle->log_info.wmi_record_lock);	\
		qdf_mem_zero(wmi_log->buf, wmi_ring_size *		\
				sizeof(struct wmi_record_type));	\
		wmi_log->length = 0;					\
		*(wmi_log->p_buf_tail_idx) = 0;				\
		qdf_spin_unlock(&wmi_handle->log_info.wmi_record_lock);	\
									\
		return count;						\
	}

GENERATE_DEBUG_WRITE_FUNCS(command_log, wmi_log_max_entry,
					wmi_command_debug);
GENERATE_DEBUG_WRITE_FUNCS(command_tx_cmp_log, wmi_log_max_entry,
					wmi_command_debug);
GENERATE_DEBUG_WRITE_FUNCS(event_log, wmi_log_max_entry,
					wmi_event_debug);
GENERATE_DEBUG_WRITE_FUNCS(rx_event_log, wmi_log_max_entry,
					wmi_event_debug);
GENERATE_DEBUG_WRITE_FUNCS(mgmt_command_log, wmi_mgmt_log_max_entry,
					wmi_command_debug);
GENERATE_DEBUG_WRITE_FUNCS(mgmt_command_tx_cmp_log,
		wmi_mgmt_log_max_entry, wmi_command_debug);
GENERATE_DEBUG_WRITE_FUNCS(mgmt_event_log, wmi_mgmt_log_max_entry,
					wmi_event_debug);

/**
 * debug_wmi_enable_write() - debugfs functions to enable/disable
 * wmi logging feature.
 *
 * @file: file handler to access wmi_handle
 * @buf: received data buffer
 * @count: length of received buffer
 * @ppos: Not used
 *
 * Return: count
 */
static ssize_t debug_wmi_enable_write(struct file *file, const char __user *buf,
					size_t count, loff_t *ppos)
{
	wmi_unified_t wmi_handle =
		((struct seq_file *)file->private_data)->private;
	int k, ret;
	char locbuf[50];

	if ((!buf) || (count > 50))
		return -EFAULT;

	if (copy_from_user(locbuf, buf, count))
		return -EFAULT;

	ret = sscanf(locbuf, "%d", &k);
	if ((ret != 1) || ((k != 0) && (k != 1)))
		return -EINVAL;

	wmi_handle->log_info.wmi_logging_enable = k;
	return count;
}

/**
 * debug_wmi_log_size_write() - reserved.
 *
 * @file: file handler to access wmi_handle
 * @buf: received data buffer
 * @count: length of received buffer
 * @ppos: Not used
 *
 * Return: count
 */
static ssize_t debug_wmi_log_size_write(struct file *file,
		const char __user *buf, size_t count, loff_t *ppos)
{
	return -EINVAL;
}

/* Structure to maintain debug information */
struct wmi_debugfs_info {
	const char *name;
	struct dentry *de[MAX_WMI_INSTANCES];
	const struct file_operations *ops;
};

#define DEBUG_FOO(func_base) { .name = #func_base,			\
	.ops = &debug_##func_base##_ops }

/**
 * debug_##func_base##_open() - Open debugfs entry for respective command
 * and event buffer.
 *
 * @inode: node for debug dir entry
 * @file: file handler
 *
 * Return: open status
 */
#define GENERATE_DEBUG_STRUCTS(func_base)				\
	static int debug_##func_base##_open(struct inode *inode,	\
						struct file *file)	\
	{								\
		return single_open(file, debug_##func_base##_show,	\
				inode->i_private);			\
	}								\
									\
									\
	static struct file_operations debug_##func_base##_ops = {	\
		.open		= debug_##func_base##_open,		\
		.read		= seq_read,				\
		.llseek		= seq_lseek,				\
		.write		= debug_##func_base##_write,		\
		.release	= single_release,			\
	};

GENERATE_DEBUG_STRUCTS(wmi_command_log);
GENERATE_DEBUG_STRUCTS(wmi_command_tx_cmp_log);
GENERATE_DEBUG_STRUCTS(wmi_event_log);
GENERATE_DEBUG_STRUCTS(wmi_rx_event_log);
GENERATE_DEBUG_STRUCTS(wmi_mgmt_command_log);
GENERATE_DEBUG_STRUCTS(wmi_mgmt_command_tx_cmp_log);
GENERATE_DEBUG_STRUCTS(wmi_mgmt_event_log);
GENERATE_DEBUG_STRUCTS(wmi_enable);
GENERATE_DEBUG_STRUCTS(wmi_log_size);

struct wmi_debugfs_info wmi_debugfs_infos[] = {
	DEBUG_FOO(wmi_command_log),
	DEBUG_FOO(wmi_command_tx_cmp_log),
	DEBUG_FOO(wmi_event_log),
	DEBUG_FOO(wmi_rx_event_log),
	DEBUG_FOO(wmi_mgmt_command_log),
	DEBUG_FOO(wmi_mgmt_command_tx_cmp_log),
	DEBUG_FOO(wmi_mgmt_event_log),
	DEBUG_FOO(wmi_enable),
	DEBUG_FOO(wmi_log_size),
};

#define NUM_DEBUG_INFOS (sizeof(wmi_debugfs_infos) /			\
		sizeof(wmi_debugfs_infos[0]))

/**
 * wmi_debugfs_create() - Create debug_fs entry for wmi logging.
 *
 * @wmi_handle: wmi handle
 * @par_entry: debug directory entry
 * @id: Index to debug info data array
 *
 * Return: none
 */
static void wmi_debugfs_create(wmi_unified_t wmi_handle,
		struct dentry *par_entry, int id)
{
	int i;

	if (par_entry == NULL || (id < 0) || (id >= MAX_WMI_INSTANCES))
		goto out;

	for (i = 0; i < NUM_DEBUG_INFOS; ++i) {

		wmi_debugfs_infos[i].de[id] = debugfs_create_file(
				wmi_debugfs_infos[i].name, 0644, par_entry,
				wmi_handle, wmi_debugfs_infos[i].ops);

		if (wmi_debugfs_infos[i].de[id] == NULL) {
			qdf_print("%s: debug Entry creation failed!\n",
					__func__);
			goto out;
		}
	}

	return;

out:
	qdf_print("%s: debug Entry creation failed!\n", __func__);
	wmi_log_buffer_free(wmi_handle);
	return;
}

/**
 * wmi_debugfs_remove() - Remove debugfs entry for wmi logging.
 * @wmi_handle: wmi handle
 * @dentry: debugfs directory entry
 * @id: Index to debug info data array
 *
 * Return: none
 */
static void wmi_debugfs_remove(wmi_unified_t wmi_handle)
{
	int i;
	struct dentry *dentry = wmi_handle->log_info.wmi_log_debugfs_dir;
	int id;

	if (!wmi_handle->log_info.wmi_instance_id)
		return;

	id = wmi_handle->log_info.wmi_instance_id - 1;
	if (dentry && (!(id < 0) || (id >= MAX_WMI_INSTANCES))) {
		for (i = 0; i < NUM_DEBUG_INFOS; ++i) {
			if (wmi_debugfs_infos[i].de[id])
				wmi_debugfs_infos[i].de[id] = NULL;
		}
	}

	if (dentry)
		debugfs_remove_recursive(dentry);

	if (wmi_handle->log_info.wmi_instance_id)
		wmi_handle->log_info.wmi_instance_id--;
}

/**
 * wmi_debugfs_init() - debugfs functions to create debugfs directory and to
 * create debugfs enteries.
 *
 * @h: wmi handler
 *
 * Return: init status
 */
static QDF_STATUS wmi_debugfs_init(wmi_unified_t wmi_handle)
{
	int wmi_index = wmi_handle->log_info.wmi_instance_id;

	if (wmi_index < MAX_WMI_INSTANCES) {
		wmi_handle->log_info.wmi_log_debugfs_dir =
			debugfs_create_dir(debugfs_dir[wmi_index], NULL);

		if (!wmi_handle->log_info.wmi_log_debugfs_dir) {
			qdf_print("error while creating debugfs dir for %s\n",
				  debugfs_dir[wmi_index]);
			return QDF_STATUS_E_FAILURE;
		}

		wmi_debugfs_create(wmi_handle,
				   wmi_handle->log_info.wmi_log_debugfs_dir,
				   wmi_index);
		wmi_handle->log_info.wmi_instance_id++;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_mgmt_cmd_record() - Wrapper function for mgmt command logging macro
 *
 * @wmi_handle: wmi handle
 * @cmd: mgmt command
 * @header: pointer to 802.11 header
 * @vdev_id: vdev id
 * @chanfreq: channel frequency
 *
 * Return: none
 */
void wmi_mgmt_cmd_record(wmi_unified_t wmi_handle, uint32_t cmd,
			void *header, uint32_t vdev_id, uint32_t chanfreq)
{

	uint32_t data[CUSTOM_MGMT_CMD_DATA_SIZE];

	data[0] = ((struct wmi_command_header *)header)->type;
	data[1] = ((struct wmi_command_header *)header)->sub_type;
	data[2] = vdev_id;
	data[3] = chanfreq;

	qdf_spin_lock_bh(&wmi_handle->log_info.wmi_record_lock);

	WMI_MGMT_COMMAND_RECORD(wmi_handle, cmd, (uint8_t *)data);

	qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);
}
#else
/**
 * wmi_debugfs_remove() - Remove debugfs entry for wmi logging.
 * @wmi_handle: wmi handle
 * @dentry: debugfs directory entry
 * @id: Index to debug info data array
 *
 * Return: none
 */
static void wmi_debugfs_remove(wmi_unified_t wmi_handle) { }
void wmi_mgmt_cmd_record(wmi_unified_t wmi_handle, uint32_t cmd,
			void *header, uint32_t vdev_id, uint32_t chanfreq) { }
static inline void wmi_log_buffer_free(struct wmi_unified *wmi_handle) { }
#endif /*WMI_INTERFACE_EVENT_LOGGING */
qdf_export_symbol(wmi_mgmt_cmd_record);

int wmi_get_host_credits(wmi_unified_t wmi_handle);
/* WMI buffer APIs */

#ifdef MEMORY_DEBUG
wmi_buf_t
wmi_buf_alloc_debug(wmi_unified_t wmi_handle, uint16_t len, uint8_t *file_name,
			uint32_t line_num)
{
	wmi_buf_t wmi_buf;

	if (roundup(len + WMI_MIN_HEAD_ROOM, 4) > wmi_handle->max_msg_len) {
		QDF_ASSERT(0);
		return NULL;
	}

	wmi_buf = qdf_nbuf_alloc_debug(NULL,
					roundup(len + WMI_MIN_HEAD_ROOM, 4),
					WMI_MIN_HEAD_ROOM, 4, false, file_name,
					line_num);

	if (!wmi_buf)
		return NULL;

	/* Clear the wmi buffer */
	OS_MEMZERO(qdf_nbuf_data(wmi_buf), len);

	/*
	 * Set the length of the buffer to match the allocation size.
	 */
	qdf_nbuf_set_pktlen(wmi_buf, len);

	return wmi_buf;
}
qdf_export_symbol(wmi_buf_alloc_debug);

void wmi_buf_free(wmi_buf_t net_buf)
{
	qdf_nbuf_free(net_buf);
}
qdf_export_symbol(wmi_buf_free);
#else
wmi_buf_t wmi_buf_alloc(wmi_unified_t wmi_handle, uint16_t len)
{
	wmi_buf_t wmi_buf;

	if (roundup(len + WMI_MIN_HEAD_ROOM, 4) > wmi_handle->max_msg_len) {
		QDF_ASSERT(0);
		return NULL;
	}

	wmi_buf = qdf_nbuf_alloc(NULL, roundup(len + WMI_MIN_HEAD_ROOM, 4),
				WMI_MIN_HEAD_ROOM, 4, false);
	if (!wmi_buf)
		return NULL;

	/* Clear the wmi buffer */
	OS_MEMZERO(qdf_nbuf_data(wmi_buf), len);

	/*
	 * Set the length of the buffer to match the allocation size.
	 */
	qdf_nbuf_set_pktlen(wmi_buf, len);
	return wmi_buf;
}
qdf_export_symbol(wmi_buf_alloc);

void wmi_buf_free(wmi_buf_t net_buf)
{
	qdf_nbuf_free(net_buf);
}
qdf_export_symbol(wmi_buf_free);
#endif

/**
 * wmi_get_max_msg_len() - get maximum WMI message length
 * @wmi_handle: WMI handle.
 *
 * This function returns the maximum WMI message length
 *
 * Return: maximum WMI message length
 */
uint16_t wmi_get_max_msg_len(wmi_unified_t wmi_handle)
{
	return wmi_handle->max_msg_len - WMI_MIN_HEAD_ROOM;
}
qdf_export_symbol(wmi_get_max_msg_len);

#ifndef WMI_CMD_STRINGS
static uint8_t *wmi_id_to_name(uint32_t wmi_command)
{
	return "Invalid WMI cmd";
}

#endif

#ifdef CONFIG_MCL
static inline void wmi_log_cmd_id(uint32_t cmd_id, uint32_t tag)
{
	WMI_LOGD("Send WMI command:%s command_id:%d htc_tag:%d\n",
		 wmi_id_to_name(cmd_id), cmd_id, tag);
}

/**
 * wmi_is_pm_resume_cmd() - check if a cmd is part of the resume sequence
 * @cmd_id: command to check
 *
 * Return: true if the command is part of the resume sequence.
 */
static bool wmi_is_pm_resume_cmd(uint32_t cmd_id)
{
	switch (cmd_id) {
	case WMI_WOW_HOSTWAKEUP_FROM_SLEEP_CMDID:
	case WMI_PDEV_RESUME_CMDID:
		return true;

	default:
		return false;
	}
}
#else
static bool wmi_is_pm_resume_cmd(uint32_t cmd_id)
{
	return false;
}
#endif

/**
 * wmi_unified_cmd_send() - WMI command API
 * @wmi_handle: handle to wmi
 * @buf: wmi buf
 * @len: wmi buffer length
 * @cmd_id: wmi command id
 *
 * Note, it is NOT safe to access buf after calling this function!
 *
 * Return: 0 on success
 */
QDF_STATUS wmi_unified_cmd_send(wmi_unified_t wmi_handle, wmi_buf_t buf,
				uint32_t len, uint32_t cmd_id)
{
	HTC_PACKET *pkt;
	QDF_STATUS status;
	uint16_t htc_tag = 0;

	if (wmi_get_runtime_pm_inprogress(wmi_handle)) {
		htc_tag =
			(A_UINT16)wmi_handle->ops->wmi_set_htc_tx_tag(
						wmi_handle, buf, cmd_id);
	} else if (qdf_atomic_read(&wmi_handle->is_target_suspended) &&
		(!wmi_is_pm_resume_cmd(cmd_id))) {
		QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_ERROR,
				  "%s: Target is suspended", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_BUSY;
	}
	if (wmi_handle->wmi_stopinprogress) {
		QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_ERROR,
			"WMI  stop in progress\n");
		return QDF_STATUS_E_INVAL;
	}

#ifndef WMI_NON_TLV_SUPPORT
	/* Do sanity check on the TLV parameter structure */
	if (wmi_handle->target_type == WMI_TLV_TARGET) {
		void *buf_ptr = (void *)qdf_nbuf_data(buf);

		if (wmi_handle->ops->wmi_check_command_params(NULL, buf_ptr, len, cmd_id)
			!= 0) {
			QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_ERROR,
			"\nERROR: %s: Invalid WMI Param Buffer for Cmd:%d",
				__func__, cmd_id);
			return QDF_STATUS_E_INVAL;
		}
	}
#endif

	if (qdf_nbuf_push_head(buf, sizeof(WMI_CMD_HDR)) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_ERROR,
			 "%s, Failed to send cmd %x, no memory",
			 __func__, cmd_id);
		return QDF_STATUS_E_NOMEM;
	}

	WMI_SET_FIELD(qdf_nbuf_data(buf), WMI_CMD_HDR, COMMANDID, cmd_id);

	qdf_atomic_inc(&wmi_handle->pending_cmds);
	if (qdf_atomic_read(&wmi_handle->pending_cmds) >= WMI_MAX_CMDS) {
		QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_ERROR,
		    "\n%s: hostcredits = %d", __func__,
		wmi_get_host_credits(wmi_handle));
		htc_dump_counter_info(wmi_handle->htc_handle);
		qdf_atomic_dec(&wmi_handle->pending_cmds);
		QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_ERROR,
			"%s: MAX %d WMI Pending cmds reached.", __func__,
			WMI_MAX_CMDS);
		QDF_BUG(0);
		return QDF_STATUS_E_BUSY;
	}

	pkt = qdf_mem_malloc(sizeof(*pkt));
	if (!pkt) {
		qdf_atomic_dec(&wmi_handle->pending_cmds);
		QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_ERROR,
			 "%s, Failed to alloc htc packet %x, no memory",
			 __func__, cmd_id);
		return QDF_STATUS_E_NOMEM;
	}

	SET_HTC_PACKET_INFO_TX(pkt,
			       NULL,
			       qdf_nbuf_data(buf), len + sizeof(WMI_CMD_HDR),
			       wmi_handle->wmi_endpoint_id, htc_tag);

	SET_HTC_PACKET_NET_BUF_CONTEXT(pkt, buf);
#ifdef CONFIG_MCL
	wmi_log_cmd_id(cmd_id, htc_tag);
#endif

#ifdef WMI_INTERFACE_EVENT_LOGGING
	if (wmi_handle->log_info.wmi_logging_enable) {
		qdf_spin_lock_bh(&wmi_handle->log_info.wmi_record_lock);
		/*
		 * Record 16 bytes of WMI cmd data -
		 * exclude TLV and WMI headers
		 *
		 * WMI mgmt command already recorded in wmi_mgmt_cmd_record
		 */
		if (wmi_handle->ops->is_management_record(cmd_id) == false) {
			WMI_COMMAND_RECORD(wmi_handle, cmd_id,
					qdf_nbuf_data(buf) +
			 wmi_handle->log_info.buf_offset_command);
		}
		qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);
	}
#endif

	status = htc_send_pkt(wmi_handle->htc_handle, pkt);

	if (QDF_STATUS_SUCCESS != status) {
		qdf_atomic_dec(&wmi_handle->pending_cmds);
		QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_ERROR,
		   "%s %d, htc_send_pkt failed", __func__, __LINE__);
		qdf_mem_free(pkt);
		return status;
	}

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wmi_unified_cmd_send);

/**
 * wmi_unified_get_event_handler_ix() - gives event handler's index
 * @wmi_handle: handle to wmi
 * @event_id: wmi  event id
 *
 * Return: event handler's index
 */
static int wmi_unified_get_event_handler_ix(wmi_unified_t wmi_handle,
					    uint32_t event_id)
{
	uint32_t idx = 0;
	int32_t invalid_idx = -1;
	struct wmi_soc *soc = wmi_handle->soc;

	for (idx = 0; (idx < soc->max_event_idx &&
		       idx < WMI_UNIFIED_MAX_EVENT); ++idx) {
		if (wmi_handle->event_id[idx] == event_id &&
		    wmi_handle->event_handler[idx] != NULL) {
			return idx;
		}
	}

	return invalid_idx;
}

/**
 * wmi_unified_register_event() - register wmi event handler
 * @wmi_handle: handle to wmi
 * @event_id: wmi event id
 * @handler_func: wmi event handler function
 *
 * Return: 0 on success
 */
int wmi_unified_register_event(wmi_unified_t wmi_handle,
				       uint32_t event_id,
				       wmi_unified_event_handler handler_func)
{
	uint32_t idx = 0;
	uint32_t evt_id;
	struct wmi_soc *soc = wmi_handle->soc;

	if (event_id >= wmi_events_max ||
		wmi_handle->wmi_events[event_id] == WMI_EVENT_ID_INVALID) {
		qdf_print("%s: Event id %d is unavailable\n",
				 __func__, event_id);
		return QDF_STATUS_E_FAILURE;
	}
	evt_id = wmi_handle->wmi_events[event_id];
	if (wmi_unified_get_event_handler_ix(wmi_handle, evt_id) != -1) {
		qdf_print("%s : event handler already registered 0x%x\n",
		       __func__, evt_id);
		return QDF_STATUS_E_FAILURE;
	}
	if (soc->max_event_idx == WMI_UNIFIED_MAX_EVENT) {
		qdf_print("%s : no more event handlers 0x%x\n",
		       __func__, evt_id);
		return QDF_STATUS_E_FAILURE;
	}
	idx = soc->max_event_idx;
	wmi_handle->event_handler[idx] = handler_func;
	wmi_handle->event_id[idx] = evt_id;
	qdf_spin_lock_bh(&soc->ctx_lock);
	wmi_handle->ctx[idx] = WMI_RX_UMAC_CTX;
	qdf_spin_unlock_bh(&soc->ctx_lock);
	soc->max_event_idx++;

	return 0;
}

/**
 * wmi_unified_register_event_handler() - register wmi event handler
 * @wmi_handle: handle to wmi
 * @event_id: wmi event id
 * @handler_func: wmi event handler function
 * @rx_ctx: rx execution context for wmi rx events
 *
 * This API is to support legacy requirements. Will be deprecated in future.
 * Return: 0 on success
 */
int wmi_unified_register_event_handler(wmi_unified_t wmi_handle,
				       wmi_conv_event_id event_id,
				       wmi_unified_event_handler handler_func,
				       uint8_t rx_ctx)
{
	uint32_t idx = 0;
	uint32_t evt_id;
	struct wmi_soc *soc = wmi_handle->soc;

	if (event_id >= wmi_events_max ||
		wmi_handle->wmi_events[event_id] == WMI_EVENT_ID_INVALID) {
		qdf_print("%s: Event id %d is unavailable\n",
				 __func__, event_id);
		return QDF_STATUS_E_FAILURE;
	}
	evt_id = wmi_handle->wmi_events[event_id];

	if (wmi_unified_get_event_handler_ix(wmi_handle, evt_id) != -1) {
		qdf_print("%s : event handler already registered 0x%x\n",
		       __func__, evt_id);
		return QDF_STATUS_E_FAILURE;
	}
	if (soc->max_event_idx == WMI_UNIFIED_MAX_EVENT) {
		qdf_print("%s : no more event handlers 0x%x\n",
		       __func__, evt_id);
		return QDF_STATUS_E_FAILURE;
	}
	QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			"Registered event handler for event 0x%8x\n", evt_id);
	idx = soc->max_event_idx;
	wmi_handle->event_handler[idx] = handler_func;
	wmi_handle->event_id[idx] = evt_id;
	qdf_spin_lock_bh(&soc->ctx_lock);
	wmi_handle->ctx[idx] = rx_ctx;
	qdf_spin_unlock_bh(&soc->ctx_lock);
	soc->max_event_idx++;

	return 0;
}
qdf_export_symbol(wmi_unified_register_event_handler);

/**
 * wmi_unified_unregister_event() - unregister wmi event handler
 * @wmi_handle: handle to wmi
 * @event_id: wmi event id
 *
 * Return: 0 on success
 */
int wmi_unified_unregister_event(wmi_unified_t wmi_handle,
					 uint32_t event_id)
{
	uint32_t idx = 0;
	uint32_t evt_id;
	struct wmi_soc *soc = wmi_handle->soc;

	if (event_id >= wmi_events_max ||
		wmi_handle->wmi_events[event_id] == WMI_EVENT_ID_INVALID) {
		qdf_print("%s: Event id %d is unavailable\n",
				 __func__, event_id);
		return QDF_STATUS_E_FAILURE;
	}
	evt_id = wmi_handle->wmi_events[event_id];

	idx = wmi_unified_get_event_handler_ix(wmi_handle, evt_id);
	if (idx == -1) {
		qdf_print("%s : event handler is not registered: evt id 0x%x\n",
		       __func__, evt_id);
		return QDF_STATUS_E_FAILURE;
	}
	wmi_handle->event_handler[idx] = NULL;
	wmi_handle->event_id[idx] = 0;
	--soc->max_event_idx;
	wmi_handle->event_handler[idx] =
		wmi_handle->event_handler[soc->max_event_idx];
	wmi_handle->event_id[idx] =
		wmi_handle->event_id[soc->max_event_idx];

	return 0;
}

/**
 * wmi_unified_unregister_event_handler() - unregister wmi event handler
 * @wmi_handle: handle to wmi
 * @event_id: wmi event id
 *
 * Return: 0 on success
 */
int wmi_unified_unregister_event_handler(wmi_unified_t wmi_handle,
					 wmi_conv_event_id event_id)
{
	uint32_t idx = 0;
	uint32_t evt_id;
	struct wmi_soc *soc = wmi_handle->soc;

	if (event_id >= wmi_events_max ||
		wmi_handle->wmi_events[event_id] == WMI_EVENT_ID_INVALID) {
		qdf_print("%s: Event id %d is unavailable\n",
				 __func__, event_id);
		return QDF_STATUS_E_FAILURE;
	}
	evt_id = wmi_handle->wmi_events[event_id];

	idx = wmi_unified_get_event_handler_ix(wmi_handle, evt_id);
	if (idx == -1) {
		qdf_print("%s : event handler is not registered: evt id 0x%x\n",
		       __func__, evt_id);
		return QDF_STATUS_E_FAILURE;
	}
	wmi_handle->event_handler[idx] = NULL;
	wmi_handle->event_id[idx] = 0;
	--soc->max_event_idx;
	wmi_handle->event_handler[idx] =
		wmi_handle->event_handler[soc->max_event_idx];
	wmi_handle->event_id[idx] =
		wmi_handle->event_id[soc->max_event_idx];

	return 0;
}
qdf_export_symbol(wmi_unified_unregister_event_handler);

/**
 * wmi_process_fw_event_default_ctx() - process in default caller context
 * @wmi_handle: handle to wmi
 * @htc_packet: pointer to htc packet
 * @exec_ctx: execution context for wmi fw event
 *
 * Event process by below function will be in default caller context.
 * wmi internally provides rx work thread processing context.
 *
 * Return: none
 */
static void wmi_process_fw_event_default_ctx(struct wmi_unified *wmi_handle,
		       HTC_PACKET *htc_packet, uint8_t exec_ctx)
{
	wmi_buf_t evt_buf;
	evt_buf = (wmi_buf_t) htc_packet->pPktContext;

#ifndef CONFIG_MCL
	wmi_handle->rx_ops.wma_process_fw_event_handler_cbk
		(wmi_handle->scn_handle, evt_buf, exec_ctx);
#else
	wmi_handle->rx_ops.wma_process_fw_event_handler_cbk(wmi_handle,
					 evt_buf, exec_ctx);
#endif

	return;
}

/**
 * wmi_process_fw_event_worker_thread_ctx() - process in worker thread context
 * @wmi_handle: handle to wmi
 * @htc_packet: pointer to htc packet
 *
 * Event process by below function will be in worker thread context.
 * Use this method for events which are not critical and not
 * handled in protocol stack.
 *
 * Return: none
 */
static void wmi_process_fw_event_worker_thread_ctx
		(struct wmi_unified *wmi_handle, HTC_PACKET *htc_packet)
{
	wmi_buf_t evt_buf;

	evt_buf = (wmi_buf_t) htc_packet->pPktContext;

	qdf_spin_lock_bh(&wmi_handle->eventq_lock);
	qdf_nbuf_queue_add(&wmi_handle->event_queue, evt_buf);
	qdf_spin_unlock_bh(&wmi_handle->eventq_lock);
	qdf_queue_work(0, wmi_handle->wmi_rx_work_queue,
			&wmi_handle->rx_event_work);

	return;
}

/**
 * wmi_get_pdev_ep: Get wmi handle based on endpoint
 * @soc: handle to wmi soc
 * @ep: endpoint id
 *
 * Return: none
 */
static struct wmi_unified *wmi_get_pdev_ep(struct wmi_soc *soc,
						HTC_ENDPOINT_ID ep)
{
	uint32_t i;

	for (i = 0; i < WMI_MAX_RADIOS; i++)
		if (soc->wmi_endpoint_id[i] == ep)
			break;

	if (i == WMI_MAX_RADIOS)
		return NULL;

	return soc->wmi_pdev[i];
}

/**
 * wmi_control_rx() - process fw events callbacks
 * @ctx: handle to wmi
 * @htc_packet: pointer to htc packet
 *
 * Return: none
 */
static void wmi_control_rx(void *ctx, HTC_PACKET *htc_packet)
{
	struct wmi_soc *soc = (struct wmi_soc *) ctx;
	struct wmi_unified *wmi_handle;
	wmi_buf_t evt_buf;
	uint32_t id;
	uint32_t idx = 0;
	enum wmi_rx_exec_ctx exec_ctx;

	evt_buf = (wmi_buf_t) htc_packet->pPktContext;

	wmi_handle = wmi_get_pdev_ep(soc, htc_packet->Endpoint);
	if (wmi_handle == NULL) {
		qdf_print
		("%s :unable to get wmi_handle to Endpoint %d\n",
			__func__, htc_packet->Endpoint);
		qdf_nbuf_free(evt_buf);
		return;
	}

	id = WMI_GET_FIELD(qdf_nbuf_data(evt_buf), WMI_CMD_HDR, COMMANDID);
	idx = wmi_unified_get_event_handler_ix(wmi_handle, id);
	if (qdf_unlikely(idx == A_ERROR)) {
		WMI_LOGD("%s :event handler is not registered: event id 0x%x\n",
				 __func__, id);
		qdf_nbuf_free(evt_buf);
		return;
	}
	qdf_spin_lock_bh(&soc->ctx_lock);
	exec_ctx = wmi_handle->ctx[idx];
	qdf_spin_unlock_bh(&soc->ctx_lock);

#ifdef WMI_INTERFACE_EVENT_LOGGING
	if (wmi_handle->log_info.wmi_logging_enable) {
		uint8_t *data;
		data = qdf_nbuf_data(evt_buf);

		qdf_spin_lock_bh(&wmi_handle->log_info.wmi_record_lock);
		/* Exclude 4 bytes of TLV header */
		WMI_RX_EVENT_RECORD(wmi_handle, id, data +
				wmi_handle->log_info.buf_offset_event);
		qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);
	}
#endif

	if (exec_ctx == WMI_RX_WORK_CTX) {
		wmi_process_fw_event_worker_thread_ctx
					(wmi_handle, htc_packet);
	} else if (exec_ctx > WMI_RX_WORK_CTX) {
		wmi_process_fw_event_default_ctx
					(wmi_handle, htc_packet, exec_ctx);
	} else {
		qdf_print("%s :Invalid event context %d\n", __func__, exec_ctx);
		qdf_nbuf_free(evt_buf);
	}

}

/**
 * wmi_process_fw_event() - process any fw event
 * @wmi_handle: wmi handle
 * @evt_buf: fw event buffer
 *
 * This function process fw event in caller context
 *
 * Return: none
 */
void wmi_process_fw_event(struct wmi_unified *wmi_handle, wmi_buf_t evt_buf)
{
	__wmi_control_rx(wmi_handle, evt_buf);
}

/**
 * __wmi_control_rx() - process serialize wmi event callback
 * @wmi_handle: wmi handle
 * @evt_buf: fw event buffer
 *
 * Return: none
 */
void __wmi_control_rx(struct wmi_unified *wmi_handle, wmi_buf_t evt_buf)
{
	uint32_t id;
	uint8_t *data;
	uint32_t len;
	void *wmi_cmd_struct_ptr = NULL;
#ifndef WMI_NON_TLV_SUPPORT
	int tlv_ok_status = 0;
#endif
	uint32_t idx = 0;

	id = WMI_GET_FIELD(qdf_nbuf_data(evt_buf), WMI_CMD_HDR, COMMANDID);

	if (qdf_nbuf_pull_head(evt_buf, sizeof(WMI_CMD_HDR)) == NULL)
		goto end;

	data = qdf_nbuf_data(evt_buf);
	len = qdf_nbuf_len(evt_buf);

#ifndef WMI_NON_TLV_SUPPORT
	if (wmi_handle->target_type == WMI_TLV_TARGET) {
		/* Validate and pad(if necessary) the TLVs */
		tlv_ok_status =
			wmi_handle->ops->wmi_check_and_pad_event(wmi_handle->scn_handle,
							data, len, id,
							&wmi_cmd_struct_ptr);
		if (tlv_ok_status != 0) {
			QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_ERROR,
				"%s: Error: id=0x%d, wmitlv check status=%d\n",
				__func__, id, tlv_ok_status);
			goto end;
		}
	}
#endif

	idx = wmi_unified_get_event_handler_ix(wmi_handle, id);
	if (idx == A_ERROR) {
		QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_ERROR,
		   "%s : event handler is not registered: event id 0x%x\n",
			__func__, id);
		goto end;
	}
#ifdef WMI_INTERFACE_EVENT_LOGGING
	if (wmi_handle->log_info.wmi_logging_enable) {
		qdf_spin_lock_bh(&wmi_handle->log_info.wmi_record_lock);
		/* Exclude 4 bytes of TLV header */
		if (wmi_handle->ops->is_management_record(id)) {
			WMI_MGMT_EVENT_RECORD(wmi_handle, id, data
				+ wmi_handle->log_info.buf_offset_event);
		} else {
			WMI_EVENT_RECORD(wmi_handle, id, data +
					wmi_handle->log_info.buf_offset_event);
		}
		qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);
	}
#endif
	/* Call the WMI registered event handler */
	if (wmi_handle->target_type == WMI_TLV_TARGET)
		wmi_handle->event_handler[idx] (wmi_handle->scn_handle,
			wmi_cmd_struct_ptr, len);
	else
		wmi_handle->event_handler[idx] (wmi_handle->scn_handle,
			data, len);

end:
	/* Free event buffer and allocated event tlv */
#ifndef WMI_NON_TLV_SUPPORT
	if (wmi_handle->target_type == WMI_TLV_TARGET)
		wmi_handle->ops->wmi_free_allocated_event(id, &wmi_cmd_struct_ptr);
#endif

	qdf_nbuf_free(evt_buf);

}

#define WMI_WQ_WD_TIMEOUT (30 * 1000) /* 30s */

static inline void wmi_workqueue_watchdog_warn(uint32_t msg_type_id)
{
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
		  "%s: Message type %x has exceeded its alloted time of %ds",
		  __func__, msg_type_id, WMI_WQ_WD_TIMEOUT / 1000);
}

#ifdef CONFIG_SLUB_DEBUG_ON
static void wmi_workqueue_watchdog_bite(void *arg)
{
	struct wmi_wq_dbg_info *info = arg;

	wmi_workqueue_watchdog_warn(info->wd_msg_type_id);
	qdf_print_thread_trace(info->task);

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
		  "%s: Going down for WMI WQ Watchdog Bite!", __func__);
	QDF_BUG(0);
}
#else
static inline void wmi_workqueue_watchdog_bite(void *arg)
{
	struct wmi_wq_dbg_info *info = arg;

	wmi_workqueue_watchdog_warn(info->wd_msg_type_id);
}
#endif

/**
 * wmi_rx_event_work() - process rx event in rx work queue context
 * @arg: opaque pointer to wmi handle
 *
 * This function process any fw event to serialize it through rx worker thread.
 *
 * Return: none
 */
static void wmi_rx_event_work(void *arg)
{
	wmi_buf_t buf;
	struct wmi_unified *wmi = arg;
	qdf_timer_t wd_timer;
	struct wmi_wq_dbg_info info;

	/* initialize WMI workqueue watchdog timer */
	qdf_timer_init(NULL, &wd_timer, &wmi_workqueue_watchdog_bite,
			&info, QDF_TIMER_TYPE_SW);
	qdf_spin_lock_bh(&wmi->eventq_lock);
	buf = qdf_nbuf_queue_remove(&wmi->event_queue);
	qdf_spin_unlock_bh(&wmi->eventq_lock);
	while (buf) {
		qdf_timer_start(&wd_timer, WMI_WQ_WD_TIMEOUT);
		info.wd_msg_type_id =
		   WMI_GET_FIELD(qdf_nbuf_data(buf), WMI_CMD_HDR, COMMANDID);
		info.wmi_wq = wmi->wmi_rx_work_queue;
		info.task = qdf_get_current_task();
		__wmi_control_rx(wmi, buf);
		qdf_timer_stop(&wd_timer);
		qdf_spin_lock_bh(&wmi->eventq_lock);
		buf = qdf_nbuf_queue_remove(&wmi->event_queue);
		qdf_spin_unlock_bh(&wmi->eventq_lock);
	}
	qdf_timer_free(&wd_timer);
}

#ifdef FEATURE_RUNTIME_PM
/**
 * wmi_runtime_pm_init() - initialize runtime pm wmi variables
 * @wmi_handle: wmi context
 */
static void wmi_runtime_pm_init(struct wmi_unified *wmi_handle)
{
	qdf_atomic_init(&wmi_handle->runtime_pm_inprogress);
}

/**
 * wmi_set_runtime_pm_inprogress() - set runtime pm progress flag
 * @wmi_handle: wmi context
 * @val: runtime pm progress flag
 */
void wmi_set_runtime_pm_inprogress(wmi_unified_t wmi_handle, A_BOOL val)
{
	qdf_atomic_set(&wmi_handle->runtime_pm_inprogress, val);
}

/**
 * wmi_get_runtime_pm_inprogress() - get runtime pm progress flag
 * @wmi_handle: wmi context
 */
inline bool wmi_get_runtime_pm_inprogress(wmi_unified_t wmi_handle)
{
	return qdf_atomic_read(&wmi_handle->runtime_pm_inprogress);
}
#else
static void wmi_runtime_pm_init(struct wmi_unified *wmi_handle)
{
}
#endif

/**
 * wmi_unified_get_soc_handle: Get WMI SoC handle
 * @param wmi_handle: WMI context got from wmi_attach
 *
 * return: Pointer to Soc handle
 */
void *wmi_unified_get_soc_handle(struct wmi_unified *wmi_handle)
{
	return wmi_handle->soc;
}

/**
 * wmi_interface_logging_init: Interface looging init
 * @param wmi_handle: Pointer to wmi handle object
 *
 * return: None
 */
#ifdef WMI_INTERFACE_EVENT_LOGGING
static inline void wmi_interface_logging_init(struct wmi_unified *wmi_handle)
{
	if (QDF_STATUS_SUCCESS == wmi_log_init(wmi_handle)) {
		qdf_spinlock_create(&wmi_handle->log_info.wmi_record_lock);
		wmi_debugfs_init(wmi_handle);
	}
}
#else
void wmi_interface_logging_init(struct wmi_unified *wmi_handle)
{
}
#endif

/**
 * wmi_target_params_init: Target specific params init
 * @param wmi_soc: Pointer to wmi soc object
 * @param wmi_handle: Pointer to wmi handle object
 *
 * return: None
 */
#ifndef CONFIG_MCL
static inline void wmi_target_params_init(struct wmi_soc *soc,
				struct wmi_unified *wmi_handle)
{
	wmi_handle->pdev_param = soc->pdev_param;
	wmi_handle->vdev_param = soc->vdev_param;
	wmi_handle->services = soc->services;
}
#else
static inline void wmi_target_params_init(struct wmi_soc *soc,
				struct wmi_unified *wmi_handle)
{
	wmi_handle->services = soc->services;
}
#endif

/**
 * wmi_unified_get_pdev_handle: Get WMI SoC handle
 * @param wmi_soc: Pointer to wmi soc object
 * @param pdev_idx: pdev index
 *
 * return: Pointer to wmi handle or NULL on failure
 */
void *wmi_unified_get_pdev_handle(struct wmi_soc *soc, uint32_t pdev_idx)
{
	struct wmi_unified *wmi_handle;

	if (pdev_idx >= WMI_MAX_RADIOS)
		return NULL;

	if (soc->wmi_pdev[pdev_idx] == NULL) {
		wmi_handle =
			(struct wmi_unified *) qdf_mem_malloc(
					sizeof(struct wmi_unified));
		if (wmi_handle == NULL) {
			qdf_print("allocation of wmi handle failed %zu\n",
					sizeof(struct wmi_unified));
			return NULL;
		}
		wmi_handle->scn_handle = soc->scn_handle;
		wmi_handle->event_id = soc->event_id;
		wmi_handle->event_handler = soc->event_handler;
		wmi_handle->ctx = soc->ctx;
		wmi_handle->ops = soc->ops;
		qdf_spinlock_create(&wmi_handle->eventq_lock);
		qdf_nbuf_queue_init(&wmi_handle->event_queue);

		qdf_create_work(0, &wmi_handle->rx_event_work,
				wmi_rx_event_work, wmi_handle);
		wmi_handle->wmi_rx_work_queue =
			qdf_create_workqueue("wmi_rx_event_work_queue");
		if (NULL == wmi_handle->wmi_rx_work_queue) {
			WMI_LOGE("failed to create wmi_rx_event_work_queue");
			goto error;
		}
		wmi_handle->wmi_events = soc->wmi_events;
		wmi_target_params_init(soc, wmi_handle);
		wmi_interface_logging_init(wmi_handle);
		qdf_atomic_init(&wmi_handle->pending_cmds);
		qdf_atomic_init(&wmi_handle->is_target_suspended);
		wmi_handle->target_type = soc->target_type;
		wmi_handle->soc = soc;

		soc->wmi_pdev[pdev_idx] = wmi_handle;
	} else
		wmi_handle = soc->wmi_pdev[pdev_idx];

	wmi_handle->wmi_stopinprogress = 0;
	wmi_handle->wmi_endpoint_id = soc->wmi_endpoint_id[pdev_idx];
	wmi_handle->htc_handle = soc->htc_handle;
	wmi_handle->max_msg_len = soc->max_msg_len[pdev_idx];

	return wmi_handle;

error:
	qdf_mem_free(wmi_handle);

	return NULL;
}
qdf_export_symbol(wmi_unified_get_pdev_handle);

static void (*wmi_attach_register[WMI_MAX_TARGET_TYPE])(wmi_unified_t);

void wmi_unified_register_module(enum wmi_target_type target_type,
			void (*wmi_attach)(wmi_unified_t wmi_handle))
{
	if (target_type < WMI_MAX_TARGET_TYPE)
		wmi_attach_register[target_type] = wmi_attach;

	return;
}
qdf_export_symbol(wmi_unified_register_module);

/**
 * wmi_unified_attach() -  attach for unified WMI
 * @scn_handle: handle to SCN
 * @osdev: OS device context
 * @target_type: TLV or not-TLV based target
 * @use_cookie: cookie based allocation enabled/disabled
 * @ops: umac rx callbacks
 * @psoc: objmgr psoc
 *
 * @Return: wmi handle.
 */
void *wmi_unified_attach(void *scn_handle,
			 osdev_t osdev, enum wmi_target_type target_type,
			 bool use_cookie, struct wmi_rx_ops *rx_ops,
			 struct wlan_objmgr_psoc *psoc)
{
	struct wmi_unified *wmi_handle;
	struct wmi_soc *soc;

	soc = (struct wmi_soc *) qdf_mem_malloc(sizeof(struct wmi_soc));
	if (soc == NULL) {
		qdf_print("Allocation of wmi_soc failed %zu\n",
				sizeof(struct wmi_soc));
		return NULL;
	}

	wmi_handle =
		(struct wmi_unified *) qdf_mem_malloc(
			sizeof(struct wmi_unified));
	if (wmi_handle == NULL) {
		qdf_mem_free(soc);
		qdf_print("allocation of wmi handle failed %zu\n",
			sizeof(struct wmi_unified));
		return NULL;
	}
	wmi_handle->soc = soc;
	wmi_handle->event_id = soc->event_id;
	wmi_handle->event_handler = soc->event_handler;
	wmi_handle->ctx = soc->ctx;
	wmi_handle->wmi_events = soc->wmi_events;
	wmi_target_params_init(soc, wmi_handle);
	wmi_handle->scn_handle = scn_handle;
	soc->scn_handle = scn_handle;
	qdf_atomic_init(&wmi_handle->pending_cmds);
	qdf_atomic_init(&wmi_handle->is_target_suspended);
	wmi_runtime_pm_init(wmi_handle);
	qdf_spinlock_create(&wmi_handle->eventq_lock);
	qdf_nbuf_queue_init(&wmi_handle->event_queue);
	qdf_create_work(0, &wmi_handle->rx_event_work,
			wmi_rx_event_work, wmi_handle);
	wmi_handle->wmi_rx_work_queue =
		qdf_create_workqueue("wmi_rx_event_work_queue");
	if (NULL == wmi_handle->wmi_rx_work_queue) {
		WMI_LOGE("failed to create wmi_rx_event_work_queue");
		goto error;
	}
	wmi_interface_logging_init(wmi_handle);
	/* Attach mc_thread context processing function */
	wmi_handle->rx_ops.wma_process_fw_event_handler_cbk =
				rx_ops->wma_process_fw_event_handler_cbk;
	wmi_handle->target_type = target_type;
	soc->target_type = target_type;
	if (wmi_attach_register[target_type]) {
		wmi_attach_register[target_type](wmi_handle);
	} else {
		WMI_LOGE("wmi attach is not registered");
		goto error;
	}
	/* Assign target cookie capablity */
	wmi_handle->use_cookie = use_cookie;
	wmi_handle->osdev = osdev;
	wmi_handle->wmi_stopinprogress = 0;
	/* Increase the ref count once refcount infra is present */
	soc->wmi_psoc = psoc;
	qdf_spinlock_create(&soc->ctx_lock);

	soc->ops = wmi_handle->ops;
	soc->wmi_pdev[0] = wmi_handle;

	return wmi_handle;

error:
	qdf_mem_free(soc);
	qdf_mem_free(wmi_handle);

	return NULL;
}

/**
 * wmi_unified_detach() -  detach for unified WMI
 *
 * @wmi_handle  : handle to wmi.
 *
 * @Return: none.
 */
void wmi_unified_detach(struct wmi_unified *wmi_handle)
{
	wmi_buf_t buf;
	struct wmi_soc *soc;
	uint8_t i;

	soc = wmi_handle->soc;
	for (i = 0; i < WMI_MAX_RADIOS; i++) {
		if (soc->wmi_pdev[i]) {
			qdf_flush_workqueue(0,
				soc->wmi_pdev[i]->wmi_rx_work_queue);
			qdf_destroy_workqueue(0,
				soc->wmi_pdev[i]->wmi_rx_work_queue);
			wmi_debugfs_remove(soc->wmi_pdev[i]);
			buf = qdf_nbuf_queue_remove(
					&soc->wmi_pdev[i]->event_queue);
			while (buf) {
				qdf_nbuf_free(buf);
				buf = qdf_nbuf_queue_remove(
						&soc->wmi_pdev[i]->event_queue);
			}

			wmi_log_buffer_free(soc->wmi_pdev[i]);

			/* Free events logs list */
			if (soc->wmi_pdev[i]->events_logs_list)
				qdf_mem_free(
					soc->wmi_pdev[i]->events_logs_list);

			qdf_spinlock_destroy(&soc->wmi_pdev[i]->eventq_lock);
			qdf_mem_free(soc->wmi_pdev[i]);
		}
	}
	qdf_spinlock_destroy(&soc->ctx_lock);

	if (soc->wmi_service_bitmap) {
		qdf_mem_free(soc->wmi_service_bitmap);
		soc->wmi_service_bitmap = NULL;
	}

	if (soc->wmi_ext_service_bitmap) {
		qdf_mem_free(soc->wmi_ext_service_bitmap);
		soc->wmi_ext_service_bitmap = NULL;
	}

	/* Decrease the ref count once refcount infra is present */
	soc->wmi_psoc = NULL;
	qdf_mem_free(soc);
}

/**
 * wmi_unified_remove_work() - detach for WMI work
 * @wmi_handle: handle to WMI
 *
 * A function that does not fully detach WMI, but just remove work
 * queue items associated with it. This is used to make sure that
 * before any other processing code that may destroy related contexts
 * (HTC, etc), work queue processing on WMI has already been stopped.
 *
 * Return: None
 */
void
wmi_unified_remove_work(struct wmi_unified *wmi_handle)
{
	wmi_buf_t buf;

	qdf_flush_workqueue(0, wmi_handle->wmi_rx_work_queue);
	qdf_spin_lock_bh(&wmi_handle->eventq_lock);
	buf = qdf_nbuf_queue_remove(&wmi_handle->event_queue);
	while (buf) {
		qdf_nbuf_free(buf);
		buf = qdf_nbuf_queue_remove(&wmi_handle->event_queue);
	}
	qdf_spin_unlock_bh(&wmi_handle->eventq_lock);
}

/**
 * wmi_htc_tx_complete() - Process htc tx completion
 *
 * @ctx: handle to wmi
 * @htc_packet: pointer to htc packet
 *
 * @Return: none.
 */
static void wmi_htc_tx_complete(void *ctx, HTC_PACKET *htc_pkt)
{
	struct wmi_soc *soc = (struct wmi_soc *) ctx;
	wmi_buf_t wmi_cmd_buf = GET_HTC_PACKET_NET_BUF_CONTEXT(htc_pkt);
	u_int8_t *buf_ptr;
	u_int32_t len;
	struct wmi_unified *wmi_handle;
#ifdef WMI_INTERFACE_EVENT_LOGGING
	uint32_t cmd_id;
#endif

	ASSERT(wmi_cmd_buf);
	wmi_handle = wmi_get_pdev_ep(soc, htc_pkt->Endpoint);
	if (wmi_handle == NULL) {
		WMI_LOGE("%s: Unable to get wmi handle\n", __func__);
		QDF_ASSERT(0);
		return;
	}
#ifdef WMI_INTERFACE_EVENT_LOGGING
	if (wmi_handle && wmi_handle->log_info.wmi_logging_enable) {
		cmd_id = WMI_GET_FIELD(qdf_nbuf_data(wmi_cmd_buf),
				WMI_CMD_HDR, COMMANDID);

	qdf_spin_lock_bh(&wmi_handle->log_info.wmi_record_lock);
	/* Record 16 bytes of WMI cmd tx complete data
	- exclude TLV and WMI headers */
	if (wmi_handle->ops->is_management_record(cmd_id)) {
		WMI_MGMT_COMMAND_TX_CMP_RECORD(wmi_handle, cmd_id,
			qdf_nbuf_data(wmi_cmd_buf) +
			wmi_handle->log_info.buf_offset_command);
	} else {
		WMI_COMMAND_TX_CMP_RECORD(wmi_handle, cmd_id,
			qdf_nbuf_data(wmi_cmd_buf) +
			wmi_handle->log_info.buf_offset_command);
	}

	qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);
	}
#endif
	buf_ptr = (u_int8_t *) wmi_buf_data(wmi_cmd_buf);
	len = qdf_nbuf_len(wmi_cmd_buf);
	qdf_mem_zero(buf_ptr, len);
	qdf_nbuf_free(wmi_cmd_buf);
	qdf_mem_free(htc_pkt);
	qdf_atomic_dec(&wmi_handle->pending_cmds);
}

/**
 * wmi_connect_pdev_htc_service() -  WMI API to get connect to HTC service
 *
 * @wmi_handle: handle to WMI.
 * @pdev_idx: Pdev index
 *
 * @Return: status.
 */
static int wmi_connect_pdev_htc_service(struct wmi_soc *soc,
						uint32_t pdev_idx)
{
	int status;
	uint32_t svc_id[] = {WMI_CONTROL_SVC, WMI_CONTROL_SVC_WMAC1,
						WMI_CONTROL_SVC_WMAC2};
	struct htc_service_connect_resp response;
	struct htc_service_connect_req connect;

	OS_MEMZERO(&connect, sizeof(connect));
	OS_MEMZERO(&response, sizeof(response));

	/* meta data is unused for now */
	connect.pMetaData = NULL;
	connect.MetaDataLength = 0;
	/* these fields are the same for all service endpoints */
	connect.EpCallbacks.pContext = soc;
	connect.EpCallbacks.EpTxCompleteMultiple =
		NULL /* Control path completion ar6000_tx_complete */;
	connect.EpCallbacks.EpRecv = wmi_control_rx /* Control path rx */;
	connect.EpCallbacks.EpRecvRefill = NULL /* ar6000_rx_refill */;
	connect.EpCallbacks.EpSendFull = NULL /* ar6000_tx_queue_full */;
	connect.EpCallbacks.EpTxComplete =
		wmi_htc_tx_complete /* ar6000_tx_queue_full */;

	/* connect to control service */
	connect.service_id = svc_id[pdev_idx];
	status = htc_connect_service(soc->htc_handle, &connect,
				&response);


	if (status != EOK) {
		qdf_print
			("Failed to connect to WMI CONTROL service status:%d\n",
			status);
		return status;
	}

	soc->wmi_endpoint_id[pdev_idx] = response.Endpoint;
	soc->max_msg_len[pdev_idx] = response.MaxMsgLength;

	return 0;
}

/**
 * wmi_unified_connect_htc_service() -  WMI API to get connect to HTC service
 *
 * @wmi_handle: handle to WMI.
 *
 * @Return: status.
 */
QDF_STATUS
wmi_unified_connect_htc_service(struct wmi_unified *wmi_handle,
				void *htc_handle)
{
	uint32_t i;
	uint8_t wmi_ep_count;

	wmi_handle->soc->htc_handle = htc_handle;

	wmi_ep_count = htc_get_wmi_endpoint_count(htc_handle);
	if (wmi_ep_count > WMI_MAX_RADIOS)
		return QDF_STATUS_E_FAULT;

	for (i = 0; i < wmi_ep_count; i++)
		wmi_connect_pdev_htc_service(wmi_handle->soc, i);

	wmi_handle->htc_handle = htc_handle;
	wmi_handle->wmi_endpoint_id = wmi_handle->soc->wmi_endpoint_id[0];
	wmi_handle->max_msg_len = wmi_handle->soc->max_msg_len[0];

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_get_host_credits() -  WMI API to get updated host_credits
 *
 * @wmi_handle: handle to WMI.
 *
 * @Return: updated host_credits.
 */
int wmi_get_host_credits(wmi_unified_t wmi_handle)
{
	int host_credits = 0;

	htc_get_control_endpoint_tx_host_credits(wmi_handle->htc_handle,
						 &host_credits);
	return host_credits;
}

/**
 * wmi_get_pending_cmds() - WMI API to get WMI Pending Commands in the HTC
 *                          queue
 *
 * @wmi_handle: handle to WMI.
 *
 * @Return: Pending Commands in the HTC queue.
 */
int wmi_get_pending_cmds(wmi_unified_t wmi_handle)
{
	return qdf_atomic_read(&wmi_handle->pending_cmds);
}

/**
 * wmi_set_target_suspend() -  WMI API to set target suspend state
 *
 * @wmi_handle: handle to WMI.
 * @val: suspend state boolean.
 *
 * @Return: none.
 */
void wmi_set_target_suspend(wmi_unified_t wmi_handle, A_BOOL val)
{
	qdf_atomic_set(&wmi_handle->is_target_suspended, val);
}

/**
 * WMI API to set crash injection state
 * @param wmi_handle:	handle to WMI.
 * @param val:		crash injection state boolean.
 */
void wmi_tag_crash_inject(wmi_unified_t wmi_handle, A_BOOL flag)
{
	wmi_handle->tag_crash_inject = flag;
}

/**
 * WMI API to set bus suspend state
 * @param wmi_handle:	handle to WMI.
 * @param val:		suspend state boolean.
 */
void wmi_set_is_wow_bus_suspended(wmi_unified_t wmi_handle, A_BOOL val)
{
	qdf_atomic_set(&wmi_handle->is_wow_bus_suspended, val);
}

void wmi_set_tgt_assert(wmi_unified_t wmi_handle, bool val)
{
	wmi_handle->tgt_force_assert_enable = val;
}

#ifndef CONFIG_MCL
/**
 * API to flush all the previous packets  associated with the wmi endpoint
 *
 * @param wmi_handle      : handle to WMI.
 */
void
wmi_flush_endpoint(wmi_unified_t wmi_handle)
{
	htc_flush_endpoint(wmi_handle->htc_handle,
		wmi_handle->wmi_endpoint_id, 0);
}
qdf_export_symbol(wmi_flush_endpoint);

/**
 * generic function to block unified WMI command
 * @param wmi_handle      : handle to WMI.
 * @return 0  on success and -ve on failure.
 */
int
wmi_stop(wmi_unified_t wmi_handle)
{
	QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_INFO,
			"WMI Stop\n");
	wmi_handle->wmi_stopinprogress = 1;
	return 0;
}

/**
 * wmi_pdev_id_conversion_enable() - API to enable pdev_id conversion in WMI
 *                     By default pdev_id conversion is not done in WMI.
 *                     This API can be used enable conversion in WMI.
 * @param wmi_handle   : handle to WMI
 * Return none
 */
void wmi_pdev_id_conversion_enable(wmi_unified_t wmi_handle)
{
	if (wmi_handle->target_type == WMI_TLV_TARGET)
		wmi_handle->ops->wmi_pdev_id_conversion_enable(wmi_handle);
}

#endif
