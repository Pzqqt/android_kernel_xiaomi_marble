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
 * @file cdp_txrx_ctrl.h
 * @brief Define the host data path control API functions
 * called by the host control SW and the OS interface module
 */

#ifndef _CDP_TXRX_CTRL_H_
#define _CDP_TXRX_CTRL_H_
/* TODO: adf need to be replaced with qdf */
/*
 * Cleanups --  Might need cleanup
 */
#if !QCA_OL_TX_PDEV_LOCK && QCA_NSS_PLATFORM || \
	(defined QCA_PARTNER_PLATFORM && QCA_PARTNER_SUPPORT_FAST_TX)
#define VAP_TX_SPIN_LOCK(_x) spin_lock(_x)
#define VAP_TX_SPIN_UNLOCK(_x) spin_unlock(_x)
#else /* QCA_OL_TX_PDEV_LOCK */
#define VAP_TX_SPIN_LOCK(_x)
#define VAP_TX_SPIN_UNLOCK(_x)
#endif /* QCA_OL_TX_PDEV_LOCK */

#if QCA_OL_TX_PDEV_LOCK
void ol_ll_pdev_tx_lock(void *);
void ol_ll_pdev_tx_unlock(void *);
#define OL_TX_LOCK(_x)  ol_ll_pdev_tx_lock(_x)
#define OL_TX_UNLOCK(_x) ol_ll_pdev_tx_unlock(_x)

#define OL_TX_PDEV_LOCK(_x)  qdf_spin_lock_bh(_x)
#define OL_TX_PDEV_UNLOCK(_x) qdf_spin_unlock_bh(_x)
#else
#define OL_TX_PDEV_LOCK(_x)
#define OL_TX_PDEV_UNLOCK(_x)

#define OL_TX_LOCK(_x)
#define OL_TX_UNLOCK(_x)
#endif /* QCA_OL_TX_PDEV_LOCK */

#if !QCA_OL_TX_PDEV_LOCK
#define OL_TX_FLOW_CTRL_LOCK(_x)  qdf_spin_lock_bh(_x)
#define OL_TX_FLOW_CTRL_UNLOCK(_x) qdf_spin_unlock_bh(_x)

#define OL_TX_DESC_LOCK(_x)  qdf_spin_lock_bh(_x)
#define OL_TX_DESC_UNLOCK(_x) qdf_spin_unlock_bh(_x)

#define OSIF_VAP_TX_LOCK(_x)  spin_lock(&((_x)->tx_lock))
#define OSIF_VAP_TX_UNLOCK(_x)  spin_unlock(&((_x)->tx_lock))

#define OL_TX_PEER_LOCK(_x, _id) qdf_spin_lock_bh(&((_x)->peer_lock[_id]))
#define OL_TX_PEER_UNLOCK(_x, _id) qdf_spin_unlock_bh(&((_x)->peer_lock[_id]))

#define OL_TX_PEER_UPDATE_LOCK(_x, _id) \
	qdf_spin_lock_bh(&((_x)->peer_lock[_id]))
#define OL_TX_PEER_UPDATE_UNLOCK(_x, _id) \
	qdf_spin_unlock_bh(&((_x)->peer_lock[_id]))

#else
#define OSIF_VAP_TX_LOCK(_x)  ol_ll_pdev_tx_lock((_x)->iv_txrx_handle)
#define OSIF_VAP_TX_UNLOCK(_x) ol_ll_pdev_tx_unlock((_x)->iv_txrx_handle)

#define OL_TX_FLOW_CTRL_LOCK(_x)
#define OL_TX_FLOW_CTRL_UNLOCK(_x)

#define OL_TX_DESC_LOCK(_x)
#define OL_TX_DESC_UNLOCK(_x)

#define OL_TX_PEER_LOCK(_x, _id)
#define OL_TX_PEER_UNLOCK(_x, _id)

#define OL_TX_PEER_UPDATE_LOCK(_x, _id) qdf_spin_lock_bh(&((_x)->tx_lock))
#define OL_TX_PEER_UPDATE_UNLOCK(_x, _id) qdf_spin_unlock_bh(&((_x)->tx_lock))

#endif /* !QCA_OL_TX_PDEV_LOCK */


extern int ol_txrx_is_target_ar900b(ol_txrx_vdev_handle vdev);
#define OL_TXRX_IS_TARGET_AR900B(vdev)  ol_txrx_is_target_ar900b(vdev)


/* WIN */
int
ol_txrx_mempools_attach(ol_pdev_handle ctrl_pdev);
int
ol_txrx_set_filter_neighbour_peers(
	ol_txrx_pdev_handle pdev,
	u_int32_t val);
/**
 * @brief set the safemode of the device
 * @details
 *  This flag is used to bypass the encrypt and decrypt processes when send and
 *  receive packets. It works like open AUTH mode, HW will treate all packets
 *  as non-encrypt frames because no key installed. For rx fragmented frames,
 *  it bypasses all the rx defragmentaion.
 *
 * @param vdev - the data virtual device object
 * @param val - the safemode state
 * @return - void
 */

void
ol_txrx_set_safemode(
	ol_txrx_vdev_handle vdev,
	u_int32_t val);
/**
 * @brief configure the drop unencrypted frame flag
 * @details
 *  Rx related. When set this flag, all the unencrypted frames
 *  received over a secure connection will be discarded
 *
 * @param vdev - the data virtual device object
 * @param val - flag
 * @return - void
 */
void
ol_txrx_set_drop_unenc(
	ol_txrx_vdev_handle vdev,
	u_int32_t val);


/**
 * @brief set the Tx encapsulation type of the VDEV
 * @details
 *  This will be used to populate the HTT desc packet type field during Tx
 *
 * @param vdev - the data virtual device object
 * @param val - the Tx encap type
 * @return - void
 */
void
ol_txrx_set_tx_encap_type(
	ol_txrx_vdev_handle vdev,
	enum htt_pkt_type val);

/**
 * @brief set the Rx decapsulation type of the VDEV
 * @details
 *  This will be used to configure into firmware and hardware which format to
 *  decap all Rx packets into, for all peers under the VDEV.
 *
 * @param vdev - the data virtual device object
 * @param val - the Rx decap mode
 * @return - void
 */
void
ol_txrx_set_vdev_rx_decap_type(
	ol_txrx_vdev_handle vdev,
	enum htt_pkt_type val);

/**
 * @brief get the Rx decapsulation type of the VDEV
 *
 * @param vdev - the data virtual device object
 * @return - the Rx decap type
 */
enum htt_pkt_type
ol_txrx_get_vdev_rx_decap_type(ol_txrx_vdev_handle vdev);

/* Is this similar to ol_txrx_peer_state_update() in MCL */
/**
 * @brief Update the authorize peer object at association time
 * @details
 *  For the host-based implementation of rate-control, it
 *  updates the peer/node-related parameters within rate-control
 *  context of the peer at association.
 *
 * @param peer - pointer to the node's object
 * @authorize - either to authorize or unauthorize peer
 *
 * @return none
 */
void
ol_txrx_peer_authorize(struct ol_txrx_peer_t *peer, u_int32_t authorize);

bool
ol_txrx_set_inact_params(ol_txrx_pdev_handle pdev,
			u_int16_t inact_check_interval,
			u_int16_t inact_normal,
			u_int16_t inact_overload);
bool
ol_txrx_start_inact_timer(
	ol_txrx_pdev_handle pdev,
	bool enable);

/**
 * @brief Set the overload status of the radio
 * @details
 *   Set the overload status of the radio, updating the inactivity
 *   threshold and inactivity count for each node.
 *
 * @param pdev - the data physical device object
 * @param overload - whether the radio is overloaded or not
 */
void
ol_txrx_set_overload(
	ol_txrx_pdev_handle pdev,
	bool overload);
/**
 * @brief Check the inactivity status of the peer/node
 *
 * @param peer - pointer to the node's object
 * @return true if the node is inactive; otherwise return false
 */
bool
ol_txrx_peer_is_inact(ol_txrx_peer_handle peer);

/**
 * @brief Mark inactivity status of the peer/node
 * @details
 *   If it becomes active, reset inactivity count to reload value;
 *   if the inactivity status changed, notify umac band steering.
 *
 * @param peer - pointer to the node's object
 * @param inactive - whether the node is inactive or not
 */
void
ol_txrx_mark_peer_inact(
	ol_txrx_peer_handle peer,
	bool inactive);


/* Should be ol_txrx_ctrl_api.h */
void ol_txrx_set_mesh_mode(ol_txrx_vdev_handle vdev, u_int32_t val);

void ol_tx_flush_buffers(struct ol_txrx_vdev_t *vdev);

#endif
