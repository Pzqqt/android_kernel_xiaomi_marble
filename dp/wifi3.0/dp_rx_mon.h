/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#ifndef _DP_RX_MON_H_
#define _DP_RX_MON_H_

#ifdef CONFIG_MCL
#include <cds_ieee80211_defines.h>
#endif

/**
* dp_rx_mon_dest_process() - Brain of the Rx processing functionality
*	Called from the bottom half (tasklet/NET_RX_SOFTIRQ)
* @soc: core txrx main context	164
* @hal_ring: opaque pointer to the HAL Rx Ring, which will be serviced
* @quota: No. of units (packets) that can be serviced in one shot.
*
* This function implements the core of Rx functionality. This is
* expected to handle only non-error frames.
*
* Return: uint32_t: No. of elements processed
*/
void dp_rx_mon_dest_process(struct dp_soc *soc, uint32_t mac_id,
	uint32_t quota);


QDF_STATUS dp_rx_pdev_mon_attach(struct dp_pdev *pdev);
QDF_STATUS dp_rx_pdev_mon_detach(struct dp_pdev *pdev);

QDF_STATUS dp_rx_pdev_mon_status_attach(struct dp_pdev *pdev);
QDF_STATUS dp_rx_pdev_mon_status_detach(struct dp_pdev *pdev);

uint32_t dp_mon_process(struct dp_soc *soc, uint32_t mac_id, uint32_t quota);
QDF_STATUS dp_rx_mon_deliver(struct dp_soc *soc, uint32_t mac_id,
	qdf_nbuf_t head_msdu, qdf_nbuf_t tail_msdu);
#endif
