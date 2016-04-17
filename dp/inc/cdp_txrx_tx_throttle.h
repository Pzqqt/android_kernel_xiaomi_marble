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
 * @file cdp_txrx_tx_throttle.h
 * @brief Define the host data path transmit throttle API
 * functions called by the host control SW and the OS interface
 * module
 */
#ifndef _CDP_TXRX_TX_THROTTLE_H_
#define _CDP_TXRX_TX_THROTTLE_H_

#if defined(QCA_SUPPORT_TX_THROTTLE)
void ol_tx_throttle_init_period(struct ol_txrx_pdev_t *pdev, int period,
				uint8_t *dutycycle_level);

void ol_tx_throttle_set_level(struct ol_txrx_pdev_t *pdev, int level);
#else
static inline void ol_tx_throttle_set_level(struct ol_txrx_pdev_t *pdev,
					    int level)
{
	/* no-op */
}

static inline void ol_tx_throttle_init_period(struct ol_txrx_pdev_t *pdev,
					      int period,
					      uint8_t *dutycycle_level)
{
	/* no-op */
}
#endif
#endif /* _CDP_TXRX_TX_THROTTLE_H_ */
