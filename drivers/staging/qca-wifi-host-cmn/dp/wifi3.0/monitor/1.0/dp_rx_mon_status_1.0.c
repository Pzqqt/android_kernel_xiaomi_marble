/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "hal_hw_headers.h"
#include "dp_types.h"
#include "dp_rx.h"
#include "dp_peer.h"
#include "hal_rx.h"
#include "hal_api.h"
#include "qdf_trace.h"
#include "qdf_nbuf.h"
#include "hal_api_mon.h"
#include "dp_internal.h"
#include "qdf_mem.h"   /* qdf_mem_malloc,free */
#include "dp_htt.h"
#include "dp_mon.h"
#include "dp_rx_mon.h"
#include "htt.h"
#include <dp_mon_1.0.h>
#include <dp_rx_mon_1.0.h>

#ifdef FEATURE_PERPKT_INFO
#include "dp_ratetable.h"
#endif

static inline
QDF_STATUS dp_rx_mon_status_buffers_replenish(struct dp_soc *dp_soc,
					      uint32_t mac_id,
					      struct dp_srng *dp_rxdma_srng,
					      struct rx_desc_pool *rx_desc_pool,
					      uint32_t num_req_buffers,
					      union dp_rx_desc_list_elem_t **desc_list,
					      union dp_rx_desc_list_elem_t **tail,
					      uint8_t owner);

/**
 * dp_rx_mon_handle_status_buf_done () - Handle status buf DMA not done
 *
 * @pdev: DP pdev handle
 * @mon_status_srng: Monitor status SRNG
 *
 * As per MAC team's suggestion, If HP + 2 entry's DMA done is set,
 * skip HP + 1 entry and start processing in next interrupt.
 * If HP + 2 entry's DMA done is not set, poll onto HP + 1 entry
 * for it's DMA done TLV to be set.
 *
 * Return: enum dp_mon_reap_status
 */
enum dp_mon_reap_status
dp_rx_mon_handle_status_buf_done(struct dp_pdev *pdev,
				 void *mon_status_srng)
{
	struct dp_soc *soc = pdev->soc;
	hal_soc_handle_t hal_soc;
	void *ring_entry;
	struct hal_buf_info hbi;
	qdf_nbuf_t status_nbuf;
	struct dp_rx_desc *rx_desc;
	void *rx_tlv;
	QDF_STATUS buf_status;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	hal_soc = soc->hal_soc;

	ring_entry = hal_srng_src_peek_n_get_next_next(hal_soc,
						       mon_status_srng);
	if (!ring_entry) {
		dp_rx_mon_status_debug("%pK: Monitor status ring entry is NULL for SRNG: %pK",
				       soc, mon_status_srng);
		return DP_MON_STATUS_NO_DMA;
	}

	hal_rx_buf_cookie_rbm_get(soc->hal_soc, (uint32_t *)ring_entry,
				  &hbi);
	rx_desc = dp_rx_cookie_2_va_mon_status(soc, hbi.sw_cookie);

	qdf_assert_always(rx_desc);

	status_nbuf = rx_desc->nbuf;

	qdf_nbuf_sync_for_cpu(soc->osdev, status_nbuf,
			      QDF_DMA_FROM_DEVICE);

	rx_tlv = qdf_nbuf_data(status_nbuf);
	buf_status = hal_get_rx_status_done(rx_tlv);

	/* If status buffer DMA is not done,
	 * 1. As per MAC team's suggestion, If HP + 2 entry's DMA done is set,
	 * replenish HP + 1 entry and start processing in next interrupt.
	 * 2. If HP + 2 entry's DMA done is not set
	 * hold on to mon destination ring.
	 */
	if (buf_status != QDF_STATUS_SUCCESS) {
		dp_err_rl("Monitor status ring: DMA is not done "
			     "for nbuf: %pK", status_nbuf);
		mon_pdev->rx_mon_stats.tlv_tag_status_err++;
		return DP_MON_STATUS_NO_DMA;
	}

	mon_pdev->rx_mon_stats.status_buf_done_war++;

	return DP_MON_STATUS_REPLENISH;
}

#ifdef WLAN_RX_PKT_CAPTURE_ENH
#include "dp_rx_mon_feature.h"
#else
static QDF_STATUS
dp_rx_handle_enh_capture(struct dp_soc *soc, struct dp_pdev *pdev,
			 struct hal_rx_ppdu_info *ppdu_info)
{
	return QDF_STATUS_SUCCESS;
}

static void
dp_rx_mon_enh_capture_process(struct dp_pdev *pdev, uint32_t tlv_status,
			      qdf_nbuf_t status_nbuf,
			      struct hal_rx_ppdu_info *ppdu_info,
			      bool *nbuf_used)
{
}
#endif

#ifdef WLAN_TX_PKT_CAPTURE_ENH
#include "dp_rx_mon_feature.h"
#else
static QDF_STATUS
dp_send_ack_frame_to_stack(struct dp_soc *soc,
			   struct dp_pdev *pdev,
			   struct hal_rx_ppdu_info *ppdu_info)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(HTT_UL_OFDMA_USER_INFO_V0_W0_VALID_M)
static inline void
dp_rx_ul_ofdma_ru_size_to_width(
	uint32_t ru_size,
	uint32_t *ru_width)
{
	uint32_t width;

	width = 0;
	switch (ru_size) {
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_26:
		width = 1;
		break;
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_52:
		width = 2;
		break;
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_106:
		width = 4;
		break;
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_242:
		width = 9;
		break;
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_484:
		width = 18;
		break;
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_996:
		width = 37;
		break;
	case HTT_UL_OFDMA_V0_RU_SIZE_RU_996x2:
		width = 74;
		break;
	default:
		dp_rx_mon_status_err("RU size to width convert err");
		break;
	}
	*ru_width = width;
}

static inline void
dp_rx_mon_handle_mu_ul_info(struct hal_rx_ppdu_info *ppdu_info)
{
	struct mon_rx_user_status *mon_rx_user_status;
	uint32_t num_users;
	uint32_t i;
	uint32_t mu_ul_user_v0_word0;
	uint32_t mu_ul_user_v0_word1;
	uint32_t ru_width;
	uint32_t ru_size;

	if (!(ppdu_info->rx_status.reception_type == HAL_RX_TYPE_MU_OFDMA ||
	      ppdu_info->rx_status.reception_type == HAL_RX_TYPE_MU_MIMO))
		return;

	num_users = ppdu_info->com_info.num_users;
	if (num_users > HAL_MAX_UL_MU_USERS)
		num_users = HAL_MAX_UL_MU_USERS;
	for (i = 0; i < num_users; i++) {
		mon_rx_user_status = &ppdu_info->rx_user_status[i];
		mu_ul_user_v0_word0 =
			mon_rx_user_status->mu_ul_user_v0_word0;
		mu_ul_user_v0_word1 =
			mon_rx_user_status->mu_ul_user_v0_word1;

		if (HTT_UL_OFDMA_USER_INFO_V0_W0_VALID_GET(
			mu_ul_user_v0_word0) &&
			!HTT_UL_OFDMA_USER_INFO_V0_W0_VER_GET(
			mu_ul_user_v0_word0)) {
			mon_rx_user_status->mcs =
				HTT_UL_OFDMA_USER_INFO_V0_W1_MCS_GET(
				mu_ul_user_v0_word1);
			mon_rx_user_status->nss =
				HTT_UL_OFDMA_USER_INFO_V0_W1_NSS_GET(
				mu_ul_user_v0_word1) + 1;

			mon_rx_user_status->mu_ul_info_valid = 1;
			mon_rx_user_status->ofdma_ru_start_index =
				HTT_UL_OFDMA_USER_INFO_V0_W1_RU_START_GET(
				mu_ul_user_v0_word1);

			ru_size =
				HTT_UL_OFDMA_USER_INFO_V0_W1_RU_SIZE_GET(
				mu_ul_user_v0_word1);
			dp_rx_ul_ofdma_ru_size_to_width(ru_size, &ru_width);
			mon_rx_user_status->ofdma_ru_width = ru_width;
			mon_rx_user_status->ofdma_ru_size = ru_size;
		}
	}
}
#else
static inline void
dp_rx_mon_handle_mu_ul_info(struct hal_rx_ppdu_info *ppdu_info)
{
}
#endif

#ifdef QCA_SUPPORT_SCAN_SPCL_VAP_STATS
/**
 * dp_rx_mon_update_scan_spcl_vap_stats() - Update special vap stats
 * @pdev: dp pdev context
 * @ppdu_info: ppdu info structure from ppdu ring
 *
 * Return: none
 */
static inline void
dp_rx_mon_update_scan_spcl_vap_stats(struct dp_pdev *pdev,
				     struct hal_rx_ppdu_info *ppdu_info)
{
	struct mon_rx_user_status *rx_user_status = NULL;
	struct dp_mon_pdev *mon_pdev = NULL;
	struct dp_mon_vdev *mon_vdev = NULL;
	uint32_t num_users = 0;
	uint32_t user = 0;

	mon_pdev = pdev->monitor_pdev;
	if (!mon_pdev || !mon_pdev->mvdev)
		return;

	mon_vdev = mon_pdev->mvdev->monitor_vdev;
	if (!mon_vdev || !mon_vdev->scan_spcl_vap_stats)
		return;

	num_users = ppdu_info->com_info.num_users;
	for (user = 0; user < num_users; user++) {
		rx_user_status =  &ppdu_info->rx_user_status[user];
		mon_vdev->scan_spcl_vap_stats->rx_ok_pkts +=
				rx_user_status->mpdu_cnt_fcs_ok;
		mon_vdev->scan_spcl_vap_stats->rx_ok_bytes +=
				rx_user_status->mpdu_ok_byte_count;
		mon_vdev->scan_spcl_vap_stats->rx_err_pkts +=
				rx_user_status->mpdu_cnt_fcs_err;
		mon_vdev->scan_spcl_vap_stats->rx_err_bytes +=
				rx_user_status->mpdu_err_byte_count;
	}
	mon_vdev->scan_spcl_vap_stats->rx_mgmt_pkts +=
				ppdu_info->frm_type_info.rx_mgmt_cnt;
	mon_vdev->scan_spcl_vap_stats->rx_ctrl_pkts +=
				ppdu_info->frm_type_info.rx_ctrl_cnt;
	mon_vdev->scan_spcl_vap_stats->rx_data_pkts +=
				ppdu_info->frm_type_info.rx_data_cnt;
}
#else
static inline void
dp_rx_mon_update_scan_spcl_vap_stats(struct dp_pdev *pdev,
				     struct hal_rx_ppdu_info *ppdu_info)
{
}
#endif

/**
 * dp_rx_mon_status_process_tlv() - Process status TLV in status
 *	buffer on Rx status Queue posted by status SRNG processing.
 * @soc: core txrx main context
 * @int_ctx: interrupt context
 * @mac_id: mac_id which is one of 3 mac_ids _ring
 * @quota: amount of work which can be done
 *
 * Return: none
 */
static inline void
dp_rx_mon_status_process_tlv(struct dp_soc *soc, struct dp_intr *int_ctx,
			     uint32_t mac_id, uint32_t quota)
{
	struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	struct hal_rx_ppdu_info *ppdu_info;
	qdf_nbuf_t status_nbuf;
	uint8_t *rx_tlv;
	uint8_t *rx_tlv_start;
	uint32_t tlv_status = HAL_TLV_STATUS_BUF_DONE;
	QDF_STATUS enh_log_status = QDF_STATUS_SUCCESS;
	struct cdp_pdev_mon_stats *rx_mon_stats;
	int smart_mesh_status;
	enum WDI_EVENT pktlog_mode = WDI_NO_VAL;
	bool nbuf_used;
	uint32_t rx_enh_capture_mode;
	struct dp_mon_soc *mon_soc = soc->monitor_soc;
	struct dp_mon_pdev *mon_pdev;

	if (!pdev) {
		dp_rx_mon_status_debug("%pK: pdev is null for mac_id = %d", soc,
				       mac_id);
		return;
	}

	mon_pdev = pdev->monitor_pdev;
	ppdu_info = &mon_pdev->ppdu_info;
	rx_mon_stats = &mon_pdev->rx_mon_stats;

	if (mon_pdev->mon_ppdu_status != DP_PPDU_STATUS_START)
		return;

	rx_enh_capture_mode = mon_pdev->rx_enh_capture_mode;

	while (!qdf_nbuf_is_queue_empty(&mon_pdev->rx_status_q)) {

		status_nbuf = qdf_nbuf_queue_remove(&mon_pdev->rx_status_q);

		if (!status_nbuf)
			return;

		rx_tlv = qdf_nbuf_data(status_nbuf);
		rx_tlv_start = rx_tlv;
		nbuf_used = false;

		if ((mon_pdev->mvdev) || (mon_pdev->enhanced_stats_en) ||
		    (mon_pdev->mcopy_mode) || (dp_cfr_rcc_mode_status(pdev)) ||
		    (rx_enh_capture_mode != CDP_RX_ENH_CAPTURE_DISABLED)) {
			do {
				tlv_status = hal_rx_status_get_tlv_info(rx_tlv,
						ppdu_info, pdev->soc->hal_soc,
						status_nbuf);

				dp_rx_mon_update_dbg_ppdu_stats(ppdu_info,
								rx_mon_stats);

				dp_rx_mon_enh_capture_process(pdev, tlv_status,
					status_nbuf, ppdu_info,
					&nbuf_used);

				dp_rx_mcopy_process_ppdu_info(pdev,
							      ppdu_info,
							      tlv_status);

				rx_tlv = hal_rx_status_get_next_tlv(rx_tlv,
						mon_pdev->is_tlv_hdr_64_bit);

				if ((rx_tlv - rx_tlv_start) >=
					RX_MON_STATUS_BUF_SIZE)
					break;

			} while ((tlv_status == HAL_TLV_STATUS_PPDU_NOT_DONE) ||
				 (tlv_status == HAL_TLV_STATUS_HEADER) ||
				 (tlv_status == HAL_TLV_STATUS_MPDU_END) ||
				 (tlv_status == HAL_TLV_STATUS_MSDU_END));
		}
		if (mon_pdev->dp_peer_based_pktlog) {
			dp_rx_process_peer_based_pktlog(soc, ppdu_info,
							status_nbuf,
							pdev->pdev_id);
		} else {
			if (mon_pdev->rx_pktlog_mode == DP_RX_PKTLOG_FULL)
				pktlog_mode = WDI_EVENT_RX_DESC;
			else if (mon_pdev->rx_pktlog_mode == DP_RX_PKTLOG_LITE)
				pktlog_mode = WDI_EVENT_LITE_RX;

			if (pktlog_mode != WDI_NO_VAL)
				dp_wdi_event_handler(pktlog_mode, soc,
						     status_nbuf,
						     HTT_INVALID_PEER,
						     WDI_NO_VAL, pdev->pdev_id);
		}

		/* smart monitor vap and m_copy cannot co-exist */
		if (ppdu_info->rx_status.monitor_direct_used &&
		    mon_pdev->neighbour_peers_added &&
		    mon_pdev->mvdev) {
			smart_mesh_status = dp_rx_handle_smart_mesh_mode(soc,
						pdev, ppdu_info, status_nbuf);
			if (smart_mesh_status)
				qdf_nbuf_free(status_nbuf);
		} else if (qdf_unlikely(mon_pdev->mcopy_mode)) {
			dp_rx_process_mcopy_mode(soc, pdev,
						 ppdu_info, tlv_status,
						 status_nbuf);
		} else if (rx_enh_capture_mode != CDP_RX_ENH_CAPTURE_DISABLED) {
			if (!nbuf_used)
				qdf_nbuf_free(status_nbuf);

			if (tlv_status == HAL_TLV_STATUS_PPDU_DONE)
				enh_log_status =
				dp_rx_handle_enh_capture(soc,
							 pdev, ppdu_info);
		} else {
			qdf_nbuf_free(status_nbuf);
		}

		if (tlv_status == HAL_TLV_STATUS_PPDU_NON_STD_DONE) {
			dp_rx_mon_deliver_non_std(soc, mac_id);
		} else if (tlv_status == HAL_TLV_STATUS_PPDU_DONE) {
			rx_mon_stats->status_ppdu_done++;
			dp_rx_mon_handle_mu_ul_info(ppdu_info);

			if (mon_pdev->tx_capture_enabled
			    != CDP_TX_ENH_CAPTURE_DISABLED)
				dp_send_ack_frame_to_stack(soc, pdev,
							   ppdu_info);

			if (mon_pdev->enhanced_stats_en ||
			    mon_pdev->mcopy_mode ||
			    mon_pdev->neighbour_peers_added)
				dp_rx_handle_ppdu_stats(soc, pdev, ppdu_info);
			else if (dp_cfr_rcc_mode_status(pdev))
				dp_rx_handle_cfr(soc, pdev, ppdu_info);

			mon_pdev->mon_ppdu_status = DP_PPDU_STATUS_DONE;

			/* Collect spcl vap stats if configured */
			if (mon_pdev->scan_spcl_vap_configured)
				dp_rx_mon_update_scan_spcl_vap_stats(pdev,
								     ppdu_info);

			/*
			* if chan_num is not fetched correctly from ppdu RX TLV,
			 * get it from pdev saved.
			 */
			if (qdf_unlikely(mon_pdev->ppdu_info.rx_status.chan_num == 0))
				mon_pdev->ppdu_info.rx_status.chan_num =
							mon_pdev->mon_chan_num;
			/*
			 * if chan_freq is not fetched correctly from ppdu RX TLV,
			 * get it from pdev saved.
			 */
			if (qdf_unlikely(mon_pdev->ppdu_info.rx_status.chan_freq == 0)) {
				mon_pdev->ppdu_info.rx_status.chan_freq =
					mon_pdev->mon_chan_freq;
			}

			if (!mon_soc->full_mon_mode)
				dp_rx_mon_dest_process(soc, int_ctx, mac_id,
						       quota);

			mon_pdev->mon_ppdu_status = DP_PPDU_STATUS_START;
		}
	}
	return;
}

/*
 * dp_rx_mon_status_srng_process() - Process monitor status ring
 *	post the status ring buffer to Rx status Queue for later
 *	processing when status ring is filled with status TLV.
 *	Allocate a new buffer to status ring if the filled buffer
 *	is posted.
 * @soc: core txrx main context
 * @int_ctx: interrupt context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @quota: No. of ring entry that can be serviced in one shot.

 * Return: uint32_t: No. of ring entry that is processed.
 */
static inline uint32_t
dp_rx_mon_status_srng_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			      uint32_t mac_id, uint32_t quota)
{
	struct dp_pdev *pdev = dp_get_pdev_for_lmac_id(soc, mac_id);
	hal_soc_handle_t hal_soc;
	void *mon_status_srng;
	void *rxdma_mon_status_ring_entry;
	QDF_STATUS status;
	enum dp_mon_reap_status reap_status;
	uint32_t work_done = 0;
	struct dp_mon_pdev *mon_pdev;

	if (!pdev) {
		dp_rx_mon_status_debug("%pK: pdev is null for mac_id = %d",
				       soc, mac_id);
		return work_done;
	}

	mon_pdev = pdev->monitor_pdev;

	mon_status_srng = soc->rxdma_mon_status_ring[mac_id].hal_srng;

	qdf_assert(mon_status_srng);
	if (!mon_status_srng || !hal_srng_initialized(mon_status_srng)) {

		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s %d : HAL Monitor Status Ring Init Failed -- %pK",
			__func__, __LINE__, mon_status_srng);
		return work_done;
	}

	hal_soc = soc->hal_soc;

	qdf_assert(hal_soc);

	if (qdf_unlikely(dp_srng_access_start(int_ctx, soc, mon_status_srng)))
		goto done;

	/* mon_status_ring_desc => WBM_BUFFER_RING STRUCT =>
	 * BUFFER_ADDR_INFO STRUCT
	 */
	while (qdf_likely((rxdma_mon_status_ring_entry =
		hal_srng_src_peek_n_get_next(hal_soc, mon_status_srng))
			&& quota--)) {
		struct hal_buf_info hbi;
		qdf_nbuf_t status_nbuf;
		struct dp_rx_desc *rx_desc;
		uint8_t *status_buf;
		qdf_dma_addr_t paddr;
		uint64_t buf_addr;
		struct rx_desc_pool *rx_desc_pool;

		rx_desc_pool = &soc->rx_desc_status[mac_id];
		buf_addr =
			(HAL_RX_BUFFER_ADDR_31_0_GET(
				rxdma_mon_status_ring_entry) |
			((uint64_t)(HAL_RX_BUFFER_ADDR_39_32_GET(
				rxdma_mon_status_ring_entry)) << 32));

		if (qdf_likely(buf_addr)) {

			hal_rx_buf_cookie_rbm_get(soc->hal_soc,
					(uint32_t *)rxdma_mon_status_ring_entry,
					&hbi);
			rx_desc = dp_rx_cookie_2_va_mon_status(soc,
						hbi.sw_cookie);

			qdf_assert_always(rx_desc);

			status_nbuf = rx_desc->nbuf;

			qdf_nbuf_sync_for_cpu(soc->osdev, status_nbuf,
				QDF_DMA_FROM_DEVICE);

			status_buf = qdf_nbuf_data(status_nbuf);

			status = hal_get_rx_status_done(status_buf);

			if (status != QDF_STATUS_SUCCESS) {
				uint32_t hp, tp;
				hal_get_sw_hptp(hal_soc, mon_status_srng,
						&tp, &hp);
				dp_info_rl("tlv tag status error hp:%u, tp:%u",
					   hp, tp);

				/* RxDMA status done bit might not be set even
				 * though tp is moved by HW.
				 */

				/* If done status is missing:
				 * 1. As per MAC team's suggestion,
				 *    when HP + 1 entry is peeked and if DMA
				 *    is not done and if HP + 2 entry's DMA done
				 *    is set. skip HP + 1 entry and
				 *    start processing in next interrupt.
				 * 2. If HP + 2 entry's DMA done is not set,
				 *    poll onto HP + 1 entry DMA done to be set.
				 *    Check status for same buffer for next time
				 *    dp_rx_mon_status_srng_process
				 */
				reap_status = dp_rx_mon_handle_status_buf_done(pdev,
									mon_status_srng);
				if (reap_status == DP_MON_STATUS_NO_DMA)
					continue;
				else if (reap_status == DP_MON_STATUS_REPLENISH) {
					if (!rx_desc->unmapped) {
						qdf_nbuf_unmap_nbytes_single(
							soc->osdev, status_nbuf,
							QDF_DMA_FROM_DEVICE,
							rx_desc_pool->buf_size);
						rx_desc->unmapped = 1;
					}
					qdf_nbuf_free(status_nbuf);
					goto buf_replenish;
				}
			}
			qdf_nbuf_set_pktlen(status_nbuf,
					    RX_MON_STATUS_BUF_SIZE);

			if (!rx_desc->unmapped) {
				qdf_nbuf_unmap_nbytes_single(soc->osdev, status_nbuf,
							     QDF_DMA_FROM_DEVICE,
							     rx_desc_pool->buf_size);
				rx_desc->unmapped = 1;
			}

			/* Put the status_nbuf to queue */
			qdf_nbuf_queue_add(&mon_pdev->rx_status_q, status_nbuf);

		} else {
			union dp_rx_desc_list_elem_t *desc_list = NULL;
			union dp_rx_desc_list_elem_t *tail = NULL;
			uint32_t num_alloc_desc;

			num_alloc_desc = dp_rx_get_free_desc_list(soc, mac_id,
							rx_desc_pool,
							1,
							&desc_list,
							&tail);
			/*
			 * No free descriptors available
			 */
			if (qdf_unlikely(num_alloc_desc == 0)) {
				work_done++;
				break;
			}

			rx_desc = &desc_list->rx_desc;
		}

buf_replenish:
		status_nbuf = dp_rx_nbuf_prepare(soc, pdev);

		/*
		 * qdf_nbuf alloc or map failed,
		 * free the dp rx desc to free list,
		 * fill in NULL dma address at current HP entry,
		 * keep HP in mon_status_ring unchanged,
		 * wait next time dp_rx_mon_status_srng_process
		 * to fill in buffer at current HP.
		 */
		if (qdf_unlikely(!status_nbuf)) {
			union dp_rx_desc_list_elem_t *desc_list = NULL;
			union dp_rx_desc_list_elem_t *tail = NULL;
			struct rx_desc_pool *rx_desc_pool;

			rx_desc_pool = &soc->rx_desc_status[mac_id];

			dp_info_rl("fail to allocate or map qdf_nbuf");
			dp_rx_add_to_free_desc_list(&desc_list,
						&tail, rx_desc);
			dp_rx_add_desc_list_to_free_list(soc, &desc_list,
						&tail, mac_id, rx_desc_pool);

			hal_rxdma_buff_addr_info_set(
				hal_soc, rxdma_mon_status_ring_entry,
				0, 0,
				HAL_RX_BUF_RBM_SW3_BM(soc->wbm_sw0_bm_id));
			work_done++;
			break;
		}

		paddr = qdf_nbuf_get_frag_paddr(status_nbuf, 0);

		rx_desc->nbuf = status_nbuf;
		rx_desc->in_use = 1;
		rx_desc->unmapped = 0;

		hal_rxdma_buff_addr_info_set(hal_soc,
					     rxdma_mon_status_ring_entry,
					     paddr, rx_desc->cookie,
					     HAL_RX_BUF_RBM_SW3_BM(soc->wbm_sw0_bm_id));

		hal_srng_src_get_next(hal_soc, mon_status_srng);
		work_done++;
	}
done:

	dp_srng_access_end(int_ctx, soc, mon_status_srng);

	return work_done;

}

uint32_t
dp_rx_mon_status_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			 uint32_t mac_id, uint32_t quota)
{
	uint32_t work_done;

	work_done = dp_rx_mon_status_srng_process(soc, int_ctx, mac_id, quota);
	quota -= work_done;
	dp_rx_mon_status_process_tlv(soc, int_ctx, mac_id, quota);

	return work_done;
}

QDF_STATUS
dp_rx_pdev_mon_status_buffers_alloc(struct dp_pdev *pdev, uint32_t mac_id)
{
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct dp_srng *mon_status_ring;
	uint32_t num_entries;
	struct rx_desc_pool *rx_desc_pool;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;
	union dp_rx_desc_list_elem_t *desc_list = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;

	soc_cfg_ctx = soc->wlan_cfg_ctx;
	mon_status_ring = &soc->rxdma_mon_status_ring[mac_id];

	num_entries = mon_status_ring->num_entries;

	rx_desc_pool = &soc->rx_desc_status[mac_id];

	dp_debug("Mon RX Desc Pool[%d] entries=%u",
		 pdev_id, num_entries);

	return dp_rx_mon_status_buffers_replenish(soc, mac_id, mon_status_ring,
						  rx_desc_pool, num_entries,
						  &desc_list, &tail,
						  HAL_RX_BUF_RBM_SW3_BM(soc->wbm_sw0_bm_id));
}

QDF_STATUS
dp_rx_pdev_mon_status_desc_pool_alloc(struct dp_pdev *pdev, uint32_t mac_id)
{
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct dp_srng *mon_status_ring;
	uint32_t num_entries;
	struct rx_desc_pool *rx_desc_pool;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;

	soc_cfg_ctx = soc->wlan_cfg_ctx;
	mon_status_ring = &soc->rxdma_mon_status_ring[mac_id];

	num_entries = mon_status_ring->num_entries;

	rx_desc_pool = &soc->rx_desc_status[mac_id];

	dp_debug("Mon RX Desc Pool[%d] entries=%u", pdev_id, num_entries);

	rx_desc_pool->desc_type = DP_RX_DESC_STATUS_TYPE;
	return dp_rx_desc_pool_alloc(soc, num_entries + 1, rx_desc_pool);
}

void
dp_rx_pdev_mon_status_desc_pool_init(struct dp_pdev *pdev, uint32_t mac_id)
{
	uint32_t i;
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct dp_srng *mon_status_ring;
	uint32_t num_entries;
	struct rx_desc_pool *rx_desc_pool;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;
	struct dp_mon_pdev *mon_pdev = pdev->monitor_pdev;

	soc_cfg_ctx = soc->wlan_cfg_ctx;
	mon_status_ring = &soc->rxdma_mon_status_ring[mac_id];

	num_entries = mon_status_ring->num_entries;

	rx_desc_pool = &soc->rx_desc_status[mac_id];

	dp_debug("Mon RX Desc status Pool[%d] init entries=%u",
		 pdev_id, num_entries);

	rx_desc_pool->owner = HAL_RX_BUF_RBM_SW3_BM(soc->wbm_sw0_bm_id);
	rx_desc_pool->buf_size = RX_MON_STATUS_BUF_SIZE;
	rx_desc_pool->buf_alignment = RX_DATA_BUFFER_ALIGNMENT;
	/* Disable frag processing flag */
	dp_rx_enable_mon_dest_frag(rx_desc_pool, false);

	dp_rx_desc_pool_init(soc, mac_id, num_entries + 1, rx_desc_pool);

	qdf_nbuf_queue_init(&mon_pdev->rx_status_q);

	mon_pdev->mon_ppdu_status = DP_PPDU_STATUS_START;

	qdf_mem_zero(&mon_pdev->ppdu_info, sizeof(mon_pdev->ppdu_info));

	/*
	 * Set last_ppdu_id to HAL_INVALID_PPDU_ID in order to avoid ppdu_id
	 * match with '0' ppdu_id from monitor status ring
	 */
	mon_pdev->ppdu_info.com_info.last_ppdu_id = HAL_INVALID_PPDU_ID;

	qdf_mem_zero(&mon_pdev->rx_mon_stats, sizeof(mon_pdev->rx_mon_stats));

	dp_rx_mon_init_dbg_ppdu_stats(&mon_pdev->ppdu_info,
				      &mon_pdev->rx_mon_stats);

	for (i = 0; i < MAX_MU_USERS; i++) {
		qdf_nbuf_queue_init(&mon_pdev->mpdu_q[i]);
		mon_pdev->is_mpdu_hdr[i] = true;
	}

	qdf_mem_zero(mon_pdev->msdu_list,
		     sizeof(mon_pdev->msdu_list[MAX_MU_USERS]));

	mon_pdev->rx_enh_capture_mode = CDP_RX_ENH_CAPTURE_DISABLED;
}

void
dp_rx_pdev_mon_status_desc_pool_deinit(struct dp_pdev *pdev, uint32_t mac_id) {
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_status[mac_id];

	dp_debug("Mon RX Desc status Pool[%d] deinit", pdev_id);

	dp_rx_desc_pool_deinit(soc, rx_desc_pool, mac_id);
}

void
dp_rx_pdev_mon_status_desc_pool_free(struct dp_pdev *pdev, uint32_t mac_id) {
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_status[mac_id];

	dp_debug("Mon RX Status Desc Pool Free pdev[%d]", pdev_id);

	dp_rx_desc_pool_free(soc, rx_desc_pool);
}

void
dp_rx_pdev_mon_status_buffers_free(struct dp_pdev *pdev, uint32_t mac_id)
{
	uint8_t pdev_id = pdev->pdev_id;
	struct dp_soc *soc = pdev->soc;
	struct rx_desc_pool *rx_desc_pool;

	rx_desc_pool = &soc->rx_desc_status[mac_id];

	dp_debug("Mon RX Status Desc Pool Free pdev[%d]", pdev_id);

	dp_rx_desc_nbuf_free(soc, rx_desc_pool);
}

/*
 * dp_rx_buffers_replenish() -  replenish monitor status ring with
 *				rx nbufs called during dp rx
 *				monitor status ring initialization
 *
 * @soc: core txrx main context
 * @mac_id: mac_id which is one of 3 mac_ids
 * @dp_rxdma_srng: dp monitor status circular ring
 * @rx_desc_pool; Pointer to Rx descriptor pool
 * @num_req_buffers: number of buffer to be replenished
 * @desc_list:	list of descs if called from dp rx monitor status
 *		process or NULL during dp rx initialization or
 *		out of buffer interrupt
 * @tail: tail of descs list
 * @owner: who owns the nbuf (host, NSS etc...)
 * Return: return success or failure
 */
static inline
QDF_STATUS dp_rx_mon_status_buffers_replenish(struct dp_soc *dp_soc,
	uint32_t mac_id,
	struct dp_srng *dp_rxdma_srng,
	struct rx_desc_pool *rx_desc_pool,
	uint32_t num_req_buffers,
	union dp_rx_desc_list_elem_t **desc_list,
	union dp_rx_desc_list_elem_t **tail,
	uint8_t owner)
{
	uint32_t num_alloc_desc;
	uint16_t num_desc_to_free = 0;
	uint32_t num_entries_avail;
	uint32_t count = 0;
	int sync_hw_ptr = 1;
	qdf_dma_addr_t paddr;
	qdf_nbuf_t rx_netbuf;
	void *rxdma_ring_entry;
	union dp_rx_desc_list_elem_t *next;
	void *rxdma_srng;
	struct dp_pdev *dp_pdev = dp_get_pdev_for_lmac_id(dp_soc, mac_id);

	if (!dp_pdev) {
		dp_rx_mon_status_debug("%pK: pdev is null for mac_id = %d",
				       dp_soc, mac_id);
		return QDF_STATUS_E_FAILURE;
	}

	rxdma_srng = dp_rxdma_srng->hal_srng;

	qdf_assert(rxdma_srng);

	dp_rx_mon_status_debug("%pK: requested %d buffers for replenish",
			       dp_soc, num_req_buffers);

	/*
	 * if desc_list is NULL, allocate the descs from freelist
	 */
	if (!(*desc_list)) {

		num_alloc_desc = dp_rx_get_free_desc_list(dp_soc, mac_id,
							  rx_desc_pool,
							  num_req_buffers,
							  desc_list,
							  tail);

		if (!num_alloc_desc) {
			dp_rx_mon_status_err("%pK: no free rx_descs in freelist",
					     dp_soc);
			return QDF_STATUS_E_NOMEM;
		}

		dp_rx_mon_status_debug("%pK: %d rx desc allocated", dp_soc,
				       num_alloc_desc);

		num_req_buffers = num_alloc_desc;
	}

	hal_srng_access_start(dp_soc->hal_soc, rxdma_srng);
	num_entries_avail = hal_srng_src_num_avail(dp_soc->hal_soc,
				rxdma_srng, sync_hw_ptr);

	dp_rx_mon_status_debug("%pK: no of available entries in rxdma ring: %d",
			       dp_soc, num_entries_avail);

	if (num_entries_avail < num_req_buffers) {
		num_desc_to_free = num_req_buffers - num_entries_avail;
		num_req_buffers = num_entries_avail;
	}

	while (count <= num_req_buffers) {
		rx_netbuf = dp_rx_nbuf_prepare(dp_soc, dp_pdev);

		/*
		 * qdf_nbuf alloc or map failed,
		 * keep HP in mon_status_ring unchanged,
		 * wait dp_rx_mon_status_srng_process
		 * to fill in buffer at current HP.
		 */
		if (qdf_unlikely(!rx_netbuf)) {
			dp_rx_mon_status_err("%pK: qdf_nbuf allocate or map fail, count %d",
					     dp_soc, count);
			break;
		}

		paddr = qdf_nbuf_get_frag_paddr(rx_netbuf, 0);

		next = (*desc_list)->next;
		rxdma_ring_entry = hal_srng_src_get_cur_hp_n_move_next(
						dp_soc->hal_soc,
						rxdma_srng);

		if (qdf_unlikely(!rxdma_ring_entry)) {
			dp_rx_mon_status_err("%pK: rxdma_ring_entry is NULL, count - %d",
					     dp_soc, count);
			qdf_nbuf_unmap_nbytes_single(dp_soc->osdev, rx_netbuf,
						     QDF_DMA_FROM_DEVICE,
						     rx_desc_pool->buf_size);
			qdf_nbuf_free(rx_netbuf);
			break;
		}

		(*desc_list)->rx_desc.nbuf = rx_netbuf;
		(*desc_list)->rx_desc.in_use = 1;
		(*desc_list)->rx_desc.unmapped = 0;
		count++;

		hal_rxdma_buff_addr_info_set(dp_soc->hal_soc,
					     rxdma_ring_entry, paddr,
					     (*desc_list)->rx_desc.cookie,
					     owner);

		dp_rx_mon_status_debug("%pK: rx_desc=%pK, cookie=%d, nbuf=%pK, paddr=%pK",
				       dp_soc, &(*desc_list)->rx_desc,
				       (*desc_list)->rx_desc.cookie, rx_netbuf,
				       (void *)paddr);

		*desc_list = next;
	}

	hal_srng_access_end(dp_soc->hal_soc, rxdma_srng);

	dp_rx_mon_status_debug("%pK: successfully replenished %d buffers",
			       dp_soc, num_req_buffers);

	dp_rx_mon_status_debug("%pK: %d rx desc added back to free list",
			       dp_soc, num_desc_to_free);

	/*
	 * add any available free desc back to the free list
	 */
	if (*desc_list) {
		dp_rx_add_desc_list_to_free_list(dp_soc, desc_list, tail,
			mac_id, rx_desc_pool);
	}

	return QDF_STATUS_SUCCESS;
}

#if !defined(DISABLE_MON_CONFIG) && defined(MON_ENABLE_DROP_FOR_MAC)
/**
 * dp_mon_status_srng_drop_for_mac() - Drop the mon status ring packets for
 *  a given mac
 * @pdev: DP pdev
 * @mac_id: mac id
 * @quota: maximum number of ring entries that can be processed
 *
 * Return: Number of ring entries reaped
 */
static uint32_t
dp_mon_status_srng_drop_for_mac(struct dp_pdev *pdev, uint32_t mac_id,
				uint32_t quota)
{
	struct dp_soc *soc = pdev->soc;
	void *mon_status_srng;
	hal_soc_handle_t hal_soc;
	void *ring_desc;
	uint32_t reap_cnt = 0;

	if (qdf_unlikely(!soc || !soc->hal_soc))
		return reap_cnt;

	mon_status_srng = soc->rxdma_mon_status_ring[mac_id].hal_srng;

	if (qdf_unlikely(!mon_status_srng ||
			 !hal_srng_initialized(mon_status_srng)))
		return reap_cnt;

	hal_soc = soc->hal_soc;

	if (qdf_unlikely(hal_srng_access_start(hal_soc, mon_status_srng)))
		return reap_cnt;

	while ((ring_desc =
		hal_srng_src_peek_n_get_next(hal_soc, mon_status_srng)) &&
		reap_cnt < MON_DROP_REAP_LIMIT && quota--) {
		uint64_t buf_addr;
		struct hal_buf_info hbi;
		struct dp_rx_desc *rx_desc;
		qdf_nbuf_t status_nbuf;
		uint8_t *status_buf;
		enum dp_mon_reap_status reap_status;
		qdf_dma_addr_t iova;
		struct rx_desc_pool *rx_desc_pool;

		rx_desc_pool = &soc->rx_desc_status[mac_id];

		buf_addr = (HAL_RX_BUFFER_ADDR_31_0_GET(ring_desc) |
		   ((uint64_t)(HAL_RX_BUFFER_ADDR_39_32_GET(ring_desc)) << 32));

		if (qdf_likely(buf_addr)) {
			hal_rx_buf_cookie_rbm_get(soc->hal_soc,
						  (uint32_t *)ring_desc,
						  &hbi);
			rx_desc = dp_rx_cookie_2_va_mon_status(soc,
							       hbi.sw_cookie);

			qdf_assert_always(rx_desc);

			status_nbuf = rx_desc->nbuf;

			qdf_nbuf_sync_for_cpu(soc->osdev, status_nbuf,
					      QDF_DMA_FROM_DEVICE);

			status_buf = qdf_nbuf_data(status_nbuf);

			if (hal_get_rx_status_done(status_buf) !=
			    QDF_STATUS_SUCCESS) {
				/* If done status is missing:
				 * 1. As per MAC team's suggestion,
				 *    when HP + 1 entry is peeked and if DMA
				 *    is not done and if HP + 2 entry's DMA done
				 *    is set. skip HP + 1 entry and
				 *    start processing in next interrupt.
				 * 2. If HP + 2 entry's DMA done is not set,
				 *    poll onto HP + 1 entry DMA done to be set.
				 *    Check status for same buffer for next time
				 *    dp_rx_mon_status_srng_process
				 */
				reap_status =
					dp_rx_mon_handle_status_buf_done(pdev,
							       mon_status_srng);
				if (reap_status == DP_MON_STATUS_NO_DMA)
					break;
			}
			qdf_nbuf_unmap_nbytes_single(soc->osdev, status_nbuf,
						     QDF_DMA_FROM_DEVICE,
						     rx_desc_pool->buf_size);
			qdf_nbuf_free(status_nbuf);
		} else {
			union dp_rx_desc_list_elem_t *rx_desc_elem;

			qdf_spin_lock_bh(&rx_desc_pool->lock);

			if (!rx_desc_pool->freelist) {
				qdf_spin_unlock_bh(&rx_desc_pool->lock);
				break;
			}
			rx_desc_elem = rx_desc_pool->freelist;
			rx_desc_pool->freelist = rx_desc_pool->freelist->next;
			qdf_spin_unlock_bh(&rx_desc_pool->lock);

			rx_desc = &rx_desc_elem->rx_desc;
		}

		status_nbuf = dp_rx_nbuf_prepare(soc, pdev);

		if (qdf_unlikely(!status_nbuf)) {
			union dp_rx_desc_list_elem_t *desc_list = NULL;
			union dp_rx_desc_list_elem_t *tail = NULL;

			dp_info_rl("fail to allocate or map nbuf");
			dp_rx_add_to_free_desc_list(&desc_list, &tail,
						    rx_desc);
			dp_rx_add_desc_list_to_free_list(soc,
							 &desc_list,
							 &tail, mac_id,
							 rx_desc_pool);

			hal_rxdma_buff_addr_info_set(hal_soc, ring_desc, 0, 0,
						     HAL_RX_BUF_RBM_SW3_BM(soc->wbm_sw0_bm_id));
			break;
		}

		iova = qdf_nbuf_get_frag_paddr(status_nbuf, 0);

		rx_desc->nbuf = status_nbuf;
		rx_desc->in_use = 1;

		hal_rxdma_buff_addr_info_set(hal_soc, ring_desc, iova,
					     rx_desc->cookie,
					     HAL_RX_BUF_RBM_SW3_BM(soc->wbm_sw0_bm_id));

		reap_cnt++;
		hal_srng_src_get_next(hal_soc, mon_status_srng);
	}

	hal_srng_access_end(hal_soc, mon_status_srng);

	return reap_cnt;
}

uint32_t dp_mon_drop_packets_for_mac(struct dp_pdev *pdev, uint32_t mac_id,
				     uint32_t quota)
{
	uint32_t work_done;

	work_done = dp_mon_status_srng_drop_for_mac(pdev, mac_id, quota);
	if (!dp_is_rxdma_dst_ring_common(pdev))
		dp_mon_dest_srng_drop_for_mac(pdev, mac_id);

	return work_done;
}
#else
uint32_t dp_mon_drop_packets_for_mac(struct dp_pdev *pdev, uint32_t mac_id,
				     uint32_t quota)
{
	return 0;
}
#endif
