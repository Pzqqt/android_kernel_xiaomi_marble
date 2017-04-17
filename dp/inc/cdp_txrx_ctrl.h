/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
#include "cdp_txrx_handle.h"

static inline int cdp_is_target_ar900b
	(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (soc->ops->ctrl_ops->txrx_is_target_ar900b)
		return soc->ops->ctrl_ops->txrx_is_target_ar900b(vdev);
	return 0;
}


/* WIN */
static inline int
cdp_mempools_attach(ol_txrx_soc_handle soc, void *ctrl_pdev)
{
	if (soc->ops->ctrl_ops->txrx_mempools_attach)
		return soc->ops->ctrl_ops->txrx_mempools_attach(ctrl_pdev);
	return 0;
}

static inline int
cdp_set_filter_neighbour_peers(ol_txrx_soc_handle soc,
	struct cdp_pdev *pdev, u_int32_t val)
{
	if (soc->ops->ctrl_ops->txrx_set_filter_neighbour_peers)
		return soc->ops->ctrl_ops->txrx_set_filter_neighbour_peers
			(pdev, val);
	return 0;
}
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

static inline void
cdp_set_safemode(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev, u_int32_t val)
{
	if (soc->ops->ctrl_ops->txrx_set_safemode)
		return soc->ops->ctrl_ops->txrx_set_safemode(vdev, val);
	return;
}
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
static inline void
cdp_set_drop_unenc(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev, u_int32_t val)
{
	if (soc->ops->ctrl_ops->txrx_set_drop_unenc)
		return soc->ops->ctrl_ops->txrx_set_drop_unenc(vdev, val);
	return;
}


/**
 * @brief set the Tx encapsulation type of the VDEV
 * @details
 *  This will be used to populate the HTT desc packet type field during Tx
 *
 * @param vdev - the data virtual device object
 * @param val - the Tx encap type (htt_cmn_pkt_type)
 * @return - void
 */
static inline void
cdp_set_tx_encap_type(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev, enum htt_cmn_pkt_type val)
{
	if (soc->ops->ctrl_ops->txrx_set_tx_encap_type)
		return soc->ops->ctrl_ops->txrx_set_tx_encap_type(vdev, val);
	return;
}

/**
 * @brief set the Rx decapsulation type of the VDEV
 * @details
 *  This will be used to configure into firmware and hardware which format to
 *  decap all Rx packets into, for all peers under the VDEV.
 *
 * @param vdev - the data virtual device object
 * @param val - the Rx decap mode (htt_cmn_pkt_type)
 * @return - void
 */
static inline void
cdp_set_vdev_rx_decap_type(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev, enum htt_cmn_pkt_type val)
{
	if (soc->ops->ctrl_ops->txrx_set_vdev_rx_decap_type)
		return soc->ops->ctrl_ops->txrx_set_vdev_rx_decap_type
			(vdev, val);
	return;
}

/**
 * @brief get the Rx decapsulation type of the VDEV
 *
 * @param vdev - the data virtual device object
 * @return - the Rx decap type (htt_cmn_pkt_type)
 */
static inline enum htt_cmn_pkt_type
cdp_get_vdev_rx_decap_type(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (soc->ops->ctrl_ops->txrx_get_vdev_rx_decap_type)
		return soc->ops->ctrl_ops->txrx_get_vdev_rx_decap_type(vdev);
	return 0;
}

/**
 * @brief set the Reo Destination ring for the pdev
 * @details
 *  This will be used to configure the Reo Destination ring for this pdev.
 *
 * @param soc - pointer to the soc
 * @param pdev - the data physical device object
 * @param val - the Reo destination ring index (1 to 4)
 * @return - void
 */
static inline void
cdp_set_pdev_reo_dest(ol_txrx_soc_handle soc,
	struct cdp_pdev *pdev, enum cdp_host_reo_dest_ring val)
{
	if (soc->ops->ctrl_ops->txrx_set_pdev_reo_dest)
		return soc->ops->ctrl_ops->txrx_set_pdev_reo_dest
			(pdev, val);
	return;
}

/**
 * @brief get the Reo Destination ring for the pdev
 *
 * @param soc - pointer to the soc
 * @param pdev - the data physical device object
 * @return - the Reo destination ring index (1 to 4), 0 if not supported.
 */
static inline enum cdp_host_reo_dest_ring
cdp_get_pdev_reo_dest(ol_txrx_soc_handle soc, struct cdp_pdev *pdev)
{
	if (soc->ops->ctrl_ops->txrx_get_pdev_reo_dest)
		return soc->ops->ctrl_ops->txrx_get_pdev_reo_dest(pdev);
	return cdp_host_reo_dest_ring_unknown;
}

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
static inline void
cdp_peer_authorize(ol_txrx_soc_handle soc,
	struct ol_txrx_peer_t *peer, u_int32_t authorize)
{
	if (soc->ops->ctrl_ops->txrx_peer_authorize)
		return soc->ops->ctrl_ops->txrx_peer_authorize
			(peer, authorize);
	return;
}

static inline bool
cdp_set_inact_params(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
			u_int16_t inact_check_interval,
			u_int16_t inact_normal,
			u_int16_t inact_overload)
{
	if (soc->ops->ctrl_ops->txrx_set_inact_params)
		return soc->ops->ctrl_ops->txrx_set_inact_params
			(pdev, inact_check_interval, inact_normal,
			inact_overload);
	return false;
}
static inline bool
cdp_start_inact_timer(ol_txrx_soc_handle soc,
	struct cdp_pdev *pdev,
	bool enable)
{
	if (soc->ops->ctrl_ops->txrx_start_inact_timer)
		return soc->ops->ctrl_ops->txrx_start_inact_timer
			(pdev, enable);
	return false;
}

/**
 * @brief Set the overload status of the radio
 * @details
 *   Set the overload status of the radio, updating the inactivity
 *   threshold and inactivity count for each node.
 *
 * @param pdev - the data physical device object
 * @param overload - whether the radio is overloaded or not
 */
static inline void
cdp_set_overload(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
	bool overload)
{
	if (soc->ops->ctrl_ops->txrx_set_overload)
		return soc->ops->ctrl_ops->txrx_set_overload(pdev, overload);
	return;
}

/**
 * @brief Check the inactivity status of the peer/node
 *
 * @param peer - pointer to the node's object
 * @return true if the node is inactive; otherwise return false
 */
static inline bool
cdp_peer_is_inact(ol_txrx_soc_handle soc, void *peer)
{
	if (soc->ops->ctrl_ops->txrx_peer_is_inact)
		return soc->ops->ctrl_ops->txrx_peer_is_inact(peer);
	return false;
}

/**
 * @brief Mark inactivity status of the peer/node
 * @details
 *   If it becomes active, reset inactivity count to reload value;
 *   if the inactivity status changed, notify umac band steering.
 *
 * @param peer - pointer to the node's object
 * @param inactive - whether the node is inactive or not
 */
static inline void
cdp_mark_peer_inact(ol_txrx_soc_handle soc,
	void *peer,
	bool inactive)
{
	if (soc->ops->ctrl_ops->txrx_mark_peer_inact)
		return soc->ops->ctrl_ops->txrx_mark_peer_inact
			(peer, inactive);
	return;
}


/* Should be ol_txrx_ctrl_api.h */
static inline void cdp_set_mesh_mode
(ol_txrx_soc_handle soc, struct cdp_vdev *vdev, u_int32_t val)
{
	if (soc->ops->ctrl_ops->txrx_set_mesh_mode)
		return soc->ops->ctrl_ops->txrx_set_mesh_mode(vdev, val);
	return;
}

/**
 * @brief set mesh rx filter
 * @details based on the bits enabled in the filter packets has to be dropped.
 *
 * @param soc - pointer to the soc
 * @param vdev - the data virtual device object
 * @param val - value to be set
 * @return - void
 */
static inline
void cdp_set_mesh_rx_filter(ol_txrx_soc_handle soc,
				struct cdp_vdev *vdev, uint32_t val)
{
	if (soc->ops->ctrl_ops->txrx_set_mesh_rx_filter)
		return soc->ops->ctrl_ops->txrx_set_mesh_rx_filter(vdev, val);
	return;
}

static inline void cdp_tx_flush_buffers
(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (soc->ops->ctrl_ops->tx_flush_buffers)
		return soc->ops->ctrl_ops->tx_flush_buffers(vdev);
	return;
}

static inline void cdp_txrx_set_vdev_param(ol_txrx_soc_handle soc,
		struct cdp_vdev *vdev, enum cdp_vdev_param_type type,
		uint32_t val)
{
	if (soc->ops->ctrl_ops->txrx_set_vdev_param)
		return soc->ops->ctrl_ops->txrx_set_vdev_param(vdev, type, val);
	return;
}

static inline void
cdp_peer_set_nawds(ol_txrx_soc_handle soc,
		struct ol_txrx_peer_t *peer, uint8_t value)
{
	if (soc->ops->ctrl_ops->txrx_peer_set_nawds)
		return soc->ops->ctrl_ops->txrx_peer_set_nawds
			(peer, value);
	return;
}

/**
 * @brief Subscribe to a specified WDI event.
 * @details
 *  This function adds the provided wdi_event_subscribe object to a list of
 *  subscribers for the specified WDI event.
 *  When the event in question happens, each subscriber for the event will
 *  have their callback function invoked.
 *  The order in which callback functions from multiple subscribers are
 *  invoked is unspecified.
 *
 * @param soc - pointer to the soc
 * @param pdev - the data physical device object
 * @param event_cb_sub - the callback and context for the event subscriber
 * @param event - which event's notifications are being subscribed to
 * @return - int
 */
static inline int
cdp_wdi_event_sub(ol_txrx_soc_handle soc,
		struct cdp_pdev *pdev, void *event_cb_sub, uint32_t event)
{
	if (!soc || !soc->ops || !soc->ops->ctrl_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->ctrl_ops->txrx_wdi_event_sub)
		return soc->ops->ctrl_ops->txrx_wdi_event_sub
			(pdev, event_cb_sub, event);
	return 0;
}

/**
 * @brief Unsubscribe from a specified WDI event.
 * @details
 *  This function removes the provided event subscription object from the
 *  list of subscribers for its event.
 *  This function shall only be called if there was a successful prior call
 *  to event_sub() on the same wdi_event_subscribe object.
 *
 * @param soc - pointer to the soc
 * @param pdev - the data physical device object
 * @param event_cb_sub - the callback and context for the event subscriber
 * @param event - which event's notifications are being subscribed to
 * @return - int
 */
static inline int
cdp_wdi_event_unsub(ol_txrx_soc_handle soc,
		struct cdp_pdev *pdev, void *event_cb_sub, uint32_t event)
{
	if (!soc || !soc->ops || !soc->ops->ctrl_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->ctrl_ops->txrx_wdi_event_unsub)
		return soc->ops->ctrl_ops->txrx_wdi_event_unsub
			(pdev, event_cb_sub, event);
	return 0;
}
#endif
