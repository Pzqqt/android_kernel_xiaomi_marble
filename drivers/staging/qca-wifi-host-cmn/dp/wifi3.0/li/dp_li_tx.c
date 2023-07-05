/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "cdp_txrx_cmn_struct.h"
#include "dp_types.h"
#include "dp_tx.h"
#include "dp_li_tx.h"
#include "dp_tx_desc.h"
#include <dp_internal.h>
#include <dp_htt.h>
#include <hal_li_api.h>
#include <hal_li_tx.h>
#include "dp_peer.h"
#ifdef FEATURE_WDS
#include "dp_txrx_wds.h"
#endif
#include "dp_li.h"

extern uint8_t sec_type_map[MAX_CDP_SEC_TYPE];

void dp_tx_comp_get_params_from_hal_desc_li(struct dp_soc *soc,
					    void *tx_comp_hal_desc,
					    struct dp_tx_desc_s **r_tx_desc)
{
	uint8_t pool_id;
	uint32_t tx_desc_id;

	tx_desc_id = hal_tx_comp_get_desc_id(tx_comp_hal_desc);
	pool_id = (tx_desc_id & DP_TX_DESC_ID_POOL_MASK) >>
			DP_TX_DESC_ID_POOL_OS;

	/* Find Tx descriptor */
	*r_tx_desc = dp_tx_desc_find(soc, pool_id,
				     (tx_desc_id & DP_TX_DESC_ID_PAGE_MASK) >>
							DP_TX_DESC_ID_PAGE_OS,
				     (tx_desc_id & DP_TX_DESC_ID_OFFSET_MASK) >>
						DP_TX_DESC_ID_OFFSET_OS);
	/* Pool id is not matching. Error */
	if ((*r_tx_desc)->pool_id != pool_id) {
		dp_tx_comp_alert("Tx Comp pool id %d not matched %d",
				 pool_id, (*r_tx_desc)->pool_id);

		qdf_assert_always(0);
	}

	(*r_tx_desc)->peer_id = hal_tx_comp_get_peer_id(tx_comp_hal_desc);
}

static inline
void dp_tx_process_mec_notify_li(struct dp_soc *soc, uint8_t *status)
{
	struct dp_vdev *vdev;
	uint8_t vdev_id;
	uint32_t *htt_desc = (uint32_t *)status;

	/*
	 * Get vdev id from HTT status word in case of MEC
	 * notification
	 */
	vdev_id = HTT_TX_WBM_COMPLETION_V2_VDEV_ID_GET(htt_desc[3]);
	if (qdf_unlikely(vdev_id >= MAX_VDEV_CNT))
		return;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
				     DP_MOD_ID_HTT_COMP);
	if (!vdev)
		return;
	dp_tx_mec_handler(vdev, status);
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_HTT_COMP);
}

void dp_tx_process_htt_completion_li(struct dp_soc *soc,
				     struct dp_tx_desc_s *tx_desc,
				     uint8_t *status,
				     uint8_t ring_id)
{
	uint8_t tx_status;
	struct dp_pdev *pdev;
	struct dp_vdev *vdev = NULL;
	struct hal_tx_completion_status ts = {0};
	uint32_t *htt_desc = (uint32_t *)status;
	struct dp_peer *peer;
	struct cdp_tid_tx_stats *tid_stats = NULL;
	struct htt_soc *htt_handle;
	uint8_t vdev_id;

	tx_status = HTT_TX_WBM_COMPLETION_V2_TX_STATUS_GET(htt_desc[0]);
	htt_handle = (struct htt_soc *)soc->htt_handle;
	htt_wbm_event_record(htt_handle->htt_logger_handle, tx_status, status);

	/*
	 * There can be scenario where WBM consuming descriptor enqueued
	 * from TQM2WBM first and TQM completion can happen before MEC
	 * notification comes from FW2WBM. Avoid access any field of tx
	 * descriptor in case of MEC notify.
	 */
	if (tx_status == HTT_TX_FW2WBM_TX_STATUS_MEC_NOTIFY)
		return dp_tx_process_mec_notify_li(soc, status);

	/*
	 * If the descriptor is already freed in vdev_detach,
	 * continue to next descriptor
	 */
	if (qdf_unlikely(!tx_desc->flags)) {
		dp_tx_comp_info_rl("Descriptor freed in vdev_detach %d",
				   tx_desc->id);
		return;
	}

	if (qdf_unlikely(tx_desc->vdev_id == DP_INVALID_VDEV_ID)) {
		dp_tx_comp_info_rl("Invalid vdev_id %d", tx_desc->id);
		tx_desc->flags |= DP_TX_DESC_FLAG_TX_COMP_ERR;
		goto release_tx_desc;
	}

	pdev = tx_desc->pdev;

	if (qdf_unlikely(tx_desc->pdev->is_pdev_down)) {
		dp_tx_comp_info_rl("pdev in down state %d", tx_desc->id);
		tx_desc->flags |= DP_TX_DESC_FLAG_TX_COMP_ERR;
		goto release_tx_desc;
	}

	qdf_assert(tx_desc->pdev);

	vdev_id = tx_desc->vdev_id;
	vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
				     DP_MOD_ID_HTT_COMP);

	if (qdf_unlikely(!vdev)) {
		dp_tx_comp_info_rl("Unable to get vdev ref  %d", tx_desc->id);
		tx_desc->flags |= DP_TX_DESC_FLAG_TX_COMP_ERR;
		goto release_tx_desc;
	}

	switch (tx_status) {
	case HTT_TX_FW2WBM_TX_STATUS_OK:
	case HTT_TX_FW2WBM_TX_STATUS_DROP:
	case HTT_TX_FW2WBM_TX_STATUS_TTL:
	{
		uint8_t tid;

		if (HTT_TX_WBM_COMPLETION_V2_VALID_GET(htt_desc[2])) {
			ts.peer_id =
				HTT_TX_WBM_COMPLETION_V2_SW_PEER_ID_GET(
						htt_desc[2]);
			ts.tid =
				HTT_TX_WBM_COMPLETION_V2_TID_NUM_GET(
						htt_desc[2]);
		} else {
			ts.peer_id = HTT_INVALID_PEER;
			ts.tid = HTT_INVALID_TID;
		}
		ts.release_src = HAL_TX_COMP_RELEASE_SOURCE_FW;
		ts.ppdu_id =
			HTT_TX_WBM_COMPLETION_V2_SCH_CMD_ID_GET(
					htt_desc[1]);
		ts.ack_frame_rssi =
			HTT_TX_WBM_COMPLETION_V2_ACK_FRAME_RSSI_GET(
					htt_desc[1]);

		ts.tsf = htt_desc[3];
		ts.first_msdu = 1;
		ts.last_msdu = 1;
		ts.status = (tx_status == HTT_TX_FW2WBM_TX_STATUS_OK ?
			     HAL_TX_TQM_RR_FRAME_ACKED :
			     HAL_TX_TQM_RR_REM_CMD_REM);
		tid = ts.tid;
		if (qdf_unlikely(tid >= CDP_MAX_DATA_TIDS))
			tid = CDP_MAX_DATA_TIDS - 1;

		tid_stats = &pdev->stats.tid_stats.tid_tx_stats[ring_id][tid];

		if (qdf_unlikely(pdev->delay_stats_flag) ||
		    qdf_unlikely(dp_is_vdev_tx_delay_stats_enabled(vdev)))
			dp_tx_compute_delay(vdev, tx_desc, tid, ring_id);
		if (tx_status < CDP_MAX_TX_HTT_STATUS)
			tid_stats->htt_status_cnt[tx_status]++;

		peer = dp_peer_get_ref_by_id(soc, ts.peer_id,
					     DP_MOD_ID_HTT_COMP);
		if (qdf_likely(peer)) {
			DP_STATS_INC_PKT(peer, tx.comp_pkt, 1,
					 qdf_nbuf_len(tx_desc->nbuf));
			DP_STATS_INCC(peer, tx.tx_failed, 1,
				      tx_status != HTT_TX_FW2WBM_TX_STATUS_OK);
		}

		dp_tx_comp_process_tx_status(soc, tx_desc, &ts, peer, ring_id);
		dp_tx_comp_process_desc(soc, tx_desc, &ts, peer);
		dp_tx_desc_release(tx_desc, tx_desc->pool_id);

		if (qdf_likely(peer))
			dp_peer_unref_delete(peer, DP_MOD_ID_HTT_COMP);

		break;
	}
	case HTT_TX_FW2WBM_TX_STATUS_REINJECT:
	{
		dp_tx_reinject_handler(soc, vdev, tx_desc, status);
		break;
	}
	case HTT_TX_FW2WBM_TX_STATUS_INSPECT:
	{
		dp_tx_inspect_handler(soc, vdev, tx_desc, status);
		break;
	}
	case HTT_TX_FW2WBM_TX_STATUS_VDEVID_MISMATCH:
	{
		DP_STATS_INC(vdev, tx_i.dropped.fail_per_pkt_vdev_id_check, 1);
		goto release_tx_desc;
	}
	default:
		dp_tx_comp_err("Invalid HTT tx_status %d\n",
			       tx_status);
		goto release_tx_desc;
	}

	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_HTT_COMP);
	return;

release_tx_desc:
	dp_tx_comp_free_buf(soc, tx_desc);
	dp_tx_desc_release(tx_desc, tx_desc->pool_id);
	if (vdev)
		dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_HTT_COMP);
}

#ifdef QCA_OL_TX_MULTIQ_SUPPORT
/*
 * dp_tx_get_rbm_id()- Get the RBM ID for data transmission completion.
 * @dp_soc - DP soc structure pointer
 * @ring_id - Transmit Queue/ring_id to be used when XPS is enabled
 *
 * Return - HAL ring handle
 */
#ifdef IPA_OFFLOAD
static inline uint8_t dp_tx_get_rbm_id_li(struct dp_soc *soc,
					  uint8_t ring_id)
{
	return (ring_id + soc->wbm_sw0_bm_id);
}
#else
static inline uint8_t dp_tx_get_rbm_id_li(struct dp_soc *soc,
					  uint8_t ring_id)
{
	return (ring_id ? soc->wbm_sw0_bm_id + (ring_id - 1) :
			  HAL_WBM_SW2_BM_ID(soc->wbm_sw0_bm_id));
}
#endif
#else

#ifdef TX_MULTI_TCL
#ifdef IPA_OFFLOAD
static inline uint8_t dp_tx_get_rbm_id_li(struct dp_soc *soc,
					  uint8_t ring_id)
{
	if (soc->wlan_cfg_ctx->ipa_enabled)
		return (ring_id + soc->wbm_sw0_bm_id);

	return soc->wlan_cfg_ctx->tcl_wbm_map_array[ring_id].wbm_rbm_id;
}
#else
static inline uint8_t dp_tx_get_rbm_id_li(struct dp_soc *soc,
					  uint8_t ring_id)
{
	return soc->wlan_cfg_ctx->tcl_wbm_map_array[ring_id].wbm_rbm_id;
}
#endif
#else
static inline uint8_t dp_tx_get_rbm_id_li(struct dp_soc *soc,
					  uint8_t ring_id)
{
	return (ring_id + soc->wbm_sw0_bm_id);
}
#endif
#endif

#if defined(CLEAR_SW2TCL_CONSUMED_DESC)
/**
 * dp_tx_clear_consumed_hw_descs - Reset all the consumed Tx ring descs to 0
 *
 * @soc: DP soc handle
 * @hal_ring_hdl: Source ring pointer
 *
 * Return: void
 */
static inline
void dp_tx_clear_consumed_hw_descs(struct dp_soc *soc,
				   hal_ring_handle_t hal_ring_hdl)
{
	void *desc = hal_srng_src_get_next_consumed(soc->hal_soc, hal_ring_hdl);

	while (desc) {
		hal_tx_desc_clear(desc);
		desc = hal_srng_src_get_next_consumed(soc->hal_soc,
						      hal_ring_hdl);
	}
}

#else
static inline
void dp_tx_clear_consumed_hw_descs(struct dp_soc *soc,
				   hal_ring_handle_t hal_ring_hdl)
{
}
#endif /* CLEAR_SW2TCL_CONSUMED_DESC */

QDF_STATUS
dp_tx_hw_enqueue_li(struct dp_soc *soc, struct dp_vdev *vdev,
		    struct dp_tx_desc_s *tx_desc, uint16_t fw_metadata,
		    struct cdp_tx_exception_metadata *tx_exc_metadata,
		    struct dp_tx_msdu_info_s *msdu_info)
{
	void *hal_tx_desc;
	uint32_t *hal_tx_desc_cached;
	int coalesce = 0;
	struct dp_tx_queue *tx_q = &msdu_info->tx_queue;
	uint8_t ring_id = tx_q->ring_id & DP_TX_QUEUE_MASK;
	uint8_t tid = msdu_info->tid;

	/*
	 * Setting it initialization statically here to avoid
	 * a memset call jump with qdf_mem_set call
	 */
	uint8_t cached_desc[HAL_TX_DESC_LEN_BYTES] = { 0 };

	enum cdp_sec_type sec_type = ((tx_exc_metadata &&
			tx_exc_metadata->sec_type != CDP_INVALID_SEC_TYPE) ?
			tx_exc_metadata->sec_type : vdev->sec_type);

	/* Return Buffer Manager ID */
	uint8_t bm_id = dp_tx_get_rbm_id_li(soc, ring_id);

	hal_ring_handle_t hal_ring_hdl = NULL;

	QDF_STATUS status = QDF_STATUS_E_RESOURCES;

	if (!dp_tx_is_desc_id_valid(soc, tx_desc->id)) {
		dp_err_rl("Invalid tx desc id:%d", tx_desc->id);
		return QDF_STATUS_E_RESOURCES;
	}

	hal_tx_desc_cached = (void *)cached_desc;

	hal_tx_desc_set_buf_addr(soc->hal_soc, hal_tx_desc_cached,
				 tx_desc->dma_addr, bm_id, tx_desc->id,
				 (tx_desc->flags & DP_TX_DESC_FLAG_FRAG));
	hal_tx_desc_set_lmac_id_li(soc->hal_soc, hal_tx_desc_cached,
				   vdev->lmac_id);
	hal_tx_desc_set_search_type_li(soc->hal_soc, hal_tx_desc_cached,
				       vdev->search_type);
	hal_tx_desc_set_search_index_li(soc->hal_soc, hal_tx_desc_cached,
					vdev->bss_ast_idx);
	hal_tx_desc_set_dscp_tid_table_id(soc->hal_soc, hal_tx_desc_cached,
					  vdev->dscp_tid_map_id);

	hal_tx_desc_set_encrypt_type(hal_tx_desc_cached,
				     sec_type_map[sec_type]);
	hal_tx_desc_set_cache_set_num(soc->hal_soc, hal_tx_desc_cached,
				      (vdev->bss_ast_hash & 0xF));

	hal_tx_desc_set_fw_metadata(hal_tx_desc_cached, fw_metadata);
	hal_tx_desc_set_buf_length(hal_tx_desc_cached, tx_desc->length);
	hal_tx_desc_set_buf_offset(hal_tx_desc_cached, tx_desc->pkt_offset);
	hal_tx_desc_set_encap_type(hal_tx_desc_cached, tx_desc->tx_encap_type);
	hal_tx_desc_set_addr_search_flags(hal_tx_desc_cached,
					  vdev->hal_desc_addr_search_flags);

	if (tx_desc->flags & DP_TX_DESC_FLAG_TO_FW)
		hal_tx_desc_set_to_fw(hal_tx_desc_cached, 1);

	/* verify checksum offload configuration*/
	if ((qdf_nbuf_get_tx_cksum(tx_desc->nbuf) ==
				   QDF_NBUF_TX_CKSUM_TCP_UDP) ||
	      qdf_nbuf_is_tso(tx_desc->nbuf))  {
		hal_tx_desc_set_l3_checksum_en(hal_tx_desc_cached, 1);
		hal_tx_desc_set_l4_checksum_en(hal_tx_desc_cached, 1);
	}

	if (tid != HTT_TX_EXT_TID_INVALID)
		hal_tx_desc_set_hlos_tid(hal_tx_desc_cached, tid);

	if (tx_desc->flags & DP_TX_DESC_FLAG_MESH)
		hal_tx_desc_set_mesh_en(soc->hal_soc, hal_tx_desc_cached, 1);

	if (!dp_tx_desc_set_ktimestamp(vdev, tx_desc))
		dp_tx_desc_set_timestamp(tx_desc);

	dp_verbose_debug("length:%d , type = %d, dma_addr %llx, offset %d desc id %u",
			 tx_desc->length,
			 (tx_desc->flags & DP_TX_DESC_FLAG_FRAG),
			 (uint64_t)tx_desc->dma_addr, tx_desc->pkt_offset,
			 tx_desc->id);

	hal_ring_hdl = dp_tx_get_hal_ring_hdl(soc, ring_id);

	if (qdf_unlikely(dp_tx_hal_ring_access_start(soc, hal_ring_hdl))) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s %d : HAL RING Access Failed -- %pK",
			 __func__, __LINE__, hal_ring_hdl);
		DP_STATS_INC(soc, tx.tcl_ring_full[ring_id], 1);
		DP_STATS_INC(vdev, tx_i.dropped.enqueue_fail, 1);
		return status;
	}

	dp_tx_clear_consumed_hw_descs(soc, hal_ring_hdl);

	/* Sync cached descriptor with HW */

	hal_tx_desc = hal_srng_src_get_next(soc->hal_soc, hal_ring_hdl);
	if (qdf_unlikely(!hal_tx_desc)) {
		dp_verbose_debug("TCL ring full ring_id:%d", ring_id);
		DP_STATS_INC(soc, tx.tcl_ring_full[ring_id], 1);
		DP_STATS_INC(vdev, tx_i.dropped.enqueue_fail, 1);
		goto ring_access_fail;
	}

	tx_desc->flags |= DP_TX_DESC_FLAG_QUEUED_TX;
	dp_vdev_peer_stats_update_protocol_cnt_tx(vdev, tx_desc->nbuf);
	hal_tx_desc_sync(hal_tx_desc_cached, hal_tx_desc);
	coalesce = dp_tx_attempt_coalescing(soc, vdev, tx_desc, tid,
					    msdu_info, ring_id);
	DP_STATS_INC_PKT(vdev, tx_i.processed, 1, tx_desc->length);
	DP_STATS_INC(soc, tx.tcl_enq[ring_id], 1);
	dp_tx_update_stats(soc, tx_desc, ring_id);
	status = QDF_STATUS_SUCCESS;

	dp_tx_hw_desc_update_evt((uint8_t *)hal_tx_desc_cached,
				 hal_ring_hdl, soc);

ring_access_fail:
	dp_tx_ring_access_end_wrapper(soc, hal_ring_hdl, coalesce);
	dp_pkt_add_timestamp(vdev, QDF_PKT_TX_DRIVER_EXIT,
			     qdf_get_log_timestamp(), tx_desc->nbuf);

	return status;
}

QDF_STATUS dp_tx_desc_pool_init_li(struct dp_soc *soc,
				   uint16_t num_elem,
				   uint8_t pool_id)
{
	uint32_t id, count, page_id, offset, pool_id_32;
	struct dp_tx_desc_s *tx_desc;
	struct dp_tx_desc_pool_s *tx_desc_pool;
	uint16_t num_desc_per_page;

	tx_desc_pool = &soc->tx_desc[pool_id];
	tx_desc = tx_desc_pool->freelist;
	count = 0;
	pool_id_32 = (uint32_t)pool_id;
	num_desc_per_page = tx_desc_pool->desc_pages.num_element_per_page;
	while (tx_desc) {
		page_id = count / num_desc_per_page;
		offset = count % num_desc_per_page;
		id = ((pool_id_32 << DP_TX_DESC_ID_POOL_OS) |
			(page_id << DP_TX_DESC_ID_PAGE_OS) | offset);

		tx_desc->id = id;
		tx_desc->pool_id = pool_id;
		dp_tx_desc_set_magic(tx_desc, DP_TX_MAGIC_PATTERN_FREE);
		tx_desc = tx_desc->next;
		count++;
	}

	return QDF_STATUS_SUCCESS;
}

void dp_tx_desc_pool_deinit_li(struct dp_soc *soc,
			       struct dp_tx_desc_pool_s *tx_desc_pool,
			       uint8_t pool_id)
{
}
