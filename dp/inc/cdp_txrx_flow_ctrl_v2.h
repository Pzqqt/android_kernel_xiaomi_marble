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
 * @file cdp_txrx_flow_ctrl_v2.h
 * @brief Define the host data path flow control version 2 API
 * functions
 */
#ifndef _CDP_TXRX_FC_V2_H_
#define _CDP_TXRX_FC_V2_H_
#include <cdp_txrx_ops.h>
#include <cdp_txrx_cmn.h>

/**
 * cdp_register_pause_cb() - Register flow control callback function pointer
 * @soc - data path soc handle
 * @pause_cb - Pause callback intend to register
 *
 * Register flow control callback function pointer and client context pointer
 *
 * return QDF_STATUS_SUCCESS success
 */
static inline QDF_STATUS
cdp_register_pause_cb(ol_txrx_soc_handle soc,
		tx_pause_callback pause_cb)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("invalid instance");
		QDF_BUG(0);
		return QDF_STATUS_E_INVAL;
	}

	if (!soc->ops->flowctl_ops ||
	    !soc->ops->flowctl_ops->register_pause_cb)
		return QDF_STATUS_SUCCESS;

	return soc->ops->flowctl_ops->register_pause_cb(soc, pause_cb);

}

/**
 * cdp_set_desc_global_pool_size() - set global device pool size
 * @soc - data path soc handle
 * @num_msdu_desc - descriptor pool size
 *
 * set global device pool size
 *
 * return none
 */
static inline void
cdp_set_desc_global_pool_size(ol_txrx_soc_handle soc,
		uint32_t num_msdu_desc)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("invalid instance");
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->flowctl_ops ||
	    !soc->ops->flowctl_ops->set_desc_global_pool_size)
		return;

	soc->ops->flowctl_ops->set_desc_global_pool_size(
			num_msdu_desc);
}

/**
 * cdp_dump_flow_pool_info() - dump flow pool information
 * @soc - data path soc handle
 *
 * dump flow pool information
 *
 * return none
 */
static inline void
cdp_dump_flow_pool_info(struct cdp_soc_t *soc)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("invalid instance");
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->flowctl_ops ||
	    !soc->ops->flowctl_ops->dump_flow_pool_info)
		return;

	soc->ops->flowctl_ops->dump_flow_pool_info(soc);
}

/**
 * cdp_tx_desc_thresh_reached() - Check if avail tx desc meet threshold
 * @soc: data path soc handle
 * @vdev_id: vdev_id corresponding to vdev start
 *
 * Return: true if threshold is met, false if not
 */
static inline bool
cdp_tx_desc_thresh_reached(struct cdp_soc_t *soc, uint8_t vdev_id)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("invalid instance");
		QDF_BUG(0);
		return false;
	}

	if (!soc->ops->flowctl_ops ||
	    !soc->ops->flowctl_ops->tx_desc_thresh_reached)
		return false;

	return soc->ops->flowctl_ops->tx_desc_thresh_reached(soc, vdev_id);
}
#endif /* _CDP_TXRX_FC_V2_H_ */
