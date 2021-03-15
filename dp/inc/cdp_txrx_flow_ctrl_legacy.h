/*
 * Copyright (c) 2016-2019,2021 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_flow_ctrl_legacy.h
 * @brief Define the host data path legacy flow control API
 * functions
 */
#ifndef _CDP_TXRX_FC_LEG_H_
#define _CDP_TXRX_FC_LEG_H_
#include <cdp_txrx_mob_def.h>
#include "cdp_txrx_handle.h"
#include <cdp_txrx_cmn.h>
#ifdef QCA_HL_NETDEV_FLOW_CONTROL

/**
 * cdp_hl_fc_register() - Register HL flow control callback.
 * @soc: data path soc handle
 * @pdev_id: datapath pdev identifier
 * @flowcontrol: callback function pointer to stop/start OS netdev queues
 *
 * Register flow control callback.
 *
 * Returns: 0 for success
 */
static inline int
cdp_hl_fc_register(ol_txrx_soc_handle soc, uint8_t pdev_id,
		   tx_pause_callback flowcontrol)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("invalid instance");
		QDF_BUG(0);
		return -EINVAL;
	}

	if (!soc->ops->l_flowctl_ops ||
	    !soc->ops->l_flowctl_ops->register_tx_flow_control)
		return -EINVAL;

	return soc->ops->l_flowctl_ops->register_tx_flow_control(soc, pdev_id,
								 flowcontrol);
}

static inline int cdp_hl_fc_set_td_limit(ol_txrx_soc_handle soc,
					 uint8_t vdev_id, uint32_t chan_freq)
{
	if (!soc->ops->l_flowctl_ops->set_vdev_tx_desc_limit)
		return 0;

	return soc->ops->l_flowctl_ops->set_vdev_tx_desc_limit(soc, vdev_id,
							       chan_freq);
}

static inline int cdp_hl_fc_set_os_queue_status(ol_txrx_soc_handle soc,
						uint8_t vdev_id,
					    enum netif_action_type action)
{
	if (!soc->ops->l_flowctl_ops->set_vdev_os_queue_status)
		return -EINVAL;

	return soc->ops->l_flowctl_ops->set_vdev_os_queue_status(soc,
								 vdev_id,
								 action);
}
#else
static inline int
cdp_hl_fc_register(ol_txrx_soc_handle soc, uint8_t pdev_id,
		   tx_pause_callback flowcontrol)
{
	return 0;
}

static inline int cdp_hl_fc_set_td_limit(ol_txrx_soc_handle soc,
					 uint8_t vdev_id, uint32_t chan_freq)
{
	return 0;
}

static inline int cdp_hl_fc_set_os_queue_status(ol_txrx_soc_handle soc,
						uint8_t vdev_id,
						enum netif_action_type action)
{
	return 0;
}

#endif /* QCA_HL_NETDEV_FLOW_CONTROL */

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
/**
 * cdp_fc_register() - Register flow control callback function pointer
 * @soc - data path soc handle
 * @vdev_id - virtual interface id to register flow control
 * @flowControl - callback function pointer
 * @osif_fc_ctx - client context pointer
 * @flow_control_is_pause: is vdev paused by flow control
 *
 * Register flow control callback function pointer and client context pointer
 *
 * return 0 success
 */
static inline int
cdp_fc_register(ol_txrx_soc_handle soc, uint8_t vdev_id,
		ol_txrx_tx_flow_control_fp flowcontrol, void *osif_fc_ctx,
		ol_txrx_tx_flow_control_is_pause_fp flow_control_is_pause)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("invalid instance");
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->l_flowctl_ops ||
	    !soc->ops->l_flowctl_ops->register_tx_flow_control)
		return 0;

	return soc->ops->l_flowctl_ops->register_tx_flow_control(
			soc, vdev_id, flowcontrol, osif_fc_ctx,
			flow_control_is_pause);
}
#else
static inline int
cdp_fc_register(ol_txrx_soc_handle soc, uint8_t vdev_id,
		ol_txrx_tx_flow_control_fp flowcontrol, void *osif_fc_ctx,
		ol_txrx_tx_flow_control_is_pause_fp flow_control_is_pause)
{
	return 0;
}
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
/**
 * cdp_fc_deregister() - remove flow control instance
 * @soc - data path soc handle
 * @vdev_id - virtual interface id to register flow control
 *
 * remove flow control instance
 *
 * return 0 success
 */
static inline int
cdp_fc_deregister(ol_txrx_soc_handle soc, uint8_t vdev_id)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("invalid instance");
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->l_flowctl_ops ||
	    !soc->ops->l_flowctl_ops->deregister_tx_flow_control_cb)
		return 0;

	return soc->ops->l_flowctl_ops->deregister_tx_flow_control_cb(
			soc, vdev_id);
}

/**
 * cdp_fc_get_tx_resource() - get data path resource count
 * @soc: data path soc handle
 * @pdev_id: datapath pdev ID
 * @peer_addr: peer mac address
 * @low_watermark: low resource threshold
 * @high_watermark_offset: high resource threshold
 *
 * get data path resource count
 *
 * return true enough data path resource available
 *        false resource is not avaialbe
 */
static inline bool
cdp_fc_get_tx_resource(ol_txrx_soc_handle soc, uint8_t pdev_id,
		       struct qdf_mac_addr peer_addr,
		       unsigned int low_watermark,
		       unsigned int high_watermark_offset)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("invalid instance");
		QDF_BUG(0);
		return false;
	}

	if (!soc->ops->l_flowctl_ops ||
	    !soc->ops->l_flowctl_ops->get_tx_resource)
		return false;

	return soc->ops->l_flowctl_ops->get_tx_resource(soc, pdev_id, peer_addr,
							low_watermark,
							high_watermark_offset);
}

/**
 * cdp_fc_ll_set_tx_pause_q_depth() - set pause queue depth
 * @soc - data path soc handle
 * @vdev_id - virtual interface id to register flow control
 * @pause_q_depth - pending tx queue delth
 *
 * set pause queue depth
 *
 * return 0 success
 */
static inline int
cdp_fc_ll_set_tx_pause_q_depth(ol_txrx_soc_handle soc,
		uint8_t vdev_id, int pause_q_depth)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("invalid instance");
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->l_flowctl_ops ||
	    !soc->ops->l_flowctl_ops->ll_set_tx_pause_q_depth)
		return 0;

	return soc->ops->l_flowctl_ops->ll_set_tx_pause_q_depth(
			soc, vdev_id, pause_q_depth);

}

/**
 * cdp_fc_vdev_flush() - flush tx queue
 * @soc: data path soc handle
 * @vdev_id: id of vdev
 *
 * flush tx queue
 *
 * return None
 */
static inline void
cdp_fc_vdev_flush(ol_txrx_soc_handle soc, uint8_t vdev_id)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("invalid instance");
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->l_flowctl_ops ||
	    !soc->ops->l_flowctl_ops->vdev_flush)
		return;

	soc->ops->l_flowctl_ops->vdev_flush(soc, vdev_id);
}

/**
 * cdp_fc_vdev_pause() - pause tx scheduler on vdev
 * @soc: data path soc handle
 * @vdev_id: id of vdev
 * @reason: pause reason
 * @pause_type: type of pause
 *
 * pause tx scheduler on vdev
 *
 * return None
 */
static inline void
cdp_fc_vdev_pause(ol_txrx_soc_handle soc, uint8_t vdev_id,
		  uint32_t reason, uint32_t pause_type)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("invalid instance");
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->l_flowctl_ops ||
	    !soc->ops->l_flowctl_ops->vdev_pause)
		return;

	soc->ops->l_flowctl_ops->vdev_pause(soc, vdev_id, reason, pause_type);
}

/**
 * cdp_fc_vdev_unpause() - resume tx scheduler on vdev
 * @soc: data path soc handle
 * @vdev_id: id of vdev
 * @reason: pause reason
 * @pause_type: type of pause
 *
 * resume tx scheduler on vdev
 *
 * return None
 */
static inline void
cdp_fc_vdev_unpause(ol_txrx_soc_handle soc, uint8_t vdev_id,
		    uint32_t reason, uint32_t pause_type)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("invalid instance");
		return;
	}

	if (!soc->ops->l_flowctl_ops ||
	    !soc->ops->l_flowctl_ops->vdev_unpause)
		return;

	soc->ops->l_flowctl_ops->vdev_unpause(soc, vdev_id, reason,
					      pause_type);
}
#endif /* _CDP_TXRX_FC_LEG_H_ */
