/*
 * Copyright (c) 2015-2020 The Linux Foundation. All rights reserved.
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
 * Host WMI unified implementation
 */
#include "htc_api.h"
#include "htc_api.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_api.h"
#include "qdf_module.h"
#include "qdf_platform.h"
#ifdef WMI_EXT_DBG
#include "qdf_list.h"
#include "qdf_atomic.h"
#endif

#ifndef WMI_NON_TLV_SUPPORT
#include "wmi_tlv_helper.h"
#endif

#include <linux/debugfs.h>
#include <target_if.h>
#include <qdf_debugfs.h>
#include "wmi_filtered_logging.h"
#include <wmi_hang_event.h>

/* This check for CONFIG_WIN temporary added due to redeclaration compilation
error in MCL. Error is caused due to inclusion of wmi.h in wmi_unified_api.h
which gets included here through ol_if_athvar.h. Eventually it is expected that
wmi.h will be removed from wmi_unified_api.h after cleanup, which will need
WMI_CMD_HDR to be defined here. */
/* Copied from wmi.h */
#undef MS
#define MS(_v, _f) (((_v) & _f##_MASK) >> _f##_LSB)
#undef SM
#define SM(_v, _f) (((_v) << _f##_LSB) & _f##_MASK)
#undef WO
#define WO(_f)      ((_f##_OFFSET) >> 2)

#undef GET_FIELD
#define GET_FIELD(_addr, _f) MS(*((uint32_t *)(_addr) + WO(_f)), _f)
#undef SET_FIELD
#define SET_FIELD(_addr, _f, _val)  \
	    (*((uint32_t *)(_addr) + WO(_f)) = \
		(*((uint32_t *)(_addr) + WO(_f)) & ~_f##_MASK) | SM(_val, _f))

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
	uint32_t	commandId:24,
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

#define WMI_MIN_HEAD_ROOM 64

/* WBUFF pool sizes for WMI */
/* Allocation of size 256 bytes */
#define WMI_WBUFF_POOL_0_SIZE 128
/* Allocation of size 512 bytes */
#define WMI_WBUFF_POOL_1_SIZE 16
/* Allocation of size 1024 bytes */
#define WMI_WBUFF_POOL_2_SIZE 8
/* Allocation of size 2048 bytes */
#define WMI_WBUFF_POOL_3_SIZE 8

#ifdef WMI_INTERFACE_EVENT_LOGGING
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))
/* TODO Cleanup this backported function */
static int wmi_bp_seq_printf(qdf_debugfs_file_t m, const char *f, ...)
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

#ifndef MAX_WMI_INSTANCES
#define CUSTOM_MGMT_CMD_DATA_SIZE 4
#endif

#ifndef WMI_INTERFACE_EVENT_LOGGING_DYNAMIC_ALLOC
/* WMI commands */
uint32_t g_wmi_command_buf_idx = 0;
struct wmi_command_debug wmi_command_log_buffer[WMI_CMD_DEBUG_MAX_ENTRY];

/* WMI commands TX completed */
uint32_t g_wmi_command_tx_cmp_buf_idx = 0;
struct wmi_command_cmp_debug
	wmi_command_tx_cmp_log_buffer[WMI_CMD_CMPL_DEBUG_MAX_ENTRY];

/* WMI events when processed */
uint32_t g_wmi_event_buf_idx = 0;
struct wmi_event_debug wmi_event_log_buffer[WMI_EVENT_DEBUG_MAX_ENTRY];

/* WMI events when queued */
uint32_t g_wmi_rx_event_buf_idx = 0;
struct wmi_event_debug wmi_rx_event_log_buffer[WMI_EVENT_DEBUG_MAX_ENTRY];
#endif

#define WMI_COMMAND_RECORD(h, a, b) {					\
	if (wmi_cmd_log_max_entry <=					\
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

#define WMI_COMMAND_TX_CMP_RECORD(h, a, b, da, pa) {			\
	if (wmi_cmd_cmpl_log_max_entry <=				\
		*(h->log_info.wmi_command_tx_cmp_log_buf_info.p_buf_tail_idx))\
		*(h->log_info.wmi_command_tx_cmp_log_buf_info.		\
				p_buf_tail_idx) = 0;			\
	((struct wmi_command_cmp_debug *)h->log_info.			\
		wmi_command_tx_cmp_log_buf_info.buf)			\
		[*(h->log_info.wmi_command_tx_cmp_log_buf_info.		\
				p_buf_tail_idx)].			\
							command	= a;	\
	qdf_mem_copy(((struct wmi_command_cmp_debug *)h->log_info.	\
				wmi_command_tx_cmp_log_buf_info.buf)	\
		[*(h->log_info.wmi_command_tx_cmp_log_buf_info.		\
			p_buf_tail_idx)].				\
		data, b, wmi_record_max_length);			\
	((struct wmi_command_cmp_debug *)h->log_info.			\
		wmi_command_tx_cmp_log_buf_info.buf)			\
		[*(h->log_info.wmi_command_tx_cmp_log_buf_info.		\
				p_buf_tail_idx)].			\
		time = qdf_get_log_timestamp();				\
	((struct wmi_command_cmp_debug *)h->log_info.			\
		wmi_command_tx_cmp_log_buf_info.buf)			\
		[*(h->log_info.wmi_command_tx_cmp_log_buf_info.		\
				p_buf_tail_idx)].			\
		dma_addr = da;						\
	((struct wmi_command_cmp_debug *)h->log_info.			\
		wmi_command_tx_cmp_log_buf_info.buf)			\
		[*(h->log_info.wmi_command_tx_cmp_log_buf_info.		\
				p_buf_tail_idx)].			\
		phy_addr = pa;						\
	(*(h->log_info.wmi_command_tx_cmp_log_buf_info.p_buf_tail_idx))++;\
	h->log_info.wmi_command_tx_cmp_log_buf_info.length++;		\
}

#define WMI_EVENT_RECORD(h, a, b) {					\
	if (wmi_event_log_max_entry <=					\
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
	if (wmi_event_log_max_entry <=					\
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

#ifndef WMI_INTERFACE_EVENT_LOGGING_DYNAMIC_ALLOC
uint32_t g_wmi_mgmt_command_buf_idx = 0;
struct
wmi_command_debug wmi_mgmt_command_log_buffer[WMI_MGMT_TX_DEBUG_MAX_ENTRY];

/* wmi_mgmt commands TX completed */
uint32_t g_wmi_mgmt_command_tx_cmp_buf_idx = 0;
struct wmi_command_debug
wmi_mgmt_command_tx_cmp_log_buffer[WMI_MGMT_TX_CMPL_DEBUG_MAX_ENTRY];

/* wmi_mgmt events when received */
uint32_t g_wmi_mgmt_rx_event_buf_idx = 0;
struct wmi_event_debug
wmi_mgmt_rx_event_log_buffer[WMI_MGMT_RX_DEBUG_MAX_ENTRY];

/* wmi_diag events when received */
uint32_t g_wmi_diag_rx_event_buf_idx = 0;
struct wmi_event_debug
wmi_diag_rx_event_log_buffer[WMI_DIAG_RX_EVENT_DEBUG_MAX_ENTRY];
#endif

#define WMI_MGMT_COMMAND_RECORD(h, a, b) {                              \
	if (wmi_mgmt_tx_log_max_entry <=                                   \
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
	if (wmi_mgmt_tx_cmpl_log_max_entry <=				\
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

#define WMI_MGMT_RX_EVENT_RECORD(h, a, b) do {				\
	if (wmi_mgmt_rx_log_max_entry <=				\
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
} while (0);

#define WMI_DIAG_RX_EVENT_RECORD(h, a, b) do {                             \
	if (wmi_diag_log_max_entry <=                                   \
		*(h->log_info.wmi_diag_event_log_buf_info.p_buf_tail_idx))\
		*(h->log_info.wmi_diag_event_log_buf_info.p_buf_tail_idx) = 0;\
	((struct wmi_event_debug *)h->log_info.wmi_diag_event_log_buf_info.buf)\
		[*(h->log_info.wmi_diag_event_log_buf_info.p_buf_tail_idx)]\
					.event = a;                     \
	qdf_mem_copy(((struct wmi_event_debug *)h->log_info.            \
				wmi_diag_event_log_buf_info.buf)        \
		[*(h->log_info.wmi_diag_event_log_buf_info.p_buf_tail_idx)].\
			data, b, wmi_record_max_length);                \
	((struct wmi_event_debug *)h->log_info.wmi_diag_event_log_buf_info.buf)\
		[*(h->log_info.wmi_diag_event_log_buf_info.p_buf_tail_idx)].\
			time = qdf_get_log_timestamp();                 \
	(*(h->log_info.wmi_diag_event_log_buf_info.p_buf_tail_idx))++;  \
	h->log_info.wmi_diag_event_log_buf_info.length++;               \
} while (0);

/* These are defined to made it as module param, which can be configured */
/* WMI Commands */
uint32_t wmi_cmd_log_max_entry = WMI_CMD_DEBUG_MAX_ENTRY;
uint32_t wmi_cmd_cmpl_log_max_entry = WMI_CMD_CMPL_DEBUG_MAX_ENTRY;
/* WMI Events */
uint32_t wmi_event_log_max_entry = WMI_EVENT_DEBUG_MAX_ENTRY;
/* WMI MGMT Tx */
uint32_t wmi_mgmt_tx_log_max_entry = WMI_MGMT_TX_DEBUG_MAX_ENTRY;
uint32_t wmi_mgmt_tx_cmpl_log_max_entry = WMI_MGMT_TX_CMPL_DEBUG_MAX_ENTRY;
/* WMI MGMT Rx */
uint32_t wmi_mgmt_rx_log_max_entry = WMI_MGMT_RX_DEBUG_MAX_ENTRY;
/* WMI Diag Event */
uint32_t wmi_diag_log_max_entry = WMI_DIAG_RX_EVENT_DEBUG_MAX_ENTRY;
/* WMI capture size */
uint32_t wmi_record_max_length = WMI_DEBUG_ENTRY_MAX_LENGTH;
uint32_t wmi_display_size = 100;

/**
 * wmi_log_init() - Initialize WMI event logging
 * @wmi_handle: WMI handle.
 *
 * Return: Initialization status
 */
#ifndef WMI_INTERFACE_EVENT_LOGGING_DYNAMIC_ALLOC
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
	struct wmi_log_buf_t *diag_event_log_buf =
			&wmi_handle->log_info.wmi_diag_event_log_buf_info;

	/* WMI commands */
	cmd_log_buf->length = 0;
	cmd_log_buf->buf_tail_idx = 0;
	cmd_log_buf->buf = wmi_command_log_buffer;
	cmd_log_buf->p_buf_tail_idx = &g_wmi_command_buf_idx;
	cmd_log_buf->size = WMI_CMD_DEBUG_MAX_ENTRY;

	/* WMI commands TX completed */
	cmd_tx_cmpl_log_buf->length = 0;
	cmd_tx_cmpl_log_buf->buf_tail_idx = 0;
	cmd_tx_cmpl_log_buf->buf = wmi_command_tx_cmp_log_buffer;
	cmd_tx_cmpl_log_buf->p_buf_tail_idx = &g_wmi_command_tx_cmp_buf_idx;
	cmd_tx_cmpl_log_buf->size = WMI_CMD_CMPL_DEBUG_MAX_ENTRY;

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
	mgmt_cmd_log_buf->size = WMI_MGMT_TX_DEBUG_MAX_ENTRY;

	/* WMI Management commands Tx completed*/
	mgmt_cmd_tx_cmp_log_buf->length = 0;
	mgmt_cmd_tx_cmp_log_buf->buf_tail_idx = 0;
	mgmt_cmd_tx_cmp_log_buf->buf = wmi_mgmt_command_tx_cmp_log_buffer;
	mgmt_cmd_tx_cmp_log_buf->p_buf_tail_idx =
		&g_wmi_mgmt_command_tx_cmp_buf_idx;
	mgmt_cmd_tx_cmp_log_buf->size = WMI_MGMT_TX_CMPL_DEBUG_MAX_ENTRY;

	/* WMI Management events when received */
	mgmt_event_log_buf->length = 0;
	mgmt_event_log_buf->buf_tail_idx = 0;
	mgmt_event_log_buf->buf = wmi_mgmt_rx_event_log_buffer;
	mgmt_event_log_buf->p_buf_tail_idx = &g_wmi_mgmt_rx_event_buf_idx;
	mgmt_event_log_buf->size = WMI_MGMT_RX_DEBUG_MAX_ENTRY;

	/* WMI diag events when received */
	diag_event_log_buf->length = 0;
	diag_event_log_buf->buf_tail_idx = 0;
	diag_event_log_buf->buf = wmi_diag_rx_event_log_buffer;
	diag_event_log_buf->p_buf_tail_idx = &g_wmi_diag_rx_event_buf_idx;
	diag_event_log_buf->size = WMI_DIAG_RX_EVENT_DEBUG_MAX_ENTRY;

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
	struct wmi_log_buf_t *diag_event_log_buf =
			&wmi_handle->log_info.wmi_diag_event_log_buf_info;

	wmi_handle->log_info.wmi_logging_enable = 0;

	/* WMI commands */
	cmd_log_buf->length = 0;
	cmd_log_buf->buf_tail_idx = 0;
	cmd_log_buf->buf = (struct wmi_command_debug *) qdf_mem_malloc(
		wmi_cmd_log_max_entry * sizeof(struct wmi_command_debug));
	cmd_log_buf->size = wmi_cmd_log_max_entry;

	if (!cmd_log_buf->buf)
		return QDF_STATUS_E_NOMEM;

	cmd_log_buf->p_buf_tail_idx = &cmd_log_buf->buf_tail_idx;

	/* WMI commands TX completed */
	cmd_tx_cmpl_log_buf->length = 0;
	cmd_tx_cmpl_log_buf->buf_tail_idx = 0;
	cmd_tx_cmpl_log_buf->buf = (struct wmi_command_cmp_debug *) qdf_mem_malloc(
		wmi_cmd_cmpl_log_max_entry * sizeof(struct wmi_command_cmp_debug));
	cmd_tx_cmpl_log_buf->size = wmi_cmd_cmpl_log_max_entry;

	if (!cmd_tx_cmpl_log_buf->buf)
		return QDF_STATUS_E_NOMEM;

	cmd_tx_cmpl_log_buf->p_buf_tail_idx =
		&cmd_tx_cmpl_log_buf->buf_tail_idx;

	/* WMI events when processed */
	event_log_buf->length = 0;
	event_log_buf->buf_tail_idx = 0;
	event_log_buf->buf = (struct wmi_event_debug *) qdf_mem_malloc(
		wmi_event_log_max_entry * sizeof(struct wmi_event_debug));
	event_log_buf->size = wmi_event_log_max_entry;

	if (!event_log_buf->buf)
		return QDF_STATUS_E_NOMEM;

	event_log_buf->p_buf_tail_idx = &event_log_buf->buf_tail_idx;

	/* WMI events when queued */
	rx_event_log_buf->length = 0;
	rx_event_log_buf->buf_tail_idx = 0;
	rx_event_log_buf->buf = (struct wmi_event_debug *) qdf_mem_malloc(
		wmi_event_log_max_entry * sizeof(struct wmi_event_debug));
	rx_event_log_buf->size = wmi_event_log_max_entry;

	if (!rx_event_log_buf->buf)
		return QDF_STATUS_E_NOMEM;

	rx_event_log_buf->p_buf_tail_idx = &rx_event_log_buf->buf_tail_idx;

	/* WMI Management commands */
	mgmt_cmd_log_buf->length = 0;
	mgmt_cmd_log_buf->buf_tail_idx = 0;
	mgmt_cmd_log_buf->buf = (struct wmi_command_debug *) qdf_mem_malloc(
		wmi_mgmt_tx_log_max_entry * sizeof(struct wmi_command_debug));
	mgmt_cmd_log_buf->size = wmi_mgmt_tx_log_max_entry;

	if (!mgmt_cmd_log_buf->buf)
		return QDF_STATUS_E_NOMEM;

	mgmt_cmd_log_buf->p_buf_tail_idx = &mgmt_cmd_log_buf->buf_tail_idx;

	/* WMI Management commands Tx completed*/
	mgmt_cmd_tx_cmp_log_buf->length = 0;
	mgmt_cmd_tx_cmp_log_buf->buf_tail_idx = 0;
	mgmt_cmd_tx_cmp_log_buf->buf = (struct wmi_command_debug *)
		qdf_mem_malloc(
		wmi_mgmt_tx_cmpl_log_max_entry *
		sizeof(struct wmi_command_debug));
	mgmt_cmd_tx_cmp_log_buf->size = wmi_mgmt_tx_cmpl_log_max_entry;

	if (!mgmt_cmd_tx_cmp_log_buf->buf)
		return QDF_STATUS_E_NOMEM;

	mgmt_cmd_tx_cmp_log_buf->p_buf_tail_idx =
		&mgmt_cmd_tx_cmp_log_buf->buf_tail_idx;

	/* WMI Management events when received */
	mgmt_event_log_buf->length = 0;
	mgmt_event_log_buf->buf_tail_idx = 0;

	mgmt_event_log_buf->buf = (struct wmi_event_debug *) qdf_mem_malloc(
		wmi_mgmt_rx_log_max_entry *
		sizeof(struct wmi_event_debug));
	mgmt_event_log_buf->size = wmi_mgmt_rx_log_max_entry;

	if (!mgmt_event_log_buf->buf)
		return QDF_STATUS_E_NOMEM;

	mgmt_event_log_buf->p_buf_tail_idx = &mgmt_event_log_buf->buf_tail_idx;

	/* WMI diag events when received */
	diag_event_log_buf->length = 0;
	diag_event_log_buf->buf_tail_idx = 0;

	diag_event_log_buf->buf = (struct wmi_event_debug *) qdf_mem_malloc(
		wmi_diag_log_max_entry *
		sizeof(struct wmi_event_debug));
	diag_event_log_buf->size = wmi_diag_log_max_entry;

	if (!diag_event_log_buf->buf)
		return QDF_STATUS_E_NOMEM;

	diag_event_log_buf->p_buf_tail_idx = &diag_event_log_buf->buf_tail_idx;

	qdf_spinlock_create(&wmi_handle->log_info.wmi_record_lock);
	wmi_handle->log_info.wmi_logging_enable = 1;

	wmi_filtered_logging_init(wmi_handle);

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
#ifdef WMI_INTERFACE_EVENT_LOGGING_DYNAMIC_ALLOC
static inline void wmi_log_buffer_free(struct wmi_unified *wmi_handle)
{
	wmi_filtered_logging_free(wmi_handle);

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
	if (wmi_handle->log_info.wmi_diag_event_log_buf_info.buf)
		qdf_mem_free(
			wmi_handle->log_info.wmi_diag_event_log_buf_info.buf);
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
		WMI_DEBUG_ENTRY_MAX_LENGTH / sizeof(uint32_t);
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
 * wmi_print_cmd_cmp_log_buffer() - wmi command completion log printer
 * @log_buffer: the command completion log buffer metadata of the buffer to print
 * @count: the maximum number of entries to print
 * @print: an abstract print method, e.g. a qdf_print() or seq_printf() wrapper
 * @print_priv: any data required by the print method, e.g. a file handle
 *
 * Return: None
 */
static void
wmi_print_cmd_cmp_log_buffer(struct wmi_log_buf_t *log_buffer, uint32_t count,
			 qdf_abstract_print *print, void *print_priv)
{
	static const int data_len =
		WMI_DEBUG_ENTRY_MAX_LENGTH / sizeof(uint32_t);
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
		struct wmi_command_cmp_debug *cmd_log = (struct wmi_command_cmp_debug *)
			&((struct wmi_command_cmp_debug *)log_buffer->buf)[idx];
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
		WMI_DEBUG_ENTRY_MAX_LENGTH / sizeof(uint32_t);
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
	wmi_print_cmd_cmp_log_buffer(
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
#define GENERATE_COMMAND_DEBUG_SHOW_FUNCS(func_base, wmi_ring_size, wmi_record_type)\
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
		qdf_spin_lock_bh(&wmi_handle->log_info.wmi_record_lock);\
		if (!wmi_log->length) {					\
			qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);\
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
		qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);\
		while (nread--) {					\
			struct wmi_record_type *wmi_record;		\
									\
			wmi_record = (struct wmi_record_type *)	\
			&(((struct wmi_record_type *)wmi_log->buf)[pos]);\
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
		qdf_spin_lock_bh(&wmi_handle->log_info.wmi_record_lock);\
		if (!wmi_log->length) {					\
			qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);\
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
		qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);\
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

GENERATE_COMMAND_DEBUG_SHOW_FUNCS(command_log, wmi_display_size,
				  wmi_command_debug);
GENERATE_COMMAND_DEBUG_SHOW_FUNCS(command_tx_cmp_log, wmi_display_size,
				  wmi_command_cmp_debug);
GENERATE_EVENT_DEBUG_SHOW_FUNCS(event_log, wmi_display_size);
GENERATE_EVENT_DEBUG_SHOW_FUNCS(rx_event_log, wmi_display_size);
GENERATE_COMMAND_DEBUG_SHOW_FUNCS(mgmt_command_log, wmi_display_size,
				  wmi_command_debug);
GENERATE_COMMAND_DEBUG_SHOW_FUNCS(mgmt_command_tx_cmp_log,
					wmi_display_size,
					wmi_command_debug);
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

	wmi_bp_seq_printf(m, "WMI command/cmpl log max size:%d/%d\n",
			  wmi_cmd_log_max_entry, wmi_cmd_cmpl_log_max_entry);
	wmi_bp_seq_printf(m, "WMI management Tx/cmpl log max size:%d/%d\n",
			  wmi_mgmt_tx_log_max_entry,
			  wmi_mgmt_tx_cmpl_log_max_entry);
	wmi_bp_seq_printf(m, "WMI event log max size:%d\n",
			  wmi_event_log_max_entry);
	wmi_bp_seq_printf(m, "WMI management Rx log max size:%d\n",
			  wmi_mgmt_rx_log_max_entry);
	return wmi_bp_seq_printf(m,
				 "WMI diag log max size:%d\n",
				 wmi_diag_log_max_entry);
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
			wmi_err("Wrong input, echo 0 to clear the wmi buffer");\
			return -EINVAL;					\
		}							\
									\
		qdf_spin_lock_bh(&wmi_handle->log_info.wmi_record_lock);\
		qdf_mem_zero(wmi_log->buf, wmi_ring_size *		\
				sizeof(struct wmi_record_type));	\
		wmi_log->length = 0;					\
		*(wmi_log->p_buf_tail_idx) = 0;				\
		qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);\
									\
		return count;						\
	}

GENERATE_DEBUG_WRITE_FUNCS(command_log, wmi_cmd_log_max_entry,
			   wmi_command_debug);
GENERATE_DEBUG_WRITE_FUNCS(command_tx_cmp_log, wmi_cmd_cmpl_log_max_entry,
			   wmi_command_cmp_debug);
GENERATE_DEBUG_WRITE_FUNCS(event_log, wmi_event_log_max_entry,
			   wmi_event_debug);
GENERATE_DEBUG_WRITE_FUNCS(rx_event_log, wmi_event_log_max_entry,
			   wmi_event_debug);
GENERATE_DEBUG_WRITE_FUNCS(mgmt_command_log, wmi_mgmt_tx_log_max_entry,
			   wmi_command_debug);
GENERATE_DEBUG_WRITE_FUNCS(mgmt_command_tx_cmp_log,
			   wmi_mgmt_tx_cmpl_log_max_entry, wmi_command_debug);
GENERATE_DEBUG_WRITE_FUNCS(mgmt_event_log, wmi_mgmt_rx_log_max_entry,
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
#ifdef WMI_INTERFACE_FILTERED_EVENT_LOGGING
GENERATE_DEBUG_STRUCTS(filtered_wmi_cmds);
GENERATE_DEBUG_STRUCTS(filtered_wmi_evts);
GENERATE_DEBUG_STRUCTS(wmi_filtered_command_log);
GENERATE_DEBUG_STRUCTS(wmi_filtered_event_log);
#endif

struct wmi_debugfs_info wmi_debugfs_infos[NUM_DEBUG_INFOS] = {
	DEBUG_FOO(wmi_command_log),
	DEBUG_FOO(wmi_command_tx_cmp_log),
	DEBUG_FOO(wmi_event_log),
	DEBUG_FOO(wmi_rx_event_log),
	DEBUG_FOO(wmi_mgmt_command_log),
	DEBUG_FOO(wmi_mgmt_command_tx_cmp_log),
	DEBUG_FOO(wmi_mgmt_event_log),
	DEBUG_FOO(wmi_enable),
	DEBUG_FOO(wmi_log_size),
#ifdef WMI_INTERFACE_FILTERED_EVENT_LOGGING
	DEBUG_FOO(filtered_wmi_cmds),
	DEBUG_FOO(filtered_wmi_evts),
	DEBUG_FOO(wmi_filtered_command_log),
	DEBUG_FOO(wmi_filtered_event_log),
#endif
};

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
			       struct dentry *par_entry)
{
	int i;

	if (!par_entry)
		goto out;

	for (i = 0; i < NUM_DEBUG_INFOS; ++i) {
		wmi_handle->debugfs_de[i] = debugfs_create_file(
				wmi_debugfs_infos[i].name, 0644, par_entry,
				wmi_handle, wmi_debugfs_infos[i].ops);

		if (!wmi_handle->debugfs_de[i]) {
			wmi_err("debug Entry creation failed!");
			goto out;
		}
	}

	return;

out:
	wmi_err("debug Entry creation failed!");
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

	if (dentry) {
		for (i = 0; i < NUM_DEBUG_INFOS; ++i) {
			if (wmi_handle->debugfs_de[i])
				wmi_handle->debugfs_de[i] = NULL;
		}
	}

	if (dentry)
		debugfs_remove_recursive(dentry);
}

/**
 * wmi_debugfs_init() - debugfs functions to create debugfs directory and to
 * create debugfs enteries.
 *
 * @h: wmi handler
 *
 * Return: init status
 */
static QDF_STATUS wmi_debugfs_init(wmi_unified_t wmi_handle, uint32_t pdev_idx)
{
	char buf[32];

	snprintf(buf, sizeof(buf), "WMI_SOC%u_PDEV%u",
		 wmi_handle->soc->soc_idx, pdev_idx);

	wmi_handle->log_info.wmi_log_debugfs_dir =
		debugfs_create_dir(buf, NULL);

	if (!wmi_handle->log_info.wmi_log_debugfs_dir) {
		wmi_err("error while creating debugfs dir for %s", buf);
		return QDF_STATUS_E_FAILURE;
	}
	wmi_debugfs_create(wmi_handle,
			   wmi_handle->log_info.wmi_log_debugfs_dir);

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
	wmi_specific_cmd_record(wmi_handle, cmd, (uint8_t *)data);
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

#ifdef WMI_EXT_DBG

/**
 * wmi_ext_dbg_msg_enqueue() - enqueue wmi message
 * @wmi_handle: wmi handler
 *
 * Return: size of wmi message queue after enqueue
 */
static uint32_t wmi_ext_dbg_msg_enqueue(struct wmi_unified *wmi_handle,
					struct wmi_ext_dbg_msg *msg)
{
	uint32_t list_size;

	qdf_spinlock_acquire(&wmi_handle->wmi_ext_dbg_msg_queue_lock);
	qdf_list_insert_back_size(&wmi_handle->wmi_ext_dbg_msg_queue,
				  &msg->node, &list_size);
	qdf_spinlock_release(&wmi_handle->wmi_ext_dbg_msg_queue_lock);

	return list_size;
}

/**
 * wmi_ext_dbg_msg_dequeue() - dequeue wmi message
 * @wmi_handle: wmi handler
 *
 * Return: wmi msg on success else NULL
 */
static struct wmi_ext_dbg_msg *wmi_ext_dbg_msg_dequeue(struct wmi_unified
						       *wmi_handle)
{
	qdf_list_node_t *list_node = NULL;

	qdf_spinlock_acquire(&wmi_handle->wmi_ext_dbg_msg_queue_lock);
	qdf_list_remove_front(&wmi_handle->wmi_ext_dbg_msg_queue, &list_node);
	qdf_spinlock_release(&wmi_handle->wmi_ext_dbg_msg_queue_lock);

	if (!list_node)
		return NULL;

	return qdf_container_of(list_node, struct wmi_ext_dbg_msg, node);
}

/**
 * wmi_ext_dbg_msg_record() - record wmi messages
 * @wmi_handle: wmi handler
 * @buf: wmi message buffer
 * @len: wmi message length
 * @type: wmi message type
 *
 * Return: QDF_STATUS_SUCCESS on successful recording else failure.
 */
static QDF_STATUS wmi_ext_dbg_msg_record(struct wmi_unified *wmi_handle,
					 uint8_t *buf, uint32_t len,
					 enum WMI_MSG_TYPE type)
{
	struct wmi_ext_dbg_msg *msg;
	uint32_t list_size;

	msg = wmi_ext_dbg_msg_get(len);
	if (!msg)
		return QDF_STATUS_E_NOMEM;

	msg->len = len;
	msg->type = type;
	qdf_mem_copy(msg->buf, buf, len);
	msg->ts = qdf_get_log_timestamp();
	list_size = wmi_ext_dbg_msg_enqueue(wmi_handle, msg);

	if (list_size >= wmi_handle->wmi_ext_dbg_msg_queue_size) {
		msg = wmi_ext_dbg_msg_dequeue(wmi_handle);
		wmi_ext_dbg_msg_put(msg);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_ext_dbg_msg_cmd_record() - record wmi command messages
 * @wmi_handle: wmi handler
 * @buf: wmi command buffer
 * @len: wmi command message length
 *
 * Return: QDF_STATUS_SUCCESS on successful recording else failure.
 */
static QDF_STATUS wmi_ext_dbg_msg_cmd_record(struct wmi_unified *wmi_handle,
					     uint8_t *buf, uint32_t len)
{
	return wmi_ext_dbg_msg_record(wmi_handle, buf, len,
				      WMI_MSG_TYPE_CMD);
}

/**
 * wmi_ext_dbg_msg_event_record() - record wmi event messages
 * @wmi_handle: wmi handler
 * @buf: wmi event buffer
 * @len: wmi event message length
 *
 * Return: QDF_STATUS_SUCCESS on successful recording else failure.
 */
static QDF_STATUS wmi_ext_dbg_msg_event_record(struct wmi_unified *wmi_handle,
					       uint8_t *buf, uint32_t len)
{
	uint32_t id;

	id = WMI_GET_FIELD(buf, WMI_CMD_HDR, COMMANDID);
	if (id != wmi_handle->wmi_events[wmi_diag_event_id])
		return wmi_ext_dbg_msg_record(wmi_handle, buf, len,
					      WMI_MSG_TYPE_EVENT);

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_ext_dbg_msg_queue_init() - create debugfs queue and associated lock
 * @wmi_handle: wmi handler
 *
 * Return: none
 */
static void wmi_ext_dbg_msg_queue_init(struct wmi_unified *wmi_handle)
{
	qdf_list_create(&wmi_handle->wmi_ext_dbg_msg_queue,
			wmi_handle->wmi_ext_dbg_msg_queue_size);
	qdf_spinlock_create(&wmi_handle->wmi_ext_dbg_msg_queue_lock);
}

/**
 * wmi_ext_dbg_msg_queue_deinit() - destroy debugfs queue and associated lock
 * @wmi_handle: wmi handler
 *
 * Return: none
 */
static void wmi_ext_dbg_msg_queue_deinit(struct wmi_unified *wmi_handle)
{
	qdf_list_destroy(&wmi_handle->wmi_ext_dbg_msg_queue);
	qdf_spinlock_destroy(&wmi_handle->wmi_ext_dbg_msg_queue_lock);
}

/**
 * wmi_ext_dbg_msg_show() - debugfs function to display whole content of
 * wmi command/event messages including headers.
 * @file: qdf debugfs file handler
 * @arg: pointer to wmi handler
 *
 * Return: QDF_STATUS_SUCCESS if all the messages are shown successfully,
 * else QDF_STATUS_E_AGAIN if more data to show.
 */
static QDF_STATUS wmi_ext_dbg_msg_show(qdf_debugfs_file_t file, void *arg)
{
	struct wmi_unified *wmi_handle = (struct wmi_unified *)arg;
	struct wmi_ext_dbg_msg *msg;
	uint64_t secs, usecs;

	msg = wmi_ext_dbg_msg_dequeue(wmi_handle);
	if (!msg)
		return QDF_STATUS_SUCCESS;

	qdf_debugfs_printf(file, "%s: 0x%x\n",
			   msg->type == WMI_MSG_TYPE_CMD ? "COMMAND" :
			   "EVENT", WMI_GET_FIELD(msg->buf, WMI_CMD_HDR,
						  COMMANDID));
	qdf_log_timestamp_to_secs(msg->ts, &secs, &usecs);
	qdf_debugfs_printf(file, "Time: %llu.%llu\n", secs, usecs);
	qdf_debugfs_printf(file, "Length:%d\n", msg->len);
	qdf_debugfs_hexdump(file, msg->buf, msg->len,
			    WMI_EXT_DBG_DUMP_ROW_SIZE,
			    WMI_EXT_DBG_DUMP_GROUP_SIZE);
	qdf_debugfs_printf(file, "\n");

	if (qdf_debugfs_overflow(file)) {
		qdf_spinlock_acquire(&wmi_handle->wmi_ext_dbg_msg_queue_lock);
		qdf_list_insert_front(&wmi_handle->wmi_ext_dbg_msg_queue,
				      &msg->node);
		qdf_spinlock_release(&wmi_handle->wmi_ext_dbg_msg_queue_lock);

	} else {
		wmi_ext_dbg_msg_put(msg);
	}

	return QDF_STATUS_E_AGAIN;
}

/**
 * wmi_ext_dbg_msg_write() - debugfs write not supported
 * @priv: private data
 * @buf: received data buffer
 * @len: length of received buffer
 *
 * Return: QDF_STATUS_E_NOSUPPORT.
 */
static QDF_STATUS wmi_ext_dbg_msg_write(void *priv, const char *buf,
					qdf_size_t len)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static struct qdf_debugfs_fops wmi_ext_dbgfs_ops[WMI_MAX_RADIOS];

/**
 * wmi_ext_debugfs_init() - init debugfs items for extended wmi dump.
 * @wmi_handle: wmi handler
 * @pdev_idx: pdev index
 *
 * Return: QDF_STATUS_SUCCESS if debugfs is initialized else
 * QDF_STATUS_E_FAILURE
 */
static QDF_STATUS wmi_ext_dbgfs_init(struct wmi_unified *wmi_handle,
				     uint32_t pdev_idx)
{
	qdf_dentry_t dentry;
	char buf[32];

	/* To maintain backward compatibility, naming convention for PDEV 0
	 * dentry is kept same as before. For more than 1 PDEV, dentry
	 * names will be appended with PDEVx.
	*/
	if (wmi_handle->soc->soc_idx == 0 && pdev_idx == 0) {
		dentry  = qdf_debugfs_create_dir(WMI_EXT_DBG_DIR, NULL);
	} else {
		snprintf(buf, sizeof(buf), "WMI_EXT_DBG_SOC%u_PDEV%u",
			 wmi_handle->soc->soc_idx, pdev_idx);
		dentry  = qdf_debugfs_create_dir(buf, NULL);
	}

	if (!dentry) {
		wmi_err("error while creating extended wmi debugfs dir");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_ext_dbgfs_ops[pdev_idx].show = wmi_ext_dbg_msg_show;
	wmi_ext_dbgfs_ops[pdev_idx].write = wmi_ext_dbg_msg_write;
	wmi_ext_dbgfs_ops[pdev_idx].priv = wmi_handle;
	if (!qdf_debugfs_create_file(WMI_EXT_DBG_FILE, WMI_EXT_DBG_FILE_PERM,
				     dentry, &wmi_ext_dbgfs_ops[pdev_idx])) {
		qdf_debugfs_remove_dir(dentry);
		wmi_err("Error while creating extended wmi debugfs file");
		return QDF_STATUS_E_FAILURE;
	}

	wmi_handle->wmi_ext_dbg_dentry = dentry;
	wmi_handle->wmi_ext_dbg_msg_queue_size = WMI_EXT_DBG_QUEUE_SIZE;
	wmi_ext_dbg_msg_queue_init(wmi_handle);

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_ext_debugfs_deinit() - cleanup/deinit debugfs items of extended wmi dump.
 * @wmi_handle: wmi handler
 *
 * Return: QDF_STATUS_SUCCESS if cleanup is successful
 */
static QDF_STATUS wmi_ext_dbgfs_deinit(struct wmi_unified *wmi_handle)
{
	struct wmi_ext_dbg_msg *msg;

	while ((msg = wmi_ext_dbg_msg_dequeue(wmi_handle)))
		wmi_ext_dbg_msg_put(msg);

	wmi_ext_dbg_msg_queue_deinit(wmi_handle);
	qdf_debugfs_remove_dir_recursive(wmi_handle->wmi_ext_dbg_dentry);

	return QDF_STATUS_SUCCESS;
}

#else

static inline QDF_STATUS wmi_ext_dbg_msg_cmd_record(struct wmi_unified
						    *wmi_handle,
						    uint8_t *buf, uint32_t len)
{
		return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wmi_ext_dbg_msg_event_record(struct wmi_unified
						      *wmi_handle,
						      uint8_t *buf, uint32_t len)
{
		return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wmi_ext_dbgfs_init(struct wmi_unified *wmi_handle,
					    uint32_t pdev_idx)
{
		return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wmi_ext_dbgfs_deinit(struct wmi_unified *wmi_handle)
{
		return QDF_STATUS_SUCCESS;
}

#endif /*WMI_EXT_DBG */

int wmi_get_host_credits(wmi_unified_t wmi_handle);
/* WMI buffer APIs */

#ifdef NBUF_MEMORY_DEBUG
wmi_buf_t
wmi_buf_alloc_debug(wmi_unified_t wmi_handle, uint32_t len,
		    const char *func_name,
		    uint32_t line_num)
{
	wmi_buf_t wmi_buf;

	if (roundup(len + WMI_MIN_HEAD_ROOM, 4) > wmi_handle->max_msg_len) {
		QDF_ASSERT(0);
		return NULL;
	}

	wmi_buf = wbuff_buff_get(wmi_handle->wbuff_handle, len, func_name,
				 line_num);
	if (!wmi_buf)
		wmi_buf = qdf_nbuf_alloc_debug(NULL,
					       roundup(len + WMI_MIN_HEAD_ROOM,
						       4),
					       WMI_MIN_HEAD_ROOM, 4, false,
					       func_name, line_num);
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
	net_buf = wbuff_buff_put(net_buf);
	if (net_buf)
		qdf_nbuf_free(net_buf);
}
qdf_export_symbol(wmi_buf_free);
#else
wmi_buf_t wmi_buf_alloc_fl(wmi_unified_t wmi_handle, uint32_t len,
			   const char *func, uint32_t line)
{
	wmi_buf_t wmi_buf;

	if (roundup(len + WMI_MIN_HEAD_ROOM, 4) > wmi_handle->max_msg_len) {
		QDF_DEBUG_PANIC("Invalid length %u (via %s:%u)",
				len, func, line);
		return NULL;
	}

	wmi_buf = wbuff_buff_get(wmi_handle->wbuff_handle, len, __func__,
				 __LINE__);
	if (!wmi_buf)
		wmi_buf = qdf_nbuf_alloc_fl(NULL, roundup(len +
				WMI_MIN_HEAD_ROOM, 4), WMI_MIN_HEAD_ROOM, 4,
				false, func, line);

	if (!wmi_buf) {
		wmi_nofl_err("%s:%d, failed to alloc len:%d", func, line, len);
		return NULL;
	}

	/* Clear the wmi buffer */
	OS_MEMZERO(qdf_nbuf_data(wmi_buf), len);

	/*
	 * Set the length of the buffer to match the allocation size.
	 */
	qdf_nbuf_set_pktlen(wmi_buf, len);

	return wmi_buf;
}
qdf_export_symbol(wmi_buf_alloc_fl);

void wmi_buf_free(wmi_buf_t net_buf)
{
	net_buf = wbuff_buff_put(net_buf);
	if (net_buf)
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

static inline void wmi_log_cmd_id(uint32_t cmd_id, uint32_t tag)
{
	wmi_debug("Send WMI command:%s command_id:%d htc_tag:%d",
		 wmi_id_to_name(cmd_id), cmd_id, tag);
}

/**
 * wmi_is_pm_resume_cmd() - check if a cmd is part of the resume sequence
 * @cmd_id: command to check
 *
 * Return: true if the command is part of the resume sequence.
 */
#ifdef WLAN_POWER_MANAGEMENT_OFFLOAD
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

#ifdef FEATURE_WLAN_D0WOW
static bool wmi_is_legacy_d0wow_disable_cmd(wmi_buf_t buf, uint32_t cmd_id)
{
	wmi_d0_wow_enable_disable_cmd_fixed_param *cmd;

	if (cmd_id == WMI_D0_WOW_ENABLE_DISABLE_CMDID) {
		cmd = (wmi_d0_wow_enable_disable_cmd_fixed_param *)
			wmi_buf_data(buf);
		if (!cmd->enable)
			return true;
		else
			return false;
	}

	return false;
}
#else
static bool wmi_is_legacy_d0wow_disable_cmd(wmi_buf_t buf, uint32_t cmd_id)
{
	return false;
}

#endif

#ifdef WMI_INTERFACE_SEQUENCE_CHECK
static inline void wmi_interface_sequence_init(struct wmi_unified *wmi_handle)
{
	qdf_spinlock_create(&wmi_handle->wmi_seq_lock);
	wmi_handle->wmi_sequence = 0;
	wmi_handle->wmi_exp_sequence = 0;
}

static inline void wmi_interface_sequence_deinit(struct wmi_unified *wmi_handle)
{
	qdf_spinlock_destroy(&wmi_handle->wmi_seq_lock);
}

static inline QDF_STATUS wmi_htc_send_pkt(struct wmi_unified *wmi_handle,
					  HTC_PACKET *pkt,
					  const char *func, uint32_t line)
{
	wmi_buf_t buf = GET_HTC_PACKET_NET_BUF_CONTEXT(pkt);
	QDF_STATUS status;

	qdf_spin_lock_bh(&wmi_handle->wmi_seq_lock);
	status = htc_send_pkt(wmi_handle->htc_handle, pkt);
	if (QDF_STATUS_SUCCESS != status) {
		qdf_spin_unlock_bh(&wmi_handle->wmi_seq_lock);
		qdf_atomic_dec(&wmi_handle->pending_cmds);
		wmi_nofl_err("%s:%d, htc_send_pkt failed, status:%d",
			     func, line, status);
		qdf_mem_free(pkt);
		return status;
	}
	/* Record the sequence number in the SKB */
	qdf_nbuf_set_mark(buf, wmi_handle->wmi_sequence);
	/* Increment the sequence number */
	wmi_handle->wmi_sequence = (wmi_handle->wmi_sequence + 1)
				   & (wmi_handle->wmi_max_cmds - 1);
	qdf_spin_unlock_bh(&wmi_handle->wmi_seq_lock);

	return status;
}

static inline void wmi_interface_sequence_check(struct wmi_unified *wmi_handle,
						wmi_buf_t buf)
{
	qdf_spin_lock_bh(&wmi_handle->wmi_seq_lock);
	/* Match the completion sequence and expected sequence number */
	if (qdf_nbuf_get_mark(buf) != wmi_handle->wmi_exp_sequence) {
		qdf_spin_unlock_bh(&wmi_handle->wmi_seq_lock);
		wmi_nofl_err("WMI Tx Completion Sequence number mismatch");
		wmi_nofl_err("Expected %d Received %d",
			     wmi_handle->wmi_exp_sequence,
			     qdf_nbuf_get_mark(buf));
		/* Trigger Recovery */
		qdf_trigger_self_recovery(wmi_handle->soc,
					  QDF_WMI_BUF_SEQUENCE_MISMATCH);
	} else {
		/* Increment the expected sequence number */
		wmi_handle->wmi_exp_sequence =
				(wmi_handle->wmi_exp_sequence + 1)
				& (wmi_handle->wmi_max_cmds - 1);
		qdf_spin_unlock_bh(&wmi_handle->wmi_seq_lock);
	}
}
#else
static inline void wmi_interface_sequence_init(struct wmi_unified *wmi_handle)
{
}

static inline void wmi_interface_sequence_deinit(struct wmi_unified *wmi_handle)
{
}

static inline QDF_STATUS wmi_htc_send_pkt(struct wmi_unified *wmi_handle,
					  HTC_PACKET *pkt,
					  const char *func, uint32_t line)
{
	QDF_STATUS status;

	status = htc_send_pkt(wmi_handle->htc_handle, pkt);
	if (QDF_STATUS_SUCCESS != status) {
		qdf_atomic_dec(&wmi_handle->pending_cmds);
		wmi_nofl_err("%s:%d, htc_send_pkt failed, status:%d",
			     func, line, status);
		qdf_mem_free(pkt);
		return status;
	}

	return status;
}

static inline void wmi_interface_sequence_check(struct wmi_unified *wmi_handle,
						wmi_buf_t buf)
{
}
#endif

static inline void wmi_unified_debug_dump(wmi_unified_t wmi_handle)
{
	wmi_nofl_err("Endpoint ID = %d, Tx Queue Depth = %d, soc_id = %u, target type = %s",
		     wmi_handle->wmi_endpoint_id,
		     htc_get_tx_queue_depth(wmi_handle->htc_handle,
					    wmi_handle->wmi_endpoint_id),
		     wmi_handle->soc->soc_idx,
		     (wmi_handle->target_type ==
		      WMI_TLV_TARGET ? "WMI_TLV_TARGET" :
						"WMI_NON_TLV_TARGET"));
}

QDF_STATUS wmi_unified_cmd_send_fl(wmi_unified_t wmi_handle, wmi_buf_t buf,
				   uint32_t len, uint32_t cmd_id,
				   const char *func, uint32_t line)
{
	HTC_PACKET *pkt;
	uint16_t htc_tag = 0;

	if (wmi_get_runtime_pm_inprogress(wmi_handle)) {
		htc_tag = wmi_handle->ops->wmi_set_htc_tx_tag(wmi_handle, buf,
							      cmd_id);
	} else if (qdf_atomic_read(&wmi_handle->is_target_suspended) &&
		   !wmi_is_pm_resume_cmd(cmd_id) &&
		   !wmi_is_legacy_d0wow_disable_cmd(buf, cmd_id)) {
			wmi_nofl_err("Target is suspended (via %s:%u)",
					func, line);
		return QDF_STATUS_E_BUSY;
	}

	if (wmi_handle->wmi_stopinprogress) {
		wmi_nofl_err("%s:%d, WMI stop in progress, wmi_handle:%pK",
			     func, line, wmi_handle);
		return QDF_STATUS_E_INVAL;
	}

#ifndef WMI_NON_TLV_SUPPORT
	/* Do sanity check on the TLV parameter structure */
	if (wmi_handle->target_type == WMI_TLV_TARGET) {
		void *buf_ptr = (void *)qdf_nbuf_data(buf);

		if (wmi_handle->ops->wmi_check_command_params(NULL, buf_ptr, len, cmd_id)
			!= 0) {
			wmi_nofl_err("%s:%d, Invalid WMI Param Buffer for Cmd:%d",
				     func, line, cmd_id);
			return QDF_STATUS_E_INVAL;
		}
	}
#endif

	if (qdf_nbuf_push_head(buf, sizeof(WMI_CMD_HDR)) == NULL) {
		wmi_nofl_err("%s:%d, Failed to send cmd %x, no memory",
			     func, line, cmd_id);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(qdf_nbuf_data(buf), sizeof(WMI_CMD_HDR));
	WMI_SET_FIELD(qdf_nbuf_data(buf), WMI_CMD_HDR, COMMANDID, cmd_id);

	qdf_atomic_inc(&wmi_handle->pending_cmds);
	if (qdf_atomic_read(&wmi_handle->pending_cmds) >=
			wmi_handle->wmi_max_cmds) {
		wmi_nofl_err("hostcredits = %d",
			     wmi_get_host_credits(wmi_handle));
		htc_dump_counter_info(wmi_handle->htc_handle);
		qdf_atomic_dec(&wmi_handle->pending_cmds);
		wmi_nofl_err("%s:%d, MAX %d WMI Pending cmds reached",
			     func, line, wmi_handle->wmi_max_cmds);
		wmi_unified_debug_dump(wmi_handle);
		htc_ce_tasklet_debug_dump(wmi_handle->htc_handle);
		qdf_trigger_self_recovery(wmi_handle->soc->wmi_psoc,
					  QDF_WMI_EXCEED_MAX_PENDING_CMDS);
		return QDF_STATUS_E_BUSY;
	}

	pkt = qdf_mem_malloc_fl(sizeof(*pkt), func, line);
	if (!pkt) {
		qdf_atomic_dec(&wmi_handle->pending_cmds);
		return QDF_STATUS_E_NOMEM;
	}

	SET_HTC_PACKET_INFO_TX(pkt,
			       NULL,
			       qdf_nbuf_data(buf), len + sizeof(WMI_CMD_HDR),
			       wmi_handle->wmi_endpoint_id, htc_tag);

	SET_HTC_PACKET_NET_BUF_CONTEXT(pkt, buf);
	wmi_log_cmd_id(cmd_id, htc_tag);
	wmi_ext_dbg_msg_cmd_record(wmi_handle,
				   qdf_nbuf_data(buf), qdf_nbuf_len(buf));
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
			uint8_t *tmpbuf = (uint8_t *)qdf_nbuf_data(buf) +
				wmi_handle->soc->buf_offset_command;

			WMI_COMMAND_RECORD(wmi_handle, cmd_id, tmpbuf);
			wmi_specific_cmd_record(wmi_handle, cmd_id, tmpbuf);
		}

		qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);
	}
#endif
	return wmi_htc_send_pkt(wmi_handle, pkt, func, line);
}
qdf_export_symbol(wmi_unified_cmd_send_fl);

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
		    wmi_handle->event_handler[idx]) {
			return idx;
		}
	}

	return invalid_idx;
}

/**
 * wmi_register_event_handler_with_ctx() - register event handler with
 * exec ctx and buffer type
 * @wmi_handle: handle to wmi
 * @event_id: wmi event id
 * @handler_func: wmi event handler function
 * @rx_ctx: rx execution context for wmi rx events
 * @rx_buf_type: rx execution context for wmi rx events
 *
 * Return: QDF_STATUS_SUCCESS on successful register event else failure.
 */
static QDF_STATUS
wmi_register_event_handler_with_ctx(wmi_unified_t wmi_handle,
				    uint32_t event_id,
				    wmi_unified_event_handler handler_func,
				    enum wmi_rx_exec_ctx rx_ctx,
				    enum wmi_rx_buff_type rx_buf_type)
{
	uint32_t idx = 0;
	uint32_t evt_id;
	struct wmi_soc *soc;

	if (!wmi_handle) {
		wmi_err("WMI handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	soc = wmi_handle->soc;

	if (event_id >= wmi_events_max ||
		wmi_handle->wmi_events[event_id] == WMI_EVENT_ID_INVALID) {
		QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_INFO,
			  "%s: Event id %d is unavailable",
					__func__, event_id);
		return QDF_STATUS_E_FAILURE;
	}
	evt_id = wmi_handle->wmi_events[event_id];

	if (wmi_unified_get_event_handler_ix(wmi_handle, evt_id) != -1) {
		wmi_info("event handler already registered 0x%x", evt_id);
		return QDF_STATUS_E_FAILURE;
	}
	if (soc->max_event_idx == WMI_UNIFIED_MAX_EVENT) {
		wmi_err("no more event handlers 0x%x",
			 evt_id);
		return QDF_STATUS_E_FAILURE;
	}
	QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
		  "Registered event handler for event 0x%8x", evt_id);
	idx = soc->max_event_idx;
	wmi_handle->event_handler[idx] = handler_func;
	wmi_handle->event_id[idx] = evt_id;

	qdf_spin_lock_bh(&soc->ctx_lock);
	wmi_handle->ctx[idx].exec_ctx = rx_ctx;
	wmi_handle->ctx[idx].buff_type = rx_buf_type;
	qdf_spin_unlock_bh(&soc->ctx_lock);
	soc->max_event_idx++;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wmi_unified_register_event(wmi_unified_t wmi_handle,
			   uint32_t event_id,
			   wmi_unified_event_handler handler_func)
{
	return wmi_register_event_handler_with_ctx(wmi_handle, event_id,
						   handler_func,
						   WMI_RX_UMAC_CTX,
						   WMI_RX_PROCESSED_BUFF);
}

QDF_STATUS
wmi_unified_register_event_handler(wmi_unified_t wmi_handle,
				   wmi_conv_event_id event_id,
				   wmi_unified_event_handler handler_func,
				   uint8_t rx_ctx)
{
	return wmi_register_event_handler_with_ctx(wmi_handle, event_id,
						   handler_func, rx_ctx,
						   WMI_RX_PROCESSED_BUFF);
}

qdf_export_symbol(wmi_unified_register_event_handler);

QDF_STATUS
wmi_unified_register_raw_event_handler(wmi_unified_t wmi_handle,
				       wmi_conv_event_id event_id,
				       wmi_unified_event_handler handler_func,
				       enum wmi_rx_exec_ctx rx_ctx)
{
	return wmi_register_event_handler_with_ctx(wmi_handle, event_id,
						   handler_func, rx_ctx,
						   WMI_RX_RAW_BUFF);
}

qdf_export_symbol(wmi_unified_register_raw_event_handler);

QDF_STATUS wmi_unified_unregister_event(wmi_unified_t wmi_handle,
					uint32_t event_id)
{
	uint32_t idx = 0;
	uint32_t evt_id;
	struct wmi_soc *soc = wmi_handle->soc;

	if (event_id >= wmi_events_max ||
		wmi_handle->wmi_events[event_id] == WMI_EVENT_ID_INVALID) {
		QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_INFO,
			  "%s: Event id %d is unavailable",
					__func__, event_id);
		return QDF_STATUS_E_FAILURE;
	}
	evt_id = wmi_handle->wmi_events[event_id];

	idx = wmi_unified_get_event_handler_ix(wmi_handle, evt_id);
	if (idx == -1) {
		wmi_warn("event handler is not registered: evt id 0x%x",
			 evt_id);
		return QDF_STATUS_E_FAILURE;
	}
	wmi_handle->event_handler[idx] = NULL;
	wmi_handle->event_id[idx] = 0;
	--soc->max_event_idx;
	wmi_handle->event_handler[idx] =
		wmi_handle->event_handler[soc->max_event_idx];
	wmi_handle->event_id[idx] =
		wmi_handle->event_id[soc->max_event_idx];

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wmi_unified_unregister_event_handler(wmi_unified_t wmi_handle,
						wmi_conv_event_id event_id)
{
	uint32_t idx = 0;
	uint32_t evt_id;
	struct wmi_soc *soc;

	if (!wmi_handle) {
		wmi_err("WMI handle is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	soc = wmi_handle->soc;

	if (event_id >= wmi_events_max ||
		wmi_handle->wmi_events[event_id] == WMI_EVENT_ID_INVALID) {
		wmi_err("Event id %d is unavailable", event_id);
		return QDF_STATUS_E_FAILURE;
	}
	evt_id = wmi_handle->wmi_events[event_id];

	idx = wmi_unified_get_event_handler_ix(wmi_handle, evt_id);
	if (idx == -1) {
		wmi_err("event handler is not registered: evt id 0x%x",
			 evt_id);
		return QDF_STATUS_E_FAILURE;
	}
	wmi_handle->event_handler[idx] = NULL;
	wmi_handle->event_id[idx] = 0;
	--soc->max_event_idx;
	wmi_handle->event_handler[idx] =
		wmi_handle->event_handler[soc->max_event_idx];
	wmi_handle->event_id[idx] =
		wmi_handle->event_id[soc->max_event_idx];

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wmi_unified_unregister_event_handler);

void wmi_process_fw_event_worker_thread_ctx(struct wmi_unified *wmi_handle,
					    void *evt_buf)
{

	qdf_spin_lock_bh(&wmi_handle->eventq_lock);
	qdf_nbuf_queue_add(&wmi_handle->event_queue, evt_buf);
	qdf_spin_unlock_bh(&wmi_handle->eventq_lock);
	qdf_queue_work(0, wmi_handle->wmi_rx_work_queue,
			&wmi_handle->rx_event_work);

	return;
}

qdf_export_symbol(wmi_process_fw_event_worker_thread_ctx);

uint32_t wmi_critical_events_in_flight(struct wmi_unified *wmi)
{
	return qdf_atomic_read(&wmi->critical_events_in_flight);
}

static bool
wmi_is_event_critical(struct wmi_unified *wmi_handle, uint32_t event_id)
{
	if (wmi_handle->wmi_events[wmi_roam_synch_event_id] == event_id)
		return true;

	return false;
}

static QDF_STATUS wmi_discard_fw_event(struct scheduler_msg *msg)
{
	struct wmi_process_fw_event_params *event_param;

	if (!msg->bodyptr)
		return QDF_STATUS_E_INVAL;

	event_param = (struct wmi_process_fw_event_params *)msg->bodyptr;
	qdf_nbuf_free(event_param->evt_buf);
	qdf_mem_free(msg->bodyptr);
	msg->bodyptr = NULL;
	msg->bodyval = 0;
	msg->type = 0;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS wmi_process_fw_event_handler(struct scheduler_msg *msg)
{
	struct wmi_process_fw_event_params *params =
		(struct wmi_process_fw_event_params *)msg->bodyptr;
	struct wmi_unified *wmi_handle;
	uint32_t event_id;

	wmi_handle = (struct wmi_unified *)params->wmi_handle;
	event_id = WMI_GET_FIELD(qdf_nbuf_data(params->evt_buf),
				 WMI_CMD_HDR, COMMANDID);
	wmi_process_fw_event(wmi_handle, params->evt_buf);

	if (wmi_is_event_critical(wmi_handle, event_id))
		qdf_atomic_dec(&wmi_handle->critical_events_in_flight);

	qdf_mem_free(msg->bodyptr);

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_process_fw_event_sched_thread_ctx() - common event handler to serialize
 *                                  event processing through scheduler thread
 * @ctx: wmi context
 * @ev: event buffer
 * @rx_ctx: rx execution context
 *
 * Return: 0 on success, errno on failure
 */
static QDF_STATUS
wmi_process_fw_event_sched_thread_ctx(struct wmi_unified *wmi,
				      void *ev)
{
	struct wmi_process_fw_event_params *params_buf;
	struct scheduler_msg msg = { 0 };
	uint32_t event_id;

	params_buf = qdf_mem_malloc(sizeof(struct wmi_process_fw_event_params));
	if (!params_buf) {
		wmi_err("malloc failed");
		qdf_nbuf_free(ev);
		return QDF_STATUS_E_NOMEM;
	}

	params_buf->wmi_handle = wmi;
	params_buf->evt_buf = ev;

	event_id = WMI_GET_FIELD(qdf_nbuf_data(params_buf->evt_buf),
				 WMI_CMD_HDR, COMMANDID);
	if (wmi_is_event_critical(wmi, event_id))
		qdf_atomic_inc(&wmi->critical_events_in_flight);

	msg.bodyptr = params_buf;
	msg.bodyval = 0;
	msg.callback = wmi_process_fw_event_handler;
	msg.flush_callback = wmi_discard_fw_event;

	if (QDF_STATUS_SUCCESS !=
		scheduler_post_message(QDF_MODULE_ID_TARGET_IF,
				       QDF_MODULE_ID_TARGET_IF,
				       QDF_MODULE_ID_TARGET_IF, &msg)) {
		qdf_nbuf_free(ev);
		qdf_mem_free(params_buf);
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_SUCCESS;
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
 * wmi_mtrace_rx() - Wrappper function for qdf_mtrace api
 * @message_id: 32-Bit Wmi message ID
 * @vdev_id: Vdev ID
 * @data: Actual message contents
 *
 * This function converts the 32-bit WMI message ID in 15-bit message ID
 * format for qdf_mtrace as in qdf_mtrace message there are only 15
 * bits reserved for message ID.
 * out of these 15-bits, 8-bits (From LSB) specifies the WMI_GRP_ID
 * and remaining 7-bits specifies the actual WMI command. With this
 * notation there can be maximum 256 groups and each group can have
 * max 128 commands can be supported.
 *
 * Return: None
 */
static void wmi_mtrace_rx(uint32_t message_id, uint16_t vdev_id, uint32_t data)
{
	uint16_t mtrace_message_id;

	mtrace_message_id = QDF_WMI_MTRACE_CMD_ID(message_id) |
		(QDF_WMI_MTRACE_GRP_ID(message_id) <<
						QDF_WMI_MTRACE_CMD_NUM_BITS);
	qdf_mtrace(QDF_MODULE_ID_WMI, QDF_MODULE_ID_WMA,
		   mtrace_message_id, vdev_id, data);
}

/**
 * wmi_process_control_rx() - process fw events callbacks
 * @wmi_handle: handle to wmi_unified
 * @evt_buf: handle to wmi_buf_t
 *
 * Return: none
 */
static void wmi_process_control_rx(struct wmi_unified *wmi_handle,
				   wmi_buf_t evt_buf)
{
	struct wmi_soc *soc = wmi_handle->soc;
	uint32_t id;
	uint32_t idx;
	enum wmi_rx_exec_ctx exec_ctx;

	id = WMI_GET_FIELD(qdf_nbuf_data(evt_buf), WMI_CMD_HDR, COMMANDID);
	idx = wmi_unified_get_event_handler_ix(wmi_handle, id);
	if (qdf_unlikely(idx == A_ERROR)) {
		wmi_debug("no handler registered for event id 0x%x", id);
		qdf_nbuf_free(evt_buf);
		return;
	}
	wmi_mtrace_rx(id, 0xFF, idx);
	qdf_spin_lock_bh(&soc->ctx_lock);
	exec_ctx = wmi_handle->ctx[idx].exec_ctx;
	qdf_spin_unlock_bh(&soc->ctx_lock);

#ifdef WMI_INTERFACE_EVENT_LOGGING
	if (wmi_handle->log_info.wmi_logging_enable) {
		uint8_t *data;
		data = qdf_nbuf_data(evt_buf);

		qdf_spin_lock_bh(&wmi_handle->log_info.wmi_record_lock);
		/* Exclude 4 bytes of TLV header */
		if (wmi_handle->ops->is_diag_event(id)) {
			WMI_DIAG_RX_EVENT_RECORD(wmi_handle, id,
				((uint8_t *) data +
				wmi_handle->soc->buf_offset_event));
		} else if (wmi_handle->ops->is_management_record(id)) {
			WMI_MGMT_RX_EVENT_RECORD(wmi_handle, id,
				((uint8_t *) data +
				wmi_handle->soc->buf_offset_event));
		} else {
			WMI_RX_EVENT_RECORD(wmi_handle, id, ((uint8_t *) data +
				wmi_handle->soc->buf_offset_event));
		}
		qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);
	}
#endif

	if (exec_ctx == WMI_RX_WORK_CTX) {
		wmi_process_fw_event_worker_thread_ctx
					(wmi_handle, evt_buf);
	} else if (exec_ctx == WMI_RX_TASKLET_CTX) {
		wmi_process_fw_event(wmi_handle, evt_buf);
	} else if (exec_ctx == WMI_RX_SERIALIZER_CTX) {
		wmi_process_fw_event_sched_thread_ctx(wmi_handle, evt_buf);
	} else {
		wmi_err("Invalid event context %d", exec_ctx);
		qdf_nbuf_free(evt_buf);
	}

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
	struct wmi_soc *soc = (struct wmi_soc *)ctx;
	struct wmi_unified *wmi_handle;
	wmi_buf_t evt_buf;

	evt_buf = (wmi_buf_t)htc_packet->pPktContext;

	wmi_handle = wmi_get_pdev_ep(soc, htc_packet->Endpoint);
	if (!wmi_handle) {
		wmi_err("unable to get wmi_handle to Endpoint %d",
			htc_packet->Endpoint);
		qdf_nbuf_free(evt_buf);
		return;
	}

	wmi_process_control_rx(wmi_handle, evt_buf);
}

#ifdef WLAN_FEATURE_WMI_DIAG_OVER_CE7
/**
 * wmi_control_diag_rx() - process diag fw events callbacks
 * @ctx: handle to wmi
 * @htc_packet: pointer to htc packet
 *
 * Return: none
 */
static void wmi_control_diag_rx(void *ctx, HTC_PACKET *htc_packet)
{
	struct wmi_soc *soc = (struct wmi_soc *)ctx;
	struct wmi_unified *wmi_handle;
	wmi_buf_t evt_buf;

	evt_buf = (wmi_buf_t)htc_packet->pPktContext;

	wmi_handle = soc->wmi_pdev[0];
	if (!wmi_handle) {
		WMI_LOGE
		("unable to get wmi_handle for diag event end point id:%d\n",
		 htc_packet->Endpoint);
		qdf_nbuf_free(evt_buf);
		return;
	}

	wmi_process_control_rx(wmi_handle, evt_buf);
}
#endif

#ifdef WLAN_FEATURE_WMI_SEND_RECV_QMI
QDF_STATUS wmi_unified_cmd_send_over_qmi(struct wmi_unified *wmi_handle,
					 wmi_buf_t buf, uint32_t buflen,
					 uint32_t cmd_id)
{
	QDF_STATUS status;
	int32_t ret;

	if (!qdf_nbuf_push_head(buf, sizeof(WMI_CMD_HDR))) {
		wmi_err("Failed to send cmd %x, no memory", cmd_id);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_zero(qdf_nbuf_data(buf), sizeof(WMI_CMD_HDR));
	WMI_SET_FIELD(qdf_nbuf_data(buf), WMI_CMD_HDR, COMMANDID, cmd_id);
	wmi_debug("Sending WMI_CMD_ID: 0x%x over qmi", cmd_id);
	status = qdf_wmi_send_recv_qmi(qdf_nbuf_data(buf),
				       buflen + sizeof(WMI_CMD_HDR),
				       wmi_handle,
				       wmi_process_qmi_fw_event);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_nbuf_pull_head(buf, sizeof(WMI_CMD_HDR));
		wmi_warn("WMI send on QMI failed. Retrying WMI on HTC");
	} else {
		ret = qdf_atomic_inc_return(&wmi_handle->num_stats_over_qmi);
		wmi_debug("num stats over qmi: %d", ret);
		wmi_buf_free(buf);
	}

	return status;
}

static int __wmi_process_qmi_fw_event(void *wmi_cb_ctx, void *buf, int len)
{
	struct wmi_unified *wmi_handle = wmi_cb_ctx;
	wmi_buf_t evt_buf;
	uint32_t evt_id;

	if (!wmi_handle || !buf)
		return -EINVAL;

	evt_buf = wmi_buf_alloc(wmi_handle, len);
	if (!evt_buf)
		return -ENOMEM;

	qdf_mem_copy(qdf_nbuf_data(evt_buf), buf, len);
	evt_id = WMI_GET_FIELD(qdf_nbuf_data(evt_buf), WMI_CMD_HDR, COMMANDID);
	wmi_debug("Received WMI_EVT_ID: %d over qmi", evt_id);
	wmi_process_control_rx(wmi_handle, evt_buf);

	return 0;
}

int wmi_process_qmi_fw_event(void *wmi_cb_ctx, void *buf, int len)
{
	struct qdf_op_sync *op_sync;
	int ret;

	if (qdf_op_protect(&op_sync))
		return -EINVAL;
	ret = __wmi_process_qmi_fw_event(wmi_cb_ctx, buf, len);
	qdf_op_unprotect(op_sync);

	return ret;
}
#endif

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
	struct wmi_raw_event_buffer ev_buf;
	enum wmi_rx_buff_type ev_buff_type;

	id = WMI_GET_FIELD(qdf_nbuf_data(evt_buf), WMI_CMD_HDR, COMMANDID);

	wmi_ext_dbg_msg_event_record(wmi_handle, qdf_nbuf_data(evt_buf),
				     qdf_nbuf_len(evt_buf));

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
				  "%s: Error: id=0x%x, wmitlv check status=%d",
				  __func__, id, tlv_ok_status);
			goto end;
		}
	}
#endif

	idx = wmi_unified_get_event_handler_ix(wmi_handle, id);
	if (idx == A_ERROR) {
		QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_ERROR,
		   "%s : event handler is not registered: event id 0x%x",
			__func__, id);
		goto end;
	}
#ifdef WMI_INTERFACE_EVENT_LOGGING
	if (wmi_handle->log_info.wmi_logging_enable) {
		qdf_spin_lock_bh(&wmi_handle->log_info.wmi_record_lock);
		/* Exclude 4 bytes of TLV header */
		if (wmi_handle->ops->is_diag_event(id)) {
			/*
			 * skip diag event logging in WMI event buffer
			 * as its already logged in WMI RX event buffer
			 */
		} else if (wmi_handle->ops->is_management_record(id)) {
			/*
			 * skip wmi mgmt event logging in WMI event buffer
			 * as its already logged in WMI RX event buffer
			 */
		} else {
			uint8_t *tmpbuf = (uint8_t *)data +
					wmi_handle->soc->buf_offset_event;

			WMI_EVENT_RECORD(wmi_handle, id, tmpbuf);
			wmi_specific_evt_record(wmi_handle, id, tmpbuf);
		}
		qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);
	}
#endif
	/* Call the WMI registered event handler */
	if (wmi_handle->target_type == WMI_TLV_TARGET) {
		ev_buff_type = wmi_handle->ctx[idx].buff_type;
		if (ev_buff_type == WMI_RX_PROCESSED_BUFF) {
			wmi_handle->event_handler[idx] (wmi_handle->scn_handle,
				wmi_cmd_struct_ptr, len);
		} else if (ev_buff_type == WMI_RX_RAW_BUFF) {
			ev_buf.evt_raw_buf = data;
			ev_buf.evt_processed_buf = wmi_cmd_struct_ptr;
			wmi_handle->event_handler[idx] (wmi_handle->scn_handle,
							(void *)&ev_buf, len);
		}
	}
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
		  "%s: WLAN_BUG_RCA: Message type %x has exceeded its alloted time of %ds",
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

	qdf_print_thread_trace(info->task);
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
static inline void wmi_interface_logging_init(struct wmi_unified *wmi_handle,
					      uint32_t pdev_idx)
{
	if (QDF_STATUS_SUCCESS == wmi_log_init(wmi_handle)) {
		qdf_spinlock_create(&wmi_handle->log_info.wmi_record_lock);
		wmi_debugfs_init(wmi_handle, pdev_idx);
	}
}
#else
static inline void wmi_interface_logging_init(struct wmi_unified *wmi_handle,
					      uint32_t pdev_idx)
{
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

	if (!soc->wmi_pdev[pdev_idx]) {
		wmi_handle =
			(struct wmi_unified *) qdf_mem_malloc(
					sizeof(struct wmi_unified));
		if (!wmi_handle)
			return NULL;

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
			qdf_alloc_unbound_workqueue("wmi_rx_event_work_queue");
		if (!wmi_handle->wmi_rx_work_queue) {
			wmi_err("failed to create wmi_rx_event_work_queue");
			goto error;
		}
		wmi_handle->wmi_events = soc->wmi_events;
		wmi_handle->services = soc->services;
		wmi_handle->soc = soc;
		wmi_handle->cmd_pdev_id_map = soc->cmd_pdev_id_map;
		wmi_handle->evt_pdev_id_map = soc->evt_pdev_id_map;
		wmi_handle->cmd_phy_id_map = soc->cmd_phy_id_map;
		wmi_handle->evt_phy_id_map = soc->evt_phy_id_map;
		wmi_interface_logging_init(wmi_handle, pdev_idx);
		qdf_atomic_init(&wmi_handle->pending_cmds);
		qdf_atomic_init(&wmi_handle->is_target_suspended);
		wmi_handle->target_type = soc->target_type;
		wmi_handle->wmi_max_cmds = soc->wmi_max_cmds;

		wmi_interface_sequence_init(wmi_handle);
		if (wmi_ext_dbgfs_init(wmi_handle, pdev_idx) !=
		    QDF_STATUS_SUCCESS)
			wmi_err("Failed to initialize wmi extended debugfs");

		soc->wmi_pdev[pdev_idx] = wmi_handle;
	} else
		wmi_handle = soc->wmi_pdev[pdev_idx];

	wmi_handle->wmi_stopinprogress = 0;
	wmi_handle->wmi_endpoint_id = soc->wmi_endpoint_id[pdev_idx];
	wmi_handle->htc_handle = soc->htc_handle;
	wmi_handle->max_msg_len = soc->max_msg_len[pdev_idx];
	wmi_handle->tag_crash_inject = false;

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
 * wmi_wbuff_register() - register wmi with wbuff
 * @wmi_handle: handle to wmi
 *
 * @Return: void
 */
static void wmi_wbuff_register(struct wmi_unified *wmi_handle)
{
	struct wbuff_alloc_request wbuff_alloc[4];

	wbuff_alloc[0].slot = WBUFF_POOL_0;
	wbuff_alloc[0].size = WMI_WBUFF_POOL_0_SIZE;
	wbuff_alloc[1].slot = WBUFF_POOL_1;
	wbuff_alloc[1].size = WMI_WBUFF_POOL_1_SIZE;
	wbuff_alloc[2].slot = WBUFF_POOL_2;
	wbuff_alloc[2].size = WMI_WBUFF_POOL_2_SIZE;
	wbuff_alloc[3].slot = WBUFF_POOL_3;
	wbuff_alloc[3].size = WMI_WBUFF_POOL_3_SIZE;

	wmi_handle->wbuff_handle = wbuff_module_register(wbuff_alloc, 4,
							 WMI_MIN_HEAD_ROOM, 4);
}

/**
 * wmi_wbuff_deregister() - deregister wmi with wbuff
 * @wmi_handle: handle to wmi
 *
 * @Return: void
 */
static inline void wmi_wbuff_deregister(struct wmi_unified *wmi_handle)
{
	wbuff_module_deregister(wmi_handle->wbuff_handle);
	wmi_handle->wbuff_handle = NULL;
}

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
			 struct wmi_unified_attach_params *param)
{
	struct wmi_unified *wmi_handle;
	struct wmi_soc *soc;

	soc = (struct wmi_soc *) qdf_mem_malloc(sizeof(struct wmi_soc));
	if (!soc)
		return NULL;

	wmi_handle =
		(struct wmi_unified *) qdf_mem_malloc(
			sizeof(struct wmi_unified));
	if (!wmi_handle) {
		qdf_mem_free(soc);
		return NULL;
	}
	wmi_handle->soc = soc;
	wmi_handle->soc->soc_idx = param->soc_id;
	wmi_handle->soc->is_async_ep = param->is_async_ep;
	wmi_handle->event_id = soc->event_id;
	wmi_handle->event_handler = soc->event_handler;
	wmi_handle->ctx = soc->ctx;
	wmi_handle->wmi_events = soc->wmi_events;
	wmi_handle->services = soc->services;
	wmi_handle->scn_handle = scn_handle;
	wmi_handle->cmd_pdev_id_map = soc->cmd_pdev_id_map;
	wmi_handle->evt_pdev_id_map = soc->evt_pdev_id_map;
	wmi_handle->cmd_phy_id_map = soc->cmd_phy_id_map;
	wmi_handle->evt_phy_id_map = soc->evt_phy_id_map;
	soc->scn_handle = scn_handle;
	qdf_atomic_init(&wmi_handle->pending_cmds);
	qdf_atomic_init(&wmi_handle->is_target_suspended);
	qdf_atomic_init(&wmi_handle->num_stats_over_qmi);
	wmi_runtime_pm_init(wmi_handle);
	qdf_spinlock_create(&wmi_handle->eventq_lock);
	qdf_nbuf_queue_init(&wmi_handle->event_queue);
	qdf_create_work(0, &wmi_handle->rx_event_work,
			wmi_rx_event_work, wmi_handle);
	wmi_handle->wmi_rx_work_queue =
		qdf_alloc_unbound_workqueue("wmi_rx_event_work_queue");
	if (!wmi_handle->wmi_rx_work_queue) {
		wmi_err("failed to create wmi_rx_event_work_queue");
		goto error;
	}
	wmi_interface_logging_init(wmi_handle, WMI_HOST_PDEV_ID_0);
	wmi_handle->target_type = param->target_type;
	soc->target_type = param->target_type;

	if (param->target_type >= WMI_MAX_TARGET_TYPE)
		goto error;

	if (wmi_attach_register[param->target_type]) {
		wmi_attach_register[param->target_type](wmi_handle);
	} else {
		wmi_err("wmi attach is not registered");
		goto error;
	}
	wmi_interface_sequence_init(wmi_handle);
	/* Assign target cookie capablity */
	wmi_handle->use_cookie = param->use_cookie;
	wmi_handle->osdev = param->osdev;
	wmi_handle->wmi_stopinprogress = 0;
	wmi_handle->wmi_max_cmds = param->max_commands;
	soc->wmi_max_cmds = param->max_commands;
	/* Increase the ref count once refcount infra is present */
	soc->wmi_psoc = param->psoc;
	qdf_spinlock_create(&soc->ctx_lock);
	soc->ops = wmi_handle->ops;
	soc->wmi_pdev[0] = wmi_handle;
	if (wmi_ext_dbgfs_init(wmi_handle, 0) != QDF_STATUS_SUCCESS)
		wmi_err("Failed to initialize wmi extended debugfs");

	wmi_wbuff_register(wmi_handle);

	wmi_hang_event_notifier_register(wmi_handle);

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

	wmi_hang_event_notifier_unregister();

	wmi_wbuff_deregister(wmi_handle);

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

			wmi_interface_sequence_deinit(soc->wmi_pdev[i]);
			wmi_ext_dbgfs_deinit(soc->wmi_pdev[i]);

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

	if (soc->wmi_ext2_service_bitmap) {
		qdf_mem_free(soc->wmi_ext2_service_bitmap);
		soc->wmi_ext2_service_bitmap = NULL;
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
	struct wmi_debug_log_info *log_info;
	uint32_t cmd_id;
	uint8_t *offset_ptr;
	qdf_dma_addr_t dma_addr;
	uint64_t phy_addr;
#endif

	ASSERT(wmi_cmd_buf);
	wmi_handle = wmi_get_pdev_ep(soc, htc_pkt->Endpoint);
	if (!wmi_handle) {
		wmi_err("Unable to get wmi handle");
		QDF_ASSERT(0);
		return;
	}
	buf_ptr = (u_int8_t *)wmi_buf_data(wmi_cmd_buf);
#ifdef WMI_INTERFACE_EVENT_LOGGING
	log_info = &wmi_handle->log_info;

	if (wmi_handle && log_info->wmi_logging_enable) {
		cmd_id = WMI_GET_FIELD(qdf_nbuf_data(wmi_cmd_buf),
				WMI_CMD_HDR, COMMANDID);

		dma_addr = QDF_NBUF_CB_PADDR(wmi_cmd_buf);
		phy_addr = qdf_mem_virt_to_phys(qdf_nbuf_data(wmi_cmd_buf));

		qdf_spin_lock_bh(&log_info->wmi_record_lock);
		/* Record 16 bytes of WMI cmd tx complete data
		 * - exclude TLV and WMI headers
		 */
		offset_ptr = buf_ptr + wmi_handle->soc->buf_offset_command;
		if (wmi_handle->ops->is_management_record(cmd_id)) {
			WMI_MGMT_COMMAND_TX_CMP_RECORD(wmi_handle, cmd_id,
						       offset_ptr);
		} else {
			WMI_COMMAND_TX_CMP_RECORD(wmi_handle, cmd_id,
						  offset_ptr, dma_addr,
						  phy_addr);
		}

		qdf_spin_unlock_bh(&log_info->wmi_record_lock);
	}
#endif

	wmi_interface_sequence_check(wmi_handle, wmi_cmd_buf);

	len = qdf_nbuf_len(wmi_cmd_buf);
	qdf_mem_zero(buf_ptr, len);
	wmi_buf_free(wmi_cmd_buf);
	qdf_mem_free(htc_pkt);
	qdf_atomic_dec(&wmi_handle->pending_cmds);
}

#ifdef FEATURE_RUNTIME_PM
/**
 * wmi_htc_log_pkt() - Print information of WMI command from HTC packet
 *
 * @ctx: handle of WMI context
 * @htc_pkt: handle of HTC packet
 *
 * @Return: none
 */
static void wmi_htc_log_pkt(void *ctx, HTC_PACKET *htc_pkt)
{
	wmi_buf_t wmi_cmd_buf = GET_HTC_PACKET_NET_BUF_CONTEXT(htc_pkt);
	uint32_t cmd_id;

	ASSERT(wmi_cmd_buf);
	cmd_id = WMI_GET_FIELD(qdf_nbuf_data(wmi_cmd_buf), WMI_CMD_HDR,
			       COMMANDID);

	wmi_debug("WMI command from HTC packet: %s, ID: %d",
		 wmi_id_to_name(cmd_id), cmd_id);
}
#else
static void wmi_htc_log_pkt(void *ctx, HTC_PACKET *htc_pkt)
{
}
#endif

/**
 * wmi_connect_pdev_htc_service() -  WMI API to get connect to HTC service
 *
 * @wmi_handle: handle to WMI.
 * @pdev_idx: Pdev index
 *
 * @Return: QDF_STATUS
 */
static QDF_STATUS wmi_connect_pdev_htc_service(struct wmi_soc *soc,
					       uint32_t pdev_idx)
{
	QDF_STATUS status;
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
	connect.EpCallbacks.ep_log_pkt = wmi_htc_log_pkt;

	/* connect to control service */
	connect.service_id = soc->svc_ids[pdev_idx];
	status = htc_connect_service(soc->htc_handle, &connect, &response);

	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to connect to WMI CONTROL service status:%d",
			 status);
		return status;
	}

	if (soc->is_async_ep)
		htc_set_async_ep(soc->htc_handle, response.Endpoint, true);

	soc->wmi_endpoint_id[pdev_idx] = response.Endpoint;
	soc->max_msg_len[pdev_idx] = response.MaxMsgLength;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wmi_unified_connect_htc_service(struct wmi_unified *wmi_handle,
				HTC_HANDLE htc_handle)
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

#ifdef WLAN_FEATURE_WMI_DIAG_OVER_CE7
/**
 * wmi_diag_connect_pdev_htc_service()
 * WMI DIAG API to get connect to HTC service
 *
 * @wmi_handle: handle to WMI.
 * @htc_handle: handle to HTC
 *
 * @Return: QDF_STATUS
 */
QDF_STATUS wmi_diag_connect_pdev_htc_service(struct wmi_unified *wmi_handle,
					     HTC_HANDLE htc_handle)
{
	QDF_STATUS status;
	struct htc_service_connect_resp response;
	struct htc_service_connect_req connect;

	OS_MEMZERO(&connect, sizeof(connect));
	OS_MEMZERO(&response, sizeof(response));

	/* meta data is unused for now */
	connect.pMetaData = NULL;
	connect.MetaDataLength = 0;
	connect.EpCallbacks.pContext = wmi_handle->soc;
	connect.EpCallbacks.EpTxCompleteMultiple = NULL;
	connect.EpCallbacks.EpRecv = wmi_control_diag_rx /* wmi diag rx */;
	connect.EpCallbacks.EpRecvRefill = NULL;
	connect.EpCallbacks.EpSendFull = NULL;
	connect.EpCallbacks.EpTxComplete = NULL;
	connect.EpCallbacks.ep_log_pkt = wmi_htc_log_pkt;

	/* connect to wmi diag service */
	connect.service_id = WMI_CONTROL_DIAG_SVC;
	status = htc_connect_service(htc_handle, &connect, &response);

	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("Failed to connect to WMI DIAG service status:%d\n",
			 status);
		return status;
	}

	if (wmi_handle->soc->is_async_ep)
		htc_set_async_ep(htc_handle, response.Endpoint, true);

	wmi_handle->soc->wmi_diag_endpoint_id = response.Endpoint;

	return QDF_STATUS_SUCCESS;
}
#endif

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
 * wmi_is_target_suspended() - WMI API to check target suspend state
 * @wmi_handle: handle to WMI.
 *
 * WMI API to check target suspend state
 *
 * Return: true if target is suspended, else false.
 */
bool wmi_is_target_suspended(struct wmi_unified *wmi_handle)
{
	return qdf_atomic_read(&wmi_handle->is_target_suspended);
}
qdf_export_symbol(wmi_is_target_suspended);

#ifdef WLAN_FEATURE_WMI_SEND_RECV_QMI
void wmi_set_qmi_stats(wmi_unified_t wmi_handle, bool val)
{
	wmi_handle->is_qmi_stats_enabled = val;
}

bool wmi_is_qmi_stats_enabled(struct wmi_unified *wmi_handle)
{
	return wmi_handle->is_qmi_stats_enabled;
}
#endif

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

/**
 * wmi_stop() - generic function to block unified WMI command
 * @wmi_handle: handle to WMI.
 *
 * @Return: success always.
 */
int
wmi_stop(wmi_unified_t wmi_handle)
{
	QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_INFO,
		  "WMI Stop");
	wmi_handle->wmi_stopinprogress = 1;
	return 0;
}

/**
 * wmi_start() - generic function to allow unified WMI command
 * @wmi_handle: handle to WMI.
 *
 * @Return: success always.
 */
int
wmi_start(wmi_unified_t wmi_handle)
{
	QDF_TRACE(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_INFO,
		  "WMI Start");
	wmi_handle->wmi_stopinprogress = 0;
	return 0;
}

/**
 * wmi_is_blocked() - generic function to check if WMI is blocked
 * @wmi_handle: handle to WMI.
 *
 * @Return: true, if blocked, false if not blocked
 */
bool
wmi_is_blocked(wmi_unified_t wmi_handle)
{
	return (!(!wmi_handle->wmi_stopinprogress));
}

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
 * wmi_pdev_id_conversion_enable() - API to enable pdev_id/phy_id conversion
 *                     in WMI. By default pdev_id conversion is not done in WMI.
 *                     This API can be used enable conversion in WMI.
 * @param wmi_handle   : handle to WMI
 * @param pdev_map     : pointer to pdev_map
 * @size               : size of pdev_id_map
 * Return none
 */
void wmi_pdev_id_conversion_enable(wmi_unified_t wmi_handle,
				   uint32_t *pdev_id_map,
				   uint8_t size)
{
	if (wmi_handle->target_type == WMI_TLV_TARGET)
		wmi_handle->ops->wmi_pdev_id_conversion_enable(wmi_handle,
							       pdev_id_map,
							       size);
}
