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

#ifndef _DP_RX_MON_H_
#define _DP_RX_MON_H_

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
QDF_STATUS dp_rx_pdev_mon_status_attach(struct dp_pdev *pdev, int mac_id);
QDF_STATUS dp_rx_pdev_mon_status_detach(struct dp_pdev *pdev, int mac_id);

/**
 * dp_reset_monitor_mode() - Disable monitor mode
 * @pdev_handle: Datapath PDEV handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_reset_monitor_mode(struct cdp_pdev *pdev_handle);

/**
 * dp_pdev_configure_monitor_rings() - configure monitor rings
 * @vdev_handle: Datapath VDEV handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_pdev_configure_monitor_rings(struct dp_pdev *pdev);

/**
 * dp_mon_link_free() - free monitor link desc pool
 * @pdev: core txrx pdev context
 *
 * This function will release DP link desc pool for monitor mode from
 * main device context.
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS dp_mon_link_free(struct dp_pdev *pdev);


uint32_t dp_mon_process(struct dp_soc *soc, uint32_t mac_id, uint32_t quota);
QDF_STATUS dp_rx_mon_deliver(struct dp_soc *soc, uint32_t mac_id,
	qdf_nbuf_t head_msdu, qdf_nbuf_t tail_msdu);
/*
 * dp_rx_mon_deliver_non_std() - deliver frames for non standard path
 * @soc: core txrx main contex
 * @mac_id: MAC ID
 *
 * This function delivers the radio tap and dummy MSDU
 * into user layer application for preamble only PPDU.
 *
 * Return: Operation status
 */
QDF_STATUS dp_rx_mon_deliver_non_std(struct dp_soc *soc, uint32_t mac_id);

/**
 * dp_rxdma_err_process() - RxDMA error processing functionality
 * @soc: core txrx main contex
 * @mac_id: mac id which is one of 3 mac_ids
 * @hal_ring: opaque pointer to the HAL Rx Ring, which will be serviced
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * Return: num of buffers processed
 */
uint32_t dp_rxdma_err_process(struct dp_intr *int_ctx, struct dp_soc *soc,
			      uint32_t mac_id, uint32_t quota);

#ifndef REMOVE_MON_DBG_STATS
/*
 * dp_rx_mon_update_dbg_ppdu_stats() - Update status ring TLV count
 * @ppdu_info: HAL RX PPDU info retrieved from status ring TLV
 * @rx_mon_stats: monitor mode status/destination ring PPDU and MPDU count
 *
 * Update status ring PPDU start and end count. Keep track TLV state on
 * PPDU start and end to find out if start and end is matching. Keep
 * track missing PPDU start and end count. Keep track matching PPDU
 * start and end count.
 *
 * Return: None
 */
static inline void
dp_rx_mon_update_dbg_ppdu_stats(struct hal_rx_ppdu_info *ppdu_info,
				struct cdp_pdev_mon_stats *rx_mon_stats)
{
	if (ppdu_info->rx_state ==
		HAL_RX_MON_PPDU_START) {
		rx_mon_stats->status_ppdu_start++;
		if (rx_mon_stats->status_ppdu_state
			!= CDP_MON_PPDU_END)
			rx_mon_stats->status_ppdu_end_mis++;
		rx_mon_stats->status_ppdu_state
			= CDP_MON_PPDU_START;
	} else if (ppdu_info->rx_state ==
		HAL_RX_MON_PPDU_END) {
		rx_mon_stats->status_ppdu_end++;
		if (rx_mon_stats->status_ppdu_state
			!= CDP_MON_PPDU_START)
			rx_mon_stats->status_ppdu_start_mis++;
		else
			rx_mon_stats->status_ppdu_compl++;
		rx_mon_stats->status_ppdu_state
			= CDP_MON_PPDU_END;
	}
}

/*
 * dp_rx_mon_init_dbg_ppdu_stats() - initialization for monitor mode stats
 * @ppdu_info: HAL RX PPDU info retrieved from status ring TLV
 * @rx_mon_stats: monitor mode status/destination ring PPDU and MPDU count
 *
 * Return: None
 */
static inline void
dp_rx_mon_init_dbg_ppdu_stats(struct hal_rx_ppdu_info *ppdu_info,
			      struct cdp_pdev_mon_stats *rx_mon_stats)
{
	ppdu_info->rx_state = HAL_RX_MON_PPDU_END;
	rx_mon_stats->status_ppdu_state
		= CDP_MON_PPDU_END;
}

#else
static inline void
dp_rx_mon_update_dbg_ppdu_stats(struct hal_rx_ppdu_info *ppdu_info,
				struct cdp_pdev_mon_stats *rx_mon_stats)
{
}

static inline void
dp_rx_mon_init_dbg_ppdu_stats(struct hal_rx_ppdu_info *ppdu_info,
			      struct cdp_pdev_mon_stats *rx_mon_stats)
{
}

#endif
#endif
