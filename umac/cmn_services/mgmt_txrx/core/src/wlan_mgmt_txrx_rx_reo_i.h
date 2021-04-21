/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 *  DOC: wlan_mgmt_txrx_rx_reo_i.h
 *  This file contains mgmt rx re-ordering related APIs
 */

#ifndef _WLAN_MGMT_TXRX_RX_REO_I_H
#define _WLAN_MGMT_TXRX_RX_REO_I_H

#include <qdf_list.h>
#include <qdf_timer.h>
#include <qdf_lock.h>
#include <qdf_nbuf.h>
#include <wlan_mgmt_txrx_utils_api.h>
#include <wlan_mgmt_txrx_rx_reo_public_structs.h>

/**
 * struct mgmt_rx_reo_list – Linked list used to reorder the management frames
 * received. Each list entry would correspond to a management frame. List
 * entries would be sorted in the same order in which they are received by MAC
 * HW.
 * @list: List used for reordering
 * @list_lock: Lock to protect the list
 * @num_entries: Number of entries in the list
 * @ageout_timer: Periodic timer to age-out the list entries
 */
struct mgmt_rx_reo_list {
	qdf_list_t list;
	qdf_spinlock_t list_lock;
	uint32_t num_entries;
	qdf_timer_t ageout_timer;
};

/**
 * struct mgmt_rx_reo_list_entry - Entry in the Management reorder list
 * @node: list node
 * @wbuf: nbuf corresponding to this frame.
 * @rx_params: Management rx event parameters
 * @insertion_timestamp: Host time stamp when this entry is inserted to
 * the list.
 */
struct mgmt_rx_reo_list_entry {
	qdf_list_node_t node;
	qdf_nbuf_t wbuf;
	struct mgmt_rx_event_params rx_params;
	uint32_t insertion_timestamp;
};


struct mgmt_rx_reo_snapshot;
/*
 * struct mgmt_rx_reo_pdev_info - Pdev information required by the Management
 * Rx REO module
 * @host_or_fw_consumed_snapshot: Snapshot of latest frame consumed by Host/FW
 * @mac_hw_snapshot: Pointer to the snapshot of latest frame seen at the MAC HW
 * @fw_consumed_snapshot: Pointer to the snapshot of latest frame consumed
 * by the FW
 * @fw_forwarded_snapshot: Pointer to the snapshot of latest frame forwarded
 * to the Host by FW
 */
struct mgmt_rx_reo_pdev_info {
	struct mgmt_rx_reo_snapshot_simplified host_or_fw_consumed_snapshot;
	struct mgmt_rx_reo_snapshot *mac_hw_snapshot;
	struct mgmt_rx_reo_snapshot *fw_consumed_snapshot;
	struct mgmt_rx_reo_snapshot *fw_forwarded_snapshot;
};
/**
 * struct mgmt_rx_reo_context - This structure holds the info required for
 * management rx-reordering. Reordering is done across all the psocs.
 * So there should be only one instance of this structure defined.
 * @reo_list: Linked list used for reordering
 * @global_ts_last_delivered_frame: HW time stamp of the last frame
 * delivered to the upper layer
 */
struct mgmt_rx_reo_context {
	struct mgmt_rx_reo_list reo_list;
	uint32_t global_ts_last_delivered_frame;
};
#endif /* _WLAN_MGMT_TXRX_RX_REO_I_H */
