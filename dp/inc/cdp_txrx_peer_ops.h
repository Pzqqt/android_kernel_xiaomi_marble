/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
 /**
 * @file cdp_txrx_peer.h
 * @brief Define the host data path peer API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_PEER_H_
#define _CDP_TXRX_PEER_H_

typedef QDF_STATUS (*ol_rx_callback_fp)(void *p_cds_gctx,
			 qdf_nbuf_t pDataBuff,
			 uint8_t ucSTAId);

/**
 * struct ol_txrx_peer_state - Peer state information
 */
enum ol_txrx_peer_state {
	OL_TXRX_PEER_STATE_INVALID,
	OL_TXRX_PEER_STATE_DISC,    /* initial state */
	OL_TXRX_PEER_STATE_CONN,    /* authentication in progress */
	OL_TXRX_PEER_STATE_AUTH,    /* authentication successful */
};

/**
 * struct ol_txrx_desc_type - txrx descriptor type
 * @sta_id: sta id
 * @is_qos_enabled: is station qos enabled
 * @is_wapi_supported: is station wapi supported
 */
struct ol_txrx_desc_type {
	uint8_t sta_id;
	uint8_t is_qos_enabled;
	uint8_t is_wapi_supported;
};

QDF_STATUS ol_txrx_register_peer(struct ol_txrx_desc_type *sta_desc);

/**
 * ol_txrx_vdev_peer_remove_cb - wma_remove_peer callback
 */
typedef void (*ol_txrx_vdev_peer_remove_cb)(void *handle, uint8_t *bssid,
					    uint8_t vdev_id,
					    ol_txrx_peer_handle peer,
					    bool roam_synch_in_progress);

QDF_STATUS ol_txrx_clear_peer(uint8_t sta_id);

QDF_STATUS ol_txrx_change_peer_state(uint8_t sta_id,
			 enum ol_txrx_peer_state sta_state,
			 bool roam_synch_in_progress);

ol_txrx_peer_handle ol_txrx_find_peer_by_addr(ol_txrx_pdev_handle pdev,
			 uint8_t *peer_addr,
			 uint8_t *peer_id);

ol_txrx_peer_handle
ol_txrx_find_peer_by_addr_and_vdev(ol_txrx_pdev_handle pdev,
			 ol_txrx_vdev_handle vdev,
			 uint8_t *peer_addr, uint8_t *peer_id);

#ifdef QCA_SUPPORT_TXRX_LOCAL_PEER_ID
uint16_t ol_txrx_local_peer_id(ol_txrx_peer_handle peer);
ol_txrx_peer_handle ol_txrx_find_peer_by_addr(ol_txrx_pdev_handle pdev,
					      uint8_t *peer_addr,
					      uint8_t *peer_id);
ol_txrx_peer_handle
ol_txrx_find_peer_by_addr_and_vdev(ol_txrx_pdev_handle pdev,
				   ol_txrx_vdev_handle vdev,
				   uint8_t *peer_addr, uint8_t *peer_id);
ol_txrx_peer_handle
ol_txrx_peer_find_by_local_id(ol_txrx_pdev_handle pdev, uint8_t local_peer_id);
#else
#define ol_txrx_local_peer_id(peer) OL_TXRX_INVALID_LOCAL_PEER_ID
#define ol_txrx_find_peer_by_addr(pdev, peer_addr, peer_id) NULL
#define ol_txrx_find_peer_by_addr_and_vdev(pdev, vdev, peer_addr, peer_id) NULL
#define ol_txrx_peer_find_by_local_id(pdev, local_peer_id) NULL
#endif /* QCA_SUPPORT_TXRX_LOCAL_PEER_ID */

QDF_STATUS
ol_txrx_peer_state_update(ol_txrx_pdev_handle pdev, uint8_t *peer_addr,
			 enum ol_txrx_peer_state state);

QDF_STATUS ol_txrx_get_vdevid(struct ol_txrx_peer_t *peer, uint8_t *vdev_id);
void *ol_txrx_get_vdev_by_sta_id(uint8_t sta_id);

QDF_STATUS ol_txrx_register_ocb_peer(void *cds_ctx, uint8_t *mac_addr,
			 uint8_t *peer_id);

/**
 * ol_txrx_peer_get_peer_mac_addr() - return mac_addr from peer handle.
 * @peer: handle to peer
 *
 * returns mac addrs for module which do not know peer type
 *
 * Return: the mac_addr from peer
 */
uint8_t *ol_txrx_peer_get_peer_mac_addr(ol_txrx_peer_handle peer);

/**
 * ol_txrx_get_peer_state() - Return peer state of peer
 * @peer: peer handle
 *
 * Return: return peer state
 */
int ol_txrx_get_peer_state(ol_txrx_peer_handle peer);

/**
 * ol_txrx_get_vdev_for_peer() - Return vdev from peer handle
 * @peer: peer handle
 *
 * Return: vdev handle from peer
 */
ol_txrx_vdev_handle
ol_txrx_get_vdev_for_peer(ol_txrx_peer_handle peer);

/**
 * ol_txrx_update_ibss_add_peer_num_of_vdev() - update and return peer num
 * @vdev: vdev handle
 * @peer_num_delta: peer nums to be adjusted
 *
 * Return: -1 for failure or total peer nums after adjustment.
 */
int16_t
ol_txrx_update_ibss_add_peer_num_of_vdev(ol_txrx_vdev_handle vdev,
					 int16_t peer_num_delta);
/**
 * ol_txrx_remove_peers_for_vdev() - remove all vdev peers with lock held
 * @vdev: vdev handle
 * @callback: callback function to remove the peer.
 * @callback_context: handle for callback function
 * @remove_last_peer: Does it required to last peer.
 *
 * Return: NONE
 */
void
ol_txrx_remove_peers_for_vdev(ol_txrx_vdev_handle vdev,
			      ol_txrx_vdev_peer_remove_cb callback,
			      void *callback_context, bool remove_last_peer);
/**
 * ol_txrx_remove_peers_for_vdev_no_lock() - remove vdev peers with no lock.
 * @vdev: vdev handle
 * @callback: callback function to remove the peer.
 * @callback_context: handle for callback function
 *
 * Return: NONE
 */
void
ol_txrx_remove_peers_for_vdev_no_lock(ol_txrx_vdev_handle vdev,
			      ol_txrx_vdev_peer_remove_cb callback,
			      void *callback_context);
#endif /* _CDP_TXRX_PEER_H_ */
