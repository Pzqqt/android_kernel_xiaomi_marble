/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: This target interface shall be used by DP
 *      to communicate with target using WMI.
 */

#ifndef _WLAN_TARGET_IF_DP_H_
#define _WLAN_TARGET_IF_DP_H_

#include <qdf_types.h>
#include <qdf_status.h>
#include <wmi_unified_priv.h>
#include <wlan_objmgr_psoc_obj.h>
#include <target_if.h>
#include <cdp_txrx_ops.h>

/**
 * struct reorder_q_setup - reorder queue setup params
 * @pdev: pdev
 * @vdev_id: vdev id
 * @peer_macaddr: peer mac address
 * @hw_qdesc: hw queue descriptor
 * @tid: tid number
 * @queue_no: queue number
 * @ba_window_size_valid: BA window size validity flag
 * @ba_window_size: BA window size
 */
struct reorder_q_setup {
	struct cdp_ctrl_objmgr_pdev *pdev;
	uint8_t vdev_id;
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE];
	qdf_dma_addr_t hw_qdesc_paddr;
	uint8_t tid;
	uint16_t queue_no;
	uint8_t ba_window_size_valid;
	uint16_t ba_window_size;
};

/**
 * target_if_peer_set_default_routing() - set peer default routing
 * @pdev: pdev pointer
 * @peer_macaddr: peer mac address
 * @vdev_id: vdev id
 * @hash_based: hash based routing
 * @ring_num: ring number
 *
 * return: void
 */
void
target_if_peer_set_default_routing(struct cdp_ctrl_objmgr_pdev *pdev,
				   uint8_t *peer_macaddr, uint8_t vdev_id,
				   bool hash_based, uint8_t ring_num);

/**
 * target_if_peer_rx_reorder_queue_setup() - setup rx reorder queue
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @peer_macaddr: peer mac address
 * @hw_qdesc: hw queue descriptor
 * @tid: tid number
 * @queue_no: queue number
 * @ba_window_size_valid: BA window size validity flag
 * @ba_window_size: BA window size
 *
 * return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
target_if_peer_rx_reorder_queue_setup(struct cdp_ctrl_objmgr_pdev *pdev,
				      uint8_t vdev_id, uint8_t *peer_macaddr,
				      qdf_dma_addr_t hw_qdesc, int tid,
				      uint16_t queue_no,
				      uint8_t ba_window_size_valid,
				      uint16_t ba_window_size);

/**
 * target_if_peer_rx_reorder_queue_remove() - remove rx reorder queue
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @peer_macaddr: peer mac address
 * @peer_tid_bitmap: peer tid bitmap
 *
 * return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
target_if_peer_rx_reorder_queue_remove(struct cdp_ctrl_objmgr_pdev *pdev,
				       uint8_t vdev_id, uint8_t *peer_macaddr,
				       uint32_t peer_tid_bitmap);

/**
 * target_if_lro_hash_config() - send LRO hash config to FW
 * @psoc_handle: psoc handle pointer
 * @lro_hash_cfg: LRO hash config parameters
 *
 * return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
target_if_lro_hash_config(struct cdp_ctrl_objmgr_pdev *pdev,
			  struct cdp_lro_hash_config *lro_hash_cfg);

#endif
