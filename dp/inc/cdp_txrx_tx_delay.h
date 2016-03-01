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
 * @file cdp_txrx_tx_delay.h
 * @brief Define the host data path histogram API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_COMPUTE_TX_DELAY_H_
#define _CDP_TXRX_COMPUTE_TX_DELAY_H_

#ifdef QCA_COMPUTE_TX_DELAY
void
ol_tx_delay(ol_txrx_pdev_handle pdev,
		 uint32_t *queue_delay_microsec,
		 uint32_t *tx_delay_microsec, int category);
void
ol_tx_delay_hist(ol_txrx_pdev_handle pdev,
		 uint16_t *bin_values, int category);
void
ol_tx_packet_count(ol_txrx_pdev_handle pdev,
		 uint16_t *out_packet_count,
		 uint16_t *out_packet_loss_count, int category);
void ol_tx_set_compute_interval(ol_txrx_pdev_handle pdev,
		 uint32_t interval);
#else
static inline void
ol_tx_delay(ol_txrx_pdev_handle pdev,
		 uint32_t *queue_delay_microsec,
		 uint32_t *tx_delay_microsec, int category)
{
	return;
}

static inline void
ol_tx_delay_hist(ol_txrx_pdev_handle pdev,
		 uint16_t *bin_values, int category)
{
	return;
}

static inline void
ol_tx_packet_count(ol_txrx_pdev_handle pdev,
		 uint16_t *out_packet_count,
		 uint16_t *out_packet_loss_count, int category)
{
	return;
}

static inline void
ol_tx_set_compute_interval(ol_txrx_pdev_handle pdev,
		 uint32_t interval)
{
	return;
}
#endif

#endif /* _CDP_TXRX_COMPUTE_TX_DELAY_H_ */
