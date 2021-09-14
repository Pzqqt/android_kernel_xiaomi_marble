/*
 * Copyright (c) 2013-2018, 2020-2021 The Linux Foundation. All rights reserved.
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
 * DOC: Roaming debug log operations routines and global data
 */

#include <qdf_types.h>
#include <qdf_atomic.h>
#include <qdf_mem.h>
#include <qdf_time.h>
#include <qdf_trace.h>
#include <qdf_module.h>
#include <wlan_cmn.h>
#include "wlan_roam_debug.h"

#ifdef FEATURE_ROAM_DEBUG
static void wlan_roam_rec_print(struct wlan_roam_debug_rec *dbg_rec,
				uint32_t idx, uint32_t delta,
				bool to_kernel);

static void wlan_conn_rec_print(struct wlan_roam_debug_rec *dbg_rec,
				uint32_t idx, uint32_t delta,
				bool to_kernel);

#ifdef WLAN_LOGGING_BUFFERS_DYNAMICALLY
static struct wlan_roam_debug_info *global_wlan_roam_debug_table;

/**
 * wlan_roam_debug_init() - Allocate log buffer dynamically
 *
 * Return: none
 */
void wlan_roam_debug_init(void)
{
	uint8_t i;
	global_wlan_roam_debug_table = qdf_mem_valloc(
				sizeof(struct wlan_roam_debug_info) * REC_MAX);

	QDF_BUG(global_wlan_roam_debug_table);

	if (global_wlan_roam_debug_table) {
		for (i = 0; i < REC_MAX; i++) {
			qdf_atomic_init(&global_wlan_roam_debug_table[i].index);
			global_wlan_roam_debug_table[i].num_max_rec =
						WLAN_ROAM_DEBUG_MAX_REC;
			if (i == REC_ROAM)
				global_wlan_roam_debug_table[i].rec_print =
					wlan_roam_rec_print;
			else
				global_wlan_roam_debug_table[i].rec_print =
					wlan_conn_rec_print;
		}
	}
}

qdf_export_symbol(wlan_roam_debug_init);

static inline struct wlan_roam_debug_info *wlan_roam_debug_get_table(
	wlan_rec_type type)
{
	if (type >= REC_MAX)
		return NULL;
	return &global_wlan_roam_debug_table[type];
}

/**
 * wlan_roam_debug_deinit() - Free log buffer allocated dynamically
 *
 * Return: none
 */
void wlan_roam_debug_deinit(void)
{
	qdf_mem_vfree(global_wlan_roam_debug_table);
	global_wlan_roam_debug_table = NULL;
}

qdf_export_symbol(wlan_roam_debug_deinit);
#else /* WLAN_LOGGING_BUFFERS_DYNAMICALLY */
/*
 * wlan roam debug log is stored in this global structure. It can be accessed
 * without requiring any psoc or vdev context. It will be accessible in
 * the crash dump without having to dereference complex stack traces.
 */
static struct wlan_roam_debug_info global_wlan_roam_debug_table[REC_MAX] = {
	[REC_ROAM] = {{ 0 },
		      .num_max_rec = WLAN_ROAM_DEBUG_MAX_REC,
		      .rec_print = wlan_roam_rec_print},
	[REC_CONN] = {{ 0 },
		      .num_max_rec = WLAN_ROAM_DEBUG_MAX_REC,
		      .rec_print = wlan_conn_rec_print},
};

static inline struct wlan_roam_debug_info *wlan_roam_debug_get_table(
	wlan_rec_type type)
{
	if (type >= REC_MAX)
		return NULL;
	return &global_wlan_roam_debug_table[type];
}
#endif /* WLAN_LOGGING_BUFFERS_DYNAMICALLY */

/**
 * wlan_roam_next_debug_log_index() - atomically increment and wrap around index
 * @index: address of index to increment
 * @size: wrap around this value
 *
 * Return: new value of index
 */
static int wlan_roam_next_debug_log_index(qdf_atomic_t *index, int size)
{
	int i = qdf_atomic_inc_return(index);

	if (i == WLAN_ROAM_DEBUG_MAX_REC)
		qdf_atomic_sub(WLAN_ROAM_DEBUG_MAX_REC, index);
	while (i >= size)
		i -= WLAN_ROAM_DEBUG_MAX_REC;

	return i;
}

/**
 * wlan_roam_debug_log() - Add a debug log entry to wlan roam debug records
 * @vdev_id: vdev identifier
 * @op: operation identifier
 * @peer_id: peer id
 * @mac_addr: mac address of peer, can be NULL
 * @peer_obj: peer object address, can be NULL
 * @arg1: extra argument #1
 * @arg2: extra argument #2
 *
 * Return: none
 */
void wlan_roam_debug_log(uint8_t vdev_id, uint8_t op,
			uint16_t peer_id, void *mac_addr,
			void *peer_obj, uint32_t arg1, uint32_t arg2)
{
	wlan_rec_debug_log(REC_ROAM, vdev_id, op, peer_id, mac_addr,
			   peer_obj, arg1, arg2);
}

qdf_export_symbol(wlan_roam_debug_log);

void wlan_rec_debug_log(wlan_rec_type rec_type, uint8_t vdev_id, uint8_t op,
			uint16_t peer_id, const void *mac_addr,
			void *peer_obj, uint32_t arg1, uint32_t arg2)
{
	uint32_t i;
	struct wlan_roam_debug_info *dbg_tbl;
	struct wlan_roam_debug_rec *rec;

	dbg_tbl = wlan_roam_debug_get_table(rec_type);
	if (!dbg_tbl)
		return;

	i = wlan_roam_next_debug_log_index(
				    &dbg_tbl->index,
				    WLAN_ROAM_DEBUG_MAX_REC);
	rec = &dbg_tbl->rec[i];
	rec->time = qdf_get_log_timestamp();
	rec->operation = op;
	rec->vdev_id = vdev_id;
	rec->peer_id = peer_id;
	if (mac_addr)
		qdf_mem_copy(rec->mac_addr.bytes, mac_addr,
			     QDF_MAC_ADDR_SIZE);
	else
		qdf_mem_zero(rec->mac_addr.bytes,
			     QDF_MAC_ADDR_SIZE);
	rec->peer_obj = peer_obj;
	rec->arg1 = arg1;
	rec->arg2 = arg2;
}

qdf_export_symbol(wlan_rec_debug_log);

/**
 * wlan_roam_debug_string() - convert operation value to printable string
 * @op: operation identifier
 *
 * Return: printable string for the operation
 */
static char *wlan_roam_debug_string(uint32_t op)
{
	switch (op) {
	case DEBUG_PEER_CREATE_SEND:
		return "peer create send";
	case DEBUG_PEER_CREATE_RESP:
		return "peer create resp_event";
	case DEBUG_PEER_DELETE_SEND:
		return "peer delete send";
	case DEBUG_PEER_DELETE_RESP:
		return "peer delete resp_event";
	case DEBUG_PEER_MAP_EVENT:
		return "peer map event";
	case DEBUG_PEER_UNMAP_EVENT:
		return "peer unmap event";
	case DEBUG_PEER_UNREF_DELETE:
		return "peer unref delete";
	case DEBUG_DELETING_PEER_OBJ:
		return "peer obj deleted";
	case DEBUG_ROAM_SYNCH_IND:
		return "roam synch ind event";
	case DEBUG_ROAM_SYNCH_CNF:
		return "roam sync conf sent";
	case DEBUG_ROAM_SYNCH_FAIL:
		return "roam sync fail event";
	case DEBUG_ROAM_EVENT:
		return "roam event";
	case DEBUG_WOW_ROAM_EVENT:
		return "wow wakeup roam event";
	case DEBUG_BUS_SUSPEND:
		return "host suspend";
	case DEBUG_BUS_RESUME:
		return "host wakeup";
	case DEBUG_WOW_REASON:
		return "wow wakeup reason";
	case DEBUG_CONN_CONNECTING:
		return "conn";
	case DEBUG_CONN_ASSOCIATION:
		return "assoc";
	case DEBUG_CONN_CONNECT_RESULT:
		return "cnrlt";
	case DEBUG_CONN_ROAMING:
		return "roaming";
	case DEBUG_CONN_ROAMED:
		return "roamed";
	case DEBUG_CONN_ROAMED_IND:
		return "rmind";
	case DEBUG_CONN_DISCONNECT:
		return "disc";
	case DEBUG_CONN_DISCONNECT_HANDLER:
		return "dishdr";
	case DEBUG_CONN_DISCONNECT_IND:
		return "disind";
	default:
		return "unknown";
	}
}

void wlan_roam_rec_print(struct wlan_roam_debug_rec *dbg_rec,
			 uint32_t idx, uint32_t delta,
			 bool to_kernel)
{
	roam_debug("index = %5d timestamp = 0x%016llx delta ms = %-12u",
		   idx, dbg_rec->time, delta);
	roam_debug("info = %-24s vdev_id = %-3d mac addr = "QDF_MAC_ADDR_FMT,
		   wlan_roam_debug_string(dbg_rec->operation),
		   (int8_t)dbg_rec->vdev_id,
		   QDF_MAC_ADDR_REF(dbg_rec->mac_addr.bytes));
	roam_debug("peer obj = 0x%pK peer_id = %-4d", dbg_rec->peer_obj,
		   (int8_t)dbg_rec->peer_id);
	roam_debug("arg1 = 0x%-8x arg2 = 0x%-8x", dbg_rec->arg1,
		   dbg_rec->arg2);
}

void wlan_conn_rec_print(struct wlan_roam_debug_rec *dbg_rec,
			 uint32_t idx, uint32_t delta,
			 bool to_kernel)
{
	if (to_kernel) {
		roam_info("i %d ti 0x%08llx ms %u vdv %d %s a1 0x%x a2 0x%x "QDF_MAC_ADDR_FMT,
			  idx, dbg_rec->time, delta, (int8_t)dbg_rec->vdev_id,
			  wlan_roam_debug_string(dbg_rec->operation),
			  dbg_rec->arg1, dbg_rec->arg2,
			  QDF_MAC_ADDR_REF(dbg_rec->mac_addr.bytes));
	} else {
		roam_debug("i %d ti 0x%08llx ms %u vdv %d %s a1 0x%x a2 0x%x "QDF_MAC_ADDR_FMT,
			   idx, dbg_rec->time, delta, (int8_t)dbg_rec->vdev_id,
			   wlan_roam_debug_string(dbg_rec->operation),
			   dbg_rec->arg1, dbg_rec->arg2,
			   QDF_MAC_ADDR_REF(dbg_rec->mac_addr.bytes));
	}
}

/**
 * wlan_rec_debug_dump_table() - Print the wlan roam debug log records
 * print all the valid debug records in the order of timestamp
 *
 * Return: none
 */
void wlan_rec_debug_dump_table(wlan_rec_type rec_type, uint32_t count,
			       bool to_kernel)
{
	uint32_t i;
	int32_t current_index;
	struct wlan_roam_debug_info *dbg_tbl;
	struct wlan_roam_debug_rec *dbg_rec;
	uint64_t startt = 0;
	uint32_t delta;

#define DEBUG_CLOCK_TICKS_PER_MSEC 19200
	if (count > WLAN_ROAM_DEBUG_MAX_REC)
		count = WLAN_ROAM_DEBUG_MAX_REC;
	dbg_tbl = wlan_roam_debug_get_table(rec_type);
	if (!dbg_tbl)
		return;

	current_index = qdf_atomic_read(&dbg_tbl->index);
	if (current_index < 0) {
		roam_debug("No records to dump");
		return;
	}
	roam_debug("dump %d rec type %d idx %d", count, rec_type,
		   current_index);

	i = (current_index + WLAN_ROAM_DEBUG_MAX_REC - count) %
		WLAN_ROAM_DEBUG_MAX_REC;
	do {
		/* wrap around */
		i = (i + 1) % WLAN_ROAM_DEBUG_MAX_REC;
		dbg_rec = &dbg_tbl->rec[i];
		/* skip unused entry */
		if (dbg_rec->time == 0)
			continue;
		if (count == 0)
			break;
		count--;

		if (startt == 0)
			startt = dbg_rec->time;
		/*
		 * Divide by 19200 == right shift 8 bits, then divide by 75
		 * 32 bit computation keeps both 32 and 64 bit compilers happy.
		 * The value will roll over after approx. 33554 seconds.
		 */
		delta = (uint32_t) (((dbg_rec->time - startt) >> 8) &
				    0xffffffff);
		delta = delta / (DEBUG_CLOCK_TICKS_PER_MSEC >> 8);

		if (dbg_tbl->rec_print)
			dbg_tbl->rec_print(dbg_rec, i, delta, to_kernel);
	} while (i != current_index);
}

qdf_export_symbol(wlan_rec_debug_dump_table);

/**
 * wlan_roam_debug_dump_table() - Print the wlan roam debug log records
 * print all the valid debug records in the order of timestamp
 *
 * Return: none
 */
void wlan_roam_debug_dump_table(void)
{
	wlan_rec_debug_dump_table(REC_ROAM, WLAN_ROAM_DEBUG_MAX_REC, false);
}
qdf_export_symbol(wlan_roam_debug_dump_table);

#endif /* FEATURE_ROAM_DEBUG */
