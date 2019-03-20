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

/**
 * @file cdp_txrx_ctrl.h
 * @brief Define the host data path control API functions
 * called by the host control SW and the OS interface module
 */

#ifndef _CDP_TXRX_CTRL_H_
#define _CDP_TXRX_CTRL_H_
#include "cdp_txrx_handle.h"
#include "cdp_txrx_cmn_struct.h"
#include "cdp_txrx_cmn.h"
#include "cdp_txrx_ops.h"

static inline int cdp_is_target_ar900b
	(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_is_target_ar900b)
		return 0;

	return soc->ops->ctrl_ops->txrx_is_target_ar900b(vdev);
}


/* WIN */
static inline int
cdp_mempools_attach(ol_txrx_soc_handle soc, void *ctrl_pdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_mempools_attach)
		return 0;

	return soc->ops->ctrl_ops->txrx_mempools_attach(ctrl_pdev);
}

/**
 * @brief set filter neighbour peers
 * @details
 *  This defines interface function to set neighbour peer filtering.
 *
 * @param soc - the pointer to soc object
 * @param pdev - the pointer physical device object
 * @param val - the enable/disable value
 * @return - int
 */
static inline int
cdp_set_filter_neighbour_peers(ol_txrx_soc_handle soc,
	struct cdp_pdev *pdev, u_int32_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_set_filter_neighbour_peers)
		return 0;

	return soc->ops->ctrl_ops->txrx_set_filter_neighbour_peers
			(pdev, val);
}

/**
 * @brief update the neighbour peer addresses
 * @details
 *  This defines interface function to update neighbour peers addresses
 *  which needs to be filtered
 *
 * @param soc - the pointer to soc object
 * @param vdev - the pointer to vdev
 * @param cmd - add/del entry into peer table
 * @param macaddr - the address of neighbour peer
 * @return - int
 */
static inline int
cdp_update_filter_neighbour_peers(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev, uint32_t cmd, uint8_t *macaddr)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_update_filter_neighbour_peers)
		return 0;

	return soc->ops->ctrl_ops->txrx_update_filter_neighbour_peers
			(vdev, cmd, macaddr);
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
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_set_safemode)
		return;

	soc->ops->ctrl_ops->txrx_set_safemode(vdev, val);
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
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_set_drop_unenc)
		return;

	soc->ops->ctrl_ops->txrx_set_drop_unenc(vdev, val);
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
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_set_tx_encap_type)
		return;

	soc->ops->ctrl_ops->txrx_set_tx_encap_type(vdev, val);
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
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_set_vdev_rx_decap_type)
		return;

	soc->ops->ctrl_ops->txrx_set_vdev_rx_decap_type
			(vdev, val);
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
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_get_vdev_rx_decap_type)
		return 0;

	return soc->ops->ctrl_ops->txrx_get_vdev_rx_decap_type(vdev);
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
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_set_pdev_reo_dest)
		return;

	soc->ops->ctrl_ops->txrx_set_pdev_reo_dest
			(pdev, val);
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
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return cdp_host_reo_dest_ring_unknown;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_get_pdev_reo_dest)
		return cdp_host_reo_dest_ring_unknown;

	return soc->ops->ctrl_ops->txrx_get_pdev_reo_dest(pdev);
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
	struct cdp_peer *peer, u_int32_t authorize)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_peer_authorize)
		return;

	soc->ops->ctrl_ops->txrx_peer_authorize
			(peer, authorize);
}

/* Should be ol_txrx_ctrl_api.h */
static inline void cdp_set_mesh_mode
(ol_txrx_soc_handle soc, struct cdp_vdev *vdev, u_int32_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_set_mesh_mode)
		return;

	soc->ops->ctrl_ops->txrx_set_mesh_mode(vdev, val);
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
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_set_mesh_rx_filter)
		return;

	soc->ops->ctrl_ops->txrx_set_mesh_rx_filter(vdev, val);
}

static inline void cdp_tx_flush_buffers
(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->tx_flush_buffers)
		return;

	soc->ops->ctrl_ops->tx_flush_buffers(vdev);
}

static inline uint32_t cdp_txrx_get_vdev_param(ol_txrx_soc_handle soc,
					       struct cdp_vdev *vdev,
					       enum cdp_vdev_param_type type)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return -1;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_get_vdev_param) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: callback not registered:", __func__);
		return -1;
	}

	return soc->ops->ctrl_ops->txrx_get_vdev_param(vdev, type);
}

static inline void cdp_txrx_set_vdev_param(ol_txrx_soc_handle soc,
		struct cdp_vdev *vdev, enum cdp_vdev_param_type type,
		uint32_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_set_vdev_param)
		return;

	soc->ops->ctrl_ops->txrx_set_vdev_param(vdev, type, val);
}

static inline void
cdp_peer_set_nawds(ol_txrx_soc_handle soc,
		struct cdp_peer *peer, uint8_t value)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_peer_set_nawds)
		return;

	soc->ops->ctrl_ops->txrx_peer_set_nawds
			(peer, value);
}

/**
 * cdp_txrx_set_pdev_param() - set pdev parameter
 * @soc: opaque soc handle
 * @pdev: data path pdev handle
 * @type: param type
 * @val: value of pdev_tx_capture
 *
 * Return: status: 0 - Success, non-zero: Failure
 */
static inline QDF_STATUS cdp_txrx_set_pdev_param(ol_txrx_soc_handle soc,
						 struct cdp_pdev *pdev,
						 enum cdp_pdev_param_type type,
						 uint8_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return QDF_STATUS_SUCCESS;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_set_pdev_param)
		return QDF_STATUS_SUCCESS;

	return soc->ops->ctrl_ops->txrx_set_pdev_param
			(pdev, type, val);
}

/**
 * cdp_enable_peer_based_pktlog()- Set flag in peer structure
 *
 * @soc: pointer to the soc
 * @pdev: the data physical device object
 * @enable: enable or disable peer based filter based pktlog
 * @peer_macaddr: Mac address of peer which needs to be
 * filtered
 *
 * This function will set flag in peer structure if peer based filtering
 * is enabled for pktlog
 *
 * Return: int
 */
static inline int
cdp_enable_peer_based_pktlog(ol_txrx_soc_handle soc,
			     struct cdp_pdev *pdev, char *peer_macaddr,
			     uint8_t enable)
{
	if (!soc || !soc->ops) {
		QDF_TRACE_ERROR(QDF_MODULE_ID_DP,
				"%s invalid instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->enable_peer_based_pktlog)
		return 0;

	return soc->ops->ctrl_ops->enable_peer_based_pktlog
			(pdev, peer_macaddr, enable);
}

/**
 * cdp_calculate_delay_stats()- get rx delay stats
 *
 * @soc: pointer to the soc
 * @vdev: vdev handle
 * @nbuf: nbuf which is passed
 *
 * This function will calculate rx delay statistics.
 */
static inline void
cdp_calculate_delay_stats(ol_txrx_soc_handle soc, struct cdp_vdev *vdev,
			  qdf_nbuf_t nbuf)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->calculate_delay_stats) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
			  "%s: callback not registered:", __func__);
		return;
	}

	return soc->ops->ctrl_ops->calculate_delay_stats(vdev, nbuf);
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

	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_wdi_event_sub)
		return 0;

	return soc->ops->ctrl_ops->txrx_wdi_event_sub
			(pdev, event_cb_sub, event);
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

	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_wdi_event_unsub)
		return 0;

	return soc->ops->ctrl_ops->txrx_wdi_event_unsub
			(pdev, event_cb_sub, event);
}

/**
 * @brief Get security type from the from peer.
 * @details
 * This function gets the Security information from the peer handler.
 * The security information is got from the rx descriptor and filled in
 * to the peer handler.
 *
 * @param soc - pointer to the soc
 * @param peer - peer handler
 * @param sec_idx - mcast or ucast frame type.
 * @return - int
 */
static inline int
cdp_get_sec_type(ol_txrx_soc_handle soc, struct cdp_peer *peer, uint8_t sec_idx)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return A_ERROR;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_get_sec_type)
		return A_ERROR;

	return soc->ops->ctrl_ops->txrx_get_sec_type
			(peer, sec_idx);
}

/**
  * cdp_set_mgmt_tx_power(): function to set tx power for mgmt frames
  * @vdev_handle: vdev handle
  * @subtype_index: subtype
  * @tx_power: Tx power
  * Return: None
  */
static inline int cdp_set_mgmt_tx_power(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev, uint8_t subtype, uint8_t tx_power)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance:", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_update_mgmt_txpow_vdev)
		return 0;

	soc->ops->ctrl_ops->txrx_update_mgmt_txpow_vdev(vdev,
							subtype, tx_power);
	return 0;
}

static inline void *
cdp_get_pldev(ol_txrx_soc_handle soc,
		struct cdp_pdev *pdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return NULL;
	}

	if (!soc->ops->ctrl_ops || !soc->ops->ctrl_ops->txrx_get_pldev)
		return NULL;

	return soc->ops->ctrl_ops->txrx_get_pldev(pdev);
}

#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
/**
 * cdp_update_pdev_rx_protocol_tag() - wrapper function to set the protocol
 *                                    tag in CDP layer from cfg layer
 * @soc: SOC TXRX handle
 * @pdev: CDP pdev pointer
 * @protocol_mask: Bitmap for protocol for which tagging is enabled
 * @protocol_type: Protocol type for which the tag should be update
 * @tag: Actual tag value for the given prototype
 * Return: Returns QDF_STATUS_SUCCESS/FAILURE
 */
static inline QDF_STATUS
cdp_update_pdev_rx_protocol_tag(ol_txrx_soc_handle soc,
				struct cdp_pdev *pdev, uint32_t protocol_mask,
				uint16_t protocol_type, uint16_t tag)
{
	if (!soc || !soc->ops) {
		dp_err("Invalid SOC instance");
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_update_pdev_rx_protocol_tag)
		return QDF_STATUS_E_FAILURE;

	return soc->ops->ctrl_ops->txrx_update_pdev_rx_protocol_tag
			(pdev, protocol_mask, protocol_type, tag);
}

#ifdef WLAN_SUPPORT_RX_TAG_STATISTICS
/**
 * cdp_dump_pdev_rx_protocol_tag_stats() - wrapper function to dump the protocol
				tag statistics for given or all protocols
 * @soc: SOC TXRX handle
 * @pdev: CDP pdev pointer
 * @protocol_type: Protocol type for which the tag should be update
 * Return: Returns QDF_STATUS_SUCCESS/FAILURE
 */
static inline QDF_STATUS
cdp_dump_pdev_rx_protocol_tag_stats(ol_txrx_soc_handle soc,
				    struct cdp_pdev *pdev,
				    uint16_t protocol_type)
{
	if (!soc || !soc->ops) {
		dp_err("Invalid SOC instance");
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_dump_pdev_rx_protocol_tag_stats)
		return QDF_STATUS_E_FAILURE;

	soc->ops->ctrl_ops->txrx_dump_pdev_rx_protocol_tag_stats(pdev,
						protocol_type);
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUPPORT_RX_TAG_STATISTICS */
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */

#ifdef ATH_SUPPORT_NAC_RSSI
/**
  * cdp_vdev_config_for_nac_rssi(): To invoke dp callback for nac rssi config
  * @soc: soc pointer
  * @vdev: vdev pointer
  * @nac_cmd: specfies nac_rss config action add, del, list
  * @bssid: Neighbour bssid
  * @client_macaddr: Non-Associated client MAC
  * @chan_num: channel number to scan
  *
  * Return: QDF_STATUS
  */
static inline QDF_STATUS cdp_vdev_config_for_nac_rssi(ol_txrx_soc_handle soc,
		struct cdp_vdev *vdev, enum cdp_nac_param_cmd nac_cmd,
		char *bssid, char *client_macaddr, uint8_t chan_num)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}

	if (!soc->ops->ctrl_ops ||
			!soc->ops->ctrl_ops->txrx_vdev_config_for_nac_rssi)
		return QDF_STATUS_E_FAILURE;

	return soc->ops->ctrl_ops->txrx_vdev_config_for_nac_rssi(vdev,
			nac_cmd, bssid, client_macaddr, chan_num);
}

/*
 * cdp_vdev_get_neighbour_rssi(): To invoke dp callback to get rssi value of nac
 * @soc: soc pointer
 * @vdev: vdev pointer
 * @macaddr: Non-Associated client MAC
 * @rssi: rssi
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS cdp_vdev_get_neighbour_rssi(ol_txrx_soc_handle soc,
						     struct cdp_vdev *vdev,
						     char *macaddr,
						     uint8_t *rssi)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}

	if (!soc->ops->ctrl_ops ||
	    !soc->ops->ctrl_ops->txrx_vdev_get_neighbour_rssi)
		return QDF_STATUS_E_FAILURE;

	return soc->ops->ctrl_ops->txrx_vdev_get_neighbour_rssi(vdev, macaddr,
								rssi);
}
#endif
#endif
