/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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
 * DOC: Roaming debug log operations declarations
 */
#ifndef _WLAN_ROAM_DEBUG_H_
#define _WLAN_ROAM_DEBUG_H_


#define roam_debug(args ...) \
		QDF_TRACE_DEBUG(QDF_MODULE_ID_ROAM_DEBUG, ## args)

/**
 * struct wlan_roam_debug_rec - roam debug information record definition
 * @time: timestamp when record was added
 * @operation: identifier for operation, command, event, etc.
 * @vdev_id: vdev identifier
 * @peer_id: peer_id. Range 0 - 255, 0xffff is invalid peer_id.
 * @mac_addr: mac address of peer
 * @peer_obj: pointer to peer object
 * @arg1: Optional argument #1
 * @arg2: Opttional argument #2
 */
struct wlan_roam_debug_rec {
	uint64_t time;
	uint8_t operation;
	uint8_t vdev_id;
	uint16_t peer_id;
	struct qdf_mac_addr mac_addr;
	void *peer_obj;
	uint32_t arg1;
	uint32_t arg2;
};

#ifndef WLAN_ROAM_DEBUG_MAX_REC
#define WLAN_ROAM_DEBUG_MAX_REC 256
#endif

/**
 * struct wlan_roam_debug_info - Buffer to store the wma debug records
 * @index: index of the most recent entry in the circular buffer
 * @num_max_rec: maximum records stored in the records array
 * @rec: array to store wma debug records, used in circular fashion
 */
struct wlan_roam_debug_info {
	qdf_atomic_t index;
	uint32_t num_max_rec;
	struct wlan_roam_debug_rec rec[WLAN_ROAM_DEBUG_MAX_REC];
};

/**
 * @DEBUG_PEER_CREATE_SEND: sent peer_create command to firmware
 * @DEBUG_PEER_CREATE_RESP: received peer create response
 * @DEBUG_PEER_DELETE_SEND: sent peer delete command to firmware
 * @DEBUG_PEER_DELETE_RESP: received peer delete response
 * @DEBUG_PEER_MAP_EVENT: received peer map event
 * @DEBUG_PEER_UNMAP_EVENT: received peer unmap event
 * @DEBUG_PEER_UNREF_DELETE: peer reference is decremented
 * @DEBUG_DELETING_PEER_OBJ: peer object is deleted
 * @DEBUG_ROAM_SYNCH_IND: received roam offload sync indication
 * @DEBUG_ROAM_SYNCH_CNF: sent roam offload sync confirmation
 * @DEBUG_ROAM_SYNCH_FAIL: received roam sync failure indication
 * @DEBUG_ROAM_EVENT: received roam event
 * @DEBUG_BUS_SUSPEND: host going into suspend mode
 * @DEBUG_BUS_RESUME: host operation resumed
 */

enum peer_debug_op {
	DEBUG_PEER_CREATE_SEND = 0,
	DEBUG_PEER_CREATE_RESP,
	DEBUG_PEER_DELETE_SEND,
	DEBUG_PEER_DELETE_RESP,
	DEBUG_PEER_MAP_EVENT,
	DEBUG_PEER_UNMAP_EVENT,
	DEBUG_PEER_UNREF_DELETE,
	DEBUG_DELETING_PEER_OBJ,
	DEBUG_ROAM_SYNCH_IND,
	DEBUG_ROAM_SYNCH_CNF,
	DEBUG_ROAM_SYNCH_FAIL,
	DEBUG_ROAM_EVENT,
	DEBUG_WOW_ROAM_EVENT,
	DEBUG_BUS_SUSPEND,
	DEBUG_BUS_RESUME,
	DEBUG_WOW_REASON,
};

#define DEBUG_INVALID_PEER_ID 0xffff
#define DEBUG_INVALID_VDEV_ID 0xff

#ifdef FEATURE_ROAM_DEBUG
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
			void *peer_obj, uint32_t arg1, uint32_t arg2);

/**
 * wlan_roam_debug_dump_table() - Print the roam debug log records
 * print all the valid debug records in the order of timestamp
 *
 * Return: none
 */
void wlan_roam_debug_dump_table(void);

#ifdef WLAN_LOGGING_BUFFERS_DYNAMICALLY
/**
 * wlan_roam_debug_init() - Allocate log buffer dynamically
 *
 * Return: none
 */
void wlan_roam_debug_init(void);
/**
 * wlan_roam_debug_deinit() - Free log buffer allocated dynamically
 *
 * Return: none
 */
void wlan_roam_debug_deinit(void);
#else /* WLAN_LOGGING_BUFFERS_DYNAMICALLY */
static inline void wlan_roam_debug_init(void)
{
}

static inline void wlan_roam_debug_deinit(void)
{
}
#endif /* WLAN_LOGGING_BUFFERS_DYNAMICALLY */

#else /* FEATURE_ROAM_DEBUG */
static inline void
wlan_roam_debug_log(uint8_t vdev_id, uint8_t op,
		    uint16_t peer_id, void *mac_addr,
		    void *peer_obj, uint32_t arg1, uint32_t arg2)
{
}

static inline void wlan_roam_debug_dump_table(void)
{
}

static inline void wlan_roam_debug_init(void)
{
}

static inline void wlan_roam_debug_deinit(void)
{
}
#endif /* FEATURE_ROAM_DEBUG */

#endif /* _WLAN_ROAM_DEBUG_H_ */
