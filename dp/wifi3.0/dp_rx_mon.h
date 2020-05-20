/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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

/*
 * MON_BUF_MIN_ENTRIES macro defines minimum number of network buffers
 * to be refilled in the RXDMA monitor buffer ring at init, remaining
 * buffers are replenished at the time of monitor vap creation
 */
#define MON_BUF_MIN_ENTRIES 64

/*
 * dp_rx_mon_status_process() - Process monitor status ring and
 *			TLV in status ring.
 *
 * @soc: core txrx main context
 * @int_ctx: interrupt context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @quota: No. of ring entry that can be serviced in one shot.

 * Return: uint32_t: No. of ring entry that is processed.
 */
uint32_t
dp_rx_mon_status_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			 uint32_t mac_id, uint32_t quota);

/**
 * dp_rx_mon_dest_process() - Brain of the Rx processing functionality
 *	Called from the bottom half (tasklet/NET_RX_SOFTIRQ)
 * @soc: core txrx main contex
 * @int_ctx: interrupt context
 * @hal_ring: opaque pointer to the HAL Rx Ring, which will be serviced
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements the core of Rx functionality. This is
 * expected to handle only non-error frames.
 *
 * Return: none
 */
void dp_rx_mon_dest_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			    uint32_t mac_id, uint32_t quota);

QDF_STATUS dp_rx_pdev_mon_desc_pool_alloc(struct dp_pdev *pdev);
QDF_STATUS dp_rx_pdev_mon_buffers_alloc(struct dp_pdev *pdev);
void dp_rx_pdev_mon_buffers_free(struct dp_pdev *pdev);
void dp_rx_pdev_mon_desc_pool_init(struct dp_pdev *pdev);
void dp_rx_pdev_mon_desc_pool_deinit(struct dp_pdev *pdev);
void dp_rx_pdev_mon_desc_pool_free(struct dp_pdev *pdev);
void dp_rx_pdev_mon_buf_buffers_free(struct dp_pdev *pdev, uint32_t mac_id);

QDF_STATUS dp_rx_pdev_mon_status_buffers_alloc(struct dp_pdev *pdev,
					       uint32_t mac_id);
QDF_STATUS dp_rx_pdev_mon_status_desc_pool_alloc(struct dp_pdev *pdev,
						 uint32_t mac_id);
void dp_rx_pdev_mon_status_desc_pool_init(struct dp_pdev *pdev,
					  uint32_t mac_id);
void dp_rx_pdev_mon_status_desc_pool_deinit(struct dp_pdev *pdev,
					    uint32_t mac_id);
void dp_rx_pdev_mon_status_desc_pool_free(struct dp_pdev *pdev,
					  uint32_t mac_id);
void dp_rx_pdev_mon_status_buffers_free(struct dp_pdev *pdev, uint32_t mac_id);
QDF_STATUS
dp_rx_pdev_mon_buf_buffers_alloc(struct dp_pdev *pdev, uint32_t mac_id,
				 bool delayed_replenish);

#ifdef QCA_SUPPORT_FULL_MON

/**
 * dp_full_mon_attach() - Full monitor mode attach
 * This API initilises full monitor mode resources
 *
 * @pdev: dp pdev object
 *
 * Return: void
 *
 */
void dp_full_mon_attach(struct dp_pdev *pdev);

/**
 * dp_full_mon_detach() - Full monitor mode attach
 * This API deinitilises full monitor mode resources
 *
 * @pdev: dp pdev object
 *
 * Return: void
 *
 */
void dp_full_mon_detach(struct dp_pdev *pdev);

/**
 * dp_rx_mon_process ()- API to process monitor destination ring for
 * full monitor mode
 *
 * @soc: dp soc handle
 * @int_ctx: interrupt context
 * @mac_id: lmac id
 * @quota: No. of ring entry that can be serviced in one shot.
 */

uint32_t dp_rx_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			   uint32_t mac_id, uint32_t quota);

#else
/**
 * dp_full_mon_attach() - attach full monitor mode resources
 * @pdev: Datapath PDEV handle
 *
 * Return: void
 */
static inline void dp_full_mon_attach(struct dp_pdev *pdev)
{
}

/**
 * dp_full_mon_detach() - detach full monitor mode resources
 * @pdev: Datapath PDEV handle
 *
 * Return: void
 *
 */
static inline void dp_full_mon_detach(struct dp_pdev *pdev)
{
}
#endif

/**
 * dp_reset_monitor_mode() - Disable monitor mode
 * @pdev_handle: Datapath PDEV handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_reset_monitor_mode(struct cdp_soc_t *soc_hdl,
				 uint8_t pdev_id,
				 uint8_t smart_monitor);

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


/**
 * dp_mon_process() - Main monitor mode processing roution.
 * @soc: core txrx main context
 * @int_ctx: interrupt context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @quota: No. of status ring entry that can be serviced in one shot.
 *
 * This call monitor status ring process then monitor
 * destination ring process.
 * Called from the bottom half (tasklet/NET_RX_SOFTIRQ)
 *
 * Return: uint32_t: No. of ring entry that is processed.
 */
uint32_t dp_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			uint32_t mac_id, uint32_t quota);
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

/**
 * dp_mon_buf_delayed_replenish() - Helper routine to replenish monitor dest buf
 * @pdev: DP pdev object
 *
 * Return: None
 */
void dp_mon_buf_delayed_replenish(struct dp_pdev *pdev);

/**
 * dp_rx_mon_link_desc_return() - Return a MPDU link descriptor to HW
 *			      (WBM), following error handling
 *
 * @dp_pdev: core txrx pdev context
 * @buf_addr_info: void pointer to monitor link descriptor buf addr info
 * Return: QDF_STATUS
 */
QDF_STATUS
dp_rx_mon_link_desc_return(struct dp_pdev *dp_pdev,
			   hal_buff_addrinfo_t buf_addr_info,
			   int mac_id);

/**
 * dp_mon_adjust_frag_len() - MPDU and MSDU may spread across
 *				multiple nbufs. This function
 *                              is to return data length in
 *				fragmented buffer
 *
 * @total_len: pointer to remaining data length.
 * @frag_len: pointer to data length in this fragment.
*/
static inline void dp_mon_adjust_frag_len(uint32_t *total_len,
					  uint32_t *frag_len)
{
	if (*total_len >= (RX_MONITOR_BUFFER_SIZE - RX_PKT_TLVS_LEN)) {
		*frag_len = RX_MONITOR_BUFFER_SIZE - RX_PKT_TLVS_LEN;
		*total_len -= *frag_len;
	} else {
		*frag_len = *total_len;
		*total_len = 0;
	}
}

/**
 * dp_rx_cookie_2_mon_link_desc() - Retrieve Link descriptor based on target
 * @pdev: core physical device context
 * @hal_buf_info: structure holding the buffer info
 * mac_id: mac number
 *
 * Return: link descriptor address
 */
static inline
void *dp_rx_cookie_2_mon_link_desc(struct dp_pdev *pdev,
				   struct hal_buf_info buf_info,
				   uint8_t mac_id)
{
	if (pdev->soc->wlan_cfg_ctx->rxdma1_enable)
		return dp_rx_cookie_2_mon_link_desc_va(pdev, &buf_info,
						       mac_id);

	return dp_rx_cookie_2_link_desc_va(pdev->soc, &buf_info);
}

/**
 * dp_rx_monitor_link_desc_return() - Return Link descriptor based on target
 * @pdev: core physical device context
 * @p_last_buf_addr_info: MPDU Link descriptor
 * mac_id: mac number
 *
 * Return: QDF_STATUS
 */
static inline
QDF_STATUS dp_rx_monitor_link_desc_return(struct dp_pdev *pdev,
					  hal_buff_addrinfo_t
					  p_last_buf_addr_info,
					  uint8_t mac_id, uint8_t bm_action)
{
	if (pdev->soc->wlan_cfg_ctx->rxdma1_enable)
		return dp_rx_mon_link_desc_return(pdev, p_last_buf_addr_info,
						  mac_id);

	return dp_rx_link_desc_return_by_addr(pdev->soc, p_last_buf_addr_info,
				      bm_action);
}

/**
 * dp_rxdma_get_mon_dst_ring() - Return the pointer to rxdma_err_dst_ring
 *					or mon_dst_ring based on the target
 * @pdev: core physical device context
 * @mac_for_pdev: mac_id number
 *
 * Return: ring address
 */
static inline
void *dp_rxdma_get_mon_dst_ring(struct dp_pdev *pdev,
				uint8_t mac_for_pdev)
{
	if (pdev->soc->wlan_cfg_ctx->rxdma1_enable)
		return pdev->soc->rxdma_mon_dst_ring[mac_for_pdev].hal_srng;

	return pdev->soc->rxdma_err_dst_ring[mac_for_pdev].hal_srng;
}

/**
 * dp_rxdma_get_mon_buf_ring() - Return monitor buf ring address
 *				    based on target
 * @pdev: core physical device context
 * @mac_for_pdev: mac id number
 *
 * Return: ring address
 */
static inline
struct dp_srng *dp_rxdma_get_mon_buf_ring(struct dp_pdev *pdev,
					  uint8_t mac_for_pdev)
{
	if (pdev->soc->wlan_cfg_ctx->rxdma1_enable)
		return &pdev->soc->rxdma_mon_buf_ring[mac_for_pdev];

	/* For MCL there is only 1 rx refill ring */
	return &pdev->soc->rx_refill_buf_ring[0];
}

/**
 * dp_rx_get_mon_desc_pool() - Return monitor descriptor pool
 *			       based on target
 * @soc: soc handle
 * @mac_id: mac id number
 * @pdev_id: pdev id number
 *
 * Return: descriptor pool address
 */
static inline
struct rx_desc_pool *dp_rx_get_mon_desc_pool(struct dp_soc *soc,
					     uint8_t mac_id,
					     uint8_t pdev_id)
{
	if (soc->wlan_cfg_ctx->rxdma1_enable)
		return &soc->rx_desc_mon[mac_id];

	return &soc->rx_desc_buf[pdev_id];
}

/**
 * dp_rx_get_mon_desc() - Return Rx descriptor based on target
 * @soc: soc handle
 * @cookie: cookie value
 *
 * Return: Rx descriptor
 */
static inline
struct dp_rx_desc *dp_rx_get_mon_desc(struct dp_soc *soc,
				      uint32_t cookie)
{
	if (soc->wlan_cfg_ctx->rxdma1_enable)
		return dp_rx_cookie_2_va_mon_buf(soc, cookie);

	return dp_rx_cookie_2_va_rxdma_buf(soc, cookie);
}

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
		ppdu_info->rx_state = HAL_RX_MON_PPDU_RESET;
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
		ppdu_info->rx_state = HAL_RX_MON_PPDU_RESET;
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
