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
 * @file cdp_txrx_tx_delay.h
 * @brief Define the host data path histogram API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_COMPUTE_TX_DELAY_H_
#define _CDP_TXRX_COMPUTE_TX_DELAY_H_
#include "cdp_txrx_handle.h"
/**
 * cdp_tx_delay() - get tx packet delay
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 * @queue_delay_microsec: tx packet delay within queue, usec
 * @tx_delay_microsec: tx packet delay, usec
 * @category: packet category
 *
 * Return: NONE
 */
static inline void
cdp_tx_delay(ol_txrx_soc_handle soc, uint8_t pdev_id,
	     uint32_t *queue_delay_microsec, uint32_t *tx_delay_microsec,
	     int category)
{
	if (!soc || !soc->ops || !soc->ops->delay_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->delay_ops->tx_delay)
		return soc->ops->delay_ops->tx_delay(soc, pdev_id,
			queue_delay_microsec, tx_delay_microsec, category);
	return;
}

/**
 * cdp_tx_delay_hist() - get tx packet delay histogram
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 * @bin_values: bin
 * @category: packet category
 *
 * Return: NONE
 */
static inline void
cdp_tx_delay_hist(ol_txrx_soc_handle soc, uint8_t pdev_id,
		  uint16_t *bin_values, int category)
{
	if (!soc || !soc->ops || !soc->ops->delay_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->delay_ops->tx_delay_hist)
		return soc->ops->delay_ops->tx_delay_hist(soc, pdev_id,
			bin_values, category);
	return;
}

/**
 * cdp_tx_packet_count() - get tx packet count
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 * @out_packet_loss_count: packet loss count
 * @category: packet category
 *
 * Return: NONE
 */
static inline void
cdp_tx_packet_count(ol_txrx_soc_handle soc, uint8_t pdev_id,
		    uint16_t *out_packet_count, uint16_t *out_packet_loss_count,
		    int category)
{
	if (!soc || !soc->ops || !soc->ops->delay_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->delay_ops->tx_packet_count)
		return soc->ops->delay_ops->tx_packet_count(soc, pdev_id,
			out_packet_count, out_packet_loss_count, category);
	return;
}

/**
 * cdp_tx_set_compute_interval() - set tx packet stat compute interval
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 * @interval: compute interval
 *
 * Return: NONE
 */
static inline void
cdp_tx_set_compute_interval(ol_txrx_soc_handle soc, uint8_t pdev_id,
			    uint32_t interval)
{
	if (!soc || !soc->ops || !soc->ops->delay_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->delay_ops->tx_set_compute_interval)
		return soc->ops->delay_ops->tx_set_compute_interval(soc,
								    pdev_id,
								    interval);
	return;
}
#endif /* _CDP_TXRX_COMPUTE_TX_DELAY_H_ */
