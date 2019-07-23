/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_MGMT_TXRX_MAIN_I_H_
#define _WLAN_MGMT_TXRX_MAIN_I_H_

/**
 * DOC:  wlan_mgmt_txrx_main_i.h
 *
 * management tx/rx layer private API and structures
 *
 */

#include "wlan_mgmt_txrx_utils_api.h"
#include "wlan_objmgr_cmn.h"
#include "qdf_list.h"


#define IEEE80211_FC0_TYPE_MASK             0x0c
#define IEEE80211_FC0_SUBTYPE_MASK          0xf0
#define IEEE80211_FC0_TYPE_MGT              0x00

/**
 * mgmt_wakelock_reason - reasons mgmt_txrx might hold a wakelock
 * @MGMT_TXRX_WAKELOCK_REASON_TX_CMP - wait for mgmt_tx_complete event
 */
enum mgmt_txrx_wakelock_reason {
	MGMT_TXRX_WAKELOCK_REASON_TX_CMP
};

/* timeout to wait for management_tx_complete event from firmware */
#define MGMT_TXRX_WAKELOCK_TIMEOUT_TX_CMP 300

/*
 * generic definitions for IEEE 802.11 frames
 */
struct ieee80211_frame {
	uint8_t i_fc[2];
	uint8_t i_dur[2];
	union {
		struct {
			uint8_t i_addr1[QDF_MAC_ADDR_SIZE];
			uint8_t i_addr2[QDF_MAC_ADDR_SIZE];
			uint8_t i_addr3[QDF_MAC_ADDR_SIZE];
		};
		uint8_t i_addr_all[3 * QDF_MAC_ADDR_SIZE];
	};
	uint8_t i_seq[2];
	/* possibly followed by addr4[QDF_MAC_ADDR_SIZE]; */
	/* see below */
} __packed;


/**
 * struct mgmt_txrx_desc_elem_t - element in mgmt desc pool linked list
 * @entry:             list entry
 * @tx_dwnld_cmpl_cb:  dma completion callback function pointer
 * @tx_ota_cmpl_cb:    ota completion callback function pointer
 * @nbuf:              frame  buffer
 * @desc_id:           descriptor id
 * @peer:              peer who wants to send this frame
 * @context:           caller component specific context
 * @vdev_id:           vdev id
 * @in_use:            flag to denote whether desc is in use
 */
struct mgmt_txrx_desc_elem_t {
	qdf_list_node_t entry;
	mgmt_tx_download_comp_cb tx_dwnld_cmpl_cb;
	mgmt_ota_comp_cb  tx_ota_cmpl_cb;
	qdf_nbuf_t nbuf;
	uint32_t desc_id;
	struct wlan_objmgr_peer *peer;
	void *context;
	uint8_t vdev_id;
	bool in_use;
};

/**
 * struct mgmt_desc_pool_t - linked list mgmt desc pool
 * @free_list:            linked list of free descriptors
 * @pool:                 pool of descriptors in use
 * @desc_pool_lock:       mgmt. descriptor free pool spinlock
 */
struct mgmt_desc_pool_t {
	qdf_list_t free_list;
	struct mgmt_txrx_desc_elem_t *pool;
	qdf_spinlock_t desc_pool_lock;
};

/**
 * struct mgmt_rx_handler - structure for storing rx cb
 * @comp_id:      component id
 * @rx_cb:        rx callback for the mgmt. frame
 * @next:         pointer to next rx cb structure
 */
struct mgmt_rx_handler {
	enum wlan_umac_comp_id comp_id;
	mgmt_frame_rx_callback rx_cb;
	struct mgmt_rx_handler *next;
};

/**
 * struct txrx_stats - txrx stats for mgmt frames
 * @pkts_success:       no. of packets successfully txed/rcvd
 * @pkts_fail:          no. of packets unsuccessfully txed/rcvd
 * @bytes_success:      no. of bytes successfully txed/rcvd
 * @bytes_fail:         no. of bytes successfully txed/rcvd
 * @assoc_req_rcvd:     no. of assoc requests rcvd
 * @assoc_rsp_rcvd:     no. of assoc responses rcvd
 * @reassoc_req_rcvd:   no. of reassoc requests rcvd
 * @reassoc_rsp_rcvd:   no. of reassoc responses rcvd
 * @probe_req_rcvd:     no. of probe requests rcvd
 * @prob_resp_rcvd:     no. of probe responses rcvd
 * @beacon_rcvd:        no. of beacons rcvd
 * @atim_rcvd:          no. of ATIMs rcvd
 * @disassoc_rcvd:      no. of disassocs rcvd
 * @auth_rcvd:          no. of auths rcvd
 * @deauth_rcvd:        no. of deauths rcvd
 * @action_rcvd:        no. of action frames rcvd
 * @action_no_ack_rcvd: no. of action frames with no ack rcvd
 */
struct txrx_stats {
	uint64_t pkts_success;
	uint64_t pkts_fail;
	uint64_t bytes_success;
	uint64_t bytes_fail;
	uint64_t assoc_req_rcvd;
	uint64_t assoc_rsp_rcvd;
	uint64_t reassoc_req_rcvd;
	uint64_t reassoc_rsp_rcvd;
	uint64_t probe_req_rcvd;
	uint64_t prob_resp_rcvd;
	uint64_t beacon_rcvd;
	uint64_t atim_rcvd;
	uint64_t disassoc_rcvd;
	uint64_t auth_rcvd;
	uint64_t deauth_rcvd;
	uint64_t action_rcvd;
	uint64_t action_no_ack_rcvd;
};

/**
 * struct mgmt_txrx_stats_t - mgmt txrx stats
 * @mgmt_tx_stats:      mgmt tx stats
 * @mgmt_rx_stats:      mgmt rx stats
 * @ota_comp:           no. of ota completions rcvd
 * @dma_comp:           no. of dma completions rcvd
 */
struct mgmt_txrx_stats_t {
	struct txrx_stats mgmt_tx_stats;
	struct txrx_stats mgmt_rx_stats;
	uint64_t ota_comp;
	uint64_t dma_comp;
};

/**
 * struct mgmt_txrx_priv_psoc_context - mgmt txrx private psoc context
 * @psoc:                psoc context
 * @mgmt_rx_comp_cb:     array of pointers of mgmt rx cbs
 * @mgmt_txrx_psoc_ctx_lock:  mgmt txrx psoc ctx lock
 */
struct mgmt_txrx_priv_psoc_context {
	struct wlan_objmgr_psoc *psoc;
	struct mgmt_rx_handler *mgmt_rx_comp_cb[MGMT_MAX_FRAME_TYPE];
	qdf_spinlock_t mgmt_txrx_psoc_ctx_lock;
};

/**
 * struct mgmt_txrx_priv_context_dev - mgmt txrx private context
 * @pdev:     pdev context
 * @mgmt_desc_pool:   pointer to mgmt desc. pool
 * @mgmt_txrx_stats:  pointer to mgmt txrx stats
 * @wakelock_tx_cmp:  mgmt tx complete wake lock
 * @wakelock_tx_runtime_cmp: mgmt tx runtime complete wake lock
 */
struct mgmt_txrx_priv_pdev_context {
	struct wlan_objmgr_pdev *pdev;
	struct mgmt_desc_pool_t mgmt_desc_pool;
	struct mgmt_txrx_stats_t *mgmt_txrx_stats;
	qdf_wake_lock_t wakelock_tx_cmp;
	qdf_runtime_lock_t wakelock_tx_runtime_cmp;
};


/**
 * wlan_mgmt_txrx_desc_pool_init() - initializes mgmt. desc. pool
 * @mgmt_txrx_pdev_ctx: mgmt txrx pdev context
 *
 * This function initializes the mgmt descriptor pool.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_mgmt_txrx_desc_pool_init(
			struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx);

/**
 * wlan_mgmt_txrx_desc_pool_deinit() - deinitializes mgmt. desc. pool
 * @mgmt_txrx_pdev_ctx: mgmt txrx pdev context
 *
 * This function deinitializes the mgmt descriptor pool.
 *
 * Return: void
 */
void wlan_mgmt_txrx_desc_pool_deinit(
			struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx);

/**
 * wlan_mgmt_txrx_desc_get() - gets mgmt. descriptor from freelist
 * @mgmt_txrx_pdev_ctx: mgmt txrx pdev context
 *
 * This function retrieves the mgmt. descriptor for mgmt. tx frames
 * from the mgmt. descriptor freelist.
 *
 * Return: mgmt. descriptor retrieved.
 */
struct mgmt_txrx_desc_elem_t *wlan_mgmt_txrx_desc_get(
			struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx);

/**
 * wlan_mgmt_txrx_desc_put() - puts mgmt. descriptor back in freelist
 * @mgmt_txrx_pdev_ctx: mgmt txrx pdev context
 * @desc_id: mgmt txrx descriptor id
 *
 * This function puts the mgmt. descriptor back in to the freelist.
 *
 * Return: void
 */
void wlan_mgmt_txrx_desc_put(
			struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx,
			uint32_t desc_id);

#endif
