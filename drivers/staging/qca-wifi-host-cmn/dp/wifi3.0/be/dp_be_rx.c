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
#include "hal_hw_headers.h"
#include "dp_types.h"
#include "dp_rx.h"
#include "dp_be_rx.h"
#include "dp_peer.h"
#include "hal_rx.h"
#include "hal_be_rx.h"
#include "hal_api.h"
#include "hal_be_api.h"
#include "qdf_nbuf.h"
#ifdef MESH_MODE_SUPPORT
#include "if_meta_hdr.h"
#endif
#include "dp_internal.h"
#include "dp_ipa.h"
#ifdef FEATURE_WDS
#include "dp_txrx_wds.h"
#endif
#include "dp_hist.h"
#include "dp_rx_buffer_pool.h"

#ifndef AST_OFFLOAD_ENABLE
static void
dp_rx_wds_learn(struct dp_soc *soc,
		struct dp_vdev *vdev,
		uint8_t *rx_tlv_hdr,
		struct dp_peer *peer,
		qdf_nbuf_t nbuf,
		struct hal_rx_msdu_metadata msdu_metadata)
{
	/* WDS Source Port Learning */
	if (qdf_likely(vdev->wds_enabled))
		dp_rx_wds_srcport_learn(soc,
				rx_tlv_hdr,
				peer,
				nbuf,
				msdu_metadata);
}
#else
#ifdef QCA_SUPPORT_WDS_EXTENDED
/**
 * dp_wds_ext_peer_learn_be() - function to send event to control
 * path on receiving 1st 4-address frame from backhaul.
 * @soc: DP soc
 * @ta_peer: WDS repeater peer
 * @rx_tlv_hdr  : start address of rx tlvs
 *
 * Return: void
 */
static inline void dp_wds_ext_peer_learn_be(struct dp_soc *soc,
					    struct dp_peer *ta_peer,
					    uint8_t *rx_tlv_hdr)
{
	uint8_t wds_ext_src_mac[QDF_MAC_ADDR_SIZE];

	/* instead of checking addr4 is valid or not in per packet path
	 * check for init bit, which will be set on reception of
	 * first addr4 valid packet.
	 */
	if (!ta_peer->vdev->wds_ext_enabled ||
	    qdf_atomic_test_bit(WDS_EXT_PEER_INIT_BIT, &ta_peer->wds_ext.init))
		return;

	if (hal_rx_get_mpdu_mac_ad4_valid(soc->hal_soc, rx_tlv_hdr)) {
		qdf_atomic_test_and_set_bit(WDS_EXT_PEER_INIT_BIT,
					    &ta_peer->wds_ext.init);
		qdf_mem_copy(wds_ext_src_mac, &ta_peer->mac_addr.raw[0],
			     QDF_MAC_ADDR_SIZE);
		soc->cdp_soc.ol_ops->rx_wds_ext_peer_learn(
						soc->ctrl_psoc,
						ta_peer->peer_id,
						ta_peer->vdev->vdev_id,
						wds_ext_src_mac);
	}
}
#else
static inline void dp_wds_ext_peer_learn_be(struct dp_soc *soc,
					    struct dp_peer *ta_peer,
					    uint8_t *rx_tlv_hdr)
{
}
#endif
static void
dp_rx_wds_learn(struct dp_soc *soc,
		struct dp_vdev *vdev,
		uint8_t *rx_tlv_hdr,
		struct dp_peer *ta_peer,
		qdf_nbuf_t nbuf,
		struct hal_rx_msdu_metadata msdu_metadata)
{
	dp_wds_ext_peer_learn_be(soc, ta_peer, rx_tlv_hdr);
}
#endif

/**
 * dp_rx_process_be() - Brain of the Rx processing functionality
 *		     Called from the bottom half (tasklet/NET_RX_SOFTIRQ)
 * @int_ctx: per interrupt context
 * @hal_ring: opaque pointer to the HAL Rx Ring, which will be serviced
 * @reo_ring_num: ring number (0, 1, 2 or 3) of the reo ring.
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements the core of Rx functionality. This is
 * expected to handle only non-error frames.
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t dp_rx_process_be(struct dp_intr *int_ctx,
			  hal_ring_handle_t hal_ring_hdl, uint8_t reo_ring_num,
			  uint32_t quota)
{
	hal_ring_desc_t ring_desc;
	hal_soc_handle_t hal_soc;
	struct dp_rx_desc *rx_desc = NULL;
	qdf_nbuf_t nbuf, next;
	bool near_full;
	union dp_rx_desc_list_elem_t *head[MAX_PDEV_CNT];
	union dp_rx_desc_list_elem_t *tail[MAX_PDEV_CNT];
	uint32_t num_pending;
	uint32_t rx_bufs_used = 0, rx_buf_cookie;
	uint16_t msdu_len = 0;
	uint16_t peer_id;
	uint8_t vdev_id;
	struct dp_peer *peer;
	struct dp_vdev *vdev;
	uint32_t pkt_len = 0;
	struct hal_rx_mpdu_desc_info mpdu_desc_info;
	struct hal_rx_msdu_desc_info msdu_desc_info;
	enum hal_reo_error_status error;
	uint32_t peer_mdata;
	uint8_t *rx_tlv_hdr;
	uint32_t rx_bufs_reaped[MAX_PDEV_CNT];
	uint8_t mac_id = 0;
	struct dp_pdev *rx_pdev;
	bool enh_flag;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	struct dp_soc *soc = int_ctx->soc;
	uint8_t core_id = 0;
	struct cdp_tid_rx_stats *tid_stats;
	qdf_nbuf_t nbuf_head;
	qdf_nbuf_t nbuf_tail;
	qdf_nbuf_t deliver_list_head;
	qdf_nbuf_t deliver_list_tail;
	uint32_t num_rx_bufs_reaped = 0;
	uint32_t intr_id;
	struct hif_opaque_softc *scn;
	int32_t tid = 0;
	bool is_prev_msdu_last = true;
	uint32_t num_entries_avail = 0;
	uint32_t rx_ol_pkt_cnt = 0;
	uint32_t num_entries = 0;
	struct hal_rx_msdu_metadata msdu_metadata;
	QDF_STATUS status;
	qdf_nbuf_t ebuf_head;
	qdf_nbuf_t ebuf_tail;
	uint8_t pkt_capture_offload = 0;
	struct dp_srng *rx_ring = &soc->reo_dest_ring[reo_ring_num];
	int max_reap_limit, ring_near_full;
	struct dp_soc *replenish_soc;

	DP_HIST_INIT();

	qdf_assert_always(soc && hal_ring_hdl);
	hal_soc = soc->hal_soc;
	qdf_assert_always(hal_soc);

	scn = soc->hif_handle;
	hif_pm_runtime_mark_dp_rx_busy(scn);
	intr_id = int_ctx->dp_intr_id;
	num_entries = hal_srng_get_num_entries(hal_soc, hal_ring_hdl);

more_data:
	/* reset local variables here to be re-used in the function */
	nbuf_head = NULL;
	nbuf_tail = NULL;
	deliver_list_head = NULL;
	deliver_list_tail = NULL;
	peer = NULL;
	vdev = NULL;
	num_rx_bufs_reaped = 0;
	ebuf_head = NULL;
	ebuf_tail = NULL;
	ring_near_full = 0;
	max_reap_limit = dp_rx_get_loop_pkt_limit(soc);

	qdf_mem_zero(rx_bufs_reaped, sizeof(rx_bufs_reaped));
	qdf_mem_zero(&mpdu_desc_info, sizeof(mpdu_desc_info));
	qdf_mem_zero(&msdu_desc_info, sizeof(msdu_desc_info));
	qdf_mem_zero(head, sizeof(head));
	qdf_mem_zero(tail, sizeof(tail));

	ring_near_full = _dp_srng_test_and_update_nf_params(soc, rx_ring,
							    &max_reap_limit);

	if (qdf_unlikely(dp_rx_srng_access_start(int_ctx, soc, hal_ring_hdl))) {
		/*
		 * Need API to convert from hal_ring pointer to
		 * Ring Type / Ring Id combo
		 */
		DP_STATS_INC(soc, rx.err.hal_ring_access_fail, 1);
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  FL("HAL RING Access Failed -- %pK"), hal_ring_hdl);
		goto done;
	}

	/*
	 * start reaping the buffers from reo ring and queue
	 * them in per vdev queue.
	 * Process the received pkts in a different per vdev loop.
	 */
	while (qdf_likely(quota &&
			  (ring_desc = hal_srng_dst_peek(hal_soc,
							 hal_ring_hdl)))) {
		error = HAL_RX_ERROR_STATUS_GET(ring_desc);

		if (qdf_unlikely(error == HAL_REO_ERROR_DETECTED)) {
			dp_rx_err("%pK: HAL RING 0x%pK:error %d",
				  soc, hal_ring_hdl, error);
			DP_STATS_INC(soc, rx.err.hal_reo_error[reo_ring_num],
				     1);
			/* Don't know how to deal with this -- assert */
			qdf_assert(0);
		}

		dp_rx_ring_record_entry(soc, reo_ring_num, ring_desc);
		rx_buf_cookie = HAL_RX_REO_BUF_COOKIE_GET(ring_desc);
		status = dp_rx_cookie_check_and_invalidate(ring_desc);
		if (qdf_unlikely(QDF_IS_STATUS_ERROR(status))) {
			DP_STATS_INC(soc, rx.err.stale_cookie, 1);
			break;
		}

		rx_desc = (struct dp_rx_desc *)
				hal_rx_get_reo_desc_va(ring_desc);
		dp_rx_desc_sw_cc_check(soc, rx_buf_cookie, &rx_desc);

		status = dp_rx_desc_sanity(soc, hal_soc, hal_ring_hdl,
					   ring_desc, rx_desc);
		if (QDF_IS_STATUS_ERROR(status)) {
			if (qdf_unlikely(rx_desc && rx_desc->nbuf)) {
				qdf_assert_always(!rx_desc->unmapped);
				dp_ipa_reo_ctx_buf_mapping_lock(
							soc,
							reo_ring_num);
				dp_ipa_handle_rx_buf_smmu_mapping(
							soc,
							rx_desc->nbuf,
							RX_DATA_BUFFER_SIZE,
							false);
				qdf_nbuf_unmap_nbytes_single(
							soc->osdev,
							rx_desc->nbuf,
							QDF_DMA_FROM_DEVICE,
							RX_DATA_BUFFER_SIZE);
				rx_desc->unmapped = 1;
				dp_ipa_reo_ctx_buf_mapping_unlock(
								soc,
								reo_ring_num);
				dp_rx_buffer_pool_nbuf_free(soc, rx_desc->nbuf,
							    rx_desc->pool_id);
				dp_rx_add_to_free_desc_list(
							&head[rx_desc->pool_id],
							&tail[rx_desc->pool_id],
							rx_desc);
			}
			hal_srng_dst_get_next(hal_soc, hal_ring_hdl);
			continue;
		}

		/*
		 * this is a unlikely scenario where the host is reaping
		 * a descriptor which it already reaped just a while ago
		 * but is yet to replenish it back to HW.
		 * In this case host will dump the last 128 descriptors
		 * including the software descriptor rx_desc and assert.
		 */

		if (qdf_unlikely(!rx_desc->in_use)) {
			DP_STATS_INC(soc, rx.err.hal_reo_dest_dup, 1);
			dp_info_rl("Reaping rx_desc not in use!");
			dp_rx_dump_info_and_assert(soc, hal_ring_hdl,
						   ring_desc, rx_desc);
			/* ignore duplicate RX desc and continue to process */
			/* Pop out the descriptor */
			hal_srng_dst_get_next(hal_soc, hal_ring_hdl);
			continue;
		}

		status = dp_rx_desc_nbuf_sanity_check(soc, ring_desc, rx_desc);
		if (qdf_unlikely(QDF_IS_STATUS_ERROR(status))) {
			DP_STATS_INC(soc, rx.err.nbuf_sanity_fail, 1);
			dp_info_rl("Nbuf sanity check failure!");
			dp_rx_dump_info_and_assert(soc, hal_ring_hdl,
						   ring_desc, rx_desc);
			rx_desc->in_err_state = 1;
			hal_srng_dst_get_next(hal_soc, hal_ring_hdl);
			continue;
		}

		if (qdf_unlikely(!dp_rx_desc_check_magic(rx_desc))) {
			dp_err("Invalid rx_desc cookie=%d", rx_buf_cookie);
			DP_STATS_INC(soc, rx.err.rx_desc_invalid_magic, 1);
			dp_rx_dump_info_and_assert(soc, hal_ring_hdl,
						   ring_desc, rx_desc);
		}

		/* Get MPDU DESC info */
		hal_rx_mpdu_desc_info_get_be(ring_desc, &mpdu_desc_info);

		/* Get MSDU DESC info */
		hal_rx_msdu_desc_info_get_be(ring_desc, &msdu_desc_info);

		if (qdf_unlikely(msdu_desc_info.msdu_flags &
				 HAL_MSDU_F_MSDU_CONTINUATION)) {
			/* previous msdu has end bit set, so current one is
			 * the new MPDU
			 */
			if (is_prev_msdu_last) {
				/* Get number of entries available in HW ring */
				num_entries_avail =
				hal_srng_dst_num_valid(hal_soc,
						       hal_ring_hdl, 1);

				/* For new MPDU check if we can read complete
				 * MPDU by comparing the number of buffers
				 * available and number of buffers needed to
				 * reap this MPDU
				 */
				if ((msdu_desc_info.msdu_len /
				     (RX_DATA_BUFFER_SIZE -
				      soc->rx_pkt_tlv_size) + 1) >
				    num_entries_avail) {
					DP_STATS_INC(soc,
						     rx.msdu_scatter_wait_break,
						     1);
					dp_rx_cookie_reset_invalid_bit(
								     ring_desc);
					break;
				}
				is_prev_msdu_last = false;
			}
		}
		core_id = smp_processor_id();
		DP_STATS_INC(soc, rx.ring_packets[core_id][reo_ring_num], 1);

		if (mpdu_desc_info.mpdu_flags & HAL_MPDU_F_RETRY_BIT)
			qdf_nbuf_set_rx_retry_flag(rx_desc->nbuf, 1);

		if (qdf_unlikely(mpdu_desc_info.mpdu_flags &
				 HAL_MPDU_F_RAW_AMPDU))
			qdf_nbuf_set_raw_frame(rx_desc->nbuf, 1);

		if (!is_prev_msdu_last &&
		    msdu_desc_info.msdu_flags & HAL_MSDU_F_LAST_MSDU_IN_MPDU)
			is_prev_msdu_last = true;

		/* Pop out the descriptor*/
		hal_srng_dst_get_next(hal_soc, hal_ring_hdl);

		rx_bufs_reaped[rx_desc->pool_id]++;
		peer_mdata = mpdu_desc_info.peer_meta_data;
		QDF_NBUF_CB_RX_PEER_ID(rx_desc->nbuf) =
			dp_rx_peer_metadata_peer_id_get_be(soc, peer_mdata);
		QDF_NBUF_CB_RX_VDEV_ID(rx_desc->nbuf) =
			dp_rx_peer_metadata_vdev_id_get_be(soc, peer_mdata);

		/* to indicate whether this msdu is rx offload */
		pkt_capture_offload =
			DP_PEER_METADATA_OFFLOAD_GET_BE(peer_mdata);

		/*
		 * save msdu flags first, last and continuation msdu in
		 * nbuf->cb, also save mcbc, is_da_valid, is_sa_valid and
		 * length to nbuf->cb. This ensures the info required for
		 * per pkt processing is always in the same cache line.
		 * This helps in improving throughput for smaller pkt
		 * sizes.
		 */
		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_FIRST_MSDU_IN_MPDU)
			qdf_nbuf_set_rx_chfrag_start(rx_desc->nbuf, 1);

		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_MSDU_CONTINUATION)
			qdf_nbuf_set_rx_chfrag_cont(rx_desc->nbuf, 1);

		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_LAST_MSDU_IN_MPDU)
			qdf_nbuf_set_rx_chfrag_end(rx_desc->nbuf, 1);

		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_DA_IS_MCBC)
			qdf_nbuf_set_da_mcbc(rx_desc->nbuf, 1);

		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_DA_IS_VALID)
			qdf_nbuf_set_da_valid(rx_desc->nbuf, 1);

		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_SA_IS_VALID)
			qdf_nbuf_set_sa_valid(rx_desc->nbuf, 1);

		if (msdu_desc_info.msdu_flags & HAL_MSDU_F_INTRA_BSS)
			qdf_nbuf_set_intra_bss(rx_desc->nbuf, 1);

		if (qdf_likely(mpdu_desc_info.mpdu_flags &
			       HAL_MPDU_F_QOS_CONTROL_VALID))
			qdf_nbuf_set_tid_val(rx_desc->nbuf, mpdu_desc_info.tid);

		/* set sw exception */
		qdf_nbuf_set_rx_reo_dest_ind_or_sw_excpt(
				rx_desc->nbuf,
				hal_rx_sw_exception_get_be(ring_desc));

		QDF_NBUF_CB_RX_PKT_LEN(rx_desc->nbuf) = msdu_desc_info.msdu_len;

		QDF_NBUF_CB_RX_CTX_ID(rx_desc->nbuf) = reo_ring_num;

		/*
		 * move unmap after scattered msdu waiting break logic
		 * in case double skb unmap happened.
		 */
		rx_desc_pool = &soc->rx_desc_buf[rx_desc->pool_id];
		dp_ipa_reo_ctx_buf_mapping_lock(soc, reo_ring_num);
		dp_ipa_handle_rx_buf_smmu_mapping(soc, rx_desc->nbuf,
						  rx_desc_pool->buf_size,
						  false);
		qdf_nbuf_unmap_nbytes_single(soc->osdev, rx_desc->nbuf,
					     QDF_DMA_FROM_DEVICE,
					     rx_desc_pool->buf_size);
		rx_desc->unmapped = 1;
		dp_ipa_reo_ctx_buf_mapping_unlock(soc, reo_ring_num);
		DP_RX_PROCESS_NBUF(soc, nbuf_head, nbuf_tail, ebuf_head,
				   ebuf_tail, rx_desc);
		/*
		 * if continuation bit is set then we have MSDU spread
		 * across multiple buffers, let us not decrement quota
		 * till we reap all buffers of that MSDU.
		 */
		if (qdf_likely(!qdf_nbuf_is_rx_chfrag_cont(rx_desc->nbuf)))
			quota -= 1;

		dp_rx_add_to_free_desc_list(&head[rx_desc->pool_id],
					    &tail[rx_desc->pool_id], rx_desc);
		num_rx_bufs_reaped++;
		/*
		 * only if complete msdu is received for scatter case,
		 * then allow break.
		 */
		if (is_prev_msdu_last &&
		    dp_rx_reap_loop_pkt_limit_hit(soc, num_rx_bufs_reaped,
						  max_reap_limit))
			break;
	}
done:
	dp_rx_srng_access_end(int_ctx, soc, hal_ring_hdl);

	replenish_soc = dp_rx_replensih_soc_get(soc, reo_ring_num);
	for (mac_id = 0; mac_id < MAX_PDEV_CNT; mac_id++) {
		/*
		 * continue with next mac_id if no pkts were reaped
		 * from that pool
		 */
		if (!rx_bufs_reaped[mac_id])
			continue;

		dp_rxdma_srng = &replenish_soc->rx_refill_buf_ring[mac_id];

		rx_desc_pool = &replenish_soc->rx_desc_buf[mac_id];

		dp_rx_buffers_replenish(replenish_soc, mac_id, dp_rxdma_srng,
					rx_desc_pool, rx_bufs_reaped[mac_id],
					&head[mac_id], &tail[mac_id]);
	}

	dp_verbose_debug("replenished %u\n", rx_bufs_reaped[0]);
	/* Peer can be NULL is case of LFR */
	if (qdf_likely(peer))
		vdev = NULL;

	/*
	 * BIG loop where each nbuf is dequeued from global queue,
	 * processed and queued back on a per vdev basis. These nbufs
	 * are sent to stack as and when we run out of nbufs
	 * or a new nbuf dequeued from global queue has a different
	 * vdev when compared to previous nbuf.
	 */
	nbuf = nbuf_head;
	while (nbuf) {
		next = nbuf->next;
		if (qdf_unlikely(dp_rx_is_raw_frame_dropped(nbuf))) {
			nbuf = next;
			DP_STATS_INC(soc, rx.err.raw_frm_drop, 1);
			continue;
		}

		rx_tlv_hdr = qdf_nbuf_data(nbuf);
		vdev_id = QDF_NBUF_CB_RX_VDEV_ID(nbuf);
		peer_id =  QDF_NBUF_CB_RX_PEER_ID(nbuf);

		if (dp_rx_is_list_ready(deliver_list_head, vdev, peer,
					peer_id, vdev_id)) {
			dp_rx_deliver_to_stack(soc, vdev, peer,
					       deliver_list_head,
					       deliver_list_tail);
			deliver_list_head = NULL;
			deliver_list_tail = NULL;
		}

		/* Get TID from struct cb->tid_val, save to tid */
		if (qdf_nbuf_is_rx_chfrag_start(nbuf)) {
			tid = qdf_nbuf_get_tid_val(nbuf);
			if (tid >= CDP_MAX_DATA_TIDS) {
				DP_STATS_INC(soc, rx.err.rx_invalid_tid_err, 1);
				qdf_nbuf_free(nbuf);
				nbuf = next;
				continue;
			}
		}

		if (qdf_unlikely(!peer)) {
			peer = dp_peer_get_ref_by_id(soc, peer_id,
						     DP_MOD_ID_RX);
		} else if (peer && peer->peer_id != peer_id) {
			dp_peer_unref_delete(peer, DP_MOD_ID_RX);
			peer = dp_peer_get_ref_by_id(soc, peer_id,
						     DP_MOD_ID_RX);
		}

		if (peer) {
			QDF_NBUF_CB_DP_TRACE_PRINT(nbuf) = false;
			qdf_dp_trace_set_track(nbuf, QDF_RX);
			QDF_NBUF_CB_RX_DP_TRACE(nbuf) = 1;
			QDF_NBUF_CB_RX_PACKET_TRACK(nbuf) =
				QDF_NBUF_RX_PKT_DATA_TRACK;
		}

		rx_bufs_used++;

		if (qdf_likely(peer)) {
			vdev = peer->vdev;
		} else {
			nbuf->next = NULL;
			dp_rx_deliver_to_pkt_capture_no_peer(
					soc, nbuf, pkt_capture_offload);

			if (!pkt_capture_offload)
				dp_rx_deliver_to_stack_no_peer(soc, nbuf);
			nbuf = next;
			continue;
		}

		if (qdf_unlikely(!vdev)) {
			qdf_nbuf_free(nbuf);
			nbuf = next;
			DP_STATS_INC(soc, rx.err.invalid_vdev, 1);
			continue;
		}

		/* when hlos tid override is enabled, save tid in
		 * skb->priority
		 */
		if (qdf_unlikely(vdev->skip_sw_tid_classification &
					DP_TXRX_HLOS_TID_OVERRIDE_ENABLED))
			qdf_nbuf_set_priority(nbuf, tid);

		rx_pdev = vdev->pdev;
		DP_RX_TID_SAVE(nbuf, tid);
		if (qdf_unlikely(rx_pdev->delay_stats_flag) ||
		    qdf_unlikely(wlan_cfg_is_peer_ext_stats_enabled(
				 soc->wlan_cfg_ctx)) ||
		    dp_rx_pkt_tracepoints_enabled())
			qdf_nbuf_set_timestamp(nbuf);

		enh_flag = rx_pdev->enhanced_stats_en;

		tid_stats =
		&rx_pdev->stats.tid_stats.tid_rx_stats[reo_ring_num][tid];

		/*
		 * Check if DMA completed -- msdu_done is the last bit
		 * to be written
		 */
		if (qdf_unlikely(!qdf_nbuf_is_rx_chfrag_cont(nbuf) &&
				 !hal_rx_attn_msdu_done_get(hal_soc,
							    rx_tlv_hdr))) {
			dp_err("MSDU DONE failure");
			DP_STATS_INC(soc, rx.err.msdu_done_fail, 1);
			hal_rx_dump_pkt_tlvs(hal_soc, rx_tlv_hdr,
					     QDF_TRACE_LEVEL_INFO);
			tid_stats->fail_cnt[MSDU_DONE_FAILURE]++;
			qdf_nbuf_free(nbuf);
			qdf_assert(0);
			nbuf = next;
			continue;
		}

		DP_HIST_PACKET_COUNT_INC(vdev->pdev->pdev_id);
		/*
		 * First IF condition:
		 * 802.11 Fragmented pkts are reinjected to REO
		 * HW block as SG pkts and for these pkts we only
		 * need to pull the RX TLVS header length.
		 * Second IF condition:
		 * The below condition happens when an MSDU is spread
		 * across multiple buffers. This can happen in two cases
		 * 1. The nbuf size is smaller then the received msdu.
		 *    ex: we have set the nbuf size to 2048 during
		 *        nbuf_alloc. but we received an msdu which is
		 *        2304 bytes in size then this msdu is spread
		 *        across 2 nbufs.
		 *
		 * 2. AMSDUs when RAW mode is enabled.
		 *    ex: 1st MSDU is in 1st nbuf and 2nd MSDU is spread
		 *        across 1st nbuf and 2nd nbuf and last MSDU is
		 *        spread across 2nd nbuf and 3rd nbuf.
		 *
		 * for these scenarios let us create a skb frag_list and
		 * append these buffers till the last MSDU of the AMSDU
		 * Third condition:
		 * This is the most likely case, we receive 802.3 pkts
		 * decapsulated by HW, here we need to set the pkt length.
		 */
		hal_rx_msdu_metadata_get(hal_soc, rx_tlv_hdr, &msdu_metadata);
		if (qdf_unlikely(qdf_nbuf_is_frag(nbuf))) {
			bool is_mcbc, is_sa_vld, is_da_vld;

			is_mcbc = hal_rx_msdu_end_da_is_mcbc_get(soc->hal_soc,
								 rx_tlv_hdr);
			is_sa_vld =
				hal_rx_msdu_end_sa_is_valid_get(soc->hal_soc,
								rx_tlv_hdr);
			is_da_vld =
				hal_rx_msdu_end_da_is_valid_get(soc->hal_soc,
								rx_tlv_hdr);

			qdf_nbuf_set_da_mcbc(nbuf, is_mcbc);
			qdf_nbuf_set_da_valid(nbuf, is_da_vld);
			qdf_nbuf_set_sa_valid(nbuf, is_sa_vld);

			qdf_nbuf_pull_head(nbuf, soc->rx_pkt_tlv_size);
		} else if (qdf_nbuf_is_rx_chfrag_cont(nbuf)) {
			msdu_len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);
			nbuf = dp_rx_sg_create(soc, nbuf);
			next = nbuf->next;

			if (qdf_nbuf_is_raw_frame(nbuf)) {
				DP_STATS_INC(vdev->pdev, rx_raw_pkts, 1);
				DP_STATS_INC_PKT(peer, rx.raw, 1, msdu_len);
			} else {
				qdf_nbuf_free(nbuf);
				DP_STATS_INC(soc, rx.err.scatter_msdu, 1);
				dp_info_rl("scatter msdu len %d, dropped",
					   msdu_len);
				nbuf = next;
				continue;
			}
		} else {
			msdu_len = QDF_NBUF_CB_RX_PKT_LEN(nbuf);
			pkt_len = msdu_len +
				  msdu_metadata.l3_hdr_pad +
				  soc->rx_pkt_tlv_size;

			qdf_nbuf_set_pktlen(nbuf, pkt_len);
			dp_rx_skip_tlvs(soc, nbuf, msdu_metadata.l3_hdr_pad);
		}

		dp_rx_send_pktlog(soc, rx_pdev, nbuf, QDF_TX_RX_STATUS_OK);

		/*
		 * process frame for mulitpass phrase processing
		 */
		if (qdf_unlikely(vdev->multipass_en)) {
			if (dp_rx_multipass_process(peer, nbuf, tid) == false) {
				DP_STATS_INC(peer, rx.multipass_rx_pkt_drop, 1);
				qdf_nbuf_free(nbuf);
				nbuf = next;
				continue;
			}
		}

		if (!dp_wds_rx_policy_check(rx_tlv_hdr, vdev, peer)) {
			dp_rx_err("%pK: Policy Check Drop pkt", soc);
			DP_STATS_INC(peer, rx.policy_check_drop, 1);
			tid_stats->fail_cnt[POLICY_CHECK_DROP]++;
			/* Drop & free packet */
			qdf_nbuf_free(nbuf);
			/* Statistics */
			nbuf = next;
			continue;
		}

		if (qdf_unlikely(peer && (peer->nawds_enabled) &&
				 (qdf_nbuf_is_da_mcbc(nbuf)) &&
				 (hal_rx_get_mpdu_mac_ad4_valid(soc->hal_soc,
								rx_tlv_hdr) ==
				  false))) {
			tid_stats->fail_cnt[NAWDS_MCAST_DROP]++;
			DP_STATS_INC(peer, rx.nawds_mcast_drop, 1);
			qdf_nbuf_free(nbuf);
			nbuf = next;
			continue;
		}

		/*
		 * Drop non-EAPOL frames from unauthorized peer.
		 */
		if (qdf_likely(peer) && qdf_unlikely(!peer->authorize) &&
		    !qdf_nbuf_is_raw_frame(nbuf)) {
			bool is_eapol = qdf_nbuf_is_ipv4_eapol_pkt(nbuf) ||
					qdf_nbuf_is_ipv4_wapi_pkt(nbuf);

			if (!is_eapol) {
				DP_STATS_INC(peer,
					     rx.peer_unauth_rx_pkt_drop, 1);
				qdf_nbuf_free(nbuf);
				nbuf = next;
				continue;
			}
		}

		if (soc->process_rx_status)
			dp_rx_cksum_offload(vdev->pdev, nbuf, rx_tlv_hdr);

		/* Update the protocol tag in SKB based on CCE metadata */
		dp_rx_update_protocol_tag(soc, vdev, nbuf, rx_tlv_hdr,
					  reo_ring_num, false, true);

		/* Update the flow tag in SKB based on FSE metadata */
		dp_rx_update_flow_tag(soc, vdev, nbuf, rx_tlv_hdr, true);

		dp_rx_msdu_stats_update(soc, nbuf, rx_tlv_hdr, peer,
					reo_ring_num, tid_stats);

		if (qdf_unlikely(vdev->mesh_vdev)) {
			if (dp_rx_filter_mesh_packets(vdev, nbuf, rx_tlv_hdr)
					== QDF_STATUS_SUCCESS) {
				dp_rx_info("%pK: mesh pkt filtered", soc);
				tid_stats->fail_cnt[MESH_FILTER_DROP]++;
				DP_STATS_INC(vdev->pdev, dropped.mesh_filter,
					     1);

				qdf_nbuf_free(nbuf);
				nbuf = next;
				continue;
			}
			dp_rx_fill_mesh_stats(vdev, nbuf, rx_tlv_hdr, peer);
		}

		if (qdf_likely(vdev->rx_decap_type ==
			       htt_cmn_pkt_type_ethernet) &&
		    qdf_likely(!vdev->mesh_vdev)) {
			dp_rx_wds_learn(soc, vdev,
					rx_tlv_hdr,
					peer,
					nbuf,
					msdu_metadata);

			/* Intrabss-fwd */
			if (dp_rx_check_ap_bridge(vdev))
				if (dp_rx_intrabss_fwd_be(soc, peer, rx_tlv_hdr,
							  nbuf,
							  msdu_metadata)) {
					nbuf = next;
					tid_stats->intrabss_cnt++;
					continue; /* Get next desc */
				}
		}

		dp_rx_fill_gro_info(soc, rx_tlv_hdr, nbuf, &rx_ol_pkt_cnt);

		dp_rx_mark_first_packet_after_wow_wakeup(vdev->pdev, rx_tlv_hdr,
							 nbuf);

		dp_rx_update_stats(soc, nbuf);
		DP_RX_LIST_APPEND(deliver_list_head,
				  deliver_list_tail,
				  nbuf);
		DP_PEER_TO_STACK_INCC_PKT(peer, 1, QDF_NBUF_CB_RX_PKT_LEN(nbuf),
					  enh_flag);
		if (qdf_unlikely(peer->in_twt))
			DP_STATS_INC_PKT(peer, rx.to_stack_twt, 1,
					 QDF_NBUF_CB_RX_PKT_LEN(nbuf));

		tid_stats->delivered_to_stack++;
		nbuf = next;
	}

	if (qdf_likely(deliver_list_head)) {
		if (qdf_likely(peer)) {
			dp_rx_deliver_to_pkt_capture(soc, vdev->pdev, peer_id,
						     pkt_capture_offload,
						     deliver_list_head);
			if (!pkt_capture_offload)
				dp_rx_deliver_to_stack(soc, vdev, peer,
						       deliver_list_head,
						       deliver_list_tail);
		} else {
			nbuf = deliver_list_head;
			while (nbuf) {
				next = nbuf->next;
				nbuf->next = NULL;
				dp_rx_deliver_to_stack_no_peer(soc, nbuf);
				nbuf = next;
			}
		}
	}

	if (qdf_likely(peer))
		dp_peer_unref_delete(peer, DP_MOD_ID_RX);

	/*
	 * If we are processing in near-full condition, there are 3 scenario
	 * 1) Ring entries has reached critical state
	 * 2) Ring entries are still near high threshold
	 * 3) Ring entries are below the safe level
	 *
	 * One more loop will move the state to normal processing and yield
	 */
	if (ring_near_full && quota)
		goto more_data;

	if (dp_rx_enable_eol_data_check(soc) && rx_bufs_used) {
		if (quota) {
			num_pending =
				dp_rx_srng_get_num_pending(hal_soc,
							   hal_ring_hdl,
							   num_entries,
							   &near_full);
			if (num_pending) {
				DP_STATS_INC(soc, rx.hp_oos2, 1);

				if (!hif_exec_should_yield(scn, intr_id))
					goto more_data;

				if (qdf_unlikely(near_full)) {
					DP_STATS_INC(soc, rx.near_full, 1);
					goto more_data;
				}
			}
		}

		if (vdev && vdev->osif_fisa_flush)
			vdev->osif_fisa_flush(soc, reo_ring_num);

		if (vdev && vdev->osif_gro_flush && rx_ol_pkt_cnt) {
			vdev->osif_gro_flush(vdev->osif_vdev,
					     reo_ring_num);
		}
	}

	/* Update histogram statistics by looping through pdev's */
	DP_RX_HIST_STATS_PER_PDEV();

	return rx_bufs_used; /* Assume no scale factor for now */
}

#ifdef RX_DESC_MULTI_PAGE_ALLOC
/**
 * dp_rx_desc_pool_init_be_cc() - initial RX desc pool for cookie conversion
 * @soc: Handle to DP Soc structure
 * @rx_desc_pool: Rx descriptor pool handler
 * @pool_id: Rx descriptor pool ID
 *
 * Return: QDF_STATUS_SUCCESS - succeeded, others - failed
 */
static QDF_STATUS
dp_rx_desc_pool_init_be_cc(struct dp_soc *soc,
			   struct rx_desc_pool *rx_desc_pool,
			   uint32_t pool_id)
{
	struct dp_hw_cookie_conversion_t *cc_ctx;
	struct dp_soc_be *be_soc;
	union dp_rx_desc_list_elem_t *rx_desc_elem;
	struct dp_spt_page_desc *page_desc;
	uint32_t ppt_idx = 0;
	uint32_t avail_entry_index = 0;

	if (!rx_desc_pool->pool_size) {
		dp_err("desc_num 0 !!");
		return QDF_STATUS_E_FAILURE;
	}

	be_soc = dp_get_be_soc_from_dp_soc(soc);
	cc_ctx  = &be_soc->rx_cc_ctx[pool_id];

	page_desc = &cc_ctx->page_desc_base[0];
	rx_desc_elem = rx_desc_pool->freelist;
	while (rx_desc_elem) {
		if (avail_entry_index == 0) {
			if (ppt_idx >= cc_ctx->total_page_num) {
				dp_alert("insufficient secondary page tables");
				qdf_assert_always(0);
			}
			page_desc = &cc_ctx->page_desc_base[ppt_idx++];
		}

		/* put each RX Desc VA to SPT pages and
		 * get corresponding ID
		 */
		DP_CC_SPT_PAGE_UPDATE_VA(page_desc->page_v_addr,
					 avail_entry_index,
					 &rx_desc_elem->rx_desc);
		rx_desc_elem->rx_desc.cookie =
			dp_cc_desc_id_generate(page_desc->ppt_index,
					       avail_entry_index);
		rx_desc_elem->rx_desc.pool_id = pool_id;
		rx_desc_elem->rx_desc.in_use = 0;
		rx_desc_elem = rx_desc_elem->next;

		avail_entry_index = (avail_entry_index + 1) &
					DP_CC_SPT_PAGE_MAX_ENTRIES_MASK;
	}

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS
dp_rx_desc_pool_init_be_cc(struct dp_soc *soc,
			   struct rx_desc_pool *rx_desc_pool,
			   uint32_t pool_id)
{
	struct dp_hw_cookie_conversion_t *cc_ctx;
	struct dp_soc_be *be_soc;
	struct dp_spt_page_desc *page_desc;
	uint32_t ppt_idx = 0;
	uint32_t avail_entry_index = 0;
	int i = 0;

	if (!rx_desc_pool->pool_size) {
		dp_err("desc_num 0 !!");
		return QDF_STATUS_E_FAILURE;
	}

	be_soc = dp_get_be_soc_from_dp_soc(soc);
	cc_ctx  = &be_soc->rx_cc_ctx[pool_id];

	page_desc = &cc_ctx->page_desc_base[0];
	for (i = 0; i <= rx_desc_pool->pool_size - 1; i++) {
		if (i == rx_desc_pool->pool_size - 1)
			rx_desc_pool->array[i].next = NULL;
		else
			rx_desc_pool->array[i].next =
				&rx_desc_pool->array[i + 1];

		if (avail_entry_index == 0) {
			if (ppt_idx >= cc_ctx->total_page_num) {
				dp_alert("insufficient secondary page tables");
				qdf_assert_always(0);
			}
			page_desc = &cc_ctx->page_desc_base[ppt_idx++];
		}

		/* put each RX Desc VA to SPT pages and
		 * get corresponding ID
		 */
		DP_CC_SPT_PAGE_UPDATE_VA(page_desc->page_v_addr,
					 avail_entry_index,
					 &rx_desc_pool->array[i].rx_desc);
		rx_desc_pool->array[i].rx_desc.cookie =
			dp_cc_desc_id_generate(page_desc->ppt_index,
					       avail_entry_index);
		rx_desc_pool->array[i].rx_desc.pool_id = pool_id;
		rx_desc_pool->array[i].rx_desc.in_use = 0;

		avail_entry_index = (avail_entry_index + 1) &
					DP_CC_SPT_PAGE_MAX_ENTRIES_MASK;
	}
	return QDF_STATUS_SUCCESS;
}
#endif

static void
dp_rx_desc_pool_deinit_be_cc(struct dp_soc *soc,
			     struct rx_desc_pool *rx_desc_pool,
			     uint32_t pool_id)
{
	struct dp_spt_page_desc *page_desc;
	struct dp_soc_be *be_soc;
	int i = 0;
	struct dp_hw_cookie_conversion_t *cc_ctx;

	be_soc = dp_get_be_soc_from_dp_soc(soc);
	cc_ctx  = &be_soc->rx_cc_ctx[pool_id];

	for (i = 0; i < cc_ctx->total_page_num; i++) {
		page_desc = &cc_ctx->page_desc_base[i];
		qdf_mem_zero(page_desc->page_v_addr, qdf_page_size);
	}
}

QDF_STATUS dp_rx_desc_pool_init_be(struct dp_soc *soc,
				   struct rx_desc_pool *rx_desc_pool,
				   uint32_t pool_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* Only regular RX buffer desc pool use HW cookie conversion */
	if (rx_desc_pool->desc_type == DP_RX_DESC_BUF_TYPE) {
		dp_info("rx_desc_buf pool init");
		status = dp_rx_desc_pool_init_be_cc(soc,
						    rx_desc_pool,
						    pool_id);
	} else {
		dp_info("non_rx_desc_buf_pool init");
		status = dp_rx_desc_pool_init_generic(soc, rx_desc_pool,
						      pool_id);
	}

	return status;
}

void dp_rx_desc_pool_deinit_be(struct dp_soc *soc,
			       struct rx_desc_pool *rx_desc_pool,
			       uint32_t pool_id)
{
	if (rx_desc_pool->desc_type == DP_RX_DESC_BUF_TYPE)
		dp_rx_desc_pool_deinit_be_cc(soc, rx_desc_pool, pool_id);
}

#ifdef DP_FEATURE_HW_COOKIE_CONVERSION
#ifdef DP_HW_COOKIE_CONVERT_EXCEPTION
QDF_STATUS dp_wbm_get_rx_desc_from_hal_desc_be(struct dp_soc *soc,
					       void *ring_desc,
					       struct dp_rx_desc **r_rx_desc)
{
	if (hal_rx_wbm_get_cookie_convert_done(ring_desc)) {
		/* HW cookie conversion done */
		*r_rx_desc = (struct dp_rx_desc *)
				hal_rx_wbm_get_desc_va(ring_desc);
	} else {
		/* SW do cookie conversion */
		uint32_t cookie = HAL_RX_BUF_COOKIE_GET(ring_desc);

		*r_rx_desc = (struct dp_rx_desc *)
				dp_cc_desc_find(soc, cookie);
	}

	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS dp_wbm_get_rx_desc_from_hal_desc_be(struct dp_soc *soc,
					       void *ring_desc,
					       struct dp_rx_desc **r_rx_desc)
{
	 *r_rx_desc = (struct dp_rx_desc *)
			hal_rx_wbm_get_desc_va(ring_desc);

	return QDF_STATUS_SUCCESS;
}
#endif /* DP_HW_COOKIE_CONVERT_EXCEPTION */
#else
QDF_STATUS dp_wbm_get_rx_desc_from_hal_desc_be(struct dp_soc *soc,
					       void *ring_desc,
					       struct dp_rx_desc **r_rx_desc)
{
	/* SW do cookie conversion */
	uint32_t cookie = HAL_RX_BUF_COOKIE_GET(ring_desc);

	*r_rx_desc = (struct dp_rx_desc *)
			dp_cc_desc_find(soc, cookie);

	return QDF_STATUS_SUCCESS;
}
#endif /* DP_FEATURE_HW_COOKIE_CONVERSION */

struct dp_rx_desc *dp_rx_desc_cookie_2_va_be(struct dp_soc *soc,
					     uint32_t cookie)
{
	return (struct dp_rx_desc *)dp_cc_desc_find(soc, cookie);
}

#ifdef WLAN_FEATURE_NEAR_FULL_IRQ
uint32_t dp_rx_nf_process(struct dp_intr *int_ctx,
			  hal_ring_handle_t hal_ring_hdl,
			  uint8_t reo_ring_num,
			  uint32_t quota)
{
	struct dp_soc *soc = int_ctx->soc;
	struct dp_srng *rx_ring = &soc->reo_dest_ring[reo_ring_num];
	uint32_t work_done = 0;

	if (dp_srng_get_near_full_level(soc, rx_ring) <
			DP_SRNG_THRESH_NEAR_FULL)
		return 0;

	qdf_atomic_set(&rx_ring->near_full, 1);
	work_done++;

	return work_done;
}
#endif

#ifndef QCA_HOST_MODE_WIFI_DISABLED
#ifdef WLAN_FEATURE_11BE_MLO
/**
 * dp_rx_intrabss_fwd_mlo_allow() - check if MLO forwarding is allowed
 * @ta_peer: transmitter peer handle
 * @da_peer: destination peer handle
 *
 * Return: true - MLO forwarding case, false: not
 */
static inline bool
dp_rx_intrabss_fwd_mlo_allow(struct dp_peer *ta_peer,
			     struct dp_peer *da_peer)
{
	/* one of TA/DA peer should belong to MLO connection peer,
	 * only MLD peer type is as expected
	 */
	if (!IS_MLO_DP_MLD_PEER(ta_peer) &&
	    !IS_MLO_DP_MLD_PEER(da_peer))
		return false;

	/* TA peer and DA peer's vdev should be partner MLO vdevs */
	if (dp_peer_find_mac_addr_cmp(&ta_peer->vdev->mld_mac_addr,
				      &da_peer->vdev->mld_mac_addr))
		return false;

	return true;
}
#else
static inline bool
dp_rx_intrabss_fwd_mlo_allow(struct dp_peer *ta_peer,
			     struct dp_peer *da_peer)
{
	return false;
}
#endif

#ifdef INTRA_BSS_FWD_OFFLOAD
/**
 * dp_rx_intrabss_ucast_check_be() - Check if intrabss is allowed
				     for unicast frame
 * @soc: SOC hanlde
 * @nbuf: RX packet buffer
 * @ta_peer: transmitter DP peer handle
 * @msdu_metadata: MSDU meta data info
 * @p_tx_vdev_id: get vdev id for Intra-BSS TX
 *
 * Return: true - intrabss allowed
	   false - not allow
 */
static bool
dp_rx_intrabss_ucast_check_be(struct dp_soc *soc, qdf_nbuf_t nbuf,
			      struct dp_peer *ta_peer,
			      struct hal_rx_msdu_metadata *msdu_metadata,
			      uint8_t *p_tx_vdev_id)
{
	uint16_t da_peer_id;
	struct dp_peer *da_peer;

	if (!qdf_nbuf_is_intra_bss(nbuf))
		return false;

	da_peer_id = dp_rx_peer_metadata_peer_id_get_be(
						soc,
						msdu_metadata->da_idx);
	da_peer = dp_peer_get_ref_by_id(soc, da_peer_id, DP_MOD_ID_RX);
	if (!da_peer)
		return false;
	*p_tx_vdev_id = da_peer->vdev->vdev_id;
	dp_peer_unref_delete(da_peer, DP_MOD_ID_RX);

	return true;
}
#else
static bool
dp_rx_intrabss_ucast_check_be(struct dp_soc *soc, qdf_nbuf_t nbuf,
			      struct dp_peer *ta_peer,
			      struct hal_rx_msdu_metadata *msdu_metadata,
			      uint8_t *p_tx_vdev_id)
{
	uint16_t da_peer_id;
	struct dp_peer *da_peer;
	bool ret = false;

	if (!qdf_nbuf_is_da_valid(nbuf) || qdf_nbuf_is_da_mcbc(nbuf))
		return false;

	da_peer_id = dp_rx_peer_metadata_peer_id_get_be(
						soc,
						msdu_metadata->da_idx);
	da_peer = dp_peer_get_ref_by_id(soc, da_peer_id,
					DP_MOD_ID_RX);
	if (!da_peer)
		return false;

	*p_tx_vdev_id = da_peer->vdev->vdev_id;
	/* If the source or destination peer in the isolation
	 * list then dont forward instead push to bridge stack.
	 */
	if (dp_get_peer_isolation(ta_peer) ||
	    dp_get_peer_isolation(da_peer))
		goto rel_da_peer;

	if (da_peer->bss_peer || da_peer == ta_peer)
		goto rel_da_peer;

	/* Same vdev, support Inra-BSS */
	if (da_peer->vdev == ta_peer->vdev) {
		ret = true;
		goto rel_da_peer;
	}

	/* MLO specific Intra-BSS check */
	if (dp_rx_intrabss_fwd_mlo_allow(ta_peer, da_peer)) {
		ret = true;
		goto rel_da_peer;
	}

rel_da_peer:
	dp_peer_unref_delete(da_peer, DP_MOD_ID_RX);
	return ret;
}
#endif
/*
 * dp_rx_intrabss_fwd_be() - API for intrabss fwd. For EAPOL
 *  pkt with DA not equal to vdev mac addr, fwd is not allowed.
 * @soc: core txrx main context
 * @ta_peer: source peer entry
 * @rx_tlv_hdr: start address of rx tlvs
 * @nbuf: nbuf that has to be intrabss forwarded
 * @msdu_metadata: msdu metadata
 *
 * Return: true if it is forwarded else false
 */
bool dp_rx_intrabss_fwd_be(struct dp_soc *soc, struct dp_peer *ta_peer,
			   uint8_t *rx_tlv_hdr, qdf_nbuf_t nbuf,
			   struct hal_rx_msdu_metadata msdu_metadata)
{
	uint8_t tx_vdev_id;
	uint8_t tid = qdf_nbuf_get_tid_val(nbuf);
	uint8_t ring_id = QDF_NBUF_CB_RX_CTX_ID(nbuf);
	struct cdp_tid_rx_stats *tid_stats = &ta_peer->vdev->pdev->stats.
					tid_stats.tid_rx_stats[ring_id][tid];

	/* if it is a broadcast pkt (eg: ARP) and it is not its own
	 * source, then clone the pkt and send the cloned pkt for
	 * intra BSS forwarding and original pkt up the network stack
	 * Note: how do we handle multicast pkts. do we forward
	 * all multicast pkts as is or let a higher layer module
	 * like igmpsnoop decide whether to forward or not with
	 * Mcast enhancement.
	 */
	if (qdf_nbuf_is_da_mcbc(nbuf) && !ta_peer->bss_peer)
		return dp_rx_intrabss_mcbc_fwd(soc, ta_peer, rx_tlv_hdr,
					       nbuf, tid_stats);

	if (dp_rx_intrabss_ucast_check_be(soc, nbuf, ta_peer,
					  &msdu_metadata, &tx_vdev_id))
		return dp_rx_intrabss_ucast_fwd(soc, ta_peer, tx_vdev_id,
						rx_tlv_hdr, nbuf, tid_stats);

	return false;
}
#endif
